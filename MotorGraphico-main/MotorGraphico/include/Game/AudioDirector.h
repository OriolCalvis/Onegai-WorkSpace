#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "Game/OptionsFile.h"
#include "Game/SessionTipos.h"
#include "RPG/Audio.h"

// H6-01 (Fase H) — La POLÍTICA de sonido: qué suena en cada momento del juego.
//
// QUÉ FALTABA. GDB-11 dejó `RPG::AudioEngine` hecho y probado: interfaz
// abstracta, `AudioEngineImpl` sobre miniaudio real, buses, loop, streaming,
// volumen por bus y callbacks, con `test_audio` en verde. Y **ningún ejecutable
// lo llama**. En `planificacion/MAPA_DEL_CODIGO.md` sus tres ficheros
// (`include/RPG/Audio.h`, `src/RPG/Audio.cpp`, `tests/RPG/AudioTests.cpp`) caen
// en «§3 Ficheros sin paquete asignado»: sin paquete no hay criterio de cierre,
// y sin criterio nadie los cablea. El motor sabe cómo sonar y no sabe cuándo.
//
// Esta pieza es el cuándo. No reproduce nada por sí misma: traduce lo que pasa
// en la partida —un cambio de modo, un golpe, el jugador bajando la música— a
// llamadas a un `AudioEngine` que le pasan por referencia, y que en las pruebas
// es un doble. Por eso es GL-free y tampoco necesita dispositivo de audio.
//
// QUÉ NO HACE.
//
//  - No carga ficheros ni sabe de formatos: eso es `AudioEngine::loadSound`.
//    Aquí solo hay ids, rutas y buses.
//  - No inventa el directorio de audio. `bind()` recibe la ruta de quien
//    cablea, que es la misma ley que siguen `OptionsFile` (H6-03) y
//    `SaveSlotState` (G-11): la pieza no decide dónde viven las cosas del
//    jugador ni los assets.
//  - No gestiona volúmenes por instancia, ni pausas de instancia, ni
//    callbacks de fin. Están en el motor y quien los necesite los llama;
//    duplicarlos aquí sería tener dos sitios donde decidir lo mismo.
//
// LAS TRES REGLAS QUE SÍ DECIDE.
//
//  1. LA MÚSICA ES EXCLUSIVA E IDEMPOTENTE. Suena una y solo una, y pedir la
//     que ya suena no la reinicia. Sin lo segundo, cualquier parpadeo de modo
//     —Exploration → Dialogue → Exploration al hablar con un PNJ— cortaría la
//     canción cada vez, que es exactamente lo que hace insoportable un juego
//     con música.
//  2. UN MODO QUE NO TIENE MÚSICA NO LA CAMBIA. `MusicForMode` devuelve
//     nullptr para Dialogue, Business y Pause: son capas encima de una escena
//     que sigue sonando. Que Pause no baje el volumen es una decisión
//     SIN TOMAR —el motor tiene `setBusVolume` y sería una línea—, no un
//     olvido; hacer ducking aquí sin que Oriol lo haya pedido sería decidir
//     por él cómo suena su juego.
//  3. LO QUE FALTA NO ROMPE NADA. Hoy hay **cero ficheros de audio** en
//     `assets/` (H6-02). Un cue sin cargar se anota en `problems()` y se
//     calla: el juego arranca en silencio y lo dice, en vez de fallar. Es lo
//     que permite cablear H6-01 ANTES de que H6-02 exista, y convierte «hacen
//     falta assets de audio» en una lista concreta de rutas que se puede
//     comprobar con un test. Y si una música no se puede cargar, la que suena
//     SIGUE sonando: quedarse con la canción anterior es mejor que quedarse
//     mudo, y el motivo ya está en problems().
//
// LOS DOS HUÉRFANOS QUE CONECTA. `Game::OptionsValues` (H6-03) guarda música,
// efectos y ambiente como enteros 0..10, que es la escala que pinta
// `OptionsState`. `AudioEngine` quiere flotantes 0..1 por bus. Nadie hacía el
// puente, así que el criterio de H6-03 —«bajar el volumen de efectos SE NOTA
// AL INSTANTE»— no se podía cumplir aunque el fichero de opciones funcionara.
// `applyVolumes()` es ese puente, y `scaleVolume()` la escala escrita una sola
// vez.
//
// UNA INCOHERENCIA QUE QUEDA DICHA Y NO SE ARREGLA AQUÍ: el catálogo no tiene
// ningún cue en el bus Ambience, porque H6-02 no lista ninguna ambientación.
// Eso significa que la fila AMBIENTE de la pantalla de Opciones no controla
// nada hasta que exista contenido. No se ha inventado una ambientación para
// taparlo: el flujo es `canon → cartas → motor` y el contenido no se fabrica
// en el motor. Está en la bandeja de Oriol.

