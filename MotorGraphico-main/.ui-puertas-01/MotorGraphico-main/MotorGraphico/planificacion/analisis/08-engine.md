## Mòdul Engine (orquestrador)

`Application` és el `main` real del joc: a `init()` crea la finestra i el context GL (`Window`), carrega shader, catàlegs d'objectes i habilitats, quatre atles de textures, construeix el `Player`, carrega les quatre aventures de la campanya de Boundington, munta el nivell inicial via `loadLevel()` i finalment cableja unes 25 peces de HUD amb coordenades escrites a mà; a `run()` cedeix el ritme a `Engine::ApplicationLoop` (pas fix de 1/60 amb acumulador) passant-li cinc lambdes de servei — `shouldKeepRunning`/`pollEvents`/`processInput`/`fixedUpdate`/`render` — i a `shutdown()` destrueix els recursos GL en ordre invers a la creació. La relació amb `GameSession` és de traductor: `Application` no decideix res de partida (moviment legal, inici de combat, compra-venda), sinó que converteix tecles en crides (`tryMovePlayer`, `interact`, `buy`, `resolveAllyAction`) i llegeix l'estat de tornada per pintar-lo; la veritat viu a `GameSession`, i el que sobreviu a un canvi de nivell (jugador, inventari, skills, or, flags narratives) viu a `Application` perquè `loadLevel()` destrueix i reconstrueix la sessió sencera. Amb el renderer la relació és d'encolador: crea `Camera` + `IsometricRenderer`, hi encua el `Player` i un `Entity` per cada objecte/enemic (`m_worldMarkers`), i cada frame els reconcilia contra les llistes vives de la sessió (`syncWorldMarkers`) abans de cridar `renderFrame()` i, a sobre, `HudManager::render()` amb la seva pròpia `SpriteBatch`. `ApplicationLoop` i `Input/` ja estan connectats i en ús; `Scene/` està escrit i provat però no el fa servir ningú.

### `Application` — *include/Engine/Application.h* (318 línies; implementació a *src/Engine/Application.cpp*, 1682 línies)
- **Què és**: el cicle de joc complet i l'únic propietari de tot l'estat de sessió gràfica — finestra, GL, càmera, renderer, HUD, jugador, narrativa i guardat. És la classe més gran del projecte i el punt on convergeixen totes les fases del Gantt.
- **Herència**: cap (ni base ni derivades). Còpia i assignació esborrades (`Application.h:53-54`); moviment no declarat.
- **Estat** (tot privat):
  - *Finestra/GL*: `m_width`, `m_height`, `m_running`, `m_window`, `m_textureManager`, `m_shaderManager`, `m_spriteShader`.
  - *Render*: `m_camera`, `m_renderer`, `m_lastCameraCell` (inicialitzat a `{-9999,-9999}` per forçar el recentrat al primer frame), `m_atlas` (tileset, es reconstrueix a cada nivell), `m_characterAtlas` (12 frames de `personaje.png`), `m_raceNpcAtlas` (7×7 idle racial), `m_boundingtonStoryAtlas` (4×3 actors de campanya), `m_shadowTexture` (blob shadows), `m_map`, `m_player`, `m_worldMarkers`.
  - *HUD*: `m_hudBatch`, `m_whiteTexture`, `m_font`, `m_hud` (`HudManager`), i ~22 widgets: panell del jugador (`m_playerPanel`, `m_statusText`, `m_healthBar`, `m_manaBar`), enemic (`m_enemyPanel`, `m_enemyText`, `m_enemyBar`), `m_fpsText`, `m_minimap`, `m_hintText`, inventari (`m_inventoryPanel`, `m_inventoryText`, `m_showInventory`), diàleg (`m_dialoguePanel`, `m_dialogue`), combat (`m_commandPanel`, `m_commandMenu`), botiga (`m_shopPanel`, `m_shopMenu`, `m_shopTitle`, `m_shopSellOffset`, `m_sellIndices`), negoci (`m_businessPanel`, `m_businessText`, `m_businessMenu`), moral (`m_moralText`, `m_moralBar`) i `m_timerBar`. `m_fpsAccumTime`/`m_fpsAccumFrames` per al comptador.
  - *Partida*: `m_objectCatalog`, `m_skillCatalog`, `m_playerSkills`, `m_inventory`, `m_session`, `m_savePath` (`"savegame.json"`), `m_currentLevelPath`.
  - *Narrativa*: `m_narrativeState`, `m_narrative`, `m_prologo`/`m_dia1`/`m_dia2`/`m_ocaso`, `m_nd6Skills`, `m_narrativaLista`, `m_narrativeCampaignTransitions`.
  - *Input*: `m_input` (`Engine::Input::InputSystem`), `m_lastInput` (marca AFK, `steady_clock`).
