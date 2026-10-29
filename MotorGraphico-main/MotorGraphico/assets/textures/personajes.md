# Personajes por capas

Todos los personajes que caminan salen del mismo cuerpo base
(`personaje.png`, 40 px de alto) y se montan **por capas**: cuerpo de raza,
ojos, pelo, barba y equipo, cada una en su propia hoja transparente de 20
frames (12 de andar y 8 de reposo). Así el jugador puede cambiar de aspecto según lo que lleva puesto,
las 43 razas del catálogo caminan en el mismo estilo que los actores, y los
personajes de historia son una combinación de piezas en vez de un retrato
pintado.

Lo genera `tools/gen_personajes.py`; el contrato está en
`personajes.json`. **No se edita a mano**: se cambia el dato y se regenera.
Estado: **arte + manifiestos + probador + motor**. Desde APARIENCIA-01
(29/09/2026) el motor monta las capas él mismo (`Game::MontajePersonajes`,
byte a byte igual que el probador: lo comprueba `test_personajes_capas` con
los 62 horneados) y el jugador lleva su propia hoja: la que elige en el paso
«Apariencia» de Nueva partida, con el equipo puesto encima. Los tres atlas
horneados siguen ahí para los PNJ.

```sh
python3 tools/gen_personajes.py            # atlas de capas, horneados y manifiestos
python3 tools/gen_personajes.py --vista    # + láminas en out/revision_personajes/
python3 tools/probador_personaje.py --listar
python3 tools/probador_personaje.py --personaje luisarda --gif
python3 tools/validar_sprites.py && python3 tools/test_validar_sprites.py
```

## Qué hay

| Fichero | Contenido |
| --- | --- |
| `actores_capas.png` | 9 actores genéricos (aventurero jugador y tres tintes, guardia, aldeano, aldeana, cultista, carcelero). Mismo atlas que en ARTE-01, ahora montado por capas. |
| `razas_andando.png` | Las 43 razas del catálogo vestidas como en `race_npc_idle.png`, andando en 4 direcciones. Una fila por raza, en el orden de `races.json`. |
| `personajes_historia.png` | 10 personajes de Boundington: Luisarda, Ben Kafka, Griffin, el duende de porcelana, la saga, el verdugo, la niña del gato, el caballero, el mercader y la naga del río. |
| `personajes/razas/*.png` | Las capas de cada raza (una fila por raza): `cuerpo`, `espalda`, `cola`, `alas`, `morro` y `rasgos`. |
| `personajes/cuerpos/*.png` | Cuerpos lisos en 9 pieles y ojos en 10 colores, por talla (para humanos de otra piel o talla). |
| `personajes/pelo/pelo_<talla>.png` | 55 peinados: 7 estilos × 7 colores, más corto y largo en plata-lila, verde musgo y azul hielo. |
| `personajes/barba/barba_<talla>.png` | Barba larga y corta en 7 colores. |
| `personajes/equipo/<ranura>_<talla>.png` | 529 piezas (INSTRUMENTOS-01, con los instrumentos y los objetos de rol): 105 de cabeza, 140 de torso, 35 de piernas, 28 de pies, 12 capas, 168 armas y 41 de mano izquierda. |
| `personajes/equipo_catalogo.json` | La pieza visible de cada objeto de `assets/catalogs/equipment.json` (795 de 850): su forma, su tinte y de dónde sale su estética (ESTETICA-01). |
| `personajes.json` | El contrato: orden de dibujo, tallas, filas de cada atlas y los personajes montados. |

Los atlas horneados tienen el mismo formato que `actores_capas.json`: una
fila por personaje con sus animaciones ya calculadas, y cada uno guarda en
`composicion` las piezas con las que se montó.

## Cómo se monta un personaje

Es lo que hace `tools/probador_personaje.py` y lo que hace el motor en
`src/Game/Personajes/MontajePersonajes.cpp` (la mezcla es la aritmética
entera de `Image.alpha_composite` de Pillow, para que salga igual). Cada atlas de capas tiene 20 columnas de 64×64 y una fila por pieza;
la fila tiene el mismo reparto que `personaje.png` (arriba 1-3, derecha
4-6, abajo 7-9, izquierda 10-12) y, detrás, el reposo (`reposo` en
`personajes.json`: 13-14 arriba, 15-16 derecha, 17-18 abajo, 19-20 izquierda;
ver «El reposo»). Es transparente fuera de la pieza. Las hojas horneadas
(actores, razas, historia, PNJ) siguen con los 12 de andar.

1. **Raza.** De `razas.lista` salen su fila, su `talla`, su `tallaRopa` y
   las `capas` que tiene; cada capa se lee de `razas.atlas[capa]`.
2. **Otra piel u otra talla** (solo razas sin rasgos, como los humanos): el
   cuerpo sale de `cuerpos.atlas[talla]` en la fila de la piel y, si los ojos
   no son los del cuerpo base (azules), se añade la fila de `ojos`.
3. **Barba y pelo**, de `barba.atlas[talla]` y `pelo.atlas[talla]`. El pelo
   no se pone si la raza lo tiene en `sin` o si la pieza de cabeza está en
   `equipo.cabeza.ocultaPelo` (yelmos cerrados).
4. **Equipo.** Cada pieza sale de `equipo[ranura].atlas[talla]`; torso,
   piernas, pies y capa usan `tallaRopa` (`medio_sin_piernas` para nagas y
   espectros). Las ranuras que la raza tiene en `sin` se ignoran.
5. **Apilar.** En cada frame, la dirección del frame decide el orden: se
   dibujan las capas de menor a mayor `orden[ranura][dirección]` (a igualdad,
   en el orden de arriba), todas en el mismo sitio.

```cpp
// TextureManager invierte los PNG al cargarlos: la región del frame f de
// un atlas de varias filas es
atlas->defineRegion(f, (f - 1) % columns, rows - 1 - (f - 1) / columns);
// y la fila n de una capa ocupa los frames (n - 1) * 12 + 1 .. n * 12.
```

### Orden de dibujo

| Ranura | abajo | arriba | derecha | izquierda | Por qué |
| --- | ---: | ---: | ---: | ---: | --- |
| `capa` | 0 | 55 | 0 | 0 | Detrás del cuerpo; sobre la espalda cuando se aleja. |
| `espalda` | 4 | 46 | 4 | 4 | Capucha de cobra de las nagas. |
| `cola` | 5 | 45 | 5 | 5 | |
| `alas` | 6 | 56 | 6 | 6 | Encima de la capa de espaldas. |
| `arma` | 80 | 8 | 80 | 80 | De espaldas, detrás del cuerpo; de perfil, en la mano de delante. |
| `mano_izq` | 80 | 8 | 8 | 8 | Escudo, libro, farol: de perfil va en la mano de atrás. |
| `cuerpo` / `ojos` | 10 / 11 | 10 / 11 | 10 / 11 | 10 / 11 | |
| `piernas` / `pies` / `torso` | 20 / 30 / 40 | igual | igual | igual | El faldón tapa la cintura del pantalón; la bota, su bajo. |
| `morro` / `barba` | 45 / 50 | igual | igual | igual | Morros y barbas caen sobre la ropa del pecho. |
| `pelo` / `rasgos` / `cabeza` | 60 / 65 / 70 | igual | igual | igual | Orejas y cuernos sobre el pelo; el casco encima de todo. |

Una fila puede traer **su propio orden** (SP-02), que manda sobre el de su ranura en los rumbos
que da: `{"id": "f_guanteletes", "nombre": "Guanteletes", "fila": 116, "orden": {"abajo": 80,
"arriba": 80, "derecha": 80, "izquierda": 80}}`. Lo traen los guanteletes del Artista Marcial
(uno en cada mano): de espaldas un arma va detrás del cuerpo, pero un guantelete tapa la mano.
El motor lo lee en `FilaDeCapa::orden` y `MontajePersonajes` lo usa al apilar; el probador,
igual (`validar_sprites` comprueba que es por dirección y con números enteros).

## Tallas

Escalar pixel art por un factor no entero lo destroza, así que las tallas
salen del cuerpo base **quitando o duplicando filas y columnas enteras** en
el mismo sitio anatómico de cada frame (`tools/actores_tallas.py`). La
cabeza no cambia: el pelo, los cascos y los rasgos valen para las cuatro.

