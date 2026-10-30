# Juicio clase a clase del motor — índice y mapa general

Generado el 28/09/2026 sobre la medición de ese día. Esto es el **juicio escrito
encima del inventario**: el inventario lo produce `tools/mapa_clases.py` y no se
edita a mano; el juicio es prosa y por tanto empieza a mentir en cuanto alguien
parte un fichero. Es lo que le pasó a `analisis_clases/` (31/08, 101 cabeceras) y a
`planificacion/analisis/` (mismo día). Ambos quedan como foto histórica; **el juicio
vigente es este**, y su evidencia se regenera con `medir_alcance.py --json` +
`mapa_clases.py`.

**Sustituye a:** `analisis_clases/INDEX.md` y `planificacion/analisis/00-sintesi.md`
como descripción del estado actual. No los borra porque documentan el árbol del
31/08 y la comparación es útil: entonces 101 cabeceras y ~23.500 líneas; hoy 213 y
54.233.

---

## 0. Correcciones del 29/09

La revisión del 29/09 volvió a comprobar las premisas de este índice contra el árbol
y contra los binarios, y cinco no se sostenían tal como estaban escritas. Se corrigen
en su sitio y se resumen aquí, porque mueven cifras de las secciones de abajo:

1. **`SceneManager` está conectado desde el 07/09** (`d0fe012`, C-16, con 10 casos en
   `test_application_scenes`). Pasar entre título y juego y cambiar de nivel son
   `requestReplace`, y el bucle llama a `fixedUpdate`/`render` a través de la escena
   activa: hay 40 símbolos de `SceneManager` en el binario del juego del 28/09. Lo que
   no usa nadie es la pila (`requestPush`/`requestPop`). El hallazgo 2 de §3 decía lo
   contrario; está reescrito.
2. **`ProjectHub.h` no está muerto.** Lo usa `level_editor` a través de
   `LevelEditorHub.cpp`, que entra en el ejecutable por `target_sources` (35 símbolos
   en su binario del 26/09). `medir_alcance.py` no lee `target_sources`: es un
   **tercer punto ciego** (§3.20). Con él corregido, las cabeceras que no alcanza
   ningún ejecutable son **35 y 4.127 líneas**, y la capa de estado del editor,
   **11 cabeceras**.
3. **La capa de estado sumaba 1.350 líneas, no 1.264**: el total no cuadraba con su
   propia tabla (§4). Sin `ProjectHub`, son 1.218; y 1.207 después de quitar el
   bloque repetido de `ProjectDatabaseState.h`.
4. **De las 130 cabeceras «sin criterio», 10 sí lo tienen**: están en un paquete de
   `dades.py` y solo les falta el bloque. Son `LayerPanel.h` (E-12),
   `EditorActions.h` (E-10), `TitleMenuState.h` (G-08), `ProjectSettings.h` (E-11) y
   seis de G-12. Sin paquete en el plan quedan 120. Y la propuesta de §5 chocaba con
   tres que ya son de G-12: `PlayerControls.h` y `PlayerControlsData.h` (propuestas
   para un paquete nuevo de controles) y `AnimatedEntity.h` (propuesta para C-19).
5. **`EditorIconos` no duplica `HUD/Iconos`: lo usa**, porque su manifiesto es un
   `Hud::Iconos`. Las duplicaciones de concepto de §3.17 eran cuatro, no cinco.

Y dos avisos de método. Lo de los cuatro subsistemas de §3.9 se verificó con el
binario del 26/09 y se ha repetido con el del 28/09 (`out/build/opencode-h007/juego`,
32.257 símbolos): siguen sin estar. No están porque nadie los usa, no porque el juego
no enlace su librería: `motor_rpg` llega a `juego` a través de `motor_level`. Y los
`MAPA_CLASES_*` commiteados salen del árbol de trabajo, no de HEAD: el de
`motor_editor` cita cuatro cabeceras que no están en git (`EditorAyuda.h`,
`EditorIconos.h`, `InspectorPanel.h` y `ToolsPanel.h`).

---

## 1. El mapa general, en números

