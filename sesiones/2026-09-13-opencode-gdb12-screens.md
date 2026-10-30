# Sesión 2026-09-13 — opencode: GDB-12.2/12.3/12.4 Pantallas de juego (Pause, GameOver, Inventario)

## Resumen
Implementación de tres pantallas de juego GL-free siguiendo el patrón de TitleScreen:
- **PauseMenu** (GDB-12.2): 8 opciones con navegación wraparound
- **GameOverScreen** (GDB-12.4): 2 opciones (Carregar, Tornar al títol)
- **InventoryScreen** (GDB-12.3): 4 tabs (Inventario, Equipo, Estado, Acciones) con navegación por tabs y lista

## Archivos creados
- `include/Game/PauseMenu.h` / `src/Game/PauseMenu.cpp`
- `include/Game/GameOverScreen.h` / `src/Game/GameOverScreen.cpp`
- `include/Game/InventoryScreen.h` / `src/Game/InventoryScreen.cpp`
- `tests/Game/GDB12ScreenTests.cpp` (7 tests)

## Detalles técnicos
- **Arquitectura GL-free**: igual que TitleScreen, estado puro sin GL, testeable
- **PauseMenu**: 8 opciones (Reprender, Personaje, Inventario/Equip, Desar, Carregar, Opcions, Tornar al títol, Sortir) con wraparound
- **GameOverScreen**: 2 opciones (Carregar, Tornar al títol) con wraparound
- **InventoryScreen**: 4 tabs (Inventario, Equipo, Estado, Acciones) con navegación por tabs (Izq/Der) y dentro de la lista (Arriba/Abajo)
  - Constructor por defecto + `rebuildEntries()` para testing sin GameSession
  - `InventoryFocus::COUNT` para navegación correcta de 4 tabs
  - Soporte para GameSession opcional (puntero nulo en testing)
  - Entry types: InventoryAction (UseItem, EquipItem, ViewStats, etc.)
- **Tests** (7/7 pasando):
  - `pause_menu_navigation`, `pause_menu_confirm`
  - `game_over_screen_navigation`, `game_over_screen_confirm`
  - `inventory_screen_basic_navigation`, `inventory_screen_entries`, `inventory_screen_status_labels`

## Fixes aplicados
- `InventoryFocus::COUNT` para navegación correcta de 4 tabs (fix wraparound)
- Constructor por defecto llama a `rebuildEntries()` para testing
- `m_session` como puntero opcional (nullptr en testing)
- Fix `moveTabRight`/`moveTabLeft` usando `InventoryFocus::COUNT`
- Métodos de InventoryScreen comprobando `m_session` antes de acceder

## Verificación
```
ctest -L gl-free
# 105/105 tests passing
./test_gdb12_screens
# 7/7 tests passing
```

## Integración CMake
- Añadidos `PauseMenu.cpp`, `GameOverScreen.cpp`, `InventoryScreen.cpp` a `motor_level`
- Nuevo target `test_gdb12_screens` linkando `motor_level`
- 105/105 GL-free tests passing (incluyendo nuevos 7 tests)

## Archivos modificados (commit c2b8ace)
- `MotorGraphico/include/Game/PauseMenu.h` (nuevo)
- `MotorGraphico/include/Game/GameOverScreen.h` (nuevo)
- `MotorGraphico/include/Game/InventoryScreen.h` (nuevo)
- `MotorGraphico/src/Game/PauseMenu.cpp` (nuevo)
- `MotorGraphico/src/Game/GameOverScreen.cpp` (nuevo)
- `MotorGraphico/src/Game/InventoryScreen.cpp` (nuevo)
- `MotorGraphico/tests/Game/GDB12ScreenTests.cpp` (nuevo)
- `MotorGraphico/CMakeLists.txt` (+ fuentes + test)
- `MotorGraphico/include/Game/GameSession.h` (+ getters playerBody/playerSkills/hasCharacterSheet/characterSheet/inventory)
- `MotorGraphico/include/Game/InventoryScreen.h` (+ default constructor, m_session como puntero)
- `MotorGraphico/src/Game/InventoryScreen.cpp` (+ default constructor, rebuildEntries, null checks)

## Zonas respetadas
- codex: E-10 (editor), C-23, C-16
- zcode: C-16, F1, D-02, D-04
- opencode (yo): RPG engine (src/RPG/, include/RPG/, tests/RPG/, src/Game/ screens)