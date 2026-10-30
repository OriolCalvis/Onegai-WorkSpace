# Tablero de estabilización — reclama tu fila ANTES de abrir el primer fichero

Plan y normas: [`PLAN_ESTABILIZACION.md`](PLAN_ESTABILIZACION.md) · Ley:
[`../../AGENTS.md`](../../AGENTS.md) · Método: [`METODOLOGIA_RELEVOS.md`](METODOLOGIA_RELEVOS.md)

> **LEE ESTO ANTES DE EMPEZAR (23/09):** el repositorio está dentro de una carpeta
> sincronizada y la sincronización **revierte ficheros en silencio**. Dos relevos
> completos se perdieron así. **Commitea pronto y a menudo**: lo que no está en git
> no existe. Detalle: [`INCIDENCIA_SINCRONIZACION_2026-09-23.md`](INCIDENCIA_SINCRONIZACION_2026-09-23.md).

**Cómo se reclama.** Escribe tu nombre y la fecha en la fila, en un commit que
**solo toque este fichero**. Si la fila ya tiene nombre, coge otra: no hay dos
agentes en el mismo fichero. Al terminar, pon `hecho` y deja el número nuevo.

**Antes de empezar**, siempre:

```
python3 tools/linea_base_core.py       # ¿está verde lo que no es mío?
git status --short                     # ¿hay trabajo sin commitear encima?
```

**Antes de cerrar**, siempre: la guardia en verde, prueba GL-free nueva que cae
si deshaces tu cambio, logs en las transiciones, `Result` en los errores, y la
línea base actualizada a la baja en tu mismo commit.

---

## Fase 0 — Desbloquear · **serial, va primero**

| ID | Tarea | Quién | Estado | Toca |
| --- | --- | --- | --- | --- |
| EST-01 | `git add` de los 5 `.cmake` sin versionar + commit del WIP ajeno | qwen (autorizado por Oriol) | **hecho** 21/09 — commits `9e20a11` `1bd3b27` `91ebf1b` `cf549f0`; guardia 0 configuracion | `CMakeLists.txt`, `cmake/*` |
| EST-02 | Regenerar `alcance_real.json` y re-congelar la línea base | qwen | **parado, espera decision** — regenerar destapa 11 muertos nuevos (42 → 50); subir el trinquete es de Oriol: `decisiones/D0007` | `planificacion/*.json` |

## Fase 1 — Cimientos · **serial, antes de la fase 2**

| ID | Tarea | Quién | Estado | Toca | No toques |
| --- | --- | --- | --- | --- | --- |
| EST-10 | Sistema de logs `Core/Log` | **qwen** 21/09 | **hecho** 21/09 — `motor_log` (librería propia, no fuentes sueltas: ley 4c) + `test_log` **30/30** GL-free en `suite_gl_free`. Datos (`LogRecord`, `LogConfig` que se valida sola), controlador (`Logger`: filtro, secuencia monótona, id de sesión, reloj inyectable) y piezas (`LogFormat` puro, `MemorySink`, `ConsoleSink`, `FileSink::open` → `Result`). Falsabilidad comprobada por mutación: quitar el vuelco de `Error` y el contador de filtrados → 4 rojos. **Ninguna excepción cruza la frontera**: sink roto y reloj nulo se cuentan (`droppedBySink`, tiempo 0) en vez de lanzar. **Cableado en `motor_editor`** (ley 4 satisfecha: `medir_alcance` da las 5 fuentes alcanzadas); lo que queda para EST-51 es el camino del jugador en `juego` | `include/Core/Log/*`, `src/Core/Log/*`, `tests/Core/LogTests.cpp`, `cmake/LogTests.cmake` | nada más |
| EST-11 | `ScopeGuard` + frontera de excepciones | **qwen** 21/09 | **hecho en lo que cabe sin tocar cuellos** 22/09 — `ScopeGuard.h` con `test_scope_guard` 7/7 GL-free y su primer consumidor real (`FileSink::open` retira la carpeta que creó si no puede abrir el fichero; 2 pruebas nuevas en `test_log`, 32/32, falsabilidad comprobada quitando la guarda). Guía de 29 líneas en `AGENT_HANDOFF.md` («Errores y logs»). Fronteras envueltas de verdad: la apertura de fichero del log y la de excepciones del proceso (`editor/src/main.cpp` → `Result`/log con categoría `editor`, más `--log <ruta>` opt-in por decisión de Oriol). **Queda y no se puede hacer aquí:** los 2 `std::sto*` de `EditorApplication.cpp:209,211` (`--frames abc` responde «stoi: no conversion»), el parseo JSON y `tinyxml2`. Ese fichero es uno de los 17 cuellos y el trinquete de la guardia prohíbe hacerle crecer ni una línea: el envoltorio entra con **EST-26**, que es quien lo parte. La prueba de que ninguna excepción cruza frontera es **EST-52** | `include/Core/Errors/ScopeGuard.h`, los 2 `std::sto*`, aperturas de fichero | la lógica de nadie |

