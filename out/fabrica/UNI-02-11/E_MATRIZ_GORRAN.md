# E-MATRIZ-02 — Matriz de decisiones del día de Gorran (Oriol, 03/11)

Base: `agente/claude/UNI-02-11@8f7f1aef`. Motor: `MotorGraphico-main/MotorGraphico/` (= MG). El mismo método que el día 1 (`E_MATRIZ_DIA1.md`, `out/fabrica/UNI-02-11/MATRIZ_DIA1.md`, `MG/tests/Engine/MatrizDia1Tests.cpp`, `caminosDia1()` en `MG/examples/fabricar_guardados.h`).

## A0 — Hueco del validador
Un beat con más de `NarrativeBeat::kMaxOpciones` (4, `MG/include/RPG/Narrative/NarrativeBeat.h:363`) opciones hace que el cargador rechace el JSON entero, y `validar_todo.py` no lo detecta. Añade la comprobación (en el validador de aventuras que ya corra `validar_todo`, o en uno nuevo registrado en `tools/validar_todo.py` y `cmake/ValidarTodoTests.cmake`), con su prueba Python y una mutación (un beat con 5 opciones → error).

## A — Inventario
`python3 MG/tools/matriz_decisiones.py` sobre `boundington_dia_gorran.json` (generalízalo si hace falta: el día por argumento). «Importante» = se lee después del día de Gorran (día 3 `boundington_segundo_dia.json`, Ocaso, eventos, cinemáticas, recompensas, diario `quests.json`, que cuenta como lector de diario pero no de juego) o cambia moral/oro/reputación. Entrega `out/fabrica/UNI-02-11/MATRIZ_GORRAN.md` (+ `.json`) con el mismo formato y los ⚠.
Decisiones vigentes que la matriz debe respetar: D0023 (los 4 líos caben; solo Tinet si se llega tarde; tregua con Gorran con aval + 3 líos; moral de un eje).

## B — Un guardado y una prueba por camino
Partiendo de los guardados del final del día 1 (elige 2 representativos: el de unirse al culto y uno neutral), juega el día de Gorran **de verdad** con `pasaElDiaDeGorranJugando` y las ayudas de los líos, y fabrica `gorran_<camino>.json` al final del día para cada decisión importante (máximo 10; el resto queda listado). Prueba por camino en un fichero nuevo `MG/tests/Engine/MatrizGorranTests.cpp`: restaura, juega el comienzo del día 3 y comprueba las consecuencias (flags, beats, oro, moral, mochila). Mutación por camino (copia temporal del JSON, como en el día 1).
Caminos que deben estar sí o sí: los 4 líos cerrados / solo 3; Mia perdida (solo Tinet); tregua con Gorran; subasta con «Dejar que estalle» y con «Acabar con él».
Ranuras de «Continuar»: añade los caminos de Gorran a `fabricar_guardados --matriz` (sin borrar los del día 1 si caben; si no, una opción `--matriz-gorran`).

## C — Capturas
Guion de 1–2 capturas para 3 caminos (tregua, Mia perdida, subasta) que enseñen la consecuencia. Prefijo `m2_`. Revisadas con la vista.

## Alcance
`MG/tools/matriz_decisiones.py` (+ prueba), el validador de A0, `MG/examples/fabricar_guardados.*`, `MG/examples/demo_boundington_partida.h` (solo ayudas del arnés), `MG/tests/Engine/MatrizGorranTests.cpp` + su registro en `cmake/`, `MG/tools/guiones/m2_*.txt`. No toca aventuras, reglas ni `src/`: los fallos de historia son hallazgos con fichero:línea.

## Aceptación
Validador nuevo con su mutación; inventario reproducible; cada camino con prueba verde y mutación roja; suite gl-free sin fallos nuevos (293 o más); capturas revisadas.
Presupuesto: 100 min.
