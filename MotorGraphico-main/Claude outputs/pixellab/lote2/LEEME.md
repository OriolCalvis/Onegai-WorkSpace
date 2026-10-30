# PixelLab · Lot 2 (01/10/2026) — icones

**60 generacions (281 de 2.000 en total; en queden 1.719).** `create_image_pixen`
a 16×16 (1 generació cadascuna) per a les 56 icones d'`iconos.json` més 4 de prova.

Resultat honest: a 16 px PixelLab només guanya en pictogrames simples. **Entren
22** (cor, gota de manà, moneda, mà, àncora, llamp, gota de sang, fletxes amunt
i avall, bandera, check, creu, casa, pergamí, interrogant, clau, poció, parada,
bosses de comprar i vendre, retrat, cargol); les altres 34 (ranures d'equip,
condicions amb cara, stats...) van sortir com taques taronges i es queden amb
el dibuix a mà de `gen_iconos.py`. `01_antes_y_todo_pixellab.png` ho ensenya
(esquerra: a mà; dreta: tot PixelLab); `02_atlas_final_mezcla.png` és l'atles
que ha quedat; `03_juego_cuaderno.png` el quadern al joc.

Com entra: `assets/textures/pixellab/iconos.json` diu quines entren (i quines
es descarten); `tools/pixellab/importar_iconos.py` les pinta a la cel·la del
seu id sense tocar el contracte `iconos.json`; `gen_iconos.py` les reposa sol
en regenerar.
