# U3-07.E01 · Inventario de interpolaciones y relojes reales

Estado: **documento de E01; no autoriza ningun cambio de codigo.** Base `9d52ae8`.
Todas las rutas son relativas a `MotorGraphico-main/MotorGraphico/`. Cada afirmacion
lleva `fichero:linea` abierto en esta sesion. Sala limpia: nada de Unity copiado; las
formulas son matematica estandar y las ya presentes en el repo. Nombres propuestos en
espanol, como el repo (el paquete U3-07.md usa nombres en ingles como placeholder).

Leyenda de estado: **E** = escrito; **I** = integrado en ejecutable (juego /
motor_editor / level_editor); **T** = con test.

## 1. Inventario

### 1.1 Interpolaciones con duracion fija (curva sobre progreso 0..1)

| # | Que | Donde | Tipo / unidad | Estado |
|---|-----|-------|---------------|--------|
| A | Easing de camara: `Linear`, `EaseInOutQuad`, `EaseOutCubic` | `src/Render/Camera.cpp:17-27`; enum `include/Render/Camera.h:46` | `float t` 0..1 -> `float`; formulas `2t^2` / `1-(-2t+2)^2/2` / `1-(1-t)^3` (`Camera.cpp:22,24`) | E I T |
| B | `Camera::update` con transicion: `elapsed/duration` clamp 0..1, `duration<=0` -> `t=1` en la PRIMERA `update`, incluso con `dt=0` (`Camera.cpp:34-41`; test `tests/Engine/CameraTransitionTests.cpp:26-34`: `update(0.0f)` aplica el destino; uso real `editor/src/level_editor.cpp:587-591`), sin dividir; mezcla `pos` y `zoom` con `start + (target-start)*eased`; al final fija el destino exacto | `src/Render/Camera.cpp:34-56` (clamp `:39-41`, mezcla `:43-45`, fijado `:51-53`); armado en `:66-82` (`duration=max(d,0)` `:73`) | **segundos** (float), posicion `Vector2` en **pixeles de mundo** (ya iso, no casillas), zoom escalar | E I T |
| C | Paso del jugador (`Player`): `progress=elapsed/duration`; factor `1-progress` (paso, lineal) o `sin(pi*progress)` (rebote al chocar); duracion `kStepDuration=0.16`, `kBumpDuration=0.08` | `src/Render/Player.cpp:61-66`; consts `:27,31,32`; `duracion` de `moveTo` `:143`; datos `include/Render/PlayerMotionData.h:8-9` | **segundos**; desplazamiento `Vector2` en pixeles (diferencia `gridToScreen` entre casillas, `Player.cpp:138-140`); la casilla logica es entera (`GridCoord`) y salta ya al llegar `:149` | E I T (`tests/Render/PlayerTests.cpp:394-420`) |
| D | Clave de orden de dibujo interpolada linealmente con el mismo progreso | `src/Render/Player.cpp:106-108` (`double progress`, `lround`) | segundos -> entero; no es visual | E I |
| E | Cinematica Trepa/Baja y Funde: `k=min(1, reloj/segundos)`, lineal, `from + (to-from)*k` | `src/Game/Cinematica.cpp:469-470` (trepa), `:486-487` (velo) | **segundos** (`m_reloj += dt` `:461`); sin clamp inferior | E I T (`tests/Game/CinematicaTests.cpp:86` ejecuta `avanza`) |
| F | Paseo de PNJ/jugador en cinematica: `q.t += dt*velocidad`, `t` en **casillas** (1.0 = una celda), resto pasa a la siguiente | `src/Game/Cinematica.cpp:509-523` (jugador), `:528` (PNJ) | casillas/segundo, no segundos | E I T |
| G | Pasar hoja (cuaderno): `m_t += dt/duracion` clamp 1; curvas propias `facilIn=0.25t+0.75t^2`, `facilOut=1+1.9u^3+0.9u^2` (u=t-1); `kDuracion=0.36`, `kFundido=0.16` | `src/Game/PasarHoja.cpp:14-18,33,39`; `include/Game/PasarHoja.h:33-34` | segundos -> grados (0..180) | E I (HUD cuaderno `src/HUD/Pergamino/Cuaderno.cpp`, usa `PasarHoja`) ; T no verificado |

