// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-06  (fase E, 9 j) - Editor vertical de comandes
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado >>>

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"
#include "Editor/EventEditorState.h"

namespace RPG {

// E-06 — Editor vertical de comandes.
//
// Cada pàgina d'un esdeveniment té una llista de comandes.
// L'editor permite inserir, esborrar, moure i entrar en
// branques (sub-llistes) de comandes. Les comandes amb
// paràmetres obren el seu formulari.

struct CommandEntry {
    std::string id;
    std::string command;        // nom de la comanda (say, open_door...)
    std::vector<std::string> parameters;  // arguments de la comanda
    std::vector<CommandEntry> children;   // branques (sub-comandes)
    int indent = 0;             // nivell de sagnat
};

class CommandListState {
public:
    explicit CommandListState(MapDocument* doc = nullptr);

    // Afegir una comanda a la llista. Retorna l'ID de la comanda.
    Result<std::string> addCommand(const std::string& command,
                                    const std::vector<std::string>& params = {});

    // Inserir una comanda a una posició concreta.
    Result<std::string> insertCommand(size_t index, const std::string& command,
                                       const std::vector<std::string>& params = {});

    // Esborrar una comanda per ID.
    bool removeCommand(const std::string& commandId);

    // Moure una comanda d'una posició a una altra.
    bool moveCommand(const std::string& commandId, size_t newIndex);

    // Obrir una branca (sub-llista) per a una comanda.
    Result<std::string> openBranch(const std::string& commandId);

    // Tancar una branca.
    bool closeBranch(const std::string& commandId);

    // Obtenir el nombre de comandes de primer nivell.
    size_t commandCount() const;

    // Executar la llista de comandes contra un EventEditorState.
    bool execute(EventEditorState& editor, const std::string& eventId);

    // Desfer l'última operació.
    bool undo();

private:
    MapDocument* m_doc = nullptr;
    std::vector<CommandEntry> m_commands;
    struct EditOp {
        enum Type { ADD, INSERT, REMOVE, MOVE, BRANCH_OPEN, BRANCH_CLOSE } type;
        std::string commandId;
        size_t oldIndex = 0;
        size_t newIndex = 0;
    };
    std::vector<EditOp> m_history;
};

} // namespace RPG
