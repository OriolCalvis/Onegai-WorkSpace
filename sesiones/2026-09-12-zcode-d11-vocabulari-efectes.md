# 12/09 (ZCode) — D-11: Vocabulari d'efectes d'habilitat

## Sesión

**Tarea:** D-11 del Gantt (corte B) — Vocabulari d'efectes d'habilitat. 12 j.
**Fase:** D — Dades reals.
**DEC-02:** "Una habilitat ha de fer exactament el que diu la carta."
**MAPA_DEL_CODIGO:** `include/RPG/SkillEffect.h` (D-11 only), `src/RPG/SkillExecutor.cpp` (D-09+D-11), `src/Game/BattleState.cpp` (C-01,D-09,D-11,G-13), `tests/RPG/SkillEffectTests.cpp` (D-11 only).

## Què es va fer

### 1. Creació de `include/RPG/SkillEffect.h`
- `EffectFamily` enum: NONE, DAMAGE, HEAL, CONDITION, SUMMON, BUFF, DEBUFF, EXTRA_DICE, REROLL, STEAL, CUSTOM
- `ScalingStat` enum: NONE, CON, INT, DES, CAR, TIER
- `SkillEffectDef` struct: parsed `effect_scaling` del JSON
- `effect_family_from_string`, `effect_family_to_string`, `scaling_stat_from_string`, `scaling_stat_to_string`
- `parse_effect_scaling(raw_scaling)` → `std::optional<SkillEffectDef>`: rebutja efectes desconeguts
- `scale_magnitude(effect, caster, base)` → int: escala segons l'stat
- `execute_effect(effect, target, rng, base_magnitude)` → bool: aplica DAMAGE/HEAL

### 2. Creació de `src/RPG/SkillEffect.cpp`
- Implementació de totes les funcions declarades a `SkillEffect.h`
- Taules de conversió `family_table[]` i `scaling_table[]`

### 3. Actualització de `include/RPG/EnhanceHooks.h` (D-11 O2)
- `SkillExecutor::Out` afegeix: `effect_family`, `effect_supported`, `effect_diagnostic`, `applied_effects`
- `pending_summon_id` ja existia (D-11 O2)

### 4. Actualització de `src/RPG/SkillExecutor.cpp`
- Step 9 (D-11): parseja `effect_scaling` i determina si l'efecte es suportat
- Si `effect_scaling` buit → `effect_family = NONE`, `effect_supported = true`
- Si scaling desconegut → `effect_supported = false`, `effect_diagnostic` explica el motiu
- Si scaling vàlid → `effect_family = to_string(family)`, `applied_effects.push_back()`

### 5. Actualització de `src/Game/Skill.cpp`
- `executeViaSkillExecutor`: si `out.effect_supported` és false, retorna `SkillApplyResult` amb `effectSupported = false` i `hpChange = 0` (no gasta cost ni torn)
- `ApplySkillEffect`: estableix `def.effect_scaling = (heal) ? "INT" : "CON"`

### 6. Actualització de `src/Game/BattleState.cpp`
- Logs l'efecte rebutjat o aplicat després de `ApplySkillEffect`
- Gestiona `pendingSummonId` i `effectSupported` en ambdós costats (ally + enemy)

### 7. Actualització de `include/Game/Skill.h`
- `SkillApplyResult` afegeix: `effectFamily`, `effectSupported`, `effectDiagnostic`, `appliedEffects`

### 8. Actualització de `include/Render/ICombatant.h`
- `class ICombatant` passa a namespace global (era dins `namespace RPG`) per compatibilitat amb codi existent que fa `class ICombatant;`

### 9. Creació de `tests/RPG/SkillEffectTests.cpp`
- 19 proves: parse_effect_scaling (7), effect_family string (5), scale_magnitude (1), execute_effect (3), skill_executor effect (3), skill_executor summon (3)

### 10. Creació de `cmake/SkillEffectTests.cmake`
- Target `test_skill_effect` linkat contra `motor_rpg`

## Resultats

- `test_skill_effect`: 19 tests, 0 failures
- `test_skill`: 13 tests, 0 failures
- `test_condition_engine`: 12 tests, 0 failures
- `suite_gl_free`: compila correctament
- `test_catalog_content`: 30 tests, 0 failures

## Commits

- `8072609` — D-11: Vocabulari d'efectes d'habilitat (11 fitxers, +558 -15)
- `ICombatant` moved from `namespace RPG` to global scope per compatibilitat
