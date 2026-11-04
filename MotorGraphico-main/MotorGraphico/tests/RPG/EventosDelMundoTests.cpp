#include "Core/Json/JsonValue.h"
#include "RPG/Narrative/EventosDelMundo.h"
#include "RPG/Narrative/NarrativeState.h"

#include "TestHarness.h"

#include <cstdio>
#include <string>

// C8-03b — Lectura y validación del catálogo de eventos del mundo. Cada regla
// de FORMATO_EVENTOS.md tiene al menos un caso que la rompe.

namespace {

using RPG::eventos::Catalogo;
using RPG::eventos::Disparador;
using RPG::eventos::Evento;

Result<Catalogo> lee(const std::string& texto) {
    auto json = JsonValue::parse(texto);
    MOTOR_CHECK(json.isOk());
    return RPG::eventos::leeCatalogo(json.value(), "prueba.json");
}

// Un catálogo con los momentos de siempre y el evento que se le pasa.
std::string con(const std::string& eventos, const std::string& extra = "") {
    return "{\"momentos\":{\"ocaso_puesta\":{\"dia\":4,\"franja\":2}}," + extra + "\"eventos\":[" + eventos + "]}";
}

// Un evento válido al que se le añade (o se le cambia) lo que se quiere romper.
std::string ev(const std::string& id, const std::string& resto) {
    return "{\"id\":\"" + id + "\"," + resto + "}";
}
const char* kBase = "\"ausencia\":\"avanza\",\"disparadores\":[{\"cruzaMomento\":\"ocaso_puesta\"}],"
                    "\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}]";

// Debe fallar, y el mensaje debe nombrar el fichero, el evento y el campo.
void fallaCon(const std::string& texto, const std::string& evento, const std::string& campo) {
    auto r = lee(texto);
    MOTOR_CHECK(!r.isOk());
    if (r.isOk()) return;
    const std::string& m = r.errorMessage();
    MOTOR_CHECK(m.find("prueba.json") != std::string::npos);
    MOTOR_CHECK(m.find(" :: " + evento + " :: ") != std::string::npos);
    MOTOR_CHECK(m.find(campo) != std::string::npos);
}

MOTOR_TEST_CASE(el_ocaso_del_contrato_se_carga_con_sus_tres_disparadores_en_orden) {
    auto r = lee(con(
        "{\"id\":\"comienza_ocaso\",\"ausencia\":\"avanza\",\"condiciones\":{\"notFlags\":[\"bnd3_campanas\"]},"
        "\"disparadores\":[{\"id\":\"aviso_venides\",\"senal\":\"bnd3_avisado\"},"
        "{\"id\":\"aviso_xila\",\"senal\":\"bnd3_xila_avisa\",\"condiciones\":{\"allFlags\":[\"bnd1_xila_honor\"]}},"
        "{\"id\":\"puesta_sol\",\"cruzaMomento\":\"ocaso_puesta\"}],"
        "\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"bnd3_campanas\"}],\"escenas\":[{\"cinematica\":\"base_campanas\"}]}",
        "\"banderasDeObservacion\":[\"bnd3_plaza_vista\",\"bnd3_arenga\"],"));
    MOTOR_CHECK(r.isOk());
    if (!r.isOk()) return;
    const Evento* e = r.value().busca("comienza_ocaso");
    MOTOR_CHECK(e != nullptr);
    if (!e) return;
    MOTOR_CHECK(r.value().busca("nada") == nullptr);
    MOTOR_CHECK(e->disparadores.size() == 3);
    MOTOR_CHECK(e->disparadores[0].id == "aviso_venides" && e->disparadores[0].tipo == Disparador::Tipo::Senal);
    MOTOR_CHECK(e->disparadores[1].id == "aviso_xila" && e->disparadores[1].condiciones.allFlags.size() == 1);
    MOTOR_CHECK(e->disparadores[2].tipo == Disparador::Tipo::CruzaMomento);
    MOTOR_CHECK(e->disparadores[2].momento.dia == 4 && e->disparadores[2].momento.franja == 2);
    MOTOR_CHECK(e->ausencia == Evento::Ausencia::Avanza);
    MOTOR_CHECK(e->condiciones.notFlags.size() == 1 && e->efectos.size() == 1);
}

MOTOR_TEST_CASE(fases_y_evento_previo_con_fase_se_resuelven) {
    auto r = lee(con(
        ev("sermon", "\"ausencia\":\"avanza\",\"disparadores\":[{\"cruzaMomento\":{\"dia\":2,\"franja\":1}}],"
                     "\"fases\":[{\"id\":\"activo\",\"disparadores\":[{\"cruzaMomento\":{\"dia\":2,\"franja\":1}}],"
                     "\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"sermon_on\"}]},"
                     "{\"id\":\"acabado\",\"disparadores\":[{\"cruzaMomento\":{\"dia\":3,\"franja\":0}}],"
                     "\"efectos\":[{\"type\":\"clearFlag\",\"arg\":\"sermon_on\"}]}]") + "," +
        ev("despues", "\"ausencia\":\"espera\",\"motivo\":\"que el jugador hable con Venides\","
                      "\"ventana\":{\"desde\":{\"dia\":1,\"franja\":0},\"hasta\":\"ocaso_puesta\"},"
                      "\"disparadores\":[{\"eventoPrevio\":\"sermon.acabado\"},{\"eventoPrevio\":\"sermon\"}],"
                      "\"efectos\":[{\"type\":\"addVariable\",\"arg\":\"pasos\",\"value\":1}]")));
    MOTOR_CHECK(r.isOk());
    if (!r.isOk()) return;
    const Evento* s = r.value().busca("sermon");
    MOTOR_CHECK(s && s->fases.size() == 2 && s->fases[1].id == "acabado" && s->efectos.empty());
    const Evento* d = r.value().busca("despues");
    MOTOR_CHECK(d && d->ausencia == Evento::Ausencia::Espera && d->tieneDesde && d->tieneHasta);
    MOTOR_CHECK(d && d->hasta.dia == 4 && d->hasta.franja == 2);
    MOTOR_CHECK(d && d->disparadores[0].evento == "sermon" && d->disparadores[0].fase == "acabado");
    MOTOR_CHECK(d && d->disparadores[1].fase.empty());
}

MOTOR_TEST_CASE(caduca_con_hasta_y_al_caducar_se_carga) {
    auto r = lee(con(ev("e", "\"ausencia\":\"caduca\",\"ventana\":{\"hasta\":\"ocaso_puesta\"},"
                             "\"alCaducar\":[{\"type\":\"setFlag\",\"arg\":\"tarde\"}],"
                             "\"disparadores\":[{\"senal\":\"s\"}],\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}]")));
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.isOk() && r.value().eventos[0].alCaducar.size() == 1);
}

