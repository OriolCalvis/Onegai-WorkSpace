// C-14 (primera rebanada) — El puente ObjectSpawn <-> MapEntityDocument.
//
// C-13 dejó las dos fichas con los mismos campos. Esto comprueba que esa
// paridad es REAL: un spawn con los once campos de autoría poblados
// atraviesa el puente en las dos direcciones y vuelve idéntico. Sin esta
// prueba, "paridad" solo quiere decir que las dos listas de campos se
// parecen, y nada impide que mañana se separen en silencio -- que es
// justo la clase de fallo que no rompe nada hasta que un nivel migrado
// llega al jugador con las puertas muertas.
//
// Unit y sin GL ni assets: todo son datos.

#include "TestHarness.h"

#include "Level/LevelDocumentBridge.h"

#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

template <class Id>
Id id(const char* value) {
    const Result<Id> result = Id::parse(value);
    if (!result.isOk()) {
        throw std::runtime_error(result.errorMessage());
    }
    return result.value();
}

bool sameCoord(GridCoord a, GridCoord b) { return a.x == b.x && a.y == b.y; }

// Un spawn con TODO puesto: ningún campo en su valor por defecto.
ObjectSpawn makeFullSpawn() {
    ObjectSpawn spawn;
    spawn.objectId = "puerta";
    spawn.position = GridCoord{1, 1};
    spawn.patrolMin = GridCoord{0, 0};
    spawn.patrolMax = GridCoord{3, 2};
    spawn.targetLevel = "assets/levels/interior_posada.json";
    spawn.targetPosition = GridCoord{7, 9};
    spawn.hasTargetPosition = true;
    spawn.loot = {"llave_oxidada", "pocion"};
    spawn.lootTable = "cofres_comunes";
    spawn.eventId = "bnd_puerta_posada";
    spawn.displayName = "Posada del Cuervo";
    spawn.variant = "madera_vieja";
    spawn.scale = 1.75f;
    spawn.effectOverride = "abrir_con_llave";
    spawn.properties = {{"cerrada", "true"}, {"llave", "llave_oxidada"}};
    return spawn;
}

void checkSameSpawn(const ObjectSpawn& actual, const ObjectSpawn& expected) {
    MOTOR_CHECK(actual.objectId == expected.objectId);
    MOTOR_CHECK(sameCoord(actual.position, expected.position));
    MOTOR_CHECK(sameCoord(actual.patrolMin, expected.patrolMin));
    MOTOR_CHECK(sameCoord(actual.patrolMax, expected.patrolMax));
    MOTOR_CHECK(actual.targetLevel == expected.targetLevel);
    MOTOR_CHECK(actual.hasTargetPosition == expected.hasTargetPosition);
    if (expected.hasTargetPosition) {
        MOTOR_CHECK(sameCoord(actual.targetPosition, expected.targetPosition));
    }
    MOTOR_CHECK(actual.loot == expected.loot);
    MOTOR_CHECK(actual.lootTable == expected.lootTable);
    MOTOR_CHECK(actual.eventId == expected.eventId);
    MOTOR_CHECK(actual.displayName == expected.displayName);
    MOTOR_CHECK(actual.variant == expected.variant);
    MOTOR_CHECK(actual.scale == expected.scale);
    MOTOR_CHECK(actual.effectOverride == expected.effectOverride);
    MOTOR_CHECK(actual.properties == expected.properties);
}

}  // namespace

// El caso central: los once campos de autoría van y vuelven idénticos.
MOTOR_TEST_CASE(the_eleven_authoring_fields_survive_the_round_trip) {
    const ObjectSpawn original = makeFullSpawn();

    const Result<ids::EntityId> entityId =
        level_document::MakeEntityId(original.objectId, original.position);
    MOTOR_CHECK(entityId.isOk());

    const Result<MapEntityDocument> entity =
        level_document::ToEntityDocument(original, entityId.value());
    MOTOR_CHECK(entity.isOk());

    checkSameSpawn(level_document::ToObjectSpawn(entity.value()), original);
}

// El id es estable y derivado de la celda: mismo nivel, mismos ids,
// ejecución tras ejecución. Sin esto, migrar dos veces el mismo nivel
// daría dos documentos distintos y ningún diff sería legible.
MOTOR_TEST_CASE(entity_ids_are_stable_and_derived_from_the_cell) {
    const Result<ids::EntityId> first = level_document::MakeEntityId("cofre", GridCoord{3, 7});
    const Result<ids::EntityId> again = level_document::MakeEntityId("cofre", GridCoord{3, 7});
    MOTOR_CHECK(first.isOk() && again.isOk());
    MOTOR_CHECK(first.value() == again.value());
    MOTOR_CHECK(first.value() == id<ids::EntityId>("cofre_3_7"));

    // Otra celda, otro id.
    const Result<ids::EntityId> other = level_document::MakeEntityId("cofre", GridCoord{3, 8});
    MOTOR_CHECK(other.isOk());
    MOTOR_CHECK(!(other.value() == first.value()));
}

