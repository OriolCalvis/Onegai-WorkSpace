# Decisiones resueltas — respuestas de Oriol del 28/09/2026

Generado por `build.py`. **La respuesta literal es de Oriol** y se extrae de donde él la escribió (la columna final del apartado 12 de cada `wf-*.md` y la columna «USUARIO DICE» de `DISCREPANCIAS_Y_DECISIONES.md`); aquí no se reescribe. La columna «Consecuencia en el plan» es el análisis de qwen: qué cambia y qué tareas lo recogen.

**51 cerradas** (Oriol respondió por escrito) · **2 asumidas** (dejó la pregunta en blanco y se aplica la recomendación diciéndolo en voz alta) · **3 abiertas** (sin respuesta: se planifica con la recomendación y siguen en su bandeja).

## Lo que necesita confirmación o respuesta

| ID | Estado | Qué se ha asumido | Por qué |
| --- | --- | --- | --- |
| D-COMB-02 | **asumida** | CRITICO = la mitad o mas de los dados son 6. El redondeo hacia arriba (1d6 con un 6 = critico) es la recomendacion aplicada porque Oriol dejo esa pregunta en blanco. Pifia sin cambio. Cambia DicePoolEngine y sus pruebas: primero el reglamento Ed. 4. | Oriol respondió en DISCREPANCIAS A2, pero dejó en blanco la aclaración posterior (casos límite): se aplica la recomendación y hay que confirmarla |
| D-COMB-05 | **asumida** | GameOver se queda como modo pero como ESTADO DE CONSECUENCIA (escena de noqueo o rescate, cicatriz y moral aplicadas, y se continua). Oriol escribio que es terminal; la compatibilidad con §10.6.B es la recomendacion aplicada, pendiente de confirmacion. | Oriol respondió en DISCREPANCIAS A4, pero dejó en blanco la aclaración posterior (casos límite): se aplica la recomendación y hay que confirmarla |
| D-MOV-05 | **abierta** | Sin respuesta. Se planifica con la recomendacion: bonificador a la defensa en vez de i-frames, que encaja en la enfrentada Ed. 3. | Sin respuesta en ninguna de las tres fuentes |
| D-MOR-09 | **abierta** | Sin respuesta. Se planifica con la recomendacion: cablear ya la lectura de factionRep en comercio y HUD, porque una magnitud que no se lee no se puede balancear. | Sin respuesta en ninguna de las tres fuentes |
| D-MIS-03 | **abierta** | Sin respuesta. Se planifica con la recomendacion: el rumor es un beat con TriggerType::Talk y condiciones. | Sin respuesta en ninguna de las tres fuentes |

## Las 56 decisiones, con su respuesta literal y su consecuencia

### WF-01 — Combate tactico: cartas, pool de d6 y caida sin game over

