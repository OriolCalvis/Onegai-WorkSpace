// PNJ-02 — Las reglas de los PNJ que andan (Game::RutinasPnj), sin GL.
// El catalogo de prueba va aqui dentro (kCatalogo), no en assets/catalogs: el
// del juego lo escribe el carril pnj_datos. De ese solo se mira que se lea.
#include "TestHarness.h"

#include "Game/GameSession.h"
#include "Game/RutinasPnj.h"
#include "Game/SaveGame.h"
#include "Level/ObjectCatalog.h"
#include "Render/ICombatant.h"
#include "World/World.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <set>
#include <string>
#include <utility>
#include <vector>

using Game::CatalogoRutinasPnj;
using Game::EstadoPnj;
using Game::MundoPnj;
using Game::RutinasPnj;

namespace {

const char* kCatalogo = R"({
  "version": 1,
  "porDefecto": { "rutina": "deambular", "radio": 2, "pausa": [1, 2], "mira": "jugador" },
  "rutinas": {
    "_nota": "ignorada",
    "mercader": { "rutina": "actividad", "radio": 1, "pausa": [4, 9], "mira": "calle",
                  "actividad": { "nombre": "pregonar", "movimiento": "vaiven", "icono": "oro", "cada": [6, 12] },
                  "frases": ["¡Fruta fresca!", "¡Mirad, mirad!", "Esta frase es demasiado larga para un bocadillo de verdad"] },
    "guardia": { "rutina": "vigilar", "radio": 0, "pausa": [5, 10], "mira": "giro" },
    "npc_race_*": { "rutina": "deambular", "radio": 3, "pausa": [1, 2] },
    "npc_*": { "rutina": "quieto" },
    "npc_race_elfo": { "rutina": "charlar", "radio": 1 },
    "herrero": { "rutina": "actividad", "radio": 0, "mira": "fijo:izquierda",
                 "actividad": { "nombre": "martillo", "movimiento": "golpe", "icono": "", "cada": [1, 1] } },
    "cartero": { "rutina": "ir_y_volver", "destino": [4, 0], "pausa": [1, 1] }
  }
})";

CatalogoRutinasPnj catalogo() {
    auto r = CatalogoRutinasPnj::desdeTexto(kCatalogo);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

ObjectCatalog objetos() {
    ObjectCatalog c;
    for (const char* id : {"aldeano", "mercader", "guardia", "herrero", "cartero", "npc_race_elfo", "npc_race_enano"}) {
        ObjectDefinition d;
        d.id = id;
        d.name = id;
        d.category = ObjectCategory::Npc;
        d.blocksMovement = true;
        d.dialogue.lines = {"Hola."};
        c.add(d);
    }
    ObjectDefinition puerta;
    puerta.id = "puerta";
    puerta.name = "Puerta";
    c.add(puerta);
    return c;
}

Engine::EntityHandle pon(Engine::World& w, const std::string& id, GridCoord p, const std::string& destino = "") {
    ObjectSpawn s;
    s.objectId = id;
    s.position = s.patrolMin = s.patrolMax = p;
    s.targetLevel = destino;
    const Engine::EntityHandle h = w.createObject(s);
    w.flushPendingChanges();
    return h;
}

int lejos(GridCoord a, GridCoord b) { return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y)); }
bool igual(GridCoord a, GridCoord b) { return a.x == b.x && a.y == b.y; }

// El mundo de las pruebas: un plano sin muros salvo los que se digan, y los objetos del World.
MundoPnj mundoDe(const Engine::World& w, GridCoord jugador, const std::set<std::pair<int, int>>& muros = {}) {
    MundoPnj m;
    m.jugador = jugador;
    m.libre = [&w, muros](const GridCoord& c) {
        return muros.count({c.x, c.y}) == 0 && !w.findObjectAt(c.x, c.y).valid() && !w.findEnemyAt(c.x, c.y).valid();
    };
    return m;
}

// Anda `segundos` en pasos de 1/30 s y mira cada fotograma con `cada`.
template <typename F>
void anda(RutinasPnj& r, Engine::World& w, const MundoPnj& m, float segundos, F cada) {
    for (int i = 0; i < static_cast<int>(segundos * 30.0f); ++i) {
        r.avanza(1.0f / 30.0f, w, m);
        cada();
    }
}

