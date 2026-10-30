// H6-01 (Fase H) — La política de sonido: qué suena en cada momento.
//
// Por qué con un DOBLE de AudioEngine y no con el de verdad. `RPG::AudioEngine`
// es una interfaz pura y `createAudioEngine()` devuelve la implementación
// miniaudio, que abre un dispositivo: en CI no hay tarjeta de sonido, y aunque
// la hubiera, lo que se prueba aquí no es que miniaudio reproduzca —eso ya lo
// cubre `test_audio`— sino QUÉ se le pide y en qué orden. El doble graba las
// llamadas y deja afirmar sobre la política: que la música es exclusiva, que no
// se reinicia, que un efecto no la corta, que lo que falta no rompe nada.
//
// Los casos 5 y 6 son el criterio de cierre de H6-01 tal como está escrito en
// JUGABILIDAD_FASE_H.md: «al entrar en combate suena la música de combate; al
// golpear, un efecto; al salir, vuelve la de exploración».

#include "Game/AudioDirector.h"

#include "TestHarness.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace {

using Game::AudioCue;
using Game::AudioCueCatalog;
using Game::AudioDirector;
using Game::FindAudioCue;
using Game::MusicForMode;
using Game::OptionsValues;
using RPG::AudioBus;
using RPG::SoundConfig;
using RPG::SoundInstanceState;

// ---------------------------------------------------------------------------
// El doble
// ---------------------------------------------------------------------------

class MotorFalso : public RPG::AudioEngine {
public:
    // Qué ficheros están. Con `existeTodo` la carga siempre va bien; si no,
    // solo los de `disponibles`. Es lo que permite probar la regla 3 (lo que
    // falta no rompe nada) sin tocar el disco.
    bool existeTodo = true;
    std::set<std::string> disponibles;
    bool playFalla = false;

    // Lo que se le pidió, en orden.
    std::vector<SoundConfig> cargas;
    std::vector<uint32_t> descargas;
    std::vector<uint32_t> reproducidos;   // soundId de cada play
    std::vector<uint32_t> parados;        // instanceId de cada stop
    std::map<AudioBus, float> volumenes;
    int shutdowns = 0;

    ::Result<bool> initialize() override { return ::Result<bool>::Ok(true); }
    void shutdown() override { ++shutdowns; }
    void update() override {}

    ::Result<uint32_t> loadSound(const SoundConfig& config) override {
        cargas.push_back(config);
        if (!existeTodo && disponibles.count(config.filePath) == 0) {
            return ::Result<uint32_t>::Error("no existe " + config.filePath);
        }
        const uint32_t id = ++m_ultimoSound;
        m_rutaDe[id] = config.filePath;
        return ::Result<uint32_t>::Ok(id);
    }

    void unloadSound(uint32_t soundId) override {
        descargas.push_back(soundId);
        m_rutaDe.erase(soundId);
    }

    ::Result<uint32_t> playSound(uint32_t soundId, const RPG::SoundEndCallback&) override {
        if (playFalla) {
            return ::Result<uint32_t>::Error("sin dispositivo");
        }
        if (m_rutaDe.count(soundId) == 0) {
            return ::Result<uint32_t>::Error("soundId no cargado");
        }
        reproducidos.push_back(soundId);
        const uint32_t instancia = ++m_ultimaInstancia;
        m_vivas.insert(instancia);
        m_soundDe[instancia] = soundId;
        return ::Result<uint32_t>::Ok(instancia);
    }

    void stopInstance(uint32_t instanceId) override {
        parados.push_back(instanceId);
        m_vivas.erase(instanceId);
    }
    void pauseInstance(uint32_t, bool) override {}
    void setInstanceVolume(uint32_t, float) override {}
    void setInstancePitch(uint32_t, float) override {}
    void setInstancePan(uint32_t, float) override {}