| Ejecutable | Para quién | Fuentes enlazadas | Ficheros alcanzados | Cabeceras | Clases |
| --- | --- | ---: | ---: | ---: | ---: |
| `juego` | jugador | 128 | 240 | 142 | 260 |
| `motor_editor` | autor | 148 | 132 | 76 | 112 |
| `level_editor` | autor (editor antiguo) | 133 | 135 | 81 | 126 |

**La unión, que es el árbol real:**

- **177 cabeceras** alcanzadas por algún ejecutable, **297 clases** distintas,
  **16.624 líneas** de cabecera, **1.194 pruebas** asociadas.
- **36 cabeceras que no alcanza ningún ejecutable**, 4.259 líneas. Son el 17 % de las
  cabeceras y el 20 % de las líneas del árbol.
- El árbol medido tiene 213 cabeceras: 177 vivas + 36 muertas o solo de pruebas.

**Quién comparte qué:**

| La alcanzan | Cabeceras |
| ---: | ---: |
| Los tres ejecutables | 43 |
| Dos | 36 |
| **Uno solo** | **98** |

El 55 % del árbol lo usa un único ejecutable. Y el dato que más dice sobre la
relación juego/editores: **el juego alcanza 260 de las 297 clases (88 %) y los dos
editores juntos añaden 37.** El editor no es un mundo paralelo, es un subconjunto más
una capa propia — y esa capa propia es justo la que no llega a ningún binario
(sección 4).

**El hueco de criterio:** **130 de las 177 cabeceras vivas no llevan bloque
`PAQUETES DEL PLAN`**, o sea que para el 73 % del árbol no existe ningún «Fet quan»
escrito contra el que juzgar «qué debería hacer». Por zonas:

| Zona | Sin criterio | | Zona | Sin criterio |
| --- | ---: | --- | --- | ---: |
| `Core` | 26 | | `Engine` | 9 |
| `Game` | 22 | | `Editor` | 5 |
| `Render` | 22 | | `Level` | 4 |
| `RPG` | 18 | | `Project` | 4 |
| `editor/include` | 13 | | `Map`, `Runtime` | 2 cada una |
| | | | `HUD`, `Import`, `World` | 1 cada una |

Oriol decidió el 28/09 que donde no hay criterio **no se inventa**: se señala y se
propone la asignación para que él la acepte o la corrija. Las propuestas están en
cada fichero de juicio, y el resumen en la sección 5.

---

## 2. Los ficheros del juicio

| Fichero | Zonas | Cabeceras | Clases | Estado |
| --- | --- | ---: | ---: | --- |
| [01-render.md](01-render.md) | Render (+`Enemy.h` muerta) | 35 + 1 | 43 | escrito |
| [02-rpg.md](02-rpg.md) | RPG (+13 inalcanzables) | 29 + 13 | 68 | escrito |
| [03-game.md](03-game.md) | Game (+4 inalcanzables) | 26 + 4 | 57 | escrito |
| [04-core.md](04-core.md) | Core (+2 inalcanzables) | 28 + 2 | 39 | escrito |
| [05-engine.md](05-engine.md) | Engine | 12 | 18 | escrito |
| [06-mapa-nivel-proyecto.md](06-mapa-nivel-proyecto.md) | Map, Level, Project, World, Runtime, Import, HUD | 23 + 4 | 43 | escrito |
| [07-editor.md](07-editor.md) | Editor, editor/include | 25 + 11 | 59 | escrito (29/09) |

**El juego y el editor están juzgados enteros.** `07-editor.md` (29/09) cubre las 36
cabeceras de `include/Editor/` y `editor/include/`: 25 que llegan a algún editor y las
11 de la capa de estado (EST-42). Las demás clases que solo alcanzan los editores —el
log, `ScopeGuard`, `Check.h` y `ProjectCatalog`— están juzgadas en `04` y `06`. El
diseño que sale de todo esto, con las decisiones de Oriol del 29/09, está en
[08-diseno.md](08-diseno.md).

Cada fichero abre con los hallazgos del área y sigue con una entrada por clase: qué
hace, qué debería hacer, qué se lo impide y cómo optimizarla.

