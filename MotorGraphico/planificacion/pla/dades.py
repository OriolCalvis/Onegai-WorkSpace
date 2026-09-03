# -*- coding: utf-8 -*-
"""Pla definitiu de MotorGraphico — Corte B.

Font única: d'aquí surten el markdown del repositori, el Gantt i la pàgina.
Cada paquet porta el seu criteri d'acceptació i la prova que el tanca, de
manera que "fet" no depengui de l'opinió de ningú.
"""

DECISIONS = [
    ("Objectiu de versió", "Corte B — cicle d'autor complet",
     "Que algú que no sigui l'Oriol pugui fer un joc amb l'editor: mapes, "
     "esdeveniments i playtest sense tocar codi. Autotiles, plugins, exportació "
     "a més plataformes i àudio queden fora, a 1.1."),
    ("Catàlegs RPG", "Arreglar-los tots els 21",
     "Cada carregador ha de llegir de veritat el seu JSON, amb una prova que "
     "comprovi el contingut d'un camp i no només que hi hagi entrades."),
    ("Codi orfe", "Consolidació prèvia, abans de res més",
     "MapDocument, SceneManager, World, HandlePool, ProjectSerializer i el "
     "post-processat es connecten primer. Quan la fase C tanqui, no hi ha d'haver "
     "dues implementacions vives del mateix concepte."),
]

PHASES = [
    ("C", "Consolidació",
     "Una sola veritat per concepte, i cap bug que el jugador noti. És la fase "
     "que no afegeix cap funció nova i sense la qual totes les altres es "
     "construeixen damunt de dos fonaments alhora.",
     "Cap concepte del motor té dues implementacions vives, els vuit bugs "
     "crítics estan tancats amb prova de regressió, i el camí que es juga de "
     "veritat té tests."),
    ("D", "Dades reals",
     "Que el que diu el JSON sigui el que arriba al joc. Avui 19 dels 21 "
     "carregadors llegeixen cinc camps i s'inventen la resta en silenci.",
     "Els 21 catàlegs carreguen tots els seus camps, un carregador que no "
     "troba el que espera falla en comptes de callar, i CI valida els assets "
     "reals a cada commit."),
    ("E", "Esdeveniments editables",
     "El cor del corte B: que una missió es pugui escriure des de l'editor, "
     "sense tocar C++ ni JSON a mà.",
     "La missió de la clau, la porta i el cofre s'escriu sencera des de "
     "l'editor visual, es valida abans d'executar-se i es pot depurar pas a pas."),
    ("F", "Playtest des de l'editor",
     "El bucle d'autoria: editar, provar, corregir sense sortir de l'eina.",
     "Des d'un mapa obert, una tecla llança la partida en aquell punt, i tornar "
     "a l'editor conserva el que s'estava editant."),
    ("G", "Tancament del corte B",
     "El que converteix una eina que funciona en una eina que es pot donar a "
     "algú altre.",
     "Un tercer instal·la el paquet, segueix el tutorial i publica un joc "
     "jugable sense ajuda."),
]

