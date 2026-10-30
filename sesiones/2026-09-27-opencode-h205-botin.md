# H2-05 (Gantt Fase H) — el botín de ganar un combate llega a la mochila

- **Fecha:** 27/09/2026 (la sesión empezó el 26/09 y cruzó medianoche; el reloj del árbol va por 27/09 05:00)
- **Autor:** opencode
- **Fila del TABLERO:** reclamada antes de tocar nada, «En curso» → movida a «Hecho» al cerrar
- **Zona tocada:** `src/Game/GameSessionInteraccion.cpp`, `include/Game/GameSession.h` (una declaración), `tests/Game/GameSessionTests.cpp` (2 casos), `planificacion/seguimiento/H2-05-BOTIN.json`
- **Zona NO tocada:** `Application.cpp` / `Application.h` (cero miembros nuevos), CMake, el WIP ajeno de `src/Project/`

## Qué pedía el Gantt

`JUGABILIDAD_FASE_H.md` H2-05 (1,5 j, depende de H0-01 que cerré el 23/09):

> Botín al ganar. Ya existe: `RPG::LootResolver` (21 líneas, muerto); `loot_tables.json` existe y el juego no lo lee. Hay que: cablear al final del combate y al registro de objetos. Cierre: **ganar un combate da objetos de su tabla; salen en el inventario y se pueden equipar o vender**.

## Lo que encontré al medir el dato (esto cambió el diseño)

1. **El botín ya estaba escrito.** `monsters.json` tiene 432 entradas y **las 432** traen `loot` con ids de pieza reales (28 distintos: `armadura_de_cuero`, `daga_de_milicia`, `pocion_de_vigor`…). `CatalogLoader<MonsterDefinition>` los mete en `MonsterDefinition::lootIds` (acepta `loot` y `lootIds`; D-03 lo dejó así en su día). No había que escribir datos: había que leerlos.
2. **Son ids de EQUIPO, no de objeto.** 27 de los 28 están en `equipment.json` (1 en `consumables.json`), ninguno en un catálogo de objetos. La mochila guarda ids de objeto. El juego **ya tiene** el camino de vuelta, `GameSession::objectIdForEquipment()`: si el catálogo de objetos tiene `item_<x>`, ese es el id; si no, el propio id. Es justo lo que usan equipar y vender, así que la pieza que cae se puede equipar y vender sin trabajo nuevo.
3. **`LootResolver` y `loot_tables.json` no tienen ruta al combate.** Las 201 tablas se referencian desde `EncounterTableDefinition::lootTableId`, y las tablas de encuentro no enrutan al combate todavía; **ningún monstruo referencia una tabla**. Cablear el resolver aquí sería inventar una ruta de datos que el canon no declara. Registrado como pendiente de enrutado (§ «Lo que queda»), no dado por cerrado.

## La solución

`GameSession::repartirBotin(objectId)` — un solo sitio, en el `GameSession` que ya tiene `m_monsterCatalog` (D-08) y `m_inventory` inyectados. Por eso **no hubo que tocar `Application`**: cero miembros nuevos, cero cruces con la zona en vuelo de otros agentes, y la ley de ficherosmembers queda intacta.

El enganche es el caso `BattleOutcome::Victory` de `syncBattleOutcome()`: el id del enemigo se copia **antes** de `m_world->destroy(...)` y el reparto va justo después de «Has derrotado al X.». Huir y perder siguen su camino sin botín.

```cpp
case BattleOutcome::Victory:
    m_world->destroy(m_battleEnemyHandle);
    logLine("Has derrotado al " + enemyName + ".");
    repartirBotin(enemyObjectId);   // H2-05
```

El log nombra las piezas con `EquipmentDefinition::name` cuando hay catálogo de equipo inyectado (H0-04) y con el id si no — mismo criterio que «Te equipas X.» y que el resto del log de la sesión (sin tildes, como todas las líneas existentes).

## Verificación

| Target | Resultado |
|---|---|
| `test_game_session` | **22/22** (2 casos nuevos) |
| `test_save_game` | 19/19 |
| `test_battle_state` | 17/17 |
| `test_inventory_engine` | 7/7 |
| `test_inventory_menu` | 9/9 |
| `test_g13_combat_commands` | 8/8 |
| `test_application_cableado` | 10/10 |
| `juego` | enlaza; `nm juego` → `GameSession::repartirBotin` dentro (regla 4: lo alcanza un ejecutable real) |
| `git diff --check` | limpio |
| `linea_base_core --rapido` | **OK** (al cierre; los 4 fallos intermedios eran del troceo ajeno de `src/Project/`, ya resuelto por su dueño) |

**Falsabilidad comprobada, no de palabra:** desactivada la llamada a `repartirBotin`, el caso `al_ganar_el_botin_del_monstruo_entra_en_la_mochila` falla en `GameSessionTests.cpp:1126` (`f.inventory.size() == 2`); restaurada la llamada, vuelve a verde. El segundo caso (`huir_no_suelta_botin_y_el_enemigo_sigue_vivo`) es la guarda del otro lado: sin él, repartir al cerrar el combate dejaría caer el botín de un enemigo que sigue vivo en su celda.

## Lo que queda (no es de este item)

- **`LootResolver` sigue muerto** hasta que las tablas de encuentro lleguen al combate. El sitio de la tabla es la misma llamada a `repartirBotin` (`resolve_loot_table` + `objectIdForEquipment`), no un camino nuevo. Apuntado en el TABLERO y en la ficha `H2-05-BOTIN.json`.
- Ningún monstruo da oro: su dato es una lista de piezas. Si el canon quiere oro al ganar, es un item de contenido aguas arriba, no de este.

## Bloqueos ajenos que NO he tocado (regla 3)

- **Troceo ajeno a medias de `src/Project/` — se resolvió solo a mitad de sesión.** Durante un rato `ProjectIndex.cpp:514` no compilaba (`dstPath` sin declarar) y `ProjectIndexDuplicate.cpp` duplicaba `createNamed`/`duplicate`/`suggestId`/`uniqueId` (4 símbolos): `linea_base_core --rapido` daba 4 fallos, todos de ese troceo. Para no meterle mano (regla 3) enlacé con `make -o` sobre ese objeto usando el `.a` ya construido. **Unos minutos después su dueño lo dejó arreglado**: la guardia rápida pasó a **OK** y la verificación final (`test_game_session` 22/22 + `juego`) se hizo **en limpio, sin el apaño**.
- **`tests/Game/ShopFlowTests 2.cpp`** (untracked): otro duplicado `* 2*`, el mismo patrón que `G13CombatCommandsTests 2.cmake`.

## Coedición

- `GameSession.h`, `GameSessionInteraccion.cpp` y `GameSessionTests.cpp` son zona H desde EST-23 (los partí yo el 23/09), así que no hubo pisarse con nadie. La H1-01 de esta misma fecha sigue intacta: los 8/8 de `test_g13_combat_commands` son los mismos.
- `MAPA_DEL_CODIGO.md` **no** se regeneró: en árbol compartido recogería WIP ajeno (mismo criterio que la ficha de Codex H8-05).

**Sin commit** (la zona H sigue sin luz verde; el commit lo hace quien la consolidate, separando hunks).
