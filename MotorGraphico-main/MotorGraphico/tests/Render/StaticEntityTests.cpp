// Copyright 2026 Motor Grafico Isometrico Pixel Art
// Licensed under the MIT License

#include "Render/StaticEntity.h"
#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector2.h"

#include "TestHarness.h"

// Forward declaration para mock
class TextureAtlas;
class SpriteBatch;

namespace {

TextureAtlas* createMockAtlas() {
    return static_cast<TextureAtlas*>(nullptr);
}

} // namespace

// --- Tests de construccion ---

MOTOR_TEST_CASE(static_entity_constructor_default_values) {
    TextureAtlas* atlas = createMockAtlas();
    StaticEntity entity(GridCoord{0, 0}, 0, atlas);
    MOTOR_CHECK(entity.gridPosition().x == 0);
    MOTOR_CHECK(entity.gridPosition().y == 0);
    MOTOR_CHECK(entity.offset().x == 0.0f);
    MOTOR_CHECK(entity.offset().y == 0.0f);
    MOTOR_CHECK(entity.visualScale() == 1.0f);
}

MOTOR_TEST_CASE(static_entity_constructor_with_custom_position) {
    TextureAtlas* atlas = createMockAtlas();
    StaticEntity entity(GridCoord{5, 10}, 0, atlas);
    MOTOR_CHECK(entity.gridPosition().x == 5);
    MOTOR_CHECK(entity.gridPosition().y == 10);
}

MOTOR_TEST_CASE(static_entity_constructor_with_custom_tile_dimensions) {
    TextureAtlas* atlas = createMockAtlas();
    StaticEntity entity(GridCoord{0, 0}, 0, atlas, 128, 64);
    MOTOR_CHECK(true); // Constructor acepta parámetros
}

// --- Tests de herencia (Entity) ---

MOTOR_TEST_CASE(static_entity_inherits_from_entity) {
    TextureAtlas* atlas = createMockAtlas();
    StaticEntity entity(GridCoord{0, 0}, 0, atlas);
    
    // StaticEntity hereda de Entity
    Entity* base = &entity;
    MOTOR_CHECK(base != nullptr);
}

MOTOR_TEST_CASE(static_entity_implements_irenderable) {
    TextureAtlas* atlas = createMockAtlas();
    StaticEntity entity(GridCoord{0, 0}, 0, atlas);
    
    IRenderable* renderable = &entity;
    MOTOR_CHECK(renderable != nullptr);
}

MOTOR_TEST_CASE(static_entity_implements_iupdatable) {
    TextureAtlas* atlas = createMockAtlas();
    StaticEntity entity(GridCoord{0, 0}, 0, atlas);
    
    IUpdatable* updatable = &entity;
    MOTOR_CHECK(updatable != nullptr);
}

// --- Tests de métodos heredados de Entity ---

MOTOR_TEST_CASE(static_entity_set_grid_position) {
    TextureAtlas* atlas = createMockAtlas();
    StaticEntity entity(GridCoord{0, 0}, 0, atlas);
    
    entity.setGridPosition(GridCoord{7, 14});
    MOTOR_CHECK(entity.gridPosition().x == 7);
    MOTOR_CHECK(entity.gridPosition().y == 14);
}

MOTOR_TEST_CASE(static_entity_set_offset) {
    TextureAtlas* atlas = createMockAtlas();
    StaticEntity entity(GridCoord{0, 0}, 0, atlas);
    
    entity.setOffset(Vector2{1.5f, 2.5f});
    MOTOR_CHECK(std::abs(entity.offset().x - 1.5f) < 0.001f);
    MOTOR_CHECK(std::abs(entity.offset().y - 2.5f) < 0.001f);
}

MOTOR_TEST_CASE(static_entity_set_visual_scale) {
    TextureAtlas* atlas = createMockAtlas();
    StaticEntity entity(GridCoord{0, 0}, 0, atlas);
    
    entity.setVisualScale(2.0f);
    MOTOR_CHECK(std::abs(entity.visualScale() - 2.0f) < 0.001f);
    
    // Verificar clamp
    entity.setVisualScale(0.1f);
    MOTOR_CHECK(std::abs(entity.visualScale() - 0.25f) < 0.001f);
    
    entity.setVisualScale(10.0f);
    MOTOR_CHECK(std::abs(entity.visualScale() - 4.0f) < 0.001f);
}

MOTOR_TEST_CASE(static_entity_set_tint) {
    TextureAtlas* atlas = createMockAtlas();
    StaticEntity entity(GridCoord{0, 0}, 0, atlas);
    
    Vector4 red{1.0f, 0.0f, 0.0f, 1.0f};
    entity.setTint(red);
    const Vector4& tint = entity.tint();
    MOTOR_CHECK(tint.x == 1.0f);
    MOTOR_CHECK(tint.y == 0.0f);
    MOTOR_CHECK(tint.z == 0.0f);
    MOTOR_CHECK(tint.w == 1.0f);
}

MOTOR_TEST_CASE(static_entity_set_atlas) {
    TextureAtlas* atlas1 = createMockAtlas();
    TextureAtlas* atlas2 = createMockAtlas();
    StaticEntity entity(GridCoord{0, 0}, 0, atlas1);
    
    entity.setAtlas(atlas2);
    MOTOR_CHECK(true); // No crash
}

MOTOR_TEST_CASE(static_entity_get_sort_key) {
    TextureAtlas* atlas = createMockAtlas();
    StaticEntity entity1(GridCoord{0, 0}, 0, atlas, 64, 32);
    StaticEntity entity2(GridCoord{1, 0}, 0, atlas, 64, 32);
    
    // entity1 en (0,0) debería tener sort key = 0
    MOTOR_CHECK(entity1.getSortKey() == 0);
    
    // entity2 debería tener sort key diferente
    MOTOR_CHECK(entity1.getSortKey() != entity2.getSortKey());
}

// --- Tests de polimorfismo ---

MOTOR_TEST_CASE(static_entity_update_is_noop) {
    TextureAtlas* atlas = createMockAtlas();
    StaticEntity entity(GridCoord{0, 0}, 0, atlas);
    
    // StaticEntity::update() es un no-op deliberado
    entity.update(0.1f);
    MOTOR_CHECK(true); // No crash
}

MOTOR_TEST_CASE(static_entity_using_entity_constructor) {
    TextureAtlas* atlas = createMockAtlas();
    // StaticEntity usa Entity::Entity a través de using
    StaticEntity entity(GridCoord{3, 4}, 42, atlas);
    
    MOTOR_CHECK(entity.gridPosition().x == 3);
    MOTOR_CHECK(entity.gridPosition().y == 4);
}

// INT-03: un prop puede ir sin la sombra blob (los dibujos del kit la llevan pintada).
// renderShadow() con la sombra apagada ni toca el batch: aqui no hay GL, asi que
// se comprueba el interruptor (por defecto, encendido, como siempre).
MOTOR_TEST_CASE(static_entity_sombra_blob_se_puede_apagar) {
    TextureAtlas* atlas = createMockAtlas();
    StaticEntity entity(GridCoord{0, 0}, 0, atlas);
    MOTOR_CHECK(entity.sombraBlob());
    entity.setSombraBlob(false);
    MOTOR_CHECK(!entity.sombraBlob());
    entity.setSombraBlob(true);
    MOTOR_CHECK(entity.sombraBlob());
}

int main() { return motor_test::runAllTests(); }
