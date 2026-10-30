# D0007 — Re-congelar el alcance real: el trinquete de los muertos sube de 42 a 50

- **Estado:** `proposed` — espera a Oriol.
- **Fecha:** 2026-09-21 · **Autor:** qwen.
- **Encargo:** «puedes tomar tú la iniciativa empezada con Claude de limpiar y
  depurar todo el proyecto». Esta decisión aparece al ejecutar **EST-02** del
  `PLAN_ESTABILIZACION.md`.
- **Alcance:** si se acepta la foto nueva del alcance y se re-congela la línea
  base con ella. No decide qué se hace con los ficheros muertos: eso es
  **EST-42**.

## ACTUALIZACION 28/09 — la medida de abajo (21/09) ya no vale

Re-medido hoy con el mismo procedimiento. Los números han crecido mucho, y la
decisión es la misma pero más urgente:

| | 21/09 | **28/09** |
| --- | ---: | ---: |
| Entran en la lista de muertos | 11 | **34** |
| Salen (ya se cablearon) | 3 | **22** |
| Neto sobre la línea base (42) | 50 | **54** |
| Veredicto de la guardia | FALLO, 11 de deuda | **FALLO, 35 de deuda nueva** |

Entran cosas nuevas de esta semana: `ExportPanel`/`ExportState` (G-15),
`ProjectHub`, `ProblemsNavigation`, `ProblemsInternal`, `Companeros`,
`ShopEngine`, `TitleScreen`, `HudTimer`, los ocho `CatalogLoaders*` y
`RpgCoreDefinitions` (que EST-30 partió y nadie volvió a enchufar),
`EventosInterno` (que EST-32 partió), y **`include/Core/Log/MemorySink.h`**.

Dos lecturas honestas de esa lista, y las dos importan para decidir:

1. **Buena noticia:** salen 22. El cableado de esta semana (`motor_slots`, los
   relevos de EST-20/23/24/27) enchufó de verdad código que estaba muerto. El
   trinquete funciona cuando se le deja medir.
2. **Mala noticia, y es estructural:** muchos de los que entran son **cabeceras
   hijas de un troceado** (`CatalogLoaders*`, `EventosInterno`,
   `ProblemsInternal`, `RpgCoreDefinitions`). `medir_alcance.py` **no sigue los
   `#include` entre cabeceras**, así que una cabecera a la que solo llega otra
   cabecera cuenta como muerta. EST-42 ya lo había detectado a mano («no son 42,
   son 9»). O sea: parte de estos 34 no son código muerto, son un **defecto del
   instrumento**.

Eso cambia la decisión. Antes de re-congelar nada habría que arreglar el
instrumento, porque re-congelar sobre una foto que cuenta mal es fijar un número
que nadie va a poder defender.

**Propuesta revisada, en este orden:**

1. Que `medir_alcance.py` siga el cierre transitivo de `#include` entre
   cabeceras. Sin esto, cada troceado de la fase 2 añade «muertos» falsos y la
   ley 4 castiga precisamente el trabajo que el plan pide hacer.
2. **Después** regenerar y re-congelar, y abrir EST-42 sobre lo que quede.
3. `MemorySink.h` es un caso aparte y es mío: es un doble de pruebas que vive en
   `include/`. O se justifica como pieza pública (sirve para volcar una traza
   sin tocar disco) o se muda a `tests/`. Lo decido yo, no hace falta D0007.

Lo que sigue debajo es la medida del 21/09, que se deja como registro de cómo se
llegó aquí.

---

## Contexto: lo que hay hoy

`tools/linea_base_core.py` compara el árbol contra
`planificacion/linea_base_core.json` y **solo admite que la deuda baje**. Una de
las partidas es `muertos`: ficheros que no alcanza ningún ejecutable. Hoy dice
42.

Esa lista sale de `planificacion/alcance_real.json`, que genera
`tools/medir_alcance.py`. La guardia avisa de que esa foto tiene **4,0 días** de
antigüedad respecto a `src/Engine/Input/InputSystem.cpp`. EST-02 pide
regenerarla.

## Lo que pasa al regenerarla (medido hoy, no estimado)

`python3 tools/medir_alcance.py --json planificacion/alcance_real.json` y luego
la guardia:

**Entran 11 partidas que la foto vieja no veía:**

| Fichero | Por qué está muerto |
| --- | --- |
| `editor/src/AssetBrowserPanel.cpp` · `include/Editor/AssetBrowserPanel.h` | E-13 los commiteó (`05eb13b`) y ningún ejecutable los enlaza |
| `src/Editor/AssetBrowserState.cpp` | idem; su cabecera ya estaba en la lista |
| `include/Editor/MapPropertiesPanel.h` | solo lo incluye `tests/Editor/MapPropertiesTests.cpp` |
| `include/Game/BattleMenuState.h` · `src/Game/BattleMenuState.cpp` | solo su propio `.cpp` y `tests/Game/BattleMenuTests.cpp`. Canta como «paquete cerrado que no llega a nadie» |
| `include/Level/LevelSerializer.h` · `src/Level/LevelSerializer.cpp` | solo `examples/demo_congelacio.cpp` y `SchemaFixtureTests` |
| `include/RPG/AdventureSerializer.h` · `src/RPG/AdventureSerializer.cpp` | idem |

**Salen 3 que ya no lo están:** `include/Editor/ProjectIndex.h`,
`include/Game/SaveSlotState.h` y `src/Game/SaveSlotState.cpp` — las ranuras se
cablearon vía `motor_slots`.

Neto: **42 → 50 muertos.**

## Por qué la foto nueva es la correcta

1. `medir_alcance.py` solo cambió en el parseo de argumentos (`--json RUTA` se
   tomaba `RUTA` como la raíz del repo). **El criterio de medida no se tocó**,
   así que la diferencia es del árbol, no del instrumento.
2. El instrumento mide los **3 ejecutables reales** (`juego`, `motor_editor`,
   `level_editor`), no los 54 demos. Que `LevelSerializer` lo alcance
   `demo_congelacio` no lo salva: la pregunta es si llega al producto.
3. Comprobado a mano con `grep` de includes: los 11 cuadran.

## La decisión

Aceptar que **el trinquete suba de 42 a 50** en el mismo commit que la foto
nueva, porque no es deuda nueva: es deuda que la foto vieja no veía.

Alternativa rechazada: dejar la foto de hace 4 días. Mantiene la guardia en
verde y el número bonito, pero la guardia **miente por omisión** — E-13 metió
código que no llega a nadie y nadie se enteró.

## Consecuencias

- **Si se acepta:** re-congelar con `linea_base_core.py --actualizar` en el mismo
  commit que el `alcance_real.json` regenerado, diciendo en el mensaje que es
  corrección de medida y no deuda nueva. Y abrir **EST-42** sobre los 11 el mismo
  día, porque un trinquete que sube sin trabajo detrás es solo un número peor.
  Conviene entonces añadir a la ley: *la línea base puede subir cuando cambia el
  instrumento, y eso se llama corrección de medida, se motiva en el commit y se
  acompaña de la fila de trabajo correspondiente*.
- **Si se rechaza:** la guardia sigue dando verdes falsos sobre ficheros muertos
  recientes, y el AVISO de foto vieja se queda permanente.
- Pase lo que pase, los 3 que salen de la lista son buena noticia y deberían
  descontarse: el cableado de `motor_slots` funcionó.
