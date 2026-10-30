// U4-07.E07 — PasosDePolvo (gl-free): el polvo de pasos del jugador. Sin glad.
#include "Game/Emisor/PasosDePolvo.h"

#include "TestHarness.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Runtime/Alturas.h"

using namespace Game::Emisor;

namespace {

constexpr double kTick = 1.0 / 60.0;
constexpr int kTW = 64, kTH = 32;

bool casi(float a, float b, float tol = 1e-3f) { return std::abs(a - b) <= tol; }

void tick(PasosDePolvo& p, GridCoord c, bool reducido = false, double dt = kTick) { p.avanza(dt, c, reducido); }
void ticks(PasosDePolvo& p, GridCoord c, int n, bool reducido = false) {
    for (int i = 0; i < n; ++i) tick(p, c, reducido);
}
std::size_t vivasEnBuffers(const PasosDePolvo& p) {
    std::size_t n = 0;
    for (const Emisor& e : p.emisores()) n += e.buffer().vivas();
    return n;
}
// Los pies de una celda: gridToScreen + (tw/2, th*0.9) a mano (sin IsoMath), con tw=64 y th=32.
Vector2 pies(int x, int y, float desfase = 0.0f) {
    return {static_cast<float>(x - y) * 32.0f + 32.0f, static_cast<float>(x + y) * 16.0f + 28.8f + desfase};
}

MOTOR_TEST_CASE(nube_normal_de_6_y_reducida_de_1) {
    for (const bool reducido : {false, true}) {
        PasosDePolvo p;
        p.monta(7, kTW, kTH, nullptr);
        tick(p, {5, 5}, reducido);      // la primera celda solo se memoriza
        MOTOR_CHECK(p.emisores().empty());
        tick(p, {6, 5}, reducido);      // el paso: nace la nube
        ticks(p, {6, 5}, 11, reducido); // 12 ticks = 0,2 s: la emision (0,15 s) ya acabo y nadie ha muerto (vida >= 0,5)
        MOTOR_CHECK(p.emisores().size() == 1);
        MOTOR_CHECK(p.emisores()[0].nacidas() == (reducido ? 1u : 6u));
        MOTOR_CHECK(p.emisores()[0].buffer().vivas() == (reducido ? 1u : 6u));
        MOTOR_CHECK(p.presupuesto().vivas == p.emisores()[0].buffer().vivas());
    }
}

MOTOR_TEST_CASE(adyacente_si_muro_no_salto_olvida_centinela_olvida) {
    PasosDePolvo p;
    p.monta(1, kTW, kTH, nullptr);
    tick(p, {2, 2});
    ticks(p, {2, 2}, 5);            // muro: misma celda, sin polvo
    MOTOR_CHECK(p.emisores().empty() && p.pasos() == 0);
    tick(p, {2, 3});                // adyacente (Manhattan 1)
    MOTOR_CHECK(p.emisores().size() == 1 && p.pasos() == 1);
    ticks(p, {2, 3}, 3);
    MOTOR_CHECK(p.presupuesto().vivas > 0);
    tick(p, {3, 4});                // diagonal = Manhattan 2: salto, olvida y sin polvo
    MOTOR_CHECK(p.emisores().empty() && p.presupuesto().vivas == 0 && p.pasos() == 1);
    tick(p, {4, 4});                // la celda del salto solo se habia memorizado: ahora si es un paso
    MOTOR_CHECK(p.emisores().size() == 1);
    tick(p, {40, 40});              // salto largo
    MOTOR_CHECK(p.emisores().empty() && p.presupuesto().vivas == 0);
    tick(p, {40, 41});
    MOTOR_CHECK(p.emisores().size() == 1);
    ticks(p, {40, 41}, 3);
    MOTOR_CHECK(p.presupuesto().vivas > 0);
    tick(p, kCeldaCentinela);       // cinematica: olvida
    MOTOR_CHECK(p.emisores().empty() && p.presupuesto().vivas == 0);
    tick(p, {7, 7});                // primera celda tras la centinela: solo se memoriza
    MOTOR_CHECK(p.emisores().empty());
    tick(p, {8, 7});
    MOTOR_CHECK(p.emisores().size() == 1);
    p.avanza(0.0, kCeldaCentinela, false);   // la centinela se atiende tambien con pausa
    MOTOR_CHECK(p.emisores().empty());
    // monta / olvida: la primera celda solo se memoriza
    PasosDePolvo q;
    q.monta(3, kTW, kTH, nullptr);
    tick(q, {1, 1});
    q.olvida();
    tick(q, {2, 1});
    MOTOR_CHECK(q.emisores().empty());
    tick(q, {3, 1});
    MOTOR_CHECK(q.emisores().size() == 1);
}

MOTOR_TEST_CASE(pies_en_la_celda_que_se_deja_con_desfase) {
    PasosDePolvo p;
    p.monta(1, kTW, kTH, nullptr);
    tick(p, {3, 2});
    tick(p, {4, 2});
    MOTOR_CHECK(p.emisores().size() == 1);
    const Vector2 e0 = pies(3, 2);
    const GrupoDeDibujo& g = p.emisores()[0].grupo;
    MOTOR_CHECK(casi(g.esquina.x, e0.x) && casi(g.esquina.y, e0.y));
    MOTOR_CHECK(g.tam.x == 64.0f && g.tam.y == 64.0f);
    MOTOR_CHECK(p.emisores()[0].origen.x == 0.0f && p.emisores()[0].origen.y == 0.0f);

    Alturas a;
    a.reinicia(10, 10);
    a.pon(3, 2, 2);    // la celda que se deja: sube 32 px
    a.pon(4, 2, 3);    // la de destino NO cuenta
    PasosDePolvo q;
    q.monta(1, kTW, kTH, &a);
    tick(q, {3, 2});
    tick(q, {4, 2});
    MOTOR_CHECK(q.emisores().size() == 1);
    const Vector2 e1 = pies(3, 2, -32.0f);
    MOTOR_CHECK(casi(q.emisores()[0].grupo.esquina.x, e1.x) && casi(q.emisores()[0].grupo.esquina.y, e1.y));
    // un salto (olvida) no suelta las alturas: el siguiente paso sigue llevando desfase
    a.pon(9, 9, 1);
    tick(q, {9, 9});
    tick(q, {9, 8});
    MOTOR_CHECK(q.emisores().size() == 1);
    const Vector2 e2 = pies(9, 9, -16.0f);
    MOTOR_CHECK(casi(q.emisores()[0].grupo.esquina.y, e2.y));
    // desmonta suelta el puntero: ya no hay desfase
    q.desmonta();
    q.monta(1, kTW, kTH, nullptr);
    tick(q, {3, 2});
    tick(q, {4, 2});
    MOTOR_CHECK(casi(q.emisores()[0].grupo.esquina.y, pies(3, 2).y));
}

MOTOR_TEST_CASE(dt_polvo_cero_congela_todo) {
    PasosDePolvo p;
    p.monta(5, kTW, kTH, nullptr);
    tick(p, {5, 5});
    p.avanza(0.0, {6, 5}, false);   // un paso en pausa: no nace nada
    MOTOR_CHECK(p.emisores().empty());
    tick(p, {6, 5});                // la deteccion espera: al reanudar nace
    MOTOR_CHECK(p.emisores().size() == 1);
    ticks(p, {6, 5}, 5);
    const std::uint64_t nacidas = p.emisores()[0].nacidas();
    const double tiempo = p.emisores()[0].tiempo();
    const std::vector<Particula> antes = p.emisores()[0].buffer().datos();
    MOTOR_CHECK(nacidas > 0 && !antes.empty());
    for (int i = 0; i < 120; ++i) p.avanza(0.0, {6, 5}, false);
    MOTOR_CHECK(p.emisores().size() == 1);
    MOTOR_CHECK(p.emisores()[0].nacidas() == nacidas && p.emisores()[0].tiempo() == tiempo);
    const std::vector<Particula>& despues = p.emisores()[0].buffer().datos();
    MOTOR_CHECK(despues.size() == antes.size());
    MOTOR_CHECK(std::memcmp(despues.data(), antes.data(), antes.size() * sizeof(Particula)) == 0);
    ticks(p, {6, 5}, 1);            // y al reanudar vuelve a correr
    MOTOR_CHECK(p.emisores()[0].tiempo() > tiempo);
}

MOTOR_TEST_CASE(tope_de_16_emisores_sin_evictar) {
    PasosDePolvo p;
    p.monta(9, kTW, kTH, nullptr);
    for (int x = 0; x <= 17; ++x) tick(p, {x, 0});   // 17 pasos en 18 ticks: ninguna nube ha acabado
    MOTOR_CHECK(p.pasos() == 17);
    MOTOR_CHECK(p.emisores().size() == static_cast<std::size_t>(kMaxEmisoresPolvo));
    const Vector2 primera = pies(0, 0), ultima = pies(15, 0);   // la 1.a sigue; la 17.a (celda 16) no se creo
    MOTOR_CHECK(casi(p.emisores().front().grupo.esquina.x, primera.x));
    MOTOR_CHECK(casi(p.emisores().back().grupo.esquina.x, ultima.x));
}

MOTOR_TEST_CASE(presupuesto_256_y_recuenta) {
    PasosDePolvo p;
    p.monta(2, kTW, kTH, nullptr);
    MOTOR_CHECK(p.presupuesto().maximo == 256);
    for (int x = 0; x <= 17; ++x) {
        tick(p, {x, 0});
        MOTOR_CHECK(p.presupuesto().vivas == vivasEnBuffers(p));
    }
    MOTOR_CHECK(p.presupuesto().vivas > 0 && p.presupuesto().vivas <= 256);
    ticks(p, {17, 0}, 90);          // todo se agota (vida <= 0,9 s) y se poda
    MOTOR_CHECK(p.emisores().empty() && p.presupuesto().vivas == 0);
    for (int x = 0; x < 4; ++x) tick(p, {x, 1});
    ticks(p, {3, 1}, 3);
    MOTOR_CHECK(p.presupuesto().vivas == vivasEnBuffers(p) && p.presupuesto().vivas > 0);
    p.olvida();
    MOTOR_CHECK(p.emisores().empty() && p.presupuesto().vivas == 0);
}

MOTOR_TEST_CASE(caminar_sin_parar_diez_segundos) {
    PasosDePolvo p;
    p.monta(11, kTW, kTH, nullptr);
    std::size_t maxEm = 0, maxVivas = 0;
    const int total = 600;          // 10 s
    for (int i = 0; i < total; ++i) {
        const int x = static_cast<int>(static_cast<double>(i) * kTick / 0.16);   // un paso cada 0,16 s
        tick(p, {x, 0});
        maxEm = std::max(maxEm, p.emisores().size());
        maxVivas = std::max(maxVivas, vivasEnBuffers(p));
        MOTOR_CHECK(p.emisores().size() <= static_cast<std::size_t>(kMaxEmisoresPolvo));
        MOTOR_CHECK(vivasEnBuffers(p) <= 256 && p.presupuesto().vivas <= 256);
    }
    MOTOR_CHECK(p.pasos() >= 60 && maxEm >= 2 && maxVivas > 6);
}

MOTOR_TEST_CASE(semillas_por_paso_y_determinismo) {
    PasosDePolvo a, b, c;
    a.monta(77, kTW, kTH, nullptr);
    b.monta(77, kTW, kTH, nullptr);
    c.monta(78, kTW, kTH, nullptr);
    for (int x = 0; x <= 6; ++x) {
        for (PasosDePolvo* p : {&a, &b, &c}) ticks(*p, {x, 0}, 3);
    }
    MOTOR_CHECK(a.emisores().size() >= 3 && a.emisores().size() == b.emisores().size());
    for (std::size_t i = 0; i < a.emisores().size(); ++i)
        for (std::size_t j = i + 1; j < a.emisores().size(); ++j)
            MOTOR_CHECK(a.emisores()[i].definicion().semilla != a.emisores()[j].definicion().semilla);
    MOTOR_CHECK(a.emisores()[0].definicion().semilla == mezclarSemilla(77ull ^ 0ull));
    MOTOR_CHECK(a.emisores()[1].definicion().semilla == mezclarSemilla(77ull ^ 1ull));
    MOTOR_CHECK(c.emisores()[0].definicion().semilla != a.emisores()[0].definicion().semilla);
    for (std::size_t i = 0; i < a.emisores().size(); ++i) {
        const auto& da = a.emisores()[i].buffer().datos();
        const auto& db = b.emisores()[i].buffer().datos();
        MOTOR_CHECK(da.size() == db.size());
        MOTOR_CHECK(da.empty() || std::memcmp(da.data(), db.data(), da.size() * sizeof(Particula)) == 0);
    }
}

MOTOR_TEST_CASE(listo_falso_sin_blanco) {
    PasosDePolvo p;
    MOTOR_CHECK(!p.listo());
    p.monta(1, kTW, kTH, nullptr);
    MOTOR_CHECK(!p.listo());
    p.ponBlanco(true);
    MOTOR_CHECK(p.listo());
    p.olvida();
    MOTOR_CHECK(p.listo());
    p.ponBlanco(false);
    MOTOR_CHECK(!p.listo());
}

MOTOR_TEST_CASE(proyeccion_de_una_nube_con_el_estilo_del_polvo) {
    const EstiloEmisor s = estiloDePolvo();
    MOTOR_CHECK(s.alfaMax == 0.7f && s.entrada == 0.02f);
    PasosDePolvo p;
    p.monta(4, kTW, kTH, nullptr);
    tick(p, {1, 1});
    tick(p, {2, 1});
    ticks(p, {2, 1}, 11);
    MOTOR_CHECK(p.emisores().size() == 1);
    std::vector<Game::Destello> out;
    const ResumenProyeccion r = proyectarParticulas(p.emisores()[0], s, 100000, out);
    MOTOR_CHECK(r.destellos == 6 && out.size() == 6 && r.descartados == 0);
    MOTOR_CHECK(r.quads == 6 * 4);              // Voluta: 4 pixeles, sin contorno
    MOTOR_CHECK(r.quads == quadsDeDestellos(out));
    for (const Game::Destello& d : out) {
        MOTOR_CHECK(d.alfa > 0.0f && d.alfa <= 0.7f + 1e-6f);
        MOTOR_CHECK(!d.suma);                   // el polvo tapa
    }
}

MOTOR_TEST_CASE(definicion_del_polvo_campo_a_campo) {
    const DefinicionEmisor d = definicionDePolvo(123456789ull);
    MOTOR_CHECK(d.familia == "polvo");
    MOTOR_CHECK(d.tasa == 40.0 && d.duracion == 0.15 && !d.cortar);
    MOTOR_CHECK(d.vidaMin == 0.5 && d.vidaMax == 0.9);
    MOTOR_CHECK(d.velocidadMin == 3.0 && d.velocidadMax == 8.0);
    MOTOR_CHECK(d.anguloCentro == -90.0 && d.abanico == 140.0);
    MOTOR_CHECK(d.maximo == 16 && d.opacidad == 1.0f && d.contraste == 0.0f);
    MOTOR_CHECK(d.semilla == 123456789ull);
    MOTOR_CHECK(kEscalaTasaReducida == 0.25);
    MOTOR_CHECK(kMaxEmisoresPolvo == 16 && kAforoPolvo == 16 && kPresupuestoPolvo == 256);
}

MOTOR_TEST_CASE(monta_reinicia_contador_y_desmonta_anula_alturas) {
    PasosDePolvo p, limpio;
    p.monta(50, kTW, kTH, nullptr);
    limpio.monta(50, kTW, kTH, nullptr);
    for (int x = 0; x < 4; ++x) tick(p, {x, 0});
    MOTOR_CHECK(p.pasos() == 3);
    p.monta(50, kTW, kTH, nullptr);
    MOTOR_CHECK(p.pasos() == 0 && p.emisores().empty());
    tick(p, {0, 0}); tick(p, {1, 0});
    tick(limpio, {0, 0}); tick(limpio, {1, 0});
    MOTOR_CHECK(p.emisores()[0].definicion().semilla == limpio.emisores()[0].definicion().semilla);
    MOTOR_CHECK(p.emisores()[0].definicion().semilla == mezclarSemilla(50ull));
    Alturas a;
    a.reinicia(10, 10);
    a.pon(3, 2, 2);
    PasosDePolvo q;
    q.monta(1, kTW, kTH, &a);
    q.desmonta();                   // sin un monta(nullptr) que lo tape
    tick(q, {3, 2});
    tick(q, {4, 2});
    MOTOR_CHECK(q.emisores().size() == 1 && casi(q.emisores()[0].grupo.esquina.y, pies(3, 2).y));
}

MOTOR_TEST_CASE(cien_ciclos_y_dos_segundos_sin_pasos) {
    PasosDePolvo p;
    for (int c = 0; c < 100; ++c) {
        p.monta(static_cast<std::uint32_t>(c), kTW, kTH, nullptr);
        tick(p, {0, 0}); tick(p, {1, 0}); tick(p, {2, 0});
        MOTOR_CHECK(p.presupuesto().vivas == vivasEnBuffers(p));
        p.olvida();
        MOTOR_CHECK(p.presupuesto().vivas == 0 && p.emisores().empty());
    }
    tick(p, {0, 0}); tick(p, {1, 0});
    ticks(p, {1, 0}, 120);          // 2 s sin pasos: todo agotado y podado
    MOTOR_CHECK(p.emisores().empty() && p.presupuesto().vivas == 0);
}

MOTOR_TEST_CASE(sin_polvo_en_combate_d_o7) {
    PasosDePolvo p;
    p.monta(5, kTW, kTH, nullptr);
    tick(p, {5, 5});
    tick(p, {6, 5});                // una nube previa al combate
    ticks(p, {6, 5}, 3);
    MOTOR_CHECK(p.emisores().size() == 1);
    const double t0 = p.emisores()[0].tiempo();
    p.avanza(kTick, {6, 5}, false, true);   // entra el combate: la nube sigue, no se olvida
    MOTOR_CHECK(p.emisores().size() == 1 && p.emisores()[0].tiempo() > t0);
    // en combate: saltos varias casillas y pasos adyacentes no levantan nada
    p.avanza(kTick, {9, 5}, false, true);
    p.avanza(kTick, {10, 5}, false, true);
    p.avanza(kTick, {10, 6}, false, true);
    MOTOR_CHECK(p.emisores().size() == 1 && p.pasos() == 1);
    for (int i = 0; i < 120; ++i) p.avanza(kTick, {10, 6}, false, true);   // la nube previa se agota sola
    MOTOR_CHECK(p.emisores().empty() && p.presupuesto().vivas == 0);
    // y al salir, la primera celda solo se memoriza (no hay nube por el salto de la vuelta)
    tick(p, {3, 3});
    MOTOR_CHECK(p.emisores().empty() && p.pasos() == 1);
    tick(p, {4, 3});
    MOTOR_CHECK(p.emisores().size() == 1 && p.pasos() == 2);
    // salir del combate con una nube viva y celda adyacente a la ultima de fuera: tampoco nace
    PasosDePolvo q;
    q.monta(5, kTW, kTH, nullptr);
    tick(q, {1, 1}); tick(q, {2, 1});
    q.avanza(kTick, {2, 1}, false, true);
    tick(q, {3, 1});
    MOTOR_CHECK(q.pasos() == 1);
}

// (c) La pausa D-O6 solo es vigilable en ApplicationBucle.cpp (Application::update no se puede construir sin GL):
// prueba textual sobre el codigo SIN comentarios ni cadenas.
std::string sinComentarios(const std::string& s) {
    std::string o;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s.compare(i, 2, "//") == 0) { while (i < s.size() && s[i] != '\n') ++i; o += '\n'; }
        else if (s.compare(i, 2, "/*") == 0) { const std::size_t f = s.find("*/", i + 2); i = f == std::string::npos ? s.size() : f + 1; }
        else if (s[i] == '"') { o += "\"\""; for (++i; i < s.size() && s[i] != '"'; ++i) if (s[i] == '\\') ++i; }
        else o += s[i];
    }
    return o;
}
std::string sinEspacios(const std::string& s) {
    std::string o;
    for (char c : s) if (!std::isspace(static_cast<unsigned char>(c))) o += c;
    return o;
}

