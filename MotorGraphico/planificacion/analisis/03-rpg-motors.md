## Mòdul Motors RPG

El bloc conté sis motors sense estat (o gairebé) que converteixen dades de catàleg en regles jugables: `DicePoolEngine` és l'aritmètica de daus Nd6 (l'única forma legítima de resoldre qualsevol acció), `SkillExecutor` és l'orquestrador d'una carta d'habilitat (context → pool → grau → magnitud), `CharacterSheet` és la fitxa única de veritat sobre la qual escriuen `InventoryEngine` (equip/slots/bonificadors) i `ConditionEngine` (condicions amb stacks i tick per ronda), `CharacterCreation`+`CharacterCreationScreen` construeixen aquesta fitxa des dels catàlegs amb un flux de 14 passos sense finestra, i `NarrativeEngine` és la línia d'esdeveniments: un conjunt de flags i una llista de beats que es disparen per trigger. La direcció de dependències és estricta i unidireccional: tots són GL-free, viuen a la llibreria `motor_rpg` i cap coneix `GameSession`; és `GameSession` qui els endolla (`setNarrative`, `setNd6SkillCatalog`, `setRandomEngine`) i qui executa el que el motor narratiu declara però no pot aplicar (or, tirades, combats). `Application` és propietària de `NarrativeEngine` + `NarrativeState` + els quatre `AdventureScript` de Boundington i els reenganxa a cada `GameSession` nova quan es canvia de nivell (`Application.cpp:789-794`), a més de calcular el `dtActivo` dels temporitzadors (`Application.cpp:1518-1526`). El pont narrativa↔daus passa sempre pel mateix `DicePoolEngine` que fa servir el combat, de manera que una prova de percepció i un cop d'espasa comparteixen regla.

---

### `TimerState` — *NarrativeEngine.h:63-69* (7 línies)
- **Què és**: la fotografia viva d'un temporitzador de missió en curs (F1, bitàcola 2.18). No és una flag perquè té magnitud contínua: el temps que queda.
- **Herència**: cap.
- **Estat**: `id`; `restante` (segons ACTIUS pendents); `activo` (arrencat i no expirat); `expirado`; `avisosDados` (vector paral·lel a `TimerDefinition::avisos`, un bool per llindar ja creuat).
- **API**: cap (struct de dades pur).
- **Flux**: el crea `NarrativeState::timerRef()` sota demanda, l'omple `NarrativeEngine::startTimer()`, l'avança `tickTimers()`, el llegeix el HUD via `NarrativeState::timers()` i el serialitza `SaveGame.cpp:117-129` / el restaura `SaveGame.cpp:246-252`.
- **Observacions**: `avisosDados` només es dimensiona a `startTimer()`; la restauració des de guardat copia el vector tal com estava sense reajustar-lo a la definició actual (risc detallat més avall).

