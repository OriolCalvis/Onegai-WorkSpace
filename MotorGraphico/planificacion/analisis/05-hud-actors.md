## Mòdul HUD i actors

Una pantalla d'HUD es composa apilant `IHudElement*` dins d'un `HudManager` per ordre d'inserció (el z-order és literalment aquest ordre): el manager fixa una projecció ortogràfica en píxels de pantalla amb origen a dalt-esquerra sobre el mateix shader `sprite` del món, crida `update()` a **tots** els elements (visibles o no) i després `render()` només als visibles, passant-los la `SpriteBatch` que posseeix el cridant. Cada widget resol la seva posició amb un `HudTransform` (ancoratge de 9 punts + offset cap a l'interior + mida), aritmètica pura i testejable sense GL, i dibuixa quads sòlids amb una textura blanca 1x1 aprofitant el tint per vèrtex, o glifs amb l'atles procedural de `BitmapFont`. La jerarquia de widgets és plana i d'un sol nivell: `HudBar`, `HudPanel`, `HudText`, `HudCommandMenu`, `HudDialogueBox` i `HudMinimap` deriven directament d'`IHudElement`, i només `HudBar` sobreescriu `update()`; cap widget posseeix la font, la textura ni les dades que llegeix. `Player` i `Enemy` són les dues cares "de render" del combat: totes dues hereten de la base concreta `AnimatedEntity` i de la interfície `ICombatant`, però `Player` guarda dades pròpies (o delega en un `RPG::CharacterSheet*` extern) mentre `Enemy` delega el 100% del contracte en un `EnemyBrain` compost — que al seu torn també és `ICombatant`, de manera que el joc real ni tan sols instancia `Enemy`.

### `HudAnchor` — *HudElement.h* (94 línies)
- **Què és**: enum class amb els 9 punts d'ancoratge estàndard d'un sistema 2D (TopLeft…BottomRight), l'única "gramàtica" de posicionament del HUD.
- **Herència**: cap (enum class sense tipus subjacent explícit).
- **Estat**: 9 enumeradors, sense valors assignats a mà.
- **API**: cap; es consumeix a `HudTransform::anchor` i a `anchorFraction()` (HudElement.cpp:14).
- **Flux**: el fixa qui construeix cada `HudTransform` (Application.cpp:436 i següents, level_editor.cpp, els demos). Cap GL.
- **Observacions**: `anchorFraction()` retorna `{0,0}` després del switch "per compiladors exigents" (HudElement.cpp:35), fet que amagaria un enumerador nou sense cas.

### `HudTransform` — *HudElement.h* (94 línies)
- **Què és**: struct pur amb ancoratge + offset + mida que resol la cantonada superior-esquerra en píxels de pantalla.
- **Herència**: cap; ningú n'hereta. És membre per valor de tots els widgets.
- **Estat**: `anchor` (TopLeft per defecte), `offset` {0,0} — desplaçament **sempre cap a l'interior** de la pantalla —, `size` {100,20}.
- **API**: `resolveTopLeft(int,int) const` (única funció; implementada a HudElement.cpp:40-65).
- **Flux**: el construeix el compositor del HUD (Application, level_editor, demos) i el consumeix cada `render()` de widget. **No necessita context OpenGL**: és aritmètica pura, verificada a `examples/demo_hud.cpp` (`testHudTransform()`).
- **Observacions**: la semàntica de l'offset canvia de signe segons la fracció (HudElement.cpp:61-62) i als eixos centrats (frac 0.5) no té "direcció cap a fora" — és un ajust fi sense significat direccional, fàcil de mal usar i sense cap test a `tests/`.

### `AnchorFraction` — *HudElement.cpp* (65 línies, struct anònim)
- **Què és**: parell `{h, v}` que redueix els 9 `HudAnchor` a dues fraccions 0/0.5/1 per no repetir la fórmula nou vegades.
- **Herència**: cap.
- **Estat**: `float h`, `float v`.
- **API**: cap (agregat); es produeix a `anchorFraction()`.
- **Flux**: detall intern de `resolveTopLeft()`, dins d'un namespace anònim. Sense GL.
- **Observacions**: cap.

