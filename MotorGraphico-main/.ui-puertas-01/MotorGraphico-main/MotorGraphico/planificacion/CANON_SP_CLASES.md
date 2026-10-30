# Canon SP-02 — De las SP de NosTale a clases de MotorGraphico

Oriol (07/10/2026): «metete de lleno en las clases nuevas». Elegido: **cada SP, una clase
nueva** (ficha, cartas, pasiva, traje y arma; se elige al crear personaje como las otras),
**clase a clase y completa**, cartas **jugables donde se pueda** (lo que el combate no cubre
queda `[pendiente]` y bloqueado, como en H2-06) y empezando por el **Espadachín entero**
(12 SP). Antes: «adaptándolas conceptualmente a nuestro juego» y «solo hay que adaptarlas
a nuestro sistema».

Fuente: *NosTale — Enciclopedia de SP y habilidades* (Obsidian de Oriol, Onegai/NosTale,
consultada el 07/10/2026; 52 SP, 627 habilidades, 483 estados). El flujo es el de siempre
(`canon → cartas → motor`): este documento son las reglas de la adaptación y
`tools/importar_clases_sp.py` es la carta (upsert por id; se corrige un texto y se vuelve
a ejecutar). Cada carta lleva en `fuente` la habilidad de NosTale de la que sale (su H).

Las reglas buscan que la conversión sea **reproducible**: las mismas cifras de NosTale dan
siempre los mismos dados y las mismas rondas. Cuando una carta se aparta de la regla, el
importador lo dice en un comentario junto a la carta.

## 1. Qué sale de cada SP

| En NosTale | En MotorGraphico |
|---|---|
| La tarjeta de especialista | Una clase (`classes.json`), `rarity` uncommon, tier 1, `especialista` con su clase base, número de SP, elemento y nombre en NosTale |
| Sus habilidades (sin el ataque básico) | 10 cartas: 7 estándar (tier 1, común, descanso corto) + 3 de Creador (tier 2, rara, descanso largo) |
| El ataque básico | No es carta: es **Atacar** con el arma de la clase. Su cadena y su elemento dan la pasiva (§6) |
| La apariencia de la SP | Un **set entero** (arma, cabeza, torso, piernas y pies): objetos sueltos de `equipment.json` que cualquiera puede ponerse y combinar. La clase **no** lo lleva puesto (§8) |

Ids: la clase, el `slug` del nombre (`guerrero`, `caballero_draconiano`); las cartas,
`<clase>_<slug>` (`guerrero_batalla_triple`) para no chocar con las 2 000 que ya hay ni
entre SP (hay nombres repetidos: *Juicio de la luz*, *La muerte se acerca*).

## 2. Qué habilidades son cartas

1. El **ataque básico** (la primera de la tarjeta, recarga < 1 s) no es carta.
2. Las **continuaciones** (las «técnicas relacionadas o habilitadas» que solo salen tras otra:
   *Arpón de dragón* tras *Siete punzadas*) se funden con la técnica que las abre: una carta.
3. Las **variantes de otra forma** (las tres meditaciones del Monje) se funden: la carta es la
   misma y la forma pasa a la pasiva (`elige_una`, §6). Si la forma se activa con una
   habilidad propia, es carta: *Ignición* y *Reignición* del Renegado.
4. Si aun así quedan más de 10, se funden las que hacen lo mismo; si quedan menos, la que
   falta es `[pendiente]` (no se inventa).

## 3. Creador y arranque

- **Creador** = las tres que la SP enseña al final (niveles de profesión 12, 16 y 20 en la
  tarjeta; si la tarjeta no da nivel, las tres últimas). Son sus definitivas: tier 2, rara,
  descanso largo, y esperan al tier 2 del héroe como las de las otras clases.
