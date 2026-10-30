# MotorGraphico Creator — Estado real del código frente al roadmap

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe el estado del codigo frente al roadmap de principios de septiembre, foto del 03/09/2026. El plan vigente es `planificacion/PLA_DEFINITIU.md` con `gantt_corte_b.puml` (generados desde `pla/dades.py`), mas `gantt_alpha_cierre.puml` y `gantt_jugabilidad_H.puml`; quien esta en que, en `TABLERO.md`. De las 79 citas de codigo que contiene, 53 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

Auditoría del repositorio `MotorGraphico/` del 28 de agosto de 2026, contrastada con los 98 paquetes de `motor_grafico_gantt_rpgmaker.puml` y las fichas de `motor_grafico_roadmap_ejecutable.md`.

## 1. Titular

El Gantt está planificado sobre una línea base que ya no existe. Asume un motor con renderer, cámara isométrica, TMX y poco más; el repositorio real ya tiene **editor de niveles con capas, deshacer y portapapeles**, **runtime con bucle de aplicación y transiciones de nivel**, **motor narrativo con condiciones, efectos y temporizadores** y **una base de datos RPG con 19 tipos de definición cargados de JSON**. Planificar esas fases desde cero es planificar trabajo ya pagado.

En el otro sentido, el roadmap da por supuestas tres cosas que el repositorio no tiene y que bloquean literalmente decenas de fichas:

1. **No existe `tests/` ni `ctest`.** Sesenta fichas nombran archivos bajo `tests/` y su criterio de aceptación es "una prueba demuestra…". Hoy la verificación son ejecutables `demo_*` que abortan al fallar. → nuevo paquete **INF-01**.
2. **Dear ImGui está descartado.** `examples/level_editor.cpp` documenta en su cabecera que la interfaz se hizo con el HUD propio del motor porque no había red para vendorizar ImGui. La ficha **EDT-02** planifica justo lo contrario. → nuevo paquete **INF-02**.
3. **El árbol real no es el árbol objetivo.** El roadmap dibuja `Project/`, `Map/`, `World/`, `Events/`, `Database/` y `editor/`; el repositorio tiene `Editor/`, `Level/`, `RPG/`, `Game/` y `Render/`, y el editor vive en `examples/`. → nuevo paquete **INF-03**.

## 2. Cifras

| Concepto | Valor |
| --- | ---: |
| Paquetes del roadmap original | 98 |
| Paquetes nuevos añadidos por la auditoría | 6 |
| Subtareas atómicas totales | 466 |
| Esfuerzo original planificado | 629 jornadas |
| Esfuerzo ya cubierto por código existente | 203 jornadas |
| **Esfuerzo restante hasta 1.0** | **426 jornadas** |
| Avance global ponderado | 32 % |

A ritmo de ratos libres y fines de semana (~8-10 h por semana, ≈1,5 jornadas de esfuerzo), las 426 jornadas restantes son del orden de **284 semanas naturales**. La conclusión práctica no es "tardaré eso": es que **1.0 completo no es un objetivo alcanzable a este ritmo** y conviene elegir un corte más pequeño (ver sección 5).

## 3. Estado por fase

| Fase | Paquetes | Estimación | Restante | Avance |
| --- | ---: | ---: | ---: | ---: |
| Fase 0 — Línea base | 8 | 26 j | 23 j | 12 % |
| Fase 1 — Arquitectura documental | 7 | 38 j | 21 j | 45 % |
| Fase 2 — Runtime jugable mínimo | 11 | 58 j | 31 j | 47 % |
| Fase 3 — Editor visual de mapas MVP | 15 | 81 j | 35 j | 57 % |
| Fase 4 — Herramientas avanzadas de mapas | 12 | 81 j | 58 j | 28 % |
| Fase 5 — Sistema de eventos tipo RPG Maker | 16 | 104 j | 77 j | 26 % |
| Fase 6 — Base de datos y sistemas de juego | 13 | 95 j | 47 j | 51 % |
| Fase 7 — Playtest, extensiones y exportación | 12 | 72 j | 68 j | 6 % |
| Fase 8 — Beta, documentación y versión 1.0 | 10 | 74 j | 66 j | 11 % |
| **Total** | **104** | **629 j** | **426 j** | **32 %** |

### Reparto por estado

| Estado | Paquetes | Significado |
| --- | ---: | --- |
| DONE | 15 | Hecho en lo esencial; queda pulido o exposición al editor |
| PARCIAL | 53 | Hay base real reutilizable; la ficha se reescribe como continuación |
| PENDIENTE | 33 | Sin empezar; la ficha vale tal cual |
| REPLANTEAR | 3 | La ficha contradice una decisión ya tomada en el código |

## 4. Los tres paquetes a REPLANTEAR

### DOC-03 — Separar MapDocument de RuntimeTileMap

La frontera documento/runtime ya existe de facto (EditorState es GL-free), pero el puente es un STRING TMX: se exporta texto y TileMap lo reparsea. Sustituir ese round-trip por texto por un builder en memoria.

- **Lo que ya hay:** include/Render/TileMap.h; src/Render/TileMap.cpp; include/Editor/EditorState.h::exportTmx()

### EDT-02 — Integrar Dear ImGui y docking

DECISIÓN YA TOMADA EN CONTRA: la UI se hizo con el HUD propio del motor porque no había red para vendorizar Dear ImGui. Esta ficha no se puede ejecutar como está escrita — hay que decidir (ver INF-02).

- **Lo que ya hay:** examples/level_editor.cpp (cabecera); include/Render/HudManager.h; include/Render/HudWidgets.h

### EDT-12 — Implementar CommandHistory y deshacer-rehacer

Hay deshacer/rehacer robusto pero por SNAPSHOT COMPLETO del mapa, no por comandos. Funciona; escala mal en memoria. Decidir si se migra o se documenta el límite.

- **Lo que ya hay:** EditorState::undo/redo/recordUndo/Snapshot

## 5. Tres cortes posibles más pequeños que 1.0

Con dedicación de ratos libres, elegir un corte importa más que ordenar las fases. Tres opciones, todas construibles sobre lo que ya existe:

| Corte | Contenido | Restante aprox. |
| --- | --- | ---: |
| **A. Editor de mapas defendible** | Infraestructura mínima + cerrar los huecos del editor que ya funciona (cuentagotas, rectángulo, capas completas, autosave) | 28 j |
| **B. Ciclo autor completo** | A + guardado de partida, eventos editables y playtest desde el editor: el primer momento en que alguien que no eres tú puede hacer un juego | 72 j |
| **C. Base documental sólida** | B + formato de proyecto versionado y editor de la base de datos | 100 j |

