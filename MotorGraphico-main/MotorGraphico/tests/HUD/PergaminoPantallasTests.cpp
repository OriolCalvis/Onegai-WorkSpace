// PERGAMINO-01 — test_pergamino (2/2): las pantallas del tema, medidas.
//
// Lo que fallaba en el HUD viejo y aqui se comprueba midiendo, sin ventana:
// que el papiro de pausa tenga las medidas de P05a y quepa en la vista, que
// haya un solo foco (una cinta), que la hoja que cae no pase de las tapas,
// que el dialogo pagine y vuelva a la primera pagina, que el aviso de nuevo
// objetivo se vaya solo y que la interfaz oculta no pinte nada.
#include <algorithm>
#include <string>
#include <vector>

#include "HUD/Iconos.h"
#include "HUD/Pergamino/Cuaderno.h"
#include "HUD/Pergamino/DialogoHoja.h"
#include "HUD/Pergamino/FuenteAtlas.h"
#include "HUD/Pergamino/HudRetales.h"
#include "HUD/Pergamino/InterfazPergamino.h"
#include "HUD/Pergamino/PaginaCuaderno.h"
#include "HUD/Pergamino/Papiro.h"
#include "HUD/Pergamino/Pincel.h"
#include "TestHarness.h"

using namespace Pergamino;

namespace {

struct Recursos {
    FuenteAtlas letras;
    Hud::Iconos iconos;
    Recursos() {
        MOTOR_CHECK(letras.carga("assets/textures/letras_pergamino.json").isOk());
        MOTOR_CHECK(iconos.cargar("assets/textures/iconos.json").isOk());
    }
};

const Recursos& recursos() {
    static const Recursos r;
    return r;
}

Pincel pincel() { return Pincel(recursos().letras, recursos().iconos); }

// Los trazos de un material concreto (la cinta del foco, una hoja...).
std::vector<Trazo> trazosDe(const Lienzo& l, Material tipo) {
    std::vector<Trazo> salida;
    for (const Trazo& t : l.trazos())
        if (t.fuente == Fuente::Material && t.material >= 0 &&
            l.materiales()[static_cast<std::size_t>(t.material)].tipo == tipo)
            salida.push_back(t);
    return salida;
}

int hojasPedidas(const Lienzo& l) {
    int n = 0;
    for (const PedidoMaterial& m : l.materiales()) n += m.tipo == Material::Hoja ? 1 : 0;
    return n;
}

DatosPapiro pausaP05a() {
    DatosPapiro d;
    d.titulo = "EN PAUSA";
    for (const char* t : {"Reanudar", "Personaje", "Guardar partida", "Cargar partida", "Opciones"})
        d.entradas.push_back({t});
    d.entradas.push_back({"Volver al título", true});
    d.entradas.push_back({"Salir del juego"});
    d.foco = 2;
    d.estado = "Partida guardada";
    d.estadoDetalle = "Centro de Boundington · 17:42";
    d.pie = {{{"ENTER"}, "Aceptar"}, {{"ESC"}, "Reanudar"}};
    return d;
}

}  // namespace

// --- Papiro -----------------------------------------------------------------

MOTOR_TEST_CASE(papiro_mide_lo_de_p05a) {
    // P05a: siete entradas en dos grupos = papiro de 480 x 640.
    MOTOR_CHECK(altoPapiro(pausaP05a()) == 640.0f);
    DatosPapiro corto;
    corto.entradas = {{"Uno"}};
    MOTOR_CHECK(altoPapiro(corto) == 400.0f);  // el minimo
}

MOTOR_TEST_CASE(papiro_un_solo_foco_en_su_fila) {
    Lienzo l;
    const Pincel p = pincel();
    const DatosPapiro d = pausaP05a();
    pintaPapiro(l, p, d, 1280, 720);
    const auto cintas = trazosDe(l, Material::Cinta);
    MOTOR_CHECK(cintas.size() == 1);
    // Fila 2 de P05a: de y=235 a 279 (papiro en y=40).
    MOTOR_CHECK(cintas[0].p[0].y >= 235.0f && cintas[0].p[2].y <= 279.0f);
    // El guardado lleva su sello (y un error no lo llevaria).
    MOTOR_CHECK(trazosDe(l, Material::Sello).size() == 1);
}