class Cuerpo : public ICombatant {
public:
    void takeDamage(int) override {}
    void heal(int) override {}
    int health() const override { return 10; }
    int maxHealth() const override { return 10; }
    bool isAlive() const override { return true; }
    int stat(RPG::Stat) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "heroe"; }
};

}  // namespace

MOTOR_TEST_CASE(sin_catalogo_todo_por_defecto_y_no_falla) {
    MOTOR_CHECK(!CatalogoRutinasPnj::carga("no/existe/rutinas_pnj.json").isOk());
    MOTOR_CHECK(!CatalogoRutinasPnj::desdeTexto("{ esto no es json").isOk());
    const auto vacio = CatalogoRutinasPnj::desdeTexto("{}");
    MOTOR_CHECK(vacio.isOk());
    const Game::DefRutinaPnj& d = vacio.value().para("quien_sea");
    MOTOR_CHECK(d.rutina == Game::RutinaPnj::Deambular && d.radio == 2);
    MOTOR_CHECK(d.pausaMin == 3.0f && d.pausaMax == 8.0f && d.mira == Game::MiraPnj::Jugador);
    // Sin fichero (las pruebas no corren donde esta el del juego, o aun no existe): no rompe nada.
    (void)CatalogoRutinasPnj::delJuego().para("aldeano");
    // Y un nivel sin "carriles", o que no existe, no tiene carriles.
    MOTOR_CHECK(RutinasPnj::leeCarriles("no/existe.json").empty());
}

MOTOR_TEST_CASE(catalogo_claves_exactas_y_prefijos) {
    const CatalogoRutinasPnj c = catalogo();
    MOTOR_CHECK(c.para("mercader").rutina == Game::RutinaPnj::Actividad);
    MOTOR_CHECK(c.para("mercader").movimiento == Game::MovimientoPnj::Vaiven);
    MOTOR_CHECK(c.para("mercader").icono == "oro" && c.para("mercader").mira == Game::MiraPnj::Calle);
    MOTOR_CHECK(c.para("mercader").frases.size() == 2);  // la de mas de 40 letras no entra
    MOTOR_CHECK(c.para("guardia").rutina == Game::RutinaPnj::Vigilar && c.para("guardia").radio == 0);
    MOTOR_CHECK(c.para("npc_race_elfo").rutina == Game::RutinaPnj::Charlar);       // gana la exacta
    MOTOR_CHECK(c.para("npc_race_enano").rutina == Game::RutinaPnj::Deambular);    // el prefijo mas largo
    MOTOR_CHECK(c.para("npc_race_enano").radio == 3);
    MOTOR_CHECK(c.para("npc_tabernero").rutina == Game::RutinaPnj::Quieto);        // el corto
    MOTOR_CHECK(c.para("npc_tabernero").pausaMax == 2.0f);                          // lo demas, de porDefecto
    MOTOR_CHECK(c.para("herrero").mira == Game::MiraPnj::Fijo && c.para("herrero").rumboFijo == 3);
    MOTOR_CHECK(c.para("cartero").destino.x == 4 && c.para("cartero").destino.y == 0);
    MOTOR_CHECK(c.para("_nota").rutina == Game::RutinaPnj::Deambular);  // los comentarios no son PNJ
}

