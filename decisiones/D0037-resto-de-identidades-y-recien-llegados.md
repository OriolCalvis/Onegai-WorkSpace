# D0037 — El resto de identidades por id, y quien llega a mitad del combate (05/11/2026)

Estado: **aceptada** (Oriol, chat del 05/11: «Sí, arregla'ls», «lo del UUID crec que es molt important»). Cierra D0036 §5 y sustituye a D0036 §4: ya no queda ningún id sacado de un nombre. Los puntos 2 y 5 son valores por defecto del Brain, a confirmar.

| # | Decisión |
|---|---|
| 1 | Una partida sin UUID estrena uno de verdad al cargarla y se guarda en el mismo fichero. |
| 2 | *Por defecto:* varias partidas antiguas del mismo personaje reciben UUID distintos, porque nada dice que sean el mismo. |
| 3 | Quien llega a mitad del combate (refuerzo o invocado) tiene casilla, la libre más cerca del héroe, y dibujo si es un acompañante. |
| 4 | La forma de las cartas es un dato, en `assets/rules/areas_de_cartas.json`, no el texto de su descripción. Va en el motor porque `skills.json` viene de las cartas y el flujo no vuelve atrás. |
| 5 | *Por defecto:* las 23 filas de la tabla salen de lo que decían hoy las descripciones: nada cambia al jugar. |
| 6 | Los apartados de misión tienen `id`, y las decisiones lo nombran en `section`. |
| 7 | El Códice busca solo por id. Un beat puede nombrar a su personaje con `speakerId`; `speaker` es un rótulo. Hoy ningún beat lo usa, porque los 89 rótulos de la campaña no tienen ficha en `npcs.json`. |
| 8 | Carteles y bocadillos se ocultan por la casilla del aviso; el retrato de espera y el marco del que tira, por índice e id. |
| 9 | Los ids del catálogo («diacono_orfeu») y los del combate («a0») no son UUID, y no hace falta: son estables, no se enseñan y se pueden cambiar. El UUID es para lo que nace jugando, como un personaje. |
