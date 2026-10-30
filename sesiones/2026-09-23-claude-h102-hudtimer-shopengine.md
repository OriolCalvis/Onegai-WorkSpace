# Sessió 2026-09-23 — claude (Cowork) — H1-02: HudTimer + ShopEngine a producció

## Tasca
**H1-02 (Gantt Fase H): G-14 HudTimer i ShopEngine a producció** (rescate, color #E3A82B).
Cobert: "Una tienda real amb stock i preus des de dades; els temporitzadors de missió es veuen en el HUD".

Clau de tancament GL-free (primera meitat): `HudTimer`, `ShopItem`, `ShopTransaction` i
`ShopEngine` vivien només dins `tests/Game/G14HudTimersAndCommerceTests.cpp` (388 línies).
Aquesta sessió els mou a producció. El cablejat GL al HUD real (H3-04, H3-06) és tasca separada.

## Fitxers d'entrada
- `tests/Game/G14HudTimersAndCommerceTests.cpp` (388 línies, 6 casos) — conté les definicions inline
  de `HudTimer`, `ShopItem`, `ShopTransaction`, `ShopEngine` a moure.

## Estat previ (a l'Arbre)
- `H1-01` (G-13): HECHO per zcode (commits `9eeb27b` + `43cc272`).
- `H1-03` (G-10): HECHO per zcode (commit `71317fb`).
- TABLERO: cap fila d'H1-02 reclamada. Lliure.

## Estat del repositori
- MotorGraphico HEAD: `71317fb` ("H1-03: MenuLogic a producció + Game::OptionsState").
- Software root HEAD: `47d9cf7c` (commiat de la sessió G-09.1).
- Altres agents amb WIP al mateix temps:
  - `opencode`: H0-01, H0-02, H0-05, H0-07 — `Application.cpp`, `InputSystem`, `ProjectIndex`, etc.
  - `zcode`: H1-01 (HECHO), H1-03 (HECHO).
  - `qwen`: E-09, E-15.
  - `codex`: EST-34, EST-35, EST-36, C-23, ALPHA-00.
- WIP a `CMakeLists.txt` d'altres agents: registre de tests `test_camera_transition`,
  `SpeedZeroFlagTests`, `ApplicationCableadoTests`, `ProjectDatabaseTests`, `animated_entity`,
  etc. **No es meu** — espatllat amb backup→checkout→commiar→restaurar.

## Pla d'acció
1. **Claim TABLERO**: H1-02 no estava reclamada a l'Arbre.
2. Extracció de codi del test a producció:
   - `HudTimer` (60.0f default, `HudTimerMode {Countdown, Bar, Secret}`) →
     `include/HUD/HudTimer.h` + `src/HUD/HudTimer.cpp`.
   - `ShopEngine`/`ShopItem`/`ShopTransaction` (buy/sell, moral, stock) →
     `include/Game/ShopEngine.h` + `src/Game/ShopEngine.cpp`.
3. **Reescriure el test**: `G14HudTimersAndCommerceTests.cpp` inclou els headers de producció
   enlloc de definicions inline. Mantén tots els 6 casos però usa `Game::` del motor.
4. **CMakeLists.txt** (motor_level): afegeix `src/HUD/HudTimer.cpp` + `src/Game/ShopEngine.cpp`.
   Patró de backup→checkout→re-aplicar→commiar→restaurar-WIP (no toca `Application.cpp`).
5. **cmake/G14HudTimersAndCommerceTests.cmake**: compila `HudTimer.cpp` + `ShopEngine.cpp`
   directament en l'executable (patró anticollisió G13).
6. **Build + verificació**: `ctest -R g14` + suite GL-free completa.
7. **Documentar**: `MAPA_DEL_CODIGO.md` amb els 4 fitxers nous.

## Acció
- Creat `include/HUD/HudTimer.h` — `namespace Game`, `HudTimer` amb `HudTimerMode{Countdown,Bar,Secret}`,
  mètodes: constructor(60.0f), `setPaused`, `paused`, `tick(dt)→bool`, `reset`, `remaining`,
  `total`, `expired`, `firedCount`, `onExpire(cb)`.
- Creat `src/HUD/HudTimer.cpp` — implementació.
- Creat `include/Game/ShopEngine.h` — `namespace Game`, `struct ShopItem{id,name,price,stock,moralImpact,vendible,requiredLevel}`,
  `struct ShopTransaction{ok,message,goldAfter,moralDelta,itemCountDelta}`, `class ShopEngine`.
- Creat `src/Game/ShopEngine.cpp` — implementació: `buy(id,qty=1,buyerLevel=1)`, `sell(id,qty=1)`,
  gold inicial 300, moral inicial 0.5f, moraleja >=0.05f → +1 (capped 1.0f), refund de venda price/2.
- Modificat `tests/Game/G14HudTimersAndCommerceTests.cpp`: elimina definicions inline
  (línies 37-176), afegeix `#include "Game/ShopEngine.h"` + `#include "HUD/HudTimer.h"`,
  `using namespace Game;`, elimina includes no usats (`Vector4.h`, `HudElement.h`).
- Modificat `cmake/G14HudTimersAndCommerceTests.cmake`: afegeix `src/HUD/HudTimer.cpp` +
  `src/Game/ShopEngine.cpp` a `add_executable`, actualitza comentari.
- Modificat `CMakeLists.txt` (motor_level): afegeix les dues fonts noves (patró backup→checkout).
- Modificat `planificacion/MAPA_DEL_CODIGO.md`: afegeix els 4 fitxers a la llista alfabètica.

## Cicle de commiat (CMakeLists.txt)
1. Backup: `cp CMakeLists.txt /tmp/cmbackup_h102.txt` (conté WIP d'altres agents).
2. `git checkout HEAD -- CMakeLists.txt` (neteja WIP ajó).
3. Edita: afegeix `src/HUD/HudTimer.cpp` + `src/Game/ShopEngine.cpp` a motor_level.
4. `git add` fitxers propis + `git-safe.sh commit -m "H1-02..."`.
5. Restaura backup: `cp /tmp/cmbackup_h102.txt CMakeLists.txt`.
6. Re-aplica canvis H1-02 sobre el backup (3 línies a motor_level).

## Resultats de verificació
- **G14 test** (`test_g14_hud_timers_commerce`): **6/6 PASS** amb headers de producció.
- **Suite GL-free**: **150/150 PASS** (100%, 0 regressions) amb WIP d'altres agents restaurat.
    - Sense WIP: 147 tests (build des de zero).
    - Amb WIP d'altres agents: 150 tests (+test_camera_transition, +SpeedZeroFlag, +ApplicationCableado).
- **motor_level**: compila amb `src/HUD/HudTimer.cpp` + `src/Game/ShopEngine.cpp`.
- **`require()`**: usat en tot moment, mai `assert()` (Release defineix NDEBUG).

## Fitxers modificats (commiats)
MotorGraphico (commit `64033dd`):
- `include/HUD/HudTimer.h` (NUEVO)
- `src/HUD/HudTimer.cpp` (NUEVO)
- `include/Game/ShopEngine.h` (NUEVO)
- `src/Game/ShopEngine.cpp` (NUEVO)
- `CMakeLists.txt` (motor_level: +2 fonts)
- `cmake/G14HudTimersAndCommerceTests.cmake` (+2 fonts a add_executable)
- `tests/Game/G14HudTimersAndCommerceTests.cpp` (rewireat a headers de producció)
- `planificacion/MAPA_DEL_CODIGO.md` (+4 fitxers a la llista)

## Pendents per a futures sessions
- **H3-04** (comercio amb stock i preus reals des de dades): depèn de H1-02.
- **H3-06** (temporitzadors de missió visibles en el HUD): depèn de H1-02.
- **H1-05** (Locale a producció): depèn de H1-03 (zcode).
- **H1-04** (G-09 creació de personatge): ja té `Player::populate_from_class_race_background`
  a producció; la integració és l'cble H0-07 (opencod, HECHO).

## Notes de coordinació
- No s'ha tocat `Application.cpp`, `Application.h` ni cap altre fitxer d'altres agents.
- El commiat a `CMakeLists.txt` només conté canvis propis (5 insercions).
- El backup de `CMakeLists.txt` està a `/tmp/cmbackup_h102.txt` (WIP d'altres agents).
