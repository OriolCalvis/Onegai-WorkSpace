# 08 · Lo que hay y lo que esperamos que haya

**Qué es.** El diseño que sale del juicio clase a clase (`00-INDEX.md` y `01`…`07`) una
vez resueltas las cuatro decisiones que lo bloqueaban. No describe código escrito:
describe **hacia dónde** va cada pieza y **en qué orden**, con el criterio que dirá que
está hecha. Cada punto se convierte en una fila del tablero o en un paquete del plan;
nada de aquí está implementado.

**Las cuatro decisiones de Oriol (29/09).**

| Pregunta | Decisión |
| --- | --- |
| ¿El combate lleva iniciativa? | **Sí, 1d6 + DES** |
| ¿Las pantallas son escenas? | **Sí: una escena por pantalla**, como dicen G-08…G-17 |
| ¿Qué pasa con `Audio`, `LootResolver`, `AdventureSerializer` y `AdventureValidation`? | **Enlazar lo que se pueda ya** |
| ¿El editor se conecta a su capa de estado (EST-42)? | **Sí, toda**: Problemas, Eventos, Base de datos, Recursos y mapa |

**Las reglas que no cambian.** La ley del repositorio: ningún fichero de `src/`,
`include/` o `editor/` pasa de 500 líneas, ninguna cabecera de 15 miembros, ninguna
librería mezcla GL con no-GL, y nada se cierra si no lo alcanza un ejecutable. Y el
flujo `canon → cartas → motor`: el motor aplica decisiones del dueño del canon, pero no
lo reescribe. Donde una decisión cambia el canon, se dice abajo qué hay que cambiar
aguas arriba.

**Lo que condiciona el orden.** Casi todo lo de abajo cae en ficheros que hoy tienen
trabajo sin commitear de otras sesiones: `Application.*`, `BattleState.*`,
`EditorApplication.cpp`, `GameSession.*`. La regla es no tocarlos, así que **el paso 0
de todo el diseño es que ese trabajo se commitee** (§3).

---

## 1. Las cuatro decisiones, en detalle

### 1.1 Combate con iniciativa: 1d6 + DES

**Lo que hay.** `BattleState` resuelve una ronda en orden fijo: cada aliado actúa
cuando alguien de fuera llama a `resolveAllyAction()`, y después una sola llamada a
`resolveEnemyTurn()` resuelve **a todos los enemigos de golpe**. Su cabecera lo dice:
«cola de turnos deliberadamente simple (sin velocidad/iniciativa)». Mientras tanto,
`ICombatant::initiative_bonus()` (DES por defecto; «las pasivas pueden
sobreescribirlo») y sus tres overrides (`Player`, `EnemyBrain`, `Enemy`) no los llama
nadie, y `tier_rules.json:93` dice «§5: Iniciativa = DES + Modificadores».

**Lo que esperamos.**

- Al empezar un combate, cada combatiente tira **1d6 + `initiative_bonus()`**. Por
  defecto eso es 1d6 + DES, y una pasiva que sobrescriba el bonus entra sola, que es la
  parte de «+ Modificadores» de §5.
- Se actúa de mayor a menor y **el orden se mantiene todo el combate** (GDD, Llibre IV,
  cap. 1).
- **Empates:** gana quien tenga más DES; si persiste, el bando del jugador. Es la regla
  del GDD y se mantiene salvo que Oriol diga otra cosa: con un d6 los empates serán
  frecuentes, así que la regla importa más que con un d20.
- Cada enemigo actúa **cuando le toca**, no todos juntos. Un combatiente caído pierde
  su turno.
- La tirada sale del `RandomEngine` (C-21: un solo generador de azar) y en las pruebas
  va con semilla fija.

**Cómo.**

1. **Una clase nueva, en su fichero y sin GL:** `Game::OrdenDeIniciativa`. Recibe los
   `ICombatant` y un `RandomEngine&`, tira, ordena, desempata y da `siguiente()`
   saltándose a los caídos. Su prueba, determinista: el orden con semilla fija, los dos
   niveles de desempate, un caído a mitad de ronda y que la tirada nunca sale de 1..6.
