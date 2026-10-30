// ESTETICA-01 — El brillo del equipo puesto, por lo fuerte que es (estilo
// NosTale, sin mejoras): Tier 1 brilla como un +1/+2, Tier 2 como un +3/+4...
//
// Casos (assets reales, copiados a build/ por copiar_assets):
//   1. La escalera: los niveles 1-2 solo destellan, el 3 ya tiene halo, el 5
//      ya echa particulas, y cuanto mas alto mas; la ropa brilla menos.
//   2. El catalogo: cada objeto trae el nivel de su tier (el primero del par
//      para la pieza de entrada, el segundo para la fuerte, las reliquias y
//      las unicas) y el color y las particulas de su esencia o su deidad.
//   3. El montaje sabe que capa queda arriba en cada pixel, y el brillo sale
//      solo de lo que se ve de cada pieza.
//   4. El halo de un arma va alrededor del arma; el de la ropa, solo por
//      fuera del cuerpo.
//   5. Los destellos y las particulas son deterministas (mismo instante,
//      mismo dibujo), cambian con el tiempo y nacen en la pieza.
#include "TestHarness.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#include "Game/ParticulasDeCartas.h"
#include "Game/Personajes/BrilloEquipo.h"
#include "Game/Personajes/MontajePersonajes.h"
#include "Game/Personajes/Vestuario.h"

namespace {

const char* const kTexturas = "assets/textures";

std::unique_ptr<Game::MontajePersonajes> montaje() {
    auto m = Game::MontajePersonajes::cargar(kTexturas);
    if (!m.isOk())
        throw std::runtime_error(m.errorMessage());
    return std::move(m.value());
}

// El humano con un objeto puesto: la hoja, el mapa de capas y el brillo.
struct Vestido {
    imagen::ImagenRGBA hoja;
    Game::MapaDeCapas mapa;
    Game::HojaDeBrillo brillo;
};

Vestido vestir(Game::MontajePersonajes& m, const std::vector<std::string>& objetos) {
    const auto& man = m.manifiesto();
    std::map<std::string, std::string> equipo;
    std::map<std::string, Game::BrilloDePieza> brillos;
    for (const std::string& id : objetos) {
        const Game::PiezaDeObjeto& p = man.piezaDeObjeto.at(id);
        equipo[p.ranura] = p.visible();
        brillos[p.ranura] = p.brillo;
    }
    Vestido v;
    auto hoja = m.montar(Game::Apariencia::deRaza(*man.raza("race_human_marches")), equipo, &v.mapa);
    if (!hoja.isOk())
        throw std::runtime_error(hoja.errorMessage());
    v.hoja = std::move(hoja.value());
    v.brillo = Game::montarBrillo(v.hoja, v.mapa, brillos);
    return v;
}

float alfaHalo(const imagen::ImagenRGBA& halo, int x, int y) { return halo.pixel(x, y)[3] / 255.0f; }

}  // namespace

MOTOR_TEST_CASE(brillo_la_escalera_sube_con_el_nivel) {
    using Game::escalonDeBrillo;
    MOTOR_CHECK(escalonDeBrillo(0, false).destellos == 0.0f);
    // Tier 1 (+1/+2): solo destellos.
    MOTOR_CHECK(escalonDeBrillo(1, false).radio == 0 && escalonDeBrillo(1, false).destellos > 0.0f);
    MOTOR_CHECK(escalonDeBrillo(2, false).radio == 0 && escalonDeBrillo(2, false).particulas == 0.0f);
    // Tier 2 (+3/+4): halo fino. Tier 3 (+5/+6): particulas.
    MOTOR_CHECK(escalonDeBrillo(3, false).radio == 1 && escalonDeBrillo(3, false).particulas == 0.0f);
    MOTOR_CHECK(escalonDeBrillo(5, false).particulas > 0.0f);
    for (int n = 1; n < 10; ++n) {
        const auto a = escalonDeBrillo(n, false), b = escalonDeBrillo(n + 1, false);
        MOTOR_CHECK(b.radio >= a.radio && b.alfa >= a.alfa && b.destellos >= a.destellos &&
                    b.particulas >= a.particulas);
        // La ropa brilla menos que un arma del mismo nivel, y se barre desde el 3.
        const auto r = escalonDeBrillo(n, true);
        MOTOR_CHECK(r.alfa <= a.alfa && r.particulas <= a.particulas && r.radio <= a.radio);
        MOTOR_CHECK(r.barrido == (n >= 3));
    }
    MOTOR_CHECK(escalonDeBrillo(10, false).radio > escalonDeBrillo(5, false).radio);
    // Los tipos de particula: la sombra y las hojas tapan, el resto suma luz.
    MOTOR_CHECK(!Game::tipoDeParticula("sombra").suma && !Game::tipoDeParticula("hojas").suma);
    MOTOR_CHECK(Game::tipoDeParticula("brasas").suma && Game::tipoDeParticula("brasas").vy < 0.0f);
    MOTOR_CHECK(Game::tipoDeParticula("escarcha").vy > 0.0f);  // la escarcha cae
    MOTOR_CHECK(Game::tipoDeParticula("no_existe").forma == Game::tipoDeParticula("chispas").forma);
}

