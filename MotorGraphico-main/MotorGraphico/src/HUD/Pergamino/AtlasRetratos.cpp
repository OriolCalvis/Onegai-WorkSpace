// UI-BG3-01 — ver include/HUD/Pergamino/AtlasRetratos.h.
#include "HUD/Pergamino/AtlasRetratos.h"

#include <cstring>

namespace Pergamino {

namespace {
constexpr int kAncho = AtlasRetratos::kLado * AtlasRetratos::kColumnas;
}  // namespace

Rect AtlasRetratos::uvDe(int celda) const {
    const float lado = 1.0f / static_cast<float>(kColumnas);
    return Rect{static_cast<float>(celda % kColumnas) * lado, static_cast<float>(celda / kColumnas) * lado, lado,
                lado};
}

void AtlasRetratos::vacia() {
    m_celdas.clear();
    m_imagen.crea(kAncho, kAncho);
    ++m_version;
}

Rect AtlasRetratos::celda(const std::string& clave, const Fabrica& fabrica) {
    const auto it = m_celdas.find(clave);
    if (it != m_celdas.end()) return uvDe(it->second);
    if (!fabrica) return Rect{};
    Imagen frame;
    frame.crea(kLado, kLado);
    if (!fabrica(frame) || frame.ancho != kLado || frame.alto != kLado) return Rect{};
    if (m_imagen.vacia() || m_celdas.size() >= static_cast<std::size_t>(kCeldas)) vacia();
    const int n = static_cast<int>(m_celdas.size());
    const int x0 = (n % kColumnas) * kLado, y0 = (n / kColumnas) * kLado;
    for (int y = 0; y < kLado; ++y)
        std::memcpy(m_imagen.pixel(x0, y0 + y), frame.pixel(0, y), static_cast<std::size_t>(kLado) * 4);
    m_celdas.emplace(clave, n);
    ++m_version;
    return uvDe(n);
}

void copiaFrame(const std::uint8_t* tira, int anchoTira, int frame, Imagen& destino) {
    const int lado = AtlasRetratos::kLado;
    if (tira == nullptr || frame < 0 || (frame + 1) * lado > anchoTira) return;
    if (destino.ancho != lado || destino.alto != lado) destino.crea(lado, lado);
    for (int y = 0; y < lado; ++y) {
        const std::uint8_t* fila = tira + (static_cast<std::size_t>(y) * static_cast<std::size_t>(anchoTira) +
                                           static_cast<std::size_t>(frame * lado)) * 4;
        std::memcpy(destino.pixel(0, y), fila, static_cast<std::size_t>(lado) * 4);
    }
}

}  // namespace Pergamino
