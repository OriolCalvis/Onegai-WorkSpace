// U3-06.E02 -- Lectura de maquinas de estados de animacion (Game::leerMaquinaAnimacion /
// leerCatalogoMaquinas). Sin GL. Los JSON se construyen en memoria; no hay assets.
#include "TestHarness.h"

#include "Game/MaquinaAnimacion.h"

#include <set>
#include <string>

using namespace Game;

namespace {

JsonValue json(const std::string& texto) {
    auto r = JsonValue::parse(texto);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

// Catalogo de la prueba: solo "idle" (clip real). Los 9 nombres legacy de HojaPersonaje::animar /
// animarReposo (idle, walk_*, idle_*) cuentan aunque no esten en el catalogo (D-T1).
bool existe(const IdClip& id) {
    static const std::set<std::string> catalogo = {"idle", "golpe"};
    static const std::set<std::string> legacy = {"idle",      "walk_up",    "walk_right",
                                                 "walk_down", "walk_left",  "idle_up",
                                                 "idle_right", "idle_down", "idle_left"};
    return catalogo.count(id.value()) > 0 || legacy.count(id.value()) > 0;
}

// Catalogo sin legacy: para distinguir "en el catalogo" de "solo legacy".
bool soloCatalogo(const IdClip& id) { return id.value() == "idle" || id.value() == "golpe"; }

const char* kPnj = R"({"schemaVersion":1,"maquinas":[{
  "id":"pnj_basico","inicial":"reposo",
  "parametros":[{"id":"andando","tipo":"booleano"}],
  "estados":[
    {"id":"reposo","clipsPorRumbo":{"arriba":"idle_up","derecha":"idle_right","abajo":"idle_down","izquierda":"idle_left"},
     "politicaRumbo":"reiniciarSiDistinto"},
    {"id":"andar","clipsPorRumbo":{"arriba":"walk_up","derecha":"walk_right","abajo":"walk_down","izquierda":"walk_left"},
     "politicaRumbo":"reiniciarSiDistinto"}],
  "transiciones":[
    {"desde":"reposo","hacia":"andar","cuando":[{"param":"andando","op":"cierto"}],"politica":"reiniciarSiempre","prioridad":0},
    {"desde":"andar","hacia":"reposo","cuando":[{"param":"andando","op":"falso"}],"politica":"reiniciarSiempre","prioridad":0}]
}]})";

const char* kEnemigo = R"({"schemaVersion":1,"maquinas":[{"id":"enemigo_base","inicial":"reposo","parametros":[],
  "estados":[{"id":"reposo","clip":"idle"}],"transiciones":[]}]})";

// Documento con una maquina cuyos "estados" y "transiciones" se pasan como texto JSON (arrays).
std::string maq(const std::string& estados, const std::string& transiciones = "[]",
                const std::string& parametros = "[]", const std::string& inicial = "a") {
    return R"({"schemaVersion":1,"maquinas":[{"id":"m","inicial":")" + inicial + R"(","parametros":)" +
           parametros + R"(,"estados":)" + estados + R"(,"transiciones":)" + transiciones + "}]}";
}

const char* kDosEstados = R"([{"id":"a","clip":"idle"},{"id":"b","clip":"golpe"}])";

void falla(const std::string& texto, const std::string& ruta, const std::string& motivo = "",
           bool (*pred)(const IdClip&) = existe) {
    const auto r = leerCatalogoMaquinas(json(texto), pred);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find(ruta) != std::string::npos);
    MOTOR_CHECK(motivo.empty() || r.errorMessage().find(motivo) != std::string::npos);
}

}  // namespace

MOTOR_TEST_CASE(pnj_basico_valido_con_clips_solo_legacy) {
    // Ninguno de los 8 clips esta en el catalogo: todos son nombres legacy validos.
    auto r = leerCatalogoMaquinas(json(kPnj), existe);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().size() == 1);
    const MaquinaAnimacion& m = r.value()[0];
    MOTOR_CHECK(m.id.value() == "pnj_basico");
    MOTOR_CHECK(m.inicial.value() == "reposo");
    MOTOR_CHECK(m.parametros.size() == 1);
    MOTOR_CHECK(m.parametros[0].id.value() == "andando");
    MOTOR_CHECK(m.parametros[0].tipo == TipoParametro::Booleano);
    MOTOR_CHECK(m.estados.size() == 2);
    MOTOR_CHECK(m.estados[0].clipsPorRumbo.size() == 4);
    MOTOR_CHECK(m.estados[0].clipsPorRumbo[0].value() == "idle_up");
    MOTOR_CHECK(m.estados[0].clipsPorRumbo[1].value() == "idle_right");
    MOTOR_CHECK(m.estados[0].clipsPorRumbo[2].value() == "idle_down");
    MOTOR_CHECK(m.estados[0].clipsPorRumbo[3].value() == "idle_left");
    MOTOR_CHECK(m.estados[1].clipsPorRumbo[3].value() == "walk_left");
    MOTOR_CHECK(m.estados[0].politicaRumbo == PoliticaCambio::ReiniciarSiDistinto);
    MOTOR_CHECK(m.transiciones.size() == 2);
    MOTOR_CHECK(m.transiciones[0].desde.has_value() && m.transiciones[0].desde->value() == "reposo");
    MOTOR_CHECK(m.transiciones[0].hacia.value() == "andar");
    MOTOR_CHECK(m.transiciones[0].cuando.size() == 1);
    MOTOR_CHECK(m.transiciones[0].cuando[0].op == OperadorCondicion::Cierto);
    MOTOR_CHECK(m.transiciones[1].cuando[0].op == OperadorCondicion::Falso);
    MOTOR_CHECK(m.transiciones[0].politica == PoliticaCambio::ReiniciarSiempre);
}

