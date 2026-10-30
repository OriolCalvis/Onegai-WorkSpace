# Relevo E-L1B
Agente y sesión: opencode · Big Pickle · 2026-10-29 · sesión nº 1
Estado: hecho
Pasos: 1 hecho · 2 hecho · 3 hecho · 4 hecho

## Hallazgos (con evidencia)
- «Los tres demo_*» = demo_creacion_personaje, demo_boundington, demo_campana_boundington. No salen de `add_test` (solo hay `demo_mision_llave` y `test_post_processing_demo_*`): se registran con la función `add_motor_gl_free_demo_test` en CMakeLists.txt:1061 y las llamadas :1110, :1121, :1123. La lista sale de `MotorGraphico-main/.ui-puertas-01/.../build/Testing/Temporary/LastTestsFailed.log` (15 fallos, 3 demos, Oct 25 14:14).
- Todos los fuentes existen en `claude/unificacion-28-10` (comprobado con `cat-file -e`): LugaresTests.cpp, demo_creacion_personaje.cpp, demo_boundington.cpp (+alternativas.cpp, partida.h), demo_campana_boundington.cpp, CatalogContentTests.cpp. **No falta ningún fichero.**
- Los logs de ctest son del 25-10 en un worktree (`.ui-puertas-01`), anteriores a la rama de referencia: los fallos se reclasifican contra el árbol de `claude/unificacion-28-10`, pero no se re-ejecutó nada (prohibido por la E).
- demo_creacion_personaje y demo_campana comparten raíz: la ruta del demo no cierra el día 1 (`falta bnd_dia_cerrado` / `no se vio cierre_creadores`). El flag sí existe en `assets/adventures/boundington_primer_dia.json` (8 apariciones, p. ej. :26974 en el árbol de la rama).

| prueba | causa | evidencia | responsable propuesto | qué hace falta para que vuelva a contar |
|---|---|---|---|---|
| test_lugares | rota de verdad (datos y contrato desincronizados) | LugaresTests.cpp:55 y :92; `assets/catalogs/lugares.json` tiene 49 fichas, 30 sin `funcion` textual (primera en índice 18) y solo 17 destinos `interior_*` únicos en ciudad_centro/este/oeste (14/1/2) | encargo nuevo (datos de lugares) | rellenar `funcion` en las 30 fichas sobrantes y cuadrar fichas↔puertas, o recortar `lugares.json` a las 17 con puerta |
| demo_creacion_personaje | rota de verdad (expectativa obsoleta) | `require(cc.races().size() == 45)` en demo_creacion_personaje.cpp:43; `assets/catalogs/races.json` tiene 72 (clases 127 ✓, trasfondos 12 ✓) | fábrica | subir el número a 72 en la línea 43 |
| demo_boundington | pendiente de ejecución | demo_boundington_partida.h:352 `fallos.empty()`; salida del 25-10: «MAL DIA 1: el forastero que pregunta / falta bnd_dia_cerrado / no se vio cierre_creadores». El flag existe en el JSON, no se puede saber sin jugar si esa ruta lo pone | fábrica | `ctest --test-dir out/build/macos-release -R "^demo_boundington$" --output-on-failure` |
| demo_campana_boundington | pendiente de ejecución (misma raíz que el anterior) | demo_campana_boundington.cpp:92 `require(j.s.hasFlag("bnd_dia_cerrado"))` tras hablar con xila/venides/skilla y anochecer | fábrica | `ctest --test-dir out/build/macos-release -R "^demo_campana_boundington$" --output-on-failure` |
| test_catalog_content | rota de verdad (expectativa obsoleta) | CatalogContentTests.cpp:223 `cat.size() == 16`; `assets/catalogs/conditions.json` tiene 20 `entries` | fábrica | actualizar el número a 20 (o justificar el recorte) |

## SIGUIENTE ACCIÓN
Ninguna: E hecha. Si Oriol quiere el dato vivo: `cd MotorGraphico-main/MotorGraphico && ctest --test-dir out/build/macos-release -R "test_lugares|demo_creacion_personaje|demo_boundington|demo_campana_boundington|test_catalog_content" --output-on-failure` (hay que compilar antes; no se hizo por protocolo).

## Dudas para Oriol
- ¿Los 3 demos y test_catalog_content se «arreglan» actualizando el número esperado (45→72, 16→20) o son señal de que el contenido creció sin revisión? — evidencia: los assets ya tienen 72 y 20 — mi propuesta: actualizar y anotarlo en la fábrica.
- test_lugares: 30 fichas de lugar sin `funcion` y sin puerta. ¿Se recortan o se completan? — evidencia: lugares.json (49) vs 17 puertas — mi propuesta: completarlas.
- El único log completo de suite es del 25-10 en `.ui-puertas-01`, no de `claude/unificacion-28-10`. ¿Hay un log más reciente que deba usar? — sin verificar.
