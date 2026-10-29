# 05 · Engine — juicio clase a clase

**Ámbito.** Las 12 cabeceras y 18 clases de `include/Engine/`. Todas llegan solo al
ejecutable `juego`, salvo `Window.h`, que llega a los tres. Es el área más pequeña
en cabeceras y la más grande en responsabilidad: aquí vive `Application`.

---

## Los cuatro hallazgos del área

### 1. `Application.h` está exactamente en su techo: 519 de 519

`GameSession.h` estaba a dos líneas; **`Application.h` está a cero**. La guardia
marca `techo 519` y la cabecera tiene 519 líneas, con **109 miembros** y **441
líneas de cuerpo de clase**, seis `.cpp` de implementación y cinco pruebas.

No se puede añadir ni una línea sin subir el trinquete, y subirlo es decisión de
Oriol (`decisiones/D0007`). O sea que el fichero más importante del motor está hoy
**bloqueado para cualquier cambio que crezca**, y los cinco que quedan en esta área
pasan casi todos por él.

El diagnóstico es el mismo que en `GameSession` y se puede decir con números: seis
`.cpp` no parten una clase, parten su implementación. Los 109 miembros siguen en la
misma cabecera y cuentan contra el mismo techo. Lo que sí existe ya en el área es el
patrón correcto —`ApplicationLoop.h` sacó el bucle a cuatro estructuras de entre 9 y
16 líneas, y `ApplicationInternal.h` las constantes— y lo que falta es aplicarlo a
los otros cuatro temas (nivel, HUD, pantallas, cierre).

### 2. C-16 se cumple; las siete escenas de G-08…G-17 no existen

*(Reescrito el 29/09. La versión anterior decía que `SceneManager` estaba sin
conectar: miró los cuatro métodos de la lista «sin llamar» —`requestPush`,
`requestPop`, `activeMapId` y `stackDepth`— y generalizó a la clase entera, que es
justo lo que el índice §3.21 advierte que esa lista no permite.)*

C-16 dice: «**El bucle principal empeny i treu escenes en comptes de mirar
banderes.** El canvi de nivell i el pas menú↔joc són transicions d'escena.»

Eso está hecho desde el 07/09 (`d0fe012`, `seguimiento/C-16.json`, 10 casos en
`test_application_scenes`). `Application` tiene un `SceneManager` (`m_scenes`) y un
adaptador, `ApplicationScene` (`ApplicationBucle.cpp:65`), con dos escenas: «Title» y
«Game». El cambio de nivel (`ApplicationBucle.cpp:117`) y la vuelta al título
(`Application.cpp:1279` y `:1335`) son `requestReplace`; las transiciones se aplican al
final del frame (`finishSceneFrame`), y el bucle llama a `fixedUpdate` y `render` a
través de la escena activa. En el binario del juego del 28/09 hay 40 símbolos de
`SceneManager` y 26 de `ApplicationScene`.

Lo que no existe es lo que el plan pide **además** de C-16. G-08, G-09, G-10, G-11,
G-12 y G-17 nombran siete ficheros de escena —`TitleScene.cpp`,
`CharacterCreationScene.cpp`, `PauseScene.cpp`, `SaveLoadScene.cpp`,
`GameOverScene.cpp`, `InventoryScene.cpp` y `OptionsScene.cpp`— que nunca se crearon
(`marcar_paquetes.py` los lista como «citados por el plan y NO existen en disco»).
Pausa, opciones, inventario, guardados, game over y creación de personaje son modos
con banderas **dentro** de la escena «Game», y la pila (`requestPush`/`requestPop`)
no la usa nadie.

**Decisión de Oriol (29/09): manda el plan, una escena por pantalla.** Las que se
superponen al juego (pausa, opciones, inventario) se apilan con push/pop; las demás
sustituyen a la anterior. Y es el camino natural para sacar estado de `Application.h`
(hallazgo 1): cada escena se lleva sus miembros. El detalle está en `08-diseno.md` §1.2.

