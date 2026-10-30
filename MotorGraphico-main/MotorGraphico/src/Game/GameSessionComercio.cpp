// Comprar, vender y vivir de las rentas.
//
// La tienda resuelve su oferta al abrirse —no en cada consulta del HUD—
// y los negocios cobran por pasos dados. Dos economias distintas que
// comparten el mismo oro, y por eso viajan juntas.
//
// Salio de src/Game/GameSession.cpp el 23/09/2026 (EST-23): aquel fichero
// tenia 1167 lineas y lo cruzan seis paquetes del plan (C-04, C-17, D-08,
// E-04, G-12, G-14) — de los sitios donde dos agentes se pisan sin
// enterarse. Se movio, no se reescribio: ni un cuerpo tocado.
//
// El estado sigue viviendo en GameSession (40 miembros). Sacarlo a una
// clase de datos es el otro medio relevo y va aparte, porque tocaria cada
// m_ de las mil lineas y el diff dejaria de poder leerse.


#include "Game/GameSession.h"

#include "RPG/InventoryEngine.h"  // G-12

#include "Level/ObjectCatalog.h"
#include "RPG/DicePoolEngine.h"
#include "Game/ComercioReputacion.h"  // AV14-23: el precio segun la faccion del tendero
#include "RPG/Definitions/DefinicionesObjetos.h"  // H2-08 — RPG::EquipmentDefinition::price
#include "RPG/Definitions/SkillDefinition.h"
#include "Runtime/RuntimeTileMap.h"

#include <algorithm>


// H2-08 — Precio de base de un objeto. Se define AQUI, y no en
// GameSession.cpp como estaba, porque su unico consumidor es este fichero:
// las dos mitades del comercio (la oferta que abre la tienda y el pago de
// una venta) lo llaman y las dos estan aqui.
//
// El precio de una pieza de EQUIPO es el del CATALOGO REAL
// (assets/catalogs/equipment.json: 413 piezas con precio generado), no el
// numero que el nivel escriba a mano en el objeto. No es un detalle de
// tienda: 282 de esas piezas llegan al jugador por vias que no pasan por el
// catalogo de objetos —el botin de los monstruos (H2-05), el equipo inicial
// de las clases, los personajes pregenerados— y en el objeto no hay
// `pickup.price`: valen 0, y 0 es «eso no tiene precio aqui», o sea, botin
// que no se puede vender. El precio del objeto del nivel queda como plan B
// para lo que no sea equipo.
int GameSession::basePriceOf(const std::string& objectId) const {
    if (const RPG::EquipmentDefinition* eq = equipmentFor(objectId)) {
        if (eq->price > 0) return eq->price;
    }
    const ObjectDefinition* def = sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(objectId) : nullptr;
    return def != nullptr ? def->pickup.price : 0;
}


void GameSession::resolveShopOffers(const ObjectDefinition& def) {
    // Oferta resuelta AHORA y no en cada consulta del HUD: el precio de
    // cada articulo puede venir de la tienda o del catalogo, y hacer esa
    // resolucion una vez al abrir evita que el HUD tenga que conocer esa
    // regla. Los articulos sin precio en ninguno de los dos sitios se
    // descartan: un articulo a coste 0 seria oro infinito al revenderlo.
    sessionData()->m_shopOffers.clear();
    // AV14-23 (D-MOR-09): lo que la faccion del tendero piensa de ti cambia
    // el precio (y, si te detestan, lo que te paga). Se fija al abrir, como
    // el resto de la oferta.
    const RPG::CharacterSheet* ficha =
        sessionData()->m_playerBody != nullptr ? sessionData()->m_playerBody->character_sheet() : nullptr;
    const int ajuste = Game::ajustePrecioPct(Game::facciones(), ficha, def.id);
    sessionData()->m_shopBuybackPercent = Game::recompraConAjuste(def.shop.buybackPercent, ajuste);
    for (const ShopItem& item : def.shop.items) {
        const ObjectDefinition* itemDef =
            sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(item.objectId) : nullptr;
        if (itemDef == nullptr) {
            logLine("(articulo desconocido en la tienda: " + item.objectId + ")");
            continue;
        }
        // El precio del PNJ manda; si no lo pone, el de base: que ya
        // consulta el catalogo real de equipo y solo cae al
        // `pickup.price` del objeto para lo que no sea equipo.
        const int price = Game::precioConAjuste(item.price > 0 ? item.price : basePriceOf(item.objectId), ajuste);
        if (price <= 0) {
            continue;
        }
        sessionData()->m_shopOffers.push_back(ShopOffer{item.objectId, itemDef->name, price});
    }
}

