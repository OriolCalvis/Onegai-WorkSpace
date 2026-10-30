# 2026-09-17 · qwen · E-13: navegador de recursos e importación (mitad sin interfaz)

**Encargo de Oriol:** coger el Gantt del motor gráfico y seguir programando con `MAPA_DEL_CODIGO`, teniendo en cuenta que hay más agentes trabajando en paralelo.

**Commit:** MotorGraphico `1621eaa` — `feat(editor): E-13 — navegador de recursos, mitad sin interfaz` (10 ficheros, +1273).

## Análisis previo: dónde NO meterse

Leídos BITÁCORA (protocolo), TABLERO, el Gantt del corte B, `MAPA_DEL_CODIGO.md`, mis dos sesiones de esta mañana (E-15 y E-09) y el estado de los dos repos.

- **Mi zona de E-09 estaba caliente.** `editor/src/event_editor.cpp` modificado a las 11:50 (81 inserciones sobre HEAD, yo había dejado 61) por otra sesión que está añadiendo el camino de `structuredCommands` a `save()` y el cuerpo de `addStructuredCommand`; `tests/Editor/E09BridgeTests.cpp` y `cmake/E09BridgeTests.cmake` (ambos sin versionar) tocados en los últimos 25 minutos. Regla 2: no lo deshago, no lo commiteo, lo registro. Mi puente de la mañana sigue vivo en el árbol y otra sesión lo está ampliando.
- **Elegido E-13** (mi fila del 14/09, 4 j). Motivo de concurrencia, no de preferencia: los ficheros que hay que escribir **no existían** (`src/Editor/AssetBrowserState.cpp`, `tests/Editor/AssetBrowserStateTests.cpp`, `cmake/AssetBrowserStateTests.cmake`), así que no hay colisión posible, y la cabecera `include/Editor/AssetBrowserState.h` está versionada y completa — es el contrato y no se toca. Además era la deuda más visible del día: 16 de las 17 declaraciones sin cuerpo que listaba `tools/comprobar_declaraciones_rpg.py`.
- **Build propio** en `out/build/qwen-e09` (configurado desde cero) para no pisar `out/build/macos-debug` —que la fila de kimi reserva para su verificación— ni el `build/` heredado.

## Qué se hizo

### Nuevo: `src/Editor/AssetBrowserState.cpp` (los 16 cuerpos que faltaban)

- **`ParsePngInfo` / `ReadPngInfo`**: firma + IHDR leídos a mano en big-endian. Sin stb_image y sin contexto GL: la previsualización de dimensiones no puede costar una ventana. Errores con palabras (no-PNG, fichero corto, primer chunk que no es IHDR, longitud imposible, dimensiones no positivas, fichero inexistente) y el mensaje de `ReadPngInfo` nombra la ruta, que es lo que el panel le enseña al autor.
- **`ComputeFrameGrid`**: imagen suelta = 1×1; una hoja que no divide la imagen en entero es un **error antes de escribir nada** (una celda partida a medias es un sprite roto y se ve mejor al importar que dentro del juego); celda mayor que la imagen, también.
- **Búsqueda**: reutiliza `NormalizeForSearch` de E-15 (DEC-15) en vez de copiar el plegado de acentos una tercera vez —que es lo que `CatalogPickerState` tuvo que hacer porque entonces `ProblemsState` estaba en vuelo y ya no es el caso. «mision» encuentra «Misión de la llave», «ÉTER» encuentra «Éter», y también se busca por id. Consulta vacía = todo; tipo `Unknown` = sin filtro.
- **Selección por identidad**, como el foco de E-15: se guarda el id y `resolverSeleccion()` lo vuelve a resolver contra el catálogo de hoy. `Kept` si sigue, `Lost` si ya no está, y **nunca** se selecciona al vecino que ha ocupado su sitio.
- **`planImport`** no escribe nada: id y referencia con `ProjectIndex::suggestId` (E-11) —la MISMA regla que los proyectos, no una segunda—, referencia **relativa** (`carpeta/id.png`, sin barras sobrantes), dimensiones del origen, frames validados y colisión por fichero **o** por id.
- **`commitImport`**: con colisión, `Reuse` devuelve la referencia que ya existe sin escribir un byte y `Cancel` se niega diciendo qué es lo que ya existe. Sin colisión, se **vuelve a comprobar** el fichero de destino y el id justo antes de escribir —en este árbol trabajan varios agentes a la vez sobre los mismos assets— y se pierde la importación antes que el original.
- La copia pasa por `<destino>.tmp` + `rename` (el patrón de `ProjectSerializer::atomicWrite`), sin `MOVEFILE_REPLACE_EXISTING` a propósito: si el destino aparece entre medias, el movimiento falla y el original queda intacto.
- **Sin `mkdir` y sin `<filesystem>` en producción**, igual que `ProjectIndex`: `ProjectHub.h` documenta que este motor no tiene ni un `mkdir` en `src/`. Si la carpeta de destino no existe, el error lo dice («este motor no crea carpetas») en vez de crearla a escondidas.

### Arreglo en E-15 (zona propia, anexo `E-15-arreglo-interruptor-local.json`)

