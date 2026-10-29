// APARIENCIA-01 — El motor monta los personajes por capas igual que el
// probador de tools/ (ARTE-02), sin ventana.
//
// Casos (assets reales de assets/textures, copiados a build/ por
// copiar_assets):
//   1. El manifiesto se lee entero: 43 razas, 55 peinados, 14 barbas, 123
//      piezas, 19 personajes montados y la pieza de cada objeto.
//   2. La mezcla es la de Pillow (Image.alpha_composite) en 40 pares de
//      pixeles con alfa parcial, sacados de Pillow 12.
//   3. Los personajes horneados (actores_capas, razas_andando,
//      personajes_historia y, desde PNJ-01, todos los PNJ de las hojas pnj_*)
//      salen IGUALES, byte a byte, montados en C++ desde su "composicion":
//      es la prueba de que el motor y el probador hacen lo mismo, y de que
//      cada PNJ esta hecho con las piezas que monta el jugador.
//   4. El orden de las capas es el del probador en los casos raros: la naga
//      del rio (quita capas y pinta pelo en una raza sin pelo), otra piel en
//      un humano (cuerpo liso + ojos), un yelmo cerrado que tapa el pelo.
//   5. Lo que no existe es un Error legible, no un personaje a medias.
//   6. La cache respeta su presupuesto y sigue montando bien.
//   7. El reparto de la hoja: ciclos de andar que empiezan quietos, el
//      reposo (ESTETICA-01: 13-20, dos por rumbo) y la vista previa que anda,
//      descansa y gira.
//   8. ESTETICA-01: una pieza con tinte ("f_daga:hierro>plaga,...") es su
//      fila con los tonos de cada material cambiados por los de otro, y nada
//      mas; y todos los objetos del catalogo se montan con su estetica.
//   9. SP-02: una fila con su propio orden de dibujo (los guanteletes del
//      Artista Marcial, uno en cada mano) va encima del cuerpo tambien de
//      espaldas, donde un arma normal va detras.
#include "TestHarness.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include "Core/Imagen/ImagenRGBA.h"
#include "Core/Json/JsonValue.h"
#include "Game/Personajes/MontajePersonajes.h"
#include "Game/Personajes/Vestuario.h"

namespace {

const char* const kTexturas = "assets/textures";

std::unique_ptr<Game::MontajePersonajes> montaje(
    std::size_t presupuesto = Game::MontajePersonajes::kPresupuestoPorDefecto) {
    auto m = Game::ManifiestoPersonajes::cargar(kTexturas);
    if (!m.isOk())
        throw std::runtime_error(m.errorMessage());
    return std::make_unique<Game::MontajePersonajes>(std::move(m.value()), kTexturas, presupuesto);
}

JsonValue leerJson(const std::string& ruta) {
    std::ifstream in(ruta, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    auto parsed = JsonValue::parse(ss.str());
    if (!parsed.isOk())
        throw std::runtime_error(ruta + ": " + parsed.errorMessage());
    return parsed.value();
}

// Cuantos bytes difieren entre la hoja montada y la fila `fila` del atlas.
int diferencias(const imagen::ImagenRGBA& hoja, const imagen::ImagenRGBA& atlas, int fila,
                std::set<int>* frames = nullptr) {
    int distintos = 0;
    const int y0 = (fila - 1) * 64;
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < std::min(hoja.ancho, atlas.ancho); ++x) {
            const std::uint8_t* a = hoja.pixel(x, y);
            const std::uint8_t* b = atlas.pixel(x, y0 + y);
            for (int c = 0; c < 4; ++c) {
                distintos += a[c] != b[c] ? 1 : 0;
                if (a[c] != b[c] && frames != nullptr) frames->insert(x / 64 + 1);
            }
        }
    }
    return distintos;
}

std::vector<std::string> ranuras(const std::vector<Game::CapaDeMontaje>& capas) {
    std::vector<std::string> out;
    for (const auto& c : capas)
        out.push_back(c.ranura);
    return out;
}

}  // namespace

