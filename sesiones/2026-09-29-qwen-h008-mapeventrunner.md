# 2026-09-29 · qwen · H0-08 (mitad GL-free): `MapEventRunner`, los eventos de mapa se disparan por posición

**Encargo de Oriol**, literal: «coge el gantt del motor graphico y sigue
programando, gracias. utiliza mapadelcodigo, pero piensa que hay mas agentes
trabajando en el proyecto de manera simultanea».

Del Gantt (`MotorGraphico/planificacion/gantt_jugabilidad_H.puml`) cogí **H0-08**,
y no por gusto: es la tarea roja que más cosas bloquea del plan entero. De ella
cuelgan H2-01, H3-01, H3-02, H3-03, H3-07, H4-08, H5-01 y H5-03, y su propia ficha
lo dice sin adornos —«sin esto, la fase E entera (49 de sus 79 jornadas) no llega a
nadie»—. Además ya era mi fila: «Ejecutar eventos de mapa en el juego — mitad
GL-free (`MapEventRunner`)», reclamada el 17/09 y vacía desde entonces.

Repo `MotorGraphico-main`, rama `feature/editor-s1-contratos`, commit **`bdcb3ff`**
(6 ficheros, +1.040 −3). Un solo commit, con los ficheros nuevos y las 9 líneas de
`CMakeLists.txt` aisladas por blob+`update-index`.

| Ruta | Qué |
| --- | --- |
| `include/RPG/MapEventRunner.h` | NUEVO, 190 líneas — el contrato y las decisiones |
| `src/RPG/MapEventRunner.cpp` | NUEVO, 254 líneas |
| `tests/RPG/MapEventRunnerTests.cpp` | NUEVO, 554 líneas, 16 casos |
| `cmake/MapEventRunnerTests.cmake` | NUEVO |
| `CMakeLists.txt` | 9 líneas (fuente en `motor_level` + `include` del test) |
| `cmake/MisionLlaveTests.cmake` | 3 tokens: `motor_slots` → `motor_saves` (ver §3) |

- **Commits:** MotorGraphico `bdcb3ff` (H0-08 mitad GL-free: `MapEventRunner` + 16 casos + el arreglo de la guarda apagada de `demo_mision_llave`). De `CMakeLists.txt`, sucio con H1-03/H1-05 de opencode, entraron 9 líneas aisladas por blob+`update-index`; sus 40 siguen en el árbol.
- **Deja esperando a Oriol:** nada nuevo, pero **DEC-10 gana una segunda pieza que la espera** — la regla de cuándo se repite un Autorun se ha dejado escrita como provisional (una vez por página activa distinta) porque la alternativa literal de RPG Maker congela la partida. Y sigue abierto el montaje en `Application` (§7), bloqueado por fichero sucio con dos filas vivas, que es lo que de verdad cierra H0-08 y con ello 8 tareas del Gantt.

---

## 1. Lo que faltaba, dicho con precisión

La fase E dejó la cadena completa menos el primer eslabón. E-01 guarda eventos en
casillas del mapa, E-03 los ejecuta a lo largo de varios frames, E-04 dice qué
significa cada comando y E-09 elige qué página está activa. Pero
`EventInterpreter::start(event, pageIndex)` recibe **un evento y un índice que
alguien tiene que haber decidido ya**, y ese alguien no existía.

El único sitio del repo donde la cadena funcionaba de punta a punta era
`examples/demo_mision_llave.cpp:236`, y lo hace **por id**:

```cpp
std::size_t interactuar(const MapDocument& mapa, const std::string& id, ...)
```

Que es exactamente lo que un jugador no puede hacer. El jugador está en una
casilla, mira a otra y pulsa acción. Entre esas dos cosas —`"puerta"` y
`(8,4) + acción`— estaba el hueco, y es lo que ahora hace `MapEventRunner`.

**Diseño, y por qué es así.** Recibe `MapEventFrame` (casilla del jugador, flanco
de acción, delta en ms) y resuelve los cuatro disparadores de
`EventTrigger`: Touch al **entrar** en la casilla, Autorun y Parallel solos, e
Interact con la regla de `GameSession::interact()` — primero los pies y luego las
cuatro vecinas, en el mismo orden de `GameSessionInteraccion.cpp:91-92`, porque
una puerta se pisa y a un PNJ se le habla de frente—. Elige página con
`ActivePageWithInventory` (E-09) y hace el tick.

Es GL-free y **no posee nada**: mapa, estado narrativo, inventario e intérprete son
referencias de quien lleva la partida. No conoce `Player`, ni `Application`, ni GL,
ni construye ningún anfitrión. No es una decisión estética: es lo que permite que
quien tiene `Application.cpp` sucio con dos filas vivas encima no tenga que esperar
a nadie, y que la pieza se pruebe entera sin ventana.

