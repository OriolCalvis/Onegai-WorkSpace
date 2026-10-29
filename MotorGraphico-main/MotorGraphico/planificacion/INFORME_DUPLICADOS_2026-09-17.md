# Informe de duplicados, código muerto y disímilares

Generado por `tools/analizar_duplicados.py` sobre `/Users/admin/Documents/Documentos - Oriol Os (2)/Software/MotorGraphico-main/MotorGraphico`. 1001 ficheros analizados. Alcance real disponible: sí.

Umbrales: clon 0.70 · duplicado 0.90 · firma 0.55 · nombre 0.72. Ningún hallazgo es una sentencia: cada uno lleva su evidencia y la regla que lo clasifica (`VEREDICTOS` en el script).

| Tipo | Hallazgos | Severidad |
| --- | ---: | --- |
| `DUPLICADO` | 6 | alta |
| `CLON_ESTRUCTURAL` | 3 | baja |
| `ENVOLTORIO` | 1 | media |
| `NOMBRE_AMBIGUO` | 1 | media |
| `LOGICA_EN_EL_TEST` | 2 | alta |
| `REGISTRO_CERRADO_SIN_ALCANCE` | 2 | alta |
| `MUERTO` | 24 | media |
| `CABECERA_ENVOLTORIO` | 3 | baja |
| `DOC_DESACTUALIZADO` | 25 | baja |
| `DISIMILAR` | 60 | info |
## Los 10 pares de scripts más parecidos (por debajo del umbral)

Ninguno llega a `SOLAPAMIENTO_FUNCIONAL` (0.55). Se listan para que el resultado negativo también tenga números: si el más parecido del árbol es 0.405, «hay herramientas duplicadas» es falso hoy — y deja de serlo en cuanto uno de estos suba.

| Firma | Script A | Script B |
| ---: | --- | --- |
| 0.405 | `tools/enganchar_boundington.py` | `tools/enganchar_cuadrantes.py` |
| 0.360 | `tools/conectividad.py` | `tools/validar_enlaces.py` |
| 0.358 | `tools/enganchar_cuadrantes.py` | `tools/unir_barrios.py` |
| 0.342 | `tools/enganchar_boundington.py` | `tools/unir_barrios.py` |

## 1. Duplicados exactos (mismo contenido en dos sitios)

*Regla:* Mismo código en dos sitios. Se borra uno y el otro queda como único dueño; si los dos tienen consumidores, se extrae el común y ambos delegan.

### DUPLICADO-ca0b89f6 — assets/world_experiment/claude/oeste_norte/connections.json · assets/world_experiment/trae/oeste_sur/connections.json · assets/world_experiment/zcode/este_norte/connections.json

- **bytes:** 24
- **roles:** `contenido`
- **sha256:** 655c02e7a35cb593
- **similitud:** 1.0
- **afectación de `assets/world_experiment/claude/oeste_norte/connections.json`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `assets/world_experiment/trae/oeste_sur/connections.json`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `assets/world_experiment/zcode/este_norte/connections.json`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DUPLICADO-e7dbeeac — assets/world_experiment/claude/oeste_norte/cultures.json · assets/world_experiment/trae/oeste_sur/cultures.json · assets/world_experiment/zcode/este_norte/cultures.json

- **bytes:** 21
- **roles:** `contenido`
- **sha256:** 8ff26b91979c9b69
- **similitud:** 1.0
- **afectación de `assets/world_experiment/claude/oeste_norte/cultures.json`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `assets/world_experiment/trae/oeste_sur/cultures.json`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `assets/world_experiment/zcode/este_norte/cultures.json`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DUPLICADO-0a27285a — assets/world_experiment/claude/oeste_norte/factions.json · assets/world_experiment/trae/oeste_sur/factions.json · assets/world_experiment/zcode/este_norte/factions.json

- **bytes:** 21
- **roles:** `contenido`
- **sha256:** bd715dec3885076b
- **similitud:** 1.0
- **afectación de `assets/world_experiment/claude/oeste_norte/factions.json`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `assets/world_experiment/trae/oeste_sur/factions.json`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `assets/world_experiment/zcode/este_norte/factions.json`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DUPLICADO-1b5e8a2f — assets/world_experiment/claude/oeste_norte/history.json · assets/world_experiment/trae/oeste_sur/history.json · assets/world_experiment/zcode/este_norte/history.json

- **bytes:** 19
- **roles:** `contenido`
- **sha256:** 1a46c0402c164e45
- **similitud:** 1.0
- **afectación de `assets/world_experiment/claude/oeste_norte/history.json`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `assets/world_experiment/trae/oeste_sur/history.json`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `assets/world_experiment/zcode/este_norte/history.json`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DUPLICADO-07b91239 — assets/world_experiment/claude/oeste_norte/locations.json · assets/world_experiment/trae/oeste_sur/locations.json · assets/world_experiment/zcode/este_norte/locations.json

- **bytes:** 22
- **roles:** `contenido`
- **sha256:** 3d799e50689c4f75
- **similitud:** 1.0
- **afectación de `assets/world_experiment/claude/oeste_norte/locations.json`:** 0 consumidores de producción, 4 pruebas, llega a nadie, paquetes —, radio 4
- **afectación de `assets/world_experiment/trae/oeste_sur/locations.json`:** 0 consumidores de producción, 4 pruebas, llega a nadie, paquetes —, radio 4
- **afectación de `assets/world_experiment/zcode/este_norte/locations.json`:** 0 consumidores de producción, 4 pruebas, llega a nadie, paquetes —, radio 4

### DUPLICADO-3bf74daa — map_entity_fields_round_trip.json · map_entity_fields_round_trip_2.json

- **bytes:** 1914
- **roles:** `otro`
- **sha256:** 680b239424654ed5
- **similitud:** 1.0
- **afectación de `map_entity_fields_round_trip.json`:** 0 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 1
- **afectación de `map_entity_fields_round_trip_2.json`:** 0 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 1

## 2b. Misma forma con otros nombres (plantilla repetida)

*Regla:* Misma forma con otros nombres. O es copia-y-renombra del mismo asunto, o es una plantilla repetida con asuntos distintos: se distingue leyendo los nombres. Si son 3 o más con la misma forma, la salida es una plantilla o un generador, no borrar ficheros.

### CLON_ESTRUCTURAL-738ba851 — examples/TextAssetManager.h · include/Core/Resources/ShaderManager.h

- **coincide_por:** tras renombrado
- **contencion:** 1.0
- **jaccard:** 1.0
- **jaccard_literal:** 0.236
- **jaccard_tras_renombrado:** 1.0
- **lineas:** {"examples/TextAssetManager.h": 10, "include/Core/Resources/ShaderManager.h": 12}
- **roles:** `produccion`, `prueba`
- **shingles_comunes:** 13
- **similitud:** 1.0
- **afectación de `examples/TextAssetManager.h`:** 0 consumidores de producción, 2 pruebas, llega a nadie, paquetes —, radio 2
- **afectación de `include/Core/Resources/ShaderManager.h`:** 4 consumidores de producción, 11 pruebas, llega a juego, motor_editor, level_editor, paquetes —, radio 15
  - consumidores: `editor/src/EditorApplication.cpp`, `editor/src/level_editor.cpp`, `include/Engine/Application.h`, `src/Core/Resources/ShaderManager.cpp`

### CLON_ESTRUCTURAL-bd5db263 — examples/TextAssetManager.h · include/Core/Resources/TextureManager.h

- **coincide_por:** tras renombrado
- **contencion:** 1.0
- **jaccard:** 1.0
- **jaccard_literal:** 0.236
- **jaccard_tras_renombrado:** 1.0
- **lineas:** {"examples/TextAssetManager.h": 10, "include/Core/Resources/TextureManager.h": 12}
- **roles:** `produccion`, `prueba`
- **shingles_comunes:** 13
- **similitud:** 1.0
- **afectación de `examples/TextAssetManager.h`:** 0 consumidores de producción, 2 pruebas, llega a nadie, paquetes —, radio 2
- **afectación de `include/Core/Resources/TextureManager.h`:** 4 consumidores de producción, 8 pruebas, llega a juego, motor_editor, level_editor, paquetes —, radio 12
  - consumidores: `editor/src/EditorApplication.cpp`, `editor/src/level_editor.cpp`, `include/Engine/Application.h`, `src/Core/Resources/TextureManager.cpp`

### CLON_ESTRUCTURAL-8cc30c18 — include/Core/Resources/ShaderManager.h · include/Core/Resources/TextureManager.h

- **coincide_por:** tras renombrado
- **contencion:** 1.0
- **jaccard:** 1.0
- **jaccard_literal:** 0.236
- **jaccard_tras_renombrado:** 1.0
- **lineas:** {"include/Core/Resources/ShaderManager.h": 12, "include/Core/Resources/TextureManager.h": 12}
- **roles:** `produccion`
- **shingles_comunes:** 13
- **similitud:** 1.0
- **afectación de `include/Core/Resources/ShaderManager.h`:** 4 consumidores de producción, 11 pruebas, llega a juego, motor_editor, level_editor, paquetes —, radio 15
  - consumidores: `editor/src/EditorApplication.cpp`, `editor/src/level_editor.cpp`, `include/Engine/Application.h`, `src/Core/Resources/ShaderManager.cpp`
- **afectación de `include/Core/Resources/TextureManager.h`:** 4 consumidores de producción, 8 pruebas, llega a juego, motor_editor, level_editor, paquetes —, radio 12
  - consumidores: `editor/src/EditorApplication.cpp`, `editor/src/level_editor.cpp`, `include/Engine/Application.h`, `src/Core/Resources/TextureManager.cpp`

## 3. Envoltorios (uno cabe dentro del otro)

*Regla:* A es un subconjunto de B. Legítimo si A delega de verdad (una línea que llama a B); es deuda si A reimplementa. El informe dice cuál de los dos casos es.

