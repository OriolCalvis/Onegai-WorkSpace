#include "Render/OcclusionRules.h"

#include <cstdlib>
#include <iostream>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "Fallo: " << message << "\n";
        std::exit(1);
    }
}

}  // namespace

int main() {
    const GridCoord player{4, 4};

    require(OcclusionRules::shouldFadeTile(player, GridCoord{4, 5}, true),
            "un muro inmediatamente delante debe atenuarse");
    require(OcclusionRules::shouldFadeTile(player, GridCoord{5, 5}, true),
            "un muro diagonal cercano debe atenuarse");
    require(!OcclusionRules::shouldFadeTile(player, GridCoord{4, 3}, true),
            "un muro detras no debe atenuarse");
    require(!OcclusionRules::shouldFadeTile(player, GridCoord{4, 7}, true),
            "un muro lejano no debe atenuarse");
    require(!OcclusionRules::shouldFadeTile(player, GridCoord{4, 5}, false),
            "un tile de suelo no debe atenuarse");

    std::cout << "[Occlusion] ventana de visibilidad verificada\n";
    return 0;
}
