# Plan de estabilización — normas, forma canónica y tareas repartibles

**21/09/2026.** Encargo de Oriol: dejar el motor en un estado en el que varios
agentes puedan ayudar sin pisarse. Todas las cifras de este documento están
medidas hoy sobre el árbol, no estimadas.

| Hoy | |
| --- | ---: |
| Ficheros de core (`src/`, `include/`, `editor/`) | 234 |
| Líneas | 45.665 |
| Ficheros por encima de **500 líneas** | **17** (suman 17.150) |
| Cabeceras con más de 15 miembros propios | **9** |
| Miembros de `include/Engine/Application.h` | **104** |
| Sistema de logs | **no existe** |
| `Result<T>` (el canal de error que ya hay) | 1.161 usos |
| `throw` / `try` / `catch` | 37 / 16 / 10 |
| Fuentes de `src/` compiladas sueltas dentro de un objetivo | 67 |
| Ficheros que no alcanza ningún ejecutable | 42 (9.251 líneas) |

---

## 1. Las normas

Las cinco primeras ya son **ley** (`AGENTS.md`) y las comprueba
`tools/linea_base_core.py` en `pre-commit`, `pre-push` y CI. Las tres últimas
entran hoy.

**N1 — 500 líneas por fichero.** Nadie va a discutir por 520; lo que no se
discute es que **la lista solo baja**. Hoy la pasan 17 ficheros.

**N2 — Ninguna estructura mixta.** Para cada lógica: una clase de **datos**,
un **controlador** de su flujo, y varias clases que se reparten el trabajo. Si
el flujo es enorme, se parte en fases lógicas. La forma exacta, en §2.

**N3 — El estado vive en las clases de datos.** Un controlador no guarda lo
que manipula: lo recibe. Un miembro solo se justifica si es un **colaborador**
(otro sistema al que llama) o si el ciclo de vida obliga — y entonces lleva su
justificación escrita al lado. `Application.h` declara 104 miembros: las 2.472
líneas son el síntoma, esto es la enfermedad.

**N4 — Errores: `Result<T>` manda; las excepciones no cruzan fronteras.**
Aquí hay una corrección honesta a la consigna: **C++ no tiene `finally`**. El
`finally` de C++ es RAII — un destructor o un `ScopeGuard`—, y ese es el
mecanismo que vamos a usar. Además el repo ya tiene un canal de error
coherente, `Result<T>`, en 1.161 sitios: meter excepciones por en medio daría
dos caminos de error para la misma cosa, que es peor que tener uno.

La regla queda así:
- Dentro de un módulo, un fallo se **devuelve** como `Result<T>` con motivo.
- `try/catch` solo en las **fronteras donde de verdad llegan excepciones**:
  parseo (`std::sto*`, JSON), ficheros, `std::bad_alloc`, y las librerías de
  terceros. Ahí se atrapa y se convierte en `Result` + log.
- **Ninguna excepción cruza la frontera de un módulo.** Es comprobable.
- Lo que hoy sería un `finally` es un `ScopeGuard` (cerrar el fichero, soltar
  el recurso GL, restaurar el modo). Se crea en la tarea EST-11.

**N5 — Logs que cuentan una historia.** No hay ninguno. No se trata de
registrarlo todo: se registra **lo que el usuario hace y lo que el motor
decide**, de modo que leyendo el log se siga la partida. Diseño en EST-10.

**N6 — Cada fichero abre diciendo por qué existe.** Ya es la costumbre de la
casa y es lo mejor que tiene este repo; 8 de cada 40 ficheros de `src/` no la
siguen. Nombres reconocibles: el fichero se llama como su clase, y la clase
dice lo que es (`...Data`, `...Controller`, `...Validator`, `...Writer`).

**N7 — Una prueba GL-free por pieza.** Si para probar una pieza hace falta
abrir una ventana, la pieza está mal cortada. Es la prueba de que N2 se
cumplió de verdad.

**N8 — Eficiencia donde se nota.** Paso por `const&` en lo que no se copia a
propósito, `reserve()` antes de un bucle que llena, catálogos cargados una vez
y compartidos por referencia. Y el que más cuesta hoy: **67 compilaciones
duplicadas de 36 fuentes** (6.870 líneas recompiladas) porque las librerías no
se pueden enlazar.

