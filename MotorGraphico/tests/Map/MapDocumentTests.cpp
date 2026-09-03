#include <vector>
#include <stdexcept>

#include "Map/MapDocument.h"
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

MapDocument makeValidMap() {
    return MapDocument(id<ids::MapId>("pueblo_inicio"), "Pueblo inicial", 4, 3, 64, 32);
}

MOTOR_TEST_CASE(map_accepts_typed_layers_without_runtime_tilemap) {
    MapDocument map = makeValidMap();

    MOTOR_CHECK(map.addLayer(MapLayer::tiles(
                    {id<ids::LayerId>("suelo"), "Suelo", true, false, 1.0f, 10},
                    TileLayerDocument(4, 3, std::vector<int>(12, 1))))
                    .isOk());
    MOTOR_CHECK(map.addLayer(MapLayer::entities(
                    {id<ids::LayerId>("entidades"), "Entidades", true, false, 0.85f, 20},
                    EntityLayerDocument(
                        {{id<ids::EntityId>("alcaldesa"), id<ids::AssetId>("npc_alcaldesa"),
                          {1, 2}}})))
                    .isOk());
    MOTOR_CHECK(map.addLayer(MapLayer::regions(
                    {id<ids::LayerId>("zonas"), "Zonas", false, true, 1.0f, 30},
                    RegionLayerDocument(
                        {{id<ids::RegionId>("plaza"), "Plaza", {0, 2, 0, 1}}})))
                    .isOk());

    MOTOR_CHECK(map.validate().isOk());
    MOTOR_CHECK(map.layers().size() == 3);
    MOTOR_CHECK(map.layers()[0].type() == MapLayerType::Tile);
    MOTOR_CHECK(map.layers()[1].type() == MapLayerType::Entity);
    MOTOR_CHECK(map.layers()[2].type() == MapLayerType::Region);
    MOTOR_CHECK(map.layers()[1].meta().opacity == 0.85f);
}

MOTOR_TEST_CASE(map_rejects_duplicate_layer_ids_and_orders) {
    MapDocument map = makeValidMap();
    const TileLayerDocument tiles(4, 3, std::vector<int>(12, 0));

    MOTOR_CHECK(map.addLayer(MapLayer::tiles(
                    {id<ids::LayerId>("suelo"), "Suelo", true, false, 1.0f, 10}, tiles))
                    .isOk());
    MOTOR_CHECK(!map.addLayer(
                        MapLayer::entities(
                            {id<ids::LayerId>("suelo"), "Objetos", true, false, 1.0f, 20}, {}))
                        .isOk());
    MOTOR_CHECK(!map.addLayer(
                        MapLayer::regions(
                            {id<ids::LayerId>("zonas"), "Zonas", true, false, 1.0f, 10}, {}))
                        .isOk());
    MOTOR_CHECK(map.layers().size() == 1);
}

MOTOR_TEST_CASE(map_rejects_layers_with_incoherent_dimensions_or_bounds) {
    MapDocument map = makeValidMap();

    MOTOR_CHECK(!map.addLayer(MapLayer::tiles(
                        {id<ids::LayerId>("suelo"), "Suelo", true, false, 1.0f, 10},
                        TileLayerDocument(3, 3, std::vector<int>(9, 1))))
                        .isOk());
    MOTOR_CHECK(!map.addLayer(MapLayer::entities(
                        {id<ids::LayerId>("entidades"), "Entidades", true, false, 1.0f, 20},
                        EntityLayerDocument(
                            {{id<ids::EntityId>("guardia"), id<ids::AssetId>("npc_guardia"),
                              {4, 1}}})))
                        .isOk());
    MOTOR_CHECK(!map.addLayer(MapLayer::regions(
                        {id<ids::LayerId>("zonas"), "Zonas", true, false, 1.0f, 30},
                        RegionLayerDocument(
                            {{id<ids::RegionId>("salida"), "Salida", {3, 4, 0, 0}}})))
                        .isOk());
    MOTOR_CHECK(map.layers().empty());
}

// =====================================================================
// C-12 — El documento se puede EDITAR.
//
// Antes solo tenia addLayer: ni borrar una capa, ni reordenarlas, ni
// tocar una celda, ni mover una entidad. Migrar el editor a este
// documento era imposible por construccion.
//
// Lo que se prueba: (1) que cada mutacion mantiene las invariantes que
// antes solo comprobaba validate(); (2) que un documento construido a
// base de mutaciones acaba EXACTAMENTE igual que el mismo documento
// construido de una pieza -- si no, la API de edicion y la de carga
// estarian produciendo cosas distintas; (3) que una mutacion invalida no
// deja el documento a medias.
// =====================================================================

