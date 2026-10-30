
---
# Archivado el 14/10/2026 (noche) por Claude (coordinador): fichero completo tal como estaba

# Claude Opus 2 — tarea actual (15/10)
**Asignada:** 14/10 (21:30) por Claude, a petición de Oriol («feines que puguem fer sense abordar el que he de revisar»)
· **Modelo:** Opus, esfuerzo **alto** (high) · **Repo:** `MotorGraphico-main` · Lee antes el PROTOCOLO de `orquestador/README.md`.

**Base:** tus ficheros de ahora en el Mac, **sin** los ocho paquetes de `REVISAR.md` (Oriol los revisa el 15/10).
Tu zona no toca ninguno de los 245 ficheros que llevan esos paquetes (salvo `CMakeLists.txt`, y solo por ancla).
Si ves que necesitas otro, **para** y escríbelo en el INFORME en vez de tocarlo. Entrega como siempre: paquete en `MotorGraphico/out/claude_<nombre>/`
con `aplicar.sh` que compruebe la preimagen, fichero de sesión, fila en el TABLERO. No commitees.

## AV13-04 + D0013 — El crítico de dos 6 y la probabilidad exacta de una tirada enfrentada
(Era de opencode, que no se ha lanzado: PO-02.) Dos partes, en este orden:

**1. El crítico pasa a «al menos dos 6»** (ley de Oriol, `decisiones/D0013`; no es una pregunta).
- Regla: **1 dado → nunca crítico · 2 dados → los dos en 6 · 3 o más → basta con dos 6.**
- Hoy `include/RPG/DicePoolEngine.h` / `src/RPG/DicePoolEngine.cpp` (`roll_pool`) pide todos los dados en 6
  (mira el comentario de la línea ~31 y la constante de la ~65).
- Actualiza los comentarios y las pruebas que fijan la regla vieja (`tests/RPG/SkillExecutorTests.cpp`,
  `tests/Game/DiceRollWidgetTests.cpp`).
- **No toques** `include/RPG/DanoEnfrentado.h` ni `examples/` (están en paquetes en revisión): si sus
  comentarios o su lógica dicen «todos 6», apúntalo en el INFORME con fichero y línea, y lo cambia Claude Cowork.
- Pruebas de los tres casos con bordes (3 dados con un solo 6 = no crítico).
- En el INFORME, el número del estudio de dificultad del Tier 1 (GDD §21): con 4 dados el crítico debería pasar
  de ~0,08 % a ~13 %. Sácalo de tus pruebas, no de `examples/simulador_dificultad.cpp` (está en revisión).

**2. AV13-04 — la probabilidad exacta** (`COMBATE_BG3.md` §6).
- **Zona:** NUEVOS `include/RPG/ProbabilidadPool.h`, `src/RPG/ProbabilidadPool.cpp`,
  `tests/RPG/ProbabilidadPoolTests.cpp`, `cmake/ProbabilidadPool.cmake` y su `include()` en `CMakeLists.txt`.
  Sin GL. **`CMakeLists.txt` está en un paquete en revisión:** la línea entra por ancla
  (`aplica_ediciones.py`, como `out/claude_reputacion01/`), y tu paquete se aplica **después** de los ocho.
- **Qué:** pool del atacante y del defensor (0..8 dados cada uno, con ventaja/desventaja de cada lado) →
  probabilidad EXACTA de ganar (diferencia > 0), empatar (roce, D0014: quita 1) y perder; la de crítico (dos 6)
  y la de pifia de cada lado; y la distribución de la diferencia de éxitos (para el daño previsto). Por
  convolución de las caras (6 = 1 éxito, 5 = 0,5; ventaja: los 4 suman 0,5; desventaja: solo los 6), no
  simulando.
- **Hecho cuando:** para todos los pares 0..8 × 0..8, con y sin ventaja/desventaja, coincide con 200 000 tiradas
  de `roll_pool` a menos de 0,5 puntos; pruebas de bordes (pool 0, un dado, dos 6). Paquete
  `out/claude_dados01/`.
- **Para después (no lo hagas):** AV13-05, la ayuda al apuntar, la conecta Claude Cowork en la barra de combate.

