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

// E-MATRIZ-01b — El oro, la moral, el nivel y la mochila de cada partida. Los trae el guardado (el arnes los apunta y
// fabricar_guardados los vuelca por las piezas del juego) y los restaura restoreSaveGame; la tabla de caminos dice cuanto
// (el oro empieza en 150: la bolsa inicial). Mutacion: una copia del JSON a la que se le quita el cobro, o se le cambia
// el acto de la decision, tiene que dar otro oro u otra moral.
bool empiezaPor(const std::string& texto, const char* cabeza) { return texto.rfind(cabeza, 0) == 0; }

bool algunoEmpiezaPor(const std::vector<std::string>& fallos, const char* cabeza) {
    return std::any_of(fallos.begin(), fallos.end(), [&](const std::string& f) { return empiezaPor(f, cabeza); });
}

MOTOR_TEST_CASE(cada_camino_trae_su_oro_su_moral_su_nivel_y_su_mochila) {
    Fabrica f;
    for (const CaminoDia1& c : caminosDia1()) {
        const SaveGameData d = dia1(f.g, f.heroe, c);
        // Lo que se escribe en el fichero (no solo lo que sale de restaurar).
        MOTOR_CHECK(d.gold == c.cuenta.oro);
        MOTOR_CHECK(d.moralityValue == c.cuenta.moral);
        MOTOR_CHECK(d.playerLevel == c.cuenta.nivel && d.playerLevelProgress == c.cuenta.progreso);
        MOTOR_CHECK(d.moral.pendientes.empty());  // la noche del cierre asienta lo del dia
        Restaurada r = restaura(f.g, f.heroe, d, std::string(c.id) + "_cuentas.json");
        MOTOR_CHECK(r.nivel->sesion->gold() == c.cuenta.oro);
        MOTOR_CHECK(r.nivel->sesion->morality().value() == c.cuenta.moral);
        MOTOR_CHECK(r.nivel->sesion->morality().tramo() == moral::tramoDe(c.cuenta.moral));
    }
}

MOTOR_TEST_CASE(sin_su_cobro_el_oro_del_camino_falla) {
    Fabrica f;
    for (const CaminoDia1& c : caminosDia1()) {
        const std::string temporal = (std::filesystem::temp_directory_path() / (std::string("matriz_oro_") + c.id + ".json")).string();
        const demo_bnd::Guiones mutada = conMutacion(f.g, c.mutOro, temporal);
        const std::vector<std::string> fallos = juegaYComprueba(mutada, f.heroe, c, nullptr, true);
        if (!algunoEmpiezaPor(fallos, "oro=")) std::fprintf(stderr, "  [%s] quitar el oro (%s / %s) no cambio el oro\n", c.id, c.mutOro.beat, c.mutOro.opcion);
        MOTOR_CHECK(algunoEmpiezaPor(fallos, "oro="));
    }
}

MOTOR_TEST_CASE(con_otro_acto_la_moral_del_camino_falla) {
    Fabrica f;
    int probados = 0;
    for (const CaminoDia1& c : caminosDia1()) {
        if (std::string(c.mutMoral.beat).empty()) {  // sin ningun acto en el camino (la pelea no corrompe): nada que mover
            MOTOR_CHECK(c.cuenta.moral == 0);
            continue;
        }
        ++probados;
        const std::string temporal = (std::filesystem::temp_directory_path() / (std::string("matriz_moral_") + c.id + ".json")).string();
        const demo_bnd::Guiones mutada = conMutacion(f.g, c.mutMoral, temporal);
        const std::vector<std::string> fallos = juegaYComprueba(mutada, f.heroe, c, nullptr, true);
        if (!algunoEmpiezaPor(fallos, "moral=")) std::fprintf(stderr, "  [%s] cambiar el acto (%s / %s / %s) no cambio la moral\n", c.id, c.mutMoral.beat, c.mutMoral.opcion, c.mutMoral.cadena);
        MOTOR_CHECK(algunoEmpiezaPor(fallos, "moral="));
    }
    MOTOR_CHECK(probados >= 14);  // todos menos los de moral 0 sin acto (la pelea, la pifia de Aigren)
}

int main() { return motor_test::runAllTests(); }
