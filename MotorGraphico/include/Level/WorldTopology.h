#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"
#include "Level/LevelLoader.h"

// Grafo de viajes entre niveles. Se construye desde uno o varios niveles
// raíz y sigue las puertas (ObjectSpawn::targetLevel) de forma recursiva.
// Es GL-free: sirve tanto al editor como a una pantalla de mapamundi, a
// los validadores y a los tests de contenido.
struct WorldNode {
    std::string levelPath;
    std::string name;
    std::string mapPath;
};

struct WorldLink {
    std::string sourceLevelPath;
    GridCoord sourcePosition;
    std::string objectId;
    std::string targetLevelPath;
    GridCoord targetPosition;
    bool hasTargetPosition = false;
};

class WorldTopology {
public:
    // maxNodes hace que un manifiesto de contenido corrupto no pueda iniciar
    // una exploración ilimitada. 512 supera holgadamente el mundo actual.
    static Result<WorldTopology> load(const std::vector<std::string>& rootLevels,
                                      std::size_t maxNodes = 512) {
        if (rootLevels.empty()) {
            return Result<WorldTopology>::Error("WorldTopology necesita al menos un nivel raiz");
        }

        WorldTopology topology;
        std::vector<std::string> pending;
        std::unordered_set<std::string> scheduled;
        for (const std::string& root : rootLevels) {
            if (!root.empty() && scheduled.insert(root).second) {
                pending.push_back(root);
            }
        }
        if (pending.empty()) {
            return Result<WorldTopology>::Error("WorldTopology no recibio rutas de nivel validas");
        }

        for (std::size_t cursor = 0; cursor < pending.size(); ++cursor) {
            if (topology.m_nodes.size() == maxNodes) {
                return Result<WorldTopology>::Error("WorldTopology supero el limite de " +
                                                    std::to_string(maxNodes) + " niveles");
            }
            const std::string& path = pending[cursor];
            auto loaded = LevelLoader::loadFromFile(path);
            if (!loaded.isOk()) {
                return Result<WorldTopology>::Error("No se pudo cargar '" + path +
                                                    "': " + loaded.errorMessage());
            }

            const LevelDefinition& level = loaded.value();
            topology.m_indices.emplace(path, topology.m_nodes.size());
            topology.m_nodes.push_back(WorldNode{path, level.name, level.mapPath});

            for (const ObjectSpawn& spawn : level.objects) {
                if (spawn.targetLevel.empty()) {
                    continue;
                }
                topology.m_links.push_back(WorldLink{path, spawn.position, spawn.objectId,
                                                     spawn.targetLevel, spawn.targetPosition,
                                                     spawn.hasTargetPosition});
                if (scheduled.insert(spawn.targetLevel).second) {
                    pending.push_back(spawn.targetLevel);
                }
            }
        }
        return Result<WorldTopology>::Ok(std::move(topology));
    }

    const std::vector<WorldNode>& nodes() const { return m_nodes; }
    const std::vector<WorldLink>& links() const { return m_links; }
    std::size_t size() const { return m_nodes.size(); }

    bool contains(const std::string& levelPath) const {
        return m_indices.find(levelPath) != m_indices.end();
    }

    // Búsqueda en anchura sobre las puertas conocidas. source == target es
    // alcanzable: no hace falta viajar para estar en el mismo nivel.
    bool canReach(const std::string& sourceLevelPath, const std::string& targetLevelPath) const {
        if (!contains(sourceLevelPath) || !contains(targetLevelPath)) {
            return false;
        }
        if (sourceLevelPath == targetLevelPath) {
            return true;
        }

        std::vector<std::string> pending{sourceLevelPath};
        std::unordered_set<std::string> visited{sourceLevelPath};
        for (std::size_t cursor = 0; cursor < pending.size(); ++cursor) {
            const std::string& current = pending[cursor];
            for (const WorldLink& link : m_links) {
                if (link.sourceLevelPath != current ||
                    !visited.insert(link.targetLevelPath).second) {
                    continue;
                }
                if (link.targetLevelPath == targetLevelPath) {
                    return true;
                }
                pending.push_back(link.targetLevelPath);
            }
        }
        return false;
    }

private:
    std::vector<WorldNode> m_nodes;
    std::vector<WorldLink> m_links;
    std::unordered_map<std::string, std::size_t> m_indices;
};
