// U3-06.E06b -- la maquina de animacion integrada: EntidadClip REAL (sin ventana ni GL: atlas
// nullptr), catalogo del juego, catalogo tolerante y el bucle (lectura textual). Lee assets/ y
// src/ (WORKING_DIRECTORY = fuentes).
#include "TestHarness.h"

#include <fstream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Engine/AnimacionMundo.h"
#include "Engine/EntidadClip.h"
#include "Engine/MaquinaEntidad.h"
#include "Game/CatalogoAnimacionJuego.h"

using namespace Game;

namespace {
TextureAtlas* const kSinAtlas = nullptr;

struct Sonda : EntidadClip {
    Sonda() : EntidadClip(GridCoord{0, 0}, kSinAtlas, 64, 32, 0.22f) {}
    int sprite() const { return m_spriteID; }
};

IdParametro par(const char* s) { return IdParametro::parse(s).value(); }
IdMaquina maqId(const char* s) { return IdMaquina::parse(s).value(); }

EntradasAnimacion pnj(double andando, double rumbo) {
    EntradasAnimacion e;
    e.valores[par("andando")] = andando;
    e.valores[par("rumbo")] = rumbo;
    return e;
}

JsonValue texto(const std::string& s) {
    auto r = JsonValue::parse(s);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

std::string leeFichero(const std::string& ruta) {
    std::ifstream f(ruta, std::ios::binary);
    MOTOR_CHECK(f.good());
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Un cuerpo base con la maquina `enemigo_base` ya puesta y el primer aplica hecho.
void enemigo(Sonda& e) {
    const CatalogoAnimacion& cat = catalogoAnimacionDelJuego();
    e.idleCuerpoBase(kSinAtlas, 64, 64, Vector2{0.0f, 0.0f});
    e.ponMaquina(&cat, cat.maquina(maqId("enemigo_base")));
}

// Cuenta llamadas al destino (para la ruta de combate).
struct Cuenta final : DestinoAnimacion {
    int llamadas = 0, reenganches = 0;
    bool control = true;
    bool esClip(const IdClip&) const override { return true; }
    void ponNombre(const IdClip&) override { ++llamadas; }
    void cambiaClip(const IdClip&, PoliticaCambio) override { ++llamadas; }
    void reengancha(const IdClip&) override { ++llamadas; ++reenganches; control = true; }
    bool tieneControl() const override { return control; }
    bool finClip() const override { return false; }
};

// Documento de clips y de estados para las pruebas de finClip y CLIP->LEGACY->CLIP.
const char* const kClips = R"({"schemaVersion":1,"clips":[
 {"id":"idle","bucle":true,"fotogramas":[{"sprite":1,"duracionMs":220},{"sprite":2,"duracionMs":220},{"sprite":3,"duracionMs":220},{"sprite":4,"duracionMs":220}],"eventos":[]},
 {"id":"cicl","bucle":true,"fotogramas":[{"sprite":1,"duracionMs":100},{"sprite":2,"duracionMs":100}],"eventos":[]},
 {"id":"golpe","bucle":false,"fotogramas":[{"sprite":5,"duracionMs":100},{"sprite":6,"duracionMs":100}],"eventos":[]}]})";
const char* const kEstados = R"({"schemaVersion":1,"maquinas":[
 {"id":"ciclico","inicial":"a","parametros":[],
  "estados":[{"id":"a","clip":"cicl"},{"id":"b","clip":"golpe"}],
  "transiciones":[{"desde":"a","hacia":"b","esperaFinClip":true,"cuando":[{"param":"finClip","op":"cierto"}]}]},
 {"id":"accion","inicial":"a","parametros":[],
  "estados":[{"id":"a","clip":"golpe"},{"id":"b","clip":"cicl"}],
  "transiciones":[{"desde":"a","hacia":"b","esperaFinClip":true,"cuando":[{"param":"finClip","op":"cierto"}]}]},
 {"id":"legacy","inicial":"a","parametros":[{"id":"go","tipo":"booleano"}],
  "estados":[{"id":"a","clip":"idle"},{"id":"b","clip":"idle_left"}],
  "transiciones":[
   {"desde":"a","hacia":"b","politica":"reiniciarSiDistinto","cuando":[{"param":"go","op":"cierto"}]},
   {"desde":"b","hacia":"a","politica":"reiniciarSiDistinto","cuando":[{"param":"go","op":"falso"}]}]}]})";

