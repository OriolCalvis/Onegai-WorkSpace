#include "Render/HudTextWidgets.h"

#include "Core/Math/UVRect.h"
#include "Render/BitmapFont.h"
#include "Render/SpriteBatch.h"

#include <algorithm>

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
// simple ("> Atacar" en vez de "Atacar"), redundante con m_highlightColor
// a proposito -- un HUD que solo cambia de color puede ser invisible para
// quien no distinga bien esos colores concretos; el prefijo funciona
// igual en blanco y negro.
const std::string kSelectedPrefix = "> ";
const std::string kUnselectedPrefix = "  ";

}  // namespace

HudText::HudText(const HudTransform& transform, BitmapFont* font)
    : m_transform(transform), m_font(font) {}

void HudText::setText(const std::string& text) {
    m_text = text;
    // F0 (auditoria UI): con caja, el texto se envuelve a su ancho; las
    // lineas largas ya no desbordan el panel. Sin caja (size.x <= 0),
    // comportamiento antiguo: se dibuja tal cual.
    if (m_transform.size.x > 0.0f && m_font != nullptr) {
        const auto lines = BitmapFont::wrapText(
            text, static_cast<int>(m_transform.size.x), m_font->glyphWidth());
        m_renderText.clear();
        for (std::size_t i = 0; i < lines.size(); ++i) {
            if (i > 0) {
                m_renderText.push_back('\n');
            }
            m_renderText += lines[i];
        }
    } else {
        m_renderText = text;
    }
}

void HudText::render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const {
    Vector2 topLeft = m_transform.resolveTopLeft(viewportWidth, viewportHeight);
    m_font->drawText(batch, m_renderText, topLeft, m_color);
}

HudCommandMenu::HudCommandMenu(const HudTransform& transform, BitmapFont* font,
                               std::vector<std::string> options)
    : m_transform(transform), m_font(font), m_options(std::move(options)) {}

Vector2 HudCommandMenu::contentSize(const BitmapFont& font,
                                    const std::vector<std::string>& options) {
    if (options.empty()) {
        return Vector2{0.0f, 0.0f};
    }
    // El ancho se mide CON el prefijo del cursor porque render() lo
    // antepone siempre (seleccionada o no, ambos prefijos ocupan lo
    // mismo): medir sin el dejaria el panel dos caracteres corto.
    float widest = 0.0f;
    for (const std::string& option : options) {
        widest = std::max(widest, font.measureText(kSelectedPrefix + option).x);
    }
    return Vector2{widest,
                   static_cast<float>(font.lineHeight()) * static_cast<float>(options.size())};
}

void HudCommandMenu::moveUp() {
    if (m_options.empty()) {
        return;
    }
    // Wraparound: 0 - 1 en size_t desbordaria a un numero enorme, no a
    // -1, por eso el caso especial en vez de "m_selectedIndex - 1" directo.
    m_selectedIndex = (m_selectedIndex == 0) ? m_options.size() - 1 : m_selectedIndex - 1;
}

void HudCommandMenu::moveDown() {
    if (m_options.empty()) {
        return;
    }
    m_selectedIndex = (m_selectedIndex + 1) % m_options.size();
}

void HudCommandMenu::setOptions(std::vector<std::string> options) {
    m_options = std::move(options);
    // Re-clampa: la lista nueva puede ser mas corta que el indice actual
    // (mismo criterio defensivo que setSelectedIndex, ver su comentario).
    setSelectedIndex(m_selectedIndex);
}

void HudCommandMenu::setSelectedIndex(std::size_t index) {
    if (m_options.empty()) {
        m_selectedIndex = 0;
        return;
    }
    m_selectedIndex = std::min(index, m_options.size() - 1);
}

void HudCommandMenu::render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const {
    Vector2 topLeft = m_transform.resolveTopLeft(viewportWidth, viewportHeight);
    float lineHeight = static_cast<float>(m_font->lineHeight());

    for (std::size_t i = 0; i < m_options.size(); ++i) {
        bool selected = (i == m_selectedIndex);
        std::string line = (selected ? kSelectedPrefix : kUnselectedPrefix) + m_options[i];
        Vector2 linePos{topLeft.x, topLeft.y + lineHeight * static_cast<float>(i)};
        m_font->drawText(batch, line, linePos, selected ? m_highlightColor : m_textColor);
    }
}

HudDialogueBox::HudDialogueBox(const HudTransform& transform, BitmapFont* font,
                               Texture* whiteTexture, std::size_t maxLines)
    : m_transform(transform)
    , m_font(font)
    , m_whiteTexture(whiteTexture)
    , m_maxLines(maxLines) {}

void HudDialogueBox::setLines(const std::vector<std::string>& lines) {
    // F0 (auditoria UI): cada linea se ENVUELVE al ancho util del panel
    // antes de entrar — antes solo se limitaba el NUMERO de lineas y las
    // largas se salian del cuadro por la derecha. Despues, solo las
    // ultimas maxLines() (si "lines" trae menos, se quedan todas).
    std::vector<std::string> wrapped;
    const int usable = static_cast<int>(m_transform.size.x - 2.0f * kTextPadding);
    for (const std::string& line : lines) {
        if (usable > 0 && m_font != nullptr) {
            const auto parts = BitmapFont::wrapText(line, usable, m_font->glyphWidth());
            wrapped.insert(wrapped.end(), parts.begin(), parts.end());
        } else {
            wrapped.push_back(line);
        }
    }
    std::size_t count = std::min(wrapped.size(), m_maxLines);
    m_visibleLines.assign(wrapped.end() - static_cast<std::ptrdiff_t>(count), wrapped.end());
}

void HudDialogueBox::render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const {
    Vector2 topLeft = m_transform.resolveTopLeft(viewportWidth, viewportHeight);

    // Fondo: mismo mecanismo que HudPanel (ver HudWidgets.cpp).
    batch.submit(topLeft, m_transform.size, kFullUV, m_whiteTexture, m_backgroundColor);

    float lineHeight = static_cast<float>(m_font->lineHeight());
    Vector2 textOrigin{topLeft.x + kTextPadding, topLeft.y + kTextPadding};
    for (std::size_t i = 0; i < m_visibleLines.size(); ++i) {
        Vector2 linePos{textOrigin.x, textOrigin.y + lineHeight * static_cast<float>(i)};
        m_font->drawText(batch, m_visibleLines[i], linePos, m_textColor);
    }
}
