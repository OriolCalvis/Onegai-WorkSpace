#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Render/Entity.h"

class TextureAtlas;

// Entity con animacion por frames (motor_grafico_clases.puml): una
// coleccion nombrada de secuencias de spriteID ("walk_up", "idle"...) de
// las que solo una esta activa a la vez. update() avanza el frame segun
// el tiempo transcurrido y reescribe Entity::m_spriteID (protected, ver
// Entity.h) para que Entity::render() ya dibuje el frame correcto sin
// tener que duplicar logica de render aqui.
//
// Base de Player/Enemy (motor_grafico_clases.puml): ninguna de las dos
// anade nada al propio ciclo de animacion, solo a que se mueva o no la
// entidad y bajo que nombre de animacion.
class AnimatedEntity : public Entity {
public:
    // frameTime (segundos por frame) no esta en el diagrama de clases:
    // sin el, update() no tendria forma de saber cuando avanzar
    // m_currentFrame (mismo criterio que Entity::tileWidth/tileHeight, ver
    // su comentario). 0.15s (~6-7 fps) es un valor razonable para pixel
    // art a baja resolucion de frames.
    AnimatedEntity(GridCoord gridPosition, TextureAtlas* atlas, int tileWidth = 64,
                   int tileHeight = 32, float frameTime = 0.15f);

    // Avanza m_elapsedTime y, cuando supera m_frameTime, m_currentFrame
    // (con wraparound: toda animacion aqui es un loop). Si no hay ninguna
    // animacion activa (play() nunca llamado, o llamado con un nombre no
    // registrado) no hace nada: Entity::m_spriteID conserva el ultimo
    // valor valido en vez de "parpadear" a un frame vacio.
    void update(float deltaTime) override;

    // Registra "frames" (spriteID de TextureAtlas, en orden) bajo "name".
    // Sobrescribe si "name" ya existia (permite recargar/redefinir).
    void addAnimation(const std::string& name, std::vector<int> frames);

    // Activa la animacion "name". Si ya es la animacion activa, no hace
    // nada (evita reiniciar el ciclo de frames cada vez que se llama
    // play() con la misma tecla mantenida, ver Player::handleInput). Si
    // "name" no esta registrada, se ignora en silencio -- igual que
    // TextureAtlas::getUV() con un id sin definir (ver su comentario):
    // es una llamada de hot path, no un error irrecuperable.
    void play(const std::string& name);

    // No estan en el diagrama de clases: necesarios para poder verificar
    // el ciclo de animacion (que frame esta activo, cuantos frames tiene
    // la animacion actual) sin tener que inspeccionar pixeles de un
    // framebuffer, mismo criterio que Entity::gridPosition()/offset().
    const std::string& currentAnimation() const { return m_currentAnimation; }
    int currentFrame() const { return m_currentFrame; }

private:
    std::unordered_map<std::string, std::vector<int>> m_animations;
    std::string m_currentAnimation;
    float m_frameTime;
    float m_elapsedTime = 0.0f;
    int m_currentFrame = 0;
};
