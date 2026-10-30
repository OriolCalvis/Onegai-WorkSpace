# 03 · Game — juicio clase a clase

**Ámbito.** Las 26 cabeceras y 57 clases de `include/Game/` que alcanza el
ejecutable `juego` (ninguna llega a los dos editores), más las 4 que no alcanza
ninguno. Es la sección `Game` de `planificacion/MAPA_CLASES_JUEGO.md`.

**Una nota sobre el área.** De las 26 cabeceras, **19 no llevan bloque PAQUETES
DEL PLAN**: es el área con más criterio ausente del juego, y contiene la clase más
grande del árbol. Nueve están sucias con WIP vivo hoy, así que buena parte de lo
que sigue se juzga y no se toca.

---

## Los cinco hallazgos del área

### 1. `GameSession` está a dos líneas de su techo y no puede crecer

`GameSession.h` tiene **544 líneas y la guardia le marca `techo 546`**. O sea:
cualquier cambio que añada dos líneas a la cabecera más importante del juego la
pone en rojo. No es un aviso de estilo, es un bloqueo físico del trabajo, y es la
respuesta literal a «qué se lo impide» para la clase central del área.

Detrás del número: **469 líneas de cuerpo de clase** y **seis ficheros de
implementación** (`GameSession.cpp`, `Comercio`, `Interaccion`, `Inventario`,
`Narrativa`, `SaveGame`). Partirla por ficheros no la partió por
responsabilidades: las seis partes siguen siendo miembros de la misma clase y
cuentan contra la misma cabecera. La división real que ya funciona en el área es
otra y está escrita en `SessionTipos.h`: `LevelTransition` «la sesión decide, el
de fuera orquesta». Falta aplicar ese mismo criterio hacia dentro.

### 2. Cuatro clases de Game no llegan a ningún ejecutable, y tres tienen gemela viva

| Muerta | Líneas | Pruebas | La gemela que sí llega |
| --- | ---: | ---: | --- |
| `TitleScreen.h` | 58 | 1 | `TitleMenuState.h` (108, 4 pruebas), alcanzada desde `Application.cpp` |
| `BattleMenuState.h` | 78 | 1 | `BattleMenuActions.h` (88), alcanzada desde `Application.cpp` |
| `ShopEngine.h` | 70 | 1 | `GameSessionComercio.cpp`, uno de los seis `.cpp` de `GameSession` |
| `Companeros.h` | 45 | 1 | — **SUCIO**, y su `.cpp` no está versionado |

El patrón es el mismo tres veces: se escribió una segunda versión de la pantalla o
del motor, la nueva se cableó y la vieja se quedó con su prueba en verde. `ctest`
no puede delatarlo porque las dos compilan y las dos pasan; solo el alcance lo ve.
`TitleScreen` está referenciada únicamente desde su propio `.cpp`, verificado con
`git grep`. Y `ShopEngine` es el segundo motor de comercio: el bueno es el de
`GameSession`, que sí llega al jugador.

### 3. El combate no tiene iniciativa, y tres sitios la definen

`BattleState.h:24` lo dice sin rodeos: «Cola de turnos deliberadamente simple
(**sin velocidad/iniciativa** — ver …)». Y sin embargo:

- `ICombatant::initiative_bonus()` existe, con base `stat(DES)` y la nota «Pasivas
  pueden sobreescribir», y 30 pruebas en su cabecera.
- `assets/rules/tier_rules.json:93` escribe la regla canónica: «§5: Iniciativa =
  DES + Modificadores».
- Hasta esta semana había una quinta copia en `CharacterSheet::initiativeBonus()`
  (borrada en `7298518`).

O sea que la iniciativa está definida en el reglamento, en la interfaz y en los
datos, y **no existe en el único sitio que la necesitaría**. No es un descuido de
código: es una decisión de diseño («deliberadamente simple») que nadie anotó donde
están las otras tres. O se implementa la cola por iniciativa, o se escribe en
`tier_rules.json` y en `ICombatant.h` que la regla §5 no se aplica al combate por
turnos. Lo que no puede seguir es la regla escrita en tres sitios y aplicada en
ninguno.

### 4. `BattleState`, la máquina de combate, no tiene criterio de cierre

