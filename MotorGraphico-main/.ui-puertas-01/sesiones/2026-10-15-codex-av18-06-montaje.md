# AV18-06 — revisión del montaje de Claude

- **Fecha:** 2026-10-15 (Europe/Madrid).
- **Autor:** Codex; implementación de Claude, autorizada por Oriol para revisar y commitear.
- **Rama:** agente/codex/AV18-06, worktree ~/onegai-ramas/codex-AV18-06.
- **Commits:** datos `3f9ea7e`; documentación `1d78915`; montaje `ff64e5c12fe3a8675000d712819f22e7f8d72729` (Oriol, coautor Claude).
- **Deja esperando a Oriol:** nada nuevo; sin push ni merge.

Se leyó la bitácora completa, AGENTS, handoff, protocolo/plan de la cola y tablero. Se reclamó la revisión y se activó hooksPath mediante git-safe. Al llegar había seis ficheros de Claude sin commit; durante la compilación apareció ff64e5c con exactamente esos seis. No se duplicó el commit ni se reescribió ningún fichero de Claude. El documento FORMATO_ACCIONES_MONSTRUOS.md no estaba modificado inicialmente y llegó durante la verificación: se revisó y commiteó intacto en 1d78915 con el mensaje solicitado. Los metadatos de datos/validador aún anuncian la IA pendiente; se registra sin tocar esa parte. El árbol principal tiene abundante WIP de otros agentes: se conserva y solo se actualizan las secciones propias de coordinación.

Revisión: cursor independiente por enemigo, selección nativa cuando no hay habilidad legada pagable, tres ganchos actuales, duración de estados sobre el siguiente turno del héroe y reutilización de objetivo/reacciones/DANO-02. Sin defecto de producto nuevo observado. Los dos casos añadidos verifican efectos y alternancia de saqueador y Gorran; no se afirma cobertura exhaustiva de todas las combinaciones.

Desde MotorGraphico-main/MotorGraphico del worktree:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j8
ctest --test-dir build-release -L gl-free --output-on-failure
```

Release completa OK. Suite **235/237**, 72,61 s. Fallan únicamente linea_base_core_configura (49 instantáneos) y linea_base_core (49 instantáneos/44 deuda), mismo diagnóstico documentado antes del montaje. test_cartas_combate (incluidos los dos casos de Claude), test_monstruos_acciones, test_validar_monstruos_acciones y test_validar_todo pasan. No se arregla ninguna parte de combate. Logs de esta ejecución: /tmp/av18-06-configure.log, /tmp/av18-06-build.log y /tmp/av18-06-ctest.log.

INFORME AV18-06 a «hecho»; fila movida a Hecho; integración ya no espera a Claude para el repertorio. Continúa AV18-04 con el recorrido de recompensas y calibración sin certificar perfiles antes de contrastar el reparto real.