MOTOR_TEST_CASE(papiro_de_pausa_del_juego_cabe_en_720) {
    // El juego tiene una entrada mas que la maqueta (Inventario).
    DatosPapiro d = pausaP05a();
    d.entradas.insert(d.entradas.begin() + 2, EntradaPapiro{"Inventario"});
    for (int alto : {720, 600}) {
        Lienzo l;
        pintaPapiro(l, pincel(), d, 1280, alto);
        for (const Trazo& t : l.trazos())
            for (const Vector2& q : t.p) MOTOR_CHECK(q.y >= 0.0f && q.y <= static_cast<float>(alto));
    }
}

MOTOR_TEST_CASE(papiro_de_opciones_mas_ancho_para_su_pie) {
    DatosPapiro d;
    d.titulo = "OPCIONES";
    d.ancho = 560.0f;
    d.entradas = {{"Música", false, "", 7}, {"Idioma", false, "Español"}, {"Volver", true}};
    d.pie = {{{"←", "→"}, "Ajustar"}, {{"ENTER"}, "Aceptar"}, {{"ESC"}, "Volver"}};
    Lienzo l;
    const Pincel p = pincel();
    pintaPapiro(l, p, d, 1280, 720);
    float anchoPie = 0.0f;
    for (const AyudaPapiro& a : d.pie) anchoPie += p.anchoAyuda(a.teclas, a.verbo);
    MOTOR_CHECK(anchoPie + 44.0f < d.ancho - 2.0f * 44.0f);
    // Nada se sale del papiro por los lados.
    const float x0 = (1280.0f - d.ancho) * 0.5f;
    for (const Trazo& t : l.trazos()) {
        if (t.fuente != Fuente::Letras) continue;
        for (const Vector2& q : t.p) MOTOR_CHECK(q.x >= x0 && q.x <= x0 + d.ancho);
    }
}

// --- Cuaderno ---------------------------------------------------------------

MOTOR_TEST_CASE(cuaderno_centrado_y_hoja_dentro_de_las_tapas) {
    const Rect t = Cuaderno::tapas(1280, 720);
    const Rect h = Cuaderno::hoja(1280, 720);
    MOTOR_CHECK(t.x == 80.0f && t.y == 60.0f && t.w == 1120.0f && t.h == 600.0f);
    MOTOR_CHECK(h.x == t.x + 32.0f && h.y == t.y + 96.0f && h.w == 1056.0f && h.h == 446.0f);
    MOTOR_CHECK(h.abajo() < t.abajo());
}

MOTOR_TEST_CASE(cuaderno_cada_cambio_de_hoja_es_una_vuelta) {
    Cuaderno c;
    c.abre(2);
    MOTOR_CHECK(!c.vuelta().enCurso() && c.vueltas() == 0);
    c.mira(2, false);
    MOTOR_CHECK(c.vueltas() == 0);
    c.mira(3, false);
    MOTOR_CHECK(c.vuelta().enCurso() && c.vuelta().haciaDelante() && c.vueltas() == 1);
    c.avanza(0.4f);
    MOTOR_CHECK(!c.vuelta().enCurso());
    c.mira(1, false);
    MOTOR_CHECK(c.vuelta().enCurso() && !c.vuelta().haciaDelante() && c.vueltas() == 2);
}

MOTOR_TEST_CASE(cuaderno_la_hoja_que_cae_no_pasa_de_las_tapas) {
    const Pincel p = pincel();
    PaginaCuaderno pagina;
    pagina.titulo = "FICHA";
    const Rect t = Cuaderno::tapas(1280, 720);
    for (float segundos : {0.10f, 0.20f, 0.28f, 0.33f}) {
        Cuaderno c;
        c.abre(1);
        c.mira(2, false);
        c.avanza(segundos);
        Lienzo l;
        c.pintaMarco(l, p, CabeceraCuaderno{"Nerea", 1, "Aarakocra · Acechador", 28, 28, 150}, pagina, 1280,
                     720);
        for (const Trazo& tr : l.trazos()) {
            if (tr.fuente != Fuente::HojaFoto && tr.fuente != Fuente::HojaViva && tr.fuente != Fuente::Material)
                continue;
            for (const Vector2& q : tr.p) MOTOR_CHECK(q.y <= t.abajo() + 4.01f);  // + la sombra de 4
        }
    }
}

