# U3-02 · Auditoria de la capa de regiones (U3-02.E01) y alcance propuesto E02..E08

Base: `9d52ae8`. Rutas relativas a `MotorGraphico-main/MotorGraphico/`. Todo `fichero:linea`
citado se abrio en esta E. Sala limpia: nada de esto copia codigo de Unity; los nombres
son del repo (espanol, `Result<T>`, tipos `ids::`). **Esta E no cambia codigo.**

Leyenda de estado: **E** escrito · **I** integrado en un ejecutable real (`juego`, `motor_editor`,
`level_editor`) · **T** probado.

## 1. Inventario de lo que existe

### 1.1 Datos de region

| Pieza | Donde | Estado |
|---|---|---|
| `struct MapRegionDocument { ids::RegionId id; std::string name; GridBounds bounds; }` | `include/Map/RegionLayerDocument.h:13-17` | E I(solo editor) T |
| `class RegionLayerDocument` (`regions()`, `find`, `addRegion`, `removeRegion`, `setRegionBounds`, `validate`) | `include/Map/RegionLayerDocument.h:19-36` | E T |
| `validate()`: id vacio, nombre vacio, bounds vacios, **id duplicado dentro de la capa** | `src/Map/RegionLayerDocument.cpp:20-38` | E T |
| Mutadores con los mismos errores; `Result<bool>` | `src/Map/RegionLayerDocument.cpp:51-90` | E T (`tests/Map/MapDocumentTests.cpp:275-280`) |
| Constructor `RegionLayerDocument(std::vector<MapRegionDocument>)` esta definido en **`src/Map/EntityLayerDocument.cpp:112`**, no en el .cpp de la region | idem | trampa, ver 3 |
| `GridBounds {minX,maxX,minY,maxY}` **ambos extremos inclusive**; `isEmpty()` si `max<min` | `include/Core/Math/GridBounds.h:8-15` | E T |
| `GridCoord {int x, y}` | `include/Core/Math/GridCoord.h:6-9` | E T |
| `ids::RegionId = StrongId<RegionIdTag>` (`operator==`/`<`, `empty()`, `value()`, `parse`) | `include/Core/Ids/ProjectIds.h:25-27,38`; `include/Core/Ids/StrongId.h:35,46-52` | E T |
| La region es una de 4 variantes de capa: `std::variant<Tile,Entity,Region,Event>`; `MapLayerMeta{id,name,visible,locked,opacity,order}` | `include/Map/MapLayer.h:32-42,51` | E T |
| Regiones **dentro del mapa** al anadirse la capa: `minX>=0, minY>=0, maxX<width, maxY<height` | `src/Map/MapDocument.cpp:120-128` | E T |
| Edicion por capa: `MapDocument::addRegion/removeRegion/setRegionBounds(layerId,...)` | `include/Map/MapDocument.h:150-152`; `src/Map/MapDocumentEdits.cpp:94-122` | E T |

**Semantica de frontera hoy:** rectangulo, extremos inclusive (`GridBounds.h:3`). Las
**superposiciones estan permitidas** (ningun codigo las rechaza: `RegionLayerDocument.cpp:20-38`
solo mira ids). La unicidad del id es **por capa**; no encontre comprobacion entre capas
(`MapDocument.cpp:60-98` solo llama `layer.validate()` y los limites). El orden de las
regiones es el del `std::vector` (orden de alta/lectura); el de capas es el de `MapDocument::layers()`
(`include/Map/MapDocument.h:42`). No hay API `regionsAt`/`contains` de ninguna clase (grep
`regionsAt|contains` sin resultados en `src/ include/`).

### 1.2 Formato persistente

