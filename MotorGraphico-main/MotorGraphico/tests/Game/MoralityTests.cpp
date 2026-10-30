// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo toca 1 paquete del corte B:
//   C-24  (fase C, 5 j) - Xarxa de seguretat al camí que es juga
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// C-24 — Xarxa de seguretat: Morality
//
// Morality es una pieza pura GL-free que hoy solo se ejercita
// indirectamente a traves de GameSessionTests (el fixture construye
// GameSession y lee morality().value()). Estas pruebas cubren directamente
// los tramos, el clamp, el historial y la fraccion, sin pasar por
// GameSession ni por ningun otro modulo.

#include "TestHarness.h"

#include "Game/Morality.h"

#include <string>
#include <vector>

namespace {

// --- record / value ---

MOTOR_TEST_CASE(morality_starts_at_zero) {
    Morality m;
    MOTOR_CHECK(m.value() == 0);
}

MOTOR_TEST_CASE(record_positive_increments_value) {
    Morality m;
    m.record(15, "ayudar al aldeano");
    MOTOR_CHECK(m.value() == 15);
}

MOTOR_TEST_CASE(record_negative_decrements_value) {
    Morality m;
    m.record(-10, "subir el alquiler");
    MOTOR_CHECK(m.value() == -10);
}

MOTOR_TEST_CASE(record_zero_is_ignored) {
    Morality m;
    m.record(0, "accion neutra");
    MOTOR_CHECK(m.value() == 0);
    MOTOR_CHECK(m.history().empty());
}

MOTOR_TEST_CASE(clamps_to_kMax) {
    Morality m;
    m.record(200, "exceso de bondad");
    MOTOR_CHECK(m.value() == Morality::kMax);
}

MOTOR_TEST_CASE(clamps_to_kMin) {
    Morality m;
    m.record(-200, "exceso de maldad");
    MOTOR_CHECK(m.value() == Morality::kMin);
}

// --- label ---

// MORAL-01 (Oriol, 08/10/2026): siete tramos, simetricos en ±20, ±60 y ±75.
MOTOR_TEST_CASE(label_is_villano_at_minus_75_and_desgraciado_from_minus_60) {
    Morality m;
    m.record(-60, "");
    MOTOR_CHECK(std::string(m.label()) == "DESGRACIADO");
    m.record(-14, "");
    MOTOR_CHECK(std::string(m.label()) == "DESGRACIADO");
    m.record(-1, "");
    MOTOR_CHECK(std::string(m.label()) == "VILLANO");
}

MOTOR_TEST_CASE(label_is_canalla_between_minus_59_and_minus_20) {
    Morality m;
    m.record(-20, "");
    MOTOR_CHECK(std::string(m.label()) == "CANALLA");
    m.record(-39, "");
    MOTOR_CHECK(std::string(m.label()) == "CANALLA");
}

MOTOR_TEST_CASE(label_is_neutral_between_minus_19_and_19) {
    Morality m;
    m.record(-19, "");
    MOTOR_CHECK(std::string(m.label()) == "NEUTRAL");
    m.record(38, "");
    MOTOR_CHECK(std::string(m.label()) == "NEUTRAL");
}

MOTOR_TEST_CASE(label_is_respetado_from_20_and_salvador_from_60) {
    Morality m;
    m.record(20, "");
    MOTOR_CHECK(std::string(m.label()) == "RESPETADO");
    m.record(39, "");
    MOTOR_CHECK(std::string(m.label()) == "RESPETADO");
    m.record(1, "");
    MOTOR_CHECK(std::string(m.label()) == "SALVADOR");
    m.record(14, "");
    MOTOR_CHECK(std::string(m.label()) == "SALVADOR");
}

MOTOR_TEST_CASE(label_is_HEROE_at_75) {
    Morality m;
    m.record(75, "");
    MOTOR_CHECK(std::string(m.label()) == "HÉROE");
    m.record(25, "");
    MOTOR_CHECK(std::string(m.label()) == "HÉROE");
}

// --- fraction ---

MOTOR_TEST_CASE(fraction_zero_maps_to_half) {
    Morality m;
    MOTOR_CHECK(m.fraction() == 0.5f);
}

MOTOR_TEST_CASE(fraction_kMin_is_zero) {
    Morality m;
    m.record(Morality::kMin, "");
    MOTOR_CHECK(m.fraction() == 0.0f);
}

MOTOR_TEST_CASE(fraction_kMax_is_one) {
    Morality m;
    m.record(Morality::kMax, "");
    MOTOR_CHECK(m.fraction() == 1.0f);
}

// --- history ---

MOTOR_TEST_CASE(history_preserves_most_recent_first) {
    Morality m;
    m.record(5, "primero");
    m.record(-3, "segundo");
    MOTOR_CHECK(m.history().size() == 2);
    MOTOR_CHECK(m.history()[0].find("segundo") != std::string::npos);
    MOTOR_CHECK(m.history()[1].find("primero") != std::string::npos);
}

MOTOR_TEST_CASE(history_format_contains_sign_and_reason) {
    Morality m;
    m.record(7, "ayudar");
    MOTOR_CHECK(m.history()[0] == "+7  ayudar");
    m.record(-4, "robar");
    MOTOR_CHECK(m.history()[0] == "-4  robar");
}

MOTOR_TEST_CASE(history_is_capped_at_12) {
    Morality m;
    for (int i = 0; i < 20; ++i) {
        m.record(1, "motivo");
    }
    MOTOR_CHECK(m.history().size() == 12);
}

// --- setValue ---

MOTOR_TEST_CASE(setValue_overwrites_without_reason) {
    Morality m;
    m.record(50, "test");
    m.setValue(-30);
    MOTOR_CHECK(m.value() == -30);
    MOTOR_CHECK(m.history().size() == 1);  // record preserved, setValue adds nothing
}

MOTOR_TEST_CASE(setValue_clamps) {
    Morality m;
    m.setValue(500);
    MOTOR_CHECK(m.value() == Morality::kMax);
    m.setValue(-500);
    MOTOR_CHECK(m.value() == Morality::kMin);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
