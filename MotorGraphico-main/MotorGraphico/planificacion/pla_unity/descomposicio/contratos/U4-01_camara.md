# U4-01.E01 · Auditoría de Camera y contrato v1 (cámara virtual)

Estado: **documento de E01; no autoriza ningún cambio de código.** Base `628f417`
(`replica-r1/U3-07`, con la utilidad de interpolación ya integrada en `Camera`).
Todas las rutas son relativas a `MotorGraphico-main/MotorGraphico/`. Cada afirmación lleva
`fichero:línea` del HEAD actual; lo que es inferencia va marcado **«inferido»**. Sala limpia:
nada de Unity ni de Cinemachine copiado; los nombres son del repo (español, como U3-07).
Ficha de origen: `out/fabrica/REPLICA-R1/U4-01_ficha.md` (raíz del worktree). Consumidor
obligatorio: `Política pura de cámara → Render::Camera → juego`. Puerta: TEMPS_TWEEN.

## 1. Auditoría del estado actual

### 1.1 `Camera` hoy (`include/Render/Camera.h` 103 líneas, `src/Render/Camera.cpp` 116)

| Qué | Dónde | Unidad / espacio |
|---|---|---|
| Clase `Camera : IUpdatable` | `Camera.h:21` | global, sin namespace |
| Ctor con viewport en píxeles | `Camera.h:23`, `Camera.cpp:29-30` | píxeles de **pantalla/ventana** |
| `update(float dt)`: transición activa → reloj float + curva; si no, **lerp exponencial** `t=clamp(8*dt,0,1)` | `Camera.cpp:32-67` (lerp `:65-66`; `m_lerpSpeed=8` en `Camera.h:88`) | segundos; posición en píxeles de mundo |
| `move(delta)`: suma al **destino** del lerp, no a la posición | `Camera.cpp:69` | delta en píxeles de mundo |
| `setZoom`, mínimo 0.01 | `Camera.cpp:71` | escalar sin unidad |
| `transitionTo(pos, zoom, dur, easing)`: reinicia desde posición/zoom **actuales**; `dur<=0` = corte en la 1ª `update`; fija `m_targetPosition` al destino | `Camera.cpp:73-89` (`Camera.h:55-56`) | segundos, píxeles de mundo |
| Reloj de la transición **float** conservado (decisión U3-07.E08) | `Camera.cpp:34-41`; `include/Core/Math/EstadoTween.h:10-12` | segundos float |
| Curvas/mezcla delegadas en `Interpolacion::progreso/curva/tweenVec2/tweenEscalar` | `Camera.cpp:44-52` | double internamente |
| `setViewportSize` (mín. 1 px) | `Camera.cpp:91-94` | píxeles de pantalla |
| `getViewProjectionMatrix`: `halfW=vw/(2*zoom)`, `halfH=vh/(2*zoom)`, ortho con **Y invertida**, `translate(-position)` | `Camera.cpp:96-104` (convención Y en `Camera.h:65-68`) | mundo → NDC |
| `screenToWorld`: `position + (screen - vp/2) / zoom` | `Camera.cpp:106-112` | pantalla (origen arriba-izq.) → mundo |
| `worldToGrid`: delega en `IsoMath::screenToGrid`, tile 64x32 por defecto | `Camera.cpp:114-116`; `IsoMath.h:30-39` | mundo → `GridCoord` (entero, con `floor`) |
| **No hay** inversa mundo→pantalla en `Camera`; la repiten a mano `FogOfWar` y `DynamicLightMap` | `src/Render/FogOfWar.cpp:125-132`, `src/Render/DynamicLightMap.cpp:90-91` | `(world-position)*zoom + vp/2` |
| **No hay** concepto de mapa, límites, zona muerta ni sacudida en `Camera` | (ausencia; leído entero) | — |

Miembros de datos de `Camera`: **14** (`m_position :85`, `m_targetPosition :86`, `m_zoom :87`,
`m_lerpSpeed :88`, `m_viewportWidth :89`, `m_viewportHeight :90` y los 8 de la transición
`:95-102`). Métodos públicos: 14 (`Camera.h:23,27,31,33,34,55,57,59,60,61,62,69,74,81`).
Con la regla «≤15 miembros» contada sobre datos, `Camera` **tiene 1 hueco**; contada sobre
métodos ya está pasada. Ver riesgo R1.

### 1.2 Sistemas de coordenadas (qué unidad usa cada cosa)

1. **Mundo lógico = casilla**: `GridCoord{int x,y}` (`include/Core/Math/GridCoord.h:6-9`). No se
   interpola nunca (U3-07 R8).
2. **Píxel de mundo isométrico** (`Vector2`, `float`, `include/Core/Math/Vector2.h:5-12`; solo
   `+ - * float`): `IsoMath::gridToScreen` da `x=(gx-gy)*tw/2`, `y=(gx+gy)*th/2`
   (`IsoMath.h:23-28`), el origen `(0,0)` es la esquina superior del rombo de la casilla `(0,0)`.
   **La posición y el destino de `Camera` están en este espacio** (`Camera.cpp:73-88`). A la
   posición del jugador se le suma además el desfase de altura del terreno
   `m_map.alturas().desfasePx(playerCell)` (`ApplicationBucle.cpp:167`; `Alturas.h:93-94`).