MOTOR_TEST_CASE(apariencia_manifiesto_entero) {
    auto r = Game::ManifiestoPersonajes::cargar(kTexturas);
    MOTOR_CHECK(r.isOk());
    const Game::ManifiestoPersonajes& m = r.value();
    MOTOR_CHECK(m.razas.size() == 72);  // DIALOGOS-01: + dos de Ayashii; + 27 del 2000 b.f.
    // PL-ROPA-01: 27 peinados y 12 barbas por colores, mas el pelo propio de cada raza sobre familia
    MOTOR_CHECK(m.pelo.filas.size() >= 57);
    MOTOR_CHECK(m.barba.filas.size() >= 14);
    // EDITOR-PERSONALIZACION-01: las marcas de la piel y las pieles de cada talla
    MOTOR_CHECK(m.marcas.filas.size() == 11);
    MOTOR_CHECK(m.pieles.count("medio") == 1 && m.pieles.at("medio").opciones.count("race_human_marches") == 1 &&
                m.pieles.at("medio").opciones.count("piel_oscura") == 1);
    MOTOR_CHECK(m.equipo.count("alas") == 1 && m.equipo.count("cola") == 1 && m.equipo.count("cuernos") == 1);
    MOTOR_CHECK(m.cuerpos.filas.size() == 9);
    MOTOR_CHECK(m.ojos.filas.size() == 10);
    std::size_t piezas = 0;
    for (const auto& [ranura, grupo] : m.equipo)
        piezas += grupo.filas.size();
    // 123 del generador + las de PixelLab (PIXELLAB-L7, importar_prendas.py).
    MOTOR_CHECK(piezas >= 123 + 11);
    MOTOR_CHECK(m.personajes.size() == 19);
    // PNJ-01: las prendas de oficio estan en los atlas, marcadas solo para PNJ.
    MOTOR_CHECK(m.equipo.at("torso").buscar("librea_guardia") != nullptr);
    MOTOR_CHECK(m.equipo.at("torso").buscar("librea_guardia")->soloPnj);
    MOTOR_CHECK(m.equipo.at("cabeza").buscar("birrete")->soloPnj);
    MOTOR_CHECK(!m.equipo.at("torso").buscar("vestido_gris")->soloPnj);
    MOTOR_CHECK(m.piezaDeObjeto.size() > 380);
    MOTOR_CHECK(m.orden.at("capa").at("arriba") > m.orden.at("cuerpo").at("arriba"));
    MOTOR_CHECK(m.orden.at("capa").at("abajo") < m.orden.at("cuerpo").at("abajo"));
    // Humanos: otra piel y otra talla; espectros y revenants no (su piel no
    // esta en el atlas de cuerpos); un minotauro tampoco (rasgos propios).
    MOTOR_CHECK(m.raza("race_human_marches")->cuerpoLiso);
    MOTOR_CHECK(!m.raza("espectros")->cuerpoLiso);
    MOTOR_CHECK(!m.raza("revenants")->cuerpoLiso);
    MOTOR_CHECK(!m.raza("minotauros_de_la_llanura")->cuerpoLiso);
    int lisas = 0;
    for (const auto& raza : m.razas)
        lisas += raza.cuerpoLiso ? 1 : 0;
    // PL-JUEGO-01: con PixelLab muchos traen su cabeza y su pelo encima de la familia («rasgos»), pero los
    // que no tienen rasgos propios en el catalogo siguen admitiendo otra piel («liso» en personajes.json),
    // la misma regla que el horneador (actores_personajes.cambia_cuerpo). Antes 19: los de marcas en la
    // piel (navegantes, cruzados, archivistas, enanos de obsidiana e historiadores, genasi y nobles de
    // Ascaria) la perdian al cambiar de piel; ahora la conservan y no cambian de piel.
    MOTOR_CHECK(lisas == 12);
    // PL-ROPA-01: el pelo que PixelLab pinto a los enanos forjadores ya no va en «rasgos» (iba a trozos):
    // va en la capa de pelo («propio/enanos_forjadores»), y «rasgos» queda vacia y no se lista.
    MOTOR_CHECK(m.raza("enanos_forjadores")->capas.size() == 1 && m.raza("enanos_forjadores")->cuerpoLiso);
}

MOTOR_TEST_CASE(apariencia_mezcla_como_pillow) {
    // {debajo}, {encima}, {lo que da Image.alpha_composite(debajo, encima)}
    const std::uint8_t casos[][3][4] = {
        {{2, 0, 1, 0}, {0, 255, 1, 1}, {0, 255, 1, 1}},
        {{0, 127, 255, 127}, {2, 255, 1, 2}, {0, 129, 251, 128}},
        {{127, 1, 0, 105}, {254, 238, 128, 2}, {129, 5, 2, 106}},
        {{1, 153, 175, 1}, {255, 254, 77, 0}, {1, 153, 175, 1}},
        {{254, 254, 254, 1}, {138, 0, 158, 255}, {138, 0, 158, 255}},
        {{0, 254, 1, 0}, {128, 127, 255, 1}, {128, 127, 255, 1}},
        {{85, 128, 255, 255}, {255, 2, 2, 127}, {170, 65, 129, 255}},
        {{0, 2, 128, 2}, {254, 2, 27, 255}, {254, 2, 27, 255}},
        {{255, 53, 0, 1}, {106, 1, 0, 0}, {255, 53, 0, 1}},
        {{1, 0, 127, 2}, {254, 186, 1, 249}, {254, 186, 1, 249}},
        {{245, 1, 1, 128}, {2, 127, 2, 128}, {83, 85, 2, 192}},
        {{128, 2, 127, 127}, {127, 127, 102, 0}, {128, 2, 127, 127}},
        {{128, 128, 254, 254}, {1, 1, 116, 254}, {1, 1, 117, 255}},
        {{104, 0, 1, 255}, {102, 255, 1, 202}, {102, 202, 1, 255}},
        {{1, 2, 0, 77}, {74, 2, 0, 1}, {2, 2, 0, 78}},
        {{255, 127, 128, 128}, {254, 255, 0, 181}, {254, 233, 22, 218}},
        {{2, 0, 2, 2}, {2, 1, 254, 1}, {2, 0, 86, 3}},
        {{127, 128, 1, 0}, {32, 127, 141, 127}, {32, 127, 141, 127}},
        {{127, 2, 1, 200}, {1, 255, 127, 1}, {126, 3, 2, 200}},
        {{254, 128, 70, 1}, {203, 127, 255, 254}, {203, 127, 255, 254}},
        {{127, 254, 254, 254}, {234, 255, 128, 1}, {127, 254, 254, 254}},
        {{1, 128, 0, 128}, {255, 255, 76, 1}, {3, 129, 1, 128}},
        {{0, 255, 128, 1}, {1, 1, 1, 0}, {0, 255, 128, 1}},
        {{255, 2, 127, 2}, {0, 127, 128, 128}, {2, 126, 128, 129}},
        {{2, 254, 128, 0}, {127, 127, 1, 221}, {127, 127, 1, 221}},
        {{128, 127, 127, 255}, {0, 0, 128, 2}, {127, 126, 127, 255}},
        {{1, 128, 128, 23}, {2, 137, 128, 254}, {2, 137, 128, 254}},
        {{127, 128, 254, 0}, {255, 42, 127, 0}, {127, 128, 254, 0}},
        {{128, 2, 0, 0}, {128, 1, 255, 166}, {128, 1, 255, 166}},
        {{128, 0, 2, 127}, {0, 2, 1, 192}, {18, 2, 1, 223}},
        {{0, 125, 0, 1}, {1, 128, 128, 127}, {1, 128, 127, 128}},
        {{118, 255, 39, 0}, {1, 254, 128, 0}, {118, 255, 39, 0}},
        {{0, 128, 127, 128}, {146, 238, 127, 1}, {1, 129, 127, 128}},
        {{0, 148, 39, 255}, {127, 1, 128, 2}, {1, 147, 40, 255}},
        {{1, 127, 254, 0}, {0, 251, 128, 255}, {0, 251, 128, 255}},
        {{255, 1, 0, 254}, {1, 0, 128, 1}, {254, 1, 1, 254}},
        {{255, 254, 128, 128}, {0, 2, 128, 254}, {1, 3, 128, 255}},
        {{254, 0, 127, 0}, {210, 128, 0, 2}, {210, 128, 0, 2}},
        {{255, 128, 128, 255}, {128, 255, 2, 1}, {255, 128, 128, 255}},
        {{106, 112, 170, 2}, {127, 2, 1, 127}, {127, 3, 2, 128}},
    };
    for (const auto& caso : casos) {
        std::uint8_t pixel[4] = {caso[0][0], caso[0][1], caso[0][2], caso[0][3]};
        Game::mezclarEncima(pixel, caso[1]);
        for (int c = 0; c < 4; ++c)
            MOTOR_CHECK(pixel[c] == caso[2][c]);
    }
}

