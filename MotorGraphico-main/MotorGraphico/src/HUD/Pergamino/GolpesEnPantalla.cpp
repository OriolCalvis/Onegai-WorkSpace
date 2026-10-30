// COMBATE-01 — Ver include/HUD/Pergamino/GolpesEnPantalla.h.
#include "HUD/Pergamino/GolpesEnPantalla.h"

#include <algorithm>
#include <cmath>

#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"

namespace Pergamino {

void GolpesEnPantalla::lanza(const Vector2& mundo, const std::string& texto, Tipo tipo) {
    // Dos golpes seguidos sobre el mismo no se pisan: el nuevo, un poco más arriba.
    Vector2 donde = mundo;
    for (const Golpe& g : m_golpes)
        if (g.edad < 0.35f && std::abs(g.mundo.x - donde.x) < 18.0f && std::abs(g.mundo.y - donde.y) < 14.0f)
            donde.y -= 14.0f;
    m_golpes.push_back(Golpe{donde, texto, tipo, 0.0f});
}

void GolpesEnPantalla::avanza(float segundos) {
    for (Golpe& g : m_golpes) g.edad += segundos;
    m_golpes.erase(std::remove_if(m_golpes.begin(), m_golpes.end(), [](const Golpe& g) { return g.edad >= kVida; }),
                   m_golpes.end());
}

void GolpesEnPantalla::olvida() {
    m_golpes.clear();
}

void GolpesEnPantalla::pinta(const Pincel& p, int anchoVista, int altoVista) {
    m_lienzo.limpia();
    for (const Golpe& g : m_golpes) {
        const float t = g.edad / kVida;
        // Salta (los primeros instantes, mas grande) y sube despacio; se apaga al final.
        const int grande = g.tipo == Tipo::Critico ? 5 : 4;
        const int escala = g.tipo == Tipo::Nada ? 2 : (t < 0.15f ? grande : grande - 1);
        const float alfa = t < 0.7f ? 1.0f : std::max(0.0f, 1.0f - (t - 0.7f) / 0.3f);
        Vector4 color = g.tipo == Tipo::Dano      ? rgb(0xE0, 0x3A, 0x22)
                        : g.tipo == Tipo::Critico ? rgb(0xF0, 0xB8, 0x3A)
                        : g.tipo == Tipo::Cura    ? rgb(0x5C, 0xC0, 0x5A)
                                                  : rgb(0xF2, 0xE4, 0xC1);
        color.w = alfa;
        Vector4 borde = rgb(0x1C, 0x12, 0x09, alfa);
        const float ancho = p.anchoPixel(g.texto, escala);
        const Vector2 pantalla{(g.mundo.x - m_camara.x) * m_zoom + static_cast<float>(anchoVista) * 0.5f,
                               (g.mundo.y - m_camara.y) * m_zoom + static_cast<float>(altoVista) * 0.5f};
        const float x = std::round(pantalla.x - ancho * 0.5f);
        const float y = std::round(pantalla.y - 40.0f * t - 9.0f * static_cast<float>(escala));
        for (const Vector2& o : {Vector2{-2, 0}, Vector2{2, 0}, Vector2{0, -2}, Vector2{0, 2}})
            p.pixel(m_lienzo, g.texto, x + o.x, y + o.y, escala, borde);
        p.pixel(m_lienzo, g.texto, x, y, escala, color);
    }
}

}  // namespace Pergamino
