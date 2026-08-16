#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "Editor/EditorState.h"
#include "Level/ObjectCatalog.h"

// Resultado de validar un nivel antes de guardarlo o abrir el playtest.
// Los errores impiden considerar el nivel listo; los avisos no bloquean
// pero muestran contenido que probablemente necesita una decisión del autor.
struct EditorValidationResult {
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    int walkableTiles = 0;
    int reachableTiles = 0;
    int unreachableTiles = 0;

    bool ok() const { return errors.empty(); }
};

class EditorValidation {
public:
    // Un tile con gid 0 está vacío y no es transitable. collisionGids
    // contiene los GIDs que bloquean movimiento en el tileset activo.
    // Este método es GL-free para que la validación del editor y la de CI
    // compartan exactamente las mismas reglas.
    static EditorValidationResult check(const EditorState& editor, const ObjectCatalog& catalog,
                                        const std::vector<int>& collisionGids) {
        EditorValidationResult result;
        const int width = editor.width();
        const int height = editor.height();
        const GridCoord start = editor.playerStart();

        auto isCollision = [&](int gid) {
            return std::find(collisionGids.begin(), collisionGids.end(), gid) !=
                   collisionGids.end();
        };
        auto isWalkable = [&](int x, int y) {
            const int gid = editor.tileAt(x, y);
            return gid != 0 && !isCollision(gid);
        };

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                result.walkableTiles += isWalkable(x, y) ? 1 : 0;
            }
        }

        if (!isWalkable(start.x, start.y)) {
            result.errors.push_back("el inicio del jugador no esta en un tile transitable");
        } else {
            std::vector<bool> visited(static_cast<std::size_t>(width) * height, false);
            std::vector<GridCoord> queue;
            queue.push_back(start);
            visited[static_cast<std::size_t>(start.y) * width + start.x] = true;

            for (std::size_t cursor = 0; cursor < queue.size(); ++cursor) {
                const GridCoord current = queue[cursor];
                ++result.reachableTiles;
                constexpr int kDx[] = {1, -1, 0, 0};
                constexpr int kDy[] = {0, 0, 1, -1};
                for (int direction = 0; direction < 4; ++direction) {
                    const int nx = current.x + kDx[direction];
                    const int ny = current.y + kDy[direction];
                    if (nx < 0 || nx >= width || ny < 0 || ny >= height || !isWalkable(nx, ny)) {
                        continue;
                    }
                    const std::size_t index = static_cast<std::size_t>(ny) * width + nx;
                    if (!visited[index]) {
                        visited[index] = true;
                        queue.push_back(GridCoord{nx, ny});
                    }
                }
            }
            result.unreachableTiles = result.walkableTiles - result.reachableTiles;
            if (result.unreachableTiles > 0) {
                result.warnings.push_back(std::to_string(result.unreachableTiles) +
                                          " tiles transitables no se alcanzan desde el inicio");
            }
        }

        for (const ObjectSpawn& spawn : editor.objects()) {
            if (catalog.find(spawn.objectId) == nullptr) {
                result.errors.push_back("objeto sin definir: " + spawn.objectId + " en " +
                                        std::to_string(spawn.position.x) + "," +
                                        std::to_string(spawn.position.y));
            }
        }
        if (result.walkableTiles == 0) {
            result.errors.push_back("el mapa no tiene tiles transitables");
        }
        return result;
    }
};
