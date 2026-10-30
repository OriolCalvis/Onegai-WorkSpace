# PixelLab · Lots 3 i 4 (01/10/2026) — terreny i props

**92 generacions més (125 de 2.000 en total; en queden 1.875).**

## Lot 3 · Terreny: 66 de les 76 rajoles de `terreno_mundo.png`

Totes amb `create_isometric_tile` (64, "thin tile", 1 generació cadascuna; 3 es
van repetir perquè la primera versió no servia: sendero_tierra sortia de maó,
adoquin_limpio massa fosc, rio amb roques). Les 10 que queden planes són
duplicats «viejo_*» que cap mapa fa servir gaire (viejo_adoquin, viejo_agua,
viejo_marmol, viejo_tierra, viejo_arena, viejo_adoquin_fino, viejo_carril,
viejo_barro, viejo_rio) i `plaza`.

`06_tileset_terreno_mundo.png` ensenya les 76 cel·les amb el seu gid i nom.

### L'arreglo important: el tileset sortia en mirall

`ApplicationNivel::loadLevel` definia la regió de cada gid amb la fila de
lectura del PNG, però `TextureManager` inverteix els PNG en carregar-los (la
resta d'atles ho compensen amb `rows-1-row`). Amb `terreno_mundo.png` de 10
files, cada gid dibuixava la cel·la de la fila mirall: la pradera (gid 1)
sortia com la chabola (gid 73), l'empedrat com el banc... Els mapes es van
generar **per nom** (`tools/remapear_suelos.py`: cesped → pradera), així que
el que es veia no era el que deien les dades. Arreglat al joc i a l'editor
antic (`editor/src/level_editor.cpp`). **Tots els mapes canvien d'aspecte** al
que deien les seves dades: la plaça de Boundington és d'herba, els carrers
d'empedrat, els edificis amb els seus terrats. Mira `01_plaza.png`,
`02_ciudad_centro.png`, `03_mapamundi.png`, `04_umedan.png`, `05_taberna.png`.

## Lot 4 · Props: 24 objectes (`create_map_object`, 1 generació cadascun)

Portes per tipus d'edifici (posada, església, castell, botiga, ferreria,
biblioteca, banys, caserna; les altres cauen a la de taverna per prefix, i
mercat/joieria/sastreria/banc/casa de te → botiga, òpera/universitat →
biblioteca, coliseu/armeria → caserna per àlies), cartell penjant (tots els
`cartel_*` d'edifici), roure, pi, arbre sec, fanal, banc de plaça, font, carro,
barrils, caixes, i el mobiliari (taula, cadira, llit, prestatgeria, cofre,
foguera) que **substitueix** el de `props_p0` només per tenir el mateix id en
una fulla posterior de l'índex.

A la plaça de Boundington hi ha ara la font, dos fanals, un banc, dos roures i
un carro (catàleg `assets/objects/pixellab_objetos.json`, col·locats a
`ciudad_centro.json`). `07_props.png` ensenya els 24.

## Comprovacions

`test_registro_sprites` 5/5, `test_object_catalog`, `test_validar_todo`
(connectivitat: 55 portes de la ciutat assolibles), `test_pergamino` 37/37; la
resta de `ctest -L gl-free` amb els mateixos fallats de sempre; `linea_base_core`
0 instantanis (el fitxer de l'editor antic queda amb les mateixes línies).

## Pendent

- Mobiliari als interiors (la taverna no té objectes col·locats).
- Transicions entre sòls (`create_topdown_tileset`) si es volen.
- Pickups (pocions, claus, armes) i monstres: lots següents.