3. **Vista / pantalla**: píxeles de ventana, origen arriba-izquierda (`Camera.cpp:107-111`). El
   **frustum visible** en píxeles de mundo es `vw/zoom × vh/zoom` centrado en `position`
   (`Camera.cpp:97-98`). *No es el tamaño del mapa*: el mapa no existe para `Camera`.
4. **Casilla de mapa y tamaño de tile**: `kTileW=64`, `kTileH=32` constantes del juego
   (`include/Engine/ApplicationInternal.h:28-29`) y, por separado, `m_map.getTileWidth()/Height()`
   del mapa cargado (`include/Runtime/RuntimeTileMap.h:51-52`). Son dos fuentes del mismo dato;
   que coincidan es **inferido** (ambos 64x32 en los niveles actuales).

### 1.3 De dónde sale el tamaño del mapa en `juego`

`RuntimeTileMap::getWidth()/getHeight()` = columnas × filas en **celdas**
(`RuntimeTileMap.h:46-47`), poblado en `loadLevel` con `m_map = std::move(preparedMap)`
(`src/Engine/ApplicationNivel.cpp:141`). Precedente de uso con la misma fuente: la niebla se
construye con `m_map.getWidth(), m_map.getHeight(), m_width, m_height, m_map.getTileWidth(),
m_map.getTileHeight()` (`ApplicationNivel.cpp:171-174`). **`Camera` y el seguimiento no consultan
el mapa en ningún sitio** (`ApplicationBucle.cpp:158-171`): hoy la cámara puede salirse del
mapa y mostrar vacío. En el editor existe el cálculo de límites en píxeles de mundo:
`Editor::fitMap` usa `left=-(rows-1)*tw/2`, `right=(cols-1)*tw/2+tw`, `bottom=(cols+rows)*th/2`
(`src/Editor/MapFraming.cpp:17-19`), con `top=0` implícito, y comenta que «el renderer coloca un
rectángulo tw x th en cada origen isométrico» (`:13`). Eso define la caja del mapa en espacio
mundo (que coincida con lo que pinta `IsometricRenderer` es **inferido**; E03 debe contrastarlo).
Los tiles altos y la altura del terreno sobresalen por arriba de `top=0` (**inferido**, ver D-T7).

### 1.4 Quién usa `Camera` y qué hace cada uno

| Consumidor | Dónde | Uso real |
|---|---|---|
| **Seguimiento del jugador** | `src/Engine/ApplicationBucle.cpp:164-171` | `transitionTo(target, zoom, 0.16f, Linear)` **solo al cambiar de casilla** (no `move`; razones en comentario `:150-163`); target = `gridToScreen(celda)` + `desfasePx`. Centra siempre: **no hay zona muerta**. `update(dt)` en `:175` |
| Reset al cargar nivel | `ApplicationNivel.cpp:470-471` | `m_lastCameraCell={-9999,-9999}` fuerza recentrado |
| Creación | `src/Engine/Application.cpp:450` | `Camera(width,height)`; `setViewportSize` **no se llama nunca en `juego`** (grep: solo `editor/src/EditorVistaDeMapa.cpp:25`) → viewport fijo al crear (inferido: si la ventana cambia de tamaño, se descuadra) |
| Cinemática del título | `ApplicationCinematica.cpp:309-310` | paseo por un camino con `transitionTo(destino, zoom, 0.001f/0.25f, EaseOutCubic)` (el `0.001f` es el «corte» de U3-07 R4) |
| Combate | `ApplicationCinematica.cpp:396,437,479` | encuadra el centro del combate con zoom `*1.7`, 0.5 s `EaseInOutQuad`; vuelve con 0.6 s (`:396`); pasa vista a `golpes->ponVista(position, zoom)` (`:441`) |
| **Sacudida existente** | `ApplicationCinematica.cpp:493-499` | `static std::minstd_rand temblor(7)` + `uniform_real_distribution(-4,4)`; offset `(d, d*0.6)` aplicado con **`transitionTo(..., 0.0f)` = machaca la posición lógica**; al acabar `transitionTo(centro, zoom, 0.08f)`. Disparo: 0.14 s (daño al aliado) y 0.24 s (crítico) en `src/Engine/ApplicationPantallas.cpp:429,431`; campo `float sacudida` en `include/Engine/ApplicationCinematica.h:51` |
| Picking | `Application.cpp:1013-1017`, `ApplicationPantallas.cpp:341` | `screenToWorld` + `IsoMath::screenToGrid` |
| HUD (depuración) | `src/Engine/ApplicationHud.cpp:378-381` | lee `position()`/`zoom()` |
| Culling | `include/Render/Culling.h:34-` (4 esquinas con `screenToWorld`), `src/Render/TileMap.cpp:161` | rango de celdas visible |
| Render | `src/Render/IsometricRenderer.cpp:134,319` | `VisibleGridRange` y `getViewProjectionMatrix` |
| Niebla / luces | `FogOfWar.cpp:125-132`, `DynamicLightMap.cpp:90-91` | leen `position()`/`zoom()` para alinear texturas con pantalla |
| `level_editor` | `editor/src/level_editor.cpp:583-591,1386` | centra con `transitionTo(...,0,Linear)`+`update(0)`; HOME con 0.25 s |
| `motor_editor` | `editor/src/EditorApplication.cpp:318,332,340-341`; `editor/include/EditorVistaDeMapa.h:42`; `editor/src/EditorVistaDeMapa.cpp:25-31` | **único uso de `move()`** (pan con ratón, lerp exponencial), zoom con rueda, `setViewportSize` y encuadre con `fitMap` |
| Demos | `examples/demo_camera.cpp:49,91-147` (+ `demo_tilemap`, `demo_lighting`, `demo_isometric_renderer`, `demo_animated_entity`, `demo_brillo_equipo`) | pruebas GL-free de lerp/transición |