MOTOR_TEST_CASE(apariencia_horneados_byte_a_byte) {
    auto m = montaje();
    int comprobados = 0, pnj = 0;
    // Los atlas que lista personajes.json en "horneados": los 62 de siempre y
    // los PNJ (PNJ-01).
    const JsonValue manifiesto = leerJson(std::string(kTexturas) + "/personajes.json");
    for (std::size_t h = 0; h < manifiesto["horneados"].size(); ++h) {
        const std::string fichero = manifiesto["horneados"][h].asString();
        const std::string nombre = fichero.substr(0, fichero.size() - 5);
        const JsonValue json = leerJson(std::string(kTexturas) + "/" + fichero);
        auto atlas = imagen::cargarPNG(std::string(kTexturas) + "/" + json["image"].asString());
        MOTOR_CHECK(atlas.isOk());
        const JsonValue& actores = json["actores"];
        for (std::size_t i = 0; i < actores.size(); ++i) {
            const JsonValue& actor = actores[i];
            auto apariencia = m->manifiesto().desdeComposicion(actor["composicion"]);
            if (!apariencia.isOk())
                MOTOR_CHECK_FAIL(actor["id"].asString() + ": " + apariencia.errorMessage());
            auto hoja = m->montar(apariencia.value());
            if (!hoja.isOk())
                MOTOR_CHECK_FAIL(actor["id"].asString() + ": " + hoja.errorMessage());
            // PL-JUEGO-01: la hoja montada lleva los 40 frames de PixelLab (2560x64), andar y respirar
            // en las cuatro diagonales, igual que las horneadas: se comparan enteras.
            MOTOR_CHECK(hoja.value().ancho == 64 * m->manifiesto().frames && hoja.value().alto == 64);
            MOTOR_CHECK(atlas.value().ancho == hoja.value().ancho);
            std::set<int> frames;
            const int distintos = diferencias(hoja.value(), atlas.value(), actor["fila"].asInt(), &frames);
            if (distintos != 0) {
                std::string cuales;
                for (const int f : frames) cuales += " " + std::to_string(f);
                MOTOR_CHECK_FAIL(nombre + "/" + actor["id"].asString() + ": " +
                                 std::to_string(distintos) + " bytes distintos del horneado (frames" + cuales + ")");
            }
            ++comprobados;
            pnj += nombre.rfind("pnj_", 0) == 0 ? 1 : 0;
        }
    }
    MOTOR_CHECK(comprobados == 91 + pnj);  // 72 razas andando (45 + 27 del 2000 b.f.), 10 de historia y 9 actores
    MOTOR_CHECK(pnj >= 250);  // un PNJ por objeto npc de assets/objects, y sus variantes
    m->vaciarCache();
}

