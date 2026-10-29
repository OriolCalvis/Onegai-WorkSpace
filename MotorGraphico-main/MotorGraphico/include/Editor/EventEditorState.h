// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-05  (fase E, 8 j) - Editor de pàgines d'esdeveniment
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Ids/ProjectIds.h"
#include "Core/Math/GridCoord.h"
#include "Map/EventLayerDocument.h"
#include "Map/MapDocument.h"

namespace RPG {

// E-05 — Editor de pàgines d'esdeveniment.
//
// Un esdeveniment és una entitat del mapa amb pàgines. Cada pàgina
// té un disparador, condicions i una llista de comandes.
// L'editor permet crear, duplicar, reordenar i esborrar pàgines
// sobre una casella del mapa.

struct EventPage {
    std::string id;
    std::string trigger;       // tipus de disparador (enter, parlars, passo...)
    std::string condition;     // condició per executar
    std::vector<std::string> commands;  // comandes de la pàgina (legacy: només el tipus)

    // E-09 — Comandes amb els seus paràmetres. `commands` és un
    // vector<string> i no hi cap un MapEventCommand amb params i children,
    // que és el que el document (E-01) sí que representa: aquesta era la
    // «limitació coneguda» que el pont retallava en silenci. Les dues
    // llistes conviuen per no trencar qui ja omplia `commands`, i a save()
    // manen aquestes: tenir les dues a mitges seria ambigu, i el que l'autor
    // ha omplert amb paràmetres és el que vol dir.
    std::vector<MapEventCommand> structuredCommands;

    int priority = 0;          // ordre de prioritat entre pàgines
};

struct EventDocument {
    std::string id;
    GridCoord position;        // casella del mapa on viu l'esdeveniment
    std::vector<EventPage> pages;
    std::string description;   // text opcional
};

// Estat de l'editor per a pàgines d'esdeveniment. GL-free.
class EventEditorState {
public:
    explicit EventEditorState(MapDocument* doc = nullptr);

    // Crear un esdeveniment sobre una casella. Retorna l'ID de l'esdeveniment.
    Result<std::string> createEvent(const GridCoord& pos);

    // Afegir una pàgina a un esdeveniment. Retorna l'ID de la pàgina.
    Result<std::string> addPage(const std::string& eventId, const EventPage& page);

    // E-09 — Afegir una comanda amb paràmetres a una pàgina. És el que
    // permet escriure la missió des de l'editor sense perdre'n els detalls:
    // «objeto.dar {objeto: llave, cantidad: 1}» arriba sencer al document.
    // No es registra a l'històric: desfer una comanda individual no hi cap
    // (EditOp no té cap tipus per a això) i fer-ho a mitges seria pitjor.
    Result<void> addStructuredCommand(const std::string& eventId,
                                      const std::string& pageId,
                                      const MapEventCommand& command);

    // Duplicar una pàgina dins d'un esdeveniment.
    Result<std::string> duplicatePage(const std::string& eventId, const std::string& pageId);

    // Esborrar una pàgina d'un esdeveniment.
    bool removePage(const std::string& eventId, const std::string& pageId);

    // Reordenar una pàgina (moure d'una posició a una altra).
    bool reorderPage(const std::string& eventId, const std::string& pageId, int newIndex);

    // Obtenir l'esdeveniment a una casella.
    const EventDocument* getEvent(const GridCoord& pos) const;

    // Obtenir el nombre de pàgines d'un esdeveniment.
    size_t pageCount(const std::string& eventId) const;

    // Desfer l'última operació.
    bool undo();

    // Guardar l'estat dins del MapDocument.
    bool save(MapDocument& doc);

private:
    MapDocument* m_doc = nullptr;
    std::vector<EventDocument> m_events;
    struct EditOp {
        enum Type { CREATE, ADD, DUPLICATE, REMOVE, REORDER } type;
        std::string eventId;
        std::string pageId;
        int oldIndex = -1;
        int newIndex = -1;
    };
    std::vector<EditOp> m_history;
};

} // namespace RPG
