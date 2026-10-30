# Revisión del trabajo sin commitear — 20/09/2026

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe la revision del trabajo sin commitear de ese dia, foto del 20/09/2026. El trabajo en vuelo se mira en `git status` y en las filas vivas de `TABLERO.md`; este expediente ya resolvio lo suyo. De las 32 citas de codigo que contiene, 15 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

**Quién:** sesión Cowork de Claude sobre `oriol-os-local`, por encargo de Oriol.
**Qué se revisó:** los 66 ficheros que `git status` daba como sucios o sin
trackear en `feature/editor-s1-contratos`, más el estado de HEAD.
**Cómo:** análisis sintáctico con `g++` de todo `src/`, `editor/src/`, `tests/`
y `examples/`; compilación y ejecución a mano de nueve baterías GL-free (sin
CMake, que no está en esta máquina); lectura de los diffs uno a uno.

---

## 1. Lo urgente: HEAD no configura, y lleva así seis commits

`CMakeLists.txt:1113` de HEAD hace `include(cmake/AventurasCatalogNetTests.cmake)`
y **ese fichero no está en HEAD**: sigue sin trackear en el árbol de Oriol.

```
$ git archive HEAD | tar -x -C /tmp/limpio     # clon limpio, sin árbol de trabajo
$ grep -n AventurasCatalogNetTests /tmp/limpio/MotorGraphico/CMakeLists.txt
1113:include(cmake/AventurasCatalogNetTests.cmake)
$ ls /tmp/limpio/MotorGraphico/cmake/AventurasCatalogNetTests.cmake
No such file or directory
```

Lo introdujo `920c90f` (G-13, 19/09) y encima han caído **seis commits más**.
Cualquiera que clone el repositorio se queda en `cmake --preset`, antes de
compilar una sola línea. Es exactamente la pregunta **0.3** del
`CHECKLIST_CIERRE_TAREA.md` — «un `include(cmake/X.cmake)` subido sin el
`.cmake`» — y es la cuarta vez que pasa.

**Consecuencia que importa:** la CI está roja en el paso de configuración, en
las tres plataformas. Todos los paquetes que dicen «cerrado localmente, CI
pendiente» —C-05, C-09, C-10, C-11, C-13…— no pueden cerrar, y no es culpa
suya.

**Arreglo:** `git add MotorGraphico/cmake/AventurasCatalogNetTests.cmake` y
commit. Un minuto.

## 2. Lo segundo: `motor_editor` no compila con g++ en Linux

`editor/src/ProblemsPanel.cpp:316` llama a `std::strncpy` y el fichero no
incluye `<cstring>` (tiene `<cstdio>` y nada más). Con clang/libc++ en macOS
cuela porque otra cabecera lo arrastra; con g++ en Ubuntu, no:

```
editor/src/ProblemsPanel.cpp:316:22: error: 'strncpy' is not a member of 'std'
```

`cmake/MotorEditor.cmake:14` mete ese fichero en el ejecutable `motor_editor`,
y la CI construye **todo** (`cmake --build build -j`) antes de pasar `ctest`,
en `ubuntu-latest` con `g++` y `clang++`, con `xorg-dev` y `libgl1-mesa-dev`
instalados — o sea que sí, el editor se construye allí.

Está así en HEAD: no lo ha traído el trabajo sin commitear. **Arreglo: una
línea**, `#include <cstring>`.

## 3. El trabajo sin commitear: tres grupos, y está bien

De los 66:

- **9 ficheros son solo ruido.** Su diff entero es el banner que genera
  `tools/marcar_paquetes.py --marcar` (`LayerPanel.h/.cpp`,
  `CatalogPickerState.h`, `CommandListState.h`, `InventoryMenuState.h`,
  `TitleMenuState.h`, `ProjectSettings.h`, `EditorToolsTests.cpp`,
  `CatalogLoaderRegressionTests.cpp`). Se pueden commitear aparte y de una vez.
- **39 ficheros con cambio real**, en tres grupos coherentes.
- El resto son documentos de planificación y las pruebas nuevas.

### Grupo A — Cableado de `Application` (fase H0), 19/09

`Application.cpp` +73/-5, `Application.h` +12, `InputMap.h` +1 (acción `Pause`),
`InputSystem.cpp` +1, `GameSession.cpp` +6.

Hace dos cosas de la fase H, y las hace bien: **H0-04**, cargar
`equipment.json` y pasarle a la sesión las reglas de inventario
(`setInventoryRules`) —sin eso los 391 objetos del manifiesto quedan
inertes—, y **H0-03**, ESC abre la pausa en vez de cerrar el juego, con
`requestPause()`/`requestResume()` delegando en `GameSession::pause()/resume()`
y el menú de pausa visible solo en ese modo. La pista del HUD ya lo anuncia:
`WASD MOVER TAB OBJ ESC PAUSA F5 GRABAR F9 CARGAR`.

