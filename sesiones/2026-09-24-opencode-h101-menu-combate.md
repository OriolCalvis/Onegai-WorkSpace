# 24/09/2026 — H1-01 (Gantt Fase H): el menú de combate sale de los datos en `Application`

Rodaja H1-01 (JUGABILIDAD_FASE_H.md: «movidas de tests/ a src/ ya hechas por zcode 22/09; que
`Application` deje de tener la lista fija de 5 entradas»). Entrega: el menú de combate real se
construye con `BattleMenuActions` (G-13) desde el `SkillSet` de la ficha + el catálogo, sin IDs a
mano. Sin commit (sin luz verde).

## Qué cambia

1. `include/Game/BattleMenuActions.h` + `src/Game/BattleMenuActions.cpp`: filas del HUD
   (`battleMenuRows`: disponible → `"Nombre  coste"`, bloqueada → `"Nombre (motivo)"`) y el puente
   transitorio `publishSheetSkills(SkillSet, SkillCatalog legacy)` → `RPG::Catalogs::SkillCatalog`.
   El puente es la acomodación H1-01 sobre el runtime actual: el combate (`BattleState`) resuelve
   hoy con el catálogo **legacy** (`Game::SkillCatalog`, `Game/Skill.h:125`) y el menú G-13 habla el
   **publicado** (`RPG::Catalogs::SkillCatalog`, `RpgCatalogs.h:187`). Solo entran las skills que
   `knownSkillIds()` conoce y que existen en el legacy — el menú nunca ofrece un id que
   `BattleState` no pueda resolver. Se retira cuando el combate hable el publicado (H2-06:
   `skills.json`). `SkillTarget` es el MISMO enum (alias H8-02), no hace falta mapearlo.
2. `src/Engine/Application.cpp`:
   - `init()`: options placeholder `{"Atacar","Pocion","Huir"}` (antes `"Tajo","Cura"` hardcodeados).
   - `refreshHud()`: en combate, filas = base + `battleMenuRows(buildBattleMenuSkillActions(...))`,
     `setOptions` solo si `rows != m_commandMenu->options()` (no se pisa el cursor; Application.h NO
     crece, sigue con 104 miembros — el diseño evita miembro nuevo).
   - `confirmBattleCommand()`: índice 0/1/2 → Atacar / Pocion / Huir; ≥3 → entrada correspondiente,
     `entry.available` guard, `selectTargetsFor(...,0)`, primer objetivo vivo, `translateToBattleAction`,
     y el resto (resolveAllyAction/enemy turn/tickRound) intacto.
3. `tests/Game/G13CombatCommandsTests.cpp`: `g13_filas_de_menu_mostran_coste_y_motivo_de_bloqueo`
   (filas con coste y motivo, formato del contrato) y `g13_puente_legacy_a_publicado_cubre_la_ficha`
   (el id inexistente en el runtime sale como bloqueada «Sense carta», no se pierde).

## Verificación (build `out/build/opencode-h007`, fecha real 24/09)

- `test_g13_combat_commands` **8/8** (2 nuevos + los 6 previos sin regresión).
- `test_battle_state` **17/17** y `test_input_system` **5/5** en verde (regresión H0-06/H0-01 intacta).
- `Application.cpp` compila (su `.o` vive en `libmotor_core.a`; build reconstruido con mi edición).

## Bloqueos ajenos registrados (regla 3, no tocados)

- `test_application_cableado` y `juego` NO enlazan: `Application.cpp` (working tree, SIN commitear por
  otro agente) referencia `Game::TitleMenuState::selectedItem()` y `Game::SaveSlots::find/load` desde
  `confirmTitleSelection`/`loadSlot`/`newestLoadableSlot` (~l.1670-1740, 2204, 2217, 2263), pero
  `TitleMenuState.cpp`/`SaveSlots.cpp` no están en ninguna lib de producción (solo en demos G-08.1
  `/demo_title_menu`, `/demo_nova_partida`). HEAD `Application.cpp` no tenía esas referencias: el
  bloqueo lo introduce su WIP en vuelo. Dueño de ese trabajo: resolverlo (añadir las fuentes a la
  lib de producción o stubeárselas al test). No es de H1-01.
- Guardia `linea_base_core.py` con 2 deudas nuevas, ambas ajenas: (a) `cmake/G13CombatCommandsTests
  2.cmake` (untracked, duplicado STALE que suma +2 `src/*.cpp` directos) — la "limpieza `* 2*`" se ha
  revertido otra vez; (b) `tests` de AdventureValidation editado por otro agente suma
  `src/RPG/AdventureBeatValidation.cpp` (+1 directo). Además `Application.cpp` 2444→2493 líneas
  (dominado por el WIP de título/slots de arriba; mi rodaja aporta el cableado ~20 líneas y la lógica
  real vive en `BattleMenuActions` con su test).
- Hallazgo: **H0-06 (`tickRound`) YA está implementado en el árbol** (`BattleState.cpp:379-417` + 
  decl `BattleState.h:166` + cableado `Application.cpp:1654-1659` + tests `BattleStateTests.cpp:623-725`,
  tica de estados verdes) y **no tiene fila en TABLERO** ni está commiteado; no lo reclamo (trabajo
  ajeno), queda registrado para su dueño.

## Notas

- Mi reclamación de fila H1-01 en «En curso» (opencode, 24/09) apareció y desapareció por edición
  concurrente (patrón documentado del 22/09). Queda entregada directamente en Hecho.
- Sin commit: nada verde de luz; `CMakeLists` y `Application.*` con trabajo ajeno vivo.