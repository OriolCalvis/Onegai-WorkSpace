# Fase H — Jugabilidad y autoría: lo que falta para que se pueda jugar y crear

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe la fase H (jugabilidad y autoria) tal como se midio el 17/09: que ya existia y que faltaba para cada tarea, foto del 17/09/2026. La columna «que ya existe» es la foto del 17/09 y NO se actualiza; lo que sigue vivo es el gantt derivado (`gantt_jugabilidad_H.puml`), las filas de `TABLERO.md` y los `planificacion/seguimiento/*.json` de cada tarea. De las 33 citas de codigo que contiene, 18 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

**Autor:** qwen, sesión AUD-01, 17/09/2026.
**Estado:** propuesta. No toca `pla/dades.py` ni los Gantt existentes; es
complementaria a la fila **ALPHA-00** de Codex (`ALPHA_CIERRE.md` +
`gantt_alpha_cierre.puml`), que trabaja el cierre de integración y aceptación.
Si Oriol prefiere un solo plan, este documento es material para fusionar, no un
plan rival.

**Gantt:** [`gantt_jugabilidad_H.puml`](gantt_jugabilidad_H.puml).

---

## 0. De dónde sale y con qué criterio

Sale de tres mediciones, no de una opinión:

1. `ALCANCE_REAL_2026-09-17.md` (Claude) — qué puede usar el jugador y el autor.
2. `planificacion/INFORME_DUPLICADOS_2026-09-17.md` (esta sesión) — 127
   hallazgos: 3152 líneas muertas, 5 paquetes cerrados que no llegan, 2 con la
   lógica dentro del test.
3. `REVISION_ALCANCE_CHATGPT_2026-09-17.md` (esta sesión) — los 9 puntos de
   Oriol con ChatGPT, verificados uno a uno, más 8 bloqueos que la lista no
   mencionaba.

**Criterio de cierre de cada tarea** ([`CHECKLIST_CIERRE_TAREA.md`](CHECKLIST_CIERRE_TAREA.md),
[`decisiones/D0006`](../../../decisiones/D0006-criterios-de-cierre-por-alcance.md)):
ninguna tarea de esta fase se cierra con «su prueba pasa». Se cierra con una
**ruta de usuario** escrita que una persona puede recorrer, y con la medición de
`tools/medir_alcance.py` confirmándola. Por eso cada fila de las tablas lleva su
ruta en la columna de cierre.

**Regla de no duplicar.** Antes de cada tarea está la columna **«qué ya existe»**.
Es la columna importante: en esta fase hay mucho más trabajo *escrito y no
llegado* que trabajo por escribir. Montar de nuevo lo que ya está es el fallo que
este plan quiere evitar.

**1 j = 1 jornada de esfuerzo (~6 h)**, como en los Gantt existentes.

**Sobre la numeración.** Conviven tres (F0–F8 del roadmap, F0–F5 del editor,
C–G del corte B) y está en la bandeja de Oriol desde el 03/09. La **H** no
resuelve eso: es un prefijo nuevo que no colisiona con ninguna de las tres y que
se puede retirar el día que Oriol decida cuál gobierna.

---

## H0 — Cableado crítico: lo que ya existe y no llega

Seis llamadas y un intérprete. Es lo de mayor rendimiento del plan: casi todo el
código ya está escrito y probado, y lo que falta es que `Application` lo llame.
**`Application.cpp` es fichero de cruce (14 paquetes lo tocan): una sola sesión,
con fila propia, en este orden.**

