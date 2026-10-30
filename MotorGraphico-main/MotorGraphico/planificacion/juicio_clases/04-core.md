# 04 · Core — juicio clase a clase

**Ámbito.** Las 28 cabeceras y 39 clases de `include/Core/` que alcanza algún
ejecutable, más las 2 que no alcanza ninguno.

**Una nota sobre el recuento.** El mapa del juego cuenta 19 cabeceras y 32 clases
en `Core`; aquí son 28 y 39 porque el juicio cubre la **unión** de los tres
ejecutables, y nueve cabeceras de Core solo las alcanzan los editores. Las nueve
son del módulo de log y de `ScopeGuard`, y esa es la primera conclusión del área.

---

## Los cinco hallazgos del área

### 1. El juego no tiene log

Las nueve cabeceras de `Core/Log` (`Logger`, `LogRecord`, `LogConfig`, `ILogSink`,
`FileSink`, `ConsoleSink`, `LogFormat`, `MemorySink`, más `ScopeGuard`) llegan
**solo a `motor_editor`**, y todas por el mismo sitio: `editor/src/main.cpp`. Ni
`juego` ni `level_editor` las alcanzan.

El módulo está bien construido y se nota: separación controlador/datos/pieza con la
norma N3 citada en los comentarios, `sequence` monotona que «delata entradas
perdidas si el fichero de log aparece con saltos», sinks que devuelven `Result` y no
lanzan porque «un fallo de log no puede tumbar una partida», y la contención de
excepciones de EST-52 (`3a6d5dc`). Y no lo usa el jugador: cuando algo falla en una
partida, no hay registro.

Dentro del propio editor el uso es parcial: **ocho métodos de `Logger` sin llamar**
(`beginSession`, `setConfig`, `trace`, `warn`, `sinkCount`, `lastSequence`,
`filteredOut`, `droppedBySink`) y **cinco de `LogConfig`** (`enable`, `disable`,
`enableAll`, `minLevel`, `setMinLevel`). O sea que la configuración del log no la
cambia nadie: funciona con los valores por defecto (nivel Info, todas las
categorías, Trace fuera a propósito por la norma N5). `droppedBySink()` —el contador
que EST-52 añadió para que un sink roto no dejara el log ciego en silencio— no lo
consulta nadie.

### 2. `ResourceManager` tiene cero pruebas y es el dueño de todos los recursos GL

90 líneas, 62 de cuerpo, la plantilla de la que derivan `TextureManager` y
`ShaderManager`, y **0 pruebas**. Es la pieza que captura las excepciones de
`loadFromDisk()` y las convierte en `Result<T*>`, o sea la que hace que «el código de
aplicación nunca necesite un try/catch por sí mismo» — esa garantía no la verifica
ningún caso. `unload` sin llamar en los tres ejecutables y `contains` sin llamar en
los dos editores.

Sus dos derivados sí están probados (`ShaderManager` 11, `TextureManager` 8), que es
probablemente por lo que pasó desapercibido: las pruebas ejercitan la plantilla a
través de las especializaciones, nunca la plantilla.

### 3. `HandlePool` no llega a ningún ejecutable, y `EntityHandle` sí

`include/Core/Memory/HandlePool.h`: 179 líneas, 2 pruebas, **inalcanzable**. Es la
pieza genérica de handles generacionales (índice + generación) que el análisis del
31/08 listaba entre las fundaciones sólidas. Y en `include/World/EntityHandle.h` hay
13 líneas que hacen exactamente eso, con el mismo motivo escrito («la generación
impide que un handle de una entidad destruida apunte por accidente a otra que
reutilizó su slot»), y esas sí llegan al juego.

Dos implementaciones del mismo concepto, la grande muerta y la pequeña viva. Es el
patrón que `revisar-antes-de-crear` describe: la pieza ya existía y se hizo otra.

### 4. `JsonWriter` tiene una prueba para un criterio que habla de todo lo que escribe

C-10 dice: «tot el que surt de `JsonWriter` torna a entrar per `JsonValue::parse`
sense pèrdua». 178 líneas, 145 de cuerpo, **1 prueba**. Y `valueNull` sin llamar en
los tres ejecutables: el escritor sabe emitir `null`, nadie lo emite, y por tanto el
viaje de ida y vuelta del `null` no se ha ejercitado nunca.

