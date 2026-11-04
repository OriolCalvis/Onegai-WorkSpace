// UI-BG3-01 — test_pergamino (3/3): el cuaderno de creacion de siete hojas
// (maquetas C02), medido sin ventana. Que cada hoja deja sus zonas para el
// raton (las siete pestanas, las teselas, las tarjetas de la historia, el
// boton), que nada se sale de la vista de 1280x720, que las teselas no se
// pisan y que sin quien monte personajes la hoja se pinta igual (sin
// retratos). Tambien el papiro del titulo con su motivo y sus zonas.
#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

#include "Game/CuadernoCreacion.h"
#include "Game/Personajes/KitBasico.h"
#include "HUD/Iconos.h"
#include "HUD/Pergamino/AtlasRetratos.h"
#include "HUD/Pergamino/FuenteAtlas.h"
#include "HUD/Pergamino/PaginasCreacion.h"
#include "HUD/Pergamino/Papiro.h"
#include "HUD/Pergamino/Pincel.h"
#include "RPG/CharacterCreation.h"
#include "TestHarness.h"

using namespace Pergamino;

namespace {

using Game::CuadernoCreacion;
using Hoja = CuadernoCreacion::Hoja;
using ZonaC = CuadernoCreacion::Zona;

struct Mundo {
    FuenteAtlas letras;
    Hud::Iconos iconos;
    RPG::CharacterCreation reglas;
    RPG::TierRules tiers;
    RPG::Catalogs::EquipmentCatalog equipo;
    Game::KitBasico::Reglas kit;
    Mundo() : reglas(std::move(RPG::CharacterCreation::load("assets/catalogs").value())) {
        MOTOR_CHECK(letras.carga("assets/textures/letras_pergamino.json").isOk());
        MOTOR_CHECK(iconos.cargar("assets/textures/iconos.json").isOk());
        (void)tiers.loadFromFile("assets/rules/tier_rules.json");
        MOTOR_CHECK(equipo.loadFromFile("assets/catalogs/equipment.json").isOk());
    }
    CuadernoCreacion::Datos datos() const {
        CuadernoCreacion::Datos d;
        d.reglas = &reglas;
        d.tiers = &tiers;
        d.equipo = &equipo;
        d.kit = &kit;
        return d;
    }
};

const Mundo& mundo() {
    static const Mundo m;
    return m;
}

// Pinta la hoja actual del cuaderno, sin montaje de personajes.
Lienzo pinta(const CuadernoCreacion& c) {
    Lienzo l;
    AtlasRetratos atlas;
    Retratista retratos(atlas, nullptr);
    const Pincel p(mundo().letras, mundo().iconos);
    pintaCreacion(l, p, c, retratos, 1280, 720);
    return l;
}

int cuantas(const Lienzo& l, ZonaC tipo) {
    int n = 0;
    for (const Zona& z : l.zonas()) n += z.tipo == static_cast<int>(tipo) ? 1 : 0;
    return n;
}

bool dentroDeLaVista(const Lienzo& l) {
    for (const Trazo& t : l.trazos())
        for (const Vector2& v : t.p)
            if (v.x < -1.0f || v.y < -1.0f || v.x > 1281.0f || v.y > 721.0f) return false;
    return true;
}

int posicion(const CuadernoCreacion& c, Hoja h, const std::string& id) {
    const auto lista = c.ids(h);
    for (std::size_t i = 0; i < lista.size(); ++i)
        if (lista[i] == id) return static_cast<int>(i);
    return -1;
}

MOTOR_TEST_CASE(creacion_cada_hoja_deja_sus_pestanas_y_cabe_en_la_vista) {
    CuadernoCreacion c(mundo().datos());
    for (int h = 0; h < CuadernoCreacion::kHojas; ++h) {
        c.irA(static_cast<Hoja>(h));
        const Lienzo l = pinta(c);
        MOTOR_CHECK(cuantas(l, ZonaC::Pestana) == CuadernoCreacion::kHojas);
        MOTOR_CHECK(cuantas(l, ZonaC::Continuar) == 1);  // CONTINUAR o «Empezar la aventura»
        MOTOR_CHECK(dentroDeLaVista(l));
    }
}

MOTOR_TEST_CASE(creacion_raza_veinte_teselas_que_no_se_pisan) {
    CuadernoCreacion c(mundo().datos());
    const Lienzo l = pinta(c);
    MOTOR_CHECK(cuantas(l, ZonaC::Tesela) == 20);  // 5 x 4 de 43
    MOTOR_CHECK(cuantas(l, ZonaC::Buscar) == 1);
    std::vector<Rect> teselas;
    for (const Zona& z : l.zonas())
        if (z.tipo == static_cast<int>(ZonaC::Tesela)) teselas.push_back(z.rect);
    for (std::size_t i = 0; i < teselas.size(); ++i)
        for (std::size_t j = i + 1; j < teselas.size(); ++j) {
            const Rect& a = teselas[i];
            const Rect& b = teselas[j];
            const bool pisa = a.x < b.derecha() && b.x < a.derecha() && a.y < b.abajo() && b.y < a.abajo();
            MOTOR_CHECK(!pisa);
        }
    // La rejilla queda en la columna izquierda de la hoja (no tapa el centro).
    for (const Rect& r : teselas) MOTOR_CHECK(r.derecha() < 640.0f && r.y > 100.0f && r.abajo() < 630.0f);
}

MOTOR_TEST_CASE(creacion_el_clic_de_la_zona_elige_lo_que_se_ve) {
    CuadernoCreacion c(mundo().datos());
    const Lienzo l = pinta(c);
    // La tercera tesela (fila 0, columna 2) es la tercera raza de la lista.
    const Zona* z = nullptr;
    for (const Zona& zz : l.zonas())
        if (zz.tipo == static_cast<int>(ZonaC::Tesela) && zz.indice == 2) z = &zz;
    MOTOR_CHECK(z != nullptr);
    if (z == nullptr) return;
    const Zona* bajo = l.zonaEn(z->rect.x + 10.0f, z->rect.y + 10.0f);
    MOTOR_CHECK(bajo != nullptr && bajo->indice == 2);
    c.pulsa(static_cast<ZonaC>(bajo->tipo), bajo->indice);
    MOTOR_CHECK(c.eleccion().raceId == c.ids(Hoja::Raza)[2]);
}

MOTOR_TEST_CASE(creacion_historia_treinta_y_cinco_tarjetas_con_origen) {
    CuadernoCreacion c(mundo().datos());
    c.irA(Hoja::Historia);
    MOTOR_CHECK(cuantas(pinta(c), ZonaC::Tarjeta) == 0);  // sin origen: «Ir a Origen»
    MOTOR_CHECK(cuantas(pinta(c), ZonaC::Editar) == 1);
    c.irA(Hoja::Origen);
    c.pulsa(ZonaC::Tesela, posicion(c, Hoja::Origen, "mes_de_la_cosecha"));
    c.irA(Hoja::Historia);
    const Lienzo l = pinta(c);
    MOTOR_CHECK(cuantas(l, ZonaC::Tarjeta) == 35);
    MOTOR_CHECK(dentroDeLaVista(l));
}

MOTOR_TEST_CASE(creacion_clase_filtros_y_sin_montaje_no_hay_retratos) {
    CuadernoCreacion c(mundo().datos());
    c.irA(Hoja::Clase);
    const Lienzo l = pinta(c);
    MOTOR_CHECK(cuantas(l, ZonaC::Rol) == static_cast<int>(CuadernoCreacion::roles().size()));
    MOTOR_CHECK(cuantas(l, ZonaC::Tesela) == 15);
    for (const Trazo& t : l.trazos()) MOTOR_CHECK(t.fuente != Fuente::Retratos);
}

MOTOR_TEST_CASE(creacion_nombre_sugerencias_y_editar_cada_eleccion) {
    CuadernoCreacion c(mundo().datos());
    c.irA(Hoja::Nombre);
    const Lienzo l = pinta(c);
    MOTOR_CHECK(cuantas(l, ZonaC::Sugerencia) == static_cast<int>(CuadernoCreacion::sugerencias().size()));
    MOTOR_CHECK(cuantas(l, ZonaC::Editar) == 5);  // raza, clase, origen, deidad e historia
}

MOTOR_TEST_CASE(atlas_de_retratos_guarda_cada_clave_una_vez_y_se_vacia_lleno) {
    AtlasRetratos atlas;
    int fabricados = 0;
    auto fabrica = [&](Imagen& f) {
        ++fabricados;
        f.pixel(0, 0)[3] = 255;
        return true;
    };
    const Rect a = atlas.celda("a", fabrica);
    const int version = atlas.version();
    const Rect a2 = atlas.celda("a", fabrica);
    MOTOR_CHECK(fabricados == 1 && atlas.version() == version);
    MOTOR_CHECK(a.x == a2.x && a.y == a2.y && a.w > 0.0f);
    MOTOR_CHECK(atlas.celda("nada", [](Imagen&) { return false; }).w == 0.0f);
    for (int i = 0; i < AtlasRetratos::kCeldas; ++i) atlas.celda("c" + std::to_string(i), fabrica);
    MOTOR_CHECK(atlas.cuantas() <= static_cast<std::size_t>(AtlasRetratos::kCeldas));
    MOTOR_CHECK(!atlas.tiene("a"));  // se lleno y empezo de nuevo
}

MOTOR_TEST_CASE(papiro_apagada_lleva_su_motivo_y_cada_entrada_su_zona) {
    DatosPapiro d;
    d.ancho = 420.0f;
    d.arriba = 236.0f;
    d.velo = false;
    EntradaPapiro continuar;
    continuar.texto = "Continuar";
    continuar.apagada = true;
    continuar.motivo = "No hay ninguna partida guardada.";
    d.entradas = {continuar, EntradaPapiro{"Nueva partida"}, EntradaPapiro{"Salir"}};
    d.foco = 1;
    d.pie = {{{"W", "S"}, "Elegir"}, {{"ENTER"}, "Aceptar"}, {{"ESC"}, "Salir"}};
    d.notaPie = "o haz clic con el ratón";
    Lienzo l;
    const Pincel p(mundo().letras, mundo().iconos);
    pintaPapiro(l, p, d, 1280, 720);
    int zonas = 0;
    for (const Zona& z : l.zonas()) zonas += z.tipo == kZonaEntradaPapiro ? 1 : 0;
    MOTOR_CHECK(zonas == 3);
    // La entrada apagada es mas alta (su motivo va debajo) y no hay velo.
    MOTOR_CHECK(l.zonas()[0].rect.h > l.zonas()[1].rect.h);
    for (const Trazo& t : l.trazos())
        MOTOR_CHECK(!(t.fuente == Fuente::Blanco && t.p[0].x <= 0.0f && t.p[2].x >= 1280.0f && t.p[2].y >= 720.0f));
    MOTOR_CHECK(dentroDeLaVista(l));
}

}  // namespace

