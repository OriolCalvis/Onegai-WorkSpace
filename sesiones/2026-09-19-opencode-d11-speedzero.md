# 2026-09-19 opencode — D-11: flag `speedZero` per `velocidad_0`

## Context
- Represa del Gantt del MotorGraphico (PLA_DEFINITIU.md). Inspeccionat el pendent D-09/Pla línia 993: «D-11 haurà de traduir més mechanicHooks a flags/statMod/damagePerRound».
- L'asset `assets/catalogs/conditions.json` (16 condicions) té ~30 hooks crus; el loader (D-04) en tradueix 6. Els únics unívocs i avalats pel GDD per traduir sense inventar mecàniques són els del llistat. `velocidad_0` és un dels que el propi comentari D-04 deixa lligat: «NO és halfSpeed (és més fort) i queda cru fins que existeixi el flag que la carta mereix».
- Documentat: `velocidad_0` apareix a 3 cartes públiques: agarrado, anclado, paralizado.
- Fila reclamada a TABLERO «En curso» abans de crear el primer fichero (regla 1). Ningú més tenia la zona.

## Canvis
1. `MotorGraphico/include/RPG/Definitions/RpgCoreDefinitions.h` — `ConditionDefinition`:
   - NOU camp `bool speedZero = false;` (additiu; els 8 paquets que comparteixen el header no canvien de comportament).
2. `MotorGraphico/include/RPG/Catalogs/RpgCatalogs.h` — loader de condicions:
   - `else if (hook == "velocidad_0") { c.speedZero = true; }`.
   - Comentari D-04 actualitzat («velocidad_0 -> speedZero (D-11; agarrado/anclado/paralizado; més fort que halfSpeed)»), eliminada la frase que deia que quedava cru.
   - El hook cru es conserva a `mechanicHooks` (contracte D-04 de DSL raw intacte).
3. `MotorGraphico/include/RPG/ConditionEngine.h` / `src/RPG/ConditionEngine.cpp`:
   - NOU `ConditionEngine::has_speed_zero(sheet, cat)`, gimmella exacta de `has_half_speed` (mateix gating: stacks>0 i roundsRemaining>0).
4. NOUS: `MotorGraphico/tests/RPG/SpeedZeroFlagTests.cpp` (4 tests GL-free) + `MotorGraphico/cmake/SpeedZeroFlagTests.cmake` (target `test_speed_zero_flag`, enllaça només `motor_rpg`+`motor_json`, SENSE `motor_core` — el target desacoblat evita Application.cpp en edició aliena).
5. `MotorGraphico/CMakeLists.txt` — `include(cmake/SpeedZeroFlagTests.cmake)` just després de `ConditionsRealAssetsTests.cmake`.

## Tests (build temporal des de zero, Release, GL-free)
- `test_speed_zero_flag` — 4/4: (1) agarrado/anclado/paralizado carreguen `speedZero` i conserven el hook cru; en tot el catàleg no hi ha cap altra carta amb speedZero (3 comptats) i almenys una té halfSpeed sense zero (ralentizado/lent, contrast); (2) helper false en motlles nets, true amb condició aplicada, false al remove; (3) `speedZero` i `halfSpeed` són independents (cap activa l'altre); (4) condició ROUNDS amb defaultRounds=1: expira i el helper torna a false.
- Regressió de la zona tocada: `test_condition_engine`, `test_conditions_real_assets`, `test_catalog_content`, `test_catalog_contract` — 4/4 greens.
- Total: 5/5 greens.

## Xocs multi-agent (registrats, Regla 3 de la Bíblia)
1. El primer configure va fallar: `add_test test_aventuras_monsterid ja existeix` (include duplicat de `AventurasCatalogNetTests.cmake`, línies ~1100/1152 del CMakeLists). Entre les meves operacions un agent germà va consolidar el duplicat (el segon grep només troba línia 1096). No vaig tocar res aliè.
2. Un commit aliè es va endur el meu `include(cmake/SpeedZeroFlagTests.cmake)` entre l'edició i el configure (patró del Pendiente 33). Detectat pel grep, reaplicat el 19/09.

## Entregat / pendent
- Entregat i verificat QA corrent amb estos 5 tests a HEAD actual. **Sense commit** (regla 2 del workspace: no hi ha llum verda de commit explícit; el WIP propi queda a l'arbre com la resta de sèries, segons patró habitual).
- Pendent natural per futures sessions: la traducció d'altres hooks unívocs (p.ex. `desventaja:todo_con_fuente_visible`, `bloquea_tag:Concentracion`, `pierde_reaccion`, `muerte:nivel_6`) quan la simulació de combat les necessiti — sempre amb flag tipat nou, no mapes silenciosos a camps existents (filosofia D-04/D-11).