MOTOR_TEST_CASE(brillo_cada_objeto_el_de_su_tier) {
    auto m = montaje();
    const auto& objetos = m->manifiesto().piezaDeObjeto;
    auto nivel = [&](const char* id) { return objetos.at(id).brillo.nivel; };
    MOTOR_CHECK(nivel("daga_de_milicia") == 1);          // Tier 1, comun
    MOTOR_CHECK(nivel("daga_de_plata") == 2);            // Tier 1, poco comun
    MOTOR_CHECK(nivel("daga_de_plaga") == 4);            // Tier 2, raro +2
    MOTOR_CHECK(nivel("sable_ligero_de_vacio") == 7);    // Tier 4, mitico de esencia
    MOTOR_CHECK(nivel("aguja_del_firmamento") == 8);     // Tier 4, reliquia
    MOTOR_CHECK(nivel("botas_de_marcha_forzada") == 6);  // Tier 3, reliquia
    for (const auto& [id, p] : objetos) {
        MOTOR_CHECK(p.brillo.nivel >= 1 && p.brillo.nivel <= 10);
        (void)id;
    }
    // El color y las particulas, de su esencia o de su deidad.
    MOTOR_CHECK(objetos.at("daga_de_plaga").brillo.particulas == "veneno");
    MOTOR_CHECK(objetos.at("sable_ligero_de_vacio").brillo.particulas == "vacio");  // PART-01: el suyo, no la sombra
    MOTOR_CHECK(objetos.at("botas_de_marcha_forzada").brillo.particulas == "brasas");
    const auto& plaga = objetos.at("daga_de_plaga").brillo.color;
    MOTOR_CHECK(plaga[1] > plaga[0] && plaga[1] > plaga[2]);  // verde
    // Vestuario: lo puesto que brilla, por ranura visual.
    RPG::EquipoPuesto puestos{};
    puestos[0] = "daga_de_plaga";
    puestos[1] = "botas_de_marcha_forzada";
    const auto brillos = Game::Vestuario::brillosVisibles(puestos, m->manifiesto());
    MOTOR_CHECK(brillos.size() == 2);
    MOTOR_CHECK(brillos.at("arma").nivel == 4 && brillos.at("pies").nivel == 6);
}

MOTOR_TEST_CASE(brillo_sale_de_lo_que_se_ve) {
    auto m = montaje();
    const Vestido v = vestir(*m, {"daga_de_plaga"});
    // El mapa: una capa por cada capa del personaje, y cada pixel con algo dibujado
    // tiene su capa de arriba.
    MOTOR_CHECK(v.mapa.arriba.size() == static_cast<std::size_t>(v.hoja.ancho * v.hoja.alto));
    int conCapa = 0, opacos = 0;
    for (std::size_t i = 0; i < v.mapa.arriba.size(); ++i) {
        opacos += v.hoja.pixeles[i * 4 + 3] != 0 ? 1 : 0;
        conCapa += v.mapa.arriba[i] != 0 ? 1 : 0;
        MOTOR_CHECK(v.mapa.arriba[i] <= v.mapa.ranuras.size());
    }
    MOTOR_CHECK(opacos == conCapa);
    MOTOR_CHECK(std::find(v.mapa.ranuras.begin(), v.mapa.ranuras.end(), "arma") != v.mapa.ranuras.end());

    // La daga brilla: una fuente, con pixeles de frente (frame 8).
    MOTOR_CHECK(v.brillo.fuentes.size() == 1);
    const Game::FuenteDeBrillo& daga = v.brillo.fuentes[0];
    MOTOR_CHECK(daga.ranura == "arma" && !daga.esRopa);
    MOTOR_CHECK(!daga.pixeles[7].empty());
    // Cada pixel de la fuente es un pixel de la daga a la vista.
    const auto id = static_cast<std::uint8_t>(
        std::find(v.mapa.ranuras.begin(), v.mapa.ranuras.end(), "arma") - v.mapa.ranuras.begin() + 1);
    for (int f = 0; f < 12; ++f) {
        for (const auto& p : daga.pixeles[static_cast<std::size_t>(f)]) {
            const std::size_t i = static_cast<std::size_t>(p[1]) * v.hoja.ancho + static_cast<std::size_t>(f * 64 + p[0]);
            MOTOR_CHECK(v.mapa.arriba[i] == id);
        }
    }
}

MOTOR_TEST_CASE(brillo_el_halo_del_arma_y_el_de_la_ropa) {
    auto m = montaje();
    // Nivel 4 (halo de radio 1): la luz cae junto a la daga y en ningun otro sitio.
    const Vestido v = vestir(*m, {"daga_de_plaga"});
    const auto& daga = v.brillo.fuentes[0].pixeles[7];
    int lejos = 0, cerca = 0;
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            const float a = alfaHalo(v.brillo.halo, 7 * 64 + x, y);
            if (a <= 0.0f)
                continue;
            int d = 99;
            for (const auto& p : daga)
                d = std::min(d, std::max(std::abs(p[0] - x), std::abs(p[1] - y)));
            (d <= 2 ? cerca : lejos) += 1;
        }
    }
    MOTOR_CHECK(cerca > 0);
    MOTOR_CHECK(lejos == 0);
    // Del color de la plaga.
    const std::uint8_t* px = nullptr;
    for (int y = 0; y < 64 && px == nullptr; ++y)
        for (int x = 0; x < 64 && px == nullptr; ++x)
            if (v.brillo.halo.pixel(7 * 64 + x, y)[3] > 0) px = v.brillo.halo.pixel(7 * 64 + x, y);
    MOTOR_CHECK(px != nullptr && px[1] > px[0] && px[1] > px[2]);

    // La ropa (las botas de la Llama Eterna, nivel 6): el halo solo por fuera del cuerpo.
    const Vestido r = vestir(*m, {"botas_de_marcha_forzada"});
    MOTOR_CHECK(r.brillo.fuentes.size() == 1 && r.brillo.fuentes[0].esRopa);
    int fuera = 0;
    for (std::size_t i = 0; i < r.mapa.arriba.size(); ++i) {
        if (r.brillo.halo.pixeles[i * 4 + 3] == 0)
            continue;
        MOTOR_CHECK(r.hoja.pixeles[i * 4 + 3] == 0);  // nunca encima del cuerpo
        ++fuera;
    }
    MOTOR_CHECK(fuera > 0);
    // Un Tier 1 no tiene halo: solo destellos.
    const Vestido t1 = vestir(*m, {"daga_de_milicia"});
    MOTOR_CHECK(t1.brillo.fuentes.size() == 1);
    MOTOR_CHECK(std::all_of(t1.brillo.halo.pixeles.begin(), t1.brillo.halo.pixeles.end(),
                            [](std::uint8_t c) { return c == 0; }));
}

