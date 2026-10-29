#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Map/MapDocument.h"
#include "RPG/EventCommand.h"
#include "RPG/EventInterpreter.h"

// H0-08 (Fase H) — La capa de DISPARADORES: del mapa y del jugador al
// intérprete.
//
// QUÉ FALTABA. La fase E dejó la cadena completa menos el primer eslabón.
// E-01 guarda eventos en casillas del mapa, E-03 los ejecuta a lo largo de
// varios frames, E-04 dice qué significa cada comando y E-09 elige qué
// página está activa. Pero `EventInterpreter::start()` recibe UN evento y
// UN índice de página: alguien tiene que decidir cuáles, y ese alguien no
// existía. `examples/demo_mision_llave.cpp:236` lo hace a mano y POR ID
// (`interactuar(mapa, "puerta", ...)`), que es justo lo que un jugador no
// puede hacer: el jugador está en una casilla, mira a otra y pulsa acción.
// Sin esta pieza el intérprete no arranca nunca desde la partida, y con él
// no llegan 49 de las 79 jornadas de la fase E.
//
// QUÉ HACE. Recibe cada frame dónde está el jugador y si ha pulsado acción,
// y hace tres cosas: resuelve qué eventos disparan (Touch al entrar en la
// casilla, Autorun y Parallel solos, Interact al pulsar), elige la página
// con `ActivePageWithInventory` (E-09, la única regla de selección que
// existe) y arranca la ejecución; y pasa el delta al intérprete.
//
// QUÉ NO HACE, Y POR QUÉ.
//
//  - No sabe qué significa ningún comando. Eso es E-04 y vive en el
//    `EventHost` que quien llama le pasó al intérprete: esta pieza recibe
//    el intérprete ya construido con su anfitrión y no construye ninguno.
//    Sí usa `ActivePageWithInventory`, que vive en `RPG/SessionCommandHost.h`
//    porque es la única regla de selección de página que sabe mirar el
//    inventario; reimplementarla aquí sería tener dos reglas donde el
//    diseño pide una.
//  - No conoce `Player`, ni `Application`, ni GL. Lo que necesita del mundo
//    cabe en `MapEventFrame`, tres campos. Quien lleva el bucle los rellena
//    con lo que ya tiene (`GameSession::playerPosition()`, el input).
//  - No posee el mapa, ni el estado narrativo, ni el inventario, ni el
//    intérprete: los referencia. Sus dueños viven más arriba y cambian de
//    nivel; este objeto se reconstruye o se le da otro mapa, no hereda
//    nada.
//  - No ejecuta el cambio de mapa. Un `mapa.ir` deja la transición en
//    `SessionCommandHost::requestedTransition()`, y es quien orquesta
//    (Application) quien la lee y carga el nivel: la misma división que ya
//    usan `GameSession` y `BattleState` —la sesión decide, el de fuera
//    orquesta—. Esta pieza no añade un segundo camino.
//
// LO QUE DECIDE, Y QUÉ DECISIÓN HAY DETRÁS.
//
//  - Touch dispara al ENTRAR en la casilla, no mientras se está en ella.
//    Sin esto, cada frame sobre una placa de presión volvería a arrancar el
//    evento. Salir y volver a entrar sí lo vuelve a disparar: es lo que
//    hace que una placa funcione dos veces.
//  - Autorun y Parallel disparan UNA VEZ POR PÁGINA ACTIVA DISTINTA.
//    PROVISIONAL, Y PENDIENTE DE DEC-10 igual que la regla de selección de
//    página: Oriol no ha respondido qué página gana ni cuándo se repite un
//    automático. La alternativa literal de RPG Maker —disparar siempre que
//    las condiciones se cumplan y no haya ejecución viva— convierte un
//    Autorun cuya página no cambia en un bucle que se rearma cada frame y
//    congela la partida. Entre un bucle infinito y un automático que no se
//    repite solo, se elige lo segundo: cambiarlo es cambiar una comparación
//    en `dispararAutomaticos()` y nada más.
//  - Mientras algo bloquee al jugador no arranca NADA nuevo. Una escena no
//    se interrumpe con otra encima. Los Parallel no bloquean —esa es toda
//    la diferencia entre paralelo y automático— y por tanto no inhiben.
//  - Interact prueba la casilla del jugador y luego las cuatro contiguas,
//    EN EL MISMO ORDEN que `GameSession::interact()`
//    (`src/Game/GameSessionInteraccion.cpp:91-92`): primero los pies,
//    porque una puerta se pisa y a un PNJ se le habla de frente. El orden
//    está duplicado a propósito y no extraído: unificarlo exige abrir
//    `GameSession.cpp`, que comparten seis paquetes y hoy está sucio.
//  - Una página cuya condición no se puede evaluar NO cuenta como falsa
//    (`ActivePageResult::trustworthy()`): se anota en `diagnostic()` y no
//    se arranca. Es la misma regla que E-04/E-09, y es la diferencia entre
//    «no tiene la llave» y «no sé mirar el inventario desde aquí».