### 1.2 Suavizados "hacia el objetivo" (sin duracion; no son tweens)

| # | Que | Donde | Unidad | Estado |
|---|-----|-------|--------|--------|
| H | Paneo de camara: `t=clamp(lerpSpeed*dt,0,1)`; `pos += (target-pos)*t` (lerp exponencial, `m_lerpSpeed=8`) | `src/Render/Camera.cpp:58-59`; `include/Render/Camera.h:88` | por segundo, depende de dt | E I T |
| I | Barra HUD: mismo patron, `animationSpeed*dt`, corte de convergencia `0.001*max(maxValue,1)` | `src/Render/HudWidgets.cpp:75-81` | por segundo | E I T (`demo_hud_update`) |
| J | Tinte (jugador/objetos): `acerca(v, objetivo, paso)` mueve a **paso fijo por componente** (no lerp), `paso=dt*1.2` jugador, `dt*1.5` objetos | `src/Engine/ApplicationCinematica.cpp:39-46` (funcion en namespace anonimo), usos `:226`, `:241` | alfa/color por segundo, `Vector4` componente a componente | E I ; T no verificado |
| K2 | Tinte de golpe (`EfectosDeGolpe::tinte`): cura = pulso `sin(pi*t)` (`src/Game/EfectosDeGolpe.cpp:86-89`); dano = parpadeo on/off cada 0.06 s con fundido lineal `1-0.5t` (`:92-94`); `t=edad/vida` sin clamp | segundos | E I (consumido en `ApplicationCinematica.cpp:228-230`) T parcial |
| K3 | Desplazamiento de golpe (`EfectosDeGolpe::desplazamiento`): progreso triangular (sube en 1/4, baja en 3/4) y **smoothstep** `c^2(3-2c)` (`EfectosDeGolpe.cpp:100-104`) | segundos -> pixeles `Vector2` redondeados | E I. Smoothstep = candidato futuro, **fuera del catalogo E03** (decision D2) |
| K | Hoja de golpe: decremento `m_hoja -= dt/kHoja` (0.9 s); edad de efectos `e.edad += dt` | `src/Game/EfectosDeGolpe.cpp:57-60`; `include/Game/EfectosDeGolpe.h:31` | segundos | E I T (`tests/Game/EfectosDeGolpeTests.cpp`) |

### 1.3 Fuentes de tiempo

- `dt` de frame (float, segundos): `Application::update(deltaTime)` -> `m_player->update(mundo)` con `mundo=0` durante la pausa del critico (`src/Engine/ApplicationBucle.cpp:136-142`); `m_camera->update(deltaTime)` (`:175`); `Cinematica` `avanza(dt,...)` (`src/Game/Cinematica.cpp:345`). El editor llama `map.camera.update(io.DeltaTime)` (`editor/src/EditorApplication.cpp:318`).
- Paso fijo + acumulador con alfa de interpolacion `interpolationAlpha` (`src/Engine/ApplicationLoop.cpp:92-94`, tipo en `include/Engine/ApplicationLoop.h:34`) **calculado pero sin consumidor visual**: `Scene::render(float)` lo ignora (`include/Engine/Scene/Scene.h:33`) y el propio codigo lo reserva "para RUN-07.4" (`src/Engine/ApplicationBucle.cpp:467`). No es un reloj de tween.
- Relojes de pared `steady_clock` (no dependen de `dt` ni de la pausa del mundo; todos **excluidos de la migracion en este paquete**):
  - `AuraJugador::segundos()` (`src/Render/AuraJugador.cpp:54-55`), segundos float desde `m_inicio`.
  - `EfectosDeCartas` (`src/Render/PintaDestellos.cpp:88-104`): `edad` en segundos (double) contra `Game::kDuracionDeCarta`, se evalua en `render`.
  - `DialogoHoja` tirada de dados (`src/HUD/Pergamino/DialogoHoja.cpp:220-227`): `t` en segundos float contra `dados::kRodar`; se salta con `MOTOR_MOVIMIENTO_REDUCIDO`.
  - `HojaPersonaje` vista previa (`src/Render/HojaPersonaje.cpp:185-189`): segundos double a `frameDeVistaPrevia`.