MOTOR_TEST_CASE(brillo_destellos_deterministas_que_nacen_en_la_pieza) {
    auto m = montaje();
    const Vestido v = vestir(*m, {"sable_ligero_de_vacio"});  // nivel 7: halo, destellos y el vacio
    std::vector<Game::Destello> a, b;
    bool cambia = false;
    int conParticulas = 0;
    for (int i = 0; i < 40; ++i) {
        const double t = 0.137 * i;
        a.clear();
        b.clear();
        Game::destellosDeBrillo(v.brillo, 8, t, a);
        Game::destellosDeBrillo(v.brillo, 8, t, b);
        MOTOR_CHECK(a.size() == b.size());
        for (std::size_t k = 0; k < a.size() && k < b.size(); ++k)
            MOTOR_CHECK(a[k].x == b[k].x && a[k].y == b[k].y && a[k].alfa == b[k].alfa);
        if (i > 0) {
            std::vector<Game::Destello> antes;
            Game::destellosDeBrillo(v.brillo, 8, 0.137 * (i - 1), antes);
            cambia = cambia || antes.size() != a.size() ||
                     (!a.empty() && (antes[0].x != a[0].x || antes[0].y != a[0].y || antes[0].alfa != a[0].alfa));
        }
        for (const Game::Destello& d : a) {
            MOTOR_CHECK(d.alfa > 0.0f && d.alfa <= 1.0f);
            // Cerca del sable: como mucho lo que sube o se mece una particula en su vida.
            MOTOR_CHECK(d.x > -12.0f && d.x < 76.0f && d.y > -30.0f && d.y < 80.0f);
            conParticulas += !d.suma ? 1 : 0;  // los destellos suman luz; el vacio la quita
        }
    }
    MOTOR_CHECK(cambia);
    MOTOR_CHECK(conParticulas > 0);  // los puntos oscuros que el vacio se traga
    // El latido del halo se queda entre 1 - latido y 1.
    const float latido = Game::escalonDeBrillo(7, false).latido;
    for (int i = 0; i < 20; ++i) {
        const float l = Game::latidoDeBrillo(v.brillo, 0.1 * i);
        MOTOR_CHECK(l >= 1.0f - latido - 1e-4f && l <= 1.0f + 1e-4f);
    }
    // Sin brillo, nada.
    Game::HojaDeBrillo vacio;
    std::vector<Game::Destello> nada;
    Game::destellosDeBrillo(vacio, 8, 1.0, nada);
    MOTOR_CHECK(nada.empty() && vacio.vacia());
}