MOTOR_TEST_CASE(apariencia_capas_como_el_probador) {
    auto m = montaje();
    const auto& man = m->manifiesto();

    // La naga del rio: sin capucha ni morro de serpiente, con pelo de persona.
    auto naga = m->capas(man.personajes.at("naga_del_rio"));
    MOTOR_CHECK(naga.isOk());
    // PL-JUEGO-01: con su cabeza de PixelLab (lo que sale de la de la familia va en «rasgos»).
    MOTOR_CHECK((ranuras(naga.value()) ==
                 std::vector<std::string>{"cuerpo", "rasgos", "pelo", "torso", "arma"}));

    // Otra piel en un humano: el cuerpo sale del atlas de cuerpos lisos, que
    // trae ojos azules, asi que los ojos de la raza (marrones) van encima.
    Game::Apariencia humano = Game::Apariencia::deRaza(*man.raza("race_human_marches"));
    humano.piel = "piel_oscura";
    humano.ojos = "marron";
    auto h = m->capas(humano);
    MOTOR_CHECK(h.isOk());
    MOTOR_CHECK(h.value()[0].atlas == "personajes/cuerpos/cuerpos_medio.png");
    MOTOR_CHECK(h.value()[1].ranura == "ojos");

    // Un yelmo cerrado tapa el pelo; un sombrero de paja no (PL-ROPA-01: la capucha de PixelLab tambien lo tapa).
    humano.ropa["cabeza"] = "yelmo";
    auto conYelmo = m->capas(humano);
    MOTOR_CHECK(conYelmo.isOk());
    for (const auto& capa : conYelmo.value())
        MOTOR_CHECK(capa.ranura != "pelo");
    // Lo puesto manda sobre la ropa elegida.
    auto conEquipo = m->capas(humano, {{"cabeza", "sombrero_paja"}, {"arma", "espada_hierro"}});
    MOTOR_CHECK(conEquipo.isOk());
    bool hayPelo = false, hayEspada = false;
    for (const auto& capa : conEquipo.value()) {
        hayPelo = hayPelo || capa.ranura == "pelo";
        hayEspada = hayEspada || (capa.ranura == "arma" && capa.fila == 1);
    }
    MOTOR_CHECK(hayPelo && hayEspada);

    // Un aarakocra no lleva botas aunque se las pongan: no tiene pies.
    Game::Apariencia pajaro = Game::Apariencia::deRaza(*man.raza("aarakocra_de_las_montanas"));
    auto p = m->capas(pajaro, {{"pies", "botas"}});
    MOTOR_CHECK(p.isOk());
    for (const auto& capa : p.value())
        MOTOR_CHECK(capa.ranura != "pies");
}

MOTOR_TEST_CASE(apariencia_lo_que_no_existe_es_error) {
    auto m = montaje();
    const auto& man = m->manifiesto();
    Game::Apariencia a = Game::Apariencia::deRaza(*man.raza("race_human_marches"));
    a.raza = "no_existe";
    MOTOR_CHECK(!m->montar(a).isOk());

    a = Game::Apariencia::deRaza(*man.raza("race_human_marches"));
    a.ropa["torso"] = "armadura_imaginaria";
    auto r = m->montar(a);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("armadura_imaginaria") != std::string::npos);

    // EDITOR-PERSONALIZACION-01: un minotauro admite otra piel de «pieles» (el cuerpo se tine), pero no
    // una que no exista ni otra talla.
    Game::Apariencia toro = Game::Apariencia::deRaza(*man.raza("minotauros_de_la_llanura"));
    toro.piel = "piel_clara";
    MOTOR_CHECK(m->capas(toro).isOk());
    toro.piel = "piel_que_no_existe";
    MOTOR_CHECK(!m->montar(toro).isOk());
    toro.piel = "";
    toro.talla = "medio";
    MOTOR_CHECK(!m->montar(toro).isOk());
}

MOTOR_TEST_CASE(apariencia_cache_con_presupuesto) {
    // 20 MB no caben las seis capas de raza (14 MB cada una, con los 20
    // frames): obliga a soltar atlas entre montajes, y la hoja tiene que
    // salir igual.
    auto grande = montaje();
    auto justo = montaje(20u * 1024u * 1024u);
    const auto& man = grande->manifiesto();
    for (const char* id : {"aarakocra_de_las_montanas", "yokai_de_las_sombras", "nagas_venenosos",
                           "race_ascaria_fey_blood", "enanos_forjadores"}) {
        const Game::Apariencia a = Game::Apariencia::deRaza(*man.raza(id), true);
        auto uno = grande->montar(a);
        auto otro = justo->montar(a);
        MOTOR_CHECK(uno.isOk() && otro.isOk());
        MOTOR_CHECK(uno.value().pixeles == otro.value().pixeles);
        // Al acabar cada montaje la cache vuelve al presupuesto, o se queda con un solo atlas (el
        // ultimo usado: el de las armas ya pasa de 35 MB descomprimido con los sets de SP-02)
        MOTOR_CHECK(justo->bytesEnCache() <= 20u * 1024u * 1024u || justo->atlasEnCache() == 1);
    }
    justo->vaciarCache();
    MOTOR_CHECK(justo->bytesEnCache() == 0);
}

