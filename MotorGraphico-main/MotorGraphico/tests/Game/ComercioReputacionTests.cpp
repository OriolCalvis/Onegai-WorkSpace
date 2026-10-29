// AV14-23 (D0015 §15) — La reputación por facción, leída (D-MOR-09), y los
// rumores como beats que saltan al hablar (D-MIS-03).
//
//   1  las facciones publicadas casan con las tiendas de verdad: cada tendero
//      de una facción existe y vende algo, y ni con el mayor descuento sale a
//      cuenta comprarle algo y revendérselo;
//   2  la tienda cobra según lo que su facción piensa de ti (con topes), te
//      paga menos si te detestan, y no cambia nada sin facción o sin ficha;
//   3  la línea de la tienda y la ficha lo dicen en palabras;
//   4  la reputación viaja en el guardado (y uno de antes la trae vacía);
//   5  un rumor es un beat `talk`: sale una vez, y detrás de la charla no se
//      la come.
//
// Los assets se leen del directorio de trabajo (copiar_assets).

#include "Game/ComercioReputacion.h"
#include "Game/GameSession.h"
#include "Game/SaveGame.h"
#include "Level/ObjectCatalog.h"
#include "RPG/CharacterSheet.h"
#include "RPG/NarrativeEngine.h"
#include "RPG/Narrative/AdventureScript.h"
#include "RPG/Narrative/NarrativeState.h"
#include "Render/ICombatant.h"
#include "TestHarness.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace {

class Heroe : public ICombatant {
public:
    RPG::CharacterSheet ficha;
    void takeDamage(int) override {}
    void heal(int) override {}
    int health() const override { return 10; }
    int maxHealth() const override { return 10; }
    bool isAlive() const override { return true; }
    int stat(RPG::Stat) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "heroe"; }
    RPG::CharacterSheet* character_sheet() override { return &ficha; }
    const RPG::CharacterSheet* character_sheet() const override { return &ficha; }
};

LevelDefinition nivel() {
    LevelDefinition level;
    level.name = "Ciudad";
    level.playerStart = GridCoord{0, 0};
    return level;
}

struct Tienda {
    ObjectCatalog catalogo;
    SkillCatalog skills;
    Heroe heroe;
    SkillSet habilidades{10};
    std::vector<std::string> mochila;
    Tienda() {
        ObjectDefinition pocion;
        pocion.id = "pocion";
        pocion.name = "Pocion";
        pocion.category = ObjectCategory::Pickup;
        pocion.pickup.price = 40;
        catalogo.add(pocion);
        ObjectDefinition rosita;
        rosita.id = "skilla";
        rosita.name = "Rosita";
        rosita.category = ObjectCategory::Npc;
        rosita.shop.items.push_back(ShopItem{"pocion", 0});
        rosita.shop.buybackPercent = 50;
        catalogo.add(rosita);
        ObjectDefinition forastero = rosita;
        forastero.id = "buhonero";
        forastero.name = "Buhonero";
        catalogo.add(forastero);
        mochila = {"pocion"};
    }
    GameSession sesion() {
        GameSession s(nullptr, &catalogo, &skills, nivel(), &heroe, &habilidades, &mochila);
        s.addGold(500);
        return s;
    }
};

std::shared_ptr<const Game::ReglasFacciones> reglasDePrueba() {
    return std::make_shared<const Game::ReglasFacciones>(Game::leeFacciones(R"({
        "precios": {"descuentoMaximo": 25, "recargoMaximo": 50},
        "facciones": [{"id": "aegroum", "nombre": "Aegroum", "tiendas": ["skilla"]},
                      {"id": "perdidos", "nombre": "Los Perdidos", "tiendas": []}]})"));
}

}  // namespace

