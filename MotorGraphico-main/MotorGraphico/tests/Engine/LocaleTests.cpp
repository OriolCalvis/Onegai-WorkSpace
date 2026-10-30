// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-17  (fase G, 7 j) - Idioma seleccionable de la interfície
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Engine/Locale.h"

#include "TestHarness.h"

#include <map>
#include <string>
#include <vector>

// G-17 — Idioma seleccionable. Nucleo sin pantalla.
//
// Lo que defiende cada caso es lo que en un proyecto traducido se rompe
// en silencio: una clave que falta y enseña otro idioma sin avisar, un
// hueco que se llama distinto en una tabla que en otra, un parametro que
// nadie pasa y deja un blanco donde iba el numero. Y, sobre todo, que la
// frase se guarde ENTERA con huecos: el motor de hoy la construye sumando
// trozos en orden castellano, y ese orden no vale en ingles.

namespace {

using namespace Engine;

const char* kManifiesto = R"({
  "idioma_de_referencia": "es",
  "idiomas": ["es", "en"]
})";

Locale cargar(const std::string& es, const std::string& en,
              const char* manifiesto = kManifiesto) {
    Locale loc;
    const Result<bool> r = loc.loadFromStrings(manifiesto, {{"es", es}, {"en", en}});
    MOTOR_CHECK(r.isOk());
    return loc;
}

// ---------------------------------------------------------------------
// DEC-15 — el texto lleva acentos, la clave no
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(una_clave_con_acentos_o_mayusculas_no_carga) {
    // La clave es el contrato entre el codigo y las tres tablas. Si
    // admite acentos, el dia que alguien la escriba de otra manera el
    // texto desaparece y no falla nada.
    MOTOR_CHECK(Locale::isValidKey("combate.ataca"));
    MOTOR_CHECK(Locale::isValidKey("grado.exito_critico"));
    MOTOR_CHECK(!Locale::isValidKey("grado.èxit"));
    MOTOR_CHECK(!Locale::isValidKey("Combate.Ataca"));
    MOTOR_CHECK(!Locale::isValidKey("combate..ataca"));
    MOTOR_CHECK(!Locale::isValidKey(".ataca"));
    MOTOR_CHECK(!Locale::isValidKey("ataca."));
    MOTOR_CHECK(!Locale::isValidKey(""));

    Locale loc;
    const Result<bool> r = loc.loadFromStrings(
        kManifiesto, {{"es", R"({"textos":{"grado.èxit":"Èxit"}})"},
                      {"en", R"({"textos":{"grado.exito":"Success"}})"}});
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("DEC-15") != std::string::npos);
    // Y no se ha quedado a medias: una carga fallida no deja media tabla.
    MOTOR_CHECK(!loc.loaded());
}

MOTOR_TEST_CASE(el_texto_visible_si_lleva_acentos_y_enye) {
    Locale loc = cargar(R"({"textos":{"combate.dano":"{n} de daño","grado.exito":"Èxit"}})",
                        R"({"textos":{"combate.dano":"{n} damage","grado.exito":"Success"}})");
    MOTOR_CHECK(loc.text("grado.exito").text == "Èxit");
    MOTOR_CHECK(loc.format("combate.dano", {{"n", "7"}}).text == "7 de daño");
}

// ---------------------------------------------------------------------
// Idioma activo y cambio en caliente
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(cambiar_de_idioma_no_recarga_nada_y_se_nota_en_el_acto) {
    Locale loc = cargar(R"({"textos":{"menu.jugar":"Jugar"}})",
                        R"({"textos":{"menu.jugar":"Play"}})");
    MOTOR_CHECK(loc.language() == "es");
    MOTOR_CHECK(loc.text("menu.jugar").text == "Jugar");

    MOTOR_CHECK(loc.setLanguage("en").isOk());
    MOTOR_CHECK(loc.text("menu.jugar").text == "Play");
    MOTOR_CHECK(loc.setLanguage("es").isOk());
    MOTOR_CHECK(loc.text("menu.jugar").text == "Jugar");
}