| ID | Estado | Respuesta literal de Oriol | Consecuencia en el plan | Tareas |
| --- | --- | --- | --- | --- |
| D-COMB-01 | cerrada | Ed. 3: cada **6 = 1 éxito**, cada **5 = 0.5**; los 4 solo suman con ventaja <br><sub>DISCREPANCIAS A1</sub> | Gana el reglamento Ed. 3: 6 = 1 exito, 5 = 0.5, los 4 solo con ventaja. Se corrige §10.6.A.4 del documento de vision. | `AV5-03` `AV1-04` `AV12-04` |
| D-COMB-02 | asumida | Crítico = La mitad del total de los dados  es 6. Pifia = **ningún** 5/6 **y** mínimo dos 1es. <br><sub>DISCREPANCIAS A2</sub> | CRITICO = la mitad o mas de los dados son 6. El redondeo hacia arriba (1d6 con un 6 = critico) es la recomendacion aplicada porque Oriol dejo esa pregunta en blanco. Pifia sin cambio. Cambia DicePoolEngine y sus pruebas: primero el reglamento Ed. 4. | `AV5-03` `AV12-04` `AV12-07` |
| D-COMB-03 | cerrada | `1d6 + Destreza` <br><sub>DISCREPANCIAS A3</sub> | Iniciativa SI: 1d6 + Destreza. Contradice el turno simple por bandos de BattleState, asi que se registra en el reglamento antes de tocar codigo y se reescriben las pruebas de turno. | `AV5-11` |
| D-COMB-04 | cerrada | Son las habilidades referenciadas por ejemplo en las clases referenciadas en discrepancias_y_decisiones <br><sub>wf-01-combate-tactico.md §12</sub> | La carta ES la habilidad referenciada por las clases (SkillDefinition), no una entidad nueva. Las 6 clases canonicas pegadas en el registro son la referencia de contenido. | `AV5-01` |
| D-COMB-05 | asumida | GameOver` existe y es **terminal**: «nada lo saca de ahí» + cicatriz afecta a moral <br><sub>DISCREPANCIAS A4</sub> | GameOver se queda como modo pero como ESTADO DE CONSECUENCIA (escena de noqueo o rescate, cicatriz y moral aplicadas, y se continua). Oriol escribio que es terminal; la compatibilidad con §10.6.B es la recomendacion aplicada, pendiente de confirmacion. | `AV5-07` `AV3-02` |
| D-COMB-06 | cerrada | Solo es cooldown dependiendo si es reposo corto o largo, si no esta indicado en una habilidad contaremos todas como largo <br><sub>wf-01-combate-tactico.md §12</sub> | Sin PA ni PM: solo cooldown ligado a reposo corto o largo. Si la habilidad no lo indica, cuenta como LARGO. | `AV5-02` |
| D-COMB-07 | cerrada | Vida fija + equipo para ampliarla <br><sub>wf-01-combate-tactico.md §12</sub> | Vida FIJA + equipo para ampliarla: el tope base no se dobla por tier. Recalcular healthCap y los tiers con ese criterio. | `AV12-04` `AV5-09` |

### WF-02 — Movimiento: locomocion, estamina, ruido y carga

| ID | Estado | Respuesta literal de Oriol | Consecuencia en el plan | Tareas |
| --- | --- | --- | --- | --- |
| D-MOV-01 | cerrada | locomoción continua con inercia, derrape de 0.4 s y multitudes + mover con el raton A* simplificado <br><sub>DISCREPANCIAS A10</sub> | CELDAS + A* + interpolacion visual y clic de raton. Oriol habia pedido continuo; al ver el coste (rehacer colision, IA, editor y pruebas) confirmo la opcion recomendada. Nueva tarea AV7-08. | `AV7-01` `AV7-08` `AV8-01` |
| D-MOV-02 | cerrada | La estamina se usaria para correr o nadar o volar cosas asi, saltar etc <br><sub>wf-02-movimiento-y-locomocion.md §12</sub> | Estamina nueva para correr, nadar, volar y saltar. Nace en el motor y se declara en el glosario. | `AV0-01` `AV7-01` `AV1-07` |
| D-MOV-03 | cerrada | carga en **kg** contra `Fuerza × 5 kg` <br><sub>DISCREPANCIAS A7</sub> | Peso en KG. Que CON haga de Fuerza es la recomendacion aplicada (Oriol dejo esa pregunta en blanco): bandas sobre CON x 5 kg. | `AV7-04` `AV0-01` |
| D-MOV-04 | cerrada | por calculo de tirada+ruido del sitio + si esta iluminado o sombreado <br><sub>wf-02-movimiento-y-locomocion.md §12</sub> | Ruido por calculo de tirada + ruido del sitio + si esta iluminado o sombreado. La iluminacion entra como factor de deteccion, no solo como estetica. | `AV7-07` `AV8-01` `AV8-02` |
| D-MOV-05 | abierta | —(sin respuesta)— <br><sub></sub> | Sin respuesta. Se planifica con la recomendacion: bonificador a la defensa en vez de i-frames, que encaja en la enfrentada Ed. 3. | `AV7-01` |

### WF-03 — Compras, ventas, regateo y hurto en mostrador

| ID | Estado | Respuesta literal de Oriol | Consecuencia en el plan | Tareas |
| --- | --- | --- | --- | --- |
| D-COM-01 | cerrada | Unificar con el nuevo sistema de moral: `ShopItem{price,stock,moralImpact,vendible,requiredLevel}` (solo lo usan sus tests <br><sub>DISCREPANCIAS B2</sub> | Unificar sobre ShopItem{price,stock,moralImpact,vendible,requiredLevel} y conectarlo con el nuevo sistema de moral. GameSession deja de mantener su ShopOffer minimo. | `AV4-01` `AV3-08` |
| D-COM-02 | cerrada | unificar la moral en multiples valores a tener en cuenta como he dicho anteriormente <br><sub>wf-03-compras-y-comercio.md §12</sub> | Moral unificada en multiples valores a tener en cuenta: un valor complejo, no un int suelto ni un float 0..1. | `AV0-01` `AV3-08` |
| D-COM-03 | cerrada | depende <br><sub>wf-03-compras-y-comercio.md §12</sub> | «Depende»: el precio de venta no es un 50% fijo universal; se concreta por tienda con precio base + dificultad de comercio/escasez. | `AV4-15` `AV12-03` |
| D-COM-04 | cerrada | poner un precio *0.5 su valor en tienda y cada tienda tenga precio base + dificultad de comercio/escasez de recursos y luego aplicar la unificacion de ShopItem en el precio <br><sub>DISCREPANCIAS B4</sub> | Venta a 0.5x del valor en tienda, precio base por tienda + dificultad/escasez, y unificacion de ShopItem en el precio. H2-08 sigue siendo el prerrequisito. | `AV4-01` `AV4-15` |
| D-COM-05 | cerrada | enfrentada <br><sub>wf-03-compras-y-comercio.md §12</sub> | Regateo = enfrentada de Carisma, reutilizando DicePoolEngine y visible en el HUD. | `AV4-02` |
| D-COM-06 | cerrada | Ambos pueden ser <br><sub>wf-03-compras-y-comercio.md §12</sub> | Ambos pueden ser: hurto sobre la mercancía fisica Y sobre el stock. La ranura fisica es el caso visible; el stock, el general. | `AV9-08` `AV6-12` |
| D-COM-07 | cerrada | ya te dije que pienso del tiempo <br><sub>wf-03-compras-y-comercio.md §12</sub> | Calendario global: ver D-NEG-01. Restock e impuestos al mismo reloj. | `AV10-02` |
| D-COM-08 | cerrada | delante <br><sub>wf-03-compras-y-comercio.md §12</sub> | «Delante»: se exige proximidad fisica. La apertura remota de E-04 queda solo para depuracion. | `AV4-01` |

### WF-04 — Moral, reputacion territorial y mutacion fisica

| ID | Estado | Respuesta literal de Oriol | Consecuencia en el plan | Tareas |
| --- | --- | --- | --- | --- |
| D-MOR-01 | cerrada | la moral personal explica tu cuerpo y tus diálogos; la reputación explica el trato de cada territorio <br><sub>wf-04-moral-reputacion-y-mutacion.md §12</sub> | Dos magnitudes: la moral personal explica tu cuerpo y tus dialogos; la reputacion explica el trato de cada territorio. | `AV3-03` `AV3-08` |
| D-MOR-02 | cerrada | unificar la moral en un valor complejo de mas de un factor <br><sub>wf-04-moral-reputacion-y-mutacion.md §12</sub> | Unificar la moral en un valor complejo de mas de un factor y darle profundidad. Oriol pide librerias que trabajen esos patrones complejos. | `AV3-08` `AV0-01` |
| D-MOR-03 | cerrada | bandas −19…+19 neutral y +20…+59 respetado +75 Heroe <br><sub>DISCREPANCIAS A9</sub> | Bandas del documento: -19..+19 neutral, +20..+59 respetado, +75 heroe. Cabo suelto: que pasa entre +60 y +74 (el doc dice Salvador); se propone mantener Salvador ahi. | `AV3-03` `AV1-01` |
| D-MOR-04 | cerrada | Como capas si <br><sub>wf-04-moral-reputacion-y-mutacion.md §12</sub> | «Como capas si»: la mutacion fisica se implementa por capas de sprite. AV3-01 pasa de solo-datos a capas. | `AV3-01` `AV3-02` |
| D-MOR-05 | cerrada | Poco a poco de momento la de los humanos y ya iremos ampliando <br><sub>wf-04-moral-reputacion-y-mutacion.md §12</sub> | Poco a poco: de momento la faccion de los humanos, e ir ampliando. Ni 3 ni 19 de golpe. | `AV3-03` `AV0-02` |
| D-MOR-06 | cerrada | Se degrada con los siglos sin interactuar <br><sub>wf-04-moral-reputacion-y-mutacion.md §12</sub> | Se degrada con los SIGLOS sin interactuar: decae, pero en escala de siglos, no de dias. | `AV3-03` |
| D-MOR-07 | cerrada | Las que sabe <br><sub>wf-04-moral-reputacion-y-mutacion.md §12</sub> | Solo las facciones que el jugador sabe: nada de tabla completa. | `AV1-01` |
| D-MOR-08 | cerrada | vincular el aumento por afinidad con su grupo, raza, estado o afinidad politica, mas tu moral mas pureza <br><sub>DISCREPANCIAS A8</sub> | El aumento de precio se vincula a la afinidad con su grupo, raza, estado o afinidad politica, mas tu moral y tu pureza. Multifactor: ni la formula lineal del motor ni las bandas planas del doc. | `AV3-08` `AV4-15` |
| D-MOR-09 | abierta | —(sin respuesta)— <br><sub></sub> | Sin respuesta. Se planifica con la recomendacion: cablear ya la lectura de factionRep en comercio y HUD, porque una magnitud que no se lee no se puede balancear. | `AV3-08` `AV4-01` |

### WF-05 — Negocios, propiedades, rentas e inversion urbana

| ID | Estado | Respuesta literal de Oriol | Consecuencia en el plan | Tareas |
| --- | --- | --- | --- | --- |
| D-NEG-01 | cerrada | El horario del mundo se debe actualizar aunque el jugador este quieto dando fases de dia/noche, donde se puede aprovechar para cambiar los patrones conductuales de los npc's y de los enemigos o mobs, incluso eventos algo mas peligrosos depende de que zonas de una ciudad se frequenten <br><sub>DISCREPANCIAS B6</sub> | Reloj de mundo en TIEMPO REAL que avanza aunque el jugador este quieto, con fases dia/noche que cambian patrones de NPC, enemigos y eventos peligrosos por zona. El contador de pasos se retira. | `AV10-02` `AV4-05` |
| D-NEG-02 | cerrada | Diaria <br><sub>wf-05-negocios-propiedades-y-rentas.md §12</sub> | Renta DIARIA (no semanal como dice §11.4 del documento). | `AV4-05` `AV10-02` |
| D-NEG-03 | cerrada | Todo lo que sea Comprable y una cosa construida que sea de alguien <br><sub>wf-05-negocios-propiedades-y-rentas.md §12</sub> | Propiedad = todo lo que sea comprable y toda cosa construida que sea de alguien. | `AV4-03` `AV6-10` |
| D-NEG-04 | cerrada | registro de eventos indirectos y directos del jugador hacia el beneficio de vida del npc <br><sub>wf-05-negocios-propiedades-y-rentas.md §12</sub> | Inquilino como registro de eventos indirectos y directos del jugador hacia su beneficio de vida, no como NPC con IA propia de entrada. | `AV4-05` |
| D-NEG-05 | cerrada | Mejora calidad de la infrastructura o la degrada <br><sub>wf-05-negocios-propiedades-y-rentas.md §12</sub> | La inversion publica mejora o degrada la calidad de la infraestructura: cambio real y medible, no solo fama. | `AV4-06` |
| D-NEG-06 | cerrada | Minijuego a ritmo <br><sub>wf-05-negocios-propiedades-y-rentas.md §12</sub> | Minijuego a ritmo (no tirada) para los oficios. | `AV4-07` `AV4-08` |
| D-NEG-07 | cerrada | Financian la prosperidad del pueblo y lo amplian si van superando ciertas quotas <br><sub>wf-05-negocios-propiedades-y-rentas.md §12</sub> | Los impuestos financian la prosperidad del pueblo y la amplian al superar ciertas cuotas: no son un sumidero. | `AV4-09` `AV12-03` |
| D-NEG-08 | cerrada | Si <br><sub>wf-05-negocios-propiedades-y-rentas.md §12</sub> | Si: se puede vender un negocio propio. | `AV4-03` |
| D-NEG-09 | cerrada | Todo, los muebles pueden subir su valor y reformarla tambien <br><sub>wf-05-negocios-propiedades-y-rentas.md §12</sub> | Todo cuenta: los muebles suben el valor de la propiedad y permiten reformarla. No solo el alquiler mueve la moral. | `AV4-04` `AV4-05` `AV3-08` |

### WF-06 — Equipo, ficha de personaje y progresion por hitos

| ID | Estado | Respuesta literal de Oriol | Consecuencia en el plan | Tareas |
| --- | --- | --- | --- | --- |
| D-EQU-01 | cerrada | la xp la podemos usar en afinidad a una cosa para dar progresion al juego, como oficios, fe o dominio de una clase, en el juego de mesa unicamente se plantean tiers de momento. <br><sub>DISCREPANCIAS A5</sub> | Tiers para el juego de mesa; la XP del documento se reutiliza como AFINIDAD (oficio, fe, dominio de clase). Progresion doble, no niveles. | `AV0-07` `AV5-09` |
| D-EQU-02 | cerrada | { CON, DES, INT, CAR }`, rango 0…8 dependiendo el tier <br><sub>DISCREPANCIAS A6</sub> | Cuatro stats {CON, DES, INT, CAR} con rango 0..8 dependiente del tier. Las cartas canonicas que dicen «Fue» se mapean a CON. | `AV0-01` |
| D-EQU-03 | cerrada | Sí <br><sub>wf-06-equipo-ficha-y-progresion.md §12</sub> | Si: las 3 opciones de carta al subir salen de learnableSkills de la clase. | `AV5-09` |
| D-EQU-04 | cerrada | carga en **kg** contra `Fuerza × 5 kg` <br><sub>DISCREPANCIAS A7</sub> | kg (con CON haciendo de Fuerza, ver D-MOV-03). | `AV7-04` `AV0-01` |
| D-EQU-05 | cerrada | Sí y efectos de aumentar caracteristicas <br><sub>wf-06-equipo-ficha-y-progresion.md §12</sub> | Si: la ropa tiene efecto social Y efectos de aumento de caracteristicas. | `AV3-01` |
| D-EQU-06 | cerrada | Si <br><sub>wf-06-equipo-ficha-y-progresion.md §12</sub> | Si: al subir de tier los PV se rellenan al tope nuevo. | `AV5-09` |
| D-EQU-07 | cerrada | Sí <br><sub>wf-06-equipo-ficha-y-progresion.md §12</sub> | Si: se expone la multiclass al jugador, en contra de la recomendacion. Habra que equilibrar dos mazos y el handLimit multiclass. | `AV5-10` |
| D-EQU-08 | cerrada | Entre combates es gratis, en un combate se penaliza con un turno para desaprendre y otro turno para aprender <br><sub>wf-06-equipo-ficha-y-progresion.md §12</sub> | Entre combates es gratis; DENTRO del combate cuesta un turno para desaprender y otro para aprender. | `AV5-10` |
| D-EQU-09 | cerrada | Sí <br><sub>wf-06-equipo-ficha-y-progresion.md §12</sub> | Si: existe degradacion del equipo. En contra de la recomendacion, entra con su coste: nueva tarea AV4-16. | `AV4-16` |

