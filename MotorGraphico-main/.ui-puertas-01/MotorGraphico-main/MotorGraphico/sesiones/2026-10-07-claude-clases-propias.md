# Veinte clases propias con su set de tier 2 · 07/10/2026

Sesión de Claude (Cowork). Sigue a `2026-10-07-claude-clases-sp.md`. Oriol:

> «Primero me gustaría con estos ejemplos y referencias del nostale, crear 3 clases más de bardo
> (en plan un actor de teatro, un cantador, un músico instrumental, y un feriante), me gustaría
> crear 2 clérigos, 2 paladines, alguna clase que utilice el rayo como elemento principal, un
> ingeniero (que se dedique a crear torretas, trampas y utiliza su ingenio), me gustaría 1
> mercader, alquimista, 1 druida, 1 necromago que invoque almas en pena, domador de bestias,
> gambito o un símil tarotista por ejemplo, un par de chamanes tipo los del wow, 2 piratas, ya que
> el origen de la piratería fue de Vinclap, una clase de piratas sea vampírico y el otro
> simplemente un forajido que usa pistola y espada, y una armadura de tier dos para cada uno?»

Y después: las partículas de todas las clases (y los anillos, collares y cinturones del tier 2), y
luego limpiar la creación de personaje y depurar desde el inicio hasta la primera misión de
Boundington. Esto es lo primero.

Seguimiento: `planificacion/seguimiento/CP-01.json`. Reglas: `planificacion/CANON_CLASES_PROPIAS.md`.

## 1. Lo que se decidió (sin preguntar: Oriol dejó el trabajo hecho)

- **Veinte clases**: los cuatro bardos que nombra (actor, cantador, músico y feriante, aunque dijo
  «3 más»), dos clérigos, dos paladines, la del rayo (el Tempestario), el ingeniero, el mercader, el
  alquimista, la tarotista (la Cartomante), el druida, el necromago, el domador, dos chamanes y los
  dos piratas.
- **La misma forma que las SP**: ficha por rol, 7 cartas + 3 de Creador, pasiva y un set entero de
  tier 2 que la clase no lleva; las escribe el mismo importador. Sin ficha de NosTale, las cifras
  van dentro de un **presupuesto** sacado de las 44 SP (`cp_reglas.revisa_presupuesto`: ninguna
  estándar con 4d6, revivir o turno extra).
- **Clérigos y paladines con su dios** del panteón: la Espiga Dorada y el Caminante Gris (clérigos),
  el Padre de la Forja (el Juramento) y el Rey Lobo.
- **Vinclap**: solo se nombra como el sitio «de donde salieron los primeros piratas».
- **La tarotista echa las cartas al azar**: su pasiva elige sola al empezar el combate (gancho nuevo
  `azar_una`, una fila «Echar las cartas» en la barra).
- **Elementos**: se escribieron con los cuatro de NosTale (Fuego, Agua, Luz y Tinieblas) y el rayo como
  **Luz**, como el Tronador. Oriol (07/10): «no estamos limitados a los 4 elementos del nostale aqui»:
  ahora son 13 y el Tempestario es de **Rayo** (PART-01, `planificacion/CANON_PARTICULAS.md`). Hoy el
  elemento es una etiqueta de la carta (el color y las partículas): el combate aún no tiene
  resistencias ni debilidades por elemento.

## 2. Las veinte

