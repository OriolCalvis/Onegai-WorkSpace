// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   C-08  (fase C, 1 j) - Un guardat antic no escriu fora de rang
//   E-02  (fase E, 3 j) - Interruptors locals i variables
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// CONTROLADOR: el bucle narrativo. Elige beat, aplica efectos sobre el
// estado que le pasan y devuelve lo que no es suyo. Los datos salieron a
// RPG/Narrative/* el 22/09/2026 (EST-25): este fichero paso de 744 a ~210.
//
// La traza de lo que ocurre viaja en NarrativeResult::log, que ya existia:
// el motor narrativo es puro a proposito (sin RNG, sin efectos laterales,
// sin escribir en disco) y quien la consume es quien la manda al Logger
// con su categoria. Eso es EST-51, no esto.

#include "RPG/NarrativeEngine.h"

#include "RPG/Narrative/FranjasDelDia.h"  // FASE1-BND-01

#include <algorithm>
#include <limits>

namespace RPG {

// ---------------------------------------------------------------------
// NarrativeEngine
// ---------------------------------------------------------------------

NarrativeResult NarrativeEngine::fire(TriggerType type, const std::string& target,
                                      NarrativeState& state) const {
    return disparaEn(type, target, state, 0, nullptr);
}

NarrativeResult NarrativeEngine::disparaEn(TriggerType type, const std::string& target, NarrativeState& state,
                                           int salto, bool* conTemas) const {
    NarrativeResult result;
    if (m_adventure == nullptr) {
        return result;
    }

    for (const NarrativeBeat& beat : m_adventure->beats) {
        if (beat.trigger.type != type) {
            continue;
        }
        // Auto ignora el target (no tiene): comparar strings vacios
        // funcionaria igual, pero dejarlo explicito evita que alguien
        // "arregle" el JSON poniendole un target a un beat auto.
        if (type != TriggerType::Auto && beat.trigger.target != target) {
            continue;
        }
        if (beat.once && state.beatFired(beat.id)) {
            continue;
        }
        if (!beat.requirements.isSatisfiedBy(state)) {
            continue;
        }

        // --- Encontrado: el PRIMERO que encaja gana (ver el header) ---
        result.fired = true;
        result.beatId = beat.id;
        result.speaker = state.conHeroe(beat.speaker);  // HEROE-01: {nombre}, {raza}...
        for (const std::string& l : beat.lines) result.lines.push_back(state.conHeroe(l));

        // Los efectos se aplican EN ORDEN y DESPUES de haber decidido el
        // beat: si un efecto enciende una flag que otro beat de mas
        // abajo esperaba, ese otro beat se disparara en el SIGUIENTE
        // disparo, no en este. Un disparo, un beat: es lo que evita las
        // cascadas accidentales en las que hablar con un NPC recorre
        // media aventura de golpe.
        aplica(beat.effects, state, result);
        // OPCIONES-01 — Lo que se puede contestar: las opciones que cumplen sus
        // requisitos ya con los efectos del beat puestos. Un beat nuevo quita
        // la eleccion que hubiera.
        state.quitaEleccion();
        EleccionPendiente eleccion;
        for (std::size_t i = 0; i < beat.opciones.size(); ++i) {
            if (!beat.opciones[i].requisitos.isSatisfiedBy(state)) continue;
            eleccion.indices.push_back(static_cast<int>(i));
            eleccion.textos.push_back(state.conHeroe(beat.opciones[i].texto));
        }
        // DIALOGOS-01 — Una pagina de charla sin temas (ya preguntados, o
        // que piden algo que no tienes) no se ensena: se pasa pagina sola,
        // sin decir nada, hasta la primera que tenga algo. Se prueba en una
        // copia del estado, para no tocar nada si no hay ninguna.
        int pasa = -1;
        bool temas = false;
        for (int i : eleccion.indices) {
            const OpcionDeDialogo& o = beat.opciones[static_cast<std::size_t>(i)];
            if (o.pasaPagina) {
                if (pasa < 0) pasa = i;
            } else if (!o.despedida) {
                temas = true;
            }
        }
        if (conTemas != nullptr) *conTemas = temas || !beat.pagina;
        if (beat.pagina && !temas && pasa >= 0) {
            constexpr int kMaxSaltos = 8;
            if (salto < kMaxSaltos) {
                NarrativeState copia = state;
                NarrativeResult nada;
                aplica(beat.opciones[static_cast<std::size_t>(pasa)].efectos, copia, nada);
                bool hay = false;
                NarrativeResult otra = disparaEn(type, target, copia, salto + 1, &hay);
                if (otra.fired && hay) {
                    state = std::move(copia);
                    if (conTemas != nullptr) *conTemas = true;
                    return otra;
                }
            }
            // Ninguna pagina tiene nada: esta, sin «Hablar de otra cosa».
            EleccionPendiente sin;
            for (std::size_t k = 0; k < eleccion.indices.size(); ++k) {
                if (beat.opciones[static_cast<std::size_t>(eleccion.indices[k])].pasaPagina) continue;
                sin.indices.push_back(eleccion.indices[k]);
                sin.textos.push_back(eleccion.textos[k]);
            }
            eleccion = std::move(sin);
        }
        if (!eleccion.textos.empty()) {
            eleccion.beatId = beat.id;
            state.ponEleccion(std::move(eleccion));
        }
        state.markBeatFired(beat.id);
        return result;
    }

    return result;
}

void NarrativeEngine::aplica(const std::vector<NarrativeEffect>& efectos, NarrativeState& state,
                             NarrativeResult& result) const {
    for (const NarrativeEffect& effect : efectos) {
        switch (effect.type) {
            case EffectType::SetFlag:
                state.setFlag(effect.arg);
                break;
            case EffectType::ClearFlag:
                state.clearFlag(effect.arg);
                break;
            case EffectType::GrantGold:
                result.goldDelta += effect.value;
                break;
            case EffectType::Log:
                result.log.push_back(effect.arg);
                break;
            // Diferido: el motor no tiene RNG ni catalogo de skills,
            // asi que solo empaqueta la peticion. GameSession la
            // resuelve contra DicePoolEngine y enciende la flag del
            // grado en el mismo NarrativeState (ver applyNarrative).
            case EffectType::SkillCheck:
                result.skillChecks.push_back(effect.skillCheck);
                break;
            // Diferido y ademas ASINCRONO: el combate dura turnos, asi
            // que la flag de victoria/derrota no se enciende aqui sino
            // cuando el BattleState se cierra (GameSession::
            // syncBattleOutcome).
            case EffectType::StartBattle:
                result.battles.push_back(effect.battle);
                state.quitaSigue();  // DIALOGOS-01: un combate corta la charla
                break;
            // F1: arrancar un temporizador de mision no es diferido:
            // el reloj vive en el NarrativeState, que esta aqui mismo.
            case EffectType::StartTimer:
                startTimer(effect.arg, state);
                break;
            // H2-04: diferido, la ficha es de GameSession.
            case EffectType::GrantMilestone:
                result.milestones += effect.value;
                if (!effect.arg.empty()) result.log.push_back("Hito: " + effect.arg + ".");
                break;
            // NIVELES-01: igual, diferido.
            case EffectType::GrantLevel:
                result.levelPoints += effect.value;
                if (!effect.arg.empty()) result.log.push_back("Por " + effect.arg + ".");
                break;
            // VARIABLES-01: los numeros viven en el estado, como las flags.
            case EffectType::SetVariable:
                state.setVariable(effect.arg, effect.value);
                break;
            case EffectType::AddVariable: {
                const long long suma = static_cast<long long>(state.variable(effect.arg)) + effect.value;
                state.setVariable(effect.arg, static_cast<int>(std::clamp<long long>(suma, effect.min, effect.max)));
                break;
            }
            // FASE1-BND-01 — El tiempo de la historia vive en el estado (dos
            // variables), como las flags: no es diferido.
            case EffectType::PasarFranja: {
                const franjas::Momento antes = franjas::momentoDe(state);
                const franjas::Momento despues = effect.arg.empty()
                                                     ? franjas::pasa(antes, effect.value)
                                                     : franjas::hasta(antes, franjas::indice(effect.arg));
                state.setVariable(franjas::kVariableDia, despues.dia);
                state.setVariable(franjas::kVariableFranja, despues.franja);
                const std::string linea = franjas::lineaRegistro(antes, despues);
                if (!linea.empty()) result.log.push_back(linea);
                break;
            }
            // Diferidos: el dado y la vida son de GameSession.
            case EffectType::TablaAleatoria:
                result.tablas.push_back(effect.tabla);
                break;
            case EffectType::Descanso:
                result.descansos.push_back(effect.arg);
                break;
            case EffectType::DarObjeto:
                result.objetos.insert(result.objetos.end(), static_cast<std::size_t>(std::max(effect.value, 0)),
                                      effect.arg);
                break;
            // MORAL-01: diferidos, la moral y la vida son de GameSession.
            case EffectType::Acto:
                result.actos.push_back(effect.arg);
                break;
            case EffectType::Herida:
                result.heridas.emplace_back(effect.arg, effect.value);
                break;
        }
    }
}

NarrativeResult NarrativeEngine::elige(int opcion, NarrativeState& state) const {
    NarrativeResult result;
    const EleccionPendiente eleccion = state.eleccion();
    if (m_adventure == nullptr || !eleccion.activa() || opcion < 0 ||
        opcion >= static_cast<int>(eleccion.indices.size())) {
        return result;
    }
    state.quitaEleccion();
    for (const NarrativeBeat& beat : m_adventure->beats) {
        if (beat.id != eleccion.beatId) continue;
        const std::size_t i = static_cast<std::size_t>(eleccion.indices[static_cast<std::size_t>(opcion)]);
        if (i >= beat.opciones.size()) return result;
        const OpcionDeDialogo& elegida = beat.opciones[i];
        result.fired = true;
        result.beatId = beat.id;
        result.speaker = state.conHeroe(elegida.speaker);
        for (const std::string& l : elegida.lineas) result.lines.push_back(state.conHeroe(l));
        // DIALOGOS-01 — Sin punto doble: «Eliges: ¿Quién eres?», no «...?.».
        const std::string dicho = state.conHeroe(elegida.texto);
        const char fin = dicho.empty() ? ' ' : dicho.back();
        result.log.push_back("Eliges: " + dicho + (fin == '.' || fin == '?' || fin == '!' ? "" : "."));
        aplica(elegida.efectos, state, result);
        // DIALOGOS-01 — La charla sigue... salvo que la opcion empiece un
        // combate: lo que quede se habla despues, si se vuelve.
        if (elegida.sigue && result.battles.empty())
            state.ponSigue(static_cast<int>(beat.trigger.type), beat.trigger.target);
        return result;
    }
    return result;  // el beat ya no esta (otra aventura): la eleccion se pierde
}

void NarrativeEngine::startTimer(const std::string& timerId, NarrativeState& state) const {
    if (m_adventure == nullptr) {
        return;
    }
    for (const TimerDefinition& t : m_adventure->timers) {
        if (t.id != timerId) {
            continue;
        }
        TimerState& ts = state.timerRef(t.id);
        if (ts.activo || ts.expirado) {
            return;  // ya esta en marcha o ya se consumo: no se reinicia
        }
        ts.activo = true;
        ts.restante = t.duracion;
        ts.avisosDados.assign(t.avisos.size(), false);
        return;
    }
    // Id desconocido = no-op (mismo criterio permisivo que el resto del
    // motor; validar_todo.py es quien lo denuncia en el contenido).
}

void NarrativeEngine::tickTimers(NarrativeState& state, float dtActivo,
                                 const std::string& nivelActual) const {
    if (m_adventure == nullptr) {
        return;
    }

    // 1) Timers que arrancan por flag (ademas del efecto startTimer).
    for (const TimerDefinition& t : m_adventure->timers) {
        if (!t.arrancaPor.empty() && state.hasFlag(t.arrancaPor)) {
            startTimer(t.id, state);
        }
    }

    for (const TimerDefinition& t : m_adventure->timers) {
        TimerState& ts = state.timerRef(t.id);

        // C-08 — El vector de avisos se ajusta a la definicion ACTUAL
        // antes de que nadie lo indexe. startTimer() lo dimensiona bien,
        // pero un guardado lo restaura con el tamano que tenia el dia en
        // que se guardo: anadir un aviso a una aventura convertia todas
        // las partidas ya empezadas en una escritura fuera de rango
        // (ts.avisosDados[i] con i < t.avisos.size()).
        //
        // resize() y no assign(): conserva el prefijo, es decir los
        // avisos que el jugador YA recibio, y rellena con false solo los
        // nuevos. Quitar un aviso tambien queda cubierto (el vector
        // sobra y se recorta) aunque ese caso no petaba.
        if (ts.avisosDados.size() != t.avisos.size()) {
            ts.avisosDados.resize(t.avisos.size(), false);
        }

        if (!ts.activo) {
            continue;
        }

        // 2) Expirar por SALIR DE LA ZONA: la casa arde aunque quedara
        //    tiempo en el reloj (bitacora 2.18).
        if (t.expiraPorSalirDeZona && !nivelActual.empty() && nivelActual != t.zona) {
            ts.activo = false;
            ts.expirado = true;
            state.setFlag(t.flagExpira);
            continue;
        }

        // 3) El tiempo solo avanza con dt ACTIVO: el orquestador pasa 0
        //    en combate, dialogo, pausa o AFK.
        if (!t.expiraPorTiempo || dtActivo <= 0.0f) {
            continue;
        }
        ts.restante -= dtActivo;

        // Avisos por umbral: fraccion sobre el tiempo RESTANTE.
        for (std::size_t i = 0; i < t.avisos.size(); ++i) {
            if (!ts.avisosDados[i] &&
                ts.restante <= t.duracion * t.avisos[i].fraccion) {
                ts.avisosDados[i] = true;
                state.setFlag(t.avisos[i].flag);
            }
        }

        if (ts.restante <= 0.0f) {
            ts.restante = 0.0f;
            ts.activo = false;
            ts.expirado = true;
            state.setFlag(t.flagExpira);
        }
    }
}

std::vector<ObjectiveStatus> NarrativeEngine::objectives(const NarrativeState& state) const {
    std::vector<ObjectiveStatus> out;
    if (m_adventure == nullptr) {
        return out;
    }
    out.reserve(m_adventure->objectives.size());
    for (const ObjectiveDefinition& def : m_adventure->objectives) {
        out.push_back(ObjectiveStatus{def.id, def.text, state.hasFlag(def.doneFlag)});
    }
    return out;
}

bool NarrativeEngine::allObjectivesDone(const NarrativeState& state) const {
    // Sin aventura no hay objetivos que cumplir; devolver true aqui
    // haria que "aventura terminada" y "no hay aventura" fueran
    // indistinguibles para quien pregunte.
    if (m_adventure == nullptr || m_adventure->objectives.empty()) {
        return false;
    }
    for (const ObjectiveDefinition& def : m_adventure->objectives) {
        if (!state.hasFlag(def.doneFlag)) {
            return false;
        }
    }
    return true;
}

NarrativeResult NarrativeEngine::sigue(NarrativeState& state) const {
    if (!state.haySigue()) return NarrativeResult{};
    const TriggerType tipo = static_cast<TriggerType>(state.sigueTipo());
    const std::string objetivo = state.sigueObjetivo();
    state.quitaSigue();
    return fire(tipo, objetivo, state);
}

}  // namespace RPG
