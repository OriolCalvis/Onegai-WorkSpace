# D0005 — Vocabulario de efectos de habilidad: qué familias existen y cómo se ejecutan

- **Estado:** `proposed` — espera a Oriol.
- **Fecha:** 2026-09-11 · **Autor:** opencode.
- **Encargo:** el plan (`pla/dades.py`, D-11) dice literalmente: *«Estimació provisional: abans de comprometre jornades cal una fitxa de decisió que enumeri i tipi cada família d'efecte; sense això, les 12 jornades són un ordre de magnitud, no un compromís.»* Esta es esa ficha.
- **Alcance:** decide la LISTA de familias y el CONTRATO motor por familia (qué trae la carta, qué señal devuelve el ejecutor, quién lo aplica, qué test lo cierra). No programa nada; desbloquea programar D-11. No toca reglas Nd6 ni contenido del canon.

## Contexto: lo medido hoy (11/09, scripts contra `assets/`)

1. **Las cartas no traen vocabulario ejecutable.** 492 skills + 210 spells: `effect` es siempre `{description (prosa), scaling}` — sin ningún `type` máquina. `scaling` sí se lee (D-02: INT 258, none 104, CAR 45, CON 34, DES 33, …). `magnitudeByDegree` existe en las 492 skills (el cargador acepta array y objeto); `magnitudeType` solo lo declaran 9 (`other`).
2. **Los campos máquina de efectos están vacíos.** `applyConditionId` / `apply_condition_id`: **0/702** cartas. `grantedSummonId` / `granted_summon_id`: **0/702**. El mecanismo D-09 (condición pendiente) y el `SummonCatalog` (201 invocaciones en `summons.json`, cargador D-02) no tienen hoy ninguna carta que los dispare.
3. **`mechanicTags` es taxonomía de sabor, no efectos.** `Invocacion` 203, `Magico` 105, `Oscuro` 89, `Fisico` 67, `Control` 62, `Veneno` 23, `Fuego` 25, `Curacion` 15…: describen de qué va la carta, no qué hace en combate. No son ejecutables y no deben serlo (prosa en castellano).
4. **El catálogo real tiene 16 condiciones** (`conditions.json`): `agarrado, anclado, asustado, caido, cegado, encantado, endeudado, enfurecido, envenenado, fatiga, guardia_rota, inconsciente, marcado, paralizado, ralentizado, sangrado`. Nótese: NO hay `quemado`, `congelado`, `aturdido` ni `silenciado` — los nombres catalanes del enunciado de D-11 no mapean 1:1 al catálogo (pregunta 1 para Oriol).
5. **El motor hoy resuelve:** daño/cura por magnitud y grado (`SkillExecutor`, intacto); condiciones vía señal pendiente + `ConditionEngine` (D-09, `test_battle_state` 13/13); ruptura por daño recibido (C-20). No resuelve nada más — y por DEC-02 no debe inventarse nada que la carta no diga.

## Hallazgo: D-11 mezcla dos trabajos distintos

El criterio de D-11 («cada família té contracte tipat, executor i prova pròpia») presupone familias tipadas que implementar. **No existen en las cartas.** Así que D-11 es en realidad:

- **(A) Contenido aguas arriba** (flujo `canon → cartas → motor`, sin aristas de vuelta): congelar la lista de familias y añadir a las cartas bloques `effect` legibles por máquina. Esto NO es trabajo de motor y NO lo puede cerrar ningún paquete C–G.
- **(B) Motor por oleadas**, siguiendo el patrón D-09 ya probado: la carta trae datos → `SkillExecutor::Out` añade señal `pending_*` (sin tocar fichas) → el orquestador (BattleState / GameSession) aplica → test por familia. Un efecto no soportado se rechaza con diagnóstico, sin gastar coste ni turno.

Programar (B) sin (A) es escribir un ejecutor sin cartas que lo alimenten — exactamente el estado actual de `granted_summon_id`.

## Opciones

### A — Tipar en C++ lo que la carta dice en prosa

