#include <string>
#include <stdexcept>

#include "Import/TmxMapImporter.h"
#include "TestHarness.h"

namespace {

ids::MapId mapId(const char* value) {
    const Result<ids::MapId> parsed = ids::MapId::parse(value);
    if (!parsed.isOk()) {
        throw std::runtime_error(parsed.errorMessage());
    }
    return parsed.value();
}

MOTOR_TEST_CASE(importer_creates_a_valid_map_document_from_the_existing_tmx_fixture) {
    TmxMapImporter importer;
    const TmxMapImportReport report = importer.importFile("assets/maps/test_map.tmx", mapId("test_map"));

    MOTOR_CHECK(!report.hasErrors());
    MOTOR_CHECK(report.document.validate().isOk());
    MOTOR_CHECK(report.document.width() == 4);
    MOTOR_CHECK(report.document.height() == 3);
    MOTOR_CHECK(report.document.layers().size() == 1);
    MOTOR_CHECK(report.document.layers()[0].meta().id == ids::LayerId::parse("suelo").value());
    MOTOR_CHECK(report.document.collisionGids().size() == 1);
    MOTOR_CHECK(report.document.collisionGids()[0] == 2);
}

MOTOR_TEST_CASE(importer_reports_invalid_csv_without_constructing_a_document) {
    TmxMapImporter importer;
    const TmxMapImportReport report =
        importer.importFile("assets/maps/test_map_invalid.tmx", mapId("test_map_invalid"));

    MOTOR_CHECK(report.hasErrors());
    MOTOR_CHECK(!report.document.validate().isOk());
    MOTOR_CHECK(!report.issues.empty());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