Result<bool> GameSession::openShop(const std::string& shopObjectId) {
    if (sessionData()->m_mode != GameMode::Exploration) {
        return Result<bool>::Error("no se puede abrir la tienda '" + shopObjectId +
                                   "': la partida no esta explorando (hay un dialogo, un combate u otra "
                                   "tienda abiertos)");
    }
    const ObjectDefinition* def = sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(shopObjectId) : nullptr;
    if (def == nullptr) {
        return Result<bool>::Error("no existe '" + shopObjectId + "' en el catalogo de objetos");
    }
    if (def->category != ObjectCategory::Npc) {
        return Result<bool>::Error("'" + shopObjectId +
                                   "' no es un personaje: solo un PNJ puede tener tienda");
    }
    resolveShopOffers(*def);
    if (sessionData()->m_shopOffers.empty()) {
        return Result<bool>::Error("'" + shopObjectId +
                                   "' no tiene nada a la venta (tienda vacia o sin precios)");
    }
    sessionData()->m_dialogueSpeaker = def->name;
    sessionData()->m_dialogueLines.clear();
    sessionData()->m_mode = GameMode::Shop;
    logLine("Tienda de " + def->name + ".");
    return Result<bool>::Ok(true);
}


int GameSession::occupancyPercent(int rentPercent) {
    // Por debajo del alquiler justo se llena del todo (nadie se va
    // porque le cobres poco). Por encima, se pierden 0.4 puntos de
    // ocupacion por cada punto de subida: a 200% queda el 60%.
    //
    // El 0.4 no es arbitrario: con 0.5 exacto, ingreso = renta x
    // ocupacion daba EXACTAMENTE lo mismo a 200% que a 100%, asi que
    // exprimir a los inquilinos no aportaba ni un oro y solo costaba
    // moral -- una decision sin dilema, que es una decision muerta. Con
    // 0.4 la curva tiene un optimo economico alrededor del 175%: subir
    // hasta ahi RENTA de verdad (y por eso tienta), pasarse ya no.
    if (rentPercent <= 100) {
        return 100;
    }
    return std::max(0, 100 - (rentPercent - 100) * 2 / 5);
}

int GameSession::incomeFor(int baseIncome, int rentPercent) {
    // El producto se hace en un solo paso y con division final para no
    // perder precision por redondeos intermedios (a estos numeros
    // importa: 12 * 150 / 100 * 75 / 100 no es lo mismo segun el orden).
    const long long value =
        static_cast<long long>(baseIncome) * rentPercent * occupancyPercent(rentPercent);
    return static_cast<int>(value / 10000);
}

bool GameSession::ownsBusiness(const std::string& objectId) const {
    for (const OwnedBusiness& b : sessionData()->m_businesses) {
        if (b.objectId == objectId) {
            return true;
        }
    }
    return false;
}

bool GameSession::buyCurrentBusiness() {
    if (sessionData()->m_mode != GameMode::Business || sessionData()->m_currentBusinessId.empty()) {
        return false;
    }
    if (ownsBusiness(sessionData()->m_currentBusinessId)) {
        logLine("Ya es tuyo.");
        return false;
    }
    if (sessionData()->m_gold < sessionData()->m_currentBusinessPrice) {
        logLine("Te faltan " + std::to_string(sessionData()->m_currentBusinessPrice - sessionData()->m_gold) +
                " oro para comprar " + sessionData()->m_currentBusinessName + ".");
        return false;
    }
    sessionData()->m_gold -= sessionData()->m_currentBusinessPrice;
    sessionData()->m_businesses.push_back(
        OwnedBusiness{sessionData()->m_currentBusinessId, sessionData()->m_currentBusinessName, sessionData()->m_currentBusinessIncome, 100});
    logLine("Compras " + sessionData()->m_currentBusinessName + " por " + std::to_string(sessionData()->m_currentBusinessPrice) +
            " oro.");
    return true;
}

// AV15-05a (EH-30) — Lo comprado por error se revende: se devuelve `percent` %
// del precio (Game::NegocioEstado::kReventaPct) y deja de rentar.
int GameSession::sellCurrentBusiness(int percent) {
    if (sessionData()->m_mode != GameMode::Business) return -1;
    auto& tuyos = sessionData()->m_businesses;
    const auto it = std::find_if(tuyos.begin(), tuyos.end(), [&](const OwnedBusiness& b) {
        return b.objectId == sessionData()->m_currentBusinessId;
    });
    if (it == tuyos.end()) return -1;
    const int recibes = std::max(0, sessionData()->m_currentBusinessPrice * percent / 100);
    tuyos.erase(it);
    sessionData()->m_gold += recibes;
    logLine("Revendes " + sessionData()->m_currentBusinessName + " por " + std::to_string(recibes) + " oro.");
    return recibes;
}

