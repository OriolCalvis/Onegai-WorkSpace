#pragma once

// DATOS: el vocabulario de una aventura.
//
// Que dispara un beat, que condiciones pide, que efectos produce y que
// le pide al resto del juego (un combate, una tirada). Son datos puros:
// la unica logica que llevan es NarrativeCondition::isSatisfiedBy, que
// solo mira un NarrativeState y no toca nada.
//
// Salio de include/RPG/NarrativeEngine.h el 22/09/2026 (EST-25):
// aquel fichero tenia 543 lineas y mezclaba el estado del mundo,
// el vocabulario de los beats, el guion y el motor. Se movio, no
// se reescribio. Ver planificacion/PLAN_ESTABILIZACION.md.

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

#include "RPG/Narrative/NarrativeState.h"

namespace RPG {

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
    // NIVELES-01 (Oriol, 06/10): las misiones de mas prestigio no se
    // anuncian como «necesitas nivel 8»; surgen cuando el heroe llega
    // («que sea una necesidad surgida, no algo estatico»). Un beat auto con
    // nivelMinimo salta solo al llegar a ese nivel. 0 = sin requisito.
    int nivelMinimo = 0;
    int tierMinimo = 0;
    // VARIABLES-01 (fase 1 de «Los Perdidos de Boundington — revisión para
    // jugarla entera»): los puntos, el afecto de Venides y la reputación con
    // los gremios son números. Cada rango pide que la variable (0 si nadie la
    // ha tocado) esté entre min y max, los dos incluidos.
    struct RangoDeVariable {
        std::string nombre;
        int min = std::numeric_limits<int>::min();
        int max = std::numeric_limits<int>::max();
    };
    std::vector<RangoDeVariable> variables;
    // BND-04 — La reputacion con facciones ("reputacion": {"aegroum": {"min": 10}}),
    // la de la ficha del heroe (-100..100), min y max incluidos. Todas a la vez.
    struct RangoDeReputacion {
        std::string faccion;
        int min = std::numeric_limits<int>::min();
        int max = std::numeric_limits<int>::max();
    };
    std::vector<RangoDeReputacion> reputaciones;
    // MORAL-01 — Un tramo de la moral asentada del heroe ("moral": {"min",
    // "max"}, los dos incluidos). Para lineas y opciones de mas, nunca para
    // cerrar el camino (wf-04: «el villano juega, no es expulsado del juego»).
    // DIALOGOS-01 — Donde tiene que estar el heroe (uno de estos niveles).
    std::vector<std::string> lugares;
    int moralMin = std::numeric_limits<int>::min();
    int moralMax = std::numeric_limits<int>::max();

