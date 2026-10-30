// H4-02 / E-11 — El modelo del panel Proyecto de motor_editor, sin ImGui.
//
// El panel (editor/src/ProjectPanel.cpp) pinta el arbol de proyectos y mapas
// y los formularios; lo que decide —que formulario esta abierto, que se
// valida, que operacion se hace y que tiene que hacer despues la sesion del
// documento— vive aqui, para que test_project_panel recorra la ruta del autor
// entera (crear proyecto, dos mapas, elegir entrada, guardar y reabrir) sin
// abrir una ventana. Es el mismo reparto que ProblemsState/ProblemsPanel.
//
// La sesion del documento sigue siendo la duena del mapa abierto: el estado
// no la toca, devuelve un Effect (abrir, cerrar o recargar) y el panel se lo
// pide a la sesion, que es quien pregunta si hay cambios sin guardar.
#pragma once

#include <map>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Editor/EditorDocumentSession.h"
#include "Editor/ProjectMaps.h"
#include "Project/Project.h"

namespace Editor {

class ProjectPanelState {
public:
    enum class Mode { None, NewProject, NewMap, DuplicateMap, RenameMap, RemoveMap, Title, Entry };

    // Lo que el formulario abierto esta editando.
    struct Form {
        Mode mode = Mode::None;
        std::string project;  // id del proyecto
        std::string level;    // fichero del nivel (duplicar, renombrar, quitar, entrada)
        std::string text;     // nombre, nombre nuevo o titulo
        int width = 16;
        int height = 16;
        int tileset = 0;    // indice en tilesets()
        std::string error;  // por que no se pudo confirmar (se ensena en el formulario)
    };

    // Lo que la sesion del documento tiene que hacer tras confirmar.
    struct Effect {
        enum class Kind { None, Open, Close, Reload } kind = Kind::None;
        std::string path;
    };

    explicit ProjectPanelState(std::string assetsRoot = "assets");

    // Vuelve a leer proyectos/index.json y los mapas de cada proyecto.
    Result<bool> reload();
    const std::string& assetsRoot() const { return m_root; }
    const std::vector<Project>& projects() const { return m_projects; }
    const Project* project(const std::string& id) const;
    const std::vector<ProjectMap>& maps(const std::string& projectId) const;

    // Abrir un formulario. Los de mapa necesitan el proyecto y, salvo
    // NewMap, el nivel; con datos que no existen el formulario no se abre.
    bool begin(Mode mode, const std::string& projectId = {}, const std::string& level = {});
    bool formOpen() const { return m_form.mode != Mode::None; }
    Form& form() { return m_form; }
    const Form& form() const { return m_form; }
    void cancel();

    // Tilesets que se pueden elegir en el formulario NewMap (se leen una vez
    // por proyecto: abrir un TMX por cada mapa no es gratis).
    const std::vector<ProjectTileset>& tilesets();

    // El id de fichero que saldria con el nombre escrito ("" si no sirve):
    // el formulario lo ensena debajo del nombre, como en la pantalla de
    // proyectos.
    std::string previewFileId() const;

    // Ejecuta el formulario. Con exito lo cierra (o, tras crear un proyecto,
    // abre el de su primer mapa) y dice que hacer con el documento; si
    // falla, el formulario sigue abierto con form().error.
    Result<Effect> confirm(const EditorDocumentSession& session);

    // El proyecto que el panel debe desplegar (el ultimo creado o tocado).
    const std::string& focusProject() const { return m_focus; }

private:
    Result<Effect> run(const EditorDocumentSession& session);
    Result<bool> refuseIfOpenAndDirty(const EditorDocumentSession& session,
                                      const std::string& path) const;

    std::string m_root;
    std::vector<Project> m_projects;
    std::map<std::string, std::vector<ProjectMap>> m_maps;
    std::map<std::string, std::vector<ProjectTileset>> m_tilesets;
    Form m_form;
    std::string m_focus;
};

}  // namespace Editor
