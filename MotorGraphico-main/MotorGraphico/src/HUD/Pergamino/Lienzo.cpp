// PERGAMINO-01 — ver include/HUD/Pergamino/Lienzo.h.
#include "HUD/Pergamino/Lienzo.h"

namespace Pergamino {

void Lienzo::limpia() {
    m_trazos.clear();
    m_materiales.clear();
    m_zonas.clear();
}

void Lienzo::desplaza(std::size_t desdeTrazo, std::size_t desdeZona, float dx, float dy) {
    for (std::size_t i = desdeTrazo; i < m_trazos.size(); ++i)
        for (Vector2& v : m_trazos[i].p) v = Vector2{v.x + dx, v.y + dy};
    for (std::size_t i = desdeZona; i < m_zonas.size(); ++i) {
        m_zonas[i].rect.x += dx;
        m_zonas[i].rect.y += dy;
    }
}

const Zona* Lienzo::zonaEn(float x, float y) const {
    for (auto it = m_zonas.rbegin(); it != m_zonas.rend(); ++it)
        if (it->contiene(x, y)) return &*it;
    return nullptr;
}

void Lienzo::rect(Fuente fuente, const Rect& d, const Rect& uv, const Vector4& color,
                  int material) {
    Trazo t;
    t.fuente = fuente;
    t.material = material;
    t.p[0] = {d.x, d.y};
    t.p[1] = {d.x + d.w, d.y};
    t.p[2] = {d.x + d.w, d.y + d.h};
    t.p[3] = {d.x, d.y + d.h};
    t.uv[0] = {uv.x, uv.y};
    t.uv[1] = {uv.x + uv.w, uv.y};
    t.uv[2] = {uv.x + uv.w, uv.y + uv.h};
    t.uv[3] = {uv.x, uv.y + uv.h};
    t.color = color;
    m_trazos.push_back(t);
}

void Lienzo::color(const Rect& destino, const Vector4& c) {
    if (destino.w <= 0.0f || destino.h <= 0.0f || c.w <= 0.0f) return;
    rect(Fuente::Blanco, destino, Rect{0.0f, 0.0f, 1.0f, 1.0f}, c);
}

void Lienzo::atenua(std::size_t desde, float factor) {
    for (std::size_t i = desde; i < m_trazos.size(); ++i) m_trazos[i].color.w *= factor;
}

int Lienzo::pide(const PedidoMaterial& pedido) {
    for (std::size_t i = 0; i < m_materiales.size(); ++i)
        if (m_materiales[i] == pedido) return static_cast<int>(i);
    m_materiales.push_back(pedido);
    return static_cast<int>(m_materiales.size() - 1);
}

void Lienzo::material(const PedidoMaterial& pedido, const Vector2& pos, const Vector4& tinte) {
    if (pedido.ancho <= 0 || pedido.alto <= 0) return;
    const int indice = pide(pedido);
    rect(Fuente::Material,
         Rect{pos.x, pos.y, static_cast<float>(pedido.ancho), static_cast<float>(pedido.alto)},
         Rect{0.0f, 0.0f, 1.0f, 1.0f}, tinte, indice);
}

void Lienzo::materialConSombra(const PedidoMaterial& pedido, const Vector2& pos,
                               const Vector4& sombra) {
    // La sombra es el mismo recorte tenido de tinta: respeta el canto
    // rasgado sin necesitar una textura propia.
    material(pedido, Vector2{pos.x, pos.y + 4.0f}, sombra);
    material(pedido, pos);
}

}  // namespace Pergamino