`BattleState.h` (213 líneas, 9 pruebas, 128 de cuerpo en la clase principal) **no
lleva bloque de paquete**. Es la pieza que resuelve los turnos, aplica efectos,
gestiona invocaciones y produce el log que lee el HUD, y no hay ningún «Fet quan»
en `PLA_DEFINITIU.md` que la nombre. **Propuesta de asignación:** C-13 o el paquete
de combate de la fase H, y mientras tanto DEC-28, que es la decisión que dice quién
arbitra cuando el motor y las cartas Java divergen.

### 5. `Skill.h` tiene un catálogo propio, paralelo al de D-02

`Game::SkillCatalog` (línea 125, 13 de cuerpo) y `RPG::Catalogs::SkillCatalog`
son dos cosas distintas con el mismo nombre. El de `Game` se documenta como
«quien construya el contenido del juego rellena aqui las habilidades disponibles
(desde JSON, a mano, o mas adelante desde su propio archivo — fuera del alcance de
esta iteracion)», y explica por qué vive junto a `Skill` y no en `Core::Resources`.
La explicación es razonable y el motivo de no usar `ResourceManager<T>` es correcto;
el problema es que **D-02 cierra sobre los catálogos de `RPG::Catalogs`** con 55
pruebas, y este otro catálogo no entra en ese criterio. `EditorValidation.h` lo
alcanza «por SkillCatalog», así que el editor valida contra uno y el juego carga el
otro.

---

## `include/Game/GameSession.h` — 544 líneas · juego · 22 pruebas · **techo 546** · **SUCIO**

### `GameSession` (línea 76, 469 de cuerpo)

- **Qué hace.** El estado de una partida en curso: el pegamento entre nivel JSON,
  catálogos, habilidades y combate por turnos, y la transición automática
  exploración↔combate. Posee `EnemyBrain`, `TierRules`, la moral, los negocios, el
  oro y el modo de juego. Seis `.cpp` la implementan por temas: comercio,
  interacción, inventario, narrativa y guardado.
- **Qué debería hacer.** C-03: «moral, negocis, passos des de l'últim cobrament i
  or es transfereixen sencers entre sessions. El traspàs és **una sola funció**, no
  una llista de camps escampada per `loadLevel`».
- **Qué se lo impide.** El techo (hallazgo 1) y ocho métodos sin llamar. De ellos,
  tres verificados y significativos: **`repartirBotin`** (repartir botín) no tiene
  consumidor, y `LootResolver` tampoco llega al binario — el botín está sin cablear
  por los dos extremos; **`talkNarrative`** tampoco, que es coherente con que
  `NarrativeEngine::fire` no se llame: hablar no dispara beats; y **`narrativeState`**
  sin llamar, o sea que el estado narrativo de la partida no se consulta desde
  fuera. `collectRent` aparece en la lista pero el propio `medir_alcance.py` la usa
  como ejemplo de método que sí se llama internamente: **falso positivo conocido**.
  `dialogueSpeaker`, `equipmentIdFor`, `lastPickupId` y `objectIdForEquipment`
  quedan como candidatos sin verificar.
- **Cómo optimizarla.** Es la pregunta más importante del área y no tiene respuesta
  de código corto. Lo que sí se puede decir con datos: 469 líneas de cuerpo, seis
  `.cpp`, 22 pruebas y un techo a dos líneas. La partición por ficheros ya no da más
  de sí porque no reduce la clase. Dos caminos reales: (a) que cada uno de los seis
  temas sea un colaborador con su propio estado (el patrón `Morality`, que ya existe
  en el área y cuya cabecera explica por qué se sacó fuera: «teniendo el concepto en
  un sitio, añadir una fuente nueva es una llamada a `record()`, no tocar la lógica
  de partida»), o (b) subir el techo, que es decidir que la clase va a seguir
  creciendo. (a) es lo que C-03 y la fractura #1 piden; (b) es lo que está pasando
  de hecho.

---

## `include/Game/EnemyBrain.h` — 295 líneas · juego · 7 pruebas

### `EnemyBrain` (línea 54, 242 de cuerpo)

- **Qué hace.** Lo que un enemigo **es**, sin nada de cómo se dibuja: vida, estado
  de IA y patrulla. GL-free a propósito —ni `TextureAtlas`, ni `SpriteBatch`, ni
  `Entity`— para que `GameSession`, que tampoco toca GL, pueda poseerlo y probarse
  sin ventana.
