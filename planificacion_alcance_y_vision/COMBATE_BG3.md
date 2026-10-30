# El combate a lo Baldur's Gate 3: de dónde partimos, qué falta y cómo se llega

> **Estado:** DISEÑO (14/10/2026, Claude). Lo pidió Oriol el 14/10: «planifica qué tareas
> necesitamos para llegar al objetivo final, que es prácticamente ser Baldur's Gate 3 en cuanto a
> combate se refiere».
> **Decisión que lo fija:** [D0014](../decisiones/D0014-combate-roce-esbirros-y-objetivo-bg3.md).
> **Flujo de diseño:** completa `workflows_juego/wf-01-combate-tactico.md`, que sigue mandando en
> las reglas: dados d6 Ed. 3, crítico con dos 6, iniciativa 1d6 + DES, cartas con enfriamiento y
> sin PA ni PM, y caer sin Game Over.
> **Tareas:** fases AV13 (combate táctico) y AV14 (gamefeel) de `dades.py`, con su Gantt.
> **Referencias:** el doc de claude.ai «Referencias para el combate táctico», que incluye los dos
> vídeos leídos con subtítulos y los cuatro repositorios.

## 1. Qué tiene que sentir el jugador

Que juega una **mesa de rol táctica**:

- ve el tablero;
- elige dónde ponerse;
- sabe antes de tirar cuánto tiene de acertar;
- y cada golpe se **siente**: suena, salpica, el marco de la hoja tiembla y el número cae sobre quien
  lo recibe.

BG3 lo resume así: «el juego hace las cuentas por ti, pero entiendes por qué aciertas o fallas».

## 2. De dónde partimos (lo que ya hay en el motor, 14/10)

| Pieza | Qué hace hoy | Dónde |
|---|---|---|
| Turno | Dragon Quest: los aliados uno a uno y luego **todos** los enemigos de golpe. Sin posiciones ni iniciativa. | `include/Game/BattleState.h` |
| Economía del turno | Una acción principal (cierra el turno), una de movimiento, una canalización, las libres y una reacción preparada por ronda (GAMEMACHINE B7). | `include/Game/CartasEnCombate.h` (`TurnoHeroe`) |
| Cartas | Ganchos de carta → efectos. Una carta queda gastada hasta el descanso. **«requiere_posiciones»** y «la velocidad de movimiento (sin posiciones)» están pendientes, y quedan 64 efectos a medias. | `GanchosCarta.h`, `assets/catalogs/skills.json` |
| Daño | DANO-02: se restan los éxitos y la diferencia son d6. Desde D0014, el empate roza (1). | `include/RPG/DanoEnfrentado.h` |
| Dados | Pool d6 con ventaja/desventaja. Crítico «dos 6» pendiente (CRITICO-DOS-SEIS-01). | `include/RPG/DicePoolEngine.h` |
| Interfaz | UI-BG3-01: barra de cartas abajo, carta ampliada, recursos del turno, orden de turno, placa del enemigo y registro. | `include/Game/BarraCombate.h`, `src/HUD/Pergamino/MesaCombate*.cpp` |
| Estados | Icono, rondas y signo de cada estado. | `include/Game/EstadosCombate.h` |
| Partículas | La carta jugada brota de quien la recibe, con la familia de su clase. | `include/Game/ParticulasDeCartas.h` |
| Golpes | Números sobre quien recibe, bandas rojas, zoom y temblor de cámara. Son de COMBATE-01, y Oriol pide integrarlos mejor. | `HUD/Pergamino/GolpesEnPantalla.*`, `ApplicationCinematica::combate` |
| Refuerzos | Llegan a mitad de combate según la vida. | `Game/Refuerzos.*` |
| Sonido | AudioDirector (qwen, H6-01) a medias; no hay ni un efecto de combate. | `Game/AudioDirector.*` |
| Moverse | Por casillas, con 140 ms de interpolación. El ciclo de andar es de 3 frames (paso, quieto, paso) por dirección, a 64 px. | `Render/Player.*`, `assets/textures/razas_andando.json` |

**La conclusión:** la mesa de cartas, los dados, el daño y la interfaz están. Lo que falta es
**el tablero**: el combate no tiene casillas, distancias ni áreas. Las primeras tareas abren eso, y
lo demás (reacciones, posición, superficies, IA) cuelga del tablero.

## 3. El tablero: el combate ocurre en las casillas del nivel

- **Dónde.** El combate no cambia de pantalla. Se juega en las casillas del nivel en que empieza,
  dentro de un recuadro de 15 × 15 casillas alrededor de los que pelean. El borde se marca en el
  suelo con tinta, y se sale andando por él para huir.
- **Quién está dónde.** El héroe y los enemigos, donde estaban. Los acompañantes y los refuerzos,
  en la casilla libre más cercana a quien acompañan. Un refuerzo entra por el borde, andando.
- **Medida.** 1 casilla = 1,5 m. El movimiento de base son **4 casillas (6 m)** y **+1 con DES 3 o más** (AV16-12, Oriol 15/10: «demasiados pasos»;
  antes 6, como BG3), y las cartas de movimiento lo cambian (la velocidad pendiente de skills.json).
