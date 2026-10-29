# 2026-10-14 · Claude (Cowork) · Lo que Oriol vio jugando (MAPA-01, DIALOGO-02, DROWS-01, CABEZA-01, BARCA-01, COMBATE-01, TITULO-01, TIENDA-04, FANTASMA-01)

Encargos de Oriol (14/10, después de probar `claude_dialogos03`):

- La lista de las 00:27:
  - drows en vez de sagas;
  - la barca y Sho-Noco;
  - lo de la cabeza en el creador;
  - «Narración: Sho-Noco: blabla»;
  - los tres comerciantes;
  - la herrería;
  - la partida guardada justo antes de un combate;
  - jugar como fantasma;
  - el lag;
  - el título sin feedback;
  - la tienda.
- A las 00:37, el combate: moverse como en un tactics, partículas y áreas, ver
  que te pegan, «la vida apenas sube y baja».
- A las 00:41: «el nombre de la ubicación del minimapa [...] no sabemos en qué
  zona estamos».

## Qué se ha hecho

1. **MAPA-01, dónde se cumple el objetivo.** Un objetivo puede decir `donde`:
   un nivel, una lista de niveles o un objeto.
   - El minimapa pone un **◆ rojo** en la entrada que lleva allí, o en la
     primera puerta del camino más corto (`Game::RumboNiveles`).
   - La misión de los tres comerciantes son ahora tres objetivos, cada uno con
     su sitio, y la herrería se encuentra siguiendo el ◆.
   - El nombre de la zona ya no empieza siempre por «Boundington».
2. **DIALOGO-02, quién habla.** Una línea `Quien, acotación: «texto»` se ve
   con QUIEN en el rótulo. «NARRACIÓN» sale solo cuando habla el narrador.
3. **DROWS-01.** Las tres «sagas» del creador son drows: ids, nombres, pieles
   y linaje.
   - `aplicar.sh` migra las partidas guardadas y deja copia en `saves_antes/`.
4. **CABEZA-01.** En el creador, la cabeza solo lleva lo que va debajo de un
   casco: pañuelo, coronas, cofia y cinta. Lo que te equipas manda sobre la
   apariencia.
5. **BARCA-01.** La barca mira al río. Sho-Noco va en la popa y tú en la proa.
   - Para eso, las cinemáticas tienen `desfase`.
   - El orden de dibujo va por la base del dibujo.
6. **COMBATE-01, primera parte.** Lo que ya se ve en combate:
   - números sobre quien recibe («-7», «Fallo», «Empate», «Pifia»);
   - bandas rojas cuando te pegan;
   - la cámara se acerca a la pelea y se sacude con cada golpe.

   Cada combate tiene ahora sus dados. Antes la semilla era siempre la misma y
   todos salían iguales.
7. **TITULO-01.** Al elegir en el título sale «Abriendo el cuaderno de
   creación…» o «Cargando la partida…» antes de la espera.
8. **TIENDA-04 (poco).** La hoja de la tienda ya no lleva folio. El rediseño
   de verdad espera a saber qué es lo que más molesta.
9. **FANTASMA-01, jugar como fantasma.** Al acabar cada recuerdo (Ceaseton
   756, la puerta de 757 y Aegroum) **andas libre** entre ellos, translúcido.
   - Al acercarte a cada uno ves un detalle suyo.
   - Para seguir, hablas con Sho-Noco. El ◆ del minimapa le marca y el
     objetivo lo dice.
   - «Mirar un poco más» cierra la charla sin seguir.
   - Antes, la charla se abría sola y no podías moverte.
   - Formato: `FORMATO_AVENTURAS.md`, «Andar libre después de una cinemática».
   - Los textos son míos: CANON §13.2, punto 7.
10. **La partida guardada**, puesta justo antes de un combate (lo pediste a las
    00:27). Con ella diste las notas del combate.

## Lo que se ha mirado y no se ha cambiado

- **Por qué la vida apenas se mueve** (reglas de DANO-02, no las he tocado).
  - Cada dado da 1 éxito con un 6 y medio con un 5. Un empate hace 0.
  - Tú (3 dados contra 1) aciertas el 58 %; un saqueador (1 contra 3), el 14 %.
  - Así, **el 59 % de los ataques no hace nada**.
  - Propuesta:
    - que un empate haga 1 de «roce» (los ataques en vano bajan al 36 %);
    - o bajar los esbirros a DES 1–2;
    - o las dos cosas.
- **«Continuar peta».** No sale en Linux, ni con ASAN. El `out/juego.log` del
  Mac es del 13/10. Si vuelve a pasar, falta el registro nuevo.
- **El lag del principio y del creador.** En la VM:
  - arrancar tarda 180 ms;
  - el primer fotograma del creador, unos 300 ms (las imágenes y los retratos).

  El `build/` del Mac está **sin optimizar**: `CMAKE_BUILD_TYPE` vacío equivale
  a -O0. Para jugar:

  ```bash
  cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
  cmake --build build-release -j8
  cd build-release && ./juego
  ```

## Cómo se ha comprobado

- **El juego real en la VM** (sondas con Xvfb).
  - El prólogo entero: anda 5 casillas en cada recuerdo, habla con Sho-Noco y
    llega al primer día (`fantasma1-3.png`).
  - El combate (`combate_golpe.png`), la barca (`barca3.png`), el título, la
    tienda y el ◆ del minimapa.
- `demo_boundington`: 58 escenas OK. La escena «grita, retrocede...» ahora
  también elige «Mirar un poco más» y vuelve. También pasan `demo_prologo` y
  `demo_campana_boundington`.
- `explora.py`: 200 partidas, **0 atascos**; todos los beats del prólogo vistos.
- `validar_todo`: 0 errores.
- ctest gl-free: 213/216, con los tres de siempre en rojo:
  - `linea_base_core*`, por ficheros nuevos sin versionar;
  - `test_marcar_paquetes`.
- ctest GL: 11/11.
- `linea_base_core` contra la línea base del Mac: 0 de deuda nueva.

## Para Oriol

- **Aplicar el paquete:** `bash MotorGraphico/out/claude_dialogos04/aplicar.sh`.
  Comprueba la preimagen: si algo cambió, no toca nada. Los ficheros nuevos de
  cmake/src/tests van en el mismo commit que su línea de CMake.
- **Compilar en Release** para jugar (arriba).
- **Decidir el daño:** el «roce» y/o la DES de los esbirros.
- **La tienda:** qué es lo que más molesta.
- **Siguiente, si te parece:**
  - el combate por casillas (moverse como en un tactics) y las áreas visibles;
  - sentarse y las animaciones dignas;
  - la animación de andar.

  Las tres piden sprites de PixelLab.
- **Revisar** CANON §13.2, punto 7: lo que se ve andando como espectro.

## Ficheros

- Paquete con preimagen: `out/claude_dialogos04/`.
- Las fuentes de los guiones: `out/claude_dialogos02/guiones/` (prologo.py,
  guia.py, esc_d0.py, explora.py, las demos).
- Seguimiento: `planificacion/seguimiento/{MAPA-01,DIALOGO-02,DROWS-01,CABEZA-01,BARCA-01,COMBATE-01,TITULO-01,TIENDA-04,FANTASMA-01}.json`.
