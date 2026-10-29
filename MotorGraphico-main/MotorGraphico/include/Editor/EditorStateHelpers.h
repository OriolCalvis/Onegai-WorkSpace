#pragma once

// Las dos piezas que TODOS los trozos de EditorState necesitan: sustituir
// una capa entera en el documento y fabricar la meta de una capa nueva.
//
// Los otros cinco ayudantes del namespace anonimo original tenian un solo
// consumidor y se fueron con el (el escapado XML/JSON a la exportacion, el
// paso a minusculas al filtro de paleta, y los de entidades a los objetos).
// Estos dos no: los usan pintar, objetos y el propio nucleo. Salieron de
// src/Editor/EditorState.cpp el 24/09/2026 (EST-24).

#include <string>

#include "Map/MapDocument.h"

namespace EditorInternal {

bool replaceLayer(MapDocument& document, MapLayer layer);
MapLayerMeta newLayerMeta(const MapDocument& document, const std::string& name);

}  // namespace EditorInternal
