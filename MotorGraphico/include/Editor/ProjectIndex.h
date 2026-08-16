#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"

// ---------------------------------------------------------------------
// ProjectIndex — los proyectos vivos del motor.
//
// Un PROYECTO es un pack de contenido: sus mapas, sus niveles, sus
// aventuras y sus catalogos de objetos. "Los Perdidos de Boundington" es
// un proyecto; cada cuadrante del experimento de las cuatro IAs es otro.
//
// POR QUE UN MANIFIESTO Y NO ADIVINAR POR PREFIJO. La primera version
// agrupaba por el prefijo del nombre de fichero, y se rompio a la
// primera: un cuadrante llamo a sus niveles "ciudad_en_*" y Boundington
// se los trago enteros (86 niveles en vez de 64), porque tambien empieza
// por "ciudad_". Con cuatro autores trabajando a la vez, quien es dueno
// de que no se deduce de un nombre: se declara.
//
// El manifiesto vive en assets/proyectos/<id>.json y es la fuente de
// verdad. Si un fichero no esta en ningun manifiesto, el indice lo
// reporta como huerfano en vez de repartirlo a ojo.
//
// GL-free a proposito, igual que EditorState: la pantalla de arranque del
// editor lo usa para pintar la lista, pero el indice no sabe nada de
// ventanas y se puede probar entero sin abrir uno.
// ---------------------------------------------------------------------
namespace Editor {

struct Project {
    std::string id;           // "boundington", "oeste_norte"...
    std::string name;         // "Los Perdidos de Boundington"
    std::string description;
    std::string epoch;        // "1981 b.f." — el mundo esta fechado
    std::string author;
    std::string prefix;       // prefijo de ids; vacio = sin prefijo (Boundington)
    std::string entry;        // aventura por la que arranca, si tiene

    std::vector<std::string> levels;     // nombres de fichero, sin ruta
    std::vector<std::string> maps;
    std::vector<std::string> adventures;
    std::vector<std::string> catalogs;

    std::size_t assetCount() const {
        return levels.size() + maps.size() + adventures.size() + catalogs.size();
    }
    bool playable() const { return !entry.empty() && !levels.empty(); }
};

// Resultado de revisar un proyecto. No es "compila / no compila": es si
// el contenido esta COMPLETO y ALCANZABLE, que es lo que rompe de verdad.
struct ProjectCheck {
    std::string projectId;
    std::vector<std::string> problems;   // vacio = todo bien
    int levelsChecked = 0;
    int missingFiles = 0;
    bool ok() const { return problems.empty(); }
};

class ProjectIndex {
public:
    // Lee <assetsRoot>/proyectos/index.json (la lista de ids) y despues
    // cada <id>.json.
    //
    // Hay un indice en vez de escanear la carpeta porque TileMap.cpp deja
    // dicho por que este motor no usa <filesystem>: "obliga a enlazar
    // stdc++fs en algunos toolchains". No se va a traer esa dependencia
    // para listar cuatro ficheros. Y encaja con el criterio del resto:
    // quien es dueno de que se DECLARA, no se deduce.
    //
    // assetsRoot suele ser "assets" (rutas relativas al cwd, igual que
    // hace el resto del motor: ver FORMATO_NIVELES.md).
    static Result<ProjectIndex> scan(const std::string& assetsRoot = "assets");

    const std::vector<Project>& projects() const { return m_projects; }
    const Project* find(const std::string& id) const;
    std::size_t size() const { return m_projects.size(); }

    // Ficheros de assets que no reclama ningun manifiesto. Sin esto, el
    // contenido de nadie se pierde en silencio.
    const std::vector<std::string>& orphans() const { return m_orphans; }

    // Comprueba que cada fichero declarado exista de verdad. La
    // conectividad y las aventuras las validan los scripts de tools/,
    // que ya lo hacen bien; aqui interesa lo que el editor necesita
    // saber ANTES de abrir nada: si el pack esta entero.
    ProjectCheck check(const std::string& id) const;

    // Esqueleto de proyecto nuevo: escribe el manifiesto, lo anade al
    // indice y devuelve el proyecto vacio. No crea niveles -- eso lo hace
    // el editor al guardar el primero.
    static Result<Project> create(const std::string& assetsRoot,
                                  const std::string& id,
                                  const std::string& name,
                                  const std::string& prefix,
                                  const std::string& author,
                                  const std::string& epoch);

    // Quita el proyecto del indice y borra su manifiesto. NO borra niveles
    // ni mapas: son ficheros que puede estar usando otro proyecto, y un
    // borrado en cascada desde el editor es justo la clase de operacion
    // que no se puede deshacer. Quien quiera limpiar assets, que mire los
    // huerfanos que reporta orphans().
    static Result<bool> remove(const std::string& assetsRoot, const std::string& id);

private:
    std::vector<Project> m_projects;
    std::vector<std::string> m_orphans;
    std::string m_root;
};

}  // namespace Editor