## Fase 2 — Troceado · **16 de 17 filas se pueden coger a la vez**

| ID | Fichero | Hoy | Quién | Estado | Objetivo |
| --- | --- | ---: | --- | --- | --- |
| EST-20 | `src/Engine/Application.cpp` | 2444 → **1283** | **Claude** 24/09 | **relevos A y B hechos** — 2305 → 1122 en HEAD, en cinco ficheros por flujo (`Bucle` 388, `Nivel` 379, `Hud` 310, `Pantallas` 190, `Cierre` 154) + `ApplicationInternal.h`. El relevo B corta ya CON trabajo ajeno dentro: corte doble, HEAD al índice y árbol al disco (§3 bis). Queda `init` (657) y `processInput` (197). Ver `seguimiento/EST-20.json` | `init` + EST-20b |
| EST-20b | `include/Engine/Application.h` | 103 miembros | — | pendiente — los 103 miembros a clases de datos. Relevo gemelo de EST-23b, y el que de verdad arregla la estructura mixta | `AppData` + flujo + piezas |
| EST-21 | `editor/src/level_editor.cpp` | 2777 → **2145** | **Claude** 24/09 | **relevos A y B hechos** — núcleo GL-free (`Paleta` 313, `Opciones` 73, `Arranque` 64) en la librería `motor_editor_niveles` con `test_editor_nucleo` (13 casos, verdes), y la pantalla de proyectos fuera del `main` (`LevelEditorHub` 193). Ver `seguimiento/EST-21.json` | EST-21b: el `main()` |
| EST-21b | `editor/src/level_editor.cpp` — el `main()` | 2145 → **2136** | **Codex 24/09** | **hecho** — `LevelEditorEditState` GL-free concentra la salida, selección temporal, búsqueda, navegación de paleta y feedback; 17 casos de núcleo y el ejecutable enlazan. No duplica `EditorState` ni `ObjectInspector`. | comando de guardar + fases input/render |
| EST-21d | `editor/src/level_editor.cpp` — comando `guardarNivel` | ~100 | **Codex 24/09** | **reservada** — contexto/resultado GL-free sobre el escritor existente; no se mueve input, HUD, proyecto ni render | guardar probado, luego input/render |
| EST-21c | **Decisión de Oriol** | — | — | `level_editor` y `motor_editor` son DOS editores vivos. Mientras los dos lo estén, cada mejora hay que hacerla dos veces. Retirar uno no es un borrado: es una migración | decidir |
| EST-22 | `include/RPG/Catalogs/RpgCatalogs.h` | 1922 | **Claude** 23/09 | **hecho** — 1922 → 211 en 8 cabeceras por familia. Los 34 consumidores sin tocar. 90 pruebas en 9 baterías, 0 fallos | una cabecera por familia |

