# 02 · RPG — juicio clase a clase

**Ámbito.** Las 29 cabeceras y 68 clases de `include/RPG/` que alcanza algún
ejecutable, más las 13 que no alcanza ninguno. Es la sección `RPG` de
`planificacion/MAPA_CLASES_JUEGO.md`, y es el área con más clases del juego.

**Método.** La lista «métodos sin llamar» de `medir_alcance.py` es su propia
advertencia: *es optimista hacia fuera y no mira dentro*. Se ha comprobado uno a
uno los que sostienen una afirmación de este documento, y el resultado cambia el
sentido de varios: `ConditionEngine::purge_expired` aparece como sin llamar y
**sí se llama**, cuatro veces, desde dentro de su propio `.cpp`. Lo que sigue
distingue entre «verificado con `git grep`» y «candidato del instrumento».

---

## Los seis hallazgos del área

### 1. DEC-09 no está cableado: el juego guarda en mitad de un evento

`Application.cpp:943`, dentro de `quickSave()`:

```cpp
const Game::SaveGate puerta = Game::CanSaveNow(m_session->mode(), /*eventRunning=*/false);
```

`eventRunning` es **la constante `false`**, pasada con su comentario de argumento
nombrado. O sea que la puerta de guardado nunca ve un evento en marcha, por mucho
que lo haya. Y el valor correcto estaba a una línea: `Application` posee
`std::unique_ptr<RPG::EventInterpreter> m_eventInterpreter` (`Application.h:374`)
e incluye la cabecera, y `EventInterpreter::canSave()` existe justamente para esto
— el comentario de `SaveSlotState.h:157` lo nombra: «`eventRunning` es lo que
responde `EventInterpreter::canSave()` al revés».

**Consecuencia sobre una decisión aceptada.** E-03 lleva DEC-09 en su criterio:
«no es pot desar enmig d'una acció important; en sortir i tornar es reprèn des de
l'últim punt segur, i cap recompensa s'entrega dues vegades». Con `false`
hardcodeado, el jugador guarda a mitad de un evento y al cargar reanuda donde el
guardado diga, no en el último punto seguro. La mitad de «ninguna recompensa se
entrega dos veces» depende de ese mismo punto seguro.

**Por qué no se ve.** `CanSaveNow()` está bien y tiene sentido; dada la entrada
que recibe, devuelve lo correcto. Y `EventInterpreter::canSave()` está entre los
17 métodos que el juego no llama, así que ninguna prueba falla: la autoridad
existe y nadie le pregunta. Es el mismo mecanismo que tenía E-06, E-09 y E-16
cerrados en falso.

**No se arregla aquí.** `src/Engine/Application.cpp` está **sucio** con el WIP de
opencode (H1-05, hoy). Regla 2: se reporta a su dueño. El arreglo es una línea —
`m_eventInterpreter && m_eventInterpreter->isRunning()` o `!canSave()` — y un caso
de prueba que guarde con un evento a medias y afirme que la puerta lo impide.

### 2. Cuatro subsistemas compilados que no llegan al binario del juego

Verificado contra el binario, no contra el instrumento: `nm -C` sobre
`out/build/macos-debug/juego` (27.140 símbolos) da **cero** apariciones de
`LootResolver`, `Audio`, `AdventureSerializer` y `AdventureValidation`, mientras
que el control positivo da 64 de `CharacterSheet`, 224 de `GameSession`, 116 de
`ConditionEngine`. Sus `.cpp` están en `CMakeLists.txt` de HEAD (líneas 97, 98,
115) pero en un target que `juego` no enlaza. *(Corregido el 29/09: sí lo enlaza.
Los tres `.cpp` de esas líneas son de `motor_rpg`, que llega a `juego` a través de
`motor_level` y `motor_saves`. No entran en el binario porque ningún código del juego
los usa, y de una librería estática el enlazador solo toma lo que alguien referencia.
Para enlazarlos no hay que tocar CMake: hay que usarlos.)*