MOTOR_TEST_CASE(pagina_lista_larga_sigue_al_foco) {
    const Pincel p = pincel();
    PaginaCuaderno pagina;
    pagina.titulo = "CÓDICE";
    pagina.cabeceraIzq = "DESCUBIERTO";
    for (int i = 0; i < 14; ++i) pagina.filas.push_back(FilaPagina{"lore", 1, "Entrada " + std::to_string(i)});
    pagina.foco = 13;
    pagina.detalle.titulo = "Entrada 13";
    Lienzo l;
    pintaPagina(l, p, pagina, Cuaderno::kHojaAncho, Cuaderno::kHojaAlto, 6, 6);
    const auto cintas = trazosDe(l, Material::Cinta);
    MOTOR_CHECK(cintas.size() == 1);
    MOTOR_CHECK(cintas[0].p[2].y <= Cuaderno::kHojaAlto - 30.0f);  // la fila con foco se ve entera
    for (const Trazo& tr : l.trazos())
        for (const Vector2& q : tr.p) MOTOR_CHECK(q.y <= Cuaderno::kHojaAlto && q.x <= Cuaderno::kHojaAncho);
}

MOTOR_TEST_CASE(pagina_vacia_sin_cinta) {
    const Pincel p = pincel();
    PaginaCuaderno pagina;
    pagina.tipo = PaginaCuaderno::Tipo::Vacia;
    pagina.titulo = "ACCIONES";
    pagina.vacioIcono = "dote";
    pagina.vacioTitulo = "No tienes acciones disponibles fuera de combate.";
    Lienzo l;
    pintaPagina(l, p, pagina, Cuaderno::kHojaAncho, Cuaderno::kHojaAlto, 4, 6);
    MOTOR_CHECK(trazosDe(l, Material::Cinta).empty());
    MOTOR_CHECK(!l.vacio());
}

// --- Dialogo ----------------------------------------------------------------

MOTOR_TEST_CASE(dialogo_una_linea_por_pagina_y_la_ultima_cierra) {
    DialogoHoja d;
    const std::vector<std::string> lineas = {"Antes esta plaza era un huerto.", "Los jóvenes ya no saludan."};
    d.mira("Anciana", lineas, recursos().letras);
    MOTOR_CHECK(d.paginas() == 2 && d.pagina() == 0 && !d.ultima());
    MOTOR_CHECK(d.avanza());
    MOTOR_CHECK(d.ultima());
    MOTOR_CHECK(!d.avanza());  // en la ultima ENTER cierra: lo hace quien llama
    // El mismo dialogo el fotograma siguiente no vuelve atras...
    d.mira("Anciana", lineas, recursos().letras);
    MOTOR_CHECK(d.pagina() == 1);
    // ...pero cerrado y abierto otra vez empieza por la primera.
    d.cierra();
    d.mira("Anciana", lineas, recursos().letras);
    MOTOR_CHECK(d.pagina() == 0);
    // Una linea que no cabe en dos renglones sigue en otra pagina.
    const std::string larga(60, 'x');
    d.mira("", {larga + " " + larga + " " + larga + " " + larga}, recursos().letras);
    MOTOR_CHECK(d.paginas() >= 2);
}