---

## 3. Los hallazgos que cruzan áreas, ordenados por lo que cuestan

No son opinión: cada uno está verificado con `git grep` o contra el binario en el
fichero que se indica.

### Los que rompen una decisión o un criterio ya aceptado

1. **DEC-09 no está cableado: el jugador puede guardar a mitad de un evento.**
   `Application.cpp:943` pasa `/*eventRunning=*/false` como constante a
   `Game::CanSaveNow()`, y `Application` posee `m_eventInterpreter` cuya
   `canSave()` existe justamente para responder eso y no la llama nadie. Ninguna
   prueba falla porque la puerta es correcta para la entrada que recibe. →
   `02-rpg.md` §1. **Fichero sucio, no se toca: se reporta.**
2. **C-16 está cumplido; lo que falta son las escenas de G-08…G-17.** *(Corregido el
   29/09: aquí decía que `SceneManager` estaba sin conectar.)* Pasar entre título y
   juego y cambiar de nivel son transiciones de escena desde `d0fe012` (07/09). Pero
   el plan nombra siete escenas que nunca se crearon (`TitleScene`,
   `CharacterCreationScene`, `PauseScene`, `SaveLoadScene`, `GameOverScene`,
   `InventoryScene` y `OptionsScene`): hoy son modos con banderas dentro de una sola
   escena «Game», y la pila (`requestPush`/`requestPop`) no la usa nadie. Oriol
   decidió el 29/09 que **manda el plan: una escena por pantalla**. → `05-engine.md`
   §2 y `08-diseno.md` §1.2.
3. **C-12 a medias: la API de mutación del mapa existe completa y el editor no la
   usa.** `moveEntity`, `moveEvent` y `moveLayer` sin llamar desde los dos
   editores; el único llamador de `moveEvent` es `editor/src/event_editor.cpp`, que
   a su vez no lo alcanza ningún ejecutable (EST-42). → `06` §2.
4. **G-17 sin su mitad de verificación.** `Locale` tiene 8 métodos sin llamar y son
   justo los que el criterio pide: `referenceLanguage` («reserva a l'idioma de
   referència»), `missingKeys`/`orphanKeys` (DEC-15, «cobertura de font
   comprovada»), `checkPlaceholders`. → `05-engine.md` §3.
5. **C-23 a medias y sin ejercitar.** La niebla está conducida de verdad; la luz
   dinámica se enchufa pero **no tiene productor de luces** (`addLight` sin ninguna
   llamada); LUT, oclusión y blob shadows no tienen camino de activación desde el
   juego; y el único asset publicado lleva `fogEnabled: false` y
   `lightingEnabled: false`. → `01-render.md` §2-4.
6. **G-14 sin sus temporizadores.** `HudTimer.h` no lo alcanza ningún ejecutable;
   sus únicos consumidores son `ShopEngine.h` (también muerto) y
   `G14HudTimersAndCommerceTests.cpp`. Una prueba verde ejercitando dos piezas que
   el jugador nunca ve. → `06` §4.
7. **G-09 sin «tornar».** `CharacterCreationScreen::anterior` sin llamar, y toda la
   cadena de creación de personaje cuelga de `PantallaNuevaPartida.cpp`, que **no
   está versionado**. → `03-game.md`, `05-engine.md` §4.
8. **C-21 y C-22 incumplidos en las cabeceras que los llevan impresos.**
   `DicePoolEngine` (`es_pifia`, `resolve_opposed`), `RandomEngine` (`seed`,
   `roll_in_range`), `TierRules` (`at`), `RuntimeTileMap` (`getTile`, `layerAt`,
   `getLayerCount`). → `02-rpg.md` §5, `06`.

### Los que son código que no llega a nadie

