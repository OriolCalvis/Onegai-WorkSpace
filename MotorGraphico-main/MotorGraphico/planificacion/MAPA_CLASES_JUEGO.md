# Mapa de clases de `juego`, desde arriba

Generado por `tools/mapa_clases.py`. **No se edita a mano:** se regenera, que es lo unico que evita que mienta. El analisis escrito de `analisis_clases/` es del 31/08 y cubria 101 cabeceras; esto sale del arbol de hoy.

`juego` enlaza **128** fuentes y alcanza **240** ficheros. De ellos **142** son cabeceras con **260** clases o structs.

Si una clase no esta aqui, este ejecutable no la alcanza.


## Render — 35 cabeceras, 43 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Render/Player.h` | 273 | 1 | `Player` | G-12 | — |
| `include/Render/IsometricRenderer.h` | 189 | 9 | `IsometricRenderer` | C-19, C-23 | — |
| `include/Render/BitmapFont.h` | 144 | 4 | `BitmapFont` | - | — |
| `include/Render/HudTextWidgets.h` | 127 | 3 | `HudText`, `HudCommandMenu`, `HudDialogueBox` | G-14 | — |
| `include/Render/Entity.h` | 116 | 12 | `Entity` | - | — |
| `include/Render/HudElement.h` | 116 | 1 | `HudTransform`, `IHudElement` | C-19 | — |
| `include/Render/HudWidgets.h` | 116 | 2 | `HudBar`, `HudPanel` | G-14 | — |
| `include/Render/TileMap.h` | 108 | 11 | `TileMap` | C-19 | — |
| `include/Render/Camera.h` | 104 | 14 | `Camera` | - | — |
| `include/Render/HudMinimap.h` | 101 | 11 | `HudMinimap` | C-02 | — |
| `include/Render/FogOfWar.h` | 97 | 8 | `FogOfWar` | - | — |
| `include/Render/SpriteBatch.h` | 93 | 3 | `SpriteBatch` | - | — |
| `include/Render/DynamicLightMap.h` | 92 | 4 | `PointLight`, `DynamicLightMap` | - | — |
| `include/Render/HudIconList.h` | 70 | 7 | `HudIconRow`, `HudIconList` | - | — |
| `include/Render/ColorLUT.h` | 69 | 3 | `ColorLUT` | - | — |
| `include/Render/HudManager.h` | 68 | 1 | `HudManager` | - | — |
| `include/Render/AnimatedEntity.h` | 67 | 5 | `AnimatedEntity` | - | — |
| `include/Render/Culling.h` | 65 | — | — | - | — |
| `include/Render/Framebuffer.h` | 62 | 5 | `Framebuffer` | - | — |
| `include/Render/ICombatant.h` | 59 | — | `ICombatant` | - | — |
| `include/Render/IsometricPostProcessData.h` | 58 | — | `IsometricLightingData`, `IsometricFogData`, `IsometricLutData`, `IsometricShadowData`, `IsometricPostProcessData` | C-23 | — |
| `include/Render/HudTextWidgetsData.h` | 55 | — | `HudTextData`, `HudCommandMenuData`, `HudDialogueBoxData` | G-14 | — |
| `include/Render/HudWidgetsData.h` | 51 | — | `HudBarData`, `HudPanelData` | G-14 | — |
| `include/Render/HudFicha.h` | 50 | 5 | `HudFicha` | - | — |
| `include/Render/HudIconos.h` | 37 | 2 | `HudIconos` | - | — |
| `include/Render/PlayerData.h` | 37 | — | `PlayerData` | G-12 | — |
| `include/Render/FormatoPPM.h` | 36 | — | — | - | — |
| `include/Render/BlobShadow.h` | 35 | — | — | - | — |
| `include/Render/IRenderable.h` | 35 | — | `IRenderable` | - | tiene (void)param: posible stub |
| `include/Render/CapturaGL.h` | 29 | — | — | - | — |
| `include/Render/IsometricSceneData.h` | 29 | — | `IsometricSceneData` | C-19 | — |
| `include/Render/OcclusionRules.h` | 28 | — | — | - | — |
| `include/Render/StaticEntity.h` | 25 | — | `StaticEntity` | - | — |
| `include/Render/IUpdatable.h` | 23 | — | `IUpdatable` | C-19 | — |
| `include/Render/PlayerMotionData.h` | 18 | — | `PlayerMotionData` | - | — |

