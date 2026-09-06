#include "Game/TitleScreen.h"

namespace Game {

namespace {
// selectedEntry() devuelve referencia: con la lista vacía (defensivo, no
// pasa desde fromProjects) devuelve esta entrada neutra en vez de una
// referencia colgando.
const TitleEntry kNoEntry{};
}  // namespace

TitleScreen TitleScreen::fromProjects(const std::vector<Editor::Project>& projects,
                                      const std::string& campaignProjectId) {
    TitleScreen screen;

    // 1) La campaña. Su etiqueta sale del nombre del manifiesto cuando
    // existe: el título no hardcodea "Boundington" más que en el id por
    // defecto del parámetro (G-08 completo lo sacará del proyecto activo).
    std::string campaignLabel = "Campaña";
    for (const Editor::Project& p : projects) {
        if (p.id == campaignProjectId) {
            if (!p.name.empty()) {
                campaignLabel = "Campaña: " + p.name;
            }
            break;
        }
    }
    screen.m_entries.push_back(TitleEntry{campaignLabel, "", TitleAction::PlayCampaign});

    // 2) Los escenarios en trabajo, en el orden del índice. Jugable =
    // con entrada resuelta por ProjectIndex (Level o Adventure) y niveles
    // declarados; lo demás va a skipped() con su id.
    for (const Editor::Project& p : projects) {
        if (p.id == campaignProjectId) {
            continue;  // ya aparece como la campaña
        }
        const bool jugable = p.playable() &&
                             p.entryKind != Editor::Project::EntryKind::None &&
                             p.entryKind != Editor::Project::EntryKind::Missing;
        if (!jugable) {
            screen.m_skipped.push_back(p.id);
            continue;
        }
        TitleEntry entry;
        entry.label = p.name.empty() ? p.id : p.name;
        entry.projectId = p.id;
        entry.action = TitleAction::OpenProject;
        screen.m_entries.push_back(entry);
    }

    // 3) Salir, siempre al final.
    screen.m_entries.push_back(TitleEntry{"Salir", "", TitleAction::Quit});
    return screen;
}

void TitleScreen::moveUp() {
    if (m_entries.empty()) {
        return;
    }
    m_selected = (m_selected == 0) ? m_entries.size() - 1 : m_selected - 1;
}

void TitleScreen::moveDown() {
    if (m_entries.empty()) {
        return;
    }
    m_selected = (m_selected + 1) % m_entries.size();
}

TitleAction TitleScreen::confirm() const {
    return selectedEntry().action;
}

const TitleEntry& TitleScreen::selectedEntry() const {
    if (m_selected >= m_entries.size()) {
        return kNoEntry;
    }
    return m_entries[m_selected];
}

std::vector<std::string> TitleScreen::labels() const {
    std::vector<std::string> out;
    out.reserve(m_entries.size());
    for (const TitleEntry& entry : m_entries) {
        out.push_back(entry.label);
    }
    return out;
}

}  // namespace Game