MOTOR_TEST_CASE(brillo_los_abalorios_echan_desde_su_sitio) {
    // Oriol: lo que no se ve puesto da particulas, por lo que da o por su lore. PART-02b: «si un anillo
    // aporta un hechizo de fuego y uno de hielo, salgan sprites azules y rojos de la mano del pj»: el anillo
    // echa desde la mano que lo lleva, el collar desde el pecho y el cinturon desde la cintura.
    auto m = montaje();
    const auto& man = m->manifiesto();
    MOTOR_CHECK(man.piezaDeObjeto.count("anillo_de_vitalidad") == 0);  // no se ve...
    MOTOR_CHECK(man.brilloDeAbalorio.count("anillo_de_vitalidad") == 1);  // ...pero brilla
    MOTOR_CHECK(man.brilloDeAbalorio.at("argolla_del_toro").nivel == 6);  // reliquia de Tier 3
    MOTOR_CHECK(man.brilloDeAbalorio.at("sello_de_la_clausula_viva").particulas == "veneno");
    // El sitio, por la ranura del catalogo: el anillo, la mano del arma; el amuleto, el pecho.
    MOTOR_CHECK(man.brilloDeAbalorio.at("anillo_de_vitalidad").sitio == "mano");
    MOTOR_CHECK(man.brilloDeAbalorio.at("amuleto_de_foco").sitio == "pecho");

    // Las anclas de las cuatro tallas, en los 20 frames. El humano (talla media) de frente y quieto (frame 8):
    // el puno de la mano del arma a la izquierda de la imagen, el de la otra a la derecha, el pecho debajo
    // del cuello y la cintura en la linea del cinturon.
    for (const char* talla : {"medio", "bajo", "pequeno", "grande"}) {
        MOTOR_CHECK(man.anclas.count(talla) == 1);
        if (man.anclas.count(talla) == 1)
            MOTOR_CHECK(man.anclas.at(talla).mano.size() == 20 && man.anclas.at(talla).cintura.size() == 20);
    }
    const Game::AnclasDeTalla& medio = man.anclas.at("medio");
    MOTOR_CHECK((medio.mano[7] == Game::AnclasDeTalla::Punto{26, 48}));
    MOTOR_CHECK((medio.otraMano[7] == Game::AnclasDeTalla::Punto{38, 48}));
    MOTOR_CHECK((medio.pecho[7] == Game::AnclasDeTalla::Punto{32, 37}));
    MOTOR_CHECK((medio.cintura[7] == Game::AnclasDeTalla::Punto{32, 47}));
    MOTOR_CHECK(medio.de("pecho") == &medio.pecho && medio.de("cuerpo") == nullptr);

    // Puestos: el anillo en las dos manos y el amuleto en el cuello.
    RPG::EquipoPuesto puestos{};
    puestos[static_cast<std::size_t>(RPG::EquipSlot::MainHand)] = "item_short_sword";
    puestos[static_cast<std::size_t>(RPG::EquipSlot::Accessory)] = "anillo_de_vitalidad";
    puestos[static_cast<std::size_t>(RPG::EquipSlot::Ring2)] = "anillo_de_vitalidad";
    puestos[static_cast<std::size_t>(RPG::EquipSlot::Neck)] = "amuleto_de_foco";
    const auto brillos = Game::Vestuario::brillosVisibles(puestos, man);
    MOTOR_CHECK(brillos.count("abalorio:mano:anillo_de_vitalidad") == 1);
    MOTOR_CHECK(brillos.count("abalorio:otraMano:anillo_de_vitalidad") == 1);  // dos anillos, dos
    MOTOR_CHECK(brillos.count("abalorio:pecho:amuleto_de_foco") == 1);
    MOTOR_CHECK(brillos.at("abalorio:otraMano:anillo_de_vitalidad").sitio == "otraMano");
    MOTOR_CHECK(brillos.at("abalorio:mano:anillo_de_vitalidad").abalorio);

    // Montado: cada uno echa desde su ancla, sin halo. El mapa trae las anclas de la talla del cuerpo.
    Game::MapaDeCapas mapa;
    const auto& espada = man.piezaDeObjeto.at("item_short_sword");
    auto hoja = m->montar(Game::Apariencia::deRaza(*man.raza("race_human_marches")),
                          {{espada.ranura, espada.visible()}}, &mapa);
    MOTOR_CHECK(hoja.isOk());
    MOTOR_CHECK(!mapa.anclas.vacia() && mapa.anclas.mano[7] == medio.mano[7]);
    std::map<std::string, Game::BrilloDePieza> abalorios;
    for (const auto& [clave, b] : brillos)
        if (b.abalorio) abalorios[clave] = b;
    const Game::HojaDeBrillo b = Game::montarBrillo(hoja.value(), mapa, abalorios);
    MOTOR_CHECK(b.fuentes.size() == 3);
    auto cerca = [](const Game::FuenteDeBrillo& f, int frame, const Game::AnclasDeTalla::Punto& p, int lado) {
        const auto& pix = f.pixeles[static_cast<std::size_t>(frame - 1)];
        return !pix.empty() && std::all_of(pix.begin(), pix.end(), [&](const std::array<std::uint8_t, 2>& q) {
            return std::abs(q[0] - p[0]) <= lado && std::abs(q[1] - p[1]) <= 1;
        });
    };
    for (const Game::FuenteDeBrillo& f : b.fuentes) {
        MOTOR_CHECK(f.esAbalorio && f.parte == 1.0f && f.conDestellos);
        for (int frame = 1; frame <= 20; ++frame) {
            const auto i = static_cast<std::size_t>(frame - 1);
            if (f.ranura == "abalorio:mano:anillo_de_vitalidad") {
                MOTOR_CHECK(cerca(f, frame, medio.mano[i], 1));
            } else if (f.ranura == "abalorio:otraMano:anillo_de_vitalidad") {
                MOTOR_CHECK(cerca(f, frame, medio.otraMano[i], 1));
            } else {
                MOTOR_CHECK(f.ranura == "abalorio:pecho:amuleto_de_foco");
                MOTOR_CHECK(cerca(f, frame, medio.pecho[i], 1));
            }
        }
        MOTOR_CHECK(f.pixeles[7].size() == 9);  // un puno: 3x3 alrededor del ancla
    }
    MOTOR_CHECK(std::all_of(b.halo.pixeles.begin(), b.halo.pixeles.end(), [](std::uint8_t c) { return c == 0; }));
    // Y sus particulas nacen alli: las del anillo de la mano principal, al lado del puno (frame 8).
    std::map<std::string, Game::BrilloDePieza> soloElAnillo{
        {"abalorio:mano:anillo_de_vitalidad", brillos.at("abalorio:mano:anillo_de_vitalidad")}};
    const Game::HojaDeBrillo anillo = Game::montarBrillo(hoja.value(), mapa, soloElAnillo);
    int particulas = 0, lejos = 0;
    for (int i = 0; i < 30; ++i) {
        std::vector<Game::Destello> d;
        Game::destellosDeBrillo(anillo, 8, 0.2 * i, d);
        for (const auto& x : d) {
            particulas += 1;
            lejos += std::abs(x.x - 26.0f) > 6.0f || x.y > 50.0f || x.y < 26.0f ? 1 : 0;
        }
    }
    MOTOR_CHECK(particulas > 0 && lejos == 0);

    // El cinturon, a lo ancho de la linea del cinturon (7x2).
    Game::BrilloDePieza cinturon;
    cinturon.nivel = 3;
    cinturon.abalorio = true;
    cinturon.sitio = "cintura";
    const Game::HojaDeBrillo cint = Game::montarBrillo(hoja.value(), mapa, {{"abalorio:cintura:prueba", cinturon}});
    MOTOR_CHECK(cint.fuentes.size() == 1 && cint.fuentes[0].pixeles[7].size() == 14);
    MOTOR_CHECK(cerca(cint.fuentes[0], 8, medio.cintura[7], 3));

    // Sin anclas (un manifiesto sin ellas) o sin sitio, del cuerpo entero, como antes.
    Game::MapaDeCapas sinAnclas = mapa;
    sinAnclas.anclas = Game::AnclasDeTalla{};
    const Game::HojaDeBrillo todo = Game::montarBrillo(hoja.value(), sinAnclas, soloElAnillo);
    std::size_t cuerpo = 0;
    for (int y = 0; y < 64; ++y)
        for (int x = 0; x < 64; ++x)
            cuerpo += hoja.value().pixel(7 * 64 + x, y)[3] != 0 ? 1 : 0;
    MOTOR_CHECK(todo.fuentes.size() == 1 && todo.fuentes[0].pixeles[7].size() == cuerpo);

    // Aunque sea de Tier 1, echa particulas (un arma de Tier 1 solo destella).
    MOTOR_CHECK(Game::escalonDeAbalorio(1).particulas > 0.0f && Game::escalonDeBrillo(1, false).particulas == 0.0f);
    MOTOR_CHECK(Game::escalonDeAbalorio(6).particulas > Game::escalonDeAbalorio(1).particulas);
}

