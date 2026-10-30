# 2026-09-29 · qwen · H3-01: `WorldTravelState`, el atlas conectado a `LevelTransition`

**Encargo de Oriol**, literal: «Perfecto coge el gantt del motor graphico y sigue
programando, gracias. utiliza mapadelcodigo, pero piensa que hay mas agentes
trabajando en el proyecto de manera simultanea».

Del Gantt (`MotorGraphico/planificacion/gantt_jugabilidad_H.puml`) cogí **H3-01 —
viajar entre naciones** (3 j, verde, depende de H0-08), por cuatro razones medibles:

1. Cuelga de **H0-08**, que cerré ayer por su mitad GL-free (`MapEventRunner`, MG
   `bdcb3ff`): es su continuación natural, y su `mapa.ir` ya usa el mismo canal.
2. Su zona está **limpia y sin filas vivas**: nada de World/Level aparece en las
   filas de «En curso» del TABLERO, y `git status` da limpios `WorldTopology.h`,
   `LevelDefinition.h`, `LevelLoader.cpp`, `ApplicationBucle.cpp`, `demo_mundo.cpp`
   y `WorldTopologyTests.cpp`.
3. Tiene un pendiente viejo del TABLERO esperándola desde el 19/08: «Mapamundi al
   42%: 13 `desembarco_landmass_*` inalcanzables — aplazado a la tarea de viajar
   entre naciones».
4. Conecta una pieza **huérfana**: `WorldTopology` solo la consumían `demo_mundo` y
   sus tests. Regla de Oriol del 28/09 — conectar lo que ya existe antes que crear.

Repo `MotorGraphico-main`, rama `feature/editor-s1-contratos`.
Commit **`4916d92`** (6 ficheros, +866 −0).

| Ruta | Qué |
| --- | --- |
| `include/Game/WorldTravelState.h` | NUEVO, 103 líneas — el contrato y las decisiones |
| `src/Game/WorldTravelState.cpp` | NUEVO, 154 líneas |
| `tests/Game/WorldTravelStateTests.cpp` | NUEVO, 495 líneas, 12 casos |
| `cmake/WorldTravelStateTests.cmake` | NUEVO |
| `examples/demo_mundo.cpp` | +77 líneas — la ruta del criterio de cierre |
| `CMakeLists.txt` | 7 líneas (fuente en `motor_level` + `include` del test) |

- **Commits:** MotorGraphico `4916d92` (H3-01 mitad GL-free: `WorldTravelState` + 12 casos + `demo_mundo` ampliado). De `CMakeLists.txt`, sucio con H1-03/H1-05 de opencode y con **staged ajeno** de ProjectHub/H4-01, entraron 7 líneas aisladas por blob+`update-index`; el staged ajeno se apartó y se restauró idéntico, re-apuntado al HEAD nuevo (§6). Repo raíz: esta sesión + la fila del TABLERO + su fila en `sesiones/INDICE.md`, los tres aislados por el mismo procedimiento.
- **Deja esperando a Oriol:** nada nuevo, pero dos cosas de esta tarea son suyas: (1) los `name` de los ocho `mundi_landmass_*` los pone `gen_mapamundi.py` y dos son **literalmente iguales** («Mistarium»), así que el menú de viaje tiene dos filas indistinguibles — es canon/contenido, no código; (2) si `conectividad.py` debe contar los viajes como aristas además de los pasos andando: mientras mida solo alcanzabilidad andando, el mapamundi seguirá «al 42 %» aunque el jugador ya pueda ir a las ocho masas. Y sigue abierto el montaje en `Application` (§7), bloqueado por ficheros sucios con filas vivas, que es lo que de verdad cierra H3-01.

---

## 1. Lo que faltaba, medido y no supuesto

La ficha del Gantt dice «nadie conecta el atlas con `LevelTransition`», y eso podía
significar dos cosas muy distintas: que faltaran los datos o que faltara el runtime.
Medí antes de escribir.

**Los datos ya están.** BFS por `targetLevel` sobre `assets/levels/*.json`, y
`tools/validar_mundo_unido.py` (EXIT 0):

- 93 niveles alcanzables desde Boundington, 191 puertas, «todo se alcanza y de todo
  se vuelve».
