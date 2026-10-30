// U3-05.E04 -- Avanzar el cursor de un clip (Game::avanzaClip). Sin GL ni assets.
#include "TestHarness.h"

#include "Game/CursorClip.h"

#include <cmath>
#include <cstdint>
#include <string>

using Game::ClipAnimacion;
using Game::CursorClip;
using Game::PasoClip;

namespace {

// Tres fotogramas de 70/50/90 ms (total 210) con un evento "e0".."e2" en cada uno.
ClipAnimacion clipTres(bool bucle, bool conEventos = true) {
    ClipAnimacion c;
    c.bucle = bucle;
    const int duraciones[3] = {70, 50, 90};
    for (int i = 0; i < 3; ++i) {
        c.fotogramas.push_back({i + 1, duraciones[i]});
        if (conEventos) c.eventos.push_back({"e" + std::to_string(i), i, "logico", "", i});
    }
    return c;
}

std::string listaIds(const PasoClip& p) {
    std::string s;
    for (const auto& e : p.eventos) s += e.id + ",";
    return s;
}

// Continua un paso con dt = 0 hasta agotar tiempoPendiente; acumula ids de eventos.
PasoClip hastaAgotar(const CursorClip& ini, double dt, const ClipAnimacion& c, std::size_t max,
                     std::uint64_t* huella, std::uint64_t* cuenta) {
    PasoClip p = Game::avanzaClip(ini, dt, c, max);
    for (;;) {
        for (const auto& e : p.eventos) {
            *huella = *huella * 1099511628211ULL + static_cast<std::uint64_t>(e.fotograma + 1);
            ++*cuenta;
        }
        if (p.tiempoPendiente <= 0.0) return p;
        p = Game::avanzaClip(p.cursor, p.tiempoPendiente, c, max);
    }
}

bool cercanos(const CursorClip& a, const CursorClip& b) {
    return a.fotograma == b.fotograma && a.vueltas == b.vueltas && a.terminado == b.terminado &&
           std::fabs(a.enFotogramaMs - b.enFotogramaMs) < 1e-3;
}

}  // namespace

MOTOR_TEST_CASE(inicio_emite_eventos_del_fotograma_cero_una_vez) {
    const ClipAnimacion c = clipTres(true);
    const PasoClip p = Game::avanzaClip({}, 0.0, c);
    MOTOR_CHECK(listaIds(p) == "e0,");
    MOTOR_CHECK(p.cursor.iniciado && p.cursor.fotograma == 0 && p.tiempoPendiente == 0.0);
    const PasoClip q = Game::avanzaClip(p.cursor, 0.0, c);
    MOTOR_CHECK(q.eventos.empty());
}

MOTOR_TEST_CASE(avance_dentro_de_un_fotograma_no_cambia_de_fotograma) {
    const ClipAnimacion c = clipTres(true);
    const PasoClip p = Game::avanzaClip(Game::avanzaClip({}, 0.0, c).cursor, 0.030, c);
    MOTOR_CHECK(p.eventos.empty());
    MOTOR_CHECK(p.cursor.fotograma == 0 && std::fabs(p.cursor.enFotogramaMs - 30.0) < 1e-9);
}

MOTOR_TEST_CASE(cruza_un_limite_y_varios_en_orden) {
    const ClipAnimacion c = clipTres(true);
    const CursorClip ini = Game::avanzaClip({}, 0.0, c).cursor;
    const PasoClip uno = Game::avanzaClip(ini, 0.080, c);
    MOTOR_CHECK(listaIds(uno) == "e1," && uno.cursor.fotograma == 1);
    MOTOR_CHECK(std::fabs(uno.cursor.enFotogramaMs - 10.0) < 1e-9);
    const PasoClip varios = Game::avanzaClip(ini, 0.130, c);
    MOTOR_CHECK(listaIds(varios) == "e1,e2," && varios.cursor.fotograma == 2);
}

