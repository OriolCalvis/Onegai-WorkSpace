# 2026-10-01 · opencode · H2-03 (Gantt Fase H): condiciones en el HUD + H0-06

**Encargo standing de Oriol:** Gantt + MAPA_DEL_CODIGO bajo coedicion
(continuacion de H2-01, sesion `2026-09-29-opencode-h201-dado.md`).

Elegida **H2-03 — condiciones en el HUD** (2 j, verde, depende de H0-08 y
H0-06): su zona estaba limpia y sin filas vivas (`grep ConditionWidget`
= 0; ningun widget del HUD ensena condiciones hoy — verificado en
`HudWidgets.h`/`HudTextWidgets.h`/`HudTimer.h`, que no nombran
"condition"), y es legibilidad de combate nuclear. De propina, su
dependencia **H0-06** era el Pendiente «HEAD roto» asignado a opencode:
`Application.cpp` llamaba `BattleState::tickRound` sin implementacion
versionada y `motor_core` no compilaba en clon limpio.

Repo `MotorGraphico-main`, rama `feature/editor-s1-contratos`.

| Ruta | Que |
| --- | --- |
| `BattleState.h` / `BattleState.cpp` | versionado H0-06 (+13/+46, `tickRound`) — acabo en `8b338a4` |
| `include/HUD/ConditionWidget.h` | NUEVO — pieza GL-free (patron `DiceRollWidget`) |
| `src/HUD/ConditionWidget.cpp` | NUEVO — 1 linea en `motor_level` + montaje documentado |
| `tests/Game/ConditionWidgetTests.cpp` | NUEVO — 11 casos |
| `cmake/ConditionWidgetTests.cmake` | NUEVO (versionado en `67581de` SIN su `include`: ver §3) |
| `CMakeLists.txt` | +2 lineas EN EL ARBOL SIN COMMITTER (ver §3, texto literal abajo) |
| `planificacion/seguimiento/H2-03-CONDICIONES.json` | NUEVO — ficha |

## 1. H0-06: la cura del HEAD roto y la carrera que la acompano

El diff del arbol en `BattleState.h/.cpp` era **adicion pura (+59/-0)** del
WIP H0-06 del 24/09 (misma sesion `2026-09-24-opencode-h006-tick-round.md`;
cero lineas ajenas en esos ficheros) y todos los ayudantes ya existen en
HEAD (`logLine`, `notifyDamageReceived`, `checkOutcome`,
`participantAlive`, `Fled`, `m_conditions`). Versionarlo era literalmente
lo que pedia el Pendiente.

Lo que paso despues son **dos trampas nuevas** (propuestas para la Parte 3):

1. **Mi commit `78cbc84` barrió 3 ficheros ajenos** (`BattleMenuActions.h`
   +19, `BattleMenuActions.cpp` +36, `ApplicationNivel.cpp` +13): entre mi
   `git add` (2 ficheros) y mi `git commit` (sin pathspec), otra sesion
   stagéo lo suyo y mi commit de indice completo se lo llevo. Detectado
   porque verifico el stat despues de cada commit (el commit decia «5
   files» y yo habia stagéado 2). Reparado con `reset HEAD~1` mixto
   (disco intacto, su WIP volvio a `M`) — contenido preservado en todo
   momento, nada que lamentar.
2. **La carrera espejo**: al re-stagéar y reintentar, el commit fallo con
   «no changes added» — la sesion concurrente habia commiteado el mismo
   contenido (`8b338a4`, mensaje «Fix: commit BattleState::tickRound...»).
   Contenido byte-identico al WIP H0-06; objetivo cumplido por quien
   fuera, sin duplicar trabajo.

**Verificacion del HEAD curado** (worktree limpio, luego borrado): a
`0476bb6` (= `8b338a4` tickRound + `0476bb6` «H1-01 segunda mitad» de otra
sesion, que versiono `publishSheetSkills` — el segundo simbolo que
`Application.cpp:867` pedia) compilan `motor_core`, `test_battle_state`,
`test_g10_pause_and_hud` y `juego`; `test_battle_state` 14/14 y
`test_g10` 5/5 en verde. El Pendiente 27/09 queda curado a dos bandas.

**Leccion (propuesta de norma): en este repo, NUNCA `git commit` sin
pathspec, y SIEMPRE `git show HEAD --stat` despues.** El lock de
`git-safe.sh` cubre UN comando; el hueco add→commit es de cualquiera.
El stat post-commit es la red que cazo las dos anomalias de hoy.

## 2. H2-03: la pieza

