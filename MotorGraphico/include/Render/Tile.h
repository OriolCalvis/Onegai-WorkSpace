#pragma once

// Compatibilidad de la API de render anterior a DOC-03. El dato vive ahora en
// Runtime/: no contiene GPU y puede ser producido por RuntimeTileMapBuilder.
#include "Runtime/RuntimeTile.h"

using Tile = RuntimeTile;
