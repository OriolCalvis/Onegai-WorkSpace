// H1-03 — Game::OptionsState: la pantalla de Opciones que G-10 dejo en
// TODO, como estado GL-free (patron PauseMenu/TitleMenuState).
//
// Criterio de cierre de la ficha: «volumen, idioma, controles: la pantalla
// existe y se puede volver». Aqui:
//   1. opciones_estructura_seis_filas        — filas, orden y ajustabilidad
//   2. opciones_navegacion_wraparound        — via MenuLogic de PRODUCCION
//   3. opciones_volumenes_clamp_0_10         — ajuste con techo y suelo
//   4. opciones_idioma_cicla                 — CATALA/CASTELLA/ENGLISH
//   5. opciones_se_puede_volver              — VOLVER->Back, CONTROLES->ShowControls
//   6. opciones_display_refleja_valores      — lo que el HUD pintaria

#include "Game/OptionsState.h"
#include "Game/MenuLogic.h"

#include "TestHarness.h"

namespace {

using Game::OptionsAction;
using Game::OptionsState;

MOTOR_TEST_CASE(opciones_estructura_seis_filas) {
    OptionsState o = OptionsState::create();
    MOTOR_CHECK(o.entries().size() == 6);

    MOTOR_CHECK(o.entries()[0].key == "music" && o.entries()[0].adjustable);
    MOTOR_CHECK(o.entries()[1].key == "sfx" && o.entries()[1].adjustable);
    MOTOR_CHECK(o.entries()[2].key == "ambience" && o.entries()[2].adjustable);
    MOTOR_CHECK(o.entries()[3].key == "language" && o.entries()[3].adjustable);
    MOTOR_CHECK(o.entries()[4].key == "controls" && !o.entries()[4].adjustable);
    MOTOR_CHECK(o.entries()[5].key == "back" && !o.entries()[5].adjustable);

    MOTOR_CHECK(o.volume("music") == 8);
    MOTOR_CHECK(o.volume("sfx") == 8);
    MOTOR_CHECK(o.volume("ambience") == 8);
    MOTOR_CHECK(o.language() == "CATALA");
}

MOTOR_TEST_CASE(opciones_navegacion_wraparound) {
    OptionsState o = OptionsState::create();
    MOTOR_CHECK(o.selected() == 0);
    o.moveUp();
    MOTOR_CHECK(o.selected() == 5);  // wraparound a VOLVER
    o.moveDown();
    MOTOR_CHECK(o.selected() == 0);
    o.moveDown(); o.moveDown();
    MOTOR_CHECK(o.selected() == 2);
    MOTOR_CHECK(o.selectedEntry().key == "ambience");
}

MOTOR_TEST_CASE(opciones_volumenes_clamp_0_10) {
    OptionsState o = OptionsState::create();
    // MUSICA al maximo y no mas alla.
    for (int i = 0; i < 30; ++i) o.adjustRight();
    MOTOR_CHECK(o.volume("music") == OptionsState::kVolumeMax);
    // Bajar hasta el suelo.
    for (int i = 0; i < 40; ++i) o.adjustLeft();
    MOTOR_CHECK(o.volume("music") == OptionsState::kVolumeMin);
    // Los otros buses no se mueven al ajustar MUSICA.
    MOTOR_CHECK(o.volume("sfx") == 8);
    MOTOR_CHECK(o.volume("ambience") == 8);

    // VOLVER no es ajustable: subir hasta ahi (wraparound) y probar.
    o.moveUp();
    MOTOR_CHECK(o.selectedEntry().key == "back");
    o.adjustLeft();
    o.adjustRight();
    MOTOR_CHECK(o.volume("music") == OptionsState::kVolumeMin);  // intacto
    MOTOR_CHECK(o.language() == "CATALA");
}

MOTOR_TEST_CASE(opciones_idioma_cicla) {
    OptionsState o = OptionsState::create();
    for (int i = 0; i < 3; ++i) o.moveDown();  // fila IDIOMA
    MOTOR_CHECK(o.selectedEntry().key == "language");

    o.adjustRight();
    MOTOR_CHECK(o.language() == "CASTELLA");
    o.adjustRight();
    MOTOR_CHECK(o.language() == "ENGLISH");
    o.adjustRight();
    MOTOR_CHECK(o.language() == "CATALA");  // cicla
    o.adjustLeft();
    MOTOR_CHECK(o.language() == "ENGLISH");  // y a la inversa

    // Ajustar idioma no toca volumenes.
    MOTOR_CHECK(o.volume("music") == 8);
}

MOTOR_TEST_CASE(opciones_se_puede_volver) {
    OptionsState o = OptionsState::create();
    MOTOR_CHECK(o.confirm() == OptionsAction::None);  // MUSICA se ajusta, no se confirma

    for (int i = 0; i < 4; ++i) o.moveDown();  // CONTROLES
    MOTOR_CHECK(o.confirm() == OptionsAction::ShowControls);

    o.moveDown();  // VOLVER
    MOTOR_CHECK(o.confirm() == OptionsAction::Back);  // «se puede volver»
}

MOTOR_TEST_CASE(opciones_display_refleja_valores) {
    OptionsState o = OptionsState::create();
    for (int i = 0; i < 2; ++i) o.adjustRight();  // MUSICA 8 -> 10
    MOTOR_CHECK(o.volume("music") == 10);

    const auto lines = o.displayLines();
    MOTOR_CHECK(lines.size() == 6);
    bool found = false;
    for (const auto& l : lines) {
        if (l.find("MUSICA") != std::string::npos && l.find("10") != std::string::npos)
            found = true;
    }
    MOTOR_CHECK(found);

    const auto labels = o.labels();
    MOTOR_CHECK(labels.size() == 6);
    MOTOR_CHECK(labels[5] == "VOLVER");

    // MenuLogic de produccion acompana: misma semantica que el menu.
    Game::MenuLogic nav(0, 6);
    nav.moveUp();
    MOTOR_CHECK(nav.selected() == 5);
    nav.select(1000);
    MOTOR_CHECK(nav.selected() == 5);
    nav.setCount(2);
    MOTOR_CHECK(nav.selected() == 1);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
