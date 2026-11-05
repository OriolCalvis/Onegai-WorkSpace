# 05/11/2026 — Claude: de los combates importantes no se huye

**Rama:** `agente/claude/UNI-02-11`. **Decisión:** `decisiones/D0031-de-los-combates-importantes-no-se-huye.md`.

- 20 `startBattle` pasan a `ineludible` y pierden `flagHuida`. Fuera la entrada de diario `bnd2_saqueadores_huido`, que ya nadie enciende.
- `rescateAlMorir`: `reviveAlInicio` prueba en una copia del estado si, con `flagDefeat`, saltaría el beat de rescate; si es así, cierra el combate como derrota y salta el beat. Lo usan los dos combates con `bnd3_sandes_gana`.
- Arnés y demos adaptados: `demo_boundington_partida.h`, `demo_boundington_alternativas.cpp`, `demo_campana_boundington.cpp` (de 30 a 29 momentos clave).
- Pruebas en `test_revivir_al_inicio`; 4 mutaciones en rojo y verde en `out/fabrica/UNI-02-11/logs_d0031/`; suite en `ctest_final24.log`, explorador en `explorar_20_d0031.log`.
- Sin comprobar: si estos combates se pueden ganar con cualquier personaje. Sin huida ni límite de reintentos, uno imposible dejaría la partida atascada. Nada se ha visto en pantalla.
- Después: `decisiones/D0032-sin-limite-de-reintentos.md`. La duda de si se pueden ganar deja de ser pendiente: es decisión de Oriol que sean un reto.
