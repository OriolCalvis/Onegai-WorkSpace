#pragma once

#include <glm/glm.hpp>

#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector2.h"
#include "Render/IUpdatable.h"

// Camara ortografica 2D con paneo suavizado (lerp exponencial hacia
// m_targetPosition) y zoom. Ver motor_grafico_clases.puml: no incluye
// viewport en el diagrama, pero getViewProjectionMatrix()/screenToWorld()
// necesitan conocer el tamano de la ventana para ser correctas, asi que
// se anade setViewportSize() (mismo criterio ya usado para Window, que
// tampoco aparece en el diagrama).
//
// : public IUpdatable (fractura #3 del analisis de coherencia, ver
// ARCHITECTURE.md): update(deltaTime) ya existia con la misma firma que
// IUpdatable::update; ahora se declara override y la camara es formalmente
// actualizable. No cambia comportamiento: quien llama camera.update(dt)
// sigue siendo Application (no se anade a ninguna cola nueva).
class Camera : public IUpdatable {
public:
    Camera(int viewportWidth, int viewportHeight);

    // Avanza m_position hacia m_targetPosition (lerp exponencial, suaviza
    // el paneo en vez de saltar de golpe). Sin efecto si ya coinciden.
    void update(float deltaTime) override;

    // Desplaza el punto de destino del paneo; update() lo alcanza con
    // suavizado en los frames siguientes.
    void move(const Vector2& delta);

    void setZoom(float zoom);
    void setViewportSize(int width, int height);

    // --- Transiciones de camara y zoom (Fase 4, pulido visual) ---
    // A diferencia del paneo continuo de move()/update() (lerp
    // exponencial hacia m_targetPosition, pensado para seguir a algo que
    // se mueve, ej. el jugador), transitionTo() es una animacion de UNA
    // VEZ y de duracion fija entre la posicion/zoom ACTUALES y un destino
    // explicito (ej. un cutscene: "lleva la camara aqui en 1.5s"). Mientras
    // esta activa, sustituye el lerp exponencial de update() por
    // interpolacion con curva de easing; al terminar dentro de update(),
    // m_targetPosition queda en el destino, asi que move() sigue
    // funcionando despues sin salto (retoma el paneo normal desde ahi).
    enum class Easing { Linear, EaseInOutQuad, EaseOutCubic };

    // Arranca una transicion desde la posicion/zoom actuales hacia
    // targetPosition/targetZoom en "duration" segundos (duration <= 0 es
    // un corte instantaneo: la siguiente update() aplica el destino
    // completo de una vez, sin esperar a un segundo frame). Cancela
    // cualquier transicion en curso reiniciando desde la posicion/zoom
    // ACTUALES (los ya interpolados, no el destino de la transicion
    // cancelada): encadenar transitionTo() no produce saltos bruscos.
    void transitionTo(const Vector2& targetPosition, float targetZoom, float duration,
                      Easing easing = Easing::EaseInOutQuad);
    bool isTransitioning() const { return m_transitionActive; }

    const Vector2& position() const { return m_position; }
    float zoom() const { return m_zoom; }
    int viewportWidth() const { return m_viewportWidth; }
    int viewportHeight() const { return m_viewportHeight; }

    // Matriz combinada vista*proyeccion para el shader (mundo -> NDC).
    // Y invertida a proposito: en este motor "abajo en pantalla" = +y en
    // mundo (misma convencion que stbi_set_flip_vertically_on_load en
    // TextureManager y que IsoMath), asi que hay que voltear para que
    // OpenGL (NDC +y = arriba) pinte en el sentido correcto.
    glm::mat4 getViewProjectionMatrix() const;

    // Punto de pantalla (pixeles, origen arriba-izquierda) -> mundo,
    // teniendo en cuenta paneo y zoom actuales. Inversa exacta de "que
    // pixel de pantalla ocupa este punto de mundo".
    Vector2 screenToWorld(const Vector2& screenPos) const;

    // Mundo -> celda de grid isometrico. Wrapper fino sobre IsoMath (unica
    // fuente de verdad, ver Core/Math/IsoMath.h): delega en screenToGrid, no
    // reimplementa la formula. tileWidth/Height con default 64x32 (tile 2:1).
    // Fractura #4 del analisis de coherencia (RESUELTA por delegacion, ver
    // ARCHITECTURE.md).
    GridCoord worldToGrid(const Vector2& worldPos, float tileWidth = 64.0f,
                          float tileHeight = 32.0f) const;

private:
    Vector2 m_position;
    Vector2 m_targetPosition;
    float m_zoom = 1.0f;
    float m_lerpSpeed = 8.0f;
    int m_viewportWidth;
    int m_viewportHeight;

    // Estado de transitionTo(). m_transitionActive controla si update()
    // usa esta interpolacion de duracion fija en vez del lerp exponencial
    // normal (ver el comentario de transitionTo() en la seccion publica).
    bool m_transitionActive = false;
    float m_transitionElapsed = 0.0f;
    float m_transitionDuration = 0.0f;
    Easing m_transitionEasing = Easing::Linear;
    Vector2 m_transitionStartPosition;
    float m_transitionStartZoom = 1.0f;
    Vector2 m_transitionTargetPosition;
    float m_transitionTargetZoom = 1.0f;
};
