# ADR-003: El runtime conserva overrides de mapa por partida

**Estado:** Aceptado  
**Fecha:** 2026-08-28  
**Decisor:** Proyecto MotorGrafico

## Contexto

DOC-03 separa el `MapDocument` editable del `RuntimeTileMap` optimizado. Un
juego necesita cambiar tiles durante una partida —abrir una puerta, destruir
un muro o activar un puente— sin convertir una acción de jugador en una
reescritura del contenido que creó el autor.

## Decisión

`RuntimeTileMap` conserva el resultado base del builder y admite
`RuntimeTileOverride` identificados por capa y celda. Un override solo vive en
la sesión: `clearTileOverrides()` restaura los datos compilados y el
`MapDocument` nunca se modifica. El sistema de guardado persistirá estos
overrides como estado de partida cuando se extienda su esquema.

## Opciones consideradas

### A. Mutar el MapDocument desde runtime

**Ventaja:** no requiere un estado adicional.

**Desventajas:** mezcla contenido de autoría con progreso de juego, dificulta
deshacer cambios y puede sobrescribir archivos compartidos o de solo lectura.

### B. Overrides de sesión sobre un documento inmutable (elegida)

**Ventajas:** separa contenido y partida, es reversible y permite guardar solo
los cambios; el builder sigue siendo puro.

**Desventajas:** se mantienen la base y las modificaciones en memoria; el
guardado necesita serializar la lista de overrides de manera versionada.

## Consecuencias

- `MapDocument` sigue siendo la fuente de verdad portable y sin estado de
  juego.
- `RuntimeTileMapBuilder` copia únicamente capas de tiles y sus metadatos de
  visibilidad/opacidad; entidades y regiones permanecen documentales.
- `TileMap` sigue importando TMX como adaptador temporal, pero renderer,
  minimapa y sesión consumen la abstracción de runtime.
- La propiedad de colisión de un tileset aún procede del adaptador TMX; la
  futura ficha de tileset documental deberá aportarla al builder.
