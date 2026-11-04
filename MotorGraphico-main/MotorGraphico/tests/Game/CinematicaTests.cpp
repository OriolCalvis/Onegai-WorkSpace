// PROLOGO-02 — test_cinematicas: el catálogo (lo que se lee y lo que se
// rechaza), cuándo empieza una cinemática, los espectros y las apariciones, y
// el reproductor contra un escenario de mentira (paseos con su desfase, el
// jugador con colisiones, beats que esperan al diálogo, niveles y el velo).
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "Game/Cinematica.h"
#include "Level/ObjectCatalog.h"
#include "TestHarness.h"

using namespace Game;

namespace {

struct Escenario : public EscenarioCinematica {
    std::map<std::string, GridCoord> sitios;
    std::map<std::string, std::string> animaciones;
    std::map<std::string, std::pair<float, float>> desfases;
    std::set<std::pair<int, int>> muros;
    std::set<std::string> flags;
    std::vector<std::string> historia;  // las flags que había cada vez que se avisó
    std::string nivel;
    bool dialogo = false, cargando = false;
    int pasosJugador = 0;
    // Lo que hace la historia al avisarla: la flag que abre un diálogo.
    std::string flagQueHabla;

    bool posicion(const std::string& actor, GridCoord& c) const override {
        auto it = sitios.find(actor);
        if (it == sitios.end()) return false;
        c = it->second;
        return true;
    }
    void pon(const std::string& actor, const GridCoord& c, float dx, float dy, const std::string& anim) override {
        sitios[actor] = c;
        desfases[actor] = {dx, dy};
        if (!anim.empty()) animaciones[actor] = anim;
    }
    float ritmo = 0.0f;
    void ritmoDelJugador(float segundos) override { ritmo = segundos; }
    bool pasoDelJugador(int dx, int dy) override {
        GridCoord c = sitios["jugador"];
        if (std::abs(dx) + std::abs(dy) != 1 || muros.count({c.x + dx, c.y + dy})) return false;
        sitios["jugador"] = GridCoord{c.x + dx, c.y + dy};
        ++pasosJugador;
        return true;
    }
    void enciende(const std::string& f) override { flags.insert(f); }
    void avanzaHistoria() override {
        historia.push_back(std::to_string(flags.size()));
        if (!flagQueHabla.empty() && flags.count(flagQueHabla)) {
            dialogo = true;
            flagQueHabla.clear();
        }
    }
    bool hablando() const override { return dialogo; }
    void cargaNivel(const std::string& ruta, const GridCoord& entrada) override {
        nivel = ruta;
        cargando = true;
        sitios.clear();
        sitios["jugador"] = entrada;
    }
    bool cargandoNivel() const override { return cargando; }
    // AV17-10
    std::map<std::pair<int, int>, bool> fuegos;
    std::map<std::string, std::string> posturas;
    std::vector<std::string> emociones;  // «actor:icono»
    void fuego(const GridCoord& c, bool si) override { fuegos[{c.x, c.y}] = si; }
    void postura(const std::string& a, const std::string& p) override { posturas[a] = p; }
    void emocion(const std::string& a, const std::string& icono, float) override { emociones.push_back(a + ":" + icono); }
};

CatalogoCinematicas catalogo(const std::string& texto) {
    auto r = CatalogoCinematicas::lee(texto);
    if (!r.isOk()) {
        MOTOR_CHECK_FAIL(r.errorMessage());
        return {};
    }
    return r.value();
}

void corre(ReproductorCinematica& r, Escenario& e, float segundos, float dt = 0.05f) {
    for (float t = 0.0f; t < segundos && r.activa(); t += dt) r.avanza(dt, e);
}

}  // namespace

MOTOR_TEST_CASE(celdas_del_paseo_primero_x_luego_y) {
    const auto c = celdasDelPaseo(GridCoord{1, 1}, {GridCoord{3, 2}, GridCoord{3, 0}});
    MOTOR_CHECK(c.size() == 5);
    MOTOR_CHECK(c[0].x == 2 && c[0].y == 1);
    MOTOR_CHECK(c[1].x == 3 && c[1].y == 1);
    MOTOR_CHECK(c[2].x == 3 && c[2].y == 2);
    MOTOR_CHECK(c[4].x == 3 && c[4].y == 0);
    MOTOR_CHECK(animacionDePaso(0, -1) == "walk_up");
    MOTOR_CHECK(animacionDePaso(1, 0) == "walk_right");
}

MOTOR_TEST_CASE(el_catalogo_rechaza_lo_que_no_entiende) {
    MOTOR_CHECK(!CatalogoCinematicas::lee(R"({"cinematicas": [{"pasos": []}]})").isOk());  // sin id
    MOTOR_CHECK(!CatalogoCinematicas::lee(R"({"cinematicas": [{"id": "a"}, {"id": "a"}]})").isOk());
    MOTOR_CHECK(!CatalogoCinematicas::lee(R"({"cinematicas": [{"id": "a", "pasos": [{"baila": "x"}]}]})").isOk());
    MOTOR_CHECK(!CatalogoCinematicas::lee(R"({"cinematicas": [{"id": "a", "pasos": [{"anda": "x"}]}]})").isOk());
    MOTOR_CHECK(!CatalogoCinematicas::lee(R"({"cinematicas": [{"id": "a", "pasos": [{"mira": "x", "hacia": "norte"}]}]})").isOk());
    MOTOR_CHECK(!CatalogoCinematicas::lee(R"({"cinematicas": [{"id": "a", "zona": [1, 2, 3]}]})").isOk());
    MOTOR_CHECK(!CatalogoCinematicas::lee(R"({"cinematicas": [{"id": "a", "pasos": [{"nivel": "x.json"}]}]})").isOk());
    // Sin fichero, vacío: el juego sigue sin cinemáticas.
    auto vacio = CatalogoCinematicas::carga("no/existe/cinematicas.json");
    MOTOR_CHECK(vacio.isOk() && vacio.value().lista.empty());
}

