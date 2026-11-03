# Informe — tanda UNI-02-11 (fábrica multiagente, modo día, 02/11/2026)

Registro paso a paso: `out/fabrica/UNI-02-11/REGISTRO.md`. Zonas dudosas para Fable: `out/fabrica/UNI-02-11/ZONAS_PARA_FABLE.md`.
Checkout de Oriol (`Software/`, `recuperat/software-copia-30-10`) **no tocado** salvo esta carpeta. Sin merge ni push.

**Rama:** `agente/claude/UNI-02-11` · worktree `~/onegai-ramas/UNI-02-11` · punta `7b7e3ee5` (+ el commit de sesión).
**Base:** `caa7565a` (`agente/claude/PLAU-U3-E01-E04`, linaje nube: prueba H/I + U3 E01–E04). La rama de integración `claude/unificacion-28-10` se perdió con el `.git` el 29→30/10.

## Estado
| Trabajo | Estado | Commits |
|---|---|---|
| UNI-01 · unificación de lo recuperado | **VALIDADA** (0 fallos nuevos frente a la base; 1 arreglado) | `caa7565a..f39feaa2` |
| UNI-01 r1 · `tweenEscalar` desbordaba (hallazgo de Codex) | **VALIDADA** con mutación | `fa3743f6` |
| U3-06.E07 r1 · verificación de animación | **VALIDADA** con mutaciones | `bc874299` |
| U4-07.E08 r1 · límites y repetibilidad de partículas | **VALIDADA** con mutaciones | `7b7e3ee5` |

## Qué contiene la rama unificada
- Linaje nube completo (G-dia1, G-creador, H-ciudad, prueba H/I, C2/C3 de w2-master, U3-02/05/07 E01–E04 + REV-U3): ya estaba en la base, comprobado fichero a fichero.
- **BND-04** (reputación en `requires`), **U1-02.E01–E04** (pruebas HandlePool/World), **PIL-E1/E2** (piloto 29-10).
- **REPLICA-R1 entero**, commit a commit con `cherry-pick -x`: U3-07 (tweens), U4-01 (cámara virtual), U4-07 (partículas y polvo de pasos), U3-05 (clips), U3-06 (máquina de animación de PNJ y enemigos), U3-02 (triggers de región).
- 8 atlas `editor_*.png` que `level_editor` carga y faltaban.
- 5 conflictos resueltos a mano (detalle y criterio en el registro): tweens (gana nube + pruebas de las dos), CursorClip (gana REPLICA + pruebas de las dos), `ApplicationBucle.cpp` ×2 (cámara U4-01 + polvo U4-07), `CMakeLists.txt` (unión).

## Evidencias
### Pruebas ejecutadas (`agente/claude/UNI-02-11` + `macos-debug`)
- `a4d0b6d9`: compilación completa 696 TU en 67 s (`juego`, `level_editor`, `motor_editor`). `ctest -L gl-free -LE lenta`: **274/287**.
- `caa7565a` (base, worktree temporal): las 13 que fallan en la rama fallan igual en la base, más `test_clases_sp` que la rama arregla. Las 13 heredadas: test_pergamino, test_ruta_casillas, test_botin_equipo, test_recompensas_economia, test_validar_monstruos_acciones, test_lugares, demo_creacion_personaje, demo_boundington, demo_campana_boundington, test_catalog_content, test_kit_basico, linea_base_core_configura, linea_base_core (son las de los relevos E-L1A/B/C).
- `fa3743f6`: `ctest -R "interpolacion|tween|camara"` 8/8. Mutación (fórmula vieja) → `test_interpolacion` ROJO; restaurada → verde.
- `bc874299`: `ctest -R "verificacion_animacion|maquina|animacion|clip|idle_cuerpo_base"` 13/13; 8 mutaciones ROJO; el Brain repitió la doble emisión en `ClipActor.cpp` → ROJO en los dos casos de eventos.
- `7b7e3ee5`: `ctest -R "emisor|particula|polvo|destello"` 7/7 (`test_emisor_carga` de 6 a 10 casos); 16 mutaciones ROJO; el Brain repitió `reserva()` con `maximo-1` → ROJO en `presupuesto_global_el_nacimiento_2048…`.
- **Suite final** sobre `7b7e3ee5`, gl-free con las `lenta` (`ctest -L gl-free -j 6`): **275/288** en 133 s; solo fallan las 13 heredadas. `test_personajes_capas` **pasa en 23,7 s**. El 29-10 tardaba ~1776 s en `9d52ae8`; ese dato es de otro commit, así que solo es referencia: la mejora no se atribuye a ningún cambio concreto (A1), aunque el linaje nube trae PERF-1…4 (`.filas`) (`ctest_final.log`).