Consecuencia (contradice la ficha, ver §6-C1): **el «seguimiento con lerp exponencial»
(`move`/`update`) no lo usa el juego**; el juego sigue con `transitionTo` de 0.16 s por casilla.
El lerp exponencial solo lo ejercita el editor y las demos.

### 1.5 Tests existentes

- `tests/Render/CameraTests.cpp` (272 líneas): constructor, zoom, viewport, lerp
  (`:94-131`), transiciones (`:136-187`), `screenToWorld` (`:189-231`), enum (`:255-259`).
  **Débiles**: `camera_move_updates_target_position` acaba en `MOTOR_CHECK(true)` (`:36`), igual
  `:43` y `camera_world_to_grid_delegates_to_iso_math` (`:242`).
- `tests/Engine/CameraTransitionTests.cpp` (301 líneas): transición lineal/zoom/flag, y la
  regresión numérica de U3-07.E08: muestras por curva (`:228`), 60 updates y fotograma de fin
  (`:242`), duración negativa (`:263`), origen no nulo (`:273`).
- Registro: `test_render_camera` (`cmake/C24RenderRpgTests.cmake:26-27`),
  `test_camera_transition` (`CMakeLists.txt:1374-1375`), `demo_camera` (`CMakeLists.txt:74,1091`).
  `motor_math = Camera.cpp` + GLM (`CMakeLists.txt:71-72`); `cmake/Interpolacion.cmake` añade
  fuentes con `target_sources(motor_math ...)` (línea 3) — patrón a repetir.
- **No existe ningún test** de seguimiento con jugador, límites de mapa, zona muerta ni de la
  sacudida de `ApplicationCinematica.cpp:493-499`.

## 2. Qué existe y qué falta para E02–E07

| E | Ya existe | Falta |
|---|---|---|
| E02 zona muerta | Seguimiento por `transitionTo` (`ApplicationBucle.cpp:164-171`); `IsoMath` | Función pura que decida **cuándo** moverse: hoy centra siempre. No existe «destino lógico» legible: `m_targetPosition` es privado (`Camera.h:86`) y el comentario `ApplicationBucle.cpp:150-157` explica que no se puede usar `move` por eso → el adaptador debe guardar su propio destino |
| E03 límites | Fórmula de cajas de mapa en `fitMap` (`MapFraming.cpp:17-19`); tamaño en `RuntimeTileMap` | Cálculo de `LimitesMapa` en píxeles de mundo + `limitarAlMapa(centro, zoom, vista)` (centrar si el mapa cabe). `fitMap` está en `Editor::` y en otra librería (`src/Editor/`), no reutilizable desde `motor_math` sin arrastrar el editor |
| E04 sacudida | Una sacudida con `minstd_rand` **estática** (`ApplicationCinematica.cpp:495`): la semilla 7 solo la fija una vez por proceso, el estado persiste entre combates → **no es determinista por combate**, ni testeable, y altera la posición lógica | Función pura sin estado: `f(estado, t, semilla)`; envolvente que acaba en 0 exacto; versión sin `rand()` global (el repo ya tiene `RPG::Xoroshiro128p` inyectable, `include/RPG/RandomEngine.h:46`, pero su API es enteros y con estado; una función hash sin estado encaja mejor, ver D-T4) |
| E05 composición | Nada | `PoseCamara` = base (seguimiento + límites) + sacudida, con orden y regla fijados; `Camera` necesita un **desplazamiento de vista** separado de `m_position` (ver R2) |
| E06 conexión | Tween compartido ya en `Camera` (`Camera.cpp:44-52`); hook de reinicio por mapa (`ApplicationNivel.cpp:470-471`); hooks de disparo de sacudida (`ApplicationPantallas.cpp:429,431`) | Adaptador entre política pura y `Camera` en `juego`; sustituir `ApplicationCinematica.cpp:493-499`; alinear niebla/luces con la posición de vista; pausa y cambio de mapa |
| E07 verificación | `MapFramingTests.cpp` como modelo de casos rectangulares (`tests/Editor/MapFramingTests.cpp:29,69,110,117`) | Casos de esquina/mapa pequeño en la política pura + evidencia de juego con captura (pendiente de Oriol) |