| Talla | Alto | Razas | Operación |
| --- | ---: | --- | --- |
| `medio` | 40 px | la mayoría (1,50-1,90 m) | el cuerpo base |
| `bajo` | 36 px | enanos (1,20-1,50 m) | −1 fila de torso y −3 de pierna; +2 columnas |
| `pequeno` | 33 px | medianos, niños, el duende (0,90-1,20 m) | −3 filas de torso y −4 de pierna |
| `grande` | 45 px | orcos, minotauros, hombres lagarto, dracónidos (1,80-2,50 m) | +3 filas de torso y +2 de pierna; +4 columnas |

PART-02b: cada talla lleva en `personajes.json` sus **anclas** (`tallas.<talla>.anclas`): en cada uno
de los 20 frames, el puño de la mano del arma (`mano`), el de la otra (`otraMano`), el pecho bajo el
cuello (`pecho`) y la línea del cinturón (`cintura`), como `"x,y"` dentro del frame separados por
espacios. Salen del mapa de partes ya operado (`tools/actores_anclas.py`), así que caen en la mano, el
torso y la pelvis de cada frame; de ahí echan sus partículas los anillos, el collar y el cinturón.

## Razas

Una fila por raza en `razas_andando.png` y en `personajes/razas/`, en el
orden de `assets/catalogs/races.json` (el generador comprueba que no falta
ninguna). La talla sale del «Tamaño» de cada descripción y el aspecto de
`race_npc_idle.png`. `sin` son las ranuras que la raza no admite.

| `id` | Nombre | Talla | Piel | Rasgos | Sin |
| --- | --- | --- | --- | --- | --- |
| `aarakocra_de_las_montanas` | Aarakocra de las Montañas | medio | plumas pardas | cresta de plumas, pico, alas | pelo, pies |
| `aarakocra_del_viento` | Aarakocra del Viento | medio | plumas blancas | cresta de plumas, pico, alas | pelo, pies |
| `alquimistas_de_aegroum` | Alquimistas de Aegroum | medio | piel clara | — | — |
| `cara_loca` | Cara Loca | medio | pelaje de gato | orejas de gato, hocico, cola de gato | pelo |
| `cazador_nocturno` | Cazador Nocturno | medio | pelaje de gato gris | orejas de gato, hocico, cola de gato | pelo |
| `diplomaticos_de_bastrea` | Diplomáticos de Bastrea | medio | piel oscura | — | — |
| `draconidos_de_fuego` | Dracónidos de Fuego | grande | escamas rojas | hocico, cuernos, cola de dragón | pelo |
| `draconidos_de_hielo` | Dracónidos de Hielo | grande | escamas de hielo | hocico, cuernos, cola de dragón | pelo |
| `drow_de_la_casa_noble` | Drow de la Casa Noble | medio | piel drow | orejas puntiagudas, corona de esquirlas de obsidiana, corteza en la mandíbula | — |
| `drow_explorador` | Drow Explorador | medio | piel drow | orejas puntiagudas, astas de rama con musgo, musgo en los hombros | — |
| `elfos_de_las_estrellas` | Elfos de las Estrellas | medio | piel élfica | orejas puntiagudas | — |
| `elfos_del_bosque` | Elfos del Bosque | medio | piel élfica | orejas puntiagudas | — |
| `enanos_forjadores` | Enanos Forjadores | bajo | piel rojiza | barba | — |
| `enanos_historiadores` | Enanos Historiadores | bajo | piel rojiza | barba | — |
| `espectros` | Espectros | medio | translúcida | cuerpo de espectro | piernas, pies |
| `guardianes_de_la_luz` | Guardianes de la Luz | medio | piel clara | — | — |
| `guardianes_del_elemento` | Guardianes del Elemento | medio | piel morena | — | — |
| `hechiceros_oscuros` | Hechiceros Oscuros | medio | piel gris | — | — |
| `hombres_lagarto_de_la_selva` | Hombres Lagarto de la Selva | grande | escamas verdes | hocico, espinas, cola | pelo |
| `hombres_lagarto_del_pantano` | Hombres Lagarto del Pantano | grande | escamas oliva | hocico, espinas, cola | pelo |
| `medianos_del_bosque` | Medianos del Bosque | pequeno | piel clara | — | — |
| `medianos_urbanos` | Medianos Urbanos | pequeno | piel morena | — | — |
| `mercenarios_de_ascaria` | Mercenarios de Ascaria | medio | piel aceituna | — | — |
| `minotauros_de_la_llanura` | Minotauros de la Llanura | grande | pelaje de toro | orejas, morro, cuernos de toro, cola | pelo, pies |
| `minotauros_del_laberinto` | Minotauros del Laberinto | grande | pelaje de toro claro | orejas, morro, cuernos de toro, cola | pelo, pies |
| `nagas_arcanos` | Nagas Arcanos | medio | escamas azules | capucha de cobra, cara de serpiente, cuerpo de serpiente | pelo, piernas, pies |
| `nagas_venenosos` | Nagas Venenosos | medio | escamas verdes | capucha de cobra, cara de serpiente, cuerpo de serpiente | pelo, piernas, pies |
| `nobles_de_ascaria` | Nobles de Ascaria | medio | piel clara | — | — |
| `orcos_del_consejo` | Orcos del Consejo | grande | piel de orco | orejas puntiagudas, colmillos | — |
| `orcos_dramaticos` | Orcos Dramáticos | grande | piel de orco clara | orejas puntiagudas, colmillos | — |
| `race_ascaria_fey_blood` | Sangre Feérica de Ascaria | medio | piel feérica | orejas puntiagudas, alas de hada | — |
| `race_dwarf_deepforge` | Enano de las Fraguas Profundas | bajo | piel morena | barba | — |
| `race_elf_canopy` | Elfo del Dosel | medio | piel aceituna | orejas puntiagudas | — |
| `race_human_marches` | Humano de las Marcas | medio | piel clara | barba corta | — |
| `race_orc_gongorguma` | Orco de Gongorguma | grande | piel de orco oliva | orejas puntiagudas, colmillos | — |
| `revenants` | Revenants | medio | piel de aparecido | — | — |
| `drow_de_la_locura` | Drow de la Locura | medio | piel de saga (drow enfermo de musgo), manos de corteza | orejas puntiagudas, astas retorcidas (una rota) con musgo, corteza en media cara | — |
| `drow_del_abismo` | Drow del Abismo | medio | piel de saga azul (drow helado), manos de piedra | orejas puntiagudas, cuernos de carnero de piedra, placas de piedra en media cara, tercer ojo | — |
| `tieflings_de_la_sombra` | Tieflings de la Sombra | medio | piel oscura violácea | cuernos, cola con punta | — |
| `tieflings_del_vacio` | Tieflings del Vacío | medio | piel lila | cuernos, cola con punta | — |
| `traidores_oscuros` | Traidores Oscuros | medio | piel pálida | — | — |
| `yokai_de_las_sombras` | Yokai de las Sombras | medio | piel pálida | orejas de zorro, tres colas de zorro (no se ve: ver PNJ-02) | — |
| `yokai_de_los_vientos` | Yokai de los Vientos | medio | piel élfica | orejas de zorro, tres colas de zorro (no se ve: ver PNJ-02) | — |

## Equipo

`python3 tools/probador_personaje.py --listar` da la lista completa con sus
`id`. Por familias:

- **Cabeza:** capucha (6 colores), chapel con almófar, yelmo cerrado (acero y
  ceniza; tapa el pelo), casco con nasal, casco con cuernos, corona,
  diadema, corona de ramas, sombrero de paja, de cuero, de mago (azul y
  púrpura), gorro de cazador con pluma, pañuelo (rojo y azul), turbante
  (blanco y púrpura), gorro de bufón, máscara de porcelana y antifaz.
- **Torso:** túnica de viaje con bandolera y túnica lisa (4 colores cada
  una), camisa y chaleco, camisa sin mangas, gambesón, cuero y cuero oscuro,
  cota de malla, escamas de bronce y de acero, placas de acero y doradas,
  sobrevesta (azul, roja, blanca), hábitos y túnicas largas (culto,
  carcelero, mago, monje, sacerdote, mercader, harapiento), kimonos,
  vestidos (teja, verde, lino, terciopelo, tabernera), harapos, traje de
  bufón y blusa con corsé.
- **Piernas:** pantalón (5 colores), calzas (roja, verde), falda (roja,
  parda, larga azul), grebas de cuero, calzas de malla y quijotes.
- **Pies:** botas (oscuras, claras, herradas, altas), zapatos, sandalias,
  zuecos, escarpes y pantuflas.
- **Capa:** larga (roja, verde, azul, parda, negra, púrpura) y corta (roja,
  verde, azul). Se construye con la silueta del cuerpo de cada frame, así
  que ondea al andar.