### Revisión estática
- Codex sobre la unificación (`a4d0b6d9`): 4 de 5 puntos sin hallazgos. 1 medio: `tweenEscalar` desbordaba en un intermedio, heredado de nube. **Confirmado** compilando con `-ffp-contract=off` (1.7e308 en lugar de 8.5e307: en Windows/MSVC el error se ve; en el Mac lo tapa la FMA). Corregido en `fa3743f6`, y `Interpolacion.cpp` se compila sin FMA para dar el mismo resultado en las dos plataformas.
- Codex sobre U4-07.E08 (`ba50c70c`): 7 hallazgos, 6 confirmados leyendo el código y 1 plausible. Los 7 resueltos en `7b7e3ee5`, cada comprobación nueva con su mutación.
- U3-06.E07: los hallazgos de Opus-adv y Haiku del 31/10, todos resueltos.

### Pendiente de ejecución gráfica (jugar en el Mac)
- `./out/build/macos-debug/juego` de esta rama: cámara (U4-01, visto D-O5), animación de PNJ (U3-06, visto D-O4; lista reproducible en `out/fabrica/REPLICA-R1/U3-06.E07_verificacion.md` del worktree), polvo de pasos (U4-07). **El idle del cuerpo base no se puede ver en el juego actual** (ningún enemigo de nivel cae a él), solo en la prueba.
- `level_editor` con los atlas restaurados.
- U4-07: coste real en GPU sin medir.

## Agentes, tiempos y consumo
| Rol | Modelo | Uso | Consumo |
|---|---|---|---|
| Brain | Claude Opus 5.5 | inventario, cherry-picks y conflictos, verificación, mutaciones repetidas, registro | no disponible |
| Zonas para Fable | subagente Sonnet (solo lectura) | 11 min | ~210k tokens |
| Implementador | subagente Sonnet ×3 | tween r1 (1,5 min), E07 r1 (9 min), E08 r1 (6 min) | ~70k + ~176k + ~143k tokens |
| Revisor | Codex (codex-cli 0.165.0, `codex exec -s read-only`) | unificación (~4 min), E08 (~8 min) | no disponible |
| Analista | Qwen | **BLOQUEADO**: «Invalid API-key» en `bailian-token-plan`; Oriol dijo que Codex cubriera ese papel | — |

Reloj: 20:45–21:50 aprox. Cuelgue: Codex esperando stdin a las 21:09 (detectado a los 2 min, relanzado con `</dev/null`).

## Desviaciones del protocolo
- Las E de la fase 2 van apiladas en la misma rama, sin una rama por E: no hay disco (7 GB libres; cada worktree con build ocupa ~3 GB).
- No se ejecutó `fabrica.py entregar` desde clon limpio: el encargo de la fábrica no estaba iniciado y la línea base costaba 30 min. En su lugar se compararon las pruebas que fallan contra la base `caa7565a`.
- Dos restauraciones de mutación con `git checkout -- <fichero>` dentro del worktree, sin `git-safe.sh` (no afectan a ninguna rama).
- No se ha tocado `TABLERO.md` del checkout de Oriol. La nota de sesión va en la rama (`sesiones/2026-11-02-claude-uni-02-11.md`).