| Pieza | Líneas | Pruebas | Qué significa que no llegue |
| --- | ---: | ---: | --- |
| `LootResolver` | 21 + .cpp | 1 | El botín existe como motor y el jugador no recibe botín por este camino |
| `Audio` | 131 + .cpp | 1 | El juego no tiene sonido. No es que esté mal: es que no está enlazado |
| `AdventureSerializer` | 39 + .cpp | 2 | La aventura no se serializa desde el juego |
| `AdventureValidation` | 130 + 2 .cpp | 2 | La validación de aventuras no corre ni al cargar. La sesión del 28/09 curó aquí un símbolo duplicado (`03a3774`): se arregló un fichero que no llega al jugador. *(29/09: no es del juego sino del editor —E-07: «l'editor la crida abans de desar i abans del playtest»—; se compila dentro de `motor_editor` y la llama `ProblemsState`, a la que no llama nadie.)* |

### 3. El instrumento tiene dos puntos ciegos, y aquí se pueden cuantificar

De las 13 cabeceras RPG que «no alcanza ningún ejecutable», **8 son falsos
negativos** comprobados:

- **Los siete `Catalogs/CatalogLoaders*.h`** (1.710 líneas en total) son
  **especializaciones de plantilla** de `CatalogLoader<T>::from_json` /
  `expected_fields`. `RpgCatalogs.h:85` y `:111` las llaman, y el juego usa
  `RpgCatalogs.h`. Pero la regla del instrumento es «ese fichero NOMBRA algo que
  la cabecera declara», y una especialización no se nombra en la llamada: se
  instancia. Los cargadores de D-01..D-07 sí llegan al jugador.
- **`Definitions/RpgCoreDefinitions.h`** es un paraguas puro: 17 líneas, seis
  `#include`, **ningún símbolo propio**. Lo incluyen 15 consumidores
  (`CharacterSheet.h`, `ConditionEngine.h`, `InventoryEngine.h`, `Player.h`, las
  seis `Definiciones*.h`). Una cabecera que no declara nada no puede ser «usada»
  nunca según esa regla.

O sea: **62 % de falsos negativos en esta lista**, frente al 40 % de falsos
muertos que ya se encontró en la línea base congelada. Es un segundo motivo,
independiente del de los `#include` entre cabeceras, para lo que ya dice la sesión
del 28/09 sobre `D0007`: **arreglar el instrumento va antes que re-congelar**.
Concretamente, hacen falta dos reglas: una cabecera que solo incluye está
alcanzada si lo está cualquiera de las que incluye; y una especialización
`template<> ... Clase<T>::metodo` cuenta como uso de la cabecera que la declara.

### 4. La progresión de tier no está cableada

`CharacterSheet::grantMilestones` y `tierUp` aparecen sin llamar desde el juego y
`git grep` lo confirma: solo existen la definición (`CharacterSheet.cpp:98`) y dos
comentarios que dicen «desde el juego no se llama a mano: el camino público es
`grantMilestones()`». El camino público tampoco se llama. H2-04 arregló que
`grantMilestones` refrescara el cap de vida al subir de tier; el arreglo es
correcto y no lo ejecuta nadie desde el juego.

### 5. Los paquetes C-21 y C-22 se incumplen en las cabeceras que los llevan impresos

C-22 dice «Retirada de l'API morta: el que no es fa servir surt». C-21 dice «un
sol generador d'atzar… la que no es fa servir surt del repositori». Y las
cabeceras que llevan esos bloques tienen API sin usar:

| Cabecera | Criterio impreso | API sin consumidor verificado |
| --- | --- | --- |
| `DicePoolEngine.h` | C-22 | `es_pifia`, `resolve_opposed` (sin llamar en juego **y** level_editor) |
| `RandomEngine.h` | C-21 | `roll_in_range`, `seed` — y `seed` es justo lo que su documentación promete («permite reproducir partidas guardando seed») |
| `TierRules.h` | C-22 | `at` (sin llamar desde level_editor) |
| `EnhanceHooks.h` | C-22 + D-09 | — limpia |

No es un reproche a quien los escribió: C-21 y C-22 son paquetes de retirada, y
retirar exige decidir primero quién debe usar lo que queda. Pero el estado real es
que **el criterio está impreso en la cabecera y la cabecera no lo cumple**, que es
exactamente lo que el mapa se generó para hacer visible.

### 6. `resolve_opposed` sin llamar significa que no hay tiradas enfrentadas

`DicePoolEngine` es «enfrentada pura + pifia nueva» según el análisis del 31/08, y
`resolve_opposed` es la enfrentada. Ni el juego ni el editor la llaman. Si Nd6 Ed. 3
resuelve los ataques enfrentando pools, falta el cable entre `SkillExecutor` y
`resolve_opposed`, o el combate está resolviendo por otro camino. Es la pregunta de
reglamento más importante del área y no se puede responder desde el código: hace
falta el canon.

---

## `include/RPG/EventInterpreter.h` — 322 líneas · juego · 5 pruebas · la base la da por MUERTA

Es la cabecera más grande del área y una de las 9 que la línea base congelada da
por muertas mientras el ejecutable las alcanza.

### `EventInterpreter` (línea 166, 155 de cuerpo)

- **Qué hace.** Intérprete de eventos de mapa que ejecuta comandas a lo largo de
  varios frames, con esperas por tiempo y por condición, procesos paralelos que no
  bloquean al jugador, traza temporal, puntos de parada y modo paso a paso.
- **Qué debería hacer.** E-03, con DEC-09 (punto seguro de guardado) y DEC-30
  dentro del criterio.
- **Qué se lo impide.** **17 métodos públicos sin llamar desde el juego**, y no
  son de detalle: `canSave`, `blocksPlayer`, `isRunning`, `runs`, `cancelAll`,
  `collectFinished`, `nextCommand`, `nextCommandPath`, `setMode`,
  `stepBlockedReason`, `lastStepNote`, `addBreakpoint`, `removeBreakpoint`,
  `clearBreakpoints`, `breakpoints`, `lastHit`, `clearTrace`. O sea: el motor de
  depuración entero (E-07/E-08) y las dos consultas de las que depende DEC-09.
  Cinco pruebas para 155 líneas de cuerpo y 17 métodos sin consumidor.
- **Cómo optimizarla.** No es un problema de tamaño sino de cable. Lo primero es
  `canSave`/`isRunning` (una línea en `quickSave()`, ver hallazgo 1). Lo segundo
  es decidir si el depurador de eventos se cablea al editor o se retira: hoy es la
  pieza más grande del área y no la ejecuta nadie.

### `EventHost` (línea 103, 17 de cuerpo)

- **Qué hace.** Lo que el intérprete necesita del mundo: `execute()` pura y
  `waitResolved()` con implementación por defecto.
- **Qué debería hacer.** E-04.
- **Qué se lo impide.** Nada. El `(void)command` que el mapa señalaba como posible
  stub es la implementación por defecto deliberada de un hook virtual, con dos
  overrides de producción y llamada real en `EventInterpreter.cpp:105`: **falso
  positivo, confirmado y juzgado**.
- **Cómo optimizarla.** Nada. Es la interfaz correcta y mínima.

### `CommandResult` (82), `EventRun` (122), `TraceEntry` (145), `Breakpoint` (155)

Cuatro estructuras de datos, de 19 a 6 líneas de cuerpo. `TraceEntry` es el «log
con orden temporal» de la especificación §15.3 y `Breakpoint` comparte coordenada
con el diagnóstico de E-07 a propósito, para que «Ir ahí» y «poner un punto de
parada» hablen el mismo idioma: dos decisiones de diseño correctas y escritas.
**Qué se lo impide:** que `clearTrace`/`breakpoints`/`lastHit` no se llamen, o sea
que la traza y los puntos de parada se acumulan sin que nadie los lea. Se optimizan
cableándolos al panel de problemas del editor, que ya existe (E-15).

---

## `include/RPG/CharacterSheet.h` — 235 líneas · juego + level_editor · 26 pruebas

### `ActiveCondition` (línea 53, 6 de cuerpo)

- **Qué hace.** El estado de una condición activa: id, origen, rondas restantes y
  pilas.
- **Qué debería hacer.** C-20: la ficha es el almacén, `ConditionEngine` el único
  escritor.
- **Qué se lo impide.** Nada. Es exactamente el almacén que C-20 pide.
- **Cómo optimizarla.** Nada.

### `CharacterSheet` (línea 60, 174 de cuerpo)

- **Qué hace.** La fuente única de verdad del personaje: stats con sus seis
  arrays de bonificadores, equipo en 7 slots, tres inventarios, habilidades,
  pasivas, rasgos, dotes, condiciones, facción, y los valores derivados en caché
  (`recompute_derived` → `healthCap`, `defenses`) que `Player`/`Enemy` leen sin
  argumentos.
- **Qué debería hacer.** C-20 (almacén, no escritor de reglas) y D-06
  («bonificadors, passives i trets de classe, raça i trasfons arriben a la fitxa»;
  vínculo, miedo, defecto, meta, ideal, personalidad y virtud se guardan).
- **Qué se lo impide.** Diez métodos sin llamar desde el juego, de los cuales tres
  están verificados como reales: `grantMilestones` y `tierUp` (progresión sin
  cablear, hallazgo 4) y el trío de facción `getRep`/`adjustRep`/`repPriceModPct`
  — que es la reputación escrita y sin cablear que ya señalaba la auditoría de
  duplicados, y que además tiene una discrepancia propia: el comentario promete
  −50 % a reputación +100 y la fórmula da −75 %. `conditionStacks` y `hasCondition`
  quedan como candidatos: el instrumento no mira dentro, y `ConditionEngine` sí las
  usa.
  Esta misma semana se le borró `initiativeBonus()`, quinta copia huérfana de
  «iniciativa = DES» (`7298518`).
- **Cómo optimizarla.** 174 líneas de cuerpo para un almacén es mucho, y la razón
  es que todavía hay reglas dentro: `stat()` clampa, `healthCapWith()` aplica la
  fórmula del equipo, `repPriceModPct()` calcula un precio. C-20 ya dictaminó el
  principio para las condiciones; falta aplicarlo a la facción y al equipo. Lo
  urgente no es eso, es que `grantMilestones` se llame desde algún sitio: una
  progresión que no progresa no se puede balancear.

---

## `include/RPG/Catalogs/RpgCatalogs.h` — 211 líneas · juego · header-only · **55 pruebas**

### `Catalog` (línea 53, 129 de cuerpo)

- **Qué hace.** Contenedor genérico de un catálogo: carga desde JSON validando que
  cada entrada traiga al menos uno de los campos declarados, indexa por id y
  expone consulta. Es la pieza que llama a `CatalogLoader<T>::from_json` y
  `expected_fields` (líneas 85 y 111).
- **Qué debería hacer.** Ocho criterios a la vez: D-01 («un carregador que no troba
  res, falla», con DEC-02 sobre el nombre canónico del campo), D-02, D-03, D-04,
  D-05 (con DEC-05), D-06, D-07 («els 21 catàlegs llegeixen tots els seus camps»)
  y E-16. Es la cabecera con más criterios del árbol.
- **Qué se lo impide.** Nada en la clase: 55 pruebas, la cobertura más alta del
  área y con diferencia. Lo que la limita es que sea **la única** pieza de los
  catálogos que el juego alcanza por nombre: los siete ficheros de
  especializaciones que hacen el trabajo de campo son invisibles para el
  instrumento (hallazgo 3), así que una auditoría automática los daría por muertos.
- **Cómo optimizarla.** 129 líneas de cuerpo en una plantilla con ocho criterios
  encima es el lugar donde una regresión cuesta más cara. La optimización real es
  de instrumentación, no de código: que el alcance sepa ver las especializaciones,
  porque hoy el 74 % del código de los catálogos aparece como inalcanzable sin
  estarlo.

---

## `include/RPG/EventCommand.h` — 208 líneas · juego · 4 pruebas · la base la da por MUERTA

### `CommandSpec` (80) y `CommandParamSpec` (71)

- **Qué hacen.** La especificación declarativa de una comanda y de sus parámetros:
  el vocabulario de E-04 en datos, no en código.
- **Qué deberían hacer.** E-04: «les comandes de les cinc famílies estan
  implementades i documentades a `FORMATO_AVENTURAS.md`», con DEC-30 (ir a otro
  mapa es terminal para el evento local).
- **Qué se lo impide.** Que de las cuatro fuentes de `Eventos/` solo
  `AnfitrionNarrativo.cpp` llegue al juego: `EventCommands.cpp`,
  `CondicionesYPaginas.cpp` y `ValidacionDeComandas.cpp` no los alcanza ningún
  ejecutable. O sea que las cinco familias se implementaron, se partieron en cuatro
  ficheros (EST-32) y tres de los cuatro no entran en el binario del jugador.
- **Cómo optimizarla.** Verificar contra el binario igual que en el hallazgo 2 y,
  si se confirma, enlazar. Es la diferencia entre «el juego de comandas existe» y
  «el juego de comandas llega al jugador».

### `ActivePageResult` (línea 159, 12 de cuerpo)

- **Qué hace.** El resultado de decidir qué página de un evento está activa. Gana
  la **última** cuyas condiciones se cumplen, y el motivo está escrito: las páginas
  se leen de arriba abajo como estados sucesivos (cofre cerrado, cofre abierto), así
  que si ganara la primera, añadir una página no cambiaría nada.
- **Qué debería hacer.** E-09, el puente editor→documento.
- **Qué se lo impide.** Su método `trustworthy` no lo llama el juego: el resultado
  sabe si es fiable y nadie se lo pregunta. Es el mismo patrón que `canSave`.
- **Cómo optimizarla.** Que quien consume el resultado consulte `trustworthy` antes
  de actuar, o que el tipo no lo ofrezca. Una bandera de fiabilidad que nadie lee es
  un fallo esperando a ser silencioso.

### `NarrativeCommandHost` (línea 189, 18 de cuerpo)

- **Qué hace.** El anfitrión de E-03 para las familias que solo tocan estado
  narrativo: banderas, variables, interruptores locales, condicional y espera. Es
  «la mitad del juego de comandas que no necesita sesión de partida», y por eso se
  pudo entregar entera.
- **Qué debería hacer.** E-04, y delegar en `SessionCommandHost` lo que necesite
  sesión.
- **Qué se lo impide.** Nada estructural. Delega de verdad (`AnfitrionNarrativo.cpp:126`
  llama a `m_fallback->waitResolved(command)`), que es lo correcto.
- **Cómo optimizarla.** Nada. Es el buen ejemplo del área: la mitad que se podía
  entregar se entregó, y la que no, se delegó.

---

## `include/RPG/Catalogs/CatalogLoader.h` — 192 líneas · juego · header-only · 14 pruebas

### `CatalogField` (línea 102, 4 de cuerpo)

- **Qué hace.** Un campo del contrato: los nombres JSON que `from_json` acepta para
  ese concepto (`names.front()` es el canónico, el que sale en los errores) y si su
  ausencia es error de carga.
- **Qué debería hacer.** D-01 con DEC-02: «el nom canònic d'un camp és el de
  l'asset; quan no coincideix amb el carregador, s'adapta el C++».
- **Qué se lo impide.** Nada. Es la pieza que hace que D-01 sea comprobable: un
  campo sin presencia se clasifica, y el `catch(...)` mudo desaparece.
- **Cómo optimizarla.** Nada. Cuatro líneas que sostienen siete paquetes.

### `CatalogLoader` (línea 180, 4 de cuerpo)

- **Qué hace.** La plantilla primaria: `from_json` y `expected_fields` que cada
  familia especializa en los siete `CatalogLoaders*.h`.
- **Qué debería hacer.** D-01.
- **Qué se lo impide.** Que la plantilla primaria no se pueda ver usada: el
  instrumento cuenta `CatalogLoader.h` como alcanzado y las siete especializaciones
  como muertas (hallazgo 3).
- **Cómo optimizarla.** Sin paquete propio. **Propuesta de asignación:** D-01, que
  es el paquete cuyo criterio describe exactamente este contrato.

---

## `include/RPG/Narrative/NarrativeBeat.h` — 184 líneas · juego · 4 pruebas

Sin paquete asignado en ninguna de sus seis clases. **Propuesta:** E-01/E-02 si el
guion de aventura tiene paquete, y si no, uno propio: es el modelo de datos del
Llibre VI del GDD y hoy no tiene criterio escrito contra el que cerrarse.

### `NarrativeCondition` (línea 33, 10 de cuerpo)

- **Qué hace.** Condición sobre flags. Las tres listas se combinan con AND entre sí;
  dentro, `allFlags` es AND, `anyFlags` es OR y `notFlags` es NOR. Vacía = siempre
  cierta, y el comentario explica por qué ese es el default correcto.
- **Qué debería.** Sin criterio. Debería, como mínimo, tener una tabla de verdad
  probada: tres listas y cuatro combinaciones son el sitio donde un error no se ve.
- **Qué se lo impide.** Cuatro pruebas para 184 líneas de cabecera.
- **Cómo optimizarla.** Casos por combinación, no por beat.

### `BeatTrigger` (64), `BattleRequest` (128), `SkillCheckRequest` (134), `NarrativeEffect` (148)

Cuatro estructuras pequeñas. `SkillCheckRequest` es la mejor documentada del área:
`skillId` fija el tamaño N de la pool desde `casting_stat`, y `cd` es la dificultad
Nd6 en escala 0.0..3.0 con la sección del GDD citada (§7.1). **Qué se lo impide:**
nada; **cómo optimizarla:** nada. `BattleRequest` con 5 líneas de cuerpo es la que
más hace con menos.

### `NarrativeBeat` (línea 162, 21 de cuerpo)

- **Qué hace.** La unidad de contenido: la «carta» del Llibre VI del GDD.
- **Qué debería hacer.** Sin criterio escrito. Es la pieza central de todo el
  sistema narrativo y no hay ningún «Fet quan» que la nombre.
- **Qué se lo impide.** Eso: que el plan no la cubra. Que la unidad de contenido
  del juego no tenga criterio de cierre es un hueco del plan, no del código.
- **Cómo optimizarla.** Asignarle paquete antes que optimizarla.

---

## `include/RPG/Definitions/SkillDefinition.h` — 180 líneas · juego + level_editor · 27 pruebas

### `SkillDefinition` (línea 77, 86 de cuerpo)

- **Qué hace.** La ficha de una habilidad o conjuro: magnitud por grado, stat de
  defensa, coste, recuperación, alcance y condición aplicada. 1:1 con `skills.json`.
- **Qué debería hacer.** D-02: «magnitud per grau, `defenseStat`, cost,
  recuperació, abast i condició aplicada es llegeixen de l'entrada. Dues cartes
  diferents donen números diferents».
- **Qué se lo impide.** Sin bloque de paquete en la cabecera, aunque D-02 la
  describe casi literalmente: el criterio existe y la cabecera no lo lleva, así que
  quien la toque no se entera. Tres métodos sin llamar: `magnitude_f` (juego y
  editor), `has_tag` y `tier_min_clamped` (editor).
- **Cómo optimizarla.** `tools/marcar_paquetes.py --marcar` para que D-02 aparezca
  en la cabecera. Es un cambio de una línea de mapa y evita que el criterio se
  pierda.

### `MagnitudePreset` (línea 167, 4 de cuerpo)

- **Qué hace.** Presets para pruebas y cartas generadas a mano. Su propio
  comentario dice que **no** son fuente de verdad para el juego (la fuente es
  `skills.json`) y que se mantienen por compatibilidad.
- **Qué debería hacer.** Nada: es compatibilidad declarada.
- **Qué se lo impide.** C-22 está impreso en esta misma cabecera y dice «el que no
  es fa servir surt». Un preset mantenido «por compatibilidad con el código que los
  usaba» es justo lo que C-22 retira.
- **Cómo optimizarla.** Contar los consumidores. Si son solo pruebas, se muda a
  `tests/`; si no queda ninguno, se borra.

---

## `include/RPG/Narrative/NarrativeState.h` — 169 líneas · juego · 25 pruebas

### `TimerState` (línea 46, 7 de cuerpo) y `NarrativeState` (línea 54, 114 de cuerpo)

- **Qué hacen.** Todo el estado narrativo de una partida como conjuntos de strings:
  flags, variables con nombre, interruptores locales y temporizadores.
- **Qué deberían hacer.** E-02: «cada esdeveniment té els seus interruptors locals
  A–D i el joc té variables numèriques amb nom. Tot va al guardat».
- **Qué se lo impide.** Nada grave: 25 pruebas y los tres métodos sin llamar
  (`clearLocalSwitches`, `clearVariable`, `hasVariable`) son de limpieza y consulta.
  La decisión de diseño está escrita y es la correcta: strings y no enums porque
  «el contenido tiene que poder inventar flags nuevas sin recompilar el motor; una
  aventura escrita mañana declara `rumor_bardo` y funciona».
- **Cómo optimizarla.** Verificar la mitad de E-02 que no se ve desde la clase:
  «tot va al guardad». Si `SaveGameData` no incluye interruptores locales y
  variables, E-02 está a medias por mucho que `NarrativeState` esté bien.

---

## `include/RPG/Definitions/*` — las cinco cabeceras de fichas

Cinco cabeceras, 17 estructuras, todas header-only, todas **sin bloque de paquete**
aunque D-02..D-07 las describan una por una. Es el mismo defecto de mapa que en
`SkillDefinition.h`, repetido cinco veces, y se arregla con una pasada de
`marcar_paquetes.py`.

| Cabecera | Clases | Pruebas | Criterio que debería llevar |
| --- | --- | ---: | --- |
| `DefinicionesContenido.h` (163) | `AdventureDefinition`, `EventDefinition`, `LocationDefinition`, `EncounterTableDefinition` | 5 | D-07. `LocationDefinition` cierra además la brecha B24 de GAMEMACHINE_NECESIDADES y su fuente está nombrada (`dndWeebCC/data/mapa/geografia.json`) |
| `DefinicionesCriaturas.h` (154) | `MonsterDefinition` (432), `SummonDefinition` (201), `MountDefinition` (50) | 10 | D-03: «cap monstre del catàleg es carrega amb vida zero» |
| `DefinicionesObjetos.h` (150) | `ConditionDefinition` (16), `EquipmentDefinition` (413), `ConsumableDefinition` (31) | 15 | D-04 y D-05, con DEC-02 y DEC-05 |
| `DefinicionesRasgos.h` (133) | `PassiveDefinition` (66), `FeatDefinition` (6), `TraitDefinition` (2), `DeityDefinition` (10) | 4 | D-06/D-07 |
| `DefinicionesPersonaje.h` (131) | `ClassDefinition` (61), `RaceDefinition` (43), `BackgroundDefinition` (50) | 13 | D-06 |

**Juicio conjunto.** El modelo de datos es la parte más sana del área: fichas
1:1 con los assets, con el recuento de cartas en el comentario, header-only para no
recompilar el mundo, y el paraguas partido por familias en EST-30 (de 687 líneas a
17) precisamente para que añadir un campo a una criatura no recompile el
inventario. Es una decisión de rendimiento correcta y documentada donde se lee.

Dos cosas lo ensucian. Primera: **`ConditionDefinition.mechanicHooks` es un
vertedero deliberado** — «el cargador mapea a los campos de arriba SOLO los que
tienen traducción unívoca; el resto queda aquí, machine-readable, para D-11, E-14 y
G-13». Está bien como transición y mal como estado: tres paquetes distintos
dependen de lo que haya en ese vector de strings, y ninguno lo ha vaciado. Los
modificadores de defensa retirados en D-04 viven ahí como `bono_ca:2` sin ejecutor
tipado. Segunda: `ClassDefinition::primary_stat_enum` y `secondary_stat_enum` no
los llama ningún ejecutable, y son los que traducen el string del asset al enum del
motor — si nadie los llama, la traducción la está haciendo otro por su cuenta.

---

## `include/RPG/NarrativeEngine.h` — 152 líneas · juego · 14 pruebas

### `NarrativeResult` (línea 75, 21 de cuerpo)

- **Qué hace.** El resultado de un disparo, con `fired = false` cuando ningún beat
  encajaba, que es lo que deja al NPC decir su diálogo de catálogo.
- **Qué debería hacer.** E-02 / G-07.
- **Qué se lo impide.** Nada. La bandera negativa está pensada y explicada.
- **Cómo optimizarla.** Nada.

### `NarrativeEngine` (línea 97, 54 de cuerpo)

- **Qué hace.** Dispara beats por trigger y target contra un `NarrativeState`, y
  ofrece `talkTo()`/`enterLevel()` como azúcar sobre `fire()`.
- **Qué debería hacer.** E-02 y G-07 (congelación de esquemas).
- **Qué se lo impide.** `fire`, `startTimer` y `allObjectivesDone` aparecen sin
  llamar desde el juego. `fire` es un **candidato, no un veredicto**: `talkTo()` y
  `enterLevel()` son inline y la llaman, así que el instrumento la cuenta como uso
  interno. Lo que sí conviene comprobar es si el juego llama a `talkTo`/`enterLevel`
  o solo a `setAdventure()` — que es lo único que `git grep` encuentra en
  `Application.cpp:351-361`. Si es así, el motor narrativo está cargado y no
  disparado. `startTimer` sin llamar es más claro: los `TimerDefinition` del guion
  existen y nadie los arranca.
- **Cómo optimizarla.** La comprobación es de una línea y decide si E-02 está
  cerrado o solo escrito.

---

## `include/RPG/CharacterCreation.h` — 144 líneas · juego · 8 pruebas · la base la da por MUERTA

### `CreationChoice` (línea 52, 13 de cuerpo) y `CharacterCreation` (línea 66, 77 de cuerpo)

- **Qué hacen.** El asistente de creación: lo que elige el jugador (stats base,
  antes de la raza) y el cálculo que los combina con raza, clase y trasfondo.
- **Qué deberían hacer.** Sin paquete propio; le aplica G-09, que es el de su
  pantalla.
- **Qué se lo impide.** Su alcance viene de `include/Engine/PantallaNuevaPartida.h`,
  y la guardia avisa de que **`src/Engine/PantallaNuevaPartida.cpp` no está
  versionado**: «configura en el arbol de quien lo escribio y en ningun clon». O sea
  que la creación de personaje llega al jugador en esta máquina y no llega en un
  clon. `statsFinales` y `OpcionesTrasfondo::vacio` sin llamar.
- **Cómo optimizarla.** Nada en la clase. El arreglo es de protocolo: versionar ese
  `.cpp` con su línea de CMake en el mismo commit, que es lo que la propia guardia
  pide.

---

## `include/RPG/CharacterCreationScreen.h` — 138 líneas · juego · 5 pruebas · la base la da por MUERTA

### `CharacterCreationScreen` (línea 41, 96 de cuerpo)

- **Qué hace.** La pantalla del asistente: pasos, navegación con volver/cancelar,
  resumen, y edición de nombre y filtro de listas.
- **Qué debería hacer.** G-09: «Nova partida obre l'assistent existent, amb
  tornar/cancel·lar i resum. Confirmar crea la sessió real amb fitxa, trets, equip
  i habilitats del projecte… sense fitxers intermedis que el jugador hagi de copiar
  a mà».
- **Qué se lo impide.** **Ocho métodos sin llamar**, y son todos de la misma mitad:
  `anterior`, `escribirEnNombre`, `borrarDelNombre`, `escribirEnFiltro`,
  `borrarDelFiltro`, `limpiarFiltro`, `filtro`, `totalSinFiltrar`. O sea: el «tornar»
  de G-09 y toda la edición de texto y de filtros. La mitad que sí se usa es la de
  avanzar. Y su único alcanzador es el `.cpp` sin versionar del punto anterior.
- **Cómo optimizarla.** 96 líneas de cuerpo para una pantalla cuyos métodos de
  navegación hacia atrás nadie llama: o el cableado del input está a medias, o la
  pantalla se maneja por otro camino y estos métodos sobran. G-09 no puede estar
  cerrado mientras `anterior` no tenga consumidor, porque «amb tornar/cancel·lar»
  está en el criterio literal.

---

## `include/RPG/EnhanceHooks.h` — 137 líneas · juego + level_editor · 5 pruebas

### `ExecutionContext` (línea 57, 42 de cuerpo)

- **Qué hace.** Los parámetros de entrada a la ejecución de una habilidad.
- **Qué debería hacer.** D-09 y C-22.
- **Qué se lo impide.** Nada: cero métodos sin llamar.
- **Cómo optimizarla.** 42 líneas de parámetros es una struct ancha; si crece, es
  señal de que la ejecución está recibiendo contexto que debería pedir al
  catálogo.

### `SkillExecutor` (línea 101, 35 de cuerpo)

- **Qué hace.** El orquestador final de la ejecución de una habilidad, que respeta
  el invariante Nd6 siempre.
- **Qué debería hacer.** D-09: «una carta que enverina, enverina. El resultat de
  l'execució diu quina condició cal aplicar i amb quina durada, i el combat
  l'aplica», con DEC-28 (banco de casos validado por Oriol cuando el motor y las
  cartas Java diverjan). Y D-11 en la pieza que ejecuta: «un efecte no suportat es
  rebutja amb diagnòstic, sense gastar cost ni torn, en comptes d'aplicar-se a
  mitges».