MOTOR_TEST_CASE(lugar_en_una_condicion_es_un_error) {
    fallaCon(con(ev("e", std::string(kBase) + ",\"condiciones\":{\"lugar\":[\"plaza\"]}")), "e", "no depende de dónde está el jugador");
    fallaCon(con(ev("e", "\"ausencia\":\"avanza\",\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}],"
                         "\"disparadores\":[{\"senal\":\"s\",\"condiciones\":{\"lugar\":\"plaza\"}}]")),
             "e", "no depende de dónde está el jugador");
}

MOTOR_TEST_CASE(una_clave_desconocida_en_una_condicion_es_un_error) {
    fallaCon(con(ev("e", std::string(kBase) + ",\"condiciones\":{\"tipo\":1}")), "e", "clave «tipo» desconocida");
}

MOTOR_TEST_CASE(una_clave_desconocida_en_el_evento_es_un_error) {
    fallaCon(con(ev("e", std::string(kBase) + ",\"lugar\":\"plaza\"")), "e", "lugar");
    fallaCon(con(ev("e", std::string(kBase) + ",\"inventada\":1")), "e", "inventada");
}

MOTOR_TEST_CASE(un_disparador_es_exactamente_uno_de_los_tres) {
    const std::string cola = "\"ausencia\":\"avanza\",\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}]";
    fallaCon(con(ev("e", "\"disparadores\":[{\"id\":\"vacio\"}]," + cola)), "e", "disparadores[0]");
    fallaCon(con(ev("e", "\"disparadores\":[{\"senal\":\"s\",\"cruzaMomento\":\"ocaso_puesta\"}]," + cola)), "e", "exactamente");
    fallaCon(con(ev("e", "\"disparadores\":[{\"entrar\":\"plaza\"}]," + cola)), "e", "entrar");
    fallaCon(con(ev("e", "\"disparadores\":[]," + cola)), "e", "disparadores");
}

MOTOR_TEST_CASE(un_momento_sin_resolver_o_mal_formado_es_un_error) {
    const std::string cola = "\"ausencia\":\"avanza\",\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}]";
    fallaCon(con(ev("e", "\"disparadores\":[{\"cruzaMomento\":\"no_existe\"}]," + cola)), "e", "no_existe");
    fallaCon(con(ev("e", "\"disparadores\":[{\"cruzaMomento\":{\"dia\":1,\"franja\":3}}]," + cola)), "e", "cruzaMomento");
    fallaCon(con(ev("e", "\"disparadores\":[{\"cruzaMomento\":{\"dia\":0,\"franja\":1}}]," + cola)), "e", "cruzaMomento");
    fallaCon(con(ev("e", "\"disparadores\":[{\"cruzaMomento\":{\"dia\":2}}]," + cola)), "e", "cruzaMomento");
    fallaCon("{\"momentos\":{\"malo\":{\"dia\":1,\"franja\":9}},\"eventos\":[]}", "momentos", "malo");
}