Es el criterio más falsable del árbol —«todo lo que sale vuelve a entrar»— y el que
menos cubierto está. Un caso de round-trip por tipo de valor (null, bool, entero,
flotante, cadena con escapes, array, objeto anidado) es exactamente lo que C-10
pide y son siete casos.

### 5. Dos excepciones llevan su diagnóstico y nadie lo lee

`MapParseException::m_lineNumber` y `ShaderCompileException::compileLog` (+ su
accessor `compileLog()`) sin llamar en los tres ejecutables. O sea: cuando falla el
parseo de un mapa, la excepción sabe la línea y nadie la muestra; cuando falla un
shader, la excepción trae el log del compilador GLSL y nadie lo imprime.

No es un detalle menor porque el reparto está bien pensado y escrito: las excepciones
son para errores de arranque y fallos fatales, y «los fallos esperables de carga usan
`Result<T>`». El problema es que el diagnóstico que justifica tener la excepción se
queda dentro de ella.

---

## `include/Core/Json/JsonWriter.h` — 178 líneas · los tres · header-only · **1 prueba**

### `JsonWriter` (línea 34, 145 de cuerpo)

- **Qué hace.** El escritor JSON mínimo, contrapartida de `JsonValue` (que solo lee).
  Existe porque hasta RUN-10 el motor solo consumía JSON y guardar una partida obligó
  por primera vez a producirlo. Mantiene el criterio de `JsonValue`: parser propio,
  cero dependencias vendorizadas.
- **Qué debería hacer.** C-10, con su otra mitad: «el parser entén `\uXXXX` i el
  codifica a UTF-8».
- **Qué se lo impide.** Una prueba para 145 líneas (hallazgo 4). Los enteros se
  escriben como `int` —que es exacto— y el resto como `float`, que es lo que el
  escritor ofrece: una limitación declarada en el código, y justo la clase de cosa
  que un round-trip destaparía si un día hace falta un `double`.
- **Cómo optimizarla.** Los siete casos de round-trip de C-10. Es la inversión con
  mejor retorno del área: protege el guardado, la exportación del editor y los
  documentos de proyecto a la vez, porque los tres pasan por aquí.

---

## `include/Core/Json/JsonValue.h` — 121 líneas · los tres · 7 pruebas · **sin paquete**

### `JsonValue` (línea 23, 99 de cuerpo)

- **Qué hace.** El valor JSON mínimo con parser propio: el motor lo necesita para los
  niveles y no hay ninguna librería vendorizada (a diferencia de TMX, que usa
  `tinyxml2`). Su cabecera deja escrita la alternativa: vendorizar `nlohmann/json` si
  el esquema crece mucho.
- **Qué debería hacer.** C-10 la nombra literalmente («torna a entrar per
  `JsonValue::parse` sense pèrdua») y **no lleva el bloque**. Es el mismo defecto de
  mapa que en `SkillDefinition.h`: el criterio existe y la cabecera no lo dice, así
  que quien la toca no se entera.
- **Qué se lo impide.** Nada estructural. 7 pruebas para 99 líneas es aceptable en un
  parser, aunque C-10 pide el round-trip y eso es cosa de las dos cabeceras juntas.
  Esta semana ganó `operator[](std::size_t)` no constante (`f6e1893`) porque desde
  E-16 escribir `v[0]` sobre un `JsonValue` mutable era ambiguo: la trampa era del
  API, no del test, y se arregló donde tocaba.
- **Cómo optimizarla.** `marcar_paquetes.py --marcar` para que C-10 aparezca aquí. Y
  la decisión de fondo —parser propio contra `nlohmann/json`— ya está escrita con su
  condición de cambio («si el esquema crece mucho»), que es como deben estar estas
  decisiones.

---

## `include/Core/Errors/Result.h` — 98 líneas · los tres · header-only · **29 pruebas**

### `Result<T>` (línea 23, 46 de cuerpo) y `Result<void>` (línea 71, 28 de cuerpo)

- **Qué hacen.** El tipo de error del motor: o valor o error, sin excepciones a través
  de las fronteras. La especialización para `void` existe porque un `Result<T>` con
  `T=void` no compila igual, y la norma N4 pide devolver el fallo en vez de tirarlo
  hacia arriba.
