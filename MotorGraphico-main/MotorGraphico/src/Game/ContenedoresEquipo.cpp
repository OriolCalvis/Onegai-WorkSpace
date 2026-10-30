#include "Game/ContenedoresEquipo.h"
#include "Game/CartasEnCombate.h"
#include "RPG/CharacterSheet.h"
#include "Render/ICombatant.h"
#include <algorithm>

namespace Game {
namespace {
// La construcción copia las cartas a la mano. Restaurar también ante una
// excepción evita que una carta prestada se aprenda o llegue a SaveGame.
struct RestauraCartas {
    std::vector<std::string>& destino;
    std::vector<std::string> antes;
    ~RestauraCartas() { destino.swap(antes); }
};
}
std::unique_ptr<CartasEnCombate> mesaConBotin(const CatalogoCartas& cartas, ICombatant& heroe,
                                            const RPG::Catalogs::EquipmentCatalog* equipo) {
    RPG::CharacterSheet* ficha = heroe.character_sheet();
    if (ficha == nullptr || equipo == nullptr) return CartasEnCombate::paraHeroe(cartas, heroe);
    RestauraCartas restaura{ficha->knownSkillIds, ficha->knownSkillIds};
    for (const std::string& id : ficha->equippedIds) {
        const auto* pieza = equipo->find(id);
        if (pieza == nullptr || pieza->linkedSkillId.empty() ||
            std::find(pieza->grantedTags.begin(), pieza->grantedTags.end(), "BotinBoundington") == pieza->grantedTags.end()) continue;
        auto& mano = ficha->knownSkillIds;
        if (std::find(mano.begin(), mano.end(), pieza->linkedSkillId) == mano.end()) mano.push_back(pieza->linkedSkillId);
    }
    return CartasEnCombate::paraHeroe(cartas, heroe);
}
}
