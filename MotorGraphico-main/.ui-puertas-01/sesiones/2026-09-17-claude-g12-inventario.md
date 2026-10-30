# 2026-09-17 · Claude (Cowork) · Reparar HEAD + G-12 (núcleo): usar, equipar y desequipar de verdad

**Encargo de Oriol:** seguir con el Gantt usando `MAPA_DEL_CODIGO`, con cuidado por los demás agentes. Va después de [E-04](2026-09-17-claude-e04-comercio.md).

## 1. HEAD volvía a no compilar — MG `fee1e85`

- **El fallo:** `motor_project` incluye `src/Project/ProjectDatabaseState.cpp` (E-16, entró con los commits automáticos de Eigent), que definía `save`/`load` como `Result<void>` mientras la cabecera los declara `bool`. Se alinearon con la cabecera; como siguen sin implementar, devuelven `false`.
- **Barrido de HEAD con `g++ -fsyntax-only`:** de las 73 fuentes de librería solo fallaba esa. También fallan `tests/RPG/EventInterpreterTests.cpp` (E-08, zcode, en curso) y `editor/src/ProblemsPanel.cpp` (E-15, qwen, en curso: le falta `<cstring>`); no se tocaron.
- **Registros de seguimiento:** `tools/normalitza_seguiment.py --escriure` sobre `E-16.json` y `G-13.json`. `DEC-02.json` se dejó como estaba: no es un paquete y `pla/seguiment.py` lo sigue rechazando.

## 2. G-12, núcleo sin interfaz — MG `9a6ecfc` (+ arreglo `f32093d`)

Los tres stubs de GDB-12 (`GameSession::useItem/equipItem/unequipItem`, marcados «TODO G-12») ahora funcionan de verdad, con motivos, vista previa y reglas de equipo. Hay un modelo nuevo, `Game::InventoryMenuState`: categorías, cantidades, acciones con su motivo, ranuras de equipo, selección por identidad y destinatario cancelable. El detalle está en `planificacion/seguimiento/G-12.json`.

**Verificación:**
- `test_inventory_menu` 9/9, también con ASan, con los catálogos publicados.
- De 11 mutaciones se detectan 10; la restante es equivalente.
- Siguen verdes las 7 suites que dependen de `GameSession`.
- Compilan todos los `.cpp` que incluyen `GameSession.h`.
- **Sin verificar:** CMake real y CI.

### ⚠️ Error mío, ya corregido

`9a6ecfc` subió `CMakeLists.txt` con dos cambios en vuelo de otra sesión. Uno era `include(cmake/E09BridgeTests.cmake)`, un fichero **sin versionar**: un clon limpio de HEAD no configuraba. La comprobación previa («¿está limpio?») falló sin que se notara, porque `cond && echo` bajo `set -e` no aborta. Minutos después, `f32093d` dejó `CMakeLists.txt` como en `777f4fa` más la única línea de G-12. Los cambios ajenos siguen en el árbol de trabajo, sin tocar: `include E09BridgeTests` y `EventCommands.cpp` dentro de `motor_rpg`.

**Trampa (para la Parte 3):** en scripts de commit, la guarda tiene que abortar de verdad (`git diff --quiet ... || exit 1`), no ser un `&&` suelto.

## Para quien siga

- **Vista de G-12:** Application/InventoryScene dibuja hoy `Game::InventoryScreen`, que no tiene cantidades, categorías ni motivos. Tiene que leer `InventoryMenuState`.
- **Reglas de equipo:** Application tiene que llamar a `GameSession::setInventoryRules` con el catálogo de equipo, la clase del personaje y las `TierRules`.
- **WIP ajeno en `CMakeLists`:** el dueño de E-09Bridge tiene que versionar `cmake/E09BridgeTests.cmake` en el mismo commit que su `include`. Además, meter `EventCommands.cpp` en `motor_rpg` duplica lo que varios tests compilan a mano; conviene revisarlo.
- **Alguien está editando** `EditorState.*` y `seguimiento/G-02.json` (probablemente la parte de interfaz de G-02): no se tocaron.
