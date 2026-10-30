// H4-02 / E-11 — Los mapas de un proyecto, gestionados sin abrir una ventana.
//
// Hasta ahora motor_editor solo podia ABRIR los mapas que alguien habia
// creado a mano: no habia forma de empezar uno nuevo, copiarlo, cambiarle el
// nombre que ve el jugador ni sacarlo del proyecto sin editar JSON. Esto es
// esa gestion, como datos: el panel Proyecto (editor/src/ProjectPanel.cpp)
// solo pinta y llama aqui, y la prueba lo recorre entero sin ImGui.
//
// UN MAPA SON TRES FICHEROS: el nivel (levels/<id>.json, lo que el juego
// carga y donde vive el nombre visible), el TMX (maps/<id>.tmx, lo que pinta
// el runtime) y el documento de autoria (maps/<id>.map.json). Los escribe
// EditorDocumentIO, el mismo que usa Guardar: un mapa recien creado es
// exactamente lo que el editor habria guardado.
//
// TODO O NADA, como ProjectIndex::duplicate (criterio de E-11: «cancel·lar o
// fallar una operacio no deixa un projecte mig registrat»). Primero los
// ficheros, despues el manifiesto; si el manifiesto falla, los ficheros nuevos
// se borran. Y nada se sobreescribe: el id se elige libre en las dos carpetas.
//
// QUITAR NO BORRA. Igual que ProjectIndex::remove con los proyectos: el mapa
// sale del manifiesto y sus ficheros se quedan en disco, porque un borrado
// desde el editor no se puede deshacer y otro proyecto puede estar usandolos.
#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Editor/EditorState.h"
#include "Project/Project.h"

namespace Editor {

// Un mapa tal como lo ve el autor en el arbol del proyecto.
struct ProjectMap {
    std::string level;   // "smoke_cueva.json": lo que guarda el manifiesto
    std::string path;    // "assets/levels/smoke_cueva.json": lo que abre el editor
    std::string name;    // nombre visible del nivel; si no se puede leer, el del fichero
    std::string map;     // TMX al que apunta el nivel ("" si no se pudo leer)
    bool entry = false;  // es donde empieza el juego (entry del manifiesto)
    bool readable = true;
    // La ruta absoluta y normalizada: el documento abierto puede venir de
    // --load con otra forma de escribir el mismo fichero.
    std::string canonical;
};

// Un tileset que se puede elegir al crear un mapa: la cabecera <tileset> de
// un TMX que ya existe, con sus muros (collision) incluidos.
struct ProjectTileset {
    std::string name;    // tilesetName del TMX
    std::string source;  // TMX del que sale, para que el autor sepa cual es
    std::string image;   // imagen YA RESUELTA (el TMX la guarda relativa a si mismo)
    TmxTilesetSettings settings;
};

class ProjectMaps {
public:
    static constexpr int kMinSide = 3;
    static constexpr int kMaxSide = 256;

    // Los mapas del proyecto, en el orden del manifiesto.
    static std::vector<ProjectMap> list(const std::string& assetsRoot, const Project& project);

    // Tilesets distintos de los mapas del proyecto; si no tiene ninguno (un
    // proyecto recien creado), los de todos los proyectos.
    static std::vector<ProjectTileset> tilesets(const std::string& assetsRoot,
                                                const std::vector<Project>& projects,
                                                const std::string& projectId);

    // El id de fichero que tendria un mapa con ese nombre visible: prefijo
    // del proyecto + id sugerido, libre en las carpetas de niveles y mapas.
    static Result<std::string> suggestFileId(const std::string& assetsRoot, const Project& project,
                                             const std::string& visibleName);

    // Mapa nuevo: suelo de la primera baldosa que no es muro, el jugador en
    // el centro, y nivel + TMX + documento en el manifiesto. La imagen del
    // tileset se vuelve a hacer relativa a la carpeta del TMX nuevo.
    static Result<ProjectMap> create(const std::string& assetsRoot, const std::string& projectId,
                                     const std::string& visibleName, int width, int height,
                                     const ProjectTileset& tileset);

    // Copia con otro nombre y otro id. Los ficheros se copian tal cual; solo
    // cambian el nombre visible, las rutas del nivel y el id del documento.
    static Result<ProjectMap> duplicate(const std::string& assetsRoot, const std::string& projectId,
                                        const std::string& level,
                                        const std::string& newVisibleName);

    // Cambia el nombre que ve el jugador. El fichero (el id) no cambia.
    static Result<bool> rename(const std::string& assetsRoot, const std::string& projectId,
                               const std::string& level, const std::string& newVisibleName);

    // Saca el mapa del manifiesto; sus ficheros siguen en disco. No deja
    // quitar el mapa donde empieza el juego.
    static Result<bool> remove(const std::string& assetsRoot, const std::string& projectId,
                               const std::string& level);

    // El juego empieza en este mapa (entry del manifiesto).
    static Result<bool> makeEntry(const std::string& assetsRoot, const std::string& projectId,
                                  const std::string& level);

    // Lo que ve el JUGADOR en la pantalla de titulo (settings.title). Vacio
    // vuelve al nombre del proyecto.
    static Result<bool> setTitle(const std::string& assetsRoot, const std::string& projectId,
                                 const std::string& title);
};

// La ruta absoluta y sin "..": para saber si dos rutas son el mismo fichero
// aunque esten escritas distinto ("assets/levels/x.json" y "/tmp/w/assets/...").
std::string canonicalPath(const std::string& path);

// Cambia el valor (cadena) de una clave de primer nivel de un objeto JSON sin
// tocar nada mas del texto: formato, orden y el resto de campos se quedan
// byte a byte. Si la clave no existe y insertIfMissing, se anade al principio.
// Error si el texto no es un objeto JSON o la clave tiene un valor no cadena.
Result<std::string> replaceTopLevelString(const std::string& jsonText, const std::string& key,
                                          const std::string& value, bool insertIfMissing);

}  // namespace Editor
