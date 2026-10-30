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


// Buscar: por nombre con acentos plegados, y quien referencia a quien.
//
// Cambia por motivos distintos que el resto: crece cuando aparece una
// clase de referencia nueva o cuando hay que plegar un caracter mas.
// Sus cinco ayudantes solo los usa esta busqueda.
//
// Salio de src/Editor/ProblemsState.cpp el 24/09/2026 (EST-31): aquel
// fichero tenia 643 lineas y tres trabajos dentro. Se movio, no se
// reescribio. El porque de cada decision sigue en el header.

#include "Editor/ProblemsState.h"

#include <algorithm>
#include <functional>
#include <set>

#include "Map/EventLayerDocument.h"
#include "RPG/EventCommand.h"

#include "Editor/ProblemsInternal.h"

namespace Editor {

using internal::EventosDe;

namespace {

bool TipoParametroEsDeClase(RPG::CommandParamType tipo, ReferenceKind clase) {
    switch (clase) {
        case ReferenceKind::Flag:        return tipo == RPG::CommandParamType::FlagRef;
        case ReferenceKind::Variable:    return tipo == RPG::CommandParamType::VariableRef;
        case ReferenceKind::LocalSwitch: return tipo == RPG::CommandParamType::SwitchLetter;
        case ReferenceKind::Event:       return tipo == RPG::CommandParamType::EventRef;
        case ReferenceKind::Map:         return tipo == RPG::CommandParamType::MapRef;
        case ReferenceKind::Object:      return tipo == RPG::CommandParamType::ObjectRef;
    }
    return false;
}

// `control.si` es la excepción de la que no se puede escapar: su
// parámetro `referencia` es una bandera, una variable o un objeto según
// lo que diga `clase`, así que el catálogo lo declara como texto libre y
// el tipo real sólo se sabe leyendo el comando. Sin este caso, buscar
// «dónde se lee la bandera puerta_abierta» no encontraría precisamente
// las condiciones, que es donde más se leen.

bool CondicionalMiraEstaClase(const MapEventCommand& comando, ReferenceKind clase) {
    if (comando.type != "control.si") {
        return false;
    }
    const auto it = comando.params.find("clase");
    const std::string valor = (it == comando.params.end()) ? std::string() : it->second;
    if (valor == "flag") return clase == ReferenceKind::Flag;
    if (valor == "variable") return clase == ReferenceKind::Variable;
    if (valor == "item") return clase == ReferenceKind::Object;
    return false;
}


ReferenceKind ClaseDeCondicion(EventConditionKind kind) {
    switch (kind) {
        case EventConditionKind::Flag:     return ReferenceKind::Flag;
        case EventConditionKind::Variable: return ReferenceKind::Variable;
        case EventConditionKind::Item:     return ReferenceKind::Object;
        // Faltaba este caso: sin el, una condicion de interruptor local caia
        // en el `return ReferenceKind::Flag` de abajo y buscar el interruptor
        // «A» no la encontraba (y buscar la bandera «A», si).
        case EventConditionKind::LocalSwitch: return ReferenceKind::LocalSwitch;
    }
    return ReferenceKind::Flag;
}

// Visita cada referencia de la clase pedida dentro de una lista de
// comandos. `nombre` vacío significa «todas»: así CollectReferenceNames y
// FindReferences comparten recorrido y no pueden discrepar.

void RecorrerComandos(const std::vector<MapEventCommand>& comandos,
                      ReferenceKind clase,
                      const std::string& nombre,
                      const std::vector<std::size_t>& prefijo,
                      const std::function<void(const std::vector<std::size_t>&,
                                               const MapEventCommand&,
                                               const std::string& clave,
                                               const std::string& valor)>& visitar) {
    for (std::size_t i = 0; i < comandos.size(); ++i) {
        const MapEventCommand& comando = comandos[i];
        std::vector<std::size_t> ruta = prefijo;
        ruta.push_back(i);

        const RPG::CommandSpec* spec = RPG::FindCommand(comando.type);
        if (spec != nullptr) {
            for (const RPG::CommandParamSpec& param : spec->params) {
                const bool porTipo = TipoParametroEsDeClase(param.type, clase);
                const bool porClase =
                    param.key == "referencia" && CondicionalMiraEstaClase(comando, clase);
                if (!porTipo && !porClase) {
                    continue;
                }
                const auto it = comando.params.find(param.key);
                if (it == comando.params.end() || it->second.empty()) {
                    continue;
                }
                if (!nombre.empty() && it->second != nombre) {
                    continue;
                }
                visitar(ruta, comando, param.key, it->second);
            }
        }

        for (std::size_t r = 0; r < comando.children.size(); ++r) {
            std::vector<std::size_t> rutaRama = ruta;
            rutaRama.push_back(r);
            RecorrerComandos(comando.children[r].children, clase, nombre, rutaRama, visitar);
        }
    }
}


}  // namespace

std::string NormalizeForSearch(const std::string& text) {
    std::string salida;
    salida.reserve(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(text[i]);
        // Latin-1 en UTF-8: dos bytes que empiezan por 0xC3. Se pliegan a
        // su letra sin acento. DEC-15 permite acentos en el texto
        // visible; quien busca escribe sin ellos.
        if (c == 0xC3 && i + 1 < text.size()) {
            const unsigned char d = static_cast<unsigned char>(text[i + 1]);
            const unsigned char b = static_cast<unsigned char>(d | 0x20);  // minúscula
            char plegado = 0;
            if (b >= 0xA0 && b <= 0xA5) plegado = 'a';
            else if (b >= 0xA8 && b <= 0xAB) plegado = 'e';
            else if (b >= 0xAC && b <= 0xAF) plegado = 'i';
            else if (b >= 0xB2 && b <= 0xB6) plegado = 'o';
            else if (b >= 0xB9 && b <= 0xBC) plegado = 'u';
            else if (b == 0xB1) plegado = 'n';
            else if (b == 0xA7) plegado = 'c';
            if (plegado != 0) {
                salida += plegado;
                ++i;
                continue;
            }
        }
        if (c >= 'A' && c <= 'Z') {
            salida += static_cast<char>(c - 'A' + 'a');
        } else {
            salida += static_cast<char>(c);
        }
    }
    return salida;
}

std::vector<SearchHit> SearchEventsByName(const MapDocument& document,
                                          const std::string& query) {
    std::vector<SearchHit> hallazgos;
    const std::string aguja = NormalizeForSearch(query);
    // Una búsqueda vacía devuelve todo, no nada: el panel abre con la
    // caja vacía y la lista llena.
    for (const MapEventDocument* evento : EventosDe(document)) {
        const std::string nombre = NormalizeForSearch(evento->name);
        const std::string id = NormalizeForSearch(evento->id.value());
        if (!aguja.empty() && nombre.find(aguja) == std::string::npos &&
            id.find(aguja) == std::string::npos) {
            continue;
        }
        SearchHit hit;
        hit.where.mapId = document.id().value();
        hit.where.eventId = evento->id.value();
        hit.eventName = evento->name;
        hit.detail = evento->name.empty() ? evento->id.value() : evento->name;
        hallazgos.push_back(std::move(hit));
    }
    return hallazgos;
}

namespace {

// Recorrido común de FindReferences y CollectReferenceNames. `nombre`
// vacío = todas las referencias de esa clase.
void RecorrerReferencias(const MapDocument& document, ReferenceKind clase,
                         const std::string& nombre,
                         const std::function<void(const SearchHit&)>& visitar) {
    for (const MapEventDocument* evento : EventosDe(document)) {
        for (std::size_t p = 0; p < evento->pages.size(); ++p) {
            const MapEventPage& pagina = evento->pages[p];

            for (const MapEventCondition& condicion : pagina.conditions) {
                if (ClaseDeCondicion(condicion.kind) != clase || condicion.reference.empty()) {
                    continue;
                }
                if (!nombre.empty() && condicion.reference != nombre) {
                    continue;
                }
                SearchHit hit;
                hit.where.mapId = document.id().value();
                hit.where.eventId = evento->id.value();
                hit.where.hasPage = true;
                hit.where.pageId = pagina.id.value();
                hit.where.pageIndex = p;
                hit.eventName = evento->name;
                hit.detail = "condición de la página: " + condicion.reference;
                visitar(hit);
            }

            RecorrerComandos(pagina.commands, clase, nombre, {},
                             [&](const std::vector<std::size_t>& ruta,
                                 const MapEventCommand& comando, const std::string& clave,
                                 const std::string& valor) {
                                 SearchHit hit;
                                 hit.where.mapId = document.id().value();
                                 hit.where.eventId = evento->id.value();
                                 hit.where.hasPage = true;
                                 hit.where.pageId = pagina.id.value();
                                 hit.where.pageIndex = p;
                                 hit.where.hasCommand = true;
                                 hit.where.commandPath = ruta;
                                 hit.where.commandType = comando.type;
                                 hit.eventName = evento->name;
                                 hit.detail = comando.type + " · " + clave + " = " + valor;
                                 visitar(hit);
                             });
        }
    }
}

}  // namespace

std::vector<SearchHit> FindReferences(const MapDocument& document, ReferenceKind kind,
                                      const std::string& name) {
    std::vector<SearchHit> hallazgos;
    // Nombre vacío no significa «todos» aquí: significa que no se ha
    // pedido nada. Devolver la lista entera al abrir el desplegable sería
    // ruido.
    if (name.empty()) {
        return hallazgos;
    }
    RecorrerReferencias(document, kind, name,
                        [&hallazgos](const SearchHit& hit) { hallazgos.push_back(hit); });
    return hallazgos;
}

std::vector<std::string> CollectReferenceNames(const MapDocument& document,
                                               ReferenceKind kind) {
    // Recorrido propio, quedándose con el valor crudo. No se reaprovecha
    // RecorrerReferencias porque su `detail` es texto para enseñar:
    // sacar el nombre de ahí ataría esta función al formato de un
    // mensaje, que puede cambiar sin que nadie se acuerde de esto.
    std::set<std::string> nombres;
    for (const MapEventDocument* evento : EventosDe(document)) {
        for (const MapEventPage& pagina : evento->pages) {
            for (const MapEventCondition& condicion : pagina.conditions) {
                if (ClaseDeCondicion(condicion.kind) == kind && !condicion.reference.empty()) {
                    nombres.insert(condicion.reference);
                }
            }
            RecorrerComandos(pagina.commands, kind, std::string(), {},
                             [&nombres](const std::vector<std::size_t>&, const MapEventCommand&,
                                        const std::string&, const std::string& valor) {
                                 nombres.insert(valor);
                             });
        }
    }
    return std::vector<std::string>(nombres.begin(), nombres.end());
}


}  // namespace Editor
