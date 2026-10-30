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

// Cualquier objeto con logica dependiente del tiempo (movimiento, IA,
// animacion...). Ver comentario de IRenderable sobre por que son dos
// interfaces separadas.
class IUpdatable {
public:
    virtual ~IUpdatable() = default;

    virtual void update(float deltaTime) = 0;
};