### WF-07 — Misiones organicas, investigacion y narrativa

| ID | Estado | Respuesta literal de Oriol | Consecuencia en el plan | Tareas |
| --- | --- | --- | --- | --- |
| D-MIS-01 | cerrada | Unificar y segregar por misiones principales, eventos, misiones secundarias, de professio, de raza, o de evento unico, como casarse tener un hijo etc. <br><sub>wf-07-misiones-investigacion-y-narrativa.md §12</sub> | Unificar y segregar por tipo: principales, eventos, secundarias, de profesion, de raza y de evento unico (casarse, tener un hijo). AdventureScript sigue siendo la unica fuente. | `AV10-01` `AV6-17` |
| D-MIS-02 | cerrada | Solo iconos de cosas en el mapa de momento <br><sub>wf-07-misiones-investigacion-y-narrativa.md §12</sub> | Solo iconos de cosas en el mapa, de momento: nada de marcadores de mision flotantes. | `AV10-03` `AV1-06` |
| D-MIS-03 | abierta | —(sin respuesta)— <br><sub></sub> | Sin respuesta. Se planifica con la recomendacion: el rumor es un beat con TriggerType::Talk y condiciones. | `AV10-03` |
| D-MIS-04 | cerrada | Depende, si es investigacion sola, inteligencia, si es conocimiento arcano carisma, etc <br><sub>wf-07-misiones-investigacion-y-narrativa.md §12</sub> | Depende de la prueba: investigacion sola = Inteligencia; conocimiento arcano = Carisma; etc. Hace falta una tabla prueba -> stat en el glosario. | `AV10-04` `AV0-01` |
| D-MIS-05 | cerrada | Se decide mision a mision que puede dar cada una <br><sub>wf-07-misiones-investigacion-y-narrativa.md §12</sub> | Se decide mision a mision que da cada una: no hay recompensa estandar. | `AV10-01` `AV10-05` |
| D-MIS-06 | cerrada | Depende, podria atacarte o irse o no querer hablar contigo nunca mas o acobardarse <br><sub>wf-07-misiones-investigacion-y-narrativa.md §12</sub> | Depende del mercenario: podria atacarte, irse, no querer hablarte nunca mas o acobardarse. Cuatro salidas, no dos. | `AV4-13` |
| D-MIS-07 | cerrada | El oro se cobra despues del encargo estar completado <br><sub>wf-07-misiones-investigacion-y-narrativa.md §12</sub> | El oro se cobra DESPUES de completar el encargo, no por adelantado. Corrige §10.11 del documento. | `AV4-14` |
| D-MIS-08 | cerrada | ya trabajaremos los caracters raros cuando todo lo demas este finalizado, no? <br><sub>DISCREPANCIAS A12</sub> | Se aplaza: los caracteres raros se trabajaran cuando todo lo demas este finalizado. Sigue la veda de tildes/ñ en los JSON de aventura. | `AV1-03` |
| D-MIS-09 | cerrada | Depende de su caracteristica si es de que han secuestrado a alguien, pueden caducarse, si es de una espada milenaria escondida en una cueva, pues seguiran alli por ejemplo <br><sub>wf-07-misiones-investigacion-y-narrativa.md §12</sub> | Depende de la naturaleza del rumor: los urgentes caducan (un secuestro), los atemporales no (una espada milenaria en una cueva). Hace falta un campo de caducidad por mision. | `AV10-01` `AV10-03` |

