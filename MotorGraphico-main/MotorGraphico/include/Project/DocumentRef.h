#pragma once

#include <string>

#include "Core/Errors/Result.h"
#include "Core/Ids/ProjectIds.h"

enum class DocumentKind { Map, Level, Adventure, Catalog };

// Referencia persistente a un documento. logicalPath sirve para localizarlo
// dentro del proyecto, pero la identidad no cambia cuando se mueve o renombra
// el archivo: las relaciones guardan DocumentId, no la ruta como identidad.
struct DocumentRef {
    ids::DocumentId id;
    DocumentKind kind = DocumentKind::Map;
    std::string logicalPath;

    Result<bool> validate() const {
        if (id.empty()) {
            return Result<bool>::Error("una referencia documental necesita un id");
        }
        if (logicalPath.empty() || logicalPath.front() == '/' || logicalPath.front() == '\\' ||
            logicalPath.find('\\') != std::string::npos ||
            logicalPath.find("../") != std::string::npos || logicalPath == "..") {
            return Result<bool>::Error("ruta lógica de documento inválida: '" + logicalPath + "'");
        }
        return Result<bool>::Ok(true);
    }
};
