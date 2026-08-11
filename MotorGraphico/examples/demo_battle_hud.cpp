// Fase 9 (motor_grafico_gantt_rpg.puml), demo de cierre: integra TODO el
// subsistema RPG construido en las Fases 6-9 sobre un contexto GL real --
// BattleState (Fase 8, combate por turnos) dibujado con los widgets de
// texto nuevos (HudCommandMenu/HudDialogueBox, Fase 9) mas un HudBar de
// vida (Fase 9, ya existente) para el HP del heroe. Mismo pipeline base
// que demo_hud.cpp (Window + SpriteBatch dedicada + HudManager con su
// propia proyeccion de pantalla), pero SIN mundo isometrico detras (no
// hace falta TileMap/IsometricRenderer para probar el HUD de combate):
// solo glClear() + el overlay de HUD.
//
// GL-dependiente (BitmapFont::generateAtlas() hace glGenTextures/
// glTexImage2D de verdad): en este entorno de trabajo sin GLFW/contexto
// GL real, solo se puede syntax-check (-fsyntax-only), no compilar+
// enlazar+ejecutar -- ver README, "verificacion" para el resto de demos
// en la misma situacion (demo_lighting.cpp, demo_fog.cpp, demo_lut.cpp,
// demo_hud.cpp). En una maquina con GLFW real (el propio Mac del
// usuario), CMake lo construye y ejecuta como cualquier otro demo GL.
//
// Verifica (con Window real):
//  1. HudCommandMenu: moveUp()/moveDown() con wraparound sobre las 4
//     opciones clasicas (Atacar/Habilidad/Objeto/Huir).
//  2. Un combate de BattleState (Fase 8) resuelto por completo (ataque
//     basico + UseSkill + turno enemigo) y su log() volcado en un
//     HudDialogueBox (Fase 9) con setLines(), recortado a las ultimas N
//     lineas.
//  3. Un frame completo con Window real: el pipeline compone HudBar +
//     HudCommandMenu + HudDialogueBox con la SpriteBatch/HudManager de
//     siempre, glGetError()==0 al final.
#include "Core/Math/Vector4.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/ShaderManager.h"
#include "Core/Resources/Texture.h"
#include "Engine/Window.h"
#include "Game/BattleState.h"
#include "Render/BitmapFont.h"
#include "Render/HudElement.h"
#include "Render/HudManager.h"
#include "Render/HudTextWidgets.h"
#include "Render/HudWidgets.h"
#include "Render/ICombatant.h"
#include "Game/Skill.h"
#include "Render/SpriteBatch.h"

#include "Check.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include <glad/glad.h>

namespace {

// Textura de 1x1 pixel blanco para HudBar/HudDialogueBox: mismo patron
// exacto que makeWhiteTexture() en demo_hud.cpp.
Texture makeWhiteTexture() {
    unsigned char whitePixel[4] = {255, 255, 255, 255};
    unsigned int glID = 0;
    glGenTextures(1, &glID);
    glBindTexture(GL_TEXTURE_2D, glID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return Texture(glID, 1, 1);
}

// ICombatant de prueba minimo (mismo que demo_skills.cpp/demo_battle.cpp,
// ver su comentario): aqui no hace falta Player/Enemy real, solo algo que
// implemente el contrato para armar un BattleState de ejemplo.
class TestCombatant : public ICombatant {
public:
    explicit TestCombatant(int maxHealth) : m_health(maxHealth), m_maxHealth(maxHealth) {}

    void takeDamage(int amount) override { m_health = std::max(0, m_health - amount); }
    void heal(int amount) override { m_health = std::clamp(m_health + amount, 0, m_maxHealth); }
    int health() const override { return m_health; }
    int maxHealth() const override { return m_maxHealth; }
    bool isAlive() const override { return m_health > 0; }

    // Stubs de los metodos RPG de ICombatant (stats/defensas/tier/id): un
    // TestCombatant de demo no necesita valores reales, solo dejar de ser
    // abstracto. Defaults neutros.
    int stat(RPG::Stat /*s*/) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "test"; }

private:
    int m_health;
    int m_maxHealth;
};

void testCommandMenuNavigation() {
    HudTransform transform;
    transform.anchor = HudAnchor::BottomLeft;
    transform.size = {200.0f, 100.0f};
    HudCommandMenu menu(transform, nullptr, {"Atacar", "Habilidad", "Objeto", "Huir"});

    require(menu.selectedIndex() == 0);
    menu.moveDown();
    require(menu.selectedIndex() == 1);
    menu.moveUp();
    menu.moveUp();  // wraparound: de 0 sube al ultimo (indice 3)
    require(menu.selectedIndex() == 3);
    menu.moveDown();  // wraparound: del ultimo baja a 0
    require(menu.selectedIndex() == 0);

    menu.setSelectedIndex(99);  // fuera de rango -> clampado al ultimo valido
    require(menu.selectedIndex() == menu.options().size() - 1);

    std::cout << "[BATTLE_HUD] HudCommandMenu::moveUp()/moveDown() con wraparound correcto.\n";
}

void testDialogueBoxFromBattleLog() {
    SkillCatalog catalog;
    catalog.add(Skill{"tajo", "Tajo", 3, 12, SkillEffect::Damage, SkillTarget::SingleEnemy});

    TestCombatant heroBody(30);
    TestCombatant slimeBody(20);
    SkillSet heroSkills(/*maxMana=*/3);
    heroSkills.learn("tajo");

    std::vector<BattleParticipant> allies{{"Heroe", &heroBody, &heroSkills}};
    std::vector<BattleParticipant> enemies{{"Slime", &slimeBody, nullptr}};
    BattleState battle(std::move(allies), std::move(enemies), &catalog);

    battle.resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", 0, ""});
    battle.resolveEnemyTurn();
    battle.resolveAllyAction(0, BattleAction{BattleActionType::UseSkill, "tajo", 0, ""});
    // Al menos 2 lineas (ataque + turno enemigo si el slime seguia vivo);
    // no se fija el numero exacto porque depende de si el slime murio
    // antes del turno enemigo (mismo criterio no-fragil que demo_battle.cpp).
    require(battle.log().size() >= 2);

    HudTransform dialogueTransform;
    dialogueTransform.anchor = HudAnchor::BottomCenter;
    dialogueTransform.size = {500.0f, 90.0f};
    HudDialogueBox dialogue(dialogueTransform, nullptr, nullptr, /*maxLines=*/3);
    dialogue.setLines(battle.log());

    std::cout << "[BATTLE_HUD] BattleState::log() (" << battle.log().size()
              << " lineas) volcado en HudDialogueBox (recortado a " << dialogue.maxLines()
              << ").\n";
}

}  // namespace