MOTOR_TEST_CASE(una_ventana_al_reves_es_un_error) {
    fallaCon(con(ev("e", std::string(kBase) + ",\"ventana\":{\"desde\":\"ocaso_puesta\",\"hasta\":{\"dia\":4,\"franja\":1}}")),
             "e", "ventana");
    fallaCon(con(ev("e", std::string(kBase) + ",\"ventana\":{\"desde\":\"ocaso_puesta\",\"hasta\":\"ocaso_puesta\"}")),
             "e", "ventana");
}

MOTOR_TEST_CASE(la_ausencia_falta_o_es_desconocida) {
    const std::string cola = "\"disparadores\":[{\"senal\":\"s\"}],\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}]";
    fallaCon(con(ev("e", cola)), "e", "ausencia");
    fallaCon(con(ev("e", "\"ausencia\":\"quizas\"," + cola)), "e", "ausencia");
}

MOTOR_TEST_CASE(caduca_necesita_hasta_y_al_caducar) {
    const std::string cola = "\"disparadores\":[{\"senal\":\"s\"}],\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}]";
    fallaCon(con(ev("e", "\"ausencia\":\"caduca\",\"alCaducar\":[{\"type\":\"setFlag\",\"arg\":\"t\"}]," + cola)),
             "e", "ventana.hasta");
    fallaCon(con(ev("e", "\"ausencia\":\"caduca\",\"ventana\":{\"hasta\":\"ocaso_puesta\"}," + cola)), "e", "alCaducar");
}

MOTOR_TEST_CASE(espera_necesita_motivo) {
    fallaCon(con(ev("e", "\"ausencia\":\"espera\",\"disparadores\":[{\"senal\":\"s\"}],"
                         "\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}]")), "e", "motivo");
}

MOTOR_TEST_CASE(solo_valen_los_efectos_de_estado_y_nunca_sobre_el_tiempo) {
    const std::string ini = "\"ausencia\":\"avanza\",\"disparadores\":[{\"senal\":\"s\"}],";
    fallaCon(con(ev("e", ini + "\"efectos\":[{\"type\":\"log\",\"arg\":\"hola\"}]")), "e", "efectos[0]");
    fallaCon(con(ev("e", ini + "\"efectos\":[{\"type\":\"grantGold\",\"value\":5}]")), "e", "efectos[0]");
    fallaCon(con(ev("e", ini + "\"efectos\":[{\"type\":\"pasarFranja\"}]")), "e", "efectos[0]");
    fallaCon(con(ev("e", ini + "\"efectos\":[{\"type\":\"setVariable\",\"arg\":\"dia\",\"value\":3}]")), "e", "dia");
    fallaCon(con(ev("e", ini + "\"efectos\":[{\"type\":\"addVariable\",\"arg\":\"franja\",\"value\":1}]")), "e", "franja");
    fallaCon(con(ev("e", ini + "\"efectos\":[{\"type\":\"setFlag\"}]")), "e", "falta «arg»");
    // el mismo criterio vale para alCaducar y para las fases
    fallaCon(con(ev("e", "\"ausencia\":\"caduca\",\"ventana\":{\"hasta\":\"ocaso_puesta\"},\"disparadores\":[{\"senal\":\"s\"}],"
                         "\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}],\"alCaducar\":[{\"type\":\"log\",\"arg\":\"a\"}]")),
             "e", "alCaducar[0]");
    fallaCon(con(ev("e", ini + "\"fases\":[{\"id\":\"f\",\"disparadores\":[{\"senal\":\"s\"}],"
                                "\"efectos\":[{\"type\":\"darObjeto\",\"arg\":\"o\"}]}]")), "e", "fases[0].efectos[0]");
}

MOTOR_TEST_CASE(haber_visto_una_escena_no_puede_ser_causa) {
    const std::string obs = "\"banderasDeObservacion\":[\"bnd3_plaza_vista\"],";
    const std::string ini = "\"ausencia\":\"avanza\",\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}],";
    fallaCon(con(ev("e", ini + "\"disparadores\":[{\"senal\":\"cine_base_vista\"}]")), "e", "senal");
    fallaCon(con(ev("e", ini + "\"disparadores\":[{\"senal\":\"bnd3_plaza_vista\"}]"), obs), "e", "senal");
    fallaCon(con(ev("e", ini + "\"disparadores\":[{\"senal\":\"s\"}],\"condiciones\":{\"allFlags\":[\"cine_a_vista\"]}")),
             "e", "allFlags");
    fallaCon(con(ev("e", ini + "\"disparadores\":[{\"senal\":\"s\"}],\"condiciones\":{\"anyFlags\":[\"bnd3_plaza_vista\"]}"), obs),
             "e", "anyFlags");
    fallaCon(con(ev("e", ini + "\"disparadores\":[{\"senal\":\"s\"}],\"condiciones\":{\"notFlags\":[\"bnd3_plaza_vista\"]}"), obs),
             "e", "notFlags");
    fallaCon(con(ev("e", ini + "\"disparadores\":[{\"senal\":\"s\",\"condiciones\":{\"notFlags\":[\"cine_a_vista\"]}}]")),
             "e", "disparadores[0].condiciones.notFlags");
}