### ENVOLTORIO-55c736f2 — include/Render/Culling.h · tests/Render/OcclusionTests.cpp

- **coincide_por:** literal
- **contencion:** 0.841
- **delega_de_verdad:** False
- **jaccard:** 0.222
- **jaccard_literal:** 0.222
- **jaccard_tras_renombrado:** 0.007
- **lineas:** {"include/Render/Culling.h": 65, "tests/Render/OcclusionTests.cpp": 174}
- **mayor:** tests/Render/OcclusionTests.cpp
- **menor:** include/Render/Culling.h
- **roles:** `produccion`, `prueba`
- **shingles_comunes:** 270
- **similitud:** 0.222
- **por qué:** include/Render/Culling.h cabe al 84% dentro de tests/Render/OcclusionTests.cpp y tiene la mitad de líneas. No menciona a OcclusionTests.cpp: parece reimplementación, no envoltorio.
- **afectación de `include/Render/Culling.h`:** 2 consumidores de producción, 1 pruebas, llega a juego, motor_editor, level_editor, paquetes —, radio 3
  - consumidores: `src/Render/IsometricRenderer.cpp`, `src/Render/TileMap.cpp`
- **afectación de `tests/Render/OcclusionTests.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

## 4b. El mismo nombre de tipo en dos módulos distintos

*Regla:* El mismo nombre corto en dos módulos distintos (Render::Entity y Editor::Entity). No es un conflicto de compilación, es una trampa de lectura: en un árbol que escriben varios agentes a la vez, el nombre dice de quién es la responsabilidad. Se renombra uno con el prefijo de su módulo.

### NOMBRE_AMBIGUO-29cf8860 — include/Game/Skill.h · include/RPG/Definitions/SkillDefinition.h

- **ambitos:** ``, `RPG`
- **declarado_en:** `include/Game/Skill.h:32 (::)`, `include/RPG/Definitions/SkillDefinition.h:42 (RPG::)`
- **tipo:** SkillTarget
- **afectación de `include/Game/Skill.h`:** 9 consumidores de producción, 12 pruebas, llega a juego, level_editor, paquetes —, radio 21
  - consumidores: `include/Editor/EditorValidation.h`, `include/Engine/Application.h`, `include/Game/EnemyCatalogBridge.h`, `include/Game/GameSession.h`, `include/Render/Player.h`, `include/World/World.h`, `src/Game/BattleState.cpp`, `src/Game/Skill.cpp`
- **afectación de `include/RPG/Definitions/SkillDefinition.h`:** 5 consumidores de producción, 9 pruebas, llega a juego, level_editor, paquetes —, radio 14
  - consumidores: `include/RPG/Catalogs/RpgCatalogs.h`, `include/RPG/CharacterSheet.h`, `src/Game/GameSession.cpp`, `src/Game/Skill.cpp`, `src/RPG/SkillDefinitionEnums.cpp`

## 6b. Paquetes cerrados cuya lógica vive dentro del test

*Regla:* El registro del paquete promete unos nombres que solo están definidos dentro de su fichero de prueba: la prueba se prueba a sí misma y ningún ejecutable puede llamarlos. Se mueve la lógica a src/ y se reabre el registro.

### LOGICA_EN_EL_TEST-fa44e77e — tests/Game/G13CombatCommandsTests.cpp

- **estado:** closed
- **nombres_que_solo_define_el_test:** `BattleMenuEntry`, `buildBattleMenuSkillActions`, `selectTargetsFor`, `translateToBattleAction`
- **paquete:** G-13
- **registro:** planificacion/seguimiento/G-13.json
- **por qué:** G-13 está cerrado y su registro promete `BattleMenuEntry`, `buildBattleMenuSkillActions`, `selectTargetsFor`, `translateToBattleAction`, pero esos nombres solo están definidos en `tests/Game/G13CombatCommandsTests.cpp`: ningún ejecutable puede llamarlos.
- **afectación de `tests/Game/G13CombatCommandsTests.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes G-13, radio 0

### LOGICA_EN_EL_TEST-6270e9a5 — tests/Game/G14HudTimersAndCommerceTests.cpp

- **estado:** closed
- **nombres_que_solo_define_el_test:** `HudTimer`, `ShopEngine`
- **paquete:** G-14
- **registro:** planificacion/seguimiento/G-14.json
- **por qué:** G-14 está cerrado y su registro promete `HudTimer`, `ShopEngine`, pero esos nombres solo están definidos en `tests/Game/G14HudTimersAndCommerceTests.cpp`: ningún ejecutable puede llamarlos.
- **afectación de `tests/Game/G14HudTimersAndCommerceTests.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes G-14, radio 0

## 7. Registros cerrados que no llegan a nadie

*Regla:* El seguimiento dice cerrado y el producto no llega a nadie. Se reabre el registro: la regla de cierre la decide Oriol (ver decisiones/D0006).

### REGISTRO_CERRADO_SIN_ALCANCE-2a0e1c61 — include/Engine/Locale.h · src/Engine/Locale.cpp

- **estado:** closed
- **ficheros_que_no_llegan:** `include/Engine/Locale.h`, `src/Engine/Locale.cpp`
- **ficheros_totales_del_paquete:** 2
- **paquete:** G-17
- **por qué:** G-17 está cerrado y 2 de sus 2 ficheros de producto no los usa ningún ejecutable.
- **afectación de `include/Engine/Locale.h`:** 1 consumidores de producción, 2 pruebas, llega a nadie, paquetes —, radio 3
  - consumidores: `src/Engine/Locale.cpp`
- **afectación de `src/Engine/Locale.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### REGISTRO_CERRADO_SIN_ALCANCE-ee2c6a1d — include/RPG/CharacterCreationScreen.h

- **estado:** cerrado
- **ficheros_que_no_llegan:** `include/RPG/CharacterCreationScreen.h`
- **ficheros_totales_del_paquete:** 2
- **paquete:** G-09
- **por qué:** G-09 está cerrado y 1 de sus 2 ficheros de producto no los usa ningún ejecutable.
- **afectación de `include/RPG/CharacterCreationScreen.h`:** 1 consumidores de producción, 3 pruebas, llega a nadie, paquetes —, radio 4
  - consumidores: `src/RPG/CharacterCreationScreen.cpp`

## 8. Código muerto (ningún ejecutable lo alcanza)

*Regla:* Ningún ejecutable lo alcanza. O se cablea (y entonces hace falta la tarea), o se retira. Dejarlo cuesta: cada sesión lo vuelve a leer y a dudar.

### MUERTO-b77c004a — editor/include/ProblemsPanel.h · editor/src/ProblemsPanel.cpp

- **implementacion:** `editor/src/ProblemsPanel.cpp`
- **lineas:** 85
- **n_pruebas_que_lo_usan:** 0
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `editor/include/ProblemsPanel.h`:** 1 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 1
  - consumidores: `editor/src/ProblemsPanel.cpp`
- **afectación de `editor/src/ProblemsPanel.cpp`:** 1 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 1
  - consumidores: `include/Editor/ProblemsState.h`

### MUERTO-ae2671ab — editor/src/event_editor.cpp · include/Editor/CommandListState.h

- **implementacion:** `editor/src/event_editor.cpp`
- **lineas:** 84
- **n_pruebas_que_lo_usan:** 1
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `editor/src/event_editor.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `include/Editor/CommandListState.h`:** 1 consumidores de producción, 2 pruebas, llega a nadie, paquetes —, radio 3
  - consumidores: `editor/src/event_editor.cpp`

### MUERTO-30647d2f — editor/src/event_editor.cpp · include/Editor/EventEditorState.h

- **implementacion:** `editor/src/event_editor.cpp`
- **lineas:** 93
- **n_pruebas_que_lo_usan:** 1
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `editor/src/event_editor.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `include/Editor/EventEditorState.h`:** 3 consumidores de producción, 2 pruebas, llega a nadie, paquetes —, radio 5
  - consumidores: `editor/src/event_editor.cpp`, `include/Editor/CommandListState.h`, `include/Editor/ProjectDatabaseState.h`

### MUERTO-c48dabc5 — include/Core/Memory/HandlePool.h

- **implementacion:** —
- **lineas:** 179
- **n_pruebas_que_lo_usan:** 2
- **paquetes:** `C-18`
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/Core/Memory/HandlePool.h`:** 0 consumidores de producción, 2 pruebas, llega a nadie, paquetes C-18, radio 2

### MUERTO-06331370 — include/Editor/AssetBrowserState.h

- **implementacion:** —
- **lineas:** 215
- **n_pruebas_que_lo_usan:** 0
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/Editor/AssetBrowserState.h`:** 1 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 2
  - consumidores: `src/Editor/AssetBrowserState.cpp`

### MUERTO-d634fbcc — include/Editor/CatalogPickerState.h · src/Editor/CatalogPickerState.cpp

- **implementacion:** `src/Editor/CatalogPickerState.cpp`
- **lineas:** 135
- **n_pruebas_que_lo_usan:** 1
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/Editor/CatalogPickerState.h`:** 2 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 3
  - consumidores: `include/Editor/ProjectDatabaseState.h`, `src/Editor/CatalogPickerState.cpp`
- **afectación de `src/Editor/CatalogPickerState.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-1a0ecd6a — include/Editor/ProblemsState.h · src/Editor/ProblemsState.cpp

- **implementacion:** `src/Editor/ProblemsState.cpp`
- **lineas:** 277
- **n_pruebas_que_lo_usan:** 1
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/Editor/ProblemsState.h`:** 3 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 4
  - consumidores: `editor/include/ProblemsPanel.h`, `src/Editor/AssetBrowserState.cpp`, `src/Editor/ProblemsState.cpp`
- **afectación de `src/Editor/ProblemsState.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-3603f19b — include/Editor/ProjectDatabaseState.h · src/Project/ProjectDatabaseState.cpp

