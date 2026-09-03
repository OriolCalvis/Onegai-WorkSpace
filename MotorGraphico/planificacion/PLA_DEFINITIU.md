# Pla definitiu — MotorGraphico Creator, corte B

Generat des de `planificacion/pla/dades.py` i `planificacion/seguimiento/*.json`. Aquest fitxer no s'edita a mà: es regenera, i així el markdown, el Gantt i la pàgina no poden divergir.

## 1. Les tres decisions que fixen el pla

**Objectiu de versió → Corte B — cicle d'autor complet.** Que algú que no sigui l'Oriol pugui fer un joc amb l'editor: mapes, esdeveniments i playtest sense tocar codi. Autotiles, plugins, exportació a més plataformes i àudio queden fora, a 1.1.

**Catàlegs RPG → Arreglar-los tots els 21.** Cada carregador ha de llegir de veritat el seu JSON, amb una prova que comprovi el contingut d'un camp i no només que hi hagi entrades.

**Codi orfe → Consolidació prèvia, abans de res més.** MapDocument, SceneManager, World, HandlePool, ProjectSerializer i el post-processat es connecten primer. Quan la fase C tanqui, no hi ha d'haver dues implementacions vives del mateix concepte.

## 2. El número

| Fase | Paquets | Jornades | Setmanes a ritme real |
| --- | ---: | ---: | ---: |
| C — Consolidació | 24 | 62 | 41 |
| D — Dades reals | 10 | 30 | 20 |
| E — Esdeveniments editables | 9 | 46 | 31 |
| F — Playtest des de l'editor | 4 | 13 | 9 |
| G — Tancament del corte B | 7 | 26 | 17 |
| **Total** | **54** | **177** | **118** |

Una jornada és ~6 h de treball enfocat. A ritme de ratos lliures i caps de setmana (~1,5 jornades per setmana), les 177 jornades són unes 118 setmanes naturals.

**Aquest número no és el de l'auditoria i convé saber per què.** El corte B s'hi xifrava en 72 jornades. Les tres decisions preses el multipliquen: la consolidació prèvia sencera afegeix 62 jornades i arreglar els 21 catàlegs en comptes dels quatre que el joc fa servir n'afegeix 30. Són decisions defensables — deixen el projecte sense mons paral·lels i amb dades reals — però el preu és aquest, i és millor tenir-lo escrit ara que descobrir-lo al mes vuit.

## 3. Què vol dir «fet» a cada fase

### Fase C — Consolidació

Una sola veritat per concepte, i cap bug que el jugador noti. És la fase que no afegeix cap funció nova i sense la qual totes les altres es construeixen damunt de dos fonaments alhora.

**Tancada quan:** Cap concepte del motor té dues implementacions vives, els vuit bugs crítics estan tancats amb prova de regressió, i el camí que es juga de veritat té tests.

### Fase D — Dades reals

Que el que diu el JSON sigui el que arriba al joc. Avui 19 dels 21 carregadors llegeixen cinc camps i s'inventen la resta en silenci.

**Tancada quan:** Els 21 catàlegs carreguen tots els seus camps, un carregador que no troba el que espera falla en comptes de callar, i CI valida els assets reals a cada commit.

### Fase E — Esdeveniments editables

El cor del corte B: que una missió es pugui escriure des de l'editor, sense tocar C++ ni JSON a mà.

**Tancada quan:** La missió de la clau, la porta i el cofre s'escriu sencera des de l'editor visual, es valida abans d'executar-se i es pot depurar pas a pas.

### Fase F — Playtest des de l'editor

El bucle d'autoria: editar, provar, corregir sense sortir de l'eina.

**Tancada quan:** Des d'un mapa obert, una tecla llança la partida en aquell punt, i tornar a l'editor conserva el que s'estava editant.

### Fase G — Tancament del corte B

El que converteix una eina que funciona en una eina que es pot donar a algú altre.

**Tancada quan:** Un tercer instal·la el paquet, segueix el tutorial i publica un joc jugable sense ajuda.

## 4. Els paquets

Cada paquet porta el seu criteri d'acceptació i la prova que el tanca. Si una d'aquestes dues coses no es pot escriure, el paquet no està prou definit per començar-lo.

### Fase C — Consolidació

#### C-01 — La curació cura qui toca

`2 j` · depèn de: cap

**Per què.** Avui l'ordre «Cura» gasta 4 PM del jugador i cura ~20 PV al monstre. `resolveAllyAction` tria sempre l'objectiu dins de `m_enemies` i ningú no llegeix `Skill::target`.

**Fitxers.** `src/Game/BattleState.cpp`, `include/Game/Skill.h`, `tests/Game/BattleStateTests.cpp`

**Fet quan.** Una habilitat amb `target = Ally` aplica el seu efecte sobre el jugador; una amb `target = Enemy`, sobre l'enemic seleccionat. El PM es gasta una sola vegada i només si l'efecte s'ha pogut aplicar.

**Ho prova.** `test_battle_state`: curar amb vida a 30/60 deixa el jugador a 50 i l'enemic intacte; el mateix cas amb el codi antic falla.

#### C-02 — El minimapa deixa de llegir memòria alliberada

`1 j` · depèn de: cap

**Per què.** `HudMinimap` guarda el punter a la `GameSession` del primer nivell i no té setter. Des de la primera porta creuada llegeix un objecte destruït a cada frame: avui no peta per sort.

**Fitxers.** `include/Render/HudMinimap.h`, `src/Render/HudMinimap.cpp`, `src/Engine/Application.cpp`

**Fet quan.** El minimapa rep la sessió per `setSession()` i `loadLevel` l'hi torna a passar. Amb sessió nul·la es dibuixa buit en comptes de llegir.

**Ho prova.** `test_hud_minimap`: construir, canviar de sessió i destruir la primera; el widget segueix responent i no toca la vella. Verificat també amb `-fsanitize=address` sobre `demo_game_session`.

#### C-03 — L'estat del jugador sobreviu el canvi de nivell

`2 j` · depèn de: cap

**Per què.** `loadLevel` rescata només l'or. Entrar a una posada et reinicia la reputació a 0, et deixa sense negocis i esborra el comptador de rendes.

**Fitxers.** `src/Engine/Application.cpp`, `include/Game/GameSession.h`, `src/Game/SaveGame.cpp`

**Fet quan.** Moral, negocis, passos des de l'últim cobrament i or es transfereixen sencers entre sessions. El traspàs és una sola funció, no una llista de camps escampada per `loadLevel`.

**Ho prova.** `test_game_session`: sessió amb moral −30, dos negocis i 90 d'or; reconstruir-la sobre un altre nivell i comprovar els quatre valors.

