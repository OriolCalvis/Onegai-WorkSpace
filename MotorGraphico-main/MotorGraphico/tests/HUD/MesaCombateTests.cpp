// UI-BG3-01 — test_pergamino: el combate en pergamino (maquetas C03a-C03c),
// medido sin ventana sobre la mesa real del Bardo contra un lobo. Que cada
// carta de la barra deja su zona para el raton y no pisa a su vecina, que el
// hueco de creador, «F Pasar» y el registro responden, que el emergente de la
// estacion ofrece sus cuatro opciones mas «Elegir» y «Ahora no», que todo
// cabe en 1280x720 y que en una vista mayor baja y se centra entero.
#include <string>
#include <vector>

#include "Game/Acompanante.h"
#include "Game/BarraCombate.h"
#include "Game/CartasPrueba.h"
#include "HUD/Iconos.h"
#include "HUD/Pergamino/AtlasRetratos.h"
#include "HUD/Pergamino/FuenteAtlas.h"
#include "HUD/Pergamino/MesaCombateHud.h"
#include "HUD/Pergamino/Pincel.h"
#include "TestHarness.h"

using namespace Pergamino;

namespace {

using Z = Game::BarraCombate::Zona;

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

Lienzo pinta(const Game::VistaBarra& v, const Game::BarraCombate& b, int ancho = 1280, int alto = 720,
             int enemigos = 1) {
    DatosCombate d;
    d.vista = &v;
    d.barra = &b;
    d.heroe = "Nerea";
    d.enemigos.push_back(EnemigoEnMesa{"Perdido saqueador", "", 22, 22, true});
    if (enemigos > 1) d.enemigos.push_back(EnemigoEnMesa{"Lobo gris", "", 60, 60, true});
    for (int i = 2; i < enemigos; ++i)  // un grupo: devotos
        d.enemigos.push_back(EnemigoEnMesa{"Devoto de los Perdidos " + std::to_string(i), "", 9, 9, true});
    d.registro = {"Nerea elige Invierno.", "Nerea juega Canción del Otoño."};
    Lienzo l;
    AtlasRetratos atlas;
    Retratista retratos(atlas, nullptr);
    const Pincel p(tinta().letras, tinta().iconos);
    pintaCombate(l, p, d, retratos, ancho, alto);
    return l;
}

std::vector<const Zona*> zonas(const Lienzo& l, Z tipo) {
    std::vector<const Zona*> out;
    for (const Zona& z : l.zonas())
        if (z.tipo == static_cast<int>(tipo)) out.push_back(&z);
    return out;
}

bool dentro(const Lienzo& l, float ancho, float alto) {
    for (const Trazo& t : l.trazos())
        for (const Vector2& v : t.p)
            if (v.x < -1.0f || v.y < -1.0f || v.x > ancho + 1.0f || v.y > alto + 1.0f) return false;
    return true;
}

}  // namespace

MOTOR_TEST_CASE(combate_cada_carta_de_la_barra_tiene_su_zona) {
    cartas_prueba::Mesa m("bardo_de_las_cuatro_estaciones");
    const Game::VistaBarra v = Game::vistaDeBatalla(*m.batalla);
    Game::BarraCombate b;
    b.mira(v, 0.0f, m.batalla.get());
    b.cancela();  // sin el emergente de la estacion
    const Lienzo l = pinta(v, b);
    const auto cartas = zonas(l, Z::Carta);
    MOTOR_CHECK(cartas.size() == 10);  // Atacar, Pocion, Huir y las siete de la mano
    for (std::size_t i = 0; i < cartas.size(); ++i) {
        MOTOR_CHECK(cartas[i]->indice == static_cast<int>(i));
        MOTOR_CHECK(cartas[i]->rect.y > 540.0f && cartas[i]->rect.abajo() < 712.0f);  // en la bandeja
        for (std::size_t j = i + 1; j < cartas.size(); ++j) {
            const Rect& a = cartas[i]->rect;
            const Rect& c = cartas[j]->rect;
            MOTOR_CHECK(!(a.x < c.derecha() && c.x < a.derecha()));  // no se pisan
        }
    }
    MOTOR_CHECK(zonas(l, Z::Creador).size() == 1);
    MOTOR_CHECK(zonas(l, Z::Pasar).size() == 1);
    MOTOR_CHECK(zonas(l, Z::Registro).size() == 1);
    MOTOR_CHECK(zonas(l, Z::Opcion).empty());
    MOTOR_CHECK(dentro(l, 1280.0f, 720.0f));
    // Lo que hay bajo el raton en el centro de la tercera carta es Huir.
    const Zona* z = l.zonaEn(cartas[2]->rect.x + 30.0f, cartas[2]->rect.y + 40.0f);
    MOTOR_CHECK(z != nullptr && z->tipo == static_cast<int>(Z::Carta) && z->indice == 2);
}

