# H2-01, H2-03, H2-02 y H3-06 — el dado, los estados, los objetivos y el reloj se ven, y «Volver al título» ya no revienta · 05/10/2026

Sesión de Claude (Cowork). Sigue a `2026-10-05-claude-ui-bg3-juego.md`. Oriol: «Continua»
(seguir el Gantt). Con la barra de combate ya en el juego, cuatro paquetes de la fase H
que tenían la mitad sin GL hecha y el montaje bloqueado encajaban en el HUD nuevo.

La entrega, con capturas del juego real y un GIF, está en
`Claude outputs/h2_dado_estados_2026-10-05/LEEME.md`.

## 1. «Volver al título» (pausa y derrota)

Reventaba: soltaba la sesión y el `update()` del mismo fotograma la usaba (estaba anotado en
H2-06 como «encontrado sin arreglar»).

- Ahora la partida se rehace de cero en el cambio de escena, al final del fotograma:
  `Application::partidaDeCero()` (en `ApplicationPantallas.cpp`) carga el nivel de `init()`
  (`m_nivelInicial`) y lo deja todo como al arrancar.
- Aplicado en el Mac antes que lo demás: `Application.h`, `Application.cpp` y
  `ApplicationPantallas.cpp`, con la pre-imagen y la post-imagen md5 coincidiendo en los 3.
- Probado en el juego real desde la pausa y desde «Has caído», con una partida nueva después.

## 2. H2-01 — el dado se ve

```
RPG::TiradaVisible   la foto de una tirada: qué, quién, contra quién, los dados de los dos, el bono y el grado
CartasEnCombate      tirar()/tirarSinRival() dejan ultimaTirada() (numerada)       → VistaBarra::tirada
NarrativeState       ponTirada()/ultimaTirada(); GameSession::resolveSkillChecks la deja con cada prueba
DadosPergamino       las caras de d6, el panel del combate y la tira del diálogo (motor_pergamino, sin GL)
BarraCombate         brilloTirada(): la tirada nueva brilla 1,2 s
DialogoHoja          miraTirada(): la tirada nueva va con el diálogo que se abre, en su última página
```

- **Combate:** el panel de la última tirada va entre el registro y la bandeja.
  - Cabecera: qué se tira y el grado.
  - Una fila por lado: el nombre, los dados y los éxitos. Si el rival no tiene dados, pone
    «sin dados».
  - Las caras cuentan como el reglamento (Ed. 3):
    - el 6 en verde;
    - el 5 en ámbar (medio éxito);
    - con ventaja, el 4 también en ámbar;
    - con desventaja, el 5 no cuenta.
  - **La pifia**: los 1 en rojo, el marco del panel rojo y «¡PIFIA!» en una caja roja. Perder
    la enfrentada da «FALLO», que no es pifia.
- **Diálogo:** la tira con «Percepcion [dados] 0,5 contra [dados] 0 ÉXITO» va encima de la
  hoja, en la página que dice «(Tirada de Percepcion contra el vacio.)». El resultado que
  sigue ya no la lleva.
- La pieza de texto de opencode (`HUD/DiceRollWidget`) sigue igual. El combate en pergamino
  usa esta. Las etiquetas son las del registro de la mesa (Fallo/Parcial/Éxito/Crítico), no
  las catalanas de `BattleState`.

## 3. H2-03 — los estados con icono y rondas

```
Game::EstadosCombate   estadosDe(batalla, aliado, i): los de la mesa + las condiciones de la ficha,
                       con icono, rondas, a favor o en contra y su texto (motor_level, sin GL)
VistaBarra             estadosHeroe, estadosEnemigos (los rellena vistaDeBatalla)
BarraCombate           Zona::Estado, estadoSenalado(), senalaNada()
MesaCombateEstados     las fichas en la placa y bajo el retrato del héroe, y su emergente
```

- Cada estado es una ficha con su icono y el borde rojo si va en contra (verde si va a
  favor). En la esquina lleva las rondas que le quedan: bajan cada ronda y la ficha se va
  cuando se acaban.
