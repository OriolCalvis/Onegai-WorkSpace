#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Render/HudElement.h"

class BitmapFont;
class SpriteBatch;
class Texture;

// Widgets de texto (motor_grafico_gantt_rpg.puml, Fase 9: "Menu de
// comandos" + "Cuadro de dialogo"), montados sobre BitmapFont.h (el atlas
// procedural) y HudElement.h/HudWidgets.h (framework + HudPanel para el
// fondo). Ninguno posee el BitmapFont* ni el Texture* blanco que reciben:
// mismo criterio de no-propiedad que HudBar/HudPanel (ver su comentario)
// -- BitmapFont vive tanto como haga falta en el sitio que arma el HUD
// (ej. Application, o un demo), no aqui.

// Una o varias lineas (separadas por '\n') de texto en una posicion fija
// del HUD. El widget mas simple posible: sin wrapping automatico, sin
// alineacion mas alla de la que ya da HudTransform::anchor -- si hace
// falta texto envuelto a un ancho, se resuelve fuera (partiendo el string
// en '\n' antes de llamar a setText()).
class HudText : public IHudElement {
public:
    HudText(const HudTransform& transform, BitmapFont* font);

    void setText(const std::string& text) { m_text = text; }
    const std::string& text() const { return m_text; }
    void setColor(const Vector4& color) { m_color = color; }

    void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const override;

private:
    HudTransform m_transform;
    BitmapFont* m_font;
    std::string m_text;
    Vector4 m_color{1.0f, 1.0f, 1.0f, 1.0f};
};

// Menu de comandos navegable (Atacar/Habilidad/Objeto/Huir, estilo Dragon
// Quest clasico): una lista de opciones de texto en columna, un indice de
// seleccion resaltado con m_highlightColor en vez de m_textColor, y
// navegacion vertical con wraparound (moveUp()/moveDown()). No lee input
// directamente: eso es responsabilidad de quien orqueste el bucle de
// juego (mismo criterio que IHudElement sin update(), ver HudElement.h;
// aqui seria "cada vez que el jugador pulsa arriba/abajo, llamar a
// moveUp()/moveDown(); al confirmar, leer selectedIndex()").
class HudCommandMenu : public IHudElement {
public:
    HudCommandMenu(const HudTransform& transform, BitmapFont* font,
                   std::vector<std::string> options);

    // Tamano exacto que ocupara el menu al dibujarse: el ancho de la
    // opcion mas larga CON el prefijo "> " del cursor, y el alto de
    // todas las lineas con su interlineado (kLineSpacing, ver el .cpp).
    // Para que quien componga el HUD (Application, level_editor) mida
    // su panel de fondo contra ESTO y no contra una cuenta duplicada
    // que se desincronice (defecto 06 del documento de diseno del
    // editor). {0,0} con options vacio.
    static Vector2 contentSize(const BitmapFont& font, const std::vector<std::string>& options);

    // No-op si options() esta vacio (nada que mover). Con wraparound: subir
    // desde el primero va al ultimo, bajar desde el ultimo va al primero
    // -- igual que la mayoria de menus de RPG por turnos, evita un "borde
    // muerto" que el jugador tenga que notar y detenerse.
    void moveUp();
    void moveDown();

    std::size_t selectedIndex() const { return m_selectedIndex; }
    // Clampado a [0, options().size()-1] (o 0 si options() esta vacio):
    // nunca deja un indice fuera de rango, mismo criterio defensivo que
    // BattleState::resolveAllyAction con indices invalidos.
    void setSelectedIndex(std::size_t index);

    const std::vector<std::string>& options() const { return m_options; }

    // Reemplaza la lista completa (el indice se re-clampa al nuevo
    // tamano). Para menus cuyo contenido cambia en marcha: el editor de
    // niveles lo usa para mostrar una VENTANA deslizante de una paleta
    // larga -- un tileset de 24 tiles no cabe entero en su panel, asi
    // que se muestran solo las N entradas alrededor de la seleccionada.
    void setOptions(std::vector<std::string> options);

    void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const override;

private:
    HudTransform m_transform;
    BitmapFont* m_font;
    std::vector<std::string> m_options;
    std::size_t m_selectedIndex = 0;
    Vector4 m_textColor{0.85f, 0.85f, 0.85f, 1.0f};
    Vector4 m_highlightColor{1.0f, 0.9f, 0.2f, 1.0f};
};

// Cuadro de dialogo: un panel de fondo (mismo mecanismo que HudPanel, ver
// HudWidgets.h -- un quad con whiteTexture) mas las ultimas maxLines()
// lineas de un log de texto. Pensado exactamente para BattleState::log()
// (ver su comentario: "para que un cuadro de dialogo del HUD muestre las
// ultimas N lineas" -- esto es esa pieza). setLines() reemplaza el
// contenido completo cada vez que se llama (se espera llamarlo tras cada
// BattleState::resolveAllyAction()/resolveEnemyTurn() con battle.log()),
// sin estado de scroll que mantener: simple a proposito, ver
// motor_grafico_dafo.md sobre sobre-ingenieria.
class HudDialogueBox : public IHudElement {
public:
    HudDialogueBox(const HudTransform& transform, BitmapFont* font, Texture* whiteTexture,
                   std::size_t maxLines = 4);

    // Se queda solo con las ultimas maxLines() lineas de "lines" (o menos,
    // si "lines" tiene menos): nunca desborda el cuadro con texto que no
    // cabria.
    void setLines(const std::vector<std::string>& lines);
    std::size_t maxLines() const { return m_maxLines; }

    void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const override;

private:
    HudTransform m_transform;
    BitmapFont* m_font;
    Texture* m_whiteTexture;
    std::size_t m_maxLines;
    std::vector<std::string> m_visibleLines;
    Vector4 m_backgroundColor{0.05f, 0.05f, 0.08f, 0.85f};
    Vector4 m_textColor{1.0f, 1.0f, 1.0f, 1.0f};
};
