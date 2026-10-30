# 12/09/2026 — D-03: la prova del pla dels 432 monstres (tancament)

- **Commits:** ninguno (sin luz verde; fichero nuevo + registro CMake solo). Rama `feature/editor-s1-contratos`.
- **Deja esperando a Oriol:** luz verde para commitear `tests/RPG/MonsterPlanProofTests.cpp` + su bloque en `CMakeLists.txt`. Queda el mismo pendiente de siempre para la WIP huerfana de loaders (`RpgCatalogs.h`, regla 2: ni tocar ni versionar sin autorización tipo rescate 03/09).

## Qué se ha hecho

El usuario pidió seguir con el Gantt usando MAPA_DEL_CODIGO y teniendo en cuenta que hay varios agentes en paralelo. Antes de nada cerré el C-24: el refactor transitorio de `RpgCatalogs.h` (sesión D-06) ya había cuadrado y **`test_skill` compila y pasa en CTest (#71)** — 7/7 suites C-24 verdes, 101 tests.

Del TABLERO salió el siguiente hueco limpio: **D-03 (Monstres)**, fila liberada por zcode — faltaba "solo la prueba del plan". Pasos:

1. **Auditoría de la realidad vía loader** (probe en /tmp enlazado contra `build/libmotor_rpg.a` fresco): los 432 entran con `maxHealth>0` y stats no todas cero **con el loader ya versionado en `ac17bac`** (D-03 rescate): `vida_zero=0`, `stats_zero=0`. El `MonsterCatalogTests.cpp` huerfano (untracked, WIP ajeno) suaviza la prueba del plan ("como máximo 1 de cada 5 puede ser vida zero") — no cumple la letra del PLA.
2. **Censo de campos** (python sobre `assets/catalogs/monsters.json`): `movement` se lee de verdad (4→5, 5→329, 6→98), `attacks` en los 432; `skillIds` está **vacío en los 432** (no hay bug de loader: no hay nada que leer); `passive` es dict `{name, description}`, no un id (el struct `passiveId` no tiene fuente). `defenses{ca,defMental,maxHealth,movement,precMag,resFis}` + `derived{}` son los campos de la vida.
3. **Prueba del plan, en target propio** (decisión: no tocar `CatalogContentTests.cpp`, zona viva de D-02/D-07): `tests/RPG/MonsterPlanProofTests.cpp` + `add_motor_unit_test(test_monster_plan_proof ...)` en CMakeLists (tras el bloque de test_skill).
   - `els_432_monstres_es_carreguen_sense_errors`: size()==432.
   - `D03_cap_monstre_duu_vida_zero`: TODOS maxHealth>0 (letra del plan).
   - `D03_cap_monstre_amb_estadistiques_totes_zero`: TODOS con suma de las 4 stats != 0 (letra del plan).
   - `D03_cartes_concretes_escrites`: 3 cartas con valores escritos (`ultimo_creador_furioso` jefe ca22/hp80/11,10,9,9/3 atac; `portador_del_feretro_real` elite ca19/hp46/9,8,7,7/2; `pedazo_de_constelacion_primigenio` elite ca18/hp46/7,7,10,8/2).
   - `D03_defenses_no_es_queden_al_default`: 0 de 432 con la tríada ca/defMental/resFis en defaults (canario del bug original «life buscado en la raíz»).
4. **Verificado**: cmake CFG=0, build del target OK, `ctest -R test_monster_plan_proof` → **1/1 Passed (CTest #72)**, 0.41 s.

## Validación

- `test_monster_plan_proof` 5/5 casos en CTest.
- Suite C-24 completa: 7/7 (incluyendo `test_skill` #71) verde.
- Sin tocar: `RpgCatalogs.h` (WIP huerfano / D-06), `tests/RPG/MonsterCatalogTests.cpp` (WIP huerfano), `CatalogContentTests.cpp` (D-02/D-07), `Application.*`, `GameSession.cpp`.

## Notas de riesgo

- `skillIds`/`passiveId` no tienen fuente en el JSON de monstruos: cuando D-11 consuma habilidades de monstruo habrá que rellenar el asset aguas arriba (no es bug de motor). Anotado para no confundirlo con una regresión del D-03.
- El pendiente del TABLERO («quien versionee el loader debe meter su prueba EN EL MISMO commit») sigue abierto para el lote de loaders; mi prueba es un fichero nuevo independiente y no lo resuelve.