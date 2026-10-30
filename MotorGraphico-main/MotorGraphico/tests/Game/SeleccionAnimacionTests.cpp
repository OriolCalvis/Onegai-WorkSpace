// U3-06.E03 -- Game::eligeTransicion y Game::clipDeEstado. Sin GL; las maquinas se montan en memoria.
#include "TestHarness.h"

#include "Game/SeleccionAnimacion.h"

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

CondicionAnimacion condFinClip() {
    CondicionAnimacion c;
    c.parametro = par(kParametroFinClip);
    c.op = OperadorCondicion::Cierto;
    return c;
}

// Transicion hacia `hacia`; desde == nullptr -> cualquier estado.
TransicionAnimacion tr(const char* desde, const char* hacia, std::vector<CondicionAnimacion> cuando,
                       int prioridad = 0, int orden = 0, bool espera = false) {
    TransicionAnimacion t;
    if (desde != nullptr) {
        t.desde = est(desde);
    }
    t.hacia = est(hacia);
    t.cuando = std::move(cuando);
    t.prioridad = prioridad;
    t.orden = orden;
    t.esperaFinClip = espera;
    return t;
}

MaquinaAnimacion maq(std::vector<TransicionAnimacion> ts) {
    MaquinaAnimacion m;
    m.inicial = est("a");
    for (const char* e : {"a", "b", "c", "d"}) {
        EstadoAnimacion s;
        s.id = est(e);
        s.clip = clip("idle");
        m.estados.push_back(s);
    }
    for (std::size_t i = 0; i < ts.size(); ++i) {
        ts[i].orden = ts[i].orden != 0 ? ts[i].orden : static_cast<int>(i);
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

std::string hacia(const MaquinaAnimacion& m, const EntradasAnimacion& e, const char* actual = "a") {
    const auto i = eligeTransicion(m, est(actual), e);
    return i ? m.transiciones[*i].hacia.value() : std::string("<ninguna>");
}

const CondicionAnimacion kX = cond("x", OperadorCondicion::Cierto);

}  // namespace

MOTOR_TEST_CASE(gana_la_mayor_prioridad_con_dos_candidatas) {
    // Dos candidatas ciertas a la vez; la de mayor prioridad esta DESPUES en el JSON.
    const auto m = maq({tr("a", "b", {kX}, 1), tr("a", "c", {kX}, 5)});
    MOTOR_CHECK(hacia(m, con({{"x", 1}})) == "c");
    // Y al reves: la mayor esta antes.
    const auto m2 = maq({tr("a", "b", {kX}, 5), tr("a", "c", {kX}, 1)});
    MOTOR_CHECK(hacia(m2, con({{"x", 1}})) == "b");
    // Prioridades negativas: -1 gana a -3.
    const auto m3 = maq({tr("a", "b", {kX}, -3), tr("a", "c", {kX}, -1)});
    MOTOR_CHECK(hacia(m3, con({{"x", 1}})) == "c");
}

MOTOR_TEST_CASE(empate_de_prioridad_gana_el_menor_orden) {
    // Mismo desde, misma prioridad: gana la de menor `orden`, sin depender de la posicion en el vector.
    auto m = maq({tr("a", "b", {kX}), tr("a", "c", {kX})});
    m.transiciones[0].orden = 7;
    m.transiciones[1].orden = 2;
    MOTOR_CHECK(hacia(m, con({{"x", 1}})) == "c");
    m.transiciones[0].orden = 1;
    MOTOR_CHECK(hacia(m, con({{"x", 1}})) == "b");
}

MOTOR_TEST_CASE(empate_de_prioridad_especifico_antes_que_cualquiera) {
    // "cualquiera" declarada antes (orden 0) pierde frente a la de desde especifico (orden 1).
    const auto m = maq({tr(nullptr, "c", {kX}), tr("a", "b", {kX})});
    MOTOR_CHECK(hacia(m, con({{"x", 1}})) == "b");
    // Pero la prioridad manda sobre la especificidad.
    const auto m2 = maq({tr(nullptr, "c", {kX}, 3), tr("a", "b", {kX}, 1)});
    MOTOR_CHECK(hacia(m2, con({{"x", 1}})) == "c");
}

MOTOR_TEST_CASE(desde_ausente_aplica_desde_cualquier_estado) {
    const auto m = maq({tr(nullptr, "d", {kX})});
    for (const char* actual : {"a", "b", "c", "d"}) {
        MOTOR_CHECK(hacia(m, con({{"x", 1}}), actual) == "d");
    }
    // Con desde concreto, solo ese estado.
    const auto m2 = maq({tr("b", "d", {kX})});
    MOTOR_CHECK(hacia(m2, con({{"x", 1}}), "a") == "<ninguna>");
    MOTOR_CHECK(hacia(m2, con({{"x", 1}}), "b") == "d");
}

MOTOR_TEST_CASE(ninguna_candidata_es_sin_transicion) {
    const auto m = maq({tr("a", "b", {kX})});
    MOTOR_CHECK(!eligeTransicion(m, est("a"), con({{"x", 0}})).has_value());
    MOTOR_CHECK(!eligeTransicion(m, est("a"), EntradasAnimacion{}).has_value());  // ausente = 0
    MOTOR_CHECK(!eligeTransicion(MaquinaAnimacion{}, est("a"), EntradasAnimacion{}).has_value());
    // Las condiciones son un AND: falta una -> no dispara.
    const auto m2 = maq({tr("a", "b", {kX, cond("y", OperadorCondicion::Cierto)})});
    MOTOR_CHECK(!eligeTransicion(m2, est("a"), con({{"x", 1}})).has_value());
    MOTOR_CHECK(eligeTransicion(m2, est("a"), con({{"x", 1}, {"y", 1}})).has_value());
}

MOTOR_TEST_CASE(devuelve_el_indice_en_transiciones) {
    const auto m = maq({tr("b", "c", {kX}), tr("a", "d", {kX}), tr("a", "c", {kX}, -1)});
    const auto i = eligeTransicion(m, est("a"), con({{"x", 1}}));
    MOTOR_CHECK(i.has_value());
    MOTOR_CHECK(*i == 1u);
}

MOTOR_TEST_CASE(esperafinclip_sin_finclip_no_dispara) {
    const auto m = maq({tr("a", "b", {}, 0, 0, true)});
    MOTOR_CHECK(hacia(m, con({})) == "<ninguna>");
    MOTOR_CHECK(hacia(m, con({}, true)) == "b");
    // Con condiciones: hacen falta ambas.
    const auto m2 = maq({tr("a", "b", {kX}, 0, 0, true)});
    MOTOR_CHECK(hacia(m2, con({{"x", 1}}, false)) == "<ninguna>");
    MOTOR_CHECK(hacia(m2, con({}, true)) == "<ninguna>");
    MOTOR_CHECK(hacia(m2, con({{"x", 1}}, true)) == "b");
}

MOTOR_TEST_CASE(condicion_finclip_usa_la_entrada_y_no_valores) {
    const auto m = maq({tr("a", "b", {condFinClip()})});
    MOTOR_CHECK(hacia(m, con({}, true)) == "b");
    MOTOR_CHECK(hacia(m, con({}, false)) == "<ninguna>");
    // Un valor "finclip" en `valores` no cuenta ni a favor ni en contra.
    MOTOR_CHECK(hacia(m, con({{"finclip", 1}}, false)) == "<ninguna>");
    MOTOR_CHECK(hacia(m, con({{"finclip", 0}}, true)) == "b");
    // Con op falso: cierta solo si finClip es falso.
    CondicionAnimacion f = condFinClip();
    f.op = OperadorCondicion::Falso;
    const auto m2 = maq({tr("a", "b", {f})});
    MOTOR_CHECK(hacia(m2, con({{"finclip", 1}}, false)) == "b");
    MOTOR_CHECK(hacia(m2, con({}, true)) == "<ninguna>");
}

MOTOR_TEST_CASE(operadores_de_entero_real_booleano_y_disparo) {
    auto una = [](CondicionAnimacion c) { return maq({tr("a", "b", {c})}); };
    auto dispara = [&](CondicionAnimacion c, double v) { return hacia(una(c), con({{"n", v}})) == "b"; };
    // Entero / real: los seis comparadores en los tres lados del umbral 3.
    const struct { OperadorCondicion op; bool menor, igual, mayor; } tabla[] = {
        {OperadorCondicion::Igual, false, true, false},      {OperadorCondicion::Distinto, true, false, true},
        {OperadorCondicion::Mayor, false, false, true},      {OperadorCondicion::MayorIgual, false, true, true},
        {OperadorCondicion::Menor, true, false, false},      {OperadorCondicion::MenorIgual, true, true, false}};
    for (const auto& f : tabla) {
        MOTOR_CHECK(dispara(cond("n", f.op, 3.0), 2.0) == f.menor);
        MOTOR_CHECK(dispara(cond("n", f.op, 3.0), 3.0) == f.igual);
        MOTOR_CHECK(dispara(cond("n", f.op, 3.0), 4.0) == f.mayor);
        // Real (valor fraccionario).
        MOTOR_CHECK(dispara(cond("n", f.op, 0.5), 0.25) == f.menor);
        MOTOR_CHECK(dispara(cond("n", f.op, 0.5), 0.5) == f.igual);
        MOTOR_CHECK(dispara(cond("n", f.op, 0.5), 0.75) == f.mayor);
    }
    // Booleano.
    MOTOR_CHECK(dispara(cond("n", OperadorCondicion::Cierto), 1.0));
    MOTOR_CHECK(!dispara(cond("n", OperadorCondicion::Cierto), 0.0));
    MOTOR_CHECK(dispara(cond("n", OperadorCondicion::Falso), 0.0));
    MOTOR_CHECK(!dispara(cond("n", OperadorCondicion::Falso), 1.0));
    // Disparo armado (1) / no armado (0 o ausente).
    MOTOR_CHECK(dispara(cond("n", OperadorCondicion::Disparado), 1.0));
    MOTOR_CHECK(!dispara(cond("n", OperadorCondicion::Disparado), 0.0));
    MOTOR_CHECK(hacia(una(cond("n", OperadorCondicion::Disparado)), con({})) == "<ninguna>");
    // Booleano ausente = falso.
    MOTOR_CHECK(hacia(una(cond("n", OperadorCondicion::Falso)), con({})) == "b");
}

MOTOR_TEST_CASE(rumbo_sale_de_valores_y_el_cambio_de_direccion_no_es_transicion) {
    const auto m = maq({tr("a", "b", {cond("rumbo", OperadorCondicion::Igual, 3.0)})});
    MOTOR_CHECK(hacia(m, con({{"rumbo", 3}})) == "b");
    MOTOR_CHECK(hacia(m, con({{"rumbo", 1}})) == "<ninguna>");
    // Sin transiciones, cambiar el rumbo no elige nada (lo resuelve clipDeEstado).
    MaquinaAnimacion solo = maq({});
    MOTOR_CHECK(!eligeTransicion(solo, est("a"), con({{"rumbo", 2}})).has_value());
}

MOTOR_TEST_CASE(clip_de_estado_modulo_4_del_rumbo) {
    EstadoAnimacion e;
    e.id = est("a");
    e.clipsPorRumbo = {clip("arr"), clip("der"), clip("aba"), clip("izq")};
    const char* esperado[4] = {"arr", "der", "aba", "izq"};
    for (int r = 0; r < 4; ++r) {
        MOTOR_CHECK(clipDeEstado(e, r).value() == esperado[r]);
    }
    MOTOR_CHECK(clipDeEstado(e, -1).value() == "izq");  // -1 -> 3
    MOTOR_CHECK(clipDeEstado(e, 4).value() == "arr");   // 4 -> 0
    MOTOR_CHECK(clipDeEstado(e, 7).value() == "izq");   // 7 -> 3
    MOTOR_CHECK(clipDeEstado(e, 5).value() == "der");
    MOTOR_CHECK(clipDeEstado(e, -4).value() == "arr");
    MOTOR_CHECK(clipDeEstado(e, -5).value() == "izq");
    // Estado de un solo clip: el rumbo no importa.
    EstadoAnimacion u;
    u.id = est("b");
    u.clip = clip("golpe");
    for (int r : {-1, 0, 2, 4, 7}) {
        MOTOR_CHECK(clipDeEstado(u, r).value() == "golpe");
    }
}

int main() { return motor_test::runAllTests(); }