#### C-05 — Tancament ordenat dels recursos GL

`1 j` · depèn de: cap

**Per què.** `shutdown()` no buida els dos ResourceManager: es destrueixen al destructor d'Application, quan `glfwTerminate()` ja ha passat i `glDeleteTextures` no té context on anar.

**Fitxers.** `src/Engine/Application.cpp`, `include/Engine/Application.h`

**Fet quan.** Textures i shaders s'alliberen mentre el context és viu. Sortir del joc no deixa cap crida GL després de `glfwTerminate()`.

**Ho prova.** Execució de `juego` amb `--frames 3` sota `ASAN_OPTIONS=detect_leaks=1`, sense fuites de textura.

#### C-06 — L'ajust de text no es penja

`1 j` · depèn de: cap

**Per què.** `wrapText` amb una caixa d'un sol glifu d'amplada no consumeix res i afegeix «-» per sempre. Hi arriba qualsevol `HudText` prou estret.

**Fitxers.** `src/Render/BitmapFont.cpp`, `tests/Render/BitmapFontTests.cpp`

**Fet quan.** Amb una amplada menor que un glifu, `wrapText` retorna el text tallat sense inventar guions i sense bucle.

**Ho prova.** `test_bitmap_font`: amplades 0, 1 i 2 sobre una frase llarga; la funció retorna en temps finit i el resultat cap a la caixa.

#### C-07 — Les aventures autònomes disparen els seus beats

`2 j` · depèn de: cap

**Per què.** Amb `--precuela`, `m_narrativeCampaignTransitions` queda a fals i `loadLevel` no crida mai `setNarrative`: l'aventura es carrega i no passa absolutament res.

**Fitxers.** `src/Engine/Application.cpp`, `include/Engine/Application.h`

**Fet quan.** Qualsevol aventura carregada s'enganxa a la sessió. La bandera de campanya només decideix si es fan les transicions de la cronologia, no si la narrativa existeix.

**Ho prova.** `demo_precuela_boundington` promogut a CTest comprova que el primer beat es dispara; abans no ho feia.

#### C-08 — Un guardat antic no escriu fora de rang

`1 j` · depèn de: cap

**Per què.** `tickTimers` indexa `avisosDados[i]` contra la mida de la definició, i el guardat el restaura amb la mida que tenia el dia que es va desar. Afegir un avís a una aventura converteix els guardats existents en escriptura fora de rang.

**Fitxers.** `src/RPG/NarrativeEngine.cpp`, `src/Game/SaveGame.cpp`, `tests/Game/SaveGameTests.cpp`

**Fet quan.** En restaurar, el vector d'avisos es redimensiona a la mida de la definició actual, conservant els que ja s'havien donat.

**Ho prova.** `test_save_game`: guardat amb dos avisos restaurat sobre una definició de quatre; cap escriptura fora de rang sota ASan.

#### C-09 — Propietat de recursos i destrucció polimòrfica

`1 j` · depèn de: cap

**Per què.** `Texture::operator=(Texture&&)` sobreescriu `m_glID` sense alliberar el que ja tenia, i `RuntimeTileMap` no té destructor virtual tot i que `TileMap` n'hereta i el renderer en guarda un punter a la base.

**Fitxers.** `include/Core/Resources/Texture.h`, `include/Runtime/RuntimeTileMap.h`

**Fet quan.** L'assignació per moviment allibera el recurs de destí abans de robar el d'origen. `RuntimeTileMap` té destructor virtual.

**Ho prova.** `test_texture` amb un doble GL fals: assignar sobre una textura viva n'allibera el nom exactament una vegada.

#### C-10 — El JSON que escrivim el podem tornar a llegir

`1 j` · depèn de: cap

**Per què.** `JsonWriter` emet `\u00xx` per als caràcters de control i `JsonParser` rebutja `\u` explícitament: el round-trip que la capçalera diu que és la comprovació que importa està trencat.

**Fitxers.** `include/Core/Json/JsonWriter.h`, `src/Core/Json/JsonValue.cpp`, `tests/Core/JsonTests.cpp`

**Fet quan.** El parser entén `\uXXXX` i el codifica a UTF-8. Tot el que surt de `JsonWriter` torna a entrar per `JsonValue::parse` sense pèrdua.

**Ho prova.** `test_json`: round-trip d'una cadena amb caràcters de control, accents, cometes i barres.

#### C-11 — Un sol format de projecte

`4 j` · depèn de: cap

**Per què.** `ProjectIndex` llegeix claus castellanes a l'arrel i `ProjectSerializer` escriu claus angleses amb `settings` niat. Cap dels dos pot llegir el fitxer de l'altre, i tots dos es diuen canònics.

**Fitxers.** `src/Project/ProjectSerializer.cpp`, `src/Project/ProjectIndex.cpp`, `FORMATO_PROYECTOS.md`, `tools/migrar_proyectos.py`

**Fet quan.** Un únic format v1, amb `schemaVersion`, que `ProjectSerializer` llegeix i escriu i que `ProjectIndex` consumeix. Els cinc manifests d'`assets/proyectos/` queden migrats i el format vell es rebutja amb un missatge que diu com migrar.

**Ho prova.** `test_project_serialization`: carregar els cinc manifests reals migrats, desar-los i comparar bytes.

**Seguiment · 2026-09-03 · Codex: Implementat i verificat al workspace; integració Git i CI pendents.**

- ProjectIndex llegeix i crea manifests mitjançant ProjectSerializer; un únic format v1 amb settings niat. El format antic o mixt es rebutja amb el comandament de migració.
- Migrats els cinc manifests reals amb metadades i referències conservades; migrador amb previsualització, còpia prèvia, validació del lot i substitució atòmica de cada fitxer. Una segona execució no canvia res.
- Actualitzats build_proyecto.py, validar_todo.py, FORMATO_PROYECTOS.md i el schema v1; les rutes es resolen respecte de la carpeta assets indicada.
- Crear o eliminar un projecte conserva referències a altres manifests invàlids o absents; l’índex buit permet crear el primer projecte.
- Afegits test_project_manifests i test_project_migration; els canvis coordinats sobre la base aliena sense versionar queden aplicats al workspace i preservats a C-11-integracion.patch.

**Verificat.** Suite GL-free local: 56/56 proves, 0 errors. Després dels ajustos finals, les tres proves test_project_* tornen a passar. · Round-trip byte a byte dels cinc manifests reals, equivalència ProjectIndex/ProjectSerializer, rebuig de camps desconeguts i resolució d’entrades sobre assets copiat. · Migrador: 6/6 casos Python; previsualització sense escriptura, idempotència, índex buit i cap migració parcial per dades invàlides. · Comparació amb el builder de HEAD sobre els manifests antics: build.json idèntic en els cinc packs. demo_proyectos passa; validar_proyectos dona 0 errors i 2 avisos previs.

