// H3-08 — la política de DEC-09 hecha código. Ver la cabecera para las reglas;
// aquí solo están los motivos en texto, que siguen el idioma de SaveGate::reason
// (español) porque son el mismo contrato: lo que la máquina le dice al jugador
// por escrito. Las ETIQUETAS del menú siguen en catalán, que es de donde son
// (GameOverScreen, GDB-12.4); unificar los dos idiomas del motor es H6-04.

#include "Game/DefeatRecovery.h"

#include <string>
#include <vector>

namespace Game {
namespace {

std::string nombreRanura(const SaveSlot& slot) {
    if (slot.index == SaveSlots::kQuickSlot) {
        return "guardado rapido";
    }
    return "ranura " + std::to_string(slot.index);
}

std::string estadoRoto(const SaveSlot& slot) {
    // Corrupt e Incompatible se distinguen a proposito (SaveSlotState.h): «tu
    // partida esta rota» y «tu partida es de otra version» llevan a acciones
    // distintas, y aqui tambien.
    const std::string base = (slot.state == SlotState::Incompatible)
                                 ? "es de otra version del formato"
                                 : "no se puede leer como partida";
    return nombreRanura(slot) + ": " + base +
           (slot.reason.empty() ? std::string() : " (" + slot.reason + ")");
}

std::string resumen(const SaveSlot& slot) {
    std::string texto = nombreRanura(slot);
    if (!slot.savedAt.empty()) {
        texto += " del " + slot.savedAt;
    }
    if (!slot.characterName.empty()) {
        texto += ", " + slot.characterName;
    }
    return texto;
}

}  // namespace

RecoveryPlan DefeatRecovery::plan(const std::vector<SaveSlot>& numbered, const SaveSlot& quick) {
    RecoveryPlan plan;

    // La rapida va PRIMERA a proposito: a igualdad de savedAt gana quien esta
    // delante, y la regla 3 de la cabecera dice que el desempate es la rapida.
    std::vector<const SaveSlot*> candidatas;
    candidatas.push_back(&quick);
    for (const SaveSlot& slot : numbered) {
        candidatas.push_back(&slot);
    }

    const SaveSlot* elegida = nullptr;
    for (const SaveSlot* slot : candidatas) {
        if (!slot->loadable()) {
            // Empty no es un descarte: es una ranura libre, y llenar refused con
            // las cinco ranuras vacias del proyecto no le dice nada a nadie.
            if (slot->occupied()) {
                plan.refused.push_back(estadoRoto(*slot));
            }
            continue;
        }
        if (elegida == nullptr || slot->savedAt > elegida->savedAt) {
            elegida = slot;
        }
    }

    if (elegida == nullptr) {
        plan.action = RecoveryAction::Title;
        plan.reason = plan.refused.empty()
                          ? "no hay ninguna partida guardada: se vuelve al titulo"
                          : "ninguna partida guardada se puede cargar: se vuelve al titulo";
        return plan;
    }

    // Regla 4: una ranura legible sin fecha se puede elegir si no hay otra, pero
    // las que pierden por eso se dicen, no desaparecen.
    for (const SaveSlot* slot : candidatas) {
        if (slot != elegida && slot->loadable() && slot->savedAt.empty()) {
            plan.refused.push_back(nombreRanura(*slot) +
                                   ": legible pero sin fecha, se trata como la mas antigua");
        }
    }

    plan.action = RecoveryAction::LoadSlot;
    plan.slot = elegida->index;
    plan.levelPath = elegida->levelPath;
    plan.characterName = elegida->characterName;
    plan.savedAt = elegida->savedAt;
    // DEC-09: se vuelve al ULTIMO PUNTO SEGURO, no a la mitad del combate. El
    // texto lo dice tal cual porque lo pinta la pantalla de derrota.
    plan.reason = "vuelves al ultimo punto seguro: " + resumen(*elegida);
    return plan;
}

RecoveryPlan DefeatRecovery::plan(const SaveSlots& slots) {
    return plan(slots.slots(), slots.quick());
}

}  // namespace Game
