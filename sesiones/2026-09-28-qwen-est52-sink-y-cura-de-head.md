# 2026-09-28 · qwen · EST-52 (hueco de Core/Log) + cura de dos rojos de HEAD

**Encargo de Oriol (28/09):** «¿puedes tomar tú la iniciativa empezada con Claude
de limpiar y depurar todo el proyecto, aparte de darle un mejor rendimiento y
estructura y legibilidad?», ampliado a «coge el Gantt del MotorGraphico y sigue
programando». Autorizó en la misma conversación: hacer EST-01 en commits
separados dejando el WIP ajeno intacto, la fase 1 entera, y arreglar el
enforzado de la ley.

## Lo primero que hay que saber de esta sesión

Empecé creyendo que estaba al principio del plan, y no: **la conversación
arrancó con una vista del repo del 21/09 y hoy es 28/09**. Cuando me di cuenta,
EST-10, EST-11, EST-51, EST-52 y EST-23b ya estaban cerrados por otras sesiones
y había ~40 commits por encima de los míos.

Consecuencia práctica que dejo anotada porque puede volver a pasar: escribí los
13 ficheros de `Core/Log` desde cero y **`git status` no dio ni un diff** — el
contenido era idéntico al ya commiteado en `5833967`. Cero daño, pero media
sesión podía haberse ido rehaciendo trabajo cerrado. Y el despiste vino de una
comprobación mal hecha: busqué si existía un sistema de logs con
`grep "class Logger\|namespace log\|..."`, y **el grep BSD de macOS no soporta
`\|` en BRE**, así que devolvió vacío y lo leí como «no existe». Antes de
afirmar que algo no existe en este repo, `grep -E` o una búsqueda por patrón
simple.

## Qué hice, en commits

Repo `MotorGraphico-main`, rama `feature/editor-s1-contratos`:

| Commit | Qué |
| --- | --- |
| `3a6d5dc` | **EST-52, el hueco que opencode me dejó a mí.** `Logger` ya contiene las excepciones de un sink |
| `03a3774` | **Cura de HEAD:** `ValidateAdventure` estaba definida dos veces; `test_adventure_validation` no enlazaba |
| `f6e1893` | **Cura de HEAD:** `v[0]` sobre un `JsonValue` mutable era ambiguo; `test_editor_state` no compilaba |

### `3a6d5dc` — ninguna excepción sale del módulo de log

opencode cerró EST-52 el 26/09 y dejó en el tablero, dirigido a este carril: un
`ILogSink` que lanza en vez de devolver `Result` incumple su contrato y
`Logger::emit` no lo contenía. Lo fijó con el caso
`un_sink_que_lanza_aun_se_sale_del_modulo` escrito **en rojo a propósito**, como
alarma, y con la instrucción de voltearlo el día que se contuviera.

`Logger` llama ahora a los sinks vía `writeSafely`/`flushSafely`, que atrapan
cualquier excepción y la devuelven como `Result`; `emit()` la cuenta en
`droppedBySink()`, así que no queda invisible. Dos ganancias que no eran
evidentes hasta medirlas:

- **Un sink roto ya no deja el log ciego.** Antes su excepción se llevaba el
  bucle por delante y los demás sinks no recibían la entrada — justo cuando hace
  falta leerlo.
- **El destructor ya no puede provocar `std::terminate`.** Vuelca los sinks; sin
  contención, una excepción saliendo de un destructor aborta el proceso.

Falsabilidad comprobada por mutación, no de palabra: quité el `catch` de `write`
y fallaron 4 casos; quité el de `flush` y `test_log` no falló, **abortó** (exit
134, SIGABRT). Restaurado: `test_log` 36/36 (4 casos nuevos) y
`test_boundary_exceptions` 8/8.

### `03a3774` — el duplicado que devolvió la sincronización

La suite entera no enlazaba: 2 *duplicate symbols* en las dos sobrecargas de
`RPG::ValidateAdventure`. Causa: EST-33 partió `AdventureValidation.cpp`
(518 → 285) moviendo los beats a `AdventureBeatValidation.cpp`, y después
`fea1ae9` («la sincronización en la nube revierte trabajo») devolvió el original
a su versión anterior al corte. El fichero nuevo sobrevivió; el viejo volvió con
sus funciones. **El bloque existía en los dos sitios.**

