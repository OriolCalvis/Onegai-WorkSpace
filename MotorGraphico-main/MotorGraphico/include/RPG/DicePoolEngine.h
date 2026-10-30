// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-22  (fase C, 2 j) - Retirada de l'API morta
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstdint>
#include <vector>

#include "RPG/RandomEngine.h"

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
//   - Crítico (D0013, ley de Oriol del 08/10/2026): salen AL MENOS DOS 6.
//       1 dado      -> nunca hay crítico;
//       2 dados     -> los dos en 6;
//       3 o más     -> basta con dos 6, saque lo que saque el resto.
//     «Ha de sortir 2 6 si tens més de 3 daus o més». Antes pedía TODOS los
//     dados en 6: con 4 dados salía un 0,08 % de las veces; ahora, un 13,2 %
//     (171 de 1296; lo cuenta ProbabilidadPool, ver su prueba).
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
    bool critical = false;    // al menos dos 6 (D0013); con un dado, nunca
    bool botch = false;       // Ed. 3: ningún 5/6 y mínimo dos 1es
    std::vector<int> rolls;   // valores reales, para el log y el HUD
};

struct OpposedOutcome {
    bool attacker_wins = false;
    bool tie = false;
    float attacker_successes = 0.0f;
    float defender_successes = 0.0f;
};

class DicePoolEngine {
public:
    // Los 6 que hacen falta para que la tirada sea crítica (D0013). Con un
    // solo dado no se llega nunca: un 6 suelto es un éxito, no un crítico.
    static constexpr int kSeisesParaCritico = 2;

    // La regla del crítico, sola, para quien cuenta caras sin tirar
    // (ProbabilidadPool) o las enseña (el HUD).
    static bool es_critico(const std::vector<int>& rolls) {
        int seises = 0;
        for (int d : rolls) {
            if (d == 6) ++seises;
        }
        return seises >= kSeisesParaCritico;
    }

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
        }
        r.critical = es_critico(r.rolls);
        r.botch = es_pifia(r.rolls);
        return r;
    }

    // Ed. 3 (bitácora 2.17): TODA resolución es enfrentada — la CA y la CD
    // ya no existen. Grado para el lado ACTIVO (atacante/lanzador):
    //
    //     PIFIA propia                       -> BOTCH (anula la comparación)
    //     PIFIA del defensor                 -> CRITICAL (confirmado por
    //                                              Oriol, bitácora 2.17)
    //     defensor gana (supera el hechizo)  -> BOTCH: se salva del todo
    //                                          (0 daño, sin debuff)
    //     empate                             -> PARTIAL: mitad de daño,
    //                                          sin debuff
    //     atacante gana por cualquier margen -> SUCCESS (incluso por 0,5):
    //                                          la habilidad entra entera
    //     atacante gana con dos 6 o más      -> CRITICAL (D0013: ver
    //                                          roll_pool y es_critico)
    static Degree opposed_degree(const PoolResult& atacante, const PoolResult& defensor) {
        if (atacante.botch) return Degree::BOTCH;
        if (defensor.botch) return Degree::CRITICAL;
        if (atacante.successes < defensor.successes) return Degree::BOTCH;
        if (atacante.successes == defensor.successes) return Degree::PARTIAL;
        if (atacante.critical) return Degree::CRITICAL;
        return Degree::SUCCESS;
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
};

} // namespace RPG
