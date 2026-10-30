# 05 — Módulos `Game` · `Level` · `Map` · `World` · `Runtime` · `Import`

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe el análisis clase a clase de agosto, foto del 31/08-03/09/2026. El juicio vigente, clase a clase y sobre el código de hoy, está en `planificacion/juicio_clases/` (12 partes, generado con `tools/mapa_clases.py`). De las 7 citas de codigo que contiene, 6 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

*Análisis clase a clase · 31/08/2026 (retomado por zcode) · 5.119 líneas: Game 1.497 h + 1.970 cpp (GameSession 498+762, EnemyBrain 296+23, BattleState 177+221, SaveGame 135+466, Skill 161+192, Morality 63+46) · Level 501+386 · Map 181+240 · World 107+134 · Runtime 106+128 · Import 57+239.*

**Estado git:** limpios (commiteados): BattleState, EnemyBrain, Morality, Skill, LevelDefinition, LevelLoader. **Modificados sin commitear:** GameSession, ObjectCatalog, WorldTopology. **Sin trackear (`??`):** Map/, World/, Runtime/, Import/ enteros + `SaveGame.*` — reestructuración en vuelo (ver 06, nota INF-03).

---

## GAME

### `GameSession` (`include/Game/GameSession.h`) — clase concreta, GL-free
**Qué es:** el estado de una partida: exploración, combate, diálogo, tienda y negocio, sobre `RuntimeTileMap` + `ObjectCatalog` + datos planos. El pegamento de las fases 6–12.
**Estado:** `m_mode` (GameMode: Exploration/Battle/GameOver/Dialogue/Shop/Business — todos modales); punteros no propietarios (mapa, catálogos, cuerpo/skills/inventario del jugador); `m_level` copiado (se muta); `m_worldObjects` y `m_enemies` (`WorldEnemy` con `spawnIndex` como identidad para guardado, `brain` unique_ptr); `m_battle` + `m_battleEnemyIndex`; `m_transition` pendiente; `m_gold`, `m_businesses` (rentPercent 50–200), `m_morality`, `m_stepsSinceRent` (ciclo de renta cada 40 pasos); narrativa enchufable + `m_rng` Xoroshiro128p inyectable; `m_pendingBattleVictory/DefeatFlag` + `m_battleQueue` (oleadas).
**API:** `tryMovePlayer` (colisión tile/blocksMovement; pickup se recoge; enemigo dispara combate); `update(dt)` solo patrullas en Exploration; `interact()` (puerta→transición; NPC/cartel contiguo→Dialogue/Shop/Business); `closeInteraction()` vuelve y hace UN tick narrativo; `setNarrative/setRandomEngine/setNd6SkillCatalog`; `enterLevelNarrative`; `updateNarrativeTimers(dtActivo, nivel)` — F1: Application pasa `dtActivo=0` si ocupado o AFK; `talkNarrative`; `buy/sell/addGold`; `buyCurrentBusiness/adjustCurrentRent/collectRent`; `occupancyPercent/incomeFor` (puras, óptimo ~175%); `battle()/syncBattleOutcome()` (Victory borra enemigo, Fled lo deja, Defeat→GameOver; enciende flags narrativos y desencola); `captureSaveGame/restoreSaveGame`; `log/logNote`.
**Flujo:** la llama Application (input, transiciones, guardado); demos/tests sin ventana. Llama a EnemyBrain (con callback de colisión = única fuente de "qué bloquea"), BattleState, NarrativeEngine, DicePoolEngine.
**Trampas:** solo instancia `LevelDefinition::objects` — el array legacy `enemies` NO se juega; `sell()` busca el tendero por NOMBRE entre los objetos del mundo (frágil); jugador muerto al construir → GameOver inmediato; id de monstruo roto enciende flag de derrota (permisivo a propósito).

### `BattleState` (`include/Game/BattleState.h`) — clase concreta, GL-free
**Qué es:** combate por turnos estilo DQ sobre `ICombatant`/`SkillSet`; Ed.3: TODO daño pasa por `ApplyBasicAttackNd6`/`ApplySkillEffect` → SkillExecutor → DicePoolEngine (sin CA/CD).
**Estado:** `m_allies/m_enemies` (BattleParticipant: name, combatant, skills, inventory* no propietarios); catálogos legacy + ObjectCatalog para items; `m_outcome`; `m_log`; RNG propio inyectable.
**API:** ctor (valida desenlace defensivo); `resolveAllyAction` (Attack/UseSkill/Item/Flee con validaciones que dejan línea de log); `resolveEnemyTurn` (IA determinista: primera habilidad alfabética pagable, sino básico; objetivo = primer aliado vivo; corta a mitad de ronda si termina); `outcome/allies/enemies/log`; `setRandomEngine`.
**Trampas:** no existe `BattleStateOnegai` como clase (la resolución Nd6 vive aquí); `degreeLabel` devuelve "Fracàs/Èxit/Crític" (catalán con acentos) junto a logs "de daño" — inconsistencia para la BitmapFont (etiquetas ya en la bandeja de Oriol).

