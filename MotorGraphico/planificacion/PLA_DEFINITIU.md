# Pla definitiu — MotorGraphico Creator, corte B

Generat des de `planificacion/pla/dades.py` i `planificacion/seguimiento/*.json`. Aquest fitxer no s'edita a mà: es regenera, i així el markdown, el Gantt i la pàgina no poden divergir.

## 1. Les decisions que fixen el pla

**Objectiu de versió → Corte B — cicle d'autor complet.** Que algú que no sigui l'Oriol pugui fer un joc amb l'editor: mapes, esdeveniments, dades locals del projecte i playtest sense tocar codi. El jugador ha de poder començar, configurar el personatge, gestionar-lo, guardar i continuar des de pantalles. Autotiles, plugins, àudio, edició avançada dels 21 catàlegs i exportació a més plataformes queden a 1.1.

**Catàlegs RPG → Arreglar-los tots els 21.** Cada carregador ha de llegir de veritat el seu JSON, amb una prova que comprovi el contingut d'un camp i no només que hi hagi entrades.

**Codi orfe → Consolidació prèvia, abans de res més.** MapDocument, SceneManager, World, HandlePool, ProjectSerializer i el post-processat es connecten primer. Quan la fase C tanqui, no hi ha d'haver dues implementacions vives del mateix concepte.

**Revisió de pantalles · 03/09/2026 → Recorreguts complets d'autor i jugador.** Encàrrec d'Oriol després de comparar les pantalles amb RPG Maker. S'afegeixen E-10..16 i G-08..16, amb dependències, acceptació i proves. Són activitats pendents, no funcionalitats entregades. Els imports de Onegai continuen sent de lectura: l'edició local no publica canon ni modifica els projectes de cartes o del vault.

## 2. El número

| Fase | Paquets | Jornades | Setmanes a ritme real |
| --- | ---: | ---: | ---: |
| C — Consolidació | 24 | 62 | 41 |
| D — Dades reals | 10 | 30 | 20 |
| E — Autoria visual i esdeveniments | 16 | 79 | 53 |
| F — Playtest des de l'editor | 4 | 13 | 9 |
| G — Tancament del corte B | 16 | 59 | 39 |
| **Total** | **70** | **243** | **162** |

Una jornada és ~6 h de treball enfocat. A ritme de ratos lliures i caps de setmana (~1,5 jornades per setmana), les 243 jornades són unes 162 setmanes naturals.

**Revisió de pantalles (03/09):** la base era de 54 paquets i 177 jornades. S'hi afegeixen 16 paquets i 66 jornades per tancar les pantalles i recorreguts. És una estimació completa d'abast, no de feina restant ni una data de lliurament. No es resten paquets parcialment verificats ni es compten caselles locals com a tancaments de CI. El calendari del Gantt és una escala d'esforç amb dependències, no una previsió de dates naturals.

## 3. Què vol dir «fet» a cada fase

### Fase C — Consolidació

Una sola veritat per concepte, i cap bug que el jugador noti. És la fase que no afegeix cap funció nova i sense la qual totes les altres es construeixen damunt de dos fonaments alhora.

**Tancada quan:** Cap concepte del motor té dues implementacions vives, els vuit bugs crítics estan tancats amb prova de regressió, i el camí que es juga de veritat té tests.

### Fase D — Dades reals

Que el que diu el JSON sigui el que arriba al joc. Avui 19 dels 21 carregadors llegeixen cinc camps i s'inventen la resta en silenci.

**Tancada quan:** Els 21 catàlegs carreguen tots els seus camps, un carregador que no troba el que espera falla en comptes de callar, i CI valida els assets reals a cada commit.

### Fase E — Autoria visual i esdeveniments

Projectes, mapes, recursos, dades RPG mínimes i esdeveniments accessibles des de pantalles. Reutilitzar els models existents; cap segon editor de dades.

**Tancada quan:** Un autor crea el projecte i la missió de la clau des de l'editor, tria o personalitza les dades locals, desa i reobre sense pèrdues, corregeix diagnòstics i depura els esdeveniments pas a pas.

### Fase F — Playtest des de l'editor

El bucle d'autoria: editar, provar, corregir sense sortir de l'eina.

**Tancada quan:** Des d'un mapa obert, una tecla llança la partida en aquell punt, i tornar a l'editor conserva el que s'estava editant.

### Fase G — Tancament del corte B

El que converteix una eina que funciona en una eina que es pot donar a algú altre.

**Tancada quan:** Un tercer segueix el tutorial, crea i exporta un joc; un altre l'obre fora del repositori, crea el personatge, juga, equipa, guarda, continua i recupera una derrota sense ordres de terminal. G-16 registra la prova.

## 4. Els paquets

Cada paquet porta el seu criteri d'acceptació i la prova que el tanca. Si una d'aquestes dues coses no es pot escriure, el paquet no està prou definit per començar-lo.

### Fase C — Consolidació

#### C-01 — La curació cura qui toca

`2 j` · depèn de: cap

**Per què.** Avui l'ordre «Cura» gasta 4 PM del jugador i cura ~20 PV al monstre. `resolveAllyAction` tria sempre l'objectiu dins de `m_enemies` i ningú no llegeix `Skill::target`.

**Fitxers.** `src/Game/BattleState.cpp`, `include/Game/Skill.h`, `tests/Game/BattleStateTests.cpp`

**Fet quan.** Una habilitat amb `target = Ally` aplica el seu efecte sobre el jugador; una amb `target = Enemy`, sobre l'enemic seleccionat. El PM es gasta una sola vegada i només si l'efecte s'ha pogut aplicar.

**Ho prova.** `test_battle_state`: curar amb vida a 30/60 deixa el jugador a 50 i l'enemic intacte; el mateix cas amb el codi antic falla.

**Seguiment · 2026-09-03 · Claude (Cowork): Tancat.**

- `resolveAllyAction` resol la habilitat ABANS de l'objectiu, perquè és `Skill::target` qui diu contra quin bàndol es valida. `Self` ignora `targetIndex`; `Self/SingleAlly/AllAllies` van al bàndol propi, la resta al contrari.
- L'ordre nou arregla de passada el PM: es comprova que la habilitat existeix i es pot pagar, es valida l'objectiu, i només llavors es gasta — una sola vegada i només si l'efecte s'aplica.
- Les variants `All*` es resolen sobre UN participant del bàndol correcte: l'àrea d'efecte és funció nova i la fase C no n'admet (punt 6 del pla). El que s'arregla és el bàndol, que era el bug.
- Prova nova `tests/Game/BattleStateTests.cpp` (`test_battle_state`), amb `ScriptedRng` perquè el resultat sigui exacte i no «el que sortís de la llavor».

**Verificat.** 4 casos verds: curar amb 30/60 deixa el jugador a 50 i l'enemic intacte i el mana a 6; una habilitat de dany segueix anant a l'enemic; `Self` s'aplica sobre qui actua encara que el HUD enviï un índex fora de rang; un objectiu invàlid no gasta mana. Amb el codi anterior, 2 dels 4 fallen. `demo_battle` i `demo_battle_nd6` segueixen passant.

**Pendent.** Res del paquet. Confirmar-ho a CI amb la resta del bloc.

#### C-02 — El minimapa deixa de llegir memòria alliberada

`1 j` · depèn de: cap

**Per què.** `HudMinimap` guarda el punter a la `GameSession` del primer nivell i no té setter. Des de la primera porta creuada llegeix un objecte destruït a cada frame: avui no peta per sort.

**Fitxers.** `include/Render/HudMinimap.h`, `src/Render/HudMinimap.cpp`, `src/Engine/Application.cpp`

**Fet quan.** El minimapa rep la sessió per `setSession()` i `loadLevel` l'hi torna a passar. Amb sessió nul·la es dibuixa buit en comptes de llegir.

**Ho prova.** `test_hud_minimap`: construir, canviar de sessió i destruir la primera; el widget segueix responent i no toca la vella. Verificat també amb `-fsanitize=address` sobre `demo_game_session`.

**Seguiment · 2026-09-03 · Claude (Cowork): Tancat.**

- `HudMinimap` guanya `setSession()`, `session()` i `isDrawable()`. `loadLevel` li torna a passar la sessió acabada de construir; `shutdown()` l'hi treu abans de destruir-la.
- Sense sessió o sense mapa el widget no llegeix RES (ni `getWidth()` ni `playerPosition()`): `isDrawable()` és l'única porta per la qual es toquen aquests dos punters, i `render()` hi surt abans de tocar el `SpriteBatch`.
- També hi ha `setMap()`, per no deixar cap punter fixat al constructor.
- Prova nova `tests/Render/HudMinimapTests.cpp` (`test_hud_minimap`). Prova la guarda i el reenganxament, no `render()`: `SpriteBatch` és recurs GL i demana finestra. S'enllacen `SpriteBatch`/glad només per resoldre símbols que no es criden, el mateix patró que `demo_bitmap_font`.

**Verificat.** 3 casos verds, inclòs el del bug: construir amb la sessió del nivell 1, canviar a la del 2, destruir la primera i comprovar que el widget respon i apunta a la nova. Repetit sota `-fsanitize=address` sense cap accés a memòria alliberada.

**Pendent.** Res del paquet.

#### C-03 — L'estat del jugador sobreviu el canvi de nivell

`2 j` · depèn de: cap

**Per què.** `loadLevel` rescata només l'or. Entrar a una posada et reinicia la reputació a 0, et deixa sense negocis i esborra el comptador de rendes.

**Fitxers.** `src/Engine/Application.cpp`, `include/Game/GameSession.h`, `src/Game/SaveGame.cpp`

**Fet quan.** Moral, negocis, passos des de l'últim cobrament i or es transfereixen sencers entre sessions. El traspàs és una sola funció, no una llista de camps escampada per `loadLevel`.

**Ho prova.** `test_game_session`: sessió amb moral −30, dos negocis i 90 d'or; reconstruir-la sobre un altre nivell i comprovar els quatre valors.

**Seguiment · 2026-09-03 · Claude (Cowork): Tancat.**

