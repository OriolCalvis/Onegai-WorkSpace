# D-11 O3 executat — SkillExecutor escala la magnitud per l'estat de la carta

**Data:** 14/09/2026
**Autor:** opencode
**Paquet:** D-11 (Vocabulari d'efectes d'habilitat) — pendent O3

## Resum

Pendent documentat de D-11: `SkillExecutor::execute` calculava
`final_magnitude` amb `skill->magnitude(d)` sense aplicar `scale_magnitude`.
Ara el PASO 10 escala per l'estat que declara la carta quan hi ha un
`CharacterSheet*` com a caster (O3 del pla D-11). El camí legat de batalla
(`Game/Skill.cpp`, bridge ICombatant→ExecutionContext amb
`override_base_dice=1+stat`/`override_target_dice`) passa `ctx.caster =
nullptr`, així que el scaling hi és no-op: **els demos i la campanya no
canvien de números**. Només realitza on hi ha dades canòniques (tests i,
en el futur, la ruta que injecti CharacterSheet reals).

## Canvis

### src/RPG/SkillExecutor.cpp — PASO 10 escala
- Hoisted `std::optional<SkillEffectDef> effect_def;` (PASO 9) perquè el
  PASO 10 el pugui reutilitzar (una sola parsejada, no doble).
- PASO 10: `if (effect_def && effect_def->scaling != ScalingStat::NONE &&
  out.final_magnitude > 0) out.final_magnitude =
  scale_magnitude(*effect_def, ctx.caster, out.final_magnitude);`
  abans del `x1.5` de CRITICAL. La condició `> 0` manté el BOTCH en 0
  (gairebé totes les cartes tenen magnitud 0 a BOTCH; `scale_magnitude`
  fa `max(1, base)` i convertiria un 0 en 1). `scale_magnitude` ja fa
  no-op amb `scaling == NONE` o `caster == nullptr`.
- `#include <optional>` afegit.

### tests/RPG/SkillEffectTests.cpp — 46/46 (test nou determinista)
- `skill_executor_escala_final_magnitude_amb_caster`: carta inline amb
  `effect_scaling="INT"` i magnitudes {0,50,100,200}:
  - SUCCESS (un 5) amb INT=8 → 100·1.08 → **108**.
  - PARTIAL (un 4) amb INT=8 → 50·1.08 → **54**.
  - INT=0 → base **100** (sense scaling).
  - Sense caster (camí legat) → **100** (no-op).
  - BOTCH (dos 1) amb INT=8 → mag 0 es manté **0**.
  - CRITICAL per sisos amb INT=8 → 200·1.08=216; ×1.5 → **324** (l'ordre
    escala-abans-que-x1.5 queda clavat).

### tests/RPG/SkillEffectRealAssetsTests.cpp — O3 a magnitud escalada
- Expectativa de `d11_skill_executor_sobre_skill_publicada_magnitud_coincideix`:
  ara `expected = scale_magnitude(parse_effect_scaling(s->effect_scaling),
  &caster, s->magnitude(final_degree))` (caster baseStats=10 → ×1.1)
  abans del ×1.5 de CRITICAL; gate `>0` per no encendre el BOTCH.

### planificacion/seguimiento/D-11.json — reparat i actualitzat
- JSON malformat (sobrava un `)` exterior a la línia 39) arreglat:
  `json.load` OK ara.
- `caveats_y_pending[0]` substituït pel registre de la connexió O3
  (gate, radio de canvis 370/492, contractes actualitzats).
- `estadisticas_tests` actualitzat (46/46 SkillEffectTests; O3 real
  assets amb reverificació pendent).

## Verificació (build de cobertura temporal — el `build/` del repo està
roto per la sessió en vol sobre Application.cpp, regla 2)

- `test_skill_effect` **46/46 PASS** (inclou el nou test de escala).
- `test_skill` 13/13 · `test_battle_state` 14/14 · `test_game_session`
  18/18 · `test_catalog_content` 32/32 — cap regressió.
