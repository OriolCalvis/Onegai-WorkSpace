# 2026-10-08 · zcode · Valoración global del proyecto y DAFO (análisis, sin cambios de código)

Sesión de análisis a petición directa de Oriol: «estudia todo el progreso,
haz una valoración y plantea un DAFO». Solo lectura: BITACORA entera
(Partes 0–8), TABLERO completo, `sesiones/INDICE.md` (177 sesiones),
`decisiones/` D0001–D0013, git logs de MotorGraphico/dndWeebCC/Onegai 2,
fichas de `planificacion/seguimiento/` (193) e inventario de código y assets.
La valoración y el DAFO completos se entregan en la conversación; aquí queda
el resumen y los datos medidos que los sostienen.

- **Commits:** solo este fichero + `sesiones/INDICE.md` regenerado, por
  pathspec con `herramientas/git-safe.sh`. Sin fila de TABLERO que mover
  (no se reclamó tarea: análisis).
- **Deja esperando a Oriol:** nada nuevo — el DAFO *agrupa* lo que ya está
  en su bandeja (31 asuntos), no añade filas.

## Datos medidos (2026-10-08)

| Qué | Valor |
| --- | --- |
| Sesiones registradas | 178 ficheros (opencode 78, codex 32, zcode 24, qwen 21, claude 20, cursor 2) — desde el 19/07 |
| Decisiones | 13 (D0001–D0013): ~8 accepted / ~6 proposed en bandeja |
| MotorGraphico | 304 commits en `feature/editor-s1-contratos` (master **273 atrás**, ff posible); 233 commits solo en septiembre; árbol con **396 M + 492 ?? + 4 D** |
| Código del motor | 505 ficheros .h/.cpp, **75.420 líneas**; 179 tests, 56 demos, 111 niveles + 111 TMX + 26 catálogos |
| dndWeebCC | 19 commits (rama java-upgrade), **1.698 ficheros sucios** (CARTAS-CLASES-01 de Claude sin commitear: 1.223 cartas nuevas + 442 actualizadas) |
| Onegai 2 | 6 commits, 2.101 notas .md, 387 sucios |
| TABLERO | ~36 filas «En curso» (muchas ya HECHO esperando mover), **31 asuntos** en «Esperando a Oriol» (el más viejo del 15/08) |
| Campaña | Boundington jugable de punta a punta: 152/152 beats (3 días + Ocaso parcial), con moral, alternativas y karma — todo el paquete de Claude del 08/10 **sin commit** |
| Última integración | Gantt Fase H: 60 tareas / 128,5 jornadas; patrón dominante «mitad GL-free hecha, montaje en `Application.*` pendiente» |

## Resumen del DAFO (el completo, en la conversación)

- **Fortalezas:** velocidad y volumen extraordinarios con trazabilidad
  (sesión→commit→ficha); sistema cooperativo real y auto-corrector
  (auditorías, falsabilidad, «verde de hoy ≠ verde anotado»); arquitectura
  disciplinada (GL-free, 1 concepto = 1 representación); contenido canónico
  jugable ya en el motor; Oriol presente (D0008: 53/56 respondidas).
- **Debilidades:** `Application.cpp` cuello de botella (13 features
  terminadas sin enchufar, 7.645 líneas — H8-06); HEAD roto
  reiteradamente y suite nunca del todo verde; ~2.500 ficheros sin
  commitear en los tres repos; bandeja de decisiones creciendo más
  deprisa de lo que se vacía; mitades duplicadas (dos diarios de
  misión) y tres Gantt conviviendo.
- **Amenazas:** exposición a pérdida catastrófica (trabajo del 08/10 de
  Claude íntegro sin commit; índice minado ya ocurrió el 28/09); rama
  de 273 commits sin fusionar ni CI remota (push pendiente desde el
  14/09); coste de coordinación creciente por sesión; Oriol como único
  decisor = punto único de fallo; la maquinaria produce más de lo que
  integra (riesgo de desmotivación al ver que no llega al jugador).
- **Oportunidades:** una sola decisión (H8-06: composition root /
  partir `Application` por tema) entrega de golpe 7.645 líneas ya
  probadas al jugador; el slice vertical Boundington está a puertas de
  alfa/playtest externo (ALPHA_CIERRE existe); el sistema cooperativo
  en sí es un resultado publicable/productizable; D0013+D0008 dejan el
  reglamento casi cerrado → banco de conformidad construible; el mazo
  físico PDF por clase abre la vía híbrida mesa/juego.

## Recomendación única priorizada

Congelar la admisión de features nuevas hasta: (1) commitear el paquete
de Claude y los ~2.500 sucios, (2) decidir H8-06 y ejecutar el montaje,
(3) fusionar `feature/editor-s1-contratos` a master y encender CI
remota. El resto de velocidad actual se pierde en la última milla.