### 3. G-17 tiene toda su API de verificación y nadie la llama

`Locale.h` (167 líneas, 5 pruebas, y una de las 9 cabeceras que la base congelada da
por MUERTA mientras el juego la alcanza) tiene **ocho métodos sin llamar**:
`checkPlaceholders`, `format`, `isValidKey`, `loadFromStrings`, `missingKeys`,
`orphanKeys`, `placeholdersOf`, `referenceLanguage`.

Y no son ocho métodos cualesquiera, son justo los que G-17 pide:

- «amb **reserva a l'idioma de referència**» → `referenceLanguage` sin llamar.
- DEC-15: «el text visible admet accents i ñ amb **cobertura de font comprovada**» →
  `missingKeys` y `orphanKeys` son esa comprobación, y nadie las llama.
- `PlaceholderMismatch` y `checkPlaceholders` existen para detectar claves cuyas
  plantillas no encajan entre idiomas → sin llamar.

O sea: el sistema de idiomas puede auditarse a sí mismo y no lo hace nadie. Lo que
sí funciona es el camino feliz (`text()`, `setLanguage()`, `language()`), que es lo
que prueba `test_options_locale_bridge` y lo que cableó H1-05 hoy.

### 4. La pantalla de Nueva Partida no está en git y no tiene pruebas

`PantallaNuevaPartida.h`: 85 líneas, **0 pruebas**, **SUCIO**, y la guardia avisa de
que `src/Engine/PantallaNuevaPartida.cpp` **no está versionado**: «configura en el
arbol de quien lo escribio y en ningun clon». `cerrar` y `nombresSugeridos` sin
llamar.

Y no es una pantalla más: es el único alcanzador de `RPG::CharacterCreation`, o sea
que **toda la creación de personaje llega al jugador a través de un fichero que no
existe en un clon**. Encadenado: `Application` → `PantallaNuevaPartida` →
`CharacterCreation` → `CharacterCreationScreen` (que tiene sus ocho métodos de
navegación sin llamar, ver `03-game.md`) → `NewGameFlow` (1 prueba).

---

## `include/Engine/Application.h` — 519 líneas · juego · 5 pruebas · **techo 519** · 109 miembros · **SUCIO**

### `Application` (línea 79, 441 de cuerpo)

- **Qué hace.** El bucle de juego completo: `init`/`run`/`shutdown`/`processInput`/
  `update`/`render`. Une todo lo construido en las fases anteriores —ventana, cámara,
  tilemap, renderer y jugador (1-3), nivel y catálogo desde JSON (6/10), habilidades y
  combate por turnos (7/8)— y posee la sesión, el intérprete de eventos, el
  guardado, el HUD, el locale y el gestor de escenas.
- **Qué debería hacer.** Dos criterios impresos. **C-05:** «textures i shaders
  s'alliberen mentre el context és viu. Sortir del joc no deixa cap crida GL després
  de `glfwTerminate()`». **C-07:** «qualsevol aventura carregada s'enganxa a la sessió.
  La bandera de campanya només decideix si es fan les transicions de la cronologia,
  **no si la narrativa existeix**».
- **Qué se lo impide.**
  - El techo (hallazgo 1): cero líneas de margen.
  - Las siete escenas que piden G-08…G-17 no existen, y sus modos siguen siendo
    miembros y banderas de `Application` (hallazgo 2; C-16 sí está cumplido).
  - **C-07** está en el aire por el otro extremo: `NarrativeEngine::fire` no tiene
    consumidor en producción y `GameSession::talkNarrative` tampoco, así que la
    narrativa se engancha a la sesión (`setAdventure`, `Application.cpp:351`) y los
    beats no se disparan. C-07 dice explícitamente que la bandera de campaña no
    decide si la narrativa existe; hoy la decide, porque fuera del prólogo no se
    dispara nada.
  - Cinco pruebas para 441 líneas de cuerpo. Es la ratio más baja del motor en la
    clase con más responsabilidad.
  - Solo un método sin llamar (`narrativeWiring`), que es llamativo en una clase de
    109 miembros: casi todo lo público se usa. El problema no es API muerta, es
    tamaño.