namespace RPG {

// Lo que el runner necesita del mundo en un frame. Datos, no referencias:
// quien lleva el bucle lo rellena con lo que ya tiene.
struct MapEventFrame {
    // La casilla del jugador. En el juego sale de
    // `GameSession::playerPosition()`.
    GridCoord playerCell;
    // Pulsado ACCIÓN en este frame. Es un FLANCO, no un nivel: mantener la
    // tecla no debe volver a hablar con el PNJ en cada frame. Quien lee el
    // input ya distingue las dos cosas (`InputAction` frente a tecla
    // repetida) y es él quien debe pasar true una sola vez.
    bool interactPressed = false;
    // Tiempo de SECUENCIA de este frame, en milisegundos. El intérprete no
    // decide qué reloj lo alimenta (E-03, §8 pide relojes separados): quien
    // llama pasa 0 con la partida en pausa y el delta real si no.
    int deltaMs = 0;
};

class MapEventRunner {
public:
    // Las cuatro referencias viven más que el runner: las pone quien lleva
    // la partida. `inventory` es el mismo vector que se le pasó a la
    // sesión, igual que hace `SessionCommandHost` — así la condición «lleva
    // la llave» mira el inventario de verdad y no una copia.
    MapEventRunner(NarrativeState& narrative, const std::vector<std::string>& inventory,
                   EventInterpreter& interpreter);

    // El mapa cuyos eventos se disparan. nullptr = no hay mapa cargado y
    // por tanto nada que disparar; el intérprete sigue avanzando lo que ya
    // tuviera vivo. Se cambia al cargar otro nivel.
    void setMap(const MapDocument* map) { m_map = map; }
    const MapDocument* map() const { return m_map; }

    // Un frame: arranca lo que los disparadores pidan y avanza lo vivo.
    void update(const MapEventFrame& frame);

    // ¿Bloquea alguna ejecución al jugador? Es `EventInterpreter::
    // blocksPlayer()`, expuesto aquí porque es quien llama el que tiene que
    // decidir no mover al jugador, y no debe tener que conocer el
    // intérprete para averiguarlo.
    bool blocksPlayer() const;

    // Qué arrancó en el último update(), y por qué no arrancó lo que se
    // miró. Vacíos si no pasó nada: el HUD y el panel de problemas del
    // editor leen esto, y un disparador que falla en silencio es un evento
    // que deja de funcionar sin decir por qué.
    const std::string& lastStartedEvent() const { return m_lastStarted; }
    const std::string& diagnostic() const { return m_diagnostic; }

    // Olvida qué se disparó ya (celdas visitadas y páginas automáticas
    // arrancadas). Se llama al cargar un nivel: los automáticos de un mapa
    // nuevo tienen que poder sonar aunque el anterior tuviera uno con el
    // mismo id. No toca el intérprete — cancelar lo vivo es de quien lo
    // posee.
    void resetTriggerState();

    // Casillas que se prueban al pulsar acción, en orden: la del jugador y
    // las cuatro contiguas. Público porque el editor quiere dibujarlas
    // («esta casilla está a tiro de interacción») y porque es el mismo
    // orden que usa `GameSession::interact()`.
    static std::vector<GridCoord> interactCandidates(GridCoord playerCell);

private:
    // Primer evento de la primera capa de eventos que ocupa esa casilla, o
    // nullptr. Una casilla, un evento: lo garantiza `EventLayerDocument`.
    const MapEventDocument* eventAt(GridCoord cell) const;

    // Página activa de un evento según la única regla de selección que
    // existe (E-09, `ActivePageWithInventory`). Devuelve false cuando no hay
    // página activa —legítimo y silencioso— o cuando no se pudo evaluar, y
    // entonces deja el motivo en `m_diagnostic`. No es const porque escribe
    // ese diagnóstico.
    bool paginaActiva(const MapEventDocument& event, const MapEventPage** pagina,
                      std::size_t* index);
    bool startPage(const MapEventDocument& event, std::size_t pageIndex);

    // El registro de «una vez por página activa distinta» de los automáticos.
    // Devuelve y guarda índice + 1, con 0 = ninguna.
    std::size_t paginaAutomaticaYaArrancada(const std::string& eventId) const;
    void registrarAutomatica(const std::string& eventId, std::size_t pageIndex);

    void dispararAutomaticos();
    void dispararTouch(const MapEventFrame& frame);
    void dispararInteract(const MapEventFrame& frame);

    NarrativeState& m_narrative;
    const std::vector<std::string>& m_inventory;
    EventInterpreter& m_interpreter;
    const MapDocument* m_map = nullptr;

    // Dónde estaba el jugador el frame anterior, y si lo sabemos. Sin la
    // bandera, el primer frame en (0,0) se confundiría con «no se ha movido
    // desde (0,0)» y un Touch puesto en el origen no dispararía nunca.
    GridCoord m_lastCell{0, 0};
    bool m_hasLastCell = false;

    // Página automática ya arrancada por evento: el registro que impide que
    // un Autorun se rearme cada frame. Índice de página + 1, con 0 =
    // «ninguna», para que la página 0 se distinga de la ausencia de
    // entrada sin necesitar un map de opcionales.
    std::vector<std::pair<std::string, std::size_t>> m_autoStarted;

    std::string m_lastStarted;
    std::string m_diagnostic;
};

}  // namespace RPG