Antes de borrar nada comprobé que las líneas 295–518 del original eran
**idénticas byte a byte** a las 22–245 del nuevo (diff vacío). Corté por la 285,
que es donde el split lo dejaba: vuelve a 285 líneas, no se pierde ni se
reescribe nada. Verifiqué que los otros cuatro fragmentos que compilan esa
fuente suelta no llaman a `ValidateAdventure` (solo necesitan `ValidateEvents`),
compilando la suite entera. Recibo: `cuellos` 6 → 5 en la línea base, y **solo
esa partida** — las otras dos bajas que cantaba la guardia vienen del WIP ajeno
y son su recibo, no el mío.

### `f6e1893` — una trampa del API, no un arreglo del test

Desde que E-16 añadió `operator[](const std::string&)` **no constante**, escribir
`v[0]` sobre un `JsonValue` mutable no compila: el `0` literal es también
constante de puntero nulo, así que compiten la conversión a `std::string`
(definida por el usuario, sin penalización de constancia) y la de `size_t`
(integral, pero sobre la versión `const`). Ninguna domina. Con un `JsonValue`
const no pasaba, por eso tardó en verse.

Añadí la sobrecarga que faltaba, `operator[](std::size_t)` no constante. Dos
decisiones: devuelve referencia **const** (no añade capacidad de mutación; el
acceso mutable por índice no existe ni debe existir, la simetría con la versión
por clave sería falsa) y delega en la `const` (nada de `Null` estático mutable).
No arreglé el test: la trampa podía morder en cualquier encadenado con índice.

## Estado de la suite

`170/175` GL-free (venía de `169/175`). Los 5 fallos que quedan son de datos y
de WIP ajeno sin commitear, ninguno de compilación propia:
`demo_creacion_personaje` espera 61 clases contra un `assets/catalogs/classes.json`
modificado sin versionar, `demo_nova_partida` el inventario tras EST-23b, y
`test_gdb12_screens`, `test_catalog_content`, `test_project_database` del mismo
WIP. Guardia: **OK**, sin deuda nueva.

Quedan **3 objetivos que no compilan en HEAD**, y los tres están ya registrados
en `TABLERO.md:57` y en `planificacion/INCIDENCIA_SINCRONIZACION_2026-09-23.md`
bajo «Rojos ajenos, registrados y sin tocar (regla 6)»:

- `test_e09_bridge` — **es mi fila (E-09)** y no la cerré. `addStructuredCommand`
  existió solo en el árbol (81 inserciones sobre HEAD, anotado el 17/09) y se lo
  llevó la reversión: `git grep` en `ed84b18` solo encuentra el test, nunca la
  implementación. **No está en git, hay que reescribirla** a partir de las 5
  llamadas del test. No la empecé: reimplantar funcionalidad perdida sin poder
  terminarla y verificarla en esta sesión era peor que dejarla señalada.
- `test_animated_entity` — el constructor de `AnimatedEntity.h` cambió; la
  cabecera está sucia, es del dueño de G-09.
- `test_inventory_menu` — `GameSession::InventoryRules` tras EST-23b; opencode
  está hoy encima.

## Dos cosas que dejo señaladas y no hice

**1. La regla anti-duplicados de la guardia tiene un agujero.**
`comprueba_simbolos` solo casa `Clase::metodo` con clase en mayúscula
(`DEFINICION`, línea 304). `ValidateAdventure` es una **función libre dentro de
`namespace RPG`**, así que su definición no lleva calificador y la regla no la
ve: el fallo que motivó crearla volvió a pasar por otro sitio.

Prototipé la extensión y **la tiré, con datos**:
- Falso negativo: no cazó `ValidateAdventure` en la copia con el bug.
- Falso positivo: cantó `Editor::existe` duplicada cuando solo existe en
  `ProjectIndex.cpp`.

La causa de los dos es la misma y es de bulto: traté «línea que empieza por `}`»
como cierre de namespace, y en este repo las llaves de cierre de cada **función**
también caen en columna 0 (`ProjectIndexDuplicate.cpp` tiene `}` en las líneas
45, 54, 70, 82…). La pila se despista. Hace falta contar profundidad de llaves
de verdad, con sus pruebas. **No la metí a medias**: una regla con falsos
positivos bloquea los commits de todos los agentes, que es peor que el agujero.

**2. `alcance_real.json` sigue siendo la foto del 22/09** y el AVISO de foto
vieja es permanente. Regenerarla destapa más muertos de los que caben en la
línea base; subir el trinquete es decisión de Oriol y está en
`decisiones/D0007` (`Estado: proposed`).