MOTOR_TEST_CASE(un_idioma_que_no_esta_en_el_manifiesto_no_se_activa) {
    Locale loc = cargar(R"({"textos":{"menu.jugar":"Jugar"}})",
                        R"({"textos":{"menu.jugar":"Play"}})");
    const Result<bool> r = loc.setLanguage("fr");
    MOTOR_CHECK(!r.isOk());
    // Y el idioma activo no se ha movido.
    MOTOR_CHECK(loc.language() == "es");
}

MOTOR_TEST_CASE(un_manifiesto_sin_idioma_de_referencia_no_carga) {
    // Sin referencia, una clave que falte en un idioma no tendria de
    // donde caer: seria no enseñar nada.
    Locale loc;
    const Result<bool> r = loc.loadFromStrings(R"({"idiomas":["es"]})",
                                               {{"es", R"({"textos":{"a.b":"x"}})"}});
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("idioma_de_referencia") != std::string::npos);
}

MOTOR_TEST_CASE(una_referencia_que_no_esta_en_la_lista_no_carga) {
    Locale loc;
    const Result<bool> r = loc.loadFromStrings(
        R"({"idioma_de_referencia":"fr","idiomas":["es","en"]})",
        {{"es", R"({"textos":{"a.b":"x"}})"}, {"en", R"({"textos":{"a.b":"y"}})"}});
    MOTOR_CHECK(!r.isOk());
}

// ---------------------------------------------------------------------
// Lo que falta se ve
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(una_clave_que_falta_cae_a_la_referencia_y_lo_dice) {
    // Caer al idioma de referencia esta bien; hacerlo CALLADO no. Una
    // pantalla catalana con una linea en castellano parece un descuido
    // de traduccion y no lo es: es una clave que nadie ha traducido.
    Locale loc = cargar(R"({"textos":{"menu.jugar":"Jugar","menu.salir":"Salir"}})",
                        R"({"textos":{"menu.jugar":"Play"}})");
    MOTOR_CHECK(loc.setLanguage("en").isOk());

    const LocalizedText traducido = loc.text("menu.jugar");
    MOTOR_CHECK(traducido.text == "Play");
    MOTOR_CHECK(!traducido.fromFallback);

    const LocalizedText caido = loc.text("menu.salir");
    MOTOR_CHECK(caido.text == "Salir");
    MOTOR_CHECK(caido.fromFallback);
    MOTOR_CHECK(!caido.missing);

    MOTOR_CHECK(loc.missingKeys("en") == std::vector<std::string>{"menu.salir"});
    MOTOR_CHECK(loc.missingKeys("es").empty());
}

MOTOR_TEST_CASE(una_clave_que_no_esta_en_ninguna_tabla_se_ve_en_pantalla) {
    Locale loc = cargar(R"({"textos":{"menu.jugar":"Jugar"}})",
                        R"({"textos":{"menu.jugar":"Play"}})");
    const LocalizedText t = loc.text("menu.no_existe");
    MOTOR_CHECK(t.missing);
    MOTOR_CHECK(t.text == "[menu.no_existe]");
    MOTOR_CHECK(!t.ok());
}

MOTOR_TEST_CASE(una_clave_que_sobra_en_un_idioma_se_dice) {
    // Es lo que queda cuando alguien renombra una clave y se olvida de
    // una tabla: el texto viejo sigue ahi, sin que nadie lo pida.
    Locale loc = cargar(R"({"textos":{"menu.jugar":"Jugar"}})",
                        R"({"textos":{"menu.jugar":"Play","menu.viejo":"Old"}})");
    MOTOR_CHECK(loc.orphanKeys("en") == std::vector<std::string>{"menu.viejo"});
    MOTOR_CHECK(loc.orphanKeys("es").empty());
}

