# Sesión 2026-09-17 — cursor — E-12 orden de capas (GL-free)

## Qué
Mitad sin interfaz del tramo «ordenar capas» de E-12: `EditorState::moveLayer` + `EditorDocumentSession::moveTileLayer`.

## Por qué
TABLERO: E-12 liberada; ImGui (`LayerPanel`/`EditorApplication`) ocupada o compartida. `MapDocument::moveLayer` ya existía; faltaba el puente del shell documental con historial.

## Zona
- `EditorState` / `EditorDocumentSession` / `tests/Editor/EditorLayerTests.cpp`
- Sin `LayerPanel`, sin `EditorApplication`

## Verificación
- `test_editor_layers` **8/8**
- `test_editor_actions` **15/15** (G-02 shell intacto)

## Pendiente
Botones ImGui subir/bajar; formularios de mapa e instancias.