MOTOR_TEST_CASE(las_facciones_publicadas_casan_con_las_tiendas_y_no_hay_reventa) {
    Game::ponFacciones(nullptr);
    const Game::ReglasFacciones& r = Game::facciones();
    MOTOR_CHECK(r.faccion("aegroum") != nullptr && r.faccion("perdidos") != nullptr);
    MOTOR_CHECK(r.faccion("barriada_surysal") != nullptr);
    MOTOR_CHECK(r.deTienda("skilla") != nullptr && r.deTienda("skilla")->id == "aegroum");
    MOTOR_CHECK(r.deTienda("en_choubar_mercado_tendero") == nullptr);  // otra nacion: sin faccion

    ObjectCatalog catalogo;
    for (const char* f : {"assets/objects/ciudad_objetos.json", "assets/objects/boundington_npcs.json"})
        MOTOR_CHECK(catalogo.loadFromFile(f).isOk());
    int tiendas = 0;
    for (const Game::Faccion& f : r.facciones) {
        for (const std::string& id : f.tiendas) {
            const ObjectDefinition* def = catalogo.find(id);
            if (def == nullptr || def->shop.items.empty())
                std::fprintf(stderr, "  la tienda %s de %s no existe o no vende nada\n", id.c_str(), f.id.c_str());
            MOTOR_CHECK(def != nullptr && !def->shop.items.empty());
            if (def == nullptr) continue;
            ++tiendas;
            // Con el mayor descuento, comprar sigue costando mas de lo que te
            // pagan al revenderlo (la recompra no sube con la reputacion).
            for (const ShopItem& item : def->shop.items) {
                const ObjectDefinition* obj = catalogo.find(item.objectId);
                const int base = item.price > 0 ? item.price : (obj != nullptr ? obj->pickup.price : 0);
                if (base <= 0) continue;
                const int compra = Game::precioConAjuste(base, -r.descuentoMaximo);
                const int venta = base * Game::recompraConAjuste(def->shop.buybackPercent, -r.descuentoMaximo) / 100;
                if (compra <= venta)
                    std::fprintf(stderr, "  %s: %s se compra a %d y se revende a %d\n", id.c_str(),
                                 item.objectId.c_str(), compra, venta);
                MOTOR_CHECK(compra > venta);
            }
        }
    }
    MOTOR_CHECK(tiendas >= 3);
}

MOTOR_TEST_CASE(la_tienda_cobra_segun_lo_que_su_faccion_piensa_de_ti) {
    Game::ponFacciones(reglasDePrueba());
    Tienda t;

    // Neutral: el precio de siempre.
    {
        GameSession s = t.sesion();
        MOTOR_CHECK(s.openShop("skilla").isOk());
        MOTOR_CHECK(s.shopOffers()[0].price == 40);
        MOTOR_CHECK(s.sellPriceOf("pocion") == 20);
    }
    // Te aprecian (+20): repPriceModPct da -15 %.
    t.heroe.ficha.adjustRep("aegroum", 20);
    {
        GameSession s = t.sesion();
        MOTOR_CHECK(s.openShop("skilla").isOk());
        MOTOR_CHECK(s.shopOffers()[0].price == 34);
        MOTOR_CHECK(s.sellPriceOf("pocion") == 20);  // no te pagan mas
        // El buhonero no es de ninguna faccion: le da igual.
    }
    {
        GameSession s = t.sesion();
        MOTOR_CHECK(s.openShop("buhonero").isOk());
        MOTOR_CHECK(s.shopOffers()[0].price == 40);
    }
    // Eres de los suyos (+80): el descuento se corta en el 25 %.
    t.heroe.ficha.adjustRep("aegroum", 60);
    {
        GameSession s = t.sesion();
        MOTOR_CHECK(s.openShop("skilla").isOk());
        MOTOR_CHECK(s.shopOffers()[0].price == 30);
    }
    // Te detestan (-100): +75 % cortado en +50 %, y te pagan la mitad de la mitad.
    t.heroe.ficha.adjustRep("aegroum", -180);
    MOTOR_CHECK(t.heroe.ficha.getRep("aegroum") == -100);
    {
        GameSession s = t.sesion();
        MOTOR_CHECK(s.openShop("skilla").isOk());
        MOTOR_CHECK(s.shopOffers()[0].price == 60);
        MOTOR_CHECK(s.sellPriceOf("pocion") == 10);
        // Y la compra cobra exactamente lo que se ve.
        const int antes = s.gold();
        MOTOR_CHECK(s.buy(0));
        MOTOR_CHECK(antes - s.gold() == 60);
    }
    Game::ponFacciones(nullptr);
}

