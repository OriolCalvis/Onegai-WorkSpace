# PNJ-01 — Tots els PNJ amb les peces del jugador (05/10/2026)

L'Oriol ho va demanar mentre es feien els interiors:

> «Todos los npcs se tienen que generar con las mismas estéticas que podría
> tener un [jugador], o sea, cualquier jugador podría copiar la estética de un
> npc estándar. Los equipos y armas modifican la apariencia, así que al final
> también es difícil que un npc y un aventurero se parezcan en extremo,
> pudiendo crear solo equipamientos para npc.»

Abans, molts PNJ no tenien aspecte propi. La Skilla, l'anciana, la
bibliotecària, la professora, la taquillera i la Xila eren la mateixa «aldeana».
Uns altres eren dibuixos de PixelLab en un altre estil: el banquer, el joier, el
posader, el Griffin, la Luisarda...

## Què hi ha ara

- **250 PNJ:** tots els objectes de categoria `npc` d'`assets/objects`. N'hi
  ha 326 aspectes en total: 76 són variants.
- **Fulls:** `pnj_boundington.png` (106 files), `pnj_mundo_1.png` i
  `pnj_mundo_2.png`, a `assets/textures`. Van els últims a `sprites.json` i
  substitueixen PixelLab i la roba prestada. Els 47 àlies de PNJ s'han tret.
- **Peces noves:** 38 en total.
  - 30 són roba de diari, també per al jugador: vestits, faldilles, armilles,
    casaques, la còfia, la toca, els gorros de llana i les capes.
  - 8 són **només de PNJ**: la llibrea de la guàrdia de Boundington, el
    davantal de ferrer, la casulla, la toga, el birret, la llibrea de palau, la
    cistella i el llibre de comptes.

## Com és un PNJ

- **La base:** és el que ofereix la creació de personatge. Són la raça, la pell
  i la talla (només als 14 pobles de cos llis), els ulls, el pentinat, la barba
  i la roba de diari. Un jugador la pot copiar tal qual.
- **L'equip:** el que porta posat o a la mà (armes, escuts, gerres, fanals...)
  i, en alguns oficis, les peces només de PNJ. `personajes.json` les marca amb
  `"soloPnj"`, la creació no les ofereix i cap objecte del catàleg les vesteix.
- **Les variants:** un PNJ col·locat més d'una vegada té una cara per
  col·locació, fins a sis. Passa amb l'anciana (12), la guàrdia (9), els
  guardians de la llum de la plaça (10)... El motor dona la variant *n* a la
  col·locació *n* del nivell.
- **Sense trampes:** cap PNJ treu capes de la raça ni fa servir una pell que
  no sigui a l'atles.

| PNJ | Com és ara |
| --- | --- |
| Skilla | monyo pèl-roig, brusa i cotilla, faldilla verda, gerra |
| Venides | cabell llarg negre, casaca blava ribetejada, botes altes |
| Aigren | calb, barba pèl-roja, davantal de ferrer, martell |
| Luisarda | cua pèl-roja, vestit vermell amb davantal |
| Xila | cabell blanc, vestit porpra |
| Griffin | pell fosca, calb, samarreta sense mànigues, bastó i fanal |
| Sacerdot | poble dels Guardians de la Llum, casulla blanca i or, orbe |
| Professora | toga i birret, grimori |
| Guàrdia | llibrea granat amb la creu d'or, xapell, llança (i variants amb escut) |

**D'on surt cada composició** (`tools/actores_pnj.py`):

- **A mà:** els de Boundington i els de les quatre nacions de l'oest, amb la
  seva fitxa o el seu retrat de PixelLab.
- **Per regles, la resta:**
  - l'ofici, de l'id, del nom o de la carta d'`assets/catalogs/npcs.json`;
  - el poble, del `raceId`, de la nació de l'id o del lloc de la carta;
  - el sexe, del nom;
  - i una llavor de l'id.

Els pobles de l'est (Choubar, Mistarium, Nocturnsea) no tenen raça a
`races.json`. Hi he posat humans amb una paleta de pell pròpia per a cada un.
Gongorguma són orcs.

## Al motor

Hi ha tres canvis petits. Cap toca els quatre fitxers que l'Oriol té oberts.

- `Game::ManifiestoPersonajes` llegeix `soloPnj`, i `Game::EditorApariencia`
  no ofereix aquestes peces, ni recorrent les files ni amb «AL AZAR».
- `Game::RegistroSprites` llegeix les `variantes` de cada fila
  (`SpriteActor::filaPara`).
- `ApplicationNivel.cpp` compta les col·locacions de cada PNJ en carregar el
  nivell.

**Cal recompilar.**

## Fitxers

- `pnj_01_antes_ara.png`: els 49 PNJ col·locats a Boundington, abans i ara.
- `pnj_02_prendas.png`: les peces noves (les de PNJ i les de tothom).
- `pnj_03_boundington.png`: el full de Boundington amb totes les variants.
- `pnj_04_mundo.png`: la resta (PNJ de raça, est, oest, llibreria).
- `pnj_05_joc_taberna.png`, `pnj_06_joc_placa.png` i `pnj_07_joc_oficis.png`:
  captures del joc. Surten la taverna, la plaça del centre amb les variants
  (tres guardians de la llum diferents), el castell, l'església, la
  universitat i el barri de l'est.

## Com es refà

```
python3 tools/gen_personajes.py      # tots els atles i els PNJ (tools/gen_pnj.py)
python3 tools/validar_pnj.py         # també el fa validar_todo.py
python3 tools/probador_personaje.py --personaje anciana~3 --gif
```

Un PNJ nou d'un catàleg surt sol en regenerar, per regles. Si té història,
s'escriu a `actores_pnj.PNJ`.

## Proves

- **`test_personajes_capas`:** el motor munta **cadascun dels 326 aspectes**
  byte a byte amb el mateix codi que el jugador (i els 62 de sempre). Ara triga
  uns 13 s.
- **`test_registro_sprites`:** cada PNJ té la seva fila, les variants, i els
  àlies i les races per a ids nous.
- **`test_editor_apariencia`:** la creació no ofereix les peces de PNJ.
- **Altres:**
  - `test_project_catalog` i `test_object_catalog` passen.
  - `validar_todo` dona 0 errors.
  - `linea_base_core` es queda amb els 35 deutes de sempre.
  - La resta de fallades són les de sempre.
- **Ja fallava abans:** `test_validar_sprites.py` té una prova de
  reproductibilitat que ja fallava abans de PNJ-01, per les peces de PixelLab
  del lot 7.
