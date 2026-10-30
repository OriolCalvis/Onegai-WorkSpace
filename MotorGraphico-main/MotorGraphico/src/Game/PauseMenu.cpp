#include "Game/PauseMenu.h"

namespace Game {

namespace {

// Entrada neutra para selectedEntry() cuando la lista está vacía
// (defensivo: no debería pasar con el menú estándar).
const PauseEntry kNoEntry{};

}  // namespace

PauseMenu PauseMenu::create() {
    PauseMenu menu;

    menu.m_entries.push_back(PauseEntry{"Reprender", PauseAction::Resume});
    menu.m_entries.push_back(PauseEntry{"Personaje", PauseAction::Character});
    menu.m_entries.push_back(PauseEntry{"Inventario/Equip", PauseAction::Inventory});
    menu.m_entries.push_back(PauseEntry{"Desar", PauseAction::Save});
    menu.m_entries.push_back(PauseEntry{"Carregar", PauseAction::Load});
    menu.m_entries.push_back(PauseEntry{"Opcions", PauseAction::Options});
    menu.m_entries.push_back(PauseEntry{"Tornar al títol", PauseAction::Title});
    menu.m_entries.push_back(PauseEntry{"Sortir", PauseAction::Quit});

    menu.m_selected = 0;
    return menu;
}

void PauseMenu::moveUp() {
    if (m_entries.empty()) return;
    m_selected = (m_selected == 0) ? m_entries.size() - 1 : m_selected - 1;
}

void PauseMenu::moveDown() {
    if (m_entries.empty()) return;
    m_selected = (m_selected + 1) % m_entries.size();
}

PauseAction PauseMenu::confirm() const {
    return selectedEntry().action;
}

const PauseEntry& PauseMenu::selectedEntry() const {
    if (m_selected >= m_entries.size()) {
        return kNoEntry;
    }
    return m_entries[m_selected];
}

std::vector<std::string> PauseMenu::labels() const {
    std::vector<std::string> out;
    out.reserve(m_entries.size());
    for (const PauseEntry& entry : m_entries) {
        out.push_back(entry.label);
    }
    return out;
}

// AV15-05a — ver PauseMenu.h.
void PauseMenu::pregunta(PauseAction accion, const std::string& titulo, const std::string& detalle,
                         const std::string& no, const std::string& si) {
    if (accion == PauseAction::None) return;
    if (!preguntando()) m_menu = m_entries;
    m_entries = {PauseEntry{no, PauseAction::None}, PauseEntry{si, PauseAction::None}};
    m_selected = 0;
    m_pendiente = accion;
    m_titulo = titulo;
    m_detalle = detalle;
}

void PauseMenu::olvidaPregunta() {
    if (!preguntando()) return;
    m_entries = m_menu;
    m_selected = 0;
    for (std::size_t i = 0; i < m_entries.size(); ++i)
        if (m_entries[i].action == m_pendiente) m_selected = i;
    m_pendiente = PauseAction::None;
    m_titulo.clear();
    m_detalle.clear();
}

}  // namespace Game