`{"type":"region","regions":[{"id","name","bounds":{"minX","maxX","minY","maxY"}}]}`.
Escritura: `src/Project/MapSerializer.cpp:139-155`. Lectura: `src/Project/MapDeserializer.cpp:245-273`
(`readId<ids::RegionId>` linea 253; `name` requerido linea 254). Ida y vuelta probada en
`tests/Map/MapDocumentTests.cpp:136-187` y `tests/Project/SerializationTests.cpp:53`,
`tests/Integration/SchemaFixtureTests.cpp:123-128,433`. **La region no tiene campo alguno que
la enlace a un evento, trigger, intervalo ni politica**: solo `id`, `name`, `bounds`. El
comentario de `RegionLayerDocument.h:10-12` anuncia que se podran anadir propiedades.

### 1.3 Quien las usa (callers)

- Serializacion/edicion/validacion/pruebas: lo citado arriba.
- `RuntimeTileMapBuilder` **ignora** las regiones (`src/Runtime/RuntimeTileMapBuilder.cpp:17` solo
  procesa `MapLayerType::Tile`; su test solo construye una region de adorno, `tests/Runtime/RuntimeTileMapBuilderTests.cpp:29`).
- **Ningun consumidor runtime**: grep de `RegionLayerDocument|MapRegionDocument|MapLayerType::Region` en
  `src/ include/ examples/ editor/` solo da modelo, (de)serializador, edits y validacion. `src/Editor/`
  no tiene herramienta, panel ni dibujo de regiones. Las "regiones" de `Application.cpp:349,465`
  y `ApplicationNivel.cpp:84-122` son regiones de **atlas de texturas** (`defineRegion`), otro concepto,
  y `AlturaRegion` (`src/Level/LevelLoader.cpp:172`) es otra cosa (alturas).
  => regiones: **E T, no I** en ningun ejecutable.

### 1.4 Runner de eventos (el destino de `dispatchRegionTriggers`)

| Pieza | Donde | Estado |
|---|---|---|
| `RPG::MapEventFrame {GridCoord playerCell; bool interactPressed; int deltaMs}` | `include/RPG/MapEventRunner.h:87-100` | E T |
| `RPG::MapEventRunner(NarrativeState&, const vector<string>& inventory, EventInterpreter&)`, `setMap(const MapDocument*)`, `update(frame)`, `blocksPlayer()`, `resetTriggerState()`, `lastStartedEvent()`, `diagnostic()` | `include/RPG/MapEventRunner.h:102-144`; `src/RPG/MapEventRunner.cpp:46-51,221-250` | E T |
| Disparadores: `EventTrigger {Interact, Touch, Autorun, Parallel}` (**no hay Enter/Stay/Exit**) | `include/Map/EventLayerDocument.h:70-75` | E T |
| Touch = al entrar en la **casilla** (compara `m_lastCell`), no mientras se esta; `m_hasLastCell` evita el falso (0,0) | `include/RPG/MapEventRunner.h:56-59,174-178`; `src/RPG/MapEventRunner.cpp:169-190` | E T (`tests/RPG/MapEventRunnerTests.cpp:330 touch_dispara_al_entrar_y_no_cada_frame`) |
| Autorun/Parallel: una vez por pagina activa distinta; **PROVISIONAL pendiente de DEC-10** | `include/RPG/MapEventRunner.h:60-68` | E T (`:358,:385`) |
| Si algo bloquea al jugador no arranca Touch ni Interact (`src/RPG/MapEventRunner.cpp:234-241`); la casilla se registra siempre (`:247-249`). **EXCEPCION: no garantiza una sola escena bloqueante.** `update()` llama a `dispararAutomaticos()` (`:229`) ANTES de mirar `blocksPlayer()` (`:234`), y esa funcion no consulta `blocksPlayer()`: un Autorun puede arrancar encima de una escena bloqueante ya viva (p. ej. una de Touch/Interact del frame anterior). `EventInterpreter::start` solo impide reentrar en el MISMO id (`src/RPG/EventInterpreter.cpp:33-45`) | `src/RPG/MapEventRunner.cpp:221-250` | E T (el hueco no tiene test) |
| `deltaMs` 0 en pausa lo pasa quien llama | `include/RPG/MapEventRunner.h:96-99` | E |
| API para arrancar un evento por id/region: **no existe**; `startPage` y `eventAt` son privados | `include/RPG/MapEventRunner.h:149-158` | gap |
| Fuente en `motor_level` (`CMakeLists.txt:266`); test `test_map_event_runner` (`cmake/MapEventRunnerTests.cmake:20-33`, `unit;gl-free`, 687 lineas de test) | idem | T |