- **Qué se lo impide.** Cinco pruebas para el orquestador final de todo el sistema
  de habilidades. Y la pregunta abierta del hallazgo 6: si `resolve_opposed` no se
  llama, ¿qué está orquestando este ejecutor en un ataque?
- **Cómo optimizarla.** Es el sitio donde DEC-28 pide un banco de casos con
  resultado esperado. Cinco pruebas no son ese banco.

---

## `include/RPG/TierRules.h` — 114 líneas · juego + level_editor · 16 pruebas

### `TierRule` (línea 50, 18 de cuerpo)

- **Qué hace.** Una fila del array `tiers` de `assets/rules/tier_rules.json`, 1:1
  con los campos del JSON; `_optional` marca lo que el JSON puede no traer.
- **Qué debería hacer.** C-22, que lleva impreso.
- **Qué se lo impide.** Nada.
- **Cómo optimizarla.** Es la cabecera donde vive la regla canónica de iniciativa
  («§5: Iniciativa = DES + Modificadores», línea 93 del JSON) que nadie implementa
  y ningún paquete menciona. **Propuesta:** que el paquete que reclame la iniciativa
  se asigne aquí, que es donde ya está la regla escrita.

### `TierRules` (línea 75, 38 de cuerpo)

- **Qué hace.** La fuente única de «qué significa un tier»: `GameSession` la posee
  y la carga desde JSON, y `CharacterSheet`/`DicePoolEngine` la toman por referencia
  constante. Singleton semántico, no singleton C++, con el motivo escrito.
