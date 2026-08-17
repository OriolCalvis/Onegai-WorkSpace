# Taxonomía de paletas del editor

La paleta no debe organizarse solo por el tipo técnico del dato. `Prop`,
`Enemy` y `Pickup` son categorías de runtime; para crear mapas se necesitan
afinidades visuales y jugables que puedan solaparse.

## Tiles

Cada tile se lee desde `assets/textures/terreno_iso.json` y aparece con su
nombre propio, afinidad, efecto previsto y requisito de habitabilidad.

| Afinidad | Ejemplos | Efecto previsto | Contramedida |
|---|---|---|---|
| Bosque / pradera | pradera, hierba, hojarasca | natural | ninguna |
| Árido / desierto | arena, arenisca, cañón | sed + calor | agua, ropa ligera o bufo de calor |
| Frío / nieve | nieve, hielo, escarcha | frío | ropa térmica o bufo de frío |
| Agua / costa | río, agua somera, estanque | humedad | botas impermeables o equipo acuático |
| Pantano / humedal | barro, turba, alcantarilla | lento + humedad | botas de pantano o movilidad |
| Volcánico / fuego | ceniza, lava, volcánico | calor + daño | resistencia al fuego |
| Roca / mina | roca, piedra, adoquín | terreno duro | botas resistentes opcionales |
| Interior / urbano | vivienda, taberna, mármol, plaza | natural | ninguna |

La primera implementación ya muestra estas propiedades en la paleta. El
siguiente paso será conectar `effect` y `requirement` al runtime de navegación,
daño ambiental, sed, frío y buffs; la etiqueta no debe confundirse con una
regla de juego activa todavía.

## Objetos

Los grupos se solapan deliberadamente. Un PNJ puede aparecer en `PNJ · ELFOS /
DROWS` y en el catálogo general; un cofre puede aparecer como interior y como
evento; un cactus puede pertenecer a decoración exterior y a desierto.

- PNJ: humanos, elfos/drows, enanos/gnomos, orcos/goblins, lagartos/nagas y
  otras razas.
- Decoración: interior, bosque, montaña/mina, selva/pantano,
  desierto/volcán y otros.
- Monstruos: animales, bípedos, no muertos, elementales, dragones,
  sagrados/legendarios y otros.
- Recogibles: consumibles y equipo.
- Eventos: puertas, portales, runas, cofres, guardados y transiciones.

Los nombres visibles proceden de `ObjectDefinition.name`; `id`, `raceId` y
los tags semánticos solo sirven para agrupar y resolver el comportamiento.
