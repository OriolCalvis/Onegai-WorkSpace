// H-PEL-1 — El pelo propio de la raza se puede volver a elegir, y el de sus razas germanas (misma cabeza:
// tools/pelo_germanos.py) tambien. Sin ventana (assets reales).
// H-PEL-2 — Los 12 mejores pelos de raza, como pentinados para todas las razas de talla medio, bajo y pequeno
// (tools/pelos_compartidos.py), tenibles; ninguna otra talla los ve. Mismo fichero: no hace falta otro objetivo.
#include "TestHarness.h"

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "Game/Personajes/EditorApariencia.h"
#include "Game/Personajes/ManifiestoPersonajes.h"
#include "Core/Imagen/ImagenRGBA.h"
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

// H-PEL-2: los 12 estilos compartidos, con el orden del manifiesto.
const std::vector<std::string>& compartidos() {
    static const std::vector<std::string> v = {
        "mono_mechones",     "pelo_voluminoso", "melena_salvaje", "peinado_atras", "pinchos_revueltos", "ondas_nobles",
        "rizos_cortos",      "despeinado",      "corto_y_cola_baja", "mechon_mojado", "melena_suelta",   "rizos_con_mono"};
    return v;
}

// Los peinados distintos que ofrece la fila Peinado (sin «sin pelo»), en el orden de salida.
std::vector<std::string> ofrecidos(Game::EditorApariencia& e) {
    marcar(e, Fila::Peinado);
    for (int i = 0; i < 90 && !e.apariencia().peloEstilo.empty(); ++i)
        e.izquierda();  // desde «sin pelo»: el primero que sale es el primero de la lista
    std::vector<std::string> out;
    for (int i = 0; i < 90; ++i) {
        e.derecha();
        const std::string id = e.apariencia().peloEstilo == "propio" ? "propio/" + e.apariencia().peloColor
                                                                       : e.apariencia().peloEstilo;
        if (!id.empty() && std::find(out.begin(), out.end(), id) == out.end())
            out.push_back(id);
    }
    return out;
}

// Si la fila `fila` (1-based) del atlas de pelo de `talla` tiene algun pixel opaco.
bool filaConDibujo(const std::string& talla, int fila) {
    static std::map<std::string, imagen::ImagenRGBA> cache;
    auto it = cache.find(talla);
    if (it == cache.end()) {
        auto r = imagen::cargarPNG("assets/textures/" + manifiesto().pelo.atlas.at(talla));
        if (!r.isOk())
            throw std::runtime_error(r.errorMessage());
        it = cache.emplace(talla, r.value()).first;
    }
    const imagen::ImagenRGBA& im = it->second;
    for (int y = (fila - 1) * 64; y < fila * 64 && y < im.alto; ++y)
        for (int x = 0; x < im.ancho; ++x)
            if (im.pixel(x, y)[3] != 0)
                return true;
    return false;
}

MOTOR_TEST_CASE(pelos_compartidos_las_tallas_medio_bajo_y_pequeno_ven_los_12_antes_de_los_de_siempre) {
    Game::EditorApariencia e(manifiesto());
    for (const char* raza : {"race_human_marches", "elfos_del_bosque", "enanos_forjadores", "medianos_urbanos"}) {
        e.empezar(raza);
        const std::vector<std::string> v = ofrecidos(e);
        std::size_t previo = 0;
        for (const std::string& c : compartidos()) {
            const auto it = std::find(v.begin(), v.end(), c);
            MOTOR_CHECK(it != v.end());
            if (it == v.end())
                continue;
            const std::size_t pos = static_cast<std::size_t>(it - v.begin());
            MOTOR_CHECK(pos >= previo);
            previo = pos;
            MOTOR_CHECK(pos < static_cast<std::size_t>(std::find(v.begin(), v.end(), "corto") - v.begin()));
        }
        // todo lo propio va antes que los 12; los de siempre (26) siguen estando
        for (std::size_t i = 0; i < v.size() && v[i].rfind("propio/", 0) == 0; ++i)
            MOTOR_CHECK(i < static_cast<std::size_t>(std::find(v.begin(), v.end(), compartidos()[0]) - v.begin()));
        MOTOR_CHECK(std::find(v.begin(), v.end(), "coleta") != v.end() && v.back() == "coleta");
    }
}

MOTOR_TEST_CASE(pelos_compartidos_una_raza_grande_no_ve_ninguno_y_una_naga_no_tiene_peinado) {
    Game::EditorApariencia e(manifiesto());
    e.empezar("orcos_del_consejo");  // talla grande: la cabeza no es la misma (G3-pelo.md, fase 2 con PixelLab)
    const std::vector<std::string> v = ofrecidos(e);
    for (const std::string& c : compartidos())
        MOTOR_CHECK(std::find(v.begin(), v.end(), c) == v.end());
    MOTOR_CHECK(std::find(v.begin(), v.end(), "corto") != v.end());
    e.empezar("nagas_arcanos");
    MOTOR_CHECK(!tiene(e, Fila::Peinado));
}

MOTOR_TEST_CASE(pelos_compartidos_ningun_peinado_ofrecido_tiene_la_fila_vacia_en_la_talla_de_la_raza) {
    Game::EditorApariencia e(manifiesto());
    int razas = 0;
    for (const Game::RazaVisual& r : manifiesto().razas) {
        e.empezar(r.id);
        if (!tiene(e, Fila::Peinado))
            continue;
        ++razas;
        for (const std::string& id : ofrecidos(e)) {
            if (id.rfind("propio/", 0) == 0)
                continue;
            const Game::FilaDeCapa* f = manifiesto().pelo.buscar(id + "/pelo_castano");
            MOTOR_CHECK(f != nullptr);
            if (f == nullptr)
                continue;
            MOTOR_CHECK(f->tallas.empty() || std::find(f->tallas.begin(), f->tallas.end(), r.talla) != f->tallas.end());
            MOTOR_CHECK(filaConDibujo(r.talla, f->fila));
        }
    }
    MOTOR_CHECK(razas > 40);
}

MOTOR_TEST_CASE(pelos_compartidos_se_tienen_con_el_motor_y_llevan_nombre) {
    Game::EditorApariencia e(manifiesto());
    e.empezar("race_human_marches");
    auto m = Game::MontajePersonajes::cargar("assets/textures");
    MOTOR_CHECK(m.isOk());
    if (!m.isOk())
        return;
    for (const std::string& c : compartidos()) {
        irA(e, c, "");
        MOTOR_CHECK(e.valor(Fila::Peinado) != "" && e.valor(Fila::Peinado).find('_') == std::string::npos);  // nombre, no id
        Game::Apariencia a = e.apariencia();
        a.peloColor = "pelo_castano";
        auto castano = m.value()->montar(a);
        a.peloColor = "pelo_rubio";
        auto rubio = m.value()->montar(a);
        a.peloEstilo.clear();
        auto calvo = m.value()->montar(a);
        MOTOR_CHECK(castano.isOk() && rubio.isOk() && calvo.isOk());
        if (!(castano.isOk() && rubio.isOk() && calvo.isOk()))
            continue;
        MOTOR_CHECK(castano.value().pixeles != calvo.value().pixeles);  // el peinado se ve
        MOTOR_CHECK(castano.value().pixeles != rubio.value().pixeles);  // y se tine
    }
    irA(e, "mono_mechones", "");
    MOTOR_CHECK(e.valor(Fila::Peinado) == "Moño con mechones");
}

int main() { return motor_test::runAllTests(); }
