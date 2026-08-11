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
// Reglas del pool (GDD Llibre II):
//   - Se tiran N d6. N = stat o valor efectivo de la fuente.
//   - Cada 6 suma 1 éxito; cada 5 suma medio; 1-4 no suman.
//   - Crítico: TODOS los dados sacan 6.   Pifia: 0 éxitos.
//   - Ventaja: los 4 también suman medio. Desventaja: solo cuentan los 6.
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
    bool botch = false;       // 0 éxitos
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
    // dice <= 0 devuelve un pool vacío marcado como pifia: quedarse sin
    // dados (por una condición que resta) es un resultado legítimo del
    // juego, no un error que deba reventar la partida.
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
        r.botch = (dice == 0) || (r.successes == 0.0f && dice > 0);
        return r;
    }

    // Tirada contra una CD numérica (0.0 a 3.0).
    //
    //     todos 6, o éxitos >= cd + 1  -> CRITICAL
    //     éxitos >= cd                 -> SUCCESS
    //     éxitos > 0                   -> PARTIAL  (algo se consigue)
    //     éxitos == 0                  -> BOTCH
    static CheckOutcome resolve_against_cd(const PoolResult& pool, float cd) {
        if (cd < 0.0f) cd = 0.0f;
        if (cd > CD_MAX) cd = CD_MAX;
        CheckOutcome o;
        o.successes = pool.successes;
        o.cd = cd;
        o.critical = pool.critical;
        if (pool.critical || pool.successes >= cd + 1.0f) {
            o.degree = Degree::CRITICAL;
        } else if (pool.successes >= cd) {
            o.degree = Degree::SUCCESS;
        } else if (pool.successes > 0.0f) {
            o.degree = Degree::PARTIAL;
        } else {
            o.degree = Degree::BOTCH;
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