Comentado con el porqué, al estilo de la casa. **Yo lo commitearía.**

### Grupo B — Comandos de sesión y ranuras (E-04 + G-11)

`SessionCommandHost.cpp` +115, su cabecera +15, `EventCommands.cpp` +19,
`ConditionEngine.cpp` +9 (`has_speed_zero`, D-11).

Añade los cuatro comandos de ranura (`setSlot`, `saveToSlot`, `loadFromSlot`,
`clearSlot`) con DEC-09 respetada —negarse a guardar en combate o diálogo con
el motivo escrito— y `SaveSlots` opcional: si no está cableado, el comando
falla con un error útil en vez de reventar.

**Sus pruebas pasan: 21 de 21.**

### Grupo C — Editor (E-12, G-02, E-05/E-06)

`event_editor.cpp` +85, `EditorActions.cpp` +70 y su cabecera +37,
`EditorState.cpp` +40, `EditorDocumentSession.cpp` +29,
`EventEditorState.h` +13, `EditorState.h` +4.

Herramientas de línea/rectángulo/cuentagotas con trazo cancelable, orden de
capas de baldosas y páginas de evento. **Sus pruebas pasan: 15 + 8 + 8 + 10 + 7.**

## 4. ¿Compila? ¿Pasan las pruebas?

Con el árbol sucio tal cual está hoy:

| Qué | Resultado |
| --- | --- |
| `src/` + `editor/src/` (94 fuentes) | 93 pasan · falla `ProblemsPanel.cpp` (punto 2, es de HEAD) |
| `tests/` (105 ficheros) | 105 pasan |
| `examples/` (52 ficheros) | 51 pasan · `demo_resource_manager` necesita un define que pone CMake |
| HEAD limpio, `src/` (87 fuentes) | 86 pasan · `Audio.cpp` solo necesita el include de miniaudio que pone CMake |

Nueve baterías GL-free compiladas y **ejecutadas** a mano: `test_speed_zero_flag`
4/4, `SessionCommandHostTests` 21/21, `ProjectDatabaseTests` 11/11,
`EditorToolsTests` 8/8, `EditorStateTests` 10/10, `EventEditorTests` 7/7,
`EditorActionsTests` 15/15, `EditorLayerTests` 8/8, `CameraTransitionTests` 7/7.
**91 pruebas, 0 fallos.**

No he podido ejecutar `test_application_cableado`: linka `motor_core` y pide
contexto GL.

## 5. Tres trampas si se commitea esto tal cual

1. **`CMakeLists.txt` lleva dos `include()` nuevos de ficheros sin trackear**
   (`ApplicationCableadoTests.cmake`, `SpeedZeroFlagTests.cmake`). Si se
   commitea el `CMakeLists.txt` sin ellos, se repite el punto 1. Van en el
   mismo commit o no van.
2. **`test_application_cableado` está etiquetado `gl`**, y la CI corre
   `ctest -L "gl-free"`. La prueba del cableado de `Application` —justo lo que
   hay que proteger— no se ejecuta nunca en CI.
3. **`DEC-02.json` rompe el generador del plan.** No tiene clave `id`, así que
   `planificacion/pla/seguiment.py` levanta `ValueError` y no se puede
   regenerar ni el Gantt ni `MAPA_DEL_CODIGO.md`. Lo escribió «Eigent User» el
   17/09. `DEC-02` tampoco está en `dades.py`: hay que decidir si se da de alta
   o si ese registro se archiva.

## 6. Lo que propongo, por orden

1. `git add` del `.cmake` que falta. **HEAD vuelve a configurar.** (1 min)
2. `#include <cstring>` en `ProblemsPanel.cpp`. **La CI de Linux vuelve a
   construir.** (1 min)
3. Commitear el WIP en cuatro commits: banner generado / grupo A / grupo B /
   grupo C, cada uno con sus `.cmake`.
4. Cambiar la etiqueta de `test_application_cableado` a `gl-free` si puede
   correr sin ventana, o añadirla al trabajo que sí corre con `xvfb` en CI.
5. Decidir sobre `DEC-02.json`.
6. Y entonces sí, la guardia: la línea base del core, para que el punto 1 no
   pueda volver a pasar sin que nadie se entere.

---

*Nada de esto se ha tocado. El único fichero compartido que esta sesión ha
modificado es `tools/analizar_duplicados.py`, una palabra: `entregas` en
`EXCLUIR_PARTES`, para que la carpeta de `tools/exportar.py` no ensucie el
informe con 134 duplicados falsos.*