`Game::ConditionWidget::show(etiqueta, active, catalog)` guarda una FOTO
(copia la lista y resuelve nombres contra el catalogo); `displayLines()`:

- `[0]` etiqueta del portador,
- `- Nombre: N rondas [xN] [(-dmg/ronda)]`, en el orden de la ficha,
- `UNTIL_CURED` pinta **«hasta curarse»** (pintar sus rondas mentiria —
  el tick no las descuenta),
- id desconocido usa el id (nunca revienta), sin condiciones pinta
  `(sin condiciones)`.

Decisiones escritas en la cabecera: **solo texto** (los iconos son de la
zona ARTE/FICHA en vuelo: `Iconos`, `HudFicha`, `HudTextLayoutTests` — no
duplicar, complementario) y **el widget no avanza rondas** (lee la ficha;
el tick es `tickRound`, H0-06).

**Verificacion:** `test_condition_widget` **11/11** (10 a mano + 1
integracion: `apply` + `tick_conditions` de verdad, 3→2 rondas);
**falsabilidad 3/3** (hasta-curarse miente: 1 rojo; marca de dano
incondicional: 1 rojo; show sin clear acumula: 2 rojos); suite GL-free
**187 tests: 176 pass**, 6 rojos reales atribuidos (`demo_creacion` +
`demo_nova` por `classes.json` M ajeno; `test_gdb12` por
`InventoryScreen` M de Codex; `test_catalog_content` por catalogos M
ajenos; 2 `linea_base_core` por `.cmake`/fuentes sin versionar ajenos) +
**5 Not Run ajenos** (`pergamino`, `quest_log`, `options_file`,
`personajes_capas`, `editor_apariencia`: sus binarios COMPILAN al
pedirlos —probado `test_options_file` y `test_quest_log`—, pero no
cuelgan de `suite_gl_free` en este arbol; detalle de cableado de sus
duenos, no se toca); `analizar_duplicados` 0 solapamiento;
`comprobar_fragmentos` solo rojo ajeno G15; `diff --check` limpio;
`juego`/`level_editor`/`motor_editor` compilan.

## 3. El commit por el calor: que entra, que queda y como reconstruirlo

El hook (`--indice`) rechaza commitear el `CMakeLists.txt` stagéado
mientras el arbol tenga refs ajenas sin versionar: hoy
Apariencia/Pergamino/Personajes(5)/Iconos/HudIconList/HudIconos/HojaPersonaje/
AuraJugador/PergaminoGL/PantallaNuevaPartida/Codice/Companeros/FichaVista
(+ los 4 tests de `FichaVistaTests.cmake`). No es mi deuda y el propio
hook dice que se documente: aqui esta.

- **Versionado en `67581de`** (pathspec verificado, exactamente 5
  ficheros): `.h`, `.cpp`, test, `.cmake` y seguimiento. El `.cmake` sin
  su `include` es inofensivo (nada lo referencia); el `include` entrara
  CON o DESPUES del `.cmake`, nunca antes (ley 3 a salvo).
- **Pendiente en el arbol** (reconstruccion literal: tras
  `src/HUD/DiceRollWidget.cpp` anadir comentario H2-03 +
  `src/HUD/ConditionWidget.cpp` en `motor_level`; tras
  `include(cmake/DiceRollWidgetTests.cmake)` anadir comentario H2-03 +
  `include(cmake/ConditionWidgetTests.cmake)`; las 5 lineas exactas estan
  en `MotorGraphico/CMakeLists.txt` del arbol y en el diff de esta
  sesion). Entraran juntas en un solo commit cuando el hook pase.

## 4. Coexistencia

- Aterrizaron durante la sesion (todo respetado, nada tocado):
  `3c52dda` (H2-02), `18bdc02` (H3-03), `5829895` (AV1-03), `64922a9`
  (H6-03), `f516f6b` (H3-02), `1ec457c` (H8-04), `5ecaab2` (save
  atomico), `0476bb6` (H1-01 2a mitad), `8b338a4` (H0-06 espejo).
- El TABLERO suma filas concurrentes (H2-02/AV1-03/AV0-01 de otro
  opencode): mi fila convivio con ellas; al cerrar, solo mi hunk.
- Queda de H2-03 (montaje, 3 ficheros sucios con dueno): miembro en
  `Application.h`, `show()` tras `battle->tickRound()`
  (`Application.cpp:847`) con el catalogo ya inyectado (H0-04), pintar
  en `ApplicationHud.cpp`. Mitad GL-free, como H2-01/H0-08/H3-01.