- `m_reloj` de cinematica: contador local reseteado por paso (`Cinematica.cpp:461`).
- Unidades: tweens en **segundos** salvo F (casillas) y los suavizados H/I/J (tasa por segundo). Posiciones visuales: `Vector2` float en pixeles de mundo; `GridCoord` (`include/Core/Math/GridCoord.h`) es entero y **nunca** se interpola (E05 lo tiene como trampa).

### 1.4 Tipos del repo a reutilizar

- `Vector2` global, `float x,y`, solo `+ - * float` (`include/Core/Math/Vector2.h:5-12`); `Vector4` (tinte) en `include/Core/Math/Vector4.h`. Sin `lerp` propio.
- `Result<T>` global, `Result<T>::Ok/Error(std::string)`, `errorMessage()`, `value()` lanza si es Error (`include/Core/Errors/Result.h:22-61`).
- Namespaces: `IsoMath` (`include/Core/Math/IsoMath.h:21`), `Game` (`include/Game/Cinematica.h:49`); Camera, Vector2, GridCoord, Result son globales.

### 1.5 Primer consumidor real para E08: **`Camera::transitionTo` / `Camera::update` (A+B)**

Justificacion:
1. Es el unico lugar con **easing nombrado + progreso + mezcla pos/zoom** ya duplicado; todo lo demas (C, E, G) o repite la mezcla lineal o usa curvas propias.
2. Esta **integrado en 3 ejecutables**: `juego` (`ApplicationBucle.cpp:170,175`, `ApplicationCinematica.cpp:309,396,437,479`), `level_editor` (`editor/src/level_editor.cpp:587`), `motor_editor` (`EditorApplication.cpp:318`), y **tiene tests** que dan comparacion numerica antes/despues (`tests/Engine/CameraTransitionTests.cpp:39,53`, `tests/Render/CameraTests.cpp:257-259`).
3. Vive ya en `motor_math` (`CMakeLists.txt:71`, GL-free, enlazado en `juego` `:935` y `level_editor` `:924`): la utilidad nueva entra sin dependencias nuevas.
4. Superficie minima: reemplazar `applyEasing` (`Camera.cpp:17-27`), el clamp (`:39-41`) y la mezcla (`:43-45`), manteniendo `enum Camera::Easing` (API publica y tests de valor entero `CameraTests.cpp:257-259`) como adaptador a la nueva enum.
5. Descartados para el primer: Player (C) mezcla ademas animacion y clave de orden; Cinematica (E) no tiene easing; tinte (J) no es lerp; PasarHoja (G) usa curvas no estandar.

Pendientes de comparar en E08: la camara trabaja en `float`, la utilidad en `double` (ver riesgo R2).

## 2. Contratos propuestos E02..E08

Convenciones: archivos nuevos `include/Core/Math/` + `src/Core/Math/` (hoy solo existe `include/Core/Math/`; `src/Core/Math/` se crea), tests en `tests/Core/` (existe), `TestHarness.h` con `MOTOR_CHECK` (nunca `assert`). Registro en un `.cmake` propio siguiendo el patron de `cmake/EfectosDeGolpe.cmake` / `cmake/RutaCasillas.cmake` (`add_executable` + `add_test` con `LABELS "unit;gl-free"`, `MOTOR_GL_FREE_TARGETS`, dependencia de `suite_gl_free`), **no** tocar el bloque gigante de `CMakeLists.txt`. CTest se incluye en `CMakeLists.txt:12` y `motor_math` se define en `:71`, asi que el `include(cmake/Interpolacion.cmake)` va justo despues de `:75` (una linea en `CMakeLists.txt`, autorizada solo en E02). Las fuentes se anaden con `target_sources(motor_math PRIVATE ...)` dentro del `.cmake`. Sin GL, sin glad; solo `<cmath>`, `Core/Errors/Result.h`, `Core/Math/Vector2.h`.