MOTOR_TEST_CASE(creacion_aspecto_muestra_dos_orientaciones_sin_pisar_las_columnas) {
    auto cargado = Game::ManifiestoPersonajes::cargar("assets/textures");
    MOTOR_CHECK(cargado.isOk()); if (!cargado.isOk()) return;
    auto personajes = cargado.value();
    auto datos = mundo().datos(); datos.personajes = &personajes;
    CuadernoCreacion c(datos);
    auto razas = c.ids(Hoja::Raza);
    const auto it = std::find(razas.begin(), razas.end(), "minotauros_de_la_llanura");
    MOTOR_CHECK(it != razas.end()); if (it == razas.end()) return;
    c.pulsa(ZonaC::Tesela, static_cast<int>(it - razas.begin())); c.irA(Hoja::Aspecto);
    Game::MontajePersonajes montaje(personajes, "assets/textures");
    AtlasRetratos atlas; Retratista retratos(atlas, &montaje);
    Lienzo l; const Pincel p(mundo().letras, mundo().iconos);
    pintaCreacion(l, p, c, retratos, 1280, 720);
    int vistas = 0;
    for (const auto& trazo : l.trazos()) if (trazo.fuente == Fuente::Retratos) {
        ++vistas; MOTOR_CHECK(trazo.p[0].x >= 650.0f); MOTOR_CHECK(trazo.p[2].x <= 906.0f);
    }
    MOTOR_CHECK(vistas == 2); MOTOR_CHECK(dentroDeLaVista(l));
}

