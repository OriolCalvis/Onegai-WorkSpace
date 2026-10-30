// TIENDA-02 — test_pergamino: la tienda en su hoja, medida sin ventana.
//
// Que la hoja quepa en la vista y se centre, que haya una sola cinta de foco
// y caiga en la fila elegida, que las pestañas, las filas y el botón
// respondan al ratón donde se ven, que sin oro el botón se apague con su
// motivo y que la venta sin nada que vender sea un estado vacío honesto.
#include <string>
#include <vector>

#include "Game/GameSession.h"
#include "Game/TiendaEstado.h"
#include "HUD/Iconos.h"
#include "HUD/Pergamino/FuenteAtlas.h"
#include "HUD/Pergamino/PaginaCuaderno.h"
#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/TiendaHoja.h"
#include "Level/ObjectCatalog.h"
#include "Render/ICombatant.h"
#include "TestHarness.h"

using namespace Pergamino;

namespace {

struct Letras {
    FuenteAtlas letras;
    Hud::Iconos iconos;
    Letras() {
        MOTOR_CHECK(letras.carga("assets/textures/letras_pergamino.json").isOk());
        MOTOR_CHECK(iconos.cargar("assets/textures/iconos.json").isOk());
    }
};

const Letras& letras() {
    static const Letras r;
    return r;
}

class Cuerpo : public ICombatant {
public:
    void takeDamage(int) override {}
    void heal(int) override {}
    int health() const override { return 20; }
    int maxHealth() const override { return 20; }
    bool isAlive() const override { return true; }
    int stat(RPG::Stat) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "test"; }
};

struct Mercado {
    ObjectCatalog catalog;
    SkillCatalog skills;
    Cuerpo cuerpo;
    SkillSet habilidades{10};
    std::vector<std::string> mochila;
    Mercado() {
        ObjectDefinition pocion;
        pocion.id = "pocion";
        pocion.name = "Poción";
        pocion.category = ObjectCategory::Pickup;
        pocion.pickup = PickupData{PickupEffect::Heal, 15, 30};
        catalog.add(pocion);
        ObjectDefinition tendero;
        tendero.id = "tendero";
        tendero.name = "Tendero del mercado";
        tendero.category = ObjectCategory::Npc;
        tendero.shop.items.push_back(ShopItem{"pocion", 30});
        tendero.shop.buybackPercent = 50;
        catalog.add(tendero);
    }
    GameSession sesion(int oro) {
        LevelDefinition nivel;
        nivel.name = "Mercado";
        GameSession s(nullptr, &catalog, &skills, nivel, &cuerpo, &habilidades, &mochila);
        s.addGold(oro);
        MOTOR_CHECK(s.openShop("tendero").isOk());
        return s;
    }
};

int cintas(const Lienzo& l) {
    int n = 0;
    for (const Trazo& t : l.trazos())
        if (t.fuente == Fuente::Material && t.material >= 0 &&
            l.materiales()[static_cast<std::size_t>(t.material)].tipo == Material::Cinta)
            ++n;
    return n;
}

const Zona* zona(const Lienzo& l, int tipo, int indice) {
    for (const Zona& z : l.zonas())
        if (z.tipo == tipo && z.indice == indice) return &z;
    return nullptr;
}

bool dentro(const Rect& a, const Rect& b) {
    return a.x >= b.x && a.y >= b.y && a.derecha() <= b.derecha() && a.abajo() <= b.abajo();
}

}  // namespace

MOTOR_TEST_CASE(la_hoja_de_la_tienda_cabe_en_la_vista_y_se_centra) {
    const Rect r = rectTienda(1280, 720);
    MOTOR_CHECK(r.w == 940.0f && r.h == 600.0f);
    MOTOR_CHECK(r.x == 170.0f && r.y == 60.0f);
    const Rect grande = rectTienda(1920, 1080);
    MOTOR_CHECK(grande.x == 490.0f && grande.y == 240.0f);
}

