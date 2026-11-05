# 05/11/2026 — Claude: UUID del personaje, borrar partidas y cambiar de personaje

**Rama:** `agente/claude/UNI-02-11`. **Decisión:** `decisiones/D0035-uuid-del-personaje-y-borrar-partidas.md`.

- `Game::nuevoUuid`; `CharacterSheet::id` lleva el UUID al crear o restaurar (antes era `pj_<clase>`, así que las cicatrices se compartían entre personajes de la misma clase). Clave aditiva `hero.uuid`.
- `personajesGuardados`, `filasDeAcciones` y `VistaPartidas`: lista, otros personajes y acciones sobre una partida. `ApplicationCinematica::confirmaPartidas` las atiende.
- Pruebas en `test_save_slots`; 5 mutaciones en rojo y verde en `out/fabrica/UNI-02-11/logs_d0035/`. Probado en el juego real con `capturas/d35_*.png`: borrar, sobrescribir y pasar a otro personaje con guardado en combate. Suite en `ctest_final27.log`, explorador en `explorar_20_d0035.log`.
- Sin revisar: qué más se distingue hoy por nombre en el resto del juego (D0035 §2). El título sigue sin «Cargar partida».
- Sin probar en el juego: una partida rota, el ratón en estas listas y más de 7 personajes (la lista se corta en 7).
