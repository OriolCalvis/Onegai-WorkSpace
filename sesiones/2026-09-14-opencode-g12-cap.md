# 14/09/2026 — G-12 (rodaja de cálculo): el bonus de vida del equip llega al healthCap()

- **Semilla (opencode 12/09):** mis rodajas D-05 dejaron anotado que «aplicar `equipMaxHealthBonus` al `healthCap()` es decisión de balance (G-12)».
- **Commits:** ninguno (sin luz verde; y CMakeLists tiene hunks ajenos en vuelo). Rama `feature/editor-s1-contratos`.
- **Zonas respetadas:** Application.cpp/GameSession.cpp/InventoryEngine.cpp (una sesión paralela está cableando el inventario ahí MISMO); editor/ (codex E-10); SkillEffect/BattleState (D-11).

## Qué se ha hecho

- **`src/RPG/CharacterSheet.cpp:67`** — `healthCap()` pasaba solo `equipArmorBonus/2` como término de equipación y aplazaba el bonus explícito. Ahora: `eqBonus = equipArmorBonus / 2 + equipMaxHealthBonus`. La fórmula (`TierRules::compute_cap_health`) ya aceptaba el `extra_equipment_health_bonus`; el término entra **dentro del min()** del cap de tier, igual que hacía la aproximación de armadura (no cambio de semántica de capping).
- **`include/RPG/CharacterSheet.h`** — comentario del campo actualizado a la realidad (G-12 ya lo suma).
- **Nuevo `tests/RPG/SheetLifeCapTests.cpp` + `test_sheet_life_cap` en CMakeLists** (link motor_rpg) — **5/5**: bonus suma exacto, cohabita con armor/2, desequipar retira, recompute/recompute incremental no duplica (10→13 = +3) ni la corrección a la baja, el clamp del tier manda con el bonus dentro (35), y el camino cacheado `recompute_derived()→healthCap()` coincide con la fórmula directa.

## Verificación

- `test_sheet_life_cap` 5/5. Suites acopladas **10/10 en verde**: condition_engine, character_creation_flow, save_game, battle_state, game_session, enemy_brain, inventory_engine, catalog_content, character_creation, skill_effect.
- GL-free global: **112/113 (99%)** — único rojo `test_title_screen` (ver hallazgos).

## Hallazgos (regla 3 — registrados, no tocados)

1. **`test_title_screen` (G-08, rodaja qwen) en rojo contra HEAD** — `entries().size()==3` falla; nada de TitleScreen está en vuelo (git limpio), así que es una regresión de HEAD plausiblemente por la consolidación de proyectos C-11/E-11 (el índice que escanea cambió). Dueño: G-08 (qwen/opencode para el seguinte tramo). No es mío; queda en Pendiente del TABLERO.
2. **Build completo en rojo por una sesión en vuelo** — 20 errores en `Application.cpp`/`GameSession.cpp`: les faltan `GameSession::{equipItem,unequipItem,useItem}`, `Game::EquipSlot`, `InputAction::{MenuLeft,MenuRight}`, `createPauseMenuWidgets`/`confirm*Command`, `kMenuPadding`… Están **integrando mis pantallas GDB-12** (mismas, otro agente) — zona ajena viva, no se toca. GL-free sigue verde porque no compila esos .cpp.

## Notas de riesgo

- Mi `healthCap()` suma ahora el bonus real del equip; si una sesión posterior decide que el bono de equip va FUERA del min() del cap (el comentario de TierRules.h lo sugiere), es cambio de balance de G-12 con su Ho prova, no de esta rodaja.
- CMakeLists.txt llevaba hunks ajenos antes que los míos (`MM`); al versionar hay que separar por pathspec/hunk manual (mismo patrón ya documentado en sesiones previas).
- MAPA no lista todavía CharacterSheet como fichero de G-12: la regeneración `marcar_paquetes.py --marcar` quedará para la consolidación de G-12 (no la ejecuté para no hostigar bloques generados con otras sesiones tocándolos, p. ej. SkillEffect.h el 13/09).