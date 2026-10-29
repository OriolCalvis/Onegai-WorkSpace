// Usar, equipar y desequipar, con el motivo por delante.
//
// Cada operacion tiene su whyCannot* que responde ANTES de intentarlo:
// el HUD necesita explicar por que un boton esta apagado, y eso no se
// puede saber a base de fallar. Por eso la razon es parte de la API.
//
// Trae consigo su namespace anonimo de ayudantes (NombreRanura,
// Puestos): solo lo usa el equipo, y partirlo fue justo el fallo que
// el compilador canto al primer intento de este corte.
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
#include "RPG/Definitions/SkillDefinition.h"
#include "Runtime/RuntimeTileMap.h"

#include <algorithm>


namespace {

const char* NombreRanura(RPG::EquipSlot slot) {
    switch (slot) {
        case RPG::EquipSlot::Head:      return "la cabeza";
        case RPG::EquipSlot::Torso:     return "el torso";
        case RPG::EquipSlot::Legs:      return "las piernas";
        case RPG::EquipSlot::Feet:      return "los pies";
        case RPG::EquipSlot::MainHand:  return "la mano principal";
        case RPG::EquipSlot::OffHand:   return "la otra mano";
        case RPG::EquipSlot::Accessory: return "el anillo";
        case RPG::EquipSlot::Arms:      return "los brazos";
        case RPG::EquipSlot::Neck:      return "el cuello";
        case RPG::EquipSlot::Belt:      return "la cintura";
        case RPG::EquipSlot::Ring2:     return "el otro anillo";
        default:                        return "esa ranura";
    }
}

// TUTORIAL-01 — Lo que el equipo sube o baja del tope de vida se nota en la de ahora: el dano que llevas no cambia.
// Ponerse el kit del cofre no deja al heroe herido (5/9 al empezar) y quitarse la armadura no le deja por
// encima del tope (35/29). Quitarse algo no mata: como poco, 1.
void SigueAlTope(ICombatant& cuerpo, int topeAntes) {
    const int tope = cuerpo.maxHealth();
    if (tope == topeAntes || cuerpo.health() <= 0) return;
    int quiere = tope - (topeAntes - cuerpo.health());
    if (quiere < 1) quiere = 1;
    if (quiere > tope) quiere = tope;
    if (quiere > cuerpo.health()) cuerpo.heal(quiere - cuerpo.health());
    else if (quiere < cuerpo.health()) cuerpo.takeDamage(cuerpo.health() - quiere);
}

// Ids distintos puestos en la ficha (una pieza a dos manos ocupa dos
// ranuras con el mismo id y cuenta una vez).
std::vector<std::string> Puestos(const RPG::CharacterSheet& sheet) {
    std::vector<std::string> ids;
    for (const std::string& id : sheet.equippedIds) {
        if (!id.empty() && std::find(ids.begin(), ids.end(), id) == ids.end()) ids.push_back(id);
    }
    return ids;
}

int ArmaduraDe(const RPG::CharacterSheet& sheet) {
    return sheet.equipArmorBonus + sheet.equipShieldBonus;
}

}  // namespace

std::string GameSession::equipmentIdFor(const std::string& objectId) {
    static const std::string kPrefijo = "item_";
    if (objectId.size() > kPrefijo.size() && objectId.compare(0, kPrefijo.size(), kPrefijo) == 0) {
        return objectId.substr(kPrefijo.size());
    }
    return objectId;
}

std::string GameSession::objectIdForEquipment(const std::string& equipmentId) const {
    // El camino de vuelta: si el catalogo de objetos tiene "item_<x>", ese
    // es el objeto; si no, el propio id (piezas definidas sin prefijo).
    const std::string conPrefijo = "item_" + equipmentId;
    if (sessionData()->m_catalog != nullptr && sessionData()->m_catalog->find(conPrefijo) != nullptr) return conPrefijo;
    return equipmentId;
}

const RPG::EquipmentDefinition* GameSession::equipmentFor(const std::string& objectId) const {
    const RPG::Catalogs::EquipmentCatalog* cat = sessionData()->m_inventoryRules.equipment;
    if (cat == nullptr) return nullptr;
    if (const RPG::EquipmentDefinition* eq = cat->find(equipmentIdFor(objectId))) return eq;
    return cat->find(objectId);
}

std::string GameSession::inventoryBusyReason() const {
    switch (sessionData()->m_mode) {
        case GameMode::Exploration:
        case GameMode::Pause:
            return "";
        case GameMode::Battle:
            return "en combate los objetos se usan desde el menu de combate";
        case GameMode::GameOver:
            return "la partida ha terminado";
        default:
            return "cierra primero la conversacion o la tienda";
    }
}

