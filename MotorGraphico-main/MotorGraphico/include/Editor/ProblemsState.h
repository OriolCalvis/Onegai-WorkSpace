// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-15  (fase E, 3 j) - Problemes i cerca d'esdeveniments
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Map/MapDocument.h"
#include "RPG/AdventureValidation.h"

// E-15 — Problemas y búsqueda de eventos. MITAD SIN INTERFAZ.
//
// QUÉ RESUELVE. E-07 ya sabe encontrar los defectos de una misión, pero
// los devuelve como una lista de textos. Un mensaje en consola no cierra
// el recorrido del autor: hace falta una lista con severidad, sitio y una
// acción «Ir ahí» que lleve al comando exacto. Esto es esa lista; el
// panel que la dibuja (editor/src/ProblemsPanel.cpp) es de E-06 y no está
// aquí a propósito — ver la nota de ubicación al final.
//
// EL PROBLEMA DE VERDAD ES QUE EL DOCUMENTO CAMBIA. La lista se calcula
// en un momento y se usa en otro. Entre medias el autor borra una página,
// mueve un comando o quita el evento entero. Si «Ir ahí» guarda «página 3
// del evento cofre» y el autor borra la página 1, el índice 3 ya es otra
// página: el editor seleccionaría algo distinto de lo que el diagnóstico
// señalaba, sin avisar. El criterio de cierre de E-15 lo dice con esas
// palabras: «una referencia borrada no provoca navegación a una entidad
// diferente».
//
// POR ESO SE GUARDA LA IDENTIDAD, NO LA POSICIÓN. Cada sitio lleva el id
// del evento, el id de la página (las páginas tienen id propio desde
// E-01) y el tipo del comando. Navegar vuelve a resolver esos tres contra
// el documento de HOY y devuelve los índices actuales. Si algo ya no
// está, se dice y no se navega a lo que ha ocupado su hueco. El índice de
// captura se conserva sólo para enseñarlo en la lista.
//
// EL FOCO SOBREVIVE AL REFRESCO. Corregir un defecto revalida y rehace la
// lista entera. Si el foco fuera un número de fila, el autor acabaría
// mirando otro problema cada vez que arregla uno. Se guarda la clave del
// problema enfocado y se vuelve a buscar; si ya no está —porque se acaba
// de arreglar— el foco pasa al siguiente de la lista, que es lo que el
// autor va a querer mirar.

namespace Editor {

// ---------------------------------------------------------------------
// Dónde
// ---------------------------------------------------------------------

// Sitio de un problema o de un hallazgo de búsqueda, con la información
// necesaria para volver a encontrarlo cuando el documento haya cambiado.
struct ProblemLocation {
    // Mapa. La lista es de todo el proyecto, no de un mapa: un evento
    // roto en otro mapa también bloquea el playtest.
    std::string mapId;
    std::string eventId;

    // Página. `pageId` es lo estable; `pageIndex` es dónde estaba cuando
    // se capturó, y sirve para enseñarlo («página 3») y nada más.
    bool hasPage = false;
    std::string pageId;
    std::size_t pageIndex = 0;

    // Comando. La ruta son índices ({2, 0, 1} = segundo comando de la
    // primera rama del tercer comando). Los comandos no tienen id, así
    // que la ruta es lo único que hay; `commandType` es el seguro: si la
    // ruta resuelve a un comando de otro tipo, es que el autor ha movido
    // cosas y no se navega.
    bool hasCommand = false;
    std::vector<std::size_t> commandPath;
    std::string commandType;

    // Identidad estable del sitio. No incluye índices.
    std::string key() const;
};

// Una línea de la lista de problemas.
struct ProblemEntry {
    RPG::DiagnosticSeverity severity = RPG::DiagnosticSeverity::Error;
    std::string message;
    ProblemLocation where;

