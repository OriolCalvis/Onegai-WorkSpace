// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-19  (fase C, 2 j) - Un sol culling i un sol contracte d'actualització
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <optional>
#include <vector>

#include "Core/Math/GridCoord.h"

class IRenderable;

// Estado GL-free de la parte de escena del renderer. Los renderables de
// la cola no son propiedad del renderer, igual que antes de este relevo.
struct IsometricSceneData {
    std::vector<IRenderable*> renderQueue;
    std::optional<GridCoord> occlusionFocus;
    bool occlusionEnabled = true;
    float occlusionAlpha = 0.40f;
};
