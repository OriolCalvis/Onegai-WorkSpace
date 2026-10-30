# 2026-10-14 · Claude (Cowork) · Moverse por casillas, la iniciativa, el alcance y el golpe integrado (AV13-07, -11, -12, -13; AV14-01, -02, -03)

Encargo de Oriol (14/10, 07:01): «continua», detrás de «luego pongamos a programar y desarrollar el
juego». Lo primero de su orden: «moverse por casillas en combate, como en un tactics, con las áreas
visibles».

## Qué se ha hecho

1. **La ruta por casillas (AV13-07)**: `Game::RutaCasillas` (GL-free, en motor_level).
   - Dijkstra en un rectángulo, 4 vecinos, desempates fijos (siempre sale la misma ruta).
   - `ruta(desde, hasta)`, `alcanzables(desde, presupuesto)`, `coste()`.
2. **El tablero sabe andar**: `TableroCombate` gana `pasos`, `reponPasos`, `alcanzables`,
   `rutaHasta`, `anda`, `aTiro` y `acerca`.
   - 6 casillas (9 m) por ronda; se reponen en `tickRound`.
   - `acerca` busca una casilla a tiro al menor coste; si no llega, la más cercana; si no puede
     acercarse, no se mueve.
3. **Tú andas por casillas (AV13-12)**:
   - En tu turno se pintan en el suelo las casillas a las que llegas: rombos de papel claro con
     filete de tinta (`SueloCombate::alcance`).
   - Al pasar el ratón por una, sale la ruta en carmín y la casilla final más marcada.
   - Clic: el héroe anda la ruta casilla a casilla (`PasosEnRuta`, 0,16 s por paso, como fuera del
     combate). `GameSession::mueveEnCombate` gasta los pasos.
4. **Los rivales se acercan** (`BattleState::resolveEnemyTurn`):
   - Antes de pegar, cada enemigo se acerca: a 1 casilla si pega cuerpo a cuerpo, a 5 si tiene
     habilidades.
   - Si no llega, el registro dice «se acerca, pero no llega» y no pega.
   - En pantalla andan su ruta (`rivalesAndan`), con la animación de andar, y al acabar la cámara
     se recentra en todos.
5. **Atacar pide estar al lado (AV13-13, solo Atacar)**: si nadie está al lado, el registro dice
   «X está lejos: acércate (clic en una casilla de al lado)» y no se gasta la acción.
6. **Arreglos**:
   - `test_g13_combat_commands` enlaza motor_level en vez de compilar el tablero suelto (la guardia
     lo contaba como deuda nueva).
   - El sombreado del alcance no se veía sobre la piedra oscura: pasa a papel claro (0,20) con
     filete.

## Cómo se ha comprobado

- **El juego real**, sonda `tactico` con Xvfb, en la emboscada de la plaza:
  - Sale el tablero 11×11 con el héroe y el saqueador en su casilla.
  - El alcance: 2 casillas (la plaza es estrecha ahí: bancos y puestos).
  - El picking ida y vuelta es exacto: casilla → pantalla → casilla.
  - La ruta en carmín; el héroe anda hasta (61,32).
  - Atacar lejos da «está lejos»; al lado, pega.
  - El roce en el registro: «Empate, 1,5 contra 1,5: roce, 1 de daño».
  - Capturas: `t_alcance_zoom.png`, `t_ruta_zoom.png`, `t_anda.png`, `t_rival.png`.
- **Pruebas:**
  - `test_ruta_casillas` 4/4 (nueva);
  - `test_tablero_combate` 6/6;
  - `test_suelo_combate` 2/2;
  - `test_g13_combat_commands` 8/8;
  - `test_render_player` 66/66;
  - `test_cartas_combate` 45/45.
- **ctest:** gl-free 216/219 (los tres de siempre); GL 11/11.
- **`linea_base_core`:** 0 de deuda nueva. Bajan Application.cpp de 1347 a 1188 y Application.h de
  519 a 515.
- **Paquete `claude_dialogos04`:** probado en limpio, «APLICADO: 114 ficheros».