- **Qué debería hacer.** C-22.
- **Qué se lo impide.** `at` sin llamar desde `level_editor`. Y el diseño es
  correcto: «lo importante es que el qué tier significa sea una única fuente de
  verdad, no hardcodear límites por ahí (respeta fractura #1)».
- **Cómo optimizarla.** Nada. Es de las pocas piezas del área que hacen justo lo
  que dicen y tienen el motivo escrito.

---

## `include/RPG/InventoryEngine.h` — 108 líneas · juego · 4 pruebas

### `InventoryEngine` (línea 24, 83 de cuerpo)

- **Qué hace.** Equipar y desequipar sobre `CharacterSheet.equippedIds[]`, con los
  límites de peso por clase y de rareza por tier, y el recálculo de bonificadores.
- **Qué debería hacer.** D-05: «els tres catàlegs llegeixen els seus camps i
  `InventoryEngine` aplica cada bonificador una sola vegada, distingint escut de
  qualsevol altre objecte a la mà esquerra».
- **Qué se lo impide.** **Siete métodos sin llamar**, y `recalc_equipment_bonuses`
  está **verificado**: `git grep` solo encuentra su definición
  (`InventoryEngine.cpp:189`), ni siquiera una llamada interna. Es la función que
  D-05 nombra explícitamente («aplica cada bonificador una sola vegada») y no la
  ejecuta nadie. Con ella, `can_equip`, `check_weight`, `check_rarity`,
  `check_symbols`, `resolve_slot` y `slot_name`: o sea, **todos los límites de
  equipamiento**. Cuatro pruebas para 83 líneas de cuerpo.
- **Cómo optimizarla.** Esto no se optimiza, se cablea o se retira. Un motor de
  inventario cuyos siete métodos de reglas no tienen consumidor es el mismo caso
  que `DynamicLightMap` sin luces: la pieza existe, está probada en aislamiento y
  no llega a ninguna partida. Y es el tercer motor de comercio/inventario del que
  se tiene noticia, junto a `GameSession::buyOffer` y `Game::ShopEngine`.

---

## `include/RPG/SkillEffect.h` — 100 líneas · juego + level_editor · 2 pruebas

### `SkillEffectDef` (línea 61, 15 de cuerpo) y `EffectFamily`/`ScalingStat`

- **Qué hacen.** El efecto resuelto de un campo `effect_scaling` del JSON, con su
  familia y su stat de escalado.
- **Qué deberían hacer.** D-11: «cada família del vocabulari té contracte tipat,
  executor i prova pròpia… un efecte no suportat es rebutja amb diagnòstic, sense
  gastar cost ni torn».
- **Qué se lo impide.** Dos pruebas. Y el criterio pide **una prueba propia por
  familia**: si hay N familias y dos pruebas, el criterio no se cumple por
  construcción. Cero métodos sin llamar, lo cual es buena señal.
- **Cómo optimizarla.** Contar las familias de `EffectFamily` y escribir el caso de
  cada una más el de rechazo con diagnóstico. Es el criterio más comprobable del
  área y el que menos cubierto está.

---

## `include/RPG/ConditionEngine.h` — 94 líneas · juego · 5 pruebas

### `ConditionEngine` (línea 20, 73 de cuerpo)

- **Qué hace.** El único escritor de las reglas de condiciones sobre
  `CharacterSheet`: aplicar respetando `Stacking`, quitar, purgar y ticar,
  recalculando los modificadores desde el catálogo. `tick_conditions` calcula
  efectos pero **no cambia PV**: el propietario de la vida aplica el efecto y
  notifica la pérdida real con `on_damage_received`.
- **Qué debería hacer.** C-20, que es el criterio que hizo borrar
  `applyCondition`/`removeCondition`/`tickEndOfRound` de la ficha:
  «`ConditionEngine` és l'únic camí… `purge_expired` purga de veritat i
  `breaksOnDamage` es dispara pel dany rebut, no per l'infligit».
- **Qué se lo impide.** Ocho métodos en la lista del instrumento, y **es el mejor
  ejemplo de por qué la lista hay que comprobarla**: `purge_expired` sí se llama,
  cuatro veces, desde dentro de su propio `.cpp` (líneas 105, 184, 202, 226). Los
  siete restantes (`active_stacks`, `grants_advantage_to_enemies`,
  `grants_disadvantage_to_caster`, `has_half_speed`, `has_prevent_action`,
  `has_speed_zero`, `recalc_condition_modifiers`) son candidatos: son las consultas
  de los flags de comportamiento que `ConditionDefinition` declara, y si el combate
  no las pregunta, los flags se parsean y no se aplican — que es literalmente lo
  que D-04 prohíbe («els modificadors que avui es parsegen i s'ignoren, o
  s'apliquen o s'esborren»).
- **Cómo optimizarla.** Verificar uno por uno los siete contra `BattleState`. La
  decisión de que el motor no toque PV y devuelva efectos es correcta y está
  escrita; lo que falta es que alguien los lea.

---

## `include/RPG/SessionCommandHost.h` — 91 líneas · juego · 2 pruebas · la base la da por MUERTA

### `SessionCommandHost` (línea 32, 29 de cuerpo)

- **Qué hace.** La mitad del anfitrión de comandas que sí necesita sesión de
  partida: inventario, oro y combate.
- **Qué debería hacer.** E-04, complementando a `NarrativeCommandHost`.
- **Qué se lo impide.** `requestedTransition`, `clearRequestedTransition` y
  `lastBattleOutcome` sin llamar. Los tres son de la misma cosa: DEC-30 dice que
  «la comanda d'anar a un altre mapa és terminal per a l'esdeveniment local», y esa
  transición se pide y se consulta por estos métodos. Si nadie los lee, la comanda
  de cambiar de mapa se ejecuta y la transición no ocurre. Dos pruebas para el
  anfitrión que toca la sesión.
- **Cómo optimizarla.** Comprobar DEC-30 de punta a punta: comanda de ir a otro
  mapa → `requestedTransition` → el juego cambia de mapa → el evento local termina
  y lo de debajo recibe diagnóstico de inalcanzable. Es un criterio aceptado y
  tiene dos pruebas.

---

## `include/RPG/RandomEngine.h` — 87 líneas · juego + level_editor · header-only · 5 pruebas

### `RandomEngine` (línea 31) y `Xoroshiro128p` (línea 46, 40 de cuerpo)

- **Qué hacen.** La interfaz de azar inyectable y su implementación Xoroshiro128+,
  header-only para no añadir unidades de compilación. **No** es un singleton
  global, y el motivo está escrito: así las pruebas pueden pasar una semilla fija y
  reproducir.
- **Qué deberían hacer.** C-21: «una sola interfície d'atzar al motor. La que no es
  fa servir surt del repositori».
- **Qué se lo impide.** `roll_in_range` y `seed` sin llamar en juego **y** editor.
  `seed` es el que duele: la documentación promete «permite reproducir partidas
  guardando seed», y reproducir una partida requiere poder fijar la semilla. Si
  nadie llama a `seed()`, las partidas no son reproducibles y las pruebas que
  dependen de azar no son deterministas por construcción.
- **Cómo optimizarla.** C-21 pide retirar lo que no se usa y aquí hay dos candidatos
  en la cabecera que lleva el criterio impreso. Pero antes de retirar `seed` hay que
  responder si la reproducibilidad es un requisito: si lo es, falta cablearla y
  guardarla en `SaveGameData`; si no lo es, se borra y se quita la promesa del
  comentario. Lo que no puede quedar es la promesa escrita y el método sin llamar.

---

## `include/RPG/Defense.h` (44 · 30 pruebas), `Stat.h` (37 · 39 pruebas), `SkillTarget.h` (27 · 10 pruebas)

Las tres header-only, las tres sin bloque de paquete, y las tres con la ratio más
alta de pruebas por línea de todo el árbol.

- **`DefenseBlock`** (línea 29, 14 de cuerpo) — las cuatro defensas. Sus cuatro
  accessors (`ca_fisica`, `dc_mag`, `save_fis`, `save_vol`) no los llama ningún
  ejecutable, y el motivo está escrito en `ICombatant.h`: «Ed. 3 resuelve ataques
  enfrentando pools de stats, **sin convertir estos valores en dificultad**». O sea
  que no son un descuido: son un bloque **legacy** «para datos y HUD», con 30
  pruebas. **Propuesta de asignación:** C-22, porque es API que se mantiene por
  compatibilidad y C-22 es el paquete que decide qué sale.
- **`Stat`** — el enum de los cuatro stats. 39 pruebas para 37 líneas: la pieza más
  probada del árbol. **Propuesta:** D-06.
- **`SkillTarget`** — el enum de objetivo. **Propuesta:** D-02.

**Juicio conjunto.** Son el suelo sobre el que todo lo demás se apoya y están bien:
pequeñas, puras, header-only y muy probadas. Lo que piden no es código sino mapa:
que `marcar_paquetes.py` les ponga el bloque, porque hoy tres de las piezas más
usadas del motor no dicen de qué paquete son.

---

## Las 13 cabeceras que no alcanza ningún ejecutable

Ocho son falsos negativos del instrumento (hallazgo 3) y cinco son reales.

| Cabecera | Líneas | Pruebas | Veredicto |
| --- | ---: | ---: | --- |
| `Catalogs/CatalogLoadersMundo.h` | 297 | 0 | **Falso negativo**: especializaciones de plantilla que `RpgCatalogs.h:85,111` instancia |
| `Catalogs/CatalogLoadersRasgos.h` | 268 | 0 | Ídem |
| `Catalogs/CatalogLoadersCriaturas.h` | 265 | 0 | Ídem |
| `Catalogs/CatalogLoadersObjetos.h` | 258 | 0 | Ídem |
| `Catalogs/CatalogLoadersPersonaje.h` | 223 | 0 | Ídem |
| `Catalogs/CatalogLoadersHabilidades.h` | 208 | 0 | Ídem |
| `Catalogs/CatalogLoadersContenido.h` | 191 | 0 | Ídem |
| `Definitions/RpgCoreDefinitions.h` | 17 | 0 | **Falso negativo**: paraguas puro, sin símbolos propios, con 15 consumidores |
| `Audio.h` | 131 | 1 | **Real**: cero símbolos en el binario del juego |
| `AdventureValidation.h` | 130 | 2 | **Real**: ídem; dos `.cpp`, ninguno enlazado |
| `EventosInterno.h` | 49 | 0 | **Parcial**: de sus cuatro consumidores en `src/RPG/Eventos/`, solo `AnfitrionNarrativo.cpp` llega al juego |
| `AdventureSerializer.h` | 39 | 2 | **Real**: cero símbolos en el binario |
| `LootResolver.h` | 21 | 1 | **Real**: cero símbolos en el binario |

Las siete primeras suman **1.710 líneas** que una auditoría automática daría por
muertas. Es el argumento más concreto que existe hoy para no re-congelar la línea
base sobre este instrumento tal como está.