## Decisiones para Oriol
1. Revisar la rama y, si os parece bien, convertirla en la nueva rama de integración en lugar de `claude/unificacion-28-10`.
2. Hacer los vistos visuales D-O4/D-O5 jugando esta rama.
3. Renovar la clave del Token Plan de Bailian si queréis volver a usar Qwen.
4. Fase de juego (sonido U1-01, misiones U3-01, sprites PixelLab): hace falta aprobar un lote con los ficheros congelados. Para el sonido: hoy no hay **ningún fichero de audio** en `assets/`.

---

# Ampliación — fase 3: pruebas heredadas (02–03/11, «continua» → «Heretades»)

**Punta nueva:** `agente/claude/UNI-02-11` @ `cf991f58` (+ commit de informe). Diagnóstico de partida: `HEREDADAS.md`.

**Resultado:** suite gl-free completa sobre `cf991f58` (`ctest -L gl-free -j 6`, con las `lenta`): **287/288** (antes 275/288). Solo queda en rojo `test_recompensas_economia` (BLOQUEADA a propósito, ver abajo). `ctest_final2.log`.

| Bloque | Pruebas | Estado | Commit |
|---|---|---|---|
| H1 | test_catalog_content, test_botin_equipo, test_kit_basico, test_validar_monstruos_acciones, demo_creacion_personaje, test_ruta_casillas | VALIDADA (pruebas desfasadas / umbral Debug; 0 cambios de código o datos) | `02477277` |
| H3a | linea_base_core_configura | VALIDADA: 7 copias de Finder « 2» fuera, guardadas con diff en `copias_finder/` para Fable | (H3a) |
| H2 + H4 | demo_boundington, demo_campana_boundington | VALIDADA: día 1 sube al cuarto; día 2 = Gorran, antiguo día 2 = día 3; 58/58 escenas | `0f86c34d`, `7fb3368a` |
| **H4b** | (bug de juego) | **VALIDADA: el Ocaso se atascaba en el juego.** `once: true` en 3 beats auto de `boundington_ocaso.json` (`NarrativeEngine.cpp:57` elige el primero que encaja) | `45e66555` |
| H3 | test_lugares, test_pergamino (+ test de carteles) | VALIDADA: lugares.json corregido (validar_lugares 30 → 0 errores); 5 placas de enemigo caben (alto 60 → 50; fichas a y+26) | `6bc14fe3` |
| H3b | linea_base_core | VALIDADA: rebase al 03/11; 53 deudas aceptadas, en `DEUDA_LINEA_BASE.md` | `cf991f58` |
| H5 | test_recompensas_economia | **BLOQUEADA** (Oriol: «otra tanda»): el recorredor ya ejecuta cinemáticas y el día 1 cierra, pero `tools/sintetizar_rutas.py` solo conoce 3 días (no Gorran), ni flags de cinemáticas ni pisar zonas del Ocaso | `dc6fbb48` (parcial) |

**Mutaciones:** cada prueba nueva o cambiada tiene la suya (`H1_mut.log`, `H2_mut.log`, `H3_mut.log`, `H4_mut.log`, `H4b_mut.log`, `H5_mut.log`). El Brain repitió una por bloque: catálogo vaciado → ROJO; alto 60 → `MesaCombateTests.cpp:305` ROJO; `once` en la copia de assets del build → 58/58 (sin él, 19 escenas atascadas).

**Trampas nuevas (han mordido hoy):**
- Tras `git checkout -- f`, el mtime puede no cambiar → binario viejo y falso verde/rojo. Hay que hacer `touch` + build.
- Los assets solo se copian al build con el build completo (`copiar_assets`); `--target X` no los sincroniza.
- `genera.py` (fuera de git, `out/claude_dialogos02/guiones/`) regenera `demo_boundington*.cpp` y **pisaría H2 y H4**.

