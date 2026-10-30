# Sesión 2026-09-17 — cursor — G-02 shell (herramientas)

## Qué
Cableado en el shell GL-free de las tres herramientas cuyo núcleo ya estaba en `EditorState` (`f556039`): Cuentagotas, Rectángulo y Línea.

## Por qué esta y no otra
TABLERO saturado (E-08/E-13/E-15/E-09/reparación HEAD). G-02 shell figuraba libre; MAPA_DEL_CODIGO apunta el arrastre/atajos a `EditorActions`, no a `EditorState`. Sin tocar filas ajenas ni `EditorApplication.cpp`.

## Zona
- `include/Editor/EditorActions.h` / `src/Editor/EditorActions.cpp`
- wrappers en `EditorDocumentSession` (`paintRect` / `paintLine` / `pickTile`)
- `tests/Editor/EditorActionsTests.cpp`
- `planificacion/seguimiento/G-02.json`

## Verificación
- `test_editor_actions` **15/15**
- `test_editor_tools` **8/8**, `test_editor_document_session` **9/9**
- `verificar_controles.py --shell` OK (17 acciones)

## Pendiente
Arrastre ImGui en `EditorApplication` (E-10): begin/update/commit + dibujar `previewCells()`. Sin commit en esta sesión (no pedido).
