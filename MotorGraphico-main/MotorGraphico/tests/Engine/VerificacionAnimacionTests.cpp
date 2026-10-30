// U3-06.E07 -- Verificacion de idle / caminar / accion con EntidadClip REAL (sin ventana ni GL:
// atlas nullptr) a varios dt (1/30, 1/60, 1/144, 0,5 s), reenganche del cuerpo base tras una
// cinematica y maquina de PRUEBA de tres clips (idle, caminar, accion). Sin codigo de producto.
// Lo ya cubierto en EntidadClipMaquinaTests / ControladorAnimacionTests no se repite (ver informe).
// Lee assets/ (WORKING_DIRECTORY = fuentes).
#include "TestHarness.h"

#include <cmath>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Engine/AnimacionMundo.h"
#include "Engine/EntidadClip.h"
#include "Game/CatalogoAnimacionJuego.h"
#include "Game/RutinasPnjAnimacion.h"

using namespace Game;

namespace {
TextureAtlas* const kSinAtlas = nullptr;
const std::vector<double> kDts = {1.0 / 30.0, 1.0 / 60.0, 1.0 / 144.0, 0.5};

struct Sonda : EntidadClip {
    Sonda() : EntidadClip(GridCoord{0, 0}, kSinAtlas, 64, 32, 0.22f) {}
    int sprite() const { return m_spriteID; }
};

IdParametro par(const char* s) { return IdParametro::parse(s).value(); }
IdMaquina maqId(const char* s) { return IdMaquina::parse(s).value(); }

void enemigo(Sonda& e) {
    const CatalogoAnimacion& cat = catalogoAnimacionDelJuego();
    e.idleCuerpoBase(kSinAtlas, 64, 64, Vector2{0.0f, 0.0f});
    e.ponMaquina(&cat, cat.maquina(maqId("enemigo_base")));
}

JsonValue texto(const std::string& s) {
    auto r = JsonValue::parse(s);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

// Pasos enteros de dt que caben en `total` segundos.
long pasosDe(double dt, double total) { return std::lround(total / dt); }

// Avance de un fotograma del idle (1..12) entre dos observaciones: 0..(pasos que cabe en dt)+1.
int delta(int antes, int ahora) { return ((ahora - antes) % 12 + 12) % 12; }

// ---------------------------------------------------------------- maquina de prueba (3 clips)
const char* const kClips = R"({"schemaVersion":1,"clips":[
 {"id":"idle","bucle":true,"fotogramas":[{"sprite":1,"duracionMs":220},{"sprite":2,"duracionMs":220}],"eventos":[]},
 {"id":"caminar","bucle":true,"fotogramas":[{"sprite":3,"duracionMs":150},{"sprite":4,"duracionMs":150}],"eventos":[]},
 {"id":"golpe","bucle":false,"fotogramas":[{"sprite":5,"duracionMs":100},{"sprite":6,"duracionMs":100}],"eventos":[]}]})";
const char* const kEstados = R"({"schemaVersion":1,"maquinas":[
 {"id":"tres","inicial":"reposo",
  "parametros":[{"id":"andando","tipo":"booleano"},{"id":"golpear","tipo":"disparo"}],
  "estados":[{"id":"reposo","clip":"idle"},{"id":"andar","clip":"caminar"},{"id":"accion","clip":"golpe"}],
  "transiciones":[
   {"desde":"reposo","hacia":"andar","politica":"reiniciarSiempre","cuando":[{"param":"andando","op":"cierto"}]},
   {"desde":"andar","hacia":"reposo","politica":"reiniciarSiempre","cuando":[{"param":"andando","op":"falso"}]},
   {"desde":"reposo","hacia":"accion","politica":"reiniciarSiempre","cuando":[{"param":"golpear","op":"disparado"}]},
   {"desde":"accion","hacia":"reposo","politica":"reiniciarSiempre","esperaFinClip":true,"cuando":[]}]}]})";

