# Verificación y demos

## Suite automática GL-free

`ctest --label-regex gl-free` ejecuta una suite sin ventana ni contexto
OpenGL. Es la única suite que CI ejecuta en Linux, Windows y macOS.

Se compila con un solo objetivo:

```sh
cmake --build build --target suite_gl_free
ctest --test-dir build --output-on-failure --label-regex gl-free
```

`suite_gl_free` lo arma CMake con todo lo registrado por
`add_motor_gl_free_demo_test` / `add_motor_unit_test`, así que **añadir un
test no obliga a tocar el workflow de CI**. Antes la lista estaba escrita a
mano en el YAML y varios tests ya registrados en CTest nunca llegaban a
compilarse allí.

| Target | Clase |
|---|---|
| `demo_resource_manager` | Smoke: carga y caché de recursos; su fixture se escribe en build. |
| `demo_handle_pool` | Smoke: pool generacional. |
| `demo_occlusion_rules` | Smoke: reglas puras de oclusión. |
| `demo_camera` | Smoke: cámara e isometría con GLM. |
| `demo_tilemap` | Smoke: carga TMX y culling; GLAD se enlaza, pero no se abre contexto. |
| `demo_editor_state` | Smoke: editor, exportación y round-trips TMX/JSON. |
| `test_handle_pool` | Unit: handles inválidos, reciclaje y `clear()`. |
| `test_strong_id` | Unit: IDs tipados, generador inyectable y referencias estables. |
| `test_application_loop` | Unit: ciclo eventos/input/update/render, timestep fijo, límite de pausas y salida. |
| `test_input_system` | Unit: snapshots de acciones, flancos, remapeo y pérdida de foco simulada. |
| `test_scene_manager` | Unit: carga, reemplazo/pila diferidos, MapId y preservación ante error. |
| `test_world` | Unit: handles generacionales, altas/bajas diferidas y orden de actualización. |
| `test_editor_state` | Unit: edición/undo, capas y filtrado editorial. |
| `test_world_topology` | Unit: recorrido del mundo por las puertas; regresión de la referencia colgante. |
| `test_project` | Unit: identidad, rutas lógicas y esquema de `Project`. |
| `test_project_serialization` | Unit: JSON canónico de proyecto/mapa, errores y guardado atómico. |
| `test_map_document` | Unit: capas tipadas, dimensiones, orden e invariantes del documento. |
| `test_runtime_tilemap` | Unit: compilación documental y overrides de partida. |
| `test_tmx_importer` | Unit: importación TMX, colisión y errores informados. |
| `test_map_round_trip` | Integración: TMX → documento → JSON → runtime frente a fixture revisable. |
| `test_save_game` | Unit: formato del guardado, escritura atómica, versión de esquema y round-trip de una `GameSession`. |

Ejecutar localmente:

```sh
cmake --preset macos-debug
cmake --build --preset macos-debug
ctest --test-dir out/build/macos-debug --output-on-failure --label-regex gl-free
```

### Guardado de partida (RUN-10)

`test_save_game` cubre las cuatro piezas del paquete: el DTO y su JSON
(`serializeSaveGame`/`parseSaveGame`), la escritura atómica
(`writeSaveGameFile` escribe en `<path>.tmp` y renombra: un corte a mitad
deja intacto el guardado anterior), el rechazo de un esquema con otra
versión, y `GameSession::captureSaveGame`/`restoreSaveGame`.

Restaurar es **todo o nada**: si el nivel cambió desde que se guardó, la
sesión se niega con un motivo en vez de aplicar el estado a medias. La
sesión no hace I/O — quien carga el nivel y llama a `restoreSaveGame` es
la Application, igual que con `LevelTransition`.

En el juego son **F5** (guardar) y **F9** (cargar) sobre `savegame.json`,
solo en exploración. El orden que sigue `Application::loadGame` importa y
está comentado en el código: primero se restaura el `NarrativeState`,
después se carga el nivel (que dispara el beat de entrada) y por último se
aplica el resto del guardado. Al revés, cargar una partida repetiría el
prólogo. Esa parte no la cubre CTest porque necesita ventana; se prueba
desde `juego`.

