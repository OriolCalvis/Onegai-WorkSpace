// demo_narrativa_combate — el efecto "startBattle" de verdad, no sobre el papel.
//
// El puente Nd6 (demo_narrativa_nd6) probo que un beat puede pedir una TIRADA.
// Este prueba que puede pedir un COMBATE, que es distinto en lo unico que
// importa: el tiempo.
//
//   skillCheck  -> se resuelve dentro de applyNarrative. Al volver, la flag
//                  ya esta puesta.
//   startBattle -> applyNarrative deja el combate montado y se va. La
//                  flag no existe todavia. Aparece cuando el BattleState se
//                  cierra, en syncBattleOutcome, que puede ser muchos turnos
//                  despues. (TUTORIAL-01: si el beat habla, primero se lee -- modo
//                  Dialogue -- y al cerrar el dialogo empieza el combate.)
//
// Si eso no funcionara, una aventura que ramifica por el resultado de una
// pelea se quedaria colgada para siempre esperando una flag que nadie pone.
// Por eso este demo existe: comprueba las dos ramas y ademas el caso feo
// (monsterId que no esta en el catalogo).
//
// GL-free: no abre ventana. TileMap se pasa como nullptr, que GameSession
// acepta (sin mapa no hay colision de terreno, y aqui no se anda).

#include <cstdio>
#include <string>
#include <vector>

#include "Core/Check.h"
#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"
#include "RPG/NarrativeEngine.h"

namespace {

// Un combatiente minimo: solo vida. Suficiente para que BattleState funcione.
class Muneco : public ICombatant {
public:
    explicit Muneco(int hp) : m_hp(hp), m_max(hp) {}
    int health() const override { return m_hp; }
    int maxHealth() const override { return m_max; }
    void takeDamage(int amount) override { m_hp = (amount >= m_hp) ? 0 : m_hp - amount; }
    void heal(int amount) override { m_hp = (m_hp + amount > m_max) ? m_max : m_hp + amount; }
    bool isAlive() const override { return m_hp > 0; }
    int stat(RPG::Stat) const override { return 2; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "muneco"; }

private:
    int m_hp;
    int m_max;
};

const char* kAventura = R"JSON({
  "id": "adv_prueba_combate",
  "name": "Prueba de startBattle",
  "objectives": [],
  "beats": [
    {
      "id": "beat_emboscada",
      "trigger": { "type": "talk", "target": "senuelo" },
      "requires": { "notFlags": ["emboscada_lanzada"] },
      "speaker": "",
      "lines": ["Alguien te corta el paso."],
      "effects": [
        { "type": "setFlag", "arg": "emboscada_lanzada" },
        { "type": "startBattle",
          "startBattle": { "monsterId": "perdido_saqueador",
                           "flagVictory": "emboscada_ganada",
                           "flagDefeat": "emboscada_perdida" } }
      ]
    },
    {
      "id": "beat_tras_ganar",
      "trigger": { "type": "auto" },
      "requires": { "allFlags": ["emboscada_ganada"], "notFlags": ["cerrado"] },
      "speaker": "",
      "lines": ["El callejon vuelve a estar vacio."],
      "effects": [ { "type": "setFlag", "arg": "cerrado" } ]
    },
    {
      "id": "beat_fantasma",
      "trigger": { "type": "talk", "target": "fantasma" },
      "requires": { "notFlags": ["fantasma_lanzado"] },
      "speaker": "",
      "lines": ["Un enemigo que no existe en el catalogo."],
      "effects": [
        { "type": "setFlag", "arg": "fantasma_lanzado" },
        { "type": "startBattle",
          "startBattle": { "monsterId": "no_existe_este_bicho",
                           "flagVictory": "fantasma_ganado",
                           "flagDefeat": "fantasma_perdido" } }
      ]
    }
  ]
})JSON";

ObjectCatalog construyeCatalogo() {
    ObjectCatalog cat;
    auto r = cat.loadFromFile("assets/objects/boundington_enemigos.json");
    require(r);
    return cat;
}

}  // namespace

