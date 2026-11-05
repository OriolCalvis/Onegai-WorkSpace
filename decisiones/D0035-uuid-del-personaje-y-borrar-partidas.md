# D0035 — UUID del personaje, borrar partidas y cambiar de personaje (05/11/2026)

Estado: **aceptada** en 1–4 (Oriol, chat del 05/11: «fem servir un UUID [...] mai faria servir cap nom per diferenciar un objectes o personatges», «en mig d'una partida haig de poder canviar de jugador», «s'ha de poder sobrescriure i esborrar saves 100%»). Sustituye a D0034 §6. El punto 5 es un valor por defecto del Brain, a confirmar.

| # | Decisión |
|---|---|
| 1 | Cada personaje tiene un UUID desde que se crea, guardado en la partida (`hero.uuid`). Sus partidas viven en `saves/<proyecto>/personajes/<uuid>/`. El nombre solo se enseña. |
| 2 | Regla general: nada se distingue por su nombre, sino por un identificador estable y actualizable. |
| 3 | Toda partida se puede borrar, también el último guardado, las de otro personaje y las rotas. Las del personaje que se juega se pueden sobrescribir. Borrar y sobrescribir piden confirmación. |
| 4 | Desde la pausa, «Cargar partida > Otros personajes» lista los demás; al cargar una de sus partidas se juega con él. |
| 5 | *Por defecto:* un guardado sin UUID estrena uno al cargarse. Sus cicatrices apuntadas con el id viejo no se traspasan. |
