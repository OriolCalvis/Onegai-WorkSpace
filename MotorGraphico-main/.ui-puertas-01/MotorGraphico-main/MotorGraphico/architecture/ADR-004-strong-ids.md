# ADR-004: IDs legibles, fuertes e inyectables

**Estado:** Aceptado  
**Fecha:** 2026-08-28  
**Decisor:** Proyecto MotorGrafico

## Contexto

El contrato existente de contenido fija IDs ASCII en minúsculas con guiones
bajos. Sin embargo, los documentos usaban `std::string` intercambiables: una
referencia a mapa podía llegar a una API de assets sin que el compilador lo
detectara. También se necesitaba generar IDs reproducibles en pruebas y en
herramientas de editor.

## Decisión

Se usa `ids::StrongId<Tag>` con aliases distintos para proyecto, documento,
mapa, capa, asset, entidad, región y evento. Los IDs se crean con `parse()`,
que aplica `[a-z0-9_]+`, y no admiten conversiones implícitas entre tipos.

La generación se recibe mediante `ids::IdGenerator`; el documento no conoce
UUID, reloj ni una fuente global de aleatoriedad. Esta decisión sigue el
contrato vigente de IDs legibles y permite usar secuencias deterministas en
tests.

## Opciones consideradas

### A. UUID aleatorio o UUID v7

**Ventaja:** generación global sencilla y baja probabilidad de colisión.

**Desventajas:** contradice los IDs legibles ya usados como claves, rutas y
argumentos de herramientas; añade una estrategia de codificación sin resolver
la confusión de dominios en C++.

### B. Strings sin tipo

**Ventaja:** migración inmediata nula.

**Desventajas:** deja pasar asignaciones entre dominios y reparte la validación
por cada cargador, editor y herramienta.

### C. IDs legibles con `StrongId<Tag>` (elegida)

**Ventajas:** compatibilidad con el contrato, errores de tipo en compilación,
serialización directa a texto y generador controlable por el llamador.

**Desventajas:** las APIs documentales deben migrarse gradualmente y cada
conversión desde JSON/TMX debe comprobar `Result`.

## Consecuencias

- `MapDocument`, sus capas, entidades, regiones y los overrides de runtime
  usan dominios de ID distintos.
- `DocumentRef` conserva ID estable al cambiar su ruta lógica; el test cubre
  un movimiento de fixture sin romper la referencia.
- `Project` y manifiestos legacy siguen exponiendo strings mientras DOC-05
  introduce el serializer canónico; no se reescriben todos los packs en esta
  migración de tipos.
- Los adaptadores externos, como TMX, generan IDs internos válidos y
  deterministas antes de entrar al runtime.
