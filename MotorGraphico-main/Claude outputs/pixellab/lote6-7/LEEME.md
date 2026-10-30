# PixelLab · Lots 6 i 7 (01/10/2026) — PNJ amb cara i roba de creació

**98 generacions més (221 de 2.000 en total; en queden 1.779).**

## Lot 6 · 12 PNJ amb cara pròpia (61 generacions)

Ben Kafka, Griffin, el follet de porcellana, la saga del bosc, el botxí, la
nena del gat, el cavaller, el mercader, Skilla, Venides, Aigren i Xila: cadascun
amb `create_character` (4 direccions, 1 generació) + `animate_character` walk
(6 frames × 4 direccions, 4 generacions). L'importador tria les dues
gambades per obertura de cames i munta la fila de 12 frames. Tots
substitueixen el personatge per capes del mateix id només per ser a la fulla
`pixellab_personajes` (14 files ara, amb el slime i la Luisarda del lot 0).

`01_pnj_14_filas.png` ensenya el frame quiet de cadascun; `02_juego_taberna.png`
la taverna amb Skilla, Venides, Ben Kafka i Griffin dins del joc.

Pendents: la naga del riu continua per capes (cos sense cames: el model
humanoide no serveix); la nena del gat ha sortit adulta (repetir amb
proporcions infantils); retrats per al diàleg (20 generacions cadascun) si
els vols.

## Lot 7 · 12 peces de roba per a la creació (37 generacions)

8 de tors (gipó de cuir, cota de malla, túnica blava, vestit verd amb davantal,
camisa amb armilla, armadura de plaques, túnica de mag, abric de pells) i 4 de
cap (barret d'ala ampla, caputxa verda, elm d'acer, corona de flors).

Mètode (validat al lot 0): `edit_image_pixen` **vesteix el cos base** en les
tres vistes quietes (avall, dreta, amunt; 1 generació cadascuna) i
`tools/pixellab/importar_prendas.py` n'extreu la peça per diferència amb el
cos (píxel nou o canvi de color > 12), la posa a les 4 talles (escalant des
dels peus) i a `sin_piernas`, i la dóna d'alta a `personajes.json` (origen
`pixellab`). L'esquerra és el mirall de la dreta; els frames de pas porten la
peça baixada 1 px on el cos base baixa el tors. Un tors no toca el cap i un
tocat no baixa dels muscles (PixelLab retoca la cara i els peus en vestir).

Les peces amb el mateix id que una de procedimental porten el sufix `_pl`:
**conviuen** i la creació ofereix totes dues (no es toca l'art dels
personatges d'història enfornats; `test_personajes_capas` continua verd).
`tools/gen_personajes.py` les reposa sol en acabar de regenerar els atles.

`03_prendas_ediciones.png` (el que ha tornat PixelLab), `05_prendas_capas_12_frames.png`
(les capes muntades) i `04_creacion_apariencia.png` (el pas Aparença amb les
peces noves sobre un aarakocra).

Pendents: cames i peus continuen procedimentals; les mànigues no segueixen el
balanceig del braç (1–2 px als frames de pas; si molesta, demanar les 12
vistes); més personalitzacions de base per raça i complements.

## Comprovacions

`test_registro_sprites` 5/5, `test_personajes_capas` 7/7, `tools/validar_sprites.py`
conforme; la resta de `ctest -L gl-free` amb els mateixos fallats de sempre;
`linea_base_core` 0 instantanis.
