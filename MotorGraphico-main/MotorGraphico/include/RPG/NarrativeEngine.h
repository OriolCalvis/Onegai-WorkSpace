// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   E-02  (fase E, 3 j) - Interruptors locals i variables
//   G-07  (fase G, 2 j) - Congelacion del esquema
//
// AVISO - FICHERO DE CRUCE. Con 2 paquetes encima, vigila que ningun
// agente E-02 deje cambios sin commitear antes de tocar el bloque de
// ownership. La declaracion de serializeAdventure() (G-07) es el unico
// miembro G-07 aqui; el descanso del fichero es E-02.
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "RPG/Narrative/AdventureScript.h"
#include "RPG/Narrative/NarrativeState.h"

// CONTROLADOR del flujo narrativo. Busca que beat toca, aplica sus
// efectos sobre el estado que le pasan y devuelve lo que no es suyo
// (oro, combates, tiradas) para que lo aplique quien corresponda.
//
// Su unico miembro es un colaborador —la aventura activa, que no posee—,
// que es la norma N3: el estado del mundo viaja como parametro.
//
// 22/09/2026 (EST-25): los datos salieron a RPG/Narrative/*. Este fichero
// paso de 543 a ~150 lineas y sigue siendo el include que usan los 22
// consumidores, asi que ninguno tuvo que cambiar.

// NarrativeEngine -- la "linea de eventos" del juego (P0-13 de
// GAMEMACHINE_NECESIDADES.md), en su version MINIMA y ampliable.
//
// La idea de una sola frase: **el mundo tiene un conjunto de flags, y una
// aventura es una lista de beats que se disparan cuando ocurre algo y las
// flags encajan**. Nada mas. Todo lo demas (misiones, ramas de dialogo,
// eventos globales, hitos de tier) se construye ANADIENDO tipos de
// disparador y tipos de efecto a este mismo bucle, no inventando sistemas
// paralelos.
//
//     ocurre algo  -->  NarrativeEngine::fire(trigger, target, state)
//                       |
//                       +-- recorre los beats EN ORDEN DE ARCHIVO
//                       +-- el primero cuyo trigger y "requires" encajen, gana
//                       +-- aplica sus efectos sobre el NarrativeState
//                       +-- devuelve speaker + lineas para el HUD
//
// "El primero que encaja gana" es deliberado y no una simplificacion
// perezosa: hace que el contenido se lea de arriba abajo como una lista
// de casos especiales seguida del caso general ("si aun no has hablado
// con el guardia, dice X; si ya, dice Y"), que es exactamente como se
// escriben las cartas del Llibre VI del GDD. Y es determinista: el mismo
// estado produce siempre el mismo beat, asi que un demo con asserts lo
// puede probar entero sin ventana.
//
// GL-free y sin dependencias de Render/: vive en motor_rpg junto a
// CharacterSheet/ConditionEngine. GameSession lo enchufa opcionalmente
// (ver GameSession::setNarrative), pero el motor narrativo no sabe que
// existe una GameSession -- misma direccion de dependencias que
// BattleState con ICombatant.


namespace RPG {

// ---------------------------------------------------------------------
// Resultado de un disparo
// ---------------------------------------------------------------------

struct NarrativeResult {
    bool fired = false;  // false = ningun beat encajaba (el NPC dira su dialogo de catalogo)
    std::string beatId;
    std::string speaker;
    std::vector<std::string> lines;

    // Efectos que el motor narrativo NO puede aplicar por si mismo
    // porque no son suyos: el oro vive en GameSession. Se devuelven para
    // que los aplique quien corresponda. Aqui es donde apareceran
    // "openShopId", "startBattleWith", "grantQuestId"... cuando toque.
    int goldDelta = 0;
    std::vector<std::string> log;
    // Tiradas Nd6 pedidas por el beat: el motor las empaqueta y GameSession
    // las resuelve con DicePoolEngine (no hay RNG en el motor narrativo).
    // Plural como "log": un beat puede pedir mas de una tirada.
    std::vector<SkillCheckRequest> skillChecks;
    // Combates pedidos por el beat. Plural por el mismo motivo que las
    // tiradas, aunque de momento GameSession solo puede tener uno vivo:
    // los siguientes quedan encolados.
    std::vector<BattleRequest> battles;
    // H2-04 — Hitos concedidos por el beat (efecto grantMilestone). Los
    // aplica GameSession sobre la ficha del heroe.
    int milestones = 0;
    // NIVELES-01 — Avance de nivel concedido por el beat (efecto
    // grantLevel), en puntos: 100 = un nivel.
    int levelPoints = 0;
    // FASE1-BND-01 — Las tablas aleatorias (se tiran como las tiradas: en
    // GameSession, que tiene el RNG) y los descansos ("corto" o "largo": la
    // vida es de GameSession) y los objetos que da (un id por unidad: la
    // mochila también es de GameSession).
    std::vector<TablaAleatoriaRequest> tablas;
    std::vector<std::string> descansos;
    std::vector<std::string> objetos;
    // MORAL-01 — Los actos (ids del catalogo) y las heridas (lo que hiere y
    // el dado): la moral y la vida son de GameSession.
    std::vector<std::string> actos;
    std::vector<std::pair<std::string, int>> heridas;
};

class NarrativeEngine {
public:
    // El puntero NO es propietario: la definicion debe seguir viva
    // mientras el motor la use (mismo criterio que GameSession con su
    // ObjectCatalog). nullptr = sin aventura activa, todo devuelve
    // fired=false, que es exactamente el comportamiento que deja al
    // juego funcionando igual que antes de existir este sistema.
    void setAdventure(const AdventureScript* adventure) { m_adventure = adventure; }
    const AdventureScript* adventure() const { return m_adventure; }

