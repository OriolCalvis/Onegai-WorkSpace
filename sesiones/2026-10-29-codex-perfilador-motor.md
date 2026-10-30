# PERF-01 — primer perfilador útil del motor

- **Fecha:** 2026-10-29
- **Autor:** Codex
- **Commits:** `7fa2117` (implementación probada); commit posterior de evidencia documental en la misma rama.
- **Deja esperando a Oriol:** aceptación jugable del CSV y decisión posterior de integración con el árbol compartido; no se presupone el visto.

## Autorización, zona y resultado

Oriol dijo «doncs comença!» tras la comparativa con Unity. Se anunció empezar
por perfilado y se usaron replica-motor-recon, replica-motor-construir y fabrica.
Rama propia/worktree gestionado `perfilador-motor`; el WIP del árbol principal
se conservó íntegro. La reclamación se escribió en TABLERO compartido antes de
crear archivos; la fila propia también viaja en el commit del worktree.

CSV opcional con MOTOR_PERFIL_CSV, tiempos del bucle y contadores de sprites
agregados entre escena y HUD. Solo frames renderizados, reloj monótono inyectable,
sin tocar timestep ni orden de callbacks. Application.h no crece y
ApplicationBucle.cpp queda en 472 líneas. Sin GL en motor_perfil; el llamador
real es juego, no solo un demo. Los cambios CMake y su include van juntos.

No se mide GPU: render_present_ms incluye swap/vsync y la captura PPM de humo.
ciclo_ms cubre tick, no el runner de eventos y transición posterior; delta_ms es
intervalo entre inicios. No hay medidas de arranque/carga, memoria ni IA desglosada.
No se afirma mejora de FPS: esta pieza permite medir las siguientes mejoras.

## Pruebas

Base completa de Fábrica antes de cambiar código: compila; cinco validadores y
16 tests GL-free fallan previamente. La guardia tiene 49 duplicados heredados.
Se conservaron logs completos en out/fabrica/PERF-01/linea_base.json y ejecuciones/.
La base llevó 291 s de build y 239 s de pruebas, especialmente composición de
personajes. No se canceló ninguna prueba ni se modificaron fallos ajenos.

Diez pruebas nuevas del núcleo y cuatro objetivos CTest de perfil/bucle/entrada/
escenas pasan. Las diez regresiones Python de paridad también pasan. La prueba
del snapshot inicial ahora lee features_iniciales.csv: ampliar la matriz activa
no rompe la comprobación del original. Se añade PERF-01 como partial con límites
y aceptación pendiente, sin reescribir los estados recibidos del resto del motor.

Humo real OpenGL: juego abre prado_del_rio durante 4 frames con perfil activo,
desactivado y ruta inválida; los tres terminan con código 0. CSV: cuatro IDs
consecutivos y contadores positivos (759 quads, 18 llamadas por frame en esta
captura; son contadores de humo, no benchmark). La ruta inválida da aviso y el
juego continúa. Logs: out/fabrica/PERF-01/humo/.

Comparación completa posterior: VERDE, ningún fallo nuevo; mismos cinco checks
y 16 tests heredados. Build completa posterior 31,3 s; CTest 175,7 s. Los 49
duplicados de la guardia coinciden exactamente antes/después. Entrega limpia:
resultado final en la evidencia.
La entrega con fabrica_perfil.json es explícitamente acotada a juego, núcleo de
perfil, tres vecinos y regresiones de paridad; no declara toda la deuda resuelta.

## Uso y prueba de Oriol

Desde el directorio del binario con assets:
`MOTOR_PERFIL_CSV=perfil.csv ./juego 120`.
Abrir un nivel, caminar y abrir inventario; cerrar y comprobar filas CSV con
quads/llamadas. Sin variable no hay captura. Para comparar escenarios conservar
resolución, build y hardware, descartar calentamiento y el frame de volcado PPM.
Guía: MotorGraphico/planificacion/replica/PERF-01.md.

## Discrepancias y preservación

Los hooks tienen la discrepancia monorepo ya conocida: la guardia se ejecutó
explícitamente, no se usó la salida 0 del hook como prueba. Sin cambios de hooks,
canon, reglamento, datos de juego, ramas ajenas, push o merge. El trabajo previo
sucio de Application, auras, shaders, HUD, assets y coordinación no entra aquí.

## Entrega limpia confirmada

Commit 7fa2117 recompilado en clon temporal limpio; juego y cuatro objetivos
CTest del lote pasan, más diez regresiones de paridad. Build limpia 34,9 s.
Humo GL limpio: cuatro filas CSV con contadores positivos, salida 0. Fuentes
externas reutilizadas: glm 1.0.1 y GLFW 3.4; ningún objeto/binario copiado.
Fábrica: VERDE para el alcance explícito de fabrica_perfil.json. Se utilizó
--sin-clon porque el clon se creó previamente con git-safe; no se verificó el
árbol sucio original. Evidencia, ZIP y CSV en out/fabrica/PERF-01/entrega_limpia/.
Comparación general previa: 224/240 tests pasan; mismos dieciséis fallos.
Aceptación manual de Oriol pendiente; no se probó Windows.
