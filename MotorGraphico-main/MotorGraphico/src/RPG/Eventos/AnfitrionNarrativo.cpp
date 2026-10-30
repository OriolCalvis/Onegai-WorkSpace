// EST-32 — EL ANFITRION NARRATIVO: EJECUTAR UNA COMANDA.
//
// Traduce una comanda ya validada a un efecto sobre el estado narrativo.
// Lo que todavia no esta cableado (lo que necesita la sesion de partida)
// falla DICIENDO que esta declarado pero no cableado, en vez de callarse:
// un comando que no hace nada en silencio es un bug que nadie encuentra.
//
// Se valida ANTES de ejecutar. Un comando a medio rellenar tiene que
// fallar diciendo que le falta, no ejecutarse con un valor inventado.

#include "RPG/EventCommand.h"
#include "RPG/EventosInterno.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

namespace RPG {

using namespace RPG::interno;

namespace {

std::string Param(const MapEventCommand& command, const std::string& key,
                  const std::string& porDefecto = {}) {
    const auto encontrado = command.params.find(key);
    return encontrado == command.params.end() ? porDefecto : encontrado->second;
}

CommandResult Fallo(std::string diagnostico) {
    CommandResult r;
    r.outcome = CommandOutcome::Failed;
    r.diagnostic = std::move(diagnostico);
    return r;
}

}  // namespace

CommandResult NarrativeCommandHost::execute(const MapEventCommand& command) {
    // Se valida ANTES de ejecutar. Un comando a medio rellenar tiene que
    // fallar diciendo qué le falta, no ejecutarse con un valor inventado.
    const Result<bool> valido = ValidateCommand(command);
    if (!valido.isOk()) {
        return Fallo(valido.errorMessage());
    }
    const CommandSpec* spec = FindCommand(command.type);

    if (command.type == "texto.mostrar") {
        const std::string hablante = Param(command, "hablante");
        m_log.push_back(hablante.empty() ? Param(command, "texto")
                                         : hablante + ": " + Param(command, "texto"));
        return CommandResult{};
    }
    if (command.type == "bandera.encender") {
        m_state.setFlag(Param(command, "bandera"));
        return CommandResult{};
    }
    if (command.type == "bandera.apagar") {
        m_state.clearFlag(Param(command, "bandera"));
        return CommandResult{};
    }
    if (command.type == "variable.fijar") {
        m_state.setVariable(Param(command, "variable"), std::atoi(Param(command, "valor").c_str()));
        return CommandResult{};
    }
    if (command.type == "variable.sumar") {
        m_state.addToVariable(Param(command, "variable"),
                              std::atoi(Param(command, "valor").c_str()));
        return CommandResult{};
    }
    if (command.type == "interruptor.fijar") {
        const std::string letra = Param(command, "letra");
        const Result<bool> puesto = m_state.setLocalSwitch(
            Param(command, "evento"), letra.empty() ? '\0' : letra[0],
            Param(command, "valor") == "true");
        return puesto.isOk() ? CommandResult{} : Fallo(puesto.errorMessage());
    }
    if (command.type == "control.esperar") {
        CommandResult r;
        r.outcome = CommandOutcome::Wait;
        r.waitMs = std::atoi(Param(command, "ms", "0").c_str());
        return r;
    }
    if (command.type == "control.si") {
        const Result<MapEventCondition> construida = ConditionFromIf(command);
        if (!construida.isOk()) {
            return Fallo(construida.errorMessage());
        }
        const Result<bool> valor = EvaluateCondition(construida.value(), m_state);
        if (!valor.isOk()) {
            // E-09 — «no la lleva» y «no sé mirar el inventario desde
            // aquí» son cosas distintas, y aquí se nota: un `si` que
            // pregunta por un objeto no se puede resolver sin partida.
            // Si hay a quién delegar, se delega el comando ENTERO, que
            // es la costura que ya usan las demás familias de partida.
            // Sin fallback se falla diciendo qué falta, como antes.
            if (m_fallback != nullptr) {
                return m_fallback->execute(command);
            }
            return Fallo(valor.errorMessage());
        }
        CommandResult r;
        r.outcome = CommandOutcome::Branch;
        // Rama 0 si se cumple, rama 1 si no. Si no hay rama 1 —un `si`
        // sin `si no`— el intérprete no entra en ninguna y sigue, que es
        // lo correcto.
        r.branch = valor.value() ? 0 : 1;
        return r;
    }

    // El resto: se delega si hay a quién, y si no se dice qué falta.
    if (m_fallback != nullptr) {
        return m_fallback->execute(command);
    }
    if (spec != nullptr && !spec->implemented) {
        return Fallo("el comando '" + spec->label +
                     "' está declarado pero todavía no cableado: necesita la sesión de partida "
                     "(E-04, segunda rebanada)");
    }
    return Fallo("nadie sabe ejecutar el comando '" + command.type + "'");
}

bool NarrativeCommandHost::waitResolved(const MapEventCommand& command) {
    if (m_fallback != nullptr) {
        return m_fallback->waitResolved(command);
    }
    // Sin servicio de diálogo ni de combate, una espera abierta no tiene
    // quien la cierre: se da por resuelta para que el recorrido avance en
    // pruebas en vez de colgarse. Con fallback, manda el fallback.
    return true;
}

}  // namespace RPG
