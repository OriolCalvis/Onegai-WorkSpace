# 2026-09-19 — claude — G-07.1 demo de congercio GL-free

## Objectiu
Verificar end-to-end el mecanisme de serialització de G-07 (serializadors de
congerció) sobre assets reals, no només sobre fixtures de test. La congerció
de G-07 està completa (serializadors + test_schema_fixtures 9/9 + fixtures
congelades), però el Gantt indica com a "Paquet ABIERTO" fins que E-16 no
congeli l'esquema de projecte. Aquest demo comprova que els serializadors
funcionen correctament amb dades reals i que la propietat d'idempotència
(serializar → parsejar → serializar == serializar) es compleixi.

## Files creades / modificades
- **Nova:** `MotorGraphico/examples/demo_congelacio.cpp` (155 línies, GL-free)
- **Modificada:** `MotorGraphico/CMakeLists.txt` (+1 línia:
  `add_motor_gl_free_demo_test(demo_congelacio)` després de `demo_aventura`)
- **Nova:** `MotorGraphico/planificacion/MAPA_DEL_CODIGO.md` (entrada demo_congelacio)
- **Modificada:** `TABLERO.md` (entrada "Hecho" G-07.1)

## Verificacions
1. **Build:** `cmake --build build/ --target demo_congelacio -j8` → 0 errors
2. **ctest:** `ctest -R demo_congelacio` → **Passed (Test #43, 0.01s)**
3. **Suite GL-free:** 97 objectius (6nou amb demo_congelacio inclòs)
4. **Demo runtime:**
   - `aventura_silbido_medianoche.json` → 8 beats, 3 objectius, 0 temps,
     idempotència OK (7022 bytes estables)
   - `test_level.json` → 2 enemics, 4 objectes, idempotència OK (1118 bytes)
   - `ciudad_centro.json` → 0 enemics, 46 objectes, idempotència OK (8123 bytes)
   - Fixture `aventura-v1.json` → idempotent (3266 bytes)
   - Fixture `nivel-v1.json` → idempotent (1335 bytes)

## Tècniques concurrent
- Altres agents estan editant `CMakeLists.txt` (E-10: `EventCommands.cpp`, E-08:
  `test_animated_entity`, D-11: `SpeedZeroFlagTests.cmake`, E-16:
  `NewGameFlowTests.cmake`, C-24: `test_camera_transition`). S'ha usat la
  tècnica backup→filter→commit→restore per només commital els propis canvis.
- Els arxius de demos existents (`demo_aventura.cpp`, `demo_level_loader.cpp`)
  no s'han tocat — només s'afegeix un arxiu nou.
- El demo usa `require()` (no `assert()`) per complir la regla de la BITACORA
  (Release defineix NDEBUG).
- Sense tildes/ñ en strings de JSON d'aventura (el demo no genera JSON).

## Commit
- MotorGraphico repo: `8241855` — "G-07.1: demo GL-free de congercio de serializers sobre assets reals + fixtures"
- Software root repo: pendent (TABLERO.md + session file)
