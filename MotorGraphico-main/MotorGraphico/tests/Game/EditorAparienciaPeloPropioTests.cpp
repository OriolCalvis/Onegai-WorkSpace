// H-PEL-1 — El pelo propio de la raza se puede volver a elegir, y el de sus razas germanas (misma cabeza:
// tools/pelo_germanos.py) tambien. Sin ventana (assets reales).
#include "TestHarness.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "Game/Personajes/EditorApariencia.h"
#include "Game/Personajes/ManifiestoPersonajes.h"
#include "Game/Personajes/MontajePersonajes.h"

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

// Los pelos propios (la raza de origen) que ofrece la fila Peinado, en el orden en que salen.
std::vector<std::string> propios(Game::EditorApariencia& e) {
    marcar(e, Fila::Peinado);
    std::vector<std::string> out;
    for (int i = 0; i < 80; ++i) {
        e.derecha();
        if (e.apariencia().peloEstilo == "propio" &&
            std::find(out.begin(), out.end(), e.apariencia().peloColor) == out.end())
            out.push_back(e.apariencia().peloColor);
    }
    return out;
}

void irA(Game::EditorApariencia& e, const std::string& estilo, const std::string& color) {
    marcar(e, Fila::Peinado);
    for (int i = 0; i < 80 && !(e.apariencia().peloEstilo == estilo && (color.empty() || e.apariencia().peloColor == color)); ++i)
        e.derecha();
    if (e.apariencia().peloEstilo != estilo)
        throw std::runtime_error("peinado no ofrecido: " + estilo + "/" + color);
}

}  // namespace

MOTOR_TEST_CASE(pelo_propio_un_drow_ve_el_suyo_y_el_de_los_drows_que_encajan) {
    Game::EditorApariencia e(manifiesto());
    e.empezar("drow_explorador");
    const std::vector<std::string> v = propios(e);
    MOTOR_CHECK(v.size() == 3);
    MOTOR_CHECK(std::find(v.begin(), v.end(), "drow_explorador") != v.end());
    MOTOR_CHECK(std::find(v.begin(), v.end(), "drow_de_las_sombras") != v.end());
    MOTOR_CHECK(std::find(v.begin(), v.end(), "drow_del_abismo") != v.end());
    // no los de otro linaje, ni los que no encajan (cabeza mas baja), ni los yokai
    MOTOR_CHECK(std::find(v.begin(), v.end(), "elfos_del_bosque") == v.end());
    MOTOR_CHECK(std::find(v.begin(), v.end(), "drow_de_la_casa_noble") == v.end());
    MOTOR_CHECK(std::find(v.begin(), v.end(), "yokai_de_las_sombras") == v.end());
    // con su nombre
    irA(e, "propio", "drow_del_abismo");
    MOTOR_CHECK(e.valor(Fila::Peinado) == "Drow del Abismo");
    // el drow del abismo ve el del explorador
    e.empezar("drow_del_abismo");
    const std::vector<std::string> w = propios(e);
    MOTOR_CHECK(std::find(w.begin(), w.end(), "drow_explorador") != w.end());
}

MOTOR_TEST_CASE(pelo_propio_se_puede_volver_a_elegir_y_el_color_se_recupera) {
    Game::EditorApariencia e(manifiesto());
    e.empezar("drow_del_abismo");
    MOTOR_CHECK(e.apariencia().peloEstilo == "propio");
    MOTOR_CHECK(!tiene(e, Fila::ColorPelo));  // el propio lleva sus colores
    irA(e, "corto", "");
    MOTOR_CHECK(tiene(e, Fila::ColorPelo));
    marcar(e, Fila::ColorPelo);
    e.derecha();
    e.derecha();
    const std::string elegido = e.apariencia().peloColor;
    MOTOR_CHECK(!elegido.empty());
    irA(e, "propio", "drow_del_abismo");  // vuelve al suyo
    MOTOR_CHECK(e.apariencia().peloColor == "drow_del_abismo" && !tiene(e, Fila::ColorPelo));
    irA(e, "largo", "");
    MOTOR_CHECK(e.apariencia().peloColor == elegido);  // y al generico, con el color que tenia
    MOTOR_CHECK(tiene(e, Fila::ColorPelo));
    // se monta
    irA(e, "propio", "drow_explorador");
    auto m = Game::MontajePersonajes::cargar("assets/textures");
    MOTOR_CHECK(m.isOk());
    if (m.isOk())
        MOTOR_CHECK(m.value()->capas(e.apariencia()).isOk());
}

MOTOR_TEST_CASE(pelo_propio_una_raza_sin_germanas_ve_el_suyo_y_un_minotauro_nada) {
    Game::EditorApariencia e(manifiesto());
    e.empezar("alquimistas_de_aegroum");
    const std::vector<std::string> v = propios(e);
    MOTOR_CHECK(v.size() == 1 && v[0] == "alquimistas_de_aegroum");
    e.empezar("trasgos_de_gliaddokx");
    const std::vector<std::string> t = propios(e);
    MOTOR_CHECK(t.size() == 1 && t[0] == "trasgos_de_gliaddokx");
    e.empezar("minotauros_del_laberinto");
    MOTOR_CHECK(!tiene(e, Fila::Peinado) && !tiene(e, Fila::ColorPelo));
    MOTOR_CHECK(e.apariencia().peloEstilo.empty());
}

MOTOR_TEST_CASE(pelo_propio_al_azar_puede_sacar_el_de_la_raza_y_el_de_las_germanas) {
    Game::EditorApariencia e(manifiesto());
    e.empezar("drow_explorador");
    marcar(e, Fila::AlAzar);
    std::set<std::string> vistos;
    int genericos = 0;
    for (int i = 0; i < 400; ++i) {
        e.derecha();
        if (e.apariencia().peloEstilo == "propio")
            vistos.insert(e.apariencia().peloColor);
        else if (!e.apariencia().peloEstilo.empty())
            ++genericos;
    }
    MOTOR_CHECK(vistos.count("drow_explorador") == 1 && vistos.count("drow_de_las_sombras") == 1 &&
                vistos.count("drow_del_abismo") == 1);
    MOTOR_CHECK(vistos.size() == 3 && genericos > 100);
}

int main() { return motor_test::runAllTests(); }
