// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-07  (fase G, 2 j) - Congelacion del esquema
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "RPG/AdventureSerializer.h"

#include "Core/Json/JsonWriter.h"
#include "Narrative/AcompanantesAventura.h"  // ACOMPANANTES-01
#include "Narrative/BeatsAventura.h"         // VARIABLES-01 / OPCIONES-01

namespace RPG {
namespace {

// Revesa de parseTriggerType (NarrativeEngine.cpp): TriggerType -> text.
const char* triggerTypeToText(TriggerType type) {
    switch (type) {
        case TriggerType::Talk:  return "talk";
        case TriggerType::Enter: return "enter";
        case TriggerType::Auto:  return "auto";
    }
    return "auto";  // mai passa amb un valor vàlid, però evita warnings
}

// Revesa de la lectura de "presentacion" (TimerDefinition::Presentacion).
const char* presentacionToText(TimerDefinition::Presentacion p) {
    switch (p) {
        case TimerDefinition::Presentacion::Contador: return "contador";
        case TimerDefinition::Presentacion::Barra:    return "barra";
        case TimerDefinition::Presentacion::Secreto:  return "secreto";
    }
    return "contador";
}

// Escriu un vector<string> com un array JSON. No salta elements buits:
// el parser (readStringArray) els filtra, aixÃ­ que escriure'l-los només
// trencaria la idempotència.
void writeStringArray(JsonWriter& w, const char* key,
                      const std::vector<std::string>& values) {
    w.key(key);
    w.beginArray();
    for (const std::string& v : values) {
        w.value(v);
    }
    w.endArray();
}

}  // namespace

std::string serializeAdventure(const AdventureScript& adv) {
    JsonWriter w;
    w.beginObject();

    w.member("id", adv.id);
    w.member("name", adv.name);
    if (!adv.description.empty()) {
        w.member("description", adv.description);
    }

    // --- Objectius -------------------------------------------------
    w.key("objectives");
    w.beginArray();
    for (const ObjectiveDefinition& obj : adv.objectives) {
        w.beginObject();
        w.member("id", obj.id);
        w.member("text", obj.text);
        w.member("doneFlag", obj.doneFlag);
        if (obj.donde.size() == 1) w.member("donde", obj.donde.front());  // MAPA-01
        if (obj.donde.size() > 1) {
            w.key("donde");
            w.beginArray();
            for (const std::string& d : obj.donde) w.value(d);
            w.endArray();
        }
        w.endObject();
    }
    w.endArray();

    // --- Beats -----------------------------------------------------
    w.key("beats");
    w.beginArray();
    for (const NarrativeBeat& beat : adv.beats) {
        w.beginObject();
        w.member("id", beat.id);

        // trigger: sempre present (el parser requereix "type")
        w.key("trigger");
        w.beginObject();
        w.member("type", triggerTypeToText(beat.trigger.type));
        w.member("target", beat.trigger.target);
        w.endObject();

        beats::escribeCondicion(w, beat.requirements);  // solo si tiene algo

        w.member("speaker", beat.speaker);
        if (!beat.speakerId.empty()) w.member("speakerId", beat.speakerId);  // D0037
        writeStringArray(w, "lines", beat.lines);
        w.member("once", beat.once);
        if (beat.pagina) w.member("pagina", true);  // DIALOGOS-01

        beats::escribeEfectos(w, beat.effects);  // siempre (array, aunque este vacio)
        beats::escribeOpciones(w, beat.opciones);  // OPCIONES-01: solo si hay

        w.endObject();
    }
    w.endArray();

    // --- Temporadors -----------------------------------------------
    w.key("timers");
    w.beginArray();
    for (const TimerDefinition& t : adv.timers) {
        w.beginObject();
        w.member("id", t.id);
        // Solo si lo tiene: las aventuras sin nombre se escriben como antes.
        if (!t.nombre.empty()) {
            w.member("nombre", t.nombre);
        }
        w.member("duracion", t.duracion);
        w.member("arrancaPor", t.arrancaPor);

        // expiraPor: cal escriure'l sempre. El parser tracta [] com a
        // "usa defaults" (expiraPerTempo=true), així que si el struct té
        // false a false, caldrí fer-ho coincidir escapant. Però en un
        // timer vàlid com la mínima expressió és true (per temps o zona).
        w.key("expiraPor");
        w.beginArray();
        if (t.expiraPorTiempo) {
            w.value("tiempo");
        }
        if (t.expiraPorSalirDeZona) {
            w.value("salir_de_zona");
        }
        w.endArray();

        if (!t.zona.empty()) {
            w.member("zona", t.zona);
        }
        w.member("flagExpira", t.flagExpira);

        w.key("avisos");
        w.beginArray();
        for (const TimerDefinition::Aviso& aviso : t.avisos) {
            w.beginObject();
            w.member("fraccion", aviso.fraccion);
            w.member("flag", aviso.flag);
            w.endObject();
        }
        w.endArray();

        w.member("presentacion", presentacionToText(t.presentacion));
        w.endObject();
    }
    w.endArray();

    // ACOMPANANTES-01 — Solo si hay alguno: las demas salen como antes.
    acompanantes::escribe(w, adv.acompanantes);

    w.endObject();
    return w.str();
}

}  // namespace RPG
