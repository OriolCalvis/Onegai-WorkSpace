# 08 — Matriz de cobertura y diagnóstico arquitectónico final

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe el análisis clase a clase de agosto, foto del 31/08-03/09/2026. El juicio vigente, clase a clase y sobre el código de hoy, está en `planificacion/juicio_clases/` (12 partes, generado con `tools/mapa_clases.py`). De las 98 citas de codigo que contiene, 44 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

*Cierre verificable de la auditoría clase a clase · 31/08/2026 · contraste directo contra el árbol de trabajo, `CMakeLists.txt` y CTest. Este documento no sustituye los capítulos 01–07: los conecta, confirma su alcance y separa el código integrado del andamiaje aún aislado.*

## Alcance y criterio de "exhaustivo"

La revisión cubre las **101 cabeceras públicas** de `include/`, las **59
unidades de implementación** de `src/`, los ejecutables, ejemplos y pruebas
que las consumen. Para cada tipo público, los capítulos enlazados describen:

- **estado/variables:** miembros, propiedad y punteros no propietarios;
- **funciones:** API pública, precondiciones y resultado de error;
- **flujo:** quién lo crea, quién lo llama y qué muta;
- **herencia:** base, interfaz o composición relevante.

Las estructuras POD, enums y subestructuras anidadas se agrupan junto a la
clase que las consume: repetir para cada DTO que no tiene comportamiento no
aportaría una falsa sensación de profundidad. Las seis ayudas locales de los
`.cpp` también fueron comprobadas: `JsonParseError`/`JsonParser` (parser
recursivo), `GlyphDef`/`GlyphDefX` (tablas 5×7), `Rgb` (color de fog) y
`AnchorFraction` (resolución de anclas HUD). No son API ni tienen ciclo de
vida independiente.

### Foto cuantitativa comprobada

| Elemento | Resultado actual | Cómo se interpreta |
|---|---:|---|
| Cabeceras de producción | 101 | Superficie pública inventariada abajo. |
| `.cpp` de producción | 59 | Código que implementa o adapta esa superficie. |
| Ejecutables explícitos CMake | 48 | Demos, editor y juego; la macro interna de test no cuenta aquí. |
| Ejecutables de unidad generados por macro | 15 | Solo cuando `BUILD_TESTING` está activo. |
| Ejecutables configurables | 63 | 48 + 15, cifra usada por el capítulo 07. |
| CTest actual | 46 | 31 demos GL-free/smoke + 15 pruebas unitarias. |

El estado de Git mostrado en los capítulos es una **foto del worktree**, no
una afirmación de que una pieza forme parte de `HEAD`. En particular,
`Map`, `Runtime`, `World`, `Project`, `Import`, `SaveGame`, los tests y parte
del editor siguen siendo trabajo local no consolidado; el código es la fuente
de verdad cuando difiera de este análisis.

## Matriz de cobertura por cabecera

La columna «análisis» enlaza al lugar que ya contiene las variables,
funciones, flujo y herencia del grupo. La matriz evita que una clase nueva o
una cabecera auxiliar se pierda entre capítulos temáticos.

### Core — 21 cabeceras