MOTOR_TEST_CASE(empieza_en_su_nivel_su_zona_y_con_sus_flags_una_vez) {
    const auto cat = catalogo(R"({"cinematicas": [
        {"id": "rio", "nivel": "assets/levels/prado.json", "zona": [5, 2, 6, 8], "requiere": "kit", "sin": ["fin"]},
        {"id": "otra", "requiere": ["subes"]}]})");
    std::set<std::string> f;
    auto flag = [&](const std::string& n) { return f.count(n) != 0; };
    MOTOR_CHECK(cat.laQueEmpieza("assets/levels/prado.json", GridCoord{5, 3}, flag) == nullptr);  // sin «kit»
    f.insert("kit");
    MOTOR_CHECK(cat.laQueEmpieza("assets/levels/prado.json", GridCoord{4, 3}, flag) == nullptr);  // fuera de la zona
    MOTOR_CHECK(cat.laQueEmpieza("assets/levels/otro.json", GridCoord{5, 3}, flag) == nullptr);   // otro nivel
    const Cinematica* c = cat.laQueEmpieza("assets/levels/prado.json", GridCoord{6, 8}, flag);
    MOTOR_CHECK(c != nullptr && c->id == "rio");
    f.insert("cine_rio_vista");  // ya se vio
    MOTOR_CHECK(cat.laQueEmpieza("assets/levels/prado.json", GridCoord{6, 8}, flag) == nullptr);
    f.insert("subes");
    c = cat.laQueEmpieza("cualquiera.json", GridCoord{0, 0}, flag);
    MOTOR_CHECK(c != nullptr && c->id == "otra");
}

MOTOR_TEST_CASE(punto_de_entrada_es_el_centro_de_la_zona_y_transitable_si_se_pide) {
    const auto cat = catalogo(R"({"cinematicas": [
        {"id": "a", "zona": [4, 2, 8, 6]}, {"id": "b", "zona": [0, 0, 1, 1]}, {"id": "sin"}]})");
    const Cinematica* a = cat.busca("a");
    MOTOR_CHECK(a != nullptr);
    if (a == nullptr) return;
    MOTOR_CHECK(a->puntoDeEntrada().x == 6 && a->puntoDeEntrada().y == 4);        // el centro
    MOTOR_CHECK(cat.busca("b")->puntoDeEntrada().x == 0);                          // division entera, como siempre
    MOTOR_CHECK(cat.busca("sin")->puntoDeEntrada().x == -1);                       // sin zona: {-1, -1}
    // El centro (6, 4) es un muro: la casilla transitable mas cercana de la zona.
    auto sinCentro = [](const GridCoord& c) { return !(c.x == 6 && c.y == 4); };
    const GridCoord p = a->puntoDeEntrada(sinCentro);
    MOTOR_CHECK(p.x == 5 && p.y == 3);  // anillo 1: primero la fila de arriba, de izquierda a derecha
    // Ninguna transitable: devuelve el centro (el validador lo avisa).
    MOTOR_CHECK(a->puntoDeEntrada([](const GridCoord&) { return false; }).x == 6);
}

MOTOR_TEST_CASE(aparicion_limitada_a_un_nivel) {
    const auto cat = catalogo(R"({"apariciones": [
        {"objeto": "mercader", "visibleSi": {"todas": ["fiesta"]}, "nivel": "assets/levels/mansion.json"},
        {"objeto": "guardia", "sin": "vacio"}]})");
    std::set<std::string> f;
    auto flag = [&](const std::string& n) { return f.count(n) != 0; };
    const std::string mansion = "assets/levels/mansion.json", ciudad = "assets/levels/ciudad.json";
    MOTOR_CHECK(!cat.visible("mercader", mansion, flag) && cat.afecta("mercader", mansion));
    MOTOR_CHECK(cat.visible("mercader", ciudad, flag) && !cat.afecta("mercader", ciudad));
    f.insert("fiesta");
    MOTOR_CHECK(cat.visible("mercader", mansion, flag));
    // sin «nivel», la regla vale en todos
    f.insert("vacio");
    MOTOR_CHECK(!cat.visible("guardia", mansion, flag) && !cat.visible("guardia", ciudad, flag));
    MOTOR_CHECK(cat.afecta("guardia", ciudad));
}

MOTOR_TEST_CASE(espectros_y_apariciones) {
    const auto cat = catalogo(R"({
        "espectro": {"flag": "recuerdo", "objetos": ["sho_noco"], "tinte": [0.5, 0.6, 1.0, 0.4]},
        "apariciones": [{"objeto": "sho_noco", "si": "barquero"}, {"objeto": "venides", "sin": "entra"}]})");
    std::set<std::string> f;
    auto flag = [&](const std::string& n) { return f.count(n) != 0; };
    MOTOR_CHECK(!cat.visible("sho_noco", "", flag));
    MOTOR_CHECK(cat.visible("venides", "", flag));
    MOTOR_CHECK(cat.visible("cualquiera", "", flag));
    f.insert("barquero");
    f.insert("entra");
    MOTOR_CHECK(cat.visible("sho_noco", "", flag));
    MOTOR_CHECK(!cat.visible("venides", "", flag));
    MOTOR_CHECK(!cat.esEspectro("sho_noco", flag) && !cat.jugadorEspectro(flag));
    f.insert("recuerdo");
    MOTOR_CHECK(cat.esEspectro("sho_noco", flag) && cat.jugadorEspectro(flag));
    MOTOR_CHECK(!cat.esEspectro("venides", flag));
    MOTOR_CHECK(std::abs(cat.espectro.tinte.w - 0.4f) < 1e-5f);
    MOTOR_CHECK(cat.afecta("sho_noco", "") && cat.afecta("venides", "") && !cat.afecta("edul", ""));
    const auto conFondo = catalogo(R"({"fondos": {"assets/levels/prado.json": [0.9, 0.92, 0.95]}})");
    MOTOR_CHECK(conFondo.fondos.size() == 1 && std::abs(conFondo.fondos[0].second.y - 0.92f) < 1e-5f);
}

