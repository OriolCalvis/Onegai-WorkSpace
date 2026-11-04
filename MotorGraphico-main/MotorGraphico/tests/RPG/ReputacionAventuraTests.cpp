#include "RPG/NarrativeEngine.h"
#include "RPG/Narrative/AdventureScript.h"
#include "RPG/Narrative/NarrativeState.h"
#include "Narrative/ReputacionAventura.h"

#include "TestHarness.h"

#include <filesystem>
#include <fstream>
#include <string>

// BND-04 — requires.reputacion: la reputacion con facciones como condicion de
// un beat. min y max incluidos y opcionales; varias facciones, todas; una
// faccion que no existe es error de carga; lo de antes (flags) sigue igual.

namespace {

void facciones() { RPG::reputacionAventura::ponFaccionesConocidas({"aegroum", "perdidos", "barriada_surysal"}); }

// Un beat de charla con el requires que se le pase.
std::string aventura(const std::string& requires) {
    return R"({"id": "rep", "beats": [{"id": "b", "trigger": {"type": "talk", "target": "pnj"}, "requires": )" +
           requires + R"(, "lines": ["Hola."]}]})";
}

RPG::AdventureScript carga(const std::string& requires) {
    auto r = RPG::AdventureScript::loadFromString(aventura(requires));
    MOTOR_CHECK(r.isOk());
    return r.isOk() ? r.value() : RPG::AdventureScript{};
}

// ¿Salta el beat con esta reputacion (y estas flags)?
bool salta(const std::string& requires, std::map<std::string, int> rep, const std::string& flag = "") {
    const RPG::AdventureScript a = carga(requires);
    RPG::NarrativeEngine motor;
    motor.setAdventure(&a);
    RPG::NarrativeState s;
    s.ponReputaciones(std::move(rep));
    if (!flag.empty()) s.setFlag(flag);
    return motor.talkTo("pnj", s).fired;
}

}  // namespace

MOTOR_TEST_CASE(el_minimo_se_cumple_desde_el_valor_exacto) {
    facciones();
    const std::string r = R"({"reputacion": {"aegroum": {"min": 10}}})";
    MOTOR_CHECK(!salta(r, {{"aegroum", 9}}));
    MOTOR_CHECK(salta(r, {{"aegroum", 10}}));
    MOTOR_CHECK(salta(r, {{"aegroum", 100}}));
    MOTOR_CHECK(!salta(r, {}));  // sin tocar: 0
}

MOTOR_TEST_CASE(el_maximo_se_cumple_hasta_el_valor_exacto) {
    facciones();
    const std::string r = R"({"reputacion": {"perdidos": {"max": -15}}})";
    MOTOR_CHECK(salta(r, {{"perdidos", -15}}));
    MOTOR_CHECK(salta(r, {{"perdidos", -100}}));
    MOTOR_CHECK(!salta(r, {{"perdidos", -14}}));
    MOTOR_CHECK(!salta(r, {}));
}

MOTOR_TEST_CASE(minimo_y_maximo_juntos_es_un_tramo) {
    facciones();
    const std::string r = R"({"reputacion": {"aegroum": {"min": -5, "max": 5}}})";
    MOTOR_CHECK(salta(r, {}));
    MOTOR_CHECK(salta(r, {{"aegroum", -5}}) && salta(r, {{"aegroum", 5}}));
    MOTOR_CHECK(!salta(r, {{"aegroum", 6}}) && !salta(r, {{"aegroum", -6}}));
}

MOTOR_TEST_CASE(con_dos_facciones_hacen_falta_las_dos) {
    facciones();
    const std::string r = R"({"reputacion": {"aegroum": {"min": 10}, "perdidos": {"max": 0}}})";
    MOTOR_CHECK(salta(r, {{"aegroum", 10}, {"perdidos", 0}}));
    MOTOR_CHECK(!salta(r, {{"aegroum", 9}, {"perdidos", 0}}));
    MOTOR_CHECK(!salta(r, {{"aegroum", 10}, {"perdidos", 1}}));
}

