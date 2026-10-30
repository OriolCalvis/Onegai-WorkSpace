# 2026-09-22 · zcode · H1-01: G-13 a producción (el menú de combate desde datos sale del test)

**Dónde:** `MotorGraphico-main` (fila propia del TABLERO, reclamada antes de crear el primer fichero).
**Commits:** MG `9eeb27b` (4 ficheros por pathspec) + MG `43cc272` (línea de `CMakeLists.txt` con blob construido).

## Qué se hizo

Oriol pidió seguir el Gantt del MotorGraphico usando el MAPA_DEL_CODIGO, con otros agentes trabajando a la vez. Elección de tarea: los dos Gantt históricos (`motor_grafico_gantt.puml` Fases 0–5 y `motor_grafico_gantt_rpg.puml` Fases 6–12) están cerrados según sus propias notas; los vivos son el **corte B** (`planificacion/gantt_corte_b.puml`, C/D cerrados, E/G en vuelo) y la **Fase H de jugabilidad** (`planificacion/gantt_jugabilidad_H.puml`, la más reciente). Del H se tomó **H1-01 — «G-13: menú de combate desde datos (mover de tests/ a src/)»**, primer rescate de la subfase H1, cuya dependencia H0-04 está entregada.

Diagnóstico verificado antes de empezar (regla: mirar qué existe de verdad):

- `buildBattleMenuSkillActions`, `selectTargetsFor`, `translateToBattleAction`, `BattleMenuEntry` y `TargetOption` vivían **solo dentro de `tests/Game/G13CombatCommandsTests.cpp`** (177 líneas de lógica en un fichero de test) — G-13 cerró en 17/09 con «su lógica vive en sus tests» (ALCANCE_REAL).
- `Player::populate_from_class_race_background()` (que la ficha H1-04 del Gantt H daba por faltante) **ya existe y está versionada** (`include/Render/Player.h:132` + `tests/Render/PlayerPopulateTests.cpp`): la ficha de JUGABILIDAD_FASE_H está desactualizada en ese punto. No se ha tocado.
- `GameSession.cpp:232` ya llama a `tick_conditions` (H0-06 también avanzado en el árbol). No se ha tocado.

Entrega:

1. **`include/Game/BattleMenuActions.h` + `src/Game/BattleMenuActions.cpp` (NUEVOS)**: las tres funciones y los tres tipos movidos a producción **sin cambio de semantica** — mismas 5 reglas de disponibilidad (isCasted, tier_ok, canPay, knows, tagsOk), mismas cadenas de coste/motivo («1 PM», «Nivell insuficient (cal T4…»), que son contrato del test y de la futura UI. Funciones puras GL-free sobre `SkillSet`/`ICombatant`/catálogos publicados, en la lib **`motor_level`** (donde ya viven `BattleState.cpp` y `BattleMenuState.cpp`).
2. **`tests/Game/G13CombatCommandsTests.cpp` rewired**: eliminadas las 177 líneas locales, ahora incluye `Game/BattleMenuActions.h` y ejercita el código de producción exactamente como lo consumirá el juego. Los 6 `MOTOR_TEST_CASE` quedan **intactos** (es la falsabilidad: si alguien borra el fichero de producción, el test deja de enlazar).
3. **CMake**: `src/Game/BattleMenuActions.cpp` añadido a los sources de `motor_level` (`CMakeLists.txt`, 1 hunk de 3 líneas) y al `add_executable` de `cmake/G13CombatCommandsTests.cmake` (mismo patrón anticolisión de compilar fuentes directas que ya usaban BattleState/Skill).

**Tramo siguiente de H1-01, NO hecho (deliberado):** que `Application.cpp:654` sustituya la lista fija de 5 entradas por `buildBattleMenuSkillActions`. `Application.cpp` es el fichero de cruce de 14 paquetes y hoy lleva el WIP H0 de opencode sin commitear encima; tocarlo ahora sería pisar trabajo vivo. Cuando ese WIP se versione, el cableado es corto: construir el menú con el `SkillSet`+`SkillCatalog` de la sesión y pintar `costLine`/`blockedReason` en el HUD de combate.

## Verificación (clon limpio de `f18d8a8` + solo mi cambio)

- `test_g13_combat_commands`: **6/6** contra `src/` (binario ejecutado directamente: los 6 casos PASS, EXIT 0).
- `test_battle_menu` y `test_battle_state`: sin regresión (3/3 binarios Passed en ctest).
- `motor_level` compila con el fichero nuevo y **`juego` enlaza** (el consumidor final ya puede usar el menú).

## Coexistencia

- Fila reclamada en TABLERO **antes** de crear ficheros (commit raíz `877a8b0d`); HEAD avanzó durante la sesión (`f18d8a8`, Codex reservando EST-36) y el clon de verificación se hizo sobre ese HEAD nuevo.
- Nada del WIP ajeno tocado ni commiteado. `CMakeLists.txt` subido como **blob construido** (HEAD + solo mis 3 líneas, técnica de la Parte 3): el índice real queda apuntando al blob nuevo y los hunks ajenos siguen viéndose como unstaged, sin cambios fantasma «preparados».
- **Rojo preexistente de HEAD documentado (patrón del Pendiente 33, otra vez):** el `CMakeLists.txt` de HEAD referencia `tests/Engine/CameraTransitionTests.cpp` (y `tests/Core/{EngineException,Result}Tests.cpp`, `tests/Editor/ProjectDatabaseTests.cpp`, `tests/Render/{Entity,OcclusionRules,Player,StaticEntity}Tests.cpp`), **todos sin versionar** — un clon limpio de HEAD no llega a configurar hasta que su dueño los versione. Copiados SOLO al clon desechable de `/tmp` para poder verificar; no se han commiteado. Registrado en el Pendiente del TABLERO.
- `BattleMenuState.*` (el menú simple anterior, consumido solo por `test_battle_menu`) no se ha tocado: H1-01 no pedía unificarlos; si alguien retira el viejo más adelante, que quede en su registro.
- Sin locks de git colgados al terminar (`ls .git/*.lock` limpio).
