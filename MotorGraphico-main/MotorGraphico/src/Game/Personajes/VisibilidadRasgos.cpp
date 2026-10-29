// Mide lo que se ve de verdad; un PNG cargado no demuestra que se vea el complemento.
#include "Game/Personajes/VisibilidadRasgos.h"
#include <array>
#include <algorithm>
#include <cmath>
namespace Game {
std::vector<std::string> VisibilidadRasgos::avisos(const Apariencia& a, const MapaDeCapas& mapa, int frames) {
    if (frames <= 0) return {};
    std::vector<std::array<int, 3>> cuenta(static_cast<std::size_t>(frames));
    const std::array<std::string, 3> ranuras{"alas", "cola", "cuernos"};
    const std::array<std::string, 3> nombres{"Alas", "Cola", "Cuernos"};
    const int ancho = frames * 64;
    for (std::size_t p = 0; p < mapa.arriba.size(); ++p) {
        const auto capa = mapa.arriba[p];
        if (!capa || capa > mapa.ranuras.size()) continue;
        for (std::size_t r = 0; r < ranuras.size(); ++r)
            if (mapa.ranuras[capa - 1] == ranuras[r]) ++cuenta[(p % ancho) / 64][r];
    }
    std::vector<std::string> out(static_cast<std::size_t>(frames));
    for (int f = 0; f < frames; ++f) {
        for (std::size_t r = 0; r < ranuras.size(); ++r) {
            const auto it = a.ropa.find(ranuras[r]);
            if (it == a.ropa.end() || it->second.empty() || cuenta[f][r] != 0) continue;
            if (!out[f].empty()) out[f] += "; ";
            out[f] += nombres[r] + (r == 1 ? " no visible en esta vista" : " no visibles en esta vista");
        }
    }
    return out;
}
int VisibilidadRasgos::frame(double segundos, Rumbo rumbo, Reparto reparto) {
    const bool andar = std::fmod(std::max(0.0, segundos), 4.0) < 2.0;
    const auto ciclo = andar ? cicloDeAndar(rumbo, reparto) : framesDeReposo(rumbo, reparto);
    if (ciclo.empty()) return frameQuieto(rumbo, reparto);
    return ciclo[static_cast<std::size_t>(std::max(0.0, segundos) * 6.0) % ciclo.size()];
}
}
