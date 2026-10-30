# 02 — Módulo `RPG` · modelo de datos

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe el análisis clase a clase de agosto, foto del 31/08-03/09/2026. El juicio vigente, clase a clase y sobre el código de hoy, está en `planificacion/juicio_clases/` (12 partes, generado con `tools/mapa_clases.py`). De las 7 citas de codigo que contiene, 6 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

*Análisis clase a clase · arbre de treball 31/08/2026 · fuente: `include/RPG/*` (16 headers / 3.321 líneas)*

**Naturaleza:** datos del Motor Nd6 Onegai Ed. 2/3 (bitácora §1.2, §2.17).
Todo el módulo es GL-free. Los catálogos cargan desde `assets/catalogs/*.json`
(24 ficheros, 2.652 entradas verificadas por `validar_catalogos_rpg`).
Regla de oro del módulo: **el orden de stats es SIEMPRE `[CON, DES, INT, CAR]`**
(NO el de D&D 5e; trampa mordida 2+ veces, BITACORA Parte 3).

## Enums y POD fundacionales

### `RPG::Stat` (`Stat.h`) — enum class uint8
`CON=0, DES=1, INT=2, CAR=3`. Cuatro atributos base (GDD Llibre II §5.1); sin
Point-Buy: empiezan en 0 y solo reciben bonificadores (equipo/raza/passives/condiciones).
Constantes `STAT_RANK_MIN=0`, `STAT_RANK_MAX=8`. Helpers: `stat_name`, `stat_from_index` (módulo 4).

### `RPG::Defense` + `RPG::DefenseBlock` (`Defense.h`)
`Defense`: `ARMOR_CLASS, PHYSICAL_SAVE, WILL_SAVE, SPELL_SAVE_DC` (GDD §5.2).
`DefenseBlock` — struct POD: `std::array<int,4> values = {10,10,10,8}` +
`get/set` + helpers `ca_fisica()/save_fis()/save_vol()/dc_mag()`.
**⚠ NO tiene campos `.ca/.resFis/...`** (trampa mordida, BITACORA Parte 3).
Layout idéntico al `ICombatant::DefenseBlock` de Render: se copian sin convertir.

### `RPG::ItemRarity` (`TierRules.h`) — enum class uint8
`Common..Mythic` (6 rarezas; Mythic reservada a contenido final).
Helpers libres: `rarityRank(string)` (devuelve −1 si desconocida — **es STRING,
nunca `static_cast<int>`**, trampa mordida), `rarity_from_name`, `rarity_name`.

### `RPG::TierRule` / `RPG::TierRules` (`TierRules.h`)
- `TierRule` — struct 1:1 con una fila del array `"tiers"` de
  `assets/rules/tier_rules.json`: tier, name/description, `handLimit`,
  `maxEquipRarity` (**string** "common"…), `healthCap`, `healthBonus`,
  `allowMulticlass`, `multiclassHandLimit`, `maxSummons`, `maxFeats`, `maxDivineCards`.
