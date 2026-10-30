// U3-05.E04 -- Avanzar el cursor de un clip (Game::avanzaClip). Sin GL ni assets.
#include "TestHarness.h"

#include "Game/CursorClip.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

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

// Clip con duraciones dadas y eventos (id, fotograma) en orden declarado.
ClipAnimacion clipCon(const std::vector<int>& duraciones,
                      const std::vector<std::pair<std::string, int>>& eventos, bool bucle = true) {
    ClipAnimacion c;
    c.bucle = bucle;
    for (std::size_t i = 0; i < duraciones.size(); ++i)
        c.fotogramas.push_back({static_cast<int>(i) + 1, duraciones[i]});
    int orden = 0;
    for (const auto& e : eventos) c.eventos.push_back({e.first, e.second, "logico", "", orden++});
    return c;
}

// Sigue llamando mientras haya pendiente (dt = tiempoPendiente); la huella incluye id y fotograma.
PasoClip hastaAgotar(const CursorClip& ini, double dt, const ClipAnimacion& c, std::size_t max,
                     std::uint64_t* huella, std::uint64_t* cuenta) {
    PasoClip p = Game::avanzaClip(ini, dt, c, max);
    for (;;) {
        for (const auto& e : p.eventos) {
            for (const char ch : e.id) *huella = *huella * 1099511628211ULL + static_cast<unsigned char>(ch);
            *huella = *huella * 1099511628211ULL + static_cast<std::uint64_t>(e.fotograma + 1);
            ++*cuenta;
        }
        if (!p.pendiente) return p;
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
    MOTOR_CHECK(p.pendiente && p.tiempoPendiente > 0.0);
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
    c.eventos.resize(1);  // un evento por vuelta de 210 ms: ~47600 eventos en 1e4 s
    std::uint64_t h1 = 0, n1 = 0, h2 = 0, n2 = 0;
    const PasoClip a = hastaAgotar({}, 1e4, c, 7, &h1, &n1);
    CursorClip k;
    for (int i = 0; i < 1000; ++i) k = hastaAgotar(k, 10.0, c, 7, &h2, &n2).cursor;
    MOTOR_CHECK(n1 == n2 && h1 == h2 && n1 > 47000 && n1 < 48000);
    MOTOR_CHECK(cercanos(a.cursor, k));
}

MOTOR_TEST_CASE(sesenta_segundos_en_pasos_de_un_sesentavo_igual_que_un_paso) {
    for (const bool bucle : {true, false}) {
        const ClipAnimacion c = clipTres(bucle);
        std::uint64_t h1 = 0, n1 = 0, h2 = 0, n2 = 0;
        const PasoClip a = hastaAgotar({}, 60.0, c, 5, &h1, &n1);
        CursorClip k;
        for (int i = 0; i < 3600; ++i) k = hastaAgotar(k, 1.0 / 60.0, c, 5, &h2, &n2).cursor;
        MOTOR_CHECK(n1 == n2 && h1 == h2);
        MOTOR_CHECK(bucle ? n1 > 800 : n1 == 3);  // 60 s / 210 ms * 3 eventos, o una sola pasada
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

MOTOR_TEST_CASE(grupo_de_tres_eventos_con_max_dos_se_reparte_sin_duplicar) {
    const ClipAnimacion c = clipCon({100, 100}, {{"a", 0}, {"b", 0}, {"c", 0}}, false);
    const PasoClip p = Game::avanzaClip({}, 0.0, c, 2);
    MOTOR_CHECK(listaIds(p) == "a,b," && p.pendiente && p.cursor.eventoPendiente == 2);
    const PasoClip q = Game::avanzaClip(p.cursor, p.tiempoPendiente, c, 2);
    MOTOR_CHECK(listaIds(q) == "c," && !q.pendiente && q.cursor.eventoPendiente == 0);
    const PasoClip r = Game::avanzaClip(q.cursor, 0.0, c, 2);
    MOTOR_CHECK(r.eventos.empty() && !r.pendiente);
}

MOTOR_TEST_CASE(corte_justo_en_frontera_es_pendiente_con_tiempo_cero) {
    const ClipAnimacion c = clipCon({70, 70}, {{"a", 0}, {"b", 1}, {"c", 1}}, false);
    const PasoClip p = Game::avanzaClip({}, 0.070, c, 2);
    MOTOR_CHECK(listaIds(p) == "a,b," && p.pendiente && p.tiempoPendiente == 0.0);
    MOTOR_CHECK(p.cursor.fotograma == 1 && p.cursor.eventoPendiente == 1);
    const PasoClip q = Game::avanzaClip(p.cursor, 0.0, c, 2);
    MOTOR_CHECK(listaIds(q) == "c," && !q.pendiente && q.cursor.eventoPendiente == 0);
}

MOTOR_TEST_CASE(repartir_dt_no_cambia_el_tick_en_que_se_cruza_la_frontera) {
    for (const bool bucle : {true, false}) {
        const ClipAnimacion c = clipCon({200, 100}, {{"x", 1}}, bucle);
        const PasoClip a = Game::avanzaClip({}, 0.2, c);
        CursorClip k;
        std::string ids;
        for (int i = 0; i < 12; ++i) {
            const PasoClip s = Game::avanzaClip(k, 1.0 / 60.0, c);
            ids += listaIds(s);
            k = s.cursor;
        }
        MOTOR_CHECK(listaIds(a) == "x," && ids == "x,");
        MOTOR_CHECK(a.cursor.fotograma == 1 && cercanos(a.cursor, k));
    }
    const ClipAnimacion s = clipCon({200, 100}, {}, true);  // misma deriva sin eventos
    CursorClip k;
    for (int i = 0; i < 12; ++i) k = Game::avanzaClip(k, 1.0 / 60.0, s).cursor;
    MOTOR_CHECK(k.fotograma == 1 && cercanos(k, Game::avanzaClip({}, 0.2, s).cursor));
}

MOTOR_TEST_CASE(max_cero_y_dt_enorme_y_duracion_cero_dejan_el_cursor_igual) {
    const ClipAnimacion c = clipTres(true);
    const CursorClip ini = Game::avanzaClip({}, 0.030, c).cursor;
    const PasoClip m0 = Game::avanzaClip(ini, 0.5, c, 0);
    MOTOR_CHECK(m0.eventos.empty() && !m0.pendiente && m0.tiempoPendiente == 0.0 && cercanos(m0.cursor, ini));
    MOTOR_CHECK(!Game::avanzaClip({}, 0.5, c, 0).cursor.iniciado);
    const PasoClip grande = Game::avanzaClip(ini, 1e306, c);
    MOTOR_CHECK(grande.eventos.empty() && !grande.pendiente && cercanos(grande.cursor, ini));
    ClipAnimacion mala = clipTres(true);
    mala.fotogramas[1].duracionMs = 0;
    const PasoClip d0 = Game::avanzaClip(ini, 0.5, mala);
    MOTOR_CHECK(d0.eventos.empty() && !d0.pendiente && cercanos(d0.cursor, ini));
}

MOTOR_TEST_CASE(cursor_incoherente_se_rechaza_sin_cambios) {
    const ClipAnimacion c = clipTres(true);
    const CursorClip base = Game::avanzaClip({}, 0.030, c).cursor;
    CursorClip malos[6] = {base, base, base, base, base, base};
    malos[0].fotograma = 3;
    malos[1].fotograma = -1;
    malos[2].enFotogramaMs = -1.0;
    malos[3].enFotogramaMs = std::nan("");
    malos[4].eventoPendiente = 5;  // e0 solo tiene un evento
    malos[5].eventoPendiente = -1;
    for (const CursorClip& m : malos) {
        const PasoClip p = Game::avanzaClip(m, 0.5, c);
        MOTOR_CHECK(p.eventos.empty() && !p.pendiente && p.tiempoPendiente == 0.0);
        MOTOR_CHECK(p.cursor.fotograma == m.fotograma && p.cursor.vueltas == m.vueltas &&
                    p.cursor.eventoPendiente == m.eventoPendiente);
    }
}

MOTOR_TEST_CASE(eventos_en_el_ultimo_fotograma_con_bucle) {
    const ClipAnimacion c = clipCon({50, 50}, {{"fin", 1}});
    const PasoClip p = Game::avanzaClip({}, 0.230, c);  // fin al entrar en el fotograma 1 (50 y 150 ms)
    MOTOR_CHECK(listaIds(p) == "fin,fin," && p.cursor.vueltas == 2 && p.cursor.fotograma == 0);
    MOTOR_CHECK(std::fabs(p.cursor.enFotogramaMs - 30.0) < 1e-9);
    std::uint64_t h = 0, n = 0;
    const PasoClip q = hastaAgotar({}, 0.230, c, 1, &h, &n);
    MOTOR_CHECK(n == 2 && cercanos(q.cursor, p.cursor));
}

int main() { return motor_test::runAllTests(); }
