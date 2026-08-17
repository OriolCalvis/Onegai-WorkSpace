#pragma once

#include "Core/Math/Vector4.h"
#include "Render/HudElement.h"

class Texture;

// Widgets concretos minimos sobre el framework de HudElement.h/
// HudManager.h (motor_grafico_gantt_rpg.puml, Fase 9: "Widgets base").
// Ambos dibujan rectangulos solidos via SpriteBatch reutilizando el
// mecanismo de tint por vertice (Vector4, Fase 4): necesitan un Texture*
// de un unico pixel blanco (whiteTexture) para que "textura * tint" de
// por resultado el tint solo -- no hace falta ningun shader nuevo (ver
// examples/demo_hud.cpp para como crear ese Texture de 1x1, mismo patron
// que makeProceduralLightmap() en demo_lighting.cpp).
//
// Todavia no hay texto en el HUD (sin sistema de fuentes bitmap, ver
// motor_grafico_gantt_rpg.puml Fase 9): HudBar/HudPanel son deliberamente
// "solo color", la base sobre la que montar el menu de comandos y el
// cuadro de dialogo mas adelante sin cambiar IHudElement/HudManager.

// Barra de valor (vida, mana...): quads superpuestos -- borde opcional
// (m_borderColor, un quad ligeramente mayor DETRAS), fondo
// (m_backgroundColor, tamano completo) y relleno (m_fillColor, ancho
// proporcional a value()/maxValue()). Value clampado a [0, maxValue] en
// setValue(), igual que ColorLUT::quantizeChannel clampa su entrada: un
// HudBar nunca puede dibujar una barra "por encima del 100%" ni negativa
// por un valor invalido de quien la usa.
//
// Extras opcionales (todos DESACTIVADOS por defecto, para que los
// llamadores existentes -- demo_hud.cpp, demo_battle_hud.cpp -- se
// comporten exactamente igual sin tocarlos):
//  - Animacion (setAnimationSpeed + update()): el valor DIBUJADO
//    persigue al valor real con suavizado exponencial, y la franja entre
//    ambos se pinta con m_ghostColor (el clasico "resto blanco" que se
//    encoge tras recibir dano, estilo Street Fighter/Zelda). update() es
//    un metodo PROPIO de HudBar, no de IHudElement: el comentario de
//    HudElement.h dice "si algun widget futuro lo necesita, se anade
//    entonces" -- es entonces, y anadirlo aqui no cambia la interfaz.
//  - Colores por umbral (setThresholdColors): el relleno cambia segun el
//    ratio REAL (no el animado): >50% high, >25% mid, resto low. El
//    aviso visual llega al instante aunque la animacion vaya por detras.
class HudBar : public IHudElement {
public:
    HudBar(const HudTransform& transform, Texture* whiteTexture);

    void setValue(float value);
    void setMaxValue(float maxValue);
    void setColors(const Vector4& background, const Vector4& fill);

    // Grosor <= 0 desactiva el borde (estado por defecto).
    void setBorder(const Vector4& color, float thickness);

    // speed: fraccion de la distancia restante recorrida por segundo
    // (suavizado exponencial, mismo espiritu que el lerp de Camera).
    // <= 0 desactiva la animacion: el valor dibujado salta al real.
    void setAnimationSpeed(float speed);
    // Avanza el valor dibujado hacia el real. No-op sin animacion. Override
    // de IHudElement::update (fractura #3, ver ARCHITECTURE.md): antes era
    // un metodo propio de HudBar sin relacion con el resto del motor; ahora
    // es coherente con el contrato update() que usan Entity/Camera.
    void update(float deltaTime) override;

    // Activa el relleno por umbrales. Llamar una vez; los tres colores
    // sustituyen a m_fillColor (que queda sin uso mientras este activo).
    void setThresholdColors(const Vector4& high, const Vector4& mid, const Vector4& low);

    float value() const { return m_value; }
    float maxValue() const { return m_maxValue; }
    // Valor que se esta dibujando ahora mismo (== value() sin animacion).
    float displayedValue() const { return m_displayedValue; }

    void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const override;

private:
    const Vector4& currentFillColor() const;

    HudTransform m_transform;
    Texture* m_whiteTexture;
    float m_value = 1.0f;
    float m_maxValue = 1.0f;
    float m_displayedValue = 1.0f;
    float m_animationSpeed = 0.0f;   // <= 0: sin animacion
    float m_borderThickness = 0.0f;  // <= 0: sin borde
    bool m_useThresholds = false;
    Vector4 m_backgroundColor{0.15f, 0.15f, 0.18f, 0.85f};
    Vector4 m_fillColor{0.8f, 0.15f, 0.15f, 1.0f};
    Vector4 m_borderColor{0.85f, 0.85f, 0.9f, 0.9f};
    Vector4 m_ghostColor{1.0f, 1.0f, 1.0f, 0.55f};
    Vector4 m_thresholdHigh{0.25f, 0.75f, 0.3f, 1.0f};
    Vector4 m_thresholdMid{0.9f, 0.75f, 0.2f, 1.0f};
    Vector4 m_thresholdLow{0.85f, 0.2f, 0.15f, 1.0f};
};

// Panel solido: un unico quad de color, pensado como fondo/contenedor
// para agrupar otros widgets encima (ej. el area del futuro menu de
// comandos, o del cuadro de dialogo). Composicion simple: un HudPanel y
// varios HudBar/otros widgets superpuestos se anaden todos a un mismo
// HudManager, en el orden en que deben dibujarse (el panel primero, para
// que quede debajo).
class HudPanel : public IHudElement {
public:
    HudPanel(const HudTransform& transform, Texture* whiteTexture,
             const Vector4& color = Vector4{0.05f, 0.05f, 0.08f, 0.75f});

    void setColor(const Vector4& color) { m_color = color; }

    // Marco opcional: un quad "thickness" pixeles mayor por cada lado,
    // dibujado DETRAS del panel (mismo mecanismo que HudBar::setBorder).
    // Grosor <= 0 desactiva el borde (estado por defecto).
    void setBorder(const Vector4& color, float thickness);

    void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const override;

private:
    HudTransform m_transform;
    Texture* m_whiteTexture;
    Vector4 m_color;
    Vector4 m_borderColor{0.85f, 0.85f, 0.9f, 0.9f};
    float m_borderThickness = 0.0f;  // <= 0: sin borde
};
