# Esbossos del Senyor de les Armes (08/10/2026)

L'Oriol vol canviar l'aspecte del Senyor de les Armes: armadura d'or, mana vermell que brilla, armes
que suren i aura daurada, amb un disseny propi. El set d'abans era un manto carmesí amb armini,
una corona d'espases, quatre espases que suren i l'orbe. Aquí hi ha tres propostes. L'Oriol: «estan perfectes els 3». **Integrades totes tres com a SETS-04** (08/10):
ara la classe té tres sets, el Set del Duque, el Set del Soberano sin Rostro i el Set del Rey de la Forja.
Mira `sesiones/2026-10-08-claude-sets04.md` i `out/claude_sets04/`.

- `esbossos_senyor_armes.png`: les tres, quietes (de cara, de perfil i d'esquena) i caminant.
- `esbossos_senyor_armes.gif`: les tres caminant en les quatre direccions.

La brillantor i les espurnes de la làmina i del GIF són una aproximació de les partícules del motor.
No surten del motor (`codi/fx.py`).

## Les propostes

1. **El Duc**
   - Casaca negra tancada, de coll alt, amb els cantells i els botons d'or.
   - Espatlleres d'or (la dreta, gran) i línies de mana vermell als braços.
   - Cinturó ample d'or amb la placa de gemma fosca, peces d'or i tires vermelles als malucs.
   - Capa vermella que neix de la cintura.
   - Calces negres amb un rombe d'or al genoll i botes negres amb la greva d'or.
   - Cabell blanc llarg, ulls violeta i la diadema amb el robí.
   - Quatre fulles suren als costats, amb el tall vermell. L'orbe, d'or amb el nucli vermell.
2. **El Sobirà sense rostre**
   - Elm negre tancat amb la corona d'or de cinc puntes d'espasa, la cresta d'or i l'escletxa en T
     que brilla de vermell.
   - Placa negra amb els cantells d'or, tres plaques d'or a cada espatlla i el tabard vermell amb la
     corona d'espases.
   - Capa llarga vermella i quatre fulles espectrals vermelles al voltant.
3. **El Rei de la Forja**
   - Ferro fosc amb esquerdes d'or fos i els artells dels guantellets roents.
   - Corona de fulles trencades amb les puntes roents.
   - Un espadó a l'esquena, acabat de forjar, amb el tall que encara brilla.
   - Cabell i barba de coure.

## Com es tornen a fer

Des de `codi/` (troba `tools/` a `MotorGraphico/tools`; si no, cal la variable `MG_TOOLS`):

    python3 lamina.py    # ../esbossos_senyor_armes.png
    python3 gif.py       # ../esbossos_senyor_armes.gif

`codi/senor.py` té les paletes (sense tons repetits amb cap altra) i les peces de les tres
propostes. Les registra als catàlegs d'`actores_equipo` i `actores_cascos` només en memòria: no toca
cap atles.

## Integració

Feta (SETS-04). Les peces són a `tools/actores_trajes_iteracion3.py` (secció B), les dades a
`tools/sets_iteracion3.py` i el tema a `tools/actores_estetica.py`. El codi d'aquesta carpeta és el dels
esbossos. Amb el repo d'ara, `senor.py` xoca amb les paletes integrades: per refer els esbossos, cal
la versió de `tools/` d'abans de SETS-04.