---
## INFORME (15/10)
- **Estado:** **hecho** (14/10, tarde). Una prueba de `examples/` falla hasta que se aplique una
  propuesta (ver «Qué tengo que mirar»).
- **Paquete:** `MotorGraphico/out/claude_dados01/`. Va **después** de los ocho de `REVISAR.md`;
  entra por ancla (también la línea de `CMakeLists.txt`).
- **Qué ha cambiado:**
  - `include/RPG/DicePoolEngine.h`: el crítico son **dos 6** (D0013); nueva `es_critico()`;
    `kDadosParaCritico` pasa a `kSeisesParaCritico`.
  - `src/RPG/SkillExecutor.cpp`, `include/RPG/EnhanceHooks.h`, `include/Game/Skill.h`: solo
    comentarios y el texto del registro («DOS 6: EXTRA ×1.5»).
  - `tests/RPG/SkillExecutorTests.cpp`: prueba nueva con los tres casos y los bordes (3 dados con
    un solo 6 = no crítico).
  - `tests/Game/DiceRollWidgetTests.cpp`: el widget enseña «Crític» con 6-1-6.
  - `tests/Game/SkillTests.cpp`, `tests/RPG/SkillEffectTests.cpp`,
    `tests/RPG/SkillEffectRealAssetsTests.cpp`, `tests/Game/MoralTests.cpp`,
    `tests/Game/ActuacionTests.cpp`: comentarios de «todos 6».
  - NUEVOS `include/RPG/ProbabilidadPool.h`, `src/RPG/ProbabilidadPool.cpp`: la probabilidad
    exacta (por convolución, con cuentas enteras) de ganar, empatar (roce) y perder, el crítico y
    la pifia de cada lado, el grado y la distribución de la diferencia de éxitos.
  - NUEVOS `tests/RPG/ProbabilidadPoolTests.cpp` y `cmake/ProbabilidadPool.cmake`, y su
    `include()` en `CMakeLists.txt`.
- **Resultados:**
  - Crítico con **4 dados: del 0,077 % al 13,194 %** (171 de 1296; en 200 000 tiradas, 13,259 %).
    Con 3 dados, 7,4 %; con 5, 19,6 %; con 6, 26,3 % (la tabla de D0013).
  - Los **729 pares** (0..8 × 0..8 dados, con las nueve mezclas de ventaja/desventaja) contra
    200 000 tiradas de `roll_pool`: la mayor distancia es **0,399 puntos** (se pide < 0,5).
  - De 0 a 6 dados, la cuenta coincide caso a caso con todas las tiradas posibles.
  - Con los ocho paquetes + este (compilado de cero): gl-free 225/227 (fallan `linea_base_core`,
    el de siempre, y `demo_dice_engine`), GL 11/11.
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
  ```
  cd MotorGraphico
  bash out/claude_dados01/aplicar.sh
  python3 out/claude_dados01/aplica_ediciones.py out/claude_dados01/propuesta_examples.json   # arreglo de examples/ (opcional, ver abajo)
  cd build && cmake . && ninja
  ctest -R "probabilidad_pool|skill_executor|dice_roll_widget|demo_dice_engine" --output-on-failure
  ./test_probabilidad_pool    # imprime la tabla del crítico y la distancia con 200 000 tiradas
  ```
- **Qué tengo que mirar yo (Oriol):**
  - **`demo_dice_engine` falla** con la regla nueva: `examples/demo_dice_engine.cpp:107` pide que
    4-4-6-6 no sea crítico. `examples/` está en revisión y no lo he tocado; el arreglo está en
    `propuesta_examples.json` (probado: pasa). Lo aplica Claude Cowork o tú.
  - **«Todos 6» en ficheros que no podía tocar** (líneas con los ocho paquetes aplicados):
    `include/RPG/DanoEnfrentado.h:13, 27, 75` (comentarios) y `:41` (el campo `todosSeis`; la
    lógica ya va bien, lo llena `pool.critical`); `include/RPG/Narrative/NarrativeBeat.h:288`;
    `examples/demo_dice_engine.cpp:88, 145, 251`; `examples/ScriptedRng.h:28`;
    `examples/demo_prologo.cpp:103`; `examples/demo_nd6_distribution.cpp:33, 40, 51`;
    `tests/Game/CartasEnCombateTests.cpp:313`.
  - «Gana / empata / pierde» va por la diferencia de éxitos, como pide la tarea. La pifia del
    defensor con empate a 0 sí hace daño (`dadosFisicos`): para la ayuda (AV13-05), mejor sumar
    con `paraCadaPar`, que ya da cada par con su probabilidad.
  - Hay que repetir el estudio de dificultad del Tier 1 con la regla nueva (no lo he hecho:
    `simulador_dificultad` está en revisión).
  - El disco de la VM se llenó un momento al copiar la prueba (lo comparten todos los agentes);
    la borré enseguida y probé en `/tmp`.