No se crea ninguna segunda `Camera`: toda la política vive en funciones puras nuevas y el único
cambio en `Camera` es el desplazamiento de vista (E06).

## 3. Firmas propuestas v1

Convenciones (heredadas de U3-07): ficheros nuevos `include/Core/Math/PoliticaCamara.h` +
`src/Core/Math/PoliticaCamara.cpp` (GL-free, en `motor_math`), tests en `tests/Core/`,
registro en `cmake/PoliticaCamara.cmake` con `target_sources(motor_math ...)` y
`MOTOR_GL_FREE_TARGETS` (modelo `cmake/Interpolacion.cmake`), una línea
`include(cmake/PoliticaCamara.cmake)` tras `CMakeLists.txt:75`. Tipos reales: `Vector2` global
(`float`), `Result<T>` global (`include/Core/Errors/Result.h:22-61`, `Ok/Error(std::string)`),
`GridCoord` global. Namespace nuevo `PoliticaCamara`. Sin `Vec2`, `CameraState` ni `FollowResult`
del texto de la ficha: no existen en el repo y no se crean tipos paralelos. Todos los cálculos
internos en `double`, resultado `float` una vez (D-T3). Todas las funciones son totales sobre
entradas finitas y devuelven `Error` ante NaN/inf, `zoom<=0` o dimensiones `<=0`.
Una cabecera ≤15 miembros / ≤500 líneas (prevista ~9 símbolos).

### E02 · Seguimiento con zona muerta (90 min)
```cpp
namespace PoliticaCamara {
struct TamanoVista { int ancho = 0; int alto = 0; };            // píxeles de pantalla
struct ZonaMuerta  { float semiAnchoPx = 0.0f; float semiAltoPx = 0.0f; };  // píxeles de PANTALLA
Result<Vector2> seguirConZonaMuerta(const Vector2& centroActual, float zoom,
                                    const Vector2& actorMundo, const ZonaMuerta& zona);
}
```
- `centroActual` = **destino vigente** de la cámara (lo guarda el adaptador, §1.4), `actorMundo` =
  píxel de mundo del actor (`gridToScreen`+`desfasePx`). Zona en píxeles de pantalla, equivale a
  `semi/zoom` en mundo.
- Por eje: si `|actor-centro| <= semi/zoom + 1e-4` → devuelve ese eje **idéntico** (sin
  movimiento ni jitter en la frontera); si no, empuja el centro lo mínimo para dejar al actor
  **sobre el borde** de la zona. `zona = {0,0}` ⇒ centra siempre = comportamiento actual.
- Error: zona con valor negativo/no finito, `zoom<=0`.
- Casos: dentro (sin cambio), cuatro costados, exactamente en el borde, actor a `+1e-3` del
  borde, zoom 0.5/2 (la zona en mundo cambia), idempotencia (aplicar dos veces = una).

### E03 · Límites al mapa real (90 min)
```cpp
struct LimitesMapa { float izquierda = 0, derecha = 0, arriba = 0, abajo = 0; };  // píxeles de mundo iso
Result<LimitesMapa> limitesDeMapa(int columnas, int filas, int anchoTile, int altoTile,
                                  float margenSuperiorPx = 0.0f);
Result<Vector2> limitarAlMapa(const Vector2& centro, float zoom, const TamanoVista& vista,
                              const LimitesMapa& limites);
```
- `limitesDeMapa`: misma geometría que `MapFraming.cpp:17-19` (+`arriba=-margenSuperiorPx`,
  que cubre sprites/alturas, D-T7). **No** depende de `RuntimeTileMap` (precedente:
  `Culling.h:26-29` mantiene esa dirección); el adaptador pasa `m_map.getWidth()...`.
- `limitarAlMapa`: por eje, `mitad = vista/(2*zoom)`; si `(max-min) <= 2*mitad` → **centrado**
  `(min+max)/2`; si no, `clamp(centro, min+mitad, max-mitad)`. Rectangular isométrico: se limita
  la caja que contiene el rombo (no el rombo) — D-O3 decide si se admite ver el vacío de las
  cuatro esquinas del rombo.
- Casos: mapa 1x1, 3x2 y 64x64 (como `MapFramingTests`), mapa mayor/menor que la vista en un
  solo eje, zoom 0.15/1/3, centro ya dentro (identidad), `columnas<=0` → Error.

### E04 · Sacudida determinista (90 min)
```cpp
struct EstadoSacudida { float amplitudPx = 0.0f; float duracion = 0.0f; };  // amplitud en píxeles de PANTALLA
Result<Vector2> desplazamientoSacudida(const EstadoSacudida& s, double transcurrido,
                                       std::uint32_t semilla, bool movimientoReducido = false);
```
- Sin estado y sin `rand()` global: hash entero (p. ej. mezcla tipo splitmix32 de
  `(semilla, floor(transcurrido*60))`) → dos valores en `[-1,1]`; envolvente
  `(1 - t/duracion)^2`; eje Y por `0.6` (conserva la anisotropía actual,
  `ApplicationCinematica.cpp:497`).