MOTOR_TEST_CASE(el_aviso_del_retrato_sobrevive_a_la_primera_reserva_del_atlas) {
    AtlasRetratos atlas; Imagen tira; tira.crea(64,64);
    atlas.guardaTira("apariencia", std::move(tira));
    atlas.guardaAviso("apariencia#0", "Cuernos no visibles en esta vista");
    atlas.celda("apariencia#0", [](Imagen& f) { f.crea(64,64); return true; });
    MOTOR_CHECK(!atlas.aviso("apariencia#0").empty());
}

// PERF-2: la rejilla de Raza pide 20 retratos y en frio cada raza cuesta casi
// un segundo; como mucho 2 montajes nuevos por fotograma, el resto en los
// siguientes. Se cuentan las celdas nuevas del atlas (una por retrato montado).
namespace {
int retratosPintados(const Lienzo& l) {
    int n = 0;
    for (const auto& t : l.trazos()) n += t.fuente == Fuente::Retratos ? 1 : 0;
    return n;
}
// Solo los de la rejilla (columna izquierda); el personaje central queda fuera.
int retratosEnRejilla(const Lienzo& l) {
    int n = 0;
    for (const auto& t : l.trazos()) n += t.fuente == Fuente::Retratos && t.p[0].x < 640.0f ? 1 : 0;
    return n;
}
}  // namespace

