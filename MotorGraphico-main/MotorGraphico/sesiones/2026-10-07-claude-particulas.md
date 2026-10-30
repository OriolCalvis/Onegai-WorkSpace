# Los elementos y las partículas de todas las clases · 07/10/2026

Sesión de Claude (Cowork). Sigue a `2026-10-07-claude-clases-propias.md`. Oriol:

> «Ahora sería trabajar partículas que reflejen todos los elementos creados, por ejemplo apóstata
> partículas de sangre de color rojo sangre, bárbaro furia y aura, el señor de las armas yo la haría
> dorada, etc. [...] no debería tener las mismas partículas en los puños un artes místicas que un puño
> de dragón [...] podríamos separar druida flamígero más puro fuego, puño dragón sprites de dragón, hay
> que analizar qué habilidades aporta cada clase y hacer una lista de las partículas requeridas para
> todas y con ello, podemos crear un anillo que dé esas partículas aumentando la equipación del tier 2
> donde van collares, cinturón y anillos [...] tier 1 que solo es casco, pechera, brazos, piernas,
> zapatos y armas».

Y del rayo, que en las clases propias era Luz (como el Tronador en NosTale): «pues hay que cambiarlo
porque no estamos limitados a los 4 elementos del nostale aquí».

Seguimiento: `planificacion/seguimiento/PART-01.json`. Reglas: `planificacion/CANON_PARTICULAS.md`.

## 1. Lo que se decidió

- **13 elementos** (`tools/elementos.py`): Fuego, Agua, Hielo, Rayo, Tierra, Viento, Luz, Tinieblas,
  Naturaleza, Veneno, Sangre, Arcano y Vacío. El elemento de una carta es **de lo que está hecha** (lo
  que dice su texto) y decide su color y sus partículas. Se reetiquetaron 8 clases y 100 cartas de las
  SP y de las propias (`part/retag.py` en el scratch de la sesión); las 63 de antes, no (propuesta en
  el canon, §7).
- **40 familias de partículas** (`tools/particulas.py` = la tabla del motor, hoy en `FamiliasDeParticulas.cpp`): las 13 de los
  elementos y 27 de los temas de las clases, con 22 formas nuevas (llama, copo, piedra, estela, runa,
  nota, naipe, moneda, engranaje, pluma, murciélago, dragón, pétalo, calavera, estrella, trazos...) y 6
  movimientos nuevos (deriva, cae hacia dentro, gira alrededor, aro que se abre, salta un instante,
  salpica).
- **Las de cada clase**: la primera es la suya (su set y sus abalorios); las otras, las que echan sus
  cartas. Qué familia echa cada carta lo decide `particulas.de_la_carta`: lo que nombra su texto de
  entre las de su clase, la variante de su clase para su elemento, la de su clase si es de su elemento,
  la de su elemento. Así el Apóstata es todo sangre, el Puño del dragón dragones, las Artes Místicas
  pétalos y el Druida flamígero llamas puras, como pidió Oriol.
- **Dónde se ven**: la escalera de ESTETICA-01 no cambia (partículas en armas y ropa desde el Tier 3;
  las de un +10, en el Tier 5). Los sets de las clases (Tier 2) llevan la familia de su clase: hoy, el
  color de su halo. En el Tier 2 las partículas las darán los abalorios (PART-02).
- **Contorno oscuro** de 1 px en las de tema (dragones, notas, naipes...), como el resto del dibujo:
  sin él, encima de un traje del mismo color no se leían.
- **Revisión de Oriol** (comentarios en la página): el veneno, calaveras verdes («parece todo lo
  contrario, curaciones»); las chispas, trazos que salpican y caen («un trazo como el típico, muy
  simple»); las estrellas y el oro, la estrella de cuatro puntas, y el oro el doble y más vivo («algo
  más intenso»); las monedas, con cruces doradas brillando entre ellas; los engranajes y las monedas
  giran sobre sí mismos. Formas nuevas (calavera, estrella, cuatro trazos, el engranaje girado y la
  moneda de canto), el movimiento «salpica» y, por familia, densidad, otra forma y giro.

- **Partículas por tier** (Oriol, viendo la prueba del Cazademonios en Tier 3, 4 y 5): «a tier 3 un poco
  más opacas, pero un pelo; en tier 4 más intensidad, y tier 5 más aún», y en el 5 «otras burbujas de un
  color más oscuro para darle contraste»: la opacidad de las partículas sube con el nivel (×1,08 a
  ×1,85) y desde el +9 un tercio sale de un tono más oscuro que tapa. Vale para todas las familias.

