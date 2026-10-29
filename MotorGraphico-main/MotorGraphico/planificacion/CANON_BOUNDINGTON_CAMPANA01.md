# Boundington: las escenas de la Campaña 01, sacadas del vault (08/10/2026)

Hecho por Claude (Cowork, BND-ESCENAS-01) con la lista «Boundington: lo que hay que sacar del gestor
documental» del 08/10. Es la base para escribir las escenas en las aventuras `boundington_*.json`.

## 0. Fuentes y reglas

- **[C01]** `Onegai 2/Onegai/THE ONEGAI PROJECT/THE ONEGAI PROJECT/Campañas/Canon/Campaña 01_ La Matanza de
  Boundington (Aventura de Venides).docx` (03/10/2024). Texto en `Onegai 2/corpus_extraido/` (11/08/2026,
  posterior al docx: está al día).
- **[PART]** `.../THE ONEGAI PROJECT/PARTIDAS.docx` (08/09/2024), la parte «Campaña 01 (La Matanza de
  Boundington)» y el «Anexo de npc's / enemigos · Partida 01» (las fichas son imágenes del docx; las
  convirtió la revisión del 05/10).
- **[OBS]** Obsidian, `Onegai/OnegaiTimeLine/Campanyes/Canon/Aegroum/Campanya 1 Boundinghton/`: los
  capítulos 1 a 3 copian [C01]; los capítulos 4 y 5 son solo títulos; las fichas de **Llocs**,
  **Personatges** y cuatro de las cinco **Subquests** están **vacías (0 bytes)**. La precuela
  (`Precuela_Taberna_Cuatro_Encargos.md`, `estado: borrador-implementado`) la escribió otra IA para el juego.
- **[TML]** `OnegaiTimeLine/TML/.../ERA XIII`: `EV-1981-BOUNDINGTON` y sus vecinos están en `estat: esborrany`
  y solo tienen la definición de una línea; el llinatge de la Casa de Venides (Vendel, «nom provisional»)
  lo propuso otra IA. No se usa como canon para el juego.

Reglas que se siguen aquí: el texto entre «» es **literal**; lo que no está en el vault se marca
**NO ESTÁ**; si [C01] y [PART] no coinciden, van las dos. Las CD del d20 se pasan a dados del rival con la
tabla del juego: CD 5 a 8 = 1 dado, 10 a 12 = 2, 15 = 3, 18 a 20 = 4, 25 = 5.

Lo que el vault zanja de la lista (§6):

| Pregunta | Respuesta del vault |
|---|---|
| ¿Cómo se llama el Cap. 3? ¿Hay más? | «Capítulo 3: Boundington y los Perdidos» (la escena de la Taberna Humilde). Después: «Capítulo 4: La Investigación en Boundington», y sin número «Tercer Día: El Ocaso de Boundington», «El Ocaso de Boundington: La Gran Escapada», y las secciones «BASE MILITAR», «CASCO ANTIGUO» y «LAS AFUERAS» [C01]. El «Escape de Boundington» solo está en [PART]. |
| ¿Skilla, la Furia de Alaro y Gorran son del juego? | **No: están en [C01]**: «Personaje Clave: Skilla, la tabernera» (antigua aventurera, Berserker) y la «Subquest de Skilla: "Recuperar el Legado Perdido"» con la Furia de Alaro y Gorran, «líder de los bandidos». [PART] no los nombra. |
| ¿Los gitanos de Surysal son la Barriada? | Sí: «La Barriada de Surysal, conocida por ser el hogar de una comunidad gitana nómada» [C01]. Su líder es **Zarela**, «la líder espiritual»; el juego tiene en su lugar a «El viejo de Surysal» (**diferencia**). |
| ¿La dueña de la Taberna Humilde tiene nombre? | Skilla [C01]. |
| ¿Dónde está Xila por la mañana? | «la Taberna del Gremio de Ladrones en Pico Dragón por la mañana, donde encontrarán a Xila sobrio» [C01]. |
| ¿El Edul de Ranmont es nieto de Venides y Verina? | Sí: «Edul Hijo de Ranmont (Nieto de Venides y Verina)» [PART, anexo de la Partida 02]. |
| ¿Los hombres de Venides dependen de cuándo o de cuántas pruebas? | De **cuándo** [C01, PART]: pruebas suficientes antes del segundo día → 20 hombres y Punto de los Creadores; «Si descubren las pruebas el mismo día» → 10 y Punto de Sho-Noco; nada → solo. El juego los decide el primer día por cuántas pistas llevas. |
| Las monedas | Los precios usan pp, pb y po; el cambio entre ellas **NO ESTÁ**. |
| ¿Qué pasó en las Santas Montañas? | **NO ESTÁ.** Solo que Venides «después de volver de las Santas Montañas no volvió muy contento» [PART] y que sus amigos «han empezado a mostrar una clara convicción por una secta que está en auge en Perishton» [C01]. |
| ¿Qué dan los Puntos de los Creadores, de Sho-Noco y de Gurkazaal? | **NO ESTÁ.** Solo se nombran. |
| ¿El cuarto encargo de la precuela? | **NO ESTÁ.** La nota tiene tres (Luisarda, Ben Kafka, Griffin); habla de «Cuatro aventureros de la partida de rol». |

## 1. El Vacío y el relato (aventura `boundington_prologo`)

Ya están en el juego enteros. Lo único sin usar: «Los personajes deben presentarse entre sí» [C01], que el
juego resuelve con un solo héroe. Las CD: Percepción 12 [C01] (el juego, 2 dados) y Conocimiento Arcano 15
(3 dados).

## 2. La Taberna Humilde, 757 (inicio del primer día)

**T-01 · Ver a Venides**
- Fuente: [C01] Cap. 3; [PART] «Pasamos a Boundington…».
- Texto: «Se encuentran sentados alrededor de una mesa en la Taberna Humilde, con una bebida local delante de
  ellos. Venides está allí, aparentemente en medio de una misión importante.» [C01] · «con una pinta de una
  exquisita agua de lúpulo de la casa, como si una gran fuerza los hubiera materializado allí» [PART].
- Tirada: Percepción CD 5 (1 dado): «Éxito: Detectan la presencia de Venides, probablemente decidirán hablar
  con él.» El fallo **NO ESTÁ**.
- En el juego: Venides te reconoce si viste la barca (`beat_venides_reconoce`), sin tirada.

**T-02 · Las dos mesas de tres**
- Texto: «hay dos mesas más donde se sientan tres personas en cada una. Parecen ser personas locales, bebiendo y
  comiendo en compañía.» [PART] · «En aquel momento les explican su preocupación por el auge de los Perdidos en
  la ciudad, e incluso alguno de ellos puede sentirse visiblemente atraído por las ideas de estos, y que ello
  desemboque en un debate.» [PART]
- Los rumores literales y el debate **NO ESTÁN**. En el juego, un parroquiano (`beat_parroquiano`).

**T-03 · Skilla**
- [C01]: «Precios: Pasar la noche: 25 pp. Cerveza: 2 pb. Comida: 5 pp.» · «Skilla es una antigua aventurera
  (Berserker) que ahora regenta la taberna. Si los aventureros logran ganarse su confianza, Skilla les ofrecerá
  información clave y ayuda durante su aventura, incluyendo una subquest.»
- Confianza: «Gánate la confianza de Skilla tras interactuar con ella en la taberna o superando una tirada de
  Persuasión (DC 12)» (2 dados). «Ella les hablará del robo y de los bandidos que acampan cerca de Pico Dragón.»
- En el juego: cama a 25 de oro (el docx dice 25 pp) y la Furia de Alaro sin tirada.

## 3. El primer día (aventura `boundington_primer_dia`)

**D1-01 · El desayuno con Venides** (mañana)
- [C01] Cap. 4: «Por la mañana, Venides se reúne con los aventureros durante el desayuno. Mientras él se dirige
  a investigar la parte rica de la ciudad y la base militar, pide a los aventureros que visiten la zona
  comercial y obtengan información útil sobre los rumores de los Perdidos y cualquier otra pista.»
- [PART]: «Venides por la mañana desayunará con los aventureros y este les pedirá que visiten la zona
  comercial, que él empezará con la parte rica y la base militar.»
- Las palabras de Venides **NO ESTÁN**. En el juego, el encargo es del segundo día (`b2_amanece`).

**D1-02 · La herrería Lingote de Oro** (mañana, zona comercial · `interior_herreria` · Aigren, hijo de Ceaseton)
- [C01]: «Si los aventureros se muestran comprensivos con Aigren y superan una tirada de Carisma (DC 15), él
  revelará información sobre la adoración de los Perdidos y el asedio a la ciudad que está por venir. Sin
  embargo, se arrepentirá de hablar y cerrará la herrería abruptamente, usando la salud de su hijo como
  excusa.» · «Rumor Menor: Si la tirada de Carisma es de 10 o más, Aigren revelará pequeños detalles o rumores
  sobre el culto, como la existencia de un líder carismático entre los Perdidos. Si los aventureros vuelven
  después, la herrería estará cerrada.»
- [PART]: «Conseguir algo de información sobre la adoración del Herrero de los Perdidos, si te muestras
  comprensible y superas la tirada de Carisma de 15. Sabrás sobre el asedio a la ciudad, luego se arrepentirá
  y os sacará de la tienda y cerrará el comercio poniendo de excusa la salud de su Hijo. (Está fuerte)» ·
  «Si saca un 10 podemos darle algun pequeño detalle o rumor.» · «Si lo has visitado permanecerá cerrado.»
- Tiradas: Carisma 15 → 3 dados: éxito o crítico = el asedio; parcial (como «un 10») = el líder carismático;
  fallo = nada.
- Da: reparar equipo 5 po; «Venta de armas variadas». **NO ESTÁ** qué armas ni a cuánto.
- Diferencia: «(Está fuerte)» en [PART] —el hijo— contra «la salud de su hijo como excusa» [C01].
- En el juego: `beat_aigren`, sin tirada.

**D1-03 · La Boutique de Madame** (mañana · `interior_sastreria` · Luisarda, hija de Boundington)
- Tienda [C01]: «Pociones: 50 po. Libros de hechizos. Víveres y munición.» [PART]: «Munición entre lo típico.»
  La lista y los precios de lo demás **NO ESTÁN**.
- [PART]: «Luisarda recalcará con algo de miedo que no tiene antorchas desde la llegada de los Aventureros, si
  le preguntan al respeto ella evadirá la conversación poniéndose más nerviosa. Si superan una tirada por
  Intimidación o Engaño 10 les llevará a una sala secreta, pidiéndoles que no hagan ese tipo de preguntas y que
  la acompañéis. En la habitación que se abre ante ellos hay una mesa redonda y unas sillas donde se dará paso a
  la conversación. Luisarda les contará que ha venido un tal Sandes a por todas ellas, ya que existe un plan
  oculto para promover la palabra de los Perdidos. Que hay mucho ajetreo en Surysal y que en Pico Dragón hay un
  señor por las tardes que quizás por algo de bebida pueda contarnos más. Luisarda estará nerviosa y con miedo
  todo el tiempo y al finalizar la conversación les pedirá que por favor vayan con cuidado, y que no se
  inmiscuyan en los ajetreos de los Perdidos.»
