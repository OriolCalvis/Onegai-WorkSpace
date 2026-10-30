# D0006 — Criterios de cierre de una tarea: probado, integrado, terminado

- **Estado:** `proposed` — en la bandeja de Oriol. Solo él lo pasa a `accepted`.
- **Fecha:** 2026-09-17 · **Autor:** qwen (sesión AUD-01), a partir del criterio
  adoptado por Oriol en conversación con ChatGPT y de la medición de Claude
  (`ALCANCE_REAL_2026-09-17.md` §6-7, punto 4, ya en su bandeja desde el 17/09).
- **Encargo de Oriol:** «quiero que también me ayudes a añadir questiones que
  avalen el cierre de una tarea».
- **Alcance:** el protocolo de trabajo (D0001) y los registros de
  `planificacion/seguimiento/`. **No** cambia código, ni reglas canónicas, ni el
  contenido de ningún paquete ya entregado.
- **Fuente operativa:** `MotorGraphico-main/MotorGraphico/planificacion/CHECKLIST_CIERRE_TAREA.md`
  (las preguntas) y `tools/medir_alcance.py` + `tools/analizar_duplicados.py`
  (las mediciones).

## Contexto

El proyecto distingue tres cosas que en la práctica se han tratado como una:

| Nivel | Definición | Quién lo comprueba hoy |
| --- | --- | --- |
| **Probado** | El código existe y su prueba pasa | `ctest` — automatizado |
| **Integrado** | El autor o el jugador puede *llegar* a ello desde la interfaz y completar la acción | `tools/medir_alcance.py` — existe desde el 17/09, pero **no obliga a nada** |
| **Terminado** | Una persona nueva lo usa sin JSON, sin terminal, sin IDs técnicos y sin ayuda | nadie — se declara en el registro |

Medido sobre HEAD `89f875e`, el resultado de cerrar solo con el primer nivel:

- **5 paquetes figuran cerrados y el jugador no los ve**: G-13 y G-14 (su lógica
  está escrita dentro de su propio fichero de prueba: la prueba se prueba a sí
  misma), G-10 (pausa inalcanzable y «Opciones» en `TODO`), G-09
  (`CharacterCreationScreen` sin consumidor) y G-17 (`Locale` sin consumidor).
- **24 cabeceras = 3152 líneas** que no alcanza ningún ejecutable, sostenidas por
  **44 pruebas en verde**.
- **El tutorial del Día 1 no se puede completar** desde «Campaña»: pide hablar
  con `tendero_mercado`, `joyero` y `sastre`, que están en un catálogo que la
  campaña no carga. `test_boundington_tutorial` pasa porque prueba el guion sin
  catálogo. Es el caso puro: *probado* en verde, *terminado* en falso.

Ninguno de los cinco es mala fe: en los cinco, quien cerró respondió «creo que
sí» a una pregunta que nadie había escrito. La regla que falta no es de
disciplina, es de formato.

## Decisión propuesta

**1. Ningún paquete se cierra si su producto no llega a un ejecutable.**
`estado: closed` en `planificacion/seguimiento/<P>.json` exige el bloque
`alcance` con `llega_al_producto: true` y la **ruta de usuario** escrita
(«Título → Campaña → Día 1 → E sobre el tendero»). Si la ruta pasa por editar un
JSON, por una flag de línea de órdenes o por un test, no es `true`.

**2. Ninguna prueba vale si prueba lo que ella misma define.** Se comprueba con
`tools/analizar_duplicados.py --solo LOGICA_EN_EL_TEST`: cruza los nombres que el
registro promete en `criterios_cierre` con el sitio donde cada nombre está
definido. Hoy señala G-13 y G-14.

**3. Los cinco paquetes se reabren.** G-13, G-14, G-10, G-09 y G-17 pasan a
`integrado_parcial` con una fila en `TABLERO.md` «Pendiente» cada uno. No se
borra su registro ni su trabajo —que es real y sirve—: se corrige la afirmación
de que ya está. Toca los JSON de sus autores, así que lo ejecuta Oriol o quien
él diga, no esta sesión.

**4. Tres claves nuevas en el registro de cierre**: `alcance`, `cierre`,
`deuda_que_dejo` (formato en el checklist). Son aditivas: los registros
existentes siguen siendo válidos y se rellenan cuando se reabran.

**5. Comprobación automática, en dos pasos.**
- *Paso A (no toca nada compartido):* el checklist se responde a mano y se pega
  en el registro. Vale desde ya.