MOTOR_TEST_CASE(apariencia_reparto_de_la_hoja) {
    using Game::Rumbo;
    MOTOR_CHECK(Game::rumboDeFrame(1) == Rumbo::Arriba);
    MOTOR_CHECK(Game::rumboDeFrame(6) == Rumbo::Derecha);
    MOTOR_CHECK(Game::rumboDeFrame(8) == Rumbo::Abajo);
    MOTOR_CHECK(Game::rumboDeFrame(12) == Rumbo::Izquierda);
    MOTOR_CHECK((Game::cicloDeAndar(Rumbo::Abajo) == std::vector<int>{8, 7, 8, 9}));
    MOTOR_CHECK((Game::cicloDeAndar(Rumbo::Arriba) == std::vector<int>{2, 1, 2, 3}));
    MOTOR_CHECK((Game::cicloDeAndar(Rumbo::Izquierda) == std::vector<int>{11, 10, 11, 12}));
    // ESTETICA-01: el reposo, dos frames por rumbo despues de los de andar.
    MOTOR_CHECK(Game::rumboDeFrame(13) == Rumbo::Arriba);
    MOTOR_CHECK(Game::rumboDeFrame(16) == Rumbo::Derecha);
    MOTOR_CHECK(Game::rumboDeFrame(17) == Rumbo::Abajo);
    MOTOR_CHECK(Game::rumboDeFrame(20) == Rumbo::Izquierda);
    MOTOR_CHECK((Game::framesDeReposo(Rumbo::Abajo) == std::vector<int>{17, 18}));
    MOTOR_CHECK((Game::framesDeReposo(Rumbo::Arriba) == std::vector<int>{13, 14}));
    MOTOR_CHECK((Game::framesDeReposo(Rumbo::Izquierda) == std::vector<int>{19, 20}));
    const std::vector<int> reposo = Game::cicloDeReposo(Rumbo::Derecha);
    MOTOR_CHECK(reposo.front() == 15 && reposo.back() == 16);
    MOTOR_CHECK(std::count(reposo.begin(), reposo.end(), 15) > std::count(reposo.begin(), reposo.end(), 16));
    // La vista previa empieza andando hacia abajo, descansa y gira a la derecha.
    MOTOR_CHECK(Game::frameDeVistaPrevia(0.0) == 7);
    MOTOR_CHECK(Game::frameDeVistaPrevia(0.18) == 8);
    MOTOR_CHECK(Game::frameDeVistaPrevia(0.17 * 8 + 0.01) == 17);
    MOTOR_CHECK(Game::frameDeVistaPrevia(0.17 * 12 + 0.01) == 18);
    MOTOR_CHECK(Game::rumboDeFrame(Game::frameDeVistaPrevia(0.17 * 16 + 0.01)) == Rumbo::Derecha);
    MOTOR_CHECK(Game::rumboDeFrame(Game::frameDeVistaPrevia(0.17 * 32 + 0.01)) == Rumbo::Arriba);
    MOTOR_CHECK(Game::rumboDeFrame(Game::frameDeVistaPrevia(0.17 * 48 + 0.01)) == Rumbo::Izquierda);
    MOTOR_CHECK(Game::frameDeVistaPrevia(-3.0) == 7);
}

// Oriol (24/10): el reparto en diagonal de los cuerpos de PixelLab, 40 frames: andar 1-24 (seis por
// rumbo: noreste, sureste, suroeste, noroeste) y respirar 25-40 (cuatro por rumbo).
MOTOR_TEST_CASE(apariencia_reparto_en_diagonal) {
    using Game::Reparto;
    using Game::Rumbo;
    const Reparto d = Reparto::Diagonal;
    MOTOR_CHECK(Game::repartoDeColumnas(12) == Reparto::Clasico);
    MOTOR_CHECK(Game::repartoDeColumnas(20) == Reparto::Clasico);
    MOTOR_CHECK(Game::repartoDeColumnas(40) == d);
    MOTOR_CHECK(Game::framesDeReparto(d) == 40);
    MOTOR_CHECK(Game::rumboDeFrame(1, d) == Rumbo::Arriba);
    MOTOR_CHECK(Game::rumboDeFrame(7, d) == Rumbo::Derecha);
    MOTOR_CHECK(Game::rumboDeFrame(18, d) == Rumbo::Abajo);
    MOTOR_CHECK(Game::rumboDeFrame(24, d) == Rumbo::Izquierda);
    MOTOR_CHECK(Game::rumboDeFrame(25, d) == Rumbo::Arriba);
    MOTOR_CHECK(Game::rumboDeFrame(32, d) == Rumbo::Derecha);
    MOTOR_CHECK(Game::rumboDeFrame(33, d) == Rumbo::Abajo);
    MOTOR_CHECK(Game::rumboDeFrame(40, d) == Rumbo::Izquierda);
    MOTOR_CHECK((Game::cicloDeAndar(Rumbo::Abajo, d) == std::vector<int>{13, 14, 15, 16, 17, 18}));
    MOTOR_CHECK((Game::framesDeReposo(Rumbo::Derecha, d) == std::vector<int>{29, 30, 31, 32}));
    MOTOR_CHECK(Game::frameQuieto(Rumbo::Izquierda, d) == 37);
    // las capas se ordenan como de frente (sureste, suroeste) o de espaldas (noreste, noroeste)
    MOTOR_CHECK(std::string(Game::vistaDeFrame(1, d)) == "arriba");
    MOTOR_CHECK(std::string(Game::vistaDeFrame(10, d)) == "abajo");
    MOTOR_CHECK(std::string(Game::vistaDeFrame(35, d)) == "abajo");
    MOTOR_CHECK(std::string(Game::vistaDeFrame(40, d)) == "arriba");
    MOTOR_CHECK(std::string(Game::vistaDeFrame(5)) == "derecha");  // el clasico, como siempre
    const std::vector<int> reposo = Game::cicloDeReposo(Rumbo::Abajo, d);
    MOTOR_CHECK(reposo.size() == 20 && reposo.front() == 33 && reposo.back() == 36);
    // la vista previa: suroeste andando, respira y gira al sureste
    MOTOR_CHECK(Game::frameDeVistaPrevia(0.0, d) == 13);
    MOTOR_CHECK(Game::frameDeVistaPrevia(0.12 * 5 + 0.01, d) == 18);
    MOTOR_CHECK(Game::frameDeVistaPrevia(0.12 * 12 + 0.01, d) == 33);
    MOTOR_CHECK(Game::frameDeVistaPrevia(0.12 * 14 + 0.01, d) == 34);
    MOTOR_CHECK(Game::rumboDeFrame(Game::frameDeVistaPrevia(0.12 * 28 + 0.01, d), d) == Rumbo::Derecha);
}