- [C01] añade: la subquest «Las Sombras de las Antorchas» (Sandes las compró «para un ritual oscuro»; Xila sabe
  más; el ritual «El Encendido de la Luz Perdida» en la iglesia del Casco Antiguo).
- Tiradas: Intimidación o Engaño 10 → 2 dados. El juego no tiene Engaño como tirada propia del héroe distinta
  de Intimidación: se ofrecen las dos como opciones.
- Después: «Si lo has visitado: Te recordará con miedo y te dará 2 pociones gratis a cambio de tu silencio.»
  [PART] · [C01]: «Al volver, les dará 2 pociones gratis a cambio de su silencio».
- Diferencia: [PART] habla de «un señor por las tardes» en Pico Dragón; [C01] dice que el hombre «frecuenta una
  taberna por las tardes» y, en la subquest, que hay que ir «por la mañana» para encontrar a Xila sobrio.
- En el juego: `beat_luisarda`, sin tirada ni sala secreta.

**D1-04 · Reunirse con Venides o explorar Pico Dragón**
- [C01]: «Después de algunas pistas, los aventureros tendrán la opción de: Reunirse con Venides en la Taberna
  Humilde. Explorar Pico Dragón.» [PART]: «Luego ofreceré a los jugadores si reunirse con Venides en la Taberna
  Humilde o ir a explorar Pico Dragón.»

**D1-05 · Los sectarios del Casco Antiguo** (`ciudad_centro`, `sectario_perdido`)
- [C01]: «Los aventureros serán abordados por sectarios de los Perdidos (x3). Intentarán convencerlos de unirse
  al culto. Tirada de Carisma (DC 15): Éxito: Los sectarios informan de que buscan hombres fuertes para
  masacrar herejes. Si los aventureros muestran interés en unirse, les hablarán de su misión de forma cautelosa
  y abstracta.»
- [PART]: «si vas allí unos sectarios de los Perdidos(x3) querrán que te unas a su culto, si tiras por Carisma
  15 te informaran que buscan hombres fuertes para masacrar a los herejes.»
- Tirada: Carisma 15 → 3 dados. En el juego, `beat_sectarios` lo dice sin tirada («Buscamos hombres
  fuertes»).

**D1-06 · La guarida de los Perdidos** (tras los sectarios · la cueva bajo la iglesia: `interior_iglesia_s1`)
- Entrar: «Al intentar entrar a la guarida de los Perdidos, ellos te pedirán que les des todas tus armas. Si
  decides entrar obtendrás un Punto de Gurkazaal.» [PART]
- Dentro: «Al entrar, verás a muchos de ellos hablando entre susurros. Te llevarán hacia una escalera de caracol
  que desciende muy abajo. Una vez allí, se encontrarán en una cueva tallada a mano, donde está sucediendo una
  ceremonia oscura y lúgubre. En el centro de la habitación se alza la estatua de un ídolo de apariencia
  demoníaca y profana» [PART].
- Historia CD 10 (2 dados): «Si saca un 10: podemos descubrir que el de la estatua es su deidad demoníaca:
  Gurkazaal.» «Si no: permanecerá siendo un misterio.» [PART]
- Hablar con los Perdidos (Carisma; [C01] dice «Tirada de Carisma Menor a 10» / «Mayor a 10», 2 dados):
  - Pifia: «Se encuentra a Sandes. Este le puede hablar de muchas cosas de su pasado si muestra interés en
    unirse, sino muestra una total devoción, lo mandara a callar y después de amenazarla y tratarle mal, al
    borde de lo que parecería un combate, le mandara reprimiendo su ira al lado de Edul para que él le cuente
    cómo funcionará su nueva vida al servicio de los perdidos.» [PART]
  - Menos de 10: «Se encuentra perdido genérico. Este le mandará a sentarse en el banco de los novicios cuya
    mentora es Verina. Si Verina se da cuenta de que no son quien dicen ser, los mandara a Edul, quien estará
    dispuesto a colaborar y sacar de allí a los aventureros.» [PART]
  - Más de 10: «Se encuentra a Edul. Éste se verá dispuesto a colaborar y sacar de allí a los aventureros.»
- Salir: «Si por el contrario deciden unirse después de la ceremonia les darán una misión, de matar a gente,
  preparar barricadas, o extorsionar a los mercaderes. En caso de no aceptar el trato de los Sectarios, si se
  les habla bien y no se les molesta, se podrán ir sin problema. En caso de que enfaden a los sectarios, estos
  pelearán sí o sí, y hay mucha probabilidad de que llamen la atención de los guardias de la ciudad,
  provocando consecuencias bastante nefastas.» [PART]
- NO ESTÁ: cómo se da cuenta Verina (la tirada), qué dicen Sandes, Edul y Verina, la ceremonia, y las misiones
  del culto (es el gancho «de la próxima partida»).
- El juego tiene la iglesia (`beat_iglesia`, el nombre de Gurkazaal escrito con hollín) pero no la guarida.

**D1-07 · Pico Dragón: la niña y el gato** (`ciudad_oeste`, `nina_del_gato`)
- [PART]: «Los aventureros se encuentran con una niña, la cual su gato se ha quedado atrapado en un árbol. Si le
  ayudan a recuperar al animal, el padre de la niña les invitará a desayunar/comer/cenar (dependiendo del
  momento del juego). Conversarán alegremente con la familia en una suerte de casa a la luz de una hoguera.
  Esta acción contará como descanso corto.»
- La tirada para bajar al gato **NO ESTÁ** (la revisión del 05/10 propuso Atletismo). Los nombres de la niña y
  del padre **NO ESTÁN**.

**D1-08 · Pico Dragón: los Verdaderos**
- [PART]: «Los aventureros se encontrarán con un grupo de 5 Perdidos que han pasado demasiado tiempo en la
  taberna. Estos están increpando a un padre y su hija (los de la otra posibilidad), por ser extranjeros que
  han venido a vivir a la ciudad, llamándose a ellos mismos como los Verdaderos. Los aventureros pueden evitar
  la pelea en el caso de que se quieran unir a las amenazas y violencia en contra del padre y la hija, o en el
  caso de superar una tirada de 20 por Carisma. En caso de pelear y salir victoriosos, el padre les ofrecerá ir
  a desayunar/comer/cenar (dependiendo del momento del juego). [...] Esta acción contará como descanso corto.»
- [C01]: «Un grupo de Perdidos, en estado de embriaguez, increpa a un padre y su hija, a quienes llaman herejes.
  Tirada de Carisma (DC 20)».
- Tirada: Carisma 20 → 4 dados. Combate: 5 Perdidos (la ficha Cultist, «Devotos»).

**D1-09 · Pico Dragón: Xila** (mañana)
- [PART]: «Si van explícitamente por la mañana a Pico Dragón se encontrarán con Xila en esas horas aún no va
  borracho, si le ofrecen o regalan alcohol les hablara un rato sobre la historia de los Perdidos y de algunos
  rumores que ha oído en la Taberna Humilde del pueblo varias Lunas atrás.»
- [C01] (subquest de Luisarda): «Si los aventureros le ofrecen alcohol o superan una tirada de Persuasión (DC
  12), él revelará que los rituales de los Perdidos se están realizando en una iglesia medio derruida en el
  Casco Antiguo.» Persuasión 12 → 2 dados.
- Historia de Xila ([PART], anexo): mercader de Aegroum que pactó con los cultistas; le traicionaron; habló con
  las autoridades a cambio de su libertad; mataron a su familia en Killville; Venides lo rescató casi muerto
  de hambre y lo llevó en su carromato a Boundington.
- Aliado: «Si se alían con Xila, este les proporcionará ayuda durante la aventura, una vez hagan contacto con
  el, les esperará delante de la Taberna, dándoles pistas, para conseguir ropa cara.» [PART]
- En el juego: `beat_xila` a cualquier hora y sin alcohol.

**D1-10 · La noche: Venides borracho** (Taberna Humilde)
- [PART]: «Si en cambio van con Venides, este estará hablando solo sobre que le ha parecido ver a Edul por el
  pueblo, pero no puede ser, serán imaginaciones suyas, murmura interrumpiendo en algún momento la
  conversación, está borracho en la Taberna Humilde, si en ese momento interactuamos con él nos contarà que ha
  descubierto que por la noche se reúnen los altos cargos en la zona rica.»
- [C01]: «les contará que los altos cargos de la ciudad se reúnen en secreto por las noches en la zona rica. Se
  desbloquea la Noche de disfraces.»
- En el juego: `beat_venides_nada` dice lo de Edul pero no da la pista de la zona rica.

**D1-11 · La pelea de taberna** (si duermes en la Taberna Humilde)
- [PART]: «Si se quedan a dormir en la Taberna Humilde se encontrarán con una pelea de taberna. Si apalean, pero
  no matan al que la inicia, obtendrán pistas sobre el plan de los perdidos.»
- Quién la empieza, cuántos y qué pistas **NO ESTÁN**.

**D1-12 · La Taberna Rica**
- [C01] precios: «Pasar la noche: 25 po. Cerveza: 2 po. Comida: 5 po.» · «Si los aventureros interactúan con los
  clientes correctos, podrían obtener información sobre los movimientos de los Perdidos en los barrios ricos o
  sobre algún miembro de la nobleza que esté apoyando secretamente al culto.»
- [PART]: «Si por otro motivo intentan dormir en la Taberna Rica, tendrán que superar un 18 en Carisma, si lo
  hacen les darán una habitación gratis, si no tendrán que pagar el precio (No tienen dinero, les invitan a
  irse)». Carisma 18 → 4 dados.
- Dónde está en la ciudad **NO ESTÁ** (el juego tiene `interior_posada`).

**D1-13 · Investigar de noche**
- [PART]: «Si por otro lado intentan investigar: Tendrán la opción del Casco Antiguo, Base Militar o Barrio Alto.»
- Casco Antiguo: «no encontrarán nada extraño a parte de un carterista, supera la tirada de Percepción de 10 para
  que no te roben.» (2 dados). [C01] lo pone «Solamente llegar al casco antiguo» (sin hora). Cuánto roba **NO
  ESTÁ**.
- Base Militar: «no te dejarán pasar, y si lo consiguen, no encontrarán nada extraño, no hay topos dentro.»
- Barrio Alto: «Te increpara un vigilante nocturno y te dirá que por la noche no puedes pasear por ese barrio.
  Tirada de 15 de Sigilo (Vale con que 1 del grupo pase la prueba, si solo son 2 aventureros un 10 de Sigilo).
  Te acompañará a la taverna humilde y se concluyen las incursiones.» [PART] · [C01]: «Si fallan la tirada, el
  vigilante los detendrá e intentará expulsarlos del barrio. En ese caso, podrán intentar convencerlo con una
  tirada de Persuasión (DC 18) o encontrar otra ruta para entrar a la fiesta.»
