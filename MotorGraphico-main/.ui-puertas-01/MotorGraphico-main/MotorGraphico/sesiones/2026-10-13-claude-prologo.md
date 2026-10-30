# 2026-10-13/14 · Claude (Cowork) · PROLOGO-02 + ROSITA-01 — el prólogo nuevo y lo que estaba a medias

Encargos de Oriol (13/10):
- la lista de fallos del título, la creación y el prólogo;
- «Sho-Noco tiene que aparecer en un prado donde solo haya un río [...] prepara
  diferentes cinemáticas [...] como si fuéramos espectros entre esos recuerdos,
  hasta llegar a la taberna»;
- en la taberna, los PNJ con su diálogo en vez del menú del parroquiano;
- la ciudad animada detrás del título;
- y, a las 23:18: «continua aplicant elements i depurant la partida preparant
  escenaris unics i el que faci falta que tinguem a mitges».

## Qué se ha hecho

1. **Las cinemáticas.** Son órdenes en `assets/cinematicas/cinematicas.json`:
   quién anda hasta dónde, a quién se mira, cuánto se espera, la flag o el beat
   que se enciende, el nivel que se carga y el fundido.
   - La lógica está en `Game/Cinematica` (motor_level, sin GL, 12 pruebas).
   - El juego, en `Engine/ApplicationCinematica`: el reproductor, el velo, los
     espectros translúcidos, las apariciones (un PNJ que se ve o no según las
     flags), el fondo de niebla de los recuerdos y la ciudad del título.
   - **Lo que se dice sigue en la aventura.** La orden `beat` enciende una flag
     y la cinemática espera a que se cierre el diálogo que la pide.
   - Formato: `FORMATO_AVENTURAS.md`, «Cinemáticas».
2. **El prólogo nuevo** (CANON §13). Caes en `prado_del_rio`. Al llegar a la
   orilla baja la barca de Sho-Noco; hablas con él («No pienso subir ahí» y las
   preguntas) y subes. Entonces, como espectros, se recorren cuatro recuerdos:
   - `recuerdo_ceaseton`: el brindis de 756;
   - `recuerdo_puerta`: la puerta cerrada de 757, con la capucha morada;
   - `recuerdo_aegroum`: la orden del capitán;
   - `recuerdo_taberna`: la llegada de Venides a la Taberna Humilde.

   Al final empieza la noche 0. El prólogo tiene 42 beats (antes, 18). Los
   niveles los genera `tools/gen_recuerdos.py`.
3. **El reparto.** Tienen sprite propio Sho-Noco, los cuatro amigos en 756 y en
   757, el capitán, la gente de Ceaseton, la gente de la Taberna Humilde y Edul.
   - Los personajes de la historia no tienen variantes de cara (`_unico`).
   - Boundington ocupa ya dos hojas (`pnj_boundington`, `pnj_boundington_2`).
4. **La taberna** tiene a su gente en su sitio, cada uno con su charla. **Edul**
   está en el yunque el segundo día si Aigren se fue, y Aigren deja de verse.
5. **El título**: detrás del pergamino se ve el centro de la ciudad, con su gente
   andando, y la cámara da la vuelta. «Nueva partida» empieza en el prado.
6. **ROSITA-01, lo que estaba a medias de DIALOGOS-01.** La antigua compañía de
   Rosita llega **a mitad del combate final** cuando Sandes baja a un cuarto de
   su vida o tú a un cuarto de la tuya, lo primero que pase. Antes llegaba al
   perderlo.
   - Formato nuevo: `"refuerzos"` en `startBattle`.
   - Motor: `Game::llegaRefuerzos` y `BattleState::sumaAliado`.
   - «Pierdes → llegan» queda para el golpe que te tumba de una vez.
   - CANON §13.4.
7. **El trinquete.** `ApplicationCinematica.cpp` era una fuente sin GL dentro de
   motor_core: `linea_base_core` lo contaba como deuda nueva (17 → 18). Ahora el
   fotograma lo limpia ella con el fondo del nivel (`limpiaPantalla`), así que
   es un fichero con GL, como sus hermanos `ApplicationHud`/`Bucle`/`Pantallas`.
   Todo lo que no necesita GL queda en `Game/Cinematica`.

## Cómo se ha comprobado

- **El juego real, en la VM** (sondas con Xvfb, también con ASAN). Se juega el
  prólogo entero, del prado al primer día, y se revisaron las capturas
  (`out/claude_dialogos02/capturas/`). El título carga la ciudad; «Nueva
  partida» empieza en el prado; Edul y Aigren aparecen y desaparecen.
- `demo_boundington`: **58 escenas** (6 del prólogo y la nueva de la compañía).
  También pasan `demo_prologo` (con las cinemáticas) y `demo_campana_boundington`.
- `explorar_boundington`: 300 partidas, **0 atascos**.
- Pruebas: `test_cinematicas` 12/12 y `test_cartas_combate` 45/45 (2 casos
  nuevos de los refuerzos).
- `validar_todo`: 0 errores (los 10 avisos de antes), con el validador nuevo
  `validar_cinematicas.py`.
- ctest gl-free: 211/214. Fallan los tres de siempre:
  - `linea_base_core*`, por ficheros nuevos sin versionar;
  - `test_marcar_paquetes`.
- ctest GL: 11/11.
- `linea_base_core` contra la línea base del Mac: 0 de deuda nueva.

## Lo que no está

- **El cuelgue de «Nueva partida» en el Mac.** No se reproduce en Linux, ni con
  ASAN ni con la vuelta título → creación → título → creación. Falta el
  registro: `MOTOR_TRACE=1 ./build/juego 2>&1 | grep -v "frame nuevo\|processInput\|render" | tail -40`.

## Para Oriol

- **Revisar:**
  - CANON §13.2: el cofre en el prado, los textos de 757, las Santas Montañas
    sin contar, la capucha morada;
  - CANON §13.4: cuándo llega la compañía.
- **Al commitear:** los ficheros nuevos de cmake/src/tests van en el mismo commit
  que su línea de CMake. Si no, `linea_base_core_configura` se pone rojo.

## Ficheros

- Paquete con preimagen: `out/claude_dialogos03/`. Contiene `aplicar.sh`,
  `preimagen.md5`, `nuevos.txt`, `postimagen.md5` y `dialogos03.tgz`.
- Los scripts y las fuentes: `out/claude_dialogos02/p02/` (`rosita/ROSITA-01.diff`
  para el combate) y `out/claude_dialogos02/guiones/`.