## Lo que no está (y va en el plan)

- **Los rivales pegan nada más empezar a andar**: el golpe se resuelve al instante y el dibujo
  llega después. Hace falta que el golpe espere a la llegada (AV14-01, el bus de golpes).
- **Las cartas aún no miran el alcance.** Solo Atacar pide estar al lado; el resto de AV13-13 está
  pendiente.
- **Los acompañantes** tienen casilla pero no dibujo, y no andan.
- **El encapuchado se dibuja algo alto** respecto a su casilla. Se corrige con el ajuste de pies.
- **Falta el turno por iniciativa (AV13-11)** y las áreas de las cartas (AV13-14).

## Segunda parte (07:25, «y seguimos para bingo»)

7. **Los rivales pegan al llegar.** El turno del rival va en dos mitades:
   - `BattleState::acercaEnemigos` mueve a los rivales en el tablero, y en pantalla se les ve andar.
   - Cuando han llegado, `ApplicationCinematica::rivalesPegan` resuelve el golpe.
   - Mientras andan, tú no juegas (`turnoRivalEnMarcha`).
   - Medido en la sonda: 38 fotogramas andando, y el golpe sale en el fotograma 38, ya llegado.
8. **El rival pega a quien tiene a tiro.**
   - Antes pegaba siempre al héroe, aunque estuviera lejos y tuviera a un acompañante al lado.
   - Ahora `ContextoBatalla::llega` le dice a las cartas a quién alcanza cada enemigo.
   - `objetivoDeEnemigo` y la provocación solo eligen entre esos.
9. **El turno por iniciativa (AV13-11).**
   - Al empezar el combate, cada uno tira 1d6 + DES (`Game::OrdenDeTurno`, `BattleState::tiraIniciativa`).
   - Se juega en ese orden, uno a uno: si el saqueador saca más, empieza él, con su rótulo «TURNO DE…».
   - La ronda se cierra al dar la vuelta (`pasaAlSiguiente`).
   - Arriba, los retratos van en el orden de la iniciativa, con la tirada en una ficha en la esquina y el
     marco carmín en quien juega.
   - En el registro sale «Iniciativa (1d6 + DES): Perdido saqueador 7, Sulivan 3.».
10. **El alcance de las cartas (AV13-13).** Las cartas ya dicen hasta dónde llegan (`range`), y aquí se
    pasa a casillas (`Game/AlcanceCartas.h`):

    | `range` | Casillas |
    |---|---|
    | sobre ti | 0 |
    | al lado | 1 |
    | corto | 4 (6 m) |
    | medio | 8 (12 m) |
    | largo | 12 (18 m) |

    - Más allá de 1 casilla hay que ver al objetivo.
    - Solo se puede apuntar a quien está a tiro; sin elegir, se apunta al primero a tiro.
    - La carta que no llega a nadie sale bloqueada y dice por qué: «nadie al lado: acércate», o «nadie a
      tiro (4 casillas, viéndole)».
    - La carta grande dice «Alcance: 4 casillas, viéndole».
11. **El alcance, en el suelo (parte de AV13-14).** Al señalar una carta, las casillas a las que llega se
    marcan con un velo y un filete carmín (`SueloCombate::tiro`).
12. **Menos deuda.** `test_g13_combat_commands` ya no compila BattleState, Skill ni el menú sueltos: enlaza
    motor_level. Las fuentes sueltas bajan de 62 a 58. `BattleState` se parte en `BattleStateTurno.cpp`
    para no pasar de 500 líneas.

Comprobado:
- **Sonda `tactico`:**
  - la iniciativa en el registro y en los retratos (`t_orden.png`);
  - el rival empieza si saca más;
  - anda y pega al llegar;
  - las cartas con su alcance: Atacar bloqueada lejos, las de 4 casillas listas;
  - 14 casillas a tiro en el suelo (`t_tiro.png`).