- **API**:
  - *Cicle de vida (públic)*: `init(width, height, title, levelPath, catalogPath, initialAdventurePath, extraCatalogPaths) -> Result<bool>`, `run(maxFrames = -1)`, `shutdown()`.
  - *Frame (privat)*: `processInput()`, `update(float)`, `render(bool capture)`.
  - *Vista ↔ model (privat)*: `refreshHud()`, `syncWorldMarkers()`.
  - *Món (privat)*: `loadLevel(path, useEntry, entry)`, `applyPendingTransition()`.
  - *Persistència (privat)*: `saveGame(path)`, `loadGame(path)`.
  - *Menús (privat)*: `confirmShopCommand()`, `confirmBusinessCommand()`, `confirmBattleCommand()`.
  - *Struct interna*: `WorldMarker` (`Application.h:180-189`).
- **Flux**: `run()` (`Application.cpp:1594`) mesura `deltaTime` amb `glfwGetTime()` i el passa a `ApplicationLoop::tick`, que executa `pollEvents → processInput → N×update(1/60) → render(alpha)`; si `rendered == false` es surt del bucle. `update()` (`1426`) fa: animació del jugador → animació de tots els marcadors → càmera (`transitionTo` només si el jugador ha canviat de cel·la) → `m_session->update(dt)` (patrulles) → `applyPendingTransition()` (canvi de nivell per porta) → cadena de transicions de campanya prologo→Dia 1→Dia 2→Ocaso segons flags → `updateNarrativeTimers` amb dt zero si el jugador està ocupat o fa >15 s que no toca res → `syncWorldMarkers()` → `refreshHud()` → `m_hud.update(dt)` → comptador de FPS. `render()` (`1556`) neteja, fixa `setOcclusionFocus`, `renderFrame()`, dibuixa el HUD amb blending, opcionalment bolca `juego_output.ppm` i fa `swapBuffers()`.
  - **Mapa de tecles** (adaptador GLFW a `Application.cpp:164-180`, semàntica a `processInput`, `1320-1424`):
    - *Qualsevol mode*: `ESC` (`Quit`) → surt.
    - *`Exploration`*: `TAB` obre/tanca l'inventari; `F5` desa, `F9` carrega (i retorna immediatament perquè la sessió s'ha reconstruït); `E` interactua (parlar amb NPC / creuar porta); `W/A/S/D` mouen una cel·la per flanc, amb prioritat N > S > O > E (`if/else if`, no diagonals).
    - *`Battle`*: `W/S` mouen el menú, `ENTER`/`ESPAI` confirmen → `confirmBattleCommand()` (0 Atacar, 1 Tajo, 2 Cura, 3 Poció, altres Fugir). No hi ha tecla de cancel·lació.
    - *`Shop`*: `W/S` i `ENTER`/`ESPAI` → `confirmShopCommand()`; `E`/`TAB` (`Cancel`) tanquen.
    - *`Business`*: idèntic a `Shop`, però confirma amb `confirmBusinessCommand()`.
    - *`Dialogue`*: `ENTER`/`ESPAI` o `E` tanquen la interacció.
    - *`GameOver`*: cap tecla lligada; només `ESC`.