- **Cómo optimizarla.** Es la pregunta más cara del proyecto y la respuesta no es una
  refactorización sino una decisión: **`Application` no puede seguir creciendo y el
  techo dice que ya no crece más.** Lo que sí se puede hacer sin decidir nada grande
  es continuar lo que `ApplicationLoop.h` empezó: los seis `.cpp` ya separan los temas
  por fichero, así que cada tema puede convertirse en una estructura de datos más un
  colaborador, igual que el bucle. El orden lo da el riesgo: `ApplicationHud` y
  `ApplicationPantallas` son los que menos tocan GL; `ApplicationNivel` es el que más.
  Y antes que nada, C-05 y C-07 necesitan cada uno su caso de prueba, que hoy no
  existe: cinco pruebas no cubren ni el cierre ordenado ni el disparo de beats.

---

## `include/Engine/Locale.h` — 167 líneas · juego · 5 pruebas · la base la da por MUERTA

### `LocalizedText` (línea 81, 17 de cuerpo)

- **Qué hace.** El resultado de pedir un texto. «Nunca es solo una cadena: el que
  pinta necesita saber si lo que le dan es lo que pidió» — lleva la cadena y la
  bandera `missing`.
- **Qué debería hacer.** G-17 y DEC-15.
- **Qué se lo impide.** Nada. Es la decisión correcta y está escrita: devolver un tipo
  enriquecido en vez de una cadena es lo que permite que `OptionsLocaleBridge`
  compruebe `result.missing`.
- **Cómo optimizarla.** Nada.

### `PlaceholderMismatch` (línea 100, 7 de cuerpo)

- **Qué hace.** Una clave cuyas plantillas no encajan entre idiomas.
- **Qué debería hacer.** DEC-15: la cobertura comprobada.
- **Qué se lo impide.** `checkPlaceholders` no la produce nunca desde el juego, así que
  el tipo existe y no se instancia. Es exactamente el mismo defecto que
  `DynamicLightMap`/`PointLight`: el dato bien diseñado sin productor.
- **Cómo optimizarla.** Que la validación de assets la llame: hay una herramienta de
  comprobación de contratos de cargadores en `tools/`, y esta es de la misma familia.

### `Locale` (línea 108, 58 de cuerpo)

- **Qué hace.** La tabla de textos por idioma con clave estable, la reserva al idioma
  de referencia y el cambio en caliente sin reiniciar.
- **Qué debería hacer.** G-17: «els textos visibles surten d'una taula per idioma amb
  clau estable i reserva a l'idioma de referència. El selector viu a Opcions (G-10) i
  el canvi s'aplica sense reiniciar», con DEC-15 y pendiente de DEC-14.
- **Qué se lo impide.** Ocho métodos sin llamar (hallazgo 3). El selector y el cambio
  en caliente sí están cableados desde hoy (H1-05, `applyOptionsLanguage()` desde
  `adjustOptionsLeft/Right()`), y `loadFromDirectory` también; lo que no se usa es
  toda la mitad de auditoría.
- **Cómo optimizarla.** `loadFromStrings` sin llamar es la puerta de entrada de las
  pruebas: si no la llama nadie, los cinco casos de `test_locale` están cargando desde
  disco, que es más frágil. El resto no es optimización sino cierre de G-17: una
  pasada que llame a `missingKeys`/`orphanKeys`/`checkPlaceholders` sobre los assets
  publicados y reporte, que es literalmente «cobertura de font comprovada».

---

## `include/Engine/Scene/SceneManager.h` — 71 líneas · juego · 3 pruebas · C-16

