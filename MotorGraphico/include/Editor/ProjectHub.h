#pragma once

#include <string>
#include <vector>

#include "Editor/ProjectIndex.h"

// ---------------------------------------------------------------------
// ProjectHub — la pantalla de arranque del editor: que proyectos hay,
// abrir uno, crear uno nuevo y sacar su build. Sin abrir una ventana.
//
// POR QUE ESTA CLASE EXISTE. La primera version de esta pantalla era una
// funcion que imprimia en stdout y salia. Se llamaba "pantalla de
// arranque" y no era una pantalla: era un --help con datos. El motivo de
// fondo es que lo headless se puede probar y lo que vive dentro de
// level_editor.cpp solo se comprueba compilando con GL delante, asi que
// lo comodo acabo siendo lo entregado.
//
// La regla que sale de ahi, y que esta clase aplica: TODO el estado de la
// pantalla (que hay seleccionado, en que modo estamos, que dijo el build)
// vive aqui, GL-free y probado en demo_proyectos. level_editor.cpp solo
// pregunta que texto pintar y le pasa las teclas. Mismo criterio que
// EditorState y ProjectIndex.
//
// Si algun dia la UI cambia de HUD propio a otra cosa, esto no se toca.
// ---------------------------------------------------------------------
namespace Editor {

class ProjectHub {
public:
    // En que esta la pantalla ahora mismo.
    enum class Mode {
        List,       // la lista de proyectos, navegable
        NewProject, // escribiendo el id del proyecto nuevo
        Message,    // mostrando el resultado de algo (build, error, aviso)
    };

    // Que quiere hacer el editor despues de una tecla. La pantalla NO abre
    // niveles ni cierra ventanas: dice lo que hay que hacer y el llamador
    // lo hace. Asi se puede probar sin que exista una ventana.
    enum class Action {
        None,
        Open,   // abrir el proyecto de openId(); el editor entra a editar
        Quit,   // salir del editor
    };

    static Result<ProjectHub> load(const std::string& assetsRoot = "assets");

    Mode mode() const { return m_mode; }

    // --- Lista ---
    // Una linea por proyecto, ya formateada para el menu del HUD. Se
    // incluye el estado porque un pack sin 'entrada' se puede abrir para
    // editarlo pero no se puede jugar, y eso hay que verlo ANTES de
    // pulsar, no despues de que no pase nada.
    const std::vector<std::string>& lines() const { return m_lines; }
    std::size_t selected() const { return m_selected; }
    // El proyecto marcado, o nullptr si no hay ninguno (assets vacio).
    const Project* current() const;
    std::size_t size() const { return m_index.size(); }

    void moveUp();
    void moveDown();

    // Detalle del proyecto marcado: las lineas del panel de la derecha.
    std::vector<std::string> detail() const;

    // --- Teclas ---
    // Una sola puerta de entrada para que el editor no reparta la logica
    // por el bucle de eventos. 'key' es un caracter logico, no un codigo
    // de GLFW: la pantalla no sabe que existe GLFW.
    //   'w'/'s' mover, '\n' abrir, 'n' nuevo, 'b' build, 27 (ESC) atras/salir
    //   en NewProject: letras y '_' escriben, '\b' borra, '\n' confirma
    Action key(char k);

    // --- Proyecto nuevo ---
    const std::string& draftId() const { return m_draft; }

    // --- Resultado / mensajes ---
    // Lo ultimo que paso, para pintarlo en un panel. Vacio = nada que decir.
    const std::vector<std::string>& message() const { return m_message; }
    bool lastOk() const { return m_lastOk; }

    // Id del proyecto que hay que abrir cuando key() devuelve Open.
    const std::string& openId() const { return m_openId; }

    // Saca la build del proyecto marcado. Llama a tools/build_proyecto.py
    // y se queda con su salida.
    //
    // POR QUE SE LLAMA AL SCRIPT Y NO SE PORTA A C++. El empaquetado ya
    // esta escrito y probado ahi (sigue el cierre transitivo de
    // targetLevel, comprueba que cada objectId tenga ficha dentro). Este
    // motor no tiene ni un mkdir en src/ -- es coherente con que evite
    // <filesystem> -- asi que portarlo significa meterle creacion de
    // carpetas y copia de ficheros, y quedarse con DOS implementaciones
    // del mismo empaquetado que se separaran con el tiempo. Todo tools/ ya
    // es Python, asi que no se anade una dependencia que no estuviera.
    //
    // El id va dentro de una linea de shell: create() ya obliga a
    // [a-z0-9_], y aqui se vuelve a comprobar antes de componerla.
    void build();

    // Rehace la lista desde disco (despues de crear o borrar).
    void refresh();

private:
    void rebuildLines();
    void setMessage(bool ok, const std::string& titulo, const std::string& cuerpo);

    ProjectIndex m_index;
    std::string m_root;
    std::vector<std::string> m_lines;
    std::size_t m_selected = 0;
    Mode m_mode = Mode::List;
    std::string m_draft;                  // id que se esta tecleando
    std::vector<std::string> m_message;
    bool m_lastOk = true;
    std::string m_openId;
};

}  // namespace Editor