// PART-02b — Un abalorio con dos efectos (un hechizo de fuego y uno de hielo) echa las particulas de los dos
// desde su sitio, a medias: llamas rojas y copos azules de la mano.
MOTOR_TEST_CASE(brillo_un_anillo_de_dos_efectos_echa_los_dos) {
    auto m = montaje();
    const auto& man = m->manifiesto();
    Game::MapaDeCapas mapa;
    auto hoja = m->montar(Game::Apariencia::deRaza(*man.raza("race_human_marches")), {}, &mapa);
    MOTOR_CHECK(hoja.isOk());
    Game::BrilloDePieza anillo;
    anillo.nivel = 6;
    anillo.abalorio = true;
    anillo.sitio = "mano";
    anillo.particulas = "llamas";
    anillo.color = {255, 90, 40};
    anillo.otrasParticulas = "escarcha";
    anillo.otroColor = {120, 200, 255};
    const Game::HojaDeBrillo doble = Game::montarBrillo(hoja.value(), mapa, {{"abalorio:mano:fuego_y_hielo", anillo}});
    MOTOR_CHECK(doble.fuentes.size() == 2);
    MOTOR_CHECK(doble.fuentes[0].brillo.particulas == "llamas" && doble.fuentes[1].brillo.particulas == "escarcha");
    MOTOR_CHECK(doble.fuentes[1].brillo.color == anillo.otroColor);
    MOTOR_CHECK(doble.fuentes[0].parte == 0.5f && doble.fuentes[1].parte == 0.5f);
    MOTOR_CHECK(doble.fuentes[0].conDestellos && !doble.fuentes[1].conDestellos);  // los destellos, una vez
    MOTOR_CHECK(doble.fuentes[0].pixeles[7] == doble.fuentes[1].pixeles[7]);       // del mismo sitio
    // Con uno solo, las mismas particulas en total (poco mas o menos: cada mitad redondea).
    Game::BrilloDePieza soloFuego = anillo;
    soloFuego.otrasParticulas.clear();
    const Game::HojaDeBrillo uno = Game::montarBrillo(hoja.value(), mapa, {{"abalorio:mano:fuego", soloFuego}});
    int llamas = 0, copos = 0, otras = 0, deUno = 0;
    for (int i = 0; i < 60; ++i) {
        std::vector<Game::Destello> d, u;
        Game::destellosDeBrillo(doble, 8, 0.1 * i, d);
        Game::destellosDeBrillo(uno, 8, 0.1 * i, u);
        for (const auto& x : d) {
            if (x.forma == Game::FormaDeParticula::Llama) ++llamas;
            else if (x.forma == Game::FormaDeParticula::Copo) ++copos;
            else ++otras;
        }
        for (const auto& x : u)
            deUno += x.forma == Game::FormaDeParticula::Llama ? 1 : 0;
    }
    MOTOR_CHECK(llamas > 0 && copos > 0);
    MOTOR_CHECK(llamas + copos <= deUno + deUno / 2 + 60 && llamas + copos >= deUno / 2);
    (void)otras;

    // PART-02c: el del catalogo (tools/abalorios_t2.py), puesto en la mano del arma; y un collar y un
    // cinturon de los temas de las clases, desde el pecho y la cintura.
    const auto& abalorios = man.brilloDeAbalorio;
    MOTOR_CHECK(abalorios.count("anillo_de_fuego_y_escarcha") == 1);
    if (abalorios.count("anillo_de_fuego_y_escarcha") == 1) {
        const Game::BrilloDePieza& fh = abalorios.at("anillo_de_fuego_y_escarcha");
        MOTOR_CHECK(fh.particulas == "llamas" && fh.otrasParticulas == "escarcha" && fh.sitio == "mano");
        MOTOR_CHECK(fh.nivel == 4);  // Tier 2, +1 y +1
    }
    MOTOR_CHECK(abalorios.count("colgante_del_diapason") == 1 && abalorios.at("colgante_del_diapason").sitio == "pecho");
    MOTOR_CHECK(abalorios.count("cinturon_del_mercader") == 1 &&
                abalorios.at("cinturon_del_mercader").sitio == "cintura" &&
                abalorios.at("cinturon_del_mercader").particulas == "monedas");
    RPG::EquipoPuesto puestos{};
    puestos[static_cast<std::size_t>(RPG::EquipSlot::Ring2)] = "anillo_de_fuego_y_escarcha";
    puestos[static_cast<std::size_t>(RPG::EquipSlot::Neck)] = "colgante_del_diapason";
    puestos[static_cast<std::size_t>(RPG::EquipSlot::Belt)] = "cinturon_del_mercader";
    const auto brillos = Game::Vestuario::brillosVisibles(puestos, man);
    MOTOR_CHECK(brillos.count("abalorio:otraMano:anillo_de_fuego_y_escarcha") == 1);
    const Game::HojaDeBrillo tres = Game::montarBrillo(hoja.value(), mapa, brillos);
    MOTOR_CHECK(tres.fuentes.size() == 4);  // el anillo son dos (fuego y escarcha), el collar y el cinturon
}

// PART-01 — Las particulas de los elementos y de las clases (tools/particulas.py,
// planificacion/CANON_PARTICULAS.md): cada familia con su forma, sus colores y su movimiento.
namespace {

const std::array<std::uint8_t, 3> kColorDePrueba{10, 200, 30};

// Un abalorio de prueba (echa particulas a cualquier nivel): un cuadrado de 5x5 alrededor de (32, 32)
// en el frame 8 que echa particulas de `familia`.
Game::HojaDeBrillo cuadradoQueBrilla(const std::string& familia, int nivel = 8) {
    Game::FuenteDeBrillo f;
    f.ranura = "abalorio:prueba";
    f.esAbalorio = true;
    f.brillo.nivel = nivel;
    f.brillo.color = kColorDePrueba;
    f.brillo.particulas = familia;
    f.brillo.abalorio = true;
    for (int y = 30; y <= 34; ++y)
        for (int x = 30; x <= 34; ++x)
            f.pixeles[7].push_back({static_cast<std::uint8_t>(x), static_cast<std::uint8_t>(y)});
    Game::HojaDeBrillo b;
    b.fuentes.push_back(f);
    return b;
}

// Los destellos (no las particulas): una cruz blanca que suma luz.
bool esDestello(const Game::Destello& d) {
    return d.forma == Game::FormaDeParticula::Cruz && d.suma && d.color == std::array<std::uint8_t, 3>{255, 255, 240};
}

}  // namespace

