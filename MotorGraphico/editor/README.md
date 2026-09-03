# Aplicación de editor

`src/level_editor.cpp` contiene la aplicación visual transitoria del
editor. Consume los modelos y servicios GL-free de `include/Editor/` y
`include/Project/`; no introduce reglas documentales propias.

EDT-01 promoverá este punto de entrada a `motor_editor` mediante
`EditorApplication`. EDT-02 migrará el shell a Dear ImGui conforme a
[`ADR-001`](../architecture/ADR-001-editor-ui.md). Mientras tanto el
target CMake conserva el nombre `level_editor` para no romper los flujos
de ejecución existentes.