- `transcurrido >= duracion`, `duracion<=0`, `amplitud<=0` o `movimientoReducido` ⇒ `{0,0}`
  **exacto** (acaba a cero). Misma entrada ⇒ misma salida (test de repetición); semillas
  distintas ⇒ distinto (test).
- `movimientoReducido` se **inyecta**; la política nunca lee `MOTOR_MOVIMIENTO_REDUCIDO` (la
  lectura de entorno sigue en el adaptador como en `ApplicationBucle.cpp:298`,
  `ApplicationHud.cpp:76`).
- La sacudida **nunca** toca la posición lógica: devuelve un offset de vista.

### E05 · Composición base + efectos (60 min)
```cpp
struct PoseCamara { Vector2 base; Vector2 vista; float zoom = 1.0f; };   // base = lógica; vista = base+sacudida
struct PoliticaPose { bool sacudidaDentroDeLimites = true; };            // regla D-T6
Result<PoseCamara> componerPose(const Vector2& baseLimitada, float zoom, const Vector2& sacudidaMundo,
                                const TamanoVista& vista, const LimitesMapa* limites,
                                const PoliticaPose& politica);
```
- Orden fijo: seguimiento (E02) → límites (E03) → **base**; luego `vista = base + sacudida`; con
  `sacudidaDentroDeLimites` se limita `vista` otra vez con E03 (en un borde la sacudida solo se
  mueve hacia dentro; no se ve vacío por sacudida). `base` **nunca** recibe la sacudida
  (cero deriva acumulada; test: 10 000 llamadas, `base` bit-idéntica).
- `limites == nullptr` ⇒ sin límites (title/cinemáticas, D-T8). Puntero opcional en lugar de
  `std::optional` (elección de estilo; E05 puede cambiarlo).

### E06 · Conectar con `Camera` y `juego` (90 min, DOCUMENTO + adaptador)
Adaptador (patrón `struct ApplicationCinematica`, `ApplicationCinematica.h:79-137`, funciones
estáticas que reciben `Application&`; el `friend` aparece en `ApplicationCinematica.h:143`):
```cpp
struct EstadoCamaraJuego { Vector2 destino; bool conDestino = false;
    float amplitud = 0, duracion = 0; double reloj = 0; std::uint32_t semilla = 0; };
struct ApplicationCamara {                         // include/Engine/ApplicationCamara.h
    static void sigueJugador(Application& app, float dt);   // sustituye ApplicationBucle.cpp:158-175
    static void sacude(Application& app, float amplitudPx, float duracion);  // sustituye ApplicationCinematica.cpp:493-499
    static void reinicia(Application& app);                 // cambio de mapa / pausa
};
```
Cambio mínimo en `Camera` (única edición del consumidor): `void setDesplazamientoVista(const
Vector2& mundo)` y `Vector2 posicionVista() const` (= `m_position + m_sacudida`); matriz
(`Camera.cpp:102`) y `screenToWorld` (`:111`) usan `posicionVista()`. `position()` sigue siendo la
lógica. `FogOfWar.cpp:129-132` y `DynamicLightMap.cpp:90-91` pasan a `posicionVista()` (si no, la
niebla/luces se desalinean durante la sacudida). El seguimiento sigue moviéndose con
`transitionTo(destino, zoom, 0.16f, Linear)` (`ApplicationBucle.cpp:170`): no se introduce otro
mecanismo de movimiento; el tween compartido **es** el movimiento (cierra la puerta TEMPS_TWEEN).
- Reinicio de efectos: cambio de mapa ya tiene hook (`ApplicationNivel.cpp:470-471`: el adaptador
  detecta el centinela `-9999` de `m_lastCameraCell`, sin editar `ApplicationNivel.cpp`);
  pausa: el adaptador deja de avanzar `reloj` y el offset vuelve a 0 (cómo se detecta la pausa
  en `juego` **no está verificado** en esta E: tarea de E06).

### E07 · Verificar esquina y zona muerta (60 min, DOCUMENTO)
Casos mínimos (todos con captura a escala real, `kTileW/H=64/32`, zoom 1): actor en el centro
(sin movimiento), en cada borde de la zona (sin vibración, 120 `update`), en las 4 esquinas del
mapa (la vista nunca excede `LimitesMapa` salvo eje no cabe), mapa pequeño (1x1, 3x2 como
`MapFramingTests.cpp:69,110`) centrado, golpe crítico (0.24 s) en esquina con y sin
`MOTOR_MOVIMIENTO_REDUCIDO`, cambio de mapa durante una sacudida. La aprobación visual es de
Oriol: se declara **pendiente** hasta observación real (porta de la ficha).

## 4. Ámbito de ficheros congelado propuesto

