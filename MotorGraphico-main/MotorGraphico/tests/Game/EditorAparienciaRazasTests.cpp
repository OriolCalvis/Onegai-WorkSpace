// CAR-1 — Pelo, barba y ojos genericos solo donde tienen sentido: minotauro, aarakocra, naga y espectro
// (cabeza de toro, ave, cobra o sin cuerpo) los llevan en «sin». Sin ventana (assets reales).
// CAR-2 — Los ojos vuelven en las razas cuya cabeza admite una capa de iris propia (personajes/ojos/): la fila
// «Ojos» y «Al azar» solo ahi; el motor monta la capa de la raza (o la de la talla con cuerpo liso).
#include "TestHarness.h"

#include <set>
#include <stdexcept>
#include <string>

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
        std::set<std::string> vistos;
        for (int i = 0; i < 50; ++i) {
            a.derecha();
            vistos.insert(a.apariencia().ojos);
            // CAR-2: solo la humana (que admite ojos) los cambia; la naga se queda con los suyos
            if (std::string(id) == "nagas_arcanos") {
                MOTOR_CHECK(a.apariencia().ojos == raza->ojos);
                MOTOR_CHECK(a.apariencia().peloEstilo.empty() && a.apariencia().barbaEstilo.empty());
            }
        }
        MOTOR_CHECK(std::string(id) == "nagas_arcanos" ? vistos.size() == 1 : vistos.size() > 3);
    }
}

// CAR-2: la fila Ojos sale exactamente en las razas que la admiten, y estas tienen capa propia de ojos.
MOTOR_TEST_CASE(apariencia_ojos_solo_en_razas_con_capa_propia) {
    const Game::ManifiestoPersonajes& m = manifiesto();
    int con = 0, sin = 0;
    for (const Game::RazaVisual& raza : m.razas) {
        Game::EditorApariencia e(m);
        e.empezar(raza.id);
        const bool tieneCapa = m.ojos.atlas.count(raza.id) != 0;
        MOTOR_CHECK(tiene(e, Fila::Ojos) == raza.admite("ojos"));
        if (raza.admite("ojos")) {
            MOTOR_CHECK(tieneCapa);
            ++con;
        } else {
            MOTOR_CHECK(!tieneCapa);
            ++sin;
        }
    }
    MOTOR_CHECK(con > 20 && sin > 10);
    // las razas de CAR-1 y alguna humanoide cuyo iris no se puede repintar siguen sin fila
    for (const char* id : {"minotauros_de_la_llanura", "aarakocra_de_las_montanas", "nagas_arcanos", "revenants",
                           "elementales_de_agua", "draconidos_de_hielo", "race_dwarf_deepforge"}) {
        Game::EditorApariencia e(m);
        e.empezar(id);
        MOTOR_CHECK(!tiene(e, Fila::Ojos));
    }
}

// CAR-2: el montaje pone la capa de ojos de la raza; con cuerpo liso (otra talla), la de la talla; una raza sin
// capa no pone ninguna (ni da error: los PNJ pueden traer «ojos» de antes).
MOTOR_TEST_CASE(montaje_elige_la_capa_de_ojos_de_la_raza_o_de_la_talla) {
    const Game::ManifiestoPersonajes& m = manifiesto();
    Game::MontajePersonajes montaje(m, "assets/textures");
    const auto ojosDe = [&](const Game::Apariencia& a) -> std::string {
        auto c = montaje.capas(a, {});
        if (!c.isOk())
            throw std::runtime_error(c.errorMessage());
        for (const Game::CapaDeMontaje& capa : c.value()) {
            if (capa.ranura == "ojos")
                return capa.atlas;
        }
        return "";
    };
    Game::Apariencia humana = Game::Apariencia::deRaza(*m.raza("race_human_marches"));
    MOTOR_CHECK(ojosDe(humana).empty());  // sus ojos: los del cuerpo
    humana.ojos = "rojo";
    MOTOR_CHECK(ojosDe(humana) == m.ojos.atlas.at("race_human_marches"));
    humana.talla = "bajo";  // cuerpo liso de otra talla: la capa de la talla
    MOTOR_CHECK(ojosDe(humana) == m.ojos.atlas.at("bajo"));
    Game::Apariencia revenant = Game::Apariencia::deRaza(*m.raza("revenants"));
    revenant.ojos = "rojo";
    MOTOR_CHECK(ojosDe(revenant).empty());
    Game::Apariencia naga = Game::Apariencia::deRaza(*m.raza("nagas_arcanos"));
    naga.ojos = "rojo";
    MOTOR_CHECK(ojosDe(naga).empty());
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

// CAR-2c: el cuerpo liso de talla grande no tiene capa de ojos (la mascara automatica no pasa `admite`): ni fila Ojos
// ni color de ojos que valga, aunque la raza humana si la tenga en las otras tallas.
MOTOR_TEST_CASE(apariencia_grande_con_cuerpo_liso_no_ofrece_ojos) {
    const Game::ManifiestoPersonajes& m = manifiesto();
    MOTOR_CHECK(m.ojos.atlas.count("grande") == 0 && m.ojos.atlas.count("bajo") == 1);
    Game::EditorApariencia e(m);
    e.empezar("race_human_marches");
    MOTOR_CHECK(tiene(e, Fila::Ojos));
    marcar(e, Fila::Ojos);
    for (int i = 0; i < 12 && e.apariencia().ojos != "rojo"; ++i)
        e.derecha();
    MOTOR_CHECK(e.apariencia().ojos == "rojo");
    marcar(e, Fila::Talla);
    for (int i = 0; i < 8 && e.apariencia().talla != "grande"; ++i)
        e.derecha();
    MOTOR_CHECK(e.apariencia().talla == "grande");
    MOTOR_CHECK(!tiene(e, Fila::Ojos));
    MOTOR_CHECK(e.apariencia().ojos.empty());  // el rojo de antes no se queda para el montaje
    for (int i = 0; i < 8 && e.apariencia().talla != "bajo"; ++i)
        e.izquierda();
    MOTOR_CHECK(e.apariencia().talla == "bajo" && tiene(e, Fila::Ojos));
}

// CAR-2c: aunque la apariencia traiga «ojos» (partida vieja, PNJ), montar grande con cuerpo liso no pinta nada: ni
// la capa de la talla «medio» ni ninguna otra. En una talla con capa (bajo), el rojo si cambia pixeles.
MOTOR_TEST_CASE(montaje_grande_con_cuerpo_liso_y_ojos_rojos_es_igual_que_sin_ojos) {
    const Game::ManifiestoPersonajes& m = manifiesto();
    Game::MontajePersonajes montaje(m, "assets/textures");
    const auto pixeles = [&](const std::string& talla, const std::string& ojos) {
        Game::Apariencia a = Game::Apariencia::deRaza(*m.raza("race_human_marches"));
        a.talla = talla;
        a.ojos = ojos;
        auto img = montaje.montar(a, {});
        if (!img.isOk())
            throw std::runtime_error(img.errorMessage());
        return img.value().pixeles;
    };
    const auto sin = pixeles("grande", "");
    MOTOR_CHECK(!sin.empty());
    MOTOR_CHECK(pixeles("grande", "rojo") == sin);
    MOTOR_CHECK(pixeles("bajo", "rojo") != pixeles("bajo", ""));
}

int main() { return motor_test::runAllTests(); }
