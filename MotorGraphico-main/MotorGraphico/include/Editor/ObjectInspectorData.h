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

// Estado GL-free de una sesión de ObjectInspector.
//
// El controlador mantiene las referencias al documento y al catálogo; estos
// datos representan únicamente lo que el autor está editando en el panel.

#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"

namespace Editor {

inline constexpr std::array<float, 5> kObjectInspectorScales = {0.5f, 0.75f, 1.0f, 1.5f, 2.0f};

inline std::string objectInspectorScaleLabel(float scale) {
    return std::to_string(static_cast<int>(scale * 100.0f + 0.5f)) + "%";
}

inline std::string objectInspectorScenarioName(const std::string& path) {
    const std::size_t slash = path.find_last_of('/');
    std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
    const std::size_t dot = name.find_last_of('.');
    if (dot != std::string::npos && dot > 0)
        name = name.substr(0, dot);
    return name.empty() ? path : name;
}

enum class ObjectInspectorField {
    NombreVisible,
    Variante,
    Escala,
    Efecto,
    Salida,
    PosicionDestino,
    LootItem,
    AnadirLoot,
    LootTabla,
    Evento,
    PropiedadItem,
    AnadirPropiedad
};

enum class ObjectInspectorMode { Navegar, Texto, ElegirSalida, ElegirLoot };

struct ObjectInspectorRow {
    ObjectInspectorField field;
    int index = -1;
};

struct ObjectInspectorData {
    bool open = false;
    GridCoord cell{0, 0};
    std::size_t selected = 0;
    ObjectInspectorMode mode = ObjectInspectorMode::Navegar;
    ObjectInspectorField textField = ObjectInspectorField::NombreVisible;
    std::string buffer;
    std::vector<std::string> destinations;
    std::vector<std::string> options;
    std::string textFilter;
    std::size_t pickerMark = 0;
    std::vector<std::string> baseLoot;
    std::vector<ObjectInspectorRow> rows;
    std::vector<std::string> lines;
    std::string notice;
};

}  // namespace Editor
