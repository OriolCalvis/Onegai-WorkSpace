# 2026-09-23 — opencode — EST-29 Split SaveGame + EST-33 Split AdventureValidation

## Qué hice

### EST-33: Split AdventureValidation.cpp
- **518 → 285** línies; nou fitxer `AdventureBeatValidation.cpp` (225 línies)
- Separació natural: validació d'esdeveniments (ValidateEvents) vs validació de beats (ValidateAdventure)
- `AdventureBeatValidation.cpp` afegit a `cmake/AdventureValidationTests.cmake`

### EST-29: Split SaveGame.cpp
- **661 → 330** línies; nou fitxer `SaveGameFormat.cpp` (331 línies)
- `SaveGameFormat.cpp` conté: helpers (writeGridCoord, readGridCoord, writeStringArray, readStringArray, fail), `serializeSaveGame`, `parseSaveGame`, `applyNarrativeState`
- `SaveGame.cpp` conserva: `writeSaveGameFile`, `readSaveGameFile`, `GameSession::captureSaveGame`, `GameSession::restoreSaveGame`
- `SaveGameFormat.cpp` afegit a `CMakeLists.txt` dins de `motor_level`

## Verificació
- `ctest -R test_adventure_validation` → 1/1 passa
- `ctest -R test_save_game` → 1/1 passa
- `ctest -L gl-free` → 150/150 passen
- `python3 tools/linea_base_core.py` → OK — `cuellos` 13→11 (2 fissions)

## Canvis fitxers
- `src/RPG/AdventureValidation.cpp` — retallat (518→285)
- `src/RPG/AdventureBeatValidation.cpp` — NOU (225)
- `cmake/AdventureValidationTests.cmake` — afegit nou fitxer
- `src/Game/SaveGame.cpp` — retallat (661→330)
- `src/Game/SaveGameFormat.cpp` — NOU (331)
- `CMakeLists.txt` — afegit SaveGameFormat.cpp a motor_level
- `planificacion/TABLERO_ESTABILIZACION.md` — EST-33 i EST-29 marcats com a "hecho"
- `planificacion/linea_base_core.json` — actualitzat (`cuellos` 13→11)
