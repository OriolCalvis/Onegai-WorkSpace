// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-04  (fase E, 7 j) - El joc de comandes
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Map/EventLayerDocument.h"
#include "RPG/EventInterpreter.h"
#include "RPG/NarrativeEngine.h"

// E-04 — El juego de comandas.
//
// QUÉ ES ESTE FICHERO. La respuesta a «¿qué comandos existen y qué recibe
// cada uno?», en un solo sitio. Hasta aquí esa pregunta no tenía dueño:
// E-01 guarda comandos sin saber qué son (a propósito, para que uno de un
// tipo desconocido no pierda datos) y E-03 los ejecuta sin mirar dentro
// (también a propósito, para que el planificador no sea un switch gigante
// con el juego dentro). Alguien tenía que decir qué significan, y es este
// paquete.
//
// QUIÉN LO CONSUME, Y POR QUÉ IMPORTA QUE SEA UNO SOLO:
//   - E-05/E-06, el editor: de aquí salen los formularios. Un comando con
//     sus parámetros declarados es un formulario que se dibuja solo.
//   - E-07, la validación estática: comprueba contra esto antes de
//     guardar y antes de probar.
//   - E-03, al ejecutar: NarrativeCommandHost de más abajo.
// Si cada uno llevara su propia lista, el editor ofrecería comandos que
// el intérprete no sabe ejecutar — que es exactamente el fallo que la
// especificación §12 pide evitar: «los disparadores presentan solo los
// soportados».
//
// DECLARADO NO ES IMPLEMENTADO. Cada ficha lleva `implemented`. Las
// familias que todavía no están cableadas aparecen en el catálogo —el
// editor necesita saber que existen— pero se marcan, y ejecutarlas falla
// con un diagnóstico que lo dice. Ofrecer al autor un comando que no hace
// nada es peor que no ofrecerlo.

