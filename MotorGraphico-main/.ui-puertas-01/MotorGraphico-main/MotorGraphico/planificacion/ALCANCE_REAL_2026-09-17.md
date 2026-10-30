# Alcance real — 17/09/2026: qué puede usar el jugador y qué puede usar el autor

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe el alcance real medido ese dia: que podia usar el jugador y que el autor, foto del 17/09/2026. El alcance se mide ahora con `tools/medir_alcance.py` sobre `planificacion/alcance_real.json`, y lo que falta para jugar esta en el gantt de la fase H y en `TABLERO.md`. De las 32 citas de codigo que contiene, 13 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

Medido sobre **HEAD `89f875e`** (rama `feature/editor-s1-contratos`), no sobre el plan ni sobre los registros de `seguimiento/`. Autor: Claude (Cowork).

**La pregunta.** No es «¿existe el código?», ni «¿pasa su prueba?», sino **«¿puede usarlo alguien?»**: el jugador con `juego` y el autor con `motor_editor` (`level_editor` es el editor antiguo).

**Cómo se reproduce:** `python3 tools/medir_alcance.py --json planificacion/alcance_real.json --comprobar`. El JSON trae el detalle fichero a fichero. La herramienta mide por **uso**, no por inclusión:
- **Cabeceras:** una cabecera cuenta si un fichero ya alcanzado la incluye **y además nombra algo que declara**; solo entonces sus `.cpp` pasan a estar alcanzados.
- **Punto de partida:** la única entrada es el fichero con `main()`. Estar enlazado no es estar usado.
- **Casos concretos:** los hallazgos de las secciones 2–4 se comprobaron a mano, uno por uno, con `grep` sobre el código (se cita dónde).

**Límites:**
- Es un analizador de texto, no un compilador.
- La lista de «métodos que nadie de fuera llama» no ve las llamadas internas de la propia clase. Por eso no se cita ninguno sin haberlo comprobado.
- Las rutas de contenido siguen puertas (`targetLevel`), no pasos: que se pueda **andar** hasta cada puerta lo mide `tools/conectividad.py`.
- En el árbol de trabajo hay trabajo en vuelo que HEAD aún no tiene: la interfaz de G-02 (cursor), el panel de E-15 y el puente de E-09 (qwen). Si entra, cambia la sección 3.

---

## 1. En una frase por lado

- **Jugador.** Llega a la exploración, las puertas, el diálogo con los PNJ que su catálogo conoce, la tienda, los negocios, el combate con un menú fijo, el uso de pociones y un único guardado rápido.
  - **No llega:** el sistema de eventos entero (fase E), la creación de personaje, la pausa, el idioma, las ranuras de guardado, el equipo ni el menú de combate desde datos.
  - **Incluso en la campaña**, casi todos los objetos de la ciudad están inertes (sección 4).
- **Autor.** En `motor_editor` puede abrir un nivel del proyecto, pintar, borrar y rellenar baldosas, colocar y quitar objetos, fijar el inicio, gestionar capas de baldosas, deshacer y guardar.
  - **No puede:** crear proyectos ni mapas, editar propiedades de instancias, escribir eventos, ver problemas, elegir recursos o datos por nombre, editar datos RPG, probar la partida ni exportar.
  - Algunas de esas cosas solo existen en el editor antiguo.

| | cabeceras de producción usadas | ficheros alcanzados | fuentes que enlaza |
| --- | ---: | ---: | ---: |
| `juego` (jugador) | 95 | 153 | 73 |
| `motor_editor` (autor) | 57 | 90 | 82 |
| `level_editor` (antiguo) | 67 | 104 | 74 |

**24 de las 132 cabeceras no las usa ningún ejecutable** (sección 5).

---

## 2. La ruta del jugador, tecla a tecla