int main() { return motor_test::runAllTests(); }

MOTOR_TEST_CASE(estetica_el_tinte_cambia_solo_los_tonos_del_material) {
    using Game::juntarTinte;
    using Game::separarTinte;
    // El texto de una pieza con tinte, ida y vuelta.
    const Game::PiezaTenida t = separarTinte("f_daga:hierro>plaga,laton>plata");
    MOTOR_CHECK(t.pieza == "f_daga");
    MOTOR_CHECK(t.tinte.size() == 2 && t.tinte[0].first == "hierro" && t.tinte[1].second == "plata");
    MOTOR_CHECK(juntarTinte(t.pieza, t.tinte) == "f_daga:hierro>plaga,laton>plata");
    MOTOR_CHECK(separarTinte("botas").pieza == "botas" && separarTinte("botas").tinte.empty());
    MOTOR_CHECK(juntarTinte("botas", {{"cuero", "cuero"}}) == "botas");  // lo que no cambia no se escribe

    auto m = montaje();
    const auto& man = m->manifiesto();
    MOTOR_CHECK(man.paletas.count("hierro") == 1 && man.paletas.count("plaga") == 1);
    MOTOR_CHECK(man.equipo.at("arma").buscar("f_daga:hierro>plaga") == man.equipo.at("arma").buscar("f_daga"));

    // La daga de plaga del catalogo (rara): el escalon raro de la daga con la hoja de plaga.
    const Game::PiezaDeObjeto& plaga = man.piezaDeObjeto.at("daga_de_plaga");
    MOTOR_CHECK(plaga.ranura == "arma" && plaga.pieza == "f_daga_raro");
    MOTOR_CHECK(plaga.visible().rfind("f_daga_raro:hierro>plaga", 0) == 0);

    // Montada con y sin tinte: solo cambian los pixeles de la daga que tenian un
    // tono de un material de origen, y pasan al mismo tono del de destino.
    const Game::Apariencia humano = Game::Apariencia::deRaza(*man.raza("race_human_marches"));
    auto sin = m->montar(humano, {{"arma", plaga.pieza}});
    auto con = m->montar(humano, {{"arma", plaga.visible()}});
    MOTOR_CHECK(sin.isOk() && con.isOk());
    std::map<std::array<std::uint8_t, 3>, std::array<std::uint8_t, 3>> esperado;
    for (const auto& [origen, destino] : plaga.tinte) {
        for (std::size_t k = 0; k < 5; ++k)
            esperado.emplace(man.paletas.at(origen)[k], man.paletas.at(destino)[k]);
    }
    int cambiados = 0, malos = 0;
    const auto& a = sin.value().pixeles;
    const auto& b = con.value().pixeles;
    for (std::size_t i = 0; i < a.size(); i += 4) {
        if (a[i] == b[i] && a[i + 1] == b[i + 1] && a[i + 2] == b[i + 2] && a[i + 3] == b[i + 3])
            continue;
        ++cambiados;
        const auto it = esperado.find({a[i], a[i + 1], a[i + 2]});
        const bool bien = it != esperado.end() && it->second == std::array<std::uint8_t, 3>{b[i], b[i + 1], b[i + 2]} &&
                          a[i + 3] == b[i + 3];
        malos += bien ? 0 : 1;
    }
    MOTOR_CHECK(cambiados > 20);
    MOTOR_CHECK(malos == 0);

    // Una paleta que no existe es un error legible.
    auto mala = m->montar(humano, {{"arma", "f_daga:hierro>adamantio_de_mentira"}});
    MOTOR_CHECK(!mala.isOk());
    MOTOR_CHECK(mala.errorMessage().find("adamantio_de_mentira") != std::string::npos);

    // Lo que se ve de lo puesto lleva su tinte (Vestuario).
    RPG::EquipoPuesto puestos{};
    puestos[0] = "daga_de_plaga";
    const auto visibles = Game::Vestuario::piezasVisibles(puestos, man);
    MOTOR_CHECK(visibles.at("arma") == plaga.visible());
}

MOTOR_TEST_CASE(estetica_todos_los_objetos_se_montan) {
    // Los 413 objetos de equipment.json: los que se ven (con su forma y su
    // tinte) se montan en un humano sin error. Las armas y la ropa comparten
    // formas, pero casi ninguno se ve igual que otro.
    auto m = montaje();
    const auto& man = m->manifiesto();
    const Game::Apariencia humano = Game::Apariencia::deRaza(*man.raza("race_human_marches"));
    std::set<std::string> aspectos;
    int montados = 0;
    for (const auto& [objeto, pieza] : man.piezaDeObjeto) {
        auto hoja = m->montar(humano, {{pieza.ranura, pieza.visible()}});
        if (!hoja.isOk())
            std::cerr << objeto << ": " << hoja.errorMessage() << "\n";
        MOTOR_CHECK(hoja.isOk());
        montados += hoja.isOk() ? 1 : 0;
        aspectos.insert(pieza.ranura + "=" + pieza.visible());
    }
    MOTOR_CHECK(montados == static_cast<int>(man.piezaDeObjeto.size()));
    MOTOR_CHECK(man.piezaDeObjeto.size() >= 400);
    // + los siete únicos de Boundington (bnd_unico_*): llevan la pieza de su base y los distingue el brillo
    MOTOR_CHECK(aspectos.size() + 13 >= man.piezaDeObjeto.size());
}

