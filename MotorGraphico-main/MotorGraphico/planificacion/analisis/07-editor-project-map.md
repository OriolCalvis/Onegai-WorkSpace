## Mòdul Editor i format de projecte

Avui l'editor és una aplicació GL (`editor/src/level_editor.cpp`, 2708 línies) amb dues fases: una pantalla de projectes (`Editor::ProjectHub`) que llista, crea i empaqueta packs, i una fase d'edició que pinta tiles per capes, col·loca objectes del catàleg, marca l'inici del jugador, enllaça nivells, selecciona rectangles amb copiar/enganxar/esborrar, desfà/refà i edita les propietats d'una instància amb un inspector modal (`Editor::ObjectInspector`). Tota la lògica és GL-free i viu a `EditorState`/`ProjectHub`/`ObjectInspector`; la vista només tradueix tecles i pinta cadenes. El format de projecte real en disc és el manifest en castellà `assets/proyectos/<id>.json` + `index.json` que llegeix `Editor::ProjectIndex` (documentat a `FORMATO_PROYECTOS.md`), i el nivell es desa com a **parell TMX + JSON** generats per `EditorState::exportTmx()` i `exportLevelJson()` — cadenes construïdes a mà amb `ostringstream`, sense esquema versionat.

La migració DOC-03 està **connectada només per un costat**. Els mòduls `Map/` (DOC-02) i `Project/` (DOC-01) existeixen, compilen com a `motor_map_document` i `motor_project`, i tenen serialització canònica v1 amb `schemaVersion`, IDs forts i escriptura atòmica. Però *cap* consumidor viu els fa servir: `MapDocument` només el toquen `RuntimeTileMapBuilder`, `TmxMapImporter`, `ProjectSerializer` i els tests (`tests/Map/`, `tests/Project/`, `tests/Integration/MapRoundTripTests.cpp`). `EditorState` no coneix `MapDocument`, `ProjectSerializer` no s'invoca enlloc fora dels tests, i `level_editor.cpp` continua desant TMX de text que `TileMap` reparseja. El que sí que ja està connectat de `Project/` és `ProjectIndex`, que és codi antic *mogut* a la carpeta nova (l'antic `Editor/ProjectIndex.h` ha quedat com a shim d'una línia).

---

### `EditorTool` — *include/Editor/EditorState.h* (337 línies)
- **Què és**: enum class amb les set eines del pinzell: `PaintTile`, `EraseTile`, `PlaceObject`, `RemoveObject`, `SetPlayerStart`, `FillTiles`, `LinkLevel` (h:34-42).
- **Herència**: cap.
- **Estat**: —
- **API**: —
- **Flux**: el fixa `EditorState::setTool()` des de les tecles 1..7 (`level_editor.cpp:1718-1731`) i el consumeix `applyAt()` (`EditorState.cpp:226-254`).
- **Observacions**: l'enum és també l'ordre de les tecles numèriques; afegir-hi una eina al mig desplaça els atajos silenciosament. No hi ha eina de comptagotes, ni de línia/rectangle, ni de moure la selecció.

