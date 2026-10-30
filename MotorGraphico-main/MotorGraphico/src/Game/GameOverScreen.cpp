#include "Game/GameOverScreen.h"

#include <string>

#include "Game/DefeatRecovery.h"

namespace Game {

namespace {

const GameOverEntry kNoEntry{};

}  // namespace

GameOverScreen GameOverScreen::create() {
    GameOverScreen screen;

    screen.m_entries.push_back(GameOverEntry{"Carregar", GameOverAction::Load});
    screen.m_entries.push_back(GameOverEntry{"Tornar al títol", GameOverAction::Title});

    screen.m_selected = 0;
    return screen;
}

GameOverScreen GameOverScreen::create(const RecoveryPlan& plan) {
    GameOverScreen screen;

    // Solo se ofrece cargar si hay algo que cargar, y se dice a dónde vuelves:
    // la fecha es lo único que el jugador reconoce sin tener que abrir la lista.
    if (plan.hasSave()) {
        std::string label = "Carregar";
        if (!plan.savedAt.empty()) {
            label += " (" + plan.savedAt + ")";
        }
        screen.m_entries.push_back(GameOverEntry{label, GameOverAction::Load});
    }
    screen.m_entries.push_back(GameOverEntry{"Tornar al títol", GameOverAction::Title});

    screen.m_selected = 0;
    return screen;
}

void GameOverScreen::moveUp() {
    if (m_entries.empty()) return;
    m_selected = (m_selected == 0) ? m_entries.size() - 1 : m_selected - 1;
}

void GameOverScreen::moveDown() {
    if (m_entries.empty()) return;
    m_selected = (m_selected + 1) % m_entries.size();
}

GameOverAction GameOverScreen::confirm() const {
    return selectedEntry().action;
}

const GameOverEntry& GameOverScreen::selectedEntry() const {
    if (m_selected >= m_entries.size()) {
        return kNoEntry;
    }
    return m_entries[m_selected];
}

std::vector<std::string> GameOverScreen::labels() const {
    std::vector<std::string> result;
    result.reserve(m_entries.size());
    for (const GameOverEntry& entry : m_entries) {
        result.push_back(entry.label);
    }
    return result;
}

}  // namespace Game