// MAPA-02 — ver include/HUD/Pergamino/GuiaMapa.h. El mapa grande se pinta en
// MapaGrande.cpp.
#include "HUD/Pergamino/GuiaMapa.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "Core/Math/IsoMath.h"
#include "Game/DiarioMisiones.h"
#include "Game/GameSession.h"
#include "Game/RumboNiveles.h"
#include "HUD/Pergamino/HudRetales.h"
#include "HUD/Pergamino/PaginaMisiones.h"
#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"

namespace Pergamino {

namespace {

constexpr float kMargenFlecha = 24.0f;  // la punta, a esto del borde (area segura)
constexpr float kApartaHud = 64.0f;     // la flecha se queda a esto de un retal

bool dentro(const Rect& r, Vector2 v) { return v.x >= r.x && v.y >= r.y && v.x <= r.derecha() && v.y <= r.abajo(); }

Rect crece(const Rect& r, float m) { return Rect{r.x - m, r.y - m, r.w + 2.0f * m, r.h + 2.0f * m}; }

bool solapan(const Rect& a, const Rect& b) {
    return a.x < b.derecha() && b.x < a.derecha() && a.y < b.abajo() && b.y < a.abajo();
}

// Los retales del HUD que ya hay en el lienzo (desde `desde`): cada material
// grande (los retales son hojas; las chinchetas y flechitas de 12 px no
// cuentan).
std::vector<Rect> retalesDe(const Lienzo& l, std::size_t desde) {
    std::vector<Rect> r;
    const std::vector<Trazo>& trazos = l.trazos();
    for (std::size_t i = desde; i < trazos.size(); ++i) {
        const Trazo& t = trazos[i];
        if (t.fuente != Fuente::Material) continue;
        float x0 = t.p[0].x, y0 = t.p[0].y, x1 = x0, y1 = y0;
        for (const Vector2& v : t.p) x0 = std::min(x0, v.x), y0 = std::min(y0, v.y), x1 = std::max(x1, v.x), y1 = std::max(y1, v.y);
        if ((x1 - x0) * (y1 - y0) >= 1200.0f) r.push_back(Rect{x0, y0, x1 - x0, y1 - y0});
    }
    return r;
}

std::string pasos(int n) { return std::to_string(n) + (n == 1 ? " PASO" : " PASOS"); }

}  // namespace

Flecha calculaFlecha(Vector2 sitio, float ancho, float alto, float margen, const std::vector<Rect>& retales) {
    // integra (15/10): los retales que se tocan (el del personaje y el del objetivo, uno encima del
    // otro) cuentan como uno; si no, la punta rebotaba de uno a otro y se quedaba encima de ellos.
    std::vector<Rect> tapados = retales;
    for (bool junta = true; junta;) {
        junta = false;
        for (std::size_t i = 0; i < tapados.size() && !junta; ++i)
            for (std::size_t j = i + 1; j < tapados.size() && !junta; ++j)
                if (solapan(tapados[i], tapados[j])) {
                    const Rect &a = tapados[i], &b = tapados[j];
                    const float x0 = std::min(a.x, b.x), y0 = std::min(a.y, b.y);
                    tapados[i] = Rect{x0, y0, std::max(a.derecha(), b.derecha()) - x0, std::max(a.abajo(), b.abajo()) - y0};
                    tapados.erase(tapados.begin() + static_cast<std::ptrdiff_t>(j));
                    junta = true;
                }
    }
    Flecha f;
    f.hay = true;
    const Rect borde{margen, margen, ancho - 2.0f * margen, alto - 2.0f * margen};
    const Vector2 centro{ancho * 0.5f, alto * 0.5f};
    Vector2 d{sitio.x - centro.x, sitio.y - centro.y};
    const float largo = std::sqrt(d.x * d.x + d.y * d.y);
    f.rumbo = largo > 0.001f ? Vector2{d.x / largo, d.y / largo} : Vector2{0.0f, -1.0f};
    if (dentro(borde, sitio)) {
        f.enPantalla = true;
        f.punto = sitio;
        return f;
    }
    // La recta desde el centro, hasta el borde.
    float t = 1e9f;
    if (std::fabs(f.rumbo.x) > 1e-4f) t = std::min(t, ((f.rumbo.x > 0 ? borde.derecha() : borde.x) - centro.x) / f.rumbo.x);
    if (std::fabs(f.rumbo.y) > 1e-4f) t = std::min(t, ((f.rumbo.y > 0 ? borde.abajo() : borde.y) - centro.y) / f.rumbo.y);
    Vector2 p{centro.x + f.rumbo.x * t, centro.y + f.rumbo.y * t};
    // Si cae sobre un retal, se desliza por el borde hasta salir de el (al
    // lado mas cercano que siga dentro de la vista).
    for (int vuelta = 0; vuelta < 4; ++vuelta) {
        bool movida = false;
        for (const Rect& r : tapados) {
            if (!dentro(r, p)) continue;
            const bool horizontal = std::fabs(p.y - borde.y) < 0.5f || std::fabs(p.y - borde.abajo()) < 0.5f;
            if (horizontal) {
                const float a = r.x - 1.0f, b = r.derecha() + 1.0f;
                const bool vaA = a >= borde.x && (b > borde.derecha() || std::fabs(p.x - a) <= std::fabs(p.x - b));
                p.x = vaA ? a : b;
            } else {
                const float a = r.y - 1.0f, b = r.abajo() + 1.0f;
                const bool vaA = a >= borde.y && (b > borde.abajo() || std::fabs(p.y - a) <= std::fabs(p.y - b));
                p.y = vaA ? a : b;
            }
            p.x = std::clamp(p.x, borde.x, borde.derecha());
            p.y = std::clamp(p.y, borde.y, borde.abajo());
            movida = true;
        }
        if (!movida) break;
    }
    f.punto = p;
    d = Vector2{sitio.x - p.x, sitio.y - p.y};
    const float l2 = std::sqrt(d.x * d.x + d.y * d.y);
    if (l2 > 0.001f) f.rumbo = Vector2{d.x / l2, d.y / l2};
    return f;
}

void GuiaMapa::ponNivel(const std::string& clave) {
    const std::string nivel = Game::RumboNiveles::nombre(clave);
    if (nivel != m_nivel) m_mapa.abierto = false;
    m_nivel = nivel;
}

const std::vector<Game::Chincheta>& GuiaMapa::chinchetasAqui() const { return m_marcas.de(m_nivel); }

void GuiaMapa::miraDiario(const Game::DiarioDatos& diario) {
    m_filas.clear();
    for (const FilaDiario& f : filasDelDiario(diario)) {
        const Game::DiarioMision& m = diario.misiones[f.mision];
        m_filas.push_back(f.tipo == FilaDiario::Tipo::Mision && Game::sePuedeSeguir(m) ? m.id : std::string());
    }
}

bool GuiaMapa::sigueFila(std::size_t fila) {
    if (fila >= m_filas.size() || m_filas[fila].empty()) return false;
    m_marcas.misionSeguida = m_filas[fila];
    return true;
}

void GuiaMapa::abre(GridCoord jugador) {
    m_mapa.abierto = true;
    m_mapa.cursor = jugador;
    m_mapa.aviso.clear();
    m_mapa.repite = m_tiempo + 0.3f;
}

void GuiaMapa::ponCursor(GridCoord celda) {
    const int w = std::max(1, m_minimapa.ancho()), h = std::max(1, m_minimapa.alto());
    m_mapa.cursor = GridCoord{std::clamp(celda.x, 0, w - 1), std::clamp(celda.y, 0, h - 1)};
}

void GuiaMapa::mueveCursor(int dx, int dy, bool nuevo) {
    if (dx == 0 && dy == 0) return;
    if (!nuevo && m_tiempo < m_mapa.repite) return;
    m_mapa.repite = m_tiempo + (nuevo ? 0.3f : 0.05f);
    ponCursor(GridCoord{m_mapa.cursor.x + dx, m_mapa.cursor.y + dy});
}

bool GuiaMapa::celdaBajo(float x, float y, GridCoord& celda) const {
    const Rect& r = m_mapa.terreno;
    if (!m_mapa.abierto || r.w <= 0.0f || m_minimapa.ancho() <= 0 || !dentro(r, Vector2{x, y})) return false;
    const float paso = r.w / static_cast<float>(m_minimapa.ancho());
    celda = GridCoord{std::min(m_minimapa.ancho() - 1, static_cast<int>((x - r.x) / paso)),
                      std::min(m_minimapa.alto() - 1, static_cast<int>((y - r.y) / paso))};
    return true;
}

Game::MarcasMapa::Cambio GuiaMapa::alternaChincheta() {
    const Game::MarcasMapa::Cambio c = m_marcas.alterna(m_nivel, m_mapa.cursor.x, m_mapa.cursor.y);
    m_mapa.aviso = c == Game::MarcasMapa::Cambio::Llena ? "Ya hay cinco chinchetas aquí: quita una con ENTER encima."
                                                         : std::string();
    return c;
}

void GuiaMapa::pintaFlecha(Lienzo& l, const Pincel& p, const GameSession& sesion, const Game::DiarioDatos* diario,
                           const Vista& v, std::size_t desdeHud) const {
    // El sitio: el del objetivo mas cercano; sin objetivo aqui, la chincheta mas cercana.
    const GridCoord yo = sesion.playerPosition();
    auto distancia = [&](GridCoord c) { return std::abs(c.x - yo.x) + std::abs(c.y - yo.y); };
    bool hay = false, esChincheta = false;
    GridCoord sitio;
    Game::Chincheta chincheta;
    for (const MinimapaTinta::Sitio& s : m_minimapa.sitiosObjetivo(sesion, diario))
        if (!hay || distancia(s.celda) < distancia(sitio)) hay = true, sitio = s.celda;
    for (const Game::Chincheta& c : hay ? std::vector<Game::Chincheta>{} : chinchetasAqui())
        if (!esChincheta || distancia(GridCoord{c.x, c.y}) < distancia(sitio))
            esChincheta = true, sitio = GridCoord{c.x, c.y}, chincheta = c;
    if (!hay && !esChincheta) return;
    const int faltan = distancia(sitio);
    if (faltan == 0) return;  // ya estas encima
    const float tw = static_cast<float>(v.anchoBaldosa > 0 ? v.anchoBaldosa : 64);
    const float th = static_cast<float>(v.altoBaldosa > 0 ? v.altoBaldosa : 32);
    const Vector2 mundo = IsoMath::gridToScreen(sitio, tw, th);
    const Vector2 pantalla{(mundo.x + tw * 0.5f - v.camaraX) * v.zoom + static_cast<float>(v.ancho) * 0.5f,
                           (mundo.y + th * 0.5f - v.camaraY) * v.zoom + static_cast<float>(v.alto) * 0.5f};
    std::vector<Rect> tapados = retalesDe(l, desdeHud);
    for (Rect& r : tapados) r = crece(r, kApartaHud);
    const Flecha f = calculaFlecha(pantalla, static_cast<float>(v.ancho), static_cast<float>(v.alto), kMargenFlecha, tapados);
    const float vaiven = v.animacionesReducidas ? 0.0f : std::round(3.0f * std::sin(m_tiempo * 4.0f));
    MarcaMinimapa marca;
    marca.tipo = esChincheta ? MarcaMinimapa::Tipo::Chincheta : MarcaMinimapa::Tipo::Objetivo;
    marca.color = chincheta.color, marca.numero = chincheta.numero;
    if (f.enPantalla) {
        // Una marca pequena encima del sitio (sobre la cabeza de quien este ahi),
        // salvo que caiga sobre un retal o sobre el aviso de la E.
        // Encima de la cabeza; si ahi hay un retal, a sus pies.
        const std::vector<Rect> retales = retalesDe(l, desdeHud);
        for (const float alto : {-64.0f * v.zoom, 22.0f * v.zoom}) {
            const Vector2 c{f.punto.x, f.punto.y + alto + vaiven};
            bool libre = true;
            for (const Rect& r : retales) libre = libre && !solapan(r, Rect{c.x - 12.0f, c.y - 12.0f, 24.0f, 24.0f});
            if (!libre) continue;
            pintaMarcaMinimapa(l, p, marca, c.x - 6.0f, c.y - 6.0f, 12.0f);
            return;
        }
        return;
    }
    // Fuera: un retal pequeno con el icono y los pasos, y su punta hacia el sitio.
    const std::string texto = esChincheta ? std::to_string(chincheta.numero) + " · " + pasos(faltan) : pasos(faltan);
    const float w = 4.0f * std::ceil((56.0f + p.anchoPixel(texto, 2)) / 4.0f);
    const float h = 36.0f;
    const Vector2 r = f.rumbo;
    const Vector2 punta{f.punto.x + r.x * vaiven, f.punto.y + r.y * vaiven};
    const float hasta = std::min(std::fabs(r.x) > 1e-3f ? (w * 0.5f) / std::fabs(r.x) : 1e9f,
                                 std::fabs(r.y) > 1e-3f ? (h * 0.5f) / std::fabs(r.y) : 1e9f);
    Vector2 c{punta.x - r.x * (14.0f + hasta), punta.y - r.y * (14.0f + hasta)};
    c.x = std::clamp(c.x, w * 0.5f + 8.0f, static_cast<float>(v.ancho) - w * 0.5f - 8.0f);
    c.y = std::clamp(c.y, h * 0.5f + 8.0f, static_cast<float>(v.alto) - h * 0.5f - 8.0f);
    const float x = std::round(c.x - w * 0.5f), y = std::round(c.y - h * 0.5f);
    l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 21, 1, 0, 0), Vector2{x, y}, Color::kSombra);
    if (esChincheta) pintaMarcaMinimapa(l, p, marca, x + 10.0f, y + 6.0f, 24.0f);
    else p.icono(l, "mision_activa", x + 14.0f, y + 10.0f, 1);
    p.pixel(l, texto, x + 40.0f, y + 10.0f, 2, Color::kTinta);
    // La punta: un triangulo de tinta de la hoja hacia el sitio.
    const Vector2 n{-r.y, r.x};
    const Vector2 base{punta.x - r.x * 14.0f, punta.y - r.y * 14.0f};
    Trazo t;
    t.p[0] = punta;
    t.p[1] = Vector2{base.x + n.x * 9.0f, base.y + n.y * 9.0f};
    t.p[2] = Vector2{base.x - n.x * 9.0f, base.y - n.y * 9.0f};
    t.p[3] = t.p[2];
    t.color = esChincheta ? colorChincheta(chincheta.color) : Color::kCarmin;
    l.trazo(t);
}

}  // namespace Pergamino
