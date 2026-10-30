// ROSITA-01 — Ver include/Game/Refuerzos.h.
#include "Game/Refuerzos.h"

#include "Game/Acompanante.h"
#include "Game/BattleState.h"
#include "Game/CartasEnCombate.h"
#include "RPG/Narrative/AdventureScript.h"
#include "RPG/Narrative/NarrativeState.h"
#include "Render/ICombatant.h"

namespace Game {

namespace {

// Vivo y a `umbral` de su vida o menos (umbral 0: no se mira).
bool bajo(const BattleParticipant* quien, float umbral) {
    if (umbral <= 0.0f || quien == nullptr || quien->combatant == nullptr) return false;
    const ICombatant& x = *quien->combatant;
    return x.isAlive() && x.maxHealth() > 0 &&
           static_cast<float>(x.health()) <= umbral * static_cast<float>(x.maxHealth());
}

}  // namespace

bool tocaRefuerzo(const BattleState& batalla, const RPG::BattleRequest::Refuerzo& refuerzo,
                  const RPG::NarrativeState& trama) {
    if (batalla.outcome() != BattleOutcome::InProgress) return false;
    if (!refuerzo.requiere.isSatisfiedBy(trama)) return false;
    if (refuerzo.golpeMortal) return batalla.golpeMortalDado();  // AV13-34: con el golpe mortal, y solo con el
    const BattleParticipant* enemigo = batalla.enemies().empty() ? nullptr : &batalla.enemies().front();
    const BattleParticipant* heroe = batalla.allies().empty() ? nullptr : &batalla.allies().front();
    return bajo(enemigo, refuerzo.vidaEnemigo) || bajo(heroe, refuerzo.vidaHeroe);
}

int llegaRefuerzos(BattleState& batalla, RPG::BattleRequest& pedido, RPG::NarrativeState& trama,
                   const RPG::AdventureScript* aventura) {
    int llegados = 0;
    auto* mesa = dynamic_cast<CartasEnCombate*>(batalla.mesa());
    for (auto it = pedido.refuerzos.begin(); it != pedido.refuerzos.end();) {
        if (!tocaRefuerzo(batalla, *it, trama)) {
            ++it;
            continue;
        }
        for (const std::string& flag : it->flags) trama.setFlag(flag);
        for (const std::string& linea : it->lineas) batalla.anota(linea);
        std::string quienes;
        for (const RPG::BattleRequest::ConAcompanante& con : it->acompanantes) {
            const RPG::AcompananteDef* def = aventura != nullptr ? aventura->acompanante(con.id) : nullptr;
            if (def == nullptr || mesa == nullptr || (!con.siFlag.empty() && !trama.hasFlag(con.siFlag))) continue;
            const int usadas = trama.variable(Acompanante::claveCuras(aventura->id, def->id));
            batalla.sumaAliado({def->nombre, mesa->sumaAcompanante(*def, def->cura.veces - usadas), nullptr, nullptr});
            quienes += (quienes.empty() ? "" : ", ") + def->nombre;
        }
        if (!quienes.empty()) batalla.anota("Llegan a tu lado: " + quienes + ".");
        it = pedido.refuerzos.erase(it);
        ++llegados;
    }
    return llegados;
}

}  // namespace Game