- **Arranque** (`startingCards.skills`) = las tres primeras de la tarjeta, salvo que la SP diga
  otras (`arranque` en sus datos). Al crear, la ficha conoce igualmente las diez (H2-06). Las
  de arranque se tienen que poder jugar: si una de las tres primeras sale bloqueada (§10),
  entra la siguiente jugable (*Cazador*, *Destructor*, *Espía*). Una SP que vive de su
  compañero empieza con la carta que lo trae (*Cazasoles*: *Curación natural*, el Lobo solar).
- **Provocar** (Oriol, 07/10: «un provocar molt bàsic i fàcil d'esquivar. Al pujar de tier ja ho
  anirem millorant»): la *Provocación* del Guerrero es de arranque y básica — un enemigo, con
  salvación de defensa mental, te ataca a ti 1 turno. Lo de NosTale (a todos, 2 turnos, rigidez,
  Voluntad del defensor) queda para cuando las cartas mejoren con el tier.
- **Pasiva** (Oriol, 07/10): la de +1 al arma vale «de momento».

## 4. Cómo se lee cada habilidad

**Objetivo**

| NosTale | `target` |
|---|---|
| «el propio personaje» | `self` |
| «aliados en un área» | `all_allies` |
| «un objetivo seleccionado» (enemigo) | `single_enemy` |
| «un objetivo seleccionado» (aliado: *Sacrificio*) | `single_ally` |
| «enemigos en un área de 3 casillas» o más | `all_enemies` |
| «enemigos en un área de 1-2 casillas», «Área Especial» (línea, cono) | `zone_radius` (sin posiciones, la zona es el elegido) |

**Acción**: todas son `accion_principal` (como *Puño Volcánico* o *Escudo de Sombras*: una
mejora también gasta el turno), salvo las **ventanas de defensa de 2 s** (*Protector de
cuchillas*, *Escudo de Dragonio*, *Defensa rotativa*…), que son `reaccion` con
`bloquea:ataques:1` («contrarresta los ataques de ese turno»): se preparan y saltan solas
ante el próximo golpe. Lo que una reacción le da a quien la juega (`propio:`) dura **2
rondas**: salta en el turno del enemigo y tiene que llegar al tuyo (con 1 se acabaría al
cerrar la misma ronda). Una que solo devuelve una carta gastada es `libre` (*Convertidor
temporal*).

**Daño**: la suma de las bonificaciones de la ficha, cuerpo a cuerpo + elemental (*Batalla
Triple*: +700 +400 = 1100) da los dados de la carta. Una técnica con continuación (§2) suma
las de sus pasos. NosTale ya rebaja las de área, así que la tabla es la misma para un blanco
y para un área.

| Bonificación total | Dados |
|---|---|
| menos de 500 | `dano:1d6` |
| 500 - 1299 | `dano:2d6` |
| 1300 - 2499 | `dano:3d6` |
| 2500 o más | `dano:4d6` |

Las cartas que no tienen bonificación (mejoras, controles) no hacen daño. Sin `defenseStat`
la carta es un **golpe** (DANO-02: tirada enfrentada; si gana, los d6 de la diferencia más los
de la carta); con `defenseStat` es **magia** (sus dados; la mitad y sin estados si el blanco
se salva).

**Salvación** (carta de magia): solo las cartas de **control sin daño** cuyo estado
principal es un «efecto **mágico** negativo» en NosTale (*Intimidación*, *Ruptura de la
armadura*, *Tinieblas mortales*). `defensa_mental` si es miedo, maldición, corrupción o
intimidación; `resistencia_fisica` si es del cuerpo (petrificación, armadura destrozada, luz
que quema). Si la carta hace daño es un golpe, y el estado entra con la tirada del golpe.

**Orden de los efectos**: el de la tarjeta, salvo que si una carta limpia y cura, **primero
limpia** (con miedo, la tirada de la cura saldría a medias).

**Duración** (segundos → rondas)

| NosTale | Rondas |
|---|---|
| hasta 6 s | 1 |
| hasta 15 s | 2 |
| hasta 45 s | 3 |
| menos de 120 s | 5 |
| 120 s o más | `combate` |

