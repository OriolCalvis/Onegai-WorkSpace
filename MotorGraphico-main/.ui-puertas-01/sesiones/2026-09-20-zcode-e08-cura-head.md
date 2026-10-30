# 2026-09-20 · zcode · E-08: cura del rojo de HEAD del intérprete de eventos + cobertura versionada

**Dónde:** `MotorGraphico-main` (fichero de mi fila E-08 del TABLERO).
**Commits:** MG `0ba7155` (único, quirúrgico por pathspec sobre `tests/RPG/EventInterpreterTests.cpp`).

## Qué se hizo

Oriol pidió seguir el Gantt del motor. Mi fila viva del TABLERO es **E-08 — depuración paso a paso (núcleo GL-free)**, que en el Gantt de paquetes F0–F8 es EVT-15. El estado real al empezar: la entrega GL-free de la fila (test de equivalencia paso/golpe) ya existía en el árbol, pero **HEAD estaba roto por mi propio fichero** (Pendiente 33.1: 9 errores `unknown type name 'NarrativeState'` desde `4720b62`/EVT-05), y el arreglo flotaba sin commitear junto a un bloque de cobertura con defectos.

1. **Verificación en clon aislado** de `be317d4` ( HEAD ) + solo mi fichero. El clon limpio no configuraba: `CMakeLists.txt:1113` incluye `cmake/AventurasCatalogNetTests.cmake`, que está **sin versionar** (zona de opencode, 19/09, sin luz verde). Copiado LOCALMENTE al clon desechable de /tmp para poder configurar; **no** se ha commiteado trabajo ajeno.
2. El bloque de cobertura hallado en el árbol tenía **tres defectos reales**, corregidos antes de versionar:
   - dos nombres de test **con espacios** rompían la macro `MOTOR_TEST_CASE`;
   - `cancel_all_...` asumía que un `resultadosPreparados` sirve a varias ejecuciones, pero `HostDePrueba` lo consume al primer uso → dos esperas vivas necesitan dos tipos de comando (`espera`/`espera_b`);
   - `start_sin_id...` llamaba `.value()` sobre un `Result` en Error (el guardia lo aborta por diseño) → ahora `EventId{}` vacío, que es exactamente lo que `start()` rechaza (`EventInterpreter.cpp:35`, «no se puede ejecutar un evento sin id»).
3. **Semántica verificada, no apañada:** `evt05_condicion_local_switch_negado` contradecía el código comprometido. `EvaluateCondition` aplica `negated` al final (`EventCommands.cpp:353`), así que `!=` + `negated` es doble negación → `true`. Revisados `FORMATO_AVENTURAS.md` y `PLA_DEFINITIU.md`: ninguno fija otra semántica para `negada` → el test corregido documenta el contrato real; sin discrepancia que registrar.
4. **Commit `0ba7155`** con git-safe.sh. El Pendiente 33.1 queda resuelto.

## Verificación

- Clon limpio `be317d4` + mi fichero: `test_event_interpreter` **38/38**, `test_event_debug` **9/9**.
- Suite GL-free entera del clon (`-k` + `ctest -L gl-free`): **137/140 pasan**. Los 3 caídos son binarios no construidos, todos ajenos y con su arreglo ya sentado en el árbol sin commitear:
  1. `test_e09_bridge` — el test y el `.cmake` de qwen **sí están versionados** (los barrió `ed84b18` «G-09») pero `addStructuredCommand` (la implementación, `EventEditorState.h`) sigue solo en el árbol. Nota añadida a su fila del TABLERO.
  2. `test_editor_state` — el 33.2 conocido (`f23dba9`).
  3. `test_animated_entity` — **rojo nuevo descubierto hoy**: `ed84b18` cambió el constructor de `AnimatedEntity.h` y la actualización de su test quedó sin versionar (`M` en el árbol).

## Coexistencia

- Nada del WIP ajeno se ha tocado ni commiteado: `EventCommands.cpp`, `EventEditorState.h`, `AnimatedEntityTests.cpp` y `AventurasCatalogNetTests.cmake` siguen exactamente como estaban.
- Registro: anexo `planificacion/seguimiento/E-08-cura-head.json` (seguiment.py sin quejas), nota con fecha en las filas E-08 y E-09 del TABLERO, Pendiente 33 actualizado con la medición de hoy.

## Pendiente propio

- La mitad GL de E-08 (panel del depurador en `editor/src/event_editor.cpp`) sigue bloqueada por el WIP vivo de E-09 en ese fichero.
- CI remota: pendiente del push que solo Oriol puede autorizar.