- **Qué deberían hacer.** Sin paquete, aunque N4 es una norma del repo y no un paquete
  del plan. **Propuesta:** C-22, que es el paquete de higiene de API, o un paquete
  propio de fundaciones.
- **Qué se lo impide.** Nada. 29 pruebas para 98 líneas: la segunda mejor ratio del
  área.
- **Cómo optimizarla.** Nada. Es la pieza que hace posible que `FileSink` se abra con
  una fábrica, que `ResourceManager::load()` convierta excepciones, y que `ILogSink`
  no lance. Todo el diseño de errores del motor descansa aquí y está bien probado.

---

## `include/Core/Log/` — el módulo de log, siete cabeceras y `ScopeGuard`

Todas llegan solo a `motor_editor`. El diseño interno es el más documentado del área:
cada cabecera abre con `CONTROLADOR:` / `DATOS:` / `PIEZA:` y cita la norma del repo
que cumple (N3, N4, N5) o la sección de `PLAN_ESTABILIZACION.md`. Esa disciplina es
la razón de que el módulo se entienda de un vistazo, y conviene no perderla.

| Cabecera | Clase | Líneas de cuerpo | Pruebas | Juicio |
| --- | --- | ---: | ---: | --- |
| `Logger.h` (94) | `Logger` | 54 | 2 | **CONTROLADOR.** Decide qué se registra, sella la entrada (nivel, categoría, sesión, secuencia, tiempo) y la reparte entre sinks. No guarda el log y no formatea. Ocho métodos sin llamar; `writeSafely`/`flushSafely` contienen las excepciones de un sink y `droppedBySink()` las cuenta, que nadie consulte el contador no invalida el arreglo |
| `LogRecord.h` (90) | `LogRecord` | 8 | 2 | **DATOS.** Una entrada ya clasificada y sellada; «la rellena `Logger` y la consume un sink, nadie más la construye en el motor». `sequence` monotona con su doble motivo escrito. Bien |
| `LogConfig.h` (50) | `LogConfig` | 30 | 2 | **DATOS.** Se valida a sí misma, «que es lo que la forma canónica pide de una clase de datos». Cinco métodos sin llamar: la configuración no la cambia nadie (hallazgo 1) |
| `ILogSink.h` (29) | `ILogSink` | 9 | 1 | **PIEZA.** El contrato del destino: no filtra ni decide, devuelve `Result<void>` y no lanza. Es la interfaz cuyo incumplimiento motivó EST-52 |
| `FileSink.h` (42) | `FileSink` | 19 | 1 | **PIEZA.** Se abre con fábrica que devuelve `Result` porque abrir un fichero es frontera de fallo esperable y «un constructor no puede devolver nada». La justificación de N4 está escrita donde se lee |
| `ConsoleSink.h` (28) | `ConsoleSink` | 11 | 1 | **PIEZA.** Sink a `std::ostream` con destino inyectable (por defecto `cerr`), para que una prueba compruebe la línea exacta sin capturar la salida del proceso. El constructor por defecto vive en el `.cpp` para no arrastrar `<iostream>`: decisión de compilación correcta y escrita |
| `LogFormat.h` (24) | — | — | 2 | `formatRecord`, función libre. Sin clases |
| `MemorySink.h` (48) | `MemorySink` | — | 2 | **No la alcanza ningún ejecutable y es correcto:** es el doble de pruebas, y vive en `include/` con la decisión documentada en `802153b`. Es el único «inalcanzable» del área que no es un defecto |

### `ScopeGuard` (`include/Core/Errors/ScopeGuard.h`, línea 28, 26 de cuerpo)

- **Qué hace.** Ejecución garantizada al salir del ámbito, con `makeScopeGuard`.
- **Qué debería hacer.** Sin paquete. **Propuesta:** ninguno — es utilería de las
  fundaciones.
- **Qué se lo impide.** Un solo consumidor en todo el árbol (`FileSink.cpp`) y una
  prueba. Llegar solo a `motor_editor` es consecuencia de eso.
- **Cómo optimizarla.** Nada. Es RAII genérico bien puesto; lo que dice el alcance es
  que el motor casi no lo necesita, y eso es buena señal.

---

