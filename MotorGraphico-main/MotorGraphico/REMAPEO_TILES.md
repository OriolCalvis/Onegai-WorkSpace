# Remapeo del tileset al arte nuevo

> **APLICADO.** Los suelos ya están en los 91 mapas del mundo. Lo que
> sigue pendiente son los **edificios**: ver «El problema» más abajo.
>
> Se hizo con `tools/gen_terreno_mundo.py` + `tools/remapear_suelos.py`,
> ambos idempotentes. La comprobación clave —que ninguna celda cambiara de
> transitable a bloqueada— pasó: **108.991 celdas comparadas, 0 cambios**.

## Cómo se aplicó sin romper la ciudad

No se puede remapear «solo la mitad» dejando la otra en el atlas viejo,
porque `TileMap` admite **un solo `<tileset>` por mapa**. Así que el atlas
de runtime lleva las dos cosas:

    GID  1..40   suelos nuevos (arte nuevo)
    GID 41..76   los 36 tiles viejos, conservados con su colisión

Los suelos pasaron al arte nuevo; los edificios siguen exactamente donde
estaban, con su bloqueo, hasta que existan tiles de muro de verdad.

---

## El problema que queda: los edificios

El tileset viejo (`ciudad_tileset.png`, 36 tiles) no era solo suelo:
**22 de sus 36 tiles bloquean el paso** y representan edificios y muros
— `muralla`, `castillo`, `iglesia`, `tienda`, `posada`, `casa_piedra`,
`seto`…

En los 81 mapas hay **82.366 celdas pintadas**, de las cuales **20.578
(el 25 %) son de esos tiles**. Son los edificios de Boundington.

El tileset nuevo (`terreno_iso.png`, 40 tiles) es **todo suelo**. Solo
tres bloquean: `rio`, `estanque`, `alcantarilla`.

Un remapeo directo habría convertido cada edificio de la ciudad en suelo
transitable. Boundington sería una explanada por la que se cruza andando
de lado a lado, y la campaña —que depende de puertas, casas y murallas—
dejaría de tener sentido. No daría ningún error: cargaría perfectamente y
estaría mal.

Por eso se conservaron en la zona 41..76 en vez de traducirlos. **Hoy
siguen viéndose como los cuadrados de color de antes**, ahora dibujados
como rombo para que encajen con los suelos nuevos.

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

## La tabla

Suelos — **ya aplicados** en los 91 mapas:

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

Bloqueantes — **conservados en 41..76**, pendientes de la decisión de arriba:

| GID viejo | Era | Destino propuesto | Nota |
|---:|---|---|---|
| 2 | muralla | muro de piedra (construcción, fila 3) | necesita tiles altos |
| 4 | agua | 14 rio | **aplicado**: bloquea igual |
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
| 36 | rio | 14 rio | **aplicado**: bloquea igual |

> «+ muro» significa que el edificio deja de ser **un** tile y pasa a ser
> suelo con un contorno de muro alrededor. Es cómo se dibuja un edificio
> en un motor isométrico de tiles, y es exactamente lo que hoy no se puede
> hacer.

---

## Un validador que adivinaba

`validar_enlaces.py` llevaba `COLISION={2,4,7..20}` escrito a mano: los
GIDs del tileset de entonces. Al cambiar el arte del mundo esos números
pasaron a ser suelos nuevos, y el validador denunció **19 puertas
perfectamente buenas**. Ahora lee la colisión del propio TMX, como el
motor. `gen_carteles.py` tenía el mismo set y también se arregló;
`conectividad.py` ya se había curado de esto mismo hace tiempo.

Un validador que hay que actualizar a mano cuando cambia el arte no
valida: adivina.

## Lo demás que se hizo

- **`terreno_iso.png`**: 40 suelos a 128×64, aplastados a 2:1 desde los
  atlas editoriales, con su índice en `terreno_iso.json`.
  (`tools/gen_tileset_iso.py`, regenerable.)
- **Arreglado el destructor de mapas**: el editor guardaba siempre el
  tileset por defecto, así que abrir un mapa de ciudad y pulsar guardar
  dejaba el TMX declarando un atlas de 2 casillas para un mapa que usa 34.
  Ya le había pasado a `ciudad_centro.tmx`.
- **Reparados** `ciudad_centro.tmx` y `ciudad_en_aldea_m.tmx`, que estaban
  así desde antes.