## RPG — 29 cabeceras, 68 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/RPG/EventInterpreter.h` | 323 | 10 | `CommandResult`, `EventHost`, `EventRun`, `TraceEntry`, `Breakpoint`, `EventInterpreter` | E-03 | la base la da por MUERTA pero el ejecutable la alcanza; tiene (void)param: posible stub |
| `include/RPG/CharacterSheet.h` | 236 | — | `ActiveCondition`, `CharacterSheet` | C-20, D-06 | — |
| `include/RPG/Catalogs/RpgCatalogs.h` | 212 | 1 | `Catalog` | D-01, D-02, D-03, D-04, D-05, D-06, D-07, E-16 | — |
| `include/RPG/EventCommand.h` | 209 | 3 | `CommandParamSpec`, `CommandSpec`, `ActivePageResult`, `NarrativeCommandHost` | E-04 | la base la da por MUERTA pero el ejecutable la alcanza |
| `include/RPG/Catalogs/CatalogLoader.h` | 193 | — | `CatalogField`, `CatalogLoader` | - | — |
| `include/RPG/Narrative/NarrativeBeat.h` | 185 | — | `NarrativeCondition`, `BeatTrigger`, `BattleRequest`, `SkillCheckRequest`, `NarrativeEffect`, `NarrativeBeat` | - | — |
| `include/RPG/Definitions/SkillDefinition.h` | 181 | — | `SkillDefinition`, `MagnitudePreset` | - | — |
| `include/RPG/Narrative/NarrativeState.h` | 170 | 5 | `TimerState`, `NarrativeState` | - | — |
| `include/RPG/Definitions/DefinicionesContenido.h` | 164 | — | `AdventureDefinition`, `EventDefinition`, `LocationDefinition`, `EncounterTableDefinition` | - | — |
| `include/RPG/Definitions/DefinicionesCriaturas.h` | 155 | — | `MonsterDefinition`, `SummonDefinition`, `MountDefinition` | - | — |
| `include/RPG/NarrativeEngine.h` | 153 | 1 | `NarrativeResult`, `NarrativeEngine` | E-02, G-07 | — |
| `include/RPG/Definitions/DefinicionesObjetos.h` | 151 | — | `ConditionDefinition`, `EquipmentDefinition`, `ConsumableDefinition` | - | — |
| `include/RPG/CharacterCreation.h` | 145 | 5 | `CreationChoice`, `CharacterCreation` | - | la base la da por MUERTA pero el ejecutable la alcanza |
| `include/RPG/DicePoolEngine.h` | 143 | — | `PoolResult`, `OpposedOutcome`, `DicePoolEngine` | C-22 | — |
| `include/RPG/CharacterCreationScreen.h` | 139 | 10 | `CharacterCreationScreen` | G-09 | la base la da por MUERTA pero el ejecutable la alcanza |
| `include/RPG/EnhanceHooks.h` | 138 | — | `ExecutionContext`, `SkillExecutor` | C-22, D-09 | — |
| `include/RPG/Definitions/DefinicionesRasgos.h` | 134 | — | `PassiveDefinition`, `FeatDefinition`, `TraitDefinition`, `DeityDefinition` | - | — |
| `include/RPG/Definitions/DefinicionesPersonaje.h` | 132 | — | `ClassDefinition`, `RaceDefinition`, `BackgroundDefinition` | - | — |
| `include/RPG/Definitions/DefinicionesMundo.h` | 122 | — | `TrapDefinition`, `LootTableDefinition`, `NpcDefinition` | - | — |
| `include/RPG/Narrative/AdventureScript.h` | 119 | — | `ObjectiveDefinition`, `ObjectiveStatus`, `TimerDefinition`, `AdventureScript` | - | — |
| `include/RPG/TierRules.h` | 115 | 1 | `TierRule`, `TierRules` | C-22 | — |
| `include/RPG/InventoryEngine.h` | 109 | — | `InventoryEngine` | - | — |
| `include/RPG/SkillEffect.h` | 101 | — | `SkillEffectDef` | D-11 | — |
| `include/RPG/ConditionEngine.h` | 95 | — | `ConditionEngine` | - | — |
| `include/RPG/SessionCommandHost.h` | 92 | 4 | `SessionCommandHost` | - | la base la da por MUERTA pero el ejecutable la alcanza |
| `include/RPG/RandomEngine.h` | 88 | 1 | `RandomEngine`, `Xoroshiro128p` | C-21 | — |
| `include/RPG/Defense.h` | 45 | — | `DefenseBlock` | - | — |
| `include/RPG/Stat.h` | 38 | — | — | - | — |
| `include/RPG/SkillTarget.h` | 28 | — | — | - | — |

