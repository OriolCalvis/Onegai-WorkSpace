#include "Project/Project.h"

#include "TestHarness.h"

namespace {

MOTOR_TEST_CASE(portable_project_with_logical_paths_is_valid) {
    Editor::Project project;
    project.id = "demo_project";
    project.name = "Proyecto de prueba";
    project.prefix = "demo_";
    project.entry = "assets/adventures/inicio.json";
    project.maps = {"inicio.tmx"};
    project.levels = {"inicio.json"};
    project.adventures = {"inicio.json"};
    project.catalogs = {"objetos.json"};
    project.settings.initialMap = "inicio.tmx";

    MOTOR_CHECK(project.validate().isOk());
}

MOTOR_TEST_CASE(project_rejects_invalid_identity_and_external_paths) {
    Editor::Project project;
    project.id = "Proyecto Invalido";
    project.name = "Proyecto";

    MOTOR_CHECK(!project.validate().isOk());

    project.id = "proyecto_valido";
    project.entry = "/Users/externo/nivel.json";

    MOTOR_CHECK(!project.validate().isOk());
}

MOTOR_TEST_CASE(settings_reject_unknown_schema_and_duplicate_documents) {
    Editor::Project project;
    project.id = "proyecto_valido";
    project.name = "Proyecto";
    project.settings.schemaVersion = 2;

    MOTOR_CHECK(!project.validate().isOk());

    project.settings.schemaVersion = Editor::ProjectSettings::kCurrentSchemaVersion;
    project.levels = {"nivel.json", "nivel.json"};

    MOTOR_CHECK(!project.validate().isOk());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