// Comparacion campo a campo: MapDocument no tiene operator== y no se le
// anade solo para el test (seria API publica nueva en fase C).
bool sameTiles(const TileLayerDocument& a, const TileLayerDocument& b) {
    return a.width() == b.width() && a.height() == b.height() && a.gids() == b.gids();
}

bool sameEntities(const EntityLayerDocument& a, const EntityLayerDocument& b) {
    if (a.entities().size() != b.entities().size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.entities().size(); ++i) {
        const MapEntityDocument& x = a.entities()[i];
        const MapEntityDocument& y = b.entities()[i];
        if (!(x.id == y.id) || !(x.objectId == y.objectId) || x.position.x != y.position.x ||
            x.position.y != y.position.y) {
            return false;
        }
    }
    return true;
}

bool sameRegions(const RegionLayerDocument& a, const RegionLayerDocument& b) {
    if (a.regions().size() != b.regions().size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.regions().size(); ++i) {
        const MapRegionDocument& x = a.regions()[i];
        const MapRegionDocument& y = b.regions()[i];
        if (!(x.id == y.id) || x.name != y.name || x.bounds.minX != y.bounds.minX ||
            x.bounds.maxX != y.bounds.maxX || x.bounds.minY != y.bounds.minY ||
            x.bounds.maxY != y.bounds.maxY) {
            return false;
        }
    }
    return true;
}

bool sameMeta(const MapLayerMeta& a, const MapLayerMeta& b) {
    return a.id == b.id && a.name == b.name && a.visible == b.visible && a.locked == b.locked &&
           a.opacity == b.opacity && a.order == b.order;
}

bool sameDocument(const MapDocument& a, const MapDocument& b) {
    if (!(a.id() == b.id()) || a.name() != b.name() || a.width() != b.width() ||
        a.height() != b.height() || a.tileWidth() != b.tileWidth() ||
        a.tileHeight() != b.tileHeight() || a.collisionGids() != b.collisionGids()) {
        return false;
    }
    if (a.layers().size() != b.layers().size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.layers().size(); ++i) {
        const MapLayer& x = a.layers()[i];
        const MapLayer& y = b.layers()[i];
        if (!sameMeta(x.meta(), y.meta()) || x.type() != y.type()) {
            return false;
        }
        switch (x.type()) {
            case MapLayerType::Tile:
                if (!sameTiles(std::get<TileLayerDocument>(x.data()),
                               std::get<TileLayerDocument>(y.data()))) {
                    return false;
                }
                break;
            case MapLayerType::Entity:
                if (!sameEntities(std::get<EntityLayerDocument>(x.data()),
                                  std::get<EntityLayerDocument>(y.data()))) {
                    return false;
                }
                break;
            case MapLayerType::Region:
                if (!sameRegions(std::get<RegionLayerDocument>(x.data()),
                                 std::get<RegionLayerDocument>(y.data()))) {
                    return false;
                }
                break;
        }
    }
    return true;
}

// El documento al que llega la secuencia de 20 mutaciones, construido de
// una sola pieza. Es el oraculo del test.
MapDocument makeExpectedDocument() {
    MapDocument map(id<ids::MapId>("pueblo_inicio"), "Pueblo inicial", 4, 3, 64, 32);

    // Suelo: base de 1, la fila 0 entera a 7 y (3,2) a 9.
    std::vector<int> gids(12, 1);
    gids[0] = 7;
    gids[1] = 7;
    gids[2] = 7;
    gids[3] = 7;
    gids[11] = 9;
    map.addLayer(MapLayer::tiles({id<ids::LayerId>("suelo"), "Suelo pintado", true, false, 0.5f, 5},
                                 TileLayerDocument(4, 3, gids)));

    map.addLayer(MapLayer::entities(
        {id<ids::LayerId>("entidades"), "Entidades", true, false, 1.0f, 20},
        EntityLayerDocument({{id<ids::EntityId>("alcaldesa"), id<ids::AssetId>("npc_alcaldesa"),
                              {3, 0}},
                             {id<ids::EntityId>("herrero"), id<ids::AssetId>("npc_herrero"),
                              {1, 1}}})));

    map.addLayer(MapLayer::regions(
        {id<ids::LayerId>("zonas"), "Zonas", false, true, 1.0f, 30},
        RegionLayerDocument({{id<ids::RegionId>("plaza"), "Plaza", {0, 3, 0, 0}}})));

    map.setCollisionGids({7, 9});
    return map;
}

