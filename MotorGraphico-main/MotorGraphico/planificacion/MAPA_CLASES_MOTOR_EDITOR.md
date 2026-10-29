# Mapa de clases de `motor_editor`, desde arriba

Generado por `tools/mapa_clases.py`. **No se edita a mano:** se regenera, que es lo unico que evita que mienta. El analisis escrito de `analisis_clases/` es del 31/08 y cubria 101 cabeceras; esto sale del arbol de hoy.

`motor_editor` enlaza **148** fuentes y alcanza **132** ficheros. De ellos **76** son cabeceras con **112** clases o structs.

Si una clase no esta aqui, este ejecutable no la alcanza.


## Core — 27 cabeceras, 39 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Core/Json/JsonWriter.h` | 179 | 5 | `JsonWriter` | C-10 | — |
| `include/Core/Json/JsonValue.h` | 122 | 6 | `JsonValue` | - | — |
| `include/Core/Errors/Result.h` | 99 | 3 | `Result`, `Result` | - | — |
| `include/Core/Log/Logger.h` | 95 | 6 | `Logger` | - | — |
| `include/Core/Log/LogRecord.h` | 91 | — | `LogRecord` | - | — |
| `include/Core/Resources/ResourceManager.h` | 91 | 1 | `ResourceManager` | - | — |
| `include/Core/Ids/StrongId.h` | 88 | 1 | `StrongId`, `IdGenerator`, `hash` | - | — |
| `include/Core/Errors/ScopeGuard.h` | 61 | 2 | `ScopeGuard` | - | — |
| `include/Core/Errors/EngineException.h` | 59 | 3 | `EngineException`, `ResourceLoadException`, `ShaderCompileException`, `RenderException`, `MapParseException` | - | — |
| `include/Core/Resources/Texture.h` | 54 | 3 | `Texture` | C-09 | — |
| `include/Core/Log/LogConfig.h` | 51 | 3 | `LogConfig` | - | — |
| `include/Core/Log/FileSink.h` | 43 | 2 | `FileSink` | - | — |
| `include/Core/Ids/ProjectIds.h` | 42 | — | `ProjectIdTag`, `DocumentIdTag`, `MapIdTag`, `LayerIdTag`, `AssetIdTag`, `EntityIdTag`, `RegionIdTag`, `EventIdTag` | - | — |
| `include/Core/Math/IsoMath.h` | 42 | — | — | - | — |
| `include/Core/Resources/TextureAtlas.h` | 41 | 4 | `TextureAtlas` | - | — |
| `include/Core/Resources/ICatalog.h` | 40 | — | `ICatalog` | - | — |
| `include/Core/Resources/Shader.h` | 34 | 1 | `Shader` | - | — |
| `include/Core/Log/ILogSink.h` | 30 | — | `ILogSink` | - | — |
| `include/Core/Log/ConsoleSink.h` | 29 | 1 | `ConsoleSink` | - | — |
| `include/Core/Log/LogFormat.h` | 25 | — | — | - | — |
| `include/Core/Math/Vector4.h` | 17 | — | `Vector4` | - | — |
| `include/Core/Math/GridBounds.h` | 16 | — | `GridBounds` | - | — |
| `include/Core/Math/Vector2.h` | 13 | — | `Vector2` | - | — |
| `include/Core/Math/UVRect.h` | 12 | — | `UVRect` | - | — |
| `include/Core/Resources/ShaderManager.h` | 12 | — | `ShaderManager` | - | — |
| `include/Core/Resources/TextureManager.h` | 12 | — | `TextureManager` | - | — |
| `include/Core/Math/GridCoord.h` | 10 | — | `GridCoord` | - | — |

## Render — 13 cabeceras, 15 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Render/IsometricRenderer.h` | 189 | 9 | `IsometricRenderer` | C-19, C-23 | — |
| `include/Render/TileMap.h` | 108 | 11 | `TileMap` | C-19 | — |
| `include/Render/Camera.h` | 104 | 14 | `Camera` | - | — |
| `include/Render/FogOfWar.h` | 97 | 8 | `FogOfWar` | - | — |
| `include/Render/SpriteBatch.h` | 93 | 3 | `SpriteBatch` | - | — |
| `include/Render/ColorLUT.h` | 69 | 3 | `ColorLUT` | - | — |
| `include/Render/Culling.h` | 65 | — | — | - | — |
| `include/Render/Framebuffer.h` | 62 | 5 | `Framebuffer` | - | — |
| `include/Render/IsometricPostProcessData.h` | 58 | — | `IsometricLightingData`, `IsometricFogData`, `IsometricLutData`, `IsometricShadowData`, `IsometricPostProcessData` | C-23 | — |
| `include/Render/IRenderable.h` | 35 | — | `IRenderable` | - | tiene (void)param: posible stub |
| `include/Render/IsometricSceneData.h` | 29 | — | `IsometricSceneData` | C-19 | — |
| `include/Render/OcclusionRules.h` | 28 | — | — | - | — |
| `include/Render/IUpdatable.h` | 23 | — | `IUpdatable` | C-19 | — |

## editor — 8 cabeceras, 9 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `editor/include/EditorVistaDeMapa.h` | 118 | — | `MapView` | - | — |
| `editor/include/EditorOpciones.h` | 54 | — | `EditorOpciones` | - | — |
| `editor/include/ToolsPanel.h` | 54 | 2 | `ToolsContext`, `ToolsPanel` | - | — |
| `editor/include/EditorIconos.h` | 51 | 6 | `EditorIconos` | - | — |
| `editor/include/EditorGui.h` | 35 | — | `GuiContext` | - | — |
| `editor/include/InspectorPanel.h` | 29 | 1 | `InspectorPanel` | - | — |
| `editor/include/LayerPanel.h` | 25 | 5 | `LayerPanel` | - | — |
| `editor/include/EditorApplication.h` | 20 | — | `EditorApplication` | E-10 | — |

## Editor — 6 cabeceras, 9 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Editor/EditorState.h` | 394 | 23 | `TmxTilesetSettings`, `EditorState` | C-14, G-02, G-03 | 23 miembros |
| `include/Editor/EditorDocumentSession.h` | 94 | 5 | `EditorDocumentSession`, `ViewportInput` | E-10 | — |
| `include/Editor/EditorActions.h` | 53 | 3 | `ActionDefinition`, `ActionInput`, `EditorActions` | - | — |
| `include/Editor/EditorAyuda.h` | 34 | — | `AyudaAccion` | - | — |
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

## HUD — 1 cabeceras, 1 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/HUD/Iconos.h` | 47 | 5 | `Iconos` | - | — |

## Import — 1 cabeceras, 3 clases

| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |
| --- | ---: | ---: | --- | --- | --- |
| `include/Import/TmxMapImporter.h` | 58 | — | `ImportIssue`, `ImportReport`, `TmxMapImporter` | - | — |
