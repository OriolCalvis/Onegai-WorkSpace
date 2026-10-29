# Las SP de NosTale como clases nuevas: Espadachín, Arquero, Mago y Artista Marcial · 07/10/2026

Sesión de Claude (Cowork). Sigue a `2026-10-06-claude-estetica.md`. Oriol:

> «Métete de lleno en las clases nuevas.»

Y a las preguntas: **cada SP, una clase nueva** (ficha, cartas, pasiva, traje y arma; se elige
al crear como las otras 63), **clase a clase y completa**, cartas **jugables donde se pueda** (lo
que el combate no cubre queda `[pendiente]` y bloqueado, como en H2-06), empezando por el
**Espadachín entero** (12 SP). A mitad, sobre el aspecto:

> «No tienen que llevar los equipos estos cada una de las clases, son magos/guerreros/
> arqueros/etc, no deben llevar el set entero, ¿sabes? Y podemos hacer muchas variaciones de
> estos sets.» — «De momento un set entero por SP.»

Seguimiento: `planificacion/seguimiento/SP-02.json`. Reglas de la adaptación:
`planificacion/CANON_SP_CLASES.md`.

## 1. De dónde sale y cómo se convierte

La fuente es la enciclopedia de SP de Oriol (Obsidian, Onegai/NosTale): 52 SP, 627
habilidades, 483 estados, con las cifras de cada ficha. La conversión tiene reglas para que
salga siempre igual (el canon):

- El **ataque básico** no es carta: es *Atacar* con el arma de la clase, y da la pasiva.
- **Creador** = las tres que la SP enseña al final (niveles de profesión 12, 16 y 20). Son sus
  definitivas; las otras siete, estándar.
- **Dados** por la bonificación de la ficha (cuerpo a cuerpo + elemental): menos de 500, 1d6;
  hasta 1299, 2d6; hasta 2499, 3d6; más, 4d6. Una técnica con continuación suma las de sus pasos.
- **Rondas** por los segundos: hasta 6 s, 1; hasta 15, 2; hasta 45, 3; menos de 120, 5; más,
  todo el combate.
- **Estados** con un 50 % o más; un diccionario dice qué rasgo es cada uno (aturdimiento y shock:
  no ataca y se defiende con desventaja; defensa −2: −1 a la CA; miedo: asustado; heridas:
  sangrado; fuego o maldición: daño por turno; «se fijarán en ti»: te atacan a ti…).
- **Magia** (salvación) solo en las cartas de control sin daño cuyo estado es «mágico» en
  NosTale: miedo y maldición contra la defensa mental, lo del cuerpo contra la resistencia física.
- Las **ventanas de defensa de 2 s** son reacciones: paran los ataques de ese turno, y lo que
  ganas al saltar dura 2 rondas.

El código: `tools/sp_reglas.py` (la regla), `tools/sp_espadachin.py` (los datos de las 12 SP:
cada carta dice de qué H sale y sus cifras pasan por las funciones de la regla) y
`tools/importar_clases_sp.py` (los escribe en los catálogos y comprueba ganchos, rasgos, que el
texto diga sus dados y sus turnos, y que cada set esté entero).

## 2. Las doce

| SP | Rol | Arma al empezar | Pasiva | Arranque | Creador (tier 2) |
|---|---|---|---|---|---|
| Guerrero | tanque, CON/DES | Espadón de Recluta de Milicia | Espadón a dos manos | Batalla Triple, Piel de acero, Torbellino | Estimulación, Terremoto, Dragón volador |
| Ninja | ágil, DES/CON | Sable Ligero de Milicia | Cuchillada de espada | Cuchillada de cruz, Cuchilla energética, Ataque en cadena | Cuchillada mortal, Borde afilado, Espada danzante |
| Cruzado | apoyo, DES/CAR | Ballesta Ligera de Milicia | Ataque elemental | Flecha santa, Ataque triple, Aliento santo | Plegaria de defensa, Plegaria de ataque, Fuerza concentrada |
| Berserker | equilibrio, DES/CON | Espadón de Recluta de Viaje | Mueve la barra | Golpe, Gran golpe, Berserker | Chute de adrenalina, Vida y muerte, Corte del tornado |
| Gladiador | equilibrio, DES/CON | Espada Larga | Golpes de todas partes | Golpe doble, Relámpago, Ataque de escudo | Gangrena, Declaración de guerra, Ejecución |
| Monje de lucha | equilibrio, DES/CON | Lanza Corta de Milicia | Meditación (eliges espíritu) | Mar partido, Punzada directa, Remolino violento | Palabras de Buda, Borrón y cuenta nueva, Lanza de dragón |
| Cosechador de almas | equilibrio, DES/CON | Guadaña de Viaje (nueva) | Guadaña de la muerte | Cuchilla, Aspirasangre, La muerte se acerca | Tinieblas mortales, Convertidor temporal, División de espíritus |
| Renegado | equilibrio, DES/CON | Espada Larga | Espada de la luz | Juicio de la luz, Patíbulo de la luz, Justicia | Reignición, Corrupción, Conversión |
| Hachero de la cascada | equilibrio, DES/CON | Gran Hacha de Milicia | Golpe de hacha | Lanzamiento de hacha, Ataque fugaz intenso, Hacha de la ira | Remolino en cascada, Poder de la cascada, Golpe furioso de cascada |
| Caballero Draconiano | tanque, CON/DES | Lanza Corta de Viaje | Golpe rápido | Golpe largo, Golpe de escudo, Embestida lancera | Destrucción de armadura, Luz de protección, Aguijón de dragón |
| Romperrocas | equilibrio, CON/DES | Martillo de Guerra de Milicia | Golpe de martillo | Extracción, Inyección mineral, Propulsión por cohete | Impulso propulsor por cohete, Defensa rotativa, Extracción explosiva |
| Aquiles | ágil, DES/INT | Ballesta Ligera de Viaje | Lanzada | Lanza del olvido, Niebla ascendente, Maldición del olvido | Mar de niebla, Lanza del Estigia, Lluvia de lanzas del Estigia |