| Donde | Qué hay | Llega | Evidencia |
| --- | --- | --- | --- |
| Título | Campaña · Continuar · Cargar partida · un escenario por proyecto jugable · Salir | sí | `Game::TitleScreen` (índice global). `este_sur` y `mundo` no salen: sin entrada. |
| Título → Campaña | prólogo en El Vacío → Día 1 en la ciudad por narrativa | sí, pero ver la sección 4 | `Application::update` encadena las aventuras. |
| Título → Continuar / Cargar | los dos cargan `savegame.json` | sí | «Cargar partida» es `// TODO: mostrar selector de guardados (G-11)`. |
| Explorar | WASD, E interactuar, Tab inventario, F5/F9 guardar y cargar | sí | `InputSystem.cpp` 20-34. |
| **Esc** | **sale del juego** | — | `bind(InputAction::Quit, InputKey::Escape)`. No hay acción de pausa. |
| Pausa (G-10) | menú de 8 opciones | **no** | Nadie llama a `GameSession::pause()`; el menú solo se ve con `mode()==Pause`. Además, «Reanudar» y Cancelar llaman a `closeInteraction()`, que no sale de Pause: si se entrara, no se podría volver. «Opciones» es un `TODO`. |
| Inventario → Usar | poción/éter | **sí** | Pestaña «Inventario» → `GameSession::useItem` (G-12). |
| Inventario → otras pestañas | Equipo · Estado · Acciones | **no** | `MenuLeft`/`MenuRight` no tienen tecla asignada en `InputSystem.cpp`. |
| Equipar | — | **no** | Aunque se llegara a la pestaña, `setInventoryRules` no se llama nunca: responde «no hay catálogo de equipo cargado». `InventoryScreen` nunca crea la acción Desequipar. |
| Combate | Atacar · Tajo · Cura · Poción · Huir | sí, fijo | `Application.cpp:654`, lista escrita a mano. **G-13 (menú desde la ficha y el catálogo) no llega.** |
| Estados alterados | se aplican en combate | a medias | `BattleState` los pone. `ConditionEngine::tick_conditions` (el único sitio que resta rondas, `ConditionEngine.cpp:211`) solo lo llaman pruebas y demos: **en el juego un estado no caduca nunca**. |
| Nueva partida con personaje (G-09) | — | **no** | `CharacterCreation` y `CharacterCreationScreen` no los usa ningún ejecutable. |
| Idioma (G-17) | — | **no** | `Engine/Locale` no lo usa ningún ejecutable. |
| Ranuras (G-11) | — | **no** | `Game::SaveSlots` no lo usa ningún ejecutable; todo va a `savegame.json`. |
| Eventos de mapa (E-01…E-09) | páginas, condiciones, comandos, interruptores, tiendas y combates por evento | **no** | `LevelLoader` lee `mapDocument`, pero `Application` no lo usa. `EventInterpreter`, `SessionCommandHost` y `EventCommand` no los usa ningún ejecutable. **La misión de E-09 (`assets/misiones/mision_llave.json`) no se puede jugar.** |
| Título desde el manifiesto (G-08) | — | **no** | `TitleMenuState` (de hoy) aún sin cablear. |
| Post-procesado (C-23) | niebla y luz dinámica | sí | `setFog`/`setPostFX` en `Application.cpp:963-971`. `setLUT` nunca se llama. |

---

## 3. La ruta del autor en `motor_editor`

**Qué hay:** 14 acciones (`EditorActions`): Guardar, Cerrar, Salir, Deshacer, Rehacer, Pintar, Borrar, Rellenar, Inicio del jugador, Objetos, Quitar objeto, Centrar, Ayuda y Capas. Además, el árbol de proyectos y niveles, la paleta, el viewport y el panel de capas de baldosas (`LayerPanel`).

| Lo que el plan promete al autor | En `motor_editor` | Dónde está hoy |
| --- | --- | --- |
| Crear proyecto o mapa, cambiar nombre, duplicar (E-11) | no | `ProjectIndex::createNamed/duplicate/suggestId` no se llaman desde ningún editor. |
| Propiedades de instancia: botín, destino de puerta, patrulla, evento… (E-12) | no | Los `EditorState::setObject*` solo los llama el editor antiguo (`ObjectInspector`). |
| Cuentagotas, rectángulo, línea (G-02) | no en HEAD | Núcleo en `EditorState`; el cableado está en vuelo (cursor). |
| Páginas de evento y lista de comandos (E-05, E-06) | no | `EventEditorState` y `CommandListState` no los usa ningún ejecutable; `editor/src/event_editor.cpp` no está en `motor_editor`. |
| Depurador paso a paso (E-08) | no | Solo `EventInterpreter`, que no usa nadie. |
| Problemas y búsqueda (E-15) | no | `ProblemsPanel.cpp` se enlaza en `motor_editor`, pero `EditorApplication` no lo llama (qwen en curso). |
| Recursos (E-13), selectores (E-14), base de datos RPG (E-16) | no | Sin consumidor. |
| Probar desde el editor (F-01) | no | Solo `level_editor` (tecla P/F7 → `./juego --nivel`). |
| Exportar (G-06, G-15) | no | No existe. |

