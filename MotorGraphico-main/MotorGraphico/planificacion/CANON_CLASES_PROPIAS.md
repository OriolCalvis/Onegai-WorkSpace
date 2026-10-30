# Canon CP-01 — Clases propias (las que no salen de una SP)

Oriol (07/10/2026): «Primero me gustaría con estos ejemplos y referencias del nostale, crear 3
clases más de bardo (en plan un actor de teatro, un cantador, un músico instrumental, y un
feriante), me gustaría crear 2 clérigos, 2 paladines, alguna clase que utilice el rayo como
elemento principal, un ingeniero (que se dedique a crear torretas, trampas y utiliza su ingenio),
me gustaría 1 mercader, alquimista, 1 druida, 1 necromago que invoque almas en pena, domador de
bestias, gambito o un símil tarotista por ejemplo, un par de chamanes tipo los del wow, 2 piratas,
ya que el origen de la piratería fue de Vinclap, una clase de piratas sea vampírico y el otro
simplemente un forajido que usa pistola y espada, y una armadura de tier dos para cada uno».

Son **20 clases** (los cuatro bardos que nombra: actor, cantador, músico y feriante). Tienen la
**misma forma que las SP** de `CANON_SP_CLASES.md` (ficha, 7 cartas + 3 de Creador, pasiva y un set
entero de tier 2 que la clase no lleva) y las escribe el mismo importador
(`tools/importar_clases_sp.py`). Lo que cambia es **de dónde salen las cifras**: no hay ficha de
NosTale, así que los dados y las rondas se escriben tal cual, dentro del **presupuesto** (§3), que
está sacado de las 44 SP ya hechas. Todo lo demás (diccionario de estados, reacciones, invocaciones,
lo que no se simula) es lo del canon de las SP.

Datos: `tools/cp_<grupo>.py` (`cp_bardos.py`, `cp_fe.py`, `cp_ingenio.py`, `cp_naturaleza.py`,
`cp_noche_y_tormenta.py`); reglas en código: `tools/cp_reglas.py`.

## 1. Qué sale de cada clase

| Pieza | Cómo |
|---|---|
| La clase | `classes.json`, `rarity` uncommon, tier 1, `propia` con su familia (bardo, clérigo, paladín...), elemento, deidad y referencias |
| 10 cartas | 7 estándar (tier 1, común, descanso corto) + 3 de **Creador** (tier 2, rara, descanso largo) |
| La pasiva | Lo que la clase hace siempre (§5) |
| El set | Arma + cabeza, torso, piernas y pies: objetos raros de tier 2 (nivel 20, 550) con la etiqueta `Set del <nombre>`; la clase **no** lo lleva (empieza con el kit de su arquetipo y un arma normal de su tipo) |

Ids: la clase, el `slug` del nombre (`bardo_de_las_dos_mascaras`); las cartas, `<clase>_<slug>`.
Cada carta lleva en `fuente` «Propia · <clase> · <referencia>»: de dónde sale la idea (una SP de
NosTale que sirve de modelo, el folclore, los chamanes de WoW...), o nada si es solo nuestra.

## 2. Referencias

«Con estos ejemplos y referencias del nostale»: las 44 SP hechas son el modelo de cómo se escribe
una clase (`tools/sp_*.py`), y algunas SP comunes de la enciclopedia de Oriol dan ideas concretas:
el **Pirata** (*Disparar un mosquete* con herida de bala, *Ataque de tormenta*, *Esconder en barril de
ron*, *Fuego de cañón* del barco), el **Entrenador de mascotas** (*Latigazo*, *Domesticación*, invocar
monstruos de entrenamiento) para el Domador, el **Atuendo de boda** y el **Pijama** (*Tocar la
pandereta*) para los bardos. Entre las SP de clase: el *Mago sagrado* y el *Ángel vengador*
(clérigos), el *Cruzado* y el *Caballero Draconiano* (paladines), el *Tronador* y *Thor* (el rayo), el
*Granadero* y el *Servidor del cañón de fuego* (el ingeniero), el *Sacerdote vudú* (tótems y
espíritus), el *Cazasoles* (el compañero), el *Vidente* (el tarot), el *Guardabosques* y el *Druida
flamígero* (el druida), el *Cosechador de almas* (el necromago), el *Renegado* y el *Almirante Yi*
(los piratas).

## 3. El presupuesto (lo que puede llevar una carta)

Sacado de las 44 SP (cuántos dados llevan según a quién apuntan y lo que añaden):