MOTOR_TEST_CASE(un_pnj_anda_con_su_desfase_y_acaba_quieto) {
    const auto cat = catalogo(R"({"cinematicas": [{"id": "a", "alTerminar": ["hecho"],
        "pasos": [{"anda": "edul", "a": [3, 1], "velocidad": 2}]}]})");
    Escenario e;
    e.sitios["edul"] = GridCoord{1, 1};
    e.sitios["jugador"] = GridCoord{0, 0};
    ReproductorCinematica r;
    r.empieza(cat.lista[0]);
    r.avanza(0.05f, e);  // empieza el paseo
    r.avanza(0.25f, e);  // medio paso: en el mundo ya está en (2, 1), dibujado medio atrás
    MOTOR_CHECK(e.sitios["edul"].x == 2);
    MOTOR_CHECK(std::abs(e.desfases["edul"].first + 0.5f) < 1e-4f);
    MOTOR_CHECK(e.animaciones["edul"] == "walk_right");
    MOTOR_CHECK(r.activa());
    corre(r, e, 2.0f);
    MOTOR_CHECK(!r.activa());
    MOTOR_CHECK(e.sitios["edul"].x == 3 && e.sitios["edul"].y == 1);
    MOTOR_CHECK(std::abs(e.desfases["edul"].first) < 1e-6f);
    MOTOR_CHECK(e.animaciones["edul"] == "idle_right");
    MOTOR_CHECK(e.flags.count("cine_a_vista") && e.flags.count("hecho"));
    MOTOR_CHECK(!e.historia.empty());  // al acabar se avisa a la historia
}

MOTOR_TEST_CASE(el_jugador_anda_con_colisiones_y_sin_esperar_va_a_la_vez) {
    const auto cat = catalogo(R"({"cinematicas": [{"id": "a", "pasos": [
        {"anda": "sho_noco", "a": [1, 4], "sinEsperar": true, "velocidad": 4},
        {"anda": "jugador", "a": [4, 0], "velocidad": 4},
        {"espera": 1.5}, {"mira": "sho_noco", "hacia": "izquierda"}]}]})");
    Escenario e;
    e.sitios["jugador"] = GridCoord{0, 0};
    e.sitios["sho_noco"] = GridCoord{1, 0};
    e.muros.insert({3, 0});  // le corta el paso en la tercera celda
    ReproductorCinematica r;
    r.empieza(cat.lista[0]);
    r.avanza(0.05f, e);
    MOTOR_CHECK(e.pasosJugador == 1);  // el primer paso, ya
    corre(r, e, 5.0f);
    MOTOR_CHECK(!r.activa());
    MOTOR_CHECK(e.sitios["jugador"].x == 2);  // se quedó ante el muro
    MOTOR_CHECK(e.sitios["sho_noco"].y == 4);  // el otro llegó igual
    MOTOR_CHECK(e.animaciones["sho_noco"] == "idle_left");
}

MOTOR_TEST_CASE(el_jugador_anda_de_corrido_al_ritmo_de_la_escena) {
    // AV16-05: cada celda dura 1/velocidad y el siguiente paso arranca justo cuando
    // acaba el anterior (sin parar), igual que un PNJ.
    const auto cat = catalogo(R"({"cinematicas": [{"id": "a", "pasos": [
        {"anda": "jugador", "a": [3, 0], "velocidad": 2}]}]})");
    Escenario e;
    e.sitios["jugador"] = GridCoord{0, 0};
    ReproductorCinematica r;
    r.empieza(cat.lista[0]);
    r.avanza(0.0f, e);
    MOTOR_CHECK(e.pasosJugador == 1);
    MOTOR_CHECK(std::abs(e.ritmo - 0.5f) < 1e-6f);
    r.avanza(0.5f, e);
    MOTOR_CHECK(e.pasosJugador == 2);  // justo al acabar el primero arranca el segundo
    r.avanza(0.49f, e);
    MOTOR_CHECK(e.pasosJugador == 2);  // no se adelanta
    r.avanza(0.02f, e);
    MOTOR_CHECK(e.pasosJugador == 3);
    corre(r, e, 3.0f);
    MOTOR_CHECK(!r.activa() && e.sitios["jugador"].x == 3);
}

MOTOR_TEST_CASE(un_beat_espera_al_dialogo_y_un_nivel_a_que_cargue) {
    const auto cat = catalogo(R"({"cinematicas": [{"id": "a", "pasos": [
        {"beat": "habla"}, {"flag": "despues"}, {"nivel": "assets/levels/b.json", "en": [3, 9]},
        {"espera": 0.5}, {"flag": "en_b"}]}]})");
    Escenario e;
    e.sitios["jugador"] = GridCoord{0, 0};
    e.flagQueHabla = "habla";
    ReproductorCinematica r;
    r.empieza(cat.lista[0]);
    r.avanza(0.05f, e);
    MOTOR_CHECK(e.dialogo && e.flags.count("habla") && !e.flags.count("despues"));
    corre(r, e, 1.0f);
    MOTOR_CHECK(!e.flags.count("despues"));  // sigue hablando
    e.dialogo = false;
    r.avanza(0.05f, e);
    MOTOR_CHECK(e.flags.count("despues") && e.nivel == "assets/levels/b.json");
    corre(r, e, 1.0f);
    MOTOR_CHECK(r.activa() && !e.flags.count("en_b"));  // el nivel no ha cargado
    e.cargando = false;
    corre(r, e, 1.0f);
    MOTOR_CHECK(!r.activa() && e.flags.count("en_b"));
    MOTOR_CHECK(e.sitios["jugador"].x == 3 && e.sitios["jugador"].y == 9);
}