**Decisiones / cosas para Oriol (fase 3):**
1. Emblemas de Gorran que salieron de `lugares.json`: `EMBLEMAS_GORRAN_PENDIENTES.md`.
2. Captura de la mesa de combate con 5 enemigos (placa compacta de 50 px).
3. Copias « 2» de Finder para Fable: `copias_finder/` (las que más cambian: ApplicationPantallas, CMakeLists, InterfazPergamino).
4. Día 2 de las demos: los 4 «líos» de la aldea de Gorran no se juegan (`gorran_misiones=3` dado por hecho); la charla directa con Gorran (`b2_gorran_hablar`) sin cubrir.
5. Economía a 4 días: tanda aparte (~2 h de herramienta).
6. Revisar el resto de beats `auto` sin `once` ni guarda: es el mismo tipo de bug que el del Ocaso (no se ha auditado).

---

# Ampliación 2 — beats que atascan la historia (03/11, «continua» → «Arreglar beats encallats»)

**Punta:** `agente/claude/UNI-02-11` @ `f5efdfb6`. Suite gl-free completa: **289/290** (2 pruebas nuevas); solo `test_recompensas_economia` (aplazada). `ctest_final3.log`.

| Bloque | Qué | Estado | Commit |
|---|---|---|---|
| Auditoría | 728 beats auto sin `once` en `assets/adventures/` (`AUDITORIA_AUTO.md`, solo lectura). Validada: detecta los 3 del Ocaso si se les quita el `once` | hecha | — |
| B1 | `b3_dia_pasa` (soft-lock si suenan las campanas sin avisar ni esperar: se reabre en cada cierre de diálogo y tapa todo el Ocaso) y `b3_xila_avisa` (se repetía hasta la arenga): `"once": true`, sin textos. NUEVO `tools/validar_beats_auto.py` + 17 pruebas, en CTest y `validar_todo` | VALIDADA (mutación del Brain: sin `once` en `b3_xila_avisa` → ROJO) | `417e8d66` |
| B2 | `b1_sectarios_seguir` (persecución [Sigilo] del día 1, da `bnd_pista_sectarios`) era inalcanzable: `b1_sectarios_despues` iba 166 beats antes con requisitos más débiles. Movido justo antes, sin tocar textos (393 beats idénticos, comprobado). El validador detecta ahora beats tapados (32 pruebas) | VALIDADA | `f5efdfb6` |

**Para Oriol:**
1. Probar jugando: la persecución de los sectarios (día 1, hablar con `sectario_perdido` tras resolverlos) y el Ocaso dejando que suenen las campanas sin avisar.
2. `tools/parche_c_trama.py` ya no se puede ejecutar (falla en `:229`) y volvería a dejar la persecución detrás: no hay que volver a ejecutarlo.
3. Límite del validador: no ve los efectos de las cinemáticas ni de los eventos del mundo.

---

# Ampliación 3 — economía a 4 días (03/11, «Economia a 4 dies»)

**Punta:** `agente/claude/UNI-02-11` @ `89d60223`. **Suite gl-free completa: 290/290** (`ctest_final4.log`).

- `sintetizar_rutas.py` y `recorrer_recompensas.cpp` a 4 días (día 2 = Gorran); las cinemáticas y los eventos del mundo cuentan como proveedores de flags; acciones nuevas `pisar` y `poner`. Rutas regeneradas por el script. Premios sin cadena: 31/41 y 57/74 → 5/48 y 10/84.
- Mutaciones: sin los 23 `pisar` del día 4 → ROJO (repetida por el Brain); sin el día de Gorran → ROJO.
- **Límites (para Oriol):**
  1. `poner` da por hechos, igual que la demo, los 4 líos de Gorran, la subasta y el «Subir a dormir» del día 2. Sus premios no se recorren.
  2. La cobertura que cuenta el script es simbólica (hay una cadena hasta el premio). En la traza real disparan 15 de 40 premios (historia) y 21 de 63 (explora/optimiza). La prueba exige llegar al final, no cubrir todos los premios.
  3. `tools/simular_economia.py` sigue modelado a 3 días; con datos reales descartaría el día 4.
