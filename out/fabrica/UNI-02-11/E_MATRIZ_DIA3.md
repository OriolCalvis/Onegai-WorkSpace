# E-MATRIZ-03 — Matriz de decisiones del día 3 (Oriol, 04/11: «Puja i segueix»)

Base: `agente/claude/UNI-02-11@9628017a`. Motor: `MotorGraphico-main/MotorGraphico/` (= MG). Mismo método que los días 1 y Gorran (`MATRIZ_DIA1.md`, `MATRIZ_GORRAN.md`, `MatrizDia1Tests.cpp`, `MatrizGorranTests.cpp`, `fabricar_guardados.h`, `tools/matriz_decisiones.py --dia`).

## A0 — Los dos ocultos que dejó E-GORRAN-TREGUA (avisos `?? HALLAZGO` del arnés)
1. `seguidor_brinda` (mansión, día 3): `demo_campana` y `test_matriz_dia1` le hablan sin haber entrado en la fiesta; falta `bnd2_fiesta_dentro` porque `b2_fiesta_puerta` no salta en esos caminos. Averigua cómo entra un jugador real en la fiesta y haz que el arnés lo haga así. Si la historia no deja entrar por ningún camino jugable, es un hallazgo con fichero:línea (no se toca la historia).
2. `venides_base` (cuartel del Ocaso): se le habla estando oculto. Igual: o el arnés llega como un jugador, o es hallazgo.
Cuando estén resueltos, el arnés pasa de avisar a **fallar** para cualquier oculto (sin lista de pendientes).

## A — Inventario
`matriz_decisiones.py --dia segundo_dia` (el día 3 es `boundington_segundo_dia.json`). Importante = se lee en el Ocaso, en eventos, cinemáticas o recompensas, o cambia moral/oro (el diario cuenta como lector solo de diario). Entrega `out/fabrica/UNI-02-11/MATRIZ_DIA3.md` (+ `.json`) con ⚠.

## B — Un guardado y una prueba por camino
Desde 2 finales de Gorran representativos (tregua y pelea), juega el día 3 de verdad y fabrica `dia3_<camino>.json` para cada decisión importante (máx. 10). Pruebas en `MG/tests/Engine/MatrizDia3Tests.cpp`: restaurar, empezar el Ocaso y comprobar consecuencias. Mutación por camino. Imprescindibles: la fiesta (entrar / no entrar), Verina (`b2_rosita_verina_arriba`, que exige `confianza_rosita >= 3`: mira si algún camino lo alcanza; si ninguno, es hallazgo) y el aviso de Xila. Ranuras: `fabricar_guardados --matriz-dia3`.

## C — Capturas
2–3 caminos con 1–2 capturas cada uno (prefijo `m3_`), revisadas con la vista.

## Alcance
`tools/matriz_decisiones.py` (+ prueba), `examples/fabricar_guardados.*`, `examples/demo_boundington_partida.h` (solo arnés), `examples/demo_*boundington*` (solo para A0), `tests/Engine/MatrizDia3Tests.cpp` + `cmake/`, `tools/guiones/m3_*.txt`. No toca aventuras, reglas ni `src/`.

## Aceptación
Arnés sin ocultos pendientes; inventario reproducible; prueba verde y mutación roja por camino; suite gl-free sin fallos nuevos (297 o más); `validar_todo` 0 errores; capturas revisadas. Presupuesto: 100 min.
