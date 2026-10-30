# Índice — Análisis clase a clase del motor

*El análisis definitivo pedido por Oriol: cada clase con sus variables, funciones, flujo y herencia, para saber qué tenemos y qué falta. Cubierto todo `include/` + `src/` + `examples/` + `tests/` (101 headers, 59 cpp, ~23.500 líneas de código + 2.708 del editor visual). La matriz 08 cierra la trazabilidad cabecera→análisis y separa código integrado de infraestructura aún aislada.*

**Historia del encargo** (por qué tarda): esta tarea se ha comido tres sesiones — Claude (cortada por límite de contexto, nada escrito) → Qwen 31/08 11:36–11:53 (escribió 01 y 02, cortada) → ZCode 31/08 12:15 (relevo documentado en TABLERO; 03–07 + este índice). Formato y numeración de Qwen respetados.

**Relación con LINEA_BASE.md:** aquello es la foto de módulos y activos (BASE-01/02); esto es el nivel clase. Ojo: **los contratos de LINEA_BASE §3 ya quedan desfasados** por la reestructuración en vuelo (TileMap ahora hereda de RuntimeTileMap; el `getTile` que lanza vive en Runtime/).

| Doc | Módulos | Estado git del territorio |
|---|---|---|
| [01_core.md](01_core.md) | Core: JSON, math, ids, memory, resources, errors | commiteado |
| [02_rpg_modelo_de_datos.md](02_rpg_modelo_de_datos.md) | RPG: enums, definitions (21 structs), catálogos (20+Location) | commiteado |
| [03_rpg_motores.md](03_rpg_motores.md) | RPG: DicePool, CharacterSheet, Inventory/Condition/Skill, Narrative, CharacterCreation | NarrativeEngine en vuelo (beatsFired RUN-10) |
| [04_render.md](04_render.md) | Render: pipeline, entidades, HUD, fuente, post-FX | DOC-03 en vuelo (7 ficheros) |
| [05_game_level_map_world.md](05_game_level_map_world.md) | Game (sesión/combate/save), Level, Map/World/Runtime/Import (nuevos) | Map/World/Runtime/Import + SaveGame sin trackear |
| [06_engine_editor_proyecto.md](06_engine_editor_proyecto.md) | Engine (Application/Loop/Input/Scene), Editor, Project (nuevo), editor/ | INF-03 a medio ejecutar |
| [07_demos_tests_y_flujos.md](07_demos_tests_y_flujos.md) | 63 ejecutables, 15 tests, los 6 flujos transversales | tests/ sin trackear |
| [08_matriz_cobertura_y_diagnostico.md](08_matriz_cobertura_y_diagnostico.md) | Cobertura de las 101 cabeceras, herencias, flujos reales y brechas priorizadas | contrastado contra árbol, CMake y CTest |

## Qué tenemos (verificado)