// DIALOGO-02 — Cada linea bajo quien la dice (Oriol: «que ponga arriba narración
// en vez del nombre y en el texto aparezca Sho-Noco: blabla, bastante cutre»).
MOTOR_TEST_CASE(dialogo_cada_linea_bajo_quien_la_dice) {
    using V = std::pair<std::string, std::string>;
    MOTOR_CHECK(DialogoHoja::vozDe("Sho-Noco: «Sube.»", "") == V("Sho-Noco", "Sube."));
    MOTOR_CHECK(DialogoHoja::vozDe("Rosita, apagando las velas: «Dormid.»", "") ==
                V("Rosita", "(apagando las velas) Dormid."));
    // La narracion se queda como esta; sin prefijo, del hablante del beat.
    MOTOR_CHECK(DialogoHoja::vozDe("El río no tiene orilla.", "") == V("", "El río no tiene orilla."));
    MOTOR_CHECK(DialogoHoja::vozDe("Mira lo que quieras.", "Tendero") == V("Tendero", "Mira lo que quieras."));
    // Un sitio o una frase antes de la cita no es nadie.
    MOTOR_CHECK(DialogoHoja::vozDe("En el límite de Pico Dragón, el letrero: «Vestuario».", "").first.empty());
    MOTOR_CHECK(DialogoHoja::vozDe("Está dormido. El tuerto te mira: «Ese no despierta.»", "").first.empty());
    // Cada pagina con su voz; el rotulo de la que se ve.
    DialogoHoja d;
    d.mira("", {"La barca se para.", "Sho-Noco: «Sube.»"}, recursos().letras);
    MOTOR_CHECK(d.paginas() == 2 && d.quienHabla().empty());
    d.avanza();
    MOTOR_CHECK(d.quienHabla() == "Sho-Noco" && d.renglones().size() == 1 && d.renglones()[0] == "Sube.");
}

// TUTORIAL-01 — Una linea larga se parte por frases: ninguna pagina deja una frase a
// medias (el prologo cortaba «...de un cadaver. Se acerca» / «sobre una barca...»).
MOTOR_TEST_CASE(dialogo_una_linea_larga_se_parte_por_frases) {
    DialogoHoja d;
    d.mira("", {"Es una figura envuelta en un manto blanco, con los brazos al aire, y esos brazos cambian: un "
                "momento parecen de un joven, al siguiente de un cadaver. Se acerca sobre una barca que flota en "
                "una corriente que no puedes ver."},
           recursos().letras);
    MOTOR_CHECK(d.paginas() == 2);
    for (int i = 0; i < d.paginas(); ++i) {
        const std::vector<std::string> r = d.renglones();
        MOTOR_CHECK(!r.empty() && r.size() <= 2 && !r.back().empty() && r.back().back() == '.');
        d.avanza();
    }
    // Una frase que sola no cabe sigue por renglones, como antes.
    const std::string larga(60, 'x');
    d.mira("", {"Corta. " + larga + " " + larga + " " + larga + " " + larga + ". Fin."}, recursos().letras);
    MOTOR_CHECK(d.paginas() >= 3);
}

MOTOR_TEST_CASE(dialogo_la_hoja_va_abajo_y_centrada) {
    DialogoHoja d;
    d.mira("Nerea", {"Antes esta plaza era un huerto."}, recursos().letras);
    Lienzo l;
    d.pinta(l, pincel(), 1280, 720);
    const auto hojas = trazosDe(l, Material::Hoja);
    MOTOR_CHECK(!hojas.empty());
    const Trazo& hoja = hojas.back();
    MOTOR_CHECK(hoja.p[0].x == 180.0f && hoja.p[2].y == 720.0f - 28.0f);
}

// --- HUD de exploracion -----------------------------------------------------

MOTOR_TEST_CASE(retales_el_aviso_de_objetivo_se_va_solo) {
    HudRetales r;
    r.observaObjetivo("Presentarte a los tres comerciantes");
    MOTOR_CHECK(r.avisoObjetivoVisible());
    r.avanza(4.0f);
    MOTOR_CHECK(r.avisoObjetivoVisible());
    r.avanza(1.1f);
    MOTOR_CHECK(!r.avisoObjetivoVisible());
    r.observaObjetivo("Presentarte a los tres comerciantes");  // el mismo: no se repite
    MOTOR_CHECK(!r.avisoObjetivoVisible());
    r.observaObjetivo("");  // sin objetivo: nada que avisar
    MOTOR_CHECK(!r.avisoObjetivoVisible());
}

MOTOR_TEST_CASE(retales_compacto_solo_el_personaje) {
    DatosExploracion d;
    d.nombre = "Nerea";
    d.pv = 7;
    d.pvMax = 28;
    d.mision = "El primer día";
    d.paso = "Presentarte";
    d.total = 3;
    Lienzo completo, compacto;
    HudRetales r;
    r.pinta(completo, pincel(), d, 1280, 720);
    d.compacto = true;
    r.pinta(compacto, pincel(), d, 1280, 720);
    // Un solo retal (su hoja y su sombra) frente a personaje, objetivo y ayudas.
    MOTOR_CHECK(hojasPedidas(compacto) == 1);
    MOTOR_CHECK(hojasPedidas(completo) >= 3);
    // Sin oro en el compacto (P03b): menos iconos (queda el corazon de la vida).
    auto iconos = [](const Lienzo& l) {
        int n = 0;
        for (const Trazo& t : l.trazos()) n += t.fuente == Fuente::Iconos ? 1 : 0;
        return n;
    };
    MOTOR_CHECK(iconos(compacto) >= 1 && iconos(compacto) < iconos(completo));
}

