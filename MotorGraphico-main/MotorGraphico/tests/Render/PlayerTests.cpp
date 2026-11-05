// Copyright 2026 Motor Grafico Isometrico Pixel Art
// Licensed under the MIT License

#include "Render/Player.h"
#include "Render/PlayerData.h"
#include "Game/PlayerControls.h"
#include "Core/Math/GridCoord.h"
#include "Core/Math/IsoMath.h"
#include "Engine/InputState.h"
#include "Game/GameSession.h"
#include "RPG/InventoryEngine.h"

#include "TestHarness.h"

#include <cmath>

// Forward declarations para mocks
class TextureAtlas;

namespace {

TextureAtlas* createMockAtlas() {
    return static_cast<TextureAtlas*>(nullptr);
}

// Clase concreta minima para probar Player (que hereda de AnimatedEntity)
// Player ya implementa todos los métodos puros virtuales, pero necesitamos
// un mock de TextureAtlas
class TestablePlayer : public Player {
public:
    TestablePlayer(GridCoord gridPosition, TextureAtlas* atlas, int tileWidth = 64, int tileHeight = 32)
        : Player(gridPosition, atlas, tileWidth, tileHeight) {}
    
    // Exponer métodos protegidos si es necesario
    using Player::gridPosition;
    using Player::setGridPosition;
    int sprite() const { return m_spriteID; }
};

// G-12: equipo real modifica la ficha que consulta Player, sin simular
// la regla de vida ni el calculo del limite en el cuerpo de la prueba.
struct EquipmentHealthFixture {
    RPG::TierRules rules;
    RPG::ClassDefinition clazz;
    RPG::CharacterSheet sheet;
    RPG::Catalogs::EquipmentCatalog equipment;

    EquipmentHealthFixture() {
        clazz.id = "guerrera";
        clazz.baseHealth = 2;
        sheet.tier = 1;
        sheet.baseStats = {1, 0, 0, 0};
        sheet.recompute_derived(rules, &clazz);
        // PART-02 (Oriol): en el Tier 1 no hay anillos; el bonus de vida, en unos brazales.
        RPG::EquipmentDefinition ring;
        ring.id = "anillo_vital";
        ring.slot = "Arms";
        ring.rarity = RPG::ItemRarity::Common;
        ring.maxHealthBonus = 10;
        equipment.insert(ring);
        MOTOR_CHECK(sheet.healthCap() == 5);
    }

    void equip() {
        RPG::InventoryEngine::add_to_inventory(sheet, "anillo_vital");
        MOTOR_CHECK(RPG::InventoryEngine::equip(
            sheet, *equipment.find("anillo_vital"), clazz, rules, equipment).isOk());
        MOTOR_CHECK(sheet.healthCap() == 15);
    }

    void unequip() {
        MOTOR_CHECK(RPG::InventoryEngine::unequip(
            sheet, RPG::EquipSlot::Arms, equipment).isOk());
        sheet.recompute_derived(rules, &clazz);
        MOTOR_CHECK(sheet.healthCap() == 5);
    }
};

} // namespace

// --- Tests de construcción ---

MOTOR_TEST_CASE(player_constructor_default_values) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    MOTOR_CHECK(player.gridPosition().x == 0);
    MOTOR_CHECK(player.gridPosition().y == 0);
    MOTOR_CHECK(player.health() == 100);
    MOTOR_CHECK(player.maxHealth() == 100);
    MOTOR_CHECK(player.isAlive());
}

MOTOR_TEST_CASE(player_constructor_with_custom_position) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{5, 10}, atlas);
    MOTOR_CHECK(player.gridPosition().x == 5);
    MOTOR_CHECK(player.gridPosition().y == 10);
}

MOTOR_TEST_CASE(player_constructor_with_custom_tile_dimensions) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas, 128, 64);
    // No hay getter para tileWidth/tileHeight, pero el constructor los acepta
    MOTOR_CHECK(true);
}

// --- Tests de movimiento y handleInput ---

MOTOR_TEST_CASE(player_handle_input_move_up) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    InputState input;
    input.moveUp = true;
    
    player.handleInput(input);
    MOTOR_CHECK(player.gridPosition().x == 0);
    MOTOR_CHECK(player.gridPosition().y == -1);
}

MOTOR_TEST_CASE(player_handle_input_move_down) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    InputState input;
    input.moveDown = true;
    
    player.handleInput(input);
    MOTOR_CHECK(player.gridPosition().x == 0);
    MOTOR_CHECK(player.gridPosition().y == 1);
}

MOTOR_TEST_CASE(player_handle_input_move_left) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    InputState input;
    input.moveLeft = true;
    
    player.handleInput(input);
    MOTOR_CHECK(player.gridPosition().x == -1);
    MOTOR_CHECK(player.gridPosition().y == 0);
}

MOTOR_TEST_CASE(player_handle_input_move_right) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    InputState input;
    input.moveRight = true;
    
    player.handleInput(input);
    MOTOR_CHECK(player.gridPosition().x == 1);
    MOTOR_CHECK(player.gridPosition().y == 0);
}

MOTOR_TEST_CASE(player_handle_input_no_movement_stays_idle) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{5, 5}, atlas);
    
    InputState input;
    // No movement keys pressed
    
    player.handleInput(input);
    MOTOR_CHECK(player.gridPosition().x == 5);
    MOTOR_CHECK(player.gridPosition().y == 5);
}

