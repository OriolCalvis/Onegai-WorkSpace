# E-MATRIZ-04 — Matriz del Ocaso y capturas que la auditoría tumbó (04/11)

Base: `agente/claude/UNI-02-11` (HEAD actual). Motor: `MotorGraphico-main/MotorGraphico/` (= MG). Mismo método que los días 1, Gorran y 3 (`MATRIZ_DIA3.md`, `MatrizDia3Tests.cpp`, `fabricar_guardados.h`).

## Evidencia obligatoria (lección de `REGISTRE_CAPTURES_I_PROVES.md`)
- Cada mutación deja un log en `out/fabrica/UNI-02-11/logs_ocaso/<camino>_mut.log` con la salida en rojo **y** la salida tras restaurar en verde.
- **Trampa**: las pruebas leen los JSON de `out/build/macos-debug/assets/`, que solo se copian al compilar. Tras mutar un JSON y tras restaurarlo: `cmake --build out/build/macos-debug` antes de ctest (comprueba con `cmp` que la copia cambió). Si la mutación se hace en un temporal cargado por la prueba, no aplica.
- La suite final, a `out/fabrica/UNI-02-11/ctest_final8.log` (con fecha y hash al principio).

## A — Inventario
`matriz_decisiones.py --dia ocaso` sobre `boundington_ocaso.json`. Como es el último día, «importante» = cambia el final, el epílogo, quién sobrevive, oro/moral/reputación finales o el diario. Entrega `MATRIZ_OCASO.md` (+ `.json`) con ⚠. Incluye el conflicto que dejó E-DIA3-FIX: `b3_xila_avisa` dice «hasta las campanas» y exige no `bnd3_arenga`, así que en el camino `xila_corre` no se ve (hallazgo, no se toca la historia).

## B — Un guardado y una prueba por final
Desde 2–3 finales del día 3 representativos (aviso de Xila sí/no; Venides sin aviso), juega el Ocaso de verdad hasta cada final distinto. `ocaso_<final>.json` + `MG/tests/Engine/MatrizOcasoTests.cpp`: comprueba el final, quién vive y el estado del diario. Máx. 10 caminos. Ranuras: `--matriz-ocaso`.

## C — Capturas (y rehacer las que fallaron)
1. **Ocaso**: las campanas sonando y la arenga, de verdad (no el fundido ni la Casa de Guardia después). Los nombres de los PNG = los nombres de `captura` del guion.
2. **`m1_`**: rehazlas con los guardados actuales (con oro) y en el momento **en que se ve la diferencia** entre unirse al culto y sembrar dudas (la mina vieja de Gorran / la plaza del Ocaso). Comprueba con `md5` que no son idénticas.
3. **`s_sectarios_seguimiento`** y **`m3_fiesta_preguntas`**: que enseñen lo que dicen (la persecución, la elección), o renómbralas a lo que enseñan.
4. 2–3 finales del Ocaso con 1 captura cada uno (prefijo `m4_`).
Mira cada una con Read y escribe en `capturas/CAPTURAS.md` una fila por PNG nuevo o rehecho: qué se ve de verdad.
5. Corrige `MATRIZ_GORRAN.md` fase C (cita `m2_tregua_sin_dialogo`/`atraviesa`, borradas; las actuales son `m2_tregua_menu/pactos/cerrada`).

## Alcance
`tools/matriz_decisiones.py`, `examples/fabricar_guardados.*`, `examples/demo_boundington_partida.h` (solo arnés), `tests/Engine/MatrizOcasoTests.cpp` + `cmake/`, `tools/guiones/*.txt`, capturas y los `.md` de `out/fabrica/UNI-02-11/`. No toca aventuras, reglas ni `src/`.

## Aceptación
Inventario reproducible; prueba verde y mutación roja **con log** por camino; suite con log; capturas rehechas que enseñan lo que dicen; `validar_todo` 0 errores. Presupuesto: 120 min.
