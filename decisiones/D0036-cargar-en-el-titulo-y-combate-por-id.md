# D0036 — «Cargar partida» en el título, y el combate identifica por id (05/11/2026)

Estado: **aceptada** (Oriol, chat del 05/11: «Això s'ha de arreglar doncs», sobre la falta de «Cargar partida» en el título y sobre que el combate situara a los participantes por su nombre). Aplica D0035 §2. Sustituye a D0035 §5 en el id de un guardado antiguo.

| # | Decisión |
|---|---|
| 1 | El título tiene «Cargar partida»: personajes, sus partidas, y cargar o borrar. Sale apagada si nadie tiene partidas. Cargar desde el título no pide confirmación: no hay nada que perder. |
| 2 | Cada participante de un combate tiene un id propio (`a0`, `e3`: bando e índice). El tablero, los golpes, las tiradas y la cámara lo usan a él. El nombre solo se enseña. |
| 3 | La línea de reputación de la tienda sale del id del tendero. |
| 4 | Un guardado sin UUID usa siempre el mismo id, sacado de su nombre y su clase, y ya no estrena uno en cada carga. Es la única excepción a la regla, y solo para partidas de antes. |
| 5 | Siguen pendientes de la auditoría, porque piden cambiar datos o afectan a lo visual: el Códice por `speaker`, las secciones de misiones por título, los carteles y bocadillos por nombre, el retrato de espera por subcadena, y la forma de las cartas leída de su descripción. |
