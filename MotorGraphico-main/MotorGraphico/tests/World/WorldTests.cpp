#include <stdexcept>
#include <vector>

#include "Core/Ids/ProjectIds.h"
#include "TestHarness.h"
#include "World/World.h"

namespace {

ids::EntityId entityId(const char* value) {
    const Result<ids::EntityId> parsed = ids::EntityId::parse(value);
    if (!parsed.isOk()) {
        throw std::runtime_error(parsed.errorMessage());
    }
    return parsed.value();
}

MOTOR_TEST_CASE(created_entities_become_live_only_after_the_deferred_flush) {
    Engine::World world;
    const Engine::EntityHandle guide = world.create({entityId("guia"), true});

    MOTOR_CHECK(guide.valid());
    MOTOR_CHECK(!world.contains(guide));
    MOTOR_CHECK(world.pendingCreateCount() == 1);
    world.flushPendingChanges();
    MOTOR_CHECK(world.contains(guide));
    MOTOR_CHECK(world.size() == 1);
    MOTOR_CHECK(world.get(guide)->authoringId == entityId("guia"));
}

MOTOR_TEST_CASE(destroyed_handles_become_stale_when_the_slot_is_reused) {
    Engine::World world;
    const Engine::EntityHandle first = world.create({entityId("primero"), true});
    world.flushPendingChanges();
    MOTOR_CHECK(world.destroy(first));
    MOTOR_CHECK(!world.contains(first));
    world.flushPendingChanges();

    const Engine::EntityHandle replacement = world.create({entityId("reemplazo"), true});
    world.flushPendingChanges();
    MOTOR_CHECK(replacement.index == first.index);
    MOTOR_CHECK(replacement.generation != first.generation);
    MOTOR_CHECK(!world.contains(first));
    MOTOR_CHECK(world.contains(replacement));
}

MOTOR_TEST_CASE(update_order_is_deterministic_and_destroy_is_safe_during_iteration) {
    Engine::World world;
    const Engine::EntityHandle first = world.create({entityId("primero"), true});
    const Engine::EntityHandle second = world.create({entityId("segundo"), true});
    world.flushPendingChanges();
    std::vector<ids::EntityId> updated;

    world.fixedUpdate([&](Engine::EntityHandle handle, Engine::WorldEntity& entity) {
        updated.push_back(entity.authoringId);
        if (handle == first) {
            MOTOR_CHECK(world.destroy(second));
            const Engine::EntityHandle spawned = world.create({entityId("tercero"), true});
            MOTOR_CHECK(spawned.valid());
            MOTOR_CHECK(!world.contains(spawned));
        }
    });

    MOTOR_CHECK((updated == std::vector<ids::EntityId>{entityId("primero")}));
    MOTOR_CHECK(!world.contains(second));
    MOTOR_CHECK(world.size() == 2);

    updated.clear();
    world.fixedUpdate([&](Engine::EntityHandle, Engine::WorldEntity& entity) {
        updated.push_back(entity.authoringId);
    });
    MOTOR_CHECK((updated == std::vector<ids::EntityId>{entityId("primero"), entityId("tercero")}));
}

MOTOR_TEST_CASE(disabled_entities_remain_addressable_but_skip_updates) {
    Engine::World world;
    const Engine::EntityHandle disabled = world.create({entityId("quieto"), false});
    world.flushPendingChanges();
    int calls = 0;
    world.fixedUpdate([&](Engine::EntityHandle, Engine::WorldEntity&) { ++calls; });
    MOTOR_CHECK(world.contains(disabled));
    MOTOR_CHECK(calls == 0);
}

// --- C-17: el World es el propietario unico de entidades del mundo ---

MOTOR_TEST_CASE(createObject_indexes_the_spawn_and_findObjectAt_finds_it) {
    Engine::World world;
    ObjectSpawn spawn;
    spawn.objectId = "cartel_posada";
    spawn.position = {4, 7};
    const Engine::EntityHandle handle = world.createObject(spawn);
    world.flushPendingChanges();
    MOTOR_CHECK(handle.valid());
    MOTOR_CHECK(world.contains(handle));

    const Engine::EntityHandle found = world.findObjectAt(4, 7);
    MOTOR_CHECK(found == handle);
    MOTOR_CHECK(world.get(handle)->kind == Engine::EntityKind::Object);
    MOTOR_CHECK(world.get(handle)->objectId() == "cartel_posada");
    const GridCoord foundPos = world.get(found)->position();
    MOTOR_CHECK(foundPos.x == 4 && foundPos.y == 7);
    // Un objeto no hace de enemigo: no hay nada enemigo en esa celda.
    MOTOR_CHECK(!world.findEnemyAt(4, 7).valid());
    // Fuera de la celda del spawn no se encuentra nada.
    MOTOR_CHECK(!world.findObjectAt(4, 8).valid());
}

MOTOR_TEST_CASE(createEnemy_tracks_identity_and_brain_position) {
    Engine::World world;
    Engine::WorldEnemy enemy;
    enemy.objectId = "slime";
    enemy.spawnIndex = 2;
    enemy.position = {1, 1};
    enemy.brain = std::make_shared<EnemyBrain>(GridCoord{1, 1}, GridCoord{1, 1}, GridCoord{3, 1}, 40);
    enemy.skills = std::make_shared<SkillSet>(10);
    const Engine::EntityHandle handle = world.createEnemy(std::move(enemy));
    world.flushPendingChanges();

    Engine::WorldEntity* live = world.get(handle);
    MOTOR_CHECK(live != nullptr);
    MOTOR_CHECK(live->kind == Engine::EntityKind::Enemy);
    MOTOR_CHECK(live->objectId() == "slime");
    MOTOR_CHECK(live->enemy.spawnIndex == 2);
    MOTOR_CHECK(world.findEnemyAt(1, 1) == handle);
    MOTOR_CHECK(!world.findObjectAt(1, 1).valid());  // no hay objeto ahi

    // El enemigo patrulla: la posicion autoritativa es la del brain, y el
    // espejo enemy.position la informa (igual que hace GameSession::update).
    live->enemy.brain->setPosition(GridCoord{3, 1});
    live->enemy.position = live->enemy.brain->position();
    const GridCoord moved = live->position();
    MOTOR_CHECK(moved.x == 3 && moved.y == 1);
    MOTOR_CHECK(world.findEnemyAt(3, 1) == handle);
    MOTOR_CHECK(!world.findEnemyAt(1, 1).valid());
}

MOTOR_TEST_CASE(object_and_enemy_can_coexist_in_the_same_cell) {
    Engine::World world;
    Engine::WorldEnemy enemy;
    enemy.objectId = "lobo";
    enemy.spawnIndex = 0;
    enemy.position = {2, 2};
    enemy.brain = std::make_shared<EnemyBrain>(GridCoord{2, 2}, GridCoord{2, 2}, GridCoord{2, 2}, 30);
    enemy.skills = std::make_shared<SkillSet>(8);

    ObjectSpawn object;
    object.objectId = "cofre";
    object.position = {2, 2};

    const Engine::EntityHandle enemyHandle = world.createEnemy(std::move(enemy));
    const Engine::EntityHandle objectHandle = world.createObject(std::move(object));
    world.flushPendingChanges();
    // Las queries espaciales distinguen por tipo: el combate no engancha
    // un prop, y el jugador no recoge al lobo.
    MOTOR_CHECK(world.findEnemyAt(2, 2) == enemyHandle);
    MOTOR_CHECK(world.findObjectAt(2, 2) == objectHandle);
}

MOTOR_TEST_CASE(destroyed_entities_leave_the_spatial_queries) {
    Engine::World world;
    Engine::WorldEnemy enemy;
    enemy.objectId = "perdido";
    enemy.spawnIndex = 5;
    enemy.position = {0, 0};
    enemy.brain = std::make_shared<EnemyBrain>(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0}, 25);
    enemy.skills = std::make_shared<SkillSet>(5);
    const Engine::EntityHandle handle = world.createEnemy(std::move(enemy));
    world.flushPendingChanges();
    MOTOR_CHECK(world.findEnemyAt(0, 0).valid());