MOTOR_TEST_CASE(player_handle_input_multiple_directions_priority) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    // Test priority: up > down > left > right
    // According to Player::handleInput: if (moveUp) ... else if (moveDown) ... else if (moveLeft) ... else if (moveRight)
    // So up has highest priority
    InputState input;
    input.moveUp = true;
    input.moveDown = true;
    input.moveLeft = true;
    input.moveRight = true;
    
    player.handleInput(input);
    // Should move up (highest priority)
    MOTOR_CHECK(player.gridPosition().x == 0);
    MOTOR_CHECK(player.gridPosition().y == -1);
}

MOTOR_TEST_CASE(player_handle_input_down_over_left) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    InputState input;
    input.moveDown = true;
    input.moveLeft = true;
    
    player.handleInput(input);
    // down has priority over left
    MOTOR_CHECK(player.gridPosition().x == 0);
    MOTOR_CHECK(player.gridPosition().y == 1);
}

// --- Tests de combate (ICombatant) ---

MOTOR_TEST_CASE(player_take_damage_reduces_health) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.takeDamage(25);
    MOTOR_CHECK(player.health() == 75);
    MOTOR_CHECK(player.isAlive());
}

MOTOR_TEST_CASE(player_take_damage_kills_when_health_reaches_zero) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.takeDamage(100);
    MOTOR_CHECK(player.health() == 0);
    MOTOR_CHECK(!player.isAlive());
}

MOTOR_TEST_CASE(player_take_damage_clamped_to_zero) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.takeDamage(150);
    MOTOR_CHECK(player.health() == 0);
}

MOTOR_TEST_CASE(player_heal_increases_health) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.takeDamage(30);
    player.heal(20);
    MOTOR_CHECK(player.health() == 90);
}

MOTOR_TEST_CASE(player_heal_clamped_to_max_health) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.heal(50);
    MOTOR_CHECK(player.health() == 100); // Cannot exceed maxHealth
}

MOTOR_TEST_CASE(player_heal_from_zero) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.takeDamage(100);
    MOTOR_CHECK(!player.isAlive());
    
    player.heal(50);
    MOTOR_CHECK(player.health() == 50);
    MOTOR_CHECK(player.isAlive());
}

// --- Tests de inventario ---

MOTOR_TEST_CASE(player_add_item_increases_inventory) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    MOTOR_CHECK(player.inventory().empty());
    
    player.addItem(42);
    MOTOR_CHECK(player.inventory().size() == 1);
    MOTOR_CHECK(player.inventory()[0] == 42);
}

MOTOR_TEST_CASE(player_add_multiple_items) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.addItem(1);
    player.addItem(2);
    player.addItem(3);
    
    MOTOR_CHECK(player.inventory().size() == 3);
    MOTOR_CHECK(player.inventory()[0] == 1);
    MOTOR_CHECK(player.inventory()[1] == 2);
    MOTOR_CHECK(player.inventory()[2] == 3);
}

// --- Tests de setters ---

MOTOR_TEST_CASE(player_set_health_valid_values) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.set_health(50);
    MOTOR_CHECK(player.health() == 50);
}

MOTOR_TEST_CASE(player_set_health_clamped_to_zero) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.set_health(-10);
    MOTOR_CHECK(player.health() == 0);
}

MOTOR_TEST_CASE(player_set_health_clamped_to_max) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.set_health(200);
    MOTOR_CHECK(player.health() == 100); // clamped to maxHealth
}

MOTOR_TEST_CASE(player_set_max_health) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.set_max_health(150);
    MOTOR_CHECK(player.maxHealth() == 150);
    MOTOR_CHECK(player.health() == 100); // health stays the same if within new max
}

MOTOR_TEST_CASE(player_set_max_health_clamps_health) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.set_health(120);
    player.set_max_health(80);
    MOTOR_CHECK(player.maxHealth() == 80);
    MOTOR_CHECK(player.health() == 80); // health clamped to new max
}

MOTOR_TEST_CASE(player_set_combatant_id) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.set_combatant_id("hero");
    MOTOR_CHECK(player.combatant_id() == "hero");
}

MOTOR_TEST_CASE(player_set_tier) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.set_tier(3);
    MOTOR_CHECK(player.tier() == 3);
}

MOTOR_TEST_CASE(player_set_tier_clamped) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    player.set_tier(0);
    MOTOR_CHECK(player.tier() == 1); // min 1
    
    player.set_tier(10);
    MOTOR_CHECK(player.tier() == 5); // max 5
}

// --- Tests de herencia ---

MOTOR_TEST_CASE(player_implements_irenderable_and_iupdatable) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    // Player hereda de AnimatedEntity que hereda de Entity
    // Entity implementa IRenderable e IUpdatable
    IRenderable* renderable = &player;
    IUpdatable* updatable = &player;
    ICombatant* combatant = &player;
    
    MOTOR_CHECK(renderable != nullptr);
    MOTOR_CHECK(updatable != nullptr);
    MOTOR_CHECK(combatant != nullptr);
}

MOTOR_TEST_CASE(player_is_animated_entity) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    // Player hereda de AnimatedEntity
    AnimatedEntity* animated = &player;
    MOTOR_CHECK(animated != nullptr);
}