MOTOR_TEST_CASE(no_pisa_puertas_ni_la_casilla_de_delante) {
    Engine::World w;
    const ObjectCatalog obj = objetos();
    pon(w, "puerta", GridCoord{6, 5}, "assets/levels/interior.json");
    const auto h = pon(w, "aldeano", GridCoord{5, 5});  // radio 2: la puerta le cae dentro
    RutinasPnj r;
    r.monta(w, obj, catalogo(), {}, 7);
    const MundoPnj m = mundoDe(w, GridCoord{20, 20});
    int pasos = 0;
    GridCoord antes = r.de(h)->casilla;
    anda(r, w, m, 400.0f, [&] {
        const GridCoord c = r.de(h)->casilla;
        MOTOR_CHECK(!(c.x == 6 && c.y == 5));
        MOTOR_CHECK(!r.prohibida(c) || igual(c, GridCoord{5, 5}));  // nacio al lado: su sitio vale
        if (!igual(c, antes)) ++pasos, antes = c;
    });
    // Nacer junto a una puerta lo deja clavado: no se mueve de ahi.
    MOTOR_CHECK(pasos == 0);

    Engine::World w2;
    pon(w2, "puerta", GridCoord{6, 5}, "assets/levels/interior.json");
    const auto h2 = pon(w2, "aldeano", GridCoord{3, 5});
    RutinasPnj r2;
    r2.monta(w2, obj, catalogo(), {}, 7);
    const MundoPnj m2 = mundoDe(w2, GridCoord{20, 20});
    pasos = 0;
    antes = r2.de(h2)->casilla;
    anda(r2, w2, m2, 400.0f, [&] {
        const GridCoord c = r2.de(h2)->casilla;
        MOTOR_CHECK(!r2.prohibida(c));
        if (!igual(c, antes)) ++pasos, antes = c;
    });
    MOTOR_CHECK(pasos > 10);  // y aun asi se mueve
}

MOTOR_TEST_CASE(no_pisa_tu_casilla_ni_los_muros) {
    Engine::World w;
    const ObjectCatalog obj = objetos();
    const auto h = pon(w, "npc_race_enano", GridCoord{10, 10});  // radio 3
    RutinasPnj r;
    r.monta(w, obj, catalogo(), {}, 3);
    const std::set<std::pair<int, int>> muros{{11, 10}, {11, 11}, {9, 9}, {10, 12}};
    GridCoord jugador{10, 11};
    int pasos = 0;
    for (int vuelta = 0; vuelta < 12; ++vuelta) {
        jugador = GridCoord{8 + vuelta % 5, 8 + (vuelta * 3) % 5};
        if (igual(jugador, r.de(h)->casilla)) continue;
        const MundoPnj m = mundoDe(w, jugador, muros);
        GridCoord antes = r.de(h)->casilla;
        anda(r, w, m, 30.0f, [&] {
            const GridCoord c = r.de(h)->casilla;
            MOTOR_CHECK(!igual(c, jugador));
            MOTOR_CHECK(muros.count({c.x, c.y}) == 0);
            MOTOR_CHECK(igual(w.get(h)->object.position, c));  // el World dice lo mismo
            if (!igual(c, antes)) ++pasos, antes = c;
        });
    }
    MOTOR_CHECK(pasos > 20);
}

MOTOR_TEST_CASE(nunca_sale_de_su_radio_y_vuelve_a_su_sitio) {
    Engine::World w;
    const ObjectCatalog obj = objetos();
    const auto h = pon(w, "aldeano", GridCoord{0, 0});
    RutinasPnj r;
    r.monta(w, obj, catalogo(), {}, 11);
    const MundoPnj m = mundoDe(w, GridCoord{30, 30});
    bool salio = false, volvio = false;
    std::set<std::pair<int, int>> vistas;
    anda(r, w, m, 300.0f, [&] {
        const EstadoPnj& e = *r.de(h);
        MOTOR_CHECK(lejos(e.casilla, e.origen) <= 2);
        vistas.insert({e.casilla.x, e.casilla.y});
        if (!igual(e.casilla, e.origen)) salio = true;
        else if (salio) volvio = true;
    });
    MOTOR_CHECK(salio && volvio);
    MOTOR_CHECK(vistas.size() >= 6);  // pasea de verdad, no va y viene entre dos casillas
}

MOTOR_TEST_CASE(no_se_para_en_un_carril_pero_lo_cruza) {
    Engine::World w;
    const ObjectCatalog obj = objetos();
    const auto h = pon(w, "npc_race_enano", GridCoord{0, 0});  // radio 3
    std::vector<GridCoord> carriles;
    for (int y = -5; y <= 5; ++y) carriles.push_back(GridCoord{2, y});  // una calle en x = 2
    RutinasPnj r;
    r.monta(w, obj, catalogo(), carriles, 5);
    const MundoPnj m = mundoDe(w, GridCoord{30, 30});
    bool cruzo = false;
    anda(r, w, m, 400.0f, [&] {
        const EstadoPnj& e = *r.de(h);
        if (e.casilla.x > 2) cruzo = true;
        if (!e.andando() && e.ruta.empty()) MOTOR_CHECK(!r.carril(e.casilla));
    });
    MOTOR_CHECK(cruzo);
}

