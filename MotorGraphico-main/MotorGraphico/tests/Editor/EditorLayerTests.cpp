#include "Editor/EditorDocumentSession.h"
#include "Project/ProjectSerializer.h"
#include "TestHarness.h"
#include <chrono>
#include <filesystem>
#include <fstream>

namespace {
namespace fs = std::filesystem;
using Session = Editor::EditorDocumentSession;
struct Fixture {
    fs::path root = fs::temp_directory_path() / ("motor-editor-layers-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Session session;
    Fixture() {
        fs::create_directory(root);
        fs::copy_file("assets/maps/ejemplo_mapa.tmx", root/"map.tmx");
        std::ofstream out(root/"level.json");
        out << "{\"name\":\"Capas\",\"map\":\"" << (root/"map.tmx").generic_string()
            << R"(","playerStart":{"x":1,"y":1},"objects":[{"objectId":"pocion","position":{"x":2,"y":1}}]})";
        out.close(); open();
    }
    ~Fixture() { std::error_code ec; fs::remove_all(root,ec); }
    void open() { MOTOR_CHECK(session.request(Session::Action::Open,(root/"level.json").string()).isOk()); }
};
MOTOR_TEST_CASE(layer_selection_is_view_state_without_dirty_history_or_new_document_generation) {
    Fixture f; auto& s=f.session; const auto first=s.activeTileLayer();
    MOTOR_CHECK(s.addTileLayer().value()); const auto second=s.activeTileLayer();
    MOTOR_CHECK(first!=second && s.state().layerCount()==2);
    MOTOR_CHECK(s.save().isOk()); const auto revision=s.revision(), generation=s.generation();
    MOTOR_CHECK(s.selectTileLayer(first).isOk()); MOTOR_CHECK(s.activeTileLayer()==first);
    MOTOR_CHECK(!s.dirty() && s.revision()==revision && s.generation()==generation);
    s.undo(); MOTOR_CHECK(s.state().layerCount()==1); // selecting did not add an undo step
    s.redo(); MOTOR_CHECK(s.state().layerCount()==2);
}
MOTOR_TEST_CASE(layer_visibility_lock_and_name_roundtrip_preserve_entities_and_identity) {
    Fixture f; auto& s=f.session;
    MOTOR_CHECK(s.addTileLayer().value()); const auto id=s.activeTileLayer();
    s.paint(3,1,2);
    auto meta=s.tileLayers().back(); meta.name="Decoración ## local"; meta.visible=false; meta.locked=true;
    MOTOR_CHECK(s.updateTileLayer(meta).value());
    s.paint(1,1,2); s.fill(1,1,2);
    MOTOR_CHECK(s.state().tileAt(1,1)==0 && s.state().tileAt(3,1)==2);
    MOTOR_CHECK(s.state().objects().size()==1 && s.state().playerStart().x==1);
    MOTOR_CHECK(s.save().isOk()); f.open();
    const auto loaded=s.tileLayers().back();
    MOTOR_CHECK(loaded.id==id && loaded.name==meta.name && !loaded.visible && loaded.locked);
    MOTOR_CHECK(s.selectTileLayer(id).isOk() && s.state().tileAt(3,1)==2);
    MOTOR_CHECK(s.state().objects().size()==1 && !s.dirty());
}
MOTOR_TEST_CASE(layer_metadata_is_one_undo_step_and_no_op_does_not_dirty_the_document) {
    Fixture f; auto& s=f.session; auto meta=s.tileLayers().front();
    const auto revision=s.revision();
    MOTOR_CHECK(!s.updateTileLayer(meta).value());
    MOTOR_CHECK(!s.dirty() && !s.state().canUndo() && s.revision()==revision);
    meta.name="Suelo nuevo"; meta.visible=false;
    MOTOR_CHECK(s.updateTileLayer(meta).value());
    s.undo(); MOTOR_CHECK(!s.dirty() && s.tileLayers().front().visible);
    s.redo(); MOTOR_CHECK(s.tileLayers().front().name=="Suelo nuevo" && s.dirty());
}
MOTOR_TEST_CASE(removing_layer_keeps_tiles_on_other_layers_and_can_be_undone_exactly) {
    Fixture f; auto& s=f.session; const auto first=s.activeTileLayer();
    MOTOR_CHECK(!s.removeTileLayer(first).isOk()); MOTOR_CHECK(!s.dirty());
    MOTOR_CHECK(s.addTileLayer().value()); const auto second=s.activeTileLayer(); s.paint(3,1,2);
    const auto before=ProjectSerializer::serializeMap(s.state().document()).value();
    MOTOR_CHECK(s.removeTileLayer(second).value());
    MOTOR_CHECK(s.activeTileLayer()==first && s.state().tileAt(3,1)==1 && s.state().objects().size()==1);
    s.undo(); MOTOR_CHECK(ProjectSerializer::serializeMap(s.state().document()).value()==before);
    MOTOR_CHECK(s.activeTileLayer()==second && s.state().tileAt(3,1)==2);
    s.redo(); MOTOR_CHECK(s.save().isOk()); f.open();
    MOTOR_CHECK(s.state().layerCount()==1 && s.state().objects().size()==1);
}
MOTOR_TEST_CASE(layer_commands_reject_stale_ids_blank_names_locked_removal_and_pending_document) {
    Fixture f; auto& s=f.session; auto meta=s.tileLayers().front();
    meta.name=" \t "; MOTOR_CHECK(!s.updateTileLayer(meta).isOk());
    const auto missing=ids::LayerId::parse("missing").value();
    MOTOR_CHECK(!s.selectTileLayer(missing).isOk() && !s.removeTileLayer(missing).isOk());
    meta=s.tileLayers().front(); meta.id=missing; MOTOR_CHECK(!s.updateTileLayer(meta).isOk());
    MOTOR_CHECK(!s.dirty() && !s.state().canUndo());
    MOTOR_CHECK(s.addTileLayer().value()); meta=s.tileLayers().back(); meta.locked=true;
    MOTOR_CHECK(s.updateTileLayer(meta).value()); MOTOR_CHECK(!s.removeTileLayer(meta.id).isOk());
    MOTOR_CHECK(s.request(Session::Action::Close).isOk());
    MOTOR_CHECK(!s.addTileLayer().value()); meta.name="No aplicar";
    MOTOR_CHECK(!s.updateTileLayer(meta).value() && !s.removeTileLayer(meta.id).value());
    MOTOR_CHECK(!s.selectTileLayer(s.tileLayers().front().id).value());
    MOTOR_CHECK(s.resolve(Session::Choice::Cancel).isOk() && s.tileLayers().back().name!="No aplicar");
}
MOTOR_TEST_CASE(layer_names_do_not_replace_ids_and_empty_sessions_are_safe) {
    Fixture f; auto& s=f.session; auto first=s.tileLayers().front();
    MOTOR_CHECK(s.addTileLayer().value()); auto second=s.tileLayers().back(); second.name=first.name;
    MOTOR_CHECK(s.updateTileLayer(second).isOk());
    MOTOR_CHECK(first.id!=second.id && s.tileLayers().front().name==s.tileLayers().back().name);
    Session empty;
    MOTOR_CHECK(empty.tileLayers().empty() && empty.activeTileLayer().empty());
    MOTOR_CHECK(!empty.addTileLayer().value() && !empty.removeTileLayer(first.id).value());
}
}
int main() { return motor_test::runAllTests(); }
