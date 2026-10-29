// UI-BG3-01 — ver include/Game/CofreKit.h.
#include "Game/CofreKit.h"

#include <algorithm>

#include "RPG/Narrative/NarrativeState.h"
#include "World/World.h"

namespace Game {

namespace {

std::string ranuraLegible(const RPG::EquipmentDefinition& e) {
    if (e.twoHanded || e.slot.find("Hand") != std::string::npos) return "Arma";
    if (e.slot == "Head") return "Cabeza";
    if (e.slot == "Torso") return "Torso";
    if (e.slot == "Legs") return "Piernas";
    if (e.slot == "Feet") return "Pies";
    if (e.slot == "Accessory") return "Accesorio";
    // PART-02: las ranuras nuevas
    if (e.slot == "Arms") return "Brazos";
    if (e.slot == "Neck") return "Collar";
    if (e.slot == "Belt") return "Cinturón";
    if (e.slot == "Ring" || e.slot == "Ring2") return "Anillo";
    return e.slot;
}

}  // namespace

Engine::EntityHandle CofreKit::alAlcance(const Engine::World& mundo, const GridCoord& p) {
    const int dx[] = {0, 0, 0, -1, 1};
    const int dy[] = {0, -1, 1, 0, 0};
    for (int i = 0; i < 5; ++i) {
        const Engine::EntityHandle h = mundo.findObjectAt(p.x + dx[i], p.y + dy[i]);
        const Engine::WorldEntity* e = mundo.get(h);
        if (e != nullptr && e->object.objectId == kObjeto) return h;
    }
    return Engine::EntityHandle{};
}

void CofreKit::abre(Engine::World& mundo, Engine::EntityHandle cofre, const std::vector<std::string>& piezas,
                    const std::vector<std::string>& objetos, const RPG::Catalogs::EquipmentCatalog& equipo,
                    const std::string& clase, std::vector<std::string>& mochila, RPG::NarrativeState& flags) {
    mochila.insert(mochila.end(), objetos.begin(), objetos.end());
    flags.setFlag(kRecogido);
    if (objetos.empty()) flags.setFlag(kPuesto);  // nada que ponerse: no se pide
    if (mundo.contains(cofre)) mundo.destroy(cofre);
    m_papiro = Papiro{};
    m_papiro.clase = clase;
    for (const std::string& id : piezas) {
        const RPG::EquipmentDefinition* e = equipo.find(id);
        if (e == nullptr) continue;
        Pieza p{e->name, ranuraLegible(*e), ""};  // AV16-02: las prendas del kit no dicen que sumen
        m_papiro.piezas.push_back(p);
    }
    m_abierto = true;
}

void CofreKit::revisa(Engine::World& mundo, const std::vector<std::string>& mochila,
                      const std::vector<std::string>& objetos, RPG::NarrativeState& flags) const {
    // ¿Hay cofre en el nivel? (y si ya se abrio, se quita: el nivel se carga
    // del JSON y lo vuelve a poner).
    Engine::EntityHandle cofre{};
    mundo.eachEntity([&](Engine::EntityHandle h, const Engine::WorldEntity& e) {
        if (e.object.objectId == kObjeto) cofre = h;
    });
    const bool recogido = flags.hasFlag(kRecogido);
    if (recogido && cofre.valid()) mundo.destroy(cofre);
    if (!recogido && !cofre.valid()) {
        flags.setFlag(kRecogido);
        flags.setFlag(kPuesto);
        return;
    }
    if (!recogido || flags.hasFlag(kPuesto)) return;
    const bool quedaAlguna = std::any_of(objetos.begin(), objetos.end(), [&](const std::string& o) {
        return std::find(mochila.begin(), mochila.end(), o) != mochila.end();
    });
    if (!quedaAlguna) flags.setFlag(kPuesto);
}

}  // namespace Game
