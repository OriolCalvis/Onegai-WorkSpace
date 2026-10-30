// EST-32 — CONDICIONES Y PAGINAS.
//
// Una pagina de evento se activa cuando TODAS sus condiciones se cumplen
// contra el estado narrativo. Aqui esta esa cuenta, y la traduccion de un
// comando `if` a la condicion que representa.
//
// ActivePage devuelve la pagina ACTIVA, no todas. Recorre de atras hacia
// delante y gana la ULTIMA que se cumple (ver ActivePageWith abajo), asi que
// el orden de las paginas es la prioridad que el autor escribio DE MENOR A
// MAYOR: la pagina sin condiciones —el caso general— va PRIMERA, arriba del
// todo, y los casos especiales debajo. Un autor que ponga la pagina sin
// condiciones al final la ve ganar siempre y concluye que las condiciones no
// funcionan.
//
// OJO, porque es lo contrario que los beats narrativos: NarrativeEngine::fire
// gana el PRIMER beat cuyas condiciones se cumplen, y alli el caso general va
// AL FINAL. Dos reglas opuestas en el mismo motor para dos vocabularios que se
// parecen mucho; H3-03 las midio las dos antes de escribir contenido.

#include "RPG/EventCommand.h"
#include "RPG/EventosInterno.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

namespace RPG {

using namespace RPG::interno;

Result<bool> EvaluateCondition(const MapEventCondition& condition, const NarrativeState& state) {
    if (condition.reference.empty()) {
        return Result<bool>::Error("una condición que no nombra nada no se puede evaluar");
    }
    const std::string op = condition.comparison.empty() ? "==" : condition.comparison;
    bool resultado = false;

    switch (condition.kind) {
        case EventConditionKind::Flag: {
            const bool encendida = state.hasFlag(condition.reference);
            // El valor por defecto de una condición de bandera es "está
            // encendida": es lo que el autor quiere decir el 90% de las
            // veces y obligarle a escribir "== true" sería ruido.
            const bool esperado = condition.value.empty() || condition.value == "true";
            if (op != "==" && op != "!=") {
                return Result<bool>::Error("una bandera solo admite == o !=, y se pidió '" + op +
                                           "'");
            }
            resultado = (op == "==") ? (encendida == esperado) : (encendida != esperado);
            break;
        }
        case EventConditionKind::Variable: {
            long long esperado = 0;
            if (!EsEnteroValido(condition.value, &esperado)) {
                return Result<bool>::Error("la condición sobre la variable '" +
                                           condition.reference + "' compara con '" +
                                           condition.value + "', que no es un número");
            }
            const long long actual = state.variable(condition.reference);
            if (op == "==") resultado = actual == esperado;
            else if (op == "!=") resultado = actual != esperado;
            else if (op == ">=") resultado = actual >= esperado;
            else if (op == "<=") resultado = actual <= esperado;
            else if (op == ">") resultado = actual > esperado;
            else if (op == "<") resultado = actual < esperado;
            else return Result<bool>::Error("comparación desconocida: '" + op + "'");
            break;
        }
        case EventConditionKind::LocalSwitch: {
            // Formato: "eventId.letra" (ej: "evt_cofre.A")
            const std::string& ref = condition.reference;
            const auto dotPos = ref.find('.');
            if (dotPos == std::string::npos || dotPos == 0 || dotPos + 1 >= ref.size()) {
                return Result<bool>::Error("condición de interruptor local inválida: se espera 'eventId.letra' (A-D)");
            }
            const std::string eventId = ref.substr(0, dotPos);
            const char letter = ref[dotPos + 1];
            const bool encendido = state.localSwitch(eventId, letter);
            const bool esperado = condition.value.empty() || condition.value == "true";
            if (op != "==" && op != "!=") {
                return Result<bool>::Error("un interruptor local solo admite == o !=, y se pidió '" + op + "'");
            }
            resultado = (op == "==") ? (encendido == esperado) : (encendido != esperado);
            break;
        }
        case EventConditionKind::Item:
            // NO se devuelve falso. "No tiene la llave" y "no sé mirar el
            // inventario desde aquí" son cosas distintas, y confundirlas
            // convierte una pieza que falta en una puerta que nunca se
            // abre y que nadie sabe por qué.
            return Result<bool>::Error(
                "las condiciones de inventario todavía no se pueden evaluar: hace falta la "
                "sesión de partida (E-04, segunda rebanada)");
    }
    return Result<bool>::Ok(condition.negated ? !resultado : resultado);
}

Result<MapEventCondition> ConditionFromIf(const MapEventCommand& command) {
    MapEventCondition condicion;
    const auto clave = command.params.find("clase");
    const std::string clase = clave == command.params.end() ? std::string() : clave->second;
    if (clase == "variable") {
        condicion.kind = EventConditionKind::Variable;
    } else if (clase == "item") {
        condicion.kind = EventConditionKind::Item;
    } else if (clase == "flag") {
        condicion.kind = EventConditionKind::Flag;
    } else if (clase == "local_switch") {
        condicion.kind = EventConditionKind::LocalSwitch;
    } else {
        return Result<MapEventCondition>::Error("clase de condición desconocida: '" + clase + "'");
    }
    auto leer = [&command](const char* clave, const std::string& porDefecto) {
        const auto it = command.params.find(clave);
        return it == command.params.end() || it->second.empty() ? porDefecto : it->second;
    };
    condicion.reference = leer("referencia", "");
    // Si el autor no escribe comparación, se deja VACIA y cada clase
    // aplica la suya: "==" para una bandera, ">=" para un objeto. Poner
    // "==" aquí convertiría «lleva la llave» en «lleva exactamente una
    // llave», y la puerta no se abriría con dos.
    condicion.comparison = leer("comparacion", "");
    condicion.value = leer("valor", "");
    condicion.negated = leer("negada", "") == "true";
    return Result<MapEventCondition>::Ok(condicion);
}

Result<bool> EvaluatePageWith(
    const MapEventPage& page,
    const std::function<Result<bool>(const MapEventCondition&)>& evaluate) {
    // Sin condiciones, la página se cumple. Es lo que espera quien pone
    // una sola página sin condición ninguna.
    if (page.conditions.empty()) {
        return Result<bool>::Ok(true);
    }
    bool alguna = false;
    for (const MapEventCondition& condicion : page.conditions) {
        const Result<bool> valor = evaluate(condicion);
        if (!valor.isOk()) {
            // Una condición que no se puede evaluar NO se cuenta como
            // falsa: se propaga el error para que el autor lo vea.
            return valor;
        }
        if (page.requiresAllConditions && !valor.value()) {
            return Result<bool>::Ok(false);
        }
        alguna = alguna || valor.value();
    }
    return Result<bool>::Ok(page.requiresAllConditions ? true : alguna);
}

Result<bool> EvaluatePage(const MapEventPage& page, const NarrativeState& state) {
    return EvaluatePageWith(page, [&state](const MapEventCondition& c) {
        return EvaluateCondition(c, state);
    });
}

ActivePageResult ActivePageWith(
    const MapEventDocument& event,
    const std::function<Result<bool>(const MapEventCondition&)>& evaluate) {
    ActivePageResult salida;
    // De atrás hacia delante: gana la ULTIMA que se cumple. Pero NO se
    // corta al encontrarla — hay que seguir mirando las de arriba para
    // saber si alguna era imposible de evaluar, porque entonces la
    // elección podría estar mal y hay que decirlo.
    for (std::size_t i = event.pages.size(); i > 0; --i) {
        const std::size_t indice = i - 1;
        const MapEventPage& pagina = event.pages[indice];
        const Result<bool> valor = EvaluatePageWith(pagina, evaluate);
        if (!valor.isOk()) {
            salida.unevaluable.push_back(indice);
            if (salida.diagnostic.empty()) {
                salida.diagnostic = valor.errorMessage();
            }
            continue;
        }
        if (valor.value() && salida.page == nullptr) {
            salida.page = &pagina;
            salida.index = indice;
        }
    }
    // Se han recorrido al revés: que la lista salga en orden de página.
    std::reverse(salida.unevaluable.begin(), salida.unevaluable.end());
    return salida;
}

ActivePageResult ActivePage(const MapEventDocument& event, const NarrativeState& state) {
    return ActivePageWith(event, [&state](const MapEventCondition& c) {
        return EvaluateCondition(c, state);
    });
}

}  // namespace RPG