std::string GameSession::whyCannotUse(const std::string& itemId) const {
    const std::string ocupada = inventoryBusyReason();
    if (!ocupada.empty()) return ocupada;
    if (sessionData()->m_inventory == nullptr ||
        std::find(sessionData()->m_inventory->begin(), sessionData()->m_inventory->end(), itemId) == sessionData()->m_inventory->end()) {
        return "no llevas '" + itemId + "'";
    }
    const ObjectDefinition* def = sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(itemId) : nullptr;
    const std::string nombre = def != nullptr ? def->name : itemId;
    if (equipmentFor(itemId) != nullptr) return nombre + " no se usa: se equipa";
    if (def == nullptr || def->category != ObjectCategory::Pickup ||
        def->pickup.effect == PickupEffect::None) {
        return nombre + " no tiene ningun efecto que usar";
    }
    if (def->pickup.effect == PickupEffect::Heal) {
        if (sessionData()->m_playerBody == nullptr) return "no hay personaje al que curar";
        if (sessionData()->m_playerBody->health() >= sessionData()->m_playerBody->maxHealth()) {
            return "ya tienes la vida al maximo";
        }
    } else {
        if (sessionData()->m_playerSkills == nullptr) return "el personaje no usa mana";
        if (sessionData()->m_playerSkills->mana() >= sessionData()->m_playerSkills->maxMana()) {
            return "ya tienes el mana al maximo";
        }
    }
    return "";
}

Result<bool> GameSession::useItem(const std::string& itemId) {
    const std::string motivo = whyCannotUse(itemId);
    if (!motivo.empty()) return Result<bool>::Error(motivo);
    const ObjectDefinition* def = sessionData()->m_catalog->find(itemId);
    if (def->pickup.effect == PickupEffect::Heal) {
        const int antes = sessionData()->m_playerBody->health();
        sessionData()->m_playerBody->heal(def->pickup.power);
        logLine("Usas " + def->name + " (+" + std::to_string(sessionData()->m_playerBody->health() - antes) +
                " PV).");
    } else {
        const int antes = sessionData()->m_playerSkills->mana();
        sessionData()->m_playerSkills->restoreMana(def->pickup.power);
        logLine("Usas " + def->name + " (+" + std::to_string(sessionData()->m_playerSkills->mana() - antes) +
                " PM).");
    }
    // UNA unidad: dos pociones son dos entradas (mismo criterio que el combate).
    sessionData()->m_inventory->erase(std::find(sessionData()->m_inventory->begin(), sessionData()->m_inventory->end(), itemId));
    return Result<bool>::Ok(true);
}

Result<std::vector<std::string>> GameSession::equipOnSheet(
        RPG::CharacterSheet& sheet, const RPG::EquipmentDefinition& eq) const {
    using R = Result<std::vector<std::string>>;
    static const RPG::ClassDefinition kSinClase;
    static const RPG::TierRules kReglas;
    const RPG::ClassDefinition& clase =
        sessionData()->m_inventoryRules.playerClass != nullptr ? *sessionData()->m_inventoryRules.playerClass : kSinClase;
    const RPG::TierRules& reglas = sessionData()->m_inventoryRules.tiers != nullptr ? *sessionData()->m_inventoryRules.tiers : kReglas;
    const RPG::Catalogs::EquipmentCatalog& cat = *sessionData()->m_inventoryRules.equipment;

    const std::vector<std::string> antes = Puestos(sheet);
    // InventoryEngine saca la pieza de la mochila de la FICHA y mete ahi
    // lo que desplaza. La mochila de verdad es la de la sesion, asi que la
    // ficha se usa solo de paso: se mete la pieza para que la saque, y lo
    // desplazado se retira de ella para devolverlo a la sesion.
    RPG::InventoryEngine::add_to_inventory(sheet, eq.id, 1, false);
    const Result<bool> puesto = RPG::InventoryEngine::equip(sheet, eq, clase, reglas, cat);
    if (!puesto.isOk()) {
        (void)RPG::InventoryEngine::remove_from_inventory(sheet, eq.id, 1, false);
        return R::Error(eq.name + ": " + puesto.errorMessage());
    }
    std::vector<std::string> desplazados;
    for (const std::string& id : antes) {
        const std::vector<std::string> ahora = Puestos(sheet);
        if (std::find(ahora.begin(), ahora.end(), id) != ahora.end()) continue;
        (void)RPG::InventoryEngine::remove_from_inventory(sheet, id, 1, false);
        desplazados.push_back(id);
    }
    return R::Ok(std::move(desplazados));
}

std::string GameSession::whyCannotEquip(const std::string& itemId) const {
    const std::string ocupada = inventoryBusyReason();
    if (!ocupada.empty()) return ocupada;
    if (sessionData()->m_inventory == nullptr ||
        std::find(sessionData()->m_inventory->begin(), sessionData()->m_inventory->end(), itemId) == sessionData()->m_inventory->end()) {
        return "no llevas '" + itemId + "'";
    }
    if (sessionData()->m_inventoryRules.equipment == nullptr) return "no hay catalogo de equipo cargado";
    const RPG::EquipmentDefinition* eq = equipmentFor(itemId);
    if (eq == nullptr) return "'" + itemId + "' no es una pieza de equipo";
    const RPG::CharacterSheet* ficha = characterSheet();
    if (ficha == nullptr) return "el personaje no tiene ficha donde equiparlo";
    // Se prueba sobre una copia: el motivo sale de las mismas reglas que
    // equipItem, sin repetirlas aqui.
    RPG::CharacterSheet copia = *ficha;
    const auto r = equipOnSheet(copia, *eq);
    return r.isOk() ? "" : r.errorMessage();
}