**Probabilidad**: entra en la carta cada estado con un **50 % o más**; los de menos se
quedan fuera (en nuestro combate el estado ya depende de ganar la tirada enfrentada).

**Elemento**: la carta lleva el de su golpe en `tags` y `mechanicTags` (`Fuego`, `Agua`,
`Luz`, `Tinieblas`), para cuando haya resistencias. PART-01 (Oriol, 07/10: «no estamos limitados a
los 4 elementos del nostale aqui»): los elementos son los del juego (`tools/elementos.py`), y la
carta que dice ser de otro (el rayo, el hielo, la sangre, el veneno...) lleva ese, aunque NosTale
la deje en uno de sus cuatro: el Tronador y Thor son de Rayo, el Mago azul de Hielo, y las cartas
sueltas de rayo, hielo, tierra, viento, plantas, gas, sangre y maná, del suyo
(`planificacion/CANON_PARTICULAS.md` §1).

## 5. Diccionario de estados (NosTale → rasgo)

Contra quien lo recibe:

| NosTale | Rasgo |
|---|---|
| «Ataque no posible» (aturdimiento, rigidez, shock, petrificación, falta de energía…) | `incapacitado` (aturdimiento y shock, además `desventaja:defensa`; el texto dice el de NosTale) |
| «No hay defensa posible», destruir escudo, destrozar armadura | `desventaja:defensa` |
| Esquiva −X % (punto débil al descubierto, gas nervioso, gas opaco) | `desventaja:defensa` |
| Derribo | `condicion:caido` |
| Defensa −1/−2 o hasta −20 % | `penal_ca:1` |
| Defensa −3 o más, o −50 % | `penal_ca:2` |
| Pierde vida cada N s por heridas (hemorragia, trauma, herida, veneno) | `condicion:sangrado` |
| Pierde vida cada N s por fuego o maldición | `dano_por_turno:1` (el estado de la carta) |
| Acierto reducido, pérdida de visibilidad, ataque reducido | `desventaja:ataque` |
| Miedo («control por miedo») | `condicion:asustado` |
| Daño recibido +X % | `bono_dano_contra:1` |
| Probabilidad de golpe fatal +50 % o más | `critico_contra` |
| «Se fijarán en ti» (atraer, provocar) | `provocado:origen` (te atacan a ti) |

A favor de quien lo recibe:

| NosTale | Rasgo |
|---|---|
| Defensa +, daño recibido −20 % o menos | `bono_ca:1` |
| Daño recibido −25 % o más, «maximiza la defensa» | `bono_ca:2` |
| Todos los ataques +10 % o menos, ataque +1 | `bono_dano_arma:1` |
| Ataque +20 % o más, ataque cuerpo a cuerpo +200 o más | `bono_dano_arma:1d6` |
| Acierto +, crítico +, moral + | `ventaja:ataque` |
| Esquiva + | `ventaja:defensa` |
| Devuelve el daño al defender | `refleja:fisico` |
| Recupera vida (10-20 % o N×25-50 HP) | `cura:1d6`; más, `cura:2d6` |
| Quita vida y te la da | `cura:dano_causado` |
| Elimina efectos dañinos | `limpia` |

Si un estado trae varias cosas, entran **las dos que más pesan**; lo que el combate no
simula se ignora sin bloquear la carta: velocidad de movimiento, empujar o atraer
(no hay posiciones), MP, enfriamientos, resistencias elementales, PvP, objetos de
recuperación, espejismos.