| Clase | Rol | Arma al empezar | Pasiva | Arranque | Creador (tier 2) |
|---|---|---|---|---|---|
| Bardo de las Dos Máscaras | apoyo, CAR/DES | Estoque de Atrezo de Viaje | Las dos máscaras | Estocada de atrezo, Monólogo, Morir en escena | Bis, Telón, Final feliz |
| Bardo del Cantar de Gesta | apoyo, CAR/CON | Laúd de Viaje | Voz de juglar | Copla burlona, Cantar de gesta, Balada de los caídos | Epopeya, El último cantar, Himno de las mil voces |
| Bardo del Tambor de Guerra | equilibrio, CAR/CON | Tambor de Guerra de Viaje | Toques de tambor | Golpe de bombo, Redoble, Contratiempo | Marcha triunfal, Tambor del juicio, Marcha fúnebre |
| Bardo de la Feria | ágil, DES/CAR | Bastón de Feriante de Viaje | El acróbata | Lanzar cuchillos, Tragafuegos, ¡Pasen y vean! | El gran espectáculo, El hombre bala, La noria |
| Clérigo de la Espiga Dorada | apoyo, CAR/CON | Hoz de Viaje | Paciencia del sembrador | Siega dorada, Hogaza compartida, Gavillas en pie | Año de abundancia, Cosecha final, El grano que renace |
| Clérigo del Caminante Gris | apoyo, CAR/INT | Bastón Ferrado de Viaje | Caminante prevenido | Barrido de bordón, La raya gris, Fuente del cruce | Atajo de niebla, La frontera cerrada, El último viaje |
| Paladín del Juramento | tanque, CAR/CON | Martillo de Guerra de Viaje | Voto del yunque | Martinete, Desafío jurado, Firme como el yunque | La obra maestra, Juicio del martillo, Clamor del yunque |
| Caballero del Rey Lobo | equilibrio, CAR/CON | Espadón de Recluta de Viaje | Pelaje de escarcha | Colmillo de escarcha, Caza en manada, Gruñido | El gran invierno, La manada entera, Volver del invierno |
| Ingeniero de Asedio | equilibrio, INT/DES | Llave de Ingeniero de Viaje | Llave bien apretada | Llavazo, Torreta de ballesta, Cepo | Torreta pesada, Campo de minas, Ariete de asedio |
| Mercader Errante | apoyo, CAR/INT | Bolsa de Monedas de Viaje | Cota bajo la casaca | Soborno, Contratar escolta, Poción de la tienda | Seguro de vida, La caravana, Liquidación total |
| Alquimista | lanzador, INT/CON | Matraz de Viaje | Tria prima | Bomba incendiaria, Transmutar en plomo, Niebla balsámica | La piedra filosofal, La panacea, La gran obra |
| Cartomante | lanzador, INT/CAR | Naipes de Viaje | La lectura (al azar) | Naipe cortante, La Rueda de la Fortuna, As en la manga | La Muerte, El Mundo, Escalera real |
| Druida del Roble | apoyo, INT/CON | Vara de Aprendiz de Viaje | Árbol tutelar | Lluvia de espinas, Raíces enredadoras, Lluvia que sana | Despertar del roble, Ira de la tierra, El bosque renace |
| Domador de Bestias | equilibrio, DES/CAR | Látigo de Cuero de Viaje | Bestia de confianza | Llamar a la bestia, ¡Muerde!, ¡Ruge! | Estampida de la manada, El vínculo salvaje, El rey de la manada |
| Chamán de los Elementos | apoyo, CAR/CON | Tótem de Viaje | Llamada de los elementos | Choque de escarcha, Tótem abrasador, Ola de sanación | Tótem de la marea, Estallido de lava, Seísmo |
| Chamán de los Espíritus | equilibrio, DES/CAR | Hacha de Una Mano de Viaje | Arma imbuida | Golpe de tormenta, Lobo espiritual, Escudo de relámpagos | Sed de sangre, Ascenso elemental, Tormenta de los antepasados |
| Tempestario | lanzador, INT/CAR | Vara de Aprendiz de Viaje | La vara cargada | Saeta de rayo, Carga estática, Pararrayos | Tormenta perfecta, Juicio del relámpago, Ojo de la tempestad |
| Necromago de las Ánimas | lanzador, INT/CAR | Farol de Ánimas de Viaje | Cortejo de ánimas | Llamar a un alma en pena, Escalofrío, Velo de ánimas | La Santa Compaña, El último aliento, Camposanto |
| Corsario Vampírico | ágil, DES/CON | Sable Ligero de Viaje | Hoja sedienta | Sable sangriento, Mordisco, Forma de niebla | Banquete de sangre, El barco fantasma, Señor de la noche |
| Forajido de Vinclap | equilibrio, DES/CON | Sable y Pistola de Viaje | Sable y pistola | Disparo a quemarropa, Estocada y disparo, Barril de ron | ¡Fuego por babor!, Duelo al amanecer, ¡Todos a cubierta! |

Invocaciones nuevas (las del Ingeniero, el Mercader, el Druida, el Domador, los chamanes y el
Necromago): torretas, la escolta a sueldo, el roble despierto, la bestia domada, el tótem abrasador,
el lobo espiritual, el alma en pena y la Santa Compaña. El catálogo queda en 127 clases, 1191
cartas, 136 pasivas, 755 objetos y 217 invocaciones.

