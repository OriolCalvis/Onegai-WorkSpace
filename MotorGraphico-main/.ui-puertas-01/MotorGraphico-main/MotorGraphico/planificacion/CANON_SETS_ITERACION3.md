# Canon SETS-03 — Los sets de tier 2 de la 3.ª iteración y las cartas del Monje y del Mago Chasqueador

Oriol (08/10/2026): «con los conceptos que hicimos los sets del Nostale me hagas los sets del apóstata,
señor de las armas, furia de los creadores, bardo de las 4 estaciones de tier 2, siguiendo los mismos
criterios, y el mago de los chasquidos y el monje que hice, por cierto, me podrías dar descripciones a
las habilidades de estos dos últimos, solo les puse la pasiva y el título».

Y a mitad: «el origen del bardo de las 4 estaciones salió en la cultura de los hombres gato, lo digo
para que adaptes mejor la estética» (§3).

Son las seis clases de CLASES-03 (`tools/importar_clases_companeros.py`). Datos de los sets:
`tools/sets_iteracion3.py`; dibujo: `tools/actores_trajes_iteracion3.py` (las piezas) y
`tools/actores_formas_iteracion3.py` (los guantes); los escribe en los catálogos el mismo importador.

## 1. Los criterios (los de las SP y las clases propias)

Los de `CANON_SP_CLASES.md` §8 y `CANON_CLASES_PROPIAS.md` §7, sin cambios:

- Un set entero por clase: el arma, la cabeza, el torso, las piernas y los pies, raros de tier 2 (nivel
  20, 550; `sp_reglas.pieza`), con la etiqueta `Set del <nombre corto>` (`set_de`; «Set de la Furia» y
  «Set de las Cuatro Estaciones», en femenino: `sp_reglas.NOMBRES_FEMENINOS*`).
- La clase **no** lo lleva: empieza como hasta ahora; son objetos sueltos que cualquiera puede ponerse.
- El peso de la armadura, el de su rol: media el apoyo (Apóstata) y el equilibrado (Furia); ligera el
  lanzador (Señor de las Armas, Mago Chasqueador), el ágil (Bardo) y el Monje, que no puede llevar más.
- Bonos: el arma, la cabeza, las piernas y los pies, +1 a los dos atributos de la clase; el torso, +1 al
  principal.
- El arma, la de su tipo (sable, orbe, maza); el Monje y el Mago Chasqueador no traen ninguna, así que
  es la que dice qué son: los guanteletes del artista marcial y los guantes con que chasquea, los dos en
  las dos manos (`PAREJA`).
- El dibujo sale de la identidad de la clase y de sus partículas (`tools/particulas.py`), sin pisar los
  64 sets que ya hay. Paletas propias sin tipo (ningún tema las toca) y sin un tono de otra paleta; lo
  del catálogo que se reutiliza lo tiñe el tema del set (`actores_estetica.TEMAS_POR_NOMBRE`, también el
  arma) y `actores_estetica.PIEZA_DE_ROPA` liga cada pieza con su dibujo. Todo va al final de cada atlas.

## 2. Los seis sets

