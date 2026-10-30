# Asset production loop

Estas hojas son referencias de autoría generadas para decidir familias y
variantes. No se asignan todavía como `spriteId`: tienen fondo editorial y la
composición no garantiza celdas runtime de 64×64 o tiles 128×64.

## Regla obligatoria para suelos

Todo suelo debe ser un tile repetible en la cuadrícula isométrica 2:1. Un
dibujo bonito que solo funciona como ilustración no entra en la paleta de
terreno. Cada material debe producir, como mínimo, una pieza de centro,
bordes N/E/S/O, cuatro esquinas y una transición hacia otro material. Los
bordes deben casar al repetir una matriz 2×2, sin costuras visibles ni cambios
de escala.

La decoración se separa del suelo: charcos, grietas, hierba, cultivos, nieve,
musgo, minerales y cactus son overlays o props con transparencia. La colisión,
la humedad, el daño elemental y la navegación se registran aparte, para poder
reutilizar el mismo suelo con distintos adornos y reglas de juego.

## Lote 01 generado

| Hoja | Contenido | Uso inmediato | Próximo paso |
|---|---|---|---|
| `actors/authoring_draconid_equipment_v1.png` | cuerpos base masculino/femenino, dracónidos, bardo, guerrero, mago, explorador y piezas de equipo | decidir siluetas y modularidad | separar cuerpo/ropa/arma en capas y crear hojas runtime |
| `creatures/forest_creatures_elemental_v1.png` | lobos, jabalíes, ciervos, oso, cuervo, conejo, serpiente, lince, polilla y salamandra elementales | catálogo de monstruos demacrados con aura | crear sprites base + aura separada y variantes tintables |
| `tiles/forest_walls_bridges_portals_v1.png` | suelos de bosque, muros, esquinas, puentes, portal rúnico, puertas y rejas | definir kit de construcción tileable | recortar piezas, comprobar bordes y producir atlas TMX |
| `ui/rpg_ui_icons_v1.png` | ataque, magia, defensa, inventario, equipo, quest, diálogo, mapa, guardado, portal y eventos | base para menús e inspector | normalizar a iconos 24/32/48 px y registrar semántica |
| `actors/races_foundation_v1.png` | drows, elfos, elementales, enanos, hombres gato, hombres pájaro, orcos, tieflings, humanos negros, hombres lagarto y fantasmas | ampliar el catálogo de cuerpos base | vincular `raceId` y crear variantes masculina/femenina donde proceda |
| `actors/equipment_layers_v1.png` | sombreros, capuchas, torsos, pantalones, faldas, calcetines, guantes, botas y armas | biblioteca de capas reutilizables | separar cada slot y mantener el mismo anclaje de pies |
| `actors/races_extension_gnomes_goblins_minotaurs_v1.png` | gnomos, goblins, minotauros y extensiones de fantasmas, lagartos, gatos, pájaros y drows | completar las razas que faltaban | convertir cada cuerpo a la misma escala y baseline |
| `tiles/bridge_directional_construction_v1.png` | rectas, diagonales, giros, cruces, finales, barandillas y pilares | resolver la construcción libre de puentes | convertir cada pieza en tile con conectores explícitos |
| `world/interiors_furniture_windows_v1.png` | mesas, sillas, bancos, camas, estanterías, forja, cofres, lámparas y ventanas | kit de interiores modular | crear variantes de esquina/lateral y footprints de colisión |
| `world/interiors_materials_furniture_v2.png` | muebles, taburetes, jarras, cerámica, madera, piedra, ladrillo, hierro y ventanas | biblioteca de materiales y decoración | convertir material + prop en variantes de catálogo |
| `tiles/terrain_agriculture_desert_mines_v1.png` | adoquines, baldosas, charcos, desierto, cactus, cultivos, agua, lava, minas y minerales | kit de suelo tileable | separar suelo base, borde, overlay y colisión |
| `creatures/fauna_domestic_fairies_v1.png` | pájaros, serpientes, murciélagos, animales domésticos y hadas | fauna de ambiente y NPC menores | registrar como `enemy`, `npc` o `prop` según comportamiento |
| `world/sky_vfx_mining_lights_v1.png` | sol, luna, estrellas, viento, lluvia, fuego, hielo, agua, lava, humo y gemas | capas de atmósfera y efectos | separar sprite estático, animación y aura tintable |
| `tiles/tileable_elemental_floors_props_v1.png` | suelos de hielo, agua, lava, mina, roca, minerales preciosos y grava; sillas, taburetes y jarras | kit elemental de suelo y props superpuestos | recortar variantes tileables y exportar props con footprint independiente |