MOTOR_TEST_CASE(brillo_las_familias_de_particulas_de_los_elementos_y_las_clases) {
    using Game::FormaDeParticula;
    using M = Game::MovimientoDeParticula;
    const std::vector<std::string> familias = Game::familiasDeParticulas();
    MOTOR_CHECK(familias.size() == 40);  // las de tools/particulas.py (test_validar_sprites compara los nombres)
    for (const char* n : {"llamas", "agua", "escarcha", "rayos", "piedras", "viento", "luz", "sombra", "hojas", "veneno",
                          "sangre", "arcano", "vacio", "llamas_puras", "dragones", "petalos", "furia", "oro", "notas",
                          "naipes", "monedas", "engranajes", "animas", "espiritus"})
        MOTOR_CHECK(std::find(familias.begin(), familias.end(), n) != familias.end());

    // Cada forma tiene su dibujo, y las de PART-01 no son un punto suelto.
    for (int f = 0; f <= static_cast<int>(FormaDeParticula::MonedaDeCanto); ++f) {
        const auto& px = Game::pixelesDeForma(static_cast<FormaDeParticula>(f));
        MOTOR_CHECK(!px.empty());
        if (f >= static_cast<int>(FormaDeParticula::Llama))
            MOTOR_CHECK(px.size() >= 2);  // la moneda de canto: dos; las demas, tres o mas
        // El contorno de las de tema: alrededor de la forma, sin pisarla y sin repetirse; las demas, sin el.
        const auto& borde = Game::contornoDeForma(static_cast<FormaDeParticula>(f));
        for (std::size_t i = 0; i < borde.size(); ++i) {
            for (const auto& p : px)
                MOTOR_CHECK(!(p.dx == borde[i].dx && p.dy == borde[i].dy));
            bool toca = false;
            for (const auto& p : px)
                toca = toca || std::abs(p.dx - borde[i].dx) + std::abs(p.dy - borde[i].dy) == 1;
            MOTOR_CHECK(toca);
            for (std::size_t j = i + 1; j < borde.size(); ++j)
                MOTOR_CHECK(!(borde[i].dx == borde[j].dx && borde[i].dy == borde[j].dy));
        }
    }
    MOTOR_CHECK(!Game::contornoDeForma(FormaDeParticula::Dragon).empty());
    MOTOR_CHECK(!Game::contornoDeForma(FormaDeParticula::Nota).empty());
    MOTOR_CHECK(Game::contornoDeForma(FormaDeParticula::Punto).empty());
    MOTOR_CHECK(Game::contornoDeForma(FormaDeParticula::Cruz).empty());  // los destellos, como siempre
    // Oriol, revisando la lista: la calavera lleva contorno (los ojos, oscuros); la estrella y los trazos, no.
    MOTOR_CHECK(Game::contornoDeForma(FormaDeParticula::Calavera).size() > 4);
    MOTOR_CHECK(Game::contornoDeForma(FormaDeParticula::Estrella).empty());
    MOTOR_CHECK(Game::contornoDeForma(FormaDeParticula::TrazoNE).empty());

    // Oriol: «no deberia tener las mismas particulas en los puños un artes misticas que un puño de dragon».
    using Game::tipoDeParticula;
    MOTOR_CHECK(tipoDeParticula("dragones").forma == FormaDeParticula::Dragon);
    MOTOR_CHECK(tipoDeParticula("petalos").forma == FormaDeParticula::Petalo);
    MOTOR_CHECK(tipoDeParticula("llamas").forma == FormaDeParticula::Llama);
    MOTOR_CHECK(tipoDeParticula("llamas_puras").colores != tipoDeParticula("llamas").colores);  // el druida flamigero
    MOTOR_CHECK(tipoDeParticula("sangre").colores.front() == (std::array<std::uint8_t, 3>{0xd2, 0x28, 0x28}));
    MOTOR_CHECK(tipoDeParticula("agua").movimiento == M::Cae && tipoDeParticula("luz").movimiento == M::Sube);
    MOTOR_CHECK(tipoDeParticula("vacio").movimiento == M::Converge && tipoDeParticula("arcano").movimiento == M::Orbita);
    MOTOR_CHECK(tipoDeParticula("ondas").movimiento == M::Expande && tipoDeParticula("rayos").movimiento == M::Chispazo);
    MOTOR_CHECK(tipoDeParticula("viento").movimiento == M::Deriva);
    // Oriol, revisando la lista: el veneno no puede parecer una cura (calaveras verdes que tapan); las estrellas
    // y el oro, la estrella de cuatro puntas (el oro, el doble: mas intenso); las chispas, trazos que salpican;
    // las monedas, con cruces doradas y girando; los engranajes, girando.
    MOTOR_CHECK(tipoDeParticula("veneno").forma == FormaDeParticula::Calavera && !tipoDeParticula("veneno").suma);
    MOTOR_CHECK(tipoDeParticula("estrellas").forma == FormaDeParticula::Estrella);
    MOTOR_CHECK(tipoDeParticula("oro").forma == FormaDeParticula::Estrella && tipoDeParticula("oro").densidad >= 2.0f);
    MOTOR_CHECK(tipoDeParticula("chispas").movimiento == M::Salpica);
    MOTOR_CHECK(tipoDeParticula("monedas").gira && tipoDeParticula("monedas").otraForma == FormaDeParticula::Estrella &&
                tipoDeParticula("monedas").conOtraForma > 0.0f);
    MOTOR_CHECK(tipoDeParticula("engranajes").gira);
    // Ninguna familia es otra con otro nombre.
    for (std::size_t i = 0; i < familias.size(); ++i) {
        for (std::size_t j = i + 1; j < familias.size(); ++j) {
            const auto a = tipoDeParticula(familias[i]), b = tipoDeParticula(familias[j]);
            MOTOR_CHECK(!(a.forma == b.forma && a.movimiento == b.movimiento && a.suma == b.suma &&
                          a.colores == b.colores && a.vy == b.vy && a.vx == b.vx && a.vida == b.vida));
        }
    }

    // Y se mueven como dicen: cada particula sale de la pieza con su forma, su color y su camino.
    for (const std::string& nombre : familias) {
        const Game::TipoDeParticula tipo = tipoDeParticula(nombre);
        MOTOR_CHECK(tipo.vida > 0.0f);
        const Game::HojaDeBrillo b = cuadradoQueBrilla(nombre);
        int vistas = 0;
        std::vector<FormaDeParticula> formasVistas;
        // las formas que puede llevar: la suya, la otra (si la tiene), su otra cara (si gira) y, si salpica, el
        // trazo hacia donde vuele
        std::vector<FormaDeParticula> validas = {tipo.movimiento == M::Expande ? FormaDeParticula::Punto : tipo.forma};
        if (tipo.conOtraForma > 0.0f)
            validas.push_back(tipo.otraForma);
        if (tipo.gira)
            validas.insert(validas.end(), {FormaDeParticula::EngranajeGirado, FormaDeParticula::MonedaDeCanto});
        if (tipo.movimiento == M::Salpica)
            validas.insert(validas.end(), {FormaDeParticula::TrazoNO, FormaDeParticula::TrazoSE, FormaDeParticula::TrazoSO});
        for (int i = 0; i < 80; ++i) {
            std::vector<Game::Destello> out;
            Game::destellosDeBrillo(b, 8, 0.05 * i, out);
            std::vector<Game::Destello> suyas;
            for (const Game::Destello& d : out)
                if (!esDestello(d))
                    suyas.push_back(d);
            if (tipo.movimiento == M::Expande)
                MOTOR_CHECK(suyas.size() % 10 == 0);  // aros de diez puntos
            for (const Game::Destello& d : suyas) {
                ++vistas;
                MOTOR_CHECK(std::find(validas.begin(), validas.end(), d.forma) != validas.end());
                if (std::find(formasVistas.begin(), formasVistas.end(), d.forma) == formasVistas.end())
                    formasVistas.push_back(d.forma);
                MOTOR_CHECK(d.suma == tipo.suma);
                if (tipo.colores.empty())
                    MOTOR_CHECK(d.color == kColorDePrueba);  // sin colores propios: el de la pieza
                else
                    MOTOR_CHECK(std::find(tipo.colores.begin(), tipo.colores.end(), d.color) != tipo.colores.end());
                MOTOR_CHECK(d.alfa >= 0.0f && d.alfa <= 1.0f);
                const float dx = d.x - 32.0f, dy = d.y - 32.0f;
                switch (tipo.movimiento) {
                    case M::Sube:
                        MOTOR_CHECK(d.y <= 34.0f + 1e-3f);
                        break;
                    case M::Cae:
                        MOTOR_CHECK(d.y >= 30.0f - 1e-3f);
                        break;
                    case M::Converge:  // nace a 6-10 px de su pixel y cae hacia el
                        MOTOR_CHECK(std::hypot(dx, dy) <= 13.0f);
                        break;
                    case M::Orbita:  // un anillo aplastado alrededor del centro de la pieza
                        MOTOR_CHECK(std::abs(dx) <= 8.01f && std::abs(dy) <= 4.01f + std::abs(tipo.vy) * tipo.vida);
                        break;
                    case M::Expande:
                        MOTOR_CHECK(std::hypot(dx, dy) <= 9.9f);
                        break;
                    case M::Chispazo:  // el rayo: un instante, joven y en su pixel
                        MOTOR_CHECK(d.alfa >= 1.0f - 0.12f / tipo.vida - 1e-3f);
                        MOTOR_CHECK(std::abs(dx) <= 2.0f && std::abs(dy) <= 2.0f);
                        break;
                    case M::Deriva:  // de lado
                        MOTOR_CHECK(std::abs(dy) <= 2.0f + std::abs(tipo.vy) * tipo.vida + 1e-3f);
                        MOTOR_CHECK(std::abs(dx) <= 2.6f + tipo.vx * tipo.vida + 1e-3f);
                        break;
                    case M::Salpica:  // sube hasta un cuarto de lo que volaria y vuelve a su altura
                        MOTOR_CHECK(std::abs(dx) <= 2.0f + tipo.vx * tipo.vida + 1e-3f);
                        MOTOR_CHECK(dy >= -2.0f - std::abs(tipo.vy) * tipo.vida / 4.0f - 1e-3f && dy <= 2.0f + 1e-3f);
                        break;
                }
            }
        }
        MOTOR_CHECK(vistas > 0);
        // las que cambian de forma, cambian: las monedas giran y brillan, los engranajes giran, las chispas vuelan
        // hacia los dos lados, subiendo y cayendo
        if (tipo.gira || tipo.conOtraForma > 0.0f)
            MOTOR_CHECK(formasVistas.size() >= 2);
        if (tipo.movimiento == M::Salpica)
            MOTOR_CHECK(formasVistas.size() >= 3);
    }
}

