// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-10  (fase E, 6 j) - Espai de treball visual i controls coherents
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Editor/EditorDocumentSession.h"
#include "Editor/EditorDocumentIO.h"
#include "Level/LevelLoader.h"
#include "Project/ProjectSerializer.h"
#include "TestHarness.h"
#include "Core/Math/IsoMath.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <limits>

namespace {
namespace fs = std::filesystem;
using Session = Editor::EditorDocumentSession;
using Action = Session::Action;
using Choice = Session::Choice;
struct Fixture {
    fs::path root = fs::temp_directory_path() / ("motor-editor-session-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Fixture() {
        fs::create_directory(root);
        fs::copy_file("assets/maps/ejemplo_mapa.tmx", root / "first.tmx");
        fs::copy_file("assets/maps/ejemplo_mapa.tmx", root / "second.tmx");
        for (const auto* id : {"first", "second"}) {
            std::ofstream out(path(std::string(id) + ".json"));
            out << "{\"name\":\"" << id << "\",\"map\":\"" << path(std::string(id) + ".tmx")
                << R"(","playerStart":{"x":1,"y":1},"objects":[{"objectId":"pocion","position":{"x":2,"y":1},"displayName":"Botin local","scale":1.5,"variant":"roja","loot":["cofre"],"eventId":"encontrado","properties":{"autor":"fixture"}}]})";
        }
    }
    ~Fixture() { std::error_code ec; fs::remove_all(root, ec); }
    std::string path(const std::string& name) const { return (root / name).generic_string(); }
    void open(Session& s) { MOTOR_CHECK(s.request(Action::Open, path("first.json")).isOk()); }
};

MOTOR_TEST_CASE(cancel_preserves_document_and_undo_returns_to_saved_state) {
    Fixture f; Session s; f.open(s);
    s.paint(1, 1, 2);
    MOTOR_CHECK(s.dirty());
    const auto open = s.request(Action::Open, f.path("second.json"));
    MOTOR_CHECK(open.isOk() && !open.value());
    MOTOR_CHECK(s.confirmationPending() && s.name() == "first");
    s.paint(2, 1, 2); // A modal stops map actions too.
    MOTOR_CHECK(s.state().tileAt(2, 1) == 1);
    MOTOR_CHECK(s.resolve(Choice::Cancel).isOk());
    MOTOR_CHECK(s.dirty() && !s.confirmationPending());
    s.undo(); MOTOR_CHECK(!s.dirty());
    s.redo(); MOTOR_CHECK(s.dirty());
}

MOTOR_TEST_CASE(save_before_switch_reopens_native_tiles_start_and_entities) {
    Fixture f; Session s; f.open(s);
    s.paint(1, 1, 2); s.playerStart({2, 3});
    MOTOR_CHECK(s.request(Action::Open, f.path("second.json")).isOk());
    MOTOR_CHECK(s.resolve(Choice::Save).isOk());
    MOTOR_CHECK(s.name() == "second" && !s.dirty());
    MOTOR_CHECK(s.request(Action::Open, f.path("first.json")).isOk());
    MOTOR_CHECK(s.state().tileAt(1, 1) == 2);
    MOTOR_CHECK(s.state().playerStart().x == 2 && s.state().playerStart().y == 3);
    MOTOR_CHECK(s.state().objects().size() == 1);
    MOTOR_CHECK(s.state().objects()[0].objectId == "pocion");
    MOTOR_CHECK(fs::is_regular_file(f.path("first.map.json")));
}

MOTOR_TEST_CASE(failed_save_keeps_dirty_document_and_pending_close) {
    Fixture f; Session s; f.open(s); s.paint(1, 1, 2);
    fs::create_directory(f.path("first.map.json"));
    MOTOR_CHECK(s.request(Action::Quit).isOk());
    const auto saved = s.resolve(Choice::Save);
    MOTOR_CHECK(!saved.isOk());
    MOTOR_CHECK(s.hasDocument() && s.dirty() && s.confirmationPending() && !s.quitReady());
    MOTOR_CHECK(s.state().tileAt(1, 1) == 2);
    MOTOR_CHECK(s.resolve(Choice::Cancel).isOk());
    MOTOR_CHECK(!s.quitReady());
}

MOTOR_TEST_CASE(failed_open_is_transactional_even_after_discard_choice) {
    Fixture f; Session s; f.open(s);
    MOTOR_CHECK(!s.request(Action::Open, f.path("missing.json")).isOk());
    MOTOR_CHECK(s.name() == "first");
    s.paint(1, 1, 2);
    MOTOR_CHECK(s.request(Action::Open, f.path("missing.json")).isOk());
    MOTOR_CHECK(!s.resolve(Choice::Discard).isOk());
    MOTOR_CHECK(s.dirty() && s.name() == "first" && s.confirmationPending());
}

MOTOR_TEST_CASE(discard_and_close_leave_original_files_and_quit_can_be_cancelled) {
    Fixture f; Session s; f.open(s); s.paint(1, 1, 2);
    MOTOR_CHECK(s.request(Action::Close).isOk());
    MOTOR_CHECK(s.resolve(Choice::Discard).isOk());
    MOTOR_CHECK(!s.hasDocument() && !s.quitReady());
    f.open(s); MOTOR_CHECK(s.state().tileAt(1, 1) == 1);
    MOTOR_CHECK(!fs::exists(f.path("first.map.json")));
    s.playerStart({2, 3}); MOTOR_CHECK(s.request(Action::Quit).isOk());
    MOTOR_CHECK(s.resolve(Choice::Cancel).isOk());
    MOTOR_CHECK(!s.quitReady() && s.dirty());
    MOTOR_CHECK(s.request(Action::Quit).isOk());
    MOTOR_CHECK(s.resolve(Choice::Discard).isOk()); MOTOR_CHECK(s.quitReady());
}