9. **Cuatro subsistemas compilados que no están en el binario del juego**, verificado
   con `nm -C` sobre 27.140 símbolos (control positivo: `CharacterSheet` 64,
   `GameSession` 224): `Audio`, `LootResolver`, `AdventureSerializer`,
   `AdventureValidation`. El juego no tiene sonido y no resuelve botín. →
   `02-rpg.md` §2. *(29/09: repetido con el binario del 28/09, 32.257 símbolos, y
   siguen sin estar. Pero no son una decisión sino cuatro: `Audio` estaba aplazado por
   escrito —`FORA` de `dades.py`—, `LootResolver` espera a que las tablas de encuentro
   lleguen al combate, `AdventureValidation` es del editor —se compila dentro de
   `motor_editor` y la llama `ProblemsState`— y `AdventureSerializer` solo lo usa la
   prueba de G-07. Oriol decidió enlazar lo que se pueda ya: ver `08-diseno.md`
   §1.3.)*
10. **`Render/Enemy.h` muerto en producción y vivo en 6 demos**, con `.cpp` y 7
    pruebas verdes. C-17 ya lo sustituyó por `WorldEnemy`; las dos referencias que
    parecen mantenerlo vivo son comentarios. → `01-render.md`.
11. **Tres gemelas muertas en Game:** `TitleScreen`, `BattleMenuState`, `ShopEngine`
    — 206 líneas y 3 pruebas verdes que no prueban nada que llegue al jugador. →
    `03-game.md` §2.
12. **`HandlePool.h` (179 líneas) muerto y `World/EntityHandle.h` (13) vivo**: el
    mismo concepto dos veces. → `04-core.md` §3.
13. **La capa de estado del editor no llega a ningún editor**: 11 cabeceras y 1.218
    líneas (sección 4; hasta el 29/09 decía «12 y 1.264»: `ProjectHub` sí llega a
    `level_editor`, y el total no cuadraba con su tabla). Es EST-42 y es lo más grande
    del inventario de muertos. `ProblemsState` y `ProblemsPanel` sí están *dentro* del
    binario de `motor_editor`, sin que nadie los llame. Oriol decidió el 29/09
    conectarla entera: ver `07-editor.md`.

### Los que son tamaño o forma

14. **`Application.h` está exactamente en su techo: 519 de 519**, con 109 miembros y
    441 líneas de cuerpo. Cero margen para crecer sin subir el trinquete (D0007). →
    `05-engine.md` §1.
15. **`GameSession.h` a dos líneas del suyo: 544 de 546**, 469 de cuerpo y seis
    `.cpp`. Partirla por ficheros no la partió por responsabilidades. → `03-game.md` §1.
16. **El combate no tiene iniciativa y tres sitios la definen.** `BattleState.h:24`
    («deliberadamente simple, sin velocidad/iniciativa») contra `ICombatant::
    initiative_bonus()`, `tier_rules.json:93` («§5: Iniciativa = DES +
    Modificadores») y la copia huérfana borrada esta semana. → `03-game.md` §3.
    *(29/09: Oriol decidió que **sí hay iniciativa, 1d6 + DES**. El GDD dice 1d20 y hay
    que cambiarlo aguas arriba. Ver `08-diseno.md` §1.1.)*
17. **Cuatro duplicaciones de concepto** encontradas al juzgar: catálogos de
    habilidades (`Game::SkillCatalog` contra `Catalogs::SkillCatalog`), entrada
    (`InputState` contra `InputSnapshot`), controles (`PlayerControls` contra
    `InputMap`) e inventario visible (`InventoryScreen` contra `InventoryOverlay`).
    → repartidas en `03`, `05`, `06`. *(29/09: la quinta, la de iconos, no lo era:
    `EditorIconos` lee su manifiesto con `HUD/Iconos`. Y el editor añade tres: un
    comando de evento con tres representaciones, dos navegadores de recursos
    —`AssetBrowserState` y `AssetBrowserPanel`— y tres historiales de deshacer. Ver
    `07-editor.md`.)*
18. **Pruebas donde no toca.** `SerializerHelpers.h` tiene **0 pruebas** y contiene
    `atomicWrite` y `checkSchema`, que son lo único que evita un guardado corrupto.
    `ResourceManager<T>` tiene 0 pruebas y es el dueño de todos los recursos GL.
    `JsonWriter` tiene 1 para un criterio (C-10) que habla de todo lo que escribe. →
    `06`, `04-core.md` §2 y §4.

