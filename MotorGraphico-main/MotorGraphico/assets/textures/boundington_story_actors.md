# Actores de historia de Boundington

`boundington_story_actors_source_v1.png` es la fuente editorial 4×3.
`boundington_story_actors_idle.png` es el atlas runtime derivado: 12 frames
de 64×64, con alpha y los pies anclados abajo.

| Frame | Actor | `objectId` que lo usa |
| ---: | --- | --- |
| 1 | Luisarda | `luisarda` |
| 2 | Ben Kafka | `ben_kafka` |
| 3 | Griffin | `griffin` |
| 4 | Duende de máscara de porcelana | `duende_porcelana` (cuando se añada) |
| 5 | Perdido encapuchado | `perdido_saqueador`, `perdido_fanatico`, `sectario_perdido` |
| 6 | Carcelero del culto | `cultista_carcelero` |
| 7 | Saga musgosa | `saga_bosque` (cuando se añada) |
| 8 | Niña rescatada | `nina_del_gato` |
| 9 | Guardia | `guardia` |
| 10 | Mercader | `parroquiano_humilde`, `tendero_mercado` |
| 11 | Naga de desagüe | `naga_desague` (cuando se añada) |
| 12 | Rata | `rata_alcantarilla` (cuando se añada) |

El runtime selecciona estos frames por `objectId` antes que el fallback
racial o el sprite temporal. Los aliases son intencionados: por ahora varias
fichas de Perdidos comparten silueta. No inventar un PNG por enemigo hasta
que el encuentro aporte una lectura distinta.

Para regenerar el atlas tras cambiar la fuente:

```sh
python3 tools/gen_boundington_story_sprites.py
```

Estado: idle runtime. Aún faltan `walk_down`, `walk_up`, `walk_left` y
`walk_right` para los actores que se muevan.