**Compañeros e invocaciones**: una SP que lucha con un compañero (el Lobo solar del
*Cazasoles*) lo trae con una carta (`invoca:combate:<id>`: hasta el final del combate, ataca
al cerrar cada turno tuyo) y sus técnicas son órdenes: cada orden es una carta que suma la
bonificación de la técnica del compañero, como una continuación (§2). El compañero va a
`summons.json` (`invocaciones` en los datos de la SP) con las cifras de un animal de tier 1 del
bestiario, en CON, DES, INT y CAR (el cargador no lee FUE).
Las invocaciones del Mago que NosTale no detalla (el *Tótem sagrado*, las tres *Arañas
espirituales* del Sacerdote vudú, los cuatro *Clones oscuros* del Vidente) llevan las cifras de una
criatura de tier 1 del bestiario y, sin segundos en la ficha, duran lo mínimo: 1 turno (atacan una
vez y se van); si lo que hace no se puede jugar (un tótem que solo apoya), la carta es `pendiente:`.
Lo que ataca una sola vez (el tiburón de *Hora de comer*, la ballena azul, la espada de magma) no
es invocación: es una carta de daño.

**Escudos de maná** (uno en cada SP del Mago: *Transfusión Mana*, *Escudo mana*, *Guardián
Fantasma*, *Burbuja de agua*...): el daño recibido −X % es `propio:bono_ca` con sus segundos
(−25 % o más, +2); lo que el escudo pasa a los MP, fuera. Las bendiciones de grupo con
«bonificación elemental +(N×K)» llegan a +200 hacia el nivel 34-40: +1d6 a los golpes con arma,
como el *Espíritu de lobo*. *Terror leve* («ataque no posible» por terror) es miedo: `asustado`.

## 6. La pasiva

NosTale no tiene pasiva por SP. La nuestra sale de lo que la SP hace **siempre**:

- Si la SP vive de una **forma o un recurso** (luz / tinieblas, meditación, ira, marcas,
  agudeza, sincronización; en el Artista Marcial, los estilos haetae y dragón del Puño del
  dragón y las figuras de druida, oso y leopardo del Druida flamígero), la pasiva es esa forma:
  `elige_una` + `si:<forma>:<rasgo>`, como las estaciones del Bardo; las técnicas de todas las
  formas se juegan siempre; lo que no se pueda simular, `pendiente_parcial`.
- Si no, sale de su **ataque básico**: su cadena de golpes y su elemento, con el gancho nuevo
  `siempre:<rasgo>` (*Espadón a dos manos* del Guerrero: `siempre:bono_dano_arma:1`).

## 7. La ficha

Atributos: los que existen (CON, DES, INT, CAR). Las cartas tiran DES (DANO-02), así que el
Espadachín cuerpo a cuerpo va DES/CON o CON/DES, y los de ballesta DES con INT o CAR.

**Con qué se tira** (el Mago): lo dice la bonificación de la ficha de NosTale. «Ataque mágico» →
la carta tira **INT** (`castingStat`, como las del Arcanista), y la SP va INT/x; «a distancia» y
«cuerpo a cuerpo» → DES, lo de siempre. Nueve SP del Mago son magia en todas sus técnicas (`tira`
en sus datos); el *Mago de las tinieblas* y *Merlín* disparan el arma mágica secundaria (DES); la
*Tormenta de fuego* mezcla: el lanzallamas tira DES y su *Círculo de fuego* y su *Remolino
llameante*, INT (`tira` en la carta). El ataque básico (**Atacar**) tira DES para todos. El
Artista Marcial es «cuerpo a cuerpo» y «a distancia» en todas sus técnicas (el Tronador dispara
el cañón de descargas del guantelete izquierdo; Thor, el guante de aliento helado): DES.

| Rol (identidad de la SP) | Vida | Escala | Armadura / arma |
|---|---|---|---|
| `tank` (reúne y aguanta) | 8 | CON ×4 | pesada / pesada |
| `balanced` | 6 | CON ×3 | media / pesada |
| `agile` (rápido, a distancia) | 5 | CON ×2 | ligera / media |
| `support` (mejoras y curas de grupo) | 5 | CON ×2 | media / media |
| `caster` (lanza desde atrás: los Magos) | 4 | CON ×1,5 | ligera / ligera |

Son las plantillas que ya usan las 63 clases para cada rol.

## 8. El set de la SP (y lo que lleva la clase)