---

## 2. La forma canónica

Para una lógica llamada `X`:

```
include/<Area>/XData.h        // DATOS. Campos + validate(). Sin flujo.
include/<Area>/XController.h  // FLUJO. Recibe XData&. Miembros: solo colaboradores.
include/<Area>/XPaso.h        // LOGICA. Piezas pequeñas, idealmente funciones puras.
tests/<Area>/XTests.cpp       // una prueba GL-free por pieza
cmake/XTests.cmake            // su registro, en el mismo commit
```

- **`XData`** guarda y se valida a sí misma. No llama a nadie. Se puede
  construir en una prueba en tres líneas.
- **`XController`** dirige: decide el orden, atrapa lo que venga de fuera, lo
  convierte en `Result`, y **escribe el log de las transiciones**. Su estado es
  solo *a quién llama*, nunca *qué datos tiene*.
- **Las piezas** hacen el trabajo y no saben que existe el controlador.
  Idealmente reciben lo que necesitan y devuelven lo que producen.

El ejemplo de referencia es el relevo de ayer:
`SerializerHelpers` (piezas) + `ProjectSerializer` / `MapSerializer` /
`MapDeserializer` (flujos separados) — 1.081 líneas en cuatro ficheros, 47
pruebas verdes y el formato byte a byte idéntico.

---

## 3. Las tareas

Están en `TABLERO_ESTABILIZACION.md` para reclamar. Aquí el orden y el porqué.

### Fase 0 — Desbloquear (serial, primero, media jornada)

| ID | Tarea | Cierre |
| --- | --- | --- |
| **EST-01** | `git add` de los 5 `cmake/*.cmake` sin versionar y commit del WIP de las otras sesiones | `python3 tools/linea_base_core.py --rapido` en verde. **HEAD vuelve a configurar** |
| **EST-02** | Regenerar `planificacion/alcance_real.json` y re-congelar la línea base | La guardia deja de avisar de foto vieja |

Sin EST-01 no hay CI, y sin CI nada de lo demás se puede cerrar de verdad.

### Fase 1 — Los cimientos que van a usar todos (serial, 3 j)

| ID | Tarea | Qué es | j |
| --- | --- | --- | ---: |
| **EST-10** | **Sistema de logs** `Core/Log` | `LogRecord` (datos) + `Logger` (controlador) + *sinks* (consola, fichero, memoria para pruebas). Niveles `error/warn/info/trace`, **categorías** (`nivel`, `combate`, `guardado`, `editor`, `contenido`), y una **traza de sesión** con id. Se registra lo que el usuario percibe: entrar a un nivel, abrir tienda, guardar, morir, cargar un catálogo, no encontrar un asset. No se registra cada función | 2 |
| **EST-11** | **Frontera de errores** | `Core/Errors/ScopeGuard.h` (el `finally` de C++), una guía corta de 20 líneas en `AGENT_HANDOFF.md`, y envolver los puntos reales: `std::sto*` sin proteger, apertura de ficheros, parseo JSON, `tinyxml2`. Cada captura produce `Result` + log con categoría | 1 |

Estas dos van primero porque **toda tarea de la fase 2 tiene que dejar logs y
errores según esta norma**. Si se hacen después, hay que repasar los 17
ficheros otra vez.

### Fase 2 — Troceado, en paralelo (un fichero, un agente, una sesión)

Cada fila es independiente de las demás: **se pueden coger a la vez**. Cada una
cierra con: el fichero por debajo de 500, la forma canónica de §2, logs en las
transiciones, errores por `Result`, prueba GL-free nueva que cae si se deshace
el cambio, y la guardia en verde.

