# D0028 — El final «caído» del Ocaso pasa a ser una huida (05/11/2026)

Estado: **aceptada** (Oriol, chat del 04/11: elige «Reescriure'l com a fugida»). Consecuencia de D0027: morir reinicia el combate, así que a este final solo se llega huyendo.

| # | Decisión |
|---|---|
| 1 | Los beats `b3_caida` y `b3_rodeo_caido` cuentan que el héroe huye a la carrera, no que cae. Se renombran `b3_huida_perdida` y `b3_rodeo_huida`; el epílogo, `b3_epilogo_huido`; la flag, `bnd3_final_huido`. |
| 2 | Huir también es salir vivo: enciende `bnd3_salio_vivo` y el diario marca «Salir de Boundington». |
| 3 | *Por defecto del Brain, a confirmar:* da el nivel y el hito de salir vivo como las otras salidas, pero no el mapa ni la ayuda de quien te acompaña. |
| 4 | El diario dice «Huiste de Boundington a la carrera.». El resto de ramas de `REVIVIR_1.md` con texto de derrota siguen sin tocar, a la espera de Oriol. |