- **Fichero de sesión:** [`sesiones/2026-10-15-claude-opus2-dados.md`](../sesiones/2026-10-15-claude-opus2-dados.md)

---
# Historial (tareas anteriores)

**Asignada:** 14/10 (12:31) por Claude, a petición de Oriol («donem diferents tasques per diferents agents»)
· **Modelo:** Opus, esfuerzo **alto** (high) · **Repo:** `MotorGraphico-main`. Lee antes el PROTOCOLO de
`orquestador/README.md`.

## AV15-05a — Confirmar lo caro e irreversible (EH-22, EH-23, EH-29, EH-30)
De `planificacion_alcance_y_vision/EVALUACION_HEURISTICA.md` (Anexo A.3 y A.4), lo primero que recomienda:

- **EH-22 — Pausa:**
  - «Salir» y «Volver al título» piden confirmación con resumen: «Salir sin guardar: perderás lo jugado
    desde las HH:MM». Si no hay nada sin guardar, no hace falta preguntar.
  - ESC en el título sigue cerrando sin preguntar: ahí no hay nada que perder.
- **EH-23 — Cargar y guardar:**
  - «Cargar» avisa de que se pierde lo jugado y dice de cuándo es la partida guardada.
  - Guardar encima de una partida avisa de que la sobrescribe.
- **EH-29 — Comprar una casa:** ENTER sobre COMPRAR ya no paga al instante. Primero sale un resumen (qué
  compras, el precio, el oro que te queda, lo que renta) y se confirma.
- **EH-30 — Revender:** lo comprado se puede revender, con la pérdida a la vista («Recibes 60 de los 120
  que pagaste»).
  - El porcentaje no está decidido: propón el 50 % y pregúntalo en la BANDEJA (PO nuevo).
  - Mientras no conteste, usa el 50 %.

**Cómo:**
- Una confirmación reutilizable, la misma para las cuatro: el papiro de siempre, con «Sí / No» y el foco
  en «No».
- Que sea GL-free (estado y textos), con sus pruebas, y que la pinte el pergamino.
- Sigue la guía de estilo (`GUIA_DE_ESTILO.md`: botones con sus estados, la voz del juego).

**Zona:**
- `src/Engine/Application.cpp`: solo `confirmPauseCommand`, `confirmBusinessCommand` y el `InputAction`
  de la pausa y el negocio. En este caso sí puedes tocar `Application`, siempre por ancla.
- `Game/PauseMenu*`, `Game/GameSessionComercio.cpp` y `GameSession.h`: solo la declaración de
  `sellBusiness`; GameSession.h está en su techo de líneas, así que si hace falta, saca otra línea.
- La pintura de la pausa y del negocio en el pergamino. **No** `InterfazPergamino.cpp` (está a 500):
  ficheros nuevos.
- **No toques** el combate (`BattleState*`, `Cartas*`, `Barra*`, `MesaCombate*`, `ApplicationCinematica*`,
  `ApplicationPantallas.cpp`, `ApplicationHud.cpp`): son de Claude (Cowork) y de Sonnet.

- **Entrega:** como hasta ahora, un paquete con preimagen en `MotorGraphico/out/claude_confirmar01/` que vaya
  **después** de `claude_dialogos04`, `claude_diario01`, `claude_cicatrices01` y `claude_reputacion01`
  (en ese orden: es el orden de `Software/REVISAR.md`). Para los ficheros que también tocan esos paquetes,
  edita **por ancla** (como `aplica_ediciones.py` de `claude_reputacion01`), nunca el fichero entero.
  Prueba el paquete sobre una copia con los cuatro aplicados antes.
