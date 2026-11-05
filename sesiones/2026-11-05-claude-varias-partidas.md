# 05/11/2026 — Claude: 15 partidas por personaje y ESC en combate

**Rama:** `agente/claude/UNI-02-11`. **Decisión:** `decisiones/D0034-varias-partidas-por-personaje.md`.

- `SaveSlots::kSlotCount` pasa de 6 a 15; cada personaje usa un `SaveSlots` propio en `saves/<proyecto>/personajes/<nombre>/`. `rapido.json` sigue en `saves/<proyecto>/`.
- `Game::filasDePartidas` (`SaveSlotState.cpp`) da las filas de la lista, de 6 en 6. `PauseMenu::eligePartida` la pinta como una pregunta con más filas. `ApplicationCinematica::confirmaPartidas` atiende el ENTER.
- ESC en combate: `escapeAction` da Pause, `GameSession::pause` admite Battle y `resume` vuelve al combate si lo hay.
- Fuera `detalleGuardado` y `motivoDeLaNota` de `Application.cpp`, que solo usaban Guardar y Cargar del papiro.
- Pruebas: `test_save_slots`, `test_game_session` y `test_revivir_al_inicio`; 6 mutaciones en rojo y verde en `out/fabrica/UNI-02-11/logs_d0034/`. Probado en el juego real: `capturas/d34_*.png`. Suite en `ctest_final26.log`, explorador en `explorar_20_d0034.log`.
- Falta: el título no tiene «Cargar partida», así que las partidas de otro personaje solo se alcanzan con «Continuar» si son la última. No hay forma de borrar una partida desde el juego.
- Sin probar en el juego: la lista paginada con más de 6 partidas (solo la prueba de datos), el clic con ratón en la lista y un combate con acompañantes.
