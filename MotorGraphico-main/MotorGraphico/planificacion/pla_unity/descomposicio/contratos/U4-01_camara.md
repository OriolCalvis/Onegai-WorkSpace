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
La regla «≤500 líneas / ≤15 miembros» viene del encargo del coordinador y del §3 de
`contratos/U3-07_tweens.md` («Ningún fichero >500 líneas ni cabecera >15 miembros previsto»); no
hay herramienta que la compruebe (**inferido** que «miembros» = datos). Con la regla contada sobre datos, `Camera` **tiene 1 hueco**; contada sobre
métodos ya está pasada. Ver riesgo R1.

### 1.2 Sistemas de coordenadas (qué unidad usa cada cosa)

1. **Mundo lógico = casilla**: `GridCoord{int x,y}` (`include/Core/Math/GridCoord.h:6-9`). No se
   interpola nunca (U3-07 R8).
2. **Píxel de mundo isométrico** (`Vector2`, `float`, `include/Core/Math/Vector2.h:5-12`; solo
   `+ - * float`): `IsoMath::gridToScreen` da `x=(gx-gy)*tw/2`, `y=(gx+gy)*th/2`
   (`IsoMath.h:23-28`); que el origen `(0,0)` sea la esquina superior del rombo de la casilla `(0,0)`
   es **inferido** de `MapFraming.cpp:13,17-19` (`top=0`), a contrastar con `IsometricRenderer` en E03.
   **La posición y el destino de `Camera` están en este espacio** (`Camera.cpp:73-88`). A la
   posición del jugador se le suma además el desfase de altura del terreno
   `m_map.alturas().desfasePx(playerCell)` (`ApplicationBucle.cpp:167`; `include/Runtime/Alturas.h:93-94`).
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
el mapa en ningún sitio** (`Camera.cpp` y `ApplicationBucle.cpp:152-172` no leen `getWidth`). Que
por eso la cámara pueda salirse del mapa y mostrar vacío es **inferido** de ese código (no observado). En el editor existe el cálculo de límites en píxeles de mundo:
`Editor::fitMap` usa `left=-(rows-1)*tw/2`, `right=(cols-1)*tw/2+tw`, `bottom=(cols+rows)*th/2`
(`src/Editor/MapFraming.cpp:17-19`), con `top=0` implícito, y comenta que «el renderer coloca un
rectángulo tw x th en cada origen isométrico» (`:13`). Eso define la caja del mapa en espacio
mundo (que coincida con lo que pinta `IsometricRenderer` es **inferido**; E03 debe contrastarlo).
Los tiles altos y la altura del terreno sobresalen por arriba de `top=0` (**inferido**, ver D-T7).

### 1.4 Quién usa `Camera` y qué hace cada uno

| Consumidor | Dónde | Uso real |
|---|---|---|
| **Seguimiento del jugador** | `src/Engine/ApplicationBucle.cpp:164-171` | `transitionTo(target, zoom, 0.16f, Linear)` **solo al cambiar de casilla** (no `move`; razones en comentario `:152-163`); target = `gridToScreen(celda)` + `desfasePx`. Centra siempre: **no hay zona muerta**. `update(dt)` en `:175` |
| Reset al cargar nivel | `ApplicationNivel.cpp:470-471` | `m_lastCameraCell={-9999,-9999}` fuerza recentrado |
| Creación | `src/Engine/Application.cpp:450` | `Camera(width,height)`; `setViewportSize` **no se llama nunca en `juego`** (grep: solo `editor/src/EditorVistaDeMapa.cpp:25`) → viewport fijo al crear (inferido: si la ventana cambia de tamaño, se descuadra) |
| Cinemática del título | `ApplicationCinematica.cpp:309-310` | paseo por un camino con `transitionTo(destino, zoom, 0.001f/0.25f, EaseOutCubic)` (el `0.001f` es el «corte» de U3-07 R4) |
| Combate | `ApplicationCinematica.cpp:396,437,479` | encuadra el centro del combate con zoom `*1.7`, 0.5 s `EaseInOutQuad`; vuelve con 0.6 s (`:396`); pasa vista a `golpes->ponVista(position, zoom)` (`:441`) |
| **Sacudida existente** | `ApplicationCinematica.cpp:492-500` | `static std::minstd_rand temblor(7)` + `uniform_real_distribution(-4,4)`; offset `(d, d*0.6)` aplicado con **`transitionTo(..., 0.0f)` = machaca la posición lógica**; al acabar `transitionTo(centro, zoom, 0.08f)`. Disparo: 0.14 s (daño al aliado) y 0.24 s (crítico) en `src/Engine/ApplicationPantallas.cpp:429,431`; campo `float sacudida` en `include/Engine/ApplicationCinematica.h:51` |
| Picking | `Application.cpp:1013-1017`, `ApplicationPantallas.cpp:341` | `screenToWorld` + `IsoMath::screenToGrid` |
| **HUD del mundo (carteles, bocadillos, guía de mapa, avisos)** | `src/Engine/ApplicationHud.cpp:378-381` rellena `camaraX/camaraY/zoom` de `EstadoJuego` (`include/HUD/Pergamino/InterfazPergamino.h:75-78`); lo leen `src/HUD/Pergamino/CartelesPuertas.cpp:103-104,116-117`, `BocadillosPnj.cpp:41-42`, `GuiaMapa.cpp:191-192`, `InterfazPergamino.cpp:288-289,304`; test `tests/HUD/CartelesPuertasTests.cpp` | proyectan puntos del mundo a pantalla con `(mundo - cámara)*zoom + vista/2`. **Si la sacudida solo mueve la matriz, estos elementos no tiemblan con el mundo** (R2, D-T11) |
| Números de golpe | `include/HUD/Pergamino/GolpesEnPantalla.h:32` (`ponVista`), `src/HUD/Pergamino/GolpesEnPantalla.cpp:46-47`; llamada en `ApplicationCinematica.cpp:441` | misma proyección |
| Culling | `include/Render/Culling.h:34-` (4 esquinas con `screenToWorld`), `src/Render/TileMap.cpp:161` | rango de celdas visible |
| Render | `src/Render/IsometricRenderer.cpp:134,319` | `VisibleGridRange` y `getViewProjectionMatrix` |
| Niebla / luces | `FogOfWar.cpp:125-132`, `DynamicLightMap.cpp:90-91` | leen `position()`/`zoom()` para alinear texturas con pantalla |
| `level_editor` | `editor/src/level_editor.cpp:583-591,1199,1345-1352,1371-1377,1386,1642,1652` | centra con `transitionTo(...,0,Linear)`+`update(0)` (`:587-591`); `screenToWorld` (`:1199`); `setZoom` (`:1345-1352`); pan WASD con `camera.move` (`:1371-1377`, lerp exponencial); HOME 0.25 s (`:1383-1386`); `update(1/60)` (`:1642`); matriz al shader (`:1652`) |
| Cierre | `src/Engine/ApplicationCierre.cpp:145` | `m_camera.reset()` |
| Tests que construyen `Camera` | `tests/Render/CameraTests.cpp`, `tests/Engine/CameraTransitionTests.cpp`, `tests/Render/CullingTests.cpp`, `OcclusionTests.cpp`, `PostProcessingTests.cpp` (grep `\bCamera\b`; `tests/Core/InterpolacionTests.cpp` y `EstadoTweenTests.cpp` solo la nombran en comentarios) | regresión de Camera y culling |
| `motor_editor` | `editor/src/EditorApplication.cpp:318,332,340-341`; `editor/include/EditorVistaDeMapa.h:42`; `editor/src/EditorVistaDeMapa.cpp:25-31` | pan con ratón vía `move()` (lerp exponencial), zoom con rueda, `setViewportSize` y encuadre con `fitMap` |
| Demos | `examples/demo_camera.cpp:49,91-147`; también `demo_animated_entity`, `demo_brillo_equipo`, `demo_dynamic_lights`, `demo_fog`, `demo_hud`, `demo_isometric_renderer`, `demo_lighting`, `demo_lut`, `demo_textured_quad`, `demo_tilemap` (grep) | lerp/transición y montaje de escenas |

