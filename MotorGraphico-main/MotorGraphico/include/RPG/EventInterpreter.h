// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-03  (fase E, 4 j) - Planificador, esperes i processos paral·lels
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Map/EventLayerDocument.h"

// E-03 — El intérprete de eventos: planificador, esperas y procesos
// paralelos.
//
// QUÉ HACE. Coge el árbol de comandos de una página de evento (E-01) y lo
// ejecuta A LO LARGO DE VARIOS FRAMES. Hasta aquí un beat se ejecutaba
// entero y de golpe: sin esperas no hay ritmo narrativo — ni una puerta
// que se abre despacio, ni un PNJ que camina mientras hablas.
//
// QUÉ NO HACE, Y ES LA LÍNEA MÁS IMPORTANTE DE ESTE FICHERO. **No sabe
// qué significa ningún comando.** "texto.mostrar", "mapa.ir" o
// "recompensa.entregar" son cadenas opacas para él. El juego de comandas
// es E-04, y quien lo implemente será el EventHost de más abajo. Esta
// separación es lo que permite escribir y probar el planificador entero
// —esperas, paralelismo, orden, límites— sin que exista todavía un solo
// comando de verdad, y lo que impide que el intérprete acabe siendo un
// switch gigante con toda la lógica del juego dentro.
//
// LO QUE SÍ DECIDE, PORQUE ORIOL LO RESPONDIÓ:
//
//  - DEC-30: cambiar de mapa TERMINA el evento local. Un comando puede
//    devolver EndEvent y lo que quede detrás no se ejecuta. La
//    continuación, si hace falta, se expresa como evento de entrada del
//    destino, no como una instrucción que sigue viva cruzando mapas.
//  - DEC-09: no se puede guardar en mitad de una acción importante.
//    canSave() dice si ahora mismo es un punto seguro, y es el intérprete
//    quien lo sabe porque es quien tiene las ejecuciones vivas.
//
// LO QUE NO DECIDE, PORQUE ORIOL NO LO HA RESPONDIDO:
//
//  - Qué página gana si se cumplen varias es DEC-10. Aquí se arranca una
//    página CONCRETA, elegida por quien llama. El intérprete no mira las
//    condiciones de las otras.
//  - Qué pasa al recargar un evento que se está ejecutando es DEC-11. No
//    hay recarga en caliente aquí; es F-03.

namespace RPG {

// Estados de una ejecución. Son los del diseño de bloques §8:
// Lista -> Ejecutando -> Esperando -> Ejecutando -> Terminada, más
// Cancelada y Fallida.
enum class EventRunState {
    Ready,      // creada, aún no ha dado un paso
    Running,    // avanzando comandos
    Waiting,    // parada esperando tiempo o al anfitrión
    Finished,   // llegó al final, o un comando la terminó (DEC-30)
    Cancelled,  // la paró alguien de fuera
    Failed      // un comando falló; se detuvo con diagnóstico
};

// Qué le ha pasado a un comando. Lo decide el anfitrión (E-04), no el
// intérprete.
enum class CommandOutcome {
    Done,      // hecho; seguir con el siguiente
    Wait,      // ha abierto algo (diálogo, combate, espera) y hay que esperar
    Branch,    // es un control: seguir por la rama que dice `branch`
    EndEvent,  // termina el evento local aquí mismo (DEC-30)
    Failed     // no se pudo; la ejecución se detiene con diagnóstico
};

struct CommandResult {
    CommandOutcome outcome = CommandOutcome::Done;

    // Con Wait: milisegundos que hay que esperar. CERO significa "no lo
    // sé, pregúntame": el intérprete llamará a waitResolved() cada tick
    // hasta que diga que sí. Es la diferencia entre "espera 400 ms" (lo
    // cuenta el reloj) y "espera a que el jugador cierre el diálogo" (lo
    // sabe el anfitrión y no tiene duración conocida).
    int waitMs = 0;

    // Con Branch: índice de la rama (children) por la que seguir. Fuera
    // de rango significa "ninguna rama", que es un final legítimo: un
    // `si` cuya condición es falsa y no tiene `si no`.
    int branch = -1;

    // Con Failed: qué pasó, para que el autor pueda leerlo. Un fallo sin
    // diagnóstico es un evento que deja de funcionar sin decir por qué.
    std::string diagnostic;
};

// Lo que el intérprete necesita del mundo. Lo implementa E-04.
class EventHost {
public:
    virtual ~EventHost() = default;

    // Ejecuta un comando. El intérprete no mira dentro: solo usa el
    // resultado para decidir si sigue, espera, se ramifica o para.
    virtual CommandResult execute(const MapEventCommand& command) = 0;

