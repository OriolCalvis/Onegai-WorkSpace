# 2026-09-19 — claude — G-08.1 demo GL-free de carrega de manifests

## Objectiu
Verificar GL-free el motor de títol de G-08 (`TitleMenuState`) contra manifests
reals del projecte. G-08 ja té el nucli verd (`test_title_flow` 7/7), però no
hi ha cap demo GL-free que carregui manifests reals i verifiqui la lògica del
menú. Aquest demo comprova:

- TitleMenuState::fromManifest amb manifests reals (`boundington.json`,
  `ejemplos.json`)
- El títol cau en project.name quan settings.title és buit
- L'entrada (entryPath) apunta a un fitxer real
- Menús: Continue deshabilitat sense guardats, NewGame habilitat si l'entrada
  existeix, Opciones, Salir
- Manifest invàlid: error en paraules, només Salir (enabled), no cai en
  Boundington
- Navegació: moveDown/moveUp amb wrap, confirm() sobre habilitat/deshabilitat

## Fitxers creats / modificats
- **Nou:** `MotorGraphico/examples/demo_title_menu.cpp` (112 línies, GL-free)
- **Modificat:** `MotorGraphico/CMakeLists.txt` (+16 línies: add_executable amb
  TitleMenuState.cpp + SaveSlotState.cpp compilats directament; +1 línia:
  add_motor_gl_free_demo_test)
- **Modificat:** `MotorGraphico/planificacion/MAPA_DEL_CODIGO.md` (+demo, 53→54)
- **Modificat:** `TABLERO.md` (fila "En curs" → "Hecho")

## Concurrencia
- `TitleMenuState.cpp` i `SaveSlotState.cpp` són G-08/G-11 (un sol paquet), ja
  comès/vert. Es compilen directament en el demo (mateix patró que
  `cmake/TitleFlowTests.cmake`), sense tocar cap librería compartida.
- `Application.cpp` és cruce (fila d'opencode, Pendiente 34): NO es tocado.
- `TitleScene` és GL: NO es tocat.
- `CMakeLists.txt` té canvis no commesos d'altres agents (test_camera_transition,
  ApplicationCableadoTests.cmake, SpeedZeroFlagTests.cmake). Tècnica
  backup→checkout HEAD→afegir→commite→restore per només commital els propis.

## Verificacions
- Build: `cmake --build build/ --target demo_title_menu -j8` → 0 errors
- ctest: `ctest -R demo_title_menu` → **Passed (Test #43, 0.01s)**
- Suite GL-free: 142 objectius (97 + test_title_menu + demo_title_menu + altres),
  100% passats

## Commits
- MotorGraphico: `5b3659e` (demo + CMake), `be317d4` (MAPA_DEL_CODIGO)
- Software root: pendent (TABLERO + sessions)
