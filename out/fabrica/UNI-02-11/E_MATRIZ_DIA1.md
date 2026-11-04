# E-MATRIZ-01 — Matriz de decisiones del día 1 (Oriol, 03/11: «Començem per dies, sí»)

Base: `agente/claude/UNI-02-11@d113c5c9`. Motor: `MotorGraphico-main/MotorGraphico/` (= MG).
Idea de Oriol: una partida guardada por cada decisión o final distinto; se cierra día a día comprobando que cada decisión importante lleva a donde queremos.

## Fase A — Inventario (sin tocar código)
`MG/assets/adventures/boundington_primer_dia.json` tiene 393 beats, 206 con elecciones. La mayoría no cambia nada más allá del diálogo.
**Decisión importante** = una opción cuyas flags o variables se leen **después del día 1**: en `requires` de otro día (`boundington_dia_gorran.json`, `boundington_segundo_dia.json`, `boundington_ocaso.json`), en eventos (`MG/assets/eventos/`), en cinemáticas o en recompensas; o que cambia la moral, la reputación o el oro de forma notable. Sácalas con un script, no a ojo, y deja el script en `MG/tools/matriz_decisiones.py` (con `ensure_ascii=False`): se reutilizará para los demás días.
Entrega A: `out/fabrica/UNI-02-11/MATRIZ_DIA1.md`, con una fila por decisión importante: beat:línea, opciones, flags y variables que pone cada opción, dónde se leen después (fichero:línea) y la consecuencia que se ve hoy. Marca **⚠** las que parezcan un error: una flag que nadie lee, opciones que dan lo mismo, o una consecuencia que contradice el texto.

## Fase B — Un guardado y una prueba por camino
Para cada decisión importante (si son más de 8, las 8 de más peso; el resto queda listado):
- Con `MG/examples/fabricar_guardados.h` y el arnés de las demos, juega el día 1 **de verdad** hasta después de la decisión, en cada opción, y fabrica el guardado del final del día (`dia1_<camino>.json`).
- Una prueba gl-free por camino: restaura el guardado en una sesión limpia, juega el inicio del día siguiente con el arnés y comprueba las consecuencias del inventario (flags, beats que saltan o no, oro, moral). Con `require()`/`MOTOR_CHECK`, nunca `assert()`.
- Mutación por prueba: quita el efecto de la opción en una copia del JSON (cárgala desde un temporal, sin tocar el original) o cambia la opción elegida; la prueba debe fallar.
- No se cambia ninguna historia. Si un camino no lleva a donde dice su texto, es un hallazgo con fichero:línea.
- Los guardados también se dejan en una ranura que «Continuar» pueda abrir (mira `Game::SaveSlots`), para que Oriol los juegue.

## Fase C — Capturas
Un guion por camino importante (máximo 4 caminos), con 1–2 capturas del momento clave del día siguiente que demuestre la consecuencia. Prefijo `m1_`.

## Alcance
Puede tocar: `MG/tools/matriz_decisiones.py` (+ su prueba), `MG/examples/fabricar_guardados.{h,cpp}`, `MG/tests/Engine/` (fichero de pruebas nuevo + su registro en `cmake/FabricarGuardados.cmake`), `MG/tools/guiones/m1_*.txt`. No toca: aventuras, reglas, motor (`src/`).

## Aceptación
Inventario completo y reproducible con el script; cada camino de la fase B con su prueba en verde y su mutación en rojo; suite gl-free sin fallos nuevos (292 o más); capturas revisadas con la vista.
Presupuesto: 90 min (A: 25, B: 50, C: 15).