CatalogoAnimacion catalogoTres() {
    auto r = construyeCatalogoAnimacion({texto(kClips)}, {texto(kEstados)});
    MOTOR_CHECK(r.isOk());
    return std::move(r.value());
}

// Clase de un sprite de la maquina de prueba: 0 idle, 1 caminar, 2 golpe.
int clase(int s) { return s <= 2 ? 0 : (s <= 4 ? 1 : 2); }

// Guion en segundos: [0,1) quieto, [1,2) andando, [2,3) quieto, golpear en el fotograma de t=3, luego quieto.
std::vector<int> corridaTres(const CatalogoAnimacion& cat, double dt, int* golpeFotogramas) {
    Sonda e;
    e.ponMaquina(&cat, cat.maquina(maqId("tres")));
    std::vector<int> seq;
    bool disparado = false;
    int enGolpe = 0;
    for (long i = 0, n = pasosDe(dt, 6.0); i < n; ++i) {
        const double t = static_cast<double>(i) * dt;
        EntradasAnimacion en;
        en.valores[par("andando")] = (t >= 1.0 - 1e-9 && t < 2.0 - 1e-9) ? 1.0 : 0.0;
        if (!disparado && t >= 3.0 - 1e-9) {
            en.valores[par("golpear")] = 1.0;  // el productor lo arma UN fotograma y lo limpia
            disparado = true;
        }
        MOTOR_CHECK(e.aplicaEntradas(en, false));
        e.update(static_cast<float>(dt));
        const int c = clase(e.sprite());
        if (seq.empty() || seq.back() != c) seq.push_back(c);
        if (c == 2) ++enGolpe;
    }
    if (golpeFotogramas != nullptr) *golpeFotogramas = enGolpe;
    return seq;
}

// Destino espia con finClip controlable (para los disparos consumidos).
struct Espia final : DestinoAnimacion {
    bool fin = false, control = false;
    int reengancha_ = 0;
    bool esClip(const IdClip&) const override { return true; }
    void ponNombre(const IdClip&) override {}
    void cambiaClip(const IdClip&, PoliticaCambio) override {}
    void reengancha(const IdClip&) override { ++reengancha_; control = true; }
    bool tieneControl() const override { return control; }
    bool finClip() const override { return fin; }
};
}  // namespace

// ---------------------------------------------------------------------------- (1) varios FPS

MOTOR_TEST_CASE(pnj_basico_misma_secuencia_de_clips_a_los_cuatro_dt) {
    // Guion por tramos de 1,5 s (rumbo, andando): la secuencia de clips que emite la maquina con
    // EntidadClip real debe ser la del oraculo RutinasPnj::animacion y la MISMA a todos los dt.
    const int rumbos[8] = {2, 1, 3, 3, 0, 0, 2, 2};
    const bool anda[8] = {false, true, true, false, true, false, true, false};
    auto estado = [&](int tramo) {
        EstadoPnj p;
        p.rumbo = rumbos[tramo];
        p.progreso = anda[tramo] ? 0.5f : 1.0f;
        return p;
    };
    std::vector<std::string> referencia;
    for (int t = 0; t < 8; ++t) {
        const std::string a = RutinasPnj::animacion(estado(t), false);
        if (referencia.empty() || referencia.back() != a) referencia.push_back(a);
    }
    MOTOR_CHECK(referencia.size() >= 7u);
    for (double dt : kDts) {
        Sonda e;
        e.idleCuerpoBase(kSinAtlas, 64, 64, Vector2{0.0f, 0.0f});
        for (const char* n : {"idle_up", "idle_right", "idle_down", "idle_left", "walk_up", "walk_right",
                              "walk_down", "walk_left"})
            e.addAnimation(n, {7});
        e.addAnimation("idle", {1, 2});
        std::vector<std::string> vista;
        for (long i = 0, n = pasosDe(dt, 12.0); i < n; ++i) {
            const int tramo = static_cast<int>(static_cast<double>(i) * dt / 1.5 + 1e-6);
            const EstadoPnj p = estado(tramo);
            if (tramo == 5 && i % 7 == 3) e.play("idle");  // cue ajena: LEGACY la repara en el mismo fotograma
            MOTOR_CHECK(Engine::animaConMaquina(e, "pnj_basico", entradasDePnj(p, false), false));
            MOTOR_CHECK(e.currentAnimation() == RutinasPnj::animacion(p, false));  // cada fotograma
            e.update(static_cast<float>(dt));
            if (vista.empty() || vista.back() != e.currentAnimation()) vista.push_back(e.currentAnimation());
        }
        MOTOR_CHECK(vista == referencia);
        MOTOR_CHECK(!e.tieneClip());  // LEGACY: los PNJ no pasan a clip
    }
}