// ESTETICA-01: parado un momento, el jugador pasa al reposo de su rumbo (y el
// reposo se anima); entre dos pasos seguidos no le da tiempo.
MOTOR_TEST_CASE(player_pasa_al_reposo_de_su_rumbo_al_quedarse_parado) {
    TestablePlayer player(GridCoord{0, 0}, createMockAtlas());
    player.addAnimation("walk_down", {8, 7, 8, 9});
    player.addAnimation("idle_down", {17, 17, 18});
    InputState abajo;
    abajo.moveDown = true;
    player.handleInput(abajo);
    MOTOR_CHECK(player.currentAnimation() == "walk_down");
    player.update(0.16f);  // termina el paso (AV14-09: 0,16 s)
    MOTOR_CHECK(player.sprite() == 8);
    player.update(0.1f);
    MOTOR_CHECK(player.currentAnimation() == "walk_down");
    player.handleInput(abajo);  // otro paso enseguida: sigue andando
    player.update(0.16f);
    MOTOR_CHECK(player.currentAnimation() == "walk_down");
    player.update(0.15f);
    player.update(0.1f);
    MOTOR_CHECK(player.currentAnimation() == "idle_down");
    MOTOR_CHECK(player.sprite() == 17);
    player.update(0.15f);  // 0,07 s por frame: va por el tercero
    MOTOR_CHECK(player.sprite() == 18);
    player.handleInput(abajo);  // y al andar otra vez, el ciclo de andar desde el quieto
    MOTOR_CHECK(player.currentAnimation() == "walk_down");
    MOTOR_CHECK(player.sprite() == 8);
}

// Con una hoja sin reposo (las de 12 frames) se queda en el quieto de andar.
MOTOR_TEST_CASE(player_sin_reposo_se_queda_en_el_quieto_de_andar) {
    TestablePlayer player(GridCoord{0, 0}, createMockAtlas());
    player.addAnimation("walk_left", {11, 10, 11, 12});
    InputState izquierda;
    izquierda.moveLeft = true;
    player.handleInput(izquierda);
    player.update(0.14f);
    player.update(1.0f);
    MOTOR_CHECK(player.currentAnimation() == "walk_left");
    MOTOR_CHECK(player.sprite() == 11);
}

// --- Tests de CharacterSheet (Fractura #1) ---

MOTOR_TEST_CASE(player_character_sheet_default_is_nullptr) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    MOTOR_CHECK(player.character_sheet() == nullptr);
}

MOTOR_TEST_CASE(player_set_character_sheet) {
    TextureAtlas* atlas = createMockAtlas();
    TestablePlayer player(GridCoord{0, 0}, atlas);
    
    // Crear un mock de CharacterSheet
    RPG::CharacterSheet sheet;
    player.set_character_sheet(&sheet);
    
    MOTOR_CHECK(player.character_sheet() == &sheet);
}

MOTOR_TEST_CASE(player_heal_uses_the_higher_cap_after_equipping) {
    EquipmentHealthFixture f;
    Player player({0, 0}, nullptr);
    player.set_character_sheet(&f.sheet);
    f.equip();
    MOTOR_CHECK(player.health() == 5);
    MOTOR_CHECK(player.maxHealth() == 15);
    player.heal(7);
    MOTOR_CHECK(player.health() == 12);
    player.heal(100);
    MOTOR_CHECK(player.health() == 15);
}

MOTOR_TEST_CASE(player_heal_uses_the_lower_cap_after_unequipping) {
    EquipmentHealthFixture f;
    f.equip();
    Player player({0, 0}, nullptr);
    player.set_character_sheet(&f.sheet);
    player.takeDamage(12);
    f.unequip();
    MOTOR_CHECK(player.health() == 3);
    MOTOR_CHECK(player.maxHealth() == 5);
    player.heal(100);
    MOTOR_CHECK(player.health() == 5);
}

MOTOR_TEST_CASE(player_set_health_uses_the_higher_cap_after_equipping) {
    EquipmentHealthFixture f;
    Player player({0, 0}, nullptr);
    player.set_character_sheet(&f.sheet);
    f.equip();
    player.set_health(12);
    MOTOR_CHECK(player.health() == 12);
    player.set_health(100);
    MOTOR_CHECK(player.health() == 15);
}

MOTOR_TEST_CASE(player_set_health_uses_the_lower_cap_after_unequipping) {
    EquipmentHealthFixture f;
    f.equip();
    Player player({0, 0}, nullptr);
    player.set_character_sheet(&f.sheet);
    f.unequip();
    player.set_health(100);
    MOTOR_CHECK(player.health() == 5);
    player.set_health(-1);
    MOTOR_CHECK(player.health() == 0);
}

MOTOR_TEST_CASE(player_equipment_bonus_can_be_healed_by_session_potions) {
    EquipmentHealthFixture f;
    Player player({0, 0}, nullptr);
    player.set_character_sheet(&f.sheet);
    f.equip();
    ObjectDefinition potion;
    potion.id = "pocion";
    potion.name = "Pocion";
    potion.category = ObjectCategory::Pickup;
    potion.pickup.effect = PickupEffect::Heal;
    potion.pickup.power = 7;
    ObjectCatalog objects;
    objects.add(potion);
    std::vector<std::string> inventory{"pocion", "pocion", "pocion"};
    GameSession session(nullptr, &objects, nullptr, LevelDefinition{},
                        &player, nullptr, &inventory);

    MOTOR_CHECK(session.whyCannotUse("pocion").empty());
    MOTOR_CHECK(session.useItem("pocion").isOk());
    MOTOR_CHECK(player.health() == 12);
    MOTOR_CHECK(inventory.size() == 2);
    MOTOR_CHECK(session.useItem("pocion").isOk());
    MOTOR_CHECK(player.health() == 15);
    MOTOR_CHECK(inventory.size() == 1);
    MOTOR_CHECK(!session.useItem("pocion").isOk());
    MOTOR_CHECK(player.health() == player.maxHealth());
    MOTOR_CHECK(inventory.size() == 1);
}