- **Armas** (mano derecha): espada (hierro, acero, oro viejo, cristal,
  ceniza), daga (hierro, plata), hacha (hierro, acero, ceniza), maza,
  martillo, lanza (hierro, bronce), tridente, bastón (roble, cristal) y arco
  (roble, oscuro).
- **Mano izquierda:** escudo (madera, acero, dorado), grimorio, orbe, farol,
  jarra y poción.

Las armas son dibujos pequeños por dirección con un punto de agarre que se
ancla al puño de cada frame y talla: suben y bajan con el brazo al andar.
De perfil el arma va siempre en la mano de delante (cambia de mano al
girar, como en los RPG clásicos) para que se vea en las dos direcciones.

### Del catálogo de objetos a la pieza visible (ESTETICA-01)

Oriol (06/10/2026): «para cada arma y armadura necesitaremos vincularlo a un
sprite, hay algunas que pueden compartir sprites, pero todas tienen que
aplicar una estética»: **forma + esencia + acabado**, y dibujo propio para las
míticas y las únicas. Lo hace `tools/actores_estetica.py`:

- **Forma**: la fila del atlas. Las armas y lo que va en la mano tienen
  formas propias en `tools/actores_formas.py` (`f_daga`, `f_cuchillo`,
  `f_estilete`, `f_sable`, `f_mandoble`, `f_hacha_mano`, `f_gran_hacha`,
  `f_ballesta`, `f_rodela`, `f_paves`, `f_simbolo`...: 40), dibujadas con
  tres materiales: lo principal (la hoja, la cabeza, la cara del escudo), el
  mango y la guarnición. La ropa usa la pieza que elegía `actores_catalogo.py`
  por el nombre.
- **Esencia**: la paleta de lo principal. Las veinte esencias de las armas
  (Ceniza, Cristal Azul, Oro Viejo, Plata, Roble Antiguo, Hierro Santo, Luna,
  Niebla, Penumbra, Plaga, Trueno, Tumba, Estrella, Juramento, Rey Lobo,
  Sangre, Sombra Roja, Corona Rota, Eternidad y Vacío), las veinte deidades de
  las reliquias, o un tema por el nombre (las series poco comunes, las armas
  con nombre) o por las etiquetas (Sagrado, Arcano, Sigilo, Oscuro...).
- **Acabado**: Milicia (acero y azul) y Viaje (hierro y cuero) en lo común;
  Insigne (remates de plata y el tinte más vivo) y Sublime (remates de oro y
  el tinte más hondo); en las armas de esencia, la guarnición dice la rareza
  (latón, plata, oro, oro rojo).

Una forma se dibuja una vez y la viste cada objeto con su **tinte**: los
cinco tonos de cada material cambiados por los de otro. El texto de la pieza
lo lleva (`"f_daga:hierro>plaga,laton>plata"`), y el motor
(`Game::MontajePersonajes`) y el probador lo pintan igual; las paletas van en
`personajes.json` → `paletas`. De los 401 objetos que se ven, 398 se ven
distintos. Quedan 12 sin pieza: anillos, amuletos, lentes y las reliquias
pequeñas de la mano (argolla, caracola, cuerno, escudilla, haz de espigas,
mojón, sello).

Para revisarlo: `python3 tools/revision_estetica.py` deja láminas en
`out/revision_estetica/` (las esencias en sus cinco formas, las veinte
deidades, las series con su Insigne y su Sublime, y el resto por forma).

### La escalera de detalle (ESTETICA-01, desde raras)

Oriol: «la epicidad y nivel de detalle en las armas [...] una escalera de
progresión de lo que debería ser el arma o armadura más básica a la más
fuerte», dibujo grande «desde raras». Lo común y lo poco común llevan la forma
básica; las raras, las épicas y las míticas suben un escalón
(`actores_formas.ESCALERAS`, `f_<forma>_raro`, `_epico`, `_mitico`):

- **raro**: más larga y una columna más ancha, guarda con las puntas vueltas,
  pomo con remate; los escudos, con remaches y el blasón más grande.
- **épico**: pomo y guarda con gema (un cuarto material, `f g h i j`, que la
  esencia tiñe: la gema verde de la plaga, la violeta del vacío), runas en la
  hoja, más grande.
- **mítico**: corona de gema, guarda con alas, más runas: la más grande.

Las cinco formas de esencia (cuchillo, daga, estilete, sable y hacha de mano)
tienen los tres escalones; el resto de formas que llevan objetos raros o
épicos, los que necesitan (35). Se dibujan de frente; de perfil salen
inclinadas (`perfil()`), en espejo (hachas) o con el canto de la básica
(escudos).

### Las únicas, con dibujo propio (ESTETICA-01)

Oriol: «algunas únicas que sean míticas o extremadamente raras, y obviamente
las únicas, aquellas que solo se puedan conseguir». Las veinte míticas de las
deidades tienen su dibujo en `actores_formas.UNICAS` (pieza `f_u_<id del
objeto>`, una fila nueva al final de `arma_<talla>.png`): la Aguja del
Firmamento con su pomo de estrella y el hilo rojo, la Espada de la Brasa
Inextinguible con la hoja en llama, la Hoz del Cuarto Menguante, que es una
luna, la Pluma-Espada del Índice, que es una pluma de cuervo, la Vara del Farol
Encendido... Llevan sus materiales (`tinte` vacío) y el brillo de su deidad.
Las de asta (lanza, tridente, guadaña, bordón, pala, farol) son más altas que
quien las lleva: la cabeza del arma queda por encima de la suya. Las armas
con nombre (raras y épicas) siguen con su forma de la escalera y su tema
hasta tener el suyo.

### Armas al doble, cogidas como en NosTale (ESTETICA-01)

Oriol (07/10): «las armas en la mano del personaje no se ven nada épicas [...]
son demasiado finas y se pierden», y «animando las armas y dándoles un poco más
de juego». Cada forma se sigue dibujando una vez, de frente y en vertical; en
la mano va **al doble** (`actores_poses.doblar`: el doble de grande con el
contorno de un píxel) y **cogida como su tipo**, que no es igual quieto que
andando. Sale de los vídeos de NosTale que pasó Oriol y de sus correcciones
(lámina `como_se_coge_cada_arma.png` en la sesión del 06/10):

| Tipo | Quieto (reposo) | Andando |
| --- | --- | --- |
| Espada, espada corta, sable, estoque | baja, en diagonal hacia fuera, la punta casi en el suelo | detrás, casi horizontal a la altura de la cadera |
| Daga, cuchillo, estilete | agarre invertido (el Asesino): la hoja sale por debajo del puño hacia atrás; de frente sube junto al antebrazo | igual, meciéndose |
| Espadón | delante, la punta arriba y por fuera de la cara (con la punta en el suelo no cabe: el puño va a la cadera) | detrás, la punta hacia abajo |
| Lanza, pica, tridente, jabalinas | tendida a la cadera: la punta hacia fuera de frente y hacia delante de perfil (el Monje de batalla); nunca recta como un bastón | igual |
| Guadaña | cruzada por delante: la hoja abajo junto al pie, el astil sube tras el otro hombro (el Segador) | igual |
| Bastón, bastón herrado | algo más recto que antes, inclinado hacia fuera (de frente) o hacia delante (de perfil); de frente, si pasaría por la cara, la cabeza lo tapa (solo ese trozo) | igual, meciéndose; de perfil, la cabeza hacia atrás |
| Vara, cetro, tea | en la mano, un poco inclinados | igual; de perfil, hacia atrás |
| Arco | vertical al costado, **sin cuerda** | igual |
| Ballesta | horizontal a la cintura (el Explorador) — pendiente: ver más abajo | igual |
| Gran hacha, martillo | **con volumen**: de frente cruzados por delante del cuerpo hacia el hombro contrario, la cabeza junto a la del personaje; de perfil al hombro, la cabeza detrás | de frente igual; de perfil caídos hacia atrás, arrastrando la cabeza (como el espadón) |
| Hacha, hacha de mano | **con volumen** y hoja de barba: la cabeza arriba, junto al hombro y hacia fuera, cogidas por el final del mango | igual, meciéndose |
| Maza, macana | abajo junto a la pierna, la cabeza hacia el suelo (como el revólver del Cazador de demonios) | se mecen como un péndulo |
| Látigo | el mango en el puño y la tralla colgando hasta el suelo | de perfil se arrastra detrás |
| Honda | colgando junto a la pierna | igual |
| Escopeta, rifle, lanzagranadas (SP-02, el Arquero) | horizontales a la cintura, como la ballesta y el lanzacohetes del Destructor: la boca hacia fuera de frente y hacia delante de perfil | igual |
| Cañón de mano (el Cañonero) | **al hombro, con volumen**: de frente cruzado hacia el hombro contrario con la boca junto a la cabeza; de perfil al hombro con la boca detrás | igual (no se arrastra) |
| Revólver (el Cazador de demonios) | abajo junto a la pierna, apuntando al suelo y un poco hacia fuera | se mece |
| Bumerán (el Guardabosques) | en alto junto al costado, el codo arriba y el otro brazo hacia fuera | igual; de perfil, hacia atrás |
| Pistola mágica (SP-02, el Mago de las tinieblas y Merlín) | como el revólver: abajo junto a la pierna | se mece |
| Lanzallamas (la Tormenta de fuego) | como la escopeta: horizontal a la cintura | igual |
| Llave, hoz, vara de rayo, bastón de paseo (CP-01) | como el cetro y la vara de aprendiz: en la mano, un poco inclinados | igual |
| Tótem (CP-01, el Chamán de los Elementos) | como un bastón | igual |
| Laúd, bolsa, matraz, naipes, farol de ánimas (CP-01) | sin pose, a su tamaño: el laúd cogido por el mástil, la bolsa por su cuerda, el matraz y los naipes en la mano, y la vara del farol de pie con el farol colgando por fuera (un farol no se inclina) | igual |
| Tambor y baqueta, sable y pistola (CP-01) | en las dos manos, como los guanteletes: la baqueta y el tambor a la cadera; el sable en la derecha y la pistola en la izquierda | igual |