| ID | Tarea | Qué ya existe (no rehacer) | Qué falta | j | Dep | Cierre: ruta de usuario |
| --- | --- | --- | --- | ---: | --- | --- |
| H0-01 | La campaña carga los catálogos de su proyecto | `openProject`, el manifiesto de Boundington declara 15 catálogos, `ObjectCatalog` | Que «Campaña» pase por el manifiesto en vez de cargar solo `boundington_npcs.json` | 1,5 | — | Título → Campaña → Día 1 → hablar con el tendero, el joyero y el sastre: **el tutorial se completa**. Prueba: las 391 referencias inertes bajan a 0 en la ruta de campaña |
| H0-02 | Teclas de las pestañas de inventario | `InputAction::MenuLeft/MenuRight` existen; `InventoryScreen` tiene las 4 pestañas | Asignar tecla en `InputSystem.cpp` | 0,5 | — | Tab → Inventario → ← / → pasa por Equipo, Estado y Acciones |
| H0-03 | La pausa existe y se puede salir de ella | `GameSession::pause()`, el menú de 8 opciones de G-10 | Esc → `pause()` (hoy → `Quit`); «Reanudar» → `resume()` en vez de `closeInteraction()` | 1 | — | Esc abre la pausa; «Reanudar» vuelve al juego; Esc no cierra el juego |
| H0-04 | Las reglas de equipo llegan al inventario | `GameSession::setInventoryRules`, `equipment.json` (413 piezas), `TierRules.maxEquipRarity`, la colisión MainHand↔OffHand (2.14) | Llamar a `setInventoryRules` con el catálogo cargado; que `InventoryScreen` cree la acción Desequipar | 1,5 | H0-01, H0-02 | Tab → Equipo → equipar una espada → la ficha cambia → desequipar → vuelve |
| H0-05 | Ranuras de guardado | `Game::SaveSlotState` (165 líneas, 2 pruebas, muerto) | Cablear F5/F9 a una ranura y «Cargar partida» al selector (hoy es un `TODO`) | 2 | H0-03 | Título → Cargar partida → lista de ranuras con fecha y nivel → cargar una. F5 guarda en la ranura activa |
| H0-06 | Los estados alterados caducan | `ConditionEngine::tick_conditions` (`ConditionEngine.cpp:211`, el único sitio que resta rondas) | Llamarlo una vez por ronda de combate | 0,5 | — | Envenenar a un enemigo y esperar N rondas: el estado desaparece solo y deja de hacer daño |
| H0-07 | El título sale del manifiesto | `Game::TitleMenuState` (escrito el 17/09, sin cablear), `G-08` | Que la lista de escenarios venga de los proyectos, no de una lista fija | 1 | — | En el título aparecen `este_sur` y `mundo`, que hoy no salen; cada uno entra por su propia entrada |
| H0-08 | **El juego ejecuta eventos de mapa** | `EventInterpreter` (322 líneas, 5 pruebas), `SessionCommandHost`, `EventCommand` (208 líneas, 4 pruebas), `LevelLoader` ya lee `mapDocument` | Que `Application` cargue el `mapDocument` del nivel y corra el intérprete con `SessionCommandHost` de fallback | 4 | H0-01 | Jugar `assets/misiones/mision_llave.json`: la puerta cerrada se abre con la llave. **Sin esto, la fase E entera (49 de sus 79 jornadas) no llega a nadie** |

**Subtotal H0: 12 j** — desbloquea la fase E, la pausa, el equipo, las ranuras y
el tutorial.

---

## H1 — Rescate de los cinco paquetes cerrados que no llegan

No es reescribir: es **mover** la lógica de `tests/` a `src/` y cablearla. Los
cinco tienen sus pruebas verdes y su diseño hecho.

| ID | Tarea | Qué ya existe | Qué falta | j | Dep | Cierre |
| --- | --- | --- | --- | ---: | --- | --- |
| H1-01 | G-13: menú de combate desde datos | `buildBattleMenuSkillActions`, `selectTargetsFor`, `translateToBattleAction`, `BattleMenuEntry` — **todo dentro de `tests/Game/G13CombatCommandsTests.cpp`**; 492 skills en `skills.json`; las 5 reglas de disponibilidad | Mover a `src/Game/`, y que `Application.cpp:654` deje de tener la lista fija de 5 entradas | 2,5 | H0-04 | En combate, el menú lista las habilidades de la ficha con coste, disponibilidad y motivo de bloqueo. Sin IDs escritos a mano |
| H1-02 | G-14: HUD, temporizadores y comercio | `HudTimer`, `ShopEngine`, `ShopTransaction` — dentro de `tests/Game/G14HudTimersAndCommerceTests.cpp` | Mover a `src/HUD/` y `src/Game/`, cablear al HUD real | 2,5 | H0-01 | Una tienda real con stock y precios desde datos; los temporizadores de misión se ven en el HUD |
| H1-03 | G-10: pausa y opciones de verdad | `PauseMenu` real (8 opciones); `MenuLogic`/`CmdMenuLogic` definidos en el propio test; «Opciones» es un `TODO` | Mover `MenuLogic` a producción y hacer que «Opciones» abra algo | 1,5 | H0-03 | Esc → Opciones → volumen, idioma, controles: la pantalla existe y se puede volver |
| H1-04 | G-09: creación de personaje integrada | `CharacterCreation` + `CharacterCreationScreen` (4 y 3 pruebas); `CharacterSheet`; `classes/races/backgrounds.json` | Que «Nueva partida» pase por ahí. Falta además `Player::populate_from_class_race_background` (BITACORA 4.3): el PJ se crea a mano, el enemigo ya tiene su equivalente | 2,5 | H0-07 | Título → Nueva partida → elegir clase, raza y trasfondo → la ficha sale con sus stats y su equipo inicial |
| H1-05 | G-17: idioma seleccionable | `Engine/Locale` (167 líneas, 2 pruebas, muerto); `assets/i18n/` existe | Cablear `Locale` a los textos del motor y al menú de Opciones | 2 | H1-03 | Opciones → Idioma → catalán/castellano: los textos del motor cambian |

