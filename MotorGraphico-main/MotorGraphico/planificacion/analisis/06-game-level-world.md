## Mòdul Partida (Game / Level / World)

Una partida en curs és una `GameSession`: un objecte GL-free que agafa un `RuntimeTileMap` (col·lisió), un `ObjectCatalog` (què és cada cosa), un `SkillCatalog` (habilitats de combat) i una còpia de `LevelDefinition` (on és cada cosa), i n'extreu l'estat viu del món — posició del jugador, objectes que queden, enemics amb cervell propi, or, moral, negocis, diàleg obert i combat en curs. L'orquestra `Engine::Application`, que tradueix teclat a crides (`tryMovePlayer`, `interact`, `closeInteraction`, `buy/sell`, `syncBattleOutcome`) i dibuixa el resultat; `Level/` és la capa de dades (parseig JSON de nivells i catàlegs, més el graf de portes `WorldTopology`), i `World/` és un propietari d'entitats runtime que **encara no està connectat a res**.

La frontera "GameSession no fa I/O" **es compleix a la pràctica**: `src/Game/GameSession.cpp` no inclou cap `<fstream>` ni cap ruta d'asset; les transicions de nivell s'exposen com a dada (`LevelTransition`) perquè l'Application les executi, i el guardat es parteix en DTO + fitxer (`SaveGame.cpp`) amb la sessió limitant-se a `captureSaveGame`/`restoreSaveGame`. Els dos matisos: aquests dos mètodes de `GameSession` estan *definits* dins de `src/Game/SaveGame.cpp`, la mateixa unitat de traducció que obre fitxers amb `std::ofstream`/`std::rename` (SaveGame.cpp:259-301), i `GameSession.h:16` arrossega `RPG/Catalogs/RpgCatalogs.h`, un header que porta `loadFromFile` amb `fopen` (RpgCatalogs.h:768). Cap de les dues coses trenca el contracte de comportament, però l'aïllament és menys net del que diu el comentari.

---

### `GameMode` — *GameSession.h* (enum, línia 62)
- **Què és**: els cinc modes d'una partida més el terminal: `Exploration`, `Battle`, `GameOver`, `Dialogue`, `Shop`, `Business`. `Dialogue`/`Shop`/`Business` són modals igual que `Battle`: mentre estan actius el jugador no es mou i el món no avança.
- **Herència**: cap (enum class).
- **Flux**: veure la màquina d'estats detallada a `GameSession`.

### `OwnedBusiness` — *GameSession.h* (línies 67-76)
- **Què és**: un local que el jugador ha comprat. El preu i l'ingrés base venen del catàleg; el que hi viu és la decisió del jugador (`rentPercent`).
- **Herència**: cap.
- **Estat**: `objectId` (el cartell que el representa), `name`, `baseIncome`, `rentPercent` (100 = lloguer just, acotat a [50, 200]).
- **API**: agregat pur, sense mètodes.
- **Flux**: el crea `buyCurrentBusiness()` (GameSession.cpp:477); el llegeixen `collectRent()`, `adjustCurrentRent()`, el HUD (Application.cpp:1078) i el guardat (`SaveGameData::businesses`).
- **Observacions**: es serialitza directament al JSON de partida, o sigui que és de facto part del format de guardat tot i viure a la capçalera de sessió.

