#pragma once

#include <string>

#include "Core/Errors/Result.h"

namespace Editor {

// Ajustes globales del documento Project. Solo contienen datos: no saben
// de ventanas, renderizadores ni servicios de I/O. Las rutas son lógicas
// y relativas a la raíz del proyecto para que un pack siga siendo portable.
struct ProjectSettings {
    static constexpr int kCurrentSchemaVersion = 1;

    int schemaVersion = kCurrentSchemaVersion;
    std::string mapsDirectory = "maps";
    std::string levelsDirectory = "levels";
    std::string adventuresDirectory = "adventures";
    std::string catalogsDirectory = "objects";
    std::string initialMap;

    Result<bool> validate() const;
};

}  // namespace Editor