Reparto en dos pares para respetar <=500 lineas y <=15 miembros por cabecera:
- Par 1 (E02-E05): `Interpolacion.h/.cpp`.
- Par 2 (E06-E07): `EstadoTween.h/.cpp` (depende del par 1).

### E02 · Progreso temporal (45 min)

```cpp
// include/Core/Math/Interpolacion.h  (namespace Interpolacion)
Result<double> progreso(double transcurrido, double duracion);
```
- `duracion` no finita o `<0` -> `Error`; `transcurrido` NaN o +-inf -> `Error`.
- `duracion == 0` -> `Ok(1.0)` **sea cual sea el signo de `transcurrido`** (fin inmediato, sin dividir; `Camera.cpp:39-41`).
- `duracion > 0` y `transcurrido < 0` -> `Ok(0.0)` (clamp, como `Camera.cpp:39`); `transcurrido >= duracion` -> `Ok(1.0)`.
- `duracion` NaN/inf/negativa -> `Error`; `transcurrido` NaN/inf -> `Error` (se comprueba antes que el caso `duracion==0`).
- Archivos E02: crea `include/Core/Math/Interpolacion.h`, `src/Core/Math/Interpolacion.cpp`, `tests/Core/InterpolacionTests.cpp`, `cmake/Interpolacion.cmake`; edita 1 linea en `CMakeLists.txt` (tras `:75`).
- Target/test: `test_interpolacion` (labels `unit;gl-free`), cwd `${CMAKE_BINARY_DIR}`; enlaza `motor_math`.
- Casos: (0.5,1)->0.5; (2,0)->1.0, (0,0)->1.0 y (-1,0)->1.0; (-1,1)->0.0; (NaN,1)->Error; (1,-1)->Error.

### E03 · Easing (60 min)

```cpp
enum class TipoCurva { Lineal, EntradaCuadratica, SalidaCuadratica, EntradaSalidaCuadratica,
                       EntradaCubica, SalidaCubica, EntradaSalidaCubica };
double curva(TipoCurva tipo, double t);                 // t se acota a [0,1]; NaN -> 0.0 documentado
Result<TipoCurva> curvaDesdeTexto(const std::string&);   // parser para datos; desconocido -> Error
```
Formulas congeladas (t en [0,1]):
- Lineal `t`
- EntradaCuadratica `t^2`; SalidaCuadratica `1-(1-t)^2`;
- EntradaSalidaCuadratica `t<0.5 ? 2t^2 : 1-(-2t+2)^2/2` (identica a `Camera.cpp:22`)
- EntradaCubica `t^3`; SalidaCubica `1-(1-t)^3` (identica a `Camera.cpp:24`);
- EntradaSalidaCubica `t<0.5 ? 4t^3 : 1-(-2t+2)^3/2`.
Invariantes a probar: `f(0)=0`, `f(1)=1` exactos; monotonas no decrecientes; simetria `InOut(t)+InOut(1-t)=1`; `SalidaX(t)=1-EntradaX(1-t)`. Textos del parser: `lineal`, `entrada_cuadratica`, ... (en minusculas sin tildes; ver decision D3).
- Archivos E03: edita `Interpolacion.h/.cpp`, `tests/Core/InterpolacionTests.cpp`. Mismo target.
- Fuera de alcance E03: curvas `facilIn/facilOut` de PasarHoja (`PasarHoja.cpp:14-18`) y el seno de rebote (`Player.cpp:65`); no son estandar (decision D2).