### `IHudElement` — *HudElement.h* (94 línies)
- **Què és**: interfície mínima d'un element de HUD: sap dibuixar-se i, opcionalment, avançar la seva animació.
- **Herència**: base arrel de la jerarquia de widgets. **Virtuals**: `~IHudElement()` (default), `render(...) const = 0` (**pur**), `update(float)` (**virtual amb cos buit**). En deriven `HudBar`, `HudPanel` (HudWidgets.h:43,101), `HudText`, `HudCommandMenu`, `HudDialogueBox` (HudTextWidgets.h:28,54,110) i `HudMinimap` (HudMinimap.h:29) — cap d'ells afegeix un segon nivell d'herència. Només `HudBar` sobreescriu `update()`.
- **Estat**: `bool m_visible = true` (protected).
- **API**: render/update (virtuals) + `setVisible()`/`isVisible()` no virtuals.
- **Flux**: el construeix el compositor com a part d'un widget concret; el crida sempre `HudManager`. Els widgets **no** poden cridar `begin()/end()/flush()` de la batch (contracte documentat a HudElement.h:71-73). Necessita GL viu indirectament: `render()` acaba en `SpriteBatch::submit()`, que només serveix si algú fa flush amb context.
- **Observacions**: la interfície **no** hereta d'`IUpdatable` (include/Render/IUpdatable.h) tot i que el comentari (HudElement.h:63-66) reivindica coherència amb aquest contracte: hi ha dos contractes `update(float)` paral·lels i sense parentiu.

### `HudManager` — *HudManager.h* (67 línies)
- **Què és**: composa i dibuixa N `IHudElement*` en una passada, en coordenades de pantalla, amb la seva pròpia projecció ortogràfica.
- **Herència**: cap; ningú n'hereta. Es fa servir per composició (`Application::m_hud`, Application.h:311).
- **Estat**: `std::vector<IHudElement*> m_elements` — **no propietari** (mateix criteri que `IsometricRenderer::addToQueue`).
- **API**: registre (`addElement`, `removeElement` amb erase-remove), cicle (`update(float)` sobre **tots**, també els invisibles), dibuix (`render(SpriteBatch&, Shader&, int, int) const`).
- **Flux**: el construeix Application (o cada demo); `update()` es crida un cop per frame després de `refreshHud()` (Application.cpp:1537) i `render()` entre `m_hudBatch->begin()/end()` amb blending activat (Application.cpp:1583-1587). **Necessita context OpenGL viu**: crida `shader.use()` i `setUniformMat4/Int` (HudManager.cpp:31-44).
- **Observacions**: no valida `viewportWidth/Height > 0` (una `glm::ortho` amb 0 és degenerada); deixa el uniform `uViewProjection` en estat de pantalla a propòsit; la no-propietat obliga Application a fer `m_hud = HudManager{}` abans de destruir widgets (Application.cpp:1646) — invariant sostinguda només per convenció.

### `HudBar` — *HudWidgets.h* (121 línies)
- **Què és**: barra de valor (vida/mana/moral/temporitzador) feta de quads superposats: vora opcional al darrere, fons, franja "fantasma" i emplenat proporcional.
- **Herència**: `public IHudElement`. **Sobreescriu** `render(...) const override` i `update(float) override`; ningú n'hereta.
- **Estat**: `m_transform`, `m_whiteTexture` (aliè), `m_value`/`m_maxValue`/`m_displayedValue` (tots 1.0f per defecte), `m_animationSpeed` (≤0 = sense animació), `m_borderThickness` (≤0 = sense vora), `m_useThresholds`, i 6 colors (fons, emplenat, vora, fantasma, i els tres llindars).
- **API**: valor (`setValue` amb clamp, `setMaxValue`, `value`, `maxValue`, `displayedValue`), aparença (`setColors`, `setBorder`, `setThresholdColors`), animació (`setAnimationSpeed`, `update` **override**), dibuix (`render` **override**), privat `currentFillColor()`.
- **Flux**: el construeix Application (5 instàncies: vida, mana, enemic, moral, temporitzador — Application.cpp:453,466,493,647,660) i els demos; `HudManager` el crida. `update()` és **aritmètica pura sense GL** (demo_hud_update.cpp el prova amb `Texture* = nullptr`); `render()` sí que necessita GL viu per acabar a la GPU.
- **Observacions**: `setValue()` acota contra `m_maxValue` **abans** que ningú hagi cridat `setMaxValue()` (HudWidgets.cpp:23) — invertir l'ordre deixa la barra clavada a 1; el fantasma només es dibuixa quan el valor mostrat va per darrere (HudWidgets.cpp:116), així que **curar-se no s'anima**: l'emplenat salta al ratio real; i el comentari de HudWidgets.h:36-39 encara afirma que `update()` "és un mètode PROPI de HudBar, no d'IHudElement", contradient l'`override` de la línia 62.