- **Pruebas:**
  - `test_orden_de_turno` 3/3 (nueva);
  - `test_g13_combat_commands` 12/12, con 4 casos nuevos: el rival lejos no pega hasta llegar; pega a
    quien tiene al lado; la iniciativa; el alcance de las cartas.
- **ctest:** gl-free 217/220 (los tres de siempre); GL 11/11.
- **`linea_base_core`:** 0 de deuda nueva.

Lo que falta de esta parte:
- **Los acompañantes no tienen puesto propio en la iniciativa:** actúan al cerrar tú tu turno, como antes.
- **Las cartas «a todos los enemigos» siguen pegando a todos:** solo piden tener a alguno a tiro. Las áreas
  de verdad son AV13-14, con la plantilla de codex (AV13-08).
- **Las cifras de corto, medio y largo son una propuesta:** viven solo en `AlcanceCartas.cpp`. Casi todas
  las cartas de clase dicen «short», y eso lo afina AV13-09.

## Tercera parte (07:50, «continua»): el golpe integrado (AV14-01, -02, -03)

13. **Un solo evento de golpe (AV14-01).**
    - La batalla apunta cada golpe: quién, a quién, cuánto y con qué grado. Los grados son fallo, pifia,
      roce, acierto, crítico, cura y periódico (`Game::GolpeCombate`, `BattleState::golpes()`).
    - Cada acción hace una foto de las vidas antes. Al acabar, apunta lo que ha cambiado y saca el grado de
      la tirada de la mesa contra ese mismo (`BattleStateGolpes.cpp`).
    - Antes, la pantalla comparaba vidas entre fotogramas y no sabía si un −1 era un roce.
    - El tablero, el orden de turno y los golpes van juntos en `CampoDeBatalla`, para que `BattleState` no
      pase de 15 miembros.
14. **El golpe en el cuerpo y en tu hoja (AV14-02, `Game::EfectosDeGolpe`).**
    - Quien recibe parpadea en carmín y retrocede, alejándose de quien pega. El roce empuja poco y el
      crítico más.
    - El crítico para el mundo 70 ms, y la cámara se sacude.
    - Quien esquiva (fallo, pifia) se aparta de lado. La cura da un pulso verde.
    - Si te pegan a ti, tu hoja (el retal de arriba a la izquierda) se tiñe por el borde rasgado y tiembla.
      La cifra de PV se pone en rojo y la vida perdida se queda un momento en la barra, en carmín claro.
    - Las bandas rojas de los bordes de la pantalla se han quitado.
    - El crítico sale en oro, más grande y con su nombre encima.
15. **Lo que entra salpica (AV14-03, parte).**
    - El roce echa polvo, el acierto chispas, y el crítico chispas y sangre. El fallo, nada.
    - La carta ya echa las partículas de su elemento al jugarse (PART-02d).

Comprobado:
- **Sonda `tactico`** (`t_golpe.png`): el saqueador pega −4 y en ese momento la hoja está tenida (0,94),
  temblando, con la barra fantasma desde 17. Sulivan parpadea en carmín y salta el «−4».
- **Pruebas:**
  - `test_efectos_de_golpe` 4/4 (nueva);
  - `test_g13_combat_commands` 13/13, con «los golpes suman lo perdido»;
  - `test_cartas_combate` 45/45, con los golpes de Venides apuntados con quién pegó.
- **ctest:** gl-free 218/221 (los tres de siempre); GL 11/11.
- **`linea_base_core`:** 0 de deuda nueva.

Lo que falta de esta parte:
- **El sonido (AV14-05):** no hay ningún sonido en assets (es AV14-04, de qwen) y `AudioDirector` no está
  cableado a la aplicación. Se hará cuando llegue el banco de sonidos.
- **Las partículas por material** (astillas, brasas): hoy son por grado. Las del elemento ya las echa la
  carta.
- **Los dos golpes de un acompañante en la misma acción** salen como uno solo: un golpe por quien lo recibe
  y por acción.

## Cuarta parte: el turno a la vista y la cámara con quien juega (AV13-15 parte, AV14-08 parte)