Todas las pasivas salen del ataque básico (+1 al daño de tus golpes con arma), salvo la del
Monje (eliges Fuerza, Iluminado o Mesura al empezar el combate, como el Bardo sus estaciones).
Las barras de NosTale (ira del Hachero, agudeza del Caballero, marcas del Romperrocas,
sincronización de Aquiles) quedan `[pendiente]` en su pasiva sin bloquearla; y los matices
que el combate no simula (crecer cuanta menos vida, devolver el 20 % del daño) van como
`pendiente_parcial`: la carta se juega igual.

Una cosa del orden: *Recuperación espiritual* del Ninja primero te libra de los estados en
contra y después te cura (con miedo, la tirada de la cura salía a medias). Queda en el canon.

## 3. Ganchos nuevos (y dos arreglos)

- `siempre:<rasgo>`: la pasiva que no se elige.
- `propio:<rondas>:<rasgo>`: quien juega la carta gana el estado, apunte adonde apunte
  (*Provocación*, *Salto con cohete*, *Niebla impenetrable*). En una reacción, al saltar.
- `limpia`: quita los estados en contra de los aliados a los que apunta, y las condiciones
  negativas de su ficha (*Recuperación espiritual*, *Palabras de Buda*, *Luz de protección*,
  *Ejecución*; en *Protección de la cascada*, al saltar).
- Un estado de `penal_ca` (−1 a la CA) se ve como perjuicio, no como «mejora».
- `CartasTiradas.cpp` había pasado el techo de 500 líneas con el piloto (505): la pasiva del
  héroe pasa a `CartasEnCombate::rasgosDePasiva` y vuelve a 493.

## 4. El aspecto: la clase no lleva su set

- La clase empieza como las otras: el cofre de El Vacío le da el kit de su arquetipo y un arma
  normal de su tipo. Faltaba una guadaña normal: *Guadaña de Viaje* (común, tier 1).
- Cada SP tiene **un set entero** (arma, cabeza, torso, piernas y pies), objetos raros de tier 2
  con la etiqueta *Set del Ninja*, *Set de Aquiles*… Cualquiera se los puede poner y combinar;
  al ser raros no entran en ningún kit inicial.
- Dibujo (`tools/actores_trajes.py`): el Ninja con su gi cruzado, el fajín, la bufanda larga
  que vuela al andar y la cinta con placa y colas; el Cruzado de plata con tabardo blanco, cruz
  de oro y yelmo alado; el Berserker negro con pinchos de hueso, faja roja y cuernos; el Monje
  con hábito azul, rosario grande y diadema de cuentas; el Cosechador con manto negro de forro
  violeta, capucha honda (la cara en sombra, los ojos se ven) y una calavera en el cinturón; el
  Renegado partido en dos (blanco y oro / negro y violeta); el Hachero con el pecho al aire, los
  correajes en aspa con la runa del agua y piel en hombros, casco y botas; el Caballero
  Draconiano en escamas blancas con alas de oro y cresta de dragón; el Romperrocas de minero
  (peto con tirantes, bolsas, casco con lámpara y gafas); Aquiles en bronce oscuro con el manto
  violeta del Estigia y la galea de cresta negra. El Gladiador usa las piezas de SP-01.
- Las piezas dibujadas usan 37 paletas propias **sin tipo**: ningún tema las tiñe. El tema del
  set tiñe el arma y lo que el set toma del catálogo (los quijotes del Berserker en negro, los
  calzones del Monje en blanco, las botas del Renegado en negro). Las botas altas del Renegado
  tapaban los quijotes blancos: pasan a botas de caña corta.