    ::Result<SoundInstanceState> getInstanceState(uint32_t instanceId) const override {
        if (m_vivas.count(instanceId) == 0) {
            return ::Result<SoundInstanceState>::Error("instancia no activa");
        }
        return ::Result<SoundInstanceState>::Ok(SoundInstanceState{});
    }

    void setBusVolume(AudioBus bus, float volume) override { volumenes[bus] = volume; }
    float getBusVolume(AudioBus bus) const override {
        const auto it = volumenes.find(bus);
        return it == volumenes.end() ? 1.0f : it->second;
    }
    void setBusMuted(AudioBus, bool) override {}
    bool isBusMuted(AudioBus) const override { return false; }
    void setMasterVolume(float v) override { master = v; }
    float getMasterVolume() const override { return master; }
    size_t getActiveInstanceCount() const override { return m_vivas.size(); }
    void cleanupFinishedInstances() override {}

    // --- Lecturas para las aserciones ---
    bool vive(uint32_t instanceId) const { return m_vivas.count(instanceId) != 0; }
    uint32_t soundDe(uint32_t instanceId) const {
        const auto it = m_soundDe.find(instanceId);
        return it == m_soundDe.end() ? 0 : it->second;
    }
    // soundId de un cue por su ruta, para no depender del orden de carga.
    uint32_t soundDeRuta(const std::string& ruta) const {
        for (const auto& par : m_rutaDe) {
            if (par.second == ruta) {
                return par.first;
            }
        }
        return 0;
    }
    std::size_t cargasDe(const std::string& ruta) const {
        std::size_t n = 0;
        for (const SoundConfig& c : cargas) {
            if (c.filePath == ruta) {
                ++n;
            }
        }
        return n;
    }

private:
    uint32_t m_ultimoSound = 0;
    uint32_t m_ultimaInstancia = 100;
    float master = 1.0f;
    std::map<uint32_t, std::string> m_rutaDe;
    std::map<uint32_t, uint32_t> m_soundDe;
    std::set<uint32_t> m_vivas;
};

// Director + motor, juntos, que es como viven en la partida.
struct Escena {
    MotorFalso motor;
    AudioDirector director{motor};
};

std::size_t cuesEnBus(AudioBus bus) {
    std::size_t n = 0;
    for (const AudioCue& cue : AudioCueCatalog()) {
        if (cue.bus == bus) {
            ++n;
        }
    }
    return n;
}

