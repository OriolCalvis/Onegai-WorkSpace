// APARIENCIA-01 — ver include/Game/Personajes/Vestuario.h.
#include "Game/Personajes/Vestuario.h"

#include "RPG/InventoryEngine.h"

namespace Game {
namespace Vestuario {

std::vector<std::string> equiparInicial(RPG::CharacterSheet& ficha,
                                        const RPG::ClassDefinition& clase,
                                        const RPG::TierRules& tiers,
                                        const RPG::Catalogs::EquipmentCatalog& equipo) {
    // equip() saca el objeto de la mochila: se recorre una copia de los ids.
    std::vector<std::string> mochila;
    for (const auto& [id, cantidad] : ficha.inventoryEquipment) {
        if (cantidad > 0)
            mochila.push_back(id);
    }
    const auto libre = [&](RPG::EquipSlot ranura) {
        return ficha.equippedIds[static_cast<std::size_t>(ranura)].empty();
    };
    std::vector<std::string> puestos;
    for (const std::string& id : mochila) {
        const RPG::EquipmentDefinition* def = equipo.find(id);
        if (def == nullptr)
            continue;
        bool porNombre = false;
        const RPG::EquipSlot ranura = RPG::InventoryEngine::resolve_slot(def->slot, porNombre);
        const bool dosManos = porNombre || def->twoHanded;
        // PART-02: un anillo cabe si queda una mano libre (equip() lo pone en la otra)
        const bool cabe = dosManos ? libre(RPG::EquipSlot::MainHand) && libre(RPG::EquipSlot::OffHand)
                          : ranura == RPG::EquipSlot::Accessory
                              ? libre(RPG::EquipSlot::Accessory) || libre(RPG::EquipSlot::Ring2)
                              : libre(ranura);
        if (!cabe)
            continue;
        if (RPG::InventoryEngine::equip(ficha, *def, clase, tiers, equipo).isOk())
            puestos.push_back(id);
    }
    return puestos;
}

std::map<std::string, std::string> piezasVisibles(const RPG::EquipoPuesto& puestos,
                                                  const ManifiestoPersonajes& manifiesto) {
    std::map<std::string, std::string> visibles;
    for (const std::string& id : puestos) {
        if (id.empty())
            continue;
        const auto it = manifiesto.piezaDeObjeto.find(id);
        if (it != manifiesto.piezaDeObjeto.end())
            visibles[it->second.ranura] = it->second.visible();  // con su tinte (ESTETICA-01)
    }
    return visibles;
}

std::string sitioDeAbalorio(RPG::EquipSlot ranura, const std::string& suyo) {
    switch (ranura) {
        case RPG::EquipSlot::Accessory:
        case RPG::EquipSlot::MainHand: return "mano";
        case RPG::EquipSlot::Ring2:
        case RPG::EquipSlot::OffHand: return "otraMano";
        case RPG::EquipSlot::Neck: return "pecho";
        case RPG::EquipSlot::Belt: return "cintura";
        default: return suyo;
    }
}

std::map<std::string, BrilloDePieza> brillosVisibles(const RPG::EquipoPuesto& puestos,
                                                     const ManifiestoPersonajes& manifiesto) {
    std::map<std::string, BrilloDePieza> brillos;
    for (std::size_t i = 0; i < puestos.size(); ++i) {
        const std::string& id = puestos[i];
        if (id.empty())
            continue;
        const auto it = manifiesto.piezaDeObjeto.find(id);
        if (it != manifiesto.piezaDeObjeto.end() && it->second.brillo.nivel > 0)
            brillos[it->second.ranura] = it->second.brillo;
        // Un abalorio no tiene ranura visual: va con su sitio y su id (PART-02b: echa desde la mano que
        // lleva el anillo, el pecho o la cintura; dos anillos iguales, uno en cada mano, son dos).
        const auto ab = manifiesto.brilloDeAbalorio.find(id);
        if (ab != manifiesto.brilloDeAbalorio.end()) {
            BrilloDePieza b = ab->second;
            b.sitio = sitioDeAbalorio(static_cast<RPG::EquipSlot>(i), b.sitio);
            brillos["abalorio:" + (b.sitio.empty() ? std::string() : b.sitio + ":") + id] = b;
        }
    }
    return brillos;
}

RPG::EquipoPuesto probar(const RPG::CharacterCreation& reglas, const RPG::CreationChoice& eleccion,
                         const RPG::TierRules& tiers, const RPG::Catalogs::EquipmentCatalog& equipo) {
    const RPG::ClassDefinition* clase = reglas.classes().find(eleccion.classId);
    if (clase == nullptr)
        return {};
    RPG::CharacterSheet prueba;
    prueba.tier = eleccion.tier;
    prueba.classId = eleccion.classId;
    prueba.baseStats = eleccion.baseStats;
    // AV16-02: la raza no suma al empezar; la prueba usa la misma ficha que la real.
    for (const std::string& id : reglas.equipoInicial(eleccion.classId, eleccion.backgroundId)) {
        prueba.inventoryEquipment.emplace_back(id, 1);
    }
    equiparInicial(prueba, *clase, tiers, equipo);
    return prueba.equippedIds;
}

}  // namespace Vestuario
}  // namespace Game
