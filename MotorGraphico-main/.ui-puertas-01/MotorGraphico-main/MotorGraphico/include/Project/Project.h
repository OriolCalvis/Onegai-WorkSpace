#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Project/ProjectSettings.h"

namespace Editor {

// Documento raíz de un pack de contenido. ProjectIndex se ocupa de cargarlo
// y guardarlo; esta estructura y validate() permanecen puramente GL-free.
struct Project {
    std::string id;
    std::string name;
    std::string description;
    std::string epoch;
    std::string author;
    std::string prefix;

    ProjectSettings settings;

    // Punto de entrada legacy: ruta lógica o nombre de aventura/nivel.
    std::string entry;
    std::vector<std::string> levels;
    std::vector<std::string> maps;
    std::vector<std::string> adventures;
    std::vector<std::string> catalogs;

    enum class EntryKind { None, Adventure, Level, Missing };
    EntryKind entryKind = EntryKind::None;
    std::string entryPath;

    std::size_t assetCount() const {
        return levels.size() + maps.size() + adventures.size() + catalogs.size();
    }
    bool playable() const { return !entry.empty() && !levels.empty(); }

    Result<bool> validate() const;
};

// Resultado de revisar contenido en disco. El índice lo completa tras I/O;
// no participa en la validación estructural de Project.
struct ProjectCheck {
    std::string projectId;
    std::vector<std::string> problems;
    int levelsChecked = 0;
    int missingFiles = 0;
    bool ok() const { return problems.empty(); }
};

}  // namespace Editor