### `TmxTilesetSettings` — *include/Editor/EditorState.h* (337 línies)
- **Què és**: paràmetres del tileset per a `exportTmx()`: descriuen la capçalera `<tileset>` del TMX que s'escriurà (h:48-62).
- **Herència**: cap.
- **Estat**: `tileWidth/tileHeight` (cel·la del MAPA, iso 2:1), `tilesetName`, `imageSource`, `imageWidth/imageHeight`, `tilesetTileWidth/tilesetTileHeight` (cel·la de l'ATLAS), `tileCount`, `columns`, i `collisionGids` (GIDs 1-based amb `collision=true`).
- **API**: cap (struct de dades pur).
- **Flux**: l'omple `guardarNivel()` a `level_editor.cpp:2012-2058`, copiant el tileset del mapa carregat si n'hi ha; si no, els valors per defecte de `test_checker`, o el bloc `editor_construction_tiles` fixat a mà.
- **Observacions**: només admet **un** `<tileset>` amb `firstgid=1`. Un mapa amb dos atlas no es pot exportar sense pèrdua. Els paràmetres del tileset de construcció estan cablejats al codi de la UI (`level_editor.cpp:2051-2058`), no en cap asset.

### `EditorState` — *include/Editor/EditorState.h* (337 línies) / *src/Editor/EditorState.cpp* (889 línies)
- **Què és**: el document de nivell en edició i totes les seves operacions, sense GL: capes de tiles, spawns d'objectes, inici del jugador, paletes filtrades, selecció, portapapers, historial i els dos exportadors de text.
- **Herència**: cap; ningú no n'hereta. Té tres tipus niats privats (`Layer`, `Snapshot`, `Clipboard`, h:280-301) i un de públic (`Rect`, h:181-186).
- **Estat**:
  - `m_width/m_height` (clampats a ≥1 al constructor, cpp:60-62).
  - `m_layers` — vector de `Layer{name, tiles}`, graella plana `[y*w+x]`, gid 0 = buida. Sempre n'hi ha almenys una, anomenada `"suelo"` (cpp:66).
  - `m_activeLayer` — les eines de tile només toquen aquesta capa; objectes i `playerStart` són del nivell, no de cap capa.
  - `m_objects` — `std::vector<ObjectSpawn>`; invariant "una cel·la, un objecte".
  - `m_playerStart` — `GridCoord`.
  - `m_undo`/`m_redo` — piles de `Snapshot` (capes senceres + capa activa + objectes + playerStart), màxim 256 (cpp:699).
  - `m_selection` (`Rect` normalitzat i retallat) i `m_clipboard` (amplada/alçada, una graella per capa d'origen, objectes amb posició **relativa**).
  - Paletes duplicades: `m_tilePaletteAll`/`m_objectPaletteAll`/`m_levelPaletteAll` (completes) i `m_tilePalette`/`m_objectPalette`/`m_levelPalette` (filtrades, que són les que exposa l'API), més `m_tilePaletteLabels`/`m_objectPaletteLabels` i `m_filter`.
  - `m_tileSelection`/`m_objectSelection`/`m_levelSelection` — índexs dins de la vista filtrada.
- **API**:
  - *Dimensions i eina*: `width()`, `height()`, `setTool()`, `tool()`.
  - *Paletes*: `setTilePalette`, `setObjectPalette`, `setLevelPalette`, `setTilePaletteLabels`, `setObjectPaletteLabels`, `tilePalette/objectPalette/levelPalette` (retornen la vista **filtrada**), `nextPaletteEntry`/`prevPaletteEntry` (amb wraparound, actuen sobre la paleta que correspon a l'eina activa), `selectedTileGid`/`selectedObjectId`/`selectedLevelPath`.
  - *Filtre*: `setPaletteFilter`, `paletteFilter`, `tilePaletteTotal`/`objectPaletteTotal`/`levelPaletteTotal`. Privat: `rebuildFilteredPalettes()` (cpp:144-188), comparació ASCII en minúscules per subcadena contra `gid + etiqueta` o `id + etiqueta`; reinicia les tres seleccions a 0.
  - *Edició*: `applyAt(x,y)` (despatxa segons l'eina), `paintTile`, `eraseTile` (= `paintTile(...,0)`), `fillTiles` (flood-fill ortogonal, un sol pas d'historial), `placeObject`, `placeSpawn` (posa un `ObjectSpawn` ja construït, el que fa servir la càrrega d'un nivell existent), `removeObjectAt`, `setObjectTransition`, `setPlayerStart`, `playerStart`.
  - *Historial*: `undo`, `redo`, `canUndo`, `canRedo`, `clearHistory`. Privat: `recordUndo`, `snapshot`, `restore`.
  - *Capes*: `layerCount`, `activeLayer`, `setActiveLayer`, `layerName`, `setLayerName`, `addLayer` (queda a dalt i passa a activa), `removeLayer` (no deixa quedar-se sense capes).
  - *Selecció i portapapers*: `setSelection`, `clearSelection`, `hasSelection`, `selection`, `copySelection`, `hasClipboard`, `clipboardWidth/Height`, `pasteAt` (retalla el que surti del mapa), `eraseSelection`.
  - *Propietats d'instància*: `setObjectPatrol`, `setObjectTargetPosition`, `setObjectScale` (clampat a [0.25, 4.0]), `setObjectDisplayName`, `setObjectVariant`, `setObjectEffectOverride`, `setObjectTargetLevel`, `addObjectLoot`, `removeLootAt`, `setObjectLootTable`, `setObjectEvent`, `setObjectProperty`, `removeObjectProperty`. Totes són no-op si la cel·la no té objecte i **no graven historial si el valor ja hi era**.
  - *Consulta*: `tileAt(x,y)` (capa activa), `tileAt(layer,x,y)`, `objects()`, `objectAt()`. Privat: `inBounds`, `mutableObjectAt`.
  - *Exportació*: `exportTmx(const TmxTilesetSettings&)` i `exportLevelJson(levelName, mapPath)` — cadenes, sense tocar disc.
- **Flux**: el construeix `level_editor.cpp` amb la mida del mapa (nou o del TMX carregat), s'omple amb `paintTile` capa a capa i `placeSpawn` per objecte (cpp:1158-1191), es crida `clearHistory()` just després d'obrir, i produeix les dues cadenes que `guardarNivel()` bolca a `saveTmx`/`saveJson`. També és l'objecte que reben `EditorValidation` i `ObjectInspector`.
- **Observacions**:
  - **Historial per snapshot complet** (`Snapshot` a h:289-294): és el paquet marcat `REPLANTEAR` EDT-12 a `planificacion/ESTADO_REAL_vs_ROADMAP.md:174`. Amb 256 passos, un mapa de 200×200 amb 4 capes són centenars de MB. A més `recordUndo()` fa `m_undo.erase(m_undo.begin())` quan s'omple (cpp:700-702), que és una còpia O(n) de snapshots sencers.
  - `setLayerName()` (cpp:86-90) **no crida `recordUndo()`**, però el nom sí que forma part del `Snapshot`: un desfer posterior revertirà el canvi de nom sense que ningú l'hagi demanat.
  - `pasteAt()` itera `l < m_clipboard.layers.size() && l < m_layers.size()` (cpp:439): enganxar contingut de 3 capes en un document de 2 en descarta una en silenci.
  - La selecció **no** està al `Snapshot`: desfer no la restaura.
  - `setObjectTransition()` (cpp:351) i `setObjectTargetLevel()` (cpp:583) fan gairebé el mateix; la capçalera ho admet explícitament (h:222-225). El primer no accepta cadena buida, el segon sí (per desconnectar).
  - `fillTiles()` (cpp:271-295) no manté conjunt de visitats i empila els 4 veïns sempre; acaba, però el vector `pending` creix fins a ~4× les cel·les pintades.
  - `exportLevelJson()` escriu `scale` amb la precisió per defecte d'`ostream` (cpp:867) i **no emet `schemaVersion`**, a diferència del format canònic de `ProjectSerializer`.
  - La capa no té ni visibilitat, ni bloqueig, ni opacitat, ni `order` explícit — coses que `MapLayerMeta` sí que té. Són dos models de capa que conviuen.

### `EditorState::Rect` — *include/Editor/EditorState.h* (337 línies)
- **Què és**: rectangle inclusiu ja normalitzat que representa la selecció (h:181-186).
- **Herència**: cap.
- **Estat**: `x0, y0, x1, y1`; `x1 = -1, y1 = -1` per defecte, que és el "no vàlid".
- **API**: `valid()`, `width()`, `height()` — totes inline i const.
- **Flux**: el produeix `setSelection()` (normalitza i retalla al mapa, cpp:381-391), el consumeixen `copySelection`, `eraseSelection` i la vista per pintar el marc.
- **Observacions**: cap.

### `EditorValidationResult` — *include/Editor/EditorValidation.h* (187 línies)
- **Què és**: informe de validar un nivell abans de desar o de jugar-hi (h:14-22).
- **Herència**: cap.
- **Estat**: `errors` i `warnings` (vectors de missatges), `walkableTiles`, `reachableTiles`, `unreachableTiles`.
- **API**: `ok()` — cert si no hi ha errors (els avisos no bloquegen).
- **Flux**: el retornen `EditorValidation::check` i `checkRpg`; `level_editor.cpp:2185-2199` en pinta un resum a la barra d'estat.
- **Observacions**: `unreachableTiles` només es calcula dins la branca on l'inici és transitable (h:78); si l'inici és invàlid es queda a 0 i sembla que no hi ha zones aïllades.

### `EditorValidation` — *include/Editor/EditorValidation.h* (187 línies)
- **Què és**: classe només amb mètodes estàtics, **header-only** (no hi ha `src/Editor/EditorValidation.cpp`), que comprova un `EditorState` contra un `ObjectCatalog`.
- **Herència**: cap.
- **Estat**: cap (sense membres).
- **API**:
  - `static EditorValidationResult check(editor, catalog, collisionGids)` — compta tiles transitables (gid ≠ 0 i no de col·lisió), fa un BFS de 4 veïns des de `playerStart` per detectar zones inabastables, i marca com a error cada `objectId` que no és al catàleg i el mapa sense cap tile transitable.
  - `static EditorValidationResult checkRpg(editor, catalog, skills, collisionGids, availableSpriteIds = {})` — crida `check()` i hi afegeix la resolució de referències entre catàlegs: sprite absent de l'atles, enemics amb `maxHealth <= 0` o `maxMana` negatiu, skills inexistents, pickups amb `power/price` negatiu o efecte amb power 0, NPCs no interactuables, patrulles invertides i transicions sobre objectes no interactuables.
- **Flux**: `check()` la crida la tecla V/F6 de l'editor i `examples/demo_editor_state.cpp:177-190`; `checkRpg()` **només** la crida el demo (`demo_editor_state.cpp:219,236`).
- **Observacions**:
  - `isWalkable()` fa servir `editor.tileAt(x, y)`, que és la **capa activa** (h:42-43). En un mapa multicapa amb el terra a la capa 0 i la decoració activa, la validació diu "el mapa no tiene tiles transitables".
  - L'editor la crida amb `collisionGids` cablejat a `{2}` (`level_editor.cpp:2186`), tot i que a la mateixa funció de desar sí que llegeix els GIDs de col·lisió reals del tileset carregat (`level_editor.cpp:2046-2049`). Validar i desar no fan servir les mateixes col·lisions.
  - `checkRpg` recorre **tot** el catàleg (milers d'entrades) encara que el nivell no en faci servir cap.

### `Editor::ObjectInspector` — *include/Editor/ObjectInspector.h* (143 línies) / *src/Editor/ObjectInspector.cpp* (506 línies)
- **Què és**: el panell modal de propietats d'un objecte ja col·locat (nom visible, variant, escala, efecte, sortida, posició d'arribada, botí, taula de botí, esdeveniment i propietats lliures), sense GL.
- **Herència**: cap. Tipus niats privats: `enum class Campo` (12 valors), `enum class Modo{Navegar, Texto, ElegirSalida, ElegirLoot}` i `struct Fila{campo, indice}`.
- **Estat**: referències no-propietàries a `EditorState&` i `const ObjectCatalog&`, la llista `m_destinos`; `m_open`, `m_cell`, `m_sel`, `m_modo`; l'edició de text en curs (`m_campoTexto`, `m_indiceTexto`, `m_buffer`); els selectors (`m_opciones`, `m_textoFiltro`, `m_marca`, `m_baseLoot`) i la sortida per a la vista (`m_filas`, `m_lineas`, `m_aviso`). `m_filtro` (h:133) està declarat i no s'utilitza mai.
- **API**:
  - *Cicle de vida*: `open(x,y)` (no obre si la cel·la no té objecte), `close()`, `isOpen()`, `cell()`, `spawn()` (nullptr si l'objecte ha desaparegut).
  - *Entrada*: `bool key(char k)` — l'única porta; W/S navega, ENTER edita o confirma, ESC cancel·la o tanca, X treu botí/propietat, lletres escriuen al buffer, BACKSPACE esborra. Retorna sempre `true` mentre és obert perquè el panell és modal.
  - *Vista*: `titulo()`, `lines()`, `selected()`, `editando()`, `editandoTexto()`, `buffer()`, `aviso()`.
  - Privat: `rebuild()` (reconstrueix files i línies segons l'spawn actual), `abrirTexto()`, `confirmar()` (**declarat a h:111 i mai definit ni cridat**), `commitTexto()`, `nombreLoot()`.
- **Flux**: el construeix `level_editor.cpp:1154` amb el mateix vector de destins que la paleta de `LinkLevel` (comentat a :1144-1149). S'obre amb clic dret sobre un objecte i escriu directament a `EditorState` mitjançant els setters `setObject*`. Es tanca sol a la següent tecla si l'spawn ha desaparegut (cpp:228-231).
- **Observacions**:
  - `confirmar()` és codi mort declarat.
  - El selector de botí carrega `m_catalog.ids()` sencer a `m_baseLoot` cada cop que s'obre (cpp:487) i refiltra amb comparació de minúscules feta a mà dins un lambda (cpp:268-276), duplicant `aMinusculas()` d'`EditorState.cpp:48`.
  - `nombreEscenario()` (cpp:17-25) està duplicat literalment a `level_editor.cpp:206`.
  - L'esborrat d'una propietat (cpp:390-401) itera el `std::map` per índex posicional; és correcte perquè `rebuild()` fa servir el mateix ordre, però l'índex és fràgil davant de qualsevol canvi d'ordenació.
  - No pot editar la **patrulla** (això només ho fa la tecla T sobre la selecció, `level_editor.cpp:1887`) ni moure l'objecte de cel·la.

### `Editor::ProjectHub` — *include/Editor/ProjectHub.h* (121 línies) / *src/Editor/ProjectHub.cpp* (346 línies)
- **Què és**: la pantalla d'arrencada de l'editor: llista de projectes, detall del marcat, creació d'un de nou i llançament de l'empaquetat. GL-free.
- **Herència**: cap. Enums niats: `Mode{List, NewProject, Message}` i `Action{None, Open, Quit}`.
- **Estat**: `m_index` (una còpia de `ProjectIndex`), `m_root` (arrel d'assets), `m_lines` (una línia formatada per projecte), `m_selected`, `m_mode`, `m_draft` (id que s'està teclejant), `m_message` + `m_lastOk`, `m_openId`.
- **API**:
  - *Construcció*: `static Result<ProjectHub> load(assetsRoot = "assets")` — escaneja i construeix les línies.
  - *Llista*: `lines()`, `selected()`, `current()` (nullptr si no hi ha res), `size()`, `moveUp()`/`moveDown()` (amb wraparound), `detail()` (les línies del panell dret: època, autor, id, prefix, recomptes, a què resol l'entrada i fins a 6 problemes).
  - *Entrada*: `Action key(char k)` — w/s mouen, `\n` obre, `n` entra al mode de projecte nou, `b` compila, 27 surt; dins de `NewProject` les lletres i `_` escriuen, `\b` esborra i `\n` confirma; dins de `Message` qualsevol tecla torna.
  - *Accions*: `build()` (llança `python3 tools/build_proyecto.py <id>` per `popen` i es queda la sortida), `refresh()` (torna a escanejar), `draftId()`, `message()`, `lastOk()`, `openId()`.
  - Privat: `rebuildLines()`, `setMessage()`.
- **Flux**: la instancia `level_editor.cpp:758-764` quan no s'ha passat `--proyecto` ni `--load`; el bucle de la fase 1 li passa tecles i pinta `lines()`, `detail()` i `message()`. Quan `key()` retorna `Open`, `opts.projectId = hub.openId()` i s'entra a la fase d'edició.
- **Observacions**:
  - `detail()` crida `m_index.check(p->id)` (cpp:173) i es crida **cada frame** des de `level_editor.cpp:869`. `check()` fa un `fopen` per cada fitxer declarat: amb `este_norte` (55 nivells + 55 mapes) són més de cent obertures de fitxer per frame.
  - `rebuildLines()` també crida `check()` per **cada** projecte (cpp:98), o sigui uns quants centenars de `fopen` per cada `refresh()`.
  - `refresh()` (cpp:207-213) s'empassa els errors d'`scan()` en silenci: si el manifest s'ha trencat, la llista es queda com estava sense dir res.
  - `build()` compon la línia de shell amb `script` **sense cometes** (cpp:232); l'id sí que està validat per `idSeguro()`, però una arrel d'assets amb espais trencaria la crida.
  - No hi ha cap camí a `ProjectIndex::remove()` des de la UI: es poden crear projectes però no esborrar-los.

### `Editor/ProjectIndex.h` (shim) — *include/Editor/ProjectIndex.h* (6 línies)
- **Què és**: un fitxer que només fa `#include "Project/ProjectIndex.h"`, amb un comentari que diu "Compatibilidad temporal durante INF-03" i que els consumidors nous han d'incloure la ruta de `Project/`.
- **Herència**: cap.
- **Estat**: cap.
- **API**: cap.
- **Flux**: encara l'inclouen `editor/src/level_editor.cpp:93` i `examples/demo_proyectos.cpp:16` — els dos únics consumidors, i cap dels dos s'ha migrat.
- **Observacions**: **no hi ha duplicitat de codi**. `Project/ProjectIndex.h` és l'única definició de `Editor::ProjectIndex` (el `namespace Editor` s'ha conservat tot i el canvi de carpeta), i `Editor/ProjectIndex.h` només hi redirigeix. La substitució ja és total; el que queda és esborrar el shim i canviar dos `#include`.

---

### `DocumentKind` / `DocumentRef` — *include/Project/DocumentRef.h* (29 línies)
- **Què és**: `enum class DocumentKind{Map, Level, Adventure, Catalog}` i la referència persistent a un document: la identitat és el `DocumentId`, no la ruta, perquè moure o reanomenar un fitxer no trenqui les relacions.
- **Herència**: cap.
- **Estat**: `id` (`ids::DocumentId`, id fort), `kind`, `logicalPath`.
- **API**: `Result<bool> validate() const` — inline a la capçalera; exigeix id no buit i una ruta lògica relativa sense `\`, sense començar per `/`, sense `../` i diferent de `..`.
- **Flux**: només l'usa `tests/Core/StrongIdTests.cpp:6`. **Cap** estructura del projecte en conté.
- **Observacions**: la promesa central del tipus ("les relacions guarden DocumentId, no la ruta") **encara no s'ha complert enlloc**: `Editor::Project` guarda vectors de `std::string` amb noms de fitxer (`Project.h:25-28`) i `ObjectSpawn::targetLevel` guarda una ruta completa. `DocumentRef` és, ara mateix, un tipus orfe.

### `ProjectSettings` — *include/Project/ProjectSettings.h* (25 línies) / *src/Project/Project.cpp* (128 línies)
- **Què és**: els ajustaments globals del document `Project`: versió d'esquema i directoris lògics relatius a l'arrel del pack.
- **Herència**: cap.
- **Estat**: `static constexpr int kCurrentSchemaVersion = 1`; `schemaVersion`, `mapsDirectory = "maps"`, `levelsDirectory = "levels"`, `adventuresDirectory = "adventures"`, `catalogsDirectory = "objects"`, `initialMap`.
- **API**: `Result<bool> validate() const` (implementat a `Project.cpp:70-93`): rebutja una `schemaVersion` diferent d'1, exigeix que els quatre directoris siguin rutes relatives vàlides i **distintes entre si**, i que `initialMap` sigui un nom de document.
- **Flux**: viu dins de `Project`. El serialitza `ProjectSerializer` com a objecte `settings` niat.
- **Observacions**: `ProjectIndex::scan()` **no llegeix cap dels quatre directoris** del manifest — assumeix `levels`, `maps`, `adventures`, `objects` cablejats (`ProjectIndex.cpp:212-213`). O sigui que els directoris configurables existeixen al model i no tenen cap efecte al camí que sí que funciona.

### `Editor::Project` — *include/Project/Project.h* (52 línies) / *src/Project/Project.cpp* (128 línies)
- **Què és**: el document arrel d'un pack de contingut. Estructura de dades pura amb validació GL-free.
- **Herència**: cap. Enum niat `EntryKind{None, Adventure, Level, Missing}`.
- **Estat**: `id`, `name`, `description`, `epoch`, `author`, `prefix`; `settings`; `entry` (punt d'entrada *legacy*: ruta lògica o nom d'aventura/nivell); els quatre vectors de noms `levels`, `maps`, `adventures`, `catalogs`; i el resultat de resoldre l'entrada, `entryKind` + `entryPath`.
- **API**: `assetCount()` (suma dels quatre vectors), `playable()` (té entrada i almenys un nivell), `Result<bool> validate() const` (`Project.cpp:95-126`): id `[a-z0-9_]`, nom no buit, prefix acabat en `_` si n'hi ha, `settings.validate()`, `entry` com a ruta relativa, cada grup sense noms invàlids ni duplicats, i `settings.initialMap` ha de pertànyer a `maps`.
- **Flux**: el construeix `ProjectIndex::scan()` llegint el manifest en castellà, i `ProjectIndex::create()` en fa un d'esquelet. `ProjectHub` en pinta les dades i `level_editor.cpp` en treu la llista de nivells per navegar i les rutes on desar.
- **Observacions**: el camp `entry` està marcat com a "legacy" al comentari (h:23) però és l'únic mecanisme d'arrencada real; `settings.initialMap` no es fa servir enlloc. Els helpers de validació `isProjectId`, `isRelativePath`, `isDocumentName` (`Project.cpp:12-51`) dupliquen les regles de `ids::isValidValue` (`StrongId.h:13-24`) i de `ProjectIndex::create` (`ProjectIndex.cpp:244-249`).

### `Editor::ProjectCheck` — *include/Project/Project.h* (52 línies)
- **Què és**: el resultat de comprovar en disc que el que declara un manifest hi és de debò.
- **Herència**: cap.
- **Estat**: `projectId`, `problems` (vector de missatges), `levelsChecked`, `missingFiles`.
- **API**: `ok()` — cert si `problems` és buit.
- **Flux**: el produeix `ProjectIndex::check()`; el consumeixen `ProjectHub::rebuildLines()`/`detail()` i la llista de text de `level_editor.cpp:637-650`.
- **Observacions**: `problems` barreja errors reals (fitxer declarat que no existeix) amb un avís de disseny ("sin 'entrada'", `ProjectIndex.cpp:229`), i `ProjectHub` pinta els dos igual, en taronja.

### `Editor::ProjectIndex` — *include/Project/ProjectIndex.h* (86 línies) / *src/Project/ProjectIndex.cpp* (349 línies)
- **Què és**: el catàleg dels projectes vius. Llegeix `assets/proyectos/index.json` i després cada `<id>.json`, i sap crear i esborrar manifestos.
- **Herència**: cap.
- **Estat**: `m_projects`, `m_orphans`, `m_root`.
- **API**:
  - `static Result<ProjectIndex> scan(assetsRoot = "assets")` — un id sense manifest, amb JSON invàlid o amb un manifest que no valida no tomba l'escaneig: s'apunta com a orfe i la resta segueix (cpp:146-185).
  - `projects()`, `find(id)`, `size()`, `orphans()`.
  - `ProjectCheck check(id) const` — comprova que cada fitxer declarat existeixi als quatre directoris cablejats i que l'entrada resolgui.
  - `static Result<Project> create(assetsRoot, id, name, prefix, author, epoch)` — valida l'id `[a-z0-9_]` i el prefix, refusa si l'id ja hi és o si hi ha un manifest solt no llistat, escriu el manifest amb `fprintf` i reescriu `index.json`.
  - `static Result<bool> remove(assetsRoot, id)` — treu del índex primer i esborra el manifest després; accepta esborrar un manifest orfe (cpp:319-329). **No** esborra nivells ni mapes.
  - Anònims al `.cpp`: `leeFichero`, `existe`, `lista`, `esc`, `resuelveEntrada` (admet a propòsit tant "ruta completa" com "nom solt" per a `entrada`, cpp:70-105), `escribeIndice`.
- **Flux**: el punt d'entrada de tot el sistema de projectes. El criden `ProjectHub::load/refresh`, `pantallaDeArranque()` i tres llocs més de `level_editor.cpp` (resolució del projecte a :690, canvi d'escenari a :2098, catàlegs per al playtest a :2145).
- **Observacions**:
  - **`orphans()` no compleix el que promet la capçalera.** El comentari de `ProjectIndex.h:54-55` diu "Ficheros de assets que no reclama ningun manifiesto", però `scan()` només hi posa problemes de *manifest* (cpp:152, 157, 179): mai no recorre `assets/levels/` ni `assets/maps/` per detectar contingut no reclamat. La detecció d'orfes reals no existeix.
  - `create()` escriu el manifest amb un `fprintf` de format fix (cpp:287-295) que emet `schemaVersion` a l'arrel però **cap bloc `settings`** — per tant `ProjectSerializer::deserializeProject()` el rebutjaria (`ProjectSerializer.cpp:229-231`).
  - `scan()` llegeix `schemaVersion` i `initialMap` de l'**arrel** del manifest (cpp:170-171), mentre `ProjectSerializer` els escriu **dins de `settings`**. Són dos formats diferents del mateix document.
  - No hi ha cap operació de *desar* un `Project` modificat: quan l'editor desa el primer nivell d'un projecte nou (a `assets/levels/<prefix>nivel_1.json`, `level_editor.cpp:705-709`), **ningú no l'afegeix a `"niveles"`** del manifest. El fitxer queda en disc i el projecte segueix declarant 0 nivells.
  - `scan()` es torna a executar sencer cada cop (cap cau), i cada `ProjectHub::load()` en copia el resultat per valor (`ProjectHub.cpp:89`).

### `ProjectSerializer` — *include/Project/ProjectSerializer.h* (25 línies) / *src/Project/ProjectSerializer.cpp* (551 línies)
- **Què és**: la persistència **canònica v1** dels documents d'autoria (projecte i mapa), amb ordre de claus fix perquè dos desats del mateix document donin els mateixos bytes.
- **Herència**: cap; només mètodes estàtics, sense estat.
- **Estat**: cap. Constants de fitxer: `kProjectSchemaVersion = 1`, `kMapSchemaVersion = 1` (cpp:20-21).
- **API**:
  - `static Result<std::string> serializeProject(const Editor::Project&)` — valida primer i emet `schemaVersion, id, name, description, epoch, author, prefix, settings{...}, entry, levels, maps, adventures, catalogs` amb `JsonWriter`.
  - `static Result<Editor::Project> deserializeProject(const std::string&)` — comprova `schemaVersion`, exigeix `id`, `name` i l'objecte `settings` amb els quatre directoris, i torna a validar el resultat.
  - `static Result<std::string> serializeMap(const MapDocument&)` / `static Result<MapDocument> deserializeMap(const std::string&)` — capes `tile` (array `tiles`), `entity` (array `entities` amb `id`/`objectId`/`position`) i `region` (array `regions` amb `bounds`), més `collisionGids`.
  - `static Result<bool> saveProjectAtomic(path, project)` / `saveMapAtomic(path, map)` — escriptura a `path + ".tmp"` i `rename`/`MoveFileExA` (cpp:130-160).
  - `static Result<Editor::Project> loadProjectFile(path)` / `Result<MapDocument> loadMapFile(path)`.
  - Helpers anònims: `requiredString/Int/Bool`, `checkSchema`, `writeStrings`/`readStrings`, `readId<Id>` (parseja IDs forts), `readGridCoord`/`writeGridCoord`, `atomicWrite`, `readFile`.
- **Flux**: **cap consumidor de producció**. Només el criden `tests/Project/SerializationTests.cpp` i `tests/Integration/MapRoundTripTests.cpp`.
- **Observacions**:
  - És **el segon format de projecte**, incompatible amb el que hi ha en disc: claus en anglès (`name`, `description`, `levels`…) contra les del manifest real (`nombre`, `descripcion`, `niveles`…), i `settings` niat contra camps a l'arrel. `deserializeProject()` no pot llegir cap dels cinc manifestos d'`assets/proyectos/`, i `serializeProject()` produiria un fitxer que `ProjectIndex::scan()` llegiria com un projecte sense nivells ni entrada. `FORMATO_PROYECTOS.md` documenta només el format antic.
  - No hi ha serialitzador de **nivell**: `ObjectSpawn`, `playerStart` i les transicions segueixen anant pel `exportLevelJson()` d'`EditorState`, que no comparteix ni `JsonWriter` ni `schemaVersion`.
  - `atomicWrite` no fa `fsync` ni del fitxer ni del directori; l'atomicitat és la del `rename`, no la de la durabilitat.

---

### `TileLayerDocument` — *include/Map/TileLayerDocument.h* (26 línies) / *src/Map/MapDocument.cpp* (240 línies)
- **Què és**: les dades d'autoria d'una capa de tiles: graella plana en ordre fila-major, GID 0 = cel·la buida (com al TMX).
- **Herència**: cap. És una de les tres alternatives del `std::variant MapLayerData`.
- **Estat**: `m_width`, `m_height`, `m_gids`.
- **API**: constructor per defecte i `(width, height, gids)`; `width()`, `height()`, `gids()`; `Result<bool> validate() const` (cpp:12-26): dimensions positives, `gids.size() == w*h`, cap GID negatiu.
- **Flux**: el construeixen `ProjectSerializer::deserializeMap` i `TmxMapImporter`; el consumeix `RuntimeTileMapBuilder`.
- **Observacions**: és **immutable un cop construïda** — no hi ha `setGid()` ni accés no-const a `m_gids`. Un editor no hi pot pintar; cal reconstruir la capa sencera i el document sencer.

### `MapEntityDocument` — *include/Map/EntityLayerDocument.h* (30 línies)
- **Què és**: la instància d'una entitat dins d'un mapa.
- **Herència**: cap.
- **Estat**: `id` (`ids::EntityId`), `objectId` (`ids::AssetId`), `position` (`GridCoord`).
- **API**: cap (struct pur).
- **Flux**: viu dins d'`EntityLayerDocument`.
- **Observacions**: **aquest és el forat més gran de la migració.** `ObjectSpawn` (l'equivalent del camí vell, `LevelDefinition.h:52`) porta a més `patrolMin/patrolMax`, `targetLevel`, `targetPosition`+`hasTargetPosition`, `loot`, `lootTable`, `eventId`, `displayName`, `variant`, `scale`, `effectOverride` i un mapa `properties`. Convertir un nivell actual a `MapDocument` avui en perdria absolutament tot excepte la posició i l'id.

### `EntityLayerDocument` — *include/Map/EntityLayerDocument.h* (30 línies) / *src/Map/MapDocument.cpp* (240 línies)
- **Què és**: la capa que conté les instàncies d'entitats.
- **Herència**: cap; alternativa del variant.
- **Estat**: `m_entities`.
- **API**: constructor per defecte i `explicit (vector)`; `entities()`; `Result<bool> validate() const` (cpp:31-46): id no buit, `objectId` no buit, ids sense duplicats (via `std::set<ids::EntityId>`).
- **Flux**: com l'anterior.
- **Observacions**: immutable un cop construïda; no hi ha `add`/`remove`. No valida que dues entitats no comparteixin cel·la, invariant que `EditorState` sí que manté ("una cel·la, un objecte").

### `MapRegionDocument` — *include/Map/RegionLayerDocument.h* (30 línies)
- **Què és**: la fitxa d'una regió rectangular d'autoria, pensada com a base dels futurs sistemes d'esdeveniments i zones.
- **Herència**: cap.
- **Estat**: `id` (`ids::RegionId`), `name`, `bounds` (`GridBounds`, extrems inclusius).
- **API**: cap.
- **Flux**: dins de `RegionLayerDocument`.
- **Observacions**: el comentari de la capçalera preveu afegir-hi propietats, però ara mateix una regió no pot dur cap dada associada (ni tipus, ni esdeveniment, ni clau/valor). No hi ha cap eina d'editor que en creï.

### `RegionLayerDocument` — *include/Map/RegionLayerDocument.h* (30 línies) / *src/Map/MapDocument.cpp* (240 línies)
- **Què és**: la capa de regions.
- **Herència**: cap; alternativa del variant.
- **Estat**: `m_regions`.
- **API**: constructor per defecte i `explicit (vector)`; `regions()`; `Result<bool> validate() const` (cpp:51-69): id no buit, nom no buit, `bounds` no buits, ids sense duplicats.
- **Flux**: com les altres.
- **Observacions**: immutable; no comprova solapaments entre regions.

### `MapLayerType` i `MapLayerMeta` — *include/Map/MapLayer.h* (49 línies)
- **Què és**: `enum class MapLayerType{Tile, Entity, Region}` i els metadades comuns a qualsevol capa.
- **Herència**: cap.
- **Estat de `MapLayerMeta`**: `id` (`ids::LayerId`), `name`, `visible = true`, `locked = false`, `opacity = 1.0f`, `order = 0`. El comentari (h:16-17) subratlla que `order` és **explícit** i no la posició accidental dins d'un vector, per tenir un ordre determinista en desar, carregar i sincronitzar.
- **API**: cap.
- **Flux**: es passa per valor als tres factories de `MapLayer`.
- **Observacions**: `visible`, `locked` i `opacity` es desen i es validen però **cap consumidor els llegeix**: `RuntimeTileMapBuilder` compila les capes de tiles i el renderer no en sap res. `EditorState::Layer` no té cap d'aquests camps.

### `MapLayer` — *include/Map/MapLayer.h* (49 línies) / *src/Map/MapDocument.cpp* (240 línies)
- **Què és**: una capa de mapa: metadades comunes + un `std::variant` amb el contingut. **El tipus d'una capa és immutable**: el variant només s'accepta al constructor i s'exposa per referència const; convertir Tile → Entity exigeix crear una capa nova i migrar-ne el contingut explícitament (h:29-31).
- **Herència**: **cap herència**. Aquesta és la clau de la jerarquia de capes del mòdul `Map/`: *no hi ha jerarquia*. On un disseny clàssic tindria una `MapLayer` base virtual amb `TileLayer`/`EntityLayer`/`RegionLayer` derivades, aquí hi ha **una sola classe concreta i tancada** que conté un `std::variant<TileLayerDocument, EntityLayerDocument, RegionLayerDocument>` (`MapLayer.h:27`). Els tres "tipus de capa" són tipus **germans i independents**, sense base comuna, sense mètodes virtuals i sense destructor virtual; l'únic que comparteixen és la signatura `Result<bool> validate() const`, que `MapLayer::validate()` despatxa per `std::visit` sobre el variant (`MapDocument.cpp:112`) — polimorfisme estàtic, no dinàmic. El motiu documentat és tancar el conjunt: afegir un quart tipus de capa exigeix tocar el variant, els tres factories i tots els `visit`, i el compilador ho detecta; amb herència es podria afegir una derivada que ningú no sabés compilar. La decisió està registrada a `architecture/ADR-002-map-layer-types.md`.
- **Estat**: `m_meta` (`MapLayerMeta`), `m_data` (`MapLayerData`).
- **API**:
  - *Construcció*: constructor **privat** `(meta, data)`; només s'hi arriba pels tres factories `static MapLayer tiles(meta, TileLayerDocument)`, `static MapLayer entities(...)`, `static MapLayer regions(...)`.
  - *Consulta*: `meta()`, `type()` (dedueix el tipus amb `holds_alternative`, cpp:86-94), `data()` (const).
  - *Validació*: `Result<bool> validate() const` (cpp:96-113): id no buit, nom no buit, `order >= 0`, `opacity` finita i dins [0,1], i després delega al contingut.
- **Flux**: el construeixen `ProjectSerializer::deserializeMap` i `TmxMapImporter`; s'insereixen amb `MapDocument::addLayer` i es recorren amb `std::visit` a `MapDocument::validateLayerForMap` i a `serializeMap`.
- **Observacions**: com que `data()` només és const i no hi ha cap setter de `meta`, ni tan sols es pot amagar o desbloquejar una capa d'un document ja construït. Els metadades editables (`visible`, `locked`, `opacity`) són de fet de només lectura.

### `MapDocument` — *include/Map/MapDocument.h* (46 línies) / *src/Map/MapDocument.cpp* (240 línies)
- **Què és**: la font de veritat editable d'un mapa. No coneix `TileMap`, ni textures, ni OpenGL, ni widgets; el comentari de la capçalera diu que els consumidors de runtime el compilaran "en DOC-03".
- **Herència**: cap; ningú no n'hereta.
- **Estat**: `m_id` (`ids::MapId`), `m_name`, `m_width`, `m_height`, `m_tileWidth`, `m_tileHeight`, `m_collisionGids`, `m_layers`.
- **API**:
  - *Construcció*: per defecte i `(id, name, width, height, tileWidth, tileHeight)`.
  - *Consulta*: `id()`, `name()`, `width()`, `height()`, `tileWidth()`, `tileHeight()`, `collisionGids()`, `layers()`.
  - *Mutació*: només dues. `Result<bool> addLayer(MapLayer)` (cpp:160-186) — rebutja si els metadades del mapa són invàlids, valida la capa contra el mapa, rebutja id i `order` duplicats, i insereix per `order` amb `upper_bound` sense alterar la posició relativa de les existents. `Result<bool> setCollisionGids(std::vector<int>)` (cpp:188-200) — exigeix GIDs positius i sense duplicats, i els ordena.
  - *Validació*: `Result<bool> validate() const` (cpp:202-239) — id/nom no buits, dimensions positives, GIDs de col·lisió positius/únics/ordenats, i per cada capa: validació contra el mapa, ids únics, `order` únic i ordre no decreixent. Privat: `validateLayerForMap()` (cpp:124-158), que amb `std::visit` comprova que una capa de tiles tingui les dimensions del mapa, que les entitats estiguin dins de límits i que les regions també.
- **Flux**: el produeixen `TmxMapImporter::importFile()` i `ProjectSerializer::deserializeMap()`; el consumeixen `ProjectSerializer::serializeMap()` i `RuntimeTileMapBuilder::build()`. La cadena sencera només s'exercita a `tests/Integration/MapRoundTripTests.cpp` (TMX → MapDocument → JSON → MapDocument → RuntimeTileMap).
- **Observacions**:
  - **És un document d'escriptura única.** No hi ha `removeLayer`, ni `setLayerMeta`, ni cap accés no-const a les dades d'una capa. Es pot construir i validar, però no *editar*. Mentre no hi hagi API de mutació, `EditorState` no s'hi pot recolzar encara que es volgués.
  - `addLayer` rebutja `order` duplicat, cosa que fa que reordenar capes (una operació d'editor bàsica) no tingui cap camí: caldria reconstruir el document sencer.
  - No té camp `schemaVersion`; la versió del format viu només a `ProjectSerializer.cpp:21`, separada de `ProjectSettings::kCurrentSchemaVersion`.
  - No té tileset: `MapDocument` guarda `collisionGids` però no la imatge, ni les columnes, ni el `firstgid` — tot això només existeix a `TmxTilesetSettings` del camí vell. Un `MapDocument` desat i tornat a carregar no sap contra quin atles es va pintar.

---

### `level_editor.cpp` — arquitectura (2708 línies)

Un sol fitxer amb `main()` i un `namespace` anònim d'ajudants. Estructura:

1. **Capçalera de 88 línies** que documenta tots els controls i les decisions d'abast (per què HUD propi i no ImGui, per què les lletres manen sobre les tecles F).
2. **Ajudants anònims** (:140-612): `makeWhiteTexture()`, `writeFramebufferPPM()` (mode fum per a CI), `toolName()`, `nombreEscenario()`, `struct TilePaletteInfo` + `semanticasTile()` + `cargarSemanticaTiles()` (etiquetes editorials dels tiles llegides del `.md` del tileset), `ObjectFilter`/`filtroAfinidadObjeto()` i `nombreDePaleta()` (arbre de famílies d'objectes), `iconoEditor()`/`iconoBibliotecaEnemigos()`/`struct RaceNpcPreview`/`previewNpcRacial()`/`uvPreviewPng()` (icones de paleta), `class KeyEdge` (detecció de flanc amb `prime()` per no arrossegar tecles entre fases), i `struct EditorOptions` + `parseSize()`/`parseArgs()` (opcions `--proyecto`, `--nivel`, `--nuevo`, `--prefijo`, `--proyectos`, `--size`, `--load`, `--construccion`, i el frame count solt com a mode fum).
3. **`pantallaDeArranque()`** (:614-659): mode text sense GL, per a scripts i CI — llista projectes o en crea un i surt.
4. **`main()`** (:661-…): resol el projecte i les rutes de desat (`saveTmx`/`saveJson`/`saveName`), i entra en un bucle de dues fases:
   - **Fase 1, pantalla de projectes** (:743-900 aprox.): construeix `ProjectHub`, li passa tecles i pinta `lines()`/`detail()`/`message()` amb `HudPanel`/`HudText` creats de nou cada frame.
   - **Fase 2, edició**: carrega tileset, TMX i JSON, omple `EditorState` capa a capa i spawn a spawn, munta els panells fixos (capçalera, eines, paleta, inspector de cel·la, guia, panell de propietats, barra d'estat) i entra al bucle de frames.

Les **eines que la UI ofereix realment**: 1-7 canvien d'`EditorTool`; Q/E naveguen la paleta (amb arbre de famílies en mode objecte, TAB/ENTER/ESC/`[`/`]`); B obre el camp de cerca que crida `setPaletteFilter`; C i SHIFT+C canvien de capa i CTRL+N en crea una; R marca les dues cantonades de la selecció; CTRL+C/V/X copien, enganxen i esborren; T dona al objecte sota el cursor la patrulla del rectangle marcat; H cicla l'escala; CTRL+Z/Y desfan i refan; WASD/+/-/HOME mouen la càmera; G desa, V valida, P llança `./juego` com a procés a part i `,`/`.` canvien d'escenari desant abans; clic dret obre `ObjectInspector`; ESC/M tornen al hub.

---

### Buits i riscos del mòdul

- **`MapDocument` no té API de mutació** (`include/Map/MapDocument.h:31-32`: només `addLayer` i `setCollisionGids`; cap `removeLayer`, cap accés no-const a `MapLayer::data()` a `MapLayer.h:40`). Mentre no la tingui, l'editor no s'hi pot migrar encara que es vulgui: DOC-03 està bloquejat pel propi model, no només per la connexió.
- **`MapEntityDocument` (`include/Map/EntityLayerDocument.h:13-17`) només guarda `id`/`objectId`/`position`**, mentre `ObjectSpawn` (`include/Level/LevelDefinition.h:52-95+`) guarda patrulla, `targetLevel`, `targetPosition`, `loot`, `lootTable`, `eventId`, `displayName`, `variant`, `scale`, `effectOverride` i `properties`. Migrar un nivell existent al camí nou avui perdria gairebé tot el treball d'autoria.
- **Dos formats de projecte incompatibles i tots dos "canònics".** `ProjectIndex::scan()` llegeix claus en castellà a l'arrel (`src/Project/ProjectIndex.cpp:162-176`); `ProjectSerializer::serializeProject()` emet claus en anglès amb `settings` niat (`src/Project/ProjectSerializer.cpp:186-205`). `deserializeProject()` rebutjaria qualsevol dels cinc manifestos reals d'`assets/proyectos/` per manca del bloc `settings` (`ProjectSerializer.cpp:229-231`), i `ProjectIndex::create()` escriu manifestos que el serialitzador no pot llegir (`ProjectIndex.cpp:287-295`).
- **Cap consumidor de producció fa servir `Project/ProjectSerializer` ni `Map/`.** Els únics `#include` fora de capçaleres i tests són `tests/Project/SerializationTests.cpp:9`, `tests/Integration/MapRoundTripTests.cpp:9`, `tests/Map/MapDocumentTests.cpp` i `tests/Core/StrongIdTests.cpp:6`. `RuntimeTileMapBuilder` i `TmxMapImporter` en depenen, però ningú no els crida des de `editor/` ni des del joc.
- **`ProjectIndex::orphans()` no detecta orfes.** La capçalera ho promet (`include/Project/ProjectIndex.h:54-55`) però `scan()` només hi afegeix problemes de manifest (`src/Project/ProjectIndex.cpp:152, 157, 179`); mai no recorre `assets/levels/` ni `assets/maps/`. Un nivell que no reclama cap projecte segueix perdent-se en silenci — exactament el que el disseny deia evitar.
- **Desar un nivell no actualitza el manifest.** `guardarNivel()` escriu `saveTmx`/`saveJson` (`editor/src/level_editor.cpp:2059-2061`), inclòs el `<prefix>nivel_1` d'un projecte acabat de crear (:705-709), però no hi ha cap ruta d'escriptura de `Project` a `ProjectIndex`: el fitxer existeix i el projecte segueix declarant 0 nivells.
- **Sortir de l'editor no avisa de canvis sense desar.** ESC o M fan `volverAlHub = true; break;` directament (`editor/src/level_editor.cpp:1713-1716`), sense cap indicador de "brut" ni cap desat automàtic. Tot el que s'hagi pintat des de l'últim G es perd sense un sol missatge.
- **La validació de l'editor usa GIDs de col·lisió cablejats.** `EditorValidation::check(editor, catalog, {2})` a `editor/src/level_editor.cpp:2186`, mentre la mateixa sessió sí que llegeix els GIDs reals del tileset per desar (:2046-2049). Validar i desar no coincideixen.
- **La validació només mira la capa activa.** `EditorValidation::isWalkable()` crida `editor.tileAt(x, y)` (`include/Editor/EditorValidation.h:42`), que és la capa activa. Amb el terra a la capa 0 i la decoració seleccionada, V informa "el mapa no tiene tiles transitables".
- **`checkRpg()` és codi mort a la UI**: existeix a `include/Editor/EditorValidation.h:106-186` i només el crida `examples/demo_editor_state.cpp:219,236`. La validació de contingut RPG abans del playtest, que és per a què es va escriure, mai no s'executa des de l'editor.
- **Historial per snapshot complet.** `EditorState::Snapshot` (`include/Editor/EditorState.h:289-294`) copia totes les capes a cada operació, amb un límit de 256 (`src/Editor/EditorState.cpp:699`) i un `erase(begin())` O(n) sobre snapshots sencers (:700-702). És el paquet marcat `REPLANTEAR` (EDT-12) a `planificacion/ESTADO_REAL_vs_ROADMAP.md:174`.
- **`setLayerName()` no grava historial** (`src/Editor/EditorState.cpp:86-90`) però el nom sí que viatja al `Snapshot`: un desfer posterior desfà també el canvi de nom, sense que l'usuari hi hagi fet res.
- **Cap UI per esborrar ni reanomenar capes.** `removeLayer()` i `setLayerName()` existeixen a `EditorState` però `level_editor.cpp` no els crida mai (només `addLayer`, `setActiveLayer`, `layerName`, `layerCount`). I com que `TileMap` no llegeix el nom de la capa del TMX, reobrir un mapa el rebateja "capa2", "capa3" (comentat a `editor/src/level_editor.cpp:1166-1171`).
- **`pasteAt()` descarta capes en silenci** quan el portapapers en té més que el document destí (`src/Editor/EditorState.cpp:439`).
- **`ProjectHub::detail()` fa I/O de disc cada frame.** Crida `m_index.check()` (`src/Editor/ProjectHub.cpp:173`) i es crida al bucle de render a `editor/src/level_editor.cpp:869`; `check()` fa un `fopen` per fitxer declarat (`src/Project/ProjectIndex.cpp:217`). Amb `este_norte` (55 nivells + 55 mapes) són més de cent obertures de fitxer per frame, a 60 fps.
- **`ProjectHub::refresh()` s'empassa els errors d'escaneig** (`src/Editor/ProjectHub.cpp:207-213`): si el manifest es trenca mentre l'editor és obert, la llista es queda congelada sense dir res.
- **`ProjectHub::build()` compon una línia de shell amb la ruta del script sense cometes** (`src/Editor/ProjectHub.cpp:232`). L'id sí que està validat per `idSeguro()` (:39-50), però una arrel d'assets amb espais trencaria l'empaquetat.
- **Sense camí a esborrar projectes des de la UI**: `ProjectIndex::remove()` (`include/Project/ProjectIndex.h:78`) no el crida ningú fora dels tests i demos.
- **`DocumentRef` és un tipus orfe** (`include/Project/DocumentRef.h:13`): la identitat estable per `DocumentId` que promet no s'aplica enlloc, perquè `Editor::Project` guarda noms de fitxer com a `std::string` (`include/Project/Project.h:25-28`) i `ObjectSpawn::targetLevel` guarda rutes.
- **`ProjectSettings` és decoratiu al camí viu**: els quatre directoris configurables (`include/Project/ProjectSettings.h:16-19`) mai no es llegeixen; `ProjectIndex::check()` té `"levels"`, `"maps"`, `"adventures"`, `"objects"` cablejats (`src/Project/ProjectIndex.cpp:212-213`). `settings.initialMap` no es fa servir per obrir res.
- **`ObjectInspector::confirmar()` està declarat i mai definit ni cridat** (`include/Editor/ObjectInspector.h:111`), i `m_filtro` (:133) és un membre mort.
- **Codi duplicat entre camins**: `nombreEscenario()` existeix idèntic a `src/Editor/ObjectInspector.cpp:17-25` i `editor/src/level_editor.cpp:206`; el pas a minúscules ASCII està a `src/Editor/EditorState.cpp:48-56` i repetit dins d'un lambda a `src/Editor/ObjectInspector.cpp:268-276`; les regles d'id `[a-z0-9_]` estan tres cops (`ids::isValidValue` a `StrongId.h:13-24`, `isProjectId` a `Project.cpp:12-23`, el bucle de `ProjectIndex::create` a `ProjectIndex.cpp:244-249` i `idSeguro` a `ProjectHub.cpp:39-50`); i `setObjectTransition`/`setObjectTargetLevel` fan gairebé el mateix (`EditorState.cpp:351` i `:583`).
- **`exportTmx()` només admet un tileset** amb `firstgid=1` (`src/Editor/EditorState.cpp:764-777`), i `exportLevelJson()` no emet `schemaVersion` (:804-889) mentre el format canònic nou sí que en té (`ProjectSerializer.cpp:20-21`) — dos formats de nivell/mapa amb polítiques de versionat oposades.
- **`tests/Map/MapDocumentTests.cpp` no s'ha pogut llegir** en aquesta auditoria (error d'E/S del sistema de fitxers de l'usuari, `Resource deadlock avoided`); consta al `CMakeLists.txt:744` com a `test_map_document`, però no n'he pogut verificar la cobertura.
