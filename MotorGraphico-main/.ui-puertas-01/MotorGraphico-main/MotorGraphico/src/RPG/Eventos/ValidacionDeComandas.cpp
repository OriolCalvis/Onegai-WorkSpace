// EST-32 — VALIDAR UNA COMANDA CONTRA SU FICHA.
//
// Lo que decide si lo que escribio el autor se puede ejecutar: que el
// comando exista en esta version, que no le falte ningun parametro
// obligatorio, y que cada valor sea del tipo que su ficha declara.
//
// LOS MENSAJES SON LA MITAD DEL TRABAJO. Un validador que dice "invalido"
// obliga al autor a adivinar; por eso cada error nombra el parametro, dice
// que se esperaba y ensena lo que hay. Un comando que no existe NO es un
// fallo del autor: puede venir de una version mas nueva, y se dice asi.

#include "RPG/EventCommand.h"
#include "RPG/EventosInterno.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

namespace RPG {

using namespace RPG::interno;

namespace {

Result<bool> ValidarValor(const CommandParamSpec& spec, const std::string& valor,
                          const std::string& donde) {
    const std::string prefijo = donde + ": el parámetro '" + spec.key + "' ";
    long long numero = 0;
    switch (spec.type) {
        case CommandParamType::Bool:
            if (valor != "true" && valor != "false") {
                return Result<bool>::Error(prefijo + "debe ser true o false, y es '" + valor + "'");
            }
            break;
        case CommandParamType::Int:
            if (!EsEnteroValido(valor, &numero)) {
                return Result<bool>::Error(prefijo + "debe ser un entero, y es '" + valor + "'");
            }
            break;
        case CommandParamType::DurationMs:
            if (!EsEnteroValido(valor, &numero) || numero < 0) {
                return Result<bool>::Error(prefijo +
                                           "debe ser una duración en ms no negativa, y es '" +
                                           valor + "'");
            }
            break;
        case CommandParamType::SwitchLetter: {
            const char letra = valor.size() == 1 ? valor[0] : '\0';
            const char mayus = (letra >= 'a' && letra <= 'z')
                                   ? static_cast<char>(letra - 'a' + 'A') : letra;
            if (mayus < NarrativeState::kFirstLocalSwitch ||
                mayus > NarrativeState::kLastLocalSwitch) {
                return Result<bool>::Error(prefijo + "debe ser A, B, C o D, y es '" + valor + "'");
            }
            break;
        }
        case CommandParamType::Comparison:
            if (!EsComparacionValida(valor)) {
                return Result<bool>::Error(prefijo + "no es una comparación válida: '" + valor +
                                           "'");
            }
            break;
        case CommandParamType::Text:
        case CommandParamType::FlagRef:
        case CommandParamType::VariableRef:
        case CommandParamType::ObjectRef:
        case CommandParamType::MapRef:
        case CommandParamType::EventRef:
            // Una referencia vacía es un hueco sin rellenar, y el editor
            // tiene que poder decirlo. Que APUNTE a algo existente no se
            // comprueba aquí: eso necesita el proyecto entero y es E-07.
            if (spec.required && valor.empty()) {
                return Result<bool>::Error(prefijo + "está vacío");
            }
            break;
    }
    return Result<bool>::Ok(true);
}

}  // namespace

Result<bool> ValidateCommand(const MapEventCommand& command) {
    const CommandSpec* spec = FindCommand(command.type);
    if (spec == nullptr) {
        // No es un fallo del autor: puede ser un proyecto hecho con una
        // versión más nueva. E-01 conserva sus datos; el editor lo enseña
        // como bloque no disponible (diseño §9).
        return Result<bool>::Error("comando no disponible en esta versión: '" + command.type + "'");
    }
    const std::string donde = "el comando '" + spec->label + "'";
    for (const CommandParamSpec& parametro : spec->params) {
        const auto encontrado = command.params.find(parametro.key);
        if (encontrado == command.params.end()) {
            if (parametro.required) {
                return Result<bool>::Error(donde + ": falta el parámetro '" + parametro.key + "'");
            }
            continue;
        }
        const Result<bool> valido = ValidarValor(parametro, encontrado->second, donde);
        if (!valido.isOk()) {
            return valido;
        }
    }
    if (command.children.size() > spec->branches) {
        return Result<bool>::Error(donde + ": admite " + std::to_string(spec->branches) +
                                   " rama(s) y tiene " + std::to_string(command.children.size()));
    }
    // Las ramas, recursivamente. Las que hay debajo de una rama son sus
    // propios hijos, así que se recorren igual.
    for (const MapEventCommand& rama : command.children) {
        for (const MapEventCommand& dentro : rama.children) {
            const Result<bool> valida = ValidateCommand(dentro);
            if (!valida.isOk()) {
                return valida;
            }
        }
    }
    return Result<bool>::Ok(true);
}

}  // namespace RPG
