# ADR-002: Los tipos de capa de mapa son inmutables

**Estado:** Aceptado  
**Fecha:** 2026-08-28  
**Decisor:** Proyecto MotorGrafico

## Contexto

El editor actual solo conserva capas de tiles en `EditorState`; las entidades
pertenecen al nivel y las regiones aún no tienen modelo. DOC-02 introduce
`MapDocument` como fuente de verdad editable, con capas de tiles, entidades y
regiones independientes del renderer. Había que decidir si una capa podía
cambiar su tipo después de creada.

Un cambio de tipo en sitio no tiene una semántica segura: una capa de tiles
puede contener miles de GIDs, una de entidades referencias de catálogo y una
de regiones límites y metadatos. No existe una conversión general que no pueda
perder información ni requerir intervención del autor.

## Decisión

`MapLayer` guarda sus datos en una variante cerrada de
`TileLayerDocument`, `EntityLayerDocument` o `RegionLayerDocument`. El tipo se
deduce de esa variante y no existe una API que la sustituya; tras crear una
capa, solo se puede consultar su contenido tipado.

Para convertir una capa se creará otra capa con un ID nuevo y se migrará el
contenido mediante una operación explícita de editor. La futura operación
deberá informar los elementos que no pueda convertir y actualizar referencias
en una única transacción de documento.

## Opciones consideradas

### A. Cambiar el tipo y reutilizar el mismo ID

**Ventaja:** interfaz de usuario aparentemente más rápida.

**Desventajas:** borra o reinterpreta datos sin contrato, complica undo/redo y
oculta una migración que puede dejar referencias inválidas.

### B. Capa genérica con campos opcionales

**Ventaja:** estructura inicial sencilla.

**Desventajas:** permite estados imposibles —por ejemplo GIDs y regiones en la
misma capa— y obliga a cada consumidor a revalidar combinaciones de campos.

### C. Variante cerrada e inmutable (elegida)

**Ventajas:** los estados válidos son explícitos, la validación se hace una vez
en el documento y el runtime puede despachar por tipo sin punteros a GPU.

**Desventajas:** la conversión necesita crear y revisar una nueva capa; añadir
un cuarto tipo exige actualizar la variante y los visitantes asociados.

## Consecuencias

- Las capas tienen ID, nombre, visibilidad, bloqueo, opacidad y orden
  determinista; IDs y órdenes son únicos dentro de un mapa.
- `MapDocument` se mantiene separado de `Render::TileMap` y de la UI.
- DOC-03 podrá compilar únicamente las capas de tiles hacia runtime, mientras
  que entidades y regiones seguirán disponibles como datos de autoría.
- DOC-04 sustituirá los strings de identidad por IDs fuertes sin cambiar esta
  frontera documental.