- La fiesta: ver D2-05.

**D1-14 · El castillo**
- «Día 1: Bloqueado. No se permite el acceso sin autorización.» [C01] (en el juego, `beat_castillo_cerrado`).

## 4. El segundo día (aventura `boundington_segundo_dia`)

[PART] junta «Primer y segundo dia»: todo lo del capítulo 4 se puede hacer cualquiera de los dos días. El juego
lo reparte.

**D2-01 · El castillo y los hombres de Venides**
- [C01]: «Día 2: Si los aventureros han recopilado pruebas suficientes sobre los Perdidos, Venides estará
  preparado en el castillo con 20 hombres armados para enfrentar el ataque inminente (Punto de los Creadores).
  Si no descubren nada: Venides estará solo, y los guardias de la ciudad no estarán preparados para el ataque.
  Si descubren las pruebas el mismo día: Los aventureros pueden avisar a Venides, y él reunirá 10 hombres para
  el enfrentamiento (Punto de Sho-Noco).»
- Diferencia con el juego: allí se decide el primer día por cuántas pistas (`beat_venides_completo/parcial`).

**D2-02 · La cueva de Gorran** (subquest de Skilla) — [C01]: «Encuentro con bandidos en el camino a la cueva en
  Pico Dragón. Lucha contra Gorran, líder de los bandidos, quien posee la espada. Recuperar la espada y los
  documentos que revelan los planes de los Perdidos.» Recompensa: «Furia de Alaro (espada mágica con
  bonificadores de fuego) o una recompensa alternativa. Información crucial sobre los Perdidos y un próximo
  ataque en Boundington.» La ficha de Gorran **NO ESTÁ**. En el juego, entera.

**D2-03 · La tienda abandonada**
- [C01]: «La tienda abandonada se encuentra en los límites del Pico Dragón [...] Si los aventureros hablan con Xila
  o Venides, recibirán una pista sobre esta tienda [...] Tirada de Investigación (DC 12): Si superan la tirada,
  encontrarán trajes en buen estado: ropas de músicos, trajes de nobles excéntricos de Ascaria, o disfraces que
  podrían servir para pasar desapercibidos en la fiesta.»
- Los disfraces [C01]: «Músicos: Ropas sobrias, pero elegantes, adecuadas para un cuarteto o grupo de
  entretenimiento. Nobles excéntricos de Ascaria: Atuendos llamativos y ostentosos, con colores vivos y bordados
  exóticos, propios de una familia rica pero extravagante. Actores: Vestuario teatral con capas largas y
  detalles antiguos, perfectos para ocultar armas pequeñas y objetos.» «La elección influirá en cómo los
  percibirán los otros invitados.»
- En el juego, la tirada (2 dados) y dos de los tres disfraces, sin elegir.

**D2-04 · Surysal y la Barriada**
- Surysal [C01]: «Si los aventureros llegan a Surysal, notarán que la gente se está preparando en masa para algo
  grande. Hay actividad en cada esquina: forjadores trabajando el metal, grupos de ciudadanos entrenándose con
  armas improvisadas, y el inconfundible murmullo de una revuelta que está a punto de comenzar. Al explorar la
  zona, encontrarán pruebas evidentes de que el ataque a Boundington es inminente.» [PART]: «Si después de eso
  vas a Surysal alli veras y encontraras varias pruebas de que el ataque es inminente».
- La Barriada [C01]: «Tirada de Persuasión o Carisma (DC 12): Si los aventureros interactúan de manera
  respetuosa con los gitanos y superan esta tirada, ganarán la confianza de Zarela, la líder espiritual de la
  comunidad.» Luego: «proteger a su gente de unos saqueadores locales [...] un grupo de 6 saqueadores», el
  ritual de fuego (Interpretación DC 10) y la recompensa: guerreros en la batalla final, «un amuleto gitano que
  puede protegerlos de maleficios», y «leyendas y conocimiento ancestral sobre Gurkazaal y los Perdidos».
- En el juego: la niña te lleva a los gitanos; habla «El viejo de Surysal»; una tirada de Persuasión (3 dados) y
  ya. **Diferencias:** Zarela, los 6 saqueadores, el ritual y el amuleto.

**D2-05 · La fiesta del Barrio Alto («Noche de disfraces»)**
- Entrada [C01]: «se celebrará una fiesta privada donde la élite de la ciudad se reunirá, incluyendo varios
  seguidores de los Perdidos [...] necesitarán ropa adecuada, algo que pueden conseguir con la ayuda de Xila o
  Venides.» La mansión: «una gran mansión de tres pisos, propiedad de uno de los comerciantes más influyentes de
  Boundington. Las luces doradas inundan el jardín, y las risas y conversaciones se escuchan desde la
  distancia. Los sirvientes van de un lado a otro con bandejas llenas de copas de vino y exquisitos
  aperitivos.»
- El vigilante: ver D1-13 (Sigilo 15 → 3 dados; si falla, Persuasión 18 → 4 dados, o «otra ruta»).
- Escuchar desde fuera: Percepción 10 (2 dados). [PART]: «Conseguirás llegar al lado de una casa donde oirás el
  ruido de una fiesta, pero por encima del ajetreo, unas palabras que te llamen la atención. Si superas una
  tirada de Percepción 10. Con ello podrás espiar la conversación, allí hablarán de muertes, y los perdidos.»
  [C01]: «captarán fragmentos de diálogo sobre una revuelta que está siendo planeada por los Perdidos. Rumores
  captados: "Los Perdidos se están preparando para algo grande..." "Dicen que los mercaderes estarán fuera
  cuando todo ocurra, no quieren involucrarse..." "La revuelta comenzará en dos días, pero nadie sabe
  exactamente cómo será."»
- Trepar: Atletismo o Acrobacias 15 (3 dados). Si falla, 1d4 [PART]: «1. Se te rompe el traje por la parte del
  culo. 2. Se te rompe el traje por la parte de las axilas. 3. Se te rompe el traje por la espalda. 4.
  Ligeramente roto*.» «*Esto solo supondrá un inconveniente si hablan con el matrimonio propietario de la
  finca.» [C01] lo explica: «Esto podría causar una situación embarazosa si alguien lo nota» · «No será tan
  evidente a menos que los aventureros hagan movimientos amplios» · «Podría pasar desapercibido por un tiempo,
  pero cualquier giro revelaría el daño» · «El traje se rasga de manera insignificante, pero podría pasar
  desapercibido si se actúa con naturalidad».
- Disfrazado: Interpretación 15 (3 dados) [C01, PART].
- Dentro [PART]: «las informaciones a obtener serán como mucho, que en dos días habrá una revuelta en la ciudad y
  que para esas fechas todos estarán fuera de la ciudad, que los cultistas se reúnen mañana por la noche cerca
  del casco antiguo y que ellos no saben en exactitud como se va a producir dicha revuelta y “que es malo para
  los negocios saber demasiado, en según qué temas, ya me entienden” disuadiendo la respuesta concreta.»
  [C01] añade los grupos (nobles, mercaderes, seguidores: «La noche de la liberación está cerca.»), los
  documentos de las habitaciones de arriba (Sigilo 15) y engañar a los invitados (Engaño o Persuasión 15).
- Los nombres del matrimonio y del comerciante **NO ESTÁN**.
- En el juego: solo por Intimidación ante el vigilante y con el disfraz.

**D2-06 · Alternativas** — [PART]: «usar a Edul o Verina, un cobrador de tributos, misiones en el puente
  principal». [C01]: el cobrador «En el Barrio Alto, un cobrador de tributos corrupto podría interrumpir a los
  aventureros, pidiéndoles una "contribución" para pasar.» Sin más.

## 5. El tercer día y el Ocaso (aventura `boundington_ocaso`)

**O-01 · El tercer día** — [PART]: «Saldrás de la taberna y podrás explorar la zona [...] los eventos pueden ser
  vistos todos ese mismo dia si no lo has hecho aún, menos el de la Barriada ni tampoco los soldados de apoyo de
  Venides.»

**O-02 · La plaza del Casco Antiguo** — [PART]: «Si en cambio has descubierto todo sobre el plan, verás a Sandes,
  Edul y Verina(Cultista Superior x 3) entre los Perdidos (x5) que se hallan predicando en la Plaza del Casco
  Antiguo.» El sermón **NO ESTÁ**. Qué pasa si atacas **NO ESTÁ**.

**O-03 · Avisar a Venides** — [PART]: «Da igual la hora del día, empieza la parte del Ocaso, la diferencia es el
  momento del día, las consecuencias es huir.»

**O-04 · Las campanas** — [C01] (literal, ya en el juego en `b3_campanas` y las arengas): «Venides, con la mirada
  endurecida, reúne a sus hombres. "¡Aventureros, debemos escapar de aquí! Si caemos, todo el reino caerá con
  nosotros. Preparaos para lo peor. Esto es solo el principio…"»

**O-05 · Las rutas** — [C01]: oeste «(Recomendada)», norte «(Alta Peligrosidad) [...] La supervivencia es casi
  imposible», quedarse «(Alta Peligrosidad)». [PART]: «encontraremos fuertes llamas y peligros demasiado grandes
  si no huimos por el oeste». Con la Barriada, «en el mapa de las afueras tendremos pocas dificultades»; con los
  guardias, «estos podrán retener un poco a los Perdidos en el distrito militar».

**O-06 · Base Militar: saqueadores** — [C01]: «3 Perdidos Saqueadores [...] CA: 14 · HP: 22 cada uno · Ataque con
  espada corta (+4 al ataque, 1d6+2 daño) · Emboscada: ventaja en su primer ataque si logran sorprender (Sigilo
  DC 12).» (En el juego, con Venides.)

**O-07 · Verina retenida** — [PART]: «Verina estará retenida por otros dos Cultistas. Está acusada de traicionar
  al Culto pasando información a las autoridades de la ciudad». Con Venides: «él la reconocerá, y pelearás
  automáticamente. Si matas a los Cultistas que tienen a Verina, la liberarás, cerrando la misión una vez te
  encuentres con Edul en Las Afueras. Verina te lo pedirá de hecho, y se unirá a tu grupo si le prometes cumplir
  esa misión. +1 Afecto de Venides.» Sin Venides: «Percepción superior a 8» (1 dado) para reconocerla «de la
  visión que les había mostrado Sho-noco»; «decidirán si pelear para liberarla [...] o simplemente dejarla
  allí». [C01]: «No Liberar a Verina: [...] Verina será ejecutada por los cultistas.»
- En el juego: el rescate sin la promesa y sin el +1 de Afecto.