Result<bool> GameSession::equipItem(const std::string& itemId) {
    const std::string motivo = whyCannotEquip(itemId);
    if (!motivo.empty()) return Result<bool>::Error(motivo);
    RPG::CharacterSheet* ficha = sessionData()->m_playerBody->character_sheet();
    const RPG::EquipmentDefinition* eq = equipmentFor(itemId);
    const int topeAntes = sessionData()->m_playerBody->maxHealth();
    const auto desplazados = equipOnSheet(*ficha, *eq);
    if (!desplazados.isOk()) return Result<bool>::Error(desplazados.errorMessage());
    SigueAlTope(*sessionData()->m_playerBody, topeAntes);  // TUTORIAL-01
    sessionData()->m_inventory->erase(std::find(sessionData()->m_inventory->begin(), sessionData()->m_inventory->end(), itemId));
    for (const std::string& id : desplazados.value()) {
        sessionData()->m_inventory->push_back(objectIdForEquipment(id));
    }
    logLine("Te equipas " + eq->name + ".");
    return Result<bool>::Ok(true);
}

std::string GameSession::whyCannotUnequip(RPG::EquipSlot slot) const {
    const std::string ocupada = inventoryBusyReason();
    if (!ocupada.empty()) return ocupada;
    if (sessionData()->m_inventoryRules.equipment == nullptr) return "no hay catalogo de equipo cargado";
    if (sessionData()->m_inventory == nullptr) return "no hay mochila donde guardarlo";
    const RPG::CharacterSheet* ficha = characterSheet();
    if (ficha == nullptr) return "el personaje no tiene ficha";
    const std::size_t i = static_cast<std::size_t>(slot);
    if (i >= ficha->equippedIds.size() || ficha->equippedIds[i].empty()) {
        return std::string("no llevas nada en ") + NombreRanura(slot);
    }
    return "";
}

Result<bool> GameSession::unequipItem(RPG::EquipSlot slot) {
    const std::string motivo = whyCannotUnequip(slot);
    if (!motivo.empty()) return Result<bool>::Error(motivo);
    RPG::CharacterSheet* ficha = sessionData()->m_playerBody->character_sheet();
    const int topeAntes = sessionData()->m_playerBody->maxHealth();
    std::string quitado;
    const Result<bool> r =
        RPG::InventoryEngine::unequip(*ficha, slot, *sessionData()->m_inventoryRules.equipment, &quitado);
    if (!r.isOk()) return r;
    (void)RPG::InventoryEngine::remove_from_inventory(*ficha, quitado, 1, false);
    // unequip() no refresca la ficha (equip() si): sin esto, el tope de
    // vida y las defensas seguirian contando la pieza que ya no llevas.
    static const RPG::TierRules kReglas;
    ficha->recompute_derived(sessionData()->m_inventoryRules.tiers != nullptr ? *sessionData()->m_inventoryRules.tiers : kReglas,
                             sessionData()->m_inventoryRules.playerClass);
    SigueAlTope(*sessionData()->m_playerBody, topeAntes);  // TUTORIAL-01
    sessionData()->m_inventory->push_back(objectIdForEquipment(quitado));
    const RPG::EquipmentDefinition* eq = sessionData()->m_inventoryRules.equipment->find(quitado);
    logLine("Te quitas " + (eq != nullptr ? eq->name : quitado) + ".");
    return Result<bool>::Ok(true);
}

Result<GameSession::EquipPreview> GameSession::previewEquip(const std::string& itemId) const {
    using R = Result<EquipPreview>;
    const std::string motivo = whyCannotEquip(itemId);
    if (!motivo.empty()) return R::Error(motivo);
    static const RPG::TierRules kReglas;
    const RPG::TierRules& reglas = sessionData()->m_inventoryRules.tiers != nullptr ? *sessionData()->m_inventoryRules.tiers : kReglas;
    const RPG::CharacterSheet& ficha = *characterSheet();
    RPG::CharacterSheet antes = ficha;
    antes.recompute_derived(reglas, sessionData()->m_inventoryRules.playerClass);
    RPG::CharacterSheet despues = ficha;
    const auto desplazados = equipOnSheet(despues, *equipmentFor(itemId));
    if (!desplazados.isOk()) return R::Error(desplazados.errorMessage());
    despues.recompute_derived(reglas, sessionData()->m_inventoryRules.playerClass);
    EquipPreview p;
    for (int i = 0; i < 4; ++i) {
        p.statsBefore[i] = antes.stat(static_cast<RPG::Stat>(i));
        p.statsAfter[i] = despues.stat(static_cast<RPG::Stat>(i));
    }
    p.armorBefore = ArmaduraDe(antes);
    p.armorAfter = ArmaduraDe(despues);
    p.healthCapBefore = antes.healthCap();
    p.healthCapAfter = despues.healthCap();
    for (const std::string& id : desplazados.value()) p.displaced.push_back(objectIdForEquipment(id));
    return R::Ok(std::move(p));
}