Tampoco ejecuta el cambio de mapa. Un `mapa.ir` deja la transición en
`SessionCommandHost::requestedTransition()` y la lee quien orquesta: la misma
división que ya usan `GameSession` y `BattleState` —la sesión decide, el de fuera
orquesta—. Añadir un segundo camino habría sido duplicar un concepto.

## 2. Las cuatro reglas que decide, y la que NO decide

1. **Touch dispara al entrar, no mientras se está.** Sin esto, cada frame de pie
   sobre una placa de presión rearma el evento 60 veces por segundo. Salir y volver
   a entrar sí lo dispara otra vez, que es lo que hace que una placa funcione dos
   veces.
2. **Mientras algo bloquee al jugador no arranca nada nuevo.** Una escena no se
   interrumpe con otra encima. Se vuelve a comprobar después de cada etapa, porque
   arrancar un Autorun cambia la respuesta. Los Parallel no bloquean —esa es toda
   la diferencia entre paralelo y automático— y por tanto no inhiben.
3. **Una condición que no se puede evaluar NO cuenta como falsa.** Se anota en
   `diagnostic()` y no se arranca. Es la regla de E-04/E-09 y es la diferencia entre
   «no tiene la llave» y «no sé mirar el inventario desde aquí»: tratarla como falsa
   convierte una pieza que falta en una puerta que no se abre nunca y que nadie sabe
   por qué.
4. **Las ejecuciones terminadas se recogen al empezar el frame siguiente**, no al
   terminar. Quien llama tiene así su frame entero para leer el desenlace en
   `interpreter.runs()`, y sin esto una partida larga acumularía una ejecución —con
   su copia del evento dentro— por cada pulsación desde el principio de la sesión.

Y la que **no** decide, dejada escrita como provisional:

> **Autorun/Parallel disparan UNA VEZ POR PÁGINA ACTIVA DISTINTA.** Está pendiente
> de **DEC-10** igual que la regla de selección de página, que Oriol no ha
> respondido. La alternativa literal de RPG Maker —rearmar siempre que las
> condiciones se cumplan y no haya ejecución viva— convierte un Autorun cuya página
> no cambia en un bucle que se rearma cada frame y **congela la partida**. Entre un
> cuelgue y un automático que no se repite solo, se elige lo segundo. Cambiarlo es
> cambiar una comparación en `dispararAutomaticos()` y nada más.

No es un detalle menor: es la misma forma que tiene `ActivePageWith` de marcar su
propia provisionalidad. DEC-10 sigue en su bandeja, y ahora hay dos piezas que la
esperan.

## 3. El hallazgo: la prueba de aceptación de TODA la fase E llevaba días apagada, y en silencio

Buscando cómo se cableaban estas piezas a mano llegué a
`cmake/MisionLlaveTests.cmake`, y su guarda no se cumple:

```cmake
if(BUILD_TESTING AND TARGET motor_level AND TARGET motor_map_document AND TARGET motor_slots)
```

`motor_slots` **no existe**. Se llamaba así cuando opencode la creó en H0-05
(«una lib GL-free nueva `motor_slots` = `src/Game/SaveSlotState.cpp`», sesión
20/09) y luego se renombró a `motor_saves` en H0-07. El `.cmake` no se actualizó.

Consecuencia: `demo_mision_llave` —el fichero que en su propia cabecera se llama
«la prueba de aceptación de toda la fase E»— **no se construía, no se registraba en
ctest y no se ejecutaba**. Y nada lo decía: una guarda `if(TARGET ...)` que no se
cumple se salta el bloque sin un solo aviso. Es el mismo mecanismo que tenía E-06,
E-09 y E-16 cerrados en falso, y que la sesión anterior encontró en DEC-09 —
`eventRunning=false` constante y «ninguna prueba falla»—. Quinta vez esta semana.

Está en **HEAD, no en el árbol**: `git show HEAD:MotorGraphico/cmake/MisionLlaveTests.cmake`
ya decía `motor_slots`, y el fichero estaba limpio. Arreglado (3 tokens), y el demo
compila y pasa sus 5 comprobaciones.

**Y barrido el resto**, porque una guarda apagada rara vez viene sola: comparé cada
`TARGET x` de todas las `if(...)` de `cmake/` contra los 252 objetivos que el
configure conoce de verdad. Era **la única**. (El barrido da un falso positivo con
`if(TARGET demo_${demo})`, que es una expansión de variable.)

Lo que merece quedarse escrito no es el nombre mal puesto, es el patrón: **una
guarda de CMake que apaga un objetivo en silencio es un test que puede llevar días
sin correrse y nadie lo echa de menos.** Quien quiera endurecerlo tiene aquí el
caso concreto.

