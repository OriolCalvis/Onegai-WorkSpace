# 2026-09-24 — opencode — EST-28 Split ProjectIndex + EST-29/EST-33

## Qué hice

### EST-28: Split ProjectIndex.cpp
- **683 → 367** línies; nou fitxer `ProjectIndexDuplicate.cpp` (316 línies)
- `ProjectIndexDuplicate.cpp` conté: E-11 (suggestId, uniqueId, createNamed, duplicate) + helpers (plegarAcento, reprefijar, copiaFichero, carpetaDe) + duplicats de leeFichero/existe/escribeIndice
- `ProjectIndex.cpp` conserva: scan, create, remove, addLevelToProject + helpers originals
- `ProjectIndexDuplicate.cpp` afegit a `CMakeLists.txt` dins de `motor_project`

### EST-33: Split AdventureValidation.cpp (ja fet abans)
- **518 → 285** línies; `AdventureBeatValidation.cpp` (225 línies)

### EST-29: Split SaveGame.cpp (ja fet abans)
- **661 → 330** línies; `SaveGameFormat.cpp` (331 línies)

## Verificació
- `ctest -R test_project` → 8/8 passen (incloent `test_project_naming` que exercita E-11)
- `ctest -L gl-free` → 150/150 unit + 2 pre-existents (`catalog_loader_contract`)
- `python3 tools/linea_base_core.py` → OK — `cuellos` 11→6

## Canvis fitxers
- `src/Project/ProjectIndex.cpp` — retallat (683→367)
- `src/Project/ProjectIndexDuplicate.cpp` — NOU (316)
- `CMakeLists.txt` — afegit ProjectIndexDuplicate.cpp a motor_project
- `planificacion/TABLERO_ESTABILIZACION.md` — EST-28 marcat com a "hecho"
- `planificacion/linea_base_core.json` — actualitzat (`cuellos` 11→6, `logica_en_pruebas` 1→0)
