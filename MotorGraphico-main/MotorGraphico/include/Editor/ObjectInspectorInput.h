// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   E-12  (fase E, 4 j) - Propietats de mapa, capes i instàncies
//   E-14  (fase E, 3 j) - Selectors de dades i referències
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// Entrada modal de ObjectInspector.
//
// Se incluye sólo desde ObjectInspector.cpp: no añade una fuente a CMake
// mientras el fichero de cruce está reservado por H0-07. Mantiene aquí la
// navegación y los pickers, separados del ciclo de vida y del commit de texto
// que permanecen en el controlador.

#pragma once

#include <cstddef>
#include <string>

#include "Editor/EditorState.h"
#include "Editor/ObjectInspector.h"
#include "Level/ObjectCatalog.h"

namespace Editor {

class ObjectInspectorInput {
public:
    static bool key(ObjectInspector& inspector, char key) {
        ObjectInspectorData& data = inspector.m_data;
        if (!data.open)
            return false;

        // El objeto puede haber desaparecido (deshacer, borrado): el panel
        // se cierra con cualquier tecla antes de intentar editarlo.
        if (inspector.spawn() == nullptr) {
            inspector.close();
            return true;
        }

        if (data.mode == ObjectInspectorMode::Texto) {
            return handleText(inspector, key);
        }
        if (data.mode == ObjectInspectorMode::ElegirSalida ||
            data.mode == ObjectInspectorMode::ElegirLoot) {
            return handlePicker(inspector, key);
        }
        return handleNavigation(inspector, key);
    }

private:
    static bool printable(char key) { return key >= 32 && key < 127; }

    static bool handleText(ObjectInspector& inspector, char key) {
        ObjectInspectorData& data = inspector.m_data;
        if (key == 27) {
            data.mode = ObjectInspectorMode::Navegar;
            data.buffer.clear();
            data.notice = "CANCELADO";
            return true;
        }
        if (key == '\n') {
            inspector.commitTexto();
            return true;
        }
        if (key == '\b') {
            if (!data.buffer.empty())
                data.buffer.pop_back();
            return true;
        }
        if (printable(key) && data.buffer.size() < 48)
            data.buffer += key;
        return true;
    }

    static void refilterLoot(ObjectInspector& inspector) {
        ObjectInspectorData& data = inspector.m_data;
        data.options.clear();
        for (const std::string& id : data.baseLoot) {
            const ObjectDefinition* definition = inspector.m_catalog.find(id);
            const std::string& name = definition != nullptr ? definition->name : id;
            const auto matches = [&data](const std::string& text) {
                std::string lower = text;
                for (char& character : lower) {
                    if (character >= 'A' && character <= 'Z') {
                        character = static_cast<char>(character - 'A' + 'a');
                    }
                }
                return lower.find(data.textFilter) != std::string::npos;
            };
            if (data.textFilter.empty() || matches(id) || matches(name))
                data.options.push_back(id);
        }
        if (data.pickerMark >= data.options.size())
            data.pickerMark = 0;

        data.lines.clear();
        data.lines.push_back("BUSCANDO: " +
                             (data.textFilter.empty() ? std::string("(todo)") : data.textFilter));
        constexpr std::size_t kWindow = 10;
        std::size_t first = 0;
        if (data.options.size() > kWindow && data.pickerMark > kWindow / 2) {
            first = data.pickerMark - kWindow / 2;
            if (first + kWindow > data.options.size())
                first = data.options.size() - kWindow;
        }
        for (std::size_t index = first; index < data.options.size() && index < first + kWindow;
             ++index) {
            data.lines.push_back((index == data.pickerMark ? "> " : "  ") +
                                 inspector.nombreLoot(data.options[index]));
        }
        if (data.options.empty())
            data.lines.push_back("(NADA CON ESE FILTRO: BACKSPACE BORRA)");
    }

