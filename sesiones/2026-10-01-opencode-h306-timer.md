# 2026-10-01 · opencode · H3-06 (Gantt Fase H): temporizadores visibles

**Encargo standing de Oriol:** Gantt + MAPA_DEL_CODIGO bajo coedicion
(continuacion de H2-03, sesion `2026-10-01-opencode-h203-condiciones.md`).

Elegida **H3-06 — temporizadores de mision visibles** (1,5 j, verde,
depende de H0-08): su zona estaba limpia y sin filas vivas (`grep
MissionTimerWidget` = 0; ningun widget ensena los timers narrativos) y el
reloj ya corre en produccion (`tickTimers` via `updateNarrativeTimers`,
F1) — faltaba solo lo visible, que es justo el hueco de mayor
rendimiento. Descartados por colision o por canon: H2-04-resto
(`GameSession*` con fila H2-05 viva y `M`), H2-07 (los 432 `maxHealth=0`
se deciden aguas arriba, no en el motor), H4-02/H4-03/H5 (zona
`Editor*` con E-12 viva + EST-21d reservada), H6-01 (cableado en
`Application` sucia), H7-02 (`ExportPanel` ajeno en vuelo).

Repo `MotorGraphico-main`, rama `feature/editor-s1-contratos`.

| Ruta | Que |
| --- | --- |
| `include/HUD/MissionTimerWidget.h` | NUEVO — pieza GL-free (patron `DiceRollWidget`) |
| `src/HUD/MissionTimerWidget.cpp` | NUEVO — 1 linea en `motor_level` + montaje documentado |
| `tests/Game/MissionTimerWidgetTests.cpp` | NUEVO — 11 casos |
| `cmake/MissionTimerWidgetTests.cmake` | NUEVO (versionado en `72f63bc` SIN su `include`: ver §2) |
| `CMakeLists.txt` | +2 lineas EN EL ARBOL SIN COMMITTER (texto literal en §2) |
| `planificacion/seguimiento/H3-06-TIMER.json` | NUEVO — ficha |

## 1. La pieza

`Game::MissionTimerWidget::show(states, definitions)` guarda una FOTO
(copia los `TimerState` y resuelve cada definicion por id);
`displayLines()` pinta en el orden de `NarrativeState::timers()`:

- Contador: `"huida: 1:30"` (M:SS siempre con dos digitos, **trunca**:
  lo visible nunca promete mas de lo que queda),
- Barra: `"huida: [###-------] 0:45"` (10 celdas, `floor(fraccion*10)`),
- **Secreto se salta entero** (decision 2.18), igual que inactivos y
  expirados; id sin definicion cae a Contador; Barra sin total
  (`duracion <= 0`) cae a Contador; vacio → vacio (lo normal: casi toda
  la partida no corre ningun timer).

Sin iconos (ARTE/FICHA en vuelo) y sin segundo reloj: el widget lee
`NarrativeState`, no avanza tiempo.

**Verificacion:** `test_mission_timer_widget` **11/11** (9 a mano + 2
integracion con `NarrativeEngine` de verdad: arranque por flag, avance
120→90, **congelacion con dt=0** y expiracion con `flagExpira`;
Barra 75/120→3 celdas); **falsabilidad 3/3** (secreto visible: 1 rojo;
redondeo: 1 rojo; barra +1: 2 rojos); vecinos **102/102** (condition 11,
dice 11, map_event_runner 21, event_interpreter 38, condition_engine 12,
narrative_variables 9); `analizar_duplicados` 0 solapamiento;
`comprobar_fragmentos` sin queja sobre el mio; `diff --check` limpio.

## 2. Calor y decisiones de commit (leer antes de reintentar el par)

- La suite GL-free completa **no construye en este arbol**: `motor_core`
  roto por WIP ajeno (`Application.h:381` usa `RPG::MapEventRunner` sin
  include; en HEAD no existe esa linea). Deuda de su dueno (zona
  H1-03/H1-05/MenuJugador), documentada y no tocada. Por el mismo roto,
  el alcance-ley-4 via `juego` queda unverificable hoy.
- El hook `--indice` rechaza el `CMakeLists.txt` stagéado por refs
  ajenas sin versionar (Apariencia, Pergamino, Personajes×5, Iconos,
  HudIconList, HudIconos, HojaPersonaje, AuraJugador, PergaminoGL,
  PantallaNuevaPartida, Codice, Companeros, FichaVista + 4 tests).
  Versionados los 5 ficheros nuevos en `72f63bc` (pathspec verificado,
  445 inserciones exactas); **las 2 lineas de CMakeLists quedan en el
  arbol** y entraran juntas (ley 3) cuando el calor baje — igual que las
  2 de H2-03, que siguen pendientes por el mismo motivo. Texto literal:
  tras `src/HUD/ConditionWidget.cpp` anadir comentario H3-06 +
  `src/HUD/MissionTimerWidget.cpp` en `motor_level`; tras
  `include(cmake/ConditionWidgetTests.cmake)` anadir comentario H3-06 +
  `include(cmake/MissionTimerWidgetTests.cmake)`.
- Racha de commits concurrentes respetada (H2-05 `40a28cd`, H0-04
  `71ba370, H3-02 `f516f6b`, H8-04, save-atomico, H1-01-2a-mitad,
  qwen-h804): el arbol trae regeneraciones de contenido (decenas de
  levels/TMX `M`) que no se tocan.

## 3. Queda (montaje, ficheros sucios con dueno)

Miembro en `Application.h`, `show()` cada frame en exploracion donde ya
corre el reloj (`ApplicationBucle.cpp:254`), pintar en
`ApplicationHud.cpp`, `hide()` sin aventura. Mitad GL-free, como
H2-01/H2-03/H0-08/H3-01.