MOTOR_TEST_CASE(un_beat_que_no_abre_dialogo_no_atasca) {
    const auto cat = catalogo(R"({"cinematicas": [{"id": "a", "pasos": [{"beat": "nadie"}, {"flag": "sigue"}]}]})");
    Escenario e;
    e.sitios["jugador"] = GridCoord{0, 0};
    ReproductorCinematica r;
    r.empieza(cat.lista[0]);
    r.avanza(0.05f, e);
    MOTOR_CHECK(!r.activa() && e.flags.count("sigue"));
}

MOTOR_TEST_CASE(el_velo_funde_y_se_va_solo_al_acabar) {
    const auto cat = catalogo(R"({"cinematicas": [{"id": "a", "pasos": [
        {"funde": 1, "segundos": 1.0, "color": "negro"}, {"espera": 0.2}]}]})");
    Escenario e;
    e.sitios["jugador"] = GridCoord{0, 0};
    ReproductorCinematica r;
    r.empieza(cat.lista[0]);
    r.avanza(0.05f, e);
    r.avanza(0.5f, e);
    MOTOR_CHECK(std::abs(r.velo() - 0.5f) < 1e-4f);
    MOTOR_CHECK(!r.veloBlanco());
    corre(r, e, 2.0f);
    MOTOR_CHECK(!r.activa());
    MOTOR_CHECK(r.velo() > 0.9f);
    for (int k = 0; k < 40; ++k) r.avanza(0.05f, e);
    MOTOR_CHECK(r.velo() == 0.0f);
}

MOTOR_TEST_CASE(quien_no_esta_en_el_nivel_no_atasca) {
    const auto cat = catalogo(R"({"cinematicas": [{"id": "a", "pasos": [
        {"anda": "fantasma", "a": [3, 3]}, {"mira": "fantasma", "hacia": "abajo"}, {"flag": "fin"}]}]})");
    Escenario e;
    e.sitios["jugador"] = GridCoord{0, 0};
    ReproductorCinematica r;
    r.empieza(cat.lista[0]);
    r.avanza(0.05f, e);
    MOTOR_CHECK(!r.activa() && e.flags.count("fin"));
}

MOTOR_TEST_CASE(aigren_se_va_y_en_su_lugar_esta_edul) {
    const auto cat = catalogo(R"({"apariciones": [
        {"objeto": "edul", "visibleSi": {"todas": ["dia2"], "alguna": ["despedido", "invitado"], "ninguna": ["redimido"]}},
        {"objeto": "aigren", "ocultoSi": {"todas": ["dia2"], "alguna": ["despedido", "invitado"], "ninguna": ["redimido"]}}]})");
    std::set<std::string> f;
    auto flag = [&](const std::string& n) { return f.count(n) != 0; };
    MOTOR_CHECK(cat.visible("aigren", "", flag) && !cat.visible("edul", "", flag));  // el primer dia
    f.insert("dia2");
    MOTOR_CHECK(cat.visible("aigren", "", flag) && !cat.visible("edul", "", flag));  // aun no se ha despedido
    f.insert("despedido");
    MOTOR_CHECK(!cat.visible("aigren", "", flag) && cat.visible("edul", "", flag));
    f.insert("redimido");
    MOTOR_CHECK(cat.visible("aigren", "", flag) && !cat.visible("edul", "", flag));
}

MOTOR_TEST_CASE(el_fondo_del_titulo_pasea_en_bucle) {
    const auto cat = catalogo(R"({"titulo": {"nivel": "assets/levels/ciudad.json", "conNivelInicial": "assets/levels/prado.json",
                                            "camino": [[0, 0], [4, 0], [4, 2]],
                                            "velocidad": 0.5, "zoom": 1.2}})");
    MOTOR_CHECK(cat.titulo.nivel == "assets/levels/ciudad.json" && cat.titulo.camino.size() == 3);
    MOTOR_CHECK(cat.titulo.conNivelInicial == "assets/levels/prado.json");
    MOTOR_CHECK(std::abs(cat.titulo.zoom - 1.2f) < 1e-5f);
    float x = 0, y = 0;
    cat.titulo.punto(2.0f, x, y);  // a medio primer tramo
    MOTOR_CHECK(std::abs(x - 2.0f) < 1e-4f && std::abs(y) < 1e-4f);
    cat.titulo.punto(5.0f, x, y);  // a medio segundo tramo
    MOTOR_CHECK(std::abs(x - 4.0f) < 1e-4f && std::abs(y - 1.0f) < 1e-4f);
    cat.titulo.punto(12.0f + 2.0f, x, y);  // el bucle mide 4 + 2 + 6 = 12: vuelve a empezar
    MOTOR_CHECK(std::abs(x - 2.0f) < 1e-4f && std::abs(y) < 1e-4f);
    MOTOR_CHECK(!CatalogoCinematicas::lee(R"({"titulo": {"camino": [[1]]}})").isOk());
}