# id, títol, fase, jornades, dependències, per què, fitxers, criteri, prova
P = [
    # ---------------- FASE C: bugs que el jugador nota ----------------
    ("C-01", "La curació cura qui toca", "C", 2, [],
     "Avui l'ordre «Cura» gasta 4 PM del jugador i cura ~20 PV al monstre. "
     "<code>resolveAllyAction</code> tria sempre l'objectiu dins de <code>m_enemies</code> "
     "i ningú no llegeix <code>Skill::target</code>.",
     ["src/Game/BattleState.cpp", "include/Game/Skill.h", "tests/Game/BattleStateTests.cpp"],
     "Una habilitat amb <code>target = Ally</code> aplica el seu efecte sobre el jugador; "
     "una amb <code>target = Enemy</code>, sobre l'enemic seleccionat. El PM es gasta "
     "una sola vegada i només si l'efecte s'ha pogut aplicar.",
     "<code>test_battle_state</code>: curar amb vida a 30/60 deixa el jugador a 50 i "
     "l'enemic intacte; el mateix cas amb el codi antic falla."),

    ("C-02", "El minimapa deixa de llegir memòria alliberada", "C", 1, [],
     "<code>HudMinimap</code> guarda el punter a la <code>GameSession</code> del primer "
     "nivell i no té setter. Des de la primera porta creuada llegeix un objecte "
     "destruït a cada frame: avui no peta per sort.",
     ["include/Render/HudMinimap.h", "src/Render/HudMinimap.cpp", "src/Engine/Application.cpp"],
     "El minimapa rep la sessió per <code>setSession()</code> i <code>loadLevel</code> "
     "l'hi torna a passar. Amb sessió nul·la es dibuixa buit en comptes de llegir.",
     "<code>test_hud_minimap</code>: construir, canviar de sessió i destruir la primera; "
     "el widget segueix responent i no toca la vella. Verificat també amb "
     "<code>-fsanitize=address</code> sobre <code>demo_game_session</code>."),

    ("C-03", "L'estat del jugador sobreviu el canvi de nivell", "C", 2, [],
     "<code>loadLevel</code> rescata només l'or. Entrar a una posada et reinicia la "
     "reputació a 0, et deixa sense negocis i esborra el comptador de rendes.",
     ["src/Engine/Application.cpp", "include/Game/GameSession.h", "src/Game/SaveGame.cpp"],
     "Moral, negocis, passos des de l'últim cobrament i or es transfereixen "
     "sencers entre sessions. El traspàs és una sola funció, no una llista de "
     "camps escampada per <code>loadLevel</code>.",
     "<code>test_game_session</code>: sessió amb moral −30, dos negocis i 90 d'or; "
     "reconstruir-la sobre un altre nivell i comprovar els quatre valors."),

    ("C-04", "Un enemic invocat per narrativa no trenca el guardat", "C", 2, ["C-03"],
     "<code>resolveBattles</code> materialitza l'enemic sense assignar <code>spawnIndex</code> "
     "(queda a −1). Si fuges d'una emboscada i guardes, la partida ja no es "
     "torna a carregar mai.",
     ["src/Game/GameSession.cpp", "src/Game/SaveGame.cpp", "tests/Game/SaveGameTests.cpp"],
     "Els enemics que no venen del nivell reben identitat pròpia i el guardat els "
     "sap distingir dels del mapa. Restaurar una partida amb un enemic invocat "
     "viu el torna a col·locar amb la seva vida.",
     "<code>test_save_game</code>: disparar un beat amb <code>startBattle</code>, fugir, "
     "guardar, carregar; l'enemic hi és amb la vida que li quedava."),

    ("C-05", "Tancament ordenat dels recursos GL", "C", 1, [],
     "<code>shutdown()</code> no buida els dos ResourceManager: es destrueixen al "
     "destructor d'Application, quan <code>glfwTerminate()</code> ja ha passat i "
     "<code>glDeleteTextures</code> no té context on anar.",
     ["src/Engine/Application.cpp", "include/Engine/Application.h"],
     "Textures i shaders s'alliberen mentre el context és viu. Sortir del joc no "
     "deixa cap crida GL després de <code>glfwTerminate()</code>.",
     "Execució de <code>juego</code> amb <code>--frames 3</code> sota "
     "<code>ASAN_OPTIONS=detect_leaks=1</code>, sense fuites de textura."),

    ("C-06", "L'ajust de text no es penja", "C", 1, [],
     "<code>wrapText</code> amb una caixa d'un sol glifu d'amplada no consumeix res i "
     "afegeix «-» per sempre. Hi arriba qualsevol <code>HudText</code> prou estret.",
     ["src/Render/BitmapFont.cpp", "tests/Render/BitmapFontTests.cpp"],
     "Amb una amplada menor que un glifu, <code>wrapText</code> retorna el text tallat "
     "sense inventar guions i sense bucle.",
     "<code>test_bitmap_font</code>: amplades 0, 1 i 2 sobre una frase llarga; la "
     "funció retorna en temps finit i el resultat cap a la caixa."),

    ("C-07", "Les aventures autònomes disparen els seus beats", "C", 2, [],
     "Amb <code>--precuela</code>, <code>m_narrativeCampaignTransitions</code> queda a fals "
     "i <code>loadLevel</code> no crida mai <code>setNarrative</code>: l'aventura es carrega "
     "i no passa absolutament res.",
     ["src/Engine/Application.cpp", "include/Engine/Application.h"],
     "Qualsevol aventura carregada s'enganxa a la sessió. La bandera de campanya "
     "només decideix si es fan les transicions de la cronologia, no si la "
     "narrativa existeix.",
     "<code>demo_precuela_boundington</code> promogut a CTest comprova que el primer "
     "beat es dispara; abans no ho feia."),

    ("C-08", "Un guardat antic no escriu fora de rang", "C", 1, [],
     "<code>tickTimers</code> indexa <code>avisosDados[i]</code> contra la mida de la "
     "definició, i el guardat el restaura amb la mida que tenia el dia que es va "
     "desar. Afegir un avís a una aventura converteix els guardats existents en "
     "escriptura fora de rang.",
     ["src/RPG/NarrativeEngine.cpp", "src/Game/SaveGame.cpp", "tests/Game/SaveGameTests.cpp"],
     "En restaurar, el vector d'avisos es redimensiona a la mida de la definició "
     "actual, conservant els que ja s'havien donat.",
     "<code>test_save_game</code>: guardat amb dos avisos restaurat sobre una definició "
     "de quatre; cap escriptura fora de rang sota ASan."),

    ("C-09", "Propietat de recursos i destrucció polimòrfica", "C", 1, [],
     "<code>Texture::operator=(Texture&&)</code> sobreescriu <code>m_glID</code> sense "
     "alliberar el que ja tenia, i <code>RuntimeTileMap</code> no té destructor virtual "
     "tot i que <code>TileMap</code> n'hereta i el renderer en guarda un punter a la base.",
     ["include/Core/Resources/Texture.h", "include/Runtime/RuntimeTileMap.h"],
     "L'assignació per moviment allibera el recurs de destí abans de robar el "
     "d'origen. <code>RuntimeTileMap</code> té destructor virtual.",
     "<code>test_texture</code> amb un doble GL fals: assignar sobre una textura viva "
     "n'allibera el nom exactament una vegada."),

    ("C-10", "El JSON que escrivim el podem tornar a llegir", "C", 1, [],
     "<code>JsonWriter</code> emet <code>\\u00xx</code> per als caràcters de control i "
     "<code>JsonParser</code> rebutja <code>\\u</code> explícitament: el round-trip que la "
     "capçalera diu que és la comprovació que importa està trencat.",
     ["include/Core/Json/JsonWriter.h", "src/Core/Json/JsonValue.cpp", "tests/Core/JsonTests.cpp"],
     "El parser entén <code>\\uXXXX</code> i el codifica a UTF-8. Tot el que surt de "
     "<code>JsonWriter</code> torna a entrar per <code>JsonValue::parse</code> sense pèrdua.",
     "<code>test_json</code>: round-trip d'una cadena amb caràcters de control, "
     "accents, cometes i barres."),

    # ---------------- FASE C: unificació de camins ----------------
    ("C-11", "Un sol format de projecte", "C", 4, [],
     "<code>ProjectIndex</code> llegeix claus castellanes a l'arrel i "
     "<code>ProjectSerializer</code> escriu claus angleses amb <code>settings</code> niat. "
     "Cap dels dos pot llegir el fitxer de l'altre, i tots dos es diuen canònics.",
     ["src/Project/ProjectSerializer.cpp", "src/Project/ProjectIndex.cpp",
      "FORMATO_PROYECTOS.md", "tools/migrar_proyectos.py"],
     "Un únic format v1, amb <code>schemaVersion</code>, que <code>ProjectSerializer</code> "
     "llegeix i escriu i que <code>ProjectIndex</code> consumeix. Els cinc manifests "
     "d'<code>assets/proyectos/</code> queden migrats i el format vell es rebutja amb "
     "un missatge que diu com migrar.",
     "<code>test_project_serialization</code>: carregar els cinc manifests reals "
     "migrats, desar-los i comparar bytes."),

    ("C-12", "MapDocument es pot editar", "C", 5, [],
     "El document de mapa nou és d'escriptura única: no té <code>removeLayer</code>, ni "
     "accés no-const a les dades d'una capa, ni manera de reordenar. Mentre sigui "
     "així, l'editor no s'hi pot migrar encara que es vulgui.",
     ["include/Map/MapDocument.h", "src/Map/MapDocument.cpp",
      "include/Map/MapLayer.h", "tests/Map/MapDocumentTests.cpp"],
     "API de mutació completa: pintar cel·les, afegir i treure entitats i regions, "
     "esborrar capes, reordenar-les i canviar-ne els metadades. Cada mutació manté "
     "les invariants que avui només comprova <code>validate()</code>.",
     "<code>test_map_document</code>: seqüència de 20 mutacions; el document valida "
     "després de cadascuna i el resultat és el mateix que construir-lo d'una peça."),

    ("C-13", "Les entitats del document no perden res", "C", 3, ["C-12"],
     "<code>MapEntityDocument</code> guarda id, objectId i posició. <code>ObjectSpawn</code> "
     "guarda a més patrulla, destí, botí, taula de botí, esdeveniment, nom visible, "
     "variant, escala, override d'efecte i un mapa de propietats. Migrar un nivell "
     "avui perdria gairebé tota la feina d'autoria.",
     ["include/Map/EntityLayerDocument.h", "src/Map/MapDocument.cpp",
      "src/Project/ProjectSerializer.cpp", "tests/Map/MapDocumentTests.cpp"],
     "<code>MapEntityDocument</code> arriba a la paritat completa amb "
     "<code>ObjectSpawn</code>, i la serialització hi va i en torna sense pèrdua.",
     "<code>test_map_round_trip</code> ampliat: nivell amb els onze camps poblats, "
     "TMX+JSON → MapDocument → JSON → MapDocument, comparació camp a camp."),

    ("C-14", "L'editor treballa sobre MapDocument", "C", 6, ["C-12", "C-13"],
     "<code>EditorState</code> té el seu propi model de capes planes i exporta TMX de "
     "text que <code>TileMap</code> reparseja. És el segon model de mapa viu i el que "
     "bloqueja DOC-03.",
     ["include/Editor/EditorState.h", "src/Editor/EditorState.cpp",
      "editor/src/level_editor.cpp", "tests/Editor/EditorStateTests.cpp"],
     "<code>EditorState</code> és una vista d'edició sobre un <code>MapDocument</code>. "
     "L'exportació TMX passa a ser un exportador d'interoperabilitat, no el format "
     "de treball. El pont per cadena de text desapareix.",
     "<code>test_editor_state</code> passa sense canvis de comportament observable, i "
     "un test nou comprova que desar i reobrir conserva noms de capa, visibilitat "
     "i ordre — cosa que avui es perd."),

    ("C-15", "Desar deixa el projecte coherent", "C", 2, ["C-11", "C-14"],
     "Desar un nivell escriu el fitxer i prou: ningú l'afegeix al manifest, de "
     "manera que el projecte segueix declarant zero nivells. I sortir de l'editor "
     "no avisa de res: tot el que no s'hagi desat es perd sense un missatge.",
     ["editor/src/level_editor.cpp", "src/Project/ProjectIndex.cpp", "src/Editor/EditorState.cpp"],
     "Desar un nivell l'inscriu al manifest del projecte. L'editor porta indicador "
     "de document brut i demana confirmació abans de sortir amb canvis pendents.",
     "<code>test_project</code>: crear projecte, desar-hi dos nivells, rellegir el "
     "manifest i trobar-hi els dos."),

    ("C-16", "L'Application delega en SceneManager", "C", 4, [],
     "<code>Scene</code> i <code>SceneManager</code> existeixen, estan provats i no els "
     "enllaça res que no sigui el seu test. Mentrestant l'Application porta les "
     "fases a mà amb banderes.",
     ["src/Engine/Application.cpp", "include/Engine/Scene/SceneManager.h",
      "tests/Engine/SceneManagerTests.cpp"],
     "El bucle principal empeny i treu escenes en comptes de mirar banderes. El "
     "canvi de nivell i el pas menú↔joc són transicions d'escena.",
     "<code>test_scene_manager</code> ampliat amb les transicions reals del joc; "
     "<code>demo_game_session</code> segueix passant."),

    ("C-17", "World és el propietari de les entitats", "C", 6, ["C-16"],
     "<code>Engine::World</code> i <code>EntityHandle</code> estan escrits i provats, i no "
     "els inclou cap fitxer de producció. La partida guarda entitats a "
     "<code>GameSession</code> i els seus marcadors de render a l'Application, "
     "reconciliats per índex a cada frame.",
     ["include/World/World.h", "src/World/World.cpp", "src/Game/GameSession.cpp",
      "src/Engine/Application.cpp"],
     "Enemics, objectes del món i els seus marcadors de render viuen a "
     "<code>World</code> amb handles estables. <code>syncWorldMarkers</code> desapareix: "
     "no hi ha res a reconciliar.",
     "<code>test_world</code> ampliat amb el cicle de vida real (crear, moure, "
     "derrotar, guardar, restaurar); <code>test_save_game</code> segueix verd."),

    ("C-18", "HandlePool sosté World", "C", 2, ["C-17"],
     "<code>HandlePool&lt;T&gt;</code> és el mecanisme genèric d'handles generacionals del "
     "motor i no l'instancia ningú. A més té un error al constructor per moviment.",
     ["include/Core/Memory/HandlePool.h", "src/World/World.cpp", "tests/Core/HandlePoolTests.cpp"],
     "<code>World</code> guarda les seves entitats en un <code>HandlePool</code>. El bug de "
     "moviment queda arreglat i cobert.",
     "<code>test_handle_pool</code> amb un cas de moviment; <code>test_world</code> "
     "segueix verd sobre la implementació nova."),

    ("C-19", "Un sol culling i un sol contracte d'actualització", "C", 2, [],
     "<code>IsometricRenderer::visibleRangeFor</code> i <code>TileMap::visibleRange</code> "
     "són còpies literals l'una de l'altra, i <code>IUpdatable</code> i "
     "<code>IHudElement::update</code> són dos contractes <code>update(float)</code> sense "
     "cap parentiu.",
     ["include/Render/IsometricRenderer.h", "include/Render/TileMap.h",
      "include/Render/IUpdatable.h", "include/Render/HudElement.h"],
     "Una sola funció de culling, amb un sol propietari. Un sol contracte "
     "d'actualització per frame, o dos noms diferents si de veritat són coses "
     "diferents.",
     "<code>test_occlusion</code>: la funció única dóna el mateix rang que les dues "
     "anteriors sobre deu càmeres i mides de mapa."),

    ("C-20", "Un sol escriptor de condicions", "C", 2, [],
     "<code>CharacterSheet::applyCondition</code> i <code>ConditionEngine</code> fan la "
     "mateixa feina amb regles diferents: la fitxa no coneix ni <code>Stacking</code>, "
     "ni <code>maxStacks</code>, ni el recàlcul de modificadors.",
     ["include/RPG/CharacterSheet.h", "src/RPG/CharacterSheet.cpp",
      "src/RPG/ConditionEngine.cpp", "tests/RPG/ConditionEngineTests.cpp"],
     "<code>ConditionEngine</code> és l'únic camí per aplicar, apilar i retirar "
     "condicions. La fitxa n'és el magatzem. A més, <code>purge_expired</code> purga de "
     "veritat i <code>breaksOnDamage</code> es dispara pel dany rebut, no per l'infligit.",
     "<code>test_condition_engine</code>: apilar fins al màxim, expirar per rondes, "
     "trencar per dany rebut; cap camí passa per la fitxa."),

    ("C-21", "Un sol generador d'atzar", "C", 1, [],
     "Hi ha <code>RandomEngine</code>/<code>Xoroshiro128p</code> en ús i "
     "<code>IRng</code>/<code>SeededRng</code> sense cap referència — i aquest segon ni tan "
     "sols enllaçaria, perquè no existeix cap <code>Rng.cpp</code>.",
     ["include/RPG/Rng.h", "include/RPG/RandomEngine.h"],
     "Una sola interfície d'atzar al motor. La que no es fa servir surt del "
     "repositori.",
     "El projecte compila i <code>ctest</code> passa sencer amb la interfície retirada."),

    ("C-22", "Retirada de l'API morta", "C", 2, ["C-21"],
     "Mitja <code>DicePoolEngine</code> està marcada «EN DESÚS des d'Ed. 3» i encara "
     "exportada; set mètodes de <code>TierRules</code> estan declarats i mai definits; "
     "<code>IQteProvider</code> no té cap implementació, de manera que el pas 2 de "
     "<code>SkillExecutor</code> és inabastable; els overrides de "
     "<code>RuntimeTileMap</code> no es criden mai i duplicen tota la memòria de tiles.",
     ["include/RPG/DicePoolEngine.h", "include/RPG/TierRules.h",
      "include/RPG/EnhanceHooks.h", "include/Runtime/RuntimeTileMap.h"],
     "El que no es fa servir surt. El que s'ha de fer servir es defineix. Cap "
     "declaració del motor es queda sense cos.",
     "Un script de CI comprova que cada mètode declarat als headers de "
     "<code>include/RPG/</code> té definició; el build actual el passaria amb set errors."),

    ("C-23", "El post-processat arriba al joc", "C", 4, ["C-16"],
     "<code>Framebuffer</code>, <code>ColorLUT</code>, <code>FogOfWar</code> i "
     "<code>DynamicLightMap</code> funcionen i només els toquen els seus demos. La boira "
     "de guerra, en particular, és una mecànica de joc que no arriba al joc.",
     ["src/Engine/Application.cpp", "src/Render/FogOfWar.cpp", "src/Render/DynamicLightMap.cpp"],
     "La boira de guerra i la llum dinàmica es poden activar per nivell des del "
     "document de mapa. El que no s'activi no costa cap crida GL.",
     "<code>demo_fog</code> i <code>demo_dynamic_lights</code> segueixen passant; un nivell "
     "d'exemple amb boira activada es juga a <code>juego --frames 60</code>."),

    ("C-24", "Xarxa de seguretat al camí que es juga", "C", 5, ["C-03", "C-16", "C-17"],
     "Els mòduls acabats de refactoritzar tenen tests i els que sostenen la "
     "partida no en tenen cap: zero per a Render, zero per al model de dades RPG, "
     "zero per a les 1.682 línies d'Application. Els dos bugs més greus del joc "
     "han sobreviscut precisament per aquí.",
     ["tests/Engine/ApplicationTests.cpp", "tests/Game/GameSessionTests.cpp",
      "tests/Render/BitmapFontTests.cpp"],
     "La lògica GL-free d'Application (<code>refreshHud</code>, <code>loadLevel</code>, el "
     "guardat, el director de campanya), la màquina d'estats de <code>GameMode</code>, "
     "l'economia de negocis i la moral tenen tests unitaris.",
     "Cobertura mesurada: cap fitxer de <code>src/Game/</code> ni la part GL-free "
     "d'<code>src/Engine/</code> per sota del 70 % de línies."),

    # ---------------- FASE D ----------------
    ("D-01", "Un carregador que no troba res, falla", "D", 3, [],
     "<code>Catalog&lt;T&gt;::loadFromString</code> embolcalla cada entrada en un "
     "<code>try/catch(...)</code> buit i compta com a bona qualsevol que doni un id. "
     "Un carregador trencat és indistingible d'un que funciona: aquesta és l'arrel "
     "de tot el problema, no un descuit puntual.",
     ["include/RPG/Catalogs/RpgCatalogs.h", "tests/RPG/CatalogContractTests.cpp"],
     "Cada <code>CatalogLoader</code> declara els camps que espera. Si una entrada no "
     "en porta cap, la càrrega és un error amb el nom del camp i el de l'entrada. "
     "El <code>catch(...)</code> mut desapareix.",
     "<code>test_catalog_contract</code>: un JSON amb els noms de camp de la versió "
     "anterior falla amb un missatge que diu quin camp falta."),

    ("D-02", "Habilitats i encanteris", "D", 4, ["D-01"],
     "Les 492 skills i els 210 encanteris es carreguen totes amb la mateixa "
     "magnitud <code>{0, 2, 4, 8}</code>, perquè <code>magnitudeByDegree</code> és null a "
     "totes les entrades i el fallback assigna base 4. I és l'únic catàleg que el "
     "joc carrega de veritat.",
     ["include/RPG/Catalogs/RpgCatalogs.h", "assets/catalogs/skills.json",
      "tests/RPG/CatalogContentTests.cpp"],
     "Magnitud per grau, <code>defenseStat</code>, cost, recuperació, abast i condició "
     "aplicada es llegeixen de l'entrada. Dues cartes diferents donen números "
     "diferents.",
     "<code>test_catalog_content</code>: tres cartes concretes amb els seus valors "
     "escrits al test; qualsevol regressió del carregador les trenca."),

    ("D-03", "Monstres", "D", 3, ["D-01"],
     "Els 432 monstres es carreguen amb <code>maxHealth = 0</code> i <code>ca = 10</code>: "
     "el JSON té la vida dins de <code>defenses{}</code> i <code>derived{}</code> i el "
     "carregador la busca a l'arrel.",
     ["include/RPG/Catalogs/RpgCatalogs.h", "assets/catalogs/monsters.json"],
     "Vida, defenses, estadístiques, tier i habilitats es llegeixen d'on són. Cap "
     "monstre del catàleg es carrega amb vida zero.",
     "<code>test_catalog_content</code>: recórrer els 432 i afirmar que "
     "<code>maxHealth &gt; 0</code> i que les quatre estadístiques no són totes zero."),

    ("D-04", "Condicions", "D", 2, ["D-01"],
     "Les 16 condicions entren totes idèntiques: el JSON porta "
     "<code>stackable</code>/<code>duration</code>/<code>effects[]</code> i el carregador "
     "espera <code>stacking</code>/<code>defaultRounds</code>/<code>damagePerRound</code>.",
     ["include/RPG/Catalogs/RpgCatalogs.h", "assets/catalogs/conditions.json"],
     "Apilament, durada i efectes per ronda es llegeixen. Els modificadors de "
     "defensa que avui es parsegen i s'ignoren, o s'apliquen o s'esborren.",
     "<code>test_catalog_content</code>: les 16 condicions donen almenys tres "
     "combinacions diferents d'apilament i durada."),

    ("D-05", "Objectes, equipament i consumibles", "D", 3, ["D-01"],
     "Tres catàlegs amb la mateixa malaltia. A més, el motor d'inventari compta "
     "dues vegades els bonificadors d'una arma a dues mans, tracta qualsevol cosa "
     "a la mà esquerra com un escut i descarta el <code>maxHealthBonus</code>.",
     ["include/RPG/Catalogs/RpgCatalogs.h", "src/RPG/InventoryEngine.cpp",
      "tests/RPG/InventoryEngineTests.cpp"],
     "Els tres catàlegs llegeixen els seus camps i <code>InventoryEngine</code> aplica "
     "cada bonificador una sola vegada, distingint escut de qualsevol altre "
     "objecte a la mà esquerra.",
     "<code>test_inventory_engine</code>: equipar una arma a dues mans dóna el mateix "
     "bonus que una d'una mà del mateix valor, no el doble."),

    ("D-06", "Classes, races i trasfons", "D", 3, ["D-01"],
     "Es carreguen com a etiquetes. A més, les set eleccions narratives de la "
     "creació de personatge es validen una per una i després no se'n copia cap: "
     "tot el pas narratiu és decoratiu.",
     ["include/RPG/Catalogs/RpgCatalogs.h", "src/RPG/CharacterCreation.cpp",
      "include/RPG/CharacterSheet.h"],
     "Bonificadors, passives i trets de classe, raça i trasfons arriben a la "
     "fitxa. Vincle, por, defecte, meta, ideal, personalitat i virtut es "
     "guarden.",
     "<code>test_character_creation</code>: dos personatges de races diferents tenen "
     "estadístiques diferents, i les set eleccions es llegeixen de la fitxa."),

    ("D-07", "Els catàlegs restants", "D", 5, ["D-01"],
     "Passives, dots, deïtats, invocacions, muntures, paranys, taules de botí, "
     "faccions i la resta. Les 66 passives, per exemple, cauen totes a "
     "<code>ALWAYS_ACTIVE</code> amb bonificadors a zero perquè el <code>trigger</code> del "
     "JSON és prosa castellana i el carregador espera identificadors.",
     ["include/RPG/Catalogs/RpgCatalogs.h", "assets/catalogs/"],
     "Els 21 catàlegs llegeixen tots els seus camps. Cap entrada del repositori "
     "es carrega amb tots els valors per defecte.",
     "<code>test_catalog_content</code> recorre els 21 catàlegs i afirma que cap té "
     "totes les entrades idèntiques en els camps mecànics."),

    ("D-08", "Els enemics del joc són els del catàleg", "D", 3, ["D-03", "C-17"],
     "<code>GameSession</code> construeix cada <code>EnemyBrain</code> només amb "
     "<code>maxHealth</code> i mai crida <code>populate_from_monster_catalog</code>: tot el "
     "motor Nd6 lluita amb estadístiques a zero i la reserva mínima de daus.",
     ["src/Game/GameSession.cpp", "include/Game/EnemyBrain.h", "tests/Game/GameSessionTests.cpp"],
     "Un enemic del món es construeix des de la seva <code>MonsterDefinition</code>: "
     "estadístiques, defenses, tier i habilitats. El combat Nd6 té números "
     "diferents per a monstres diferents.",
     "<code>test_game_session</code>: un llop i un drac del catàleg donen reserves de "
     "daus i defenses diferents."),

    ("D-09", "Les cartes apliquen les seves condicions", "D", 2, ["D-04", "C-20"],
     "<code>SkillDefinition</code> porta <code>apply_condition_id</code>, "
     "<code>apply_condition_min_degree</code> i <code>condition_duration_rounds</code>, i "
     "<code>SkillExecutor::execute()</code> no els toca ni els reporta: el cridant no té "
     "cap senyal que hi hagi una condició pendent.",
     ["src/RPG/SkillExecutor.cpp", "include/RPG/EnhanceHooks.h", "src/Game/BattleState.cpp"],
     "Una carta que enverina, enverina. El resultat de l'execució diu quina "
     "condició cal aplicar i amb quina durada, i el combat l'aplica.",
     "<code>test_skill_executor</code>: carta amb condició a partir de grau ÈXIT; "
     "amb PARCIAL no s'aplica, amb ÈXIT sí."),

    ("D-10", "CI valida els assets reals", "D", 2, ["D-01", "D-07"],
     "Res impedeix avui que un canvi al JSON o al carregador els torni a "
     "desaparellar en silenci. És exactament com hi hem arribat.",
     [".github/workflows/ctest.yml", "tools/validar_catalogos.py", "tests/RPG/CatalogContentTests.cpp"],
     "CI carrega els catàlegs reals d'<code>assets/</code> a cada commit i falla si una "
     "entrada perd camps mecànics respecte de la línia base registrada.",
     "El job de CI falla si es torna a introduir a mà el desaparellament de "
     "<code>saveAttribute</code>/<code>defenseStat</code>."),

    # ---------------- FASE E ----------------
    ("E-01", "L'esdeveniment és part del document de mapa", "E", 3, ["C-13"],
     "Avui <code>ObjectSpawn::eventId</code> s'edita a l'inspector i no el llegeix "
     "ningú. Un esdeveniment no existeix com a dada: la narrativa viu en fitxers "
     "d'aventura a part, per NPC i per nivell.",
     ["include/Map/EventLayerDocument.h", "include/Map/MapDocument.h",
      "src/Project/ProjectSerializer.cpp"],
     "Un esdeveniment és una entitat del mapa amb pàgines, cada pàgina amb "
     "condicions, disparador i llista de comandes. Es desa amb el mapa i es "
     "valida amb ell.",
     "<code>test_map_document</code>: mapa amb dos esdeveniments de tres pàgines; "
     "round-trip complet sense pèrdua."),

    ("E-02", "Interruptors locals i variables", "E", 3, ["E-01"],
     "<code>NarrativeState</code> té flags globals i prou. Sense interruptors locals, "
     "cada cofre d'un mapa necessita una flag global amb nom únic, que és com es "
     "fa impracticable escriure contingut.",
     ["include/RPG/NarrativeEngine.h", "src/RPG/NarrativeEngine.cpp", "src/Game/SaveGame.cpp"],
     "Cada esdeveniment té els seus interruptors locals A–D i el joc té variables "
     "numèriques amb nom. Tot va al guardat.",
     "<code>test_narrative</code>: dos cofres amb el mateix interrupter local en mapes "
     "diferents no s'interfereixen; el guardat els conserva."),

    ("E-03", "Planificador, esperes i processos paral·lels", "E", 4, ["E-02"],
     "Un beat s'executa sencer i de cop. Sense esperes no hi ha ritme narratiu: "
     "ni una porta que s'obre a poc a poc, ni un NPC que camina mentre parles.",
     ["include/RPG/EventInterpreter.h", "src/RPG/EventInterpreter.cpp"],
     "L'intèrpret executa comandes al llarg de diversos frames, amb esperes per "
     "temps i per condició, i admet processos paral·lels que no bloquegen el "
     "jugador.",
     "<code>test_event_interpreter</code>: seqüència amb dues esperes; el nombre de "
     "ticks fins al final és determinista."),

    ("E-04", "El joc de comandes", "E", 7, ["E-03", "D-09"],
     "Les comandes que el contingut necessita per no dependre de C++: diàleg amb "
     "eleccions, canvis d'estat i inventari, moviment, càmera, teletransport, "
     "branques, bucles i esdeveniments comuns.",
     ["src/RPG/EventCommands.cpp", "include/RPG/EventCommand.h", "src/Game/GameSession.cpp"],
     "Les comandes de les cinc famílies estan implementades i documentades a "
     "<code>FORMATO_AVENTURAS.md</code>, amb el seu efecte sobre la sessió.",
     "<code>test_event_commands</code>: una comanda de cada família, amb el seu efecte "
     "observable sobre una <code>GameSession</code> de prova."),

    ("E-05", "Editor de pàgines d'esdeveniment", "E", 8, ["E-01", "C-14"],
     "És la primera meitat del que fa que això sigui un creador de jocs i no un "
     "motor: poder crear un esdeveniment sobre una casella i donar-li pàgines "
     "amb condicions.",
     ["editor/src/event_editor.cpp", "include/Editor/EventEditorState.h",
      "tests/Editor/EventEditorTests.cpp"],
     "Des de l'editor es crea un esdeveniment sobre una casella, se li afegeixen "
     "pàgines, i cada pàgina té disparador, condicions i aparença. Tot GL-free "
     "sota <code>EventEditorState</code>.",
     "<code>test_event_editor</code>: crear, duplicar, reordenar i esborrar pàgines; "
     "desfer restaura l'estat anterior."),

    ("E-06", "Editor vertical de comandes", "E", 9, ["E-04", "E-05"],
     "La segona meitat: la llista de comandes d'una pàgina, editable, amb "
     "inserció, sagnat de les branques i moviment.",
     ["editor/src/event_editor.cpp", "include/Editor/CommandListState.h"],
     "La llista de comandes s'edita amb teclat: inserir, esborrar, moure, entrar "
     "en una branca. Les comandes amb paràmetres obren el seu formulari.",
     "<code>test_command_list</code>: construir una seqüència amb branca i bucle des "
     "de l'API de l'editor i executar-la a l'intèrpret."),

    ("E-07", "Validació estàtica d'aventures", "E", 4, ["E-04"],
     "<code>EditorValidation::checkRpg</code> ja resol referències de skills, sprites, "
     "enemics i transicions, i l'editor no la crida mai. Ampliar-la a "
     "esdeveniments és continuació, no obra nova.",
     ["include/RPG/AdventureValidation.h", "src/RPG/AdventureValidation.cpp",
      "editor/src/level_editor.cpp"],
     "Beats i esdeveniments es validen: flags que ningú encén, beats "
     "inabastables, ids duplicats, referències a NPC, enemics o skills "
     "inexistents, temporitzadors que no arrenca ningú. L'editor la crida abans "
     "de desar i abans del playtest.",
     "<code>test_adventure_validation</code>: una aventura amb sis defectes coneguts "
     "dóna sis diagnòstics amb la seva ubicació."),

    ("E-08", "Depuració pas a pas", "E", 5, ["E-03", "E-06"],
     "Quan un esdeveniment no fa el que l'autor esperava, avui l'única eina és "
     "llegir el JSON i endevinar.",
     ["editor/src/event_editor.cpp", "src/RPG/EventInterpreter.cpp"],
     "Es pot executar un esdeveniment comanda a comanda, veure l'estat de flags "
     "i variables a cada pas i posar punts d'aturada.",
     "<code>test_event_interpreter</code>: mode pas a pas; l'estat després de N passos "
     "és igual que executar-ne N de cop."),

    ("E-09", "La missió de la clau, la porta i el cofre", "E", 3, ["E-07", "E-08"],
     "La prova d'acceptació de tota la fase: la missió canònica de qualsevol "
     "creador de jocs, escrita sense tocar codi.",
     ["assets/adventures/mision_llave.json", "assets/levels/"],
     "La missió s'escriu sencera des de l'editor: cofre amb clau, porta tancada "
     "que la demana, NPC que en parla, i final. Validada i jugable.",
     "<code>demo_mision_llave</code> a CTest: recorre la missió sencera de manera "
     "determinista i comprova l'estat final."),

    # ---------------- FASE F ----------------
    ("F-01", "Playtest des de l'editor", "F", 4, ["C-16", "E-09"],
     "Avui la tecla P llança <code>./juego</code> com a procés a part, amb el que hi "
     "hagi desat al disc. No és provar el que estàs editant: és provar l'última "
     "vegada que vas prémer G.",
     ["editor/src/level_editor.cpp", "src/Engine/Application.cpp"],
     "El playtest arrenca dins del mateix procés, amb el document en memòria, "
     "sense desar. Sortir-ne torna exactament a l'estat d'edició.",
     "<code>test_playtest</code>: entrar i sortir del playtest deixa "
     "<code>EditorState</code> byte a byte igual."),

    ("F-02", "Provar des de la casella seleccionada", "F", 2, ["F-01"],
     "Provar una missió del final del mapa obligant-te a caminar-hi cada vegada "
     "és el que fa que els autors deixin de provar.",
     ["editor/src/level_editor.cpp"],
     "El playtest arrenca amb el jugador a la casella marcada, amb l'estat de "
     "partida que l'autor esculli (flags i objectes preparats).",
     "<code>test_playtest</code>: arrencar a tres caselles diferents amb tres jocs de "
     "flags."),

    ("F-03", "Recàrrega en calent", "F", 4, ["F-01"],
     "El cicle editar→provar→corregir només és útil si és de segons.",
     ["src/Engine/Application.cpp", "include/Map/MapDocument.h"],
     "Modificar el mapa o un esdeveniment mentre el playtest corre l'actualitza "
     "sense reiniciar la partida. El que no es pot recarregar en calent ho diu "
     "clar en comptes de fallar a mitges.",
     "<code>test_hot_reload</code>: canviar un tile i un esdeveniment amb la partida "
     "viva; l'estat del jugador es conserva."),

    ("F-04", "Consola i inspector d'estat", "F", 3, ["F-01"],
     "Sense veure flags, variables i inventari en viu, depurar contingut és "
     "endevinar.",
     ["editor/src/console.cpp", "include/Editor/ConsoleState.h"],
     "Durant el playtest es poden veure i modificar flags, variables, or i "
     "inventari, i llegir el registre d'esdeveniments disparats.",
     "<code>test_console</code>: encendre una flag des de la consola dispara el beat "
     "que hi depèn."),

    # ---------------- FASE G ----------------
    ("G-01", "Autosave i recuperació", "G", 2, ["C-15"],
     "Perdre feina d'autoria per un tancament inesperat és el defecte que fa que "
     "una eina no es torni a obrir.",
     ["src/Editor/EditorState.cpp", "editor/src/level_editor.cpp"],
     "El document es desa sol cada N canvis en un fitxer de recuperació. En "
     "obrir, si n'hi ha un de més nou que el desat, l'editor ho ofereix.",
     "<code>test_autosave</code>: simular una caiguda amb canvis pendents i recuperar-los."),

    ("G-02", "Les eines d'edició que falten", "G", 3, ["C-14"],
     "L'editor té pinzell, esborrar, omplir, col·locar i enllaçar. Li falten "
     "comptagotes, rectangle i línia — les tres que qualsevol autor busca la "
     "primera hora.",
     ["include/Editor/EditorState.h", "src/Editor/EditorState.cpp",
      "editor/src/level_editor.cpp"],
     "Comptagotes, rectangle i línia, cadascuna com una sola operació d'historial.",
     "<code>test_editor_state</code>: cada eina nova, amb el seu desfer."),

    ("G-03", "Historial per comandes", "G", 5, ["C-14", "G-02"],
     "L'historial copia el mapa sencer a cada pinzellada, amb un límit de 256 "
     "instantànies i un descart O(n) d'instantànies completes. És el paquet ja "
     "marcat REPLANTEAR al Gantt: amb mapes grans, l'editor no s'aguanta.",
     ["include/Editor/EditorState.h", "src/Editor/EditorState.cpp"],
     "L'historial guarda comandes reversibles, no fotografies. Un mapa de "
     "200×200 amb quatre capes admet mil operacions sense creixement notable de "
     "memòria.",
     "<code>test_editor_state</code>: mil operacions barrejades; desfer-les totes "
     "retorna al document inicial i la memòria es manté sota un límit."),

    ("G-04", "Joc d'exemple de trenta minuts", "G", 5, ["E-09", "F-03"],
     "La prova que el corte B és real: un joc petit fet només amb l'eina.",
     ["assets/proyectos/ejemplo/", "assets/levels/", "assets/adventures/"],
     "Un joc de trenta minuts amb tres mapes, sis esdeveniments, combat i final, "
     "construït sense tocar C++.",
     "<code>demo_juego_ejemplo</code> a CTest el recorre sencer de manera determinista."),

    ("G-05", "Tutorial de l'editor", "G", 4, ["G-04"],
     "Una eina que necessita que hi siguis tu al costat no és una eina que puguis "
     "donar a ningú.",
     ["docs/tutorial/", "editor/src/level_editor.cpp"],
     "Un tutorial guiat dins de l'editor que porta de zero a un mapa jugable amb "
     "un esdeveniment.",
     "Prova amb una persona que no hagi vist mai el projecte: acaba el tutorial "
     "sense preguntar res."),

    ("G-06", "Empaquetat i exportació", "G", 5, ["G-04"],
     "Un joc que només es pot jugar des del directori de build no s'ha acabat.",
     ["tools/exportar.py", "CMakeLists.txt"],
     "Un projecte s'exporta a un executable autocontingut per a Linux i Windows, "
     "amb només els assets que fa servir.",
     "CI construeix l'export a les dues plataformes i l'executa amb "
     "<code>--frames 60</code>."),

    ("G-07", "Congelació dels esquemes", "G", 2, ["G-04", "G-06"],
     "A partir d'aquí, qualsevol canvi de format ha de saber llegir el que ja hi "
     "ha escrit.",
     ["FORMATO_PROYECTOS.md", "FORMATO_NIVELES.md", "FORMATO_AVENTURAS.md"],
     "Els esquemes de projecte, mapa, aventura i guardat es congelen a la versió "
     "1.0-B, documentats, amb ruta de migració per a cada canvi futur.",
     "<code>test_schema_freeze</code>: fixtures de la versió 1.0-B es carreguen sense "
     "avisos amb el codi del dia."),
]

# Fora de l'abast, per escrit: el que NO farem al corte B.
FORA = [
    ("Autotiles i terrenys intel·ligents", "MAP-03", "Molta feina d'editor per a un "
     "resultat que es pot pintar a mà en mapes de la mida que fem ara."),
    ("Sistema de plugins (runtime i editor)", "TOOL-06..08", "No té sentit obrir una "
     "API d'extensió abans que l'API interna estigui estable. Depèn de la fase C."),
    ("Àudio, música i ambient", "GDB-11", "Cap peça del motor el toca avui; és una "
     "vertical sencera i no bloqueja el cicle d'autor."),
    ("Base de dades completa i editor mestre-detall", "GDB-01..12", "La fase D deixa "
     "els catàlegs llegibles; editar-los des de l'eina és 1.1."),
    ("Exportació a macOS i empaquetat de botiga", "TOOL-10", "Linux i Windows "
     "cobreixen la prova del corte B."),
    ("Combat tàctic avançat i IA de grup", "—", "El combat per torns actual és "
     "suficient per validar el cicle d'autor."),
]
