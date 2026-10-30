# 11/09/2026 — C-24: arranque de la red de seguridad (moral 19/19)

- **Commits:** ninguno (no autorizado; trabajo en vuelo en los ficheros correspondientes). Rama `feature/editor-s1-contratos`.
- **Deja esperando a Oriol:** luz verde para commitear por pathspec lo acumulado de C-17 más el arranque de C-24 (sigue pendiente de la pregunta ya hecha).

## Qué se ha hecho

El usuario pidió coger el Gantt de MotorGraphico y seguir programando. C-17 y C-18 ya estaban cerrados por otra sesión (`seguimiento/C-17.json`, `C-18.json` = «Tancat») y D-02 cometido como `86bf869`. Se eligió **C-24 «Xarxa de seguretat al camí que es juga»** (depende de C-03/C-16/C-17, todas cerradas; trabajo aditivo de tests).

### Segunda pasada (mismo día): el tramo GameMode que quedaba delegado

- `MotorGraphico/tests/Game/BusinessModeTests.cpp` — **nuevo, 16/16 PASS vía CTest** (`test_business_mode`). Cubre justo lo que el compañero dejó escrito en `BusinessEconomyTests.cpp` («aquestes les fa integrals i les deixa per a un futur test d'integració»): abrir el cartel con `interact()` (celda del jugador, objeto `Prop` con `business.price > 0`), compra (gasto, doble compra, sin oro, cartel cerrado), renta («la moral se mueve con el CAMBIO»: +50→-10, -50→+10, bucle no cultiva, topes kMin/kMax sin devolución de cambio, negocio ajeno) y cobro integrado tras la compra (justo/abusivo/generoso/cero). Valores esperados derivados línea a línea de `src/Game/GameSession.cpp` (595-631).
- `MotorGraphico/CMakeLists.txt` — una línea aditiva más (`add_motor_unit_test(test_business_mode ...)`, mismo patrón de enlace que sus hermanas). CMake reconfigurado (`cmake -S . -B build`, 0 errores) y el target construido y registrado.

### Tercera pasada (mismo día): la màquina d'estats del GameMode

- `MotorGraphico/tests/Game/GameModeTests.cpp` — **nuevo, 14/14 PASS vía CTest** (`test_game_mode`). La otra mitad pendiente escrita en `seguimiento/C-24.json`: "la màquina d'estats de GameMode ... tenen tests unitaris". Cubre Exploración→Diálogo (speaker/líneas), Diálogo→Exploración vía `closeInteraction()`, NPC mudo (diálogo vacío), tienda (ofertas resueltas con precio del catálogo, `buy` cobra y entrega, sin oro falla, fuera de rango falla, comprar en diálogo no hace nada, `sell` paga `buybackPercent` y retira, vender sin inventario), contenido mal escrito (tienda sin precio resuelto → muta a Diálogo; artículo desconocido → salta con registro en `log()`) y GameOver de salida con combatiente muerto. Registros derivados de `startInteraction`/`buy`/`sell`/`closeInteraction` y del ctor (src/Game/GameSession.cpp).
- `MotorGraphico/CMakeLists.txt` — una línea aditiva más (`add_motor_unit_test(test_game_mode ...)`, mismo patrón de enlace). CMake reconfigurado (0 errores) y target construido y registrado en CTest.

### Cuarta pasada (mismo día): el traspàs entre sesiones (la C-03 que C-24 exige)

- `MotorGraphico/tests/Game/CarryOverTests.cpp` — **nuevo, 7/7 PASS vía CTest** (`test_carry_over`). Pina la invariante del criterio de C-03 ("Moral, negocis, passos des de l'últim cobrament i or es transfereixen sencers entre sessions; el traspàs és una sola funció") contra `captureCarryOver`/`applyCarryOver` (las mismas que usa SaveGame.cpp): oro y moral enteros, el negocio comprado viaja intacto (precio, renta 150, ingreso base), `applyCarryOver` ASIGNA (no acumula) y clampa el oro a >=0, la moral viaja con `setValue` SIN ensuciar el historial (solo el valor, el puro del comentario de `applyCarryOver`), el cobro tras el traspaso parte de la renta traspasada (incomeFor(12,150)=14 + sostenido -2) y un traspaso vacío resetea.
- **Un fallo real del propio test al primer intento**: asumí que el historial se arrastraba; `setValue` no toca `history()` (solo `m_value`), así que la afirmación correcta es "valor -10 y historial vacío". El test quedó arreglado y es más exacto al diseño.
- `MotorGraphico/CMakeLists.txt` — una línea aditiva más (`add_motor_unit_test(test_carry_over ...)`). CMake reconfigurado (0 errores) y target construido y verificado.

### Cinquena passada (mateix dia): EnemyBrain pur, el test que mai va existir

- `MotorGraphico/tests/Game/EnemyBrainTests.cpp` — **nou, 14/14 PASS** (compilat/linkat de forma autònoma; CTest `test_enemy_brain` registrat a CMakeLists però SENSE generar ni executar). Cobreix el branc que el bridge de D-08 no toca: la patrulla per rang (rang degenerat patrolMin.x==patrolMax.x → kIdle, inclòs invertit), el rebot als extrems AL MATEIX tic (sense tic de pausa, com exigeix el comentari del header), la tornada a l'origen, l'acumulació de temps de pas, l'obstacle (inverteix sentit i gasta el tic, no empeny), tots els clamps defensius del constructor (vida mínima 1, interval mínim 0.01), el combat ICombatant (dany/curació/parts), les defenses en llesca legacy sense sheet (ca = 10+DES+equip), els clamps de populate/tier/stat/health, la font única amb `CharacterSheet` (recompute_derived, knownSkillIds, passiveIds) i la sincronització de `set_character_sheet` amb el caché precomputat.
- **Dos descobriments reals** que el test va treure a la llum:
  1. **Deriva de float a l'acumulació de patrulla**: 0.4f+0.1f NO és 0.5f exacte → el pas no es fa fins que s'acumula prou (observat empíricament a /tmp/probe.cpp). No és un bug de gameplay, però cap test ho havia dit. El test usa valors binari-exactes (0.25/0.5/1.25) perquè la invitació a 0.4+0.1 el faria aleatori.
  2. **Amb sheet adjunt però sense `recompute_derived`, `maxHealth()` llegeix el CACHE del sheet (0)**, NO el m_maxHealth: és el contracte declarat al CharacterSheet ("si no se ha llamado: defaults 0s"). El bus real (GameSession/Catàlegs) sempre passa TierRules; el test fixa que un 'arranjament' silenciós d'aquest cas truqui l'atenció.
- **BLOQUEJAT (parcial)**: la reconfiguració global de CMake falla per treball PARAL·LEL A PUNT D'ACABAR: `CMakeLists.txt:976` fa `include(cmake/CatalogValidationTests.cmake)` i aquest target (`catalog_content_validation`) encara no existeix (`add_dependencies` error). No ho toco (regla 2). El meu bloc aditiu `test_enemy_brain` JA és a CMakeLists: quan l'altra sessió tanqui el seu include, la regeneració l'agafarà sol.

### Sisena passada (mateix dia): la resolució Nd6, el cor del combat

- `MotorGraphico/tests/Game/SkillTests.cpp` — **nou, 13/13 PASS** (standalone enllaçant contra `build/libmotor_level.a` fresc de 10:16). Cobreix el nucli que BattleStateTests mai prement directament: el grau BOTCH/PARTIAL/SUCCESS/CRITICAL i les magnituds reals de `ApplyBasicAttackNd6` (6→crític+×1.5 all-sixes=22, 5→èxit=10, 4→parcial=5), la pifia amb dos 1s sense 5/6 (BOTCH=0), defensa que guanya l'enfrontada (BOTCH), la curació sense rival i les seves magnituds per grau (10/5/22 amb clamp al màxim), la condició pendent que només viatja si el grau hi arriba (minDegree clampada i rondes 0→1), power negatiu clampat a 0, i el SkillSet (aprendre/ordenar/canUse/spend/restaura/recortex) + SkillCatalog (add/has/find/size/nullptr). Els valors provenen de les regles exactes de `DicePoolEngine` i `SkillExecutor`, no de la semilla: ScriptedRng escriu la cara.
- **Bug propi del test (interessant)**: el primer fixture arrencava el combatent a vida plena, així que la curació clampava i `hpChange` sortia 0 amb el motor correcte. Nou fixture amb (`maxHP, hpActual, stats`).
- `MotorGraphico/CMakeLists.txt` — registrat `add_motor_unit_test(test_skill ...)` amb `examples/` (ScriptedRng), mateix patró que test_battle_state. Link: motor_level motor_rpg motor_json.
- **BLOQUEJAT (transitori, altre agent)**: `include/RPG/Catalogs/RpgCatalogs.h` (modificat 10:17, una sessió D-06 refactoritzant ARA MATEIX) duplica especialitzacions `from_json`/`expected_fields` i el compilador dóna "redefinition of". Torna a no compilar el build complet. No ho toco (regla 2). El meu target ja és a CMakeLists; quan tanquin el refactor, CMake el construirà i CTest l'executarà. Verificat standalone: 13/13.

## Validació (sisena passada)

- `test_skill` 13/13 PASS standalone. No executat a CTest pel refactor transitori d'un altre agent sobre RpgCatalogs.h.
- Abans del refactor (10:16, build INF-06): les 6 suites C-24 (incloent `test_enemy_brain`) construïdes i **6/6 verdes a CTest**.

## Validació (cinquena passada)

- Las 5 suites de C-24 verdes en CTest: `test_business_economy`, `test_morality` (19/19), `test_business_mode` (16/16), `test_game_mode` (14/14), `test_carry_over` (7/7). Nada regresado por los añadidos.

## Validación (tercera pasada)

- Los 6 binarios de C-24 verdes en CTest: `test_business_economy`, `test_morality` (19/19), `test_business_mode` (16/16), `test_game_mode` (14/14), `test_game_session`, `test_battle_state`. CTest global: 72/74 (los 2 rotos son ajenos, ver abajo).

## Validación (segunda pasada)

- `test_morality` 19/19, `test_business_economy`, `test_game_session`, `test_save_game`, `test_world`, `test_battle_state` y `test_business_mode` **16/16**: los 8 verdes en CTest (60 = test_business_mode). CTest global 72/74·green.
- Los 2 fallos de CTest NO son míos ni los toco (regla 2): `demo_ciudad` (smoke GL-free que cae en `caminarHasta` — binario del build frente a los cambios de sesión de hoy de GameSession/World sin commitear, zona ajena) y `test_event_layer_document` (registrado por otra sesión pero nunca construido: binario inexistente).
- En la tercera pasada el CTest global pasó a 73/76: se sumó un tercer fallo ajeno, `test_character_creation` (4/4 en `result.isOk()`, binario construido a las 13:19 por otra sesión) — es el trabajo D-06 en vuelo de un agente (tests/RPG/CharacterCreationTests.cpp + src/RPG/CharacterCreation.cpp, zona CharacterCreation/CharacterSheet/RpgCatalogs, sin fila reclamada). No tocado (regla 2); documentado.
- La puerta "el árbol compila" volvió a abrirse el 11/09 con los commits de D-04 (`795f8f6`) y C-14 (`05edf18`); el ABI-mismatch de los objetos scratch de ayer queda obsoleto — ahora se enlaza contra `build/libmotor_*.a` frescos.

## Notas (ABI y alcance)

- Los objetos scratch de `GameSession` compilados el 10/09 no casan con el `LevelDefinition` actual (`mapDocumentPath` añadido por un agente en paralelo): pasar LevelDefinition por valor a ese `.o` viejo corrompe memoria (`~basic_string` en 0xffffffffffffffff). Cualquier test con sesión real solo se valida en build completo del corte B, no contra objetos viejos. **Superado**: verificado contra `build/libmotor_level.a` fresco en esta pasada.
- La banda restante del C-24 según `seguimiento/C-24.json`: la xarxa GL-free de Application (refreshHud, loadLevel, guardado, director de campaña) — zona reservada por codex en C-23, no piseada.

## Sin commitear a propósito

Mismo criterio que C-17: sin luz verde explícita del usuario no hay commit; además `Application.*` sigue en zona de codex+agentes, `CMakeLists.txt` tiene hunks concurrentes y el árbol no compila por WIP ajeno (regla 2).

Trabajo verificado de la sesión: fila en el TABLERO + este fichero.