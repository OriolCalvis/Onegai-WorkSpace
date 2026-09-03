#include "Core/Memory/HandlePool.h"

#include "TestHarness.h"

#include <string>

namespace {

MOTOR_TEST_CASE(default_handle_is_not_valid) {
    HandlePool<int> pool;
    const HandlePool<int>::Handle invalid;

    MOTOR_CHECK(pool.empty());
    MOTOR_CHECK(!pool.contains(invalid));
    MOTOR_CHECK(pool.get(invalid) == nullptr);
    MOTOR_CHECK(!pool.erase(invalid));
}

MOTOR_TEST_CASE(reused_slot_invalidates_its_previous_handle) {
    HandlePool<std::string> pool;
    const auto first = pool.emplace("first");
    const auto second = pool.emplace("second");

    MOTOR_CHECK(pool.erase(first));
    const auto replacement = pool.emplace("replacement");

    MOTOR_CHECK(replacement.index == first.index);
    MOTOR_CHECK(replacement.generation != first.generation);
    MOTOR_CHECK(!pool.contains(first));
    MOTOR_CHECK(pool.contains(second));
    MOTOR_CHECK(pool.get(replacement) != nullptr);
    MOTOR_CHECK(*pool.get(replacement) == "replacement");
}

MOTOR_TEST_CASE(clear_invalidates_every_live_handle_and_reuses_slots) {
    HandlePool<int> pool;
    const auto first = pool.emplace(10);
    const auto second = pool.emplace(20);

    pool.clear();

    MOTOR_CHECK(pool.empty());
    MOTOR_CHECK(!pool.contains(first));
    MOTOR_CHECK(!pool.contains(second));

    const auto replacement = pool.emplace(30);
    MOTOR_CHECK(pool.contains(replacement));
    MOTOR_CHECK(!pool.contains(first));
    MOTOR_CHECK(!pool.contains(second));
    MOTOR_CHECK(pool.get(replacement) != nullptr);
    MOTOR_CHECK(*pool.get(replacement) == 30);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
