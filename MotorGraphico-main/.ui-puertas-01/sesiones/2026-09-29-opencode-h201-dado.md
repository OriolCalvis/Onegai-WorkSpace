# 2026-09-29 · opencode · H2-01 (Gantt Fase H): el dado se ve

**Encargo de Oriol (standing):** «coge el gantt del motor graphico y sigue
programando, utiliza mapadelcodigo, pero piensa que hay mas agentes
trabajando en el proyecto de manera simultanea».

Del Gantt (`MotorGraphico/planificacion/gantt_jugabilidad_H.puml`) cogi
**H2-01 — el dado se ve** (2 j, verde, depende de H0-08, ya cerrada), por
cuatro razones medibles:

1. **H3-01 ya la cogio qwen hoy mismo** (su sesion
   `2026-09-29-qwen-h301-viaje-entre-naciones.md` documenta el mismo
   encargo literal): repetirla seria pisar su WIP (`WorldTravelState.*`
   sin versionar + fila viva en TABLERO).
2. Su zona esta **limpia y sin filas vivas**: `grep DiceRoll` en el arbol
   daba 0 resultados antes de empezar; nada de HUD/widgets aparece en las
   filas «En curso» del TABLERO; `include/HUD/` solo tiene WIP ajeno de
   otros nombres (`Iconos.*`, `HudFicha`, `HudIconos`, `HudTextLayoutTests`).
3. Es del camino critico del jugador: el motor resuelve de verdad
   (`DicePoolEngine::roll_pool` + `opposed_degree`, Ed. 3) pero **nadie lo
   ensena** — ni al hablar, ni al atacar, ni al usar una habilidad.
4. Cabe en ficheros NUEVOS al 100 %: ningun fichero de cruce (MAPA §1) se
   toca; la unica friccion son 2 lineas en `CMakeLists.txt`, aisladas por
   blob y commiteadas por hunk (ley 3) sin barrer el WIP ajeno.

Repo `MotorGraphico-main`, rama `feature/editor-s1-contratos`.

| Ruta | Que |
| --- | --- |
| `include/HUD/DiceRollWidget.h` | NUEVO — la pieza GL-free (namespace `Game`, patron `OptionsState`/`HudTimer`) |
| `src/HUD/DiceRollWidget.cpp` | NUEVO — 1 linea en `motor_level` + montaje pendiente documentado |
| `tests/Game/DiceRollWidgetTests.cpp` | NUEVO — 11 casos |
| `cmake/DiceRollWidgetTests.cmake` | NUEVO — patron `WorldTravelStateTests` sin `copiar_assets` |
| `CMakeLists.txt` | +2 lineas por hunk-staging (fuente + include, mismo commit que su `.cmake`) |
| `planificacion/seguimiento/H2-01-DADO.json` | NUEVO — ficha de la tarea |

## 1. La pieza y sus decisiones

`Game::DiceRollWidget::show(pool, degree, label)` guarda una FOTO del
resultado (copia los dados); `displayLines()` devuelve lo que pinta el HUD:

- `[0]` etiqueta (`"Ataque: tu tirada"`),
- `[1]` dados (`"Dados: [6] [4] [1]"`, o `"Dados: (sin dados)"` con pool vacio),
- `[2]` exitos (`"Exitos: 1.5"`, `%g`: el medio exito de Ed. 3 se ve),
- `[3]` grado (`"Grado: Èxit"`),
- `[4]` solo si hubo pifia: `"!! PIFIA !!"`.

Tres decisiones, escritas en la cabecera para que no se re-debatan:

1. **Las etiquetas son las del log de combate** (`BattleState.cpp`:
   `Fracàs/Parcial/Èxit/Crític`): una sola tabla, sin divergir. La pregunta
   catalan/castellano sigue en la bandeja de Oriol; cuando responda se
   cambian las dos a la vez, no este fichero solo.
2. **`result.botch` distingue la PIFIA del grado BOTCH**: un BOTCH por
   derrota en la enfrentada (el defensor supera) lleva `Grado: Fracàs` pero
   NO la marca. La marca distingue, no el grado — es lo que pide la ficha
   («dados, caras, grado, pifia» son cuatro cosas).
3. **No se tira de nuevo para mostrar**: el montaje usa el `PoolResult` ya
   calculado donde se decide el grado (`GameSession.cpp:380,393`,
   `syncBattleOutcome`). Sin esto habria dos tiradas distintas (la que
   decide y la que se ve).

## 2. Verificacion

- **`test_dice_roll_widget` 11/11 PASS** (0,00 s), `ctest -R` Passed con
  etiquetas `unit;gl-free`. 10 casos con `PoolResult` escrito a mano + 1 de
  integracion con el motor real (`Xoroshiro128p(1234)` → medido `[4,3,5]`,
  0,5 exitos → `SUCCESS`; valores fijados: si la aritmetica del motor
  cambia, el caso lo dice).