2. **`BattleState` consume esa cola.** `turnoActual()` dice a quién le toca;
   `resolveAllyAction()` solo vale para el aliado del turno; `resolveEnemyTurn()` pasa a
   resolver un único enemigo. **`BattleState.h` y `.cpp` están sucios** (hay trabajo de
   combate de otra sesión sin commitear): esta fila espera a que se commitee.
3. **El menú de combate** (`BattleMenuActions` y quien lo pinta) pregunta de quién es el
   turno en vez de suponer que siempre es del jugador.
4. **El canon.** El GDD dice `1d20 + DES`. La decisión es del dueño del canon, así que
   el motor la aplica, pero hay que llevarla aguas arriba (Onegai → cartas) para que el
   GDD y el motor no digan cosas distintas. `tier_rules.json` ya es compatible. Hasta
   entonces, `OrdenDeIniciativa` lleva escrito de dónde sale el d6.
5. **El plan.** El índice (§5) ya proponía un paquete nuevo de combate, porque
   `BattleState`, `ICombatant` y `BattleMenuActions` no tienen ningún «Fet quan». Este
   es su primer criterio.

**Criterio propuesto** (paquete nuevo de combate): *«En començar un combat, cada
combatent tira 1d6 + DES (+ modificadors de passives). S'actua de més alt a més baix i
l'ordre es manté. Empat: més DES, després el bàndol del jugador. Un combatent caigut
perd el torn. Prova determinista amb llavor fixa.»*

### 1.2 Una escena por pantalla

**Lo que hay.** C-16 está cumplido desde el 07/09: `Application` tiene un
`SceneManager` y un adaptador, `ApplicationScene`, con dos escenas, «Title» y «Game».
Pasar entre título y juego y cambiar de nivel son `requestReplace`. Todo lo demás son
**modos con banderas dentro de «Game»**: pausa, opciones, inventario, combate, game
over, creación de personaje y selector de guardados. La pila (`requestPush` y
`requestPop`) no la usa nadie. Y `Application.h` está en su techo: 519 de 519 líneas y
109 miembros, cuando la ley pide 15.

**Lo que esperamos** (G-08…G-17, que ya lo decían):

| Escena | La pide | Cómo entra | Estado que ya existe y se lleva |
| --- | --- | --- | --- |
| `TitleScene` | G-08 | sustituye | `TitleMenuState` |
| `CharacterCreationScene` | G-09 | sustituye | `PantallaNuevaPartida` (**sin versionar**), `CharacterCreationScreen` |
| `PauseScene` | G-10 | **se apila** sobre el juego | la pausa de `GameSession` (H0-03); `PauseMenuState.h` no existe aunque el plan lo cita |
| `OptionsScene` | G-17 (G-10) | **se apila** sobre pausa o título | `OptionsState`, `OptionsLocaleBridge` |
| `InventoryScene` | G-12 | **se apila** sobre el juego | `InventoryScreen` + `InventoryMenuState` (commiteados el 29/09, `c4b939e`) |
| `SaveLoadScene` | G-11 | se apila sobre pausa o título | `SaveSlotState` |
| `GameOverScene` | G-11 | sustituye | la pantalla de game over que hoy es un miembro de `Application` |

El combate no tiene escena en el plan. Es candidato natural (se apila sobre el mundo y
al acabar vuelve), pero no se añade sin decidirlo.

**Cómo.**

1. **Un contrato de servicios antes que la primera escena.** Cada escena recibe los
   colaboradores que necesita (sesión, HUD, idioma, entrada, recursos) en una estructura
   de datos, en vez de capturar `Application` entera como hace hoy el adaptador con sus
   `[this]`. Sin eso, cada escena nueva sería otra puerta a los 109 miembros.
2. **Relevos, una escena por relevo, de menos GL a más:** Opciones → Pausa →
   Inventario → Guardados → Game over → Título → Creación. Cada relevo **mueve** código
   (no lo reescribe) desde `Application*.cpp`, se lleva sus miembros de `Application.h`
   y actualiza la línea base en el mismo commit, que es el recibo de lo que baja
   (METODOLOGIA_RELEVOS, «corte doble» si el fichero tiene trabajo ajeno).
3. **La semántica de la pila se decide y se prueba en el primer relevo** (pausa): la
   escena de debajo se dibuja pero no se actualiza, y los atajos solo llegan a la de
   arriba. `test_scene_manager` y `test_application_scenes` crecen con esos casos.