**O-08 · Los fanáticos** — [C01]: «2 Fanáticos del Culto [...] CA: 13 · HP: 18 cada uno · Daga (+3 al ataque, 1d4+2)
  o Hechizos (Nivel 2 conjuradores) · Explosión de Fuego (nivel 2): 3d6 de daño · Escudo de Fe: Da +2 CA a un
  aliado durante 1 minuto.» En el Casco Antiguo, los mismos con «HP: 28».

**O-09 · Sandes** (Casco Antiguo) — [PART]: «Si rescatamos a Verina: Este nos pedirá que la entreguemos al culto
  ganandonos la lealtad de los perdidos · Si vamos con Venides: Discusión + posible combate. · Si se alían con
  Sandes, les pedirá que matemos a Edul y/o Verina. Venides huirá en caso de estar con ellos y no lo volveremos a
  encontrar. Sho-Noco explicará después la huida. · Si no rescatamos a nadie: Nos intentará captar para el culto
  pero nos pedirá que demostremos nuestra lealtad matando a dos personas.(Edul y Verina). · Si no seguimos sus
  planes deberemos combatir sí o sí, escapará al quedarse a 0 de vida.» [C01]: «Sandes huye siempre que vaya en
  contra de los intereses de Sandes» y su ficha: «Brujo/Chain Devil Combinado · CA: 16 · HP: 105 (Resistencia a
  daño no mágico, Vulnerabilidad a daño radiante) · Cadenas Animadas (2 ataques): +7, 2d6+4 · Rayo Abrasador ·
  Maldición de la Cadena · Armadura de Agathys · Aplastar con Cadenas (una vez, 4d6) · Defensa Infernal».
  Con él, «dos Sectarios del Culto» (CA 12, HP 20, espada corta +3, 1d6+1; «Convicción Fanática: Una vez por
  combate, pueden sumar +5 a una tirada de ataque fallida») y dos Fanáticos «Mientras el se va en otra
  dirección». Las palabras de Sandes **NO ESTÁN**.

**O-10 · Las Afueras: la barricada** — [PART]: «Tirada de Percepción 7: Al seguir los gritos, se encuentran con una
  barricada la cual está en llamas y bastante destrozada, y pueden llegar a darse cuenta de que pueden acabar de
  destruirla mediante fuerza bruta (arriesgándose a obtener daño por fuego), o a distancia mediante armas o
  hechizos que puedan llegar a hacerlo. En caso de no conseguir la tirada de percepción [...] deberán dar más
  vuelta de la esperada y enfrentarse con un número mayor de enemigos». Percepción 7 → 1 dado. El daño del fuego
  **NO ESTÁ**.

**O-11 · Venides retenido** (si no le avisaste) — [PART]: «este estará siendo retenido por 3 Guardia, allí podrás
  ayudarlo a escapar para redimirte, o intentar acabar con él, pero como contamos antes Venides siempre huye en
  cuando su vida llega a 0. Aquí veremos como Venides pasa un mal rato y los cultistas están intentando
  secuestrar y/o obtener información del reino tras sus conocimientos. Si aquí le ayudamos y lo acompañamos
  posteriormente a las afueras de la ciudad, la misión también contará como desenlace POSITIVO. En cambio si no
  ayudas a Venides, Sho-Noco nos contará como él escapó».

**O-12 · La lealtad de Venides** — [PART]: «Si hemos completado toda la información a Venides, nos ganaremos su
  lealtad, y podremos acompañarlo en sus aventuras y desventuras de una forma más cercana. Esto nos proporciona
  fondos para comer y alojarse, reconocimiento y soporte del ejército Real. La enemistad de los Perdidos.
  Conocimientos del reino de Aegroum.» La escala del Afecto **NO ESTÁ** (solo el «+1 Afecto de Venides»).

**O-13 · Edul retenido** — [PART]: «Nos encontraremos a Edul hijo de Perishton al salir de las Afueras siendo
  retenido por 3 Guardias. Rescatarlo nos dará paso a la misión de rescate de Verina. Sandes los ha engañado y
  arrastrado en sus malvados intereses para obtener poder.» · «En caso de haber encontrado y reconocido a Verina
  en el Barrio Militar, pero estas aliado con Sandes, Edul se pelea con los aventureros en un ataque de ira (lo
  cual le dará ventaja en sus tiradas de resistencias y ataque, añadiendo un +5 en su CA)». [C01] (Las
  Afueras): «Si los aventureros no han rescatado a Verina, Edul les suplicará que vayan a salvarla. Si han
  rescatado a Verina, ella les suplicará que la ayuden a rescatar a Edul.» · «Si los aventureros no lucharon
  contra Sandes en el Casco Antiguo, lo encontrarán aquí, en Las Afueras, luchando contra Venides.»

**O-14 · Xila en el Ocaso** — «Xila acompañará a los aventureros, ayudándoles con algo de curación 1d4 si se ven en
  grandes dificultades.» [PART]; [C01]: «Venides, Xila y los aventureros que logren sobrevivir serán los únicos
  que logren escapar con vida de la ciudad.» (En el juego, `acompanantes`.)

**O-15 · Escape de Boundington** — [PART], literal: «Una vez terminadas las decisiones nombradas anteriormente, los
  aventureros serán succionados de nuevo hacia donde todo había empezado, el bote en el que Sho-Noco navegaba en
  unas aguas invisibles. También yacían ellos allí, ahora conscientes del embrujo al que habían estado
  sometidos. Sho-Noco habla entonces. Les explica que Venides logró escapar (en el caso de que ellos mismos no
  lo hayan visto), y les cuenta que corrió por las calles de las afueras hasta dejar atrás la ciudad,
  dirigiéndose al Norte. Desde los Campos del Otoño hasta Nefaria. No les cuenta mucho más, pues los aventureros
  necesitan descansar, se sienten somnolientos y caen uno a uno en un profundo sueño.»
- En el juego, `b3_epilogo` (Sho-Noco), sin la huida de Venides.

## 6. Personajes ([PART], «Anexo de personajes»)

| Quién | Lo que dice el vault |
|---|---|
| Venides, hijo de Nefaria | «Caballero de Aegroum recién nombrado. Jóven y afable, musculoso y carismático. En contra de los pensamientos de los Perdidos. Fiel a la Familia Real y a sus gentes.» Ficha: Soldier (16 PV). «Venides siempre huye en cuando su vida llega a 0». |
| Sandes, hijo de Killville | «Comerciante. Trabaja ayudando a su padre en el Mercado Central de Killville. Tiene curiosidad por los ideales de los Perdidos aunque no lo muestre públicamente. Esconde ansias de poder debajo de una careta de buena persona gracias a su labia. Está en contacto con ciertas organizaciones secretas que existen en Killville. Codicioso pero prudente.» |
| Edul, hijo de Perishton | «Trabaja en la forja de su pueblo por obligación de su padre, pero su verdadera pasión es la Alquimia, la cual practica en secreto. Se le conoce como a un patoso debido a su poca arte con la forja, lo cual provoca en él un odio interno [...] Su hermana Verina le entiende y consigue aplacar ese odio [...] Es tímido y reservado, pero se muestra amistoso con los suyos.» |
| Verina, hija de Perishton | «Es una chica alegre y optimista. Sus buenas formas se han ganado la estima de todo su pueblo, y trabaja en la taberna local. Le vuelve loca el conocimiento y lee todo libro que llega a sus manos. Las escrituras de los Perdidos son un estímulo para ella y su imaginación, y se siente secretamente atraída por ellas.» |
| Xila, hijo de Killville | Ver D1-09. «Si lo salvamos en Boundington, se convertirá en un hombre de honor, aún no ha abandonado su vieja costumbre de beber hasta quedarse dormido, pero ayudará a Venides y a los aventureros en todas sus dificultades siempre que se le permita.» |
| Aigren, hijo de Ceaseton | Herrero del Lingote de Oro; un hijo. Nada más. |
| Luisarda, hija de Boundington | «Madame» de la Boutique. Nada más fuera de D1-03. |
| Skilla | T-03. |
| Zarela | D2-04. |
| La niña del gato y su padre | «extranjeros que han venido a vivir a la ciudad» [PART]. Sin nombres. |
| Los Verdaderos | Los 5 Perdidos borrachos de D1-08: «llamándose a ellos mismos como los Verdaderos». |
| Sin nombre en el vault | El vigilante nocturno, el matrimonio dueño de la finca, el carterista, el que empieza la pelea, el cobrador de tributos, el capitán del castillo. La Regidora Gorlak **no sale** en la campaña. |
| Del juego, sin vault | El parroquiano, el guardia del castillo, el tendero, el joyero, el sastre, Ben Kafka y Griffin (precuela), «El viejo de Surysal». |

## 7. Lo que piden al motor (y qué escena lo pide)

| Pieza | Escenas |
|---|---|
| Franjas del día | D1-01 (mañana), D1-03 (el señor «por las tardes»), D1-09 (Xila «por la mañana»), D1-10 a D1-13 (la noche), D1-07/08 (comer según la hora) |
| Huir a 0 PV y pelear sin matar | D1-11 («apalean, pero no matan»), O-09 (Sandes «escapará al quedarse a 0»), O-11 (Venides «siempre huye») |
| Tabla aleatoria | D2-05 (el 1d4 del traje) |
| Descanso corto | D1-07, D1-08 (la comida con la familia) |
| Refuerzos | D1-06 («llamen la atención de los guardias»), O-10 (más enemigos si fallas) |
| Dejar las armas | D1-06 |
| Pruebas de grupo | D1-13 («Vale con que 1 del grupo pase»): con un solo héroe no hace falta |
| Monedas | T-03, D1-12 (pp, pb, po) |
| Fuego y barricada; los mapas del Ocaso | O-05, O-10 (los mapas `base_militar_GM.png`, `casco_antiguo_GM.png` y `las_afueras_GM.png` están en `infologia/Mapas/PARTIDA 1/`) |

## 8. Preguntas para Oriol (lo que el vault no zanja)

1. Los hombres de Venides: el docx dice **cuándo** (antes del segundo día, 20; el mismo día, 10). ¿Cambiamos el
   juego a eso, ahora que hay franjas?
2. ¿«El viejo de Surysal» pasa a ser **Zarela**? ¿Y la Barriada con sus 6 saqueadores, el ritual de fuego y el
   amuleto?
3. Las monedas: ¿1 po = 10 pp = 100 pb?
4. ¿Qué pasó en las Santas Montañas? ¿Y qué dan los Puntos de los Creadores, de Sho-Noco y de Gurkazaal?
5. Lo que no tiene texto: los rumores de las dos mesas, las palabras de Sandes, Edul y Verina (en la guarida y en
   el Ocaso), quién empieza la pelea de taberna y qué pistas da, cuánto roba el carterista, el sermón de la
   plaza, el daño del fuego. ¿Los escribo yo con lo que hay, o los escribes tú?