MOTOR_TEST_CASE(pausa_del_polvo_en_application_bucle) {
    std::ifstream f(std::string(MOTOR_FUENTES_DIR) + "/src/Engine/ApplicationBucle.cpp");
    MOTOR_CHECK(f.good());
    std::stringstream ss;
    ss << f.rdbuf();
    const std::string c = sinEspacios(sinComentarios(ss.str()));
    const std::string decl = "constfloatdtPolvo=(mundo==0.0f||(m_session!=nullptr&&m_session->mode()==GameMode::Pause))?0.0f:deltaTime;";
    MOTOR_CHECK(c.find(decl) != std::string::npos);
    MOTOR_CHECK(c.find("m_vida->avanzaPolvo(dtPolvo,") != std::string::npos);
    MOTOR_CHECK(c.find("m_vida->avanza(deltaTime,m_session->playerPosition());") != std::string::npos);  // mascotas intactas
    MOTOR_CHECK(c.find("m_vida->avanzaPolvo(dtPolvo,ApplicationCinematica::enCurso(*this)?GridCoord{-1000,-1000}:"
                       "m_session->playerPosition(),m_session->mode()==GameMode::Battle);") != std::string::npos);
}

std::string fuente(const char* ruta) {
    std::ifstream f(std::string(MOTOR_FUENTES_DIR) + "/" + ruta);
    MOTOR_CHECK(f.good());
    std::stringstream ss;
    ss << f.rdbuf();
    return sinEspacios(sinComentarios(ss.str()));
}

MOTOR_TEST_CASE(cableado_de_hud_y_vida_nivel) {
    const std::string h = fuente("src/Engine/ApplicationHud.cpp");
    MOTOR_CHECK(h.find("if(m_vida!=nullptr)m_vida->ponBlanco(m_whiteTexture.get());if(m_aura){") != std::string::npos);
    const std::string v = fuente("src/Render/VidaNivel.cpp");
    MOTOR_CHECK(v.find("m_polvo.monta(llavor,tileW,tileH,&mapa.alturas());renderer.removeFromQueue(&m_polvo);"
                       "renderer.addToQueue(&m_polvo);") != std::string::npos);
    MOTOR_CHECK(v.find("renderer.removeFromQueue(&m_polvo);m_polvo.desmonta();") != std::string::npos);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
