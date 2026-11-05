# 05/11/2026 — Claude: «Cargar partida» en el título y el combate por id

**Rama:** `agente/claude/UNI-02-11`. **Decisión:** `decisiones/D0036-cargar-en-el-titulo-y-combate-por-id.md`.

- `TitleCommand::LoadGame`; el título pinta la lista con el papiro de la pausa. `abrePersonajes` pagina de 6 en 6: antes la lista se cortaba en 7.
- `BattleParticipant::id`, puesto por `BattleState` y al sumar aliados o invocar. `TableroCombate` sigue con claves de texto, ahora ese id. `TiradaVisible` lleva `quienId` y `contraId`; `GolpeCombate`, `deId`.
- Bug real que arregla: los dos mineros (`boundington_dia_gorran.json`) y los dos guerreros (`boundington_segundo_dia.json`) con el mismo nombre compartían casilla, y al caer uno el otro se quedaba sin tablero. Un héroe llamado como un enemigo saltaba a su casilla.
- Tienda: `m_currentBusinessId` también guarda el tendero; fuera `tenderoPorNombre`.
- Pruebas nuevas en `test_g13_combat_commands`, `test_game_session`, `test_save_slots` y `test_title_flow`. Mutaciones en `out/fabrica/UNI-02-11/logs_d0036/`; algunas se hacen por pares porque hay dos caminos. Capturas `d36_*`. Suite en `ctest_final29.log`, explorador en `explorar_20_d0036.log`.
- Sin prueba de la línea de la tienda con dos tenderos que se llamen igual: solo cambia de dónde sale el id. Sin probar en el juego: los dos mineros en su escena real, solo en la prueba de sesión.
- Hallado y sin tocar: las invocaciones y los refuerzos entran en combate sin casilla en el tablero (leído en el código, sin reproducir).
- Pendiente: D0036 §5.