### 1.5 Integracion en ejecutables (hallazgo importante)

- `juego` crea el runner (`src/Engine/Application.cpp:531`) y lo actualiza solo si `m_mapEventRunner->map()`
  no es nulo (`src/Engine/ApplicationBucle.cpp:448-452`), pasando `playerPosition()` y `deltaTime*1000`.
- **Nadie llama `setMap(...)` sobre ese runner** (grep `setMap(` en `src include examples`: solo la
  definicion `MapEventRunner.h:114` e `InputSystem`). Luego `map()` es siempre nulo y
  **`update()` no se ejecuta nunca en `juego`**. Ademas `juego` carga `LevelDefinition`, no `MapDocument`
  (`ToLevelDefinition`/`ApplyToMapDocument` solo se definen en `include/Level/LevelDocumentBridge.h:77-78`
  sin llamadores en `src/ examples/ editor/`). Es decir, **el runner existente tampoco esta integrado**:
  esta escrito y probado pero inalcanzable en el juego. U3-02.E08 depende de resolver esto (ver 3, D2).
- `motor_editor` y `level_editor` no usan regiones (ni runner).

## 2. Alcance de codigo congelado propuesto (E02..E08)

### 2.1 Convenciones reales a respetar

`Result<T>` global (`include/Core/Errors/Result.h:22-40`, `Ok/Error`, `isOk()`); tipos de `Map/` sin
namespace, los de RPG en `namespace RPG` (`MapEventRunner.h:83`); `Game::RutaCasillas` como ejemplo de
modulo GL-free. Propongo `namespace Regiones` para la parte pura (decision menor, 3-D6). Registro de tests
al estilo `cmake/RutaCasillas.cmake` (fichero `.cmake` propio + `include(cmake/X.cmake)` en
`CMakeLists.txt` **en el mismo commit**, ley 3 de `AGENTS.md:36`; ahi se fijan `LABELS "unit;gl-free"`,
`MOTOR_GL_FREE_TARGETS` y `suite_gl_free`). No existe `tests/CMakeLists.txt`. Limites: 500 lineas por
fichero y 15 miembros por cabecera (`AGENTS.md:21-30`); `MOTOR_CHECK`/`require()`, nunca `assert()`.

### 2.2 Parte pura (E02..E05, E07): un par nuevo + una prueba

| Fichero (nuevo salvo indicado) | Contenido |
|---|---|
| `include/Map/RegionPresence.h` | structs de datos y declaracion de funciones libres (≤15 miembros, solo structs y 7 funciones) |
| `src/Map/RegionPresence.cpp` | implementacion; se anade a `add_library(motor_map_document ...)` (`CMakeLists.txt:177-185`, 1 linea). Ya enlaza `motor_json motor_project`, sin GL |
| `tests/Map/RegionTriggersTests.cpp` | casos positivo, frontera y negativo de E02..E05 y E07 |
| `cmake/RegionTriggersTests.cmake` | `if(BUILD_TESTING AND TARGET motor_map_document)` (como `cmake/MapEventRunnerTests.cmake:13`) + `add_executable(test_region_triggers ...)`, `target_link_libraries(... motor_map_document)`, `add_test(NAME test_region_triggers ...)`, `LABELS "unit;gl-free"`, `MOTOR_GL_FREE_TARGETS`, `suite_gl_free` (copia de `cmake/RutaCasillas.cmake`) |
| `CMakeLists.txt` (existente, 2 lineas) | `include(cmake/RegionTriggersTests.cmake)` junto a `CMakeLists.txt:987` + la fuente en `motor_map_document` |

