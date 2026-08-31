# 07 — Demos, tests y flujos transversales

*Análisis clase a clase · 31/08/2026 (retomado por zcode) · inventario de ejecutables + batería de tests + los 6 flujos que cruzan módulos.*

## A. Inventario de ejecutables

**Cómputo real: 48 `add_executable` directos + 15 creados por `add_motor_unit_test()` bajo `BUILD_TESTING` = 63 ejecutables.** Los demos GL-free verificados se registran en ctest (31 con etiqueta `gl-free;smoke`, `WORKING_DIRECTORY=build/`) y acumulan en `suite_gl_free` (RUN-11.3), que es lo que corre CI.

| Target | Qué verifica | GL |
|---|---|---|
| `juego` | Partida jugable completa (Application, fases 1–4 + 6–12) | ventana |
| `level_editor` | Editor visual — **fuente ahora en `editor/src/level_editor.cpp`** (2.708 líneas, untracked) | ventana |
| `sandbox_window` | Fase 1: ventana + contexto GL 3.3 core | GL |
| `demo_textured_quad` | Fase 1: quad con GL_NEAREST; modo N frames → PPM | GL |
| `demo_isometric_renderer` | Fases 2–3: culling + Painter's con TileMap y entidades | GL |
| `demo_animated_entity` | Fase 3: AnimatedEntity/Player/Enemy (animación, patrulla) | GL |
| `demo_lighting` / `demo_fog` / `demo_lut` | Fase 4: post-FX (lightmap, niebla con memoria, LUT) | GL |
| `demo_hud` / `demo_battle_hud` | Fase 9: HudBar/Panel; BattleState + menús + BitmapFont real | GL |
| `demo_dynamic_lights` | Fase 11: point lights + sombras blob | GL |
| `demo_bitmap_font` | glyphUV/measureText puros (nunca genera atlas) | GL-free de facto, **fuera de ctest** |
| `demo_hud_update` | Fractura #3: update alcanza a TODOS los widgets | ídem |
| `demo_enemy_sprite` | Fractura #1: Enemy delega en EnemyBrain (atlas nullptr) | ídem |
| `demo_resource_manager` | ResourceManager aislado con fixtures (INF-06.2) | GL-free, ctest |
| `demo_handle_pool` | Pool generacional (handles sin colgantes) | GL-free, ctest |
| `demo_occlusion_rules` | Regla pura de oclusión | GL-free, ctest |
| `demo_camera` | Camera + IsoMath sin ventana | GL-free, ctest |
| `demo_tilemap` | TileMap::loadFromFile sobre TMX real | GL-free, ctest |
| `demo_editor_state` | Núcleo del editor: pintar, objetos, paletas, round-trip | GL-free, ctest |
| `demo_level_loader` | Fase 6: JsonValue + LevelLoader | GL-free, ctest |
| `demo_skills` | Fase 7: Skill/SkillSet/ApplySkillEffect | GL-free, ctest |
| `demo_battle` | Fase 8: combate por turnos sobre ICombatant | GL-free, ctest |
| `demo_battle_nd6` | Combate Nd6 ×50 por el camino real | GL-free, ctest |
| `demo_dice_engine` | Nd6 con ScriptedRng: grados, ventaja, crítico, pifia, semilla | GL-free, ctest |
| `demo_nd6_distribution` | Distribución de 20.000 tiradas (informativo) | GL-free, ctest |
| `demo_enemy_brain` | Vida+patrulla+IA separadas del sprite | GL-free, ctest |
| `demo_game_session` | Ciclo exploración↔combate con TileMap/catálogo reales | GL-free, ctest |
| `demo_object_inspector` | Panel de propiedades (texto, escala, salidas, botín) | GL-free, ctest |
| `demo_creacion_personaje` | Creación: RECHAZA elecciones ilegales | GL-free, ctest |
| `demo_mundo` | B24: 19 naciones, 95 ciudades desde locations.json | GL-free, ctest |
| `demo_ciudad` | Ciclo social/comercial: NPCs, tiendas, puertas | GL-free, ctest |
| `demo_aventura` | P0-13: NarrativeEngine, 6 permutaciones | GL-free, ctest |
| `demo_temporizadores` | F1: timers de misión (dt activo, umbrales, expiración) | GL-free, ctest |
| `demo_narrativa_nd6` | Puente narrativa→Nd6 (skillCheck → flag de grado) | GL-free, ctest |
| `demo_narrativa_combate` | startBattle: flag DESPUÉS de cerrar; id roto | GL-free, ctest |
| `demo_prologo` / `demo_boundington` / `demo_precuela_boundington` / `demo_campana_boundington` | Campaña: 17 beats / 3 finales / consecuencias / 72 beats con UN state | GL-free, ctest |
| `demo_senal_ceniza` | 2000 b.f., 4 rutas + cobertura | GL-free, ctest |
| `demo_este_norte` / `demo_este_sur` | Cuadrantes: 55 niveles / 3 prototipos TMX+JSON | GL-free, ctest |
| `demo_proyectos` | Hub: descubrir/revisar/crear proyectos. **Fuera de ctest** (necesita `tools/build_proyecto.py` en cwd; TOOL-09) | GL-free |