namespace RPG {

// Tipos de parámetro. Salen del diseño de bloques §5 («Tipos y encajes»).
// El tipo es lo que permite que el editor dibuje el control adecuado —una
// casilla, un número, un selector de objeto— en vez de una caja de texto
// para todo, y lo que permite a E-07 decir «esto no es una duración».
enum class CommandParamType {
    Bool,
    Int,
    Text,
    DurationMs,    // entero, en milisegundos, no negativo
    FlagRef,       // nombre de una bandera narrativa
    VariableRef,   // nombre de una variable numérica
    SwitchLetter,  // A-D, el interruptor local de un evento
    ObjectRef,     // id del catálogo de objetos
    MapRef,        // id de un mapa
    EventRef,      // id de un evento
    Comparison     // "==", "!=", ">=", "<=", ">", "<"
};

struct CommandParamSpec {
    std::string key;
    CommandParamType type = CommandParamType::Text;
    bool required = true;
    // Lo que el editor enseña al autor. No es el nombre de la clave: la
    // clave es estable y este texto es traducible (G-17).
    std::string help;
};

struct CommandSpec {
    std::string type;    // id estable; NO es lo que ve el autor
    std::string label;   // nombre visible, traducible
    std::string help;
    std::vector<CommandParamSpec> params;
    // Cuántas ramas admite. 0 = no es un control. Las ramas son los
    // `children` del comando (E-01).
    std::size_t branches = 0;
    // Si es false, el comando está declarado pero NO cableado. El editor
    // debe enseñarlo deshabilitado, no ofrecerlo como si funcionara.
    bool implemented = false;
};

// El catálogo entero, en orden estable.
const std::vector<CommandSpec>& CommandCatalog();
// nullptr si ese tipo no está en el catálogo. Un tipo desconocido NO es
// un error aquí: E-01 los conserva a propósito para no perder datos al
// abrir un proyecto hecho con una versión más nueva.
const CommandSpec* FindCommand(const std::string& type);

// Comprueba un comando y, recursivamente, sus ramas: que el tipo exista,
// que estén los parámetros obligatorios y que cada valor case con su
// tipo. Es lo que E-07 llamará antes de guardar y antes de probar.
//
// Un tipo desconocido se reporta como tal para que el editor lo enseñe
// como «bloque no disponible», que es lo que pide el diseño §9.
Result<bool> ValidateCommand(const MapEventCommand& command);

// Evalúa una condición de página (E-01) contra el estado narrativo.
//
// Devuelve Error —y no falso— cuando la condición no se PUEDE evaluar:
// «no tiene la llave» y «no sé mirar el inventario desde aquí» son cosas
// distintas, y confundirlas convierte una pieza que falta en una puerta
// que nunca se abre y que nadie sabe por qué.
Result<bool> EvaluateCondition(const MapEventCondition& condition,
                               const NarrativeState& state);

// La condición que describe un `control.si`. La usan los dos anfitriones
// —el narrativo y el de partida— para no tener dos lecturas distintas de
// los mismos parámetros: si una entendiera «negada» y la otra no, el
// mismo comando ramificaría distinto según quién lo ejecute.
Result<MapEventCondition> ConditionFromIf(const MapEventCommand& command);

// Evalúa todas las condiciones de una página con su AND/OR.
Result<bool> EvaluatePage(const MapEventPage& page, const NarrativeState& state);

// La misma evaluación, pero con un evaluador de condición cualquiera.
// Existe para que la regla del AND/OR viva en UN sitio: quien sabe mirar
// el inventario (SessionCommandHost) le pasa su evaluador y no tiene que
// reimplementar la combinación.
Result<bool> EvaluatePageWith(
    const MapEventPage& page,
    const std::function<Result<bool>(const MapEventCondition&)>& evaluate);

// E-09 — QUÉ PÁGINA DE UN EVENTO ESTÁ ACTIVA.
//
// Gana la ÚLTIMA página cuyas condiciones se cumplen. Las páginas se leen
// de arriba abajo como estados sucesivos —cofre cerrado, cofre abierto— y
// el autor añade el estado nuevo debajo. Si ganara la primera, añadir una
// página no cambiaría nada y el cofre no se abriría nunca.
//
// PROVISIONAL, PENDIENTE DE DEC-10. Es la regla que el propio DEC-10
// propone como borrador («gana la última página cuyas condiciones se
// cumplan»), no una decisión tomada: Oriol contestó que no entendía la
// pregunta y la ficha sigue abierta. Está en UNA función a propósito, de
// modo que cambiarla sea cambiar este bucle y nada más.
//
// La otra mitad del borrador de DEC-10 —«lo que ya está en pantalla
// termina; el cambio se nota la siguiente vez que se hable»— ya es cierta
// sin hacer nada: el intérprete de E-03 no vuelve a mirar las condiciones
// de una página que ya arrancó. Elegir página es algo que pasa ANTES de
// start(), no durante.
//
// UNA CONDICIÓN QUE NO SE PUEDE EVALUAR NO CUENTA COMO FALSA. Si una
// página pide «lleva la llave» y aquí no hay inventario que mirar, la
// respuesta correcta no es «no la lleva»: es «no lo sé». Tratarlo como
// falso convierte una pieza que falta en una puerta que nunca se abre y
// que nadie sabe por qué. Por eso el resultado lleva la lista de páginas
// que no se pudieron mirar: si no está vacía, la elección NO es de fiar.
struct ActivePageResult {
    // nullptr = ninguna página se cumple. El evento existe y no hace nada,
    // que es legítimo: un cofre ya vaciado puede no tener página activa.
    const MapEventPage* page = nullptr;
    std::size_t index = 0;
    // Índices de las páginas cuya condición dio error.
    std::vector<std::size_t> unevaluable;
    // El primero de esos errores, tal cual, para enseñarlo.
    std::string diagnostic;

    bool trustworthy() const { return unevaluable.empty(); }
};

ActivePageResult ActivePageWith(
    const MapEventDocument& event,
    const std::function<Result<bool>(const MapEventCondition&)>& evaluate);

// Atajo para el caso sin inventario.
ActivePageResult ActivePage(const MapEventDocument& event, const NarrativeState& state);

// Anfitrión de E-03 para las familias que solo tocan el estado
// narrativo: banderas, variables, interruptores locales, condicional y
// espera. Es la mitad del juego de comandas que no necesita sesión de
// partida, y por eso se puede entregar y probar entera ahora.
//
// Las familias que SÍ necesitan sesión —inventario, oro, combate,
// comercio, cambio de mapa— se resuelven delegando en `fallback` si se
// le da uno. Sin fallback, fallan con un diagnóstico que dice qué falta.
// Ese es el punto de costura por el que entrará la segunda rebanada sin
// tocar nada de esto.
class NarrativeCommandHost : public EventHost {
public:
    explicit NarrativeCommandHost(NarrativeState& state, EventHost* fallback = nullptr)
        : m_state(state), m_fallback(fallback) {}

    CommandResult execute(const MapEventCommand& command) override;
    bool waitResolved(const MapEventCommand& command) override;

    // Textos emitidos por `texto.mostrar`, en orden. Sin servicio de
    // diálogo todavía (G-14), el texto se recoge aquí para que el
    // recorrido se pueda probar y para el log de depuración de F-04.
    const std::vector<std::string>& log() const { return m_log; }

private:
    NarrativeState& m_state;
    EventHost* m_fallback = nullptr;
    std::vector<std::string> m_log;
};

}  // namespace RPG