El corte **B** es el que convierte el proyecto en lo que promete su nombre. Todo lo demás (autotiles, plugins, exportación, audio) es 1.1.

## 6. Ficha por ficha

Leyenda de estado: **DONE** hecho · **PARCIAL** hay base reutilizable · **PENDIENTE** sin empezar · **REPLANTEAR** la ficha contradice el código.


### Fase 0 — Línea base

#### BASE-01 — Inventariar y congelar la base gráfica actual

`PARCIAL` · avance **70 %** · estimación 2 j · restante **1 j** · depende de Ninguna

- **En el repo hoy:** ARCHITECTURE.md; AUDITORIA_CONTRATOS.md; motor_grafico_clases.puml; MEMORY_AND_INHERITANCE.md
- **Lectura:** Hay inventario disperso en varios .md, pero no una foto congelada y fechada de la línea base. Cerrar como documento único antes de tocar DOC-01.

#### BASE-02 — Verificar CI, demos y compatibilidad multiplataforma

`PARCIAL` · avance **75 %** · estimación 3 j · restante **1 j** · depende de BASE-01

- **En el repo hoy:** .github/workflows/ci.yml; .github/workflows/release.yml; .clang-format; .clang-tidy; .pre-commit-config.yaml
- **Lectura:** CI real y potente (Debug/Release x g++/clang, ASan/UBSan, clang-format, clang-tidy, cppcheck, xvfb) pero SOLO ubuntu-latest y SIN ctest: la verificación son demos que abortan. La ficha asume tests/ y multiplataforma; ver INF-01 e INF-05.

#### INF-01 — Crear tests/ y ctest, y convertir los demos-aserción en pruebas

`PENDIENTE` · avance **0 %** · estimación 5 j · restante **5 j** · depende de BASE-02

- **En el repo hoy:** —
- **Lectura:** El roadmap referencia tests/ en 60 fichas y el repo NO TIENE tests/ ni enable_testing(). Sin esto, ningún criterio de aceptación de las fichas se puede cumplir literalmente.

#### INF-06 — Limpiar duplicados y ruido del repositorio

`PENDIENTE` · avance **0 %** · estimación 1 j · restante **1 j** · depende de INF-01

- **En el repo hoy:** —
- **Lectura:** Hay archivos duplicados por sincronización ('FORMATO_NIVELES 2.md', 'demo_battle 2.cpp'), .DS_Store versionados y salidas .ppm/.png en la raíz. Es barato y evita editar el archivo equivocado.

#### INF-02 — Decidir la tecnología de UI del editor (HUD propio vs Dear ImGui)

`PENDIENTE` · avance **0 %** · estimación 3 j · restante **3 j** · depende de INF-06

- **En el repo hoy:** —
- **Lectura:** EDT-02 planifica integrar Dear ImGui, pero level_editor.cpp ya documenta que se descartó por falta de red para vendorizar. Toda la fase 3 depende de cerrar esto.

#### INF-04 — Decidir dependencias externas y política de vendorizado

`PENDIENTE` · avance **0 %** · estimación 2 j · restante **2 j** · depende de INF-02

- **En el repo hoy:** —
- **Lectura:** No hay audio (GDB-11), no hay ImGui (EDT-02) y el entorno de desarrollo no tiene red. third_party/ solo tiene glad, stb y tinyxml2. Es una restricción real que atraviesa tres fases.

#### INF-05 — Ampliar CI a Windows y macOS

`PENDIENTE` · avance **0 %** · estimación 4 j · restante **4 j** · depende de INF-04

- **En el repo hoy:** —
- **Lectura:** BASE-02 declara compatibilidad multiplataforma pero ci.yml solo usa ubuntu-latest; TOOL-10 promete exportación a Windows sin haberlo compilado nunca allí.

#### INF-03 — Reorganizar el árbol hacia la arquitectura objetivo

`PENDIENTE` · avance **0 %** · estimación 6 j · restante **6 j** · depende de INF-05

- **En el repo hoy:** —
- **Lectura:** La arquitectura objetivo del roadmap (Project/, Map/, World/, Events/, Database/, editor/) no coincide con el árbol real (Editor/, Level/, RPG/, Game/, Render/). Cada ficha DOC/RUN empieza con rutas que no existen.


### Fase 1 — Arquitectura documental

#### DOC-01 — Definir Project y ProjectSettings

`PARCIAL` · avance **60 %** · estimación 5 j · restante **2 j** · depende de INF-03