**Subtotal H1: 11 j.**

---

## H2 — Jugabilidad: combate, progresión y dados

| ID | Tarea | Qué ya existe | Qué falta | j | Dep | Cierre |
| --- | --- | --- | --- | ---: | --- | --- |
| H2-01 | El dado se ve | `DicePoolEngine::roll_pool` y `opposed_degree` ya resuelven de verdad (reglamento Ed. 3, sin CA ni CD, enfrentada pura); `GameSession.cpp:380,393` | **Presentación**: un widget de tirada sobre el HUD (dados, caras, grado, pifia). No hay nada de esto en `src/HUD/` | 2 | H0-08 | Al hablar, al atacar y al usar una habilidad se ve la tirada y su grado. La pifia se distingue a simple vista |
| H2-02 | Objetivos visibles | `selectTargetsFor` con SELF/SINGLE_ENEMY/ALL_ENEMIES/SINGLE_ALLY/ALL_ALLIES/ZONE_RADIUS; los muertos salen `alive=false` | La selección en pantalla: señalar, ver alcance y bandol, cancelar sin gastar | 2,5 | H1-01 | En combate, elegir un hechizo de área → se marcan los afectados → cancelar no gasta PM |
| H2-03 | Condiciones en el HUD | `ConditionEngine` con 3 modos de stacking y `damage = damagePerRound × stacks` (2.12); `CharacterSheet::conditions` | Iconos con rondas restantes; que `tick_conditions` alimente el widget | 2 | H0-06 | Un enemigo envenenado muestra el icono y el contador, y ambos bajan por ronda |
| H2-04 | Progresión por hitos y tiers | `CharacterSheet::tierUp(TierRules, milestoneCount)` declarado **e implementado** (`CharacterSheet.cpp:75`); `grantMilestoneOnEnd` ya se parsea de las aventuras (`RpgCatalogs.h:1539`); `TierRules` | **Nadie lo llama.** Falta: contar los hitos concedidos en `GameSession`, llamar a `tierUp`, y mostrar el tier en la ficha. No hay que inventar XP: el reglamento va de hitos | 2 | H1-04 | Terminar una aventura que concede hito → el personaje sube de tier → su mano y sus límites cambian (TierRules) |
| H2-05 | Botín al ganar | `RPG::LootResolver` (21 líneas, muerto); `loot_tables.json` existe y el juego no lo lee | Cablear al final del combate y al registro de objetos | 1,5 | H0-01 | Ganar un combate da objetos de su tabla; salen en el inventario y se pueden equipar o vender |
| H2-06 | Hechizos y clases con contenido | `spells.json` (210) y `classes.json` (61) cargan en las pruebas | El juego no lee `spells`. Y aguas arriba: **202 de 210 hechizos huérfanos y 59 de 61 clases con `spells: []`** (BITACORA 4.1) | 2 + contenido | H1-01 | Elegir el Apóstata en la creación → tiene hechizos → los lanza en combate. La parte de contenido va por `canon → cartas → motor`, no se inventa en el motor |
| H2-07 | Enemigos con vida real | `populate_from_monster_catalog()` ya corrige vía `recompute_derived()`; `monsters.json` con 432 fichas | **432 `MonsterDefinition` tienen `maxHealth == 0` en el JSON fuente.** Decidir si el canónico es el JSON o el calculado (BITACORA 4.3) y rellenar | 1,5 | H0-01 | Cualquier enemigo del catálogo aparece con su vida correcta sin depender del cálculo de emergencia |
| H2-08 | Consumibles y equipo desde datos | `consumables.json`, `equipment.json` (413 piezas); `InventoryEngine` con `rarityRank(string)` (2.13) | Que el juego lea esos dos catálogos (hoy lee 3 de 24) | 2 | H0-04 | En la tienda hay objetos del catálogo real; en el inventario se equipan y se usan |

