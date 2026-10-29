// EST-21 (relevo A) — LAS OPCIONES DE ARRANQUE DEL EDITOR.
//
// La linea de comandos y nada mas. Sin ventana, sin GL y sin disco: se
// puede probar entera pasandole un argv de mentira, que es justo lo que
// hace tests/Editor/LevelEditorNucleoTests.cpp.
//
// EL ARGUMENTO NUMERICO SUELTO SIGUE SIENDO EL MODO HUMO (N fotogramas y
// salir), como en todos los demos GL. Las opciones nuevas se anaden sin
// romper esa convencion, porque CI depende de ella.

#pragma once

#include <string>

namespace EditorNiveles {

// Opciones de linea de comandos. El argumento numerico suelto sigue
// siendo el modo humo (N frames y salir), como en todos los demos GL:
// las opciones nuevas se anaden sin romper esa convencion.
struct EditorOptions {
    int mapWidth = 8;
    int mapHeight = 8;
    std::string loadLevelPath;  // vacio = mapa nuevo en blanco
    int maxFrames = -1;

    // --- Proyectos (pantalla de arranque) ---
    std::string projectId;      // proyecto activo: manda donde se GUARDA
    std::string levelName;      // nivel suyo a abrir; vacio = el primero
    std::string newProjectId;   // --nuevo: crear y salir
    std::string newPrefix;
    bool listProjects = false;  // --proyectos: listar y salir
    bool useConstructionTiles = false;  // atlas básico para crear mapa nuevo
};

// "64x48" -> {64, 48}. Devuelve false si no tiene la forma AxB con
// ambos numeros > 0 (mejor rechazar que arrancar con un mapa absurdo).
bool parseSize(const std::string& text, int& outW, int& outH);

EditorOptions parseArgs(int argc, char** argv);

}  // namespace EditorNiveles