MOTOR_TEST_CASE(twenty_mutations_leave_the_same_document_as_building_it_at_once) {
    MapDocument map = makeValidMap();
    int mutations = 0;
    // Cada mutacion deja el documento valido: es la invariante del
    // paquete, y por eso se comprueba DESPUES DE CADA UNA y no solo al
    // final.
    const auto step = [&](const Result<bool>& result) {
        MOTOR_CHECK(result.isOk());
        MOTOR_CHECK(map.validate().isOk());
        ++mutations;
    };

    // 1-3: las tres capas de partida.
    step(map.addLayer(MapLayer::tiles({id<ids::LayerId>("suelo"), "Suelo", true, false, 1.0f, 10},
                                      TileLayerDocument(4, 3, std::vector<int>(12, 1)))));
    step(map.addLayer(MapLayer::entities(
        {id<ids::LayerId>("entidades"), "Entidades", true, false, 1.0f, 20},
        EntityLayerDocument(std::vector<MapEntityDocument>{}))));
    step(map.addLayer(MapLayer::regions(
        {id<ids::LayerId>("zonas"), "Zonas", true, false, 1.0f, 30}, RegionLayerDocument(std::vector<MapRegionDocument>{}))));

    // 4-5: una capa de mas, y fuera. Borrar capas no existia.
    step(map.addLayer(MapLayer::tiles({id<ids::LayerId>("techos"), "Techos", true, false, 1.0f, 40},
                                      TileLayerDocument(4, 3, std::vector<int>(12, 3)))));
    step(map.removeLayer(id<ids::LayerId>("techos")));

    // 6-9: pintar. Una celda suelta y una fila entera.
    step(map.setTile(id<ids::LayerId>("suelo"), GridCoord{3, 2}, 9));
    step(map.fillTiles(id<ids::LayerId>("suelo"), GridBounds{0, 3, 0, 0}, 7));
    step(map.setTile(id<ids::LayerId>("suelo"), GridCoord{0, 1}, 4));
    step(map.setTile(id<ids::LayerId>("suelo"), GridCoord{0, 1}, 1));  // deshecho a mano

    // 10-14: entidades. Anadir tres, quitar una y mover otra.
    step(map.addEntity(id<ids::LayerId>("entidades"),
                       {id<ids::EntityId>("alcaldesa"), id<ids::AssetId>("npc_alcaldesa"),
                        {1, 2}}));
    step(map.addEntity(id<ids::LayerId>("entidades"),
                       {id<ids::EntityId>("perro"), id<ids::AssetId>("npc_perro"), {2, 2}}));
    step(map.addEntity(id<ids::LayerId>("entidades"),
                       {id<ids::EntityId>("herrero"), id<ids::AssetId>("npc_herrero"), {1, 1}}));
    step(map.removeEntity(id<ids::LayerId>("entidades"), id<ids::EntityId>("perro")));
    step(map.moveEntity(id<ids::LayerId>("entidades"), id<ids::EntityId>("alcaldesa"),
                        GridCoord{3, 0}));

    // 15-17: regiones. Anadir dos, quitar una y recortar la que queda.
    step(map.addRegion(id<ids::LayerId>("zonas"), {id<ids::RegionId>("plaza"), "Plaza",
                                                   {0, 2, 0, 1}}));
    step(map.addRegion(id<ids::LayerId>("zonas"), {id<ids::RegionId>("pozo"), "Pozo",
                                                   {2, 3, 2, 2}}));
    step(map.removeRegion(id<ids::LayerId>("zonas"), id<ids::RegionId>("pozo")));
    step(map.setRegionBounds(id<ids::LayerId>("zonas"), id<ids::RegionId>("plaza"),
                             GridBounds{0, 3, 0, 0}));

    // 19: metadatos de capa (nombre, opacidad y orden a la vez); el orden
    // nuevo la lleva a la primera posicion, que ya ocupaba.
    step(map.setLayerMeta(id<ids::LayerId>("suelo"),
                          {id<ids::LayerId>("suelo"), "Suelo pintado", true, false, 0.5f, 5}));

    // 20: la capa de zonas, oculta y bloqueada.
    step(map.setLayerMeta(id<ids::LayerId>("zonas"),
                          {id<ids::LayerId>("zonas"), "Zonas", false, true, 1.0f, 30}));

    // 21: GIDs de colision (ya existia, pero cierra el documento).
    step(map.setCollisionGids({9, 7}));

    // 21 mutaciones (el pla pedia veinte): cada una validada al
    // aplicarse, y el resultado identico al documento de una pieza.
    MOTOR_CHECK(mutations == 21);
    MOTOR_CHECK(sameDocument(map, makeExpectedDocument()));
}

