# E-MISIONES-01 — Matriz de misiones del día 1 (Oriol, 04/11: «Puja i segueix»)

Base: `agente/claude/UNI-02-11@cef7d75b`. Motor: `MotorGraphico-main/MotorGraphico/` (= MG).
Por qué: la matriz de decisiones solo cubre lo que pesa en días posteriores; Oriol pide ver **todas las misiones y contenidos** de cada día. Cobertura real hoy (explorador, 20 partidas, `out/fabrica/UNI-02-11/explorar_20_post.log`): día 1, 298 de 393 beats vistos; 95 nunca.

## A — Inventario de misiones (script, reutilizable para los otros días)
`MG/tools/matriz_misiones.py --dia primer_dia` (prueba Python registrada en ctest). Una **misión** = un grupo de beats con un comienzo (gancho / primer beat que la abre) y un cierre (flags `*_resuelto|_resuelta|_cerrado|_cerrada|_hecho` o equivalente). Agrupa por el prefijo del id y por las flags que comparten; revisa a mano los grupos dudosos. Para cada misión:
- PNJ y nivel donde empieza; cómo se abre; cómo se cierra (todas las salidas: éxito, parcial, fracaso, pifia).
- Recompensa (oro, objetos, moral, nivel) y entrada en el diario (`quests.json`).
- **Cobertura del explorador**: beats vistos / total, y qué salidas no ha visto nadie (cruza con `explorar_20_post.log`; si 20 partidas son pocas, corre `explorar_boundington 60 2` en segundo plano con `perl -e 'alarm 1800'` y úsalo).
- **¿Se puede cerrar?** Comprobación estática: cada beat de cierre tiene `requires` alcanzables (flags que alguien pone, PNJ no oculto en ese momento según `cinematicas.json`, nivel existente). Lo inalcanzable es ⚠ con fichero:línea.
Entrega `out/fabrica/UNI-02-11/MISIONES_DIA1.md` (+ `.json`): tabla de misiones + ⚠.

## B — Prueba por misión
Para cada misión cerrable (todas), una escena en `MG/tests/Engine/MisionesDia1Tests.cpp` con el arnés (respeta `ocultoSi`): abre y cierra por la salida principal; para las salidas no vistas por el explorador, al menos una prueba que llegue a ellas (forzando la tirada si hace falta, como hace el arnés). Mutación por misión (copia temporal del JSON, como `MatrizDia1Tests`), con log en `out/fabrica/UNI-02-11/logs_misiones1/<mision>_mut.log` (rojo y verde).

## C — Capturas
Una captura por misión (el cierre o el momento clave), prefijo `q1_`, con guion; mira cada una con Read y apúntala en `capturas/CAPTURAS.md` (qué se ve de verdad). Si son más de 15 misiones, las 15 más largas y el resto listadas.

## Alcance
`tools/matriz_misiones.py` (+ prueba), `examples/fabricar_guardados.*`, el arnés (solo ayudas), `tests/Engine/MisionesDia1Tests.cpp` + `cmake/`, `tools/guiones/q1_*.txt`, `.md` y capturas en `out/fabrica/UNI-02-11/`. No toca aventuras, reglas ni `src/`: los fallos son hallazgos.

## Aceptación
Inventario reproducible; prueba verde + mutación roja con log por misión; suite gl-free con log (`ctest_final10.log`, 299 o más); `validar_todo` 0 errores; capturas revisadas. Presupuesto: 150 min.