Consecuencia (contradice la ficha, ver §6-C1): **el «seguimiento con lerp exponencial»
(`move`/`update`) no lo usa `juego`** (grep: ningún `m_camera->move` en `src/`); sigue con
`transitionTo` de 0.16 s por casilla. El lerp exponencial lo ejercitan `motor_editor`,
`level_editor` y las demos. Que la cámara salga del mapa en `juego` es **inferido** (§1.3).

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
  sacudida de `ApplicationCinematica.cpp:492-500`.

## 2. Qué existe y qué falta para E02–E07

| E | Ya existe | Falta |
|---|---|---|
| E02 zona muerta | Seguimiento por `transitionTo` (`ApplicationBucle.cpp:164-171`); `IsoMath` | Función pura que decida **cuándo** moverse: hoy centra siempre. No existe «destino lógico» legible: `m_targetPosition` es privado (`Camera.h:86`) y el comentario `ApplicationBucle.cpp:152-163` explica que no se puede usar `move` por eso → el adaptador debe guardar su propio destino |
| E03 límites | Fórmula de cajas de mapa en `fitMap` (`MapFraming.cpp:17-19`); tamaño en `RuntimeTileMap` | Cálculo de `LimitesMapa` en píxeles de mundo + `limitarAlMapa(centro, zoom, vista)` (centrar si el mapa cabe). `fitMap` está en `Editor::` y en otra librería (`src/Editor/`), no reutilizable desde `motor_math` sin arrastrar el editor |
| E04 sacudida | Una sacudida con `minstd_rand` **estática** (`ApplicationCinematica.cpp:495`): la semilla 7 solo la fija una vez por proceso, el estado persiste entre combates → **no es determinista por combate**, ni testeable, y altera la posición lógica | Función pura sin estado: `f(estado, t, semilla)`; envolvente que acaba en 0 exacto; versión sin `rand()` global (el repo ya tiene `RPG::Xoroshiro128p` inyectable, `include/RPG/RandomEngine.h:46`, pero su API es enteros y con estado; una función hash sin estado encaja mejor, ver D-T4) |
| E05 composición | Nada | `PoseCamara` = base (seguimiento + límites) + sacudida, con orden y regla fijados; `Camera` necesita un **desplazamiento de vista** separado de `m_position` (ver R2) |
| E06 conexión | Tween compartido ya en `Camera` (`Camera.cpp:44-52`); hook de reinicio por mapa (`ApplicationNivel.cpp:470-471`); hooks de disparo de sacudida (`ApplicationPantallas.cpp:429,431`) | Adaptador entre política pura y `Camera` en `juego`; sustituir `ApplicationCinematica.cpp:492-500`; alinear niebla/luces con la posición de vista; pausa y cambio de mapa |
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
- `centroActual` = **destino vigente** de la cámara (lo guarda el adaptador, §2 fila E02), `actorMundo` =
  píxel de mundo del actor (`gridToScreen`+`desfasePx`). Zona en píxeles de pantalla, equivale a
  `semi/zoom` en mundo.
- Por eje: si `|actor-centro| <= semi/zoom + 1e-4` → devuelve ese eje **idéntico** (sin
  movimiento ni jitter en la frontera); si no, empuja el centro lo mínimo para dejar al actor
  **sobre el borde** de la zona. `zona = {0,0}` ⇒ centra siempre = comportamiento actual.