// Reordenar de verdad: mover una capa por encima de otra cambia el orden
// del vector, y validate() lo sigue aceptando (exige que este ordenado).
MOTOR_TEST_CASE(moving_a_layer_reorders_the_document) {
    MapDocument map = makeValidMap();
    MOTOR_CHECK(map.addLayer(MapLayer::tiles(
                    {id<ids::LayerId>("suelo"), "Suelo", true, false, 1.0f, 10},
                    TileLayerDocument(4, 3, std::vector<int>(12, 1)))).isOk());
    MOTOR_CHECK(map.addLayer(MapLayer::tiles(
                    {id<ids::LayerId>("techos"), "Techos", true, false, 1.0f, 20},
                    TileLayerDocument(4, 3, std::vector<int>(12, 2)))).isOk());

    MOTOR_CHECK(map.layers()[0].meta().id == id<ids::LayerId>("suelo"));
    MOTOR_CHECK(map.moveLayer(id<ids::LayerId>("techos"), 5).isOk());
    MOTOR_CHECK(map.validate().isOk());
    MOTOR_CHECK(map.layers()[0].meta().id == id<ids::LayerId>("techos"));
    MOTOR_CHECK(map.layers()[1].meta().id == id<ids::LayerId>("suelo"));

    // Y no se puede aterrizar sobre el orden de otra capa.
    MOTOR_CHECK(!map.moveLayer(id<ids::LayerId>("techos"), 10).isOk());
    MOTOR_CHECK(map.layers()[0].meta().id == id<ids::LayerId>("techos"));  // sin efecto
}

// Todo-o-nada: una mutacion invalida devuelve el error y deja el
// documento EXACTAMENTE como estaba. Es lo que separa una API de edicion
// de un puñado de setters.
MOTOR_TEST_CASE(a_rejected_mutation_leaves_the_document_untouched) {
    MapDocument map = makeValidMap();
    MOTOR_CHECK(map.addLayer(MapLayer::tiles(
                    {id<ids::LayerId>("suelo"), "Suelo", true, false, 1.0f, 10},
                    TileLayerDocument(4, 3, std::vector<int>(12, 1)))).isOk());
    MOTOR_CHECK(map.addLayer(MapLayer::entities(
                    {id<ids::LayerId>("entidades"), "Entidades", true, false, 1.0f, 20},
                    EntityLayerDocument({{id<ids::EntityId>("alcaldesa"),
                                          id<ids::AssetId>("npc_alcaldesa"), {1, 1}}}))).isOk());

    const MapDocument before = map;

    // Celda fuera de la cuadricula.
    MOTOR_CHECK(!map.setTile(id<ids::LayerId>("suelo"), GridCoord{9, 9}, 2).isOk());
    // GID negativo.
    MOTOR_CHECK(!map.setTile(id<ids::LayerId>("suelo"), GridCoord{0, 0}, -1).isOk());
    // Relleno que se sale a medias: ni una sola celda debe quedar pintada.
    MOTOR_CHECK(!map.fillTiles(id<ids::LayerId>("suelo"), GridBounds{2, 9, 0, 1}, 5).isOk());
    // Entidad fuera de los limites del MAPA (esto solo lo sabe el documento).
    MOTOR_CHECK(!map.addEntity(id<ids::LayerId>("entidades"),
                               {id<ids::EntityId>("fuera"), id<ids::AssetId>("npc"), {4, 0}})
                     .isOk());
    // Mover una entidad existente fuera del mapa.
    MOTOR_CHECK(!map.moveEntity(id<ids::LayerId>("entidades"), id<ids::EntityId>("alcaldesa"),
                                GridCoord{-1, 0})
                     .isOk());
    // Id de entidad duplicado.
    MOTOR_CHECK(!map.addEntity(id<ids::LayerId>("entidades"),
                               {id<ids::EntityId>("alcaldesa"), id<ids::AssetId>("otro"), {0, 0}})
                     .isOk());
    // Tipo de capa equivocado.
    MOTOR_CHECK(!map.setTile(id<ids::LayerId>("entidades"), GridCoord{0, 0}, 1).isOk());
    // Capa inexistente.
    MOTOR_CHECK(!map.removeLayer(id<ids::LayerId>("no_existe")).isOk());
    MOTOR_CHECK(!map.removeEntity(id<ids::LayerId>("entidades"), id<ids::EntityId>("nadie"))
                     .isOk());

    MOTOR_CHECK(sameDocument(map, before));
    MOTOR_CHECK(map.validate().isOk());
}

// findLayer: consulta por id, para no recorrer layers() a mano en cada
// llamador (el editor lo hara en cada clic).
MOTOR_TEST_CASE(find_layer_returns_the_layer_or_null) {
    MapDocument map = makeValidMap();
    MOTOR_CHECK(map.addLayer(MapLayer::tiles(
                    {id<ids::LayerId>("suelo"), "Suelo", true, false, 1.0f, 10},
                    TileLayerDocument(4, 3, std::vector<int>(12, 1)))).isOk());

    const MapLayer* found = map.findLayer(id<ids::LayerId>("suelo"));
    MOTOR_CHECK(found != nullptr);
    MOTOR_CHECK(found->meta().name == "Suelo");
    MOTOR_CHECK(map.findLayer(id<ids::LayerId>("no_existe")) == nullptr);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