- **Falsabilidad por mutacion, 3/3 cazadas** (restaurado y reverificado
  11/11 despues de cada una): etiqueta `SUCCESS→Parcial` (2 rojos), marca
  de pifia desactivada (1 rojo), `Exitos: %d` (2 rojos).
- **Suite GL-free: 180 tests, 174 Passed, 6 Failed — los seis atribuidos
  con `git status`, ninguno mio**: `demo_creacion_personaje` +
  `demo_nova_partida` (abortan: `classes.json` M ajeno, 61 vs 63 — ya
  documentado por qwen ayer), `test_gdb12_screens` (`InventoryScreen` M,
  fila G12 de Codex), `test_catalog_content` (`classes.json`/`npcs.json` M
  ajenos), `linea_base_core_configura` + `linea_base_core` (rojos
  preexistentes por `.cmake`/fuentes sin versionar de 4+ sesiones; mis 3
  AVISOS son de la misma clase y se curan con este commit).
- **Guardas**: `analizar_duplicados --comprobar` OK (0
  SOLAPAMIENTO_FUNCIONAL); `comprobar_fragmentos_cmake` solo senala el rojo
  ajeno `G15ExportPanelTests` (el mio registrado); `diff --check` limpio.
- **Los 3 ejecutables compilan** (`juego`, `level_editor`, `motor_editor`,
  EXIT 0) con la fuente nueva en `motor_level`.

## 3. Coexistencia: lo que NO toque y lo que encontre

- Sin tocar (todos `M` en el arbol con filas vivas): `Application.*`
  (H1-03/H1-05 + MenuJugador), `GameSession*` (H2-05/H2-08),
  `BattleState.*`, `HUD/Iconos.*` + `HudFicha/HudIconList/HudIconos` +
  `HudTextLayoutTests` (WIP ARTE/FICHA/MISIONES de otras sesiones),
  zonas H3-01/H4-01/G11/E-12. `MAPA_DEL_CODIGO.md` no se regenera
  (`pla/dades.py` reservado por ALPHA-00) ni `alcance_real.json` (H8-01/D0007).
- **Durante la sesion aterrizaron 4 commits ajenos** (H4-01 `e02bc9c`,
  H2-08 `2576535`, G-11 `1aa4e7f`, H3-01 `4916d92`): el indice que a las
  08:36 tenia 7 staged ajenos (ver sesion de qwen) amanecio limpio, asi que
  el par `.cmake`+`include` (ley 3) SI pudo entrar en mi commit — por
  hunk-staging de `CMakeLists.txt`, verificando que `diff --cached` de ese
  fichero trae solo mis 2 lineas. El resto de mis ficheros son rutas
  nuevas: ningun orden de commit podia romper a nadie.
- **Trampa del `make` rancio**: tras restaurar la mutacion 3, el rebuild
  dijo `Built target` sin recompilar y el test siguio en rojo; `touch` +
  rebuild verboso (ver `DiceRollWidget.cpp.o` en la salida) lo curo. Misma
  familia que los `os.utime` de qwen: no fiarse del `Built target` sin ver
  el objeto.
- **Aviso para el siguiente**: `herramientas/git-safe.sh` aparece `M` en
  el repo raiz (otra sesion lo toco); no lo commiteo ni lo uso a ciegas —
  mi commit de MotorGraphico usa la version del disco tras leerla entera
  (solo envuelve `git` con lock, sin logica que afecte a mis paths).
- **Hallazgo sin dueno**: `cmake/HudTextLayoutTests.cmake` +
  `tests/Render/HudTextLayoutTests.cpp` (`??`, otra sesion, layout de
  texto del HUD) conviven con este widget sin colision (cadenas aqui,
  layout alli). Si su dueno lee esto: `displayLines()` ya devuelve las
  lineas listas para medir/pintar.

## 4. Lo que queda de H2-01: el montaje (3 ficheros sucios, con dueno)

La pieza esta decidida y probada; el jugador aun no la ve. Falta (detallado
en `src/HUD/DiceRollWidget.cpp`):

1. `include/Engine/Application.h` — `M` (H1-03/H1-05 + MenuJugador):
   miembro `Game::DiceRollWidget m_diceWidget`.
2. Donde se resuelve (`GameSession.cpp:380,393`, `syncBattleOutcome` en
   `GameSessionInteraccion.cpp` — `M` H2-05/H2-08):
   `m_diceWidget.show(pool, grade, etiqueta)` con el pool existente.
3. `src/Engine/ApplicationHud.cpp` — `M`: pintar `displayLines()` con
   `HudCommandMenu`; `hide()` al cambiar de modo; la linea `!! PIFIA !!`
   en color de aviso (el widget solo da texto).

Por eso la fila vuelve a «Hecho» como **mitad GL-free** (igual que H0-08 y
H3-01), no como tarea cerrada: el criterio de cierre («se ve en partida»)
necesita esos tres ficheros limpios.