- Error: zona con valor negativo/no finito, `zoom<=0`.
- Casos: dentro (sin cambio), cuatro costados, exactamente en el borde, actor a `+1e-3` del
  borde, zoom 0.5/2 (la zona en mundo cambia), idempotencia (aplicar dos veces = una).

### E03 · Límites al mapa real (90 min) — r1: centro siempre sobre terreno (decisión de Oriol 30/10)
```cpp
struct RomboMapa { Vector2 superior, derecho, inferior, izquierdo; };            // contorno real, px de mundo
struct LimitesMapa { float izquierda = 0, derecha = 0, arriba = 0, abajo = 0;   // caja, px de mundo iso
                     bool usaRombo = false; RomboMapa rombo; };
Result<RomboMapa> romboDeMapa(int columnas, int filas, int anchoTile, int altoTile);
Result<LimitesMapa> limitesDeMapa(int columnas, int filas, int anchoTile, int altoTile,
                                  float margenSuperiorPx = 0.0f);
Result<Vector2> limitarAlMapa(const Vector2& centro, float zoom, const TamanoVista& vista,
                              const LimitesMapa& limites);
```
- `limitesDeMapa`: caja con la misma geometría que `MapFraming.cpp:17-19` (+`arriba=-margenSuperiorPx`,
  que cubre sprites/alturas, D-T7) y `rombo = romboDeMapa(...)`, `usaRombo=true`. **No** depende de
  `RuntimeTileMap` (precedente: `Culling.h:26-29`); el adaptador pasa `m_map.getWidth()...`.
- Rombo: el rectángulo `tw x th` de la casilla (gx,gy) arranca en `IsoMath::gridToScreen`; su rombo
  tiene la esquina superior en `+tw/2`. Vértices: superior `(tw/2, 0)`; derecho = esquina derecha de
  `(columnas-1,0)`; inferior = esquina inferior de `(columnas-1,filas-1)`; izquierdo = esquina izquierda
  de `(0,filas-1)` (paralelogramo; rombo si columnas==filas). Cuenta como «sobre terreno» cualquier punto
  del rombo de alguna casilla (se usa el rombo entero de la casilla).
- `limitarAlMapa` (**r4**, sustituye r1-r3 de ancla/segmento): rango de la caja para el centro: por eje,
  `mitad = vista/(2*zoom)`; si `(max-min) <= 2*mitad` el rango es el punto **centrado** `(min+max)/2`; si
  no, `[min+mitad, max-mitad]`. Con `usaRombo` el resultado es la **proyección euclídea del centro sobre
  el polígono convexo rango∩rombo** (rombo recortado contra el rectángulo con Sutherland-Hodgman): el
  propio centro si ya está dentro (tolerancia 1e-3 px) y, si no, el punto más cercano de sus aristas.
  Es 1-Lipschitz y vale igual con rango 2D, casi colapsado o colapsado: sin umbrales ni ancla, sin
  saltos al barrer la sacudida (E05) ni el zoom (tween de combate). Si rango∩rombo es vacío gana el
  rombo: el resultado es el punto del rombo más cercano al **rectángulo del rango** (no al centro dado:
  así es continuo en la frontera en que la intersección se vacía al cambiar el zoom; la caja no se
  respeta). Resultado siempre sobre el rombo, en el rango cuando rango∩rombo no es vacío, e
  idempotente; en bordes y esquinas puede verse algo de vacío, nunca una pantalla sin terreno en el
  centro. (Defectos de r1-r3 corregidos: saltos de 32 px en 30x1 con rango casi colapsado, de 36-787 px
  al vaciarse la intersección y derivada ≈10 en la rama del ancla; hallazgos de la revisión de E05.)
- Casos (calculados a mano y con una referencia independiente en Python; comprobados por
  `PoliticaCamaraLimitesTests.cpp`): ver §E05 (64x64, 3x2); mapa 30x1 (vista 800x600, zoom 1, centro
  (-5000,900)) ⇒ `(464,248)` (rango = segmento y=248, x∈[400,592]; ∩ rombo x∈[464,528]); 3x2 zoom 20
  centro (-100,-100) ⇒ `(-12; 22)`; centro (500,500) ⇒ `(108; 58)` (antes `(-9,2; 20,6)` y
  `(105,2; 59,4)`, que eran «caja y luego rombo», no el cercano de la intersección); `columnas<=0` →
  Error. **margen 96:** 3x2 zoom 1 centro (0,0) ⇒ `(41,6; 4,8)` (vacío: cercano del rombo al rango
  `(48,-8)`; antes `(6,4; 12,8)`); 1x1 zoom 0,15 centro (1,575; -33,55) ⇒ `(32; 0)` (antes
  `(21,08; 5,46)`); 30x1 zoom 1,5 centro (643,296) ⇒ `(624; 296)` (antes `(619,626; 296)`).

### E04 · Sacudida determinista (90 min)
```cpp
struct EstadoSacudida { float amplitudPx = 0.0f; float duracion = 0.0f; };  // amplitud en píxeles de PANTALLA
Result<Vector2> desplazamientoSacudida(const EstadoSacudida& s, double transcurrido,
                                       std::uint32_t semilla, float factorMovimiento = 1.0f);  // devuelve PÍXELES DE PANTALLA
```
- Sin estado y sin `rand()` global: hash entero (p. ej. mezcla tipo splitmix32 de
  `(semilla, floor(transcurrido*60))`) → dos valores en `[-1,1]`; envolvente
  `(1 - t/duracion)^2`; eje Y por `0.6` (conserva la anisotropía actual,
  `ApplicationCinematica.cpp:497`).
- `factorMovimiento` en `[0,1]` escala la amplitud (1 = normal, 0 = apagada; fuera de rango o no
  finito ⇒ Error). Deja abiertas D-O4 (a) off = 0 y (b) 25 % = 0,25.
