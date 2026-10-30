# Sesión 2026-09-22 — opencode — H0-07: el título sale del manifiesto (+ reconstrucción del cableado que un reset ajeno borró)

Sigue el mandato de Oriol 20/09: «coge el gantt del motor gráfico y sigue programando; utiliza MAPA_DEL_CODIGO, pero piensa que hay más agentes trabajando de manera simultánea».

## Contexto y reglas cumplidas
- Inner repo `MotorGraphico-main/MotorGraphico`, rama `feature/editor-s1-contratos`, HEAD al llegar `91ebf1b`, HEAD al terminar `11c6b0e` (EST-34 de Codex). Build propio NUEVO `out/build/opencode-h007` (no toco los build dirs de otros; reconfigurado desde cero).
- Fila reclamada en TABLERO ANTES de tocar ficheros (regla 1). Sin commit (sin luz verde; CMakeLists es cruce con WIP ajeno). Zonas ajenas no tocadas.
- MAPA_DEL_CODIGO usado: `Application.cpp/.h` = fichero de cruce de 14 paquetes (zona mía desde 20/09); `TitleMenuState.*`/`SaveSlotState.*` = G-08/G-11 ya versionados.

## Incidente del reset (registrado, resuelto)
- Al reclamar la fila, HEAD estaba **rojo**: `test_application_cableado` (H0-05, versionado en `91ebf1b`) tenía 4 casos cuyo contrato dependía de `requestPause/requestResume/m_equipmentCatalog/setInventoryRules/setupSaveSlots/loadSlot(newestLoadableSlot())`… y nada de eso existía en la rama. El cableado H0-04/H0-05 entregado el 20/09 **no llegó a commitearse** (sin luz verde) y un reset de la mañana del 22/09 se lo llevó en el árbol de trabajo, dejando el test versionado huérfano.
- Se reconstruyó el cableado completo en la zona mía (ver abajo) hasta dejar los contratos versionados en verde. No se tocó ni deshizo ningún WIP ajeno.
- Evidencia de que la reconstrucción restaura material previo (no añade deuda nueva): `tools/linea_base_core.py` contra el baseline commiteado (que registraba el estado pre-reset) reporta **`Application.cpp` 2472→2412 (BAJA 60)** y **`Application.h` 104→103 miembros (BAJA 1)**. Las 3 «deuda nueva» que reporta el guard son ajenas (`RpgCatalogs.h` 1862→1922, `RpgCoreDefinitions.h` 655→687, `ProjectSerializer.cpp` 1081 cruza el techo).

## Qué se hizo
1. **Reconstrucción H0-04 (cableado)**: carga de `assets/catalogs/equipment.json` en `init` (no-fatal, `CatalogStrictness::Structural`) + miembro `m_equipmentCatalog` + `setInventoryRules` dentro de `loadLevel` (cada transición reconstruye la sesión); `requestPause()/requestResume()` delegando en `GameSession::pause()/resume()`; ramal `InputAction::Quit` en `processInput`: Exploration → `requestPause()`, Pause → `requestResume()`, resto → `m_running=false` (binding intacto en `InputSystem.cpp:gameplayDefaults()`: Escape→Quit, F5→QuickSave, F9→QuickLoad; no se tocó InputMap); `confirmPauseCommand` Resume → `requestResume()`.
2. **Reconstrucción H0-05**: `setupSaveSlots()/quickSave()/quickLoad()/loadSlot(i)/newestLoadableSlot()` sobre `Game::SaveSlots(root, projectId)` (`motor_game_model`); F5/F9 → `quickSave()/quickLoad()`; pausa Save/Load → ranuras; Game Over Load → `loadSlot(newestLoadableSlot())`; `m_savePath`/`saveGame`/`loadGame` eliminados; `restoreSave(data)` conserva el tramo reparador (C-24 intacto).
3. **H0-07 (la rodaja)**: bloque de título del `init` reescrito: `m_projectIndex` asignado ANTES de usarlo (corregido un uso de `.value()` desbordado), `m_title = Game::TitleMenuState::fromManifest("assets/proyectos/<id>.json", "", "saves")`, `m_projectId = campana.id` + `setupSaveSlots()`, y las pantallas Pause/GameOver/Inventory se construyen SIEMPRE con `startAtTitle` (fuera de la rama del índice). `confirmTitleSelection` reescrito para `TitleCommand`: Continue → `loadSlot(m_title->continueSlot())`; Nueva partida → `makeScene(false)`; Opciones → hint G-10; Salir → `requestQuit()`; `None` → hint con `selectedItem().reason` (el motivo de una entrada apagada). `Application.h`: tipo de `m_title` → `unique_ptr<TitleMenuState>` + includes de `TitleMenuState.h`/`SaveSlotState.h`.
4. **CMakeLists.txt**: lib GL-free única `motor_game_model` (= `src/Game/SaveSlotState.cpp` + `src/Game/TitleMenuState.cpp`), `PUBLIC` en `motor_core`. Nacida aparte de motor_level por el mismo motivo que `motor_slots`: los tests GL-free que compilan esos .cpp en su ejecutable no deben ver el símbolo dos veces.
5. **tests/Engine/ApplicationCableadoTests.cpp**: accesores nuevos (`title`, `chooseTitle`, `pendingSceneReplace`, `projectId`) + caso `cableado_titulo_sale_del_manifiesto` (título «Los Perdidos de Boundington», 4 items con Continue apagado + motivo sin guardados del proyecto, `selected()==1` saltando la entrada apagada, confirm en Continue → `None`+motivo, Nueva partida confirma y encola la escena, `m_projectId` correcto; limpieza determinista de `saves/boundington`).