**Pendent.** Incorporar en Git la base Project/Map/CTest prèvia amb el seu autor i després els canvis coordinats de C-11. No s’ha commitejat treball aliè ni una migració de dades sense el lector corresponent. · Validar a CI per tancar formalment el paquet. Els tres packs no jugables mantenen les mateixes mancances de contingut que abans de la migració.

#### C-12 — MapDocument es pot editar

`5 j` · depèn de: cap

**Per què.** El document de mapa nou és d'escriptura única: no té `removeLayer`, ni accés no-const a les dades d'una capa, ni manera de reordenar. Mentre sigui així, l'editor no s'hi pot migrar encara que es vulgui.

**Fitxers.** `include/Map/MapDocument.h`, `src/Map/MapDocument.cpp`, `include/Map/MapLayer.h`, `tests/Map/MapDocumentTests.cpp`

**Fet quan.** API de mutació completa: pintar cel·les, afegir i treure entitats i regions, esborrar capes, reordenar-les i canviar-ne els metadades. Cada mutació manté les invariants que avui només comprova `validate()`.

**Ho prova.** `test_map_document`: seqüència de 20 mutacions; el document valida després de cadascuna i el resultat és el mateix que construir-lo d'una peça.

#### C-16 — L'Application delega en SceneManager

`4 j` · depèn de: cap

**Per què.** `Scene` i `SceneManager` existeixen, estan provats i no els enllaça res que no sigui el seu test. Mentrestant l'Application porta les fases a mà amb banderes.

**Fitxers.** `src/Engine/Application.cpp`, `include/Engine/Scene/SceneManager.h`, `tests/Engine/SceneManagerTests.cpp`

**Fet quan.** El bucle principal empeny i treu escenes en comptes de mirar banderes. El canvi de nivell i el pas menú↔joc són transicions d'escena.

**Ho prova.** `test_scene_manager` ampliat amb les transicions reals del joc; `demo_game_session` segueix passant.

#### C-19 — Un sol culling i un sol contracte d'actualització

`2 j` · depèn de: cap

**Per què.** `IsometricRenderer::visibleRangeFor` i `TileMap::visibleRange` són còpies literals l'una de l'altra, i `IUpdatable` i `IHudElement::update` són dos contractes `update(float)` sense cap parentiu.

**Fitxers.** `include/Render/IsometricRenderer.h`, `include/Render/TileMap.h`, `include/Render/IUpdatable.h`, `include/Render/HudElement.h`

**Fet quan.** Una sola funció de culling, amb un sol propietari. Un sol contracte d'actualització per frame, o dos noms diferents si de veritat són coses diferents.

**Ho prova.** `test_occlusion`: la funció única dóna el mateix rang que les dues anteriors sobre deu càmeres i mides de mapa.

#### C-20 — Un sol escriptor de condicions

`2 j` · depèn de: cap

**Per què.** `CharacterSheet::applyCondition` i `ConditionEngine` fan la mateixa feina amb regles diferents: la fitxa no coneix ni `Stacking`, ni `maxStacks`, ni el recàlcul de modificadors.

**Fitxers.** `include/RPG/CharacterSheet.h`, `src/RPG/CharacterSheet.cpp`, `src/RPG/ConditionEngine.cpp`, `tests/RPG/ConditionEngineTests.cpp`

**Fet quan.** `ConditionEngine` és l'únic camí per aplicar, apilar i retirar condicions. La fitxa n'és el magatzem. A més, `purge_expired` purga de veritat i `breaksOnDamage` es dispara pel dany rebut, no per l'infligit.

**Ho prova.** `test_condition_engine`: apilar fins al màxim, expirar per rondes, trencar per dany rebut; cap camí passa per la fitxa.

**Seguiment · 2026-09-03 · Codex: Nucli implementat i verificat localment; paquet encara obert.**

- Retirats applyCondition, removeCondition i tickEndOfRound de CharacterSheet: ConditionEngine concentra les regles d'aplicació, apilat, retirada i caducitat.
- purge_expired elimina entrades amb rondes o càrregues esgotades i recalcula modificadors; les consultes ignoren entrades caducades.
- El tick conserva l'efecte de l'última ronda i evita tornar a aplicar efectes ja caducats.
- on_damage_received elimina totes les condicions breaksOnDamage del receptor i recalcula els modificadors; zero, curació i dany bloquejat no les trenquen.
- Afegit test_condition_engine amb 11 casos i registre CTest GL-free independent del scaffolding pendent d'altres sessions.

**Verificat.** Abans del canvi: 2 regressions reproduïdes (purga i ruptura pel dany calculat del tick). · 11/11 casos amb AddressSanitizer i UndefinedBehaviorSanitizer, sense errors. · Build de suite_gl_free i CTest: 54/54 proves GL-free en verd (macOS Debug).

**Pendent.** Connectar la notificació de PV realment perduts al combat de producció quan disposi del catàleg de condicions; actualment ConditionEngine només tenia consumidors de demo. No s'han modificat GameSession/Application, reservats per una altra sessió. · Passar la prova a CI abans de tancar formalment C-20 (regla 6 del pla).

#### C-21 — Un sol generador d'atzar

`1 j` · depèn de: cap

**Per què.** Hi ha `RandomEngine`/`Xoroshiro128p` en ús i `IRng`/`SeededRng` sense cap referència — i aquest segon ni tan sols enllaçaria, perquè no existeix cap `Rng.cpp`.

**Fitxers.** `include/RPG/Rng.h`, `include/RPG/RandomEngine.h`

**Fet quan.** Una sola interfície d'atzar al motor. La que no es fa servir surt del repositori.

**Ho prova.** El projecte compila i `ctest` passa sencer amb la interfície retirada.

#### C-04 — Un enemic invocat per narrativa no trenca el guardat

`2 j` · depèn de: C-03

**Per què.** `resolveBattles` materialitza l'enemic sense assignar `spawnIndex` (queda a −1). Si fuges d'una emboscada i guardes, la partida ja no es torna a carregar mai.

**Fitxers.** `src/Game/GameSession.cpp`, `src/Game/SaveGame.cpp`, `tests/Game/SaveGameTests.cpp`

**Fet quan.** Els enemics que no venen del nivell reben identitat pròpia i el guardat els sap distingir dels del mapa. Restaurar una partida amb un enemic invocat viu el torna a col·locar amb la seva vida.