- Desde `ciudad_centro.json` se llega al mapamundi, a los 8 `mundi_landmass_*` y a
  las 7 naciones (`on_bomengrid/havargruztak/ostad/umedan`,
  `es_dhin_thyraxion/naka_tol/taurengrad`).
- Fuera del grafo quedan exactamente los 5 niveles que el validador excluye a
  propósito (fixtures, `interior_vacio`).

**Y el runtime ya sabe ejecutar un viaje:** `Application::applyPendingTransition()`
(`ApplicationBucle.cpp:112`) lee `GameSession::pendingTransition()` y hace
`loadLevel(ruta, hasEntryPosition, entryPosition)`.

**Lo que no existe es cómo se cruza el mar:**

- `mapamundi.json` tiene 14 desembarcos; 8 declaran `targetLevel`, 6 no.
- Su `playerStart` es (20,87) = `desembarco_landmass_1`. El flood fill de
  `tools/conectividad.py` alcanza **1 de 14**: las otras 13 puertas están en islas
  disjuntas, y por eso se reportan como AVISO aplazado a esta tarea.
- Consecuencia en la partida: el jugador llega al mar y lo único que puede hacer es
  desembarcar en la masa de la que vino. Las otras siete naciones existen, tienen
  mapa y tienen puertas, y no hay forma de decidir ir a ellas.

El hueco era exactamente uno: **quién decide el viaje cuando no se puede andar hasta
la puerta**. Eso es `WorldTravelState`, y no hace falta nada más: la transición que
produce es la que `Application` ya ejecuta.

## 2. La pieza y las tres reglas que decide

`Game::WorldTravelState::open(atlas, nivel, ruta)` convierte las puertas del nivel
actual en una lista de destinos navegables; `confirm()` devuelve el
`LevelTransition`. GL-free, sin I/O y sin poseer nada: atlas y nivel son referencias
de quien lleva la partida. Misma división que `MapEventRunner` — la pieza decide, el
de fuera orquesta — y por el mismo motivo: quien tiene `Application.cpp` sucio con
dos filas vivas encima no tiene que esperar a nadie.

1. **Un viaje sin vuelta no se puede tomar.** `hasReturn = atlas.canReach(destino,
   nivelActual)`; si es falsa, la fila se ve con «(sin retorno)», se explica en
   `diagnostic()` y `confirm()` devuelve `pending=false`. Es la regla 2 de
   `validar_mundo_unido.py` («una puerta sin retorno deja al jugador encerrado y
   carga sin dar ningún error») aplicada por fin en la partida, y no solo en el
   validador que corre en CI.
2. **Orden por nombre, no por orden de fichero.** Un menú de viaje se memoriza; si
   sigue el JSON, cualquier reorden del autor —o un `gen_mapamundi.py` regenerado—
   mueve las filas bajo el cursor. Desempate por `objectId`, porque `std::sort` no
   es estable y hoy hay dos masas que se llaman igual.
3. **Lo que no se puede tomar se ve y se explica, no se oculta.** Borrar la fila
   dejaría al autor sin la única señal de que su nivel nuevo no tiene puerta de
   vuelta. Es el mismo principio que apliqué ayer en `MapEventRunner` con las
   condiciones no evaluables.

Y lo que **no** decide: cuándo se abre el menú. En un nivel corriente las puertas ya
se usan andando, y ofrecerlas aquí sería viaje rápido — otra decisión, otro paquete.

## 3. Reutilizar en vez de crear (regla de Oriol del 28/09)

Busqué la pieza similar antes de escribir, y apareció:

- **`Game::MenuLogic`** (en producción desde H1-03) hace la navegación: no hay un
  segundo wraparound en el motor.
- **`WorldTopology`** da los nombres (`WorldNode::name`) y el retorno (`canReach`):
  la pieza no vuelve a leer niveles ni duplica el grafo.
- La salida es **`LevelTransition`** (`Game/SessionTipos.h`), el canal que
  `Application` ya ejecuta: no nace un segundo mecanismo de viaje.
- El contrato de HUD está calcado de **`Game::OptionsState`** (`labels()`,
  `displayLines()`, `confirm()`), para que quien cablee pinte con lo que ya tiene.
- `tools/analizar_duplicados.py --comprobar`: OK, 32 casos plantados vistos, y en el
  árbol real **0 SOLAPAMIENTO_FUNCIONAL**.

## 4. Verificación

