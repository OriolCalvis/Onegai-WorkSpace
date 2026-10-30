# D0014 — El combate (14/10/2026): el empate roza, los esbirros pegan más, el objetivo es BG3 y todo se pule hasta el detalle

- **Estado:** `accepted` — lo decide Oriol el 14/10/2026, tras jugar 10 minutos al paquete
  `claude_dialogos04` y leer el análisis del daño. **No se vuelve a preguntar.** Si algún día cambia,
  será otra decisión que sustituya a esta.
- **Fecha:** 2026-10-14 · **Decide:** Oriol · **Lo escribe:** Claude (Cowork).
- **Completa a:** DANO-02 (06/10, `include/RPG/DanoEnfrentado.h`: «empate o pierde → 0») y
  [D0013](D0013-reglas-cerradas-por-oriol-critico-topes-atributo-tier1.md) (el crítico son dos 6,
  que sigue igual).

## Por qué existe este fichero

Oriol, 14/10: «¿cómo es que la vida apenas sube y baja? Mira los registros de daño». El análisis del
paquete `claude_dialogos04` dio que el 59 % de los ataques no hacía nada: un empate valía 0, y un
saqueador (1 dado contra los 3 del héroe) solo acertaba el 14 % de las veces. Oriol eligió las dos
propuestas y fijó el objetivo de todo el combate.

## Decisión

1. **El empate hace 1 de daño («roce»).** Un golpe físico que empata ya no se pierde: roza y quita
   1 PV. Los ataques que no hacen nada bajan del 59 % al 36 %, y el combate dura unas 4–5 rondas.
   Para la magia, el empate ya era «la supera»: la mitad del daño (DANO-02), y no cambia.
2. **Ayuda de dados en los primeros niveles.** Oriol, 14/10: «poner ayuda de dados en los primeros
   niveles». Antes de tirar, el juego enseña cuánto tienes de ganar, empatar (roce) o fallar, y la
   primera vez explica la tirada. Va encendida de serie en los Tiers 0 y 1 y se puede apagar en
   Opciones. Es ayuda para entender los dados, no un dado de más: el % sale del pool de verdad.
3. **Los esbirros suben a DES 1–2**, para que te peguen el 30–35 % de las veces en vez del 14 %.
   Oriol: «vamos a balancearlo para que al principio sea más dinámico pero sin pasarnos; queremos que
   sea una experiencia inmersiva y de un buen reto en cuanto a los combates, no que sea imposible».
   Medido el mismo 14/10 con `simulador_dificultad esbirros`: Tier 1 con el kit, las 43 razas y las
   4 clases, ya con el roce.
   - **El saqueador pasa a 5 PV y DES 2** (antes, 11 PV y DES 0).
     - Acierta el 30 % de sus ataques y roza otro 20 %.
     - Contra dos, el combate dura unas 4 rondas, el héroe gana el ~87 % y acaba con ~3/4 de su vida.
     - Contra tres, cae el ~40 %, que es la regla de Oriol del 06/10 («1 contra 3, mueres un 40 %»).
   - **El saqueador de la Barriada se queda en DES 0.** Con el roce, los seis ya tumban al héroe el
     ~43 % (antes, ~32 %): otra vez el 40 % del 1 contra 3. Con DES 1 serían el 83 %.
   - Lo que había propuesto antes de medir (gana más de 9 de 10 contra dos y le queda el 40–70 % de la
     vida) **no puede darse a la vez**: con DES 2 y vida baja gana el ~90 %, pero le queda el ~78 %.
     Manda el 30–35 % de Oriol y el 40 % del 1 contra 3.
4. **El objetivo del combate es Baldur's Gate 3** («prácticamente ser BG3 en cuanto a combate»).
   Concretamente:
   - moverse por casillas como en un tactics;
   - las áreas visibles;
   - partículas y sonido en cada golpe.

   Las referencias que dio Oriol y lo que se toma de cada una están en
   `planificacion_alcance_y_vision/COMBATE_BG3.md`. Siguen valiendo:
   - los dados d6 (Ed. 3);
   - el crítico con dos 6 (D0013);
   - la iniciativa 1d6 + DES (D-COMB-03);
   - las cartas con enfriamiento por descanso corto o largo y sin PA ni PM (D-COMB-06).
5. **El feedback está bien, pero no basta: todo se integra artísticamente y hasta el detalle.**
   Oriol, 14/10: «el recuadro rojo podría ser como el marco de la hoja y no un cuadrado allí
   forzado; el número −7 está bien, pero además un sonidito y unas partículas sería mucho mejor; y así
   con todo, absolutamente todo, hay que ir a un detalle más allá, a una perfección más minuciosa».
   Es la vara de medir de cualquier tarea visible: nada se cierra con un recuadro o un texto puestos
   encima si puede ser parte de la hoja, del mundo o del sonido.
6. **Los menús, también.** Oriol, 14/10: «revisar que todos los botones vayan bien y fluidos en los
   menús también es importante». Cada botón de cada pantalla responde, se ve cuando está encima o
   pulsado, y la transición no se nota a tirones.
7. **Lo siguiente, en este orden** (Oriol, 14/10):
   1. moverse por casillas en combate, como en un tactics, con las áreas visibles;
   2. que los personajes se sienten, y animaciones más dignas;
   3. una animación de andar nueva, con gamefeel: «no puede ser que el movimiento no se vea fluido».

8. **El turno y el alcance** (Oriol, 14/10, 08:21, a las dos preguntas de Claude):
   - **La acción principal deja de cerrar el turno**, como en BG3 (COMBATE_BG3.md §4): atacas y
     después puedes apartarte. El turno acaba con «Terminar turno», o cuando no queda nada que gastar.
     Cambia GAMEMACHINE B7.
   - **Las distancias de las cartas** valen como las propuso Claude (COMBATE_BG3.md §5,
     `Game/AlcanceCartas.h`): al lado 1, corto 4 (6 m), medio 8 (12 m) y largo 12 (18 m) casillas;
     más allá de 1, viéndole.

## Consecuencias

- **Reglamento primero** (canon → cartas → motor): el roce y la ayuda de dados se escriben en
  `dndWeebCC-master/docs/Sistema_Cartas_Tiers.md` antes de tocar `DanoEnfrentado.h`.
- **Motor:** `RPG::dano::dadosFisicos` da 1 en vez de 0 con el empate; el bus de golpes del combate
  lo enseña como «Roce». Los esbirros de `assets/objects/boundington_enemigos.json` (y los del
  catálogo con el mismo papel) pasan a DES 1–2 y se re-mide con el simulador.
- **Plan:** las fases AV13 (combate táctico a lo BG3) y AV14 (gamefeel y pulido de lo jugado) de
  `planificacion_alcance_y_vision/` recogen todo esto, con las tareas desmenuzadas y separadas en dos
  carriles: lo principal (Claude) y lo que pueden hacer las otras IAs sin pisarlo.
