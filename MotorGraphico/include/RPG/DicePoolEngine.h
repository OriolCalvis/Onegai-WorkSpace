#pragma once

#include <cstdint>
#include <vector>

#include "RPG/Defense.h"
#include "RPG/RandomEngine.h"
#include "RPG/Stat.h"

namespace RPG {

// Motor de tiradas (GAMEMACHINE_NECESIDADES.md P0-5): la ÚNICA forma de
// resolver una acción -- ataque, salvación, hechizo, prueba de
// característica o tirada enfrentada. Sustituye al
// `target.takeDamage(skill.power)` del motor actual, donde el daño era
// plano y no había tirada.
//
// Reglas del pool (GDD Llibre II + Ed. 3, bitácora 2.17):
//   - Se tiran N d6. N = stat o valor efectivo de la fuente.
//   - Cada 6 suma 1 éxito; cada 5 suma medio; 1-4 no suman.
//   - Crítico: TODOS los dados sacan 6.
//   - Pifia (Ed. 3): la tirada no tiene NINGÚN 5 ni 6 Y saca mínimo dos 1es.
//     (Antes de Ed. 3 era «0 éxitos»; ya no.)
//   - Ventaja: los 4 también suman medio. Desventaja: solo cuentan los 6.
//   - Ed. 3: la CA y la CD se eliminan; TODA resolución (combate, hechizos
//     y checks narrativos) es una tirada enfrentada — ver opposed_degree().
//
// GL-free y SIN ESTADO: todos los métodos son estáticos y el azar entra
// por RandomEngine& (ver RandomEngine.h). Una misma semilla da siempre la
// misma batalla, que es lo que permite testear combate de verdad.
enum class DiceMod : uint8_t { NORMAL = 0, ADVANTAGE = 1, DISADVANTAGE = 2 };

// Grado de éxito. Importa que sean cuatro y no un booleano: la magnitud de
// una habilidad depende del grado (SkillDefinition::magnitude_by_degree).
enum class Degree : uint8_t { BOTCH = 0, PARTIAL = 1, SUCCESS = 2, CRITICAL = 3 };

struct PoolResult {
    float successes = 0.0f;   // 0, 0.5, 1.0, 1.5, ...
    bool critical = false;    // todos los dados == 6
    bool botch = false;       // Ed. 3: ningún 5/6 y mínimo dos 1es
    std::vector<int> rolls;   // valores reales, para el log y el HUD
};

struct CheckOutcome {
    Degree degree = Degree::BOTCH;
    float successes = 0.0f;
    float cd = 0.0f;          // dificultad contra la que se comparó
    bool critical = false;
};

struct OpposedOutcome {
    bool attacker_wins = false;
    bool tie = false;
    float attacker_successes = 0.0f;
    float defender_successes = 0.0f;
};

class DicePoolEngine {
public:
    // Pifia Ed. 3 (bitácora 2.17): la tirada no tiene ningún 5 ni ningún 6
    // Y saca mínimo dos 1es. «Si no sale ningún 5 ni 6 y mínimo dos 1es,
    // es pifia» — Oriol, 17/08/2026. Un pool vacío (0 dados, por una
    // condición que resta) NO es pifia: no hay dados que la materialicen;
    // simplemente no suma nada y la enfrentada la resolverá.
    static bool es_pifia(const std::vector<int>& rolls) {
        bool tiene_5o6 = false;
        int unos = 0;
        for (int d : rolls) {
            if (d == 5 || d == 6) tiene_5o6 = true;
            if (d == 1) ++unos;
        }
        return !tiene_5o6 && unos >= 2;
    }

    // dice <= 0 devuelve un pool vacío: quedarse sin dados (por una
    // condición que resta) es un resultado legítimo del juego, no un error
    // que deba reventar la partida.
    static PoolResult roll_pool(int dice, DiceMod mod, RandomEngine& rng) {
        PoolResult r;
        if (dice < 0) dice = 0;
        if (dice > 24) dice = 24;
        r.rolls.reserve(dice);
        bool all_six = (dice > 0);

        for (int i = 0; i < dice; ++i) {
            int d = rng.roll_d6();
            r.rolls.push_back(d);
            float add = 0.0f;
            switch (d) {
                case 6: add = 1.0f; break;
                case 5: add = (mod == DiceMod::DISADVANTAGE) ? 0.0f : 0.5f; break;
                case 4: add = (mod == DiceMod::ADVANTAGE) ? 0.5f : 0.0f; break;
                default: add = 0.0f;
            }
            r.successes += add;
            if (d != 6) all_six = false;
        }
        r.critical = all_six;
        r.botch = es_pifia(r.rolls);
        return r;
    }