MOTOR_TEST_CASE(combate_el_emergente_de_la_estacion_tapa_la_barra) {
    cartas_prueba::Mesa m("bardo_de_las_cuatro_estaciones");
    const Game::VistaBarra v = Game::vistaDeBatalla(*m.batalla);
    Game::BarraCombate b;
    b.mira(v, 0.0f, m.batalla.get());
    MOTOR_CHECK(b.emergente().abierto);
    const Lienzo l = pinta(v, b);
    // Cuatro tarjetas y el boton «Elegir ...» (que juega la del foco).
    const auto opciones = zonas(l, Z::Opcion);
    MOTOR_CHECK(opciones.size() == 5);
    MOTOR_CHECK(zonas(l, Z::AhoraNo).size() == 1);
    MOTOR_CHECK(dentro(l, 1280.0f, 720.0f));
    // El emergente va encima: en una carta de la bandeja que queda debajo,
    // el raton encuentra la ultima zona pintada, la del emergente o nada de
    // la barra si cae fuera del emergente.
    const Zona* z = l.zonaEn(opciones[3]->rect.x + 20.0f, opciones[3]->rect.y + 20.0f);
    MOTOR_CHECK(z != nullptr && z->tipo == static_cast<int>(Z::Opcion) && z->indice == 3);
}

MOTOR_TEST_CASE(combate_registro_oculto_y_turno_del_rival) {
    cartas_prueba::Mesa m("bardo_de_las_cuatro_estaciones");
    const Game::VistaBarra v = Game::vistaDeBatalla(*m.batalla);
    Game::BarraCombate b;
    b.mira(v, 0.0f, m.batalla.get());
    b.cancela();
    const std::size_t conRegistro = pinta(v, b).cuantos();
    b.alternaRegistro();
    const Lienzo oculto = pinta(v, b);
    MOTOR_CHECK(oculto.cuantos() < conRegistro);
    const auto retal = zonas(oculto, Z::Registro);
    MOTOR_CHECK(retal.size() == 1 && retal[0]->rect.w < 200.0f);  // el retal pequeno «L Registro»
    // El rotulo del turno del rival suma su retal.
    b.esperaRival("Perdido saqueador");
    MOTOR_CHECK(pinta(v, b).cuantos() > oculto.cuantos());
}

MOTOR_TEST_CASE(combate_en_una_vista_mayor_baja_y_se_centra) {
    cartas_prueba::Mesa m("bardo_de_las_cuatro_estaciones");
    const Game::VistaBarra v = Game::vistaDeBatalla(*m.batalla);
    Game::BarraCombate b;
    b.mira(v, 0.0f, m.batalla.get());
    b.cancela();
    const Lienzo base = pinta(v, b);
    const Lienzo grande = pinta(v, b, 1600, 900);
    MOTOR_CHECK(dentro(grande, 1600.0f, 900.0f));
    const Zona* a = zonas(base, Z::Pasar).front();
    const Zona* g = zonas(grande, Z::Pasar).front();
    MOTOR_CHECK(g->rect.x == a->rect.x + 160.0f && g->rect.y == a->rect.y + 180.0f);
}

// H2-03 — Los estados: en la placa, cada uno en su ficha (icono y rondas) con
// el nombre al lado; con tres o mas, solo las fichas; los del heroe bajo su
// retrato, y el emergente con lo que hace al senalar uno.
namespace {

Game::EstadoVisible estado(const std::string& id, int rondas, Game::EstadoVisible::Signo signo) {
    Game::EstadoVisible e;
    e.id = id;
    e.icono = id;
    e.nombre = id;
    e.descripcion = "Lo que hace " + id + ", en una o dos lineas de texto para el emergente.";
    e.rondas = rondas;
    e.signo = signo;
    return e;
}

bool dentroDe(const Rect& a, const Rect& b) {
    return a.x >= b.x && a.y >= b.y && a.derecha() <= b.derecha() && a.abajo() <= b.abajo();
}

}  // namespace