- **Observacions**: és el deute principal del mòdul. Fa com a mínim una dotzena de feines separables: (1) bootstrap d'assets i definició de regions dels quatre atles, amb dues taules d'`objectId → frame` codificades a mà (`Application.cpp:82-124`, 58 entrades); (2) contingut de joc (tres `Skill` hardcodejades a `259-270`, stats inicials del jugador a `351-354`, els 150 d'or de sortida a `424`); (3) construcció i *layout* del HUD (~240 línies de coordenades literals, `427-692`); (4) formatació de tot el text del HUD i construcció dinàmica de la llista de botiga i de la fitxa de negoci (`refreshHud`, 207 línies); (5) reconciliació vista↔model (`syncWorldMarkers`); (6) controladors de menú (botiga/negoci/combat); (7) I/O de guardat; (8) director de campanya (la cadena prologo→Ocaso dins d'`update`, `1478-1510`); (9) adaptador d'entrada; (10) càmera; (11) comptador de FPS; (12) captura de framebuffer a PPM. Extraccions naturals: un `EngineAssets`/`AssetBootstrap`, un `GameHud` (construcció + `refreshHud`), un `CampaignDirector`, un `SaveService`, un `WorldMarkerView` i controladors de menú per mode — cadascun testable sense GL, tal com ja s'ha fet amb `ApplicationLoop` i `InputSystem`. Comentaris de disseny molt bons i extensos, però són el 40% del fitxer i substitueixen documentació d'arquitectura que no té lloc propi.

### `Application::WorldMarker` — *include/Engine/Application.h:180-189* (10 línies)
- **Què és**: la representació *visual* d'un objecte o enemic de `GameSession`; parella `(id, posició)` + l'`Entity` encuada al renderer.
- **Herència**: cap. Struct niat privat dins d'`Application`.
- **Estat**: `id` (objectId del catàleg), `position` (cel·la), `isEnemy`, `enemyIndex` (índex dins de `GameSession::enemies()`, només vàlid si `isEnemy`), `entity` (`unique_ptr<Entity>`, propietat exclusiva).
- **API**: cap mètode; és pur agregat. El construeix la lambda `makeMarker` de `loadLevel` (`Application.cpp:807-871`) i el manté `syncWorldMarkers` (`1113-1176`).
- **Flux**: a `loadLevel` es crea un marcador per cada `worldObjects()` i cada `enemies()`; els NPC i enemics són `AnimatedEntity` amb sprite d'atles de campanya → racial → genèric, i els pickup/prop són `StaticEntity` amb tint per categoria. Cada frame, `syncWorldMarkers` fa mark-and-sweep: els enemics es reconcilien per índex (perquè patrullen) i se'ls actualitza la posició; els objectes, per `(id, posició)`; el que no té respatller es treu de la cua del renderer i mor. Després es reassignen els `enemyIndex` en una passada.
- **Observacions**: la reassignació d'índexs (`1163-1175`) empareja per `objectId` en ordre; amb dos enemics del mateix `objectId` on mor el primer, l'emparellament pot desplaçar-se i un marcador acabar seguint un altre enemic. La rutina d'objectes és O(marcadors × objectes) per frame — acceptable a l'escala actual, però no està acotat enlloc.

### `Window` — *include/Engine/Window.h* (42 línies; *src/Engine/Window.cpp*, 54 línies)
- **Què és**: embolcall prim de GLFW: crea finestra + context OpenGL 3.3 Core, carrega els punters de funció amb GLAD i exposa `pollEvents`/`swapBuffers`/`shouldClose`.
- **Herència**: cap.
- **Estat**: `m_handle` (`GLFWwindow*`), `m_width`, `m_height`.
- **API**: constructor `(width, height, title)`; `shouldClose()`, `hasInputFocus()`, `pollEvents()`, `swapBuffers()`, `width()`, `height()`, `handle()`. Còpia i moviment esborrats (`Window.h:21-27`).
- **Flux**: el constructor fa `glfwInit` → *hints* 3.3 Core (+ `FORWARD_COMPAT` a macOS) → `glfwCreateWindow` → `glfwMakeContextCurrent` → `gladLoadGLLoader` → `glViewport` → `glfwSwapInterval(1)`. Cada fallada llança `EngineException` després de netejar el que ja s'havia creat. El destructor destrueix la finestra i crida `glfwTerminate()`.
- **Observacions**: `glfwTerminate()` incondicional al destructor fa la classe implícitament singleton — dues instàncies simultànies trencarien GLFW, i això no està imposat pel tipus. No hi ha *callback* de redimensionat ni de *framebuffer size*, així que `Application::m_width/m_height` queden desincronitzats si l'usuari redimensiona: el HUD es projectaria contra mides falses. `pollEvents()`/`swapBuffers()` són `const` però muten l'estat de GLFW. Tampoc hi ha `glfwSetErrorCallback`, de manera que el motiu real d'un `glfwInit()` fallit es perd.