    world.destroy(handle);
    MOTOR_CHECK(!world.findEnemyAt(0, 0).valid());
    world.flushPendingChanges();
    MOTOR_CHECK(!world.contains(handle));
    MOTOR_CHECK(!world.findEnemyAt(0, 0).valid());
}

MOTOR_TEST_CASE(eachEntity_visits_only_live_entities_in_slot_order) {
    Engine::World world;
    const Engine::EntityHandle e1 = world.createObject([] {
        ObjectSpawn s;
        s.objectId = "uno";
        s.position = {0, 0};
        return s;
    }());
    const Engine::EntityHandle e2 = world.create({entityId("dosp"), true});
    world.flushPendingChanges();
    world.destroy(e1);
    world.flushPendingChanges();

    std::vector<ids::EntityId> seen;
    world.eachEntity([&](Engine::EntityHandle, Engine::WorldEntity& entity) {
        seen.push_back(entity.authoringId);
    });
    // e1 se destruyo: solo queda e2. Y e1 se creo por ObjectSpawn, asi que
    // su authoringId era el defecto (no es dato de autoria).
    MOTOR_CHECK(seen == std::vector<ids::EntityId>{entityId("dosp")});

    const auto constCount = [&] {
        std::size_t count = 0;
        const Engine::World& cw = world;
        cw.eachEntity([&](Engine::EntityHandle, const Engine::WorldEntity&) { ++count; });
        return count;
    }();
    MOTOR_CHECK(constCount == 1);
}

// Caracterizacion U1-02.E02
MOTOR_TEST_CASE(create_does_not_grow_size_until_flush) {
    Engine::World world;
    const Engine::EntityHandle first = world.create({entityId("padre"), true});
    world.flushPendingChanges();
    MOTOR_CHECK(world.size() == 1);

    const Engine::EntityHandle second = world.create({entityId("hijo"), true});
    MOTOR_CHECK(world.size() == 1);
    MOTOR_CHECK(world.pendingCreateCount() == 1);
    world.flushPendingChanges();
    MOTOR_CHECK(world.size() == 2);
    MOTOR_CHECK(world.pendingCreateCount() == 0);
}