MOTOR_TEST_CASE(misma_semilla_mismo_paseo) {
    auto paseo = [](std::uint32_t semilla) {
        Engine::World w;
        const ObjectCatalog obj = objetos();
        const auto a = pon(w, "aldeano", GridCoord{0, 0});
        const auto b = pon(w, "npc_race_enano", GridCoord{4, 1});
        RutinasPnj r;
        r.monta(w, obj, catalogo(), {}, semilla);
        const MundoPnj m = mundoDe(w, GridCoord{30, 30});
        std::vector<int> traza;
        anda(r, w, m, 120.0f, [&] {
            traza.push_back(r.de(a)->casilla.x * 100 + r.de(a)->casilla.y);
            traza.push_back(r.de(b)->casilla.x * 100 + r.de(b)->casilla.y);
        });
        return traza;
    };
    MOTOR_CHECK(paseo(42) == paseo(42));
    MOTOR_CHECK(paseo(42) != paseo(43));
}

MOTOR_TEST_CASE(si_le_hablas_se_para_y_te_mira) {
    ObjectCatalog obj = objetos();
    SkillCatalog skills;
    Cuerpo cuerpo;
    SkillSet habilidades{10};
    std::vector<std::string> mochila;
    LevelDefinition nivel;
    nivel.name = "plaza";
    nivel.playerStart = GridCoord{0, 0};
    ObjectSpawn s;
    s.objectId = "aldeano";
    s.position = s.patrolMin = s.patrolMax = GridCoord{1, 0};
    nivel.objects.push_back(s);
    GameSession sesion(nullptr, &obj, &skills, nivel, &cuerpo, &habilidades, &mochila);
    // Sin montar las rutinas, como siempre: no se mueve nadie.
    for (int i = 0; i < 300; ++i) sesion.update(0.1f);
    MOTOR_CHECK(sesion.worldObjects()[0].position.x == 1 && sesion.worldObjects()[0].position.y == 0);

    sesion.rutinasPnj().monta(*sesion.world(), obj, catalogo(), {}, 9);
    const EstadoPnj& e = sesion.rutinasPnj().pnjs().front();
    MOTOR_CHECK(sesion.interact());  // esta al lado: se habla
    MOTOR_CHECK(sesion.mode() == GameMode::Dialogue);
    MOTOR_CHECK(e.rumbo == 3);  // tu estas a su izquierda
    for (int i = 0; i < 300; ++i) sesion.update(0.1f);  // 30 s de dialogo: ni un paso
    MOTOR_CHECK(igual(e.casilla, GridCoord{1, 0}) && !e.andando() && e.ruta.empty());
    sesion.closeInteraction();
    sesion.update(0.5f);
    MOTOR_CHECK(igual(e.casilla, GridCoord{1, 0}) && e.rumbo == 3);  // aun te mira un poco
    bool anduvo = false;
    for (int i = 0; i < 600 && !anduvo; ++i) {
        sesion.update(0.1f);
        anduvo = !igual(e.casilla, GridCoord{1, 0});
    }
    MOTOR_CHECK(anduvo);  // luego sigue con lo suyo
}

