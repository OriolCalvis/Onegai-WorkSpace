# 05/11/2026 — Claude: el resto de identidades por id, y quien llega a mitad del combate

**Rama:** `agente/claude/UNI-02-11`. **Decisión:** `decisiones/D0037-resto-de-identidades-y-recien-llegados.md`.

- `SaveSlots::load` da UUID a una partida antigua y reescribe el fichero. Fuera `idDeGuardadoAntiguo`.
- `BattleState::colocaLosNuevos`, desde `sumaAliado` y `anotaGolpes`; `sincronizaTablero` crea el dibujo que falte.
- `areas_de_cartas.json` (23 filas) y `areaDe` sin mirar la descripción. `QuestSection::id` y `quests.json` migrado. `NarrativeBeat::speakerId`. `pintaCarteles` y `pintaBocadillosPnj` reciben la casilla del aviso.
- Pruebas: `test_g13_combat_commands`, `test_save_slots`, `test_diario_completo`, `test_codice` y `test_pergamino`; 8 mutaciones en rojo y verde en `out/fabrica/UNI-02-11/logs_d0037/`. Suite en `ctest_final31.log`, explorador en `explorar_20_d0037.log`.
- Sin prueba automática: el retrato de espera, el marco del que tira, el bocadillo por casilla y el dibujo de un refuerzo.
- Sin probar en el juego: un refuerzo, una invocación y una carta de línea o cono.
- Queda por nombre: la marca del suelo se borra al hablar otro comparando el nombre (`InterfazPergamino.cpp`, solo visual), porque el diálogo solo conoce el rótulo de quien habla.
- Pendiente de contenido: dar ficha y `speakerId` a los 89 rótulos para que el Códice los trate por id.