- `transcurrido >= duracion`, `duracion<=0`, `amplitud<=0` o `factorMovimiento==0` ⇒ `{0,0}`
  **exacto** (acaba a cero). Misma entrada ⇒ misma salida (test de repetición); semillas
  distintas ⇒ distinto (test).
- `factorMovimiento` se **inyecta**; la política nunca lee `MOTOR_MOVIMIENTO_REDUCIDO` (la
  lectura de entorno sigue en el adaptador como en `ApplicationBucle.cpp:298`,
  `ApplicationHud.cpp:76`).
- La sacudida **nunca** toca la posición lógica: devuelve un offset de vista.

### E05 · Composición base + efectos (60 min)
```cpp
struct PoseCamara { Vector2 base; Vector2 vista; float zoom = 1.0f; };   // base = posición recortada; vista = base+sacudida/zoom
struct PoliticaPose { bool sacudidaDentroDeLimites = true; };            // regla D-T6
Result<PoseCamara> componerPose(const Vector2& posicionLogica, float zoom, const Vector2& sacudidaPantalla,
                                const TamanoVista& vista, const LimitesMapa* limites,
                                const PoliticaPose& politica);
```
- **Unidades**: la sacudida llega en píxeles de **pantalla** (salida de E04) y `componerPose` la
  divide por `zoom` para pasarla a mundo; es el único sitio donde se divide. `posicionLogica` y
  `zoom` son los **actuales** de `Camera` tras `update` (zoom interpolado, no el destino).
- Pose por fotograma: `base = limitarAlMapa(posicionLogica, zoom, vista, limites)` (E03, con el
  zoom actual); luego `vista = base + sacudida/zoom`; con
  `sacudidaDentroDeLimites` se limita `vista` otra vez con E03 (en un borde la sacudida solo se
  mueve hacia dentro; no se ve vacío por sacudida). `base` **nunca** recibe la sacudida
  (cero deriva acumulada; test: 10 000 llamadas, `base` bit-idéntica). `PoseCamara.vista -
  posicionLogica` es el **desplazamiento de vista** que el adaptador entrega a `Camera`
  (incluye el recorte de límites y la sacudida, ver E06 y D-T5/D-T12).
- Casos numéricos obligatorios (mapa 64x64, tiles 64x32 ⇒ caja `izquierda=-2016`, `derecha=2080`,
  `arriba=0`, `abajo=2048`; rombo `(32,0) (2080,1024) (32,2048) (-2016,1024)`; vista 800x600; **r4**: el centro se proyecta
  sobre caja∩rombo): zoom 1,7, centro x=-2000 y=1000 ⇒ `(-1780,706; 1000)` (la caja
  recorta X, el punto ya está en el rombo); zoom 1, centro (0,0) ⇒ `(0,300)` (solo Y; ya en el rombo);
  zoom 1, centro (-2000,-50) ⇒ proyección convexa sobre caja∩rombo ⇒ **`(-1573,6; 802,8)`** (r4; antes
  `(-1406,4; 719,2)` con «caja y luego rombo»); zoom 3, centro (2080,2048) ⇒ **`(1670,4; 1228,8)`** (r4; antes
  `(1603,733; 1262,133)`); zoom 1, centro **exactamente** en `min+mitad` y sobre el eje del rombo
  (-1616,1024) ⇒ identidad, y (-1616,001; 1024) ⇒ `(-1616,1024)` con tolerancia 1e-3; mapa 3x2 (`izq=-32,
  der=128, arr=0, aba=80`) con vista 800x600 ⇒ `base=(48,40)` (centro del paralelogramo); cambio de
  mapa: destino viejo (1500,1000) sobre mapa 3x2 ⇒ `(48,40)`.
- `limites == nullptr` ⇒ sin límites (dónde se aplican: D-O6). Puntero opcional en lugar de
  `std::optional` (elección de estilo; E05 puede cambiarlo).

### E06 · Conectar con `Camera` y `juego` (90 min, DOCUMENTO + adaptador)

**Cambio de diseño respecto a la v0**: el adaptador **no recibe `Application&`**, así que no
necesita `friend`. `Application.h:141-143` declara tres `friend`
(`ApplicationSceneTestAccess`, `ApplicationTestAccess` y `ApplicationCinematica`); añadir otro `friend` o miembros haría crecer
`Application.h` (523 líneas), que se quiere dejar sin crecimiento neto. Las funciones del
adaptador son libres de `Application`: reciben `Camera&`, el estado y los datos ya extraídos por
el llamador (que sí es miembro de `Application` o `ApplicationCinematica`).

```cpp
// include/Engine/ApplicationCamara.h
// #include <cstdint>  "Core/Math/PoliticaCamara.h"  "Core/Math/Vector2.h"  "Render/Camera.h"   (nunca Application.h)
struct EstadoCamaraJuego {                       // propietario: EstadoCinematicas::camara (ver abajo)
    Vector2 destino;  bool conDestino = false;   // destino vigente del seguimiento, YA LIMITADO (D-T12)
    float amplitudPx = 0, duracion = 0;  double reloj = 0;  std::uint32_t semilla = 0;
};
namespace ApplicationCamara {
    // Reinicio (cambio de mapa, entrar/salir de combate, pausa): conDestino=false, sacudida a 0, desplazamiento a 0.
    // Idempotente. Tras reiniciar, el siguiente seguimiento centra en el actor.
    void reinicia(EstadoCamaraJuego& e, Camera& cam);
    // Paso 1: sustituye el bloque ApplicationBucle.cpp:152-172. Solo se llama cuando el jugador CAMBIA de casilla y no es Battle.
    void sigueJugador(Camera& cam, EstadoCamaraJuego& e, const Vector2& objetivoMundo,
                      const PoliticaCamara::LimitesMapa* limites, const PoliticaCamara::ZonaMuerta& zona);
    // Disparo (sustituye `e.sacudida = max(...)` de ApplicationPantallas.cpp:429,431).
    void sacude(EstadoCamaraJuego& e, float amplitudPx, float duracion, std::uint32_t semilla);
    // Paso 4: tras Camera::update(dt) compone y entrega el desplazamiento de vista.
    void compone(Camera& cam, EstadoCamaraJuego& e, float dt,
                 const PoliticaCamara::LimitesMapa* limites, float factorMovimiento);
}
```
(`limites == nullptr` = sin límites, D-O6; `enCombate`/`cambioDeCelda` ya no son parámetros: los
decide el llamador en `ApplicationBucle.cpp`.)