MOTOR_TEST_CASE(la_partida_guarda_su_sitio_y_se_puede_cargar) {
    ObjectCatalog obj = objetos();
    SkillCatalog skills;
    Cuerpo cuerpo;
    SkillSet habilidades{10};
    std::vector<std::string> mochila;
    LevelDefinition nivel;
    nivel.name = "plaza";
    nivel.playerStart = GridCoord{0, 0};
    ObjectSpawn s;
    s.objectId = "npc_race_enano";
    s.position = s.patrolMin = s.patrolMax = GridCoord{5, 5};
    nivel.objects.push_back(s);
    GameSession sesion(nullptr, &obj, &skills, nivel, &cuerpo, &habilidades, &mochila);
    sesion.rutinasPnj().monta(*sesion.world(), obj, catalogo(), {}, 1);
    for (int i = 0; i < 2000 && igual(sesion.worldObjects()[0].position, GridCoord{5, 5}); ++i) sesion.update(0.05f);
    MOTOR_CHECK(!igual(sesion.worldObjects()[0].position, GridCoord{5, 5}));  // se ha movido
    const SaveGameData guardada = sesion.captureSaveGame("assets/levels/plaza.json");
    MOTOR_CHECK(guardada.worldObjects.size() == 1);
    MOTOR_CHECK(igual(guardada.worldObjects[0].position, GridCoord{5, 5}));  // se guarda donde nacio
    GameSession otra(nullptr, &obj, &skills, nivel, &cuerpo, &habilidades, &mochila);
    std::string error;
    MOTOR_CHECK(otra.restoreSaveGame(guardada, &error));
}

MOTOR_TEST_CASE(una_cinematica_lo_mueve_y_manda_ella) {
    Engine::World w;
    const ObjectCatalog obj = objetos();
    const auto h = pon(w, "aldeano", GridCoord{0, 0});
    RutinasPnj r;
    r.monta(w, obj, catalogo(), {}, 2);
    const MundoPnj m = mundoDe(w, GridCoord{30, 30});
    r.ponPausa(true);
    w.get(h)->object.position = GridCoord{10, 10};  // la cinematica lo lleva al puente
    anda(r, w, m, 20.0f, [] {});
    MOTOR_CHECK(igual(w.get(h)->object.position, GridCoord{10, 10}));  // en pausa no se toca
    r.ponPausa(false);
    anda(r, w, m, 120.0f, [&] { MOTOR_CHECK(lejos(r.de(h)->casilla, GridCoord{10, 10}) <= 2); });
    MOTOR_CHECK(igual(r.de(h)->origen, GridCoord{10, 10}));  // su sitio es ahora ese
}

MOTOR_TEST_CASE(charlan_de_cara_y_el_guardia_vigila_en_su_sitio) {
    Engine::World w;
    const ObjectCatalog obj = objetos();
    const auto elfo = pon(w, "npc_race_elfo", GridCoord{0, 0});
    const auto otro = pon(w, "aldeano", GridCoord{2, 0});
    const auto guardia = pon(w, "guardia", GridCoord{8, 8});
    RutinasPnj r;
    r.monta(w, obj, catalogo(), {}, 4);
    const MundoPnj m = mundoDe(w, GridCoord{30, 30});
    bool deCara = false;
    std::set<int> rumbos;
    anda(r, w, m, 120.0f, [&] {
        const EstadoPnj& a = *r.de(elfo);
        const EstadoPnj& b = *r.de(otro);
        if (!a.andando() && !b.andando() && a.casilla.y == b.casilla.y && a.casilla.x < b.casilla.x &&
            a.rumbo == 1 && b.rumbo == 3)
            deCara = true;
        MOTOR_CHECK(igual(r.de(guardia)->casilla, GridCoord{8, 8}));
        rumbos.insert(r.de(guardia)->rumbo);
    });
    MOTOR_CHECK(deCara);
    MOTOR_CHECK(rumbos.size() == 4);  // de cara en cara
}

