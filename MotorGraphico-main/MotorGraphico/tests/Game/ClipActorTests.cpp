// U3-05.E07a -- Game::ClipActor (adaptador clip -> puerto de sprite). Sin GL ni assets.
#include "TestHarness.h"

#include "Game/ClipActor.h"

#include <string>
#include <vector>

using Game::ClipActor;
using Game::ClipAnimacion;
using Game::IdClip;
using Game::PoliticaCambio;

namespace {

IdClip idDe(const char* s) { return IdClip::parse(s).value(); }

struct PuertosFalsos : Game::PuertosEventoClip {
    std::vector<std::string> logicos, visuales, sonidos;
    void aplicaLogico(const Game::EventoFotograma& e) override { logicos.push_back(e.id); }
    void muestraVisual(const Game::EventoFotograma& e) override { visuales.push_back(e.id); }
    void suena(const Game::EventoFotograma& e) override { sonidos.push_back(e.id); }
};

struct DestinoFalso : Game::DestinoFotograma {
    std::vector<int> recibidos;
    void ponerFotograma(int s) override { recibidos.push_back(s); }
};

// Fotogramas con sprites 5,6,7 de 100 ms; eventos e0 (logico, f0), e1 (visual, f1), e2 (sonido, f2).
ClipAnimacion clipTres(bool bucle) {
    ClipAnimacion c;
    c.bucle = bucle;
    const char* tipos[3] = {"logico", "visual", "sonido"};
    for (int i = 0; i < 3; ++i) {
        c.fotogramas.push_back({5 + i, 100});
        c.eventos.push_back({"e" + std::to_string(i), i, tipos[i], "", i});
    }
    return c;
}

}  // namespace

MOTOR_TEST_CASE(entrega_el_sprite_solo_cuando_cambia) {
    ClipActor a(idDe("idle"), clipTres(true));
    PuertosFalsos p;
    DestinoFalso d;
    a.avanza(0.0, p, d);  // primer avance: entrega el fotograma 0
    for (int i = 0; i < 5; ++i) a.avanza(0.01, p, d);  // 50 ms: mismo fotograma
    MOTOR_CHECK(d.recibidos == std::vector<int>({5}));
    a.avanza(0.06, p, d);  // 110 ms: fotograma 1
    a.avanza(0.01, p, d);
    MOTOR_CHECK(d.recibidos == std::vector<int>({5, 6}));
    a.avanza(0.1, p, d);  // fotograma 2
    a.avanza(0.1, p, d);  // vuelta al 0
    MOTOR_CHECK(d.recibidos == std::vector<int>({5, 6, 7, 5}));
}

MOTOR_TEST_CASE(eventos_una_vez_por_paso_real_y_a_su_puerto) {
    ClipActor a(idDe("idle"), clipTres(true));
    PuertosFalsos p;
    DestinoFalso d;
    a.avanza(0.0, p, d);
    a.avanza(0.05, p, d);
    a.avanza(0.05, p, d);  // cruza al fotograma 1
    a.avanza(0.01, p, d);
    MOTOR_CHECK(p.logicos == std::vector<std::string>({"e0"}));
    MOTOR_CHECK(p.visuales == std::vector<std::string>({"e1"}));
    MOTOR_CHECK(p.sonidos.empty());
    a.avanza(0.1, p, d);  // fotograma 2
    MOTOR_CHECK(p.sonidos == std::vector<std::string>({"e2"}));
}

MOTOR_TEST_CASE(drena_el_pendiente_y_no_pierde_eventos) {
    // 200 fotogramas de 1 ms con un evento cada uno: un solo dt grande excede el presupuesto de
    // eventos de avanzaClip (64) varias veces; el actor debe entregarlos todos.
    ClipAnimacion c;
    c.bucle = false;
    for (int i = 0; i < 200; ++i) {
        c.fotogramas.push_back({i + 1, 1});
        c.eventos.push_back({"ev" + std::to_string(i), i, "logico", "", i});
    }
    ClipActor a(idDe("rafaga"), c);
    PuertosFalsos p;
    DestinoFalso d;
    a.avanza(1.0, p, d);
    MOTOR_CHECK(p.logicos.size() == 200);
    MOTOR_CHECK(p.logicos.front() == "ev0" && p.logicos.back() == "ev199");
    MOTOR_CHECK(d.recibidos.size() == 1 && d.recibidos[0] == 200);  // solo el sprite final
}

MOTOR_TEST_CASE(tope_de_drenado_guarda_el_resto_sin_perder_ni_duplicar) {
    ClipAnimacion c;
    c.bucle = false;
    const int n = 64 * (ClipActor::kMaxPasosPorAvance + 3);
    for (int i = 0; i < n; ++i) {
        c.fotogramas.push_back({i + 1, 1});
        c.eventos.push_back({"ev" + std::to_string(i), i, "logico", "", i});
    }
    ClipActor a(idDe("rafaga"), c);
    PuertosFalsos p;
    DestinoFalso d;
    a.avanza(10.0, p, d);
    MOTOR_CHECK(static_cast<int>(p.logicos.size()) < n);  // el tope corto el avance
    for (int i = 0; i < 10; ++i) a.avanza(0.0, p, d);
    MOTOR_CHECK(static_cast<int>(p.logicos.size()) == n);
    for (int i = 0; i < n; ++i) MOTOR_CHECK(p.logicos[static_cast<std::size_t>(i)] == "ev" + std::to_string(i));
}

