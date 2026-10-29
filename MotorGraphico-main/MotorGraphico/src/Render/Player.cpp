// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-12  (fase G, 5 j) - Inventari, equip i fitxa utilitzables
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Render/Player.h"

#include <algorithm>
#include <cmath>

#include "Engine/InputState.h"
#include "Core/Math/IsoMath.h"
#include "Game/PlayerControls.h"

namespace {
// AV14-09 (Oriol, 14/10: «no puede ser que el movimiento no se vea fluido»): el
// paso visual dura un poco MAS que la cadencia de la tecla (0,14 s,
// PlayerControls). Andando seguido, el paso siguiente llega antes de que acabe
// el anterior y lo continua desde donde va el dibujo: nunca hay un fotograma
// quieto entre dos casillas (antes, el dibujo se paraba y enseñaba el quieto).
constexpr float kStepDuration = 0.16f;
// Parado este rato tras un paso, se enseña el quieto (no en cuanto acaba el paso:
// entre dos pasos seguidos parpadeaba).
constexpr float kAntesDelQuieto = 0.06f;
constexpr float kBumpDuration = 0.08f;
constexpr float kBumpReach = 0.12f; // fraccion de casilla hacia el muro
// ESTETICA-01: parado este rato, el jugador pasa del quieto de andar al reposo
// de su rumbo ("walk_down" -> "idle_down"). Entre dos pasos seguidos no da
// tiempo: el ciclo de andar sigue sin reiniciarse.
constexpr float kAntesDelReposo = 0.2f;
}  // namespace

Player::Player(GridCoord gridPosition, TextureAtlas* atlas, int tileWidth, int tileHeight)
    : AnimatedEntity(gridPosition, atlas, tileWidth, tileHeight, 0.07f) {
    m_data.motion.tileWidth = static_cast<float>(tileWidth);
    m_data.motion.tileHeight = static_cast<float>(tileHeight);
}

void Player::update(float deltaTime) {
    if (!std::isfinite(deltaTime) || deltaTime <= 0.0f) return;
    auto& motion = m_data.motion;
    if (!motion.active) {
        if (currentAnimation().rfind("walk_", 0) != 0) {
            AnimatedEntity::update(std::min(deltaTime, 0.25f));
        } else {
            motion.parado += deltaTime;
            if (motion.parado >= kAntesDelQuieto) showRestFrame();
            // Sin ciclos de reposo (una hoja de 12 frames) play() no hace nada
            // y se queda en el quieto de andar, como antes.
            if (motion.parado >= kAntesDelReposo) play("idle_" + currentAnimation().substr(5));
        }
        return;
    }
    motion.parado = 0.0f;
    if (!motion.bump) AnimatedEntity::update(std::min(deltaTime, motion.duration - motion.elapsed));
    motion.elapsed = std::min(motion.elapsed + deltaTime, motion.duration);
    const float progress = motion.elapsed / motion.duration;
    // Paso: llega desde displacement hasta 0. Rebote: sale y vuelve (seno).
    const float factor = motion.bump ? std::sin(progress * 3.14159265f) : 1.0f - progress;
    setOffset(motion.baseOffset + motion.displacement * factor);
    if (motion.elapsed >= motion.duration) {
        motion.active = false;
        motion.bump = false;
        motion.duration = kStepDuration;
        setOffset(motion.baseOffset);
    }
}

void Player::bump(const GridCoord& step) {
    auto& motion = m_data.motion;
    if (motion.active || (step.x == 0 && step.y == 0)) return;
    const auto here = gridPosition();
    const GridCoord wall{here.x + step.x, here.y + step.y};
    motion.baseOffset = offset();
    motion.displacement = (IsoMath::gridToScreen(wall, motion.tileWidth, motion.tileHeight) -
                           IsoMath::gridToScreen(here, motion.tileWidth, motion.tileHeight)) *
                          kBumpReach;
    motion.startSortKey = Entity::getSortKey();
    motion.elapsed = 0.0f;
    motion.duration = kBumpDuration;
    motion.bump = true;
    motion.active = true;
    play(step.y < 0 ? "walk_up" : step.y > 0 ? "walk_down" : step.x < 0 ? "walk_left" : "walk_right");
    showRestFrame();
}