// EST-36: los datos se construyen sin una entidad, y cada jugador conserva
// su estado propio sin copiar la ficha externa que comparte con la sesion.
MOTOR_TEST_CASE(player_data_has_legacy_defaults_without_an_entity) {
    PlayerData data;
    MOTOR_CHECK(data.health == 100);
    MOTOR_CHECK(data.maxHealth == 100);
    MOTOR_CHECK(data.inventory.empty());
    for (int stat : data.stats) MOTOR_CHECK(stat == 0);
    MOTOR_CHECK(data.equipArmor == 0);
    MOTOR_CHECK(data.tier == 1);
    MOTOR_CHECK(data.combatantId == "player");
    MOTOR_CHECK(data.sheet == nullptr);
}

MOTOR_TEST_CASE(player_instances_keep_independent_runtime_data) {
    Player first({0, 0}, nullptr);
    Player second({1, 1}, nullptr);
    first.set_max_health(80);
    first.set_health(37);
    first.set_stat(RPG::Stat::CON, 6);
    first.set_stat(RPG::Stat::DES, 3);
    first.set_equip_armor(4);
    first.set_tier(3);
    first.set_combatant_id("first");
    first.addItem(42);

    MOTOR_CHECK(second.health() == 100);
    MOTOR_CHECK(second.maxHealth() == 100);
    MOTOR_CHECK(second.stat(RPG::Stat::CON) == 0);
    MOTOR_CHECK(second.stat(RPG::Stat::DES) == 0);
    MOTOR_CHECK(second.defenses().values[0] == 10);
    MOTOR_CHECK(second.tier() == 1);
    MOTOR_CHECK(second.combatant_id() == "player");
    MOTOR_CHECK(second.inventory().empty());
    MOTOR_CHECK(second.character_sheet() == nullptr);
    second.takeDamage(9);
    second.addItem(7);
    MOTOR_CHECK(first.health() == 37);
    MOTOR_CHECK(first.maxHealth() == 80);
    MOTOR_CHECK(first.inventory().size() == 1);
    MOTOR_CHECK(first.inventory().front() == 42);
}

MOTOR_TEST_CASE(player_reads_live_character_sheet_instead_of_a_snapshot) {
    EquipmentHealthFixture f;
    f.sheet.id = "before";
    Player player({0, 0}, nullptr);
    player.set_character_sheet(&f.sheet);
    MOTOR_CHECK(player.health() == 5);
    MOTOR_CHECK(player.combatant_id() == "before");

    f.sheet.id = "after";
    f.sheet.tier = 2;
    f.sheet.baseStats = {4, 3, 2, 1};
    f.sheet.equipArmorBonus = 2;
    f.sheet.recompute_derived(f.rules, &f.clazz);
    const Player& view = player;
    MOTOR_CHECK(view.character_sheet() == &f.sheet);
    MOTOR_CHECK(view.combatant_id() == "after");
    MOTOR_CHECK(view.tier() == 2);
    MOTOR_CHECK(view.stat(RPG::Stat::CON) == 4);
    MOTOR_CHECK(view.stat(RPG::Stat::DES) == 3);
    MOTOR_CHECK(view.stat(RPG::Stat::INT) == 2);
    MOTOR_CHECK(view.stat(RPG::Stat::CAR) == 1);
    MOTOR_CHECK(view.initiative_bonus() == 3);
    MOTOR_CHECK(view.maxHealth() == f.sheet.healthCap());
    MOTOR_CHECK(view.defenses().values == f.sheet.defenseValues().values);
    MOTOR_CHECK(view.health() == 5);
    player.takeDamage(2);
    MOTOR_CHECK(view.health() == 3);
    MOTOR_CHECK(view.maxHealth() == f.sheet.healthCap());
}

MOTOR_TEST_CASE(player_binding_a_sheet_preserves_partial_health_and_legacy_cap) {
    EquipmentHealthFixture f;
    Player player({0, 0}, nullptr);
    player.set_max_health(80);
    player.set_health(3);
    player.set_character_sheet(&f.sheet);
    MOTOR_CHECK(player.health() == 3);
    MOTOR_CHECK(player.maxHealth() == 5);

    player.set_character_sheet(nullptr);
    MOTOR_CHECK(player.character_sheet() == nullptr);
    MOTOR_CHECK(player.health() == 3);
    MOTOR_CHECK(player.maxHealth() == 80);
    player.heal(100);
    MOTOR_CHECK(player.health() == 80);
}

MOTOR_TEST_CASE(player_legacy_setters_stay_separate_from_the_bound_sheet) {
    EquipmentHealthFixture f;
    f.sheet.id = "sheet";
    Player player({0, 0}, nullptr);
    player.set_health(3);
    player.set_character_sheet(&f.sheet);
    player.set_stat(RPG::Stat::CON, -1);
    player.set_stat(RPG::Stat::DES, 12);
    player.set_stat(RPG::Stat::INT, 4);
    player.set_stat(RPG::Stat::CAR, 2);
    player.set_equip_armor(6);
    player.set_tier(3);
    player.set_combatant_id("legacy");

    MOTOR_CHECK(player.stat(RPG::Stat::CON) == 1);
    MOTOR_CHECK(player.stat(RPG::Stat::DES) == 0);
    MOTOR_CHECK(player.stat(RPG::Stat::INT) == 0);
    MOTOR_CHECK(player.stat(RPG::Stat::CAR) == 0);
    MOTOR_CHECK(player.tier() == 1);
    MOTOR_CHECK(player.combatant_id() == "sheet");
    MOTOR_CHECK(player.defenses().values == f.sheet.defenseValues().values);

    player.set_character_sheet(nullptr);
    MOTOR_CHECK(player.stat(RPG::Stat::CON) == 0);
    MOTOR_CHECK(player.stat(RPG::Stat::DES) == 8);
    MOTOR_CHECK(player.stat(RPG::Stat::INT) == 4);
    MOTOR_CHECK(player.stat(RPG::Stat::CAR) == 2);
    MOTOR_CHECK(player.tier() == 3);
    MOTOR_CHECK(player.combatant_id() == "legacy");
    MOTOR_CHECK(player.defenses().values[0] == 24);
    player.set_equip_armor(-2);
    MOTOR_CHECK(player.defenses().values[0] == 18);
    MOTOR_CHECK(player.health() == 3);
    MOTOR_CHECK(player.maxHealth() == 100);
}


