// BND-04 — requires.reputacion de una aventura: {"aegroum": {"min": 10, "max": 40}}.
// La reputacion con cada faccion (CharacterSheet::getRep, -100..100) dentro del
// rango, min y max incluidos y opcionales; con varias facciones, todas. Una
// faccion que no esta en assets/rules/facciones.json es un error de carga, no un
// «falso» callado. Pieza interna de la carga y la escritura (la llama
// MoralAventura, que ya cuelga de BeatsAventura.cpp): no es API.
#pragma once

#include <set>
#include <string>

#include "Core/Errors/Result.h"
#include "RPG/Narrative/NarrativeBeat.h"

class JsonValue;
class JsonWriter;

namespace RPG {
namespace reputacionAventura {

Result<bool> leeCondicion(const JsonValue& requiresValue, NarrativeCondition& c, const std::string& donde);
void escribeCondicion(JsonWriter& w, const NarrativeCondition& c);

// Las facciones que existen. Por defecto, las de assets/rules/facciones.json
// (leido una vez); las pruebas las sustituyen. Un conjunto vacio vuelve al fichero.
void ponFaccionesConocidas(std::set<std::string> ids);

}  // namespace reputacionAventura
}  // namespace RPG