| Subsistema | Piezas | Estado |
|---|---|---|
| **Fundaciones** | JSON propio (sin \uXXXX, por contrato), ids tipados fuertes, HandlePool generacional, `Result<T>`, excepciones de arranque, ResourceManager RAII | Sólido, GL-free |
| **Motor Nd6 Ed.3** | DicePoolEngine (enfrentada pura + pifia nueva) + SkillExecutor (9 pasos: pasivos→QTE→pool→enfrentada→upgrades→crítico→magnitud) + RNG inyectable | Completo y testeado |
| **Modelo de datos RPG** | 21 structs + 20 catálogos tipados + LocationCatalog (2.416 entradas cargando de 24 JSON) | Completo; deuda de contenido aguas arriba |
| **Personaje** | CharacterSheet único (dual-mode) + CharacterCreation/Screen (14 pasos) | Fuente única resuelta, **cableado a medias** (ver falta #2) |
| **Sesión de juego** | GameSession (6 modos) + BattleState + EnemyBrain + Morality + SaveGame atómico | Jugable end-to-end (campaña 72/72) |
| **Narrativa** | NarrativeEngine (beats/flags/objetivos/timers F1) + 5 aventuras de campaña | Funcional; efectos P0-12 pendientes |
| **Render** | Camera/TileMap(adaptador)/SpriteBatch(ring 3)/IsometricRenderer + post-FX exclusivos + HUD (7 widgets) + BitmapFont con acentos | Pipeline completo; F0 texto a medias |
| **Editor** | EditorState + ObjectInspector + ProjectHub + ProjectIndex + ProjectSerializer canónico | Funcional (proyectos declarados, build empaqueta) |
| **Nueva arquitectura (en vuelo)** | MapDocument/World/Runtime/Import/Project + ApplicationLoop fixed-step + InputSystem + SceneManager + 15 tests | **Sin commitear** — ver falta #1 |
| **Verificación** | 63 ejecutables (46 demos, 31 en ctest + 15 unit) + suite_gl_free + validadores Python | CI ubuntu; macOS verificado local; Windows nunca compilado |

## Qué falta (priorizado, cruzando módulos)

1. **Commitear la reestructuración INF-03/DOC-01..03** — módulos enteros (`Map/World/Runtime/Import/Project`, `SaveGame`, `tests/`, `editor/`) viven sin trackear y ~25 ficheros M. Es el riesgo nº1 de colisión del workspace: quien toque lo mismo, pisa.
2. **Fractura #1 a medias en runtime:** `populate_from_monster_catalog`/`set_character_sheet` solo los ejercita un demo; GameSession construye enemigos por la vía legacy (`CombatData`). Falta `Player::populate_from_class_race_background()` y pasar BattleState/GameSession por la hoja única.
3. **Retirada Ed.3 incompleta:** `resolve_against_cd/defense_to_cd/override_target_defense` vivos; comentarios de `Skill.h` aún explican CD/ARMOR_CLASS; GDD de dndWeebCC §5 igual (ya en bandeja).
4. **Condiciones con doble representación** (CharacterSheet vs ConditionEngine): dejar una, aliasar la otra.
5. **`m_timerBar` fantasma:** la barra F1 nunca se dibuja (widget creado, nadie lo alimenta) — el propio F1 está cojo de HUD.
6. **Infraestructura sin consumidor:** SceneManager/Scene, World (ECS-lite), WorldTopology, DocumentRef — listos y probados por tests, sin cablear en Application.
7. **Doble formato de manifiesto:** ProjectIndex (legacy español, fprintf) vs ProjectSerializer (canónico v1 inglés) — sin round-trip.
8. **Triple vía de entrada:** InputSystem (juego) / KeyEdge (editor) / InputState legacy (animación Player).
9. **F0 del texto:** `clipText` sin consumidor de producción; HudText sin límite de alto; acentos contradictorios (Morality "HÉROE" contra su propio header; degreeLabel catalán).
10. **Latentes y muertos:** `ObjectCatalog::exportJson` declarado sin definición (error de enlazado); `DicePoolEngine.cpp` TU vacío; `SeededRng::nextInt` sin definir; `kChase` nunca asignado; `LevelDefinition::enemies` sin consumidor; `creacion_personaje.cpp` huérfano de compilación; `Check 2.h`.
11. **Deuda de datos:** 432 monstruos con `maxHealth==0` en JSON fuente; tablas de frames raciales/story hardcodeadas en Application; defaults de tiers duplicados (código vs JSON).
12. **P1/P2 declarados en comentarios:** triggers de pasivas, MonsterPhase (jefes), efectos OpenShop/GrantQuest/LevelTransition/SetMilestone, vínculo AdventureScript↔AdventureDefinition, multiclass en creación, condiciones que afecten iniciativa/saves.

## Cómo mantener esto vivo

- Es una **foto del 31/08/2026** sobre HEAD `3258f6f` + trabajo declarado en vuelo. El código manda; si algo no cuadra, este doc queda viejo a propósito (misma regla que LINEA_BASE).
- Al cerrar una deuda de la lista, táchala aquí y deja huella en `sesiones/` (la numeración D0001+ del workspace para decisiones).
- Las clases nuevas de la reestructuración (Map/World/Runtime/Import/Project) están documentadas con marca [WIP]/[??]: cuando INF-03 aterrice, quitar las marcas es parte del cierre.
