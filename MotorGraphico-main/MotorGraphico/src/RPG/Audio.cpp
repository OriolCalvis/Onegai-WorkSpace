// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   GDB-11  (fase G, 7 j) - Implementar audio, música y ambiente
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "RPG/Audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <algorithm>
#include <unordered_map>

namespace RPG {

// ============================================================
// Implementación interna con miniaudio
// ============================================================

struct SoundData {
    SoundConfig config;
    ma_decoder decoder;           // Para streaming
    ma_sound sound;               // Para reproducción completa en memoria
    bool isStream = false;
    bool loaded = false;
    uint32_t id = 0;
};

struct InstanceData {
    uint32_t id = 0;
    uint32_t soundId = 0;
    ma_sound sound;
    SoundEndCallback onEnd;
    bool active = false;
    bool ended = false;
};

class AudioEngineImpl : public AudioEngine {
public:
    AudioEngineImpl() : m_initialized(false), m_masterVolume(1.0f), m_nextSoundId(1), m_nextInstanceId(1) {
        for (int i = 0; i < static_cast<int>(AudioBus::Count); ++i) {
            m_busVolumes[i] = 1.0f;
            m_busMuted[i] = false;
        }
    }

    ~AudioEngineImpl() override {
        shutdown();
    }

    Result<bool> initialize() override {
        if (m_initialized) {
            return Result<bool>::Ok(true);
        }

        ma_engine_config config = ma_engine_config_init();
        config.noDevice = false;
        config.channels = 2;
        config.sampleRate = 48000;

        ma_result result = ma_engine_init(&config, &m_engine);
        if (result != MA_SUCCESS) {
            return Result<bool>::Error("Error inicializando miniaudio: " + std::to_string(result));
        }

        m_initialized = true;
        return Result<bool>::Ok(true);
    }

    void shutdown() override {
        if (!m_initialized) return;

        // Detener todas las instancias
        for (auto& [id, inst] : m_instances) {
            if (inst.active) {
                ma_sound_stop(&inst.sound);
            }
            ma_sound_uninit(&inst.sound);
        }
        m_instances.clear();

        // Descargar todos los sonidos
        for (auto& [id, sound] : m_sounds) {
            if (sound.loaded) {
                if (sound.isStream) {
                    ma_decoder_uninit(&sound.decoder);
                } else {
                    ma_sound_uninit(&sound.sound);
                }
                sound.loaded = false;
            }
        }
        m_sounds.clear();

        ma_engine_uninit(&m_engine);
        m_initialized = false;
    }

    void update() override {
        if (!m_initialized) return;

        cleanupFinishedInstances();

        // Procesar callbacks de fin de sonido
        std::vector<uint32_t> toRemove;
        for (auto& [id, inst] : m_instances) {
            if (inst.active && inst.ended) {
                if (inst.onEnd) {
                    inst.onEnd(id);
                }
                toRemove.push_back(id);
            }
        }
        for (uint32_t id : toRemove) {
            stopInstance(id);
        }
    }

    Result<uint32_t> loadSound(const SoundConfig& config) override {
        if (!m_initialized) {
            return Result<uint32_t>::Error("AudioEngine no inicializado");
        }

        SoundData sound;
        sound.config = config;
        sound.id = m_nextSoundId++;
        sound.isStream = config.stream;

        if (config.stream) {
            // Streaming: usar decoder
            ma_decoder_config decoderConfig = ma_decoder_config_init(
                ma_format_f32, 2, 48000);
            ma_result result = ma_decoder_init_file(config.filePath.c_str(), &decoderConfig, &sound.decoder);
            if (result != MA_SUCCESS) {
                return Result<uint32_t>::Error("Error abriendo archivo para streaming: " + config.filePath);
            }
            sound.loaded = true;
        } else {
            // Cargar completo en memoria usando ma_sound
            ma_uint32 flags = 0;
            ma_result result = ma_sound_init_from_file(&m_engine, config.filePath.c_str(), flags, nullptr, nullptr, &sound.sound);
            if (result != MA_SUCCESS) {
                return Result<uint32_t>::Error("Error cargando sonido: " + config.filePath);
            }
            sound.loaded = true;
        }

        uint32_t id = sound.id;
        m_sounds[id] = std::move(sound);
        return Result<uint32_t>::Ok(id);
    }

