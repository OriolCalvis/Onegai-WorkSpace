// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-14  (fase G, 3 j) - HUD, diàlegs i comerç amb accions visibles
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include "Render/HudWidgetsData.h"

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
// (borderColor, un quad ligeramente mayor DETRAS), fondo
// (backgroundColor, tamano completo) y relleno (fillColor, ancho
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
//    ambos se pinta con ghostColor (el clasico "resto blanco" que se
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
    // sustituyen a fillColor (que queda sin uso mientras este activo).
    void setThresholdColors(const Vector4& high, const Vector4& mid, const Vector4& low);

    float value() const { return m_data.value; }
    float maxValue() const { return m_data.maxValue; }
    // Valor que se esta dibujando ahora mismo (== value() sin animacion).
    float displayedValue() const { return m_data.displayedValue; }

    void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const override;

private:
    const Vector4& currentFillColor() const;

    Texture* m_whiteTexture;  // colaborador de render, no propietario.
    HudBarData m_data;        // estado visual y geométrico GL-free.
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

    void setColor(const Vector4& color) { m_data.color = color; }
    // H2-06 — para el panel de un menu que crece (ver HudCommandMenu::setTransform).
    const HudTransform& transform() const { return m_data.transform; }
    void setTransform(const HudTransform& transform) { m_data.transform = transform; }

    // Marco opcional: un quad "thickness" pixeles mayor por cada lado,
    // dibujado DETRAS del panel (mismo mecanismo que HudBar::setBorder).
    // Grosor <= 0 desactiva el borde (estado por defecto).
    void setBorder(const Vector4& color, float thickness);

    void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const override;

private:
    Texture* m_whiteTexture;  // colaborador de render, no propietario.
    HudPanelData m_data;      // estado visual y geométrico GL-free.
};