6. ¿El camino del culto (sus misiones) se juega aquí o es el gancho de la próxima partida, como dice el docx?
7. La precuela: ¿cuál es el cuarto encargo?
8. **La fiesta: ¿primera o segunda noche?** Los docx la ponen la primera; el juego, la segunda (ver §10.1).
9. **Sandes**: su ficha de [C01] es de 105 PV y CA 16 («Brujo/Chain Devil»). Contra un héroe de Tier 1 con Venides
   no es un combate, aunque huya a 0 PV. ¿Con cuánta vida y con qué acompañantes? Tampoco tiene sprite.
10. **Edul y Venides retenidos por «3 Guardias»**: hace falta la ficha de enemigo del guardia (hay sprite, `guardia`).
11. **El daño del fuego** de la barricada y la Furia de Alaro como objeto (espada mágica con fuego: ¿qué hace?).
12. **La Taberna Rica**: ¿es la Posada del viajero de la plaza?

## 9. En el juego: el primer día (BND-ESCENAS-01, 08/10, Claude)

Lo que ya está en `assets/adventures/boundington_primer_dia.json` (78 beats; los de la plaza y el tutorial siguen
igual). Las fichas de arriba son la fuente; aquí, cómo quedan y qué decidí yo donde el vault no llega.

| Ficha | Beats | Cómo queda |
|---|---|---|
| Llegada | `beat_llegada`, `b1_reloj` | Empieza el «Día 1 · Mañana» (el HUD lo pone junto al nombre de la zona). Una partida guardada de antes llega sin reloj: `b1_reloj` lo pone en marcha al primer tick. |
| T-03 Skilla | `beat_skilla_primera`, `beat_skilla_espada`, `b1_skilla_hora` | La cama y la Furia de Alaro como antes. Después, Skilla es el reloj: «Comer algo mientras pasa la mañana» (5, la comida de [C01]) pasa a la tarde; «Esperar a que anochezca»; «Subir a dormir» (siempre de noche). |
| D1-02 Aigren | `b1_aigren` y sus tres salidas | Persuasión contra 3 dados (Carisma 15). Éxito: la adoración y el asedio, se arrepiente y cierra «usando la salud de su hijo». Parcial («un 10»): el líder carismático. Después, cerrado. |
| D1-03 Luisarda | `b1_luisarda`, `b1_luisarda_sala`, `b1_luisarda_pociones` | Intimidación o Engaño contra 2 dados. Éxito: la sala secreta y lo que cuenta en [PART]. Al volver, las 2 pociones por el silencio (efecto nuevo `darObjeto`). |
| D1-05 Sectarios | `b1_sectarios` y salidas | Persuasión contra 3 dados: «hombres fuertes para masacrar a los herejes». «Decir que te interesa»: hablan cauto y te invitan a la cripta, sin armas. |
| D1-06 Guarida | `b1_guarida_puerta` … `b1_guarida_perdida` | En la cripta de la iglesia (`interior_iglesia_s1`): dejar las armas (Punto de Gurkazaal), la ceremonia y el ídolo (Gurkazaal con 2 dados), hablar con ellos: pifia Sandes, parcial Verina, éxito Edul; siempre acaba con Edul. Salir con él, unirte (otro Punto de Gurkazaal) o plantarles cara (3 Devotos, y se avisa a la guardia). Es una prueba, como la iglesia. |
| D1-07/08 Pico Dragón | `b1_gato`, `b1_verdaderos`, `b1_familia_mesa_*` | Por la mañana, el gato (Atletismo, 2 dados). Por la tarde o la noche, los Verdaderos: sumarte, calmarlos (Persuasión, 4 dados: Carisma 20) o plantarles cara (5 Devotos). Si se salva a la familia, a desayunar, comer o cenar según la hora: descanso corto. |
| D1-09 Xila | `b1_xila_manana`, `b1_xila_tarde`, `b1_xila_noche` | Por la mañana, sobrio: un trago o Persuasión (2 dados, DC 12 de [C01]). Por la tarde, solo con un trago. De noche duerme la borrachera. Lo que cuenta: [PART] (los rumores de la Taberna Humilde de hace varias lunas) y [C01] (la iglesia medio derruida). |
| D1-10 Venides borracho | `b1_venides_borracho` | De noche: Edul («serán imaginaciones mías») y los altos cargos que se reúnen de noche en la zona rica. |
| D1-11 Pelea de taberna | `b1_pelea` y salidas | Al subir a dormir. Sin matar (`sinMatar`). Si le apaleas, habla. |
| D1-13 Investigar de noche | `b1_carterista`, `b1_cuartel_noche`, `b1_vigilante` y salidas | Casco Antiguo: el carterista (Percepción, 2 dados). Base Militar: nada, «no hay topos dentro». Barrio Alto: el vigilante (Sigilo, 3 dados; si te para, Persuasión con 4 dados o te acompaña y se acaban las incursiones). |
| D1-14 Castillo | `beat_castillo_cerrado` | Igual. |
| Venides y los hombres | `beat_venides_completo`, `beat_venides_parcial` | 20 hombres: una prueba (la iglesia o la guarida) y **tres testimonios** (la variable `pistas`: Aigren, Luisarda, los sectarios, Xila, Edul). Antes eran la iglesia, Aigren y Luisarda, sin tiradas. |
| Cierres | `cierre_*` | Igual, y dormir es un descanso largo. |

### 9.1 Lo que decidí yo (para que Oriol lo mire)

1. **El reloj lo lleva el jugador** con Skilla (comer, esperar a la noche, dormir). No pasa el tiempo solo.
2. **La niña del gato o los Verdaderos** («la otra posibilidad», [PART]): por la hora. Por la mañana el gato; los
   Verdaderos «han pasado demasiado tiempo en la taberna», así que por la tarde o la noche. Si no bajas al gato, la
   familia sigue ahí y por la tarde se la encuentran los Verdaderos.
3. **Las tiradas de Carisma son de Persuasión** (el héroe no tira atributos sueltos). **Historia → Conocimiento
   Arcano** para el ídolo: no hay Historia en `skills.json`.
4. **El parcial** (empate) cuenta como fallo donde el docx tiene dos salidas, salvo donde da un término medio (Aigren:
   «un 10») o es reconocer algo fácil (el gato, el carterista, el ídolo).
5. **Precios y cantidades que no están**: el trago de Xila, 1; lo que roba el carterista, 10; la comida de Skilla, 5
   (la de [C01]). Las monedas siguen siendo una sola (pregunta 3).
6. **Quién empieza la pelea de taberna**: un Devoto de los Perdidos (por eso «obtendrán pistas sobre el plan de los
   Perdidos»). Lo que suelta junta dos hechos del vault: «masacrar a los herejes» (los sectarios) y «la revuelta
   comenzará en dos días» (la fiesta).
7. **La guarida**: dejar las armas solo se cuenta (el motor aún no las quita; el combate de dentro se pelea con
   ellas). Verina se da cuenta sin tirada (no está cómo). Plantarles cara: 3 Devotos, y la flag
   `bnd1_guardias_avisados` queda para las «consecuencias bastante nefastas», que no están. Unirse al culto deja
   `bnd1_unido_al_culto` y nada más (pregunta 6).
8. **El vigilante la primera noche**: si pasas, ves que en la mansión se prepara algo para mañana por la noche
   (`bnd1_fiesta_sabida`): la fiesta es del segundo día en el juego.
9. **La Taberna Rica** (D1-12) **no está**: no hay una taberna rica en el mapa (¿es la Posada del viajero?).
10. **Puntos de Gurkazaal**: la variable `puntos_gurkazaal` (+1 al bajar sin armas, +1 al unirte). Qué dan sigue
    siendo la pregunta 4.

### 9.2 Textos míos (a revisar)

Los hechos son del vault; las palabras de estos beats son mías: `b1_skilla_hora`, `b1_aigren` (y `_asedio`, `_lider`,
`_nada`, `_cerrado`), `b1_luisarda` (y `_calla`, `_pociones`; la sala sigue a [PART] casi palabra por palabra),
`b1_sectarios` (y sus salidas), `b1_xila_*`, `b1_gato*`, `b1_verdaderos*`, `b1_familia_*`, `b1_venides_borracho`,
`b1_guarida_*`, `b1_charla_*`, `b1_edul`, `b1_carterista*`, `b1_cuartel_noche`, `b1_vigilante*` y `b1_pelea*`.


## 10. En el juego: el segundo día y el Ocaso (BND-ESCENAS-01, 08/10, Claude)

**El segundo día** (`boundington_segundo_dia.json`, 47 beats):

| Ficha | Beats | Cómo queda |
|---|---|---|
| Amanecer | `b2_amanece`, `b2_reloj` | «Día 2 · Mañana». Venides da además la pista de la tienda abandonada ([C01]: «Si los aventureros hablan con Xila o Venides, recibirán una pista sobre esta tienda»). |
| Skilla | `b2_skilla_hora` | El reloj, como el primer día. «Subir a dormir» cierra el día (`b2_cierre_todo` o `b2_cierre`, con descanso largo). Antes, el cierre pedía haber resuelto los gitanos y la fiesta; ahora se puede dormir sin hacerlo, y en el Ocaso cuenta lo que no hiciste. |
| D2-01 Castillo | `b2_castillo_20`, `_10`, `_solo` | Lo que dice [C01] del segundo día: Venides en el castillo con 20 hombres, con 10, o solo y la guardia sin preparar. |
| D2-02 Gorran | igual | Sin cambios (la Furia de Alaro sigue siendo una flag: no hay ficha de equipo para ella). |
| D2-03 Tienda abandonada | `b2_tienda_abandonada`, `b2_disfraz_hallado` | Investigación contra 2 dados (DC 12). Los tres disfraces de [C01] para elegir: músico, noble excéntrico de Ascaria o actor. |
| D2-04 Surysal | `b2_surysal` | Al entrar en Surysal, el texto de [C01]: es una **prueba** del ataque (vale para el informe a Venides). Los gitanos: la tirada pasa de 3 a 2 dados (DC 12 de [C01]). |
| D2-05 La fiesta | `b2_barrio_alto_dia`, `b2_fiesta_puerta` … `b2_sonsaca_fallo` | De día solo se ve la mansión preparándose. De noche, [C01] entero: el vigilante (Sigilo, 3 dados; si te para, Persuasión con 4, otra ruta por los jardines o irte), la mansión, escuchar desde las ventanas (Percepción, 2 dados: los rumores), trepar (Atletismo, 3; si fallas, el 1d4 del traje con las cuatro notas de [C01]), entrar disfrazado (Interpretación, 3); dentro, los grupos (nobles, mercaderes, seguidores) y lo de [PART] («es malo para los negocios saber demasiado...»), y una de dos: las habitaciones de arriba (Sigilo, 3: los documentos de la revuelta) o sonsacar a los invitados (Engaño, 3; con el traje roto, 4). |
| Informe | `b2_venides_noche_todo`, `b2_venides_noche` | Con los papeles de Gorran y la fiesta, el texto de antes; con cualquier otra prueba (Surysal, los documentos de arriba), uno corto. |