// AV17-10 — Las ordenes nuevas: trepar y bajar, arder, tumbado, patrulla y emocion.
MOTOR_TEST_CASE(trepar_sube_el_dibujo_y_bajar_lo_devuelve) {
    const auto cat = catalogo(R"({"cinematicas": [{"id": "a", "pasos": [
        {"trepa": "heroe", "segundos": 1.0, "altura": 2.0}, {"flag": "arriba"},
        {"baja": "heroe", "segundos": 1.0}, {"flag": "abajo"}]}]})");
    Escenario e;
    e.sitios["heroe"] = GridCoord{4, 4};
    ReproductorCinematica r;
    r.empieza(cat.lista[0]);
    r.avanza(0.05f, e);
    r.avanza(0.5f, e);  // a medio camino: la mitad de la altura (dx = dy = -altura)
    MOTOR_CHECK(std::abs(e.desfases["heroe"].first + 1.0f) < 0.1f && e.animaciones["heroe"] == "climb");
    r.avanza(0.6f, e);
    MOTOR_CHECK(e.flags.count("arriba") && std::abs(e.desfases["heroe"].second + 2.0f) < 0.01f);
    MOTOR_CHECK(e.sitios["heroe"].x == 4 && e.sitios["heroe"].y == 4);  // no cambia de celda
    r.avanza(0.5f, e);
    MOTOR_CHECK(e.animaciones["heroe"] == "climb_down" && e.desfases["heroe"].first < -0.5f);
    corre(r, e, 2.0f);
    MOTOR_CHECK(!r.activa() && e.flags.count("abajo"));
}

MOTOR_TEST_CASE(arder_y_apagar_piden_el_fuego_de_la_casilla) {
    const auto cat = catalogo(R"({"cinematicas": [{"id": "a", "pasos": [
        {"arde": [3, 2]}, {"arde": [4, 2]}, {"apaga": [3, 2]}]}]})");
    Escenario e;
    e.sitios["jugador"] = GridCoord{0, 0};
    ReproductorCinematica r;
    r.empieza(cat.lista[0]);
    corre(r, e, 1.0f);
    MOTOR_CHECK(!e.fuegos.at({3, 2}) && e.fuegos.at({4, 2}));
}

MOTOR_TEST_CASE(tumbado_herido_muerto_y_de_pie) {
    const auto cat = catalogo(R"({"cinematicas": [{"id": "a", "pasos": [
        {"tumba": "edul", "como": "muerto"}, {"tumba": "xila"}, {"levanta": "xila"}]}]})");
    Escenario e;
    e.sitios["jugador"] = GridCoord{0, 0};
    ReproductorCinematica r;
    r.empieza(cat.lista[0]);
    corre(r, e, 1.0f);
    MOTOR_CHECK(e.posturas["edul"] == "muerto" && e.posturas["xila"] == "de_pie");
    MOTOR_CHECK(!CatalogoCinematicas::lee(R"({"cinematicas": [{"id": "a", "pasos": [{"tumba": "x", "como": "dormido"}]}]})").isOk());
}

MOTOR_TEST_CASE(la_patrulla_va_y_vuelve_donde_empezo) {
    const auto cat = catalogo(R"({"cinematicas": [{"id": "a", "pasos": [
        {"patrulla": "perdido", "por": [[3, 0], [3, 2]], "veces": 2, "velocidad": 20}, {"flag": "fin"}]}]})");
    Escenario e;
    e.sitios["jugador"] = GridCoord{9, 9};
    e.sitios["perdido"] = GridCoord{0, 0};
    ReproductorCinematica r;
    r.empieza(cat.lista[0]);
    int maxX = 0;
    for (int k = 0; k < 400 && r.activa(); ++k) {
        r.avanza(0.05f, e);
        maxX = std::max(maxX, e.sitios["perdido"].x);
    }
    MOTOR_CHECK(!r.activa() && e.flags.count("fin"));
    MOTOR_CHECK(maxX == 3 && e.sitios["perdido"].x == 0 && e.sitios["perdido"].y == 0);
}

MOTOR_TEST_CASE(la_emocion_enseña_el_icono_y_espera_si_se_pide) {
    const auto cat = catalogo(R"({"cinematicas": [{"id": "a", "pasos": [
        {"emocion": "xila", "icono": "asustado", "segundos": 1.0}, {"flag": "ya"},
        {"emocion": "edul", "icono": "enfurecido", "segundos": 1.0, "aguarda": true}, {"flag": "luego"}]}]})");
    Escenario e;
    e.sitios["jugador"] = GridCoord{0, 0};
    ReproductorCinematica r;
    r.empieza(cat.lista[0]);
    r.avanza(0.05f, e);
    MOTOR_CHECK(e.flags.count("ya") && !e.flags.count("luego") && e.emociones.size() == 2);  // el primero no espera
    r.avanza(0.5f, e);
    MOTOR_CHECK(!e.flags.count("luego"));
    corre(r, e, 1.5f);
    MOTOR_CHECK(e.flags.count("luego") && e.emociones[0] == "xila:asustado" && e.emociones[1] == "edul:enfurecido");
    MOTOR_CHECK(!CatalogoCinematicas::lee(R"({"cinematicas": [{"id": "a", "pasos": [{"emocion": "x"}]}]})").isOk());
}

// ------------------------------------------------------------------------------------------------
// E-DISPARADORES-1 E2 (D0026) — el disparador: radio por ambito, linea de vision, zona o radio, lleva y accion.

namespace {

// Un nivel de mentira: tiles con colision y objetos del catalogo (con `tapaVista` o sin ella) en sus casillas.
struct NivelDeMentira {
    ObjectCatalog catalogo;
    std::map<std::pair<int, int>, std::string> objetos;
    std::set<std::pair<int, int>> tilesConColision;
    NivelDeMentira() {
        const auto r = catalogo.loadFromString(R"({"objects": [
            {"id": "muro", "category": "prop", "blocksMovement": true, "tapaVista": true},
            {"id": "carro", "category": "prop", "blocksMovement": true, "tapaVista": true},
            {"id": "farola", "category": "prop", "blocksMovement": true},
            {"id": "taburete", "category": "prop", "blocksMovement": true},
            {"id": "guardia", "category": "npc", "blocksMovement": true}]})");
        MOTOR_CHECK(r.isOk());
    }
    VistaDelNivel vista(AmbitoNivel ambito) const {
        VistaDelNivel v;
        v.ambito = ambito;
        v.tapa = [this](int x, int y) {
            if (tilesConColision.count({x, y})) return true;
            const auto it = objetos.find({x, y});
            const ObjectDefinition* d = it == objetos.end() ? nullptr : catalogo.find(it->second);
            return d != nullptr && d->tapaVista;
        };
        return v;
    }
};