- Probado en las cuatro tallas: la cresta de Aquiles tocaba el borde de la celda en la talla
  grande (la cabeza sube 5) y baja a la fila 6, que queda en el canon como límite.

## 5. Verificación

- `test_clases_sp` 26/26: las 12 clases y los 12 sets, la pasiva de +1 en todas menos el Monje
  y una prueba por SP de lo suyo (limpiar y curar, parar y afilar, magia contra el cuerpo,
  curar lo que quitas, devolver una carta, quemar cada ronda, el espíritu del Monje…).
- `test_cartas_combate`, `test_barra_combate`, `test_clases_companeros`, `test_kit_basico`,
  `test_cuaderno_creacion`, `test_inventory_menu` en verde.
- ctest: las mismas que fallaban antes del piloto; la deuda de líneas del piloto, arreglada.
- `test_validar_sprites.py` 40/41 (la de siempre), `validar_catalogos_rpg.py` sin fallos,
  `gen_personajes` sin avisos.
- Juego real: 75 clases en la cuadrícula; la hoja de Clase del Cosechador de almas con su
  pasiva, sus 7 + 3 cartas y la Guadaña de Viaje en el kit. El combate de cartas de SP ya se
  vio con el Guerrero (Provocación, Batalla Triple, la pasiva).
- Motor (`demo_brillo_equipo`): los 12 sets puestos con su tinte, andando y en reposo.

## 6. Las respuestas de Oriol y el Arquero

A las cuatro preguntas del Espadachín: la pasiva de +1, «Està bé de moment deixem ho així»; la
Provocación, «Un provocar molt bàsic i fàcil d'esquivar. Al pujar de tier ja ho anirem
millorant» (hecho: de arranque, un enemigo, con salvación de defensa mental, 1 turno); los sets,
«Estan bastant be»; la clase no empieza con ninguna pieza de su set. Y: «Si, continua amb
Arquero, Mago y Artista Marcial.»

Dos decisiones más, por pregunta: las **armas de fuego** existen («Añadir el concepte d'armes de
foc però, en comptes de pòlvora alquímia i màgia son la base tecnològica»: escopeta, cañón de
mano, revólver, lanzagranadas y rifle cargan sal de fuego, granadas alquímicas y balas rúnicas);
y el Artista Marcial irá con **guanteletes grandes**, una familia de arma nueva (uno en cada
mano).

Las doce SP del Arquero, con las mismas reglas (los datos, en `tools/sp_arquero.py`; las reglas
comunes de las tres clases base pasan a `tools/sp_reglas.py`):

| SP | Rol | Arma al empezar | Pasiva | Arranque | Creador (tier 2) |
|---|---|---|---|---|---|
| Cazador | ágil, DES/INT | Arco Cazador de Milicia | Tiro de arco | Ataque de falcón, Tiro de cabeza, Ataque relámpago | Explosión relámpago, Recuperación rápida, Tormenta |
| Asesino | ágil, DES/CON | Daga de Milicia | Puñalada | Acierto crítico, Cuchillada sangrante, Niebla espesa | Cuchillada giradora, Cuchillada mortal, Bomba |
| Destructor | equilibrio, DES/CON | Escopeta de Viaje (nueva) | Dispara la escopeta | Tiro rápido, Tiro lejano, Bomba de gas tóxico | Tiro boom, Mina de fuego, Caída del infierno |
| Guardabosques | apoyo, DES/CAR | Bumerán de Viaje (nueva) | Lanza el bumerán | Golpe de las serpientes, Zarpa de bestia, Espíritu de águila | Hiedra, Espíritu de oso, Fuerza del bosque |
| Servidor del cañón de fuego | equilibrio, DES/CON | Cañón de Mano de Viaje (nueva) | Bombardeo | Disparos retardados, Desviamiento de cañón, Disparo de cañón | Montar en cohete, Maestro de las armas, Cañón supremo |
| Espía | ágil, DES/INT | Ballesta Ligera de Milicia | Ataque elemental | Vuelo en picado de halcón, Retroceso, Fuerza vital | Recarga rápida, Emboscada, Francotirador |
| Cazademonios | ágil, DES/CAR | Revólver de Viaje (nueva) | Tiro del diablo | Lluvia de balas, Hoja de tiro, Escalofrío del espíritu de la venganza | Explosión de espíritus, Posesión, Cuchilla de almas |
| Ángel vengador | equilibrio, DES/CAR | Espada Corta | Energía de los elementos (eliges) | Corte huidizo, Explosión aguzada, Espada del juicio | Punzada meteorito, Espada divina, Pincho fantasmal |
| Cazasoles | apoyo, DES/CAR | Jabalinas de Milicia | Lanzamiento | Orden de ataque fugaz, Lanza solar, Curación natural | Erupción solar, Protección de la naturaleza, Red lumínica |
| Granadero | equilibrio, DES/CON | Lanzagranadas de Viaje (nueva) | Disparar granada | Granada de choque, Disparo vertiginoso, Granada de napalm | Disparo HEAT, Transformación energética, Aliento de dragón |
| Cazanieblas | ágil, DES/INT | Rifle de Viaje (nueva) | Disparo sencillo | Andanada, Vial de gas nervioso, Fuego de retirada | Bloqueo de visión, Tiro perforante, Disparo a todo gas |
| Almirante Yi | equilibrio, DES/CON | Sable Ligero de Milicia | Cuchillada de espada | Golpe en el tobillo, Golpe de la corriente marina, Doble marea | Bendición del Dragón de los Mares, Flota de guerra orgullosa, Oleada salvaje |