- El icono:
  - el de la condición, si es del canon (paralizado, envenenado...);
  - si es de una carta, el de lo que hace: el daño por turno sangra, la marca marca...
- **Dónde salen:**
  - en la placa del enemigo, con su nombre (uno o dos; con tres o más, solo las fichas y
    «+N»);
  - los del héroe, bajo su retrato del orden de turno, que crece para ellos.
- Con el ratón encima, o con un clic, sale un emergente: el nombre, lo que queda y lo que
  hace.
- `BattleState::conditionCatalog()` y `CartasEnCombate::catalogo()` son lecturas nuevas.
  `Application::teclasCombate` apaga el emergente cuando el ratón sale.
- `HUD/ConditionWidget` (opencode, texto, solo la ficha) sigue igual.

## 4. H2-02 — a quién apunta la carta

```
CartasEnCombate::alcance()   la regla de a quién alcanza un objetivo: la usa jugar() y la usa la barra
VistaBarra                   aliadosVivos, enemigosVivos
BarraCombate                 mueveObjetivo() (W/S), objetivoDe(), afectados(), Zona::Objetivo  (BarraObjetivos.cpp)
Jugada                       objetivo → Application::confirmBattleCommand(fila, objetivo)
MesaCombateRetales           retícula carmín en la placa y el retrato de los afectados
```

- La carta señalada (la primera pulsación) marca a quién alcanzaría con una retícula
  carmín (cuatro esquinas) en su placa y en su retrato. El marco entero sigue siendo de
  quien juega.
  - Las de un objetivo marcan al elegido; sin elegir, al primero en pie, como antes.
  - Las de área marcan a todos los que siguen en pie; las tuyas, a ti.
- W/S, arriba/abajo o el ratón (sobre la placa o el retrato) eligen a otro. Un clic sobre
  un marcado juega la carta.
- **Nada se gasta hasta jugarla**: mirar otra carta es volver atrás. Lo comprueban las
  pruebas (la carta no queda gastada, el turno sigue abierto y el registro no cambia).
- La carta grande dice a quién va y «(W/S: otro)» si hay a quién elegir.
- En el juego los combates aún son de uno contra uno (`startBattle` mete un enemigo); con
  varios ya funciona (probado con dos lobos).
- Para que cupiera sin pasar del umbral, los relojes de la barra van agrupados
  (`Rotulo`, `TiradaVista`): `BarraCombate.h` baja a 13 miembros.

## 5. H3-06 — el temporizador de misión

```
MissionTimerWidget   relojesVisibles() y formatoReloj(): la regla de qué se ve, sacada para compartirla
HudRetales           DatosExploracion::relojes; filas de reloj en el retal del objetivo (o en uno suyo)
InterfazPergamino    EstadoJuego::timers (los de la aventura activa), relojes desde el NarrativeState
ApplicationHud       estado.timers = m_narrative.adventure()->timers
```

- El retal del objetivo crece una fila por reloj: «TIEMPO», lo que queda y la barra si la
  pide (ámbar; rojo el último cuarto, o el último medio minuto si no hay total). Sin
  objetivo seguido, el reloj va en un retal suyo.
- La regla de qué se ve es la de opencode, sacada a `relojesVisibles()`, y su widget de
  texto la usa: sus 11 pruebas siguen igual.
- La congelación es la de siempre (`updateNarrativeTimers`).
- Juego real con una copia del prólogo con un reloj de 25 s en barra, solo en el
  contenedor:
  - 0:24 y 0:19 explorando;
  - 6 s de combate y al huir 0:18;
  - la cuenta hasta 0:00, en rojo el último cuarto;
  - el beat de expirar, como diálogo.
- `TimerDefinition` no tiene nombre: el retal dice «TIEMPO».

## Verificación