**Subtotal H2: 15,5 j** (+ contenido aguas arriba).

---

## H3 — Jugabilidad: mundo, historia e interacción

| ID | Tarea | Qué ya existe | Qué falta | j | Dep | Cierre |
| --- | --- | --- | --- | ---: | --- | --- |
| H3-01 | Viajar entre naciones | 8 TMX de mapamundi; `locations.json` (19 naciones, 95 ciudades); `LevelTransition`; `conectividad.py` | Nadie conecta el atlas con `LevelTransition`. 13 `desembarco_landmass_*` inalcanzables (mapamundi al 42%) | 3 | H0-08 | Desde Boundington se llega al mapamundi y a otra nación, y se vuelve. `conectividad.py` en verde |
| H3-02 | Diario de misiones | `questIds` en los documentos; `AUDITORIA_CONTRATOS.md` avisa de que **`QuestDefinition` no existe** y cuelga al vacío | El contrato, el cargador y la pantalla | 3 | H0-08 | Abrir el diario → misiones activas, objetivos y estado. Los `questIds` de los niveles resuelven |
| H3-03 | Diálogo ramificado por interruptores | `NarrativeEngine` con beats y `skillCheck` diferido (4 grados → 4 flags); la fase E tiene interruptores locales (evt05) | Que las ramas dependan de interruptores y de hitos jugados, y se vea | 2,5 | H0-08 | Una conversación cambia según lo que se hizo antes; volver a hablar no repite la misma rama |
| H3-04 | Comercio con datos reales | `ShopEngine` (H1-02), DEC-05 aisló los catálogos de tienda; mercado real: poción 30, éter 45, pan 8 | La pantalla de compra/venta con stock, oro y precios desde el catálogo | 2 | H1-02 | Entrar en la tienda → ver stock y precios → comprar → el oro baja y el objeto va al inventario → vender |
| H3-05 | Negocios y rentas | `GameSession::collectRent` existe (lo cita `medir_alcance` como método que nadie de fuera llama) | Que el jugador pueda gestionarlo y verlo | 1,5 | H3-04 | Comprar un negocio → cobrar la renta → ver el efecto en el oro |
| H3-06 | Temporizadores de misión visibles | F1 de zcode: `updateNarrativeTimers(dtActivo)`, congelación en diálogo/combate/AFK>15 s, presentación Contador/Barra/Secreto (2.18) | El widget del HUD y el enganche del `dt` activo | 1,5 | H1-02 | Una misión con límite muestra su barra; al entrar en combate se congela; al expirar sale como beat de diálogo |
| H3-07 | Objetos del mundo que hacen algo | `MapEntityDocument`, las propiedades de instancia (`EditorState::setObject*`), patrullas, botín, destino de puerta | Que el runtime las lea y las ejecute: hoy los objetos de la ciudad son inertes | 3 | H0-08, H4-03 | Una puerta lleva donde dice su destino; un guardia patrulla; un cofre da su botín |
| H3-08 | Recuperarse de una derrota | G-11 menciona «slots y recuperación de derrota» | Decidir y hacer: dónde reapareces, qué pierdes, cómo se guarda | 2 | H0-05 | Perder un combate no cierra el juego: reapareces donde toca y la partida sigue |

**Subtotal H3: 18,5 j.**

---

## H4 — Autoría: crear y gestionar desde `motor_editor`

Lo que el autor puede hacer hoy en `motor_editor`: abrir un nivel del proyecto,
pintar, borrar, rellenar, colocar y quitar objetos, fijar el inicio, gestionar
capas de baldosas, deshacer y guardar. **14 acciones.** Todo lo demás o no está o
solo está en el editor antiguo.