### `NarrativeState` — *NarrativeEngine.h:71-104* (34 línies)
- **Què és**: TOT l'estat narratiu d'una partida: un conjunt de flags de text lliure, el registre de beats ja disparats i els temporitzadors vius. És l'única font de veritat del progrés d'una aventura.
- **Herència**: cap.
- **Estat**: `m_flags` (`unordered_set<string>` — strings i no enum perquè el contingut JSON pugui inventar flags noves sense recompilar); `m_beatsFired` (comptabilitat interna del motor per a `once`, deliberadament separada de les flags); `m_timers` (`vector<TimerState>`).
- **API**:
  - Flags: `hasFlag`, `setFlag`, `clearFlag`, `flagCount`, `flags()` (còpia ORDENADA — mai s'exposa el set intern per referència).
  - Beats: `beatFired`, `markBeatFired`, `beatsFired()` (còpia ordenada, per guardar).
  - Temporitzadors: `timerRef(id)` (referència viva, crea l'entrada si no existeix), `timers()` (còpia per al HUD).
  - Cicle: `reset()` buida les tres estructures.
- **Flux**: el construeix `Application` (`Application.h:202`) i viu més que qualsevol `GameSession`; el muta el `NarrativeEngine` (setFlag/clearFlag/timers) i TAMBÉ `GameSession` (flags de grau de tirada, `GameSession.cpp:315-318`, i flags de victòria/derrota, `GameSession.cpp:744-747`) — hi ha dos escriptors declarats i assumits. `SaveGame` el fotografia sencer.
- **Observacions**: `timerRef()` retorna una referència a l'interior d'un `std::vector`, que qualsevol `push_back` posterior invalida. Avui cap caller en guarda dues alhora, però és una API fràgil de fer servir malament.

### `NarrativeCondition` — *NarrativeEngine.h:117-126* (10 línies)
- **Què és**: la condició que un beat imposa sobre les flags per poder disparar-se.
- **Herència**: cap.
- **Estat**: `allFlags` (AND), `anyFlags` (OR), `notFlags` (NOR). Les tres llistes es combinen amb AND entre elles. Buida = sempre certa, i és el cas més comú (el JSON pot ometre `requires` sencer).
- **API**: `isSatisfiedBy(state)`, `isEmpty()`.
- **Flux**: la parseja `AdventureScript::loadFromString` (`NarrativeEngine.cpp:223-226`), l'avalua `NarrativeEngine::fire` (`:434`).
- **Observacions**: cap. La implementació (`:119-143`) coincideix exactament amb el que documenta.

### `BeatTrigger` / `enum class TriggerType` — *NarrativeEngine.h:146-151* (6 línies)
- **Què és**: què fa que un beat entri en joc.
- **Herència**: cap.
- **Estat**: `type` (`Talk` / `Enter` / `Auto`), `target` (objectId de l'NPC per a Talk, ruta o id de nivell per a Enter, buit per a Auto).
- **API**: cap.
- **Flux**: `Talk` el dispara `GameSession::startInteraction` (`GameSession.cpp:231`) i `talkNarrative`; `Enter` el dispara `Application::loadLevel` via `enterLevelNarrative` (`Application.cpp:794`); `Auto` el dispara `NarrativeEngine::tick()` des de `GameSession::closeInteraction` (`GameSession.cpp:616-617`) i des de `updateNarrativeTimers` (`GameSession.cpp:416`).
- **Observacions**: **els tres valors estan implementats de debò a `fire()`** (`:422-430`) — cap trigger declarat i ignorat. Ara bé, `Pickup`, `Defeat` i `Temporal`, que el comentari `:143-145` dóna per "els tres següents a afegir", NO existeixen a l'enum, i el parseig és estrictament tancat a talk/enter/auto (`:31-45`): les cartes de `assets/catalogs/events.json` que ja porten `trigger.type: temporal` farien fallar la càrrega sencera de l'aventura si s'hi importessin.

### `enum class EffectType` + `NarrativeEffect` — *NarrativeEngine.h:165-240* (76 línies)
- **Què és**: què canvia al món quan un beat es dispara. `NarrativeEffect` és la instància: tipus + `arg` (string) + `value` (int) + dos sub-blocs opcionals.
- **Herència**: cap.
- **Estat**: `type`, `arg`, `value`, `skillCheck` (només si `SkillCheck`), `battle` (només si `StartBattle`).
- **API**: cap; el parseig viu a `parseEffectType` (`NarrativeEngine.cpp:47-77`).
- **Flux — quins valors estan implementats de debò a `fire()`** (`NarrativeEngine.cpp:450-483`): els **set** valors tenen `case` propi i cap cau al buit. Ara bé, només tres es resolen dins del motor; la resta són DIFERITS i només tenen efecte real si algú aplica el `NarrativeResult`:
  - `SetFlag` / `ClearFlag` → `state.setFlag/clearFlag` — **aplicat aquí mateix**.
  - `StartTimer` → `startTimer(effect.arg, state)` — **aplicat aquí mateix** (el rellotge viu al `NarrativeState`, que ja el té a mà).
  - `GrantGold` → només acumula a `result.goldDelta`; l'aplica `GameSession::applyNarrative` amb `addGold` (`GameSession.cpp:372`).
  - `Log` → només empeny a `result.log`.
  - `SkillCheck` → només empaqueta a `result.skillChecks`; el resol `GameSession::resolveSkillChecks` (`GameSession.cpp:277-318`) amb `DicePoolEngine`.
  - `StartBattle` → només empaqueta a `result.battles`; el materialitza `GameSession::resolveBattles` i les flags de desenllaç no s'encenen fins a `syncBattleOutcome` (`GameSession.cpp:744-747`).
- **Observacions**: qui cridi `fire()` directament sense passar per `applyNarrative` (tots els demos de `examples/`) perd or, tirades i combats en silenci. A més, el missatge d'error de tipus desconegut (`NarrativeEngine.cpp:241`) enumera els vàlids però **s'ha oblidat `startTimer`**: el contingut que s'equivoqui rebrà una llista incompleta.

### `SkillCheckRequest` i `BattleRequest` — *NarrativeEngine.h:212-230* (19 línies)
- **Què són**: les dues peticions que el motor narratiu empaqueta per a l'orquestrador.
- **Herència**: cap.
- **Estat**: `SkillCheckRequest` → `skillId` (el seu `casting_stat` fixa N dels Nd6), `rivalDice` (Ed. 3: la prova és enfrontada), `cd` (float 0.0–3.0, **EN DESÚS**, només compatibilitat), i quatre flags mútuament excloents `flagBotch/flagPartial/flagSuccess/flagCritical` — s'encén NOMÉS la del grau obtingut, cosa que permet ramificar en quatre variants per tirada. `BattleRequest` → `monsterId` (ObjectDefinition de categoria enemy), `flagVictory`, `flagDefeat`.
- **API**: cap.
- **Flux**: el parseig valida que els cinc camps del skillCheck siguin no buits, `rivalDice` ∈ [0,24] i `cd` ∈ [0,3] (`NarrativeEngine.cpp:251-287`); i que les tres del battle siguin no buides (`:289-305`). La conversió de compatibilitat `cd → rivalDice` (0.5→1 … 2.5→5) la fa `GameSession.cpp:296-301`, no el motor.
- **Observacions**: `cd` encara es valida amb rang i missatge d'error com si fos canònic tot i estar marcat en desús; és un camp mort que el contingut nou pot omplir per error creient que fa alguna cosa.

### `NarrativeBeat` — *NarrativeEngine.h:246-266* (21 línies)
- **Què és**: la unitat de contingut, la "carta" del Llibre VI del GDD: un trigger, unes condicions, unes línies i uns efectes.
- **Herència**: cap.
- **Estat**: `id` (únic dins l'aventura, només per a `once`/depuració), `trigger`, `requirements`, `speaker` (buit = veu del narrador: el HUD decideix si dibuixa marc de diàleg), `lines`, `effects`, `once`.
- **API**: cap.
- **Flux**: `fire()` recorre els beats EN ORDRE D'ARXIU i el primer que encaixa guanya; els efectes s'apliquen en ordre i DESPRÉS d'haver decidit el beat, de manera que una flag encesa per un efecte només pot activar un altre beat al disparament SEGÜENT (això és el que evita cascades on parlar amb un NPC recorre mitja aventura).
- **Observacions**: `state.markBeatFired(beat.id)` s'executa per a TOTS els beats (`:485`), no només els `once`; el set creix sense límit i es guarda sencer al savegame, encara que només els `once` el consultin.

### `ObjectiveDefinition` i `ObjectiveStatus` — *NarrativeEngine.h:276-286* (11 línies)
- **Què són**: el diari de missions. Un objectiu no té lògica pròpia: és una etiqueta de text lligada a una flag (`doneFlag`); el `Status` hi afegeix el `done` resolt contra l'estat actual. El diari és una VISTA de les flags, no una segona font de veritat.
- **Herència**: cap.
- **Estat**: `id`, `text`, `doneFlag` / `id`, `text`, `done`.
- **API**: cap; els resol `NarrativeEngine::objectives()` i `allObjectivesDone()`.
- **Observacions**: la càrrega rebutja un objectiu sense `doneFlag` (`NarrativeEngine.cpp:184-189`) perquè mai es podria completar. Correcte i explícit.

### `TimerDefinition` (+ `Presentacion`, `Aviso`) — *NarrativeEngine.h:317-335* (19 línies)
- **Què és**: la definició de contingut d'un temporitzador de missió: temps NARRATIU, no calendari ni segons crus (la casa que crema, el rescat, l'assassí que torna a matar).
- **Herència**: cap; conté l'enum imbricat `Presentacion { Contador, Barra, Secreto }` i l'struct `Aviso { fraccion, flag }`.
- **Estat**: `id`; `duracion` (segons ACTIUS); `arrancaPor` (flag que l'arrenca, alternativa a l'efecte `StartTimer`); `expiraPorTiempo` / `expiraPorSalirDeZona` (poden ser totes dues); `zona` (nivell on corre); `flagExpira` (obligatòria: sense ella ningú s'assabenta que ha expirat); `avisos` (llindars sobre la fracció de temps RESTANT, cadascun amb la seva flag); `presentacion` (comptador, barra o SECRET — el jugador només ho nota pel que passa al món).
- **API**: cap.
- **Flux**: la parseja `loadFromString` (`NarrativeEngine.cpp:312-389`) amb validació estricta: `flagExpira` obligatòria, `expiraPor` només accepta `tiempo`/`salir_de_zona`, zona obligatòria si expira per sortida, `duracion > 0` si expira per temps, `fraccion ∈ (0,1]`, presentació d'una llista tancada. El camp `timers` és opcional: les aventures antigues carreguen igual.
- **Observacions**: `presentacion` es parseja i es valida però **cap consumidor del repositori la llegeix**: el HUD no distingeix entre Contador, Barra i Secreto, de manera que un timer marcat `secreto` avui no és més secret que un altre. És l'únic camp declarat i completament ignorat del bloc F1.

### `AdventureScript` — *NarrativeEngine.h:337-352* (16 línies de declaració; càrrega a *NarrativeEngine.cpp:149-408*, 260 línies)
- **Què és**: el fitxer de contingut jugable — el GUIÓ. Es diu Script i no Definition perquè `RPG::AdventureDefinition` ja existeix i és una altra cosa: la CARTA d'aventura del catàleg (adv_10 i les seves 300 germanes), amb tier, facció, enemics i loot tables. Aquella és la fitxa; això és la partida.
- **Herència**: cap.
- **Estat**: `id`, `name` (per defecte l'id), `description`, `objectives`, `beats`, `timers`.
- **API**:
  - Càrrega: `static Result<AdventureScript> loadFromString(jsonText)` i `static loadFromFile(path)`. Mateix contracte que `ObjectCatalog`/`LevelLoader`: `Result<T>` i mai excepcions (és contingut, no arrencada), i **tot-o-res** — es construeix en una còpia local i només es retorna si TOT ha parsejat, perquè mitja aventura carregada és una partida encallada sense dir per què. `loadFromFile` afegeix la ruta al missatge d'error, perquè "beats[3]" tot sol no localitza res.
- **Flux**: qui la construeix és `Application::inicializar` amb quatre fitxers fixos (`Application.cpp:362-365`: pròleg, primer dia, segon dia, ocàs) més una aventura opcional per paràmetre (`:376`); `NarrativeEngine::setAdventure` en pren un punter NO propietari que ha de sobreviure el motor.
- **Observacions**: `id` de beat es valida com a no buit però **no es valida la unicitat**: dos beats amb el mateix id comparteixen la comptabilitat de `once` i el segon no es dispararà mai un cop el primer ho hagi fet. Tampoc es valida que un `startTimer` apunti a un timer declarat (queda no-op silenciós, `:509-511`), ni que les flags d'un `requires` les encengui algú: un error tipogràfic en una flag és un beat que mai dispara, que és exactament el bug de contingut que el parseig estricte de triggers volia evitar.

### `NarrativeResult` — *NarrativeEngine.h:358-378* (21 línies)
- **Què és**: el que retorna un disparament: el que s'ha de dir més la llista d'efectes que el motor NO pot aplicar per si mateix perquè no són seus.
- **Herència**: cap.
- **Estat**: `fired` (false = cap beat encaixava; l'NPC dirà el seu diàleg de catàleg), `beatId`, `speaker`, `lines`, `goldDelta` (l'or viu a `GameSession`), `log`, `skillChecks` (plural: un beat pot demanar més d'una tirada), `battles` (plural pel mateix motiu, encara que avui només un pot estar viu i la resta s'encuen).
- **API**: cap.
- **Flux**: el produeix `fire()`, el consumeix `GameSession::applyNarrative` (`GameSession.cpp:371-388`), que en un sol lloc: posa speaker/lines, buida ofertes, passa a `GameMode::Dialogue`, aplica or, resol tirades, encua o arrenca combats i bolca log + línies al log de partida.
- **Observacions**: `addGold` fa clamp a 0 (`GameSession.cpp:213`), de manera que un `grantGold` negatiu més gran que el saldo (un peatge) es cobra parcialment i el contingut no se n'assabenta: no hi ha manera que un beat pregunti "tenia prou or?".

### `NarrativeEngine` — *NarrativeEngine.h:380-433* (54 línies; implementació a *NarrativeEngine.cpp:414-591*)
- **Què és**: el bucle sencer del sistema narratiu. Sense estat propi: només un punter no propietari a l'aventura activa; tot l'estat mutable viu al `NarrativeState` que rep per referència, la qual cosa el fa determinista i provable sense finestra.
- **Herència**: cap (ni base ni derivats).
- **Estat**: `m_adventure` (`const AdventureScript*`, `nullptr` = sense aventura → tot retorna `fired=false`, que és exactament el comportament previ a l'existència del sistema).
- **API**:
  - Configuració: `setAdventure`, `adventure()`.
  - Disparament: `fire(type, target, state)` (el bucle sencer) i el sucre `talkTo`, `enterLevel`, `tick`.
  - Temporitzadors (F1): `tickTimers(state, dtActivo, nivelActual)`, `startTimer(timerId, state)`.
  - Diari: `objectives(state)`, `allObjectivesDone(state)`.
  - Cap mètode és `virtual` ni `static`; tots són `const` (muten només el `state` que reben).
- **Flux — el cicle sencer**:
  1. **Trigger**: passa alguna cosa al món. `GameSession::startInteraction` crida `talkTo(def.id)` amb l'id de l'NPC; `Application::loadLevel` crida `enterLevelNarrative(levelPath)` → `enterLevel`; `GameSession::closeInteraction` i `updateNarrativeTimers` criden `tick()` (Auto, sense target).
  2. **Selecció**: `fire` recorre `m_adventure->beats` en ordre d'arxiu i descarta el beat si el tipus de trigger no coincideix (`:422`), si el target no coincideix — excepte Auto, que l'ignora explícitament (`:428`) —, si és `once` i ja s'ha disparat (`:431`), o si les `requirements` no es compleixen (`:434`). **El primer que encaixa guanya**, i és deliberat: el contingut es llegeix de dalt a baix com una llista de casos especials seguits del cas general.
  3. **Efectes**: s'apliquen en ordre sobre l'estat i sobre el resultat (vegeu `EffectType` més amunt): flags i timers al `NarrativeState`, or/log/tirades/combats al `NarrativeResult`.
  4. **Comptabilitat**: `markBeatFired(beat.id)` i retorn immediat — **un disparament, un beat**.
  5. **NarrativeResult** puja a `GameSession::applyNarrative`, que reparteix: l'or a `addGold`, els `skillChecks` a `resolveSkillChecks` (que tira la pool del jugador amb `m_playerBody->stat(def->casting_stat)` contra `rivalDice` i encén una de les quatre flags de grau al MATEIX `NarrativeState` — el resultat ja és visible en tornar), els `battles` a `resolveBattles` (que materialitza l'enemic al costat del jugador, passa a mode Battle i APUNTA les flags pendents; les encén `syncBattleOutcome` molts torns després), i el log al log de partida. Les línies queden a `m_dialogueLines` i el mode passa a `Dialogue`.
  - **Temporitzadors (F1) i com hi encaixa `tickTimers`**: és una passada separada i explícitament fora del cicle de beats. `Application` calcula el `dtActivo` (0.0 si `mode() != Exploration` o si fa més de 15 s sense input — AFK; `Application.cpp:1518-1525`) i el passa amb la ruta del nivell actual a `GameSession::updateNarrativeTimers`, que crida `tickTimers`. Aquest (a) arrenca tots els timers amb `arrancaPor` encesa (`:520-524`), (b) per a cada timer actiu comprova primer l'expiració per SORTIR DE ZONA (`:534-539`) — la casa crema encara que quedés temps —, (c) resta `dtActivo` de `restante` només si `expiraPorTiempo`, (d) encén les flags d'avís en creuar cada llindar (`:549-555`) i (e) en arribar a 0 marca `expirado` i encén `flagExpira`. **El timer no inventa efectes nous**: les conseqüències les disparen els beats `Auto` de sempre, i per això `updateNarrativeTimers` compara `flagCount()` abans i després i, si ha canviat i estem en Exploration, dispara UN `tick()` perquè la conseqüència surti com a diàleg («se senten crits a dalt...») en comptes de passar en silenci.
- **Observacions**: `allObjectivesDone` retorna false quan no hi ha aventura o no hi ha objectius (`:582`), deliberadament, perquè "acabada" i "no n'hi ha" no siguin indistingibles. La resta de deute està a la secció final.

---

### `enum class DiceMod`, `enum class Degree`, `PoolResult`, `CheckOutcome`, `OpposedOutcome` — *DicePoolEngine.h:31-56* (26 línies)
- **Què són**: el vocabulari del motor de daus. `DiceMod {NORMAL, ADVANTAGE, DISADVANTAGE}`; `Degree {BOTCH, PARTIAL, SUCCESS, CRITICAL}` — són quatre i no un booleà perquè la magnitud d'una habilitat depèn del grau (`SkillDefinition::magnitude_by_degree`). `PoolResult` porta `successes` (float: 0, 0.5, 1.0…), `critical` (tots els daus 6), `botch` (Ed. 3: cap 5 ni 6 i mínim dos 1) i `rolls` (els valors reals, per al log i el HUD: una tirada que no es pot veure no es pot discutir). `CheckOutcome` i `OpposedOutcome` són els resultats de les dues formes de resolució.
- **Herència**: cap.
- **Observacions**: `CheckOutcome` (amb el seu camp `cd`) pertany sencer a la via en desús des d'Ed. 3.

### `DicePoolEngine` — *DicePoolEngine.h:58-194* (137 línies; el .cpp és un fitxer mort)
- **Què és**: l'ÚNICA forma de resoldre una acció — atac, salvació, conjur, prova de característica o tirada enfrontada. Substitueix el `target.takeDamage(skill.power)` pla del motor antic.
- **Herència**: cap. Classe purament estàtica i sense estat: l'atzar entra sempre per `RandomEngine&`, de manera que una mateixa llavor dóna sempre la mateixa batalla.
- **Estat**: només `static constexpr float CD_MAX = 3.0f` (sostre de dificultat: sense ell una defensa alta faria impossible encertar mai).
- **API** (tot `static`, tot inline al header):
  - Tirada: `roll_pool(dice, mod, rng)` — clampa `dice` a [0,24]; cada 6 suma 1, cada 5 mig (0 amb desavantatge), cada 4 mig només amb avantatge; `critical` = tots 6 i exigeix `dice > 0`.
  - Pifia: `es_pifia(rolls)` — Ed. 3: cap 5 ni 6 I mínim dos 1. Un pool buit no és pifia: no hi ha daus que la materialitzin.
  - Resolució canònica Ed. 3: `opposed_degree(atacant, defensor)` — pifia pròpia → BOTCH; pifia del defensor → CRITICAL; el defensor guanya → BOTCH (se salva del tot); empat → PARTIAL (mitja magnitud); l'atacant guanya per qualsevol marge → SUCCESS; guanya amb tots 6 → CRITICAL.
  - **En desús des d'Ed. 3**: `resolve_against_cd(pool, cd)`, `resolve_against_defense(pool, defense_value)`, `defense_to_cd(defense_value)` (cada punt sobre 10 són +0.5), `resolve_opposed` (retorna `OpposedOutcome` amb l'empat marcat a part, sense decidir-lo).
- **Flux**: el criden `SkillExecutor::execute` (dues vegades: pool de l'atacant i pool del defensor) i `GameSession::resolveSkillChecks` (`GameSession.cpp:290-305`). Produeix `PoolResult`/`Degree`, mai danys: la magnitud la posa la carta.
- **Observacions**: l'ordre de `resolve_against_cd` (BOTCH comprovat ABANS de comparar amb la CD) està documentat com a resposta a un incident real: amb cd 0.0, un `0 >= 0` avaluat primer feia que TOT atac encertés al tram inicial del joc. `src/RPG/DicePoolEngine.cpp` és un fitxer de 6 línies que declara haver quedat supersedat i **no apareix a `CMakeLists.txt:94-103`**: no es compila, no és un TU, i existeix només com a comentari.

---

### `enum class ContextBonusFlag`, `enum class QteResult`, `IQteProvider` — *EnhanceHooks.h:16-51* (36 línies)
- **Què són**: la capa d'"enhancements" pre-tirada. `ContextBonusFlag` és un bitfield de nou situacions (flanqueig, terreny alt, sorpresa, inspiració, fatiga, equip incompatible, enemic sagnant, llançador a poca vida, `TARGET_CA_FLANKED`) amb `operator|`, `operator&` i `has_flag`. `QteResult {MISSED, GOOD, PERFECT}`.
- **Herència**: `IQteProvider` és una interfície pura (destructor virtual + dos mètodes purs) que implementa `Application` amb el HUD de timing; el motor la CONSULTA i mai dibuixa.
- **API**: `IQteProvider::do_reaction_dodge_qte(window_ms, enemy_skill_id)` i `do_attack_timing_qte(window_ms, skill_id)`, tots dos `virtual` purs.
- **Observacions**: `TARGET_CA_FLANKED` (bit 8) està declarada i **cap línia de `SkillExecutor.cpp` la llegeix**: és un valor d'enum mort.

### `ExecutionContext` — *EnhanceHooks.h:54-98* (45 línies)
- **Què és**: tot el que necessita l'executor per resoldre una carta, i alhora la pissarra on l'executor acumula els seus propis càlculs intermedis (entrada i sortida barrejades a propòsit).
- **Herència**: cap.
- **Estat**: orígens (`caster`, `target` — `CharacterSheet*` no propietaris —, `skill`), `flags`, `qte`, `rng` (OBLIGATORI), `is_player_attacking`; acumuladors (`extra_dice`, `force_dice_mod`, `post_roll_success_bonus`); permisos d'ascens (`can_upgrade_botch_to_partial`, `can_upgrade_partial_to_success`, `perfect_critical_confirm_allowed`); i els overrides per a la capa adaptadora de Render que només té `ICombatant*` (`override_base_dice`, `override_target_dice`, `defender_extra_dice`, `defender_dice_mod`, i `override_target_defense` **en desús i ignorat**).
- **Flux**: l'omple qui munta l'acció (`src/Game/Skill.cpp:112`, `demo_battle_nd6`, `demo_nd6_distribution`) i el passa per referència no-const a `execute`.
- **Observacions**: `force_dice_mod` és una única casella amb semàntica d'"últim que escriu guanya" (`SkillExecutor.cpp:25,34,38,67`): un personatge alhora SORPRENENT i FATIGAT acaba amb desavantatge només perquè la fatiga es comprova després, i un QTE de dodge PERFECT sobreescriu qualsevol desavantatge previ. No hi ha acumulació ni cancel·lació de modificadors.

### `SkillExecutor` (+ `SkillExecutor::Out`) — *EnhanceHooks.h:101-118* / *SkillExecutor.cpp:52-177* (18 + 126 línies)
- **Què és**: l'orquestrador final d'una carta d'habilitat, que garanteix la invariant "SEMPRE es tira una pool Nd6".
- **Herència**: cap; `Out` és un struct imbricat.
- **Estat**: cap (classe estàtica). `Out` porta `pool`, `defender_pool`, `opposed`, `final_degree`, `effective_successes`, `final_magnitude`, `applied_enhancements` (log llegible per al HUD) i `was_all_sixes_critical`.
- **API**: `static Out execute(ExecutionContext& ctx)` — un únic punt d'entrada.
- **Flux (9 passos)**: (1) bonificadors passius pre-tirada des de les flags; (2) finestra QTE, diferent si el jugador ataca o esquiva; (3) tirada de la pool amb `N = base_dice + extra_dice`, on `base_dice` ve de `override_base_dice` o de `caster->stat(skill->casting_stat)`; (4) `effective_successes = successes + post_roll_bonus` amb terra a 0; (5) resolució Ed. 3 — si hi ha defensor (per override o per `target`) es tria el stat de defensa (el `save_attribute` de la carta per a màgia; el MAJOR entre DES i CON per a físic no jugador), es tira la seva pool i es crida `opposed_degree`; si no n'hi ha (curacions, buffs) el grau surt de la tirada pròpia amb la regla especial "0 èxits sense pifia → PARTIAL"; (6) ascensos BOTCH→PARTIAL i PARTIAL→SUCCESS; (7) confirmació de crític amb 1d6 extra si el timing va ser PERFECT i el grau just SUCCESS; (8) marca del crític de tots-sisos; (9) `final_magnitude = skill->magnitude(grau)`, ×1.5 si tots-sisos.
- **Observacions**: `eff_pool` es construeix amb els èxits efectius però **hereta `botch` i `critical` de la tirada crua** (`:96-97`), que és el comportament correcte però subtil: un `post_roll_success_bonus` positiu no pot salvar mai d'una pifia. `skill->apply_condition_id`, `apply_condition_min_degree` i `condition_duration_rounds` existeixen a la carta i l'executor **no els toca**: aplicar la condició queda íntegrament a mans del caller, sense cap senyal a `Out` que digui que caldria fer-ho. `magnitude_type` (dany vs. curació) tampoc es propaga a `Out`. Els `#include <array>` i `<cstdio>` (`:1-2`) no es fan servir.

---

### `enum class EquipSlot` i `ActiveCondition` — *CharacterSheet.h:33-46* (14 línies)
- **Què són**: `EquipSlot {Head, Torso, Legs, Feet, MainHand, OffHand, Accessory, SLOT_COUNT}` i l'estat d'una condició activa (`conditionId`, `sourceId` per a logs, `roundsRemaining`, `stacks`).
- **Observacions**: conviuen `EQUIP_SLOT_COUNT = 6` (constant declarada a `:38`, sense cap ús real), `SLOT_COUNT = 7` dins l'enum i un `std::array<std::string,7> equippedIds`. Tres nombres per al mateix concepte.

### `CharacterSheet` — *CharacterSheet.h:48-181* (134 línies; implementació a *CharacterSheet.cpp*, 157 línies)
- **Què és**: la font ÚNICA de veritat del personatge (invariant "1 concepte = 1 representació"). `Player.h`/`Enemy.h` no emmagatzemen stats, vida, tier ni equip: hi delegen amb un punter no propietari.
- **Herència**: cap. És l'eix central: `BattleState`, `SkillExecutor`, `InventoryEngine` i `ConditionEngine` la prenen per referència.
- **Estat**: identitat (`id`, `displayName`, `classId`, `secondClassId` per a multiclasse, `raceId`, `backgroundId`, `deityId`, `tier`); economia i social (`gold`, `primaryFactionId`, `factionRep`, `morality`); **sis blocs de stats separats** (`baseStats`, `racialBonuses`, `classBonuses`, `equipBonuses`, `featBonuses`, `conditionModifiers`) que `stat()` suma i clampa a [0,8] — es guarden per separat precisament perquè canviar de raça hagi de poder restar el bonus vell; bonificadors de defensa (`equipArmorBonus`, `equipShieldBonus`, `physicalSaveBonus`, `mentalSaveBonus`, `spellSaveExtra`); `inspirationPoints`; equip (`equippedIds` + tres inventaris: equipament, consumibles, materials); coneixement (`knownSkillIds`, `knownSpellIds`, `passiveIds`, `traitIds`, `featIds`); `conditions`; i la memòria cau `mutable cachedHealthCap` / `cachedDefenses`.
- **API**:
  - Stats: `stat(Stat)` i els àlies `con()`, `des()`, `int_()`, `car()`; `initiativeBonus()`.
  - Derivats: `defenses(cls)` (CA = 10+DES+armadura+escut; resistència física = 10+CON; voluntat = 10+CAR; DC de conjurs = 10+INT), `healthCap(rules, cls)`, `recompute_derived(rules, cls)` (const, escriu sobre els camps `mutable`), i els accessors sense arguments `healthCap()` / `defenseValues()` per a `ICombatant`.
  - Progressió: `handLimit(rules)`, `tierUp(rules, milestoneCount)`.
  - Condicions: `applyCondition`, `removeCondition`, `hasCondition`, `conditionStacks`, `tickEndOfRound`.
  - Facció: `getRep`, `adjustRep` (clampa a ±100), `repPriceModPct`.
  - Cap mètode `virtual`; cap `static`.
- **Flux**: la construeix `CharacterCreation::construir`; l'escriuen `InventoryEngine` (blocs d'equip) i `ConditionEngine` (`conditionModifiers`); la llegeixen `SkillExecutor` (`stat(casting_stat)`) i tot el HUD. `recompute_derived` l'ha de cridar el propietari cada cop que canviï qualsevol entrada.
- **Observacions**: `defenses()` ignora completament el paràmetre `cls` (`CharacterSheet.cpp:38`) tot i que el comentari canònic parla de "peso_armadura_permitido" de la classe. `healthCap` aproxima el bonus d'equip com `equipArmorBonus / 2` amb un "aproximacion" explícit. `tierUp` conté una línia morta (`int needed = 1 + tier + 1;` immediatament sobreescrita per `needed = tier + 1;`, `:68-69`) i un comentari que dubta de la pròpia regla del GDD. `initiativeBonus()` recorre les condicions per no fer-hi res (`(void)c`). `repPriceModPct` retorna `-0.75*rep` mentre el comentari immediatament anterior promet −50% a rep +100 (dóna −75%). I `applyCondition`/`tickEndOfRound` dupliquen la feina de `ConditionEngine`: dos escriptors per al mateix concepte.

---

### `CreationChoice`, `OpcionNarrativa`, `OpcionesTrasfondo` — *CharacterCreation.h:52-110* (agrupats, ~35 línies)
- **Què són**: `CreationChoice` és el que tria el jugador (nom, raça, classe, trasfons, deïtat opcional, tier, `baseStats` abans de la raça, i les set eleccions narratives: vincle, por, defecte, meta, ideal, personalitat, virtut). `OpcionNarrativa` és `{id, name}` — porta el nom canònic perquè la pantalla pinti "La balança de la família" i no l'id intern. `OpcionesTrasfondo` agrupa les set llistes i té `vacio()`.
- **Herència**: cap.
- **Observacions**: `OpcionesTrasfondo::vacio()` només mira `bonds`, `fears` i `flaws`; un trasfons amb aquestes tres plenes i les altres quatre buides passa per "no buit" i llavors `revisaEleccion` exigeix una meta/ideal/personalitat/virtut que no existeix a cap llista (però amb `opciones.empty()` la validació d'inclusió se salta i només queda l'exigència de no-buit → el jugador no pot avançar).

### `CharacterCreation` — *CharacterCreation.h:66-142* (77 línies; implementació a *CharacterCreation.cpp*, 291 línies)
- **Què és**: de "vull ser un nan forjador" a una fitxa jugable, amb les fórmules del GDD i sense inventar-ne cap. Explícitament NO calcula vida ni defenses: la primera versió del fitxer les reescrivia i això era una segona còpia de la fórmula.
- **Herència**: cap.
- **Estat**: quatre catàlegs propis (`m_races`, `m_classes`, `m_backgrounds`, `m_deities`) i `m_opciones` (vector de parells id-de-trasfons → set llistes).
- **API**:
  - Constants de regla: `static constexpr kStatMin = 1`, `kStatMaxTier01 = 6`, `kPuntosRepartibles = 12` — declarades aquí amb el seu perquè, perquè el GDD fixa el rang però no la bossa de punts.
  - Càrrega: `static Result<CharacterCreation> load(catalogsRoot)`.
  - Consulta: `races()`, `classes()`, `backgrounds()`, `deities()`, `opcionesDe(backgroundId)`, `static puntosGastados(base)`.
  - Validació i construcció: `problemas(e)` (retorna TOTS els problemes, no només el primer: una pantalla que corregeix d'un en un és una pantalla que s'abandona), `statsFinales(e)`, `construir(e)` (falla si `problemas` no és buit), `recalcular(ficha, reglas)`.
- **Flux**: la construeix qui obre la creació (`demo_creacion_personaje`, `CharacterCreationScreen`); produeix un `Result<CharacterSheet>` amb identitat, stats repartits en `baseStats` + `racialBonuses`, `knownSkillIds` i equip inicial de classe i trasfons (sense duplicar) i `gold` del trasfons.
- **Observacions**: **les set eleccions narratives es validen exhaustivament i després es llencen**: `construir()` (`:241-281`) no copia `bondId`, `fearId`, `flawId`, `goalId`, `idealId`, `personalityId` ni `virtueId` enlloc, i `CharacterSheet` no té camps on posar-les. Tampoc s'apliquen passives ni trets de la deïtat, la raça o el trasfons (`passiveIds`/`traitIds` queden buits) ni `classBonuses`. `ficha.id = "pj_" + e.classId` (`:242`) fa que dos personatges de la mateixa classe comparteixin identificador. `backgrounds.json` es llegeix i es parseja dues vegades (`:94` pel catàleg i `:102-106` en cru per a les set llistes). `leeFichero` retorna `ok=true` encara que el `fread` falli.

### `CharacterCreationScreen` (+ `Paso`, `Accion`) — *CharacterCreationScreen.h:30-125* (96 línies; implementació a *CharacterCreationScreen.cpp*, 355 línies)
- **Què és**: el flux de crear un personatge pas a pas SENSE obrir cap finestra — mateix criteri que `Editor::ProjectHub`: el que viu dins d'un fitxer GL només es pot passar pel compilador, així que l'estat sencer es queda aquí i es prova de debò.
- **Herència**: cap; conté els enums `Paso` (14 passos: Raza, Clase, Trasfondo, Reparto, les set narratives, Deidad, Nombre, Resumen, + COUNT) i `Accion {Ninguna, Terminado, Cancelado}`.
- **Estat**: `m_reglas` (referència const a `CharacterCreation`), `m_eleccion`, `m_paso`, `m_opciones` (ids) i `m_etiquetas` (el que es pinta) en paral·lel, `m_marcado`, `m_totalSinFiltrar`, `m_filtro`, `m_aviso`, `m_statMarcado`.
- **API**: consulta de pas (`paso`, `tituloPaso`, `numeroDePaso`, `static totalDePasos`); llista (`opciones`, `etiquetas`, `marcado`, `totalSinFiltrar`); filtre (`escribirEnFiltro`, `borrarDelFiltro`, `limpiarFiltro`) — necessari perquè hi ha 43 races i 61 classes i passar-les d'una en una amb les fletxes és la mateixa paret que la paleta de l'editor amb 5.439 objectes; repartiment (`statMarcado`, `puntosLibres`, `subirStat`, `bajarStat`); nom (`escribirEnNombre`, `borrarDelNombre`); navegació (`arriba`, `abajo`, `siguiente`, `anterior`); una sola porta d'entrada de teclat (`tecla(char)`); i sortida (`aviso`, `eleccion`, `construir`, `problemas`).
- **Flux**: el construeix qui obre la pantalla passant-li les regles ja carregades; `rehacerLista()` recalcula la llista del pas actual, l'ORDENA per nom (el `Catalog` guarda en `unordered_map` i una llista de 61 classes que es baralla sola cada execució és impossible d'usar i de provar) i posa "(ninguna)" primer al pas de deïtat; `guardarMarcado()` només s'executa dins de `siguiente()`, i canviar de trasfons neteja les set eleccions narratives perquè els seus ids són de les llistes del trasfons vell.
- **Observacions**: `anterior()` posa `m_marcado = 0` sense recuperar el que ja s'havia triat, i `siguiente()` torna a cridar `guardarMarcado()`: tornar enrere i endavant **sobreescriu silenciosament la raça/classe/trasfons ja escollits pel primer element alfabètic**. Al pas Nombre, el `switch` de `tecla()` intercepta `'w'` i `'s'` abans d'arribar a la branca d'escriptura, així que **cap nom pot contenir una 'w' o una 's' minúscules**. El `tier` és fix a 1 al constructor i no hi ha cap pas per triar-lo, tot i que `CreationChoice` i `problemas()` en suporten 0–5. Al Resumen, prémer Enter amb problemes retorna `Ninguna` sense omplir `m_aviso`: el rebuig és mut.

---

### `InventoryEngine` (+ `RecalcMode`) — *InventoryEngine.h:24-106* (83 línies; implementació a *InventoryEngine.cpp*, 321 línies)
- **Què és**: equipar i desequipar sobre `CharacterSheet.equippedIds[]` fent complir els límits de raresa per tier, pes per classe i símbols de compatibilitat, i mantenint sincronitzats els bonificadors d'equip.
- **Herència**: cap. Disseny stateless declarat: tots els mètodes són `static` i prenen referències.
- **Estat**: cap.
- **API**:
  - Operacions: `equip(sheet, eq, clazz, tierRules, cat)`, `unequip(sheet, slot, cat, out_unequipped_id)`.
  - Inventari: `add_to_inventory`, `remove_from_inventory` (esborra l'entrada en arribar a 0).
  - Validadors públics per a la UI: `check_rarity`, `check_weight`, `check_symbols`, `can_equip`.
  - Recàlcul: `recalc_equipment_bonuses(sheet, cat, mode)` amb `RecalcMode {APPLY_EQUIP, UNAPPLY_EQUIP, REBUILD}`.
  - Traducció: `resolve_slot(nom, out_two_handed)`, `slot_name(slot)`.
  - Privats: `weight_rank`, `is_weapon_slot`, `apply_bonus`.
- **Flux**: `equip` valida amb `can_equip`, resol el slot, buida el que hi hagi (i les dues mans si l'arma és a dues mans), escriu l'id (duplicat a MainHand i OffHand si és a dues mans), el treu de l'inventari i suma els bonificadors incrementalment; `unequip` fa el camí invers i el torna a l'inventari.
- **Observacions**: `recalc_equipment_bonuses(REBUILD)` itera `equippedIds` i, com que una arma a dues mans hi apareix DOS cops, **li compta els bonificadors dues vegades**; a més posa a zero `physicalSaveBonus`, `mentalSaveBonus` i `spellSaveExtra`, que són camps compartits amb altres fonts (condicions, dots, trets) i queden esborrats. `can_equip` no comprova cap col·lisió de dues mans tot i que la seva documentació (`InventoryEngine.h:81`) promet "symbols → weight → rarity → twoHanded collisions". `apply_bonus` decideix si el `caBonus` és d'escut buscant "hield"/"escudo"/"OffHand" dins del nom del slot, de manera que **qualsevol cosa a la mà secundària compta com a escut**. `maxHealthBonus` de l'equipament està explícitament ignorat (`(void)eq.maxHealthBonus`, `:147`) amb un comentari que diu que es fa a `recalc_equipment_bonuses`, on tampoc s'hi fa. `resolve_slot` retorna MainHand per defecte per a qualsevol string desconegut, inclòs "Consumable", i té una condició duplicada (`s.find("torso") || s.find("torso")`, `:37`). `check_rarity` compara l'ordinal de l'enum `ItemRarity` amb el rang derivat d'un string via `rarityRank`: funcionen només mentre els dos ordres no divergeixin.

---

### `ConditionEngine` (+ `TickEvent`) — *ConditionEngine.h:38-104* (67 línies; implementació a *ConditionEngine.cpp*, 227 línies)
- **Què és**: condicions apilables amb tick per ronda: aplicar, treure, fer córrer el rellotge i recalcular els modificadors que en surten.
- **Herència**: cap; totes les operacions són `static` i sense estat. `TickEvent` és un struct imbricat.
- **Estat**: cap. `TickEvent` porta `conditionId`, `damageInflicted` (>0 dany, <0 curació), `expired`, `stacksRemaining`.
- **API**:
  - `apply(sheet, cond, cat, sourceId)` — respecta les tres polítiques de `ConditionDefinition::Stacking`: NONE reemplaça, DURATION només refresca durada, STACK_INTENSITY suma un stack respectant `maxStacks` i refresca durada.
  - `remove(sheet, conditionId, cat, stacks_to_remove)` (0 = totes).
  - `purge_expired(sheet)`, `tick_conditions(sheet, cat)`.
  - `recalc_condition_modifiers(sheet, cat)` — públic perquè la UI el necessita després de carregar una partida.
  - Consultes: `active_stacks`, `has_flag(pred)`, `has_prevent_action`, `has_half_speed`, `grants_advantage_to_enemies`, `grants_disadvantage_to_caster`.
  - Privats: `find_active`, `find_active_c`.
- **Flux**: el criden BattleState/GameSession al final de cada ronda; escriu directament sobre `sheet.conditions` i `sheet.conditionModifiers`, i retorna el `vector<TickEvent>` perquè el caller apliqui el dany i animi el que calgui.
- **Observacions**: la capçalera documenta un sistema de triggers `onApply` / `onTick` / `onRemove` "que estan a `ConditionDefinition`" — **`ConditionDefinition` (RpgCoreDefinitions.h:208-236) no té cap d'aquests hooks** i el .cpp no en crida cap: és documentació d'un sistema que no existeix. `recalc_condition_modifiers` només aplica `statMod`; `caMod`, `physicalSaveMod` i `willSaveMod` estan declarats a la definició i mai s'apliquen (el propi codi ho reconeix a `:220-223`). `breaksOnDamage` es comprova contra el dany que la PRÒPIA condició infligeix (`:183-186`), no contra el dany rebut de fora, que és la semàntica que "encantat" necessita. `purge_expired` és funcionalment mort (detall a sota).

---

### Buits i riscos del mòdul

- **Lectura fora de rang als avisos de temporitzador**: `NarrativeEngine.cpp:549-551` itera `i < t.avisos.size()` i indexa `ts.avisosDados[i]` sense comprovar la mida del vector. `startTimer` (`:506`) el dimensiona correctament, però `applyNarrativeState` (`SaveGame.cpp:246-252`) el copia tal com estava al guardat: una partida desada abans d'afegir un avís a l'aventura, o un JSON de guardat manipulat, provoca escriptura fora de rang. Cal `assign`/`resize` defensiu a `tickTimers`.
- **`TimerDefinition::presentacion` és contingut mort**: es parseja i es valida a `NarrativeEngine.cpp:363-374` i cap consumidor del repositori la llegeix; un temporitzador marcat `secreto` es pinta igual que un de `contador`. La regla de la bitàcola 2.18 (contador / barra / secret) no està implementada.
- **`tickTimers` materialitza tots els temporitzadors encara que no s'hagin arrencat mai**: el bucle de `NarrativeEngine.cpp:526-527` crida `state.timerRef(t.id)` per a cada `TimerDefinition`, i `timerRef` (`:100-107`) crea l'entrada si no existeix. Cada partida acaba amb un `TimerState` inert per timer declarat, tots serialitzats a cada guardat.
- **Expiració per sortir de zona sense haver-hi entrat mai**: `NarrativeEngine.cpp:534` només comprova `nivelActual != t.zona`. Un timer que arrenqui per flag mentre el jugador ja és fora de la zona expira al primer tick, sense que hagi tingut mai l'oportunitat de complir-lo.
- **El missatge d'error de tipus d'efecte desconegut és incomplet**: `NarrativeEngine.cpp:241` llista "setFlag/clearFlag/grantGold/log/skillCheck/startBattle" i omet `startTimer`, que sí que és vàlid des de `:72`. L'autor de contingut que s'equivoqui rebrà una llista que li nega una opció existent.
- **Ids de beat duplicats no es detecten**: `AdventureScript::loadFromString` valida que `beat.id` no sigui buit (`NarrativeEngine.cpp:202-206`) però no la unicitat, tot i que `once` i `beatsFired` depenen exclusivament d'aquest id.
- **`TriggerType` no cobreix el contingut ja escrit**: l'enum té tres valors (`NarrativeEngine.h:146`) i el parseig és estrictament tancat (`NarrativeEngine.cpp:31-45`); les cartes de `assets/catalogs/events.json` amb `trigger.type: temporal`, que el mateix comentari `:143-145` reconeix, farien fallar la càrrega sencera de l'aventura per la política tot-o-res.
- **Els efectes diferits desapareixen si no es passa per `GameSession`**: `grantGold`, `skillCheck` i `startBattle` només tenen efecte via `applyNarrative` (`GameSession.cpp:371-388`). Qualsevol crida directa a `fire()`/`tick()` — el patró de tots els demos de `examples/` — els descarta en silenci, sense cap avís.
- **`grantGold` negatiu es pot cobrar a mitges**: `GameSession::addGold` clampa a 0 (`GameSession.cpp:213`), de manera que un peatge més car que el saldo es cobra parcialment i el beat no té cap manera de saber si s'ha pogut pagar.
- **Doble comptatge de bonificadors en armes a dues mans**: `InventoryEngine.cpp:159-164` (`RecalcMode::REBUILD`) recorre `equippedIds`, on una arma a dues mans hi apareix a MainHand i a OffHand, i li crida `apply_bonus` dues vegades.
- **`REBUILD` esborra bonificadors que no són d'equip**: `InventoryEngine.cpp:155-158` posa a zero `physicalSaveBonus`, `mentalSaveBonus` i `spellSaveExtra`, camps compartits amb qualsevol altra font del `CharacterSheet`.
- **`can_equip` no fa el que documenta**: `InventoryEngine.h:81` promet incloure les col·lisions de dues mans; la implementació (`InventoryEngine.cpp:102-127`) només fa símbols, pes i raresa, i ignora el `sheet` explícitament (`:106`).
- **Qualsevol objecte a OffHand compta com a escut**: `InventoryEngine.cpp:137-141` detecta l'escut per substring del nom del slot, de manera que una torxa o una daga secundària sumen al `equipShieldBonus` en comptes de a l'armadura.
- **`maxHealthBonus` de l'equipament no s'aplica enlloc**: `InventoryEngine.cpp:147` el descarta amb `(void)` i el comentari `:145-146` remet a `recalc_equipment_bonuses`, que tampoc el toca. Un "anell de vitalitat" no dóna vida.
- **`resolve_slot` té un fallback perillós**: `InventoryEngine.cpp:53-55` retorna MainHand per a qualsevol slot desconegut, inclòs "Consumable", i `:37` repeteix la mateixa condició dues vegades (`s.find("torso") || s.find("torso")`).
- **`purge_expired` no purga res**: `ConditionEngine.cpp:149-159` exigeix `roundsRemaining <= 0 && stacks <= 0`, i `stacks` mai baixa de 1 perquè `remove`/`tick_conditions` esborren l'entrada sencera. A més conté un bucle de comptatge mort (`:154`) el resultat del qual es descarta immediatament a `:156`.
- **Els triggers de condició documentats no existeixen**: `ConditionEngine.h:28-36` descriu `onApply`, `onTick` i `onRemove` "a `ConditionDefinition`"; aquesta struct (`RpgCoreDefinitions.h:208-236`) no en té cap.
- **`caMod`, `physicalSaveMod` i `willSaveMod` de les condicions són camps morts**: `ConditionEngine.cpp:210-224` només suma `statMod`; els tres modificadors de defensa es parsegen i mai s'apliquen.
- **`breaksOnDamage` té la semàntica invertida**: `ConditionEngine.cpp:183-186` trenca la condició pel dany que ella mateixa infligeix, no pel dany que rep el portador — que és el cas d'ús real ("encantat" es trenca quan et peguen).
- **Dos escriptors per a les condicions**: `CharacterSheet::applyCondition`/`removeCondition`/`tickEndOfRound` (`CharacterSheet.cpp:88-129`) fan la mateixa feina que `ConditionEngine` amb regles diferents (no coneixen `Stacking` ni `maxStacks` ni `recalc_condition_modifiers`). Segons la invariant de fractura #1 del propi mòdul, una de les dues sobra.
- **`ContextBonusFlag::TARGET_CA_FLANKED` i `ExecutionContext::override_target_defense` són declarats i ignorats**: `EnhanceHooks.h:26` i `:97`; cap línia de `SkillExecutor.cpp` els llegeix.
- **`force_dice_mod` és una única casella amb "l'últim guanya"**: `SkillExecutor.cpp:25`, `:34`, `:38` i `:67` s'hi escriuen en seqüència; sorpresa + fatiga dóna desavantatge net i un dodge PERFECT esborra qualsevol desavantatge previ. No hi ha regla de composició declarada.
- **`SkillExecutor` no aplica les condicions de la carta**: `SkillDefinition::apply_condition_id`, `apply_condition_min_degree` i `condition_duration_rounds` (`SkillDefinition.h:106-108`) no es toquen a `execute()` ni apareixen a `Out`, de manera que el caller no té cap senyal que hi hagi una condició pendent d'aplicar.
- **La meitat de `DicePoolEngine` és API en desús encara viva**: `resolve_against_cd`, `resolve_against_defense`, `defense_to_cd`, `resolve_opposed`, `CheckOutcome` i `OpposedOutcome` (`DicePoolEngine.h:125-189`) estan marcats "EN DESUSO desde Ed. 3" i segueixen exportats; `DefenseBlock` i `CharacterSheet::defenses()` alimenten exclusivament aquesta via morta.
- **`src/RPG/DicePoolEngine.cpp` no es compila**: no apareix a la llista de `motor_rpg` (`CMakeLists.txt:94-103`); és un fitxer de sis línies que existeix només com a nota.
- **Les set eleccions narratives de creació es validen i es perden**: `CharacterCreation.cpp:210-221` les comprova una per una i `construir()` (`:241-281`) no en copia cap; `CharacterSheet` no té camps per a vincle, por, defecte, meta, ideal, personalitat ni virtut. Tot el pas narratiu de la creació és avui decoratiu.
- **`ficha.id = "pj_" + classId`**: `CharacterCreation.cpp:242` fa que dos personatges de la mateixa classe comparteixin identificador, cosa que el contracte d'ids del projecte assumeix únic.
- **La creació no aplica passives, trets ni bonificadors de classe**: `passiveIds`, `traitIds`, `featIds` i `classBonuses` queden buits tot i que raça, deïtat i trasfons els declaren als catàlegs.
- **`anterior()` + `siguiente()` sobreescriuen l'elecció anterior**: `CharacterCreationScreen.cpp:295-297` reinicia `m_marcado = 0` sense restaurar el que estava triat, i `siguiente()` (`:277`) torna a desar el marcat: tornar enrere per consultar i avançar canvia la raça pel primer element alfabètic sense avisar.
- **Cap nom de personatge pot contenir 'w' ni 's'**: `CharacterCreationScreen.cpp:313-314` intercepten aquestes dues tecles per navegar abans que la branca de Nombre (`:338-345`) pugui rebre-les.
- **El `tier` no es pot triar a la pantalla**: es fixa a 1 al constructor (`CharacterCreationScreen.cpp:29`) i no hi ha cap `Paso` per canviar-lo, tot i que `problemas()` valida 0–5 i el filtre de classes per tier (`:102-105`) hi depèn.
- **Rebuig mut al resum**: `CharacterCreationScreen.cpp:302-305` retorna `Accion::Ninguna` quan hi ha problemes sense omplir `m_aviso`; el jugador prem Enter i no passa res ni se li diu per què.
- **`OpcionesTrasfondo::vacio()` només mira tres de les set llistes** (`CharacterCreation.h:109`): un trasfons amb bonds/fears/flaws però sense goals/ideals/personalities/virtues bloqueja la creació amb un "falta elegir meta" impossible de satisfer.
- **`CharacterSheet::tierUp` porta una línia morta i un dubte sense resoldre**: `CharacterSheet.cpp:68-69` calcula `needed` i el sobreescriu a la línia següent, amb un comentari que pregunta quina és la regla del GDD ("T1→T2 = 3?").
- **`repPriceModPct` contradiu el seu propi comentari**: `CharacterSheet.cpp:149-155` promet −50% a reputació +100 i la fórmula (`-0.75f * rep`) en dóna −75%.
- **`CharacterSheet::defenses()` ignora la classe**: `CharacterSheet.cpp:38` descarta el paràmetre `cls` tot i que la fórmula documentada (`:33`) inclou el pes d'armadura permès per classe; i `healthCap` aproxima el bonus d'equip com `equipArmorBonus / 2` (`:56`).
- **Tres nombres per als slots d'equip**: `EQUIP_SLOT_COUNT = 6` (`CharacterSheet.h:38`, sense cap ús), `EquipSlot::SLOT_COUNT = 7` i `std::array<std::string,7> equippedIds` (`:87`).
- **`updateNarrativeTimers` detecta canvis comptant flags**: `GameSession.cpp:407-410` compara `flagCount()` abans i després; si un avís torna a encendre una flag que ja hi era, o si un tick encén una flag i n'apaga una altra, el canvi no es detecta i la conseqüència no aflora com a diàleg.