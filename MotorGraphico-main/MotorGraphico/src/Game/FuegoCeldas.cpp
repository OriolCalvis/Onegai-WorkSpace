// C2-04c — ver include/Game/FuegoCeldas.h.
#include "Game/FuegoCeldas.h"

#include <algorithm>

namespace Game {

std::vector<GridCoord> celdasAEncender(const EscenaViva& escena) {
    std::vector<GridCoord> out;
    out.reserve(escena.fuegos().size());
    for (const auto& c : escena.fuegos()) out.push_back(GridCoord{c.first, c.second});
    std::sort(out.begin(), out.end(), [](const GridCoord& a, const GridCoord& b) {
        return a.y != b.y ? a.y < b.y : a.x < b.x;
    });
    return out;
}

std::uint32_t semillaDe(const GridCoord& celda) {
    // FNV-1a sobre las coordenadas; el +1 evita la semilla 0.
    std::uint32_t h = 2166136261u;
    for (const std::uint32_t v : {static_cast<std::uint32_t>(celda.x), static_cast<std::uint32_t>(celda.y)})
        for (int i = 0; i < 4; ++i) h = (h ^ ((v >> (8 * i)) & 0xFFu)) * 16777619u;
    return h + 1u;
}

}  // namespace Game
