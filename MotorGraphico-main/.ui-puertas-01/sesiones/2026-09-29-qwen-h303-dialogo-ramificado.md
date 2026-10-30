# 2026-09-29 · qwen · H3-03 por la vía de eventos, y el validador que decía «todas las 14 puertas alcanzables»

Continuación, en la misma sesión de trabajo, de
[H3-01 `WorldTravelState`](2026-09-29-qwen-h301-viaje-entre-naciones.md). Dos
entregas y una pregunta a Oriol en medio.

- **Commits:** MotorGraphico `a9e0b8d` (H3-01 cierre: `tools/conectividad.py` cuenta los viajes como aristas y deja de decir una frase falsa) y `18bdc02` (H3-03: `assets/misiones/conversacion_ramificada.json` + 5 casos en `MapEventRunnerTests.cpp` + `AdventureValidation.cpp` en su `.cmake` + el comentario corregido de `CondicionesYPaginas.cpp`). Repo raíz: esta sesión + las dos filas del TABLERO + su fila en `sesiones/INDICE.md`, aislados por blob como siempre.
- **Deja esperando a Oriol:** una decisión de diseño que salió de medir H3-03 y que **no** he tomado yo: el motor tiene **dos reglas opuestas** para elegir qué rama gana — en las páginas de evento gana la **última** que se cumple (el caso general va arriba) y en los beats narrativos gana el **primero** (el caso general va abajo). Se puede convivir con ello (está documentado en la cabecera de `CondicionesYPaginas.cpp`), o se puede unificar. Y sigue en su bandeja lo de H3-01: los nombres duplicados de los `mundi_landmass_*`.

---

## 1. La pregunta, y por qué se le hizo

La ficha de H3-03 pide «que las ramas dependan de **interruptores** y de hitos
jugados». Al medir el terreno apareció que el motor tiene **dos vocabularios de
condiciones** haciendo lo mismo:

| | dónde | qué entiende | quién lo evalúa |
| --- | --- | --- | --- |
| `MapEventCondition` (fase E) | `include/Map/EventLayerDocument.h` | `Flag`, `Variable`, `Item`, `LocalSwitch`, con `==  !=  >=  <=  >  <` y `negated` | `CondicionesYPaginas.cpp` + `SessionCommandHost.cpp` |
| `NarrativeCondition` (aventuras) | `include/RPG/Narrative/NarrativeBeat.h` | **solo flags** (`allFlags`/`anyFlags`/`notFlags`) | `NarrativeState.cpp:175` |

Ampliar el segundo para que entienda interruptores habría dejado la duplicación
instalada — el patrón que Oriol criticó el 28/09 (tres morales, dos motores de
comercio). Unificar los dos toca formatos y el grafo de dependencias. Así que se
le preguntó, con las tres opciones y su coste, y **eligió la vía de eventos: no
duplicar**. Consecuencia práctica: no se tocan `NarrativeBeat.h`,
`NarrativeState.cpp` ni el loader de aventuras.

## 2. `conectividad.py` — el cierre de H3-01 y una frase que mentía

El validador llevaba desde el 19/08 aplazando los 13 `desembarco_landmass_*` «a
la tarea de viajar entre naciones». Lo hacía de la peor manera posible: los
quitaba de la lista de inalcanzables, imprimía un AVISO y acto seguido la línea
de resumen decía

```
mapamundi: 1664/3888 celdas libres alcanzables (42%)
   todas las 14 puertas alcanzables
```

**«todas las 14 puertas alcanzables» era falso**: solo se puede pisar 1 (el
`playerStart`). Quien leyera la salida concluía que el mapamundi estaba entero.

Con `WorldTravelState` ya versionado, las puertas del mar se toman eligiendo
destino, no andando. La excepción por nombre de objeto
(`o.startswith("desembarco_landmass_")`) se volvió una propiedad declarada del
nivel —`VIAJE = {"mapamundi"}`— y el recuento pasó de una categoría a tres:
**andando**, **por viaje** (la puerta declara destino) y **sin destino** (objeto
marcado sin `targetLevel`: contenido pendiente, las seis masas del canon que aún
no tienen mapa). Ahora dice:

```
mapamundi: 1664/3888 celdas libres alcanzables (42%)  (el resto es mar: se cruza por viaje)
   [VIAJE] 7 puertas se toman por viaje, no andando: [desembarco_landmass_2 … 8]
   [AVISO] 6 objetos sin destino (contenido pendiente, no puertas): [desembarco_landmass_9 … 14]
   1 objeto alcanzable andando y 7 puertas por viaje
```