Lo nuevo del Arquero, que queda en el canon:

- **Arranque jugable**: si una de las tres primeras sale bloqueada (velocidad, una bomba sin
  cifras), entra la siguiente jugable (Cazador, Destructor, Espía).
- **El compañero**: el Lobo solar del Cazasoles es una invocación (`summons.json`, CON 2, DES 4,
  5 PV, como un lobo de tier 1) que trae *Curación natural* para todo el combate y ataca al
  cerrar cada turno tuyo; sus técnicas son las órdenes, cartas que suman la bonificación del
  lobo como una continuación. El Cazasoles empieza con la carta que lo trae.
- **El Ángel vengador** elige su energía una vez por combate (Llama, Hielo, Halo, Oscuridad o
  Sin elemento), como el Monje su espíritu: no lleva el +1.
- «Esquiva −X %» (punto débil al descubierto, gas nervioso) entra al diccionario: se defiende
  con desventaja.
- **El nombre del set**: el del *Servidor del cañón de fuego* es largo, así que su set y sus
  piezas son «del Cañonero» (`set_de`).

## 7. Las armas nuevas y cómo se cogen

Seis formas nuevas en `tools/actores_formas.py`, con su escalón raro (el de los sets, con la
gema como carga alquímica): escopeta, rifle, lanzagranadas, revólver, cañón de mano y bumerán.
Las de fuego se dibujan de lado (como se ven de perfil) y se ponen de pie para la maestra. Como
se cogen sale de los vídeos de NosTale (`actores_poses`): la escopeta, el rifle y el lanzagranadas
horizontales a la cintura, como la ballesta y el lanzacohetes; el **cañón al hombro**, con
volumen como el martillo (y al andar sigue al hombro); los **revólveres abajo junto a la
pierna**, apuntando al suelo; el bumerán en alto junto al costado. Van al final del atlas de
armas: ninguna fila de antes se mueve. `test_cada_arma_se_coge_como_en_nostale` lo comprueba.

## 8. Los doce sets del Arquero

En `tools/actores_trajes_arquero.py` (se registra detrás de los del Espadachín):

- **Cazador**: jubón de cuero, mangas y capucha verde azulado con una pluma, el pañuelo azul y la
  aljaba a la espalda con las flechas.
- **Asesino**: chaleco de cuero negro sobre la camisa gris azulada, la correa de los cuchillos,
  la faja roja de cabos largos y la capucha negra honda con el forro rojo.
- **Destructor**: chaqueta acolchada naranja con la bandolera de bombas alquímicas (la mecha
  encendida) y el pañuelo rojo atado a la cabeza con las gafas de taller.
- **Guardabosques**: túnica verde con el manto de hojas y el cuello de piel, astas de ciervo en
  la diadema y botas con borde de piel.
- **Cañonero**, como el *Fire Cannoneer* de los vídeos de Oriol: chaqueta de cuero de aviador
  con el cuello de borrego, la bufanda roja al viento y la correa cruzada, y el gorro de aviador
  con orejeras y las gafas de vuelo. (Las piezas pasan a llamarse *Chaqueta* y *Gorro de aviador
  del Cañonero*.)
- **Espía**: gabardina corta azul pizarra con el forro tostado, el halcón posado en el hombro y el
  gorro con dos plumas de halcón.
- **Cazademonios**: las piezas del cazador de demonios de SP-01 (sombrero de ala ancha, abrigo
  largo con forro rojo, botas con hebillas).
- **Ángel vengador**: placas blancas con el oro, el sol en el pecho, dos alas pequeñas a la
  espalda y el aro de oro con dos alitas en las sienes.
- **Cazasoles**: peto de cuero claro con el sol de oro, la piel de lobo en los hombros y el tocado
  de cabeza de lobo con el sol en la frente.
- **Granadero**: peto de acero sobre la camisa de campaña, la correa de granadas al pecho y el
  casco con la visera de cristal subida.
- **Cazanieblas**: gabardina larga pizarra con el forro oliva, los botes de gas del cinturón y la
  máscara de gas colgando al cuello, y la capucha honda.
- **Almirante Yi**: la armadura de Joseon azul tachonada de oro con el forro rojo, y el sombrero
  negro de ala ancha con la borla roja.

