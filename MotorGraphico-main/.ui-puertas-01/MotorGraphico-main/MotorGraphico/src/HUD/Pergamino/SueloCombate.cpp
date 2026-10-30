// AV13-10 — Ver include/HUD/Pergamino/SueloCombate.h.
#include "HUD/Pergamino/SueloCombate.h"

namespace Pergamino {

void SueloCombate::pon(GridCoord o, int w, int h, const std::function<bool(int x, int y)>& sePisa) {
    origen = o;
    ancho = w;
    alto = h;
    pisable.assign(static_cast<std::size_t>(w > 0 && h > 0 ? w * h : 0), false);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            pisable[static_cast<std::size_t>(y * w + x)] = sePisa && sePisa(o.x + x, o.y + y);
    visible = true;
}

bool SueloCombate::esPisable(int x, int y) const {
    const int lx = x - origen.x, ly = y - origen.y;
    if (lx < 0 || ly < 0 || lx >= ancho || ly >= alto) return false;
    return pisable[static_cast<std::size_t>(ly * ancho + lx)];
}

std::vector<AristaSuelo> SueloCombate::aristas() const {
    std::vector<AristaSuelo> out;
    for (int y = origen.y; y < origen.y + alto; ++y) {
        for (int x = origen.x; x < origen.x + ancho; ++x) {
            if (!esPisable(x, y)) continue;
            // Hacia donde da cada lado del rombo y sus dos vertices.
            if (y == origen.y) out.push_back({{x, y}, {x + 1, y}});                         // arriba-derecha
            if (x == origen.x + ancho - 1) out.push_back({{x + 1, y}, {x + 1, y + 1}});      // abajo-derecha
            if (y == origen.y + alto - 1) out.push_back({{x + 1, y + 1}, {x, y + 1}});       // abajo-izquierda
            if (x == origen.x) out.push_back({{x, y + 1}, {x, y}});                         // arriba-izquierda
        }
    }
    return out;
}

}  // namespace Pergamino
