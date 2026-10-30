#pragma once

#include "Core/Errors/Result.h"
#include "Map/MapDocument.h"
#include "Runtime/RuntimeTileMap.h"

// Frontera DOC -> RUN. Compila solo las capas de tiles que necesita el
// runtime; entidades y regiones permanecen en MapDocument para sus sistemas
// de juego y edición. No hace I/O, no abre una ventana y no toca GPU.
class RuntimeTileMapBuilder {
public:
    Result<RuntimeTileMap> build(const MapDocument& document) const;
};