| Set | Rol, peso | Atributos | Lo dibujado | Lo del catálogo, con su tinte |
|---|---|---|---|---|
| Apóstata | apoyo, media | CAR / CON | coraza de paladín ennegrecida con las hombreras de pinchos y el tabardo gris rasgado con el sol de la orden bordado boca abajo en rojo; la cadena por cinturón; capucha granate con la aureola partida flotando encima | sable de hoja negra con el filo rojo; grebas de acero ennegrecido, botas altas negras |
| Señor de las Armas | lanzador, ligera | INT / CON | **tres sets** (SETS-04, §7): el Duque, el Soberano sin Rostro y el Rey de la Forja. Sustituyen al de antes (manto real carmesí con armiño y el arsenal flotando, corona de espadas con el rubí) | el orbe de cada set, con su color; nada más del catálogo |
| Furia | equilibrado, media | CAR / CON | peto de placas blancas con los cantos de oro, los brazos al aire con las manos vendadas, la piel de oso negra y la faja roja de guerra; yelmo blanco con la corona de rayos de oro detrás; botas de cuero rojo con el borde de piel negra | maza de aletas de oro con el mango de cuero rojo; grebas blancas |
| Cuatro Estaciones | ágil, ligera | CAR / DES | **poncho de lana dorada** con la Marcha de las Cuatro Estaciones tejida en las cenefas y los flecos; corona de hojas que cambia de estación al dar la vuelta (deja libres las orejas); pantalones de lana parda (§3) | sable de hoja escarchada con el puño verde, la guarda de oro y la gema roja; botas blandas de cuero pardo |
| Chasqueador | lanzador, ligera | INT / DES | chaqueta corta azul noche de cuello alto con los alamares de plata y la faja violeta; bombín con la cinta violeta y la estrella de plata; pantalón con la franja violeta | **guantes blancos de mago** (forma nueva, §4); botas altas negras |
| Despertar | Monje, ligera | DES / CON | hábito de marfil sin mangas con las solapas y la faja de jade y las vendas en los antebrazos; diadema con el tercer ojo de jade; pantalones de jade atados al tobillo | guanteletes de jade con los nudillos de oro; sandalias |

## 3. El Bardo de las Cuatro Estaciones, de los hombres gato

Oriol (08/10): el Bardo «salió en la cultura de los hombres gato». Lo que dice su canon (el vault, «Onegai
2»):

- **Gran Compendio de los Creadores, «Hombres Gato (Bosmurg)»**: se asentaron en las **Praderas de Oro**
  y desarrollaron «una cultura nómada y centrada en la caza y la narración oral»; «su cultura se basaba en
  la tradición oral, en las canciones y las historias que contaban alrededor de fuegos o en grandes plazas
  frente a los que querían escuchar».