MOTOR_TEST_CASE(idle_de_enemigo_base_mismo_fotograma_final_y_sin_saltos_a_los_cuatro_dt) {
    for (double dt : kDts) {
        Sonda e;
        enemigo(e);
        int antes = 0, saltos = 0;
        const int maxDelta = static_cast<int>(dt / 0.22) + 1;
        for (long i = 0, n = pasosDe(dt, 6.0); i < n; ++i) {
            MOTOR_CHECK(e.aplicaEntradas({}, false));
            e.update(static_cast<float>(dt));
            if (antes != 0 && delta(antes, e.sprite()) > maxDelta) ++saltos;  // un reinicio a 1 se vera aqui
            antes = e.sprite();
        }
        MOTOR_CHECK(saltos == 0);
        MOTOR_CHECK(e.tieneClip() && e.controlExterno());
        MOTOR_CHECK(e.sprite() == 4);  // 6,0 s / 0,22 s = 27,27 -> fotograma 27 mod 12 = 3 -> sprite 4
    }
}

// ----------------------------------------------------------- (2)(2b')(3) reenganche, 600 fotogramas

MOTOR_TEST_CASE(enemigo_base_cue_600_fotogramas_vuelve_al_clip_una_sola_vez) {
    Sonda e;
    enemigo(e);
    e.addAnimation("idle_left", {9});
    e.aplicaEntradas({}, false);
    e.update(0.5f);
    e.play("idle_left");  // la cue de la cinematica
    int reenganches = 0;
    for (int i = 0; i < 600; ++i) {  // cinematica en curso: el bucle de enemigos NO corre (!enCurso)
        const bool conClip = e.tieneClip();
        e.update(1.0f / 60.0f);
        if (!conClip && e.tieneClip()) ++reenganches;
        MOTOR_CHECK(!e.tieneClip() && e.sprite() == 9);  // la pose, todo el rato
    }
    MOTOR_CHECK(reenganches == 0);
    e.aplicaEntradas({}, false);  // acaba la cinematica
    MOTOR_CHECK(e.tieneClip() && e.controlExterno() && e.sprite() == 1);
    int antes = 1, saltos = 0, vueltas = 0;
    for (int i = 0; i < 600; ++i) {  // 10 s de juego normal: sin segundo reenganche ni eventos duplicados
        e.aplicaEntradas({}, false);
        e.update(1.0f / 60.0f);
        if (delta(antes, e.sprite()) > 1) ++saltos;
        if (e.sprite() == 1 && antes == 12) ++vueltas;
        antes = e.sprite();
    }
    MOTOR_CHECK(saltos == 0);
    MOTOR_CHECK(vueltas == 3);  // 10 s / 2,64 s = 3,79 vueltas completas
    MOTOR_CHECK(e.sprite() == 10);  // 10 s / 0,22 = 45,45 -> 45 mod 12 = 9 -> sprite 10
}