Lo que el set toma del catálogo (calzas, pantalones, botas, grebas) lo tiñe su tema. Dos piezas
salían idénticas a otras (los pantalones del Destructor a los del Romperrocas, las botas del
Granadero a las de cazador): el tema del Destructor tiñe la tela y el Granadero lleva botas
herradas de su acero. Probados en las cuatro tallas y en el motor.

## 9. Verificación (Arquero)

- `test_clases_sp` 40/40: las 24 clases y los 24 sets; ninguna carta de arranque bloqueada; la
  pasiva de +1 en todas menos el Monje y el Ángel; y por SP del Arquero: Ataque de falcón,
  Tambalearse (para y afila), Cura milagrosa (limpia y refuerza), Espíritu de lobo (al grupo),
  Emboscada, Hoja de tiro (cura lo que quita), la energía del Ángel (Hielo y Llama), *Curación
  natural* (el Lobo solar todo el combate, y primero limpia y después cura), Escudo energético
  (para y da CA; la Granada gravitatoria, pendiente), Bloqueo de visión y Doble marea.
- `test_catalog_content`: las invocaciones pasan a 204 (el Lobo solar); falla solo lo de antes.
- `test_validar_sprites` 40/41 (la de siempre) con lo nuevo de las armas; `gen_personajes` sin
  avisos; `test_personajes_capas` falla solo por pnj_boundington.
- Juego real: 87 clases en la cuadrícula y la hoja de Clase del Cazasoles (pasiva, 7 + 3 cartas,
  las Jabalinas de Milicia en el kit).
- Motor (`demo_brillo_equipo`): los 12 sets del Arquero puestos con su tinte y sus armas.

## 10. El Mago

El Arquero se aplicó en el Mac (las 41 preimágenes coincidían: 43 ficheros, 87 clases). Las doce
SP del Mago, con las mismas reglas (los datos, en `tools/sp_mago.py`):

| SP | Rol | Tira | Arma al empezar | Pasiva | Arranque | Creador (tier 2) |
|---|---|---|---|---|---|---|
| Mago de fuego | lanzador, INT/CON | INT | Vara de Aprendiz de Milicia | Pernos de fuego | Llamarada de fuego, Aliento de fuego, Transfusión Mana | Lluvia de meteoritos, Inferno, Ataque de los meteoritos |
| Mago sagrado | apoyo, INT/CAR | INT | Cetro de Bronce de Milicia | Ataque santo | Cura, Luz santa, Escudo mana | Arma santa, Bendición, Martillo santo |
| Mago azul | lanzador, INT/DES | INT | Orbe de Mano de Milicia | Bola de hielo | Fragmento de hielo, Destello de hielo, Lanza de hielo | Granizo fuerte, Frío, Tormenta de nieve |
| Mago de las tinieblas | ágil, DES/INT | DES | Pistola Mágica de Viaje (nueva) | Dispara el arma mágica | Tiro combinado, Rayo mágico, Guardián Fantasma | Poción maléfica, Réquiem, Invasión de los fantasmas |
| Volcano | equilibrio, INT/CON | INT | Bastón Ferrado de Milicia | Bola de magma | Erupción volcánica, Gas volcánico, Tanque de magma | Pantano de lava, Espada de magma, Tormenta de cenizas |
| Señor de las mareas | lanzador, INT/CAR | INT | Vara de Aprendiz de Viaje | Tridente del trueno | Megatormenta, El gran tridente, Bendición del océano | Tsunami, Burbuja de agua, ¡Cuidado, una ballena azul! |
| Vidente | lanzador, INT/DES | INT | Orbe de Mano de Viaje | Bola mágica | Caos, Jaula mortal, Parpadeo | Cambio de hoja, Pérdida de tiempo, Súperabracadabra |
| Archimago | lanzador, INT/CAR | INT | Bastón Arcano | Magia de la luz | Catalizador lumínico, Supresión, Prisma sacro | Teleportador en masa, Tormenta de meteoritos, Iluminación |
| Sacerdote vudú | lanzador, INT/CAR | INT | Cetro de Bronce de Viaje | Rayo maldito | Nube venenosa, Araña espiritual, Figura de espíritu | Figura de acuántropo, Absorción del alma, Maldición mortal |
| Grávitas | equilibrio, INT/CON | INT | Bastón Ferrado de Viaje | Telequinesis | Atracción, Empujar, Aplastar | Reflexión por antigravedad, Ingravidez, Agujero negro |
| Tormenta de fuego | equilibrio, DES/INT | DES (dos, INT) | Lanzallamas de Viaje (nuevo) | Infierno llameante | Infierno llameante reforzado, Círculo de fuego, Lanzamiento de vial de aceite | Haz de llamas, Reignición, Explosión infernal |
| Merlín | apoyo, DES/INT | DES | Pistola Mágica de Viaje | Rayo de luz | Destello luminoso, Velocidad de la luz, Rayo de luz salvador | Luz unificadora, Don de la naturaleza, ¡Aparece, Excalibur! |