    static bool handlePicker(ObjectInspector& inspector, char key) {
        ObjectInspectorData& data = inspector.m_data;
        if (key == 27) {
            data.mode = ObjectInspectorMode::Navegar;
            data.notice = "CANCELADO";
            inspector.rebuild();
            return true;
        }
        if (key == 'w' && !data.options.empty()) {
            data.pickerMark = data.pickerMark == 0 ? data.options.size() - 1 : data.pickerMark - 1;
            return true;
        }
        if (key == 's' && !data.options.empty()) {
            data.pickerMark = (data.pickerMark + 1) % data.options.size();
            return true;
        }
        if (key == '\n') {
            if (data.options.empty()) {
                data.mode = ObjectInspectorMode::Navegar;
                data.notice = "NO HAY NADA QUE ELEGIR";
                inspector.rebuild();
                return true;
            }
            if (data.mode == ObjectInspectorMode::ElegirSalida) {
                const std::string& destination =
                    data.pickerMark == 0 ? std::string() : data.destinations[data.pickerMark - 1];
                inspector.m_editor.setObjectTargetLevel(data.cell.x, data.cell.y, destination);
                data.notice = destination.empty()
                                  ? "SALIDA DESCONECTADA"
                                  : "SALIDA A " + objectInspectorScenarioName(destination);
            } else {
                const std::string& id = data.options[data.pickerMark];
                const std::size_t before = inspector.spawn()->loot.size();
                inspector.m_editor.addObjectLoot(data.cell.x, data.cell.y, id);
                data.notice = inspector.spawn()->loot.size() > before
                                  ? "BOTIN ANADIDO: " + inspector.nombreLoot(id)
                                  : "ESE BOTIN YA ESTABA";
            }
            data.mode = ObjectInspectorMode::Navegar;
            data.textFilter.clear();
            inspector.rebuild();
            return true;
        }
        if (data.mode != ObjectInspectorMode::ElegirLoot)
            return true;
        if (key == '\b') {
            if (!data.textFilter.empty()) {
                data.textFilter.pop_back();
                refilterLoot(inspector);
            }
            return true;
        }
        if ((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9') || key == '_') {
            data.textFilter += key;
            refilterLoot(inspector);
        }
        return true;
    }

    static void beginExitPicker(ObjectInspector& inspector) {
        ObjectInspectorData& data = inspector.m_data;
        data.mode = ObjectInspectorMode::ElegirSalida;
        data.options.clear();
        data.options.push_back("(SIN SALIDA)");
        for (const std::string& destination : data.destinations)
            data.options.push_back(destination);
        data.pickerMark = 0;
        const ObjectSpawn* spawn = inspector.spawn();
        if (spawn != nullptr && !spawn->targetLevel.empty()) {
            for (std::size_t index = 1; index < data.options.size(); ++index) {
                if (data.destinations[index - 1] == spawn->targetLevel) {
                    data.pickerMark = index;
                    break;
                }
            }
        }
        data.lines.clear();
        for (std::size_t index = 0; index < data.options.size(); ++index) {
            data.lines.push_back((index == data.pickerMark ? "> " : "  ") +
                                 (index == 0 ? data.options[index]
                                             : objectInspectorScenarioName(data.options[index])));
        }
        data.notice = "ELIGE EL DESTINO DE LA SALIDA";
    }

    static void beginLootPicker(ObjectInspector& inspector) {
        ObjectInspectorData& data = inspector.m_data;
        data.mode = ObjectInspectorMode::ElegirLoot;
        data.baseLoot = inspector.m_catalog.ids();
        data.options = data.baseLoot;
        data.textFilter.clear();
        data.pickerMark = 0;
        data.lines.clear();
        data.lines.push_back("BUSCANDO: (todo)");
        data.notice = "TECLEA PARA FILTRAR, ENTER ANADE, ESC CANCELA";
    }

    static bool handleNavigation(ObjectInspector& inspector, char key) {
        ObjectInspectorData& data = inspector.m_data;
        if (key == 27) {
            inspector.close();
            return true;
        }
        if (data.lines.empty())
            return true;
        if (key == 'w') {
            data.selected = data.selected == 0 ? data.lines.size() - 1 : data.selected - 1;
            return true;
        }
        if (key == 's') {
            data.selected = (data.selected + 1) % data.lines.size();
            return true;
        }
        if (key == 'x') {
            const ObjectInspectorRow& row = data.rows[data.selected];
            if (row.field == ObjectInspectorField::LootItem && inspector.spawn() != nullptr) {
                const std::size_t index = static_cast<std::size_t>(row.index);
                if (index < inspector.spawn()->loot.size()) {
                    const std::string id = inspector.spawn()->loot[index];
                    inspector.m_editor.removeLootAt(data.cell.x, data.cell.y, index);
                    data.notice = "BOTIN QUITADO: " + inspector.nombreLoot(id);
                    inspector.rebuild();
                }
            } else if (row.field == ObjectInspectorField::PropiedadItem &&
                       inspector.spawn() != nullptr) {
                std::size_t index = 0;
                for (const auto& property : inspector.spawn()->properties) {
                    if (static_cast<int>(index) == row.index) {
                        inspector.m_editor.removeObjectProperty(data.cell.x, data.cell.y,
                                                                property.first);
                        data.notice = "PROPIEDAD QUITADA: " + property.first;
                        inspector.rebuild();
                        break;
                    }
                    ++index;
                }
            }
            return true;
        }
        if (key != '\n')
            return true;

        const ObjectInspectorRow row = data.rows[data.selected];
        switch (row.field) {
            case ObjectInspectorField::NombreVisible:
                data.textField = ObjectInspectorField::NombreVisible;
                inspector.abrirTexto(inspector.spawn()->displayName);
                break;
            case ObjectInspectorField::Variante:
                data.textField = ObjectInspectorField::Variante;
                inspector.abrirTexto(inspector.spawn()->variant);
                break;
            case ObjectInspectorField::Efecto:
                data.textField = ObjectInspectorField::Efecto;
                inspector.abrirTexto(inspector.spawn()->effectOverride);
                break;
            case ObjectInspectorField::LootTabla:
                data.textField = ObjectInspectorField::LootTabla;
                inspector.abrirTexto(inspector.spawn()->lootTable);
                break;
            case ObjectInspectorField::Evento:
                data.textField = ObjectInspectorField::Evento;
                inspector.abrirTexto(inspector.spawn()->eventId);
                break;
            case ObjectInspectorField::PosicionDestino: {
                data.textField = ObjectInspectorField::PosicionDestino;
                std::string initial;
                if (inspector.spawn()->hasTargetPosition) {
                    initial = std::to_string(inspector.spawn()->targetPosition.x) + "," +
                              std::to_string(inspector.spawn()->targetPosition.y);
                }
                inspector.abrirTexto(initial);
                break;
            }
            case ObjectInspectorField::AnadirPropiedad:
                data.textField = ObjectInspectorField::AnadirPropiedad;
                inspector.abrirTexto("");
                break;
            case ObjectInspectorField::Escala: {
                std::size_t next = 0;
                for (std::size_t index = 0; index < kObjectInspectorScales.size(); ++index) {
                    if (inspector.spawn()->scale <= kObjectInspectorScales[index] + 0.01f) {
                        next = (index + 1) % kObjectInspectorScales.size();
                        break;
                    }
                }
                inspector.m_editor.setObjectScale(data.cell.x, data.cell.y,
                                                  kObjectInspectorScales[next]);
                data.notice = "ESCALA " + objectInspectorScaleLabel(kObjectInspectorScales[next]);
                inspector.rebuild();
                break;
            }
            case ObjectInspectorField::Salida:
                beginExitPicker(inspector);
                break;
            case ObjectInspectorField::AnadirLoot:
                beginLootPicker(inspector);
                break;
            case ObjectInspectorField::LootItem:
                data.notice = "X QUITA ESTA ENTRADA DE BOTIN";
                break;
            case ObjectInspectorField::PropiedadItem:
                data.notice = "X QUITA ESTA PROPIEDAD";
                break;
        }
        return true;
    }
};

}  // namespace Editor
