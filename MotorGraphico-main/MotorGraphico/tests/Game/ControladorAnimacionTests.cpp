// U3-06.E06a -- Game::ControladorAnimacion + Game::CatalogoAnimacion con un destino falso que
// registra llamadas. Sin GL. Lee assets/animaciones/*.json (WORKING_DIRECTORY = fuentes).
#include "TestHarness.h"

#include "Game/CatalogoAnimacion.h"
#include "Game/ControladorAnimacion.h"

#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace Game;

namespace {

IdClip clipId(const std::string& s) { return IdClip::parse(s).value(); }
IdParametro par(const std::string& s) { return IdParametro::parse(s).value(); }

JsonValue leeJson(const std::string& ruta) {
    std::ifstream f(ruta);
    MOTOR_CHECK(f.good());
    std::stringstream ss;
    ss << f.rdbuf();
    auto doc = JsonValue::parse(ss.str());
    MOTOR_CHECK(doc.isOk());
    return doc.value();
}

JsonValue textoJson(const std::string& s) {
    auto doc = JsonValue::parse(s);
    MOTOR_CHECK(doc.isOk());
    return doc.value();
}

CatalogoAnimacion catalogoReal() {
    auto r = construyeCatalogoAnimacion({leeJson("assets/animaciones/idle_cuerpo_base.clips.json")},
                                        {leeJson("assets/animaciones/pnj_basico.estados.json")});
    MOTOR_CHECK(r.isOk());
    return r.value();
}

// Destino falso: registra cada llamada como texto. `clips` = ids que son CLIP. reengancha deja el
// control puesto (como ponClip); `perderControl()` simula un play() ajeno o un update que lo suelta.
class Falso final : public DestinoAnimacion {
public:
    std::set<std::string> clips;
    std::vector<std::string> llamadas;
    PoliticaCambio ultimaPolitica = PoliticaCambio::ReiniciarSiDistinto;
    bool control = false;
    bool fin = false;

    bool esClip(const IdClip& id) const override { return clips.count(id.value()) > 0; }
    void ponNombre(const IdClip& id) override { llamadas.push_back("nombre:" + id.value()); }
    void cambiaClip(const IdClip& id, PoliticaCambio p) override {
        ultimaPolitica = p;
        llamadas.push_back("cambia:" + id.value());
    }
    void reengancha(const IdClip& id) override {
        control = true;
        llamadas.push_back("reengancha:" + id.value());
    }
    bool tieneControl() const override { return control; }
    bool finClip() const override { return fin; }
    void perderControl() { control = false; }
};

EntradasAnimacion entradas(double andando, double rumbo) {
    EntradasAnimacion e;
    e.valores[par("andando")] = andando;
    e.valores[par("rumbo")] = rumbo;
    return e;
}

// Maquina de dos estados con disparo y finClip: a (idle, cada rumbo igual) --finClip--> b;
// a --disparo "salta"--> b (reiniciarSiempre).
const char* const kJsonFin = R"({"schemaVersion":1,"maquinas":[{"id":"m","inicial":"a",
 "parametros":[{"id":"salta","tipo":"disparo"}],
 "estados":[{"id":"a","clip":"idle"},{"id":"b","clip":"idle_up"}],
 "transiciones":[{"desde":"a","hacia":"b","esperaFinClip":true,"cuando":[{"param":"finClip","op":"cierto"}]}]}]})";
const char* const kJsonDisparo = R"({"schemaVersion":1,"maquinas":[{"id":"m","inicial":"a",
 "parametros":[{"id":"salta","tipo":"disparo"}],
 "estados":[{"id":"a","clip":"idle"},{"id":"b","clip":"idle_up"}],
 "transiciones":[{"desde":"a","hacia":"b","politica":"conservarFase","cuando":[{"param":"salta","op":"disparado"}]}]}]})";

const char* const kJsonSoloMismoClip = R"({"schemaVersion":1,"maquinas":[{"id":"m","inicial":"a",
 "parametros":[{"id":"go","tipo":"booleano"}],
 "estados":[{"id":"a","clip":"idle"},{"id":"b","clip":"idle"}],
 "transiciones":[{"desde":"a","hacia":"b","politica":"reiniciarSiDistinto","cuando":[{"param":"go","op":"cierto"}]}]}]})";
