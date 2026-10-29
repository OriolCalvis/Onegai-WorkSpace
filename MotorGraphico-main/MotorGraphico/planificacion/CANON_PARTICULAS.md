# Canon PART-01 — Los elementos y las partículas

Oriol (07/10/2026): «Ahora sería trabajar partículas que reflejen todos los elementos creados, por
ejemplo apóstata partículas de sangre de color rojo sangre, bárbaro furia y aura, el señor de las
armas yo la haría dorada, etc. En verdad no será una partícula para cada clase, ni nada de eso, más
bien que todas tengan partículas y los efectos visuales necesarios: no debería tener las mismas
partículas en los puños un Artes Místicas que un Puño de dragón (...) podríamos separar druida
flamígero, más puro fuego, puño dragón sprites de dragón. Hay que analizar qué habilidades aporta cada
clase y hacer una lista de las partículas requeridas para todas». Y sobre el rayo, que en NosTale es
Luz: «hay que cambiarlo porque no estamos limitados a los 4 elementos del nostale aquí».

## 0. En corto

- **13 elementos** (§1), no los cuatro de NosTale: Fuego, Agua, Hielo, Rayo, Tierra, Viento, Luz,
  Tinieblas, Naturaleza, Veneno, Sangre, Arcano y Vacío. El elemento de una carta es **de lo que está
  hecha** y decide su color y sus partículas.
- **40 familias de partículas** (§2): las 13 de los elementos y 27 de los temas de las clases
  (dragones, pétalos, notas, naipes, monedas, ánimas...), cada una con su forma, su movimiento y sus
  colores.
- **Cada clase, sus partículas** (§3): de una a seis familias, sacadas de lo que hacen sus cartas;
  la primera es la suya.
- **Dónde se ven** (§4): en el equipo, por lo fuerte que es (la escalera de ESTETICA-01: partículas
  desde el Tier 3, las de un +10 solo en el Tier 5); en los **abalorios** de Tier 2 (anillos, collar
  y cinturón), que las echan desde su sitio (§6); y en las cartas al jugarse.
- **Ranuras por tier** (§5): el Tier 1 es casco, pechera, brazos, piernas, zapatos y armas; el Tier 2
  desbloquea collar, cinturón y anillos.

Datos: `tools/elementos.py` (los elementos), `tools/particulas.py` (las familias y las de cada
clase). Motor: `Game::tipoDeParticula` y `Game::pixelesDeForma`
(`src/Game/Personajes/FamiliasDeParticulas.cpp`), que pinta `Render/AuraJugador`. Las dos listas de
familias son la misma (lo comprueba `test_validar_sprites.ParticulasTests`), y
`test_brillo_equipo` comprueba que cada familia sale con su forma, sus colores y su movimiento.

## 1. Los elementos

| Elemento | En la carta | Color | Partículas | Cartas | Clases (SP y propias) |
|---|---|---|---|---|---|
| **Fuego** | «3d6 de daño de fuego» | `#ff8c3c` | llamas | 102 | 12 |
| **Agua** | «3d6 de daño de agua» | `#5ab4ff` | agua | 60 | 9 |
| **Hielo** | «3d6 de daño de hielo» | `#bff0ff` | escarcha | 27 | 2 |
| **Rayo** | «3d6 de daño de rayo» | `#fff27a` | rayos | 29 | 3 |
| **Tierra** | «3d6 de daño de tierra» | `#c89a5a` | piedras | 4 | 0 |
| **Viento** | «3d6 de daño de viento» | `#d8f5e6` | viento | 3 | 0 |
| **Luz** | «3d6 de daño de luz» | `#fff0b4` | luz | 94 | 13 |
| **Tinieblas** | «3d6 de daño de tinieblas» | `#7a4ab4` | sombra | 103 | 12 |
| **Naturaleza** | «3d6 de daño de naturaleza» | `#8cd25a` | hojas | 40 | 1 |
| **Veneno** | «3d6 de daño de veneno» | `#a0e650` | veneno | 14 | 0 |
| **Sangre** | «3d6 de daño de sangre» | `#d22828` | sangre | 9 | 1 |
| **Arcano** | «3d6 de daño arcano» | `#c88cff` | arcano | 10 | 1 |
| **Vacío** | «3d6 de daño del vacío» | `#5a3c8c` | vacio | 8 | 0 |

Sin elemento (físicas, apoyo, utilidad): 460 cartas. Alias que el catálogo ya traía: Sagrado = Luz, Oscuro = Tinieblas, Vacío = Vacio.

Reglas:

- El elemento de una carta es **de lo que está hecha**, lo que dice su texto («3d6 de daño de
  rayo»). Una carta física, de apoyo o de utilidad no lleva elemento: sus partículas son las de su
  clase.
- **El Rayo deja de ser Luz** (como el Tronador en NosTale) y **el Hielo deja de ser Agua**. Se
  reetiquetaron 8 clases y 100 cartas de las SP y de las propias: el Tempestario, el Tronador y Thor
  son de Rayo; el Mago azul y el Caballero del Rey Lobo, de Hielo; la Cartomante, Arcana; el Druida
  del Roble, de Naturaleza; el Corsario Vampírico, de Sangre; y cartas sueltas de otras (el
  Terremoto del Guerrero es de Tierra; las bombas de gas, de Veneno; el Escudo maná, Arcano...).
- Hoy el elemento decide el **color y las partículas**. Las resistencias por elemento llegarán con
  el combate que las use (aún no las hay).
- Las 63 clases de antes no se han tocado: ver la pregunta 3 de §7.

## 2. Las familias de partículas