## 2. Lo que se hizo

- `tools/elementos.py`, `tools/particulas.py` (nuevos); `cp_reglas.py` usa los elementos del juego.
- Motor: `Game::tipoDeParticula` (40 familias con colores propios y movimiento),
  `Game::pixelesDeForma` (29 formas), `Game::contornoDeForma`; `destellosDeBrillo` mueve cada familia a
  su manera; `Render/AuraJugador` pinta el contorno debajo.
- `actores_estetica.brillo`: las piezas de los sets, la familia y el color de su clase; el vacío, su
  familia. `equipo_catalogo.json` regenerado (las piezas de los 64 sets, con su familia).
- `demo_brillo_equipo`: `pieza@familia[:rrggbb]`, `@familia` (abalorio), `DEMO_BRILLO_NIVEL`,
  `DEMO_BRILLO_FONDO`.
- Pruebas: `test_brillo_equipo` (nuevo caso: las 40 familias, su forma, sus colores, su contorno y cómo
  se mueve cada una) y `test_validar_sprites.ParticulasTests` (4).
- Página para Oriol: `particulas.html` (los elementos, las 40 familias animadas con el código del motor,
  un GIF del motor con los cuatro sets de sus ejemplos y las partículas de cada clase).

## 2b. La prueba del Cazademonios por tier (guardada)

Oriol pidió ver cómo subiría una armadura por tier con el Cazademonios (Tier 3 «de Plata», 4 «del Ocaso» y
5 «del Abismo»). Se le enseñó en el motor y en grande; de ahí salieron las partículas por tier (PART-01c).
«Guarda lo del cazademonios, ya seguiremos otro día con las armaduras de tiers superiores»: la prueba está
en `tools/actores_trajes_caza.py` (no la importa nadie) y `tools/ver_cazademonios_tiers.py`, y cómo verla
en el motor, en `planificacion/seguimiento/TIERS-01.json`.

## 2c. Las ranuras por tier (PART-02a)

«Retomo las ranuras». Seguimiento: `planificacion/seguimiento/PART-02.json`; reglas, §5 del canon.

- **11 ranuras** (`RPG::EquipSlot`): las 7 de antes con los mismos índices (el accesorio es ahora el
  anillo de la mano principal) y detrás los brazos, el collar, el cinturón y el anillo de la otra mano.
  `RPG::tierDeRanura` y `RPG::ranuraAbierta`: los brazos, desde el Tier 1; el collar, el cinturón y los
  anillos, desde el 2. `InventoryEngine` no deja ponerse nada en una cerrada, y el segundo anillo va a
  la otra mano.
- Lo enseñan la ficha, el inventario y la página de equipo (las cerradas, «desde el Tier 2»), la
  creación, el papiro del cofre y los mensajes de la partida.
- Catálogo: el Amuleto de Foco es un collar y el Anillo de Vitalidad, la Argolla del Toro y el Sello de
  la Cláusula Viva, anillos. Las siete clases que empiezan con el amuleto lo llevan en la mochila hasta
  el Tier 2.
- **Efecto en el kit del cofre** (es de Tier 1): 19 clases llevaban el amuleto en la cabeza. Ahora 13
  empiezan con la Máscara del Duelista, 3 con la Diadema de Lectora, una con el Casco de Abordaje, otra
  con la Corona de Ramas Secas y la Tejedora de Mareas sin nada en la cabeza; todas con el mismo +2, +2,
  +1, +1.
- Pruebas nuevas: las ranuras nuevas y su nombre, sin anillos ni collar ni cinturón en el Tier 1, dos
  anillos uno en cada mano, y las cerradas en la ficha.

## 2d. Los abalorios echan desde su sitio (PART-02b)

- **Anclas** de cada talla y frame (`tools/actores_anclas.py`, en `personajes.json`): el puño de la mano
  del arma (el mismo donde se coge el arma), el de la otra, el pecho bajo el cuello y la línea del
  cinturón, sacados del mapa de partes. De perfil y quieto la otra mano queda tapada por el cuerpo:
  entonces vale la de delante.
- **El motor** echa cada abalorio desde unos píxeles alrededor de su ancla en cada frame (y las
  partículas van con la mano al andar): el anillo de la mano principal, de la mano del arma; el de la
  otra, de la otra; el collar, del pecho; el cinturón, a lo ancho de la cintura; lo que se lleva en la
  mano sin verse (la caracola, el cuerno), de la mano. El sitio sale de la ranura del catálogo y de
  dónde va puesto. Sin anclas, del cuerpo entero como antes.
