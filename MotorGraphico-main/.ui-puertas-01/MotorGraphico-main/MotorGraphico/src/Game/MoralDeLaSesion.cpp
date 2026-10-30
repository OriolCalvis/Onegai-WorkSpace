// MORAL-01 — Ver include/Game/MoralDeLaSesion.h.
#include "Game/MoralDeLaSesion.h"

#include <algorithm>

#include "Game/ReglasMoral.h"
#include "Game/SessionData.h"
#include "RPG/CharacterSheet.h"
#include "RPG/Narrative/FranjasDelDia.h"
#include "RPG/Narrative/NarrativeState.h"
#include "Render/ICombatant.h"

namespace moral {

namespace {

std::string conSigno(int v) { return (v > 0 ? "+" : v < 0 ? "−" : "") + std::to_string(v < 0 ? -v : v); }

// La noche que se asienta es la del día que acaba: si el beat ya ha pasado a
// la mañana siguiente, la del día anterior.
int diaQueAcaba(const SessionData& d) {
    if (d.m_narrativeState == nullptr) return 1;
    const int dia = std::max(1, d.m_narrativeState->variable(RPG::franjas::kVariableDia));
    const int franja = d.m_narrativeState->variable(RPG::franjas::kVariableFranja);
    return franja == 0 && dia > 1 ? dia - 1 : dia;
}

}  // namespace

std::vector<std::string> apuntaActos(SessionData& d, const std::vector<std::string>& ids) {
    std::vector<std::string> lineas;
    const Reglas& r = reglas();
    RPG::CharacterSheet* ficha = d.m_playerBody != nullptr ? d.m_playerBody->character_sheet() : nullptr;
    for (const std::string& id : ids) {
        const DefinicionActo* def = r.acto(id);
        if (def == nullptr) {
            lineas.push_back("(El acto «" + id + "» no está en el catálogo de actos.)");
            continue;
        }
        if (def->signo != 0) {
            d.m_morality.apunta(def->apuntado());
            lineas.push_back(def->motivo + (def->signo > 0 ? ". Esta noche te honrará." : ". Esta noche te pesará."));
        }
        if (def->pureza != 0) {
            d.m_morality.cambiaPureza(def->pureza, def->motivo);
            lineas.push_back(def->motivo + ": pureza " + conSigno(def->pureza) + ".");
        }
        // D-MOR-01: la reputación es aparte, por facción, y solo de lo que se
        // vio (el catálogo pone quién lo vio). Aún no se enseña (D-MOR-09).
        if (ficha != nullptr)
            for (const auto& [faccion, delta] : def->reputacion) ficha->adjustRep(faccion, delta);
    }
    return lineas;
}

std::vector<std::string> asientaNoche(SessionData& d) {
    std::vector<std::string> lineas;
    if (!d.m_morality.pendientes().empty()) {
        const Asiento a = d.m_morality.asienta(reglas().parametros, diaQueAcaba(d));
        const Tramo antes = tramoDe(a.antes), despues = tramoDe(a.despues);
        lineas.push_back("Esta noche se asienta lo de hoy: moral " + conSigno(a.antes) + " → " + conSigno(a.despues) +
                         " (" + nombreTramo(despues) + ").");
        if (despues != antes) {
            if (despues == Tramo::Neutral)
                lineas.push_back("Vuelves a ser Neutral.");
            else
                lineas.push_back(std::string(a.despues > a.antes ? "Algo en ti ha cambiado: ahora eres "
                                                                 : "Algo en ti se ha torcido: ahora eres ") +
                                 nombreTramo(despues) + ".");
        }
        for (const std::string& motivo : a.marcasNuevas) lineas.push_back("Eso no se va: " + motivo + ".");
    }
    reflejaEnLaAventura(d);
    return lineas;
}

void reflejaEnLaAventura(SessionData& d) {
    if (d.m_narrativeState != nullptr) d.m_narrativeState->ponMoral(d.m_morality.value());
}

}  // namespace moral
