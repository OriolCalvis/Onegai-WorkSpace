// MORAL-01 — Leer y escribir lo de la moral en una aventura: el efecto "acto"
// (y "herida", que llegó con la barricada del Ocaso), el tramo de moral de los
// "requires" y los "exitos" fijos de una tirada. Pieza interna de la carga y la
// escritura (BeatsAventura.cpp), como AcompanantesAventura: no es API. El
// formato, en FORMATO_AVENTURAS.md.
#pragma once

#include <string>

#include "Core/Errors/Result.h"
#include "RPG/Narrative/NarrativeBeat.h"

class JsonValue;
class JsonWriter;

namespace RPG {
namespace moralAventura {

// "acto": arg con el id del acto. "herida": arg con lo que hiere y value con el
// dado (2..20). Los demás efectos pasan sin mirar.
Result<bool> leeEfecto(const JsonValue& entrada, NarrativeEffect& effect, const std::string& donde);

// "moral": {"min", "max"} en -100..100, min <= max. Sin "moral", nada.
Result<bool> leeCondicion(const JsonValue& requiresValue, NarrativeCondition& c, const std::string& donde);
void escribeCondicion(JsonWriter& w, const NarrativeCondition& c);

// "exitos" de un skillCheck (0..24). Se escribe solo si los hay.
Result<bool> leeExitos(const JsonValue& skillCheck, SkillCheckRequest& r, const std::string& donde);
void escribeExitos(JsonWriter& w, const SkillCheckRequest& r);

}  // namespace moralAventura
}  // namespace RPG