Oriol (07/10): «no tienen que llevar los equipos estos cada una de las clases, son
magos/guerreros/arqueros/etc, no deben llevar el set entero» y «de momento un set entero
por sp».

- **La clase no lleva su set.** Empieza como las otras 63: el cofre de El Vacío le da el kit
  de su arquetipo (`Game::KitBasico`, +2/+2/+1/+1 exactos) con un **arma normal de su tipo**
  (el Guerrero, el *Espadón de Recluta de Milicia*). Lo que la hace suya son la ficha, las
  cartas y la pasiva.
- **Un set entero por SP**: el arma y las cuatro piezas (*Espadón, Yelmo, Coraza, Quijotes y
  Escarpes del Guerrero*), objetos raros de tier 2 (nivel 20, 550 de precio, como las raras
  del catálogo), con la etiqueta `Set del <SP>` (`Set de Aquiles`: nombre propio, sin
  artículo). Al ser raros no entran en ningún kit inicial: se consiguen jugando y cualquiera
  se los puede poner y combinar con lo suyo.
- Lo que se ve es lo de la SP en NosTale: las piezas propias se dibujan en
  `tools/actores_trajes.py` (siluetas que salen del cuerpo; ninguna tapa la cara) y
  `actores_estetica.PIEZA_DE_ROPA` liga cada nombre con su pieza; las que no necesitan dibujo
  propio usan una del catálogo (quijotes, escarpes, pantalones, sandalias). El arma toma su
  forma por el nombre (*espadón* → `f_mandoble`, *katana* → `f_sable`).
- **El nombre del set** es el de la SP (`Set del Ninja`); si es largo, el corto que llevan sus
  piezas (`set_de`: el *Servidor del cañón de fuego* viste el `Set del Cañonero`). Los nombres
  propios van sin artículo (`Set de Aquiles`, `Set de Volcano`, `Set de Grávitas`, `Set de
  Merlín`, `Set de Thor`), los femeninos con «la» (`Set de la Tormenta de fuego`) y los
  femeninos en plural con «las» (`Set de las Artes Místicas`).
- **Armas de fuego** (Oriol, 07/10: «Añadir el concepte d'armes de foc però, en comptes de
  pòlvora alquímia i màgia son la base tecnològica»): la escopeta, el cañón de mano, el
  revólver, el lanzagranadas y el rifle del Arquero existen, pero no llevan pólvora: cargan sal
  de fuego, granadas alquímicas y balas rúnicas. Como el catálogo no tenía ninguna (ni
  bumerán), cada SP trae la suya normal de tier 1, «de Viaje» (`armas_nuevas`), para empezar.
  Tienen forma propia (`actores_formas`: escopeta, rifle, lanzagranadas, revólver, cañón de mano
  y bumerán, con su escalón raro) y se cogen como en los vídeos de NosTale: las largas
  horizontales a la cintura, el cañón al hombro, los revólveres abajo junto a la pierna.
- **Los sets del Arquero** se dibujan en `tools/actores_trajes_arquero.py` (detrás de los del
  Espadachín); el Cazademonios lleva las piezas del cazador de demonios de SP-01 y el Cañonero
  sigue al *Fire Cannoneer* de los vídeos de Oriol (gorro de aviador con gafas, chaqueta de
  cuero). Si dos piezas del catálogo salen idénticas a otras, el tema las tiñe o se cambia la
  pieza (cada objeto se ve distinto: `estetica_todos_los_objetos_se_montan`).
- **Los sets del Mago** se dibujan en `tools/actores_trajes_mago.py` (detrás de los del Arquero);
  el Mago de fuego lleva la corona de llamas y la túnica de SP-01. Sus dos armas nuevas son de
  magia y alquimia: la **pistola mágica** del Mago de las tinieblas y de Merlín (balas de maná; se
  coge como el revólver) y el **lanzallamas** de la Tormenta de fuego (aceite alquímico; como la
  escopeta), con su arma normal «de Viaje» y su escalón raro.
