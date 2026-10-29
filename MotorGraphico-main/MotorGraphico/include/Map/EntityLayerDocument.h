// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-13  (fase C, 3 j) - Les entitats del document no perden res
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <map>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Ids/ProjectIds.h"
#include "Core/Math/GridCoord.h"

// Instancia de una entidad en un mapa. objectId se resolverá contra un
// catálogo en una fase posterior; este documento solo conserva su referencia
// estable y su posición, sin actores de runtime ni recursos gráficos.
//
// C-13 — PARIDAD CON ObjectSpawn (include/Level/LevelDefinition.h).
//
// Hasta aquí esta ficha guardaba id, objectId y posición. ObjectSpawn --
// lo que el juego carga HOY de los niveles reales -- guarda además la
// patrulla, el destino de la puerta, el botín, la tabla de botín, el
// evento, el nombre visible, la variante, la escala, el override de
// efecto y un mapa de propiedades. Migrar un nivel a este documento
// habría perdido casi toda la obra de autoría, en silencio y sin que
// nada fallara: el nivel seguiría cargando, solo que las puertas no
// llevarían a ningún sitio y los cofres estarían vacíos.
//
// El mapeo es uno a uno, campo a campo:
//
//   ObjectSpawn::objectId        -> MapEntityDocument::objectId
//   ObjectSpawn::position        -> position
//   ObjectSpawn::patrolMin/Max   -> patrolMin/patrolMax
//   ObjectSpawn::targetLevel     -> targetLevel
//   ObjectSpawn::targetPosition  -> targetPosition (+ hasTargetPosition)
//   ObjectSpawn::loot            -> loot
//   ObjectSpawn::lootTable       -> lootTable
//   ObjectSpawn::eventId         -> eventId
//   ObjectSpawn::displayName     -> displayName
//   ObjectSpawn::variant         -> variant
//   ObjectSpawn::scale           -> scale
//   ObjectSpawn::effectOverride  -> effectOverride
//   ObjectSpawn::properties      -> properties
//
// Lo único que ObjectSpawn NO tiene es el id de instancia: en un nivel la
// identidad es la posición dentro del array, y por eso un enemigo
// invocado por narrativa se quedaba sin identidad (ver C-04). El
// documento le da uno estable desde el principio.
//
// Si algún día se añade un campo a ObjectSpawn, ESTA es la lista que hay
// que ampliar a la vez; el round-trip de test_map_round_trip los recorre
// todos y falla si uno se pierde por el camino.
struct MapEntityDocument {
    ids::EntityId id;
    ids::AssetId objectId;
    GridCoord position;

    // Patrulla. Igual que en ObjectSpawn, "sin patrulla" se expresa con
    // patrolMin == patrolMax == position; el deserializador rellena así
    // los que no traen el campo. Ambas celdas son de ESTE mapa, y el
    // documento comprueba que caen dentro.
    GridCoord patrolMin;
    GridCoord patrolMax;

    // Transición a otro nivel. El destino es de ESTA puerta, no del tipo
    // "puerta": dos posadas comparten definición y llevan a interiores
    // distintos. targetPosition NO se valida contra los límites de este
    // mapa -- es una celda del mapa de destino.
    std::string targetLevel;
    GridCoord targetPosition;
    bool hasTargetPosition = false;

    // Botín de la instancia: ids fijos del catálogo y/o una tabla del
    // catálogo RPG de botín. Pueden convivir.
    std::vector<std::string> loot;
    std::string lootTable;

    // Evento/misión que dispara esta instancia (un id que el motor
    // narrativo pueda escuchar). Vacío = no dispara nada.
    std::string eventId;

    // Variante local del arquetipo: lo que distingue ESTA instancia sin
    // duplicar la ficha del catálogo.
    std::string displayName;
    std::string variant;
    float scale = 1.0f;
    std::string effectOverride;
    std::map<std::string, std::string> properties;
};

class EntityLayerDocument {
public:
    EntityLayerDocument() = default;
    explicit EntityLayerDocument(std::vector<MapEntityDocument> entities);

    const std::vector<MapEntityDocument>& entities() const { return m_entities; }

    // --- C-12: mutacion (ver el comentario de TileLayerDocument) ---
    //
    // Los limites del MAPA no se comprueban aqui -- esta capa no sabe en
    // que mapa vive. De eso se encarga MapDocument, que revalida la capa
    // contra sus dimensiones despues de cada mutacion.
    const MapEntityDocument* find(const ids::EntityId& id) const;
    Result<bool> addEntity(MapEntityDocument entity);
    Result<bool> removeEntity(const ids::EntityId& id);
    Result<bool> setEntityPosition(const ids::EntityId& id, GridCoord position);

    Result<bool> validate() const;

private:
    std::vector<MapEntityDocument> m_entities;
};
