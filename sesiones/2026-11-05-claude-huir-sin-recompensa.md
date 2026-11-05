# 05/11/2026 — Claude: huir sin recompensa y con dado de oro; morir reinicia desde la primera oleada

**Rama:** `agente/claude/UNI-02-11`. **Decisión:** `decisiones/D0030-huir-sin-recompensa-y-reiniciar-desde-el-inicio.md`.

- Sin nivel, hito ni oro en `b3_huida_perdida`, `b3_rodeo_huida` y `tut_huida`; quitada la entrada `d1-tut-botin` de `tools/recompensas_escenas.json`; el camino `huido` de la matriz acaba en nivel 5.
- `Game::pagaLaHuida` (`GameSessionRevivir.cpp`): el dado de oro al huir, en todos los combates.
- `FotoCombate` guarda la primera oleada, la cola y las variables (curas apuntadas al cerrar una oleada); `reviveAlInicio` destruye la oleada en curso y vuelve a invocar la primera.
- Pruebas: `test_revivir_al_inicio` (3 nuevas o reescritas). 4 mutaciones en rojo en `out/fabrica/UNI-02-11/logs_d0030/`; suite en `ctest_final21.log`, explorador en `explorar_20_d0030.log`.
- Sin comprobar en pantalla. En las aventuras actuales ningún beat encadena dos `startBattle`: las oleadas de la cola solo las ejercitan las pruebas.
- Pendiente de Oriol: si el amuleto de `b2_saqueadores_ganan` también se quita (D0030 §5) y los porcentajes del dado (D0030 §3).