- Pruebas nuevas en verde:
  - `test_pergamino` 60/60 (`DadosPergaminoTests`, 3 casos de estados y 1 de objetivos en
    `MesaCombateTests`, y 1 del reloj en `PergaminoPantallasTests`);
  - `test_mission_timer_widget` 12/12;
  - `test_barra_combate` 18/18 (con `EstadosCombateTests`: el enemigo envenenado enseña
    el icono y el contador 3, 2, 1 y se va; y con `ObjetivosBarraTests`, contra dos lobos);
  - `test_cartas_combate` 32/32;
  - `test_game_session` 26/26;
  - `test_narrative_variables` 10/10.
- ctest en el contenedor: 203/211, las pruebas con GL con `EGL_PLATFORM=surfaceless`. Las 8
  que fallan son las mismas que con UI-BG3-01:
  - `demo_nova_partida`, `test_gdb12_screens` y `test_catalog_content`;
  - `test_g14_hud_timers_commerce`, que no enlaza;
  - `test_application` y `test_application_cableado`, que revientan al cerrar en el
    destructor de `Texture`;
  - `test_application_scenes`, con 4 fallos, como ayer;
  - `linea_base_core`.
- La guardia entera da las mismas 44 deudas.
  - `Application.h` tiene 518 líneas (el techo es 519).
  - `BarraCombate.h` se queda en 13 miembros (el umbral es 15).
- Juego real con OpenGL por software:
  - un combate contra el saqueador: tu tirada, la del rival, una carta y el registro oculto;
  - Canción del Otoño en el saqueador (2 → 1 → se acaba);
  - Invocación Elemental en Nerea (2 → 1 → se acaba), con los dos emergentes;
  - la prueba de Percepción del prólogo;
  - los objetivos: Atacar marca al saqueador, Bendición a Nerea y Canción del Otoño a
    todos; un clic en la placa la juega.
- Antes de aplicar en el Mac, su árbol de las ~19:35 UTC (otra sesión estaba con las
  razas y PNJ-02) se trajo al contenedor, se fusionó, se compiló y se probó: 202/211.
  - Las mismas 8 de antes.
  - Una más, `demo_creacion_personaje`, por el recuento de razas, que es trabajo en curso
    de esa sesión.
- Aplicado en el Mac en dos paquetes, sin commit y con la pre-imagen y la post-imagen md5
  coincidiendo:
  - H2-01 y H2-03: 35 ficheros, en `out/claude_h2/`;
  - H2-02: 15 ficheros, en `out/claude_h202/`;
  - H3-06: 9 ficheros, en `out/claude_h306/`.

## Pendiente

- **El commit.**
  - Van juntos los ficheros nuevos y sus líneas de CMake: `src/Game/EstadosCombate.cpp` y
    `src/Game/BarraObjetivos.cpp` en `motor_level` (CMakeLists.txt), y
    `DadosPergamino.cpp` y `MesaCombateEstados.cpp` en `cmake/Pergamino.cmake`.
  - Las pruebas: `DadosPergaminoTests.cpp` en `test_pergamino`, y
    `EstadosCombateTests.cpp` y `ObjetivosBarraTests.cpp` en `test_barra_combate`.
- **Visto al ver los dados** (para Oriol, sin tocar):
  - Nerea recién creada ataca con 1 dado (base 0 + 1).
  - El saqueador defiende con 0 («sin dados») y ataca con 1.
  - Los combates del principio son casi a cara o cruz.
- Con varias tiradas en una acción (un área), solo se ve la última. El registro las tiene
  todas.
- Fuera del combate aún no se ven las condiciones que duran más allá de él.
- Los nombres sin tilde («Percepcion», «Cancion del Otono») vienen del catálogo.
- No se marca al enemigo en el mundo, solo en su placa y en su retrato.
- Los temporizadores no tienen nombre (el retal dice «TIEMPO») y hoy ninguna aventura
  tiene uno visible.
- `DiceRollWidget`, `ConditionWidget` y `TargetSelection` (de opencode) siguen sin uso en el
  juego. La guardia los cuenta como muertos, y también `MissionTimerWidget`, pero en este
  caso porque la foto de `alcance_real.json` es anterior: ahora `juego` lo alcanza a
  través de `HudRetales`.
