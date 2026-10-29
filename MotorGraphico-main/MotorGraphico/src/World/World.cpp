// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   C-17  (fase C, 6 j) - World és el propietari de les entitats
//   C-18  (fase C, 2 j) - HandlePool sosté World
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "World/World.h"

#include <utility>

namespace Engine {

// C-17 — Los spawns del nivel solo son la receta: las entidades son el
// plato. Estos dos metodos son la unica puerta por la que el contenido
// (el level JSON a traves de GameSession, los invocados de la narrativa,
// los restaurados del guardado) entra en el World. Viven aqui y no en las
// clases de fuera para que crear una entidad del mundo tenga UN solo
// sitio.
EntityHandle World::createObject(ObjectSpawn spawn) {
    WorldEntity entity;
    entity.kind = EntityKind::Object;
    entity.object = std::move(spawn);
    return create(std::move(entity));
}

EntityHandle World::createEnemy(WorldEnemy enemy) {
    WorldEntity entity;
    entity.kind = EntityKind::Enemy;
    entity.enemy = std::move(enemy);
    return create(std::move(entity));
}

EntityHandle World::findObjectAt(int x, int y) const {
    for (std::uint32_t index = 0; index < m_slots.size(); ++index) {
        const Slot& slot = m_slots[index];
        if (!slot.entity.has_value() || slot.destroyRequested || !slot.entity->enabled) {
            continue;
        }
        const WorldEntity& entity = *slot.entity;
        if (entity.kind == EntityKind::Object && entity.object.position.x == x &&
            entity.object.position.y == y) {
            return EntityHandle{index, slot.generation};
        }
    }
    return {};
}

EntityHandle World::findEnemyAt(int x, int y) const {
    for (std::uint32_t index = 0; index < m_slots.size(); ++index) {
        const Slot& slot = m_slots[index];
        if (!slot.entity.has_value() || slot.destroyRequested || !slot.entity->enabled) {
            continue;
        }
        const WorldEntity& entity = *slot.entity;
        if (entity.kind == EntityKind::Enemy && entity.enemy.position.x == x &&
            entity.enemy.position.y == y) {
            return EntityHandle{index, slot.generation};
        }
    }
    return {};
}

EntityHandle World::create(WorldEntity entity) {
    const EntityHandle handle = reserveSlot();
    if (!handle.valid()) {
        return handle;
    }
    m_pendingCreates.push_back(PendingCreate{handle, std::move(entity)});
    return handle;
}

bool World::destroy(EntityHandle handle) {
    if (!matches(handle)) {
        return false;
    }
    Slot& slot = m_slots[handle.index];
    if (slot.destroyRequested) {
        return false;
    }
    if (!slot.entity.has_value() && !slot.reservedCreate) {
        return false;
    }
    slot.destroyRequested = true;
    m_pendingDestroys.push_back(handle);
    return true;
}

bool World::contains(EntityHandle handle) const { return isLive(handle); }

WorldEntity* World::get(EntityHandle handle) {
    return isLive(handle) ? &*m_slots[handle.index].entity : nullptr;
}

const WorldEntity* World::get(EntityHandle handle) const {
    return isLive(handle) ? &*m_slots[handle.index].entity : nullptr;
}

void World::flushPendingChanges() {
    if (m_isUpdating) {
        return;
    }

    for (PendingCreate& pending : m_pendingCreates) {
        if (!matches(pending.handle)) {
            continue;
        }
        Slot& slot = m_slots[pending.handle.index];
        if (!slot.destroyRequested && slot.reservedCreate) {
            slot.entity.emplace(std::move(pending.entity));
            slot.reservedCreate = false;
            ++m_size;
        }
    }
    m_pendingCreates.clear();

    for (EntityHandle handle : m_pendingDestroys) {
        if (!matches(handle)) {
            continue;
        }
        Slot& slot = m_slots[handle.index];
        if (!slot.destroyRequested) {
            continue;
        }
        if (slot.entity.has_value()) {
            slot.entity.reset();
            --m_size;
        }
        releaseSlot(handle.index);
    }
    m_pendingDestroys.clear();
}

void World::fixedUpdate(const UpdateFunction& update) {
    flushPendingChanges();
    if (!update) {
        return;
    }

    m_isUpdating = true;
    for (std::uint32_t index = 0; index < m_slots.size(); ++index) {
        Slot& slot = m_slots[index];
        if (!slot.entity.has_value() || slot.destroyRequested || !slot.entity->enabled) {
            continue;
        }
        update(EntityHandle{index, slot.generation}, *slot.entity);
    }
    m_isUpdating = false;
    flushPendingChanges();
}

bool World::matches(EntityHandle handle) const {
    return handle.valid() && handle.index < m_slots.size() &&
           m_slots[handle.index].generation == handle.generation;
}

bool World::isLive(EntityHandle handle) const {
    return matches(handle) && m_slots[handle.index].entity.has_value() &&
           !m_slots[handle.index].destroyRequested;
}

EntityHandle World::reserveSlot() {
    if (!m_freeSlots.empty()) {
        const std::uint32_t index = m_freeSlots.back();
        m_freeSlots.pop_back();
        Slot& slot = m_slots[index];
        slot.reservedCreate = true;
        return EntityHandle{index, slot.generation};
    }
    if (m_slots.size() >= static_cast<std::size_t>(EntityHandle::kInvalidIndex)) {
        return {};
    }
    Slot slot;
    slot.reservedCreate = true;
    m_slots.push_back(std::move(slot));
    return EntityHandle{static_cast<std::uint32_t>(m_slots.size() - 1), m_slots.back().generation};
}

void World::releaseSlot(std::uint32_t index) {
    Slot& slot = m_slots[index];
    slot.entity.reset();
    slot.reservedCreate = false;
    slot.destroyRequested = false;
    ++slot.generation;
    if (slot.generation == 0) {
        ++slot.generation;
    }
    m_freeSlots.push_back(index);
}

}  // namespace Engine