MOTOR_TEST_CASE(la_pose_anda_se_gira_y_hace_su_oficio) {
    EstadoPnj e;
    e.rumbo = 2;
    MOTOR_CHECK(RutinasPnj::animacion(e, false) == "idle_down");
    e.desde = GridCoord{0, 0}, e.casilla = GridCoord{1, 0}, e.progreso = 0.25f, e.rumbo = 1;
    MOTOR_CHECK(RutinasPnj::animacion(e, false) == "walk_right");
    MOTOR_CHECK(e.desfaseX() == -0.75f && e.desfaseY() == 0.0f);  // el dibujo aun no ha llegado
    MOTOR_CHECK(RutinasPnj::saltoPx(e, false) == 0.0f);
    e.progreso = 1.0f;
    e.def.movimiento = Game::MovimientoPnj::Vaiven;
    e.def.icono = "oro";
    e.actividad = 2.0f;
    MOTOR_CHECK(e.iconoVisible() == "oro");
    std::set<std::string> vistas;
    for (int i = 0; i < 24; ++i) {
        e.reloj = 0.1f * static_cast<float>(i);
        vistas.insert(RutinasPnj::animacion(e, false));
        MOTOR_CHECK(RutinasPnj::animacion(e, true) == "idle_right");  // reducidas: sin vaiven
    }
    MOTOR_CHECK(vistas == (std::set<std::string>{"idle_left", "idle_right"}));
    e.def.movimiento = Game::MovimientoPnj::AndarSitio;
    MOTOR_CHECK(RutinasPnj::animacion(e, false) == "walk_right");
    MOTOR_CHECK(RutinasPnj::animacion(e, true) == "idle_right");  // reducidas: sin andar_sitio
    e.def.movimiento = Game::MovimientoPnj::Golpe;
    std::set<float> saltos;
    for (int i = 0; i < 20; ++i) e.reloj = 0.05f * static_cast<float>(i), saltos.insert(RutinasPnj::saltoPx(e, false));
    MOTOR_CHECK(saltos == (std::set<float>{0.0f, 2.0f}));  // baja 2 px y sube
    e.def.movimiento = Game::MovimientoPnj::Giro;
    vistas.clear();
    for (int i = 0; i < 40; ++i) e.reloj = 0.1f * static_cast<float>(i), vistas.insert(RutinasPnj::animacion(e, false));
    MOTOR_CHECK(vistas.size() == 4);
    e.actividad = 0.0f;
    MOTOR_CHECK(e.iconoVisible().empty());
    saltos.clear();
    for (int i = 0; i < 30; ++i) e.reloj = 0.1f * static_cast<float>(i), saltos.insert(RutinasPnj::saltoPx(e, false));
    MOTOR_CHECK(saltos == (std::set<float>{-1.0f, 0.0f}));  // respira
}

MOTOR_TEST_CASE(frases_solo_si_estas_cerca_y_el_oficio_con_su_icono) {
    Engine::World w;
    const ObjectCatalog obj = objetos();
    const auto h = pon(w, "mercader", GridCoord{0, 0});
    const auto herrero = pon(w, "herrero", GridCoord{10, 0});
    RutinasPnj r;
    r.monta(w, obj, catalogo(), {}, 8);
    bool frase = false, icono = false, golpe = false;
    anda(r, w, mundoDe(w, GridCoord{9, 9}), 60.0f, [&] { MOTOR_CHECK(r.de(h)->frase.empty()); });  // lejos: nada
    anda(r, w, mundoDe(w, GridCoord{2, 2}), 60.0f, [&] {
        const EstadoPnj& e = *r.de(h);
        if (!e.frase.empty()) {
            frase = true;
            MOTOR_CHECK(e.frase == "¡Fruta fresca!" || e.frase == "¡Mirad, mirad!");
        }
        if (e.iconoVisible() == "oro") icono = true;
        MOTOR_CHECK(lejos(e.casilla, e.origen) <= 1);
        if (RutinasPnj::saltoPx(*r.de(herrero), false) == 2.0f) golpe = true;
        MOTOR_CHECK(r.de(herrero)->rumbo == 3 && igual(r.de(herrero)->casilla, GridCoord{10, 0}));
    });
    MOTOR_CHECK(frase && icono && golpe);
}

MOTOR_TEST_CASE(ir_y_volver_y_la_zona_que_reserva) {
    Engine::World w;
    const ObjectCatalog obj = objetos();
    const auto h = pon(w, "cartero", GridCoord{0, 0});
    RutinasPnj r;
    r.monta(w, obj, catalogo(), {}, 6);
    bool alla = false, aca = false;
    anda(r, w, mundoDe(w, GridCoord{30, 30}), 60.0f, [&] {
        const GridCoord c = r.de(h)->casilla;
        if (igual(c, GridCoord{4, 0})) alla = true;
        if (alla && igual(c, GridCoord{0, 0})) aca = true;
    });
    MOTOR_CHECK(alla && aca);
    // La zona de un PNJ (para las mascotas): su radio; de un objeto cualquiera, su casilla.
    ObjectDefinition aldeano;
    aldeano.id = "aldeano_sin_catalogo";
    aldeano.category = ObjectCategory::Npc;
    Engine::World vacio;
    MOTOR_CHECK(RutinasPnj::zona(&aldeano, GridCoord{5, 5}, vacio).size() == 25);  // porDefecto: radio 2
    MOTOR_CHECK(RutinasPnj::zona(nullptr, GridCoord{5, 5}, vacio).size() == 1);
    pon(vacio, "mascota_gato", GridCoord{6, 6});
    MOTOR_CHECK(RutinasPnj::zona(&aldeano, GridCoord{5, 5}, vacio).size() == 24);  // la casa del gato, libre
}

