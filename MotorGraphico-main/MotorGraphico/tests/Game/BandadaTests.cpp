// BLOQUES-02 — test_bandada: la bandada sin OpenGL.
#include <cmath>
#include <vector>

#include "Game/Bandada.h"
#include "TestHarness.h"

using Game::Azotea;
using Game::Bandada;

namespace {
std::vector<Azotea> azoteasDePrueba() {
    std::vector<Azotea> a;
    for (int i = 0; i < 20; ++i) a.push_back({GridCoord{2 + i * 3, 4 + (i % 4) * 5}, 1 + i % 3});
    return a;
}
}  // namespace

MOTOR_TEST_CASE(sin_azoteas_no_hay_pajaros) {
    Bandada b;
    b.prepara({}, 7u, 10);
    MOTOR_CHECK(b.vacia());
    b.avanza(0.5f, GridCoord{0, 0});
    MOTOR_CHECK(b.pajaros().empty());
}

MOTOR_TEST_CASE(empiezan_posados_en_una_azotea_y_es_determinista) {
    Bandada b, c;
    b.prepara(azoteasDePrueba(), 99u, 8, 2);
    c.prepara(azoteasDePrueba(), 99u, 8, 2);
    MOTOR_CHECK(b.pajaros().size() == 8);
    for (std::size_t i = 0; i < 8; ++i) {
        const auto& p = b.pajaros()[i];
        MOTOR_CHECK(!p.volando);
        const Azotea& a = b.azoteas()[static_cast<std::size_t>(p.destino)];
        MOTOR_CHECK(std::fabs(p.x - (a.celda.x + 0.5f)) < 0.01f && std::fabs(p.y - (a.celda.y + 0.5f)) < 0.01f);
        MOTOR_CHECK(p.altura > 32.0f * a.plantas && p.altura < 32.0f * a.plantas + 16.0f);
        MOTOR_CHECK(p.especie == 0 || p.especie == 1);
        MOTOR_CHECK(p.destino == c.pajaros()[i].destino && p.especie == c.pajaros()[i].especie);
    }
}

MOTOR_TEST_CASE(el_jugador_los_espanta_y_vuelan_alto_hasta_otra_azotea) {
    Bandada b;
    b.prepara(azoteasDePrueba(), 5u, 1);
    const auto& p = b.pajaros()[0];
    const int origen = p.destino;
    const GridCoord lejos{60, 60};
    b.avanza(0.1f, lejos);
    MOTOR_CHECK(!p.volando);
    // El jugador se planta a su lado: despega hacia otra azotea.
    const GridCoord alLado{static_cast<int>(p.x), static_cast<int>(p.y) + 1};
    b.avanza(0.1f, alLado);
    MOTOR_CHECK(p.volando && p.destino != origen);
    // A mitad de vuelo va a la altura de crucero (mas alto que 3 plantas) y
    // bate las alas; al final aterriza en el destino.
    float maxAltura = 0.0f;
    bool batio = false;
    for (int i = 0; i < 400 && p.volando; ++i) {
        b.avanza(0.05f, lejos);
        maxAltura = std::max(maxAltura, p.altura);
        if (Bandada::frameDe(p) == 2) batio = true;
    }
    MOTOR_CHECK(!p.volando && batio);
    MOTOR_CHECK(std::fabs(maxAltura - Bandada::kAlturaVuelo) < 0.01f && maxAltura > 96.0f);
    const Azotea& a = b.azoteas()[static_cast<std::size_t>(p.destino)];
    MOTOR_CHECK(std::fabs(p.x - (a.celda.x + 0.5f)) < 0.01f && std::fabs(p.y - (a.celda.y + 0.5f)) < 0.01f);
    MOTOR_CHECK(Bandada::frameDe(p) == 0);
}

MOTOR_TEST_CASE(se_cansan_de_estar_posados) {
    Bandada b;
    b.prepara(azoteasDePrueba(), 11u, 4);
    const GridCoord lejos{60, 60};
    int despegues = 0;
    bool antes[4] = {false, false, false, false};
    for (int i = 0; i < 300; ++i) {
        b.avanza(0.1f, lejos);
        for (std::size_t k = 0; k < 4; ++k) {
            if (b.pajaros()[k].volando && !antes[k]) ++despegues;
            antes[k] = b.pajaros()[k].volando;
        }
    }
    MOTOR_CHECK(despegues >= 4);  // en 30 s todos han despegado al menos una vez
}

MOTOR_TEST_CASE(als_teulats_del_kit_es_posen_al_carener) {
    // MASC-01: un posador amb punt i alcada propis (el carener d'una casa del
    // kit) mana sobre el centre de la cel·la i les plantes.
    std::vector<Azotea> a = azoteasDePrueba();
    for (Azotea& t : a) {
        t.dx = 0.25f;
        t.dy = 1.0f;
        t.altura = 96.0f;
        t.orden = GridCoord{t.celda.x + 2, t.celda.y + 1};
    }
    Bandada b;
    b.prepara(a, 13u, 6);
    for (const auto& p : b.pajaros()) {
        const Azotea& t = b.azoteas()[static_cast<std::size_t>(p.destino)];
        MOTOR_CHECK(std::fabs(p.x - (t.celda.x + 0.25f)) < 1e-4f && std::fabs(p.y - (t.celda.y + 1.0f)) < 1e-4f);
        MOTOR_CHECK(std::fabs(p.altura - 96.0f) < 1e-4f);
    }
    // Despega, vola i aterra al carener d'un altre teulat.
    const GridCoord lluny{90, 90};
    const auto& p = b.pajaros()[0];
    b.avanza(0.1f, GridCoord{static_cast<int>(p.x), static_cast<int>(p.y)});
    MOTOR_CHECK(p.volando);
    for (int i = 0; i < 600 && p.volando; ++i) b.avanza(0.05f, lluny);
    MOTOR_CHECK(!p.volando && std::fabs(p.altura - 96.0f) < 1e-4f);
    const Azotea& t = b.azoteas()[static_cast<std::size_t>(p.destino)];
    MOTOR_CHECK(std::fabs(p.x - (t.celda.x + 0.25f)) < 1e-3f && std::fabs(p.y - (t.celda.y + 1.0f)) < 1e-3f);
}

int main() { return motor_test::runAllTests(); }
