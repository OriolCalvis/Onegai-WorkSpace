#pragma once

#include <string>
#include <vector>

#include "Map/MapDocument.h"

enum class ImportSeverity { Warning, Error };

struct ImportIssue {
    ImportSeverity severity = ImportSeverity::Warning;
    std::string message;
    int line = 0;
};

// Un informe conserva todas las limitaciones visibles para el autor. Si hay
// errores, document queda inválido y no debe guardarse; las advertencias
// indican datos TMX admitidos parcialmente que aún no tienen representación
// documental canónica.
template <class Document>
struct ImportReport {
    Document document;
    std::vector<ImportIssue> issues;

    bool hasErrors() const;
    bool hasWarnings() const;
};

template <class Document>
bool ImportReport<Document>::hasErrors() const {
    for (const ImportIssue& issue : issues) {
        if (issue.severity == ImportSeverity::Error) {
            return true;
        }
    }
    return false;
}

template <class Document>
bool ImportReport<Document>::hasWarnings() const {
    for (const ImportIssue& issue : issues) {
        if (issue.severity == ImportSeverity::Warning) {
            return true;
        }
    }
    return false;
}

using TmxMapImportReport = ImportReport<MapDocument>;

// Adaptador de Tiled/TMX hacia el modelo documental. Soporta mapas finitos,
// tilesets embebidos y capas tile CSV. Las funciones no soportadas nunca se
// pierden silenciosamente: entran en el informe como warning o error.
class TmxMapImporter {
public:
    TmxMapImportReport importFile(const std::string& path, ids::MapId mapId) const;
};