MOTOR_TEST_CASE(creacion_raza_monta_como_mucho_dos_retratos_por_fotograma_y_los_llena_en_diez) {
    auto cargado = Game::ManifiestoPersonajes::cargar("assets/textures");
    MOTOR_CHECK(cargado.isOk()); if (!cargado.isOk()) return;
    auto personajes = cargado.value();
    auto datos = mundo().datos(); datos.personajes = &personajes;
    CuadernoCreacion c(datos);
    Game::MontajePersonajes montaje(personajes, "assets/textures");
    AtlasRetratos atlas; Retratista retratos(atlas, &montaje);
    const Pincel p(mundo().letras, mundo().iconos);
    int previos = 0, pintados = 0;
    for (int fotograma = 0; fotograma < 10; ++fotograma) {
        atlas.nuevoFotograma();
        Lienzo l; pintaCreacion(l, p, c, retratos, 1280, 720);
        const int nuevas = static_cast<int>(atlas.cuantas()) - previos;
        previos = static_cast<int>(atlas.cuantas());
        MOTOR_CHECK(nuevas <= AtlasRetratos::kMontajesPorFotograma);
        MOTOR_CHECK(atlas.montajes() <= AtlasRetratos::kMontajesPorFotograma);
        if (fotograma == 0) MOTOR_CHECK(retratosEnRejilla(l) == 2);  // las dos primeras casillas
        pintados = retratosEnRejilla(l);
    }
    MOTOR_CHECK(pintados == cuantas(pinta(c), ZonaC::Tesela));  // las 20 con retrato
}

MOTOR_TEST_CASE(creacion_aspecto_el_central_monta_aunque_el_cupo_este_gastado) {
    auto cargado = Game::ManifiestoPersonajes::cargar("assets/textures");
    MOTOR_CHECK(cargado.isOk()); if (!cargado.isOk()) return;
    auto personajes = cargado.value();
    auto datos = mundo().datos(); datos.personajes = &personajes;
    CuadernoCreacion c(datos);
    c.pulsa(ZonaC::Tesela, 0); c.irA(Hoja::Aspecto);
    Game::MontajePersonajes montaje(personajes, "assets/textures");
    AtlasRetratos atlas; Retratista retratos(atlas, &montaje);
    for (int i = 0; i < AtlasRetratos::kMontajesPorFotograma; ++i) atlas.cuentaMontaje();
    const Pincel p(mundo().letras, mundo().iconos);
    Lienzo l; pintaCreacion(l, p, c, retratos, 1280, 720);
    MOTOR_CHECK(retratosPintados(l) == 2);  // sin cupo, el central se monta igual
    MOTOR_CHECK(atlas.montajes() == AtlasRetratos::kMontajesPorFotograma);  // y no cuenta
}