## Familias pendientes del loop

### Actores y equipo

- 43 razas/linajes existentes: cuerpo base neutro, variante masculina y
  femenina cuando el diseño lo requiera.
- Ampliación cubierta en el lote actual: drow, elfos, elementales, enanos,
  hombres gato, hombres pájaro, orcos, tieflings, humanos negros, hombres
  lagarto, fantasmas, gnomos, goblins y minotauros. Las proporciones pequeñas
  de gnomos/goblins y las siluetas grandes de minotauros se mantienen en una
  hoja separada para no deformar la escala del resto.
- Capas de equipo: `head`, `chest`, `legs`, `socks`, `gloves`, `boots`,
  `weapon_main`, `weapon_off`, `back`, `aura`.
- Roles mínimos: bardo, guerrero, mago, explorador, clérigo, mercader y
  guardia.
- Estados visuales: idle, caminar abajo/arriba/lateral, daño y caída.

### Monstruos y fauna

- Bosque: lobo, jabalí, ciervo, oso, cuervo, conejo, serpiente, polilla,
  salamandra y murciélago.
- Variantes económicas por tintado/aura: normal, musgo, fuego, electricidad,
  hielo, veneno y corrupción.
- Un solo cuerpo base por familia siempre que la silueta aguante la variante.

### Construcción y terreno

- Suelo tileable: hierba, barro, roca, nieve, arena, adoquín, baldosa, madera,
  hielo, agua, lava, mina, roca, minerales preciosos, grava y suelo ritual; todos con centro, bordes, esquinas y
  transiciones documentados.
- Props de interior reutilizables: silla, taburete y jarra de cerveza; nunca se
  hornean dentro del tile de suelo.
- Pared: recta, esquina interior, esquina exterior, final, puerta y hueco.
- Puente: recto horizontal, recto vertical, giro, cruce, final y barandilla.
- Puente isométrico: diagonales, giros de 90°, T, cruce, remate, pilar y
  barandilla independiente; cada pieza debe declarar conectores N/E/S/O.
- Portales: marco, runa central, estado apagado y estado activo.
- Regla: si no tiene sprite o tile definido, no aparece en la paleta de
  producción; las entradas conceptuales viven en documentación, no en la
  lista visual.

### Mundo e interfaz

- Plantas: hierba alta, flores, helechos, arbustos, árboles jóvenes, árboles
  antiguos y árboles muertos.
- Interacción: puertas, cofres, carteles, fogatas, bancos, mesas, talleres y
  puntos de guardado.
- Interiores: muebles básicos, ventanas frontales y laterales, puertas
  izquierdas/derechas, esquinas interiores y objetos con footprint de
  colisión separado del dibujo.
- Señalética: misión disponible, misión activa, objetivo, evento, peligro,
  tienda, portal y transición.
- Menús: pausa, inventario, equipo, habilidades, diario, mapa, batalla,
  tienda, diálogo y confirmación.

## Criterio de conversión a runtime

Una hoja pasa de `authoring` a `runtime` solo cuando cumple todo esto:

1. Fondo transparente o máscara validada.
2. Celdas y anclajes documentados.
3. Ningún elemento toca la celda vecina.
4. Si es suelo, sus cuatro bordes repiten correctamente y supera una prueba
   visual 2×2; además declara centro, bordes, esquinas y transiciones.
5. El sprite puede registrarse con un `spriteId` estable.
6. Existe una entrada JSON que describe la familia y sus variantes.
7. El editor lo muestra en la categoría correcta: tile, prop, NPC, enemy,
   pickup o UI.
8. El playtest lo carga sin warning de atlas ni de catálogo.

## Economía de assets

La prioridad es maximizar variantes por imagen base:

```text
cuerpo base + equipo por capas + tintado + aura + datos de catálogo
                 ↓
        muchas identidades jugables
```

No se generará un PNG completo para cada combinación de raza y profesión. Se
generan piezas modulares y solo se hornean sprites completos cuando una
animación o silueta especial lo justifica.

## Integración prevista

El proyecto ya dispone de un atlas runtime de 43 referencias de raza en
`assets/textures/race_npc_idle.png` y `race_npc_idle.json`. El archivo
`race_layer_manifest.json` añade la convención de capas sin reemplazar ese
atlas: primero se reutiliza la raza existente y después se superponen ropa,
armas, tintes y aura. Para construcción, `construction_connectors.json`
define los conectores N/E/S/O que necesita el editor para validar puentes y
paredes antes de exportar un TMX.