// PART-01 (Oriol, con la prueba del Cazademonios por tier): «a tier 3 un poco mas opacas, pero un pelo; en
// tier 4 mas intensidad, y tier 5 mas aun», y en el 5 «otras burbujas de un color mas oscuro para darle
// contraste».
MOTOR_TEST_CASE(brillo_las_particulas_suben_con_el_tier) {
    using Game::escalonDeAbalorio;
    using Game::escalonDeBrillo;
    for (int n = 0; n <= 4; ++n)
        MOTOR_CHECK(escalonDeBrillo(n, false).opacidad == 1.0f && escalonDeBrillo(n, false).contraste == 0.0f);
    for (int n = 4; n < 10; ++n) {
        MOTOR_CHECK(escalonDeBrillo(n + 1, false).opacidad > escalonDeBrillo(n, false).opacidad);
        // la ropa y los abalorios, igual de intensos que un arma de su nivel
        MOTOR_CHECK(escalonDeBrillo(n, true).opacidad == escalonDeBrillo(n, false).opacidad);
        MOTOR_CHECK(escalonDeAbalorio(n).opacidad == escalonDeBrillo(n, false).opacidad);
    }
    MOTOR_CHECK(escalonDeBrillo(6, false).opacidad <= 1.2f);   // Tier 3: un pelo
    MOTOR_CHECK(escalonDeBrillo(7, false).opacidad >= 1.3f);   // Tier 4: mas
    MOTOR_CHECK(escalonDeBrillo(9, false).opacidad >= 1.6f);   // Tier 5: mas aun
    for (int n = 0; n <= 10; ++n) {
        MOTOR_CHECK((escalonDeBrillo(n, false).contraste > 0.0f) == (n >= 9));
        MOTOR_CHECK((escalonDeBrillo(n, true).contraste > 0.0f) == (n >= 9));
        MOTOR_CHECK((escalonDeAbalorio(n).contraste > 0.0f) == (n >= 9));
    }

    // En el motor: la misma familia de luz (que suma), en un +6 y en un +10.
    auto mira = [](int nivel, double& alfaMedia, int& oscuras, int& total) {
        const Game::HojaDeBrillo b = cuadradoQueBrilla("luz", nivel);
        double suma = 0.0;
        oscuras = total = 0;
        for (int i = 0; i < 120; ++i) {
            std::vector<Game::Destello> out;
            Game::destellosDeBrillo(b, 8, 0.05 * i, out);
            for (const Game::Destello& d : out) {
                if (esDestello(d))
                    continue;
                MOTOR_CHECK(d.alfa >= 0.0f && d.alfa <= 1.0f);
                ++total;
                suma += d.alfa;
                if (!d.suma) {
                    ++oscuras;  // de contraste: el color de la pieza, oscurecido
                    MOTOR_CHECK(d.color[0] < kColorDePrueba[0] + 1 && d.color[1] < kColorDePrueba[1] &&
                                d.color[2] < kColorDePrueba[2]);
                }
            }
        }
        alfaMedia = total > 0 ? suma / total : 0.0;
    };
    double alfa6 = 0.0, alfa10 = 0.0;
    int oscuras6 = 0, oscuras10 = 0, total6 = 0, total10 = 0;
    mira(6, alfa6, oscuras6, total6);
    mira(10, alfa10, oscuras10, total10);
    MOTOR_CHECK(total6 > 0 && total10 > total6);  // mas, y...
    MOTOR_CHECK(alfa10 > alfa6 + 0.05);           // ...mas vivas
    MOTOR_CHECK(oscuras6 == 0);                   // sin contraste por debajo del Tier 5
    MOTOR_CHECK(oscuras10 > total10 / 6 && oscuras10 < total10 / 2);  // en el 5, unas cuantas (un tercio)
}

