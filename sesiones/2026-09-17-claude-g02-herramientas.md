# 2026-09-17 · Claude (Cowork) · G-02: cuentagotas, rectángulo y línea (mitad sin interfaz)

**Encargo de Oriol:** seguir con el Gantt usando `MAPA_DEL_CODIGO`, teniendo en cuenta a los demás agentes. Viene después de la colisión en E-14 ([sesión](2026-09-17-claude-e14-colision.md)).

## Elección

Lo que queda sin registro está bloqueado por decisiones (F-01..F-04 por DEC-06, G-01 por DEC-13), depende de paquetes abiertos (G-04/05/06/12/15/16) o lo lleva ya otra sesión (G-13, E-16). Queda **G-02**. Según el mapa, sus rutas son `EditorState.*` y `level_editor.cpp`. `EditorState.*` no tenía WIP y la fila de codex de E-12 la deja expresamente fuera. `level_editor.cpp` y el shell se quedan sin tocar.

## Qué se hizo

Fila en el TABLERO antes de empezar. Desarrollo en una copia aislada de `d5ec681`, con las inserciones escritas como script anclado. Antes de aplicarlo al árbol se comprobó que `EditorState.*` y `CMakeLists.txt` seguían iguales que HEAD, y antes del commit, que el árbol coincidía con lo verificado.

MG `f556039`:
- `EditorState::paintRect`, `paintLine` y `pickTileAt`, más `RectCells`/`LineCells` estáticas (detalle en `planificacion/seguimiento/G-02.json`).
- `tests/Editor/EditorToolsTests.cpp` + `cmake/EditorToolsTests.cmake` (`test_editor_tools`), una línea `include` en `CMakeLists.txt` y los dos ficheros nuevos añadidos a la entrada G-02 de `dades.py`.

## Verificación

- 8/8 contra `d5ec681` aislado, contra el árbol vivo y contra el árbol commiteado. 8/8 con ASan+UBSan.
- Las 8 mutaciones se detectan.
- `test_editor_state` 10/10 y `test_editor_state_history` 5/5 siguen verdes; `verificar_controles` (legacy y `--shell`) y `comprobar_declaraciones_rpg` también.
- **Sin verificar:** CMake real y CI. La VM no tiene cmake ni red; se compiló con g++ 11 y el glm de `out/build/ci-sim/_deps`.
- El commit se hizo por plumbing (índice temporal + `update-ref` con valor antiguo), sin tocar lo preparado por otros. Los locks vacíos se renombraron; el repo quedó sin ninguno.

## Para quien siga

- **E-10 (codex):** falta cablear en `EditorActions` las herramientas Cuentagotas/Rectángulo/Línea, el arrastre con vista previa (`RectCells`/`LineCells`) y los atajos. No se tocó `EditorTool`, para no mover el verificador de controles.
- **HEAD no configura en un clon limpio:** `CMakeLists.txt` hace `include(cmake/ProjectDatabaseTests.cmake)`, y ese fichero sigue sin versionar (E-16 en vuelo). No es mío; que lo cierre su dueño al commitear.
- **`pla/seguiment.py` sigue en rojo** por registros ajenos: `DEC-02.json` y `E-16.json` sin `id`, `G-13.json` sin `actions`.
- **`MAPA_DEL_CODIGO.md` no se regeneró:** tiene cambios ajenos sin commitear. La próxima regeneración recogerá los dos ficheros nuevos de G-02 desde `dades.py`.