### `InputState` — *include/Engine/InputState.h* (13 línies)
- **Què és**: contracte mínim d'entrada de moviment de 4 direccions que consumeix `Player::handleInput`; deliberadament sense res de GLFW.
- **Herència**: cap.
- **Estat**: `moveUp`, `moveDown`, `moveLeft`, `moveRight` (tots `false` per defecte).
- **API**: cap; struct pur.
- **Flux**: `Application::processInput` el construeix a partir del `dx/dy` ja decidit (`Application.cpp:1380-1384`) només per disparar l'animació del `Player`; la posició autoritativa la torna a fixar `setGridPosition(m_session->playerPosition())` tot seguit.
- **Observacions**: és una peça heretada que ara conviu amb `Engine::Input::InputAction`, que cobreix el mateix i més. Amb `Player::handleInput` movent també la graella, `Application` ha d'anul·lar-ho immediatament — un doble camí de moviment que convida a errors.

### `ApplicationLoopSettings` — *include/Engine/ApplicationLoop.h:11-23* (13 línies)
- **Què és**: configuració del ritme del bucle.
- **Herència**: cap.
- **Estat**: `fixedStepSeconds` (1/60), `maxFrameSeconds` (0.10, sostre contra pauses de depurador o finestra minimitzada), `maxFixedUpdatesPerFrame` (6, límit contra la *spiral of death*).
- **API**: cap.
- **Flux**: el constructor d'`ApplicationLoop` normalitza els tres camps si són no finits, ≤0 o zero (`ApplicationLoop.cpp:25-33`).
- **Observacions**: cap.

### `ApplicationLoopFrame` — *include/Engine/ApplicationLoop.h:25-35* (11 línies)
- **Què és**: informe observable del que ha passat en un frame.
- **Herència**: cap.
- **Estat**: `rendered`, `inputProcessed`, `fixedUpdates`, `acceptedFrameSeconds`, `droppedSimulationSeconds` (temps de simulació descartat explícitament), `interpolationAlpha` (fracció [0,1) pendent del pas fix).
- **API**: cap.
- **Flux**: `tick()` el retorna sempre; `run()` només en llegeix `rendered` per decidir si surt (`Application.cpp:1620`).
- **Observacions**: `interpolationAlpha` i `droppedSimulationSeconds` són informació que ningú consumeix encara — el comentari de `Application.cpp:1631` ho reconeix ("queda disponible per a RUN-07.4").

### `ApplicationLoopServices` — *include/Engine/ApplicationLoop.h:37-45* (9 línies)
- **Què és**: les cinc operacions de plataforma/joc que el bucle necessita, injectades com a `std::function`.
- **Herència**: cap.
- **Estat**: `shouldKeepRunning`, `pollEvents`, `processInput`, `fixedUpdate(float)`, `render(float)`.
- **API**: cap.
- **Flux**: `hasRequiredServices()` (`ApplicationLoop.cpp:14-18`) exigeix les cinc; si en falta una, `tick()` no executa res, per no amagar una integració a mitges.
- **Observacions**: cinc `std::function` construïdes de nou cada frame a `Application::run` (`1610-1618`) — assignació per frame evitable movent l'struct fora del bucle.