### Bucle de aplicación (RUN-01)

`test_application_loop` no abre una ventana: inyecta servicios de plataforma,
entrada, actualización y render en `Engine::ApplicationLoop`. Verifica que el
input se procesa una vez por frame, la simulación avanza a 60 Hz, una pausa se
limita y no deja deuda ilimitada, y una solicitud de salida detiene el frame
antes de actualizar o renderizar. `juego` adapta GLFW a ese mismo contrato.

### Entrada (RUN-02)

`test_input_system` convierte estados de teclado simulados en acciones de
juego. Cubre `pressed`/`held`/`released`, bindings editables y la liberación
de acciones al perder el foco, sin depender de GLFW ni de un dispositivo real.

### Escenas (RUN-03)

`test_scene_manager` valida que una escena entrante cargue antes de sustituir
la anterior, que un fallo deje la pila sin cambios y que pausa/menú puedan ser
overlays. Las escenas de mundo exponen `MapId`; los overlays no tienen mapa.

### World (RUN-04)

`test_world` comprueba que un spawn no sea visible hasta el flush, que una
baja durante iteración no vuelva a actualizarse y que un handle anterior no
resuelva tras reutilizar su slot. El World no incluye código de render.

En otros sistemas se puede sustituir el preset por
`cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug` y usar `build` como
directorio de pruebas.

### Demos promovidos a CTest (RUN-11.3)

Además de los cinco de la línea base, la suite ejecuta los demos GL-free
que cubren el vertical slice completo: `demo_level_loader`,
`demo_object_catalog`, `demo_catalogos_rpg`, `demo_skills`, `demo_battle`,
`demo_battle_nd6`, `demo_dice_engine`, `demo_nd6_distribution`,
`demo_enemy_brain`, `demo_game_session`, `demo_object_inspector`,
`demo_creacion_personaje`, `demo_mundo`, `demo_ciudad`, `demo_aventura`,
`demo_temporizadores`, `demo_narrativa_nd6`, `demo_narrativa_combate`,
`demo_prologo`, `demo_boundington`, `demo_precuela_boundington`,
`demo_campana_boundington`, `demo_senal_ceniza`, `demo_este_norte` y
`demo_este_sur`.

Criterio para promover uno: sale con código 0 y **dos ejecuciones seguidas
dan exactamente la misma salida**. Los que tiran dados usan `ScriptedRng` o
semilla fija. Todos leen `assets/` con rutas relativas al cwd, de ahí que
el `WORKING_DIRECTORY` de las pruebas sea el directorio de build.

`demo_proyectos` se queda fuera: además de `assets/` necesita
`tools/build_proyecto.py` en el cwd (compila un proyecto de verdad desde
la pantalla del hub) y CMake solo copia `assets/`. Promoverlo pide antes
decidir si `tools/` se copia también, que es TOOL-09.

## Demos que requieren pantalla

Estos targets enlazan `motor_core` y deben ejecutarse con un contexto
OpenGL real: `sandbox_window`, `demo_textured_quad`,
`demo_isometric_renderer`, `demo_animated_entity`, `demo_lighting`,
`demo_fog`, `demo_lut`, `demo_hud`, `demo_battle_hud`,
`demo_dynamic_lights`, `level_editor` y `juego`.

No se ejecutan en CTest. En Linux se validan localmente mediante Xvfb o
una sesión gráfica; en macOS y Windows, desde una sesión de escritorio.

## Otros demos sin ventana

De los candidatos headless que había aquí solo queda `demo_proyectos`, y
por el motivo explicado más arriba (necesita `tools/` en el cwd). Los
demás están promovidos a la suite automática.

`demo_bitmap_font`, `demo_hud_update` y `demo_enemy_sprite` son casos
especiales: enlazan GLAD para resolver símbolos de render, aunque sus
rutas verificadas no abren un contexto. Permanecen fuera de la suite
automática hasta aislar sus dependencias de enlace.
