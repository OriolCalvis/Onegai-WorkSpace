# Sesión 15/10 — Claude (Opus), carril `pnj_motor`: los PNJ andan y hacen su oficio

Ronda: `orquestador/ronda_15-10.md`. Paquete: `MotorGraphico/out/claude_pnj_motor01/`.

## Qué hace

- Cada PNJ de un nivel (objeto `Npc` del World) lleva una rutina: `Game::RutinasPnj`
  (`include/Game/RutinasPnj.h`, `src/Game/RutinasPnj.cpp`, `src/Game/RutinasPnjCatalogo.cpp`, sin GL, en motor_level).
- Lee `assets/catalogs/rutinas_pnj.json` (el contrato de la ronda). Sin fichero, o sin entrada: `porDefecto`.
  Un JSON roto no rompe nada: todos con el valor de serie (deambular, radio 2, pausa 3–8 s, mira al jugador).
- Rutinas: `quieto`, `deambular`, `ir_y_volver`, `charlar`, `vigilar`, `actividad`. Movimientos: `respirar`,
  `vaiven`, `golpe`, `giro`, `andar_sitio`. `mira`: jugador, calle, vecino, giro, fijo:….
- Anda casilla a casilla, a 0,45 s por casilla (el jugador va a 0,16). Ruta corta dentro de su radio.
- Reglas: nunca pisa tu casilla, ni una puerta, ni las 4 casillas de al lado de una puerta, ni nada que bloquee
  (muro, objeto, enemigo, otro PNJ, mascota). No se para en un carril (`"carriles"` del nivel); sí lo cruza.
- Si le hablas (E), se para, te mira y aún te mira 2 s al cerrar. En combate, diálogo, tienda o negocio acaba
  el paso que lleva y no echa a andar. En una cinemática no hace nada (manda ella); si la cinemática lo deja en
  otro sitio, ese pasa a ser su sitio.
- Azar con semilla fija por PNJ (ruta del nivel + id + casilla): misma partida, mismo paseo.
- Un PNJ nacido junto a una puerta se queda clavado (no puede volver a su sitio sin pisarla).

## Cómo se ve

- Andar: `walk_*` del dibujo de antes a su casilla (desfase interpolado). Parado: el quieto de su rumbo
  (`idle_*`, el frame quieto del ciclo de andar: las hojas de PNJ no traen reposo).
- Respirar: sube 1 px un rato cada 2,4 s. Golpe: baja 2 px, rítmico. Vaivén: mira izquierda/derecha.
  Giro: los cuatro rumbos. Andar en el sitio: el ciclo de andar sin moverse.
- `MOTOR_MOVIMIENTO_REDUCIDO=1` (la misma variable que el HUD): sin vaivén ni andar en el sitio.
- Bocadillo sobre la cabeza (retal de pergamino con flecha, como el aviso «[E] Hablar»): el icono de la
  actividad mientras la hace (a 8 casillas o menos) y sus frases (solo a 4 o menos, 3,5 s, cada 8–16 s).
  No sale sobre quien ya lleva el «[E] Hablar».
- Las mascotas no entran en la zona por la que pasea cada PNJ (salvo su propia casa); los PNJ no pisan mascotas.

Capturas en `out/claude_pnj_motor01/capturas/` (ciudad_centro, jugador en el puente (32,32), con un catálogo
de prueba, `rutinas_captura.json`, porque el de verdad lo escribe pnj_datos):
`antes_despues.png`, `tira_despues.png` (4 capturas, ~6 s entre una y otra), `tira_antes.png`,
`bocadillos.png` (el oro del mercader y «¡Mirad, mirad!»), `sin_catalogo.png` (sin catálogo: anda igual).

## Cambios por ancla (`ediciones.json`, 19 trozos, 9 ficheros)

- `GameSession.h`: `rutinasPnj()` y el miembro `m_rutinas` (sin crecer: 546 líneas, el techo).
- `GameSession.cpp`: se crea y se mueve en `update()` (antes del corte de «solo en exploración»).
- `GameSessionInteraccion.cpp`: al hablar con un PNJ, `habla()`.
- `SaveGame.cpp`: un PNJ se guarda **donde nació** (los objetos se reconocen por su casilla al cargar). Sin
  esto, guardar con un PNJ paseando rompía la carga. Una línea, sin crecer.
- `ApplicationNivel.cpp` (499 líneas, sin crecer): `montaNivel` al cargar, las mascotas, la zona de cada PNJ.
- `ApplicationBucle.cpp`: la pausa en cinemáticas y los PNJ a la vista (animación, desfase, salto).
- `InterfazPergamino.h/.cpp`: una línea cada uno, `pintaBocadillosPnj` (nuevo `src/HUD/Pergamino/BocadillosPnj.cpp`).
- `CMakeLists.txt`: `include(cmake/RutinasPnj.cmake)` detrás de `ComercioReputacion`.

Las sesiones de prueba (y cualquier `GameSession` sin `monta`) siguen como antes: nadie se mueve.

## Cómo se prueba

    bash out/claude_pnj_motor01/aplicar.sh
    cd build && cmake . && ninja -j1 juego test_rutinas_pnj && ctest -R rutinas_pnj --output-on-failure

`test_rutinas_pnj` (16 casos, sin GL; corre en la raíz del repo): sin catálogo no falla; claves exactas y
prefijos; no pisa puertas ni la de delante; no pisa tu casilla ni muros; nunca sale de su radio y vuelve;
no se para en un carril pero lo cruza; misma semilla, mismo paseo; si le hablas se para y te mira; la partida
guarda su sitio y se carga; una cinemática manda; charlan de cara y el guardia vigila; la pose; frases solo
cerca y el oficio con su icono; ir y volver y la zona; el catálogo del juego (si está) se lee; en la sesión
no se pone encima de ningún objeto.

En el juego: `./juego --nivel assets/levels/ciudad_centro.json` y mirar la plaza del puente.

## Cómo se hizo (aviso)

La VM se colgó al empezar (device_bash dejó de responder). Todo se hizo en el contenedor con los ficheros del
Mac subidos uno a uno: motor GL-free + pruebas compiladas y pasadas; `juego` compilado entero (GLFW 3.4 X11
con cabeceras mínimas) y capturado con Xvfb y una sonda propia (`capturas/sonda_pnj.cpp`, carga el nivel en una
casilla y vuelca capturas). El paquete se probó sobre una copia limpia (aplicar dos veces: la segunda se niega).
También pasan `GameSessionTests` y `SaveGameTests` (salvo 3 casos que piden assets que no subí).
**No se ha compilado con el CMake del repo** (no estaba entero en el contenedor): `cmake/RutinasPnj.cmake` es
como los demás, pero que Claude Cowork lo compile en la VM al integrar.

## Lo que queda / dudas para Oriol

- Los carriles: el contrato dice «nunca pisa»; el encargo dice «no se para, puede cruzarlos». Hice lo segundo.
- «Respirar» con `cicloDeReposo` no vale para los PNJ: sus hojas son de 12 frames (sin reposo). Respiran con
  1 px de vaivén vertical. Si se hornean hojas de PNJ con reposo, basta cambiar `idle_*` en ApplicationBucle.
- `animacionesReducidas` aún no tiene opción en el menú (variable de entorno, como el HUD).
- El aviso «[E] Hablar» y el rombo del suelo van a la casilla del PNJ, no a su dibujo: mientras anda, un poco
  por delante.
