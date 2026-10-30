// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo toca 1 paquete del corte B:
//   E-05  (fase E, 8 j) - Editor de pàgines d'esdeveniment
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Editor/EventEditorState.h"
#include "Editor/CommandListState.h"

#include <algorithm>
#include <string>
#include <variant>
#include <vector>

#include "Map/MapLayer.h"

namespace RPG {

namespace {

bool sameCell(GridCoord a, GridCoord b) { return a.x == b.x && a.y == b.y; }

// E-09 — La capa del document on viuen els esdeveniments. El nom no és
// negociable: és el que fa servir tothom qui busca la capa.
constexpr const char* kCapaEsdeveniments = "events";

EventTrigger triggerDe(const std::string& text) {
    if (text == "touch")    return EventTrigger::Touch;
    if (text == "autorun")  return EventTrigger::Autorun;
    if (text == "parallel") return EventTrigger::Parallel;
    // Buit o desconegut: el disparador per defecte del document. No es
    // retorna error perque una pagina sense trigger es legal a l'editor.
    return EventTrigger::Interact;
}

// Una pàgina de l'editor (E-05) a una pàgina del document (E-01).
Result<MapEventPage> paginaDe(const EventPage& pagina) {
    Result<ids::EventId> id = ids::EventId::parse(pagina.id);
    if (!id.isOk()) {
        return Result<MapEventPage>::Error("id de pagina no valid: " + id.errorMessage());
    }
    MapEventPage out;
    out.id = id.value();
    out.name = pagina.id;
    out.trigger = triggerDe(pagina.trigger);

    // L'editor te una sola condicio i sempre es una bandera narrativa; el
    // document en te una llista amb mena, comparacio i valor. Escriure una
    // condicio que no diu res la rebutjaria validate(), aixi que el buit no
    // es converteix.
    if (!pagina.condition.empty()) {
        MapEventCondition condicio;
        condicio.kind = EventConditionKind::Flag;
        condicio.reference = pagina.condition;
        out.conditions.push_back(condicio);
    }

    if (!pagina.structuredCommands.empty()) {
        out.commands = pagina.structuredCommands;
    } else {
        out.commands.reserve(pagina.commands.size());
        for (const std::string& legacy : pagina.commands) {
            // Una comanda sense tipus no passa validate() i faria fallar tot
            // el document; val mes saltar-se-la que enverinar la capa.
            if (legacy.empty()) continue;
            MapEventCommand comanda;
            comanda.type = legacy;
            out.commands.push_back(comanda);
        }
    }
    return Result<MapEventPage>::Ok(out);
}

}  // namespace

EventEditorState::EventEditorState(MapDocument* doc) : m_doc(doc) {}

Result<std::string> EventEditorState::createEvent(const GridCoord& pos) {
    if (getEvent(pos) != nullptr) {
        return Result<std::string>::Error("Ya existe un evento en esta casilla");
    }
    std::string id = "event_" + std::to_string(m_events.size());
    EventDocument ev;
    ev.id = id;
    ev.position = pos;
    m_events.push_back(ev);
    EditOp op;
    op.type = EditOp::CREATE;
    op.eventId = id;
    m_history.push_back(op);
    return Result<std::string>::Ok(id);
}

Result<std::string> EventEditorState::addPage(const std::string& eventId, const EventPage& page) {
    for (auto& e : m_events) {
        if (e.id == eventId) {
            std::string pageId = "page_" + std::to_string(e.pages.size());
            EventPage p = page;
            p.id = pageId;
            e.pages.push_back(p);
            EditOp op;
            op.type = EditOp::ADD;
            op.eventId = eventId;
            op.pageId = pageId;
            m_history.push_back(op);
            return Result<std::string>::Ok(pageId);
        }
    }
    return Result<std::string>::Error("Esdeveniment no trobat");
}

Result<void> EventEditorState::addStructuredCommand(const std::string& eventId,
                                                    const std::string& pageId,
                                                    const MapEventCommand& command) {
    if (command.type.empty()) {
        return Result<void>::Error("una comanda sense tipus no es pot afegir");
    }
    for (auto& esdeveniment : m_events) {
        if (esdeveniment.id != eventId) continue;
        for (auto& pagina : esdeveniment.pages) {
            if (pagina.id != pageId) continue;
            pagina.structuredCommands.push_back(command);
            return Result<void>::Ok();
        }
        return Result<void>::Error("Pagina no trobada: " + pageId);
    }
    return Result<void>::Error("Esdeveniment no trobat: " + eventId);
}

Result<std::string> EventEditorState::duplicatePage(const std::string& eventId, const std::string& pageId) {
    for (auto& e : m_events) {
        if (e.id == eventId) {
            for (auto& p : e.pages) {
                if (p.id == pageId) {
                    std::string newId = "page_" + std::to_string(e.pages.size());
                    EventPage copy = p;
                    copy.id = newId;
                    e.pages.push_back(copy);
                    EditOp op;
                    op.type = EditOp::DUPLICATE;
                    op.eventId = eventId;
                    op.pageId = pageId;
                    m_history.push_back(op);
                    return Result<std::string>::Ok(newId);
                }
            }
            return Result<std::string>::Error("Pàgina no trobada");
        }
    }
    return Result<std::string>::Error("Esdeveniment no trobat");
}

bool EventEditorState::removePage(const std::string& eventId, const std::string& pageId) {
    for (auto& e : m_events) {
        if (e.id == eventId) {
            auto it = std::remove_if(e.pages.begin(), e.pages.end(),
                [&pageId](const EventPage& p) { return p.id == pageId; });
            if (it != e.pages.end()) {
                e.pages.erase(it, e.pages.end());
                EditOp op;
                op.type = EditOp::REMOVE;
                op.eventId = eventId;
                op.pageId = pageId;
                m_history.push_back(op);
                return true;
            }
            return false;
        }
    }
    return false;
}

bool EventEditorState::reorderPage(const std::string& eventId, const std::string& pageId, int newIndex) {
    for (auto& e : m_events) {
        if (e.id == eventId) {
            size_t oldIndex = 0;
            bool found = false;
            for (size_t i = 0; i < e.pages.size(); ++i) {
                if (e.pages[i].id == pageId) {
                    oldIndex = i;
                    found = true;
                    break;
                }
            }
            if (!found) return false;
            if (newIndex < 0 || newIndex >= (int)e.pages.size()) return false;
            EventPage p = e.pages[oldIndex];
            e.pages.erase(e.pages.begin() + oldIndex);
            e.pages.insert(e.pages.begin() + newIndex, p);
            EditOp op;
            op.type = EditOp::REORDER;
            op.eventId = eventId;
            op.pageId = pageId;
            op.oldIndex = (int)oldIndex;
            op.newIndex = newIndex;
            m_history.push_back(op);
            return true;
        }
    }
    return false;
}

const EventDocument* EventEditorState::getEvent(const GridCoord& pos) const {
    for (const auto& e : m_events) {
        if (sameCell(e.position, pos)) return &e;
    }
    return nullptr;
}

size_t EventEditorState::pageCount(const std::string& eventId) const {
    for (const auto& e : m_events) {
        if (e.id == eventId) return e.pages.size();
    }
    return 0;
}

bool EventEditorState::undo() {
    if (m_history.empty()) return false;
    EditOp op = m_history.back();
    m_history.pop_back();
    if (op.type == EditOp::CREATE) {
        auto it = std::remove_if(m_events.begin(), m_events.end(),
            [&op](const EventDocument& e) { return e.id == op.eventId; });
        m_events.erase(it, m_events.end());
    } else if (op.type == EditOp::ADD || op.type == EditOp::DUPLICATE) {
        for (auto& e : m_events) {
            if (e.id == op.eventId) {
                auto it = std::remove_if(e.pages.begin(), e.pages.end(),
                    [&op](const EventPage& p) { return p.id == op.pageId; });
                e.pages.erase(it, e.pages.end());
                break;
            }
        }
    } else if (op.type == EditOp::REMOVE) {
        // Simplified: just mark as undone
    } else if (op.type == EditOp::REORDER) {
        for (auto& e : m_events) {
            if (e.id == op.eventId) {
                if (op.oldIndex >= 0 && op.oldIndex < (int)e.pages.size()) {
                    EventPage p = e.pages[op.newIndex];
                    e.pages.erase(e.pages.begin() + op.newIndex);
                    e.pages.insert(e.pages.begin() + op.oldIndex, p);
                }
                break;
            }
        }
    }
    return true;
}

// E-09 — El pont de l'editor al document. Abans era un stub que retornava
// true sense escriure res, i la missió «escrita des de l'editor» no arribava
// enlloc.
//
// Dos contractes que surten de la prova i no són evidents:
//   - Idempotent. Desar dues vegades no duplica els esdeveniments: si ja hi
//     són, se'n substitueixen les pàgines i la posició.
//   - Tot o res. Primer es converteix tot a una llista local i només després
//     es toca el document: si un id no es parsejable, el document queda com
//     estava. És el mateix contracte que les capes fan servir per dins.
bool EventEditorState::save(MapDocument& doc) {
    // Sense esdeveniments no es crea la capa: una capa buida és soroll al
    // document i al fitxer desat.
    if (m_events.empty()) {
        return true;
    }

    std::vector<MapEventDocument> convertits;
    convertits.reserve(m_events.size());
    for (const EventDocument& esdeveniment : m_events) {
        Result<ids::EventId> id = ids::EventId::parse(esdeveniment.id);
        if (!id.isOk()) {
            return false;
        }
        MapEventDocument med;
        med.id = id.value();
        med.name = esdeveniment.id;
        med.position = esdeveniment.position;
        med.pages.reserve(esdeveniment.pages.size());
        for (const EventPage& pagina : esdeveniment.pages) {
            Result<MapEventPage> convertida = paginaDe(pagina);
            if (!convertida.isOk()) {
                return false;
            }
            med.pages.push_back(convertida.value());
        }
        convertits.push_back(med);
    }

    Result<ids::LayerId> capaId = ids::LayerId::parse(kCapaEsdeveniments);
    if (!capaId.isOk()) {
        return false;
    }
    const ids::LayerId& capa = capaId.value();

    if (doc.findLayer(capa) == nullptr) {
        MapLayerMeta meta;
        meta.id = capa;
        meta.name = kCapaEsdeveniments;
        meta.order = static_cast<int>(doc.layers().size());
        if (!doc.addLayer(MapLayer::events(meta, EventLayerDocument())).isOk()) {
            return false;
        }
    }

    for (const MapEventDocument& med : convertits) {
        const MapLayer* capaTrobada = doc.findLayer(capa);
        const auto* dades = capaTrobada != nullptr
                                ? std::get_if<EventLayerDocument>(&capaTrobada->data())
                                : nullptr;
        const bool jaHiEs = dades != nullptr && dades->find(med.id) != nullptr;
        if (!jaHiEs) {
            if (!doc.addEvent(capa, med).isOk()) {
                return false;
            }
        } else {
            if (!doc.setEventPages(capa, med.id, med.pages).isOk()) {
                return false;
            }
            if (!doc.moveEvent(capa, med.id, med.position).isOk()) {
                return false;
            }
        }
    }
    return true;
}

// E-06 — ComandListState implementation.

CommandListState::CommandListState(MapDocument* doc) : m_doc(doc) {}

Result<std::string> CommandListState::addCommand(const std::string& command,
                                                  const std::vector<std::string>& params) {
    std::string id = "cmd_" + std::to_string(m_commands.size());
    CommandEntry ce;
    ce.id = id;
    ce.command = command;
    ce.parameters = params;
    m_commands.push_back(ce);
    return Result<std::string>::Ok(id);
}

Result<std::string> CommandListState::insertCommand(size_t index, const std::string& command,
                                                     const std::vector<std::string>& params) {
    if (index > m_commands.size()) return Result<std::string>::Error("Índio fora de rang");
    std::string id = "cmd_" + std::to_string(m_commands.size());
    CommandEntry ce;
    ce.id = id;
    ce.command = command;
    ce.parameters = params;
    if (index < m_commands.size()) {
        m_commands.insert(m_commands.begin() + index, ce);
    } else {
        m_commands.push_back(ce);
    }
    return Result<std::string>::Ok(id);
}

bool CommandListState::removeCommand(const std::string& commandId) {
    auto it = std::remove_if(m_commands.begin(), m_commands.end(),
        [&commandId](const CommandEntry& c) { return c.id == commandId; });
    if (it != m_commands.end()) {
        m_commands.erase(it, m_commands.end());
        return true;
    }
    return false;
}

bool CommandListState::moveCommand(const std::string& commandId, size_t newIndex) {
    size_t oldIndex = 0;
    bool found = false;
    for (size_t i = 0; i < m_commands.size(); ++i) {
        if (m_commands[i].id == commandId) {
            oldIndex = i;
            found = true;
            break;
        }
    }
    if (!found) return false;
    if (newIndex >= m_commands.size()) return false;
    CommandEntry c = m_commands[oldIndex];
    m_commands.erase(m_commands.begin() + oldIndex);
    m_commands.insert(m_commands.begin() + newIndex, c);
    return true;
}

Result<std::string> CommandListState::openBranch(const std::string& commandId) {
    (void)commandId;
    return Result<std::string>::Ok("branch_0");
}

bool CommandListState::closeBranch(const std::string& commandId) {
    (void)commandId;
    return true;
}

size_t CommandListState::commandCount() const {
    return m_commands.size();
}

bool CommandListState::execute(EventEditorState& editor, const std::string& eventId) {
    (void)editor; (void)eventId;
    return true;
}

bool CommandListState::undo() {
    if (m_history.empty()) return false;
    m_history.pop_back();
    return true;
}

} // namespace RPG