## Deuda de contenido aguas arriba: la solución que dio Oriol

| Deuda | Cifra | Efecto en los flujos | Solución de Oriol |
| --- | --- | --- | --- |
| Hechizos huérfanos | 202 | WF-01: cartas que no se pueden jugar | Darles valores utilizando las clases que te voy a referenziar: |
| Monstruos con vida 0 | 432 | WF-01: enemigos que no pueden combatirse | Poner valores y testear su dificultad por el daño maximo que podria hacer todas las clases referenciadas en grupos de 1 contra 1*n(siendo n=0 y cada bucle n++;) hasta un total de 10 enemigos, asi sabremos el daño maximo de cada clase y podremos trabajar mejor en el balanceo |
| Clases sin hechizos | 59 | WF-06: mazos vacíos | Habra que referenciarlas y modificar las habilidades para que encajen con el tono de las clases referenciadas, pero con variedad de opciones multiplicando las maneras de jugar |
| `lootIds` reales | 28 | WF-01/WF-06: botín casi nulo | habra que mirar como incorporarlo en la economia del juego |
| Precios de equipo generados | 413 (282 alcanzables) | WF-03: nada se puede vender | se debe poder vender, aunque cada npc tiene que tener una cantidad de dinero predeterminaa impidiendo que un vendedor pueda pagar cifraz exhorbitadas excepto quien si pueda pagarlas |

Las seis clases canónicas que Oriol pegó en la sección C del registro (Monje del Despertar, Señor de las Armas, Bardo de las 4 estaciones, Apóstata, Furia de los Creadores y Mago chasqueador) son la referencia de contenido para dar valores a los hechizos huérfanos y adaptar las clases sin hechizos. Van por `canon → cartas → motor`, no se inventan en el motor (AV12-05).

## Registro formal

Estas respuestas se registran como `decisiones/D0008-respuestas-workflows-diseno.md` (estado `accepted`: las decidió Oriol). Las tres abiertas y las dos asumidas siguen en la bandeja «Esperando a Oriol» del `TABLERO.md`.