// ---------------------------------------------------------------------
// Huecos con nombre: el motivo de todo esto
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(la_misma_frase_sale_en_otro_orden_segun_el_idioma) {
    // ESTE es el caso que justifica el paquete. El motor de hoy escribe
    //   actor + " ataca a " + objetivo + " (" + grado + ": " + n + " de daño)."
    // y ese orden es castellano. Con huecos con nombre, cada tabla decide
    // donde cae cada cosa y el codigo pasa los mismos parametros.
    Locale loc = cargar(
        R"({"textos":{"combate.ataca":"{actor} ataca a {objetivo} ({grado}: {dano} de daño)."}})",
        R"({"textos":{"combate.ataca":"{grado}! {actor} hits {objetivo} for {dano}."}})");

    const std::map<std::string, std::string> args = {
        {"actor", "Kael"}, {"objetivo", "el golem"}, {"grado", "Èxit"}, {"dano", "7"}};

    const LocalizedText es = loc.format("combate.ataca", args);
    MOTOR_CHECK(es.ok());
    MOTOR_CHECK(es.text == "Kael ataca a el golem (Èxit: 7 de daño).");

    MOTOR_CHECK(loc.setLanguage("en").isOk());
    const LocalizedText en = loc.format("combate.ataca", args);
    MOTOR_CHECK(en.ok());
    MOTOR_CHECK(en.text == "Èxit! Kael hits el golem for 7.");
}

MOTOR_TEST_CASE(un_hueco_sin_valor_se_queda_a_la_vista_y_se_dice) {
    // Rellenarlo con cadena vacia convertiria «(Èxit: 7 de daño)» en
    // «(: de daño)», que parece un texto raro y no un fallo.
    Locale loc = cargar(R"({"textos":{"combate.ataca":"{actor} ataca a {objetivo}."}})",
                        R"({"textos":{"combate.ataca":"{actor} hits {objetivo}."}})");
    const LocalizedText t = loc.format("combate.ataca", {{"actor", "Kael"}});
    MOTOR_CHECK(t.text == "Kael ataca a {objetivo}.");
    MOTOR_CHECK(t.missingArgs == std::vector<std::string>{"objetivo"});
    MOTOR_CHECK(!t.ok());
}

MOTOR_TEST_CASE(un_parametro_que_la_plantilla_no_usa_se_dice) {
    // Casi siempre es un hueco renombrado en una tabla y no en otra: el
    // texto sale "bien" y le falta el numero.
    Locale loc = cargar(R"({"textos":{"combate.ataca":"{actor} ataca."}})",
                        R"({"textos":{"combate.ataca":"{actor} attacks."}})");
    const LocalizedText t = loc.format("combate.ataca", {{"actor", "Kael"}, {"dano", "7"}});
    MOTOR_CHECK(t.text == "Kael ataca.");
    MOTOR_CHECK(t.unusedArgs == std::vector<std::string>{"dano"});
    MOTOR_CHECK(!t.ok());
}

MOTOR_TEST_CASE(dos_llaves_seguidas_son_una_llave_literal) {
    Locale loc = cargar(R"({"textos":{"tec.json":"{{\"id\": {valor}}}"}})",
                        R"({"textos":{"tec.json":"{{\"id\": {valor}}}"}})");
    const LocalizedText t = loc.format("tec.json", {{"valor", "3"}});
    MOTOR_CHECK(t.text == "{\"id\": 3}");
    MOTOR_CHECK(t.ok());
}

MOTOR_TEST_CASE(los_huecos_de_una_plantilla_se_pueden_listar_sin_ejecutarla) {
    const std::vector<std::string> h =
        Locale::placeholdersOf("{actor} ataca a {objetivo} ({grado}: {dano}) y {actor} sonrie");
    MOTOR_CHECK(h.size() == 4);
    MOTOR_CHECK(h[0] == "actor");
    MOTOR_CHECK(h[3] == "dano");
    MOTOR_CHECK(Locale::placeholdersOf("sin huecos").empty());
    MOTOR_CHECK(Locale::placeholdersOf("{{no es hueco}}").empty());
}

MOTOR_TEST_CASE(un_hueco_renombrado_en_un_idioma_se_encuentra_sin_ejecutar_nada) {
    // El fallo clasico y el mas dificil de ver: la frase inglesa esta
    // bien escrita, se lee bien, y le falta el numero para siempre.
    Locale loc = cargar(
        // Delimitador propio: el texto lleva `)"` dentro y cerraria el
        // literal en bruto antes de tiempo.
        R"J({"textos":{"combate.ataca":"{actor} ataca ({dano})","menu.jugar":"Jugar"}})J",
        R"J({"textos":{"combate.ataca":"{actor} attacks ({damage})","menu.jugar":"Play"}})J");

    const std::vector<PlaceholderMismatch> fallos = loc.checkPlaceholders();
    MOTOR_CHECK(fallos.size() == 1);
    MOTOR_CHECK(fallos[0].key == "combate.ataca");
    MOTOR_CHECK(fallos[0].language == "en");
    MOTOR_CHECK(fallos[0].missing == std::vector<std::string>{"dano"});
    MOTOR_CHECK(fallos[0].extra == std::vector<std::string>{"damage"});
}

