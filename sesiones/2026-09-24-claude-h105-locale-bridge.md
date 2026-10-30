# H1-05 — Bridge OptionsState ↔ Locale (G-17)

## Data/Autor
24/09/2026 — claude (Cowork)

## Gantt origen
`MotorGraphico/planificacion/gantt_jugabilidad_H.puml` — fila **H1-05 (2 j)**:
`G-17: Locale cableado a los textos del motor`. Depende de **H1-03** (ja commital).
Criteri de tancament (pla): *"Opciones → Idioma → catalán/castellano: los textos
del motor cambian"*.

## Prioritat relativa
- **H1-02** (Cowork, ja commital `64033dd`): HudTimer + ShopEngine a producció. ✅
- **H1-05** (aquesta sessió): bridge OptionsState ↔ Locale, GL-free.
- **H6-04** (no tocat): cablejat GL real a Application.cpp:823 — zona d'opencod.

Altres agents actius (segons TABLERO al 24/09):
- opencod: H0-01, H0-02, H0-05, H0-07 (zona Application.cpp)
- zcode: H1-03 (commital), G-06
- qwen: E-09, E-15
- codex: H8-02, H8-05, EST-34/35/36, C-23
- Eigent: auto-commits

## Context prèvi (exploració)

### Locale (`Engine/Locale`)
- **Header**: `include/Engine/Locale.h` (168 línies). API:
  `loadFromDirectory(dir)`, `loadFromStrings(manifestJson, tablesJson)`,
  `loaded()`, `referenceLanguage()`, `languages()`, `language()`,
  `setLanguage(code)` → `Result<bool>`, `text(key)` → `LocalizedText`,
  `format(key, args)` → `LocalizedText`, `missingKeys(code)`,
  `orphanKeys(code)`, `checkPlaceholders()`, `isValidKey()`,
  `placeholdersOf(tmpl)`.
  `LocalizedText{text, fromFallback, missing, missingArgs, unusedArgs, ok()}`.
- **Source**: `src/Engine/Locale.cpp` (402 línies).
- **Assets**: `assets/i18n/` — `manifiesto.json` (`idioma_de_referencia: "es"`,
  `idiomas: ["en","ca","es"]`, `nombres: {en/ca/es}`), `ca.json`, `en.json`, `es.json`.
- **Tests existents**:
  - `tests/Engine/LocaleTests.cpp` (14 casos, 325 línies)
  - `tests/Engine/LocaleExtensionTests.cpp` (245 línies)
  - `cmake/LocaleTests.cmake`: compila `Locale.cpp` **directament** (no a cap libreria).
    Comentari explícit: *"Locale.cpp se compila AQUI y todavia no dentro de ninguna
    libreria: meterlo en el bloque compartido del CMakeLists es justo lo que
    hace que dos agentes se pisan. Cuando Opciones (G-10) lo necesite enlazado,
    ese paquete lo mueve."*

### OptionsState (`Game/OptionsState`)
- **Header**: `include/Game/OptionsState.h` (83 línies, H1-03 de zcode).
  `language()` retorna `"CATALA"/"CASTELLA"/"ENGLISH"` (no codis ISO).
  Comentari: *"su consumo real (Locale, G-17/H1-05) va aparte."*
- **Source**: `src/Game/OptionsState.cpp` (125 línies). Cicle: `CATALA → CASTELLA
  → ENGLISH → CATALA` via `adjustRight()`/`adjustLeft()`.
- **IMPORTANT**: `OptionsState.cpp` **no està a motor_level** en el CMakeLists
  actual. `test_options_state` enllaça `motor_level` però no troba simbólos
  d'OptionsState (link error preexistent). No és el meu bloc.

### Estructura del repo
- Git root: `MotorGraphico-main/`. Fitxers amb prefix `MotorGraphico/`.
- HEAD actual: `62ee066` ("C-24: Xarxa de seguretat al camí que es juga").
- El meu H1-02 està en l'ancestria (`64033dd`).

