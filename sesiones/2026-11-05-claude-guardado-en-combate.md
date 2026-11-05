# 05/11/2026 — Claude: F5 y F9 en combate

**Rama:** `agente/claude/UNI-02-11`. **Decisión:** `decisiones/D0033-guardado-y-carga-rapidos-en-combate.md`.

- `FotoCombate` guarda una partida completa al empezar el combate. `GameSession::guardadoDelCombate` la devuelve con los enemigos y los pedidos de la historia; `reanudaCombate` vuelve a montarlo.
- `CombateGuardado` en `SaveGameData`: clave aditiva `combate`, sin subir el esquema. Sin combate, el fichero no cambia.
- `Application::processInput` atiende F5 y F9 en combate. `cargaPartida` limpia la barra y lo visual: sin eso, el combate nuevo podía nacer en la misma dirección y no tiraba iniciativa.
- Pruebas en `test_revivir_al_inicio`; 4 mutaciones en `out/fabrica/UNI-02-11/logs_d0033/`. Probado en el juego real con `capturas/f5_*.png`: F5, turnos, F9, seguir jugando y reabrir el guardado. Suite en `ctest_final25.log`, explorador en `explorar_20_d0033.log`.
- Sin prueba automática: la limpieza visual de `cargaPartida`, solo capturas. Sin probar: un combate de historia con acompañantes o con oleadas en el juego real, y el cierre con la ventana a mitad de combate.
- `CanSaveNow` sigue negando el combate: `quickSave` lo salta solo si hay foto del combate.