MOTOR_TEST_CASE(bucle_vuelve_a_emitir_el_fotograma_cero) {
    const ClipAnimacion c = clipTres(true);
    const PasoClip p = Game::avanzaClip({}, 0.250, c);  // inicio + e1 + e2 + vuelta
    MOTOR_CHECK(listaIds(p) == "e0,e1,e2,e0,");
    MOTOR_CHECK(p.cursor.vueltas == 1 && p.cursor.fotograma == 0 && !p.cursor.terminado);
    MOTOR_CHECK(std::fabs(p.cursor.enFotogramaMs - 40.0) < 1e-9);
}

MOTOR_TEST_CASE(sin_bucle_termina_en_el_ultimo_y_no_emite_mas) {
    const ClipAnimacion c = clipTres(false);
    const PasoClip p = Game::avanzaClip({}, 5.0, c);
    MOTOR_CHECK(listaIds(p) == "e0,e1,e2,");
    MOTOR_CHECK(p.cursor.terminado && p.cursor.fotograma == 2 && p.tiempoPendiente == 0.0);
    const PasoClip q = Game::avanzaClip(p.cursor, 5.0, c);
    MOTOR_CHECK(q.eventos.empty() && q.cursor.terminado && q.cursor.fotograma == 2);
}

MOTOR_TEST_CASE(presupuesto_emite_exactamente_max_y_deja_tiempo_pendiente) {
    const ClipAnimacion c = clipTres(true);
    const PasoClip p = Game::avanzaClip({}, 1.0, c, 4);
    MOTOR_CHECK(p.eventos.size() == 4 && listaIds(p) == "e0,e1,e2,e0,");
    MOTOR_CHECK(p.tiempoPendiente > 0.0);
    MOTOR_CHECK(p.cursor.fotograma == 0 && p.cursor.enFotogramaMs == 0.0 && p.cursor.vueltas == 1);
    const PasoClip q = Game::avanzaClip(p.cursor, p.tiempoPendiente, c, 2);
    MOTOR_CHECK(listaIds(q) == "e1,e2,");
}

MOTOR_TEST_CASE(dt_invalido_no_cambia_nada) {
    const ClipAnimacion c = clipTres(true);
    const CursorClip ini = Game::avanzaClip({}, 0.030, c).cursor;
    const double malos[3] = {-0.1, std::nan(""), INFINITY};
    for (const double dt : malos) {
        const PasoClip p = Game::avanzaClip(ini, dt, c);
        MOTOR_CHECK(p.eventos.empty() && p.tiempoPendiente == 0.0);
        MOTOR_CHECK(cercanos(p.cursor, ini) && p.cursor.iniciado == ini.iniciado);
    }
    MOTOR_CHECK(!Game::avanzaClip({}, -1.0, c).cursor.iniciado);
}

MOTOR_TEST_CASE(dt_cero_sin_pendiente_no_cambia_el_cursor) {
    const ClipAnimacion c = clipTres(true);
    const CursorClip ini = Game::avanzaClip({}, 0.095, c).cursor;
    const PasoClip p = Game::avanzaClip(ini, 0.0, c);
    MOTOR_CHECK(p.eventos.empty() && p.tiempoPendiente == 0.0 && cercanos(p.cursor, ini));
}

MOTOR_TEST_CASE(dt_enorme_con_presupuesto_pequeno_igual_que_pasos_repartidos) {
    ClipAnimacion c = clipTres(true);
    c.eventos.resize(1);  // un evento por vuelta: 4.7M eventos, prueba rapida
    std::uint64_t h1 = 0, n1 = 0, h2 = 0, n2 = 0;
    const PasoClip a = hastaAgotar({}, 1e6, c, 7, &h1, &n1);
    CursorClip k;
    for (int i = 0; i < 1000; ++i) k = hastaAgotar(k, 1000.0, c, 7, &h2, &n2).cursor;
    MOTOR_CHECK(n1 == n2 && h1 == h2 && n1 > 4700000);
    MOTOR_CHECK(cercanos(a.cursor, k));
}

