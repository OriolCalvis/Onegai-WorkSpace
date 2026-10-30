// U4-07.E02/E03 — tests de leerEmisor y de emisiones (residuo). GL-free.
#include "Game/Emisor/DefinicionEmisor.h"
#include "Game/Emisor/EmisionConResiduo.h"
#include "Game/Personajes/FamiliasDeParticulas.h"

#include "TestHarness.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

using namespace Game::Emisor;

namespace {

Result<DefinicionEmisor> lee(const std::string& texto) {
    const auto j = JsonValue::parse(texto);
    if (!j.isOk()) return Result<DefinicionEmisor>::Error("JSON de test mal escrito: " + j.errorMessage());
    return leerEmisor(j.value());
}

bool contiene(const std::string& s, const std::string& parte) { return s.find(parte) != std::string::npos; }

const char* const kValido =
    "{\"familia\":\"agua\",\"tasa\":40,\"duracion\":0.15,\"cortar\":true,\"vida\":[0.2,0.5],"
    "\"velocidad\":[10,60],\"angulo\":-90,\"abanico\":120,\"maximo\":16,\"semilla\":4503599627370496,"
    "\"opacidad\":1.5,\"contraste\":0.35,\"color\":\"5AB4ff\"}";

MOTOR_TEST_CASE(json_valido_da_los_campos_exactos) {
    const auto r = lee(kValido);
    MOTOR_CHECK(r.isOk());
    if (!r.isOk()) return;
    const auto& d = r.value();
    MOTOR_CHECK(d.familia == "agua" && d.tasa == 40.0 && d.duracion == 0.15 && d.cortar);
    MOTOR_CHECK(d.vidaMin == 0.2 && d.vidaMax == 0.5 && d.velocidadMin == 10.0 && d.velocidadMax == 60.0);
    MOTOR_CHECK(d.anguloCentro == -90.0 && d.abanico == 120.0 && d.maximo == 16);
    MOTOR_CHECK(d.opacidad == 1.5f && d.contraste == 0.35f);
    MOTOR_CHECK(d.colorBase[0] == 0x5a && d.colorBase[1] == 0xb4 && d.colorBase[2] == 0xff);
}

MOTOR_TEST_CASE(semilla_borde_2_53) {
    // 2^53-1 cabe; 2^53 y 2^53+1 (que el parser ya redondea a 2^53) son Error.
    const auto ok = lee("{\"familia\":\"agua\",\"vida\":[1,1],\"maximo\":4,\"semilla\":9007199254740991}");
    MOTOR_CHECK(ok.isOk() && ok.value().semilla == 9007199254740991ULL);
    const auto a = lee("{\"familia\":\"agua\",\"vida\":[1,1],\"maximo\":4,\"semilla\":9007199254740992}");
    MOTOR_CHECK(!a.isOk() && contiene(a.errorMessage(), "emisor.semilla"));
    const auto b = lee("{\"familia\":\"agua\",\"vida\":[1,1],\"maximo\":4,\"semilla\":9007199254740993}");
    MOTOR_CHECK(!b.isOk() && contiene(b.errorMessage(), "emisor.semilla"));
}

MOTOR_TEST_CASE(opacidad_que_se_vuelve_cero_en_float_es_error) {
    const auto r = lee("{\"familia\":\"agua\",\"vida\":[1,1],\"maximo\":4,\"opacidad\":1e-46}");
    MOTOR_CHECK(!r.isOk() && contiene(r.errorMessage(), "emisor.opacidad"));
}

MOTOR_TEST_CASE(familia_no_texto_dice_obligatoria) {
    const auto r = lee("{\"familia\":3,\"vida\":[1,1],\"maximo\":4}");
    MOTOR_CHECK(!r.isOk() && contiene(r.errorMessage(), "emisor.familia") && contiene(r.errorMessage(), "obligatoria (texto)"));
}

MOTOR_TEST_CASE(minimo_usa_los_por_defecto) {
    const auto r = lee("{\"familia\":\"chispas\",\"vida\":[1,1],\"maximo\":1}");
    MOTOR_CHECK(r.isOk());
    if (!r.isOk()) return;
    const auto& d = r.value();
    MOTOR_CHECK(d.tasa == 0.0 && d.duracion == 0.0 && !d.cortar && d.abanico == 360.0 && d.semilla == 0);
    MOTOR_CHECK(d.opacidad == 1.0f && d.contraste == 0.0f && d.colorBase[0] == 255 && d.colorBase[1] == 240);
}

MOTOR_TEST_CASE(acepta_las_40_familias) {
    for (const auto& f : Game::familiasDeParticulas()) {
        const auto r = lee("{\"familia\":\"" + f + "\",\"vida\":[1,1],\"maximo\":1}");
        MOTOR_CHECK(r.isOk());
    }
    MOTOR_CHECK(Game::familiasDeParticulas().size() == 40);
}

MOTOR_TEST_CASE(acepta_los_extremos_de_los_limites) {
    const auto alto = lee("{\"familia\":\"agua\",\"tasa\":500,\"duracion\":60,\"vida\":[10,10],"
                          "\"velocidad\":[0,2000],\"angulo\":1000000,\"abanico\":360,\"maximo\":256,"
                          "\"opacidad\":2,\"contraste\":1}");
    MOTOR_CHECK(alto.isOk());
    const auto& a = alto.value();
    MOTOR_CHECK(a.tasa == 500.0 && a.duracion == 60.0 && a.vidaMin == 10.0 && a.vidaMax == 10.0);
    MOTOR_CHECK(a.velocidadMin == 0.0 && a.velocidadMax == 2000.0 && a.anguloCentro == 1000000.0);
    MOTOR_CHECK(a.abanico == 360.0 && a.maximo == 256 && a.opacidad == 2.0f && a.contraste == 1.0f);
    const auto bajo = lee("{\"familia\":\"agua\",\"tasa\":0,\"duracion\":0,\"vida\":[0.001,0.001],"
                          "\"angulo\":-1000000,\"abanico\":0.5,\"maximo\":1,\"contraste\":0}");
    MOTOR_CHECK(bajo.isOk());
    const auto& b = bajo.value();
    MOTOR_CHECK(b.tasa == 0.0 && b.duracion == 0.0 && b.vidaMin == 0.001 && b.anguloCentro == -1000000.0);
    MOTOR_CHECK(b.abanico == 0.5 && b.maximo == 1 && b.contraste == 0.0f);
}

MOTOR_TEST_CASE(familia_inexistente_es_error_y_dice_cual) {
    const auto r = lee("{\"familia\":\"no_existe\",\"vida\":[1,1],\"maximo\":4}");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(contiene(r.errorMessage(), "emisor.familia") && contiene(r.errorMessage(), "no_existe"));
}

MOTOR_TEST_CASE(clave_desconocida_dice_la_ruta_y_la_clave) {
    const auto r = lee("{\"familia\":\"agua\",\"vida\":[1,1],\"maximo\":4,\"opacity\":0.5}");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage() == "emisor: clave desconocida: opacity");
}