    bool isError() const { return severity == RPG::DiagnosticSeverity::Error; }
    // Identidad de la línea. Lleva el mensaje además del sitio porque un
    // mismo comando puede tener dos defectos a la vez (falta un
    // parámetro Y el tipo no existe) y el foco tiene que distinguirlos.
    std::string key() const;
};

// ---------------------------------------------------------------------
// Ir ahí
// ---------------------------------------------------------------------

enum class NavigationOutcome {
    // Todo resuelto: el destino es exactamente lo que el diagnóstico
    // señalaba.
    Ok,
    // No hay nada enfocado.
    NoTarget,
    // El mapa ya no está en el proyecto.
    MapGone,
    // El evento ya no existe. No se navega: no hay a dónde.
    EventGone,
    // El evento sí, la página no. Se abre el evento y se dice que la
    // página ya no está, en vez de seleccionar la que haya caído en su
    // índice.
    PageGone,
    // Evento y página sí; el comando ya no está donde estaba, o el que
    // hay ahí es otro. Se abre la página sin seleccionar comando.
    CommandGone
};

struct NavigationRequest {
    NavigationOutcome outcome = NavigationOutcome::NoTarget;
    // Destino resuelto contra el documento de hoy: los índices de aquí
    // son los buenos, no los de captura. Sólo se rellena hasta donde se
    // ha podido resolver.
    ProblemLocation target;
    // Para la barra de estado. Vacío cuando outcome == Ok.
    std::string note;

    bool ok() const { return outcome == NavigationOutcome::Ok; }
    // ¿Hay algo que abrir, aunque sea parcial? PageGone y CommandGone
    // sí: el evento existe y llevar ahí al autor le sirve.
    bool navigable() const {
        return outcome == NavigationOutcome::Ok ||
               outcome == NavigationOutcome::PageGone ||
               outcome == NavigationOutcome::CommandGone;
    }
};

// Resuelve un sitio contra un documento. Función libre y sin estado: el
// panel la necesita para pintar en gris lo que ya no existe, sin tener
// que pedirle permiso a nadie.
NavigationRequest ResolveNavigation(const MapDocument& document,
                                    const ProblemLocation& where);

// ---------------------------------------------------------------------
// La lista
// ---------------------------------------------------------------------

class ProblemsState {
public:
    ProblemsState() = default;

    // Valida el documento y mete sus hallazgos en la lista, sustituyendo
    // los que hubiera de ESE mapa y dejando los de los demás. Es lo que
    // hace el editor al guardar o al terminar de editar un mapa: no
    // conviene borrar los problemas de los otros mapas sólo porque el
    // autor ha tocado éste.
    void refreshMap(const MapDocument& document);

    // Igual, pero con un informe ya calculado. Sirve para las aventuras
    // de beats (ValidateAdventure), que no tienen documento de mapa.
    void setReport(const std::string& mapId, const RPG::ValidationReport& report,
                   const MapDocument* document);

    void removeMap(const std::string& mapId);
    void clear();

    // Todos los problemas, en orden estable: errores antes que avisos y,
    // dentro de cada grupo, por mapa, evento, página y comando. El orden
    // no depende del orden en que se hayan validado los mapas.
    const std::vector<ProblemEntry>& problems() const { return m_problems; }

    std::size_t errorCount() const;
    std::size_t warningCount() const;
    // La puerta de E-07, extendida a todo el proyecto: se puede guardar
    // con avisos, no se puede probar ni exportar con errores.
    bool canPlaytest() const { return errorCount() == 0; }

    // --- Foco ---
    bool hasFocus() const { return m_focusIndex < m_problems.size(); }
    std::size_t focusIndex() const { return m_focusIndex; }
    const ProblemEntry* focused() const;
    // Enfoca por clave. Devuelve false y no toca el foco si esa clave no
    // está en la lista.
    bool focusOn(const std::string& key);
    bool focusIndexAt(std::size_t index);
    void focusNext();
    void focusPrevious();
    void clearFocus();
    // Qué le pasó al foco en el último refresco. Es lo que el panel
    // enseña cuando el problema que el autor estaba mirando desaparece.
    enum class FocusChange { Unchanged, Kept, MovedToNext, Lost, None };
    FocusChange lastFocusChange() const { return m_lastFocusChange; }

