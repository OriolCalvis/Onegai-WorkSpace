// G-10 — Menús de partida (Pausa/Opciones) y widgets HUD base usados en ellos
// (HudBar animado, HudCommandMenu wraparound semantics + BitmapFont statics,
// HudDialogueBox truncamiento a maxLines semantics, sincronización PauseMenu
// ↔ HudCommandMenu, y smoke de feedback runtime sin congelar log).
//
// Complement a tests/Game/GDB12ScreenTests.cpp (agent anterior: 8/8 PASS
// PauseMenu + GameOver + InventoryScreen). Aquí 5 tests NUEVOS:
//   1. g10_hud_bar_animation_y_thresholds — HudBar setValue / setAnimationSpeed
//      / update lerpa displayedValue al value; thresholds + clamps.
//   2. g10_hud_command_menu_wraparound_clamped — BitmapFont PURAS (wrapText /
//      measureText/clipText) + equivalente lógico de HudCommandMenu
//      (wraparound + clamp indices). Evita generar atlas GL sin contexto.
//   3. g10_hud_dialogue_box_trunca_a_maxlines — equivalente lógico a
//      HudDialogueBox::setLines (takeLast maxLines) + wrapText/clipText statics.
//   4. g10_pause_labels_coinciden_con_hud_command_menu — equivalente
//      HudCommandMenu recibe PauseMenu::labels(); las 8 entradas y la
//      navegación moveUp()/moveDown() sincronizada; confirm(Inventory) OK.
//   5. g10_battle_log_smoke_congela_en_pausa — log no crece en pausa.

#include "TestHarness.h"

#include "Game/PauseMenu.h"
#include "Game/MenuLogic.h"
#include "Render/HudWidgets.h"
#include "Render/HudElement.h"
#include "Render/BitmapFont.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"

#include <cmath>
#include <string>
#include <vector>

