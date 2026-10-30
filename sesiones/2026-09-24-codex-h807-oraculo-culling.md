# H8-07 · oráculo histórico de Culling

Fecha: 2026-09-24  
Agente: Codex  
Commit: `ac8443d` — `H8-07: registrar oráculo histórico de culling`

## Propósito

El Gantt y `MAPA_DEL_CODIGO.md` sitúan C-19 como la consolidación de un único
culling. `VisibleGridRange` ya es el único código de producción; la función
`legacyVisibleRange` de `tests/Render/OcclusionTests.cpp` conserva el
comportamiento anterior para comparar la equivalencia en 40 combinaciones. El
analizador la confundía con un envoltorio/reimplementación sin conocer esa
intención.

## Resultado

- `C-19.json` registra el par exacto `include/Render/Culling.h` ↔
  `tests/Render/OcclusionTests.cpp`, el oráculo, `test_occlusion`, el motivo y
  la regla de cambio.
- `tools/analizar_duplicados.py` reclasifica únicamente ese `ENVOLTORIO` ya
  detectado como `ORACULO_HISTORICO_DE_PRUEBA` de severidad `info`.
- La excepción exige dos rutas relativas distintas, una de producción y una de
  `tests/`, y los cuatro metadatos obligatorios. Registros malformados o pares
  declarados dos veces no silencian nada; una excepción válida que deje de
  aplicarse avisa y hace fallar `--comprobar` en el árbol real.
- El comentario de la prueba ya no afirma una copia byte a byte: es un oráculo
  histórico de comportamiento, adaptado en firma y construcción del resultado
  al arnés.

## Concurrencia

No se regeneró `MAPA_DEL_CODIGO.md` ni `alcance_real.json`, para no recoger
WIP ajeno. No se tocaron `Culling.h`, renderer, CMake, `Application.*` ni
archivos de cruce. El commit contiene exclusivamente los cinco archivos de
H8-07.

## Verificación

- `python3 -m py_compile tools/analizar_duplicados.py tools/test_analizar_duplicados.py`
- `python3 tools/test_analizar_duplicados.py` — 32 casos plantados en verde.
- `python3 tools/analizar_duplicados.py --comprobar` — verde y ancla real.
- Enfoque de `include/Render/Culling.h` —
  `ORACULO_HISTORICO_INTENCIONAL`, severidad `info`, sin excepciones sin aplicar.
- `cmake --build out/build/codex-h805 --target test_occlusion demo_tilemap --parallel 2`
- `ctest --test-dir out/build/codex-h805 -R '^test_occlusion$' --output-on-failure`
- `out/build/codex-h805/demo_tilemap`
- `python3 tools/linea_base_core.py --rapido` y `git diff --check`