MOTOR_TEST_CASE(el_catalogo_del_juego_si_esta_se_lee) {
    // CTest corre en la raiz del repositorio: si pnj_datos ya ha dejado el suyo, tiene que leerse.
    std::ifstream f("assets/catalogs/rutinas_pnj.json");
    if (!f) return;  // aun no esta: el motor sigue con porDefecto
    const auto r = CatalogoRutinasPnj::carga("assets/catalogs/rutinas_pnj.json");
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().entradas() > 0);
}

MOTOR_TEST_CASE(en_la_sesion_no_se_pone_encima_de_ningun_objeto) {
    ObjectCatalog obj = objetos();
    ObjectDefinition barril;
    barril.id = "barril";
    barril.blocksMovement = true;
    obj.add(barril);
    ObjectDefinition alfombra;  // no bloquea, pero un PNJ no se pone encima de nada
    alfombra.id = "alfombra";
    obj.add(alfombra);
    SkillCatalog skills;
    Cuerpo cuerpo;
    SkillSet habilidades{10};
    std::vector<std::string> mochila;
    LevelDefinition nivel;
    nivel.name = "plaza";
    nivel.playerStart = GridCoord{9, 9};
    std::set<std::pair<int, int>> ocupadas;
    auto pon = [&](const char* id, GridCoord c) {
        ObjectSpawn s;
        s.objectId = id;
        s.position = s.patrolMin = s.patrolMax = c;
        nivel.objects.push_back(s);
        if (std::string(id) != "npc_race_enano") ocupadas.insert({c.x, c.y});
    };
    pon("npc_race_enano", GridCoord{0, 0});
    pon("barril", GridCoord{1, 0});
    pon("barril", GridCoord{0, 2});
    pon("alfombra", GridCoord{-1, -1});
    pon("alfombra", GridCoord{2, 2});
    GameSession sesion(nullptr, &obj, &skills, nivel, &cuerpo, &habilidades, &mochila);
    sesion.rutinasPnj().monta(*sesion.world(), obj, catalogo(), {}, 21);
    const EstadoPnj& e = sesion.rutinasPnj().pnjs().front();
    int pasos = 0;
    GridCoord antes = e.casilla;
    for (int i = 0; i < 6000; ++i) {
        sesion.update(1.0f / 30.0f);
        MOTOR_CHECK(ocupadas.count({e.casilla.x, e.casilla.y}) == 0);
        if (!igual(e.casilla, antes)) ++pasos, antes = e.casilla;
    }
    MOTOR_CHECK(pasos > 10);
}

// AV19-02 — Un acantilado (2 niveles o mas) no se cruza: el PNJ nunca pisa la meseta.
MOTOR_TEST_CASE(no_cruza_un_acantilado) {
    Engine::World w;
    const ObjectCatalog obj = objetos();
    const auto h = pon(w, "npc_race_enano", GridCoord{10, 10});  // radio 3
    RutinasPnj r;
    r.monta(w, obj, catalogo(), {}, 3);
    MundoPnj m = mundoDe(w, GridCoord{-50, -50});
    auto nivel = [](const GridCoord& c) { return c.x >= 11 ? 2 : 0; };  // a partir de x=11, dos niveles arriba
    m.pasa = [nivel](const GridCoord& de, const GridCoord& a) { return std::abs(nivel(de) - nivel(a)) <= 1; };
    anda(r, w, m, 120.0f, [&] { MOTOR_CHECK(r.de(h)->casilla.x <= 10); });
}

int main() { return motor_test::runAllTests(); }
