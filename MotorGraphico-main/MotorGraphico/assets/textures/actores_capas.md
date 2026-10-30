# Actores andando por capas

`actores_capas.png` es un atlas **runtime** de 12 columnas × 9 filas de
64×64. Cada fila es un actor completo que camina en cuatro direcciones.
Lo genera `tools/gen_personajes.py` montando por capas el cuerpo base
`personaje.png` (ver [personajes.md](personajes.md)); su contrato está en
`actores_capas.json`, y cada actor guarda en `composicion` las piezas con
las que se montó. **No se edita a mano**: se cambia la composición y se
regenera.

```sh
python3 tools/gen_personajes.py               # este atlas, razas, historia y capas
python3 tools/gen_personajes.py --vista       # + láminas en out/revision_personajes/
python3 tools/validar_sprites.py              # comprueba atlas contra manifiesto
```

## Reparto de frames de cada fila

Es el reparto **real** de `personaje.png`, que el atlas conserva:

| Frames de la fila | Dirección | Quieto |
| ---: | --- | ---: |
| 1-3 | arriba (de espaldas) | 2 |
| 4-6 | derecha | 5 |
| 7-9 | abajo (de frente) | 8 |
| 10-12 | izquierda | 11 |

| Animación | Frames de la fila |
| --- | --- |
| `idle` | 8 |
| `idle_up` / `idle_right` / `idle_left` | 2 / 5 / 11 |
| `walk_down` | 7, 8, 9, 8 |
| `walk_up` | 1, 2, 3, 2 |
| `walk_right` | 4, 5, 6, 5 |
| `walk_left` | 10, 11, 12, 11 |

Frame global = (fila − 1) × 12 + frame de la fila. El manifiesto trae ya
calculadas las animaciones de cada actor.

## Filas

| Fila | `id` | Primer frame | Uso |
| ---: | --- | ---: | --- |
| 1 | `aventurero` | 1 | Jugador: sustituto de `personaje.png` como protagonista. |
| 2 | `aventurero_azul` | 13 | Tinte: compañero o PNJ viajero. |
| 3 | `aventurero_rojo` | 25 | Tinte: compañero o PNJ viajero. |
| 4 | `aventurero_pardo` | 37 | Tinte: compañero o PNJ viajero. |
| 5 | `guardia` | 49 | Guardia de Boundington que patrulla. |
| 6 | `aldeano` | 61 | Parroquiano, tendero o vecino que pasea. |
| 7 | `aldeana` | 73 | Parroquiana, tabernera o vecina que pasea. |
| 8 | `cultista` | 85 | Perdido encapuchado que se mueve. |
| 9 | `cultista_carcelero` | 97 | Carcelero del culto. |

Estas filas sirven a los PNJ y enemigos genéricos que **se mueven**, y a
los que hoy caen en el «bebé» de `personaje.png` por no tener hoja propia.
Los personajes de historia andando (Luisarda, Ben, Griffin y compañía)
están en `personajes_historia.png` y las 43 razas vestidas en
`razas_andando.png`, con el mismo formato.

## Cómo se hace

Por capas, como todos los personajes: el mapa de partes del cuerpo base,
las prendas que siguen la silueta y las plantillas de pelo y tocados están
explicados en [personajes.md](personajes.md).

### Añadir un actor

1. Una entrada en `ACTORES` de `tools/actores_personajes.py`: raza, pelo,
   ojos y piezas de equipo (`python3 tools/probador_personaje.py --listar`).
2. Probarla: `python3 tools/probador_personaje.py --raza ... --equipo ...`.
3. Regenerar con `tools/gen_personajes.py` y validar.

## Pendiente de cablear en el motor

Por decisión de Oriol (25/09) esta entrega es **arte + manifiestos**: el
motor todavía no carga este atlas. Al cablearlo, no en `Application.cpp`
(está por encima del techo de 500 líneas), sino en una clase que lea los
manifiestos:

```cpp
// TextureManager invierte los PNG al cargarlos: en un atlas de varias
// filas la fila se invierte, igual que en race_npc_idle.
for (int f = 1; f <= columns * rows; ++f)
    atlas->defineRegion(f, (f - 1) % columns, rows - 1 - (f - 1) / columns);
// Animaciones: las de "animaciones" de cada actor en actores_capas.json.
```

### El fallo que destapó en `personaje.png` (arreglado)

`Application::init` registraba `walk_down {1,2,3,4}`, `walk_up {5..8}`,
`walk_right` y `walk_left {9..12}` e `idle {1}`. La hoja real es arriba,
derecha, abajo, izquierda con tres frames cada una: **el jugador se quedaba
de espaldas** cuando estaba quieto y «caminaba hacia abajo» enseñando la
espalda. Desde APARIENCIA-01 los ciclos los reparte
`HojaPersonaje::animar()` con el reparto de `personaje.json`, empezando cada
ciclo en el frame quieto ({8, 7, 8, 9} hacia abajo), que es el que el
jugador enseña al pararse.

## Escala

El cuerpo base mide 40 px de alto (pies en la fila 60). Los retratos del
atlas de historia miden unos 58 px. Esa diferencia ya existía entre el
jugador y los PNJ de Boundington; unificarla (escala visual por objeto o
un cuerpo base más alto) es una decisión de producto pendiente.
