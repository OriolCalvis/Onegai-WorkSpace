#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <optional>
#include <vector>

#include "Core/Ids/ProjectIds.h"
#include "World/EntityHandle.h"

namespace Engine {

// RUN-04 — Entidad runtime mínima. authoringId es opcional: dos NPCs pueden
// compartir el mismo prefab/EntityId de autoría, pero siempre tienen handles
// runtime distintos. Los componentes llegarán en RUN-05, no se mezclan aquí.
struct WorldEntity {
    ids::EntityId authoringId;
    bool enabled = true;
};

class World {
public:
    using UpdateFunction = std::function<void(EntityHandle, WorldEntity&)>;

    // create reserva un handle de inmediato, pero la entidad no participa en
    // consultas ni updates hasta flushPendingChanges(). Así un sistema puede
    // solicitar spawns durante su iteración sin invalidarla.
    EntityHandle create(WorldEntity entity = {});

    // La baja se hace visible de inmediato para las APIs públicas (get()
    // devuelve nullptr), y su memoria se libera después de terminar update.
    bool destroy(EntityHandle handle);

    bool contains(EntityHandle handle) const;
    WorldEntity* get(EntityHandle handle);
    const WorldEntity* get(EntityHandle handle) const;
    std::size_t size() const { return m_size; }
    bool empty() const { return m_size == 0; }

    std::size_t pendingCreateCount() const { return m_pendingCreates.size(); }
    std::size_t pendingDestroyCount() const { return m_pendingDestroys.size(); }

    // Aplica altas y bajas diferidas. Es pública para inicializar un mundo
    // antes del primer tick; fixedUpdate la llama al inicio y al final.
    void flushPendingChanges();

    // Recorre índices crecientes, por lo que el orden de update es estable.
    // Las entidades marcadas para baja por un callback posterior no reciben
    // más callbacks en el mismo tick.
    void fixedUpdate(const UpdateFunction& update);

private:
    struct Slot {
        std::optional<WorldEntity> entity;
        std::uint32_t generation = 1;
        bool reservedCreate = false;
        bool destroyRequested = false;
    };

    struct PendingCreate {
        EntityHandle handle;
        WorldEntity entity;
    };

    bool matches(EntityHandle handle) const;
    bool isLive(EntityHandle handle) const;
    EntityHandle reserveSlot();
    void releaseSlot(std::uint32_t index);

    // deque mantiene las referencias a slots existentes cuando un callback
    // solicita un spawn y añade un slot al final durante fixedUpdate().
    std::deque<Slot> m_slots;
    std::vector<std::uint32_t> m_freeSlots;
    std::vector<PendingCreate> m_pendingCreates;
    std::vector<EntityHandle> m_pendingDestroys;
    std::size_t m_size = 0;
    bool m_isUpdating = false;
};

}  // namespace Engine