CatalogoAnimacion catalogoDePrueba() {
    auto r = construyeCatalogoAnimacion({texto(kClips)}, {texto(kEstados)});
    MOTOR_CHECK(r.isOk());
    return std::move(r.value());
}

// Quita comentarios (// y /* */) respetando cadenas y caracteres.
std::string sinComentarios(const std::string& s) {
    std::string o;
    for (std::size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        if (c == '"' || c == '\'') {
            o += c;
            for (++i; i < s.size() && s[i] != c; ++i) {
                o += s[i];
                if (s[i] == '\\' && i + 1 < s.size()) o += s[++i];
            }
            if (i < s.size()) o += s[i];
        } else if (c == '/' && i + 1 < s.size() && s[i + 1] == '/') {
            while (i < s.size() && s[i] != '\n') ++i;
            o += '\n';
        } else if (c == '/' && i + 1 < s.size() && s[i + 1] == '*') {
            i += 2;
            while (i + 1 < s.size() && !(s[i] == '*' && s[i + 1] == '/')) ++i;
            ++i;
        } else {
            o += c;
        }
    }
    return o;
}
}  // namespace

// ---------------------------------------------------------------------------------- catalogo

MOTOR_TEST_CASE(catalogo_del_juego_tiene_clip_y_las_dos_maquinas) {
    const CatalogoAnimacion& cat = catalogoAnimacionDelJuego();
    MOTOR_CHECK(cat.clip(IdClip::parse("idle").value()) != nullptr);
    MOTOR_CHECK(cat.maquina(maqId("pnj_basico")) != nullptr);
    const MaquinaAnimacion* e = cat.maquina(maqId("enemigo_base"));
    MOTOR_CHECK(e != nullptr);
    MOTOR_CHECK(e->estados.size() == 1u && e->transiciones.empty());
}

MOTOR_TEST_CASE(catalogo_no_se_copia_tras_tomar_punteros) {  // (d)
    const CatalogoAnimacion* a = &catalogoAnimacionDelJuego();
    const MaquinaAnimacion* m = a->maquina(maqId("enemigo_base"));
    Sonda e;
    enemigo(e);
    MOTOR_CHECK(&catalogoAnimacionDelJuego() == a);
    MOTOR_CHECK(catalogoAnimacionDelJuego().maquina(maqId("enemigo_base")) == m);
}

MOTOR_TEST_CASE(tolerante_con_clips_rotos_avisa_una_vez_y_conserva_las_maquinas) {  // (c)
    std::ostringstream avisos;
    const JsonValue estados = texto(leeFichero("assets/animaciones/enemigo_base.estados.json"));
    const CatalogoAnimacion cat =
        construyeCatalogoTolerante({texto(R"({"schemaVersion":9,"clips":[]})")}, {estados}, avisos);
    MOTOR_CHECK(cat.clips.empty());
    MOTOR_CHECK(cat.maquina(maqId("enemigo_base")) != nullptr);  // idle es nombre legacy: valida
    int lineas = 0;
    for (char c : avisos.str()) lineas += c == '\n' ? 1 : 0;
    MOTOR_CHECK(lineas == 1);
}

MOTOR_TEST_CASE(tolerante_con_una_maquina_rota_solo_esa_no_existe) {  // (c)
    std::ostringstream avisos;
    const JsonValue buena = texto(leeFichero("assets/animaciones/enemigo_base.estados.json"));
    const JsonValue rota = texto(R"({"schemaVersion":1,"maquinas":[{"id":"rota","inicial":"x","parametros":[],
        "estados":[{"id":"x","clip":"no_existe_nunca"}],"transiciones":[]}]})");
    const JsonValue pnjDoc = texto(leeFichero("assets/animaciones/pnj_basico.estados.json"));
    const CatalogoAnimacion cat = construyeCatalogoTolerante(
        {texto(leeFichero("assets/animaciones/idle_cuerpo_base.clips.json"))}, {buena, rota, pnjDoc}, avisos);
    MOTOR_CHECK(cat.maquina(maqId("enemigo_base")) != nullptr);
    MOTOR_CHECK(cat.maquina(maqId("pnj_basico")) != nullptr);
    MOTOR_CHECK(cat.maquina(maqId("rota")) == nullptr);
    MOTOR_CHECK(cat.clip(IdClip::parse("idle").value()) != nullptr);
    int lineas = 0;
    for (char c : avisos.str()) lineas += c == '\n' ? 1 : 0;
    MOTOR_CHECK(lineas == 1);
}