**Propietario y ciclo de vida de `EstadoCamaraJuego`**: miembro `EstadoCamaraJuego camara;` de
`EstadoCinematicas` (`include/Engine/ApplicationCinematica.h:38`, que ya posee `sacudida` en
`:51` y cuelga de `Application` por `std::unique_ptr<EstadoCinematicas> m_cine`,
`Application.h:348`). Así **`Application.h` no cambia**. Nace con `m_cine` y se reinicia en
`loadLevel` (centinela de `m_lastCameraCell`, `ApplicationNivel.cpp:470-471`) sin editar
`ApplicationNivel.cpp`. **Detección del cambio de mapa** (corregida en r2): el centinela se asigna a
`m_lastCameraCell` (`ApplicationNivel.cpp:471`), no a la celda del jugador (`playerPosition()`,
`ApplicationBucle.cpp:162`), así que el adaptador **no puede** verlo. La detecta el propio
`ApplicationBucle.cpp`, antes del bloque de seguimiento y antes de que `:171` sobrescriba
`m_lastCameraCell`: `if (m_lastCameraCell.x == -9999 && m_lastCameraCell.y == -9999)
ApplicationCamara::reinicia(m_cine->camara, *m_camera);` (con `m_cine != nullptr`). Si el nivel se
cargara en modo `Battle` (no ocurre en los flujos actuales: **inferido**), `m_lastCameraCell` seguiría
siendo el centinela y se reiniciaría en cada fotograma; por eso `reinicia` es idempotente y el test
de E06 lo cubre. Prueba exigida: cambio de mapa **durante una sacudida** (sacudida activa de 0,24 s,
`loadLevel`, siguiente `update`): `amplitud`, `reloj`, desplazamiento de vista y `destino` quedan a
cero/vacíos y la pose es la centrada del mapa nuevo (caso 3x2 ⇒ `(48,40)` del §E05).
`EstadoCinematicas::sacudida` (`:51`) se **elimina** y los dos disparos de
`ApplicationPantallas.cpp:429,431` (`e` es `*app.m_cine`, ya accesible; no hace falta `friend`)
pasan a `ApplicationCamara::sacude(e.camara, amp, dur, semilla)`.

**Numeración de líneas**: las citas de `ApplicationCinematica.cpp` de este E06 son del HEAD actual
(antes de E06a). Tras E06a hay que restar 54 a todo número posterior a `:317`: `:398`→`:344`,
`:396,437,479`→`:342,383,425`, `:441`→`:387`, `:492-500`→`:438-446`. Las de `ApplicationBucle.cpp`
no cambian.

**Secuencia exacta por fotograma** (`Application::update`, `ApplicationBucle.cpp:152-175`):
0. Reinicio por mapa nuevo (arriba): `m_lastCameraCell == centinela` ⇒ `reinicia`.
1. Seguimiento (sustituye `:152-172`, **incluida la llave `}` de `:172`** que cierra el `if` de `:164`;
   con `:152-163` de comentario). Si el jugador cambia de casilla y no es `Battle`
   (condición de `:164`): `objetivo = gridToScreen(celda)+desfasePx` (`:165-167`);
   `destino = seguirConZonaMuerta(e.destino, zoom, objetivo, zona)` (con `conDestino=false` ⇒
   centrado en el objetivo); `destino = limitarAlMapa(destino, zoomActual, ...)`;
   **`e.destino = destino` (se guarda ya limitado)**; solo si difiere del destino anterior,
   `transitionTo(destino, zoom, 0.16f, Linear)` (`:170`); `m_lastCameraCell = celda`.
2. `ApplicationCinematica::combate` (`:173`): puede llamar `transitionTo` con **destinos sin limitar**
   (`ApplicationCinematica.cpp:396,437,479`) y **no actualiza `e.destino`**; se acepta porque el
   límite se aplica a la pose (paso 4), y al entrar/salir de combate se llama `reinicia` (R5), con lo
   que el primer seguimiento posterior vuelve a centrar. (Resuelve la aparente contradicción
   pasos 1/2: `e.destino` es solo la memoria del **seguimiento**, ya limitada; la verdad de la
   cámara es `Camera` y la pose final se limita siempre.) `combate` **ya no** llama a `ponVista`.
3. `m_camera->update(deltaTime)` (`:175`): `Camera.cpp:49-59` interpola posición **y zoom** y fija el
   destino exacto al acabar.
4. `ApplicationCamara::compone` **inmediatamente después** (antes de `m_fog->updateTexture` y
   `m_lightMap->updateTexture`, `ApplicationBucle.cpp:183-188`): lee `cam.position()` y `cam.zoom()`
   (de este fotograma), llama `componerPose` y entrega
   `cam.setDesplazamientoVista(pose.vista - posicionLogica)`. El límite se cumple también durante
   un `transitionTo` de combate con zoom ×1,7 (caso x=-2000 del §E05).