MOTOR_TEST_CASE(cinematica_con_los_idle_registrados_pose_toda_la_cinematica_y_un_reenganche) {  // (2b')
    Sonda e;
    enemigo(e);
    for (const char* n : {"idle_up", "idle_right", "idle_down", "idle_left"}) e.addAnimation(n, {9});  // S10
    e.aplicaEntradas({}, false);
    int reenganches = 0, plays = 0;
    for (int i = 0; i < 600; ++i) {  // ApplicationCinematica: play solo si currentAnimation() != animacion
        if (e.currentAnimation() != "idle_left") {
            e.play("idle_left");
            ++plays;
        }
        const bool conClip = e.tieneClip();
        e.update(1.0f / 60.0f);
        if (!conClip && e.tieneClip()) ++reenganches;
        MOTOR_CHECK(!e.tieneClip() && e.sprite() == 9 && e.currentAnimation() == "idle_left");
    }
    MOTOR_CHECK(plays == 1 && reenganches == 0);
    e.aplicaEntradas({}, false);
    MOTOR_CHECK(e.tieneClip() && e.controlExterno() && e.sprite() == 1 && e.currentAnimation().empty());
    for (int i = 0; i < 600; ++i) {
        const bool conClip = e.tieneClip();
        e.aplicaEntradas({}, false);
        e.update(1.0f / 60.0f);
        MOTOR_CHECK(conClip && e.tieneClip());  // nunca se pierde el clip: ningun otro reenganche
    }
}

MOTOR_TEST_CASE(estado_igual_no_reinicia_en_600_fotogramas_a_cada_dt) {  // (3)
    for (double dt : kDts) {
        Sonda e;
        enemigo(e);
        int antes = 0, reinicios = 0;
        const int maxDelta = static_cast<int>(dt / 0.22) + 1;
        for (int i = 0; i < 600; ++i) {
            e.aplicaEntradas({}, false);
            e.update(static_cast<float>(dt));
            if (antes != 0 && delta(antes, e.sprite()) > maxDelta) ++reinicios;  // un reinicio no cabe en un paso
            antes = e.sprite();
        }
        MOTOR_CHECK(reinicios == 0);
    }
}

MOTOR_TEST_CASE(h2_al_acabar_la_ruta_el_idle_salta_de_fase_una_vez) {  // efecto conocido (decision Brain: se mantiene)
    Sonda e;
    enemigo(e);
    e.addAnimation("walk_left", {9});  // si estuviera registrado, el play de la ruta le quitaria el control
    e.aplicaEntradas({}, false);
    e.update(0.5f);
    MOTOR_CHECK(e.sprite() == 3);
    for (int i = 0; i < 30; ++i) {  // ruta de combate sin play efectivo: la maquina no escribe
        e.update(0.0f);
        e.aplicaEntradas({}, true);
        MOTOR_CHECK(e.tieneClip() && e.controlExterno() && e.sprite() == 3);
    }
    e.aplicaEntradas({}, false);  // reanuda() fuerza UN reengancha aunque no se perdiera el control
    MOTOR_CHECK(e.sprite() == 1);  // salto de fase (3 -> 1): lo que vera Oriol
    e.update(0.23f);
    MOTOR_CHECK(e.sprite() == 2);  // y desde ahi, normal
    e.aplicaEntradas({}, false);
    MOTOR_CHECK(e.sprite() == 2);  // una sola vez
}

// ------------------------------------------------------------------------------- (4) accion

MOTOR_TEST_CASE(maquina_de_tres_clips_misma_secuencia_idle_caminar_idle_accion_idle_a_los_cuatro_dt) {
    const CatalogoAnimacion cat = catalogoTres();
    for (double dt : kDts) {
        int enGolpe = 0;
        const std::vector<int> seq = corridaTres(cat, dt, &enGolpe);
        MOTOR_CHECK((seq == std::vector<int>{0, 1, 0, 2, 0}));
        if (dt < 0.1) {
            // esperaFinClip: el golpe (200 ms) dura 200 ms, no un fotograma. 12 pasos de 1/60 (+-2).
            const int esperado = static_cast<int>(std::lround(0.2 / dt));
            MOTOR_CHECK(enGolpe >= esperado - 1 && enGolpe <= esperado + 2);
        } else {
            MOTOR_CHECK(enGolpe >= 1);  // dt 0,5 s: el clip entero cabe en un paso, pero se entrega
        }
    }
}

