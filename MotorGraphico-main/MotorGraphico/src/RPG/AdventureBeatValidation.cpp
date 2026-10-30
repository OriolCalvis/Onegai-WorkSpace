// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-07  (fase E, 4 j) - Validació estàtica d'aventures
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "RPG/AdventureValidation.h"

#include <algorithm>
#include <set>
#include <string>
#include <vector>

// E-07 — Validació de beats d'aventura. Extret d'AdventureValidation.cpp
// (EST-33) per reduir-lo per sota de 500 línies. Consulta la nota llarga
// del header per al контекст complet.

namespace RPG {

namespace {

Diagnostic HallazgoBeat(DiagnosticSeverity severity, std::string message,
                        std::string beatId = {}) {
    Diagnostic d;
    d.severity = severity;
    d.message = std::move(message);
    d.eventId = std::move(beatId);
    d.hasPage = false;   // un beat no tiene paginas
    return d;
}

// Las banderas que unos efectos ENCIENDEN o APAGAN (y, VARIABLES-01, las
// variables que escriben). Es lo que permite decir despues «esta se pide y
// no la enciende nadie».
void BanderasQueEscribe(const std::vector<NarrativeEffect>& efectos, std::set<std::string>& salida,
                        std::set<std::string>& variables) {
    for (const NarrativeEffect& efecto : efectos) {
        switch (efecto.type) {
            case EffectType::SetFlag:
            case EffectType::ClearFlag:
                if (!efecto.arg.empty()) salida.insert(efecto.arg);
                break;
            case EffectType::SkillCheck:
                // Un skillCheck enciende UNA de las cuatro segun el grado.
                for (const std::string& f : {efecto.skillCheck.flagBotch,
                                             efecto.skillCheck.flagPartial,
                                             efecto.skillCheck.flagSuccess,
                                             efecto.skillCheck.flagCritical}) {
                    if (!f.empty()) salida.insert(f);
                }
                break;
            case EffectType::StartBattle:
                if (!efecto.battle.flagVictory.empty()) salida.insert(efecto.battle.flagVictory);
                if (!efecto.battle.flagDefeat.empty()) salida.insert(efecto.battle.flagDefeat);
                if (!efecto.battle.flagHuida.empty()) salida.insert(efecto.battle.flagHuida);  // CAER-01
                break;
            case EffectType::SetVariable:
            case EffectType::AddVariable:
                if (!efecto.arg.empty()) variables.insert(efecto.arg);
                break;
            case EffectType::PasarFranja:  // FASE1-BND-01: el dia y la franja
                variables.insert("dia");
                variables.insert("franja");
                break;
            case EffectType::TablaAleatoria:  // una de sus flags, la de la cara que sale
                for (const std::string& f : efecto.tabla.flags)
                    if (!f.empty()) salida.insert(f);
                break;
            case EffectType::GrantGold:
            case EffectType::Log:
            case EffectType::StartTimer:
            case EffectType::GrantMilestone:
            case EffectType::GrantLevel:
            case EffectType::Descanso:
            case EffectType::DarObjeto:
            case EffectType::Acto:    // MORAL-01: la moral, no flags
            case EffectType::Herida:
                break;
        }
    }
}

// Las de un beat: las suyas y las de sus opciones (OPCIONES-01).
void BanderasQueEscribe(const NarrativeBeat& beat, std::set<std::string>& salida, std::set<std::string>& variables) {
    BanderasQueEscribe(beat.effects, salida, variables);
    for (const OpcionDeDialogo& opcion : beat.opciones) BanderasQueEscribe(opcion.efectos, salida, variables);
}

// OPCIONES-01 — Los efectos de un beat y los de sus opciones, juntos (copia:
// los beats son pocos y esto solo corre al validar).
std::vector<NarrativeEffect> TodosLosEfectos(const NarrativeBeat& beat) {
    std::vector<NarrativeEffect> todos = beat.effects;
    for (const OpcionDeDialogo& opcion : beat.opciones)
        todos.insert(todos.end(), opcion.efectos.begin(), opcion.efectos.end());
    return todos;
}

void BanderasQueLee(const NarrativeCondition& cond, std::set<std::string>& salida) {
    for (const auto* lista : {&cond.allFlags, &cond.anyFlags, &cond.notFlags}) {
        for (const std::string& f : *lista) {
            if (!f.empty()) salida.insert(f);
        }
    }
}

}  // namespace

ValidationReport ValidateAdventure(const AdventureScript& adventure) {
    ValidationReport informe;

    // --- Primera pasada: que enciende quien ---
    std::set<std::string> escritas;
    std::set<std::string> variablesEscritas;  // VARIABLES-01
    std::set<std::string> temporizadoresArrancados;
    for (const NarrativeBeat& beat : adventure.beats) {
        BanderasQueEscribe(beat, escritas, variablesEscritas);
        for (const NarrativeEffect& efecto : TodosLosEfectos(beat)) {
            if (efecto.type == EffectType::StartTimer && !efecto.arg.empty()) {
                temporizadoresArrancados.insert(efecto.arg);
            }
        }
    }
    // Un temporizador tambien puede arrancar por bandera, sin efecto.
    for (const TimerDefinition& timer : adventure.timers) {
        if (!timer.arrancaPor.empty()) {
            temporizadoresArrancados.insert(timer.id);
        }
        // Y sus avisos y su expiracion encienden banderas.
        if (!timer.flagExpira.empty()) escritas.insert(timer.flagExpira);
        for (const TimerDefinition::Aviso& aviso : timer.avisos) {
            if (!aviso.flag.empty()) escritas.insert(aviso.flag);
        }
    }

    // --- Segunda pasada: que esta mal ---
    std::set<std::string> idsVistos;
    for (const NarrativeBeat& beat : adventure.beats) {
        if (beat.id.empty()) {
            informe.diagnostics.push_back(HallazgoBeat(
                DiagnosticSeverity::Error, "hay un beat sin id: no se puede marcar como 'once' "
                                           "ni nombrarlo en un diagnostico"));
        } else if (!idsVistos.insert(beat.id).second) {
            // Dos beats con el mismo id: `once` marca uno y calla el otro.
            informe.diagnostics.push_back(HallazgoBeat(
                DiagnosticSeverity::Error,
                "hay dos beats con el id '" + beat.id + "'", beat.id));
        }

        // Banderas que el beat pide y que nadie enciende: no se dispara
        // nunca. Aviso y no error, por el mismo motivo que en los
        // eventos: puede encenderla el juego o un evento del mapa, que
        // esta funcion no mira.
        std::set<std::string> leidas;
        BanderasQueLee(beat.requirements, leidas);
        for (const std::string& bandera : leidas) {
            if (escritas.count(bandera) == 0) {
                informe.diagnostics.push_back(HallazgoBeat(
                    DiagnosticSeverity::Warning,
                    "ningun beat enciende la bandera '" + bandera +
                        "': este beat no se disparara nunca",
                    beat.id));
            }
        }
        // VARIABLES-01 — Una variable que se pide y no escribe nadie vale 0
        // siempre. OPCIONES-01 — Y lo mismo con lo que piden las opciones.
        std::vector<const NarrativeCondition*> condiciones{&beat.requirements};
        for (const OpcionDeDialogo& opcion : beat.opciones) condiciones.push_back(&opcion.requisitos);
        for (const NarrativeCondition* condicion : condiciones) {
            for (const NarrativeCondition::RangoDeVariable& rango : condicion->variables) {
                if (variablesEscritas.count(rango.nombre) == 0)
                    informe.diagnostics.push_back(HallazgoBeat(
                        DiagnosticSeverity::Warning,
                        "ningun beat escribe la variable '" + rango.nombre + "': siempre vale 0", beat.id));
            }
        }
        for (const OpcionDeDialogo& opcion : beat.opciones) {
            std::set<std::string> deLaOpcion;
            BanderasQueLee(opcion.requisitos, deLaOpcion);
            for (const std::string& bandera : deLaOpcion) {
                if (escritas.count(bandera) == 0)
                    informe.diagnostics.push_back(HallazgoBeat(
                        DiagnosticSeverity::Warning,
                        "ningun beat enciende la bandera '" + bandera + "': la opcion '" + opcion.texto +
                            "' no saldra nunca",
                        beat.id));
            }
        }

        // Un beat sin lineas y sin efectos se dispara y no hace nada.
        if (beat.lines.empty() && beat.effects.empty()) {
            informe.diagnostics.push_back(HallazgoBeat(
                DiagnosticSeverity::Warning, "el beat no dice nada ni hace nada", beat.id));
        }

        for (const NarrativeEffect& efecto : TodosLosEfectos(beat)) {
            // Un temporizador que se arranca y no esta declarado no
            // existe: el efecto no hace nada y nadie avisa.
            if (efecto.type == EffectType::StartTimer) {
                bool declarado = false;
                for (const TimerDefinition& timer : adventure.timers) {
                    if (timer.id == efecto.arg) { declarado = true; break; }
                }
                if (!declarado) {
                    informe.diagnostics.push_back(HallazgoBeat(
                        DiagnosticSeverity::Error,
                        "se arranca el temporizador '" + efecto.arg +
                            "', que no esta declarado en 'timers'",
                        beat.id));
                }
            }
            // Una batalla sin enemigo, o un skillCheck sin habilidad, no
            // se pueden resolver.
            if (efecto.type == EffectType::StartBattle && efecto.battle.monsterId.empty()) {
                informe.diagnostics.push_back(HallazgoBeat(
                    DiagnosticSeverity::Error, "hay un combate sin enemigo", beat.id));
            }
            if (efecto.type == EffectType::SkillCheck && efecto.skillCheck.skillId.empty()) {
                informe.diagnostics.push_back(HallazgoBeat(
                    DiagnosticSeverity::Error, "hay una tirada sin habilidad", beat.id));
            }
        }
    }

    // Objetivos cuya bandera de completado no la enciende nadie: el
    // objetivo se queda pendiente para siempre y el jugador no sabe por que.
    for (const ObjectiveDefinition& objetivo : adventure.objectives) {
        if (objetivo.doneFlag.empty()) {
            informe.diagnostics.push_back(HallazgoBeat(
                DiagnosticSeverity::Error,
                "el objetivo '" + objetivo.id + "' no tiene bandera de completado"));
        } else if (escritas.count(objetivo.doneFlag) == 0) {
            informe.diagnostics.push_back(HallazgoBeat(
                DiagnosticSeverity::Warning,
                "nadie enciende '" + objetivo.doneFlag + "': el objetivo '" + objetivo.id +
                    "' no se completara nunca"));
        }
    }

    // Temporizadores que nadie arranca: el plan lo pide con estas
    // palabras («temporitzadors que no arrenca ningu»).
    for (const TimerDefinition& timer : adventure.timers) {
        if (temporizadoresArrancados.count(timer.id) == 0) {
            informe.diagnostics.push_back(HallazgoBeat(
                DiagnosticSeverity::Warning,
                "nadie arranca el temporizador '" + timer.id + "'"));
        }
        if (timer.expiraPorTiempo && timer.duracion <= 0.0f) {
            informe.diagnostics.push_back(HallazgoBeat(
                DiagnosticSeverity::Error,
                "el temporizador '" + timer.id + "' expira por tiempo y dura cero"));
        }
    }

    return informe;
}

// E-07 (cierre) — Referencias externas con catalogos. Se anade a lo que
// ya encontro el nucleo: que lo que un beat NOMBRA exista de verdad.
// Catalogo nulo = esa familia no se comprueba (el overload publico sin
// refs llama aqui con todo nulo y el comportamiento es el de siempre).
ValidationReport ValidateAdventure(const AdventureScript& adventure,
                                   const AdventureCatalogRefs& refs) {
    ValidationReport informe = ValidateAdventure(adventure);

    // Recorre los efectos buscando skillCheck y startBattle: son las dos
    // referencias que apuntan FUERA del guion. Se anaden AL FINAL para
    // no cambiar el orden de los diagnosticos que ya existian (los tests
    // de la rodaja anterior dependen de el).
    if (refs.skills != nullptr) {
        for (const NarrativeBeat& beat : adventure.beats) {
            for (const NarrativeEffect& efecto : TodosLosEfectos(beat)) {
                if (efecto.type != EffectType::SkillCheck) continue;
                const std::string& skillId = efecto.skillCheck.skillId;
                if (!skillId.empty() && refs.skills->find(skillId) == nullptr) {
                    informe.diagnostics.push_back(HallazgoBeat(
                        DiagnosticSeverity::Error,
                        "la tirada usa la habilidad '" + skillId +
                            "', que no esta en el catalogo de skills",
                        beat.id));
                }
            }
        }
    }
    if (refs.monsters != nullptr) {
        for (const NarrativeBeat& beat : adventure.beats) {
            for (const NarrativeEffect& efecto : TodosLosEfectos(beat)) {
                if (efecto.type != EffectType::StartBattle) continue;
                // Todos los del grupo (o el unico), cada id una vez.
                std::vector<std::string> vistos;
                for (const std::string& monsterId : efecto.battle.grupo()) {
                    if (monsterId.empty() || std::find(vistos.begin(), vistos.end(), monsterId) != vistos.end())
                        continue;
                    vistos.push_back(monsterId);
                    if (refs.monsters->find(monsterId) == nullptr) {
                        informe.diagnostics.push_back(HallazgoBeat(
                            DiagnosticSeverity::Error,
                            "el combate invoca a '" + monsterId +
                                "', que no esta en el catalogo de monstruos",
                            beat.id));
                    }
                }
            }
        }
    }
    return informe;
}

}  // namespace RPG
