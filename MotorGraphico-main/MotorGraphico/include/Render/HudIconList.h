// MISIONES-01 — Lista del menu del jugador con icono por fila: inventario
// (consumible / equipo / objeto de mision), ranuras de equipo, diario de
// misiones (mision activa/hecha, objetivo pendiente/cumplido) y codice.
//
// Mismo papel que HudCommandMenu (cursor, seleccion, opciones) pero cada
// fila lleva un frame del atlas de iconos, un color y un sangrado; y hace
// scroll para que la fila seleccionada siempre se vea. Quien decide que
// icono lleva cada fila es GL-free (Hud::Iconos + los modelos de Game/).
#pragma once

#include <string>
#include <vector>

#include "Core/Math/Vector4.h"
#include "Render/HudElement.h"
#include "Render/HudIconos.h"

class BitmapFont;
class Texture;

struct HudIconRow {
    int frame = 0;               // 0 = sin icono (se reserva el hueco igual)
    std::string text;
    Vector4 color{0.92f, 0.92f, 0.95f, 1.0f};
    int indent = 0;              // niveles de sangrado (objetivos bajo su mision)
    bool selectable = true;      // una cabecera no se selecciona
    std::string detail;          // texto largo de la fila seleccionada (pie, envuelto)
};

class HudIconList : public IHudElement {
public:
    HudIconList(const HudTransform& transform, BitmapFont* font, Texture* iconos,
                Hud::Iconos manifiesto);

    void setTitle(std::string title) { m_title = std::move(title); }
    void setRows(std::vector<HudIconRow> rows);
    const std::vector<HudIconRow>& rows() const { return m_rows; }
    // Indice dentro de rows(); se acota y se salta a la siguiente fila
    // seleccionable. Con showCursor=false solo sirve para el scroll.
    void setSelectedIndex(std::size_t index);
    std::size_t selectedIndex() const { return m_selected; }
    // Cursor circular saltando las filas no seleccionables (misma API que
    // HudCommandMenu, para que la tienda navegue igual).
    void moveUp();
    void moveDown();
    // Los iconos pueden llegar despues (la tienda se crea antes de cargar
    // la textura de iconos).
    void setIconos(HudIconos iconos) { m_iconos = std::move(iconos); }
    void setShowCursor(bool show) { m_showCursor = show; }
    const HudIconos& iconos() const { return m_iconos; }
    // Filas que caben en el alto del transform (menos el pie de detalle,
    // kDetailLines lineas, si alguna fila lo trae).
    std::size_t visibleRows() const;
    static constexpr int kDetailLines = 3;

    void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const override;

private:
    float rowHeight() const;
    bool hasDetail() const;

    HudTransform m_transform;
    BitmapFont* m_font;
    HudIconos m_iconos;
    std::string m_title;
    std::vector<HudIconRow> m_rows;
    std::size_t m_selected = 0;
    bool m_showCursor = true;
};