MOTOR_TEST_CASE(accion_se_entrega_ya_y_termina_con_el_clip_no_ciclico) {
    const CatalogoAnimacion cat = catalogoTres();
    Sonda e;
    e.ponMaquina(&cat, cat.maquina(maqId("tres")));
    e.aplicaEntradas({}, false);
    MOTOR_CHECK(e.sprite() == 1);
    EntradasAnimacion golpe;
    golpe.valores[par("golpear")] = 1.0;
    e.aplicaEntradas(golpe, false);
    MOTOR_CHECK(e.sprite() == 5);  // reposo -> accion: ya, sin update
    e.update(0.05f);
    e.aplicaEntradas({}, false);
    MOTOR_CHECK(e.sprite() == 5);  // esperaFinClip: a 50 ms sigue en la accion
    e.update(0.07f);               // 120 ms -> segundo fotograma
    e.aplicaEntradas({}, false);
    MOTOR_CHECK(e.sprite() == 6);
    e.update(0.2f);                // clip terminado
    e.aplicaEntradas({}, false);   // finClip -> reposo
    MOTOR_CHECK(e.sprite() == 1);
    for (int i = 0; i < 100; ++i) {  // sin disparo armado no vuelve a golpear
        e.aplicaEntradas({}, false);
        e.update(0.016f);
        MOTOR_CHECK(e.sprite() <= 2);
    }
}

MOTOR_TEST_CASE(ruta_de_combate_suspende_la_maquina_de_tres_clips_y_el_disparo_no_se_pierde_ni_se_gasta) {
    const CatalogoAnimacion cat = catalogoTres();
    Sonda e;
    e.ponMaquina(&cat, cat.maquina(maqId("tres")));
    e.aplicaEntradas({}, false);
    EntradasAnimacion golpe;
    golpe.valores[par("golpear")] = 1.0;
    for (int i = 0; i < 10; ++i) {  // otro propietario manda: el disparo armado no se atiende
        e.update(0.016f);
        e.aplicaEntradas(golpe, true);
        MOTOR_CHECK(e.sprite() <= 2);
    }
    e.aplicaEntradas(golpe, false);  // acaba la ruta: reemision unica y ahora si atiende el disparo
    MOTOR_CHECK(e.sprite() == 5);
}

MOTOR_TEST_CASE(el_disparo_golpear_se_consume_una_sola_vez) {
    const CatalogoAnimacion cat = catalogoTres();
    ControladorAnimacion c(cat.maquina(maqId("tres")));
    Espia d;
    std::vector<IdParametro> consumidos;
    EntradasAnimacion en;
    c.aplica(en, d, &consumidos);  // primer aplica (reengancha)
    MOTOR_CHECK(consumidos.empty());
    en.valores[par("golpear")] = 1.0;
    MOTOR_CHECK(c.aplica(en, d, &consumidos));
    MOTOR_CHECK(consumidos.size() == 1u && consumidos[0] == par("golpear"));
    MOTOR_CHECK(c.vivo().estado.value() == "accion");
    for (int i = 0; i < 5; ++i) {  // el disparo sigue armado (el productor no lo limpio): no se re-consume
        c.aplica(en, d, &consumidos);
        MOTOR_CHECK(consumidos.empty() && c.vivo().estado.value() == "accion");
    }
    d.fin = true;  // acaba el clip no ciclico
    en.valores.clear();
    MOTOR_CHECK(c.aplica(en, d, &consumidos));
    MOTOR_CHECK(c.vivo().estado.value() == "reposo" && consumidos.empty());
}

int main() { return motor_test::runAllTests(); }