Lo nuevo del Mago, que queda en el canon:

- **Con qué se tira**: la ficha de NosTale lo dice. «Ataque mágico» → la carta tira INT
  (`castingStat`, como las del Arcanista); «a distancia» → DES. Nueve SP son magia entera
  (`tira` en sus datos); el Mago de las tinieblas y Merlín disparan el arma mágica secundaria
  (DES); la Tormenta de fuego mezcla (su *Círculo de fuego* y su *Remolino llameante*, INT).
- **El rol `caster`** (4 PV, CON ×1,5, ligera / ligera: la plantilla de las clases lanzadoras del
  catálogo) para los que lanzan desde atrás; Volcano, Grávitas y la Tormenta de fuego van de
  equilibrio (el tanque de magma, la gravedad que atrae, el lanzallamas), el Mago sagrado y Merlín
  de apoyo, el Mago de las tinieblas ágil (dispara).
- **Escudos de maná** (uno por SP): el daño recibido −X % es +1 a la CA (+2 desde el 25 %);
  lo que pasa a los MP, fuera. Las bendiciones elementales de grupo, +1d6 al arma, como el
  *Espíritu de lobo*. *Terror leve* es miedo (`asustado`).
- **Invocaciones** sin cifras (el Tótem sagrado, las tres Arañas espirituales, los cuatro Clones
  oscuros): las de una criatura de tier 1 del bestiario y 1 turno; el Tótem, que solo apoya, sale
  `pendiente`. El tiburón, la ballena azul y la espada de magma, que atacan una vez, son cartas de
  daño.
- **Bloqueadas** (sin nada que jugar): *Tótem sagrado*, *Remolino gigante* (atrapar y frenar son
  posiciones) y tres del Archimago (*Carril de adelantamiento*, que solo sale el 5 % de las veces;
  *Teleportador en masa*; *Tormenta de meteoritos*, sin daño en la ficha). Ninguna de arranque.
- **El nombre del set**: sin artículo los propios (`Set de Volcano`, `Set de Grávitas`, `Set de
  Merlín`), con «la» los femeninos (`Set de la Tormenta de fuego`).

Los borradores de las doce los escribieron tres agentes (cuatro SP cada uno) con un encargo
cerrado (lo decidido por SP: rol, atributos, con qué tiran, arma, nombres de las piezas) y
`importar_clases_sp.py --borrador`; se revisaron y se juntaron en `sp_mago.py`.

## 11. La pistola mágica, el lanzallamas y los doce sets del Mago

Dos formas nuevas en `tools/actores_formas.py`, de lado como las del Arquero y con su escalón
raro: la **pistola mágica** (llave de latón, cañón fino con boca de campana y el cristal de maná
desde raras) se coge como el revólver, abajo junto a la pierna; el **lanzallamas** (el depósito
de aceite alquímico bajo el tubo y la llama piloto desde raras), como la escopeta, a la cintura.
Al final del atlas de armas.

Los sets, en `tools/actores_trajes_mago.py` (detrás de los del Arquero; 34 paletas propias sin
tipo):

- **Mago de fuego**: la corona de llamas y la túnica de SP-01.
- **Mago sagrado**: casulla blanca con la estola y la cruz de oro; mitra alta con la cruz (de
  espaldas, las ínfulas; de perfil, las dos puntas).
- **Mago azul**: túnica azul con hombreras de hielo con picos, cuello de piel y el bajo blanco de
  copos; capucha con ribete de piel y el cristal de hielo; botas de piel blanca.
- **Mago de las tinieblas**: levita negra con el forro violeta, botones de plata, chorrera y
  faldones; sombrero de copa con la cinta violeta.
- **Volcano**: coraza de basalto con las vetas de lava y hombreras con picos de obsidiana; yelmo
  con dos cuernos de obsidiana y puntas de lava.
- **Señor de las mareas**: escamas verde mar, hombreras de concha y la capa de agua; corona de
  coral y oro con perlas; el tridente de oro.
- **Vidente**: túnica violeta con estrellas de oro y la faja magenta con dos cartas; turbante
  con la joya del ojo; el orbe.
- **Archimago**: túnica blanca y oro con el cuello alto y el libro al cinto; el sombrero alto con
  la estrella; el grimorio.
- **Sacerdote vudú**: el pecho pintado de blanco con las correas, el collar de huesos y el
  faldellín de rafia; tocado de plumas verdes y rojas con la calavera.
- **Grávitas**: túnica azul noche con tres piedras que orbitan; el aro con la piedra que flota
  sobre la cabeza; la esfera.
- **Tormenta de fuego**: chaqueta ignífuga negra con bandas naranjas y el depósito a la espalda;
  casco de bombero de latón.