MOTOR_TEST_CASE(los_ids_repetidos_o_mal_escritos_son_un_error) {
    fallaCon(con(ev("e", kBase) + "," + ev("e", kBase)), "e", "repetido");
    fallaCon(con(ev("Mal-Id", kBase)), "Mal-Id", "id");
    fallaCon(con(ev("e", std::string(kBase) + ",\"fases\":[{\"id\":\"f\",\"disparadores\":[{\"senal\":\"s\"}]},"
                                              "{\"id\":\"f\",\"disparadores\":[{\"senal\":\"s\"}]}]")), "e", "repetida");
    fallaCon(con(ev("e", std::string(kBase) + ",\"fases\":[{\"id\":\"F 1\",\"disparadores\":[{\"senal\":\"s\"}]}]")), "e", "fases[0]");
    fallaCon(con(ev("e", "\"ausencia\":\"avanza\",\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}],"
                         "\"disparadores\":[{\"id\":\"d\",\"senal\":\"s\"},{\"id\":\"d\",\"senal\":\"t\"}]")), "e", "repetido");
}

MOTOR_TEST_CASE(evento_previo_a_algo_que_no_existe_es_un_error) {
    const std::string cola = "\"ausencia\":\"avanza\",\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}]";
    fallaCon(con(ev("e", "\"disparadores\":[{\"eventoPrevio\":\"fantasma\"}]," + cola)), "e", "fantasma");
    fallaCon(con(ev("a", kBase) + "," + ev("e", "\"disparadores\":[{\"eventoPrevio\":\"a.nofase\"}]," + cola)), "e", "nofase");
}

MOTOR_TEST_CASE(los_ciclos_de_evento_previo_son_un_error) {
    const std::string cola = "\"ausencia\":\"avanza\",\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}]";
    auto r = lee(con(ev("a", "\"disparadores\":[{\"eventoPrevio\":\"b\"}]," + cola) + "," +
                     ev("b", "\"disparadores\":[{\"eventoPrevio\":\"c\"}]," + cola) + "," +
                     ev("c", "\"disparadores\":[{\"eventoPrevio\":\"a\"}]," + cola)));
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(!r.isOk() || r.errorMessage().find("ciclo") != std::string::npos);
    fallaCon(con(ev("a", "\"disparadores\":[{\"eventoPrevio\":\"a\"}]," + cola)), "a", "sí mismo");
}

MOTOR_TEST_CASE(un_evento_sin_efectos_ni_fases_es_un_error) {
    fallaCon(con(ev("e", "\"ausencia\":\"avanza\",\"disparadores\":[{\"senal\":\"s\"}]")), "e", "efectos");
}

MOTOR_TEST_CASE(un_catalogo_que_no_es_un_catalogo_o_un_fichero_que_no_existe_dan_error) {
    auto r = lee("[1,2]");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(!RPG::eventos::cargaCatalogo("/no/existe/eventos.json").isOk());
}


// ---------------------------------------------------------------------
// C8-03b-2 — El evaluador
// ---------------------------------------------------------------------

const char* kOcaso =
    "{\"id\":\"comienza_ocaso\",\"ausencia\":\"avanza\",\"condiciones\":{\"notFlags\":[\"bnd3_campanas\"]},"
    "\"disparadores\":[{\"id\":\"aviso_venides\",\"senal\":\"bnd3_avisado\"},"
    "{\"id\":\"aviso_xila\",\"senal\":\"bnd3_xila_avisa\",\"condiciones\":{\"allFlags\":[\"bnd1_xila_honor\"]}},"
    "{\"id\":\"puesta_sol\",\"cruzaMomento\":\"ocaso_puesta\"}],"
    "\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"bnd3_campanas\"}]}";

RPG::eventos::Catalogo catalogo(const std::string& eventos) {
    auto r = lee(con(eventos));
    MOTOR_CHECK(r.isOk());
    return r.isOk() ? r.value() : RPG::eventos::Catalogo{};
}

void en(RPG::NarrativeState& s, int dia, int franja) {
    s.setVariable("dia", dia);
    s.setVariable("franja", franja);
}

MOTOR_TEST_CASE(ocaso_solo_el_reloj_salta_a_dia_4_noche_con_causa_3) {
    const auto cat = catalogo(kOcaso);
    RPG::NarrativeState s;
    en(s, 4, 1);
    MOTOR_CHECK(RPG::eventos::evalua(cat, s).empty());
    MOTOR_CHECK(!s.hasFlag("evt_comienza_ocaso"));
    en(s, 4, 2);
    const auto r = RPG::eventos::evalua(cat, s);
    MOTOR_CHECK(r.size() == 1);
    if (r.size() != 1) return;
    MOTOR_CHECK(r[0].evento == "comienza_ocaso" && r[0].fase.empty() && r[0].causa == 3 && !r[0].caducado);
    MOTOR_CHECK(s.hasFlag("evt_comienza_ocaso") && s.hasFlag("bnd3_campanas"));
    MOTOR_CHECK(s.variable("evt_comienza_ocaso__causa") == 3);
}

