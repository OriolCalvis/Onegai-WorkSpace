#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "Editor/EditorState.h"
#include "Game/Skill.h"
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

    // Validacion de contenido RPG antes del playtest. Mantiene el contrato
    // ligero de check() y anade la resolucion de referencias entre catalogos:
    // una entrada puede ser JSON valido y aun asi estar rota para el juego
    // (por ejemplo, un enemigo que menciona una skill inexistente).
    //
    // availableSpriteIds es opcional porque el atlas real pertenece a la
    // capa de render. Si se pasa, un sprite no registrado se convierte en
    // error; sin ese argumento solo se avisa de spriteId == -1.
    static EditorValidationResult checkRpg(const EditorState& editor,
                                           const ObjectCatalog& catalog,
                                           const SkillCatalog& skills,
                                           const std::vector<int>& collisionGids,
                                           const std::vector<int>& availableSpriteIds = {}) {
        EditorValidationResult result = check(editor, catalog, collisionGids);

        auto hasSprite = [&](int spriteId) {
            return std::find(availableSpriteIds.begin(), availableSpriteIds.end(), spriteId) !=
                   availableSpriteIds.end();
        };
        auto spriteLabel = [](const ObjectDefinition& def) {
            return "objeto '" + def.id + "'";
        };

        for (const std::string& id : catalog.ids()) {
            const ObjectDefinition* def = catalog.find(id);
            if (def == nullptr) {
                continue;
            }

            if (def->spriteId < 0) {
                result.warnings.push_back(spriteLabel(*def) + " no tiene sprite asignado");
            } else if (!availableSpriteIds.empty() && !hasSprite(def->spriteId)) {
                result.errors.push_back(spriteLabel(*def) + " referencia spriteId " +
                                        std::to_string(def->spriteId) + " ausente en el atlas");
            }

            if (def->category == ObjectCategory::Enemy) {
                if (def->combat.maxHealth <= 0) {
                    result.errors.push_back(spriteLabel(*def) +
                                            " enemigo con maxHealth menor o igual que cero");
                }
                if (def->combat.maxMana < 0) {
                    result.errors.push_back(spriteLabel(*def) +
                                            " enemigo con maxMana negativo");
                }
                for (const std::string& skillId : def->combat.skillIds) {
                    if (!skills.has(skillId)) {
                        result.errors.push_back(spriteLabel(*def) + " referencia skill inexistente '" +
                                                skillId + "'");
                    }
                }
            }

            if (def->category == ObjectCategory::Pickup) {
                if (def->pickup.power < 0 || def->pickup.price < 0) {
                    result.errors.push_back(spriteLabel(*def) +
                                            " tiene power o price negativo");
                }
                if (def->pickup.effect != PickupEffect::None && def->pickup.power == 0) {
                    result.warnings.push_back(spriteLabel(*def) +
                                              " tiene efecto consumible con power 0");
                }
            }

            if (def->category == ObjectCategory::Npc && !def->interactable) {
                result.warnings.push_back(spriteLabel(*def) +
                                          " es NPC pero no esta marcado como interactuable");
            }
        }

        for (const ObjectSpawn& spawn : editor.objects()) {
            const ObjectDefinition* def = catalog.find(spawn.objectId);
            if (def == nullptr) {
                continue;  // ya informado por check(), no duplicar el error
            }
            if (def->category == ObjectCategory::Enemy &&
                (spawn.patrolMin.x > spawn.patrolMax.x || spawn.patrolMin.y > spawn.patrolMax.y)) {
                result.errors.push_back("patrulla invalida para '" + spawn.objectId + "' en " +
                                        std::to_string(spawn.position.x) + "," +
                                        std::to_string(spawn.position.y));
            }
            if (!spawn.targetLevel.empty() && !def->interactable) {
                result.warnings.push_back("transicion de '" + spawn.objectId +
                                          "' configurada en objeto no interactuable");
            }
        }

        return result;
    }
};