### E04 · Escalar (45 min)

```cpp
double tweenEscalar(double a, double b, double p);       // nombre alt. propuesto: mezclaEscalar
```
- `p` se acota a [0,1]; `p<=0 -> a` y `p>=1 -> b` **exactos** (como el fijado final `Camera.cpp:51-52`); intermedio `a + (b-a)*p`.
- Entradas no finitas: devuelve NaN (no lanza); la validacion es del llamador (`progreso` ya devuelve `Result`). Decision D4 si Oriol prefiere `Result<double>`.
- Casos: (3,3,.5)=3; (10,0,.5)=5; (0,1,0)=0; (0,1,1)=1 exacto. Tolerancia de test `1e-12` (double puro); vs camara actual `1e-5` (float).
- Archivos E04: `Interpolacion.h/.cpp`, `InterpolacionTests.cpp`.

### E05 · Vector (60 min)

```cpp
Vector2 tweenVec2(const Vector2& a, const Vector2& b, double p);   // global Vector2, float
```
- Componente a componente con `tweenEscalar`, resultado en `float`; `p` acotado; sin heap/allocs (valor por retorno).
- **No** hay version para `GridCoord` (entero): la casilla logica no se interpola; solo el desplazamiento visual en pixeles (ver `Player.cpp:138-140`). Test negativo: no compila con `GridCoord` (no hay sobrecarga).
- Opcional diferido: `Vector4` para tinte (J), fuera de E05 salvo decision.
- Casos: ({0,0},{10,20},.5)={5,10}; p=1 devuelve b exacto; a==b constante.
- Archivos E05: `Interpolacion.h/.cpp`, `InterpolacionTests.cpp`. (<=15 miembros: ~8 funciones/tipos libres.)

### E06 · Avanzar y completar (90 min)

```cpp
// include/Core/Math/EstadoTween.h
struct EstadoTween { double duracion = 0; double transcurrido = 0; TipoCurva curva = TipoCurva::Lineal;
                     bool pausado = false; bool terminado = false; };
struct PasoTween  { EstadoTween estado; double progreso = 0; double valorCurvado = 0; bool acabaDeTerminar = false; };
Result<PasoTween> avanzaTween(const EstadoTween& estado, double dt);
```
- `dt` no finito o `<0` -> `Error`; `dt==0` o `pausado` -> mismo estado, `acabaDeTerminar=false`.
- `acabaDeTerminar` es verdadero **solo** en la llamada que cruza el final; `terminado` ya cierto -> `false` siempre (no re-emite). `duracion==0` (no pausado) termina en el primer avance, **incluso con `dt==0`**, igual que `Camera::update` (`Camera.cpp:34-41`, test `CameraTransitionTests.cpp:26-34`); `pausado` manda sobre todo (no avanza ni termina). Con `duracion>0`, `dt==0` no avanza.
- Equivalencia de particion: avanzar `dt` en n trozos == avanzar `dt` de una vez (tolerancia `1e-12`).
- Tolerancia de final relativa (r1): termina si `transcurrido+dt >= duracion - kTolFin*duracion`, `kTolFin = 1e-9`, y fija `transcurrido=duracion` (10 x 0.1 acumula 0.9999999999999999). Un margen de 1e-6 NO termina. `dt==0` nunca avanza ni termina con `duracion>0`, aunque `transcurrido>=duracion`.
- Archivos E06: crea `include/Core/Math/EstadoTween.h`, `src/Core/Math/EstadoTween.cpp`, `tests/Core/EstadoTweenTests.cpp`, ampl. `cmake/Interpolacion.cmake`. Target/test `test_estado_tween`.

### E07 · Cancelar (60 min)

