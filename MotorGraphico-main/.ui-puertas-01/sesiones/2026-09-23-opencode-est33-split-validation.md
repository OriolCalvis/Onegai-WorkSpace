# 2026-09-23 — opencode — EST-33 Split AdventureValidation

## Qué hice
- **EST-33**: Split `src/RPG/AdventureValidation.cpp` (518→285 línies) en dos fitxers:
  - `AdventureValidation.cpp` (285 línies): `ValidationReport` methods + `ValidateEvents` + helpers d'esdeveniments
  - `AdventureBeatValidation.cpp` (225 línies NOU): `ValidateAdventure` (2 overloads) + helpers de beats
- Separació natural a la línia 286: la meitat d'esdeveniments vs la meitat de beats
- `AdventureBeatValidation.cpp` afegit a `cmake/AdventureValidationTests.cmake`
- Cap canvi de funcionalitat: només moviment de codi

## Verificació
- `cmake --build . --target test_adventure_validation` → OK
- `ctest -R test_adventure_validation` → 1/1 passa
- `ctest -L gl-free` → 149/149 passen (abans 145/146 + 4 nous tests d'altres agents)
- `python3 tools/linea_base_core.py` → OK — `cuellos` 13→12

## Canvis fitxers
- `src/RPG/AdventureValidation.cpp` — retallat (518→285)
- `src/RPG/AdventureBeatValidation.cpp` — NOU (225)
- `cmake/AdventureValidationTests.cmake` — afegit nou fitxer
- `planificacion/TABLERO_ESTABILIZACION.md` — EST-33 marcat com a "hecho"
- `planificacion/linea_base_core.json` — actualitzat (`cuellos` 13→12)