| Familia | Elemento | Forma | Movimiento | Colores | Qué es | Clases (en cuántas es la primera) |
|---|---|---|---|---|---|---|
| **llamas** | Fuego | llama | sube | `#fff2a8` `#ffb040` `#ff6a2a` | lenguas de fuego que suben y se apagan, del amarillo al naranja | 15 (7) |
| **agua** | Agua | gota | cae | el de la pieza | gotas que caen | 11 (10) |
| **escarcha** | Hielo | copo | cae | `#ffffff` `#bff0ff` `#8ad8ff` | copos y cristales de escarcha que caen meciéndose | 7 (2) |
| **rayos** | Rayo | rayo | salta un instante | el de la pieza | un rayo en zigzag que salta un instante | 9 (6) |
| **piedras** | Tierra | piedra | cae (tapa) | `#c89a5a` `#8a6a40` `#e6c690` | esquirlas de piedra y tierra que saltan y caen | 5 (3) |
| **viento** | Viento | estela | deriva de lado | `#f4fff8` `#d8f5e6` | estelas de aire que cruzan de lado | 7 (4) |
| **luz** | Luz | punto | sube | el de la pieza | motas de luz que suben | 20 (16) |
| **sombra** | Tinieblas | voluta | sube (tapa) | el de la pieza | volutas de sombra que suben y tapan | 16 (10) |
| **hojas** | Naturaleza | hoja | cae (tapa) | el de la pieza | hojas que caen meciéndose | 6 (2) |
| **veneno** | Veneno | calavera, ×0.6 | sube (tapa) | `#b4f064` `#8cd23c` `#64aa28` | calaveras verdes que suben meciéndose | 6 (2) |
| **sangre** | Sangre | gota | cae (tapa) | `#d22828` `#8a1018` | gotas de sangre que caen | 7 (2) |
| **arcano** | Arcano | runa | gira alrededor | `#e6b4ff` `#b48cff` `#ffb4e6` | runas que giran alrededor y se desvanecen | 4 (2) |
| **vacío** | Vacío | cruz | cae hacia dentro (tapa) | `#3c1e5a` `#5a3c8c` `#8a5ac8` | motas oscuras que caen hacia dentro, como si la pieza se las tragara | 3 (3) |
| **brasas** | Fuego | punto | sube | el de la pieza | brasas de fragua que suben | 3 (1) |
| **ceniza** | Fuego | punto | sube (tapa) | el de la pieza | ceniza que flota | 2 (0) |
| **chispas** | — | trazo | salpica y cae | el de la pieza | chispas: trazos que saltan hacia arriba y a los lados y caen, como en la fragua | 6 (6) |
| **estrellas** | Arcano | estrella de cuatro puntas | sube | el de la pieza | estrellas de cuatro puntas que titilan | 3 (3) |
| **llamas puras** | Fuego | llama | sube | `#ffffff` `#fff7c8` `#ffe070` | fuego puro: llamas blancas y amarillas, sin humo (el Druida flamígero) | 1 (1) |
| **dragones** | Fuego | dragón | sube | `#ffd25a` `#ff7a3a` `#e8483a` | dragones de fuego diminutos que suben serpenteando (el Puño del dragón) | 1 (1) |
| **pétalos** | Naturaleza | pétalo | cae | `#ffd2e6` `#ffffff` `#ffa0c8` | pétalos que caen girando (el loto de las Artes Místicas, las flores de Merlín) | 2 (1) |
| **furia** | — | estela vertical | sube | `#ff5a3a` `#ff9a3a` `#c8281e` | el aura de la furia: llamaradas rojas que suben por el cuerpo | 6 (4) |
| **oro** | Luz | estrella de cuatro puntas, ×2 | sube | `#fff6c8` `#ffd23c` `#f5a51e` | destellos de oro, el doble y más vivos (el Señor de las Armas) | 2 (1) |
| **notas** | — | nota | sube | el de la pieza | notas musicales que suben bailando | 6 (4) |
| **ondas** | — | punto | aro que se abre | el de la pieza | ondas de sonido que se abren (el tambor, el eco) | 2 (2) |
| **naipes** | — | naipe | deriva de lado (tapa) | `#ffffff` `#c83232` `#28283c` | naipes que revolotean | 1 (1) |
| **monedas** | — | moneda y estrella de cuatro puntas, gira | cae | `#ffe68a` `#e6b432` | monedas que caen girando, con cruces doradas brillando entre ellas | 2 (1) |
| **engranajes** | — | engranaje, gira | cae (tapa) | `#c8c8c8` `#b48c3c` | engranajes que caen girando sobre sí mismos | 3 (2) |
| **humo** | — | voluta | sube (tapa) | `#e6e6e6` `#a0a0a8` | humo, vapor y niebla | 5 (5) |
| **pólvora** | Fuego | punto | sube | `#ffe68a` `#ff9a3a` | chispas de sal de fuego y pólvora | 6 (5) |
| **alquimia** | — | burbuja | sube | `#78f0a0` `#ff7ab4` `#7ab4ff` `#ffe66e` | burbujas de colores (el Alquimista) | 1 (1) |
| **plumas** | — | pluma | cae (tapa) | `#ffffff` `#e6e0d2` | plumas que caen meciéndose | 3 (2) |
| **murciélagos** | Tinieblas | murciélago | deriva de lado (tapa) | `#2a1e32` `#4a2a46` | murciélagos diminutos que salen volando | 2 (2) |
| **ánimas** | Tinieblas | voluta | sube | `#c8ffc8` `#8ae6a0` | las ánimas: fuegos fatuos verdes y pálidos que suben | 6 (4) |
| **espíritus** | — | voluta | gira alrededor | `#b4e6ff` `#78c8ff` | espíritus azules que giran alrededor (lobos, osos, antepasados) | 6 (3) |
| **esporas** | Naturaleza | punto | deriva de lado | `#e6f0a0` `#c8dc78` | esporas que flotan despacio | 1 (1) |
| **ki** | — | estela vertical | sube | `#ffffff` `#c8f0ff` | el ki: trazos de luz que suben deprisa | 2 (1) |
| **grano** | Luz | punto | cae | `#fff0a0` `#e6c85a` | granos de trigo dorado | 1 (1) |
| **confeti** | — | naipe | cae (tapa) | `#ff5a5a` `#5ab4ff` `#ffe65a` `#78e678` | confeti de colores (el teatro, la feria) | 2 (2) |
| **polvo** | Tierra | voluta | deriva de lado (tapa) | `#c8aa82` `#a08664` | polvo y arena que levantan los pasos | 2 (2) |
| **elementos** | — | cruz | gira alrededor | `#ff8c3c` `#5ab4ff` `#c89a5a` `#d8f5e6` | los cuatro elementos que giran alrededor (el Chamán de los Elementos) | 1 (1) |

