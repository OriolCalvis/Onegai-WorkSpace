# SESIÓN 2026-09-03 — Codex — Pantallas críticas integradas en el Corte B

- **Dónde:** `MotorGraphico-main/MotorGraphico/planificacion/`, `TABLERO.md`, `decisiones/D0002-pantallas-criticas-corte-b.md` y este registro.
- **Commits:** MotorGraphico `5c96eb8` (11 archivos propios de planificación y generación).
- **Encargo:** Oriol pide revisar la integración en el plan de los huecos críticos de pantallas del editor y del jugador detectados en la comparación con RPG Maker.
- **Deja esperando a Oriol:** Nada nuevo; las decisiones históricas del tablero conservan su estado.

## Terminado y cómo

1. **Trazabilidad de pantallas:** matriz de 28 filas en la fuente `pla/dades.py`, visible en `PLA_DEFINITIU.md` y `render/pla.html`. Las 25 filas del Corte B enlazan paquetes y recorrido de aceptación; tres grupos identifican ampliaciones aplazadas. La fotografía inicial no se presenta como porcentaje de finalización.
2. **16 actividades nuevas:** E-10..16 concretan shell visual, proyectos y árbol de mapas, propiedades/capas, recursos, selectores, diagnósticos y edición RPG mínima local. G-08..16 concretan título, personaje integrado, pausa/opciones, slots/derrota, inventario/equipo/ficha, combate desde datos, feedback, asistente de exportación y aceptación final.
3. **Integración con lo existente:** E-05/07/09, G-01/02/04/07 y sus dependencias incorporan esos recorridos. Se reutilizan los modelos y servicios existentes. F-01 reconoce que P ya guarda antes de lanzar el juego. G-14 exige coordinar el resultado de F1 con la reserva de zcode; no se han editado GameSession/Application.
4. **Alcance explícito:** el mínimo de datos RPG locales entra en el Corte B; los imports canónicos siguen siendo de lectura. Las cuatro familias editables son objetos/consumibles, equipo, habilidades y enemigos. La edición avanzada, audio, autotiles, plugins, generador gráfico y exportación nativa macOS siguen aplazados. D0002 registra esta integración solicitada.
5. **Estimación recalculada:** de 54 paquetes / 177 jornadas a 70 / 243 (+16 / +66). C 62, D 30, E 79, F 13, G 59. Se etiqueta como esfuerzo total del alcance, no trabajo restante ni compromiso de fechas.
6. **Cierre verificable:** G-16 exige un autor y un jugador nuevos recorriendo las pantallas reales, con evidencia de interacción, efectos, cancelaciones y errores. Las pruebas del modelo por sí solas no cierran una pantalla.
7. **Generación consistente:** corregidas las puertas de fase para esperar todas las ramas terminales, y el orden de las flechas PlantUML para situar después del predecesor que acaba más tarde. G-16 es el único terminal de G. Markdown, HTML y Gantt SVG/PNG regenerados. La página incluye UTF-8; los tableros históricos enlazan la revisión del Corte B y avisan de no sumar sus cifras.

## Verificación realizada

- Generador Python: 70 IDs únicos, 243 jornadas; 28 filas de cobertura, 25 incluidas y todas con paquetes válidos; sin ciclos ni dependencias hacia fases posteriores.
- Cálculo de dependencias: el primer inicio de cada fase no precede al último final de la anterior; G termina en G-16.
- Regeneración idempotente: Markdown, PlantUML y HTML tienen el mismo SHA-256 antes y después de repetir el generador.
- PlantUML local 1.2026.7: SVG y PNG generados correctamente; revisión visual del Gantt.
- Navegador: 70 fichas, UTF-8 correcto, cero enlaces internos rotos, sin desbordamiento horizontal; revisión visual de la matriz de pantallas.
- `git diff --check` y `git diff --cached --check` limpios. Índice contrastado con la lista exacta de 11 archivos propios antes del commit.

## Trabajo ajeno preservado

Al entrar había 12 anotaciones de avance de Claude/Cowork escritas directamente en el Markdown y sin fuente JSON: C-01/02/03/04/05/06/07/08/12/13/14/19. Para que regenerar no las borrara, se trasladaron a registros **locales sin añadir a Git** en `seguimiento/`, conservando autor, texto original, verificaciones y pendientes. `source_markdown` permite reproducir literalmente esas anotaciones. Se comprobó su conservación; el workspace muestra 18 registros en total.

El commit se generó sobre los seis registros ya versionados, con artefactos preparados aparte: no incorpora esos avances ajenos ni los cambios C++ preexistentes. Las diferencias de seguimiento continúan en el workspace para que su autor las integre. Copia de seguridad y artefactos aislados en la ruta indicada por `/tmp/motor-plan-pantallas-current`.

## Siguiente ejecución

Las nuevas actividades quedan pendientes. Se mantiene el orden C → D → E → F → G y la obligación de reclamar tarea en TABLERO antes de programar. Las tres numeraciones históricas siguen siendo un asunto pendiente de Oriol; esta revisión conserva los IDs del Corte B y documenta la correspondencia, sin declarar resuelta aquella decisión.
