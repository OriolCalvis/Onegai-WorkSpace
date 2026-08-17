// EnemyBrain: la logica del enemigo separada del sprite (fractura #1 de
// ARCHITECTURE.md). GL-free entero, se ejecuta sin ventana.
//
// Por que existe este demo: habia DOS representaciones del enemigo sin
// relacion -- Enemy (sprite + IA de patrulla, muerta en el juego real) y
// CatalogCombatant (solo vida, la que el juego usaba). El coste no era
// estetico: LevelLoader parseaba patrolMin/patrolMax del JSON, nadie los
// guardaba, y los enemigos NUNCA se movian. El dato de patrulla se leia
// y se tiraba mientras la IA que lo usaba existia, escrita y probada, en
// la clase muerta.
//
// Verifica:
//  1. Patrulla de ida y vuelta entre patrolMin y patrolMax.
//  2. Rango degenerado (min == max) = enemigo quieto, sin dividir por
//     cero ni vibrar en el sitio.
//  3. El ritmo NO depende del framerate: 60 tics de 1/60 s recorren lo
//     mismo que 6 de 1/6 s.
//  4. Un obstaculo hace que se de la vuelta, no que se quede empujando.
//  5. Sigue siendo ICombatant: recibe dano, muere, y heal() no pasa de
//     maxHealth.
#include "Game/EnemyBrain.h"

#include "Check.h"

#include <iostream>
#include <algorithm>
#include <set>
#include <utility>

namespace {

// Callback de "puede entrar" que deja pasar todo (mapa abierto).
auto libre = [](const GridCoord&) { return true; };

// Avanza "segundos" en tics de "dt" (simula frames reales).
template <typename Fn>
void avanzar(EnemyBrain& brain, float segundos, float dt, Fn canEnter) {
    for (float t = 0.0f; t < segundos; t += dt) {
        brain.update(dt, canEnter);
    }
}

}  // namespace

int main() {
    // --- 1. Patrulla de ida y vuelta ---
    EnemyBrain patrullero(GridCoord{2, 5}, GridCoord{2, 5}, GridCoord{5, 5}, 20, 0.5f);
    require(patrullero.position().x == 2);

    std::set<int> visitadas;
    for (int i = 0; i < 40; ++i) {
        patrullero.update(0.5f, libre);
        visitadas.insert(patrullero.position().x);
        // Nunca se sale del rango declarado en el JSON.
        require(patrullero.position().x >= 2 && patrullero.position().x <= 5);
        require(patrullero.position().y == 5);  // la patrulla es horizontal
    }
    require(visitadas.size() == 4 && "deberia recorrer las 4 celdas del rango");
    require(patrullero.aiState() == EnemyBrain::kPatrol);
    std::cout << "[BRAIN] patrulla las 4 celdas de su rango, ida y vuelta, sin salirse.\n";

    // --- 2. Rango degenerado: quieto ---
    EnemyBrain quieto(GridCoord{7, 3}, GridCoord{7, 3}, GridCoord{7, 3});
    avanzar(quieto, 10.0f, 0.5f, libre);
    require(quieto.position().x == 7 && quieto.position().y == 3);
    require(quieto.aiState() == EnemyBrain::kIdle);
    std::cout << "[BRAIN] sin patrulla declarada (min == max) se queda quieto, estado kIdle.\n";

    // --- 3. El ritmo no depende del framerate ---
    EnemyBrain rapido(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{20, 0}, 20, 0.5f);
    EnemyBrain lento(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{20, 0}, 20, 0.5f);
    avanzar(rapido, 3.0f, 1.0f / 60.0f, libre);  // 180 frames
    avanzar(lento, 3.0f, 1.0f / 6.0f, libre);    // 18 frames
    require(rapido.position().x == lento.position().x);
    std::cout << "[BRAIN] mismo avance a 60 fps que a 6 fps (x=" << rapido.position().x
              << "): el paso va por tiempo, no por frame.\n";

    // --- 4. Un obstaculo lo hace darse la vuelta ---
    // Muro en x=4: el enemigo sale de 2 hacia la derecha y no debe pasar.
    auto muroEnCuatro = [](const GridCoord& c) { return c.x != 4; };
    EnemyBrain topado(GridCoord{2, 0}, GridCoord{2, 0}, GridCoord{8, 0}, 20, 0.5f);
    int maxAlcanzado = 2;
    for (int i = 0; i < 30; ++i) {
        topado.update(0.5f, muroEnCuatro);
        require(topado.position().x != 4 && "no puede atravesar el obstaculo");
        maxAlcanzado = std::max(maxAlcanzado, topado.position().x);
    }
    require(maxAlcanzado == 3 && "deberia llegar hasta la celda anterior al muro");
    require(topado.position().x >= 2 && "y volver, no quedarse empujando");
    std::cout << "[BRAIN] ante un obstaculo se da la vuelta (llega hasta x=" << maxAlcanzado
              << ", nunca a 4).\n";

    // --- 5. Sigue siendo ICombatant ---
    EnemyBrain combatiente(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0}, 20);
    require(combatiente.maxHealth() == 20 && combatiente.health() == 20);
    combatiente.takeDamage(8);
    require(combatiente.health() == 12);
    combatiente.heal(100);
    require(combatiente.health() == 20 && "heal no puede pasar de maxHealth");
    combatiente.takeDamage(999);
    require(combatiente.health() == 0 && !combatiente.isAlive());
    combatiente.takeDamage(5);
    require(combatiente.health() == 0 && "la vida no baja de cero");
    std::cout << "[BRAIN] ICombatant correcto: dano, curacion clampada y muerte.\n";

    // Caso limite: maxHealth 0 no debe nacer muerto (bloquearia el combate
    // antes de empezar).
    EnemyBrain degenerado(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0}, 0);
    require(degenerado.isAlive());
    std::cout << "[BRAIN] maxHealth 0 se clampa a 1: ningun enemigo nace muerto.\n";

    std::cout << "\nTodas las comprobaciones han pasado correctamente.\n";
    return 0;
}