- **Reglas del motor:** ningún fichero de `src/` o `include/` por encima de 500 líneas; ninguna cabecera
  con más de 15 miembros `m_`; un `.cpp` sin GL no va en `motor_core`. `python3 tools/linea_base_core.py`
  debe decir «0 de deuda nueva» (en un árbol con `.git`; fuera de él cuenta mal).
- Tu fila en el TABLERO, tu fichero de sesión y el INFORME. Preguntas para Oriol, a la BANDEJA (regla 7).

**Hecho cuando:** las cuatro confirmaciones funcionan en el juego (capturas), con sus pruebas, y se revende.

---
## INFORME
**Hecho (14/10).** Sesión: [sesiones/2026-10-14-claude-opus2-confirmar.md](../sesiones/2026-10-14-claude-opus2-confirmar.md).
- **Pausa (EH-22, EH-23):** «Salir del juego», «Volver al título», «Cargar partida» y «Guardar partida»
  (encima de otra) preguntan en el mismo papiro, con la hora: «Perderás lo jugado desde las 12:31.»,
  «Vuelves a la de las 12:31: pierdes lo de después.», «Sustituye la partida de las 12:31.». Si la
  partida es igual que la guardada, no pregunta. ESC en el título no cambia.
- **Negocio (EH-29, EH-30):** hoja de pergamino nueva (`HojaNegocio`). ENTER en «Comprar» abre un resumen
  («Te quedarán 500 de oro. Te dará 24 de oro por ciclo.»). «Revender» dice la pérdida («Recibes 500 de
  los 1000 que pagaste.»); al **50 %** hasta la **PO-42**.
- **Una sola pregunta** reutilizable y sin GL (`Game/Confirmacion`), «No / Sí» con el foco en «No».
- **Pruebas:**
  - `test_confirmar` (nueva, 8 casos);
  - GL `test_application_scenes` 12/12;
  - gl-free 223/225 (los dos de siempre);
  - `linea_base_core` 0 de deuda nueva (`Application.cpp` baja de 1347 a 1196).
- **Capturas del juego de verdad** (ventana con el GLFW de capturas): `out/claude_confirmar01/capturas/`.
- **Paquete:** `MotorGraphico/out/claude_confirmar01/aplicar.sh`, después de los cuatro de `REVISAR.md`.
  Entra por ancla, y lo he probado sobre ellos.
- `InterfazPergamino.cpp` y `ApplicationHud.cpp` tocados con permiso de Oriol, sin añadir líneas.
- **Queda:**
  - F5 y F9 siguen sin preguntar;
  - la PO-42.

---
# Historial (tareas anteriores, ya hechas)

