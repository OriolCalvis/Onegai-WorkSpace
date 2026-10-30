#pragma once

// Las tres piezas que comparten la navegacion, el estado y la busqueda de
// la lista de problemas: listar los eventos de un documento, encontrar uno
// por id y seguir una ruta de comandos anidados hasta el que se senala.
//
// Los otros cinco ayudantes del namespace anonimo original tenian un solo
// consumidor y se fueron con el (cuatro a la busqueda, uno a la
// navegacion). Estos tres no: se conto quien los llama, no se decidio a
// ojo. Salieron de src/Editor/ProblemsState.cpp el 24/09/2026 (EST-31).
//
// Es interna del modelo de problemas: no la incluye el panel.

#include <cstddef>
#include <string>
#include <vector>

#include "Map/MapDocument.h"
#include "RPG/EventCommand.h"

namespace Editor {
namespace internal {

std::vector<const MapEventDocument*> EventosDe(const MapDocument& document);
const MapEventDocument* BuscarEvento(const MapDocument& document,
                                     const std::string& eventId);
const MapEventCommand* SeguirRuta(const std::vector<MapEventCommand>& raiz,
                                  const std::vector<std::size_t>& ruta);

}  // namespace internal
}  // namespace Editor
