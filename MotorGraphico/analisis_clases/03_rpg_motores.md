# 03 — Módulo `RPG` · motores

*Análisis clase a clase · 31/08/2026 (continúa `02_rpg_modelo_de_datos.md`; sesión qwen 11:53 retomada por zcode 12:15) · 10 headers / 10 cpp (1.576 + 2.405 líneas)*

**Trabajo en vuelo detectado** (`git status`): `NarrativeEngine.h/.cpp` modificados sin commitear por otra sesión — añade `NarrativeState::beatsFired()` (+6/+6) para que `src/Game/SaveGame.cpp` persista los beats disparados (RUN-10). Documentado, no tocado.

---

### `DicePoolEngine` (`include/RPG/DicePoolEngine.h`) — clase estática (header-only)
**Qué es:** el único resolvedor de tiradas del juego (Nd6): 6=1 éxito, 5=½, crítico=todos 6, pifia Ed.3=sin 5/6 y ≥2 unos. GL-free, sin estado.
**Estado:** ninguno; tipos de apoyo `PoolResult` (éxitos/rolls/critical/botch), `CheckOutcome` (grado vs CD), `OpposedOutcome` (enfrentada con `tie` aparte), enums `DiceMod` y `Degree` (BOTCH/PARTIAL/SUCCESS/CRITICAL).
**API:** `es_pifia(rolls)`; `roll_pool(dice, mod, rng)` (clampea 0..24 dados); `opposed_degree(atacante, defensor)` — grado del lado activo, pifia del defensor→CRITICAL; `resolve_against_cd` / `resolve_against_defense` / `defense_to_cd` / `resolve_opposed` — **EN DESUSO desde Ed.3**; `CD_MAX=3.0`.
**Flujo:** lo llaman `SkillExecutor` (pasos 3 y 5) y `GameSession::applyNarrative` para los `skillCheck` narrativos.
**Trampas/deuda:** la ruta CD/`DefenseBlock` se conserva "hasta migrar SkillExecutor/BattleStateOnegai y el skillCheck narrativo" — su retirada es paso propio de ARCHITECTURE.md. El botch-se-comprueba-antes-que-la-CD es a propósito (incidente §3.7, fijado por `demo_dice_engine`).

### `DicePoolEngine.cpp` — TU vacío
6 líneas, SUPERSEDED (todo inline en el header). Deuda: borrarlo o des-inlinear.