### Los que son del instrumento, y cambian cómo se lee todo lo anterior

19. **La línea base congelada da por muertas 9 cabeceras que el juego alcanza hoy.**
    El mapa no repite la etiqueta: la contradice donde toca.
20. **`medir_alcance.py` tiene dos puntos ciegos, cuantificados:** no ve las
    **especializaciones de plantilla** (los siete `CatalogLoaders*.h`, 1.710 líneas,
    que `RpgCatalogs.h:85,111` sí instancia) ni los **paraguas sin símbolos propios**
    (`RpgCoreDefinitions.h`, 15 consumidores). 8 de las 13 inalcanzables de RPG son
    falsos negativos: **62 %**, contra el 40 % de falsos muertos de la línea base. →
    `02-rpg.md` §3. *(29/09) **Y un tercero:** no lee `target_sources(...)`, así que no
    ve las fuentes que un `.cmake` añade a un ejecutable ya creado. Hoy son tres:
    `LevelEditorHub.cpp` en `level_editor` —y con ella `ProjectHub.h`, que daba por
    muerta— y `EditorGui.cpp` y `EditorVistaDeMapa.cpp` en `motor_editor`.*
21. **La lista de «métodos sin llamar» no mira dentro.** Comprobado: `purge_expired`
    aparece y se llama cuatro veces desde su propio `.cpp`; `removeLayer` aparece y
    tiene tres llamadores en el editor; `collectRent` es el ejemplo que la propia
    docstring del instrumento usa. **La lista sirve para mirar, no para afirmar**, y
    en estos ficheros cada afirmación importante se ha vuelto a comprobar.

---

## 4. Las 36 cabeceras que no alcanza ningún ejecutable

4.259 líneas. Con veredicto, porque «no la alcanza nadie» significa cinco cosas
distintas.

**Falsos negativos del instrumento (8 cabeceras, 1.727 líneas) — no están muertas.**

| Cabecera | Líneas | Por qué el instrumento no la ve |
| --- | ---: | --- |
| `RPG/Catalogs/CatalogLoadersMundo.h` | 297 | Especialización de plantilla que `RpgCatalogs.h:85,111` instancia |
| `RPG/Catalogs/CatalogLoadersRasgos.h` | 268 | Ídem |
| `RPG/Catalogs/CatalogLoadersCriaturas.h` | 265 | Ídem |
| `RPG/Catalogs/CatalogLoadersObjetos.h` | 258 | Ídem |
| `RPG/Catalogs/CatalogLoadersPersonaje.h` | 223 | Ídem |
| `RPG/Catalogs/CatalogLoadersHabilidades.h` | 208 | Ídem |
| `RPG/Catalogs/CatalogLoadersContenido.h` | 191 | Ídem |
| `RPG/Definitions/RpgCoreDefinitions.h` | 17 | Paraguas puro: seis `#include`, ningún símbolo propio, 15 consumidores |

**Doble de pruebas, correcto que no llegue (1).**

| Cabecera | Líneas | Veredicto |
| --- | ---: | --- |
| `Core/Log/MemorySink.h` | 48 | Es el sink de pruebas; la decisión de que viva en `include/` está documentada en `802153b` |

**La capa de estado del editor sin cablear (12 cabeceras en esta tabla y 1.350 líneas; en realidad 11 y 1.218, porque `ProjectHub` sí llega a `level_editor`) — EST-42.**