---

## 4. Contenido: lo que hay en `assets/` frente a lo que carga una ruta

**Niveles.** De 98, **96** están en alguna ruta del título (el mundo está muy enlazado por puertas). Los otros dos son de prueba: `ejemplo_nivel.json` y `test_level.json`. No hay puertas rotas.

**La campaña carga un solo catálogo de objetos.** `juego` arranca la campaña con `assets/objects/boundington_npcs.json`, y «Campaña» no pasa por el manifiesto de Boundington, que declara 15 catálogos.
- **391 referencias a objetos** de los 94 niveles de esa ruta no tienen definición. `GameSession` las crea como objetos inertes, que no hablan, no venden, no pelean y no bloquean el paso.
- **Solo en los 23 niveles propios de Boundington:** 96 referencias sin definición (64 props y 32 PNJ), todas definidas en los catálogos del manifiesto.
- **Consecuencia concreta:** el tutorial del Día 1 pide hablar con `tendero_mercado`, `joyero` y `sastre`. Los tres están en `ciudad_objetos.json`, no en el catálogo de la campaña, así que **desde «Campaña» ese tutorial no se puede completar**. `test_boundington_tutorial` pasa porque prueba el guion sin catálogo.

**Escenarios del título:**

| Proyecto | Entrada | Niveles declarados / alcanzables por puertas | Objetos sin definición en su ruta |
| --- | --- | --- | ---: |
| ejemplos | nivel | 2 / 2 | 0 |
| este_norte | nivel | 55 / 93 | 0 |
| oeste_norte | aventura | 4 / 93 | 332 |
| este_sur | — (no sale en el título) | 3 / 0 | — |
| mundo | — (no sale en el título) | 8 / 0 | — |

Los números de alcanzables salen altos porque las puertas llevan a la red compartida del mundo. En `oeste_norte`, esos niveles traen objetos cuyos catálogos ese proyecto no declara.

**Catálogos RPG.** El juego lee **3 de 24**: `skills`, `conditions` y `monsters`.
- **No los lee:** `equipment` (413 piezas), `consumables`, `classes`, `races`, `backgrounds`, `feats`, `traits`, `passives`, `deities`, `spells` (210), `summons`, `mounts`, `traps`, `loot_tables`, `npcs`, `locations`, `events` ni `adventures`.
- **Qué implica:** el trabajo de D-02…D-07 sobre esos catálogos solo lo ven las pruebas.

**Aventuras.**
- **Siempre cargadas:** las cuatro de Boundington.
- **Solo con `juego --precuela`:** `boundington_precuela_taberna`.
- **Como entrada de proyecto:** `on_camino_del_norte`.
- **No las carga ninguna ruta:** `aventura_silbido_medianoche`, `este_norte_senal_ceniza` y `plaza_de_las_razas`.

**Mapas nativos con eventos:** solo existe `assets/misiones/mision_llave.json`, y el juego no lo lee.

---

## 5. Cabeceras que no usa ningún ejecutable (24)

| Cabecera | Paquete | Comentario |
| --- | --- | --- |
| `RPG/EventInterpreter.h`, `RPG/EventCommand.h`, `RPG/SessionCommandHost.h`, `RPG/AdventureValidation.h` | E-03, E-04, E-07 | El núcleo de la fase E, con 13 pruebas entre todas. |
| `Editor/EventEditorState.h`, `Editor/CommandListState.h`, `Editor/ProblemsState.h`, `editor/ProblemsPanel.h` | E-05, E-06, E-15 | Editor de eventos y problemas. |
| `Editor/CatalogPickerState.h`, `Editor/AssetBrowserState.h`, `Editor/ProjectDatabaseState.h` | E-13, E-14, E-16 | `AssetBrowserState` no tiene ninguna prueba en HEAD. |
| `RPG/CharacterCreation.h`, `RPG/CharacterCreationScreen.h` | D-06, G-09 | Creación de personaje. |
| `Engine/Locale.h` | G-17 | Idioma. |
| `Game/SaveSlotState.h` | G-11 | Ranuras. |
| `Game/InventoryMenuState.h`, `Game/TitleMenuState.h` | G-12, G-08 | Escritas hoy; sin cablear. |
| `RPG/LootResolver.h`, `RPG/Audio.h` | GDB-10, GDB-11 | Botín y audio. |
| `Render/Enemy.h` | — | El juego dibuja los enemigos por el World, no por esta clase. |
| `Core/Memory/HandlePool.h`, `Level/WorldTopology.h`, `Project/DocumentRef.h` | C-18 y ninguno | Ya señaladas el 03/09. |
| `Editor/ProjectIndex.h` | — | Reexporta `Project/ProjectIndex.h` (6 líneas). |