int Player::getSortKey() const {
    // PROLOGO-02 — Con el dibujo corrido (el «desfase» de una cinematica: subido a
    // la barca desde la orilla), la profundidad es la del dibujo: si no, la barca,
    // una celda mas alla, lo tapa. Sin desfase la base es 0. El multiplicador es el
    // de Entity::getSortKey (la profundidad va en los bits altos).
    const Vector2 base = m_data.motion.active ? m_data.motion.baseOffset : offset();
    return claveDelPaso() + static_cast<int>(std::lround(base.y)) * (1 << 16);
}

int Player::claveDelPaso() const {
    const auto& motion = m_data.motion;
    const int destination = Entity::getSortKey();
    if (!motion.active) return destination;
    const double progress = motion.elapsed / motion.duration;
    return static_cast<int>(std::lround(motion.startSortKey +
        (static_cast<double>(destination) - motion.startSortKey) * progress));
}

GridCoord Player::movementCommand(const Engine::Input::InputSnapshot& input,
                                  double now, bool enabled) {
    return PlayerControls::poll(m_data.controls, input, now, enabled);
}

void Player::setGridPosition(const GridCoord& destination) {
    if (m_data.motion.active) setOffset(m_data.motion.baseOffset);
    m_data.motion.active = false;
    m_data.motion.bump = false;
    m_data.motion.duration = kStepDuration;
    m_data.controls = {};
    m_data.controls.waitingForRelease = true;
    Entity::setGridPosition(destination);
    play("idle");
}

void Player::moveTo(const GridCoord& destination, float duracion) {
    const auto previous = gridPosition();
    const int dx = destination.x - previous.x;
    const int dy = destination.y - previous.y;
    if (dx == 0 && dy == 0) return; // Un muro no desplaza ni reinicia el sprite.
    if (std::abs(dx) + std::abs(dy) != 1) {
        setGridPosition(destination);
        return;
    }
    auto& motion = m_data.motion;
    if (!motion.active) motion.baseOffset = offset();
    motion.displacement = offset() - motion.baseOffset +
        IsoMath::gridToScreen(previous, motion.tileWidth, motion.tileHeight) -
        IsoMath::gridToScreen(destination, motion.tileWidth, motion.tileHeight);
    motion.startSortKey = claveDelPaso();
    motion.elapsed = 0.0f;
    motion.duration = duracion > 0.0f ? duracion : kStepDuration;
    motion.bump = false;
    motion.active = true;
    // AV19-02: si el terreno sube o baja entre las dos casillas, el dibujo arranca en la altura de la
    // anterior y llega a la de la nueva (como ya hace con x e y).
    motion.displacement.y += elevacionEn(previous) - elevacionEn(destination);
    Entity::setGridPosition(destination);
    setOffset(motion.baseOffset + motion.displacement);
    play(dy < 0 ? "walk_up" : dy > 0 ? "walk_down" : dx < 0 ? "walk_left" : "walk_right");
}

void Player::handleInput(const InputState& input) {
    GridCoord pos = gridPosition();

    if (input.moveUp) pos.y -= 1;
    else if (input.moveDown) pos.y += 1;
    else if (input.moveLeft) pos.x -= 1;
    else if (input.moveRight) pos.x += 1;

    moveTo(pos);
}

void Player::takeDamage(int amount) { m_data.health = std::max(0, m_data.health - amount); }

// G-12: el equipo puede cambiar healthCap despues de vincular la ficha.
// Curar y asignar vida usan el mismo limite que consultan el HUD y la tienda;
// maxHealth() conserva el maximo legacy cuando no hay CharacterSheet.
void Player::set_health(int v) { m_data.health = std::clamp(v, 0, maxHealth()); }

void Player::heal(int amount) { set_health(m_data.health + amount); }

void Player::addItem(int itemID) { m_data.inventory.push_back(itemID); }