Target/test: **`test_region_triggers`**, comando `ctest -R test_region_triggers` desde el directorio de build
(WORKING_DIRECTORY = `CMAKE_BINARY_DIR`, como el resto). No toca `RegionLayerDocument.*`, ni el
serializador, ni `MapDocument.*` (los tres siguen intactos; sin riesgo para ficheros ≤500).

### 2.3 Firmas propuestas, adaptadas a tipos reales

Identidad compuesta: `RegionId` es unico solo **por capa** (`RegionLayerDocument.cpp:34,63`; `MapDocument`
no impone unicidad global, `MapDocument.cpp:60-98`). Toda presencia/delta/trigger usa `RegionRef`, nunca
`RegionId` suelto.

```cpp
// include/Map/RegionPresence.h  (namespace Regiones; tipos globales: ids::LayerId, ids::RegionId, GridCoord, Result<T>)
struct RegionRef     { ids::LayerId capa; ids::RegionId region; };                      // == y < lexicograficos (capa, region)
struct RegionDelta   { std::vector<RegionRef> entered, stayed, exited; };               // disjuntos
struct StayPolicy    { int intervalMs = 1000; };                                        // por region (D7); sin catch-up
struct RegionPresence{ std::vector<RegionRef> current; std::vector<int> stayMs; bool known = false; };
struct StayBatch     { std::vector<RegionRef> fired; };                                 // como mucho UNO por region y llamada
struct RegionTrigger { enum class Kind { Enter, Stay, Exit }; Kind kind; RegionRef ref; };
struct RegionPolicy  { bool exitsFirst = true; };                                       // PROVISIONAL pendiente de DEC-10 (D3)

bool contains(const GridBounds& b, GridCoord c);                                        // inclusive, base de E02
// E02
std::vector<RegionRef> regionsAt(const RegionLayerDocument&, const ids::LayerId&, GridCoord);  // orden = declaracion
std::vector<RegionRef> regionsAt(const MapDocument&, GridCoord);                         // capas Region en orden de layers(); fuera de [0,w)x[0,h) -> {}
// E03
RegionDelta regionDelta(const std::vector<RegionRef>& before, const std::vector<RegionRef>& now);
//   entered/stayed en el orden de `now`; exited en el orden de `before`
// E04  (D4 aceptado: estado mutable)
Result<StayBatch> stayEvents(RegionPresence&, const std::vector<RegionRef>& now, int dtMs, const StayPolicy&);  // corregido en ronda 1 por hallazgo de Codex: calcula regionDelta(presence.current, now); solo las stayed acumulan/disparan, las exited pierden contador, las entered empiezan en 0; presence.current == now. Error: dtMs<0, intervalMs<=0; dt=0 (pausa) -> Ok vacio y NO acumula; dt grande -> max 1 Stay, resto descartado (sin catch-up entre llamadas)
// E05  (el enunciado omitia el StayBatch; sin el no hay Stay que ordenar)
std::vector<RegionTrigger> orderRegionEvents(const RegionDelta&, const StayBatch&, const RegionPolicy&);
// E07
RegionPresence resetRegionPresence();                                                    // sin parametros (D5): known=false, current={}, stayMs vacio
```

Criterios de aceptacion anadidos:
- **E03:** mover la presencia de `(capaA, "zona")` a `(capaB, "zona")` (mismo `RegionId`, otra capa) produce
  `exited={A/zona}` y `entered={B/zona}`, **no** `stayed`. Sin duplicados con la misma `RegionRef`.
- **E04:** `dt=0` no dispara ni acumula; `dt=5*intervalo` produce a lo sumo un Stay por region (sin catch-up).
- **E05:** orden provisional (DEC-10): Exit, luego Enter, luego Stay (`RegionPolicy::exitsFirst=false`: Enter, Exit,
  Stay); dentro de cada grupo se conserva el orden de entrada (exited = `before`, entered/Stay = `now`, que ya es
  orden de capa en `layers()` y luego de declaracion por `regionsAt`); la funcion no reordena por `RegionId`. Un
  `fired` fuera de `stayed` se ignora. El bloqueo de evento es de E06.