### `LevelTransition` — *GameSession.h* (línies 84-89)
- **Què és**: una porta trepitjada pendent d'executar. És la peça que manté la sessió sense I/O: detecta que toca canviar de mapa i ho publica.
- **Herència**: cap.
- **Estat**: `pending`, `levelPath`, `entryPosition`, `hasEntryPosition` (si és fals, s'usa el `playerStart` del nivell destí).
- **API**: agregat; s'accedeix per `pendingTransition()` / `clearTransition()`.
- **Flux**: l'omple `interact()` (GameSession.cpp:552-556) quan hi ha un objecte amb `targetLevel` sota els peus; el consumeix `Application::applyPendingTransition()` (Application.cpp:888), que la neteja **abans** de carregar per no reintentar en bucle si el fitxer no existeix.

### `ShopOffer` — *GameSession.h* (línies 94-98)
- **Què és**: una línia de la botiga oberta, amb el preu ja resolt (el de la botiga si en té, si no el del catàleg).
- **Herència**: cap.
- **Estat**: `objectId`, `name`, `price`.
- **Flux**: la construeix `startInteraction()` en obrir (GameSession.cpp:248-261), descartant articles amb preu ≤ 0 per evitar or infinit; viu mentre `mode()==Shop` i la llegeixen `buy()` i el HUD.

### `WorldEnemy` — *GameSession.h* (línies 112-131)
- **Què és**: un enemic viu al món: quina definició és, on és i el seu estat de combat. El cos i les habilitats són propietat de la sessió.
- **Herència**: cap.
- **Estat**: `objectId`; `spawnIndex` (posició dins de `LevelDefinition::objects`, l'única identitat que sobreviu a un guardat); `position` (mirall de `brain->position()`, que és l'autoritativa); `unique_ptr<EnemyBrain> brain`; `unique_ptr<SkillSet> skills`.
- **Flux**: el construeix el ctor de `GameSession` a partir de `CombatData` del catàleg (GameSession.cpp:33-47), i també `resolveBattles()` per a emboscades narratives (:354-363). El llegeixen `update()`, `startBattle()`, el minimapa i `Application::syncWorldMarkers()`.
- **Observacions**: els enemics creats per narrativa **no reben `spawnIndex`** (queda a -1, GameSession.cpp:354-356), cosa que trenca el guardat (veure Buits).

### `GameSession` — *GameSession.h* (498 línies) / *GameSession.cpp* (762) / *SaveGame.cpp* (parts)
- **Què és**: el cor del bloc — l'estat complet d'una partida en curs i les regles que el mouen, sense GL, sense I/O i sense conèixer `Player`/`Enemy` concrets. És el pegament entre nivell JSON, catàleg d'objectes, habilitats, combat per torns, economia de negocis, moral i motor narratiu.
- **Herència**: cap; no és base de ningú.
- **Estat, per subsistema**:
  - *Dependències no propietàries*: `m_map` (RuntimeTileMap), `m_catalog`, `m_skillCatalog`, `m_playerBody` (`ICombatant*`), `m_playerSkills`, `m_inventory` — tots han de sobreviure la partida. `m_level` és **còpia** de la `LevelDefinition`.
  - *Exploració*: `m_playerPosition`, `m_worldObjects` (props/pickups vius), `m_enemies`, `m_lastPickupId`, `m_log`.
  - *Combat*: `m_mode`, `unique_ptr<BattleState> m_battle`, `m_battleEnemyIndex`.
  - *Diàleg/botiga*: `m_dialogueSpeaker`, `m_dialogueLines`, `m_shopOffers`, `m_gold`.
  - *Negocis*: `m_businesses`, `m_currentBusinessId/Name/Price/Income`, `m_stepsSinceRent` + `kStepsPerRentCycle = 40` (el temps del joc són passes, no rellotge, per determinisme).
  - *Narrativa*: `m_narrative` (const, contingut), `m_narrativeState` (mutable, partida), `m_rng` (Xoroshiro128p propi) + `m_rngInUse`, `m_nd6Skills`, `m_pendingBattleVictoryFlag`/`DefeatFlag`, `m_battleQueue` (onades pendents).
  - *Transversal*: `m_morality`, `m_transition`.
- **API**:
  - *Consulta*: `mode()`, `playerPosition()`, `worldObjects()`, `enemies()`, `log()`, `lastPickupId()`.
  - *Exploració*: `tryMovePlayer(dx,dy)`, `update(dt)`.
  - *Interacció*: `interact()`, `closeInteraction()`.
  - *Botiga/economia*: `shopOffers()`, `shopKeeper()`, `buy(i)`, `sell(i)`, `gold()`, `addGold()`.
  - *Negocis*: `currentBusiness*()`, `businesses()`, `ownsBusiness()`, `buyCurrentBusiness()`, `adjustCurrentRent(delta)`, `collectRent()`, i **`static`** `occupancyPercent()` / `incomeFor()` (pures, perquè el cartell pugui ensenyar el càlcul abans de decidir) + `static constexpr kMinRent/kMaxRent`.
  - *Moral*: `morality()` (const i no-const).
  - *Combat*: `battle()`, `syncBattleOutcome()`.
  - *Transició*: `pendingTransition()`, `clearTransition()`.
  - *Narrativa*: `setNarrative()`, `narrativeState()`, `setRandomEngine()`, `setNd6SkillCatalog()`, `enterLevelNarrative(levelId)`, `talkNarrative(npcId)`, `updateNarrativeTimers(dtActiu, nivell)`.
  - *Guardat*: `captureSaveGame(levelPath) const`, `restoreSaveGame(data, &error)`, `logNote()`.
  - *Privats*: `logLine`, `enemyIndexAt`, `worldObjectIndexAt`, `tileBlocks`, `startBattle`, `startInteraction`, `basePriceOf`, `applyNarrative`, `resolveSkillChecks`, `resolveBattles`. Cap `virtual`.
- **Flux i màquina d'estats de `GameMode`**:
  - *Construcció*: només `Application::loadLevel()` (Application.cpp:778) i els demos/tests. El ctor reparteix `LevelDefinition::objects` en dues llistes segons la categoria del catàleg (Enemy → `WorldEnemy` amb `EnemyBrain` + `SkillSet`; la resta → `m_worldObjects`), i un `objectId` desconegut es tracta com a objecte inert (criteri permissiu, igual que `LevelLoader`). Si el jugador ja ve mort → `GameOver` de sortida (:56-58).
  - *`Exploration → Battle`*: `tryMovePlayer()` en topar amb la cel·la d'un enemic (`startBattle`, :112-115) — retorna `false` però canvia el mode, així que qui crida ha de mirar `mode()`, no el booleà. També `resolveBattles()` des d'un beat narratiu amb effect `startBattle` (:328).
  - *`Battle → Exploration`*: `syncBattleOutcome()` amb `Victory` (l'enemic s'esborra de `m_enemies`) o `Fled` (l'enemic segueix viu al seu lloc).
  - *`Battle → GameOver`*: `syncBattleOutcome()` amb `Defeat`. `GameOver` és terminal — res no en surt tret de `restoreSaveGame()` amb un jugador viu (SaveGame.cpp:463).
  - *`Exploration → Dialogue`*: `interact()` amb un NPC a la pròpia cel·la o a una de les quatre veïnes (`startInteraction`, :581); `applyNarrative()` des de `enterLevelNarrative`, `talkNarrative`, `updateNarrativeTimers` o el tick de `closeInteraction`.
  - *`Exploration → Shop`*: mateix `startInteraction()` si el NPC té articles vàlids (:263) — botiga i diàleg són el mateix camí, decidit per si `m_shopOffers` queda buida.
  - *`Exploration → Business`*: `interact()` sobre un objecte amb `business.price > 0` (:585-592).
  - *`Dialogue|Shop|Business → Exploration`*: només `closeInteraction()` (:597), que a més fa **un** tick narratiu (mai un bucle `while`, per no penjar el joc amb contingut mal escrit) i pot tornar a entrar a `Dialogue` immediatament.
  - *Sense transició*: `update(dt)` només té efecte en `Exploration`; en qualsevol altre mode el món es congela.
  - *Produeix*: un log únic de partida (exploració + combat, amb el log del `BattleState` copiat abans de destruir-lo), la llista d'objectes/enemics que l'Application converteix en marcadors de render, i el `SaveGameData`.
- **Observacions**:
  - `m_level` es copia sencera i, després del constructor, **només se'n llegeix `playerStart`** (línia 20) i `objects` (línia 29). El comentari de la capçalera (:137-139) diu que "la sessió el va mutant"; no és cert — les mutacions van a `m_worldObjects`. És una còpia morta que a més queda desincronitzada del món real.
  - `LevelDefinition::enemies` no es llegeix mai aquí (documentat i verificat).
  - Doble log de les línies narratives quan un beat surt per boca d'un NPC: `applyNarrative` ja les registra (:386-389) i `startInteraction` les torna a registrar (:264-266).
  - `restoreSaveGame` no neteja `m_currentBusinessId/Name/Price/Income`, ni `m_battleQueue`, ni les flags de batalla pendents.
  - `setRandomEngine()` només afecta les tirades narratives: el `BattleState` que crea `startBattle()` es queda amb el seu Xoroshiro propi (els demos han de fer `session.battle()->setRandomEngine(...)` a mà, demo_game_session.cpp:169).

---

### `BattleAction` — *BattleState.h* (línies 41-60)
- **Què és**: l'ordre d'un torn: tipus, id d'habilitat, índex d'objectiu i id d'ítem.
- **Herència**: cap.
- **Estat**: `type` (`Attack`/`UseSkill`/`Item`/`Flee`), `skillId`, `targetIndex` (índex dins del bàndol contrari), `itemId`.
- **Observacions**: `itemId` va deliberadament l'últim per no trencar en silenci els inicialitzadors posicionals de 3 elements anteriors a la Fase 10 — el comentari explica que inserir-lo al mig compilaria i petaria en runtime.

### `BattleParticipant` — *BattleState.h* (línies 72-83)
- **Què és**: un lluitador dins d'un combat: nom per al log més punters no propietaris al seu cos, habilitats i inventari reals.
- **Herència**: cap.
- **Estat**: `name`, `combatant` (`ICombatant*`), `skills` (`SkillSet*`, pot ser nul), `inventory` (`vector<string>*`, nul per als enemics).
- **Flux**: els construeix `GameSession::startBattle()` (GameSession.cpp:684-686) passant el cos/habilitats/inventari **reals** del jugador, perquè el que gasta en combat es conservi en tornar a explorar.

### `BattleState` — *BattleState.h* (177 línies) / *BattleState.cpp* (221)
- **Què és**: el combat per torns estil DQ clàssic, GL-free, sobre interfícies (`ICombatant`/`SkillSet`), amb cua de torns deliberadament simple (sense iniciativa).
- **Herència**: cap.
- **Estat**: `m_allies`, `m_enemies`, `m_catalog` (skills), `m_objectCatalog` (ítems, per defecte `nullptr`), `m_outcome`, `m_log`, `m_rng` + `m_rngInUse`.
- **API**: `resolveAllyAction(i, action)`, `resolveEnemyTurn()` (resol TOTS els enemics vius d'un cop), `outcome()`, `allies()`, `enemies()`, `log()`, `setRandomEngine()`. Privats: `checkOutcome()`, `logLine()`, `resolveItemAction()`. Cap `virtual`.
- **Flux**: el crea i el posseeix `GameSession::startBattle()`; qui orquestra el torn (Application.cpp:1240-1247, o un demo) crida `resolveAllyAction` i després `resolveEnemyTurn`, i finalment `GameSession::syncBattleOutcome()` aplica el desenllaç al món. El dany passa per `ApplyBasicAttackNd6`/`ApplySkillEffect` → `SkillExecutor` → `DicePoolEngine`, de manera que cada cop té grau (fracàs/parcial/èxit/crític).
- **Observacions**: `resolveAllyAction` valida l'objectiu **sempre contra `m_enemies`** i hi aplica l'efecte tal qual — les habilitats de curació apunten a l'enemic (veure Buits). Les etiquetes de grau del log estan en català (`degreeLabel`, BattleState.cpp:12-25) mentre la resta de missatges del mòdul són en castellà sense accents.

---

### `EnemyBrain` — *EnemyBrain.h* (296 línies) / *EnemyBrain.cpp* (23)
- **Què és**: què *és* un enemic sense res de com es *dibuixa*: vida, estat d'IA, patrulla i fitxa RPG. Resol la "fractura #1" — abans hi havia `Enemy` (amb sprite i IA, però exigia `TextureAtlas`) i un `CatalogCombatant` amagat dins de GameSession.cpp (només vida): la patrulla es parsejava del JSON i es llençava.
- **Herència**: **`public ICombatant`**. Ningú no n'hereta.
- **Estat**: `m_position`, `m_patrolMin/Max`, `m_health`/`m_maxHealth`, `m_aiState`, `m_direction`, `m_stepInterval`/`m_stepElapsed`; capa RPG: `m_stats[4]`, `m_equip_armor`, `m_tier`, `m_combatant_id` i `m_sheet` (`RPG::CharacterSheet*` extern, no propietari — si és no nul, mana ell).
- **API**: `update(dt, canEnter)` (plantilla, amb callback de col·lisió perquè la classe no hagi de conèixer el mapa); overrides d'`ICombatant` (`takeDamage`, `heal`, `health`, `maxHealth`, `isAlive`, `stat`, `defenses`, `initiative_bonus`, `tier`, `combatant_id`); `set_character_sheet`/`character_sheet`; `populate_from_monster_catalog(mon, tierRules, defaultClass)`; setters `set_stat`, `set_equip_armor`, `set_tier`, `set_combatant_id`, `set_max_health`, `set_health`; `position()`, `setPosition()`, `aiState()`, `direction()`; constants `static constexpr kIdle/kPatrol/kChase`.
- **Flux**: el crea `GameSession` (ctor i `resolveBattles`) des de `CombatData`; el mou `GameSession::update()` amb el callback de col·lisió; el llegeix `BattleState` com a `ICombatant` i `Render/Enemy` per triar l'animació via `direction()`. `restoreSaveGame` hi escriu amb `setPosition`/`set_health`.
- **Observacions**: la patrulla és **només horitzontal** (`stepOnce` toca `next.x`); `patrolMin.y`/`patrolMax.y` es parsegen, es guarden i no s'usen mai. `kChase` està declarat i no s'assigna enlloc: no hi ha persecució. `GameSession` no crida mai `set_stat` ni `populate_from_monster_catalog`, així que tots els enemics del joc entren al combat Nd6 amb stats `{0,0,0,0}`.

### `Morality` — *Morality.h* (63 línies) / *Morality.cpp* (46)
- **Què és**: la reputació moral en una escala única de -100 (vilà) a +100 (heroi), amb historial de motius.
- **Herència**: cap.
- **Estat**: `m_value` (acotat), `m_history` (màxim `kHistoryLimit = 12`, del més recent al més antic).
- **API**: `record(delta, reason)` (delta 0 no registra res), `value()`, `fraction()` (0..1 per a barres de HUD), `label()` (VILLANO/CANALLA/NEUTRAL/JUSTO/HÉROE, amb trams asimètrics a propòsit), `history()`, `setValue()` (per restaurar sense registrar motiu); constants públiques `kMin`/`kMax`.
- **Flux**: viu dins de `GameSession` (`m_morality`); només l'escriuen `adjustCurrentRent()` (canvi de lloguer) i `collectRent()` (lloguer sostingut); el llegeixen el HUD (Application.cpp:1043) i `captureSaveGame`.
- **Observacions**: el comentari de `label()` promet "majúscules **sense accents** perquè la dibuixa la BitmapFont del motor", però retorna `"HÉROE"` amb accent (Morality.cpp:45) — contradicció directa amb el seu propi contracte. `setValue()` no buida `m_history`, així que en carregar una partida el valor és el guardat però els motius són els de la sessió anterior.

### `Skill` — *Skill.h* (línies 24-34)
- **Què és**: dades pures d'una habilitat de combat llegada (no confondre amb `RPG::SkillDefinition`).
- **Estat**: `id`, `name`, `mpCost`, `power`, `effect` (`Damage`/`Heal`), `target` (`Self`/`SingleEnemy`/`SingleAlly`/`AllEnemies`/`AllAllies`).
- **Observacions**: `target` **no el llegeix ningú** al motor — només s'escriu (Application.cpp:260-270, demos). És el camp que hauria d'evitar el bug de la curació.

### `SkillSet` — *Skill.h* (línies 42-87) / *Skill.cpp*
- **Què és**: les habilitats que coneix una entitat més la seva reserva de mana; no posseeix les `Skill`, només ids contra un catàleg.
- **Herència**: cap.
- **Estat**: `m_mana`, `m_maxMana`, `m_known` (`unordered_map<string,bool>` com a set, per deixar la porta oberta a guardar-hi més coses).
- **API**: `learn`, `knows`, `knownSkillIds()` (còpia **ordenada alfabèticament** a propòsit: la IA enemiga la recorre i sense ordre fix la tria no seria determinista), `mana`, `maxMana`, `setMaxMana`, `canUse`, `spend` (no revalida), `restoreMana`.
- **Flux**: un per enemic (propietat de `WorldEnemy`) i un del jugador (propietat de l'Application, passat per punter).

### `SkillCatalog` — *Skill.h* (línies 103-115) / *Skill.cpp*
- **Què és**: catàleg de `Skill` per id.
- **Herència**: **`public ICatalog<Skill>`** (fractura #2 — mateix contracte `has`/`find`/`size` que `ResourceManager` i `ObjectCatalog`).
- **Estat**: `m_skills` (`unordered_map<string, Skill>`).
- **API**: `add(Skill)`, `has()` `override`, `find()` `override` (nullptr si no hi és), `size()` `override`.
- **Observacions**: no té càrrega des de JSON — l'Application registra les tres habilitats del joc a mà (Application.cpp:259-271) amb un TODO explícit.

### `SkillApplyResult` + funcions lliures — *Skill.h* (línies 121-161) / *Skill.cpp* (62-192)
- **Què és**: el resultat enriquit d'aplicar dany/curació: `hpChange` real (amb clamp) i `degree` (0=BOTCH … 3=CRITICAL).
- **API lliure**: `ApplyBasicAttackNd6(caster, target, rng)` i `ApplySkillEffect(skill, caster, target, rng)`, que adapten la `Skill` llegada a una `RPG::SkillDefinition` temporal (`casting_stat` DES per a dany, INT per a curació; magnituds B/P/S/C = 0 / power÷2 / power / power×1.5) i deleguen a `SkillExecutor`. No descompten mana.
- **Observacions**: la funció aplica l'efecte sobre el `target` que li passin, sense mirar `skill.target` — el criteri de qui és l'objectiu queda íntegrament en mans de `BattleState`, que sempre passa un enemic.

---

### `SavedWorldObject` — *SaveGame.h* (línies 43-46)
- **Què és**: un objecte que encara és al món. La identitat és la **posició** (props i pickups no es mouen); `objectId` hi va com a comprovació d'integritat.
- **Estat**: `objectId`, `position`.

### `SavedEnemy` — *SaveGame.h* (línies 55-60)
- **Què és**: un enemic viu. Aquí la identitat **sí** que és un índex (`spawnIndex`) perquè els enemics patrullen i la cel·la no diu qui són.
- **Estat**: `spawnIndex`, `objectId`, `position`, `health` (perquè fugir d'un combat a mitges no regali la vida ja treta).

### `SaveGameData` — *SaveGame.h* (línies 62-94) / *SaveGame.cpp*
- **Què és**: el DTO del guardat i res més: dada plana que no coneix `TileMap`, ni `ObjectCatalog`, ni l'Application. Guarda el que el jugador ha **canviat**, mai el contingut (mapa, catàleg i definició de nivell es tornen a llegir de disc).
- **Herència**: cap.
- **Estat**: `static constexpr kSchemaVersion = 1` + `schemaVersion`; `levelPath`; `playerPosition`/`playerHealth`/`playerMaxHealth`; `inventory`, `gold`, `moralityValue`, `stepsSinceRent`; `businesses`, `worldObjects`, `enemies`; `narrativeFlags`, `narrativeBeatsFired`, `narrativeTimers`.
- **API lliure associada**: `serializeSaveGame()` (mai falla), `parseSaveGame()` (valida versió i `levelPath` **abans** de tocar res, i omple una còpia local que només s'entrega al final), `applyNarrativeState()` (idempotent, només encén — existeix separada perquè qui carrega ha de preparar la narrativa *abans* de carregar el nivell o el beat d'entrada es repetiria), `writeSaveGameFile()` (escriptura **atòmica** via `.tmp` + `rename`, en binari perquè el fitxer sigui byte a byte igual a les tres plataformes), `readSaveGameFile()`.
- **Flux**: `Application::quickSave/quickLoad` (Application.cpp:1264, 1302) → `captureSaveGame`/`restoreSaveGame`. `restoreSaveGame` és tot-o-res: fase 1 valida les dues llistes senceres sense escriure (posició+id per als objectes, `spawnIndex`+id per als enemics), fase 2 aplica. La vida del jugador es restaura curant al màxim i aplicant el dany que falta, perquè `ICombatant` no té setter de vida a propòsit.
- **Observacions**: cobert per `tests/Game/SaveGameTests.cpp`, l'únic test unitari del bloc Game.

---

### `EnemySpawn` — *LevelDefinition.h* (línies 19-41)
- **Què és**: el spawn d'enemic de la Fase 6, amb `type` contra un "catàleg d'enemics" que ja no existeix.
- **Estat**: `type`, `position`, `patrolMin`, `patrolMax`, `skillIds`.
- **Flux**: el parseja `LevelLoader` (LevelLoader.cpp:44-67) i el llegeix **només `examples/demo_level_loader.cpp`**. Ni `GameSession` ni `Application` ni l'editor no el toquen.
- **Observacions**: supervivència per compatibilitat, tal com documenta `LevelDefinition.h:118-125` i `GameSession.h:49-56` — **verificat**: és codi de contingut mort al runtime.

### `ObjectSpawn` — *LevelDefinition.h* (línies 52-106)
- **Què és**: la generalització de la Fase 10 ("tot és un objecte"): quin objecte (`objectId` contra `ObjectCatalog`) i on, més les dades que són d'*aquesta instància* i no del tipus.
- **Estat**: `objectId`, `position`, `patrolMin/Max`; transició (`targetLevel`, `targetPosition`, `hasTargetPosition`); botí (`loot`, `lootTable`); `eventId`; variant local (`displayName`, `variant`, `scale`, `effectOverride`, `properties`).
- **Què llegeix realment el joc** (verificat per grep): `objectId`, `position`, `patrolMin`, `patrolMax` (GameSession.cpp:30-49), `targetLevel`/`targetPosition`/`hasTargetPosition` (GameSession.cpp:552-556) i `scale` (Application.cpp:873, per a l'escala del marcador de render).
- **Què és només autoria**: `loot`, `lootTable`, `eventId`, `displayName`, `variant`, `effectOverride`, `properties` — els escriuen i llegeixen exclusivament `EditorState`, `ObjectInspector` i els seus demos. El comentari de `loot` ho admet ("el runtime ho farà quan arribi el sistema de botí"); el d'`eventId` també ("el runtime els consumirà després"). Els de `displayName`/`variant`/`effectOverride` diuen "variant local de l'arquetip" sense advertir que el runtime els ignora: `GameSession` sempre fa servir `def->name` del catàleg, mai `spawn.displayName`.

### `LevelDefinition` — *LevelDefinition.h* (128 línies)
- **Què és**: les dades d'un nivell jugable: nom, ruta al TMX (referenciada, no carregada) i els dos arrays de spawns.
- **Estat**: `name`, `mapPath`, `playerStart`, `enemies`, `objects`.
- **Flux**: la produeix `LevelLoader`, la consumeixen `Application::loadLevel` (que en muta `playerStart` abans de moure-la a la sessió, Application.cpp:775) i `WorldTopology`.
- **Observacions**: `name` no es mostra enlloc del joc (només l'usa `WorldTopology` per etiquetar nodes).

### `LevelLoader` — *LevelLoader.h* (26) / *LevelLoader.cpp* (159)
- **Què és**: el parser JSON → `LevelDefinition`. GL-free i sense excepcions.
- **Herència**: cap; classe només amb **`static`**.
- **API**: `static Result<LevelDefinition> loadFromFile(path)` i `static ... loadFromString(jsonText)` (separada per poder provar l'esquema sense tocar disc).
- **Flux**: `Application::loadLevel`, `WorldTopology::load`, editor i demos.
- **Observacions**: repartiment explícit obligatori/opcional — rebutja el nivell si falta `map`, si `enemies`/`objects` no són arrays o si un spawn no té `type`/`objectId`; tota la resta té valor per defecte. **No valida cap id** contra cap catàleg (ni `objectId`, ni els de `loot`, ni `targetLevel`): resoldre'ls és del consumidor, criteri que `GameSession` respecta tractant els ids desconeguts com a objectes inerts.

### `CombatData` / `PickupData` / `ShopItem` / `ShopData` / `DialogueData` / `BusinessData` — *ObjectCatalog.h* (47-123)
- **Què són**: els blocs de dades per categoria dins d'`ObjectDefinition`. Són membres plans sempre presents (no un `variant`) i només tenen sentit si la categoria coincideix.
- **Estat**: `CombatData{maxHealth=10, maxMana=0, skillIds}` (llegit pel ctor de `GameSession`); `PickupData{effect, power, price}` (`effect` el consumeix `BattleState::resolveItemAction`, `price` el consumeixen `buy`/`sell`); `ShopItem{objectId, price}` (price ≤ 0 = usa el del catàleg); `ShopData{items, buybackPercent=50}`; `DialogueData{lines}` (sense branques ni condicions, per decisió anti sobre-enginyeria); `BusinessData{price, baseIncome, businessName}` — el lloguer **no** hi és perquè és estat de partida (`OwnedBusiness`).
- **Observacions**: `price` es parseja **fora** del bloc `pickup` a propòsit (ObjectCatalog.cpp:166-169), de manera que qualsevol categoria pot tenir preu.

### `ObjectDefinition` — *ObjectCatalog.h* (línies 125-157)
- **Què és**: l'entrada del catàleg: què és un objecte, com es dibuixa i quines dades per categoria porta.
- **Estat**: `id`, `name`, `category`, `spriteId` (frame d'atles, -1 = sense sprite), `previewPng` (autoria), `raceId`, `blocksMovement`, `interactable`, i els cinc blocs de dades.
- **Observacions**: `interactable` no el consulta mai el runtime — `GameSession::interact()` decideix per `category == Npc` o per `business.price > 0`; qui el llegeix és el validador de l'editor (`EditorValidation.h:162,179`). El comentari diu que els NPCs bloquegen el pas, però això depèn que el JSON posi `blocksMovement`: el codi no ho imposa.

### `ObjectCatalog` — *ObjectCatalog.h* (210) / *ObjectCatalog.cpp* (227)
- **Què és**: el catàleg genèric d'objectes per id, GL-free.
- **Herència**: **`public ICatalog<ObjectDefinition>`**.
- **Estat**: `m_objects` (`unordered_map`).
- **API**: `add()` (`insert_or_assign`: l'últim guanya); `has`/`find`/`size` `override`; per a autoria `ids()`, `ids(category)`, `missingPreviewIds()` (totes ordenades); `loadFromString`/`loadFromFile` (fan **merge**, no buiden, per poder apilar fitxers base + d'àrea); `exportJson()` (camí de tornada de l'editor, amb ordre estable per no barallar el fitxer al control de versions).
- **Flux**: el crea i el carrega `Application::init`; el consulten `GameSession` (categoria, col·lisió, combat, pickup, diàleg, botiga, negoci), `BattleState` (ítems) i tot l'editor.
- **Observacions**: la càrrega és tot-o-res (es parseja a un buffer local i només s'aplica si les N entrades són vàlides), i les categories/efectes desconeguts són error dur, no un default silenciós. Coherent i ben tancat.

### `WorldNode` / `WorldLink` / `WorldTopology` — *WorldTopology.h* (137 línies, header-only)
- **Què és**: el graf de viatges entre nivells, construït des d'un o més nivells arrel seguint recursivament les portes (`ObjectSpawn::targetLevel`).
- **Herència**: cap.
- **Estat**: `WorldNode{levelPath, name, mapPath}`; `WorldLink{sourceLevelPath, sourcePosition, objectId, targetLevelPath, targetPosition, hasTargetPosition}`; la classe té `m_nodes`, `m_links`, `m_indices`.
- **API**: `static Result<WorldTopology> load(rootLevels, maxNodes=512)`, `nodes()`, `links()`, `size()`, `contains()`, `canReach(source, target)` (BFS; `source == target` és assolible).
- **Flux**: el fan servir el validador de contingut, `demo_mundo` i `tests/Level/WorldTopologyTests.cpp`. **No el fa servir el runtime**: `Application` no en construeix cap i les transicions es resolen porta a porta.
- **Observacions**: conté una lliçó ja apresa i documentada — el bucle fa `push_back` sobre el vector que recorre, i mantenir-hi una referència penjava `demo_mundo` amb un `std::length_error`; per això la ruta es copia per valor (línies 60-69 i 116-118). `maxNodes` protegeix contra manifests corruptes.

---

### `EntityHandle` — *World/EntityHandle.h* (25 línies)
- **Què és**: identitat efímera d'una entitat viva a `World`: índex per a cerca compacta + generació perquè un handle d'una entitat destruïda no apunti per accident a la que ha reutilitzat el seu slot.
- **Herència**: cap. Namespace `Engine`.
- **Estat**: `index` (`kInvalidIndex` = max uint32), `generation` (0 = invàlid).
- **API**: `valid()`, `operator==`/`!=` (friends), `static constexpr kInvalidIndex`.

### `WorldEntity` — *World/World.h* (línies 18-21)
- **Què és**: l'entitat runtime mínima de RUN-04. Els components arribaran a RUN-05 i explícitament no es barregen aquí.
- **Estat**: `ids::EntityId authoringId` (opcional: dos NPCs poden compartir prefab però mai handle), `enabled`.

### `World` — *World/World.h* (82) / *World/World.cpp* (134)
- **Què és**: el propietari GL-free d'entitats runtime amb handles generacionals i altes/baixes diferides.
- **Herència**: cap. Namespace `Engine`.
- **Estat**: `m_slots` (**`deque`** a propòsit: manté vives les referències quan un callback demana un spawn durant `fixedUpdate`), `m_freeSlots`, `m_pendingCreates`, `m_pendingDestroys`, `m_size`, `m_isUpdating`. `Slot{optional<WorldEntity>, generation, reservedCreate, destroyRequested}`.
- **API**: `create(entity)` (reserva handle immediatament, però l'entitat no participa fins al flush), `destroy(handle)` (la baixa és visible a l'instant per a `get()`, la memòria s'allibera després), `contains`, `get()` (const i no-const), `size`, `empty`, `pendingCreateCount`, `pendingDestroyCount`, `flushPendingChanges()` (pública per poder inicialitzar abans del primer tick; és no-op mentre `m_isUpdating`), `fixedUpdate(UpdateFunction)` (recorre índexs creixents → ordre estable; flush abans i després). Privats: `matches`, `isLive`, `reserveSlot`, `releaseSlot` (incrementa generació saltant el 0).
- **Flux**: **cap**. `add_library(motor_world src/World/World.cpp)` (CMakeLists.txt:132) i l'únic `target_link_libraries` és `test_world` (:731). Cap fitxer de `src/`, `examples/` o `editor/` inclou `World/World.h`; `Engine::World` només apareix a `tests/World/WorldTests.cpp`.
- **Observacions**: és una peça nova, correcta i provada, **completament desconnectada** del joc. El motor real segueix guardant les entitats a `GameSession` (`m_worldObjects`, `m_enemies`) i a `Application` (`m_worldMarkers`), amb reconciliació per índex. La documentació ho emmarca com a pas previ ("el render queda fora fins al proxy de RUN-05"), però avui hi ha dues nocions de "món" al repositori que no es parlen.

---

### Buits i riscos del mòdul

- **La curació cura l'enemic.** `BattleState::resolveAllyAction` valida i escull sempre l'objectiu dins de `m_enemies` (BattleState.cpp:92-97) i li passa a `ApplySkillEffect` (:111-112) sense mirar `Skill::target`, que ningú no llegeix (Skill.h:33). Al joc real, l'ordre "Cura" del menú de combat (Application.cpp:1230, habilitat registrada com a `SkillTarget::Self` a :261) gasta 4 PM del jugador i cura ~20 PV **al monstre**. És el bug funcional més greu del bloc.
- **Un enemic invocat per narrativa fa el guardat il·legible.** `resolveBattles` crea el `WorldEnemy` sense assignar `spawnIndex` (GameSession.cpp:354-356), que queda a -1. Si el jugador fuig d'aquesta emboscada i guarda, `captureSaveGame` desa un `SavedEnemy` amb `spawnIndex == -1` (SaveGame.cpp:341) i `restoreSaveGame` no el trobarà mai a la sessió acabada de construir → "el guardado no encaja con el nivel: enemigo fuera de rango (-1)" (SaveGame.cpp:393). La partida queda inservible per una decisió de contingut.
- **Creuar una porta esborra la moral i els negocis.** `Application::loadLevel` rescata només l'or de la sessió sortint (Application.cpp:771, 781) i destrueix la resta; `m_morality`, `m_businesses` i `m_stepsSinceRent` són membres de `GameSession` (GameSession.h:433-446). Entrar a una posada reinicia la reputació a 0 i et deixa sense locals. El comentari "l'or pertany al jugador, no al mapa" identifica el problema exacte i només l'arregla per a un camp.
- **Les línies narratives es dupliquen al log.** `applyNarrative` ja registra cada línia (GameSession.cpp:386-389) i `startInteraction` les torna a registrar en sortir (:264-266) quan el beat ha disparat per boca d'un NPC.
- **`restoreSaveGame` neteja mig estat.** SaveGame.cpp:457-462 buida combat, diàleg, botiga i transició, però deixa `m_currentBusinessId/Name/Price/Income`, `m_battleQueue` i `m_pendingBattleVictoryFlag/DefeatFlag` com estaven — càrrega en calent amb un cartell obert o una onada encolada arrossega estat mort.
- **`m_level` és una còpia morta i mentidera.** Es guarda sencera (GameSession.h:409) i després del constructor només se'n llegeix `playerStart`; el comentari de :137-139 afirma que la sessió la va mutant amb els pickups recollits, cosa que no passa: es queda eternament amb l'estat inicial del nivell.
- **Els enemics lluiten amb stats a zero.** `GameSession` construeix `EnemyBrain` només amb `maxHealth` (GameSession.cpp:41-42) i mai crida `set_stat`/`populate_from_monster_catalog`, així que `stat(DES)`/`stat(CON)` valen 0 (EnemyBrain.h:289). Tot el motor Nd6 (`override_base_dice = 1 + stat`, Skill.cpp:89) queda en el pool mínim per a qualsevol enemic del joc: `CombatData` no té camp d'stats i ningú no ha tancat el pont amb `RPG::MonsterDefinition`.
- **Farmeig de moral amb el lloguer sostingut.** `collectRent` suma `(100 - rentPercent)/25` per negoci i cicle (GameSession.cpp:529-534): amb el lloguer al mínim (50%) són +2 de moral cada 40 passes, indefinidament. La protecció contra el farmeig només es va posar al canvi de lloguer (`adjustCurrentRent`, comentari a :498-501), no al cobrament.
- **L'RNG de la sessió no arriba al combat.** `GameSession::setRandomEngine` (GameSession.h:229) només cobreix les tirades narratives; `startBattle` no propaga res al `BattleState` (GameSession.cpp:688). Un combat disparat per un beat no és reproduïble sense tocar `session.battle()` des de fora just després (demo_game_session.cpp:169).
- **El venedor es busca per nom.** `sell()` recorre `m_worldObjects` comparant `def->name == m_dialogueSpeaker` per recuperar el `buybackPercent` (GameSession.cpp:657-664). Dos NPCs amb el mateix nom visible donen el percentatge equivocat, i el propi comentari admet que és l'única dada que es va desar en obrir la botiga.
- **`patrolMin.y`/`patrolMax.y` es parsegen i es llencen.** `EnemyBrain::stepOnce` només mou l'eix X (EnemyBrain.h:259-276). És exactament la mateixa classe de deute que la capçalera d'`EnemyBrain` presumeix d'haver eliminat ("el dato de patrulla se leía y se tiraba"), reduïda a una dimensió. `kChase` (EnemyBrain.h:51) no s'assigna mai.
- **`Morality::label()` incompleix el seu propi contracte**: promet cadenes sense accents per a la BitmapFont i retorna `"HÉROE"` (Morality.cpp:45). `setValue()` tampoc no buida l'historial, així que una partida carregada mostra motius que no li pertanyen.
- **`World/` no està connectat.** Només `test_world` l'enllaça (CMakeLists.txt:132, 731) i cap fitxer de producció inclou `World/World.h`. Mentrestant hi ha una segona representació d'entitats de món a `GameSession` + `Application::m_worldMarkers`. Com més creixi cadascuna per separat, més cara serà la fusió.
- **Camps documentats com a runtime que només són autoria.** `ObjectSpawn::loot`, `lootTable`, `eventId`, `displayName`, `variant`, `effectOverride` i `properties` (LevelDefinition.h:86-105) els escriu l'editor i no els llegeix ningú al joc — verificat per grep. El comentari de `displayName`/`variant` ("així una mateixa poció pot ser curativa, de mana o de missió") descriu una capacitat que el runtime no implementa.
- **`LevelDefinition::enemies` és contingut mort.** Només el llegeix `examples/demo_level_loader.cpp:111`. La compatibilitat que justifica mantenir-lo (LevelDefinition.h:118-125) no la fa servir cap nivell jugable: el retard a retirar-lo manté viu un camí de parseig (LevelLoader.cpp:40-67) que ningú no consumeix.
- **Cobertura desigual.** El bloc té un sol test unitari (`tests/Game/SaveGameTests.cpp`) més `WorldTopologyTests` i `WorldTests`; tota la resta — màquina d'estats de `GameMode`, economia de negocis, moral, botiga, interacció — es prova via demos promoguts a CTest (`demo_game_session`, `demo_ciudad`). Els bugs de la curació i de la moral perduda en canviar de nivell són precisament de les vores que cap demo travessa.
