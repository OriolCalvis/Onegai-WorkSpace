#pragma once

#include <map>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"

// Definicion de datos de un nivel jugable (motor_grafico_gantt_rpg.puml,
// Fase 6 "Sistema de niveles y contenido"): lo que LevelLoader produce a
// partir de un JSON (ver assets/levels/test_level.json). Es SOLO DATOS --
// no construye TileMap ni Enemy/Player, ni sabe nada de OpenGL ni de
// SpriteBatch: esa instanciacion es responsabilidad de quien consuma
// LevelDefinition (la futura Application, o un demo -- ver
// examples/demo_level_loader.cpp para el ejemplo minimo). Mismo criterio
// de separacion que TileMap::loadFromFile() (parsea TMX a las estructuras
// propias de TileMap), pero un nivel arriba: LevelDefinition referencia
// un mapa TMX por RUTA (m_mapPath), no lo carga ella misma.
struct EnemySpawn {
    // "type" referencia una entrada de un catalogo de enemigos externo
    // (por ahora un id de texto libre, ej. "slime", "goblin"; el catalogo
    // real -- stats base, sprite, que habilidades usa por defecto -- es
    // contenido futuro, ver Fase 6/7 del Gantt) en vez de duplicar sus
    // stats en cada spawn: mismo principio que un TMX referenciar
    // tilesets por gid en vez de incrustar la imagen en cada capa.
    std::string type;
    GridCoord position;

    // Patrulla opcional (ver Enemy::Enemy): si el JSON no trae
    // "patrolMin"/"patrolMax", ambos quedan iguales a "position" (sin
    // patrulla, el enemigo se queda quieto). Es responsabilidad de quien
    // instancie Enemy decidir que hacer con eso; LevelLoader no impone
    // la semantica de "sin patrulla" mas alla de rellenar el dato.
    GridCoord patrolMin;
    GridCoord patrolMax;

    // Habilidades que puede usar en combate (Fase 7, "Sistema de
    // habilidades"): ids que resuelven contra un SkillCatalog (ver
    // Skill.h). Vacio = enemigo sin habilidades (solo ataque basico).
    std::vector<std::string> skillIds;
};

// Fase 10 ("todo es un objeto"): un objeto de CUALQUIER categoria del
// ObjectCatalog (ver ObjectCatalog.h) colocado en el nivel -- una llave,
// un arbusto, un enemigo. Generaliza EnemySpawn: aqui solo se dice QUE
// objeto (objectId, resuelto contra el catalogo) y DONDE (position, mas
// patrulla opcional que solo tiene sentido para category==Enemy); todo
// lo demas (stats, habilidades, sprite, colision) vive en el catalogo,
// no en cada spawn -- las habilidades de un slime son del TIPO slime
// (CombatData::skillIds), no de cada slime individual, a diferencia de
// EnemySpawn::skillIds (que se mantiene por compatibilidad, ver abajo).
struct ObjectSpawn {
    std::string objectId;
    GridCoord position;
    // Igual que en EnemySpawn: sin "patrolMin"/"patrolMax" en el JSON,
    // ambos quedan iguales a position (objeto quieto). Ignorados para
    // categorias que no patrullan (Prop/Pickup).
    GridCoord patrolMin;
    GridCoord patrolMax;

    // --- Transicion a otro nivel (puertas de edificio, escaleras, pasos
    // entre mapas). Va en el SPAWN y no en el catalogo porque el destino
    // es de esta puerta concreta, no del tipo "puerta": dos posadas
    // comparten definicion pero llevan a interiores distintos. ---

    // Ruta del nivel destino ("assets/levels/interior_posada.json"), o
    // vacia si este objeto no lleva a ningun sitio. Es lo que distingue
    // una puerta de un adorno.
    std::string targetLevel;
    // Donde aparece el jugador al llegar. Sin "targetPosition" en el
    // JSON se usa el playerStart del nivel destino (lo resuelve quien
    // ejecuta la transicion, no LevelLoader: aqui solo se guarda el dato
    // y si venia o no).
    GridCoord targetPosition;
    bool hasTargetPosition = false;

    // --- Variante local del arquetipo ---
    // El catalogo define el objeto reutilizable; estos campos solo viven
    // en ESTA instancia. Asi una misma pocion puede ser curativa, de mana
    // o de mision sin duplicar la ficha base ni romper los niveles viejos.
    std::string displayName;
    std::string variant;
    float scale = 1.0f;
    std::string effectOverride;
    std::map<std::string, std::string> properties;
};

struct LevelDefinition {
    std::string name;
    // Ruta al TMX del mapa base (Fase 2 del Gantt original,
    // TileMap::loadFromFile): LevelDefinition no lo carga, solo
    // referencia la ruta -- separa "que nivel es" (JSON, contenido de
    // diseno) de "como se dibuja el suelo" (TMX, ya resuelto desde la
    // Fase 2).
    std::string mapPath;
    GridCoord playerStart;

    // "enemies" (Fase 6) y "objects" (Fase 10) conviven a proposito:
    // los niveles existentes con "enemies" siguen cargando igual (sin
    // migracion forzosa), y los nuevos pueden usar solo "objects" (que
    // cubre enemigos Y todo lo demas). Un mismo nivel puede traer ambos;
    // quien instancie el nivel los recorre por separado. Si en el futuro
    // todo el contenido migra a "objects", "enemies" se retira entonces
    // (no antes: retirar API con usuarios es un paso propio, no un
    // efecto colateral).
    std::vector<EnemySpawn> enemies;
    std::vector<ObjectSpawn> objects;
};