4b. **Golpes** (cambio r2): nueva función `ApplicationCinematica::vistaGolpes(Application&)`
   (declarada en `ApplicationCinematica.h`, definida en `ApplicationCinematica.cpp`, +1 línea de
   declaración; con `m_pergamino != nullptr` hace
   `golpes().ponVista(m_camera->posicionVista(), m_camera->zoom())`) llamada **justo después de
   `compone`**. Se elimina `ApplicationCinematica.cpp:441` (`ponVista` dentro de `combate`, que
   usaba la cámara del fotograma anterior: `GolpesEnPantalla.h:32-34` solo guarda centro y zoom y
   `GolpesEnPantalla.cpp:46-47` los usa al proyectar). Criterio numérico: viewport 800x600, zoom 2,
   desplazamiento de vista `(1,5; -1)` (= sacudida `(3,-2)` px / zoom 2), punto de mundo `(100,50)` ⇒
   pantalla `((100-1,5)*2+400, (50+1)*2+300) = (597; 402)`; la proyección de `GolpesEnPantalla`, la de
   `Camera::getViewProjectionMatrix` convertida a píxeles y la de `EstadoJuego`
   (`camaraX/camaraY`) deben coincidir a `|Δ| < 0,01 px` en cada fotograma de una sacudida de
   0,24 s con una transición de zoom 1→1,7 simultánea.
5. Niebla, luces, HUD del mundo (`ApplicationHud.cpp:379-380`), golpes y render leen
   `cam.posicionVista()` (D-T11).
**Título**: `ApplicationCinematica::titulo` (`:270-311`; tras E06a vive en
`ApplicationCinematicaTitulo.cpp`) llama a su propio `update(dt)` (`:310`): E06 añade ahí, justo
después, `ApplicationCamara::compone(..., limites=nullptr, ...)` (D-O6) para dejar el
desplazamiento a 0. Por eso **`ApplicationCinematicaTitulo.cpp` entra en el ámbito de E06**
(E06a lo crea sin cambios de comportamiento).

**Cambio en `Camera`** (única edición del consumidor): miembro `Vector2 m_desplazamientoVista`
(15º de datos); `void setDesplazamientoVista(const Vector2&)`; `Vector2 posicionVista() const`
(= `m_position + m_desplazamientoVista`). `getViewProjectionMatrix` (`Camera.cpp:102`) y
`screenToWorld` (`:111`) usan `posicionVista()`. `position()` sigue siendo la lógica.
`transitionTo` y el reloj float no se tocan (`Camera.cpp:73-89`). Un solo desplazamiento
representa límites **y** sacudida: así no hace falta un segundo miembro.

**Consumidores que pasan de `position()` a `posicionVista()`** (todos dentro del ámbito de E06):
`src/Engine/ApplicationHud.cpp:379-380` (alimenta `EstadoJuego`, de donde leen
`CartelesPuertas`, `BocadillosPnj`, `GuiaMapa`, `InterfazPergamino`; con ello **no se editan**
esos cuatro ficheros), `src/Render/FogOfWar.cpp:129-132`, `src/Render/DynamicLightMap.cpp:90-91`,
`ApplicationCinematica.cpp:441` (se mueve a `vistaGolpes`, paso 4b). `Culling.h`/`IsometricRenderer` van por `screenToWorld` y la
matriz (ya cubiertos). `ApplicationHud.cpp:378-381` pasa a leer también el zoom (sin cambios).
El picking (`Application.cpp:1013-1017`, `ApplicationPantallas.cpp:341`) usa `screenToWorld`,
es decir la vista: coincide con lo dibujado.

**Extracción previa obligatoria (E06a, mecánica, sin cambio de comportamiento)**:
`ApplicationCinematica.cpp` tiene 516 líneas (`wc -l`) y quitar `:492-500` (9) solo da 507.
Se mueven a un fichero nuevo `src/Engine/ApplicationCinematicaTitulo.cpp` el bloque
`namespace { acabaEn }` (`:264-268`), `ApplicationCinematica::titulo` (`:270-311`) y
`alSalirDelTitulo` (`:313-317`; usa `acabaEn`, que no se usa en ningún otro punto: grep). Son
54 líneas ⇒ 516 → 462 → ~452 tras E06 (`:492-500` y `:398`). Registro: una línea en `CMakeLists.txt` junto a `:835`.
Includes que copia `ApplicationCinematicaTitulo.cpp` (derivados de lo que usan `titulo`,
`acabaEn` y `alSalirDelTitulo`, **inferido**; E06a debe compilar para confirmarlo):
`Engine/ApplicationCinematica.h`, `Engine/Application.h`, `Engine/ApplicationInternal.h`
(`kTileW/H`), `Render/Camera.h`, `Render/AnimatedEntity.h`, `Render/VidaNivel.h`, `Render/Player.h`,
`<iostream>`, `<string>`, `<functional>`. Cabecera sin cambios (son miembros ya declarados en
`ApplicationCinematica.h`). Va en su propio commit **antes** del resto de E06.
La llave: el bloque a quitar es `:492-500` (comentario en `:492`; el `if` abre en `:493`, el `else` está en
`:499` y la llave cierra en `:500`; `:501` cierra la función). Quitar solo `:493-499` dejaría una
llave huérfana.

- Reinicio por **pausa**: cómo se detecta una pausa en `juego` **no está verificado** en esta E
  (tarea de E06: localizar quién deja de llamar a `Application::update`); el requisito es que el
  `reloj` no avance y el desplazamiento vuelva a 0 al reanudar.

**Implementación r1 de E06 (orden ejecutable, desviaciones conscientes)**:
- La secuencia 0-4 vive ahora en `ApplicationCamara::fotogramaCamara(cam, estado, EntradaFotograma, PasosFotograma)`
  (sin GL, sin `Application`); `Application::update` la llama una vez con tres callbacks (combate, golpes, niebla y
  luces). `ApplicationCamara::actualizaTitulo` hace lo mismo para el título (sin límites ni sacudida). Los tests
  ejecutan la función con dobles que registran el orden y la vista que ve cada consumidor.