MOTOR_TEST_CASE(comprar_pinta_precio_boton_y_una_sola_cinta_en_la_fila_elegida) {
    Mercado m;
    GameSession s = m.sesion(100);
    Game::TiendaEstado t;
    t.sigue(true);
    const DatosTienda d = datosTienda(t, s);
    MOTOR_CHECK(d.tendero == "Tendero del mercado" && d.oro == 100 && d.pestana == 0);
    MOTOR_CHECK(d.pagina.filas.size() == 1);
    MOTOR_CHECK(d.pagina.filas[0].nombre == "Poción" && d.pagina.filas[0].valor == "30");
    MOTOR_CHECK(!d.pagina.filas[0].apagada);
    MOTOR_CHECK(d.pagina.detalle.efecto == "Recupera 15 PV.");
    MOTOR_CHECK(d.pagina.detalle.boton == "COMPRAR 1 · 30 ORO" && d.pagina.detalle.botonActivo);
    MOTOR_CHECK(d.pagina.detalle.motivo.empty());
    const Pincel p(letras().letras, letras().iconos);
    Lienzo l;
    pintaTienda(l, p, d, 1280, 720);
    MOTOR_CHECK(cintas(l) == 1);
    const Rect hoja = rectTienda(1280, 720);
    for (int i : {0, 1}) {
        const Zona* pe = zona(l, kZonaPestanaTienda, i);
        MOTOR_CHECK(pe != nullptr && dentro(pe->rect, hoja));
    }
    const Zona* fila = zona(l, kZonaFilaPagina, 0);
    const Zona* boton = zona(l, kZonaBotonPagina, 0);
    MOTOR_CHECK(fila != nullptr && dentro(fila->rect, hoja));
    MOTOR_CHECK(boton != nullptr && dentro(boton->rect, hoja));
    MOTOR_CHECK(boton->rect.x > fila->rect.derecha());  // el detalle, a la derecha de la lista
    MOTOR_CHECK(l.zonaEn(boton->rect.x + 4.0f, boton->rect.y + 4.0f)->tipo == kZonaBotonPagina);
}

MOTOR_TEST_CASE(sin_oro_la_fila_se_apaga_y_el_boton_dice_por_que) {
    Mercado m;
    GameSession s = m.sesion(10);
    Game::TiendaEstado t;
    t.sigue(true);
    const DatosTienda d = datosTienda(t, s);
    MOTOR_CHECK(d.pagina.filas[0].apagada);
    MOTOR_CHECK(!d.pagina.detalle.botonActivo);
    MOTOR_CHECK(d.pagina.detalle.motivo == "Te faltan 20 de oro.");
}

MOTOR_TEST_CASE(vender_dice_cuanto_recibes_antes_de_vender) {
    Mercado m;
    GameSession s = m.sesion(5);
    m.mochila = {"pocion", "pocion"};
    Game::TiendaEstado t;
    t.sigue(true);
    t.cambiaPestana(+1);
    const DatosTienda d = datosTienda(t, s);
    MOTOR_CHECK(d.pestana == 1 && d.pagina.titulo == "VENDER");
    MOTOR_CHECK(d.pagina.filas.size() == 1 && d.pagina.filas[0].valor == "15");
    MOTOR_CHECK(d.pagina.detalle.boton == "VENDER 1 · RECIBES 15" && d.pagina.detalle.botonActivo);
    bool tendras = false;
    for (const DatoPagina& dato : d.pagina.detalle.datos) tendras |= dato.clave == "Tendrás" && dato.valor == "20 de oro";
    MOTOR_CHECK(tendras);
}

MOTOR_TEST_CASE(vender_sin_nada_que_vender_es_un_estado_vacio_honesto) {
    Mercado m;
    GameSession s = m.sesion(5);
    Game::TiendaEstado t;
    t.sigue(true);
    t.cambiaPestana(+1);
    const DatosTienda d = datosTienda(t, s);
    MOTOR_CHECK(d.pagina.tipo == PaginaCuaderno::Tipo::Vacia);
    MOTOR_CHECK(d.pagina.vacioTitulo == "No llevas nada que le interese");
    const Pincel p(letras().letras, letras().iconos);
    Lienzo l;
    pintaTienda(l, p, d, 1280, 720);
    MOTOR_CHECK(cintas(l) == 0);
    MOTOR_CHECK(zona(l, kZonaBotonPagina, 0) == nullptr);
}

MOTOR_TEST_CASE(el_resultado_sale_en_la_hoja) {
    Mercado m;
    GameSession s = m.sesion(100);
    Game::TiendaEstado t;
    t.sigue(true);
    t.confirma(s);
    const DatosTienda d = datosTienda(t, s);
    MOTOR_CHECK(d.estado == "Comprado: Poción · –30 de oro · ahora tienes 1");
    MOTOR_CHECK(!d.estadoError);
}
