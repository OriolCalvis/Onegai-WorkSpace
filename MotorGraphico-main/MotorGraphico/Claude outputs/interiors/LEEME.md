# INT-01 — Els interiors de Boundington amb el kit (05/10/2026)

L'Oriol demana «els furnitures de la ciutat i interiors dels edificis». Va triar:

- **Quins:** els 17 interiors que ja hi havia, cadascun més o menys gran segons
  la seva importància.
- **Plantes:** els edificis importants han de ser força grans i tenir com a
  mínim dues plantes.
- **Estil:** va veure el prototip (la taverna i el saló del tron) i va dir
  «sigue así».

Abans, els 17 interiors eren caixes buides de 13×11 amb el tender i la sortida.

## Com és una sala

- **Com a Habbo:** les dues parets del fons (la fila y = 0 i la columna x = 0),
  primes i altes, i el davant obert.
- **Parets:** cada cel·la de paret és un objecte, `paret_<estil>[_alta]_<decoració>`
  i `_o3` per a la columna. Les decoracions són finestra, quadre, prestatge,
  aplic, torxa, tapís, escut, llar de foc, llibreria, armes, mapa, pissarra, teló,
  reixa, vitrall, nínxol i porta.
- **Estils de paret:** fusta amb guix, estuc, paper pintat (vermell, blau, verd),
  pedra, pedra fosca, rajola, maó. Les sales grans tenen parets de 136 en lloc
  de 104.
- **Terres:** rajoles noves de `terreno_mundo` (`int_*`): taulons, parquet
  d'espiga, lloses, marbre a escacs, rajola, terra batuda, sorra, tatami. També
  catifes vermella, blava i verda de qualsevol mida, en nou peces.
- **Sortida:** la porta és a la paret. L'objecte de la sortida
  (`salida_interior_*`) és l'estora de davant, per àlies.
- **Plantes:** cada planta és un nivell. Les escales porten d'una a l'altra amb
  E, com les portes. Cada escala té la seva id, `escalera_<planta>_<sube|baja>`
  (per exemple `escalera_castillo_p1_sube`), perquè les plantes del mig en tenen
  dues i cada una es dibuixa diferent.
- **Peces grans:** van com els monuments, sobre rajoles `int_peca_*`, i el motor
  les dibuixa a tires. Són la tarima del tron, l'escenari de l'òpera, l'altar,
  la piscina, la forja, la taula de guerra i el llit amb dosser.

## Els edificis

| Edifici | Plantes | Sales (cel·les) |
| --- | --- | --- |
| Taverna Humil | 2 | sala amb la barra i la llar de foc 14×11 · habitacions 14×9 |
| Posada | 2 | rebedor i menjador 15×11 · habitacions 15×10 |
| Mercat | 2 | nau de parades 17×12 · magatzem 14×9 |
| Banc | 2 | sala del taulell amb reixa 13×10 · cambra cuirassada (soterrani) 10×8 |
| Banys | 2 | termes amb la piscina 16×12 · sala de vapor 11×8 |
| Ferreria, armeria | 1 | 11×9 cadascuna |
| Joieria | 1 | 9×8 |
| Sastreria | 1 | 10×8 |
| Casa de te | 1 | 10×9 |
| Castell | 3 | saló del tron 22×16 · sala de guerra 16×12 · aposents reials 14×10 |
| Església | 2 | nau en ruïnes 20×14 · cripta dels Perduts 13×9 |
| Òpera | 2 | platea amb l'escenari 21×15 · camerinos 16×10 |
| Universitat | 2 | aula magna 19×14 · laboratori i observatori 15×10 |
| Biblioteca | 2 | sala de lectura 19×14 · arxiu prohibit 15×10 |
| Coliseu | 2 | galeria dels gladiadors 18×12 · fossats 13×9 |
| Quarter | 2 | menjador i armeria 17×12 · dormitori 15×10 |

En total hi ha 30 plantes: 17 de baixes i 13 de noves. Els PNJ que hi havia
(la Skilla, el Venides, l'Aigren, la Luisarda, els tenders...) hi són, cadascun
on li toca. Cap dels canvis afecta les aventures, perquè els seus «enter» van
per nivell. Les plantes baixes recuperen el nom («Taberna Humilde»,
«Armeria»...): moltes es deien «Los Perdidos de Boundington».

El Vacío (el pròleg) feia servir el mapa de l'església. Ara té el seu propi
mapa, `interior_vacio.tmx`, i és la mateixa sala d'abans.

## Els mobles

Són 145 peces del kit, a la manera dels «furni» d'Habbo, en tres mòduls:

- **`tools/iso/mobles_comuns.py`:** taules, cadires, llits, armaris, llums,
  escales.
- **`tools/iso/mobles_oficis.py`:** taverna, posada, mercat, ferreria, armeria,
  banc, joieria, sastreria, casa de te, banys.
- **`tools/iso/mobles_monuments.py`:** tron, armadures, bancs d'església, butaques,
  piano, pupitres, esfera armil·lar, alquímia, gàbies, lliteres...

Els que tenen sentit girats surten en 4 orientacions. Els llargs (barres,
taulells, taules, bancs d'església, llits, lliteres, sarcòfags) van en trams. Els
trams porten el contorn compartit, perquè la juntura no es vegi.

## Fitxers

- `int_00_mobles_a.png` i `_b.png`: tots els mobles.
- `int_01_peces_grans.png`: les set peces grans.
- `int_02_hud_*.png`: captures del joc amb el jugador i el HUD.
- `int_03_escala_taberna_dalt.png`: la taverna després de pujar les escales.
- `int_10_taberna.png` a `int_26_cuartel.png`: totes les plantes de cada edifici,
  capturades al joc.

## Com es refà

```
python3 tools/iso/fer_interiors.py            # tot: terres, mobles, rajoles, peces grans i sales
python3 tools/iso/fer_interiors.py taberna    # la cadena, i les sales només d'un edifici
```

Els plans de cada edifici són a `tools/interiors_kit/` (un mòdul per edifici o
grup). El generador és `tools/gen_interiores_kit.py`, i comprova:

- que de l'entrada s'arriba a tot arreu, i que no hi ha cap racó tancat;
- que es pot parlar amb cada PNJ i arribar a cada escala;
- que tots els objectes són al catàleg.

Si falla, mostra el pla en text.

## Proves

- `test_registro_sprites`, `test_project_catalog`, `test_monumentos`, `test_bloques`,
  `test_object_catalog`, `test_mascotes` i `test_bandada` passen. La resta de fallades
  són les de sempre.
- `validar_enlaces`: 0 problemes. `validar_mundo_unido`: s'arriba a 106 nivells i de
  tots es torna.
- `linea_base_core`: igual, amb 35 deutes.
- **L'únic canvi de C++ és una prova,** `tests/Game/RegistroSpritesTests.cpp`:
  - La sortida de l'església ara és l'estora del kit, per àlies.
  - `mesa` i `barril` ara són de `kit_mobles`.
  - Comprova el mateix amb ids que no han canviat.