**Ho prova.** `test_save_game`: disparar un beat amb `startBattle`, fugir, guardar, carregar; l'enemic hi és amb la vida que li quedava.

#### C-13 — Les entitats del document no perden res

`3 j` · depèn de: C-12

**Per què.** `MapEntityDocument` guarda id, objectId i posició. `ObjectSpawn` guarda a més patrulla, destí, botí, taula de botí, esdeveniment, nom visible, variant, escala, override d'efecte i un mapa de propietats. Migrar un nivell avui perdria gairebé tota la feina d'autoria.

**Fitxers.** `include/Map/EntityLayerDocument.h`, `src/Map/MapDocument.cpp`, `src/Project/ProjectSerializer.cpp`, `tests/Map/MapDocumentTests.cpp`

**Fet quan.** `MapEntityDocument` arriba a la paritat completa amb `ObjectSpawn`, i la serialització hi va i en torna sense pèrdua.

**Ho prova.** `test_map_round_trip` ampliat: nivell amb els onze camps poblats, TMX+JSON → MapDocument → JSON → MapDocument, comparació camp a camp.

#### C-17 — World és el propietari de les entitats

`6 j` · depèn de: C-16

**Per què.** `Engine::World` i `EntityHandle` estan escrits i provats, i no els inclou cap fitxer de producció. La partida guarda entitats a `GameSession` i els seus marcadors de render a l'Application, reconciliats per índex a cada frame.

**Fitxers.** `include/World/World.h`, `src/World/World.cpp`, `src/Game/GameSession.cpp`, `src/Engine/Application.cpp`

**Fet quan.** Enemics, objectes del món i els seus marcadors de render viuen a `World` amb handles estables. `syncWorldMarkers` desapareix: no hi ha res a reconciliar.

**Ho prova.** `test_world` ampliat amb el cicle de vida real (crear, moure, derrotar, guardar, restaurar); `test_save_game` segueix verd.

#### C-22 — Retirada de l'API morta

`2 j` · depèn de: C-21

**Per què.** Mitja `DicePoolEngine` està marcada «EN DESÚS des d'Ed. 3» i encara exportada; set mètodes de `TierRules` estan declarats i mai definits; `IQteProvider` no té cap implementació, de manera que el pas 2 de `SkillExecutor` és inabastable; els overrides de `RuntimeTileMap` no es criden mai i duplicen tota la memòria de tiles.

**Fitxers.** `include/RPG/DicePoolEngine.h`, `include/RPG/TierRules.h`, `include/RPG/EnhanceHooks.h`, `include/Runtime/RuntimeTileMap.h`

**Fet quan.** El que no es fa servir surt. El que s'ha de fer servir es defineix. Cap declaració del motor es queda sense cos.

**Ho prova.** Un script de CI comprova que cada mètode declarat als headers de `include/RPG/` té definició; el build actual el passaria amb set errors.

**Seguiment · 2026-09-03 · qwen + Codex: Nucli i API de dificultat retirats; RuntimeTileMap i CI pendents.**

- TierRules: retirats els 7 accessors declarats i mai definits (healthCap, healthBonus, allowsMulticlass, canEquipRarity, maxSummons, maxFeats, maxDivineCards); ningú els cridava, per això l'enllaçador no s'havia queixat mai.
- EnhanceHooks: retirats IQteProvider i QteResult (cap implementació; el pas de timing de SkillExecutor era inabastable), el camp ExecutionContext::qte i l'override_target_defense que Ed. 3 ja ignorava.
- SkillExecutor: eliminat el pas QTE mort i renumerats els passos; Skill.cpp deixa d'assignar nullptr al camp retirat.
- tools/comprobar_declaraciones_rpg.py: xarxa estàtica que comprova que cada mètode declarat a include/RPG/ té definició; registrat a CTest com rpg_declaration_check (cmake/DeclarationCheck.cmake), amb un hunk propi a CMakeLists sense canvis d'altres sessions.
- Codex (continuació): retirats resolve_against_cd, resolve_against_defense, defense_to_cd, CD_MAX i CheckOutcome; eliminades les referències dels consumidors i dels comentaris de codi.
- Migrades les cinc demos pendents: dice_engine i nd6_distribution usen enfrentades; narrativa_nd6, prologo i senal_ceniza resolen els requests amb GameSession real mitjançant una fixture sense mapa ni GL.
- Conservades la ruta SUCCESS del pròleg i les quatre rutes de La Senyal de la Cendra, amb guions explícits del jugador i del rival i comprovació del nombre de daus consumits.
- Coberts els quatre graus narratius, la importació de cd legacy com a pool rival, la prioritat de rivalDice explícit, els pools buits i els casos límit de pifia i tot-sisos. Els camps antics de les aventures continuen sent llegibles pel codi de partida.
- Actualitzats només els tres enllaços necessaris a CMake. No s’han modificat GameSession/Application ni el bloc RuntimeTileMap pendent d’integració.

**Verificat.** Abans de la retirada, el script reportava exactament els 7 errors que prediu el pla; després, 0 sobre 93 declaracions en 15 capçaleres. · Build completa de tots els targets en macos-debug sense errors (juego i level_editor inclosos). · Suite gl-free 57/57 en verd, inclòs rpg_declaration_check i els tests de la sessió concurrent. · Continuació Codex: en executar el camí real amb els guions antics, prologo i senal_ceniza fallen; després de guionitzar també els daus rivals, les cinc demos passen. · Build completa local de tots els targets, inclosos juego i level_editor: correcta. Suite GL-free: 57/57 proves en verd (macOS Debug). · Comprovador de declaracions RPG: 0 errors sobre 93 declaracions; cerca a include/src/examples/tests: cap referència a la API de dificultat retirada. · Huella SHA-256 dels quatre fitxers reservats GameSession/Application idèntica abans i després.

**Pendent.** Els overrides de RuntimeTileMap (setTileOverride/clearTileOverrides) no els crida cap codi de producció, però viuen al bloc orfe sense commitear del 28/08 amb el seu test; la retirada correspon a qui consolidi aquest bloc (regla 2 del protocol). · Passar la comprovació a CI abans de tancar formalment el paquet (regla 6 del pla).

#### C-23 — El post-processat arriba al joc

`4 j` · depèn de: C-16

**Per què.** `Framebuffer`, `ColorLUT`, `FogOfWar` i `DynamicLightMap` funcionen i només els toquen els seus demos. La boira de guerra, en particular, és una mecànica de joc que no arriba al joc.