MOTOR_TEST_CASE(viewport_coordinates_obey_focus_text_modals_offsets_and_retina) {
    Editor::ViewportInput input{100, 80, 400, 300, 2, 2, true, false, false};
    const auto p = input.framebufferPoint(125, 100);
    MOTOR_CHECK(p && p->x == 50 && p->y == 40);
    MOTOR_CHECK(!input.framebufferPoint(99, 100));
    MOTOR_CHECK(!input.framebufferPoint(500, 100));
    input.textEditing = true; MOTOR_CHECK(!input.framebufferPoint(125, 100));
    input.textEditing = false; input.modal = true; MOTOR_CHECK(!input.framebufferPoint(125, 100));
    input.modal = false; input.hovered = false; MOTOR_CHECK(!input.framebufferPoint(125, 100));
    input.hovered = true; input.scaleX = 0; MOTOR_CHECK(!input.framebufferPoint(125, 100));
    input.scaleX = 2; MOTOR_CHECK(!input.framebufferPoint(std::numeric_limits<float>::quiet_NaN(), 100));
}

MOTOR_TEST_CASE(clicking_rendered_tile_center_selects_that_cell_not_its_neighbor) {
    for (const auto cell : {GridCoord{0, 0}, GridCoord{1, 3}, GridCoord{40, 30}}) {
        auto point = IsoMath::gridToScreen(cell, 64, 32);
        point.x += 32; point.y += 16;
        const auto picked = Editor::renderedMapCell(point, 64, 32);
        MOTOR_CHECK(picked.x == cell.x && picked.y == cell.y);
    }
}
MOTOR_TEST_CASE(object_placement_preserves_existing_instance_and_rejects_unknown_or_outside) {
    Fixture f; Session s; f.open(s); ObjectCatalog catalog;
    MOTOR_CHECK(catalog.loadFromString(R"({"objects":[{"id":"pocion","category":"pickup"},{"id":"cofre","category":"prop"}]})").isOk());
    const auto original = ProjectSerializer::serializeMap(s.state().document()).value();
    const auto revision = s.revision();
    MOTOR_CHECK(!s.placeObject({2,1},"pocion",catalog).value());
    MOTOR_CHECK(!s.placeObject({2,1},"cofre",catalog).isOk());
    MOTOR_CHECK(!s.placeObject({3,1},"unknown",catalog).isOk());
    MOTOR_CHECK(!s.placeObject({-1,1},"cofre",catalog).value());
    MOTOR_CHECK(!s.placeObject({s.state().width(),1},"cofre",catalog).value());
    MOTOR_CHECK(!s.removeObject({-1,1}).value());
    MOTOR_CHECK(!s.dirty() && !s.state().canUndo() && s.revision() == revision);
    MOTOR_CHECK(ProjectSerializer::serializeMap(s.state().document()).value() == original);
    MOTOR_CHECK(s.placeObject({3,1},"cofre",catalog).value());
    MOTOR_CHECK(s.state().objectAt(3,1)->patrolMin.x == 3 && s.state().tileAt(3,1) == 1);
    s.undo(); MOTOR_CHECK(!s.dirty());
    s.redo(); MOTOR_CHECK(s.save().isOk());
    f.open(s); MOTOR_CHECK(s.state().objectAt(3,1)->objectId == "cofre");
}

MOTOR_TEST_CASE(object_removal_preserves_native_metadata_through_undo_and_respects_locks) {
    Fixture f; Session s; f.open(s);
    auto document = s.state().document();
    auto layerId = ids::LayerId::parse("level_objects").value();
    auto meta = document.findLayer(layerId)->meta();
    meta.locked = true;
    MOTOR_CHECK(document.setLayerMeta(layerId,meta).isOk());
    MOTOR_CHECK(ProjectSerializer::saveMapAtomic(f.path("first.map.json"),document).isOk());
    // Register the native document without changing its locked entity layer.
    std::ofstream out(f.path("first.json"));
    out << "{\"name\":\"locked\",\"map\":\"" << f.path("first.tmx")
        << "\",\"mapDocument\":\"" << f.path("first.map.json") << "\",\"playerStart\":{\"x\":1,\"y\":1}}";
    out.close(); f.open(s);
    MOTOR_CHECK(!s.removeObject({2,1}).isOk());
    MOTOR_CHECK(s.state().objectAt(2,1) && !s.dirty() && !s.state().canUndo());
    meta.locked = false;
    MOTOR_CHECK(document.setLayerMeta(layerId,meta).isOk());
    MOTOR_CHECK(ProjectSerializer::saveMapAtomic(f.path("first.map.json"),document).isOk());
    f.open(s);
    const auto original = ProjectSerializer::serializeMap(s.state().document()).value();
    MOTOR_CHECK(s.removeObject({2,1}).value());
    MOTOR_CHECK(!s.state().objectAt(2,1));
    s.undo();
    MOTOR_CHECK(ProjectSerializer::serializeMap(s.state().document()).value() == original && !s.dirty());
    s.redo(); MOTOR_CHECK(s.save().isOk()); f.open(s);
    MOTOR_CHECK(!s.state().objectAt(2,1));
}
}  // namespace

int main() { return motor_test::runAllTests(); }