const char* const kJsonLegacyAClip = R"({"schemaVersion":1,"maquinas":[{"id":"m","inicial":"a",
 "parametros":[{"id":"go","tipo":"booleano"}],
 "estados":[{"id":"a","clip":"idle"},{"id":"b","clip":"idle_up"}],
 "transiciones":[{"desde":"a","hacia":"b","cuando":[{"param":"go","op":"cierto"}]}]}]})";

CatalogoAnimacion catalogoDe(const char* json) {
    auto r = construyeCatalogoAnimacion({}, {textoJson(json)});
    MOTOR_CHECK(r.isOk());
    return r.value();
}

}  // namespace

MOTOR_TEST_CASE(secuencia_completa_pnj_basico_legacy) {
    const CatalogoAnimacion cat = catalogoReal();
    const MaquinaAnimacion* m = cat.maquina(IdMaquina::parse("pnj_basico").value());
    MOTOR_CHECK(m != nullptr);
    ControladorAnimacion c(m);
    Falso d;  // ningun clip: todo LEGACY
    MOTOR_CHECK(c.vivo().estado.value() == "reposo");
    MOTOR_CHECK(c.aplica(entradas(0, 2), d));  // primer aplica: clip abajo distinto del 0 inicial
    MOTOR_CHECK(c.vivo().clip.value() == "idle_down");
    MOTOR_CHECK(!c.aplica(entradas(0, 2), d));  // nada cambia
    MOTOR_CHECK(c.aplica(entradas(1, 3), d));   // andar, izquierda
    MOTOR_CHECK(c.vivo().estado.value() == "andar");
    MOTOR_CHECK(c.vivo().clip.value() == "walk_left");
    MOTOR_CHECK(c.aplica(entradas(1, 1), d));  // mismo estado, otro rumbo
    MOTOR_CHECK(c.vivo().clip.value() == "walk_right");
    MOTOR_CHECK(c.aplica(entradas(0, 1), d));  // vuelve a reposo
    MOTOR_CHECK(c.vivo().estado.value() == "reposo");
    MOTOR_CHECK(c.vivo().clip.value() == "idle_right");
    const std::vector<std::string> esperado = {"nombre:idle_down", "nombre:idle_down",
                                               "nombre:walk_left", "nombre:walk_right",
                                               "nombre:idle_right"};
    MOTOR_CHECK(d.llamadas == esperado);
}

MOTOR_TEST_CASE(legacy_ponNombre_en_cada_aplica_600_fotogramas) {
    const CatalogoAnimacion cat = catalogoReal();
    ControladorAnimacion c(cat.maquina(IdMaquina::parse("pnj_basico").value()));
    Falso d;
    for (int i = 0; i < 600; ++i) {
        c.aplica(entradas(0, 2), d);
    }
    MOTOR_CHECK(d.llamadas.size() == 600);
    for (const std::string& l : d.llamadas) {
        MOTOR_CHECK(l == "nombre:idle_down");
    }
}

MOTOR_TEST_CASE(clip_flanco_de_control_y_cambio) {
    const CatalogoAnimacion cat = catalogoReal();
    ControladorAnimacion c(cat.maquina(IdMaquina::parse("pnj_basico").value()));
    Falso d;
    d.clips = {"idle_down", "walk_down"};
    c.aplica(entradas(0, 2), d);  // primero: sin control -> 1 reengancha
    MOTOR_CHECK(d.llamadas.size() == 1 && d.llamadas[0] == "reengancha:idle_down");
    d.llamadas.clear();
    for (int i = 0; i < 50; ++i) {
        c.aplica(entradas(0, 2), d);  // con control y sin cambio: nada
    }
    MOTOR_CHECK(d.llamadas.empty());
    c.aplica(entradas(1, 2), d);  // cambio de clip: 1 cambiaClip con la politica de la transicion
    MOTOR_CHECK(d.llamadas.size() == 1 && d.llamadas[0] == "cambia:walk_down");
    MOTOR_CHECK(d.ultimaPolitica == PoliticaCambio::ReiniciarSiempre);
    d.llamadas.clear();
    d.perderControl();  // flanco
    c.aplica(entradas(1, 2), d);
    MOTOR_CHECK(d.llamadas.size() == 1 && d.llamadas[0] == "reengancha:walk_down");
    d.llamadas.clear();
    for (int i = 0; i < 20; ++i) {
        c.aplica(entradas(1, 2), d);
    }
    MOTOR_CHECK(d.llamadas.empty());
}

