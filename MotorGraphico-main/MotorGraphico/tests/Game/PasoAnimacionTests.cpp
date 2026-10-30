// U3-06.E04 -- Game::pasoAnimacion. Sin GL; las maquinas se montan en memoria.
#include "TestHarness.h"

#include "Game/PasoAnimacion.h"

#include <string>

using namespace Game;

namespace {

IdEstado est(const char* s) { return IdEstado::parse(s).value(); }
IdParametro par(const char* s) { return IdParametro::parse(s).value(); }
IdClip clip(const char* s) { return IdClip::parse(s).value(); }

CondicionAnimacion cond(const char* p, OperadorCondicion op, double v = 0.0) {
    CondicionAnimacion c;
    c.parametro = par(p);
    c.op = op;
    c.valor = v;
    return c;
}

TransicionAnimacion tr(const char* desde, const char* hacia, std::vector<CondicionAnimacion> cuando,
                       PoliticaCambio pol = PoliticaCambio::ReiniciarSiempre, int prioridad = 0,
                       bool espera = false) {
    TransicionAnimacion t;
    if (desde != nullptr) {
        t.desde = est(desde);
    }
    t.hacia = est(hacia);
    t.cuando = std::move(cuando);
    t.politica = pol;
    t.prioridad = prioridad;
    t.esperaFinClip = espera;
    return t;
}

EstadoAnimacion estado(const char* id, const char* c) {
    EstadoAnimacion e;
    e.id = est(id);
    e.clip = clip(c);
    return e;
}

EstadoAnimacion estadoRumbo(const char* id, PoliticaCambio pol) {
    EstadoAnimacion e;
    e.id = est(id);
    e.clipsPorRumbo = {clip("arr"), clip("der"), clip("aba"), clip("izq")};
    e.politicaRumbo = pol;
    return e;
}

// reposo(clip idle) / andar(4 rumbos) / golpe(clip golpe) / muerto(clip muerto).
MaquinaAnimacion maq(std::vector<TransicionAnimacion> ts,
                     PoliticaCambio polRumbo = PoliticaCambio::ReiniciarSiDistinto) {
    MaquinaAnimacion m;
    m.inicial = est("reposo");
    m.estados = {estado("reposo", "idle"), estadoRumbo("andar", polRumbo), estado("golpe", "golpe"),
                 estado("muerto", "muerto")};
    for (std::size_t i = 0; i < ts.size(); ++i) {
        ts[i].orden = static_cast<int>(i);
    }
    m.transiciones = std::move(ts);
    return m;
}

EntradasAnimacion con(std::initializer_list<std::pair<const char*, double>> vs, bool fin = false) {
    EntradasAnimacion e;
    for (const auto& v : vs) {
        e.valores[par(v.first)] = v.second;
    }
    e.finClip = fin;
    return e;
}

EstadoAnimacionVivo vivo(const char* e, const char* c) { return {est(e), clip(c)}; }

const CondicionAnimacion kAnda = cond("anda", OperadorCondicion::Cierto);

}  // namespace

MOTOR_TEST_CASE(mismo_estado_no_reinicia_en_600_pasos) {
    const auto m = maq({tr("reposo", "andar", {kAnda})});
    EstadoAnimacionVivo v = vivo("reposo", "idle");
    for (int i = 0; i < 600; ++i) {
        const PasoAnimacion p = pasoAnimacion(m, v, con({{"anda", 0}}));
        MOTOR_CHECK(!p.cambioDeClip);
        MOTOR_CHECK(p.disparosConsumidos.empty());
        v = p.vivo;
    }
    MOTOR_CHECK(v.estado == est("reposo"));
    // Estado con rumbos, mismo rumbo, 600 pasos: tampoco.
    v = vivo("andar", "izq");
    for (int i = 0; i < 600; ++i) {
        MOTOR_CHECK(!pasoAnimacion(m, v, con({{"rumbo", 3}})).cambioDeClip);
    }
}