- **`test_world_travel_state` 12/12** (0,04 s), registrado en CTest con etiqueta
  `unit;gl-free`. Nueve casos con fixtures que el propio test escribe y borra
  (mismo patrón que `WorldTopologyTests.cpp`) y **tres sobre los assets publicados**,
  que son el criterio de cierre: `el_mapamundi_ofrece_las_ocho_masas_que_tienen_mapa`
  (14 desembarcos, 6 sin destino, 8 destinos con nombre y con vuelta),
  `ningun_destino_del_mapamundi_deja_al_jugador_encerrado` (los 8 confirman y llevan
  a donde dicen; `diagnostic()` vacío) y `de_boundington_al_mapamundi_a_otra_nacion_y_de_vuelta`
  (la ruta entera, salto a salto, terminando en `ciudad_centro.json`).
- **Falsabilidad con 10 mutaciones**, todas cazadas y cada una solo por los casos que
  la defienden: sin orden alfabético (4 rojos), `hasReturn` siempre cierto (1),
  ignorar la entrada declarada (1), `confirm()` sin la regla de retorno (1), sin
  error cuando no hay destinos (1), sin comprobar que el nivel esté en el atlas (1),
  puertas sin destino contadas como destino (1), `label` = ruta en vez del nombre del
  atlas (4), deduplicar destinos por nivel (1), `displayLines()` sin el aviso (1).
  Con `os.utime(+2 s)` antes de cada compilación y exigiendo ver
  `WorldTravelState.cpp.o` en la salida del build (las dos trampas de ayer).
  Restaurado y reverificado 12/12 al terminar.
- **`demo_mundo` ampliado y verde** (0,02 s): imprime el menú de viaje real del
  mapamundi y recorre la ruta del criterio de cierre emitiendo `LevelTransition` de
  verdad. Los bucles de navegación van acotados a propósito: `moveDown()` envuelve y
  un destino ausente convertiría el demo en un cuelgue dentro de CTest.
- **Suite GL-free completa: 179 tests, 173 Passed, 0 Not Run, 6 Failed** — los seis
  atribuidos con `git status`, no supuestos:
  | Rojo | Motivo medido | Test | Sujeto |
  | --- | --- | --- | --- |
  | `demo_creacion_personaje` | `CHECK cc.classes().size() == 61`, el catálogo da 63 | limpio | `assets/catalogs/classes.json` `M` |
  | `demo_nova_partida` | `!hoja.inventoryEquipment.empty()` (misma causa) | limpio | `classes.json` `M` |
  | `test_catalog_content` | `cat.size() == 492` y `== 134` | **`M`** | `classes.json`/`npcs.json` `M` |
  | `test_gdb12_screens` | `InventoryScreen::focus()`, `labels.size() == 4` | limpio | `InventoryScreen.h/.cpp` `M` (fila G12 de Codex) |
  | `linea_base_core_configura` | 1 CONFIGURACION: mi `.cmake` sin versionar | — | mío, se cura con el commit |
  | `linea_base_core` | idem | — | idem |
  Los cuatro primeros son los mismos cuatro de ayer: en todos el test está limpio y
  el sujeto sucio. Ninguno toca nada de World/Level.
- **`juego`, `level_editor` y `motor_editor` compilan** (EXIT 0): la fuente nueva
  entra en `motor_level` y no cambia ninguna firma existente.
- **Guardas:** `comprobar_fragmentos_cmake.py` no señala mi `.cmake` (sí uno ajeno,
  §5); `analizar_duplicados.py --comprobar` OK.

## 5. Hallazgos que no arreglo

- **Los nombres del menú son inútiles para el jugador.** Los `name` de los ocho
  `mundi_landmass_*` empiezan todos por «Mapamundi (2000 b.f.) — …», así que el menú
  enseña ocho filas casi idénticas y **dos literalmente iguales**: landmass_7 y
  landmass_8 se llaman las dos «Mistarium». La pieza desempata por `objectId` para
  que el orden sea determinista, pero el jugador no puede distinguir esas dos filas.
  Es del generador (`gen_mapamundi.py`) y del canon, no de la pieza. Propuesta
  concreta: que el `name` de cada masa sea el de la masa («Aeon», «Mistarium Norte»)
  y la fecha viva en otro campo. **No lo toco**: son assets publicados y el generador
  no tiene fila mía.