    // El bucle entero del sistema. Busca el primer beat cuyo trigger sea
    // (type, target) y cuyas condiciones se cumplan, aplica sus efectos
    // sobre "state" y lo devuelve.
    NarrativeResult fire(TriggerType type, const std::string& target, NarrativeState& state) const;

    // Azucar para las llamadas habituales.
    NarrativeResult talkTo(const std::string& npcId, NarrativeState& state) const {
        return fire(TriggerType::Talk, npcId, state);
    }
    NarrativeResult enterLevel(const std::string& levelId, NarrativeState& state) const {
        return fire(TriggerType::Enter, levelId, state);
    }
    // Se llama despues de cada accion del jugador (o una vez por frame:
    // es barato y determinista). Es lo que hace que un cierre salte solo
    // en cuanto se cumple su condicion, sin que el contenido tenga que
    // colgarlo del ultimo NPC de la cadena -- que es justo lo que
    // permite que la aventura se resuelva en CUALQUIER orden.
    NarrativeResult tick(NarrativeState& state) const {
        return fire(TriggerType::Auto, "", state);
    }

    // OPCIONES-01 — El jugador contesta: la opcion n (0..) de las que ofrece
    // la eleccion pendiente (NarrativeState::eleccion). Aplica sus efectos
    // sobre "state" (como un beat), quita la eleccion y devuelve sus lineas
    // y lo que no es suyo (oro, tiradas, combates...). fired=false si no
    // habia eleccion o n no es una de sus opciones.
    NarrativeResult elige(int opcion, NarrativeState& state) const;

    // DIALOGOS-01 — Si la ultima opcion elegida pedia que la charla siguiera,
    // vuelve a disparar su trigger (hablar con el mismo, entrar en el mismo
    // sitio o lo automatico) y la gasta. Sin charla pendiente, nada.
    // Quien la llama mira antes lo automatico (tick): lo que acaba de pasar
    // (una tirada, un combate) se cuenta antes de seguir hablando.
    NarrativeResult sigue(NarrativeState& state) const;

    // F1: reloj de los temporizadores de misión. El orquestador lo llama
    // cada frame con el dt de EXPLORACIÓN ACTIVA (0.0 si hay combate,
    // diálogo, pausa o el jugador lleva un rato sin tocar nada — AFK) y
    // con el nivel donde está el jugador. Arranca los timers cuya flag
    // arrancaPor esté encendida, enciende las flags de aviso al cruzar
    // sus umbrales y las de expiración (por tiempo o por salir de la
    // zona). Las consecuencias las disparan los beats Auto de siempre.
    void tickTimers(NarrativeState& state, float dtActivo,
                    const std::string& nivelActual) const;

    // Arranca un timer por id (efecto StartTimer). Id desconocido = no-op.
    void startTimer(const std::string& timerId, NarrativeState& state) const;

    // Diario de misiones: los objetivos de la aventura con su estado
    // resuelto contra las flags actuales. Vacio si no hay aventura.
    std::vector<ObjectiveStatus> objectives(const NarrativeState& state) const;

    // true si todos los objetivos estan cumplidos.
    bool allObjectivesDone(const NarrativeState& state) const;

private:
    // fire() de verdad. DIALOGOS-01: "salto" cuenta las paginas de charla
    // vacias que se han saltado; "conTemas" dice si el beat que queda
    // ofrece algo mas que pasar pagina y despedirse.
    NarrativeResult disparaEn(TriggerType type, const std::string& target, NarrativeState& state, int salto,
                              bool* conTemas) const;

    // Los efectos de un beat o de una opcion, en orden: lo del estado aqui y
    // lo de los demas en "result".
    void aplica(const std::vector<NarrativeEffect>& efectos, NarrativeState& state,
                NarrativeResult& result) const;

    const AdventureScript* m_adventure = nullptr;
};

}  // namespace RPG