- **Merlín**: túnica blanca con el bajo de flores bajo el manto azul cielo con runas de oro;
  corona de flores; la pistola de luz.

De paso, un arreglo del Arquero: las alas del Ángel vengador solo salían de espaldas (fuera del
cuerpo la etiqueta es la vacía y se comparaba con «.»); ahora asoman de frente y de perfil.

## 12. Verificación (Mago)

- `test_clases_sp` 53/53: las 36 clases y los 36 sets; ninguna carta de arranque bloqueada; con
  qué tira cada carta (DES el Espadachín y el Arquero; INT los Magos salvo los del arma mágica y
  el lanzallamas); y por SP del Mago: el Aliento de fuego (con INT 1 no pasa la defensa; con INT
  5 quema cada ronda), Efecto cura y Cura de grupo, Cadena helada, Posesión (magia, DES),
  Tanque de magma, El canto de las sirenas, los cuatro Clones oscuros (atacan y se van), el
  Catalizador lumínico, las tres Arañas espirituales, Atracción y Reflexión por antigravedad, el
  Círculo de fuego (INT) frente al lanzallamas (DES) y Adivinación.
- `test_catalog_content`: las invocaciones pasan a 207; falla solo lo de antes.
  `test_cuaderno_creacion` cuenta las lanzadoras del catálogo (antes, 16 a mano) y
  `test_personajes_capas` comprueba la regla de la caché (presupuesto, o un solo atlas: el de
  armas pasa de 35 MB descomprimido); falla solo pnj_boundington.
- `test_validar_sprites` 40/41 (la de siempre), con cómo se cogen la pistola y el lanzallamas.
- Juego real: 99 clases en la cuadrícula y la hoja de Clase de Volcano (equilibrio, INT/CON, la
  pasiva, 7 + 3 cartas). Motor (`demo_brillo_equipo`): los 12 sets del Mago puestos con su tinte
  y sus armas, andando y en reposo.

## 13. El Artista Marcial

Las ocho SP del Artista Marcial, como las del Mago: los borradores de tres agentes con un encargo
cerrado (`ENCARGO.md`: las reglas del canon, lo decidido por SP y las piezas del set), revisados
con `importar_clases_sp.py --borrador` y juntados en `tools/sp_artista_marcial.py`.

| SP | Rol | Ficha | Lo propio |
|---|---|---|---|
| Puño del dragón (fuego) | equilibrio | DES/CON | los dos estilos en la pasiva (haetae: atacas con ventaja; dragón: +1 de fuego); Agarre, Garra ígnea y la Llama final (4d6) |
| Artes Místicas (agua) | ágil | DES/INT | Entrada de luz lunar y Posición floral (todo el combate), Eclipse lunar (marca y te oculta), Trampa de loto, Aguante (reacción) |
| Maestro Lobo (luz) | equilibrio | DES/CON | las variantes definitivas van con su técnica (pendiente); Estilo de combate definitivo: ventaja 2 turnos; Bloqueo (reacción) |
| Guerrero demoníaco (tinieblas) | ágil | DES/CON | Contraalas (reacción que devuelve los golpes), Cadenas sangrientas (cura lo que quita), Poderes demoníacos al grupo |
| Druida flamígero (fuego) | defensa | CON/DES | las tres figuras en la pasiva (druida +1, oso +1 CA, leopardo te defiendes con ventaja); la cadena Remolino → Salto de fuego → Gancho (4d6); Llama sanadora |
| Puño Hidráulico (tinieblas) | defensa | CON/DES | Tanquear, bloqueada (el combustible no se simula); el Protocolo cura 1d6; las continuaciones de los dos saltos suman |
| Tronador (luz) | equilibrio | DES/INT | dispara el cañón del guantelete izquierdo; el Amplificador de batería, bloqueado (las marcas); Onda eléctrica, Escudo de electrones |
| Thor (agua) | apoyo | DES/CAR | la sincronización en la pasiva (pendiente); Escudo helado (reacción); Trueno creciente: +1 al grupo todo el combate |

Todas tiran DES (cuerpo a cuerpo o a distancia: §7). Lo revisado de los borradores: la Bravata
del oso da +1 a la CA (lo seguro, no el 25 % de la mitad de las veces); la Trampa de loto lleva
dentro las Flores de la Semilla de loto (minas, no invocaciones). Los nombres de set nuevos:
`Set de las Artes Místicas` (femenino plural) y `Set de Thor` (nombre propio).

## 14. Los guanteletes y los ocho sets

