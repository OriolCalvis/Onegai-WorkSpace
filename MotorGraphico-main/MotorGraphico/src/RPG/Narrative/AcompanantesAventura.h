// ACOMPANANTES-01 — Leer y escribir los acompañantes de una aventura: la
// lista "acompanantes" de arriba (quien puede pelear al lado del heroe, y
// como) y la de cada "startBattle" (quien pelea en ese combate). Pieza
// interna de la carga (AdventureScriptLoader.cpp) y de la escritura
// (AdventureSerializer.cpp); no es API. El formato, en FORMATO_AVENTURAS.md.
#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "RPG/Narrative/AdventureScript.h"

class JsonValue;
class JsonWriter;

namespace RPG {
namespace acompanantes {

// La lista de la aventura. Error con nombre si falta el id, se repite, la
// vida no llega a 1, los golpes no van de 0 a 4, un stat no va de 0 a 8,
// "alCaer" no es "cae" ni "se_retira" o la cura esta a medias.
Result<std::vector<AcompananteDef>> lee(const JsonValue& lista);

// La de un combate: cada id tiene que estar en `definidos`, sin repetirse,
// y como mucho BattleRequest::kMaxAcompanantes. `donde` encabeza el error.
Result<std::vector<BattleRequest::ConAcompanante>> leeDeCombate(const JsonValue& lista,
                                                                const std::vector<AcompananteDef>& definidos,
                                                                const std::string& donde);

// ROSITA-01 — Los "refuerzos" de un combate (BattleRequest::Refuerzo): cada
// uno con "requires" (como un beat), "cuando" ({"vidaEnemigo", "vidaHeroe"}
// en (0, 1], al menos uno), "flags", "lineas" y "acompanantes" (como los del
// combate). Error con nombre si falta el "cuando" o un acompañante no esta.
Result<std::vector<BattleRequest::Refuerzo>> leeRefuerzos(const JsonValue& lista,
                                                          const std::vector<AcompananteDef>& definidos,
                                                          const std::string& donde);

// De vuelta a texto. Se escriben solo si hay alguno: las aventuras sin
// acompañantes salen igual que antes. La de un combate escribe sus
// "acompanantes" y sus "refuerzos" (ROSITA-01).
void escribe(JsonWriter& w, const std::vector<AcompananteDef>& lista);
void escribeDeCombate(JsonWriter& w, const BattleRequest& combate);

}  // namespace acompanantes
}  // namespace RPG