bool GameSession::adjustCurrentRent(int deltaPercent) {
    if (sessionData()->m_mode != GameMode::Business || sessionData()->m_currentBusinessId.empty()) {
        return false;
    }
    for (OwnedBusiness& b : sessionData()->m_businesses) {
        if (b.objectId != sessionData()->m_currentBusinessId) {
            continue;
        }
        const int before = b.rentPercent;
        b.rentPercent = std::clamp(b.rentPercent + deltaPercent, kMinRent, kMaxRent);
        const int applied = b.rentPercent - before;
        if (applied == 0) {
            return false;  // ya estaba en el tope
        }

        // La moral se mueve con el CAMBIO, no con el nivel absoluto: si
        // no, bastaria con subir y bajar en bucle para farmear
        // reputacion. Bajar el alquiler te acerca a heroe, subirlo a
        // villano, uno a uno por cada 10 puntos.
        // Un punto de moral por cada 5 de alquiler: con /10 el rango
        // entero (50%..200%) apenas movia 15 puntos de una escala de
        // 200, y el sistema no se notaba.
        const int moral = -applied / 5;
        const std::string que = applied > 0 ? "Subes" : "Bajas";
        sessionData()->m_morality.record(moral, que + " el alquiler de " + b.name + " al " +
                                     std::to_string(b.rentPercent) + "%");
        logLine(que + " el alquiler de " + b.name + " al " + std::to_string(b.rentPercent) +
                "% (ocupacion " + std::to_string(occupancyPercent(b.rentPercent)) + "%).");
        return true;
    }
    logLine("Ese negocio no es tuyo.");
    return false;
}

int GameSession::collectRent() {
    int total = 0;
    for (const OwnedBusiness& b : sessionData()->m_businesses) {
        total += incomeFor(b.baseIncome, b.rentPercent);

        // La moral tambien depende de lo que SOSTIENES, no solo de lo
        // que cambias: cada cobro con alquiler abusivo resta, y cada
        // cobro con alquiler generoso suma. Sin esto, el jugador podia
        // subir la renta, comerse el golpe de moral una vez y cobrar
        // caro para siempre a coste cero; ahora mantener un alquiler
        // injusto tiene un precio recurrente.
        const int sostenido = (100 - b.rentPercent) / 25;
        if (sostenido != 0) {
            sessionData()->m_morality.record(
                sostenido,
                (sostenido > 0 ? "Alquiler generoso en " : "Alquiler abusivo en ") + b.name);
        }
    }
    if (total > 0) {
        sessionData()->m_gold += total;
        logLine("Rentas cobradas: +" + std::to_string(total) + " oro.");
    }
    return total;
}


bool GameSession::buy(std::size_t index) {
    if (sessionData()->m_mode != GameMode::Shop || index >= sessionData()->m_shopOffers.size()) {
        return false;
    }
    const ShopOffer& offer = sessionData()->m_shopOffers[index];
    if (sessionData()->m_gold < offer.price) {
        logLine("No te llega para " + offer.name + " (" + std::to_string(offer.price) + " oro).");
        return false;
    }
    // Cobrar y entregar en el mismo paso: sin estados intermedios donde
    // el oro ya se fue pero el objeto no ha llegado.
    sessionData()->m_gold -= offer.price;
    if (sessionData()->m_inventory != nullptr) {
        sessionData()->m_inventory->push_back(offer.objectId);
    }
    logLine("Compras " + offer.name + " por " + std::to_string(offer.price) + " oro.");
    return true;
}

// TIENDA-02 — Lo que paga el tendero abierto por un objeto: su porcentaje de
// recompra (fijado al abrir) del precio de base, y nunca menos de 1. 0 = no
// lo compra (sin precio: una llave de mision). La hoja de la tienda lo
// ensena antes de vender y sell() cobra exactamente esto.
int GameSession::sellPriceOf(const std::string& objectId) const {
    const int base = basePriceOf(objectId);
    if (base <= 0) return 0;
    return std::max(1, base * sessionData()->m_shopBuybackPercent / 100);
}

bool GameSession::sell(std::size_t index) {
    if (sessionData()->m_mode != GameMode::Shop || sessionData()->m_inventory == nullptr || index >= sessionData()->m_inventory->size()) {
        return false;
    }
    const std::string objectId = (*sessionData()->m_inventory)[index];
    const int paid = sellPriceOf(objectId);
    if (paid <= 0) {
        logLine("Eso no tiene precio aqui.");
        return false;
    }
    sessionData()->m_gold += paid;
    const ObjectDefinition* def = sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(objectId) : nullptr;
    logLine("Vendes " + (def != nullptr ? def->name : objectId) + " por " + std::to_string(paid) +
            " oro.");
    sessionData()->m_inventory->erase(sessionData()->m_inventory->begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