---

## 6. Paquetes que su registro da por cerrados y el jugador no ve

Lo más grave: dos paquetes «cerrados» (G-13 y G-14) tienen **la lógica escrita dentro de su propio fichero de prueba**, y no en el código del juego. Sus pruebas se prueban a sí mismas. En G-10 pasa en parte:

| Paquete | Registro | Lo que hay de verdad |
| --- | --- | --- |
| **G-13** Comandas de combate desde datos | `closed` | `buildBattleMenuSkillActions`, `selectTargets` y `translateToBattleAction` solo existen en `tests/Game/G13CombatCommandsTests.cpp`. El juego usa el menú fijo. |
| **G-14** HUD, diálogos y comercio | `closed` | `HudTimer` y `ShopEngine`/`ShopTransaction` solo existen en `tests/Game/G14HudTimersAndCommerceTests.cpp`. |
| **G-10** Pausa, opciones y foco | `cerrado` | La prueba usa el `PauseMenu` real, pero además define en el propio fichero sus `MenuLogic` y `CmdMenuLogic`. En el juego la pausa es inalcanzable y «Opciones» es un `TODO` (sección 2). |
| **G-09** Creación dentro de Nueva partida | `cerrado` | `CharacterCreationScreen` existe y tiene pruebas, pero `Application` no la usa. |
| **G-17** Idioma seleccionable | `closed` | `Locale` existe y tiene pruebas; ningún ejecutable la usa. |
| **E-09** La misión de la llave | abierto | El registro ya lo dice: la misión existe como fichero y en su demo, pero no se juega ni se escribe desde el editor. |

La tabla fichero a fichero de todos los paquetes está en `alcance_real.json` → `paquetes`. **Está medida por fichero, así que es optimista:** un paquete que solo toca `Application.cpp` aparece como alcanzado aunque su función no lo esté. Por eso las conclusiones de esta sección salen de las comprobaciones de las secciones 2–4, no de esa tabla.

---

## 7. Qué haría, por rendimiento

1. **Una sesión de cableado en `Application` (fichero de cruce, con fila propia).** Casi todo lo que falta al jugador está a una llamada de distancia:
   - campaña con los catálogos del manifiesto de Boundington (`openProject`);
   - tecla para `MenuLeft`/`MenuRight`;
   - Esc → `pause()`, y que Reanudar llame a `resume()`;
   - `setInventoryRules` con `equipment.json`;
   - `SaveSlots` para F5/F9 y para «Cargar partida»;
   - `tick_conditions` por ronda de combate.

   Cada punto con una prueba de `Application` (el arnés de C-24 ya existe).
2. **Ejecutar eventos en el juego.** Cargar el `mapDocument` del nivel y correr `EventInterpreter` con `SessionCommandHost` como fallback. Sin esto, la parte de eventos de la fase E (E-01…E-09 y E-15, 49 de sus 79 jornadas) no llega a nadie; con esto, E-09 se puede jugar.
3. **Mover a producción la lógica que vive en pruebas** (G-13, G-14 y la parte de G-10) y reabrir esos registros, junto con G-09 y G-17: hoy están cerrados sin estarlo.
4. **Regla de cierre.** Un paquete de interfaz o de juego no se cierra si `tools/medir_alcance.py` dice que sus ficheros de producto no los usa ningún ejecutable, o si su prueba define lo que prueba. Registrarlo en CTest para que la regla se compruebe sola: toca `CMakeLists` y lo decide Oriol.
