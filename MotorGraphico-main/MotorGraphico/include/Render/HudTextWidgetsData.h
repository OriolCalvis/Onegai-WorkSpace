// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-14  (fase G, 3 j) - HUD, diàlegs i comerç amb accions visibles
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

#include "Core/Math/Vector4.h"
#include "Render/HudElement.h"

// Estado GL-free de los widgets de texto. BitmapFont y Texture se quedan
// fuera porque son colaboradores de render no propietarios; estos datos
// describen únicamente lo que el usuario ve y navega.

struct HudTextData {
    explicit HudTextData(const HudTransform& initialTransform) : transform(initialTransform) {}

    HudTransform transform;
    std::string text;
    std::string renderText;
    Vector4 color{1.0f, 1.0f, 1.0f, 1.0f};
};

// AV1-03 — Una fila ya envuelta de HudCommandMenu: el texto TAL COMO SE
// DIBUJA (con el prefijo del cursor, y con la sangria de dos espacios en
// las filas de continuacion) y a que OPCION pertenece. Guarda el indice
// de la opcion y no el de la fila porque una opcion larga ocupa varias
// filas y todas comparten su color de seleccion: sin esto, envolver una
// etiqueta larga dejaria la mitad del menu resaltada y la otra no.
struct HudMenuRow {
    std::string text;
    std::size_t optionIndex = 0;
};

struct HudCommandMenuData {
    explicit HudCommandMenuData(const HudTransform& initialTransform)
        : transform(initialTransform) {}

    HudTransform transform;
    std::vector<std::string> options;
    std::size_t selectedIndex = 0;
    Vector4 textColor{0.85f, 0.85f, 0.85f, 1.0f};
    Vector4 highlightColor{1.0f, 0.9f, 0.2f, 1.0f};
};

struct HudDialogueBoxData {
    HudDialogueBoxData(const HudTransform& initialTransform, std::size_t initialMaxLines)
        : transform(initialTransform), maxLines(initialMaxLines) {}

    HudTransform transform;
    std::size_t maxLines;
    std::vector<std::string> visibleLines;
    Vector4 backgroundColor{0.05f, 0.05f, 0.08f, 0.85f};
    Vector4 textColor{1.0f, 1.0f, 1.0f, 1.0f};
};