MOTOR_TEST_CASE(ocaso_la_senal_de_venides_salta_con_causa_1_sin_tocar_el_tiempo) {
    const auto cat = catalogo(kOcaso);
    RPG::NarrativeState s;
    en(s, 3, 0);
    s.setFlag("bnd3_avisado");
    const auto r = RPG::eventos::evalua(cat, s);
    MOTOR_CHECK(r.size() == 1 && r[0].causa == 1);
    MOTOR_CHECK(s.variable("dia") == 3 && s.variable("franja") == 0);
}

MOTOR_TEST_CASE(ocaso_la_senal_de_xila_pide_su_honor) {
    const auto cat = catalogo(kOcaso);
    RPG::NarrativeState s;
    en(s, 3, 0);
    s.setFlag("bnd3_xila_avisa");
    MOTOR_CHECK(RPG::eventos::evalua(cat, s).empty());
    MOTOR_CHECK(!s.hasFlag("evt_comienza_ocaso"));
    s.setFlag("bnd1_xila_honor");
    const auto r = RPG::eventos::evalua(cat, s);
    MOTOR_CHECK(r.size() == 1 && r[0].causa == 2);
}

MOTOR_TEST_CASE(las_tres_causas_a_la_vez_son_una_sola_confirmacion_y_no_se_repite) {
    const auto cat = catalogo(kOcaso);
    RPG::NarrativeState s;
    en(s, 4, 2);
    s.setFlag("bnd3_avisado");
    s.setFlag("bnd1_xila_honor");
    s.setFlag("bnd3_xila_avisa");
    const auto r = RPG::eventos::evalua(cat, s);
    MOTOR_CHECK(r.size() == 1 && r[0].causa == 1);
    const auto flags = s.flags();
    const auto vars = s.variables();
    for (int i = 0; i < 100; ++i) MOTOR_CHECK(RPG::eventos::evalua(cat, s).empty());
    MOTOR_CHECK(s.flags() == flags && s.variables() == vars);
}

MOTOR_TEST_CASE(si_el_tiempo_salta_el_evento_no_se_pierde) {
    const auto cat = catalogo(kOcaso);
    RPG::NarrativeState s;
    en(s, 3, 1);
    MOTOR_CHECK(RPG::eventos::evalua(cat, s).empty());
    en(s, 5, 0);
    const auto r = RPG::eventos::evalua(cat, s);
    MOTOR_CHECK(r.size() == 1 && r[0].causa == 3);
}

MOTOR_TEST_CASE(una_cadena_a_luego_b_salta_en_una_sola_llamada_aunque_b_vaya_primero) {
    // «a_b» va antes que «b_a» por id y depende de él: hace falta repetir la pasada.
    const std::string cola = "\"ausencia\":\"avanza\",";
    const auto cat = catalogo(
        ev("a_b", cola + "\"disparadores\":[{\"eventoPrevio\":\"b_a\"}],\"efectos\":[{\"type\":\"addVariable\",\"arg\":\"n\",\"value\":5,\"max\":3}]") +
        "," + ev("b_a", cola + "\"disparadores\":[{\"senal\":\"s\"}],\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}]"));
    RPG::NarrativeState s;
    s.setFlag("s");
    const auto r = RPG::eventos::evalua(cat, s);
    MOTOR_CHECK(r.size() == 2);
    if (r.size() != 2) return;
    MOTOR_CHECK(r[0].evento == "b_a" && r[1].evento == "a_b");
    MOTOR_CHECK(s.variable("n") == 3);  // addVariable respeta el tope
}

MOTOR_TEST_CASE(la_prioridad_manda_y_despues_el_id) {
    const std::string cola = "\"ausencia\":\"avanza\",\"disparadores\":[{\"senal\":\"s\"}],\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}]";
    const auto cat = catalogo(ev("a", cola) + "," + ev("b", cola + ",\"prioridad\":5") + "," + ev("c", cola + ",\"prioridad\":5"));
    RPG::NarrativeState s;
    s.setFlag("s");
    const auto r = RPG::eventos::evalua(cat, s);
    MOTOR_CHECK(r.size() == 3 && r[0].evento == "b" && r[1].evento == "c" && r[2].evento == "a");
}