// Una cinematica con disparador, sin zona: el foco es `lleva.a` (10, 10).
CatalogoCinematicas conFoco(const std::string& extra = "") {
    return catalogo(R"({"cinematicas": [{"id": "x", "nivel": "assets/levels/n.json",
        "disparador": {"lleva": {"a": [10, 10], "mira": "arriba"})" + extra + R"(}, "pasos": [{"flag": "hecho"}]}]})");
}

const std::string kNivel = "assets/levels/n.json";

}  // namespace

MOTOR_TEST_CASE(el_disparador_se_lee_y_rechaza_lo_raro) {
    const auto cat = catalogo(R"({"cinematicas": [{"id": "a", "zona": [4, 2, 8, 6],
        "disparador": {"lleva": {"a": [5, 3], "mira": "abajo", "como": "fundido"}, "accion": {"beat": "mi_beat"},
                       "once": false, "radio": 7, "vision": false}}]})");
    const Cinematica* a = cat.busca("a");
    MOTOR_CHECK(a != nullptr && a->disparador.hay);
    if (a == nullptr) return;
    const Disparador& d = a->disparador;
    MOTOR_CHECK(d.hayLleva && d.lleva.x == 5 && d.lleva.y == 3 && d.mira == "abajo" && !d.andando);
    MOTOR_CHECK(d.accion == Disparador::Accion::Beat && d.accionId == "mi_beat" && !d.once && d.radio == 7 && d.vision == 0);
    MOTOR_CHECK(a->puntoDeEntrada().x == 5 && a->puntoDeEntrada().y == 3);  // lleva.a manda sobre el centro de la zona
    const auto minimo = catalogo(R"({"cinematicas": [{"id": "b", "disparador": {}}]})");
    const Disparador& m = minimo.busca("b")->disparador;
    MOTOR_CHECK(m.hay && !m.hayLleva && m.andando && m.once && m.radio == -1 && m.vision == -1 &&
                m.accion == Disparador::Accion::Cinematica && m.accionId.empty());
    auto malo = [](const std::string& disparador) {
        return CatalogoCinematicas::lee(R"({"cinematicas": [{"id": "a", "disparador": )" + disparador + "}]}").isOk();
    };
    MOTOR_CHECK(!malo(R"({"accion": {"beat": "a", "marca": "b"}})"));        // una sola accion
    MOTOR_CHECK(!malo(R"({"accion": {}})"));
    MOTOR_CHECK(!malo(R"({"accion": {"combate": "enc"}})"));                  // combate es E4
    MOTOR_CHECK(!malo(R"({"accion": {"baila": "x"}})"));
    MOTOR_CHECK(!malo(R"({"lleva": {"mira": "arriba"}})"));                   // sin «a»
    MOTOR_CHECK(!malo(R"({"lleva": {"a": [1, 1], "como": "volando"}})"));
    MOTOR_CHECK(!malo(R"({"lleva": {"a": [1, 1], "mira": "norte"}})"));
    MOTOR_CHECK(!malo(R"({"radio": -2})"));
    MOTOR_CHECK(!malo(R"({"radii": 3})"));                                    // una errata no se traga
    MOTOR_CHECK(!malo(R"("zona")"));
    MOTOR_CHECK(malo(R"({"radio": "nivel", "vision": true, "once": true})"));
}

MOTOR_TEST_CASE(el_radio_depende_del_ambito_y_es_chebyshev) {
    const auto cat = conFoco();
    NivelDeMentira n;
    std::set<std::string> f;
    auto flag = [&](const std::string& s) { return f.count(s) != 0; };
    auto dispara = [&](AmbitoNivel a, GridCoord jugador) {
        const VistaDelNivel v = n.vista(a);
        return cat.laQueEmpieza(kNivel, jugador, flag, &v) != nullptr;
    };
    // Exterior: 10. En el limite (diagonal incluida: Chebyshev) si; uno mas, no.
    MOTOR_CHECK(dispara(AmbitoNivel::Exterior, {20, 10}));
    MOTOR_CHECK(dispara(AmbitoNivel::Exterior, {20, 20}));
    MOTOR_CHECK(!dispara(AmbitoNivel::Exterior, {21, 10}));
    MOTOR_CHECK(!dispara(AmbitoNivel::Exterior, {0, 21}));
    // Interior grande: 5.
    MOTOR_CHECK(dispara(AmbitoNivel::InteriorGrande, {15, 15}));
    MOTOR_CHECK(!dispara(AmbitoNivel::InteriorGrande, {16, 10}));
    // Interior pequeno: al entrar, estes donde estes del nivel.
    MOTOR_CHECK(dispara(AmbitoNivel::InteriorPequeno, {0, 0}));
    MOTOR_CHECK(dispara(AmbitoNivel::InteriorPequeno, {10, 10}));
    // «radio» del disparador manda sobre el ambito; «nivel» vuelve al del ambito.
    const auto r3 = conFoco(R"(, "radio": 3)");
    const VistaDelNivel ext = n.vista(AmbitoNivel::Exterior);
    MOTOR_CHECK(r3.laQueEmpieza(kNivel, {13, 10}, flag, &ext) != nullptr);
    MOTOR_CHECK(r3.laQueEmpieza(kNivel, {14, 10}, flag, &ext) == nullptr);
    // Otro nivel o la flag de vista: no.
    MOTOR_CHECK(cat.laQueEmpieza("assets/levels/otro.json", {10, 10}, flag, &ext) == nullptr);
    f.insert("cine_x_vista");
    MOTOR_CHECK(cat.laQueEmpieza(kNivel, {10, 10}, flag, &ext) == nullptr);
}

