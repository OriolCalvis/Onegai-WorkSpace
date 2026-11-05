# D0031 — De los combates importantes no se huye (05/11/2026)

Estado: **aceptada** (Oriol, chat del 05/11: «Fer que no puguis fugir en combata importants», y sus respuestas: «Caps i rescats», «Que surti en morir», «No es pot fugir d'aquí»). Deja sin efecto D0029 §3 y D0030 §5: la rama de huir de la Barriada ya no se alcanza.

| # | Decisión |
|---|---|
| 1 | Son ineludibles 20 combates. Caps: Sandes (5), el diácono Orfeu (1) y Edul furioso (2). Rescates: Verina (5), Edul (3) y Venides (1). Y los saqueadores de la Barriada (3). |
| 2 | Quien cae en ellos revive al inicio del combate, con cicatriz y coste moral (D0027). No hay límite de reintentos. |
| 3 | La antigua compañía de Rosita llega al morir contra Sandes, la primera vez, si se cumplen los requisitos de `b3_compania`; ese combate se cierra sin cicatriz. Nuevo campo `rescateAlMorir` en `startBattle`. |
| 4 | `b3_compania` vuelve a empezar por «Vas a morir»: sobra la frase de huida de D0029. |
| 5 | Quedan sin poder alcanzarse por estos combates: `bgd_perdida`, `b2_saqueadores_ganan`, `b3_sandes_gana_casco`, `b3_sandes_sigue`, `b3_verina_perdida` y `b3_edul_perdido`: `bnd3_verina_muerta`, `bnd3_edul_perdido` y `bnd3_edul_gana` solo los encendían estos combates. Sus textos no se borran. |