MOTOR_TEST_CASE(cambio_de_estado_con_cada_politica) {
    // reposo -> andar (rumbo 1 = der); el clip cambia en las tres politicas.
    for (PoliticaCambio pol : {PoliticaCambio::ReiniciarSiempre, PoliticaCambio::ReiniciarSiDistinto,
                               PoliticaCambio::ConservarFase}) {
        const auto m = maq({tr("reposo", "andar", {kAnda}, pol)});
        const PasoAnimacion p = pasoAnimacion(m, vivo("reposo", "idle"), con({{"anda", 1}, {"rumbo", 1}}));
        MOTOR_CHECK(p.cambioDeClip);
        MOTOR_CHECK(p.politica == pol);
        MOTOR_CHECK(p.clipNuevo == clip("der"));
        MOTOR_CHECK(p.vivo.estado == est("andar"));
        MOTOR_CHECK(p.vivo.clip == clip("der"));
    }
    // Mismo clip de destino (golpe -> golpe via otro estado con el mismo clip): la politica decide.
    auto m = maq({tr("golpe", "golpe", {kAnda}, PoliticaCambio::ReiniciarSiDistinto)});
    MOTOR_CHECK(!pasoAnimacion(m, vivo("golpe", "golpe"), con({{"anda", 1}})).cambioDeClip);
    m = maq({tr("golpe", "golpe", {kAnda}, PoliticaCambio::ReiniciarSiempre)});
    PasoAnimacion p = pasoAnimacion(m, vivo("golpe", "golpe"), con({{"anda", 1}}));
    MOTOR_CHECK(p.cambioDeClip && p.politica == PoliticaCambio::ReiniciarSiempre);
    m = maq({tr("golpe", "golpe", {kAnda}, PoliticaCambio::ConservarFase)});
    p = pasoAnimacion(m, vivo("golpe", "golpe"), con({{"anda", 1}}));
    MOTOR_CHECK(p.cambioDeClip && p.politica == PoliticaCambio::ConservarFase);
}

MOTOR_TEST_CASE(reiniciar_siempre_dos_veces_son_dos_cambios_y_si_distinto_uno) {
    const auto siempre = maq({tr("golpe", "golpe", {kAnda}, PoliticaCambio::ReiniciarSiempre)});
    int cambios = 0;
    EstadoAnimacionVivo v = vivo("golpe", "golpe");
    for (int i = 0; i < 2; ++i) {
        const PasoAnimacion p = pasoAnimacion(siempre, v, con({{"anda", 1}}));
        cambios += p.cambioDeClip ? 1 : 0;
        v = p.vivo;
    }
    MOTOR_CHECK(cambios == 2);
    // reposo -> andar -> (dos veces el mismo destino con SiDistinto) = 1 cambio.
    const auto dist = maq({tr("reposo", "andar", {kAnda}, PoliticaCambio::ReiniciarSiDistinto),
                           tr("andar", "andar", {kAnda}, PoliticaCambio::ReiniciarSiDistinto)});
    cambios = 0;
    v = vivo("reposo", "idle");
    for (int i = 0; i < 2; ++i) {
        const PasoAnimacion p = pasoAnimacion(dist, v, con({{"anda", 1}, {"rumbo", 2}}));
        cambios += p.cambioDeClip ? 1 : 0;
        v = p.vivo;
    }
    MOTOR_CHECK(cambios == 1);
}

MOTOR_TEST_CASE(cambio_de_rumbo_dentro_del_estado_usa_politica_rumbo) {
    for (PoliticaCambio pol : {PoliticaCambio::ReiniciarSiDistinto, PoliticaCambio::ConservarFase,
                               PoliticaCambio::ReiniciarSiempre}) {
        const auto m = maq({}, pol);
        const PasoAnimacion p = pasoAnimacion(m, vivo("andar", "der"), con({{"rumbo", 3}}));
        MOTOR_CHECK(p.cambioDeClip);
        MOTOR_CHECK(p.politica == pol);
        MOTOR_CHECK(p.clipNuevo == clip("izq"));
        MOTOR_CHECK(p.vivo.estado == est("andar") && p.vivo.clip == clip("izq"));
        // Siguiente paso con el mismo rumbo: ya no hay cambio (1 solo cambio).
        MOTOR_CHECK(!pasoAnimacion(m, p.vivo, con({{"rumbo", 3}})).cambioDeClip);
    }
    // Rumbo fuera de rango: modulo 4 (-1 -> izq); ausente = 0 (arr).
    const auto m = maq({});
    MOTOR_CHECK(pasoAnimacion(m, vivo("andar", "arr"), con({{"rumbo", -1}})).clipNuevo == clip("izq"));
    MOTOR_CHECK(!pasoAnimacion(m, vivo("andar", "arr"), con({})).cambioDeClip);
}

MOTOR_TEST_CASE(una_sola_transicion_por_paso) {
    // reposo -> andar (anda) y andar -> golpe (incondicional via 'pega'): tras la primera, la
    // segunda tambien es cierta, pero no se encadena.
    const auto m = maq({tr("reposo", "andar", {kAnda}), tr("andar", "golpe", {cond("pega", OperadorCondicion::Cierto)})});
    const PasoAnimacion p = pasoAnimacion(m, vivo("reposo", "idle"), con({{"anda", 1}, {"pega", 1}}));
    MOTOR_CHECK(p.vivo.estado == est("andar"));
    // El paso siguiente si da la segunda.
    MOTOR_CHECK(pasoAnimacion(m, p.vivo, con({{"anda", 1}, {"pega", 1}})).vivo.estado == est("golpe"));
}

