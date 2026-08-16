# Medidas de sprites y contrato visual del motor

## Regla que no se negocia

El mundo se proyecta en isométrica **2:1**. Una celda de mapa mide **64×32
unidades dibujadas**. Por tanto, todo suelo debe terminar siendo un rombo
con esta relación:

```text
ancho = 2 × alto
suelo runtime: 128×64 px fuente  ->  64×32 unidades en pantalla
```

“Partir el sprite de suelo por la mitad” significa reducir su **alto** a la
mitad de su ancho al convertirlo a runtime. No significa cortar el rombo
en dos piezas ni reducir también el ancho. Un suelo 128×128 cuadrado se
vería aplastado o saldría de su celda; el destino correcto es 128×64.

## Tabla de medidas vigentes

| Recurso | Medida de archivo | Celda fuente | Tamaño en motor | Uso y estado |
| --- | ---: | ---: | ---: | --- |
| Celda del mapa | — | — | **64×32** | Unidad espacial, cámara, clic, colisión y culling. No cambiar por mapa. |
| `terreno_iso.png` | 1024×320 | **128×64**, 8×5, 40 tiles | 64×32 | Suelo runtime nuevo. Es la conversión correcta de las bibliotecas editoriales. |
| `ciudad_tileset.png` | 96×96 | 16×16, 6×6, 36 tiles | 64×32 | Tileset runtime heredado de las ciudades. Sus GIDs siguen vivos en 81 mapas. |
| `editor_terrain_nature.png` | 1252×1252 | aprox. 313×313, 4×4 | No directo | Fuente editorial cuadrada: recortar una celda y convertir a 128×64. |
| `editor_terrain_civilizations.png` | 1252×1252 | aprox. 313×313, 4×4 | No directo | Igual: adoquines, interiores y ruinas pasan por conversión 2:1. |
| `editor_construction_tiles.png`, filas 1–2 | 1252×1252 | aprox. 313×313 | No directo | Suelo transitable: convertir a 128×64 para `terreno_iso`. |
| `editor_construction_tiles.png`, filas 3–4 | 1252×1252 | aprox. 313×313, alto | Aún no | Muros, vallas, torreones y setos: son **tiles altos**, no suelo. |
| `personaje.png` | 768×64 | **64×64**, 12 frames | 64×64, ancla Y −32 | Actor temporal: jugador, PNJ y enemigos. Sus pies se apoyan en la base de una celda 64×32. |
| `editor_enemy_library_v1.png` | 1448×1086 | **362×362**, 4×3 | Solo editor | Preview de enemigos. No asignar como `spriteId` hasta crear hoja animada. |
| `editor_object_icons.png` | 1252×1252 | aprox. 313×313, 4×4 | Solo editor | Pictogramas de autoría, no sprites finales. |

## Conversión de suelo: de editor a juego

El proceso oficial ya está automatizado en
`tools/gen_tileset_iso.py`:

```text
celda editorial cuadrada (≈313×313)
        ↓ recortar una sola celda
        ↓ limpiar restos de estructuras altas
        ↓ redimensionar conservando el rombo a 128×64
terreno_iso.png, 8 columnas × 5 filas
        ↓ el renderer lo dibuja en 64×32
celda isométrica del mapa
```

La elección de 128×64 como fuente conserva detalle al hacer zoom; 64×32
también es válido si se busca menos memoria. Ambos cumplen 2:1. No crear
un suelo de 128×128 ni usar un sprite de personaje como tile.

## Suelo, estructura y actor no comparten molde

| Tipo | Proporción de imagen | Anclaje | Ejemplos | Norma |
| --- | --- | --- | --- | --- |
| Suelo | 2:1 | Ocupa el rombo de la celda | adoquín, césped, agua, tarima | 128×64 fuente; se puede pisar o bloquear por GID. |
| Estructura alta | ≥1:1, crece hacia arriba | Base en el borde inferior del rombo | muro, valla, torreón, árbol, pozo | No aplastar a 2:1. Requiere soporte de tile alto o `prop`. |
| Actor | normalmente 1:1 | Pies en la base del rombo | jugador, PNJ, enemigo | 64×64 actual; ancho igual a tile, alto doble del suelo. |
| Icono editorial | libre | Centrado en preview | catálogo, selector, inspector | Nunca se usa para el render de partida. |

## Cómo elegir la medida al crear algo nuevo

1. ¿El jugador camina encima? Es **suelo**: preparar el arte a 2:1 y
   añadirlo a `terreno_iso.png` mediante el generador.
2. ¿Impide pasar y sobresale por encima del suelo? Es **estructura alta**:
   hoy usar `prop` si necesita verse grande; cuando el renderer soporte
   tiles altos, se convertirá en tile anclado a la base.
3. ¿Se mueve, habla o combate? Es un **actor**: hoja de frames cuadrados,
   64×64 como mínimo, con sus animaciones de caminar.
4. ¿Solo ayuda a elegir contenido? Es un **icono editorial** y puede ser
   grande/cuadrado sin entrar en el juego.

## Límites actuales antes de remapear las ciudades

Los 81 mapas urbanos contienen edificios como tiles sólidos. El nuevo
`terreno_iso.png` reúne 40 suelos y solo bloquea río, alcantarilla y
estanque. Sustituir los GIDs de ciudad directamente convertiría muros,
casas y templos en suelo transitable.

Por ello, la migración correcta es en dos fases:

1. Mantener `ciudad_tileset.png` para edificios y colisiones actuales.
2. Añadir render de **tiles altos anclados a la base** o convertir los
   edificios en `prop`; entonces remapear cada suelo siguiendo
   `REMAPEO_TILES.md`.

Esta regla protege el aspecto y la jugabilidad: no se sacrifican las
ciudades por aplicar un atlas de suelo más bonito.
