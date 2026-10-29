#include <cmath>
#include <vector>

#include "Engine/ApplicationLoop.h"
#include "TestHarness.h"

namespace {

struct RecordedLoop {
    bool running = true;
    int polls = 0;
    int inputs = 0;
    int renders = 0;
    std::vector<float> updates;
    std::vector<float> interpolationAlphas;

    Engine::ApplicationLoopServices services() {
        return Engine::ApplicationLoopServices{
            [this] { return running; },
            [this] { ++polls; },
            [this] { ++inputs; },
            [this](float dt) { updates.push_back(dt); },
            [this](float alpha) {
                ++renders;
                interpolationAlphas.push_back(alpha);
            }};
    }
};

bool nearlyEqual(float left, float right) { return std::fabs(left - right) < 0.0001f; }

MOTOR_TEST_CASE(loop_polls_and_processes_input_once_per_rendered_frame) {
    Engine::ApplicationLoop loop;
    RecordedLoop recorded;

    const Engine::ApplicationLoopFrame first = loop.tick(1.0f / 120.0f, recorded.services());
    const Engine::ApplicationLoopFrame second = loop.tick(1.0f / 120.0f, recorded.services());

    MOTOR_CHECK(first.rendered);
    MOTOR_CHECK(first.inputProcessed);
    MOTOR_CHECK(first.fixedUpdates == 0);
    MOTOR_CHECK(nearlyEqual(first.interpolationAlpha, 0.5f));
    MOTOR_CHECK(second.rendered);
    MOTOR_CHECK(second.fixedUpdates == 1);
    MOTOR_CHECK(nearlyEqual(recorded.updates.front(), 1.0f / 60.0f));
    MOTOR_CHECK(recorded.polls == 2);
    MOTOR_CHECK(recorded.inputs == 2);
    MOTOR_CHECK(recorded.renders == 2);
}

MOTOR_TEST_CASE(loop_clamps_a_pause_and_discards_backlog_after_the_budget) {
    Engine::ApplicationLoop loop({0.01f, 0.05f, 3});
    RecordedLoop recorded;

    const Engine::ApplicationLoopFrame stalled = loop.tick(2.0f, recorded.services());
    MOTOR_CHECK(stalled.fixedUpdates == 3);
    MOTOR_CHECK(nearlyEqual(stalled.acceptedFrameSeconds, 0.05f));
    MOTOR_CHECK(nearlyEqual(stalled.droppedSimulationSeconds, 0.02f));
    MOTOR_CHECK(nearlyEqual(stalled.interpolationAlpha, 0.0f));

    const Engine::ApplicationLoopFrame next = loop.tick(0.01f, recorded.services());
    MOTOR_CHECK(next.fixedUpdates == 1);
    MOTOR_CHECK(nearlyEqual(next.droppedSimulationSeconds, 0.0f));
    MOTOR_CHECK(recorded.updates.size() == 4);
}

MOTOR_TEST_CASE(loop_stops_after_platform_or_input_requests_exit) {
    Engine::ApplicationLoop loop;
    RecordedLoop recorded;
    Engine::ApplicationLoopServices services = recorded.services();
    services.pollEvents = [&recorded] {
        ++recorded.polls;
        recorded.running = false;
    };

    const Engine::ApplicationLoopFrame frame = loop.tick(1.0f / 60.0f, services);
    MOTOR_CHECK(!frame.rendered);
    MOTOR_CHECK(!frame.inputProcessed);
    MOTOR_CHECK(frame.fixedUpdates == 0);
    MOTOR_CHECK(recorded.polls == 1);
    MOTOR_CHECK(recorded.inputs == 0);
    MOTOR_CHECK(recorded.renders == 0);
}

MOTOR_TEST_CASE(loop_normalizes_invalid_configuration_and_ignores_invalid_clock_values) {
    Engine::ApplicationLoop loop({0.0f, -1.0f, 0});
    RecordedLoop recorded;

    MOTOR_CHECK(nearlyEqual(loop.settings().fixedStepSeconds, 1.0f / 60.0f));
    MOTOR_CHECK(nearlyEqual(loop.settings().maxFrameSeconds, 0.10f));
    MOTOR_CHECK(loop.settings().maxFixedUpdatesPerFrame == 6);

    const Engine::ApplicationLoopFrame frame = loop.tick(-3.0f, recorded.services());
    MOTOR_CHECK(frame.rendered);
    MOTOR_CHECK(nearlyEqual(frame.acceptedFrameSeconds, 0.0f));
    MOTOR_CHECK(frame.fixedUpdates == 0);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