| Cabecera | Líneas | Pruebas | Implementación |
| --- | ---: | ---: | --- |
| `Editor/ProblemsState.h` | 277 | 1 | `ProblemsNavigation.cpp`, `ProblemsState.cpp` |
| `Editor/AssetBrowserState.h` | 215 | 1 | `AssetBrowserState.cpp` |
| `Editor/CatalogPickerState.h` | 135 | 1 | `CatalogPickerState.cpp` |
| `Editor/ProjectHub.h` | 132 | 1 | `ProjectHub.cpp` — **vivo en `level_editor`** (falso negativo, §3.20) |
| `Editor/EventEditorState.h` | 112 | 2 | `editor/src/event_editor.cpp` |
| `Editor/ProjectDatabaseState.h` | 89 | 2 | `ProjectDatabaseState.cpp` |
| `editor/include/ProblemsPanel.h` | 85 | 0 | `editor/src/ProblemsPanel.cpp` |
| `Editor/CommandListState.h` | 84 | 1 | `editor/src/event_editor.cpp` |
| `Editor/ExportState.h` | 73 | 1 | `editor/src/ExportPanel.cpp` |
| `Editor/MapPropertiesPanel.h` | 64 | 1 | header-only |
| `Editor/AssetBrowserPanel.h` | 53 | 1 | `AssetBrowserPanel.cpp` |
| `Editor/ProblemsInternal.h` | 31 | 0 | header-only |

Es el bloque más grande del inventario y el más importante del editor: los estados de
los paneles existen, tienen pruebas y **no llegan a `motor_editor` ni a
`level_editor`**. Dos de ellos los tocó esta misma semana el cierre de E-09
(`EventEditorState::save()`, que dejó de ser un stub en `25d464d`) y E-16
(`ProjectDatabaseState::save()`/`load()`, en `235fcf6`): se curaron implementaciones
de clases que ningún binario alcanza. La sesión del 28/09 ya lo dejó anotado como
EST-42; aquí está medido.

*(29/09) Oriol decidió **conectarla entera a `motor_editor`**. `07-editor.md` separa la
mitad buena de las cuatro copias que no tocan el documento, y propone sustituir esas
en vez de conectarlas tal cual.*

**Gemelas muertas con sustituto vivo (3 cabeceras, 206 líneas).**

| Cabecera | Líneas | La gemela que sí llega |
| --- | ---: | --- |
| `Game/BattleMenuState.h` | 78 | `Game/BattleMenuActions.h` |
| `Game/ShopEngine.h` | 70 | `GameSessionComercio.cpp` + `SessionTipos.h::ShopOffer` |
| `Game/TitleScreen.h` | 58 | `Game/TitleMenuState.h` |

**Sustituido por C-17 (1).**

| Cabecera | Líneas | Pruebas | Veredicto |
| --- | ---: | ---: | --- |
| `Render/Enemy.h` | 88 | 7 | Sustituido por `World/World.h::WorldEnemy`. Solo lo referencian comentarios y 6 demos |

**Subsistemas compilados que no entran en el binario (4, verificados con `nm`).**

| Cabecera | Líneas | Pruebas | Qué significa |
| --- | ---: | ---: | --- |
| `RPG/Audio.h` | 131 | 1 | El juego no tiene sonido |
| `RPG/AdventureValidation.h` | 130 | 2 | Es del editor (E-07): va con el panel de Problemas, que está dentro de `motor_editor` sin llamar *(corregido el 29/09)* |
| `RPG/AdventureSerializer.h` | 39 | 2 | La aventura no se serializa desde el juego |
| `RPG/LootResolver.h` | 21 | 1 | El botín no se resuelve por este camino |

**Duplicado muerto (1).**

| Cabecera | Líneas | Pruebas | Veredicto |
| --- | ---: | ---: | --- |
| `Core/Memory/HandlePool.h` | 179 | 2 | `World/EntityHandle.h` hace lo mismo en 13 líneas y sí llega |

**Piezas sueltas por decidir (6).**

| Cabecera | Líneas | Pruebas | Veredicto |
| --- | ---: | ---: | --- |
| `Level/WorldTopology.h` | 137 | 2 | La más grande del grupo. ¿Topología de autoría o de diseño? |
| `HUD/HudTimer.h` | 56 | 1 | La pieza de los temporizadores de G-14; solo la usan `ShopEngine` (muerto) y un test |
| `RPG/EventosInterno.h` | 49 | 0 | De sus 4 consumidores en `src/RPG/Eventos/`, solo `AnfitrionNarrativo.cpp` llega al juego |
| `Game/Companeros.h` | 45 | 1 | **SUCIO** y con el `.cpp` sin versionar: puede ser WIP vivo, no se toca |
| `Project/DocumentRef.h` | 29 | 1 | O se cablea con `ProjectIndex` o se retira |
| `Level/LevelSerializer.h` | 24 | 2 | Convive con `ProjectSerializer`; dos serializadores de nivel |

