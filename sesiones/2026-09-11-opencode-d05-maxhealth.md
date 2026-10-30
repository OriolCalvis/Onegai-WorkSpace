# 11/09/2026 — D-05 maxHealth: el motor ja no descarta el bonus de vida

- **Gantt:** corte B, fase D (continuación de mi rebanada D-05 motor). Elegido con `MAPA_DEL_CODIGO.md`: `CharacterSheet.h` solo lo citan C-20 (cerrado) y D-06 (sin dueño) y ninguna fila viva lo reclama.
- **Fila reclamada antes del primer fichero.** Sin commit a propósito (regla 2). Rama `feature/editor-s1-contratos`.

## Qué se ha hecho

El tercer bug motor de D-05 («descarta el `maxHealthBonus`»): `apply_bonus()` lo tiraba con `(void)eq.maxHealthBonus`.

- `include/RPG/CharacterSheet.h`: campo nuevo `equipMaxHealthBonus = 0` junto a los otros bonus de equipo.
- `src/RPG/InventoryEngine.cpp`: `apply_bonus()` lo acumula con signo (equip/unequip); `REBUILD` lo pone a cero y lo reconstruye con la misma deduplicación de ids de la sesión anterior (un 2H con bonus de vida tampoco duplica).
- `tests/RPG/InventoryEngineTests.cpp` (mío): 2 casos nuevos — anillo +5 (acumula, REBUILD conserva, unequip retira) y 2H con bonus 4 (una sola vez, incluso tras REBUILD).

## Límites explícitos (no hechos a propósito)

- **`healthCap()` intacto.** Consumir el acumulador en la vida tope cambia números de combate → decisión de balance, territorio G-12/D-05-completo. Medido: **0 entradas** de `assets/catalogs/*.json` traen bonus de vida ≠ 0, así que este cambio no altera hoy ningún número del juego.
- **Loaders `RpgCatalogs.h`** (zcode D-04 vivo), **DEC-05/assets** (zona codex), **editor/Application** (codex), **`tests/Game|Engine`** (C-24) — ni mirados.

## Verificación (build limpio en temporal)

- `test_inventory_engine` **6/6** (4 anteriores + 2 nuevos).
- Falsabilidad: los 2 tests nuevos no compilan contra HEAD (el campo no existe) — el bug era estructural, no de valor.
- Regresión cercana (misma lib): `test_condition_engine` 12/12 (11 la última vez — alguien añadió un caso en paralelo; pasa igual).
- De paso: `test_battle_state` **13/13 por vía CMake normal** — el pendiente de enlace de mi sesión D-09 lo resolvió otra mano (`motor_level PUBLIC … motor_world` ya en árbol); actualicé mi fila Hecho del D-09.

## Trabajo ajeno en vivo (regla 2)

- C-24 (otro opencode) ya creó `tests/Game/BusinessEconomyTests.cpp`, `BusinessTests.cpp`, `MoralityTests.cpp` — confirmo que `tests/Game` sigue vetado para mí.
- zcode D-04 sigue en `RpgCatalogs.h`/`CatalogContentTests.cpp`/`CatalogContractTests.cpp` (modificados hoy 12:47).
- codex en editor/Application/Project/Level + plan (`dades.py`, gantt, `seguimiento/C-13.json`, `C-14.json` tocados).
- Cabecera ajena de `marcar_paquetes.py` también en `CharacterSheet.h` — no la toco ni la arrastro.

## Pendiente de commit (pathspec, cuando se asiente el árbol)

`include/RPG/CharacterSheet.h` (mi campo) + `src/RPG/InventoryEngine.cpp` (mis hunks) + `tests/RPG/InventoryEngineTests.cpp` (nuevo) + hunk `test_inventory_engine` del CMakeLists. Súmense mi D-09 (6 ficheros + `D-09.json`) y mi D-05-motor anterior.
