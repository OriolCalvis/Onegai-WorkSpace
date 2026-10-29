# 06 — Módulos `Engine` · `Editor` · `Project`

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe el análisis clase a clase de agosto, foto del 31/08-03/09/2026. El juicio vigente, clase a clase y sobre el código de hoy, está en `planificacion/juicio_clases/` (12 partes, generado con `tools/mapa_clases.py`). De las 7 citas de codigo que contiene, 7 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

*Análisis clase a clase · 31/08/2026 (retomado por zcode) · 6.531 líneas. Engine: Application 318+1682, ApplicationLoop 64+99, Window 42+54, Input/* 142+108, Scene/* 96+146. Editor: EditorState 337+889, ObjectInspector 143+506, ProjectHub 121+346, EditorValidation 187, ProjectIndex.h 6 (shim). Project (nuevo, untracked): Project/ProjectIndex/ProjectSerializer/DocumentRef/ProjectSettings + editor/src/level_editor.cpp 2.708.*

**Reestructuración en vuelo (INF-03, ver `architecture/MIGRATION_MAP.md`):** `src/Editor/ProjectIndex.cpp` (DELETED) → `src/Project/ProjectIndex.cpp` (nuevo, +14 líneas: validación de manifiesto al escanear); `include/Editor/ProjectIndex.h` es ahora un **shim de 6 líneas** hacia `Project/ProjectIndex.h`; `examples/level_editor.cpp` (borrado en disco) → **`editor/src/level_editor.cpp`** (2.708 líneas, untracked) con `editor/README.md`; CMake ya apunta el target ahí. Pendiente según el mapa: ProjectHub/EditorState/EditorValidation/ObjectInspector a `editor/`, Level a Project+Map, ObjectCatalog a Database, GameSession/WorldTopology a World.

---

## ENGINE

### `Application` (`include/Engine/Application.h`) — orquestador
**Qué es:** posee ventana, contexto GL, assets, HUD, GameSession y narrativa; traduce input a llamadas de sesión y dibuja. Toda la lógica de partida delega en GameSession (GL-free).
**Estado (~40 miembros):** `m_input` (InputSystem), GL en unique_ptr por orden de ciclo de vida (`m_window/m_hudBatch/m_atlas`); `m_player/m_playerSkills/m_inventory` (sobreviven transiciones); `m_session` (destruida en cada `loadLevel`); **narrativa FUERA de la sesión a propósito**: `m_narrativeState`, `m_narrative`, 4 `AdventureScript` (prologo/dia1/dia2/ocaso), `m_nd6Skills`, `m_narrativaLista`, `m_narrativeCampaignTransitions`; `m_currentLevelPath` + `m_lastInput` (base del AFK F1); `m_savePath` (una ranura); `WorldMarker` anidado (reconciliación de sprites por índice); ~25 widgets HUD; `m_lastCameraCell{-9999,-9999}` fuerza recentrado inicial.
**API:** `init(...)` → `Result` (convierte EngineException); `run(maxFrames)` (≥0 = modo humo con volcado PPM); `shutdown()` en orden inverso; `processInput()` según GameMode (refresca `m_lastInput` solo si `input.hasActivity()`); `update(dt)` — animaciones, cámara por `transitionTo` al cambiar celda, `session->update`, `applyPendingTransition`, saltos de capítulo (flags privadas `bnd_diaN_empezado`), **dt activo** = 0 si modo≠Exploration o >15 s sin input → `updateNarrativeTimers(dtActivo, nivel)`, `syncWorldMarkers/refreshHud/hud.update/FPS`; `render(capture)` escena+HUD+PPM; `loadLevel` (monta mapa/tileset con regiones EN BLOQUE desde firstGid, sesión, marcadores; rescata oro; re-enchufa narrativa + `enterLevelNarrative`); `applyPendingTransition` consume la puerta ANTES de cargar (anti-bucle); `saveGame/loadGame` F5/F9 solo en exploración (load restaura narrativa ANTES del nivel para no repetir beats); `confirmShop/Business/BattleCommand`; `refreshHud`; `syncWorldMarkers` (mark-and-sweep).
**Flujo:** la llama `juego.cpp::main`.
**Trampas/deuda:** skills registradas a mano en `init()` (pendiente JSON); tablas hardcodeadas `raceNpcFrame` (43 razas) y `boundingtonStoryFrame` (15 actores) duplican assets; **`m_timerBar` es un widget fantasma**: se crea y añade (init l. 660-664, `setVisible(false)`) pero NADIE hace `setValue`/`setVisible` después — la barra F1 nunca se dibuja; flags de campaña solo viven aquí; pila de apagado manual de ~25 resets; **SceneManager existe y Application NO lo usa** (monolito).

### `Window` — envoltorio "tonto" de GLFW
Crea ventana + GL 3.3 Core + GLAD (glad ANTES que GLFW, documentado); lanza `EngineException` si falla. Move borrado a propósito; `glfwTerminate()` en el destructor impide dos ventanas vivas.

### `Engine::ApplicationLoop` (+`Settings`/`Frame`/`Services`) — núcleo fixed-step (RUN-01/ADR-007)
Independiente de GLFW/OpenGL: `Settings` (fixedStep 1/60, maxFrame 0.10, máx 6 fixedUpdates); `tick(realSeconds, services)` = poll → input → acumular (clamp, NaN→0) → N fixedUpdates → descarte de atraso (spiral of death) → render(alpha). Application lo instancia por partida inyectando lambdas; también tests sin ventana. **`interpolationAlpha` se calcula pero Application lo ignora** (RUN-07.4 pendiente).

### `Engine::Input::RawInputState` + `InputMap` — vocabulario de entrada (RUN-02)
Enums `InputKey`/`InputAction`, estado físico por tecla, mapa acción→teclas; `gameplayDefaults()` (WASD/E/Tab/F5/F9/Enter/Space/Escape; Cancel=E y Tab). **Solo 11 teclas**; el editor NO usa esto (usa su propio `KeyEdge`).

### `Engine::Input::InputSnapshot` + `InputSystem` — foto de frame (ADR-008)
`ActionState{pressed, held, released}` por acción; `hasActivity()` (base del AFK). InputSystem traduce Raw→Snapshot comparando con el frame anterior; `releaseAll()` sin foco. Coexiste con el `InputState` legacy (4 bools que sintetiza Application solo para animar el sprite del Player) — **dos vías de entrada paralelas + la del editor**.

### `Engine::Scene` (interfaz) + `SceneManager` — RUN-03/ADR-009
Contrato de escena (load Result, fixedUpdate/update/render, `mapId` fuerte). SceneManager: pila con transiciones diferidas (una por frame, anti-reentrada; load fuera de callbacks; fallo descarta y conserva la anterior). **Infraestructura lista SIN consumidores**: Application sigue monolítico; solo tests.

## EDITOR

### `EditorState` — el modelo del nivel, GL-free
**Qué es:** capas, tiles, objetos, playerStart, historial, selección/portapapeles y paletas con filtro. (UI = HUD de Fase 9; ImGui imposible sin red — decisión INF-02.)
**Estado:** `m_layers` (nombre+gids, siempre ≥1 "suelo"), capa activa, `m_objects` (ObjectSpawn), `m_playerStart`, `m_undo/m_redo` (snapshots COMPLETOS), selección/portapapeles, 7 herramientas (PaintTile/EraseTile/PlaceObject/RemoveObject/SetPlayerStart/FillTiles/LinkLevel), paletas dobles (completas + filtradas) con etiquetas editoriales.
**API:** paletas (`set*Palette/set*PaletteLabels/next/prev/selected`, filtro — el filtro ES la paleta visible); edición (`applyAt` fuera de rango no-op; `paintTile/eraseTile/fillTiles` flood fill ortogonal = un paso de historial; `placeObject` reemplaza; `placeSpawn` conserva transición); historial `undo/redo/clearHistory`; capas (no deja 0); selección `copySelection/pasteAt/eraseSelection` (un paso aunque toque mil celdas); **propiedades por instancia**: `setObjectPatrol/TargetPosition/Scale/DisplayName/Variant/EffectOverride/TargetLevel`, loot (`addObjectLoot/removeLootAt/setObjectLootTable`), `setObjectEvent`, `set/removeObjectProperty`; export `exportTmx(settings)` (round-trip verificado) y `exportLevelJson` (strings puros).
**Trampas:** historial por snapshot completo (caro, consciente); comentario del header aún dice "objects sin patrulla" aunque ya se exportan (doc desfasada); no valida ids contra catálogo (permisivo).

### `EditorValidation` (+`EditorValidationResult`) — reglas de "nivel listo" compartidas editor/CI
`check(editor, catalog, collisionGids)`: inicio transitable (error), alcanzabilidad BFS (warning si hay inalcanzables), objetos sin ficha (error), mapa sin transitable (error). `checkRpg(...)` añade: spriteId ausente, maxHealth≤0, mana<0, skills inexistentes, power/price<0, NPC no interactuable (warning), patrullas invertidas, transición en no-interactuable. **Trampa real: `check` valida la CAPA ACTIVA** (`tileAt` sin índice) — la opinión cambia según la capa seleccionada.

### `Editor::ObjectInspector` — panel modal GL-free de propiedades del objeto colocado
Nombre visible, variante, escala, efecto, salida, destino, botín, tabla, evento, propiedades k/v; edición de texto y pickers filtrables. `open(x,y)` (sin objeto no abre); **`key(char)` única puerta** (modal: true = tecla consumida; W/S navegan, ENTER edita/confirma, ESC cancela, Q/E ciclan y filtran, X quita loot, buffer máx 48). Picker de salida con "(sin salida)" en posición 0 para DESCONECTAR. Si el spawn desaparece (deshacer), se cierra en la siguiente tecla.

### `Editor::ProjectHub` — pantalla de arranque GL-free
Todo el estado aquí; la vista solo pinta `lines()/detail()/message()` y pasa teclas. `load(assetsRoot)` estático; modos List/NewProject/Message; `key` — List: ENTER→`Action::Open` (**no abre niveles ni cierra ventanas**), n nuevo, b build, ESC Quit; NewProject: `[a-z0-9_]` máx 24, prefijo AUTO `draft.substr(0,3)+"_"`; `detail()` ficha del proyecto marcado (a qué resolvió `entrada` de verdad, primeros 6 problemas); `build()` lanza `python3 tools/build_proyecto.py <id>` vía popen (busca el script subiendo 6 niveles) y distingue exit status real. **Trampas:** `build()` BLOQUEA el hilo de render; id en línea de shell (mitigado por `idSeguro`); prefijo no editable.

### `Editor/ProjectIndex.h` — shim de 6 líneas hacia `Project/ProjectIndex.h` ("se retirará cuando no haya referencias"). Aún lo consumen `examples/demo_proyectos.cpp` y `editor/src/level_editor.cpp`.

## PROJECT [?? nuevo, untracked]

### `Editor::Project` + `ProjectCheck` — documento raíz de un pack
id/nombre/época/autor/prefijo/settings/entry + listas niveles/mapas/aventuras/catálogos; `entryKind/entryPath` (None/Adventure/Level/Missing) los resuelve el índice. `validate()`: id `[a-z0-9_]`, prefijo vacío o acabado en `_`, settings válidos, entry relativa, nombres de documento sin `/` `\` `.` `..` ni duplicados, `initialMap ∈ maps`. `playable()` = entry + niveles. Validación estructural pura (sin I/O).

### `Editor::ProjectSettings` — POD: `schemaVersion` (kCurrentSchemaVersion=1), 4 directorios lógicos (relativos, válidos y DISTINTOS), `initialMap`.

### `DocumentRef` — referencia persistente a documento por `ids::DocumentId` (fuerte), no por ruta; `validate()` inline. **Sin usuarios todavía** (espera a los documentos Level/Adventure).

### `Editor::ProjectIndex` — los proyectos vivos
Lee `assets/proyectos/index.json` + manifiestos (fuente de verdad declarada, no deducida por prefijo). `scan`: manifiesto ausente/roto/inválido → **huérfano, no tumba el editor**; admite claves LEGACY en español (`nombre/niveles/mapas/...`) + `schemaVersion/initialMap`; `resuelveEntrada` admite entry con barra o nombre suelto (busca en adventures/ y levels/). `check(id)`: existencia real de cada fichero, entrada vacía ("no se puede jugar") o Missing. `create`: valida id/prefijo, rechaza duplicado en índice Y manifiesto suelto (anti-machaque), escribe esqueleto. `remove`: quita del índice primero y borra SOLO el manifiesto (**sin cascada**). I/O con fopen sin `<filesystem>` (decisión de casa); `esc()` mínimo sin `\uXXXX`.

### `ProjectSerializer` — persistencia canónica v1
Serialize/deserialize de Project y MapDocument con **orden de claves fijo** (dos guardados = mismos bytes) y **escritura atómica** (.tmp + rename; MoveFileEx en Windows). Claves INGLESAS canónicas (`name/levels/...` + objeto `settings`); `schemaVersion` exacta; ids fuertes (`MapId/LayerId/EntityId/AssetId/RegionId::parse`). serializeMap NO guarda displayName/loot/eventos de entidades (quedan fuera del canónico). Lo consumen `test_project_serialization`/`test_map_round_trip` y el futuro motor_editor.

---

## Qué falta / deuda de los módulos

1. **Doble formato de manifiesto:** `ProjectIndex` lee/escribe legacy español (fprintf) mientras `ProjectSerializer` escribe canónico v1 inglés — **no hay round-trip entre ambos**; unificar es el paso siguiente natural de DOC-01.
2. **`m_timerBar` fantasma** en Application: la barra F1 no llega a pantalla (falta getter de timers visibles en GameSession + bloque en refreshHud).
3. **SceneManager/Scene sin consumidor**; Application monolítico.
4. **Triple vía de entrada:** InputSystem (juego) vs `KeyEdge` local del editor vs `InputState` legacy (animación de Player).
5. **Datos duplicados en Application** (tablas de frames raciales/story, skills a mano).
6. **EditorValidation valida solo la capa activa.**
7. **`build()` de ProjectHub bloquea** el render (popen síncrono).
8. **DocumentRef sin usuarios**; comentarios de EditorState desfasados.
9. **Reestructuración INF-03 a medio ejecutar** (ver nota inicial): riesgo de colisión hasta que se commitee.