16. **Lo que te queda por andar.**
    - En la tira del turno, delante de ACCIÓN, sale «PASOS» con un rombo por casilla: en oro las que
      quedan, en gris las gastadas (`VistaBarra::pasos`, `pintaTurno`).
17. **La cámara va con quien juega.**
    - Se coloca entre todos, pero un 60 % hacia el que tiene el turno (`ApplicationCinematica::actorDe`).
    - Se recoloca al cambiar de turno y cuando acaban de andar.
    - En la sonda: en el turno del saqueador mira a él; en el tuyo, a ti.

Lo que falta de AV13-15 y AV14-08:
- **Correr, Esquivar, Retirarse y Ayudar** dependen de la pregunta de §4: con la acción principal cerrando el
  turno, Correr no sirve de nada.
- **La velocidad del combate (×1, ×1,5, ×2) y saltar animaciones** van en Opciones. Quedan para cuando se
  repase el menú (AV14-16).

## Quinta parte (09:01, tus respuestas a la bandeja): las reglas nuevas en el motor

Tus respuestas están en `decisiones/D0015`, y los apuntes de la UOC, en
`planificacion_alcance_y_vision/USABILIDAD_UOC.md` (los leyó un agente, 75 ficheros). Programado:

18. **La acción principal ya no cierra el turno (AV13-28).**
    - Con tablero, la mesa va «a lo BG3» (`CartasEnCombate::ponTurnoBG3`): atacar gasta la acción
      principal y el turno sigue abierto. Puedes andar y usar lo libre.
    - Atacar y la poción salen en gris («ya has actuado este turno»).
    - El turno se cierra con «Terminar» (la F, el botón de la esquina) o solo, cuando no queda nada que
      gastar: la principal gastada, sin pasos y ninguna carta jugable.
    - Los acompañantes y las invocaciones actúan al terminar.
    - Sin tablero (las pruebas de antes), la principal sigue cerrándolo.
19. **El crítico atraviesa el bloqueo, salvo bloqueo crítico (AV13-30).**
    - Con un bloqueo preparado, el atacante tira. Si saca crítico, el que bloquea tira su defensa, y solo
      para el golpe con otro crítico (`CartasBloqueo.cpp`).
    - **La pifia del atacante da un ataque de oportunidad**: un golpe de arma gratis. Nunca un hechizo, y
      no encadena.
20. **La pifia en una salvación (AV13-31):** el daño entero y el estado con el doble de rondas («pifia la
    salvación: le dura el doble»). Ya no da un dado más.
21. **Cicatrices (AV13-33):** al caer en un combate que no se perdona, se apunta una cicatriz más en
    `saves/<proyecto>/cicatrices.txt`, fuera de la partida. Así cargar no la borra, y no hay que tocar el
    fichero de la partida (lo que rompía «Continuar»). «Has caído» lo dice: «Una cicatriz más: ya llevas
    2. Vuelves a tu último guardado.»
22. **La compañía de Rosita con el golpe mortal (AV13-34):**
    - En el Ocaso, cuando Sandes está a un cuarto de su vida o menos, su siguiente acción te deja a las
      puertas (1 PV), te matara o no. Entonces llega la compañía.
    - En los datos: `"cuando": {"vidaEnemigo": 0.25, "golpeMortal": true}`
      (`BattleState::ponGolpeMortal`, `FORMATO_AVENTURAS.md`).
    - El guion `ocaso.py` está cambiado y la aventura, regenerada.

Comprobado:
- **Sonda `tactico`:** tras atacar, el turno sigue abierto, te quedan 5 pasos y Atacar está en gris.
- **Sonda `caer`:** la cicatriz se apunta para `pj_cazademonios`.
- **Pruebas:**
  - `test_cartas_combate` 50/50, con 6 casos nuevos: el turno a lo BG3, el crítico que atraviesa, el
    bloqueo crítico, la pifia con ataque de oportunidad, el golpe mortal, y la salvación pifiada
    (`efectoDoble`);
  - `test_cicatrices` 2/2 (nueva).