Cómo (`tools/actores_poses.py`): cada grupo dice, para cada vista (de frente y
de perfil; de espaldas y a la izquierda son su espejo) y cada estado, un gesto
(`colgar`: lo principal por debajo del puño; `alzar`: por encima; `tender`:
casi horizontal) con su sentido y su inclinación en cada paso (paso A, quieto,
paso B; reposo y respirando). Los gestos son cizallas y la traspuesta de la
maestra, que guardan el píxel del dibujo. Las únicas se cogen como la familia a
la que se parecen (`actores_formas.COGIDA_COMO`). Lo de la mano izquierda
(escudos, libros, orbes...) va como siempre.

**Nunca tapan la cara** (Oriol: «en las animaciones hay que evitar siempre tapar la
cara del jugador, a no ser que sea la estética que queramos darle a alguna clase»):
donde el arma se pinta encima del cuerpo (de frente y de perfil), ningún píxel suyo
cae en la cara (la cabeza sin la coronilla; de perfil, su mitad de delante).
`formas_en_talla` prueba la pose y, si tapa, la inclina más hacia fuera; lo que sube
por encima del puño de un arma que cuelga (mango, pomo) se acorta; y si nada basta
(un arco o un cetro en las tallas pequeñas, con el puño cerca de la cabeza) el arma se
lleva colgada. Andando de perfil los bastones y los cetros llevan la cabeza hacia
atrás: con el brazo atrás, inclinados hacia delante pasarían por la cara. Prueba:
`test_ningun_arma_tapa_la_cara`, en todas las tallas y frames.

Las correcciones de Oriol del 07/10 por la tarde: el bastón «un poco más recto»
(pero inclinado le gusta más) y que la cabeza lo tape de frente solo si hace falta y
solo el trozo justo (`TAPA_LA_CABEZA`); el arco sin cuerda (`SIN_CUERDA`); la daga se
queda como estaba (de las tres opciones del puño, eligió la de antes); y la ballesta
no le convence ni como estaba ni cruzada abajo a dos manos: para cogerla de verdad
con las dos manos el cuerpo necesita otra postura de brazos (pendiente).

Las armas de fuego del Arquero (SP-02; Oriol, 07/10: «Añadir el concepte d'armes de foc però,
en comptes de pòlvora alquímia i màgia son la base tecnològica») se dibujan **de lado**, como se
ven de perfil (`actores_formas._de_lado` las pone de pie para la maestra), y tienen su escalón
raro (el de los sets, con la gema como carga alquímica). Van al final del atlas de armas.
Detrás, las dos del Mago: la pistola mágica (llave de latón, cañón fino con boca de campana y,
desde raras, el cristal de maná) y el lanzallamas (el depósito de aceite alquímico bajo el tubo y,
desde raras, la llama piloto en la boquilla).

Los **guanteletes del Artista Marcial** (SP-02; Oriol, 07/10: «Guanteletes grandes», uno en cada
mano) son una sola pieza de la ranura `arma` que se pinta en los dos puños
(`actores_armas.PAREJA`, `capa_pareja`). Se dibujan ya a su tamaño (no se doblan ni se cogen con
pose): de frente, el de la mano de la izquierda de la imagen con el pulgar hacia el cuerpo (el
otro en espejo; de espaldas igual), y de perfil, los nudillos delante. De frente y de espaldas
los dos van encima del cuerpo; de perfil, el de delante encima de todo y el de atrás solo sobre
su mano, su brazo o fuera del cuerpo (si su mano no se ve, no hay guantelete atrás). Cuatro
formas, al final del atlas de armas: `guanteletes` (placas de metal, el puño de cuero y los
aros de guarnición; desde raras, los nudillos de guarnición y la gema en el dorso), `garras`
(la manopla de cuero o pelaje con tres garras bajo los nudillos; con su escalón raro),
`brazos_mecanicos` (el antebrazo de placas con el pistón por fuera y un puño mucho más grande)
y `canon_de_mano` (el Tronador: en la derecha el guantelete con las bobinas; en la izquierda, el
cañón de descargas, que de perfil va en la mano de delante: `abajo_2`/`derecha_2`). Detrás, los
`guantes` del Mago Chasqueador (SETS-03, `tools/actores_formas_iteracion3.py`): el guante blanco de
mago con el puño vuelto y, desde raros, las runas violeta en las yemas; 7×6 de frente y 8×6 de perfil,
lo justo para tapar más del 70 % de la mano en todas las tallas y frames.

#### Armas con volumen (`tools/actores_volumen.py`)

Oriol: «la hacha le falta perspectivas igual que al martillo, se entienden fatal» y
«si aumentas el número de sprites por lado de cada arma todo quedaría súper mejor».
Las armas se dibujaban de lado y se giraban en el plano de la pantalla: la cabeza
de un martillo era siempre el mismo rectángulo, como una pegatina. Ahora las de
`actores_poses.VOLUMEN` (gran hacha, martillo, hachas) se convierten en un objeto de
vóxeles: la maestra al doble extruida con el grosor de cada material (`GROSOR`: la
cabeza del martillo es un bloque, la hoja del hacha es fina y se afila hacia el
filo, el astil es redondo), girada en 3D como se coge en cada vista (la misma
inclinación de siempre, más `giro` sobre su eje para que la cabeza enseñe la cara y
`hondo` hacia el que mira) y proyectada con la cámara un poco desde arriba. Cada vista
y cada paso sale con su perspectiva. La luz de cada cara da los tonos (en las hojas
se queda el tono del dibujo: el filo claro); el contorno va por fuera y donde una
parte tapa a otra; y lo que queda por detrás del cuerpo se marca con su tono «de
detrás» (`DETRAS`), que `pintar_arma` no pinta donde está el cuerpo. La salida es la
misma rejilla de caracteres de siempre: acortar para que quepa, no tapar la cara y
pintar con los materiales del objeto siguen igual.

Las hachas de una mano llevan maestra dibujada ya al doble
(`actores_poses.MAESTRAS_2X`, hecha con polígonos): hoja de barba (filo largo, cuello
estrecho hasta el ojo de metal, el astil asomando por arriba), con virolas las raras,
gema las épicas y runas las míticas; el Hacha del Cuerno Roto, con la hoja de cuerno.
El látigo también: el mango con pomo y virola y la tralla cayendo hasta el suelo.

Para verlo: `python3 tools/probador_personaje.py --raza race_human_marches
--equipo arma=f_mandoble --gif` (anda en las cuatro direcciones y descansa de
frente) y, en el motor, `DEMO_BRILLO_PASEO=1 DEMO_BRILLO_ZOOM=4 demo_brillo_equipo
645 <objeto> <objeto> ...` (andan sin moverse y descansan, girando). Prueba:
`test_cada_arma_se_coge_como_en_nostale`.

### El reposo (frames 13-20)

