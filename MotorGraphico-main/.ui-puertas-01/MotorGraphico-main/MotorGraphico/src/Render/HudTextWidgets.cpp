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

#include "Render/HudTextWidgets.h"

#include "Core/Math/UVRect.h"
#include "Render/BitmapFont.h"
#include "Render/SpriteBatch.h"

#include <algorithm>
#include <cstddef>
#include <utility>

#include "Core/Math/Vector2.h"

namespace {

// Mismo patron que HudWidgets.cpp: whiteTexture es 1x1, esta UV completa
// siempre cae en ese unico pixel.
constexpr UVRect kFullUV{0.0f, 0.0f, 1.0f, 1.0f};

// Margen interior en pixeles entre el borde de un panel (HudDialogueBox,
// futuros widgets con fondo) y el texto que dibuja encima: sin esto el
// texto quedaria pegado al borde del panel, dificil de leer. Valor fijo,
// no parametrizable: no hay hoy ningun caso de uso que necesite ajustarlo
// (mismo criterio que SpriteBatch::kRingCount).
constexpr float kTextPadding = 6.0f;

// Prefijo de la opcion seleccionada en HudCommandMenu: un cursor de texto
// simple ("> Atacar" en vez de "Atacar"), redundante con el color de resaltado
// a proposito -- un HUD que solo cambia de color puede ser invisible para
// quien no distinga bien esos colores concretos; el prefijo funciona
// igual en blanco y negro.
const std::string kSelectedPrefix = "> ";
const std::string kUnselectedPrefix = "  ";

}  // namespace

HudText::HudText(const HudTransform& transform, BitmapFont* font)
    : m_font(font), m_data(transform) {}

void HudText::setText(const std::string& text) {
    m_data.text = text;
    // F0 (auditoria UI): con caja, el texto se envuelve a su ancho; las
    // lineas largas ya no desbordan el panel. Sin caja (size.x <= 0),
    // comportamiento antiguo: se dibuja tal cual.
    if (m_data.transform.size.x > 0.0f && m_font != nullptr) {
        const auto lines = BitmapFont::wrapText(text, static_cast<int>(m_data.transform.size.x),
                                                m_font->glyphWidth());
        m_data.renderText.clear();
        for (std::size_t i = 0; i < lines.size(); ++i) {
            if (i > 0) {
                m_data.renderText.push_back('\n');
            }
            m_data.renderText += lines[i];
        }
    } else {
        m_data.renderText = text;
    }
}

void HudText::render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const {
    Vector2 topLeft = m_data.transform.resolveTopLeft(viewportWidth, viewportHeight);
    m_font->drawText(batch, m_data.renderText, topLeft, m_data.color);
}

HudCommandMenu::HudCommandMenu(const HudTransform& transform, BitmapFont* font,
                               std::vector<std::string> options)
    : m_font(font), m_data(transform) {
    m_data.options = std::move(options);
}

Vector2 HudCommandMenu::contentSize(const BitmapFont& font,
                                    const std::vector<std::string>& options) {
    return contentSize(font, options, 0);
}

Vector2 HudCommandMenu::contentSize(const BitmapFont& font,
                                    const std::vector<std::string>& options, int maxWidth) {
    if (options.empty()) {
        return Vector2{0.0f, 0.0f};
    }
    // Sale de layoutRows() y no de un max() por opcion como antes, para que
    // el ancho y el alto que salen de aqui sean los de las MISMAS filas que
    // dibuja render(): si las dos cosas se midieran por separado, un techo
    // de ancho haria que el panel quedara mas bajo que el texto.
    //
    // Con maxWidth <= 0 layoutRows() devuelve una fila por opcion, y el
    // ancho sale igual que antes: el prefijo del cursor mide lo mismo
    // seleccionado que sin seleccionar ("> " y "  "), y se mide con la
    // opcion 0 marcada precisamente para no quedarse un glifo corto.
    const auto rows = layoutRows(font, options, /*selectedIndex=*/0, maxWidth);
    float widest = 0.0f;
    for (const HudMenuRow& row : rows) {
        widest = std::max(widest, font.measureText(row.text).x);
    }
    return Vector2{widest,
                   static_cast<float>(font.lineHeight()) * static_cast<float>(rows.size())};
}

int HudCommandMenu::usableWidth(int boxWidth, int boxLeft, int viewportWidth) {
    // Sin pantalla conocida (una prueba que no monta contexto) no se puede
    // poner un techo: solo cuenta la caja, que es lo que se podia
    // escribir antes.
    if (viewportWidth <= 0) {
        return boxWidth;
    }
    // El margen de los dos lados es el mismo kTextPadding que usa
    // HudDialogueBox: el texto no debe tocar ni el borde de su caja ni el
    // borde de la pantalla.
    const int margin = static_cast<int>(kTextPadding);
    const int toRightEdge = viewportWidth - boxLeft - margin;
    const int wholeScreen = viewportWidth - 2 * margin;
    return std::max(0, std::min(boxWidth, std::min(toRightEdge, wholeScreen)));
}

