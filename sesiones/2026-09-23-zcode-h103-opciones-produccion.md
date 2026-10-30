# 2026-09-23 · zcode · H1-03: MenuLogic a producción + Game::OptionsState (Opciones deja de ser TODO)

**Dónde:** `MotorGraphico-main` (fila propia del TABLERO, reclamada en `687dc464` antes de crear el primer fichero).
**Commits:** MG `71317fb` (producción + tests, 23/09) + MG `4e6c4bd` (cierre: desacople del test, 27/09).
**Nota de continuidad:** la sesión arrancó el 23/09 y se interrumpió varias veces (cortes de herramienta los días 23→27); este fichero cubre el arco completo y el cierre del 27/09.

## Qué se hizo

Continuación del Gantt de jugabilidad (`planificacion/gantt_jugabilidad_H.puml`) tras H1-01. Ficha **H1-03 — «G-10: pausa y opciones de verdad»**: «Mover MenuLogic a producción y hacer que Opciones abra algo». Criterio de cierre: «Esc → Opciones → volumen, idioma, controles: la pantalla existe y se puede volver».

Diagnóstico real antes de programar (la ficha estaba a medio desactualizar, como H1-04):

- La navegación de menú ya existía en producción (`PauseMenu::moveUp/moveDown`), pero **no había primitivo GL-free reutilizable**: `tests/Game/G10PauseAndHudTests.cpp` duplicaba la semántica como dos mocks inline (`MenuLogic`/`CmdMenuLogic`).
- La pantalla de Opciones era **un TODO del G-10**: `PauseMenu` tiene la entrada, `confirm()` devuelve `PauseAction::Options`, y nadie la consume.

Entrega (rodaja GL-free, sin tocar `Application.cpp` — cruce con la zona H de opencode):

1. **`include/Game/MenuLogic.h`** (nuevo, solo cabecera): wraparound + clamp con guarda de vacío, la semántica que fijaba el mock. Los dos mocks del test G-10 pasan a consumirla (falsabilidad: si se borra, `test_g10_pause_and_hud` no compila).
2. **`include/Game/OptionsState.h` + `src/Game/OptionsState.cpp`** (nuevos): pantalla de Opciones GL-free al patrón `PauseMenu`/`TitleMenuState` — 6 filas (MUSICA/EFECTOS/AMBIENTE con pasos 0..10 y clamp, IDIOMA que cicla CATALA/CASTELLA/ENGLISH, CONTROLES, VOLVER); `confirm()` → `Back`/`ShowControls`/`None`; `displayLines()`/`labels()` para el HUD; `volume(key)`/`language()` para quien cablee. Deliberadamente fuera: persistir valores (H6-03), consumir `Locale` (H1-05) y el mapeo a `AudioEngine` (va en el cableado).
3. **`tests/Game/OptionsStateTests.cpp`** (nuevo, 6 casos) y registro en `cmake/G10PauseAndHudTests.cmake` junto al test de su paquete — cero líneas en el bloque de includes de `CMakeLists.txt`.
4. Bug propio cazado por la verificación: el primer test hacía 6 `moveDown` desde la fila 0 y volvía a la 0 (6 entradas, vuelta completa); corregido a 1 `moveUp` (wraparound) para llegar a VOLVER.

## Incidencias del arco (todas documentadas)

- **Commit del blob de CMakeLists abortado el 23/09** (corte de herramienta tras el diff): mi línea de `OptionsState.cpp` en `motor_level` no llegó a HEAD, y el hunk del árbol se perdió en las reescrituras ajenas de esos días. **Curado por otro agente**: al re-verificar el 27/09, `src/Game/OptionsState.cpp` ya está en los sources de `motor_level` de HEAD. Deuda cerrada sin acción mía.
- **Refs fantasma bloqueando clones/checkouts**: locks rancios renombrados DENTRO de `refs/` (`.lock.rancio.*`) dejan de ser ignorados por git y se leen como refs inválidas (`fatal: bad object refs/heads/...`). Reubicados en `.git/rancio-locks/` (renombre, sin borrar nada). **Trampa nueva para la Parte 3**: los locks rancios dentro de `refs/` deben moverse FUERA del árbol de refs, nunca renombrarse in situ. También un duplicado « 2» de `.git/refs/stash` (la trampa de macOS en el peor sitio) causaba `bad object refs/stash`; reubicado igual.
- **Rojo ajeno en HEAD (27/09, sigue vivo):** `Application.cpp:905` llama `BattleState::tickRound` desde `d7bc4e1` (H0-04/H0-05, cableado de Application), pero `BattleState.h/.cpp` con la implementación siguen **sin versionar en el árbol** → `motor_core` no compila en clon limpio y arrastra a `test_g10_pause_and_hud` y a `juego`. Dueño de la cura: opencode (le basta versionar sus dos ficheros). Registrado en el Pendiente del TABLERO.
- Mi `test_options_state` enlazaba `motor_core` por inercia del patrón G-10; como OptionsState no toca HudWidgets, **desacoplado** (`4e6c4bd`) para que el rojo ajeno no lo tumbe — mismo criterio que `test_skill_effect_real_assets` en su día.
- Mi bullet del Pendiente del 22/09 (HEAD referenciaba 8 tests sin versionar) queda **curado por EST-33** (`e192931`/`fe6ec67` de Codex): HEAD puro ahora configura en checkout limpio (verificado en worktree sin copiar nada).

## Verificación

- Worktree de HEAD `f181b70` (checkout limpio de tracked-only): configure OK, `motor_level` compila, **`test_options_state` 6/6** (binario directo y por ctest), con `motor_core` roto de por medio.
- El 23/09 se verificó además en clon limpio de `0bd1652` + el cambio: `test_g10_pause_and_hud` rewireado en verde y `test_g13_combat_commands` sin regresión, `motor_level` y `juego` compilan (ese día HEAD aún enlazaba).
- **Claude construyó H1-05 encima** el 24/09 (`OptionsLocaleBridge`, MG `3deaac5`): OptionsState ya tiene consumidor puente hacia `Locale`, y compila `OptionsState.cpp` directo en su test.

## Qué falta de H1-03 (tramo de cableado, no hecho)

Que `Application` abra Opciones: Esc → pausa → Opciones → pintar `OptionsState::displayLines()` con HudCommandMenu, Izquierda/Derecha → `adjustLeft/Right`, VOLVER → `Back`. `Application.cpp` es el cruce de 14 paquetes y hoy además está roto por `tickRound` ajeno — cuando se versione, el cableado es mecánico. El puente de idioma ya existe (H1-05 de claude); persistir valores es H6-03.

## Coexistencia

- Fila reclamada antes de crear ficheros; cerrada a Hecho el 27/09.
- Nada del WIP ajeno commiteado: `BattleState.h/.cpp` con `tickRound` siguen en el árbol, intactos, esperando a su dueño.
- Locks: tras cada operación, `.git/*.lock` limpio; los 4 fantasmas reubicados visibles en `.git/rancio-locks/`.
- Worktree de verificación `/tmp/mg-verify-h103b` retirado al terminar.