4. **La creación de personaje va la última**: cuelga de `PantallaNuevaPartida.cpp`, que
   no está en git, y hay que versionarlo antes.

**Criterio de salida** (lo que dirá que C-16 y G-08…G-17 están hechos de verdad):
*`Application` no tiene banderas de modo; «dónde estoy» lo responde solo la pila de
escenas. Pausa, opciones e inventario son push/pop sobre el juego.* Y medible:
`Application.h` baja de su techo en cada relevo.

### 1.3 Enlazar lo que se pueda ya

Los cuatro subsistemas tienen dueños distintos, así que la decisión se aplica a cada
uno por separado:

| Pieza | Enlazar ya significa | Lo que lo limita |
| --- | --- | --- |
| `Audio` (miniaudio, 433 líneas, 1 prueba) | El juego arranca con el motor de audio vivo y los volúmenes de Opciones mueven los buses de música, efectos y ambiente | No hay ni un sonido en `assets/`: sin encargar los sonidos, el jugador no oye nada. Y el plan lo tiene **fuera del corte B** por escrito (`FORA`: «no prometre música, controls de volum») |
| `LootResolver` (1 prueba) | Al ganar un combate que viene de una tabla de encuentro, el botín es el del monstruo (H2-05) más `resolve_loot_table()` de la tabla de ese encuentro | Las tablas de encuentro no llegan al combate. Qué zona o qué spawn usa qué tabla es un dato del canon: si no está declarado aguas arriba, es un bug de contenido y se registra; no se inventa en el motor |
| `AdventureValidation` (E-07, 2 pruebas) | Corre en `motor_editor` dentro del panel de Problemas (§1.4) | Nada: ya se compila dentro de `motor_editor`. Falta que alguien llame al panel |
| `AdventureSerializer` (G-07, 2 pruebas) | Hoy no hay a qué enlazarlo: el juego no escribe aventuras y ningún editor las edita | Se queda como la mitad de escritura del formato de G-07, que es lo que prueba `SchemaFixtureTests`. Tendrá consumidor el día que haya un editor de aventuras, que hoy no está en el plan |

**Audio, cómo.**

1. **El plan primero:** sacar «Àudio, música i ambient» (GDB-11) de `FORA` y de la fila
   «Aplaçats expressament», y abrir un paquete mínimo de audio. Lo ha decidido Oriol, y
   si el plan sigue diciendo lo contrario, el próximo que lo lea lo deshará.
2. **Enlazar:** no hace falta tocar CMake. `Audio.cpp` ya está en `motor_rpg`, que el
   juego recibe a través de `motor_level`; no entra en el binario porque nadie lo usa.
   Quien posea los servicios (hoy `Application`; mañana el contrato de §1.2) crea el
   `AudioEngine`, y `OptionsState::volume()` pasa a mover los buses. Lo que sí hay que
   comprobar al hacerlo es qué necesita miniaudio en cada plataforma (en macOS, los
   frameworks de audio).
3. **Sin sonidos, el motor calla sin romper nada:** un fichero que no está es un error en
   el log (`Result`), no una excepción ni un cierre.
4. **Encargar los sonidos.** Hasta que existan, lo que el jugador ve es que los volúmenes
   ya no mienten.

*Criterio propuesto:* *«El joc arrenca amb el motor d'àudio viu. Els volums d'Opcions
canvien el bus corresponent i es desen. Un so que falta no atura res i surt al log.»*

**LootResolver, cómo:** llevar las tablas de encuentro al combate es una ruta de datos
(zona o spawn → tabla de encuentro → combate → tabla de botín). El código es poco; lo
que falta es que el canon diga qué zona usa qué tabla. El primer paso es mirar si
`LocationDefinition` o los spawns de nivel ya lo declaran. Si no, se registra como bug
de contenido aguas arriba y esta pieza espera.

### 1.4 El editor conecta toda su capa de estado

El detalle está en `07-editor.md`. El resumen:

| Grupo | Se conecta | Se sustituye (propuesta, necesita el visto bueno de Oriol) | Antes de conectar |
| --- | --- | --- | --- |
| Problemas (E-15, E-07) | `ProblemsState` + `ProblemsPanel` y la validación de E-07 | — | un dueño de los paneles fuera de `EditorApplication.cpp`; meter `EditorValidation` en la misma lista |
| Recursos y mapa (E-13, E-12) | `AssetBrowserState` | `AssetBrowserPanel` (retirarlo) y la mitad de capas de `MapPropertiesPanel` | un panel ImGui nuevo; las propiedades del mapa como vista del documento |
| Base de datos (E-16, E-14) | `CatalogPickerState`, alimentado por `RpgCatalogs` y el catálogo del proyecto | las fichas sin tipo de `ProjectDatabaseState` → las definiciones tipadas | validar con los mismos cargadores que usa el juego |
| Eventos (E-05, E-06, E-08) | `EventEditorState` | `CommandListState`, reescrita sobre `MapEventCommand` y `CommandSpec` | que el estado sea una vista de la capa `events` y comparta el historial del mapa |

**Cómo, en este orden:**

1. **Un dueño de los paneles.** `EditorApplication.cpp` está en 500 de 500 y sucio. Hace
   falta una clase nueva en `editor/src/` que posea cada panel y su estado, y a la que
   `run()` solo le pida dibujar, que es el patrón de `LayerPanel`, `ToolsPanel` e
   `InspectorPanel`. Antes, EDITOR-UX-01 tiene que estar commiteado.
2. **Problemas**, que es lo más barato porque ya se compila dentro. Con él, «los errores
   bloquean el playtest» empieza a ser verdad en el editor bueno.
3. **Recursos**, porque `AssetBrowserState` es la pieza mejor terminada de la capa.
4. **Base de datos**, que obliga a tipar las fichas.
5. **Eventos**, la última porque es la única que hay que reescribir antes de conectarla.

**Y dos cosas que no son de la capa de estado pero van en el mismo paquete de trabajo:**

- **Traer lo que solo tiene `level_editor`.** El editor bueno usa 30 de los 86 métodos
  de `EditorState`, y el antiguo 69. Empezando por `ObjectInspector`, que ya es GL-free
  y funciona por teclas lógicas: en `motor_editor` es un panel ImGui que pinta
  `lines()`. Detrás van las paletas con nombres legibles, la selección con copiar y
  pegar, y los botones de rectángulo, línea y cuentagotas de G-02.
- **G-03 de verdad.** Está cerrado con el criterio cambiado: el plan pide «comandes
  reversibles, no fotografies» y el código guarda hasta 256 copias del documento entero.
  Hay que reabrirlo con el criterio del plan, y ese historial por comandos es también
  donde deben confluir los tres historiales del editor.

---

## 2. El resto del juicio: lo que hay y lo que esperamos, por área

Lo que no dependía de ninguna decisión, con su primer paso.

| Área | Lo que hay | Lo que esperamos | Primer paso |
| --- | --- | --- | --- |
| Guardado | `CanSaveNow(mode, /*eventRunning=*/false)` en `Application.cpp:943`: se puede guardar a mitad de un evento (DEC-09) | La puerta pregunta a `m_eventInterpreter.canSave()` | Una línea y su prueba, en cuanto `Application.cpp` se commitee |
| Idiomas (G-17) | Funciona el camino feliz; los 8 métodos de verificación del `Locale` no los llama nadie | Claves que faltan, huérfanas y marcadores comprobados, con reserva al idioma de referencia | Una prueba en CTest que use `missingKeys`/`orphanKeys`/`checkPlaceholders` sobre los idiomas reales |
| Render (C-23) | La niebla funciona; la luz dinámica no tiene quien produzca luces; LUT, oclusión y sombras no se pueden activar desde el juego; el único asset las lleva apagadas | Un nivel publicado que las use | Un productor de luces (desde los spawns o el mapa) y un nivel de prueba con `lightingEnabled: true` |
| HUD (G-14) | `HudTimer` solo lo usan `ShopEngine` (muerto) y una prueba | Temporizadores visibles en el juego | Conectarlo al HUD real y retirar su uso desde `ShopEngine` |
| Creación de personaje (G-09) | Cuelga de `PantallaNuevaPartida.cpp`, **sin versionar**; «anterior» sin llamar | Volver atrás en el asistente; la cadena en git | Versionarla (su dueño) y después §1.2 |
| RPG (C-21, C-22) | Métodos impresos en las cabeceras de `DicePoolEngine`, `RandomEngine`, `TierRules` y `RuntimeTileMap` sin uso | «El que no es fa servir surt» | Retirarlos con la herramienta de declaraciones en verde |
| Código muerto con sustituto | `TitleScreen`, `BattleMenuState`, `ShopEngine`, `Render/Enemy.h` y `HandlePool` | Fuera | Una fila por pieza, con el sustituto nombrado. **Necesita el visto bueno de Oriol**: son borrados |
| Tamaño | `Application.h` 519/519 y `GameSession.h` 544/546 | Por debajo del techo y cerca de 15 miembros | §1.2 para `Application`; para `GameSession`, repartir datos (EST-23b), no ficheros |
| Pruebas donde no hay | `SerializerHelpers.h` (el guardado atómico) y `ResourceManager<T>` (todos los recursos GL), 0 pruebas; `ObjectInspector`, `ProjectHub` y `EditorValidation` solo se prueban en demos que no ejecuta nadie | Una prueba en CTest por cada una | Empezar por `atomicWrite` y `checkSchema`, que son lo que evita un guardado corrupto |
| Instrumento | `medir_alcance.py` no ve paraguas, especializaciones de plantilla ni `target_sources` | Tres reglas más antes de re-congelar la línea base (D0007) | Las tres, con sus casos sembrados |