MOTOR_TEST_CASE(tasa_y_vida_fuera_de_limites_dicen_el_campo) {
    const auto t = lee("{\"familia\":\"agua\",\"tasa\":501,\"vida\":[1,1],\"maximo\":4}");
    MOTOR_CHECK(!t.isOk() && contiene(t.errorMessage(), "emisor.tasa"));
    const auto v = lee("{\"familia\":\"agua\",\"vida\":[1,10.5],\"maximo\":4}");
    MOTOR_CHECK(!v.isOk() && contiene(v.errorMessage(), "emisor.vida"));
}

MOTOR_TEST_CASE(cada_caso_negativo_es_error) {
    struct Caso { const char* nombre; const char* json; };
    const Caso casos[] = {
        {"sin familia", R"J({"vida":[1,1],"maximo":4})J"},
        {"familia no_existe", R"J({"familia":"no_existe","vida":[1,1],"maximo":4})J"},
        {"familia no texto", R"J({"familia":3,"vida":[1,1],"maximo":4})J"},
        {"tasa -1", R"J({"familia":"agua","tasa":-1,"vida":[1,1],"maximo":4})J"},
        {"tasa 1e9", R"J({"familia":"agua","tasa":1e9,"vida":[1,1],"maximo":4})J"},
        {"tasa 501", R"J({"familia":"agua","tasa":501,"vida":[1,1],"maximo":4})J"},
        {"tasa texto", R"J({"familia":"agua","tasa":"x","vida":[1,1],"maximo":4})J"},
        {"tasa inf", R"J({"familia":"agua","tasa":1e999,"vida":[1,1],"maximo":4})J"},
        {"duracion 61", R"J({"familia":"agua","duracion":61,"vida":[1,1],"maximo":4})J"},
        {"cortar no bool", R"J({"familia":"agua","cortar":1,"vida":[1,1],"maximo":4})J"},
        {"sin vida", R"J({"familia":"agua","maximo":4})J"},
        {"vida 0", R"J({"familia":"agua","vida":[0,1],"maximo":4})J"},
        {"vida invertida", R"J({"familia":"agua","vida":[2,1],"maximo":4})J"},
        {"vida 11", R"J({"familia":"agua","vida":[1,11],"maximo":4})J"},
        {"vida 1 elem", R"J({"familia":"agua","vida":[1],"maximo":4})J"},
        {"vida escalar", R"J({"familia":"agua","vida":1,"maximo":4})J"},
        {"velocidad invertida", R"J({"familia":"agua","vida":[1,1],"velocidad":[5,1],"maximo":4})J"},
        {"velocidad 2001", R"J({"familia":"agua","vida":[1,1],"velocidad":[0,2001],"maximo":4})J"},
        {"velocidad negativa", R"J({"familia":"agua","vida":[1,1],"velocidad":[-1,1],"maximo":4})J"},
        {"abanico 0", R"J({"familia":"agua","vida":[1,1],"abanico":0,"maximo":4})J"},
        {"abanico 361", R"J({"familia":"agua","vida":[1,1],"abanico":361,"maximo":4})J"},
        {"angulo texto", R"J({"familia":"agua","vida":[1,1],"angulo":"a","maximo":4})J"},
        {"sin maximo", R"J({"familia":"agua","vida":[1,1]})J"},
        {"maximo 0", R"J({"familia":"agua","vida":[1,1],"maximo":0})J"},
        {"maximo 100000", R"J({"familia":"agua","vida":[1,1],"maximo":100000})J"},
        {"maximo 257", R"J({"familia":"agua","vida":[1,1],"maximo":257})J"},
        {"maximo 2.5", R"J({"familia":"agua","vida":[1,1],"maximo":2.5})J"},
        {"semilla -1", R"J({"familia":"agua","vida":[1,1],"maximo":4,"semilla":-1})J"},
        {"semilla 1.5", R"J({"familia":"agua","vida":[1,1],"maximo":4,"semilla":1.5})J"},
        {"semilla 1e300", R"J({"familia":"agua","vida":[1,1],"maximo":4,"semilla":1e300})J"},
        {"semilla 2^53+2", R"J({"familia":"agua","vida":[1,1],"maximo":4,"semilla":9007199254740994})J"},
        {"semilla texto", R"J({"familia":"agua","vida":[1,1],"maximo":4,"semilla":"7"})J"},
        {"opacidad 0", R"J({"familia":"agua","vida":[1,1],"maximo":4,"opacidad":0})J"},
        {"opacidad 2.5", R"J({"familia":"agua","vida":[1,1],"maximo":4,"opacidad":2.5})J"},
        {"contraste 2", R"J({"familia":"agua","vida":[1,1],"maximo":4,"contraste":2})J"},
        {"contraste -0.1", R"J({"familia":"agua","vida":[1,1],"maximo":4,"contraste":-0.1})J"},
        {"color zzzzzz", R"J({"familia":"agua","vida":[1,1],"maximo":4,"color":"zzzzzz"})J"},
        {"color corto", R"J({"familia":"agua","vida":[1,1],"maximo":4,"color":"fff"})J"},
        {"color con almohadilla", R"J({"familia":"agua","vida":[1,1],"maximo":4,"color":"#ffffff"})J"},
        {"color no texto", R"J({"familia":"agua","vida":[1,1],"maximo":4,"color":16777215})J"},
        {"duracion -1", R"J({"familia":"agua","duracion":-1,"vida":[1,1],"maximo":4})J"},
        {"vida 3 elem", R"J({"familia":"agua","vida":[1,2,3],"maximo":4})J"},
        {"opacidad 1e-46", R"J({"familia":"agua","vida":[1,1],"maximo":4,"opacidad":1e-46})J"},
        {"semilla 2^53", R"J({"familia":"agua","vida":[1,1],"maximo":4,"semilla":9007199254740992})J"},
        {"semilla 2^53+1", R"J({"familia":"agua","vida":[1,1],"maximo":4,"semilla":9007199254740993})J"},
        {"clave desconocida", R"J({"familia":"agua","vida":[1,1],"maximo":4,"tassa":5})J"},
        {"clave desconocida (errata de opcional)", R"J({"familia":"agua","vida":[1,1],"maximo":4,"opacity":0.5})J"},
        {"no objeto", R"J([1,2])J"},
    };
    for (const auto& c : casos) {
        const auto r = lee(c.json);
        if (r.isOk()) MOTOR_CHECK_FAIL(std::string("deberia ser Error: ") + c.nombre);
        else MOTOR_CHECK(contiene(r.errorMessage(), "emisor"));
    }
}

// ---- E03: emisiones con residuo ----

struct Total { int cuantas = 0; double residuo = 0.0; bool ok = true; int recortes = 0; };

// Trocea `n` pasos de `dt` con la tasa dada (Error -> ok=false).
Total corre(int n, double dt, double tasa, double residuo0 = 0.0) {
    Total t;
    t.residuo = residuo0;
    for (int i = 0; i < n; ++i) {
        const auto r = emisiones(EstadoEmision{t.residuo}, dt, tasa);
        if (!r.isOk()) { t.ok = false; return t; }
        t.cuantas += r.value().cuantas;
        t.residuo = r.value().estado.residuo;
        t.recortes += r.value().recortado ? 1 : 0;
        if (t.residuo < 0.0 || t.residuo >= 1.0) t.ok = false;
    }
    return t;
}

// Replica del orden del tick que cruza `duracion` (contrato E05, paso 1): se emite solo hasta `duracion`.
Total nube(double tasa, double duracion, double dt) {
    Total t;
    double tiempo = 0.0;
    while (tiempo < duracion - 1e-9) {
        const double dtE = std::min(dt, duracion - tiempo);
        const auto r = emisiones(EstadoEmision{t.residuo}, dtE, tasa);
        if (!r.isOk()) { t.ok = false; return t; }
        t.cuantas += r.value().cuantas;
        t.residuo = r.value().estado.residuo;
        tiempo += dt;
    }
    return t;
}

MOTOR_TEST_CASE(emisiones_residuo_exacto_entre_pasos) {
    const auto a = emisiones(EstadoEmision{}, 0.05, 10.0);
    MOTOR_CHECK(a.isOk() && a.value().cuantas == 0 && a.value().estado.residuo == 0.5 && !a.value().recortado);
    const auto b = emisiones(a.value().estado, 0.05, 10.0);
    MOTOR_CHECK(b.isOk() && b.value().cuantas == 1 && b.value().estado.residuo == 0.0);
    // residuo de entrada 0.75 + 0.5 = 1.25 -> 1 y 0.25
    const auto c = emisiones(EstadoEmision{0.75}, 0.05, 10.0);
    MOTOR_CHECK(c.isOk() && c.value().cuantas == 1 && c.value().estado.residuo == 0.25);
}

MOTOR_TEST_CASE(emisiones_tasa_cero_y_dt_cero_dejan_el_estado_intacto) {
    for (const double r0 : {0.0, 0.3, 0.999}) {
        const auto t = emisiones(EstadoEmision{r0}, 0.1, 0.0);
        MOTOR_CHECK(t.isOk() && t.value().cuantas == 0 && t.value().estado.residuo == r0 && !t.value().recortado);
        const auto d = emisiones(EstadoEmision{r0}, 0.0, 500.0);  // pausa (D-O6)
        MOTOR_CHECK(d.isOk() && d.value().cuantas == 0 && d.value().estado.residuo == r0 && !d.value().recortado);
    }
    const auto cien = corre(100, 0.0, 40.0, 0.5);
    MOTOR_CHECK(cien.ok && cien.cuantas == 0 && cien.residuo == 0.5);
    const auto tasa0 = corre(100, 1.0 / 60.0, 0.0);
    MOTOR_CHECK(tasa0.ok && tasa0.cuantas == 0 && tasa0.residuo == 0.0);
}

MOTOR_TEST_CASE(emisiones_entradas_invalidas_son_error) {
    const double nan = std::numeric_limits<double>::quiet_NaN(), inf = std::numeric_limits<double>::infinity();
    MOTOR_CHECK(!emisiones(EstadoEmision{}, nan, 10.0).isOk());
    MOTOR_CHECK(!emisiones(EstadoEmision{}, inf, 10.0).isOk());
    MOTOR_CHECK(!emisiones(EstadoEmision{}, -inf, 10.0).isOk());
    MOTOR_CHECK(!emisiones(EstadoEmision{}, -0.001, 10.0).isOk());
    MOTOR_CHECK(!emisiones(EstadoEmision{}, 0.1, nan).isOk());
    MOTOR_CHECK(!emisiones(EstadoEmision{}, 0.1, inf).isOk());
    MOTOR_CHECK(!emisiones(EstadoEmision{}, 0.1, -1.0).isOk());
    MOTOR_CHECK(!emisiones(EstadoEmision{}, 0.1, kMaxTasa + 0.001).isOk());
    MOTOR_CHECK(emisiones(EstadoEmision{}, 0.1, kMaxTasa).isOk());
    MOTOR_CHECK(!emisiones(EstadoEmision{-0.001}, 0.1, 10.0).isOk());
    MOTOR_CHECK(!emisiones(EstadoEmision{1.0}, 0.1, 10.0).isOk());
    MOTOR_CHECK(!emisiones(EstadoEmision{nan}, 0.1, 10.0).isOk());
    // El error gana aunque el paso sea una pausa
    MOTOR_CHECK(!emisiones(EstadoEmision{}, 0.0, -1.0).isOk());
    MOTOR_CHECK(!emisiones(EstadoEmision{2.0}, 0.0, 0.0).isOk());
    MOTOR_CHECK(emisiones(EstadoEmision{0.9999999999999999}, 0.0, 5.0).isOk());
}

MOTOR_TEST_CASE(emisiones_recorte_de_catch_up) {
    static_assert(kMaxPasoEmision == 0.25 && kTolEmision == 1e-9);
    const auto r = emisiones(EstadoEmision{}, 5.0, 10.0);
    MOTOR_CHECK(r.isOk() && r.value().recortado && r.value().cuantas == 2 && r.value().estado.residuo == 0.5);
    // Justo en el tope: sin recorte; un pelo por encima: recorte con el mismo resultado.
    const auto e = emisiones(EstadoEmision{}, 0.25, 10.0);
    MOTOR_CHECK(e.isOk() && !e.value().recortado && e.value().cuantas == 2 && e.value().estado.residuo == 0.5);
    const auto p = emisiones(EstadoEmision{}, 0.2500001, 10.0);
    MOTOR_CHECK(p.isOk() && p.value().recortado && p.value().cuantas == 2 && p.value().estado.residuo == 0.5);
    // Tasa maxima con dt enorme: 500*0.25 = 125 y nada mas
    const auto g = emisiones(EstadoEmision{}, 1e9, kMaxTasa);
    MOTOR_CHECK(g.isOk() && g.value().recortado && g.value().cuantas == 125 && g.value().estado.residuo == 0.0);
    // Una pausa larga no acumula: 3 x 5 s dan 3 recortes (7,5 part.), no 15 s
    const auto t = corre(3, 5.0, 10.0);
    MOTOR_CHECK(t.ok && t.cuantas == 7 && t.recortes == 3);
    // Con tasa 0 el recorte se informa igualmente
    const auto z = emisiones(EstadoEmision{}, 5.0, 0.0);
    MOTOR_CHECK(z.isOk() && z.value().recortado && z.value().cuantas == 0);
}

MOTOR_TEST_CASE(emisiones_tolerancia_numerica) {
    // 60 x (1/60) a tasa 10: la suma da 9,9999999999999982 y el algoritmo debe dar 10.
    double s = 0.0;
    for (int i = 0; i < 60; ++i) s += 10.0 * (1.0 / 60.0);
    MOTOR_CHECK(s < 10.0 && s > 9.999999999);
    const auto t = corre(60, 1.0 / 60.0, 10.0);
    MOTOR_CHECK(t.ok && t.cuantas == 10 && t.residuo == 0.0);
    // 1000 x 0.001 a tasa 7 == 7
    const auto m = corre(1000, 0.001, 7.0);
    MOTOR_CHECK(m.ok && m.cuantas == 7);
    // Acumulado a 1e-9 del siguiente entero: nace (borde incluido); 2e-9 por debajo con tasa*dt: no
    const auto borde = emisiones(EstadoEmision{0.9999999995}, 0.0001, 1.0);  // 0.9999999995 + 1e-4 = 1.0000999995
    MOTOR_CHECK(borde.isOk() && borde.value().cuantas == 1);
    const auto justo = emisiones(EstadoEmision{0.9999999995}, 1e-12, 1.0);  // 0.9999999995 + 1e-12 + 1e-9 >= 1
    MOTOR_CHECK(justo.isOk() && justo.value().cuantas == 1 && justo.value().estado.residuo == 0.0);
    const auto antes = emisiones(EstadoEmision{0.99999999}, 1e-12, 1.0);      // 1e-8 por debajo: no nace
    MOTOR_CHECK(antes.isOk() && antes.value().cuantas == 0 && antes.value().estado.residuo > 0.99999999);
}

MOTOR_TEST_CASE(emisiones_particion_equivalente) {
    // 3 s de tasa 12 troceados distinto (todos <= kMaxPasoEmision): mismo total, residuo 0.
    const struct { int n; double dt; } trozos[] = {
        {180, 1.0 / 60.0}, {432, 1.0 / 144.0}, {225, 0.016}, {12, 0.25}, {3, 1.0},
    };
    const double tasa = 12.0;
    for (const auto& c : trozos) {
        const auto t = corre(c.n, c.dt, tasa);
        const double total = c.n * c.dt;
        MOTOR_CHECK(t.ok);
        if (c.dt <= kMaxPasoEmision) {
            const int esperado = static_cast<int>(std::floor(tasa * total + 1e-6));
            MOTOR_CHECK(t.cuantas == esperado);
            MOTOR_CHECK(std::fabs(t.residuo - (tasa * total - esperado)) < 1e-6);
        }
    }
    MOTOR_CHECK(corre(180, 1.0 / 60.0, tasa).cuantas == 36);
    MOTOR_CHECK(corre(432, 1.0 / 144.0, tasa).cuantas == 36);
    MOTOR_CHECK(corre(12, 0.25, tasa).cuantas == 36);
    MOTOR_CHECK(corre(1, 0.25, tasa).cuantas == 3 && corre(1, 0.25, tasa).residuo == 0.0);
    // 0.016 x 225 = 3.6 s a tasa 12 = 43,2 -> 43 y residuo ~0.2
    const auto c016 = corre(225, 0.016, tasa);
    MOTOR_CHECK(c016.cuantas == 43 && std::fabs(c016.residuo - 0.2) < 1e-6);
    // 0.3 recortado a 0.25: el total NO es el del tiempo real, y se prueba aparte
    const auto rec = corre(10, 0.3, tasa);
    MOTOR_CHECK(rec.ok && rec.cuantas == 30 && rec.recortes == 10);   // 10 x 0.25 x 12, no 36
    // Un solo paso de 0.25 == 15 de 1/60 (a tasa 12: 3)
    MOTOR_CHECK(corre(15, 0.25 / 15.0, tasa).cuantas == corre(1, 0.25, tasa).cuantas);
}

MOTOR_TEST_CASE(emisiones_determinista_y_residuo_acotado) {
    const auto a = corre(500, 0.0137, 33.3), b = corre(500, 0.0137, 33.3);
    MOTOR_CHECK(a.ok && b.ok && a.cuantas == b.cuantas && a.residuo == b.residuo);
    // barrido: el residuo nunca sale de [0,1) (corre marca ok=false si ocurre)
    for (const double tasa : {0.1, 1.0, 7.0, 40.0, 333.3, 500.0})
        for (const double dt : {1e-6, 0.001, 1.0 / 144.0, 1.0 / 60.0, 0.05, 0.1, 0.25, 0.3, 7.0}) {
            const auto t = corre(200, dt, tasa);
            MOTOR_CHECK(t.ok && t.residuo >= 0.0 && t.residuo < 1.0);
        }
    // un dt minusculo no inventa nacimientos
    MOTOR_CHECK(corre(1000, 1e-12, 40.0).cuantas == 0);
}

MOTOR_TEST_CASE(emisiones_polvo_recuento_exacto_y_corte_en_duracion) {
    // Nube de polvo: tasa 40, duracion 0.15, ticks de 1/60, emision recortada al ultimo tick (9 ticks).
    const auto normal = nube(40.0, 0.15, 1.0 / 60.0);
    MOTOR_CHECK(normal.ok && normal.cuantas == 6);
    const auto reducido = nube(40.0 * 0.25, 0.15, 1.0 / 60.0);   // escalaTasa 0,25 (D-O3)
    MOTOR_CHECK(reducido.ok && reducido.cuantas == 1 && std::fabs(reducido.residuo - 0.5) < 1e-9);
    // Mitad de tick: duracion 0.15 con ticks de 0.1 -> 2o tick emite solo 0.05, no 0.1
    const auto medio = nube(100.0, 0.15, 0.1);
    MOTOR_CHECK(medio.ok && medio.cuantas == 15);
    // Sin recorte al llegar a duracion saldrian 20
    MOTOR_CHECK(corre(2, 0.1, 100.0).cuantas == 20);
    // La escala es multiplicativa sobre la tasa: x0.25 da 1/4 del recuento largo
    MOTOR_CHECK(nube(40.0, 1.0, 1.0 / 60.0).cuantas == 40 && nube(10.0, 1.0, 1.0 / 60.0).cuantas == 10);
    // Polvo reducido: tasa efectiva 10 a 1/60 -> el primer nacimiento llega en el 6.o tick, no antes
    int primero = 0;
    double res = 0.0;
    for (int i = 1; i <= 9; ++i) {
        const auto r = emisiones(EstadoEmision{res}, 1.0 / 60.0, 10.0);
        res = r.value().estado.residuo;
        if (r.value().cuantas == 1 && primero == 0) primero = i;
    }
    MOTOR_CHECK(primero == 6);
}


}  // namespace

int main() { return motor_test::runAllTests(); }
