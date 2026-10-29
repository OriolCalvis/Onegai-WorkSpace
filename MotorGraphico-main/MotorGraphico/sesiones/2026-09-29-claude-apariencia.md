# 29/09/2026 — Claude: el creador de apariencias, dentro del juego (APARIENCIA-01)

Encargo de Oriol: «me gusta mucho el motor de crear apariencias de
personajes, intégralo en el juego, así podremos modificar nuestra apariencia
y stats antes de empezar a jugar». El «motor» es el probador de ARTE-02 (el
artifact web y `tools/probador_personaje.py`). Decidió: un paso
**Apariencia justo después de la raza**, con la vista previa acompañando a
todos los pasos, y que el **equipo inicial empiece puesto**.

Todo está **en el árbol, sin commit** (git-safe.sh vive fuera de la carpeta
conectada). Seguimiento: `planificacion/seguimiento/APARIENCIA-01.json`.

## Qué se ve ahora

- **Nueva partida tiene 15 pasos.** Al pasar razas, a la derecha anda cada
  pueblo tal cual viste; en «Qué aspecto tienes» se cambian piel y talla (los
  14 pueblos de cuerpo liso), ojos, peinado y su color, barba y su color, y la
  ropa de diario (cabeza, torso, piernas, pies, capa), más AL AZAR y COMO TU
  PUEBLO. W/S fila, A/D o flechas cambian.
- **La vista previa** es un escenario con el personaje a ×5 andando y girando
  (abajo, derecha, arriba, izquierda) y sus cuatro lados quietos debajo. Al
  pasar clases se ve el equipo inicial de cada una ya puesto; en el reparto,
  lo que se gana: el coloso de ceniza se pone la armadura de placas al subir
  CON a 3 (la pide la carta). El resumen dice con qué empiezas puesto.
- **En partida** el jugador lleva su propia hoja. TAB → Equipo → desequipar la
  armadura deja ver la ropa de diario; volver a equiparla la pone encima.

## Cómo está partido (ley del repo)

GL-free, con pruebas: `motor_imagen` (librería nueva: un PNG a RGBA con una
copia privada de stb_image) y, en `motor_level`, `Game::ManifiestoPersonajes`
(el contrato), `Game::MontajePersonajes` (las capas y la mezcla, con caché
LRU de 96 MB que se vacía al salir de la creación), `Game::EditorApariencia`
(el paso) y `Game::Vestuario` (equipo inicial puesto y piezas visibles). El
paso vive en `Game::NewGameFlow` y no en `CharacterCreationScreen`: la
pantalla es de reglas y la apariencia no cambia ningún número.

Con GL, en `motor_core`: `HojaPersonaje` (sube la hoja a una textura, reparte
los ciclos de andar y, como widget del HUD, pinta la vista previa). Habla con
GL, así que no añade rehenes a la librería mezclada.

`Application.h` no cambia. `Application.cpp` baja 12 líneas (hoy, de 1257 a 1245): el reparto
viejo de `personaje.png` (1-4 abajo, 5-8 arriba, que dejaba al jugador de
espaldas al pararse) se sustituye por `HojaPersonaje::animar()`. Los ciclos
empiezan en el frame quieto ({8, 7, 8, 9} hacia abajo) porque `Player`
descansa en el primero; el `{7, 8, 9, 8}` de los atlas horneados lo dejaría a
medio paso.

## Lo que había que arreglar para que se viera

- **La creación estaba congelada en 1/14** (fallo 1 de las capturas del
  28/09): la escena de título no llama a `update()`, así que nadie refrescaba
  el HUD. Ahora `processInput` llama a `refreshHud()` tras `procesar()`.
- **Título y pistas partidos palabra a palabra**: los `HudText` tenían el
  ancho por defecto (100 px). Ahora llevan el de su caja.
- **La pista decía A/D y solo funcionaban las flechas**: ahora valen las dos.
- **Cierre**: las hojas nuevas son texturas GL; `shutdown()` suelta
  `m_nuevaPartida` antes que la ventana (sin eso, violación de segmento al
  salir).

## Verificación

- `test_personajes_capas` 7/7: los **62 personajes horneados** salen **byte a
  byte** montados en C++; la mezcla coincide con Pillow en 40 pares con alfa
  parcial. Probado por mutación: tocar la mezcla o el orden la pone en rojo.
- `test_editor_apariencia` 8/8 (AL AZAR montable en las 43 razas; flujo de 15
  pasos; `finalizar()` equipando). Las pruebas del flujo de 14 pasos siguen
  verdes sin tocarlas.
- Juego real en el contenedor (Mesa llvmpipe + GLFW null con guion, las
  herramientas de las capturas del 28/09): 33 capturas y un vídeo de 34 s, en
  `Claude outputs/apariencia_2026-09-29/`.
- `linea_base_core.py`: 0 de deuda nueva. Solo falla porque
  `cmake/AparienciaTests.cmake` no está versionado: **va en el mismo commit que
  su `include()`**.
- **Rehecho sobre el árbol del Mac de las 18:00**, con PERGAMINO-01 dentro.
  Desde la foto de la 01:33 otras sesiones habían cambiado o añadido unos 120
  ficheros (PERGAMINO-01, el juicio de clases, H0-07, H0-08, H2-01, H2-08,
  H3-01, H4-01, G-11, AV1-03, la selección de objetivo). De los míos
  coincidían `CMakeLists.txt` y cuatro `Application*.cpp`: dos choques de
  líneas vecinas (en `CMakeLists.txt` y en `ApplicationHud.cpp`), y
  PERGAMINO-01 ya había hecho el mismo arreglo de `refreshHud()` tras
  `procesar()`. Con todo junto compila y en `ctest` fallan las mismas que sin
  este cambio (las de GL sin pantalla y las de la lista de abajo);
  `test_application_shutdown` 4/4 y `test_pergamino` 33/33 con EGL. El juego
  real llega a Boundington con el HUD de pergamino y el jugador con su pelo
  y la armadura de placas; en el cuaderno (TAB → Equipo) sale el equipo
  inicial puesto, y desequipar la armadura o volver a equiparla desde
  Inventario rehace el sprite (capturas 32-34 y `jugador_con_pergamino_zoom.png`).
  La guardia entera da lo mismo con y sin este cambio (la deuda nueva que
  marca hoy es de otras sesiones en vuelo), salvo `Application.cpp`, que baja
  de 1257 a 1245. `medir_alcance.py` nuevo: las seis cabeceras nuevas las
  alcanza `juego`.

## Avisos

- `equipment.json` pone escudos, orbes y focos en `MainHand`: no conviven con
  el arma. El caballero templario empieza con espada y el escudo en la mochila.
  No lo he cambiado: es canon/cartas.
- La apariencia no se guarda: `SaveGame` sigue sin serializar la ficha.
- Visto al probar con el cuaderno: al desequipar la armadura los PV no se
  recortan al nuevo máximo (la cabecera dice 35/29 hasta volver a ponerla).
  Es del desequipar, no del aspecto; con el equipo puesto de salida se ve
  enseguida. No lo he tocado: es de inventario/ficha.
- Ya fallaban antes y siguen igual (comprobado compilando el árbol sin estos
  cambios): `test_application`, `test_application_scenes` y
  `test_g14_hud_timers_commerce` no compilan; `test_catalog_content` y
  `test_gdb12_screens` fallan; `demo_creacion_personaje` espera 61 clases (hay
  63) y `demo_nova_partida` elige `mago_chasqueador`, que no trae equipo
  inicial (ni su trasfondo), así que la mochila sale vacía con o sin este
  cambio.
