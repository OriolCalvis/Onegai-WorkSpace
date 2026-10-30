# 2026-09-28 · qwen · DIAGRAMAS-MG: 5 diagramas archify del motor C++ + visor web

**Encargo directo de Oriol (28/09):** «hazme un diagrama de flujo, de clase y
de arquitectura con archify», ampliado en la misma conversación a los cinco
tipos que soporta la herramienta (`architecture`, `workflow`, `sequence`,
`dataflow`, `lifecycle`) y a «una página web super sencilla para poder ver
todos los diagramas». Alcance elegido por Oriol: **el motor C++
(MotorGraphico)**.

## Nota sobre el «diagrama de clases»

Archify v2.17 no tiene tipo `class`: sus cinco tipos son architecture,
workflow, sequence, dataflow y lifecycle (`archify --help`). Oriol pidió
entonces los cinco. La vista de clases sigue viviendo donde estaba:
`MotorGraphico/motor_grafico_clases.puml` (862 líneas) y
`analisis_clases/01..08`. El diagrama de **arquitectura** cubre la parte
estructural (12 componentes + fronteras), no la jerarquía de herencia.

## Qué hice

Rutas NUEVAS propias, todas bajo `MotorGraphico/architecture/diagramas/`
(5 specs JSON + 5 HTML generados + `index.html` + `README.md`). **Cero
edición de código ajeno**: solo lectura de `include/`, `src/`, `CMakeLists.txt`,
`ARCHITECTURE.md`, `architecture/ADR-*` y `DEPENDENCY_POLICY.md`.

| Diagrama | Spec | HTML | Contenido |
|---|---|---|---|
| Arquitectura | `motor.architecture.json` | `motor-architecture.html` | 12 componentes, 13 relaciones, 3 fronteras (GL / núcleo GL-free / editor) |
| Flujo | `motor.workflow.json` | `motor-workflow.html` | 5 calles: init, bucle, fin de frame, cierre, fallos |
| Secuencia | `motor.sequence.json` | `motor-sequence.html` | 19 mensajes de un frame real, 8 participantes |
| Datos | `motor.dataflow.json` | `motor-dataflow.html` | 4 stages, 12 nodos, 13 flujos: JSON → modelo → píxel/PPM |
| Ciclo de vida | `motor.lifecycle.json` | `motor-lifecycle.html` | Pila de escenas ADR-009 con fallo recuperable |

El visor es `index.html`: cinco botones + un `<iframe>`, sin dependencias ni
build. Cada HTML de archify es autocontenido (tema claro/oscuro, zoom,
búsqueda, foco por relación, exportación PNG/JPEG/WebP/SVG).

## Verificación

- `archify validate <tipo>` → **ok: true en los cinco** (calidad `standard`).
- `archify check <html>` → **9/9 checks en los cinco**, 0 fallos.
- No se pasó la puerta `showcase`: en arquitectura falla
  `composition/desktop-readability` y Oriol pidió explícitamente dejar de
  pulir errores y tener los flujos visibles. Los HTML se generaron con
  `render --quality standard`. Queda registrado como deuda estética, no de
  contenido.
- `deliver` (que congela la spec y da SHA-256) no se llegó a usar por lo
  mismo; se usó `render`.

## Hallazgos que corrigen documentación existente

Leídos del **árbol** del 28/09, no de un commit fijado (hay 166 ficheros con
WIP de otras sesiones; anclar `meta.repository` a HEAD habría sido falso, así
que los diagramas no llevan `sources` y lo declaran en una tarjeta):

1. **`analisis_clases/INDEX.md` (foto del 31/08) está desfasado** en su falta
   #6 «infraestructura sin consumidor»: `Application` **sí** tiene
   `Engine::SceneManager m_scenes` (`include/Engine/Application.h:142`) y
   `std::unique_ptr<Engine::World> m_world` (línea 329), y las usa
   (`m_scenes.start/requestReplace/applyPendingTransition` en
   `Application.cpp:815,974,1025`; `m_world->eachEntity` en
   `ApplicationNivel.cpp:378`). C-16 aterrizó.
2. **`GameMode` tiene 7 modos, no 6**: `Exploration, Battle, GameOver,
   Dialogue, Shop, Business, Pause` (`include/Game/SessionTipos.h:25`). El
   INDEX dice «GameSession (6 modos)».
3. **El bucle ya no es el del `.puml` antiguo**: `motor_grafico_flujo.puml`
   describe `Application::run()` con un `repeat` propio. El real
   (`src/Engine/ApplicationBucle.cpp:318`) delega el ritmo en
   `Engine::ApplicationLoop` (ADR-007): `loop.tick(deltaTime, services)` con
   cinco callbacks inyectados, paso fijo 1/60, tope 0,10 s y máximo 6
   `fixedUpdate` por frame. El render ya no lo llama `run()` directamente:
   va `loop → m_scenes.render(α) → ApplicationScene::render → Application::render`.
4. **`glfw` y `glm` siguen entrando por `FetchContent`**
   (`CMakeLists.txt:62-69` y `:648`), cuando `DEPENDENCY_POLICY.md` pide
   vendorizarlos en `third_party/`. GLAD, stb, tinyxml2 e ImGui sí están
   vendorizados.

Ninguna de las cuatro se ha tocado: se registran aquí y en las tarjetas de
los diagramas, que es lo que toca cuando el hallazgo es de zona ajena.

## Gotchas de archify que han mordido (para el siguiente)

- `node` no está en el PATH mínimo del agente: `export PATH="/opt/homebrew/bin:$PATH"`.
- En `dataflow` y `lifecycle`, **`meta.viewBox` NO gobierna la geometría**: el
  paso de columna es fijo (~224 px en dataflow, 154 px en lifecycle) y el
  ancho útil de nodo es el del ejemplo. Poner `width` grande da
  «exceeds the horizontal bounds of the viewBox» por grande que sea el viewBox.
- La máscara de una etiqueta de flujo incluye también su `classification`:
  un `classification` largo hace que la etiqueta «solape» aunque sea corta.
- `lifecycle` exige una lane con id reservado **`main`**; `terminal` es la
  banda de abajo y el resto de lanes comparten la banda de eventos, cuyas
  columnas son **0..2 alineadas debajo de la columna principal N+2**. Un
  estado de fallo no puede ponerse debajo de `col 1`.
- `workflow`: `mainPath` no puede retroceder de columna ni saltarse pares sin
  arista; con 6 columnas como máximo, un flujo que cambia de calle se queda
  fuera del `mainPath` (se marca con `role: "main"` en sus aristas).
- `sequence`: las cajas de participante son de 86 px fijos; con nombres como
  `IsometricRenderer` hay que poner `meta.column_fit: "spread"` en vez de
  acortar el nombre.

## Pendiente

- **Sin commit.** Los 12 ficheros nuevos están en el árbol de
  `MotorGraphico-main` (repo propio) y esta sesión + la fila del TABLERO en el
  de `Software`. Oriol decide si se versionan; el índice de MG viene de una
  incidencia (1386 ficheros staged como borrados, arreglada por opencode el
  28/09), así que antes de cualquier commit hay que mirar
  `git diff --cached --name-status`.
- La puerta `showcase` de arquitectura queda sin pasar (deuda estética).