- **implementacion:** `src/Project/ProjectDatabaseState.cpp`
- **lineas:** 78
- **n_pruebas_que_lo_usan:** 1
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/Editor/ProjectDatabaseState.h`:** 1 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 2
  - consumidores: `src/Project/ProjectDatabaseState.cpp`
- **afectación de `src/Project/ProjectDatabaseState.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-be0ea972 — include/Editor/ProjectIndex.h

- **implementacion:** —
- **lineas:** 6
- **n_pruebas_que_lo_usan:** 0
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/Editor/ProjectIndex.h`:** 6 consumidores de producción, 5 pruebas, llega a nadie, paquetes —, radio 11
  - consumidores: `editor/src/EditorApplication.cpp`, `editor/src/level_editor.cpp`, `include/Editor/ProjectHub.h`, `include/Engine/Application.h`, `src/Editor/AssetBrowserState.cpp`, `src/Project/ProjectIndex.cpp`

### MUERTO-2a0e1c61 — include/Engine/Locale.h · src/Engine/Locale.cpp

- **implementacion:** `src/Engine/Locale.cpp`
- **lineas:** 167
- **n_pruebas_que_lo_usan:** 2
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/Engine/Locale.h`:** 1 consumidores de producción, 2 pruebas, llega a nadie, paquetes —, radio 3
  - consumidores: `src/Engine/Locale.cpp`
- **afectación de `src/Engine/Locale.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-dc348ec0 — include/Game/InventoryMenuState.h · src/Game/InventoryMenuState.cpp

- **implementacion:** `src/Game/InventoryMenuState.cpp`
- **lineas:** 121
- **n_pruebas_que_lo_usan:** 1
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/Game/InventoryMenuState.h`:** 1 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 2
  - consumidores: `src/Game/InventoryMenuState.cpp`
- **afectación de `src/Game/InventoryMenuState.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-620cbbdd — include/Game/SaveSlotState.h · src/Game/SaveSlotState.cpp

- **implementacion:** `src/Game/SaveSlotState.cpp`
- **lineas:** 165
- **n_pruebas_que_lo_usan:** 2
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/Game/SaveSlotState.h`:** 2 consumidores de producción, 2 pruebas, llega a nadie, paquetes —, radio 4
  - consumidores: `src/Game/SaveSlotState.cpp`, `src/Game/TitleMenuState.cpp`
- **afectación de `src/Game/SaveSlotState.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-0c7b9af5 — include/Game/TitleMenuState.h · src/Game/TitleMenuState.cpp

- **implementacion:** `src/Game/TitleMenuState.cpp`
- **lineas:** 108
- **n_pruebas_que_lo_usan:** 1
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/Game/TitleMenuState.h`:** 1 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 2
  - consumidores: `src/Game/TitleMenuState.cpp`
- **afectación de `src/Game/TitleMenuState.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-293522f6 — include/Level/WorldTopology.h

- **implementacion:** —
- **lineas:** 137
- **n_pruebas_que_lo_usan:** 2
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/Level/WorldTopology.h`:** 0 consumidores de producción, 2 pruebas, llega a nadie, paquetes —, radio 2

### MUERTO-99f92e6f — include/Project/DocumentRef.h

- **implementacion:** —
- **lineas:** 29
- **n_pruebas_que_lo_usan:** 1
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/Project/DocumentRef.h`:** 0 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 1

### MUERTO-c86c3f39 — include/RPG/AdventureValidation.h · src/RPG/AdventureValidation.cpp

- **implementacion:** `src/RPG/AdventureValidation.cpp`
- **lineas:** 130
- **n_pruebas_que_lo_usan:** 2
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/RPG/AdventureValidation.h`:** 2 consumidores de producción, 2 pruebas, llega a nadie, paquetes —, radio 4
  - consumidores: `include/Editor/ProblemsState.h`, `src/RPG/AdventureValidation.cpp`
- **afectación de `src/RPG/AdventureValidation.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-06ecdc23 — include/RPG/Audio.h · src/RPG/Audio.cpp

- **implementacion:** `src/RPG/Audio.cpp`
- **lineas:** 131
- **n_pruebas_que_lo_usan:** 1
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/RPG/Audio.h`:** 1 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 2
  - consumidores: `src/RPG/Audio.cpp`
- **afectación de `src/RPG/Audio.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-24eaa760 — include/RPG/CharacterCreation.h · src/RPG/CharacterCreation.cpp

- **implementacion:** `src/RPG/CharacterCreation.cpp`
- **lineas:** 144
- **n_pruebas_que_lo_usan:** 4
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/RPG/CharacterCreation.h`:** 2 consumidores de producción, 3 pruebas, llega a nadie, paquetes —, radio 5
  - consumidores: `include/RPG/CharacterCreationScreen.h`, `src/RPG/CharacterCreation.cpp`
- **afectación de `src/RPG/CharacterCreation.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-bcd9beb8 — include/RPG/CharacterCreationScreen.h · src/RPG/CharacterCreationScreen.cpp

- **implementacion:** `src/RPG/CharacterCreationScreen.cpp`
- **lineas:** 138
- **n_pruebas_que_lo_usan:** 3
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/RPG/CharacterCreationScreen.h`:** 1 consumidores de producción, 3 pruebas, llega a nadie, paquetes —, radio 4
  - consumidores: `src/RPG/CharacterCreationScreen.cpp`
- **afectación de `src/RPG/CharacterCreationScreen.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-b3c74edc — include/RPG/EventCommand.h · src/RPG/EventCommands.cpp

- **implementacion:** `src/RPG/EventCommands.cpp`
- **lineas:** 208
- **n_pruebas_que_lo_usan:** 4
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/RPG/EventCommand.h`:** 4 consumidores de producción, 3 pruebas, llega a nadie, paquetes —, radio 7
  - consumidores: `include/RPG/SessionCommandHost.h`, `src/Editor/ProblemsState.cpp`, `src/RPG/AdventureValidation.cpp`, `src/RPG/EventCommands.cpp`
- **afectación de `src/RPG/EventCommands.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-df8597a7 — include/RPG/EventInterpreter.h · src/RPG/EventInterpreter.cpp

- **implementacion:** `src/RPG/EventInterpreter.cpp`
- **lineas:** 322
- **n_pruebas_que_lo_usan:** 5
- **paquetes:** `E-03`
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/RPG/EventInterpreter.h`:** 2 consumidores de producción, 3 pruebas, llega a nadie, paquetes E-03, radio 5
  - consumidores: `include/RPG/EventCommand.h`, `src/RPG/EventInterpreter.cpp`
- **afectación de `src/RPG/EventInterpreter.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes E-03, radio 0

### MUERTO-5e6d9984 — include/RPG/LootResolver.h · src/RPG/LootResolver.cpp

- **implementacion:** `src/RPG/LootResolver.cpp`
- **lineas:** 21
- **n_pruebas_que_lo_usan:** 1
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/RPG/LootResolver.h`:** 1 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 2
  - consumidores: `src/RPG/LootResolver.cpp`
- **afectación de `src/RPG/LootResolver.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-9a1dfb63 — include/RPG/SessionCommandHost.h · src/RPG/SessionCommandHost.cpp

- **implementacion:** `src/RPG/SessionCommandHost.cpp`
- **lineas:** 91
- **n_pruebas_que_lo_usan:** 2
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/RPG/SessionCommandHost.h`:** 1 consumidores de producción, 2 pruebas, llega a nadie, paquetes —, radio 3
  - consumidores: `src/RPG/SessionCommandHost.cpp`
- **afectación de `src/RPG/SessionCommandHost.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### MUERTO-51dd80d9 — include/Render/Enemy.h · src/Render/Enemy.cpp

- **implementacion:** `src/Render/Enemy.cpp`
- **lineas:** 88
- **n_pruebas_que_lo_usan:** 6
- **paquetes:** —
- **paquetes_que_lo_dan_por_cerrado:** —
- **afectación de `include/Render/Enemy.h`:** 4 consumidores de producción, 7 pruebas, llega a nadie, paquetes —, radio 11
  - consumidores: `include/Game/EnemyBrain.h`, `include/Render/Entity.h`, `include/Render/ICombatant.h`, `src/Render/Enemy.cpp`
- **afectación de `src/Render/Enemy.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

## 9. Cabeceras que solo reexportan

*Regla:* Cabecera que solo reexporta otra. Se mantiene si existe por compatibilidad de rutas y se dice en un comentario; si no, los consumidores incluyen la real.

### CABECERA_ENVOLTORIO-ddac1fb7 — examples/Check.h

- **contenido_extra:** 
- **lineas:** 5
- **reexporta:** `Core/Check.h`
- **afectación de `examples/Check.h`:** 1 consumidores de producción, 43 pruebas, llega a nadie, paquetes —, radio 44
  - consumidores: `editor/src/level_editor.cpp`

### CABECERA_ENVOLTORIO-be0ea972 — include/Editor/ProjectIndex.h

- **contenido_extra:** 
- **lineas:** 7
- **reexporta:** `Project/ProjectIndex.h`
- **afectación de `include/Editor/ProjectIndex.h`:** 6 consumidores de producción, 5 pruebas, llega a nadie, paquetes —, radio 11
  - consumidores: `editor/src/EditorApplication.cpp`, `editor/src/level_editor.cpp`, `include/Editor/ProjectHub.h`, `include/Engine/Application.h`, `src/Editor/AssetBrowserState.cpp`, `src/Project/ProjectIndex.cpp`

### CABECERA_ENVOLTORIO-ae589636 — include/Render/Tile.h

- **contenido_extra:** using Tile = RuntimeTile;
- **lineas:** 8
- **reexporta:** `Runtime/RuntimeTile.h`
- **afectación de `include/Render/Tile.h`:** 1 consumidores de producción, 0 pruebas, llega a motor_editor, paquetes —, radio 1
  - consumidores: `include/Render/TileMap.h`

## 10. Documentación desactualizada

*Regla:* El doc cita código que cambió después. O se actualiza, o se fecha y se declara histórico en la cabecera (como la Parte 0 de la bitácora).

