# 11/09/2026 — D-05 meitat motor: 2H sense doble bonus + escut vs mà esquerra

- **Gantt:** corte B, fase D. Triat amb `planificacion/MAPA_DEL_CODIGO.md` (fichero de cruce + qué toca cada paquete) contra el TABLERO en vivo.
- **Fila reclamada antes del primer fichero** («D-05 meitat motor (sense RpgCatalogs.h)»).
- **Commits:** ninguno a propósito (regla 2, ver abajo). Rama `feature/editor-s1-contratos`.
- **Deja esperando a Oriol:** nada nuevo.

## Alcance deliberado (qué NO se toca y por qué)

D-05 entero pide además loaders + DEC-05 + maxHealthBonus. Quedan fuera a propósito:

- **Loaders `RpgCatalogs.h` (equipment/consumables/objects):** fila viva de zcode (D-04) sobre el mismo fichero — ni mirarlo.
- **DEC-05 (test_objects.json + masmorra a proyecto de ejemplos):** exige tocar `Application.h:67` y defaults del editor — zona viva de codex.
- **`maxHealthBonus`:** no hay acumulador en `CharacterSheet` y `healthCap()` lo deja explícitamente a P1 (`eqBonus = equipArmorBonus / 2`). Cablearlo pide campo nuevo + cambio de fórmula de vida (balance de combate) — territorio D-05-complet/G-12, no de esta rebanada.

## Qué se ha hecho (`src/RPG/InventoryEngine.cpp`, 35 inserciones)

1. **REBUILD no duplica 2H.** `recalc_equipment_bonuses(REBUILD)` iteraba los 7 `equippedIds` y aplicaba el bonus por slot: un arma a dos manos (mismo id en MainHand+OffHand) contaba doble, divergiendo del equip incremental (correcto, una vez). Ahora cada id se aplica una sola vez.
2. **Escudo vs mano izquierda.** `apply_bonus()` mandaba a `equipShieldBonus` todo item cuyo slot contuviera `OffHand`: una daga en la izquierda contaba como escudo. Nuevo helper `is_shield_item()` (slot dice shield/escudo, case-insensitive): solo el escudo real va a shield; lo demás (incluida la izquierda) va a armor. Los escudos del asset (`escudo_de_madera`, slot MainHand) no cambian de rama — su slot se arregla con los loaders, no aquí.

## Prueba (`tests/RPG/InventoryEngineTests.cpp`, NUEVO + hunk CMakeLists)

- `una_arma_a_dues_mans_no_duplica_el_bonus` — el criterio de aceptación del paquete: 2H = 1M del mismo valor, incluso tras REBUILD.
- `desequipar_dues_mans_resta_el_bonus_una_vegada` — unequip por OffHand vacía ambas y resta una vez.
- `un_objecte_a_la_ma_esquerra_no_compta_com_a_escut` — daga OffHand → armor 2, shield 0.
- `un_escut_de_veritat_continua_anant_a_shield` — escudo → shield 2, armor 0 (no-regresión).
- Registro: `add_motor_unit_test(test_inventory_engine …)` + link `motor_rpg` en `CMakeLists.txt` lín. ~880, región sin hunks ajenos.

## Verificación (build limpio en temporal, `cmake -S MotorGraphico -B <tmp>`)

- `test_inventory_engine` **4/4** en verde.
- **Falsabilidad:** con el `InventoryEngine.cpp` de HEAD fallan exactamente los 2 tests de bug (2H y daga) y pasan los otros 2; restaurado el fix, 4/4. (Nota: el restore cayó en el mismo segundo que el `.o` y make lo dio por bueno — `touch` + rebuild para el verde final.)
- Regresión cercana (misma lib): `test_condition_engine` 11/11.

## Trabajo ajeno en vivo (regla 2: no deshecho ni commiteado)

- codex: `EditorState.*`, `EditorDocumentIO.*`, `level_editor.cpp`, Level*, `ProjectSerializer.*`, `Application.*` + regeneración + plan (`dades.py`, gantt, `pla.html`).
- opencode (otra sesión, C-24): `tests/Game/*`, `tests/Render/BitmapFontTests.cpp`, `tests/Engine/*` — por eso el test nuevo va en `tests/RPG/` y no toco `src/` ajeno.
- zcode (D-04): `RpgCatalogs.h`, `ConditionDefinition.h`, `conditions.json`, `CatalogContentTests.cpp`, `CatalogContractTests.cpp`.
- Cabeceras «PAQUETES DEL PLAN» de `marcar_paquetes.py` en ~40 ficheros incluido el mío (con el mapa ya generado: `MAPA_DEL_CODIGO.md`, usado en esta sesión).
- Novedad útil: el `CMakeLists.txt` ya trae `motor_level PUBLIC … motor_world` — el pendiente de enlace de mi sesión D-09 quedó resuelto por otra mano.

## Sin commitear a propósito

Mi `InventoryEngine.cpp` lleva la cabecera ajena de `marcar_paquetes.py` y el `CMakeLists.txt` trae hunks concurrentes (motor_level, test_world, editor, city_regeneration). Candidato a commit por pathspec cuando se asiente: `src/RPG/InventoryEngine.cpp` (mis hunks) + `tests/RPG/InventoryEngineTests.cpp` (nuevo) + hunk `test_inventory_engine` del CMakeLists. También queda pendiente de commit mi D-09 anterior (6 ficheros + `D-09.json`).