// Una capa entera desde los "objects" de un nivel, con la desambiguación
// de celda compartida: el formato lo permite y perder el segundo objeto
// al migrar es justo lo que este puente evita.
MOTOR_TEST_CASE(a_whole_level_becomes_a_layer_without_losing_anyone) {
    std::vector<ObjectSpawn> spawns;
    spawns.push_back(makeFullSpawn());

    ObjectSpawn arbusto;
    arbusto.objectId = "arbusto";
    arbusto.position = GridCoord{2, 2};
    arbusto.patrolMin = arbusto.position;
    arbusto.patrolMax = arbusto.position;
    spawns.push_back(arbusto);

    ObjectSpawn compartida = arbusto;  // MISMA celda que el anterior
    compartida.displayName = "El otro arbusto";
    spawns.push_back(compartida);

    const Result<EntityLayerDocument> layer = level_document::ToEntityLayer(spawns);
    MOTOR_CHECK(layer.isOk());
    MOTOR_CHECK(layer.value().entities().size() == 3);
    MOTOR_CHECK(layer.value().entities()[0].id == id<ids::EntityId>("puerta_1_1"));
    MOTOR_CHECK(layer.value().entities()[1].id == id<ids::EntityId>("arbusto_2_2"));
    MOTOR_CHECK(layer.value().entities()[2].id == id<ids::EntityId>("arbusto_2_2_2"));
    MOTOR_CHECK(layer.value().validate().isOk());  // ids únicos de verdad

    // Y la vuelta desde un documento completo.
    MapDocument document(id<ids::MapId>("pueblo"), "Pueblo", 4, 3, 64, 32);
    MOTOR_CHECK(document.addLayer(MapLayer::entities(
                                      {id<ids::LayerId>("entidades"), "Entidades", true, false,
                                       1.0f, 20},
                                      layer.value()))
                    .isOk());
    MOTOR_CHECK(document.validate().isOk());

    const std::vector<ObjectSpawn> back = level_document::ToObjectSpawns(document);
    MOTOR_CHECK(back.size() == 3);
    checkSameSpawn(back[0], spawns[0]);
    checkSameSpawn(back[1], spawns[1]);
    checkSameSpawn(back[2], spawns[2]);
}

// Un objectId que no es un identificador válido NO se arregla por las
// bravas: un id mutilado en silencio es un objeto que deja de resolver
// contra el catálogo y un nivel que se rompe más tarde y en otro sitio.
MOTOR_TEST_CASE(an_invalid_object_id_fails_loudly_and_leaves_nothing_half_built) {
    ObjectSpawn roto;
    roto.objectId = "Puerta Grande";  // mayúsculas y espacio
    roto.position = GridCoord{0, 0};

    MOTOR_CHECK(!level_document::MakeEntityId(roto.objectId, roto.position).isOk());

    std::vector<ObjectSpawn> spawns;
    spawns.push_back(makeFullSpawn());
    spawns.push_back(roto);
    const Result<EntityLayerDocument> layer = level_document::ToEntityLayer(spawns);
    MOTOR_CHECK(!layer.isOk());
    MOTOR_CHECK(!layer.errorMessage().empty());
}

// Celda negativa: no es celda de ningún mapa y produciría un id que ni
// siquiera es válido. Se rechaza aquí, con el mensaje que dice por qué.
MOTOR_TEST_CASE(a_negative_cell_is_rejected_with_its_own_message) {
    const Result<ids::EntityId> negative =
        level_document::MakeEntityId("cofre", GridCoord{-1, 4});
    MOTOR_CHECK(!negative.isOk());
    MOTOR_CHECK(negative.errorMessage().find("negativa") != std::string::npos);
}

// Un documento sin capas de entidades devuelve una lista vacía, no un
// fallo: un mapa de puro terreno es legítimo.
MOTOR_TEST_CASE(a_document_without_entity_layers_yields_no_spawns) {
    MapDocument document(id<ids::MapId>("pradera"), "Pradera", 2, 2, 64, 32);
    MOTOR_CHECK(document.addLayer(MapLayer::tiles(
                                      {id<ids::LayerId>("suelo"), "Suelo", true, false, 1.0f, 10},
                                      TileLayerDocument(2, 2, {1, 1, 1, 1})))
                    .isOk());
    MOTOR_CHECK(level_document::ToObjectSpawns(document).empty());
}

int main() { return motor_test::runAllTests(); }