## 4. Verificación

- **`test_map_event_runner` 16/16.** Los cinco primeros son el criterio de cierre de
  H0-08 sobre el **asset publicado** (`assets/misiones/mision_llave.json`) conducido
  **por casillas**: la puerta no se abre sin llave, el cofre da una sola llave aunque
  se insista (gana la página siguiente, E-09), el anciano cambia de frase en cada
  tramo y el altar solo cierra la misión al final. Pulsar acción lejos no hace nada,
  y mantener la tecla no repite la frase.
- **Falsabilidad con 9 mutaciones**, cada una puesta rojos solo los casos que la
  defienden: Touch que se rearma, Autorun que se rearma, vecinas antes que los pies,
  condición no evaluable tratada como falsa, `update()` vacío (13 rojos), bloqueo que
  no inhibe, Parallel que bloquea, sin `collectFinished`, e `interactPressed`
  ignorado. Restaurado y reverificado 16/16 al terminar.
- **Vecinos de la zona de eventos 9/9**: `test_event_interpreter`, `test_event_commands`,
  `test_event_debug`, `test_session_command_host`, `test_event_layer_document`,
  `test_event_editor`, `test_e09_bridge`, `test_diario_misiones` y el nuevo.
- **Suite GL-free completa 171/177.** Los 4 rojos son ajenos y están atribuidos con
  `git status`, no supuestos: `test_catalog_content` (falla en `cat.size() == 492` y
  `== 134`; su propio test y `assets/catalogs/{classes,npcs}.json` están `M`),
  `demo_nova_partida` y `demo_creacion_personaje` (fallan en
  `!hoja.inventoryEquipment.empty()` tras leer «63 classes» del `classes.json` sucio;
  los demos están limpios) y `test_gdb12_screens` (falla en `InventoryScreen::focus()`;
  `InventoryScreen.h/.cpp` están `M`, fila G12 viva de Codex). **En los cuatro el test
  está limpio y el sujeto sucio.** Los otros dos eran `linea_base_core`, ver §5.
- **`linea_base_core` 3/3 en verde tras el commit.** Antes daba 2 rojos con **un solo
  problema instantáneo, el mío**: «`CMakeLists.txt` hace
  `include(cmake/MapEventRunnerTests.cmake)` y ese fichero NO ESTA VERSIONADO... Va en
  el mismo commit que su `include()`». La guardia tiene razón y es la regla que ya
  mordió en E-09: el `.cmake`, la fuente y sus dos líneas de CMake tienen que entrar
  juntos o un clon limpio no configura. Entraron juntos.

## 5. Del `CMakeLists.txt` sucio entraron 9 líneas, y solo 9

`CMakeLists.txt` tiene encima el trabajo de H1-03/H1-05 de opencode (40 líneas: el
`Locale.cpp` a `motor_i18n`, `OptionsLocaleBridge` header-only). Se aisló por
blob+`update-index` como en G-08 y D-01: `git show HEAD:...` → aplicar mis dos
inserciones con asserts de ancla única → `hash-object -w` → `update-index --cacheinfo`.
Comprobado antes de commitear que `git diff --cached` daba **9 inserciones, 0
borrados**, y después que el árbol **sigue** con las 40 líneas de opencode y con
`Application.cpp`, `Application.h` y `GameSession.cpp` `M`.

Un paso que casi me cuesta un disgusto y conviene dejar dicho: **HEAD se movió dos
veces mientras trabajaba** (`c4b939e` y `315957f`, CONTROLES-02/G-12 de Codex). Como
el blob se construye desde `git show HEAD:...`, commitear sobre un HEAD nuevo con un
blob calculado sobre el viejo **revierte el `CMakeLists.txt` del otro**. Volver a
comparar `/tmp/CMakeLists.head` contra `git show HEAD:` justo antes del `update-index`
es lo que lo impide. Barato, y no lo era.

## 6. Lo que no toqué

Ningún fichero sucio, y con gente encima hoy: `Application.*` (H1-03/H1-05,
opencode), `GameSession.*` (H2-05/H2-08, opencode), `InventoryScreen.*` (G12,
Codex), `Player.*` y `AnimatedEntity.*` (CONTROLES-02, Codex — que además
aterrizaron dos commits en mitad de la sesión), `EditorState.*` y `LayerPanel.cpp`
(E-12/H4-09, Codex), los cinco `assets/catalogs/*.json`, `MAPA_DEL_CODIGO.md`,
`TABLERO_ESTABILIZACION.md`, y las piezas de la propia cadena de eventos
(`EventInterpreter.*`, `EventCommand.*`, `SessionCommandHost.*`,
`EventLayerDocument.h`, `MapDocument.*`, `examples/demo_mision_llave.cpp`): se leen,
se usan y no se modifican.