| Carta estándar | Dados |
|---|---|
| Golpe sin nada más, a uno o a una zona (`single_enemy`, `zone_radius`) | `3d6` |
| Golpe sin nada más a todos (`all_enemies`) | `2d6` |
| Golpe con un efecto (un estado de 1-2 turnos, una condición, robar vida) | `2d6` (a todos, `1d6`-`2d6`) |
| Golpe que deja sin atacar (`incapacitado`) o con dos efectos | `1d6`-`2d6` |
| Control sin daño (magia, con salvación): sin atacar | 1 turno a uno |
| Control sin daño a todos (desventaja, asustado) | 1-2 turnos |
| Mejora propia (ventaja, +1 CA, +1 al arma) | 2-3 turnos; `combate` solo un +1 |
| Mejora de grupo (`all_allies`) | +1 o ventaja, 2-3 turnos |
| Cura | `cura:2d6` a uno, `cura:1d6` a todos |
| Invocación | 2-3 rondas, criatura de tier 1 (5 PV) |
| Reacción | `bloquea:ataque` (+ una condición al atacante o lo que gana, 2 turnos), `defiende_aliado`, `dados_extra:2` |

| Carta de Creador | |
|---|---|
| Golpe a uno o a una zona | `4d6` (+ un efecto) |
| Golpe a todos | `3d6`-`4d6` (+ un efecto) |
| Lo grande | `revive`, `turno_extra`, `invoca:combate`, grupo `bono_dano_arma:1d6` o `bono_ca:2` todo el combate, `limpia` + `cura:2d6` a todos |

- **Nunca** `4d6`, `revive` ni `turno_extra` en una estándar (`cp_reglas.revisa_presupuesto`).
- **Rondas**: 1-3, o `combate`. Lo que deja sin atacar, 1 turno (2 solo en Creador o sin daño a uno).
- **Estados** (el diccionario de `CANON_SP_CLASES.md` §5): `incapacitado`, `desventaja:ataque`,
  `desventaja:defensa`, `desventaja:todo`, `penal_ca:N`, `bono_dano_contra:1`, `critico_contra`,
  `dano_por_turno:1`, `absorbe_por_turno:1d6`, `provocado:origen`; a favor: `ventaja:ataque`,
  `ventaja:defensa`, `bono_ca:N`, `bono_dano_arma:1|1d6`, `bono_cura:N`, `refleja:fisico`.
- **Condiciones** del catálogo que el combate entiende: `sangrado` (1 de daño por turno), `marcado`
  (tus golpes le hacen +1), `asustado` y `envenenado` (ataca con desventaja), `caido` (ataca con
  desventaja y los golpes cuerpo a cuerpo contra él, con ventaja), `cegado` (ataca con desventaja y
  todo contra él, con ventaja), `paralizado`, `guardia_rota` (−2 a la CA), `inconsciente`. Las demás
  (`encantado`, `endeudado`, `agarrado`, `ralentizado`...) no hacen nada en combate: no se usan.
- **El texto dice sus cifras**: los dados («2d6 de daño de fuego»), «N turno(s)» y «hasta el final
  del combate» (lo comprueba el importador).

## 4. Con qué se tira y la salvación

Cada clase tira con su atributo principal (`tira` en sus datos; sin nada, DES): los bardos, los
clérigos, los paladines y los chamanes con **CAR** (la fe, la voz, los espíritus), los magos de
oficio (ingeniero, alquimista, cartomante, druida, tempestario, necromago) con **INT**, y los de
arma (piratas, domador) con **DES**. Una carta puede tirar con otro (`tira` en la carta: el
pregón del Feriante con CAR). El ataque básico (**Atacar**) tira DES para todos.

Salvación: solo las cartas de **control sin daño** que son magia (`defensa_mental` si es miedo,
encanto, maldición o sueño; `resistencia_fisica` si es del cuerpo: raíces, ácido, plomo). Si la
carta hace daño es un golpe y el estado entra con la tirada.

## 5. La pasiva

Una de tres formas:

- **Elegir** (`elige_una` + `si:<opción>:<rasgo>`, como las estaciones del Bardo): las máscaras del
  actor, los toques del tambor, el juramento del paladín, los árboles del druida, la bestia del
  domador, los elementos y las armas imbuidas de los chamanes, la *tria prima* del alquimista.
  Dos a cuatro opciones con un rasgo cada una.