## Estratègia d'Stack — Patró G-14 anticollisió

Decisió clau: **NO moure `Locale.cpp` a `motor_level`**. Motius:

1. `cmake/LocaleTests.cmake` i `cmake/G17LocaleExtensionTests.cmake` compilen
   `Locale.cpp` directament amb el comentari explícit de no tocar CMakeLists.txt
   compartit. Mimir-ho hi crea conflictes entre agents.
2. El patró G-14 (`cmake/G14HudTimersAndCommerceTests.cmake`) ja complica
   directament `HudTimer.cpp` + `ShopEngine.cpp` + `BitmapFont.cpp` i linka
   `motor_rpg motor_math` (NO `motor_level`). El mateix patró per mi:
   compilar `Locale.cpp` + `OptionsState.cpp` + `OptionsLocaleBridge.cpp`
   directament i linkar `motor_level motor_json motor_math motor_core`.
3. Tocar `CMakeLists.txt` (bloque de `motor_level`) és "fichero de cruce con
   WIP vivo" (segons la propi propa de zcode a G10).

Així, el únic canvi a `CMakeLists.txt` és 1 `include()` (3 línies), que no
toba el bloc de `motor_level`.

## Fitxers creats/modificats

### Nous
1. **`include/Game/OptionsLocaleBridge.h`** (46 línies) — Declara la classe
   `Game::OptionsLocaleBridge` amb:
   - `static std::string localeCode(const std::string& optionsLanguage)` —
     "CATALA" → "ca", "CASTELLA" → "es", "ENGLISH" → "en".
   - `static std::string optionsLanguage(const std::string& code)` — inversa.
   - `static bool applyLanguage(const OptionsState& options, Engine::Locale& locale)` —
     aplica `locale.setLanguage()` i verifica que el text canvia.
   - Retorna `""` per idiomes desconeguts; usa taula `LanguageMapping` única font.

2. **`src/Game/OptionsLocaleBridge.cpp`** (94 línies) — Implementació. La taula
   de mapatge és la única font d'aplicació: `CATALA→ca`, `CASTELLA→es`,
   `ENGLISH→en`. `applyLanguage` comprova `locale.loaded()`, tradueix el nom,
   crida `locale.setLanguage(code)`, verifica que `locale.language()` coincideix
   i que la clau `"combate.ataca"` no és `missing`.

3. **`tests/Game/OptionsLocaleBridgeTests.cpp`** (109 línies, 6 casos):
   - `locale_mapping_catala_ca` — "CATALA" → "ca"
   - `locale_mapping_castella_es` — "CASTELLA" → "es" + inversa
   - `locale_mapping_english_en` — "ENGLISH" → "en" + inversa
   - `locale_mapping_unknown_returns_empty` — "" / "FRANCES" / "fr" → ""
   - `bridge_apply_language_changes_locale` — OptionsState CATALA→ca: Locale
     canvia a "ca"; cicla a CASTELLA→es: Locale canvia a "es"
   - `bridge_text_changes_between_languages` — clau `"combate.ataca"` canvia
     efectivament: ca "ataca {objetivo} ({dano} de dany)" ≠ es
     "ataca a {objetivo} ({dano} de daño)"

4. **`cmake/OptionsLocaleBridgeTests.cmake`** (29 línies) — Compila
   `OptionsState.cpp` + `Locale.cpp` + `OptionsLocaleBridge.cpp` directament
   (patró G-14), linka `motor_level motor_core motor_math motor_json`,
   té `copiar_assets` com a dependència, `WORKING_DIRECTORY` = build dir,
   etiquetes `unit;gl-free`.

### Modificats
5. **`MotorGraphico/CMakeLists.txt`** — +1 línia: `include(cmake/OptionsLocaleBridgeTests.cmake)`
   després de G14. PATRÓ: backup → checkout HEAD → re-aplicar només aquest
   canvi → commiar → restaurar WIP d'altres agents (E-24 removal en working
   tree preservat).

