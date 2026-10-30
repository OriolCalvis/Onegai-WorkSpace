# ADR-005: JSON canónico, legible y atómico para documentos

**Estado:** Aceptado  
**Fecha:** 2026-08-28  
**Decisor:** Proyecto MotorGrafico

## Contexto

Los proyectos y mapas deben ser editables por autores, revisables en Git y
cargarse sin acoplarse al runtime. El motor ya contiene `JsonValue` y un
escritor propio GL-free; introducir otra biblioteca de JSON duplicaría la
superficie de dependencias sin resolver el contrato de orden, versiones y
escritura segura.

## Decisión

`ProjectSerializer` usa el parser y escritor propios para formatos v1 de
proyecto y mapa. Emite claves en orden fijo, valida antes de escribir y guarda
en `<ruta>.tmp` antes de reemplazar atómicamente el destino. Cada esquema se
publica como JSON Schema revisable en `schemas/`.

## Consecuencias

- Dos serializaciones del mismo documento producen los mismos bytes.
- Los errores de parseo, versión y campos se devuelven mediante `Result` sin
  modificar el destino ni entregar objetos a medio cargar.
- `ProjectIndex` y los manifiestos legacy permanecen como adaptadores hasta
  que todos los packs migren al formato canónico.
- En Windows se usa reemplazo nativo con `MoveFileEx`; en POSIX, `rename`.