## `include/Core/Resources/` — cinco cabeceras

### `ResourceManager<T>` (`ResourceManager.h`, línea 29, 62 de cuerpo) · **0 pruebas**

- **Qué hace.** El almacén RAII genérico de recursos con nombre: carga, retiene y
  libera. Captura las excepciones de `loadFromDisk()` y las devuelve como
  `Result<T*>`.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-09, que es el criterio de
  «propietat de recursos i destrucció polimòrfica» y habla justo de esto.
- **Qué se lo impide.** Cero pruebas (hallazgo 2) y `unload`/`contains` sin
  consumidor. Que `unload` no se llame en los tres ejecutables es coherente con RAII
  —se libera al destruir el manager—, pero entonces `unload` público es API muerta.
- **Cómo optimizarla.** Tres casos: carga dos veces el mismo nombre y devuelve el
  mismo recurso; un `loadFromDisk()` que lanza devuelve `Result::Error` y no propaga;
  destruir el manager libera todo. Son los tres comportamientos de los que depende el
  resto del motor.

### `Texture` (`Texture.h`, línea 17, 37 de cuerpo) · 13 pruebas · C-09

- **Qué hace.** Wrapper RAII sobre una textura GL. «No posee el nombre GL hasta que se
  construye correctamente: no hay estados a medio construir».
- **Qué debería hacer.** C-09: «l'assignació per moviment allibera el recurs de destí
  abans de robar el d'origen».
- **Qué se lo impide.** Nada. 13 pruebas para 37 líneas de cuerpo, y el criterio de
  C-09 es exactamente lo que hay que probar en un RAII con asignación por movimiento.
- **Cómo optimizarla.** Nada. Es el modelo del área: pequeña, propietaria, con el
  invariante escrito y probado.

### `TextureAtlas` (`TextureAtlas.h`, línea 13, 28 de cuerpo) · 9 pruebas

- **Qué hace.** Recorta subregiones (columna, fila) de un atlas y devuelve el `UVRect`
  para `SpriteBatch::submit()`. No posee la textura: el ciclo de vida lo gestiona
  `TextureManager`, «igual que en el resto del motor».
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-09.
- **Qué se lo impide.** `getUVInset` sin llamar desde `motor_editor`.
- **Cómo optimizarla.** Nada. La convención de no poseer es consistente en todo el
  motor y está escrita en cada sitio, que es lo que evita que alguien la rompa.

### `Shader` (`Shader.h`, línea 15, 19 de cuerpo) · 2 pruebas

- **Qué hace.** Compila y enlaza un programa GLSL. El constructor lanza
  `ShaderCompileException` si falla, y `ResourceManager<Shader>::load()` la convierte
  en `Result<Shader*>::Error`.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-09.
- **Qué se lo impide.** `setUniformVec2` sin llamar en los tres, y `setUniformFloat` y
  `setUniformVec3` tampoco desde `level_editor`. Y el `compileLog` de su excepción
  nadie lo lee (hallazgo 5).
- **Cómo optimizarla.** Dos pruebas para una clase que habla con el driver GL es poco,
  pero es de las que no se pueden probar sin contexto. Lo que sí se puede y no se
  hace: que cuando falle, el log del compilador llegue a algún sitio.

### `ICatalog` (`ICatalog.h`, línea 29, 11 de cuerpo) · **0 pruebas**

- **Qué hace.** La interfaz de catálogo que `Level/ObjectCatalog.h` implementa.
- **Qué debería hacer.** Sin paquete. **Propuesta:** el paquete de catálogo de objetos
  del editor.
- **Qué se lo impide.** Cero pruebas y una sola implementación conocida. Una interfaz
  con un único implementador es una abstracción que todavía no se ha ganado el coste.
- **Cómo optimizarla.** Si no va a haber un segundo catálogo, la interfaz sobra y
  `ObjectCatalog` puede ir directamente. Si va a haberlo —el editor de proyecto y el
  de nivel tienen catálogos distintos— entonces falta la prueba que fije el contrato.

### `ShaderManager` (11 líneas, 4 de cuerpo, 11 pruebas) y `TextureManager` (11, 4, 8)