### `SceneManager` (línea 23, 47 de cuerpo)

- **Qué hace.** La pila de escenas con transiciones diferidas: `requestPush`/
  `requestPop` devuelven `Result<bool>` y la transición se aplica en un punto seguro
  del frame, no en mitad del bucle.
- **Qué debería hacer.** C-16, que es uno de los criterios mejor formulados del plan:
  «el bucle principal empeny i treu escenes **en comptes de mirar banderes**».
- **Qué se lo impide.** Nada en la clase: está conectada desde el 07/09 y hace el
  reemplazo entre título y juego y el de nivel (hallazgo 2, corregido el 29/09). Lo que
  no se usa es la pila: `requestPush`, `requestPop`, `stackDepth` y `activeMapId` solo
  tienen consumidores en las pruebas, porque ninguna pantalla es todavía una escena
  propia.
- **Cómo optimizarla.** No se toca: se usa entera. Con la decisión del 29/09 (una
  escena por pantalla) la pila deja de ser API muerta: pausa, opciones e inventario son
  push/pop sobre el juego. Y cada modo que sale de `Application` a su escena se lleva
  sus miembros, que es lo que necesita el hallazgo 1.

---

## `include/Engine/Scene/Scene.h` — 36 líneas · juego · header-only · 1 prueba

### `Scene` (línea 13, 22 de cuerpo)

- **Qué hace.** El contrato mínimo de una escena (RUN-03). La carga puede fallar; el
  resto del ciclo no devuelve errores porque se ejecuta por frame, y una escena se
  activa solo después de completar `load()`.
- **Qué debería hacer.** C-16.
- **Qué se lo impide.** Que solo lo implementa un adaptador, `ApplicationScene`, que
  envuelve el juego entero en dos escenas. Un contrato bien pensado para una pila que
  todavía no se empuja.
- **Cómo optimizarla.** La decisión de que solo `load()` devuelva error es correcta y
  está justificada por el coste de por-frame. Nada que cambiar; lo que faltan son las
  siete escenas del plan (hallazgo 2).

---

## `include/Engine/ApplicationLoop.h` — 64 líneas · juego · 1 prueba

### `ApplicationLoopSettings` (11, 13 de cuerpo) · `ApplicationLoopFrame` (25, 11) · `ApplicationLoopServices` (37, 9) · `ApplicationLoop` (47, 16)

- **Qué hacen.** El bucle extraído de `Application` en cuatro piezas: configuración,
  datos del frame, servicios a los que llama y el bucle mismo. Ninguna supera las 16
  líneas de cuerpo.
- **Qué deberían hacer.** Sin paquete. **Propuesta:** C-16, porque el bucle es justo lo
  que C-16 describe.
- **Qué se lo impide.** Una prueba para las cuatro. Y que sea el único tema de
  `Application` que se extrajo de verdad: los otros cinco siguen siendo miembros y
  métodos de la clase grande.
- **Cómo optimizarla.** Es el **modelo a seguir** para el hallazgo 1 y conviene decirlo
  así: 64 líneas y cuatro tipos donde había un trozo de una clase de 441. Si se
  replicara para nivel, HUD, pantallas y cierre, `Application.h` bajaría de su techo
  sin decidir nada nuevo.

---

## `include/Engine/PantallaNuevaPartida.h` — 85 líneas · juego · **0 pruebas** · **SUCIO** · `.cpp` sin versionar

### `PantallaNuevaPartida` (línea 34, 50 de cuerpo)

- **Qué hace.** La pantalla que abre el asistente de creación de personaje y conecta
  con `RPG::CharacterCreation`.
- **Qué debería hacer.** G-09: «Nova partida obre l'assistent existent, amb
  tornar/cancel·lar i resum… sense fitxers intermedis que el jugador hagi de copiar a
  mà».