- **Qué debería hacer.** D-08: «un enemic del món es construeix des de la seva
  `MonsterDefinition`: estadístiques, defenses, tier i habilitats. El combat Nd6 té
  números diferents per a monstres diferents».
- **Qué se lo impide.** Nada estructural: cero métodos sin llamar, y su cabecera
  documenta que resuelve la fractura #1 de `ARCHITECTURE.md` porque «había DOS
  representaciones». Es el arreglo bien hecho de la duplicación que `Render/Enemy.h`
  dejó a medias: `EnemyBrain` es la versión viva y `Enemy` la muerta.
- **Cómo optimizarla.** 242 líneas de cuerpo es mucho para una IA de patrulla y
  combate. El corte natural ya existe en el área (`EnemyBrainData.h`, 48 líneas,
  5 pruebas): el estado está fuera, así que lo que queda dentro es comportamiento.
  Si la IA crece (D-08 pide habilidades por monstruo), conviene separar la decisión
  de la ejecución antes que añadir más ramas.

---

## `include/Game/BattleState.h` — 213 líneas · juego · 9 pruebas · **SUCIO** · sin paquete

### `BattleAction` (línea 42, 20 de cuerpo)

- **Qué hace.** La acción de combate resuelta: quién, qué, sobre quién y con qué
  resultado.
- **Qué debería hacer.** Sin criterio (hallazgo 4). Debería, como mínimo, ser lo
  que DEC-28 compare contra las cartas Java.
- **Qué se lo impide.** Nada en la estructura.
- **Cómo optimizarla.** Nada.

### `BattleParticipant` (línea 73, 12 de cuerpo)

- **Qué hace.** Un participante: el `ICombatant` real (Player/Enemy/doble de
  prueba), su `SkillSet` (nullable, porque un enemigo «solo ataca básico» no tiene)
  y un nombre para el log. Ningún puntero es propietario.
- **Qué debería hacer.** Sin criterio.
- **Qué se lo impide.** Nada. La decisión de no poseer está escrita y es la misma
  que en el resto del motor.
- **Cómo optimizarla.** Nada. 12 líneas bien puestas.

### `BattleState` (línea 86, 128 de cuerpo)

- **Qué hace.** La máquina de combate por turnos: cola simple, resolución de
  acciones, invocaciones pendientes y el log de texto del que lee el cuadro de
  diálogo del HUD.
- **Qué debería hacer.** Sin paquete. **Propuesta:** el paquete de combate de la
  fase H, con DEC-28 como árbitro y D-11 para los efectos. Y DEC-09 en la mitad que
  le toca: guardar a mitad de combate obligaría a restaurar el combate a medias.
- **Qué se lo impide.** La cola de turnos «deliberadamente simple (sin
  velocidad/iniciativa)» frente a la regla §5 escrita en tres sitios (hallazgo 3).
  `pendingSummons` sin llamar: las invocaciones se acumulan y nadie las recoge, y
  `SummonDefinition` son 201 cartas en el catálogo. Está sucio (`GameSession.h`,
  `InventoryMenuState.h` y esta cabecera son el WIP vivo de EST-23b).
- **Cómo optimizarla.** 9 pruebas para 128 líneas de cuerpo en el corazón del
  combate es poco margen, y el criterio que falta es justo el que diría cuántas
  hacen falta. Lo primero no es optimizar: es asignarle paquete.

---

## `include/Game/Skill.h` — 194 líneas · juego + level_editor · **31 pruebas**

### `Skill` (línea 39, 18 de cuerpo)

- **Qué hace.** Una habilidad: id, nombre, coste, objetivo y efecto.
- **Qué debería hacer.** C-01: «una habilitat amb `target = Ally` aplica el seu
  efecte sobre el jugador; una amb `target = Enemy`, sobre l'enemic seleccionat. El
  PM es gasta una sola vegada i només si l'efecte s'ha pogut aplicar».
- **Qué se lo impide.** Nada. C-01 es el criterio mejor formulado del área: dos
  comportamientos y una condición de gasto, todos comprobables.
- **Cómo optimizarla.** Nada.

### `SkillSet` (línea 64, 46 de cuerpo)

- **Qué hace.** Las habilidades que conoce una entidad más su reserva de maná. No
  posee las `Skill`: referencia por id a un catálogo externo, igual que
  `EnemySpawn` no duplica los stats del tipo de enemigo.