## Lo que no toqué

`TABLERO_ESTABILIZACION.md` lleva el WIP de opencode documentando la regresión
de EST-23b; el commit `9029322` se hizo con el índice aislado (HEAD + solo mi
fila) y su hunk sigue intacto en el árbol, comprobado después. Ni
`CMakeLists.txt`, ni `Application.h`, ni `EditorApplication.cpp`: los tres
sucios y con gente encima (EST-20b, EST-26b y EST-40 chocan con eso hoy).

**Pendiente del protocolo, y lo digo en vez de dejarlo implícito:** no moví mi
fila en `TABLERO.md` ni regeneré `sesiones/INDICE.md`. Los dos ficheros están
sucios con trabajo de opencode (sus dos sesiones de hoy siguen sin versionar), y
commitearlos enteros me habría llevado su registro por delante. Este fichero sí
va versionado porque es nuevo y no pisa a nadie.

---

## Posdata (misma sesión, a petición de Oriol): la regla anti-duplicados, bien hecha

Oriol eligió esta entre las cuatro que le ofrecí. Commit `afc8856`.

Arriba dejé escrita la razón por la que **no** la metí a medias: el primer
prototipo daba un falso negativo y un falso positivo a la vez. La versión que sí
va tiene profundidad de llaves real y tres decisiones que son las que la hacen
usable:

- Un limpiador que retira comentarios de línea, de bloque y literales antes de
  contar `{` y `}`. Sin eso una llave dentro de un string descoloca el fichero
  entero.
- Una definición libre solo cuenta si está **exactamente un nivel** por dentro
  del namespace más interno. Eso excluye de golpe lo que hay dentro de un cuerpo
  de función y de un cuerpo de clase, que era la otra fuente de falsos positivos.
- Namespace anónimo no cuenta (vinculación interna: dos copias del mismo
  ayudante son legales, y fallar ahí bloquearía commits ajenos), y una línea que
  termina en `;` es una declaración, no una definición.

Verificación: 52 casos existentes siguen verdes, 9 nuevos → **61/61**; con la
regla vieja los casos nuevos revientan (falsabilidad); **cero falsos positivos**
en el árbol real; y sobre la copia de `AdventureValidation.cpp` anterior a
`03a3774` reporta `RPG::ValidateAdventure está definido en 2 fuentes`.

El susto de la sesión, y vale la pena dejarlo: el fichero **ya tenía** un
`_sin_comentarios(texto)` para los `#` de CMake. El mío se llamaba igual y, al
definirse después, el de CMake se lo comía — `TypeError` en la primera llamada.
Lo encontraron las pruebas, no la lectura. Va renombrado a
`_sin_comentarios_cpp`.

Limitación declarada en el código: no entiende *raw strings* `R"(...)"`. En este
repo no hay ninguna.

---

## Segunda posdata: E-09 cerrada (`25d464d`)

Oriol dejó la elección en mi mano y cogí la fila que era mía y estaba abierta:
`test_e09_bridge`. Resultó ser bastante más que `addStructuredCommand`.

