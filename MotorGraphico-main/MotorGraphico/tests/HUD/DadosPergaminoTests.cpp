// H2-01 — test_pergamino: el dado se ve. Las caras con sus puntos y el color
// de lo que cuentan (Ed. 3), el panel de la ultima tirada del combate y la
// tira de la prueba de un dialogo, medidos sin ventana.
#include <algorithm>
#include <string>
#include <vector>

#include "HUD/Iconos.h"
#include "HUD/Pergamino/DadosPergamino.h"
#include "HUD/Pergamino/DialogoHoja.h"
#include "HUD/Pergamino/FuenteAtlas.h"
#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"
#include "TestHarness.h"

using namespace Pergamino;

namespace {

struct Tinta {
    FuenteAtlas letras;
    Hud::Iconos iconos;
    Tinta() {
        MOTOR_CHECK(letras.carga("assets/textures/letras_pergamino.json").isOk());
        MOTOR_CHECK(iconos.cargar("assets/textures/iconos.json").isOk());
    }
};

const Tinta& tinta() {
    static const Tinta t;
    return t;
}

RPG::PoolResult pool(std::vector<int> caras, float exitos, bool pifia = false) {
    RPG::PoolResult r;
    r.rolls = std::move(caras);
    r.successes = exitos;
    r.botch = pifia;
    return r;
}

// Ataque de Nerea (6, 5, 3: 1,5) contra el saqueador (5, 2: 0,5): exito.
RPG::TiradaVisible ataque() {
    RPG::TiradaVisible t;
    t.numero = 1;
    t.que = "Ataque";
    t.quien = "Nerea";
    t.contra = "Perdido saqueador";
    t.tirada = pool({6, 5, 3}, 1.5f);
    t.rival = pool({5, 2}, 0.5f);
    t.enfrentada = true;
    t.grado = RPG::Degree::SUCCESS;
    return t;
}

struct Caja {
    float x0 = 1e9f, y0 = 1e9f, x1 = -1e9f, y1 = -1e9f;
};

Caja caja(const Lienzo& l, std::size_t desde = 0) {
    Caja c;
    for (std::size_t i = desde; i < l.trazos().size(); ++i)
        for (const Vector2& v : l.trazos()[i].p) {
            c.x0 = std::min(c.x0, v.x);
            c.y0 = std::min(c.y0, v.y);
            c.x1 = std::max(c.x1, v.x);
            c.y1 = std::max(c.y1, v.y);
        }
    return c;
}

bool igual(const Vector4& a, const Vector4& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

int deColor(const Lienzo& l, const Vector4& c) {
    return static_cast<int>(
        std::count_if(l.trazos().begin(), l.trazos().end(), [&](const Trazo& t) { return igual(t.color, c); }));
}

}  // namespace

MOTOR_TEST_CASE(dados_cuentan_como_el_reglamento) {
    using namespace dados;
    // Normal: el 6 es un exito, el 5 medio y el 4 nada.
    const RPG::PoolResult normal = pool({6, 5, 4, 1}, 1.5f);
    MOTOR_CHECK(cuentaDe(normal, 6) == kExito);
    MOTOR_CHECK(cuentaDe(normal, 5) == kMedio);
    MOTOR_CHECK(cuentaDe(normal, 4) == kNada);
    MOTOR_CHECK(cuentaDe(normal, 1) == kNada);  // un 1 suelto no es pifia
    // Con ventaja el 4 tambien vale medio (lo dicen los exitos que trae).
    MOTOR_CHECK(cuentaDe(pool({6, 5, 4}, 2.0f), 4) == kMedio);
    // Con desventaja solo cuenta el 6.
    MOTOR_CHECK(cuentaDe(pool({6, 5}, 1.0f), 5) == kNada);
    MOTOR_CHECK(cuentaDe(pool({6, 5}, 1.0f), 6) == kExito);
    // Pifia: los 1 en rojo.
    const RPG::PoolResult pifia = pool({1, 1, 3}, 0.0f, true);
    MOTOR_CHECK(cuentaDe(pifia, 1) == kPifia);
    MOTOR_CHECK(cuentaDe(pifia, 3) == kNada);
    MOTOR_CHECK(exitos(1.5f) == "1,5" && exitos(2.0f) == "2" && exitos(0.0f) == "0");
}

MOTOR_TEST_CASE(dados_una_cara_tiene_tantos_puntos_como_su_valor) {
    for (int v = 1; v <= 6; ++v) {
        Lienzo l;
        dados::cara(l, v, 10.0f, 20.0f, 21.0f, dados::kNada);
        MOTOR_CHECK(static_cast<int>(l.cuantos()) == 2 + v);  // borde, fondo y los puntos
        const Caja c = caja(l);
        MOTOR_CHECK(c.x0 >= 10.0f && c.y0 >= 20.0f && c.x1 <= 31.0f && c.y1 <= 41.0f);
    }
    // En el 6 los tres puntos de cada columna no se tocan (si no, se lee
    // como dos rayas): 3 px de punto cada 5.
    Lienzo seis;
    dados::cara(seis, 6, 0.0f, 0.0f, 21.0f, dados::kExito);
    std::vector<float> ys;
    for (std::size_t i = 2; i < seis.trazos().size(); ++i)
        if (seis.trazos()[i].p[0].x < 10.0f) ys.push_back(seis.trazos()[i].p[0].y);
    std::sort(ys.begin(), ys.end());
    MOTOR_CHECK(ys.size() == 3 && ys[1] - ys[0] == 5.0f && ys[2] - ys[1] == 5.0f);
}

MOTOR_TEST_CASE(dados_el_panel_del_combate_cabe_en_su_sitio) {
    const Pincel p(tinta().letras, tinta().iconos);
    // Con rival: dos filas y 112 de alto; la sombra del papel baja 4 px.
    Lienzo l;
    MOTOR_CHECK(pintaTiradaCombate(l, p, ataque(), 972.0f, 428.0f, 0.0f) == 112.0f);
    Caja c = caja(l);
    MOTOR_CHECK(c.x0 >= 971.0f && c.y0 >= 427.0f && c.x1 <= 1253.0f && c.y1 <= 428.0f + 112.0f + 5.0f);
    // Recien salida lleva su halo de tres anillos, sin pisar el registro
    // (hasta y = 418) ni la bandeja (desde y = 548).
    Lienzo brilla;
    pintaTiradaCombate(brilla, p, ataque(), 972.0f, 428.0f, 1.0f);
    MOTOR_CHECK(brilla.cuantos() == l.cuantos() + 3);
    c = caja(brilla);
    MOTOR_CHECK(c.x0 >= 963.0f && c.y0 > 418.0f && c.y1 <= 548.0f);
    // Sin rival (una curacion): una fila y 82 de alto.
    RPG::TiradaVisible sola = ataque();
    sola.enfrentada = false;
    sola.rival = RPG::PoolResult{};
    Lienzo s;
    MOTOR_CHECK(pintaTiradaCombate(s, p, sola, 972.0f, 428.0f, 0.0f) == 82.0f);
    MOTOR_CHECK(s.cuantos() < l.cuantos());
    // Un rival sin dados (sin DES ni CON) tiene su fila: «sin dados».
    RPG::TiradaVisible inerme = ataque();
    inerme.rival = RPG::PoolResult{};
    Lienzo i;
    MOTOR_CHECK(pintaTiradaCombate(i, p, inerme, 972.0f, 428.0f, 0.0f) == 112.0f);
    // Doce dados se encogen y no se salen, y el nombre sigue a la izquierda.
    RPG::TiradaVisible muchos = ataque();
    muchos.tirada = pool({6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 6}, 4.0f);
    muchos.bono = 0.5f;  // «(+0,5) 4,5»
    Lienzo m;
    pintaTiradaCombate(m, p, muchos, 972.0f, 428.0f, 0.0f);
    c = caja(m);
    MOTOR_CHECK(c.x0 >= 971.0f && c.x1 <= 1253.0f && c.y1 <= 428.0f + 112.0f + 5.0f);
    // Una de las tiradas de un area lleva su «2/3» en la cabecera, sin salirse.
    Lienzo area;
    pintaTiradaCombate(area, p, ataque(), 972.0f, 428.0f, 0.0f, 2, 3);
    MOTOR_CHECK(area.cuantos() > l.cuantos());
    c = caja(area);
    MOTOR_CHECK(c.x0 >= 971.0f && c.x1 <= 1253.0f);
    Lienzo una;  // «1/1» no se escribe
    pintaTiradaCombate(una, p, ataque(), 972.0f, 428.0f, 0.0f, 1, 1);
    MOTOR_CHECK(una.cuantos() == l.cuantos());
    // De 20 px mientras quepan; si no, mas pequenos, pero no de menos de 10.
    MOTOR_CHECK(dados::anchoFila(2, 120.0f) == 45.0f);
    MOTOR_CHECK(dados::anchoFila(6, 120.0f) <= 120.0f && dados::anchoFila(6, 120.0f) > 100.0f);
    MOTOR_CHECK(dados::anchoFila(12, 120.0f) == 12.0f * 13.0f - 3.0f);
}

MOTOR_TEST_CASE(dados_la_pifia_se_ve_y_perder_no_es_pifiar) {
    const Pincel p(tinta().letras, tinta().iconos);
    // Perder la enfrentada: FALLO, sin rojo.
    RPG::TiradaVisible pierde = ataque();
    pierde.tirada = pool({5, 3, 2}, 0.5f);
    pierde.rival = pool({6, 6}, 2.0f);
    pierde.grado = RPG::Degree::BOTCH;
    MOTOR_CHECK(!pierde.pifia());
    MOTOR_CHECK(std::string(dados::rotulo(pierde)) == "FALLO");
    Lienzo l;
    pintaTiradaCombate(l, p, pierde, 972.0f, 426.0f, 0.0f);
    // DIALOGOS-01 (Oriol, 13/10): perder lleva el marco rojo (4 lados), y nada mas en rojo.
    MOTOR_CHECK(deColor(l, Color::kError) == 4);
    // Pifiar: ni un 5 ni un 6 y dos 1. Marco rojo, rotulo en su caja roja y
    // los dos 1 con el borde rojo.
    RPG::TiradaVisible pifia = pierde;
    pifia.tirada = pool({1, 1, 3}, 0.0f, true);
    MOTOR_CHECK(pifia.pifia());
    MOTOR_CHECK(std::string(dados::rotulo(pifia)) == "¡PIFIA!");
    Lienzo r;
    pintaTiradaCombate(r, p, pifia, 972.0f, 426.0f, 0.0f);
    MOTOR_CHECK(deColor(r, Color::kError) >= 4 + 1 + 2);
    // La pifia que una proteccion sube a parcial ya no es pifia.
    RPG::TiradaVisible salvada = pifia;
    salvada.grado = RPG::Degree::PARTIAL;
    MOTOR_CHECK(!salvada.pifia());
    MOTOR_CHECK(std::string(dados::rotulo(salvada)) == "PARCIAL");
    // El critico de un defensor que pifia: el grado lo dice.
    RPG::TiradaVisible critico = ataque();
    critico.rival = pool({1, 1}, 0.0f, true);
    critico.grado = RPG::Degree::CRITICAL;
    MOTOR_CHECK(std::string(dados::rotulo(critico)) == "CRÍTICO");
}

MOTOR_TEST_CASE(dados_la_tira_del_dialogo_se_apoya_en_su_esquina) {
    const Pincel p(tinta().letras, tinta().iconos);
    RPG::TiradaVisible t;
    t.numero = 3;
    t.que = "Percepcion";
    t.quien = "Nerea";
    t.tirada = pool({6, 4, 2}, 1.0f);
    t.rival = pool({5, 1}, 0.5f);
    t.enfrentada = true;
    t.grado = RPG::Degree::SUCCESS;
    Lienzo l;
    pintaTiradaDialogo(l, p, t, 1076.0f, 522.0f);
    const Caja c = caja(l);
    MOTOR_CHECK(c.x1 <= 1076.0f + 1.0f && c.y1 <= 522.0f + 5.0f);  // con la sombra
    MOTOR_CHECK(c.y0 >= 522.0f - 44.0f - 1.0f);
    MOTOR_CHECK(c.x0 > 180.0f);  // no llega al rotulo del hablante
}

MOTOR_TEST_CASE(dados_el_dialogo_lleva_la_tirada_de_su_beat_en_la_ultima_pagina) {
    const Pincel p(tinta().letras, tinta().iconos);
    const std::vector<std::string> beat = {"Algo en este blanco no es blanco de verdad. Te paras a mirar.",
                                           "(Tirada de Percepcion contra el vacio.)"};
    RPG::TiradaVisible t;
    t.numero = 1;
    t.que = "Percepcion";
    t.tirada = pool({6, 5}, 1.5f);
    t.rival = pool({4, 3}, 0.0f);
    t.enfrentada = true;
    t.grado = RPG::Degree::SUCCESS;
    // Lo que queda por encima de la hoja (y = 720 - 28 - 162 = 530, el
    // rotulo del hablante sube hasta 512): la tira de la tirada.
    auto conTira = [&](const DialogoHoja& d) {
        Lienzo l;
        d.pinta(l, p, 1280, 720);
        return caja(l).y0 < 500.0f;
    };

    // El beat se abre y su tirada sale en el mismo fotograma.
    DialogoHoja d;
    d.mira("", beat, tinta().letras);
    d.miraTirada(t);
    MOTOR_CHECK(d.conTirada() && d.paginas() == 2);
    MOTOR_CHECK(!conTira(d));  // la primera pagina aun no la ensena
    MOTOR_CHECK(d.avanza());
    MOTOR_CHECK(conTira(d));   // la ultima, la de «Tirada de...», si
    // La misma tirada no se pega al dialogo siguiente (el del resultado).
    d.mira("", {"Lo ves claro: esto es un vacio blanco infinito."}, tinta().letras);
    d.miraTirada(t);
    MOTOR_CHECK(!d.conTirada() && !conTira(d));
    // Ni una tirada hecha fuera del dialogo al siguiente que se abra.
    d.cierra();
    t.numero = 2;
    d.miraTirada(t);
    d.mira("Guardia", {"Alto ahi."}, tinta().letras);
    d.miraTirada(t);
    MOTOR_CHECK(!d.conTirada());
    // Una partida nueva cuenta desde cero: su primera tirada tambien se ve.
    RPG::TiradaVisible nueva = t;
    nueva.numero = 0;
    d.miraTirada(nueva);
    nueva.numero = 1;
    d.mira("", beat, tinta().letras);
    d.miraTirada(nueva);
    MOTOR_CHECK(d.conTirada());
}
