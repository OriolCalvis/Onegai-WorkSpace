# Mapa de clases de `level_editor`, desde arriba

Generado por `tools/mapa_clases.py`. **No se edita a mano:** se regenera, que es lo unico que evita que mienta. El analisis escrito de `analisis_clases/` es del 31/08 y cubria 101 cabeceras; esto sale del arbol de hoy.

`level_editor` enlaza **133** fuentes y alcanza **135** ficheros. De ellos **81** son cabeceras con **126** clases o structs.

Si una clase no esta aqui, este ejecutable no la alcanza.


## Core — 20 cabeceras, 32 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Core/Json/JsonWriter.h` | 179 | 5 | `JsonWriter` | C-10 | — |
| `include/Core/Json/JsonValue.h` | 122 | 6 | `JsonValue` | - | — |
| `include/Core/Errors/Result.h` | 99 | 3 | `Result`, `Result` | - | — |
| `include/Core/Resources/ResourceManager.h` | 91 | 1 | `ResourceManager` | - | — |
| `include/Core/Ids/StrongId.h` | 88 | 1 | `StrongId`, `IdGenerator`, `hash` | - | — |
| `include/Core/Errors/EngineException.h` | 59 | 3 | `EngineException`, `ResourceLoadException`, `ShaderCompileException`, `RenderException`, `MapParseException` | - | — |
| `include/Core/Resources/Texture.h` | 54 | 3 | `Texture` | C-09 | — |
| `include/Core/Check.h` | 50 | — | — | - | — |
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

## Render — 15 cabeceras, 19 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Render/BitmapFont.h` | 144 | 4 | `BitmapFont` | - | — |
| `include/Render/HudTextWidgets.h` | 127 | 3 | `HudText`, `HudCommandMenu`, `HudDialogueBox` | G-14 | — |
| `include/Render/HudElement.h` | 116 | 1 | `HudTransform`, `IHudElement` | C-19 | — |
| `include/Render/HudWidgets.h` | 116 | 2 | `HudBar`, `HudPanel` | G-14 | — |
| `include/Render/TileMap.h` | 108 | 11 | `TileMap` | C-19 | — |
| `include/Render/Camera.h` | 104 | 14 | `Camera` | - | — |
| `include/Render/SpriteBatch.h` | 93 | 3 | `SpriteBatch` | - | — |
| `include/Render/HudManager.h` | 68 | 1 | `HudManager` | - | — |
| `include/Render/Culling.h` | 65 | — | — | - | — |
| `include/Render/ICombatant.h` | 59 | — | `ICombatant` | - | — |
| `include/Render/HudTextWidgetsData.h` | 55 | — | `HudTextData`, `HudCommandMenuData`, `HudDialogueBoxData` | G-14 | — |
| `include/Render/HudWidgetsData.h` | 51 | — | `HudBarData`, `HudPanelData` | G-14 | — |
| `include/Render/FormatoPPM.h` | 36 | — | — | - | — |
| `include/Render/CapturaGL.h` | 29 | — | — | - | — |
| `include/Render/IUpdatable.h` | 23 | — | `IUpdatable` | C-19 | — |

## RPG — 11 cabeceras, 18 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/RPG/CharacterSheet.h` | 236 | — | `ActiveCondition`, `CharacterSheet` | C-20, D-06 | — |
| `include/RPG/Definitions/SkillDefinition.h` | 181 | — | `SkillDefinition`, `MagnitudePreset` | - | — |
| `include/RPG/DicePoolEngine.h` | 143 | — | `PoolResult`, `OpposedOutcome`, `DicePoolEngine` | C-22 | — |
| `include/RPG/EnhanceHooks.h` | 138 | — | `ExecutionContext`, `SkillExecutor` | C-22, D-09 | — |
| `include/RPG/Definitions/DefinicionesPersonaje.h` | 132 | — | `ClassDefinition`, `RaceDefinition`, `BackgroundDefinition` | - | — |
| `include/RPG/TierRules.h` | 115 | 1 | `TierRule`, `TierRules` | C-22 | — |
| `include/RPG/SkillEffect.h` | 101 | — | `SkillEffectDef` | D-11 | — |
| `include/RPG/RandomEngine.h` | 88 | 1 | `RandomEngine`, `Xoroshiro128p` | C-21 | — |
| `include/RPG/Defense.h` | 45 | — | `DefenseBlock` | - | — |
| `include/RPG/Stat.h` | 38 | — | — | - | — |
| `include/RPG/SkillTarget.h` | 28 | — | — | - | — |