- **Qué debería hacer.** C-01 en la mitad del gasto de PM.
- **Qué se lo impide.** `setMaxMana` sin llamar. Es el setter del valor que C-01
  protege por el lado del gasto: si nadie fija el máximo, el máximo es el de
  construcción.
- **Cómo optimizarla.** La decisión de no duplicar datos está escrita y es la
  correcta. `setMaxMana` o se llama al construir el personaje (que sería el sitio
  natural, junto a `recompute_derived`) o se borra.

### `SkillCatalog` (línea 125, 13 de cuerpo)

- **Qué hace.** Catálogo de `Skill` por id, que quien construye el contenido rellena.
- **Qué debería hacer.** D-02, si fuera el catálogo canónico. No lo es: es el
  paralelo (hallazgo 5).
- **Qué se lo impide.** Que existan dos. El de `RPG::Catalogs` carga `skills.json`
  con contrato de campos y 55 pruebas; este se rellena a mano o desde JSON sin
  contrato.
- **Cómo optimizarla.** Retirarlo en favor del de `Catalogs`, o escribir aquí por
  qué el juego necesita uno distinto. Lo que no puede seguir es que el editor
  valide contra uno y el juego cargue el otro.

### `SkillApplyResult` (línea 143, 19 de cuerpo)

- **Qué hace.** El resultado enriquecido de aplicar un efecto: el grado de éxito
  Nd6 (BOTCH/PARTIAL/SUCCESS/CRITICAL) y la magnitud final realmente aplicada, para
  que `BattleState` pueda registrar «(Parcial: 5 de dano)» en vez de «10».
- **Qué debería hacer.** C-01 y D-11 («un efecte no suportat es rebutja amb
  diagnòstic, sense gastar cost ni torn»).
- **Qué se lo impide.** Nada. Es el tipo correcto para que D-11 sea comprobable:
  el diagnóstico viaja en el resultado.
- **Cómo optimizarla.** Nada. Buena pieza.

---

## `include/Game/SaveGame.h` — 174 líneas · juego · 7 pruebas · sin paquete

### `SavedWorldObject` (línea 47, 4 de cuerpo)

- **Qué hace.** Un objeto que sigue en el mundo. Los recogidos no se guardan: **su
  ausencia es el dato**. La identidad es la posición, no el índice, con el motivo
  escrito: el motor ya trata la celda como identidad (`World::findObjectAt` busca
  por `(x,y)`) y los props y pickups no se mueven.
- **Qué debería hacer.** G-07 (congelación de esquemas a 1.0-B) si se le asigna;
  hoy no tiene paquete.
- **Qué se lo impide.** Nada.
- **Cómo optimizarla.** Nada. Cuatro líneas con una decisión de identidad justificada
  son mejor que cuarenta sin ella.

### `SavedEnemy` (línea 59, 17 de cuerpo)

- **Qué hace.** Un enemigo vivo; los derrotados no aparecen. Guarda la vida para que
  huir de un combate a medias no le regale al enemigo la vida que ya le habías
  quitado. Aquí la identidad **sí** es un índice (`WorldEnemy::spawnIndex`), porque
  los enemigos patrullan y su posición cambia.
- **Qué debería hacer.** Ídem.
- **Qué se lo impide.** Nada. Y la asimetría con `SavedWorldObject` no es
  inconsistencia: está motivada por el mismo criterio (qué se mueve y qué no).
- **Cómo optimizarla.** Nada. Es el mejor ejemplo del área de dos decisiones
  parecidas resueltas de forma distinta porque el problema es distinto.

### `SaveGameData` (línea 77, 57 de cuerpo)

- **Qué hace.** El paquete completo de una partida guardada.
- **Qué debería hacer.** G-07: esquema congelado a 1.0-B con ruta de migración
  documentada para cada cambio futuro. Y C-03 en lo que le toca: lo que es del
  jugador cruza intacto.
- **Qué se lo impide.** Sin paquete asignado, así que la congelación de G-07 no
  tiene esta cabecera en su lista de ficheros. Y la pregunta abierta del área: si
  `NarrativeState` (flags, variables, interruptores locales) no está aquí, E-02
  («tot va al guardat») no se cumple aunque la clase esté bien.
