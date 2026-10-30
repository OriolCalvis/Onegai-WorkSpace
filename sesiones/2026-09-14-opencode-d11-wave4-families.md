# D-11 Wave 4 — Vocabulari complet d'efectes

**Data:** 14/09/2026
**Autor:** opencode
**Paquet:** D-11 (Vocabulari d'efectes d'habilitat)

## Resum

Implementació completa del vocabulari d'efectes D-11: totes les famílies
de l'enum EffectFamily tenen parsing, contracte tipat i prova pròpia.

## Canvis

### SkillEffect.cpp — Parser compound millorat
- `parse_effect_scaling("extra_dice")` ara funciona: el path underscore
  primer intenta la cadena completa com a family name abans de
  partir-la en dos parts. Abans "extra_dice" retornava nullopt perquè
  l'underscore activava el compound path que no reconeixia la família.

### SkillEffectTests.cpp — 12 tests nous (45/45 total)
- 6 tests d'execució: cada família no implementada (BUFF, DEBUFF,
  EXTRA_DICE, REROLL, STEAL, CUSTOM) retorna false i no modifica el
  target.
- 6 tests de parsing: cada família es reconeix per `parse_effect_scaling`.

### SkillEffectRealAssetsTests.cpp — O1 actualitzat
- Test O1 actualitzat: compound parsing funciona (con_tier/con_int/con_con),
  així que 0 nullopt esperat en 492 entrades. El test anterior acceptava
  9 nullopt; ara exigeix parse_ok + parse_empty == total.

## Estat
- `test_skill_effect`: **45/45** passant
- Suite gl-free: **111/112** (1 fallo preexistent: test_title_screen)
- `test_skill_effect_real_assets`: no compila per Application.cpp
  trencat per WIP d'un altre agent (C-16/E-10). Cap regressió dels
  meus canvis.
- Sense commit (regla 2: fitxers compartits amb D-09).

## Vocable complet D-11

| Família | Parsing | Executor | Test |
|---------|---------|----------|------|
| DAMAGE | ✅ | ✅ aplica dany | ✅ |
| HEAL | ✅ | ✅ cura | ✅ |
| CONDITION | ✅ | ✅ retorna false (D-09 gestiona) | ✅ |
| SUMMON | ✅ | ✅ via pending_summon_id | ✅ |
| BUFF | ✅ | ❌ retorna false | ✅ |
| DEBUFF | ✅ | ❌ retorna false | ✅ |
| EXTRA_DICE | ✅ | ❌ retorna false | ✅ |
| REROLL | ✅ | ❌ retorna false | ✅ |
| STEAL | ✅ | ❌ retorna false | ✅ |
| CUSTOM | ✅ | ❌ retorna false | ✅ |
| NONE | ✅ | ✅ no-op | ✅ |