    void unloadSound(uint32_t soundId) override {
        auto it = m_sounds.find(soundId);
        if (it == m_sounds.end()) return;

        SoundData& sound = it->second;
        if (sound.loaded) {
            if (sound.isStream) {
                ma_decoder_uninit(&sound.decoder);
            } else {
                ma_sound_uninit(&sound.sound);
            }
            sound.loaded = false;
        }

        // Detener instancias que usen este sonido
        std::vector<uint32_t> toStop;
        for (auto& [instId, inst] : m_instances) {
            if (inst.active && inst.soundId == soundId) {
                toStop.push_back(instId);
            }
        }
        for (uint32_t instId : toStop) {
            stopInstance(instId);
        }

        m_sounds.erase(it);
    }

    Result<uint32_t> playSound(uint32_t soundId, const SoundEndCallback& onEnd) override {
        if (!m_initialized) {
            return Result<uint32_t>::Error("AudioEngine no inicializado");
        }

        auto soundIt = m_sounds.find(soundId);
        if (soundIt == m_sounds.end()) {
            return Result<uint32_t>::Error("Sonido no encontrado: " + std::to_string(soundId));
        }

        SoundData& sound = soundIt->second;
        if (!sound.loaded) {
            return Result<uint32_t>::Error("Sonido no cargado correctamente");
        }

        InstanceData inst;
        inst.id = m_nextInstanceId++;
        inst.soundId = soundId;
        inst.onEnd = onEnd;
        inst.active = true;
        inst.ended = false;

        ma_uint32 flags = 0;

        if (sound.isStream) {
            // Para streaming, crear sound desde decoder
            ma_result result = ma_sound_init_from_data_source(&m_engine, &sound.decoder, flags, nullptr, &inst.sound);
            if (result != MA_SUCCESS) {
                return Result<uint32_t>::Error("Error creando instancia de streaming");
            }
            // Reiniciar decoder para nueva reproducción
            ma_decoder_seek_to_pcm_frame(&sound.decoder, 0);
        } else {
            // Clonar el sound cargado en memoria
            ma_result result = ma_sound_init_copy(&m_engine, &sound.sound, flags, nullptr, &inst.sound);
            if (result != MA_SUCCESS) {
                return Result<uint32_t>::Error("Error clonando sonido");
            }
        }

        // Configurar propiedades iniciales
        float busVol = m_busVolumes[static_cast<int>(sound.config.bus)];
        if (m_busMuted[static_cast<int>(sound.config.bus)]) busVol = 0.0f;
        float finalVol = sound.config.volume * busVol * m_masterVolume;
        ma_sound_set_volume(&inst.sound, finalVol);
        ma_sound_set_pitch(&inst.sound, sound.config.pitch);
        ma_sound_set_looping(&inst.sound, sound.config.loop);

        uint32_t id = inst.id;
        m_instances[id] = std::move(inst);
        return Result<uint32_t>::Ok(id);
    }

    void stopInstance(uint32_t instanceId) override {
        auto it = m_instances.find(instanceId);
        if (it == m_instances.end()) return;

        InstanceData& inst = it->second;
        if (inst.active) {
            ma_sound_stop(&inst.sound);
            ma_sound_uninit(&inst.sound);
            inst.active = false;
        }
        m_instances.erase(it);
    }

    void pauseInstance(uint32_t instanceId, bool pause) override {
        auto it = m_instances.find(instanceId);
        if (it == m_instances.end()) return;

        InstanceData& inst = it->second;
        if (!inst.active) return;

        if (pause) {
            ma_sound_stop(&inst.sound);
        } else {
            ma_sound_start(&inst.sound);
        }
    }

    void setInstanceVolume(uint32_t instanceId, float volume) override {
        auto it = m_instances.find(instanceId);
        if (it == m_instances.end()) return;

        InstanceData& inst = it->second;
        if (!inst.active) return;

        auto soundIt = m_sounds.find(inst.soundId);
        if (soundIt == m_sounds.end()) return;

        SoundData& sound = soundIt->second;
        float busVol = m_busVolumes[static_cast<int>(sound.config.bus)];
        if (m_busMuted[static_cast<int>(sound.config.bus)]) busVol = 0.0f;
        float finalVol = std::clamp(volume, 0.0f, 1.0f) * sound.config.volume * busVol * m_masterVolume;
        ma_sound_set_volume(&inst.sound, finalVol);
    }

    void setInstancePitch(uint32_t instanceId, float pitch) override {
        auto it = m_instances.find(instanceId);
        if (it == m_instances.end()) return;

        InstanceData& inst = it->second;
        if (!inst.active) return;

        ma_sound_set_pitch(&inst.sound, std::clamp(pitch, 0.5f, 2.0f));
    }

    void setInstancePan(uint32_t instanceId, float pan) override {
        auto it = m_instances.find(instanceId);
        if (it == m_instances.end()) return;

        InstanceData& inst = it->second;
        if (!inst.active) return;

        ma_sound_set_pan(&inst.sound, std::clamp(pan, -1.0f, 1.0f));
    }