- **Orden.** Iniciativa 1d6 + DES (D-COMB-03, ya decidida; tarea AV5-11), con retratos arriba. Cada
  uno juega su turno, enemigos incluidos: se acaba lo de «todos los enemigos de golpe».
- **Ruta.** A* sobre las casillas transitables. La ruta se ve antes de andar:
  - las casillas que alcanzas, en tinta clara;
  - la ruta, con las casillas que gasta;
  - un aviso si pasa junto a un enemigo (ataque de oportunidad) o por suelo peligroso.

## 4. El turno: la economía de GAMEMACHINE B7 sobre el tablero

La economía de las cartas se queda: una principal, una de movimiento, una canalización, las libres
y una reacción. Encima se pone el **movimiento en casillas**, que se gasta en cualquier momento del
turno.

**Decidido por Oriol (14/10, D0014 §8):** la acción principal **no cierra el turno**, para poder
atacar y después apartarse, como en BG3. El turno acaba con «Terminar turno», o solo cuando no queda
nada que gastar. Es un cambio de B7.

Las **acciones básicas**, sin carta, gastan la acción principal:

- **Correr:** el doble de movimiento.
- **Esquivar:** desventaja a quien te ataque hasta tu turno.
- **Retirarse:** moverse sin ataques de oportunidad.
- **Ayudar:** levantar a un aliado caído.
- **Empujar:** una tirada enfrentada que mueve 1–2 casillas; si choca, roza.

## 5. Alcance y áreas

**Las distancias (Oriol, 14/10, D0014 §8):** al lado 1 casilla; corto 4 (6 m); medio 8 (12 m); largo 12
(18 m); más allá de 1, viéndole. Viven en `Game/AlcanceCartas.h`.

- **Alcance de cada carta:**
  - cuerpo a cuerpo (al lado, 1 casilla);
  - alcance N;
  - a distancia (con línea de visión);
  - sobre sí.

  Hoy las cartas no lo dicen. Se rellena aguas arriba (las cartas de dndWeebCC) y el motor lo lee.
  Hasta entonces, el motor lo deduce del `SkillTarget` y de las etiquetas («Arco», «Distancia»).
- **Plantillas de área:** círculo (radio N), cono (N casillas, 90°), línea (N casillas) y cuadrado.
  Se ven en el suelo antes de confirmar y **resaltan a todos los que pillan, aliados incluidos**.
  Con el área se acaba lo de «a todos los enemigos» por arte de magia (`ALL_ENEMIES` pasa a ser un
  área).

## 6. Antes de tirar: el % de verdad (la ayuda de dados de D0014)

Al apuntar sale **ganar · roce · fallar**, en %, calculado exacto con los dos pools. No es una
simulación: es una convolución de dados con 6 = 1 éxito, 5 = medio, la ventaja/desventaja, el
crítico de dos 6 y la pifia. Encima sale el daño previsto (mín.–máx.) y la vida que le quedaría.

- La **primera vez**, una nota explica qué significa cada número.
- Va encendida de serie en Tier 0–1 y se apaga en Opciones.

## 7. La posición importa (como ±dados, que es lo nuestro)

| Situación | Efecto |
|---|---|
| **Flanqueo** (un aliado al otro lado del enemigo) | Ventaja en cuerpo a cuerpo |
| **Altura** (más alto que el objetivo; cuando haya altura en los niveles, D0004) | Ventaja a distancia |
| **Cobertura** (un objeto entre medias) | Media: −1 dado al atacante. Total: no hay línea de visión |
| **Disparar con un enemigo al lado** | Desventaja (BG3) |
| **Ataque de oportunidad** | Salir del alcance de un enemigo sin Retirarse le da un ataque (su reacción) |

## 8. Caer, tipos de daño, concentración

- **Caído, no muerto** (los aliados y el héroe; los enemigos, según la carta):
  - a 0 PV se cae;
  - cada turno se tira por ti: tres fallos, fuera de combate; tres éxitos, estable;
  - un aliado te levanta con «Ayudar».
  - El héroe que cae del todo va a la consecuencia narrativa sin Game Over (D-COMB-05, AV5-07).
- **Tipos de daño** (cortante, contundente, perforante, fuego, frío, rayo, ácido, veneno, sagrado,
  profano, psíquico). Resistencia = mitad; vulnerabilidad = doble; inmunidad = 0. Lo dicen las fichas
  de los monstruos y las cartas. La partícula y el sonido del golpe salen del tipo.
- **Canalización = concentración.** Un efecto sostenido a la vez. Si te dañan, tiras CON contra el
  daño para no perderlo.

## 9. Más adelante (cuando lo anterior funcione)

- **Superficies** que se combinan, con barriles y objetos del nivel (AV9-05, AV5-08):
  - fuego (quema cada ronda);
  - agua (moja: el rayo hace más);
  - grasa (resbala; arde);
  - hielo (resbala);
  - humo (sin visión).
- **Ronda de sorpresa** si el combate empieza desde el sigilo (AV8).
- **Encuentros por presupuesto** según la dificultad, y refuerzos con tope (butchery: menos de 4 por
  tipo y menos de 40 en total).
