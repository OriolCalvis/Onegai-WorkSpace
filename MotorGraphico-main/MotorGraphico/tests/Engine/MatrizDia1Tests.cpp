// E-MATRIZ-01 — Un guardado y una prueba por camino del dia 1 (examples/fabricar_guardados.h, CaminoDia1).
//
// Cada camino juega el prologo y el dia 1 DE VERDAD con el arnes de las demos hasta cerrar el dia, fotografia la
// partida, la escribe y la lee con el lector del juego, la restaura en una sesion limpia sobre el nivel real y
// juega el inicio del dia siguiente (Gorran). Comprueba lo que la matriz (tools/matriz_decisiones.py) dice que la
// decision deja: sus flags y variables, que sigan en pie tras amanecer, y que los beats de los dias posteriores que
// las leen se vean (o no) con lo que trae ese guardado.
// Mutacion: la MISMA comprobacion sobre un guardado fabricado con una copia del JSON a la que se le quita el efecto
// de la opcion (temporal, el original no se toca) tiene que fallar.
// Sin GL ni ventana.
#include <algorithm>
#include <filesystem>
#include <string>

#include "TestHarness.h"
#include "fabricar_guardados.h"

namespace {

using namespace fabricar;
using demo_bnd::Suerte;  // (los caminos viven en fabricar_guardados.h: caminosDia1())

struct Fabrica {
    demo_bnd::Guiones g = demo_bnd::cargaTodo();
    Heroe heroe = creaHeroe();
};

std::vector<std::string> juegaYComprueba(const demo_bnd::Guiones& g, const Heroe& h, const CaminoDia1& c, std::vector<std::string>* jugado = nullptr,
                                         bool mutada = false) {
    const SaveGameData d = dia1(g, h, c, jugado, !mutada);
    Restaurada r = restaura(g, h, d, std::string(c.id) + ".json");
    return compruebaDia1(g, r, c);
}

}  // namespace

MOTOR_TEST_CASE(cada_camino_del_dia1_deja_lo_que_dice_la_matriz) {
    Fabrica f;
    for (const CaminoDia1& c : caminosDia1()) {
        std::vector<std::string> jugado;
        const std::vector<std::string> fallos = juegaYComprueba(f.g, f.heroe, c, &jugado);
        for (const std::string& x : fallos) std::fprintf(stderr, "  [%s] %s\n", c.id, x.c_str());
        MOTOR_CHECK(fallos.empty());
    }
}

MOTOR_TEST_CASE(sin_el_efecto_de_la_opcion_el_camino_falla) {
    Fabrica f;
    for (const CaminoDia1& c : caminosDia1()) {
        const std::string temporal = (std::filesystem::temp_directory_path() / (std::string("matriz_mut_") + c.id + ".json")).string();
        const demo_bnd::Guiones mutada = conMutacion(f.g, c.mutacion, temporal);
        const std::vector<std::string> fallos = juegaYComprueba(mutada, f.heroe, c, nullptr, true);
        if (fallos.empty()) std::fprintf(stderr, "  [%s] la mutacion (%s / %s / %s) no hizo fallar nada\n", c.id, c.mutacion.beat, c.mutacion.opcion, c.mutacion.cadena);
        else if (std::getenv("MATRIZ_VERBOSO")) std::fprintf(stderr, "  [%s] mutacion en rojo: %s%s\n", c.id, fallos.front().c_str(), fallos.size() > 1 ? " (+mas)" : "");
        MOTOR_CHECK(!fallos.empty());
    }
}

int main() { return motor_test::runAllTests(); }