`EventEditorState::save()` era **un stub**: `(void)doc; return true;`. O sea que
el criterio de E-09 («la misión se escribe des de l'editor») llevaba en verde
sobre el papel sin escribir nada. La implementación existió en el árbol el 17/09
(81 inserciones, anotadas en `TABLERO.md`) y se la llevó la reversión; `git grep`
en `ed84b18` encuentra la prueba pero nunca el código. Está reescrita a partir de
la prueba, que son 365 líneas de especificación.

`save()` convierte eventos, páginas, triggers y condiciones a la capa `events`,
es **idempotente** (guardar dos veces no duplica), **todo-o-nada** (convierte a
una lista local antes de tocar el documento) y usa `addEvent`/`setEventPages`/
`moveEvent` de `MapDocument` en vez de `mutableData()`, que son las que
revalidan contra el mapa. Y lo que de verdad cierra la fila: `EventPage` gana
`structuredCommands` (`vector<MapEventCommand>`) más `addStructuredCommand()`,
que es lo que faltaba para que los **parámetros** dejaran de perderse —la
«limitación conocida» que el propio `cmake/E09BridgeTests.cmake` documentaba. Si
una página lleva las dos listas, mandan las estructuradas.

**8/8**, compilando a la primera. Falsabilidad: con `save()` devuelto al stub
caen 7 de 8.

### Lo que más me costó ver, y es generalizable

ctest daba `test_e09_bridge` por **PASADO** incluso cuando el objetivo no
compilaba, porque ejecutaba un binario rancio de antes de la reversión. Es la
**tercera** vez en esta sesión que el directorio de build miente por objetos
viejos: antes pasó con `motor_input` (un `test_input_system` que fallaba sin que
el código estuviera mal) y otra vez al restaurar `event_editor.cpp` tras la
mutación de falsabilidad.

Regla que me aplico: **un verde de ctest sin reconstrucción verificada no es un
verde.** Si un objetivo «ya está», `touch` de la fuente y rebuild a la vista.

### Dos cosas que NO cierra esta fila

- El alcance: `medir_alcance` sigue diciendo que `editor/src/event_editor.cpp` no
  lo alcanza ninguno de los tres ejecutables reales, y `MapPropertiesPanel.cpp`
  (que sí usa `EventEditorState`) está entre los muertos nuevos de esta semana.
  Eso es **EST-42**.
- `addStructuredCommand` no entra en el historial de `undo()`: `EditOp` no tiene
  tipo para ello y añadirlo a medias sería peor. Está dicho en la cabecera.

Mi fila E-09 en `TABLERO.md` queda actualizada, commiteada con el índice aislado
(1 línea: la mía) y con el WIP de opencode intacto en el árbol —comprobado
después. Sigue sin regenerarse `sesiones/INDICE.md`, por la misma razón de antes.

---

## Tercera posdata — «déjalo impoluto» (Oriol, misma sesión)

Interpreté «impoluto» como lo único que se puede comprobar: **que la suite se
pueda construir entera y que ningún verde sea mentira**. No como «commitear el
WIP de los demás», que la regla 2 prohíbe y que además no llegaría a verde (los
catálogos están a medio cambiar).

Tres curas más, todas en ficheros **limpios y versionados**, o sea rojos de HEAD
y no WIP de nadie:

| Commit | Qué | Antes → después |
| --- | --- | --- |
| `dd3979a` | `test_animated_entity`: un `MockTextureAtlas` vacío pasado como `TextureAtlas*` (sin derivar, sin conversión posible) y **sin `int main()`** | no compilaba ni enlazaba → **11/11** |
| `235fcf6` | `ProjectDatabaseState::save()`/`load()` eran **stubs que devolvían `false`** | 5 fallos → **18/18** |
| `802153b` | `MemorySink.h`: documentada la decisión de que viva en `include/` | solo comentarios |

**El patrón, que es lo que de verdad importa:** tres veces hoy he encontrado un
criterio de paquete dado por cerrado con la implementación en stub. `save()` de
E-09 (`(void)doc; return true;`), `save()`/`load()` de E-16 (`return false;`), y
una prueba que nunca compiló. Los tres en ficheros versionados, los tres en
verde en el tablero. Un stub que devuelve un valor fijo pasa cualquier revisión
que no ejecute el código.

Y la razón de que duraran tanto: **ctest los daba por PASADOS ejecutando
binarios rancios**. Cuarta vez en la sesión que `out/build` miente por objetos
viejos. Esto merece una puerta propia, y no la he escrito porque una regla de
detección de stubs a medias bloquearía commits ajenos; va como propuesta:

> Que la guardia (o el CI) compare la marca de tiempo del binario con la de sus
> fuentes, o que `suite_gl_free` se ejecute tras un `--clean-first` periódico.
> Y un detector de cuerpos `(void)param; return constante;` en funciones no
> `inline`, como aviso y no como veto.

### Estado final medido

- **Objetivos que no compilan en HEAD: 4 → 1.** El único que queda,
  `test_inventory_menu`, sí es WIP vivo de opencode (`GameSession.h`,
  `InventoryMenuState.h` y la propia prueba, sucios hoy).
- **Suite GL-free: 169/175 → 171/175.** Los 4 fallos restantes vienen todos de
  ficheros sucios con gente encima, comprobado uno a uno:
  `demo_creacion_personaje` y `test_catalog_content` contra
  `assets/catalogs/*.json` modificados sin versionar (el demo espera 61 clases),
  `demo_nova_partida` contra `GameSession.h`, `test_gdb12_screens` contra
  `InventoryScreen.h`.
- Guardia **OK**, sin deuda nueva.
- `include/Editor/ProjectDatabaseState.h` lleva el bloque generado de PAQUETES
  DEL PLAN **duplicado** (líneas 1-10 y 12-21). No lo quito a mano: lo regenera
  `tools/marcar_paquetes.py` y volvería. Hace falta mirar por qué el marcador lo
  imprimió dos veces.

Sigue sin regenerarse `sesiones/INDICE.md`, y `TABLERO.md` sigue sucio con lo de
opencode; mis filas sí están actualizadas y commiteadas con el índice aislado.

---

## Cuarta posdata — barrido sistemático de stubs (Oriol amplió el mandato)

Oriol: «quiero que te sientas con la libertad de juzgar trabajo hecho por otros
si no están en trabajo activo sobre él en este momento». Guardado en memoria
como regla permanente: **el criterio para meterse no es la autoría, es si el
fichero está limpio o sucio.** Limpio y versionado → también es mío. Sucio →
regla 2, se documenta y no se toca.

Lo primero que hice con el mandato fue dejar de encontrar stubs por accidente.
Barrido de todo `src/` y `editor/src/` buscando cuerpos `(void)param; return
<constante>;` o vacíos: **6 candidatos, 4 falsos positivos** (`layerCount` y
`assetCount` son one-liners legítimos, `ObjectInspector::key` delega en
`ObjectInspectorInput` —el corte de EST-34— y `SpriteBatch::end` llama a
`flush()`). Los dos restantes sí son reales:

### `CommandListState::openBranch` / `closeBranch` (E-06) — stub, y con prueba verde

`editor/src/event_editor.cpp:392-400`, fichero limpio:

```cpp
Result<std::string> CommandListState::openBranch(const std::string& commandId) {
    (void)commandId;
    return Result<std::string>::Ok("branch_0");   // SIEMPRE el mismo id
}
bool CommandListState::closeBranch(const std::string& commandId) {
    (void)commandId;
    return true;                                   // no hace nada
}
```

Y aquí está lo que convierte esto en algo más que un stub olvidado. La prueba
`obrir_tancar_branqa` (`tests/Editor/CommandListTests.cpp:78`) **pasa**, y pasa
porque solo afirma los dos valores que el stub tiene hardcodeados:

```cpp
require_true("obrir branca retorna ok", branch.isOk());   // Ok("branch_0")
require_true("tancar branca retorna true", closed);       // true
```

No comprueba que dos `openBranch` den ids distintos (no los dan), que las
comandas añadidas después entren **dentro** de la rama, ni que `closeBranch`
devuelva false con un id desconocido. Es una prueba verde sobre una función que
no existe. El mismo patrón que `EventEditorState::save` y
`ProjectDatabaseState::save`, pero con la agravante de que aquí **hay prueba** y
aun así no delata nada.

**No lo he implementado, y es deliberado.** `m_commands` es un vector plano de
`CommandEntry`: anidar ramas de verdad es decidir un modelo de árbol, no rellenar
un cuerpo. Y `include/Editor/CommandListState.h` está en la lista de muertos
(no lo alcanza ninguno de los tres ejecutables reales), o sea que sería diseñar
un árbol para código que hoy no llega a nadie. Hace falta decidir primero si
E-06 se cablea o se retira — que es `EST-42`, y es de Oriol.

Lo que sí dejo dicho con precisión, para que no se pierda: **la conclusión de
E-06 no se sostiene**. `test_command_list` pasa 100% y su caso de ramas es
vacuo.

### `ExportPanelCore::cancelInProgress` — `return true;`

`editor/src/ExportPanel.cpp:107`. **Sucio**, es WIP activo (G-15): se reporta y
no se toca.

### Lo que propongo, y por qué no lo he escrito yo hoy

Una puerta en la guardia para esto tiene que ser **aviso, no veto**: mi propio
barrido da 4 falsos positivos de 6, y un veto con esa tasa bloquearía commits
ajenos todo el día. Convierte los cuatro en ruido y el hallazgo real en uno más.

Lo que sí vale la pena, y es barato:

1. Que una prueba que solo afirma `isOk()` y `== true` sobre una función que
   devuelve una constante se pueda señalar. La señal barata no es estática: es
   **mutación** — si cambiar el cuerpo por `return true;` no pone roja la prueba,
   la prueba no prueba nada. Ya lo usé cuatro veces hoy a mano; automatizarlo
   sobre los casos nuevos es lo que habría cazado E-06, E-09 y E-16 el día que
   se escribieron.
2. El barrido de stubs como **informe** (`tools/`), no como puerta.

---

## Quinta posdata — el mapa del juego (Oriol: «sin un mapa general no se puede tener claro nada»)

Oriol corrigió el rumbo: nada de soluciones baratas; si algo importante está
hardcodeado se cambia, y si ya no hace falta se borra. Y recordó el encargo de
fondo: el mapa **clase por clase**, para saber qué hace cada una, qué debería
hacer, qué se lo impide y cómo optimizarla. Empezar por el **juego**, desde
arriba; luego el editor; luego uno general; y más adelante un diseño de «lo que
hay + lo que esperamos que haya».

Primero leí lo que ya existe, porque existe y es mucho: `analisis_clases/01..08`
+ INDEX (~115 KB), `motor_grafico_clases.puml` (862 líneas), `architecture/` con
10 ADRs y un `.mdj`. Pero es del **31/08**, y medido contra hoy:

| | 31/08 | hoy |
| --- | ---: | ---: |
| Cabeceras | 101 | **213** |
| Líneas | ~23.500 | **54.233** |

Y el `.puml` no conoce `NarrativeEngine`, `SessionData`, `Logger`, `ScopeGuard`,
`MapDocument` ni `EditorOpciones`. Su propio INDEX confiesa que la tarea «se ha
comido tres sesiones», con un agente cortado por límite de contexto.

**Conclusión que le di y que aceptó implícitamente al pedirme el mapa:** un mapa
escrito a mano aquí miente en un mes, y ese mes ya ha mentido — es el mismo
mecanismo que tenía E-06, E-09 y E-16 cerrados en falso. Así que el mapa se
**genera**: `tools/mapa_clases.py` + `planificacion/MAPA_CLASES_JUEGO.md`
(commit `MAPA`). Las cuatro preguntas con evidencia, no con prosa: qué hace
(inventario medido), qué debería (la columna Paquete sale del bloque
`PAQUETES DEL PLAN` que cada cabecera ya lleva), qué se lo impide (Bloqueos,
cruzado con la línea base) y cómo optimizarla (líneas y miembros).

**Validación cruzada:** mi contador da 109 miembros en `Application.h` y 40 en
`SessionData.h`, los mismos números que la guardia obtiene con otro código. Dos
instrumentos independientes, misma cifra.

### El hallazgo que no buscaba, y es el más importante del día

**17 de los 42 ficheros de la lista `muertos` de la línea base commiteada los
alcanza HOY un ejecutable real.** El 40% de la lista es falso: `Locale`,
`InventoryMenuState`, `SaveSlotState`, `TitleMenuState`, `CharacterCreation`
(+`Screen`), `EventCommand`, `EventInterpreter`, `SessionCommandHost`, con sus
`.cpp`.

La foto congelada no está solo desfasada: está mal **en las dos direcciones a la
vez**. Deja fuera muertos nuevos y da por muertos ficheros vivos. Eso le cambia
el motivo a `D0007`, y para mal: ya no es solo que subir el trinquete necesite
permiso, es que re-congelar sobre `medir_alcance.py` tal como está fijaría una
lista que ya se sabe falsa. **Arreglar el instrumento va antes que re-congelar**,
y ahora con un segundo motivo además del de los `#include` entre cabeceras.

El mapa no repite la etiqueta: donde toca la contradice («la base la da por
MUERTA pero el ejecutable la alcanza»), que son 9 cabeceras.

### Lo que sigue, en el orden que sale del propio mapa

1. Los 3 «posible stub» que quedan por juzgar: `EventInterpreter.h`,
   `CharacterSheet.h`, `OptionsLocaleBridge.h`. (`IRenderable.h` es falso
   positivo: interfaz con implementación vacía a propósito.)
2. El juicio clase a clase del juego: qué debería hacer cada una y qué se lo
   impide. El mapa da el inventario; el juicio hay que escribirlo encima, área
   por área, y con el mapa al lado no se escribe sobre aire.
3. `mapa_clases.py motor_editor` y `level_editor`: el editor, que es lo que
   Oriol quiere después del juego.
4. Con los tres, el mapa general — y entonces sí el diseño de «lo que hay + lo
   que esperamos que haya».

`CommandListState` (E-06) sigue sin decidirse: implementar las ramas o borrarlo.
El mapa no lo resuelve solo porque `CommandListState.h` **no aparece en el mapa
del juego** — el ejecutable `juego` no lo alcanza. Eso ya es un dato: es código
de editor, y la decisión depende del mapa del editor, no del juego.
