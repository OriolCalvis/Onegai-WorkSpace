## El titular

MotorGraphico té molta més **superfície** que **profunditat**, i la distància entre les dues és el resultat principal d'aquesta auditoria.

El que funciona de debò és un nucli sorprenentment complet: una partida jugable amb exploració, combat per torns Nd6, diàleg, botigues, negocis i moral; un editor de mapes amb capes, selecció, portapapers i desfer; un motor narratiu amb condicions, efectes i temporitzadors; i un format de guardat amb escriptura atòmica. Això no és un prototip: és un joc petit que es pot acabar.

Al voltant d'aquest nucli hi ha tres cinturons que **no toquen terra**:

1. **Un model de dades RPG de 21 catàlegs i milers d'entrades que el joc no llegeix** — i, pitjor, que els carregadors llegeixen malament quan els llegeixen.
2. **Peces noves ben escrites i ben provades que ningú no ha connectat encara**: `World/`, `Scene/`, `SceneManager`, `MapDocument`, `ProjectSerializer`, `HandlePool`.
3. **API especulativa que ni tan sols enllaçaria** si algú la cridés: `SeededRng`, set mètodes de `TierRules`, `IQteProvider`.

La conseqüència pràctica: el projecte sembla molt més gran del que és executable, i això distorsiona qualsevol estimació. Les 426 jornades restants del Gantt compten feina de connectar coses que ja existeixen barrejada amb feina d'escriure coses que no existeixen, i aquestes dues no costen el mateix.

## Els sis patrons transversals

Cap agent podia veure aquests patrons des del seu mòdul; només apareixen quan es llegeixen els vuit alhora.

### 1. Codi escrit que no s'executa mai

No és un cas aïllat, és un patró de tot el repositori. L'inventari complet:

| Peça | On és | Estat |
| --- | --- | --- |
| `Engine::World` + `EntityHandle` | `World/` | Correcte i provat. Només l'enllaça `test_world`. Cap fitxer de producció l'inclou. |
| `Scene` + `SceneManager` | `Engine/Scene/` | Correcte i provat. `motor_scene` només s'enllaça amb `test_scene_manager`. |
| `MapDocument` i les tres capes | `Map/` | Només el toquen tests, `TmxMapImporter` i `RuntimeTileMapBuilder`. L'editor no el coneix. |
| `ProjectSerializer` | `Project/` | Format canònic v1 sense cap consumidor de producció. |
| `DocumentRef` | `Project/` | Tipus orfe: la identitat estable per id que promet no s'aplica enlloc. |
| `HandlePool<T>` | `Core/Memory/` | Cap instanciació al motor. I té un bug al moviment. |
| Overrides de `RuntimeTileMap` | `Runtime/` | `setTileOverride` i companyia no es criden mai — i `m_baseLayers` duplica **tota** la memòria de tiles per sostenir-los. |
| `IRng` / `SeededRng` + els dos adaptadors pont | `RPG/Rng.h`, `RandomEngine.h` | Zero referències. No enllaçaria. |
| `IQteProvider` | `RPG/EnhanceHooks.h` | Cap implementació → el PAS 2 de `SkillExecutor` és inabastable. |
| Mitja `DicePoolEngine` | `RPG/` | `resolve_against_cd`, `resolve_opposed`, `defense_to_cd`… marcats "EN DESÚS des d'Ed. 3" i encara exportats. |
| Càrrega de `tier_rules.json` | `RPG/TierRules.cpp` | El fitxer existeix i és correcte. Ningú crida `loadFromFile`. |
| Post-FX sencer | `Render/` | `Framebuffer`, `ColorLUT`, `FogOfWar`, `DynamicLightMap`: només els demos. |
| `EditorValidation::checkRpg` | `Editor/` | La validació de contingut RPG abans del playtest no la crida mai l'editor. |
| `m_timerBar` | `Engine/Application.cpp` | Es crea, es configura, es fa invisible i no es torna a tocar. |

Res d'això és inútil: és feina pagada i aparcada. Però mentre estigui desconnectada no protegeix de res, i cada dia que passa se separa més del codi viu.

### 2. Els carregadors i el JSON no es parlen

Aquest és el descobriment més gros de l'auditoria i el que té conseqüències més directes al joc.

Dels 21 `CatalogLoader<T>` del sistema RPG, **només dos llegeixen de debò el fitxer que tenen al davant**: `LocationDefinition` (escrit després del seu JSON) i, a mitges, `LootTableDefinition`. Els altres 19 carreguen `id`, `name`, `description`, `tier` i `rarity`, i deixen tota la mecànica als valors per defecte, sense un sol avís.

El que això significa en números concrets:

- Les **492 skills i els 210 encanteris** tenen tots exactament la mateixa magnitud `{0, 2, 4, 8}`, perquè `magnitudeByDegree` és `null` a totes les entrades i el fallback assigna `base = 4`.
- Els **432 monstres** es carreguen amb `maxHealth = 0` i `ca = 10`: el JSON té la vida dins de `defenses{}` i `derived{}`, i el carregador la busca a l'arrel.
- Les **16 condicions** entren totes idèntiques: el JSON porta `stackable`/`duration`/`effects[]` i el carregador espera `stacking`/`defaultRounds`/`damagePerRound`.
- Les **66 passives** cauen totes a `ALWAYS_ACTIVE` amb bonificadors a zero, perquè `trigger` al JSON és prosa castellana i el carregador espera identificadors.
- `saveAttribute` **mai** es llegeix (el JSON el diu `defenseStat`), de manera que la branca de tria d'estadística defensiva de `SkillExecutor` no s'executa cap vegada.