- **Formas** (29), en píxeles del sprite, como el resto del dibujo: punto, cruz, burbuja, voluta,
  hoja, rayo y gota (las de antes); llama, copo, piedra, estela, estela vertical, runa, nota, naipe,
  moneda, engranaje, pluma, murciélago, dragón y pétalo (PART-01); y, al revisarlo Oriol, la
  calavera, la estrella de cuatro puntas, el trazo de la chispa (en los cuatro sentidos en que vuela)
  y la otra cara del engranaje y de la moneda, para que giren. Las de tema llevan un contorno oscuro
  de 1 px, para que se lean encima del cuerpo.
- **Movimientos** (8): sube o cae meciéndose; deriva de lado (el viento, los naipes, los
  murciélagos); cae hacia dentro de la pieza (el Vacío); gira alrededor de ella (lo arcano, los
  espíritus, los elementos del chamán); un aro que se abre (las ondas del tambor); salta un instante
  (el rayo); salpica y cae (las chispas).
- **Al revisarlo Oriol** (07/10): el veneno son calaveras verdes («parece todo lo contrario,
  curaciones»); las chispas, trazos («un trazo como el típico, muy simple»); las estrellas y el oro,
  la estrella de cuatro puntas, y el oro el doble y más vivo («algo más intenso»); las monedas, con
  cruces doradas brillando entre ellas; los engranajes y las monedas giran sobre sí mismos.
- **Suma o tapa**: la luz se suma (fuego, rayo, luz, notas...); lo oscuro o lo que tiene cuerpo tapa
  (sombra, sangre, hojas, piedras, plumas, naipes, murciélagos...).
- **Sin colores propios** (agua, luz, sombra, hojas, veneno, rayos, notas...): el color de la pieza,
  el de su esencia o su deidad (un veneno verde de plaga, una luz dorada de la Espiga).

## 3. Las partículas de cada clase

La primera familia es **la suya**: la de su set, la de sus abalorios y la de sus cartas que no digan
otra cosa. Las otras son las que echan algunas de sus cartas. Qué familia echa cada carta
(`particulas.de_la_carta`), en este orden:

1. **la que nombra su texto** de entre las de su clase, si no es de otro elemento que la carta (la
   Canción de Invierno del bardo de las estaciones, escarcha; el halcón de la Espía, plumas);
2. **la variante de su clase** para su elemento (el fuego del Druida flamígero son llamas puras; el
   del Puño del dragón, dragones; el del Paladín del Juramento, brasas de fragua);
3. si es **del elemento de su clase**, la de su clase (las cartas de Agua de las Artes Místicas, que
   en NosTale son de Agua, echan pétalos del loto; las de Tinieblas del Apóstata, sangre);
4. si no, **la de su elemento**; y sin elemento, la de su clase.

«Cartas por elemento»: cuántas de sus cartas son de cada elemento (— = sin elemento). «Partículas
(cartas)»: cuántas cartas echan cada familia («set» = solo su set y sus abalorios). Los ejemplos de
Oriol: el Apóstata, **sangre** roja (y sombra); el bárbaro (Furia
Salvaje), **furia**, el aura que sube por el cuerpo (y hojas, por chamánico); el Señor de las Armas,
**oro**; el Puño del dragón, **dragones** diminutos que suben serpenteando; las Artes Místicas,
**pétalos** del loto (no las mismas que el Puño del dragón); el Druida flamígero, **llamas puras**
(blancas y amarillas, sin humo) y no las llamas naranjas del Mago de fuego.

### 3.1 Las 63 de antes (63)