### DOC_DESACTUALIZADO-5353c597 — ARCHITECTURE.md

- **cambiado_despues:** 7
- **codigo_citado:** 13
- **ejemplos:** `editor/src/EditorApplication.cpp`, `examples/Check.h`, `examples/ScriptedRng.h`, `include/Game/EnemyBrain.h`, `include/Render/Tile.h`, `include/Render/TileMap.h`, `src/Game/GameSession.cpp`
- **por qué:** ARCHITECTURE.md cita 13 ficheros de código y 7 cambiaron después de su última revisión.
- **afectación de `ARCHITECTURE.md`:** 1 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 1
  - consumidores: `include/Render/ICombatant.h`

### DOC_DESACTUALIZADO-c3c9eb3a — AUDITORIA_CONTRATOS.md

- **cambiado_despues:** 17
- **codigo_citado:** 21
- **ejemplos:** `examples/demo_editor_state.cpp`, `include/Editor/EditorValidation.h`, `include/Editor/ProjectIndex.h`, `include/Game/Skill.h`, `include/Level/LevelDefinition.h`, `include/Level/ObjectCatalog.h`, `include/RPG/Catalogs/RpgCatalogs.h`, `include/RPG/CharacterSheet.h`, `include/RPG/Definitions/SkillDefinition.h`, `include/RPG/NarrativeEngine.h`
- **por qué:** AUDITORIA_CONTRATOS.md cita 21 ficheros de código y 17 cambiaron después de su última revisión.
- **afectación de `AUDITORIA_CONTRATOS.md`:** 0 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 1

### DOC_DESACTUALIZADO-d018360e — AUDITORIA_UI_ETIQUETAS.md

- **cambiado_despues:** 11
- **codigo_citado:** 18
- **ejemplos:** `editor/src/EditorApplication.cpp`, `editor/src/level_editor.cpp`, `examples/creacion_personaje.cpp`, `examples/demo_editor_state.cpp`, `examples/juego.cpp`, `src/Game/BattleState.cpp`, `src/Game/GameSession.cpp`, `src/Project/ProjectIndex.cpp`, `tools/build_proyecto.py`, `tools/test_verificar_controles.py`
- **por qué:** AUDITORIA_UI_ETIQUETAS.md cita 18 ficheros de código y 11 cambiaron después de su última revisión.
- **afectación de `AUDITORIA_UI_ETIQUETAS.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-34253343 — COMPARATIVA_CODIGO_UML_PLANIFICACION.md

- **cambiado_despues:** 7
- **codigo_citado:** 7
- **ejemplos:** `editor/src/EditorApplication.cpp`, `editor/src/level_editor.cpp`, `include/RPG/Catalogs/RpgCatalogs.h`, `planificacion/pla/dades.py`, `src/Editor/EditorState.cpp`, `src/Engine/Application.cpp`, `src/Game/GameSession.cpp`
- **por qué:** COMPARATIVA_CODIGO_UML_PLANIFICACION.md cita 7 ficheros de código y 7 cambiaron después de su última revisión.
- **afectación de `COMPARATIVA_CODIGO_UML_PLANIFICACION.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-79cf46c2 — ESTUDIO_EDITOR_RPG.md

- **cambiado_despues:** 3
- **codigo_citado:** 4
- **ejemplos:** `tools/build_proyecto.py`, `tools/test_verificar_controles.py`, `tools/validar_todo.py`
- **por qué:** ESTUDIO_EDITOR_RPG.md cita 4 ficheros de código y 3 cambiaron después de su última revisión.
- **afectación de `ESTUDIO_EDITOR_RPG.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-900ea938 — GAMEMACHINE_NECESIDADES.md

- **cambiado_despues:** 16
- **codigo_citado:** 18
- **ejemplos:** `examples/demo_dice_engine.cpp`, `include/Engine/Application.h`, `include/Game/BattleState.h`, `include/Game/EnemyBrain.h`, `include/Game/GameSession.h`, `include/Game/Skill.h`, `include/Level/ObjectCatalog.h`, `include/RPG/CharacterSheet.h`, `include/RPG/ConditionEngine.h`, `include/RPG/Definitions/SkillDefinition.h`
- **por qué:** GAMEMACHINE_NECESIDADES.md cita 18 ficheros de código y 16 cambiaron después de su última revisión.
- **afectación de `GAMEMACHINE_NECESIDADES.md`:** 1 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 1
  - consumidores: `include/RPG/DicePoolEngine.h`

### DOC_DESACTUALIZADO-3c590064 — PARALLEL_DEVELOPMENT_BOARD.md

- **cambiado_despues:** 5
- **codigo_citado:** 5
- **ejemplos:** `editor/src/EditorApplication.cpp`, `editor/src/level_editor.cpp`, `examples/demo_proyectos.cpp`, `examples/juego.cpp`, `src/Editor/ProjectHub.cpp`
- **por qué:** PARALLEL_DEVELOPMENT_BOARD.md cita 5 ficheros de código y 5 cambiaron después de su última revisión.
- **afectación de `PARALLEL_DEVELOPMENT_BOARD.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-8ec9a00b — README.md

- **cambiado_despues:** 20
- **codigo_citado:** 92
- **ejemplos:** `editor/src/level_editor.cpp`, `examples/demo_battle.cpp`, `examples/juego.cpp`, `include/Core/Errors/Result.h`, `include/Core/Json/JsonValue.h`, `include/Core/Resources/Texture.h`, `include/Engine/Application.h`, `include/Game/GameSession.h`, `include/Game/Skill.h`, `include/Render/HudElement.h`
- **por qué:** README.md cita 92 ficheros de código y 20 cambiaron después de su última revisión.
- **afectación de `README.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-bfa078f4 — analisis_clases/01_core.md

- **cambiado_despues:** 5
- **codigo_citado:** 8
- **ejemplos:** `examples/Check.h`, `include/Core/Check.h`, `include/Core/Errors/Result.h`, `include/Core/Ids/ProjectIds.h`, `include/Core/Json/JsonValue.h`
- **por qué:** analisis_clases/01_core.md cita 8 ficheros de código y 5 cambiaron después de su última revisión.
- **afectación de `analisis_clases/01_core.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-df8bc362 — analisis_clases/02_rpg_modelo_de_datos.md

- **cambiado_despues:** 6
- **codigo_citado:** 7
- **ejemplos:** `include/RPG/Catalogs/RpgCatalogs.h`, `include/RPG/Defense.h`, `include/RPG/Definitions/RpgCoreDefinitions.h`, `include/RPG/Definitions/SkillDefinition.h`, `include/RPG/NarrativeEngine.h`, `include/RPG/TierRules.h`
- **por qué:** analisis_clases/02_rpg_modelo_de_datos.md cita 7 ficheros de código y 6 cambiaron después de su última revisión.
- **afectación de `analisis_clases/02_rpg_modelo_de_datos.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-f50f5ae0 — analisis_clases/03_rpg_motores.md

- **cambiado_despues:** 13
- **codigo_citado:** 16
- **ejemplos:** `examples/ScriptedRng.h`, `include/RPG/CharacterCreationScreen.h`, `include/RPG/CharacterSheet.h`, `include/RPG/ConditionEngine.h`, `include/RPG/DicePoolEngine.h`, `include/RPG/EnhanceHooks.h`, `include/RPG/NarrativeEngine.h`, `include/RPG/RandomEngine.h`, `src/Game/SaveGame.cpp`, `src/Game/Skill.cpp`
- **por qué:** analisis_clases/03_rpg_motores.md cita 16 ficheros de código y 13 cambiaron después de su última revisión.
- **afectación de `analisis_clases/03_rpg_motores.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-7f108ac4 — analisis_clases/05_game_level_map_world.md

- **cambiado_despues:** 5
- **codigo_citado:** 7
- **ejemplos:** `include/Game/BattleState.h`, `include/Game/EnemyBrain.h`, `include/Game/GameSession.h`, `include/Game/SaveGame.h`, `include/Game/Skill.h`
- **por qué:** analisis_clases/05_game_level_map_world.md cita 7 ficheros de código y 5 cambiaron después de su última revisión.
- **afectación de `analisis_clases/05_game_level_map_world.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-65c45dfc — analisis_clases/06_engine_editor_proyecto.md

- **cambiado_despues:** 7
- **codigo_citado:** 8
- **ejemplos:** `editor/src/level_editor.cpp`, `examples/juego.cpp`, `include/Editor/ProjectIndex.h`, `include/Engine/Application.h`, `include/Project/ProjectIndex.h`, `src/Project/ProjectIndex.cpp`, `tools/build_proyecto.py`
- **por qué:** analisis_clases/06_engine_editor_proyecto.md cita 8 ficheros de código y 7 cambiaron después de su última revisión.
- **afectación de `analisis_clases/06_engine_editor_proyecto.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-2d70fd8f — analisis_clases/07_demos_tests_y_flujos.md

- **cambiado_despues:** 6
- **codigo_citado:** 6
- **ejemplos:** `editor/src/level_editor.cpp`, `examples/creacion_personaje.cpp`, `examples/juego.cpp`, `src/Game/Skill.cpp`, `tests/TestHarness.h`, `tools/build_proyecto.py`
- **por qué:** analisis_clases/07_demos_tests_y_flujos.md cita 6 ficheros de código y 6 cambiaron después de su última revisión.
- **afectación de `analisis_clases/07_demos_tests_y_flujos.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-808e7d49 — analisis_clases/08_matriz_cobertura_y_diagnostico.md

- **cambiado_despues:** 41
- **codigo_citado:** 101
- **ejemplos:** `include/Core/Errors/Result.h`, `include/Core/Json/JsonValue.h`, `include/Core/Json/JsonWriter.h`, `include/Core/Memory/HandlePool.h`, `include/Core/Resources/Texture.h`, `include/Editor/EditorState.h`, `include/Editor/ObjectInspector.h`, `include/Editor/ProjectHub.h`, `include/Engine/Application.h`, `include/Engine/Input/InputMap.h`
- **por qué:** analisis_clases/08_matriz_cobertura_y_diagnostico.md cita 101 ficheros de código y 41 cambiaron después de su última revisión.
- **afectación de `analisis_clases/08_matriz_cobertura_y_diagnostico.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-1ae9c92f — motor_grafico_clases.puml