**El Ocaso** (`boundington_ocaso.json`, 27 beats; primera parte):

| Ficha | Beats | Cómo queda |
|---|---|---|
| Reloj | `b3_plaza`, `b3_reloj`, `b3_campanas` | La plaza es la mañana del tercer día; las campanas suenan de noche («empiezan esta noche», dice Venides). |
| O-07 Verina | `b3_verina_salvada` | Liberada con Venides: +1 Afecto de Venides (`afecto_venides`). Se une si le prometes buscar a Edul (`bnd3_promesa_edul`); si no, se va sola. |
| O-10 La barricada | `b3_barricada_tras_verina`, `b3_barricada`, `b3_barricada_rota`, `b3_rodeo`, `b3_rodeo_hecho` | Antes de salir: Percepción contra 1 dado (Percepción 7). Si la ves, a golpes o desde lejos; si no, el rodeo y dos Perdidos más (perderlo es caer). |
| O-15 Escape | `b3_epilogo_caido` | Si caíste (no viste huir a Venides), Sho-Noco lo cuenta con [PART]: las afueras, el Norte, de los Campos del Otoño a Nefaria. |

**Falta del Ocaso** (la siguiente parte): Sandes (O-09), Edul y Venides retenidos (O-11, O-13), el daño del fuego
de la barricada, los mapas del máster (Base Militar, Casco Antiguo, Las Afueras) y los tres Cultistas Superiores
con cinco Perdidos en la plaza.

### 10.1 Lo que decidí yo (segunda tanda)

1. **La fiesta es la segunda noche**, como en el juego y en el plan del 05/10. En los dos docx es la primera noche
   (Venides borracho «desbloquea la Noche de disfraces»; [PART] la pone en «investigar» esa misma noche). Por eso el
   rumor literal «La revuelta comenzará en dos días» queda **«La revuelta empieza mañana»**, y de lo que se saca
   dentro según [PART] («en dos días habrá una revuelta») se deja fuera el «en dos días»; lo de los nobles («la
   noche siguiente cerca del Casco Antiguo») sí cuadra. La primera noche, el vigilante deja ver cómo se prepara.
2. **Una sola acción dentro de la fiesta** (arriba o sonsacar) y te vas.
3. **El disfraz no cambia las tiradas** ([C01]: «influirá en cómo los percibirán», sin decir cómo).
4. **Lo que hay en los documentos de arriba** es lo que [C01] pone en «Éxito total» (los detalles del ataque, los
   nombres, el sitio de la reunión), sin nombres concretos, que no están.
5. **El rodeo de la barricada**: dos Perdidos («un número mayor de enemigos»).
6. **Dormir el segundo día ya no pide** haber resuelto los gitanos y la fiesta.

## 11. Las alternativas y el karma (Oriol, 08/10; MORAL-01 y BND-ALT, Claude)

Oriol contestó el 08/10 en el documento «Boundington: alternativas, karma y moral» (Claude Docs). Lo que
quedó zanjado:

- **Los seis números** de la moral (peso 2/5/12, repetición 100/50/25/0 %, tope ±10 por noche, resistencia,
  asentar al dormir y marcas): sí. Van en `assets/rules/moral.json`; los actos, en `assets/rules/actos.json`
  (ver `FORMATO_AVENTURAS.md`, «La moral: el libro del día»).
- **Siete tramos**: Villano (−75), Desgraciado (−60), Canalla (−20), Neutral, Respetado (+20), Salvador (+60) y
  Héroe (+75). Héroe: halo grande y alas celestiales con motas de luz; Villano: halo grande y alas demoníacas
  con motas de sombra.
- **Pureza**: aparte de la moral; mide lo que te ha tocado lo profano.
- **Sho-Noco** es una representación más de Chronos, pero los aventureros quizá nunca lo sepan: se descubre en
  una misión del Tier 4. En Boundington no se dice.
- **Si Verina muere aquí**, otra mujer ocupa su puesto, y Edul se convierte en un hombre atormentado, sin ganas
  de vivir ni esperanza.
- **El camino del culto** se desbloquea al terminar Boundington, para bien o para mal: unirse a los Perdidos ya
  no da misión el segundo día.
- **«Éxitos, a la letra»**: «superar 2 seises» es una tirada de 2 éxitos fijos, sin rival (`skillCheck.exitos`).
- **Skilla pasa a llamarse Rosita**: una antigua exploradora de Tier 5, muy famosa en el lugar, que lo esconde;
  te pide un par de favores antes de revelar nada de su fuerza, su historia o que fue aventurera.

### 11.1 El primer día, con las alternativas (BND-ALT-01)

| Escena | Beats | Cómo queda |
|---|---|---|
| T-03 Rosita | `beat_skilla_primera`, `beat_skilla_primera_luisarda`, `b1_rosita_luisarda` | Al cobrarte la cama, el primer favor: ir a ver a Luisarda. Si Luisarda acaba en la taberna, Rosita da las gracias (2 pociones) y sube `confianza_rosita`. El id sigue siendo `skilla`. |
| D1-02 Aigren | `b1_aigren`, `b1_aigren_hijo`, `b1_aigren_perdidos`, `b1_aigren_se_va`, `b1_aigren_asedio`, `b1_aigren_redimido`, `b1_aigren_lider`, `b1_aigren_nada` | Dos temas. Por su hijo: **A** convencerle de que lo saque (Persuasión, 2 éxitos: el rumor del líder, y se va esa noche, `bnd1_aigren_se_va`) o **B** presionarle (Intimidación, 2 éxitos: el asedio, y se lo cuenta a los Perdidos, `bnd1_aigren_delata`). Por los Perdidos: **C** convencerle de que son el mal (Persuasión, 4 éxitos: forja sus armas, `bnd1_aigren_redimido`) o **D** decirle que te interesan (la invitación «de parte del herrero», `bnd1_invitado_por_aigren`). Parcial: el líder; pifia: nada. |
| D1-03 Luisarda | `b1_luisarda`, `b1_luisarda_animada`, `b1_luisarda_sala`, `b1_luisarda_pociones`, `b1_luisarda_ida` | Intimidación o Engaño (2 dados, como antes) o **C** Persuasión de 4 éxitos. En la sala, **A** prometerle silencio (al volver, las 2 pociones; `bnd1_luisarda_silencio`) o **D** mandarla con Rosita (`bnd1_luisarda_con_rosita`). **B**, delatarla, se hace delante de Sandes (`b1_charla_sandes`, `bnd1_luisarda_delatada`). |
| D1-05/06 La guarida | `b1_guarida_puerta`, `b1_guarida_por_aigren`, `b1_guarida_exterminada`, `b1_edul` | Con la invitación de Aigren se entra «de parte del herrero», con las armas y sin charla: sacar a Verina (`bnd1_verina_salvada_guarida`, la deja con Rosita), sonsacar para Venides (una pista y `bnd1_sabe_barricadas`), unirte o intentar acabar con todos (4 Devotos y 2 Fanáticos; si ganas, `bnd1_guarida_exterminada`). Con Edul, además, sembrar dudas en Verina (`bnd1_dudas_verina`). Unirse ya no da misión: «cuando esto acabe», y sabes dónde irán las barricadas. |
| D1-07/08 Pico Dragón | `b1_familia_mesa_*`, `b1_familia_despues_barriada`, `b1_familia_despues_cobrada` | En la comida, llevarlos a la Barriada al amanecer (`bnd1_familia_en_barriada`) o, si les defendiste de los Verdaderos, cobrarle al padre (10 de oro, `bnd1_familia_cobrada`). |
| D1-09 Xila | `b1_xila_desconocido`, `b1_xila_resentido`, `b1_xila_manana`, `b1_xila_tarde`, `b1_xila_habla_sobrio`, `b1_xila_habla` | Primero es un desconocido que llora en un rincón: sentarte con él (te cuenta su historia) o tratarle con desprecio (`bnd1_xila_maltratado`: no se une). Después, **A** sin alcohol (Persuasión por la mañana: aliado y hombre de honor, `bnd1_xila_honor`) o **B** con un trago (la misma pista y la historia de los Perdidos, `bnd1_xila_borracho`: el Ocaso lo pasa borracho). |
| D1-10 Venides borracho | `b1_venides_borracho` | Contarle lo de Edul (+1 Afecto, `bnd1_venides_sabe_edul`), callártelo (`bnd1_edul_callado`), confiar en él (+1 Afecto) o desconfiar (`bnd1_venides_desconfianza`). |
| D1-11 La pelea | `b1_pelea_apaleado`, `b1_pelea_rosita` | Hacerle hablar (la pista de siempre), entregarle a la guardia (`bnd1_capitan_te_conoce`) o rematarle: Rosita te para («si quieres matarlo, sácalo de aquí vivo») y lo matas en la calle o lo dejas. |
| D1-13 El carterista | `b1_carterista_pillado` | Dejarle ir (`bnd1_carterista_amigo`: en el Ocaso, un paso por el Casco Antiguo) o quedarte lo que lleva (10 de oro). |

Cada elección con peso apunta su acto (los 29 del primer día de `actos.json`). Lo que leerán el segundo día y el
Ocaso son las flags de la tabla; sus escenas son BND-ALT-02 y BND-ALT-03.

### 11.2 Lo que decidí yo (primer día)

1. **Los hombres de Venides, por cuándo** (hecho en BND-ALT-02, D2-01 A): con rumores el primer día, Venides
   espera pruebas (`bnd1_venides_espera`, cierre `cierre_espera`); si llegan ese mismo día, 20.
2. **A de Aigren da el rumor del líder, no una pista**: salvar a la familia es lo que gana; B y C dan la pista.
3. **El trago de Xila da una pista, no dos**: «cuenta más» es la historia de los Perdidos (`bnd1_pista_historia`)
   para más adelante; así no cambia cuántas pistas hacen falta para los 20 hombres.
4. **Despreciar a Xila no cierra la pista**: un trago lo acepta de cualquiera. No se une.
5. **El primer favor de Rosita es Luisarda**, y el segundo, la Furia de Alaro (el segundo día).
6. **De parte del herrero no se dejan las armas** (sin el Punto de Gurkazaal de la puerta) y no hay charla con
   Sandes, Verina o Edul: el herrero responde por ti.
7. **Los actos de intención van en la opción** (presionar, despreciar, cobrar) y los de resultado en el beat del
   resultado (convencer, bajar el gato, calmar).

### 11.3 Textos míos (a revisar)

Las líneas nuevas de Rosita, Aigren (A–D), Luisarda (C, la sala), la entrada de parte del herrero, Verina (las
dudas, sacarla), Xila (su historia contada por él, con las palabras de Oriol), el padre de la niña, Venides
borracho, la pelea y el carterista son mías. Lo que cuenta Xila sigue a Oriol: tratos con los Perdidos, la
traición, su familia muerta y él culpable, la prisión y la huida a una ciudad que no conoce.