Dos especializaciones de `ResourceManager<T>` de cuatro líneas de cuerpo cada una, y
las dos mejor probadas que la plantilla de la que salen. `ShaderManager` resuelve un
prefijo a `.vert`/`.frag`; `TextureManager` sube el PNG con `GL_NEAREST`, que es el
requisito del pixel art nítido, y lo dice. **Juicio:** correctas y mínimas. La
asimetría de pruebas con su base es el hallazgo 2.

---

## `include/Core/Ids/` — identidad fuerte

### `StrongId<Tag>` (`StrongId.h`, línea 30, 31 de cuerpo)

- **Qué hace.** Identificador fuertemente tipado por etiqueta: un `MapId` no se puede
  pasar donde se espera un `LayerId`, y el compilador lo impide.
- **Qué debería hacer.** Sin paquete. **Propuesta:** G-07, porque los ids son parte del
  esquema congelado.
- **Qué se lo impide.** Nada. Una prueba propia, pero 25 en `ProjectIds.h`, que es
  donde se ejercita de verdad.
- **Cómo optimizarla.** Nada. Es de las decisiones que pagan solas: cuesta 31 líneas y
  elimina una familia entera de bugs de parámetros intercambiados.

### `IdGenerator` (`StrongId.h`, línea 65, 5 de cuerpo)

- **Qué hace.** Fuente inyectable de ids: «los tests pueden generar una secuencia fija
  y las herramientas de autoría pueden decidir su estrategia sin acoplar los
  documentos a UUID, reloj del sistema o aleatoriedad global».
- **Qué debería hacer.** Sin paquete. **Propuesta:** G-07.
- **Qué se lo impide.** Nada. Cinco líneas con tres motivos escritos.
- **Cómo optimizarla.** Nada.

### `ProjectIds.h` — ocho etiquetas, 41 líneas, **25 pruebas**

`ProjectIdTag`, `DocumentIdTag`, `MapIdTag`, `LayerIdTag`, `AssetIdTag`,
`EntityIdTag`, `RegionIdTag`, `EventIdTag`: tres líneas cada una, 25 pruebas en
conjunto. **Juicio:** es la declaración del vocabulario de identidades del motor y
está bien donde está. Lo que conviene comprobar es que las ocho se usen: si
`RegionIdTag` o `AssetIdTag` no tienen consumidor, son vocabulario previsto y no
vivo, que es distinto de estar muerto.

*(El dossier cuenta además un `hash` en la línea 81: es la especialización de
`std::hash`, no una clase. Artefacto del contador, anotado para que no se repita.)*

---

## `include/Core/Errors/EngineException.h` — 58 líneas · los tres · 2 pruebas

### `EngineException` (línea 15, 8 de cuerpo)

- **Qué hace.** La excepción base del motor, reservada para errores de inicialización
  que impiden arrancar y fallos fatales e irrecuperables durante el render.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-22, que es donde se decide qué
  API vive.
- **Qué se lo impide.** Nada. El reparto con `Result<T>` está escrito y es el correcto:
  «los fallos esperables de carga de recursos usan `Result<T>` en su lugar».
- **Cómo optimizarla.** Nada.

### `ResourceLoadException` (24, 5) · `RenderException` (42, 5)

Dos excepciones de cinco líneas, cada una con su caso de uso en el reparto de arriba.
**Juicio:** correctas, mínimas, sin nada que optimizar.

### `ShaderCompileException` (30, 11 de cuerpo) y `MapParseException` (48, 11 de cuerpo)

- **Qué hacen.** La primera lleva el log del compilador GLSL; la segunda, el número de
  línea del fallo de parseo.
- **Qué deberían hacer.** Que su diagnóstico llegue a quien lo puede arreglar: el autor
  del mapa, o quien escribe el shader.
- **Qué se lo impide.** **Nadie lee `compileLog` ni `m_lineNumber`** en los tres
  ejecutables (hallazgo 5). Son las dos excepciones del motor que cargan información
  útil y la tiran.
- **Cómo optimizarla.** Imprimir el payload en el sitio que captura la excepción. Es
  una línea cada una y convierte un fallo mudo («no carga el mapa») en uno actionable
  («no carga el mapa, línea 214»). `MapParseException` además tiene a `TmxMapImporter`
  al lado, cuya cabecera promete que «las funciones no soportadas nunca se pierden
  silenciosamente: entran en el informe como warning o error» — la misma filosofía,
  aplicada a las excepciones.

