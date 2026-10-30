# 2026-09-17 · Claude (Cowork) · G-07: serializadores de congeracio + fixtures

**Encargo de Oriol:** seguir con el Gantt usando `MAPA_DEL_CODIGO`, amb atencio pels
altres agents que treballen alhora. G-07 proporciona els serializadors i fixtures
que la congelacio de esquemes (1.0-B) necessita.

## Elecció

- **Zonas ocupades:** C-22 (claude, altra sessio), E-04 (Claude), E-05/E-06/E-08 (qwen),
  E-13 (qwen), E-15 (qwen), C-22 (claude), G-12b (opencode), E-12 (cursor), AUD-01 (qwen),
  AUD-01 (qwen). G-02 (cursor, sense fila).
- **G-07:** implementar `serializeAdventure` i `serializeLevelDefinition` com a
  inversos de `loadFromString`, amb fixtures congelades i tests d'idempotencia.
- **Lo que no se tocó:** el parser de `NarrativeEngine.cpp` (E-02) i `LevelLoader.cpp`
  (C-11); el codi existent de G-08/G-12/G-13/G-14.

## Qué se hizo (MG `fd9027d`)

### Serializadors (archivos nuevos, G-07 exclusiu)

- `src/RPG/AdventureSerializer.cpp` — `RPG::serializeAdventure(const AdventureScript&)`:
  usa `JsonWriter` (header-only, `include/Core/Json/JsonWriter.h`); mapeja enums a texto
  amb `triggerTypeToText`/`effectTypeToText`/`presentacionToText`; helper
  `writeStringArray` en anonymous namespace. Camps condicionals: `description` només si
  no està buit, `requires` només si `isEmpty()`, `skillCheck`/`startBattle` només si el
  tipus ho demana, `zona` només si no està buit. `expiraPor` com a array de strings.

- `src/Level/LevelSerializer.cpp` — `serializeLevelDefinition(const LevelDefinition&)`:
  usa `JsonWriter`; helpers `writeGridCoord`/`writeStringArray`. `patrolMin`/`patrolMax`
  només si difereixen de `position`; `targetPosition` només si `hasTargetPosition`;
  `scale` només si != 1.0; `fog`/`lighting` només si `enabled`.

### Declaració al header de cruce

- `include/RPG/NarrativeEngine.h` — afegit `serializeAdventure()` al costat de
  `loadFromString`/`loadFromFile` (crosse E-02 + G-07; ownership block actualitzat).
  La implementació viu a `AdventureSerializer.cpp`, no a `NarrativeEngine.cpp`.

### Fixtures i tests

- `tests/Integration/SchemaFixtureTests.cpp`: afegits `aventuraDeReferencia()` i
  `nivelDeReferencia()` builders, 2 tests de contingut novellos
  (`l_aventura_congelada_se_abre_i_conserva_els_seus_camps`,
  `el_nivell_congelat_se_obre_i_conserva_les_seves_entrades`), i extensos el test
  d'idempotencia amb aventura i nivell. Mode `--escribir` genera `aventura-v1.json` i
  `nivel-v1.json`.

- `tests/fixtures/esquemas/aventura-v1.json` (congelada) — 3 beats (Enter/Talk/Auto,
  requisits allFlags/notFlags/anyFlags, efectes SetFlag/Log/GrantGold/SkillCheck/StartBattle),
  2 objectius, 1 timer amb `expiraPor` dual, avisos i `presentacion: secreto`.
- `tests/fixtures/esquemas/nivel-v1.json` (congelada) — enemic amb patrulla, porta de
  transicio, cofre amb loot/propietats/scale=1.5, fog i lighting C-23.

### Documentacio

- `FORMATO_AVENTURAS.md §7` i `FORMATO_NIVELES.md §7`: documenta mapeo d'envs, camps
  condicionals i el contracte d'idempotencia.
- `planificacion/MAPA_DEL_CODIGO.md`: registra els 4 archis nuevos a G-07;
  `NarrativeEngine.h` marcat com a compartit amb G-07.

### CMakeLists.txt (commit quirurgico per hunks)

La linha `src/RPG/EventCommands.cpp` (E-10) i `include(cmake/E09BridgeTests.cmake)`
(E-09) ja estaven en el arbre sense commitear. S'han deixat en el working tree;
només s'han fet servir els hunks propis (AdventureSerializer.cpp a `motor_rpg`,
LevelSerializer.cpp a `motor_level`). El backup temporal s'ha esborrat.

## Verificación

- `test_schema_fixtures`: **9/9 verdes** (7 existents + 2 nous). Inclou el test
  d'idempotencia que comprova que reescriure `aventura-v1.json` i `nivell-v1.json`
  no canvia un sol byte.
- Suite GL-free: 94/94 (2 Not Run son ajenos en vuelo: `test_asset_browser_state`,
  `test_title_flow`). Sense regressions.
- Regenerat `tests/fixtures/esquemas/` amb `--escribir` + `cmake ..` per copiar al
  build dir (el `file(COPY ...)` de CMakeLists corre al configurar, no al compilar).

## Para quien siga

- **G-07 completa:** la resta de la tasca (congeracio a 1.0-B, migracions futures,
  `schemaVersion`) queda pendient de que E-16 acabi el seu canvi al projecte.
  Aquests serializadors són el mecanisme que aquella congelacio necessita.
- **NarrativeEngine.h:** cualsevol canvi en `AdventureScript` (afegir camps, canviar
  enums) ha de ser retropropagat a `serializeAdventure` i a la fixture
  `aventura-v1.json` (regenerar amb `--escribir` + reconfigurar CMake).