# (anterior)
**Asignada:** 14/10 (10:16) por Claude, a petición de Oriol («models d'agent… per seguir treballant en paral·lel»)
· **Modelo:** Opus, esfuerzo **alto** (high) · **Repo:** `MotorGraphico-main` · **Tipo:** trabajo nuevo → rama
propia (regla 2 del protocolo). Lee antes el PROTOCOLO de `orquestador/README.md`.

## AV14-22 — El diario de misiones completo (H3-02 + MISIONES-01 juntos)
Oriol (D0015 §16): «unir las dos mitades en un sistema de seguimiento completo y detallado».
Plan: `planificacion_alcance_y_vision/` (fase AV14). Hoy hay dos mitades: la ficha `quests.json` (H3-02,
`QuestLogState`) y la vista de las flags (MISIONES-01, `DiarioMisiones`).

- **Zona (solo esto):** `include/Game/DiarioMisiones.h`, `include/Game/QuestLogState.h` y sus `.cpp`;
  `src/HUD/Pergamino/PaginaCuaderno.cpp` y `PaginasJugador.cpp`; `assets/**/quests*.json`; sus pruebas;
  ficheros NUEVOS que necesites (con su `cmake/<Nombre>.cmake`).
- **No toques** (son del carril de combate de Claude, vivo): `Application*`, `GameSession*`,
  `BattleState*`, `CartasEnCombate*`, `InterfazPergamino.cpp`, `MesaCombate*`, `assets/adventures/`.
  Si necesitas una línea en uno de ellos (por ejemplo, avisar al diario cuando cambia una flag), no la
  escribas: déjala en tu INFORME con el código exacto y dónde va, y Claude la pone.
- **Qué:**
  1. Una sola fuente: cada misión con sus apartados, las decisiones que tomaste y su consecuencia, y lo
     que aún se te pide. Decide tú cómo se juntan las dos mitades y escríbelo en una decisión `D00xx`.
  2. El cuaderno lo enseña así (apartados, decisión → consecuencia, lo pendiente).
  3. Sigue `planificacion_alcance_y_vision/USABILIDAD_UOC.md`: el jugador debe saber en 5 s qué le toca.
- **Reglas del motor:** ningún fichero de `src/` o `include/` por encima de 500 líneas; ninguna cabecera
  con más de 15 miembros `m_`; un `.cpp` sin GL no va en `motor_core`. Comprueba con
  `python3 tools/linea_base_core.py` («0 de deuda nueva»).
- **Hecho cuando:**
  - la misión de Boundington (Campaña 01) se ve entera en el cuaderno con sus decisiones;
  - hay pruebas con Boundington (una decisión tomada sale con su consecuencia);
  - `ctest -L gl-free` sigue como estaba (fallan solo `linea_base_core_configura`, `linea_base_core` y
    `test_marcar_paquetes`, los de siempre).
- Tu fila en el TABLERO y tu fichero de sesión. Si tienes preguntas de diseño, a la BANDEJA (regla 7).

---
## SIGUIENTE (solo si Oriol te lo dice al pegar el mensaje) — AV14-23 — Reputación por facción y rumores
D0015 §15 (D-MOR-09 y D-MIS-03: «sí»). `factionRep` existe y nadie lo lee.
- **Qué:** el precio de la tienda depende de la reputación con su facción, y la página del jugador del
  cuaderno la enseña. Los rumores, como beats con `TriggerType::Talk`: escribe el formato y un ejemplo en
  `FORMATO_AVENTURAS.md`; **no** edites `assets/adventures/` (déjalo propuesto en el INFORME).
- **Zona:** el comercio (`Comercio*`, la tienda), `PaginasJugador.cpp` y sus pruebas.

---
## INFORME
**AV14-22 hecha (14/10, ~11:15).** Sesión: [`sesiones/2026-10-14-claude-opus2-diario-misiones.md`](../sesiones/2026-10-14-claude-opus2-diario-misiones.md) · decisión: [D0016](../decisiones/D0016-diario-de-misiones-una-sola-fuente.md).

- **Cómo se juntan (D0016):** la ficha de `quests.json` da la forma (apartados, decisiones con sus
  consecuencias, fallos) y las flags de la partida dan el estado. La ficha nombra los objetivos de la
  aventura por id, sin copiarlos. `QuestLogState` (progreso aparte, sin uso) se retira: se aparta a
  `out/claude_diario01/retirados/`.
- **Boundington:** 4 fichas, 14 apartados, 3 encargos opcionales, 31 decisiones (96 salidas) y 32
  consecuencias de después, cada una colgada de la flag del beat que la cuenta. Sin inventar lore.
- **Cuaderno:** la misión activa arriba con «Ahora: …»; debajo, sus apartados con su cuenta y cada
  decisión tomada; al elegir una decisión, «Elegiste…», lo que pasó y lo que pasó después.
- **Ficheros** (todos en mi zona): `Game/DiarioMisiones.*`, NUEVOS `src/Game/FichasDeMisiones.cpp`,
  `HUD/Pergamino/PaginaMisiones.*`, `tests/Game/DiarioCompletoTests.cpp`, `cmake/DiarioCompleto.cmake`;
  `RPG/Definitions/DefinicionesMisiones.h`, `RPG/Catalogs/CatalogLoadersMisiones.h`,
  `HUD/Pergamino/PaginaCuaderno.{h,cpp}` (icono y líneas de una fila del detalle), `PaginasJugador.cpp`,
  `assets/catalogs/quests.json`, `tests/Game/QuestLogTests.cpp`, `cmake/QuestLogTests.cmake`, y tres
  líneas de `CMakeLists.txt` (por ancla). **Ninguna línea para Claude (Cowork)**: no ha hecho falta tocar
  `Application`, `GameSession`, `InterfazPergamino` ni las aventuras.
- **Cómo se prueba:** `bash MotorGraphico/out/claude_dialogos04/aplicar.sh` y después
  `bash MotorGraphico/out/claude_diario01/aplicar.sh`; `ctest -R "diario|quest_log"`. En el juego: TAB,
  hoja Misiones. Capturas: `out/claude_diario01/capturas/`.
- **Resultado:** `test_diario_completo` 5/5 (con las aventuras de verdad; una decisión tomada sale con
  su consecuencia), ctest gl-free 222/224 (fallan `test_marcar_paquetes`, el de siempre, y
  `test_ruta_casillas`, que mide tiempo y en un Debug del contenedor no llega a 5 ms: no es de esta
  tarea), `linea_base_core` sin deuda nueva, `juego` compila.
- **Desvío del protocolo:** sin rama propia. Esta sesión solo llega a la carpeta `Software/`, no a
  `~/onegai-ramas/`; se entrega como paquete con preimagen, como hace Claude (Cowork).
- **Falta / para quien siga:** las cicatrices en el cuaderno (lo apuntó la sesión del combate, no
  estaba en esta ficha); quien añada una decisión a una aventura tiene que añadir su salida a la ficha
  (la prueba dice si la flag existe). `out/claude_diario01/_to_delete/` son copias de trabajo mías:
  se pueden borrar.

---
**AV14-23 hecha (14/10, ~12:15; la SIGUIENTE, pedida por Oriol a las 11:23).** Sesión:
[`sesiones/2026-10-14-claude-opus2-reputacion-rumores.md`](../sesiones/2026-10-14-claude-opus2-reputacion-rumores.md).

- **Reputación (D-MOR-09):** la tienda cobra según la facción del tendero (`assets/rules/facciones.json`,
  `Game/ComercioReputacion`, usado por `GameSessionComercio`): `repPriceModPct` acotado a −25 %/+50 %; si
  te detestan, te pagan menos; nunca sale a cuenta revender (la prueba lo mira en todas las tiendas
  publicadas). Se ve en la hoja de la tienda y en un bloque REPUTACIÓN de la ficha del cuaderno.
- **Arreglado de paso:** la reputación no se guardaba (volvía a 0 al cargar). Ahora va en
  `SavedHero::reputacion` y la recupera `NewGameFlow::fichaGuardada` (una línea en `SaveGame.h`,
  `SaveGameFormat.cpp`, `SaveGame.cpp` y `NewGameFlow.cpp`).
- **Rumores (D-MIS-03):** formato y ejemplo en `FORMATO_AVENTURAS.md` («Rumores»). **No** he tocado
  `assets/adventures/`: tres rumores listos para pegar en `out/claude_reputacion01/rumores_propuestos.json`
  (Rosita y las antorchas, Luisarda y las treinta espadas, Aigren y la espada de Gorran), cada uno detrás
  de la charla de su PNJ.
- **Cómo se prueba:** después de `claude_dialogos04` y `claude_diario01`,
  `bash MotorGraphico/out/claude_reputacion01/aplicar.sh`; `ctest -R "comercio|shop|tienda|save_game"`.
  En el juego: compra a Rosita antes y después de un acto que Aegroum vea; TAB → Ficha.
- **Resultado:** `test_comercio_reputacion` 5/5; ctest gl-free 221/223 (los dos de siempre) con el
  `dialogos04` de las 11:42; `linea_base_core` sin deuda nueva; `juego` compila.
- **Los dos paquetes, rehechos para entrar por ancla:** `claude_dialogos04` se ha republicado cuatro veces
  durante estas sesiones. Ahora `claude_diario01` y `claude_reputacion01` no pisan ficheros: cada trozo de
  antes tiene que estar una vez en su fichero, o no se toca nada.
- **Falta:** que un beat pueda pedir reputación (`requires` por facción no existe en
  `NarrativeCondition`); tiendas para la Barriada y los Perdidos (su reputación se ve, pero no cambia
  ningún precio).
- **Ojo al probar sobre un árbol con `tar xzf`:** el tar deja las fechas viejas y `ninja` no recompila lo
  que depende de esas cabeceras; me salieron tres pruebas de combate rompiendo (`free(): invalid size`)
  hasta que hice `touch` de lo extraído y recompilé. No era de ningún paquete.