MOTOR_TEST_CASE(combate_los_estados_del_enemigo_van_en_su_placa) {
    cartas_prueba::Mesa m("bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(m.jugar("Elegir Invierno"));
    MOTOR_CHECK(m.jugar("Canción del Otoño"));  // desventaja dos rondas al lobo
    const Game::VistaBarra v = Game::vistaDeBatalla(*m.batalla);
    Game::BarraCombate b;
    b.mira(v, 0.0f, m.batalla.get());
    const Lienzo l = pinta(v, b);
    const auto fichas = zonas(l, Z::Estado);
    MOTOR_CHECK(fichas.size() == 1);
    MOTOR_CHECK(fichas[0]->indice == Game::BarraCombate::kEstadoPorQuien);  // el primero del enemigo 0
    // En la placa (x 688..938, desde y = 186), debajo de su vida.
    MOTOR_CHECK(dentroDe(fichas[0]->rect, Rect{688.0f, 186.0f, 250.0f, 300.0f}) && fichas[0]->rect.y > 186.0f + 76.0f);
    MOTOR_CHECK(fichas[0]->rect.w == 24.0f);
    MOTOR_CHECK(dentro(l, 1280.0f, 720.0f));

    // Tres o mas: solo las fichas, en fila; las que no caben, «+N».
    Game::VistaBarra muchas = v;
    muchas.estadosEnemigos[0].clear();
    for (const char* id : {"envenenado", "sangrado", "marcado", "cegado", "asustado", "caido", "fatiga", "anclado", "caido"})
        muchas.estadosEnemigos[0].push_back(estado(id, 2, Game::EstadoVisible::Signo::Contra));
    const Lienzo f = pinta(muchas, b);
    const auto fila = zonas(f, Z::Estado);
    MOTOR_CHECK(fila.size() >= 3 && fila.size() < 9);
    for (const Zona* z : fila) MOTOR_CHECK(z->rect.y == fila[0]->rect.y && z->rect.derecha() <= 938.0f);
}

MOTOR_TEST_CASE(combate_los_estados_del_heroe_van_bajo_su_retrato) {
    cartas_prueba::Mesa m("senor_de_las_armas", 2);
    MOTOR_CHECK(m.jugar("Moral Inconmensurable"));  // +1 CA todo el combate
    const Game::VistaBarra v = Game::vistaDeBatalla(*m.batalla);
    MOTOR_CHECK(v.estadosHeroe.size() == 1);
    Game::BarraCombate b;
    b.mira(v, 0.0f, m.batalla.get());
    const Lienzo l = pinta(v, b);
    const auto fichas = zonas(l, Z::Estado);
    MOTOR_CHECK(fichas.size() == 1 && fichas[0]->indice == 0 && fichas[0]->rect.w == 20.0f);
    // Bajo el retrato del orden de turno (y 28..98), dentro de su retal, que
    // crece hasta y = 138 sin llegar al registro ni a la carta grande (148).
    MOTOR_CHECK(fichas[0]->rect.y > 98.0f && fichas[0]->rect.abajo() < 138.0f);
    // Tres caben; con cinco, dos y «+3».
    Game::VistaBarra cinco = v;
    for (int i = 0; i < 4; ++i) cinco.estadosHeroe.push_back(estado("mejora", i + 1, Game::EstadoVisible::Signo::AFavor));
    MOTOR_CHECK(zonas(pinta(cinco, b), Z::Estado).size() == 2);
}

MOTOR_TEST_CASE(combate_al_senalar_un_estado_sale_lo_que_hace) {
    cartas_prueba::Mesa m("bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(m.jugar("Elegir Invierno"));
    MOTOR_CHECK(m.jugar("Canción del Otoño"));
    const Game::VistaBarra v = Game::vistaDeBatalla(*m.batalla);
    Game::BarraCombate b;
    b.mira(v, 0.0f, m.batalla.get());
    const std::size_t sin = pinta(v, b).cuantos();
    b.senala(v, Z::Estado, Game::BarraCombate::kEstadoPorQuien);
    const Lienzo con = pinta(v, b);
    MOTOR_CHECK(con.cuantos() > sin + 4);  // la hoja, la marca, el icono y los textos
    MOTOR_CHECK(dentro(con, 1280.0f, 720.0f));
    // El raton se va: el emergente tambien.
    b.senalaNada();
    MOTOR_CHECK(pinta(v, b).cuantos() == sin);
}

// H2-02 — A quien apunta la carta senalada: una reticula carmin en su placa y
// en su retrato; las placas y los retratos se senalan con el raton.
namespace {

// Trazos carmin en la esquina de arriba a la izquierda de una placa (x 684
// a 690, la reticula sale 4 px por fuera) entre y0 e y1.
int flechas(const Lienzo& l, float y0, float y1) {
    int n = 0;
    for (const Trazo& t : l.trazos())
        if (t.color.x == Color::kCarmin.x && t.color.y == Color::kCarmin.y && t.color.z == Color::kCarmin.z &&
            t.p[0].x >= 683.0f && t.p[0].x < 690.0f && t.p[0].y >= y0 && t.p[0].y < y1)
            ++n;
    return n;
}

}  // namespace

MOTOR_TEST_CASE(combate_la_carta_senalada_marca_a_quien_alcanza) {
    cartas_prueba::Mesa m("bardo_de_las_cuatro_estaciones", 1, 4, 2);
    MOTOR_CHECK(m.jugar("Elegir Invierno"));
    const Game::VistaBarra v = Game::vistaDeBatalla(*m.batalla);
    Game::BarraCombate b;
    b.mira(v, 0.0f, m.batalla.get());
    // Atacar, sin elegir: el primero. Placas en y 186 y 288 (92 de alto + 10).
    Lienzo l = pinta(v, b, 1280, 720, 2);
    MOTOR_CHECK(flechas(l, 180.0f, 278.0f) > 0 && flechas(l, 282.0f, 380.0f) == 0);
    // Retratos (heroe y dos) y placas (dos) se senalan con el raton.
    const auto objetivos = zonas(l, Z::Objetivo);
    MOTOR_CHECK(objetivos.size() == 5);
    const Zona* placa = l.zonaEn(800.0f, 300.0f);  // en la placa del lobo gris
    MOTOR_CHECK(placa != nullptr && placa->tipo == static_cast<int>(Z::Objetivo) &&
                placa->indice == Game::BarraCombate::kObjetivoEnemigo + 1);
    // Elegido el otro, la marca se va con el.
    b.mueveObjetivo(v, +1);
    l = pinta(v, b, 1280, 720, 2);
    MOTOR_CHECK(flechas(l, 180.0f, 278.0f) == 0 && flechas(l, 282.0f, 380.0f) > 0);
    // Una cancion de area marca a los dos.
    MOTOR_CHECK(b.tecla(v, 3).tipo == Game::Jugada::Tipo::Nada);  // tecla 4: Cancion del Otono
    l = pinta(v, b, 1280, 720, 2);
    MOTOR_CHECK(flechas(l, 180.0f, 278.0f) > 0 && flechas(l, 282.0f, 380.0f) > 0);
    MOTOR_CHECK(dentro(l, 1280.0f, 720.0f));
}

// Oriol (05/10): combates en grupo. Con cinco enemigos las placas enteras no
// caben encima del rotulo del turno (y = 452): van compactas, todas a la
// vista y cada una con su zona para apuntarla.
MOTOR_TEST_CASE(combate_un_grupo_de_cinco_lleva_las_placas_compactas) {
    const Game::VistaBarra v = Game::vistaBasica(1);
    const Game::BarraCombate b;
    const Lienzo dos = pinta(v, b, 1280, 720, 2);
    const Lienzo cinco = pinta(v, b, 1280, 720, 5);
    auto placas = [](const Lienzo& l) {
        std::vector<const Zona*> out;
        for (const Zona& z : l.zonas())
            if (z.tipo == static_cast<int>(Z::Objetivo) && z.indice >= Game::BarraCombate::kObjetivoEnemigo &&
                z.rect.x >= 680.0f && z.rect.w >= 240.0f)
                out.push_back(&z);
        return out;
    };
    MOTOR_CHECK(placas(dos).size() == 2);
    const std::vector<const Zona*> p5 = placas(cinco);
    MOTOR_CHECK(p5.size() == 5);
    for (const Zona* z : p5) MOTOR_CHECK(z->rect.y >= 110.0f && z->rect.y + z->rect.h <= 446.0f);
    MOTOR_CHECK(!placas(dos).empty() && placas(dos)[0]->rect.y == 186.0f);  // con dos, como siempre
}

// ACOMPANANTES-01 — Venides y Xila van en el orden de turno detras del
// heroe, cada uno con su zona para apuntarle una carta de aliado; debajo,
// la vida de Venides en una barra y las curas que le quedan a Xila en
// marcas verdes. Los datos salen de la batalla (aliadosEnMesa).
MOTOR_TEST_CASE(combate_los_acompanantes_van_en_el_orden_de_turno) {
    cartas_prueba::Mesa m("bardo_de_las_cuatro_estaciones");
    RPG::AcompananteDef xila;
    xila.id = "xila";
    xila.nombre = "Xila";
    xila.vida = 6;
    xila.atacable = false;
    xila.cura = {4, 1, 2, 0.34f};
    auto mesa = Game::CartasEnCombate::paraHeroe(cartas_prueba::catalogo(), m.heroe);
    RPG::AcompananteDef venides;
    venides.id = "venides";
    venides.nombre = "Venides";
    venides.vida = 16;
    std::vector<BattleParticipant> aliados{{"Nerea", &m.heroe, nullptr, nullptr}};
    aliados.push_back({"Venides", mesa->sumaAcompanante(venides, 0), nullptr, nullptr});
    aliados.push_back({"Xila", mesa->sumaAcompanante(xila, 2), nullptr, nullptr});
    aliados[1].combatant->takeDamage(4);
    SkillCatalog legado;
    BattleState batalla(std::move(aliados), {{"Lobo", &m.lobo, nullptr, nullptr}}, &legado);
    batalla.setMesa(std::move(mesa));
    const std::vector<AliadoEnMesa> enMesa = aliadosEnMesa(batalla);
    MOTOR_CHECK(enMesa.size() == 2);
    if (enMesa.size() != 2) return;
    MOTOR_CHECK(enMesa[0].nombre == "Venides" && enMesa[0].objectId == "venides" && enMesa[0].pv == 12 &&
                enMesa[0].pvMax == 16 && enMesa[0].curas == -1 && enMesa[0].vivo);
    MOTOR_CHECK(enMesa[1].nombre == "Xila" && enMesa[1].curas == 2);

    const Game::VistaBarra v = Game::vistaDeBatalla(batalla);
    Game::BarraCombate b;
    b.mira(v, 0.0f, &batalla);
    b.cancela();
    DatosCombate d;
    d.vista = &v;
    d.barra = &b;
    d.heroe = "Nerea";
    d.aliados = enMesa;
    d.enemigos.push_back(EnemigoEnMesa{"Lobo", "", 60, 60, true});
    Lienzo l;
    AtlasRetratos atlas;
    Retratista retratos(atlas, nullptr);
    const Pincel p(tinta().letras, tinta().iconos);
    pintaCombate(l, p, d, retratos, 1280, 720);
    // En el orden de turno (arriba): el heroe (0), Venides (1), Xila (2) y el
    // lobo, cada caja 74 px a la derecha de la anterior.
    std::vector<const Zona*> arriba;
    for (const Zona& z : l.zonas())
        if (z.tipo == static_cast<int>(Z::Objetivo) && z.rect.y < 60.0f) arriba.push_back(&z);
    MOTOR_CHECK(arriba.size() == 4);
    if (arriba.size() == 4) {
        MOTOR_CHECK(arriba[0]->indice == 0 && arriba[1]->indice == 1 && arriba[2]->indice == 2 &&
                    arriba[3]->indice == Game::BarraCombate::kObjetivoEnemigo);
        MOTOR_CHECK(arriba[1]->rect.x - arriba[0]->rect.x == 74.0f && arriba[2]->rect.x - arriba[1]->rect.x == 74.0f);
    }
    // Las dos curas de Xila: dos marcas verdes de 8x8 debajo de su caja.
    int marcas = 0;
    for (const Trazo& t : l.trazos())
        if (t.p[0].y > 100.0f && t.p[0].y < 120.0f && t.color.y > t.color.x &&
            arriba.size() == 4 && t.p[0].x >= arriba[2]->rect.x && t.p[0].x < arriba[2]->rect.x + 64.0f)
            ++marcas;
    MOTOR_CHECK(marcas >= 2);
    MOTOR_CHECK(dentro(l, 1280.0f, 720.0f));
}
