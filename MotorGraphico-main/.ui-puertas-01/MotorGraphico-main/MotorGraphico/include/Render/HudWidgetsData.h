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

// Estado GL-free de HudBar y HudPanel.
//
// Los widgets conservan únicamente Texture como colaborador de render no
// propietario. Todo lo que describe su geometría, valor y apariencia vive
// aquí para que el estado visual no infle sus controladores OpenGL.

#pragma once

#include "Core/Math/Vector4.h"
#include "Render/HudElement.h"

struct HudBarData {
    explicit HudBarData(const HudTransform& initialTransform) : transform(initialTransform) {}

    HudTransform transform;
    float value = 1.0f;
    float maxValue = 1.0f;
    float displayedValue = 1.0f;
    float animationSpeed = 0.0f;
    float borderThickness = 0.0f;
    bool useThresholds = false;
    Vector4 backgroundColor{0.15f, 0.15f, 0.18f, 0.85f};
    Vector4 fillColor{0.8f, 0.15f, 0.15f, 1.0f};
    Vector4 borderColor{0.85f, 0.85f, 0.9f, 0.9f};
    Vector4 ghostColor{1.0f, 1.0f, 1.0f, 0.55f};
    Vector4 thresholdHigh{0.25f, 0.75f, 0.3f, 1.0f};
    Vector4 thresholdMid{0.9f, 0.75f, 0.2f, 1.0f};
    Vector4 thresholdLow{0.85f, 0.2f, 0.15f, 1.0f};
};

struct HudPanelData {
    HudPanelData(const HudTransform& initialTransform, const Vector4& initialColor)
        : transform(initialTransform), color(initialColor) {}

    HudTransform transform;
    Vector4 color;
    Vector4 borderColor{0.85f, 0.85f, 0.9f, 0.9f};
    float borderThickness = 0.0f;
};
