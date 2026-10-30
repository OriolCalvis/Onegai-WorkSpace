// VARIABLES-01 / OPCIONES-01 — Leer y escribir lo que llevan dentro un beat y
// sus opciones: las condiciones ("requires"), los efectos ("effects") y las
// opciones ("opciones"). Pieza interna de la carga
// (AdventureScriptLoader.cpp) y de la escritura (AdventureSerializer.cpp); no
// es API. El formato, en FORMATO_AVENTURAS.md.
#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "RPG/Narrative/AdventureScript.h"

class JsonValue;
class JsonWriter;

namespace RPG {
namespace beats {

// Lista de strings de un array JSON, saltando los vacios.
std::vector<std::string> cadenas(const JsonValue& value);

// Las condiciones de un "requires" (vacio = siempre). `donde` encabeza el
// error: nivel o tier fuera de rango, o una variable sin rango valido.
Result<NarrativeCondition> leeCondicion(const JsonValue& requiresValue, const std::string& donde);

// Los efectos, en orden. Un startBattle solo puede pedir acompañantes de
// `acompanantes` (los de la aventura).
Result<std::vector<NarrativeEffect>> leeEfectos(const JsonValue& effectsValue, const std::string& donde,
                                                const std::vector<AcompananteDef>& acompanantes);

// Las opciones de un beat: como mucho NarrativeBeat::kMaxOpciones, cada una
// con su texto corto; sin "speaker", contesta `speakerDelBeat`.
Result<std::vector<OpcionDeDialogo>> leeOpciones(const JsonValue& opcionesValue, const std::string& donde,
                                                 const std::string& speakerDelBeat,
                                                 const std::vector<AcompananteDef>& acompanantes);

// Lo mismo de vuelta, con el formato que entiende la lectura: "requires"
// solo si no esta vacia, "effects" siempre y "opciones" solo si hay.
void escribeCondicion(JsonWriter& w, const NarrativeCondition& c);
void escribeEfectos(JsonWriter& w, const std::vector<NarrativeEffect>& efectos);
void escribeOpciones(JsonWriter& w, const std::vector<OpcionDeDialogo>& opciones);

}  // namespace beats
}  // namespace RPG