MOTOR_TEST_CASE(la_vista_la_tapa_un_objeto_tapaVista_y_no_un_pnj_ni_un_objeto_pequeno) {
    const auto cat = conFoco();
    NivelDeMentira n;
    std::set<std::string> f;
    auto flag = [&](const std::string& s) { return f.count(s) != 0; };
    const GridCoord jugador{14, 10};  // a 4 del foco (10, 10): en el radio de un interior grande (5)
    auto dispara = [&]() {
        const VistaDelNivel v = n.vista(AmbitoNivel::InteriorGrande);
        return cat.laQueEmpieza(kNivel, jugador, flag, &v) != nullptr;
    };
    MOTOR_CHECK(dispara());                       // despejado
    n.objetos[{12, 10}] = "guardia";              // un PNJ en medio (bloquea el paso, no la vista)
    MOTOR_CHECK(dispara());
    n.objetos[{12, 10}] = "farola";               // objetos pequenos
    MOTOR_CHECK(dispara());
    n.objetos[{12, 10}] = "taburete";
    MOTOR_CHECK(dispara());
    n.objetos[{12, 10}] = "muro";                 // una pared (tapaVista): tapa
    MOTOR_CHECK(!dispara());
    n.objetos[{12, 10}] = "carro";
    MOTOR_CHECK(!dispara());
    n.objetos.clear();
    n.tilesConColision.insert({11, 10});          // un tile con colision (un edificio): tapa
    MOTOR_CHECK(!dispara());
    n.tilesConColision.clear();
    // Interior pequeno: sin vision (al entrar), aunque haya una pared en medio.
    n.objetos[{12, 10}] = "muro";
    const VistaDelNivel pequeno = n.vista(AmbitoNivel::InteriorPequeno);
    MOTOR_CHECK(cat.laQueEmpieza(kNivel, jugador, flag, &pequeno) != nullptr);
    // «vision»: true la fuerza en el pequeno; false la quita en el grande.
    const auto conVision = conFoco(R"(, "vision": true, "radio": 5)");
    MOTOR_CHECK(conVision.laQueEmpieza(kNivel, jugador, flag, &pequeno) == nullptr);
    const auto sinVision = conFoco(R"(, "vision": false)");
    const VistaDelNivel grande = n.vista(AmbitoNivel::InteriorGrande);
    MOTOR_CHECK(sinVision.laQueEmpieza(kNivel, jugador, flag, &grande) != nullptr);
}

MOTOR_TEST_CASE(la_zona_sigue_valiendo_sin_vision_y_el_radio_es_otra_via) {
    // Zona [2, 1, 8, 5], foco = centro (5, 3). Una pared de tiles en la columna 6 (filas 2 a 4) y otra en (5, 6).
    const auto cat = catalogo(R"({"cinematicas": [{"id": "z", "nivel": "assets/levels/n.json", "zona": [2, 1, 8, 5],
        "disparador": {"accion": {"marca": "paso"}}}]})");
    NivelDeMentira n;
    n.tilesConColision = {{6, 2}, {6, 3}, {6, 4}, {5, 6}};
    std::set<std::string> f;
    auto flag = [&](const std::string& s) { return f.count(s) != 0; };
    const VistaDelNivel v = n.vista(AmbitoNivel::Exterior);
    MOTOR_CHECK(cat.laQueEmpieza(kNivel, {8, 3}, flag, &v) != nullptr);   // en la zona: dispara aunque no vea el foco
    MOTOR_CHECK(cat.laQueEmpieza(kNivel, {8, 2}, flag, &v) != nullptr);
    MOTOR_CHECK(cat.laQueEmpieza(kNivel, {9, 3}, flag, &v) == nullptr);   // fuera de la zona, por radio: la pared la tapa
    MOTOR_CHECK(cat.laQueEmpieza(kNivel, {5, 9}, flag, &v) == nullptr);   // fuera, a 6: tapado por (5, 6)
    MOTOR_CHECK(cat.laQueEmpieza(kNivel, {0, 3}, flag, &v) != nullptr);   // fuera de la zona, por radio y a la vista
    MOTOR_CHECK(cat.laQueEmpieza(kNivel, {5, 20}, flag, &v) == nullptr);  // fuera del radio
    // Sin vista (arnes, explorador): la zona o EN el foco; lo demas, no.
    MOTOR_CHECK(cat.laQueEmpieza(kNivel, {8, 3}, flag) != nullptr);
    MOTOR_CHECK(cat.laQueEmpieza(kNivel, cat.busca("z")->puntoDeEntrada(), flag) != nullptr);
    MOTOR_CHECK(cat.laQueEmpieza(kNivel, {0, 3}, flag) == nullptr);
    // Las cinematicas SIN disparador no cambian con ni sin vista: su zona, o donde sea si no la tienen.
    const auto vieja = catalogo(R"({"cinematicas": [{"id": "v", "zona": [4, 2, 6, 4]}, {"id": "w", "requiere": ["k"]}]})");
    f.insert("k");
    MOTOR_CHECK(vieja.laQueEmpieza(kNivel, {5, 3}, flag, &v) == vieja.busca("v"));
    MOTOR_CHECK(vieja.laQueEmpieza(kNivel, {2, 3}, flag, &v) == vieja.busca("w"));
    MOTOR_CHECK(vieja.laQueEmpieza(kNivel, {2, 3}, flag) == vieja.busca("w"));
}