- **6 desembarcos (9-14) sin `targetLevel`:** el canon tiene 14 masas
  (`Onegai-Core/atlas/world_grid.json`, 26×26) y solo 8 tienen mapa. La pieza ofrece
  las 8 y no inventa las 6; el test lo afirma sobre el asset publicado.
- **`comprobar_fragmentos_cmake.py` da 1 rojo que no es mío:**
  `cmake/G15ExportPanelTests.cmake` existe (`??`, ExportPanel en vuelo) y
  `CMakeLists.txt` no lo incluye → su test no llega a CI. `linea_base_core` añade una
  docena de AVISOS del mismo estilo (`ApplicationMenuJugador.cpp`,
  `PantallaNuevaPartida.cpp`, `Codice.cpp`, `Companeros.cpp`, `DiarioMisiones.cpp`,
  `FichaVista.cpp`, `Iconos.cpp`, `HudFicha/HudIconList/HudIconos.cpp`,
  `EditorIconos.cpp` y sus tests): todo trabajo de otra sesión en vuelo, sin
  versionar. Zona ajena: se documenta y no se toca.
- **`MAPA_DEL_CODIGO.md` no se actualiza:** se regenera desde `pla/dades.py`,
  reservado por la fila ALPHA-00 de Codex. `WorldTravelState` entrará en la próxima
  regeneración.
- **`alcance_real.json` NO regenerado**, igual que ayer: artefacto compartido, la
  fila H8-01 de Codex dice explícitamente que no se regenera y D0007 manda arreglar
  el instrumento antes. Aviso para quien lo regenere: `WorldTravelState.h` saldrá
  como cabecera inalcanzable hasta que `Application` la consuma. No es un muerto, es
  una pieza que acaba de nacer.

## 6. El índice con 7 ficheros staged ajenos: cómo se commiteó sin tocarlos

A las 08:36, al ir a commitear, `git diff --cached` en `MotorGraphico-main` daba
**7 ficheros staged que no son míos** (H4-01/ProjectHub: `ProjectHub.h`,
`ProjectHub.cpp`, `LevelEditorHub.cpp`, `cmake/ProjectHubTests.cmake`,
`tests/Editor/ProjectHubTests.cpp`, `FORMATO_PROYECTOS.md` y 3 líneas de
`CMakeLists.txt`), y el mtime de `.git/index` era **42 segundos anterior**: otra
sesión estaba a punto de commitear. Un `git commit` mío sin más se los llevaba
enteros (regla 2), y des-stagiarlos para siempre habría roto su commit. Monté un
monitor sobre `git rev-parse HEAD` y esperé.

**Seis horas después su staged seguía ahí, intacto y sin commitear**: la sesión se
fue sin cerrar. Con el trabajo propio verificado y el índice ajeno quieto, la
secuencia fue esta, con copia de seguridad primero:

1. `git ls-files -s` de las 7 rutas → sus SHAs staged, guardados.
2. `git reset -q -- <las 7 rutas>`: apartar **solo su staged**. Un reset mixto por
   ruta no toca el árbol ni HEAD, así que su trabajo en disco no se movió.
3. `git hash-object -w` + `git update-index --cacheinfo` de mis 6 ficheros, con
   `CMakeLists.txt` como **blob construido** = `HEAD:CMakeLists.txt` + mis 7 líneas
   (nunca el fichero del árbol, que lleva 40 líneas de opencode). Verificado
   `git diff --cached --numstat` = `7 0` antes de seguir.
4. `herramientas/git-safe.sh commit -F …` → **`4916d92`**, 6 ficheros, 866
   inserciones, **0 borrados**.
5. Restaurar su staged: los 6 ficheros con sus SHAs originales, y `CMakeLists.txt`
   re-apuntado a un blob **nuevo** = `HEAD_nuevo:CMakeLists.txt` + **sus** 3 líneas.
   Comprobado al final: `git diff --cached` vuelve a dar sus 7 rutas y su
   `CMakeLists.txt` sigue siendo `+3/-0`.

El paso 5 es el que importa y el que es fácil de olvidar. Si su staged se restaura
al blob viejo —calculado sobre el HEAD anterior—, su próximo commit **revertiría
mis 7 líneas**: es el patrón que ya mordió con `TitleFlowTests.cmake` y sigue
abierto en «Pendiente». Restaurar re-apuntando al HEAD nuevo conserva su intención
exacta sin conservar la mía dentro.

