#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Ids/ProjectIds.h"
#include "Level/LevelDefinition.h"
#include "Map/EntityLayerDocument.h"
#include "Map/MapDocument.h"

// C-14 (primera rebanada) — El puente entre las dos representaciones de
// una entidad colocada en un mapa.
//
// POR QUE EXISTE. Tras C-13, MapEntityDocument y ObjectSpawn guardan
// exactamente los mismos datos de autoría. Eso es una promesa sobre el
// papel hasta que hay una función que los convierta y una prueba que la
// recorra: sin ella, "paridad" solo significa que las dos listas de
// campos se parecen, y nada impide que mañana se separen en silencio.
//
// Es además la pieza que C-14 necesita para que EditorState pueda pasar a
// ser una vista sobre un MapDocument sin perder por el camino el trabajo
// de autoría de los niveles ya escritos.
//
// DONDE VIVE Y POR QUE. En Level/ y no en Map/: el documento de mapa no
// debe saber que existe LevelDefinition. motor_level ya depende de
// motor_map_document, así que la flecha va en la dirección que ya
// existía; ponerlo del otro lado la invertiría.
//
// QUE NO CUBRE (y no es un olvido):
//  - LevelDefinition::enemies (EnemySpawn). Es el formato de la Fase 6
//    que ObjectSpawn ya generaliza; migrarlo es contenido, no motor.
//  - name, mapPath y playerStart del nivel. No tienen sitio en un
//    MapDocument todavía; dónde viven es una decisión de C-14, y
//    adelantarla aquí sería inventar formato.

namespace level_document {

// Id de entidad derivado de forma ESTABLE del spawn: mismo nivel, mismos
// ids, ejecución tras ejecución y máquina tras máquina. Se construye como
// "<objectId>_<x>_<y>" porque en un nivel la identidad de un objeto ES su
// celda (la misma regla que ya usa GameSession::worldObjectIndexAt y el
// guardado de partida, ver Game/SaveGame.h).
//
// Si dos spawns comparten celda -- el formato lo permite aunque el juego
// no lo espere -- el segundo recibe un sufijo "_2", "_3"... en vez de
// pisar al primero: perder una entidad al migrar es exactamente lo que
// este puente existe para evitar.
Result<ids::EntityId> MakeEntityId(const std::string& objectId, GridCoord position,
                                   int disambiguator = 1);

// ObjectSpawn -> MapEntityDocument. Falla si el objectId no es un
// identificador válido ([a-z0-9_]+, ver Core/Ids/StrongId.h): NO se
// "arregla" por las bravas, porque un id mutilado en silencio es un
// objeto que ya no resuelve contra el catálogo y un nivel que se rompe
// más tarde y en otro sitio.
Result<MapEntityDocument> ToEntityDocument(const ObjectSpawn& spawn, ids::EntityId entityId);

// MapEntityDocument -> ObjectSpawn. No puede fallar: todo lo que el
// documento guarda cabe en el spawn.
ObjectSpawn ToObjectSpawn(const MapEntityDocument& entity);

// Los "objects" de un nivel entero como capa de entidades, con los ids
// generados y desambiguados. Todo-o-nada: si un solo spawn no se puede
// convertir, no se devuelve una capa a medias.
Result<EntityLayerDocument> ToEntityLayer(const std::vector<ObjectSpawn>& spawns);

// Y la vuelta: todas las entidades de todas las capas de entidades del
// documento, en orden de capa y dentro de cada capa en su propio orden.
std::vector<ObjectSpawn> ToObjectSpawns(const MapDocument& document);

}  // namespace level_document
