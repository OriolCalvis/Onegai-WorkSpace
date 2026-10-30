# Sesión 2026-09-24 — Codex — H8-02: unificar `SkillTarget`

## Decisión de alcance

El Gantt de Fase H medía dos tipos distintos llamados `SkillTarget`: uno global
en `Game/Skill.h` y otro canónico de catálogo en `RPG/Definitions/SkillDefinition.h`.
Se tomó `RPG::SkillTarget` como única representación. El mapa de código sitúa
`Game/Skill.h` en C-01 y `SkillDefinition.h` en D-02; `BattleState.cpp` es un
cruce C-01/D-09/D-11/G-13 y se dejó intacto.

No se tocó `Application.*`, CMake, catálogos ni WIP ajeno. Tampoco se fingió
que `ZONE_RADIUS` tenga ya resolución de área o que H1-01 esté integrado en el
flujo del jugador.

## Cambios

- Nuevo `include/RPG/SkillTarget.h`: enum canónico de seis destinos.
- `Game/Skill.h` reexporta el tipo con `using SkillTarget = RPG::SkillTarget`.
  Las grafías PascalCase históricas son enumeradores alias del mismo enum, para
  no cruzarse con `Application.cpp` mientras migra de forma independiente.
- `SkillDefinition.h` consume el header canónico y deja de declarar el enum.
- La adaptación temporal `Skill -> SkillDefinition` conserva el objetivo real
  de cada habilidad, en vez de fijarlo a `SINGLE_ENEMY`.
- La prueba de habilidades verifica identidad de tipo y compatibilidad de las
  grafías heredadas.

## Verificación

- `test_skill` 1/1.
- `test_battle_state` 1/1.
- `test_catalog_content` 1/1.
- `test_g13_combat_commands` 1/1.
- `tools/linea_base_core.py --rapido` y `git diff --check` verdes.
- `tools/analizar_duplicados.py --solo NOMBRE_AMBIGUO`: 3 → 2; desaparece
  únicamente `SkillTarget`. Permanecen `BattleAction` y `ShopItem`, que no son
  parte de H8-02.

## Entrega

MotorGraphico: commit `23fccc9` (`H8-02: unificar SkillTarget canonico`).

## Siguiente paso

Los consumidores nuevos deben usar `RPG::SkillTarget::SINGLE_*`. Los nombres
PascalCase son compatibilidad temporal; migrarlos puede ocurrir cuando se libere
el carril de `Application.cpp`, sin volver a introducir otro tipo.