## Game — 26 cabeceras, 57 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Game/GameSession.h` | 545 | 1 | `GameSession` | C-03 | techo 546 |
| `include/Game/EnemyBrain.h` | 296 | 1 | `EnemyBrain` | D-08 | — |
| `include/Game/BattleState.h` | 214 | 10 | `BattleAction`, `BattleParticipant`, `BattleState` | - | — |
| `include/Game/Skill.h` | 195 | 4 | `Skill`, `SkillSet`, `SkillCatalog`, `SkillApplyResult` | C-01 | — |
| `include/Game/SaveGame.h` | 175 | — | `SavedWorldObject`, `SavedEnemy`, `SaveGameData` | - | — |
| `include/Game/SaveSlotState.h` | 169 | 4 | `SaveSlot`, `SaveSlots`, `SaveGate` | G-11 | la base la da por MUERTA pero el ejecutable la alcanza |
| `include/Game/OptionsLocaleBridge.h` | 132 | — | `OptionsLocaleBridge` | - | tiene (void)param: posible stub |
| `include/Game/InventoryMenuState.h` | 119 | 9 | `ItemAction`, `InventoryRow`, `EquipmentSlotRow`, `InventoryMenuState` | - | la base la da por MUERTA pero el ejecutable la alcanza |
| `include/Game/InventoryScreen.h` | 116 | 12 | `InventoryEntry`, `InventoryScreen` | - | — |
| `include/Game/FichaVista.h` | 113 | 2 | `FichaStat`, `FichaRanura`, `FichaRasgo`, `FichaCondicion`, `FichaDatos`, `FichaCatalogos`, `FichaVista` | - | — |
| `include/Game/TitleMenuState.h` | 109 | 12 | `TitleMenuItem`, `TitleMenuState` | - | la base la da por MUERTA pero el ejecutable la alcanza |
| `include/Game/EnemyCatalogBridge.h` | 105 | — | — | - | — |
| `include/Game/SessionTipos.h` | 98 | — | `OwnedBusiness`, `PlayerCarryOver`, `LevelTransition`, `ShopOffer` | - | — |
| `include/Game/PlayerControls.h` | 94 | — | `PlayerControls` | - | — |
| `include/Game/SessionData.h` | 91 | 40 | `InventoryRules`, `SessionData` | - | 40 miembros |
| `include/Game/BattleMenuActions.h` | 89 | — | `BattleMenuEntry`, `TargetOption` | - | — |
| `include/Game/OptionsState.h` | 83 | 6 | `OptionsEntry`, `OptionsState` | - | — |
| `include/Game/NewGameFlow.h` | 75 | 3 | `NewGameFlow` | - | — |
| `include/Game/Codice.h` | 66 | 1 | `CodiceEntrada`, `CodiceDatos`, `Codice` | - | — |
| `include/Game/EnemyBrainData.h` | 65 | — | `EnemyBrainData` | - | — |
| `include/Game/Morality.h` | 64 | 2 | `Morality` | - | — |
| `include/Game/MenuLogic.h` | 60 | 2 | `MenuLogic` | - | — |
| `include/Game/DiarioMisiones.h` | 53 | — | `DiarioObjetivo`, `DiarioMision`, `DiarioDatos` | - | — |
| `include/Game/PauseMenu.h` | 41 | 2 | `PauseEntry`, `PauseMenu` | - | — |
| `include/Game/GameOverScreen.h` | 38 | 2 | `GameOverEntry`, `GameOverScreen` | - | — |
| `include/Game/PlayerControlsData.h` | 17 | — | `PlayerControlsData` | - | — |

## Core — 19 cabeceras, 32 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Core/Json/JsonWriter.h` | 179 | 5 | `JsonWriter` | C-10 | — |
| `include/Core/Json/JsonValue.h` | 122 | 6 | `JsonValue` | - | — |
| `include/Core/Errors/Result.h` | 99 | 3 | `Result`, `Result` | - | — |
| `include/Core/Resources/ResourceManager.h` | 91 | 1 | `ResourceManager` | - | — |
| `include/Core/Ids/StrongId.h` | 88 | 1 | `StrongId`, `IdGenerator`, `hash` | - | — |
| `include/Core/Errors/EngineException.h` | 59 | 3 | `EngineException`, `ResourceLoadException`, `ShaderCompileException`, `RenderException`, `MapParseException` | - | — |
| `include/Core/Resources/Texture.h` | 54 | 3 | `Texture` | C-09 | — |
| `include/Core/Ids/ProjectIds.h` | 42 | — | `ProjectIdTag`, `DocumentIdTag`, `MapIdTag`, `LayerIdTag`, `AssetIdTag`, `EntityIdTag`, `RegionIdTag`, `EventIdTag` | - | — |
| `include/Core/Math/IsoMath.h` | 42 | — | — | - | — |
| `include/Core/Resources/TextureAtlas.h` | 41 | 4 | `TextureAtlas` | - | — |
| `include/Core/Resources/ICatalog.h` | 40 | — | `ICatalog` | - | — |
| `include/Core/Resources/Shader.h` | 34 | 1 | `Shader` | - | — |
| `include/Core/Math/Vector4.h` | 17 | — | `Vector4` | - | — |
| `include/Core/Math/GridBounds.h` | 16 | — | `GridBounds` | - | — |
| `include/Core/Math/Vector2.h` | 13 | — | `Vector2` | - | — |
| `include/Core/Math/UVRect.h` | 12 | — | `UVRect` | - | — |
| `include/Core/Resources/ShaderManager.h` | 12 | — | `ShaderManager` | - | — |
| `include/Core/Resources/TextureManager.h` | 12 | — | `TextureManager` | - | — |
| `include/Core/Math/GridCoord.h` | 10 | — | `GridCoord` | - | — |

