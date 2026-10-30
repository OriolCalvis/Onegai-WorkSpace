# H1-03 (Gantt Fase H, zona opencode) — cableado Esc→Opciones en Application

- **Fecha:** 27/09/2026
- **Autor:** opencode
- **Fila del TABLERO:** cruce de la rodaja H1-03 de zcode — el wiring en `Application` es zona opencode (así lo declara la fila de zcode).
- **Zona tocada (todo en el árbol, sin commit):**
  - `include/Engine/Application.h` (miembro `m_options`, `confirmOptionsCommand`, include de `Game/OptionsState.h`)
  - `src/Engine/Application.cpp` (ruteo Pause con submenu, `confirmPauseCommand` → Options, `confirmOptionsCommand`)
  - `CMakeLists.txt` (1 línea: `src/Game/OptionsState.cpp` en `motor_level`)
  - `cmake/OptionsLocaleBridgeTests.cmake` (quitar `OptionsState.cpp` de la compilación directa para que no haya símbolo duplicado con motor_level)
  - `tests/Engine/ApplicationCableadoTests.cpp` (accesos + caso `cableado_opciones_se_abre_y_vuelve`)

## Qué pedía el Gantt

`JUGABILIDAD_FASE_H.md` H1-03:

> Pausa y opciones de verdad. «Opciones» era un TODO; hay que hacer que esc → Opciones → volumen, idioma, controles abra algo y se pueda volver (criterio: «la pantalla existe y se puede volver»).

La rodaja GL-free (`MenuLogic` + `Game::OptionsState`, 6 filas: música/efectos/ambiente/idioma/controles/volver) la entregó zcode el 23/09; el cableado desde `Application` (cruzar opciones ↔ submenu) es el tramo de la zona H de opencode que `Application.cpp` tenía declarado como pendiente en la fila de zcode.

## Qué hice

1. **Miembro colaborador `m_options`** (`std::unique_ptr<Game::OptionsState>`, justificado N3): null = manda el menú de pausa; no-null = las teclas del modo Pause van al submenu.
2. **Ruteo en `processInput` (modo Pause):** si `m_options` está activo, MenuUp/MenuDown → `moveUp/moveDown` del OptionsState, MenuLeft/MenuRight → `adjustLeft/adjustRight`, Confirm → `confirmOptionsCommand()`, Cancel → cierra submenu y vuelve al menú de pausa (no cierra la pausa). Se mantiene la sincronización del widget de pausa (index).
3. **`confirmPauseCommand`:** el caso `PauseAction::Options` (antes `TODO G-10`) ahora crea `m_options = std::make_unique<Game::OptionsState>(Game::OptionsState::create())`.
4. **`confirmOptionsCommand()` nueva:** `OptionsAction::Back` → cierra submenu, devuelve la selección al menú de pausa; `ShowControls` → hint «Controles: pendiente de su pantalla (H1-03)»; `None` → nada (volumen/idioma se ajustan con izq/der, no con confirmar).
5. **`OptionsState.cpp` a `motor_level`** (CMakeLists, cruce autorizado). El bridge GL-free H1-05 (`OptionsLocaleBridgeTests.cmake`) lo compilaba directo a propósito (patrón G-14 anticolisión); ya no — el símbolo viene de `motor_level`, o habría duplicado al enlazar `test_options_locale_bridge` con `motor_level`.
6. **Test `cableado_opciones_se_abre_y_vuelve`** (title → pause → row 5 Opcions → confirmar → submenu con 6 entradas, fila música seleccionada, volumen 8 → 5×abajo → Volver → confirmOptions → vuelve al menú de pausa) + accesos de prueba (`options`, `hasOptions`, `pauseNavigateDown`, `confirmOptions`).

## Verificación

- `g++ -fsyntax-only` con los include reales del build: `src/Engine/Application.cpp` y `tests/Engine/ApplicationCableadoTests.cpp` **sin errores**.
- **Enlazado y ejecutado en el árbol (27/09, al disolverse el bloqueo):** `test_application_cableado` **PASSED (2,07 s)** — incluye el caso `cableado_opciones_se_abre_y_vuelve`. Además `test_options_state` PASSED, `test_options_locale_bridge` PASSED y `test_g10_pause_and_hud` PASSED. El WIP ajeno roto de `src/Project/ProjectIndex.cpp` (`.error()` sin mensaje) se resolvió cuando su dueño versionó el corte; `motor_project` vuelve a compilar y el enlace completo es posible.
- **Cruce detectado con el commit de Oriol `e096b0a` «H1-03 (cierre): test_options_state desacoplado de motor_core»:** ese commit versionó `Application.h` (con `m_options`/`confirmOptionsCommand`), `InputMap.h`/`InputSystem.cpp`, el `cmake/ApplicationCableadoTests.cmake` y el test `cableado_opciones_se_abre_y_vuelve` — pero **no** `Application.cpp`. La implementación del wiring quedó (y sigue) en el árbol como diff no commiteado; con ella el enlace y los tests pasan. A quien consolide: `Application.cpp` (wiring H1-03) + `cmake/OptionsLocaleBridgeTests.cmake` (quitar `OptionsState.cpp` de la compilación directa) son los dos ficheros que faltan por versionar para que esta rodaja quede en HEAD íntegra. **Atención porque en `Application.cpp` conviven hunks ajenos E-03/E-04 (intérprete de eventos) sin commitear en regiones distintas: hay que separar pathspec.**

## Lo que queda (registrado, no hecho)

- Versionar el wiring de `Application.cpp` (H1-03) + `OptionsLocaleBridgeTests.cmake`, separando los hunks ajenos E-03/E-04 (sin luz verde de commit para la zona H; solo cruce CMakeLists autorizado por Oriol).
- El tramo GL de la pantalla de Opciones (volumen/idioma/controles visibles) es de otro paquete (HUD/H6-04), no de esta rodaja GL-free.
- **Punto de atención multi-agente:** `Application.cpp/.h` tienen además hunks E-03/E-04 ajenos en el árbol (intérprete de eventos). Mis cambios conviven en regiones distintas; si otro agente recompone o rebasa, hay que revisar que no se pisen.