MOTOR_TEST_CASE(enemigo_base_minimo_con_listas_vacias) {
    auto r = leerCatalogoMaquinas(json(kEnemigo), soloCatalogo);  // idle esta en el catalogo
    MOTOR_CHECK(r.isOk());
    const MaquinaAnimacion& m = r.value()[0];
    MOTOR_CHECK(m.id.value() == "enemigo_base");
    MOTOR_CHECK(m.parametros.empty() && m.transiciones.empty());
    MOTOR_CHECK(m.estados.size() == 1 && m.estados[0].clip.value() == "idle");
    MOTOR_CHECK(m.estados[0].clipsPorRumbo.empty());
    // Tambien por la via de una sola maquina.
    auto una = leerMaquinaAnimacion(json(kEnemigo)["maquinas"][0], soloCatalogo);
    MOTOR_CHECK(una.isOk() && una.value().id.value() == "enemigo_base");
}

MOTOR_TEST_CASE(clip_ausente_que_no_es_legacy_es_error_y_legacy_fuera_del_catalogo_se_acepta) {
    const std::string desconocido = maq(R"([{"id":"a","clip":"no_existe"}])");
    falla(desconocido, "maquinas[0].estados[0].clip", "clip no existe: no_existe");
    const std::string legacy = maq(R"([{"id":"a","clip":"idle_up"}])");
    MOTOR_CHECK(leerCatalogoMaquinas(json(legacy), existe).isOk());                // legacy, fuera del catalogo
    MOTOR_CHECK(!leerCatalogoMaquinas(json(legacy), soloCatalogo).isOk());         // sin legacy: error
    // El error por rumbo apunta a la clave concreta.
    falla(maq(R"([{"id":"a","clipsPorRumbo":{"arriba":"idle_up","derecha":"idle_right","abajo":"fantasma","izquierda":"idle_left"}}])"),
          "estados[0].clipsPorRumbo.abajo", "clip no existe: fantasma");
    // Predicado vacio = ningun clip existe.
    MOTOR_CHECK(!leerCatalogoMaquinas(json(kEnemigo), ExisteClip{}).isOk());
}

MOTOR_TEST_CASE(schema_version_ausente_o_distinta) {
    falla(R"({"maquinas":[]})", "schemaVersion");
    falla(R"({"schemaVersion":0,"maquinas":[]})", "schemaVersion");
    falla(R"({"schemaVersion":2,"maquinas":[]})", "schemaVersion");
    falla(R"({"schemaVersion":"1","maquinas":[]})", "schemaVersion");
    falla(R"({"schemaVersion":1})", "maquinas");
    falla(R"({"schemaVersion":1,"maquinas":[]})", "maquinas");
}

MOTOR_TEST_CASE(ids_repetidos_maquina_estado_parametro) {
    falla(R"({"schemaVersion":1,"maquinas":[
      {"id":"m","inicial":"a","estados":[{"id":"a","clip":"idle"}]},
      {"id":"m","inicial":"a","estados":[{"id":"a","clip":"idle"}]}]})",
          "maquinas[1].id", "maquina repetida");
    falla(maq(R"([{"id":"a","clip":"idle"},{"id":"b","clip":"golpe"},{"id":"a","clip":"idle"}])"),
          "maquinas[0].estados[2].id", "estado repetido");
    falla(maq(kDosEstados, "[]", R"([{"id":"x","tipo":"booleano"},{"id":"x","tipo":"entero"}])"),
          "maquinas[0].parametros[1].id", "parametro repetido");
}