Como en NosTale, quieto el personaje casi no se mueve, pero está vivo. Cada
rumbo tiene dos frames de reposo (`actores_partes.REPOSO`, sacados del quieto):
el cuerpo quieto y el cuerpo respirando (lo de encima de las piernas baja un
píxel, `actores_tallas.respirar`), con el arma como se lleva parado y un poco
mecida en el segundo; las colas también se mecen. Todas las capas los llevan
(las prendas de PixelLab también: `importar_prendas.py`). En el motor
(`MontajePersonajes::kFrames` = 20, la hoja montada es de 1280×64):
`framesDeReposo(rumbo)` y `cicloDeReposo(rumbo)` (12 fotogramas quieto y 9
respirando, a los 0,07 s del jugador); `HojaPersonaje::animarReposo` da al actor
`idle_up/right/down/left` (e `idle`, el de abajo), y `Player` pasa a ellos cuando
lleva 0,2 s parado (entre dos pasos seguidos sigue el ciclo de andar). Con una
hoja de 12 frames (`personaje.png`, los horneados) no hay reposo y se queda en
el quieto de andar, como antes. La vista previa de la creación anda, descansa y
gira.

### Que nada se salga de la celda

La celda es de 64×64 y el puño queda a 10-16 píxeles del borde de abajo (menos
en las tallas pequeñas): un arma al doble no cabe entera. `pintar_arma` recorta
sin avisar, así que `actores_armas.formas_en_talla` acorta la maestra (al doble)
lo justo para su talla, con todos los pasos de un mismo estado a la vez (el arma
no cambia de largo al andar): por cada lado del puño, quita tramos repetidos de
la hoja o del astil (filas iguales, o corridas una columna), de los tramos más
largos primero, y la punta, la guarda, el pomo y las runas se quedan. Si lo que
se sale es el ancho de la cabeza y no el largo (el tridente tendido en las
tallas pequeñas), el arma se endereza un poco. Lo de la mano izquierda se acorta
igual sin doblar. `test_validar_sprites` comprueba que ningún arma se sale en
ninguna talla ni en ningún frame.

El atlas de armas tiene ahora 99 filas (6336 px de alto). Una textura de más
de 8192 px de alto no carga en todas las tarjetas: caben 128 filas.

### El brillo por fuerza (ESTETICA-01, estilo NosTale)

Oriol: «nada de implementar el sistema de mejoras de NosTale [...] podemos
igualar los brillos en relación a lo fuerte que sea el arma: de +1 a +2 Tier
1, +3 a +4 Tier 2, y así consecutivamente». Cada objeto del catálogo lleva
`brillo`: su nivel (1..10: el primero del par de su tier para la pieza de
entrada, el segundo para la fuerte, las reliquias y las únicas), su color y sus
partículas (una de las 40 familias de `tools/particulas.py`: las de los 13
elementos y las de los temas de las clases), por su esencia, su deidad o su tema
(`actores_estetica.brillo`). PART-01: las piezas de los sets de las clases llevan
la familia de su clase (la primera de `particulas.CLASES`: los guanteletes del
Puño del dragón, dragones; los de las Artes Místicas, pétalos), y el color de esa
familia (`planificacion/CANON_PARTICULAS.md`).

Cuánto brilla cada nivel lo dice el motor (`Game::escalonDeBrillo`):

| Nivel | Tier | Armas | Ropa |
|---|---|---|---|
| +1, +2 | 1 | destellos sueltos | destellos sueltos |
| +3, +4 | 2 | + halo fino que late | + barrido de luz |
| +5, +6 | 3 | + partículas de su esencia | + borde por fuera del cuerpo y alguna partícula |
| +7, +8 | 4 | halo más ancho, más partículas | |
| +9, +10 | 5 | halo fuerte, muchas partículas | |

PART-01 (Oriol, con la prueba del Cazademonios por tier): las partículas también suben con el tier: un
pelo más opacas en el Tier 3, más intensas en el 4 y más aún en el 5, que además echa un tercio de un
tono más oscuro, de contraste (`EscalonDeBrillo::opacidad` y `contraste`).

Lo que no se ve puesto (anillos, amuletos, reliquias pequeñas) también brilla:
`equipo_catalogo.json` → `sinPieza` lleva su `brillo` (por lo que da o por su
lore: el anillo de vitalidad, luz roja; el sello de la Serpiente, veneno) y el
motor lo echa sin halo y con partículas desde el Tier 1
(`Game::escalonDeAbalorio`). PART-02b: **desde su sitio**, con las anclas de la
talla: el anillo de la mano principal, del puño de la mano del arma; el de la
otra mano, de la otra; el collar, del pecho; el cinturón, a lo ancho de la
cintura; lo que se lleva en la mano sin verse (la caracola, el cuerno), de la
mano. El sitio sale de la ranura del catálogo (`slot`) y de dónde va puesto
(`Vestuario::sitioDeAbalorio`); sin anclas, del cuerpo entero como antes. Un
abalorio con dos efectos (`otrasParticulas` y `otroColor` en su `brillo`) echa
las partículas de los dos a medias: el anillo de fuego y hielo de Oriol, llamas
y escarcha de la mano.

El brillo sale solo de lo que se ve de cada pieza (`MontajePersonajes::montar`
con `MapaDeCapas` dice qué capa queda arriba en cada pixel). `HojaPersonaje`
monta el halo en otra textura con el mismo reparto de frames, y
`AuraJugador` lo dibuja sumando luz encima del sprite, con su latido, y los
destellos y partículas pixel a pixel (las de tema, con un contorno oscuro para
que se lean encima del cuerpo). `demo_brillo_equipo` lo enseña en el motor de
verdad (vuelca fotogramas a PPM); con `pieza@familia` le pone a una pieza las
partículas de una familia y con `@familia` viste un abalorio que las echa
(PART-02b: `mano@llamas,escarcha` en la mano del arma, con dos efectos;
`otraMano@`, `pecho@` y `cintura@`, en los otros sitios).

### Los sets de especialista (SP-01, SP-02)

Las SP de NosTale son clases (`tools/importar_clases_sp.py`) y cada una tiene **un set
entero**: arma, cabeza, torso, piernas y pies, objetos raros que cualquiera puede ponerse. La
clase no lo lleva puesto (Oriol, 07/10: «son magos/guerreros/arqueros, no deben llevar el set
entero»). Las piezas que piden silueta propia se dibujan en `tools/actores_trajes.py` (hombreras
grandes, tabardos, abrigos largos, sombreros de ala, cascos con cresta o penacho; ninguna tapa
la cara) y se registran al final de cada atlas, así que ninguna fila de antes se mueve (las de
PixelLab, que se reponen después, sí). `actores_estetica.PIEZA_DE_ROPA` liga el nombre del
objeto con su pieza; las que no necesitan dibujo propio usan una del catálogo.

Una paleta nueva de `actores_trajes` nunca puede pisar una que ya existe (el módulo no carga):
cambiaría todo lo que la usa. Y no debe compartir tonos con otra (el generador sabe de qué
material es cada píxel por su color).

Los doce sets del Espadachín (los del Arquero, más abajo):

| SP | Lo dibujado (`actores_trajes`) | Lo del catálogo, con el tinte de su tema |
|---|---|---|
| Guerrero | coraza de placas con tabardo carmesí y llama; yelmo con penacho | quijotes y escarpes |
| Ninja | gi cruzado azul noche, fajín, bufanda larga celeste, brazales; cinta con placa y colas; hakama atada; tabi | — |
| Cruzado | placas de plata, tabardo blanco con la cruz de oro; yelmo alado | quijotes y escarpes de plata |
| Berserker | placas negras, hombreras con pinchos de hueso, faja roja; yelmo astado | quijotes y escarpes negros |
| Gladiador | coraza musculada, hombrera, faldellín (SP-01); galea con cresta | grebas (SP-01), sandalias |
| Monje de lucha | hábito azul con cuello cruzado y mangas anchas, rosario grande; diadema de cuentas | calzones blancos, sandalias |
| Cosechador de almas | manto negro con forro violeta y calavera en el cinturón; capucha honda; botas con hebillas | calzas negras |
| Renegado | armadura partida (blanco y oro / negro y violeta: `mitad`); diadema con gema | quijotes blancos, botas negras |
| Hachero de la cascada | correajes en aspa con la runa del agua, manto de piel; casco con borde de piel; botas de piel | calzas azules con grebas de cuero |
| Caballero Draconiano | escamas blancas, alas de oro, hombreras de ala; yelmo con cresta de dragón | quijotes y escarpes blancos |
| Romperrocas | camisa, peto de minero con tirantes, bolsas; casco con lámpara y gafas | pantalones y botas herradas |
| Aquiles | coraza de bronce oscuro, faldellín, manto violeta del Estigia; galea con cresta negra; grebas | sandalias oscuras |

