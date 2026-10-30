# 2026-09-03 · zcode — Comparativa código vs UML vs planificación

## Encargo

Oriol pidió: analizar la estructura del código, compararla con el UML y la planificación general, plantear preguntas y remarcar puntos de mejora en un markdown.

## Qué se hizo

- **Entregable:** `MotorGraphico/COMPARATIVA_CODIGO_UML_PLANIFICACION.md` (commit MG `3f1d521`, rama `feature/editor-s1-contratos`, solo ese fichero por pathspec). Contiene: foto de la estructura real (12 módulos, 101 cabeceras, LOC medidos, estado git por módulo), comparativa elemento a elemento contra `motor_grafico_clases.puml`, genealogía de los 9 documentos de planificación con su estado, 7 preguntas y 8 puntos de mejora priorizados.
- **Hallazgo principal:** el UML de clases no se toca desde el commit inicial (`2f801c5`, 11/08, pre-Onegai-Core) — verificado con `git log --follow`. Describe ~60 elementos frente a las 101 cabeceras actuales; faltan enteros el módulo RPG, NarrativeEngine, SaveGame, proyectos, tests y la arquitectura Map/World/Runtime/Import (en vuelo). La planificación, en cambio, ya está reconciliada (ESTADO_REAL 28/08 → PLA_DEFINITIU corte B generado de `dades.py`).
- **Desajustes vivos registrados:** tres numeraciones de fases coexistiendo (F0–F8 subtareas / F0–F5 editor 2.18 / C–G corte B) sin mapa de equivalencias; `gantt_subtareas_F0.puml` duplicado (Software raíz vs `planificacion/subtareas/`); ESTADO_REAL subestimando INF-03 por ser foto del 28/08; `motor_grafico_flujo.puml` igual de congelado que el de clases; `MotorGraphico-HUD-Redesign/` y `Incomplete request*` sin estatus claro (preguntado, no tocado).

## Verificación

- Lectura: bitácora entera (protocolo), TABLERO, UML de clases íntegro, flujo (cabecera), ESTADO_REAL_vs_ROADMAP íntegro, PLA_DEFINITIU §1–4, analisis_clases/INDEX, ROADMAP_RPG_EDITOR, inventarios de planificacion/ y architecture/.
- Mediciones propias: LOC por módulo, 101 cabeceras, enums `ObjectCategory` (4 valores) y `EditorTool` (7) contra headers, `git log --follow` de ambos .puml.
- Cero cambios de código; trabajo concurrente (C-20 de codex commiteado como `2ca74eb` durante la sesión; migración INF-03 sin commitear) intacto y respetado.

## Pendiente que dejo apuntado

- Las 7 preguntas del doc son candidatas a bandeja «Esperando a Oriol»; las dejo dentro del doc para que Oriol las lea en contexto y se suben las que proceda.

## Cierre (segunda pasada de la sesión)

- **Commits:** MotorGraphico `3f1d521` (doc COMPARATIVA_CODIGO_UML_PLANIFICACION.md, solo ese fichero por pathspec con `git-safe.sh`, rama `feature/editor-s1-contratos`); raíz `5b5f999` (TABLERO + esta ficha) y el commit de cierre de esta segunda pasada (bandeja + INDICE).
- **Deja esperando a Oriol:** 6 filas nuevas en la bandeja del TABLERO — UML vivo vs histórico; qué numeración de fases gobierna; INF-03 sin commitear; RUN-05 vs `World/` ECS-lite; adelgazar `Application` en el corte B; estatus de `MotorGraphico-HUD-Redesign/` e `Incomplete request*`. Detalle en COMPARATIVA §4.