// ---------------------------------------------------------------------- reenganche (EntidadClip real)

MOTOR_TEST_CASE(entrega_inicial_sprite_correcto_tras_aplica_sin_update) {  // D-T8
    Sonda e;
    enemigo(e);
    e.addAnimation("idle_left", {9});
    e.play("idle_left");
    e.update(0.01f);
    MOTOR_CHECK(e.sprite() == 9 && !e.tieneClip());
    MOTOR_CHECK(e.aplicaEntradas({}, false));
    MOTOR_CHECK(e.sprite() == 1);  // sin update
}

MOTOR_TEST_CASE(reenganche_play_update_aplica_update) {
    Sonda e;
    enemigo(e);
    e.addAnimation("idle_left", {9});
    e.aplicaEntradas({}, false);  // primer aplica
    e.play("idle_left");
    e.update(0.01f);
    MOTOR_CHECK(!e.tieneClip() && e.sprite() == 9);
    e.aplicaEntradas({}, false);
    MOTOR_CHECK(e.tieneClip() && e.controlExterno() && e.sprite() == 1);
    e.update(0.23f);
    MOTOR_CHECK(e.tieneClip() && e.sprite() == 2);  // el clip avanza, no se destruye
    e.update(0.23f);
    MOTOR_CHECK(e.sprite() == 3);  // un fotograma por paso: nada duplicado ni saltado
}

MOTOR_TEST_CASE(reenganche_play_aplica_update_sin_update_intermedio) {
    Sonda e;
    enemigo(e);
    e.addAnimation("idle_left", {9});
    e.aplicaEntradas({}, false);
    e.play("idle_left");  // el clip sigue vivo pero ya sin control
    MOTOR_CHECK(e.tieneClip() && !e.controlExterno());
    e.aplicaEntradas({}, false);
    MOTOR_CHECK(e.tieneClip() && e.controlExterno() && e.sprite() == 1);
    e.update(0.23f);
    MOTOR_CHECK(e.tieneClip() && e.sprite() == 2);
}

MOTOR_TEST_CASE(play_de_un_nombre_no_registrado_no_pierde_el_control) {
    Sonda e;
    enemigo(e);
    e.aplicaEntradas({}, false);
    e.update(0.5f);
    const int antes = e.sprite();
    MOTOR_CHECK(antes == 3);
    e.play("walk_left");  // no registrado: se ignora
    MOTOR_CHECK(e.tieneClip() && e.controlExterno());
    e.aplicaEntradas({}, false);
    MOTOR_CHECK(e.sprite() == antes);  // sin reenganche espurio (no reinicio a 1)
}

MOTOR_TEST_CASE(reanuda_cada_fotograma_sin_ruta_no_reemite) {
    Sonda e;
    enemigo(e);
    e.aplicaEntradas({}, false);
    e.update(0.5f);
    for (int i = 0; i < 50; ++i) {
        e.aplicaEntradas({}, false);
        e.update(0.0f);
    }
    MOTOR_CHECK(e.sprite() == 3);
}

// ---------------------------------------------------------------- ruta de combate (D-T7)

MOTOR_TEST_CASE(ruta_activa_cero_llamadas_y_al_acabar_una_reemision) {
    const CatalogoAnimacion cat = catalogoDePrueba();
    ControladorAnimacion c(cat.maquina(maqId("legacy")));
    Cuenta d;
    Engine::aplicaConRuta(c, {}, d, false);  // primer aplica: reemite
    const int base = d.llamadas;
    for (int i = 0; i < 20; ++i) Engine::aplicaConRuta(c, {}, d, true);
    MOTOR_CHECK(d.llamadas == base);  // 0 llamadas con ruta activa
    Engine::aplicaConRuta(c, {}, d, false);
    MOTOR_CHECK(d.llamadas == base + 1 && d.reenganches == 2);  // ruta acabada: 1 reemision
    for (int i = 0; i < 20; ++i) Engine::aplicaConRuta(c, {}, d, false);
    MOTOR_CHECK(d.llamadas == base + 1);  // reanuda() cada fotograma no reemite
}