| EST-23 | `src/Game/GameSession.cpp` + `.h` | 1167 / 40 miembros | **Claude** 23/09 | **fuente hecha** — 1167 → 296 en 5 ficheros. 145 pruebas, 0 fallos. **Queda EST-23b**: los 40 miembros de la cabecera a una clase de datos, que es otro relevo | `SessionData` + flujo + piezas |
| EST-24 | `src/Editor/EditorState.cpp` + `.h` | 1075 | **Claude** 24/09 | **hecho** — 1075 → 367 en 6 ficheros (pintar / objetos / exportar / núcleo + 2 ayudantes compartidos). 50 pruebas, 0 fallos | datos + controlador + herramientas |
| EST-25 | `src/RPG/NarrativeEngine.cpp` + `.h` | 744 / 543 | **Claude** 22/09 | **hecho y commiteado** `f03f9d4` — 543→152 y 744→228, datos a `RPG/Narrative/*`. Revertido por la sincronización el 23/09 y restaurado; 122 pruebas verdes | datos + carga + flujo |
| EST-26 | `editor/src/EditorApplication.cpp` | 738 → **597** | **Claude** 24/09 | **relevo A hecho** — `EditorOpciones` (contenedor + flujo, GL-free, **13 pruebas**), `EditorGui` y `EditorVistaDeMapa` fuera de un `run()` de 541 líneas. **Cierra el fallo que EST-11 dejó anotado**: `--frames abc` respondía «stoi: no conversion» y ahora dice qué opción falla y por qué; `strtol`/`strtof` con `errno`, sin lanzar nada. Ver `seguimiento/EST-26.json` | EST-26b: el andamio del humo |
| EST-26b | `editor/src/EditorApplication.cpp` — el andamio del test de humo | 214 de las 541 líneas de `run()` | — | pendiente — 16 bloques `if (smoke …)` repartidos por dentro del bucle de fotograma: **no es el editor, es la prueba del editor**, y mezclarlos es la estructura mixta que la ley prohíbe. Cabe en un `GuionDeHumo` con los 13 miembros que sólo usa el harness. Con eso `run()` baja a ~390 y entra bajo el techo. **Necesita una máquina con pantalla**: el oráculo no puede ejecutar `--smoke-check` | `GuionDeHumo` |
| EST-27 | `src/Map/MapDocument.cpp` | 724 | **Claude** 23/09 | **hecho** — 724 → 334 en 5 ficheros (una clase de capa por fichero + la API de edición). 59 pruebas en 7 baterías, 0 fallos | capas / entidades / regiones |
| EST-28 | `src/Project/ProjectIndex.cpp` | 683 → 367 | **opencode** 24/09 | **hecho** — split a `ProjectIndexDuplicate.cpp` (316 línies); `cuellos` 11→10 | índice + operaciones |
| EST-29 | `src/Game/SaveGame.cpp` | 661 → 330 | **opencode** 23/09 | **hecho** — split a `SaveGameFormat.cpp` (331 línies); `cuellos` 12→11; 150/150 gl-free | escribir / leer / migrar |
| EST-30 | `include/RPG/Definitions/RpgCoreDefinitions.h` | 687 | **Claude** 23/09 | **hecho** — 687 → 17 en 6 cabeceras, emparejadas con las familias de `Catalogs/`. Los 15 consumidores sin tocar. 82 pruebas, 0 fallos | partir por familia (es datos) |
| EST-31 | `src/Editor/ProblemsState.cpp` | 643 | **Claude** 24/09 | **hecho** — 643 → 228 en 4 ficheros, **y con librería propia** (`motor_editor_problems`): se compilaba suelto en tres objetivos | problemas + detectores |
| EST-32 | `src/RPG/EventCommands.cpp` | 585 → **178** | **Claude** 24/09 | **hecho** — un trabajo por fichero: catálogo (178, se queda con el nombre porque lo compilan sueltos 7 objetivos), `Eventos/ValidacionDeComandas` (122), `Eventos/CondicionesYPaginas` (185), `Eventos/AnfitrionNarrativo` (134) + `EventosInterno.h`. 4/4 baterías verdes. Sale de la lista de cuellos | un fichero por familia |
| EST-33 | `src/RPG/AdventureValidation.cpp` | 518 → 285 | **opencode** 23/09 | **hecho** — split a `AdventureBeatValidation.cpp` (225 línies); `cuellos` 13→12; 149/149 gl-free | una regla, una pieza |
| EST-34 | `src/Editor/ObjectInspector.cpp` | 515 / 18 | **Codex 22/09** | **hecho**: `ObjectInspectorData` + `ObjectInspectorInput`; sin tocar `EditorState`, `level_editor` ni `CMakeLists.txt` | 515 → 209 líneas; 18 → 3 miembros; `demo_object_inspector` 1/1, `motor_level` compila y el alcance llega a `level_editor`; el objetivo final está bloqueado por WIP ajeno en `Application`; cuellos 17 → 16, estado 8 → 7 |
| EST-35 | `include/Render/IsometricRenderer.h` | 27 miembros | **Codex 23/09 — hecho** | `IsometricSceneData` e `IsometricPostProcessData` separan estado GL-free; FBOs siguen en el renderer; sin tocar `Application`, `EditorApplication`, CMake ni tests | 27 → 9 miembros; C-19 1/1, C-23 GL 1/1; `juego` y `motor_editor` compilan y el alcance llega a ambos |
| EST-36 | `HudWidgets.h` · `EnemyBrain.h` · `HudTextWidgets.h` · `Player.h` | 20/19/18/17 | **Codex 23/09 — `HudWidgets`, `EnemyBrain` y `HudTextWidgets` hechos** | `HudTextWidgetsData` concentra texto, menú y diálogo; sin tocar `Application`, `HudWidgets`, `Player`, `HudManager` ni CMake | `HudTextWidgets` 18 → 7 miembros; G-10/G-14 1/1, `juego` y `level_editor` compilan; el alcance llega a ambos y la línea base de estado baja 6 → 5 |

