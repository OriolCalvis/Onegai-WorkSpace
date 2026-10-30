# 11/09/2026 — D0005: ficha de vocabulario de efectos (desbloquea D-11)

- **Gantt:** corte B, fase D. El plan exige esta ficha antes de comprometer las 12 j de D-11.
- **Fila reclamada antes del primer fichero.** Rama: N/A (solo `decisiones/` + tablero).
- **Programación este turno:** ninguna a propósito — ver «Por qué una ficha y no código».

## Por qué una ficha y no código

Con `MAPA_DEL_CODIGO.md` + TABLERO en vivo, todo paquete programable con criterio de cierre está reclamado o bloqueado: RpgCatalogs (zcode D-04), editor/Application/Project/Level + regeneración (codex), tests/Game|Engine|BitmapFont (C-24, que además ya creó `BusinessEconomyTests`, `BusinessTests`, `MoralityTests`), diálogo/Application (C-16), `src/Game/Morality.cpp` (solaparía los tests de moral de C-24), C-18 (trabajo ajeno sin commitear + refactor diferido por su autor), E-/F-/G- (dependencias sin cumplir o ficheros codex). Verificado además: INF-06 (F0) está cerrado en efecto — cero ficheros con ` 2`/`.DS_Store`/`.ppm` bajo seguimiento (`Check 2.h` difiere de `Check.h` y está ignorado explícitamente: no se toca, regla 2) y `.gitignore` ya cubre `builds/`, `.DS_Store`, `*_output.ppm`.

## Qué se ha hecho

- `decisiones/D0005-vocabulario-efectos-habilidad.md` (NUEVO, MADR, `proposed`): enumera y tipa cada familia antes de programar D-11.
- Mediciones que la sostienen (scripts de solo lectura contra `assets/`):
  - 492 skills + 210 spells: `effect` = `{description, scaling}` siempre, sin `type` máquina; `scaling` INT 258 / none 104 / CAR 45 / CON 34 / DES 33…
  - `applyConditionId`: **0/702**. `grantedSummonId`: **0/702**. El mecanismo D-09 y `SummonCatalog` no tienen carta que los dispare.
  - 16 condiciones reales (`agarrado…sangrado`): sin `quemado`/`congelado`/`aturdido`/`silenciado` (pregunta 1).
  - `mechanicTags` = sabor (`Invocacion` 203…), no ejecutable. `magnitudeType` solo en 9.
- Propuesta: oleadas O1 condiciones (motor hecho, falta contenido) / O2 invocación (~2 j) / O3 buffs+robo (sin diseño) / O4 excluida (reglas Nd6 + `tallar`/`devastar` sin mapeo). 5 preguntas numeradas para Oriol.

## Verificación

- La ficha cita solo datos medidos (números reproducibles con los one-liners de arriba) y ficheros/campos reales del motor (`SkillExecutor::Out.pending_*`, `ConditionEngine`, `GameSession::spawnEnemyFromCatalog`, `SummonCatalog`).
- `seguiment.py`-style: N/A (decisiones, no seguimiento). Formato espejo de D0004 (Estado/Fecha/Autor/Encargo/Alcance/Contexto/Opciones/Decisión/Consecuencias/Preguntas).

## Tablero

- Fila En curso → Hecho con esta huella. Fila nueva en «Esperando a Oriol» para D0005 (precedente qwen D0003/D0004).
- **Commit:** `35ae457` — solo `decisiones/D0005-vocabulario-efectos-habilidad.md` vía `herramientas/git-safe.sh` (fichero nuevo, cero contenido ajeno).

## Pendiente de commit (mío, cuando se asiente el árbol)

D-09 (6 ficheros + `D-09.json`), D-05-motor (`InventoryEngine.cpp` + `InventoryEngineTests.cpp` + hunk CMakeLists), D-05-maxHealth (`CharacterSheet.h` + 2 hunks + 2 tests). Más la fila D-04 de zcode, que desapareció de En curso este turno (¿terminó? `RpgCatalogs.h` sigue modificado — se trata como vivo).