Politica del primer frame (D5, decidido): aparecer dentro de una region (carga, cambio de mapa, teletransporte)
**dispara Enter**. Por tanto `RegionPresence{known=false}` + primera casilla genera `entered`; el cambio de mapa
descarta la presencia anterior (sin Exit sobre el mapa nuevo). `resetRegionPresence` deja `known=false`, `current={}`,
`stayMs` vacio; la primera llamada posterior con las regiones de la casilla de entrada da `entered` de todas y ningun
`exited` del mapa viejo, ni siquiera de una `RegionRef` igual (carga y teletransporte no generan salidas del mapa
antiguo sobre el nuevo). Firma sin parametros: recibir la lista de la casilla de entrada (`atEntryCell`) para
suprimir Enter contradice D5, asi que no existe. Idempotente.
`EntryContext` no existe en el repo; no se crea.

### 2.4 Parte de integracion (E06, E08): solo si se resuelven D1 y D2

`include/RPG/MapEventRunner.h` ya incumple la ley 1b (9 datos + 16 metodos): **no se extiende**. E06 no toca
`MapEventRunner.*`.

| Fichero | Cambio |
|---|---|
| `include/RPG/RegionTriggerDispatch.h` + `src/RPG/RegionTriggerDispatch.cpp` (nuevos, `namespace RPG`, fuente en `motor_level` junto a `CMakeLists.txt:266`) | funcion libre o clase pequena: `Result<int> dispatchRegionTriggers(const std::vector<Regiones::RegionTrigger>&, ...)`, que usa solo la API publica existente del runner/interprete (`EventInterpreter::start(const MapEventDocument&, size_t)`, `blocksPlayer()`, `isRunning`). Si la API publica no basta (el runner no expone arrancar un evento por id y `startPage`/`eventAt` son privados, `MapEventRunner.h:149-158`) se devuelve BLOQUEADA, no se amplia el runner |
| Exclusion con Autorun (obligatoria en E06) | el dispatcher consulta `blocksPlayer()` antes de arrancar y, como el runner puede arrancar un Autorun en el mismo frame (hueco de 1.4), E06 define el orden (region antes o despues de `MapEventRunner::update`) y lo cierra con un **test negativo con dos escenas bloqueantes**: una escena bloqueante viva + trigger de region (y Autorun + trigger de region) => la segunda NO arranca, `TriggerResult` lo reporta |
| `tests/RPG/RegionTriggerDispatchTests.cpp` + `cmake/RegionTriggerDispatchTests.cmake` (`test_region_trigger_dispatch`, `unit;gl-free`, guardia `TARGET motor_level AND TARGET motor_map_document`, enlaza `motor_level motor_map_document motor_rpg motor_json motor_map` como `cmake/MapEventRunnerTests.cmake:13,26-27`) | E06 |
| E08 (`document: demo producto`) | no es codigo congelable todavia: exige que `juego` llame `setMap` y use la capa (1.5); `src/Engine/ApplicationBucle.cpp:448-452` y `Application.cpp:531` son ficheros de cruce de otras sesiones |

## 3. Riesgos, trampas y decisiones para Oriol

**Decisiones (NO decididas aqui):**

- **D1 · Como se enlaza una region con un evento.** Hoy `MapRegionDocument` no tiene campo de enlace
  (`RegionLayerDocument.h:13-17`) ni `EventTrigger` tiene Enter/Stay/Exit (`EventLayerDocument.h:70-75`).
  Opciones: (a) anadir a la region `onEnter/onStay/onExit` = id de evento (cambia formato JSON,
  serializador, deserializador, schema y `FORMATO_NIVELES.md`); (b) anadir 3 valores a `EventTrigger` y que
  el evento diga a que region escucha; (c) solo triggers programaticos sin datos de autor. **Bloquea E06 y E08.**
- **D2 · Camino al juego.** El runner existente no corre en `juego` (1.5): ni `setMap` ni `MapDocument`.
  Hay que decidir si el juego pasa a cargar `MapDocument` o si las regiones se puentean desde `LevelDefinition`.
  **Bloquea E08** y deja E06 como "producido pendiente de integracion" (porta de cierre del paquete).
