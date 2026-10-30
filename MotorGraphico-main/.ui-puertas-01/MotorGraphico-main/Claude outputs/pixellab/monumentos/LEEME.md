# Monuments amb sprite propi (01/10/2026, BLOQUES-03)

**16 generacions més (352 de 2.000 en total; en queden 1.648).**

El coliseu, l'església, el castell, la caserna, l'òpera, la universitat i la
biblioteca ja no són blocs genèrics: cadascun és **un sprite gran de PixelLab**
que es veu sencer des de fora i s'ordena bé amb la gent que hi passa pel
davant i pel darrere.

## Com funciona

- `Game::Monumentos` (sense GL) llegeix `assets/textures/monumentos.json`: per
  cada monument, el tile que el forma al mapa i la seva **empremta** (ample ×
  alt en cel·les: coliseu 8×6, església 10×9, castell 13×10, caserna 11×8,
  òpera 10×9, universitat 10×9, biblioteca 12×9). Si al mapa l'edifici fa
  exactament aquesta mida, es dibuixa amb el sprite; si no (l'armeria, que és
  `viejo_militar` de 7×5), continua sortint amb blocs.
- El sprite es talla en **tires de 32 px** d'ample i tot l'alt; cada tira és una
  entitat recolzada a la cel·la de l'empremta que està **més endavant** en
  aquella columna de pantalla, i s'ordena just abans del que trepitgi aquella
  cel·la. Així un PNJ que passa per davant tapa l'edifici, un que passa per
  darrere queda tapat, i el jugador al llindar de la porta es veu davant de la
  façana. Cap canvi al renderer: és la mateixa cua ordenada per x+y.
- `MonumentosNivel::atenua()` fon només les tires que de debò cauen sobre el cos
  del jugador quan és darrere de l'edifici (`05_coliseo_en_juego_puerta_y_detras.png`
  a baix); a la porta no se'n fon cap.
- Les cel·les de l'empremta no aixequen blocs (`Bloques::planifica` rep les
  cel·les excloses). El llindar de la porta i l'objecte `puerta_*` segueixen
  igual.
- `tools/pixellab/importar_monumentos.py`: retalla el marge, escala el dibuix a
  l'amplada de l'empremta ((ample+alt)·32 px, veí més proper), fa la fulla de
  tires (`assets/textures/monumentos/<id>.png` + `.json`, registrada a
  `sprites.json` com `monumento_<id>`) i posa l'ancoratge al punt més baix del
  centre (la cantonada davantera de l'empremta).

## PixelLab i el fons

`create_image_pixen` a 512×512 amb `no_background` dóna el millor art
(coliseu, església, òpera, universitat, biblioteca, caserna), però sovint
pinta un **terra de rajoles** al voltant de l'edifici. Es treu per inundació
des de les vores amb tolerància de color (el terra és d'una família de colors
diferent de l'edifici) i quedant-se amb la component opaca més gran. Quan el
terra i les parets són del mateix gris (castell) no es pot: el castell és
`create_map_object` 400×400 (fons gris pla, art més senzill). Les proves que no
han servit queden anotades a `lote10/generaciones.json`.

## Verificació

`test_monumentos` 3/3 (geometria de les tires: cada tira cau a la cel·la més
endavant de la seva columna; només les empremtes de la mida declarada; cel·les
cobertes), `test_bloques` 5/5, `ctest -L gl-free` només amb els fallats de
sempre, `linea_base_core` sense deute nou (la implementació de
`MonumentosNivel` viu a `src/Render/BloquesNivel.cpp` per no afegir una font
més sense GL a `motor_core`).

**Cal recompilar.** `aplicar_bloques3.sh` insereix dues línies de CMake amb
àncora: `src/Game/Monumentos.cpp` sota `src/Game/Bandada.cpp` i
`include(cmake/MonumentosTests.cmake)` sota `include(cmake/BandadaTests.cmake)`.

## Pendent

- Castell més bonic (el pixen amb torres va sortir amb terra que no es pot
  netejar): 1-2 generacions més amb un altre fons, o retocar-lo a mà.
- Armeria (7×5) i banys públics amb sprite propi; les cases de te i posades
  grans també podrien ser-ho.
- Interiors d'aquests edificis a joc (són els de sempre).