## Editor — 7 cabeceras, 9 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Editor/EditorState.h` | 394 | 23 | `TmxTilesetSettings`, `EditorState` | C-14, G-02, G-03 | 23 miembros |
| `include/Editor/ObjectInspectorInput.h` | 330 | — | `ObjectInspectorInput` | E-12, E-14 | — |
| `include/Editor/EditorValidation.h` | 188 | — | `EditorValidationResult`, `EditorValidation` | - | — |
| `include/Editor/ObjectInspector.h` | 116 | 3 | `ObjectInspector` | E-12, E-14 | — |
| `include/Editor/ObjectInspectorData.h` | 84 | — | `ObjectInspectorRow`, `ObjectInspectorData` | E-12, E-14 | — |
| `include/Editor/EditorStateHelpers.h` | 22 | — | — | - | — |
| `include/Editor/EditorDocumentIO.h` | 17 | — | `EditorDocumentIO` | - | — |

## Map — 6 cabeceras, 13 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Map/MapDocument.h` | 188 | 14 | `MapDocument` | C-12, E-01, F-03 | — |
| `include/Map/EventLayerDocument.h` | 180 | 1 | `MapEventCondition`, `MapEventCommand`, `MapEventPage`, `MapEventDocument`, `EventLayerDocument` | E-01 | — |
| `include/Map/EntityLayerDocument.h` | 121 | 1 | `MapEntityDocument`, `EntityLayerDocument` | C-13 | — |
| `include/Map/MapLayer.h` | 81 | 2 | `MapLayerMeta`, `MapLayer` | C-12 | — |
| `include/Map/TileLayerDocument.h` | 45 | 3 | `TileLayerDocument` | - | — |
| `include/Map/RegionLayerDocument.h` | 37 | 1 | `MapRegionDocument`, `RegionLayerDocument` | - | — |

## Project — 6 cabeceras, 6 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Project/ProjectIndex.h` | 185 | 4 | `ProjectIndex` | E-11 | — |
| `include/Project/SerializerHelpers.h` | 89 | — | — | - | — |
| `include/Project/Project.h` | 53 | — | `Project`, `ProjectCheck` | - | — |
| `include/Project/ProjectSettings.h` | 48 | — | `ProjectSettings` | - | — |
| `include/Project/ProjectCatalog.h` | 35 | — | `ProjectCatalog` | D-05 | — |
| `include/Project/ProjectSerializer.h` | 26 | — | `ProjectSerializer` | - | — |

## editor — 6 cabeceras, 5 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `editor/include/LevelEditorPaleta.h` | 91 | — | `TilePaletteInfo`, `RaceNpcPreview` | - | — |
| `editor/include/LevelEditorEditState.h` | 87 | 11 | `LevelEditorEditState` | - | — |
| `editor/include/LevelEditorInterno.h` | 64 | — | `KeyEdge` | - | — |
| `editor/include/LevelEditorOpciones.h` | 42 | — | `EditorOptions` | - | — |
| `editor/include/LevelEditorHub.h` | 39 | — | — | - | — |
| `editor/include/LevelEditorArranque.h` | 17 | — | — | - | — |

## Level — 4 cabeceras, 12 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Level/ObjectCatalog.h` | 211 | 1 | `CombatData`, `PickupData`, `ShopItem`, `ShopData`, `DialogueData`, `BusinessData`, `ObjectDefinition`, `ObjectCatalog` | - | — |
| `include/Level/LevelDefinition.h` | 141 | — | `EnemySpawn`, `ObjectSpawn`, `LevelDefinition` | - | — |
| `include/Level/LevelDocumentBridge.h` | 81 | — | — | - | — |
| `include/Level/LevelLoader.h` | 27 | — | `LevelLoader` | - | — |

## Runtime — 3 cabeceras, 4 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Runtime/RuntimeTileMap.h` | 74 | 5 | `RuntimeTileLayer`, `RuntimeTileMap` | C-09, C-22 | — |
| `include/Runtime/RuntimeTile.h` | 21 | 3 | `RuntimeTile` | - | — |
| `include/Runtime/RuntimeTileMapBuilder.h` | 14 | — | `RuntimeTileMapBuilder` | - | — |

## Engine — 1 cabeceras, 1 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Engine/Window.h` | 43 | 3 | `Window` | - | — |

## Game — 1 cabeceras, 4 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Game/Skill.h` | 195 | 4 | `Skill`, `SkillSet`, `SkillCatalog`, `SkillApplyResult` | C-01 | — |

## Import — 1 cabeceras, 3 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Import/TmxMapImporter.h` | 58 | — | `ImportIssue`, `ImportReport`, `TmxMapImporter` | - | — |
