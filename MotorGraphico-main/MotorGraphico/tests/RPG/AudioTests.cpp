#include "RPG/Audio.h"
#include "TestHarness.h"

namespace {

using RPG::AudioEngine;
using RPG::SoundConfig;
using RPG::AudioBus;
using RPG::SoundInstanceState;

MOTOR_TEST_CASE(audio_engine_crea_y_destruye) {
    auto engine = RPG::createAudioEngine();
    MOTOR_CHECK(engine != nullptr);

    auto result = engine->initialize();
    MOTOR_CHECK(result.isOk());

    engine->shutdown();
}

MOTOR_TEST_CASE(audio_engine_carga_sonido_inexistente_falla) {
    auto engine = RPG::createAudioEngine();
    MOTOR_CHECK(engine->initialize().isOk());

    SoundConfig config;
    config.filePath = "no_existe.wav";
    config.bus = AudioBus::SFX;

    auto result = engine->loadSound(config);
    MOTOR_CHECK(!result.isOk());
    MOTOR_CHECK(result.errorMessage().find("no_existe") != std::string::npos ||
                result.errorMessage().find("Error") != std::string::npos);

    engine->shutdown();
}

MOTOR_TEST_CASE(audio_bus_volumen_y_mute) {
    auto engine = RPG::createAudioEngine();
    MOTOR_CHECK(engine->initialize().isOk());

    // Volumen por defecto
    MOTOR_CHECK(engine->getBusVolume(AudioBus::Master) == 1.0f);
    MOTOR_CHECK(engine->getBusVolume(AudioBus::Music) == 1.0f);
    MOTOR_CHECK(engine->getBusVolume(AudioBus::SFX) == 1.0f);
    MOTOR_CHECK(engine->getBusVolume(AudioBus::Ambience) == 1.0f);

    // Cambiar volumen
    engine->setBusVolume(AudioBus::Music, 0.5f);
    MOTOR_CHECK(engine->getBusVolume(AudioBus::Music) == 0.5f);
    MOTOR_CHECK(engine->getBusVolume(AudioBus::SFX) == 1.0f); // No afectado

    // Mute
    engine->setBusMuted(AudioBus::SFX, true);
    MOTOR_CHECK(engine->isBusMuted(AudioBus::SFX));
    MOTOR_CHECK(!engine->isBusMuted(AudioBus::Music));

    // Volumen maestro
    engine->setMasterVolume(0.7f);
    MOTOR_CHECK(engine->getMasterVolume() == 0.7f);

    // Clamping
    engine->setBusVolume(AudioBus::Master, 1.5f);
    MOTOR_CHECK(engine->getBusVolume(AudioBus::Master) == 1.0f);
    engine->setBusVolume(AudioBus::Master, -0.5f);
    MOTOR_CHECK(engine->getBusVolume(AudioBus::Master) == 0.0f);

    engine->shutdown();
}

MOTOR_TEST_CASE(audio_sin_inicializar_falla) {
    auto engine = RPG::createAudioEngine();
    // No llamar initialize()

    SoundConfig config;
    config.filePath = "dummy.wav";
    config.bus = AudioBus::SFX;

    auto result = engine->loadSound(config);
    MOTOR_CHECK(!result.isOk());

    auto playResult = engine->playSound(1);
    MOTOR_CHECK(!playResult.isOk());
}

} // namespace

int main() { return motor_test::runAllTests(); }