`ClaseDeCondicion()` no tenía el caso `EventConditionKind::LocalSwitch` y caía en el `return ReferenceKind::Flag` del final. Doble consecuencia: `FindReferences(LocalSwitch, "A")` no encontraba las **condiciones** de página que leen ese interruptor, y `FindReferences(Flag, "A")` devolvía una cosa que no es una bandera. Caso añadido + regresión en `ProblemsStateTests.cpp`. Salió al compilar `ProblemsState.cpp` dentro del target nuevo de E-13: era el único warning de los dos targets.

### Plan

- `planificacion/pla/dades.py`: añadidos `src/Editor/AssetBrowserState.cpp` y `tests/Editor/AssetBrowserStateTests.cpp` a la lista de ficheros de E-13.
- `planificacion/MAPA_DEL_CODIGO.md` regenerado con `tools/marcar_paquetes.py --mapa`: estaba **caduco** (también recoge entradas ya versionadas de E-04, D-01 y G-02). `PLA_DEFINITIU.md` y el Gantt **no** se regeneran: `build.py` leería `seguimiento/*.json` que otras sesiones tienen sin versionar.
- Banners de los ficheros nuevos generados con la propia función `banner()` de `marcar_paquetes.py`, para que un `--marcar` futuro los dé por «ya al día».
- `cmake/AssetBrowserStateTests.cmake` + una línea de `include` en `CMakeLists.txt`. `AssetBrowserState.cpp` **no** entra en ninguna librería, igual que `ProblemsState.cpp` (E-15) y `CatalogPickerState.cpp` (E-14): editar el bloque compartido del CMakeLists con cinco agentes encima no compensa hasta que el panel lo necesite enlazado.

## Verificación

- `test_asset_browser_state`: **20/20** verde, sin ventana ni GL. Cubre los cuatro puntos del criterio (búsqueda plegada, selección por identidad que sobrevive al refresco, PNG leído sin GL, importación con referencia relativa que nunca sobreescribe) y el de `dades.py`: «mover la carpeta del proyecto y reabrir conserva el recurso» —se importa, se renombra la raíz y la referencia relativa sigue resolviendo contra la raíz de hoy.
- `test_problems_state`: **18/18** (17 previos + la regresión del interruptor local).
- `tools/comprobar_declaraciones_rpg.py`: de **17 deudas conocidas a 1** (queda `ObjectInspector::confirmar`, de E-12/E-14).
- **HEAD limpio**: `git archive` de `1621eaa` en `/tmp`, configure OK y `suite_gl_free` construye `test_asset_browser_state` (49 %) y `test_problems_state` (52 %) —o sea que la dependencia de la suite queda registrada aunque el `include` vaya por detrás de su creación—; el test pasa 20/20 en el clon limpio.
- **La suite GL-free de HEAD NO compila entera, y no es de este commit**: `tests/RPG/EventInterpreterTests.cpp` da 9 errores `unknown type name 'NarrativeState'` (líneas 500+). `git show --stat HEAD` no toca ese fichero; viene de `4720b62` (EVT-05, opencode) y en el árbol hay sin commitear justo la línea que lo arregla (`using RPG::NarrativeState;`). Zona ajena viva: registrado, no tocado.
- CI remota pendiente, como todos los paquetes de esta rama.

## Registros para otras sesiones (no resueltos en silencio)

1. **C-22 (claude):** las 16 líneas de `include/Editor/AssetBrowserState.h` en `CONOCIDAS` de `tools/comprobar_declaraciones_rpg.py` ya tienen cuerpo; el comprobador las pide borrar (avisa, no falla). El fichero es de su fila viva.
2. **Para quien tenía staged el borrado de `include(cmake/TitleFlowTests.cmake)`:** al fijar el índice de `CMakeLists.txt` a HEAD + mi única línea, ese staged se ha caído del índice. En el árbol la línea sigue (1153) y `cmake/TitleFlowTests.cmake` existe: no se ha perdido trabajo, pero quien lo quitara a propósito debe repetirlo.
3. **E-09:** en el árbol hay `include(cmake/E09BridgeTests.cmake)` sin versionar y ni el `.cmake` ni el test lo están; tienen que entrar en el mismo commit o HEAD no configura (ya lo señalaba Claude en Pendiente).
4. **EVT-05 / E-08:** HEAD rojo por `EventInterpreterTests.cpp` (ver arriba). El arreglo está en el árbol, sin commitear.
5. **`pla/seguiment.py`** sigue sin poder leer los registros por `DEC-02.json` (sin `id`), que no es un paquete. Mis dos registros (E-13 principal y E-15 anexo) cumplen el contrato de las seis claves, comprobado con el propio cargador.

## Concurrencia: qué se ha commiteado y qué no

Commit propio con la técnica del blob (índice = HEAD + sólo mis líneas) porque `CMakeLists.txt` lleva hunks ajenos sin versionar (`src/RPG/EventCommands.cpp` en `motor_rpg`, el `include` de E-09). Comprobado después: el árbol **sigue** con el WIP de cursor (G-02 shell), claude (G-12), zcode (E-08), opencode (EVT-05) y la sesión E-09 intacto.

**Sin commitear a propósito:** `TABLERO.md` y `sesiones/INDICE.md`. El TABLERO del árbol lleva filas sin versionar de Codex (ALPHA-00), Claude (G-07) y cursor (E-12), y el INDICE regenerado citaría 20+ ficheros de `sesiones/` que siguen sin versionar (los de opencode del 10 al 12/09). Mis ediciones están en el árbol, visibles para todos; las commitea quien versionee el conjunto.
