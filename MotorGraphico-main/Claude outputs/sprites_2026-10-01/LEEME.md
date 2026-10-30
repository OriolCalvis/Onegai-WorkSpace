# SPRITES-01 — les fulles de sprites passen a ser dades (01/10/2026)

Primer pas del pla PixelLab (`Claude outputs/pixellab/PLA_PIXELLAB.md`): abans
que arribi cap dibuix nou, el motor ha de poder rebre una fulla de sprites
afegint una línia a un índex, no una taula en C++.

## Què canvia al joc

- `assets/textures/sprites.json` és l'índex: quines fulles hi ha (els mateixos
  manifestos que generen `tools/gen_personajes.py` i `gen_props_p0.py`), els
  àlies (id del catàleg → id de la fulla) i els patrons per prefix o sufix
  (`puerta_` → porta de taverna, `_tendero` → mercader, `cartel_` → tauló).
- `Game/RegistroSprites` (sense GL) llegeix l'índex; `Render/BancoAtlas` puja
  cada fulla una sola vegada i en retalla una fila (actors, 12 frames) o tota
  la fulla (props i pickups).
- `ApplicationNivel::makeMarker` pregunta al registre: els actors amb fulla
  surten vestits i animats (idle/walk), els props i pickups surten amb el seu
  dibuix en lloc d'un tile tenyit. El que no té dibuix es queda com abans.
- 42 PNJ col·locats als nivells sense fulla pròpia (sacerdot, hostaler,
  estudiant, nen, bard, carterista...) porten **roba prestada** de les fulles
  que hi ha, perquè ningú surti despullat. Cada àlies es treu quan PixelLab
  doni cara a aquell PNJ (lot 4 del pla).

## Captures (joc real, Mesa + GLFW guionitzada)

| | |
|---|---|
| `01_ciudad_junto_a_la_anciana.png` | Portes i cartells amb dibuix; l'Anciana vestida d'aldeana; l'estudiant i el nen amb roba prestada. |
| `02_puerta_de_la_sastreria.png` | La porta de la sastreria amb el seu sprite i el cartell al costat. |
| `03_interior_sastreria.png` | L'interior: la porta de sortida i la dependenta vestides. |

## Comprovacions

- `test_registro_sprites` 5/5 (índex real sense avisos; cada PNG mesura el que
  diu el seu manifest; àlies, sufix, raça i prefixos; un índex trencat avisa i
  no tomba el joc).
- `juego` compila sobre la foto del Mac del 01/10 (10:08). `ctest -L gl-free`:
  els mateixos fallats que sense aquest canvi.
- `linea_base_core`: 0 instantanis; les 35 línies de "deuda nueva" ja eren a
  l'arbre del Mac abans d'aquest canvi (comprovat sobre l'arbre sense tocar).

## Pendent (el pla segueix)

- Monstres: `slime` i els 431 del catàleg continuen amb el cos base fins al
  lot 2 de PixelLab.
- Commit: `cmake/RegistroSpritesTests.cmake` i els fitxers nous van al mateix
  commit que les seves línies de CMake (ho demana `linea_base_core`).