## Verificación (reales)
- Build desde cero en `out/build/opencode-h007` (Unix Makefiles, Debug; assets copiados incl. `proyectos/boundington.json`, `catalogs/equipment.json`, `adventures/boundington_prologo.json`).
- `test_application_cableado` **8/8 PASS** (los 4 del contrato H0-05 reconstruido + ESC-pausa/catálogo H0-04 + título/manifiesto H0-07 nuevo).
- `test_input_system` **4/4 PASS** · `test_save_slots` **13/13 PASS** · `demo_title_menu` Pass (ctest) · `test_application_shutdown` sin símbolos duplicados de `motor_game_model`.
- `juego` enlaza con `motor_core → motor_game_model → motor_level`.
- `ctest` completo sin filtrar: el grueso de «failed» son ejecutables que este build dir no compiló (`Unable to find executable`), no regresiones; el único fracaso REAL es `test_motor_editor_smoke_1` (editor gl smoke, zona ajena, falla inmediato — fuera de mi paquete).
- `tools/linea_base_core.py` (sin `--actualizar`): mi zona en BAJA (Application.cpp 2412, Application.h 103 miembros); deuda nueva únicamente ajena (RpgCatalogs.h, RpgCoreDefinitions.h, ProjectSerializer.cpp).

## Hallazgos registrados
- **Reset 22/09**: el cableado 20/09 sin commitear se evaporó de la mañana; HEAD quedó rojo con `test_application_cableado` (contrato H0-05 versionado en `91ebf1b`). Queda a la atención de Oriol: valorar commitear la zona H para que un troceo/rebase no vuelva a borrarla (ría: el patrón ya mordió el 22/09).
- **CMakeLists.txt sigue siendo cruce congestionado** (G-09 test_entity/player/isometric, ExportarTests G-06, ValidarTodoTests). Mi hunk `motor_game_model` se mezcla con ellos; un commit quirúrgico por hunks es la única vía (patrón Pendiente 33).
- **Nice-to-have**: el aviso `refreshHud` switch sin `GameMode::Pause` (Application.cpp:1284) sigue ahí; sin G-10 vista no aporta.

- **Guard/reachability**: al regenerar `planificacion/alcance_real.json` (avisaba de 0,9 días de desfase) `linea_base_core.py` pasó de 3 a 26 deuda nueva: la foto de alcance del 21/09 y el árbol actual divergen en ~20 ficheros ajenos (AssetBrowserPanel, BattleMenuState, Narrative, SerializerHelpers, ProjectSerializer…) y **`TitleScreen.*`, que deja de estar al alcance de los ejecutables por H0-07** (Application ya no lo incluye). Revertí el JSON regenerado (ya está como estaba) porque realinear una foto con la otra exige `--actualizar` en el MISMO commit (patrón D0007) y eso necesita luz verde. Pendiente para quien consolide: decidir si `TitleScreen.*` se retira de `motor_core`/se archiva (G-08 quedó sustituido por TitleMenuState).

## Siguiente
- H0-13+ del Gantt Fase H o re-verificación de la suite (preferible con build completo de los ~150 targets, tras pactar el commit de consolidación).
- Propuesta: consolidar el commit de la zona H (Application + motor_game_model + test) para cerrar el riesgo de reset; requiere decisión de Oriol y separar hunks ajenos.