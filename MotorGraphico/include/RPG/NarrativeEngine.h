#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "Core/Errors/Result.h"

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
// Estado del mundo: las flags
// ---------------------------------------------------------------------

// TODO el estado narrativo de una partida es un conjunto de strings.
//
// Por que strings y no un enum o una struct con campos: el contenido
// (JSON de aventuras) tiene que poder inventar flags nuevas sin
// recompilar el motor. Una aventura escrita manana declara
// "rumor_bardo" y funciona; con un enum habria que tocar C++ por cada
// linea de dialogo nueva, que es justo lo que mata a los sistemas
// narrativos.
//
// beatsFired es aparte de las flags a proposito: "este beat ya se ha
// disparado" es contabilidad del motor (para once=true), no estado del
// mundo que el contenido deba consultar. Si una aventura necesita
// preguntar "¿ya paso esto?", el beat debe poner una flag explicita --
// asi el contenido nunca depende de ids internos de beats.
class NarrativeState {
public:
    bool hasFlag(const std::string& flag) const { return m_flags.count(flag) > 0; }
    void setFlag(const std::string& flag) { m_flags.insert(flag); }
    void clearFlag(const std::string& flag) { m_flags.erase(flag); }

    bool beatFired(const std::string& beatId) const { return m_beatsFired.count(beatId) > 0; }
    void markBeatFired(const std::string& beatId) { m_beatsFired.insert(beatId); }

    std::size_t flagCount() const { return m_flags.size(); }
    // Copia ordenada, para HUD/depuracion/guardado. No se expone el set
    // interno por referencia para que nadie lo mute por la puerta de
    // atras (mismo criterio que ObjectCatalog con su mapa).
    std::vector<std::string> flags() const;

    void reset();

private:
    std::unordered_set<std::string> m_flags;
    std::unordered_set<std::string> m_beatsFired;
};

// ---------------------------------------------------------------------
// Condiciones
// ---------------------------------------------------------------------

// Condicion sobre las flags. Las tres listas se combinan con AND entre
// si (todas deben cumplirse); dentro de cada lista, allFlags es AND,
// anyFlags es OR y notFlags es NOR.
//
// Vacia = siempre cierta. Es el default y es el caso mas comun (un beat
// de bienvenida no condiciona nada), asi que el JSON puede omitir
// "requires" entero.
struct NarrativeCondition {
    std::vector<std::string> allFlags;  // hacen falta TODAS
    std::vector<std::string> anyFlags;  // hace falta AL MENOS UNA
    std::vector<std::string> notFlags;  // no debe haber NINGUNA

    bool isSatisfiedBy(const NarrativeState& state) const;
    bool isEmpty() const {
        return allFlags.empty() && anyFlags.empty() && notFlags.empty();
    }
};

// ---------------------------------------------------------------------
// Disparadores
// ---------------------------------------------------------------------

// Que hace que un beat entre en juego.
//
//  Talk  -- el jugador habla con el NPC "target" (target = objectId del
//           ObjectCatalog: "guardia", "anciana", "bardo").
//  Enter -- el jugador entra en el nivel "target" (target = ruta o id de
//           nivel). Aun no lo llama GameSession; el demo lo prueba.
//  Auto  -- se evalua en cada tick() sin que el jugador haga nada. Es el
//           disparador de los cierres y de las consecuencias diferidas:
//           "cuando se cumplan estas flags, ocurre esto".
//
// Los tres siguientes que tocaria anadir cuando haga falta, sin cambiar
// nada mas: Pickup (recoger un objeto), Defeat (derrotar a un enemigo) y
// Temporal (pasos/dias, como el "trigger.type: temporal" que ya traen
// las cartas de assets/catalogs/events.json).
enum class TriggerType { Talk, Enter, Auto };

struct BeatTrigger {
    TriggerType type = TriggerType::Auto;
    std::string target;  // vacio para Auto
};

// ---------------------------------------------------------------------
// Efectos
// ---------------------------------------------------------------------

// Que cambia en el mundo cuando un beat se dispara.
//
// Este enum es el punto de crecimiento natural del sistema: la lista de
// P0-12 (OpenShop, GrantQuest, StartBattle, ChangeFactionRep,
// LevelTransition, SetMilestone...) se anade AQUI, y NarrativeResult
// gana un campo por cada uno para que el orquestador (GameSession, la
// Application) lo ejecute. El motor narrativo decide QUE pasa; no sabe
// como se abre una tienda ni como se empieza un combate.
enum class EffectType {
    SetFlag,    // arg = flag a encender
    ClearFlag,  // arg = flag a apagar
    GrantGold,  // value = oro (puede ser negativo: un peaje)
    Log,        // arg = linea suelta para el log de la partida
    // Pide al orquestador que resuelva una tirada Nd6 contra una CD y
    // encienda la flag del grado obtenido. Diferido como grantGold: el
    // motor narrativo no tiene RNG ni catalogo de skills, asi que solo
    // empaqueta la peticion en NarrativeResult.skillChecks y deja que
    // GameSession la resuelva con DicePoolEngine (ver GameSession::
    // applyNarrative). El grado Nd6 (BOTCH/PARTIAL/SUCCESS/CRITICAL) se
    // traduce a CUATRO flags distintas, una por grado: asi el contenido
    // puede ramificar en cuatro variantes por cada tirada (GDD §7.1).
    SkillCheck
};