| EST-36p | include/Render/Player.h — datos del jugador | 8 campos reales → **1** colaborador | **Codex 26/09** | **implementado, GL-free verificado** — PlayerData conserva estado y ficha externa; 40 casos Player + 19 regresiones verdes, alcance a juego. Guardia rápida OK. Humo de juego bloqueado por vtable ajena de SessionCommandHost; guardia completa por Application.h 107/104 | seguimiento/EST-36-PLAYER.json; EST-36 datos completados |

| EST-28r | Reparar relevo incompleto de ProjectIndex en HEAD | 682 → **402** | **Codex 26/09** | **implementado y verificado** — fuera cuatro duplicados; fuente extraída enlazada y escritor original preservado. 5/5 CTest de proyectos, ambos editores enlazan; guardia rápida verde | seguimiento/EST-28-REPARACION.json |

## Fase 3 — Estructura

| ID | Tarea | Quién | Estado |
| --- | --- | --- | --- |
| EST-53 | **Pruebas registradas que nunca se construían** | 42 en CTest | **Claude** 24/09 | **hecho** — `cmake/C24RenderRpgTests.cmake` registraba 42 baterías con `add_motor_unit_test()`, que no enlaza nada: CTest las listaba como «Not Run» y parecían cobertura. 34 eran duplicados de pruebas ya bien cableadas (comprobado ejecutando las 34 equivalentes); las 8 propias quedan enlazadas y colgadas de `suite_gl_free`. Además, **7 de esas 8 no estaban versionadas** (126 casos que sólo existían en un disco), y `HudTransformTests` fallaba 5 casos con expectativas escritas desde una fórmula supuesta. Suite GL-free: 71% → **91%**. ⚠️ Commiteado con el rótulo «EST-33», que ya era de opencode; el trabajo es éste | mentir menos en la lista |
| EST-54 | La ley de «y en ningún clon», para las fuentes | — | **Claude** 24/09 | **hecho** — `comprueba_fuentes()` en la guardia: toda fuente que CMake nombre tiene que estar versionada. Falla en el índice, avisa en el disco. Lo primero que encontró no era mío: HEAD citaba `tests/Engine/CameraTransitionTests.cpp` desde E-13 sin versionarlo, así que **un clon limpio de HEAD no configuraba**. 52 casos en `test_linea_base_core.py` | que no se repita |
| EST-41a | Compilaciones sueltas **redundantes** | `fuentes_directas` 72 | **Claude** 24/09 | **hecho** — 14 objetivos compilaban una fuente suelta *además* de enlazar la librería que ya la contiene: mismo binario, un objeto de más y 14 puntos falsos en la métrica. Sólo las demostrables (cierre transitivo de `target_link_libraries`). **72 → 58**, mismos 6 fallos de construcción antes y después | que la métrica mida |
| EST-23b | `include/Game/GameSession.h` — 40 miembros | 664 → **552** | **opencode** 28/09 | **hecho, y REPARADO el mismo día.** 40 miembros extraídos a `SessionData.h` (clase de datos). `GameSession.h` queda con 1 data member (`unique_ptr<SessionData>`). 5 ficheros `.cpp` a `sessionData()->m_xxx`. **Regresión que dejó el primer aterrizaje** (`f128e4a`): al vacirse la lista de inicialización del constructor (solo `m_data`), las ocho asignaciones de colaboradores no se trasladaron al cuerpo — `m_map`, `m_catalog`, `m_skillCatalog`, `m_monsterCatalog`, `m_level`, `m_playerBody`, `m_playerSkills`, `m_inventory` **no se asignaban en ningún sitio del motor**. Toda sesión arrancaba sin catálogo, sin nivel, sin cuerpo, sin inventario: `test_game_session` daba 12 fallos y **segfault** (EXIT=139) y `test_application_cableado` 2/11. Reparado en `src/Game/GameSession.cpp` copiando las asignaciones de `f128e4a^` al principio del cuerpo, antes de que las lean `m_playerPosition` y el reparto de spawns. **Ahora: `test_game_session` 24/24, `test_application_cableado` 10/11, 135 casos de la vía de sesión/inventario/negocio sin un fallo.** Lección: al mover 40 miembros a una clase de datos, la lista de inicialización se reescribe entera, no se vacía. | `SessionData` + flujo + piezas |
| EST-23b1 | `include/Game/GameSession.h` — tipos de valor | 741 → **664** | **Claude** 24/09 | **primera mitad hecha** — `SessionTipos.h` (97) se lleva `GameMode`, `OwnedBusiness`, `PlayerCarryOver`, `LevelTransition`, `ShopOffer`. `SaveGame.h` y `SaveSlotState.h` dejan de arrastrar `BattleState`, `NarrativeEngine`, `World` y los 20 catálogos Nd6 para usar dos tipos. **Los 40 miembros siguen donde estaban**: ésa es EST-23b | `SessionTipos.h` |
| EST-40 | `motor_core` → `motor_hud` + `motor_entidades` (solo CMake) | — | bloqueada por EST-01 |
| EST-41 | Matar las compilaciones sueltas | — | **72 → 58 el 24/09** (EST-41a: las 14 redundantes). Las 58 que quedan **no** son redundantes: casi todas evitan arrastrar GL, así que dependen de EST-40 |
| EST-42 | **Cablear o retirar: 9 funcionalidades terminadas que nadie enchufó** | **Oriol** | medido el 24/09 con `tools/buscar_sobrante.py --muertos`. **No son 42**: `medir_alcance.py` daba 43 «sin alcance» pero no sigue los `#include` entre cabeceras (siete eran los `CatalogLoaders*` que `RpgCatalogs.h` incluye). Con el criterio comprobable — ¿se refiere a ella alguien que no sea ella, su `.cpp` y su prueba? — quedan **9, 4959 líneas**, todas con pruebas verdes:<br>`AssetBrowserState` 1405 · `InventoryMenuState` 823 · `Audio` 653 · `ProjectDatabaseState` 625 · `ProblemsPanel` 454 · `TitleScreen` 280 · `OptionsLocaleBridge` 252 · `LootResolver` 241 · `BattleMenuState` 226.<br>**No es basura: es trabajo hecho sin enchufar.** Dos avisos: `ProblemsPanel.cpp` **se compila dentro de `motor_editor`** y nadie lo llama (peso muerto en un binario vivo), y `TitleScreen` es el único **superado** de verdad (H0-07 lo sustituyó por `TitleMenuState`) — se puede retirar en cuanto ese trabajo se commitee |
| EST-55 | Doce copias del volcado a PPM → una | **Claude** 24/09 | **hecho** — `motor_ppm` (el formato, GL-free, **6 pruebas**) + `motor_captura_gl` (la lectura). Cinco de las doce copias **no volteaban las filas**: llevaban años escribiendo la captura del revés. `level_editor.cpp` tenía **dos** volcados al mismo fichero |
| EST-56 | Seis pruebas que CI no ejecutaba nunca | **Claude** 24/09 | **hecho** — cuatro fragmentos de `cmake/` que nadie incluía: `test_exportar` (30 casos), `test_linea_base_core` (52), `linea_base_core` en sus dos modos, `test_validar_todo`. Escritas, verdes, invisibles. Más `tools/buscar_sobrante.py`, que es lo que hace repetible preguntar qué sobra |