MOTOR_TEST_CASE(una_faccion_desconocida_o_un_rango_roto_es_error_de_carga) {
    facciones();
    auto mal = [](const std::string& requires) { return !RPG::AdventureScript::loadFromString(aventura(requires)).isOk(); };
    MOTOR_CHECK(mal(R"({"reputacion": {"gremio_inventado": {"min": 1}}})"));
    MOTOR_CHECK(mal(R"({"reputacion": {"aegroum": {}}})"));
    MOTOR_CHECK(mal(R"({"reputacion": {"aegroum": 5}})"));
    MOTOR_CHECK(mal(R"({"reputacion": {"aegroum": {"min": 20, "max": 10}}})"));
    MOTOR_CHECK(mal(R"({"reputacion": {"aegroum": {"min": -101}}})"));
    MOTOR_CHECK(mal(R"({"reputacion": ["aegroum"]})"));
    MOTOR_CHECK(!mal(R"({"reputacion": {"barriada_surysal": {"min": -100, "max": 100}}})"));
}

MOTOR_TEST_CASE(las_condiciones_de_antes_siguen_igual_y_se_combinan) {
    facciones();
    const std::string solo = R"({"allFlags": ["hablo"]})";
    MOTOR_CHECK(!salta(solo, {{"aegroum", 50}}) && salta(solo, {}, "hablo"));  // sin reputacion, como siempre
    const std::string junto = R"({"allFlags": ["hablo"], "reputacion": {"aegroum": {"min": 10}}})";
    MOTOR_CHECK(salta(junto, {{"aegroum", 10}}, "hablo"));
    MOTOR_CHECK(!salta(junto, {{"aegroum", 10}}));           // falta la flag
    MOTOR_CHECK(!salta(junto, {{"aegroum", 9}}, "hablo"));   // falta la reputacion
}

MOTOR_TEST_CASE(la_reputacion_se_escribe_y_se_lee_de_vuelta_igual) {
    facciones();
    const RPG::AdventureScript a = carga(R"({"reputacion": {"aegroum": {"min": 10}, "perdidos": {"min": -3, "max": 7}}})");
    MOTOR_CHECK(a.beats[0].requirements.reputaciones.size() == 2);
    const std::string una = RPG::serializeAdventure(a);
    auto otra = RPG::AdventureScript::loadFromString(una);
    MOTOR_CHECK(otra.isOk() && RPG::serializeAdventure(otra.value()) == una);
    MOTOR_CHECK(una.find("\"reputacion\"") != std::string::npos);
}

MOTOR_TEST_CASE(claves_desconocidas_y_valores_no_enteros_se_rechazan_como_en_python) {
    facciones();
    auto mal = [](const std::string& requires) { return !RPG::AdventureScript::loadFromString(aventura(requires)).isOk(); };
    MOTOR_CHECK(mal(R"({"reputacion": {"aegroum": {"min": 1, "tope": 5}}})"));
    MOTOR_CHECK(mal(R"({"reputacion": {"aegroum": {"minimo": 1}}})"));
    MOTOR_CHECK(mal(R"({"reputacion": {"aegroum": {"min": 1.5}}})"));
    MOTOR_CHECK(mal(R"({"reputacion": {"aegroum": {"max": "10"}}})"));
    MOTOR_CHECK(mal(R"({"reputacion": {"aegroum": {"max": true}}})"));
    MOTOR_CHECK(!mal(R"({"reputacion": {"aegroum": {"min": 1, "max": 5}}})"));
}

MOTOR_TEST_CASE(sin_facciones_json_da_error_con_el_camino_y_no_se_guarda_vacio) {
    namespace fs = std::filesystem;
    const fs::path aqui = fs::current_path();
    const fs::path vacio = fs::temp_directory_path() / "bnd04_sin_assets";
    fs::create_directories(vacio);
    RPG::reputacionAventura::ponFaccionesConocidas({});  // vuelve a leer del fichero
    fs::current_path(vacio);
    auto r = RPG::AdventureScript::loadFromString(aventura(R"({"reputacion": {"aegroum": {"min": 1}}})"));
    fs::current_path(aqui);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(!r.isOk() && r.errorMessage().find("facciones.json") != std::string::npos &&
                r.errorMessage().find("bnd04_sin_assets") != std::string::npos);
    // Desde un directorio con assets/, la siguiente carga lo encuentra: no quedo guardado el vacio.
    fs::create_directories(vacio / "assets" / "rules");
    {
        std::ofstream f(vacio / "assets" / "rules" / "facciones.json");
        f << R"({"facciones": [{"id": "aegroum"}]})";
    }
    fs::current_path(vacio);
    auto bien = RPG::AdventureScript::loadFromString(aventura(R"({"reputacion": {"aegroum": {"min": 1}}})"));
    fs::current_path(aqui);
    MOTOR_CHECK(bien.isOk());
    RPG::reputacionAventura::ponFaccionesConocidas({});
    fs::remove_all(vacio);
}

int main() { return motor_test::runAllTests(); }