| E | Crear | Ampliar | Notas / límites |
|---|---|---|---|
| E02 | `include/Core/Math/PoliticaCamara.h`, `src/Core/Math/PoliticaCamara.cpp`, `tests/Core/PoliticaCamaraTests.cpp`, `cmake/PoliticaCamara.cmake` | `CMakeLists.txt` (+1 línea tras `:75`, solo en E02) | Target `test_politica_camara`, labels `unit;gl-free`. Sin GL |
| E03 | — | `PoliticaCamara.h/.cpp`, `PoliticaCamaraTests.cpp` | Si el `.cpp` pasa de 400 líneas, partir `PoliticaCamaraLimites.cpp` (no en v1) |
| E04 | — | ídem | Test de repetición bit a bit y de «acaba en 0» |
| E05 | — | ídem | Cabecera final prevista: ~9 símbolos (<15), <200 líneas |
| E06 | `include/Engine/ApplicationCamara.h`, `src/Engine/ApplicationCamara.cpp`, `tests/Engine/ApplicationCamaraTests.cpp` (si es GL-free; si no, test de política) | `include/Render/Camera.h` (+1 miembro, +2 métodos), `src/Render/Camera.cpp`, `src/Engine/ApplicationBucle.cpp` (bloque `:158-175` **sustituido**, neto negativo), `src/Engine/ApplicationCinematica.cpp` (quitar `:493-499`, **debe quedar <500**), `include/Engine/ApplicationCinematica.h` (`sacudida` `:51` pasa a disparar `ApplicationCamara::sacude`), `src/Render/FogOfWar.cpp`, `src/Render/DynamicLightMap.cpp`, `CMakeLists.txt` (una línea junto a `:835` para la fuente nueva) | **Cruce con otras sesiones**: ver R3. `ApplicationNivel.cpp` NO se edita |
| E07 | `out/fabrica/REPLICA-R1/` solo evidencia | tests de política (esquinas) | Sin código de producto |

Límites: `Camera.h` 103 → ~108 líneas; `Camera.cpp` 116 → ~125. `Camera` pasaría a **15**
miembros de datos (`m_sacudida`): exactamente el límite; cualquier miembro más obliga a extraer
la transición a un `struct EstadoTransicion` (propuesta en R1) — no se hace en v1.

## 5. Decisiones

### 5.1 Del Brain (técnicas; pueden tomarse ya)

- **D-T1** Política en `Core/Math/PoliticaCamara.*` (namespace `PoliticaCamara`, `motor_math`),
  no en `Render/` ni en la `Camera` (patrón U3-07). *Bloquea: E02.*
- **D-T2** Unidades: zona muerta y amplitud de sacudida en **píxeles de pantalla** (invariantes
  al zoom, divididos por `zoom` al pasar a mundo); límites en **píxeles de mundo iso**; `GridCoord`
  nunca entra en la política (se convierte antes con `IsoMath`). *E02, E03, E04.*
- **D-T3** `Vector2`/`float` en la interfaz, `double` internamente, conversión a `float` una vez
  (como D4 de U3-07). *Todas.*
- **D-T4** Sacudida = función hash sin estado de `(semilla, tiempo)`, no `RPG::Xoroshiro128p` ni
  `std::minstd_rand`: no necesita estado en `Camera`, es evaluable en cualquier `t` (testeable,
  pausable) y no hay `rand()` global. La semilla de producción la pone el adaptador
  (p. ej. derivada de `e.dados->seed()` del combate, `ApplicationCinematica.cpp:415`; la semilla
  de dados es aleatoria por combate, `:362-366`). *E04, E06.*
- **D-T5** La sacudida es un **desplazamiento de vista** separado de `m_position`
  (`setDesplazamientoVista`), no un `transitionTo`. Razón: cumple «no altera posición lógica» de
  la ficha y evita que la sacudida corrompa `m_transitionStart*` (hoy lo hace,
  `ApplicationCinematica.cpp:497`). *E05, E06.*
- **D-T6** Regla sacudida vs límites: **recortar la vista final** (`sacudidaDentroDeLimites=true`):
  nunca se ve vacío por sacudida; en un borde el temblor es unilateral. Alternativa B: sacudida
  libre (puede mostrar ≤ amplitud px de vacío). Depende de D-O3 si Oriol admite ver fuera.
  *E05.*
- **D-T7** Margen superior de `limitesDeMapa`: parámetro `margenSuperiorPx` (defecto 0) para
  sprites altos y alturas de terreno (`Alturas::desfasePx` es negativo hacia arriba,
  `Alturas.h:93`); el valor concreto lo fija E03 con el nivel de mayor altura. *E03.*
- **D-T8** Dónde se aplican los límites: exploración y combate **sí**; cinemática del título
  (`ApplicationCinematica.cpp:309`, camino libre) **no** (`limites=nullptr`). Revisar en E07 si el
  combate cerca del borde queda descentrado. *E06.*
- **D-T9** `Camera` conserva su reloj `float` (U3-07 R2, `Camera.cpp:34-41`); el reloj de la
  sacudida es `double` en el adaptador. No se migra `Camera` a `avanzaTween`. *E06.*