- **Qué se lo impide.** Todo a la vez: cero pruebas, fichero sucio, `.cpp` sin
  versionar, y `cerrar` sin llamar —que es la mitad de «tornar/cancel·lar» de G-09—.
  Y es el único alcanzador de `CharacterCreation` (hallazgo 4).
- **Cómo optimizarla.** No se puede juzgar el código que no está en git. Lo primero es
  versionarlo con su línea de CMake en el mismo commit, que es lo que la propia guardia
  pide; lo segundo, el caso de G-09 de punta a punta.

---

## `include/Engine/ApplicationInternal.h` — 83 líneas · juego · header-only · **0 pruebas** · sin paquete

- **Qué hace.** Las constantes internas del dibujo: `kTileH`, `kCharacterDrawW`,
  `kCharacterDrawH`, `characterAnchor`. No declara clases.
- **Qué debería hacer.** Sin criterio. Y aquí conviene decirlo claro: **son dimensiones
  de dibujo hardcodeadas en una cabecera**, que es exactamente la clase de cosa que
  Oriol pidió revisar («si algo importante está hardcodeado se cambia»).
- **Qué se lo impide.** Que el tamaño del personaje y su anclaje sean constantes de
  compilación y no datos del nivel o del atlas. Los tres ejecutables dibujan con las
  mismas cifras, así que un tile de otro tamaño requiere recompilar.
- **Cómo optimizarla.** Que salgan del documento de mapa o del atlas, igual que
  `TileMap` ya lee `getTileWidth()`/`getTileHeight()`. Mientras sean constantes, al
  menos deberían tener una prueba que fije la relación entre ellas (el anclaje contra
  la altura del tile), porque hoy cero pruebas las cubren y un cambio de signo no lo
  delata nada.

---

## `include/Engine/Input/` — tres cabeceras, el diseño de entrada

### `RawInputState` (`InputMap.h`, línea 54, 8 de cuerpo)

- **Qué hace.** El estado físico de las teclas recogido durante un frame. «Se puede
  construir en tests, GLFW o cualquier otra plataforma sin introducir sus cabeceras en
  el núcleo del motor.»
- **Qué debería hacer.** Sin paquete. **Propuesta:** el paquete de controles (la fila de
  codex del 27/09 en el TABLERO es de controles).
- **Qué se lo impide.** `isDown` sin llamar desde el juego, que es su único método.
- **Cómo optimizarla.** La decisión de no meter GLFW en el núcleo es la correcta y está
  escrita. Si `isDown` no se llama, o el estado se consulta por otro camino o sobra.

### `InputMap` (`InputMap.h`, línea 63, 14 de cuerpo)

- **Qué hace.** La traducción de tecla física a `InputAction`.
- **Qué debería hacer.** Sin paquete. **Propuesta:** ídem, y es la pieza que hace
  posible que `PlayerControls` (Game) sea configurable.
- **Qué se lo impide.** `isBound` sin llamar. Y la relación con `PlayerControls.h` (93
  líneas, sucio, 1 prueba) y `PlayerControlsData.h` no está escrita en ningún sitio:
  hay dos representaciones del mapa de controles.
- **Cómo optimizarla.** Decidir cuál de las dos manda. Es el mismo defecto que
  `Game::SkillCatalog` contra `Catalogs::SkillCatalog`.

### `InputSnapshot` (`InputSystem.h`, línea 12, 17 de cuerpo)

- **Qué hace.** El snapshot inmutable de acciones de un frame. «La distinción entre
  pressed, held y released evita que cada consumidor implemente su propio detector de
  flancos y permite que los tests simulen secuencias completas de entrada.»
- **Qué debería hacer.** Sin paquete.
- **Qué se lo impide.** **`released` sin llamar.** La clase existe precisamente para que
  nadie reimplemente la detección de flancos, y uno de los tres flancos no lo lee
  nadie.
- **Cómo optimizarla.** Comprobar quién detecta «soltar la tecla» en el juego: si hay
  un consumidor que mira `held` dos frames seguidos, está reimplementando justo lo que
  `released` ya da.