MOTOR_TEST_CASE(clip_politica_de_rumbo_sin_transicion) {
    const CatalogoAnimacion cat = catalogoReal();
    ControladorAnimacion c(cat.maquina(IdMaquina::parse("pnj_basico").value()));
    Falso d;
    d.clips = {"idle_down", "idle_left"};
    c.aplica(entradas(0, 2), d);
    d.llamadas.clear();
    c.aplica(entradas(0, 3), d);  // mismo estado, otro rumbo: politicaRumbo del estado
    MOTOR_CHECK(d.llamadas.size() == 1 && d.llamadas[0] == "cambia:idle_left");
    MOTOR_CHECK(d.ultimaPolitica == PoliticaCambio::ReiniciarSiDistinto);
}

MOTOR_TEST_CASE(suspendido_no_toca_el_destino) {
    const CatalogoAnimacion cat = catalogoReal();
    ControladorAnimacion c(cat.maquina(IdMaquina::parse("pnj_basico").value()));
    Falso d;
    c.suspende();
    MOTOR_CHECK(c.suspendido());
    const EstadoAnimacionVivo antes = c.vivo();
    for (int i = 0; i < 100; ++i) {
        MOTOR_CHECK(!c.aplica(entradas(1, 3), d));
    }
    MOTOR_CHECK(d.llamadas.empty());
    MOTOR_CHECK(c.vivo().estado == antes.estado && c.vivo().clip == antes.clip);
}

MOTOR_TEST_CASE(reanuda_fuerza_una_reemision_legacy) {
    const CatalogoAnimacion cat = catalogoReal();
    ControladorAnimacion c(cat.maquina(IdMaquina::parse("pnj_basico").value()));
    Falso d;
    c.aplica(entradas(0, 2), d);
    c.suspende();
    c.aplica(entradas(0, 2), d);
    MOTOR_CHECK(d.llamadas.size() == 1);
    c.reanuda();
    MOTOR_CHECK(!c.suspendido());
    c.aplica(entradas(0, 2), d);
    MOTOR_CHECK(d.llamadas.size() == 2 && d.llamadas[1] == "nombre:idle_down");
}

MOTOR_TEST_CASE(reanuda_fuerza_exactamente_un_reengancha_clip_con_control) {
    const CatalogoAnimacion cat = catalogoReal();
    ControladorAnimacion c(cat.maquina(IdMaquina::parse("pnj_basico").value()));
    Falso d;
    d.clips = {"idle_down"};
    c.aplica(entradas(0, 2), d);
    d.llamadas.clear();
    c.suspende();
    c.aplica(entradas(0, 2), d);
    MOTOR_CHECK(d.llamadas.empty());
    MOTOR_CHECK(d.tieneControl());  // el control NO se perdio
    c.reanuda();
    c.aplica(entradas(0, 2), d);
    MOTOR_CHECK(d.llamadas.size() == 1 && d.llamadas[0] == "reengancha:idle_down");
    for (int i = 0; i < 10; ++i) {
        c.aplica(entradas(0, 2), d);
    }
    MOTOR_CHECK(d.llamadas.size() == 1);  // solo una
}

MOTOR_TEST_CASE(finClip_se_lee_del_destino) {
    const CatalogoAnimacion cat = catalogoDe(kJsonFin);
    const MaquinaAnimacion* m = cat.maquina(IdMaquina::parse("m").value());
    MOTOR_CHECK(m != nullptr);
    {
        ControladorAnimacion c(m);
        Falso d;
        d.clips = {"idle", "idle_up"};
        d.fin = false;  // clip ciclico: el destino nunca dice fin
        for (int i = 0; i < 30; ++i) {
            MOTOR_CHECK(!c.aplica(EntradasAnimacion{}, d));
        }
        MOTOR_CHECK(c.vivo().estado.value() == "a");
    }
    {
        ControladorAnimacion c(m);
        Falso d;
        d.clips = {"idle", "idle_up"};
        c.aplica(EntradasAnimacion{}, d);
        d.fin = true;
        MOTOR_CHECK(c.aplica(EntradasAnimacion{}, d));
        MOTOR_CHECK(c.vivo().estado.value() == "b");
        MOTOR_CHECK(c.vivo().clip.value() == "idle_up");
    }
}