### `EnemyBrain` (`include/Game/EnemyBrain.h`) — clase concreta : `ICombatant`
**Qué es:** lo que un enemigo ES sin GL: vida, IA de patrulla, stats. Dual-mode CharacterSheet (`m_sheet`≠nullptr → lee de la hoja).
**Estado:** posición/patrolMin/patrolMax, `m_health/m_maxHealth` (clamp mín 1: maxHealth==0 no nace muerto), `m_aiState` (kIdle/kPatrol/kChase), `m_direction`, `m_stepInterval` (mín 0.01 anti-bucle), stats legacy, `m_sheet` no propietario.
**API:** ctor simple `(pos, patMin, patMax, maxHealth=50, step=0.5)`; `update(dt, canEnter)` con callback; ICombatant completo; `set_character_sheet`; `populate_from_monster_catalog(mon, tierRules, defaultClass)` rellena AMBOS (legacy + sheet con `recompute_derived`, ratio de vida preservado).
**Trampas:** **`populate_from_monster_catalog` solo lo llama `demo_catalogos_rpg.cpp`** — GameSession crea EnemyBrain desde `CombatData` del ObjectCatalog SIN sheet: la vía catálogo RPG→hoja no está cableada en el juego real (fractura #1 a medias). `kChase` existe y nunca se asigna (IA de persecución no implementada).

### `Morality` (`include/Game/Morality.h`) — clase concreta
Escala única [−100,+100], historial de 12. `label()` tramos asimétricos (VILLANO ≤−60, CANALLA ≤−20, NEUTRAL <40, JUSTO <75, HÉROE). **Trampa:** el header promete "sin acentos porque BitmapFont" pero `label()` devuelve `"HÉROE"` con tilde — contradicción interna.

### `Skill` / `SkillSet` / `SkillCatalog` (`include/Game/Skill.h`) — legacy + adaptadores Ed.3
`Skill` POD legacy (id/name/mpCost/power/effect/target); `SkillSet` (learn/knows, mana clampado, ids ordenados alfabéticamente = IA determinista); `SkillCatalog : ICatalog<Skill>`. `ApplyBasicAttackNd6` (DES, magnitudes 0/5/10/15) y `ApplySkillEffect` (Damage→DES, Heal→INT) fabrican un `SkillDefinition` temporal y ejecutan vía SkillExecutor con `override_base_dice=1+stat` y `override_target_dice` = stat de defensa del defensor — **tirada enfrentada pura**. **Trampa:** los comentarios del header aún describen "CD = defense_to_cd(ARMOR_CLASS)" — doc obsoleta frente al .cpp Ed.3.

### `SaveGame` (`include/Game/SaveGame.h`) — POD + funciones libres [?? sin commitear]
**Qué es:** RUN-10. `SaveGameData` DTO (kSchemaVersion=1: levelPath, posición/vida/inventario, oro, moral, negocios, objetos restantes por POSICIÓN con objectId de comprobación, enemigos por `spawnIndex`, flags/beats/timers narrativos).
**API:** `serializeSaveGame` (nunca falla); `parseSaveGame` (rechaza versión/estructura; resto con defaults); `applyNarrativeState` (solo ENCIENDE, idempotente); `writeSaveGameFile` (atómico: .tmp + rename); `readSaveGameFile`. Aquí viven `GameSession::captureSaveGame` (combate a medias se guarda como mundo pre-combate) y `restoreSaveGame` (validación todo-o-nada en dos fases; vida restaurada vía heal-a-max + takeDamage porque ICombatant no tiene setter).
**Trampas:** si el nivel cambió (objeto movido/borrado), el restore se niega EN BLOQUE (correcto pero duro).

## LEVEL

### `LevelDefinition` — POD puro
Datos de un nivel JSON: name, mapPath (TMX referenciado), playerStart, `enemies` (legacy, **nadie lo instancia**) y `objects` (`ObjectSpawn`: objectId/position/patrol, `targetLevel/targetPosition` puertas, `loot/lootTable`, `eventId`, variante local displayName/variant/scale/effectOverride/`properties` map).

### `LevelLoader` — utility estática
JSON→LevelDefinition vía JsonValue con `Result<T>`. Obligatorios: `map` no vacío, `type`/`objectId` por spawn; resto permisivo; patrol sin JSON → min=max=position; propiedades por instancia vía `objectValues()` (solo strings). No valida ids contra catálogos (cosa del consumidor).

### `ObjectCatalog` — catálogo + PODs (Fase 10)
"Todo es un objeto": `ObjectDefinition` plano (id/name/category/spriteId/previewPng/raceId/blocksMovement/interactable + CombatData/PickupData/DialogueData/ShopData/BusinessData según categoría). Decisión 2.9: N paralelo al RPG tipado, NO se unifican. `add` (último gana), `has/find/size` (ICatalog), `ids()/ids(category)/missingPreviewIds()` (ordenados, para paletas del editor), `loadFromString/File` (merge todo-o-nada; categorías estrictas; `price` FUERA del bloque pickup). **Trampa gorda:** **`exportJson()` declarado y comentado pero SIN definición en ningún .cpp** — error de enlazado si alguien lo llama; el demo que verificaba el round-trip ya no lo llama.

### `WorldTopology` — clase header-only [M sin commitear]
Grafo de conectividad entre niveles siguiendo puertas (`targetLevel`) en BFS desde niveles raíz; `load` con límite anti-manifiesto-corrupto; `canReach`. Lo usan `demo_mundo` y tests. **No cableado en Application.** Comentario documenta bug cazado: recorrer `pending` POR VALOR (push_back realoca).

## MAP [?? nuevo, sin commitear]

### `MapDocument` — documento editable de mapa, sin GL
Metadatos (MapId, nombre, dims, tile size), `collisionGids` ordenados, capas. `addLayer` (valida duplicados/orden/límites; inserta por `order` con upper_bound); `setCollisionGids`; `validate` integral. Sin setters de nombre/dimensiones — inmutable salvo capas/colisión.

### `MapLayer` / `TileLayerDocument` / `EntityLayerDocument` / `RegionLayerDocument`
Capa tipada por `std::variant` (Tile/Entity/Region) con `MapLayerMeta` (LayerId, name, visible, locked, opacity∈[0,1], order). TileLayer: GIDs fila-major (0=vacío). EntityLayer: `MapEntityDocument{EntityId, AssetId objectId, GridCoord}` sin ids duplicados. RegionLayer: `MapRegionDocument{RegionId, name, GridBounds}` no vacíos. Factories estáticas + `validate()` por documento. ObjectId "se resolverá contra catálogo en fase posterior".

## WORLD [?? nuevo, namespace Engine]

### `EntityHandle` — POD (index + generation; generación 0 = inválido)
### `World` — mundo runtime mínimo (RUN-04/ADR-010)
`WorldEntity` = {authoringId opcional, enabled} — **sin componentes todavía** (RUN-05). Altas/bajas diferidas: `create` (visible tras flush), `destroy` (invisible ya, memoria tras update), `fixedUpdate(fn)` recorre índices crecientes con flush al inicio y final; deque a propósito (referencias estables ante spawns en iteración). **Solo lo usan tests** — Application no lo integra.

## RUNTIME [?? nuevo, sin commitear]

### `RuntimeTile` — POD compacto {tilesetID, variant, collision}; isEmpty = id 0.
### `RuntimeTileMap` — mapa compilado GL-free
Capas fila-major (`RuntimeTileLayer`: sourceLayerId/visible/opacity) + **overrides de sesión** (`RuntimeTileOverride`): `setTileOverride` valida y aplica en capas + registra; `clearTileOverrides` restaura `m_baseLayers`; `layerAt/getTile` LANZAN out_of_range (aquí vive ahora el contrato que LINEA_BASE atribuía a TileMap); `replaceCompiledData` protegido (solo builder y adaptador). `Render/TileMap` **hereda de ella**. **Deuda:** doble copia de capas (base+actual) asumida; overrides no entran en SaveGameData.

### `RuntimeTileMapBuilder` — frontera DOC→RUN
Valida MapDocument y compila SOLO capas Tile (binary_search contra collisionGids). Sin I/O ni GPU. Capas Entity/Region no se compilan (por diseño).

## IMPORT [?? nuevo, sin commitear]

### `ImportReport`/`ImportIssue` — PODs con severidades (errores → documento inválido; warnings = admitido parcialmente).
### `TmxMapImporter` — adaptador Tiled/TMX → MapDocument (ADR-006)
Mapas finitos, tilesets embebidos, capas tile CSV. `importFile(path, mapId)` valida MapId, raíz `<map>`, dims; colisión desde propiedades `collision` de tiles embebidos; capa con nombre inválido → warning + id fallback; revalida al final. Solo CSV (base64→error); tilesets externos (`source`) → warning "no inspeccionado" (colisión perdida); objectgroup/group/imagelayer → warnings de no representado; orientación no-orthogonal aceptada con warning.

---

## Qué falta / deuda de los módulos

1. **Sin commitear (lo más urgente):** Map/World/Runtime/Import + SaveGame + `tests/` enteros están `??`; GameSession/ObjectCatalog/WorldTopology `M`. Riesgo de colisión para cualquiera que toque lo mismo (regla 2).
2. **`ObjectCatalog::exportJson()` declarado sin definición** — error de enlazado latente.
3. **CharacterSheet dual-mode sin cablear en el juego real** (populate/set_character_sheet solo en demo; GameSession usa `CombatData` legacy): fractura #1 a medias en runtime.
4. **Doc-drift Ed.3** (comentarios de Skill.h con CD/ARMOR_CLASS).
5. **Acentos en cadenas de HUD** (Morality "HÉROE", degreeLabel catalán) — bandeja de Oriol.
6. **Muerto/pendiente:** `kChase` nunca asignado; `LevelDefinition::enemies` sin consumidor; `World` sin integrar; `WorldTopology` sin cablear.
7. **Rentas acopladas a pasos** (40 pasos/ciclo): AFK indistinguible de caminar; overrides de RuntimeTileMap fuera del savegame.
8. **Fragilidades reconocidas:** `sell()` por nombre; restore todo-o-nada si el nivel cambió.