// Caracterizacion U1-02.E02
MOTOR_TEST_CASE(entity_created_during_fixedUpdate_is_updated_only_on_the_next_tick) {
    Engine::World world;
    const Engine::EntityHandle padre = world.create({entityId("padre"), true});
    world.flushPendingChanges();

    Engine::EntityHandle hijo{};
    bool spawned = false;
    std::size_t padreCalls = 0;
    std::size_t hijoCalls = 0;

    world.fixedUpdate([&](Engine::EntityHandle handle, Engine::WorldEntity&) {
        if (handle == padre) {
            ++padreCalls;
            if (!spawned) {
                hijo = world.create({entityId("hijo"), true});
                spawned = true;
            }
        } else if (handle == hijo) {
            ++hijoCalls;
        }
    });

    MOTOR_CHECK(hijoCalls == 0);
    MOTOR_CHECK(padreCalls == 1);
    MOTOR_CHECK(world.contains(hijo));

    world.fixedUpdate([&](Engine::EntityHandle handle, Engine::WorldEntity&) {
        if (handle == padre) {
            ++padreCalls;
        } else if (handle == hijo) {
            ++hijoCalls;
        }
    });

    MOTOR_CHECK(hijoCalls == 1);
    MOTOR_CHECK(padreCalls == 2);
}

MOTOR_TEST_CASE(enemies_snapshot_shares_the_live_brain) {
    // Las vistas derivadas (GameSession::enemies()) copian WorldEnemy pero
    // NO el brain: compartir la instancia viva es lo que hace que mutar la
    // vista siga viendose en el World (BattleStateTests se apoya en esto).
    Engine::World world;
    Engine::WorldEnemy enemy;
    enemy.objectId = "slime";
    enemy.position = {1, 1};
    enemy.brain = std::make_shared<EnemyBrain>(GridCoord{1, 1}, GridCoord{1, 1}, GridCoord{1, 1}, 40);
    enemy.skills = std::make_shared<SkillSet>(10);
    const Engine::EntityHandle handle = world.createEnemy(std::move(enemy));
    world.flushPendingChanges();

    const Engine::WorldEnemy snapshot = world.get(handle)->enemy;  // copia barata (shared_ptr)
    snapshot.brain->takeDamage(10);
    MOTOR_CHECK(world.get(handle)->enemy.brain->health() == 30);
    MOTOR_CHECK(snapshot.brain == world.get(handle)->enemy.brain);
}

// Caracterizacion U1-02.E03
MOTOR_TEST_CASE(destroy_of_invalid_or_default_handle_returns_false) {
    Engine::World world;
    const Engine::EntityHandle live = world.create({entityId("viva"), true});
    world.flushPendingChanges();
    MOTOR_CHECK(world.size() == 1);

    MOTOR_CHECK(!world.destroy(Engine::EntityHandle{}));
    MOTOR_CHECK(!world.destroy(Engine::EntityHandle{live.index + 100, live.generation}));
    MOTOR_CHECK(!world.destroy(Engine::EntityHandle{live.index, live.generation + 1}));

    MOTOR_CHECK(world.size() == 1);
    MOTOR_CHECK(world.pendingDestroyCount() == 0);
    MOTOR_CHECK(world.contains(live));
}

// Caracterizacion U1-02.E03
MOTOR_TEST_CASE(second_destroy_returns_false_and_does_not_duplicate_the_queue) {
    Engine::World world;
    const Engine::EntityHandle doomed = world.create({entityId("condenada"), true});
    const Engine::EntityHandle other = world.create({entityId("otra"), true});
    world.flushPendingChanges();
    MOTOR_CHECK(world.size() == 2);

    MOTOR_CHECK(world.destroy(doomed));
    MOTOR_CHECK(!world.destroy(doomed));
    MOTOR_CHECK(world.pendingDestroyCount() == 1);

    world.flushPendingChanges();
    MOTOR_CHECK(world.size() == 1);
    MOTOR_CHECK(!world.contains(doomed));
    MOTOR_CHECK(world.contains(other));
}

// Caracterizacion U1-02.E03
MOTOR_TEST_CASE(create_and_destroy_before_flush_leave_nothing_alive) {
    Engine::World world;
    const Engine::EntityHandle handle = world.create({entityId("efimera"), true});
    MOTOR_CHECK(world.destroy(handle));

    world.flushPendingChanges();
    MOTOR_CHECK(world.size() == 0);
    MOTOR_CHECK(!world.contains(handle));
    MOTOR_CHECK(world.get(handle) == nullptr);
    MOTOR_CHECK(world.pendingCreateCount() == 0);
    MOTOR_CHECK(world.pendingDestroyCount() == 0);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