### `ApplicationLoop` — *include/Engine/ApplicationLoop.h:47-62* (16 línies; *src/Engine/ApplicationLoop.cpp*, 99 línies)
- **Què és**: el nucli del bucle amb pas fix i acumulador, sense GLFW, GL ni lògica de joc — l'única peça de l'orquestrador verificable sense finestra.
- **Herència**: cap.
- **Estat**: `m_settings`, `m_accumulatorSeconds` (`double`, per no perdre precisió acumulant floats).
- **API**: constructor amb `ApplicationLoopSettings`, `settings()`, `reset()`, `tick(realFrameSeconds, services)`.
- **Flux**: `tick` comprova serveis i `shouldKeepRunning` → `pollEvents` → *recheck* → `processInput` → *recheck* → sanejat del delta (NaN/inf → 0, negatiu → 0, sostre a `maxFrameSeconds`) → acumula → bucle de `fixedUpdate` mentre hi càpiga un pas i no s'esgoti el pressupost, amb `kClockEpsilon` per absorbir l'error de coma flotant → si s'ha esgotat el pressupost, descarta l'endarreriment sencer excepte la fracció interpolable i ho reporta a `droppedSimulationSeconds` → calcula `interpolationAlpha` → `render`.
- **Observacions**: peça nova, ben feta i **connectada**: `Application::run` la fa servir (`Application.cpp:1599`) i té bateria pròpia (`tests/Engine/ApplicationLoopTests.cpp`, objectiu `test_application_loop` a `CMakeLists.txt:721`). Cap deute intern; els riscos són a l'integrador, no aquí.

### `RawInputState` — *include/Engine/Input/InputMap.h:50-57* (8 línies)
- **Què és**: fotografia de l'estat físic de les tecles d'un frame, construïble des de GLFW o des d'un test.
- **Herència**: cap.
- **Estat**: `std::array<bool, kInputKeyCount> m_down`.
- **API**: `isDown(InputKey)`, `setDown(InputKey, bool = true)`.
- **Flux**: l'omple `readGameKeyboard()` (`Application.cpp:164-180`), l'únic punt del projecte que coneix codis `GLFW_KEY_*`.
- **Observacions**: `isDown`/`setDown` indexen sense comprovar; passar-hi `InputKey::Count` és accés fora de rang silenciós.

### `InputMap` — *include/Engine/Input/InputMap.h:59-72* (14 línies)
- **Què és**: mapa acció→tecles, on es concentra tota decisió de *quina tecla fa què*.
- **Herència**: cap.
- **Estat**: `std::array<std::vector<InputKey>, kInputActionCount> m_bindings`.
- **API**: `gameplayDefaults()` (estàtica), `bind`, `clear`, `isBound`, `bindings`.
- **Flux**: `gameplayDefaults()` (`InputSystem.cpp:18-36`) lliga `Quit←ESC`, `MoveNorth←W`, `MoveSouth←S`, `MoveWest←A`, `MoveEast←D`, `Interact←E`, `ToggleInventory←TAB`, `QuickSave←F5`, `QuickLoad←F9`, `MenuUp←W`, `MenuDown←S`, `Confirm←ENTER/ESPAI`, `Cancel←E/TAB`.
- **Observacions**: els solapaments són intencionats (W és `MoveNorth` i `MenuUp`) i funcionen perquè `Application` desambigua per `GameMode`; però això vol dir que el mapa sol no descriu el comportament — cal llegir `processInput` per entendre'l. `InputKey` només cobreix 11 tecles: no hi ha manera de lligar res més sense tocar l'enum i l'adaptador GLFW alhora. No hi ha serialització del mapa (remapatge per l'usuari) ni cap acció de gamepad.

### `InputSnapshot` — *include/Engine/Input/InputSystem.h:12-28* (17 línies)
- **Què és**: instantània immutable de les accions d'un frame amb `pressed`/`held`/`released` ja resolts, perquè cap consumidor s'hagi de fer el seu detector de flancs.
- **Herència**: cap. Declara `friend class InputSystem`.
- **Estat**: `std::array<ActionState, kInputActionCount> m_actions`, on `ActionState` és un struct niat amb `pressed`/`held`/`released`.
- **API**: `pressed(action)`, `held(action)`, `released(action)`, `hasActivity()`.
- **Flux**: només `InputSystem::update` el pot omplir (amistat). `hasActivity()` l'usa `Application::processInput` (`1328`) per refrescar la marca AFK únicament quan hi ha hagut activitat real.
- **Observacions**: cap. El comentari del codi documenta que abans la marca AFK es refrescava per sondeig i el temporitzador narratiu no arribava mai a considerar el jugador absent — corregit.