MOTOR_TEST_CASE(parametro_con_nombre_reservado_o_tipo_malo) {
    falla(maq(kDosEstados, "[]", R"([{"id":"rumbo","tipo":"entero"}])"), "parametros[0].id", "reservado");
    falla(maq(kDosEstados, "[]", R"([{"id":"finClip","tipo":"booleano"}])"), "parametros[0].id", "reservado");
    falla(maq(kDosEstados, "[]", R"([{"id":"x","tipo":"cadena"}])"), "parametros[0].tipo", "tipo desconocido");
    falla(maq(kDosEstados, "[]", R"([{"id":"x"}])"), "parametros[0].tipo");
}

MOTOR_TEST_CASE(inicial_ausente_o_desconocido) {
    falla(maq(kDosEstados, "[]", "[]", "zzz"), "maquinas[0].inicial", "estado desconocido: zzz");
    falla(R"({"schemaVersion":1,"maquinas":[{"id":"m","estados":[{"id":"a","clip":"idle"}]}]})",
          "maquinas[0].inicial");
}

MOTOR_TEST_CASE(estado_con_dos_modos_o_ninguno) {
    falla(maq(R"([{"id":"a","clip":"idle","clipsPorRumbo":{"arriba":"idle","derecha":"idle","abajo":"idle","izquierda":"idle"}}])"),
          "estados[0]", "exactamente uno");
    falla(maq(R"([{"id":"a"}])"), "estados[0]", "exactamente uno");
}

MOTOR_TEST_CASE(clips_por_rumbo_con_distinto_de_cuatro_entradas) {
    falla(maq(R"([{"id":"a","clipsPorRumbo":{"arriba":"idle","derecha":"idle","abajo":"idle"}}])"),
          "estados[0].clipsPorRumbo", "exactamente 4 entradas (hay 3)");
    falla(maq(R"([{"id":"a","clipsPorRumbo":{"arriba":"idle","derecha":"idle","abajo":"idle","izquierda":"idle","centro":"idle"}}])"),
          "estados[0].clipsPorRumbo", "(hay 5)");
    // 4 entradas pero una clave ajena: falta "izquierda".
    falla(maq(R"([{"id":"a","clipsPorRumbo":{"arriba":"idle","derecha":"idle","abajo":"idle","centro":"idle"}}])"),
          "estados[0].clipsPorRumbo.izquierda", "ausente");
    falla(maq(R"([{"id":"a","clipsPorRumbo":[]}])"), "estados[0].clipsPorRumbo");
    falla(maq(R"([{"id":"a","clip":"idle","politicaRumbo":"conservarFase"}])"), "estados[0].politicaRumbo");
    falla(maq(R"([{"id":"a","clipsPorRumbo":{"arriba":"idle","derecha":"idle","abajo":"idle","izquierda":"idle"},"politicaRumbo":"x"}])"),
          "estados[0].politicaRumbo", "politica desconocida");
}

MOTOR_TEST_CASE(transicion_a_estado_inexistente) {
    falla(maq(kDosEstados, R"([{"desde":"a","hacia":"fantasma","esperaFinClip":true}])"),
          "maquinas[0].transiciones[0].hacia", "estado desconocido: fantasma");
    falla(maq(kDosEstados, R"([{"hacia":"b","esperaFinClip":true},{"desde":"fantasma","hacia":"b","esperaFinClip":true}])"),
          "transiciones[1].desde", "estado desconocido: fantasma");
    falla(maq(kDosEstados, R"([{"desde":"a","esperaFinClip":true}])"), "transiciones[0].hacia");
}

MOTOR_TEST_CASE(condicion_con_parametro_no_declarado_op_desconocido_o_incompatible) {
    const std::string p = R"([{"id":"andando","tipo":"booleano"},{"id":"vel","tipo":"real"},{"id":"ataque","tipo":"disparo"}])";
    auto trans = [](const std::string& cond) {
        return "[{\"desde\":\"a\",\"hacia\":\"b\",\"cuando\":[" + cond + "]}]";
    };
    falla(maq(kDosEstados, trans(R"({"param":"fantasma","op":"cierto"})"), p),
          "transiciones[0].cuando[0].param", "no declarado: fantasma");
    falla(maq(kDosEstados, trans(R"({"param":"andando","op":"quizas"})"), p),
          "cuando[0].op", "desconocido: quizas");
    falla(maq(kDosEstados, trans(R"({"param":"andando","op":"mayor","valor":1})"), p),
          "cuando[0].op", "mayor no aplica a parametro booleano");
    falla(maq(kDosEstados, trans(R"({"param":"vel","op":"cierto"})"), p),
          "cuando[0].op", "no aplica a parametro real");
    falla(maq(kDosEstados, trans(R"({"param":"ataque","op":"cierto"})"), p),
          "cuando[0].op", "no aplica a parametro disparo");
    falla(maq(kDosEstados, trans(R"({"param":"vel","op":"mayor"})"), p), "cuando[0].valor", "ausente");
    falla(maq(kDosEstados, trans(R"({"param":"vel","op":"mayor","valor":"1"})"), p),
          "cuando[0].valor", "numero");
    // Validas: parametros reservados sin declarar, disparo y numerico con valor.
    const std::string ok = "[{\"desde\":\"a\",\"hacia\":\"b\",\"cuando\":[" +
                           std::string(R"({"param":"rumbo","op":"igual","valor":2},)") +
                           R"({"param":"finClip","op":"cierto"},{"param":"ataque","op":"disparado"},)" +
                           R"({"param":"vel","op":"mayorIgual","valor":0.5}]}])";
    auto r = leerCatalogoMaquinas(json(maq(kDosEstados, ok, p)), existe);
    MOTOR_CHECK(r.isOk());
    const auto& c = r.value()[0].transiciones[0].cuando;
    MOTOR_CHECK(c.size() == 4);
    MOTOR_CHECK(c[0].op == OperadorCondicion::Igual && c[0].valor == 2.0);
    MOTOR_CHECK(c[1].parametro.value() == kParametroFinClip);  // "finClip" del JSON, id interno
    MOTOR_CHECK(c[2].op == OperadorCondicion::Disparado);
    MOTOR_CHECK(c[3].op == OperadorCondicion::MayorIgual && c[3].valor == 0.5);
}

