# E-L1C — Clasificar fallas heredadas de ctest (agente: Claude; antes opencode · DeepSeek v4.1)

Parte de L1 de FRONT-01. **Solo diagnóstico**: no se arregla nada, no se compila y no se lanza `ctest`. Rama de referencia: `claude/unificacion-28-10`. Los ficheros cmake están en `MotorGraphico-main/MotorGraphico/cmake/`.

## Pruebas
- `test_personajes_capas` (`AparienciaTests.cmake`). Falla por **timeout**: anota el `TIMEOUT` que tiene y cuántos casos lanza.
- `test_kit_basico` (`CreacionBg3Tests.cmake`)
- `test_clases_sp` (`ClasesSpTests.cmake`)

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