| ID | Tarea | Qué ya existe | Qué falta | j | Dep | Cierre: ruta del autor |
| --- | --- | --- | --- | ---: | --- | --- |
| H4-01 | Crear, duplicar y borrar proyectos | `ProjectIndex::createNamed`, `duplicate`, `suggestId` — escritos y sin consumidor; `ProjectHub`; `FORMATO_PROYECTOS.md` | La pantalla y el cableado en `motor_editor` | 2,5 | — | Abrir el editor → Nuevo proyecto → darle nombre → aparece en el árbol → duplicarlo → borrar la copia |
| H4-02 | Crear y gestionar mapas de un proyecto | `EditorDocumentSession`, `MapDocument`, el serializador con round-trip | CRUD de mapas: nuevo, renombrar, borrar, elegir tamaño y tileset | 2,5 | H4-01 | Dentro de un proyecto → Nuevo mapa → elegir tamaño → pintar → guardar → reabrir y está |
| H4-03 | Propiedades de instancia | `EditorState::setObject*` completo — **solo lo llama el editor antiguo** (`ObjectInspector`); `OBJECT_VARIANTS_FOR_EDITOR.md` | El panel en `motor_editor` y que el runtime lea las propiedades (H3-07) | 3 | H4-02 | Seleccionar un objeto → editar su botín, su destino de puerta, su patrulla o su evento → guardar → el juego lo respeta |
| H4-04 | Cuentagotas, rectángulo y línea | Núcleo en `EditorState` (`LineCells`, `RectCells`); shell en `EditorActions` (cursor, 15/15 tests) | **El arrastre en ImGui dentro de `EditorApplication` (E-10)** | 2 | H4-02 | Elegir rectángulo → arrastrar sobre el viewport → la zona se pinta; cuentagotas → clic → la baldosa pasa a la paleta |
| H4-05 | Panel de problemas y búsqueda | `ProblemsPanel.cpp` (qwen, E-15) ya se enlaza en `motor_editor`; `ProblemsState.h` (277 líneas) | Que `EditorApplication` lo llame. Y que los problemas vengan de la validación real (H5-06) | 2 | H4-02 | Abrir el panel → ver los errores del mapa (puertas sin vuelta, objetos sin definición) → clic en uno → salta a la celda |
| H4-06 | Navegador de recursos | `AssetBrowserState.h` (215 líneas, muerto, sin pruebas en HEAD); `AssetBrowserState.cpp` (E-13, qwen, en vuelo); `NormalizeForSearch` (DEC-15) | Terminar E-13 y cablearlo: el autor elige por nombre, no por ID | 3 | H4-02 | Abrir el navegador → buscar «mercado» con acentos plegados → arrastrar el recurso al mapa |
| H4-07 | Selectores por nombre | `CatalogPickerState` (E-14) con `addEntries`/`findById`; **sus tests actuales no comprueban el criterio** (`loadCatalogs` devuelve true sin cargar nada) | Rehacer la prueba con los 8 casos de aceptación de `seguimiento/E-14-alternativa-claude.md` y cablear | 2 | H4-03, H5-05 | En el campo «enemigo» de una instancia aparece una lista con nombres, no un cuadro de texto con IDs |
| H4-08 | Probar la partida desde el editor | Existe en el editor antiguo: tecla P/F7 → `./juego --nivel` (F-01) | Portarlo a `motor_editor` con el estado sin guardar resuelto | 2 | H4-02, H0-08 | Botón «Probar» → el juego arranca en el nivel que se está editando → al salir se vuelve al editor con el documento intacto |
| H4-09 | Capas: orden, visibilidad y bloqueo | `moveLayer`/`moveTileLayer` (cursor, E-12, 8/8); `LayerPanel` de baldosas ya existe | El panel ImGui de orden/visibilidad/bloqueo y los formularios de mapa | 2 | H4-02 | Reordenar capas arrastrando → ocultar una → bloquear otra → lo que se ve en el viewport coincide |
| H4-10 | Autotiles y terrenos | `REMAPEO_TILES.md`, `remapear_suelos.py`, `gen_tileset_iso.py` | La regla de autotile en el editor (está en 1.1, aplazada por D0002) | 3 | H4-04 | Pintar una esquina de muro y que el borde salga solo, sin elegir las 16 variantes |

**Subtotal H4: 24 j.**

---

## H5 — Autoría: datos, eventos y validación