---

## 5. Las 130 cabeceras sin criterio: propuesta de asignación

*(29/09) Diez de las 130 ya tienen paquete en `dades.py` (§0.4). Y lo que frena el
marcado no era el bug del bloque duplicado, que estaba en un solo fichero y ya está
corregido en `marcar_paquetes.py`: lo frena que esta propuesta no está aceptada.
Cuando lo esté, se marca con `--solo`, fichero a fichero; la herramienta ya se salta
el trabajo sin commitear y no deja ningún fichero por encima de su techo.*

Oriol decidió que se señale y se proponga en vez de inventar especificación en
silencio. Esto es la propuesta, por paquete existente, para que la acepte o la
corrija. No se ha marcado ninguna cabecera: `tools/marcar_paquetes.py` es quien
escribe esos bloques, y hacerlo a mano lo revertiría la siguiente pasada.

| Paquete | Cabeceras que deberían llevarlo | Por qué |
| --- | --- | --- |
| **C-19** (un sol culling i un sol contracte d'actualització) | `Render/Entity.h`, `Render/Camera.h`, `Render/SpriteBatch.h`, `Render/HudManager.h`, `Render/IUpdatable.h`, `Render/AnimatedEntity.h`, `Core/Math/IsoMath.h`, `Core/Math/GridBounds.h`, `Runtime/RuntimeTile.h` | Son las piezas del contrato de coordenadas y de dibujo por frame |
| **C-09** (propietat de recursos) | `Core/Resources/ResourceManager.h`, `TextureAtlas.h`, `Shader.h` | `Texture.h` ya lo lleva; sus tres hermanas no |
| **C-22** (retirada de l'API morta) | `Core/Errors/Result.h`, `Core/Errors/EngineException.h`, `RPG/Defense.h`, `RPG/Definitions/SkillDefinition.h` | Es el criterio que decide qué API vive, y estas son las que tienen API sin consumidor |
| **C-21** (un sol generador d'atzar) | *(ya está en `RandomEngine.h`)* | — |
| **D-01** (un carregador que no troba res, falla) | `Level/LevelLoader.h`, `RPG/Catalogs/CatalogLoader.h` | Los dos son cargadores con contrato de campos |
| **D-02 / D-06** | `RPG/Definitions/SkillDefinition.h` (D-02), `RPG/Stat.h` y `SkillTarget.h` (D-06), las cinco `Definiciones*.h` (D-03..D-07 según familia) | D-02..D-07 describen estas fichas casi literalmente y ninguna lleva el bloque |
| **D-05 / G-14** | `Level/ObjectCatalog.h` | Sus `ShopData`/`ShopItem` son el comercio; sus 33 pruebas no tienen criterio |
| **D-08** | `Level/LevelDefinition.h::EnemySpawn` | «Un enemic del món es construeix des de la seva MonsterDefinition» |
| **G-07** (congelació dels esquemes) | `Game/SaveGame.h`, `Level/LevelDefinition.h`, `Project/ProjectSettings.h`, `Project/ProjectSerializer.h`, `Core/Ids/StrongId.h` | Son los esquemas y los ids que G-07 congela |
| **G-09** (creació de personatge dins de Nova partida) | `Engine/PantallaNuevaPartida.h`, `RPG/CharacterCreation.h` | Son la cadena que G-09 describe |
| **G-12** | `Render/HudFicha.h`, `Game/FichaVista.h`, `HUD/Iconos.h` | La ficha visible y sus iconos |
| **G-14** | `Render/BitmapFont.h`, `Game/HudTimer` (si se cablea), `Game/Codice.h`, `Game/DiarioMisiones.h` | Textos, temporizadores, códice y diario visibles |
| **G-17** | *(ya está en `Locale.h`)* | — |
| **C-13** (paritat document↔nivel) | `Level/LevelDocumentBridge.h`, `Map/TileLayerDocument.h`, `Map/EntityLayerDocument.h` | Son el puente y sus dos mitades |
| **C-12** | `Map/RegionLayerDocument.h` | `MapDocument.h` y `MapLayer.h` ya lo llevan |
| **F-03** | *(ya está en `MapDocument.h`)* | — |
| **Un paquete nuevo de combate** | `Game/BattleState.h`, `Render/ICombatant.h`, `Game/BattleMenuActions.h`, `Game/SessionTipos.h::ShopOffer` | No existe ningún «Fet quan» que nombre la máquina de combate. Es el hueco más grande: `BattleState` resuelve los turnos y `ICombatant` es el contrato de medio sistema, con 30 pruebas y sin criterio |
| **Un paquete nuevo de narrativa** | `RPG/Narrative/NarrativeBeat.h` (6 clases), `RPG/Narrative/AdventureScript.h` | `NarrativeBeat` es «la unidad de contenido, la carta del Llibre VI del GDD» y no hay criterio que la nombre |
| **Un paquete nuevo de controles** | `Engine/Input/InputMap.h`, `Engine/Input/InputSystem.h`, `Engine/InputState.h`, `Game/PlayerControls.h`, `Game/PlayerControlsData.h` | Cinco cabeceras y dos sistemas de entrada conviviendo sin criterio que decida cuál manda |

Quedan sin propuesta honesta las piezas de utilería (`Render/FormatoPPM.h`,
`CapturaGL.h`, `BlobShadow.h`, `OcclusionRules.h`, `Core/Check.h`, `ScopeGuard.h`,
`Engine/ApplicationLoop.h`, `ApplicationInternal.h`): son internas y asignarles un
paquete de jugabilidad sería ruido. Para esas la propuesta es otra y está en
`05-engine.md`: `ApplicationInternal.h` tiene dimensiones de dibujo hardcodeadas y
cero pruebas, que es lo que Oriol pidió revisar explícitamente.

---

## 6. Lo que falta

1. ~~**El juicio del editor**~~. Escrito el 29/09: `07-editor.md`.
2. **El diseño de «lo que hay + lo que esperamos que haya».** Escrito el 29/09:
   `08-diseno.md`. Lo desbloquearon las cuatro decisiones de Oriol de ese día:
   - **Combate:** sí hay iniciativa, **1d6 + DES** (más los modificadores que las
     pasivas pongan en `initiative_bonus()`). El GDD dice 1d20 y hay que cambiarlo
     aguas arriba.
   - **Escenas:** manda el plan, **una escena por pantalla**, con push/pop para las
     que se superponen al juego.
   - **Los cuatro subsistemas:** **enlazar lo que se pueda ya**. Audio en el juego (con
     los sonidos por encargar y el plan por cambiar, porque `FORA` lo excluía), las
     tablas de encuentro al combate con `LootResolver`, y el panel de Problemas con la
     validación de E-07 en `motor_editor`. `AdventureSerializer` no tiene hoy a qué
     enlazarse y se queda como pieza de G-07.
   - **Editor (EST-42):** **se conecta toda la capa de estado**: Problemas, Eventos,
     Base de datos, y Recursos y mapa.
3. **Arreglar `medir_alcance.py` antes de re-congelar la línea base** (§3.20). Ya son
   tres reglas: un paraguas está alcanzado si lo está cualquiera de las cabeceras que
   incluye; `template<> Clase<T>::metodo` cuenta como uso de la cabecera que lo
   declara; y las fuentes de `target_sources(...)` son del ejecutable. Sin eso,
   `D0007` fijaría una lista que ya se sabe falsa.
4. **Marcar las cabeceras con su paquete**, cuando Oriol acepte la propuesta de §5.
   `marcar_paquetes.py` ya no es el freno: desde el 29/09 quita todos los bloques
   (no solo el primero), reconoce el cierre mal escrito, no se lleva código por
   delante, se salta el trabajo sin commitear, no deja ningún fichero por encima de
   su techo y acepta `--solo`. Lo prueba `test_marcar_paquetes`.