    bool isSatisfiedBy(const NarrativeState& state) const;
    bool tieneMoral() const {
        return moralMin != std::numeric_limits<int>::min() || moralMax != std::numeric_limits<int>::max();
    }
    bool isEmpty() const {
        return allFlags.empty() && anyFlags.empty() && notFlags.empty() && nivelMinimo <= 0 && tierMinimo <= 0 &&
               variables.empty() && reputaciones.empty() && !tieneMoral() && lugares.empty();
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
    SkillCheck,
    // Pide al orquestador que arranque un combate contra un enemigo del
    // ObjectCatalog. Mismo criterio que SkillCheck: el motor narrativo no
    // tiene BattleState ni catalogo de enemigos, asi que empaqueta la
    // peticion en NarrativeResult.battles y la resuelve GameSession.
    //
    // La diferencia con SkillCheck es el TIEMPO. Una tirada Nd6 se resuelve
    // dentro del mismo applyNarrative y la flag esta puesta al volver. Un
    // combate dura turnos: el beat termina, el jugador pelea, y solo
    // despues se sabe el resultado. Por eso las flags de victoria/derrota
    // no las enciende applyNarrative sino syncBattleOutcome, cuando el
    // BattleState se cierra. El contenido no nota la diferencia: pide una
    // flag y esa flag acaba encendida.
    StartBattle,
    // F1 (bitacora 2.18): arranca el temporizador de mision cuyo id va en
    // "arg". El reloj solo avanza en exploracion activa (ni combate, ni
    // dialogo, ni pausa, ni AFK — de esas reglas se ocupa el orquestador,
    // que es quien pasa el dt activo a tickTimers).
    StartTimer,
    // H2-04 — Concede un hito al heroe: el paso de tier, que es llegar al
    // nivel 20, 40... (value = cuantos, 1 si no se dice; arg = por que, sale
    // en el registro). Para el final de una campana. Diferido como
    // grantGold: la ficha y las reglas de tier viven en GameSession.
    GrantMilestone,
    // NIVELES-01 — Sube niveles al heroe (arg = por que). En el JSON
    // "value" son niveles y admite fracciones (0.25 = un cuarto de nivel, 1
    // si no se dice); aqui value guarda PUNTOS (100 = un nivel). Veinte
    // niveles por tier, y sin hito no se pasa del 20.
    GrantLevel,
    // VARIABLES-01 — Pone una variable con nombre (arg) a value.
    SetVariable,
    // VARIABLES-01 — Suma value (o resta, si es negativo) a la variable arg,
    // sin salirse de [min, max] si el efecto los trae (la reputación va de 0
    // a 4).
    AddVariable,
    // FASE1-BND-01 (fase 1 de «Los Perdidos de Boundington — revisión para
    // jugarla entera»: «Franjas del día»). Pasa el tiempo de la historia: el
    // día y la franja (mañana, tarde, noche) son las variables "dia" y
    // "franja" (RPG/Narrative/FranjasDelDia.h). Sin arg, pasan value franjas
    // (1 si no se dice); con arg ("manana", "tarde", "noche"), hasta la
    // siguiente vez que sea esa franja (dormir es "manana"). Después de la
    // noche viene la mañana del día siguiente.
    PasarFranja,
    // FASE1-BND-01 («Tabla aleatoria»: el 1d4 del traje roto de la fiesta).
    // Diferido como SkillCheck: el motor narrativo no tiene RNG. GameSession
    // tira 1d<dado> y enciende la flag de esa cara.
    TablaAleatoria,
    // FASE1-BND-01 («Descanso corto y largo»: la comida con la familia de
    // Pico Dragón). arg = "corto" (1d8 de vida por tier, la regla del
    // reglamento, §7.5) o "largo" (toda la vida). Diferido: la vida es de
    // GameSession.
    Descanso,
    // FASE1-BND-01 («Da:» de las fichas de escena: las 2 pociones de
    // Luisarda). arg = id del objeto en el catálogo, value = cuántos (1 si no
    // se dice). Diferido: la mochila es de GameSession.
    DarObjeto,
    // MORAL-01 (Oriol, 08/10: «trabajar el carma con las diferentes opciones
    // que el jugador vaya marcando»). arg = id del acto en
    // assets/rules/actos.json: lo que pesa en la moral (al dormir), lo que
    // toca la pureza y a quien se lo cuentan. Diferido: la moral es de la
    // sesion (Game/MoralDeLaSesion).
    Acto,
    // BND-ALT-03 (la barricada en llamas: «arriesgandose a obtener dano por
    // fuego»). value = caras del dado (1d<value>), arg = lo que hiere («El
    // fuego»). No mata: deja al heroe con 1 PV como poco. Diferido: la vida es
    // de GameSession.
    Herida
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
// Combate pedido desde un beat (effect "startBattle").
struct BattleRequest {
    std::string monsterId;    // id de un ObjectDefinition de categoria enemy (el primero si hay grupo)
    std::string flagVictory;  // se enciende al ganar
    std::string flagDefeat;   // se enciende al perder (y al huir, si no hay flagHuida)
    // Combate en grupo (Oriol, 05/10: «los combates siguen siendo de uno
    // contra uno»): todos los que salen, en orden y con repeticiones
    // («perdido_saqueador» tres veces). Vacio = solo monsterId.
    std::vector<std::string> enemigos;
    static constexpr std::size_t kMaxEnemigos = 8;
    std::vector<std::string> grupo() const {
        return enemigos.empty() ? std::vector<std::string>{monsterId} : enemigos;
    }
    // ACOMPANANTES-01 — Quien pelea a tu lado (ids de la lista
    // "acompanantes" de la aventura), cada uno si su flag esta encendida
    // (vacia = siempre). Oriol (05/10): «solo en Boundington, y solo en
    // algunos momentos de la historia».
    struct ConAcompanante {
        std::string id;
        std::string siFlag;
    };
    std::vector<ConAcompanante> acompanantes;
    static constexpr std::size_t kMaxAcompanantes = 3;
    // CAER-01 — Huir y caer, segun la mision. Oriol (08/10): «de entrada, no
    // tienes ninguna obligacion de terminarla [...] a no ser que sean
    // momentos de historia ineludibles». Asi que, por defecto, perder un
    // combate de la historia no acaba la partida: el heroe se levanta con un
    // cuarto de la vida, los que ganaron se van y se enciende flagDefeat.
    //  - flagHuida: se enciende al huir. Vacia = flagDefeat, como antes.
    //  - ineludible: un momento que no se puede esquivar. No se puede huir
    //    (la carta sale bloqueada) y caer acaba la partida.
    // Van al final para no mover a nadie que rellene los de arriba por nombre.
    std::string flagHuida;
    bool ineludible = false;
    // FASE1-BND-01 — «Huir a 0 PV y pelear sin matar» (Campaña 01: Sandes
    // «escapará al quedarse a 0 de vida»; la pelea de taberna, «si apalean,
    // pero no matan al que la inicia»). huyen: los ids del grupo que a 0 PV
    // escapan en vez de caer; sinMatar: los que caen quedan fuera de combate,
    // vivos. Para el combate es lo mismo (a 0 PV dejan de pelear); cambia lo
    // que se cuenta, y la aventura sigue por sus flags.
    std::vector<std::string> huyen;
    bool sinMatar = false;
    // ROSITA-01 — Los que llegan a mitad del combate (la antigua compañía de
    // Rosita; Oriol: «cuando Sandes esté a 1/4 de vida [...] y a punto de
    // morir»). Cada refuerzo llega una vez, la primera vez que, con
    // `requiere` cumplido, el enemigo principal (monsterId, el primero del
    // grupo) baja a `vidaEnemigo` de su vida o el heroe a `vidaHeroe` (0 = no
    // se mira; lo primero que pase). Entonces se encienden sus `flags`, sus
    // `lineas` van al registro del combate y sus acompañantes (ids de la
    // lista de la aventura, como mucho kMaxAcompanantes) se suman a tu bando.
    // Lo hace Game::llegaRefuerzos (Game/Refuerzos.h).
    struct Refuerzo {
        NarrativeCondition requiere;
        float vidaEnemigo = 0.0f;
        float vidaHeroe = 0.0f;
        // AV13-34 (Oriol, 14/10, D0015 §13): llega cuando el enemigo principal, a `vidaEnemigo` o
        // menos, te da un golpe mortal; scripteado para que pase siempre (BattleState::ponGolpeMortal).
        bool golpeMortal = false;
        std::vector<std::string> flags;
        std::vector<std::string> lineas;
        std::vector<ConAcompanante> acompanantes;
    };
    std::vector<Refuerzo> refuerzos;
};

// FASE1-BND-01 — Una tabla aleatoria pedida desde un beat: 1d<dado>, y la
// cara que salga enciende su flag (flags[cara - 1]). Hay tantas flags como
// caras.
struct TablaAleatoriaRequest {
    int dado = 0;
    std::vector<std::string> flags;
    std::string que;  // lo que se tira, para el registro («El traje»); vacío = nada
};

struct SkillCheckRequest {
    std::string skillId;
    // Ed. 3 (bitácora 2.17): la prueba es enfrentada — el rival declarado
    // por el beat tira rivalDice dados. "cd" queda EN DESUSO, solo como
    // compatibilidad de beats antiguos (se convierte a dados: 0.5→1,
    // 1.0→2, 1.5→3, 2.0→4, 2.5→5).
    int rivalDice = 0;
    // MORAL-01 (Oriol, 08/10: «superar 2 seises en tu tirada de carisma», «a
    // la letra»): los exitos que hay que sacar uno solo, sin rival. 0 = la
    // enfrentada de siempre. Con exitos, el grado es: exito con exitos o mas
    // (critico si todos 6), parcial si falta uno como mucho, y fallo (o pifia).
    int exitos = 0;
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
    // Solo para type == AddVariable: los topes (sin ellos, ninguno).
    int min = std::numeric_limits<int>::min();
    int max = std::numeric_limits<int>::max();
    // Solo para type == SkillCheck. Para los demas tipos se ignora.
    SkillCheckRequest skillCheck;
    // Solo para type == StartBattle.
    BattleRequest battle;
    // Solo para type == TablaAleatoria (FASE1-BND-01).
    TablaAleatoriaRequest tabla;
};

// ---------------------------------------------------------------------
// Beat: la unidad de contenido (la "carta" del Llibre VI del GDD)
// ---------------------------------------------------------------------

// OPCIONES-01 (fase 1 de la revisión de Boundington: «Unirse al culto o no,
// Sandes, entregar o salvar a Verina, ayudar a Venides o acabar con él»). Lo
// que el jugador puede contestar al acabar de leer un beat. Solo se ofrecen
// las que cumplen sus requisitos (mirados justo después de los efectos del
// beat); al elegir una, sus efectos se aplican como los de un beat y, si
// trae líneas, se leen (quien las dice: speaker, o el del beat si no lo
// dice). Sin líneas, el diálogo se cierra y la aventura sigue por sus flags.
struct OpcionDeDialogo {
    std::string texto;  // lo que se lee en la lista: corto («Entregarle a Verina»)
    NarrativeCondition requisitos;
    std::string speaker;
    std::vector<std::string> lineas;
    std::vector<NarrativeEffect> efectos;
    // DIALOGOS-01 — La charla sigue: despues de esta opcion (y de lo que salte
    // solo), se vuelve a hablar con el mismo (el mismo trigger del beat), y el
    // primer beat que encaje ofrece lo que queda por hablar.
    bool sigue = false;
    // DIALOGOS-01 — Los papeles de las opciones de una pagina de charla:
    // pasaPagina = «Hablar de otra cosa» (va a la pagina siguiente);
    // despedida = la salida de la charla. Lo demas son temas.
    bool pasaPagina = false;
    bool despedida = false;
};

struct NarrativeBeat {
    std::string id;  // unico dentro de la aventura; solo para once/depuracion
    BeatTrigger trigger;
    NarrativeCondition requirements;

    // Lo que se muestra. speaker vacio = voz del narrador (un cierre, una
    // consecuencia): el HUD decide si dibuja marco de dialogo o no.
    std::string speaker;
    std::vector<std::string> lines;

    std::vector<NarrativeEffect> effects;

    // OPCIONES-01 — Lo que se puede contestar (vacío = nada: se lee y se
    // cierra). Como mucho kMaxOpciones: caben en la hoja y en las teclas 1-4.
    std::vector<OpcionDeDialogo> opciones;
    static constexpr std::size_t kMaxOpciones = 4;

    // once = el beat no vuelve a dispararse nunca, aunque sus
    // condiciones sigan cumpliendose. Sin esto, un beat Auto de cierre
    // se dispararia en cada tick() para siempre. La alternativa
    // "apagate a ti mismo con una flag" tambien vale y es lo que hace la
    // aventura de ejemplo, porque asi el estado queda EXPLICITO en las
    // flags (y por tanto en el guardado) en vez de escondido en la
    // contabilidad interna del motor.
    bool once = false;

    // DIALOGOS-01 — Una pagina de charla («entre tema y tema»): si al
    // dispararse no le queda ningun tema (solo pasar pagina y despedirse),
    // el motor pasa solo a la siguiente que tenga algo. Si no le queda nada a
    // ninguna, se queda en esta sin «Hablar de otra cosa».
    bool pagina = false;
};

}  // namespace RPG
