#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Project/Project.h"

// G-08 — Título y arranque del proyecto. MITAD SIN INTERFAZ.
//
// QUÉ RESUELVE. La rodaja inicial (Game::TitleScreen) arranca del ÍNDICE
// GLOBAL de proyectos y siempre ofrece la campaña de Boundington. Un juego
// exportado no es un índice: es UN proyecto. Esto es el título de ese
// proyecto, construido desde su manifiesto: su nombre (o el título que
// fija E-11), y Nueva partida, Continuar, Opciones y Salir.
//
// TRES REGLAS DEL CRITERIO QUE MANDAN EN EL DISEÑO:
//
// 1. UN MANIFIESTO QUE NO SE PUEDE LEER NO CAE EN BOUNDINGTON. Se dice
//    qué ha fallado, en palabras, y solo se ofrece Salir. Caer en otro
//    proyecto en silencio es lo peor que puede hacer un juego exportado:
//    el jugador cree que es el juego y no lo es.
//
// 2. «CONTINUAR» DESACTIVADO DICE POR QUÉ. Sin guardados, con guardados
//    rotos o de otra versión, la opción sigue en la lista, apagada y con
//    su motivo. Con guardados, apunta al más reciente de las ranuras del
//    proyecto (el mismo servicio que usa G-11 para el selector).
//
// 3. LAS OPCIONES DESACTIVADAS NO SE EJECUTAN. confirm() sobre una
//    entrada apagada devuelve None y deja el motivo en message().

namespace Game {

enum class TitleCommand { None, NewGame, Continue, LoadGame, Options, Quit };

struct TitleMenuItem {
    TitleCommand command = TitleCommand::None;
    std::string label;
    bool enabled = true;
    std::string reason;  // vacío si enabled
};

class TitleMenuState {
public:
    // manifestPath: el .json del proyecto (assets/proyectos/<id>.json).
    // assetsBase: la carpeta respecto de la que son relativas las rutas
    //   del manifiesto («assets/levels/…»); la del ejecutable en un juego
    //   exportado, el directorio de trabajo en desarrollo.
    // savesRoot: la raíz de guardados (Game::SaveSlots). Vacía = no se
    //   miran guardados y «Continuar» sale apagado con su motivo.
    static TitleMenuState fromManifest(const std::string& manifestPath,
                                       const std::string& assetsBase,
                                       const std::string& savesRoot);

    // ¿Se pudo leer el proyecto? Si no, error() dice por qué, items()
    // solo tiene Salir y project() está vacío.
    bool ok() const { return m_error.empty(); }
    const std::string& error() const { return m_error; }
    const Editor::Project& project() const { return m_project; }

    // Lo que se pinta arriba: el título de E-11 si lo hay, si no el
    // nombre del proyecto, y si no hay proyecto, un rótulo neutro.
    const std::string& title() const { return m_title; }
    const std::string& subtitle() const { return m_subtitle; }

    // --- Menú ---
    const std::vector<TitleMenuItem>& items() const { return m_items; }
    std::size_t selected() const { return m_selected; }
    // Las flechas pasan por encima de las entradas apagadas? No: se
    // pueden señalar (hay que poder leer su motivo), solo no se ejecutan.
    void moveUp();
    void moveDown();
    const TitleMenuItem& selectedItem() const;
    TitleCommand confirm();
    const std::string& message() const { return m_message; }

    // --- Lo que necesita quien arranque ---
    // Nivel de entrada, ya resuelto contra assetsBase. Vacío si no hay.
    const std::string& entryPath() const { return m_entryPath; }
    // Ranura a cargar con «Continuar»; -1 si no hay ninguna cargable.
    int continueSlot() const { return m_continueSlot; }
    const std::string& continueSummary() const { return m_continueSummary; }

private:
    void buildItems();

    Editor::Project m_project;
    std::string m_error;
    std::string m_title;
    std::string m_subtitle;
    std::string m_entryPath;
    std::string m_newGameReason;
    std::string m_continueReason;
    std::string m_loadReason;  // D0036
    int m_continueSlot = -1;
    std::string m_continueSummary;
    std::vector<TitleMenuItem> m_items;
    std::size_t m_selected = 0;
    std::string m_message;
};

}  // namespace Game

// NOTA DE UBICACIÓN. include/Game + src/Game, sin GL. Su .cpp (y el de
// SaveSlotState, que usa) se compilan dentro de cmake/TitleFlowTests.cmake
// y no en una librería todavía: meterlos obliga a editar el bloque
// compartido del CMakeLists. Quien cablee la escena (TitleScene,
// Application, examples/juego.cpp --proyecto) los mueve.