// CONTROLES-01: secuencias reales de InputSystem, sin reloj ni ventana del SO.
namespace {
using Engine::Input::InputKey;
using Engine::Input::RawInputState;
struct ControlsFixture {
    Engine::Input::InputSystem input;
    Player player{{0, 0}, nullptr};
    RawInputState keys;
    GridCoord poll(double now, bool enabled = true) {
        return player.movementCommand(input.update(keys), now, enabled);
    }
};
bool stopped(GridCoord step) { return step.x == 0 && step.y == 0; }
void animations(Player& player) {
    player.addAnimation("idle", {1});
    player.addAnimation("walk_up", {5, 6, 7, 8});
    player.addAnimation("walk_down", {1, 2, 3, 4});
    player.addAnimation("walk_left", {9, 10, 11, 12});
    player.addAnimation("walk_right", {9, 10, 11, 12});
    player.play("idle");
}
}

MOTOR_TEST_CASE(player_hold_moves_immediately_then_repeats_at_controlled_intervals) {
    ControlsFixture f;
    f.keys.setDown(InputKey::W);
    MOTOR_CHECK(f.poll(0).y == -1);
    MOTOR_CHECK(stopped(f.poll(0.1)));
    // AV14-09: mantenida, anda seguido desde el primer paso (antes esperaba 0,24 s).
    MOTOR_CHECK(stopped(f.poll(0.139)));
    MOTOR_CHECK(f.poll(0.14).y == -1);
    MOTOR_CHECK(stopped(f.poll(0.27)));
    MOTOR_CHECK(f.poll(0.29).y == -1);
    f.keys.setDown(InputKey::W, false);
    MOTOR_CHECK(stopped(f.poll(0.8)));
    f.keys.setDown(InputKey::W);
    MOTOR_CHECK(f.poll(0.81).y == -1);
}

MOTOR_TEST_CASE(player_turns_on_the_latest_direction_and_resumes_the_held_one) {
    ControlsFixture f;
    f.keys.setDown(InputKey::W);
    MOTOR_CHECK(f.poll(0).y == -1);
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(stopped(f.poll(0.03)));
    MOTOR_CHECK(f.poll(0.14).x == 1);
    f.keys.setDown(InputKey::D, false);
    MOTOR_CHECK(stopped(f.poll(0.16)));
    MOTOR_CHECK(f.poll(0.28).y == -1);
    f.keys.setDown(InputKey::S);
    MOTOR_CHECK(stopped(f.poll(0.30)));
    MOTOR_CHECK(stopped(f.poll(0.8)));
}

MOTOR_TEST_CASE(player_slow_frame_does_not_queue_a_burst_of_steps) {
    ControlsFixture f;
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(f.poll(0).x == 1);
    MOTOR_CHECK(f.poll(3).x == 1);
    MOTOR_CHECK(stopped(f.poll(3)));
    MOTOR_CHECK(stopped(f.poll(3.01)));
    MOTOR_CHECK(f.poll(3.15).x == 1);
}

MOTOR_TEST_CASE(player_repeat_keeps_the_same_cadence_at_different_frame_rates) {
    int counts[3]{};
    const int fps[] = {30, 60, 120};
    for (int rate = 0; rate < 3; ++rate) {
        ControlsFixture f;
        f.keys.setDown(InputKey::A);
        for (int frame = 0; frame <= fps[rate] * 2; ++frame)
            if (f.poll(static_cast<double>(frame) / fps[rate]).x == -1) ++counts[rate];
    }
    MOTOR_CHECK(counts[0] == counts[1]);
    MOTOR_CHECK(counts[1] == counts[2]);
    MOTOR_CHECK(counts[0] == 15);  // AV14-09: sin la espera de 0,24 s tras el primer paso
}

MOTOR_TEST_CASE(player_modal_or_focus_loss_requires_release_before_walking_again) {
    ControlsFixture f;
    f.keys.setDown(InputKey::W);
    MOTOR_CHECK(f.poll(0).y == -1);
    MOTOR_CHECK(stopped(f.poll(0.1, false)));
    MOTOR_CHECK(stopped(f.poll(1)));
    f.keys.setDown(InputKey::W, false);
    MOTOR_CHECK(stopped(f.poll(1.1)));
    f.keys.setDown(InputKey::W);
    MOTOR_CHECK(f.poll(1.2).y == -1);
}