| Clase | Elemento | Cartas por elemento | Partículas (cartas) | De qué sale |
|---|---|---|---|---|
| Acechador de Tejados | — | — 4 | **sombra** 4 | sigilo urbano: se esconde en la sombra |
| Ancla del Vacío | — | Vacío 8 | **vacío** 8 | sus cartas son del Vacío |
| Arcanista | — | Arcano 1, — 1 | **arcano** 2 | control y utilidad arcana |
| Astrónomo Errante | — | — 4 | **estrellas** 4 | presagios de las estrellas |
| Bardo de la Cuerda Rota | — | Tinieblas 4 | **notas** 1 · sombra 3 | cánticos corruptos: notas en la sombra |
| Bardo de la Voz Divina | — | Luz 4 | **notas** 1 · luz 3 | cánticos sagrados |
| Bardo de las Cuatro Estaciones | — | — 10, Naturaleza 4, Hielo 1, Fuego 1 | **notas** 13 · hojas 2 · escarcha 1 | canciones de estación |
| Caballero Templario | — | Luz 5 | **luz** 5 | sus cartas son sagradas |
| Cambiaformas | — | Naturaleza 4, — 1 | **espíritus** 5 | magia de transformación animal |
| Cantor de la Tierra | — | Naturaleza 4 | **piedras** 2 · notas 2 | canta muros, temblores y piedra |
| Carcelero de Hierro | — | — 4 | **chispas** 4 | cadenas de hierro |
| Cazarrecompensas de Bastrea | — | — 4 | **chispas** 4 | redes y herramientas de captura |
| Centinela del Alba | — | — 4 | **luz** 4 | el alba |
| Cocinero de Campaña | — | — 4 | **humo** 4 | el vapor de la cocina |
| Coloso de Ceniza | — | Fuego 4 | **brasas** 3 · ceniza 1 | un incendio sagrado que no se apaga |
| Cronista de Batallas | — | — 4 | **plumas** 4 | la pluma del cronista |
| Danzarín del Filo | — | — 4 | **viento** 4 | baila y corta |
| Devoto del Bosque | — | Naturaleza 5 | **hojas** 5 | paladín de la naturaleza |
| El Apóstata | — | — 10, Tinieblas 4 | **sangre** 14 | sacrifica su sangre por el poder oscuro (Oriol: sangre roja; también sus cartas de Tinieblas) |
| Forjacantos | — | — 3, Fuego 1 | **chispas** 3 · notas 1 | la fragua y los cantos |
| Francotirador | — | — 5 | **pólvora** 5 | cañones arcanotecnológicos |
| Furia Salvaje | — | Naturaleza 4 | **furia** 3 · hojas 1 | el bárbaro chamánico: furia y aura (Oriol) |
| Furia de los Creadores | — | — 10, Luz 4 | **furia** 12 · oro 2 | la furia bárbara con la devoción: furia dorada |
| Guardaespaldas | — | — 4 | **chispas** 4 | el escudo del escolta |
| Guardián de Hierro | — | — 2 | **chispas** 2 | el escudo de hierro |
| Guardián de la Presa | — | — 2, Hielo 2 | **agua** 4 | embalsa el agua y la suelta en descargas |
| Guerrero del Puño Cortante | — | — 4 | **furia** 4 | furia y golpes seguidos |
| Hermano de Sangre | — | Luz 2, Sangre 2 | **sangre** 3 · furia 1 | furia divina de sangre |
| Hoja Venenosa | — | Veneno 3, Naturaleza 2, — 1 | **veneno** 4 · hojas 2 | venenos y trampas naturales |
| Hoja del Monzón | — | — 4 | **agua** 4 | esgrimista del agua |
| Inventor | — | — 4 | **engranajes** 4 | artilugios de magia y tecnología |
| Jinete de Tormentas | — | — 4 | **rayos** 4 | corcel de tormenta |
| Ladrón de Rostros | — | — 4 | **sombra** 4 | yokai que roba rostros |
| Lancero del Vendaval | — | — 4 | **viento** 4 | el asta del Aire |
| Maestro de Ceremonias | — | Luz 2, — 2 | **luz** 4 | burocracia divina y ceremonia |
| Mago Chasqueador | — | — 10 | **arcano** 10 | magia de reflejos y suerte |
| Mago del Paso Ligero | — | Tinieblas 4 | **sombra** 4 | sigilo mágico |
| Monje del Despertar | — | — 10 | **ki** 10 | acumula ki |
| Muralla de Udrax | — | — 4 | **piedras** 4 | acumula Piedra |
| Médium de la Tomba | — | Tinieblas 4 | **ánimas** 4 | portavoz de los muertos |
| Nacido del Trueno | — | — 4 | **rayos** 4 | el poder del trueno |
| Peregrino de los Doce Meses | — | — 2, Luz 2 | **luz** 4 | las bendiciones de los meses |
| Portador de Plagas | — | Veneno 4 | **veneno** 4 | insectos y plagas |
| Portador del Farol | — | Luz 2, Fuego 1, — 1 | **luz** 3 · brasas 1 | su farol |
| Portaestandarte | — | — 4 | **luz** 4 | el aura de la bandera |
| Quebraguardia | — | — 5 | **llamas** 5 | sus técnicas de fuego |
| Quemador de Reliquias | — | Fuego 4 | **llamas** 4 | piromante: quema reliquias |
| Rastreador de Ecos | — | — 4 | **ondas** 4 | caza por el sonido |
| Redimido | — | Luz 4 | **luz** 4 | brujo que volvió a la luz |
| Renacido | — | Tinieblas 4 | **sombra** 2 · furia 2 | pacto oscuro y furia |
| Rompemareas | — | — 4 | **agua** 4 | los puertos del sur |
| Sable Errante | — | — 3 | **viento** 3 | el filo sin magia |
| Saltador de Planos | — | Tinieblas 4 | **vacío** 4 | atraviesa los planos |
| Sanadora de Esporas | — | Naturaleza 4 | **esporas** 4 | hongos simbiontes |
| Señor de las Armas | — | — 13 | **oro** 13 | armas místicas y reliquias: dorado (Oriol) |
| Señor de los Astros | — | — 6 | **estrellas** 6 | el sol, la luna y las estrellas |
| Sombra de la Duna | — | — 4 | **polvo** 4 | el desierto: arena y emboscada |
| Sombra del Camino | — | — 4 | **sombra** 4 | sigilo |
| Tejedora de Mareas | — | — 15, Agua 1 | **agua** 16 | mueve las mareas |
| Titiritero Maldito | — | Tinieblas 4 | **sombra** 4 | hilos oscuros |
| Usurero de Almas | — | Tinieblas 4 | **ánimas** 1 · monedas 3 | presta con almas |
| Verdugo del Silencio | — | — 4 | **sombra** 4 | el miedo del condenado |
| Voz del Alba | — | — 2 | **luz** 2 | curación divina |

### 3.2 Las SP del Espadachín (12)