---

## `include/Core/Math/` — seis cabeceras, las mejores ratios del árbol

| Cabecera | Líneas | Pruebas | Qué hace | Juicio |
| --- | ---: | ---: | --- | --- |
| `GridCoord.h` | 9 | **63** | Coordenada entera del grid isométrico, distinta de `Vector2` | La ratio más alta del árbol: 63 pruebas para 4 líneas de cuerpo. Que dos tipos de coordenada no se confundan es el invariante más barato y más rentable del motor, y está blindado |
| `Vector2.h` | 12 | 20 | Vector 2D de posición continua mundo/pantalla | POD con defaults. Nada que tocar |
| `Vector4.h` | 16 | 8 | Color/tint RGBA para `SpriteBatch` | El default `{1,1,1,1}` (blanco opaco, sin cambio) está escrito como motivo. Correcto |
| `GridBounds.h` | 15 | 5 | Rango de celdas inclusive para el culling de `TileMap` | Su comentario apunta a `TileMap::visibleRange()`, que es justo el método que **no llama ningún ejecutable** (ver `01-render.md`). La pieza matemática está probada y su consumidor no existe |
| `UVRect.h` | 11 | 3 | Región UV dentro de un atlas | Correcta |
| `IsoMath.h` | 41 | 2 | `gridToScreen`/`screenToGrid`, funciones libres | **Dos pruebas para la proyección isométrica entera**, y la usan los tres ejecutables. Es la ratio más baja del área y la conversión con más riesgo de errores de signo. Además `Camera::worldToGrid` es una tercera vía para la misma conversión y no la llama nadie |

**Juicio conjunto.** El área matemática es la más sana del motor: tipos pequeños, POD,
header-only, con el motivo de cada default escrito y ratios de prueba altísimas. Las
dos excepciones son `IsoMath` (2 pruebas para la proyección que sostiene toda la vista
isométrica) y la duplicación con `Camera::worldToGrid`. Ninguna de las seis lleva
bloque de paquete; **propuesta:** C-19 para `IsoMath` y `GridBounds`, que son las dos
que participan del culling y del contrato de coordenadas por frame.

---

## `include/Core/Check.h` — 49 líneas · solo `level_editor` · **45 pruebas**

- **Qué hace.** `require()` y compañía: la comprobación que sí funciona en Release, a
  diferencia de `assert()`.
- **Qué debería hacer.** Sin paquete, pero es la pieza que hace posible la regla del
  workspace entero («`require()` y nunca `assert()` en demos, porque Release define
  `NDEBUG`»). **Propuesta:** que se le asigne explícitamente, porque una regla de todo
  el proyecto descansa sobre esta cabecera y no tiene criterio.
- **Qué se lo impide.** Que solo la alcance `level_editor`. Los demos y los demás tests
  la usan, pero no cuentan como ejecutables reales, así que el mapa la enseña más
  pequeña de lo que es.
- **Cómo optimizarla.** 45 pruebas para 49 líneas es la segunda ratio más alta del
  árbol y es correcta: es la pieza que decide si una comprobación existe en la build
  que se publica. Nada que cambiar.

---

## Las dos cabeceras de Core que no alcanza ningún ejecutable

| Cabecera | Líneas | Pruebas | Veredicto |
| --- | ---: | ---: | --- |
| `Memory/HandlePool.h` | 179 | 2 | **Real y duplicado.** `World/EntityHandle.h` hace lo mismo en 13 líneas y sí llega al juego. Ver hallazgo 3 |
| `Log/MemorySink.h` | 48 | 2 | **Falso positivo.** Es el doble de pruebas del log, vive en `include/` con la decisión documentada en `802153b`. Correcto que no llegue a ningún binario |

`HandlePool` es el único caso del área donde hay código muerto de verdad: 179 líneas
de una fundación que el análisis del 31/08 daba por sólida y que hoy no usa nadie, con
un sustituto vivo de 13 líneas. Antes de borrarla conviene comprobar que `EntityHandle`
cubre lo mismo (pool generacional con reutilización de slots) y no solo la mitad —
`World` tiene `Slot` y `PendingCreate` propios, que podrían ser la otra mitad.
