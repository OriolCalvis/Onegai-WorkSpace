#pragma once

#include "Core/Errors/Result.h"
#include "Editor/EditorState.h"

// Guarda autoría nativa y sus dos salidas legacy como un lote. Prepara todos
// los archivos antes de sustituirlos; un fallo de I/O revierte los ya instalados.
// La recuperación tras apagado en mitad del lote pertenece a C-15/G-01.
class EditorDocumentIO {
public:
    static std::string documentPathFor(const std::string& tmxPath);
    static Result<bool> save(const EditorState& editor, const std::string& levelName,
                             const std::string& tmxPath, const std::string& levelPath,
                             const TmxTilesetSettings& tileset,
                             const std::vector<EnemySpawn>& legacyEnemies = {});
};