    // ¿Ha terminado ya lo que abrió un comando que devolvió Wait con
    // waitMs == 0? Se pregunta una vez por tick. Por defecto sí, para que
    // un anfitrión mínimo (o un doble de pruebas) no tenga que
    // implementarlo si no usa esperas abiertas.
    virtual bool waitResolved(const MapEventCommand& command) {
        (void)command;
        return true;
    }
};

// Una ejecución viva.
struct EventRun {
    // Identidad: qué evento y qué página. Dos instancias del mismo cofre
    // son dos eventos con id distinto, así que esto las distingue.
    std::string eventId;
    std::size_t pageIndex = 0;
    // Los eventos paralelos no bloquean al jugador; los demás sí. Se
    // copia del disparador de la página al arrancar.
    bool parallel = false;
    EventRunState state = EventRunState::Ready;
    std::string diagnostic;
};

// E-08 — Cómo avanza el intérprete.
//
// En Continuous avanza solo, que es como corre el juego. En Paused no
// avanza nadie salvo que se le pida un paso: es el modo del depurador del
// editor. La pausa es GLOBAL y no por ejecución a propósito — el autor
// quiere congelar la escena entera para mirarla, no una hebra de ella.
enum class RunMode { Continuous, Paused };

// Un comando ya ejecutado, en orden temporal. Es el «log con orden
// temporal» que pide la especificación §15.3: sin él, depurar un evento
// que ramifica es adivinar por dónde pasó.
struct TraceEntry {
    std::string eventId;
    std::size_t pageIndex = 0;
    std::vector<std::size_t> commandPath;
    std::string commandType;
};

// Dónde parar. Es la misma coordenada que usa el diagnóstico de E-07, a
// propósito: así «Ir ahí» desde la lista de problemas y «poner un punto
// de parada» hablan el mismo idioma.
struct Breakpoint {
    std::string eventId;
    std::size_t pageIndex = 0;
    std::vector<std::size_t> commandPath;

    friend bool operator==(const Breakpoint& a, const Breakpoint& b) {
        return a.eventId == b.eventId && a.pageIndex == b.pageIndex &&
               a.commandPath == b.commandPath;
    }
};

class EventInterpreter {
public:
    // Presupuesto por tick: cuántos comandos como mucho avanza CADA
    // ejecución antes de ceder. Sin un tope, una secuencia larga sin
    // esperas congela el frame; con él, se reparte. El diseño de bloques
    // (§8) pide que el límite exista y se mida, no que se invente: 64 es
    // el valor de partida.
    static constexpr std::size_t kCommandsPerTick = 64;

    explicit EventInterpreter(EventHost& host) : m_host(host) {}

    // Arranca una página concreta de un evento.
    //
    // Falla si ese evento YA tiene una ejecución viva: no se reentra en
    // el mismo evento mientras está activo (diseño de bloques §8). Una
    // segunda interacción no arranca otra copia — quien llama decide si
    // eso se le dice al jugador ("está ocupado") o se ignora.
    Result<bool> start(const MapEventDocument& event, std::size_t pageIndex);

    // Avanza el tiempo. deltaMs es tiempo de SECUENCIA: quien llama
    // decide qué reloj lo alimenta y si la pausa lo congela (§8 pide
    // relojes separados; cuál es cuál no lo decide el intérprete).
    void tick(int deltaMs);

    // -----------------------------------------------------------------
    // E-08 — Depuración paso a paso
    // -----------------------------------------------------------------

    // Al volver a Continuous se pasa por encima del punto en el que se
    // paró, una vez: si no, el intérprete se volvería a parar en el mismo
    // sitio en el tick siguiente y «continuar» no continuaría.
    void setMode(RunMode mode);
    RunMode mode() const { return m_mode; }

    // Avanza UN comando de UNA ejecución, aunque esté en pausa. Devuelve
    // false si no se pudo avanzar, y entonces `stepBlockedReason()` dice
    // por qué.
    //
    // UNA ESPERA NO SE SALTA EN SILENCIO. La especificación §15.3 lo pide
    // con esas palabras. Aquí se resuelve así: dar un paso sobre una
    // ejecución que está esperando SÍ la atraviesa —si no, el depurador
    // se quedaría trabado para siempre, porque en pausa el tiempo no
    // corre— pero lo ANOTA en lastStepNote(), y el panel lo enseña. El
    // autor tiene que poder distinguir un recorrido que el juego hace de
    // uno que solo hace el depurador.
    bool stepOnce();
    // Por qué el último paso no pudo darse. Vacío si se dio.
    const std::string& stepBlockedReason() const { return m_stepBlocked; }
    // Qué tuvo de particular el último paso (por ejemplo, que se saltó
    // una espera). Vacío si fue un paso normal.
    const std::string& lastStepNote() const { return m_stepNote; }

