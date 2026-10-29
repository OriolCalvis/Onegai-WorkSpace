# Props y pickups P0

Dos atlas **runtime** de celdas de 64×64, 8 columnas, generados por
`tools/gen_props_p0.py` a partir de las láminas de autoría del repo: no se
ha inventado arte nuevo, se ha convertido el que ya había.

| Atlas | Celdas | Sprites | Escala |
| --- | --- | ---: | --- |
| `props_p0.png` | 8×5 | 33 | de mundo: una persona = 40 px, como el cuerpo base |
| `pickups_p0.png` | 8×3 | 20 | de lectura: 1,5-2 veces el tamaño real, para que se vea en el suelo |

```sh
python3 tools/gen_props_p0.py            # PNG + JSON de los dos atlas (~10 s)
python3 tools/gen_props_p0.py --vista    # + out/revision_props_p0.png
python3 tools/validar_sprites.py
```

## Contrato

- Frames 1-based por filas; `frames` en el JSON da el frame de cada `id`.
- Cada dibujo está centrado en x y apoya su base en la **fila 61** de su
  celda, como los pies de los actores: se dibuja a 64×64 con el mismo
  `characterAnchor()`.
- Ningún dibujo toca el borde de su celda (lo comprueba el validador).
- `TextureManager` invierte los PNG al cargarlos: la región del frame `f`
  es `defineRegion(f, (f-1) % 8, rows - 1 - (f-1) / 8)`.

## Cómo se convierte una lámina

1. **Fondo.** Las láminas `editor_*` traen alpha y una rejilla 4×4. Las de
   `assets/generated/authoring/` tienen fondo oscuro liso: se rellena
   desde el borde lo que se parece al fondo y la sombra de suelo del
   dibujo (el motor ya pinta su sombra blob), sin saltar nunca un
   contorno. Así la cara en sombra de un mueble, del mismo marrón que el
   fondo, sigue siendo mueble.
2. **Objeto.** Una semilla (un punto del dibujo, en píxeles de la
   lámina) elige el objeto; para dos dibujos que se tocan, `recorte`.
3. **Reducción.** Media por área con alpha premultiplicado, cobertura al
   50 %, realce suave, paleta de 18 colores sin tramado, borde de la
   silueta oscurecido y rendijas cerradas tapadas. Sin suavizado.
4. **Variantes.** Las pociones de maná, veneno, éter y misión y la llave
   de hierro son el mismo dibujo con otro tinte: una variante de datos,
   no otro dibujo.

## Props

| Frame | `id` | Familia (backlog) | Tamaño | Fuente |
| ---: | --- | --- | ---: | --- |
| 1 | `mesa` | Mesa | 25×24 | interiores v1 |
| 2 | `mesa_larga` | Mesa | 30×24 | interiores v2 |
| 3 | `mesa_redonda` | Mesa | 26×22 | interiores v2 |
| 4 | `silla` | Silla / taburete | 20×28 | interiores v1 |
| 5 | `taburete` | Silla / taburete | 14×15 | interiores v2 |
| 6 | `banco` | Silla / taburete | 23×22 | interiores v2 |
| 7 | `barra_taberna` | Barra de taberna | 37×30 | interiores v2 |
| 8 | `barra_esquina` | Barra de taberna | 39×28 | interiores v1 |
| 9 | `cama` | Cama | 34×30 | interiores v2 |
| 10 | `estanteria` | Estanteria | 29×46 | interiores v2 |
| 11 | `estanteria_botellas` | Estanteria | 33×46 | interiores v1 |
| 12 | `banco_trabajo` | Mesa | 25×24 | interiores v1 |
| 13 | `barril` | Barril / caja / saco | 18×24 | interiores v2 |
| 14 | `caja` | Barril / caja / saco | 20×21 | interiores v2 |
| 15 | `cofre` | Cofre | 20×22 | interiores v2 |
| 16 | `cofre_tesoro` | Cofre | 29×26 | editor_object_icons |
| 17 | `antorcha_pared` | Antorcha / farol | 14×25 | interiores v2 |
| 18 | `brasero` | Antorcha / farol | 22×29 | interiores v2 |
| 19 | `farol` | Antorcha / farol | 22×34 | interiores v1 |
| 20 | `puerta_taberna` | Puerta y salida | 49×50 | editor_object_icons |
| 21 | `portal_runico` | Puerta y salida | 49×54 | editor_object_icons |
| 22 | `poste_indicador` | Cartel | 33×38 | bosque |
| 23 | `tablon_anuncios` | Cartel | 44×40 | bosque |
| 24 | `hoguera` | Antorcha / farol | 23×20 | bosque |
| 25 | `arbusto` | Vegetacion | 30×24 | bosque |
| 26 | `helecho` | Vegetacion | 25×19 | bosque |
| 27 | `arbusto_seco` | Hongo / raiz corrupta | 27×24 | bosque |
| 28 | `tocon` | Hongo / raiz corrupta | 27×18 | bosque |
| 29 | `arbolito` | Estructuras altas (P1) | 30×46 | bosque |
| 30 | `poste_valla` | Estructuras altas (P1) | 15×24 | bosque |
| 31 | `valla` | Estructuras altas (P1) | 31×24 | bosque |
| 32 | `piedra_runica` | Altar oscuro | 29×32 | bosque |
| 33 | `menhir_runico` | Altar oscuro | 33×40 | bosque |

