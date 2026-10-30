
---
# Archivado el 14/10/2026 (noche) por Claude (coordinador): fichero completo tal como estaba

# opencode — tarea actual
**Asignada:** 14/10 por Claude · **Repo:** `MotorGraphico-main` · **Tipo:** trabajo nuevo → rama propia (regla 2)

## CRITICO-DOS-SEIS-01 — el crítico pasa a «al menos dos 6»
Ley de Oriol, decisión `decisiones/D0013`. No es una pregunta.
- Regla: **1 dado → nunca crítico · 2 dados → los dos en 6 · 3 o más → basta con dos 6.**
- Hoy `include/RPG/DicePoolEngine.h` / `src/RPG/DicePoolEngine.cpp` (`roll_pool`) pide todos los dados en 6.
- Actualiza comentarios y las pruebas que fijan la regla vieja: `tests/RPG/SkillExecutorTests.cpp`, `tests/Game/DiceRollWidgetTests.cpp`, y lo que haya en `examples/`.
- Añade pruebas de los tres casos (1, 2, 3+ dados) incluyendo los bordes (3 dados con un solo 6 = no crítico).
- Al final, repite el número del estudio de dificultad del Tier 1 (GDD §21): con 4 dados el crítico debería pasar de ~0,08 % a ~13 %. Pon el número que te salga en el informe.

Reclama la fila en el TABLERO (está en «Pendiente»). **Hecho cuando:** las pruebas nuevas pasan y fallan si se vuelve a la regla vieja; ninguna otra prueba se rompe.

---
## SIGUIENTE (solo si Oriol te lo dice al pegar el mensaje) — AV13-04 — Probabilidad exacta de una tirada enfrentada
Plan: `planificacion_alcance_y_vision/` (fase AV13, carril paralelo) y `COMBATE_BG3.md` §6. Depende de que tu tarea actual (crítico de dos 6) esté hecha: usa ya la regla nueva.
- **Zona (solo esto):** NUEVOS `include/RPG/ProbabilidadPool.h`, `src/RPG/ProbabilidadPool.cpp`, `tests/RPG/ProbabilidadPoolTests.cpp`, `cmake/ProbabilidadPool.cmake` y su `include()` en `CMakeLists.txt`. Sin GL. No toques `DicePoolEngine.*` (solo léelo).
- **Qué:** dada una tirada enfrentada (pool del atacante y del defensor, 0..8 dados cada uno, con ventaja/desventaja de cada lado), la probabilidad EXACTA de: gana (diferencia > 0), empata (roce, D0014: quita 1), pierde; más la de crítico (dos 6, D0013) y la de pifia de cada lado; y la distribución de la diferencia de éxitos (para el daño previsto). Por convolución de las caras (6 = 1 éxito, 5 = 0,5; ventaja: los 4 suman 0,5; desventaja: solo los 6), no simulando.
- **Hecho cuando:** para todos los pares 0..8 × 0..8, con y sin ventaja/desventaja, coincide con 200 000 tiradas de `roll_pool` a menos de 0,5 puntos; pruebas de bordes (pool 0, un dado, dos 6). Fila en el TABLERO («Pendiente» → «En curso» → «Hecho»).

---
## INFORME (lo rellena el agente al terminar)
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:** `agente/...` · hashes
- **Qué ha cambiado:** ficheros y en una frase cada uno
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
- **Qué tengo que mirar yo (Oriol):** decisiones tomadas, dudas, riesgos
- **Fichero de sesión:** `sesiones/...`
