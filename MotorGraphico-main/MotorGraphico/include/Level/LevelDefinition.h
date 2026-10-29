#pragma once

#include <map>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Runtime/Alturas.h"

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

    // G4-01 — Puerta cerrada por flags: "cerradaSi": {"todas": [...], "ninguna": [...]}.
    // Cerrada = estan encendidas TODAS las de `cerradaTodas` y NINGUNA de
    // `cerradaNinguna`. Con las dos listas vacias (o sin el campo) la puerta
    // esta abierta. Cerrada, no hay transicion: se dispara el beat
    // `beatCerrada` (un talk cuyo target es ese id) o, sin beat, un aviso.
    std::vector<std::string> cerradaTodas;
    std::vector<std::string> cerradaNinguna;
    std::string beatCerrada;
    template <class TieneFlag>
    bool estaCerrada(TieneFlag tieneFlag) const {
        if (cerradaTodas.empty() && cerradaNinguna.empty()) return false;
        for (const std::string& f : cerradaTodas) if (!tieneFlag(f)) return false;
        for (const std::string& f : cerradaNinguna) if (tieneFlag(f)) return false;
        return true;
    }

    // --- Botin de la instancia (cofres, enemigos con drop propio) ---
    // El catalogo define el TIPO de objeto; lo que suelta ESTA instancia
    // es decision del autor del nivel (mismo principio que targetLevel
    // con las puertas). "loot" lleva ids fijos del ObjectCatalog que se
    // conceden al abrir/derrotar; "lootTable" referencia opcionalmente
    // una tabla del catalogo RPG de botin (assets/catalogs/
    // loot_tables.json). Pueden convivir: quien consuma el spawn decide
    // como combinarlos (el runtime lo hara cuando llegue el sistema de
    // botin; hasta entonces el editor guarda la decision del autor).
    std::vector<std::string> loot;
    std::string lootTable;

    // Evento/mision que dispara esta instancia: un id estable que el
    // motor narrativo pueda escuchar (un beat/flag de la aventura
    // activa reacciona a el). Vacio = la instancia no dispara nada.
    // Es la semilla de los eventos de mapa del roadmap del editor (S7):
    // el autor ya puede declararlos desde el editor aunque el runtime
    // los consuma despues.
    std::string eventId;

    // AV18-07 (PO-52) — Primer dia de la historia en que este objeto se puede usar.
    // 0 = siempre (lo normal). Lo escribe tools/repartir_botin.py en los
    // contenedores del dia 2 y 3 que estan en mapas que ya se pisan el dia 1:
    // hasta ese dia el cofre se ve pero no se abre («Todavia no»). Solo lo
    // consulta el abrir contenedores; no toca el guardado (el dia es una
    // variable narrativa que ya se guarda).
    int disponibleDia = 0;

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
    // AV19-02 — Altura del terreno (ver Runtime/Alturas.h). Vacio = todo plano.
    std::vector<AlturaRegion> alturas;

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
    // Documento de autoría; ausente en niveles legacy. mapPath sigue siendo
    // la salida TMX consumida por el runtime. Rutas relativas al proyecto/cwd.
    std::string mapDocumentPath;

    // C-23 — Post-processat per nivell. Quan cap dels dos esta activat,
    // Application no crea cap objecte ni fa cap crida GL (cost zero).
    bool fogEnabled = false;
    int fogRevealRadius = 1;  // radi Chebyshev al voltant del jugador
    bool lightingEnabled = false;
    float ambientR = 0.35f;
    float ambientG = 0.35f;
    float ambientB = 0.40f;
};