    Result<SoundInstanceState> getInstanceState(uint32_t instanceId) const override {
        auto it = m_instances.find(instanceId);
        if (it == m_instances.end()) {
            return Result<SoundInstanceState>::Error("Instancia no encontrada");
        }

        const InstanceData& inst = it->second;
        SoundInstanceState state;
        state.playing = inst.active && !inst.ended;
        state.paused = inst.active && ma_sound_is_playing(&inst.sound) == MA_FALSE && !inst.ended;
        state.volume = ma_sound_get_volume(&inst.sound);
        state.pitch = ma_sound_get_pitch(&inst.sound);
        state.pan = ma_sound_get_pan(&inst.sound);
        ma_uint64 cursor = 0;
        ma_sound_get_cursor_in_pcm_frames(&inst.sound, &cursor);
        state.framesPlayed = cursor;

        if (inst.soundId != 0) {
            auto soundIt = m_sounds.find(inst.soundId);
            if (soundIt != m_sounds.end()) {
                if (!soundIt->second.isStream) {
                    ma_uint64 length = 0;
                    ma_sound_get_length_in_pcm_frames(&soundIt->second.sound, &length);
                    state.totalFrames = length;
                }
            }
        }

        return Result<SoundInstanceState>::Ok(state);
    }

    void setBusVolume(AudioBus bus, float volume) override {
        int idx = static_cast<int>(bus);
        if (idx >= 0 && idx < static_cast<int>(AudioBus::Count)) {
            m_busVolumes[idx] = std::clamp(volume, 0.0f, 1.0f);
            updateBusVolumes();
        }
    }

    float getBusVolume(AudioBus bus) const override {
        int idx = static_cast<int>(bus);
        if (idx >= 0 && idx < static_cast<int>(AudioBus::Count)) {
            return m_busVolumes[idx];
        }
        return 1.0f;
    }

    void setBusMuted(AudioBus bus, bool muted) override {
        int idx = static_cast<int>(bus);
        if (idx >= 0 && idx < static_cast<int>(AudioBus::Count)) {
            m_busMuted[idx] = muted;
            updateBusVolumes();
        }
    }

    bool isBusMuted(AudioBus bus) const override {
        int idx = static_cast<int>(bus);
        if (idx >= 0 && idx < static_cast<int>(AudioBus::Count)) {
            return m_busMuted[idx];
        }
        return false;
    }

    void setMasterVolume(float volume) override {
        m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
        updateBusVolumes();
    }

    float getMasterVolume() const override {
        return m_masterVolume;
    }

    size_t getActiveInstanceCount() const override {
        size_t count = 0;
        for (const auto& [id, inst] : m_instances) {
            if (inst.active) ++count;
        }
        return count;
    }

    void cleanupFinishedInstances() override {
        std::vector<uint32_t> toRemove;
        for (auto& [id, inst] : m_instances) {
            if (inst.active && inst.ended) {
                ma_sound_uninit(&inst.sound);
                inst.active = false;
                toRemove.push_back(id);
            } else if (inst.active && ma_sound_is_playing(&inst.sound) == MA_FALSE && !inst.ended) {
                // Comprobar si terminó naturalmente (no loop)
                auto soundIt = m_sounds.find(inst.soundId);
                if (soundIt != m_sounds.end() && !soundIt->second.config.loop) {
                    inst.ended = true;
                }
            }
        }
        for (uint32_t id : toRemove) {
            m_instances.erase(id);
        }
    }

private:
    void updateBusVolumes() {
        for (auto& [id, inst] : m_instances) {
            if (!inst.active) continue;
            auto soundIt = m_sounds.find(inst.soundId);
            if (soundIt == m_sounds.end()) continue;

            SoundData& sound = soundIt->second;
            float busVol = m_busVolumes[static_cast<int>(sound.config.bus)];
            if (m_busMuted[static_cast<int>(sound.config.bus)]) busVol = 0.0f;
            float finalVol = sound.config.volume * busVol * m_masterVolume;
            ma_sound_set_volume(&inst.sound, finalVol);
        }
    }

    ma_engine m_engine;
    bool m_initialized;
    float m_masterVolume;
    float m_busVolumes[static_cast<int>(AudioBus::Count)];
    bool m_busMuted[static_cast<int>(AudioBus::Count)];
    uint32_t m_nextSoundId;
    uint32_t m_nextInstanceId;
    std::unordered_map<uint32_t, SoundData> m_sounds;
    std::unordered_map<uint32_t, InstanceData> m_instances;
};

std::unique_ptr<AudioEngine> createAudioEngine() {
    return std::make_unique<AudioEngineImpl>();
}

} // namespace RPG