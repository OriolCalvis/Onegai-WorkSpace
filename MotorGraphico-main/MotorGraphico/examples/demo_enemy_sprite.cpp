// Enemy delegando en EnemyBrain: test de NO REGRESION del refactor de la
// fractura #1. Repite los asserts de patrulla de
// demo_animated_entity::testEnemy, pero SIN ventana (atlas nullptr, no se
// llama a render()), para que se ejecute en cualquier maquina y en CI.
//
// Existe porque el refactor SI rompio el comportamiento y este test lo
// pillo: al extraer la patrulla a EnemyBrain, la version nueva invertia
// el sentido "al intentar salir" del rango en vez de "al llegar" al
// extremo. Resultado: el enemigo se quedaba un tic parado en cada punta y
// su ida y vuelta dejaba de ser regular. Compilaba perfecto y parecia
// correcto leyendolo; solo ejecutar la secuencia exacta lo delato.
//
// Moraleja para el proximo refactor: que compile no prueba que el
// comportamiento se conserve. Hace falta un test que fije la SECUENCIA,
// no solo el estado final.
#include "Render/Enemy.h"
#include "Check.h"
#include <iostream>

int main() {
    // atlas nullptr: no se llama a render(), solo a update()/estado.
    Enemy enemy(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{3, 0}, nullptr, 64, 32, 0.5f);

    enemy.update(0.5f);
    require(enemy.gridPosition().x == 1 && enemy.aiState() == Enemy::kPatrol);
    enemy.update(0.5f);
    require(enemy.gridPosition().x == 2);
    enemy.update(0.5f);
    require(enemy.gridPosition().x == 3);   // toca patrolMax
    enemy.update(0.5f);
    require(enemy.gridPosition().x == 2);   // e invierte
    enemy.update(0.5f);
    require(enemy.gridPosition().x == 1);
    enemy.update(0.5f);
    require(enemy.gridPosition().x == 0);   // toca patrolMin
    enemy.update(0.5f);
    require(enemy.gridPosition().x == 1);
    std::cout << "[ENEMY] patrulla identica a la de antes del refactor.\n";

    require(enemy.health() == 50 && enemy.isAlive());
    enemy.takeDamage(60);
    require(enemy.health() == 0 && !enemy.isAlive());
    std::cout << "[ENEMY] vida por defecto 50 y muerte, delegadas al brain.\n";

    Enemy quieto(GridCoord{5, 5}, GridCoord{5, 5}, GridCoord{5, 5}, nullptr);
    quieto.update(10.0f);
    require(quieto.gridPosition().x == 5 && quieto.gridPosition().y == 5);
    require(quieto.aiState() == Enemy::kIdle);
    std::cout << "[ENEMY] rango degenerado sigue dando kIdle.\n";

    // La delegacion es REAL: tocar el brain se ve desde Enemy.
    enemy.brain().heal(25);
    require(enemy.health() == 25 && "Enemy y su brain comparten la MISMA vida");
    std::cout << "[ENEMY] una sola vida: no hay copia que se desincronice.\n";

    std::cout << "\nTodas las comprobaciones han pasado correctamente.\n";
    return 0;
}