MOTOR_TEST_CASE(una_fase_salta_despues_del_evento_y_una_sola_vez) {
    const auto cat = catalogo(
        ev("sermon", "\"ausencia\":\"avanza\",\"disparadores\":[{\"cruzaMomento\":{\"dia\":4,\"franja\":0}}],"
                     "\"fases\":[{\"id\":\"acabado\",\"disparadores\":[{\"cruzaMomento\":{\"dia\":4,\"franja\":1}}],"
                     "\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"sermon_acabado\"}]}]") +
        "," + ev("despues", "\"ausencia\":\"avanza\",\"disparadores\":[{\"eventoPrevio\":\"sermon.acabado\"}],"
                            "\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"y\"}]"));
    RPG::NarrativeState s;
    en(s, 4, 0);
    auto r = RPG::eventos::evalua(cat, s);
    MOTOR_CHECK(r.size() == 1 && r[0].evento == "sermon" && r[0].fase.empty());
    MOTOR_CHECK(!s.hasFlag("sermon_acabado") && !s.hasFlag("y"));
    en(s, 4, 1);
    r = RPG::eventos::evalua(cat, s);
    MOTOR_CHECK(r.size() == 2 && r[0].fase == "acabado" && r[0].causa == 1 && r[1].evento == "despues");
    MOTOR_CHECK(s.variable("evt_sermon__acabado__causa") == 1);
    MOTOR_CHECK(s.hasFlag("evt_sermon__acabado") && s.hasFlag("sermon_acabado") && s.hasFlag("y"));
    MOTOR_CHECK(RPG::eventos::evalua(cat, s).empty());
}

MOTOR_TEST_CASE(las_condiciones_comunes_del_evento_frenan_un_disparador_cumplido) {
    const auto cat = catalogo(
        ev("e", "\"ausencia\":\"avanza\",\"condiciones\":{\"allFlags\":[\"permiso\"]},"
                "\"disparadores\":[{\"senal\":\"s\"}],\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"hecho\"}]"));
    RPG::NarrativeState s;
    s.setFlag("s");
    MOTOR_CHECK(RPG::eventos::evalua(cat, s).empty());
    MOTOR_CHECK(!s.hasFlag("evt_e") && !s.hasFlag("hecho"));
    s.setFlag("permiso");
    MOTOR_CHECK(RPG::eventos::evalua(cat, s).size() == 1);
    MOTOR_CHECK(s.hasFlag("evt_e") && s.hasFlag("hecho"));
}

MOTOR_TEST_CASE(caduca_aplica_al_caducar_una_vez_y_no_salta_despues) {
    const auto cat = catalogo(
        ev("e", "\"ausencia\":\"caduca\",\"ventana\":{\"hasta\":{\"dia\":3,\"franja\":0}},"
                "\"disparadores\":[{\"senal\":\"s\"}],\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"hecho\"}],"
                "\"alCaducar\":[{\"type\":\"setFlag\",\"arg\":\"perdido\"}]"));
    RPG::NarrativeState s;
    en(s, 3, 0);
    const auto r = RPG::eventos::evalua(cat, s);
    MOTOR_CHECK(r.size() == 1 && r[0].caducado && r[0].causa == 0);
    MOTOR_CHECK(s.hasFlag("perdido") && s.hasFlag("evt_e__caducado") && !s.hasFlag("evt_e"));
    s.setFlag("s");
    MOTOR_CHECK(RPG::eventos::evalua(cat, s).empty());
    MOTOR_CHECK(!s.hasFlag("hecho"));
}

MOTOR_TEST_CASE(avanza_con_la_ventana_cerrada_ya_no_salta_y_no_caduca) {
    const auto cat = catalogo(
        ev("e", "\"ausencia\":\"avanza\",\"ventana\":{\"hasta\":{\"dia\":3,\"franja\":0}},"
                "\"disparadores\":[{\"senal\":\"s\"}],\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"hecho\"}]"));
    RPG::NarrativeState s;
    en(s, 3, 1);
    s.setFlag("s");
    MOTOR_CHECK(RPG::eventos::evalua(cat, s).empty());
    MOTOR_CHECK(!s.hasFlag("hecho") && !s.hasFlag("evt_e__caducado"));
}

MOTOR_TEST_CASE(con_desde_en_el_futuro_no_salta_y_luego_si) {
    const auto cat = catalogo(
        ev("e", "\"ausencia\":\"avanza\",\"ventana\":{\"desde\":{\"dia\":2,\"franja\":1}},"
                "\"disparadores\":[{\"senal\":\"s\"}],\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"hecho\"}]"));
    RPG::NarrativeState s;
    en(s, 2, 0);
    s.setFlag("s");
    MOTOR_CHECK(RPG::eventos::evalua(cat, s).empty());
    en(s, 2, 1);
    MOTOR_CHECK(RPG::eventos::evalua(cat, s).size() == 1);
}

MOTOR_TEST_CASE(un_guardado_copia_banderas_y_variables_y_no_salta_nada_de_nuevo) {
    const auto cat = catalogo(kOcaso);
    RPG::NarrativeState s;
    en(s, 4, 2);
    MOTOR_CHECK(RPG::eventos::evalua(cat, s).size() == 1);
    RPG::NarrativeState cargado;
    for (const auto& f : s.flags()) cargado.setFlag(f);
    for (const auto& [k, v] : s.variables()) cargado.setVariable(k, v);
    MOTOR_CHECK(RPG::eventos::evalua(cat, cargado).empty());
    MOTOR_CHECK(cargado.flags() == s.flags() && cargado.variables() == s.variables());
}

