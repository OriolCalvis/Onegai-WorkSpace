# sesión 2026-09-27 opencode — H1-05: aplicar el idioma de Opciones al Locale (cableado)

## Zona tocada
- `MotorGraphico/src/Engine/Application.cpp` — carga de `assets/i18n` en `init()` (best-effort, `m_locale`), cableado del submenu de Opciones: `adjustOptionsLeft/Right()` llaman a `applyOptionsLanguage()` al pulsar Izq/Der.
- `MotorGraphico/include/Engine/Application.h` — miembro colaborador `std::unique_ptr<Engine::Locale> m_locale` (justificación N3) + `#include "Engine/Locale.h"` + helpers `applyOptionsLanguage()/adjustOptionsLeft()/adjustOptionsRight()`.
- `MotorGraphico/tests/Engine/ApplicationCableadoTests.cpp` — caso nuevo `cableado_idioma_se_aplica_a_locale` (titulo→pausa→Opcions→fila IDIOMA→Der→`locale.language()!=inicial`→Izq→vuelve) + accessor `locale()` y `adjustOptions*()`.
- `MotorGraphico/cmake/ApplicationCableadoTests.cmake` — G-14 directo: `src/Engine/Locale.cpp` + `src/Game/OptionsLocaleBridge.cpp` compilados en el target (los llama `motor_core`); NO se toca el `CMakeLists.txt` (cruce con WIP ajeno).

## Estado
- Implementación completa. Enlazado+ejecución **bloqueados por causas ajenas** (ver incidente abajo). El caso se modela igual que `cableado_opciones_se_abre_y_vuelve` (H1-03), que esta mañana SÍ estuvo VERDE.

## Incidencia multi-agente (importante)
- A las 15:49 `test_application_cableado` enlazó y pasó (H1-03 verde) porque el árbol completo era coherente.
- A las 15:56 la sesión ajena H3-04 (`06f86f7`/`7b1deb3` + `reset: moving to HEAD` ×2) **partió el árbol**:
  - Barrió del working tree las **definiciones** del WIP ajeno de EST-20: `BattleState::tickRound` (en `include/Game/BattleState.h` y `src/Game/BattleState.cpp`) y `publishSheetSkills` (en `include/Game/BattleMenuActions.h` y `src/Game/BattleMenuActions.cpp`). HEAD/SWTODAY no las definen pero `Application.cpp` las usa → `motor_core` no compila.
  - Barrió también MI wiring H1-03/H1-05 sin commitear de `Application.cpp/.h` y del test.
- El WIP barrido vive en `stash@{0}` (16:00). **Recuperé solo mis tres ficheros** desde el stash (`git checkout stash@{0} -- Application.cpp/h + ApplicationCableadoTests.cpp`), que restaura el estado coherente del árbol (mis hunks + hunks ajenos E-03/E-04 que ya estaban en el árbol esta mañana).
- Las **definiciones ajenas de `tickRound`/`publishSheetSkills` siguen guardadas en `stash@{0}`** (BattleState.h/.cpp, BattleMenuActions.h/.cpp). Son de otro agente; no las toco. Su dueño debería reincorporarlas (o commitealas) para que `motor_core` vuelva a compilar. Advertencia: el stash es amplio (de otro o de varios) — extraer SOLO esos 4 ficheros, no todo.

## Verificación
- `test_options_locale_bridge` (bridge de Claude, semántica de `applyLanguage` con tablas reales): **VERDE**.
- `test_options_state` y `test_g10_pause_and_hud` (H1-03): **VERDE** esta mañana.
- `test_application_cableado` (mi caso H1-05 + H1-03): bloqueado por el incidente ajeno (arriba), no por mi código. `-fsyntax-only` de mis ficheros sin errores propios.

## Punto de atención
- `Application.cpp` conviven mis hunks H1-03/H1-05 con los **ajenos E-03/E-04** (intérprete de eventos) y con usos de `publishSheetSkills`/`tickRound` (WIP de EST-20). No hay luz verde de commit para la zona H; quien consolide debe separar pathspec.
- Cuando el dueño de EST-20 reponga `tickRound`/`publishSheetSkills` (o se versione su corte), `test_application_cableado` debería enlazar con el G-14 de este cmake.
## Actualización 28/09 (segundo barrido + VERDE del caso H1-05)
- **Enlace restablecido:** las definiciones `tickRound`/`publishSheetSkills` ya están en el árbol (WIP EST-20 aterrizó, ficheros M) y `motor_core` compila con ellas.
- **EL wiring H1-03/H1-05 fue barrido OTRA VEZ** (2º barrido) del working tree de `Application.cpp/.h` (HEAD `4fc8c00`→`7da10a0`→`9363aa8`; solo quedaban hunks ajenos TIENDA-01/G-12/NUEVA-PARTIDA). Reaplicado a mano de `stash@{0}` en la sesión de 28/09: cabecera (`m_locale` N3 + `applyOptionsLanguage`/`adjustOptionsLeft/Right`) y cpp (carga i18n en `init()`, bloque `if (m_options)` en el modo Pause, ramo Options de `confirmPauseCommand`, 4 funciones), balance de llaves 0/0.
- **Test reescrito a invariante real** (el test original asumía que el idioma inicial del Locale era `ca`, pero el manifiesto i18n pone `idioma_de_referencia: "es"` → el Der `CATALA→CASTELLA`=es no produce cambio visible con el Locale ya en `es`). Ahora verifica mutable-invariant: tras CADA ajuste Der/Izq, `locale.language() == OptionsLocaleBridge::localeCode(options.language())`, recorriendo el ciclo completo de idiomas. Diag temporal (`DiagMode.cpp`) confirmó el wiring antes de borrarse.
- **`cableado_idioma_se_aplica_a_locale` VERDE** en build limpio `out/build/opencode-h105` (cmake G-14 presente: `Locale.cpp`+`OptionsLocaleBridge.cpp` en target).
- **Rojo ajeno documentado, NO tocado:** `test_application_cableado` da 8 fallos ajenos — EST-23b (extracción `SessionData`, f128e4a/9363aa8) dejó `session.mode()==GameOver` tras `init()` en el build actual (diag confirmado: `hp=100 maxHp=100 VIVO=1` pero `mode=GameOver`), rompiendo pausa/H1-03/cancelar/f5/equipo/título en cadena. No enmendarlo: es trabajo de Claude/EST-23b sin commitear.
- Nota git: un `git stash`/`pop` mío momentáneo para comparar (inmediatamente restaurado) demostró que el árbol porta ~43 ficheros de WIP ajeno sin commitear (4437 inserciones). No consolidar nada sin separar pathspec.