- **cambiado_despues:** 3
- **codigo_citado:** 5
- **ejemplos:** `examples/demo_bitmap_font.cpp`, `examples/demo_editor_state.cpp`, `examples/juego.cpp`
- **por qué:** motor_grafico_clases.puml cita 5 ficheros de código y 3 cambiaron después de su última revisión.
- **afectación de `motor_grafico_clases.puml`:** 6 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 7
  - consumidores: `include/Engine/Application.h`, `include/Engine/InputState.h`, `include/Render/AnimatedEntity.h`, `include/Render/Enemy.h`, `include/Render/Entity.h`, `include/Render/Player.h`

### DOC_DESACTUALIZADO-fa5028b1 — motor_grafico_gantt_rpg.puml

- **cambiado_despues:** 3
- **codigo_citado:** 4
- **ejemplos:** `examples/demo_editor_state.cpp`, `examples/juego.cpp`, `include/Editor/EditorState.h`
- **por qué:** motor_grafico_gantt_rpg.puml cita 4 ficheros de código y 3 cambiaron después de su última revisión.
- **afectación de `motor_grafico_gantt_rpg.puml`:** 11 consumidores de producción, 9 pruebas, llega a nadie, paquetes —, radio 20
  - consumidores: `editor/src/level_editor.cpp`, `include/Game/BattleState.h`, `include/Game/Skill.h`, `include/Level/LevelDefinition.h`, `include/Level/ObjectCatalog.h`, `include/Render/BitmapFont.h`, `include/Render/BlobShadow.h`, `include/Render/DynamicLightMap.h`

### DOC_DESACTUALIZADO-6584b2f0 — planificacion/AUDITORIA_2026-09-03_ORFES_I_COBERTURA.md

- **cambiado_despues:** 7
- **codigo_citado:** 15
- **ejemplos:** `editor/src/EditorApplication.cpp`, `examples/juego.cpp`, `include/Core/Memory/HandlePool.h`, `include/Engine/Scene/SceneManager.h`, `include/Level/LevelDocumentBridge.h`, `include/World/World.h`, `src/Engine/Application.cpp`
- **por qué:** planificacion/AUDITORIA_2026-09-03_ORFES_I_COBERTURA.md cita 15 ficheros de código y 7 cambiaron después de su última revisión.
- **afectación de `planificacion/AUDITORIA_2026-09-03_ORFES_I_COBERTURA.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-b56d5d2f — planificacion/DISENO_AUTORIA_BLOQUES.md

- **cambiado_despues:** 3
- **codigo_citado:** 3
- **ejemplos:** `include/Game/BattleState.h`, `include/Game/Skill.h`, `include/RPG/Definitions/SkillDefinition.h`
- **por qué:** planificacion/DISENO_AUTORIA_BLOQUES.md cita 3 ficheros de código y 3 cambiaron después de su última revisión.
- **afectación de `planificacion/DISENO_AUTORIA_BLOQUES.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-b445fd36 — planificacion/ESPECIFICACION_EXPERIENCIA_RPG.md

- **cambiado_despues:** 10
- **codigo_citado:** 13
- **ejemplos:** `editor/src/level_editor.cpp`, `examples/juego.cpp`, `include/Editor/ObjectInspector.h`, `include/Editor/ProjectHub.h`, `include/Level/LevelDefinition.h`, `include/RPG/CharacterCreationScreen.h`, `include/Render/HudElement.h`, `planificacion/pla/dades.py`, `src/Engine/Application.cpp`, `src/Render/HudWidgets.cpp`
- **por qué:** planificacion/ESPECIFICACION_EXPERIENCIA_RPG.md cita 13 ficheros de código y 10 cambiaron después de su última revisión.
- **afectación de `planificacion/ESPECIFICACION_EXPERIENCIA_RPG.md`:** 1 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 1
  - consumidores: `include/Map/EventLayerDocument.h`

### DOC_DESACTUALIZADO-80738cd0 — planificacion/ESTADO_REAL_vs_ROADMAP.md

- **cambiado_despues:** 37
- **codigo_citado:** 81
- **ejemplos:** `editor/src/level_editor.cpp`, `examples/demo_ciudad.cpp`, `examples/juego.cpp`, `include/Core/Json/JsonValue.h`, `include/Core/Json/JsonWriter.h`, `include/Core/Memory/HandlePool.h`, `include/Editor/EditorState.h`, `include/Editor/ObjectInspector.h`, `include/Editor/ProjectHub.h`, `include/Engine/Application.h`
- **por qué:** planificacion/ESTADO_REAL_vs_ROADMAP.md cita 81 ficheros de código y 37 cambiaron después de su última revisión.
- **afectación de `planificacion/ESTADO_REAL_vs_ROADMAP.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-625eeb0d — planificacion/PLA_DEFINITIU.md

- **cambiado_despues:** 33
- **codigo_citado:** 190
- **ejemplos:** `cmake/MotorEditor.cmake`, `cmake/SkillEffectRealAssetsTests.cmake`, `editor/src/ProblemsPanel.cpp`, `editor/src/event_editor.cpp`, `include/Editor/AssetBrowserState.h`, `include/Editor/CatalogPickerState.h`, `include/Editor/CommandListState.h`, `include/Editor/EditorState.h`, `include/Editor/ProjectDatabaseState.h`, `include/Game/GameSession.h`
- **por qué:** planificacion/PLA_DEFINITIU.md cita 190 ficheros de código y 33 cambiaron después de su última revisión.
- **afectación de `planificacion/PLA_DEFINITIU.md`:** 98 consumidores de producción, 45 pruebas, llega a nadie, paquetes —, radio 143
  - consumidores: `editor/include/EditorApplication.h`, `editor/include/LayerPanel.h`, `editor/include/ProblemsPanel.h`, `editor/src/EditorApplication.cpp`, `editor/src/LayerPanel.cpp`, `editor/src/ProblemsPanel.cpp`, `editor/src/event_editor.cpp`, `editor/src/level_editor.cpp`

### DOC_DESACTUALIZADO-8b6cc2b2 — planificacion/motor_grafico_roadmap_ejecutable_v2.md

- **cambiado_despues:** 51
- **codigo_citado:** 118
- **ejemplos:** `editor/src/EditorApplication.cpp`, `editor/src/level_editor.cpp`, `editor/src/main.cpp`, `examples/demo_ciudad.cpp`, `examples/juego.cpp`, `include/Core/Json/JsonValue.h`, `include/Core/Memory/HandlePool.h`, `include/Editor/EditorState.h`, `include/Editor/ObjectInspector.h`, `include/Editor/ProjectHub.h`
- **por qué:** planificacion/motor_grafico_roadmap_ejecutable_v2.md cita 118 ficheros de código y 51 cambiaron después de su última revisión.
- **afectación de `planificacion/motor_grafico_roadmap_ejecutable_v2.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-7286f8c3 — planificacion/seguimiento/E-14-alternativa-claude.md

