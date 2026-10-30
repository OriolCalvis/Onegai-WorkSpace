# Sessió — G-09.1: Demo GL-free d'integració "Nova partida"

**Data:** 19/09/2026
**Autor:** Claude (Cowork)
**Objectiu:** Demostrat el flux complet de "Nova partida" sense tocar `Application.cpp` (cruce G-08, fila d'opencode) ni `GameSession.cpp` (G-12).

## GANTT de referència
- G-08 (Títol i arrencada) — COMPLET per opencode (cablejat Application).
- G-09 (Creació de personatge dins Nova partida) → depèn de G-08 ✅.
- G-07 (Congelació d'esquemes) → ja complet; el level loader ja és verd.
- G-11 (Selector de guardats) → dependeix G-09/G-10; SaveSlots ja és GL-free.

## Tasques completades
1. **[G-08]** `TitleMenuState::fromManifest("assets/proyectos/ejemplos.json", "", "")` — verifica títol, entryPath real, NewGame habilitat, `confirm()` retorna `NewGame`.
2. **[G-07]** `LevelLoader::loadFromFile(entryPath())` — carrega `Galeria de objetos e interacciones` + mapa `ejemplo_mapa.tmx` (ambdues existents).
3. **[G-09]** `CharacterCreation::load("assets/catalogs")` → carrega 43 races, 61 classes, 12 backgrounds des de JSON reals. Un `CreationChoice` legal (primera raza + primera classe + primer background amb 7 opcions seleccionades) passa `problemas()` i produeix un `CharacterSheet` vàlid amb:
   - `healthCap() == 30` (via `cc.recalcular(hoja, reglas)` amb TierRules).
   - 3 habilitats de classe + 3 peces d'equip d'arran (GDD: "nadie empieza con una hoja en blanco").
4. **[G-11]** `SaveSlots("build/saves/nova_partida_fresh", projectId)` — verifica 6 ranures buides + quick-slot buit en partida nova.

## Arxius creats/modificats
| Arxiu | Repo | Acció |
|---|---|---|
| `examples/demo_nova_partida.cpp` (206 línies) | MotorGraphico | Nou |
| `CMakeLists.txt` | MotorGraphico | +17 línies (target + ctest) |
| `planificacion/MAPA_DEL_CODIGO.md` | MotorGraphico | +1 demo |
| `TABLERO.md` | Software | G-09.1 a "Hecho" |

## Tècniques usades
- **`Catalog<T>` sense `operator[]`**: s'usa `forEach()` amb lambda `auto` per obtenir el primer ID de cada catàleg.
- **`TierRules::loadFromFile` no és `static`**: cal crear instància `TierRules reglas;` i cridar `reglas.loadFromFile(...)`.
- **`backup → checkout HEAD → afegir → commit → restore`**: per només commital canvis propis a `CMakeLists.txt` sense tocar fitxers d'altres agents en working tree (patró de `Pendiente 33`).
- **`CharacterSheet` per còpia**: cal copiar (`hoja = hojaResult.value()`) per cridar `recalcular()` que modifica `mutable` cached fields.

## Resultats de verificació
```
ctest -R "demo_nova_partida" → 100% tests passed out of 1
Full suite: ctest -L gl-free → 100% tests passed out of 148
```

## Fitxers que no s'han tocat (cruce / altres agents)
- `src/Application.cpp` — cruce, fila d'opencode G-08.
- `src/Game/GameSession.cpp` — G-12, en funcionament.
- `CMakeLists.txt` (canvis d'altres agents en working tree: `test_camera_transition`, `SpeedZeroFlagTests`, `ApplicationCableadoTests`, etc.) — preservats via backup/restore.

## Próxima tasca del GANTT
El proper G-task programable sense cruce és complex: G-09 (Nova partida wiring a Application), G-10 (Opcions), G-11 (SaveLoadScene), G-13/G-14 (comandes de combat / HUD), tots depenen de Application.cpp o de tasques WIP (D-06, D-10, E-16). Les pròximes demos GL-free possible serien:
- `demo_pause_menu` (G-10) — `PauseMenu` ja és GL-free (`include/Game/PauseMenu.h`).
- `demo_inventory` (G-12) — `InventoryScreen` ja és GL-free.
