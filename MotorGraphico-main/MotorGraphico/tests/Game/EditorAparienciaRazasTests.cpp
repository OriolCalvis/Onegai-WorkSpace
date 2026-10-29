// CAR-1 — Pelo, barba y ojos genericos solo donde tienen sentido: minotauro, aarakocra, naga y espectro
// (cabeza de toro, ave, cobra o sin cuerpo) los llevan en «sin». Sin ventana (assets reales).
#include "TestHarness.h"

#include <stdexcept>
#include <string>

#include "Game/Personajes/EditorApariencia.h"
#include "Game/Personajes/ManifiestoPersonajes.h"

namespace {

using Fila = Game::EditorApariencia::Fila;

const Game::ManifiestoPersonajes& manifiesto() {
    static const Game::ManifiestoPersonajes m = [] {
        auto r = Game::ManifiestoPersonajes::cargar("assets/textures");
        if (!r.isOk())
            throw std::runtime_error(r.errorMessage());
        return r.value();
    }();
    return m;
}

bool tiene(const Game::EditorApariencia& e, Fila f) {
    for (Fila x : e.filas()) {
        if (x == f)
            return true;
    }
    return false;
}

void marcar(Game::EditorApariencia& e, Fila f) {
    for (std::size_t i = 0; i < e.filas().size() && e.filaMarcada() != f; ++i)
        e.abajo();
    if (e.filaMarcada() != f)
        throw std::runtime_error("fila no disponible");
}

}  // namespace

MOTOR_TEST_CASE(apariencia_sin_pelo_barba_ni_ojos_en_cabezas_no_humanas) {
    Game::EditorApariencia e(manifiesto());
    for (const char* id : {"minotauros_de_la_llanura", "aarakocra_de_las_montanas", "nagas_arcanos",
                           "elementales_de_aire"}) {
        e.empezar(id);
        MOTOR_CHECK(!tiene(e, Fila::Peinado) && !tiene(e, Fila::ColorPelo));
        MOTOR_CHECK(!tiene(e, Fila::Barba) && !tiene(e, Fila::ColorBarba));
        MOTOR_CHECK(!tiene(e, Fila::Ojos));
    }
    e.empezar("race_human_marches");
    MOTOR_CHECK(tiene(e, Fila::Peinado) && tiene(e, Fila::Barba) && tiene(e, Fila::Ojos));
}

MOTOR_TEST_CASE(apariencia_al_azar_no_cambia_los_ojos_ni_pone_pelo_en_una_naga) {
    for (const char* id : {"race_human_marches", "nagas_arcanos"}) {
        const Game::RazaVisual* raza = manifiesto().raza(id);
        Game::EditorApariencia a(manifiesto());
        a.empezar(id);
        marcar(a, Fila::AlAzar);
        for (int i = 0; i < 50; ++i) {
            a.derecha();
            MOTOR_CHECK(a.apariencia().ojos == raza->ojos);
            if (std::string(id) == "nagas_arcanos")
                MOTOR_CHECK(a.apariencia().peloEstilo.empty() && a.apariencia().barbaEstilo.empty());
        }
    }
}

MOTOR_TEST_CASE(apariencia_de_raza_y_partida_vieja_ignoran_lo_que_la_raza_no_admite) {
    // deRaza no copia la barba de una raza que no la admite (la mutacion que quita la condicion lo notaria).
    Game::RazaVisual naga = *manifiesto().raza("nagas_arcanos");
    naga.barbaEstilo = "corta";
    naga.barbaColor = "pelo_castano";
    naga.peloEstilo = "corto";
    naga.peloColor = "pelo_castano";
    MOTOR_CHECK(Game::Apariencia::deRaza(naga).barbaEstilo.empty());
    MOTOR_CHECK(Game::Apariencia::deRaza(naga).peloEstilo.empty());
    Game::RazaVisual humana = *manifiesto().raza("race_human_marches");
    humana.barbaEstilo = "corta";
    humana.barbaColor = "pelo_castano";
    MOTOR_CHECK(Game::Apariencia::deRaza(humana).barbaEstilo == "corta");

    // Una partida vieja de una naga con pelo, barba y ojos: se limpian; en una humana, se respetan.
    Game::Apariencia vieja = Game::Apariencia::deRaza(*manifiesto().raza("nagas_arcanos"));
    vieja.peloEstilo = "corto";
    vieja.peloColor = "pelo_castano";
    vieja.barbaEstilo = "corta";
    vieja.barbaColor = "pelo_castano";
    vieja.ojos = "rojo";
    Game::Apariencia humano = vieja;
    vieja.limpiarSegunRaza(*manifiesto().raza("nagas_arcanos"));
    MOTOR_CHECK(vieja.peloEstilo.empty() && vieja.peloColor.empty());
    MOTOR_CHECK(vieja.barbaEstilo.empty() && vieja.barbaColor.empty());
    MOTOR_CHECK(vieja.ojos == manifiesto().raza("nagas_arcanos")->ojos);
    humano.limpiarSegunRaza(*manifiesto().raza("race_human_marches"));
    MOTOR_CHECK(humano.peloEstilo == "corto" && humano.barbaEstilo == "corta" && humano.ojos == "rojo");
}

int main() { return motor_test::runAllTests(); }