- **Dos efectos**: «si un anillo aporta un hechizo de fuego y uno de hielo, salgan sprites azules y
  rojos de la mano del pj». Un abalorio con `otrasParticulas` echa las dos familias a medias desde el
  mismo sitio. En el demo: `mano@llamas,escarcha`.
- GIF del motor andando y girando: el anillo de fuego y hielo en la mano del arma; uno de fuego en una
  mano y uno de hielo en la otra; un collar de notas y un cinturón de monedas.

## 2e. Los abalorios del Tier 2 (PART-02c)

«Podemos crear un anillo que dé esas partículas aumentando la equipación del tier 2»:
`tools/abalorios_t2.py` escribe 43 en el catálogo (con su objeto puente para el inventario y la tienda):

- **13 anillos de elemento** (el Anillo de Brasa Viva, de la Marea, de Escarcha, del Trueno...), con
  las partículas de su elemento.
- **4 anillos de dos efectos**: el de Fuego y Escarcha que pedía Oriol, el de la Tormenta (rayo y
  agua), el del Eclipse (luz y tinieblas) y el de la Ciénaga (veneno y naturaleza).
- **16 collares y 10 cinturones**, uno por cada familia de tema que es la primera de alguna clase: el
  Colgante del Diapasón de los bardos, el Colgante del Dragón del Puño del dragón, el Collar del Loto
  de las Artes Místicas, el Cinturón del Mercader, la Bandolera de Pólvora, el Fajín del Tahúr...
- Raros de Tier 2, +1 al atributo principal de las clases que los buscarán, el nivel de su regla. La
  tabla, en el §6 del canon. Aún no están en tiendas ni botines (el Tier 2 todavía no se juega).

## 2f. Las cartas al jugarse en el combate (PART-02d)

- Qué familia echa cada carta (la regla del §3 del canon) va al motor en
  `assets/textures/particulas_cartas.json` (`tools/particulas_cartas.py`); una carta sin clase echa la de
  su elemento.
- Al jugarse, la carta se apunta en la mesa (`CartasEnCombate::cartasJugadas`) y sus partículas brotan
  del cuerpo de quien la recibe durante 1,4 s: el enemigo al que va, o el héroe si es suya
  (`Render::EfectosDeCartas`, con la misma pintura que el brillo del equipo).
- De momento, solo las del héroe: los acompañantes y las invocaciones no tienen sprite, y las habilidades
  de los monstruos no llevan elemento.
- `BrilloEquipo.cpp` había pasado el techo de 500 líneas con PART-01b: las familias van ahora en
  `FamiliasDeParticulas` (PART-01d).
- GIF del motor: cartas de sangre (el Apóstata), dragones (el Puño del dragón), pétalos (las Artes
  Místicas) y llamas puras (el Druida flamígero).

## 3. Lo siguiente (PART-02)

1. ~~Las ranuras por tier~~ (hecho, PART-02a). Quedan el dibujo de los brazales, los iconos propios de
   las ranuras nuevas y que la partida guarde la ficha con su equipo (GUARDAR-FICHA, ver TUTORIAL-01).
2. ~~Los puntos de cada talla y frame y el abalorio desde su sitio~~ (hecho, PART-02b).
3. ~~Los abalorios de Tier 2~~ (hecho, PART-02c). Falta repartirlos en tiendas y botines cuando se
   juegue el Tier 2.
4. ~~Las partículas de las cartas al jugarse en el combate~~ (hecho, PART-02d; faltan las de los
   acompañantes, las invocaciones y los monstruos).

Después: limpiar la creación de personaje y depurar desde el inicio hasta la primera misión de
Boundington.

## 4. Para Oriol

1. ¿Te valen los 13 elementos? ¿Falta alguno (el Sonido de los bardos, el Metal)?
2. ¿Las partículas de cada clase? Sobre todo la primera, la de su set y sus abalorios.
3. Las 63 de antes tienen cartas de rayo, tierra, viento o sangre con otro elemento o ninguno: ¿las
   cambio al suyo?
4. ¿El tamaño (un píxel del sprite, con contorno en las de tema)?
5. ¿Dos anillos (uno en cada mano), un collar y un cinturón en el Tier 2?
6. El kit de 13 clases empieza ahora con la Máscara del Duelista (el +1 de CAR que les daba el amuleto):
   ¿te vale, o prefieres otra pieza de cabeza de Tier 1 con +1 de CAR (una capucha, un pañuelo)?
7. ¿Los 43 abalorios del Tier 2 (la tabla del §6 del canon)? Los nombres, el atributo de cada uno y si
   cada collar o cinturón va con su tema.