## 3. Los sets

Cada clase, un set raro de tier 2 (arma, cabeza, torso, piernas y pies) que cualquiera puede
ponerse. Dibujos en `tools/actores_trajes_propias.py` (paletas propias; lo del catálogo lo tiñe el
tema del set) y armas nuevas en `tools/actores_formas_propias.py`: el laúd, el tambor con la
baqueta, el bastón de paseo, la hoz, la llave inglesa, la bolsa de monedas, el matraz, los naipes,
el tótem, la vara de rayo, el farol de ánimas y el sable y la pistola. La tabla de lo que lleva cada
uno está en `assets/textures/personajes.md` («Los veinte sets de las clases propias»).

Lo que se corrigió mirando las láminas (de frente, de perfil y de espaldas, en las cuatro tallas y
en el motor):

- Lo fino con los tonos a mano (una línea de 1 px sombreada sale toda de contorno, negra): las
  astas del Druida, los cuernos de carnero, los colmillos, las trenzas del Chamán de los Espíritus,
  los naipes de la Cartomante, las velas y los pararrayos.
- Las cintas y las bandas, a la altura de la frente y del ancho de la cabeza (flotaban encima del
  pelo).
- El gorro del Domador es ahora **la cabeza de un oso** (orejas, ojos, hocico y colmillos): un gorro
  con orejas sin cara se leía como unas orejas de gato.
- Los cuernos del Chamán de los Elementos, enroscados junto a las sienes con sus estrías (los de
  antes, finos y oscuros, parecían unos auriculares).
- La túnica del Necromago, negra con las mangas y el bajo violeta (con la estola y la cadena
  cruzada no se leía como túnica); su farol cuelga de una vara **sin pose**: un farol no se inclina.
- La vara del Tempestario, corta y con la chispa maciza (con el rayo en zigzag parecía una
  antorcha).
- El Forajido con botas cortas, para que se vean las rayas del pantalón.
- El bastón del Feriante es un **bastón de paseo** nuevo: con el cetro, el de Viaje se veía igual que
  el Cetro de Bronce de Viaje (cada objeto se ve distinto, `estetica_todos_los_objetos_se_montan`).

Nombres que dicen lo que se ve: «Bonete del Mercader», «Diadema del Cartomante» y «Cabeza de Oso
del Domador» (eran sombreros), y las descripciones de lo demás.

## 4. Motor

- `azar_una:<op>,...` (`GanchosCarta`): una pasiva cuya opción sale al azar al empezar el combate
  (`CartasEnCombate::echarSuerte`; en la barra, la fila «Echar las cartas»).
- Nada más: lo demás son datos y dibujos.

## 5. Pruebas

- `test_clases_propias` (26): las 20 clases se eligen al crear, sus cartas y pasivas, los sets
  enteros (raros, tier 2, de nadie al empezar), el azar de la Cartomante, las torretas del Ingeniero,
  el seguro de vida del Mercader...
- `test_validar_sprites`: `test_las_armas_de_las_clases_propias` (cada arma con su forma y su
  escalón raro, cómo se coge, y que pasa por el puño en todas las tallas y frames) y las parejas
  (el tambor y el sable y pistola, en las dos manos).
- `test_catalog_content`: 217 invocaciones.
- Lo que falla igual que antes (línea base): demo_creacion_personaje, test_gdb12_screens, test_g14
  (no enlaza), test_application y test_application_cableado (segfault), test_catalog_content (skills
  492 y NPC 134), test_application_scenes, test_personajes_capas (pnj_boundington/aigren),
  linea_base_core y, en Python, `test_personajes_son_reproducibles` (las filas de PixelLab no las
  genera `generar()`; falla igual en `ecfee91`).

## 6. Para el móvil

`clases_propias/`: `sets_cp_1..5.gif` (los veinte sets en el motor, andando y girando).

## 7. Preguntas para Oriol

1. ¿Te gustan los veinte sets? Sin referencias tuyas; los corrijo al verlos.
2. ¿Te vale que la Cartomante eche las cartas al azar (y no elija)?
3. Hice los cuatro bardos que nombraste (no tres): ¿alguno sobra?
4. ~~¿El rayo como Luz o un quinto elemento?~~ Respondida: el Rayo es un elemento suyo (PART-01).
