# 11/09/2026 — D-07 mounts: armor, vida, plazas y reglas de monta leídos

- **Gantt:** corte B, fase D (rebanada de D-07: solo monturas, 50 cartas).
- **Fila reclamada antes del primer fichero.** Sin commit a propósito (regla 2). Rama `feature/editor-s1-contratos`.

## El hueco (medido)

El loader leía `maxHealth/ca/carryWeightKg/mountedStatBonus…` que 0/50 cartas traen e ignoraba `health` (50/50), `armor` (11–15), `capacity` (1×20/2×30, plazas NO kg), `mountedTags`, `terrain`, `dismountTrigger`, `ridingRequirements{minimo,stat}`, `upkeep` (50/50) y `stats` (struct sin campo: CON/DES a cero siempre).

## Qué se ha hecho

- `RpgCoreDefinitions.h` (limpio): `MountDefinition` += `stats[4]`, `capacity = 1`, `terrain`, `mountedTags`, `dismountTrigger`, `requiredRideStat/Min`, `upkeep`.
- `RpgCatalogs.h`: `health`→vida, `armor`→CA, resto directo; `canFly/canSwim` intactos (inferirlos de `terrain:"aire"` sería inventar); `mountedSkill` (objeto sin id) para D-11.
- `CatalogContentTests.cpp`: spot Ave de Silla (CA 11, vida 12, 1 plaza, aire+llano, DES 1, regla literal) + barrido (50 con vida y terreno, 20/30 plazas).

## Verificación (build limpio en temporal)

- `test_catalog_content` **22/22**, `test_catalog_contract` 7/7, `test_inventory_engine` 7/7.
- Falsabilidad: con el loader de HEAD fallan 12 (2 D-03 + 4 D-05 + 2 deidades + 2 summons + 2 mounts).

## Pendiente de commit (pathspec)

Súmense hunks mount a la pila (D-09, D-03, D-05×3, D-07 deidades+summons).
