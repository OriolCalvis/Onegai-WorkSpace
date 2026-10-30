#include "RPG/AdventureValidation.h"

#include "TestHarness.h"

#include <string>
#include <variant>
#include <vector>

// E-07 — Validacion estatica de eventos.
//
// Cada caso defiende un fallo que HOY no da error en ninguna parte y que
// el autor solo descubriria jugando: una bandera que nadie enciende, un
// comando detras de un cambio de mapa, un interruptor de un evento que no
// existe, un comando que esta version no conoce.

namespace {

using namespace RPG;

ids::EventId evt(const std::string& v) { return ids::EventId::parse(v).value(); }
ids::LayerId lyr(const std::string& v) { return ids::LayerId::parse(v).value(); }

MapEventCommand cmd(const std::string& tipo,
                    std::map<std::string, std::string> params = {}) {
    MapEventCommand c;
    c.type = tipo;
    c.params = std::move(params);
    return c;
}

MapEventDocument evento(const std::string& id, std::vector<MapEventCommand> comandos, int x = 0) {
    MapEventDocument e;
    e.id = evt(id);
    e.name = id;
    e.position = GridCoord{x, 0};
    MapEventPage p;
    p.id = evt(id + "_p1");
    p.commands = std::move(comandos);
    e.pages.push_back(std::move(p));
    return e;
}

MapDocument mapaCon(std::vector<MapEventDocument> eventos) {
    MapDocument doc(ids::MapId::parse("mapa").value(), "mapa", 16, 16, 64, 32);
    MapLayerMeta meta;
    meta.id = lyr("eventos");
    meta.name = "eventos";
    doc.addLayer(MapLayer::events(meta, EventLayerDocument()));
    for (const MapEventDocument& e : eventos) {
        doc.addEvent(lyr("eventos"), e);
    }
    return doc;
}

// Cuantos diagnosticos contienen ese texto.
std::size_t conteo(const ValidationReport& r, const std::string& trozo) {
    std::size_t n = 0;
    for (const Diagnostic& d : r.diagnostics) {
        if (d.message.find(trozo) != std::string::npos) ++n;
    }
    return n;
}

MOTOR_TEST_CASE(una_mision_sana_no_produce_ni_un_diagnostico) {
    // Si el validador chilla con una mision correcta, el autor aprende a
    // ignorarlo y deja de servir para nada.
    const MapDocument doc = mapaCon({
        evento("cofre", {cmd("bandera.encender", {{"bandera", "tiene_llave"}}),
                         cmd("texto.mostrar", {{"texto", "Una llave."}})}, 1),
        evento("puerta", {cmd("control.si", {{"clase", "flag"}, {"referencia", "tiene_llave"}})}, 2),
    });
    const ValidationReport r = ValidateEvents(doc);
    MOTOR_CHECK(r.diagnostics.empty());
    MOTOR_CHECK(r.canPlaytest());
}

MOTOR_TEST_CASE(una_bandera_que_nadie_enciende_es_aviso_y_no_error) {
    // La puerta no se abriria nunca y no habria error en ninguna parte.
    // Es AVISO porque puede encenderla un beat antiguo, que este
    // validador no mira: convertirlo en error bloquearia proyectos sanos.
    const MapDocument doc = mapaCon({
        evento("puerta", {cmd("control.si", {{"clase", "flag"}, {"referencia", "tiene_llave"}})}),
    });
    const ValidationReport r = ValidateEvents(doc);
    MOTOR_CHECK(conteo(r, "tiene_llave") == 1);
    MOTOR_CHECK(r.errorCount() == 0);
    MOTOR_CHECK(r.warningCount() == 1);
    MOTOR_CHECK(r.canPlaytest());   // se puede probar, con el aviso a la vista
}

MOTOR_TEST_CASE(lo_que_va_detras_de_un_cambio_de_mapa_se_marca_inalcanzable) {
    // DEC-30. El plan lo pide con estas palabras: «el que hi hagi a sota
    // rep diagnostic d'inabastable».
    const MapDocument doc = mapaCon({
        evento("salida", {cmd("mapa.ir", {{"mapa", "bosque"}}),
                          cmd("texto.mostrar", {{"texto", "nunca"}}),
                          cmd("texto.mostrar", {{"texto", "tampoco"}})}),
    });
    const ValidationReport r = ValidateEvents(doc);
    // UNA vez, no una por comando: cincuenta avisos identicos no ayudan.
    MOTOR_CHECK(conteo(r, "no se ejecutará nunca") == 1);

    // Y el diagnostico sabe DONDE esta, que es lo que «Ir ahi» necesita.
    for (const Diagnostic& d : r.diagnostics) {
        if (d.message.find("no se ejecutará nunca") != std::string::npos) {
            MOTOR_CHECK(d.eventId == "salida");
            MOTOR_CHECK(d.hasPage);
            MOTOR_CHECK(d.pageIndex == 0);
            MOTOR_CHECK(d.commandPath.size() == 1);
            MOTOR_CHECK(d.commandPath[0] == 1);   // el segundo comando
        }
    }
}

MOTOR_TEST_CASE(un_terminal_dentro_de_una_rama_no_corta_lo_de_fuera) {
    // Un `si` que en una rama lleva a otro mapa NO hace inalcanzable lo
    // que viene despues del `si`: solo lo que viene despues dentro de esa
    // rama. Confundirlo llenaria de avisos falsos cualquier mision con
    // una salida opcional.
    MapEventCommand si = cmd("control.si", {{"clase", "flag"}, {"referencia", "huye"}});
    MapEventCommand rama;
    rama.type = "rama.si";
    rama.children.push_back(cmd("mapa.ir", {{"mapa", "bosque"}}));
    rama.children.push_back(cmd("texto.mostrar", {{"texto", "nunca"}}));
    si.children.push_back(rama);

    MapEventDocument e = evento("guardia", {si, cmd("texto.mostrar", {{"texto", "sí se ejecuta"}})});
    e.pages[0].commands[0] = si;
    MapDocument doc = mapaCon({e});
    // La bandera la enciende nadie: se ignora ese aviso y se mira el otro.
    const ValidationReport r = ValidateEvents(doc);
    MOTOR_CHECK(conteo(r, "no se ejecutará nunca") == 1);
    for (const Diagnostic& d : r.diagnostics) {
        if (d.message.find("no se ejecutará nunca") != std::string::npos) {
            // Dentro de la rama: ruta {0 (el si), 0 (la rama), 1}
            MOTOR_CHECK(d.commandPath.size() == 3);
        }
    }
}

MOTOR_TEST_CASE(un_interruptor_de_un_evento_que_no_existe_es_error) {
    // Nadie lo leeria nunca, y el autor no tendria forma de saberlo.
    const MapDocument doc = mapaCon({
        evento("cofre", {cmd("interruptor.fijar",
                             {{"evento", "cofre_fantasma"}, {"letra", "A"}, {"valor", "true"}})}),
    });
    const ValidationReport r = ValidateEvents(doc);
    MOTOR_CHECK(conteo(r, "cofre_fantasma") == 1);
    MOTOR_CHECK(r.errorCount() == 1);
    MOTOR_CHECK(!r.canPlaytest());

    // Apuntando a si mismo, sin problema.
    const MapDocument bueno = mapaCon({
        evento("cofre", {cmd("interruptor.fijar",
                             {{"evento", "cofre"}, {"letra", "A"}, {"valor", "true"}})}),
    });
    MOTOR_CHECK(ValidateEvents(bueno).diagnostics.empty());
}

MOTOR_TEST_CASE(un_comando_que_esta_version_no_conoce_impide_probar) {
    // Sin esto, el autor abre un proyecto hecho con una version mas nueva,
    // monta la mision y descubre al jugarla que media no hace nada. (Hasta
    // el cierre de E-04 este caso usaba comercio.abrir, declarado y sin
    // cablear; hoy ningun comando del catalogo esta en ese estado.)
    const MapDocument doc = mapaCon({
        evento("tendero", {cmd("comercio.regatear", {{"tienda", "general"}})}),
    });
    const ValidationReport r = ValidateEvents(doc);
    MOTOR_CHECK(conteo(r, "no disponible") == 1);
    MOTOR_CHECK(!r.canPlaytest());

    // Y comercio.abrir, ya cableado, se puede probar.
    const MapDocument bien = mapaCon({
        evento("tendero", {cmd("comercio.abrir", {{"tienda", "general"}})}),
    });
    const ValidationReport ok = ValidateEvents(bien);
    MOTOR_CHECK(ok.errorCount() == 0);
    MOTOR_CHECK(ok.canPlaytest());
}

MOTOR_TEST_CASE(un_parametro_que_falta_se_reporta_con_su_sitio_exacto) {
    const MapDocument doc = mapaCon({
        evento("cofre", {cmd("texto.mostrar", {{"texto", "hola"}}),
                         cmd("bandera.encender")}),   // le falta 'bandera'
    });
    const ValidationReport r = ValidateEvents(doc);
    MOTOR_CHECK(r.errorCount() == 1);
    const Diagnostic& d = r.diagnostics[0];
    MOTOR_CHECK(d.message.find("bandera") != std::string::npos);
    MOTOR_CHECK(d.eventId == "cofre");
    MOTOR_CHECK(d.commandPath.size() == 1);
    MOTOR_CHECK(d.commandPath[0] == 1);
}

MOTOR_TEST_CASE(un_evento_o_una_pagina_vacios_avisan_sin_bloquear) {
    // El diseno de bloques permite guardar borradores incompletos, asi
    // que esto no puede ser un error. Pero el autor querra saberlo antes
    // de preguntarse por que su cofre no responde.
    MapEventDocument vacio;
    vacio.id = evt("cofre_vacio");
    vacio.position = GridCoord{5, 5};

    MapDocument doc = mapaCon({evento("normal", {cmd("texto.mostrar", {{"texto", "x"}})}, 1)});
    doc.addEvent(lyr("eventos"), vacio);

    const ValidationReport r = ValidateEvents(doc);
    MOTOR_CHECK(conteo(r, "no tiene ninguna página") == 1);
    MOTOR_CHECK(r.errorCount() == 0);
    MOTOR_CHECK(r.canPlaytest());
}

MOTOR_TEST_CASE(un_mapa_sin_capa_de_eventos_no_da_ningun_diagnostico) {
    MapDocument doc(ids::MapId::parse("mapa").value(), "mapa", 8, 8, 64, 32);
    MapLayerMeta meta;
    meta.id = lyr("suelo");
    meta.name = "suelo";
    doc.addLayer(MapLayer::tiles(meta, TileLayerDocument(8, 8, std::vector<int>(64, 0))));
    MOTOR_CHECK(ValidateEvents(doc).diagnostics.empty());
}

// =====================================================================
// E-07 (segunda mitad) — Aventuras de beats
// =====================================================================

NarrativeBeat beat(const std::string& id) {
    NarrativeBeat b;
    b.id = id;
    b.lines.push_back("...");
    return b;
}

NarrativeEffect efecto(EffectType tipo, const std::string& arg = {}) {
    NarrativeEffect e;
    e.type = tipo;
    e.arg = arg;
    return e;
}

MOTOR_TEST_CASE(una_aventura_sana_no_produce_ni_un_diagnostico) {
    AdventureScript a;
    a.id = "prologo";
    NarrativeBeat b1 = beat("b1");
    b1.effects.push_back(efecto(EffectType::SetFlag, "hablado"));
    NarrativeBeat b2 = beat("b2");
    b2.requirements.allFlags.push_back("hablado");
    b2.effects.push_back(efecto(EffectType::SetFlag, "prologo_hecho"));
    a.beats = {b1, b2};

    ObjectiveDefinition o;
    o.id = "obj1";
    o.text = "Habla con el posadero";
    o.doneFlag = "prologo_hecho";
    a.objectives.push_back(o);

    const ValidationReport r = ValidateAdventure(a);
    MOTOR_CHECK(r.diagnostics.empty());
}

MOTOR_TEST_CASE(un_beat_que_pide_una_bandera_que_nadie_enciende_no_se_dispara_nunca) {
    AdventureScript a;
    NarrativeBeat b = beat("puerta");
    b.requirements.allFlags.push_back("tiene_llave");
    a.beats = {b};

    const ValidationReport r = ValidateAdventure(a);
    MOTOR_CHECK(conteo(r, "tiene_llave") == 1);
    MOTOR_CHECK(r.errorCount() == 0);   // aviso: puede encenderla un evento del mapa
    for (const Diagnostic& d : r.diagnostics) {
        if (d.message.find("tiene_llave") != std::string::npos) {
            MOTOR_CHECK(d.eventId == "puerta");
            MOTOR_CHECK(!d.hasPage);    // un beat no tiene paginas
        }
    }
}

MOTOR_TEST_CASE(dos_beats_con_el_mismo_id_son_error) {
    // `once` marcaria uno y callaria el otro para siempre.
    AdventureScript a;
    NarrativeBeat uno = beat("igual");
    uno.effects.push_back(efecto(EffectType::Log, "a"));
    NarrativeBeat dos = beat("igual");
    dos.effects.push_back(efecto(EffectType::Log, "b"));
    a.beats = {uno, dos};

    const ValidationReport r = ValidateAdventure(a);
    MOTOR_CHECK(conteo(r, "dos beats con el id") == 1);
    MOTOR_CHECK(r.errorCount() == 1);
}

MOTOR_TEST_CASE(un_temporizador_que_nadie_arranca_avisa_y_uno_sin_declarar_es_error) {
    // El plan lo pide con estas palabras: «temporitzadors que no arrenca
    // ningu».
    AdventureScript a;
    TimerDefinition t;
    t.id = "huida";
    t.duracion = 60.0f;
    t.flagExpira = "se_escapo";
    a.timers.push_back(t);
    NarrativeBeat b = beat("b1");
    b.effects.push_back(efecto(EffectType::Log, "x"));
    a.beats = {b};

    const ValidationReport r = ValidateAdventure(a);
    MOTOR_CHECK(conteo(r, "nadie arranca el temporizador 'huida'") == 1);
    MOTOR_CHECK(r.errorCount() == 0);

    // Arrancar uno que no esta declarado SI es error: el efecto no hace
    // nada y nadie avisa.
    AdventureScript b2;
    NarrativeBeat arranca = beat("b1");
    arranca.effects.push_back(efecto(EffectType::StartTimer, "fantasma"));
    b2.beats = {arranca};
    const ValidationReport r2 = ValidateAdventure(b2);
    MOTOR_CHECK(conteo(r2, "fantasma") == 1);
    MOTOR_CHECK(r2.errorCount() == 1);
}

MOTOR_TEST_CASE(un_temporizador_que_expira_por_tiempo_y_dura_cero_es_error) {
    AdventureScript a;
    TimerDefinition t;
    t.id = "roto";
    t.duracion = 0.0f;
    t.arrancaPor = "empieza";
    a.timers.push_back(t);
    NarrativeBeat b = beat("b1");
    b.effects.push_back(efecto(EffectType::SetFlag, "empieza"));
    a.beats = {b};

    const ValidationReport r = ValidateAdventure(a);
    MOTOR_CHECK(conteo(r, "dura cero") == 1);
    MOTOR_CHECK(!r.canPlaytest());
}

MOTOR_TEST_CASE(un_objetivo_que_nadie_completa_se_queda_pendiente_para_siempre) {
    AdventureScript a;
    NarrativeBeat b = beat("b1");
    b.effects.push_back(efecto(EffectType::Log, "x"));
    a.beats = {b};
    ObjectiveDefinition o;
    o.id = "obj1";
    o.doneFlag = "nunca_encendida";
    a.objectives.push_back(o);

    const ValidationReport r = ValidateAdventure(a);
    MOTOR_CHECK(conteo(r, "no se completara nunca") == 1);

    // Y sin bandera de completado, error: no hay forma de cerrarlo.
    AdventureScript b2;
    ObjectiveDefinition sin;
    sin.id = "obj2";
    b2.objectives.push_back(sin);
    MOTOR_CHECK(ValidateAdventure(b2).errorCount() >= 1);
}

MOTOR_TEST_CASE(las_banderas_de_un_skillcheck_y_de_un_combate_cuentan_como_encendidas) {
    // Sin esto, cualquier aventura que ramifique por tirada se llenaria
    // de avisos falsos: las cuatro banderas del grado no las enciende un
    // setFlag, las enciende el propio efecto.
    AdventureScript a;
    NarrativeBeat tirada = beat("percepcion");
    NarrativeEffect check;
    check.type = EffectType::SkillCheck;
    check.skillCheck.skillId = "percepcion";
    check.skillCheck.flagSuccess = "vio_algo";
    tirada.effects.push_back(check);

    NarrativeBeat despues = beat("reaccion");
    despues.requirements.allFlags.push_back("vio_algo");
    despues.effects.push_back(efecto(EffectType::Log, "!"));
    a.beats = {tirada, despues};

    MOTOR_CHECK(ValidateAdventure(a).diagnostics.empty());

    // Y un combate sin enemigo no se puede resolver.
    AdventureScript b2;
    NarrativeBeat pelea = beat("pelea");
    NarrativeEffect batalla;
    batalla.type = EffectType::StartBattle;
    pelea.effects.push_back(batalla);
    b2.beats = {pelea};
    MOTOR_CHECK(conteo(ValidateAdventure(b2), "combate sin enemigo") == 1);
}

// La prueba del plan (ficha E-07): una aventura con SEIS defectes
// coneguts dona sis diagnostics amb la seva ubicacio. Cada defecte planta
// una sola cosa mala y se comprueba que el diagnostico llega CON su sitio
// (eventId del beat, del objetivo o del temporizador).
MOTOR_TEST_CASE(sis_defectes_coneguts_donen_sis_diagnostics_amb_ubicacio) {
    AdventureScript a;
    a.id = "rota";

    NarrativeBeat sano = beat("sano");
    sano.effects.push_back(efecto(EffectType::SetFlag, "encendida"));

    // 1. Dos beats con el mismo id.
    NarrativeBeat duplicado = beat("sano");

    // 2. Combate sin enemigo.
    NarrativeBeat combate = beat("combate_roto");
    NarrativeEffect batalla;
    batalla.type = EffectType::StartBattle;  // monsterId vacio
    combate.effects.push_back(batalla);

    // 3. Tirada sin habilidad.
    NarrativeBeat tirada = beat("tirada_rota");
    NarrativeEffect check;
    check.type = EffectType::SkillCheck;  // skillId vacio
    tirada.effects.push_back(check);

    // 4. Temporizador arrancado y no declarado.
    NarrativeBeat arranque = beat("arranque_fantasma");
    arranque.effects.push_back(efecto(EffectType::StartTimer, "fuego_nunca_declarado"));

    a.beats = {sano, duplicado, combate, tirada, arranque};

    // 5. Objetivo sin bandera de completado.
    ObjectiveDefinition obj;
    obj.id = "obj_sin_flag";
    obj.text = "Nunca completable (mal declarado)";
    a.objectives.push_back(obj);

    // 6. Temporizador que expira por tiempo y dura cero.
    TimerDefinition t;
    t.id = "timer_cero";
    t.duracion = 0.0f;
    t.expiraPorTiempo = true;
    a.timers.push_back(t);

    const ValidationReport r = ValidateAdventure(a);
    MOTOR_CHECK(r.errorCount() >= 6);
    // Cada defecto nombra su sitio: el beat, el objetivo o el timer.
    MOTOR_CHECK(conteo(r, "dos beats con el id 'sano'") == 1);
    MOTOR_CHECK(conteo(r, "combate sin enemigo") == 1);
    MOTOR_CHECK(conteo(r, "tirada sin habilidad") == 1);
    MOTOR_CHECK(conteo(r, "fuego_nunca_declarado") == 1);
    MOTOR_CHECK(conteo(r, "no tiene bandera de completado") == 1);
    MOTOR_CHECK(conteo(r, "'timer_cero' expira por tiempo y dura cero") == 1);
    // Y la ubicacion viaja: el combate roto se localiza por su beat.
    for (const Diagnostic& d : r.diagnostics) {
        if (d.message.find("combate sin enemigo") != std::string::npos) {
            MOTOR_CHECK(d.eventId == "combate_roto");
        }
    }
}

// E-07 (cierre) — Referencias externas: lo que un beat nombra tiene que
// existir en los catalogos del proyecto.
MOTOR_TEST_CASE(les_referencies_externes_es_comproven_amb_els_catalogs) {
    RPG::Catalogs::SkillCatalog skills;
    RPG::SkillDefinition sigilo;
    sigilo.id = "sigilo";
    skills.insert(sigilo);

    RPG::Catalogs::MonsterCatalog monsters;
    RPG::MonsterDefinition bandido;
    bandido.id = "gorran_bandido";
    monsters.insert(bandido);

    NarrativeBeat roto = beat("roto");
    NarrativeEffect check;
    check.type = EffectType::SkillCheck;
    check.skillCheck.skillId = "habitat_inventado";
    roto.effects.push_back(check);
    NarrativeEffect batalla;
    batalla.type = EffectType::StartBattle;
    batalla.battle.monsterId = "monstruo_inventado";
    roto.effects.push_back(batalla);

    NarrativeBeat bueno = beat("bueno");
    NarrativeEffect checkOk = check;
    checkOk.skillCheck.skillId = "sigilo";
    bueno.effects.push_back(checkOk);
    NarrativeEffect batallaOk = batalla;
    batallaOk.battle.monsterId = "gorran_bandido";
    bueno.effects.push_back(batallaOk);

    AdventureScript a;
    a.id = "refs";
    a.beats = {roto, bueno};

    AdventureCatalogRefs refs;
    refs.skills = &skills;
    refs.monsters = &monsters;
    const ValidationReport r = ValidateAdventure(a, refs);
    MOTOR_CHECK(conteo(r, "habitat_inventado") == 1);
    MOTOR_CHECK(conteo(r, "monstruo_inventado") == 1);
    // La ubicacion: ambos del mismo beat.
    for (const Diagnostic& d : r.diagnostics) {
        if (!d.message.empty() && d.message.find("inventado") != std::string::npos) {
            MOTOR_CHECK(d.eventId == "roto");
        }
    }
    // Los ids que SI estan en el catalogo no producen nada.
    MOTOR_CHECK(conteo(r, "sigilo") == 0);
    MOTOR_CHECK(conteo(r, "gorran_bandido") == 0);
}

// Sin catalogos no se miran las referencias externas: el overload base
// mantiene el comportamiento de siempre (documentado en el header).
MOTOR_TEST_CASE(sense_catalogs_no_es_miren_les_referencies_externes) {
    NarrativeBeat roto = beat("roto");
    NarrativeEffect check;
    check.type = EffectType::SkillCheck;
    check.skillCheck.skillId = "habitat_inventado";
    roto.effects.push_back(check);

    AdventureScript a;
    a.id = "refs";
    a.beats = {roto};

    const ValidationReport r = ValidateAdventure(a);
    MOTOR_CHECK(conteo(r, "habitat_inventado") == 0);
    MOTOR_CHECK(conteo(r, "catalogo") == 0);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
