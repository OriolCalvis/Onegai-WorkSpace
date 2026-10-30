// MISIONES-01 — ver include/Render/HudIconList.h.
#include "Render/HudIconList.h"

#include <algorithm>
#include <utility>

#include "Render/BitmapFont.h"
#include "Render/SpriteBatch.h"

namespace {
constexpr float kHueco = 6.0f;
constexpr float kSangria = 22.0f;
const Vector4 kTitulo{1.0f, 0.86f, 0.45f, 1.0f};
const Vector4 kCursor{1.0f, 0.95f, 0.6f, 1.0f};
const char* const kCursorTexto = ">";
}  // namespace

HudIconList::HudIconList(const HudTransform& transform, BitmapFont* font, Texture* iconos,
                         Hud::Iconos manifiesto)
    : m_transform(transform), m_font(font), m_iconos(iconos, std::move(manifiesto)) {}

void HudIconList::setRows(std::vector<HudIconRow> rows) {
    m_rows = std::move(rows);
    setSelectedIndex(m_selected);
}

void HudIconList::setSelectedIndex(std::size_t index) {
    if (m_rows.empty()) {
        m_selected = 0;
        return;
    }
    index = std::min(index, m_rows.size() - 1);
    // Hacia delante hasta una seleccionable; si no hay, hacia atras.
    std::size_t i = index;
    while (i < m_rows.size() && !m_rows[i].selectable) ++i;
    if (i >= m_rows.size()) {
        i = index;
        while (i > 0 && !m_rows[i].selectable) --i;
    }
    m_selected = i;
}

void HudIconList::moveUp() {
    if (m_rows.empty()) return;
    for (std::size_t paso = 0; paso < m_rows.size(); ++paso) {
        m_selected = (m_selected == 0) ? m_rows.size() - 1 : m_selected - 1;
        if (m_rows[m_selected].selectable) return;
    }
}

void HudIconList::moveDown() {
    if (m_rows.empty()) return;
    for (std::size_t paso = 0; paso < m_rows.size(); ++paso) {
        m_selected = (m_selected + 1) % m_rows.size();
        if (m_rows[m_selected].selectable) return;
    }
}

float HudIconList::rowHeight() const {
    return std::max(static_cast<float>(m_font->lineHeight()), m_iconos.lado() + 4.0f);
}

bool HudIconList::hasDetail() const {
    return std::any_of(m_rows.begin(), m_rows.end(), [](const HudIconRow& r) { return !r.detail.empty(); });
}

std::size_t HudIconList::visibleRows() const {
    float alto = m_transform.size.y - (m_title.empty() ? 0.0f : static_cast<float>(m_font->lineHeight()) + 4.0f);
    if (hasDetail()) alto -= static_cast<float>(m_font->lineHeight()) * (kDetailLines + 0.5f);
    return static_cast<std::size_t>(std::max(1.0f, alto / rowHeight()));
}

void HudIconList::render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const {
    Vector2 pos = m_transform.resolveTopLeft(viewportWidth, viewportHeight);
    if (!m_title.empty()) {
        m_font->drawText(batch, m_title, pos, kTitulo);
        pos.y += static_cast<float>(m_font->lineHeight()) + 4.0f;
    }
    if (m_rows.empty()) return;
    const std::size_t caben = visibleRows();
    // Scroll: la seleccionada siempre dentro de la ventana.
    std::size_t primera = 0;
    if (m_selected >= caben) primera = m_selected - caben + 1;
    primera = std::min(primera, m_rows.size() > caben ? m_rows.size() - caben : 0);

    const float alto = rowHeight();
    const float cursorAncho = static_cast<float>(m_font->glyphWidth()) + kHueco;
    const float dyTexto = std::max(0.0f, (m_iconos.lado() - static_cast<float>(m_font->glyphHeight())) * 0.5f);
    for (std::size_t i = primera; i < m_rows.size() && i < primera + caben; ++i) {
        const HudIconRow& fila = m_rows[i];
        float x = pos.x;
        if (m_showCursor) {
            if (i == m_selected) m_font->drawText(batch, kCursorTexto, Vector2{x, pos.y + dyTexto}, kCursor);
            x += cursorAncho;
        }
        x += kSangria * static_cast<float>(fila.indent);
        m_iconos.dibujar(batch, fila.frame, Vector2{x, pos.y});
        x += m_iconos.lado() + kHueco;
        const int glifos = std::max(1, static_cast<int>((pos.x + m_transform.size.x - x) / m_font->glyphWidth()));
        const std::string texto = BitmapFont::clipText(fila.text, glifos * m_font->glyphWidth(), m_font->glyphWidth());
        m_font->drawText(batch, texto, Vector2{x, pos.y + dyTexto}, fila.color);
        pos.y += alto;
    }
    // Pie: el detalle de la fila seleccionada, envuelto a kDetailLines lineas.
    if (hasDetail() && m_selected < m_rows.size() && !m_rows[m_selected].detail.empty()) {
        const Vector2 origen = m_transform.resolveTopLeft(viewportWidth, viewportHeight);
        const float lineaAlto = static_cast<float>(m_font->lineHeight());
        float y = origen.y + m_transform.size.y - lineaAlto * kDetailLines;
        const auto lineas = BitmapFont::wrapText(m_rows[m_selected].detail,
                                                 static_cast<int>(m_transform.size.x), m_font->glyphWidth());
        for (std::size_t i = 0; i < lineas.size() && i < static_cast<std::size_t>(kDetailLines); ++i) {
            std::string linea = lineas[i];
            if (i + 1 == static_cast<std::size_t>(kDetailLines) && lineas.size() > static_cast<std::size_t>(kDetailLines)) {
                linea = BitmapFont::clipText(linea + " ...", static_cast<int>(m_transform.size.x), m_font->glyphWidth());
            }
            m_font->drawText(batch, linea, Vector2{origen.x, y}, m_rows[m_selected].color);
            y += lineaAlto;
        }
    }
    // Indicadores de que hay mas arriba / abajo.
    if (primera > 0) m_font->drawText(batch, "^", Vector2{pos.x + m_transform.size.x - m_font->glyphWidth(), m_transform.resolveTopLeft(viewportWidth, viewportHeight).y}, kCursor);
    if (primera + caben < m_rows.size()) m_font->drawText(batch, "v", Vector2{pos.x + m_transform.size.x - m_font->glyphWidth(), pos.y - alto}, kCursor);
}
