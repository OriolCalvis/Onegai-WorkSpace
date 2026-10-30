# 2026-10-14 · Claude (Cowork) · El combate a lo BG3: referencias, plan (AV13/AV14) y los primeros pasos

Encargos de Oriol (14/10, de 00:40 a 01:51):

- «mírate estas referencias para el sistema de combate»: 2 vídeos y 4 repositorios.
- «planifica qué tareas necesitamos para llegar al objetivo final, que es prácticamente ser
  Baldur's Gate 3 en cuanto a combate».
- Las decisiones del roce y de los esbirros.
- «hay que ir a un detalle más allá, a una perfección más minuciosa».
- «revisar que todos los botones vayan bien y fluidos en los menús».
- «esmenuzar cada tarea en múltiples tareas [...] actualizar el gantt para que otras IAs puedan
  trabajar en elementos que no molesten en nuestro trabajo principal».
- «luego pongamos a programar y desarrollar el juego».
- El orden: moverse por casillas con áreas; sentarse y animaciones dignas; andar nuevo con
  gamefeel.

## Qué se ha hecho

1. **Las referencias.** Se han leído los 4 repositorios y los 2 vídeos enteros, con sus subtítulos
   (Oriol los abrió en el navegador). Están en el doc de claude.ai «Referencias para el combate
   táctico». El minuto 16:18 de The Shaggy Dev es el sonido: música en bucle constante y efectos de
   foley.
2. **Las decisiones**, en `decisiones/D0014`:
   - el roce;
   - la ayuda de dados;
   - los esbirros;
   - BG3 como objetivo;
   - el feedback integrado;
   - los menús;
   - el orden.

   El reglamento (`dndWeebCC-master/docs/Sistema_Cartas_Tiers.md` §7.4) ya dice el roce, la ayuda y
   los esbirros.
3. **El diseño** (`planificacion_alcance_y_vision/COMBATE_BG3.md`):
   - de dónde partimos;
   - el tablero, el turno, el alcance y las áreas;
   - el % antes de tirar;
   - la posición como dados;
   - caer sin morir;
   - la IA por puntuación;
   - el golpe que se siente;
   - lo que se pide a Pixel.
4. **El plan** (`planificacion_alcance_y_vision/`, `build.py`):
   - las fases AV13 (27 tareas) y AV14 (19), con 77,5 jornadas;
   - cada tarea con su **carril** (principal o paralelo), quién la tiene y su zona.
   - `build.py` las enseña en el markdown, en el Gantt (las paralelas con trama) y en la página.
   - Comprueba que cada paralela tenga zona.
5. **El reparto.**
   - En `orquestador/`, la sección SIGUIENTE para opencode, trae, codex, gemini, qwen y zcode, y
     `pixel.md` con los encargos de PixelLab.
   - En el TABLERO, las filas paralelas en «Pendiente», la mía en «En curso» y la pregunta nueva
     (¿la acción principal deja de cerrar el turno?) en la bandeja de Oriol.
6. **AV13-02, el roce.** Un golpe físico empatado quita 1 (`RPG::dano::roza`, `kRoce`). Las cartas
   físicas también, sin sus dados. El número dice «Roce».
7. **AV13-03, los esbirros** (`simulador_dificultad esbirros`, modo nuevo). El saqueador pasa a
   **5 PV y DES 2**, y la Barriada se queda en DES 0. Lo medido está en D0014 §3:
   - aciertan el 30 %;
   - contra dos, 4 rondas y el héroe gana el ~87 %;
   - contra tres, cae el ~40 %, la regla de Oriol;
   - la Barriada tumba al héroe el ~43 % (antes ~32 %) por el roce.
8. **AV13-06, el tablero** (`Game::TableroCombate`, sin GL):
   - recuadro de 11×11 casillas del nivel;
   - quién está dónde;
   - distancia (la diagonal cuenta 1);
   - línea de visión;
   - la libre más cercana;
   - el borde.
9. **AV13-10, cada uno en su casilla.** Al empezar el combate, `GameSession::montaTablero`
   (`GameSessionTablero.cpp`) pone al héroe y a los enemigos donde estaban, y a los acompañantes en
   la libre más cercana. El borde del tablero se ve en tinta sobre el suelo
   (`Pergamino::SueloCombate`, en el pase de sombras).
10. **AV14-09, andar fluido** (con los frames de hoy).
    - Mantener la tecla anda seguido desde el primer paso. Antes esperaba 0,24 s y el dibujo se
      paraba.
    - El paso visual dura 0,16 s (la tecla manda uno cada 0,14), así que el siguiente lo continúa sin
      pararse.
    - El quieto solo sale parado (antes parpadeaba entre casillas).
    - La cámara va lineal y a la par del paso (antes, EaseOutCubic: un tirón en cada casilla).

## Cómo se ha comprobado

- **El juego real** (sondas con Xvfb):
  - `combate`: el tablero se monta (11×11, héroe y saqueador en su casilla) y se ve el borde
    (`tablero2.png`, con un recuadro de 5 para que quepa en pantalla).
  - `andar` (nueva), 2 s andando a la derecha:

    | | Antes | Ahora |
    |---|---|---|
    | Fotogramas quietos | 0 | 0 |
    | Velocidad del dibujo por fotograma | 1,66–4,47 px | 3,37–4,46 px |
    | Tirones de cámara (> 1,5 px respecto al jugador) | 83 de 117 | 0 |

- **Pruebas:**
  - `test_tablero_combate` 4/4 y `test_suelo_combate` 2/2, nuevas;
  - `test_render_player` 66/66, con la nueva `player_held_walk_never_stands_still_between_cells` a
    30, 60 y 144 fps; 9 casos ajustados a 0,16 s;
  - `test_cartas_combate` 45/45, con 3 casos que esperaban «el empate no entra» ajustados al roce;
  - `demo_narrativa_combate`, ajustado al saqueador de 5 PV.
- `explora.py` 200 partidas sin atascos; `demo_boundington` 58 escenas OK.
- ctest gl-free 215/218: fallan los tres de siempre (`linea_base_core*` por ficheros sin versionar y
  `test_marcar_paquetes`). ctest GL 11/11.
- `linea_base_core` contra la línea base del Mac: 0 de deuda nueva.

## Lo que no está (y va en el plan)

- **El borde del tablero apenas se ve** con el zoom de combate (11×11 no cabe entero entre los
  retales). Lo que se verá de verdad es lo de AV13-12: las casillas a las que llegas, en tu turno.
- **Los acompañantes tienen casilla pero no dibujo en el mapa** (antes tampoco salían).
- **No hay prueba de `montaTablero`** con un mapa de verdad: se ha mirado con la sonda.

## Para Oriol

- **La pregunta de COMBATE_BG3 §4**: ¿la acción principal deja de cerrar el turno?
- **El saqueador de 5 PV y DES 2** cambia los combates del día 1: más dinámicos y peligrosos en grupo.
- **Para las otras IAs:** la SIGUIENTE de cada una se empieza solo si lo dices al pegar el mensaje.