MOTOR_TEST_CASE(el_limite_de_pasadas_para_y_lo_dice) {
    // Una cadena de 70 eventos en orden inverso al de los ids: cada pasada salta uno solo.
    std::string lst;
    for (int i = 0; i < 70; ++i) {
        char id[8], previo[8];
        std::snprintf(id, sizeof id, "e%02d", 69 - i);
        std::snprintf(previo, sizeof previo, "e%02d", 70 - i);
        const std::string disp = i == 0 ? "{\"senal\":\"s\"}" : std::string("{\"eventoPrevio\":\"") + previo + "\"}";
        lst += (i ? "," : "") + ev(id, "\"ausencia\":\"avanza\",\"disparadores\":[" + disp + "],\"efectos\":[{\"type\":\"setFlag\",\"arg\":\"x\"}]");
    }
    const auto cat = catalogo(lst);
    RPG::NarrativeState s;
    s.setFlag("s");
    std::string diag;
    const auto r = RPG::eventos::evalua(cat, s, &diag);
    MOTOR_CHECK(r.size() == 64);
    MOTOR_CHECK(!diag.empty());
}

// C8-04a — El Ocaso real (assets/eventos/boundington.json): lo dice la cronología
// o el aviso de Venides, y avisar no cambia la hora.
MOTOR_TEST_CASE(el_ocaso_de_boundington_real_salta_por_la_cronologia_o_por_el_aviso) {
    const auto r = RPG::eventos::cargaCatalogo("assets/eventos/boundington.json");
    MOTOR_CHECK(r.isOk());
    if (!r.isOk()) return;
    const auto& cat = r.value();
    MOTOR_CHECK(cat.busca("comienza_ocaso") != nullptr);
    {
        RPG::NarrativeState s;
        en(s, 4, 1);
        MOTOR_CHECK(RPG::eventos::evalua(cat, s).empty());
        MOTOR_CHECK(!s.hasFlag("bnd3_campanas"));
    }
    {
        RPG::NarrativeState s;
        en(s, 4, 2);
        const auto sal = RPG::eventos::evalua(cat, s);
        MOTOR_CHECK(sal.size() == 1 + 1 && sal[0].evento == "comienza_ocaso" && sal[0].causa == 3);  // + destino de Venides (C8-04b)
        MOTOR_CHECK(s.hasFlag("bnd3_campanas"));
    }
    {
        RPG::NarrativeState s;
        en(s, 3, 0);
        s.setFlag("bnd3_avisado");
        const auto sal = RPG::eventos::evalua(cat, s);
        MOTOR_CHECK(sal.size() == 1 + 1 && sal[0].evento == "comienza_ocaso" && sal[0].causa == 1);
        MOTOR_CHECK(s.hasFlag("bnd3_campanas"));
        MOTOR_CHECK(s.variable("dia") == 3 && s.variable("franja") == 0);
    }
    {
        // Partida antigua: las campanas ya sonaron, no se tocan de nuevo.
        RPG::NarrativeState s;
        en(s, 4, 2);
        s.setFlag("bnd3_campanas");
        s.setFlag("bnd3_avisado");
        MOTOR_CHECK(RPG::eventos::evalua(cat, s).empty());
        MOTOR_CHECK(!s.hasFlag("evt_comienza_ocaso"));
    }
}

// C8-04b — El destino de Venides sale del estado del mundo, no de presenciar la arenga.
// Con las campanas, o está retenido (no se le avisó) o ha salido con su tropa; la
// compañía (bnd3_con_venides) la enciende solo la arenga, nunca el evento.
MOTOR_TEST_CASE(el_destino_de_venides_sale_del_estado_y_no_regala_la_compania) {
    const auto r = RPG::eventos::cargaCatalogo("assets/eventos/boundington.json");
    MOTOR_CHECK(r.isOk());
    if (!r.isOk()) return;
    const auto& cat = r.value();
    {
        RPG::NarrativeState s;
        en(s, 4, 2);
        s.setFlag("bnd2_venides_sin_aviso");
        RPG::eventos::evalua(cat, s);
        MOTOR_CHECK(s.hasFlag("bnd3_campanas"));
        MOTOR_CHECK(s.hasFlag("bnd3_sin_venides"));
        MOTOR_CHECK(!s.hasFlag("bnd3_venides_sale"));
        MOTOR_CHECK(!s.hasFlag("bnd3_con_venides"));
    }
    {
        RPG::NarrativeState s;
        en(s, 4, 2);
        RPG::eventos::evalua(cat, s);
        MOTOR_CHECK(s.hasFlag("bnd3_campanas"));
        MOTOR_CHECK(s.hasFlag("bnd3_venides_sale"));
        MOTOR_CHECK(!s.hasFlag("bnd3_sin_venides"));
        MOTOR_CHECK(!s.hasFlag("bnd3_con_venides"));
    }
    {
        // Avisando a Venides, a cualquier hora, ocurre igual.
        RPG::NarrativeState s;
        en(s, 3, 0);
        s.setFlag("bnd3_avisado");
        RPG::eventos::evalua(cat, s);
        MOTOR_CHECK(s.hasFlag("bnd3_venides_sale") && !s.hasFlag("bnd3_con_venides"));
    }
}