Oriol eligió «Guanteletes grandes»: uno en cada mano. Es **una sola pieza** de la ranura `arma`
(a dos manos en sus restricciones) que se pinta en los dos puños (`actores_armas.capa_pareja`):
de frente y de espaldas, los dos encima del cuerpo; de perfil, el de delante encima de todo y el
de atrás solo sobre su mano, su brazo o fuera del cuerpo. Como de espaldas un arma va detrás del
cuerpo, la fila de los guanteletes en `personajes.json` trae **su propio orden de dibujo**
(`"orden"`, 80 en los cuatro rumbos): el motor lo lee (`FilaDeCapa::orden`) y `MontajePersonajes`
lo usa antes que el de la ranura; el probador y `validar_sprites`, también.

Cuatro formas nuevas, dibujadas a su tamaño (sin doblar ni pose), al final del atlas de armas:
los **guanteletes** (y su escalón raro: los nudillos de guarnición y la gema en el dorso), las
**garras** (la manopla de cuero o pelaje con tres garras; con su raro), los **brazos mecánicos**
(el pistón por fuera y un puño enorme) y el **cañón de descargas** del Tronador (en la derecha
el guantelete de las bobinas; en la izquierda, el cañón, que de perfil va en la mano de
delante). Los Guanteletes de Viaje son los normales de tier 1.

Los sets (`tools/actores_trajes_artista.py`, 32 paletas sin tipo):

- **Puño del dragón**: chaqueta de kung-fu roja (cuello de oro, botones de nudo, faja negra, el
  dragón bordado a la espalda); la cinta del Ninja en rojo; pantalones anchos negros atados en
  rojo y zapatillas de tela; guanteletes granates con los nudillos de oro.
- **Artes Místicas**: túnica blanca cruzada con la solapa azul y la faja de flores de loto; el aro
  de plata con la luna creciente; pantalones anchos azules; guanteletes azules con la gema rosa.
- **Maestro Lobo**: chaleco de piel gris oscuro sobre el pecho vendado, brazos vendados, faja
  blanca; el tocado de lobo del Cazasoles en gris; vendas en los pies; garras de plata.
- **Guerrero demoníaco**: coraza negra con pinchos rojos y alas de murciélago; diadema con la gema
  roja y los cuernos del Berserker en negro; botas con punteras rojas; garras rojas.
- **Druida flamígero**: jubón de cuero con la piel de oso en los hombros, las hojas rojas y el
  collar de colmillos; la cabeza de oso (orejas redondas, ojos de brasa); garras de brasa.
- **Puño Hidráulico**: peto de cuero y placas de hierro con el depósito de combustible; gafas de
  mecánico con lentes violeta; botas con punteras de hierro; los brazos mecánicos de bronce.
- **Tronador**: chaqueta amarilla con bandas blancas, bobinas de cobre y el cable cruzado; casco
  de latón con la antena del rayo; el cañón de descargas en la izquierda.
- **Thor**: placas de plata con el rayo de oro, hombreras azul hielo y la capa azul oscuro; el
  yelmo alado del Cruzado en plata; guanteletes azul hielo.

## 15. Verificación (Artista Marcial)

- `test_clases_sp` 62/62: las 44 clases y los 44 sets; todas tiran DES salvo las del Mago que
  tiran INT; la pasiva de +1 en todas menos las de forma (Monje, Ángel vengador, Puño del
  dragón, Druida flamígero); y por SP: el estilo dragón y el Agarre, Aguante, el Estilo de
  combate definitivo y la Patada con salto, Contraalas, la figura de oso y la Llama sanadora,
  Tanquear bloqueada y el Protocolo, la Onda eléctrica, el Escudo helado y el Trueno creciente.
- `test_personajes_capas`: los guanteletes quedan encima de las dos manos de espaldas (una
  espada, no); falla solo pnj_boundington, como antes.
- `test_validar_sprites` 41/42 (la de siempre), con los guanteletes en las dos manos en las
  cuatro tallas; `validar_sprites` y `validar_catalogos_rpg` sin fallos; las filas de antes de
  los atlas no cambian (las de PixelLab, detrás, como siempre). ctest: las mismas que antes.
- Juego real: 107 clases en la cuadrícula y la hoja de Clase del Tronador (pasiva, 7 + 3 cartas,
  los Guanteletes de Viaje en el kit). Motor (`demo_brillo_equipo`): los 8 sets puestos con su
  tinte, andando y en reposo en las cuatro direcciones.

## Pendiente

- Lo que Oriol diga de los sets del Arquero y del Mago, y de las armas de fuego (sin referencias
  suyas todavía, salvo el Cañonero de sus vídeos).
- Variaciones de los sets (otros tintes del mismo dibujo).
- El bastón y el tridente no tienen escalón raro (solo épico): los de los sets del Mago de fuego,
  Volcano y el Señor de las mareas son la forma de siempre con su tinte (la escalera de detalle,
  ESTETICA-01).
- Lo que Oriol diga de los sets y los guanteletes del Artista Marcial; si simulamos el
  combustible y las marcas (Tanquear y el Amplificador de batería salen bloqueadas).
- Las SP comunes.