// ---------------------------------------------------------------------
// Las tablas de verdad
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(las_tres_tablas_publicadas_estan_completas_y_encajan) {
    // Es la prueba que pide el plan: «cada clau existeix en els tres
    // idiomes o cau a la de referencia amb avis». Aqui se exige lo
    // primero: que no falte ninguna. Un idioma incompleto se puede
    // cargar —el motor no se cae— pero no se publica.
    Locale loc;
    const Result<bool> r = loc.loadFromDirectory("assets/i18n");
    MOTOR_CHECK(r.isOk());
    if (!r.isOk()) {
        return;
    }
    MOTOR_CHECK(loc.languages().size() == 3);
    MOTOR_CHECK(loc.referenceLanguage() == "es");

    for (const std::string& code : loc.languages()) {
        MOTOR_CHECK(loc.missingKeys(code).empty());
        MOTOR_CHECK(loc.orphanKeys(code).empty());
    }
    MOTOR_CHECK(loc.checkPlaceholders().empty());
}

MOTOR_TEST_CASE(el_log_de_combate_en_castellano_sale_exactamente_igual_que_hoy) {
    // Las plantillas castellanas se capturaron literalmente de
    // src/Game/BattleState.cpp. Si no reprodujeran la frase de hoy,
    // pasar el log a la tabla cambiaria lo que ve el jugador sin que
    // nadie lo hubiera pedido.
    Locale loc;
    if (!loc.loadFromDirectory("assets/i18n").isOk()) {
        MOTOR_CHECK(false);
        return;
    }
    MOTOR_CHECK(loc.language() == "es");

    MOTOR_CHECK(loc.format("combate.ataca", {{"actor", "Kael"},
                                             {"objetivo", "el golem"},
                                             {"grado", "Èxit"},
                                             {"dano", "7"}})
                    .text == "Kael ataca a el golem (Èxit: 7 de daño).");
    MOTOR_CHECK(loc.format("combate.huye", {{"actor", "Kael"}}).text ==
                "Kael huye del combate.");
    MOTOR_CHECK(loc.format("combate.cura", {{"actor", "Kael"},
                                            {"objetivo", "x"},
                                            {"objeto", "Poción"},
                                            {"cantidad", "12"}})
                    .unusedArgs == std::vector<std::string>{"objetivo"});
    MOTOR_CHECK(loc.format("combate.cura",
                           {{"actor", "Kael"}, {"objeto", "Poción"}, {"cantidad", "12"}})
                    .text == "Kael usa Poción (+12 PV).");
    MOTOR_CHECK(loc.format("combate.objeto_no_esta",
                           {{"actor", "Kael"}, {"objeto", "llave"}})
                    .text == "Kael no tiene \"llave\".");
    MOTOR_CHECK(loc.text("grado.fracaso").text == "Fracàs");
    MOTOR_CHECK(loc.text("grado.critico").text == "Crític");
}

MOTOR_TEST_CASE(en_ingles_las_abreviaturas_cambian_y_la_frase_tambien) {
    Locale loc;
    if (!loc.loadFromDirectory("assets/i18n").isOk()) {
        MOTOR_CHECK(false);
        return;
    }
    MOTOR_CHECK(loc.setLanguage("en").isOk());
    MOTOR_CHECK(loc.format("combate.cura",
                           {{"actor", "Kael"}, {"objeto", "Potion"}, {"cantidad", "12"}})
                    .text == "Kael uses Potion (+12 HP).");
    MOTOR_CHECK(loc.setLanguage("ca").isOk());
    MOTOR_CHECK(loc.format("combate.huye", {{"actor", "Kael"}}).text ==
                "Kael fuig del combat.");
}

}  // namespace

int main() { return motor_test::runAllTests(); }
