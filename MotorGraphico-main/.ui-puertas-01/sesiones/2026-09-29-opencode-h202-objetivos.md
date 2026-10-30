# 2026-09-29 · opencode · H2-02 (Gantt Fase H): los objetivos se signalan

**Encargo de Oriol (standing):** «coge el gantt del motor graphico y sigue
programando, utiliza mapadelcodigo, pero piensa que hay mas agentes
trabajando en el proyecto de manera simultanea».

Del Gantt (`MotorGraphico/planificacion/gantt_jugabilidad_H.puml`) cogi
**H2-02 — objetivos visibles**, por cuatro razones medibles:

1. H1-01 (G-13 a produccion, la fila que abrio el menu de combate) dejo las
   tres funciones **puras** en `Game/BattleMenuActions.h`:
   `selectTargetsFor()` dice a quien puede apuntar una entrada segun alcance y
   bandol, y `translateToBattleAction()` la traduce a `BattleAction`. Lo que
   **no existia** era la SELECCION: con que dedo se senala, como se ve el
   alcance y el bandol, y como se vuelve atras. El jugador tenia una lista de
   objetivos en la que no se puede mover, y la unica forma de gastar PM era
   acertar a la primera.
2. Es el criterio de cierre mas corto de toda la fase H y el mas medible:
   «al elegir un objetivo se ve a quien se apunta, con que alcance y de que
   bando es, y se puede volver atras sin gastar nada».
3. Cabe en ficheros **NUEVOS** al 100 %: los unicos ficheros de cruce son
   `Application*` y `BattleMenuState*`, y estan con dueno (H1-03/H1-05,
   AV1-03, FICHA-01, CAPTURAS-UI). Las 2 lineas de `CMakeLists.txt` salen
   por hunk-staging.
4. H2-01 la tomo yo mismo tres horas antes; repetir el patron (pieza GL-free
   probada + montaje documentado) es lo que el Gantt pide para el tramo.

Repo `MotorGraphico-main`, rama `feature/editor-s1-contratos`.

| Ruta | Que |
| --- | --- |
| `include/Game/TargetSelection.h` | NUEVO — la pieza GL-free (namespace `Game`, patron `OptionsState`/`HudTimer`/`BattleMenuState`), 5 miembros |
| `src/Game/TargetSelection.cpp` | NUEVO — 1 linea en `motor_level` + montaje pendiente documentado |
| `tests/Game/TargetSelectionTests.cpp` | NUEVO — 15 casos, 3 de ellos con `BattleState::resolveAllyAction` real |
| `cmake/TargetSelectionTests.cmake` | NUEVO — patron `BattleMenuStateTests` sin `copiar_assets` (sin assets) |
| `CMakeLists.txt` | +2 hunk (fuente + `include` del test, mismo commit que su `.cmake`) |
| `planificacion/seguimiento/H2-02-OBJETIVOS.json` | NUEVO — ficha de la tarea |

## 1. La pieza y sus decisiones

`TargetSelection` es el estado GL-free de la pantalla «a quien apunto»:
`open(entry, allies, enemies, casterAllyIndex)`, `moveUp()`/`moveDown()` con
wraparound de `MenuLogic` (el de produccion), `cancel()`, `confirm()`,
`displayLines()`, `bandLine()` y las tres tablas de etiquetas
(`rangeLabel`/`bandLabel`/`sideLabel`) publicas y estaticas para que el HUD y
las pruebas hablen el mismo idioma.

Cuatro decisiones, y el porque de cada una:

1. **No reimplementa las reglas de G-13.** `open()` llama a
   `selectTargetsFor` y `confirm()` a `translateToBattleAction`. Una sola
   regla de «a quien puedo apuntar» y una sola de «esto es una accion
   valida»: si esta clase decidiera por su cuenta, el menu y la pantalla
   podrian discrepar justo en el caso que mas caro sale, gastar PM en una
   accion que no se puede hacer.
2. **`confirm()` DEVUELVE la `BattleAction`; no la aplica.** El gasto es de
   `BattleState::resolveAllyAction`, que es quien cobra los PM. Por eso
   cancelar aqui **no puede costar nada**: no hay nada que deshacer porque
   todavia no se ha gastado. La prueba lo comprueba contra el `SkillSet`
   real (`mana()` antes y despues), no con un mock.
3. **Una entrada bloqueada no abre la pantalla.** El menu ya ensena el
   motivo (`battleMenuRows`); abrir un cursor para algo que no se puede
   hacer es invitar a confirmar y leer un error.
4. **Los caidos SE VEN y NO se confirman.** Es el contrato que G-13 cerro
   («los muertos salen `alive=false` para que la UI pueda ensenarlos en gris
   y rechazarlos al traducir»): ver quien esta en pie es informacion, y
   confirmar uno caido devuelve el motivo en vez de fallar en silencio. No
   se inventan aqui reglas de salto de cursor: si hay que saltar a los
   vivos, se decide en el HUD y se escribe aqui, no al reves.

En un hechizo de area se **marcan** los afectados y se elige igual: el area
la resuelve `translateToBattleAction` (fija el indice), no el dedo del
jugador. Marcar es informar, no decidir.

## 2. Verificacion

- `test_target_selection` **15/15 PASS**; `ctest -R '^test_target_selection$'`
  Passed.