namespace Game {

// Un sonido del juego. `id` es estable y es lo que escribe quien dispara;
// `label` es lo que se le enseñaría al autor y es traducible (G-17).
struct AudioCue {
    std::string id;
    std::string label;
    // Ruta RELATIVA al directorio que recibe bind(). Absoluta sería decidir
    // aquí dónde viven los assets, que es de quien cablea.
    std::string file;
    RPG::AudioBus bus = RPG::AudioBus::SFX;
    bool loop = false;
    bool stream = false;
    // Exclusiva: solo suena una a la vez. Es lo que distingue «poner la otra
    // canción» de «sumar una canción encima».
    bool music = false;
};

// El catálogo entero, en orden estable. Es LA respuesta a «¿qué sonidos
// necesita el juego?», en un solo sitio y con el mismo papel que
// `RPG::CommandCatalog()` en E-04: de aquí salen la carga en el arranque, la
// lista de lo que H6-02 tiene que producir aguas arriba, y lo que un test
// puede comprobar. Si cada consumidor llevara su lista, el juego pediría
// sonidos que el catálogo no conoce.
const std::vector<AudioCue>& AudioCueCatalog();

// nullptr si ese id no está en el catálogo. Un id desconocido NO es un fallo
// de carga sino un error de quien dispara, y se dice (ver problems()).
const AudioCue* FindAudioCue(const std::string& id);

// Qué música corresponde a cada modo de la partida. nullptr = este modo no
// cambia la música (regla 2 de la cabecera).
const char* MusicForMode(GameMode mode);

class AudioDirector {
public:
    // El motor no es suyo: lo pasa quien lo creó en el arranque y lo destruye
    // al cerrar. En producción es `RPG::createAudioEngine()`; en las pruebas,
    // un doble que graba las llamadas.
    explicit AudioDirector(RPG::AudioEngine& engine) : m_engine(engine) {}

    // Carga el catálogo entero desde `audioDir`. Se llama una vez, después de
    // `AudioEngine::initialize()`. Los cues cuyo fichero no esté se quedan sin
    // cargar y van a problems(); el resto funciona igual.
    void bind(const std::string& audioDir);

    // La música del modo. Idempotente (regla 1).
    void onModeChanged(GameMode mode);

    // Música explícita, para lo que no es un GameMode: la pantalla de título
    // vive antes de que exista sesión. `playMusic("")` equivale a stopMusic().
    void playMusic(const std::string& cueId);
    void stopMusic();

    // Un disparo puntual: un golpe, una cura, el dado, el cursor del menú. Los
    // efectos se solapan entre sí y NO tocan la música.
    void play(const std::string& cueId);

    // Los volúmenes del jugador, de la escala de OptionsState a la del motor.
    // Toca Music, SFX y Ambience; Master no está en OptionsValues y no se
    // toca — inventarse un volumen maestro que el jugador no puede guardar
    // sería un valor que nadie controla.
    void applyVolumes(const OptionsValues& values);

    // Cierra lo que esta pieza abrió: la instancia de música viva. NO llama a
    // `AudioEngine::shutdown()`, que es de quien posee el motor.
    void shutdown();

    // Qué no se pudo cargar y qué id se pidió sin existir. Es a la vez el
    // diagnóstico y la lista de la compra de H6-02. Sin duplicados: un modo
    // que parpadea no puede llenarlo.
    const std::vector<std::string>& problems() const { return m_problems; }

    // ¿Ese cue está cargado y se puede disparar?
    bool isBound(const std::string& cueId) const;
    // La música que suena ahora (id de cue), o vacío si ninguna.
    const std::string& currentMusic() const { return m_currentMusic; }

    // OptionsState::kVolumeMin..kVolumeMax -> 0.0..1.0. Pública porque la
    // pantalla de Opciones quiere enseñar el mismo número que va a sonar, y
    // porque la escala escrita dos veces acaba siendo dos escalas.
    static float scaleVolume(int optionValue);

private:
    // Arranca un cue ya cargado. 0 = no se pudo (y el motivo va a problems()).
    uint32_t start(const AudioCue& cue);
    // Id de sonido que el motor devolvió al cargar ese cue, o 0 si no se cargó.
    uint32_t soundIdOf(const std::string& cueId) const;
    // Sin duplicados: un modo que parpadea 60 veces por segundo no puede
    // convertir el diagnóstico en una lista infinita.
    void noteProblem(const std::string& message);

    RPG::AudioEngine& m_engine;
    // id de cue -> id de sonido del motor. Lo que no está aquí, no suena.
    std::vector<std::pair<std::string, uint32_t>> m_bound;
    std::string m_currentMusic;
    uint32_t m_musicInstance = 0;
    std::vector<std::string> m_problems;
};

}  // namespace Game