- **ctest:** gl-free 219/222 (los tres de siempre); GL 11/11.
- **`linea_base_core`:** 0 de deuda nueva.

Lo que falta de esto:
- **AV13-29, los acompañantes jugados por ti:** lo siguiente, y es grande.
- **La escena de noqueo antes del Game Over:** hoy sale directamente «Has caído».
- **La moral de la cicatriz:** no dijiste cuánta, así que no se toca.
- **Las cicatrices, en el cuaderno:** van con AV14-22.

## Sexta parte (09:51, «Donem tasques per chatgpt i per tu»): los acompañantes juegan su turno (AV13-29)

Lo que pediste en D0015 §1: cada acompañante tiene su turno en la iniciativa y lo juegas tú, salvo los
que su ficha diga `"ia": true`.

23. **La iniciativa, de todos.** Venides y Xila tiran 1d6 + DES como tú y salen en la tira de arriba en
    su sitio, con su tirada en la esquina. El marco carmín va en quien juega.
24. **En su turno, lo juegas tú.** La barra enseña sus cartas (Venides: Atacar, que golpea dos veces;
    Xila: Curar), andas con el ratón por su alcance y la cámara va con él. «Terminar» (F) le pasa el
    turno al siguiente.
25. **Están en el mapa.** Al empezar el combate, cada acompañante aparece en su casilla con el dibujo de
    su PNJ, y anda cuando lo mueves. Al acabar, se va. No se guarda en la partida.

    ```cpp
    // GameSessionTablero.cpp, montaTablero(): uno por acompañante, en su casilla del tablero
    ObjectSpawn s;
    s.objectId = kAcompanante + aliados[i].combatant->combatant_id();  // "acompanante_venides"
    s.position = s.patrolMin = s.patrolMax = *donde;
    sessionData()->m_world->createObject(s);
    ```

    Su dibujo lo pone `ApplicationCinematica::acompanantesALaVista`, cada fotograma del combate, a quien
    aún no lo tiene.
26. **Dos fallos que salieron al probarlo:**
    - **Los enemigos de las emboscadas del Ocaso no se veían.** Los dibujos solo nacían al cargar el
      nivel; los que la narrativa hace aparecer después no tenían. La misma función de arriba se lo pone.
    - **El héroe podía quedarse fuera de su casilla.** Si la suya no valía, el tablero lo ponía en la
      libre más cercana pero su dibujo seguía donde estaba. Ahora va a donde lo pone el tablero.

Comprobado:
- **Sonda `acomp`:** un combate del Ocaso con dos saqueadores, Venides y Xila.
  - Los dos acompañantes dibujados en el tablero, y la tira en orden de iniciativa.
  - En el turno de Venides, 27 casillas de alcance: anda y su dibujo le sigue.
  - Venides ataca dos veces y Pasar da el turno al siguiente.
  - Capturas `a_venides.png`, `a_anda.png` y `a_ataca.png`.
- **Pruebas:** `test_game_session` 38/38, con una nueva: el objeto del acompañante, que le sigue al
  andar, no se guarda y se va al acabar.
- **ctest:** gl-free 219/222 (los tres de siempre); GL 11/11. **`linea_base_core`:** 0 de deuda nueva.

Lo que falta de esto:
- **Los de `"ia": true`:** actúan sin andar.
- **El enemigo derrotado:** sigue dibujado hasta que acaba el combate.
- **El parpadeo y el retroceso del golpe:** todavía no salen sobre el acompañante.

## Séptima parte (10:26, «El meu carril de combat»): caer (AV13-33) y lo que faltaba de los acompañantes