| Cabecera | Tipos o contrato cubierto | Análisis |
|---|---|---|
| `Core/Check.h` | `require`, `check_detail::fail` | [01 · Check](01_core.md#check-includecorecheckh) |
| `Core/Errors/EngineException.h` | `EngineException` y cuatro especializaciones | [01 · Errors](01_core.md#errors) |
| `Core/Errors/Result.h` | `Result<T>` | [01 · Result](01_core.md#resultt-coreerrorsresulth--clase-plantilla-dato-con-invariante) |
| `Core/Ids/ProjectIds.h` | ocho tags y aliases de id | [01 · Ids](01_core.md#ids-coreids) |
| `Core/Ids/StrongId.h` | `StrongId<Tag>`, `IdGenerator`, `generate` | [01 · Ids](01_core.md#ids-coreids) |
| `Core/Json/JsonValue.h` | `JsonValue`, `Type` | [01 · Json](01_core.md#json-corejson) |
| `Core/Json/JsonWriter.h` | `JsonWriter` | [01 · JsonWriter](01_core.md#jsonwriter--clase-concreta-header-only-flujo-no-%C3%A1rbol) |
| `Core/Math/GridBounds.h` | `GridBounds` | [01 · Math](01_core.md#math-coremath--todos-pod-struct) |
| `Core/Math/GridCoord.h` | `GridCoord` | [01 · Math](01_core.md#math-coremath--todos-pod-struct) |
| `Core/Math/IsoMath.h` | conversión isométrica | [01 · IsoMath](01_core.md#namespace-isomath-header-only) |
| `Core/Math/UVRect.h` | `UVRect` | [01 · Math](01_core.md#math-coremath--todos-pod-struct) |
| `Core/Math/Vector2.h` | `Vector2` | [01 · Math](01_core.md#math-coremath--todos-pod-struct) |
| `Core/Math/Vector4.h` | `Vector4` | [01 · Math](01_core.md#math-coremath--todos-pod-struct) |
| `Core/Memory/HandlePool.h` | `HandlePool<T>`, `Handle`, `Slot` | [01 · Memory](01_core.md#memory-corememory) |
| `Core/Resources/ICatalog.h` | `ICatalog<T>` | [01 · Resources](01_core.md#resources-coreresources) |
| `Core/Resources/ResourceManager.h` | `ResourceManager<T>` | [01 · Resources](01_core.md#resources-coreresources) |
| `Core/Resources/Shader.h` | `Shader` RAII | [01 · Resources](01_core.md#resources-coreresources) |
| `Core/Resources/ShaderManager.h` | `ShaderManager` | [01 · Resources](01_core.md#resources-coreresources) |
| `Core/Resources/Texture.h` | `Texture` RAII | [01 · Resources](01_core.md#resources-coreresources) |
| `Core/Resources/TextureAtlas.h` | `TextureAtlas` | [01 · Resources](01_core.md#resources-coreresources) |
| `Core/Resources/TextureManager.h` | `TextureManager` | [01 · Resources](01_core.md#resources-coreresources) |

### RPG — 16 cabeceras

| Cabecera | Tipos o contrato cubierto | Análisis |
|---|---|---|
| `RPG/Stat.h` | `Stat`, límites y conversores | [02 · Fundaciones](02_rpg_modelo_de_datos.md#enums-y-pod-fundacionales) |
| `RPG/Defense.h` | `Defense`, `DefenseBlock` | [02 · Fundaciones](02_rpg_modelo_de_datos.md#enums-y-pod-fundacionales) |
| `RPG/TierRules.h` | `ItemRarity`, `TierRule`, `TierRules` | [02 · Fundaciones](02_rpg_modelo_de_datos.md#enums-y-pod-fundacionales) |
| `RPG/Definitions/RpgCoreDefinitions.h` | 19 definiciones y submodelos de catálogo | [02 · Definiciones](02_rpg_modelo_de_datos.md#rpgcoredefinitionsh-19-structs) |
| `RPG/Definitions/SkillDefinition.h` | enums de skill, `SkillDefinition`, presets | [02 · SkillDefinition](02_rpg_modelo_de_datos.md#skilldefinitionh--enums--struct) |
| `RPG/Catalogs/RpgCatalogs.h` | `CatalogLoader<T>`, `Catalog<T>`, 21 aliases | [02 · Catálogos](02_rpg_modelo_de_datos.md#sistema-de-cat%C3%A1logos-rpgcatalogsrpgcatalogsh-header-only) |
| `RPG/DicePoolEngine.h` | pool, grados y resolución Nd6 | [03 · DicePool](03_rpg_motores.md#dicepoolengine-includerpgdicepoolengineh--clase-est%C3%A1tica-header-only) |
| `RPG/CharacterSheet.h` | `EquipSlot`, `ActiveCondition`, `CharacterSheet` | [03 · CharacterSheet](03_rpg_motores.md#charactersheet-includerpgcharactersheeth--cpp--clase-concreta-fuente-%C3%BAnica-del-personaje) |
| `RPG/RandomEngine.h` | `RandomEngine`, adaptadores, `Xoroshiro128p` | [03 · Random](03_rpg_motores.md#randomengine--xoroshiro128p--adaptadores-includerpgrandomengineh--interfaz--3-clases) |
| `RPG/Rng.h` | `IRng`, `SeededRng` legacy | [03 · RNG legacy](03_rpg_motores.md#irng--seededrng-includerpgrngh--interfaz--clase-legacy) |
| `RPG/InventoryEngine.h` | `InventoryEngine` | [03 · Inventory](03_rpg_motores.md#inventoryengine-includerpginventoryengineh--cpp--clase-est%C3%A1tica-stateless) |
| `RPG/ConditionEngine.h` | `ConditionEngine`, `TickEvent` | [03 · Condition](03_rpg_motores.md#conditionengine-includerpgconditionengineh--cpp--clase-est%C3%A1tica-stateless) |
| `RPG/EnhanceHooks.h` | flags, QTE, contexto y `SkillExecutor` | [03 · SkillExecutor](03_rpg_motores.md#skillexecutor--executioncontext--enhancehooks-includerpgenhancehooksh-srcrpgskillexecutorcpp--clase-est%C3%A1tica--pods--interfaz) |
| `RPG/NarrativeEngine.h` | estado, beats, timers, script y motor narrativo | [03 · Narrative](03_rpg_motores.md#narrativeengine--narrativestate--modelos-includerpgnarrativeengineh--cpp--clase--pods) |
| `RPG/CharacterCreation.h` | `CreationChoice`, `CharacterCreation` | [03 · Creation](03_rpg_motores.md#charactercreation-includerpgcharactercreationh--cpp--clase-concreta) |
| `RPG/CharacterCreationScreen.h` | `CharacterCreationScreen` | [03 · Wizard](03_rpg_motores.md#charactercreationscreen-includerpgcharactercreationscreenh--cpp--wizard-gl-free) |

### Render — 25 cabeceras

| Cabecera | Tipos o contrato cubierto | Análisis |
|---|---|---|
| `Render/Camera.h` | `Camera : IUpdatable` | [04 · Camera](04_render.md#camera-includerendercamerah--clase-concreta--iupdatable) |
| `Render/Tile.h` | alias `Tile = RuntimeTile` | [04 · Tile](04_render.md#tile-includerendertileh--alias-de-tipo-wip) |
| `Render/TileMap.h` | `TileMap : RuntimeTileMap` | [04 · TileMap](04_render.md#tilemap-includerendertilemaph--clase-concreta--runtimetilemap-wip) |
| `Render/IRenderable.h` | interfaz `IRenderable` | [04 · Interfaces](04_render.md#irenderable--iupdatable--interfaces) |
| `Render/IUpdatable.h` | interfaz `IUpdatable` | [04 · Interfaces](04_render.md#irenderable--iupdatable--interfaces) |
| `Render/SpriteBatch.h` | `SpriteBatch` | [04 · SpriteBatch](04_render.md#spritebatch-includerenderspritebatchh--clase-concreta-no-copiable) |
| `Render/Framebuffer.h` | `Framebuffer` RAII | [04 · Framebuffer](04_render.md#framebuffer-includerenderframebufferh--raii-movible) |
| `Render/IsometricRenderer.h` | `IsometricRenderer` | [04 · Renderer](04_render.md#isometricrenderer-includerenderisometricrendererh--orquestadora-wip) |
| `Render/Entity.h` | `Entity : IRenderable, IUpdatable` | [04 · Entity](04_render.md#entity-includerenderentityh--base--irenderableiupdatable) |
| `Render/StaticEntity.h` | `StaticEntity : Entity` | [04 · Entidades](04_render.md#entidades) |
| `Render/AnimatedEntity.h` | `AnimatedEntity : Entity` | [04 · Entidades](04_render.md#entidades) |
| `Render/Player.h` | `Player : AnimatedEntity, ICombatant` | [04 · Player](04_render.md#player-includerenderplayerh--) |
| `Render/Enemy.h` | `Enemy : AnimatedEntity, ICombatant` | [04 · Enemy](04_render.md#enemy-includerenderenemyh--) |
| `Render/ICombatant.h` | interfaz `ICombatant` | [04 · Combatant](04_render.md#icombatant-includerendericombatant-h--interfaz) |
| `Render/BitmapFont.h` | `BitmapFont` | [04 · Fuente](04_render.md#fuente-y-hud) |
| `Render/HudElement.h` | `HudAnchor`, `HudTransform`, `IHudElement` | [04 · Fuente y HUD](04_render.md#fuente-y-hud) |
| `Render/HudManager.h` | `HudManager` | [04 · Fuente y HUD](04_render.md#fuente-y-hud) |
| `Render/HudWidgets.h` | `HudBar`, `HudPanel` | [04 · Fuente y HUD](04_render.md#fuente-y-hud) |
| `Render/HudTextWidgets.h` | `HudText`, `HudCommandMenu`, `HudDialogueBox` | [04 · Fuente y HUD](04_render.md#fuente-y-hud) |
| `Render/HudMinimap.h` | `HudMinimap : IHudElement` | [04 · Fuente y HUD](04_render.md#fuente-y-hud) |
| `Render/FogOfWar.h` | `FogOfWar` | [04 · Post-FX](04_render.md#post-procesado) |
| `Render/ColorLUT.h` | `ColorLUT` | [04 · Post-FX](04_render.md#post-procesado) |
| `Render/DynamicLightMap.h` | `PointLight`, `DynamicLightMap` | [04 · Post-FX](04_render.md#post-procesado) |
| `Render/BlobShadow.h` | funciones de sombra blob | [04 · Post-FX](04_render.md#post-procesado) |
| `Render/OcclusionRules.h` | regla pura de oclusión | [04 · Post-FX](04_render.md#post-procesado) |

### Game, Level, Map, Runtime, World e Import — 21 cabeceras

| Cabecera | Tipos o contrato cubierto | Análisis |
|---|---|---|
| `Game/GameSession.h` | modos, transición, oferta, `WorldEnemy`, `GameSession` | [05 · GameSession](05_game_level_map_world.md#gamesession-includegamegamesessionh--clase-concreta-gl-free) |
| `Game/BattleState.h` | acción, outcome, participante, `BattleState` | [05 · BattleState](05_game_level_map_world.md#battlestate-includegamebattlestateh--clase-concreta-gl-free) |
| `Game/EnemyBrain.h` | `EnemyBrain : ICombatant` | [05 · EnemyBrain](05_game_level_map_world.md#enemybrain-includegameenemybrainh--clase-concreta--icombatant) |
| `Game/Morality.h` | `Morality` | [05 · Morality](05_game_level_map_world.md#morality-includegamemoralityh--clase-concreta) |
| `Game/Skill.h` | skill legacy, `SkillSet`, `SkillCatalog`, puente Nd6 | [05 · Skill](05_game_level_map_world.md#skill--skillset--skillcatalog-includegameskillh--legacy--adaptadores-ed3) |
| `Game/SaveGame.h` | DTOs de guardado y funciones de persistencia | [05 · SaveGame](05_game_level_map_world.md#savegame-includegamesavegameh--pod--funciones-libres-sin-commitear) |
| `Level/LevelDefinition.h` | `EnemySpawn`, `ObjectSpawn`, `LevelDefinition` | [05 · Level](05_game_level_map_world.md#level) |
| `Level/LevelLoader.h` | `LevelLoader` | [05 · Level](05_game_level_map_world.md#level) |
| `Level/ObjectCatalog.h` | categorías, DTOs, `ObjectCatalog` | [05 · ObjectCatalog](05_game_level_map_world.md#objectcatalog--cat%C3%A1logo--pods-fase-10) |
| `Level/WorldTopology.h` | nodo, enlace, `WorldTopology` | [05 · WorldTopology](05_game_level_map_world.md#worldtopology--clase-header-only-m-sin-commitear) |
| `Map/MapLayer.h` | tipo/meta de capa, `MapLayer` | [05 · Map](05_game_level_map_world.md#map-nuevo-sin-commitear) |
| `Map/TileLayerDocument.h` | `TileLayerDocument` | [05 · Map](05_game_level_map_world.md#maplayer--tilelayerdocument--entitylayerdocument--regionlayerdocument) |
| `Map/EntityLayerDocument.h` | entidad documental, `EntityLayerDocument` | [05 · Map](05_game_level_map_world.md#maplayer--tilelayerdocument--entitylayerdocument--regionlayerdocument) |
| `Map/RegionLayerDocument.h` | región documental, `RegionLayerDocument` | [05 · Map](05_game_level_map_world.md#maplayer--tilelayerdocument--entitylayerdocument--regionlayerdocument) |
| `Map/MapDocument.h` | `MapDocument` | [05 · MapDocument](05_game_level_map_world.md#mapdocument--documento-editable-de-mapa-sin-gl) |
| `Runtime/RuntimeTile.h` | `RuntimeTile` | [05 · Runtime](05_game_level_map_world.md#runtime-nuevo-sin-commitear) |
| `Runtime/RuntimeTileMap.h` | capas/overrides, `RuntimeTileMap` | [05 · Runtime](05_game_level_map_world.md#runtimetilemap--mapa-compilado-gl-free) |
| `Runtime/RuntimeTileMapBuilder.h` | `RuntimeTileMapBuilder` | [05 · Runtime](05_game_level_map_world.md#runtimetilemapbuilder--frontera-docrun) |
| `World/EntityHandle.h` | `EntityHandle` | [05 · World](05_game_level_map_world.md#world-nuevo-namespace-engine) |
| `World/World.h` | `WorldEntity`, `World` | [05 · World](05_game_level_map_world.md#world--mundo-runtime-m%C3%ADnimo-run-04adr-010) |
| `Import/TmxMapImporter.h` | severidad, reporte, `TmxMapImporter` | [05 · Import](05_game_level_map_world.md#import-nuevo-sin-commitear) |

### Engine, Editor y Project — 18 cabeceras

| Cabecera | Tipos o contrato cubierto | Análisis |
|---|---|---|
| `Engine/Application.h` | `Application` y marcador interno de mundo | [06 · Application](06_engine_editor_proyecto.md#application-includeengineapplicationh--orquestador) |
| `Engine/ApplicationLoop.h` | settings, frame, servicios, `ApplicationLoop` | [06 · Loop](06_engine_editor_proyecto.md#engineapplicationloop-settingsframeservices--n%C3%BAcleo-fixed-step-run-01adr-007) |
| `Engine/Window.h` | `Window` | [06 · Window](06_engine_editor_proyecto.md#window--envoltorio-tonto-de-glfw) |
| `Engine/InputState.h` | `InputState` legacy | [06 · Input](06_engine_editor_proyecto.md#engineinputrawinputstate--inputmap--vocabulario-de-entrada-run-02) |
| `Engine/Input/InputMap.h` | teclas, acciones, raw state, `InputMap` | [06 · Input](06_engine_editor_proyecto.md#engineinputrawinputstate--inputmap--vocabulario-de-entrada-run-02) |
| `Engine/Input/InputSystem.h` | snapshot, estado de acción, `InputSystem` | [06 · Input](06_engine_editor_proyecto.md#engineinputinputsnapshot--inputsystem--foto-de-frame-adr-008) |
| `Engine/Scene/Scene.h` | interfaz `Scene` | [06 · Scene](06_engine_editor_proyecto.md#enginescene-interfaz--scenemanager--run-03adr-009) |
| `Engine/Scene/SceneManager.h` | `SceneManager` | [06 · Scene](06_engine_editor_proyecto.md#enginescene-interfaz--scenemanager--run-03adr-009) |
| `Editor/EditorState.h` | herramientas, tileset y `EditorState` | [06 · EditorState](06_engine_editor_proyecto.md#editorstate--el-modelo-del-nivel-gl-free) |
| `Editor/EditorValidation.h` | resultado y `EditorValidation` | [06 · Validation](06_engine_editor_proyecto.md#editorvalidation-editorvalidationresult--reglas-de-nivel-listo-compartidas-editorci) |
| `Editor/ObjectInspector.h` | `ObjectInspector` | [06 · Inspector](06_engine_editor_proyecto.md#editorobjectinspector--panel-modal-gl-free-de-propiedades-del-objeto-colocado) |
| `Editor/ProjectHub.h` | `ProjectHub` | [06 · ProjectHub](06_engine_editor_proyecto.md#editorprojecthub--pantalla-de-arranque-gl-free) |
| `Editor/ProjectIndex.h` | shim de compatibilidad | [06 · Shim](06_engine_editor_proyecto.md#editorprojectindexh--shim-de-6-l%C3%ADneas-hacia-projectprojectindexh-se-retirar%C3%A1-cuando-no-haya-referencias) |
| `Project/Project.h` | `Project`, `ProjectCheck` | [06 · Project](06_engine_editor_proyecto.md#editorproject--projectcheck--documento-ra%C3%ADz-de-un-pack) |
| `Project/ProjectSettings.h` | `ProjectSettings` | [06 · Project](06_engine_editor_proyecto.md#editorprojectsettings--pod) |
| `Project/DocumentRef.h` | `DocumentKind`, `DocumentRef` | [06 · Project](06_engine_editor_proyecto.md#documentref--referencia-persistente-a-documento-por-idsdocumentid-fuerte-no-por-ruta) |
| `Project/ProjectIndex.h` | `ProjectIndex` | [06 · ProjectIndex](06_engine_editor_proyecto.md#editorprojectindex--los-proyectos-vivos) |
| `Project/ProjectSerializer.h` | `ProjectSerializer` | [06 · Serializer](06_engine_editor_proyecto.md#projectserializer--persistencia-can%C3%B3nica-v1) |

## Herencias y fronteras de propiedad verificadas

No hay jerarquía ECS oculta: la mayor parte del dominio usa composición y PODs.
Las herencias públicas reales son las siguientes:

```text
std::exception ← EngineException ← {ResourceLoad, ShaderCompile, Render, MapParse}Exception
ICatalog<T>    ← ResourceManager<T> ← {ShaderManager, TextureManager}
ICatalog<T>    ← {SkillCatalog, ObjectCatalog, RPG::Catalog<T>}

IUpdatable ← Camera
IRenderable + IUpdatable ← Entity ← {StaticEntity, AnimatedEntity} ← {Player, Enemy}
ICombatant ← {Player, Enemy, EnemyBrain}
IHudElement ← {HudBar, HudPanel, HudText, HudCommandMenu, HudDialogueBox, HudMinimap}
RuntimeTileMap ← TileMap
RandomEngine ← {RngAsRandomEngine, Xoroshiro128p}; IRng ← {RandomEngineAsIRng, SeededRng}
```

Esto revela tres decisiones ya materializadas:

1. Las interfaces de catálogos y combate son los puntos de polimorfismo
   realmente usados; no hay necesidad actual de introducir otra base común de
   «entidad» de juego.
2. La representación visual (`Entity`) y la simulación (`EnemyBrain`,
   `GameSession`) están separadas, pero `Player` sigue siendo una frontera
   híbrida porque mantiene HP y un fallback legacy junto a `CharacterSheet`.
3. `TileMap : RuntimeTileMap` es una adaptación de compatibilidad. `MapDocument`
   es autoría; `RuntimeTileMap` es el producto compilado; no se deben mezclar
   responsabilidades entre ambos.

## Flujos que existen de verdad

```text
assets JSON/TMX
  ├─ RPG::Catalog / LevelLoader ───────────────────────────┐
  └─ TmxMapImporter → MapDocument → RuntimeTileMapBuilder → TileMap
                                                       │
juego → ApplicationLoop → Application → GameSession ──┼→ BattleState → SkillExecutor → DicePoolEngine
                                     │                 └→ NarrativeEngine → NarrativeState / SaveGame
                                     └→ Camera + IsometricRenderer + HUD

editor visual → ProjectHub / ProjectIndex → EditorState → export TMX + Level JSON
                               └→ ProjectSerializer (formato canónico, aún no es la misma ruta)
```

Las flechas anteriores corresponden a consumidores presentes. En contraste,
`SceneManager`, `World`, `WorldTopology` y `DocumentRef` tienen API y pruebas,
pero no consumidor de producción fuera de su implementación propia: son
infraestructura preparada, no parte del bucle de juego actual.

## Diagnóstico final: qué falta, separado por naturaleza

### P0 — coherencia de fuentes de verdad

1. **Unificar el manifiesto de proyecto.** `ProjectIndex` persiste el formato
   legacy español mientras `ProjectSerializer` produce el canónico inglés;
   ambos se leen parcialmente, pero no forman un round-trip único.
2. **Terminar la fuente única de personaje.** El runtime crea enemigos con
   `ObjectCatalog::CombatData`; la ruta `MonsterDefinition → CharacterSheet →
   EnemyBrain` está demostrada pero no conectada al juego. Antes de añadir más
   combate, hay que decidir y aplicar una única representación.
3. **Dejar una sola máquina de condiciones.** `CharacterSheet` aún ofrece
   `applyCondition/tickEndOfRound` y `ConditionEngine` vuelve a aplicar,
   expirar y recalcular modificadores. La duplicación puede divergir en daño,
   duración y cachés.

### P1 — integración de la arquitectura que ya está escrita

4. **Elegir cuándo entra `SceneManager`.** No debe conectarse por decoración:
   primero hay que partir `Application` en escenas reales (hub, exploración,
   combate, diálogo) y migrar un flujo completo; mientras tanto, el monolito
   es la ruta vigente.
5. **Decidir el alcance de `World`.** Su garantía de handles generacionales y
   altas/bajas diferidas está probada, pero no tiene componentes ni uso en
   `GameSession`; integrarlo ahora sin resolver CharacterSheet generaría un
   segundo modelo de entidades.
6. **Cerrar la migración documental Map→Runtime.** Mantener la adaptación
   `TileMap` hasta que editor y juego lean/escriban la ruta canónica. Eliminarla
   antes rompería los metadatos de tileset que el editor necesita para el
   round-trip.
7. **Hacer visible F1.** `m_timerBar` se crea, se oculta y se registra en HUD,
   pero no recibe valor ni vuelve a ser visible. Hace falta una consulta de
   timer visible desde sesión/narrativa y su actualización en `refreshHud`.

### P2 — deuda funcional y código latente

8. **Retirar la ruta CD de Ed.3** (`resolve_against_cd`,
   `resolve_against_defense`, `defense_to_cd` y override asociado) después de
   migrar todos los llamadores y actualizar comentarios.
9. **Resolver las APIs incompletas y código muerto:** `ObjectCatalog::exportJson`
   está declarado sin definición; `SeededRng::nextInt` no está implementado;
   `DicePoolEngine.cpp` es una unidad vacía; `EnemyBrain::kChase` y
   `LevelDefinition::enemies` carecen de consumidor.
10. **Reducir las tres vías de input:** `InputSystem` para juego,
    `InputState` legacy para animación y `KeyEdge` local para editor. El primer
    objetivo razonable es retirar el `InputState` heredado desde `Player`, no
    forzar al editor visual a compartir todavía todos los bindings.
11. **Completar texto/HUD y datos:** `HudText` no limita alto y `clipText` no
    tiene consumidor productivo; los 432 monstruos con `maxHealth == 0`, los
    tiers duplicados y tablas hardcodeadas de Application siguen siendo deuda
    de contenido/configuración.

## Conclusión operativa

El proyecto **no está vacío ni en fase de esqueleto**: tiene un vertical slice
funcional de carga → exploración → combate Nd6 → narrativa → guardado, editor
de niveles y una batería GL-free de 46 ejecuciones. Lo que limita el siguiente
salto no es falta de clases, sino terminar cuatro convergencias: formatos de
proyecto, ficha/personaje, condiciones y entrada de la nueva arquitectura en
el bucle real. Esa es la frontera segura para continuar el Gantt de desarrollo.