MOTOR_TEST_CASE(player_visual_step_preserves_logical_position_and_offset_anchor) {
    Player player({0, 0}, nullptr);
    animations(player);
    player.setOffset({4, 7});
    player.moveTo({1, 0});
    MOTOR_CHECK(player.gridPosition().x == 1);
    MOTOR_CHECK(player.currentAnimation() == "walk_right");
    MOTOR_CHECK(std::abs(player.offset().x - (-28.0f)) < 0.001f);
    MOTOR_CHECK(std::abs(player.offset().y - (-9.0f)) < 0.001f);
    player.update(0.08f);  // AV14-09: el paso visual dura 0,16 s
    MOTOR_CHECK(std::abs(player.offset().x - (-12.0f)) < 0.001f);
    MOTOR_CHECK(std::abs(player.offset().y - (-1.0f)) < 0.001f);
    player.update(0.08f);
    MOTOR_CHECK(player.offset().x == 4);
    MOTOR_CHECK(player.offset().y == 7);
    MOTOR_CHECK(player.currentAnimation() == "walk_right");
}

MOTOR_TEST_CASE(player_blocked_step_does_not_start_a_walk_cycle) {
    Player player({3, 3}, nullptr);
    animations(player);
    player.moveTo({3, 3});
    player.update(0.1f);
    MOTOR_CHECK(player.gridPosition().x == 3);
    MOTOR_CHECK(player.offset().x == 0);
    MOTOR_CHECK(player.offset().y == 0);
    MOTOR_CHECK(player.currentAnimation() == "idle");
}

MOTOR_TEST_CASE(player_rapid_steps_continue_from_the_current_visual_position) {
    Player player({0, 0}, nullptr);
    animations(player);
    player.moveTo({1, 0});
    player.update(0.07f);
    const auto before = player.offset();
    player.moveTo({1, 1});
    // Cambia la celda proyectada (-32,+16), compensada exactamente por el offset.
    MOTOR_CHECK(std::abs(player.offset().x - (before.x + 32)) < 0.001f);
    MOTOR_CHECK(std::abs(player.offset().y - (before.y - 16)) < 0.001f);
    player.update(0.16f);
    MOTOR_CHECK(player.offset().x == 0 && player.offset().y == 0);
}

MOTOR_TEST_CASE(player_teleport_cancels_visual_motion_and_held_input) {
    ControlsFixture f;
    animations(f.player);
    f.player.setOffset({2, 3});
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(f.poll(0).x == 1);
    f.player.moveTo({1, 0});
    f.player.update(0.05f);
    f.player.setGridPosition({20, 20});
    MOTOR_CHECK(f.player.offset().x == 2 && f.player.offset().y == 3);
    MOTOR_CHECK(stopped(f.poll(0.3)));
    f.player.update(10.0f);
    MOTOR_CHECK(f.player.gridPosition().x == 20);
    MOTOR_CHECK(f.player.offset().x == 2 && f.player.offset().y == 3);
    MOTOR_CHECK(f.player.currentAnimation() == "idle");
}

MOTOR_TEST_CASE(player_escape_closes_interactions_and_never_quits_a_running_battle) {
    using Action = PlayerEscapeAction;
    MOTOR_CHECK(PlayerControls::escapeAction(GameMode::Exploration, true, false) == Action::Quit);
    MOTOR_CHECK(PlayerControls::escapeAction(GameMode::Exploration, false, false) == Action::Pause);
    MOTOR_CHECK(PlayerControls::escapeAction(GameMode::Exploration, false, true) == Action::CloseInventory);
    MOTOR_CHECK(PlayerControls::escapeAction(GameMode::Pause, false, false) == Action::Resume);
    MOTOR_CHECK(PlayerControls::escapeAction(GameMode::Dialogue, false, false) == Action::CloseInteraction);
    MOTOR_CHECK(PlayerControls::escapeAction(GameMode::Shop, false, false) == Action::CloseInteraction);
    MOTOR_CHECK(PlayerControls::escapeAction(GameMode::Business, false, false) == Action::CloseInteraction);
    MOTOR_CHECK(PlayerControls::escapeAction(GameMode::Battle, false, false) == Action::Pause);
    MOTOR_CHECK(PlayerControls::escapeAction(GameMode::GameOver, false, false) == Action::None);
}

MOTOR_TEST_CASE(player_rests_facing_the_last_direction_without_walking_in_place) {
    TestablePlayer player({0, 0}, nullptr);
    animations(player);
    player.moveTo({0, -1});
    player.update(0.16f);
    MOTOR_CHECK(player.currentAnimation() == "walk_up");
    player.update(0.06f);  // AV14-09: el quieto sale parado un momento, no entre dos pasos
    MOTOR_CHECK(player.sprite() == 5);
    const int phase = player.currentFrame();
    player.update(10.0f);
    player.handleInput({});
    MOTOR_CHECK(player.sprite() == 5);
    MOTOR_CHECK(player.currentFrame() == phase);
    player.moveTo({-1, -1});
    player.update(0.16f);
    player.update(0.06f);
    MOTOR_CHECK(player.currentAnimation() == "walk_left");
    MOTOR_CHECK(player.sprite() == 9);
}

MOTOR_TEST_CASE(player_walk_cycle_continues_across_cells_and_rest_gaps) {
    TestablePlayer player({0, 0}, nullptr);
    animations(player);
    player.moveTo({0, -1});
    player.update(0.07f);
    MOTOR_CHECK(player.sprite() == 6);
    player.update(0.07f);
    MOTOR_CHECK(player.currentFrame() == 2);
    player.update(0.1f); // Retardo del primer paso, sin reiniciar la fase.
    player.moveTo({0, -2});
    player.update(0.07f);
    MOTOR_CHECK(player.sprite() == 8);
    player.update(0.07f);
    MOTOR_CHECK(player.currentFrame() == 0);
    MOTOR_CHECK(player.sprite() == 5);
}