MOTOR_TEST_CASE(ruta_de_combate_con_entidad_real_no_toca_y_reengancha_una_vez) {
    Sonda e;
    enemigo(e);
    e.addAnimation("idle_left", {9});
    e.aplicaEntradas({}, false);
    for (int i = 0; i < 30; ++i) {  // ApplicationCinematica::combate hace play() cada fotograma
        e.play("idle_left");
        e.update(0.016f);
        e.aplicaEntradas({}, true);
    }
    MOTOR_CHECK(!e.tieneClip() && e.sprite() == 9);  // la maquina no escribio
    e.update(0.016f);
    e.aplicaEntradas({}, false);
    MOTOR_CHECK(e.tieneClip() && e.controlExterno() && e.sprite() == 1);
    e.update(0.23f);
    MOTOR_CHECK(e.sprite() == 2);
}

MOTOR_TEST_CASE(hayRutaDeCombate_por_handle) {
    Engine::RutasDeCombate rutas;
    const Engine::EntityHandle a{1, 1}, b{2, 1};
    MOTOR_CHECK(!Engine::hayRutaDeCombate(rutas, a));
    rutas.emplace_back(a, Game::PasosEnRuta{});
    MOTOR_CHECK(Engine::hayRutaDeCombate(rutas, a));
    MOTOR_CHECK(!Engine::hayRutaDeCombate(rutas, b));
    MOTOR_CHECK(!Engine::tieneRutaDeCombate(nullptr, a));
}

// ---------------------------------------------------------------------- PNJ (LEGACY)

MOTOR_TEST_CASE(pnj_basico_play_cada_fotograma_autorrepara_tras_la_cinematica) {
    Sonda e;
    e.idleCuerpoBase(kSinAtlas, 64, 64, Vector2{0.0f, 0.0f});
    for (const char* n : {"idle_down", "idle_left", "walk_left"}) e.addAnimation(n, {7});
    e.addAnimation("idle", {1, 2});
    MOTOR_CHECK(Engine::animaConMaquina(e, "pnj_basico", pnj(0, 2), false));
    MOTOR_CHECK(e.currentAnimation() == "idle_down");
    e.play("idle");  // cue «idle» de la cinematica
    MOTOR_CHECK(e.currentAnimation() == "idle");
    for (int i = 0; i < 600; ++i) {
        e.update(1.0f / 60.0f);
        MOTOR_CHECK(Engine::animaConMaquina(e, "pnj_basico", pnj(0, 2), false));
    }
    MOTOR_CHECK(e.currentAnimation() == "idle_down");  // como hoy
    MOTOR_CHECK(!e.tieneClip());                       // LEGACY: los PNJ no pasan a clip
}

MOTOR_TEST_CASE(actor_sin_maquina_comportamiento_intacto) {
    Sonda e;
    e.idleCuerpoBase(kSinAtlas, 64, 64, Vector2{0.0f, 0.0f});
    MOTOR_CHECK(!e.aplicaEntradas(pnj(0, 2), false));
    MOTOR_CHECK(e.tieneClip() && e.controlExterno() && e.sprite() == 1);
    MOTOR_CHECK(!Engine::animaConMaquina(e, "no_existe_la_maquina", pnj(0, 2), false));
    MOTOR_CHECK(e.tieneClip() && e.controlExterno() && e.currentAnimation().empty());
    e.update(0.23f);
    MOTOR_CHECK(e.sprite() == 2);
    MOTOR_CHECK(e.esCuerpoBase());
    Sonda otro;
    MOTOR_CHECK(!otro.esCuerpoBase());
}

MOTOR_TEST_CASE(esCuerpoBase_sobrevive_a_la_cinematica) {
    Sonda e;
    enemigo(e);
    e.addAnimation("idle_left", {9});
    e.play("idle_left");
    e.update(0.01f);
    MOTOR_CHECK(!e.tieneClip() && e.esCuerpoBase());
}

// ------------------------------------------------------- obligaciones heredadas de E06a r1