- **Cómo optimizarla.** 57 líneas de cuerpo para el esquema congelado es el sitio
  donde cada campo nuevo cuesta una migración. **Propuesta de asignación:** G-07,
  que es el paquete que ya habla de «dades RPG locals i metadades de slots».

---

## `include/Game/SaveSlotState.h` — 168 líneas · juego · 4 pruebas · la base la da por MUERTA

### `SaveSlot` (línea 73, 20 de cuerpo)

- **Qué hace.** Los metadatos de una ranura: personaje, mapa y fecha.
- **Qué debería hacer.** G-11: «slots separats per projecte amb personatge, mapa i
  data».
- **Qué se lo impide.** Nada.
- **Cómo optimizarla.** Nada.

### `SaveSlots` (línea 96, 51 de cuerpo)

- **Qué hace.** El servicio: sabe dónde están los ficheros de un proyecto y qué hay
  en cada uno. **No guarda estado del juego**, y lo dice.
- **Qué debería hacer.** G-11, con DEC-09 dentro: «desar només en punts segurs», y
  «corruptes/incompatibles mostren motiu i no destrueixen la sessió».
- **Qué se lo impide.** `pathFor` sin llamar, y cuatro pruebas para un servicio que
  G-11 describe con cinco comportamientos distintos (seleccionar, guardar con
  confirmación de sobreescritura, cargar, mostrar motivo en corruptos, y F5/F9
  reutilizando el servicio).
- **Cómo optimizarla.** Los cinco comportamientos de G-11 son cinco casos de prueba.
  Hoy hay cuatro pruebas para todo.

### `SaveGate` (línea 150, 6 de cuerpo) y `CanSaveNow`

- **Qué hacen.** La puerta de DEC-09: si se puede guardar ahora y, si no, **por qué**
  en palabras que se pueden enseñar tal cual.
- **Qué deberían hacer.** DEC-09, que es decisión aceptada y va en el criterio de
  G-11 y en el de E-03.
- **Qué se lo impide.** **El llamador le pasa la constante `false`** como
  `eventRunning` (`Application.cpp:943`). La clase es correcta; la puerta está
  alimentada con un valor fijo. Detalle completo y verificado en `02-rpg.md`,
  hallazgo 1.
- **Cómo optimizarla.** No es un problema de esta clase. Es el defecto más grave del
  área y vive una línea más allá, en un fichero sucio.

---

## Las nueve pantallas y estados GL-free

Todas siguen el mismo patrón y está bien elegido: una clase GL-free que decide, y
`Application` que pinta con `HudCommandMenu` y traduce las teclas. El patrón se
documenta igual en las nueve («esta clase no sabe nada de ventanas ni de OpenGL»),
que es señal de que es convención y no casualidad.