| ID | Tarea | Qué ya existe | Qué falta | j | Dep | Cierre |
| --- | --- | --- | --- | ---: | --- | --- |
| H5-01 | Editor de páginas de evento | `EventEditorState.h` muerto; `editor/src/event_editor.cpp` no está en `motor_editor`; E-09 (qwen, en vuelo) hace el puente a `MapDocument` | El panel de páginas con condiciones y lista de comandos | 3 | H4-02, H0-08 | Seleccionar un objeto → pestaña Evento → añadir página → escribir condición y comandos → guardar |
| H5-02 | Lista de comandos | `CommandListState.h` muerto (E-06); `EventCommand` con 208 líneas y 4 pruebas | El editor visual de la lista: añadir, reordenar, ramificar | 3 | H5-01 | Dentro de una página, añadir «dar objeto», «abrir puerta», «iniciar combate» y reordenarlos |
| H5-03 | Depurador paso a paso | E-08 (zcode, en vuelo): modo paso a paso del intérprete, estado tras N pasos == ejecutar N de golpe | La ventana del depurador con el estado de interruptores y variables | 2,5 | H5-02, H0-08 | Ejecutar el evento paso a paso → ver qué interruptor cambia en cada paso |
| H5-04 | Base de datos RPG del proyecto | `ProjectDatabaseState.h` (E-16, muerto) — **hoy rompe el build**: `.cpp` devuelve `Result<void>` y la cabecera declara `bool` | Alinear las firmas primero, y luego el panel de objetos/consumibles/equipo/habilidades/enemigos locales | 3,5 | H4-01 | Crear un objeto propio del proyecto → aparece en los selectores → se puede colocar en un mapa |
| H5-05 | Validación de contenido dentro del editor | `validar_todo.py` orquesta 5 validadores + referencias cruzadas + objectIds + beats (485 líneas) — **y no está en CTest ni en CI** | Que el panel de problemas consuma esa validación en vez de duplicarla | 2 | H4-05 | El panel muestra los mismos errores que `validar_todo.py`, con el mismo texto |
| H5-06 | Aventuras y diálogos editables | `FORMATO_AVENTURAS.md`, los beats, el `skillCheck` diferido, 9 aventuras en `assets/adventures/` — **3 no las carga ninguna ruta** | El editor de guion: beats, líneas, ramas y pruebas | 4 | H5-01 | Escribir una aventura nueva sin tocar JSON → se puede jugar desde el título |

**Subtotal H5: 18 j.**

---

## H6 — Presentación: audio, idioma y accesibilidad

| ID | Tarea | Qué ya existe | Qué falta | j | Dep | Cierre |
| --- | --- | --- | --- | ---: | --- | --- |
| H6-01 | El motor de sonido llega al juego | `include/RPG/Audio.h` (131 líneas) + `src/RPG/Audio.cpp` con `AudioEngineImpl` sobre **miniaudio real**, ya vendido en `third_party/miniaudio/`, compilado en `CMakeLists.txt:98` y con `test_audio` en `:1023`. Buses, loop, streaming, volumen por bus, callbacks | **Ningún ejecutable lo llama.** Cablear a `Application`: init/shutdown, un bus por categoría, disparo en eventos | 2,5 | H0-03 | Al entrar en combate suena la música de combate; al golpear, un efecto; al salir, vuelve la de exploración |
| H6-02 | Que exista algo que sonar | **0 ficheros de audio en `assets/`** | La colección mínima: título, exploración, combate, tienda, victoria, derrota + SFX de interfaz, pasos, golpe, cura, dado | 3 + assets | H6-01 | Todos los estados del juego tienen su sonido; los assets están en `assets/audio/` y los declara el manifiesto |
| H6-03 | Opciones de audio | `AudioBus` y el volumen por bus ya existen | La pantalla: volumen maestro y por bus, y que se guarde | 1 | H1-03, H6-01 | Opciones → bajar el volumen de efectos → se nota al instante → se guarda y persiste |
| H6-04 | Textos del motor traducidos | `Locale` (H1-05) y `assets/i18n/` | Extraer las cadenas del motor a los ficheros de idioma | 2 | H1-05 | Cambiar de idioma cambia todos los textos del motor, no solo los del contenido |
| H6-05 | Accesibilidad | `BitmapFont::wrapText`/`clipText`; la fuente dibuja acentos desde el 17/08 | Tamaño de texto, remapeo de teclas, y que los grados de éxito no dependan solo del color | 2,5 | H1-03 | Cambiar el tamaño del texto y remapear una tecla; un daltónico distingue CRITICAL de SUCCESS sin color |

