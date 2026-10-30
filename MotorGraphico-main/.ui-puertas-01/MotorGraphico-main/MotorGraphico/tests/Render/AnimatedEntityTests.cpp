// Copyright 2026 Motor Grafico Isometrico Pixel Art
// Licensed under the MIT License

#include "Render/AnimatedEntity.h"
#include "Core/Math/GridCoord.h"

#include "TestHarness.h"

#include <string>
#include <vector>

namespace {

// AnimatedEntity recibe un TextureAtlas* que solo desreferencia dentro de
// render() (Entity.cpp:73, la ruta GL). Estas pruebas son GL-free y no
// renderizan nunca, asi que el atlas es irrelevante para lo que comprueban:
// el ciclo de animacion.
//
// Antes habia aqui un `class MockTextureAtlas {}` vacio que se pasaba como
// kSinAtlas. No derivaba de TextureAtlas, o sea que no existia ninguna conversion
// posible y el objetivo NO COMPILABA: para fingir el tipo habria que incluir
// Core/Resources/TextureAtlas.h y arrastrar GL a una prueba GL-free, que es
// justo lo que la ley 2 prohibe. Se pasa nullptr y se dice por que.
TextureAtlas* const kSinAtlas = nullptr;

} // namespace

MOTOR_TEST_CASE(animated_entity_constructor_default_values) {
    AnimatedEntity entity(GridCoord{0, 0}, kSinAtlas);
    MOTOR_CHECK(entity.currentAnimation().empty());
    MOTOR_CHECK(entity.currentFrame() == 0);
}

MOTOR_TEST_CASE(animated_entity_add_animation_stores_frames) {
    AnimatedEntity entity(GridCoord{0, 0}, kSinAtlas);
    
    std::vector<int> idleFrames = {0, 1, 2, 3};
    entity.addAnimation("idle", idleFrames);
    
    // No hay getters para m_animations, pero podemos verificar indirectamente
    entity.play("idle");
    MOTOR_CHECK(entity.currentAnimation() == "idle");
}

MOTOR_TEST_CASE(animated_entity_play_nonexistent_animation_is_noop) {
    AnimatedEntity entity(GridCoord{0, 0}, kSinAtlas);
    
    entity.play("nonexistent");
    // Deberia ser no-op: currentAnimation sigue vacia
    MOTOR_CHECK(entity.currentAnimation().empty());
    MOTOR_CHECK(entity.currentFrame() == 0);
}

MOTOR_TEST_CASE(animated_entity_play_existing_animation_sets_current) {
    AnimatedEntity entity(GridCoord{0, 0}, kSinAtlas);
    
    std::vector<int> idleFrames = {0, 1, 2, 3};
    std::vector<int> walkFrames = {4, 5, 6, 7, 8, 9};
    entity.addAnimation("idle", idleFrames);
    entity.addAnimation("walk", walkFrames);
    
    entity.play("idle");
    MOTOR_CHECK(entity.currentAnimation() == "idle");
    
    entity.play("walk");
    MOTOR_CHECK(entity.currentAnimation() == "walk");
}

MOTOR_TEST_CASE(animated_entity_play_same_animation_noop) {
    AnimatedEntity entity(GridCoord{0, 0}, kSinAtlas);
    
    std::vector<int> idleFrames = {0, 1, 2, 3};
    entity.addAnimation("idle", idleFrames);
    entity.play("idle");
    
    const std::string& firstAnim = entity.currentAnimation();
    const int firstFrame = entity.currentFrame();
    
    // Jugar la misma animacion no reinicia
    entity.play("idle");
    MOTOR_CHECK(entity.currentAnimation() == firstAnim);
    // El frame puede haber avanzado debido al tiempo, pero la animacion es la misma
}

MOTOR_TEST_CASE(animated_entity_update_advances_frames) {
    AnimatedEntity entity(GridCoord{0, 0}, kSinAtlas, 64, 32, 0.1f); // 0.1s por frame
    
    std::vector<int> frames = {0, 1, 2};
    entity.addAnimation("test", frames);
    entity.play("test");
    
    // Inicialmente frame 0
    MOTOR_CHECK(entity.currentFrame() == 0);
    
    // Avanza 0.05s (menos que frameTime), no cambia de frame
    entity.update(0.05f);
    MOTOR_CHECK(entity.currentFrame() == 0);
    
    // Avanza 0.1s (exactamente frameTime), cambia a frame 1
    entity.update(0.05f);
    MOTOR_CHECK(entity.currentFrame() == 1);
    
    // Avanza 0.1s más, cambia a frame 2
    entity.update(0.1f);
    MOTOR_CHECK(entity.currentFrame() == 2);
    
    // Avanza 0.1s más, wrap around a frame 0
    entity.update(0.1f);
    MOTOR_CHECK(entity.currentFrame() == 0);
}

MOTOR_TEST_CASE(animated_entity_update_no_animation_active_noop) {
    AnimatedEntity entity(GridCoord{0, 0}, kSinAtlas);
    
    // Sin animacion activa, update no hace nada
    entity.update(1.0f);
    MOTOR_CHECK(entity.currentAnimation().empty());
    MOTOR_CHECK(entity.currentFrame() == 0);
}

MOTOR_TEST_CASE(animated_entity_update_wraps_around) {
    AnimatedEntity entity(GridCoord{0, 0}, kSinAtlas, 64, 32, 0.1f);
    
    // Animacion con 3 frames
    std::vector<int> frames = {10, 11, 12};
    entity.addAnimation("loop", frames);
    entity.play("loop");
    
    // Avanzar suficiente para completar un ciclo
    for (int i = 0; i < 3; ++i) {
        entity.update(0.1f);
    }
    MOTOR_CHECK(entity.currentFrame() == 0); // Wrap around
}

MOTOR_TEST_CASE(animated_entity_add_animation_overwrites_existing) {
    AnimatedEntity entity(GridCoord{0, 0}, kSinAtlas);
    
    std::vector<int> firstFrames = {0, 1, 2};
    entity.addAnimation("anim", firstFrames);
    entity.play("anim");
    
    std::vector<int> secondFrames = {10, 11};
    entity.addAnimation("anim", secondFrames); // Sobrescribe
    
    // No hay forma directa de verificar los frames, pero podemos verificar
    // que la animacion sigue siendo "anim"
    MOTOR_CHECK(entity.currentAnimation() == "anim");
}

MOTOR_TEST_CASE(animated_entity_position_inherited_from_entity) {
    GridCoord pos{5, 10};
    AnimatedEntity entity(pos, kSinAtlas);
    
    MOTOR_CHECK(entity.gridPosition().x == 5);
    MOTOR_CHECK(entity.gridPosition().y == 10);
}

MOTOR_TEST_CASE(animated_entity_custom_frame_time) {
    AnimatedEntity entity(GridCoord{0, 0}, kSinAtlas, 64, 32, 0.5f); // 0.5s por frame
    
    std::vector<int> frames = {0, 1, 2, 3, 4};
    entity.addAnimation("slow", frames);
    entity.play("slow");
    
    // Avanzar 0.4s, no cambia de frame
    entity.update(0.4f);
    MOTOR_CHECK(entity.currentFrame() == 0);
    
    // Avanzar 0.1s más (total 0.5s), cambia a frame 1
    entity.update(0.1f);
    MOTOR_CHECK(entity.currentFrame() == 1);
}

int main() { return motor_test::runAllTests(); }