    // Ir ahí, sobre el problema enfocado.
    NavigationRequest navigate(const MapDocument& document) const;

private:
    void rebuildOrder();
    void restoreFocus(const std::string& previousKey, bool hadFocus);

    std::vector<ProblemEntry> m_problems;
    std::size_t m_focusIndex = static_cast<std::size_t>(-1);
    FocusChange m_lastFocusChange = FocusChange::None;
};

// ---------------------------------------------------------------------
// Búsqueda
// ---------------------------------------------------------------------

// Qué se está buscando. Los nombres son los del vocabulario del autor,
// no los de la implementación: para él una bandera y una variable son
// dos cosas distintas aunque las dos sean texto.
enum class ReferenceKind {
    Flag,         // bandera narrativa
    Variable,     // variable numérica con nombre
    LocalSwitch,  // interruptor local A-D de un evento
    Event,        // otro evento, por id
    Map,          // otro mapa, por id
    Object        // objeto del catálogo
};

struct SearchHit {
    ProblemLocation where;
    // Para enseñar la línea sin volver a buscar el evento.
    std::string eventName;
    // Qué ha casado: el nombre del evento, la condición de la página o
    // el parámetro concreto del comando.
    std::string detail;
};

// Busca eventos por nombre. Sin distinguir mayúsculas NI acentos: DEC-15
// dice que el texto visible lleva ortografía correcta, así que los
// eventos se van a llamar «Misión de la llave» y el autor va a escribir
// «mision» en la caja de búsqueda. Una búsqueda que no encontrara eso
// sería una búsqueda inútil.
std::vector<SearchHit> SearchEventsByName(const MapDocument& document,
                                          const std::string& query);

// Dónde se usa una bandera, una variable, un interruptor local, un
// evento, un mapa o un objeto. Mira las condiciones de cada página y
// todos los parámetros de todos los comandos.
//
// No lleva una lista de comandos escrita a mano: pregunta al catálogo de
// E-04 de qué tipo es cada parámetro (FlagRef, VariableRef...). Así, un
// comando nuevo aparece en la búsqueda el día que se añade al catálogo,
// sin tocar este fichero.
std::vector<SearchHit> FindReferences(const MapDocument& document,
                                      ReferenceKind kind,
                                      const std::string& name);

// Todos los nombres usados de un tipo, ordenados y sin repetir. Es lo
// que el panel necesita para ofrecer la lista desplegable de «variables
// de este proyecto» en vez de obligar a escribir el nombre de memoria.
std::vector<std::string> CollectReferenceNames(const MapDocument& document,
                                               ReferenceKind kind);

// Plegado que usa la búsqueda: minúsculas y sin acentos. Público porque
// el panel filtra listas ya cargadas con el mismo criterio y dos
// criterios distintos de búsqueda en la misma pantalla desconciertan.
std::string NormalizeForSearch(const std::string& text);

}  // namespace Editor

// NOTA DE UBICACIÓN. Esto vive en include/Editor + src/Editor y no en
// editor/, que es la capa ImGui. Tal cual está no toca GL, así que se
// prueba sin ventana; el panel de E-06 lo consumirá desde arriba. Mismo
// reparto que E-07 (validación) y E-03 (intérprete): el estado abajo y
// sin GL, el dibujo arriba.
//
// Su .cpp se compila dentro de cmake/ProblemsStateTests.cmake y todavía
// no dentro de ninguna librería. Es deliberado: meterlo en motor_level
// obliga a editar el bloque compartido del CMakeLists, que varios
// agentes tocan a la vez. Cuando el panel lo necesite enlazado, ese
// paquete lo mueve.
