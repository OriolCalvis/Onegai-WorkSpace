#include "Editor/EditorActions.h"
#include "TestHarness.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <set>
#include <tuple>

namespace {
namespace fs = std::filesystem;
using namespace Editor;
using Id = EditorAction;
using Session = EditorDocumentSession;
struct Fixture {
    fs::path root = fs::temp_directory_path() / ("motor-actions-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Session session;
    EditorActions actions;
    Fixture() {
        fs::create_directory(root);
        fs::copy_file("assets/maps/ejemplo_mapa.tmx", root / "map.tmx");
        std::ofstream out(root / "level.json");
        out << "{\"name\":\"Actions\",\"map\":\"" << (root / "map.tmx").generic_string()
            << "\",\"playerStart\":{\"x\":1,\"y\":1},\"objects\":[{\"objectId\":\"pocion\",\"position\":{\"x\":2,\"y\":3}}]}";
        out.close();
        MOTOR_CHECK(session.request(Session::Action::Open, (root / "level.json").string()).isOk());
    }
    ~Fixture() { std::error_code ec; fs::remove_all(root, ec); }
    void execute(Id id) { MOTOR_CHECK(actions.execute(id, session).isOk()); }
    void click(int x, int y, int gid = 2) { actions.applyAt(session, {x,y}, gid); }
};

MOTOR_TEST_CASE(action_table_has_unique_actions_and_unambiguous_advertised_shortcuts) {
    std::set<Id> ids;
    std::set<std::tuple<std::string,bool,bool>> chords;
    for (const auto& def : editorActionDefinitions()) {
        MOTOR_CHECK(ids.insert(def.id).second && *def.label && *def.key);
        MOTOR_CHECK(chords.emplace(def.key, def.primary, def.shift).second);
        MOTOR_CHECK(shortcutLabel(def, false).find(def.key) != std::string::npos);
        if (def.primary) MOTOR_CHECK(shortcutLabel(def, true).find("Cmd+") == 0);
        if (*def.alternateKey) MOTOR_CHECK(chords.emplace(def.alternateKey, def.primary, false).second);
    }
    MOTOR_CHECK(ids.size() == 14);
}

MOTOR_TEST_CASE(redo_uses_shift_z_or_primary_y_without_triggering_undo) {
    Fixture f; f.click(1,1); f.execute(Id::Undo);
    ActionInput input{false,false,false,true};
    const auto shiftZ = f.actions.shortcut("Z",true,true,false,input,f.session);
    const auto y = f.actions.shortcut("Y",true,false,false,input,f.session);
    MOTOR_CHECK(shiftZ == Id::Redo && y == Id::Redo);
    f.execute(*shiftZ);
    MOTOR_CHECK(f.session.dirty() && f.session.state().tileAt(1,1) == 2);
    MOTOR_CHECK(f.actions.shortcut("Z",true,false,false,input,f.session) == Id::Undo);
    MOTOR_CHECK(!f.actions.shortcut("Z",true,true,false,input,f.session)); // no redo left
    MOTOR_CHECK(!f.actions.shortcut("G",true,false,false,input,f.session));
    MOTOR_CHECK(!f.actions.shortcut("G",false,true,false,input,f.session));
    MOTOR_CHECK(!f.actions.shortcut("G",false,false,true,input,f.session));
}

MOTOR_TEST_CASE(text_widgets_active_items_and_modals_capture_all_shortcuts) {
    Fixture f; f.click(1,1);
    for (const auto input : {ActionInput{true,false,false,true}, ActionInput{false,true,false,true}, ActionInput{false,false,true,true}}) {
        for (const auto& def : editorActionDefinitions())
            MOTOR_CHECK(!f.actions.shortcut(def.key,def.primary,def.shift,false,input,f.session));
    }
    MOTOR_CHECK(f.session.dirty());
    MOTOR_CHECK(f.actions.tool() == Id::Paint);
}

MOTOR_TEST_CASE(tool_shortcuts_require_map_focus_but_document_shortcuts_do_not) {
    Fixture f;
    ActionInput outside;
    for (const auto key : {"B","E","G","P","O","X","Home"})
        MOTOR_CHECK(!f.actions.shortcut(key,false,false,false,outside,f.session));
    MOTOR_CHECK(f.actions.shortcut("S",true,false,false,outside,f.session) == Id::Save);
    outside.viewportFocused = true;
    for (const auto& def : editorActionDefinitions())
        if (def.group == ActionGroup::Tools)
            MOTOR_CHECK(f.actions.shortcut(def.key,false,false,false,outside,f.session) == def.id);
}

MOTOR_TEST_CASE(fill_is_one_undo_step_and_preserves_entities_and_start) {
    Fixture f; f.execute(Id::Fill); f.click(1,1);
    MOTOR_CHECK(f.session.state().tileAt(1,1) == 2 && f.session.state().tileAt(6,4) == 2);
    MOTOR_CHECK(f.session.state().objects().size() == 1 && f.session.state().playerStart().x == 1);
    MOTOR_CHECK(f.session.dirty());
    f.execute(Id::Undo);
    MOTOR_CHECK(!f.session.dirty() && !f.session.state().canUndo());
    MOTOR_CHECK(f.session.state().tileAt(1,1) == 1 && f.session.state().tileAt(6,4) == 1);
    f.execute(Id::Redo); f.execute(Id::Save);
    MOTOR_CHECK(f.session.request(Session::Action::Open, (f.root/"level.json").string()).isOk());
    MOTOR_CHECK(f.session.state().tileAt(6,4) == 2 && !f.session.dirty());
    MOTOR_CHECK(f.session.state().objects().size() == 1);
}

MOTOR_TEST_CASE(erase_and_right_click_use_the_same_history_without_flood_erasing) {
    Fixture f; f.execute(Id::Erase); f.click(1,1);
    MOTOR_CHECK(f.session.state().tileAt(1,1) == 0 && f.session.state().tileAt(2,1) == 1);
    f.execute(Id::Undo); MOTOR_CHECK(!f.session.dirty());
    f.execute(Id::Fill); f.actions.applyAt(f.session,{1,1},2,true);
    MOTOR_CHECK(f.session.state().tileAt(1,1) == 0 && f.session.state().tileAt(2,1) == 1);
    MOTOR_CHECK(f.actions.tool() == Id::Fill);
    f.execute(Id::Undo); MOTOR_CHECK(!f.session.dirty());
    f.execute(Id::PlayerStart); f.click(3,3);
    MOTOR_CHECK(f.session.state().playerStart().x == 3 && f.session.state().playerStart().y == 3);
    f.execute(Id::Undo); MOTOR_CHECK(!f.session.dirty());
}

MOTOR_TEST_CASE(pending_close_blocks_buttons_tools_and_shortcuts_until_cancel) {
    Fixture f; f.click(1,1); f.execute(Id::Close);
    MOTOR_CHECK(f.session.confirmationPending());
    for (const auto& def : editorActionDefinitions()) MOTOR_CHECK(!f.actions.enabled(def.id,f.session));
    f.execute(Id::Fill); f.click(2,1);
    MOTOR_CHECK(f.actions.tool() == Id::Paint && f.session.state().tileAt(2,1) == 1);
    MOTOR_CHECK(f.session.resolve(Session::Choice::Cancel).isOk());
    MOTOR_CHECK(f.session.hasDocument() && f.session.dirty());
}

MOTOR_TEST_CASE(fill_rejects_unknown_tiles_and_outside_cells_without_history) {
    Fixture f; f.execute(Id::Fill);
    f.click(1,1,-1); f.click(1,1,99); f.click(-1,1); f.click(8,6);
    MOTOR_CHECK(!f.session.dirty() && !f.session.state().canUndo());
    f.click(0,0); // already gid 2: no history
    MOTOR_CHECK(!f.session.dirty() && !f.session.state().canUndo());
}

MOTOR_TEST_CASE(help_and_center_are_view_state_and_empty_sessions_disable_editing) {
    EditorActions actions; Session empty;
    MOTOR_CHECK(!actions.enabled(Id::Fill,empty) && !actions.enabled(Id::Save,empty));
    MOTOR_CHECK(actions.execute(Id::Help,empty).isOk() && actions.helpVisible());
    MOTOR_CHECK(actions.execute(Id::Help,empty).isOk() && !actions.helpVisible());
    Fixture f; f.execute(Id::Center);
    MOTOR_CHECK(f.actions.takeCenterRequest() && !f.actions.takeCenterRequest());
    MOTOR_CHECK(!f.session.dirty() && !f.session.state().canUndo());
}
MOTOR_TEST_CASE(object_tools_use_catalog_and_history_without_changing_tiles) {
    Fixture f; ObjectCatalog catalog;
    MOTOR_CHECK(catalog.loadFromString(R"({"objects":[{"id":"pocion","name":"Pocion","category":"pickup"}]})").isOk());
    f.execute(Id::Objects);
    MOTOR_CHECK(!f.actions.applyAt(f.session,{3,1},2).isOk());
    MOTOR_CHECK(!f.actions.applyAt(f.session,{3,1},2,false,&catalog,"missing").isOk());
    MOTOR_CHECK(!f.session.dirty());
    MOTOR_CHECK(f.actions.applyAt(f.session,{3,1},2,false,&catalog,"pocion").isOk());
    MOTOR_CHECK(f.session.state().objectAt(3,1) && f.session.state().tileAt(3,1) == 1);
    MOTOR_CHECK(f.session.dirty());
    f.execute(Id::Undo); MOTOR_CHECK(!f.session.dirty());
    f.execute(Id::Redo);
    MOTOR_CHECK(f.actions.applyAt(f.session,{3,1},2,true).isOk());
    MOTOR_CHECK(!f.session.state().objectAt(3,1) && f.session.state().tileAt(3,1) == 1);
    f.execute(Id::Undo);
    f.execute(Id::RemoveObject); f.click(3,1);
    MOTOR_CHECK(!f.session.state().objectAt(3,1));
    f.execute(Id::Undo); f.execute(Id::Save);
    MOTOR_CHECK(f.session.request(Session::Action::Open,(f.root/"level.json").string()).isOk());
    MOTOR_CHECK(f.session.state().objectAt(3,1) && !f.session.dirty());
}

MOTOR_TEST_CASE(object_tools_do_not_run_during_pending_close) {
    Fixture f; f.execute(Id::RemoveObject); f.click(2,3); f.execute(Id::Close);
    MOTOR_CHECK(f.session.confirmationPending());
    f.execute(Id::Objects);
    MOTOR_CHECK(!f.actions.applyAt(f.session,{2,3},2).value());
    MOTOR_CHECK(f.actions.tool() == Id::RemoveObject);
    MOTOR_CHECK(f.session.resolve(Session::Choice::Cancel).isOk());
    f.execute(Id::Undo); MOTOR_CHECK(!f.session.dirty() && f.session.state().objectAt(2,3));
}
} // namespace
int main() { return motor_test::runAllTests(); }