- **Al azar** (`azar_una`, gancho nuevo, §8): la lectura del tarot de la Cartomante.
- **Siempre** (`siempre:<rasgo>`): un +1 (`bono_dano_arma:1`, `bono_ca:1`, `bono_cura:1`) o
  `ventaja:defensa`.

`recupera_carta` es el Desgarro del Apóstata: no se usa.

## 6. El arranque

Tres cartas estándar **jugables** (sin `pendiente`) que digan qué es la clase: una que pegue o
controle, una de su mecánica (la que trae al compañero o al tótem, la que monta la trampa) y una
de apoyo o defensa (`arranque` en los datos).

## 7. El set de tier 2 y el arma

- **El set**: el arma y las cuatro piezas, raras de tier 2 (`sp_reglas.pieza`), con el peso de la
  armadura de su rol (pesada el tanque, media el equilibrado y el apoyo, ligera el ágil y el
  lanzador). Bonos como en las SP: el arma, la cabeza, las piernas y los pies, +1 a los dos
  atributos de la clase; el torso, +1 al principal. El nombre del set es el corto que llevan sus
  piezas (`set_de`): los bardos, por su oficio (`Set del Comediante`, `Set del Juglar`, `Set del
  Tamborilero`, `Set del Feriante`); los de fe, por su dios o su voto (`Set de la Espiga Dorada`,
  `Set del Caminante Gris`, `Set del Juramento`, `Set del Rey Lobo`).
- **El arma normal** (la de tier 1 con que empieza): una del catálogo si la hay de su tipo (el
  sable, el látigo, el martillo de guerra, el espadón, la vara, el bastón ferrado, el hacha de una
  mano); si no, una nueva «de Viaje» (`arma_comun`): el estoque de atrezo, el laúd, el tambor, el
  bastón de feriante, la hoz, la llave de ingeniero, la bolsa de monedas, el matraz, los naipes, el
  farol de ánimas, el tótem y el sable y pistola (los dos a la vez, como los guanteletes).
- **Armas de fuego** (Oriol, 07/10): sin pólvora; la pistola del Forajido carga sal de fuego y balas
  rúnicas, como las del Arquero.
- **El dibujo** (como los de las SP): lo propio de cada set (casi todas las cabezas y los torsos, y
  las calzas, pantalones y botas que no hay en el catálogo) en `tools/actores_trajes_propias.py`, con
  paletas propias sin tipo (ningún tema las toca); lo que reutiliza del catálogo (pantalones,
  quijotes, botas, sandalias) lo tiñe el tema del set (`actores_estetica.TEMAS_POR_NOMBRE`, que tiñe
  también el arma), y `actores_estetica.PIEZA_DE_ROPA` liga cada pieza con su dibujo. Todo va al
  final de cada atlas (ninguna fila de antes se mueve). Lo fino (astas, cuernos, colmillos, trenzas,
  naipes, velas) va con los tonos a mano: una línea de 1 px con el sombreado automático sale toda de
  contorno.
- **Las armas nuevas** (`tools/actores_formas_propias.py`, cada una con su escalón raro, el de los
  sets): el laúd, la bolsa, el matraz, los naipes y el farol de ánimas, sin pose (a su tamaño,
  colgando o de pie en la mano: un farol no se inclina); la llave, la hoz, la vara de rayo y el bastón
  de paseo, con pose como la vara de aprendiz; el tótem como un bastón; el tambor (con la baqueta) y
  el sable y la pistola, en las dos manos (`PAREJA`, como los guanteletes). La luz verde de las
  ánimas (el farol y las velas del Necromago) es un material del catálogo (`luz_animas`): el farol es
  un arma y las armas se pintan sin los trajes.
- **Nombres que dicen lo que se ve**: si el dibujo no es lo que decía el nombre, cambia el nombre
  (el «Bonete del Mercader», la «Diadema del Cartomante», la «Cabeza de Oso del Domador») o la
  descripción (las máscaras en la copa, la estola verde de la Espiga, el bajo violeta del
  Necromago).

## 8. Ganchos nuevos (motor)

| Gancho | Dónde | Qué hace |
|---|---|---|
| `azar_una:<op>,<op>...` | pasiva | Como `elige_una`, pero la opción sale al azar al empezar el combate: una fila «Echar las cartas» (la Cartomante). |

## 9. Lo que no se simula

Lo del canon de las SP (§10): posiciones, velocidad, MP, enfriamientos, cargas y medidores. Una
carta que sin eso no hace nada es `pendiente:` y sale bloqueada; mejor no escribirla.