- El `reinicia` de entrada/salida de combate ya no está en `ApplicationCinematica::combate`: lo hace
  `fotogramaCamara` al cambiar la identidad del combate (`EstadoCamaraJuego::combate`), justo después del paso del
  combate (mismo efecto: `reinicia` no toca el tween en curso).
- `movimientoReducidoEnEntorno()` ya no cachea (lee el entorno en cada llamada) para que las pruebas lo conmuten.
- **Desviación consciente**: la pausa **congela** la sacudida (`compone` con `dt = 0`; la vista conserva el
  desplazamiento) en vez de llevarla a 0. Hoy no hay sacudida fuera de combate, así que no se ve.
- **Hueco documentado**: lo único no ejecutable sin ventana es que HUD, niebla y luces lean `posicionVista()` y que
  `Application::update` y el título llamen a las dos funciones; los vigila un test textual sobre el código sin
  comentarios (solo presencia). `GolpesEnPantalla::aPantalla` (nuevo, la fórmula de `pinta`) se compara con la matriz
  de la cámara y con la fórmula de `EstadoJuego` (copiada en el test; el HUD real no es gl-free).
- **Para E07**: tras un combate, cambiar de casilla durante la transición de salida (0,6 s) deja el zoom intermedio
  (preexistente, no introducido por E06).

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
| E06a | `src/Engine/ApplicationCinematicaTitulo.cpp` | `src/Engine/ApplicationCinematica.cpp` (quitar `:264-317`), `CMakeLists.txt` (+1 línea junto a `:835`) | Solo movimiento; `ApplicationCinematica.cpp` 516 → 462 |
| E06 | `include/Engine/ApplicationCamara.h`, `src/Engine/ApplicationCamara.cpp`, `tests/Engine/ApplicationCamaraTests.cpp` (GL-free, sin `Application`), `cmake/ApplicationCamara.cmake` | `include/Render/Camera.h` (+1 miembro, +2 métodos), `src/Render/Camera.cpp` (`:102`, `:111` + 2 métodos), `src/Engine/ApplicationBucle.cpp` (sustituir `:152-172`, añadir reinicio por centinela antes y `compone` + `vistaGolpes` tras `:175`; neto ≈ −2), `src/Engine/ApplicationCinematica.cpp` (HEAD actual: quitar `:492-500`, `:398` y `:441`; tras E06a: `:438-446`, `:344`, `:387`; añadir `vistaGolpes`), **`src/Engine/ApplicationCinematicaTitulo.cpp`** (creado por E06a; E06 añade `compone` tras el `update` del título), `include/Engine/ApplicationCinematica.h` (`sacudida` `:51` → `EstadoCamaraJuego camara;` y declaración de `vistaGolpes`), `src/Engine/ApplicationPantallas.cpp` (`:429,431`), `src/Engine/ApplicationHud.cpp` (`:379-380`), `src/Render/FogOfWar.cpp` (`:129-132`), `src/Render/DynamicLightMap.cpp` (`:90-91`), `CMakeLists.txt` (fuentes `ApplicationCamara.cpp`) | **`Application.h` NO cambia**. `ApplicationNivel.cpp` NO se edita. Cruce: ver R3 |
| E07 | `out/fabrica/REPLICA-R1/` solo evidencia | tests de política (esquinas) | Sin código de producto |

Límites: `Camera.h` 103 → ~108 líneas; `Camera.cpp` 116 → ~125. `Camera` pasaría a **15**
miembros de datos (`m_desplazamientoVista`): exactamente el límite; cualquier miembro más obliga a extraer
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
- **D-T5** La sacudida **y** el recorte de límites son un único **desplazamiento de vista**
  (`setDesplazamientoVista`) separado de `m_position`, no un `transitionTo`. Razón: cumple «no altera posición lógica» de
  la ficha y evita que la sacudida corrompa `m_transitionStart*` (hoy lo hace,
  `ApplicationCinematica.cpp:497`). *E05, E06.*
- **D-T11** La vista (con desplazamiento) es lo que se lee para proyectar mundo→pantalla:
  `ApplicationHud.cpp:379-380` lee `posicionVista()` y de ahí heredan carteles, bocadillos, guía
  de mapa y avisos (`EstadoJuego`, `InterfazPergamino.h:78`); también niebla, luces y golpes. Así
  todo tiembla con el mundo. `position()` queda solo para lógica. *E06.*
- **D-T12** El límite se aplica a la **pose por fotograma** (tras `update`, zoom interpolado), no
  solo al destino del seguimiento; los `transitionTo` de combate/título quedan sin tocar
  (`ApplicationCinematica.cpp:396,437,479`). *E05, E06.*
- **D-T13** Adaptador sin `Application&` ni `friend` nuevo; estado en `EstadoCinematicas`;
  extracción previa E06a. *E06.*
- **D-T6** Regla sacudida vs límites: **recortar la vista final** (`sacudidaDentroDeLimites=true`):
  nunca se ve vacío por sacudida; en un borde el temblor es unilateral. Alternativa B: sacudida
  libre (puede mostrar ≤ amplitud px de vacío). Depende de D-O3 si Oriol admite ver fuera.
  *E05.*
- **D-T7** Margen superior de `limitesDeMapa`: parámetro `margenSuperiorPx` (defecto 0) para
  sprites altos y alturas de terreno (`Alturas::desfasePx` es negativo hacia arriba,
  `include/Runtime/Alturas.h:93`); el valor concreto lo fija E03 con el nivel de mayor altura. *E03.*
