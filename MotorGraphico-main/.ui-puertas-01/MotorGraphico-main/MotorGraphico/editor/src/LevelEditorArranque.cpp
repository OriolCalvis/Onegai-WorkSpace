// EST-21 (relevo B) — Implementacion. Ver la cabecera.

#include "LevelEditorArranque.h"

#include "Project/ProjectIndex.h"

#include <cstdio>
#include <iostream>
#include <string>

namespace EditorNiveles {

// Pantalla de arranque en modo texto. Va ANTES de abrir la ventana a
// proposito: listar proyectos o crear uno no necesita contexto GL, y asi
// tambien sirve desde un script o desde CI. La misma informacion es la que
// pinta el editor en su lista cuando arranca con ventana.
int pantallaDeArranque(const EditorOptions& opts) {
    auto r = Editor::ProjectIndex::scan("assets");
    if (!r.isOk()) {
        std::cerr << "No se pudo leer assets/proyectos: " << r.errorMessage() << "\n";
        return 1;
    }
    const Editor::ProjectIndex idx = r.value();

    if (!opts.newProjectId.empty()) {
        auto nuevo = Editor::ProjectIndex::create("assets", opts.newProjectId,
                                                  opts.newProjectId, opts.newPrefix,
                                                  "editor", "2000 b.f.");
        if (!nuevo.isOk()) {
            std::cerr << "No se pudo crear: " << nuevo.errorMessage() << "\n";
            return 1;
        }
        std::cout << "Proyecto '" << nuevo.value().id << "' creado con prefijo '"
                  << nuevo.value().prefix << "'.\n"
                  << "Abrelo con:  ./level_editor --proyecto " << nuevo.value().id << "\n";
        return 0;
    }

    std::cout << "PROYECTOS VIVOS\n\n";
    std::printf("  %-13s %-32s %4s %4s %4s  %s\n",
                "id", "nombre", "niv", "av", "cat", "estado");
    for (const Editor::Project& p : idx.projects()) {
        const Editor::ProjectCheck c = idx.check(p.id);
        std::printf("  %-13s %-32s %4d %4d %4d  %s\n",
                    p.id.c_str(), p.name.substr(0, 32).c_str(),
                    (int)p.levels.size(), (int)p.adventures.size(),
                    (int)p.catalogs.size(),
                    c.ok() ? "completo" : (std::to_string(c.problems.size()) + " problema(s)").c_str());
        if (!c.ok()) {
            for (const std::string& s : c.problems) {
                std::printf("       - %s\n", s.c_str());
            }
        }
    }
    for (const std::string& o : idx.orphans()) {
        std::printf("  [huerfano] %s\n", o.c_str());
    }
    std::cout << "\n  abrir:  ./level_editor --proyecto <id>\n"
                 "  nuevo:  ./level_editor --nuevo <id> --prefijo <p_>\n"
                 "  build:  python3 tools/build_proyecto.py <id>\n";
    return 0;
}

}  // namespace EditorNiveles
