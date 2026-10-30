# El riu de Boundington, molls, vaixells i vegetació (01/10/2026, BLOQUES-04)

**18 generacions més (370 de 2.000 en total; en queden 1.630).**

Segons el mapa original de Boundington, un riu travessa la ciutat d'oest a
est i el comerç fluvial amb vaixells de vela de màgia de vent és la manera
més ràpida d'abastir-la. Ara el riu hi és, afegit sobre la ciutat que ja
teníem, sense redistribuir-la:

- **Centre:** l'avinguda est-oest és ara el riu (tres cel·les d'aigua, files
  29–31) entre dos molls de pedra (files 28 i 32); les portes oest i est no
  s'han mogut (fila 32). El pont de l'avinguda nord-sud el creua (4 cel·les
  d'ample, amb pretil). Moll de càrrega del Distrito Comercial a la riba nord
  aigües avall del pont (grua, fardells, bol·lards, barrils, dos vaixells de
  vela i un pontó); amarradors humils del Casco Antiguo a la riba sud (barques,
  pontó, xarxa de pesca). La font s'ha quedat al peu del pont.
- **Oest:** el riu fa de fossat del Barrio Militar (moll de pedra al nord,
  riba de terra a Pico Dragón, pont de la calle del cuartel) i s'hi uneix el
  braç que ja baixava cap a Surysal pel marge oest. Substitueix l'empalissada.
- **Est:** el passeig del riu entre les calles 22 i 34: jardins a les dues
  ribes, moll amb balustrada de setos, pont de l'avinguda, pontó i barca.
- **Surysal:** sense canvis d'estructura; salzes, joncs i barques al vado.
- **Vegetació:** salzes (grans, 128 px), fruiters, mates, flors silvestres
  sobre herba i terra, jardineres sobre pavimentat, joncs només a la vora de
  l'aigua. `vegetar()` només posa el que bloqueja el pas en cel·les
  despejades (7 de 8 veïnes lliures) perquè no tanqui mai un carreró.
- Les sortides que eren al cruïlla (ara el pont) s'han mogut: la plaza de las
  razas darrere la porta nord (32,1) i el camí a Egaroth per la porta sud
  (32,62); els seus nivells tornen a apuntar-hi.

## Com funciona

- Les baldoses són planes, així que el **mur del moll** (la cara que la càmera
  veu) va pintat dins de la baldosa d'aigua que té davant:
  `tools/pixellab/componer_rio.py` compon `rio_muro_izq/der/ambos` (banda de
  pedra sota el vorell superior esquerre/dret del rombe, prolongada a tota
  l'amplada perquè no surti a dents de serra) i `puente_pretil_izq/der`
  (tauler amb pretil al costat llunyà). `gen_ciudad_rio.orillas()` tria la
  variant de cada cel·la d'aigua segons si el veí de dalt (-x o -y) és pedra,
  pont o muralla; vora herba o terra l'aigua no porta mur (riba natural).
- `tools/pixellab/importar_tiles.py` ara admet `"nuevas"`: baldoses que es
  donen d'alta al final de l'índex (gid 77–84) i fan créixer
  `terreno_mundo.png` per files (ara 1024×704, 84 tiles). El motor reparteix
  els gids per les files reals de la textura, així que els mapes vells
  funcionen igual; `tools/actualizar_tileset_tmx.py` deixa els 92 TMX amb el
  tilecount i l'alt nous perquè l'editor i Tiled no es queixin.
- Els **vaixells** van sobre l'aigua a posta: al catàleg porten
  `"flotante": true`, i tant `gen_ciudad.py` com `tools/conectividad.py` ho
  saben (no es recol·loquen ni compten com a portes inabastables).
- **Props grans:** hoja nova `pixellab_props_grandes` (cel·les de 128×128,
  base a la fila 125) per als vaixells, el pontó, la grua i el salze.
  `Application::loadLevel` dibuixa cada hoja fixa a la mida de la seva
  cel·la, centrada i amb la base a la base del rombe (abans tot era 64×64).
- `Game::Monumentos::planifica` admet diverses definicions pel mateix tile,
  per mida i amb variants (hash estable de la posició): és la base del lot de
  cases ARTE-03 que ve a continuació (test_monumentos 4/4).
- L'eina `tools/pixellab/limpiar_fondo.py` treu el fons pla que a vegades
  pinta `create_image_pixen`.

## Verificació

`conectividad.py` 100 % als quatre mapes, `validar_enlaces.py` 0 problemes,
`validar_todo.py` OK (12 avisos de sempre), `test_monumentos` 4/4,
`test_registro_sprites` 5/5, `ctest -L gl-free` només amb els fallats de
sempre (demo_creacion_personaje espera 61 classes i n'hi ha 63; demo_nova_partida
intermitent; test_gdb12_screens, test_g14, test_catalog_content,
test_marcar_paquetes, linea_base_core amb els 35 deutes previs).

**Cal recompilar** (canvien `src/Engine/ApplicationNivel.cpp` i
`src/Game/Monumentos.cpp`; cap línia nova de CMake).

## Pendent

- Aigua animada (ara és una baldosa fixa amb dues variants).
- Els vaixells no es mouen; més endavant una barcassa que vagi amunt i avall.
- Lot de cases ARTE-03 (ja generant-se): les cases i locals amb sprite propi,
  coherents amb els monuments.