MOTOR_TEST_CASE(entradas_del_llamador_no_pueden_forzar_finClip) {
    const CatalogoAnimacion cat = catalogoDe(kJsonFin);
    ControladorAnimacion c(cat.maquina(IdMaquina::parse("m").value()));
    Falso d;
    d.clips = {"idle", "idle_up"};
    EntradasAnimacion e;
    e.finClip = true;  // el destino manda: dice false
    MOTOR_CHECK(!c.aplica(e, d));
    MOTOR_CHECK(c.vivo().estado.value() == "a");
}

MOTOR_TEST_CASE(disparos_consumidos_se_rellenan) {
    const CatalogoAnimacion cat = catalogoDe(kJsonDisparo);
    ControladorAnimacion c(cat.maquina(IdMaquina::parse("m").value()));
    Falso d;
    d.clips = {"idle", "idle_up"};
    std::vector<IdParametro> out = {par("viejo")};
    c.aplica(EntradasAnimacion{}, d, &out);
    MOTOR_CHECK(out.empty());  // sin disparo armado: lista sustituida (vacia)
    EntradasAnimacion e;
    e.valores[par("salta")] = 1.0;
    MOTOR_CHECK(c.aplica(e, d, &out));
    MOTOR_CHECK(out.size() == 1 && out[0] == par("salta"));
    MOTOR_CHECK(d.ultimaPolitica == PoliticaCambio::ConservarFase);
    c.suspende();
    out = {par("viejo")};
    c.aplica(e, d, &out);
    MOTOR_CHECK(out.empty());  // suspendido: nada consumido
}

MOTOR_TEST_CASE(pnj_basico_y_catalogo_de_clips_son_disjuntos) {
    const CatalogoAnimacion cat = catalogoReal();
    MOTOR_CHECK(!cat.clips.empty());
    const MaquinaAnimacion* m = cat.maquina(IdMaquina::parse("pnj_basico").value());
    MOTOR_CHECK(m != nullptr);
    for (const EstadoAnimacion& e : m->estados) {
        if (e.clipsPorRumbo.empty()) {
            MOTOR_CHECK(cat.clip(e.clip) == nullptr);
        }
        for (const IdClip& id : e.clipsPorRumbo) {
            MOTOR_CHECK(cat.clip(id) == nullptr);
            MOTOR_CHECK(esNombreLegacy(id));
        }
    }
    MOTOR_CHECK(cat.clip(clipId("idle")) != nullptr);  // el unico clip real de hoy
}

MOTOR_TEST_CASE(catalogo_ids_desconocidos_y_orden_de_carga) {
    const JsonValue estados = textoJson(
        R"({"schemaVersion":1,"maquinas":[{"id":"m","inicial":"a","parametros":[],
        "estados":[{"id":"a","clip":"no_existe_ni_legacy"}],"transiciones":[]}]})");
    MOTOR_CHECK(!construyeCatalogoAnimacion({}, {estados}).isOk());
    // Un clip del catalogo (no legacy) referenciado por una maquina: solo vale si los clips se
    // leen antes (aqui van en docsClips; sin ellos es error).
    const JsonValue clips = textoJson(
        R"({"schemaVersion":1,"clips":[{"id":"golpe","bucle":false,
        "fotogramas":[{"sprite":1,"duracionMs":100}]}]})");
    const JsonValue maq = textoJson(
        R"({"schemaVersion":1,"maquinas":[{"id":"m","inicial":"a","parametros":[],
        "estados":[{"id":"a","clip":"golpe"}],"transiciones":[]}]})");
    MOTOR_CHECK(!construyeCatalogoAnimacion({}, {maq}).isOk());
    auto ok = construyeCatalogoAnimacion({clips}, {maq});
    MOTOR_CHECK(ok.isOk());
    MOTOR_CHECK(ok.value().clip(clipId("golpe")) != nullptr);
    MOTOR_CHECK(ok.value().maquina(IdMaquina::parse("m").value()) != nullptr);
    // Duplicados entre documentos: error.
    MOTOR_CHECK(!construyeCatalogoAnimacion({clips, clips}, {}).isOk());
    MOTOR_CHECK(!construyeCatalogoAnimacion({clips}, {maq, maq}).isOk());
}

MOTOR_TEST_CASE(catalogo_busquedas_inexistentes_son_nullptr) {
    const CatalogoAnimacion cat = catalogoReal();
    MOTOR_CHECK(cat.clip(clipId("no_hay")) == nullptr);
    MOTOR_CHECK(cat.maquina(IdMaquina::parse("no_hay").value()) == nullptr);
    const CatalogoAnimacion vacio;
    MOTOR_CHECK(vacio.clip(clipId("idle")) == nullptr);
}