- **D3 · Orden de simultaneos (E05) — ABIERTA (DEC-10 sigue abierta).** E05 implementa el defecto determinista PROVISIONAL Exit -> Enter -> Stay (`RegionPolicy::exitsFirst=false`: Enter, Exit, Stay); dentro de cada grupo se conserva el orden de entrada (exited = `before`, entered/Stay = `now` / `fired`, ya en orden de capa y declaracion por `regionsAt`), sin reordenar por `RegionId`; `fired` debe venir sin duplicados. No cierra la decision: espera a Oriol (`MapEventRunner.h:60-68`).
- **D4 · Estado mutable en E04 — ACEPTADA por Brain (30/10):** `stayEvents(RegionPresence&, ...)`.
- **D5 · Primer frame / mapa nuevo — DECIDIDA por Oriol (30/10): SI.** Aparecer dentro de una region (carga o cambio de mapa) dispara Enter.
- **D6 · Nombre de namespace** `Regiones` (propuesto) y si `visible=false`/`locked` de la capa excluye sus regiones (propuesto: no excluye, es autoria).
- **D7 · Politica de Stay — DECIDIDA por Oriol (30/10):** Stay cada N ms configurable por region (ej. 1000), sin catch-up (un dt grande da como mucho 1 Stay), en pausa no cuenta.

**Trampas:**

1. Inclusivo en ambos extremos (`GridBounds.h:3`): `maxX` es una casilla valida; un `<` en vez de `<=` rompe la frontera.
2. Dos niveles: `RegionLayerDocument` aislada solo valida bounds no vacios (`RegionLayerDocument.cpp:59,85`), sin conocer el mapa; en cambio `MapDocument::setRegionBounds` (`MapDocumentEdits.cpp:115-123`) pasa por `mutateLayer`, que copia, muta y revalida con `validateLayerForMap` (`MapDocument.cpp:225-237`) y esa rechaza `maxX>=width`/negativos (`:120-128`). Para un `MapDocument` valido las regiones estan dentro; para una `RegionLayerDocument` suelta (tests) no, asi que `regionsAt(RegionLayerDocument,...)` no presupone limites.
3. `RegionId` unico solo por capa (`RegionLayerDocument.cpp:34,63`); el mismo id en dos capas es valido. Por eso la identidad es `RegionRef{LayerId, RegionId}` en todo el pipeline; `regionsAt(MapDocument)` no dedupe por `RegionId` solo.
4. `RegionLayerDocument.cpp:12` incluye `Map/MapDocument.h` y el constructor vive en `EntityLayerDocument.cpp:112`: cualquier reordenacion de fuentes en `motor_map_document` puede romper el enlace.
5. `m_hasLastCell`: el origen (0,0) como primera casilla es legitimo (`MapEventRunner.h:174-178`); `RegionPresence.known` repite la misma leccion.
6. Un `update()` por frame del runner con `deltaMs` entero truncado (`ApplicationBucle.cpp:451`): dt de frames muy cortos puede dar 0 ms repetidos; `stayEvents` no debe perder el resto (acumular, no descartar).
7. `assert()` prohibido en demos y tests (Release define NDEBUG): usar `MOTOR_CHECK`/`require()`.
8. Marca de ley 4 (`AGENTS.md:38`): la parte pura (2.2) sola es codigo "muerto" hasta que E06/E08 la alcance un ejecutable real; que la prueba la llame no la salva. Plan: marcar E02..E05+E07 como "producido pendiente de integracion".
9b. El runner permite un Autorun encima de una escena bloqueante (`MapEventRunner.cpp:229` antes de `:234`); E06 debe cerrarlo con test negativo (ver 2.4). No se modifica el runner en U3-02.
9. Hay trabajo ajeno sin commitear en el checkout principal (`git status` inicial); no se ha tocado. Esta auditoria se hizo sobre el worktree `9d52ae8` limpio.