| Clase | Elemento | Cartas por elemento | Partículas (cartas) | De qué sale |
|---|---|---|---|---|
| Guerrero | Fuego | Fuego 5, — 4, Tierra 1 | **llamas** 9 · piedras 1 | golpes de fuego y el terremoto |
| Ninja | Agua | Agua 7, — 3 | **agua** 10 | cuchillas de agua |
| Cruzado | Luz | — 6, Luz 4 | **luz** 10 | flechas y cruces santas |
| Berserker | Tinieblas | Tinieblas 7, — 3 | **furia** 10 | la furia del berserker (también sus cartas de Tinieblas) |
| Gladiador | Fuego | Fuego 8, — 2 | **llamas** 10 | golpes de fuego |
| Monje de lucha | Agua | Agua 8, — 2 | **agua** 10 | la lanza del agua |
| Cosechador de almas | Tinieblas | Tinieblas 5, — 4, Sangre 1 | **ánimas** 4 · sombra 5 · sangre 1 | cosecha almas |
| Renegado | Luz | Luz 8, — 2 | **luz** 10 | la luz que se corrompe |
| Hachero de la cascada | Agua | Agua 7, — 3 | **agua** 10 | la cascada |
| Caballero Draconiano | Luz | Luz 8, — 2 | **luz** 10 | la luz del dragonio |
| Romperrocas | Fuego | Fuego 7, — 3 | **piedras** 3 · llamas 7 | roca, minería y cohetes |
| Aquiles | Tinieblas | Tinieblas 7, — 3 | **humo** 10 | la niebla del Estigia (también sus cartas de Tinieblas) |

### 3.3 Las SP del Arquero (12)

| Clase | Elemento | Cartas por elemento | Partículas (cartas) | De qué sale |
|---|---|---|---|---|
| Cazador | Agua | — 3, Agua 3, Rayo 3, Hielo 1 | **agua** 6 · rayos 3 · escarcha 1 | flechas de agua, relámpagos y helada |
| Asesino | Tinieblas | — 5, Tinieblas 4, Sangre 1 | **sombra** 9 · sangre 1 | la sombra y la cuchillada sangrante |
| Destructor | Fuego | Fuego 7, — 2, Veneno 1 | **pólvora** 3 · llamas 6 · veneno 1 | bombas y gas tóxico |
| Guardabosques | Luz | — 5, Luz 4, Naturaleza 1 | **espíritus** 9 · hojas 1 | los espíritus del bosque |
| Servidor del cañón de fuego | Fuego | Fuego 9, — 1 | **pólvora** 1 · llamas 9 | el cañón de fuego |
| Espía | Agua | Agua 6, — 4 | **viento** 7 · plumas 3 | el halcón |
| Cazademonios | Tinieblas | Tinieblas 8, — 2 | **sombra** 9 · ánimas 1 | los espíritus de la venganza |
| Ángel vengador | Luz | Luz 8, Rayo 1, — 1 | **luz** 9 · rayos 1 | alas de ángel y espadas de luz |
| Cazasoles | Luz | Luz 8, — 2 | **luz** 4 · espíritus 6 | el lobo solar |
| Granadero | Fuego | Fuego 5, — 5 | **pólvora** 5 · llamas 5 | granadas |
| Cazanieblas | Tinieblas | Tinieblas 4, Veneno 4, — 2 | **humo** 6 · veneno 4 | gas y niebla |
| Almirante Yi | Agua | Agua 7, — 3 | **agua** 10 | el dragón de los mares |

### 3.4 Las SP del Mago (12)

| Clase | Elemento | Cartas por elemento | Partículas (cartas) | De qué sale |
|---|---|---|---|---|
| Mago de fuego | Fuego | Fuego 8, Arcano 1, — 1 | **llamas** 9 · arcano 1 | fuego |
| Mago sagrado | Luz | — 7, Luz 2, Arcano 1 | **luz** 9 · arcano 1 | luz santa |
| Mago azul | Hielo | Hielo 9, — 1 | **escarcha** 10 | hielo |
| Mago de las tinieblas | Tinieblas | Tinieblas 5, — 5 | **sombra** 7 · ánimas 3 | tinieblas y fantasmas |
| Volcano | Fuego | Fuego 8, — 2 | **llamas** 9 · ceniza 1 | lava y ceniza |
| Señor de las mareas | Agua | Agua 5, — 4, Rayo 1 | **agua** 9 · rayos 1 | el mar y la tormenta |
| Vidente | Tinieblas | Tinieblas 5, — 5 | **estrellas** 5 · sombra 5 | el destino y la maldición |
| Archimago | Luz | Luz 5, — 5 | **luz** 10 | luz sagrada y libros de encantamientos |
| Sacerdote vudú | Tinieblas | — 5, Tinieblas 4, Veneno 1 | **plumas** 5 · veneno 1 · sombra 4 | plumas, serpientes y almas |
| Grávitas | Agua | Agua 7, — 3 | **vacío** 10 | la gravedad: lo atrae todo hacia sí |
| Tormenta de fuego | Fuego | Fuego 5, — 5 | **llamas** 10 | fuego |
| Merlín | Luz | Luz 7, Naturaleza 2, — 1 | **luz** 8 · pétalos 2 | luz y flores |

### 3.5 Las SP del Artista Marcial (8)

| Clase | Elemento | Cartas por elemento | Partículas (cartas) | De qué sale |
|---|---|---|---|---|
| Puño del dragón | Fuego | Fuego 8, — 2 | **dragones** 10 | sprites de dragón (Oriol) |
| Artes Místicas | Agua | Agua 6, — 4 | **pétalos** 10 | el loto y la luna (no las mismas que el Puño del dragón) |
| Maestro Lobo | Luz | Luz 6, — 4 | **espíritus** 9 · ki 1 | el espíritu del lobo |
| Guerrero demoníaco | Tinieblas | Tinieblas 6, — 3, Sangre 1 | **murciélagos** 3 · sombra 6 · sangre 1 | murciélagos y tinieblas |
| Druida flamígero | Fuego | — 5, Fuego 5 | **llamas puras** 10 | fuego más puro (Oriol) |
| Puño Hidráulico | Tinieblas | Tinieblas 6, — 4 | **humo** 8 · engranajes 2 | vapor y mecánica |
| Tronador | Rayo | Rayo 8, — 2 | **rayos** 10 | electricidad |
| Thor | Rayo | Hielo 6, Rayo 3, — 1 | **rayos** 4 · escarcha 6 | el trueno y el hielo |

