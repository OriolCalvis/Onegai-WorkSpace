// EST-11 - Pruebas GL-free de ScopeGuard, el `finally` de C++ (norma N4).
//
// Lo que se comprueba aqui es el contrato entero: que la accion ocurre al
// salir del ambito por las TRES salidas (normal, retorno temprano con un
// Result de error, y excepcion que desenvuelve), que ocurre exactamente una
// vez, que release() la desarma y que mover la guarda transfiere la accion en
// vez de duplicarla. Sin ventana y sin disco: es una plantilla sobre un
// callable.

#include "Core/Errors/Result.h"
#include "Core/Errors/ScopeGuard.h"

#include "TestHarness.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

// La forma real de uso: una funcion que devuelve Result y tiene varios puntos
// de salida. Sin guarda, cada retorno temprano tendria que acordarse de
// limpiar; con guarda, se limpia solo y una vez.
Result<void> openWithRollback(std::vector<std::string>& trace, bool failHalfway) {
    auto rollback = makeScopeGuard([&trace] { trace.push_back("limpia"); });
    trace.push_back("abre");
    if (failHalfway) {
        return Result<void>::Error("fallo a mitad");
    }
    rollback.release();
    trace.push_back("entrega");
    return Result<void>::Ok();
}

MOTOR_TEST_CASE(the_action_runs_when_the_scope_ends) {
    int runs = 0;
    {
        ScopeGuard guard{[&runs] { ++runs; }};
        MOTOR_CHECK(guard.armed());
        MOTOR_CHECK(runs == 0);
    }
    MOTOR_CHECK(runs == 1);
}

MOTOR_TEST_CASE(the_action_runs_on_the_early_error_return) {
    std::vector<std::string> trace;
    const Result<void> failed = openWithRollback(trace, true);

    MOTOR_CHECK(!failed.isOk());
    MOTOR_CHECK(failed.errorMessage() == "fallo a mitad");
    MOTOR_CHECK(trace.size() == 2);
    MOTOR_CHECK(trace[0] == "abre");
    MOTOR_CHECK(trace[1] == "limpia");
}

MOTOR_TEST_CASE(release_disarms_the_guard_on_the_success_path) {
    std::vector<std::string> trace;
    const Result<void> ok = openWithRollback(trace, false);

    MOTOR_CHECK(ok.isOk());
    MOTOR_CHECK(trace.size() == 2);
    MOTOR_CHECK(trace[0] == "abre");
    MOTOR_CHECK(trace[1] == "entrega");
}

MOTOR_TEST_CASE(the_action_runs_while_an_exception_unwinds) {
    std::vector<std::string> trace;
    bool caught = false;
    try {
        auto guard = makeScopeGuard([&trace] { trace.push_back("limpia"); });
        trace.push_back("trabaja");
        throw std::runtime_error("fallo del modulo de abajo");
    } catch (const std::exception& error) {
        caught = true;
        trace.push_back(std::string("atrapada: ") + error.what());
    }

    MOTOR_CHECK(caught);
    // La guarda se ejecuta DURANTE el desenvolvimiento, antes que el catch del
    // llamador: es lo que la hace util en la frontera.
    MOTOR_CHECK(trace.size() == 3);
    MOTOR_CHECK(trace[0] == "trabaja");
    MOTOR_CHECK(trace[1] == "limpia");
    MOTOR_CHECK(trace[2] == "atrapada: fallo del modulo de abajo");
}

MOTOR_TEST_CASE(moving_a_guard_transfers_the_action_instead_of_duplicating_it) {
    int runs = 0;
    {
        ScopeGuard original{[&runs] { ++runs; }};
        ScopeGuard moved{std::move(original)};
        MOTOR_CHECK(!original.armed());
        MOTOR_CHECK(moved.armed());
    }
    MOTOR_CHECK(runs == 1);
}

MOTOR_TEST_CASE(guards_run_in_reverse_order_of_construction) {
    std::vector<std::string> trace;
    {
        auto first = makeScopeGuard([&trace] { trace.push_back("primera"); });
        auto second = makeScopeGuard([&trace] { trace.push_back("segunda"); });
        auto third = makeScopeGuard([&trace] { trace.push_back("tercera"); });
        MOTOR_CHECK(trace.empty());
    }
    MOTOR_CHECK(trace.size() == 3);
    MOTOR_CHECK(trace[0] == "tercera");
    MOTOR_CHECK(trace[1] == "segunda");
    MOTOR_CHECK(trace[2] == "primera");
}

MOTOR_TEST_CASE(the_factory_accepts_a_callable_with_captured_state) {
    std::string text = "suelto";
    {
        auto guard = makeScopeGuard([&text] { text = "retenido"; });
        MOTOR_CHECK(text == "suelto");
        guard.release();
    }
    MOTOR_CHECK(text == "suelto");

    auto second = makeScopeGuard([&text] { text = "limpiado"; });
    MOTOR_CHECK(second.armed());
    second.release();
    MOTOR_CHECK(!second.armed());
    MOTOR_CHECK(text == "suelto");
}

}  // namespace

int main() { return motor_test::runAllTests(); }