MOTOR_TEST_CASE(sesenta_segundos_en_pasos_de_un_sesentavo_igual_que_un_paso) {
    for (const bool bucle : {true, false}) {
        const ClipAnimacion c = clipTres(bucle);
        std::uint64_t h1 = 0, n1 = 0, h2 = 0, n2 = 0;
        const PasoClip a = hastaAgotar({}, 60.0, c, 5, &h1, &n1);
        CursorClip k;
        for (int i = 0; i < 3600; ++i) k = hastaAgotar(k, 1.0 / 60.0, c, 5, &h2, &n2).cursor;
        MOTOR_CHECK(n1 == n2 && h1 == h2 && n1 >= 3);
        MOTOR_CHECK(cercanos(a.cursor, k));
    }
}

MOTOR_TEST_CASE(clip_sin_eventos_avanza_por_aritmetica) {
    const ClipAnimacion c = clipTres(true, false);
    const PasoClip p = Game::avanzaClip({}, 1e9, c);  // 1e12 ms: sin bucles por vuelta
    MOTOR_CHECK(p.eventos.empty() && p.cursor.vueltas > 0 && p.tiempoPendiente == 0.0);
    MOTOR_CHECK(p.cursor.fotograma >= 0 && p.cursor.fotograma < 3);
    MOTOR_CHECK(Game::avanzaClip({}, 1e9, clipTres(false, false)).cursor.terminado);
}

MOTOR_TEST_CASE(avanzar_exactamente_hasta_el_limite_entra_en_el_fotograma_siguiente) {
    // 500 y 250 ms (dt=0.5 s exacto): el limite pertenece al fotograma siguiente.
    ClipAnimacion c;
    c.bucle = true;
    c.fotogramas.push_back({1, 500});
    c.fotogramas.push_back({2, 250});
    c.eventos.push_back({"e0", 0, "logico", "", 0});
    c.eventos.push_back({"e1", 1, "logico", "", 1});
    const CursorClip ini = Game::avanzaClip({}, 0.0, c).cursor;
    const PasoClip p = Game::avanzaClip(ini, 0.5, c);
    MOTOR_CHECK(listaIds(p) == "e1," && p.cursor.fotograma == 1 && p.cursor.enFotogramaMs == 0.0);
    // Y el clip de tres fotogramas: dt = 0.070 s justo en el limite del fotograma 0.
    const ClipAnimacion t = clipTres(true);
    const PasoClip q = Game::avanzaClip(Game::avanzaClip({}, 0.0, t).cursor, 0.070, t);
    MOTOR_CHECK(listaIds(q) == "e1," && q.cursor.fotograma == 1);
    // Ultimo fotograma sin bucle: llegar justo al final termina.
    ClipAnimacion s = c;
    s.bucle = false;
    const PasoClip r = Game::avanzaClip(Game::avanzaClip(ini, 0.5, s).cursor, 0.25, s);
    MOTOR_CHECK(r.cursor.terminado && r.cursor.fotograma == 1);
}

MOTOR_TEST_CASE(dt_finito_que_desborda_al_pasar_a_ms_se_rechaza) {
    for (const bool conEventos : {true, false}) {
        const ClipAnimacion c = clipTres(true, conEventos);
        const CursorClip ini = Game::avanzaClip({}, 0.030, c).cursor;
        const PasoClip p = Game::avanzaClip(ini, 1.7e308, c, 3);
        MOTOR_CHECK(p.eventos.empty() && p.tiempoPendiente == 0.0);
        MOTOR_CHECK(cercanos(p.cursor, ini) && std::isfinite(p.cursor.enFotogramaMs));
        MOTOR_CHECK(!Game::avanzaClip({}, 1.7e308, c).cursor.iniciado);
    }
}

int main() { return motor_test::runAllTests(); }
