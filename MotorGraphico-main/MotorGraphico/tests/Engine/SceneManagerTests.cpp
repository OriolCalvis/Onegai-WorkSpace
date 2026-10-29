// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-16  (fase C, 4 j) - L'Application delega en SceneManager
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include <stdexcept>
#include <string>
#include <vector>

#include "Core/Ids/ProjectIds.h"
#include "Engine/Scene/SceneManager.h"
#include "TestHarness.h"

namespace {

ids::MapId mapId(const char* value) {
    const Result<ids::MapId> parsed = ids::MapId::parse(value);
    if (!parsed.isOk()) {
        throw std::runtime_error(parsed.errorMessage());
    }
    return parsed.value();
}

class TestScene final : public Engine::Scene {
public:
    TestScene(std::string name, std::vector<std::string>& events, ids::MapId map = {},
              bool shouldLoad = true)
        : m_name(std::move(name)), m_events(events), m_map(std::move(map)), m_shouldLoad(shouldLoad) {}

    std::string_view name() const override { return m_name; }
    ids::MapId mapId() const override { return m_map; }

    Result<bool> load() override {
        m_events.push_back(m_name + ".load");
        return m_shouldLoad ? Result<bool>::Ok(true)
                            : Result<bool>::Error("fixture rechazó la carga");
    }
    void onEnter() override { m_events.push_back(m_name + ".enter"); }
    void onPause() override { m_events.push_back(m_name + ".pause"); }
    void onResume() override { m_events.push_back(m_name + ".resume"); }
    void onExit() override { m_events.push_back(m_name + ".exit"); }
    void update(float) override { m_events.push_back(m_name + ".update"); }

private:
    std::string m_name;
    std::vector<std::string>& m_events;
    ids::MapId m_map;
    bool m_shouldLoad = true;
};

MOTOR_TEST_CASE(replace_is_deferred_until_after_update_and_preserves_map_identity) {
    std::vector<std::string> events;
    Engine::SceneManager scenes;
    MOTOR_CHECK(scenes.start(std::make_unique<TestScene>("pradera", events, mapId("pradera"))).isOk());

    scenes.update(0.016f);
    MOTOR_CHECK(scenes.requestReplace(
                    std::make_unique<TestScene>("ciudad", events, mapId("ciudad_centro")))
                    .isOk());
    MOTOR_CHECK(scenes.activeScene()->name() == "pradera");
    MOTOR_CHECK(scenes.hasPendingTransition());

    const Result<bool> applied = scenes.applyPendingTransition();
    MOTOR_CHECK(applied.isOk());
    MOTOR_CHECK(applied.value());
    MOTOR_CHECK(scenes.activeScene()->name() == "ciudad");
    MOTOR_CHECK(scenes.activeMapId() == mapId("ciudad_centro"));
    MOTOR_CHECK((events == std::vector<std::string>{"pradera.load", "pradera.enter", "pradera.update",
                                                     "ciudad.load", "pradera.exit", "ciudad.enter"}));
}

MOTOR_TEST_CASE(failed_load_keeps_the_previous_scene_alive_and_discards_request) {
    std::vector<std::string> events;
    Engine::SceneManager scenes;
    MOTOR_CHECK(scenes.start(std::make_unique<TestScene>("estable", events, mapId("estable"))).isOk());
    MOTOR_CHECK(scenes.requestReplace(
                    std::make_unique<TestScene>("rota", events, mapId("rota"), false))
                    .isOk());

    const Result<bool> applied = scenes.applyPendingTransition();
    MOTOR_CHECK(!applied.isOk());
    MOTOR_CHECK(scenes.activeScene()->name() == "estable");
    MOTOR_CHECK(!scenes.hasPendingTransition());
    MOTOR_CHECK((events == std::vector<std::string>{"estable.load", "estable.enter", "rota.load"}));
}

MOTOR_TEST_CASE(push_and_pop_pause_and_resume_without_destroying_the_root) {
    std::vector<std::string> events;
    Engine::SceneManager scenes;
    MOTOR_CHECK(scenes.start(std::make_unique<TestScene>("mundo", events, mapId("mundo"))).isOk());
    MOTOR_CHECK(scenes.requestPush(std::make_unique<TestScene>("pausa", events)).isOk());
    MOTOR_CHECK(scenes.applyPendingTransition().isOk());
    MOTOR_CHECK(scenes.stackDepth() == 2);
    MOTOR_CHECK(scenes.activeScene()->name() == "pausa");
    MOTOR_CHECK(scenes.activeMapId().empty());

    MOTOR_CHECK(scenes.requestPop().isOk());
    MOTOR_CHECK(scenes.applyPendingTransition().isOk());
    MOTOR_CHECK(scenes.stackDepth() == 1);
    MOTOR_CHECK(scenes.activeScene()->name() == "mundo");
    MOTOR_CHECK(!scenes.requestPop().isOk());
    MOTOR_CHECK((events == std::vector<std::string>{"mundo.load", "mundo.enter", "pausa.load",
                                                     "mundo.pause", "pausa.enter", "pausa.exit",
                                                     "mundo.resume"}));
}

MOTOR_TEST_CASE(only_one_transition_can_be_queued_per_frame) {
    std::vector<std::string> events;
    Engine::SceneManager scenes;
    MOTOR_CHECK(scenes.start(std::make_unique<TestScene>("inicio", events)).isOk());
    MOTOR_CHECK(scenes.requestReplace(std::make_unique<TestScene>("uno", events)).isOk());
    MOTOR_CHECK(!scenes.requestReplace(std::make_unique<TestScene>("dos", events)).isOk());
    MOTOR_CHECK(scenes.applyPendingTransition().isOk());
    MOTOR_CHECK(scenes.activeScene()->name() == "uno");
}

}  // namespace

int main() { return motor_test::runAllTests(); }
