# Sesión 2026-09-20 — opencode — H0-05: ranuras de guardado en Application

Sigue el mandato de Oriol 20/09: «coge el gantt del motor gráfico y sigue programando; utiliza MAPA_DEL_CODIGO, pero piensa que hay más agentes trabajando de manera simultánea».

## Contexto y reglas cumplidas
- Inner repo `MotorGraphico-main/MotorGraphico`, rama `feature/editor-s1-contratos`, HEAD `05eb13b` (E-13). Build propio NUEVO `out/build/opencode-h005` (no toco los build dirs de otros agentes; reconfigurado desde cero — nunca confío en binarios viejos).
- Fila reclamada en TABLERO ANTES de tocar ficheros (regla 1). Sin commit (sin luz verde; CMakeLists de otros). Zonas ajenas no tocadas: `SaveSlotState.*` (G-11), tests GL-free que compilan ese .cpp a mano, el resto de CMakeLists, GameSession/editor/D-11/etc.
- MAPA_DEL_CODIGO usado para decidir la rodaja y respetar los paquetes: `Application.cpp/.h` = fichero de cruce de 14 paquetes; `SaveSlotState.cpp/.h` = G-11.

## Decisión de rodaja (del Gantt Fase H, ítems red)
Del Gantt Fase H (qwen) quedaban libres: H0-02 (MenuLeft/Right inventario) y H0-04 (acción Desequipar) — **ya cableados** en Application (MenuLeft/Right ~l.1828-1835, `m_session->unequipItem` en confirmInventoryCommand); H0-01/H0-06 bloqueados (MapEventRunner/D-11); **H0-05 (ranuras de guardado en F5/F9 y en «Cargar partida») era el hueco real**: Application seguía escribiendo `m_savePath = "savegame.json"` y la Pendiente 36 que decía «SaveSlots F5/F9 ya versionados y verificados» estaba **incorrecta** (el servicio G-11 estaba versionado, pero nada lo usaba desde la ejecución).

## Qué se hizo
1. `CMakeLists.txt`: lib GL-free nueva `motor_slots` = `src/Game/SaveSlotState.cpp`; enlazada `PUBLIC` en `motor_core`. Nacida aparte de motor_level a propósito: los tests GL-free (SaveSlotsTests, SessionCommandHostTests, TitleFlowTests) ya compilan ese .cpp dentro de SU ejecutable, y si entrara en motor_level verían el símbolo dos veces. Las libs GL-free no la ven.
2. `include/Engine/Application.h`:
   - `#include "Game/SaveSlotState.h"`.
   - Miembros `std::unique_ptr<Game::SaveSlots> m_saveSlots` + `std::string m_projectId` (vacía = raíz, juego suelto).
   - Métodos privados `setupSaveSlots`, `quickSave`, `quickLoad`, `loadSlot`, `newestLoadableSlot`, `restoreSave`.
3. `src/Engine/Application.cpp`:
   - `init`: `setupSaveSlots()` tras el HUD compuesto (siempre disponible).
   - `loadGame(path)` refactorizado: el tramo de restaurar (`requestReplace` → `loadLevel(..., &data)` + `applyNarrativeState`) pasa a `restoreSave(data)`; `loadGame` solo lee el fichero y delega. Comportamiento idéntico (cubierto por C-24 ApplicationTests intacto).
   - `quickSave`: `m_saveSlots->save(kQuickSlot, captureSaveGame(m_currentLevelPath))` + log de fallo/ok.
   - `loadSlot(index)` → `m_saveSlots->load` + `restoreSave`; `quickLoad()` = `loadSlot(kQuickSlot)`.
   - `newestLoadableSlot()` → ranura enumerada `loadable()` con `savedAt` mayor (refresca antes).
   - Cableado: **F5/F9** (explotación) → `quickSave()/quickLoad()`; **Pausa Save/Load** → `quickSave()/quickLoad()`; **«Continuar»** (título) → `quickLoad()` con hint «No hay partida en la ranura rapida (F5)»; **«Cargar partida»** → `loadSlot(newestLoadableSlot())` (TODO del selector G-10 se queda) con hint si no hay ninguna; **openProject** → `m_projectId = project.id; setupSaveSlots();` (cada proyecto guarda en `saves/<proyecto>/`, no se pisan).
4. `tests/Engine/ApplicationCableadoTests.cpp` (GL): accesos `ApplicationTestAccess` para `m_saveSlots`/`setupSaveSlots(projectId)`/`quickSave`/`quickLoad`/`loadSlot`/`newestLoadableSlot`/`m_currentLevelPath`. Tres casos nuevos:
   - `cableado_f5_usa_ranura_rapida`: quickSave → `slots.quick().occupied()`, path `saves/rapido.json`, `gold == baseline+17`, quickLoad encola carga válida.
   - `cableado_cargar_partida_elige_ranura_enumerada`: se guarda una partida real solo en la ranura 3 → `newestLoadableSlot()==3` y `loadSlot(3)` ok.
   - `cableado_ranuras_separadas_por_proyecto`: proyecto A y B guardan en carpetas distintas sin pisarse.
   - Limpieza con `<filesystem>` de los ficheros/dirs de prueba.

## Verificación (reales)
- Reconfiguración desde cero `-S . -B out/build/opencode-h005` exit 0 («Suite GL-free: 102 objetivos»).
- `test_application_cableado` **7/7 PASS** (4 previos + 3 nuevos) por binario y por `ctest` (labels unit;gl).
- `test_input_system` 5/5 PASS (regresión del binding Pause→Escape intacto).
- `test_aventuras_monsterid` (python, ctest) Pass.
- `juego` enlaza OK con motor_core→motor_slots→motor_level. Aviso benigno del linker: *ignoring duplicate libraries* (motor_level llega por dos vías: directa del exe y vía motor_slots). Candidato a limpieza futura, no bloqueante.
- `test_application` (C-24 completo) Sigue en rojo por el WIP de D-11 (`ActiveCondition`, ya diagnosticado en la sesión anterior, no es regresión mía); ApplicationTests compila en cualquier caso (Application.cpp se compila y enlaza en mi target).

## Hallazgos registrados
- **Pendiente 36 corregida**: el cableado SaveSlots→F5/F9/«Cargar partida» NO existía (ahora existe). El texto previo trataba el servicio versionado como si ya estuviera al alcance del jugador.
- **CMakeLists.txt es un cruce congestionado**: mis hunks (motor_slots + línea motor_core) conviven con WIP ajeno sin commitear de la tanda del Pendiente 32 (test_entity/test_player/test_isometric_renderer de G-09; `include(cmake/ExportarTests.cmake)` de G-06/TRAE; `include(cmake/ValidarTodoTests.cmake)` de D-10/INF-06). Quien consolide el commit debe decidir cómo separarlos (todos los `.cmake` citados existen en el árbol).
- Primer fallo de test arreglado: `test_level` arranca con oro base ≠ 0; la aserción pasaba por comparar contra baseline, no contra 17 literal.

## Siguiente
- H0-07 (título desde `TitleMenuState::fromManifest`) y G-12 vista (`swap InventoryScreen→InventoryMenuState`) siguen libres en Application, misma frontera de cruce; pedir fila propia.
- El aviso de libs duplicadas puede limpiarse (linkar motor_level una sola vez) en una sesión que toque CMakeLists sin WIP ajeno encima.