MOTOR_TEST_CASE(no_ciclico_termina_y_deja_de_entregar) {
    ClipActor a(idDe("golpe"), clipTres(false));
    PuertosFalsos p;
    DestinoFalso d;
    a.avanza(0.0, p, d);
    a.avanza(5.0, p, d);
    MOTOR_CHECK(a.cursor().terminado);
    MOTOR_CHECK(d.recibidos == std::vector<int>({5, 7}));
    const std::size_t eventos = p.logicos.size() + p.visuales.size() + p.sonidos.size();
    for (int i = 0; i < 5; ++i) a.avanza(1.0, p, d);
    MOTOR_CHECK(d.recibidos == std::vector<int>({5, 7}));
    MOTOR_CHECK(p.logicos.size() + p.visuales.size() + p.sonidos.size() == eventos);
}

MOTOR_TEST_CASE(cambia_con_reiniciar_si_distinto_no_reinicia_el_mismo_idle) {
    ClipActor a(idDe("idle"), clipTres(true));
    PuertosFalsos p;
    DestinoFalso d;
    a.avanza(0.15, p, d);
    for (int i = 0; i < 4; ++i) {
        a.cambia(idDe("idle"), clipTres(true), PoliticaCambio::ReiniciarSiDistinto);
        a.avanza(0.001, p, d);
    }
    MOTOR_CHECK(a.cursor().fotograma == 1);
    MOTOR_CHECK(p.logicos == std::vector<std::string>({"e0"}));  // sin re-emitir e0
    MOTOR_CHECK(d.recibidos == std::vector<int>({6}));
    ClipAnimacion otro = clipTres(true);
    otro.fotogramas[0].sprite = 20;
    a.cambia(idDe("run"), otro, PoliticaCambio::ReiniciarSiDistinto);
    a.avanza(0.0, p, d);
    MOTOR_CHECK(a.id() == idDe("run") && a.cursor().fotograma == 0);
    MOTOR_CHECK(d.recibidos == std::vector<int>({6, 20}));
    MOTOR_CHECK(p.logicos == std::vector<std::string>({"e0", "e0"}));
}

MOTOR_TEST_CASE(tipo_desconocido_se_cuenta_y_repinta_solo_visual) {
    ClipAnimacion c = clipTres(true);
    c.eventos.push_back({"raro", 0, "otro", "", 3});
    ClipActor a(idDe("idle"), c);
    PuertosFalsos p;
    DestinoFalso d;
    MOTOR_CHECK(a.avanza(0.0, p, d) == 1);
    MOTOR_CHECK(p.logicos == std::vector<std::string>({"e0"}));
    a.avanza(0.15, p, d);  // fotograma 1 (visual e1)
    PuertosFalsos q;
    a.repinta(q);
    a.repinta(q);
    MOTOR_CHECK(q.visuales == std::vector<std::string>({"e1", "e1"}));
    MOTOR_CHECK(q.logicos.empty() && q.sonidos.empty());
}

MOTOR_TEST_CASE(destino_funcion_reenvia_el_sprite) {
    int ultimo = 0;
    Game::DestinoFuncion f([&](int s) { ultimo = s; });
    ClipActor a(idDe("idle"), clipTres(true));
    PuertosFalsos p;
    a.avanza(0.0, p, f);
    MOTOR_CHECK(ultimo == 5);
}

namespace {
// Rafaga de n fotogramas de 1 ms, un evento logico por fotograma; agota el tope de drenado.
ClipAnimacion rafaga(int n) {
    ClipAnimacion c;
    c.bucle = false;
    for (int i = 0; i < n; ++i) {
        c.fotogramas.push_back({i + 1, 1});
        c.eventos.push_back({"ev" + std::to_string(i), i, "logico", "", i});
    }
    return c;
}
}  // namespace

MOTOR_TEST_CASE(cambiar_tras_agotar_el_tope_descarta_el_resto_con_cada_politica) {
    const PoliticaCambio pols[3] = {PoliticaCambio::ReiniciarSiempre, PoliticaCambio::ReiniciarSiDistinto,
                                    PoliticaCambio::ConservarFase};
    for (const PoliticaCambio pol : pols) {
        ClipActor a(idDe("a"), rafaga(64 * (ClipActor::kMaxPasosPorAvance + 3)));
        PuertosFalsos p;
        DestinoFalso d;
        a.avanza(10.0, p, d);  // agota el tope: queda resto del clip A
        PuertosFalsos q;
        a.cambia(idDe("b"), clipTres(false), pol);
        a.avanza(0.0, q, d);  // sin tiempo real: el clip B no debe consumir el resto de A
        MOTOR_CHECK(!a.cursor().terminado);
        MOTOR_CHECK(q.logicos.size() + q.visuales.size() + q.sonidos.size() <= 1);
    }
}

MOTOR_TEST_CASE(cambiar_reentrega_el_sprite_aunque_coincida_con_el_ultimo) {
    ClipActor a(idDe("a"), clipTres(true));  // sprite 5 en el fotograma 0
    PuertosFalsos p;
    DestinoFalso d;
    a.avanza(0.0, p, d);
    MOTOR_CHECK(d.recibidos == std::vector<int>({5}));
    a.cambia(idDe("b"), clipTres(true), PoliticaCambio::ReiniciarSiDistinto);  // tambien empieza en 5
    a.avanza(0.0, p, d);
    MOTOR_CHECK(d.recibidos == std::vector<int>({5, 5}));
}

int main() { return motor_test::runAllTests(); }