- Tipus nou `PlayerCarryOver` (or, moral, passos des de l'últim cobrament, negocis) i el parell `GameSession::captureCarryOver()` / `applyCarryOver()`: l'ÚNIC camí pel qual l'estat del jugador travessa la frontera entre sessions.
- `Application::loadLevel` ja no rescata només l'or. Un sol tipus en comptes d'una llista de camps escampada.
- `SaveGame.cpp` fa servir el mateix traspàs a `captureSaveGame` i `restoreSaveGame`: afegir demà un camp del jugador surt als dos llocs o a cap.
- `applyCarryOver` ASSIGNA, no acumula: aplicar-lo dues vegades dona el mateix (una recàrrega del mateix nivell no duplica l'or).
- Prova nova `tests/Game/GameSessionTests.cpp` (`test_game_session`).

**Verificat.** 4 casos verds: el cas del pla (moral −30, dos negocis, 90 d'or, reconstruït sobre un altre nivell), que una sessió nova neix buida, la idempotència i que el traspàs no toca vida ni inventari (viuen fora de la sessió). `test_save_game` sencer segueix verd després de reescriure els seus quatre camps.

#### C-05 — Tancament ordenat dels recursos GL

`1 j` · depèn de: cap

**Per què.** `shutdown()` no buida els dos ResourceManager: es destrueixen al destructor d'Application, quan `glfwTerminate()` ja ha passat i `glDeleteTextures` no té context on anar.

**Fitxers.** `src/Engine/Application.cpp`, `include/Engine/Application.h`

**Fet quan.** Textures i shaders s'alliberen mentre el context és viu. Sortir del joc no deixa cap crida GL després de `glfwTerminate()`.

**Ho prova.** Execució de `juego` amb `--frames 3` sota `ASAN_OPTIONS=detect_leaks=1`, sense fuites de textura.

**Seguiment · 2026-09-08 · Claude (Cowork): Implementat i verificat amb GL real i ASan/UBSan; CI pendent.**

- `shutdown()` buida `m_shaderManager` i `m_textureManager` amb el context GL encara viu, després dels atles (que només referencien les textures) i just abans de `m_window.reset()`.
- Abans no es tocaven: sobrevivien fins al destructor d'`Application`, que corre DESPRÉS que `~Window` hagi cridat `glfwTerminate()` — i llavors `~Texture`/`~Shader` emetien `glDeleteTextures`/`glDeleteProgram` sense cap context on anar.
- Els dos gestors ja estaven declarats després de `m_window` (ordre invers de destrucció correcte), però això no bastava perquè `shutdown()` solta la finestra molt abans. Documentat al header.
- És idempotent: `shutdown()` es crida a mà i un altre cop des del destructor.

**Verificat.** Compila net amb `-Wall -Wextra`. · 08/09: test_application_shutdown passa amb context GL real, Debug i ASan/UBSan (detect_leaks=0 per cachés del driver). Suite completa 66/66.

**Pendent.** CI remota pendent; LeakSanitizer no verificat en aquest entorn.

#### C-06 — L'ajust de text no es penja

`1 j` · depèn de: cap

**Per què.** `wrapText` amb una caixa d'un sol glifu d'amplada no consumeix res i afegeix «-» per sempre. Hi arriba qualsevol `HudText` prou estret.

**Fitxers.** `src/Render/BitmapFont.cpp`, `tests/Render/BitmapFontTests.cpp`

**Fet quan.** Amb una amplada menor que un glifu, `wrapText` retorna el text tallat sense inventar guions i sense bucle.

**Ho prova.** `test_bitmap_font`: amplades 0, 1 i 2 sobre una frase llarga; la funció retorna en temps finit i el resultat cap a la caixa.

**Seguiment · 2026-09-03 · Claude (Cowork): Tancat.**

- Amb la caixa d'UN sol glif no hi cap «glif + guió»: `maxGlyphs - 1` era 0, no es consumia cap lletra de la paraula i el bucle empenyia «-» per sempre. No era un penjat teòric: el vector creixia fins a quedar-se sense memòria, i hi arribava qualsevol `HudText` prou estret.
- Amb `maxGlyphs == 1` es talla a pèl, un glif per línia i sense inventar guions: el guió és una ajuda de lectura, no una obligació. Amb espai, el comportament d'abans no canvia.
- Guarda addicional: si una volta no consumeix res, se surt en comptes de girar. La mateixa classe de fallada que arregla el paquet.
- Prova nova `tests/Render/BitmapFontTests.cpp` (`test_bitmap_font`).

**Verificat.** 5 casos verds: caixa d'un glif sense penjar-se ni inventar guions; amplades 0, 1 i 2 en temps finit i cap línia més ampla que la caixa; cap lletra perduda; amb espai, el guió es manté; les accentuades segueixen comptant com un glif. Amb el codi antic, 4 dels 5 moren de `bad_alloc` (amb el límit de memòria posat; sense límit, no tornen).

#### C-07 — Les aventures autònomes disparen els seus beats

`2 j` · depèn de: cap

**Per què.** Amb `--precuela`, `m_narrativeCampaignTransitions` queda a fals i `loadLevel` no crida mai `setNarrative`: l'aventura es carrega i no passa absolutament res.

**Fitxers.** `src/Engine/Application.cpp`, `include/Engine/Application.h`

**Fet quan.** Qualsevol aventura carregada s'enganxa a la sessió. La bandera de campanya només decideix si es fan les transicions de la cronologia, no si la narrativa existeix.

**Ho prova.** `demo_precuela_boundington` promogut a CTest comprova que el primer beat es dispara; abans no ho feia.

**Seguiment · 2026-09-03 · Claude (Cowork): Implementat; sense prova de regressió.**

- La condició de `loadLevel` era `m_narrativaLista && m_narrativeCampaignTransitions`. Amb `--precuela` (o qualsevol aventura autònoma) la bandera de campanya és false, no es cridava mai `setNarrative()` i l'aventura es carregava per no fer absolutament res.
- I al revés: el bloc de transicions de la cronologia d'`update()` NO comprovava la bandera, o sigui que una aventura autònoma encadenava igualment els dies de Boundington.
- Invertit: tota aventura carregada s'enganxa a la sessió; la bandera decideix només les transicions de la cronologia, que és el que significa. Documentat al header.

**Verificat.** `Application.cpp` compila net.

**Pendent.** `demo_precuela_boundington` prova el `NarrativeEngine` directament, no el cablejat d'`Application`: mai va cobrir aquest bug i segueix sense cobrir-lo. La guarda viu dins d'`Application`, que és GL i no entra a la suite gl-free. Per la regla 2 del punt 6, el paquet NO està tancat fins que hi hagi arnès a nivell d'Application (C-16/C-24).

#### C-08 — Un guardat antic no escriu fora de rang

`1 j` · depèn de: cap

**Per què.** `tickTimers` indexa `avisosDados[i]` contra la mida de la definició, i el guardat el restaura amb la mida que tenia el dia que es va desar. Afegir un avís a una aventura converteix els guardats existents en escriptura fora de rang.

**Fitxers.** `src/RPG/NarrativeEngine.cpp`, `src/Game/SaveGame.cpp`, `tests/Game/SaveGameTests.cpp`

**Fet quan.** En restaurar, el vector d'avisos es redimensiona a la mida de la definició actual, conservant els que ja s'havien donat.

**Ho prova.** `test_save_game`: guardat amb dos avisos restaurat sobre una definició de quatre; cap escriptura fora de rang sota ASan.

**Seguiment · 2026-09-03 · Claude (Cowork): Tancat.**

- `tickTimers` redimensiona `avisosDados` a la mida de la definició ACTUAL abans que ningú l'indexi. `resize()` i no `assign()`: conserva el prefix — els avisos que el jugador ja va rebre — i omple amb false només els nous. Treure un avís també queda cobert.
- `applyNarrativeState` copia el vector tal com ve: allà no hi ha `AdventureScript` amb què comparar-lo. Qui coneix la definició viva és `NarrativeEngine`, i és allà on s'ajusta. Documentat als dos llocs.
- 2 casos nous a `test_save_game`.

**Verificat.** Verds: guardat amb dos avisos restaurat sobre una definició de quatre (el cas del pla) i el simètric, quatre sobre un. Amb el codi antic tots dos fallen.

**Nota per a qui hi torni.** ASan NO veu aquesta escriptura fora de rang: `avisosDados` és `std::vector<bool>`, un camp de bits, i l'índex de més cau dins la mateixa paraula ja reservada. És a dir que el bug era encara més silenciós del que sembla, i el que el fixa és la comprovació de MIDA, no el sanititzador.

#### C-09 — Propietat de recursos i destrucció polimòrfica

`1 j` · depèn de: cap

**Per què.** `Texture::operator=(Texture&&)` sobreescriu `m_glID` sense alliberar el que ja tenia, i `RuntimeTileMap` no té destructor virtual tot i que `TileMap` n'hereta i el renderer en guarda un punter a la base.

**Fitxers.** `include/Core/Resources/Texture.h`, `include/Runtime/RuntimeTileMap.h`

**Fet quan.** L'assignació per moviment allibera el recurs de destí abans de robar el d'origen. `RuntimeTileMap` té destructor virtual.

**Ho prova.** `test_texture` amb un doble GL fals: assignar sobre una textura viva n'allibera el nom exactament una vegada.

**Seguiment · 2026-09-03 · qwen: Tancat localment; el tanca formalment el pas per CI.**

- Texture::operator=(Texture&&) allibera el recurs del destí abans de robar el de l'origen (fuga del nom GL); la definició passa a Texture.cpp perquè necessita glDeleteTextures.
- RuntimeTileMap guanya destructor virtual: TileMap n'hereda i IsometricRenderer/HudMinimap guarden punters a la base (la línia viu al fitxer orfe sense commitear del 28/08).
- test_texture nou (4 casos) amb doble GL fals: redirigeix glad_glDeleteTextures sense obrir context i compta cada alliberació exactament una vegada.

**Verificat.** test_texture verd; el cas de la fuga falla amb el codi anterior. · Build completa de tots els targets en macos-debug i suite gl-free sencera en verd (commit 50098f8).

**Pendent.** El destructor virtual de RuntimeTileMap.h queda dins el bloc orfe sense commitear del 28/08 (regla 2); es commitearà amb la consolidació d'aquest bloc.

#### C-10 — El JSON que escrivim el podem tornar a llegir

`1 j` · depèn de: cap

**Per què.** `JsonWriter` emet `\u00xx` per als caràcters de control i `JsonParser` rebutja `\u` explícitament: el round-trip que la capçalera diu que és la comprovació que importa està trencat.

**Fitxers.** `include/Core/Json/JsonWriter.h`, `src/Core/Json/JsonValue.cpp`, `tests/Core/JsonTests.cpp`

**Fet quan.** El parser entén `\uXXXX` i el codifica a UTF-8. Tot el que surt de `JsonWriter` torna a entrar per `JsonValue::parse` sense pèrdua.

**Ho prova.** `test_json`: round-trip d'una cadena amb caràcters de control, accents, cometes i barres.

**Seguiment · 2026-09-03 · qwen: Tancat localment; el tanca formalment el pas per CI.**

- JsonValue::parse decodifica \uXXXX a UTF-8, inclosos els parells de substitució; rebutja substituts orfes, dígits no hexadecimals i escapes incomplets amb missatge propi.
- El round-trip JsonWriter → JsonValue::parse deixa d'estar trencat: els caràcters de control que l'escriptor escapa com \u00xx tornen a entrar sense pèrdues.
- test_json nou: round-trip amb control/accents/cometes/barres, escapes directes, parells de substitució i casos de rebuig.

**Verificat.** test_json verd; build completa i suite gl-free sencera en verd (commit 50098f8).

**Pendent.** La trampa de la BITACORA Part 3 sobre \uXXXX queda obsoleta des d'aquest paquet; actualitzar-la quan es toqui la BITACORA (ara té capes alienes sense commitear).

#### C-11 — Un sol format de projecte

`4 j` · depèn de: cap

**Per què.** `ProjectIndex` llegeix claus castellanes a l'arrel i `ProjectSerializer` escriu claus angleses amb `settings` niat. Cap dels dos pot llegir el fitxer de l'altre, i tots dos es diuen canònics.

**Fitxers.** `src/Project/ProjectSerializer.cpp`, `src/Project/ProjectIndex.cpp`, `FORMATO_PROYECTOS.md`, `tools/migrar_proyectos.py`

**Fet quan.** Un únic format v1, amb `schemaVersion`, que `ProjectSerializer` llegeix i escriu i que `ProjectIndex` consumeix. Els cinc manifests d'`assets/proyectos/` queden migrats i el format vell es rebutja amb un missatge que diu com migrar.

**Ho prova.** `test_project_serialization`: carregar els cinc manifests reals migrats, desar-los i comparar bytes.

**Seguiment · 2026-09-07 · Codex: Integrat en Git i verificat localment; CI pendent.**

- ProjectIndex llegeix i crea manifests mitjançant ProjectSerializer; un únic format v1 amb settings niat. El format antic o mixt es rebutja amb el comandament de migració.
- Migrats els cinc manifests reals amb metadades i referències conservades; migrador amb previsualització, còpia prèvia, validació del lot i substitució atòmica de cada fitxer. Una segona execució no canvia res.
- Actualitzats build_proyecto.py, validar_todo.py, FORMATO_PROYECTOS.md i el schema v1; les rutes es resolen respecte de la carpeta assets indicada.
- Crear o eliminar un projecte conserva referències a altres manifests invàlids o absents; l’índex buit permet crear el primer projecte.
- Afegits test_project_manifests i test_project_migration; els canvis coordinats sobre la base aliena sense versionar queden aplicats al workspace i preservats a C-11-integracion.patch.
- Revisió 07/09: el rescat 9e40405/52d696f ja incorpora la base i els hunks de C-11. No es reaplica el parche ni es versionen els canvis posteriors aliens de Map/Project.

**Verificat.** Suite GL-free local: 56/56 proves, 0 errors. Després dels ajustos finals, les tres proves test_project_* tornen a passar. · Round-trip byte a byte dels cinc manifests reals, equivalència ProjectIndex/ProjectSerializer, rebuig de camps desconeguts i resolució d’entrades sobre assets copiat. · Migrador: 6/6 casos Python; previsualització sense escriptura, idempotència, índex buit i cap migració parcial per dades invàlides. · Comparació amb el builder de HEAD sobre els manifests antics: build.json idèntic en els cinc packs. demo_proyectos passa; validar_proyectos dona 0 errors i 2 avisos previs. · 07/09: les 12 empremtes SHA-256 del registre coincideixen amb HEAD; git apply --reverse --check --cached del parche complet passa (inclòs CMake). Build completa macos-debug i CTest GL-free: 62/62.

**Pendent.** Validar a CI per tancar formalment el paquet. La verificació 07/09 és local; no s’ha publicat ni executat un workflow remot.

#### C-12 — MapDocument es pot editar

`5 j` · depèn de: cap

**Per què.** El document de mapa nou és d'escriptura única: no té `removeLayer`, ni accés no-const a les dades d'una capa, ni manera de reordenar. Mentre sigui així, l'editor no s'hi pot migrar encara que es vulgui.

**Fitxers.** `include/Map/MapDocument.h`, `src/Map/MapDocument.cpp`, `include/Map/MapLayer.h`, `tests/Map/MapDocumentTests.cpp`

**Fet quan.** API de mutació completa: pintar cel·les, afegir i treure entitats i regions, esborrar capes, reordenar-les i canviar-ne els metadades. Cada mutació manté les invariants que avui només comprova `validate()`.

**Ho prova.** `test_map_document`: seqüència de 20 mutacions; el document valida després de cadascuna i el resultat és el mateix que construir-lo d'una peça.

**Seguiment · 2026-09-03 · Claude (Cowork): Tancat.**

- API de mutació completa a `MapDocument`: `removeLayer`, `setLayerMeta`, `moveLayer`, `setTile`, `fillTiles`, `addEntity`/`removeEntity`/`moveEntity`, `addRegion`/`removeRegion`/`setRegionBounds`, més `findLayer` per no recórrer `layers()` a mà a cada clic de l'editor.
- Tot passa per un únic `mutateLayer`: COPIA la capa, hi aplica la mutació, la revalida contra el mapa (dimensions, límits, ids únics) i només llavors la instal·la. Una mutació invàlida deixa el document EXACTAMENT com estava. En un sol lloc perquè cap mutació se'l pugui saltar per descuit.
- `MapLayer::mutableMeta()`/`mutableData()` són privats amb `friend class MapDocument`: es pot editar una capa, però no a esquena del document. Públics, permetrien deixar una entitat fora del mapa sense que ningú se n'assabentés. El TIPUS de la capa segueix sent immutable.
- Les tres classes de dades (`TileLayerDocument`, `EntityLayerDocument`, `RegionLayerDocument`) guanyen els seus mutadors, tots amb `Result` i tot-o-res: pintar és una acció d'usuari, «aquí no» ha de ser un missatge i no un crash. `fill` comprova les cantonades abans de pintar cap cel·la.
- Proves noves a `test_map_document`.

**Verificat.** 7 casos verds (els 3 previs inclosos). El central: 21 mutacions encadenades, `validate()` comprovat DESPRÉS DE CADA UNA, i el document resultant idèntic camp a camp al mateix document construït d'una peça — si l'API d'edició i la de càrrega produïssin coses distintes, es veuria aquí. Més el tot-o-res amb 9 mutacions rebutjades (cel·la fora, GID negatiu, farciment que se surt a mitges, entitat fora dels límits del mapa, id duplicat, tipus de capa equivocat, capa inexistent) i el document intacte després. `test_map_round_trip`, `test_runtime_tilemap`, `test_project_serialization` i `test_tmx_importer` segueixen verds.

**Nota.** Això desbloqueja C-13 i C-14.

#### C-16 — L'Application delega en SceneManager

`4 j` · depèn de: cap

**Per què.** `Scene` i `SceneManager` existeixen, estan provats i no els enllaça res que no sigui el seu test. Mentrestant l'Application porta les fases a mà amb banderes.

**Fitxers.** `src/Engine/Application.cpp`, `include/Engine/Scene/SceneManager.h`, `tests/Engine/SceneManagerTests.cpp`

**Fet quan.** El bucle principal empeny i treu escenes en comptes de mirar banderes. El canvi de nivell i el pas menú↔joc són transicions d'escena.

**Ho prova.** `test_scene_manager` ampliat amb les transicions reals del joc; `demo_game_session` segueix passant.

**Seguiment · 2026-09-07 · Codex + agentes de implementación y pruebas: Implementat i verificat localment; CI remot pendent.**

- Application utilitza SceneManager com a autoritat de títol/joc i per despatxar simulació i render; retirada m_atTitle.
- Títol, portes, cronologia i càrrega de guardats sol·liciten transicions que es resolen al final del frame.
- La càrrega prepara mapa, textura i atlas abans de substituir la sessió. Els projectes preparen també catàlegs i narrativa; un error conserva el context anterior. La memòria cau de tilesets s’identifica per ruta.
- La compatibilitat del guardat es comprova en una sessió aïllada abans de modificar jugador/inventari/narrativa. El tancament descarta escenes i peticions abans dels recursos GL.
- Deu regressions sobre Application real amb context GL, fixtures temporals i escenes de producció; workflow propi Linux Debug/Release. GameSession no requereix modificacions.

**Verificat.** Build completa Debug en directori aïllat codex-c16: juego, editor, demos i proves. · 63/63 CTest GL-free; test_application_scenes (10 casos) i test_application_shutdown passen amb GL real. · Els 10 casos de transicions passen amb AddressSanitizer/UndefinedBehaviorSanitizer; LeakSanitizer desactivat en aquesta execució macOS. · juego 3: sortida normal, framebuffer 1280x720 generat. · Diff propi revisat; canvis previs i simultanis de Map/Project/CMake, C-05 i assets preservats.

**Pendent.** Execució remota del workflow Application scenes; no s’ha fet push ni s’ha tancat formalment el paquet a CI.

#### C-19 — Un sol culling i un sol contracte d'actualització

`2 j` · depèn de: cap

**Per què.** `IsometricRenderer::visibleRangeFor` i `TileMap::visibleRange` són còpies literals l'una de l'altra, i `IUpdatable` i `IHudElement::update` són dos contractes `update(float)` sense cap parentiu.

**Fitxers.** `include/Render/IsometricRenderer.h`, `include/Render/TileMap.h`, `include/Render/IUpdatable.h`, `include/Render/HudElement.h`

**Fet quan.** Una sola funció de culling, amb un sol propietari. Un sol contracte d'actualització per frame, o dos noms diferents si de veritat són coses diferents.

**Ho prova.** `test_occlusion`: la funció única dóna el mateix rang que les dues anteriors sobre deu càmeres i mides de mapa.

**Seguiment · 2026-09-03 · Claude (Cowork): Tancat.**

- Les dues còpies del culling eren idèntiques línia a línia: el `visibleRangeFor()` anònim d'`IsometricRenderer.cpp` i `TileMap::visibleRange()`. Ara hi ha `include/Render/Culling.h` amb `VisibleGridRange(map, camera)`, header-only i GL-free — no obliga a moure ningú de llibreria ni a enllaçar res nou.
- Viu a `Render/` i no a `Runtime/` perquè el culling és del renderer: `RuntimeTileMap` segueix sense dependre de `Camera`. `TileMap::visibleRange` hi delega i es queda com a API de compatibilitat de l'importador TMX.
- Segona meitat del paquet: `IHudElement` HERETA d'`IUpdatable`. Eren dos `update(float)` amb la mateixa signatura i el mateix significat i cap relació de tipus. El cos buit es declara a `IHudElement` i no a `IUpdatable`: per a una `Entity` avançar en el temps segueix sent obligatori.
- Prova nova `tests/Render/OcclusionTests.cpp` (`test_occlusion`).

**Verificat.** La prova porta dins la versió històrica com a oracle i compara les dues sobre 10 càmeres × 4 mides de mapa = 40 combinacions, totes idèntiques; més el mapa buit i la invariant que el rang mai se surt del mapa (de la qual depèn `getTile` dins el bucle de dibuix). `demo_tilemap` (que prova `visibleRange` amb assets reals) segueix passant.

#### C-20 — Un sol escriptor de condicions

`2 j` · depèn de: cap

**Per què.** `CharacterSheet::applyCondition` i `ConditionEngine` fan la mateixa feina amb regles diferents: la fitxa no coneix ni `Stacking`, ni `maxStacks`, ni el recàlcul de modificadors.

**Fitxers.** `include/RPG/CharacterSheet.h`, `src/RPG/CharacterSheet.cpp`, `src/RPG/ConditionEngine.cpp`, `tests/RPG/ConditionEngineTests.cpp`

**Fet quan.** `ConditionEngine` és l'únic camí per aplicar, apilar i retirar condicions. La fitxa n'és el magatzem. A més, `purge_expired` purga de veritat i `breaksOnDamage` es dispara pel dany rebut, no per l'infligit.

**Ho prova.** `test_condition_engine`: apilar fins al màxim, expirar per rondes, trencar per dany rebut; cap camí passa per la fitxa.

**Seguiment · 2026-09-08 · Codex: Nucli i integració de combat implementats i verificats localment; CI pendent.**

- Retirats applyCondition, removeCondition i tickEndOfRound de CharacterSheet: ConditionEngine concentra les regles d'aplicació, apilat, retirada i caducitat.
- purge_expired elimina entrades amb rondes o càrregues esgotades i recalcula modificadors; les consultes ignoren entrades caducades.
- El tick conserva l'efecte de l'última ronda i evita tornar a aplicar efectes ja caducats.
- on_damage_received elimina totes les condicions breaksOnDamage del receptor i recalcula els modificadors; zero, curació i dany bloquejat no les trenquen.
- Afegit test_condition_engine amb 11 casos i registre CTest GL-free independent del scaffolding pendent d'altres sessions.
- BattleState notifica ConditionEngine després dels atacs bàsics i habilitats dels dos bàndols amb els PV realment perduts pel receptor.
- ICombatant exposa la fitxa opcional; Player i EnemyBrain implementen el contracte sense crear fitxes als combatents legacy.
- Application carrega i conserva el catàleg de condicions; GameSession el propaga a les batalles noves i actives. Restaurar PV fora del combat no activa la ruptura.

**Verificat.** Abans del canvi: 2 regressions reproduïdes (purga i ruptura pel dany calculat del tick). · 11/11 casos amb AddressSanitizer i UndefinedBehaviorSanitizer, sense errors. · Build de suite_gl_free i CTest: 54/54 proves GL-free en verd (macOS Debug). · 08/09: build completa Debug correcta; 63/63 proves GL-free i 2/2 executables de regressió GL (escenes i tancament). · test_battle_state: 8 casos, amb variants dels dos bàndols, atac/habilitat, curació, dany bloquejat, fallada, compatibilitat legacy i propagació de GameSession abans/durant la batalla. · test_battle_state i test_condition_engine passen amb AddressSanitizer i UndefinedBehaviorSanitizer.

**Pendent.** Executar CI remota abans de tancar formalment C-20 (regla 6 del pla). · Fase D: interpretar les dades canòniques de condicions (D-04) i aplicar-les des de cartes (D-09); la integració actual actua sobre combatents amb CharacterSheet vinculada.

#### C-21 — Un sol generador d'atzar

`1 j` · depèn de: cap

**Per què.** Hi ha `RandomEngine`/`Xoroshiro128p` en ús i `IRng`/`SeededRng` sense cap referència — i aquest segon ni tan sols enllaçaria, perquè no existeix cap `Rng.cpp`.

**Fitxers.** `include/RPG/Rng.h`, `include/RPG/RandomEngine.h`

**Fet quan.** Una sola interfície d'atzar al motor. La que no es fa servir surt del repositori.

**Ho prova.** El projecte compila i `ctest` passa sencer amb la interfície retirada.

**Seguiment · 2026-09-03 · qwen: Tancat localment; el tanca formalment el pas per CI.**

- Retirada la interfície morta ::IRng/SeededRng (cap referència al repo i sense definició — Rng.cpp no va existir mai) i els dos adaptadors de RandomEngine.h.
- RPG::RandomEngine queda com a única interfície d'atzar del motor; el comentari de la capçalera registra la retirada.

**Verificat.** La prova que demana el pla: el projecte compila i ctest passa sencer amb la interfície retirada — build completa i suite gl-free en verd (commit 50098f8).

**Pendent.** El borrado físic d'include/RPG/Rng.h viu a l'arbre des de les 12:25 (el va fer una altra sessió, sense commitear); el fitxer ja no té cap referència. Qui commiteu la retirada ha d'incloure aquest borrado, o restaurar-lo.

#### C-04 — Un enemic invocat per narrativa no trenca el guardat

`2 j` · depèn de: C-03

**Per què.** `resolveBattles` materialitza l'enemic sense assignar `spawnIndex` (queda a −1). Si fuges d'una emboscada i guardes, la partida ja no es torna a carregar mai.

**Fitxers.** `src/Game/GameSession.cpp`, `src/Game/SaveGame.cpp`, `tests/Game/SaveGameTests.cpp`

**Fet quan.** Els enemics que no venen del nivell reben identitat pròpia i el guardat els sap distingir dels del mapa. Restaurar una partida amb un enemic invocat viu el torna a col·locar amb la seva vida.

**Ho prova.** `test_save_game`: disparar un beat amb `startBattle`, fugir, guardar, carregar; l'enemic hi és amb la vida que li quedava.

**Seguiment · 2026-09-03 · Claude (Cowork): Tancat.**

- `WorldEnemy` i `SavedEnemy` guanyen `summoned`. Per als invocats, `spawnIndex` deixa de ser un índex a `LevelDefinition::objects` (no n'hi ha cap) i passa a ser un ordinal propi de la sessió, únic, que no es reutilitza.
- Helper nou `GameSession::spawnEnemyFromCatalog`, compartit per `resolveBattles` (l'emboscada) i `restoreSaveGame` (tornar a posar dret un invocat que era viu en desar). Abans només existia dins de `resolveBattles`, i per això restaurar-ne un no era ni possible.
- `restoreSaveGame`: els del nivell es BUSQUEN entre els que la sessió ja ha construït; els invocats no hi són i es materialitzen del catàleg amb la seva posició i la vida que els quedava. La fase de validació els cobreix (si algú ha esborrat el monstre del catàleg, es rebutja amb missatge propi) i segueix sense escriure res fins que tot valida.
- Clau JSON nova amb valor per defecte i SENSE pujar `kSchemaVersion`: un guardat anterior no la porta, es llegeix com a false — «del nivell», que és l'únic que hi havia. És el cas que ja contemplava el comentari de `parseSaveGame`.
- 3 casos nous a `test_save_game`.

**Verificat.** Verds: el cas del pla (disparar un beat amb `startBattle`, fugir, desar, carregar; l'enemic hi és amb 18 PV al seu lloc), que un invocat vençut no torna, i que un guardat sense la clau segueix carregant. Amb el codi antic `restoreSaveGame` retorna false — exactament el símptoma: la partida no es tornava a obrir mai.

#### C-13 — Les entitats del document no perden res

`3 j` · depèn de: C-12

**Per què.** `MapEntityDocument` guarda id, objectId i posició. `ObjectSpawn` guarda a més patrulla, destí, botí, taula de botí, esdeveniment, nom visible, variant, escala, override d'efecte i un mapa de propietats. Migrar un nivell avui perdria gairebé tota la feina d'autoria.

**Fitxers.** `include/Map/EntityLayerDocument.h`, `src/Map/MapDocument.cpp`, `src/Project/ProjectSerializer.cpp`, `tests/Map/MapDocumentTests.cpp`

**Fet quan.** `MapEntityDocument` arriba a la paritat completa amb `ObjectSpawn`, i la serialització hi va i en torna sense pèrdua.

**Ho prova.** `test_map_round_trip` ampliat: nivell amb els onze camps poblats, TMX+JSON → MapDocument → JSON → MapDocument, comparació camp a camp.

**Seguiment · 2026-09-03 · Claude (Cowork): Tancat.**

- `MapEntityDocument` arriba a paritat: patrulla, nivell i cel·la de destí (amb el seu `hasTargetPosition`), botí, taula de botí, esdeveniment, nom visible, variant, escala, override d'efecte i mapa de propietats. El header porta la taula de correspondència camp a camp amb `ObjectSpawn`, perquè qui n'afegeixi un demà sàpiga que ha d'ampliar les dues llistes alhora.
- L'única cosa que `ObjectSpawn` no té és l'id d'instància: en un nivell la identitat és la posició dins l'array, i és exactament per això que un enemic invocat per narrativa es quedava sense identitat (C-04). El document en dona un d'estable des del principi.
- Validació dels camps nous: caixa de patrulla invertida, escala no positiva, entrada de botí buida i propietat sense nom es rebutgen. La patrulla es comprova contra els límits del mapa; `targetPosition` NO, perquè és una cel·la del mapa de destí, que aquest document no coneix.
- `ProjectSerializer` els escriu i els llegeix tots. S'escriuen NOMÉS si difereixen del valor per defecte: cap fitxer de mapa del repositori canvia per recompilar, els diffs segueixen sent llegibles, i segueix sent sense pèrdua perquè el lector restaura aquests mateixos defectes. Les propietats van en `std::map`, o sigui ordre determinista i el mateix fitxer byte a byte en desar dues vegades.
- Al llegir, tot és opcional: un mapa escrit abans d'aquest paquet carrega igual, i la patrulla absent s'omple amb la posició — que és com `LevelLoader` expressa «aquest objecte no patrulla» des de la Fase 6. Cap canvi de `kMapSchemaVersion`: claus additives amb valor per defecte.

**Verificat.** `test_map_round_trip` ampliat a 5 casos: els onze camps poblats seguits per tot el camí real (TMX → MapDocument → JSON a disc → MapDocument) amb comparació camp a camp, més una entitat buida al costat per cobrir també els defectes; desar el que s'acaba de llegir dona els mateixos bytes; un mapa anterior a C-13 carrega amb els defectes correctes; i un document sense camps d'autoria produeix exactament el JSON d'abans. `test_map_document` puja a 8 amb la validació dels camps nous. Amb el serialitzador antic, el round-trip falla al primer camp. Sense regressions: `test_project_serialization`, `test_project`, `test_tmx_importer` i `test_runtime_tilemap` verds.

**Pendent.** Res del paquet. El conversor `ObjectSpawn` ⇄ `MapEntityDocument` que fa comprovable aquesta paritat s'ha escrit tot seguit com a primera llesca de C-14 (`Level/LevelDocumentBridge`), que és on toca: `motor_level` ja depèn de `motor_map_document`, no al revés.

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

**Seguiment · 2026-09-07 · qwen + Codex: Retirada completa implementada i verificada localment; CI pendent.**

- TierRules: retirats els 7 accessors declarats i mai definits (healthCap, healthBonus, allowsMulticlass, canEquipRarity, maxSummons, maxFeats, maxDivineCards); ningú els cridava, per això l'enllaçador no s'havia queixat mai.
- EnhanceHooks: retirats IQteProvider i QteResult (cap implementació; el pas de timing de SkillExecutor era inabastable), el camp ExecutionContext::qte i l'override_target_defense que Ed. 3 ja ignorava.
- SkillExecutor: eliminat el pas QTE mort i renumerats els passos; Skill.cpp deixa d'assignar nullptr al camp retirat.
- tools/comprobar_declaraciones_rpg.py: xarxa estàtica que comprova que cada mètode declarat a include/RPG/ té definició; registrat a CTest com rpg_declaration_check (cmake/DeclarationCheck.cmake), amb un hunk propi a CMakeLists sense canvis d'altres sessions.
- Codex (continuació): retirats resolve_against_cd, resolve_against_defense, defense_to_cd, CD_MAX i CheckOutcome; eliminades les referències dels consumidors i dels comentaris de codi.
- Migrades les cinc demos pendents: dice_engine i nd6_distribution usen enfrentades; narrativa_nd6, prologo i senal_ceniza resolen els requests amb GameSession real mitjançant una fixture sense mapa ni GL.
- Conservades la ruta SUCCESS del pròleg i les quatre rutes de La Senyal de la Cendra, amb guions explícits del jugador i del rival i comprovació del nombre de daus consumits.
- Coberts els quatre graus narratius, la importació de cd legacy com a pool rival, la prioritat de rivalDice explícit, els pools buits i els casos límit de pifia i tot-sisos. Els camps antics de les aventures continuen sent llegibles pel codi de partida.
- Actualitzats només els tres enllaços necessaris a CMake. No s’han modificat GameSession/Application ni el bloc RuntimeTileMap pendent d’integració.
- 07/09 (Codex multiagent): retirats RuntimeTileOverride, setTileOverride, clearTileOverrides, tileOverrides i findLayerIndex; eliminats m_baseLayers i m_overrides. El runtime conserva una sola col·lecció de capes compilades.
- Substituïda la prova de la API retirada per independència entre document i snapshots compilats; cobertes coordenades isomètriques i consultes fora de límits.
- Discrepància documental explícita: ADR-003 (28/08) acceptava overrides; C-22 del pla posterior en demana la retirada. Aquesta entrega executa C-22 encarregat per Oriol el 07/09; no modifica l’ADR acceptat.

**Verificat.** Abans de la retirada, el script reportava exactament els 7 errors que prediu el pla; després, 0 sobre 93 declaracions en 15 capçaleres. · Build completa de tots els targets en macos-debug sense errors (juego i level_editor inclosos). · Suite gl-free 57/57 en verd, inclòs rpg_declaration_check i els tests de la sessió concurrent. · Continuació Codex: en executar el camí real amb els guions antics, prologo i senal_ceniza fallen; després de guionitzar també els daus rivals, les cinc demos passen. · Build completa local de tots els targets, inclosos juego i level_editor: correcta. Suite GL-free: 57/57 proves en verd (macOS Debug). · Comprovador de declaracions RPG: 0 errors sobre 93 declaracions; cerca a include/src/examples/tests: cap referència a la API de dificultat retirada. · Huella SHA-256 dels quatre fitxers reservats GameSession/Application idèntica abans i després. · 07/09: build completa macos-debug (juego i level_editor inclosos), CTest GL-free 62/62, zero errors. · Runtime: compilació aïllada amb -DNDEBUG -O2, 4/4 casos; cap referència als símbols retirats a include/src/tests/examples.

**Pendent.** Validació remota de CI pendent; no s’ha fet push ni executat un workflow remot.

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

**Seguiment · 2026-09-03 · Claude (Cowork): Primera llesca feta; paquet OBERT.**

Fet només el pont de dades, no la migració de l'editor. Es registra aquí perquè és la peça de C-14 que no depèn de GL i que es podia tancar amb prova en aquesta sessió.

- `Level/LevelDocumentBridge` (header + `.cpp` + test): `MakeEntityId`, `ToEntityDocument`, `ToObjectSpawn`, `ToEntityLayer` i `ToObjectSpawns`.
- Els ids d'entitat es deriven de la cel·la (`<objectId>_<x>_<y>`), que és la identitat que el motor ja fa servir per als objectes del món (`worldObjectIndexAt`, el guardat de partida). Són ESTABLES: migrar dues vegades el mateix nivell dona els mateixos ids i un diff llegible. Dos spawns a la mateixa cel·la — el format ho permet — reben sufix `_2`, `_3` en comptes de trepitjar-se: perdre una entitat en migrar és justament el que aquest pont evita.
- Un `objectId` que no és `[a-z0-9_]+` NO s'arregla per les braves: es retorna error. Un id mutilat en silenci és un objecte que deixa de resoldre contra el catàleg i un nivell que es trenca més tard i en un altre lloc. `ToEntityLayer` és tot-o-res.
- Viu a `Level/` i no a `Map/`: el document de mapa no ha de saber que existeix `LevelDefinition`. `motor_level` ja depèn de `motor_map_document`, o sigui que la fletxa va en la direcció que ja hi havia.
- Fora d'abast, i no per descuit: `LevelDefinition::enemies` (el format de la Fase 6 que `ObjectSpawn` ja generalitza) i `name`/`mapPath`/`playerStart`, que encara no tenen lloc dins un `MapDocument`. On viuen és una decisió d'aquest mateix paquet i avançar-la aquí seria inventar format.

**Verificat.** `test_level_document_bridge`, 6 casos verds: els onze camps d'autoria van i tornen idèntics; els ids són estables i derivats de la cel·la; un nivell sencer es converteix en capa sense perdre ningú (inclosa la cel·la compartida) i torna; un `objectId` invàlid falla fort i no deixa res a mitges; una cel·la negativa es rebutja amb el seu missatge; un document sense capes d'entitats dona llista buida i no error. Registrat a `motor_level` i a CTest.

**Pendent (el gruix del paquet).** `EditorState` segueix amb el seu model propi de capes planes i el seu `std::vector<ObjectSpawn>`: convertir-lo en una vista sobre un `MapDocument` i degradar `exportTmx` a exportador d'interoperabilitat és la feina de veritat, i toca `editor/src/level_editor.cpp` (2.708 línies de UI GL). No s'ha començat: la sessió on s'escriu això no pot compilar ni executar res que porti GL, i deixar l'editor a mitges era pitjor que no tocar-lo.

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

**Seguiment · 2026-09-07 · qwen: Mecanisme implementat i verificat localment; paquet OBERT (els 20 catàlegs encara carreguen en mode estructural).**

- Contracte de camps a RpgCatalogs.h: CatalogField (noms acceptats + required), CatalogStrictness (Structural / AllDeclared) i els auxiliars catalog_field_present, catalog_missing_fields, catalog_entry_label, catalog_field_hint i catalog_join.
- Els 20 CatalogLoader<T> declaren expected_fields(): 308 conceptes amb els seus àlies. Les llistes s'han DERIVAT del propi from_json (claus e["..."] i les cadenes de defaults encadenats) amb un generador d'un sol ús; no són una llista escrita a mà, que és la trampa de la Part 3 sobre els validadors manuals.
- Catalog<T>::loadFromString reescrit: fora el catch(...) mut (l'excepció ara es converteix en un Error amb l'entrada i el motiu); fora el `continue` que descartava en silenci una entrada sense id; un camp obligatori absent dona Error amb l'entrada (índex + id), el camp i els àlies acceptats; i la càrrega és tot-o-res amb un buffer local abans de tocar m_store, el mateix criteri que ObjectCatalog::loadFromString.
- Camps obligatoris avui: id i name als 20 carregadors (40 camps). Són els únics que els 21 assets publicats porten al 100% de les 2650 entrades; exigir-ne més hauria trencat la càrrega de tot el joc, que és justament la feina de D-02..D-09.
- test_catalog_contract (7 casos) a tests/RPG/CatalogContractTests.cpp amb TestHarness.h, registrat amb cmake/CatalogContractTests.cmake i una línia d'include al CMakeLists; WORKING_DIRECTORY el directori de build perquè llegeix assets/catalogs/.

**Verificat.** Porta verda abans de tocar res: build completa i suite gl-free 62/62. · Build completa de tots els targets amb el preset macos-debug, codi de sortida 0, amb el treball concurrent de C-16 i C-05 present a l'arbre. · Suite gl-free 63/63 (test_catalog_contract és la prova nova). demo_catalogos_rpg continua carregant les 2510 entrades amb els mateixos recomptes per catàleg i rpg_declaration_check (la xarxa estàtica de C-22) passa. · La prova que demana el pla: el JSON de condicions amb els noms de la versió anterior (stackable/duration/effects) falla en mode AllDeclared amb un missatge que anomena stacking, defaultRounds i damagePerRound i l'entrada cond_sangrado. · Falsabilitat comprovada per mutació: posar name a required=false a ConditionDefinition trenca 3 dels 7 casos; revertit i reverificat a 7/7. · Els 21 assets publicats carreguen en mode estructural i a tots el comptador retornat coincideix amb size(): cap entrada es descarta en silenci pel camí.

**Pendent.** El paquet no es tanca: els 20 catàlegs carreguen en mode estructural, que per als camps no obligatoris és el comportament antic. D-02..D-09 han d'arreglar cada asset i pujar el conjunt de required del seu catàleg. Buit mesurat: 150 dels 308 camps declarats no els porta cap entrada de cap asset, i en mode AllDeclared fallarien les 2650 entrades. · AllDeclared NO és l'estat objectiu: la majoria de camps declarats són legítimament opcionals (channelRounds és 0 a gairebé totes les habilitats, minGold no aplica a una taula de botí sense or). És l'instrument d'auditoria; el que cada D-0x fa és pujar els required del seu catàleg. · Les declaracions s'han derivat amb un generador d'un sol ús que no s'ha versionat. Si un from_json canvia, el seu expected_fields() pot quedar desincronitzat: falta una comprovació estàtica a CTest que els derivi i els compari, que encaixa a D-10 (CI valida els assets reals). · Passar la prova a CI abans de tancar formalment (regla 6 del pla). La línia d'include al CMakeLists.txt queda sense commitear perquè el fitxer porta hunks en curs de C-16 i C-05 d'una altra sessió (regla 2 del protocol). · decisiones/D0003 (proposed): qui decideix quins camps són obligatoris i en quin ordre es migra cada catàleg. Espera l'Oriol.

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

**Fitxers.** `../.github/workflows/ci.yml`, `tools/validar_catalogos.py`, `tests/RPG/CatalogContentTests.cpp`

**Fet quan.** CI carrega els catàlegs reals d'`assets/` a cada commit i falla si una entrada perd camps mecànics respecte de la línia base registrada.

**Ho prova.** El job de CI falla si es torna a introduir a mà el desaparellament de `saveAttribute`/`defenseStat`.

### Fase E — Autoria visual i esdeveniments

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

#### E-10 — Espai de treball visual i controls coherents

`6 j` · depèn de: C-14, C-16

**Per què.** El shell ImGui només està compromès a ADR-001; els formularis nous no tenen una base visual integrada. El verificador de controls apunta a una ruta antiga.

**Fitxers.** `editor/src/EditorApplication.cpp`, `editor/src/level_editor.cpp`, `third_party/imgui/`, `CMakeLists.txt`, `tools/verificar_controles.py`

**Fet quan.** Aplicar ADR-001: ImGui privat del motor_editor, un viewport amb docking, mapa central, menús i eines visibles. Ratolí i teclat invoquen les mateixes accions. Escriure en un camp no pinta el mapa ni dispara dreceres. Hi ha indicador de canvis i Desar/Descartar/Cancel·lar abans de tancar o canviar de document. El HUD del jugador conserva la seva tecnologia.

**Ho prova.** `test_editor_actions`: cancel·lar conserva el document; guardar fallit no el tanca; una tecla en un input no pinta. Smoke de UI a CI amb obrir, editar i reobrir; captura llegible a 1280×720 i DPI 2×; `verificar_controles.py` passa sobre editor/src.

#### E-03 — Planificador, esperes i processos paral·lels

`4 j` · depèn de: E-02

**Per què.** Un beat s'executa sencer i de cop. Sense esperes no hi ha ritme narratiu: ni una porta que s'obre a poc a poc, ni un NPC que camina mentre parles.

**Fitxers.** `include/RPG/EventInterpreter.h`, `src/RPG/EventInterpreter.cpp`

**Fet quan.** L'intèrpret executa comandes al llarg de diversos frames, amb esperes per temps i per condició, i admet processos paral·lels que no bloquegen el jugador.

**Ho prova.** `test_event_interpreter`: seqüència amb dues esperes; el nombre de ticks fins al final és determinista.

#### E-11 — Projectes i arbre de mapes

`5 j` · depèn de: E-10, C-11

**Per què.** ProjectHub ja crea i obre; el recorregut continua depenent de noms tècnics i de canviar de mapa amb anterior/següent.

**Fitxers.** `editor/src/ProjectPanel.cpp`, `include/Editor/ProjectHub.h`, `include/Project/ProjectIndex.h`, `src/Project/ProjectSerializer.cpp`

**Fet quan.** Reutilitzar ProjectHub/ProjectIndex en un assistent de nom, carpeta i plantilla; IDs generats i noms visibles separats. Arbre amb crear, obrir, duplicar i canviar nom visible de mapa. Triar mapa/casella inicials i títol del joc. Duplicar remapeja IDs locals; no sobreescriu altres projectes. Cancel·lar o fallar una operació no deixa un projecte mig registrat.

**Ho prova.** `test_project_panel`: dos projectes en carpetes amb espais, duplicació sense col·lisions i cancel·lació sense fitxers parcials. Recorregut UI: crear projecte, dos mapes, triar entrada, desar i reobrir.

#### E-04 — El joc de comandes

`7 j` · depèn de: E-03, D-09

**Per què.** Les comandes que el contingut necessita per no dependre de C++: diàleg amb eleccions, canvis d'estat i inventari, moviment, càmera, teletransport, branques, bucles i esdeveniments comuns.

**Fitxers.** `src/RPG/EventCommands.cpp`, `include/RPG/EventCommand.h`, `src/Game/GameSession.cpp`

**Fet quan.** Les comandes de les cinc famílies estan implementades i documentades a `FORMATO_AVENTURAS.md`, amb el seu efecte sobre la sessió.

**Ho prova.** `test_event_commands`: una comanda de cada família, amb el seu efecte observable sobre una `GameSession` de prova.

#### E-12 — Propietats de mapa, capes i instàncies

`4 j` · depèn de: E-11, C-13

**Per què.** Tenir camps al document no equival a poder-los editar: falten el formulari de mapa i controls complets de capes. ObjectInspector ja resol part de les instàncies.

**Fitxers.** `editor/src/MapPropertiesPanel.cpp`, `editor/src/LayerPanel.cpp`, `include/Editor/ObjectInspector.h`, `src/Editor/EditorState.cpp`

**Fet quan.** Editar nom, dimensions, tileset, entrada i opcions de llum/boira ja suportades. Capes: crear, renombrar, ordenar, mostrar/ocultar i bloquejar. Reduir dimensions avisa de les entitats afectades i permet cancel·lar. L'inspector manté nom, escala, variant, botí, patrulla i sortida; tot passa per MapDocument i l'historial. Opacitat arbitrària queda per a 1.1.

**Ho prova.** `test_map_properties`: canvi/desfer/desar/reobrir conserva ordre, visibilitat i entitats; una capa bloquejada no es pinta. UI: configurar una porta i seguir-ne el destí, cancel·lar una reducció destructiva.

#### E-13 — Navegador visual i importació de recursos

`4 j` · depèn de: E-11

**Per què.** Les llistes amb cerca ajuden a trobar IDs, però no permeten incorporar un sprite propi des de la mateixa eina.

**Fitxers.** `editor/src/AssetBrowserPanel.cpp`, `include/Editor/AssetBrowserState.h`, `src/Project/ProjectSerializer.cpp`

**Fet quan.** Cerca per nom/tipus, miniatures de tiles i sprites i previsualització de l'element seleccionat. Importar PNG i spritesheets compatibles cap al projecte amb selector de fitxer, revisió de mides/frames i avís de col·lisió; reutilitzar o cancel·lar sense sobreescriure. Es desa una referència relativa estable. No inclou autotiles ni editor gràfic.

**Ho prova.** `test_asset_browser`: importar un PNG vàlid, rebutjar format invàlid i nom duplicat sense perdre l'original. Moure la carpeta del projecte i reobrir conserva el recurs; smoke UI de miniatura i col·locació.

#### E-14 — Selectors de dades i referències

`3 j` · depèn de: E-13, D-10

**Per què.** Un camp eventId o skillId lliure trasllada a l'autor el problema de conèixer i escriure els identificadors interns.

**Fitxers.** `editor/src/CatalogPicker.cpp`, `include/Editor/CatalogPickerState.h`, `include/Editor/ObjectInspector.h`, `include/Editor/CommandListState.h`

**Fet quan.** Selectors amb nom, tipus, cerca, descripció i ID intern per a objectes, habilitats, enemics, condicions, mapes i esdeveniments. Integrats als inspectors actuals; E-05/06 els incorporen als formularis d'esdeveniment. Amb Obrir definició i avisos de referències inexistents. Catàleg importat de Onegai en lectura; només referències del tipus correcte, sense haver d'escriure IDs per completar el tutorial.

**Ho prova.** `test_catalog_picker`: noms iguals amb IDs diferents, filtre entre mil entrades, tipus incompatible rebutjat i referència perduda localitzable. UI: triar botí i destí d'una porta pel nom.

#### E-05 — Editor de pàgines d'esdeveniment

`8 j` · depèn de: E-01, C-14, E-12, E-14

**Per què.** És la primera meitat del que fa que això sigui un creador de jocs i no un motor: poder crear un esdeveniment sobre una casella i donar-li pàgines amb condicions.

**Fitxers.** `editor/src/event_editor.cpp`, `include/Editor/EventEditorState.h`, `tests/Editor/EventEditorTests.cpp`

**Fet quan.** Des de l'editor es crea un esdeveniment sobre una casella, se li afegeixen pàgines, i cada pàgina té disparador, condicions i aparença. Tot GL-free sota `EventEditorState`.

**Ho prova.** `test_event_editor`: crear, duplicar, reordenar i esborrar pàgines; desfer restaura l'estat anterior.

#### E-16 — Edició RPG mínima del projecte

`8 j` · depèn de: E-14, D-02, D-03, D-05

**Per què.** Llegir els 21 catàlegs no deixa crear una habilitat o un enemic des de l'editor. És el buit crític de base de dades que aquesta revisió incorpora.

**Fitxers.** `editor/src/ProjectDatabasePanel.cpp`, `include/Editor/ProjectDatabaseState.h`, `src/Project/ProjectSerializer.cpp`, `include/RPG/Catalogs/RpgCatalogs.h`, `FORMATO_PROYECTOS.md`

**Fet quan.** Formularis mínims per a objectes/consumibles, equip, habilitats i enemics: crear, duplicar a projecte, editar, validar i desar els camps mecànics suportats. Assignar habilitats a la configuració del personatge/enemic. Imports canònics de lectura; les còpies locals tenen IDs de projecte, origen explícit i s'exporten amb ell. Cap escriptura a cartes/canon ni regles noves de Nd6. La resta de famílies es consulten amb E-14; editar les 21 famílies, lots i dissenyar fórmules es manté a 1.1.

**Ho prova.** `test_project_database`: duplicar una habilitat, ajustar un camp admès i assignar-la; desar/reobrir conserva la còpia. G-15/16 verifiquen la còpia dins de l'export final. Import immutable per hash, IDs sense col·lisions i referències invàlides rebutjades sense desar a mitges. G-13 prova que apareix i funciona al combat.

#### E-06 — Editor vertical de comandes

`9 j` · depèn de: E-04, E-05

**Per què.** La segona meitat: la llista de comandes d'una pàgina, editable, amb inserció, sagnat de les branques i moviment.

**Fitxers.** `editor/src/event_editor.cpp`, `include/Editor/CommandListState.h`

**Fet quan.** La llista de comandes s'edita amb teclat: inserir, esborrar, moure, entrar en una branca. Les comandes amb paràmetres obren el seu formulari.

**Ho prova.** `test_command_list`: construir una seqüència amb branca i bucle des de l'API de l'editor i executar-la a l'intèrpret.

#### E-07 — Validació estàtica d'aventures

`4 j` · depèn de: E-04, E-16

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

#### E-15 — Problemes i cerca d'esdeveniments

`3 j` · depèn de: E-07, E-06

**Per què.** Els diagnòstics necessiten una pantalla que porti al lloc a corregir; un missatge de consola no tanca el recorregut d'autor.

**Fitxers.** `editor/src/ProblemsPanel.cpp`, `include/Editor/ProblemsState.h`

**Fet quan.** Llista de problemes amb severitat, mapa, entitat/pàgina/comanda i acció Anar-hi. Cercar esdeveniments per nom i usos de variables/interruptors. Errors bloquegen playtest/export; es pot desar feina incompleta amb diagnòstics visibles. Actualitzar un diagnòstic conserva el focus quan existeix.

**Ho prova.** `test_problems_panel`: els sis defectes d'E-07 obren el lloc correcte; corregir-los buida la llista i habilita provar. Una referència esborrada no provoca navegació a una entitat diferent.

#### E-09 — La missió de la clau, la porta i el cofre

`3 j` · depèn de: E-07, E-08, E-15

**Per què.** La prova d'acceptació de tota la fase: la missió canònica de qualsevol creador de jocs, escrita sense tocar codi.

**Fitxers.** `assets/adventures/mision_llave.json`, `assets/levels/`

**Fet quan.** Des d'un projecte nou, la missió s'escriu sencera des de l'editor: cofre amb clau, porta tancada que la demana, NPC que en parla, i final. Es desa i reobre abans de provar-la; cap pas exigeix editar JSON o escriure un ID.

**Ho prova.** `demo_mision_llave` a CTest: recorre la missió sencera de manera determinista i comprova l'estat final.

### Fase F — Playtest des de l'editor

#### F-01 — Playtest des de l'editor

`4 j` · depèn de: C-16, E-09

**Per què.** Avui P desa el nivell i llança `./juego` en un procés separat, bloquejant l'editor. Ja prova els canvis acabats de desar; falta provar el document en memòria sense escriure'l i recuperar l'estat d'edició.

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

#### G-01 — Autosave i recuperació

`2 j` · depèn de: C-15, E-10

**Per què.** Perdre feina d'autoria per un tancament inesperat és el defecte que fa que una eina no es torni a obrir.

**Fitxers.** `src/Editor/EditorState.cpp`, `editor/src/level_editor.cpp`

**Fet quan.** El document es desa sol cada N canvis en un fitxer de recuperació. En obrir, si n'hi ha un de més nou que el desat, l'editor ho ofereix.

**Ho prova.** `test_autosave`: simular una caiguda amb canvis pendents i recuperar-los.

#### G-08 — Títol i arrencada del projecte

`4 j` · depèn de: C-16, E-11

**Per què.** juego arrenca en un nivell fix; exportar una altra història no garanteix que s'obri amb el seu nom, entrada i configuració.

**Fitxers.** `src/Engine/Scene/TitleScene.cpp`, `examples/juego.cpp`, `include/Game/TitleMenuState.h`, `src/Engine/Application.cpp`

**Fet quan.** Arrencar des del manifest del projecte; mostrar nom/imatge disponibles i Nova partida, Continuar, Opcions i Sortir. Continuar desactivat amb motiu si no hi ha guardat vàlid; G-11 connecta el selector. Dades absents mostren un error comprensible. Playtest contextual continua saltant el títol.

**Ho prova.** `test_title_flow`: dos projectes amb noms i entrades diferents; manifest invàlid no cau en Boundington. Smoke del runtime amb manifest mostra el títol i les accions reals; G-15/16 repeteixen el recorregut sobre el binari exportat.

**Seguiment · 2026-09-06 · qwen: Rodaja inicial feta i verificada (encargo directo de Oriol); paquet obert.**

- Pantalla de títol a ./juego sense arguments: campanya de Boundington + escenaris en treball (projectes jugables de l'índex) + Sortir; amb --nivel/--precuela s'entra directe com abans (editor F7, proves).
- Game::TitleScreen amb tot l'estat GL-free (convenció ProjectHub): navegació amb wraparound, skipped() amb els projectes sense entrada jugable i etiquetes des del nom del manifest.
- Application: m_atTitle congela update(), reenvia processInput() (W/S+ENTER com a combat/botiga) i canvia el HUD a refreshHud(); OpenProject carrega catàlegs del projecte, narrativa si l'entrada és aventura (cronologia Boundington apagada, com --precuela) i loadLevel sobre l'entrada resolta; un fallada deixa missatge al títol.
- test_title_screen (5 casos) registrat com a CTest gl-free (cmake/TitleScreenTests.cmake); TitleScreen.cpp a motor_core. Hunk de CMakeLists exclusiu, sense tocar el treball C-14/auditoria en curs d'una altra sessió.

**Verificat.** Build completa de tots els targets en macos-debug sense errors. · Suite gl-free 62/62 en verd (inclou test_title_screen i el treball concurrent). · Fum ./juego 3 amb bolcat de frame: títol i menú dibuixats (1156 px daurats del títol, 2040 px del cursor).

**Pendent.** Resta del paquet segons el pla: arrencada des del manifest del projecte actiu (no només l'índex global), Nova partida/Continuar/Opcions amb els seus motius, imatge del projecte quan n'hi hagi; depèn de C-16 i E-11. · G-09 (creació de personatge dins de Nova partida) i G-11 (selector de guardats) connectaran amb aquesta pantalla. · Passar la prova a CI abans de tancar formalment (regla 6 del pla).

#### G-02 — Les eines d'edició que falten

`3 j` · depèn de: C-14, E-12

**Per què.** L'editor té pinzell, esborrar, omplir, col·locar i enllaçar. Li falten comptagotes, rectangle i línia — les tres que qualsevol autor busca la primera hora.

**Fitxers.** `include/Editor/EditorState.h`, `src/Editor/EditorState.cpp`, `editor/src/level_editor.cpp`

**Fet quan.** Comptagotes, rectangle i línia, cadascuna com una sola operació d'historial.

**Ho prova.** `test_editor_state`: cada eina nova, amb el seu desfer.

#### G-10 — Menú de partida, opcions i focus

`3 j` · depèn de: G-08

**Per què.** Escape surt de cop i les accions de partida només viuen en dreceres poc visibles.

**Fitxers.** `src/Engine/Scene/PauseScene.cpp`, `include/Game/PauseMenuState.h`, `src/Engine/Application.cpp`

**Fet quan.** Escape obre el menú amb Reprendre, Personatge, Inventari/Equip, Desar, Carregar, Opcions, Tornar al títol i Sortir, connectats en G-11/12. Confirmació per abandonar progrés. Opcions mínimes: finestra/pantalla completa, escala de text i ajuda de controls persistents per usuari. El món i temporitzadors no avancen pausats; el focus no filtra tecles.

**Ho prova.** `test_pause_flow`: pausa no avança temps, reprèn sense moure el jugador, cancel·lar sortida conserva sessió. Smoke amb teclat i ratolí verifica navegació i llegibilitat després de canviar escala.

#### G-03 — Historial per comandes

`5 j` · depèn de: C-14, G-02

**Per què.** L'historial copia el mapa sencer a cada pinzellada, amb un límit de 256 instantànies i un descart O(n) d'instantànies completes. És el paquet ja marcat REPLANTEAR al Gantt: amb mapes grans, l'editor no s'aguanta.

**Fitxers.** `include/Editor/EditorState.h`, `src/Editor/EditorState.cpp`

**Fet quan.** L'historial guarda comandes reversibles, no fotografies. Un mapa de 200×200 amb quatre capes admet mil operacions sense creixement notable de memòria.

**Ho prova.** `test_editor_state`: mil operacions barrejades; desfer-les totes retorna al document inicial i la memòria es manté sota un límit.

#### G-14 — HUD, diàlegs i comerç amb accions visibles

`3 j` · depèn de: G-10, E-04, D-05

**Per què.** Exploració, diàleg, botiga i negocis ja funcionen; cal tancar el feedback i controls, inclosa la barra de temporitzador desconnectada de F1.

**Fitxers.** `src/Engine/Application.cpp`, `src/Game/GameSession.cpp`, `src/Render/HudWidgets.cpp`, `examples/demo_timers_partida.cpp`

**Fet quan.** Mantenir PV/PM, minimapa i moral; temporitzadors visibles segons Contador/Barra/Secret i temps actiu. Diàlegs i eleccions navegables amb ratolí/teclat; botiga mostra preu, quantitat i resultat, negoci compra/renda i impacte moral. Errors expliquen manca d'or/requisits. Coordinar amb F1 del TABLERO i reutilitzar-ne el resultat, sense una segona implementació.

**Ho prova.** `test_runtime_feedback`: congelació de timer en pausa/diàleg/combat, visualització de l'estat vigent, compra/venda i renda sense doble acció. Captures de textos llargs i smoke de cada modal amb retorn a exploració.

#### G-09 — Creació de personatge dins de Nova partida

`3 j` · depèn de: G-08, D-06, E-16

**Per què.** CharacterCreationScreen té catorze passos però cap ruta del joc l'obre.

**Fitxers.** `src/Engine/Scene/CharacterCreationScene.cpp`, `include/RPG/CharacterCreationScreen.h`, `examples/creacion_personaje.cpp`, `src/Engine/Application.cpp`

**Fet quan.** Nova partida obre l'assistent existent, amb tornar/cancel·lar i resum. Confirmar crea la sessió real amb fitxa, trets, equip i habilitats del projecte; continuar una partida no el repeteix. Sense fitxers intermedis que el jugador hagi de copiar a mà.

**Ho prova.** `test_new_game_flow`: personatge de dues combinacions diferents arriba al nivell amb els valors esperats. Cancel·lar torna al títol sense partida parcial. Smoke recorre els controls de l'assistent integrat.

#### G-11 — Selector de guardats i recuperació de derrota

`4 j` · depèn de: G-09, G-10, C-24

**Per què.** F5/F9 només són guardat ràpid; no hi ha slots visibles, Continuar ni camí de recuperació des de Game Over.

**Fitxers.** `include/Game/SaveSlotState.h`, `src/Game/SaveGame.cpp`, `src/Engine/Scene/SaveLoadScene.cpp`, `src/Engine/Scene/GameOverScene.cpp`

**Fet quan.** Slots separats per projecte amb personatge, mapa i data; seleccionar, desar amb confirmació de sobreescriptura i carregar. Corruptes/incompatibles mostren motiu i no destrueixen la sessió. Derrota ofereix Carregar o Tornar al títol; Continuar usa el mateix selector. F5/F9 reutilitzen el servei.

**Ho prova.** `test_save_slots`: dos projectes no comparteixen slots; corrupció i cancel·lació preserven dades. UI: guardar, tancar procés, continuar i recuperar derrota, comprovant fitxa, equip, inventari, moral i flags.

#### G-13 — Comandes de combat i objectius des de dades

`5 j` · depèn de: G-09, D-02, D-09, E-16

**Per què.** El menú Atacar/Tajo/Cura/Pocion/Huir és fix i envia objectiu 0; les habilitats configurades pel projecte no apareixen automàticament.

**Fitxers.** `include/Game/BattleMenuState.h`, `src/Engine/Application.cpp`, `src/Game/BattleState.cpp`

**Fet quan.** Construir accions des de fitxa i inventari; mostrar nom, cost, disponibilitat i motiu de bloqueig. Triar objectius vàlids segons abast i bàndol, amb cancel·lació anterior a gastar recursos. Mostrar resultat Nd6, consum i condicions coherents amb la sessió. Cap taula de noms d'habilitats al HUD.

**Ho prova.** `test_battle_menu`: habilitat amb ID nou creada en E-16 apareix sense recompilar, es tria el segon enemic i només rep ell l'efecte; Cura arriba a un aliat. Cancel·lar i manca de PM no gasten torn ni recursos. Smoke pel menú real, amb cost i resultat visibles.

#### G-12 — Inventari, equip i fitxa utilitzables

`5 j` · depèn de: G-11, D-05

**Per què.** El panell TAB enumera objectes; el motor d'inventari no té una pantalla des d'on consumir, equipar o consultar les conseqüències.

**Fitxers.** `include/Game/InventoryMenuState.h`, `src/Engine/Scene/InventoryScene.cpp`, `src/RPG/InventoryEngine.cpp`, `src/Game/GameSession.cpp`

**Fet quan.** Llistes per categories, quantitats, descripció i accions Usar/Equipar/Desequipar; triar destinatari quan pertoqui. Fitxa amb estadístiques, trets i condicions; previsualitzar canvis d'equip. Accions impossibles expliquen el motiu i no gasten res. Reutilitzar les regles existents; els objectes de missió no es poden perdre amb una acció de gestió.

**Ho prova.** `test_inventory_menu`: consumir una poció una vegada, equipar arma de dues mans sense duplicar bonus, cancel·lar objectiu sense consum. UI: veure el canvi de PV/stats i retrobar-lo després de guardar/carregar.

#### G-04 — Joc d'exemple de trenta minuts

`5 j` · depèn de: E-09, F-03, G-01, G-03, G-11, G-12, G-13, G-14

**Per què.** La prova que el corte B és real: un joc petit fet només amb l'eina.

**Fitxers.** `assets/proyectos/ejemplo/`, `assets/levels/`, `assets/adventures/`

**Fet quan.** Un joc de trenta minuts amb tres mapes, sis esdeveniments, combat i final, creat des de les pantalles. Inclou nova partida, personatge, menú de pausa, inventari/equip, habilitat local, compra/venda, guardat i derrota.

**Ho prova.** `demo_juego_ejemplo` recorre la campanya de manera determinista. Smoke de la UI real: crear personatge, equipar, combatre, guardar, tancar, continuar i recuperar una derrota; s'adjunten captures.

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

**Fet quan.** Els esquemes de projecte, mapa, aventura i guardat es congelen a la versió 1.0-B, inclosos configuració de títol, dades RPG locals i metadades de slots, documentats, amb ruta de migració per a cada canvi futur.

**Ho prova.** `test_schema_freeze`: fixtures de la versió 1.0-B es carreguen sense avisos amb el codi del dia.

#### G-15 — Assistent d'exportació des de l'editor

`3 j` · depèn de: G-06, E-11, E-15

**Per què.** Un script d'empaquetat no tanca la pantalla Exportar que necessita un tercer.

**Fitxers.** `editor/src/ExportPanel.cpp`, `include/Editor/ExportState.h`, `tools/exportar.py`

**Fet quan.** Triar projecte, plataforma Linux/Windows disponible i carpeta de sortida; validar, mostrar progrés/errors i obrir la carpeta del resultat. El motor d'exportació de G-06 s'invoca una sola vegada. Si l'host no té el runtime objectiu disponible, s'explica i no es presenta com a exportació correcta. No cal terminal ni tenir el repositori per jugar el resultat.

**Ho prova.** `test_export_panel`: cancel·lar o faltar runtime no publica un paquet parcial; error d'E-15 navega al problema. UI exporta i arrenca el resultat en cada runner Linux/Windows des d'una carpeta externa al build.

#### G-16 — Acceptació de pantalles d'autor i jugador

`3 j` · depèn de: G-05, G-07, G-15, F-02, F-04

**Per què.** Una suite GL-free verda no comprova que els botons siguin accessibles ni que el destinatari pugui acabar un recorregut sense ajuda.

**Fitxers.** `tests/UI/`, `docs/acceptance/corte-b-pantalles.md`, `../.github/workflows/ci.yml`

**Fet quan.** Prova amb autor nou: projecte, mapa, recurs, habilitat local, NPC/missió, diagnòstic, desar/reobrir, playtest i exportar, sense C++/JSON/terminal. Un altre jugador obre l'export: Nova partida, personatge, diàleg, equip, combat, botiga, desar, reiniciar, continuar i recuperar derrota. Cap control inert, text essencial tallat o error sense sortida. Registrar plataforma, passos, captures, incidències i resultat; tanca G.

**Ho prova.** CI executa smoke dels recorreguts de les pantalles reals i proves dels seus efectes sobre GameSession/MapDocument. Acta de prova humana amb teclat/ratolí a 1280×720 i DPI 2×; bloquejos oberts impedeixen tancar el corte.

## 5. Fora d'abast, per escrit

El corte B es defineix tant pel que inclou com pel que no. Això queda per a 1.1, i està aquí perquè ningú (jo inclòs) ho coli sense adonar-se'n.

| Queda fora | Paquets del Gantt vell | Motiu |
| --- | --- | --- |
| Autotiles i terrenys intel·ligents | MAP-03 | Molta feina d'editor per a un resultat que es pot pintar a mà en mapes de la mida que fem ara. |
| Sistema de plugins (runtime i editor) | TOOL-06..08 | No té sentit obrir una API d'extensió abans que l'API interna estigui estable. Depèn de la fase C. |
| Àudio, música i ambient | GDB-11 | Cap peça del motor el toca avui; és una vertical sencera i no bloqueja el cicle d'autor. |
| Edició avançada dels 21 catàlegs, lots i fórmules | GDB-01..12 (resta) | E-14 consulta els imports i E-16 edita les quatre famílies mínimes locals; el manteniment complet de regles i canon continua aigües amunt. |
| Exportació a macOS i empaquetat de botiga | TOOL-10 | Linux i Windows cobreixen la prova del corte B. |
| Combat tàctic avançat i IA de grup | — | El combat per torns actual és suficient per validar el cicle d'autor. |
| Generador gràfic de personatges, editor visual de HUD i diari avançat | GDB-08/12 (resta) | El corte B connecta personatge, HUD i missions; no inclou aquestes eines noves. El generador gràfic no és l'assistent de fitxa de G-09. |
| Opacitat de capes, autotiling avançat i borrat massiu de projectes | EDT/MAP (resta) | E-11/12 cobreixen crear, navegar, duplicar i ordenar; aquestes funcions de productivitat no són condició dels recorreguts mínims. |

## 6. Regles de treball

- **Un paquet no es comença sense les seves dependències tancades.** L'ordre topològic del punt 4 ja el respecta.
- **Un paquet es tanca quan la seva prova passa a CI**, no quan el codi compila a la màquina de qui l'ha escrit.
- **Cap paquet nou entra al pla sense criteri d'acceptació i prova.** Si no es poden escriure, el que falta és entendre el problema, no programar-lo.
- **Si un paquet supera el doble de la seva estimació, es para i es replanteja**, en comptes d'anar-hi afegint jornades.
- **Les fases no s'encavalquen.** Cada fase s'obre quan la anterior ha tancat: és el que vol dir «consolidació abans de res més». Al Gantt això són les portes de fase.
- **La fase C no admet funcions noves.** Qualsevol idea que aparegui durant la consolidació es registra per després del seu tancament. Les pantalles autoritzades en aquesta revisió viuen a E/G; la resta segueix a 1.1.
- **Una pantalla no es tanca només amb tests del model.** Cal exercitar el control visible, comprovar l'efecte real i registrar el recorregut de retorn/error. G-16 és la porta final d'acceptació amb persones noves.

## 7. Cobertura de pantalles i funcionalitats

Fotografia de la revisió del 03/09/2026; no és un percentatge ni una declaració de feina acabada. Els IDs són compromisos d'execució. El seguiment de cada paquet continua al seu registre.

| Àmbit / pantalla | Fotografia de partida | Abast / paquets | Recorregut que ho tanca |
| --- | --- | --- | --- |
| Editor · Inici i projectes | Hub existent; assistent incomplet | Corte B: E-11, G-16 | Crear en una carpeta triada, obrir, desar i reobrir dos projectes independents. |
| Editor · Espai de treball i controls | HUD propi; ADR ImGui pendent | Corte B: E-10, G-16 | Accions visibles amb ratolí i teclat; focus, text i canvi de document sense pèrdues. |
| Editor · Arbre de mapes | Navegació anterior/següent | Corte B: E-11 | Crear, duplicar, triar entrada i obrir un mapa pel nom. |
| Editor · Edició de mapa | Pintura, emplenat, selecció i historial existents | Corte B: C-14, E-12, G-02, G-03 | Pintar rectangle/línia, mostrejar, desfer, desar i reobrir. |
| Editor · Capes i propietats del mapa | Dades parcials sense panell complet | Corte B: E-12 | Dimensions, tileset, ordre, visibilitat i bloqueig; cancel·lar canvis que eliminen entitats. |
| Editor · Paleta i recursos | Llistes filtrades; falta importació guiada | Corte B: E-13 | Importar un sprite, veure miniatura, col·locar-lo i moure el projecte sense perdre'l. |
| Editor · Propietats d'instància | Inspector existent amb camps i pickers | Corte B: E-12, E-14, E-05 | Triar botí/destí pel nom i obrir l'esdeveniment associat. |
| Editor · Pàgines d'esdeveniment | No integrades | Corte B: E-01, E-02, E-05, E-09 | Cofre amb pàgina tancada/oberta i interruptor local, sense escriure IDs. |
| Editor · Comandes d'esdeveniment | No integrades | Corte B: E-03, E-04, E-06, E-09 | Diàleg, elecció, condició, moviment, espera i final construïts amb formularis. |
| Editor · Consulta i edició RPG mínima | Catàlegs llegits; sense CRUD visual | Corte B: D-10, E-14, E-16, G-13 | Crear còpia local d'habilitat, assignar-la i usar-la al combat sense recompilar. |
| Editor · Problemes i cerca d'esdeveniments | Validació parcial; no navegació des de diagnòstics | Corte B: E-07, E-15 | Anar de l'error a la comanda, corregir-lo i desbloquejar provar/exportar. |
| Editor · Playtest i depuració | P desa i llança procés separat | Corte B: E-08, F-01, F-02, F-03, F-04 | Provar des d'una casella, inspeccionar, fer passos i tornar al mateix document. |
| Editor · Exportació | Empaquetat existent; falta assistent i garanties standalone | Corte B: G-06, G-15, G-16 | Exportar des de pantalla i executar fora del repositori en Linux i Windows. |
| Editor · Ajuda i recuperació | Sense tutorial ni recuperació automàtica | Corte B: E-10, G-01, G-05, G-16 | Autor nou acaba el tutorial i recupera canvis després d'una caiguda simulada. |
| Jugador · Títol | Arrencada directa al nivell | Corte B: G-08, G-11 | Nova partida, Continuar, Opcions i Sortir corresponen al projecte exportat. |
| Jugador · Creació de personatge | Assistent de 14 passos separat del joc | Corte B: D-06, G-09 | La fitxa confirmada arriba a la partida; cancel·lar torna al títol. |
| Jugador · Menú durant la partida | Escape surt; accions en dreceres | Corte B: G-10, G-11, G-12 | Pausar, gestionar, guardar/carregar i tornar sense tecles filtrades ni avanç de temps. |
| Jugador · Inventari, equip i fitxa | Panell informatiu; regles sense accions de menú | Corte B: D-05, G-12 | Consumir, equipar i desequipar amb canvi observable de PV/stats i persistència. |
| Jugador · Combat i selecció d'objectiu | Cinc comandes i objectiu fixos | Corte B: G-13 | Habilitat de projecte al menú, cost correcte, segon enemic seleccionable i cancel·lació. |
| Jugador · Exploració, moral i temporitzadors | HUD existent; F1 amb connexió pendent | Corte B: G-10, G-14 | Informació visible, timer pausat quan toca, minimapa viu després de travessar portes. |
| Jugador · Diàleg i eleccions | Motor i modal existents | Corte B: E-06, E-09, G-14 | NPC creat a l'editor mostra eleccions i canvia de resposta segons estat. |
| Jugador · Botiga i negocis | Compra/venda i rendes existents | Corte B: G-14 | Preu, quantitat, error i resultat visibles; efectes en or/inventari/moral sense duplicats. |
| Jugador · Desar i carregar | F5/F9 en exploració; sense slots visibles | Corte B: G-11 | Desar, sortir del procés i continuar; corruptes no esborren la partida actual. |
| Jugador · Derrota | Avís sense recorregut de recuperació | Corte B: G-11, G-16 | Carregar un slot vàlid o tornar al títol amb controls visibles. |
| Jugador · Opcions i ajuda de controls | Sense pantalla pròpia | Corte B: G-10 | Canviar mode de finestra/escala de text i consultar controls; preferències persistents. |
| Editor · Base de dades avançada de 21 famílies | Fora del mínim revisat | 1.1: fora del corte | E-16 només cobreix objectes, equip, habilitats i enemics locals; la resta es consulta. |
| Editor · Autotiles, plugins i generador gràfic | No integrats | 1.1: fora del corte | Ampliacions futures; no es confonen amb G-09 ni amb la importació de PNG. |
| Editor/Jugador · Àudio i exportació macOS/botigues | Aplaçats expressament | 1.1: fora del corte | Exclusió mantinguda: no prometre música, controls de volum ni export natiu macOS al corte B. |

## 8. Ordre d'execució i correspondència amb plans previs

C → D → E → F → G. Cada arrel de fase espera tots els terminals de l'anterior, no només l'últim ID. La revisió manté els IDs existents. E-10..14 concreten ADR-001/EDT-01..06 i els inspectors; E-15 connecta diagnòstics; E-16 avança el mínim de GDB a dades locals de projecte. G-08..14 connecten pantalles del jugador; G-15 embolcalla G-06; G-16 tanca el recorregut complet.

La pantalla F1 de temporitzadors del TABLERO conserva el seu propietari: G-14 integra el resultat, no crea una tasca rival. Els roadmaps F0–F8/EDT/GDB i les auditories anteriors són referències de procedència; no s'hi sumen jornades per segona vegada. Aquesta revisió només actualitza el Corte B; no resol d'amagat la numeració històrica pendent al TABLERO.