- **D-T8** (pasa a producto: ver **D-O6**). Mientras Oriol no decida, defecto = exploración y
  combate con límites, título sin ellos. *E06.*
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
| **D-O3** | ¿La cámara puede mostrar fuera del mapa? | (a) nunca (salvo mapa menor que la vista: centrado); (b) margen de 1 tile; (c) libre | **(a)**; mapas menores que la ventana se centran con el vacío alrededor. **Precisión de Oriol (30/10, isométrico): «el centro de la pantalla siempre sobre terreno»**: caja + proyección del centro al rombo real del mapa; en bordes/esquinas puede verse algo de vacío | Parámetro por defecto en **E06** y criterio de aceptación de **E07**; E03 lo implementa parametrizado (0 = estricto) |
| **D-O4** | Reduced motion: ¿sacudida apagada o atenuada? | (a) off (`factorMovimiento=0`); (b) 25 % (`0,25`) | **(a) off**, coherente con cómo el HUD omite animaciones (`DialogoHoja.cpp:219`) | Defecto en **E06**; E04 ya admite ambos |
| **D-O6** | ¿Dónde se aplican los límites? | (a) exploración y combate, no título/cinemáticas; (b) también combate sin límites (el encuadre del combate manda); (c) solo exploración | **(a)**; revisar en E07 si el combate junto al borde queda descentrado | Defecto en **E06**; E07 lo verifica |
| **D-O5** | Aprobación visual/jugable (dead zone, esquinas, sacudida a escala real) | — | Observación real por Oriol | **E07**, y el cierre del paquete (porta de la ficha) |

## 6. Discrepancias con la ficha (registradas, no resueltas)

- **C1.** La ficha habla de «seguimiento (lerp exponencial)» como estado actual de la cámara. En
  `juego` **no se usa**: sigue con `transitionTo` 0,16 s por cambio de casilla
  (`ApplicationBucle.cpp:170`); `move`+lerp lo usa solo el editor (`EditorApplication.cpp:340`).
  E02/E06 deben partir de `transitionTo`, no del lerp.
- **C2.** La ficha pide una sacudida «determinista» y que «no altere la posición lógica»; ya
  existe una sacudida integrada que no cumple ninguna de las dos
  (`ApplicationCinematica.cpp:492-500`: estado `static`, `transitionTo`). E06 la sustituye, no la
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
- **C3b.** Renombres de operaciones de la ficha: `followTarget→seguirConZonaMuerta`,
  `clampCamera→limitarAlMapa` (+`limitesDeMapa`), `shakeOffset→desplazamientoSacudida`,
  `cameraPose→componerPose`; E01, E06 y E07 son documentos y no tienen operación nueva (E06 añade el
  adaptador `ApplicationCamara`). Además `shakeOffset(..., double t, uint32_t seed)` gana
  `factorMovimiento` y devuelve `Result`.
- **C5.** U3-07 documentó `Camera.cpp` con 109 líneas; ahora son 116 (post-E08, `628f417`).
- **C6.** Dependencia de E `PREP.E10` y de paquete U3-07: U3-07 está integrado en la base;
  `PREP.E10` no verificada aquí.

## 7. Riesgos y trampas

- **R1. Límite de miembros de `Camera`**: 14 de datos hoy (`Camera.h:85-102`); +`m_desplazamientoVista` = 15.
  Contando métodos (14 + 2) ya se pasa. Si se quiere holgura, extraer los 8 de la transición a
  `struct EstadoTransicion` dentro de `Camera.h` (neutral en comportamiento, cubierto por
  `CameraTransitionTests.cpp:228-273`); queda como decisión del Brain en E06, no en v1.
- **R2. `position()` vs posición de vista**: `FogOfWar`/`DynamicLightMap`/`golpes->ponVista` leen
  `position()` (`FogOfWar.cpp:129-132`, `DynamicLightMap.cpp:90-91`, `ApplicationCinematica.cpp:441`).
  Si la sacudida solo cambia la matriz, niebla y luces se despegan del mapa durante el temblor.
  E06 debe migrarlos y probar alineación; los golpes leen la vista sacudida (D-T11) vía
  `vistaGolpes` tras `compone` (E06 paso 4b).
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
  `Player` (que va con paso de 0,16 s, `src/Render/Player.cpp:27`; update en `:61-66`), o aparecerá jitter por doble tween
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
  defecto concretos no entran en la política). E05 fija la regla D-T6 con el valor «recortar» como
  defecto del parámetro; si Oriol responde D-O3 (b) o (c) basta cambiar `PoliticaPose`. Pueden
  arrancar en cuanto el Brain dé por buenas D-T1…D-T13.
- **E06a** (extracción mecánica de `titulo`) puede hacerse ya, independiente de todo lo demás.
- **Parcial**: E06 (integración con valores) necesita D-O1, D-O2, D-O3, D-O4, D-O6; puede prepararse
  con defecto = comportamiento actual (zona 0, amplitud actual, límites estrictos, reducido off,
  límites en exploración y combate).
- **Bloqueada por Oriol**: E07 (D-O5, observación real).

## 9. Cierre de E01

- Comandos usados: lectura con `sed -n`/`cat -n` de cada tramo citado; `grep -rn` de
  `Camera`, `m_camera`, `setViewportSize`, `sacudida`, `MOTOR_MOVIMIENTO_REDUCIDO`, `fitMap`;
  `wc -l` (Camera.h 103, Camera.cpp 116, CameraTests 272, CameraTransitionTests 301,
  ApplicationNivel 506, ApplicationCinematica 516, ApplicationBucle 473). Sin build ni tests
  (E documental).
- Citas comprobadas por contenido (ronda 1): `out/fabrica/REPLICA-R1/U4-01.E01_citas.log` (versionado; r2: para cada rango imprime la primera y la última línea y comprueba que existen todas las del rango; el contenido de las intermedias no se compara).
- Hash de contexto: base `628f417`.
