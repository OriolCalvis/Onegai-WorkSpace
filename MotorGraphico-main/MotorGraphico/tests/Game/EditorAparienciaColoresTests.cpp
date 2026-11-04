// NET-E1a — El color del pelo y de la barba en el creador, con los atlas de siempre y con los recortados a
// la fila castana (los demas colores salen tenyendo esa fila): se conserva al cambiar de peinado o de barba,
// la barba sigue al pelo y los colores salen siempre en el mismo orden, sin que falte ninguno.
#include "TestHarness.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include "Game/Personajes/EditorApariencia.h"

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

void marcar(Game::EditorApariencia& e, Fila f) {
    for (std::size_t i = 0; i < e.filas().size() && e.filaMarcada() != f; ++i)
        e.abajo();
    if (e.filaMarcada() != f)
        throw std::runtime_error("fila no disponible");
}

const char* const kOrdenColores[] = {"pelo_castano",     "pelo_rubio",       "pelo_negro",      "pelo_pelirrojo",
                                     "pelo_oscuro",      "pelo_blanco",      "pelo_gris",       "pelo_plata_lila",
                                     "pelo_verde_musgo", "pelo_azul_hielo", "pelo_llama"};

// NET-E1a: el manifiesto con los atlas de pelo y barba recortados a la fila castana (lo que dejara NET-E1b):
// los demas colores salen tenyendo esa fila. Mismas filas con su numero, sin las de color.
const Game::ManifiestoPersonajes& manifiestoRecortado() {
    static const Game::ManifiestoPersonajes m = [] {
        Game::ManifiestoPersonajes r = manifiesto();
        for (Game::GrupoDeCapas* g : {&r.pelo, &r.barba}) {
            const Game::GrupoDeCapas completo = *g;
            g->filas.erase(std::remove_if(g->filas.begin(), g->filas.end(),
                                          [&](const Game::FilaDeCapa& f) {
                                              return f.color != "pelo_castano" &&
                                                     completo.buscar(f.estilo + "/pelo_castano") != nullptr;
                                          }),
                           g->filas.end());
        }
        return r;
    }();
    return m;
}

// Mismo recorrido con las dos fuentes: se elige un peinado y una barba del atlas, un color que NO es el
// primero, se cambia de peinado y de barba y el color se queda; y la barba que iba a juego sigue al pelo.
void irA(Game::EditorApariencia& e, Fila fila, const std::string& (*campo)(const Game::Apariencia&),
         const std::string& valor) {
    marcar(e, fila);
    for (int i = 0; i < 60 && campo(e.apariencia()) != valor; ++i)
        e.derecha();
    MOTOR_CHECK(campo(e.apariencia()) == valor);
}
const std::string& campoPeinado(const Game::Apariencia& a) { return a.peloEstilo; }
const std::string& campoBarba(const Game::Apariencia& a) { return a.barbaEstilo; }
const std::string& campoColorPelo(const Game::Apariencia& a) { return a.peloColor; }
const std::string& campoColorBarba(const Game::Apariencia& a) { return a.barbaColor; }

void colorSeConserva(const Game::ManifiestoPersonajes& man) {
    Game::EditorApariencia e(man);
    e.empezar("race_human_marches");
    irA(e, Fila::Peinado, campoPeinado, "corto");
    irA(e, Fila::Barba, campoBarba, "BARBA_CORTA");
    irA(e, Fila::ColorPelo, campoColorPelo, "pelo_blanco");
    MOTOR_CHECK(e.apariencia().peloColor == "pelo_blanco");
    // Al cambiar de peinado y de barba, los colores se conservan.
    irA(e, Fila::Peinado, campoPeinado, "largo");
    MOTOR_CHECK(e.apariencia().peloColor == "pelo_blanco");
    irA(e, Fila::Barba, campoBarba, "BARBA_LARGA");
    MOTOR_CHECK(!e.apariencia().barbaColor.empty());
    // La barba que iba a juego sigue al pelo; con colores distintos, cada uno por su lado.
    irA(e, Fila::ColorBarba, campoColorBarba, "pelo_blanco");
    irA(e, Fila::ColorPelo, campoColorPelo, "pelo_negro");
    MOTOR_CHECK(e.apariencia().barbaColor == "pelo_negro");
    irA(e, Fila::ColorBarba, campoColorBarba, "pelo_rubio");
    irA(e, Fila::ColorPelo, campoColorPelo, "pelo_gris");
    MOTOR_CHECK(e.apariencia().barbaColor == "pelo_rubio");
    irA(e, Fila::Barba, campoBarba, "BARBA");
    MOTOR_CHECK(e.apariencia().barbaColor == "pelo_rubio");
    // Los colores salen en el orden de siempre (el castano, el primero).
    irA(e, Fila::ColorPelo, campoColorPelo, "pelo_castano");
    const char* const orden[] = {"pelo_castano",    "pelo_rubio",       "pelo_negro",      "pelo_pelirrojo",
                                 "pelo_oscuro",     "pelo_blanco",      "pelo_gris",       "pelo_plata_lila",
                                 "pelo_verde_musgo", "pelo_azul_hielo", "pelo_llama"};
    for (const char* color : orden) {
        MOTOR_CHECK(e.apariencia().peloColor == color);
        e.derecha();
    }
    MOTOR_CHECK(e.apariencia().peloColor == "pelo_castano");  // y da la vuelta
}


// Un estilo real de siete colores en el atlas (rizado, PERILLA): salen todos los de la paleta, en el
// orden de kOrden, igual con el atlas completo que con el recortado.
void ordenDeUnEstilo(const Game::ManifiestoPersonajes& man) {
    Game::EditorApariencia e(man);
    e.empezar("race_human_marches");
    irA(e, Fila::Peinado, campoPeinado, "rizado");
    irA(e, Fila::Barba, campoBarba, "PERILLA");
    for (Fila fila : {Fila::ColorPelo, Fila::ColorBarba}) {
        const auto campo = fila == Fila::ColorPelo ? campoColorPelo : campoColorBarba;
        irA(e, fila, campo, "pelo_castano");
        std::vector<std::string> vistos;
        for (std::size_t i = 0; i < std::size(kOrdenColores) + 1; ++i) {
            vistos.push_back(campo(e.apariencia()));
            e.derecha();
        }
        MOTOR_CHECK(vistos.size() == std::size(kOrdenColores) + 1);
        for (std::size_t i = 0; i < std::size(kOrdenColores) && i < vistos.size(); ++i)
            MOTOR_CHECK(vistos[i] == kOrdenColores[i]);
        MOTOR_CHECK(vistos.back() == "pelo_castano");  // da la vuelta
        for (const auto& [material, tonos] : man.paletas) {
            (void)tonos;
            if (material.rfind("pelo_", 0) == 0)
                MOTOR_CHECK(std::find(vistos.begin(), vistos.end(), material) != vistos.end());
        }
    }
}

}  // namespace

// NET-E1a: con los atlas de siempre y con los de pelo y barba recortados a la fila castana.
MOTOR_TEST_CASE(apariencia_el_color_se_conserva_al_cambiar_de_peinado) {
    colorSeConserva(manifiesto());
    colorSeConserva(manifiestoRecortado());
}

MOTOR_TEST_CASE(apariencia_el_orden_de_los_colores_es_siempre_el_mismo) {
    ordenDeUnEstilo(manifiesto());
    ordenDeUnEstilo(manifiestoRecortado());
}


int main() { return motor_test::runAllTests(); }