### `HudPanel` — *HudWidgets.h* (121 línies)
- **Què és**: un únic quad de color sòlid amb marc opcional, pensat com a fons/contenidor d'altres widgets.
- **Herència**: `public IHudElement`; sobreescriu només `render(...) const override` (hereta l'`update()` buit). Ningú n'hereta.
- **Estat**: `m_transform`, `m_whiteTexture` (aliè), `m_color`, `m_borderColor`, `m_borderThickness` (≤0 = sense marc).
- **API**: `setColor()`, `setBorder()`, `render()` **override**.
- **Flux**: el construeix Application (7 panells: jugador, enemic, inventari, diàleg, comandes, botiga, negoci) i el level_editor (level_editor.cpp:843,1229,1282,1389). Ha d'anar-hi **abans** que el contingut a `addElement()` per quedar a sota. `render()` necessita GL viu (via SpriteBatch).
- **Observacions**: la vora es dibuixa com un quad més gran al darrere, no com 4 tires, així que amb un fons semitransparent la zona de la vora acumula dues capes d'alfa.

### `HudText` — *HudTextWidgets.h* (131 línies)
- **Què és**: una o diverses línies de text (separades per `\n`) en una posició fixa, amb ajust automàtic a l'ample de la caixa si el transform en porta.
- **Herència**: `public IHudElement`; sobreescriu només `render(...) const override`. Ningú n'hereta.
- **Estat**: `m_transform`, `m_font` (aliè), `m_text` (original), `m_renderText` (embolcallat) i `m_color`.
- **API**: `setText()` (fa el wrapping via `BitmapFont::wrapText`), `text()`, `setColor()`, `render()` **override**.
- **Flux**: el construeix Application (8 instàncies: estat, enemic, FPS, inventari, pista, títol de botiga, negoci, moral) i el level_editor; el manager el dibuixa. `setText()` és pur i sense GL; `render()` necessita GL viu i **un `BitmapFont` construït amb context** (l'atles es genera al constructor).
- **Observacions**: `setText()` comprova `m_font != nullptr` (HudTextWidgets.cpp:42) però `render()` el desreferencia sense comprovar (línia 59) — asimetria defensiva. Si `size.x` és més petit que dos glifs, `wrapText` entra en bucle infinit (vegeu Buits i riscos).

### `HudCommandMenu` — *HudTextWidgets.h* (131 línies)
- **Què és**: llista vertical d'opcions de text amb índex seleccionat, prefix de cursor "> " i navegació amb wraparound; no llegeix input per si mateix.
- **Herència**: `public IHudElement`; sobreescriu només `render(...) const override`. Ningú n'hereta.
- **Estat**: `m_transform`, `m_font` (aliè), `m_options`, `m_selectedIndex`, `m_textColor`, `m_highlightColor`.
- **API**: mesura (`static Vector2 contentSize(const BitmapFont&, const std::vector<std::string>&)` — **static**, mesura amb el prefix inclòs), navegació (`moveUp`, `moveDown`, `setSelectedIndex` amb clamp, `selectedIndex`), contingut (`options`, `setOptions` amb re-clamp), dibuix (`render` **override**).
- **Flux**: el construeix Application (menú de combat, botiga, negoci — Application.cpp:582,614,632) i el level_editor (eines i paleta); l'input el mou des de `Application::handleInput` (Application.cpp:1392-1393). `contentSize()`, `moveUp/moveDown` i `setOptions` són **purs sense GL** (demo_battle_hud.cpp:100 els prova amb font `nullptr`); `render()` sí necessita GL.
- **Observacions**: no té scroll propi; el level_editor implementa una finestra lliscant fora del widget (`paletteWindow` + `setOptions`, level_editor.cpp:1373-1405), duplicant lògica que hauria de viure aquí. `render()` desreferencia `m_font` sense comprovar (línia 115).

### `HudDialogueBox` — *HudTextWidgets.h* (131 línies)
- **Què és**: quadre de diàleg amb fons propi que mostra les últimes N línies d'un log (pensat per a `BattleState::log()`), envoltant cada línia a l'ample útil del panell.
- **Herència**: `public IHudElement`; sobreescriu només `render(...) const override`. Ningú n'hereta.
- **Estat**: `m_transform`, `m_font` i `m_whiteTexture` (aliens), `m_maxLines` (4 per defecte), `m_visibleLines`, `m_backgroundColor`, `m_textColor`.
- **API**: `setLines(const std::vector<std::string>&)` (wrapping + retall a les últimes `maxLines`), `maxLines()`, `render()` **override**.
- **Flux**: Application en construeix una (Application.cpp:562) amb un `HudPanel` 4px més gran al darrere fent de marc; `setLines()` es crida amb `battle.log()`. `setLines()` és pur sense GL; `render()` necessita GL i emet primer el quad de fons (textura blanca) i després els glifs (atles) — dos canvis de textura, dos flushos.
- **Observacions**: `render()` desreferencia `m_font` sense comprovar (línia 157) tot i que el demo el construeix amb `nullptr` (demo_battle_hud.cpp:141); l'ample útil resta un `kTextPadding` fix de 6px no parametritzable.

### `HudMinimap` — *HudMinimap.h* (66 línies)
- **Què és**: minimapa cenital (no isomètric) que pinta el `RuntimeTileMap` cel·la a cel·la més els marcadors de jugador, enemics vius i objectes llegits de `GameSession`.
- **Herència**: `public IHudElement`; sobreescriu només `render(...) const override`. Ningú n'hereta.
- **Estat**: `m_transform`, `m_whiteTexture`, `m_map`, `m_session` (tots aliens i **no propietaris**), `m_maxCells = 24` i sis colors (fons, transitable, bloquejat, objecte, enemic, jugador).
- **API**: `setMaxCells(int)` (clamp mínim 4), `maxCells()`, `render()` **override**.
- **Flux**: només l'instancia Application (Application.cpp:512) amb `&m_map` i la sessió. Llegeix l'estat en calent a cada `render()`: no cal refrescar-lo. Necessita GL viu; a més **necessita un `RuntimeTileMap` i una `GameSession` vius** en tot moment.
- **Observacions**: `render()` desreferencia `m_map` i `m_session` sense comprovar nul (HudMinimap.cpp:28,47); la finestra es centra al jugador i s'enganxa a les vores amb `std::clamp`; el cost és constant però alt: fins a 24×24 = 576 quads per frame només de terreny (HudMinimap.cpp:79-91), més un marcador per objecte i per enemic.

### `GlyphDef` / `GlyphDefX` — *BitmapFont.cpp* (466 línies, structs anònims)
- **Què són**: taules constexpr de patrons 5x7. `GlyphDef` indexa per `char` (ASCII 32..95), `GlyphDefX` per codepoint Unicode Latin-1 (fila estesa d'accentuades).
- **Herència**: cap.
- **Estat**: `char ch` / `uint32_t cp` + `unsigned char rows[7]` (bit 4 = columna esquerra).
- **API**: cap; es recorren a `glyphRowsForCell()` (BitmapFont.cpp:196) i `cellIndexForCodepoint()` (línia 182).
- **Flux**: dades pures compilades; només `generateAtlas()` les llegeix. Sense GL.
- **Observacions**: `kGlyphs` té 59 entrades sobre 64 cel·les (per tant `$`, `@`, backtick i companyia queden en blanc silenciós); `kGlyphsExtra` en té exactament 16, que omplen fins l'última cel·la de l'atles.

### `BitmapFont` — *BitmapFont.h* (143 línies)
- **Què és**: font bitmap monoespaiada 5x7 de caixa alta amb atles **procedural** generat en codi (no hi ha cap PNG de font vendoritzat), amb suport UTF-8 per a les accentuades del castellà/català.
- **Herència**: cap; ningú n'hereta. Còpia i assignació **eliminades** (`= delete`) perquè posseeix l'atles.
- **Estat**: `Texture* m_atlas` (**propietari**, s'esborra al destructor), `m_scale` (mínim 1), `m_glyphWidth = 5*scale`, `m_glyphHeight = 7*scale`; constant privada `kLineSpacing = 2` (píxels lògics).
- **API**: — **estàtiques i pures**: `glyphUV(char)`, `glyphUVCodepoint(uint32_t)`, `wrapText(text, maxWidth, glyphWidth)`, `clipText(text, maxWidth, glyphWidth)`, `measureText(text, glyphWidth, glyphHeight)`; — **d'instància**: `measureText(text) const` (usa `lineHeight()`, no `glyphHeight()`), `drawText(SpriteBatch&, text, topLeft, color) const`, `glyphWidth()`, `glyphHeight()`, `lineHeight()`, `atlas()`; — **privada estàtica**: `generateAtlas()`.
- **Flux**: l'instancia Application un cop (`scale=2`, Application.cpp:429) i el level_editor; els widgets de text el reben per punter aliè. **El constructor NECESSITA context OpenGL viu**: `generateAtlas()` fa `glGenTextures/glTexImage2D/glTexParameteri` (BitmapFont.cpp:424-437) — per això `demo_bitmap_font.cpp` diu explícitament que "MAI es construeix un BitmapFont" i només prova les estàtiques. `drawText()` també necessita GL (submits contra l'atles).
- **Suport d'accents/UTF-8**: l'atles és 16 columnes × 5 files = 80 cel·les; les 64 primeres són ASCII 32..95 i les 16 últimes són `À Á È É Ì Í Ò Ó Ù Ú Ñ Ç Ü ¿ ¡ ·` (BitmapFont.cpp:117-134). `nextCodepoint()` (línia 156) descodifica seqüències d'1, 2 i 3 bytes i consumeix un sol byte davant d'una seqüència malformada; `normalizeCodepoint()` (línia 140) passa `a-z → A-Z` i Latin-1 `0xE0-0xFE` (excepte `÷`) a majúscula restant 0x20. Tot el que no té cel·la cau a l'índex 0 (espai) sense llançar. `wrapText`/`clipText`/`measureText` compten **codepoints**, no bytes, i el tall amb guionet respecta els límits de caràcter.
- **Observacions**: `kExtendedBase (64) + kNumExtra (16) == kCols*kRows (80)` exactament — **no queda ni una cel·la lliure**, així que afegir un sol glifu obliga a tocar `kRows` (BitmapFont.cpp:23) i totes les UV derivades. Falta la dièresi catalana `Ï/ï` (i `Ë`, `Ê`, `Î`), que es dibuixa com a espai en blanc — precisament el símptoma que la F0 de l'auditoria deia haver arreglat.

### `ICombatant` — *ICombatant.h* (52 línies)
- **Què és**: contracte de "participant en combat Onegai Nd6" que el motor RPG (GL-free) fa servir per no conèixer mai `Player`/`Enemy` de `Render/`.
- **Herència**: interfície arrel. **Mètodes purs (9)**: `takeDamage(int)`, `heal(int)`, `health()`, `maxHealth()`, `isAlive()`, `stat(RPG::Stat)`, `defenses()`, `tier()`, `combatant_id()`. **Virtual amb implementació per defecte (1)**: `initiative_bonus()` → `stat(RPG::Stat::DES)`. **Helper no virtual**: `defense_value(RPG::Defense)` → `defenses().get(d)`. Destructor virtual per defecte. Implementadors reals: `Player` (Player.h:35), `Enemy` (Enemy.h:29), `EnemyBrain` (EnemyBrain.h:44) i set classes de test/demo (`TestCombatant` a demo_skills, demo_battle, demo_battle_hud, demo_object_catalog, demo_game_session, tests/Game/SaveGameTests.cpp:24, `TestBody`, `Muneco`, `CombatantNd6`).
- **Estat**: cap (interfície pura sense membres).
- **API**: vida (takeDamage/heal/health/maxHealth/isAlive), fitxa RPG (stat/defenses/tier/initiative_bonus), identitat (combatant_id), conveniència (defense_value).
- **Flux**: la consumeixen `Skill.cpp` (`ApplyBasicAttackNd6`, `ApplySkillEffect`), `BattleState.cpp`, `GameSession` (que rep un `ICombatant* playerBody`, GameSession.h:141,411) i `SaveGame.cpp`. **No necessita GL**, i aquest és precisament el seu motiu d'existir.
- **Cobertura per Player i Enemy**: tots dos la implementen **sencera, sense stubs buits**. `Player` sobreescriu els 9 purs més `initiative_bonus()` (Player.h:97, redundant: torna exactament el mateix que el default). `Enemy` sobreescriu els 9 purs més `initiative_bonus()`, tots delegant 1:1 al `m_brain` (Enemy.h:52-62). Tots dos redeclaren `defense_value()` **no virtual** (Player.h:95, Enemy.h:59), cosa que només amaga el de la base sense canviar res.
- **Observacions**: typo "Hepler" a la línia 50; el comentari de la línia 45 diu tier 1..3 però `Player::set_tier` accepta 1..5; `defenses()` retorna per valor, així que `defense_value()` construeix un `DefenseBlock` sencer per consulta.

### `Player` — *Player.h* (162 línies)
- **Què és**: entitat controlada pel jugador amb moviment discret per cel·la resolt a `handleInput()`, i alhora el cos de combat del PJ.
- **Herència**: `public AnimatedEntity` (base concreta: Entity → AnimatedEntity) **i** `public ICombatant` (interfície) — herència múltiple d'una base concreta + una interfície, el patró que ARCHITECTURE.md permet. Ningú n'hereta. **Sobreescriu**: `update(float) override` (delega en `AnimatedEntity::update`), i d'ICombatant `health`, `maxHealth`, `isAlive`, `stat`, `defenses`, `initiative_bonus`, `tier`, `combatant_id`, `takeDamage`, `heal`.
- **Estat**: `m_health`/`m_maxHealth` (100/100), `m_inventory` (`vector<int>`), `m_stats` (array de 4: CON/DES/INT/CAR, tots 0), `m_equip_armor`, `m_tier = 1`, `m_combatant_id = "player"`, i `RPG::CharacterSheet* m_sheet` **extern no propietari** que, si no és nul, passa a ser la font única per a stat/maxHealth/tier/id/defenses.
- **API**: cicle (`update` **override**), input (`handleInput(const InputState&)`, prioritat fixa amunt > avall > esquerra > dreta), ICombatant (el bloc de dalt), fitxa (`set_character_sheet`, `character_sheet()` en dues versions), setters legacy (`set_stat` amb clamp 0..8, `set_equip_armor`, `set_tier` clamp 1..5, `set_combatant_id`, `set_max_health`, `set_health`), inventari (`inventory()`, `addItem(int)`).
- **Flux**: el construeix Application (`m_player`, Application.h:171) i vuit demos; `handleInput()` el crida Application.cpp:1388 **només per l'animació** (la posició autoritativa la posa `GameSession::tryMovePlayer` i s'hi torna a escriure abans i després, línies 1379 i 1389). `takeDamage`/`heal` els criden les habilitats. Necessita GL viu indirectament (dibuixa via `Entity::render` amb `TextureAtlas`), però tot el bloc `ICombatant` és pur i testejable sense finestra.
- **Observacions**: el seu propi comentari qualifica la via legacy `m_stats` de "punto muerto" (Player.h:69) tot i que és la que fan servir tots els demos i tests; `defenses()` legacy no coincideix amb `RPG::Defense` (vegeu Buits i riscos); `heal()` acota contra `m_maxHealth` i no contra `maxHealth()`.

### `Enemy` — *Enemy.h* (88 línies)
- **Què és**: **només el sprite** de l'enemic — animació i dibuix — que reflecteix la patrulla i la vida que viuen dins d'un `EnemyBrain` compost.
- **Herència**: `public AnimatedEntity` **i** `public ICombatant`, igual que `Player`. Ningú n'hereta. **Sobreescriu**: `update(float) override` i tot `ICombatant` (`health`, `maxHealth`, `stat`, `defenses`, `initiative_bonus`, `tier`, `combatant_id`, `takeDamage`, `heal`, `isAlive`), **tots delegant al brain**, sense cap cos buit. Composició deliberada en lloc d'heretar també d'`EnemyBrain` (seria una segona base concreta).
- **Estat**: només `EnemyBrain m_brain`; **no hi ha cap segona còpia de la vida** aquí. Constants d'àlies `kIdle/kPatrol/kChase` = les d'`EnemyBrain`.
- **API**: cicle (`update` **override**: anima, avança el brain amb un callback de col·lisió que sempre diu que sí, i si la cel·la ha canviat fixa la posició i tria `walk_right`/`walk_left`), ICombatant delegat, accés (`aiState()`, `brain()` en dues versions).
- **Flux**: el **joc real no l'instancia**: `GameSession` guarda `std::unique_ptr<EnemyBrain>` directament (GameSession.h:129) i és aquest brain el que arriba a `BattleState`. Els únics constructors vius són a `examples/demo_animated_entity.cpp:134,159`. Necessita GL viu per dibuixar-se (via `AnimatedEntity`/`Entity`); la lògica és al brain, GL-free.
- **Observacions**: la vida per defecte de 50 (Enemy.cpp:9) és "el valor històric que els demos donen per fet" — un default que ja no correspon a res del joc; `Enemy` i el seu `m_brain` són **dos `ICombatant*` diferents per al mateix enemic**.

### Buits i riscos del mòdul

- **Bucle infinit a `BitmapFont::wrapText`** quan `maxWidth / glyphWidth == 1` (src/Render/BitmapFont.cpp:255-278): amb `maxGlyphs == 1`, el `while (taken < maxGlyphs - 1 …)` no consumeix res, `word.erase(0, i)` és un no-op i `glyphs -= taken` no decreix, així que es fa `out.push_back("-")` per sempre fins a esgotar memòria. És accessible des de `HudText::setText` (src/Render/HudTextWidgets.cpp:43) amb un `transform.size.x` entre 1 i 2 glifs, i des de `HudDialogueBox::setLines` (src/Render/HudTextWidgets.cpp:141) quan `size.x - 12` cau en aquesta franja.
- **Falta la dièresi catalana `Ï/ï`** (i `Ë`, `Ê`, `Î`) a `kGlyphsExtra` (src/Render/BitmapFont.cpp:117-134): es dibuixen com a espai en blanc, exactament el símptoma que la F0 deia haver eliminat, en un projecte amb la UI en català.
- **L'atles està ple al mil·límetre**: `kExtendedBase (64) + kNumExtra (16) == kCols*kRows (80)` (src/Render/BitmapFont.cpp:20-26,135). Afegir un sol glifu obliga a canviar `kRows` i recalcular totes les UV; no hi ha cap assert que ho protegeixi.
- **Buits silenciosos dins del rang ASCII**: `$` (36), `@` (64), backtick (96, fora de rang) i companyia no tenen `GlyphDef` (src/Render/BitmapFont.cpp:38-106) i es pinten com a espai — la mateixa classe de bug que ja va obligar a afegir `/`, `>` i `%` a posteriori (comentari a la línia 85-91).
- **`BitmapFont::glyphUV(char)` sobre bytes UTF-8 solts** retorna sempre la cel·la de l'espai (src/Render/BitmapFont.cpp:230-232): és API pública i convida a reintroduir el bug d'accents; l'únic consumidor viu és `examples/demo_bitmap_font.cpp`.
- **Desreferències sense comprovar nul** a tots els widgets de text i al minimapa: `HudText::render` (src/Render/HudTextWidgets.cpp:59), `HudCommandMenu::render` (:115), `HudDialogueBox::render` (:157) i `HudMinimap::render` (src/Render/HudMinimap.cpp:28 i 47). És contradictori amb `HudText::setText`, que sí comprova (`:42`), i amb els demos que construeixen aquests widgets amb `nullptr` a propòsit (examples/demo_battle_hud.cpp:100,141) confiant que ningú cridarà `render()`.
- **Trampa d'ordre a `HudBar`**: `setValue()` acota contra `m_maxValue`, que val 1.0f fins que es crida `setMaxValue()` (src/Render/HudWidgets.cpp:22-27). Application ho fa en l'ordre correcte (src/Engine/Application.cpp:454-455), però res ho imposa.
- **La curació no s'anima**: `HudBar::render` només pinta la franja fantasma quan `displayedRatio > ratio` i l'emplenat sempre usa el ratio real (src/Render/HudWidgets.cpp:109-126), així que `m_displayedValue` només és visible en baixar; en pujar, la barra fa un salt.
- **Comentari obsolet que contradiu el codi**: include/Render/HudWidgets.h:36-39 afirma que `update()` "és un mètode PROPI de HudBar, no d'IHudElement", mentre la línia 62 del mateix fitxer el declara `override`.
- **`IHudElement` no hereta d'`IUpdatable`** (include/Render/HudElement.h:87 vs include/Render/IUpdatable.h) tot i que el comentari reivindica coherència amb aquell contracte: hi ha dos `update(float)` sense parentiu de tipus.
- **Punters penjats latents a `HudManager`**: no posseeix res i no té cap protecció; l'única salvaguarda és que Application faci `m_hud = HudManager{}` abans de destruir widgets (src/Engine/Application.cpp:1646). Tampoc valida `viewportWidth/Height > 0` abans de la `glm::ortho` (src/Render/HudManager.cpp:40-42).
- **Mapatge de defenses incoherent amb `RPG::Defense`**: la via legacy de `Player::defenses()` (include/Render/Player.h:87-91) i d'`EnemyBrain::defenses()` (include/Game/EnemyBrain.h:116-119) omplen `values[2] = 10 + INT` i `values[3] = 10 + CAR`, però include/RPG/Defense.h:20-25 defineix `WILL_SAVE = 10 + CAR` i `SPELL_SAVE_DC = 8 + INT`. INT i CAR estan intercanviats i la DC de conjur surt 2 punts alta sempre que no hi hagi `CharacterSheet` — és a dir, a tots els demos i tests.
- **`maxHealth()` pot valer 0 amb fitxa**: `Player::maxHealth()` retorna `m_sheet->healthCap()` (include/Render/Player.h:59-63), que és el `cachedHealthCap` inicialitzat a 0 fins que algú crida `recompute_derived()` (include/RPG/CharacterSheet.h:143,151,156). Un `Player` amb fitxa sense recalcular alimenta `HudBar::setMaxValue(0)` (src/Engine/Application.cpp:454) i la barra de vida queda buida per sempre.
- **`Player::heal()` acota contra el màxim equivocat**: usa `m_maxHealth` legacy (src/Render/Player.cpp:47) mentre `maxHealth()` pot llegir del sheet — amb fitxa, curar mai arriba al màxim real (o el sobrepassa) segons quin dels dos sigui més gran.
- **Rangs de tier contradictoris**: `ICombatant::tier()` documenta 1..3 (include/Render/ICombatant.h:45), `Player::set_tier` acota a 1..5 (include/Render/Player.h:133) i `EnemyBrain::populate_from_monster_catalog` també a 1..5 (include/Game/EnemyBrain.h:156).
- **`defense_value()` redeclarat sense virtualitat** a include/Render/Player.h:95 i include/Render/Enemy.h:59: només amaga el helper de la base, no aporta res i fa pensar que hi ha una personalització.
- **`Enemy` és pràcticament codi mort**: el joc real només fa servir `EnemyBrain` (include/Game/GameSession.h:129) i l'única instanciació viva de `Enemy` són dues línies d'`examples/demo_animated_entity.cpp:134,159`. El seu default de 50 PV (src/Render/Enemy.cpp:9) ja no descriu cap enemic del joc.
- **Dos `ICombatant*` per al mateix enemic**: `Enemy` implementa la interfície i alhora conté un `EnemyBrain` que també la implementa (include/Render/Enemy.h:29 i include/Game/EnemyBrain.h:44). Els valors coincideixen perquè `Enemy` delega, però qualsevol comparació per identitat de punter ("és aquest el meu objectiu?") donaria fals.
- **Doble font de veritat per a la cel·la del jugador**: `Player::handleInput()` mou la posició de graella sense saber res de col·lisions (src/Render/Player.cpp:19-35) i Application ha de sobreescriure-la abans i després de cridar-lo (src/Engine/Application.cpp:1379,1385-1389). El comentari ho reconeix; la interfície no ho impedeix.
- **`setMaxValue` de la barra de vida no es refresca mai**: es fixa una sola vegada a la inicialització (src/Engine/Application.cpp:454) mentre `refreshHud()` només actualitza el valor (:908), així que una pujada de nivell o un canvi de fitxa deixa el màxim obsolet.
- **Zero cobertura de tests reals**: no hi ha cap fitxer a `tests/` per a HUD, `BitmapFont`, `Player` ni `Enemy`; l'única aparició d'`ICombatant` és `tests/Game/SaveGameTests.cpp:24` amb un `TestCombatant` propi. Tota la verificació viu a `examples/demo_hud.cpp`, `demo_hud_update.cpp`, `demo_bitmap_font.cpp` i `demo_battle_hud.cpp`, fora de l'arbre de tests.
- **`HudCommandMenu` sense finestra pròpia**: el level_editor reimplementa la finestra lliscant a fora amb `paletteWindow()` + `setOptions()` (editor/src/level_editor.cpp:1373-1405), la mateixa duplicació que `contentSize()` va néixer per evitar.
- **Cost de dibuix per alternança de textura**: `SpriteBatch::submit` fa flush a cada canvi de textura (src/Render/SpriteBatch.cpp:55-57) i el HUD alterna textura blanca i atles de font panell a panell; `HudDialogueBox` sol ja provoca dos flushos, i `HudMinimap` afegeix fins a 576 quads per frame (src/Render/HudMinimap.cpp:79-91).
- **`wrapText` emet una línia buida final** per a cada línia dura buida (src/Render/BitmapFont.cpp:313), de manera que una entrada de log en blanc consumeix una de les `maxLines` del quadre de diàleg.
