# 13/09/2026 — Verificación post-consolidación (solo lectura, sin código)

- **Rama MG:** `feature/editor-s1-contratos` (sucia, 100+ entradas, con staged ajeno). Build limpio en `/tmp` (`mg-full`), sin tocar el árbol.
- Reproduce con: `cmake -S MotorGraphico -B <tmp> -DCMAKE_BUILD_TYPE=Debug`, build por target y `ctest -R ^<binario>$` desde el build dir.

## VERDE (16 binarios, todos 100%)

| Binario | Casos | Nota |
|---|---|---|
| test_catalog_content | 32 | 26 míos+previos + 6 ajenos NPC/Loc/Adv conviviendo |
| test_catalog_contract | 7 | declaraciones incl. mis aliases |
| test_inventory_engine | 7 | commiteado `5738655` |
| test_battle_state | 14 | 8 previos + 5 míos D-09 + 1 ajeno de items |
| test_condition_engine | — | verde |
| test_game_session | — | verde |
| test_save_game | — | verde |
| test_skill_effect | 19 | D-11 O2: mi mecanismo extendido funciona |
| test_enemy_catalog_bridge | 6 | D-08 ajeno funciona |
| test_character_creation | 4 | flujo fantasma D-06 funciona |
| test_boundington_tutorial, catalog_content_validation, test_city_regeneration, test_project_catalog, test_editor_document_session, test_editor_actions | — | verdes |

## ROJO (6 binarios, UNA sola causa raíz — no tocar)

`test_application_scenes`, `test_catalog_classes_races_backgrounds`, `test_skill_effect_real_assets`, `test_enemy_catalog_real_assets`, `test_conditions_real_assets`, `test_catalog_restants`: **no compilan** todos por `src/Engine/Application.cpp` (widgets/comandos de menú Pause/GameOver/Inventory/Shop referenciados sin declarar: `createPauseMenuWidgets`, `confirmPauseCommand`, `MenuLeft/Right`, `useItem/equipItem/unequipItem`, `kMenuPadding`…). Parche de menús G-10/G-11/G-12 a medias, con staged ajeno encima — **vetado** (regla 2 + fila de nadie; que lo cierre su autor).
Ojo: `test_catalog_classes_races_backgrounds` y `test_catalog_restants` son cobertura D-06/D-07 que HOY no puede correr nadie por este único fichero.

## Sin probar

5 `test_post_processing*` + 2 `motor_editor_smoke_*`: necesitan GL/display (en macOS local no hay; los cubre CI Linux).

## Para Oriol (estado de mi pila tras la consolidación)

Casi todo mi trabajo 11-12/09 está versionado por mí o absorbido (`5738655`, `8072609`, rescates). Pendiente solo mío y bloqueado por coautoría: mis 5 tests D-09 en `tests/Game/BattleStateTests.cpp` (conviven con tests de items ajenos) + hunks D-07/loot/traps/deity en `CatalogContentTests.cpp` si alguien los reclama.