MOTOR_TEST_CASE(la_tienda_y_la_ficha_lo_dicen_en_palabras) {
    const auto r = reglasDePrueba();
    MOTOR_CHECK(Game::comoTeVen(0) == "Neutral");
    MOTOR_CHECK(Game::comoTeVen(12) == "Te aprecian");
    MOTOR_CHECK(Game::comoTeVen(30) == "Te estiman");
    MOTOR_CHECK(Game::comoTeVen(70) == "Eres de los suyos");
    MOTOR_CHECK(Game::comoTeVen(-10) == "Recelan");
    MOTOR_CHECK(Game::comoTeVen(-30) == "Desconfían");
    MOTOR_CHECK(Game::comoTeVen(-60) == "Te detestan");

    RPG::CharacterSheet ficha;
    MOTOR_CHECK(Game::lineaDeLaTienda(*r, &ficha, "skilla").empty());  // neutral: nada que decir
    MOTOR_CHECK(Game::reputacionEnLaFicha(*r, &ficha).empty());
    ficha.adjustRep("aegroum", 12);
    ficha.adjustRep("perdidos", -25);
    ficha.adjustRep("gremio_raro", 5);
    MOTOR_CHECK(Game::lineaDeLaTienda(*r, &ficha, "skilla") == "Aegroum: te aprecian (+12). Pagas un 9 % menos.");
    MOTOR_CHECK(Game::lineaDeLaTienda(*r, &ficha, "buhonero").empty());
    const auto filas = Game::reputacionEnLaFicha(*r, &ficha);
    MOTOR_CHECK(filas.size() == 3);
    MOTOR_CHECK(filas[0].first == "Aegroum" && filas[0].second == "Te aprecian (+12)");
    MOTOR_CHECK(filas[1].first == "Los Perdidos" && filas[1].second == "Desconfían (-25)");
    MOTOR_CHECK(filas[2].first == "gremio_raro");  // la que el fichero no conoce, con su id
    ficha.adjustRep("aegroum", -112);
    MOTOR_CHECK(Game::lineaDeLaTienda(*r, &ficha, "skilla") ==
                "Aegroum: te detestan (-100). Pagas un 50 % más, y te pagan menos.");

    // El tendero abierto, por su nombre.
    Tienda t;
    MOTOR_CHECK(Game::tenderoPorNombre(*r, &t.catalogo, "Rosita") == "skilla");
    MOTOR_CHECK(Game::tenderoPorNombre(*r, &t.catalogo, "Buhonero").empty());
}

MOTOR_TEST_CASE(la_reputacion_viaja_en_el_guardado) {
    SaveGameData d;
    d.levelPath = "assets/levels/ciudad_centro.json";
    d.hero.classId = "guerrero";
    d.hero.reputacion = {{"aegroum", 12}, {"perdidos", -25}};
    SaveGameData leido;
    std::string error;
    MOTOR_CHECK(parseSaveGame(serializeSaveGame(d), &leido, &error));
    MOTOR_CHECK(leido.hero.reputacion.size() == 2);
    MOTOR_CHECK(leido.hero.reputacion["aegroum"] == 12 && leido.hero.reputacion["perdidos"] == -25);

    // Sin reputacion no se escribe la clave (los guardados de antes se leen igual).
    d.hero.reputacion.clear();
    const std::string texto = serializeSaveGame(d);
    MOTOR_CHECK(texto.find("reputacion") == std::string::npos);
    SaveGameData viejo;
    MOTOR_CHECK(parseSaveGame(texto, &viejo, &error));
    MOTOR_CHECK(viejo.hero.reputacion.empty());
}

MOTOR_TEST_CASE(un_rumor_es_un_beat_que_salta_al_hablar) {
    const auto a = RPG::AdventureScript::loadFromString(R"({"id": "adv_rumor", "name": "R", "beats": [
        {"id": "charla_rosita", "trigger": {"type": "talk", "target": "skilla"},
         "requires": {"allFlags": ["cena_servida"], "notFlags": ["cena_comida"]},
         "speaker": "Rosita", "lines": ["Come, que se enfria."],
         "effects": [{"type": "setFlag", "arg": "cena_comida"}]},
        {"id": "rumor_trolls_rosita", "trigger": {"type": "talk", "target": "skilla"},
         "requires": {"allFlags": ["bnd0_noche"], "notFlags": ["rumor_trolls"]},
         "speaker": "Rosita", "lines": ["Cuenta que la Furia mato a doce trolls."],
         "effects": [{"type": "setFlag", "arg": "rumor_trolls"}, {"type": "setFlag", "arg": "bnd_rumor"}]}]})");
    MOTOR_CHECK(a.isOk());
    RPG::NarrativeEngine motor;
    motor.setAdventure(&a.value());
    RPG::NarrativeState e;

    // De dia no lo cuenta (sus condiciones).
    MOTOR_CHECK(!motor.talkTo("skilla", e).fired);
    e.setFlag("bnd0_noche");
    // Detras de la charla: si la charla tiene algo, habla la charla.
    e.setFlag("cena_servida");
    RPG::NarrativeResult r = motor.talkTo("skilla", e);
    MOTOR_CHECK(r.fired && r.beatId == "charla_rosita");
    // Sin nada mas que hablar, el rumor; deja su flag.
    r = motor.talkTo("skilla", e);
    MOTOR_CHECK(r.fired && r.beatId == "rumor_trolls_rosita");
    MOTOR_CHECK(e.hasFlag("rumor_trolls") && e.hasFlag("bnd_rumor"));
    // Y una sola vez.
    MOTOR_CHECK(!motor.talkTo("skilla", e).fired);
    // A otro PNJ no le toca.
    MOTOR_CHECK(!motor.talkTo("aigren", e).fired);
}

int main() { return motor_test::runAllTests(); }
