# 11/09/2026 — D-07 deidades: domain + plegarias + favor leídos

- **Gantt:** corte B, fase D (rebanada de D-07: solo deidades, 10 cartas).
- **Fila reclamada antes del primer fichero.** Sin commit a propósito (regla 2). Rama `feature/editor-s1-contratos`.

## Por qué deidades y no otra cosa

Con mapa + tablero: RpgCatalogs seguía libre de lógica ajena; `CharacterCreation.cpp` trae WIP ajeno SIN FILA (+47, rebanada D-06 de un fantasma: passives/traits del trasfondo + las 7 elecciones narrativas) — D-06 entero vetado y documentado aquí para que su autor reclame. Mounts (50, desajuste grande) y passives (trigger en prosa, D-11) quedan para después; traps/NPCs son narrativa.

## El hueco (medido)

El loader leía `domains[]`, `prayerSkillIds`, `title/alignment/symbolTags…` que 0/10 cartas traen, e ignoraba `domain` (string con comas, 10/10), `grantedSpells` (100 hechizos, 100/100 resuelven en spells.json), `favor{description,scaling}`, `obligations[]`, `compatibleWith[]`, `flavorText`, `rank`, `tier`, `rarity`.

## Qué se ha hecho

- `RpgCoreDefinitions.h` (limpio): `DeityDefinition` += `flavorText`, `rank`, `rarity`, `compatibleWithIds`, `obligations`, `favorDescription`, `favorScaling`.
- `RpgCatalogs.h`: `domain` se parte SOLO por comas (`split_comma_trim` nuevo; el " y " interior se conserva — partirlo inventaría fronteras: "noche urbana y protección…" queda entero); `grantedSpells`→`prayerSkillIds` (alias que suma); `tier`→`tier_min`; resto directo. `expected_fields()` actualizado.
- `CatalogContentTests.cpp`: spot Padre de la Forja (2 dominios con el " y " intacto, 10 plegarias, favor CAR, obligación literal) + barrido (10 con dominio, 19 totales, 100 plegarias resuelven).

## Verificación (build limpio en temporal)

- `test_catalog_content` **18/18**, `test_catalog_contract` 7/7, `test_inventory_engine` 7/7.
- Falsabilidad: con el loader de HEAD fallan 8 (2 D-03 + 4 D-05 + 2 deidades).
- Método (3ª vez): el restore cae a veces en el mismo segundo que el `.o` — `touch` + rebuild antes del verde final.

## Pendiente de commit (pathspec)

Súmense a la pila: `RpgCoreDefinitions.h` + hunks deidad en `RpgCatalogs.h`/`CatalogContentTests.cpp`.
