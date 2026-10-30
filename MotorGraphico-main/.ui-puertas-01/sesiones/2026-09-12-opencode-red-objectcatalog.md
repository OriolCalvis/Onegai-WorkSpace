# 12/09/2026 — Tancament C-24, prova del pla de D-03 i red sobre el ObjectCatalog

- **Commits:** ninguno (sin luz verde; ficheros nuevos + registros CMake). Rama `feature/editor-s1-contratos`.
- **Deja esperando a Oriol:** (1) luz verde para commitear por pathspec los 5 ficheros míos acumulados (`tests/Game/{Skill,EnemyBrain,CarryOver}Tests.cpp`, `tests/RPG/MonsterPlanProofTests.cpp`, `tests/Level/ObjectCatalogTests.cpp` + bloques CMake); (2) decidir `ObjectCatalog::exportJson()` (declarada sin definición, hallazgo abajo).

## Qué se ha hecho

### C-24 cerrado del todo (101 testes)
- El refactor transitorio de `RpgCatalogs.h` (sesión D-06) cuajó y `test_skill` compila y pasa en CTest (#71): **7/7 suites C-24 verdes** (BusinessEconomy 18, Morality 19, BusinessMode 16, GameMode 14, CarryOver 7, EnemyBrain 14, Skill 13).

### D-03 — la prueba del plan, en target propio
- Auditoría vía probe contra el loader de `ac17bac`: los **432** entran con `maxHealth>0` y stats no todas cero (la letra del PLA ya se cumplía); `movement` se lee (4→5, 5→329, 6→98); `skillIds` está **vacío en los 432** (correcto: no hay nada que leer); `passive` es dict `{name,description}`, no un id.
- Nuevo `tests/RPG/MonsterPlanProofTests.cpp` + `add_motor_unit_test(test_monster_plan_proof)`: 2 barridos a la letra del plan (432 vida>0; 432 stats no todas cero), 3 cartas concretas (`ultimo_creador_furioso`, `portador_del_feretro_real`, `pedazo_de_constelacion_primigenio`), y canario de defenses no-default. **5/5 en CTest (#72)**. El WIP huerfano `tests/RPG/MonsterCatalogTests.cpp` queda intacto (regla 2): su escombra es más laxa que el PLA, el mío cumple la letra.

### Red sobre el ObjectCatalog del camino jugado (refuerzo de C-24)
- `ObjectCatalog` no lo cita ningún paquete del plan (MAPA_DEL_CODIGO tabla 3) pero es la base de objetos del camino que se juega. Nuevo `tests/Level/ObjectCatalogTests.cpp` + `test_object_catalog` (#73), **7/7 verde**:
  - 97 cargan, 55 prop / 19 pickup / 23 npc / 0 enemy.
  - Sin defaults silenciosos en lo que el JSON trae (todos con spriteId>=0, pickups con precio>0 o efecto, 0 NPCs mudos, negocios con ingreso base y nombre).
  - **Integridad referencial de `shop.items[]` → ids del catálogo** (la red que pilla las referencias a inexistentes).
  - Cartas concretas del mercado DEC-05: pocion 30/heal15, eter 45/restoreMana5, pan 8/heal5, cartel_banco (Banco, 4200, 85), tendero_mercado (2 líneas, buyback 50, items pocion-eter-pan).
  - Merge aditivo y override «último archivo gana»; errores estructurales (sin objects / sin id / categoría desconocida / efecto de pickup desconocido / todo-o-nada) y salida `exportJson` → documentado, ver hallazgo.

## Hallazgos (regla 3 — registrados, no resueltos en silencio)

1. **`ObjectCatalog::exportJson()` está declarada en el header sin definición en ningún `.cpp`** (el comentario daba por verificado el round-trip «en demo_object_catalog», demo inexistente). Es exactamente la clase de violación que C-22 dijo arreglar («cap declaració es queda sense cos») y sobrevivió en el árbol. Mi red no pudo fijar el round-trip. **No se toca la declaración** (regla 2); queda en Pendiente del TABLERO para que Oriol/una sesión la implemente o la retire.
2. **`demo_battle` y `demo_ciudad` siguen en rojo** (smoke GL-free, pre-existentes). `demo_battle`:135 espera «Mordisco venenoso» en el log del turno del goblin y no llega — podría ser transitorio por el trabajo de la sesión D-11 sobre `BattleState.cpp` (zona ajena en vuelo, no toco). `demo_ciudad`:171 falla un `caminarHasta`.
3. **`test_skill_effect` (#94) pasa y `test_skill_effect_real_assets` (#95) falla** — ambos de la sesión D-11 (zona ajena activa). Observado, no tocado. **Nota al cierre:** en el CTest de verificación final `test_skill_effect_real_assets` ya salió verde (la sesión D-11 lo dejó cerrado mientras yo documentaba); observación sin restos.

## Validación

- CTest full: **107 tests, 98% (105 verde)**, únicos rojos `demo_battle`/`demo_ciudad` (smoke pre-existentes).
- Míos en CTest: #71 test_skill 13/13, #72 test_monster_plan_proof 5/5, #73 test_object_catalog 7/7, #65-#70 C-24 (6 anteriores).

## Notas de riesgo

- `assets/objects/ciudad_objetos.json` es contenido publicado: los contadores fijos (97/55/19/23) y las cartas concretas avisan si alguien lo retoca (no callan).
- `test_skill_effect_real_assets` rojo = D-11 en vuelo: si el D-11 no lo deja verde al cerrar, sería un rojo heredado a registrar (no es mío).