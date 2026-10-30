# 11/09/2026 — D-09: les cartes apliquen les seves condicions (implementación)

- **Gantt:** corte B, fase D (`MotorGraphico/planificacion/gantt_corte_b.puml` + `pla/dades.py`). Fila reclamada en `TABLERO.md` antes del primer fichero.
- **Commits:** ninguno a propósito (ver «Sin commitear a propósito»). Rama `feature/editor-s1-contratos`.
- **Deja esperando a Oriol:** nada. DEC-28 (banco de casos Java vs C++ validado por Oriol) sigue abierto para las familias más allá de condiciones; D-11 lo hereda.

## Qué se ha hecho

D-09 implementado en sus dos mitades (el cargador ya leía los 3 campos desde D-01; nadie los usaba):

1. **El ejecutor reporta, no toca.** `SkillExecutor::Out` lleva `pending_condition_id` + `pending_condition_rounds` (`include/RPG/EnhanceHooks.h`). Al final de `execute()` (`src/RPG/SkillExecutor.cpp`, PASO 8; la magnitud pasa a PASO 9): si la carta trae `apply_condition_id` y `final_degree >= apply_condition_min_degree`, se informa con las rondas de la carta (mínimo 1). Sin regla nueva de Nd6.
2. **Puente del Skill legacy.** `Skill` (`include/Game/Skill.h`) lleva `applyConditionId` / `applyConditionMinDegree` (int 0..3, por defecto 2=SUCCESS) / `conditionDurationRounds` (3); `SkillApplyResult` devuelve `pendingConditionId`/`pendingConditionRounds`. `src/Game/Skill.cpp`: `make_skel_def` acepta la condición (por defecto vacía = comportamiento antiguo) y `executeViaSkillExecutor` la propaga al resultado.
3. **El combate aplica.** `src/Game/BattleState.cpp`: helper `applyPendingCondition()` (anónimo) — aplica vía `ConditionEngine::apply` y estira `roundsRemaining` hasta las rondas de la carta sin acortar nunca una duración mayor ya activa. Se llama en `resolveAllyAction` (UseSkill) y `resolveEnemyTurn` (skill elegida), **después** de `notifyDamageReceived` para que el daño del propio golpe portador no rompa al instante una condición `breaksOnDamage` recién aplicada. Sin catálogo (`m_conditions == nullptr`), sin ficha o con id desconocido: no hace nada (el cargador ya falla en duro por D-01/D-10; el combate no debe reventar la partida). Línea de log `"<actor> aplica <id> a <objetivo>."`.

Ficheros tocados (los 6, todos limpios al empezar): `include/RPG/EnhanceHooks.h`, `src/RPG/SkillExecutor.cpp`, `include/Game/Skill.h`, `src/Game/Skill.cpp`, `src/Game/BattleState.cpp`, `tests/Game/BattleStateTests.cpp`. Más `planificacion/seguimiento/D-09.json` (nuevo, validado con `seguiment.load_progress`).

## Verificación

El target `test_battle_state` no enlaza en árbol sucio: el `GameSession.cpp` del C-17 (sin commitear) necesita `motor_world` y el `CMakeLists.txt` concurrente aún no se lo pasa. Sin tocarlo (regla 2), verificado compilando en limpio (`cmake -S MotorGraphico -B <tmp>`) y enlazando a mano con `libmotor_world.a`:

- `test_battle_state` **13/13**: 8 casos previos intactos + 5 D-09 nuevos —
  `executor_reporta_condicio_pendent_nomes_si_el_grau_hi_arriba` (dado {5}→SUCCESS con pendiente; {4}→PARTIAL sin pendiente),
  `una_carta_que_enverina_enverina_a_partir_d_exit` (10 de daño + `poison` con 5 rondas — las de la carta, no las 3 del catálogo — + línea de log),
  `amb_parcial_no_s_aplica_la_condicio` (5 de daño, sin condición),
  `el_torn_enemic_tambe_aplica_condicions`,
  `sense_cataleg_de_condicions_el_combat_no_rebenta`.
- Regresiones cercanas: `test_condition_engine` 11/11, `test_game_session` 4/4, `test_save_game` 19/19.

## Trabajo ajeno detectado (regla 2: no deshecho ni commiteado)

- Sesión concurrente pasando `tools/marcar_paquetes.py` (untracked): cabeceras «PAQUETES DEL PLAN» estampadas en ~40 ficheros, **incluidos mis 6** (llegaron mientras trabajaba). No las quito ni las arrastro a commit.
- `EditorState.cpp` (904 líneas) + `Application.cpp` + `GameSession.cpp` + `SaveGame.cpp` + `CMakeLists.txt` + `dades.py`/`gantt_corte_b.puml`/`pla.html` moviéndose en vivo (codex + otros). El desfase `EditorState.h/.cpp` que bloqueaba `motor_level` el 10/09 ya está resuelto en el árbol.
- `tests/RPG/CatalogContentTests.cpp` + `cmake/CatalogContentTests.cmake` + `tests/tools/` untracked (zcode, D-02).

## Sin commitear a propósito

Dos motivos: (1) mis 6 ficheros contienen la cabecera ajena de `marcar_paquetes.py` — commitearlos arrastraría trabajo ajeno; quitarla lo desharía; (2) el enlace de `test_battle_state` en el árbol requiere el hunk de `CMakeLists.txt` del propietario del C-17. Candidato a commit por pathspec cuando se asiente el árbol: mis hunks de los 6 ficheros + `D-09.json`.

## Pendiente (para quien corresponda)

- Propietario del `CMakeLists.txt`/C-17: pasar `motor_world` a los targets que enlazan `motor_level` (`test_battle_state` como mínimo) y cerrar el commit del C-17; entonces este D-09 puede commitearse detrás.
- D-04 sigue abierto (los nombres del asset mandan por DEC-02); este D-09 no depende de él porque usa los campos ya declarados.