- **cambiado_despues:** 3
- **codigo_citado:** 5
- **ejemplos:** `cmake/CatalogPickerTests.cmake`, `include/Editor/CatalogPickerState.h`, `src/Editor/CatalogPickerState.cpp`
- **por qué:** planificacion/seguimiento/E-14-alternativa-claude.md cita 5 ficheros de código y 3 cambiaron después de su última revisión.
- **afectación de `planificacion/seguimiento/E-14-alternativa-claude.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DOC_DESACTUALIZADO-6ba7381c — planificacion/seguimiento/EXPEDIENTE-WIP-2026-09-12.md

- **cambiado_despues:** 23
- **codigo_citado:** 25
- **ejemplos:** `editor/src/level_editor.cpp`, `include/Game/EnemyCatalogBridge.h`, `include/Map/EventLayerDocument.h`, `include/RPG/AdventureValidation.h`, `include/RPG/Catalogs/RpgCatalogs.h`, `include/RPG/Definitions/RpgCoreDefinitions.h`, `include/RPG/EnhanceHooks.h`, `include/RPG/EventCommand.h`, `include/RPG/EventInterpreter.h`, `include/RPG/NarrativeEngine.h`
- **por qué:** planificacion/seguimiento/EXPEDIENTE-WIP-2026-09-12.md cita 25 ficheros de código y 23 cambiaron después de su última revisión.
- **afectación de `planificacion/seguimiento/EXPEDIENTE-WIP-2026-09-12.md`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

## 11. Disímilares: se parecen de nombre, NO se fusionan

*Regla:* Se parecen de nombre, no de función. NO se fusionan: el informe dice qué hace cada uno para que la próxima sesión no vuelva a dudarlo.

### DISIMILAR-fb2eb0d1 — cmake/ApplicationSceneTests.cmake · cmake/ApplicationTests.cmake

- **en_que_se_diferencian:** {"incluye": {"a": [], "b": []}, "tipos_que_define": {"a": [], "b": []}}
- **lineas:** {"cmake/ApplicationSceneTests.cmake": 14, "cmake/ApplicationTests.cmake": 14}
- **roles:** {"cmake/ApplicationSceneTests.cmake": "build", "cmake/ApplicationTests.cmake": "build"}
- **similitud_de_codigo:** 0.056
- **similitud_de_nombre:** 0.815
- **similitud:** 0.815
- **afectación de `cmake/ApplicationSceneTests.cmake`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `cmake/ApplicationTests.cmake`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DISIMILAR-21cd1375 — cmake/CatalogContentTests.cmake · cmake/CatalogContractTests.cmake

- **en_que_se_diferencian:** {"incluye": {"a": [], "b": []}, "tipos_que_define": {"a": [], "b": []}}
- **lineas:** {"cmake/CatalogContentTests.cmake": 23, "cmake/CatalogContractTests.cmake": 22}
- **roles:** {"cmake/CatalogContentTests.cmake": "build", "cmake/CatalogContractTests.cmake": "build"}
- **similitud_de_codigo:** 0.099
- **similitud_de_nombre:** 0.828
- **similitud:** 0.828
- **afectación de `cmake/CatalogContentTests.cmake`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `cmake/CatalogContractTests.cmake`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DISIMILAR-0d623dca — cmake/CharacterCreationFlowTests.cmake · cmake/CharacterCreationTests.cmake

- **en_que_se_diferencian:** {"incluye": {"a": [], "b": []}, "tipos_que_define": {"a": [], "b": []}}
- **lineas:** {"cmake/CharacterCreationFlowTests.cmake": 15, "cmake/CharacterCreationTests.cmake": 15}
- **roles:** {"cmake/CharacterCreationFlowTests.cmake": "build", "cmake/CharacterCreationTests.cmake": "build"}
- **similitud_de_codigo:** 0.057
- **similitud_de_nombre:** 0.895
- **similitud:** 0.895
- **afectación de `cmake/CharacterCreationFlowTests.cmake`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes G-09, radio 0
- **afectación de `cmake/CharacterCreationTests.cmake`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes G-09, radio 0

### DISIMILAR-ef68fb44 — editor/include/EditorApplication.h · include/Editor/EditorActions.h

- **en_que_se_diferencian:** {"incluye": {"a": [], "b": ["Editor/EditorDocumentSession.h", "array", "optional", "string", "vector"]}, "tipos_que_define": {"a": ["EditorApplication"], "b": ["EditorAction", "ActionGroup", "ActionDefinition", "ActionInput", "EditorActions", "Stroke"]}}
- **lineas:** {"editor/include/EditorApplication.h": 20, "include/Editor/EditorActions.h": 88}
- **roles:** {"editor/include/EditorApplication.h": "produccion", "include/Editor/EditorActions.h": "produccion"}
- **similitud_de_codigo:** 0.0
- **similitud_de_nombre:** 0.8
- **similitud:** 0.8
- **afectación de `editor/include/EditorApplication.h`:** 2 consumidores de producción, 0 pruebas, llega a motor_editor, paquetes —, radio 2
  - consumidores: `editor/src/EditorApplication.cpp`, `editor/src/main.cpp`
- **afectación de `include/Editor/EditorActions.h`:** 2 consumidores de producción, 1 pruebas, llega a motor_editor, paquetes —, radio 3
  - consumidores: `editor/src/EditorApplication.cpp`, `src/Editor/EditorActions.cpp`

### DISIMILAR-189c65a6 — editor/include/EditorApplication.h · include/Editor/EditorValidation.h

- **en_que_se_diferencian:** {"incluye": {"a": [], "b": ["algorithm", "string", "vector", "Editor/EditorState.h", "Game/Skill.h", "Level/ObjectCatalog.h"]}, "tipos_que_define": {"a": ["EditorApplication"], "b": ["EditorValidationResult", "EditorValidation"]}}
- **lineas:** {"editor/include/EditorApplication.h": 20, "include/Editor/EditorValidation.h": 188}
- **roles:** {"editor/include/EditorApplication.h": "produccion", "include/Editor/EditorValidation.h": "produccion"}
- **similitud_de_codigo:** 0.0
- **similitud_de_nombre:** 0.848
- **similitud:** 0.848
- **afectación de `editor/include/EditorApplication.h`:** 2 consumidores de producción, 0 pruebas, llega a motor_editor, paquetes —, radio 2
  - consumidores: `editor/src/EditorApplication.cpp`, `editor/src/main.cpp`
- **afectación de `include/Editor/EditorValidation.h`:** 1 consumidores de producción, 1 pruebas, llega a level_editor, paquetes —, radio 2
  - consumidores: `editor/src/level_editor.cpp`

### DISIMILAR-41989c48 — editor/src/EditorApplication.cpp · src/Editor/EditorActions.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["EditorApplication.h", "LayerPanel.h", "glad/glad.h", "GLFW/glfw3.h", "imgui.h", "imgui_internal.h", "imgui_impl_glfw.h", "imgui_impl_opengl3.h"], "b": ["Editor/EditorActions.h"]}, "tipos_que_define": {"a": ["GuiContext", "MapView"], "b": []}}
- **lineas:** {"editor/src/EditorApplication.cpp": 739, "src/Editor/EditorActions.cpp": 169}
- **roles:** {"editor/src/EditorApplication.cpp": "produccion", "src/Editor/EditorActions.cpp": "produccion"}
- **similitud_de_codigo:** 0.001
- **similitud_de_nombre:** 0.8
- **similitud:** 0.8
- **afectación de `editor/src/EditorApplication.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `src/Editor/EditorActions.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DISIMILAR-82eeabf5 — editor/src/event_editor.cpp · editor/src/level_editor.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["Editor/EventEditorState.h", "Editor/CommandListState.h", "Map/EventLayerDocument.h", "Map/MapLayer.h", "algorithm"], "b": ["Core/Math/IsoMath.h", "Core/Json/JsonValue.h", "cstdio", "memory", "Editor/ProjectHub.h", "Editor/ProjectIndex.h", "Core/Math/Vector4.h", "Core/Resources/Shader.h"]}, "tipos_que_define": {"a": [], "b": ["TilePaletteInfo", "RaceNpcPreview", "KeyEdge", "EditorOptions"]}}
- **lineas:** {"editor/src/event_editor.cpp": 351, "editor/src/level_editor.cpp": 2778}
- **roles:** {"editor/src/event_editor.cpp": "produccion", "editor/src/level_editor.cpp": "produccion"}
- **similitud_de_codigo:** 0.002
- **similitud_de_nombre:** 0.818
- **similitud:** 0.818
- **afectación de `editor/src/event_editor.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `editor/src/level_editor.cpp`:** 2 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 3
  - consumidores: `include/Editor/ObjectInspector.h`, `src/Engine/Application.cpp`

### DISIMILAR-263f835a — examples/creacion_personaje.cpp · examples/demo_creacion_personaje.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["cstdio", "cstdlib", "fstream", "memory", "string", "vector", "Engine/Window.h", "RPG/CharacterCreationScreen.h"], "b": ["cstdio", "string", "Check.h", "RPG/CharacterCreation.h", "RPG/CharacterCreationScreen.h", "RPG/TierRules.h"]}, "tipos_que_define": {"a": ["KeyEdge"], "b": []}}
- **lineas:** {"examples/creacion_personaje.cpp": 457, "examples/demo_creacion_personaje.cpp": 284}
- **roles:** {"examples/creacion_personaje.cpp": "prueba", "examples/demo_creacion_personaje.cpp": "prueba"}
- **similitud_de_codigo:** 0.008
- **similitud_de_nombre:** 0.895
- **similitud:** 0.895
- **afectación de `examples/creacion_personaje.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `examples/demo_creacion_personaje.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DISIMILAR-af919f40 — examples/demo_battle.cpp · examples/demo_battle_hud.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["Game/BattleState.h", "Render/ICombatant.h", "Game/Skill.h", "Check.h", "ScriptedRng.h", "algorithm", "iostream", "string"], "b": ["Core/Math/Vector4.h", "Core/Resources/Shader.h", "Core/Resources/ShaderManager.h", "Core/Resources/Texture.h", "Engine/Window.h", "Game/BattleState.h", "Render/BitmapFont.h", "Render/HudElement.h"]}, "tipos_que_define": {"a": ["TestCombatant", "Caso"], "b": ["TestCombatant"]}}
- **lineas:** {"examples/demo_battle.cpp": 263, "examples/demo_battle_hud.cpp": 241}
- **roles:** {"examples/demo_battle.cpp": "prueba", "examples/demo_battle_hud.cpp": "prueba"}
- **similitud_de_codigo:** 0.166
- **similitud_de_nombre:** 0.87
- **similitud:** 0.87
- **afectación de `examples/demo_battle.cpp`:** 1 consumidores de producción, 2 pruebas, llega a nadie, paquetes —, radio 3
  - consumidores: `include/Game/BattleState.h`
- **afectación de `examples/demo_battle_hud.cpp`:** 1 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 1
  - consumidores: `include/Render/HudWidgets.h`

### DISIMILAR-6b5a63f6 — examples/demo_battle.cpp · examples/demo_battle_nd6.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["Game/BattleState.h", "Render/ICombatant.h", "Game/Skill.h", "Check.h", "ScriptedRng.h", "algorithm", "iostream", "string"], "b": ["Render/ICombatant.h", "Game/Skill.h", "RPG/RandomEngine.h", "algorithm", "cstdio", "string"]}, "tipos_que_define": {"a": ["TestCombatant", "Caso"], "b": ["CombatantNd6", "BandStats"]}}
- **lineas:** {"examples/demo_battle.cpp": 263, "examples/demo_battle_nd6.cpp": 198}
- **roles:** {"examples/demo_battle.cpp": "prueba", "examples/demo_battle_nd6.cpp": "prueba"}
- **similitud_de_codigo:** 0.047
- **similitud_de_nombre:** 0.87
- **similitud:** 0.87
- **afectación de `examples/demo_battle.cpp`:** 1 consumidores de producción, 2 pruebas, llega a nadie, paquetes —, radio 3
  - consumidores: `include/Game/BattleState.h`