## Fase 4 — Que no vuelva

| ID | Tarea | Quién | Estado |
| --- | --- | --- | --- |
| EST-50 | La guardia detecta estructura mixta | **Codex** 23/09 | **hecho** — detector conservador en la puerta rápida; 38 casos sembrados y sin candidatos en el árbol. No toca CMake, código productivo ni la línea base concurrente |
| EST-51 | Cobertura de logs del camino del jugador | **opencode** 26/09 | **hecho** 26/09 — `test_player_path_log` **7/7** GL-free en `suite_gl_free`. Las 7 etapas (título, partida, nivel, combate, tienda, guardar, morir) dejan entrada con su categoría, y el rastro se lee: una línea por evento, secuencia correlativa sin huecos e id de sesión en su propio campo. El guardado se comprueba de verdad (serializa y vuelve a leer). Fijos dos contratos: lo filtrado no consume secuencia, y un id con salto de línea se rechaza. Commits `7f9e86f` (fila) `a87a40b` (prueba). No toco `Core/Log` (es de qwen) |
| EST-52 | Prueba de que ninguna excepción cruza frontera | **opencode** 26/09 | **hecho 8/8, con un hueco que es de qwen** 26/09 — `test_boundary_exceptions` 8/8 GL-free. N4 se comprueba llamando a la frontera con entradas que la obligan a fallar: el log (sink que falla, sink nulo, reloj nulo, id con salto de línea, ruta imposible) y el guardado (leer basura, guardar donde no se puede). Falsabilidad **comprobada de verdad**: rompí `parseSaveGame` para que aceptara basura, la prueba se puso roja, y al restaurar volvió a verde. **HALLAZGO PARA qwen:** un `ILogSink` que *lanza* en vez de devolver `Result` incumple su contrato y `Logger::emit` **no lo contiene** — la excepción sale del módulo. Hay `try`/`catch` en ninguno de los `src/Core/Log/*.cpp`. No lo he tocado porque `Core/Log` es su carril. El caso `un_sink_que_lanza_aun_se_sale_del_modulo` lo fija a propósito, así que **se pondrá en rojo el día que se contenga**: es la alarma. **CERRADO por qwen 28/09 (`3a6d5dc`)**: `Logger` llama a los sinks a través de `writeSafely`/`flushSafely`, que atrapan cualquier excepción y la devuelven como `Result`; `emit()` la cuenta en `droppedBySink()`, así que no queda invisible. El caso se volteó a lo que pedía su propio comentario: `un_sink_que_lanza_no_se_sale_del_modulo`. Dos ganancias que no eran evidentes hasta medirlas: un sink roto ya no deja el log **ciego** (antes su excepción se llevaba el bucle por delante y los demás sinks no recibían nada), y el destructor ya no puede provocar `std::terminate`. Falsabilidad comprobada por mutación: sin el `catch` de `write` fallan 4 casos; sin el de `flush` el binario **aborta** (exit 134). `test_log` 36/36 (4 casos nuevos), `test_boundary_exceptions` 8/8, guardia OK sin deuda nueva. Commits `f128e4a` (`.cmake`, lo recogió EST-23b) `9363aa8` (claim) `0388558` (prueba) |