// Una tirada de habilidad pedida desde un beat. El motor la rellena al
// parsear el effect "skillCheck" y la pasa tal cual al orquestador.
//
//   skillId  -> id en SkillCatalog (ej. "percepcion"); su casting_stat
//               fija el tamano N de la pool (N = stat del jugador).
//   cd       -> dificultad Nd6 en escala 0.0..3.0 (GDD §7.1:
//               0.5 Facil, 1 Normal, 1.5 Dificil, 2 Muy dificil,
//               2.5+ Extraordinaria).
//   flag*    -> cuatro flags mutuamente excluyentes: se enciende SOLO la
//               del grado obtenido. El contenido suele quererlas asi para
//               poder cerrarlas (clearFlag) y re-tirar, o para acumular
//               "mejor resultado hasta ahora" con anyFlags.
struct SkillCheckRequest {
    std::string skillId;
    float cd = 1.0f;
    std::string flagBotch;
    std::string flagPartial;
    std::string flagSuccess;
    std::string flagCritical;
};

struct NarrativeEffect {
    EffectType type = EffectType::SetFlag;
    std::string arg;
    int value = 0;
    // Solo para type == SkillCheck. Para los demas tipos se ignora.
    SkillCheckRequest skillCheck;
};

// ---------------------------------------------------------------------
// Beat: la unidad de contenido (la "carta" del Llibre VI del GDD)
// ---------------------------------------------------------------------

struct NarrativeBeat {
    std::string id;  // unico dentro de la aventura; solo para once/depuracion
    BeatTrigger trigger;
    NarrativeCondition requirements;

    // Lo que se muestra. speaker vacio = voz del narrador (un cierre, una
    // consecuencia): el HUD decide si dibuja marco de dialogo o no.
    std::string speaker;
    std::vector<std::string> lines;

    std::vector<NarrativeEffect> effects;

    // once = el beat no vuelve a dispararse nunca, aunque sus
    // condiciones sigan cumpliendose. Sin esto, un beat Auto de cierre
    // se dispararia en cada tick() para siempre. La alternativa
    // "apagate a ti mismo con una flag" tambien vale y es lo que hace la
    // aventura de ejemplo, porque asi el estado queda EXPLICITO en las
    // flags (y por tanto en el guardado) en vez de escondido en la
    // contabilidad interna del motor.
    bool once = false;
};

// ---------------------------------------------------------------------
// Objetivos (lo que ve el jugador en el diario de misiones)
// ---------------------------------------------------------------------

// Un objetivo no tiene logica propia: es una etiqueta de texto atada a
// una flag. Se marca como cumplido cuando la flag esta encendida. Toda
// la logica sigue viviendo en los beats, y el diario es una VISTA de las
// flags -- no una segunda fuente de verdad que pueda desincronizarse.
struct ObjectiveDefinition {
    std::string id;
    std::string text;
    std::string doneFlag;
};

struct ObjectiveStatus {
    std::string id;
    std::string text;
    bool done = false;
};

// ---------------------------------------------------------------------
// Aventura: el archivo de contenido
// ---------------------------------------------------------------------

// Script y no "Definition" porque RPG::AdventureDefinition ya existe
// (RpgCoreDefinitions.h) y es otra cosa: la CARTA de aventura del
// catalogo (adv_10 y sus 300 hermanas en assets/catalogs/adventures.json),
// con su tier, su faccion, sus enemigos y sus loot tables. Eso es la
// ficha; esto es el GUION que se puede jugar.
//
// Las dos se encontraran cuando toque: un AdventureScript llevara el id
// de su AdventureDefinition, y ahi es donde las 300 cartas ya escritas
// dejaran de ser metadatos y empezaran a ser partidas. De momento van
// por separado a proposito -- primero que funcione una.
struct AdventureScript {
    std::string id;
    std::string name;
    std::string description;
    std::vector<ObjectiveDefinition> objectives;
    std::vector<NarrativeBeat> beats;

    // Mismo contrato de carga que ObjectCatalog/LevelLoader: Result<T>,
    // nunca excepciones (es contenido, no arranque), y todo-o-nada (si
    // el beat 7 esta mal, no se aplica ninguno). Campos estructurales
    // (id de aventura, id de beat, tipo de trigger/efecto desconocido)
    // son Error; los de detalle tienen default.
    static Result<AdventureScript> loadFromString(const std::string& jsonText);
    static Result<AdventureScript> loadFromFile(const std::string& path);
};

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

    // Diario de misiones: los objetivos de la aventura con su estado
    // resuelto contra las flags actuales. Vacio si no hay aventura.
    std::vector<ObjectiveStatus> objectives(const NarrativeState& state) const;

    // true si todos los objetivos estan cumplidos.
    bool allObjectivesDone(const NarrativeState& state) const;

private:
    const AdventureScript* m_adventure = nullptr;
};

}  // namespace RPG