```cpp
enum class PoliticaCancelacion { MantenerActual, Terminar };
PasoTween cancelaTween(const EstadoTween& estado, PoliticaCancelacion politica);
```
- `MantenerActual`: congela `transcurrido`, `terminado=true`, valor = el actual. `Terminar`: `transcurrido=duracion`, `progreso=1`.
- Idempotente (cancelar dos veces no cambia nada y `acabaDeTerminar=false` la segunda); sin callbacks (ni `std::function`) en esta capa: el llamador reacciona a `acabaDeTerminar`.
- Cancelar un tween ya terminado por avance: sin efecto.
- Archivos E07: `EstadoTween.h/.cpp`, `EstadoTweenTests.cpp`.

### E08 · Consumidor: `Camera` (90 min)

- Sustituir en `src/Render/Camera.cpp` `applyEasing` (`:17-27`), el calculo de `t` (`:39-41`) y las dos mezclas (`:43-45`) por `Interpolacion::progreso/curva/tweenVec2/tweenEscalar`; mapear `Camera::Easing` -> `TipoCurva` (tres valores existentes). Mantener la API publica de `include/Render/Camera.h:46-57` intacta.
- Archivos E08: `src/Render/Camera.cpp` (<=500 lin., hoy 109), `tests/Engine/CameraTransitionTests.cpp` (prueba de regresion numerica: muestras de `t` en 0, .25, .5, .75, 1 para las 3 curvas contra los valores previos, tolerancia `1e-5`). `include/Render/Camera.h` solo si se anade un miembro; evitarlo.
- Reloj (decision Brain, r1): `Camera::update` acumula en `float` (`Camera.cpp:36`); con 60 x (1.0f/60) llega a 0.99999970 y sigue activa, mientras `avanzaTween` en double llega a 1.00000005 y termina un fotograma antes. En E08 `Camera` CONSERVA su reloj float y solo reutiliza `progreso/curva/tweenVec2/tweenEscalar`; si algun dia migra a `avanzaTween` sera con prueba diferencial del fotograma de fin.
- Prueba real: `test_camera_transition`, `test_occlusion`, `demo_camera` y arrancar `juego` (camara al cambiar de casilla, `ApplicationBucle.cpp:170`) + `level_editor` (`level_editor.cpp:587`); observacion visual/jugable = pendiente de Oriol, no se declara como verificada por test.
- Porta de cierre del paquete: la utilidad esta "produida pendent d'integracio" hasta que E08 llegue a `juego`.

## 3. Alcance de codigo congelado propuesto (resumen)

```
include/Core/Math/Interpolacion.h        (E02-E05)
src/Core/Math/Interpolacion.cpp          (E02-E05)
include/Core/Math/EstadoTween.h          (E06-E07)
src/Core/Math/EstadoTween.cpp            (E06-E07)
tests/Core/InterpolacionTests.cpp        (E02-E05)  target: test_interpolacion
tests/Core/EstadoTweenTests.cpp          (E06-E07)  target: test_estado_tween
cmake/Interpolacion.cmake                (E02, E06) + 1 linea include(...) en CMakeLists.txt tras :75 (E02)
src/Render/Camera.cpp                    (E08)
tests/Engine/CameraTransitionTests.cpp   (E08)
```
Ningun fichero >500 lineas ni cabecera >15 miembros previsto; sin GL; `require`/`MOTOR_CHECK`, nunca `assert`. Comando de prueba: `cmake --build <build> --target test_interpolacion test_estado_tween && ctest --test-dir <build> -R "test_interpolacion|test_estado_tween" -L gl-free` (cwd = build; confirmar nombre de directorio de build con el coordinador).

## 4. Riesgos y trampas

