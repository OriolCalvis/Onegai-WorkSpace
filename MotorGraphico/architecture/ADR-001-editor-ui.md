# ADR-001: Dear ImGui vendorizado para la interfaz del editor

**Estado:** Aceptado  
**Fecha:** 2026-08-28  
**Decisor:** Proyecto MotorGrafico

## Contexto

El editor actual funciona, pero `examples/level_editor.cpp` concentra 2.707
líneas de composición de HUD, layout, captura manual de teclado/ratón,
modales y paneles. El modelo (`EditorState`), los servicios de proyecto
(`ProjectHub`/`ProjectIndex`) y el inspector son GL-free, por lo que no
deben conocer la tecnología visual.

El HUD propio (`HudPanel`, `HudText`, `HudCommandMenu`, `HudManager`) es
parte del runtime y sirve bien al juego. Sin embargo, el editor necesita
un árbol de documentos, navegador de assets, búsqueda, controles de texto,
ventanas ocultables y docking. Implementar esos comportamientos en el HUD
extiende la captura de entrada ad-hoc que ya domina el ejemplo actual.

La restricción histórica era no tener red para descargar ImGui. A fecha de
esta decisión se verificó que el remoto oficial de Dear ImGui es accesible.
El árbol `third_party/` ya contiene dependencias vendorizadas; una
dependencia descargada mediante `FetchContent` rompería builds offline y
la reproducibilidad de CI.

## Decisión

Se usará **Dear ImGui vendorizado y con versión fijada** para el shell del
editor. Se integrará únicamente en el ejecutable `motor_editor` y sus
paneles; el runtime, el HUD propio y las bibliotecas GL-free seguirán sin
incluir ni enlazar ImGui.

La primera integración usa GLFW + OpenGL3 sobre el contexto ya existente,
un único viewport nativo y docking. Se desactiva multi-viewport en el MVP.
El layout es local y nunca sustituye datos de proyecto ni de nivel.

## Opciones consideradas

### A. Mantener y ampliar el HUD propio

| Dimensión | Evaluación |
|---|---|
| Complejidad inmediata | Baja |
| Coste al añadir paneles | Alto |
| Adecuación para runtime | Alta |
| Adecuación para herramientas | Baja |
| Dependencia externa | Ninguna |

**Ventajas:** reutiliza código existente y mantiene la compilación sin
dependencias nuevas.

**Desventajas:** docking, árbol, entrada de texto, filtros y modales se
codifican a mano; el ejemplo ya demuestra ese acoplamiento entre UI e
input. Mantenerlo para el editor multiplica código específico que no
aporta funcionalidad al motor.

### B. Dear ImGui mediante FetchContent

| Dimensión | Evaluación |
|---|---|
| Complejidad inmediata | Media |
| Coste al añadir paneles | Bajo |
| Reproducibilidad offline | Baja |
| Adecuación para herramientas | Alta |

**Ventajas:** integración CMake breve y actualizaciones sencillas.

**Desventajas:** exige red durante configure, contradice el requisito de
un checkout autocontenido y hace el resultado dependiente del remoto.

### C. Dear ImGui vendorizado y fijado (elegida)

| Dimensión | Evaluación |
|---|---|
| Complejidad inmediata | Media |
| Coste al añadir paneles | Bajo |
| Reproducibilidad offline | Alta |
| Adecuación para herramientas | Alta |

**Ventajas:** docking, árboles, inputs y modales probados; el checkout y
CI permanecen autocontenidos; el motor no arrastra la dependencia.

**Desventajas:** aumenta el tamaño de `third_party/`; hay que fijar la
versión, conservar la licencia MIT y adaptar el backend GLFW/OpenGL3.

## Consecuencias

- `EditorState`, `ProjectHub`, `ProjectIndex` y `ObjectInspector` siguen
  siendo la fuente de verdad y se prueban sin una ventana.
- El HUD propio no se elimina: continúa como UI de juego.
- El nuevo shell sustituirá gradualmente la interfaz de
  `examples/level_editor.cpp`; ese ejemplo no se reescribe de golpe.
- `third_party/imgui/` deberá contener las fuentes, backends GLFW/OpenGL3,
  licencia y una nota con tag y commit de procedencia.
- `imgui.ini` es configuración local ignorada; ningún documento de
  proyecto se serializa en él.

## Replanificación de EDT-01 a EDT-06

| Paquete | Replanificación ejecutable |
|---|---|
| EDT-01 | Crear `motor_editor` y `EditorApplication`; mover el arranque del ejemplo sin cambiar `EditorState` ni el runtime. |
| EDT-02 | Vendorizar ImGui, crear target privado `editor_imgui`, integrar GLFW/OpenGL3, dockspace y layout local; sin multi-viewport. |
| EDT-03 | Extraer el FBO existente a `RenderTarget` RAII y presentarlo con `ImGui::Image`; formalizar foco, DPI y conversión panel→grid. |
| EDT-04 | Implementar `ProjectPanel` con árbol ImGui que emite comandos a `ProjectHub`/`ProjectIndex`, nunca mutaciones directas de archivos. |
| EDT-05 | Implementar `AssetBrowserPanel` con índice GL-free, búsqueda y payloads de `AssetId`; las miniaturas se resuelven fuera de la lógica documental. |
| EDT-06 | Implementar `TilePalettePanel` sobre `EditorState`, con grid, filtros, selección y `ToolContext`; los IDs, no UVs, son el estado persistente. |

## Acciones

1. En INF-04, fijar versión, licencia y política de actualización de
   `third_party/imgui/`.
2. En EDT-01, crear `motor_editor` sin migrar aún los paneles activos ni
   mezclar sus cambios locales con el nuevo shell.
3. En EDT-02, añadir el backend y una prueba de smoke de arranque/cierre.
4. Mantener CTest GL-free separado de las pruebas visuales de ImGui.
