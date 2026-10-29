// H1-03 — implementacion de Game::OptionsState (ver cabecera).
// Patron PauseMenu: estado puro GL-free, Application lo pinta.

#include "Game/OptionsState.h"

#include <cstddef>
#include <string>
#include <vector>

namespace Game {

namespace {

// Idiomas del ciclo de la fila IDIOMA. El consumo real es G-17/H1-05
// (Locale); aqui solo se guarda cual esta elegido.
const std::vector<std::string>& languagesList() {
    static const std::vector<std::string> kLanguages{"CATALA", "CASTELLA", "ENGLISH"};
    return kLanguages;
}

std::string volumeText(int v) { return std::to_string(v); }

}  // namespace

OptionsState OptionsState::create() {
    OptionsState o;
    o.m_entries = {
        {"music", "MUSICA", "8", true},
        {"sfx", "EFECTOS", "8", true},
        {"ambience", "AMBIENTE", "8", true},
        {"language", "IDIOMA", languagesList()[0], true},
        {"controls", "CONTROLES", "", false},
        {"back", "VOLVER", "", false},
    };
    o.m_nav = MenuLogic(0, o.m_entries.size());
    o.refreshValues();
    return o;
}

const std::vector<std::string>& OptionsState::languages() { return languagesList(); }

void OptionsState::moveUp() { m_nav.moveUp(); }
void OptionsState::moveDown() { m_nav.moveDown(); }

void OptionsState::adjustLeft() {
    const OptionsEntry& e = m_entries[m_nav.selected()];
    if (e.key == "language") {
        m_langIndex = (m_langIndex - 1 + static_cast<int>(languagesList().size())) %
                      static_cast<int>(languagesList().size());
    } else if (e.key == "music") {
        if (m_music > kVolumeMin) --m_music;
    } else if (e.key == "sfx") {
        if (m_sfx > kVolumeMin) --m_sfx;
    } else if (e.key == "ambience") {
        if (m_ambience > kVolumeMin) --m_ambience;
    } else {
        return;  // CONTROLES/VOLVER no ajustan
    }
    refreshValues();
}

void OptionsState::adjustRight() {
    const OptionsEntry& e = m_entries[m_nav.selected()];
    if (e.key == "language") {
        m_langIndex = (m_langIndex + 1) % static_cast<int>(languagesList().size());
    } else if (e.key == "music") {
        if (m_music < kVolumeMax) ++m_music;
    } else if (e.key == "sfx") {
        if (m_sfx < kVolumeMax) ++m_sfx;
    } else if (e.key == "ambience") {
        if (m_ambience < kVolumeMax) ++m_ambience;
    } else {
        return;
    }
    refreshValues();
}

OptionsAction OptionsState::confirm() const {
    const std::string& key = m_entries[m_nav.selected()].key;
    if (key == "back") return OptionsAction::Back;
    if (key == "controls") return OptionsAction::ShowControls;
    return OptionsAction::None;
}

std::size_t OptionsState::selected() const { return m_nav.selected(); }

const OptionsEntry& OptionsState::selectedEntry() const { return m_entries[m_nav.selected()]; }

const std::vector<OptionsEntry>& OptionsState::entries() const { return m_entries; }

std::vector<std::string> OptionsState::labels() const {
    std::vector<std::string> out;
    out.reserve(m_entries.size());
    for (const OptionsEntry& e : m_entries) out.push_back(e.label);
    return out;
}

std::vector<std::string> OptionsState::displayLines() const {
    std::vector<std::string> out;
    out.reserve(m_entries.size());
    for (const OptionsEntry& e : m_entries) {
        out.push_back(e.value.empty() ? e.label : e.label + "  " + e.value);
    }
    return out;
}

void OptionsState::setVolume(const std::string& key, int value) {
    const int v = value < kVolumeMin ? kVolumeMin : (value > kVolumeMax ? kVolumeMax : value);
    if (key == "music") m_music = v;
    else if (key == "sfx") m_sfx = v;
    else if (key == "ambience") m_ambience = v;
    else return;  // CONTROLES/VOLVER no tienen volumen, y una clave inventada tampoco
    refreshValues();
}

bool OptionsState::setLanguage(const std::string& language) {
    const std::vector<std::string>& lista = languagesList();
    for (std::size_t i = 0; i < lista.size(); ++i) {
        if (lista[i] == language) {
            m_langIndex = static_cast<int>(i);
            refreshValues();
            return true;
        }
    }
    return false;
}

int OptionsState::volume(const std::string& key) const {
    if (key == "music") return m_music;
    if (key == "sfx") return m_sfx;
    if (key == "ambience") return m_ambience;
    return 0;
}

const std::string& OptionsState::language() const { return languagesList()[m_langIndex]; }

void OptionsState::refreshValues() {
    for (OptionsEntry& e : m_entries) {
        if (e.key == "music") e.value = volumeText(m_music);
        else if (e.key == "sfx") e.value = volumeText(m_sfx);
        else if (e.key == "ambience") e.value = volumeText(m_ambience);
        else if (e.key == "language") e.value = languagesList()[m_langIndex];
    }
}

}  // namespace Game