MOTOR_TEST_CASE(sp02_los_guanteletes_van_en_las_dos_manos_y_encima_tambien_de_espaldas) {
    // SP-02 (Oriol: «Guanteletes grandes», uno en cada mano): la fila de los
    // guanteletes trae su propio orden de dibujo (encima del cuerpo en los
    // cuatro rumbos); un arma normal va detras del cuerpo de espaldas.
    auto m = montaje();
    const auto& man = m->manifiesto();
    const Game::FilaDeCapa* guantes = man.equipo.at("arma").buscar("f_guanteletes");
    // (la espada no: Oriol la pone delante en unos frames desde la Forja; un hacha sigue normal)
    const Game::FilaDeCapa* espada = man.equipo.at("arma").buscar("f_hacha");
    MOTOR_CHECK(guantes != nullptr && espada != nullptr);
    if (guantes == nullptr || espada == nullptr)
        return;
    MOTOR_CHECK(guantes->orden.size() == 4);
    MOTOR_CHECK(guantes->orden.count("arriba") == 1 && guantes->orden.at("arriba") > man.orden.at("cuerpo").at("arriba"));
    // PL-JUEGO-01: un arma de PixelLab trae su orden frame a frame (delante o detras del cuerpo segun la
    // muneca este mas cerca o mas lejos de la camara que los hombros) y, si otra talla cambia, «<talla>/f<n>».
    MOTOR_CHECK(espada->orden.size() >= 4 + 40 && espada->orden.count("f25") == 1);
    MOTOR_CHECK(man.orden.at("arma").at("arriba") < man.orden.at("cuerpo").at("arriba"));
    // Los guanteletes de Viaje del catalogo son esa pieza (con el tinte de Viaje).
    MOTOR_CHECK(man.piezaDeObjeto.count("guanteletes_de_viaje") == 1);
    MOTOR_CHECK(man.piezaDeObjeto.at("guanteletes_de_viaje").pieza == "f_guanteletes");

    // De espaldas y quieto (PL-JUEGO-01: el primer frame de respirar al noreste, el 25; el 2 antes), en
    // las dos manos del humano de talla media (sus anclas): encima queda el guantelete; con un hacha, la mano.
    const Game::Apariencia humano = Game::Apariencia::deRaza(*man.raza("race_human_marches"));
    const int espaldas = Game::frameQuieto(Game::Rumbo::Arriba, Game::repartoDeColumnas(man.frames));
    const auto& anclas = man.anclas.at("medio");
    const std::size_t qi = static_cast<std::size_t>(espaldas - 1);
    for (const char* pieza : {"f_guanteletes", "f_hacha"}) {
        Game::MapaDeCapas mapa;
        auto hoja = m->montar(humano, {{"arma", pieza}}, &mapa);
        MOTOR_CHECK(hoja.isOk());
        if (!hoja.isOk())
            return;
        const auto it = std::find(mapa.ranuras.begin(), mapa.ranuras.end(), "arma");
        MOTOR_CHECK(it != mapa.ranuras.end());
        const int arma = static_cast<int>(it - mapa.ranuras.begin()) + 1;
        const int ancho = hoja.value().ancho;
        const int x0 = 64 * (espaldas - 1);
        for (const auto& punto : {anclas.mano[qi], anclas.otraMano[qi]}) {
            const int arriba = mapa.arriba[static_cast<std::size_t>(punto[1] * ancho + x0 + punto[0])];
            if (std::string(pieza) == "f_guanteletes")
                MOTOR_CHECK(arriba == arma);
            else
                MOTOR_CHECK(arriba != arma);
        }
    }
}

MOTOR_TEST_CASE(forja_un_frame_suelto_puede_poner_el_arma_delante) {
    // Oriol (23/10): la Forja de Onegai pone el arma delante o detras del personaje frame a frame. La fila
    // trae «f<n>» en su orden, que manda sobre el de la direccion: con la espada en un frame en que va
    // detras del cuerpo (PL-JUEGO-01: el primero de la talla media; antes, el 2, de espaldas) puesta
    // delante, se ve mas espada.
    auto m = montaje();
    const auto& man = m->manifiesto();
    const Game::FilaDeCapa* espada = man.equipo.at("arma").buscar("f_espada");
    MOTOR_CHECK(espada != nullptr);
    if (espada == nullptr)
        return;
    const Game::Apariencia humano = Game::Apariencia::deRaza(*man.raza("race_human_marches"));
    int frame = 0;
    for (int f = 1; f <= man.frames && frame == 0; ++f) {
        const auto it = espada->orden.find("f" + std::to_string(f));
        if (it != espada->orden.end() && it->second < man.orden.at("cuerpo").at("abajo")) frame = f;
    }
    MOTOR_CHECK(frame > 0);
    if (frame == 0)
        return;
    const std::string clave = "f" + std::to_string(frame);
    auto cuenta = [&]() {
        Game::MapaDeCapas mapa;
        auto hoja = m->montar(humano, {{"arma", "f_espada"}}, &mapa);
        MOTOR_CHECK(hoja.isOk());
        if (!hoja.isOk())
            return -1;
        const int arma = static_cast<int>(std::find(mapa.ranuras.begin(), mapa.ranuras.end(), "arma") - mapa.ranuras.begin()) + 1;
        const int ancho = hoja.value().ancho;
        int n = 0;
        const int x0 = 64 * (frame - 1);
        for (int y = 0; y < 64; ++y)
            for (int x = x0; x < x0 + 64; ++x) n += mapa.arriba[static_cast<std::size_t>(y * ancho + x)] == arma ? 1 : 0;
        return n;
    };
    const int detras = cuenta();
    auto& fila = const_cast<Game::FilaDeCapa&>(*espada);  // solo en la prueba: el manifiesto ya cargado
    const int antes = fila.orden.at(clave);
    fila.orden[clave] = 90;
    const int delante = cuenta();
    fila.orden[clave] = antes;
    MOTOR_CHECK(detras >= 0 && delante > detras);
}

