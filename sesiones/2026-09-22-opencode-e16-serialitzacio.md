# 22/09/2026 — E-16: serialització completa (save/load round-trip)

## Feito
- **E-16 completat**: implementació de `save()` i `load()` al `ProjectDatabaseState`
  - `writeJsonValue()` recursiu que serialitza qualsevol `JsonValue` amb `JsonWriter`
  - `readFile()` per llegir fitxers a string
  - Format JSON: `{"entries": [{id, name, type, valid, fields}]}`
  - `save()` usa `std::fwrite` directe (GL-free)
  - `load()` usa `JsonValue::parse` + deserialització camps per camps

## Tests (18/18)
- 10 model in-memory: crear, duplicar, validar, actualitzar, verificar id, obtenir per id, count
- 3 round-trip: simple, camps complexos (string/int/bool), buit
- 3 edge cases: fitxer inexistent, JSON invàlid, JSON sense entries
- 2 id preservation: save→load manté IDs exactes

## Fix preexistent
- `RpgCatalogs.h` línia 1449: `return l;` orfanat fora de funció (introduït pel commit `4d5ecd5`). Causava errors en cascada en tot binary que incloïa `ProjectDatabaseState.h` (via `RpgCatalogs.h`). Eliminat.

## CMake
- `cmake/ProjectDatabaseTests.cmake` actualitzat: apunta a `tests/Editor/ProjectDatabaseTests.cpp`, linka només `motor_json` (GL-free pur)

## Suite gl-free: 145/145
- 7 tests menys que l'última medició (152) per `-DMOTOR_GL_TESTS=OFF` en la reconfiguració

## Coordinaixó
- Evitat `Application.cpp` (14 paquetes), `GameSession.cpp` (6), `level_editor.cpp` (10)
- Només tocat: `ProjectDatabaseState.cpp` (propi E-16), `RpgCatalogs.h` (fix sintaxi), `cmake/ProjectDatabaseTests.cmake`, test nou
- `tests/Project/ProjectDatabaseTests.cpp` (fitxer vell, 6 tests) no tocat — seguit per un altre agent