- `demo_nd6_distribution` corre i magnitude mitjana 3.17 (mateixa
  distribució que abans: l'escala no afecta el camí legat).
- **`test_skill_effect_real_assets` NO verificable avui**: el target
  enllaça `motor_core` i `src/Engine/Application.cpp` (edició ajena en
  vol, zona GDB-12) no compila. La seva expectativa O3 s'ha ajustat per
  construcció (mateixa font `scale_magnitude`), queda pendent de
  reverificar quan el repositori torni a compilar.

## Sin commit (regla 2)
Ficheros nous/editats sense versionar (SkillExecutor.cpp compartit D-09,
pila D-11 uncommitted que espera el commit del seu dueño per als rojos
ZCode: `test_object_catalog`, `test_skill_effect_real_assets`).

## Ampliació 14/09 — desacoblament de motor_core i O3 verificat

`test_skill_effect_real_assets` no es podia executar: el seu cmake
enllaçava `motor_core`, que arrossegava `src/Engine/Application.cpp`
(edició aliena en vol) i no compilava. El target només fa servir motor_rpg
+ motor_json (SkillCatalog, CharacterSheet, ExecutionContext, SkillExecutor,
SkillEffect). S'ha tret `motor_core` de
`cmake/SkillEffectRealAssetsTests.cmake` (fitxer propi de la sessió D-11;
no toca cap cmake aliè ni Application.cpp).

**Verificat (build temporal GL-free):** test_skill_effect_real_assets
**6/6 PASS**, ara amb el contracte O3 a magnitud escalada
(`d11_skill_executor_sobre_skill_publicada_magnitud_coincideix` passa amb
la expectativa `scale_magnitude`). Regressió ratllada alhora: test_skill
13/13, test_battle_state 14/14, test_game_session 18/18, test_catalog_content
32/32, test_skill_effect 46/46. El target queda desacoblat de la zona

---

## AFEGIT (vespre del 14/09) — Race multi-agent detectada; no s'ha tocat res ajeno

Després de la verificació es va intentar compilar l'agregat `suite_gl_free`
en el build temporal i va esclatar amb `Error 2` a `motor_rpg`:
`SkillEffect.cpp:177 unknown type name 'ConditionEngine'` i
`SkillEffect.cpp:195 no member named 'Catalogs' in namespace 'RPG'`.

Forensic (repo anidat; toplevel = `MotorGraphico-main`, paths amb prefix
`MotorGraphico/`):

- El canvi O3 de `SkillExecutor.cpp` i el test
  `skill_executor_escala_final_magnitude_amb_caster` **ja són a HEAD**,
  inclosos al commit `b80dd43 "Fix y push del dia 14"` (un agent va
  escombrar l'edició no cometada en un push posterior). Irreversible; es
  registra el fet.
- Un agent ajeno té WIP sense cometar **en vol sobre fitxers compartits**:
  - `src/RPG/SkillEffect.cpp` (+44): `execute_effect` guanya placeholders
    per CONDITION/SUMMON/BUFF/DEBUFF/EXTRA_DICE/REROLL/STEAL/CUSTOM que
    retornen `true`, i `CharacterSheet* target_sheet =
    target.character_sheet();` sense els includes corresponents. En un
    estat transitori (sense includes) `motor_rpg` no compila — l'error de
    `ConditionEngine` es va veure just en un d'aquests instants i ja no
    reprodueix text literal al diff.
  - `tests/RPG/SkillEffectTests.cpp`: 5 renames de tests
    (`execute_effect_buff_returns_false`→`_true`, id. debuff/extra_dice/
    reroll/steal) alineats amb els nous placeholders `returns true`.
- **Regla 2 aplicada**: no s'ha canviat, corregit ni comitcat cap d'aquest
  WIP ajeno. **Regla 3**: queda registrat aquí i a `D-11.json`.
- Conseqüència: tot el que depèn de `motor_rpg` (incloses les meves
  verificacions D-11) **no és reconstruible mentre l'agent ajeno no tanqui**
  el seu canvi a SkillEffect. Les verificacions 46/46 i 6/6 es van executar
  abans d'aquesta finestra i són vàlides per a l'HEAD `b80dd43`.
- Següent pas natural: quan l'agent ajeno aterri el canvi,
  recompilar `suite_gl_free` i reintentar el test dues vegades (el 46/46 i
  el 6/6) al nou HEAD, per re-verificar D-11 sense fricció.
d'Application.cpp millorant a més la sol·licitud de ZCode dels «rojos que
només esperen commit del seu dueño».