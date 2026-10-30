# 12/09/2026 — D-07 loot: tierRange, oro, drops y pista leídos

- **Gantt:** corte B, fase D (rebanada de D-07: solo loot, 201 tablas).
- **Fila reclamada antes del primer fichero.** Sin commit a propósito (regla 2). Rama `feature/editor-s1-contratos`.

## El hueco (medido)

El loader leía `minGold/maxGold` planos que 0/201 cartas traen e ignoraba `gold{min,max}`, `tierRange[2]` (`tier` plano es 0 en las 201), `clue` y el sistema alternativo `drops[]` (chance+condición+item, en paralelo 1:1 a `entries[]` peso/min/max).

## Qué se ha hecho

- `RpgCoreDefinitions.h` (limpio): `LootTableDefinition` += `tierMin/tierMax`, `clue`, `struct Drop{itemId, chance, condition}` + `vector drops`. D-11 decidirá qué sistema ejecuta; aquí solo se lee.
- `RpgCatalogs.h`: `gold.min/max`, `tierRange[0/1]` (defensivo: len>=2), `clue`, `drops[]` (item←item/itemId/refId). `expected_fields()` actualizado.
- `CatalogContentTests.cpp` (al final, tras los tests ajenos NPC/Loc/Adv): spot Adalid (rango 5/5, oro 300–750, pista, drops==entries) + barrido (201 oro coherente, 201 paralelas, items no vacíos).

## Verificación (build limpio en temporal)

- `test_catalog_content` **30/30** (22 míos+previos + 6 ajenos + 2 loot), contrato 7/7, inventario 7/7.
- Falsabilidad: con el loader de HEAD fallan 14 (todos mis tests de loader).
- Convivencia: mis hunks + los 6 tests ajenos (NPC/Loc/Adv, otra sesión opencode) intactos en el mismo fichero; regiones distintas.

## Tablero ajeno (12/09, mucho movimiento)

- C-23/C-24/D-08 salieron de En curso (commits `757e063`, `bffc0c7` + sesiones); D-10 vive (cmake/validar_todo, con commits). Mi pila sigue sin commitear.
- `RpgCatalogs.h` trae además hunks ajenos en race/trait (¿D-06 en marcha sin fila?) + sello: mis regiones intactas, no tocadas las suyas.

## ⚠️ Conflicto detectado al cerrar (D-03 duplicado)

Al mover mi fila a Hecho he visto una fila nueva: **zcode reclama D-03 hoy 12/09** (`RpgCatalogs.h` solo loader MonsterDefinition + struct + `CatalogContentTests.cpp` solo casos de monstruos) — **la región exacta de mi D-03 terminado y verificado el 11/09** (Hecho + sesión `2026-09-11-opencode-d03-monstres.md`, 12/12 con falsabilidad). No toco su fila (protocolo) ni mi código. Posibilidades: no vio mi fila Hecho, o considera que sin commit el trabajo no existe. Lo dejo a Oriol/zcode: mis hunks están intactos y verificados hoy de paso (los 2 tests D-03 fallan con HEAD y pasan con mi loader, verificado en la ronda de falsabilidad de esta misma sesión). Si zcode reimplementa desde HEAD habrá conflicto textual en `defenses{}`/`derived{}`/`lootIds` al commitear.

## Pendiente de commit (pathspec, pila creciente)

D-09, D-03, D-05×3, D-07 deidades/summons/mounts/loot.