Tampoco `include/RPG/EventosInterno.h`, que sigue sin versionar (EST-32) y al que no
toca nadie.

**No regeneré `planificacion/alcance_real.json`**, aunque la guardia avisa de que se
ha quedado viejo respecto a mi fichero nuevo. Es un artefacto compartido, la fila
H8-01 de Codex dice explícitamente «no se regenera `alcance_real.json` compartido», y
D0007 manda arreglar el instrumento antes que re-congelar. Queda dicho para que nadie
se sorprenda: **`MapEventRunner.h` saldrá como cabecera inalcanzable** en la próxima
regeneración hasta que `Application` la consuma. No es un muerto, es una pieza que
acaba de nacer y todavía no tiene consumidor — que es justo el estado en el que la
fase E entera llevaba días.

## 7. Lo que queda de H0-08, y por qué no lo hizo esta sesión

El montaje en `Application`. Concretamente, y con las piezas ya existentes:

1. Crear el `SessionCommandHost` (sesión + inventario), el `NarrativeCommandHost`
   (estado narrativo + el anterior de fallback) y el `EventInterpreter` sobre ese
   anfitrión. `Application` **ya posee** `m_eventInterpreter` (`Application.h:374`).
2. Cargar el `mapDocument` del nivel —`LevelDefinition::mapDocumentPath` ya se lee
   desde `LevelLoader.cpp:38-42`— y pasárselo con `runner.setMap()`.
3. Cada frame: `MapEventFrame{session.playerPosition(), flanco de InputAction::Interact,
   deltaMs}` → `runner.update(f)`. El flanco, no la tecla mantenida.
4. Leer `runner.blocksPlayer()` para no mover al jugador, y `runner.diagnostic()`
   para enseñarlo.
5. Ejecutar `SessionCommandHost::requestedTransition()` y, al cambiar de nivel,
   `resetTriggerState()`.
6. Y de paso, con el intérprete ya vivo en la partida: **DEC-09**, que la sesión
   anterior dejó reportado y sin tocar — `Application.cpp:943` pasa
   `eventRunning=false` constante a `CanSaveNow` cuando
   `EventInterpreter::canSave()` existe justo para eso. Ahora `canSave()` tiene
   por fin un intérprete con ejecuciones de verdad.

No lo hice porque `Application.cpp` está sucio con dos filas vivas de opencode y lo
tocan **14 paquetes** (`MAPA_DEL_CODIGO.md` §1: es el fichero donde dos agentes se
pisan). Regla 2. El trozo GL-free —el que podía hacerse sin abrir ese fichero— está
hecho, probado y versionado.

## 8. Tres trampas que mordieron y conviene dejar escritas

1. **`TestHarness` escribe `[FAIL]` en `stderr`, no en `stdout`**
   (`tests/TestHarness.h:57`). Un bucle de falsabilidad que solo lea `stdout` cuenta
   **cero rojos** y concluye que la prueba no defiende nada. La primera pasada de las
   9 mutaciones salió entera «0 rojos» por esto, y la conclusión correcta habría sido
   «mi suite es decorativa». Era mentira, pero costó una pasada entera descubrirlo.
2. **`make` compara mtimes con granularidad de un segundo.** En un bucle que muta →
   compila → restaura → muta, dos escrituras dentro del mismo segundo hacen que la
   segunda no se recompile y el test corra **contra el binario de la mutación
   anterior**. Dos de las nueve mutaciones dieron resultado falso así. Se detecta
   exigiendo que la salida del build contenga `MapEventRunner.cpp.o`, y se cura con
   `os.utime(src, ahora+2)`. Es la misma familia que la trampa que ya documenté en
   E-09: **ctest dio PASADO con un binario rancio**.
3. **`ctest` «Not Run» no es «Passed» ni «Failed».** La primera pasada de la suite
   completa dio «14% tests passed, 153 failed out of 177», que leído deprisa es una
   catástrofe y en realidad eran 151 objetivos **sin construir** porque
   `suite_gl_free` todavía no se había compilado en ese build dir. Hay que construir
   la suite antes de contar, y separar las tres columnas.

Y una de método que sí salió bien: **jugar el asset publicado en vez de un mapa
inventado**. Los cinco primeros casos leen `assets/misiones/mision_llave.json` de
disco. Si alguien cambia el serializador, el formato o el contenido, esos casos lo
dicen; con un mapa construido en el propio test habrían seguido en verde sobre un
fichero que el jugador ya no puede cargar.
