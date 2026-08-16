# Remapeo del tileset viejo al nuevo — propuesta para revisar

**No se ha aplicado nada.** Este documento es la tabla que pediste revisar
antes de tocar los 81 mapas, y trae un problema que no estaba a la vista
cuando lo decidimos.

---

## El problema: el 25 % de la ciudad dejaría de existir

El tileset viejo (`ciudad_tileset.png`, 36 tiles) no era solo suelo:
**22 de sus 36 tiles bloquean el paso** y representan edificios y muros
— `muralla`, `castillo`, `iglesia`, `tienda`, `posada`, `casa_piedra`,
`seto`…

En los 81 mapas hay **82.366 celdas pintadas**, de las cuales **20.578
(el 25 %) son de esos tiles**. Son los edificios de Boundington.

El tileset nuevo (`terreno_iso.png`, 40 tiles) es **todo suelo**. Solo
tres bloquean: `rio`, `estanque`, `alcantarilla`.

Un remapeo directo convertiría cada edificio de la ciudad en suelo
transitable. Boundington pasaría a ser una explanada por la que se cruza
andando de lado a lado, y la campaña —que depende de puertas, casas y
murallas— dejaría de tener sentido. No daría ningún error: se cargaría
perfectamente y estaría mal.

## Por qué el arte nuevo no lo resuelve solo

`editor_construction_tiles.png` **sí** tiene muros, murallas, vallas,
portillos, setos y un pozo: es justo lo que hace falta. Están en sus
filas 3 y 4.

Pero son **estructuras más altas que la celda** —su dibujo ocupa los 313
píxeles de alto porque se levanta sobre el rombo— y
`IsometricRenderer::renderLayer()` dibuja **cada tile exactamente al
tamaño de celda**:

```cpp
Vector2 tileSize{static_cast<float>(m_map->getTileWidth()),
                 static_cast<float>(m_map->getTileHeight())};
```

Metidas hoy en el atlas, saldrían tumbadas dentro del rombo. Por eso
`gen_tileset_iso.py` las deja fuera y lo dice, en vez de colarlas.

## Las tres salidas

| | Qué implica | Coste |
|---|---|---|
| **A. Enseñar al renderer a dibujar tiles altos** | Anclar el sprite a la base del rombo y dejarlo crecer hacia arriba. Entonces los muros y vallas del atlas de construcción entran como tiles de verdad y los edificios se construyen con ellos | Cambio contenido en `renderLayer` + orden de dibujado (un muro alto tapa lo que hay detrás). Es la solución buena |
| **B. Duplicar tiles como "edificio"** | Añadir al atlas copias de ciertos suelos marcadas con colisión, para que el remapeo conserve el bloqueo | Rápido, pero los edificios se verían como suelo. Tapar el problema |
| **C. Edificios como objetos, no como tiles** | Sacarlos del mapa de tiles y ponerlos en el catálogo de objetos, que ya tiene fichas y colisión | Lo más limpio a largo plazo. Toca los 81 mapas y el generador de ciudad |

Mi recomendación es **A**: el arte ya existe y está pensado para eso, y sin
tiles altos este motor no puede dibujar una ciudad que parezca una ciudad.

---

## La tabla, para cuando se desbloquee

Suelos (se pueden remapear ya, no bloquean nada):

| GID viejo | Era | GID nuevo | Pasa a ser |
|---:|---|---:|---|
| 1 | adoquin | 17 | adoquin_limpio |
| 3 | cesped | 1 | pradera |
| 5 | marmol | 20 | marmol_real |
| 6 | tierra | 3 | sendero_tierra |
| 21 | arena | 9 | arena |
| 22 | escenario | 37 | tarima_madera |
| 23 | umbral | 18 | adoquin_gastado |
| 24 | grava | 34 | tierra_piedras |
| 28 | adoquin_fino | 35 | adoquin_ajedrez |
| 30 | ruina | 29 | ciudad_ruinas |
| 31 | carril | 25 | pavimento_antiguo |
| 32 | barro | 30 | callejon_embarrado |
| 34 | tendedero | 34 | tierra_piedras |
| 35 | puente | 37 | tarima_madera |

Bloqueantes (**dependen de la decisión de arriba**):

| GID viejo | Era | Destino propuesto | Nota |
|---:|---|---|---|
| 2 | muralla | muro de piedra (construcción, fila 3) | necesita tiles altos |
| 4 | agua | 14 rio | ya bloquea en el nuevo |
| 7 | castillo | torreón (construcción, fila 4) | necesita tiles altos |
| 8 | iglesia | 23 mosaico_templo + muro | el suelo existe, el edificio no |
| 9 | universidad | 24 suelo_biblioteca + muro | ídem |
| 10 | biblioteca | 24 suelo_biblioteca + muro | ídem |
| 11 | opera | 19 plaza + muro | ídem |
| 12 | coliseo | 26 arenisca_desertica + muro | ídem |
| 13 | militar | 32 patio_fortificado + muro | ídem |
| 14 | tienda | 21 suelo_vivienda + muro | ídem |
| 15 | ropa | 21 suelo_vivienda + muro | ídem |
| 16 | joyeria | 21 suelo_vivienda + muro | ídem |
| 17 | banco | 25 pavimento_antiguo + muro | ídem |
| 18 | posada | 22 suelo_taberna + muro | ídem |
| 19 | banos | 38 losa_clara + muro | ídem |
| 20 | seto | seto con bayas (construcción, fila 4) | necesita tiles altos |
| 25 | casa_piedra | 38 losa_clara + muro | ídem |
| 26 | casa_madera | 37 tarima_madera + muro | ídem |
| 27 | casa_noble | 20 marmol_real + muro | ídem |
| 29 | piedra_vieja | 16 piedra_antigua + muro | ídem |
| 33 | chabola | 39 losa_oscura + muro | ídem |
| 36 | rio | 14 rio | ya bloquea en el nuevo |

> «+ muro» significa que el edificio deja de ser **un** tile y pasa a ser
> suelo con un contorno de muro alrededor. Es cómo se dibuja un edificio
> en un motor isométrico de tiles, y es exactamente lo que hoy no se puede
> hacer.

---

## Lo que sí se hizo ya, y no dependía de esto

- **`terreno_iso.png`**: 40 suelos a 128×64, aplastados a 2:1 desde los
  atlas editoriales, con su índice en `terreno_iso.json`.
  (`tools/gen_tileset_iso.py`, regenerable.)
- **Arreglado el destructor de mapas**: el editor guardaba siempre el
  tileset por defecto, así que abrir un mapa de ciudad y pulsar guardar
  dejaba el TMX declarando un atlas de 2 casillas para un mapa que usa 34.
  Ya le había pasado a `ciudad_centro.tmx`.
- **Reparados** `ciudad_centro.tmx` y `ciudad_en_aldea_m.tmx`, que estaban
  así desde antes.