    // Ed. 3 (bitácora 2.17): TODA resolución es enfrentada — la CA y la CD
    // ya no existen. Grado para el lado ACTIVO (atacante/lanzador):
    //
    //     PIFIA propia                       -> BOTCH (anula la comparación)
    //     PIFIA del defensor                 -> CRITICAL  [interpretación
    //                                              registrada en la bitácora]
    //     empate (sin pifia)                 -> BOTCH (el defensor se impone)
    //     defensor gana                      -> PARTIAL (hechizo/maldición:
    //                                              mitad de daño, sin debuff)
    //     atacante gana por cualquier margen -> SUCCESS (incluso por 0,5)
    //     atacante gana con todos los 6      -> CRITICAL
    static Degree opposed_degree(const PoolResult& atacante, const PoolResult& defensor) {
        if (atacante.botch) return Degree::BOTCH;
        if (defensor.botch) return Degree::CRITICAL;
        if (atacante.successes == defensor.successes) return Degree::BOTCH;
        if (atacante.successes < defensor.successes) return Degree::PARTIAL;
        if (atacante.critical) return Degree::CRITICAL;
        return Degree::SUCCESS;
    }

    // EN DESUSO desde Ed. 3 (bitácora 2.17): la CA y la CD se eliminan y
    // toda resolución pasa a enfrentada (opposed_degree). Se conserva
    // hasta migrar SkillExecutor/BattleStateOnegai y el skillCheck
    // narrativo; después, retirar (un paso propio, ver ARCHITECTURE.md).
    //
    // Tirada contra una CD numérica (0.0 a 3.0).
    //
    //     éxitos == 0                  -> BOTCH   (PRIMERO, siempre)
    //     todos 6, o éxitos >= cd + 1  -> CRITICAL
    //     éxitos >= cd                 -> SUCCESS
    //     éxitos > 0                   -> PARTIAL  (algo se consigue)
    //
    // El BOTCH se comprueba antes que la comparación con la CD a
    // propósito: cd 0.0 es la defensa 10 (el enemigo típico de tier I),
    // y si el 0 >= 0 se evaluara antes, una tirada sin éxitos saldría
    // SUCCESS y TODO ataque acertaría en el tramo inicial del juego
    // (incidente real, ARCHITECTURE.md §3.7; el test que lo fija está en
    // demo_dice_engine, testGrados).
    static CheckOutcome resolve_against_cd(const PoolResult& pool, float cd) {
        if (cd < 0.0f) cd = 0.0f;
        if (cd > CD_MAX) cd = CD_MAX;
        CheckOutcome o;
        o.successes = pool.successes;
        o.cd = cd;
        o.critical = pool.critical;
        if (pool.successes <= 0.0f) {
            o.degree = Degree::BOTCH;
        } else if (pool.critical || pool.successes >= cd + 1.0f) {
            o.degree = Degree::CRITICAL;
        } else if (pool.successes >= cd) {
            o.degree = Degree::SUCCESS;
        } else {
            o.degree = Degree::PARTIAL;
        }
        return o;
    }

    // Tirada contra una defensa entera (DefenseBlock::get). Traduce con
    // defense_to_cd y delega en resolve_against_cd.
    static CheckOutcome resolve_against_defense(const PoolResult& pool, int defense_value) {
        return resolve_against_cd(pool, defense_to_cd(defense_value));
    }

    // Tirada enfrentada. El empate se marca aparte (tie) en vez de
    // decidirlo aquí: en el GDD suele favorecer al defensor, pero es la
    // regla concreta la que debe decidirlo, no el motor de dados.
    static OpposedOutcome resolve_opposed(const PoolResult& attacker,
                                          const PoolResult& defender) {
        OpposedOutcome o;
        o.attacker_successes = attacker.successes;
        o.defender_successes = defender.successes;
        if (attacker.successes > defender.successes) { o.attacker_wins = true; o.tie = false; }
        else if (attacker.successes == defender.successes) { o.attacker_wins = false; o.tie = true; }
        else { o.attacker_wins = false; o.tie = false; }
        return o;
    }

    // Defensa entera -> CD. Cada punto por encima de 10 son +0.5:
    // 10 -> 0.0, 12 -> 1.0, 15 -> 2.5, 16+ -> 3.0 (techo).
    static float defense_to_cd(int defense_value) {
        float cd = (defense_value - 10) * 0.5f;
        if (cd < 0.0f) cd = 0.0f;
        if (cd > CD_MAX) cd = CD_MAX;
        return cd;
    }

    // Techo de CD: sin él, una defensa muy alta daría una dificultad
    // inalcanzable y el jugador no podría acertar jamás.
    static constexpr float CD_MAX = 3.0f;
};

} // namespace RPG