// C8-04c — Xila oye a dos Perdidos y avisa a Venides sola, la tarde del Ocaso.
// Es el tercer disparador de comienza_ocaso (causa 2, entre Venides y la puesta de sol).
MOTOR_TEST_CASE(xila_avisa_a_venides_la_tarde_del_ocaso_si_es_aliada_y_sobria) {
    const auto r = RPG::eventos::cargaCatalogo("assets/eventos/boundington.json");
    MOTOR_CHECK(r.isOk());
    if (!r.isOk()) return;
    const auto& cat = r.value();
    {
        // Por la mañana todavia no.
        RPG::NarrativeState s;
        en(s, 4, 0);
        s.setFlag("bnd1_xila_honor");
        MOTOR_CHECK(RPG::eventos::evalua(cat, s).empty());
        MOTOR_CHECK(!s.hasFlag("bnd3_xila_avisa") && !s.hasFlag("bnd3_campanas"));
    }
    {
        // La tarde, con honor: suenan las campanas sin cambiar la hora.
        RPG::NarrativeState s;
        en(s, 4, 1);
        s.setFlag("bnd1_xila_honor");
        const auto sal = RPG::eventos::evalua(cat, s);
        MOTOR_CHECK(s.hasFlag("evt_xila_avisa_a_venides") && s.hasFlag("bnd3_xila_avisa"));
        MOTOR_CHECK(s.hasFlag("bnd3_campanas") && s.variable("evt_comienza_ocaso__causa") == 2);
        MOTOR_CHECK(s.variable("dia") == 4 && s.variable("franja") == 1);
        MOTOR_CHECK(!sal.empty() && sal[0].evento == "xila_avisa_a_venides");
    }
    {
        // Venides ya salio solo hacia Las Afueras (no le avisaste el dia 2): Xila no le encuentra en la base.
        RPG::NarrativeState s;
        en(s, 4, 1);
        s.setFlag("bnd1_xila_honor");
        s.setFlag("bnd2_venides_sin_aviso");
        RPG::eventos::evalua(cat, s);
        MOTOR_CHECK(!s.hasFlag("bnd3_xila_avisa") && !s.hasFlag("bnd3_campanas"));
    }
    for (const char* mala : {"bnd1_xila_borracho", ""}) {
        // Borracha o sin honor: la tarde no salta, la puesta de sol si (causa 3).
        RPG::NarrativeState s;
        en(s, 4, 1);
        if (*mala) { s.setFlag("bnd1_xila_honor"); s.setFlag(mala); }
        RPG::eventos::evalua(cat, s);
        MOTOR_CHECK(!s.hasFlag("bnd3_xila_avisa") && !s.hasFlag("bnd3_campanas"));
        en(s, 4, 2);
        RPG::eventos::evalua(cat, s);
        MOTOR_CHECK(s.hasFlag("bnd3_campanas") && !s.hasFlag("bnd3_xila_avisa"));
        MOTOR_CHECK(s.variable("evt_comienza_ocaso__causa") == 3);
    }
    {
        // Venides ya avisado el dia 3: causa 1 y, D0025.3, Xila avisa igual la tarde del Ocaso (las campanas ya suenan).
        RPG::NarrativeState s;
        en(s, 3, 0);
        s.setFlag("bnd1_xila_honor");
        s.setFlag("bnd3_avisado");
        RPG::eventos::evalua(cat, s);
        MOTOR_CHECK(s.variable("evt_comienza_ocaso__causa") == 1);
        en(s, 4, 1);
        RPG::eventos::evalua(cat, s);
        MOTOR_CHECK(s.hasFlag("bnd3_xila_avisa") && s.hasFlag("evt_xila_avisa_a_venides"));
    }
    {
        // ... pero no si Venides salio solo (no esta en la base), aunque se haya corrido a avisar.
        RPG::NarrativeState s;
        en(s, 3, 0);
        s.setFlag("bnd1_xila_honor");
        s.setFlag("bnd3_avisado");
        s.setFlag("bnd2_venides_sin_aviso");
        RPG::eventos::evalua(cat, s);
        en(s, 4, 1);
        RPG::eventos::evalua(cat, s);
        MOTOR_CHECK(!s.hasFlag("bnd3_xila_avisa"));
    }
}

}  // namespace

int main() { return motor_test::runAllTests(); }
