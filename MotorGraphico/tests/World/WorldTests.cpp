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

}  // namespace

int main() { return motor_test::runAllTests(); }