### `CharacterSheet` (`include/RPG/CharacterSheet.h` + `.cpp`) — clase concreta (fuente única del personaje)
**Qué es:** la ficha autoritativa (Fractura #1): identidad, stats, equipo, condiciones, facciones. Player/EnemyBrain no almacenan nada: delegan en un puntero no propietario (`m_sheet`; `nullptr` → fallback legacy = dual-mode).
**Estado:** identidad (ids clase/raza/trasfondo/deidad, tier, gold, facciones, `morality`); 6 arrays `[CON,DES,INT,CAR]` de bonos (base/racial/class/equip/feat/condition); defensas extra; `equippedIds[7]`; 3 vectores de inventario (id+stack); skills/spells/pasives/traits/feats; `conditions` (`ActiveCondition`); cachés `mutable` (`cachedHealthCap`, `cachedDefenses`).
**API:** `stat(s)` suma los 6 arrays y clampa + azúcares con/des/int/car; `initiativeBonus()` (solo DES hoy); `defenses(cls)` (CA=10+DES+eq, mental=10+CAR, resF=10+CON, spell=10+INT — el parámetro `cls` se ignora); `healthCap(rules, cls)`; `recompute_derived()` + helpers sin args (para ICombatant); `handLimit`; `tierUp(rules, milestones)`; `applyCondition/removeCondition/hasCondition/conditionStacks`; `tickEndOfRound()`; `getRep/adjustRep/repPriceModPct`. El .cpp añade `ClassDefinition::primary/secondary_stat_enum()` (default INT si el string no encaja).
**Flujo:** la construyen `CharacterCreation::construir` y los loaders de enemigos; la leen BattleState/SkillExecutor/Inventory/ConditionEngine; GameSession llama `recompute_derived` al cambiar cualquier entrada.
**Trampas/deuda:** `tierUp` tiene línea muerta (`needed = 1+tier+1;` sobrescrito); `initiativeBonus` itera condiciones con `(void)c` (placeholder P1); `healthCap` aproxima equipo con `equipArmorBonus/2`; **doble vía de condiciones**: `applyCondition/tickEndOfRound` duplican lo que hace `ConditionEngine` (la de la ficha no aplica daño ni recalcula modifiers).

### `RandomEngine` / `Xoroshiro128p` / adaptadores (`include/RPG/RandomEngine.h`) — interfaz + 3 clases
**Qué es:** contrato de azar inyectable (`roll_d6`, `roll_in_range`, `seed`) con puente B6 hacia el legacy `::IRng`.
**API:** `RngAsRandomEngine` (IRng→RandomEngine; `seed()` siempre 0); `RandomEngineAsIRng` (inversa); `Xoroshiro128p` header-only, semilla fija → batallas reproducibles.
**Flujo:** GameSession posee su `Xoroshiro128p`; se inyecta a SkillExecutor vía `ExecutionContext.rng`.
**Trampas/deuda:** `roll_d6` de Xoroshiro usa truncamiento de double (sesgo leve); el adaptador pierde la semilla.

### `IRng` / `SeededRng` (`include/RPG/Rng.h`) — interfaz + clase legacy
**Qué es:** la interfaz de azar PREVIA (un solo `nextInt(a,b)`), conservada por compatibilidad.
**Trampas/deuda:** `SeededRng::nextInt` declarado **sin definición** y sin usuarios — código muerto; retirar cuando B6 termine.

### `InventoryEngine` (`include/RPG/InventoryEngine.h` + `.cpp`) — clase estática stateless
**Qué es:** equip/desequip con validación (raridad ≤ tier, peso ≤ clase, símbolos compatibles) y recálculo de bonos sobre la ficha.
**API:** `equip(sheet, eq, clazz, rules, cat)` — gestiona TwoHanded (ocupa MainHand+OffHand) y auto-desequipa; `unequip`; `add/remove_from_inventory` (stacks); `check_rarity/check_weight/check_symbols/can_equip`; `recalc_equipment_bonuses` (solo REBUILD); `resolve_slot/slot_name` (ES/EN, default MainHand).
**Flujo:** llamado desde GameSession/UI; muta `sheet.equippedIds` y bonos.
**Trampas/deuda:** `apply_bonus` clasifica shield-vs-armor por substrings del slot; `maxHealthBonus` ignorado (`(void)`); en `equip` un if/else tiene **ambas ramas idénticas**; `resolve_slot` matchea `"pie"` antes que `"mainhand"` (orden frágil).

### `ConditionEngine` (`include/RPG/ConditionEngine.h` + `.cpp`) — clase estática stateless
**Qué es:** condiciones stackeables con tick por ronda; struct de salida `TickEvent` (conditionId, daño, expired, stacks).
**API:** `apply(sheet, cond, cat, sourceId)` con 3 modos (NONE=reset, DURATION=refresca, STACK_INTENSITY=+1 hasta maxStacks); `remove`; `purge_expired`; `tick_conditions` (−1 ronda, daño `damagePerRound*stacks`, `breaksOnDamage`, borrado por índices inversos); `recalc_condition_modifiers` (statMod×stacks); `has_prevent_action/has_half_speed/grants_advantage_to_enemies/grants_disadvantage_to_caster`.
**Flujo:** lo llaman BattleState/GameSession tras las skills; los flags los lee el montador del `ExecutionContext`.
**Trampas/deuda:** `purge_expired` con bucle muerto + recomputo torpe; save-bonus de condición para P1; coexiste con la vía duplicada de CharacterSheet.

### `SkillExecutor` + `ExecutionContext` + `EnhanceHooks` (`include/RPG/EnhanceHooks.h`, `src/RPG/SkillExecutor.cpp`) — clase estática + PODs + interfaz
**Qué es:** el orquestador FINAL de una skill: enhancements pasivos pre-tirada → QTE opcional → pool Nd6 INVIOLABLE → enfrentada Ed.3 → upgrades → confirm de crítico → magnitud.
**Estado:** `ContextBonusFlag` (bitmask: FLANKED, HIGH_GROUND, SURPRISED, INSPIRED, FATIGUED, EQUIP_INCOMPATIBLE, ENEMY_BLEEDING, CASTER_LOW_HP, TARGET_CA_FLANKED — este último **sin efecto aún**); `IQteProvider`; `ExecutionContext` (caster/target/skill, flags, qte, rng obligatorio, overrides `override_base_dice`/`override_target_dice`/`defender_extra_dice`/`defender_dice_mod`, y `override_target_defense` **EN DESUSO Ed.3**); `SkillExecutor::Out` (pool, defender_pool, opposed, final_degree, effective_successes, final_magnitude, applied_enhancements, was_all_sixes_critical).
**API:** `execute(ctx)` — 9 pasos documentados en el propio cpp.
**Flujo:** lo llaman `src/Game/Skill.cpp` y BattleState (con overrides desde ICombatant).
**Trampas/deuda:** ventanas QTE hardcodeadas (1200/1000 ms); defensa física = `save_attribute` de la carta o `max(DES,CON)`; `effective_successes` puede superar los éxitos reales del pool.

### `NarrativeEngine` + `NarrativeState` + modelos (`include/RPG/NarrativeEngine.h` + `.cpp`) — clase + PODs
**Qué es:** la línea de eventos: flags de mundo + beats disparados por trigger (Talk/Enter/Auto) cuando sus condiciones encajan — "el primero que encaja gana". Incluye objetivos de diario y timers de misión (F1).
**Estado:** `NarrativeState` (set de flags, set de beats disparados, vector de `TimerState`); `NarrativeEngine` solo `m_adventure` (puntero no propietario). PODs: `NarrativeCondition` (all/any/not), `BeatTrigger`, `EffectType` (SetFlag/ClearFlag/GrantGold/Log/SkillCheck/StartBattle/StartTimer), `SkillCheckRequest` (`rivalDice` Ed.3 + `cd` legacy), `BattleRequest`, `NarrativeEffect`, `NarrativeBeat`, `ObjectiveDefinition`, `TimerDefinition` (con `Aviso` y `Presentacion`), `AdventureScript`, `NarrativeResult` (efectos diferidos).
**API:** `setAdventure`; `fire(type, target, state)`; azúcares `talkTo/enterLevel/tick`; `tickTimers(state, dtActivo, nivel)` (arranque por flag, expiración por zona o tiempo, avisos por fracción); `startTimer`; `objectives`; `AdventureScript::loadFromString/File` (todo-o-nada, mensajes con índice de beat).
**Flujo:** GameSession lo enchufa; resuelve `skillChecks` con DicePoolEngine (convierte `cd` legacy→`rivalDice = cd*2+0.5`, clamp 1..5), encola `startBattle` y llama `tickTimers` con el dt ACTIVO. SaveGame persiste `flags()` y `beatsFired()`.
**Trampas/deuda:** el accessor `beatsFired()` es lo en vuelo; el comentario de `SkillCheckRequest` quedó pegado sobre `BattleRequest`; el error de `parseEffectType` lista tipos válidos **sin `startTimer`** aunque lo acepta; sin cascadas (un disparo, un beat).

### `CharacterCreation` (`include/RPG/CharacterCreation.h` + `.cpp`) — clase concreta
**Qué es:** de "quiero ser un enano forjador" a ficha jugable: valida la elección legal y la traduce a CharacterSheet SIN recalcular fórmulas.
**Estado:** 4 catálogos (races/classes/backgrounds/deities) + `m_opciones` (las 7 listas narrativas del trasfondo, re-parseadas del JSON crudo).
**API:** constantes de reparto (`kStatMin=1`, `kStatMaxTier01=6`, `kPuntosRepartibles=12` — bolsa declarada aquí, no canon GDD); `load`; accesores; `opcionesDe(backgroundId)`; `puntosGastados`; `problemas(choice)` (devuelve TODOS); `statsFinales` (base+racial, sin techo); `construir(choice)` → `Result<CharacterSheet>` (`id="pj_"+classId`); `recalcular(ficha, reglas)`.
**Trampas/deuda:** re-parsea `backgrounds.json` una segunda vez; `ficha.id` no es único (dos PJ de la misma clase colisionan); no valida `secondClassId` ni hitos.

### `CharacterCreationScreen` (`include/RPG/CharacterCreationScreen.h` + `.cpp`) — wizard GL-free
**Qué es:** flujo de creación paso a paso, 14 pasos (Raza→Clase→Trasfondo→Reparto→7 narrativas→Deidad→Nombre→Resumen) con filtro de texto.
**Estado:** ref a CharacterCreation, `CreationChoice` en construcción, paso, lista filtrada (ordenada por nombre — el unordered_map barajaría), índice, filtro, aviso, stat marcado.
**API:** consultas; filtro (`escribirEnFiltro/...`); reparto (`puntosLibres/subirStat/bajarStat`); nombre; navegación; **`tecla(k)` única puerta** (w/s, +/-, \n, ESC, \b, resto texto).
**Flujo:** la ventana GL (o test) solo pinta y pasa teclas; al `Terminado` el llamador hace `construir()`.
**Trampas/deuda:** en pasos de lista no se filtra por "w"/"s" (reservadas); `aMinusculas` solo ASCII; `siguiente()` trata Deidad con segundo `if` fuera del `else if`.

### `TierRules.cpp` — implementación
Tabla de 6 tiers con defaults hardcodeados + carga JSON (merge por tier + sort; claves camelCase y snake_case); `compute_cap_health` = `min(base+CON*3+CAR*1+bonus+eq, cap)`.
**Trampa:** **doble fuente** — `kDefaultTiers` duplica `assets/rules/tier_rules.json`.

### `SkillDefinitionEnums.cpp` — parsers enum
Roundtrip string↔enum con alias ES/EN; unknown-string cae en defaults **silenciosamente** (contraste deliberado con el parser narrativo estricto).

---

## Qué falta / deuda del módulo

1. **Retirada Ed.3 incompleta (la mayor):** `resolve_against_cd/resolve_against_defense/defense_to_cd/resolve_opposed` y `override_target_defense` siguen vivos; el propio header pide su retirada como paso propio.
2. **Condiciones con doble representación** (CharacterSheet vs ConditionEngine): dejar una sola y reducir la otra a alias.
3. **Trabajo en vuelo ajeno:** `beatsFired()` para RUN-10 — no commitear ni deshacer.
4. **Código muerto/cepillable:** `DicePoolEngine.cpp` (TU vacío), `SeededRng::nextInt` sin definir, `TARGET_CA_FLANKED` sin efecto, línea muerta en `tierUp`, `(void)c` en iniciativa.
5. **Torpezas heredadas:** shield/armor por substring; `maxHealthBonus` ignorado; ramas idénticas en `equip`; defaults de tiers duplicados; segundo parseo de `backgrounds.json`; error de `parseEffectType` sin `startTimer`.
6. **P1/P2 declarados:** condiciones que afecten iniciativa/saves, equipo con vida explícita, triggers narrativos `Pickup/Defeat/Temporal`, efectos OpenShop/GrantQuest/LevelTransition/SetMilestone, vínculo `AdventureScript`↔`AdventureDefinition` (las ~300 cartas de historia), multiclass en creación (`secondClassId` nunca se rellena).
7. **RNG:** homogeneizar `IRng`/`RandomEngine` (B6) y decidir el destino de `Rng.h`.