- **D-T10** `fitMap` y `limitesDeMapa` duplican la misma geometría (`MapFraming.cpp:17-19`);
  v1 no refactoriza el editor. Registrar para un paquete posterior (que `fitMap` delegue en la
  política). *E03.*

### 5.2 Para Oriol (producto/visual)

| ID | Pregunta | Opciones | Recomendación | Bloquea |
|---|---|---|---|---|
| **D-O1** | Tamaño de la zona muerta por defecto | (a) 0 = centrar siempre (hoy); (b) ±1 tile `64x32` px; (c) ±1,5 tiles `96x48` px; (d) 25 % de la ventana | **(c) ±96x±48 px**, constante en un solo sitio y ajustable en E07 tras verlo | Valor en **E06**; E02 se implementa y se prueba sin él (recibe `ZonaMuerta`). Defecto de código = (a) hasta decidir |
| **D-O2** | Intensidad y duración de la sacudida | (a) como hoy: ±4/±2,4 px, 0,14 s (daño) y 0,24 s (crítico) pero con caída a 0; (b) más suave (±3); (c) más fuerte (±6, crítico 0,35 s) | **(a)** conservar valores, añadir caída. Nota: hoy son 4 px de **mundo**, con zoom de combate 1,7 se ven ≈6,8 px de pantalla; la amplitud de v1 es de pantalla, así que (a) equivale a pasar `4*1,7≈7` px | Valores en **E06**; E04 no | 
| **D-O3** | ¿La cámara puede mostrar fuera del mapa? | (a) nunca (salvo mapa menor que la vista: centrado); (b) margen de 1 tile; (c) libre | **(a)**; mapas menores que la ventana se centran con el vacío alrededor | Parámetro por defecto en **E06** y criterio de aceptación de **E07**; E03 lo implementa parametrizado (0 = estricto) |
| **D-O4** | Reduced motion: ¿sacudida apagada o atenuada? | (a) off; (b) 25 % de amplitud | **(a) off**, coherente con cómo el HUD omite animaciones (`DialogoHoja.cpp:219`) | Defecto en **E06**; E04 ya admite ambos |
| **D-O5** | Aprobación visual/jugable (dead zone, esquinas, sacudida a escala real) | — | Observación real por Oriol | **E07**, y el cierre del paquete (porta de la ficha) |

## 6. Discrepancias con la ficha (registradas, no resueltas)

- **C1.** La ficha habla de «seguimiento (lerp exponencial)» como estado actual de la cámara. En
  `juego` **no se usa**: sigue con `transitionTo` 0,16 s por cambio de casilla
  (`ApplicationBucle.cpp:170`); `move`+lerp lo usa solo el editor (`EditorApplication.cpp:340`).
  E02/E06 deben partir de `transitionTo`, no del lerp.
- **C2.** La ficha pide una sacudida «determinista» y que «no altere la posición lógica»; ya
  existe una sacudida integrada que no cumple ninguna de las dos
  (`ApplicationCinematica.cpp:493-499`: estado `static`, `transitionTo`). E06 la sustituye, no la
  añade.
- **C3.** Las firmas de la ficha usan `Vec2`, `CameraState`, `DeadZone`, `MapBounds`, `ViewSize`,
  `ShakeState`, `CameraPose`, `uint32_t`: en el repo solo existen `Vector2` (global) y
  `Result<T>`. Mapeo: `Vec2→Vector2`, `MapBounds→LimitesMapa`, `ViewSize→TamanoVista`,
  `DeadZone→ZonaMuerta`, `ShakeState→EstadoSacudida`, `CameraPose→PoseCamara`,
  `CameraState→(centro, zoom)` (sin struct nueva). `CameraPolicy→PoliticaPose`.
- **C4.** La ficha no menciona que el viewport de `juego` nunca se actualiza tras crear la
  cámara (`Application.cpp:450`; `setViewportSize` solo en el editor): los límites (E03)
  dependen de `anchoVista/altoVista`; si la ventana es redimensionable el cálculo se desalinea
  (inferido; hay que comprobar si la ventana de `juego` es redimensionable antes de E06).
- **C5.** U3-07 documentó `Camera.cpp` con 109 líneas; ahora son 116 (post-E08, `628f417`).
- **C6.** Dependencia de E `PREP.E10` y de paquete U3-07: U3-07 está integrado en la base;
  `PREP.E10` no verificada aquí.

## 7. Riesgos y trampas

- **R1. Límite de miembros de `Camera`**: 14 de datos hoy (`Camera.h:85-102`); +`m_sacudida` = 15.
  Contando métodos (14 + 2) ya se pasa. Si se quiere holgura, extraer los 8 de la transición a
  `struct EstadoTransicion` dentro de `Camera.h` (neutral en comportamiento, cubierto por
  `CameraTransitionTests.cpp:228-273`); queda como decisión del Brain en E06, no en v1.