---

## 3. Orden propuesto

Cada paso dice qué desbloquea. Los que tocan ficheros con trabajo ajeno esperan a que
ese trabajo se commitee.

| # | Paso | Desbloquea | Espera a |
| ---: | --- | --- | --- |
| 0 | Commitear el trabajo en vuelo: EDITOR-UX-01, G-15, el menú del jugador, `PantallaNuevaPartida`, el combate y el refactor de `GameSession` | Todo lo demás | Sus dueños (Oriol coordina) |
| 1 | `medir_alcance.py` con las tres reglas y re-congelar la línea base | Medir de verdad lo que baja o sube en cada paso | — |
| 2 | DEC-09: la puerta del guardado pregunta al intérprete | Que no se pueda guardar a mitad de un evento | 0 (`Application.cpp`) |
| 3 | Dueño de los paneles del editor + Problemas con la validación | Que el editor bueno bloquee el playtest con errores | 0 (`EditorApplication.cpp`) |
| 4 | Contrato de servicios de escena + Opciones y Pausa como escenas apiladas | Que `Application.h` empiece a bajar | 0 (`Application.*`) |
| 5 | Iniciativa 1d6 + DES (`OrdenDeIniciativa` + `BattleState`) | El combate del GDD | 0 (combate) |
| 6 | Audio enlazado y volúmenes reales | Que Opciones no mienta | el cambio del plan (§1.3) |
| 7 | Inventario, Guardados, Game over, Título y Creación como escenas | C-16 entero y G-08…G-17 | 4 |
| 8 | Editor: Recursos, Base de datos y Eventos; `ObjectInspector` en `motor_editor`; G-03 por comandos | La mitad del editor que solo tiene `level_editor` | 3 |
| 9 | `LootResolver` con las tablas de encuentro | El botín de las 201 tablas | la ruta en el canon |
| 10 | Retirar el código muerto con sustituto | 1.000 líneas menos de mantenimiento | el visto bueno de Oriol |

---

## 4. Lo que hay que cambiar en el plan

El diseño no se sostiene si `dades.py` sigue diciendo lo contrario:

1. **Audio:** sacar GDB-11 de `FORA` y de «Aplaçats expressament», y abrir su paquete.
2. **Combate:** el paquete nuevo, con el criterio de §1.1.
3. **G-03:** reabrirlo con su criterio original («comandes reversibles, no fotografies»).
4. **Escenas:** nada que cambiar; el plan ya las pedía. Lo que cambia es que C-16 se da
   por cumplido y las siete escenas pasan a ser el trabajo de G-08…G-17.
5. **Aguas arriba:** la iniciativa con d6 en el GDD (Onegai → cartas).

`dades.py` alimenta `PLA_DEFINITIU.md`, el Gantt y los bloques de cada fichero, así que
estos cambios se hacen en él y se regenera. Ojo: `build.py` lee también los
`seguimiento/*.json`, y varios de otras sesiones no están versionados.
