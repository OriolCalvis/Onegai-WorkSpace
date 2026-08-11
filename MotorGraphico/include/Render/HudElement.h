#pragma once

#include "Core/Math/Vector2.h"

class SpriteBatch;

// Framework de HUD modular (motor_grafico_gantt_rpg.puml, Fase 9): la
// pieza base es IHudElement (interfaz minima, un solo metodo), y como se
// POSICIONA cada elemento en pantalla es HudTransform, independiente de
// como se dibuja. Un HudManager (ver HudManager.h) compone varios
// IHudElement* y los dibuja en orden, con su propia proyeccion
// ortografica en pixeles de pantalla -- NO la de Camera (mundo isometrico
// con zoom/paneo): el HUD no debe moverse ni escalar con la camara.
//
// Deliberadamente pequeno: nada de layout automatico (grids, flexbox...),
// nada de eventos/input todavia. Un widget se posiciona con un ancla +
// desplazamiento (igual que la mayoria de motores 2D: Unity UI, Godot
// Control), y punto. Anadir mas se puede hacer sin romper esto (ver
// motor_grafico_gantt_rpg.puml, Fase 9: fuentes bitmap, menu de comandos,
// cuadro de dialogo son widgets NUEVOS sobre esta misma base, no cambios
// a IHudElement/HudTransform).

// Punto de anclaje dentro del viewport. Los 9 valores estandar de
// cualquier sistema de anclas 2D (Unity UI, Godot Control...).
enum class HudAnchor {
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

// Posicion + tamano de un elemento de HUD, resuelto a pixeles de pantalla
// (origen arriba-izquierda, igual que HudManager::render()) segun el
// tamano del viewport. Pura: no toca GL, no depende de nada mas que
// aritmetica, así que se puede testear sin ventana ni contexto OpenGL
// (ver examples/demo_hud.cpp, testHudTransform()).
struct HudTransform {
    HudAnchor anchor = HudAnchor::TopLeft;
    // Desplazamiento en pixeles desde el anchor HACIA EL INTERIOR de la
    // pantalla (un margen, nunca hacia fuera): en TopRight, offset.x
    // positivo mueve el elemento hacia la izquierda (no lo saca de
    // pantalla); en TopLeft, offset.x positivo mueve hacia la derecha.
    // Ver resolveTopLeft() para el porque exacto.
    Vector2 offset{0.0f, 0.0f};
    Vector2 size{100.0f, 20.0f};

    // Devuelve la esquina superior-izquierda del rectangulo resultante,
    // en pixeles de pantalla (origen arriba-izquierda). El rectangulo
    // completo es [resolveTopLeft(), resolveTopLeft() + size].
    Vector2 resolveTopLeft(int viewportWidth, int viewportHeight) const;
};

// Interfaz minima de un elemento de HUD: sabe dibujarse y, opcionalmente,
// avanzar su propia animacion con el tiempo. update(deltaTime) tiene cuerpo
// vacio por defecto: la mayoria de widgets no tienen estado temporal (su
// valor lo fija quien los posee, ej. "hpBar.setValue(player.health())" cada
// frame); los que SI animan por si mismos (HudBar suaviza el relleno hacia
// el valor objetivo) hacen override. Asi un widget animado es coherente con
// el contrato IUpdatable del resto del motor, sin obligar a los widgets
// estaticos a implementar nada (fractura #3 del analisis de coherencia,
// ver ARCHITECTURE.md).
class IHudElement {
public:
    virtual ~IHudElement() = default;

    // batch: del que sea dueno HudManager::render() (ver su comentario);
    // el elemento solo llama a batch.submit(), nunca begin()/end()/flush()
    // (eso es responsabilidad de quien orquesta el frame completo).
    virtual void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const = 0;

    // Avance de animacion propio (opcional). Lo llama
    // HudManager::update(), que recorre TODOS los elementos: dejarlo en
    // manos de quien posee cada widget ya causo un bug real -- la barra
    // de moral se creo con setAnimationSpeed() pero nadie la actualizaba,
    // asi que no se animaba, y nada avisaba de ello. Un contrato que hay
    // que acordarse de invocar a mano por cada widget nuevo es un bug
    // esperando su turno.
    //
    // Cuerpo vacio por defecto: la mayoria de widgets no tienen estado
    // temporal (su valor lo fija quien los posee cada frame), y no deben
    // verse obligados a implementar nada.
    virtual void update(float /*deltaTime*/) {}

    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

protected:
    bool m_visible = true;
};
