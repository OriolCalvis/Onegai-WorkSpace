#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "Map/MapDocument.h"
#include "Project/ProjectSerializer.h"
#include "TestHarness.h"

namespace {

template <class Id>
Id id(const char* value) {
    const Result<Id> result = Id::parse(value);
    if (!result.isOk()) {
        throw std::runtime_error(result.errorMessage());
    }
    return result.value();
}

Editor::Project makeProject() {
    Editor::Project project;
    project.id = "demo_project";
    project.name = "Proyecto de serialización";
    project.description = "Fixture canónica";
    project.epoch = "Otoño";
    project.author = "Pruebas";
    project.prefix = "demo_";
    project.entry = "adventures/inicio.json";
    project.maps = {"inicio.map.json"};
    project.levels = {"inicio.json"};
    project.adventures = {"inicio.json"};
    project.catalogs = {"objetos.json"};
    project.settings.initialMap = "inicio.map.json";
    return project;
}

MapDocument makeMap() {
    MapDocument map(id<ids::MapId>("inicio"), "Inicio", 3, 2, 64, 32);
    map.addLayer(MapLayer::tiles({id<ids::LayerId>("suelo"), "Suelo", true, false, 0.75f, 10},
                                 TileLayerDocument(3, 2, {1, 2, 3, 4, 5, 6})));
    map.addLayer(MapLayer::entities(
        {id<ids::LayerId>("entidades"), "Entidades", true, false, 1.0f, 20},
        EntityLayerDocument(
            {{id<ids::EntityId>("guia"), id<ids::AssetId>("npc_guia"), {1, 1}}})));
    map.addLayer(MapLayer::regions(
        {id<ids::LayerId>("zonas"), "Zonas", true, true, 1.0f, 30},
        RegionLayerDocument({{id<ids::RegionId>("salida"), "Salida", {2, 2, 0, 1}}})));
    map.setCollisionGids({2, 5});
    return map;
}

MOTOR_TEST_CASE(project_json_is_canonical_and_round_trips) {
    const Result<std::string> first = ProjectSerializer::serializeProject(makeProject());
    MOTOR_CHECK(first.isOk());
    const Result<Editor::Project> decoded = ProjectSerializer::deserializeProject(first.value());
    MOTOR_CHECK(decoded.isOk());
    const Result<std::string> second = ProjectSerializer::serializeProject(decoded.value());

    MOTOR_CHECK(second.isOk());
    MOTOR_CHECK(first.value() == second.value());
    MOTOR_CHECK(decoded.value().settings.initialMap == "inicio.map.json");
}

MOTOR_TEST_CASE(map_json_round_trips_all_layer_types_and_ids) {
    const Result<std::string> first = ProjectSerializer::serializeMap(makeMap());
    MOTOR_CHECK(first.isOk());
    const Result<MapDocument> decoded = ProjectSerializer::deserializeMap(first.value());
    MOTOR_CHECK(decoded.isOk());
    const Result<std::string> second = ProjectSerializer::serializeMap(decoded.value());

    MOTOR_CHECK(second.isOk());
    MOTOR_CHECK(first.value() == second.value());
    MOTOR_CHECK(decoded.value().layers().size() == 3);
    MOTOR_CHECK(decoded.value().collisionGids() == std::vector<int>({2, 5}));
    MOTOR_CHECK(decoded.value().layers()[1].meta().id == id<ids::LayerId>("entidades"));
    MOTOR_CHECK(std::get<EntityLayerDocument>(decoded.value().layers()[1].data()).entities()[0]
                    .objectId == id<ids::AssetId>("npc_guia"));
}

MOTOR_TEST_CASE(corrupt_or_incompatible_json_returns_a_descriptive_error) {
    const Result<Editor::Project> invalidProject =
        ProjectSerializer::deserializeProject("{\"schemaVersion\": 99}");
    const Result<MapDocument> invalidMap =
        ProjectSerializer::deserializeMap("{\"schemaVersion\": 1, \"id\": \"Malo\"}");

    MOTOR_CHECK(!invalidProject.isOk());
    MOTOR_CHECK(invalidProject.errorMessage().find("incompatible") != std::string::npos);
    MOTOR_CHECK(!invalidMap.isOk());
    MOTOR_CHECK(!invalidMap.errorMessage().empty());
}

MOTOR_TEST_CASE(atomic_save_does_not_replace_an_original_when_validation_fails) {
    const std::string path = "project_serializer_atomic_test.json";
    std::remove(path.c_str());
    std::remove((path + ".tmp").c_str());
    {
        std::ofstream original(path, std::ios::binary);
        original << "original intact";
    }

    Editor::Project invalid = makeProject();
    invalid.id = "ID inválido";
    MOTOR_CHECK(!ProjectSerializer::saveProjectAtomic(path, invalid).isOk());

    std::ifstream verify(path, std::ios::binary);
    std::string actual((std::istreambuf_iterator<char>(verify)), std::istreambuf_iterator<char>());
    MOTOR_CHECK(actual == "original intact");

    MOTOR_CHECK(ProjectSerializer::saveMapAtomic(path, makeMap()).isOk());
    MOTOR_CHECK(!std::ifstream(path + ".tmp", std::ios::binary).good());
    MOTOR_CHECK(ProjectSerializer::loadMapFile(path).isOk());
    std::remove(path.c_str());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