### `InputSystem` — *include/Engine/Input/InputSystem.h:30-53* (24 línies; *src/Engine/Input/InputSystem.cpp*, 108 línies)
- **Què és**: conserva l'estat físic del frame anterior i en deriva les transicions d'acció d'aquest.
- **Herència**: cap.
- **Estat**: `m_map`, `m_previous`.
- **API**: constructor amb `InputMap` (per defecte `gameplayDefaults()`), `map()`, `setMap()` (que fa `reset()`), `reset()`, `update(RawInputState)` (per valor, a propòsit), `releaseAll()`.
- **Flux**: `update` recorre les `kInputActionCount` accions, calcula `wasDown`/`isDown` amb `actionIsDown` (una acció està premuda si *qualsevol* tecla lligada ho està), n'omple el snapshot i desa l'estat actual. `releaseAll()` és `update(RawInputState{})`: genera `released` per tot el que estava `held`, i s'invoca quan la finestra perd el focus (`Application.cpp:1324`) perquè cap tecla no quedi "enganxada" en tornar.
- **Observacions**: peça nova, **connectada i provada** (`tests/Engine/InputSystemTests.cpp`, objectiu `test_input_system`). Cap deute intern.

### `Scene` — *include/Engine/Scene/Scene.h:13-34* (22 línies)
- **Què és**: contracte mínim d'escena (RUN-03): només `load()` pot fallar; la resta de *hooks* són per frame i no retornen error.
- **Herència**: base abstracta pura. **Cap classe de producció n'hereta**; l'única implementació és `TestScene` a `tests/Engine/SceneManagerTests.cpp:19`.
- **Estat**: cap (interfície).
- **API**: purs `name()` i `load()`; virtuals amb cos buit `mapId()` (retorna `ids::MapId{}` per als *overlays* sense mapa), `onEnter`, `onPause`, `onResume`, `onExit`, `fixedUpdate`, `update`, `render(interpolationAlpha)`.
- **Flux**: l'escena entrant carrega mentre l'actual segueix viva; si `load()` falla, el manager conserva l'anterior sense haver-li cridat cap *hook*.
- **Observacions**: **peça encara no connectada**. `Application` no la coneix (no hi ha cap `#include "Engine/Scene/..."` fora de `src/Engine/Scene` i dels tests), i la seva firma `render(float interpolationAlpha)` encaixa exactament amb el que `ApplicationLoop` ja calcula i `Application` descarta — el disseny està pensat, la connexió no s'ha fet.

### `SceneManager` — *include/Engine/Scene/SceneManager.h:12-58* (47 línies; *src/Engine/Scene/SceneManager.cpp*, 146 línies)
- **Què és**: pila d'escenes amb transicions diferides (Replace/Push/Pop) aplicades al final del frame.
- **Herència**: cap. Còpia i assignació esborrades.
- **Estat**: `m_stack` (`vector<unique_ptr<Scene>>`), `m_pending` (`unique_ptr<PendingTransition>`); l'struct niat privat `PendingTransition` (`SceneManager.h:49-52`) té `kind` (`TransitionKind::Replace` per defecte) i `incoming`.
- **API**: `start(initialScene)`, `shutdown()`, `requestReplace/requestPush/requestPop`, `hasPendingTransition()`, `applyPendingTransition()`, `activeScene()` (const i no-const), `activeMapId()`, `stackDepth()`, `fixedUpdate`, `update`, `render`.
- **Flux**: `start` rebutja nul, pila no buida o transició pendent, carrega, i només llavors empeny i crida `onEnter`. `request` només admet una transició per frame — decisió explícita perquè l'ordre sigui observable i un *overlay* creat per un altre *overlay* no executi dos cicles en el mateix frame — i prohibeix retirar l'escena arrel. `applyPendingTransition` consumeix la petició abans d'invocar cap *hook* (evita reentrades): Pop fa `onExit`+`pop`+`onResume`; Push carrega, `onPause`, empeny, `onEnter`; Replace carrega, `onExit`, substitueix, `onEnter`. `shutdown` buida la pila cridant `onExit` de dalt a baix i el destructor la crida.
- **Observacions**: **peça no connectada**. `motor_scene` només s'enllaça amb `test_scene_manager` (`CMakeLists.txt:728`); no forma part de `motor_core` ni de l'executable `juego`. Detall d'implementació: si `load()` de la transició entrant falla, la petició es descarta i l'escena entrant es destrueix silenciosament dins de `pending` — el missatge d'error torna al cridant, però l'escena a mig construir es perd sense cap *hook* de neteja.