**Anomalías:** `examples/creacion_personaje.cpp` existe pero **no tiene `add_executable`** (huérfano de compilación); `examples/Check 2.h` sin trackear (duplicado del tramposo " 2").

## B. Tests (`tests/`, INF-01, sin commitear — 15 + TestHarness.h + fixtures/maps)

| Test | Qué prueba |
|---|---|
| `test_handle_pool` | Reciclaje y handles colgantes (INF-01.2) |
| `test_strong_id` | IDs tipados fuertes (`Core/Ids`) |
| `test_editor_state` | Reglas puras del modelo del editor (INF-01.3) |
| `test_application_loop` | RUN-01: orden determinista sin GLFW |
| `test_input_system` | RUN-02: teclado simulado, bindings, snapshots |
| `test_scene_manager` | RUN-03: escenas sin ventana, transiciones diferidas |
| `test_world` | RUN-04: entidades runtime con handles generacionales |
| `test_map_document` | DOC-02: modelo documental sin GL |
| `test_runtime_tilemap` | DOC-03: builder MapDocument→RuntimeTileMap |
| `test_tmx_importer` | TMX→MapDocument (ADR-006) |
| `test_map_round_trip` | Integración: TMX→documento→runtime→reescritura idempotente |
| `test_project` | DOC-01: validación Project/ProjectSettings |
| `test_project_serialization` | Round-trip de manifiestos/índice |
| `test_world_topology` | Regresión de referencia colgante en WorldTopology |
| `test_save_game` | RUN-10: formato, escritura atómica, round-trip sobre sesión real |

## C. Flujos transversales

### C.1 Bucle principal (`Application::run` → `ApplicationLoop::tick`, ADR-007)
Arranque: `juego.cpp` → `Application::init` → Window → SpriteBatch HUD (tras contexto, no antes: crash documentado) → ShaderManager → atlas → widgets → ObjectCatalog → SkillCatalog → aventuras JSON → `loadLevel`.
Por frame: (1) `shouldKeepRunning`; (2) `Window::pollEvents` → InputSystem; (3) `processInput` según GameMode (refresca `m_lastInput`); (4) acumulador → N× `update(dt fijo)`: animaciones → cámara al cambiar celda → `session->update` → `applyPendingTransition` → saltos de capítulo (flags `bnd_diaN`) → **dtActivo** (0 si ≠Exploration o AFK) → `updateNarrativeTimers` → `syncWorldMarkers` → `refreshHud` → `hud.update`; (5) `render(alpha)`: clear → occlusion focus → `renderFrame` (mapa culled + cola + post-FX) → HUD (begin/render/end) → PPM en modo humo → swap.

### C.2 Carga de nivel
`loadLevel` → `LevelLoader` (JSON → LevelDefinition; `enemies` legacy NO se juega) → `TileMap::loadFromFile` → `TmxMapImporter` → MapDocument → `RuntimeTileMapBuilder` → RuntimeTileMap (+ metadatos tileset en el adaptador) → `TextureManager` + `TextureAtlas` con **regiones en bloque desde firstGid** → re-apuntar atlas → rescatar oro → `GameSession` (reparte spawns: Enemy→WorldEnemy{EnemyBrain, SkillSet}; resto→worldObjects) → narrativa `setAdventure` + `enterLevelNarrative` (beat "enter") → `WorldMarker`s → `addToQueue` + recentrado de cámara.