**Subtotal H6: 11 j** (+ los assets de audio, que son contenido y van aguas arriba).

---

## H7 — Exportar y aceptar

| ID | Tarea | Qué ya existe | Qué falta | j | Dep | Cierre |
| --- | --- | --- | --- | ---: | --- | --- |
| H7-01 | Núcleo del exportador | `tools/build_proyecto.py` ya construye un proyecto; nada más | Empaquetar assets + ejecutable + manifiesto en un juego autónomo (G-06 no existe) | 4 | H0-01, H0-07 | Exportar Boundington → sale una carpeta → se ejecuta sin el repo ni las herramientas |
| H7-02 | Asistente de exportación | G-15 previsto; `ProjectHub` | La pantalla en el editor con opciones y progreso | 2,5 | H7-01, H4-01 | En el editor: Archivo → Exportar → elegir destino y opciones → barra de progreso → mensaje de éxito |
| H7-03 | Prueba de aceptación con gente nueva | G-16 prevista; `tools/verificar_controles.py`; este checklist | Que la pasen un autor y un jugador que no hayan leído el código | 2 | H4-08, H7-02 | Un autor nuevo crea un proyecto con un mapa, un NPC y un evento; un jugador nuevo termina la campaña. Se registra quién, cuándo y qué falló |
| H7-04 | Export nativo y tiendas | Aplazado por D0002 | Decidir si entra | — | H7-01 | Fuera de esta fase: se queda como decisión pendiente |

**Subtotal H7: 8,5 j.**

---

## H8 — Deuda que midió el analizador

Sale de `INFORME_DUPLICADOS_2026-09-17.md`. Son tareas cortas y evitan que la
fase H se construya encima de suelo movido.

| ID | Tarea | Medido | j | Cierre |
| --- | --- | --- | ---: | --- |
| H8-01 | Decidir sobre las 24 cabeceras muertas | **3152 líneas** que no alcanza ningún ejecutable, con **44 pruebas** sosteniéndolas. La mayoría se cablea en H0-H6; el resto se retira | 2 | `analizar_duplicados --solo MUERTO` devuelve 0, o cada muerto tiene su `alcance.motivo` escrito (D0006 punto 6) |
| H8-02 | `SkillTarget` existe dos veces | global en `include/Game/Skill.h:32` y `RPG::SkillTarget` en `include/RPG/Definitions/SkillDefinition.h:42` | 1 | Uno gana y el otro es un alias; `--solo NOMBRE_AMBIGUO` en 0 |
| H8-03 | Duplicados exactos de contenido | 5 JSON idénticos del experimento de los cuatro (`connections/cultures/factions/history/locations.json` entre claude, trae y zcode) + `map_entity_fields_round_trip_2.json` | 0,5 | Decisión de Oriol (pregunta 5 de D0006): se borran o se dejan como prueba del experimento, y queda escrito |
| H8-04 | Docs desactualizados | **25** cuyo código citado cambió después. Los peores: `motor_grafico_roadmap_ejecutable_v2.md` (118 citas, 51 cambiadas), `analisis_clases/08_matriz_cobertura_y_diagnostico.md` (101/41), `ESTADO_REAL_vs_ROADMAP.md` (81/37), `PLA_DEFINITIU.md` (190/33), `README.md` (92/20) | 2 | Cada uno se actualiza o se fecha y se declara histórico en la cabecera, que es lo que ya hace la Parte 0 de la bitácora |
| H8-05 | Cabeceras envoltorio | `examples/Check.h` → `Core/Check.h`; `include/Editor/ProjectIndex.h` → `Project/ProjectIndex.h` (6 líneas); `include/Render/Tile.h` → `Runtime/RuntimeTile.h` con `using Tile = RuntimeTile` | 0,5 | Cada una dice en un comentario por qué existe, o los consumidores pasan a incluir la real |
| H8-06 | `Application.cpp` | En la bandeja de Oriol desde el 03/09: tienda, negocio, temporizadores y tablas raciales dentro del mismo fichero de cruce que 14 paquetes tocan | 3 | Se decide: entra en el plan o se registra como deuda con ficha. **Esta fase lo toca en H0, así que hay que decidirlo antes de H0** |
| H8-07 | `Culling.h` copiado dentro de un test | `tests/Render/OcclusionTests.cpp` lleva la versión histórica byte a byte (C-19, y el propio test lo explica en su cabecera) | 0,5 | Se deja constancia de que es intencional en el registro del paquete, para que el analizador no lo vuelva a señalar como deuda |