27. **Al caer, la escena.** Antes de «Has caído»:
    - tu personaje se hunde y se apaga, la pantalla se oscurece y salen las frases una tras otra;
    - con ENTER pasas a la siguiente, y el ENTER no elige nada en el menú de debajo;
    - si alguno de los tuyos sigue en pie, es un **rescate**: «Venides se pone delante y te arrastra lejos
      de la pelea.» Si no, un **noqueo**: «Oyes pasos que se alejan. Después, nada.»;
    - después, el papiro de «Has caído» con la cicatriz («Te queda una cicatriz.»).

    La lógica es GL-free y tiene prueba (`Game::EscenaNoqueo`):

    ```cpp
    // src/Game/EscenaNoqueo.cpp
    EscenaNoqueo EscenaNoqueo::para(const std::string& rescatador) {
        EscenaNoqueo e;
        e.m_rescate = !rescatador.empty();
        e.m_lineas.push_back("El golpe te tumba. El suelo sube a tu encuentro.");
        ...
    ```
28. **Si cae el héroe, se pierde.** Antes la batalla seguía mientras quedara alguien de los tuyos en pie,
    y con Xila, a la que no atacan, no acababa nunca.

    ```cpp
    // src/Game/BattleState.cpp, checkOutcome()
    if (firstAliveIndex(m_allies) < 0 || (m_allies.front().combatant != nullptr && !m_allies.front().combatant->isAlive())) {
        m_outcome = BattleOutcome::Defeat;
    ```
29. **Lo que faltaba de los acompañantes:**
    - el de `"ia": true` anda antes de actuar: hacia los enemigos si pega; si no (Xila), junto a ti;
    - **quien cae deja libre su casilla y su dibujo se va.** Antes el saqueador muerto seguía de pie hasta
      el final;
    - los números, el parpadeo y el retroceso de los golpes ya salen sobre los acompañantes. Antes no les
      salía nada, porque no tenían dibujo que buscar (`ApplicationCinematica::dibujoDe`).
30. **Un enemigo de emboscada podía quedarse dibujado fuera del tablero** si su casilla no valía. Ahora
    se coloca al empezar.

Comprobado:
- **Sonda `noqueo`:** un combate ineludible del Ocaso con Venides y Xila; caes y te rescata Venides.
  Capturas `n_cae.png`, `n_rescate.png` y `n_papiro.png`.
- **Sonda `acomp`:** el saqueador que cae desaparece (`a_ataca.png`).
- **Pruebas:** `test_escena_noqueo` 3/3 (nueva) y `test_cartas_combate` 52/52 (1 nueva).
- **ctest:** gl-free 220/223 (los tres de siempre); GL 11/11. **`linea_base_core`:** 0 de deuda nueva.

Lo que falta de esto:
- **La moral de caer:** PO-38, en la bandeja.
- **El segundo golpe de Venides:** se pierde si el primero mata.
- **AV13-14:** espera las plantillas de ChatGPT.
- **AV13-35:** espera la medida de Haiku.

## Octava parte (10:58, «Esborrar el combat antic»): el combate antiguo, medido y borrado (AV13-35)

31. **La medida, primero.** Está en `planificacion_alcance_y_vision/MEDIDA_COMBATE_VIEJO.md`:
    - las 127 clases tienen cartas, así que el juego, con un personaje creado o cargado, siempre tiene mesa;
    - solo llegaban al combate antiguo la prueba de nivel del editor (héroe sin ficha) y las pruebas;
    - huir y usar un objeto no son del combate antiguo: los resuelve la batalla también con mesa, y se
      quedan.
32. **Siempre hay mesa.** Sin ficha, la mesa solo tiene las básicas (Atacar, Poción, Huir); sin catálogo
    (las pruebas), se monta con uno vacío:

    ```cpp
    // src/Game/GameSessionInteraccion.cpp, startBattle()
    static const Game::CatalogoCartas kSinCartas{{}, {}, {}, {}};
    if (sessionData()->m_playerBody != nullptr)
        mesa = Game::CartasEnCombate::paraHeroe(m_cartas != nullptr ? *m_cartas : kSinCartas, *sessionData()->m_playerBody);
    ```