**La salida de los otros 97 niveles es idéntica byte a byte** — comprobado
filtrando el bloque del mapamundi y comparando las 194 líneas restantes —, el
exit code sigue siendo 0 y el recuento de «100 %» que lee
`regenerar_ciudad.sh:45` sigue siendo 97.

De paso, un defecto de la familia de las guardas apagadas: el aviso de los JSON
sin clave `"map"` en `assets/levels/` estaba **detrás del `sys.exit()`**, así que
no se imprimía nunca y `fallos += len(saltados)` no sumaba nunca. Un manifiesto
dejado en el directorio de niveles pasaba en silencio. Reordenado; hoy no hay
ninguno (medido: 0 JSON sin `"map"`), así que el exit code no cambia.

## 3. H3-03 — lo que ya existía, medido antes de escribir

El mecanismo de la fase E está **entero**, y no hizo falta tocarlo:

- `CondicionesYPaginas.cpp` evalúa `Flag`, `Variable` (las seis comparaciones) y
  `LocalSwitch` con el formato `"evento.letra"`; `EvaluateItemCondition`
  (`SessionCommandHost.cpp`) resuelve contra el inventario real y, sin cantidad,
  «tiene al menos uno».
- `NarrativeCommandHost::execute` (`Eventos/AnfitrionNarrativo.cpp:49-78`)
  ejecuta `texto.mostrar`, `bandera.*`, `variable.fijar/sumar` e
  `interruptor.fijar` sobre **el mismo** `NarrativeState` que se lee al elegir
  página. La sospecha de que el interruptor se escribía en la sesión y se leía en
  el estado narrativo —que habría sido un defecto de cableado gordo— resultó
  falsa: `GameSession::setLocalSwitch` delega en `m_data->m_narrativeState`.
- Las 15 familias de comando publicadas cubren el diálogo ramificado entero
  (`texto.mostrar`, `control.si`, `interruptor.fijar`, `variable.*`,
  `bandera.*`, `inventario.*`).

**Lo que no existía era contenido que lo ejercitara.** Volcado el único mapa
publicado con eventos (`assets/misiones/mision_llave.json`: 4 eventos, 9
páginas): **todas** sus condiciones son `flag`. Ni un interruptor local, ni una
variable, ni una condición de objeto en ningún asset del repo. Y ningún nivel
publicado declara `mapDocumentPath`, o sea que ese mapa ni siquiera se carga en
la partida.

La entrega es por tanto contenido + prueba, no código:

- **`assets/misiones/conversacion_ramificada.json`** (NUEVO, 417 líneas), «La
  taberna de las tres ramas». La tabernera ramifica por **interruptor local**
  (`tabernera.A`) y por la **variable** `charlas`; el forastero, por llevar
  `carta_sellada` en el inventario; el veterano, por el **hito**
  `mision_completada` — el mismo que enciende el altar de `mision_llave.json`,
  así que las dos piezas de contenido se hablan. La carta se coge en el
  mostrador y se deja en el buzón: la rama por objeto **tiene vuelta** y la del
  hito **no**, que es la diferencia entre una condición y un hito y conviene que
  un autor la vea en un ejemplo. Textos sin tildes ni eñes (regla 4).
- **5 casos nuevos en `tests/RPG/MapEventRunnerTests.cpp`** (21/21), conducidos
  **por casillas** sobre el asset publicado: `pulsar(4,4)` habla con la tabernera
  sin que nadie diga su id. El primero pasa el mapa por `ValidateEvents` (E-07),
  porque un contenido de ejemplo roto enseña al autor a escribir mal.
- `misionLlave()` se generalizó a `mapaPublicado(ruta)` en vez de duplicar la
  carga del segundo asset.

## 4. El hallazgo: la documentación decía lo contrario que el código

La cabecera de `CondicionesYPaginas.cpp` afirmaba:

> «ActivePage devuelve la **PRIMERA** pagina que se cumple»

y `ActivePageWith` (línea 150) recorre **de atrás hacia delante**: gana la
**ÚLTIMA**. Un autor que siguiera el comentario pondría la página sin condiciones
al final —donde gana siempre— y concluiría que las condiciones no funcionan. La
mutación C3 de la falsabilidad es exactamente ese error, y los casos la cazan.

Corregido el comentario (el diff de ese fichero es **0 líneas de código**: solo
comentario), y dejado escrito lo que de verdad muerde: **las dos reglas del motor
son opuestas**. Páginas de evento → gana la última, caso general arriba. Beats
narrativos (`NarrativeEngine::fire`) → gana el primero, caso general abajo. Dos
vocabularios que se parecen mucho y se ordenan al revés.

## 5. Verificación, y la trampa del arnés