Las piezas dibujadas usan paletas **sin tipo** (`actores_estetica.TIPO` no las conoce): ningún
tema las toca. El tema del set (`TEMAS_POR_NOMBRE`, `del <sp>`) tiñe el arma y lo que el set
toma del catálogo. Las cabezas pueden llevar varias plantillas con su propio material 2 (el
yelmo y sus alas, el casco con sus gafas y su lámpara), y ninguna sube de la fila 6 (en la
talla grande la cabeza sube 5). Cada set se mira en las cuatro tallas.

    demo_brillo_equipo 645 espadon_del_guerrero+yelmo_del_guerrero+coraza_del_guerrero+...

viste a una figura con el set entero (`+` junta varios objetos en la misma figura).

Los doce sets del Arquero (`tools/actores_trajes_arquero.py`, que se registra detrás de los del
Espadachín):

| SP | Lo dibujado | Lo del catálogo, con el tinte de su tema |
|---|---|---|
| Cazador | jubón de cuero con mangas verde azulado, pañuelo azul, aljaba a la espalda; capucha con pluma | calzas de cuero, botas oscuras |
| Asesino | chaleco de cuero negro sobre camisa gris azulada, correa de cuchillos, faja roja; capucha honda con forro rojo | calzas y botas negras |
| Destructor | chaqueta acolchada naranja con la bandolera de bombas; pañuelo atado con gafas de taller | pantalones pardos, botas herradas |
| Guardabosques | túnica verde, manto de hojas con cuello de piel; diadema con astas de ciervo; botas con borde de piel | calzas verdes |
| Cañonero | chaqueta de cuero de aviador, cuello de borrego, bufanda roja, correa; gorro de aviador con orejeras y gafas de vuelo | pantalones y botas altas de cuero |
| Espía | gabardina corta azul pizarra con forro tostado y el halcón en el hombro; gorro con plumas de halcón | calzas tostadas, botas claras |
| Cazademonios | las piezas del cazador de demonios de SP-01 | pantalones negros |
| Ángel vengador | placas blancas y oro, sol en el pecho, dos alas pequeñas a la espalda; aro de oro con alitas | quijotes y escarpes blancos |
| Cazasoles | peto de cuero claro con el sol de oro, piel de lobo en los hombros; tocado de cabeza de lobo con el sol | calzas tostadas, botas claras |
| Granadero | peto de acero sobre camisa de campaña, correa de granadas; casco con visera de cristal | pantalones de campaña, botas herradas de acero |
| Cazanieblas | gabardina larga pizarra con forro oliva, botes de gas y la máscara al cuello; capucha honda | pantalones y botas oscuros |
| Almirante Yi | armadura de Joseon azul tachonada de oro con forro rojo; sombrero negro de ala ancha con borla roja | grebas y botas negras |

Los doce sets del Mago (`tools/actores_trajes_mago.py`, detrás de los del Arquero):

| SP | Lo dibujado | Lo del catálogo, con el tinte de su tema |
|---|---|---|
| Mago de fuego | la corona de llamas y la túnica del mago de fuego de SP-01 | calzas granate, zapatos |
| Mago sagrado | casulla blanca con la estola y la cruz de oro; mitra blanca con la cruz (de espaldas, las ínfulas) | calzas blancas, sandalias de oro |
| Mago azul | túnica azul con hombreras de hielo, cuello de piel y el bajo blanco de copos; capucha con ribete de piel y el cristal de hielo; botas de piel blanca | calzas azules |
| Mago de las tinieblas | levita negra con forro y solapas violetas, botones de plata y chorrera; sombrero de copa con la cinta violeta | pantalones y botas negras |
| Volcano | coraza de basalto con vetas de lava y hombreras con picos de obsidiana; yelmo con cuernos de obsidiana y puntas de lava | grebas y botas de basalto |
| Señor de las mareas | coraza de escamas verde mar, hombreras de concha y la capa de agua; corona de coral y oro con perlas | calzas verde mar, sandalias de coral |
| Vidente | túnica violeta con estrellas de oro, faja magenta con dos cartas; turbante magenta con la joya del ojo | calzas y babuchas violetas |
| Archimago | túnica blanca y oro con el cuello alto y el libro al cinto; sombrero alto con la estrella | calzas y zapatos blancos |
| Sacerdote vudú | el pecho pintado de blanco, correas cruzadas, collar de huesos; tocado de plumas verdes y rojas con la calavera; faldellín de rafia | sandalias de cuero |
| Grávitas | túnica azul noche con las piedras que orbitan alrededor; aro de piedra con la piedra que flota sobre la cabeza | calzas azul noche, botas de piedra |
| Tormenta de fuego | chaqueta ignífuga negra con bandas naranjas y el depósito a la espalda; casco de bombero de latón | pantalones y botas altas negras |
| Merlín | túnica blanca con el bajo de flores bajo el manto azul cielo con runas de oro; corona de flores | calzas y botas blancas |

Los ocho sets del Artista Marcial (`tools/actores_trajes_artista.py`, detrás de los del Mago), con
sus guanteletes:

| SP | Lo dibujado | Lo del catálogo, con el tinte de su tema |
|---|---|---|
| Puño del dragón | chaqueta de kung-fu roja con el cuello de oro, los botones de nudo, la faja negra y el dragón de oro a la espalda; la cinta del Ninja en rojo con la placa de oro; pantalones anchos negros atados en rojo y zapatillas de tela | guanteletes granates con los nudillos de oro |
| Artes Místicas | túnica blanca cruzada con la solapa azul, mangas anchas y la faja azul con flores de loto; aro de plata con la luna creciente; pantalones anchos azules | guanteletes azules y blancos con la gema rosa (el loto), sandalias |
| Maestro Lobo | chaleco de piel gris oscuro abierto sobre el pecho vendado, el cuello de piel, los brazos vendados y la faja blanca; el tocado de lobo del Cazasoles en gris; vendas en los pies | garras de plata con la manopla gris oscuro, pantalones gris pizarra |
| Guerrero demoníaco | coraza negra con hombreras de ribete y pinchos rojos y las alas de murciélago; diadema negra con la gema roja y los cuernos del Berserker en negro; botas negras con punteras rojas | garras rojas con la manopla negra, grebas negras |
| Druida flamígero | jubón de cuero con la piel del oso pardo en los hombros, las hojas rojas y el collar de colmillos; la cabeza de oso con las orejas redondas y los ojos de brasa; botas de piel | garras de brasa con la manopla de piel de oso, calzas de cuero |
| Puño Hidráulico | peto de cuero con placas de hierro, hombreras de hierro y bronce y el depósito de combustible a la espalda; gafas de mecánico de bronce con lentes violeta; botas con punteras de hierro | brazos mecánicos de bronce y hierro, pantalones de mecánico con rodilleras de hierro |
| Tronador | chaqueta amarilla con las bandas blancas, las bobinas de cobre en los hombros y el cable cruzado; casco de latón con la antena del rayo | guanteletes de acero (el izquierdo, el cañón de descargas), pantalones y botas negros |
| Thor | placas de plata con el rayo de oro, hombreras azul hielo y la capa azul oscuro; el yelmo alado del Cruzado en plata | guanteletes azul hielo con los aros de oro, grebas y botas de plata |

Los veinte sets de las clases propias (CP-01; Oriol, 07/10: «una armadura de tier dos para cada
uno»; `tools/actores_trajes_propias.py`, detrás de los del Artista Marcial; las armas nuevas en
`tools/actores_formas_propias.py`):