- **En el repo hoy:** include/Editor/ProjectIndex.h (struct Project); include/Editor/ProjectHub.h; src/Editor/ProjectIndex.cpp; assets/proyectos/*.json; FORMATO_PROYECTOS.md; examples/demo_proyectos.cpp
- **Lectura:** Ya existe un Project con índice y hub, pero vive bajo Editor/ y no tiene ProjectSettings ni versión de esquema. El trabajo real es MOVER y completar, no crear de cero.

#### DOC-02 — Definir MapDocument y capas tipadas

`PARCIAL` · avance **45 %** · estimación 7 j · restante **4 j** · depende de DOC-01

- **En el repo hoy:** include/Level/LevelDefinition.h; include/Editor/EditorState.h (capas de tiles); FORMATO_NIVELES.md
- **Lectura:** LevelDefinition + las capas de EditorState hacen de documento de mapa, pero solo hay capas de TILES: entidades y regiones no son capas. Falta el tipo MapDocument como fuente de verdad fuera del editor.

#### DOC-03 — Separar MapDocument de RuntimeTileMap

`REPLANTEAR` · avance **40 %** · estimación 7 j · restante **5 j** · depende de DOC-02

- **En el repo hoy:** include/Render/TileMap.h; src/Render/TileMap.cpp; include/Editor/EditorState.h::exportTmx()
- **Lectura:** La frontera documento/runtime ya existe de facto (EditorState es GL-free), pero el puente es un STRING TMX: se exporta texto y TileMap lo reparsea. Sustituir ese round-trip por texto por un builder en memoria.

#### DOC-04 — Implementar IDs estables y referencias

`PARCIAL` · avance **50 %** · estimación 4 j · restante **2 j** · depende de DOC-03

- **En el repo hoy:** CONTRATO_IDS.md; include/Core/Memory/HandlePool.h; include/Core/Resources/ICatalog.h
- **Lectura:** Hay contrato de IDs escrito y handles, pero los ids son std::string sueltos sin tipo fuerte, sin tabla de renombrado ni detección de referencias rotas centralizada.

#### DOC-05 — Serialización JSON y versionado de esquema

`PARCIAL` · avance **55 %** · estimación 6 j · restante **3 j** · depende de DOC-04

- **En el repo hoy:** include/Core/Json/JsonValue.h; src/Core/Json/JsonValue.cpp; src/Level/LevelLoader.cpp; FORMATO_NIVELES.md; FORMATO_AVENTURAS.md; FORMATO_PROYECTOS.md
- **Lectura:** JSON propio y tres formatos documentados y en uso. Lo que falta es exactamente lo caro: campo schemaVersion, migraciones y política de compatibilidad.

#### DOC-06 — Convertir el parser TMX en importador

`PARCIAL` · avance **40 %** · estimación 5 j · restante **3 j** · depende de DOC-05

- **En el repo hoy:** third_party/tinyxml2; src/Render/TileMap.cpp (loadFromFile TMX); include/Editor/EditorState.h::exportTmx()
- **Lectura:** Hoy TMX es la fuente de verdad del grid, no un formato de importación. Invertir la dirección es la tarea; el parser ya está escrito.

#### DOC-07 — Pruebas de ida y vuelta TMX-JSON-Runtime

`PARCIAL` · avance **60 %** · estimación 4 j · restante **2 j** · depende de DOC-06

- **En el repo hoy:** examples/demo_editor_state.cpp
- **Lectura:** El demo ya verifica round-trip TMX->TileMap y JSON->LevelLoader. Falta añadir el eslabón MapDocument cuando exista y convertirlo en prueba de ctest.


### Fase 2 — Runtime jugable mínimo

#### RUN-01 — Implementar Application y bucle principal

`PARCIAL` · avance **80 %** · estimación 5 j · restante **1 j** · depende de DOC-07

- **En el repo hoy:** include/Engine/Application.h; src/Engine/Application.cpp; include/Engine/Window.h
- **Lectura:** Bucle completo (run/processInput/update/render/shutdown, maxFrames para CI). Problema: Application conoce tienda, negocio y batalla — está acoplada a ESTE juego. La tarea real es adelgazarla.

#### RUN-02 — Implementar InputSystem

`PARCIAL` · avance **60 %** · estimación 4 j · restante **2 j** · depende de RUN-01

- **En el repo hoy:** include/Engine/InputState.h; src/Engine/Application.cpp::processInput()
- **Lectura:** Hay estado de entrada, pero el mapeo tecla->acción está incrustado en Application y en level_editor.cpp. Falta la capa de acciones configurable.

#### RUN-03 — Implementar SceneManager y carga de mapas

`PARCIAL` · avance **45 %** · estimación 6 j · restante **4 j** · depende de RUN-02

- **En el repo hoy:** src/Engine/Application.cpp::applyPendingTransition(); include/Level/LevelLoader.h; include/Level/WorldTopology.h
- **Lectura:** El cambio de escena funciona, pero no hay un SceneManager: la lógica está repartida entre Application y GameSession.

#### RUN-04 — Implementar World y ciclo de vida de entidades

`PARCIAL` · avance **35 %** · estimación 6 j · restante **4 j** · depende de RUN-03

- **En el repo hoy:** include/Game/GameSession.h (WorldEnemy, spawns); include/Render/Entity.h
- **Lectura:** GameSession gestiona el ciclo de vida de enemigos y objetos del nivel, pero es específico del juego, no un World genérico con EntityId.

#### RUN-05 — Migrar Entity a composición por componentes

`PENDIENTE` · avance **10 %** · estimación 7 j · restante **7 j** · depende de RUN-04

- **En el repo hoy:** include/Render/Entity.h; include/Render/Player.h; include/Render/Enemy.h; include/Render/AnimatedEntity.h; MEMORY_AND_INHERITANCE.md
- **Lectura:** Hoy es jerarquía por herencia y está documentada como decisión consciente. Migrar a componentes es un cambio de arquitectura: NO empezarlo sin decidirlo explícitamente (ver la puerta de aclaración).

#### RUN-06 — Implementar animación por frames

`DONE` · avance **90 %** · estimación 5 j · restante **1 j** · depende de RUN-05

- **En el repo hoy:** include/Render/AnimatedEntity.h; src/Render/AnimatedEntity.cpp; examples/demo_animated_entity.cpp
- **Lectura:** Animación por frames resuelta. Queda exponerla como dato editable desde el editor, no como código.

#### RUN-07 — Implementar movimiento y colisión dinámica

`PARCIAL` · avance **65 %** · estimación 7 j · restante **3 j** · depende de RUN-06

- **En el repo hoy:** src/Render/TileMap.cpp (colisión por GID); include/Game/GameSession.h; include/Render/OcclusionRules.h
- **Lectura:** Colisión contra el mapa sí; colisión dinámica entre entidades y empuje, no.

#### RUN-08 — Implementar cámara de seguimiento

`DONE` · avance **90 %** · estimación 3 j · restante **1 j** · depende de RUN-07

- **En el repo hoy:** include/Render/Camera.h; src/Render/Camera.cpp; examples/demo_camera.cpp
- **Lectura:** Cámara isométrica con seguimiento hecha. Falta solo límites de mapa configurables por documento.

#### RUN-09 — Implementar portales y cambio de mapa

`DONE` · avance **80 %** · estimación 4 j · restante **1 j** · depende de RUN-08

- **En el repo hoy:** include/Level/WorldTopology.h; include/Game/GameSession.h (LevelTransition); EditorState::setObjectTransition(); examples/demo_mundo.cpp; examples/demo_ciudad.cpp
- **Lectura:** Portales y grafo de mundo funcionando y editables. Falta el punto de aparición por destino bien definido.

#### RUN-10 — Implementar estado y guardado básico

`DONE` · avance **100 %** · estimación 6 j · restante **0 j** · depende de RUN-09

- **En el repo hoy:** include/Game/SaveGame.h; src/Game/SaveGame.cpp; include/Core/Json/JsonWriter.h; tests/Game/SaveGameTests.cpp; src/Engine/Application.cpp::saveGame/loadGame
- **Lectura:** Cerrado el 28-08-2026. `SaveGameData` es el DTO (10.1); `GameSession::captureSaveGame`/`restoreSaveGame` fotografían y restauran la partida (10.2); `writeSaveGameFile` escribe en `.tmp` y renombra, de modo que un corte a mitad deja intacto el guardado anterior (10.3); `parseSaveGame` rechaza un esquema con otra versión antes de tocar nada (10.4). Expuesto en el juego con F5/F9 sobre `savegame.json`. 14 pruebas en `test_save_game`.

#### RUN-11 — Crear vertical slice jugable del runtime

`DONE` · avance **100 %** · estimación 5 j · restante **0 j** · depende de RUN-10

- **En el repo hoy:** examples/juego.cpp; los 25 demos GL-free registrados en CTest; CMakeLists.txt::suite_gl_free; tests/Level/WorldTopologyTests.cpp
- **Lectura:** Cerrado el 28-08-2026. El vertical slice ya existía y era más ambicioso que lo que pedía la ficha; lo que faltaba era **11.3, el modo headless determinista**: 25 demos GL-free promovidos a CTest tras comprobar uno a uno que salen con código 0 y que dos ejecuciones seguidas dan salida idéntica. CI ahora construye el target agregado `suite_gl_free` en vez de una lista escrita a mano en el YAML (por la que varios tests ya registrados nunca se compilaban). Promover `demo_mundo` destapó un fallo real: `WorldTopology` guardaba una **referencia** a un elemento del vector que iba ampliando mientras lo recorría, y con un nivel de muchas puertas la realocación dejaba la referencia colgando — `std::length_error` al construir la topología de Boundington. Arreglado y con test de regresión que falla con el código antiguo. `demo_proyectos` queda fuera: necesita `tools/` en el cwd (es TOOL-09).


### Fase 3 — Editor visual de mapas MVP

#### EDT-01 — Crear ejecutable MotorEditor

`PARCIAL` · avance **60 %** · estimación 4 j · restante **2 j** · depende de RUN-11

- **En el repo hoy:** examples/level_editor.cpp (2707 líneas); CMakeLists.txt::level_editor
- **Lectura:** El editor existe y es grande, pero es un ejemplo, no un ejecutable MotorEditor con su propio árbol editor/. Tarea real: promoverlo.

#### EDT-02 — Integrar Dear ImGui y docking

`REPLANTEAR` · avance **0 %** · estimación 5 j · restante **5 j** · depende de EDT-01

- **En el repo hoy:** examples/level_editor.cpp (cabecera); include/Render/HudManager.h; include/Render/HudWidgets.h
- **Lectura:** DECISIÓN YA TOMADA EN CONTRA: la UI se hizo con el HUD propio del motor porque no había red para vendorizar Dear ImGui. Esta ficha no se puede ejecutar como está escrita — hay que decidir (ver INF-02).

#### EDT-03 — Integrar viewport OpenGL del mapa

`DONE` · avance **80 %** · estimación 6 j · restante **2 j** · depende de EDT-02

- **En el repo hoy:** examples/level_editor.cpp; include/Render/IsometricRenderer.h; include/Render/Framebuffer.h
- **Lectura:** El viewport del mapa dibuja con el renderer real. Falta zoom/pan formalizados y rejilla configurable.

#### EDT-04 — Crear panel de proyecto y árbol de mapas

`PARCIAL` · avance **50 %** · estimación 6 j · restante **3 j** · depende de EDT-03

- **En el repo hoy:** include/Editor/ProjectHub.h; include/Editor/ProjectIndex.h; examples/demo_proyectos.cpp
- **Lectura:** Hub de proyectos con modos y comprobaciones. Falta el árbol de mapas navegable dentro del editor.

#### EDT-05 — Crear navegador de assets

`PARCIAL` · avance **45 %** · estimación 6 j · restante **4 j** · depende de EDT-04

- **En el repo hoy:** EditorState::setPaletteFilter(); assets/generated/authoring/asset_manifest.json; assets/objects/previews_pendientes.json
- **Lectura:** Hay filtro por texto y familias sobre miles de entradas. Falta navegador con miniaturas: previews_pendientes.json es literalmente la lista de lo que falta.

#### EDT-06 — Crear paleta de tiles

`DONE` · avance **85 %** · estimación 6 j · restante **1 j** · depende de EDT-05

- **En el repo hoy:** EditorState::setTilePalette/setTilePaletteLabels/nextPaletteEntry; EDITOR_TAXONOMIA.md
- **Lectura:** Paleta con etiquetas de autor, filtro y paginación. Falta la rejilla visual del tileset.

#### EDT-07 — Implementar selección y picking isométrico

`DONE` · avance **85 %** · estimación 5 j · restante **1 j** · depende de EDT-06

- **En el repo hoy:** include/Core/Math/IsoMath.h; examples/level_editor.cpp (picking bajo cursor)
- **Lectura:** Picking isométrico resuelto y en uso diario.

#### EDT-08 — Implementar lápiz, borrador y cuentagotas

`PARCIAL` · avance **75 %** · estimación 6 j · restante **2 j** · depende de EDT-07

- **En el repo hoy:** EditorState::paintTile/eraseTile/applyAt
- **Lectura:** Lápiz y borrador sí. Cuentagotas (tomar el GID de la celda bajo el cursor) NO existe y es media jornada.

#### EDT-09 — Implementar rectángulo, relleno y selección

`PARCIAL` · avance **80 %** · estimación 6 j · restante **2 j** · depende de EDT-08

- **En el repo hoy:** EditorState::fillTiles/setSelection/copySelection/pasteAt/eraseSelection
- **Lectura:** Relleno conectado, selección rectangular y portapapeles hechos, con un solo paso de historial. Falta la herramienta rectángulo/línea que PINTA.

#### EDT-10 — Crear panel de capas

`PARCIAL` · avance **65 %** · estimación 5 j · restante **2 j** · depende de EDT-09

- **En el repo hoy:** EditorState::addLayer/removeLayer/setActiveLayer/layerName
- **Lectura:** Capas creables, renombrables y activables. Faltan visibilidad, bloqueo, opacidad y reordenar — justo lo que hace útil el panel.

#### EDT-11 — Crear inspector contextual

`DONE` · avance **80 %** · estimación 6 j · restante **2 j** · depende de EDT-10

- **En el repo hoy:** include/Editor/ObjectInspector.h; src/Editor/ObjectInspector.cpp; EditorState::setObject*(); commit 22aac83, 008585f
- **Lectura:** Inspector por instancia con botín, eventos, variantes y propiedades libres. Es de lo más maduro del editor.

#### EDT-12 — Implementar CommandHistory y deshacer-rehacer

`REPLANTEAR` · avance **70 %** · estimación 7 j · restante **3 j** · depende de EDT-11

- **En el repo hoy:** EditorState::undo/redo/recordUndo/Snapshot
- **Lectura:** Hay deshacer/rehacer robusto pero por SNAPSHOT COMPLETO del mapa, no por comandos. Funciona; escala mal en memoria. Decidir si se migra o se documenta el límite.

#### EDT-13 — Implementar copiar, pegar y duplicar

`DONE` · avance **85 %** · estimación 4 j · restante **1 j** · depende de EDT-12

- **En el repo hoy:** EditorState::copySelection/pasteAt/eraseSelection/Clipboard
- **Lectura:** Copiar, pegar y borrar selección con capas y objetos incluidos.

#### EDT-14 — Implementar guardado, autosave y recuperación

`PARCIAL` · avance **40 %** · estimación 5 j · restante **3 j** · depende de EDT-13

- **En el repo hoy:** EditorState::exportTmx/exportLevelJson; examples/level_editor.cpp (guardado)
- **Lectura:** Se guarda a petición. No hay autosave, ni archivo de recuperación, ni aviso de cambios sin guardar.

#### EDT-15 — Validar creación de un mapa sin usar Tiled

`PARCIAL` · avance **70 %** · estimación 4 j · restante **2 j** · depende de EDT-14

- **En el repo hoy:** include/Editor/EditorValidation.h; examples/demo_editor_state.cpp; level_editor_output.png
- **Lectura:** Ya se crean mapas sin Tiled. Falta el guion de validación formal y la evidencia reproducible en CI.


### Fase 4 — Herramientas avanzadas de mapas

#### MAP-01 — Crear importador y editor visual de tilesets

`PARCIAL` · avance **35 %** · estimación 8 j · restante **6 j** · depende de EDT-15

- **En el repo hoy:** include/Core/Resources/TextureAtlas.h; assets/textures/terreno_iso.json; REMAPEO_TILES.md; SPRITE_DIMENSIONS_GUIDE.md
- **Lectura:** Atlas y remapeo resueltos por JSON y por scripts. Editor visual de tilesets: no existe.

#### MAP-02 — Implementar TileDefinition y metadatos

`PARCIAL` · avance **40 %** · estimación 6 j · restante **4 j** · depende de MAP-01

- **En el repo hoy:** include/Render/Tile.h; TmxTilesetSettings::collisionGids; EDITOR_TAXONOMIA.md
- **Lectura:** Metadatos de tile hoy = colisión booleana por GID. Falta TileDefinition con terreno, coste, sonido y etiquetas.

#### MAP-03 — Implementar autotiles isométricos

`PENDIENTE` · avance **0 %** · estimación 9 j · restante **9 j** · depende de MAP-02

- **En el repo hoy:** —
- **Lectura:** Nada hecho. Es la tarea de mayor riesgo técnico de la fase 4 en isométrico.

#### MAP-04 — Crear editor de colisiones direccionales

`PARCIAL` · avance **25 %** · estimación 7 j · restante **6 j** · depende de MAP-03

- **En el repo hoy:** include/Render/OcclusionRules.h; src/Render/TileMap.cpp
- **Lectura:** Colisión booleana + reglas de oclusión. Direccional (por lado de celda) no existe.

#### MAP-05 — Crear capas y canales de regiones

`PENDIENTE` · avance **10 %** · estimación 7 j · restante **7 j** · depende de MAP-04

- **En el repo hoy:** include/Core/Math/GridBounds.h
- **Lectura:** Los rectángulos existen como matemática, no como regiones autorales con canal y semántica.

#### MAP-06 — Implementar editor de propiedades del mapa

`PARCIAL` · avance **30 %** · estimación 5 j · restante **4 j** · depende de MAP-05

- **En el repo hoy:** include/Level/LevelDefinition.h; FORMATO_NIVELES.md
- **Lectura:** Hay campos de nivel en el formato pero no un panel de propiedades del mapa.

#### MAP-07 — Implementar colocación de entidades

`DONE` · avance **85 %** · estimación 7 j · restante **2 j** · depende de MAP-06

- **En el repo hoy:** EditorState::placeObject/placeSpawn/setObjectPatrol; include/Level/LevelDefinition.h::ObjectSpawn
- **Lectura:** Colocación de entidades con patrulla, escala, variante y destino. Muy por delante de la ficha.

#### MAP-08 — Implementar prefabs y plantillas

`PARCIAL` · avance **30 %** · estimación 7 j · restante **5 j** · depende de MAP-07

- **En el repo hoy:** include/Level/ObjectCatalog.h; OBJECT_VARIANTS_FOR_EDITOR.md
- **Lectura:** Variantes de objeto sí; prefabs multi-celda y plantillas de habitación, no.

#### MAP-09 — Implementar navegación y pathfinding

`PARCIAL` · avance **20 %** · estimación 7 j · restante **6 j** · depende de MAP-08

- **En el repo hoy:** include/Game/EnemyBrain.h; src/Game/EnemyBrain.cpp
- **Lectura:** IA de enemigo con movimiento simple. Sin malla de navegación ni A*.

#### MAP-10 — Implementar conexiones y puntos de aparición

`DONE` · avance **80 %** · estimación 5 j · restante **1 j** · depende de MAP-09

- **En el repo hoy:** EditorState::setObjectTargetLevel/setObjectTargetPosition; include/Level/WorldTopology.h
- **Lectura:** Conexiones y destino por instancia editables desde el mapa.

#### MAP-11 — Mejorar profundidad, elevación y capas altas

`PARCIAL` · avance **45 %** · estimación 8 j · restante **5 j** · depende de MAP-10

- **En el repo hoy:** include/Render/OcclusionRules.h; include/Render/BlobShadow.h; src/Render/IsometricRenderer.cpp
- **Lectura:** Orden de dibujo y oclusión resueltos. Elevación real (alturas) no existe.

#### MAP-12 — Validar aldea, interior y mazmorra conectados

`PARCIAL` · avance **55 %** · estimación 5 j · restante **3 j** · depende de MAP-11

- **En el repo hoy:** examples/demo_ciudad.cpp; examples/demo_mundo.cpp; examples/demo_este_norte.cpp; examples/demo_este_sur.cpp; assets/levels/
- **Lectura:** Hay aldea e interiores conectados; falta la mazmorra y el guion de validación conjunto.


### Fase 5 — Sistema de eventos tipo RPG Maker

#### EVT-01 — Definir EventDefinition y EventPage

`PARCIAL` · avance **45 %** · estimación 6 j · restante **4 j** · depende de MAP-12

- **En el repo hoy:** include/RPG/Definitions/RpgCoreDefinitions.h::EventDefinition; include/RPG/NarrativeEngine.h::NarrativeBeat; assets/adventures/*.json
- **Lectura:** Existe EventDefinition y el beat narrativo, con aventuras reales en JSON. Falta el concepto de PÁGINA con condiciones que selecciona qué versión del evento corre.

#### EVT-02 — Definir Condition, Trigger y EventCommand

`PARCIAL` · avance **55 %** · estimación 6 j · restante **3 j** · depende de EVT-01

- **En el repo hoy:** NarrativeEngine.h::NarrativeCondition/BeatTrigger(Talk,Enter,Auto)/NarrativeEffect/EffectType
- **Lectura:** Condiciones, disparadores y efectos ya son datos tipados. Falta que el efecto sea un EventCommand extensible en vez de un enum cerrado.

#### EVT-03 — Implementar EventContext e EventInterpreter

`PARCIAL` · avance **50 %** · estimación 8 j · restante **4 j** · depende de EVT-02

- **En el repo hoy:** include/RPG/NarrativeEngine.h::NarrativeEngine/NarrativeState; src/RPG/NarrativeEngine.cpp
- **Lectura:** Hay intérprete con estado que ejecuta beats y devuelve resultados. Falta el contexto de ejecución por evento y la pila de llamadas.

#### EVT-04 — Implementar variables e interruptores

`PARCIAL` · avance **60 %** · estimación 5 j · restante **2 j** · depende de EVT-03

- **En el repo hoy:** NarrativeState::setFlag/clearFlag/contadores
- **Lectura:** Interruptores y contadores globales funcionan. Falta exponerlos como tabla editable con nombres.

#### EVT-05 — Implementar interruptores locales por evento

`PENDIENTE` · avance **10 %** · estimación 3 j · restante **3 j** · depende de EVT-04

- **En el repo hoy:** NarrativeState::markBeatFired()
- **Lectura:** Lo más parecido es marcar un beat como disparado. Interruptores locales por instancia de evento no existen.

#### EVT-06 — Implementar scheduler, esperas y procesos paralelos

`PARCIAL` · avance **30 %** · estimación 7 j · restante **5 j** · depende de EVT-05

- **En el repo hoy:** NarrativeEngine.h::TimerDefinition/TimerState; examples/demo_temporizadores.cpp
- **Lectura:** Temporizadores de misión hechos (commit f5ef2fd). Esperas dentro de un evento y procesos paralelos, no.

#### EVT-07 — Implementar comandos de diálogo y elecciones

`PARCIAL` · avance **55 %** · estimación 6 j · restante **3 j** · depende de EVT-06

- **En el repo hoy:** include/Level/ObjectCatalog.h::DialogueData; GameMode::Dialogue; assets/adventures/*.json
- **Lectura:** Diálogos con elecciones funcionan en las aventuras. Falta el comando de diálogo genérico dentro del intérprete de eventos.

#### EVT-08 — Implementar comandos de estado e inventario

`PARCIAL` · avance **55 %** · estimación 6 j · restante **3 j** · depende de EVT-07

- **En el repo hoy:** include/RPG/InventoryEngine.h; include/RPG/ConditionEngine.h; NarrativeEffect
- **Lectura:** Dar/quitar objeto y aplicar estado ya son efectos. Falta cubrir dinero, experiencia y equipo desde el mismo comando.

#### EVT-09 — Implementar movimiento, audio, cámara y teletransporte

`PARCIAL` · avance **30 %** · estimación 7 j · restante **5 j** · depende de EVT-08

- **En el repo hoy:** GameSession::LevelTransition; include/Render/Camera.h
- **Lectura:** Teletransporte sí, cámara programable a medias, AUDIO NO EXISTE en el repo (ver GDB-11).

#### EVT-10 — Implementar ramas, bucles y eventos comunes

`PARCIAL` · avance **25 %** · estimación 7 j · restante **6 j** · depende de EVT-09

- **En el repo hoy:** NarrativeEngine (condiciones por beat)
- **Lectura:** Hay ramificación por condición de beat, pero no bucles, ni etiquetas, ni eventos comunes reutilizables.

#### EVT-11 — Crear editor visual de páginas de evento

`PENDIENTE` · avance **5 %** · estimación 9 j · restante **9 j** · depende de EVT-10

- **En el repo hoy:** EditorState::setObjectEvent()
- **Lectura:** La semilla está puesta: una instancia ya puede referenciar un eventId. Todo el editor de páginas está por hacer.

#### EVT-12 — Crear editor vertical de comandos

`PENDIENTE` · avance **0 %** · estimación 10 j · restante **10 j** · depende de EVT-11

- **En el repo hoy:** —
- **Lectura:** El editor vertical de comandos es el paquete más caro del roadmap (10 j.) y no tiene nada hecho. Es LA pieza de 'sin tocar C++'.

#### EVT-13 — Crear editor visual de rutas de movimiento

`PARCIAL` · avance **15 %** · estimación 6 j · restante **6 j** · depende de EVT-12

- **En el repo hoy:** EditorState::setObjectPatrol(); level_editor.cpp (tecla T)
- **Lectura:** Las patrullas se marcan con el rectángulo. Rutas con pasos ordenados no existen.

#### EVT-14 — Implementar validación estática de eventos

`PARCIAL` · avance **35 %** · estimación 6 j · restante **4 j** · depende de EVT-13

- **En el repo hoy:** include/Editor/EditorValidation.h::checkRpg(); AUDITORIA_CONTRATOS.md
- **Lectura:** Ya valida referencias de skills, sprites, enemigos, pickups, NPC y transiciones. Ampliarlo a eventos es continuación, no obra nueva.

#### EVT-15 — Implementar depuración paso a paso de eventos

`PENDIENTE` · avance **0 %** · estimación 7 j · restante **7 j** · depende de EVT-14

- **En el repo hoy:** —
- **Lectura:** Sin depurador paso a paso. Depende de EVT-03 rehecho con contexto.

#### EVT-16 — Validar misión de llave, puerta y cofre

`PARCIAL` · avance **40 %** · estimación 5 j · restante **3 j** · depende de EVT-15

- **En el repo hoy:** assets/adventures/*.json; NarrativeEngine::ObjectiveDefinition
- **Lectura:** Hay misiones con objetivos en JSON escritas a mano; falta hacerlo íntegramente desde el editor, que es el criterio real.


### Fase 6 — Base de datos y sistemas de juego

#### GDB-01 — Crear GameDatabase extensible

`PARCIAL` · avance **65 %** · estimación 8 j · restante **3 j** · depende de EVT-16

- **En el repo hoy:** include/RPG/Catalogs/RpgCatalogs.h (Catalog<T>, CatalogLoader); include/Core/Resources/ICatalog.h; assets/catalogs/
- **Lectura:** Ya hay una GameDatabase de facto: catálogo genérico + 19 tipos de definición cargados de JSON. Falta el registro extensible y el índice inverso de referencias.

#### GDB-02 — Crear editor maestro-detalle de registros

`PENDIENTE` · avance **10 %** · estimación 8 j · restante **8 j** · depende de GDB-01

- **En el repo hoy:** include/Editor/ProjectHub.h (patrón de menús)
- **Lectura:** El editor maestro-detalle de la base de datos no existe. Hoy los catálogos se editan a mano en JSON.

#### GDB-03 — Implementar actores, NPC y enemigos

`DONE` · avance **80 %** · estimación 7 j · restante **2 j** · depende de GDB-02

- **En el repo hoy:** RpgCoreDefinitions.h::NpcDefinition/MonsterDefinition; include/RPG/CharacterSheet.h; include/Game/EnemyBrain.h
- **Lectura:** Actores, NPC y enemigos con definición rica. Falta editarlos sin tocar JSON (GDB-02).

#### GDB-04 — Implementar atributos, estados y progresión

`DONE` · avance **80 %** · estimación 7 j · restante **2 j** · depende de GDB-03

- **En el repo hoy:** include/RPG/Stat.h; include/RPG/TierRules.h; include/RPG/ConditionEngine.h; include/RPG/DicePoolEngine.h; include/Game/Morality.h
- **Lectura:** Atributos, estados, progresión por tiers y moralidad implementados y probados en demos.

#### GDB-05 — Implementar objetos, equipo e inventario

`DONE` · avance **80 %** · estimación 7 j · restante **2 j** · depende de GDB-04

- **En el repo hoy:** RpgCoreDefinitions.h::EquipmentDefinition/ConsumableDefinition; include/RPG/InventoryEngine.h; include/RPG/Defense.h
- **Lectura:** Objetos, equipo e inventario con reglas de defensa.

#### GDB-06 — Implementar habilidades y efectos

`DONE` · avance **85 %** · estimación 7 j · restante **2 j** · depende de GDB-05

- **En el repo hoy:** include/RPG/Definitions/SkillDefinition.h; src/RPG/SkillExecutor.cpp; include/RPG/EnhanceHooks.h; examples/demo_skills.cpp
- **Lectura:** Habilidades y efectos con ejecutor y ganchos de mejora.

#### GDB-07 — Implementar combate básico configurable

`PARCIAL` · avance **70 %** · estimación 10 j · restante **4 j** · depende de GDB-06

- **En el repo hoy:** include/Game/BattleState.h; src/Game/BattleState.cpp; examples/demo_battle_nd6.cpp; reglamento Ed.3 (commits b89759f, 0a2a172)
- **Lectura:** Combate enfrentado nd6 cerrado y validado. Lo que falta es que sea CONFIGURABLE por el creador, no fijo en C++.

#### GDB-08 — Implementar misiones y objetivos

`PARCIAL` · avance **65 %** · estimación 8 j · restante **3 j** · depende de GDB-07

- **En el repo hoy:** NarrativeEngine.h::ObjectiveDefinition/ObjectiveStatus/TimerDefinition
- **Lectura:** Objetivos y temporizadores hechos; falta diario de misiones y edición visual.

#### GDB-09 — Implementar diálogos reutilizables

`PARCIAL` · avance **50 %** · estimación 6 j · restante **3 j** · depende de GDB-08

- **En el repo hoy:** ObjectCatalog.h::DialogueData; assets/adventures/
- **Lectura:** Diálogos por objeto y por aventura. No hay árbol de diálogo reutilizable entre NPC.

#### GDB-10 — Implementar tablas de loot y encuentros

`PARCIAL` · avance **55 %** · estimación 6 j · restante **3 j** · depende de GDB-09

- **En el repo hoy:** RpgCoreDefinitions.h::LootTableDefinition; EditorState::setObjectLootTable/addObjectLoot
- **Lectura:** Tablas de botín definidas y asignables por instancia. Tablas de encuentro, no.

#### GDB-11 — Implementar audio, música y ambiente

`PENDIENTE` · avance **0 %** · estimación 6 j · restante **6 j** · depende de GDB-10

- **En el repo hoy:** —
- **Lectura:** NO HAY AUDIO en el repositorio. Requiere elegir y vendorizar una dependencia; con el entorno sin red, es una decisión de infraestructura (ver INF-04).

#### GDB-12 — Implementar título, menús y carga de partida

`PARCIAL` · avance **40 %** · estimación 8 j · restante **5 j** · depende de GDB-11

- **En el repo hoy:** include/RPG/CharacterCreationScreen.h; include/Render/HudManager.h; include/Editor/ProjectHub.h
- **Lectura:** Creación de personaje y menús HUD existen. Carga de partida imposible hasta RUN-10.

#### GDB-13 — Validar aventura completa de treinta minutos

`PARCIAL` · avance **50 %** · estimación 7 j · restante **4 j** · depende de GDB-12

- **En el repo hoy:** examples/demo_campana_boundington.cpp; examples/demo_aventura.cpp; GDD_MG.md
- **Lectura:** Ya hay campaña larga jugable, pero creada con código y JSON a mano, no desde el editor.


### Fase 7 — Playtest, extensiones y exportación

#### TOOL-01 — Implementar playtest desde el editor

`PENDIENTE` · avance **10 %** · estimación 6 j · restante **6 j** · depende de GDB-13

- **En el repo hoy:** examples/level_editor.cpp; examples/juego.cpp
- **Lectura:** Editor y juego son dos ejecutables sin puente. Lanzar el juego desde el editor está por hacer.

#### TOOL-02 — Implementar probar desde la celda seleccionada

`PENDIENTE` · avance **0 %** · estimación 4 j · restante **4 j** · depende de TOOL-01

- **En el repo hoy:** —
- **Lectura:** Depende de TOOL-01.

#### TOOL-03 — Implementar recarga en caliente segura

`PENDIENTE` · avance **0 %** · estimación 6 j · restante **6 j** · depende de TOOL-02

- **En el repo hoy:** include/Core/Resources/ResourceManager.h
- **Lectura:** Sin recarga en caliente. El ResourceManager es el punto de enganche.

#### TOOL-04 — Crear consola e inspector de estado

`PARCIAL` · avance **20 %** · estimación 6 j · restante **5 j** · depende de TOOL-03

- **En el repo hoy:** include/Render/HudWidgets.h; salida de consola de los demos
- **Lectura:** Los demos imprimen estado, pero no hay consola ni inspector en vivo.

#### TOOL-05 — Crear gestor completo de recursos y referencias

`PARCIAL` · avance **30 %** · estimación 8 j · restante **6 j** · depende de TOOL-04

- **En el repo hoy:** include/Core/Resources/ResourceManager.h; assets/generated/authoring/asset_manifest.json; include/Editor/EditorValidation.h
- **Lectura:** Manifiesto de assets y validación de referencias ya existen: es la base del gestor.

#### TOOL-06 — Diseñar API de plugins de runtime

`PENDIENTE` · avance **0 %** · estimación 7 j · restante **7 j** · depende de TOOL-05

- **En el repo hoy:** —
- **Lectura:** Sin API de plugins. Decidir antes si el objetivo 1.0 la necesita o se pospone a 1.1.

#### TOOL-07 — Diseñar API de extensiones de editor

`PENDIENTE` · avance **0 %** · estimación 7 j · restante **7 j** · depende de TOOL-06

- **En el repo hoy:** —
- **Lectura:** Ídem TOOL-06.

#### TOOL-08 — Implementar permisos, versiones y migraciones de plugins

`PENDIENTE` · avance **0 %** · estimación 6 j · restante **6 j** · depende de TOOL-07

- **En el repo hoy:** —
- **Lectura:** Ídem TOOL-06.

#### TOOL-09 — Implementar empaquetado de recursos usados

`PENDIENTE` · avance **0 %** · estimación 6 j · restante **6 j** · depende de TOOL-08

- **En el repo hoy:** assets/generated/authoring/asset_manifest.json
- **Lectura:** El manifiesto permitiría calcular los recursos usados; el empaquetado no existe.

#### TOOL-10 — Implementar exportación Windows y Linux

`PARCIAL` · avance **20 %** · estimación 8 j · restante **7 j** · depende de TOOL-09

- **En el repo hoy:** .github/workflows/release.yml; CMakePresets.json
- **Lectura:** Hay flujo de release y presets, pero exporta el motor, no un juego del usuario.

#### TOOL-11 — Implementar diagnósticos de exportación

`PENDIENTE` · avance **0 %** · estimación 5 j · restante **5 j** · depende de TOOL-10

- **En el repo hoy:** —
- **Lectura:** Depende de TOOL-09/TOOL-10.

#### TOOL-12 — Publicar alpha del creador de juegos

`PENDIENTE` · avance **0 %** · estimación 3 j · restante **3 j** · depende de TOOL-11

- **En el repo hoy:** —
- **Lectura:** Hito de publicación.


### Fase 8 — Beta, documentación y versión 1.0

#### REL-01 — Crear juego de ejemplo completo

`PARCIAL` · avance **45 %** · estimación 10 j · restante **6 j** · depende de TOOL-12

- **En el repo hoy:** assets/adventures/ (9 aventuras); GDD.md; GDD_MG.md; examples/demo_campana_boundington.cpp
- **Lectura:** El universo Boundington/Onegai está muy desarrollado como contenido. Falta rehacerlo con las herramientas públicas, que es el criterio de la ficha.

#### REL-02 — Crear tutorial interactivo del editor

`PENDIENTE` · avance **0 %** · estimación 7 j · restante **7 j** · depende de REL-01

- **En el repo hoy:** —
- **Lectura:** Sin tutorial interactivo.

#### REL-03 — Documentar formatos, runtime y plugins

`PARCIAL` · avance **60 %** · estimación 7 j · restante **3 j** · depende de REL-02

- **En el repo hoy:** FORMATO_NIVELES.md; FORMATO_AVENTURAS.md; FORMATO_PROYECTOS.md; ARCHITECTURE.md; CONTRATO_IDS.md; SPRITE_AUTHORING_GUIDE.md
- **Lectura:** Documentación de formatos muy por delante de lo habitual. Falta la de runtime y plugins.

#### REL-04 — Realizar pruebas con proyectos grandes

`PENDIENTE` · avance **0 %** · estimación 7 j · restante **7 j** · depende de REL-03

- **En el repo hoy:** —
- **Lectura:** Sin pruebas de carga con proyectos grandes.

#### REL-05 — Realizar pruebas de usabilidad con creadores

`PENDIENTE` · avance **0 %** · estimación 8 j · restante **8 j** · depende de REL-04

- **En el repo hoy:** ESTUDIO_EDITOR_RPG.md
- **Lectura:** Hay estudio de mercado, no pruebas con creadores reales.

#### REL-06 — Corregir bloqueos y pérdida de datos

`PENDIENTE` · avance **0 %** · estimación 10 j · restante **10 j** · depende de REL-05

- **En el repo hoy:** —
- **Lectura:** 

#### REL-07 — Optimizar mapas, entidades y editor

`PENDIENTE` · avance **0 %** · estimación 10 j · restante **10 j** · depende de REL-06

- **En el repo hoy:** —
- **Lectura:** 

#### REL-08 — Preparar instalador y actualización

`PENDIENTE` · avance **0 %** · estimación 7 j · restante **7 j** · depende de REL-07

- **En el repo hoy:** —
- **Lectura:** 

#### REL-09 — Congelar esquema 1.0 y compatibilidad

`PENDIENTE` · avance **0 %** · estimación 5 j · restante **5 j** · depende de REL-08

- **En el repo hoy:** —
- **Lectura:** 

#### REL-10 — Publicar MotorGraphico Creator 1.0

`PENDIENTE` · avance **0 %** · estimación 3 j · restante **3 j** · depende de REL-09

- **En el repo hoy:** —
- **Lectura:** 