El motor interpreta `effect.description` / `mechanicTags` (`Veneno` → envenena). Rápido de programar, **contrario a DEC-02** (manda el asset; la prosa no es contrato) y frágil ante cualquier reescritura de texto. Se descarta.

### B — (Recomendada) Primero contenido, motor por oleadas (propuesta abajo)

Oriol congela familias (preguntas 1–4); las cartas ganan bloques máquina (quién los escribe: pregunta 5); el motor implementa por oleadas con su test cada una. Las 12 jornadas se reestiman por oleada, no en bloque.

### C — Cerrar D-11 solo con condiciones

El mecanismo ya existe (D-09) y bastaría añadir `applyConditionId` a cartas. Pero el criterio de D-11 exige *cada* familia, y deja fuera invocar, robar, buffs y dados. Parche, no cierre. Se descarta como cierre (vale como oleada 1).

## Decisión propuesta: oleadas

- **Oleada 1 — Condiciones (motor HECHO, falta contenido).** Mecanismo D-09 verificado. Trabajo restante: añadir `applyConditionId` + `applyConditionMinDegree` + `conditionDurationRounds` a las cartas que correspondan (aguas arriba). Test: el `test_battle_state` actual + recorrer el catálogo afirmando que las cartas marcadas aplican (extensión de `test_catalog_content`, sin tocar su zona viva: coordinar con zcode).
- **Oleada 2 — Invocación (pequeña, solo motor).** Campos y catálogo ya cargan (`granted_summon_id`, `SummonCatalog`); falta la señal `pending_summon_*` en `Out` y que el combate la materialice (`GameSession::spawnEnemyFromCatalog` existe desde C-17). Estimación orientativa: ~2 j + test `test_skill_effects_summon`.
- **Oleada 3 — Buffs/debuffs y robar (necesita diseño).** Bonificar aliados/uno mismo y penalizar enemigos: ¿vía `conditionModifiers` de `CharacterSheet` (entran en el `stat()` total y las ve todo el combate) o vía nueva fuente de bonus? Robar: ¿oro (`GameSession` lo lleva), objetos, ambos? ¿qué pasa con grado PARCIAL? Sin respuesta no se estima.
- **Oleada 4 — EXCLUIDA hasta ruling.** Acciones extra, más dados y repetir tirada tocan las reglas de dados («Cap regla nova de Nd6», criterio de D-11). `tallar`/`devastar` no tienen mapeo conocido (¿sangrado con otro nombre? ¿daño mayor?). Preguntas 2–3.

Verificación por oleada: una carta de cada familia aplica su efecto observable sobre sesión de prueba; familia desconocida falla con el nombre del efecto sin dejar la sesión a medias (criterio D-11, intacto).

## Consecuencias

- Las 12 j de D-11 quedan **descompuestas, no comprometidas**: O1 ~0 motor (contenido), O2 ~2 j, O3 sin estimar hasta diseño, O4 excluida. El total real se fija cuando Oriol responda.
- DEC-28 intacto: donde el motor y las cartas Java diverjan, banco de casos validado por Oriol → prueba automática. Candidato natural: la oleada 3.
- D-09 no se reabre: es la plantilla, no el precedente a reescribir.

## Preguntas para Oriol (esto es lo que desbloquea programar)

1. Las 16 condiciones son la lista cerrada, ¿o faltan `quemado`/`congelado`/`aturdido`/`silenciado` (los nombres del enunciado D-11)? Si faltan, ¿quién las da de alta y con qué stacking/daño?
2. ¿Qué significan `tallar` y `devastar` en el reglamento? (¿sangrado? ¿daño?)
3. Acciones extra / más dados / repetir tirada: ¿entran al Nd6 o quedan fuera del corte B?
4. Buffs/debuffs: ¿como condiciones con `statMod` o como fuente propia? ¿Y robar qué roba?
5. Los bloques máquina de las cartas (efecto tipado), ¿los escribe el lado cartas (dndWeebCC) o se prototipan en el motor con copias locales `origin` explícito (criterio E-16)?