- **Gran Compendio, «La Marcha de las Cuatro Estaciones — 8790 b.f.»**: la campaña del Clan Cara Loca
  (Mahn'în el Cara Loca y su general, Kaestra Mirada del Alba) para unir a los clanes, en cuatro fases:
  verano (la Conquista de los Bosques), otoño (la Llama del Desafío), invierno (el Fracaso en las
  Montañas) y primavera (la Alianza de las Llanuras, donde nace la Nación de Bosmurg). Su legado: **«La
  Balada de las Cuatro Estaciones: una canción transmitida de generación en generación que narra la
  marcha y el destino de cada estación»**.
- **«Hombres Gato (Felinos)»** (la ficha de raza): «viven en tribus nómadas, o de forma aislada, en
  lugares donde puedan estar en contacto con la naturaleza»; se mueven en silencio (Sigilo y Percepción).

Así que el set es el de **un bardo de camino, no de corte** (antes era un jubón cuartelado con calzas
partidas, que además se parecía al del Comediante). Lo que es canon y lo que es dibujo:

| Pieza | De dónde sale |
|---|---|
| Poncho de lana dorada con flecos | nómadas de las Praderas de Oro; el oro es también el color de Bosmurg en el mapa (`#e8d98a`, `locations.json`). El poncho es una propuesta de dibujo, no un dato del canon |
| Las cenefas | la Marcha tejida en su orden (verano, otoño, invierno, primavera), en bloques de 2 px; la del pecho, si cabe (en la talla pequeña no) |
| La corona de hojas de las cuatro estaciones | la de antes: rodea la cabeza por debajo de las orejas de gato (`actores_rasgos.OREJAS_GATO`) |
| Botas blandas de suela fina | «moverse en silencio» |
| Pantalones de lana parda | de viaje |
| El sable | el de su tipo (la Canción del Verano ya es fuego, como las espadas del verano de la Marcha) |

Probado en los dos pueblos de hombres gato (Cara Loca y Cazador Nocturno), en un humano y en las cuatro
tallas.

## 4. Los guantes del Chasqueador

Forma nueva `guantes` (`tools/actores_formas_iteracion3.py`): el guante blanco de mago con el puño
vuelto (todo material 1) y, en el escalón raro (el del set), las runas violeta en las yemas (la gema).
Van en las dos manos (`PAREJA`) y, como los guanteletes, tienen que tapar más del 70 % de la mano que
se ve en todas las tallas y frames (`test_los_guanteletes_van_en_las_dos_manos`): por eso miden 7×6 de
frente y 8×6 de perfil (la mano de la talla grande, andando de perfil, va en horizontal y ocupa 12 px).

## 5. Las cartas del Monje del Despertar y del Mago Chasqueador

Oriol solo había puesto el nombre y la pasiva. Las descripciones las escribió Claude desde cada nombre,
dentro del presupuesto de las clases propias (`CANON_CLASES_PROPIAS.md` §3), con su gancho. Las pasivas
(Ki y Cascada Letal) siguen como las dejó Oriol: ninguna carta depende de ellas.

**Monje del Despertar** (tira DES). Arranque: Tres Golpes de Puro Dolor, Abdomen de Marfil y
Rejuvenecimiento de Jade (pega, aguanta y se cura).

| Carta | Qué hace | Gancho |
|---|---|---|
| Purificación Corporal | te libras de los estados en contra y te defiendes con ventaja 2 turnos | `limpia`, `estado:2:ventaja:defensa` |
| Abdomen de Marfil (reacción) | el próximo golpe rebota; +1 a la CA 2 turnos | `bloquea:ataque`, `propio:2:bono_ca:1` |
| Rejuvenecimiento de Jade | te curas 2d6 | `cura:2d6` |
| Ráfaga Elemental | 3d6 de daño | `dano:3d6` |
| Tres Golpes de Puro Dolor | tres ataques con los puños | `ataques_arma:3` |
| Golpe Sanguíneo | 2d6 y sangra 3 turnos | `dano:2d6`, `condicion:sangrado:3` |
| Puño Ensordecedor | 1d6, aturdido 1 turno y se defiende con desventaja | `dano:1d6`, `estado:1:incapacitado`, `estado:1:desventaja:defensa` |
| Estampida (Creador) | 3d6 a cada enemigo y caen al suelo 1 turno | `dano:3d6`, `condicion:caido:1` |
| Sentidos de Alerta (Creador) | atacas y te defiendes con ventaja todo el combate | `estado:combate:ventaja:ataque`, `…:defensa` |
| Canción de Dios (Creador) | tú y tus aliados os libráis de los estados en contra y os curáis 2d6 | `limpia`, `cura:2d6` |

**Mago Chasqueador** (tira INT: todo lo lanza chasqueando los dedos; las estándar llevan la etiqueta
`estandar` para la Recuperación Prestidigitadora).

| Carta | Qué hace | Gancho |
|---|---|---|
| Bola de Fuego | 3d6 de fuego | `dano:3d6` |
| Paso Instantáneo (movimiento) | te defiendes con ventaja 1 turno | `estado:1:ventaja:defensa` |
| Sombra Cegadora (salva: resistencia física) | cegado 2 turnos | `condicion:cegado:2` |
| Rayo Electrizante | 2d6 de rayo y se defiende con desventaja 2 turnos | `dano:2d6`, `estado:2:desventaja:defensa` |
| Ocupa mi Lugar | un aliado se libra de los estados en contra y se defiende con ventaja 2 turnos | `limpia`, `estado:2:ventaja:defensa` |
| Espiral de Espinas | 2d6 de naturaleza y sangra 2 turnos | `dano:2d6`, `condicion:sangrado:2` |
| Recuperación Prestidigitadora (libre) | recuperas una carta estándar gastada | `devuelve_carta:estandar` |
| Sin Tregua (Creador) | 4d6 arcano y se defiende con desventaja 2 turnos | `dano:4d6`, `estado:2:desventaja:defensa` |
| Repliegue Mágico (Creador, reacción) | contrarrestas todos los ataques 1 turno y te defiendes con ventaja el resto del combate | `bloquea:ataques:1`, `propio:combate:ventaja:defensa` |
| Sin Sentido (Creador; salva: defensa mental) | 2 turnos sin atacar y defendiéndose con desventaja | `estado:2:incapacitado`, `estado:2:desventaja:defensa` |

## 6. Lo que queda para Oriol

- ¿Le gustan los seis sets, y el Bardo como bardo de camino de las Praderas de Oro?
- La compañera bardo reclutable (`companero_bardo_de_las_cuatro_estaciones`, «Ilse de las Estaciones»)
  sigue siendo una elfa del bosque provisional: ¿la hacemos Cara Loca?
- Las pasivas Ki y Cascada Letal siguen con sus `[pendiente]`.
- El Monje y el Mago Chasqueador no traen arma inicial (empiezan con el kit de su arquetipo).

## 7. SETS-04 — Los tres sets del Señor de las Armas

Oriol (08/10/2026): «vull modificar estèticament la classe del Senyor de les armes», con el oro, el maná rojo
que brilla, las armas que flotan y el aura dorada de una imagen de referencia, pero en un diseño propio («vull
un disseny basat en l'armadura i efectes visuals [...] per fer una cosa completament nova»). La referencia era
de un personaje conocido, así que no se copia su conjunto: los diseños no llevan su pelo, el pecho al aire con
la marca, las cadenas ni los portales. De los tres esbozos (`Claude outputs/esbossos_senyor_armes/`): «estan
perfectes els 3».

Son tres sets del Señor de las Armas con los mismos criterios (§1) y los mismos bonos y peso (lanzador,
ligera, INT / CON). Sustituyen al de SETS-03, que sale de los catálogos (`sets_iteracion3.RETIRADOS`; el
importador lo quita). La casaca y la diadema del Duque ocupan las filas del manto y de la corona de antes; lo
demás va al final de cada atlas.

| Set | Lo dibujado | El orbe (`f_orbe`, teñido) |
|---|---|---|
| **Set del Duque** | casaca negra cerrada de cuello alto con los cantos y los botones de oro, las líneas de maná rojo por los brazos y los puños de oro; hombrera grande de oro en el brazo del arma y pequeña en el otro; cinturón ancho de oro con la placa de gema; placas de oro y tiras rojas en las caderas; capa roja que nace de la cintura (de frente, el viento la lleva a un lado); cuatro hojas que flotan a los lados, con el filo de maná rojo. Diadema de oro con el rubí. Calzas negras con el rombo de oro en la rodilla; botas negras con la greba de oro en punta | maná rojo en montura de oro |
| **Set del Soberano sin Rostro** | yelmo negro cerrado con la corona de oro de cinco puntas de espada, la cresta de oro y la rendija en T que brilla de rojo (tapa el pelo); placas negras con los cantos de oro, tres placas de oro en cada hombro, el tabardo rojo con la corona de espadas y la capa larga roja con el bajo de oro; cuatro hojas espectrales rojas, translúcidas, que le rondan la cintura. Grebas negras con el rombo de oro; escarpes negros con la puntera de oro | rojo en montura de oro viejo |
| **Set del Rey de la Forja** | láminas de hierro oscuro con las grietas de oro fundido, los nudillos de oro fundido, hombreras con el canto al rojo y el mandoble recién forjado a la espalda (de espaldas, entero; de frente, lo que asoma); corona de hojas rotas con las puntas al rojo. Grebas de hierro con una grieta en cada espinilla; botas de hierro con la puntera de oro fundido | oro fundido en montura de hierro |

Las tres brillan con las partículas de la clase (oro). El pelo, la barba y los ojos de los esbozos son del
personaje, no del set.
