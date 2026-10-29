// DIALOGOS-01 — Leer y escribir lo de las charlas a lo D&D en una aventura
// («Boundington: los diálogos»): el "lugar" de los "requires" (el beat solo
// salta en ese nivel) y los papeles de una opcion ("sigue": la charla sigue;
// "pasaPagina": «Hablar de otra cosa»; "despedida": la salida). Pieza interna
// de la carga y la escritura (BeatsAventura.cpp), como MoralAventura: no es
// API. El formato, en FORMATO_AVENTURAS.md.
#pragma once

#include <string>

#include "Core/Errors/Result.h"
#include "RPG/Narrative/NarrativeBeat.h"

class JsonValue;
class JsonWriter;

namespace RPG {
namespace charlaAventura {

// "lugar": un nivel ("assets/levels/x.json") o una lista. Sin "lugar", nada.
Result<bool> leeCondicion(const JsonValue& requiresValue, NarrativeCondition& c, const std::string& donde);
void escribeCondicion(JsonWriter& w, const NarrativeCondition& c);

// "sigue", "pasaPagina" y "despedida" de una opcion: true o no estan.
void leeOpcion(const JsonValue& o, OpcionDeDialogo& opcion);
void escribeOpcion(JsonWriter& w, const OpcionDeDialogo& o);

}  // namespace charlaAventura
}  // namespace RPG