- `TierRules` — clase concreta; singleton SEMÁNTICO (lo posee GameSession; los
  demás lo toman por `const&`; respeta fractura #1). **Estado:** `vector<TierRule> m_tiers`.
  **API:** `loadFromString/loadFromFile -> Result<int>` (todo-o-nada) · `find(tier)`
  (nullptr si no existe) · `at(tier)` · helpers por tier (`handLimit`, `healthCap`,
  `canEquipRarity(tier, rarity STRING)`, …) · `compute_cap_health(tier, base, CON, CAR, eqBonus)`
  = `min(base + CON*3 + CAR*1 + healthBonus, healthCap)`. Tiers 0..5 (`TIER_MIN/MAX`).

## Definiciones de catálogo (`RPG/Definitions/`)

21 structs POD (regla morfológica ARCHITECTURE §1: datos sin invariantes =
`struct`). Todos con `id` string (contrato `[a-z0-9_]+`). **⚠ `.name` en
Monster/Class; `displayName` solo existe en CharacterSheet** (trampa mordida).

### `RpgCoreDefinitions.h` (19 structs)
| Struct | Nº | Campos clave |
|---|---:|---|
| `ClassDefinition` | 61 | role, baseHealth, primary/secondaryStat (string→enum helper), maxSkillCards, compatibilidades de símbolos/tags, pesos máx., `passiveId`, `startingSkillIds`, `learnableSkillIds` (antes se tiraba; recuperado), equipo inicial, especializaciones, multiclass |
| `RaceDefinition` | 43 | baseSpeed, idiomas, deidades compatibles, `statBonuses[4]`, 3 rasgos (passive/active/racial), afinidades/limitaciones/unlocks |
| `BackgroundDefinition` | 50 | oro inicial, virtud/defecto/objetivo, equipo y skills de arranque, facción + reputación inicial |
| `PassiveDefinition` | 66 | `Trigger` enum (10 valores: ON_TURN_START, ON_CRIT_DEALT, ALWAYS_ACTIVE…), `extra_dice_on_trigger`, `success_bonus_on_trigger`, `required_tag`, statBonuses + bonos a defensas/vida/mano |
| `FeatDefinition` | 6 | requisitos clase/bg/tier, statBonuses, passives/skills otorgadas |
| `TraitDefinition` | 2 | activa/pasiva, `timesPerRest`, recovery, skill vinculada |
| `DeityDefinition` | 10 | dominios, symbolTags, plegarias (`prayerSkillIds`), passivas de dogma, `dogmaStatBonus` |
| `ConditionDefinition` | 15 | `Stacking` enum (NONE/DURATION/STACK_INTENSITY), **`defaultRounds`/`damagePerRound`** (⚠ NUNCA `durationRounds`/`dotMagnitude` — trampa), `isNegative`, statMod + mods defensas, flags de comportamiento (preventAction, halfSpeed, grantAdvantageToEnemies, grantDisadvantageToCaster, breaksOnDamage) |
| `EquipmentDefinition` | 413 | slot (9 valores string), weightCategory, `compatibleSymbols` (△○□✦☠✝♞⚙), grantedTags, statBonuses, caBonus/maxHealthBonus/saves/damageBonus, `linkedSkillId`, `twoHanded` |
| `ConsumableDefinition` | 31 | healHp, restorePileCards, clearOneCondition, grantCondition (+rondas), bonusGold, grantPassive (+rondas) |
| `MonsterDefinition` | 432 | rank (criatura/elite/jefe/inocente), role táctico, facción, `stats[4]`, maxHealth (⚠ 432 con `maxHealth==0` en JSON fuente, deuda 4.3), ca/defMental/resFis/precMag/movement, `MonsterAttack`{name,defense,effect,magnitudeHint}, passiveId, conditions/skills/loot, `MonsterPhase` (jefes; declarado, no usado) |
| `SummonDefinition` | 201 | duración, stats, attacks/passives, summonRole, concentración |
| `MountDefinition` | 50 | movement/carga, bonuses montados, canFly/canSwim |
| `TrapDefinition` | 51 | daño, saveAttribute, cd Nd6, detección, condición aplicada, oneShot/cooldown |
| `LootTableDefinition` | 201 | `Entry{refId, weight, min, max}` (refId puede ser `__gold__` u otra tabla), minGold/maxGold |
| `NpcDefinition` | 12 | race/class/facción, isMerchant+shopLootId, hasDialogue+dialogueId, isHostile+monsterId, gold |
| `AdventureDefinition` | 78 | tier recomendado, quests, NPCs, facción, story cards inicio/fin, `grantMilestoneOnEnd` |
| `EventDefinition` | 1 | triggerKind, zonas válidas, peso, `Option{label, gold, condición, quest, combate, storyCard, transición}` |
| `LocationDefinition` | 19+95+26 | `Kind` (Nation/City/Zone) en UN tipo con discriminador; **mapa FECHADO** (`epoch` 2000 b.f.); fuente: `dndWeebCC/data/mapa/geografia.json` (única geometría del mundo, BITACORA §2.1); nación = polígono; ciudad = posición + tamaño/población/gobernante/rasgo; zona = terreno/peligro; refs a story/npc/deity/race |

### `SkillDefinition.h` — enums + struct
- Enums: `ActionType` (6: principal/movement/reacción/canal/free/pasiva),
  `RecoveryPile` (5: none/active/short/long/passive), `SkillTarget` (6),
  `SaveAttribute` (5), `MagnitudeType` (5: damage/heal/move/condition/other)
  + pares `*_from_string`/`to_string` para parsear JSON.
- `SkillDefinition` — ~500 habilidades + 210 hechizos (parseados de
  `assets/catalogs/skills.json` + `spells.json`). Campos: tier_min, rarity,
  action_type/channel_rounds/recovery/target, **`casting_stat`** (N dados de la
  pool), save_attribute, `magnitude_by_degree[4]` float `[BOTCH,PARTIAL,SUCCESS,CRITICAL]`,
  aplicar condición (id + grado mínimo + duración), tags/required/incompatible,
  `granted_summon_id`. Helpers: `has_tag`, `magnitude_f/magnitude(Degree)`, `tier_min_clamped`.
- `MagnitudePreset` / `MAGNITUDE_PRESETS[]` — presets de test (Petit/Mitjà/Gran/Llegendari);
  NO son fuente de verdad (lo es skills.json).

## Sistema de catálogos (`RPG/Catalogs/RpgCatalogs.h`, header-only)

### Helpers internos (solo ese TU)
`tolower_inline`, `copy_json_int_array<N>`, `copy_json_string_vec`,
`rarity_from_json` (acepta int o string, ES/EN), `degree_from_string`.

### `CatalogLoader<T>` — struct plantilla, especialización obligatoria
`static T from_json(const JsonValue&)`. La primaria SIN código da linker error
a propósito: no hay fallback silencioso. 20 especializaciones (una por struct;
SkillDefinition sirve también para spells). Tolerantes a alias de campo
(`tierMin`/`tier_min`/`tier`).

### `Catalog<T> : ICatalog<T>` — clase plantilla (el caballo de carga)
**Estado:** `unordered_map<string, unique_ptr<T>> m_store`. No copiable, movible.
**API:** `loadFromString(json)` — espera wrapper `{"entries":[...]}`; `Result<int>`
con nº de entradas; id vacío se descarta; loader que lanza se ignora y se sigue ·
`loadFromFile(path)` (fopen/fread) · `insert(T)` para crear por programa ·
`has/find/size` (ICatalog; **`find` retorna `const T*`, nullptr si no existe —
NO `Result<T>`**, trampa mordida 3 veces) · `forEach(fn)` (⚠ **NO hay
`begin()/end()`**, trampa mordida) · `clear`.
**⚠ Namespace:** los catálogos son `RPG::Catalogs::EquipmentCatalog` NUNCA
`RPG::EquipmentCatalog` (trampa mordida 5 veces).

### 21 aliases (`using`)
Skill, Spell (comparte struct con Skill), Class, Race, Background, Passive,
Feat, Trait, Deity, Condition, Equipment, Consumable, Monster, Summon, Mount,
Trap, LootTable, Npc, Adventure, Event, Location. Totales del snapshot 28/08:
**2.416 entradas RPG + LocationCatalog** (el mundo, brecha B24).

## Qué falta / deuda del modelo

1. **432 monstruos con `maxHealth==0`** en el JSON fuente (deuda BITACORA 4.3):
   `populate_from_monster_catalog` tiene que inventar vida mínima.
2. `MonsterPhase` (fases de jefe) declarado pero sin usar (P1).
3. Los triggers complejos de `PassiveDefinition` (P2) necesitarán scripting/handlers;
   hoy SkillExecutor solo lee flags de contexto, no triggers de pasiva.
4. `RecoveryPile::ACTIVE` con semántica dudosa (comentario del enum lo admite).
5. `AdventureDefinition` (carta) y `AdventureScript` (guion) aún no se tocan —
   encuentro pendiente documentado en NarrativeEngine.h.
6. LocationDefinition: 3 deidades de la Parte 4.1 de la bitácora siguen ausentes
   del canon cargado (avisos de `validar_todo`).
