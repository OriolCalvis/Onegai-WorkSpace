// AV14-02 — Lo que hace cada golpe en el cuerpo y en tu hoja (Game::EfectosDeGolpe).
#include "Game/EfectosDeGolpe.h"
#include "TestHarness.h"

#include <cmath>

namespace {

Game::GolpeCombate golpe(bool aliado, std::size_t i, Game::GolpeCombate::Grado g, int cambio) {
    Game::GolpeCombate x;
    x.aAliado = aliado;
    x.i = i;
    x.grado = g;
    x.cambio = cambio;
    return x;
}

}  // namespace

MOTOR_TEST_CASE(el_acierto_parpadea_en_carmin_y_retrocede_alejandose_de_quien_pega) {
    Game::EfectosDeGolpe e;
    e.golpe(golpe(false, 1, Game::GolpeCombate::Grado::Acierto, -5), Vector2{1.0f, 0.0f});
    MOTOR_CHECK(e.activo(false, 1) && !e.activo(false, 0));
    MOTOR_CHECK(e.tinte(false, 1).y < 0.5f);  // encendido: carmin
    e.avanza(0.075f);                          // el primer cuarto: lo mas lejos
    MOTOR_CHECK(e.desplazamiento(false, 1).x > 4.0f);
    MOTOR_CHECK(e.tinte(false, 1).y == 1.0f);  // apagado (60-120 ms)
    e.avanza(0.3f);
    MOTOR_CHECK(!e.activo(false, 1));
    MOTOR_CHECK(e.desplazamiento(false, 1).x == 0.0f && e.tinte(false, 1).x == 1.0f);
    MOTOR_CHECK(e.hoja() == 0.0f && e.pausa() == 0.0f);  // no era a ti, ni critico
}

MOTOR_TEST_CASE(el_fallo_se_aparta_de_lado_y_no_parpadea) {
    Game::EfectosDeGolpe e;
    e.golpe(golpe(false, 0, Game::GolpeCombate::Grado::Fallo, 0), Vector2{1.0f, 0.0f});
    e.avanza(0.07f);
    MOTOR_CHECK(e.desplazamiento(false, 0).x == 0.0f && std::abs(e.desplazamiento(false, 0).y) > 2.0f);
    MOTOR_CHECK(e.tinte(false, 0).y == 1.0f);
}

MOTOR_TEST_CASE(el_critico_para_el_mundo_70_ms_y_empuja_mas) {
    Game::EfectosDeGolpe e, a;
    e.golpe(golpe(false, 0, Game::GolpeCombate::Grado::Critico, -12), Vector2{0.0f, 1.0f});
    a.golpe(golpe(false, 0, Game::GolpeCombate::Grado::Acierto, -5), Vector2{0.0f, 1.0f});
    MOTOR_CHECK(e.pausaMundo(0.05f) && e.pausaMundo(0.05f) && !e.pausaMundo(0.05f));
    e.avanza(0.11f);
    a.avanza(0.075f);
    MOTOR_CHECK(e.desplazamiento(false, 0).y > a.desplazamiento(false, 0).y);
}

MOTOR_TEST_CASE(si_te_pegan_a_ti_tu_hoja_se_tine_tiembla_y_vuelve) {
    Game::EfectosDeGolpe e;
    e.golpe(golpe(true, 0, Game::GolpeCombate::Grado::Roce, -1), Vector2{}, 0.1f);
    MOTOR_CHECK(e.hoja() >= 0.35f);  // hasta un roce se nota
    e.avanza(0.02f);
    const Vector2 t = e.temblorHoja();
    MOTOR_CHECK(t.x != 0.0f || t.y != 0.0f);
    e.avanza(1.0f);
    MOTOR_CHECK(e.hoja() == 0.0f);
    MOTOR_CHECK(e.temblorHoja().x == 0.0f && e.temblorHoja().y == 0.0f);
    // Una cura sobre ti no tine la hoja: un pulso verde en el cuerpo.
    e.golpe(golpe(true, 0, Game::GolpeCombate::Grado::Cura, 4), Vector2{});
    e.avanza(0.25f);
    MOTOR_CHECK(e.hoja() == 0.0f && e.tinte(true, 0).x < 1.0f && e.tinte(true, 0).y == 1.0f);
}

int main() { return motor_test::runAllTests(); }
