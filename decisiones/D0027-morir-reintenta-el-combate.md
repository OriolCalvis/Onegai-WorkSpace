# D0027 — Morir: se revive al inicio del combate, con cicatriz y coste moral (04/11/2026)

Estado: **aceptada** en 1–3 (Oriol, chat del 04/11: «cada cop que morim deixem reviure el jugador, al inici del combat»; «evidentment s'aplica cicatriu i baixa la visio estetica de la gent cap a tu afectant la moralitat»). Los puntos 4–6 son valores por defecto del Brain, a la espera de que Oriol los confirme. Sustituye a D0015 §7 («Game Over y vuelves al ultimo punto de guardado») y a CAER-01 en lo que contradigan.

| # | Decisión |
|---|---|
| 1 | Cada vez que el héroe muere en un combate, **revive al inicio de ese mismo combate**: el combate vuelve a empezar. Vale para los de la historia y los del mapa. No hay pantalla de «Has caído» con cargar partida. |
| 2 | Cada muerte deja **una cicatriz más** (el `Game::Cicatrices` que ya existe, fuera de la partida guardada). |
| 3 | Cada muerte **baja cómo te ve la gente y eso baja la moral**. |
| 4 | *Por defecto:* el combate se reinicia en la oleada donde caíste, con PV, estados, objetos, oro y curas de los acompañantes como al empezarla; los enemigos, también. |
| 5 | *Por defecto:* morir no enciende `flagDefeat`; esas ramas solo se alcanzan **huyendo** (como ya hacía CAER-01). Una rama cuyo texto cuente una derrota física se lista como hallazgo; no se reescribe historia. |
| 6 | *Por defecto:* las peleas `sinMatar` (10, donde nadie muere) siguen igual: pierdes, te levantas y la historia sigue, sin cicatriz. El coste moral es de gravedad `menor` con el sistema de `moral.json`. |
