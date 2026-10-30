# Relevo E-L1C
Agente y sesión: Claude (Opus, sesión de claude.ai enlazada al Mac) · 29-10 15:55 · sesión nº 1
Estado: hecho
Pasos: test_personajes_capas hecho · test_kit_basico hecho · test_clases_sp hecho

Evidencia de ejecución: no he compilado. He usado los ctest reales que la fábrica pasó **hoy en el Mac**: `out/fabrica/RECURSOS-02/ejecuciones/20261029-145309_completa/ctest_gl_free.log` (y los mismos resultados en RECURSOS-01 y PERF-01, seis ejecuciones). Esas ejecuciones son de la rama `codex/perfilador-motor` @ 9349d3e. He comprobado que en `claude/unificacion-28-10` las líneas que fallan son las mismas (CofreKitTests.cpp:66, ClasesSpTests.cpp:202) y que todos los fuentes y datos existen.

## Hallazgos (con evidencia)
| prueba | causa | evidencia | responsable propuesto | qué hace falta para que vuelva a contar |
|---|---|---|---|---|
| `test_personajes_capas` | **lenta** (depende de la máquina **y del tipo de compilación**; ver corrección abajo) | En el Mac **pasa** en 117–124 s (6 de 6). En la nube de la fábrica de noche dio Timeout. `AparienciaTests.cmake:26-29` no le pone `TIMEOUT`, así que manda el que use quien lance ctest. Monta 62 personajes byte a byte. | fábrica | `set_tests_properties(... TIMEOUT 600)` y etiqueta `lenta`, sin quitarle casos. Medir cuánto tarda en la nube; si pasa de 600 s, decidir si va a una fase aparte. |
| `test_kit_basico` | **rota de verdad: la prueba está desfasada** | Falla `CofreKitTests.cpp:66`: espera `piezas[1].bono == "+1 CAR"`. Pero `src/Game/CofreKit.cpp:53` deja el bono vacío a propósito: `Pieza p{e->name, ranuraLegible(*e), ""};  // AV16-02: las prendas del kit no dicen que sumen` (commit 763e2ad, 15-10). Los otros 5 casos pasan. | fábrica (está en la zona de la prueba) | Cambiar la línea 66 para que espere `bono.empty()`, como la línea 65 con el arma. Confirmar con Oriol que AV16-02 sigue vigente. |
| `test_clases_sp` | **rota de verdad: la prueba está desfasada** | Falla `ClasesSpTests.cpp:202`: espera que `leerGancho("siempre:velocidad_0")` sea `Desconocido` («lo que el combate no simula no vale como rasgo»). Pero `src/Game/GanchosCarta.cpp:84` ya reconoce `velocidad_0` como `VelocidadCero` (estados nuevos del combate, commit 763e2ad, 15-10). Los demás casos pasan. | fábrica (está en la zona de la prueba) | Cambiar el ejemplo por un gancho que de verdad no exista (p. ej. `siempre:no_existe`), manteniendo la intención del caso. |

Ninguna de las tres tiene que ir a cuarentena: las tres se arreglan sin tocar el código del juego.

## SIGUIENTE ACCIÓN
Nada en esta E. Para quien junte L1: las dos pruebas desfasadas se arreglan en una E aparte (dos líneas de prueba + `TIMEOUT 600` en `AparienciaTests.cmake`), y después se pasa `ctest -R "test_kit_basico|test_clases_sp|test_personajes_capas"` en el Mac.

## Dudas para Oriol
- ¿Sigue vigente AV16-02 («las prendas del kit no dicen que sumen»)? Si sí, se arregla la prueba; si no, el error está en `CofreKit.cpp`.

## Corrección (29-10, 19:00): el dato de 120 s era de otra rama
Los 117–124 s salen de ejecuciones sobre **`codex/perfilador-motor`**, no sobre `claude/unificacion-28-10`. Las dos compilan en `macos-debug`, pero la unificada lleva 138 commits más, y en ellos cambian justo esta prueba y su código:
- `tests/Game/PersonajesCapasTests.cpp`: +265 líneas;
- `src/Game/Personajes/MontajePersonajes.cpp`: +211 líneas;
- muchos más sprites de personajes en `assets/textures/personajes/`.

En la unificada, la prueba **pasa en 1.776 s** (medida del piloto del 29-10). El error fue tratar una medida de otra rama como si fuera de esta.

Lo que queda abierto, en una E aparte: **por qué en la unificada tarda 15 veces más**. Puede que sea normal (más casos y más personajes) o puede ser una regresión de rendimiento en `MontajePersonajes.cpp`. Oriol mantiene `TIMEOUT 600` (29-10, 18:45), así que la prueba fallará hasta que se resuelva.
