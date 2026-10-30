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

#include "Core/Math/Vector2.h"
#include "Render/HudTextWidgetsData.h"

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
// del HUD. F0 (auditoria UI): si el transform trae caja (size.x > 0), el
// texto SE ENVUELVE a ese ancho en setText() (BitmapFont::wrapText); sin
// caja se comporta como antes (sin wrapping). Antes la caja se ignoraba
// y las lineas largas desbordaban el panel.
class HudText : public IHudElement {
public:
    HudText(const HudTransform& transform, BitmapFont* font);

    void setText(const std::string& text);
    const std::string& text() const { return m_data.text; }
    void setColor(const Vector4& color) { m_data.color = color; }

    void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const override;

private:
    BitmapFont* m_font;  // colaborador de render, no propietario.
    HudTextData m_data;  // estado visual y geométrico GL-free.
};

// Menu de comandos navegable (Atacar/Tajo/Cura/Pocion/Huir, estilo Dragon
// Quest clasico): una lista de opciones de texto en columna, un indice de
// seleccion resaltado con highlightColor en vez de textColor, y
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

    // AV1-03 — Lo mismo, pero con un TECHO de ancho: si las opciones no
    // caben en "maxWidth" pixels se envuelven y el alto que sale ya es el
    // de las filas envueltas, no el de las opciones. Con maxWidth <= 0 es
    // exactamente la sobrecarga de arriba.
    static Vector2 contentSize(const BitmapFont& font, const std::vector<std::string>& options,
                               int maxWidth);

    // AV1-03 — Ancho por el que se puede escribir una linea de este menu
    // SIN salirse de la pantalla: el menor entre el ancho de la caja, lo
    // que queda a la derecha de su borde izquierdo y la pantalla menos
    // el margen de los dos lados.
    //
    // El segundo y el tercero de esos tres minimos son los que importan.
    // Los seis menus del juego (titulo, pausa, game over, comandos, tabs
    // del inventario, cartel de negocio) se dimensionan a si mismos con
    // contentSize(), o sea, la caja crece con la etiqueta mas larga: sin
    // este techo, un nombre de proyecto largo o una habilidad de catalogo
    // empuja el texto fuera de la pantalla y de su panel, sin avisar.
    // viewportWidth <= 0 = no se sabe la pantalla (pruebas): se usa solo
    // el ancho de la caja, que es el comportamiento de siempre.
    // Pura y sin GL.
    static int usableWidth(int boxWidth, int boxLeft, int viewportWidth);

    // AV1-03 — Las filas del menu tal como las dibuja render(), pero como
    // dato: cada opcion envuelta a "usableWidth" (con el prefijo "> " del
    // cursor en la primera fila y dos espacios de sangria en las de
    // continuacion, para que el texto no se meta debajo del cursor). Una
    // opcion puede ocupar N filas, y cada una dice a que opcion pertenece.
    // Con usableWidth <= 0, una fila por opcion (antes).
    //
    // Vive aqui y no dentro de render() para que el ancho y el alto que
    // devuelve contentSize() no se puedan desincronizar de lo que se ve:
    // las dos cosas salen de esta misma funcion. Pura y sin GL: es lo que
    // la prueba ejercita sin contexto OpenGL.
    static std::vector<HudMenuRow> layoutRows(const BitmapFont& font,
                                              const std::vector<std::string>& options,
                                              std::size_t selectedIndex, int usableWidth);

    // No-op si options() esta vacio (nada que mover). Con wraparound: subir
    // desde el primero va al ultimo, bajar desde el ultimo va al primero
    // -- igual que la mayoria de menus de RPG por turnos, evita un "borde
    // muerto" que el jugador tenga que notar y detenerse.
    void moveUp();
    void moveDown();

    std::size_t selectedIndex() const { return m_data.selectedIndex; }
    // Clampado a [0, options().size()-1] (o 0 si options() esta vacio):
    // nunca deja un indice fuera de rango, mismo criterio defensivo que
    // BattleState::resolveAllyAction con indices invalidos.
    void setSelectedIndex(std::size_t index);

    const std::vector<std::string>& options() const { return m_data.options; }

    // Reemplaza la lista completa (el indice se re-clampa al nuevo
    // tamano). Para menus cuyo contenido cambia en marcha: el editor de
    // niveles lo usa para mostrar una VENTANA deslizante de una paleta
    // larga -- un tileset de 24 tiles no cabe entero en su panel, asi
    // que se muestran solo las N entradas alrededor de la seleccionada.
    void setOptions(std::vector<std::string> options);
    // H2-06 — Un menu que cambia de opciones en marcha (el de combate con
    // las cartas del heroe) se vuelve a medir con contentSize() y se recoloca.
    const HudTransform& transform() const { return m_data.transform; }
    void setTransform(const HudTransform& transform) { m_data.transform = transform; }

    void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const override;

private:
    BitmapFont* m_font;         // colaborador de render, no propietario.
    HudCommandMenuData m_data;  // estado visual y de navegación GL-free.
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
    std::size_t maxLines() const { return m_data.maxLines; }

    void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const override;

private:
    BitmapFont* m_font;         // colaborador de render, no propietario.
    Texture* m_whiteTexture;    // colaborador de render, no propietario.
    HudDialogueBoxData m_data;  // estado visual y geométrico GL-free.
};