---

## Hecho

| ID | Tarea | Quién | Fecha | Resultado |
| --- | --- | --- | --- | --- |
| — | Exportador G-06 (núcleo) | Claude | 20/09 | `tools/exportar.py`, 30 pruebas, entrega de 13 MB |
| — | Guardia `linea_base_core` | Claude | 20/09 | techo, estado, frontera GL, configuración. 30 pruebas |
| — | Ley del repositorio en 5 puertas | Claude | 21/09 | `AGENTS.md`, handoff, pre-commit, pre-push, CI, checklist 0.5 |
| — | Relevo: `ProjectSerializer.cpp` | Claude | 21/09 | **1081 → 218** en 4 ficheros, 47 pruebas verdes |
| EST-10 | Sistema de logs `Core/Log` | qwen | 21/09 | `motor_log` (5 fuentes) + `test_log` **30/30** GL-free. Una entrada, una línea: `[HH:MM:SS.mmm] NIVEL categoria #seq s=id mensaje`. Filtro por nivel mínimo y por categoría, reloj inyectable, sinks de memoria/consola/fichero con `Result` en el fallo |
| EST-11 | `ScopeGuard` + frontera de errores | qwen | 22/09 | `ScopeGuard.h` (solo cabecera) + `test_scope_guard` 7/7; estrena uso real en `FileSink::open`. Guía «Errores y logs» en `AGENT_HANDOFF.md`. `main.cpp` del editor convierte la excepción en log con categoría y añade `--log <ruta>` opt-in: `motor_editor` ya alcanza las 5 fuentes de `Core/Log` |
| EST-33 | Split `AdventureValidation.cpp` | opencode | 23/09 | **518 → 285** línies; nou fitxer `AdventureBeatValidation.cpp` (225 línies). `cuellos` 13→12. 149/149 gl-free |
| EST-29 | Split `SaveGame.cpp` | opencode | 23/09 | **661 → 330** línies; nou fitxer `SaveGameFormat.cpp` (331 línies: serialització + parse + `applyNarrativeState`). `cuellos` 12→11. 150/150 gl-free |
| EST-28 | Split `ProjectIndex.cpp` | opencode | 24/09 | **683 → 367** línies; nou fitxer `ProjectIndexDuplicate.cpp` (316 línies: E-11 suggestId/uniqueId/createNamed/duplicate). `cuellos` 11→6. 150/150 gl-free |
