#include "LevelEditorEditState.h"

#include <algorithm>

namespace EditorNiveles {

ReturnToHubRequest LevelEditorEditState::requestReturnToHub(bool documentModified) {
    if (documentModified && !m_returnConfirmationPending) {
        m_returnConfirmationPending = true;
        return ReturnToHubRequest::ConfirmDiscard;
    }
    m_returnConfirmationPending = false;
    m_exit = EditPhaseExit::ReturnToHub;
    return ReturnToHubRequest::ReturnToHub;
}

void LevelEditorEditState::cancelReturnConfirmation() {
    m_returnConfirmationPending = false;
}

void LevelEditorEditState::beginSelection(GridCoord firstCorner) {
    m_selectionFirstCorner = firstCorner;
    m_selectionPending = true;
}

void LevelEditorEditState::finishSelection() {
    m_selectionPending = false;
}

void LevelEditorEditState::moveObjectPalettePageBack() {
    if (m_objectPalettePage == ObjectPalettePage::Entries) {
        m_objectPalettePage = ObjectPalettePage::Subgroups;
    } else if (m_objectPalettePage == ObjectPalettePage::Subgroups) {
        m_objectPalettePage = ObjectPalettePage::Root;
    }
}

void LevelEditorEditState::showStatusFor(int frames) {
    m_statusFrames = std::max(0, frames);
}

bool LevelEditorEditState::advanceStatusFrame() {
    if (m_statusFrames == 0) {
        return false;
    }
    --m_statusFrames;
    return m_statusFrames == 0;
}

void LevelEditorEditState::tick(float deltaSeconds) {
    const float elapsed = std::max(0.0f, deltaSeconds);
    m_animationTime += elapsed;
    m_saveFlashAlpha = std::max(0.0f, m_saveFlashAlpha - elapsed * 2.5f);
}

}  // namespace EditorNiveles