// EDITOR-PERSONALIZACION-01 (Oriol, 26/10): cualquier piel, cualquier color de pelo, marcas, alas, cola y
// cuernos, para cualquier raza.
MOTOR_TEST_CASE(personalizacion_piel_pelo_marcas_y_rasgos) {
    auto m = montaje();
    const auto& man = m->manifiesto();

    // Un dracónido (cabeza propia, rasgos) con la piel de un orco: las capas de la raza, el cuerpo tenido
    // color a color con los pares de «pieles»; los rasgos (cuernos, escamas) sin tocar.
    Game::Apariencia dragon = Game::Apariencia::deRaza(*man.raza("draconidos_de_fuego"));
    dragon.piel = "orcos_del_consejo";
    auto d = m->capas(dragon);
    MOTOR_CHECK(d.isOk());
    if (d.isOk()) {
        bool cuerpoTenido = false;
        for (const auto& capa : d.value()) {
            if (capa.ranura == "cuerpo")
                cuerpoTenido = !capa.cambios.empty();
            else if (capa.ranura != "orejas" && capa.ranura != "morro")
                MOTOR_CHECK(capa.cambios.empty());
        }
        MOTOR_CHECK(cuerpoTenido);
        auto hoja = m->montar(dragon);
        MOTOR_CHECK(hoja.isOk());
    }
    // Una piel que no esta en «pieles» ni en el atlas de cuerpos: error claro.
    dragon.piel = "piel_de_mentira";
    MOTOR_CHECK(!m->capas(dragon).isOk());

    // Un aarakocra con melena larga de color llama: el pelo ya no esta en «sin», y como el atlas no trae
    // «melena_larga/pelo_llama», se tine la fila castana.
    Game::Apariencia pajaro = Game::Apariencia::deRaza(*man.raza("aarakocra_de_las_montanas"));
    pajaro.peloEstilo = "melena_larga";
    pajaro.peloColor = "pelo_llama";
    pajaro.barbaEstilo = "BARBA_TRENZADA";
    pajaro.barbaColor = "pelo_azul_hielo";
    auto p = m->capas(pajaro);
    MOTOR_CHECK(p.isOk());
    if (p.isOk()) {
        bool pelo = false, barba = false;
        for (const auto& capa : p.value()) {
            if (capa.ranura == "pelo") {
                pelo = true;
                MOTOR_CHECK(capa.fila == man.pelo.buscar("melena_larga/pelo_castano")->fila && capa.cambios.size() == 5);
            }
            if (capa.ranura == "barba")
                barba = true;
        }
        MOTOR_CHECK(pelo && barba);
    }
    // El color que el atlas trae se usa tal cual, sin tinte; NET-E1a: si el atlas va recortado a la fila
    // castana (sin «melena_larga/pelo_rubio»), la castana tenida: mismo resultado, otra via.
    pajaro.peloColor = "pelo_rubio";
    p = m->capas(pajaro);
    MOTOR_CHECK(p.isOk());
    const auto* filaRubia = man.pelo.buscar("melena_larga/pelo_rubio");
    const auto* filaCastana = man.pelo.buscar("melena_larga/pelo_castano");
    MOTOR_CHECK(filaCastana != nullptr);
    if (p.isOk() && filaCastana != nullptr)
        for (const auto& capa : p.value())
            if (capa.ranura == "pelo")
                MOTOR_CHECK(filaRubia != nullptr ? (capa.cambios.empty() && capa.fila == filaRubia->fila)
                                                  : (!capa.cambios.empty() && capa.fila == filaCastana->fila));

    // Marcas (dos, una con tinte), alas, cola y cuernos en un humano: cada una su capa y su orden.
    Game::Apariencia humano = Game::Apariencia::deRaza(*man.raza("race_human_marches"));
    humano.marcas = {"tatuaje_cara:tinta_negra>plaga", "cicatriz_pecho"};
    humano.ropa["alas"] = "alas_libelula";
    humano.ropa["cola"] = "cola_zorro";
    humano.ropa["cuernos"] = "cuernos_astas";
    auto h = m->capas(humano);
    MOTOR_CHECK(h.isOk());
    if (h.isOk()) {
        int marcas = 0;
        bool alas = false, cola = false, cuernos = false, tinte = false;
        for (const auto& capa : h.value()) {
            if (capa.ranura == "marcas") {
                ++marcas;
                tinte = tinte || !capa.cambios.empty();
            }
            alas = alas || capa.ranura == "alas";
            cola = cola || capa.ranura == "cola";
            cuernos = cuernos || capa.ranura == "cuernos";
        }
        MOTOR_CHECK(marcas == 2 && tinte && alas && cola && cuernos);
        MOTOR_CHECK(man.orden.at("marcas").at("abajo") == 12 && man.orden.at("cuernos").at("abajo") == 66);
        auto hoja = m->montar(humano);
        MOTOR_CHECK(hoja.isOk());
    }
    // La clave distingue las marcas.
    Game::Apariencia sinMarcas = humano;
    sinMarcas.marcas.clear();
    MOTOR_CHECK(humano.clave() != sinMarcas.clave());
    m->vaciarCache();
}
