// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-23  (fase C, 4 j) - El post-processat arriba al joc
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include "Core/Math/Vector4.h"

class ColorLUT;
class FogOfWar;
class Shader;
class Texture;

// Configuración GL-free de cada efecto. Los recursos son colaboradores no
// propietarios; los Framebuffer que exigen un contexto GL siguen en el
// renderer, que es quien controla su ciclo de vida.
struct IsometricLightingData {
    Shader* shader = nullptr;
    Texture* lightmap = nullptr;
    Vector4 ambientColor{0.35f, 0.35f, 0.4f, 1.0f};
    bool enabled = false;
};

struct IsometricFogData {
    Shader* shader = nullptr;
    FogOfWar* fog = nullptr;
    Vector4 color{0.05f, 0.07f, 0.12f, 1.0f};
    bool enabled = false;
};

struct IsometricLutData {
    Shader* shader = nullptr;
    ColorLUT* lut = nullptr;
    bool enabled = false;
};

struct IsometricShadowData {
    Texture* texture = nullptr;
    bool enabled = false;
};

struct IsometricPostProcessData {
    IsometricLightingData lighting;
    IsometricFogData fog;
    IsometricLutData lut;
    IsometricShadowData shadows;
    Vector4 sceneClearColor{0.0f, 0.0f, 0.0f, 1.0f};
    int viewportWidth = 0;
    int viewportHeight = 0;
};
