# 05/11/2026 — Claude: las ramas a las que se llega huyendo cuentan una huida

**Rama:** `agente/claude/UNI-02-11`. **Decisión:** `decisiones/D0029-ramas-de-huida-cuentan-una-huida.md` (`proposed`: Oriol respondió «Continúa» a la recomendación; los textos esperan a que los lea).

- Textos de `bgd_perdida`, `bgb_detras_perdida`, `b2_saqueadores_ganan`, `b1_secta_ganan`, `b3_sandes_gana_casco`, `b3_sandes_sigue` y una frase nueva en `b3_compania`. Ningún efecto cambia.
- Diario: las entradas de la cripta y los saqueadores pasan a `bnd1_guarida_huida` y `bnd2_saqueadores_huido`, que sí se encienden.
- Pruebas: `las_ramas_a_las_que_se_llega_huyendo_cuentan_una_huida` en `test_matriz_ocaso`; 10 mutaciones en rojo y verde (`out/fabrica/UNI-02-11/logs_ramas_huida/`). Suite gl-free 304/304 (`ctest_final19.log`), `validar_todo` 0 errores y 20 avisos, explorador sin atascos (`explorar_20_h3.log`).
- **Sin comprobar en pantalla:** estas escenas no salen al cargar un guardado, porque el juego solo evalúa los beats `auto` tras un evento (cerrar un diálogo, acabar un combate). Los 7 intentos de captura no muestran la escena (`logs_ramas_huida/intentos_captura/`).
- Sigue pendiente de Oriol: leer los textos, D0029.3, D0028.3 y los valores por defecto de D0027.