| Set | Lo dibujado | Lo del catálogo (o forma nueva), con el tinte de su tema |
|---|---|---|
| Comediante | jubón partido negro y blanco con la gorguera y los botones de oro; sombrero de ala negro con la pluma blanca y las dos máscaras en la copa (la blanca sonriente y la dorada triste); calzas de una pierna negra y otra blanca | estoque raro con la cazoleta de oro; zapatos negros |
| Juglar | sobrevesta a rombos rojos y dorados con las mangas de oro y la correa del laúd; birrete rojo con la pluma de faisán | laúd de nogal (forma nueva); calzas rojas, botas de cuero |
| Tamborilero | casaca militar azul con los correajes blancos y los botones de oro; chacó azul con la visera, el penacho rojo y los cordones; pantalón blanco con la franja roja | tambor azul a la cadera y baqueta (forma nueva, en las dos manos); botas altas negras |
| Feriante | frac rojo con las solapas negras y los botones de oro; chistera roja con la cinta dorada; pantalón a rayas negras y blancas | bastón de paseo negro con el pomo de oro (forma nueva); botas negras |
| Espiga Dorada | túnica ocre hasta la rodilla con la estola verde, la espiga y el bajo de oro y el cinturón de cuerda; sombrero de paja con las espigas | hoz de oro (forma nueva); pantalón pardo, sandalias |
| Caminante Gris | hábito gris con la esclavina, el cinturón y la calabaza; sombrero de peregrino con la concha | bordón herrado; pantalón gris, botas |
| Juramento | coraza de acero con el peto, el martillo de latón, las hombreras y el delantal de herrero; yelmo con la cresta del yunque (la cara se ve) | martillo raro de acero; quijotes y escarpes |
| Rey Lobo | coraza de hierro ennegrecido con el manto de piel de lobo y la cabeza del lobo de plata; yelmo con las orejas de lobo; botas de piel | espadón escarchado; quijotes de hierro negro |
| Ingeniero | camisa de lona con el mandil, las bolsas de herramientas, el correaje y las hombreras; casco de zapador con las gafas subidas; pantalón con rodilleras; botas con puntera de hierro | llave inglesa (forma nueva) |
| Mercader | casaca de brocado granate y oro con la balanza, los botones y las bolsas; bonete granate con la pluma y la moneda de oro; bombachos pardos | bolsa de monedas (forma nueva); botas altas marrones |
| Alquimista | gabán verde botella con el forro, el mandil manchado y la bandolera de frascos; sombrero de copa baja con los anteojos de latón | matraz dorado (forma nueva); pantalón y botas altas negros |
| Cartomante | levita morada de cuello alto sobre el chaleco negro con los botones de oro; diadema morada con tres naipes en abanico | abanico de naipes (forma nueva); pantalón y zapatos negros |
| Druida | túnica verde musgo hasta los pies con el manto de hojas de roble y el cinturón de corteza; corona de hojas con bellotas y dos astas de ciervo; pantalón vendado | báculo con la cabeza de hojas; sandalias |
| Domador | chaleco de cuero sobre la camisa con la hombrera de piel de oso y el collar de colmillos; la cabeza de oso (orejas, ojos, hocico y colmillos); botas con la vuelta de piel | látigo; pantalón de cuero |
| Chamán de los Elementos | el pecho pintado de rojo y ocre, el chaleco de cuero, la piel en los hombros y el collar de huesos; la cinta con los cuernos de carnero; pantalón con flecos; botas de piel | tótem con la turquesa (forma nueva) |
| Chamán de los Espíritus | cota de malla con las hombreras de hueso y la runa turquesa; banda en la frente con el amuleto y dos trenzas con cuentas; botas de piel | hacha de mano de acero rúnico; faldar de malla |
| Tempestario | túnica azul noche con el rayo y el bajo amarillos y las bobinas de cobre en los hombros; sombrero de ala con dos pararrayos y el rayo que salta entre ellos; botas con hebillas de cobre | vara de horquilla de cobre con el rayo (forma nueva); pantalón azul oscuro |
| Necromago | túnica negra raída hasta los pies con las mangas y el bajo violeta, el cinturón de cadena y el cordón de huesos; capucha negra con la corona de velas de luz verde (la Santa Compaña) | farol de ánimas colgado de una vara (forma nueva); pantalón y botas negros |
| Corsario Vampírico | casaca larga granate de cuello alto con el forro rojo, los botones de plata y la chorrera; tricornio negro con la calavera alada de plata; botas altas con la vuelta granate | sable de hoja negra con la guarda de plata; calzones negros |
| Forajido | camisa blanca, chaleco de cuero, fajín rojo y bandolera con las balas rúnicas; pañuelo rojo y el aro de oro en la oreja; pantalón a rayas rojas y blancas | sable y pistola de sal de fuego (forma nueva, en las dos manos); botas marrones |

Los seis sets de la 3.ª iteración (SETS-03; Oriol, 08/10: los sets del Apóstata, el Señor de las
Armas, la Furia de los Creadores, el Bardo de las Cuatro Estaciones, el Mago Chasqueador y el Monje del
Despertar «siguiendo los mismos criterios»; `tools/actores_trajes_iteracion3.py`, detrás de los de las
clases propias; canon en `planificacion/CANON_SETS_ITERACION3.md`):

| Set | Lo dibujado | Lo del catálogo (o forma nueva), con el tinte de su tema |
|---|---|---|
| Apóstata | coraza de paladín ennegrecida con las hombreras de pinchos y el tabardo gris rasgado con el sol de la orden boca abajo, la cadena por cinturón; capucha granate con la aureola partida encima | sable de hoja negra con el filo rojo; grebas ennegrecidas, botas altas negras |
| Señor de las Armas | manto real carmesí hasta los pies con el cuello de armiño y su arsenal flotando a la espalda (dos espadas a cada lado); corona de espadas de punta con el rubí | orbe de oro en su montura de plata; calzas carmesí, zapatos dorados |
| Furia | peto de placas blancas con los cantos de oro, los brazos al aire, la piel de oso negra y la faja roja; yelmo blanco con la corona de rayos de oro detrás; botas de cuero rojo con el borde de piel negra | maza de aletas de oro con el mango de cuero rojo; grebas blancas |
| Cuatro Estaciones | poncho de lana dorada (las Praderas de Oro de los hombres gato) con la Marcha de las Cuatro Estaciones tejida en las cenefas y los flecos; corona de hojas que cambia de estación al dar la vuelta y deja libres las orejas; pantalones de lana parda | sable de hoja escarchada; botas blandas de cuero pardo |
| Chasqueador | chaqueta corta azul noche de cuello alto con los alamares de plata y la faja violeta; bombín con la cinta violeta y la estrella de plata; pantalón con la franja violeta | guantes blancos de mago con las runas violeta (forma nueva, en las dos manos); botas altas negras |
| Despertar | hábito de marfil sin mangas con las solapas y la faja de jade y las vendas; diadema con el tercer ojo de jade; pantalones de jade atados al tobillo | guanteletes de jade con los nudillos de oro; sandalias |

## Personajes montados

`personajes.json` → `personajes` guarda los 19 montados como datos: los 9
actores de `actores_capas.png` y los 10 de historia. Un personaje nuevo es
una entrada así:

```json
{"raza": "race_human_marches", "piel": "piel_morena", "ojos": "marron",
 "pelo": ["corto", "pelo_negro"], "barba": ["BARBA", "pelo_negro"],
 "equipo": {"cabeza": "turbante_purpura", "torso": "tunica_mercader",
            "pies": "zapatos", "mano_izq": "pocion"}}
```

`pelo: null` deja calvo, `barba: null` afeita la barba de la raza,
`quitar_capas` quita capas de la raza (la naga del río tiene cara y pelo de
persona) y `talla` cambia la talla de un humano (la niña y el duende son
`pequeno`, el verdugo `grande`).

## Los PNJ (PNJ-01)

Desde el 05/10/2026 **todos los PNJ se montan con las piezas del jugador**
(Oriol: «cualquier jugador podría copiar la estética de un PNJ estándar; los
equipos y las armas cambian la apariencia, y puede haber equipo solo de
PNJ»). Cada objeto de categoría `npc` de `assets/objects/` tiene su fila en
las hojas `pnj_boundington.png`, `pnj_mundo_1.png` y `pnj_mundo_2.png`; ya no
hay dibujos de PixelLab ni «ropa prestada» de otro PNJ (los alias se quitaron
de `sprites.json`, y las hojas `pnj_*` van las últimas del índice).

- **La base** sale de lo que ofrece la creación: raza, piel y talla (solo los
  14 pueblos de cuerpo liso), ojos, peinado, barba y ropa de diario.
- **El equipo** es lo que llevan puesto o en la mano. Para algunos oficios hay
  **prendas solo de PNJ** (`"soloPnj": true` en `equipo`): la librea de la
  guardia de Boundington, el mandil del herrero, la casulla, la toga, el
  birrete, la librea de palacio, la cesta, el libro de cuentas y el reloj de
  arena del viajero de la Plaza de las Razas (CHRONOS, en la mano izquierda). Se montan
  como las demás, pero la creación no las ofrece (`EditorApariencia`) y
  ningún objeto del catálogo las viste.