6. **`planificacion/MAPA_DEL_CODIGO.md`** — Afegit a la secció G-17
   (`include/Game/OptionsLocaleBridge.h`, `src/Game/OptionsLocaleBridge.cpp`,
   `tests/Game/OptionsLocaleBridgeTests.cpp`, `cmake/OptionsLocaleBridgeTests.cmake`)
   + actualització de la llistiga alfabètica de fitxers.

### NO modificats (regla d'anticorrupció)
- `MotorGraphico/CMakeLists.txt` bloc de `motor_level` (respetat WIP d'altres)
- `cmake/LocaleTests.cmake` i `cmake/G17LocaleExtensionTests.cmake`
  (compilen Locale.cpp directament, com ja estaban)
- `include/Engine/Locale.h`, `src/Engine/Locale.cpp` (G-17, no tocar)
- `include/Game/OptionsState.h`, `src/Game/OptionsState.cpp` (H1-03 de zcode, no tocar)
- `src/Engine/Application.cpp` (zona GL d'opencod)
- `tests/Editor/E09BridgeTests.cpp` (qwen, link error preexistent per `addStructuredCommand`)

## Build & verificació

### Reconfiguració cmake
```
cd MotorGraphico/build && cmake .. -DBUILD_TESTING=ON
```
Sortida: `-- Suite GL-free: 104 objectius` (103 + el meu nou test).

### Build
```
cmake --build . --target test_options_locale_bridge
```
- Compila correctament: OptionsState.cpp + Locale.cpp + OptionsLocaleBridge.cpp
- Linck correcte amb motor_level + motor_json + motor_math + motor_core
- Warning: `ignoring duplicate libraries` (motor_json apareix dues vegades —
  és harmless, ja que el test el linka explícitament i motor_level també el
  porta com a PUBLIC).

### Tests
```
ctest -R "test_options_locale_bridge|test_locale|test_locale_extension|test_g14_hud_timers_commerce" --output-on-failure
```
Resultat: **4/4 PASS** (test_options_locale_bridge 6/6 casos, test_locale 14/14,
test_locale_extension, test_g14 6/6).

### Regressió
La suite completa GL-free NO es pot construir des de zero a causa d'errors
preexistents d'altres agents:
- `test_options_state`: OptionsState.cpp no està a motor_level (zcode ho deixat
  com a WIP; "CMakeLists cruce").
- `demo_editor_state`: linker error per `EditorState::addLayer` etc. (qwen EST-24
  refactor amb WIP no commital).
- `test_render_animated_entity`: `AnimatedEntity` constructor mismatch (Altres agent(s)).

Cap d'aquests és atribuïble al meu canvi. Els tests que SÍ es poden construir
i ejecutar passen sense regressió.

## Patró de commiat (backup→checkout→re-apply)
1. `cp CMakeLists.txt /tmp/cm_h105_my_version.txt` (salva WIP + canvis propis)
2. `git checkout HEAD -- CMakeLists.txt` (restaura HEAD)
3. `edit()` — afegeix només l'include de G14 + comment
4. `git add` + `git commit` amb `git-safe.sh`
5. `cp /tmp/cm_h105_my_version.txt CMakeLists.txt` (restaura WIP altres agents)
6. Verificat: `git diff HEAD -- CMakeLists.txt` mostra només la resta d'alta
   agents (E-24 removal), NO els meus canvis (ja commital).

## Conclusió
H1-05 està **HECHO**. El bridge `OptionsLocaleBridge` connecta OptionsState
(català/castellano) amb Locale (ca/es/en) sense tocar Application.cpp. El test
verifica el mapatge, el canvi en calent i el canvi efectiu de text amb les
taules reals d'assets/i18n/. El cablejat GL real (Application → HudText) és
H6-04 i zona d'opencod.