MOTOR_TEST_CASE(lleva_andando_al_jugador_y_solo_luego_ejecuta_la_accion) {
    const auto cat = catalogo(R"({"cinematicas": [{"id": "x", "nivel": "assets/levels/n.json", "zona": [0, 0, 9, 0],
        "disparador": {"lleva": {"a": [3, 0], "mira": "arriba"}, "accion": {"marca": "llegado"}}}]})");
    const Cinematica* c = cat.busca("x");
    Escenario e;
    e.sitios["jugador"] = GridCoord{0, 0};
    LlevaDelJugador lleva;
    lleva.jugador = GridCoord{0, 0};
    lleva.ruta = [](const GridCoord& d) {  // la ruta que daria rutaAndando: casilla a casilla
        std::vector<GridCoord> r;
        for (int x = 1; x <= d.x; ++x) r.push_back(GridCoord{x, 0});
        return r;
    };
    ReproductorCinematica r;
    r.empieza(cat.efectiva(*c, &lleva));
    // Mientras anda, la marca no esta; al terminar, el jugador esta en la casilla y la marca encendida.
    bool marcaConElJugadorFuera = false;
    for (float t = 0.0f; t < 10.0f && r.activa(); t += 0.05f) {
        r.avanza(0.05f, e);
        if (e.flags.count("llegado") && e.sitios["jugador"].x != 3) marcaConElJugadorFuera = true;
    }
    MOTOR_CHECK(!marcaConElJugadorFuera);
    MOTOR_CHECK(!r.activa() && e.flags.count("llegado") == 1 && e.flags.count("cine_x_vista") == 1);
    MOTOR_CHECK(e.sitios["jugador"].x == 3 && e.sitios["jugador"].y == 0);
    MOTOR_CHECK(e.pasosJugador == 3);                      // andando, no a saltos
    MOTOR_CHECK(e.animaciones["jugador"] == "idle_up");    // mira hacia arriba al llegar
    // Ya en la casilla: ni anda ni funde; solo la accion.
    Escenario e2;
    e2.sitios["jugador"] = GridCoord{3, 0};
    LlevaDelJugador alli = lleva;
    alli.jugador = GridCoord{3, 0};
    ReproductorCinematica r2;
    r2.empieza(cat.efectiva(*c, &alli));
    corre(r2, e2, 2.0f);
    MOTOR_CHECK(e2.pasosJugador == 0 && e2.flags.count("llegado") == 1 && e2.animaciones["jugador"] == "idle_up");
}

MOTOR_TEST_CASE(lleva_con_fundido_si_no_hay_camino_o_se_pide) {
    const std::string base = R"({"cinematicas": [{"id": "x", "disparador": {"lleva": {"a": [7, 7], "como": ")";
    for (const char* como : {"fundido", "andando"}) {  // «andando» sin camino (ruta vacia) cae al fundido (D0026.2)
        const auto cat = catalogo(base + como + R"("}, "accion": {"marca": "llegado"}}}]})");
        Escenario e;
        e.sitios["jugador"] = GridCoord{0, 0};
        LlevaDelJugador lleva;
        lleva.jugador = GridCoord{0, 0};
        lleva.ruta = [](const GridCoord&) { return std::vector<GridCoord>{}; };  // muro por medio: no hay camino
        ReproductorCinematica r;
        r.empieza(cat.efectiva(*cat.busca("x"), &lleva));
        float velo = 0.0f;
        bool marcaAntesDeColocar = false;
        for (float t = 0.0f; t < 5.0f && r.activa(); t += 0.05f) {
            r.avanza(0.05f, e);
            velo = std::max(velo, r.velo());
            if (e.flags.count("llegado") && e.sitios["jugador"].x != 7) marcaAntesDeColocar = true;
        }
        MOTOR_CHECK(velo > 0.9f);  // la pantalla se fundio a negro
        MOTOR_CHECK(!r.veloBlanco());
        MOTOR_CHECK(e.pasosJugador == 0 && e.sitios["jugador"].x == 7 && e.sitios["jugador"].y == 7);
        MOTOR_CHECK(!marcaAntesDeColocar && e.flags.count("llegado") == 1);
    }
}

MOTOR_TEST_CASE(la_accion_es_una_cinematica_otra_un_beat_o_una_marca) {
    const auto cat = catalogo(R"({"cinematicas": [
        {"id": "propia", "disparador": {}, "pasos": [{"flag": "de_la_propia"}]},
        {"id": "otra", "pasos": [{"flag": "de_otra"}]},
        {"id": "remite", "disparador": {"accion": {"cinematica": "otra"}}, "pasos": [{"flag": "NO"}]},
        {"id": "beat", "disparador": {"accion": {"beat": "bnd_x"}}, "pasos": [{"flag": "NO"}]},
        {"id": "marca", "disparador": {"accion": {"marca": "paso_por_aqui"}}}]})");
    auto juega = [&](const char* id) {
        Escenario e;
        e.sitios["jugador"] = GridCoord{0, 0};
        ReproductorCinematica r;
        r.empieza(cat.efectiva(*cat.busca(id)));
        corre(r, e, 2.0f);
        MOTOR_CHECK(!r.activa() && e.flags.count(std::string("cine_") + id + "_vista") == 1);
        return e.flags;
    };
    MOTOR_CHECK(juega("propia").count("de_la_propia") == 1);
    const auto remite = juega("remite");
    MOTOR_CHECK(remite.count("de_otra") == 1 && remite.count("NO") == 0);
    const auto beat = juega("beat");
    MOTOR_CHECK(beat.count("bnd_x") == 1 && beat.count("NO") == 0);
    const auto marca = juega("marca");
    MOTOR_CHECK(marca.count("paso_por_aqui") == 1);
    // Sin disparador, los pasos de siempre (efectiva no toca nada).
    MOTOR_CHECK(cat.pasosEfectivos(*cat.busca("otra")).size() == 1);
}

int main() { return motor_test::runAllTests(); }
