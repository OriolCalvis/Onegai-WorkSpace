// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-18  (fase C, 2 j) - HandlePool sosté World
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Core/Memory/HandlePool.h"

#include "TestHarness.h"

#include <string>

namespace {

MOTOR_TEST_CASE(default_handle_is_not_valid) {
    HandlePool<int> pool;
    const HandlePool<int>::Handle invalid;

    MOTOR_CHECK(pool.empty());
    MOTOR_CHECK(!pool.contains(invalid));
    MOTOR_CHECK(pool.get(invalid) == nullptr);
    MOTOR_CHECK(!pool.erase(invalid));
}

MOTOR_TEST_CASE(reused_slot_invalidates_its_previous_handle) {
    HandlePool<std::string> pool;
    const auto first = pool.emplace("first");
    const auto second = pool.emplace("second");

    MOTOR_CHECK(pool.erase(first));
    const auto replacement = pool.emplace("replacement");

    MOTOR_CHECK(replacement.index == first.index);
    MOTOR_CHECK(replacement.generation != first.generation);
    MOTOR_CHECK(!pool.contains(first));
    MOTOR_CHECK(pool.contains(second));
    MOTOR_CHECK(pool.get(replacement) != nullptr);
    MOTOR_CHECK(*pool.get(replacement) == "replacement");
}

MOTOR_TEST_CASE(clear_invalidates_every_live_handle_and_reuses_slots) {
    HandlePool<int> pool;
    const auto first = pool.emplace(10);
    const auto second = pool.emplace(20);

    pool.clear();

    MOTOR_CHECK(pool.empty());
    MOTOR_CHECK(!pool.contains(first));
    MOTOR_CHECK(!pool.contains(second));

    const auto replacement = pool.emplace(30);
    MOTOR_CHECK(pool.contains(replacement));
    MOTOR_CHECK(!pool.contains(first));
    MOTOR_CHECK(!pool.contains(second));
    MOTOR_CHECK(pool.get(replacement) != nullptr);
    MOTOR_CHECK(*pool.get(replacement) == 30);
}

// C-18 — Test de constructor por movimiento
MOTOR_TEST_CASE(move_constructor_transfers_free_list_correctly) {
    HandlePool<std::string> original;
    const auto first = original.emplace("first");
    const auto second = original.emplace("second");
    
    // Borramos el primero para crear un hueco en la free list
    MOTOR_CHECK(original.erase(first));
    
    // Movemos el pool
    HandlePool<std::string> moved(std::move(original));
    
    // Verificamos que el pool movido funciona correctamente
    MOTOR_CHECK(moved.size() == 1);
    MOTOR_CHECK(moved.contains(second));
    MOTOR_CHECK(!moved.contains(first));
    
    // El hueco debe poder reutilizarse
    const auto replacement = moved.emplace("replacement");
    MOTOR_CHECK(moved.contains(replacement));
    MOTOR_CHECK(replacement.index == first.index);  // Reutiliza el hueco
    MOTOR_CHECK(replacement.generation != first.generation);  // Nueva generación
}

// C-18 — Test de asignación por movimiento
MOTOR_TEST_CASE(move_assignment_transfers_free_list_correctly) {
    HandlePool<int> original;
    const auto first = original.emplace(10);
    MOTOR_CHECK(original.erase(first));
    
    HandlePool<int> target;
    target.emplace(100);  // Algo de contenido previo
    
    target = std::move(original);
    
    MOTOR_CHECK(target.size() == 0);  // El contenido previo se perdió, solo queda el hueco
    
    const auto replacement = target.emplace(20);
    MOTOR_CHECK(replacement.index == first.index);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
