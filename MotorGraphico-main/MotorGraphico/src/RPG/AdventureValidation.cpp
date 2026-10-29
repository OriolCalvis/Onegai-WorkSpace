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
#include <map>
#include <set>
#include <string>
#include <variant>
#include <vector>

#include "RPG/EventCommand.h"

// E-07 — Implementación. Ver la nota larga del header.

namespace RPG {

std::size_t ValidationReport::errorCount() const {
    return static_cast<std::size_t>(
        std::count_if(diagnostics.begin(), diagnostics.end(),
                      [](const Diagnostic& d) { return d.isError(); }));
}

std::size_t ValidationReport::warningCount() const {
    return diagnostics.size() - errorCount();
}

namespace {

// Lo que el validador va aprendiendo del documento entero antes de poder
// juzgar nada. «Nadie enciende esta bandera» no se puede decir mirando un
// evento: hay que haber visto todos.
struct Inventario {
    std::set<std::string> banderasEncendidas;   // alguien las pone
    std::set<std::string> banderasLeidas;       // alguien las consulta
    std::set<std::string> eventosExistentes;
    // evento -> páginas, para poder decir «esta página no existe».
    std::map<std::string, std::size_t> paginasPorEvento;
};

Diagnostic Hallazgo(DiagnosticSeverity severity, std::string message,
                    const std::string& eventId, std::size_t pageIndex,
                    std::vector<std::size_t> path) {
    Diagnostic d;
    d.severity = severity;
    d.message = std::move(message);
    d.eventId = eventId;
    d.hasPage = true;
    d.pageIndex = pageIndex;
    d.commandPath = std::move(path);
    return d;
}

std::string Param(const MapEventCommand& command, const std::string& key) {
    const auto encontrado = command.params.find(key);
    return encontrado == command.params.end() ? std::string() : encontrado->second;
}

// Un comando es TERMINAL si acaba el evento local. Hoy solo `mapa.ir`
// (DEC-30). Se mira por tipo y no por una marca del catálogo porque el
// catálogo describe parámetros, no control de flujo; cuando haya un
// segundo terminal, este es el sitio donde se añade.
bool EsTerminal(const MapEventCommand& command) { return command.type == "mapa.ir"; }

// Primera pasada: recoge lo que el documento entero contiene.
void Recoger(const std::vector<MapEventCommand>& comandos, Inventario& inv) {
    for (const MapEventCommand& comando : comandos) {
        if (comando.type == "bandera.encender" || comando.type == "bandera.apagar") {
            const std::string bandera = Param(comando, "bandera");
            if (!bandera.empty()) {
                inv.banderasEncendidas.insert(bandera);
            }
        }
        if (comando.type == "control.si" && Param(comando, "clase") == "flag") {
            const std::string bandera = Param(comando, "referencia");
            if (!bandera.empty()) {
                inv.banderasLeidas.insert(bandera);
            }
        }
        for (const MapEventCommand& rama : comando.children) {
            Recoger(rama.children, inv);
        }
    }
}

// Segunda pasada: juzga.
void Revisar(const std::vector<MapEventCommand>& comandos, const Inventario& inv,
             const std::string& eventId, std::size_t pageIndex,
             std::vector<std::size_t> prefijo, ValidationReport& informe) {
    bool yaTerminado = false;
    for (std::size_t i = 0; i < comandos.size(); ++i) {
        const MapEventCommand& comando = comandos[i];
        std::vector<std::size_t> ruta = prefijo;
        ruta.push_back(i);

        // DEC-30 — Lo que va detrás de un comando terminal no se ejecuta
        // jamás. El plan lo pide con estas palabras: «el que hi hagi a
        // sota rep diagnòstic d'inabastable». Se avisa UNA vez, no una
        // por comando: cincuenta avisos idénticos no ayudan a nadie.
        if (yaTerminado) {
            informe.diagnostics.push_back(Hallazgo(
                DiagnosticSeverity::Warning,
                "esto no se ejecutará nunca: va detrás de un comando que termina el evento",
                eventId, pageIndex, ruta));
            break;
        }

        // Forma del comando: tipo conocido, parámetros obligatorios,
        // tipos correctos, ramas dentro de lo que admite. Lo hace E-04,
        // que es quien sabe qué es cada comando.
        const Result<bool> forma = ValidateCommand(comando);
        if (!forma.isOk()) {
            informe.diagnostics.push_back(Hallazgo(DiagnosticSeverity::Error,
                                                   forma.errorMessage(), eventId, pageIndex, ruta));
        } else {
            const CommandSpec* spec = FindCommand(comando.type);
            // Declarado pero no cableado: se puede guardar, no se puede
            // probar. Si no fuera un error, el autor montaría la misión
            // entera y descubriría al jugarla que media no hace nada.
            if (spec != nullptr && !spec->implemented) {
                informe.diagnostics.push_back(Hallazgo(
                    DiagnosticSeverity::Error,
                    "el comando '" + spec->label +
                        "' todavía no está implementado: no se puede probar una misión que lo use",
                    eventId, pageIndex, ruta));
            }
            // Un interruptor local de un evento que no existe no lo lee
            // nadie nunca.
            if (comando.type == "interruptor.fijar") {
                const std::string destino = Param(comando, "evento");
                if (!destino.empty() && inv.eventosExistentes.count(destino) == 0) {
                    informe.diagnostics.push_back(Hallazgo(
                        DiagnosticSeverity::Error,
                        "no existe ningún evento con el id '" + destino + "'",
                        eventId, pageIndex, ruta));
                }
            }
            // Una bandera que se consulta y que nadie enciende. Es AVISO
            // y no error: puede encenderla un beat antiguo, que este
            // validador no mira.
            if (comando.type == "control.si" && Param(comando, "clase") == "flag") {
                const std::string bandera = Param(comando, "referencia");
                if (!bandera.empty() && inv.banderasEncendidas.count(bandera) == 0) {
                    informe.diagnostics.push_back(Hallazgo(
                        DiagnosticSeverity::Warning,
                        "ningún evento enciende la bandera '" + bandera +
                            "': esta condición no se cumplirá nunca",
                        eventId, pageIndex, ruta));
                }
            }
        }

        if (EsTerminal(comando)) {
            yaTerminado = true;
        }
        // Las ramas, recursivamente. Un terminal DENTRO de una rama no
        // hace inalcanzable lo que viene después de la rama: solo lo que
        // viene después dentro de esa misma rama.
        for (std::size_t r = 0; r < comando.children.size(); ++r) {
            std::vector<std::size_t> rutaRama = ruta;
            rutaRama.push_back(r);
            Revisar(comando.children[r].children, inv, eventId, pageIndex, rutaRama, informe);
        }
    }
}

}  // namespace

ValidationReport ValidateEvents(const MapDocument& document) {
    ValidationReport informe;

    // --- Primera pasada: qué hay ---
    Inventario inv;
    std::vector<const MapEventDocument*> eventos;
    for (const MapLayer& capa : document.layers()) {
        const EventLayerDocument* capaEventos = std::get_if<EventLayerDocument>(&capa.data());
        if (capaEventos == nullptr) {
            continue;
        }
        for (const MapEventDocument& evento : capaEventos->events()) {
            eventos.push_back(&evento);
            inv.eventosExistentes.insert(evento.id.value());
            inv.paginasPorEvento[evento.id.value()] = evento.pages.size();
            for (const MapEventPage& pagina : evento.pages) {
                Recoger(pagina.commands, inv);
                for (const MapEventCondition& condicion : pagina.conditions) {
                    if (condicion.kind == EventConditionKind::Flag && !condicion.reference.empty()) {
                        inv.banderasLeidas.insert(condicion.reference);
                    }
                }
            }
        }
    }

    // --- Segunda pasada: qué está mal ---
    for (const MapEventDocument* evento : eventos) {
        const std::string id = evento->id.value();

        // Un evento sin páginas es una casilla que no hace nada. No es un
        // error —el diseño permite guardar borradores— pero el autor
        // querrá saberlo antes de preguntarse por qué su cofre no responde.
        if (evento->pages.empty()) {
            Diagnostic d;
            d.severity = DiagnosticSeverity::Warning;
            d.message = "el evento no tiene ninguna página: no hará nada";
            d.eventId = id;
            informe.diagnostics.push_back(d);
            continue;
        }

        for (std::size_t p = 0; p < evento->pages.size(); ++p) {
            const MapEventPage& pagina = evento->pages[p];

            for (const MapEventCondition& condicion : pagina.conditions) {
                if (condicion.kind == EventConditionKind::Flag &&
                    !condicion.reference.empty() &&
                    inv.banderasEncendidas.count(condicion.reference) == 0) {
                    Diagnostic d;
                    d.severity = DiagnosticSeverity::Warning;
                    d.message = "ningún evento enciende la bandera '" + condicion.reference +
                                "': esta página no se activará nunca";
                    d.eventId = id;
                    d.hasPage = true;
                    d.pageIndex = p;
                    informe.diagnostics.push_back(d);
                }
                if (condicion.kind == EventConditionKind::LocalSwitch &&
                    !condicion.reference.empty()) {
                    // Formato: "eventId.letra"
                    const std::string& ref = condicion.reference;
                    const auto dotPos = ref.find('.');
                    if (dotPos == std::string::npos || dotPos == 0 || dotPos + 1 >= ref.size()) {
                        Diagnostic d;
                        d.severity = DiagnosticSeverity::Error;
                        d.message = "condición de interruptor local inválida: se espera 'eventId.letra' (A-D)";
                        d.eventId = id;
                        d.hasPage = true;
                        d.pageIndex = p;
                        informe.diagnostics.push_back(d);
                    } else {
                        const std::string eventId = ref.substr(0, dotPos);
                        const char letter = ref[dotPos + 1];
                        if (!inv.eventosExistentes.count(eventId)) {
                            Diagnostic d;
                            d.severity = DiagnosticSeverity::Warning;
                            d.message = "interruptor local referencia evento inexistente '" + eventId + "'";
                            d.eventId = id;
                            d.hasPage = true;
                            d.pageIndex = p;
                            informe.diagnostics.push_back(d);
                        }
                    }
                }
            }

            // Una página sin comandos se activa y no hace nada, que casi
            // siempre es un descuido.
            if (pagina.commands.empty()) {
                Diagnostic d;
                d.severity = DiagnosticSeverity::Warning;
                d.message = "la página no tiene ningún comando";
                d.eventId = id;
                d.hasPage = true;
                d.pageIndex = p;
                informe.diagnostics.push_back(d);
            }

            Revisar(pagina.commands, inv, id, p, {}, informe);
        }
    }

    return informe;
}


}  // namespace RPG