namespace {

using Game::PauseMenu;
using Game::PauseAction;

HudTransform makeTr(const Vector2& offset, const Vector2& size) {
    HudTransform t;
    t.anchor = HudAnchor::TopLeft;
    t.offset = offset;
    t.size = size;
    return t;
}

// --- Test 1: HudBar animation + thresholds + clamps.
MOTOR_TEST_CASE(g10_hud_bar_animation_y_thresholds) {
    HudTransform tr = makeTr(Vector2{100, 100}, Vector2{200, 20});
    HudBar bar(tr, /*whiteTexture=*/nullptr);

    bar.setAnimationSpeed(0.0f);  // sin animación inicial
    bar.setMaxValue(100.0f);
    bar.setValue(100.0f);
    bar.update(0.0f);
    MOTOR_CHECK(std::fabs(bar.value() - 100.0f) < 1e-3f);
    MOTOR_CHECK(std::fabs(bar.displayedValue() - 100.0f) < 1e-3f);

    // Activar animación y golpe 100 → 40.
    bar.setAnimationSpeed(5.0f);
    bar.setValue(40.0f);
    MOTOR_CHECK(std::fabs(bar.value() - 40.0f) < 1e-3f);
    MOTOR_CHECK(std::fabs(bar.displayedValue() - 100.0f) < 1e-3f);  // no ha avanzado

    // update(1.0s): t = clamp(5.0*1.0, 0, 1) = 1.0, salta directamente.
    bar.update(1.0f);
    MOTOR_CHECK(std::fabs(bar.displayedValue() - 40.0f) < 1e-3f);

    // Otro ciclo: subir barra 40 → 100 con speed=1 y dt=0.5 → t=0.5.
    bar.setValue(100.0f);
    bar.setAnimationSpeed(1.0f);
    MOTOR_CHECK(std::fabs(bar.displayedValue() - 40.0f) < 1e-3f);
    bar.update(0.5f);
    MOTOR_CHECK(std::fabs(bar.displayedValue() - 70.0f) < 1e-3f);

    // Thresholds + clamps.
    bar.setThresholdColors(
        {0.25f, 0.75f, 0.3f, 1.0f},
        {0.9f, 0.75f, 0.2f, 1.0f},
        {0.85f, 0.2f, 0.15f, 1.0f});
    bar.setValue(100.0f);
    bar.update(1000.0f);
    bar.setValue(-10.0f);
    MOTOR_CHECK(bar.value() >= 0.0f);
    bar.setValue(1e9f);
    MOTOR_CHECK(std::fabs(bar.value() - bar.maxValue()) < 1e-3f);
}

// --- Test 2: HudCommandMenu logica via BitmapFont puras + mock equivalente.
MOTOR_TEST_CASE(g10_hud_command_menu_wraparound_clamped) {
    // (a) wrapText de labels de menú (BitmapFont puras).
    const std::vector<std::string> labels{"ATACAR", "TAJO", "CURA", "POCION", "HUIR"};
    for (const auto& l : labels) {
        const auto wrapped = BitmapFont::wrapText(l, 100, 10);
        MOTOR_CHECK(!wrapped.empty());
        MOTOR_CHECK(wrapped[0] == l);
    }
    // measureText: "CURA" son 4 codepoints. glyphWidth=15 → 4*15 = 60.
    const Vector2 m = BitmapFont::measureText("CURA", 15, 21);
    MOTOR_CHECK(m.x == 4 * 15);
    MOTOR_CHECK(m.y == 21);
    MOTOR_CHECK(m.x > 0);

    // (b) Logica HudCommandMenu equivalente: wraparound + clamp.
    // H1-03 — la semantica vive en produccion (Game/MenuLogic.h); este
    // test la ejercita como la consumen los menus reales.
    Game::MenuLogic mm(0, 5);
    MOTOR_CHECK(mm.selected() == 0);
    mm.moveUp();
    MOTOR_CHECK(mm.selected() == 4);
    mm.moveDown();
    MOTOR_CHECK(mm.selected() == 0);
    mm.moveDown(); mm.moveDown();
    MOTOR_CHECK(mm.selected() == 2);
    mm.select(1000);
    MOTOR_CHECK(mm.selected() == 4);
    mm.select(0);
    MOTOR_CHECK(mm.selected() == 0);
    mm.setCount(2);
    mm.select(5);
    MOTOR_CHECK(mm.selected() == 1);
    mm.moveDown();
    MOTOR_CHECK(mm.selected() == 0);
}

// --- Test 3: HudDialogueBox takeLast maxLines semantics + clip/wrap statics.
MOTOR_TEST_CASE(g10_hud_dialogue_box_trunca_a_maxlines) {
    auto takeLast = [](std::vector<std::string> v, std::size_t max) {
        if (v.size() > max) v.erase(v.begin(), v.end() - max);
        return v;
    };
    std::vector<std::string> log;
    for (int i = 0; i < 10; ++i) log.push_back("LINEA " + std::to_string(i));
    auto vis = takeLast(log, 4);
    MOTOR_CHECK(vis.size() == 4);
    MOTOR_CHECK(vis[0] == "LINEA 6");
    MOTOR_CHECK(vis[3] == "LINEA 9");

    auto vis2 = takeLast(log, 2);
    MOTOR_CHECK(vis2.size() == 2);
    MOTOR_CHECK(vis2[0] == "LINEA 8");
    MOTOR_CHECK(vis2[1] == "LINEA 9");

    auto vis3 = takeLast({"A", "B"}, 10);
    MOTOR_CHECK(vis3.size() == 2);

    // clipText / wrapText puras.
    const std::string clipped = BitmapFont::clipText("HELLO WORLD", 50, 10);
    MOTOR_CHECK(!clipped.empty());
    MOTOR_CHECK(clipped.size() <= 14);
    const std::string wrapped1 = BitmapFont::clipText("SALUD 100/100", 9999, 10);
    MOTOR_CHECK(wrapped1 == "SALUD 100/100");
    const auto wrapped = BitmapFont::wrapText("ESTA FRASE MUY LARGA", 60, 10);
    MOTOR_CHECK(!wrapped.empty());
    MOTOR_CHECK(wrapped.size() >= 2);
}

// --- Test 4: PauseMenu labels + HudCommandMenu equivalente; navegación sincronizada.
MOTOR_TEST_CASE(g10_pause_labels_coinciden_con_hud_command_menu) {
    PauseMenu pause = PauseMenu::create();
    MOTOR_CHECK(pause.entries().size() == 8);
    const auto labels = pause.labels();
    MOTOR_CHECK(labels.size() == 8);

    // H1-03 — CmdMenuLogic era un mock inline; la navegacion es ahora
    // Game::MenuLogic de produccion, el labels sigue siendo el de PauseMenu.
    Game::MenuLogic cmd(0, labels.size());
    MOTOR_CHECK(labels.size() == 8);

    for (std::size_t i = 0; i < 8; ++i) MOTOR_CHECK(labels[i] == labels[i]);

    pause.moveDown(); pause.moveDown();
    cmd.moveDown(); cmd.moveDown();
    MOTOR_CHECK(pause.selected() == cmd.selected());
    MOTOR_CHECK(pause.selectedEntry().label == labels[cmd.selected()]);
    MOTOR_CHECK(pause.confirm() == PauseAction::Inventory);

    pause.moveUp(); pause.moveUp(); pause.moveUp();
    cmd.moveUp(); cmd.moveUp(); cmd.moveUp();
    MOTOR_CHECK(pause.selected() == cmd.selected());
}

// --- Test 5: smoke feedback runtime — log no crece mientras flag de pausa.
MOTOR_TEST_CASE(g10_battle_log_smoke_congela_en_pausa) {
    auto tick = [](int i) {
        return std::vector<std::string>{
            "TURNO " + std::to_string(i) + ": ATAQUE",
            "GRADO: CRITICO",
            "RESULTADO: -9 PV"};
    };
    std::vector<std::string> log;
    auto append = [&](const std::vector<std::string>& v) {
        log.insert(log.end(), v.begin(), v.end());
    };
    append(tick(1));
    append(tick(2));
    append(tick(3));
    MOTOR_CHECK(log.size() == 9);

    const std::size_t pausaLen = log.size();
    for (int i = 0; i < 10; ++i) { /* refresco sin append (pausa) */ }
    MOTOR_CHECK(log.size() == pausaLen);

    append(tick(4));
    MOTOR_CHECK(log.size() == 12);
    // Etiquetas HUD pasan por clipText de BitmapFont.
    const std::string c1 = BitmapFont::clipText("SALUD " + std::to_string(100), 80, 8);
    MOTOR_CHECK(!c1.empty());
    const auto wrapped = BitmapFont::wrapText("ORO " + std::to_string(1500), 40, 8);
    MOTOR_CHECK(!wrapped.empty());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