**Fitxers.** `src/Engine/Application.cpp`, `src/Render/FogOfWar.cpp`, `src/Render/DynamicLightMap.cpp`

**Fet quan.** La boira de guerra i la llum dinàmica es poden activar per nivell des del document de mapa. El que no s'activi no costa cap crida GL.

**Ho prova.** `demo_fog` i `demo_dynamic_lights` segueixen passant; un nivell d'exemple amb boira activada es juga a `juego --frames 60`.

#### C-14 — L'editor treballa sobre MapDocument

`6 j` · depèn de: C-12, C-13

**Per què.** `EditorState` té el seu propi model de capes planes i exporta TMX de text que `TileMap` reparseja. És el segon model de mapa viu i el que bloqueja DOC-03.

**Fitxers.** `include/Editor/EditorState.h`, `src/Editor/EditorState.cpp`, `editor/src/level_editor.cpp`, `tests/Editor/EditorStateTests.cpp`

**Fet quan.** `EditorState` és una vista d'edició sobre un `MapDocument`. L'exportació TMX passa a ser un exportador d'interoperabilitat, no el format de treball. El pont per cadena de text desapareix.

**Ho prova.** `test_editor_state` passa sense canvis de comportament observable, i un test nou comprova que desar i reobrir conserva noms de capa, visibilitat i ordre — cosa que avui es perd.

#### C-18 — HandlePool sosté World

`2 j` · depèn de: C-17

**Per què.** `HandlePool&lt;T&gt;` és el mecanisme genèric d'handles generacionals del motor i no l'instancia ningú. A més té un error al constructor per moviment.

**Fitxers.** `include/Core/Memory/HandlePool.h`, `src/World/World.cpp`, `tests/Core/HandlePoolTests.cpp`

**Fet quan.** `World` guarda les seves entitats en un `HandlePool`. El bug de moviment queda arreglat i cobert.

**Ho prova.** `test_handle_pool` amb un cas de moviment; `test_world` segueix verd sobre la implementació nova.

#### C-24 — Xarxa de seguretat al camí que es juga

`5 j` · depèn de: C-03, C-16, C-17

**Per què.** Els mòduls acabats de refactoritzar tenen tests i els que sostenen la partida no en tenen cap: zero per a Render, zero per al model de dades RPG, zero per a les 1.682 línies d'Application. Els dos bugs més greus del joc han sobreviscut precisament per aquí.

**Fitxers.** `tests/Engine/ApplicationTests.cpp`, `tests/Game/GameSessionTests.cpp`, `tests/Render/BitmapFontTests.cpp`

**Fet quan.** La lògica GL-free d'Application (`refreshHud`, `loadLevel`, el guardat, el director de campanya), la màquina d'estats de `GameMode`, l'economia de negocis i la moral tenen tests unitaris.

**Ho prova.** Cobertura mesurada: cap fitxer de `src/Game/` ni la part GL-free d'`src/Engine/` per sota del 70 % de línies.

#### C-15 — Desar deixa el projecte coherent

`2 j` · depèn de: C-11, C-14

**Per què.** Desar un nivell escriu el fitxer i prou: ningú l'afegeix al manifest, de manera que el projecte segueix declarant zero nivells. I sortir de l'editor no avisa de res: tot el que no s'hagi desat es perd sense un missatge.

**Fitxers.** `editor/src/level_editor.cpp`, `src/Project/ProjectIndex.cpp`, `src/Editor/EditorState.cpp`

**Fet quan.** Desar un nivell l'inscriu al manifest del projecte. L'editor porta indicador de document brut i demana confirmació abans de sortir amb canvis pendents.

**Ho prova.** `test_project`: crear projecte, desar-hi dos nivells, rellegir el manifest i trobar-hi els dos.

### Fase D — Dades reals

#### D-01 — Un carregador que no troba res, falla

`3 j` · depèn de: cap

**Per què.** `Catalog&lt;T&gt;::loadFromString` embolcalla cada entrada en un `try/catch(...)` buit i compta com a bona qualsevol que doni un id. Un carregador trencat és indistingible d'un que funciona: aquesta és l'arrel de tot el problema, no un descuit puntual.

**Fitxers.** `include/RPG/Catalogs/RpgCatalogs.h`, `tests/RPG/CatalogContractTests.cpp`

**Fet quan.** Cada `CatalogLoader` declara els camps que espera. Si una entrada no en porta cap, la càrrega és un error amb el nom del camp i el de l'entrada. El `catch(...)` mut desapareix.

**Ho prova.** `test_catalog_contract`: un JSON amb els noms de camp de la versió anterior falla amb un missatge que diu quin camp falta.

#### D-02 — Habilitats i encanteris

`4 j` · depèn de: D-01

**Per què.** Les 492 skills i els 210 encanteris es carreguen totes amb la mateixa magnitud `{0, 2, 4, 8}`, perquè `magnitudeByDegree` és null a totes les entrades i el fallback assigna base 4. I és l'únic catàleg que el joc carrega de veritat.

**Fitxers.** `include/RPG/Catalogs/RpgCatalogs.h`, `assets/catalogs/skills.json`, `tests/RPG/CatalogContentTests.cpp`

**Fet quan.** Magnitud per grau, `defenseStat`, cost, recuperació, abast i condició aplicada es llegeixen de l'entrada. Dues cartes diferents donen números diferents.

**Ho prova.** `test_catalog_content`: tres cartes concretes amb els seus valors escrits al test; qualsevol regressió del carregador les trenca.

#### D-03 — Monstres

`3 j` · depèn de: D-01

**Per què.** Els 432 monstres es carreguen amb `maxHealth = 0` i `ca = 10`: el JSON té la vida dins de `defenses{}` i `derived{}` i el carregador la busca a l'arrel.

**Fitxers.** `include/RPG/Catalogs/RpgCatalogs.h`, `assets/catalogs/monsters.json`

**Fet quan.** Vida, defenses, estadístiques, tier i habilitats es llegeixen d'on són. Cap monstre del catàleg es carrega amb vida zero.

**Ho prova.** `test_catalog_content`: recórrer els 432 i afirmar que `maxHealth &gt; 0` i que les quatre estadístiques no són totes zero.

#### D-04 — Condicions

`2 j` · depèn de: D-01

**Per què.** Les 16 condicions entren totes idèntiques: el JSON porta `stackable`/`duration`/`effects[]` i el carregador espera `stacking`/`defaultRounds`/`damagePerRound`.

**Fitxers.** `include/RPG/Catalogs/RpgCatalogs.h`, `assets/catalogs/conditions.json`

**Fet quan.** Apilament, durada i efectes per ronda es llegeixen. Els modificadors de defensa que avui es parsegen i s'ignoren, o s'apliquen o s'esborren.