**Subtotal H8: 9,5 j.**

---

## Camino crítico y totales

```
H8-06 (decidir Application) ─┐
                             ├─► H0 (12 j) ─┬─► H1 (11 j) ─┬─► H2 (15,5 j)
                             │              │              └─► H6 (11 j) ─► H7-01 ─► H7-02
                             │              └─► H3 (18,5 j)                    │
                             │                                                 │
H4 (24 j) ──► H5 (18 j) ─────┴────────────────────────────────────────────► H7-03
H8 (9,5 j, en paralelo salvo H8-06 que va antes de H0)
```

| Subfase | Jornadas | Qué compra |
| --- | ---: | --- |
| H0 Cableado crítico | 12 | Que lo ya escrito llegue al jugador. **Mayor rendimiento del plan** |
| H1 Rescate de cerrados | 11 | 5 paquetes que figuran hechos y no lo están |
| H2 Combate y progresión | 15,5 | Dados visibles, objetivos, condiciones, tiers, botín, equipo |
| H3 Mundo e historia | 18,5 | Viajar, diario, ramas, comercio, objetos vivos, derrota |
| H4 Autoría de niveles | 24 | Crear proyectos y mapas, propiedades, herramientas, probar |
| H5 Autoría de datos | 18 | Eventos, comandos, depurador, base de datos, guion |
| H6 Presentación | 11 | Audio, idioma, accesibilidad |
| H7 Exportar y aceptar | 8,5 | Juego autónomo y prueba con gente nueva |
| H8 Deuda medida | 9,5 | No construir encima de suelo movido |
| **Total** | **128,5 j** | Sin contar el contenido aguas arriba (hechizos, audio, mapamundi) |

**Si solo hay una semana:** H8-06 + H0 entero (12 j). Al terminar, el jugador
tiene pausa, pestañas, equipo, ranuras, condiciones que caducan, eventos de mapa
y el tutorial del Día 1 completo — y la fase E deja de ser trabajo invisible.

**Si hay dos:** H0 + H1 (23 j). Los cinco paquetes mal cerrados pasan a estar
cerrados de verdad, y el combate tiene menú desde datos.

---

## Lo que este plan NO incluye, a propósito

- **Reglas nuevas de Nd6.** D0005 sigue en `proposed` y su oleada 4 está
  excluida hasta que Oriol responda. Añadir dados extra o acciones nuevas aquí
  sería legislar sin mandato.
- **Contenido aguas arriba.** Los 202 hechizos huérfanos, las 59 clases sin
  hechizos, las 7 deidades que faltan, los 41 asentamientos sin nombre y los 13
  `desembarco_landmass_*` son de `canon → cartas → motor`. El motor puede
  integrarlos (H2-06, H3-01) pero no puede inventarlos.
- **Audio, autotiles, plugins y generador gráfico de personajes** ya estaban
  aplazados por D0002; aquí entran solo donde hay código muerto que lo pide
  (H6-01, H4-10).
- **La decisión de las tres numeraciones.** Sigue en la bandeja de Oriol desde el
  03/09. La H convive y se retira si hace falta.
- **`pla/dades.py` y el HTML del plan.** Mientras E-13 tenga WIP vivo, la fila
  ALPHA-00 de Codex los tiene reservados. Estas tareas van primero en este
  documento y en el `TABLERO.md`; entrar en `dades.py` es un paso posterior y
  coordinado.

## Cómo se convierte en filas del TABLERO

Cada ID de estas tablas es una fila candidata en «Pendiente — libre para quien
la coja», con su zona (las rutas de la columna «qué falta») y su entrega
verificable (la columna de cierre). No las he volcado todas de golpe: 60 filas
nuevas de una vez hacen el tablero ilegible y tapan el WIP vivo. Propuesta de
orden de entrada:

1. **Ya:** H8-06 (decisión), H0-01…H0-08 (una sola sesión, fichero de cruce).
2. **Cuando H0 aterrice:** H1-01…H1-05 y H4-01, H4-02.
3. **El resto, por subfases**, a medida que se libere WIP de E-09/E-13/E-15/E-16.