### 11.4 El segundo día, con las alternativas (BND-ALT-02)

| Escena | Beats | Cómo queda |
|---|---|---|
| D2-01 Los hombres de Venides | `beat_venides_parcial`, `cierre_espera` (primer día), `b2_venides_noche*`, `b2_venides_hombres`, `b2_castillo_10` | **Por cuándo llevas las pruebas**: el primer día, 20 y Punto de los Creadores (`beat_venides_completo`, también después de los rumores); el segundo, el informe de la noche da 10 y Punto de Sho-Noco; sin informe, Venides solo. |
| D2-01 El capitán | `b2_capitan`, `b2_capitan_convencido`, `b2_capitan_no` | Al entrar en el castillo: Persuasión (3 dados) o, si le entregaste al camorrista el primer día, sin tirada. Convencido: `bnd2_guardia_preparada` (menos saqueadores en la Base Militar). |
| D2-02 Gorran | `b2_gorran_victoria` | El combate es sin matar: Gorran cae de rodillas y eliges dejarle huir (`bnd2_gorran_perdonado`: sus bandidos, contra los Perdidos en el Ocaso) o acabar con él (acto nuevo `gorran_rematado`, −medio). La Furia de Alaro va a la mochila. |
| D2-02 Rosita | `b2_skilla_espada_devuelta`, `b2_rosita_revela`, `b2_rosita_verina`, `b2_rosita_verina_arriba` | Devolverle la espada (+1 de confianza), devolvérsela y contarle todo (+2 y +1 Afecto de Venides: «se lo diré a Venides») o decirle que no la encontraste (`bnd2_rosita_desconfia`: no te ayuda en el Ocaso). Con confianza 2, cuenta que fue exploradora; con 3, habla de Verina (o de la Verina que ya duerme arriba). |
| D2-04 Zarela | `b2_familia_barriada`, `b2_gitanos`, `b2_gitanos_familia`, `b2_zarela_pide`, `b2_saqueadores_*`, `b2_ritual*` | Zarela en lugar del viejo de Surysal. Si llevaste a la familia del gato, te recibe sin tirada. Pide ayuda contra 6 saqueadores de Surysal; si ganas, sus guerreros estarán en el Ocaso (`bnd2_guerreros_gitanos`) y el fuego del ritual (Interpretación, 2 dados) da el amuleto gitano y las leyendas de Gurkazaal (parcial: solo el amuleto). |
| D2-04 B | `b2_sectarios` | A los sectarios del Casco Antiguo: «la Barriada se arma» (`bnd2_barriada_delatada`: los gitanos no llegarán a la huida). |
| D2-05 La fiesta | `b2_dentro`, `b2_invitados`, `b2_sonsaca_exito`, `b2_robo_bien`, `b2_robo_pillado` | Dentro: arriba, mezclarte con los invitados o irte. Entre los invitados: sonsacar (con el traje roto, más difícil), robar (Sigilo, 3 dados: 40 de oro; si te pillan, el matrimonio de la finca te denuncia) o irte. Si sonsacas, el noble que brinda: señalárselo a Venides (`bnd2_noble_senalado`) o chantajearle (30 de oro). |
| D2-06 El cobrador y Edul | `b2_fiesta_puerta`, `b2_cobrador`, `b2_edul_fiesta` | A la puerta: el cobrador (pagar 20, denunciarle al vigilante o amenazarle: cualquiera te deja llegar a la mansión) o, si conoces a Edul, entrar con él como seguidor (dentro directamente) y hablarle de Venides (`bnd2_edul_sabe_venides`). |
| La noche | `b2_venides_noche_todo`, `b2_venides_noche` | Contárselo todo (+1 Afecto, `bnd2_venides_leal`), contarle el ataque pero no lo de Edul y Verina (`bnd2_venides_enganado`) o no contarle nada (`bnd2_venides_sin_aviso`: en el Ocaso le retienen). |
| Lo del primer día | `b2_venides_edul`, `b2_aigren_daga`, `b2_aigren_se_va`, `b2_luisarda_cerrada`, `b2_luisarda_rosita`, `b2_xila`, `b2_tienda_con_pista` | Venides quiere ir a por Edul; Aigren te da su daga o se va con su hijo; Luisarda cierra (delatada) o duerme con Rosita; Xila, aliado, sabe dónde están los trajes (la tienda sin tirada). |

Objetos nuevos (`assets/objects/boundington_objetos.json`, en el manifiesto): la Furia de Alaro, la daga de Aigren y
el amuleto gitano, de momento sin estadísticas. Enemigo nuevo: el saqueador de Surysal (4 PV, CON 0, DES 0).

### 11.5 Lo que decidí yo (segundo día)

1. **Los 6 saqueadores son esbirros** (4 PV, CON 0, DES 0): seis contra el héroe solo es un 1 contra 3 (cae el ~32 %
   con el kit, `simulador_dificultad alternativas`). Como saqueadores, huyen a 0 PV.
2. **Acabar con todos en la cripta** son 3 Devotos y 1 Fanático, no 4 y 2: con 4 y 2 el héroe caía el 95 %; así,
   el 68 % («bastante difícil»). Plantarles cara a los Verdaderos (5 Devotos, de antes) da un 84 %.
3. **Contarle todo a Rosita va con la espada** (la C de la misma escena), y suma la confianza de los dos favores.
4. **El capitán da una sola oportunidad**, y «No molestarle» la gasta.
5. **El cobrador deja pasar en los tres casos** (pagar, denunciar o amenazar), sin tirada: «otra entrada a la fiesta».
6. **Edul te mete dentro**: sin vigilante ni tirada de mansión.
7. **Ocultarle algo a Venides** (lo de Edul y Verina, o todo) es −menor en los dos casos; si le cuentas el ataque,
   reúne sus 10 hombres igual.
8. **La herrería «vacía»**: el sprite de Aigren sigue ahí (los PNJ no desaparecen por flag), así que Aigren está
   cargando el carro y se va.
   *(PROLOGO-02, 13/10: ya desaparecen. El segundo día, si Aigren se fue, no está y en su lugar está Edul; ver §13.3.)*

## 12. Los diálogos en el juego (DIALOGOS-01 y HEROE-01, 13/10, Claude)

Fuente: «Boundington: los diálogos» (el documento del 10/10, corregido por Oriol y
guardado en el vault el 13/10: `Campanya 1 Boundinghton/Boundington los diálogos.md`).
Las notas de Oriol que mandan: la compañía de Rosita con las SP de la cuarta tanda de
NosTale; el día 2, si Aigren se fue, Edul está en la herrería y te trata «seco, sin
coba»; el rescate final por la antigua compañía de Rosita; y «està bé, falta
ampliar-ho».

### 12.1 Qué hay ahora

| Aventura | Beats antes → ahora | Lo nuevo |
|---|---|---|
| Prólogo | 17 → 18 | Qué haces al caer (mirar [Percepción], gritar, esperar), ante la barca ([Conocimiento arcano], preguntar, retroceder) y la visión de Ceaseton como charla. |
| Primer día | 94 → 235 | **La noche 0**: llegas de noche a la Taberna Humilde (la cama de Rosita, Venides con su cuaderno, las mesas, la ventana y el rincón de Tomás) y el desayuno con Venides; después, todas las escenas del día como charlas. |
| Segundo día | 76 → 133 | El castillo (la puerta, Venides, el capitán), Rosita y la espada, Gorran, Edul en la herrería, Xila en su escalón, la Barriada (el forjador, el guardián, Zarela), la fiesta (el vigilante, el cobrador, Edul) y la noche. |
| El Ocaso | 27 → 95 | Entero: la plaza y el sermón, la base, los tres caminos, Verina en la barricada, Rosita en su calle, Pulga, Sandes en el Casco, Las Afueras (Zarela, Gorran, la barricada, Edul, Venides retenido), el combate final, la compañía de Rosita, las cuatro salidas, el epílogo y la barca. |

**El estándar de las charlas.** Una conversación tiene apertura, temas que se
preguntan una vez y en el orden que se quiera («¿Qué escribes?», «Preguntar por los
Perdidos»…), «Hablar de otra cosa» para pasar página (tres temas como mucho por
página) y una despedida. Un tema puede tener sus propias preguntas («Dejarlo ahí»
vuelve). Las decisiones de verdad (pagar la cama, convencer a Aigren, entregar a
Verina) siguen siendo opciones que cierran. En el formato: `"sigue"` (la charla
sigue), `"pasaPagina"`, `"despedida"`, `"pagina"` y `requires.lugar`
(`FORMATO_AVENTURAS.md`). Una página que se ha quedado sin temas se salta sola.

**El héroe que se creó.** Raza, clase, origen, deidad, las siete respuestas, el
nombre y los atributos llegan como flags `heroe_*`, variables `heroe_con/des/int/car`
y textos `{nombre}`, `{origen}`, `{pueblo}`… 25 beats cambian según el héroe: los
vecinos de la ventana y los Verdaderos increpan a quien se nota que no es humano,
los sectarios miran con interés al de sangre oscura, Mira le habla distinto al
bestial, Xila te reconoce el oficio (tanque, ágil, mago), Aigren habla de forja con
el de fe artesana, el ritual de la Barriada cambia con el elemento fuego, al que
tiene Destreza 3 el carterista no le roba (le coges la muñeca), a Gorran le basta
mirarte los brazos si tienes Constitución 3, Venides nota al que es de Aegroum, y
la compañía de Rosita reconoce a un guerrero demoníaco.

### 12.2 Lo que decidí yo (para revisar)

1. **El día 1 empieza la noche antes**, en la Taberna Humilde (antes empezaba en la
   plaza por la mañana). Lo pide el documento; el juego carga `interior_taberna`.
2. **La compañía de Rosita** salta si **pierdes el combate final con Sandes** y
   salvaste a Edul y a Verina, Xila y Venides están contigo, Rosita te contó su
   historia el día 2 y su confianza es 2 o más. Entonces se repite el combate con
   ellos a tu lado. El motor deja como mucho **tres acompañantes**: van el Mago de
   las Tinieblas, el Guardabosques y el Guerrero Demoníaco (la cuarta, Rosita, se
   queda en su calle, que es lo que cuenta el epílogo). Tu nota decía «cuando Sandes
   esté a 1/4 de vida, con las habilidades de Venides y Xila gastadas y a punto de
   morir»: eso pide que el combate mire su propio estado a mitad, y **no está
   hecho**. Hoy es «pierdes → llegan».
   *(ROSITA-01, 14/10: hecho; ver §13.4. Llegan a mitad del combate, y «pierdes →
   llegan» queda para el golpe que te tumba de una vez.)*
