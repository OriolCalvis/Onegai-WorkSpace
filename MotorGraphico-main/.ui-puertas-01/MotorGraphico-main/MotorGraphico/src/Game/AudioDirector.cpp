// H6-01 (Fase H) — La política de sonido. Las tres reglas y los motivos están
// en include/Game/AudioDirector.h; aquí solo está el cómo.

#include "Game/AudioDirector.h"

#include <cstddef>

namespace Game {
namespace {

using RPG::AudioBus;
using RPG::SoundConfig;

AudioCue musica(const char* id, const char* label, const char* file) {
    AudioCue cue;
    cue.id = id;
    cue.label = label;
    cue.file = file;
    cue.bus = AudioBus::Music;
    cue.loop = true;     // una canción que se acaba sola es un silencio que
    cue.stream = true;   // nadie pidió; y larga, se transmite, no se carga.
    cue.music = true;
    return cue;
}

AudioCue efecto(const char* id, const char* label, const char* file) {
    AudioCue cue;
    cue.id = id;
    cue.label = label;
    cue.file = file;
    cue.bus = AudioBus::SFX;
    return cue;
}

std::vector<AudioCue> construirCatalogo() {
    std::vector<AudioCue> c;

    // --- Música: una por escena en la que el jugador está un rato. Es la
    // colección mínima que pide H6-02, ni una más: cada cue de aquí es un
    // fichero que alguien tiene que producir aguas arriba.
    c.push_back(musica("musica.titulo", "Título", "musica/titulo.ogg"));
    c.push_back(musica("musica.exploracion", "Exploración", "musica/exploracion.ogg"));
    c.push_back(musica("musica.combate", "Combate", "musica/combate.ogg"));
    c.push_back(musica("musica.tienda", "Tienda", "musica/tienda.ogg"));
    // La derrota ES música y no un efecto porque GameOver es un MODO: el
    // jugador se queda ahí mirando la pantalla, y un sonido de tres segundos
    // dejaría silencio detrás.
    c.push_back(musica("musica.derrota", "Derrota", "musica/derrota.ogg"));

    // --- Efectos: momentos, no estados.
    c.push_back(efecto("sfx.interfaz", "Interfaz", "sfx/interfaz.wav"));
    c.push_back(efecto("sfx.paso", "Paso", "sfx/paso.wav"));
    c.push_back(efecto("sfx.golpe", "Golpe", "sfx/golpe.wav"));
    c.push_back(efecto("sfx.cura", "Curación", "sfx/cura.wav"));
    c.push_back(efecto("sfx.dado", "Tirada de dados", "sfx/dado.wav"));
    // La victoria va en SFX y no en Music a propósito. Ganar no es un estado:
    // al cerrar el combate el modo vuelve a Exploration y su música vuelve con
    // él. Una «música de victoria» exclusiva tendría que pararla alguien, y
    // ese alguien no existe. Como efecto suena ENCIMA de la exploración que
    // vuelve, que es lo que hace un jingle.
    c.push_back(efecto("sfx.victoria", "Victoria", "sfx/victoria.wav"));

    return c;
}

}  // namespace

const std::vector<AudioCue>& AudioCueCatalog() {
    // Estática local y no global: el orden de inicialización entre unidades de
    // traducción no está definido, y un consumidor que la lea antes no puede
    // encontrarse una lista a medio construir.
    static const std::vector<AudioCue> catalogo = construirCatalogo();
    return catalogo;
}

const AudioCue* FindAudioCue(const std::string& id) {
    for (const AudioCue& cue : AudioCueCatalog()) {
        if (cue.id == id) {
            return &cue;
        }
    }
    return nullptr;
}

const char* MusicForMode(GameMode mode) {
    switch (mode) {
        case GameMode::Exploration: return "musica.exploracion";
        case GameMode::Battle:      return "musica.combate";
        case GameMode::Shop:        return "musica.tienda";
        case GameMode::GameOver:    return "musica.derrota";
        // Dialogue, Business y Pause son capas encima de una escena que sigue
        // sonando: cortarle la música al jugador cada vez que un PNJ habla es
        // exactamente el parpadeo que la regla 1 de la cabecera prohíbe.
        case GameMode::Dialogue:
        case GameMode::Business:
        case GameMode::Pause:       return nullptr;
    }
    return nullptr;
}

void AudioDirector::bind(const std::string& audioDir) {
    // Volver a llamar bind() es legítimo (recargar assets). Se descarga lo
    // anterior primero: cargar dos veces el mismo fichero dejaría en el motor
    // un sonido que ya nadie referencia y que nadie va a descargar.
    for (const auto& entrada : m_bound) {
        m_engine.unloadSound(entrada.second);
    }
    m_bound.clear();
    m_currentMusic.clear();
    m_musicInstance = 0;

    for (const AudioCue& cue : AudioCueCatalog()) {
        SoundConfig config;
        config.filePath = audioDir.empty() ? cue.file : audioDir + "/" + cue.file;
        config.bus = cue.bus;
        config.loop = cue.loop;
        config.stream = cue.stream;
        const Result<uint32_t> cargado = m_engine.loadSound(config);
        if (!cargado.isOk()) {
            // Este es el caso normal hoy: hay CERO ficheros de audio en
            // assets/. Se dice y se sigue, que es la regla 3.
            noteProblem("audio: '" + cue.id + "' no se pudo cargar desde " + config.filePath +
                        " — " + cargado.errorMessage());
            continue;
        }
        m_bound.emplace_back(cue.id, cargado.value());
    }
}

bool AudioDirector::isBound(const std::string& cueId) const { return soundIdOf(cueId) != 0; }

uint32_t AudioDirector::soundIdOf(const std::string& cueId) const {
    for (const auto& entrada : m_bound) {
        if (entrada.first == cueId) {
            return entrada.second;
        }
    }
    return 0;
}

uint32_t AudioDirector::start(const AudioCue& cue) {
    const uint32_t soundId = soundIdOf(cue.id);
    if (soundId == 0) {
        return 0;  // sin cargar: el motivo ya está en problems() desde bind()
    }
    const Result<uint32_t> instancia = m_engine.playSound(soundId);
    if (!instancia.isOk()) {
        noteProblem("audio: '" + cue.id + "' no sonó — " + instancia.errorMessage());
        return 0;
    }
    return instancia.value();
}

void AudioDirector::playMusic(const std::string& cueId) {
    if (cueId.empty()) {
        stopMusic();
        return;
    }
    // Idempotente: pedir la que ya suena no la reinicia (regla 1).
    if (cueId == m_currentMusic && m_musicInstance != 0) {
        return;
    }
    const AudioCue* cue = FindAudioCue(cueId);
    if (cue == nullptr) {
        noteProblem("audio: id de cue desconocido '" + cueId + "'");
        return;
    }
    if (!cue->music) {
        noteProblem("audio: '" + cueId + "' no es música y no se puede poner como música");
        return;
    }
    if (!isBound(cueId)) {
        // Sin cargar: la que suena SIGUE sonando (regla 3). Quedarse con la
        // canción anterior es mejor que quedarse mudo, y el motivo ya se dijo
        // en bind().
        return;
    }
    // Se para la anterior ANTES de arrancar la nueva para que la exclusividad
    // sea cierta en todo momento: dos pistas a la vez no es una transición, es
    // un fallo. El hueco de un frame es inaudible; el solapamiento no.
    if (m_musicInstance != 0) {
        m_engine.stopInstance(m_musicInstance);
        m_musicInstance = 0;
        m_currentMusic.clear();
    }
    const uint32_t instancia = start(*cue);
    if (instancia == 0) {
        return;  // motivo anotado por start()
    }
    m_musicInstance = instancia;
    m_currentMusic = cueId;
}

void AudioDirector::stopMusic() {
    if (m_musicInstance != 0) {
        m_engine.stopInstance(m_musicInstance);
        m_musicInstance = 0;
    }
    m_currentMusic.clear();
}

void AudioDirector::onModeChanged(GameMode mode) {
    const char* cueId = MusicForMode(mode);
    if (cueId == nullptr) {
        return;  // este modo no cambia la música (regla 2)
    }
    playMusic(cueId);
}

void AudioDirector::play(const std::string& cueId) {
    const AudioCue* cue = FindAudioCue(cueId);
    if (cue == nullptr) {
        noteProblem("audio: id de cue desconocido '" + cueId + "'");
        return;
    }
    // Los efectos no tocan la música: van en su bus y se solapan entre sí.
    start(*cue);
}

float AudioDirector::scaleVolume(int optionValue) {
    const int minimo = OptionsState::kVolumeMin;
    const int maximo = OptionsState::kVolumeMax;
    if (optionValue < minimo) {
        optionValue = minimo;
    }
    if (optionValue > maximo) {
        optionValue = maximo;
    }
    // Ni 0..maximo a pelo: se resta el mínimo para que la escala siga siendo
    // cierta si algún día OptionsState no empieza en 0.
    return static_cast<float>(optionValue - minimo) / static_cast<float>(maximo - minimo);
}

void AudioDirector::applyVolumes(const OptionsValues& values) {
    m_engine.setBusVolume(AudioBus::Music, scaleVolume(values.music));
    m_engine.setBusVolume(AudioBus::SFX, scaleVolume(values.sfx));
    m_engine.setBusVolume(AudioBus::Ambience, scaleVolume(values.ambience));
}

void AudioDirector::shutdown() {
    // Solo lo que esta pieza abrió. `AudioEngine::shutdown()` es de quien
    // posee el motor, y llamarlo desde aquí cerraría el dispositivo a espaldas
    // de su dueño.
    stopMusic();
    for (const auto& entrada : m_bound) {
        m_engine.unloadSound(entrada.second);
    }
    m_bound.clear();
}

void AudioDirector::noteProblem(const std::string& message) {
    for (const std::string& existente : m_problems) {
        if (existente == message) {
            return;
        }
    }
    m_problems.push_back(message);
}

}  // namespace Game
