# Medida del combate antiguo (AV13-35)

Claude, 14/10/2026. Oriol (D0015 §11): «borrar el combate y las pruebas antiguas, después de medir que
nadie los usa».

**El combate antiguo** es el camino de `BattleState` **sin mesa de cartas**:
- **el héroe:** el ataque básico Nd6 y las habilidades con maná;
- **el enemigo:** la IA de «la primera habilidad que pueda pagar»;
- **el menú de habilidades:** `BattleMenuActions`, `BattleMenuState`, `TargetSelection`, y la barra
  que las enseñaba como cartas.

## 1. ¿Quién llega al camino sin mesa?

| Quién | ¿Llega? | Por qué |
|---|---|---|
| El juego, con un personaje creado o cargado | **No** | Las 127 clases tienen cartas en `skills.json` (medido con un script): `CartasEnCombate::paraHeroe` siempre da mesa. |
| La prueba de nivel del editor (`--nivel`, F7) | **Sí** | El héroe no tiene ficha, `paraHeroe` devuelve `nullptr` y el combate es el antiguo. |
| `GameSession` sin catálogo de cartas | **Solo pruebas** | El juego siempre lo pone (`ApplicationNivel.cpp:232`). Las pruebas de `GameSession` no. |
| Con mesa: huir y usar un objeto | Sí, pero **no es lo antiguo** | La mesa devuelve `false` y lo resuelve `BattleState` (`Flee`, `resolveItemAction`). **Se queda.** |
| Con mesa: el turno del enemigo | **No** | `CartasEnCombate::resolverEnemigo` devuelve siempre `true`. |
| Con mesa: una habilidad que no está en la mano | **No**, desde la barra | La barra con mesa no enseña habilidades antiguas. |

## 2. Las piezas y quién las usa

| Pieza | Líneas | La usa | Qué se hace |
|---|---|---|---|
| `BattleState::resolveAllyAction`: Attack y UseSkill sin mesa | ~80 | prueba del editor, pruebas | **Borrar** |
| `BattleState::resuelveEnemigo`: IA sin mesa | ~60 | prueba del editor, pruebas | **Borrar** |
| `BattleState`: invocaciones pendientes (`takePendingSummons`) | ~15 | solo las habilidades antiguas | **Borrar** |
| `BattleMenuActions.{h,cpp}` | 299 | `vistaDeBatalla` sin mesa, `Application::confirmBattleCommand` sin mesa | **Borrar** |
| `BattleMenuState.{h,cpp}` | 155 | nadie en `src/` (solo `test_battle_menu`) | **Borrar** |
| `TargetSelection.{h,cpp}` | 370 | nadie en `src/` (solo sus pruebas) | **Borrar** |
| `vistaDeBatalla(…, legado)`: habilidades antiguas como cartas | ~25 | sin mesa | **Borrar** |
| `Application::confirmBattleCommand`: filas 3+ como habilidad antigua | ~20 | sin mesa | **Borrar** |
| `BattleState::tickRound` con `ConditionEngine` (veneno por ronda) | — | estados puestos por otros sistemas, y sus pruebas | **Se queda** (no es solo del combate antiguo) |
| `ApplySkillEffect` | — | la mesa (`CartasTiradas.cpp`) | **Se queda** |

## 3. Para que nadie se quede sin combate

- **Héroe sin ficha** (la prueba del editor): `paraHeroe` da igualmente una mesa, solo con las
  básicas (Atacar, Poción, Huir, Pasar).
- **`GameSession` sin catálogo** (las pruebas): la mesa se monta con un catálogo vacío.

## 4. Pruebas

| Prueba | Qué se hace |
|---|---|
| `test_battle_menu`, `test_target_selection` | Se borran con su código. |
| `test_battle_state` | Se borran los casos del ataque y las habilidades antiguas. Se quedan los de huir, los objetos y el tick de estados. |
| `test_g13_combat_commands` | Se borran los casos del menú antiguo. Se quedan los AV13/AV14 (el tablero, el alcance, los golpes). |
| Las demás | Se ajustan si esperaban el texto del combate antiguo. |