int main() { return motor_test::runAllTests(); }

// PART-02d — Las cartas al jugarse en el combate: cada una echa las suyas (tools/particulas_cartas.py, la regla
// de CANON_PARTICULAS §3) y, si no es de ninguna clase, las de su elemento. Brotan del cuerpo de quien la
// recibe, entran deprisa y se apagan.
MOTOR_TEST_CASE(brillo_las_cartas_jugadas_echan_las_suyas) {
    auto mapa = Game::ParticulasDeCartas::cargar(std::string(kTexturas) + "/particulas_cartas.json");
    MOTOR_CHECK(mapa.isOk());
    if (!mapa.isOk()) return;
    const Game::ParticulasDeCartas& p = mapa.value();
    MOTOR_CHECK(p.cartas() > 900);
    MOTOR_CHECK(p.familiaDe("puno_del_dragon_aranazo", {"Fuego"}) == "dragones");          // no las llamas
    MOTOR_CHECK(p.familiaDe("druida_flamigero_ataque_en_salto", {"Fuego"}) == "llamas_puras");
    MOTOR_CHECK(p.familiaDe("cancion_del_otono", {}) == "notas");
    MOTOR_CHECK(p.familiaDe("absorcion_de_alma", {}) == "sangre");                          // el Apostata
    // Sin clase: la de su elemento (y sus alias); sin elemento, ninguna.
    MOTOR_CHECK(p.familiaDe("carta_de_un_objeto", {"Arma", "Fuego"}) == "llamas");
    MOTOR_CHECK(p.familiaDe("carta_de_un_objeto", {"Sagrado"}) == "luz");
    MOTOR_CHECK(p.familiaDe("carta_de_un_objeto", {"Oscuro"}) == "sombra");
    MOTOR_CHECK(p.familiaDe("carta_de_un_objeto", {"Arma"}).empty());
    MOTOR_CHECK(Game::familiaDeElemento("Hielo") == "escarcha" && Game::familiaDeElemento("Vacío") == "vacio");
    // Los colores de las que no traen los suyos: la luz, calida; el agua, azul.
    const auto luz = p.colorDe("luz"), agua = p.colorDe("agua");
    MOTOR_CHECK(luz[0] > 200 && luz[1] > 200 && agua[2] > agua[0]);

    // Lo que se ve: deterministas, con la forma de su familia, sobre el cuerpo, y se apagan al final.
    auto ver = [](const std::string& familia, double edad) {
        std::vector<Game::Destello> d;
        Game::destellosDeCarta(familia, {255, 240, 210}, 7u, edad, d);
        return d;
    };
    const auto a = ver("dragones", 0.5), b = ver("dragones", 0.5);
    MOTOR_CHECK(!a.empty() && a.size() == b.size());
    for (std::size_t i = 0; i < a.size() && i < b.size(); ++i)
        MOTOR_CHECK(a[i].x == b[i].x && a[i].y == b[i].y && a[i].alfa == b[i].alfa);
    MOTOR_CHECK(std::any_of(a.begin(), a.end(), [](const Game::Destello& d) {
        return d.forma == Game::FormaDeParticula::Dragon;
    }));
    MOTOR_CHECK(std::all_of(a.begin(), a.end(), [](const Game::Destello& d) {
        return d.x > 10.0f && d.x < 54.0f && d.y > 0.0f && d.y < 64.0f;  // alrededor del cuerpo
    }));
    auto suma = [](const std::vector<Game::Destello>& d) {
        float s = 0.0f;
        for (const auto& x : d) s += x.alfa;
        return s;
    };
    MOTOR_CHECK(suma(ver("dragones", 1.35)) < suma(ver("dragones", 0.6)));  // se apagan
    MOTOR_CHECK(ver("dragones", Game::kDuracionDeCarta).empty() && ver("dragones", -0.1).empty());
    MOTOR_CHECK(ver("", 0.5).empty());
    // Mas que las de un abalorio de su nivel: es un estallido.
    MOTOR_CHECK(ver("llamas", 0.5).size() > 6);
}