- *Paso B (toca `CMakeLists.txt`, fichero de cruce con 14 paquetes vivos):*
  un test `cierre_por_alcance` que recorre `planificacion/seguimiento/*.json`, y
  para cada `estado: closed` exige (a) el bloque `alcance` con
  `llega_al_producto: true`, (b) que `medir_alcance` confirme que los ficheros
  del paquete los usa algún ejecutable, y (c) que `analizar_duplicados` no lo
  señale en `LOGICA_EN_EL_TEST`. **Se propone aplazarlo a que `Application.cpp`
  y `CMakeLists.txt` dejen de estar en vuelo**, porque meter un test nuevo en el
  cruce hoy es la trampa que la Parte 3 ya documenta.

**6. Excepción explícita: el núcleo puede cerrar sin alcance, si se dice.**
Un paquete de motor puro (fase C, herramientas, catálogos) no tiene ejecutable
propio y es legítimo que cierre en nivel *probado*. Entonces escribe
`alcance.llega_al_producto: false` **más** `alcance.motivo` y deja una fila en
«Pendiente» con el cableado que le falta. La diferencia con hoy no es la
exigencia, es que quede escrito: `LootResolver`, `WorldTopology` y `HandlePool`
son núcleo sin consumidor desde el 03/09 y nadie lo había anotado como deuda.

**7. `analizar_duplicados.py` no se registra en CTest en esta decisión.**
Su prueba (`tools/test_analizar_duplicados.py`, 26 casos plantados con sus
negativos) se ejecuta sola y con `--comprobar`. Registrarla toca
`cmake/DeclarationCheck.cmake`, que es zona de C-22 (claude, en vuelo).

## Alternativas consideradas

- **A. No formalizar; fiarse de la revisión por sesión.** Es lo que hay. Ha
  dado 5 paquetes mal cerrados en dos semanas, y el coste no lo paga quien
  cierra sino quien viene detrás a descubrirlo.
- **B. Exigencia total: nada cierra sin llegar al jugador.** Descartada: haría
  incerrables el núcleo, las herramientas y los catálogos, que son trabajo útil
  y necesario. La excepción del punto 6 cubre esto sin romper la regla.
- **C. Gate en CI desde ya.** Descartada por ahora: `CMakeLists.txt` es el
  fichero de cruce con más sesiones vivas y la Parte 3 documenta tres roturas de
  HEAD por tocarlo en concurrente. Paso A primero, paso B cuando se estabilice.
- **D. Medir por cobertura de tests.** No responde a la pregunta: G-13 tiene 6/6
  casos verdes y el jugador no ve su menú. La cobertura mide líneas ejecutadas
  por la prueba, no capacidad alcanzable por una persona.

## Consecuencias

- Cerrar pasa de ser una opinión a ser una afirmación con evidencia adjunta.
  Cuesta unos minutos por paquete y se paga una vez.
- Los cinco paquetes reabiertos **no** son trabajo perdido: su lógica existe y
  funciona. Lo que hay que hacer es moverla de `tests/` a `src/` y cablearla —
  que es justo lo que propone la fase H (`planificacion/JUGABILIDAD_FASE_H.md`).
- `ALCANCE_REAL_2026-09-17.md` deja de ser una foto fechada y pasa a ser el
  informe de una puerta: se puede volver a medir en cualquier momento.
- Si Oriol lo acepta, la fila «Regla de cierre por alcance» sale de su bandeja
  de «Esperando a Oriol» (está ahí desde el 17/09, puesta por Claude).

## Preguntas para Oriol

1. ¿Se aceptan los tres niveles como definición única de «hecho» en todo el
   proyecto (motor, editor, juego, contenido, herramientas)?
2. Los cinco paquetes reabiertos (G-09, G-10, G-13, G-14, G-17): ¿los reabres tú
   editando sus JSON, o autorizas a que lo haga una sesión con fila propia? Sus
   registros son de otros autores y la regla 2 de la bitácora los protege.
3. ¿La excepción del punto 6 (núcleo sin consumidor) se acepta con `motivo`
   escrito, o prefieres que todo paquete de motor tenga un demo ejecutable que lo
   use —que es más caro pero no deja nada flotando?
4. El gate en CTest (paso B): ¿se aplaza hasta que `Application.cpp` y
   `CMakeLists.txt` dejen de estar en vuelo, o lo autorizas ya asumiendo el
   riesgo de colisión?
5. `map_entity_fields_round_trip_2.json` y los 5 JSON idénticos del experimento
   de los cuatro: ¿se borran los duplicados (INF-06) o se dejan como prueba del
   experimento? Son contenido, no código, y la regla 2 protege el trabajo ajeno.
