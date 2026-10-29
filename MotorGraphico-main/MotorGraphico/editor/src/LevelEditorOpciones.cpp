// EST-21 (relevo A) — Implementacion. Ver la cabecera para el criterio.

#include "LevelEditorOpciones.h"

#include <cctype>
#include <cstdlib>
#include <iostream>

namespace EditorNiveles {

// "64x48" -> {64, 48}. Devuelve false si no tiene la forma AxB con
// ambos numeros > 0 (mejor rechazar que arrancar con un mapa absurdo).
bool parseSize(const std::string& text, int& outW, int& outH) {
    const std::size_t sep = text.find('x');
    if (sep == std::string::npos || sep == 0 || sep + 1 >= text.size()) {
        return false;
    }
    outW = std::atoi(text.substr(0, sep).c_str());
    outH = std::atoi(text.substr(sep + 1).c_str());
    return outW > 0 && outH > 0;
}

EditorOptions parseArgs(int argc, char** argv) {
    EditorOptions opts;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if ((arg == "--size" || arg == "-s") && i + 1 < argc) {
            if (!parseSize(argv[++i], opts.mapWidth, opts.mapHeight)) {
                std::cerr << "Tamano invalido (se esperaba AnchoxAlto, ej. 64x64)\n";
            }
        } else if ((arg == "--load" || arg == "-l") && i + 1 < argc) {
            opts.loadLevelPath = argv[++i];
        } else if (!arg.empty() && (std::isdigit(static_cast<unsigned char>(arg[0])) != 0)) {
            opts.maxFrames = std::atoi(arg.c_str());
        } else if (arg == "--proyectos" || arg == "-p") {
            opts.listProjects = true;
        } else if (arg == "--proyecto" && i + 1 < argc) {
            opts.projectId = argv[++i];
        } else if (arg == "--nivel" && i + 1 < argc) {
            opts.levelName = argv[++i];
        } else if (arg == "--nuevo" && i + 1 < argc) {
            opts.newProjectId = argv[++i];
        } else if (arg == "--prefijo" && i + 1 < argc) {
            opts.newPrefix = argv[++i];
        } else if (arg == "--construction" || arg == "--construccion") {
            opts.useConstructionTiles = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout <<
                "Uso: level_editor [opciones] [frames]\n"
                "\n  PROYECTOS (no abren ventana)\n"
                "  --proyectos              lista los proyectos vivos y su estado\n"
                "  --nuevo <id> --prefijo <p_>\n"
                "                           crea un proyecto y sale\n"
                "\n  EDICION\n"
                "  --proyecto <id>          abre ese proyecto; F5 guarda EN EL,\n"
                "                           no en editor_map.tmx\n"
                "  --nivel <fichero.json>   nivel suyo a abrir (por defecto, el primero)\n"
                "  --size 64x64             mapa nuevo en blanco de ese tamano\n"
                "  --construccion            usa el atlas de suelos, muros y vallas\n"
                "                           para un mapa nuevo\n"
                "  --load ruta.json         abre un nivel suelto (sin proyecto)\n"
                "  frames                   modo humo: N frames, vuelca PPM y sale\n"
                "\n  Dentro del editor: F7 modo jugador, F8/F9 cambia de escenario,\n"
                "  ESC vuelve a la pantalla.\n"
                "\n  La build de un proyecto se saca con:\n"
                "    python3 tools/build_proyecto.py <id>\n";
            std::exit(0);
        }
    }
    return opts;
}

}  // namespace EditorNiveles