**Ho prova.** `test_catalog_content`: les 16 condicions donen almenys tres combinacions diferents d'apilament i durada.

#### D-05 — Objectes, equipament i consumibles

`3 j` · depèn de: D-01

**Per què.** Tres catàlegs amb la mateixa malaltia. A més, el motor d'inventari compta dues vegades els bonificadors d'una arma a dues mans, tracta qualsevol cosa a la mà esquerra com un escut i descarta el `maxHealthBonus`.

**Fitxers.** `include/RPG/Catalogs/RpgCatalogs.h`, `src/RPG/InventoryEngine.cpp`, `tests/RPG/InventoryEngineTests.cpp`

**Fet quan.** Els tres catàlegs llegeixen els seus camps i `InventoryEngine` aplica cada bonificador una sola vegada, distingint escut de qualsevol altre objecte a la mà esquerra.

**Ho prova.** `test_inventory_engine`: equipar una arma a dues mans dóna el mateix bonus que una d'una mà del mateix valor, no el doble.

#### D-06 — Classes, races i trasfons

`3 j` · depèn de: D-01

**Per què.** Es carreguen com a etiquetes. A més, les set eleccions narratives de la creació de personatge es validen una per una i després no se'n copia cap: tot el pas narratiu és decoratiu.

**Fitxers.** `include/RPG/Catalogs/RpgCatalogs.h`, `src/RPG/CharacterCreation.cpp`, `include/RPG/CharacterSheet.h`

**Fet quan.** Bonificadors, passives i trets de classe, raça i trasfons arriben a la fitxa. Vincle, por, defecte, meta, ideal, personalitat i virtut es guarden.

**Ho prova.** `test_character_creation`: dos personatges de races diferents tenen estadístiques diferents, i les set eleccions es llegeixen de la fitxa.

#### D-07 — Els catàlegs restants

`5 j` · depèn de: D-01

**Per què.** Passives, dots, deïtats, invocacions, muntures, paranys, taules de botí, faccions i la resta. Les 66 passives, per exemple, cauen totes a `ALWAYS_ACTIVE` amb bonificadors a zero perquè el `trigger` del JSON és prosa castellana i el carregador espera identificadors.

**Fitxers.** `include/RPG/Catalogs/RpgCatalogs.h`, `assets/catalogs/`

**Fet quan.** Els 21 catàlegs llegeixen tots els seus camps. Cap entrada del repositori es carrega amb tots els valors per defecte.

**Ho prova.** `test_catalog_content` recorre els 21 catàlegs i afirma que cap té totes les entrades idèntiques en els camps mecànics.

#### D-08 — Els enemics del joc són els del catàleg

`3 j` · depèn de: D-03, C-17

**Per què.** `GameSession` construeix cada `EnemyBrain` només amb `maxHealth` i mai crida `populate_from_monster_catalog`: tot el motor Nd6 lluita amb estadístiques a zero i la reserva mínima de daus.

**Fitxers.** `src/Game/GameSession.cpp`, `include/Game/EnemyBrain.h`, `tests/Game/GameSessionTests.cpp`

**Fet quan.** Un enemic del món es construeix des de la seva `MonsterDefinition`: estadístiques, defenses, tier i habilitats. El combat Nd6 té números diferents per a monstres diferents.

**Ho prova.** `test_game_session`: un llop i un drac del catàleg donen reserves de daus i defenses diferents.

#### D-09 — Les cartes apliquen les seves condicions

`2 j` · depèn de: D-04, C-20

**Per què.** `SkillDefinition` porta `apply_condition_id`, `apply_condition_min_degree` i `condition_duration_rounds`, i `SkillExecutor::execute()` no els toca ni els reporta: el cridant no té cap senyal que hi hagi una condició pendent.

**Fitxers.** `src/RPG/SkillExecutor.cpp`, `include/RPG/EnhanceHooks.h`, `src/Game/BattleState.cpp`

**Fet quan.** Una carta que enverina, enverina. El resultat de l'execució diu quina condició cal aplicar i amb quina durada, i el combat l'aplica.

**Ho prova.** `test_skill_executor`: carta amb condició a partir de grau ÈXIT; amb PARCIAL no s'aplica, amb ÈXIT sí.

#### D-10 — CI valida els assets reals

`2 j` · depèn de: D-01, D-07

**Per què.** Res impedeix avui que un canvi al JSON o al carregador els torni a desaparellar en silenci. És exactament com hi hem arribat.

**Fitxers.** `.github/workflows/ctest.yml`, `tools/validar_catalogos.py`, `tests/RPG/CatalogContentTests.cpp`

**Fet quan.** CI carrega els catàlegs reals d'`assets/` a cada commit i falla si una entrada perd camps mecànics respecte de la línia base registrada.

**Ho prova.** El job de CI falla si es torna a introduir a mà el desaparellament de `saveAttribute`/`defenseStat`.

### Fase E — Esdeveniments editables

#### E-01 — L'esdeveniment és part del document de mapa

`3 j` · depèn de: C-13

**Per què.** Avui `ObjectSpawn::eventId` s'edita a l'inspector i no el llegeix ningú. Un esdeveniment no existeix com a dada: la narrativa viu en fitxers d'aventura a part, per NPC i per nivell.

**Fitxers.** `include/Map/EventLayerDocument.h`, `include/Map/MapDocument.h`, `src/Project/ProjectSerializer.cpp`

**Fet quan.** Un esdeveniment és una entitat del mapa amb pàgines, cada pàgina amb condicions, disparador i llista de comandes. Es desa amb el mapa i es valida amb ell.

**Ho prova.** `test_map_document`: mapa amb dos esdeveniments de tres pàgines; round-trip complet sense pèrdua.

#### E-02 — Interruptors locals i variables

`3 j` · depèn de: E-01

**Per què.** `NarrativeState` té flags globals i prou. Sense interruptors locals, cada cofre d'un mapa necessita una flag global amb nom únic, que és com es fa impracticable escriure contingut.

**Fitxers.** `include/RPG/NarrativeEngine.h`, `src/RPG/NarrativeEngine.cpp`, `src/Game/SaveGame.cpp`

**Fet quan.** Cada esdeveniment té els seus interruptors locals A–D i el joc té variables numèriques amb nom. Tot va al guardat.

**Ho prova.** `test_narrative`: dos cofres amb el mateix interrupter local en mapes diferents no s'interfereixen; el guardat els conserva.

#### E-05 — Editor de pàgines d'esdeveniment

`8 j` · depèn de: E-01, C-14