- **Jefes que se preparan**: pociones antes de pelear, escolta y frase de entrada.
- **Posturas** (Combat_Skill_Proficiencies) como cartas de canalización. **Mejora con precio**
  (Morituri): unos turnos fuerte y luego fatiga.

## 10. La IA enemiga: por puntuación, no por lista

Para cada enemigo, en su turno, se puntúa cada pareja (casilla a la que puede llegar × carta que
puede jugar desde ahí):

- **% de ganar** (el mismo cálculo exacto que ve el jugador) × daño previsto;
- **bonos:** rematar a quien está bajo, flanquear y quedar en cobertura;
- **castigos:** ataques de oportunidad recibidos, pisar suelo peligroso y quedarse solo.

El rol cambia los pesos:

- **bruto:** busca el cuerpo a cuerpo;
- **tirador:** busca distancia y cobertura;
- **flanqueador:** rodea;
- **cobarde:** huye al 25 % de vida, como ya dice WF-01.

Y los enemigos se reparten objetivos para no amontonarse en el mismo. Es lo que el autor de
Elemechs dice que le faltó.

## 11. Que cada golpe se sienta (la vara de D0014: «absolutamente todo, un detalle más allá»)

**Un solo evento de golpe** (patrón de sachitnjk/CombatSystem). La resolución emite `Golpe {quién,
a quién, cuánto, tipo, grado: fallo · roce · acierto · crítico · pifia}`. Todo lo demás se cuelga de
ese evento, sin escribirlo cuatro veces:

1. **Número**, en la tinta de la hoja:
   - el rojo carmín de los retales, con trazo de pluma;
   - «Roce» en tinta gris, «Crítico» más grande y con salpicadura, «Fallo» tachado;
   - cae y se desvanece sobre el personaje, como ya hace.
2. **Partículas** del tipo de daño, con las familias de CANON_PARTICULAS:
   - astillas de madera y metal para cortante y contundente;
   - chispas para el rayo;
   - brasas para el fuego;
   - sangre, según el monstruo.
3. **Sonido:** el golpe según el tipo y el material (carne, cuero, metal), el silbido del fallo, el
   roce y el crítico, todos con variaciones para que no se repitan.
4. **El cuerpo que lo recibe:**
   - parpadeo blanco de 2 fotogramas;
   - se echa atrás medio paso y vuelve;
   - una pausa de 60–80 ms en el impacto (hit-stop), solo en el crítico y en los golpes al héroe.
5. **Cuando te pegan a ti:** el **marco de la hoja** (el borde del pergamino del HUD) se tiñe de
   carmín por los cantos y tiembla un instante. Sustituye a las bandas rojas superpuestas.
6. **La cámara** se acerca al que actúa y le sigue, como BG3. Ya se acerca a la pelea; falta que
   siga el turno.
7. **Los dados del pool ruedan** en el centro: los 6 brillan, los 5 a medias, y se ve contra qué
   dados del defensor. Tarea AV1-04, que ya existía.
8. **Velocidad del combate** en Opciones (×1, ×1,5, ×2) y saltar animaciones. Como el daño y el
   sonido salen de la animación, todo acelera junto. Es accesibilidad: el autor de Elemechs lo
   recomienda.
9. **Música de combate:** un bucle de energía constante, sin solista ni cambios bruscos, para que
   no canse (Elemechs, minuto 16:18). Entra con un golpe de tambor al empezar y se va al acabar.

## 12. Fuera del combate, con la misma vara

- **Andar fluido:**
  - mantener la tecla no para entre casillas;
  - la interpolación con aceleración y frenada;
  - la cámara con suavizado;
  - pasos que suenan según el suelo;
  - un poco de polvo.

  Se hace ya, con los frames de hoy.
- **El ciclo de andar nuevo:** de 3 a 6–8 frames por dirección. Afecta a todas las capas del
  personaje (razas, prendas): primero los cuerpos base y los PNJ con sprite propio, con Pixel; luego
  las prendas, por lotes.
- **Animaciones dignas:**
  - sentarse en sillas y bancos (con su punto de asiento en el mueble);
  - brindar;
  - escribir;
  - llamar a la puerta;
  - apoyarse en la barra;
  - dormir.

  Las cinemáticas las piden con una orden («sienta», «brinda»).
- **Menús:** un recorrido automático pulsa cada botón de cada pantalla y apunta los que no responden.
  Cada botón tiene estado de encima y de pulsado, sonido de papel y transición sin tirones.

## 13. Lo que se pide a Pixel (sprites)

Formato de las hojas de hoy: celdas de 64 px, filas de 12 frames, en el orden arriba, derecha,
abajo, izquierda (`tools/pixellab/importar_actores.py`).

1. **El ciclo de andar de 6 frames** en 4 direcciones, para los cuerpos base y los personajes de la
   historia.
2. **Las poses:** sentado (4 direcciones), brindar, escribir, llamar y apoyado.
3. **Los iconos de la economía del turno** (acción, movimiento, canalización, reacción, terminar
   turno) y los de los tipos de daño.
4. **Los iconos de los objetos de la tienda**, para el rediseño de TIENDA.
