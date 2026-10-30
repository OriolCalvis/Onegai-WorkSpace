# 11/09/2026 — D-05 loaders: equipment y consumibles leen sus campos

- **Gantt:** corte B, fase D. Continuación de mis rebanadas D-05 (motor + maxHealth).
- **Fila reclamada antes del primer fichero.** Sin commit a propósito (regla 2). Rama `feature/editor-s1-contratos`.

## Survey previo (decidió el alcance)

- `equipment.json` (413): el cargador leía `caBonus/maxHealthBonus/saves` que **ninguna carta trae** (`defenseBonuses{}` vacío en las 413); e ignoraba lo que SÍ traen: `linkedSkill` (33), `compatibilitySymbols` (13), `requiredStats` (108, p. ej. arco_corto DES 2), `penalties` (6, p. ej. armadura_de_placas DES 2). `restrictions` (305) y `mechanicTags` son prosa — se ignoran a propósito. `type` siempre `"equipment"`.
- `consumables.json` (31): el cargador leía 8 campos aspiracionales (`healHp`, `grantConditionId`…) que **0 cartas** traen; ignoraba `uses` (31/31, todos 1) y `actionType` (19 `accion_menor` / 9 `accion` / 3 `reaccion`).
- Tercer catálogo («objectes»): survey de `assets/objects/*.json` (5559 objetos) — claves 1:1 con `ObjectDefinition` salvo metadatos `_` — sano, nada que hacer. Los 33 `linkedSkill` resuelven todos en skills∪spells.

## Qué se ha hecho

- `RpgCoreDefinitions.h` (limpio, sin dueño): `EquipmentDefinition` += `requiredStats[4]` + `statPenalties[4]`; `ConsumableDefinition` += `uses = 1`, `actionType`, `flavorText`.
- `RpgCatalogs.h`: equipment += alias `linkedSkill`, fusión `compatibilitySymbols` (deduplicada), `requiredStats`, `penalties`; consumables += `uses`, `actionType`, `flavorText`. `expected_fields()` actualizado (unicidad verificada por contrato).
- `InventoryEngine.cpp` (mío): `can_equip()` exige `requiredStats` contra el total del PJ (mensaje con stat/valor/mínimo); `apply_bonus()` resta `statPenalties`.
- `CatalogContentTests.cpp`: spot arco_corto (linkedSkill + DES 2), placa (penalizador), fusión de símbolos (asset redundante + entrada sintética solo-mayúscula), sweeps (108 / 33 que resuelven / 6), aceite_de_filo + reparto 19/9/3.
- `InventoryEngineTests.cpp` (mío): `els_requisits_de_la_carta_es_compleixen_o_no_s_equipa` (DES 0 falla sin tocar slot; DES 2 equipa).

## Verificación (build limpio en temporal)

- `test_catalog_content` **16/16**, `test_catalog_contract` 7/7, `test_inventory_engine` **7/7**.
- Falsabilidad: con el loader de HEAD fallan 5 (2 D-03 + arco + sweep + aceite); el sub-caso sintético falla por inspección (la clave no se lee en HEAD).
- Queda fuera: loaders `RpgCatalogs.h` ya no debe nada a D-05 salvo DEC-05 (necesita defaults en `Application.h`/editor = zona codex) y el `healthCap` (G-12). `passive`-objeto y `effect.description` quedan para D-11.

## Trabajo ajeno en vivo (regla 2)

- codex C-23 nuevo (Render visual + `MapDocument.visualSettings` + hooks `Application` + `tests/Render/LevelVisualEffectsTests`): Render, MapDocument, Application y ProjectSerializer vetados.
- C-24 sigue en `tests/Game|Engine`; zcode sin fila pero `RpgCatalogs.h`/`CatalogContentTests.cpp` verificados con solo-sello antes de tocar.
- `EventLayerDocument.h/.cpp` untracked sin fila (posible E-01 en marcha): vetado.

## Pendiente de commit (pathspec)

`RpgCoreDefinitions.h` (limpio: commiteable solo) + `RpgCatalogs.h` + `CatalogContentTests.cpp` + `InventoryEngine.cpp` + `InventoryEngineTests.cpp` + hunk CMakeLists. Súmense D-09, D-03, D-05-motor/maxHealth.