MOTOR_TEST_CASE(esNombreLegacy_exactamente_los_nueve) {
    const char* const si[] = {"idle",       "walk_up",    "walk_right", "walk_down", "walk_left",
                              "idle_up",    "idle_right", "idle_down",  "idle_left"};
    for (const char* n : si) {
        MOTOR_CHECK(esNombreLegacy(clipId(n)));
    }
    const char* const no[] = {"idle_diag", "walk", "idle_", "idle2", "golpe", "walk_up2", "a"};
    for (const char* n : no) {
        MOTOR_CHECK(!esNombreLegacy(clipId(n)));
    }
    MOTOR_CHECK(!esNombreLegacy(IdClip{}));
}

MOTOR_TEST_CASE(reanuda_es_idempotente_sin_suspension) {
    const CatalogoAnimacion cat = catalogoReal();
    ControladorAnimacion c(cat.maquina(IdMaquina::parse("pnj_basico").value()));
    Falso d;
    d.clips = {"idle_down"};
    c.aplica(entradas(0, 2), d);
    d.llamadas.clear();
    for (int i = 0; i < 5; ++i) {
        c.reanuda();
        c.aplica(entradas(0, 2), d);
    }
    MOTOR_CHECK(d.llamadas.empty());
    c.suspende();
    c.reanuda();
    c.aplica(entradas(0, 2), d);
    MOTOR_CHECK(d.llamadas.size() == 1 && d.llamadas[0] == "reengancha:idle_down");
}

MOTOR_TEST_CASE(finClip_solo_se_lee_en_destino_clip) {
    const CatalogoAnimacion cat = catalogoDe(kJsonFin);
    ControladorAnimacion c(cat.maquina(IdMaquina::parse("m").value()));
    Falso d;  // LEGACY: ningun clip
    d.fin = true;
    for (int i = 0; i < 5; ++i) {
        MOTOR_CHECK(!c.aplica(EntradasAnimacion{}, d));
    }
    MOTOR_CHECK(c.vivo().estado.value() == "a");
}

MOTOR_TEST_CASE(primer_aplica_reengancha_aunque_haya_control) {
    const CatalogoAnimacion cat = catalogoReal();
    ControladorAnimacion c(cat.maquina(IdMaquina::parse("pnj_basico").value()));
    Falso d;
    d.clips = {"idle_down"};
    d.control = true;  // reproduce otro clip del catalogo
    c.aplica(entradas(0, 2), d);
    MOTOR_CHECK(d.llamadas.size() == 1 && d.llamadas[0] == "reengancha:idle_down");
    c.aplica(entradas(0, 2), d);
    MOTOR_CHECK(d.llamadas.size() == 1);
}

MOTOR_TEST_CASE(cambio_de_estado_con_mismo_clip_devuelve_true) {
    const CatalogoAnimacion cat = catalogoDe(kJsonSoloMismoClip);
    ControladorAnimacion c(cat.maquina(IdMaquina::parse("m").value()));
    Falso d;
    c.aplica(EntradasAnimacion{}, d);
    EntradasAnimacion e;
    e.valores[par("go")] = 1.0;
    MOTOR_CHECK(c.aplica(e, d));  // cambio de estado, sin cambio de clip
    MOTOR_CHECK(c.vivo().estado.value() == "b");
}

MOTOR_TEST_CASE(forzar_se_consume_tambien_en_legacy) {
    const CatalogoAnimacion cat = catalogoDe(kJsonLegacyAClip);
    ControladorAnimacion c(cat.maquina(IdMaquina::parse("m").value()));
    Falso d;
    d.clips = {"idle_up"};  // "idle" es LEGACY; "idle_up" es CLIP
    c.aplica(EntradasAnimacion{}, d);  // primera reemision, consumida por ponNombre
    c.suspende();
    c.reanuda();
    c.aplica(EntradasAnimacion{}, d);  // reemision forzada, consumida por ponNombre
    d.control = true;
    d.llamadas.clear();
    EntradasAnimacion e;
    e.valores[par("go")] = 1.0;
    c.aplica(e, d);
    MOTOR_CHECK(d.llamadas.size() == 1 && d.llamadas[0] == "cambia:idle_up");
}

int main() { return motor_test::runAllTests(); }
