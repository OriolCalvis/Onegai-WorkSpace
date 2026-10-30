# 07/09/2026 — Codex: primer lote multiagente del Gantt

- **Commits:** MotorGraphico `be527ec` (10 archivos propios).
- **Deja esperando a Oriol:** nada nuevo; CI remota pendiente y discrepancia ADR-003/C-22 registrada.

## Encargo y coordinación

Oriol pide programar las tareas del Gantt con múltiples agentes. Se sigue el corte B y su prioridad de consolidación. Leídos protocolo, histórico mínimo, propiedad 2.1, trampas Parte 3, TABLERO y handoff. Filas reclamadas antes de crear archivos. Coordinador Codex: auditoría C-11, integración, build, CI y registro. Agente runtime: C-22. Agente controles: verificador tras INF-03 y ayuda del editor.

## Entrega

Commit MotorGraphico **be527ec** sobre `feature/editor-s1-contratos` (git-safe, rutas explícitas):

- C-22: retirados overrides sin consumidores y almacenamiento duplicado de tiles. Conservados builder, consultas, geometría, límites y destructor virtual; pruebas de independencia entre documento y snapshots.
- Verificador del editor: ruta `editor/src`, diagnóstico de lectura, detección de controles fantasma y comprobación de identidades de herramientas. Ayuda TAB corregida a 3 (acción ya existente), SPACE documentada; sin alterar comportamiento gráfico.
- CI registra verificador y sus 13 regresiones Python.
- C-11: las 12 huellas del registro de integración coinciden con HEAD; el parche completo, incluido CMake, pasa `git apply --reverse --check --cached`. El rescate 9e40405/52d696f ya había versionado la integración. Seguimiento corregido; no se reaplica el parche.

## Verificación

- `cmake --preset macos-debug` y build completa: exit 0 (incluidos juego y level_editor). Recompilación del editor tras corregir ayuda: exit 0.
- `ctest --test-dir out/build/macos-debug --output-on-failure -L gl-free`: **62/62**, 0 fallos.
- Runtime aislado con C++17, `-DNDEBUG -O2`: **4/4**, comprobaciones activas en Release.
- `python3 tools/test_verificar_controles.py`: **13/13**; `python3 tools/verificar_controles.py`: 55 teclas / 7 herramientas, exit 0.
- Diff revisado y `git diff --check` sin errores. La prueba de controles es estática, no una aceptación visual de E-10/G-16.
- No se ha hecho push ni ejecutado CI remota: C-11/C-22 siguen pendientes de ese cierre formal.

## Trabajo previo preservado y límites

Se encontraron modificaciones previas en CMakeLists, EntityLayerDocument, MapDocument, ProjectSerializer, pruebas Map/RoundTrip y salidas generadas del plan; también LevelDocumentBridge y sus pruebas sin versionar, registros de seguimiento C-01..14/C-19, auditoría y residuos wt_*.json/map_*.json/FUSE/duplicados. Ninguno se incorpora al commit ni se revierte. La suite integrada se ejecuta sobre ese workspace; no equivale a CI de un checkout limpio. Los renders/markdown generados del plan ya contenían trabajo ajeno: no se regeneran ni se incluyen; los nuevos estados viven en los JSON de seguimiento versionados.

GameSession/Application y temporizadores siguen reservados a zcode; no se tocan. No se abre fase D/E ni se marcan paquetes dependientes como completados.

Discrepancia registrada: ADR-003 (28/08) acepta overrides de runtime; C-22 del plan posterior manda retirarlos. Esta entrega ejecuta explícitamente C-22 del Gantt encargado por Oriol. No se reescribe el ADR aceptado ni se cambia canon; queda pendiente armonizar ese documento histórico con el plan vigente.