### 3.6 Las 20 propias (20)

| Clase | Elemento | Cartas por elemento | Partículas (cartas) | De qué sale |
|---|---|---|---|---|
| Bardo de las Dos Máscaras | — | — 9, Tinieblas 1 | **confeti** 10 | el teatro |
| Bardo del Cantar de Gesta | Luz | — 7, Luz 3 | **notas** 7 · luz 3 | el laúd y la gesta |
| Bardo del Tambor de Guerra | — | — 9, Tinieblas 1 | **ondas** 10 | el tambor |
| Bardo de la Feria | — | — 8, Fuego 2 | **confeti** 8 · llamas 2 | la feria y el tragafuegos |
| Clérigo de la Espiga Dorada | Luz | — 7, Luz 3 | **grano** 7 · luz 3 | la cosecha |
| Clérigo del Caminante Gris | Luz | — 7, Luz 3 | **humo** 7 · luz 3 | la niebla de los caminos |
| Paladín del Juramento | Fuego | — 7, Fuego 3 | **chispas** 7 · brasas 3 | la fragua |
| Caballero del Rey Lobo | Hielo | Hielo 6, — 4 | **escarcha** 5 · espíritus 5 | el invierno y la manada |
| Ingeniero de Asedio | — | — 7, Fuego 3 | **engranajes** 7 · pólvora 3 | torretas y minas |
| Mercader Errante | — | — 10 | **monedas** 10 | el oro |
| Alquimista | — | — 6, Fuego 2, Veneno 1, Hielo 1 | **alquimia** 6 · llamas 2 · veneno 1 · escarcha 1 | frascos de colores |
| Cartomante | Arcano | Arcano 7, — 2, Tinieblas 1 | **naipes** 9 · sombra 1 | las cartas del tarot |
| Druida del Roble | Naturaleza | Naturaleza 10 | **hojas** 10 | el roble |
| Domador de Bestias | — | — 10 | **polvo** 10 | la estampida |
| Chamán de los Elementos | — | Tierra 3, Agua 3, Fuego 2, Hielo 1, Viento 1 | **elementos** (set) · llamas 2 · agua 3 · piedras 3 · viento 1 · escarcha 1 | los cuatro tótems |
| Chamán de los Espíritus | — | Rayo 3, — 3, Fuego 2, Viento 1, Sangre 1 | **rayos** 4 · espíritus 2 · llamas 2 · viento 1 · sangre 1 | la tormenta y los espíritus |
| Tempestario | Rayo | Rayo 10 | **rayos** 10 | el rayo |
| Necromago de las Ánimas | Tinieblas | — 5, Tinieblas 5 | **ánimas** 10 | las almas en pena |
| Corsario Vampírico | Sangre | — 4, Sangre 3, Tinieblas 2, Viento 1 | **murciélagos** 6 · sangre 3 · viento 1 | murciélagos y sangre |
| Forajido de Vinclap | — | — 8, Fuego 2 | **pólvora** 10 | la pistola de sal de fuego |

## 4. Dónde se ven y cuánto

**El equipo puesto**, por lo fuerte que es (ESTETICA-01, «de +1 a +2 Tier 1, +3 a +4 Tier 2...»):

| Tier | Nivel | Armas | Ropa | Las partículas |
|---|---|---|---|---|
| 1 | 1-2 | destellos sueltos | destellos | — |
| 2 | 3-4 | + un halo fino que late | un borde por fuera y un barrido de luz | — (las dan los abalorios) |
| 3 | 5-6 | + **partículas** de su familia | + pocas partículas | un pelo más opacas (×1,08 y ×1,15) |
| 4 | 7-8 | halo más ancho y más partículas | más | más intensas (×1,35 y ×1,45) |
| 5 | 9-10 | halo fuerte y **muchas partículas** (las de un +10 de NosTale) | más | aún más (×1,7 y ×1,85), y un tercio de un tono más oscuro que tapa, de contraste |

Oriol: «las partículas +10 del NosTale son solo para tier 5». Y con la prueba del Cazademonios por tier:
«a tier 3 un poco más opacas, pero un pelo; en tier 4 más intensidad, y tier 5 más aún», y en el 5
«otras burbujas de un color más oscuro para darle contraste» (`EscalonDeBrillo::opacidad` y
`contraste`: con más opacidad salen más vivas y tardan más en apagarse). Los sets de las clases son de Tier 2
(nivel 3-4): llevan la familia de su clase, que hoy es el **color de su halo**; no echan partículas.
En el Tier 2, las partículas las dan **los abalorios**.

**Los abalorios** (anillos, collar, cinturón): no se ven puestos, pero echan partículas **a cualquier
nivel**, sin halo (`escalonDeAbalorio`: 0,8 + 0,5 × nivel por segundo), desde su sitio (§6).

**Las cartas**, al jugarse en combate (PART-02d): la familia que le toca a cada una por §3
(`particulas.de_la_carta`, exportada a `assets/textures/particulas_cartas.json` por
`tools/particulas_cartas.py`; una carta que no es de ninguna clase echa la de su elemento). Brotan del
cuerpo de quien la recibe (el enemigo al que va, o el héroe si es suya) durante 1,4 s, con el triple
de partículas que un abalorio de +8, y se apagan (`Game::destellosDeCarta`, que pinta
`Render::EfectosDeCartas`). De momento, las del héroe: los acompañantes y las invocaciones no tienen
sprite, y las habilidades de los monstruos no llevan elemento.

## 5. Las ranuras por tier

Oriol: «tier 1 que solo es casco, pechera, brazos, piernas, zapatos y armas, no tiene nada de anillos
ni todas esas cosas que se desbloquean en tier 2 (...) donde van collares, cinturón y anillos como
equipos añadidos».