bool hayProblemaCon(const AudioDirector& d, const std::string& trozo) {
    for (const std::string& p : d.problems()) {
        if (p.find(trozo) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// =======================================================================
// 1. El catálogo: el contrato de lo que H6-02 tiene que producir
// =======================================================================

MOTOR_TEST_CASE(el_catalogo_cubre_la_coleccion_minima_de_H6_02) {
    // H6-02 pide: título, exploración, combate, tienda, victoria, derrota +
    // SFX de interfaz, pasos, golpe, cura, dado. Ni uno más: cada cue es un
    // fichero que alguien tiene que producir aguas arriba.
    for (const char* id : {"musica.titulo", "musica.exploracion", "musica.combate",
                           "musica.tienda", "musica.derrota", "sfx.interfaz", "sfx.paso",
                           "sfx.golpe", "sfx.cura", "sfx.dado", "sfx.victoria"}) {
        if (FindAudioCue(id) == nullptr) {
            MOTOR_CHECK_FAIL(std::string("falta el cue ") + id);
        }
    }
    MOTOR_CHECK(AudioCueCatalog().size() == 11);
    MOTOR_CHECK(cuesEnBus(AudioBus::Music) == 5);
    MOTOR_CHECK(cuesEnBus(AudioBus::SFX) == 6);
}

MOTOR_TEST_CASE(el_catalogo_no_tiene_ids_ni_rutas_repetidas) {
    // Dos cues con el mismo id son dos nombres para una cosa; dos con la misma
    // ruta cargan el mismo fichero dos veces y ocupan el doble.
    for (std::size_t i = 0; i < AudioCueCatalog().size(); ++i) {
        const AudioCue& a = AudioCueCatalog()[i];
        MOTOR_CHECK(!a.id.empty());
        MOTOR_CHECK(!a.file.empty());
        MOTOR_CHECK(!a.label.empty());
        // Una ruta relativa: absoluta decidiría aquí dónde viven los assets.
        MOTOR_CHECK(a.file[0] != '/');
        for (std::size_t j = i + 1; j < AudioCueCatalog().size(); ++j) {
            const AudioCue& b = AudioCueCatalog()[j];
            MOTOR_CHECK(a.id != b.id);
            MOTOR_CHECK(a.file != b.file);
        }
    }
}

MOTOR_TEST_CASE(toda_musica_es_exclusiva_en_bucle_y_los_efectos_no) {
    // `music` es lo que distingue «poner la otra canción» de «sumar una
    // encima». Una música sin loop se acabaría sola y dejaría un silencio que
    // nadie pidió.
    for (const AudioCue& cue : AudioCueCatalog()) {
        if (cue.bus == AudioBus::Music) {
            MOTOR_CHECK(cue.music);
            MOTOR_CHECK(cue.loop);
            MOTOR_CHECK(cue.stream);
        } else {
            MOTOR_CHECK(!cue.music);
            MOTOR_CHECK(!cue.loop);
        }
    }
}

MOTOR_TEST_CASE(cada_modo_que_no_es_capa_tiene_su_musica) {
    MOTOR_CHECK(MusicForMode(GameMode::Exploration) == std::string("musica.exploracion"));
    MOTOR_CHECK(MusicForMode(GameMode::Battle) == std::string("musica.combate"));
    MOTOR_CHECK(MusicForMode(GameMode::Shop) == std::string("musica.tienda"));
    MOTOR_CHECK(MusicForMode(GameMode::GameOver) == std::string("musica.derrota"));
    // Dialogue, Business y Pause son capas encima de una escena que sigue
    // sonando (regla 2).
    MOTOR_CHECK(MusicForMode(GameMode::Dialogue) == nullptr);
    MOTOR_CHECK(MusicForMode(GameMode::Business) == nullptr);
    MOTOR_CHECK(MusicForMode(GameMode::Pause) == nullptr);
    // Y toda música que nombra un modo existe de verdad en el catálogo: un
    // id mal escrito aquí sería un silencio que nadie sabe explicar.
    for (const GameMode modo : {GameMode::Exploration, GameMode::Battle, GameMode::Shop,
                                 GameMode::GameOver}) {
        MOTOR_CHECK(FindAudioCue(MusicForMode(modo)) != nullptr);
    }
}

// =======================================================================
// 2. La carga: quién pone la ruta y qué pasa cuando no hay nada
// =======================================================================

MOTOR_TEST_CASE(bind_con_assets_carga_el_catalogo_entero_una_sola_vez) {
    Escena e;
    e.director.bind("assets/audio");

    MOTOR_CHECK(e.director.problems().empty());
    MOTOR_CHECK(e.motor.cargas.size() == AudioCueCatalog().size());
    for (const AudioCue& cue : AudioCueCatalog()) {
        MOTOR_CHECK(e.director.isBound(cue.id));
        MOTOR_CHECK(e.motor.cargasDe("assets/audio/" + cue.file) == 1);
    }
    // El bus y el loop los pone el catálogo, no quien cablea.
    for (const SoundConfig& c : e.motor.cargas) {
        const bool esMusica = c.filePath.find("musica/") != std::string::npos;
        MOTOR_CHECK(c.bus == (esMusica ? AudioBus::Music : AudioBus::SFX));
        MOTOR_CHECK(c.loop == esMusica);
        MOTOR_CHECK(c.stream == esMusica);
    }
}

MOTOR_TEST_CASE(la_ruta_la_pone_quien_cablea_no_la_pieza) {
    // Misma ley que OptionsFile (H6-03) y SaveSlotState (G-11): el directorio
    // no se inventa dentro.
    Escena conRaiz;
    conRaiz.director.bind("assets/audio");
    MOTOR_CHECK(conRaiz.motor.cargasDe("assets/audio/musica/combate.ogg") == 1);

    Escena sinRaiz;
    sinRaiz.director.bind("");
    MOTOR_CHECK(sinRaiz.motor.cargasDe("musica/combate.ogg") == 1);
}

MOTOR_TEST_CASE(bind_sin_ningun_asset_no_rompe_nada_y_lista_lo_que_falta) {
    // Este es el estado REAL del motor hoy: cero ficheros en assets/audio
    // (H6-02). El juego tiene que arrancar en silencio y decirlo, no fallar.
    Escena e;
    e.motor.existeTodo = false;
    e.director.bind("assets/audio");

    MOTOR_CHECK(e.director.problems().size() == AudioCueCatalog().size());
    for (const AudioCue& cue : AudioCueCatalog()) {
        MOTOR_CHECK(!e.director.isBound(cue.id));
    }
    // Y la partida sigue: ni una sola reproducción, ni un fallo.
    e.director.onModeChanged(GameMode::Battle);
    e.director.play("sfx.golpe");
    MOTOR_CHECK(e.motor.reproducidos.empty());
    MOTOR_CHECK(e.director.currentMusic().empty());
}

MOTOR_TEST_CASE(lo_que_falta_es_la_lista_de_la_compra_de_H6_02) {
    // El diagnóstico nombra la RUTA, que es lo que necesita quien produce el
    // contenido: «hacen falta assets de audio» no se puede encargar.
    Escena e;
    e.motor.existeTodo = false;
    e.director.bind("assets/audio");

    for (const AudioCue& cue : AudioCueCatalog()) {
        if (!hayProblemaCon(e.director, "assets/audio/" + cue.file)) {
            MOTOR_CHECK_FAIL("el diagnóstico no nombra " + cue.file);
        }
    }
}

MOTOR_TEST_CASE(volver_a_bind_no_deja_sonidos_huerfanos) {
    // Recargar assets es legítimo. Sin descargar lo anterior quedaría en el
    // motor un sonido que ya nadie referencia y que nadie va a descargar.
    Escena e;
    e.director.bind("assets/audio");
    const std::size_t primeraVuelta = e.motor.cargas.size();
    e.director.bind("assets/audio");

    MOTOR_CHECK(e.motor.descargas.size() == primeraVuelta);
    MOTOR_CHECK(e.director.problems().empty());
}

// =======================================================================
// 3. El criterio de cierre de H6-01
// =======================================================================

MOTOR_TEST_CASE(al_entrar_en_combate_suena_la_musica_de_combate) {
    Escena e;
    e.director.bind("assets/audio");
    e.director.onModeChanged(GameMode::Exploration);

    e.director.onModeChanged(GameMode::Battle);

    MOTOR_CHECK(e.director.currentMusic() == "musica.combate");
    MOTOR_CHECK(e.motor.reproducidos.size() == 2);
    MOTOR_CHECK(e.motor.reproducidos.back() ==
                e.motor.soundDeRuta("assets/audio/musica/combate.ogg"));
    // La de exploración queda parada: exclusiva quiere decir una.
    MOTOR_CHECK(e.motor.parados.size() == 1);
    MOTOR_CHECK(!e.motor.vive(e.motor.parados.front()));
}

MOTOR_TEST_CASE(al_golpear_suena_un_efecto_y_al_salir_vuelve_la_exploracion) {
    // El criterio literal, entero y en orden.
    Escena e;
    e.director.bind("assets/audio");

    e.director.onModeChanged(GameMode::Exploration);
    const uint32_t exploracion = e.motor.reproducidos.back();

    e.director.onModeChanged(GameMode::Battle);
    e.director.play("sfx.golpe");

    // El efecto suena y NO corta la música de combate.
    MOTOR_CHECK(e.motor.reproducidos.back() == e.motor.soundDeRuta("assets/audio/sfx/golpe.wav"));
    MOTOR_CHECK(e.director.currentMusic() == "musica.combate");

    e.director.onModeChanged(GameMode::Exploration);

    MOTOR_CHECK(e.director.currentMusic() == "musica.exploracion");
    MOTOR_CHECK(e.motor.reproducidos.back() == exploracion ||
                e.motor.reproducidos.back() ==
                    e.motor.soundDeRuta("assets/audio/musica/exploracion.ogg"));
    // Y sigue habiendo una instancia viva: la música que ha vuelto.
    MOTOR_CHECK(e.motor.getActiveInstanceCount() > 0);
}

MOTOR_TEST_CASE(game_over_pone_la_musica_de_derrota) {
    Escena e;
    e.director.bind("assets/audio");
    e.director.onModeChanged(GameMode::Battle);
    e.director.onModeChanged(GameMode::GameOver);

    MOTOR_CHECK(e.director.currentMusic() == "musica.derrota");
    // Y la victoria es un jingle, no un estado: no toca la música.
    e.director.play("sfx.victoria");
    MOTOR_CHECK(e.director.currentMusic() == "musica.derrota");
}

MOTOR_TEST_CASE(la_pantalla_de_titulo_tiene_musica_sin_ser_un_modo) {
    // El título vive antes de que exista sesión, así que no hay GameMode que
    // lo nombre. Por eso playMusic() es público y no solo interno.
    Escena e;
    e.director.bind("assets/audio");
    e.director.playMusic("musica.titulo");
    MOTOR_CHECK(e.director.currentMusic() == "musica.titulo");

    // Y al empezar la partida, la música del título deja paso a la otra.
    e.director.onModeChanged(GameMode::Exploration);
    MOTOR_CHECK(e.director.currentMusic() == "musica.exploracion");
    MOTOR_CHECK(e.motor.parados.size() == 1);
}

// =======================================================================
// 4. Las reglas que impiden que la música sea insoportable
// =======================================================================

MOTOR_TEST_CASE(la_musica_que_ya_suena_no_se_reinicia) {
    // Sin esto, Exploration -> Battle -> Exploration en cada combate, o un
    // bucle que repite el modo cada frame, cortarían la canción cada vez.
    Escena e;
    e.director.bind("assets/audio");

    e.director.onModeChanged(GameMode::Battle);
    for (int i = 0; i < 50; ++i) {
        e.director.onModeChanged(GameMode::Battle);
    }

    MOTOR_CHECK(e.motor.reproducidos.size() == 1);
    MOTOR_CHECK(e.motor.parados.empty());
}

MOTOR_TEST_CASE(dialogo_negocio_y_pausa_no_cortan_la_musica) {
    Escena e;
    e.director.bind("assets/audio");
    e.director.onModeChanged(GameMode::Exploration);
    const uint32_t antes = e.motor.reproducidos.size();

    e.director.onModeChanged(GameMode::Dialogue);
    e.director.onModeChanged(GameMode::Business);
    e.director.onModeChanged(GameMode::Pause);
    e.director.onModeChanged(GameMode::Dialogue);

    MOTOR_CHECK(e.motor.reproducidos.size() == antes);
    MOTOR_CHECK(e.motor.parados.empty());
    MOTOR_CHECK(e.director.currentMusic() == "musica.exploracion");
}

MOTOR_TEST_CASE(un_efecto_no_toca_la_musica) {
    Escena e;
    e.director.bind("assets/audio");
    e.director.onModeChanged(GameMode::Exploration);
    MOTOR_CHECK(e.motor.parados.empty());

    e.director.play("sfx.paso");
    e.director.play("sfx.interfaz");
    e.director.play("sfx.dado");

    // Tres efectos nuevos, la música intacta y ninguna parada.
    MOTOR_CHECK(e.motor.reproducidos.size() == 4);
    MOTOR_CHECK(e.motor.parados.empty());
    MOTOR_CHECK(e.director.currentMusic() == "musica.exploracion");
}

MOTOR_TEST_CASE(una_musica_sin_cargar_deja_sonar_la_anterior) {
    // Regla 3, segunda mitad: quedarse con la canción anterior es mejor que
    // quedarse mudo, y el motivo ya está en problems().
    Escena e;
    e.motor.existeTodo = false;
    e.motor.disponibles.insert("assets/audio/musica/exploracion.ogg");
    e.director.bind("assets/audio");
    e.director.onModeChanged(GameMode::Exploration);
    MOTOR_CHECK(e.director.currentMusic() == "musica.exploracion");

    e.director.onModeChanged(GameMode::Battle);   // combate no está cargado

    MOTOR_CHECK(e.director.currentMusic() == "musica.exploracion");
    MOTOR_CHECK(e.motor.parados.empty());
    MOTOR_CHECK(hayProblemaCon(e.director, "musica.combate"));
}

MOTOR_TEST_CASE(stop_music_deja_el_juego_en_silencio) {
    Escena e;
    e.director.bind("assets/audio");
    e.director.onModeChanged(GameMode::Exploration);

    e.director.stopMusic();

    MOTOR_CHECK(e.director.currentMusic().empty());
    MOTOR_CHECK(e.motor.parados.size() == 1);
    // Y volver a pedir la misma ya no es «la que suena»: arranca de nuevo.
    e.director.onModeChanged(GameMode::Exploration);
    MOTOR_CHECK(e.motor.reproducidos.size() == 2);
}

// =======================================================================
// 5. Lo que se dice cuando algo está mal escrito
// =======================================================================

MOTOR_TEST_CASE(un_id_desconocido_se_dice_en_vez_de_callarse) {
    Escena e;
    e.director.bind("assets/audio");

    e.director.play("sfx.inventado");
    e.director.playMusic("musica.que_no_existe");

    MOTOR_CHECK(hayProblemaCon(e.director, "sfx.inventado"));
    MOTOR_CHECK(hayProblemaCon(e.director, "musica.que_no_existe"));
    MOTOR_CHECK(e.motor.reproducidos.empty());
}

MOTOR_TEST_CASE(un_efecto_no_se_puede_poner_como_musica) {
    // Si colara, sonaría en bucle un golpe de tres segundos y la música
    // exclusiva quedaría ocupada por un efecto.
    Escena e;
    e.director.bind("assets/audio");
    e.director.onModeChanged(GameMode::Exploration);

    e.director.playMusic("sfx.golpe");

    MOTOR_CHECK(e.director.currentMusic() == "musica.exploracion");
    MOTOR_CHECK(hayProblemaCon(e.director, "no es música"));
    MOTOR_CHECK(e.motor.reproducidos.size() == 1);
}

MOTOR_TEST_CASE(un_modo_que_parpadea_no_llena_el_diagnostico) {
    // problems() se escribe una vez por motivo. Sin deduplicar, una partida
    // con el asset que falta acabaría con millones de líneas idénticas.
    Escena e;
    e.motor.existeTodo = false;
    e.motor.disponibles.insert("assets/audio/musica/exploracion.ogg");
    e.director.bind("assets/audio");
    const std::size_t trasBind = e.director.problems().size();

    for (int i = 0; i < 200; ++i) {
        e.director.onModeChanged(GameMode::Battle);
        e.director.play("sfx.inventado");
        e.director.onModeChanged(GameMode::Exploration);
    }

    MOTOR_CHECK(e.director.problems().size() == trasBind + 1);
}

MOTOR_TEST_CASE(si_el_motor_no_puede_reproducir_se_dice) {
    Escena e;
    e.director.bind("assets/audio");
    e.motor.playFalla = true;

    e.director.onModeChanged(GameMode::Exploration);

    MOTOR_CHECK(e.director.currentMusic().empty());
    MOTOR_CHECK(hayProblemaCon(e.director, "no sonó"));
}

// =======================================================================
// 6. El puente con las opciones del jugador (H6-03)
// =======================================================================

MOTOR_TEST_CASE(la_escala_de_volumen_es_la_de_options_state) {
    MOTOR_CHECK(AudioDirector::scaleVolume(Game::OptionsState::kVolumeMin) == 0.0f);
    MOTOR_CHECK(AudioDirector::scaleVolume(Game::OptionsState::kVolumeMax) == 1.0f);
    MOTOR_CHECK(AudioDirector::scaleVolume(5) == 0.5f);
    // Fuera de rango se recorta: el motor también recorta, pero recortarlo aquí
    // es lo que hace que la escala tenga una sola fuente.
    MOTOR_CHECK(AudioDirector::scaleVolume(-5) == 0.0f);
    MOTOR_CHECK(AudioDirector::scaleVolume(99) == 1.0f);
}

MOTOR_TEST_CASE(los_volumenes_del_jugador_llegan_al_motor_por_bus) {
    // Sin este puente el criterio de H6-03 («bajar el volumen de efectos SE
    // NOTA AL INSTANTE») no se puede cumplir aunque el fichero de opciones
    // funcione: OptionsValues habla en 0..10 y AudioEngine en 0..1.
    Escena e;
    OptionsValues valores;
    valores.music = 8;
    valores.sfx = 0;
    valores.ambience = 10;

    e.director.applyVolumes(valores);

    MOTOR_CHECK(e.motor.getBusVolume(AudioBus::Music) == 0.8f);
    MOTOR_CHECK(e.motor.getBusVolume(AudioBus::SFX) == 0.0f);
    MOTOR_CHECK(e.motor.getBusVolume(AudioBus::Ambience) == 1.0f);
    // Master no está en OptionsValues: tocarlo sería un valor que el jugador
    // no puede ni guardar ni recuperar.
    MOTOR_CHECK(e.motor.getMasterVolume() == 1.0f);
}

MOTOR_TEST_CASE(bajar_la_musica_en_opciones_se_nota_sin_reiniciar_nada) {
    // El caso de usuario entero: suena la exploración, el jugador baja la
    // música a 3, la canción SIGUE sonando (más baja) y no se corta.
    Escena e;
    e.director.bind("assets/audio");
    e.director.onModeChanged(GameMode::Exploration);
    const std::size_t reproducciones = e.motor.reproducidos.size();

    OptionsValues valores;
    valores.music = 3;
    valores.sfx = 8;
    valores.ambience = 8;
    e.director.applyVolumes(valores);

    MOTOR_CHECK(e.motor.getBusVolume(AudioBus::Music) == 0.3f);
    MOTOR_CHECK(e.motor.reproducidos.size() == reproducciones);
    MOTOR_CHECK(e.motor.parados.empty());
    MOTOR_CHECK(e.director.currentMusic() == "musica.exploracion");
}

// =======================================================================
// 7. El cierre
// =======================================================================

MOTOR_TEST_CASE(shutdown_para_la_musica_y_descarga_sin_cerrar_el_motor) {
    // El dispositivo es de quien lo abrió. Cerrarlo desde aquí lo cerraría a
    // espaldas de su dueño, que aún tiene que pintar un frame.
    Escena e;
    e.director.bind("assets/audio");
    e.director.onModeChanged(GameMode::Exploration);
    const std::size_t cargados = e.motor.cargas.size();

    e.director.shutdown();

    MOTOR_CHECK(e.motor.shutdowns == 0);
    MOTOR_CHECK(e.motor.parados.size() == 1);
    MOTOR_CHECK(e.motor.descargas.size() == cargados);
    MOTOR_CHECK(e.director.currentMusic().empty());
    MOTOR_CHECK(!e.director.isBound("musica.exploracion"));
}

}  // namespace

int main() { return motor_test::runAllTests(); }