- `test_map_event_runner` **21/21** (16 de H0-08 + 5 nuevos).
- **7 mutaciones del contenido publicado, todas cazadas** y cada una solo por los
  casos que la defienden: interruptor cambiado por bandera que nadie enciende,
  primera charla sin `variable.sumar`, página sin condiciones al final, forastero
  que ignora el inventario, veterano que ignora el hito, buzón que no recoge la
  carta, interruptor encendido en `B` mientras la página mira `A`.
- **La primera pasada dio «0 rojos en las 7»**, y esa es la quinta trampa de
  verificación de esta semana: ejecutaba el binario como
  `./out/build/qwen-h301/test_map_event_runner` **desde la raíz del motor**, así
  que leía el asset real de las fuentes y no la copia mutada del build dir (los
  tests GL-free corren con `WORKING_DIRECTORY = CMAKE_BINARY_DIR`). El sospechoso
  era el arnés, no el test: «0 rojos en N mutaciones independientes» es imposible
  si la prueba defiende algo. Corregido con `cwd=BUILD` y repetido entero.
- Suite GL-free **182 tests: 176 Passed, 0 Not Run, 6 Failed** — los 4 de siempre
  atribuidos con `git status` (`classes.json`/`npcs.json` e `InventoryScreen.*`
  están `M`: el test limpio y el sujeto sucio) y 2 de `linea_base_core` por
  `cmake/Pergamino.cmake` sin versionar (ajeno, en vuelo). **En modo `--indice`,
  que es lo que mira el gancho de pre-commit desde `ddad0d7`, la guardia da OK.**
- `conectividad.py`: exit 0 antes y después, 194 líneas ajenas idénticas, y
  `test_validar_todo` PASSED (el orquestador lo ejecuta y toma su returncode, así
  que quitar la regla de viaje pone CI en rojo: comprobado con `VIAJE = set()`).
- `cmake/MapEventRunnerTests.cmake` gana `src/RPG/AdventureValidation.cpp` porque
  **no está en ninguna librería**: se compila suelto en 5 objetivos
  (`AdventureValidationTests`, `AssetBrowserStateTests`, `MisionLlaveTests`,
  `MotorEditor`, `ProblemsStateTests`) y ahora en 6. Es la misma deuda que tenía
  `Locale.cpp` antes de H1-05 y que su propio `cmake/LocaleTests.cmake:12-15`
  predijo. Se siguió el patrón existente en vez de meterlo en `motor_rpg`, que
  habría tocado cinco `.cmake` de otras sesiones.

## 6. Lo que no está

**«y se vea».** El criterio de cierre de H3-03 termina en que la conversación
cambie **y se vea**. Hoy el diálogo llega a `NarrativeCommandHost::log()` y de ahí
no sale: enseñarlo es `Application` + HUD, y ambos tienen filas vivas encima
(opencode H1-03/H1-05, y el menú de jugador en vuelo). Es el mismo montaje que
bloquea H0-08 (§7 de la sesión anterior) y H3-01 (§7): tres tareas de la fase H
esperando el mismo fichero, que es justo lo que H8-06 («decidir qué hacer con
`Application.cpp`, 14 paquetes) existe para resolver.

## 7. Hallazgos que no arreglo

- **`AdventureValidation.cpp` sin librería** (6 objetivos lo compilan suelto).
  Meterlo en `motor_rpg` es un cambio de 6 líneas repartidas en 5 `.cmake` ajenos.
- **`mision_llave.json` lleva tildes minúsculas** en `texto` («Está cerrada»,
  «vacío», «rincón») y la regla 4 del protocolo las prohíbe en los campos de
  texto; además `BitmapFont.h:20` documenta la fila extendida del atlas **en
  mayúsculas** (À Á È É Í Ò Ó Ú Ñ Ç Ü). No verifiqué si las minúsculas acentuadas
  se pintan —no es mi zona y no hay fichero sucio que lo explique—, pero si alguien
  ve cuadrados en el diálogo del cofre, es esto. Mi asset nuevo va sin tildes.
- **`ValidateEvents` (E-07) no detecta el orden de páginas**: una página sin
  condiciones colocada al final gana siempre y no es un error sintáctico.
  Propuesta concreta: aviso cuando hay más de una página y la última no tiene
  condiciones. Es lo que la mutación C3 simula y hoy solo cazan mis tests.
- **Ningún nivel publicado declara `mapDocumentPath`**, así que ni
  `mision_llave.json` ni el mapa nuevo se cargan en la partida: los dos son
  contenido de prueba que solo viven en los tests y en `demo_mision_llave`. Que
  un nivel real los referencie es parte del montaje pendiente.
