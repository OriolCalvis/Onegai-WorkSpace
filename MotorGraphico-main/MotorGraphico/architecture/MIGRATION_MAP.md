# Mapa de migración hacia la arquitectura objetivo

Este mapa es la referencia para los movimientos de `INF-03`. Un cambio de
ruta no cambia reglas de dominio, formato ni API observable: se conserva
una cabecera de compatibilidad mientras existan consumidores en la ruta
antigua.

| Ubicación actual | Destino objetivo | Estado y regla de migración |
|---|---|---|
| `include/Core`, `src/Core` | `Core` | Se mantiene. Es infraestructura sin dependencia gráfica. |
| `include/Engine`, `src/Engine` | `Engine` | Se mantiene. Consume runtime/render, no editor. |
| `include/Render`, `src/Render` | `Render` | Se mantiene para OpenGL y proxies visuales. El HUD de juego sigue aquí. |
| `include/Editor/ProjectIndex.h`, `src/Editor/ProjectIndex.cpp` | `include/Project`, `src/Project` | Primera migración: datos de proyecto e índice, sin UI ni GL. |
| `include/Editor/ProjectHub.h`, `src/Editor/ProjectHub.cpp` | `editor/include/Editor`, `editor/src` | Es un controlador/pantalla de editor; depende de Project, pero no pertenece al modelo. |
| `EditorState`, `EditorValidation`, `ObjectInspector` | `editor/include/Editor`, `editor/src` | Núcleo y servicios de autoría. Mantener GL-free y pruebas bajo `tests/Editor`. |
| `LevelDefinition`, `LevelLoader` | `Project` + `Map` | Transición posterior: el formato/documento queda en Project y las capas/tiles en Map. No mezclar con `TileMap` runtime. |
| `ObjectCatalog` | `Database` | Datos configurables de juego; no debe depender de Render. |
| `WorldTopology`, `GameSession`, `EnemyBrain` | `World` | Runtime que consume documentos, nunca editor. |
| `BattleState`, `Skill`, `Morality`, `RPG/*` | `Database` + sistemas de juego | Se separará por ficha GDB, no mediante una mudanza masiva. |
| `examples/` | `examples/` | Vertical slices y pruebas manuales; no contiene APIs de producto nuevas. |
| `tests/` | `tests/` | Pruebas por dominio, independientes de UI y ventana cuando sea posible. |
| `assets/`, `schemas/` | `assets/`, `schemas/` | Datos y contratos; migraciones de formato solo por tareas DOC/MAP. |
| `third_party/` | `third_party/` | Dependencias fijadas según `DEPENDENCY_POLICY.md`. |

## Orden verificable

1. Mover `ProjectIndex` y el dato `Project` a `Project/`, dejando una
   cabecera de reenvío temporal en `Editor/`.
2. Actualizar CMake y los consumidores internos del módulo; compilar
   `demo_proyectos` y la suite CTest.
3. Crear el árbol `editor/` y trasladar servicios de autoría sin tocar el
   runtime. El ejemplo visual conserva su ruta mientras se sustituye por
   `motor_editor`.
4. Retirar las cabeceras de compatibilidad solo cuando ninguna referencia
   incluya la ruta antigua.

## Límites que no se cruzan

- Los documentos (`Project`, `Map`, `Database`) no incluyen `Render`,
  `GLFW`, `glad` ni Dear ImGui.
- El editor llama a servicios documentales; no los duplica ni escribe
  archivos por fuera de esos servicios.
- El runtime consume datos documentales y nunca incluye una cabecera del
  editor.
- Los movimientos de rutas se validan con CTest y con los demos del
  dominio afectado antes de pasar al siguiente paquete.