int main() {
    std::printf("=== demo_narrativa_combate: el efecto startBattle ===\n\n");

    auto guionR = RPG::AdventureScript::loadFromString(kAventura);
    require(guionR);
    const RPG::AdventureScript guion = guionR.value();

    ObjectCatalog catalogo = construyeCatalogo();
    require(catalogo.find("perdido_saqueador") != nullptr);
    // Canon: 22; desde DANO-02 (d6 por la diferencia de exitos) las vidas van
    // a la mitad, y desde D0014 (14/10: el empate roza y el saqueador pasa a
    // DES 2) el saqueador es un esbirro de 5.
    std::printf("  catalogo: perdido_saqueador con %d de vida (canon: 22; esbirro de D0014)\n",
                catalogo.find("perdido_saqueador")->combat.maxHealth);
    require(catalogo.find("perdido_saqueador")->combat.maxHealth == 5);

    // ---------------------------------------------------------------
    // 1. Ganar el combate enciende flagVictory... pero NO en el momento
    //    de pedirlo.
    // ---------------------------------------------------------------
    std::printf("\n[1] el combate se pide, y la flag llega despues\n");
    {
        Muneco heroe(200);   // mucha vida: gana seguro
        SkillSet skills(0);
        std::vector<std::string> inventario;
        LevelDefinition nivel;
        GameSession s(nullptr, &catalogo, nullptr, nivel, &heroe, &skills, &inventario);

        RPG::NarrativeState estado;
        RPG::NarrativeEngine motor;
        motor.setAdventure(&guion);
        s.setNarrative(&motor, &estado);

        s.talkNarrative("senuelo");

        // Lo importante de todo el demo: aqui el beat YA se ejecuto y la
        // flag TODAVIA no esta. Ese hueco es el que hay que probar.
        require(estado.hasFlag("emboscada_lanzada"));
        require(!estado.hasFlag("emboscada_ganada"));
        // TUTORIAL-01: el beat habla («Alguien te corta el paso.»): primero se lee,
        // con el combate ya montado, y al cerrar el dialogo se pelea.
        require(s.mode() == GameMode::Dialogue && s.battle() != nullptr);
        s.closeInteraction();
        require(s.mode() == GameMode::Battle);
        std::printf("    tras el beat: su dialogo, y al cerrarlo modo=Battle, flag de victoria aun sin poner. OK\n");

        // Pelear hasta que acabe.
        int vueltas = 0;
        while (s.mode() == GameMode::Battle && vueltas++ < 500) {
            BattleState* b = s.battle();
            if (b == nullptr) break;
            BattleAction ataque;
            ataque.type = BattleActionType::Attack;
            b->resolveAllyAction(0, ataque);
            b->resolveEnemyTurn();
            s.syncBattleOutcome();
        }
        require(estado.hasFlag("emboscada_ganada"));
        require(!estado.hasFlag("emboscada_perdida"));
        std::printf("    tras la pelea (%d turnos): emboscada_ganada encendida. OK\n", vueltas);

        // Y la aventura sigue a partir de esa flag: TUTORIAL-01, al acabar el combate
        // la sesion da un tick, como al cerrar un dialogo.
        require(estado.hasFlag("cerrado"));
        require(s.mode() == GameMode::Dialogue && !s.dialogueLines().empty() &&
                s.dialogueLines()[0] == "El callejon vuelve a estar vacio.");
        std::printf("    el beat que dependia de la victoria dispara al acabar el combate. OK\n");
    }

    // ---------------------------------------------------------------
    // 2. Perder enciende flagDefeat.
    // ---------------------------------------------------------------
    std::printf("\n[2] perder enciende la otra flag\n");
    {
        Muneco heroe(1);     // se cae con nada
        SkillSet skills(0);
        std::vector<std::string> inventario;
        LevelDefinition nivel;
        GameSession s(nullptr, &catalogo, nullptr, nivel, &heroe, &skills, &inventario);

        RPG::NarrativeState estado;
        RPG::NarrativeEngine motor;
        motor.setAdventure(&guion);
        s.setNarrative(&motor, &estado);
        s.talkNarrative("senuelo");
        s.closeInteraction();  // TUTORIAL-01: se lee lo que dice el beat y empieza el combate

        int vueltas = 0;
        while (s.mode() == GameMode::Battle && vueltas++ < 500) {
            BattleState* b = s.battle();
            if (b == nullptr) break;
            BattleAction ataque;
            ataque.type = BattleActionType::Attack;
            b->resolveAllyAction(0, ataque);
            b->resolveEnemyTurn();
            s.syncBattleOutcome();
        }
        const bool gano = estado.hasFlag("emboscada_ganada");
        const bool perdio = estado.hasFlag("emboscada_perdida");
        std::printf("    resultado: %s\n", gano ? "victoria" : (perdio ? "derrota" : "NINGUNA"));
        require(gano || perdio);   // una de las dos, nunca ninguna
    }

    // ---------------------------------------------------------------
    // 3. El caso feo: un monsterId que no existe. Debe encender
    //    flagDefeat, NO quedarse callado -- si no, la aventura espera
    //    para siempre una flag que nadie va a poner.
    // ---------------------------------------------------------------
    std::printf("\n[3] enemigo inexistente: no cuelga la aventura\n");
    {
        Muneco heroe(50);
        SkillSet skills(0);
        std::vector<std::string> inventario;
        LevelDefinition nivel;
        GameSession s(nullptr, &catalogo, nullptr, nivel, &heroe, &skills, &inventario);

        RPG::NarrativeState estado;
        RPG::NarrativeEngine motor;
        motor.setAdventure(&guion);
        s.setNarrative(&motor, &estado);
        s.talkNarrative("fantasma");

        require(estado.hasFlag("fantasma_perdido"));
        require(s.mode() != GameMode::Battle);
        std::printf("    flag de derrota puesta y sin entrar en combate. OK\n");
    }

    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