33. **Borrado:**
    - en `BattleState`, el ataque Nd6 y las habilidades con maná sin mesa, la IA de habilidades del enemigo
      y las invocaciones pendientes. El fichero baja de 483 a 286 líneas;
    - `BattleMenuActions`, `BattleMenuState` y `TargetSelection`, con sus pruebas;
    - `demo_battle`;
    - la parte antigua de la barra y de `confirmBattleCommand`;
    - en las pruebas, los 11 casos antiguos de `test_battle_state`. Las de AV13/AV14 de
      `test_g13_combat_commands` llevan ahora su mesa, como en el juego.
34. **El paquete no borra nada en tu carpeta:** `aplicar.sh` aparta los ficheros borrados a
    `out/claude_dialogos04/borrados/`. Bórralos tú cuando lo hayas probado.

Comprobado:
- **Sonda `editor`:** héroe sin ficha y sin título. Tiene mesa con las 3 básicas y Atacar va por la mesa.
- **ctest:** gl-free 217/220 (los tres de siempre); GL 11/11.
- **`linea_base_core`:** 0 de deuda nueva; las fuentes sueltas bajan de 62 a 57.

## Novena parte (11:31, «Que el combat s'entengui millor»): la evaluación heurística en el combate

35. **En el turno del rival nada parece jugable (EH-12).** Todas las cartas salen en gris y el panel
    dice «Espera: juega Perdido saqueador» (`Game::esperaAlRival`). Antes seguía con «ENTER Jugar».
36. **Cada carta, su icono (EH-13).** El icono sale de sus etiquetas, por familia:
    - cura: corazón;
    - golpe: espada;
    - magia;
    - en área: flecha roja;
    - mejora: flecha verde;
    - control, invocación y defensa.

    Además lleva abajo una franja con el color de su elemento: Fuego, rojo; Tinieblas, morado… Las de la
    mano son más anchas y el nombre va a 16 px, en tres líneas.
37. **Las tiradas, en palabras (EH-16, EH-21):**

    ```
    antes: Venides ataca a Lobo (Exito, 1,5 contra 0: 2d6, 7 de dano).
    ahora: Venides ataca a Lobo: el ataque (1,5) supera la defensa (0). 7 de daño (2d6).
    ```

    También «crítico: … arrolla …», «… empatan. 1 de daño (solo roza).», «la defensa (1) para el ataque
    (0,5). Sin daño.» y «pifia: el ataque falla del todo» (`Game::frases`, con prueba).
38. **El segundo golpe de Venides:** si el primero tumba al enemigo, el segundo va a otro que tenga al
    lado. El de IA ya no pega a quien no tiene al lado.

Comprobado:
- **Sonda `tactico`:**
  - `t_espera.png`: el turno del rival, todo en gris;
  - `t_orden.png`: los iconos y las franjas;
  - el registro, en palabras.
- **Pruebas:** `test_frases_combate` 2/2 (nueva) y `test_cartas_combate` 56/56 (4 nuevas).
- **ctest:** gl-free 218/221 (los tres de siempre); GL 11/11. **`linea_base_core`:** 0 de deuda nueva.

## Décima parte (12:12): las áreas (AV13-14), con las plantillas de ChatGPT

ChatGPT entregó AV13-08 (`out/chatgpt_av13-08/`): la geometría de las áreas (círculo, cono, línea y
cuadrado) con 20 pruebas. **Va dentro de este paquete**: no hay que aplicar su rama aparte.

39. **La forma de cada carta.** Las cartas aún no la dicen, así que se deduce (`Game::areaDe`):
    - una zona es un círculo de radio 1 en el objetivo, o el radio que diga su texto («radio de 8 pies»:
      2 casillas);
    - «a todos los enemigos» es un círculo de radio 2;
    - si dice «línea» o «cono», esa forma, desde quien la lanza.
40. **Pilla a todos los de dentro, aliados incluidos.** La bola de fuego contra el lobo quema también a
    Venides si está a su lado:

    ```cpp
    // src/Game/CartasJugar.cpp, jugar()
    std::vector<Ref> principales = objetivos(c, def.target, objetivo);
    if (const auto area = areaDe(def); area && c.casillaDe) principales = enElArea(c, *area, objetivo);  // AV13-14
    ```