- **afectación de `examples/demo_battle_nd6.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DISIMILAR-a276d4eb — examples/demo_battle_hud.cpp · examples/demo_battle_nd6.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["Core/Math/Vector4.h", "Core/Resources/Shader.h", "Core/Resources/ShaderManager.h", "Core/Resources/Texture.h", "Engine/Window.h", "Game/BattleState.h", "Render/BitmapFont.h", "Render/HudElement.h"], "b": ["Render/ICombatant.h", "Game/Skill.h", "RPG/RandomEngine.h", "algorithm", "cstdio", "string"]}, "tipos_que_define": {"a": ["TestCombatant"], "b": ["CombatantNd6", "BandStats"]}}
- **lineas:** {"examples/demo_battle_hud.cpp": 241, "examples/demo_battle_nd6.cpp": 198}
- **roles:** {"examples/demo_battle_hud.cpp": "prueba", "examples/demo_battle_nd6.cpp": "prueba"}
- **similitud_de_codigo:** 0.044
- **similitud_de_nombre:** 0.846
- **similitud:** 0.846
- **afectación de `examples/demo_battle_hud.cpp`:** 1 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 1
  - consumidores: `include/Render/HudWidgets.h`
- **afectación de `examples/demo_battle_nd6.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DISIMILAR-3e0bb344 — examples/demo_bitmap_font.cpp · tests/Render/BitmapFontTests.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["Render/BitmapFont.h", "Check.h", "iostream"], "b": ["TestHarness.h", "Render/BitmapFont.h", "string", "vector"]}, "tipos_que_define": {"a": [], "b": []}}
- **lineas:** {"examples/demo_bitmap_font.cpp": 171, "tests/Render/BitmapFontTests.cpp": 115}
- **roles:** {"examples/demo_bitmap_font.cpp": "prueba", "tests/Render/BitmapFontTests.cpp": "prueba"}
- **similitud_de_codigo:** 0.008
- **similitud_de_nombre:** 0.833
- **similitud:** 0.833
- **afectación de `examples/demo_bitmap_font.cpp`:** 1 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 1
  - consumidores: `include/Render/BitmapFont.h`
- **afectación de `tests/Render/BitmapFontTests.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DISIMILAR-8f46d64a — examples/demo_boundington.cpp · examples/demo_campana_boundington.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["cstdio", "string", "vector", "Check.h", "RPG/NarrativeEngine.h"], "b": ["cstdio", "string", "vector", "Check.h", "RPG/NarrativeEngine.h"]}, "tipos_que_define": {"a": [], "b": ["Suerte", "Guiones", "Ruta"]}}
- **lineas:** {"examples/demo_boundington.cpp": 178, "examples/demo_campana_boundington.cpp": 374}
- **roles:** {"examples/demo_boundington.cpp": "prueba", "examples/demo_campana_boundington.cpp": "prueba"}
- **similitud_de_codigo:** 0.081
- **similitud_de_nombre:** 0.811
- **similitud:** 0.811
- **afectación de `examples/demo_boundington.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `examples/demo_campana_boundington.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DISIMILAR-340c96ce — examples/demo_boundington.cpp · examples/demo_precuela_boundington.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["cstdio", "string", "vector", "Check.h", "RPG/NarrativeEngine.h"], "b": ["cstdio", "Check.h", "RPG/NarrativeEngine.h"]}, "tipos_que_define": {"a": [], "b": []}}
- **lineas:** {"examples/demo_boundington.cpp": 178, "examples/demo_precuela_boundington.cpp": 50}
- **roles:** {"examples/demo_boundington.cpp": "prueba", "examples/demo_precuela_boundington.cpp": "prueba"}
- **similitud_de_codigo:** 0.02
- **similitud_de_nombre:** 0.789
- **similitud:** 0.789
- **afectación de `examples/demo_boundington.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `examples/demo_precuela_boundington.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DISIMILAR-86b05f04 — examples/demo_editor_state.cpp · tests/Editor/EditorStateTests.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["Editor/EditorState.h", "Editor/EditorValidation.h", "Level/LevelLoader.h", "Render/TileMap.h", "Check.h", "cstdio", "fstream", "iostream"], "b": ["Editor/EditorState.h", "TestHarness.h", "Core/Json/JsonValue.h", "Level/LevelDocumentBridge.h", "Project/ProjectSerializer.h", "limits"]}, "tipos_que_define": {"a": [], "b": []}}
- **lineas:** {"examples/demo_editor_state.cpp": 570, "tests/Editor/EditorStateTests.cpp": 324}
- **roles:** {"examples/demo_editor_state.cpp": "prueba", "tests/Editor/EditorStateTests.cpp": "prueba"}
- **similitud_de_codigo:** 0.045
- **similitud_de_nombre:** 0.846
- **similitud:** 0.846
- **afectación de `examples/demo_editor_state.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `tests/Editor/EditorStateTests.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes G-03, radio 0

### DISIMILAR-7c5111c5 — examples/demo_enemy_brain.cpp · tests/Game/EnemyBrainTests.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["Game/EnemyBrain.h", "Check.h", "iostream", "algorithm", "set", "utility"], "b": ["TestHarness.h", "Game/EnemyBrain.h", "RPG/TierRules.h"]}, "tipos_que_define": {"a": [], "b": []}}
- **lineas:** {"examples/demo_enemy_brain.cpp": 115, "tests/Game/EnemyBrainTests.cpp": 229}
- **roles:** {"examples/demo_enemy_brain.cpp": "prueba", "tests/Game/EnemyBrainTests.cpp": "prueba"}
- **similitud_de_codigo:** 0.021
- **similitud_de_nombre:** 0.833
- **similitud:** 0.833
- **afectación de `examples/demo_enemy_brain.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `tests/Game/EnemyBrainTests.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0

### DISIMILAR-c32cb2c8 — examples/demo_game_session.cpp · tests/Game/GameSessionTests.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["Game/GameSession.h", "Level/ObjectCatalog.h", "Game/Skill.h", "Render/TileMap.h", "Check.h", "ScriptedRng.h", "algorithm", "iostream"], "b": ["TestHarness.h", "Game/GameSession.h", "Level/ObjectCatalog.h", "Render/ICombatant.h", "RPG/NarrativeEngine.h", "RPG/RandomEngine.h", "algorithm", "cstdint"]}, "tipos_que_define": {"a": ["TestCombatant"], "b": ["TestCombatant", "Fixture", "RngFixo", "StatCombatant"]}}
- **lineas:** {"examples/demo_game_session.cpp": 308, "tests/Game/GameSessionTests.cpp": 1039}
- **roles:** {"examples/demo_game_session.cpp": "prueba", "tests/Game/GameSessionTests.cpp": "prueba"}
- **similitud_de_codigo:** 0.056
- **similitud_de_nombre:** 0.846
- **similitud:** 0.846
- **afectación de `examples/demo_game_session.cpp`:** 2 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 2
  - consumidores: `include/Engine/Application.h`, `include/Game/GameSession.h`
- **afectación de `tests/Game/GameSessionTests.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes C-24, radio 0

### DISIMILAR-b857e82d — examples/demo_handle_pool.cpp · tests/Core/HandlePoolTests.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["Core/Memory/HandlePool.h", "Check.h", "iostream", "string"], "b": ["Core/Memory/HandlePool.h", "TestHarness.h", "string"]}, "tipos_que_define": {"a": ["Particle"], "b": []}}
- **lineas:** {"examples/demo_handle_pool.cpp": 63, "tests/Core/HandlePoolTests.cpp": 109}
- **roles:** {"examples/demo_handle_pool.cpp": "prueba", "tests/Core/HandlePoolTests.cpp": "prueba"}
- **similitud_de_codigo:** 0.053
- **similitud_de_nombre:** 0.833
- **similitud:** 0.833
- **afectación de `examples/demo_handle_pool.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `tests/Core/HandlePoolTests.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes C-18, radio 0

### DISIMILAR-b94cb586 — examples/demo_object_catalog.cpp · tests/Level/ObjectCatalogTests.cpp

- **en_que_se_diferencian:** {"incluye": {"a": ["Game/BattleState.h", "Render/ICombatant.h", "Level/LevelLoader.h", "Level/ObjectCatalog.h", "Game/Skill.h", "Check.h", "algorithm", "iostream"], "b": ["Level/ObjectCatalog.h", "TestHarness.h", "cstddef", "string", "vector"]}, "tipos_que_define": {"a": ["TestCombatant"], "b": []}}
- **lineas:** {"examples/demo_object_catalog.cpp": 283, "tests/Level/ObjectCatalogTests.cpp": 227}
- **roles:** {"examples/demo_object_catalog.cpp": "prueba", "tests/Level/ObjectCatalogTests.cpp": "prueba"}
- **similitud_de_codigo:** 0.021
- **similitud_de_nombre:** 0.867
- **similitud:** 0.867
- **afectación de `examples/demo_object_catalog.cpp`:** 0 consumidores de producción, 0 pruebas, llega a nadie, paquetes —, radio 0
- **afectación de `tests/Level/ObjectCatalogTests.cpp`:** 0 consumidores de producción, 1 pruebas, llega a nadie, paquetes —, radio 1

### DISIMILAR-da6a5dd5 — include/Core/Ids/ProjectIds.h · include/Editor/ProjectIndex.h

- **en_que_se_diferencian:** {"incluye": {"a": ["Core/Ids/StrongId.h"], "b": ["Project/ProjectIndex.h"]}, "tipos_que_define": {"a": ["ProjectIdTag", "DocumentIdTag", "MapIdTag", "LayerIdTag", "AssetIdTag", "EntityIdTag", "RegionIdTag", "EventIdTag"], "b": []}}
- **lineas:** {"include/Core/Ids/ProjectIds.h": 42, "include/Editor/ProjectIndex.h": 7}
- **roles:** {"include/Core/Ids/ProjectIds.h": "produccion", "include/Editor/ProjectIndex.h": "produccion"}
- **similitud_de_codigo:** 0.0
- **similitud_de_nombre:** 0.818
- **similitud:** 0.818
- **afectación de `include/Core/Ids/ProjectIds.h`:** 12 consumidores de producción, 3 pruebas, llega a juego, motor_editor, level_editor, paquetes —, radio 15
  - consumidores: `include/Editor/EventEditorState.h`, `include/Editor/ProjectDatabaseState.h`, `include/Engine/Scene/Scene.h`, `include/Level/LevelDocumentBridge.h`, `include/Map/EntityLayerDocument.h`, `include/Map/EventLayerDocument.h`, `include/Map/MapDocument.h`, `include/Map/MapLayer.h`