## Engine — 12 cabeceras, 18 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Engine/Application.h` | 520 | 109 | `Application` | C-05, C-07 | techo 519; 109 miembros |
| `include/Engine/Locale.h` | 168 | 4 | `LocalizedText`, `PlaceholderMismatch`, `Locale` | G-17 | la base la da por MUERTA pero el ejecutable la alcanza |
| `include/Engine/PantallaNuevaPartida.h` | 86 | 13 | `PantallaNuevaPartida` | - | — |
| `include/Engine/ApplicationInternal.h` | 84 | — | — | - | — |
| `include/Engine/Input/InputMap.h` | 79 | 2 | `RawInputState`, `InputMap` | - | — |
| `include/Engine/Scene/SceneManager.h` | 72 | 2 | `SceneManager` | C-16 | — |
| `include/Engine/ApplicationLoop.h` | 65 | 2 | `ApplicationLoopSettings`, `ApplicationLoopFrame`, `ApplicationLoopServices`, `ApplicationLoop` | - | — |
| `include/Engine/Input/InputSystem.h` | 56 | 3 | `InputSnapshot`, `InputSystem` | - | — |
| `include/Engine/MenuJugadorHud.h` | 52 | — | `InventoryOverlay` | - | — |
| `include/Engine/Window.h` | 43 | 3 | `Window` | - | — |
| `include/Engine/Scene/Scene.h` | 37 | — | `Scene` | - | — |
| `include/Engine/InputState.h` | 14 | — | `InputState` | - | — |

## Map — 6 cabeceras, 13 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Map/MapDocument.h` | 188 | 14 | `MapDocument` | C-12, E-01, F-03 | — |
| `include/Map/EventLayerDocument.h` | 180 | 1 | `MapEventCondition`, `MapEventCommand`, `MapEventPage`, `MapEventDocument`, `EventLayerDocument` | E-01 | — |
| `include/Map/EntityLayerDocument.h` | 121 | 1 | `MapEntityDocument`, `EntityLayerDocument` | C-13 | — |
| `include/Map/MapLayer.h` | 81 | 2 | `MapLayerMeta`, `MapLayer` | C-12 | — |
| `include/Map/TileLayerDocument.h` | 45 | 3 | `TileLayerDocument` | - | — |
| `include/Map/RegionLayerDocument.h` | 37 | 1 | `MapRegionDocument`, `RegionLayerDocument` | - | — |

## Project — 5 cabeceras, 5 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Project/ProjectIndex.h` | 185 | 4 | `ProjectIndex` | E-11 | — |
| `include/Project/SerializerHelpers.h` | 89 | — | — | - | — |
| `include/Project/Project.h` | 53 | — | `Project`, `ProjectCheck` | - | — |
| `include/Project/ProjectSettings.h` | 48 | — | `ProjectSettings` | - | — |
| `include/Project/ProjectSerializer.h` | 26 | — | `ProjectSerializer` | - | — |

## Level — 3 cabeceras, 12 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Level/ObjectCatalog.h` | 211 | 1 | `CombatData`, `PickupData`, `ShopItem`, `ShopData`, `DialogueData`, `BusinessData`, `ObjectDefinition`, `ObjectCatalog` | - | — |
| `include/Level/LevelDefinition.h` | 141 | — | `EnemySpawn`, `ObjectSpawn`, `LevelDefinition` | - | — |
| `include/Level/LevelLoader.h` | 27 | — | `LevelLoader` | - | — |

## Runtime — 3 cabeceras, 4 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Runtime/RuntimeTileMap.h` | 74 | 5 | `RuntimeTileLayer`, `RuntimeTileMap` | C-09, C-22 | — |
| `include/Runtime/RuntimeTile.h` | 21 | 3 | `RuntimeTile` | - | — |
| `include/Runtime/RuntimeTileMapBuilder.h` | 14 | — | `RuntimeTileMapBuilder` | - | — |

## World — 2 cabeceras, 4 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/World/World.h` | 202 | 6 | `WorldEnemy`, `WorldEntity`, `World` | C-17 | — |
| `include/World/EntityHandle.h` | 26 | — | `EntityHandle` | - | — |

## HUD — 1 cabeceras, 1 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/HUD/Iconos.h` | 47 | 5 | `Iconos` | - | — |

## Import — 1 cabeceras, 3 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Import/TmxMapImporter.h` | 58 | — | `ImportIssue`, `ImportReport`, `TmxMapImporter` | - | — |