MOTOR_TEST_CASE(player_depth_follows_the_visual_step_and_preserves_rapid_turns) {
    Player player({2, 2}, nullptr);
    const int start = player.getSortKey();
    const int end = Player({3, 2}, nullptr).getSortKey();
    player.moveTo({3, 2});
    MOTOR_CHECK(player.getSortKey() == start);
    player.update(0.08f);
    const int halfway = player.getSortKey();
    MOTOR_CHECK(std::abs(halfway - (start + end) / 2) <= 1);
    player.moveTo({3, 1});
    MOTOR_CHECK(player.getSortKey() == halfway);
    player.update(0.16f);
    MOTOR_CHECK(player.getSortKey() == Player({3, 1}, nullptr).getSortKey());
    player.setGridPosition({-2, -2});
    MOTOR_CHECK(player.getSortKey() == Player({-2, -2}, nullptr).getSortKey());
}

MOTOR_TEST_CASE(player_bump_faces_the_wall_and_rebounds_without_leaving_the_cell) {
    TestablePlayer player({2, 2}, nullptr);
    animations(player);
    player.setOffset({4, 7});
    const int key = player.getSortKey();
    player.bump({1, 0});
    MOTOR_CHECK(player.gridPosition().x == 2 && player.gridPosition().y == 2);
    MOTOR_CHECK(player.currentAnimation() == "walk_right");
    MOTOR_CHECK(player.sprite() == 9);
    MOTOR_CHECK(std::abs(player.offset().x - 4.0f) < 0.001f); // Arranca en su sitio.
    player.update(0.04f); // Mitad del rebote: desplazado hacia el muro.
    MOTOR_CHECK(player.offset().x > 4.0f);
    MOTOR_CHECK(player.offset().y > 7.0f);
    MOTOR_CHECK(player.getSortKey() == key);
    MOTOR_CHECK(player.sprite() == 9); // El rebote no camina en el sitio.
    player.update(0.04f);
    MOTOR_CHECK(std::abs(player.offset().x - 4.0f) < 0.001f);
    MOTOR_CHECK(std::abs(player.offset().y - 7.0f) < 0.001f);
    MOTOR_CHECK(player.gridPosition().x == 2);
}

MOTOR_TEST_CASE(player_bump_is_ignored_mid_step_and_a_step_after_a_bump_keeps_its_duration) {
    TestablePlayer player({0, 0}, nullptr);
    animations(player);
    player.moveTo({0, -1});
    player.update(0.05f);
    const float mid = player.offset().y;
    player.bump({1, 0});
    MOTOR_CHECK(player.currentAnimation() == "walk_up"); // No interrumpe el paso.
    player.update(0.0f);
    MOTOR_CHECK(std::abs(player.offset().y - mid) < 0.001f);
    player.update(0.11f);
    player.bump({0, 1});
    player.update(0.08f);
    player.moveTo({1, -1});
    player.update(0.08f); // Un paso dura 160 ms (AV14-09) aunque venga de un rebote de 80.
    MOTOR_CHECK(std::abs(player.offset().x) > 0.001f);
    player.update(0.08f);
    MOTOR_CHECK(std::abs(player.offset().x) < 0.001f);
    MOTOR_CHECK(player.currentAnimation() == "walk_right");
}


// AV14-09 (Oriol, 14/10: «no puede ser que el movimiento no se vea fluido»).
// Con la tecla mantenida, a 30, 60 y 144 fps, el dibujo no se queda quieto ni
// un fotograma entre dos casillas.
MOTOR_TEST_CASE(player_held_walk_never_stands_still_between_cells) {
    for (const int fps : {30, 60, 144}) {
        TestablePlayer player({0, 0}, nullptr);
        animations(player);
        Engine::Input::InputSystem input;
        RawInputState keys;
        keys.setDown(InputKey::D);
        const float dt = 1.0f / static_cast<float>(fps);
        auto dibujo = [&] {
            return IsoMath::gridToScreen(player.gridPosition(), 64.0f, 32.0f) + player.offset();
        };
        int quietos = 0, pasos = 0;
        Vector2 antes = dibujo();
        for (int frame = 0; frame < fps * 2; ++frame) {  // 2 s andando
            const GridCoord paso = player.movementCommand(input.update(keys), frame * static_cast<double>(dt), true);
            if (paso.x != 0 || paso.y != 0) {
                player.moveTo({player.gridPosition().x + paso.x, player.gridPosition().y + paso.y});
                ++pasos;
            }
            player.update(dt);
            const Vector2 ahora = dibujo();
            if (frame > 0 && std::abs(ahora.x - antes.x) < 0.01f && std::abs(ahora.y - antes.y) < 0.01f) ++quietos;
            antes = ahora;
        }
        MOTOR_CHECK(pasos == 15);
        MOTOR_CHECK(quietos == 0);
    }
}

MOTOR_TEST_CASE(player_rapid_direction_changes_keep_only_the_latest_buffered_step) {
    ControlsFixture f;
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(f.poll(0).x == 1);
    f.keys.setDown(InputKey::D, false);
    f.keys.setDown(InputKey::W);
    MOTOR_CHECK(stopped(f.poll(0.03)));
    f.keys.setDown(InputKey::W, false);
    f.keys.setDown(InputKey::A);
    MOTOR_CHECK(stopped(f.poll(0.06)));
    f.keys.setDown(InputKey::A, false);
    f.keys.setDown(InputKey::S);
    MOTOR_CHECK(stopped(f.poll(0.09)));
    MOTOR_CHECK(stopped(f.poll(0.139)));
    MOTOR_CHECK(f.poll(0.14).y == 1);
    MOTOR_CHECK(stopped(f.poll(0.14)));
    MOTOR_CHECK(stopped(f.poll(0.27)));
    MOTOR_CHECK(f.poll(0.28).y == 1);  // AV14-09: S sigue abajo, sigue andando sin esperar 0,24 s
}

