# Inventario de lo sobrante (AV14-24)

**Hecho por:** Claude Haiku, 15/10/2026, solo lectura. No se ha cambiado ningún fichero del motor.
**Entrada:** `tools/buscar_sobrante.py` (copia de `MotorGraphico-main` de hoy, sin cambios).
**Salida del script:** `$HOME/ag/haiku/sobrante.txt` y `$HOME/ag/haiku/sobrante.json` (en la máquina de Oriol, fuera de la carpeta sincronizada).

**Nota sobre el número:** el título de la tarea hablaba de 13 piezas. El script encuentra **11 cabeceras** que solo se refieren ellas mismas, su `.cpp` y su prueba. No he buscado las 2 que faltan. Si el 13 venía de otro criterio, hay que decirlo al repetir la medida.

**Cómo leer las líneas:** es el total de la familia (cabecera + `.cpp` + prueba), como lo cuenta el script, no solo la cabecera.

## Piezas (11)

| # | Pieza (ficheros) | Líneas | Qué hace | Quién la podría usar en el juego | Pruebas |
|---|---|---|---|---|---|
| 1 | `include/Editor/AssetBrowserState.h` + `src/Editor/AssetBrowserState.cpp` | 1405 | Modelo del navegador de recursos: buscar por nombre y tipo, ver dimensiones y frames, importar un PNG al proyecto. No tiene dibujo (el panel ImGui `AssetBrowserPanel.cpp` no está hecho). | Editor de niveles, en el panel de recursos (paquete E-13). | Sí: `tests/Editor/AssetBrowserStateTests.cpp`, `tests/Editor/AssetBrowserTests.cpp` (registradas en CMake). |
| 2 | `include/Game/AudioDirector.h` + `src/Game/AudioDirector.cpp` | 1056 | Política de sonido: decide qué suena en cada momento (cambio de modo, golpes, bajar la música) y lo traduce a llamadas de `RPG::AudioEngine`. El motor de audio ya existe y está probado, pero ningún ejecutable lo llama. | `GameSession` o `Application`, en los cambios de modo y de combate (paquete H6-01). | Sí: `tests/Game/AudioDirectorTests.cpp`. |
| 3 | `include/Game/TargetSelection.h` + `src/Game/TargetSelection.cpp` | 839 | Selección de objetivo en combate: el cursor, las filas, cancelar y confirmar. Usa las reglas de `BattleMenuActions` y devuelve la `BattleAction`. | Menú de combate de `BattleState` / `Application` (paquete H2-02). | Sí: `tests/Game/TargetSelectionTests.cpp`. |
| 4 | `include/Editor/ProjectDatabaseState.h` + `src/Project/ProjectDatabaseState.cpp` | 657 | Formularios de edición RPG del proyecto: crear objetos, equipo, habilidades y enemigos. | Editor, pantallas de catálogo RPG (paquete E-16). | Sí: `tests/Editor/ProjectDatabaseTests.cpp` y `tests/Project/ProjectDatabaseTests.cpp`. |
| 5 | `include/Game/QuestLogState.h` + `src/Game/QuestLogState.cpp` | 570 | Diario de misiones: copia de cada misión al empezar, progreso de objetivos, estado activa, completada o fallida. | Pantalla de diario en el HUD de la partida. | Sí: `tests/Game/QuestLogTests.cpp`. |
| 6 | `editor/include/ProblemsPanel.h` + `editor/src/ProblemsPanel.cpp` | 454 | Panel ImGui de problemas y búsqueda de eventos. Dibuja `ProblemsState`, que sí está compilado. | Editor, panel de problemas (paquete E-15). | No hay prueba del panel (es dibujo). `ProblemsState` tiene la suya. |
| 7 | `include/HUD/ConditionWidget.h` + `src/HUD/ConditionWidget.cpp` | 346 | Qué condiciones muestra el HUD para un combatiente: nombre y rondas restantes. Solo texto. | HUD de combate, en el enemigo o en la ficha (paquete H2-03). El montaje en `Application` está pendiente, según el propio `.cpp`. | Sí: `tests/Game/ConditionWidgetTests.cpp`. |
| 8 | `include/HUD/DiceRollWidget.h` + `src/HUD/DiceRollWidget.cpp` | 333 | Muestra una tirada Nd6: caras, éxitos, grado y pifia. Usa las mismas etiquetas que el log de combate. | HUD al hablar, atacar o usar una habilidad (paquete H2-01). El montaje está pendiente. | Sí: `tests/Game/DiceRollWidgetTests.cpp`. |
| 9 | `include/Game/TitleScreen.h` + `src/Game/TitleScreen.cpp` | 280 | Pantalla de título: lista fija (campaña, escenarios en trabajo, salir) y qué proyectos no salen y por qué. | Arranque del juego, antes de elegir partida (paquete G-08). | Sí: `tests/Game/TitleScreenTests.cpp`. |
| 10 | `include/RPG/LootResolver.h` + `src/RPG/LootResolver.cpp` | 241 | Resuelve una tabla de botín: oro, objetos de entrada y de caída. Versión con semilla para pruebas. | Fin de combate, cofres o enemigos (datos de `loot_tables`). Hoy no se lee ningún `loot_tables.json`. | Sí: `tests/RPG/LootResolverTests.cpp` (`add_motor_unit_test` en CMake, línea 1373). |
| 11 | `include/Game/BattleMenuState.h` + `src/Game/BattleMenuState.cpp` | 226 | Menú de combate configurable desde datos: nombre, coste, disponibilidad y motivo de bloqueo de cada acción. | Menú de combate de `BattleState` (paquete G-13). Hoy el menú vivo es otro. | Sí: `tests/Game/BattleMenuTests.cpp` (`cmake/BattleMenuTests.cmake`). |