### Buits i riscos del mòdul

- **Ús de memòria alliberada al minimapa a cada canvi de nivell.** `m_minimap` es construeix una sola vegada amb `m_session.get()` (`src/Engine/Application.cpp:512-513`) i `HudMinimap` guarda `const GameSession* m_session` sense cap *setter* (`include/Render/HudMinimap.h:54`). `loadLevel()` substitueix `m_session` per un `GameSession` nou (`src/Engine/Application.cpp:778`) i destrueix l'anterior: a partir de la primera porta creuada, el minimapa llegeix cada frame un objecte mort. És el risc més greu del bloc.
- **Recursos GL destruïts després de `glfwTerminate()`.** `shutdown()` (`src/Engine/Application.cpp:1639-1682`) allibera els widgets i acaba amb `m_window.reset()`, però mai buida `m_textureManager` ni `m_shaderManager` (`include/Engine/Application.h:138-139`). Aquests es destrueixen més tard, al destructor d'`Application`, i llavors `~Texture` crida `glDeleteTextures` (`src/Core/Resources/Texture.cpp:7`) i `~Shader` crida `glDeleteProgram` (`src/Core/Resources/Shader.cpp:61`) sense context viu. `m_raceNpcAtlas` i `m_boundingtonStoryAtlas` tampoc es reinicien allà (només `m_atlas` i `m_characterAtlas`, línies 1679-1680), incoherència que delata que la llista manual s'ha quedat enrere.
- **Les aventures autònomes no arriben a executar-se.** Amb `--precuela` (o qualsevol `initialAdventurePath`), `init()` posa `m_narrativeCampaignTransitions = false` (`src/Engine/Application.cpp:382`), però `loadLevel()` només crida `setNarrative`/`setNd6SkillCatalog`/`enterLevelNarrative` si `m_narrativaLista && m_narrativeCampaignTransitions` (`src/Engine/Application.cpp:788-795`). Resultat: l'aventura alternativa es carrega i es fixa com a activa, però la `GameSession` no rep mai el motor narratiu i cap *beat* no dispara.
- **La cadena de campanya no respecta la mateixa condició.** La transició prologo→Dia 1→Dia 2→Ocaso d'`update()` només comprova `m_narrativaLista` (`src/Engine/Application.cpp:1478`), no `m_narrativeCampaignTransitions`, al contrari del criteri documentat a `include/Engine/Application.h:210-211`. Una aventura autònoma que casualment posés `bnd_prologo_terminado` forçaria una recàrrega a `ciudad_centro.json`.
- **La càrrega ràpida no restaura quina aventura és l'activa.** `SaveGameData` desa flags, *beats* i temporitzadors (`include/Game/SaveGame.h:91-93`) però no l'`AdventureScript` en curs; `loadGame()` (`src/Engine/Application.cpp:1275-1318`) restaura l'estat i el nivell, i com que les flags `bnd_diaN_empezado` tornen posades, la cadena d'`update()` ja no corregeix res: `m_narrative` es queda apuntant a l'aventura que hi hagués al procés.
- **El comptador de "FPS reals" mesura el pas fix, no el temps real.** `m_fpsAccumTime += deltaTime` viu dins d'`update()` (`src/Engine/Application.cpp:1545`), que `ApplicationLoop` invoca sempre amb `fixedStepSeconds`; el número mostrat convergeix a 60 sigui quin sigui el rendiment, malgrat el comentari de `include/Engine/Application.h:314-315`.
- **`loadLevel()` es pot executar dins d'un `fixedUpdate` intermedi.** Amb un frame lent, `ApplicationLoop` crida `update()` fins a 6 vegades (`include/Engine/ApplicationLoop.h:22`); si la primera passada creua una porta o dispara una transició d'aventura (`src/Engine/Application.cpp:1467`, `1487`, `1508`), les passades següents operen sobre una sessió i uns marcadors acabats de reconstruir, dins del mateix frame.
- **La barra de temporitzador de missió és codi mort.** `m_timerBar` es crea, es configura i s'afegeix al HUD (`src/Engine/Application.cpp:656-664`) amb `setVisible(false)`, i cap altre punt del fitxer la torna a tocar: la funcionalitat F1 documentada a `include/Engine/Application.h:306-309` no es dibuixa mai. A més s'afegeix al `HudManager` abans que els panells (línia 664 contra 669+), trencant la regla d'ordre d'inserció que enuncia el comentari de la línia 667.
- **`Scene`/`SceneManager` són codi escrit, provat i desconnectat.** `motor_scene` només s'enllaça amb `test_scene_manager` (`CMakeLists.txt:128` i `728`), no amb `motor_core` (`CMakeLists.txt:521`). El bucle ja produeix `interpolationAlpha` i `Scene::render(float)` l'espera: la integració està a mig camí i, mentrestant, `Application` fa de gestor d'escena implícit amb `loadLevel`.
- **Cap gestió del redimensionat de finestra.** `include/Engine/Window.h` no registra cap *callback* de mida i `m_width`/`m_height` d'`Application` es fixen a `init()`: en redimensionar, el `glViewport`, la projecció del HUD i el volcat PPM queden contra mides obsoletes.
- **Fallades d'assets amb dos criteris diferents dins d'`init()`.** Shaders, catàlegs, textures de personatge, atles racial, atles de campanya i nivell inicial retornen `Result::Error` i avorten l'arranc (`src/Engine/Application.cpp:236-239`, `243-253`, `279-282`, `300-302`, `313-316`, `416-419`); en canvi les quatre aventures i el catàleg Nd6 fallen *tou*: s'imprimeix el motiu a `stderr`, `m_narrativaLista` queda a `false` i el joc arrenca sense narrativa (`367-401`). L'excepció dins de l'excepció: si la *campanya* carrega bé però l'aventura passada per `--precuela` falla, sí que s'avorta (`378`). L'asimetria és defensable però no està documentada al contracte d'`init()` a `include/Engine/Application.h:56-64`, i el jugador que perdi `boundington_prologo.json` obté un joc que arrenca i sembla sencer.
- **`applyPendingTransition()` i `refreshHud()` desreferencien `m_session` sense comprovar-lo** (`src/Engine/Application.cpp:889`, `905`), mentre que `render()` sí que ho fa (`1563`) i `saveGame()` també (`1261`). `run()` també llegeix `m_session->mode()` al missatge final (`1635`), de manera que cridar `run()` després d'un `init()` fallit peta en lloc de sortir.
- **El `Player` es construeix amb un atles nul.** `m_player` (línia 326) i `IsometricRenderer` (línia 404) reben `m_atlas.get()` quan `m_atlas` encara no existeix; només queda vàlid quan `loadLevel()` fa `setAtlas` a les línies 755-758. Funciona per ordre d'execució, però és una invariant no expressada al tipus.
- **`m_shopMenu` es reconstrueix cada frame amb `setOptions` mentre l'índex seleccionat es conserva** (`src/Engine/Application.cpp:1008`): si el nombre de línies encongeix en vendre l'últim exemplar d'un objecte, la selecció pot quedar fora de rang abans que `confirmShopCommand` (`1200-1213`) la validi. Els guardes de `confirmShopCommand` eviten el pitjor, però `m_session->buy(selected)` es crida sense comprovar `selected < shopOffers().size()`.
- **No hi ha cap test del bloc `Application`.** `tests/Engine/` només conté `ApplicationLoopTests.cpp`, `InputSystemTests.cpp` i `SceneManagerTests.cpp`: les 1682 línies d'`src/Engine/Application.cpp` — inclosos `refreshHud`, `syncWorldMarkers`, `loadLevel`, `saveGame`/`loadGame` i el director de campanya, tots lògica pura i separable de GL — no tenen cap cobertura automàtica.
