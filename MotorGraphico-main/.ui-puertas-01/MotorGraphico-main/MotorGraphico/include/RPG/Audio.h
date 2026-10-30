#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "Core/Errors/Result.h"

// ============================================================
// GDB-11 — Sistema de audio (miniaudio backend)
// ============================================================
//
// Interfaz de alto nivel para reproducción de audio en el motor.
// Usa miniaudio (header-only, cross-platform) como backend.
//
// Diseño:
// - AudioEngine: gestor global del dispositivo de audio
// - Sound: sonido individual (SFX, música, ambiente)
// - SoundInstance: reproducción activa de un Sound (permite múltiples
//   instancias simultáneas del mismo sonido)
// - Buses: canales de mezcla (master, music, sfx, ambience) con
//   control de volumen independiente
//
// NO HACE: gestión de assets (carga de archivos), eso es E-05/EDT-05.
// NO HACE: síntesis procedural, solo reproducción de samples.

namespace RPG {

// Tipos de bus de mezcla. El orden define la prioridad de mezcla.
enum class AudioBus : uint8_t {
    Master = 0,    // Bus principal (afecta todo)
    Music,         // Música de fondo
    SFX,           // Efectos de sonido
    Ambience,      // Ambiente/ambientación
    Count
};

// Configuración de un sonido al crearse
struct SoundConfig {
    std::string filePath;     // Ruta al archivo (wav, ogg, flac, mp3)
    AudioBus bus = AudioBus::SFX;
    float volume = 1.0f;      // 0.0 - 1.0
    float pitch = 1.0f;       // 0.5 - 2.0
    bool loop = false;        // Repetir indefinidamente
    bool stream = false;      // Streaming (para música larga) vs cargar completo
    bool spatial = false;     // Audio 3D posicional (futuro)
};

// Estado de una instancia de reproducción
struct SoundInstanceState {
    bool playing = false;
    bool paused = false;
    float volume = 1.0f;      // Multiplicador sobre el volumen del Sound
    float pitch = 1.0f;
    float pan = 0.0f;         // -1.0 (izq) a 1.0 (der), 0 = centro
    uint64_t framesPlayed = 0;
    uint64_t totalFrames = 0; // 0 si desconocido (streaming)
};

// Callback cuando un sonido termina (solo si no es loop)
using SoundEndCallback = std::function<void(uint32_t instanceId)>;

// Interfaz principal del motor de audio
class AudioEngine {
public:
    virtual ~AudioEngine() = default;

    // Inicializa el dispositivo de audio. Debe llamarse una vez al arranque.
    // Devuelve error si no hay dispositivo disponible.
    virtual Result<bool> initialize() = 0;

    // Cierra el dispositivo y libera todos los recursos.
    virtual void shutdown() = 0;

    // Actualiza el motor (procesa callbacks, streaming, etc.).
    // Debe llamarse una vez por frame.
    virtual void update() = 0;

    // Carga un sonido desde archivo. Devuelve ID del sonido o error.
    // El archivo puede ser wav, ogg, flac, mp3 (soportados por miniaudio).
    virtual Result<uint32_t> loadSound(const SoundConfig& config) = 0;

    // Descarga un sonido. Invalida todas sus instancias activas.
    virtual void unloadSound(uint32_t soundId) = 0;

    // Reproduce un sonido. Devuelve ID de la instancia o error.
    // Si soundId no existe, devuelve error.
    virtual Result<uint32_t> playSound(uint32_t soundId, const SoundEndCallback& onEnd = nullptr) = 0;

    // Detiene una instancia específica.
    virtual void stopInstance(uint32_t instanceId) = 0;

    // Pausa/reanuda una instancia.
    virtual void pauseInstance(uint32_t instanceId, bool pause) = 0;

    // Establece volumen de una instancia (0.0 - 1.0, multiplicador).
    virtual void setInstanceVolume(uint32_t instanceId, float volume) = 0;

    // Establece pitch de una instancia (0.5 - 2.0).
    virtual void setInstancePitch(uint32_t instanceId, float pitch) = 0;

    // Establece pan de una instancia (-1.0 izq, 0 centro, 1.0 der).
    virtual void setInstancePan(uint32_t instanceId, float pan) = 0;

    // Obtiene estado de una instancia.
    virtual Result<SoundInstanceState> getInstanceState(uint32_t instanceId) const = 0;

    // Volumen global de un bus (0.0 - 1.0).
    virtual void setBusVolume(AudioBus bus, float volume) = 0;
    virtual float getBusVolume(AudioBus bus) const = 0;

    // Silencia/reactiva un bus.
    virtual void setBusMuted(AudioBus bus, bool muted) = 0;
    virtual bool isBusMuted(AudioBus bus) const = 0;

    // Volumen maestro (afecta a todo).
    virtual void setMasterVolume(float volume) = 0;
    virtual float getMasterVolume() const = 0;

    // Número de instancias activas.
    virtual size_t getActiveInstanceCount() const = 0;

    // Limpia instancias terminadas (llamado internamente en update).
    virtual void cleanupFinishedInstances() = 0;
};

// Fábrica para crear la implementación concreta (miniaudio).
std::unique_ptr<AudioEngine> createAudioEngine();

} // namespace RPG