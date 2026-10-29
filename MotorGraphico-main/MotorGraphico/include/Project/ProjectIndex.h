// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-11  (fase E, 5 j) - Projectes i arbre de mapes
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Project/Project.h"

// ---------------------------------------------------------------------
// ProjectIndex — los proyectos vivos del motor. Aunque el editor lo usa,
// este indice es documental y no depende de UI ni de OpenGL.
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

    // C-15 — Añade un nivel al manifiesto de un proyecto existente.
    //
    // El nivel debe existir ya en disco, dentro de la carpeta de niveles
    // que declare el propio proyecto (settings.levelsDirectory, "levels"
    // por defecto) — NO en una ruta fija: un proyecto puede tenerla en
    // otro sitio.
    //
    // levelFileName es un NOMBRE DE FICHERO, no una ruta: cualquier
    // separador se rechaza. Es idempotente: volver a añadir el mismo
    // nivel devuelve Ok sin duplicarlo, porque un generador que se
    // ejecuta dos veces es lo normal.
    static Result<bool> addLevelToProject(const std::string& assetsRoot,
                                          const std::string& projectId,
                                          const std::string& levelFileName);

    // ----------------------------------------------------------------
    // E-11 — Nombre visible arriba, id generado abajo
    // ----------------------------------------------------------------
    //
    // Hoy crear un proyecto obliga a teclear un id tecnico: [a-z0-9_],
    // sin acentos, sin espacios. Es lo que la ficha de E-11 señala como
    // el problema («el recorregut continua depenent de noms tecnics»): el
    // autor quiere escribir «La llave del santuario» y no
    // «la_llave_del_santuario», y menos aun tener que saber que la ñ no
    // vale.
    //
    // Asi que el autor escribe un NOMBRE y el id sale de el. Los dos se
    // guardan: el nombre es lo que se enseña y se puede cambiar cuando
    // quiera; el id es la identidad, no cambia nunca y es lo que aparece
    // en las rutas y en las referencias entre ficheros.

    // "La llave del santuario" -> "la_llave_del_santuario".
    // Minusculas, sin acentos ni ñ, y cualquier otra cosa se convierte en
    // un solo "_". Error —y no un id raro— si del nombre no sale ni un
    // caracter util: "¿?" no es un proyecto.
    static Result<std::string> suggestId(const std::string& visibleName);

    // El mismo id, garantizado libre: si "cofre" esta ocupado devuelve
    // "cofre_2", luego "cofre_3". No falla nunca por colision, porque
    // fallar obligaria al autor a inventarse un nombre distinto para un
    // proyecto que se puede llamar igual que otro.
    static std::string uniqueId(const std::string& baseId,
                                const std::vector<std::string>& taken);

    // Crear desde el nombre visible. El id se genera y se hace unico
    // contra los proyectos que ya hay Y contra los manifiestos sueltos.
    static Result<Project> createNamed(const std::string& assetsRoot,
                                       const std::string& visibleName,
                                       const std::string& prefix = "",
                                       const std::string& author = "",
                                       const std::string& epoch = "");

    // Lo que queda despues de duplicar. Se devuelve y no se imprime
    // porque quien duplica tiene que poder enseñarlo: «se han copiado 3
    // niveles y 2 catalogos siguen compartidos» es informacion que el
    // autor necesita ANTES de ponerse a editar la copia.
    struct DuplicateReport {
        Project project;
        // Ficheros copiados con el prefijo nuevo: son suyos y editarlos
        // no toca el original.
        std::vector<std::string> copied;
        // Ficheros que NO llevaban el prefijo del proyecto original y por
        // tanto no son suyos: se referencian igual y se COMPARTEN.
        // Copiarlos seria duplicar contenido de otro.
        std::vector<std::string> shared;
    };

    // Duplicar un proyecto entero.
    //
    // QUE SE COPIA Y QUE NO. Lo que lleva el prefijo del proyecto es
    // suyo: se copia con el prefijo nuevo. Lo que no lo lleva es
    // contenido compartido —un catalogo del mundo, un mapa comun— y se
    // sigue referenciando sin tocarlo. Copiar eso seria quedarse con una
    // version congelada del trabajo de otro sin que nadie lo pidiera.
    //
    // TODO O NADA. Si algo falla a mitad, se borra lo que se hubiera
    // escrito y el indice se queda exactamente como estaba. Es el
    // criterio de cierre, con esas palabras: «cancel·lar o fallar una
    // operacio no deixa un projecte mig registrat».
    //
    // Y NO SOBREESCRIBE NADA: si el id nuevo estuviera ocupado o alguno
    // de los ficheros de destino ya existiera, falla antes de escribir.
    static Result<DuplicateReport> duplicate(const std::string& assetsRoot,
                                             const std::string& sourceId,
                                             const std::string& newVisibleName);

private:
    std::vector<Project> m_projects;
    std::vector<std::string> m_declaredIds; // incluye manifiestos invalidos/ausentes
    std::vector<std::string> m_orphans;
    std::string m_root;
};

}  // namespace Editor
