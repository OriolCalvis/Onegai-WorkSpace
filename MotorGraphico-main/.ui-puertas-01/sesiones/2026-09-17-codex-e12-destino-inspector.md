# E-12 — Validación de llegada de puertas en el inspector

- **Fecha:** 2026-09-17
- **Autor:** Codex
- **Proyecto:** MotorGraphico (ejecución)
- **Commits:** MG `d5ec681`.

## Entrega

Tras consultar BITACORA, TABLERO, `AGENT_HANDOFF.md`, Gantt fuente y
`MAPA_DEL_CODIGO.md`, se reclamó una rodaja aislada de E-12 antes de editar.

`ObjectInspector` ya permitía introducir manualmente la casilla de llegada de
una puerta, pero aceptaba coordenadas negativas y valores que podían truncarse
de `long` a `int`. Ahora rechaza ambos sin modificar la llegada válida que ya
tenía el objeto. La cota superior dependiente del mapa de destino se mantiene
para el flujo que conoce sus dimensiones.

La regresión se añade al demo GL-free existente: tras configurar `7,1`, editar
a `-1,3` conserva `7,1` y muestra el aviso de validación. La reserva queda
liberada en el tablero.

## Verificación

`cmake --build out/build/macos-debug --target demo_object_inspector -j2`

`ctest --test-dir out/build/macos-debug --output-on-failure -R '^demo_object_inspector$'`

Resultado: 1/1 verde. Durante la reconstrucción apareció un warning ajeno en
`src/Game/InventoryScreen.cpp` por el caso `COUNT` no manejado; no se tocó.

## Concurrencia

El cambio se limita a `src/Editor/ObjectInspector.cpp` y
`examples/demo_object_inspector.cpp`. Se preservaron los WIP de E-13, E-07,
EVT-05, G-13 y los cambios de Application/GameSession. La reserva queda
liberada al cerrar esta sesión.