| Cabecera | Clases | Pruebas | Estado | Juicio |
| --- | --- | ---: | --- | --- |
| `OptionsState.h` (82) | `OptionsEntry`, `OptionsState` | 3 | limpio | H1-03, la pantalla que G-10 dejó en TODO. `displayLines` y `volume` sin llamar: el volumen se guarda y no se aplica, que es coherente con que `Audio` no llegue al binario |
| `TitleMenuState.h` (108) | `TitleMenuItem`, `TitleMenuState` | 4 | base la da por MUERTA | La viva. `continueSummary` y `subtitle` sin llamar, que son justo lo que G-11 pide para el selector («Continuar usa el mismo selector») |
| `PauseMenu.h` (40) | `PauseEntry`, `PauseMenu` | 2 | limpio | GDB-12.2. Bien. Es el molde del patrón |
| `GameOverScreen.h` (37) | `GameOverEntry`, `GameOverScreen` | 1 | limpio | GDB-12.4, Carregar/Tornar al títol. **Ojo con D-COMB-05**, que está en la bandeja de Oriol: escribió que `GameOver` es terminal, pero §10.6.B promete caída sin Game Over. Esta pantalla implementa la versión terminal |
| `InventoryScreen.h` (115) | `InventoryEntry`, `InventoryScreen` | 2 | **SUCIO** | GDB-12.3. **Nueve métodos sin llamar** y son todos de presentación: `actionLabels`, `equipmentLabels`, `inventoryLabels`, `statusLabels`, `inventoryIconIds`, `morale`, `moraleLabel`, `entriesInFocus`, `setExternalRowCount`. Una pantalla cuyas etiquetas nadie pide |
| `InventoryMenuState.h` (118) | `ItemAction`, `InventoryRow`, `EquipmentSlotRow`, `InventoryMenuState` | 1 | **SUCIO**, base la da por MUERTA | EST-23b en vuelo. Cinco métodos sin llamar (`moveSelection`, `selectedRow`, `selectedId`, `previewSelected`, `countIn`) y una prueba. `previewSelected` es justo lo que G-12 pide («previsualitzar canvis d'equip») |
| `FichaVista.h` (112) | `FichaStat`, `FichaRanura`, `FichaRasgo`, `FichaCondicion`, `FichaDatos`, `FichaCatalogos`, `FichaVista` | 1 | **SUCIO** | La ficha visible. Siete clases para una pantalla y una prueba. Su alcanzador es `src/Game/Codice.cpp`, **que no está versionado**. `FichaCatalogos` toma punteros no propietarios y cualquiera puede ser nulo, mostrando el id legible en vez del nombre: degradación elegante y escrita |
| `Codice.h` (65) | `CodiceEntrada`, `CodiceDatos`, `Codice` | 1 | **SUCIO** | Sin versionar en su `.cpp`. `etiquetaTipo`, `idDeNivel` y `cuantas` sin llamar |
| `DiarioMisiones.h` (52) | `DiarioObjetivo`, `DiarioMision`, `DiarioDatos` | 1 | **SUCIO** | Sin versionar en su `.cpp`. `activas` y `completas` sin llamar: el diario tiene las dos listas y nadie las lee. `AdventureScript.h` documenta que el diario es una **vista** de las flags y no una segunda fuente de verdad — si `DiarioMisiones` guarda estado propio, esa decisión se está rompiendo |
| `PlayerControls.h` (93) + `PlayerControlsData.h` (16) | `PlayerControls`, `PlayerControlsData` | 1 y 0 | **SUCIO** | 81 líneas de cuerpo para el mapa de controles, con su estructura GL-free aparte y cero pruebas en ella |
| `NewGameFlow.h` (74) | `NewGameFlow` | 1 | limpio | H1-04. **La mejor documentada del área**: su cabecera explica los dos defectos que cierra —que `CharacterCreationScreen::construir()` no llamaba a `recompute_derived()` y la ficha salía con `healthCap()==0` («un personaje muerto de nacimiento»), y que hablaba en teclas crudas— y cómo los cierra. Una prueba para un flujo de 14 pasos |

**Juicio conjunto del bloque.** El patrón es correcto y consistente, y la proporción
de pruebas es el problema común: 1, 1, 1, 2, 3 pruebas para pantallas con entre
cuatro y nueve métodos sin consumidor. Cinco de las nueve están sucias y tres de
esas cinco tienen el `.cpp` **sin versionar**, o sea que en un clon limpio no
existen. Antes de optimizar nada aquí hace falta que ese trabajo entre en git.

---

## `include/Game/SessionTipos.h` — 97 líneas · juego · header-only · 19 pruebas · sin paquete

Cuatro tipos pequeños que resuelven cuatro problemas de diseño distintos, todos con
el motivo escrito. Es la cabecera mejor pensada del área.

### `OwnedBusiness` (línea 30, 10 de cuerpo)

- **Qué hace.** Un local comprado por el jugador. El precio y el ingreso base son
  del catálogo y no cambian; lo que vive aquí es lo que el jugador decide: cuánto
  cobra de alquiler.
- **Qué debería hacer.** Sin paquete. **Propuesta:** el paquete de economía/moral de
  la fase H, junto a `Morality`.
- **Qué se lo impide.** Que `GameSession::collectRent` sea interno: el alquiler se
  cobra dentro de la sesión y este tipo no tiene consumidor propio.
- **Cómo optimizarla.** La separación dato del catálogo / decisión del jugador es la
  correcta y está escrita. Nada que cambiar.

### `PlayerCarryOver` (línea 52, 6 de cuerpo)

- **Qué hace.** Lo que es del jugador y no del mapa, y por tanto cruza intacto cada
  puerta. Existe como tipo —y no como lista de campos copiados a mano— **por el bug
  que arregla**: `loadLevel` rescataba el oro y nada más, así que entrar en una
  posada te reiniciaba la reputación a 0, te dejaba sin negocios y borraba el
  contador de rentas.
- **Qué debería hacer.** C-03, que es literalmente este tipo: «el traspàs és una
  sola funció, no una llista de camps escampada per `loadLevel`».
- **Qué se lo impide.** Nada. Es el criterio hecho tipo, y la cabecera explica que
  con un tipo «añadir mañana un campo del jugador es tocar UN sitio».
- **Cómo optimizarla.** Es el modelo a seguir para `GameSession` (hallazgo 1): el
  problema de los campos escampados ya se resolvió aquí, con seis líneas y el motivo
  escrito.

### `LevelTransition` (línea 65, 6 de cuerpo)

- **Qué hace.** Una transición de nivel pendiente (una puerta pisada). `GameSession`
  **no carga niveles**: no hace I/O ni conoce rutas de assets, igual que no crea
  texturas. Solo detecta que toca cambiar de mapa y lo expone para que `Application`
  lo ejecute.
- **Qué debería hacer.** C-03 y DEC-30 (la comanda de ir a otro mapa es terminal
  para el evento local).
- **Qué se lo impide.** Que `SessionCommandHost::requestedTransition` tampoco se
  llame: hay dos mecanismos para pedir un cambio de mapa, el de la sesión y el del
  anfitrión de comandas, y el segundo no tiene lector.
- **Cómo optimizarla.** «La sesión decide, el de fuera orquesta» es la división
  correcta y está escrita. Lo que sobra es que haya dos caminos para la misma
  decisión.

### `ShopOffer` (línea 75, 5 de cuerpo)

- **Qué hace.** El estado de una tienda abierta: qué se vende, a qué precio y sobre
  qué línea está el cursor. Vive mientras `mode()==Shop`, igual que `BattleState`
  mientras `mode()==Battle`.
- **Qué debería hacer.** G-14 en su mitad de comercio: «botiga mostra preu,
  quantitat i resultat, negoci compra/renda i impacte moral».
- **Qué se lo impide.** Que `ShopEngine.h` exista aparte y no llegue a ningún
  ejecutable (hallazgo 2). Dos representaciones del comercio.
- **Cómo optimizarla.** La simetría con `BattleState` es elegante y correcta. Lo que
  hace falta es retirar `ShopEngine`.

---

## `include/Game/OptionsLocaleBridge.h` — 131 líneas · juego · 2 pruebas · **SUCIO**

### `OptionsLocaleBridge` (línea 25, 105 de cuerpo)

- **Qué hace.** El puente entre el ciclo de idiomas de `OptionsState`
  (CATALA/CASTELLA/ENGLISH) y los códigos de `Engine::Locale` (ca/es/en), con una
  tabla emparejada para que las dos direcciones sean O(1) y haya una sola fuente.
- **Qué debería hacer.** Sin paquete. **Propuesta:** H1-05, que es la fila que lo
  está reescribiendo hoy.
- **Qué se lo impide.** Está en WIP vivo: pasó a header-only hace unas horas y su
  `.cpp` está borrado en el árbol sin versionar. `languageTable`, `localeCode` y
  `optionsLanguage` sin llamar desde el juego, que son las tres piezas de la tabla.
  Y los dos `(void)` que el mapa señalaba como posible stub son restos de una
  verificación que el comentario promete y el código no hace: `beforeText` se
  captura **después** de `setLanguage()`, así que ni comparándolo podría detectar
  «el texto de antes vs el de ahora», y `previousLanguage` se guarda sin camino de
  deshacer. Veredicto completo en el commit `7298518`.
- **Cómo optimizarla.** No se toca: regla 2. Se le dice a su dueño que 105 líneas de
  cuerpo para una tabla de tres entradas es mucho, y que la verificación que el
  comentario anuncia no existe.

---

## `include/Game/SessionData.h` — 90 líneas · juego · header-only · 2 pruebas · **40 miembros**

### `InventoryRules` (línea 31, 5 de cuerpo) y `SessionData` (línea 37, 54 de cuerpo)

- **Qué hacen.** El estado de la sesión como datos planos, separados de la clase que
  decide. `InventoryRules` es el patrón de inyección de reglas del inventario.
- **Qué deberían hacer.** Sin paquete. **Propuesta:** C-03, porque es el estado que
  cruza entre sesiones.
- **Qué se lo impide.** La guardia le cuenta **40 miembros**, que es el número que
  vigila porque crece antes de que nada se rompa. Dos pruebas para 40 miembros. Y
  `InventoryRules` es la pieza que la auditoría de duplicados ya señaló como
  inyectada desde ningún sitio: el patrón es correcto y no tiene consumidor.
- **Cómo optimizarla.** 40 miembros planos en una sola estructura es el mismo
  problema que `GameSession` pero sin techo que lo frene. Agruparlos por dominio
  (lo que es del jugador, lo que es del mapa, lo que es del combate) es el paso
  previo a poder partirlos, y `PlayerCarryOver` ya demuestra que el patrón funciona.

---

## `include/Game/MenuLogic.h` — 59 líneas · juego · header-only · 2 pruebas · sin paquete

### `MenuLogic` (línea 20, 38 de cuerpo)

- **Qué hace.** La navegación de menú (wraparound + clamp) como primitiva reutilizable
  GL-free.
- **Qué debería hacer.** Sin paquete. **Propuesta:** H1-03, que es la fila que la
  subió a producción.
- **Qué se lo impide.** Su propia cabecera cuenta la historia: la navegación «se
  duplicaba como mock inline en `tests/Game/G10PauseAndHudTests.cpp` porque no
  existía ninguna primitiva reutilizable GL-free: `HudCommandMenu` la implementa pero
  vive atada al render, y `PauseMenu::moveUp/moveDown` son…». O sea que **quedan al
  menos dos implementaciones más** de la misma regla de navegación. `m_count` y
  `setCount` sin llamar.
- **Cómo optimizarla.** Lo que falta no es código sino terminar la migración: que
  `HudCommandMenu`, `PauseMenu`, `TitleMenuState`, `OptionsState`,
  `InventoryMenuState` y `GameOverScreen` deleguen aquí. Seis consumidores potenciales
  y hoy la usa `OptionsState`. Es exactamente el caso que `revisar-antes-de-crear`
  describe: la pieza ya existe y los demás siguen con la suya.

---

## `include/Game/Morality.h` — 63 líneas · juego · 2 pruebas · sin paquete

### `Morality` (línea 20, 44 de cuerpo)

- **Qué hace.** La reputación moral del jugador en una escala única de −100 (villano)
  a +100 (héroe). Pieza propia y no un `int` suelto dentro de `GameSession` «porque
  va a crecer: hoy la mueve el alquiler que le pones a tus negocios, mañana la
  moverán robar, ayudar en una misión, perdonar a un enemigo».
- **Qué debería hacer.** Sin paquete. **Propuesta:** D-MOR del diseño de alcance y
  visión, que es donde viven las bandas (neutral −19..+19, respetado +20..+59,
  héroe +75) y el cabo suelto D-MOR-03: entre +60 y +74 no hay banda.
- **Qué se lo impide.** `history` sin llamar: la moral tiene historial y nadie lo
  lee, y G-14 pide «impacte moral» visible en el comercio. Dos pruebas para una
  escala con bandas. Y la auditoría de duplicados encontró **tres implementaciones de
  la moral** en el motor: esta es la que llega al juego.
- **Cómo optimizarla.** El motivo de existir como clase aparte está bien escrito y es
  el correcto. Lo que hace falta es cerrar el hueco de bandas (+60..+74), cablear
  `history` al HUD o borrarlo, y retirar las otras dos implementaciones.

---

## Las cuatro cabeceras de Game que no alcanza ningún ejecutable

| Cabecera | Líneas | Pruebas | Veredicto |
| --- | ---: | ---: | --- |
| `BattleMenuState.h` | 78 | 1 | Gemela muerta de `BattleMenuActions.h`, que sí llega. Retirar |
| `ShopEngine.h` | 70 | 1 | Segundo motor de comercio; el vivo es `GameSessionComercio.cpp` + `ShopOffer`. Retirar |
| `TitleScreen.h` | 58 | 1 | Gemela muerta de `TitleMenuState.h`. Solo la referencia su propio `.cpp`, verificado con `git grep`. Retirar |
| `Companeros.h` | 45 | 1 | **SUCIO** y con el `.cpp` sin versionar. Es la única de las cuatro que podría ser trabajo en vuelo y no código muerto: no se toca, se documenta |

Las tres primeras suman 206 líneas y 3 pruebas verdes que no prueban nada que llegue
al jugador. Es el mismo mecanismo que cerró E-06, E-09 y E-16 en falso, pero a
escala de clase entera en vez de método.