Dos cosas se movieron en mitad del proceso, y las dos las cazó reverificar en vez
de suponer:

- **HEAD se movió justo antes del commit.** Otro agente aterrizó `ddad0d7` (el
  gancho de pre-commit mirando el índice) entre mi preparación del índice y el
  `commit`. Llevaba un `test "$(git rev-parse HEAD)" = <sha esperado>` en la cadena:
  falló con exit 1 y **el commit no se ejecutó**. Reconstruido sobre el HEAD nuevo;
  `ddad0d7` no toca `CMakeLists.txt`, así que el blob seguía siendo válido — pero
  eso se comprueba (`git show --stat`), no se asume.
- **El gancho de pre-commit cambió ese mismo día.** Ahora corre
  `linea_base_core.py --rapido --indice`: mira lo que se COMMITEA y convierte en
  FALLO las fuentes sin versionar de tu propio commit. Lo ejecuté a mano antes
  (OK) en vez de descubrirlo dentro del hook. Mi `.cmake` y su `include()` van en
  el mismo commit, que es justo lo que la guarda exige.

## 7. Lo que queda de H3-01: el montaje

La pieza está lista y probada; el jugador todavía no puede viajar. Falta el montaje,
que hoy choca con la regla 2 porque tres de sus cuatro ficheros están sucios con
filas vivas:

1. `include/Engine/Input/InputMap.h` — **limpio**: añadir `InputAction::ToggleTravel`
   y su tecla (el enum ya tiene `MenuUp/MenuDown/Confirm/Cancel`, que es todo lo que
   el menú necesita una vez abierto).
2. `include/Engine/Application.h` — **`M`** (H1-03/H1-05 de opencode + MenuJugador en
   vuelo): un miembro `std::optional<Game::WorldTravelState> m_travel`.
3. `src/Engine/Application.cpp:1018 processInput()` — **`M`**: tres ramas (abrir,
   navegar, confirmar/cancelar).
4. Ejecutar el viaje. **No hace falta pasar por `GameSession`**: no tiene setter
   público de transición (solo `pendingTransition()`/`clearTransition()`, y la
   escritura vive en `GameSessionInteraccion.cpp`, sucio con H2-05/H2-08). Lo limpio
   es hacer en `ApplicationBucle.cpp` (**limpio**) lo mismo que ya hace
   `applyPendingTransition()`: `m_scenes.requestReplace(makeScene(false, [...] {
   return loadLevel(t.levelPath, t.hasEntryPosition, t.entryPosition); }))`.
5. Pintar el menú: `ApplicationHud.cpp` (**`M`**), con `displayLines()` y el mismo
   `HudCommandMenu` que ya usan PauseMenu y OptionsState.

Con `open()` cargando el atlas una vez por nivel (no por frame) y `WorldTopology::load`
midiendo 0,02 s sobre los 93 niveles del mundo, no hay coste que justifique una caché.

## 8. Criterio de cierre: qué está y qué no

El criterio de la ficha es «**Desde Boundington se llega al mapamundi y a otra
nación, y se vuelve. `conectividad.py` en verde**».

- La ruta existe, está medida y la ejecuta la pieza sobre los assets publicados:
  `ciudad_centro --camino_a_egaroth--> mundi_landmass_1 --regreso_al_mundo-->
  mapamundi --VIAJE--> mundi_landmass_2 --acceso_es_dhin_thyraxion--> otra nación`,
  y la vuelta completa hasta `ciudad_centro`. Demostrado en `test_world_travel_state`
  y en `demo_mundo`.
- **Pero el jugador todavía no puede hacerla**: falta el montaje del §7. Y
  `conectividad.py` seguirá reportando los 13 desembarcos como AVISO aplazado, porque
  lo que mide es alcanzabilidad andando y el viaje no es andar — ese aviso solo
  cambiará cuando se decida si el validador debe contar los viajes como aristas
  (propuesta: que `conectividad.py` acepte un segundo modo «alcanzable andando **o
  por viaje**», y entonces el mapamundi pasa del 42% al 100%).

Por eso la fila vuelve a «Hecho» como **mitad GL-free**, igual que H0-08 ayer, y no
como tarea cerrada. Lo que queda es un montaje de cuatro ficheros, tres de ellos con
gente encima hoy.