**Per què.** És la primera meitat del que fa que això sigui un creador de jocs i no un motor: poder crear un esdeveniment sobre una casella i donar-li pàgines amb condicions.

**Fitxers.** `editor/src/event_editor.cpp`, `include/Editor/EventEditorState.h`, `tests/Editor/EventEditorTests.cpp`

**Fet quan.** Des de l'editor es crea un esdeveniment sobre una casella, se li afegeixen pàgines, i cada pàgina té disparador, condicions i aparença. Tot GL-free sota `EventEditorState`.

**Ho prova.** `test_event_editor`: crear, duplicar, reordenar i esborrar pàgines; desfer restaura l'estat anterior.

#### E-03 — Planificador, esperes i processos paral·lels

`4 j` · depèn de: E-02

**Per què.** Un beat s'executa sencer i de cop. Sense esperes no hi ha ritme narratiu: ni una porta que s'obre a poc a poc, ni un NPC que camina mentre parles.

**Fitxers.** `include/RPG/EventInterpreter.h`, `src/RPG/EventInterpreter.cpp`

**Fet quan.** L'intèrpret executa comandes al llarg de diversos frames, amb esperes per temps i per condició, i admet processos paral·lels que no bloquegen el jugador.

**Ho prova.** `test_event_interpreter`: seqüència amb dues esperes; el nombre de ticks fins al final és determinista.

#### E-04 — El joc de comandes

`7 j` · depèn de: E-03, D-09

**Per què.** Les comandes que el contingut necessita per no dependre de C++: diàleg amb eleccions, canvis d'estat i inventari, moviment, càmera, teletransport, branques, bucles i esdeveniments comuns.

**Fitxers.** `src/RPG/EventCommands.cpp`, `include/RPG/EventCommand.h`, `src/Game/GameSession.cpp`

**Fet quan.** Les comandes de les cinc famílies estan implementades i documentades a `FORMATO_AVENTURAS.md`, amb el seu efecte sobre la sessió.

**Ho prova.** `test_event_commands`: una comanda de cada família, amb el seu efecte observable sobre una `GameSession` de prova.

#### E-06 — Editor vertical de comandes

`9 j` · depèn de: E-04, E-05

**Per què.** La segona meitat: la llista de comandes d'una pàgina, editable, amb inserció, sagnat de les branques i moviment.

**Fitxers.** `editor/src/event_editor.cpp`, `include/Editor/CommandListState.h`

**Fet quan.** La llista de comandes s'edita amb teclat: inserir, esborrar, moure, entrar en una branca. Les comandes amb paràmetres obren el seu formulari.

**Ho prova.** `test_command_list`: construir una seqüència amb branca i bucle des de l'API de l'editor i executar-la a l'intèrpret.

#### E-07 — Validació estàtica d'aventures

`4 j` · depèn de: E-04

**Per què.** `EditorValidation::checkRpg` ja resol referències de skills, sprites, enemics i transicions, i l'editor no la crida mai. Ampliar-la a esdeveniments és continuació, no obra nova.

**Fitxers.** `include/RPG/AdventureValidation.h`, `src/RPG/AdventureValidation.cpp`, `editor/src/level_editor.cpp`

**Fet quan.** Beats i esdeveniments es validen: flags que ningú encén, beats inabastables, ids duplicats, referències a NPC, enemics o skills inexistents, temporitzadors que no arrenca ningú. L'editor la crida abans de desar i abans del playtest.

**Ho prova.** `test_adventure_validation`: una aventura amb sis defectes coneguts dóna sis diagnòstics amb la seva ubicació.

#### E-08 — Depuració pas a pas

`5 j` · depèn de: E-03, E-06

**Per què.** Quan un esdeveniment no fa el que l'autor esperava, avui l'única eina és llegir el JSON i endevinar.

**Fitxers.** `editor/src/event_editor.cpp`, `src/RPG/EventInterpreter.cpp`

**Fet quan.** Es pot executar un esdeveniment comanda a comanda, veure l'estat de flags i variables a cada pas i posar punts d'aturada.

**Ho prova.** `test_event_interpreter`: mode pas a pas; l'estat després de N passos és igual que executar-ne N de cop.

#### E-09 — La missió de la clau, la porta i el cofre

`3 j` · depèn de: E-07, E-08

**Per què.** La prova d'acceptació de tota la fase: la missió canònica de qualsevol creador de jocs, escrita sense tocar codi.

**Fitxers.** `assets/adventures/mision_llave.json`, `assets/levels/`

**Fet quan.** La missió s'escriu sencera des de l'editor: cofre amb clau, porta tancada que la demana, NPC que en parla, i final. Validada i jugable.

**Ho prova.** `demo_mision_llave` a CTest: recorre la missió sencera de manera determinista i comprova l'estat final.

### Fase F — Playtest des de l'editor

#### F-01 — Playtest des de l'editor

`4 j` · depèn de: C-16, E-09

**Per què.** Avui la tecla P llança `./juego` com a procés a part, amb el que hi hagi desat al disc. No és provar el que estàs editant: és provar l'última vegada que vas prémer G.

**Fitxers.** `editor/src/level_editor.cpp`, `src/Engine/Application.cpp`

**Fet quan.** El playtest arrenca dins del mateix procés, amb el document en memòria, sense desar. Sortir-ne torna exactament a l'estat d'edició.

**Ho prova.** `test_playtest`: entrar i sortir del playtest deixa `EditorState` byte a byte igual.

#### F-02 — Provar des de la casella seleccionada

`2 j` · depèn de: F-01

**Per què.** Provar una missió del final del mapa obligant-te a caminar-hi cada vegada és el que fa que els autors deixin de provar.

**Fitxers.** `editor/src/level_editor.cpp`

**Fet quan.** El playtest arrenca amb el jugador a la casella marcada, amb l'estat de partida que l'autor esculli (flags i objectes preparats).

**Ho prova.** `test_playtest`: arrencar a tres caselles diferents amb tres jocs de flags.

#### F-03 — Recàrrega en calent

`4 j` · depèn de: F-01

**Per què.** El cicle editar→provar→corregir només és útil si és de segons.

**Fitxers.** `src/Engine/Application.cpp`, `include/Map/MapDocument.h`

**Fet quan.** Modificar el mapa o un esdeveniment mentre el playtest corre l'actualitza sense reiniciar la partida. El que no es pot recarregar en calent ho diu clar en comptes de fallar a mitges.

**Ho prova.** `test_hot_reload`: canviar un tile i un esdeveniment amb la partida viva; l'estat del jugador es conserva.

#### F-04 — Consola i inspector d'estat