MOTOR_TEST_CASE(finClip_con_clip_ciclico_real_no_dispara) {  // (a)
    const CatalogoAnimacion cat = catalogoDePrueba();
    Sonda e;
    e.ponMaquina(&cat, cat.maquina(maqId("ciclico")));
    for (int i = 0; i < 400; ++i) {
        e.update(0.05f);
        e.aplicaEntradas({}, false);
        MOTOR_CHECK(e.sprite() == 1 || e.sprite() == 2);  // jamas llega a b (golpe: 5/6)
    }
}

MOTOR_TEST_CASE(finClip_con_clip_no_ciclico_real_dispara_una_vez) {
    const CatalogoAnimacion cat = catalogoDePrueba();
    Sonda e;
    e.ponMaquina(&cat, cat.maquina(maqId("accion")));
    e.aplicaEntradas({}, false);
    MOTOR_CHECK(e.sprite() == 5);
    std::set<int> vistos;
    for (int i = 0; i < 20; ++i) {
        e.update(0.05f);
        e.aplicaEntradas({}, false);
        vistos.insert(e.sprite());
    }
    MOTOR_CHECK(vistos.count(6) == 1u);
    MOTOR_CHECK(e.sprite() == 1 || e.sprite() == 2);  // golpe -> ciclo por finClip
}

MOTOR_TEST_CASE(clip_legacy_clip_con_un_play_ignorado_no_reinicia) {  // (b)
    const CatalogoAnimacion cat = catalogoDePrueba();
    Sonda e;
    e.ponMaquina(&cat, cat.maquina(maqId("legacy")));
    EntradasAnimacion go0, go1;
    go0.valores[par("go")] = 0;
    go1.valores[par("go")] = 1;
    e.aplicaEntradas(go0, false);  // CLIP idle (reengancha)
    e.update(0.5f);
    MOTOR_CHECK(e.sprite() == 3);
    e.aplicaEntradas(go1, false);  // LEGACY idle_left: no registrado -> play ignorado
    MOTOR_CHECK(e.tieneClip() && e.controlExterno() && e.sprite() == 3);
    e.aplicaEntradas(go0, false);  // vuelta a CLIP con el mismo clip: ReiniciarSiDistinto no reinicia
    MOTOR_CHECK(e.tieneClip() && e.controlExterno() && e.sprite() == 3);
    e.update(0.23f);
    MOTOR_CHECK(e.sprite() == 4);
}

// --------------------------------------------------------------- el bucle (lectura textual)

MOTOR_TEST_CASE(bucle_de_enemigos_dentro_de_no_enCurso_y_con_ruta_de_combate) {  // mutacion (c)
    const std::string src = sinComentarios(leeFichero("src/Engine/ApplicationBucle.cpp"));
    const std::string guarda = "if (!ApplicationCinematica::enCurso(*this)) {";
    const std::size_t ini = src.find(guarda);
    MOTOR_CHECK(ini != std::string::npos);
    MOTOR_CHECK(src.find(guarda, ini + 1) == std::string::npos);
    int prof = 0;
    std::size_t fin = std::string::npos;
    for (std::size_t i = ini + guarda.size() - 1; i < src.size(); ++i) {
        if (src[i] == '{') ++prof;
        if (src[i] == '}' && --prof == 0) {
            fin = i;
            break;
        }
    }
    MOTOR_CHECK(fin != std::string::npos);
    const std::string dentro = src.substr(ini, fin - ini);
    const std::string fuera = src.substr(0, ini) + src.substr(fin);
    for (const char* clave : {"\"enemigo_base\"", "\"pnj_basico\"", "tieneRutaDeCombate", "esCuerpoBase"}) {
        MOTOR_CHECK(dentro.find(clave) != std::string::npos);
        MOTOR_CHECK(fuera.find(clave) == std::string::npos);
    }
    // El registro de nombres S8 se mantiene y va antes de aplicar la maquina.
    const std::size_t reg = dentro.find("pnj->dibujoListo = true;");
    MOTOR_CHECK(reg != std::string::npos && reg < dentro.find("\"pnj_basico\""));
    // El play() de siempre solo como politica sin maquina.
    MOTOR_CHECK(dentro.find("actor->play(Game::RutinasPnj::animacion(*pnj, reducidas))") != std::string::npos);
}

int main() { return motor_test::runAllTests(); }
