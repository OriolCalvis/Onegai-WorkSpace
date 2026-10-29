// PERGAMINO-01 — ver include/HUD/Pergamino/Pincel.h.
#include "HUD/Pergamino/Pincel.h"

#include <algorithm>
#include <cmath>

namespace Pergamino {

namespace {
// Las voces de la interfaz se leen con su color; el atlas es blanco.
void vuelca(Lienzo& l, const FuenteAtlas& f, const std::vector<GlifoColocado>& glifos,
            const Vector4& color) {
    const float aw = static_cast<float>(f.anchoAtlas());
    const float ah = static_cast<float>(f.altoAtlas());
    for (const GlifoColocado& g : glifos)
        l.rect(Fuente::Letras, g.destino,
               Rect{g.origen.x / aw, g.origen.y / ah, g.origen.w / aw, g.origen.h / ah}, color);
}
}  // namespace

float Pincel::texto(Lienzo& l, Letra letra, const std::string& s, float x, float yCaja,
                    float altoCaja, const Vector4& color) const {
    if (s.empty()) return 0.0f;
    const int asc = m_letras.ascenso(letra), desc = m_letras.descenso(letra);
    const float base =
        yCaja + std::round((altoCaja - static_cast<float>(asc + desc)) * 0.5f) + static_cast<float>(asc);
    std::vector<GlifoColocado> glifos;
    const float ancho = m_letras.coloca(letra, s, x, base, 1, glifos);
    vuelca(l, m_letras, glifos, color);
    return ancho;
}

float Pincel::pixel(Lienzo& l, const std::string& s, float x, float yCaja, int escala,
                    const Vector4& color) const {
    if (s.empty()) return 0.0f;
    std::vector<GlifoColocado> glifos;
    const float ancho = m_letras.coloca(Letra::Pixel, s, std::round(x),
                                        std::round(yCaja) + static_cast<float>(9 * escala), escala,
                                        glifos);
    vuelca(l, m_letras, glifos, color);
    return ancho;
}

float Pincel::parrafo(Lienzo& l, Letra letra, const std::string& s, float x, float y,
                      float anchoMax, float altoLinea, const Vector4& color, int maxLineas) const {
    std::vector<std::string> lineas = m_letras.envuelve(letra, s, anchoMax);
    if (maxLineas > 0 && static_cast<int>(lineas.size()) > maxLineas) {
        // La ultima visible absorbe la siguiente: asi no cabe y recorta()
        // le pone los puntos suspensivos que dicen «hay mas».
        const std::size_t ultima = static_cast<std::size_t>(maxLineas - 1);
        lineas[ultima] = m_letras.recorta(letra, lineas[ultima] + " " + lineas[ultima + 1], anchoMax);
        lineas.resize(static_cast<std::size_t>(maxLineas));
    }
    for (std::size_t i = 0; i < lineas.size(); ++i)
        texto(l, letra, lineas[i], x, y + altoLinea * static_cast<float>(i), altoLinea, color);
    return altoLinea * static_cast<float>(lineas.size());
}

void Pincel::icono(Lienzo& l, const std::string& nombre, float x, float y, int escala,
                   const Vector4& tinte) const {
    iconoFrame(l, m_iconos.frame(nombre), x, y, escala, tinte);
}

void Pincel::iconoFrame(Lienzo& l, int frame, float x, float y, int escala,
                        const Vector4& tinte) const {
    const int cols = m_iconos.columnas(), filas = m_iconos.filas();
    if (frame <= 0 || cols <= 0 || filas <= 0) return;
    const int col = (frame - 1) % cols, fila = (frame - 1) / cols;
    const float lado = static_cast<float>(m_iconos.lado() * escala);
    l.rect(Fuente::Iconos, Rect{std::round(x), std::round(y), lado, lado},
           Rect{static_cast<float>(col) / cols, static_cast<float>(fila) / filas, 1.0f / cols,
                1.0f / filas},
           tinte);
}

float Pincel::anchoTecla(const std::string& etiqueta) const {
    return std::max(Medida::kTecla, anchoPixel(etiqueta, 2) + 12.0f);
}

float Pincel::tecla(Lienzo& l, const std::string& etiqueta, float x, float y) const {
    const float w = anchoTecla(etiqueta);
    l.color(Rect{x - 2.0f, y - 2.0f, w + 4.0f, Medida::kTecla + 4.0f}, Color::kLinea);
    l.color(Rect{x, y, w, Medida::kTecla}, Color::kPapelLuz);
    l.color(Rect{x, y + Medida::kTecla - 3.0f, w, 3.0f}, Color::kPapelMancha);
    pixel(l, etiqueta, x + std::round((w - anchoPixel(etiqueta, 2)) * 0.5f), y + 5.0f, 2,
          Color::kTinta);
    return w;
}

float Pincel::anchoAyuda(const std::vector<std::string>& teclas, const std::string& verbo) const {
    float w = 0.0f;
    for (std::size_t i = 0; i < teclas.size(); ++i) w += anchoTecla(teclas[i]) + (i > 0 ? 12.0f : 0.0f);
    if (!verbo.empty()) w += 8.0f + static_cast<float>(m_letras.ancho(Letra::Lora18, verbo));
    return w;
}

float Pincel::ayuda(Lienzo& l, const std::vector<std::string>& teclas, const std::string& verbo,
                    float x, float y, const Vector4& colorVerbo) const {
    float cursor = x;
    for (std::size_t i = 0; i < teclas.size(); ++i) {
        if (i > 0) cursor += 12.0f;  // gap de la ayuda + margen entre teclas
        cursor += tecla(l, teclas[i], cursor, y);
    }
    if (!verbo.empty()) {
        cursor += 8.0f;
        cursor += texto(l, Letra::Lora18, verbo, cursor, y + 1.0f, 26.0f, colorVerbo);
    }
    return cursor - x;
}

float Pincel::insignia(Lienzo& l, const std::string& etiqueta, float x, float y,
                       const Vector4& color, const Vector4& fondo) const {
    const float w = anchoInsignia(etiqueta);
    if (fondo.w > 0.0f) l.color(Rect{x, y, w, 26.0f}, fondo);
    contorno(l, Rect{x, y, w, 26.0f}, 2.0f, color);
    pixel(l, etiqueta, x + 8.0f, y + 4.0f, 2, color);
    return w;
}

void Pincel::barra(Lienzo& l, const Rect& r, float fraccion, const Vector4& relleno) const {
    const float f = std::clamp(fraccion, 0.0f, 1.0f);
    l.color(Rect{r.x - 2.0f, r.y - 2.0f, r.w + 4.0f, r.h + 4.0f}, Color::kLinea);
    l.color(r, Color::kPapelLuz);
    const float w = std::round(r.w * f);
    if (w <= 0.0f) return;
    l.color(Rect{r.x, r.y, w, r.h}, relleno);
    l.color(Rect{r.x, r.y, w, 3.0f}, Vector4{1.0f, 1.0f, 1.0f, 0.22f});
}

void Pincel::filaFoco(Lienzo& l, const Rect& fila) const {
    l.color(fila, Color::kLavadoFoco);
    l.color(Rect{fila.x, fila.abajo() - 2.0f, fila.w, 2.0f}, Color::kSubrayadoFoco);
    l.material(pieza(Material::Cinta), Vector2{fila.x + 2.0f, std::round(fila.y + (fila.h - 28.0f) * 0.5f)});
}

void Pincel::contorno(Lienzo& l, const Rect& r, float g, const Vector4& color) const {
    l.color(Rect{r.x, r.y, r.w, g}, color);
    l.color(Rect{r.x, r.abajo() - g, r.w, g}, color);
    l.color(Rect{r.x, r.y + g, g, r.h - 2.0f * g}, color);
    l.color(Rect{r.derecha() - g, r.y + g, g, r.h - 2.0f * g}, color);
}

void Pincel::punteadoV(Lienzo& l, float x, float y0, float y1, const Vector4& color) const {
    for (float y = y0; y < y1; y += 8.0f) l.color(Rect{x, y, 2.0f, std::min(4.0f, y1 - y)}, color);
}

void Pincel::punteadoH(Lienzo& l, float x0, float x1, float y, const Vector4& color) const {
    for (float x = x0; x < x1; x += 8.0f) l.color(Rect{x, y, std::min(4.0f, x1 - x), 2.0f}, color);
}

void Pincel::tituloBloque(Lienzo& l, const std::string& etiqueta, float x, float y,
                          float ancho) const {
    const float w = pixel(l, etiqueta, x, y, 2, Color::kTinta3);
    Vector4 trazo = Color::kAdorno;
    trazo.w = 0.75f;
    punteadoH(l, x + w + 8.0f, x + ancho, y + 10.0f, trazo);
}

void Pincel::tituloConFiletes(Lienzo& l, const std::string& etiqueta, float x, float y,
                              float ancho, int anchoFilete) const {
    const float wt = anchoPixel(etiqueta, 2);
    const float total = wt + 2.0f * (static_cast<float>(anchoFilete) + 16.0f);
    const float x0 = std::round(x + (ancho - total) * 0.5f);
    l.material(adorno(anchoFilete), Vector2{x0, y + 4.0f});
    pixel(l, etiqueta, x0 + static_cast<float>(anchoFilete) + 16.0f, y, 2, Color::kTinta2);
    l.material(adorno(anchoFilete), Vector2{x0 + static_cast<float>(anchoFilete) + 32.0f + wt, y + 4.0f});
}

}  // namespace Pergamino
