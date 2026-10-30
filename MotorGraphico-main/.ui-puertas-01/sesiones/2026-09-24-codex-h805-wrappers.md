# Sesión — H8-05: retirada de cabeceras envoltorio

Fecha: 2026-09-24  
Agente: Codex

## Alcance reclamado

Retirar únicamente las tres rutas de compatibilidad ya auditadas:

- `MotorGraphico/examples/Check.h`
- `MotorGraphico/include/Editor/ProjectIndex.h`
- `MotorGraphico/include/Render/Tile.h`

No se toca `Application.*`, CMake ni ningún WIP de otros agentes.

## Mapa consultado

Se consultó `MotorGraphico/planificacion/MAPA_DEL_CODIGO.md` antes de editar:

- C-19 sitúa `include/Render/TileMap.h` en el contrato de culling; solo se
  cambió su ruta de include, no su API ni lógica.
- E-11 sitúa `include/Project/ProjectIndex.h` y su implementación canónica;
  `level_editor` conserva la misma API y sólo deja de usar el wrapper.
- El inventario completo aún enumeraba los tres wrappers junto a
  `RuntimeTile`; no se regeneró el mapa para no mezclar WIP ajeno.

## Resultado

- Los 46 consumidores de `Check.h` incluyen ahora `Core/Check.h`.
- `level_editor` y `demo_proyectos` incluyen `Project/ProjectIndex.h`.
- `TileMap` y `demo_tilemap` usan `Runtime/RuntimeTile.h` y `RuntimeTile`.
- Las tres cabeceras envoltorio fueron eliminadas. No quedan referencias a sus
  rutas históricas.

## Verificación aislada

- Configuración CMake Debug con tests, en `out/build/codex-h805`, sin usar el
  build compartido.
- Compilan `demo_tilemap`, `demo_proyectos`, `level_editor`, `juego` y
  `motor_editor`, y los 45 demos alcanzables migrados.
- `demo_tilemap` y `demo_proyectos` se ejecutan correctamente.
- Pasan 6/6: `test_project_levels`, `test_project_naming`, `test_project`,
  `test_project_catalog`, `test_runtime_tilemap`, `test_map_round_trip`.
- `demo_mision_llave` no se registra porque su fragmento exige `motor_slots`,
  inexistente en esta configuración; su fuente sí pasa compilación sintáctica
  con `Core/Check.h`.
- Pasan `python3 tools/linea_base_core.py --rapido` y `git diff --check`.

## Handoff

Las rutas canónicas son definitivas. Cualquier consumidor nuevo debe incluir
`Core/Check.h`, `Project/ProjectIndex.h` o `Runtime/RuntimeTile.h` de forma
directa; no reintroducir wrappers ni aliases de compatibilidad.
