#include "Game/PauseMenu.h"
#include "Game/GameOverScreen.h"
#include "Game/InventoryScreen.h"
#include "TestHarness.h"

namespace {

using Game::PauseMenu;
using Game::GameOverScreen;
using Game::InventoryScreen;
using Game::PauseAction;
using Game::GameOverAction;
using Game::InventoryFocus;
using Game::InventoryAction;
using Game::InventoryEntry;

MOTOR_TEST_CASE(pause_menu_navigation) {
    PauseMenu menu = PauseMenu::create();
    MOTOR_CHECK(menu.entries().size() == 8);
    MOTOR_CHECK(menu.labels().size() == 8);
    
    // Initial selection
    MOTOR_CHECK(menu.selected() == 0);
    MOTOR_CHECK(menu.selectedEntry().action == PauseAction::Resume);
    
    // Move down
    menu.moveDown();
    MOTOR_CHECK(menu.selected() == 1);
    MOTOR_CHECK(menu.selectedEntry().action == PauseAction::Character);
    
    // Move down to end
    for (int i = 0; i < 6; ++i) menu.moveDown();
    MOTOR_CHECK(menu.selected() == 7);
    MOTOR_CHECK(menu.selectedEntry().action == PauseAction::Quit);
    
    // Wraparound down
    menu.moveDown();
    MOTOR_CHECK(menu.selected() == 0);
    MOTOR_CHECK(menu.selectedEntry().action == PauseAction::Resume);
    
    // Move up (wraparound up)
    menu.moveUp();
    MOTOR_CHECK(menu.selected() == 7);
    MOTOR_CHECK(menu.selectedEntry().action == PauseAction::Quit);
}

MOTOR_TEST_CASE(pause_menu_confirm) {
    PauseMenu menu = PauseMenu::create();
    MOTOR_CHECK(menu.confirm() == PauseAction::Resume);
    
    menu.moveDown(); // Character
    MOTOR_CHECK(menu.confirm() == PauseAction::Character);
    
    menu.moveDown(); // Inventory
    MOTOR_CHECK(menu.confirm() == PauseAction::Inventory);
}

MOTOR_TEST_CASE(game_over_screen_navigation) {
    GameOverScreen screen = GameOverScreen::create();
    MOTOR_CHECK(screen.entries().size() == 2);
    MOTOR_CHECK(screen.labels().size() == 2);
    
    MOTOR_CHECK(screen.selected() == 0);
    MOTOR_CHECK(screen.selectedEntry().action == GameOverAction::Load);
    
    screen.moveDown();
    MOTOR_CHECK(screen.selected() == 1);
    MOTOR_CHECK(screen.selectedEntry().action == GameOverAction::Title);
    
    // Wraparound
    screen.moveDown();
    MOTOR_CHECK(screen.selected() == 0);
    MOTOR_CHECK(screen.selectedEntry().action == GameOverAction::Load);
    
    screen.moveUp();
    MOTOR_CHECK(screen.selected() == 1);
    MOTOR_CHECK(screen.selectedEntry().action == GameOverAction::Title);
}

MOTOR_TEST_CASE(game_over_screen_confirm) {
    GameOverScreen screen = GameOverScreen::create();
    MOTOR_CHECK(screen.confirm() == GameOverAction::Load);
    
    screen.moveDown();
    MOTOR_CHECK(screen.confirm() == GameOverAction::Title);
}

MOTOR_TEST_CASE(inventory_screen_basic_navigation) {
    InventoryScreen screen;
    
    // Initial focus is Inventory
    MOTOR_CHECK(screen.focus() == InventoryFocus::Inventory);
    MOTOR_CHECK(screen.selectedInFocus() == 0);
    
    // Move tabs
    screen.moveTabRight(); // Equipment
    MOTOR_CHECK(screen.focus() == InventoryFocus::Equipment);
    MOTOR_CHECK(screen.selectedInFocus() == 0);
    
    screen.moveTabRight(); // Status
    MOTOR_CHECK(screen.focus() == InventoryFocus::Status);
    
    screen.moveTabRight(); // Actions
    MOTOR_CHECK(screen.focus() == InventoryFocus::Actions);

    // MISIONES-01 añadió el diario (Quests) y el códice (Codex) detrás de
    // Actions: la vuelta a Inventory llega después de ellas.
    screen.moveTabRight(); // Quests
    MOTOR_CHECK(screen.focus() == InventoryFocus::Quests);

    screen.moveTabRight(); // Codex
    MOTOR_CHECK(screen.focus() == InventoryFocus::Codex);

    screen.moveTabRight(); // Wraparound to Inventory
    MOTOR_CHECK(screen.focus() == InventoryFocus::Inventory);

    // Move left
    screen.moveTabLeft(); // Codex, la última
    MOTOR_CHECK(screen.focus() == InventoryFocus::Codex);
}

MOTOR_TEST_CASE(inventory_screen_entries) {
    InventoryScreen screen;
    
    // Inventory tab should have "(vacío)" entry
    auto invEntries = screen.entriesInFocus(); // focus is Inventory initially
    MOTOR_CHECK(invEntries.size() == 1);
    MOTOR_CHECK(invEntries[0].label == "(vacío)");
    
    // Switch to Equipment
    screen.moveTabRight();
    auto eqEntries = screen.entriesInFocus();
    MOTOR_CHECK(eqEntries.size() == 1); // "(sin ficha de personaje)"
    
    // Status tab
    screen.moveTabRight();
    auto statusEntries = screen.entriesInFocus();
    MOTOR_CHECK(statusEntries.size() == 3); // Ver stats, Ver rasgos, Ver condiciones
    
    // Actions tab
    screen.moveTabRight();
    auto actionEntries = screen.entriesInFocus();
    MOTOR_CHECK(actionEntries.size() == 1); // Cerrar
}

MOTOR_TEST_CASE(inventory_screen_status_labels) {
    InventoryScreen screen;
    
    // La pestaña Estado es la ficha (FichaVista::lineas) más la moral. Las
    // cuatro líneas de antes (Salud, Mana, Oro, Moral) son hoy la línea
    // «PV .. PM .. Oro .. Insp ..» de la ficha, que prueba test_ficha_vista.
    // Sin sesión no hay ficha: lo dice, y la moral sigue.
    auto labels = screen.statusLabels();
    MOTOR_CHECK(labels.size() == 2);
    if (labels.size() != 2) return;
    MOTOR_CHECK(labels[0] == "(sin ficha)");
    MOTOR_CHECK(labels[1].find("Moral") != std::string::npos);
}

} // namespace

int main() { return motor_test::runAllTests(); }