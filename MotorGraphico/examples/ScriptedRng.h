#pragma once

#include <cstddef>
#include <vector>

#include "RPG/RandomEngine.h"

// RandomEngine con la tirada escrita a mano, para los demos que hacen de
// test. Vive en examples/ y no en include/, igual que Check.h: es utilidad
// de los tests, no del motor.
//
// POR QUE HACE FALTA. Desde que el combate pasa por DicePoolEngine, el
// daño de un ataque depende del grado de la tirada (pifia 0, parcial la
// mitad, exito lo normal, critico x1.5). Un test que afirme un numero de
// PV exacto sin controlar el dado no comprueba la regla: comprueba qué
// salio de la semilla por defecto de Xoroshiro128p. Eso pasa igual de
// verde con las reglas bien que con las reglas mal, y se rompe en cuanto
// alguien cambia CUANTOS dados se tiran aunque el resultado siga siendo
// correcto -- que es exactamente lo que ocurrio al arreglar el orden de
// las ramas de resolve_against_cd.
//
// Con esto, un test dice lo que quiere decir:
//
//     ScriptedRng dado({6});                // saco un 6: 1 exito
//     battle.setRandomEngine(dado);
//     ...                                   // -> CRITICAL -> x1.5
//
// Recordatorio de la conversion (DicePoolEngine::roll_pool):
//     6 -> 1 exito | 5 -> medio | 1-4 -> nada
//     con ADVANTAGE el 4 tambien vale medio
//     con DISADVANTAGE el 5 deja de valer
class ScriptedRng : public RPG::RandomEngine {
public:
    explicit ScriptedRng(std::vector<int> rolls) : m_rolls(std::move(rolls)) {}

    // Al agotarse el guion repite el ultimo valor en vez de leer fuera del
    // vector: asi un pool mas largo de lo previsto da un resultado
    // aburrido y estable en lugar de basura o un cuelgue.
    int roll_d6() override {
        ++m_calls;
        if (m_rolls.empty()) return 1;
        if (m_next >= m_rolls.size()) return m_rolls.back();
        return m_rolls[m_next++];
    }

    int roll_in_range(int a, int b) override { return a > b ? b : a; }
    uint64_t seed() const override { return 0; }

    // Cuenta LLAMADAS, no posiciones del guion: si contara posiciones
    // dejaria de subir al agotarse el guion, y "cuantos dados se han
    // tirado" saldria mal justo cuando mas importa (techo de 24 dados).
    std::size_t consumed() const { return m_calls; }

    // Vuelve al principio del guion, para reutilizar el mismo dado en
    // varias fases de un test sin construir otro.
    void rewind() { m_next = 0; }

private:
    std::vector<int> m_rolls;
    std::size_t m_next = 0;
    std::size_t m_calls = 0;
};