MOTOR_TEST_CASE(transicion_sin_condiciones_tipos_erroneos_y_prioridad) {
    falla(maq(kDosEstados, R"([{"desde":"a","hacia":"b"}])"), "transiciones[0].cuando", "siempre cierta");
    falla(maq(kDosEstados, R"([{"desde":"a","hacia":"b","cuando":[]}])"), "transiciones[0].cuando");
    falla(maq(kDosEstados, R"([{"desde":"a","hacia":"b","cuando":3}])"), "transiciones[0].cuando", "array");
    falla(maq(kDosEstados, R"([{"desde":"a","hacia":"b","esperaFinClip":"si"}])"), "transiciones[0].esperaFinClip");
    falla(maq(kDosEstados, R"([{"desde":"a","hacia":"b","esperaFinClip":true,"prioridad":1.5}])"),
          "transiciones[0].prioridad", "entero");
    falla(maq(kDosEstados, R"([{"desde":"a","hacia":"b","esperaFinClip":true,"prioridad":"1"}])"),
          "transiciones[0].prioridad");
    falla(maq(kDosEstados, R"([{"desde":"a","hacia":"b","esperaFinClip":true,"politica":"nunca"}])"),
          "transiciones[0].politica", "politica desconocida");
    falla(maq(kDosEstados, R"([7])"), "transiciones[0]", "objeto");
    falla(maq(kDosEstados, "{}"), "maquinas[0].transiciones", "array");
}

MOTOR_TEST_CASE(orden_prioridad_y_defecto_de_transiciones) {
    const std::string t = R"([
      {"desde":"a","hacia":"b","esperaFinClip":true,"prioridad":5},
      {"hacia":"a","esperaFinClip":true,"prioridad":-2,"politica":"conservarFase"},
      {"desde":"b","hacia":"a","esperaFinClip":true}])";
    auto r = leerCatalogoMaquinas(json(maq(kDosEstados, t)), existe);
    MOTOR_CHECK(r.isOk());
    const auto& ts = r.value()[0].transiciones;
    MOTOR_CHECK(ts.size() == 3);
    for (std::size_t i = 0; i < ts.size(); ++i) {
        MOTOR_CHECK(ts[i].orden == static_cast<int>(i));  // posicion del JSON, sin reordenar
    }
    MOTOR_CHECK(ts[0].prioridad == 5 && ts[1].prioridad == -2 && ts[2].prioridad == 0);
    MOTOR_CHECK(ts[0].hacia.value() == "b" && ts[1].hacia.value() == "a" && ts[2].desde->value() == "b");
    MOTOR_CHECK(!ts[1].desde.has_value());  // sin "desde" = cualquier estado
    MOTOR_CHECK(ts[1].politica == PoliticaCambio::ConservarFase);
    MOTOR_CHECK(ts[2].politica == PoliticaCambio::ReiniciarSiempre);
    MOTOR_CHECK(ts[0].esperaFinClip && ts[0].cuando.empty());
}

MOTOR_TEST_CASE(raiz_y_maquina_no_objeto) {
    MOTOR_CHECK(!leerCatalogoMaquinas(json("[]"), existe).isOk());
    auto r = leerMaquinaAnimacion(json("3"), existe);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("objeto") != std::string::npos);
    falla(R"({"schemaVersion":1,"maquinas":[5]})", "maquinas[0]", "objeto");
}

int main() { return motor_test::runAllTests(); }
