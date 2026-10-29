// EST-21b — Estado temporal de la fase de edición clásica, sin ventana.
//
// No duplica EditorState: aquel es dueño del documento, su historial,
// paletas y filtro. Tampoco duplica ObjectInspector, que ya posee la sesión
// modal de las propiedades. Esta clase reúne únicamente los datos que hacen
// falta entre frames para que la futura fase de edición no capture locales de
// main() por referencia.

#pragma once

#include <cstddef>

#include "Core/Math/GridCoord.h"

namespace EditorNiveles {

enum class ObjectPalettePage {
    Root,
    Subgroups,
    Entries,
};

enum class ReturnToHubRequest {
    ConfirmDiscard,
    ReturnToHub,
};

enum class EditPhaseExit {
    Continue,
    ReturnToHub,
    ChangeLevel,
};

// Datos efímeros de una edición abierta. El controlador traduce GLFW y el
// HUD; esta clase solo conserva las transiciones entre un frame y el siguiente.
class LevelEditorEditState {
public:
    ReturnToHubRequest requestReturnToHub(bool documentModified);
    void cancelReturnConfirmation();
    bool returnConfirmationPending() const { return m_returnConfirmationPending; }

    void requestLevelChange() { m_exit = EditPhaseExit::ChangeLevel; }
    EditPhaseExit exit() const { return m_exit; }

    bool selectionPending() const { return m_selectionPending; }
    void beginSelection(GridCoord firstCorner);
    GridCoord selectionFirstCorner() const { return m_selectionFirstCorner; }
    void finishSelection();

    bool searchOpen() const { return m_searchOpen; }
    void toggleSearch() { m_searchOpen = !m_searchOpen; }
    void closeSearch() { m_searchOpen = false; }

    ObjectPalettePage objectPalettePage() const { return m_objectPalettePage; }
    void setObjectPalettePage(ObjectPalettePage page) { m_objectPalettePage = page; }
    void moveObjectPalettePageBack();
    std::size_t objectRootGroup() const { return m_objectRootGroup; }
    void setObjectRootGroup(std::size_t group) { m_objectRootGroup = group; }
    std::size_t objectGroup() const { return m_objectGroup; }
    void setObjectGroup(std::size_t group) { m_objectGroup = group; }

    void showStatusFor(int frames);
    bool statusMessageActive() const { return m_statusFrames > 0; }
    // Avanza un frame de aviso y devuelve true solo cuando acaba ahora.
    bool advanceStatusFrame();

    void tick(float deltaSeconds);
    float animationTime() const { return m_animationTime; }
    void triggerSaveFlash() { m_saveFlashAlpha = 1.0f; }
    float saveFlashAlpha() const { return m_saveFlashAlpha; }

private:
    bool m_returnConfirmationPending = false;
    bool m_selectionPending = false;
    GridCoord m_selectionFirstCorner{0, 0};
    bool m_searchOpen = false;
    ObjectPalettePage m_objectPalettePage = ObjectPalettePage::Root;
    std::size_t m_objectRootGroup = 0;
    std::size_t m_objectGroup = 0;
    int m_statusFrames = 0;
    float m_animationTime = 0.0f;
    float m_saveFlashAlpha = 0.0f;
    EditPhaseExit m_exit = EditPhaseExit::Continue;
};

}  // namespace EditorNiveles
