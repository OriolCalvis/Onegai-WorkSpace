#include "Render/AnimatedEntity.h"

#include <cstddef>

AnimatedEntity::AnimatedEntity(GridCoord gridPosition, TextureAtlas* atlas, int tileWidth,
                               int tileHeight, float frameTime)
    // spriteID inicial 0: ninguna animacion esta activa todavia (play() no
    // se ha llamado), asi que no hay un frame "correcto" que elegir aun.
    // El primer play() lo sustituye antes del primer render() util.
    : Entity(gridPosition, 0, atlas, tileWidth, tileHeight), m_frameTime(frameTime) {}

void AnimatedEntity::update(float deltaTime) {
    auto it = m_animations.find(m_currentAnimation);
    if (it == m_animations.end() || it->second.empty()) {
        return;
    }
    const std::vector<int>& frames = it->second;

    m_elapsedTime += deltaTime;
    // Bucle en vez de "if": un deltaTime grande (frame lento, o varios
    // pasos de test de golpe) puede tener que saltar mas de un frame de
    // animacion para no quedarse atras.
    while (m_elapsedTime >= m_frameTime) {
        m_elapsedTime -= m_frameTime;
        m_currentFrame = (m_currentFrame + 1) % static_cast<int>(frames.size());
    }
    m_spriteID = frames[static_cast<std::size_t>(m_currentFrame)];
}

void AnimatedEntity::addAnimation(const std::string& name, std::vector<int> frames) {
    m_animations[name] = std::move(frames);
}

void AnimatedEntity::play(const std::string& name) {
    if (name == m_currentAnimation) {
        return;
    }
    auto it = m_animations.find(name);
    if (it == m_animations.end()) {
        return;
    }
    m_currentAnimation = name;
    m_currentFrame = 0;
    m_elapsedTime = 0.0f;
    if (!it->second.empty()) {
        m_spriteID = it->second[0];
    }
}