### C.3 Tick narrativo
`update` calcula dtActivo → `tickTimers` (arranque por flag, expiración por zona/tiempo, avisos por umbral) → si cambió `flagCount()` y Exploration → `applyNarrative(engine->tick(state))`: speaker/líneas→Dialogue; gold; `resolveSkillChecks` (diferido: `roll_pool` vs `rivalDice` → `opposed_degree` → `setFlag(flag{Grado})`); `resolveBattles` (materializa emboscada + `startBattle`; flags victory/defeat PENDIENTES hasta `syncBattleOutcome`). Otros disparos: `interact()`→`talkTo` (la narrativa MANDA sobre el diálogo fijo del catálogo); `closeInteraction()`→`tick` (un cierre, un beat); `loadLevel`→`enterLevel`. Motor: `fire()` = primer beat que encaja (trigger+target, once, requirements) → efectos inmediatos (SetFlag/ClearFlag/StartTimer) + diferidos (NarrativeResult) → `markBeatFired`.

### C.4 Combate Ed.3
`tryMovePlayer` a celda ocupada → `startBattle` → BattleState → `resolveAllyAction` (Attack→`ApplyBasicAttackNd6`; UseSkill→canUse/spend+`ApplySkillEffect`; Item→pickup contra ObjectCatalog; Flee) → `Game/Skill.cpp` monta `ExecutionContext` con `override_base_dice=1+stat` y `override_target_dice` = defensa del defensor (porque ICombatant≠CharacterSheet) → `SkillExecutor::execute`: pasivos → QTE opcional → **`roll_pool` INVIOLABLE** → Ed.3 enfrentada (`opposed_degree`: botch atacante→BOTCH, botch defensor→CRITICAL, menor→BOTCH, igual→PARTIAL, mayor+todos-seis→CRITICAL, resto SUCCESS) → upgrades → confirm crítico → `magnitude(grado)` → `takeDamage/heal` → log → `checkOutcome`. Enemigo: primera skill pagable alfabética o básico, mismo camino, objetivo = primer aliado vivo (determinista). Cierre: `syncBattleOutcome` + flags narrativos + desencolar oleada siguiente.

### C.5 Guardados
**Partida (F5/F9):** `captureSaveGame` → `SaveGameData` (posición, vida, inventario, oro, moral, negocios, objetos por posición+id, enemigos por spawnIndex, flags/beats/timers) → `JsonWriter` → `.tmp` + rename atómico. Carga: `parseSaveGame` → **`applyNarrativeState` ANTES de cargar nivel** (no repite beats) → `loadLevel` → `restoreSaveGame` (fase 1 valida todo sin escribir; fase 2 aplica) → Exploration o GameOver.
**Editor (G):** lambda `guardarNivel()` → `TmxTilesetSettings` copia el tileset DEL TMX de origen (evita el "destructor de mapas") → `exportTmx` + `exportLevelJson` → rutas resueltas por `ProjectIndex::scan("assets")` (`assets/maps/<base>.tmx` + `assets/levels/<base>.json`). Navegación `,`/`.` guarda ANTES de cambiar. Proyecto nuevo: `ProjectHub` → `ProjectIndex::create`; "B compilar" → `popen` de `tools/build_proyecto.py`.

### C.6 Arranque del editor
`editor/src/level_editor.cpp::main` → `parseArgs` (`--nivel/--catalogo/--proyecto/--proyectos/--nuevo/--frames`) → bucle externo proyecto↔edición: FASE 1 `ProjectHub::load("assets")` (lógica GL-free probada por demo_proyectos; el main pinta `lines()/detail()` y traduce teclas; `Action::Open`→resolver proyecto; `Action::Quit`→salir); FASE 2 edición: catálogos fusionados del proyecto + nivel + `TileMap` (las dimensiones del TMX mandan) + `EditorState` + `ObjectInspector` + bucle con `IsometricRenderer`/HUD propio y ratón. Aquí vive `KeyEdge` (l. 500) con `prime()` al cambiar de fase (el ESC fantasma), y los controles letra-first (G guarda, V valida, P juega lanzando `./juego` aparte, M menú, `,`/`.` escenarios, `[`/`]` subgrupos; F-keys como alias).

---

## Deuda del inventario

1. `creacion_personaje.cpp` huérfano de compilación (sin add_executable).
2. `demo_proyectos` fuera de ctest por su dependencia del cwd (TOOL-09).
3. `examples/Check 2.h` (duplicado " 2" — trampa conocida de la BITACORA).
4. tests/ enteros sin commitear (INF-01 en curso) — el CI no puede ejecutarlos todavía.
