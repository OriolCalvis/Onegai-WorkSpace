# D0030 — Huir no da recompensa y puede costar oro; morir reinicia el combate desde el principio (05/11/2026)

Estado: **aceptada** en 1, 2 y 4 (Oriol, chat del 05/11: «si fuges no hi ha recompenses ni res», «tires els daus a veure si perds diners per la fugida», «Es reinicia el combat des de l'inici, no des de on has mort»). El punto 3 es un valor por defecto del Brain, a confirmar. Sustituye a D0028 §3 y a D0027 §4.

| # | Decisión |
|---|---|
| 1 | Huir no da nivel, hito ni oro. Se quitan de `b3_huida_perdida` y `b3_rodeo_huida` (2 niveles y el hito) y de `tut_huida` (0,6 de nivel y 15 de oro). |
| 2 | Al huir de cualquier combate se tira un dado para ver si se pierde oro. Sin oro no hay tirada. |
| 3 | *Por defecto:* con un 1 se pierde el 30 % del oro; con un 2, el 20 %; con un 3, el 10 %; con 4, 5 o 6, nada. |
| 4 | Morir reinicia el combate desde su primera oleada, con todo como al empezar, aunque se cayera en una oleada posterior. |
| 5 | `b2_saqueadores_ganan` sigue dando el amuleto y el acto `barriada_sangre` (D0029 §3, aceptada), a la espera de que Oriol diga si la regla del punto 1 también le alcanza. |