41. **Se ve antes de confirmar.** Al señalar la carta, sus casillas salen en carmín en el suelo y los de
    dentro se marcan en la tira de retratos. «A todos los enemigos» ahora se apunta a uno: el área va
    donde está él.

Comprobado:
- **Sonda `tactico`:** `t_area.png`.
- **Pruebas:** `test_cartas_combate` 57/57 (1 nueva), `test_g13_combat_commands` 6/6 (1 nueva) y
  `test_plantilla_area` 20/20.
- **ctest:** gl-free 219/222 (los tres de siempre); GL 11/11. **`linea_base_core`:** 0 de deuda nueva.

Lo que falta:
- **La forma de verdad:** la tienen que decir las cartas (AV13-09).
- ~~**Los muros:**~~ hecho a las 12:31: lo que queda detrás de un muro, visto desde el centro, no se
  pilla (`Game::esMuro`, pasado a la plantilla en la mesa y en el suelo; con prueba).

## Undécima parte (14:42, «els rètols amb el nom dels llocs»): los sitios con nombre (AV15-02)

Con `assets/catalogs/lugares.json` de ChatGPT (AV15-02b, con tus PO-39/40/41), cada puerta dice adónde
lleva:

42. **`Game::Lugares` (nuevo, motor_level):** lee el fichero y encuentra el sitio por el nivel al que lleva
    la puerta, con o sin carpeta delante (`de("…/interior_herreria.json")` → «Lingote de Oro»). Sin emblema
    propio se pinta el genérico `lugar` (`Lugar::icono()`); hoy 6 de 17 tienen el suyo.
43. **El cartel sobre la puerta (`CartelesPuertas.cpp`, nuevo):** las puertas a menos de 8 casillas
    llevan un retal con emblema, nombre y función («La Caja Firme / banco y cambio»), encima del dintel.
    El de la puerta en la que estás no sale: ya lo dice el aviso («Entrar en La Caja Firme»).
44. **El minimapa:** cada entrada con su emblema, y abajo «Cerca: La Caja Firme» (la más cercana).
45. **Al entrar:** el minimapa se titula con el nombre («La Boutique de Madame») y arriba sale un rótulo
    con nombre y función que se funde a los 3 segundos; si coincide con «Nuevo objetivo», va debajo:

    ```cpp
    // src/HUD/Pergamino/InterfazPergamino.cpp, ponZona()
    if (lugar != nullptr && claveNivel != m_claveNivel) m_retales.anunciaLugar(lugar->nombre, lugar->funcion, lugar->icono());
    ```

46. **La tienda dice dónde estás:** la cabecera pone «El Peso Justo · Tendero del mercado» (en
    `InterfazPergamino::pintaTiendaAbierta`, para no tocar lo que edita `claude_reputacion01`).

Comprobado:
- **Sonda `tienda`:** `l_tienda.png`.
- **Sonda `lugares`:** `l_calle.png` (cartel del banco, minimapa con emblemas y «Cerca:»), `l_cartel.png`
  (el cartel de cerca), `l_dentro.png` (la sastrería: título del minimapa y rótulo bajo el objetivo).
- **Pruebas:** `test_lugares` 6/6 (1 nueva: buscar por nivel e icono genérico).
- **ctest:** gl-free 220/223 (los tres de siempre); GL 11/11. **`linea_base_core`:** 0 de deuda nueva.
- **Cadena de 5 paquetes** (dialogos04 → diario01 → cicatrices01 → reputacion01 → confirmar01): aplican,
  compilan, gl-free 225/226 (solo el ratchet, fuera de git), GL 11/11.

Lo que falta:
- **Emblemas propios** para los 11 sitios que no tienen (baños, castillo, herrería, iglesia, mercado,
  ópera, posada, sastrería, taberna, universidad, casa de té): piezas de arte nuevas.
- **AV15-03** (despejar la ciudad) cuando lo asignes.

## Para Oriol

- Las dos preguntas (el turno y las distancias) las contestaste a las 08:21; lo demás, en `BANDEJA_PRODUCT_OWNER.md`.