- **Variantes:** un PNJ colocado varias veces (la anciana, la guardia, los
  PNJ de raza de la plaza) tiene hasta seis caras: filas `<id>~2`, `<id>~3`…
  y `"variantes"` en la fila del id. El motor da la colocación *n* del nivel a
  la variante *n* (`SpriteActor::filaPara`).

Las composiciones están en `tools/actores_pnj.py`: a mano los de Boundington
y las naciones del oeste (con su ficha o su retrato de PixelLab), y por reglas
el resto (oficio por la id, el nombre o la carta de `npcs.json`; pueblo por
`raceId`, la nación o el lugar de la carta; sexo por el nombre; semilla de la
id). Las prendas nuevas, en `tools/actores_oficios.py`. `gen_personajes.py`
las hornea (`tools/gen_pnj.py`) y `personajes.json` → `horneados` las lista,
así que `test_personajes_capas` comprueba que el motor monta **cada PNJ**
byte a byte con el mismo código que al jugador. `tools/validar_pnj.py`
(también en `validar_todo.py`) comprueba que ninguno cae en otra hoja, que
todos son copiables salvo sus prendas de oficio y que no se repiten.

```sh
python3 tools/gen_personajes.py                      # todo, PNJ incluidos
python3 tools/validar_pnj.py
python3 tools/probador_personaje.py --personaje skilla --gif
python3 tools/probador_personaje.py --personaje anciana~3
```

Un PNJ nuevo en un catálogo sale solo al regenerar (por reglas); uno con
historia se escribe en `actores_pnj.PNJ`.

## Los pueblos del lore (PNJ-02)

Oriol, 05/10/2026: «Pueblos del este: Choubar (hombres pájaro), Mistarium
(tieflings) y Nocturnsea (inhabitada, es la entrada a la casa de la muerte,
dicen los antiguos). Gongorguma son orcos. Ayashe, drows. Los yokai son
espíritus, así que ya dejaron de tener cuerpo material: siguen en el lore,
pero no son visibles. Los sagas son los drows».

- **Naciones** (`actores_pnj.NACIONES`): Choubar → aarakocra; Mistarium y su
  clan Neblinia → tieflings; Gongorguma y Qethatos → orcos; Ayashii → drows;
  Nocturnsea → **muertos**: la farera y el posadero de Grytoz son espectros y
  el herrero, el mercader y el tabernero, renacidos (mismos sitios y mismos
  diálogos). La nación sale de la id o, si la id no la dice, del nombre de los
  niveles donde está colocado el PNJ («Choubar - Klimnebra»): así el capitán
  Ventobélico es aarakocra y la archivera de los Espejos, tiefling.
- **Yokai:** ningún PNJ puede serlo (`actores_pnj.SIN_CUERPO`; `comprobar()`
  lo rechaza). No hay objetos `npc_race_yokai_*`, la Plaza de las Razas tiene
  **once** pueblos y la creación de personaje no los ofrece ni los acepta
  (`"playable": false` en `races.json` → `RaceDefinition::playable`). Sus
  capas siguen en el atlas: el códice y el lore los conservan.
- **Los drows, corrompidos** (`tools/actores_corrupcion.py`): elfos a los que
  les crece encima piedra, corteza, musgo y partes de animal, sin perder la
  forma (orejas puntiagudas, cuerpo esbelto, pelo claro). De menos a más: la
  casa noble (corona de esquirlas de obsidiana, corteza en la mandíbula), el
  explorador (astas de rama, musgo), las sagas de la locura (astas retorcidas,
  corteza en media cara y en las manos) y las del abismo (cuernos de carnero
  de piedra, placas de piedra, manos de piedra, tercer ojo). Las sagas son
  drows: la piel es la del drow, enferma de musgo o helada de abismo, y ya no
  llevan la nariz ganchuda. Las plantillas se dibujan por coordenadas
  (`Dibujo`: trazo de 1 px + contorno, espejo para el otro lado o la espalda)
  y se registran en `actores_rasgos`.

## Cómo se hace

| Módulo | Qué pone |
| --- | --- |
| `actores_partes.py` | Mapa de partes del cuerpo base: cada píxel es cabeza, cuello, torso, pelvis, brazo, mano, pierna o pie. |
| `actores_tallas.py` | Las cuatro tallas por cirugía de filas y columnas, aplicada al dibujo y al mapa de partes. |
| `actores_pintura.py` | Prendas que siguen la silueta: un material de cinco tonos sobre unas partes conserva el volumen y el ciclo de caminar. |
| `actores_equipo.py` | Torso, piernas, pies y capas: constructores (túnica, cota, pantalón, botas…) con materiales y texturas (anillas, escamas, acolchado). |
| `actores_tocados.py`, `actores_pelo.py`, `actores_cascos.py`, `actores_rasgos.py` | Plantillas por dirección de pelo, cascos, sombreros, orejas, cuernos, morros y barbas. `actores_siluetas.py` sombrea una silueta con la luz del cuerpo base. |
| `actores_colas.py`, `actores_alas.py`, `actores_cuerpos.py` | Colas (tubos sombreados anclados a la cadera), alas ancladas a los hombros, la cola de serpiente de las nagas y el jirón de los espectros. |
| `actores_armas.py` | Armas por familias ancladas al puño. |
| `actores_anclas.py` | PART-02b: las anclas de cada talla y frame (las dos manos, el pecho y la cintura), de donde echan los abalorios. |
| `actores_razas.py`, `actores_personajes.py` | Las 43 razas y los personajes montados, como datos. |
| `actores_oficios.py`, `actores_pnj.py`, `gen_pnj.py` | PNJ-01: las prendas de oficio (solo PNJ) y la ropa nueva; las composiciones de todos los PNJ y sus variantes; el horneado de las hojas `pnj_*`. |
| `actores_corrupcion.py` | PNJ-02: la corrupción de los drows (corona de piedra, astas, musgo, corteza, cuernos de carnero, placas, tercer ojo), dibujada por coordenadas. |
| `actores_materiales.py` | Las rampas de cinco tonos (pieles, pelajes, escamas, telas, metales). |

### Añadir una pieza

1. Si hace falta, un material de cinco colores en `actores_materiales.py`.
2. Una entrada en el catálogo de su ranura (`TORSO`, `PIERNAS`, `PIES` o
   `CAPAS` en `actores_equipo.py`, `_pieza()` en `actores_cascos.py`,
   `ARMAS` en `actores_armas.py`). Casi siempre basta un constructor
   existente con otros materiales.
3. Regenerar, validar y mirarla en el probador. Si es de un objeto del
   catálogo, añadir la palabra a `actores_catalogo.py` (ropa) o la forma a
   `actores_estetica.FORMAS_DE_ARMA` (armas), y su tema si lo necesita.

## Pendiente

- ~~Cablear en el motor~~ **Hecho en APARIENCIA-01** para el jugador:
  `Game::ManifiestoPersonajes` lee este contrato, `Game::MontajePersonajes`
  apila las capas, `Render/HojaPersonaje` sube la hoja a la GPU y reparte
  los ciclos de andar (también los de `personaje.png`, que se leía mal), y el
  equipo puesto se ve (`Game::Vestuario`, con `equipo_catalogo.json`).
  ~~Queda montar así a los PNJ~~: **PNJ-01** los hornea con las mismas piezas
  (el motor los monta igual, lo comprueba `test_personajes_capas`).
- Las 43 razas y los 19 personajes usan el cuerpo de 40 px; los retratos
  de `boundington_story_actors_idle.png` miden unos 58. La escala común
  sigue siendo una decisión de producto.
- Criaturas no humanoides del atlas de historia (el monstruo del pantano,
  la rata) necesitan su propio cuerpo base.
- ESTETICA-01: los PNJ horneados no tienen reposo (sus hojas son de 12 frames y
  el motor los deja en el quieto); y no hay guanteletes (el artista marcial de
  NosTale los lleva enormes en las dos manos): ¿familia nueva para el Monje del
  Despertar?
- ESTETICA-01: lo que se coge con las dos manos (la ballesta, y del todo la gran
  hacha y el martillo) necesita otra postura de brazos en el cuerpo (los dos puños
  juntos delante, como el Explorador o el Destructor de NosTale): el cuerpo, la ropa
  y el motor (una variante por postura). La ballesta se queda como estaba hasta
  entonces. Y solo hay andar: si se quiere correr (el bastón «caído atrás al
  correr, más recto al andar»), hace falta un ciclo de correr.