I això travessa a producció: `SkillCatalog` és **l'únic** catàleg que el joc carrega de veritat.

La causa d'arrel és estructural, no un descuit: `Catalog<T>::loadFromString` embolcalla cada entrada en un `try/catch(...)` buit i compta com a bona qualsevol que doni un `id`. Un carregador que no troba cap dels camps que busca és indistingible d'un que funciona. **Cap prova comprova el contingut d'un camp carregat** — `demo_catalogos_rpg` només valida `total > 0`.

### 3. Funcions declarades i mai definides

Bombes de linkatge: no petes fins que algú les crida.

- `TierRules`: `healthCap`, `healthBonus`, `allowsMulticlass`, `canEquipRarity`, `maxSummons`, `maxFeats`, `maxDivineCards` — **set mètodes** declarats al header i absents del `.cpp`.
- `SeededRng::nextInt` i `SeededRng::nextRaw`: no existeix cap `Rng.cpp`.
- `Editor::ObjectInspector::confirmar()`: declarat, mai definit ni cridat.
- `src/RPG/DicePoolEngine.cpp` no és a la llista de fonts de `motor_rpg`: no es compila.

### 4. Dos de tot

El repositori té sistemàticament dues implementacions del mateix concepte, una de vella i viva i una de nova i desconnectada:

| Concepte | Camí viu | Camí nou | Es parlen? |
| --- | --- | --- | --- |
| Format de projecte | manifest castellà a l'arrel (`ProjectIndex`) | claus angleses amb `settings` niat (`ProjectSerializer`) | **No.** Cap dels dos pot llegir el fitxer de l'altre. |
| Model de mapa | `EditorState` (capes planes, TMX de text) | `MapDocument` (capes tipades, JSON versionat) | No. |
| Món d'entitats | `GameSession` + `Application::m_worldMarkers` | `Engine::World` | No. |
| Condicions | `CharacterSheet::applyCondition` | `ConditionEngine` | No, i tenen regles diferents. |
| Catàleg de skills | `::SkillCatalog` (Game) | `RPG::Catalogs::SkillCatalog` | Tipus sense cap relació, mateix nom. |
| Contracte `update(float)` | `IUpdatable` | `IHudElement::update` | Dos contractes sense parentiu. |
| Culling de tiles | `IsometricRenderer::visibleRangeFor` | `TileMap::visibleRange` | Còpia literal; el segon només l'usa un demo. |
| Atzar | `RandomEngine` / `Xoroshiro128p` | `IRng` / `SeededRng` | Hi ha dos adaptadors pont; cap dels dos s'usa. |

Cada parella té un cost fix: qui toca el codi ha de saber quin dels dos és el bo, i la resposta no és evident des del fitxer.

### 5. Els bugs que un jugador notaria

Aquests no són deute: són coses que fallen ara mateix.

Els he ordenats per **quant de mal fan al jugador**, no per elegància de la solució.

### 6. Els tests protegeixen el que és nou, no el que es juga

La cobertura està invertida respecte del risc. Els mòduls acabats de refactoritzar (`Map/`, `Project/`, `Runtime/`, `Engine/Input`, `Engine/Scene`, `World/`) tenen tests unitaris propis. Els que sostenen la partida real no en tenen cap:

- **Zero tests** de `Render/` sencer — nucli i HUD.
- **Zero tests** del model de dades RPG, que és precisament on viu el patró 2.
- **Zero tests** de les 1.682 línies d'`Application`, incloent-hi `refreshHud`, `syncWorldMarkers`, `loadLevel`, el guardat i el director de campanya, que són lògica pura i separable de GL.
- La màquina d'estats de `GameMode`, l'economia de negocis, la moral i la botiga només es proven a través de demos.

I això explica per què els dos bugs més greus del joc (la curació i la moral perduda) han sobreviscut: cap demo travessa aquestes vores.

## Què tenim i què ens falta

La pregunta original. Aquesta és la lectura honesta mòdul a mòdul.

**Tenim, i funciona:** el bucle de joc complet amb pas fix; exploració amb col·lisions, pickups i portes; combat per torns amb graus d'èxit Nd6; diàleg, botigues i negocis amb moral; el motor narratiu amb condicions, efectes, temporitzadors i beats; el guardat de partida amb escriptura atòmica i validació de versió; un editor de mapes amb capes, selecció, portapapers, desfer i inspector d'instància; un hub de projectes; un importador TMX; un HUD complet amb tipografia pròpia i accents.

**Tenim, però no ho fem servir:** el model documental de mapa i projecte, el gestor d'escenes, el món d'entitats, el pool de handles, tot el post-processat de render (boira, llum dinàmica, LUT), i el 90% del model de dades RPG.

**No tenim, i el Gantt ho dóna per fet:** un pont entre `MonsterDefinition` i els enemics reals (els enemics del joc lluiten amb stats a zero); aplicació de condicions des de les cartes d'habilitat; botí i taules de loot al runtime; esdeveniments per casella (`eventId` s'edita i no es llegeix); persistència de l'aventura activa al guardat; redimensionat de finestra; i qualsevol validació que comprovi que el contingut carregat té els camps que diu tenir.

**La conclusió pràctica:** abans d'afegir cap fase nova del Gantt, el treball de més retorn no és escriure codi nou sinó **connectar i verificar el que ja hi és**. Concretament: arreglar els carregadors de catàleg (o retirar els 19 que no serveixen), connectar o esborrar les peces del patró 1, i posar tests al camí que es juga de veritat.