3. **Epílogo de Verina sin Edul.** Si salvas a Verina pero dejas a Edul, el documento
   solo tiene «salió de la ciudad con su hermano». He añadido (texto mío):
   «La lectora salió de la ciudad. Sola. Buscó a su hermano entre los que salían,
   hasta que ya no salió nadie más. Tiene muchos libros por delante. Los leerá por
   los dos.»
4. **Xila en su escalón** (día 2): si le diste el desayuno, al entrar en Pico Dragón
   está esperando y la tienda abandonada no salta hasta que hablas con él. «¿Qué
   sabes hoy?» te lleva directamente a la tienda.
5. **El vigilante de la fiesta**: preguntarle «¿Muchos años de vigilante?» ya no
   acaba la conversación, y «Volver más tarde» deja volver a intentarlo.
6. **Sandes en la cripta** habla contigo una sola vez (antes, «Quiero unirme» le hacía
   repetir la presentación sin fin).
7. **Dormirse en la barca es un descanso largo**: la moral del Ocaso se asienta
   aunque no le preguntes a Sho-Noco si lo que hiciste cambió algo.
8. **Edul en la herrería** usa de momento el sprite de Aigren (no hay uno suyo), y
   las mesas de la taberna se hablan a través del parroquiano.
   *(PROLOGO-02, 13/10: hecho. Edul tiene sprite propio y la taberna tiene su gente, cada uno con su charla; ver §13.3.)*

### 12.3 Cómo se ha comprobado

- `demo_campana_boundington`: la campaña entera, seguida, por dos jugadores (el héroe
  que lo hace todo y el que va a lo justo) con las cuatro suertes. Al héroe le llega
  la compañía de Rosita.
- `demo_boundington`: **50 escenas** con nombre (17 del primer día, 15 del segundo y
  18 del Ocaso), cada una con lo que tiene que pasar.
- `explorar_boundington`: cientos de partidas al azar con héroes hechos con la
  creación de personaje (41 razas, 127 clases, 12 orígenes, 23 deidades). **Ninguna
  partida atascada**; salen los tres finales; lo único que nunca sale al azar son
  cosas que piden una cadena muy concreta (la compañía, Edul con lo de Venides, las
  pociones de Luisarda el día 2) y las prueban las escenas.
- La moral (media al final, 600 partidas): el que juega bien acaba en +22 (la mitad,
  Respetado), el que juega mal en −18 (cuatro de cada diez, Canalla), y el que juega
  al azar en +4. Sube y baja noche a noche, sin saltos.

## 13. El prólogo nuevo: el prado, el barquero y los recuerdos (PROLOGO-02, 13/10, Claude)

Oriol (13/10): «Sho-Noco tiene que aparecer en un prado donde solo haya un río;
cuando exploremos el río aparezca el barquero, que es Sho-Noco; no queremos
subir, hablamos con él, subimos al barco; nos lleva a los recuerdos de Venides:
prepara diferentes cinemáticas donde pase cada cosa y donde estemos Sho-Noco y yo
paseando como si fuéramos espectros entre esos recuerdos, hasta llegar a la
taberna».

### 13.1 Cómo va

| Momento | Nivel | Qué pasa |
|---|---|---|
| La caída | `prado_del_rio` | Caes sobre la hierba de un prado sin horizonte, con niebla blanca y un río. El cofre del kit está a tu lado, varado. Mirar [Percepción], gritar o esperar. |
| El barquero | `prado_del_rio` | Al llegar a la orilla, una barca baja por el río (cinemática). [Conocimiento arcano] para mirarle. Sho-Noco salta a tierra. |
| Sho-Noco | `prado_del_rio` | Charla: «Sube.» Primero «No pienso subir ahí» y las preguntas (quién eres, dónde estoy, adónde lleva el río, qué vamos a ver, por qué yo); luego «Subir a la barca». Al subir, el jugador se vuelve translúcido: es un espectro. |
| Ceaseton, 756 | `recuerdo_ceaseton` | La taberna del puerto. Los espectros entran y se acercan a la mesa de la ventana: el brindis de los cuatro (el texto de siempre, partido en tres momentos), «Nadie te ve» (mirar a Sandes, a Verina y Venides) y el giro: la mesa se vacía. |
| Ceaseton, 757 | `recuerdo_puerta` | La casa de Venides. Vuelve de las Santas Montañas, deshace el equipaje. Llaman: son los tres, con la capucha morada. Le piden que vaya a la reunión del almacén del puerto; él dice «Otro día» y cierra la puerta. Charla con Sho-Noco. |
| Aegroum, 757 | `recuerdo_aegroum` | La sala de mando de la guardia real. El capitán le destina a Boundington, donde ha crecido «un culto: se hacen llamar los Perdidos». Venides acepta y se lleva un cuaderno nuevo, en blanco. Charla con Sho-Noco. |
| Boundington, 757 | `recuerdo_taberna` | La Taberna Humilde, de noche, con lluvia. Venides llega mojado, Rosita le da algo caliente y él escribe la fecha en el cuaderno. Sho-Noco: «Boundington. Año 757. Tres días.» Se va, y el jugador vuelve a ser de carne y hueso. Empieza la noche 0 del primer día. |

### 13.2 Lo que decidí yo (para revisar)

1. **El cofre del kit sigue estando**: está en el prado, «varado en la hierba
   como si el río lo hubiera dejado ahí». Pediste un prado «donde solo haya un
   río». Si lo quieres fuera, hay que dar el kit de otra manera.
2. **Los recuerdos de 757 tienen texto mío.** El canon solo dice que Venides
   «volvió de las montañas y cerró la puerta a sus amigos; ellos ya rezaban a
   otra cosa» y que «un año después, el reino le mandó a una ciudad donde se
   rezaba a lo mismo».
   - Las escenas de la puerta (la reunión del almacén del puerto, Perishton,
     «Otro día») y de la orden del capitán son invención mía.
   - También lo es el cuaderno en blanco, que ata con el Venides que escribe
     en la Taberna Humilde.
3. **Qué pasó en las Santas Montañas sigue sin contarse** (NO ESTÁ en el vault).
   Sho-Noco: «Pregúntaselo a él. No te lo dirá.»
4. **La última escena es la llegada de Venides a la Taberna Humilde**, la misma
   noche en que empieza el primer día.
5. **Sho-Noco** lleva capucha oscura, manto blanco, pala y farol.
6. **La capucha morada de los tres amigos en 757** es la señal visual de que ya
   «rezan a otra cosa».

**Oriol, 14/10 (D0015 §12):** el cofre, bien; los recuerdos de 757, decentes (los revisará); lo de
las Santas Montañas es que **dos amigos se pelearon por su distinta fe**, y se deja entender cuando
aparecen rezando a un dios nuevo, sin contarlo; lo del espectro, sin revisar todavía. **Los tres
amigos llevan túnica, sin capucha; después se ponen la capucha y salen andando de la habitación**
(tarea AV14-21).
7. **Lo que ves andando como espectro** (FANTASMA-01, 14/10). Al acabar cada
   recuerdo andas libre entre ellos y, al acercarte, ves un detalle de cada
   uno. Los textos son míos:
   - Venides se toca la capa nueva;
   - Verina tiene un libro debajo de la jarra;
   - Edul tiene quemaduras de forja y mira a Verina;
   - Sandes no ha bebido de su copa;
   - el tabernero seca una jarra seca y un pescador duerme;
   - en 757, Venides con la mano en la madera;
   - en Aegroum, el capitán ya con otro papel y Boundington como un punto en
     el mapa.

   Para seguir hablas con Sho-Noco. Con «Mirar un poco más» te quedas.

### 13.3 Cómo está hecho

- **Las cinemáticas** (`assets/cinematicas/cinematicas.json`, `Game/Cinematica.h`)
  son órdenes: quién anda hasta dónde, a quién se mira, cuánto se espera, qué
  nivel se carga, el fundido a blanco.
- **Lo que se dice está en la aventura**, como siempre. La orden «beat»
  enciende una flag, salta el beat que la pide y la cinemática espera a que se
  cierre el diálogo.
- **Los espectros** (el jugador y Sho-Noco) van translúcidos y azulados
  mientras está encendida `bnd_prologo_espectro`.
- **Las apariciones** esconden a un PNJ hasta una flag o desde otra: la barca
  antes de bajar, los amigos que llaman a la puerta, Sho-Noco cuando se va.
- **Los recuerdos tienen un fondo de niebla clara** en vez del azul oscuro.
- **La Taberna Humilde** (`interior_taberna`) tiene ahora su gente en su sitio,
  cada uno con su sprite y su charla (la bordadora y su marido, el carretero
  jubilado, Tomás y Berto, el jornalero callado, el encapuchado, Skilla...), en
  vez de hablar con las mesas a través del parroquiano.
- **Edul tiene sprite propio** (mandil, martillo) y está en la herrería
  (`interior_herreria`, junto al yunque) solo el segundo día, si Aigren se fue;
  Aigren deja de verse en ese mismo momento (las apariciones `edul` y `aigren`).
  Esto cierra §11.2.8 y §12.2.8.
- **El título**: detrás del pergamino se ve el centro de la ciudad, con su gente
  andando, y la cámara da la vuelta despacio.

### 13.4 La compañía de Rosita, a mitad del combate (ROSITA-01, 14/10, Claude)

Lo que faltaba de §12.2.2. En el combate final con Sandes, si se cumple lo de
siempre (salvaste a Edul y a Verina, Xila y Venides están contigo, Rosita te
contó su historia y su confianza es 2 o más), **la compañía llega dentro del
mismo combate**: el trueno sin nube, el fuego negro, la flecha de luz, y el
Mago de las Tinieblas, el Guardabosques y el Guerrero Demoníaco se ponen a tu
lado, con Venides y Xila. Si eres guerrero demoníaco, el suyo te habla en ese
momento («sin dejar de pegar»).

Lo que decidí yo (para revisar):

1. **Cuándo**: cuando Sandes baja a un cuarto de su vida **o** tú bajas a un
   cuarto de la tuya, lo primero que pase. Tu nota pedía las tres cosas a la
   vez (Sandes a 1/4, las habilidades de Venides y Xila gastadas y tú a punto de
   morir); juntas casi nunca pasan, y entonces la compañía no saldría.
2. **«Las habilidades gastadas» no se miran.** Se podría pedir que a Xila no le
   queden curas; dímelo si lo quieres.
3. **Si te tumban de un golpe** (de más de un cuarto a 0), no les da tiempo:
   llegan al caer y se repite el combate con ellos, como hasta ahora.
4. **Venides y Xila se quedan**: con la compañía sois seis en tu bando (el límite
   de tres acompañantes es por lista, y la compañía es otra lista).

**Oriol, 14/10 (D0015 §13):** la compañía llega **cuando Sandes, estando a 1/4 de vida o menos, te da
un golpe mortal**. Si no pasa solo, se scriptea para que pase siempre así (tarea AV13-34).