## Pickups

| Frame | `id` | Familia (backlog) | Tamaño | Fuente |
| ---: | --- | --- | ---: | --- |
| 1 | `pocion` | Pocion | 15×22 | editor_object_icons |
| 2 | `llave` | Llave | 17×20 | editor_object_icons |
| 3 | `espada` | Arma en suelo | 29×26 | editor_biomes |
| 4 | `hacha` | Arma en suelo | 29×26 | editor_biomes |
| 5 | `arco` | Arma en suelo | 26×26 | editor_biomes |
| 6 | `escudo` | Armadura / escudo | 22×22 | editor_biomes |
| 7 | `daga` | Arma en suelo | 20×16 | equipo |
| 8 | `baston` | Arma en suelo | 13×30 | equipo |
| 9 | `grimorio` | Libro / grimorio | 18×18 | equipo |
| 10 | `cota_malla` | Armadura / escudo | 20×20 | equipo |
| 11 | `yelmo` | Armadura / escudo | 16×20 | equipo |
| 12 | `botas` | Armadura / escudo | 16×20 | equipo |
| 13 | `antorcha` | Antorcha / farol | 19×30 | editor_object_icons |
| 14 | `cerveza` | Comida | 14×16 | interiores v2 |
| 15 | `jarra` | Comida | 13×17 | interiores v2 |
| 16 | `pocion_mana` | Pocion | 15×22 | tinte de `pocion` |
| 17 | `pocion_veneno` | Pocion | 15×22 | tinte de `pocion` |
| 18 | `eter` | Pocion | 15×22 | tinte de `pocion` |
| 19 | `pocion_mision` | Pocion | 15×22 | tinte de `pocion` |
| 20 | `llave_hierro` | Llave | 17×20 | tinte de `llave` |

## Pendiente de cablear en el motor

Hoy `StaticEntity` pinta props y pickups contra el **tileset del nivel**
(`def->spriteId`) con un tinte por categoría: por eso en partida un
arbusto o una poción son un rombo de suelo teñido. Esta entrega es arte +
manifiestos (decisión de Oriol, 25/09); al cablearlo, el prop usa
`setCharacterSprite(atlas, 64, 64, characterAnchor())` con el frame del
JSON, igual que los actores.

Correspondencia propuesta con los objetos más colocados en los niveles:

| `objectId` del catálogo | Apariciones en niveles | Sprite |
| --- | ---: | --- |
| `arbusto` | 18 | `props_p0` → `arbusto` |
| `regreso_al_mundo` | 15 | `props_p0` → `portal_runico` |
| `pocion` | 11 | `pickups_p0` → `pocion` |
| `eter` | 6 | `pickups_p0` → `eter` |
| `llave_cueva` | 5 | `pickups_p0` → `llave` |
| `puerta_*`, `salida_*`, `on_puerta_*` | 1 c/u | `props_p0` → `puerta_taberna` |

## Lo que no se ha podido sacar

No hay dibujo fuente en las láminas para **pozo, jaula, alcantarilla,
huesos, saco, pan, carta, monedas ni gemas**: hay que generarlos antes en
el loop de autoría. Los **árboles** grandes (`forest_world_props_v1`) no
caben en 64×64 sin encogerse: son estructuras altas (P1) y piden un atlas
de celdas mayores.