MOTOR_TEST_CASE(espera_fin_de_clip_y_clip_ciclico_nunca_termina) {
    const auto m = maq({tr("golpe", "reposo", {}, PoliticaCambio::ReiniciarSiDistinto, 0, true)});
    MOTOR_CHECK(pasoAnimacion(m, vivo("golpe", "golpe"), con({}, false)).vivo.estado == est("golpe"));
    const PasoAnimacion p = pasoAnimacion(m, vivo("golpe", "golpe"), con({}, true));
    MOTOR_CHECK(p.vivo.estado == est("reposo") && p.cambioDeClip && p.clipNuevo == clip("idle"));
    // Un clip ciclico no pone finClip (lo garantiza el adaptador): con false nunca se sale, 1000 pasos.
    EstadoAnimacionVivo v = vivo("golpe", "golpe");
    for (int i = 0; i < 1000; ++i) {
        v = pasoAnimacion(m, v, con({}, false)).vivo;
    }
    MOTOR_CHECK(v.estado == est("golpe"));
}

MOTOR_TEST_CASE(cualquiera_no_reinicia_el_clip_del_estado_actual) {
    // muerto: "cualquiera -> muerto" con reiniciarSiempre. Desde muerto no se elige: sin cambio.
    const auto m = maq({tr(nullptr, "muerto", {cond("muerto", OperadorCondicion::Cierto)})});
    PasoAnimacion p = pasoAnimacion(m, vivo("reposo", "idle"), con({{"muerto", 1}}));
    MOTOR_CHECK(p.cambioDeClip && p.vivo.estado == est("muerto") && p.clipNuevo == clip("muerto"));
    EstadoAnimacionVivo v = p.vivo;
    for (int i = 0; i < 600; ++i) {
        p = pasoAnimacion(m, v, con({{"muerto", 1}}));
        MOTOR_CHECK(!p.cambioDeClip);  // el clip avanza, no se reinicia
        v = p.vivo;
    }
    // Y no bloquea una salida de menor prioridad desde muerto.
    const auto m2 = maq({tr(nullptr, "muerto", {cond("muerto", OperadorCondicion::Cierto)}, PoliticaCambio::ReiniciarSiempre, 9),
                         tr("muerto", "reposo", {cond("muerto", OperadorCondicion::Cierto)}, PoliticaCambio::ReiniciarSiempre, 1)});
    MOTOR_CHECK(pasoAnimacion(m2, vivo("muerto", "muerto"), con({{"muerto", 1}})).vivo.estado == est("reposo"));
}

MOTOR_TEST_CASE(entrega_inicial_clip_nuevo_en_el_mismo_paso) {
    // D-T8: el paso que cambia de estado ya trae el clip a entregar (clipNuevo), sin esperar al siguiente.
    const auto m = maq({tr("reposo", "andar", {kAnda})});
    const PasoAnimacion p = pasoAnimacion(m, vivo("reposo", "idle"), con({{"anda", 1}, {"rumbo", 2}}));
    MOTOR_CHECK(p.cambioDeClip && p.clipNuevo == clip("aba") && p.vivo.clip == clip("aba"));
}

MOTOR_TEST_CASE(disparos_consumidos_una_sola_vez_y_solo_de_la_ganadora) {
    const auto disp = cond("golpear", OperadorCondicion::Disparado);
    const auto m = maq({tr("reposo", "golpe", {disp, disp}), tr("reposo", "andar", {cond("otro", OperadorCondicion::Disparado)}, PoliticaCambio::ReiniciarSiempre, -1)});
    PasoAnimacion p = pasoAnimacion(m, vivo("reposo", "idle"), con({{"golpear", 1}, {"otro", 1}}));
    MOTOR_CHECK(p.vivo.estado == est("golpe"));
    MOTOR_CHECK(p.disparosConsumidos.size() == 1u && p.disparosConsumidos[0] == par("golpear"));
    // Sin transicion no se consume nada.
    p = pasoAnimacion(m, vivo("golpe", "golpe"), con({{"golpear", 1}}));
    MOTOR_CHECK(p.disparosConsumidos.empty());
}

MOTOR_TEST_CASE(determinismo_1000_iteraciones_y_estado_desconocido) {
    const auto m = maq({tr("reposo", "andar", {kAnda})});
    const auto e = con({{"anda", 1}, {"rumbo", 1}});
    const PasoAnimacion base = pasoAnimacion(m, vivo("reposo", "idle"), e);
    for (int i = 0; i < 1000; ++i) {
        const PasoAnimacion p = pasoAnimacion(m, vivo("reposo", "idle"), e);
        MOTOR_CHECK(p.vivo.estado == base.vivo.estado && p.vivo.clip == base.vivo.clip);
        MOTOR_CHECK(p.cambioDeClip == base.cambioDeClip && p.politica == base.politica);
    }
    const PasoAnimacion x = pasoAnimacion(m, vivo("d", "idle"), e);
    MOTOR_CHECK(!x.cambioDeClip && x.vivo.estado == est("d"));
}

int main() { return motor_test::runAllTests(); }
