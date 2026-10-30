# E-L1D — Clasificar fallas heredadas de ctest (agente: opencode · Beam, esfuerzo bajo)

Parte de L1 de FRONT-01. **Solo diagnóstico**: no se arregla nada, no se compila y no se lanza `ctest`. Rama de referencia: `claude/unificacion-28-10`. Los ficheros cmake están en `MotorGraphico-main/MotorGraphico/cmake/`.

## Pruebas
- `linea_base_core` (`LineaBaseCore.cmake`; también aparece en `Pergamino.cmake` de `.ui-puertas-01/`)
- `linea_base_core_configura` (`LineaBaseCore.cmake`)
- `simulador_dificultad_humo` (`SimuladorDificultad.cmake`)

## Pasos (uno por prueba; actualiza el relevo después de cada prueba)
Para cada prueba:
1. **Qué necesita.** En su `.cmake`, busca `grep -n "<prueba>" <fichero>.cmake` y anota los fuentes (`.cpp`) y los datos (`.json`, carpetas) que usa.
2. **Si existen en la rama.** Para cada uno:
   `git -c core.checkStat=minimal cat-file -e claude/unificacion-28-10:<ruta> && echo SI || echo NO`
   (la ruta empieza por `MotorGraphico-main/`).
3. **Si falta, dónde está.** `git -c core.checkStat=minimal log --all --oneline -3 -- <ruta>`. Anota la rama y el commit si aparece.
4. **Clasificar** con una de estas causas:
   - falta un fichero;
   - rota de verdad;
   - depende del Mac;
   - lenta;
   - frágil;
   - **pendiente de ejecución**: si no se puede saber sin ejecutarla. En este caso, escribe el comando exacto que habría que lanzar.

## Resultado (en el relevo)
Una tabla con estas columnas:

| prueba | causa | evidencia | responsable propuesto | qué hace falta para que vuelva a contar |
|---|---|---|---|---|

El responsable propuesto es la fábrica, Oriol o un encargo nuevo.

Nota: `linea_base_core` y `linea_base_core_configura` probablemente van juntas (una configura y la otra compila). Si es así, se anotan como un mismo grupo con una sola causa.