### `InputSystem` (`InputSystem.h`, línea 30, 24 de cuerpo)

- **Qué hace.** Produce el snapshot a partir del estado físico y del mapa.
- **Qué debería hacer.** Sin paquete. **Propuesta:** el paquete de controles.
- **Qué se lo impide.** Dos pruebas para las dos clases. Nada más: cero métodos sin
  llamar, que es buena señal.
- **Cómo optimizarla.** Nada.

---

## `include/Engine/Window.h` — 42 líneas · **los tres ejecutables** · 11 pruebas · sin paquete

### `Window` (línea 16, 27 de cuerpo)

- **Qué hace.** Envuelve la creación de ventana y contexto OpenGL 3.3 Core (GLFW) más
  la carga de punteros GL (GLAD). Es «deliberadamente tonta»: no sabe nada de
  `Application`, `Camera` ni render; solo posee la ventana y el contexto y expone
  `pollEvents`/`swapBuffers`/`shouldClose`. Fallo de inicialización = `EngineException`.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-05, porque es la dueña del
  contexto cuya vida ordena el cierre.
- **Qué se lo impide.** Nada. `hasInputFocus` sin llamar en los dos editores, que es
  razonable: el foco lo necesita el juego.
- **Cómo optimizarla.** Es la única cabecera de `Engine` que llega a los tres
  ejecutables y la mejor proporcionada del área: 27 líneas de cuerpo, 11 pruebas, una
  responsabilidad y el motivo de ser tonta escrito. Nada que cambiar.

---

## `include/Engine/InputState.h` — 13 líneas · juego · header-only · 7 pruebas

### `InputState` (línea 8, 6 de cuerpo)

- **Qué hace.** La entrada cruda de movimiento para `Player::handleInput`. Fija el
  contrato mínimo de un movimiento de cuatro direcciones sobre el grid, y no sabe nada
  de GLFW.
- **Qué debería hacer.** Sin paquete.
- **Qué se lo impide.** Que su único consumidor sea `Player::handleInput`, que a su vez
  **no lo llama el juego** (ver `01-render.md`). O sea: 7 pruebas para un contrato de
  entrada cuya función receptora está huérfana. Y hay un tercer mapa de entrada en
  `Engine/Input/`, así que el área tiene dos sistemas de entrada: el crudo
  (`InputState` → `Player::handleInput`) y el de acciones (`InputSystem` →
  `InputSnapshot`).
- **Cómo optimizarla.** Decidir cuál de los dos es el camino. `InputSnapshot` con sus
  tres flancos es claramente el más capaz; `InputState` es el anterior. Retirar el
  viejo junto a `Player::handleInput` es un solo cambio.

---

## `include/Engine/MenuJugadorHud.h` — 51 líneas · juego · header-only · **0 pruebas** · **SUCIO** · `.cpp` sin versionar

### `InventoryOverlay` (línea 25, 7 de cuerpo)

- **Qué hace.** El estado del overlay de inventario del menú de jugador, más
  `refreshInventoryOverlay`.
- **Qué debería hacer.** Sin paquete. **Propuesta:** G-12 (inventario y ficha
  utilizables) o GDB-12.3, que es lo que `InventoryScreen` ya cubre por su lado.
- **Qué se lo impide.** Cero pruebas, fichero sucio y `src/Engine/ApplicationMenuJugador.cpp`
  **sin versionar** — la guardia lo canta en cada commit. Y solapa con
  `InventoryScreen.h` (115 líneas, 2 pruebas, nueve métodos sin llamar): dos
  representaciones del inventario visible.
- **Cómo optimizarla.** Versionar primero. Después, decidir si el overlay y la pantalla
  son la misma cosa, que es lo que el mapa sugiere al ponerlas en áreas distintas
  (`Engine` y `Game`) para el mismo concepto.
