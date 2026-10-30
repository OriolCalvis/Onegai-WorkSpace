#include <string>
#include <stdexcept>
#include <type_traits>

#include "Core/Ids/ProjectIds.h"
#include "Project/DocumentRef.h"
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

class FixtureIdGenerator final : public ids::IdGenerator {
public:
    std::string next(std::string_view idNamespace) override {
        return std::string(idNamespace) + "_fixture_" + std::to_string(m_next++);
    }

private:
    int m_next = 1;
};

MOTOR_TEST_CASE(strong_ids_validate_contract_and_do_not_cross_assign) {
    static_assert(!std::is_convertible<ids::AssetId, ids::MapId>::value,
                  "AssetId no debe convertirse implícitamente a MapId");

    const Result<ids::MapId> valid = ids::MapId::parse("ciudad_centro");
    MOTOR_CHECK(valid.isOk());
    MOTOR_CHECK(valid.value() == id<ids::MapId>("ciudad_centro"));
    MOTOR_CHECK(!ids::MapId::parse("Ciudad Centro").isOk());
}

MOTOR_TEST_CASE(injected_generator_is_deterministic_and_keeps_namespaces) {
    FixtureIdGenerator generator;
    const Result<ids::MapId> map = ids::generate<ids::MapId>(generator);
    const Result<ids::AssetId> asset = ids::generate<ids::AssetId>(generator);

    MOTOR_CHECK(map.isOk());
    MOTOR_CHECK(asset.isOk());
    MOTOR_CHECK(map.value().value() == "map_fixture_1");
    MOTOR_CHECK(asset.value().value() == "asset_fixture_2");
}

MOTOR_TEST_CASE(document_reference_survives_a_logical_path_move) {
    DocumentRef reference{id<ids::DocumentId>("inicio_mapa"), DocumentKind::Map,
                          "maps/pueblo_inicio.map.json"};
    MOTOR_CHECK(reference.validate().isOk());

    const ids::DocumentId stableId = reference.id;
    reference.logicalPath = "maps/campania_1/pueblo_inicio.map.json";

    MOTOR_CHECK(reference.validate().isOk());
    MOTOR_CHECK(reference.id == stableId);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
