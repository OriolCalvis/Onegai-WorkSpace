// U4-07.E02 — tests de leerEmisor (definicion del emisor de particulas). GL-free.
#include "Game/Emisor/DefinicionEmisor.h"
#include "Game/Personajes/FamiliasDeParticulas.h"

#include "TestHarness.h"

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

}  // namespace

int main() { return motor_test::runAllTests(); }