- **R2. `position()` vs posición de vista**: `FogOfWar`/`DynamicLightMap`/`golpes->ponVista` leen
  `position()` (`FogOfWar.cpp:129-132`, `DynamicLightMap.cpp:90-91`, `ApplicationCinematica.cpp:441`).
  Si la sacudida solo cambia la matriz, niebla y luces se despegan del mapa durante el temblor.
  E06 debe migrarlos y probar alineación; `golpes->ponVista` decide con Oriol si quiere la vista
  sacudida o la lógica.
- **R3. Ficheros de cruce con otras sesiones (zona caliente)**: `ApplicationBucle.cpp` (473 líneas,
  editado por muchas AV*), `ApplicationCinematica.cpp` (**516 líneas, ya >500**; E06 solo puede
  restar), `ApplicationNivel.cpp` (**506 líneas, ya >500**: no tocar), `Application.cpp`
  (1282), `CMakeLists.txt` (bloque `:835`). Regla: ediciones netas negativas en los >500 y
  `git diff` mínimo; la lógica nueva va a `ApplicationCamara.*`.
- **R4. Reloj `float` de `Camera`** (`Camera.cpp:34-41`): con 60 × (1/60) llega a 0,9999997 y
  termina un fotograma más tarde que `avanzaTween` en double (U3-07 R2). El seguimiento 0,16 s
  por casilla depende de ese fotograma de fin: no migrar el reloj en este paquete. No reiniciar
  `transitionTo` cada frame (reiniciaría la animación; comentario `ApplicationBucle.cpp:159-163`).
- **R5. Pisadas entre seguimiento y cinemática de combate**: ambos llaman `transitionTo` sobre la
  misma cámara (`ApplicationBucle.cpp:170` está excluido en `Battle` por `:164`;
  `ApplicationCinematica.cpp:437,479`). La política de zona muerta no puede suponer que el
  último destino lo puso ella: `reinicia()` al entrar/salir de combate y de título.
- **R6. Dead zone con movimiento discreto**: el actor salta de casilla (`GridCoord`); el
  seguimiento debe usar la casilla **lógica destino**, no la posición visual interpolada del
  `Player` (que va con paso de 0,16 s, `Player.cpp:61-66`), o aparecerá jitter por doble tween
  (inferido). `desfasePx` de altura cambia el `y` del actor entre casillas: entra en la zona.
- **R7. Persistencia del `m_lastCameraCell`**: el centinela `-9999` (`ApplicationNivel.cpp:471`)
  es el único aviso de «nivel nuevo»; si E06 no lo respeta, la cámara conserva destino del mapa
  anterior y limita con los límites del nuevo.
- **R8. Zoom variable y límites**: el combate hace zoom ×1,7 (`ApplicationCinematica.cpp:436`);
  `limitarAlMapa` depende de `zoom`: un límite calculado con zoom 1 queda mal al cambiar el zoom
  durante la transición. La composición (E05) debe recalcular con el zoom **final** de la pose.
- **R9. Culling**: `VisibleGridRange` toma el viewport sin sacudida (`Culling.h:34-`); con vista
  sacudida ≤ pocos px, el margen de 1 celda lo cubre (inferido), pero hay que comprobarlo en E06.
- **R10. Tests débiles existentes** (`CameraTests.cpp:36,43,242` con `MOTOR_CHECK(true)`): no son
  prueba de regresión del seguimiento; los tests de E02–E05 deben ser numéricos y con caso
  negativo.
- **R11. `fitMap` y límites duplicados** (D-T10): si cambia la fórmula del renderer, hay dos
  sitios a corregir.
- **R12. Orden E02→E07 secuencial** sobre `PoliticaCamara.h/.cpp` (no paralelizable), como U3-07.
- **R13. Efecto de `MOTOR_MOVIMIENTO_REDUCIDO`**: se lee con `static const bool` por sitio
  (`ApplicationBucle.cpp:298`): no cambia en caliente; el test de E07 debe lanzar el proceso con y
  sin la variable.

## 8. Qué puede empezar ya

- **Sin decisión de Oriol**: E02, E03, E04 y E05 (se implementan parametrizadas; los valores por
  defecto concretos no entran en la política). Pueden arrancar en cuanto el Brain dé por
  buenas D-T1…D-T10.
- **Parcial**: E06 (integración con valores) necesita D-O1, D-O2, D-O3, D-O4; puede prepararse con
  defecto = comportamiento actual (zona 0, amplitud 4 px/0,14-0,24 s, límites estrictos, reducido
  off).
- **Bloqueada por Oriol**: E07 (D-O5, observación real).

## 9. Cierre de E01

- Comandos usados: lectura con `sed -n`/`cat -n` de cada tramo citado; `grep -rn` de
  `Camera`, `m_camera`, `setViewportSize`, `sacudida`, `MOTOR_MOVIMIENTO_REDUCIDO`, `fitMap`;
  `wc -l` (Camera.h 103, Camera.cpp 116, CameraTests 272, CameraTransitionTests 301,
  ApplicationNivel 506, ApplicationCinematica 516, ApplicationBucle 473). Sin build ni tests
  (E documental).
- Citas comprobadas: `out/fabrica/REPLICA-R1/U4-01.E01_citas.log`.
- Hash de contexto: base `628f417`.