- **R1. Doble contabilidad de tiempo en el jugador**: `Player` mezcla animacion de frames (`AnimatedEntity::update(min(dt, duration-elapsed))`, `Player.cpp:61`) y movimiento; fuera de E08 por eso.
- **R2. float vs double**: Camera/Player usan `float`; el util propuesto `double`. Resultado final en float puede diferir ~1e-7; el fijado exacto del destino (`Camera.cpp:51-52`) debe conservarse. La comparacion antes/despues exige tolerancia explicita (E04/E08).
- **R3. Division por cero latente existente**: `Funde` (`Cinematica.cpp:112-117`) no valida `segundos>0` y `:486` divide `m_reloj/p.segundos`; `Trepa/Baja` si lo valida (`:93`). Registrar, no corregir en U3-07.
- **R4. Truco de duracion 0.001**: `ApplicationCinematica.cpp:309` usa `0.001f` para "corte"; la camara ya admite 0 (`Camera.cpp:39`). Tras E08, no cambiar este comportamiento sin decision.
- **R5. Reinicio desde posicion actual**: `transitionTo` encadena sin saltos porque guarda `start = m_position` (`Camera.cpp:75`); el util es puro y sin estado, esa semantica sigue en Camera.
- **R6. `interpolationAlpha` huerfano** (`ApplicationLoop.cpp:92-94`): no reutilizar como reloj de tween; si se conecta (RUN-07.4) es otro paquete.
- **R7. Relojes de pared** (`AuraJugador.cpp:54`, `PintaDestellos.cpp:88-104`, `DialogoHoja.cpp:220-227`, `HojaPersonaje.cpp:185-189`): ignoran la pausa; fuera de este paquete.
- **R8. Casilla vs pixel**: no interpolar `GridCoord`; E05 expone solo `Vector2`.
- **R9. Contratos de camara**: `Camera::Easing` y sus valores enteros 0/1/2 estan testeados (`CameraTests.cpp:257-259`); no renumerar.
- **R10. Colision de ficheros comunes**: `CMakeLists.txt` es zona caliente multiagente; limitar a 1 linea y usar `.cmake` propio (patron `cmake/EfectosDeGolpe.cmake`).
- **R11. Estado de trabajo**: las E son secuenciales por dependencia (E02->...->E08), no paralelizables sobre `Interpolacion.h/.cpp`.

## 5. Decisiones

Resueltas (30/10):
- **D2 (Oriol)**: catalogo de curvas = `Lineal` + Cuadratica/Cubica x Entrada/Salida/EntradaSalida (7 en total). `facilIn/Out` de PasarHoja, el seno del rebote y smoothstep quedan fuera.
- **D1 (Brain)**: `Camera` es el primer consumidor de E08.
- **D3 (Brain)**: nombres de curvas en datos en espanol, como el enum (`lineal`, `entrada_cuadratica`, ...).
- **D4 (Brain)**: `tweenEscalar`/`tweenVec2` son funciones totales sobre entradas finitas y acotan `p` a [0,1]; la validacion de no finitos vive en `progreso()` y el parser (`curvaDesdeTexto`), no en ellas. Con entradas no finitas el resultado no esta definido por contrato (documentarlo en la cabecera).
- **D5**: resuelta por compatibilidad con `Camera` (ver E06).

Pendiente de Oriol:
- **D6 · Aprobacion visual/jugable de E08**: requiere observacion real; no se declara por test.

## 6. Cierre de E01

- Comandos usados: `grep -rnEi "lerp|easing|tween|smoothstep|interpol" src include`, lectura con `awk 'NR>=a&&NR<=b'` de cada tramo citado; `wc -l src/Render/Camera.cpp` = 109 lineas. Sin build ni tests (E documental).
- Hash de contexto: base `9d52ae8`.
- Dependencias de paquete (`U3-07.md`): U2-01; dependencia de E: PREP.E10; puerta TEMPS_TWEEN. **No verificadas en esta E; anotadas.**
- Registro CMake: `include(cmake/Interpolacion.cmake)` en `CMakeLists.txt` tras `:75` se ejecuta ANTES de crear `suite_gl_free` (`:1454`), por lo que `if(TARGET suite_gl_free)` seria falso alli; los tests entran en la suite por la propiedad global `MOTOR_GL_FREE_TARGETS` (`CMakeLists.txt:1452`), que el `.cmake` debe rellenar con `set_property(GLOBAL APPEND ...)`.