// H3-06 — Los temporizadores de mision: en el retal del objetivo, que crece
// para ellos, con la cuenta y la barra; sin objetivo, en un retal suyo; lo
// que queda en rojo el ultimo cuarto.
MOTOR_TEST_CASE(retales_el_temporizador_va_con_el_objetivo) {
    DatosExploracion d;
    d.nombre = "Nerea";
    d.pv = 7;
    d.pvMax = 28;
    d.mision = "El primer día";
    d.paso = "Salir de la torre";
    Lienzo sinReloj, conReloj, soloReloj, apurado;
    HudRetales r;
    r.pinta(sinReloj, pincel(), d, 1280, 720);
    Game::RelojVisible huida;
    huida.id = "huida";
    huida.restante = 90.0f;
    huida.total = 120.0f;
    huida.barra = true;
    d.relojes = {huida};
    r.pinta(conReloj, pincel(), d, 1280, 720);
    // El mismo numero de retales (el reloj va dentro del objetivo), mas trazos.
    MOTOR_CHECK(hojasPedidas(conReloj) == hojasPedidas(sinReloj));
    MOTOR_CHECK(conReloj.cuantos() > sinReloj.cuantos() + 4);
    auto rojos = [](const Lienzo& l) {
        int n = 0;
        for (const Trazo& t : l.trazos())
            n += t.color.x == Color::kError.x && t.color.y == Color::kError.y && t.color.z == Color::kError.z;
        return n;
    };
    MOTOR_CHECK(rojos(conReloj) == 0);
    // Con nombre («El incendio») va el nombre en lugar de «TIEMPO»: mas letras.
    Lienzo conNombre;
    d.relojes[0].nombre = "El incendio";
    r.pinta(conNombre, pincel(), d, 1280, 720);
    MOTOR_CHECK(conNombre.cuantos() > conReloj.cuantos());
    MOTOR_CHECK(hojasPedidas(conNombre) == hojasPedidas(conReloj));
    d.relojes[0].nombre.clear();
    d.relojes[0].restante = 20.0f;  // el ultimo cuarto
    r.pinta(apurado, pincel(), d, 1280, 720);
    MOTOR_CHECK(rojos(apurado) > 0);
    // Sin objetivo seguido, el reloj tiene su retal.
    d.paso.clear();
    d.mision.clear();
    r.pinta(soloReloj, pincel(), d, 1280, 720);
    MOTOR_CHECK(hojasPedidas(soloReloj) == hojasPedidas(sinReloj));
    d.relojes.clear();
    Lienzo nada;
    r.pinta(nada, pincel(), d, 1280, 720);
    MOTOR_CHECK(hojasPedidas(nada) == hojasPedidas(sinReloj) - 1);
}

// --- La interfaz entera -----------------------------------------------------

MOTOR_TEST_CASE(interfaz_oculta_no_pinta_nada) {
    InterfazPergamino i;
    MOTOR_CHECK(i.carga("assets").isOk());
    EstadoJuego e;
    e.oculta = true;
    i.actualiza(e);
    MOTOR_CHECK(i.capaBaja().vacio() && i.capaAlta().vacio() && i.hojaViva().vacio());
    MOTOR_CHECK(!i.hojaVisible() && !i.marcaSuelo().visible);
    EstadoJuego sinSesion;
    i.actualiza(sinSesion);
    MOTOR_CHECK(i.capaBaja().vacio() && i.capaAlta().vacio());
}

MOTOR_TEST_CASE(interfaz_sin_letras_no_carga) {
    InterfazPergamino i;
    MOTOR_CHECK(!i.carga("no_existe").isOk());
    MOTOR_CHECK(!i.cargada());
}