## Lo que he visto de paso

- **Ninguna de las 11 la incluye otro fichero.** El script no ha encontrado un solo `#include` fuera de su familia.
- **Ojo con las pruebas:** 10 de 11 tienen prueba registrada en CMake (la que falta es el panel de problemas, que es dibujo). Las cabeceras de la tabla de arriba son las que nadie enchufa, pero sus pruebas sí se ejecutan. Eso hace que la decisión de cablear o retirar sea la cuestión, no si funcionan.
- **Montajes pendientes con nombre:** `ConditionWidget` y `DiceRollWidget` dicen en su cabecera que el montaje en `Application` está pendiente. Eso no es basura, es trabajo sin terminar.

## Repetidos (lo que da el script en la segunda lista)

8 grupos de cuerpos iguales en ficheros distintos, unas 91 líneas de sobra. Son los que más cuestan, porque un arreglo llega a unas copias y no a otras:

- `makeWhiteTexture` (13 líneas × 3): `examples/creacion_personaje.cpp`, `examples/demo_battle_hud.cpp`, `examples/demo_hud.cpp`.
- `leeJson` (9 × 3): `src/Game/Bloques.cpp`, `src/Game/Monumentos.cpp`, `src/Game/RegistroSprites.cpp`.
- `leeFichero` (10 × 2): `src/Game/ReglasMoral.cpp`, `src/RPG/Narrative/HeroeEnLaHistoria.cpp`.
- `aMinusculas` (9 × 2): `src/Editor/EditorState.cpp`, `src/RPG/CharacterCreationScreen.cpp`.
- `unir` (9 × 2): `src/Game/Codice.cpp`, `src/HUD/Pergamino/PaginasJugador.cpp`.
- `trim` (7 × 2): `src/Editor/ProjectMapsFiles.cpp`, `src/Editor/ProjectPanelState.cpp`.
- `fail` (6 × 2): `src/Game/SaveGame.cpp`, `src/Game/SaveGameFormat.cpp`.
- `Fallo` (6 × 2): `src/RPG/Eventos/AnfitrionNarrativo.cpp`, `src/RPG/SessionCommandHost.cpp`.

## Qué decide Oriol (no yo)

Para cada una de las 11: cablear (y en qué paquete) o retirar. Para los repetidos: unificar o dejar. No he tocado nada de eso.
