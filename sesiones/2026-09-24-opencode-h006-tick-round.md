# 24/09/2026 — H0-06 (Gantt Fase H): los estados alterados caducan (tick por ronda) + cierre del enlace GL

Rodaja H0-06 (JUGABILIDAD_FASE_H.md): `tick_conditions` consumido una vez por ronda de combate real.
Hallazgo en la fila H1-01 (h101): H0-06 ya estaba implementado en el árbol **sin fila ni commit** y,
además, `test_application_cableado`/`juego` **no enlazaban**. Esta sesión reclama H0-06, verifica el
DOT por ronda y cierra el enlace GL (cruce de CMakeLists, autorizado por Oriol).

## Qué cambia

### H0-06 — fin de sesión verificada (batalla real, una vez por ronda)

- `BattleState::tickRound()` (include/Game/BattleState.h, src/Game/BattleState.cpp): itera aliados y
  enemigos vivos con ficha + `m_conditions`, llama `ConditionEngine::tick_conditions`, acumula DOT
  (`damageInflicted`), aplica con `takeDamage`, loguea y notifica (`notifyDamageReceived`), y hace
  `checkOutcome` (para de procesar un bando cae por el propio DOT). No-op si `m_outcome != InProgress`
  o sin catálogo.
- Cableado en `Application::confirmBattleCommand`: si `battle->outcome() == InProgress`, tras
  `resolveEnemyTurn()` se llama `tickRound()` (ronda = acción aliado + turno enemigo). Zona H0, arc-ria
  recta: es el único punto por-ronda real (`resolveEnemyTurn` se llama solo ahí, Application.cpp:1610).
- Tests nuevos en `tests/Game/BattleStateTests.cpp` con fixture `dotConditionsCatalog()` (poison:
  defaultRounds=3, damagePerRound=4): `h0_06_el_veneno_hace_dano_por_ronda_y_caduca_solo`
  (50→46→42→38, la condición desaparece y el tick siguiente no daña), `h0_06_tick_hace_dano_en_ambos_
  bandos_y_loguea_la_caducidad` (verifica log «poison se ha agotado») y `h0_06_sin_catalogo_de_
  condiciones_el_tick_no_hace_nada`.

### Cierre del enlace GL (cruce CMakeLists autorizado)

`test_application_cableado` (y `juego`) no enlazaban: `Application.cpp` (zona H0/H1 sin commitear)
referencia `TitleMenuState` + `SaveSlots`, pero esos `.cpp` se compilaban solo sueltos en demos/tests
de otros. Resuelto respetando la ley 4c (crear librería, no añadir fuente directa):

- **Librería GL-free nueva `motor_saves`** = `src/Game/TitleMenuState.cpp` + `src/Game/SaveSlotState.cpp`,
  enlazada vía `motor_core` (juego, test de cableado). Los targets ajenos (test_title_flow,
  test_save_slots, demo_title_menu, demo_nova_partida) siguen con su patrón actual — no los toco.
- **`src/Game/SaveGameFormat.cpp` entró en `motor_level`**: su gemelo `SaveGame.cpp` ya estaba ahí,
  pero `SaveGameFormat` (parseSaveGame/serializeSaveGame/applyNarrativeState) nunca se cableó al
  enlazador (EST-29). Sin él no enlazaba NADA que tocara guardado — confirmado que `demo_nova_partida`
  tampoco enlazaba antes de este fix.

## Verificado

- `test_battle_state` **17/17** (14 previos + 3 nuevos h0_06_*).
- `test_application_cableado` **9/9** (incluido `cableado_campana_catalogo_desde_manifiesto`),
  `test_title_flow` 7/7, `test_save_slots` 13/13 (sanity: no tocados).
- `juego`, `demo_title_menu`, `demo_nova_partida`: enlazan y construyen.
- Guardia `linea_base_core`: **1 deuda nueva restante — `Application.cpp` +49 (2444→2493)**. No es de
  H0-06 (1 hunk de 2 líneas): es el acumulado sin commitear de mi zona H (H0-01/04/05/07, ver filas del
  20-23/09) + el cruce EST-24 que otros agentes tocan ahora mismo (EditorState). No lo troceo en
  solitario (relevo no anunciado sobre fichero activo ajeno, ley 5).

## Sin commit

Zona H sin luz verde (CMakeLists cruce) — todo queda en el árbol, sin commitear, igual que el resto de
mi zona H desde el 20/09. El cruce CMakeLists concretito (motor_saves + SaveGameFormat en motor_level)
sí fue autorizado por Oriol, pero no el commit.