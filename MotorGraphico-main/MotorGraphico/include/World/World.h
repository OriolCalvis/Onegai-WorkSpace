// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-17  (fase C, 6 j) - World és el propietari de les entitats
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Core/Ids/ProjectIds.h"
#include "Core/Math/GridCoord.h"
#include "Game/EnemyBrain.h"
#include "Game/Skill.h"
#include "Level/LevelDefinition.h"
#include "World/EntityHandle.h"

namespace Engine {

// C-17 — Clasificacion de las entidades del mundo. Los objetos (prop,
// pickup, puerta, NPC, cartel...) y los enemigos comparten slot en World
// pero llevan datos diferentes; el kind dice cual de los dos miembros de
// WorldEntity es el significativo.
enum class EntityKind {
    Object,
    Enemy,
};

// Un enemigo vivo. Vive DENTRO de World (C-17): esta es la forma que la
// entidad le da al cuerpo de combate. Antes esto era GameSession quien lo
// poseia (m_enemies); ahora lo posee el World y GameSession solo lo lee
// o lo da de baja con destroy().
//
// brain/skills son shared_ptr a proposito: las VISTAS derivadas
// (GameSession::enemies()) copian este struct para que quien ya iteraba
// listas no cambie, y compartir la misma instancia viva (en vez de
// clonar el brain) es exactamente lo que una vista debe hacer -- el
// World sigue siendo el propietario, la vista comparte sin poseer.
//
// objectId es la CLAVE de contenido (el id del catalogo de objetos:
// "slime", "perdido"...). spawnIndex es la identidad que sobrevive al
// guardado (C-04): la posicion en LevelDefinition::objects para un
// enemigo del nivel, u un ORDINAL de sesion si summoned (lo materializo
// la narrativa con un startBattle).
struct WorldEnemy {
    std::string objectId;
    int spawnIndex = -1;
    bool summoned = false;
    // La posicion AUTORITATIVA vive en brain->position(): este campo es
    // su espejo para quien ya lo lee (minimapa, marcadores de
    // Application, queries espaciales). Una sola verdad, un solo sitio
    // donde se escribe (GameSession::update).
    GridCoord position;
    std::shared_ptr<EnemyBrain> brain;
    std::shared_ptr<SkillSet> skills;
};

// RUN-04 — Entidad runtime. authoringId es opcional: dos NPCs pueden
// compartir el mismo prefab/EntityId de autoría, pero siempre tienen
// handles runtime distintos.
//
// C-17 — WorldEntity deja de ser una entidad vacia para ser la entidad
// del mundo de verdad. Segun kind, UNO de los dos miembros de datos es
// el significativo: object (el ObjectSpawn tal cual del nivel) para
// objetos, o enemy (identidad + cuerpo de combate) para enemigos.
// objectId() y position() unifican la lectura para quien no necesita
// distinguir el tipo.
struct WorldEntity {
    ids::EntityId authoringId;
    bool enabled = true;
    EntityKind kind = EntityKind::Object;

    // Solo UNO de los dos es significativo, segun kind.
    ObjectSpawn object;  // kind == Object: el spawn del nivel, tal cual
    WorldEnemy enemy;    // kind == Enemy: identidad + cuerpo de combate

    // Clave de contenido (id del catalogo), igual para los dos tipos.
    const std::string& objectId() const {
        return kind == EntityKind::Enemy ? enemy.objectId : object.objectId;
    }
    // Posicion viva actual: la del espejo del enemigo o la del objeto.
    GridCoord position() const {
        return kind == EntityKind::Enemy ? enemy.position : object.position;
    }
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

    // --- C-17: el World es el unico propietario de entidades ---

    // Factoria de objeto: kind=Object, objectId=spawn.objectId, posicion del
    // spawn. El prefab (authoringId) queda sin setear: es dato de autoria,
    // no de contenido (ver WorldEntity).
    EntityHandle createObject(ObjectSpawn spawn);

    // Factoria de enemigo: kind=Enemy. brain/skills ya construidos por quien
    // conoce el catalogo (GameSession::spawnEnemyFromCatalog y la carga de
    // nivel); aqui solo se agrupan bajo una entidad.
    EntityHandle createEnemy(WorldEnemy enemy);

    // Handle de la entidad viva CONCRETA en (x, y), o invalido. Distinguen
    // objetos de enemigos para que el combate no enganche un prop: dos
    // entidades de tipos distintos pueden coexistir por celda.
    EntityHandle findObjectAt(int x, int y) const;
    EntityHandle findEnemyAt(int x, int y) const;

    // Recorre las entidades VIVAS (ya aplicadas, no pendientes de spawn)
    // en orden de slot, como fixedUpdate. No muta la estructura del world,
    // asi que crear/destruir dentro de la iteracion es seguro (las altas
    // quedan pendientes y las bajas se marcan para el siguiente flush).
    // Const para lectores, mutador para quien actualiza (GameSession::update).
    template <typename Fn>
    void eachEntity(Fn fn) {
        for (std::uint32_t index = 0; index < m_slots.size(); ++index) {
            Slot& slot = m_slots[index];
            if (!slot.entity.has_value() || slot.destroyRequested || !slot.entity->enabled) {
                continue;
            }
            fn(EntityHandle{index, slot.generation}, *slot.entity);
        }
    }
    template <typename Fn>
    void eachEntity(Fn fn) const {
        for (std::uint32_t index = 0; index < m_slots.size(); ++index) {
            const Slot& slot = m_slots[index];
            if (!slot.entity.has_value() || slot.destroyRequested || !slot.entity->enabled) {
                continue;
            }
            fn(EntityHandle{index, slot.generation}, *slot.entity);
        }
    }

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