- **Los guanteletes del Artista Marcial** (Oriol, 07/10: «Guanteletes grandes»): un par, uno en
  cada mano, que es **una sola pieza** de la ranura `arma` (a dos manos: `A dos manos` en sus
  restricciones, como el espadón). Se dibujan ya a su tamaño, sin pose (`actores_formas`:
  guanteletes, garras, brazos mecánicos y el cañón de descargas del Tronador, que va en la mano
  izquierda: `abajo_2`/`derecha_2`), y se pintan en los dos puños (`actores_armas.PAREJA`,
  `capa_pareja`): de frente y de espaldas los dos encima del cuerpo (las manos van a los lados);
  de perfil, el de delante encima de todo y el de atrás solo sobre su mano, su brazo o fuera del
  cuerpo. Por eso su fila de `personajes.json` trae su **propio orden de dibujo** (`"orden"`: 80
  en los cuatro rumbos; un arma va detrás del cuerpo de espaldas), que el motor
  (`FilaDeCapa::orden`, `MontajePersonajes`) y el probador usan antes que el de la ranura. La
  normal de tier 1 son los **Guanteletes de Viaje** (comunes), con su escalón raro (`_raro`: los
  nudillos de guarnición y la gema en el dorso).
- **Los sets del Artista Marcial** se dibujan en `tools/actores_trajes_artista.py` (detrás de los
  del Mago). Varias cabezas reutilizan plantillas de otras SP con sus colores: la cinta del Ninja
  (el Puño del dragón), el tocado de lobo del Cazasoles (el Maestro Lobo; el oso del Druida, con
  las orejas redondas), los cuernos del Berserker (el Guerrero demoníaco) y el yelmo alado del
  Cruzado (Thor).
- **Colores**: las piezas dibujadas usan paletas propias **sin tipo** (`actores_estetica.TIPO`
  no las conoce), así que ningún tema las toca. El tema del set (`TEMAS_POR_NOMBRE`, clave
  `del <sp>`) tiñe el arma y las piezas del catálogo que el set reutiliza: los quijotes del
  Berserker en hierro negro, los calzones del Monje en blanco. Una paleta nueva no puede pisar
  una que ya existe (el módulo no carga) ni compartir tonos con otra.
- **Medidas**: nada de una pieza de cabeza sube de la fila 6 (en la talla grande la cabeza
  sube 5 y el dibujo no puede tocar el borde de la celda), y todas se prueban en las cuatro
  tallas.
- **Variaciones**: más adelante («podemos hacer muchas variaciones de estos sets»). Cada set
  se dibuja una vez; una variación es otro tinte del mismo dibujo (el sistema de ESTETICA-01).
- Sin referencias de Oriol todavía (tarea pendiente: «biblioteca de referencias por SP»), cada
  set es una adaptación desde la identidad y el elemento de la SP: se corrige al verlo.

## 9. Ganchos nuevos (motor)

| Gancho | Dónde | Qué hace |
|---|---|---|
| `siempre:<rasgo>` | pasiva | El rasgo pesa sobre el héroe todo el combate (sin elegir nada). |
| `propio:<rondas>:<rasgo>` | carta | Quien juega la carta gana ese estado, apunte adonde apunte (*Provocación*: los enemigos te atacan y tú aguantas). En una reacción, al saltar. |
| `limpia` | carta | Quita los estados en contra de los aliados a los que apunta (a ti, si es `self`). En una reacción, al saltar. |

Y un arreglo: un estado de `penal_ca` se ve como perjuicio (antes salía como mejora).

## 10. Lo que no se simula

Posiciones (empujar, atraer, saltar, teletransporte), velocidad, MP, enfriamientos, cargas
y medidores que NosTale no publica con cifras. Si sin eso la carta se queda sin efecto,
es `pendiente:` y sale bloqueada con su motivo; si solo pierde un matiz,
`pendiente_parcial:` y se juega.