MOTOR_TEST_CASE(player_short_repeated_taps_cannot_bypass_the_step_interval) {
    ControlsFixture f;
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(f.poll(0).x == 1);
    f.keys.setDown(InputKey::D, false);
    MOTOR_CHECK(stopped(f.poll(0.01)));
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(stopped(f.poll(0.02)));
    f.keys.setDown(InputKey::D, false);
    MOTOR_CHECK(stopped(f.poll(0.03)));
    MOTOR_CHECK(stopped(f.poll(0.139)));
    MOTOR_CHECK(f.poll(0.14).x == 1);
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(stopped(f.poll(0.15)));
    f.keys.setDown(InputKey::D, false);
    MOTOR_CHECK(stopped(f.poll(0.16)));
    MOTOR_CHECK(stopped(f.poll(0.279)));
    MOTOR_CHECK(f.poll(0.281).x == 1);
    MOTOR_CHECK(stopped(f.poll(0.9)));
}

MOTOR_TEST_CASE(player_buffered_turn_survives_release_while_another_direction_is_held) {
    ControlsFixture f;
    f.keys.setDown(InputKey::W);
    MOTOR_CHECK(f.poll(0).y == -1);
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(stopped(f.poll(0.03)));
    f.keys.setDown(InputKey::D, false);
    MOTOR_CHECK(stopped(f.poll(0.05)));
    MOTOR_CHECK(f.poll(0.14).x == 1);
    MOTOR_CHECK(stopped(f.poll(0.279)));
    MOTOR_CHECK(f.poll(0.28).y == -1);
}

MOTOR_TEST_CASE(player_opposed_keys_cancel_the_pending_step_on_their_axis) {
    ControlsFixture f;
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(f.poll(0).x == 1);
    f.keys.setDown(InputKey::D, false);
    f.keys.setDown(InputKey::W);
    MOTOR_CHECK(stopped(f.poll(0.03)));
    f.keys.setDown(InputKey::S);
    MOTOR_CHECK(stopped(f.poll(0.04)));
    MOTOR_CHECK(stopped(f.poll(0.2)));
    f.keys.setDown(InputKey::W, false);
    f.keys.setDown(InputKey::S, false);
    MOTOR_CHECK(stopped(f.poll(0.3)));
}

MOTOR_TEST_CASE(player_modal_or_focus_loss_discards_a_buffered_tap) {
    ControlsFixture f;
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(f.poll(0).x == 1);
    f.keys.setDown(InputKey::D, false);
    f.keys.setDown(InputKey::W);
    MOTOR_CHECK(stopped(f.poll(0.03)));
    MOTOR_CHECK(stopped(f.poll(0.04, false)));
    f.keys.setDown(InputKey::W, false);
    MOTOR_CHECK(stopped(f.poll(0.05)));
    MOTOR_CHECK(stopped(f.poll(0.2)));
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(f.poll(0.21).x == 1);
}

MOTOR_TEST_CASE(player_teleport_discards_a_buffered_tap) {
    ControlsFixture f;
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(f.poll(0).x == 1);
    f.keys.setDown(InputKey::D, false);
    f.keys.setDown(InputKey::W);
    MOTOR_CHECK(stopped(f.poll(0.03)));
    f.player.setGridPosition({20, 20});
    f.keys.setDown(InputKey::W, false);
    MOTOR_CHECK(stopped(f.poll(0.05)));
    MOTOR_CHECK(stopped(f.poll(0.2)));
    MOTOR_CHECK(f.player.gridPosition().x == 20);
}

MOTOR_TEST_CASE(player_buffered_tap_after_a_slow_frame_never_becomes_a_burst) {
    ControlsFixture f;
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(f.poll(0).x == 1);
    f.keys.setDown(InputKey::D, false);
    f.keys.setDown(InputKey::W);
    MOTOR_CHECK(stopped(f.poll(0.01)));
    f.keys.setDown(InputKey::W, false);
    MOTOR_CHECK(stopped(f.poll(0.02)));
    MOTOR_CHECK(f.poll(3).y == -1);
    MOTOR_CHECK(stopped(f.poll(3)));
    MOTOR_CHECK(stopped(f.poll(3.01)));
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(stopped(f.poll(3.02)));
    MOTOR_CHECK(f.poll(3.15).x == 1);
}

MOTOR_TEST_CASE(player_releasing_the_fallback_key_does_not_queue_a_ghost_step) {
    ControlsFixture f;
    f.keys.setDown(InputKey::W);
    MOTOR_CHECK(f.poll(0).y == -1);
    f.keys.setDown(InputKey::D);
    MOTOR_CHECK(stopped(f.poll(0.03)));
    f.keys.setDown(InputKey::D, false);
    MOTOR_CHECK(stopped(f.poll(0.05)));
    MOTOR_CHECK(f.poll(0.14).x == 1);
    f.keys.setDown(InputKey::W, false);
    MOTOR_CHECK(stopped(f.poll(0.16)));
    MOTOR_CHECK(stopped(f.poll(0.3)));
}

int main() { return motor_test::runAllTests(); }
