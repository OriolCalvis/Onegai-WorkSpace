#include "Check.h"
#include "Level/LevelLoader.h"
#include "Render/TileMap.h"

#include <array>
#include <iostream>

int main() {
    const std::array<const char*, 3> levels = {
        "assets/levels/es_taurengrad.json",
        "assets/levels/es_dhin_thyraxion.json",
        "assets/levels/es_naka_tol.json",
    };

    for (const char* path : levels) {
        auto levelResult = LevelLoader::loadFromFile(path);
        require(levelResult.isOk());
        const LevelDefinition& level = levelResult.value();
        require(!level.name.empty());
        require(!level.mapPath.empty());
        require(!level.objects.empty());

        TileMap map;
        auto mapResult = map.loadFromFile(level.mapPath);
        require(mapResult.isOk());
        require(map.getWidth() >= 48);
        require(map.getHeight() >= 48);
        std::cout << "[ESTE_SUR] " << level.name << ": "
                  << map.getWidth() << "x" << map.getHeight() << ", "
                  << level.objects.size() << " objetos\n";
    }

    std::cout << "[ESTE_SUR] 3 prototipos cargados correctamente.\n";
    return 0;
}