    // Puntos de parada. Al alcanzar uno, el intérprete pasa a Paused ANTES
    // de ejecutar ese comando: el autor quiere ver el estado con el que el
    // comando va a correr, no con el que dejó.
    void addBreakpoint(Breakpoint breakpoint);
    bool removeBreakpoint(const Breakpoint& breakpoint);
    void clearBreakpoints();
    const std::vector<Breakpoint>& breakpoints() const { return m_breakpoints; }
    // El punto que acaba de parar la ejecución, o nullptr. Se limpia al
    // reanudar.
    const Breakpoint* lastHit() const { return m_hit ? &m_lastHit : nullptr; }

    // Qué comando va a ejecutarse ahora en esa ejecución, o nullptr si no
    // hay ninguna viva o ya no queda comando. Es lo que el panel de
    // ejecución enseña como «comando actual».
    const MapEventCommand* nextCommand(const std::string& eventId) const;
    // Y su coordenada, para poder resaltarlo en el editor.
    std::vector<std::size_t> nextCommandPath(const std::string& eventId) const;

    const std::vector<TraceEntry>& trace() const { return m_trace; }
    void clearTrace() { m_trace.clear(); }

    // Para una ejecución. Lo que ya hizo, hecho está: cancelar no
    // deshace, solo impide que siga.
    bool cancel(const std::string& eventId);
    void cancelAll();

    bool isRunning(const std::string& eventId) const;
    // Ejecuciones vivas (Ready/Running/Waiting), en orden de arranque.
    std::vector<EventRun> active() const;
    // Todas, incluidas las terminadas de este tick, para que quien llama
    // pueda leer el desenlace y el diagnóstico antes de que se limpien.
    const std::vector<EventRun>& runs() const { return m_runs; }
    // Quita las terminadas/canceladas/fallidas. Se llama cuando quien
    // llama ya ha leído sus desenlaces.
    void collectFinished();

    // ¿Bloquea alguna ejecución al jugador? Un evento paralelo no cuenta:
    // esa es toda la diferencia entre paralelo y automático.
    bool blocksPlayer() const;

    // DEC-09 — ¿Es ahora mismo un punto seguro para guardar? Lo es cuando
    // no queda ninguna ejecución viva. Guardar con un evento a medias
    // exigiría serializar el programa, el punto de continuación, los
    // ámbitos y el estado de espera; el piloto no lo hace, y decir que sí
    // aquí sería prometerlo.
    bool canSave() const { return active().empty(); }

private:
    // Un marco del recorrido: qué lista de comandos y por dónde va.
    struct Frame {
        const std::vector<MapEventCommand>* commands = nullptr;
        std::size_t index = 0;
    };

    struct Execution {
        EventRun run;
        MapEventDocument event;   // copia: el documento puede cambiar bajo los pies
        std::vector<Frame> stack;
        int waitRemainingMs = 0;
        bool waitingOnHost = false;
        const MapEventCommand* waitCommand = nullptr;
        // E-08 — Al reanudar desde un punto de parada hay que pasar por
        // encima de ESE punto una vez; si no, se volvería a parar en el
        // mismo sitio y no se avanzaría nunca. Es por ejecución y no
        // global porque dos ejecuciones pueden estar paradas a la vez.
        bool skipBreakpointOnce = false;
    };

    // Ruta del comando que la ejecución tiene apuntado ahora mismo. Se
    // deriva de la pila de marcos: cada marco aporta su índice.
    static std::vector<std::size_t> RutaActual(const Execution& execution);
    Execution* find(const std::string& eventId);
    const Execution* find(const std::string& eventId) const;
    // ¿Hay un punto de parada en lo que esa ejecución va a ejecutar?
    const Breakpoint* breakpointAt(const Execution& execution) const;

    void stepOne(Execution& execution);
    void applyResult(Execution& execution, const MapEventCommand& command,
                     const CommandResult& result);
    void finish(Execution& execution, EventRunState state, std::string diagnostic = {});

    EventHost& m_host;
    RunMode m_mode = RunMode::Continuous;
    std::vector<Breakpoint> m_breakpoints;
    Breakpoint m_lastHit;
    bool m_hit = false;
    std::string m_stepBlocked;
    std::string m_stepNote;
    std::vector<TraceEntry> m_trace;
    // unique_ptr y no Execution directamente: los marcos del recorrido
    // guardan punteros a listas de comandos que viven DENTRO de la copia
    // del evento de cada ejecucion. Con un vector<Execution>, anadir una
    // ejecucion mueve las demas, y aunque mover un std::vector conserve
    // las direcciones de sus elementos —asi que hoy funcionaria— eso es
    // una sutileza que el siguiente que toque esto no tiene por que
    // conocer. Con punteros, la ejecucion no se mueve nunca y el
    // problema no existe.
    std::vector<std::unique_ptr<Execution>> m_executions;
    // Espejo público, reconstruido en cada cambio de estado: runs()
    // devuelve una referencia y no puede exponer Execution, que lleva
    // punteros a comandos vivos.
    std::vector<EventRun> m_runs;
};

}  // namespace RPG