int main(int argc, char** argv) {
    const int width = 1280;
    const int height = 720;
    const int maxFrames = argc > 1 ? std::atoi(argv[1]) : -1;

    testCommandMenuNavigation();
    testDialogueBoxFromBattleLog();

    try {
        Window window(width, height, "Motor Grafico - Combate + HUD (Fase 9, cierre)");
        std::cout << "Contexto OpenGL creado correctamente.\n";

        ShaderManager shaderManager;
        auto spriteShader = shaderManager.load("sprite", "assets/shaders/sprite");
        if (!spriteShader.isOk()) {
            std::cerr << "Error cargando shader sprite: " << spriteShader.errorMessage() << "\n";
            return 1;
        }

        // --- Contenido de combate real (Fase 6-8): un heroe con una
        // habilidad contra un slime, igual que demo_battle.cpp. ---
        SkillCatalog catalog;
        catalog.add(Skill{"tajo", "Tajo", 3, 12, SkillEffect::Damage, SkillTarget::SingleEnemy});
        TestCombatant heroBody(30);
        TestCombatant slimeBody(20);
        SkillSet heroSkills(/*maxMana=*/3);
        heroSkills.learn("tajo");
        std::vector<BattleParticipant> allies{{"Heroe", &heroBody, &heroSkills}};
        std::vector<BattleParticipant> enemies{{"Slime", &slimeBody, nullptr}};
        BattleState battle(std::move(allies), std::move(enemies), &catalog);
        battle.resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", 0, ""});

        // --- HUD: fuente + textura blanca + widgets + HudManager (mismo
        // patron de composicion que demo_hud.cpp). ---
        SpriteBatch hudBatch;
        Texture whiteTexture = makeWhiteTexture();
        BitmapFont font(/*scale=*/2);

        HudTransform hpTransform;
        hpTransform.anchor = HudAnchor::TopLeft;
        hpTransform.offset = {16.0f, 16.0f};
        hpTransform.size = {200.0f, 18.0f};
        HudBar hpBar(hpTransform, &whiteTexture);
        hpBar.setMaxValue(static_cast<float>(heroBody.maxHealth()));
        hpBar.setValue(static_cast<float>(heroBody.health()));

        HudTransform menuTransform;
        menuTransform.anchor = HudAnchor::BottomLeft;
        menuTransform.offset = {16.0f, 16.0f};
        menuTransform.size = {220.0f, 100.0f};
        HudCommandMenu commandMenu(menuTransform, &font, {"Atacar", "Habilidad", "Objeto", "Huir"});

        HudTransform dialogueTransform;
        dialogueTransform.anchor = HudAnchor::BottomCenter;
        dialogueTransform.offset = {0.0f, 16.0f};
        dialogueTransform.size = {500.0f, 90.0f};
        HudDialogueBox dialogue(dialogueTransform, &font, &whiteTexture, /*maxLines=*/3);
        dialogue.setLines(battle.log());

        HudManager hud;
        hud.addElement(&hpBar);
        hud.addElement(&commandMenu);
        hud.addElement(&dialogue);

        int frame = 0;
        while (!window.shouldClose() && (maxFrames < 0 || frame < maxFrames)) {
            window.pollEvents();

            glClearColor(0.10f, 0.10f, 0.14f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            hudBatch.begin();
            hud.render(hudBatch, *spriteShader.value(), width, height);
            hudBatch.end();

            window.swapBuffers();
            ++frame;
        }

        GLenum glError = glGetError();
        std::cout << "[GL] glGetError() tras el render = " << glError << " (esperado: 0)\n";
        require(glError == GL_NO_ERROR);

        std::cout << "\nCombate + HUD de texto ejecutado sin errores GL.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error fatal al arrancar: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