| Tier | Ranuras |
|---|---|
| 1 | casco, pechera, **brazos**, piernas, zapatos, arma (y la otra mano: escudo, daga, segunda arma) |
| 2 | + **collar**, **cinturón** y **dos anillos** (uno en cada mano) |

En el motor (PART-02a) son **11 ranuras** (`RPG::EquipSlot`): las 7 de antes (el accesorio es ahora
el anillo de la mano principal) y detrás los brazos, el collar, el cinturón y el anillo de la otra
mano. Las reglas:

- `RPG::tierDeRanura` dice desde qué tier se abre cada una y `InventoryEngine` no deja ponerse nada
  en una cerrada («los anillos, el collar y el cinturón se abren en el Tier 2»). Una ficha sin tier
  (0) cuenta como de Tier 1, que es donde empieza la creación.
- Un anillo va a la mano principal y, si ya lleva uno y la otra está libre, a la otra.
- La ficha, el inventario y la página de equipo enseñan las cerradas («desde el Tier 2»).
- En el catálogo, el slot de las piezas: `Arms`, `Neck`, `Belt` y `Ring`. El Amuleto de Foco es
  un collar (antes iba en la cabeza) y el Anillo de Vitalidad, la Argolla del Toro y el Sello de la
  Cláusula Viva, anillos (antes, en la cabeza o en la mano). Las siete clases que empiezan con el
  amuleto lo llevan en la mochila hasta el Tier 2.
- El kit del cofre de El Vacío es de Tier 1, así que ya no lleva el amuleto. De las 19 clases que
  lo llevaban en la cabeza, 18 llevan ahora una pieza de cabeza de verdad (la Máscara del Duelista
  en 13, la Diadema de Lectora en 3, el Casco de Abordaje y la Corona de Ramas Secas) y la Tejedora
  de Mareas ninguna; seis, con una pieza menos. Todas con el mismo +2, +2, +1, +1.
- Falta: el dibujo de los brazales, los iconos propios de las ranuras nuevas (hoy, el del torso y el
  del accesorio) y que la partida guarde la ficha con su equipo.

## 6. Los abalorios y sus partículas

- Un abalorio trae **uno o dos efectos** (un hechizo, un elemento o su lore), y cada efecto, **su
  familia**. Oriol: «si un anillo aporta un hechizo de fuego y uno de hielo, salgan sprites azules y
  rojos de la mano del pj».
- **Salen de su sitio** (PART-02b): el anillo de la mano principal, del puño de la mano del arma; el
  de la otra mano, de la otra; el collar, del pecho (debajo del cuello); el cinturón, a lo ancho de la
  línea del cinturón; lo que se lleva en la mano sin verse (la caracola, el cuerno), de la mano. Para
  eso cada talla trae, por frame, dónde están las dos manos, el pecho y la cintura
  (`personajes.json`, «anclas», sacadas del mapa de partes por `tools/actores_anclas.py`), y el sitio
  sale de la ranura del catálogo y de dónde va puesto (`Vestuario::sitioDeAbalorio`). Las partículas
  van con la mano al andar.
- **Cuántas**: las de un abalorio de su nivel, repartidas entre sus efectos (con dos, la mitad cada
  uno, y los destellos una vez). En el catálogo, `otrasParticulas` y `otroColor` en su brillo; en
  `demo_brillo_equipo`, `mano@llamas,escarcha`.
- **Los del Tier 2** (PART-02c, `tools/abalorios_t2.py`): un anillo por elemento (13), cuatro anillos de
  dos efectos (el de fuego y escarcha de Oriol, la tormenta, el eclipse y la ciénaga) y un collar o un
  cinturón por cada familia de tema que es la primera de alguna clase (26), para que cada clase
  encuentre el suyo. Collar lo que sale de la voz, el alma o la fe (del pecho); cinturón lo del oficio y
  el cuerpo (de la cintura). Todos raros, de Tier 2, con +1 al atributo principal de las clases que los
  buscarán (los de dos efectos, +1 y +1) y el nivel de su regla (20 o 26). El elemento va en sus
  etiquetas: el efecto de juego (las resistencias) llegará con el combate que lo use. Aún no están en
  ninguna tienda ni botín: el Tier 2 todavía no se juega.

