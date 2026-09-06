#include "Game/TitleScreen.h"

#include "TestHarness.h"

#include <string>
#include <vector>

namespace {

using Game::TitleAction;
using Game::TitleScreen;

// Los proyectos del test se construyen a mano (structs simples): el
// título no llama a ProjectIndex, recibe la lista ya resuelta, así que
// la prueba no necesita assets ni I/O.
Editor::Project proyecto(const std::string& id, const std::string& name,
                         const std::string& entry,
                         Editor::Project::EntryKind kind,
                         const std::vector<std::string>& levels) {
    Editor::Project p;
    p.id = id;
    p.name = name;
    p.entry = entry;
    p.entryKind = kind;
    p.entryPath = entry.empty() ? std::string() : "assets/levels/" + entry;
    p.levels = levels;
    return p;
}

MOTOR_TEST_CASE(lists_campaign_playable_projects_and_quit) {
    const auto screen = TitleScreen::fromProjects({
        proyecto("boundington", "Los Perdidos de Boundington",
                 "boundington_prologo.json", Editor::Project::EntryKind::Adventure,
                 {"interior_vacio.json"}),
        proyecto("este_norte", "Egaroth - cuadrante este norte",
                 "ciudad_en_guskedor.json", Editor::Project::EntryKind::Level,
                 {"ciudad_en_guskedor.json"}),
    });

    MOTOR_CHECK(screen.entries().size() == 3);
    MOTOR_CHECK(screen.entries()[0].action == TitleAction::PlayCampaign);
    MOTOR_CHECK(screen.entries()[0].label == "Campaña: Los Perdidos de Boundington");
    MOTOR_CHECK(screen.entries()[1].action == TitleAction::OpenProject);
    MOTOR_CHECK(screen.entries()[1].projectId == "este_norte");
    MOTOR_CHECK(screen.entries()[1].label == "Egaroth - cuadrante este norte");
    MOTOR_CHECK(screen.entries()[2].action == TitleAction::Quit);
    MOTOR_CHECK(screen.labels().size() == screen.entries().size());
    MOTOR_CHECK(screen.skipped().empty());
}

MOTOR_TEST_CASE(skips_projects_without_playable_entry_and_reports_them) {
    const auto screen = TitleScreen::fromProjects({
        proyecto("mundo", "Mundo", "", Editor::Project::EntryKind::None, {"a.json"}),
        proyecto("roto", "Roto", "x.json", Editor::Project::EntryKind::Missing, {"a.json"}),
        proyecto("ok", "OK", "a.json", Editor::Project::EntryKind::Level, {"a.json"}),
    });

    MOTOR_CHECK(screen.entries().size() == 3);  // campaña + ok + salir
    MOTOR_CHECK(screen.entries()[1].projectId == "ok");
    MOTOR_CHECK(screen.skipped().size() == 2);
    MOTOR_CHECK(screen.skipped()[0] == "mundo");
    MOTOR_CHECK(screen.skipped()[1] == "roto");
}

MOTOR_TEST_CASE(navigation_wraps_around) {
    auto screen = TitleScreen::fromProjects({
        proyecto("ok", "OK", "a.json", Editor::Project::EntryKind::Level, {"a.json"}),
    });
    MOTOR_CHECK(screen.entries().size() == 3);
    MOTOR_CHECK(screen.selected() == 0);

    screen.moveUp();
    MOTOR_CHECK(screen.selected() == 2);
    screen.moveDown();
    MOTOR_CHECK(screen.selected() == 0);
    screen.moveDown();
    MOTOR_CHECK(screen.selected() == 1);
}

MOTOR_TEST_CASE(confirm_reports_the_selected_action) {
    auto screen = TitleScreen::fromProjects({
        proyecto("este_norte", "Este Norte", "a.json", Editor::Project::EntryKind::Level, {"a.json"}),
    });

    MOTOR_CHECK(screen.confirm() == TitleAction::PlayCampaign);
    screen.moveDown();
    MOTOR_CHECK(screen.confirm() == TitleAction::OpenProject);
    MOTOR_CHECK(screen.selectedEntry().projectId == "este_norte");
    screen.moveDown();
    MOTOR_CHECK(screen.confirm() == TitleAction::Quit);
}

MOTOR_TEST_CASE(empty_index_still_offers_campaign_and_quit) {
    auto screen = TitleScreen::fromProjects({});
    MOTOR_CHECK(screen.entries().size() == 2);
    MOTOR_CHECK(screen.entries()[0].label == "Campaña");
    MOTOR_CHECK(screen.confirm() == TitleAction::PlayCampaign);
    screen.moveDown();
    MOTOR_CHECK(screen.confirm() == TitleAction::Quit);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