`3 j` · depèn de: F-01

**Per què.** Sense veure flags, variables i inventari en viu, depurar contingut és endevinar.

**Fitxers.** `editor/src/console.cpp`, `include/Editor/ConsoleState.h`

**Fet quan.** Durant el playtest es poden veure i modificar flags, variables, or i inventari, i llegir el registre d'esdeveniments disparats.

**Ho prova.** `test_console`: encendre una flag des de la consola dispara el beat que hi depèn.

### Fase G — Tancament del corte B

#### G-02 — Les eines d'edició que falten

`3 j` · depèn de: C-14

**Per què.** L'editor té pinzell, esborrar, omplir, col·locar i enllaçar. Li falten comptagotes, rectangle i línia — les tres que qualsevol autor busca la primera hora.

**Fitxers.** `include/Editor/EditorState.h`, `src/Editor/EditorState.cpp`, `editor/src/level_editor.cpp`

**Fet quan.** Comptagotes, rectangle i línia, cadascuna com una sola operació d'historial.

**Ho prova.** `test_editor_state`: cada eina nova, amb el seu desfer.

#### G-01 — Autosave i recuperació

`2 j` · depèn de: C-15

**Per què.** Perdre feina d'autoria per un tancament inesperat és el defecte que fa que una eina no es torni a obrir.

**Fitxers.** `src/Editor/EditorState.cpp`, `editor/src/level_editor.cpp`

**Fet quan.** El document es desa sol cada N canvis en un fitxer de recuperació. En obrir, si n'hi ha un de més nou que el desat, l'editor ho ofereix.

**Ho prova.** `test_autosave`: simular una caiguda amb canvis pendents i recuperar-los.

#### G-03 — Historial per comandes

`5 j` · depèn de: C-14, G-02

**Per què.** L'historial copia el mapa sencer a cada pinzellada, amb un límit de 256 instantànies i un descart O(n) d'instantànies completes. És el paquet ja marcat REPLANTEAR al Gantt: amb mapes grans, l'editor no s'aguanta.

**Fitxers.** `include/Editor/EditorState.h`, `src/Editor/EditorState.cpp`

**Fet quan.** L'historial guarda comandes reversibles, no fotografies. Un mapa de 200×200 amb quatre capes admet mil operacions sense creixement notable de memòria.

**Ho prova.** `test_editor_state`: mil operacions barrejades; desfer-les totes retorna al document inicial i la memòria es manté sota un límit.

#### G-04 — Joc d'exemple de trenta minuts

`5 j` · depèn de: E-09, F-03

**Per què.** La prova que el corte B és real: un joc petit fet només amb l'eina.

**Fitxers.** `assets/proyectos/ejemplo/`, `assets/levels/`, `assets/adventures/`

**Fet quan.** Un joc de trenta minuts amb tres mapes, sis esdeveniments, combat i final, construït sense tocar C++.

**Ho prova.** `demo_juego_ejemplo` a CTest el recorre sencer de manera determinista.

#### G-05 — Tutorial de l'editor

`4 j` · depèn de: G-04

**Per què.** Una eina que necessita que hi siguis tu al costat no és una eina que puguis donar a ningú.

**Fitxers.** `docs/tutorial/`, `editor/src/level_editor.cpp`

**Fet quan.** Un tutorial guiat dins de l'editor que porta de zero a un mapa jugable amb un esdeveniment.

**Ho prova.** Prova amb una persona que no hagi vist mai el projecte: acaba el tutorial sense preguntar res.

#### G-06 — Empaquetat i exportació

`5 j` · depèn de: G-04

**Per què.** Un joc que només es pot jugar des del directori de build no s'ha acabat.

**Fitxers.** `tools/exportar.py`, `CMakeLists.txt`

**Fet quan.** Un projecte s'exporta a un executable autocontingut per a Linux i Windows, amb només els assets que fa servir.

**Ho prova.** CI construeix l'export a les dues plataformes i l'executa amb `--frames 60`.

#### G-07 — Congelació dels esquemes

`2 j` · depèn de: G-04, G-06

**Per què.** A partir d'aquí, qualsevol canvi de format ha de saber llegir el que ja hi ha escrit.

**Fitxers.** `FORMATO_PROYECTOS.md`, `FORMATO_NIVELES.md`, `FORMATO_AVENTURAS.md`

**Fet quan.** Els esquemes de projecte, mapa, aventura i guardat es congelen a la versió 1.0-B, documentats, amb ruta de migració per a cada canvi futur.

**Ho prova.** `test_schema_freeze`: fixtures de la versió 1.0-B es carreguen sense avisos amb el codi del dia.

## 5. Fora d'abast, per escrit

El corte B es defineix tant pel que inclou com pel que no. Això queda per a 1.1, i està aquí perquè ningú (jo inclòs) ho coli sense adonar-se'n.

| Queda fora | Paquets del Gantt vell | Motiu |
| --- | --- | --- |
| Autotiles i terrenys intel·ligents | MAP-03 | Molta feina d'editor per a un resultat que es pot pintar a mà en mapes de la mida que fem ara. |
| Sistema de plugins (runtime i editor) | TOOL-06..08 | No té sentit obrir una API d'extensió abans que l'API interna estigui estable. Depèn de la fase C. |
| Àudio, música i ambient | GDB-11 | Cap peça del motor el toca avui; és una vertical sencera i no bloqueja el cicle d'autor. |
| Base de dades completa i editor mestre-detall | GDB-01..12 | La fase D deixa els catàlegs llegibles; editar-los des de l'eina és 1.1. |
| Exportació a macOS i empaquetat de botiga | TOOL-10 | Linux i Windows cobreixen la prova del corte B. |
| Combat tàctic avançat i IA de grup | — | El combat per torns actual és suficient per validar el cicle d'autor. |

## 6. Regles de treball

- **Un paquet no es comença sense les seves dependències tancades.** L'ordre topològic del punt 4 ja el respecta.
- **Un paquet es tanca quan la seva prova passa a CI**, no quan el codi compila a la màquina de qui l'ha escrit.
- **Cap paquet nou entra al pla sense criteri d'acceptació i prova.** Si no es poden escriure, el que falta és entendre el problema, no programar-lo.
- **Si un paquet supera el doble de la seva estimació, es para i es replanteja**, en comptes d'anar-hi afegint jornades.
- **Les fases no s'encavalquen.** Cada fase s'obre quan la anterior ha tancat: és el que vol dir «consolidació abans de res més». Al Gantt això són les portes de fase.
- **La fase C no admet funcions noves.** Qualsevol idea que aparegui durant la consolidació va a la llista de 1.1.
