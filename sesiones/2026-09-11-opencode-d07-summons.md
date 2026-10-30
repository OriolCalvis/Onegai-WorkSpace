# 11/09/2026 — D-07 summons: vida, duración, control y origen leídos

- **Gantt:** corte B, fase D (rebanada de D-07: solo summons, 201 cartas; base de la O2 de mi D0005).
- **Fila reclamada antes del primer fichero.** Sin commit a propósito (regla 2). Rama `feature/editor-s1-contratos`.

## El hueco (medido)

El loader leía `durationRounds/maxHealth` que 0/201 cartas traen e ignoraba `health` (201/201 > 0), `duration` ("concentration" 117 / "1_min" 84), `control` (jugador 130 / car_invocador 57 / automatica 14) y `summonedBy` (el reverso de `granted_summon_id`). Los 201 summons cargaban con vida 0.

## Qué se ha hecho

- `RpgCoreDefinitions.h` (limpio): `SummonDefinition` += `durationRaw`, `control` (en crudo: `car_invocador` es opaco, no se interpreta), `summonedBySkillId`.
- `RpgCatalogs.h`: `health`→`maxHealth`; `duration` en crudo + `"concentration"` activa el `concentrationRequired` que el struct ya preveía; `"1_min"` a rondas lo decide D-11 O2 (1 min ≠ rondas sin regla); `control` y `summonedBy` directos. `expected_fields()` actualizado.
- **Bug cazado por el test nuevo:** el loader leía `concentrationRequired` DOS veces y la segunda (plana) pisaba la primera — eliminada con comentario.
- `CatalogContentTests.cpp`: spot Arma Flotante (vida 4, concentration, jugador, arsenal_del_rey) + barrido (201 con vida, 117/84, 130/57/14).
- Fuera: `attacks` (prosa sin defensa/magnitud), `passive`-objeto, `stats` (ausente en carta: 201 a cero — hueco de contenido aguas arriba, no de motor), `ca` sin dato (default 10).

## Verificación (build limpio en temporal)

- `test_catalog_content` **20/20**, `test_catalog_contract` 7/7, `test_inventory_engine` 7/7.
- Falsabilidad: con el loader de HEAD fallan 10 (2 D-03 + 4 D-05 + 2 deidades + 2 summons).

## Tablero ajeno (regla 2)

- Reparada duplicación de mi fila D-07 en En curso (edición concurrente la restauró tras moverla a Hecho; la de Hecho estaba intacta).
- La fila D-08 es de la otra sesión opencode (bridge + tests/Game + mención a "zcode C-16" que no está en el tablero): ni mirada.
- codex C-23 (Render/MapDocument/Application) y C-24 (tests) intactos.

## Pendiente de commit (pathspec)

Súmense hunks summon a la pila (D-09, D-03, D-05×3, D-07 deidades).