- **Falsabilidad por mutacion 6/6 cazadas** (restaurado y reverificado
  15/15): `open()` ignora la entrada bloqueada (1 rojo), `open()` no reinicia
  el cursor (1), `isAffected` marca a cualquiera (1), `confirm()` no devuelve
  el motivo (1), `cancel()` deja la seleccion abierta (1), `open()` no filtra
  la banda vacia (1).
- **La primera version de la suite NO era falsable en la marca de area**, y
  eso fue un fallo de diseno mio, no del test: la fila era
  `affectsWholeBand() && isAffected(side)`, dos guardas que se tapaban la una
  a la otra, asi que ninguna mutacion de una mitad se notaba. Se elimino
  `affectsWholeBand` y quedo una sola funcion que responde «este objetivo lo
  alcanza esta entrada?». Se deja escrito porque es el tipo de fallo que
  pasa por «todo verde» si no se busca.
- Vecinos: `ctest -R '^(test_target_selection|test_battle_state|
  test_g13_combat_commands|test_battle_menu|test_inventory_menu|
  test_h104_new_game_flow|demo_battle|demo_battle_nd6|
  demo_narrativa_combat|demo_title_menu)$'` → **10/10 Passed**.
- `juego` compila y arranca sin ventana: 11004 frames, salida normal.
- `git archive HEAD` en `/tmp` → configurar + compilar
  `test_target_selection` + **15/15** en un clon limpio del commit: el par
  `.cmake` + `include()` entra junto (ley 3) y nadie recibe un arbol roto.

## 3. Coexistencia: lo que NO toque y lo que encontre

- **Zonas respetadas:** `Application.*` (H1-03/H1-05 + AV1-03 + FICHA-01 +
  CAPTURAS-UI), `GameSession*` (H2-05/H2-08), `BattleState.*` /
  `BattleMenuActions.*` / `MenuLogic.h` (**solo se consumen**), `HUD/Iconos.*`,
  y las zonas H2-01/H3-01/H3-03/H4-01/G11/E-12. `grep TargetSelection` solo
  da mis 5 ficheros: sin colision de nombres.
- `MAPA_DEL_CODIGO.md`, `alcance_real.json` y `dades.py` intactos: reservadas
  por ALPHA-00, H8-01 y D0007 respectivamente.
- **Trampa de ambito (mordida de verdad):** `BattleMenuEntry`, `TargetOption`
  y `TargetSide` estan en el **ambito global** — H1-01 los escribio sin
  `namespace`. `Game::TargetSelection` los usa sin qualificar y el test **no
  hace `using Game::`**, que es lo que el compilador recuerda a los dos.
- **Lo que me entro en el commit sin pedirlo:** el indice git de este repo es
  **compartido** entre sesiones. Otro agente (que estaba arreglando
  `medir_alcance.py`, commit `5504b63`) hizo `commit` con el indice tal como
  estaba y **se llevo mis 5 ficheros** — los 4 nuevos y mi hunk de
  `CMakeLists.txt` — bajo un mensaje que habla del instrumento. Nada se
  perdio y el clon limpio de `HEAD` configura y pasa 15/15, pero la atribucion
  es incorrecta y la dejo escrita aqui y en la ficha en vez de disimularla.
  Leccion: con indice compartido, el `commit` de cualquiera barre lo que otro
  dejo staged; hay que **des-stagear lo ajeno justo antes** de commitear
  (eso si se hizo: `alcance_real.json` y `medir_alcance.py` se desataron y se
  volvieron a dejar staged) y **no dejar el trabajo propio a medias staged**,
  que es exactamente lo que paso.

## 4. Lo que queda de H2-02: el montaje (3 ficheros con dueno)

- Dueño del menu de combate en `Application` (`battleMenuRows` y el consumo
  de la entrada) + `BattleMenuState`. Los tres con WIP ajeno.
- Punto de montaje minimo: 1 miembro `Game::TargetSelection` en el objeto que
  ya guarda la eleccion de menu; `open(entry, bandos, indiceAliado)` al
  elegir la entrada; `confirm()` al Aceptar, devolviendo la `BattleAction` a
  `BattleState::resolveAllyAction`; `cancel()` al Cancelar; y `displayLines()`
  pintado con el mismo widget de la lista del menu (`HudCommandMenu`, el que
  ya usa H2-01).
- **Deuda que deja (ley 4):** `include/Game/TargetSelection.h` y
  `src/Game/TargetSelection.cpp` todavia **no los alcanza** ningun ejecutable;
  `linea_base_core` los marca como DEUDA NUEVA. Es el mismo estado en que
  entro H2-01 con `DiceRollWidget` y las 21 clases muertas de la linea base
  (`InventoryMenuState`, `SaveSlotState`, `TitleMenuState`...). Se arregla
  con el montaje de arriba, **no** actualizando la linea base: su
  re-congelado es trabajo de D0007/EST-42 y no de esta sesion.
- El veredicto de la guardia sobre el **arbol** da 35 de deuda nueva, 2
  mias y 33 de WIP ajeno sin commitear. Con el `HEAD` actual el instrumento
  ya sigue el cierre transitivo de cabeceras (`5504b63`), asi que el
  veredicto sobre mis dos ficheros es fiable y no un defecto de la medida.