| Abalorio | Ranura | Partículas | Atributo | Para (la primera familia de...) |
|---|---|---|---|---|
| Anillo de Brasa Viva | anillo | llamas | +1 INT | Quebraguardia, Quemador de Reliquias, Guerrero, Gladiador y 3 más |
| Anillo de la Marea | anillo | agua | +1 DES | Guardián de la Presa, Hoja del Monzón, Rompemareas, Tejedora de Mareas y 6 más |
| Anillo de Escarcha | anillo | escarcha | +1 INT | Mago azul, Caballero del Rey Lobo |
| Anillo del Trueno | anillo | rayos | +1 DES | Jinete de Tormentas, Nacido del Trueno, Tronador, Thor y 2 más |
| Anillo de Piedra Viva | anillo | piedras | +1 CON | Cantor de la Tierra, Muralla de Udrax, Romperrocas |
| Anillo del Vendaval | anillo | viento | +1 DES | Sable Errante, Danzarín del Filo, Lancero del Vendaval, Espía |
| Anillo del Alba | anillo | luz | +1 CAR | Caballero Templario, Centinela del Alba, Voz del Alba, Maestro de Ceremonias y 12 más |
| Anillo de la Penumbra | anillo | sombra | +1 DES | Acechador de Tejados, Sombra del Camino, Ladrón de Rostros, Mago del Paso Ligero y 6 más |
| Anillo de Raíz | anillo | hojas | +1 INT | Devoto del Bosque, Druida del Roble |
| Anillo de la Víbora | anillo | veneno | +1 DES | Hoja Venenosa, Portador de Plagas |
| Anillo de Sangre | anillo | sangre | +1 CAR | El Apóstata, Hermano de Sangre |
| Anillo Arcano | anillo | arcano | +1 INT | Arcanista, Mago Chasqueador |
| Anillo del Vacío | anillo | vacio | +1 INT | Ancla del Vacío, Saltador de Planos, Grávitas |
| Anillo de Fuego y Escarcha | anillo | llamas + escarcha | +1 INT, +1 CON | dos efectos |
| Anillo de la Tormenta | anillo | rayos + agua | +1 DES, +1 CAR | dos efectos |
| Anillo del Eclipse | anillo | luz + sombra | +1 CAR, +1 DES | dos efectos |
| Anillo de la Ciénaga | anillo | veneno + hojas | +1 DES, +1 INT | dos efectos |
| Colgante del Diapasón | collar | notas | +1 CAR | Bardo de la Cuerda Rota, Bardo de la Voz Divina, Bardo de las Cuatro Estaciones, Bardo del Cantar de Gesta |
| Relicario de las Ánimas | collar | animas | +1 CAR | Médium de la Tomba, Usurero de Almas, Cosechador de almas, Necromago de las Ánimas |
| Collar del Firmamento | collar | estrellas | +1 INT | Astrónomo Errante, Señor de los Astros, Vidente |
| Collar de Colmillos del Espíritu | collar | espiritus | +1 DES | Cambiaformas, Guardabosques, Maestro Lobo |
| Collar de Plumas de Cuervo | collar | plumas | +1 INT | Cronista de Batallas, Sacerdote vudú |
| Colgante del Tambor | collar | ondas | +1 CAR | Rastreador de Ecos, Bardo del Tambor de Guerra |
| Gargantilla del Murciélago | collar | murcielagos | +1 DES | Guerrero demoníaco, Corsario Vampírico |
| Gorguera del Bufón | collar | confeti | +1 CAR | Bardo de las Dos Máscaras, Bardo de la Feria |
| Colgante de Esporas | collar | esporas | +1 INT | Sanadora de Esporas |
| Rosario del Despertar | collar | ki | +1 DES | Monje del Despertar |
| Colgante del Dragón | collar | dragones | +1 DES | Puño del dragón |
| Collar del Loto | collar | petalos | +1 DES | Artes Místicas |
| Colgante del Sol Blanco | collar | llamas_puras | +1 CON | Druida flamígero |
| Colgante de la Espiga | collar | grano | +1 CAR | Clérigo de la Espiga Dorada |
| Collar de los Elementos | collar | elementos | +1 CAR | Chamán de los Elementos |
| Collar de Dientes de la Furia | collar | furia | +1 CON | Furia de los Creadores, Furia Salvaje, Guerrero del Puño Cortante, Berserker |
| Cinturón de la Fragua | cinturón | chispas | +1 CON | Carcelero de Hierro, Cazarrecompensas de Bastrea, Guardián de Hierro, Forjacantos y 2 más |
| Cinturón de Saquitos de Humo | cinturón | humo | +1 CON | Cocinero de Campaña, Aquiles, Cazanieblas, Puño Hidráulico y 1 más |
| Bandolera de Pólvora | cinturón | polvora | +1 DES | Francotirador, Destructor, Servidor del cañón de fuego, Granadero y 1 más |
| Cinturón de Herramientas | cinturón | engranajes | +1 INT | Inventor, Ingeniero de Asedio |
| Faja del Desierto | cinturón | polvo | +1 DES | Sombra de la Duna, Domador de Bestias |
| Faja de Brasas | cinturón | brasas | +1 CON | Coloso de Ceniza |
| Cinturón del Mercader | cinturón | monedas | +1 CAR | Mercader Errante |
| Cinturón de Redomas | cinturón | alquimia | +1 INT | Alquimista |
| Fajín del Tahúr | cinturón | naipes | +1 INT | Cartomante |
| Cinturón del Campeón | cinturón | oro | +1 INT | Señor de las Armas |

## 7. Para revisar

1. ¿Te valen los **13 elementos**? ¿Falta alguno (el Sonido de los bardos, el Metal)? Hoy las notas
   y las ondas de los bardos son un tema, no un elemento.
2. ¿Las **partículas de cada clase** (§3)? Sobre todo la primera, que es la de su set y sus abalorios.
3. Las **63 clases de antes**: hay cartas cuyo texto es de rayo, tierra, viento, hielo o sangre y no
   llevan ese elemento (o llevan otro). No las he tocado; propuesta:
   - Rayo: el Nacido del Trueno (Golpe de Trueno, Cadena de Rayos, Paso del Trueno, Tambor de
     Tormenta), el Jinete de Tormentas (Carga Eléctrica, Pisada de Trueno, Galope entre Relámpagos,
     Invocar Corcel de Tormenta), el Ataque del Rayo de la Furia de los Creadores y el Rayo
     Electrizante del Mago Chasqueador.
   - Tierra: el Cantor de la Tierra (Costilla de Piedra, Muro Cantado, Temblor Menor, hoy de
     Naturaleza) y la Sombra de la Duna (Marea de Arena, Tumba de Arena).
   - Viento: el Lancero del Vendaval (Molinete de Viento, Ojo del Huracán).
   - Hielo: la Cancion del Invierno del Bardo de las Cuatro Estaciones (la otra, la Canción de
     Invierno, ya es de Hielo).
   - Sangre: la Ofrenda de Sangre del Apóstata (hoy de Tinieblas: su texto dice «daño oscuro»; sus
     partículas ya son de sangre).
4. **El tamaño**: las partículas son de un píxel del sprite, como el dibujo. ¿Quieres las de tema
   (dragones, notas, naipes, monedas) más grandes?
5. **Dos anillos** (uno en cada mano), un collar y un cinturón en el Tier 2: ¿así?