- **afectación de `include/Editor/ProjectIndex.h`:** 6 consumidores de producción, 5 pruebas, llega a nadie, paquetes —, radio 11
  - consumidores: `editor/src/EditorApplication.cpp`, `editor/src/level_editor.cpp`, `include/Editor/ProjectHub.h`, `include/Engine/Application.h`, `src/Editor/AssetBrowserState.cpp`, `src/Project/ProjectIndex.cpp`

### DISIMILAR-b241d29d — include/Core/Ids/ProjectIds.h · include/Project/Project.h

- **en_que_se_diferencian:** {"incluye": {"a": ["Core/Ids/StrongId.h"], "b": ["cstddef", "string", "vector", "Project/ProjectSettings.h"]}, "tipos_que_define": {"a": ["ProjectIdTag", "DocumentIdTag", "MapIdTag", "LayerIdTag", "AssetIdTag", "EntityIdTag", "RegionIdTag", "EventIdTag"], "b": ["Project", "EntryKind", "ProjectCheck"]}}
- **lineas:** {"include/Core/Ids/ProjectIds.h": 42, "include/Project/Project.h": 53}
- **roles:** {"include/Core/Ids/ProjectIds.h": "produccion", "include/Project/Project.h": "produccion"}
- **similitud_de_codigo:** 0.0
- **similitud_de_nombre:** 0.824
- **similitud:** 0.824
- **afectación de `include/Core/Ids/ProjectIds.h`:** 12 consumidores de producción, 3 pruebas, llega a juego, motor_editor, level_editor, paquetes —, radio 15
  - consumidores: `include/Editor/EventEditorState.h`, `include/Editor/ProjectDatabaseState.h`, `include/Engine/Scene/Scene.h`, `include/Level/LevelDocumentBridge.h`, `include/Map/EntityLayerDocument.h`, `include/Map/EventLayerDocument.h`, `include/Map/MapDocument.h`, `include/Map/MapLayer.h`
- **afectación de `include/Project/Project.h`:** 6 consumidores de producción, 2 pruebas, llega a juego, motor_editor, level_editor, paquetes —, radio 8
  - consumidores: `include/Game/TitleMenuState.h`, `include/Game/TitleScreen.h`, `include/Project/ProjectCatalog.h`, `include/Project/ProjectIndex.h`, `include/Project/ProjectSerializer.h`, `src/Project/Project.cpp`

### DISIMILAR-b64af0ec — include/Core/Ids/ProjectIds.h · include/Project/ProjectIndex.h

- **en_que_se_diferencian:** {"incluye": {"a": ["Core/Ids/StrongId.h"], "b": ["string", "vector", "Core/Errors/Result.h", "Project/Project.h"]}, "tipos_que_define": {"a": ["ProjectIdTag", "DocumentIdTag", "MapIdTag", "LayerIdTag", "AssetIdTag", "EntityIdTag", "RegionIdTag", "EventIdTag"], "b": ["ProjectIndex", "DuplicateReport"]}}
- **lineas:** {"include/Core/Ids/ProjectIds.h": 42, "include/Project/ProjectIndex.h": 185}
- **roles:** {"include/Core/Ids/ProjectIds.h": "produccion", "include/Project/ProjectIndex.h": "produccion"}
- **similitud_de_codigo:** 0.0
- **similitud_de_nombre:** 0.818
- **similitud:** 0.818
- **afectación de `include/Core/Ids/ProjectIds.h`:** 12 consumidores de producción, 3 pruebas, llega a juego, motor_editor, level_editor, paquetes —, radio 15
  - consumidores: `include/Editor/EventEditorState.h`, `include/Editor/ProjectDatabaseState.h`, `include/Engine/Scene/Scene.h`, `include/Level/LevelDocumentBridge.h`, `include/Map/EntityLayerDocument.h`, `include/Map/EventLayerDocument.h`, `include/Map/MapDocument.h`, `include/Map/MapLayer.h`
- **afectación de `include/Project/ProjectIndex.h`:** 7 consumidores de producción, 5 pruebas, llega a juego, motor_editor, level_editor, paquetes C-15, radio 12
  - consumidores: `editor/src/EditorApplication.cpp`, `editor/src/level_editor.cpp`, `include/Editor/ProjectHub.h`, `include/Editor/ProjectIndex.h`, `include/Engine/Application.h`, `src/Editor/AssetBrowserState.cpp`, `src/Project/ProjectIndex.cpp`

### DISIMILAR-cdb8fff5 — include/Core/Math/Vector2.h · include/Core/Math/Vector4.h

- **en_que_se_diferencian:** {"incluye": {"a": [], "b": []}, "tipos_que_define": {"a": ["Vector2"], "b": ["Vector4"]}}
- **lineas:** {"include/Core/Math/Vector2.h": 13, "include/Core/Math/Vector4.h": 17}
- **roles:** {"include/Core/Math/Vector2.h": "produccion", "include/Core/Math/Vector4.h": "produccion"}
- **similitud_de_codigo:** 0.132
- **similitud_de_nombre:** 0.923
- **similitud:** 0.923
- **afectación de `include/Core/Math/Vector2.h`:** 11 consumidores de producción, 2 pruebas, llega a juego, motor_editor, level_editor, paquetes —, radio 13
  - consumidores: `include/Core/Math/IsoMath.h`, `include/Render/BitmapFont.h`, `include/Render/Camera.h`, `include/Render/Culling.h`, `include/Render/DynamicLightMap.h`, `include/Render/Entity.h`, `include/Render/HudElement.h`, `include/Render/HudTextWidgets.h`
- **afectación de `include/Core/Math/Vector4.h`:** 11 consumidores de producción, 7 pruebas, llega a juego, motor_editor, level_editor, paquetes —, radio 18
  - consumidores: `editor/src/level_editor.cpp`, `include/Render/BitmapFont.h`, `include/Render/ColorLUT.h`, `include/Render/DynamicLightMap.h`, `include/Render/Entity.h`, `include/Render/HudMinimap.h`, `include/Render/HudTextWidgets.h`, `include/Render/HudWidgets.h`

### DISIMILAR-0813516b — include/Editor/EditorDocumentIO.h · include/Editor/EditorDocumentSession.h

- **en_que_se_diferencian:** {"incluye": {"a": ["Core/Errors/Result.h", "Editor/EditorState.h"], "b": ["optional", "string", "Editor/EditorState.h", "Level/ObjectCatalog.h", "Render/TileMap.h"]}, "tipos_que_define": {"a": ["EditorDocumentIO"], "b": ["EditorDocumentSession", "Action", "Choice", "Document", "Pending", "ViewportInput"]}}
- **lineas:** {"include/Editor/EditorDocumentIO.h": 17, "include/Editor/EditorDocumentSession.h": 101}
- **roles:** {"include/Editor/EditorDocumentIO.h": "produccion", "include/Editor/EditorDocumentSession.h": "produccion"}
- **similitud_de_codigo:** 0.001
- **similitud_de_nombre:** 0.865
- **similitud:** 0.865
- **afectación de `include/Editor/EditorDocumentIO.h`:** 3 consumidores de producción, 2 pruebas, llega a motor_editor, level_editor, paquetes —, radio 5
  - consumidores: `editor/src/level_editor.cpp`, `src/Editor/EditorDocumentIO.cpp`, `src/Editor/EditorDocumentSession.cpp`
- **afectación de `include/Editor/EditorDocumentSession.h`:** 4 consumidores de producción, 2 pruebas, llega a motor_editor, paquetes —, radio 6
  - consumidores: `editor/include/LayerPanel.h`, `editor/src/EditorApplication.cpp`, `include/Editor/EditorActions.h`, `src/Editor/EditorDocumentSession.cpp`

### DISIMILAR-3c45e2ac — include/Editor/EditorState.h · include/Editor/EventEditorState.h

- **en_que_se_diferencian:** {"incluye": {"a": ["functional", "string", "unordered_map", "vector", "Core/Errors/Result.h", "Core/Math/GridCoord.h", "Level/LevelDefinition.h", "Map/MapDocument.h"], "b": ["cstddef", "string", "vector", "Core/Errors/Result.h", "Core/Ids/ProjectIds.h", "Core/Math/GridCoord.h", "Map/EventLayerDocument.h", "Map/MapDocument.h"]}, "tipos_que_define": {"a": ["EditorTool", "TmxTilesetSettings", "EditorState", "Rect", "Snapshot", "Clipboard", "Entity"], "b": ["EventPage", "EventDocument", "EventEditorState", "EditOp", "Type"]}}
- **lineas:** {"include/Editor/EditorState.h": 398, "include/Editor/EventEditorState.h": 105}
- **roles:** {"include/Editor/EditorState.h": "produccion", "include/Editor/EventEditorState.h": "produccion"}
- **similitud_de_codigo:** 0.012
- **similitud_de_nombre:** 0.815
- **similitud:** 0.815
- **afectación de `include/Editor/EditorState.h`:** 6 consumidores de producción, 5 pruebas, llega a motor_editor, level_editor, paquetes C-15, radio 11
  - consumidores: `editor/src/level_editor.cpp`, `include/Editor/EditorDocumentIO.h`, `include/Editor/EditorDocumentSession.h`, `include/Editor/EditorValidation.h`, `src/Editor/EditorState.cpp`, `src/Editor/ObjectInspector.cpp`
- **afectación de `include/Editor/EventEditorState.h`:** 3 consumidores de producción, 2 pruebas, llega a nadie, paquetes —, radio 5
  - consumidores: `editor/src/event_editor.cpp`, `include/Editor/CommandListState.h`, `include/Editor/ProjectDatabaseState.h`

*(35 hallazgos más de este tipo en el JSON)*