| ID | Fichero | Hoy | Corte propuesto | j |
| --- | --- | ---: | --- | ---: |
| **EST-20** | `src/Engine/Application.cpp` + `.h` | 2472 / **104 miembros** | Los 7 relevos de `METODOLOGIA_RELEVOS.md` §4. **Depende de EST-01** | 12 |
| **EST-21** | `editor/src/level_editor.cpp` | 2777 | Es el editor antiguo: decidir primero **si se retira** en vez de partirlo | 1 + ? |
| **EST-22** | `include/RPG/Catalogs/RpgCatalogs.h` | 1862 | Una cabecera por familia de catálogo (habilidades, condiciones, monstruos, equipo, contenido) | 3 |
| **EST-23** | `src/Game/GameSession.cpp` + `.h` | 1173 / 40 miembros | `SessionData` (datos) + `GameSession` (flujo) + piezas: economía, moral, transiciones | 4 |
| **EST-24** | `src/Editor/EditorState.cpp` + `.h` | 1115 / 23 | `EditorDocumentData` + controlador + herramientas | 3 |
| **EST-25** | `src/RPG/NarrativeEngine.cpp` + `.h` | 744 / 543 | Beats (datos) + intérprete (flujo) + evaluación de condiciones | 2,5 |
| **EST-26** | `editor/src/EditorApplication.cpp` | 738 | Paneles a piezas; la ventana se queda | 2 |
| **EST-27** | `src/Map/MapDocument.cpp` | 724 | Capas / entidades / regiones | 2 |
| **EST-28** | `src/Project/ProjectIndex.cpp` | 683 | Índice (datos) + operaciones (alta, baja, duplicar) | 2 |
| **EST-29** | `src/Game/SaveGame.cpp` | 661 | Escribir / leer / migrar versiones | 2 |
| **EST-30** | `include/RPG/Definitions/RpgCoreDefinitions.h` | 655 | Es clase de datos: basta con partir por familia, sin controlador | 1,5 |
| **EST-31** | `src/Editor/ProblemsState.cpp` | 643 | Problemas (datos) + detectores por tipo | 2 |
| **EST-32** | `src/RPG/EventCommands.cpp` | 585 | Un fichero por familia de comando | 2 |
| **EST-33** | `src/RPG/AdventureValidation.cpp` | 518 | Reglas de validación, una por pieza | 1,5 |
| **EST-34** | `src/Editor/ObjectInspector.cpp` | 515 / 18 | Campos (datos) + edición | 1,5 |
| **EST-35** | `include/Render/IsometricRenderer.h` | 27 miembros | Sacar los datos de cámara/capas a una clase de datos | 2 |
| **EST-36** | `HudWidgets.h` 20 · `EnemyBrain.h` 19 · `HudTextWidgets.h` 18 · `Player.h` 17 | estado | Mismo tratamiento, son pequeñas | 2 |

**Subtotal fase 2: ~46 jornadas, y 16 de las 17 se pueden hacer a la vez.**

### Fase 3 — Estructura (después de trocear lo que toque)

| ID | Tarea | j |
| --- | --- | ---: |
| **EST-40** | Partir `motor_core` en `motor_hud` + `motor_entidades` (12 ficheros GL-free rehenes, 959 líneas). Solo CMake | 0,5 |
| **EST-41** | Matar las 67 compilaciones sueltas: cada prueba enlaza una librería | 2 |
| **EST-42** | Los 42 ficheros muertos: uno a uno, se cablea o se retira | 4 |

### Fase 4 — Que no vuelva (continuo)

| ID | Tarea | j |
| --- | --- | ---: |
| **EST-50** | La guardia detecta estructura mixta: un `.cpp` que define datos **y** dirige flujo | 1,5 |
| **EST-51** | Cobertura de logs del camino del jugador: título → partida → nivel → combate → tienda → guardar → morir, todo con su rastro | 1 |
| **EST-52** | Una prueba que falle si una excepción cruza la frontera de un módulo | 1 |

---

## 4. Lo que yo añadiría a tus consignas

1. **Un fichero se parte por trabajo, no por tamaño.** Cortar 1.000 líneas en
   dos de 500 no arregla nada si las dos mitades siguen cambiando juntas.
2. **Mover y reescribir nunca en el mismo commit.** Es lo que hace que un
   troceado se pueda revisar en dos minutos en vez de en dos horas.
3. **El log es parte del criterio de cierre, no un extra.** Una tarea de la
   fase 2 sin logs en sus transiciones no está cerrada.
4. **Ningún cambio de formato sin su fixture.** Ya existe el mecanismo
   (`SchemaFixtureTests`, y comprueba byte a byte); solo hay que exigirlo.
5. **La guardia se actualiza a la baja y se commitea con el trabajo.** La línea
   base no es un trámite: es el recibo de lo que hiciste.