std::vector<HudMenuRow> HudCommandMenu::layoutRows(const BitmapFont& font,
                                                   const std::vector<std::string>& options,
                                                   std::size_t selectedIndex,
                                                   int usable) {
    std::vector<HudMenuRow> rows;
    // El prefijo del cursor ocupa dos glifos, y las filas de continuacion
    // llevan la misma sangria para que el texto quede debajo de la opcion y
    // no debajo del cursor.
    const int glyphW = font.glyphWidth();
    const int room = usable - 2 * glyphW;

    for (std::size_t i = 0; i < options.size(); ++i) {
        const std::string prefix = (i == selectedIndex) ? kSelectedPrefix : kUnselectedPrefix;
        if (usable <= 0 || room <= 0) {
            // Caja tan estrecha que no cabe ni el prefijo (o sin techo):
            // una fila por opcion, como antes. wrapText() con un ancho de
            // cero o menor devuelve el texto entero, y con el de un solo
            // glifo corta sin inventar guiones (C-06), asi que ninguna de
            // las dos ramas puede entrar en bucle.
            rows.push_back(HudMenuRow{prefix + options[i], i});
            continue;
        }
        bool first = true;
        for (const std::string& part : BitmapFont::wrapText(options[i], room, glyphW)) {
            rows.push_back(HudMenuRow{(first ? prefix : kUnselectedPrefix) + part, i});
            first = false;
        }
    }
    return rows;
}

void HudCommandMenu::moveUp() {
    if (m_data.options.empty()) {
        return;
    }
    // Wraparound: 0 - 1 en size_t desbordaria a un numero enorme, no a
    // -1, por eso el caso especial en vez de "selectedIndex - 1" directo.
    m_data.selectedIndex =
        (m_data.selectedIndex == 0) ? m_data.options.size() - 1 : m_data.selectedIndex - 1;
}

void HudCommandMenu::moveDown() {
    if (m_data.options.empty()) {
        return;
    }
    m_data.selectedIndex = (m_data.selectedIndex + 1) % m_data.options.size();
}

void HudCommandMenu::setOptions(std::vector<std::string> options) {
    m_data.options = std::move(options);
    // Re-clampa: la lista nueva puede ser mas corta que el indice actual
    // (mismo criterio defensivo que setSelectedIndex, ver su comentario).
    setSelectedIndex(m_data.selectedIndex);
}

void HudCommandMenu::setSelectedIndex(std::size_t index) {
    if (m_data.options.empty()) {
        m_data.selectedIndex = 0;
        return;
    }
    m_data.selectedIndex = std::min(index, m_data.options.size() - 1);
}

void HudCommandMenu::render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const {
    Vector2 topLeft = m_data.transform.resolveTopLeft(viewportWidth, viewportHeight);
    float lineHeight = static_cast<float>(m_font->lineHeight());

    // AV1-03 — Las filas salen de layoutRows() con el techo de usableWidth()
    // (nunca mas alla de la caja ni de la pantalla), y no de una fila por
    // opcion: hasta ahora una etiqueta larga se salia del panel y de la
    // pantalla sin avisar, y los seis menus del juego se autodimensionan
    // con contentSize(), o sea, que la caja crecia con ella.
    const int usable = usableWidth(static_cast<int>(m_data.transform.size.x),
                                   static_cast<int>(topLeft.x), viewportWidth);
    const auto rows = layoutRows(*m_font, m_data.options, m_data.selectedIndex, usable);

    for (std::size_t i = 0; i < rows.size(); ++i) {
        const bool selected = (rows[i].optionIndex == m_data.selectedIndex);
        Vector2 linePos{topLeft.x, topLeft.y + lineHeight * static_cast<float>(i)};
        m_font->drawText(batch, rows[i].text, linePos, selected ? m_data.highlightColor : m_data.textColor);
    }
}

HudDialogueBox::HudDialogueBox(const HudTransform& transform, BitmapFont* font,
                               Texture* whiteTexture, std::size_t maxLines)
    : m_font(font), m_whiteTexture(whiteTexture), m_data(transform, maxLines) {}

void HudDialogueBox::setLines(const std::vector<std::string>& lines) {
    // F0 (auditoria UI): cada linea se ENVUELVE al ancho util del panel
    // antes de entrar — antes solo se limitaba el NUMERO de lineas y las
    // largas se salian del cuadro por la derecha. Despues, solo las
    // ultimas maxLines() (si "lines" trae menos, se quedan todas).
    std::vector<std::string> wrapped;
    const int usable = static_cast<int>(m_data.transform.size.x - 2.0f * kTextPadding);
    for (const std::string& line : lines) {
        if (usable > 0 && m_font != nullptr) {
            const auto parts = BitmapFont::wrapText(line, usable, m_font->glyphWidth());
            wrapped.insert(wrapped.end(), parts.begin(), parts.end());
        } else {
            wrapped.push_back(line);
        }
    }
    std::size_t count = std::min(wrapped.size(), m_data.maxLines);
    m_data.visibleLines.assign(wrapped.end() - static_cast<std::ptrdiff_t>(count), wrapped.end());
}

void HudDialogueBox::render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const {
    Vector2 topLeft = m_data.transform.resolveTopLeft(viewportWidth, viewportHeight);

    // Fondo: mismo mecanismo que HudPanel (ver HudWidgets.cpp).
    batch.submit(topLeft, m_data.transform.size, kFullUV, m_whiteTexture, m_data.backgroundColor);

    float lineHeight = static_cast<float>(m_font->lineHeight());
    Vector2 textOrigin{topLeft.x + kTextPadding, topLeft.y + kTextPadding};
    for (std::size_t i = 0; i < m_data.visibleLines.size(); ++i) {
        Vector2 linePos{textOrigin.x, textOrigin.y + lineHeight * static_cast<float>(i)};
        m_font->drawText(batch, m_data.visibleLines[i], linePos, m_data.textColor);
    }
}
