# TABLERO — trabajo cooperativo (4 IAs + Oriol)

<!-- REGLAS (protocolo; ver decisiones/D0001):
  1. ANTES de crear el primer fichero de una tarea: escribe tu fila en «En curso»
     con «Tomada por» y «Desde». Sin fila, la tarea está libre.
  2. No toques rutas de una fila ajena viva; las zonas por prefijo (on_/os_/en_/es_,
     bnd_…) siguen mandando igual que en el CONTRATO del experimento.
  3. Al terminar: mueve la fila a «Hecho» enlazando tu fichero de sesiones/.
  4. «Esperando a Oriol» es SU bandeja: sólo él la vacía, aceptando o rechazando
     (editando Estado en decisiones/DNNNN, o decidiendo el asunto de la fila).
  5. Lo editan agentes Y Oriol. Nunca se borra la fila de otro: se comenta.
  Sembrado el 28/08/2026 desde la Parte 0 de la BITACORA (congelada ese día).
-->

## En curso
| Tarea | Tomada por | Desde | Zona (rutas / prefijo) | Entrega verificable |
|---|---|---|---|---|
| D-10: CI valida los assets reales (completar integración en CI) | claude | 17/09 | `../.github/workflows/ci.yml`, `tools/validar_catalogos.py`, `tools/linea_base_catalogos.json`, `tools/validar_catalogos_ci.py`, `tests/RPG/CatalogContentTests.cpp` (compartido con D-02). Sin tocar catálogos ni loaders de datos. | CI ejecuta `validar_catalogos_ci.py` en cada commit, carga catálogos reales de `assets/` y falla si una entrada pierde campos mecánicos respecto a la línea base registrada. |
| E-15: panel ImGui de problemas y busqueda (mitad interfaz) | qwen | 17/09 | `editor/src/ProblemsPanel.cpp`, `editor/include/ProblemsPanel.h`, `cmake/MotorEditor.cmake` (solo linea del target motor_editor). Sin tocar `EditorApplication.cpp` (E-10), `level_editor.cpp` (cruce), `ProblemsState.h/.cpp` (ya versionado). | Panel con 3 pestanas (Problemas, Buscar, Referencias), barra de estado con canPlaytest, testPoint para humo. Integracion en EditorApplication pendiente del pase de integracion. Build bloqueado por WIP E-16 (ProjectDatabaseState.cpp firma rota). |
| E-08: depuración paso a paso (núcleo GL-free del intérprete; sin UI de editor) | zcode | 17/09 | `include/RPG/EventInterpreter.h`, `src/RPG/EventInterpreter.cpp`, `tests/RPG/EventInterpreterTests.cpp` | `test_event_interpreter`: modo paso a paso; el estado tras N pasos es igual que ejecutar N de golpe |
| Diagnóstico de 767 mensajes Java de VS Code (dndWeebCC) | codex | 14/09 | Lista adjunta, configuración Java/Maven y compilación/tests en copia aislada; sin editar pom.xml ajeno ni reglas/cartas. | Identificar causas de los 675 errores y separar avisos del IDE de fallos reproducibles. |
| E-13: navegador de recursos (mitad sin interfaz) | qwen | 14/09 | `include/Editor/AssetBrowserState.h`, `src/Editor/AssetBrowserState.cpp`, `tests/Editor/AssetBrowserStateTests.cpp`, `cmake/AssetBrowserStateTests.cmake`, seguimiento/E-13 y entradas propias de dades/MAPA. Sin tocar ProjectSerializer, level_editor, EditorApplication, EditorState ni paneles ImGui (zonas E-12/E-10). | `test_asset_browser_state`: búsqueda por nombre/tipo con acentos plegados (DEC-15, reutiliza NormalizeForSearch), selección por identidad que sobrevive al refresco, dimensiones PNG leídas sin GL, e importación que genera referencia relativa estable y NUNCA sobreescribe (colisión → reutilizar o cancelar). GL-free. |
| F1: reserva liberada por Oriol el 07/09; trabajo previo preservado | zcode (reserva finalizada) | 19/08 | Sin exclusividad; Application/GameSession pasan a C-16 | `demo_timers_partida` verde (congelación por diálogo, expiración por tiempo/zona, aviso por umbral) |
| Estudio del proyecto + guía de build desde Visual Studio/VS Code (estado real del build verificado) | kimi | 14/09 | Documentación (`GUIA_VISUAL_STUDIO.md`, raíz), `sesiones/` y verificación en `MotorGraphico-main/MotorGraphico/out/build/macos-debug` (solo lectura de código ajeno; sin tocar WIP en vuelo de opencode/codex/qwen/zcode). | Guía paso a paso actualizada + build/ctest verificados localmente con el preset macos-debug. |