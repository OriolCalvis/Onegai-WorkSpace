# INFORME — tanda PLAU01-R1 (lote U1-02.R1 · identidad y vida útil de las entidades)

Encargo: `orquestador/plau01/LOTE_U1-02_R1.md` (aprobado por Oriol el 29-10 a las 19:10). Skill: `fabrica-multiagente` (A1–A7), modo día.
Brain: Claude Code (Opus 5.5). Registro vivo con todos los pasos e ids: `out/fabrica/PLAU01-R1/REGISTRO.md`.
Intervalo: 19:10–20:52 del 29-10.

## Estado

| E | Estado | Implementador | Commit | Rama | Rondas |
|---|---|---|---|---|---|
| U1-02.E01 · Contrato de vida útil + prueba de crecimiento | **validada** | subagente Sonnet | `9e84394` → `66d935d` | `agente/claude-sonnet/U1-02-R1` | 1 (2 hallazgos de Codex) |
| U1-02.E02 · Creación diferida | **validada** | OpenCode `opencode/glm-5.3-flash-free` | `924853e` | `agente/opencode/U1-02-R1` | 0 |
| U1-02.E03 · Baja inmediata | **validada** | subagente Sonnet | `b790ff9` | `agente/claude-sonnet/U1-02-R1` | 0 |
| U1-02.E04 · Identidad tras reutilizar un slot | **validada** | OpenCode `opencode/deepseek-v4.1-flash-free` | `f5cebcb` | `agente/opencode/U1-02-R1` | 0 |

- Worktree: `~/onegai-ramas/PLAU01-R1`. Cadena: `9d52ae8 → 9e84394 → 66d935d → 924853e → b790ff9 → f5cebcb`.
- Ramas: `agente/opencode/U1-02-R1` apunta a `f5cebcb` (el lote completo) y `agente/claude-sonnet/U1-02-R1` a `b790ff9`. Las dos avanzan solo por fast-forward, comprobado con `merge-base --is-ancestor`. **Sin merge ni push.**
- Ámbito: solo `tests/World/WorldTests.cpp`, `tests/Core/HandlePoolTests.cpp` y `planificacion/pla_unity/descomposicio/contratos/U1-02_identidad.md` (nuevo). `git diff --stat 9d52ae8..f5cebcb`: 3 ficheros. Nada en `src/` ni `include/`.
- Ninguna prueba nueva falló sobre el código actual, así que no hubo hallazgos de caracterización que preguntar (A3).

## Resumen del cambio
- **E01**: el contrato `U1-02_identidad.md` tiene una tabla por operación de `World` (create/get/destroy/eachEntity/fixedUpdate/flushPendingChanges) y de `HandlePool` (emplace/get/erase/clear/reserve). Para cada una dice si siguen siendo válidos el handle, el puntero y la referencia, con fichero:línea de `9d52ae8`.
  - También explica por qué `World` usa `deque` y el pool `vector`. Solo `World.h:192-194` justifica el `deque`. El motivo del `vector` se marca como «inferido, sin respaldo en el código».
  - Las operaciones del pool que reasignan, e invalidan los `T*` y `T&`, son `emplace` (sin slot libre y con el vector lleno) y `reserve`.
  - Prueba nueva: `pointer_may_move_but_handle_stays_valid_after_growth`.
  - Observación sin cambio: `World` **no usa** `HandlePool`, aunque el bloque generado de `HandlePool.h:3` dice «C-18 HandlePool sosté World».
- **E02**: `create_does_not_grow_size_until_flush` y `entity_created_during_fixedUpdate_is_updated_only_on_the_next_tick`.
- **E03**: `destroy_of_invalid_or_default_handle_returns_false`, `second_destroy_returns_false_and_does_not_duplicate_the_queue` y `create_and_destroy_before_flush_leave_nothing_alive`.
- **E04**: `erase_and_reuse_keep_the_other_live_entry_and_do_not_alias_the_old_handle` y `erase_of_an_already_erased_handle_returns_false`.

## Evidencias

### 1. Pruebas ejecutadas (rama@commit, preset `macos-debug`)
- **Línea base** (A5, reutilizada): `~/onegai-ramas/PILOTO-29-10-E1/out/fabrica/PIL-E1/linea_base.json` @ `9d52ae8`, fase completa. `ctest_gl_free` da 15 fallos heredados. Con `ctest -R "^(test_world|test_handle_pool)$"` @ `9d52ae8` pasan 2/2 (`ctest_base.log`).
- **Ciclo** (por E): se compilan los targets `test_world` y `test_handle_pool` y se ejecuta `ctest -R "^(test_world|test_handle_pool)$"`.

| E | Commit | ctest 2/2 | Mutación | Resultado con la mutación |
|---|---|---|---|---|
| E01 | `9e84394` | OK (`E01_ctest.log`) | `findSlot` sin comprobar la generación | ROJO, 3 fallos, incl. `pointer_may_move…:133 !pool.contains(victim)` (`E01_mut_test.log`) |
| E02 | `924853e` | OK, test_world 12/12 | `create` publica al momento | ROJO, 4 fallos, incl. las 2 nuevas: `:217 size()==1` y `:247 hijoCalls==0` |
| E03 | `b790ff9` | OK, test_world 15/15 | sin `if (slot.destroyRequested) return false;` | ROJO, 1 fallo: `second_destroy…:307 !world.destroy(doomed)` |
| E04 | `f5cebcb` | OK, test_handle_pool 8/8 | `erase` sin `++generation` | ROJO, 5 fallos, incl. las 2 nuevas: `:156` y `:177` |

  Todas las mutaciones se deshicieron, con `git diff --stat src/ include/` vacío, y la prueba volvió a verde tras recompilar.
- **Validadores rápidos** (`fabrica.py check --fase rapida`, durante E01): VERDE, con 3 fallos heredados.
- **Entrega única** desde un clon limpio de `f5cebcb` (`fabrica.py entregar`, 20:14–20:51): **VERDE, ningún fallo nuevo.**
  - `compilar`: ok, 349,9 s.
  - `ctest_gl_free`: 226/241 pasan, 1676,9 s. Los 15 fallos son exactamente los de la línea base: test_pergamino, test_ruta_casillas, test_botin_equipo, test_recompensas_economia, test_validar_monstruos_acciones, test_lugares, demo_creacion_personaje, demo_boundington, demo_campana_boundington, test_catalog_content, test_personajes_capas (Timeout, heredado), test_kit_basico, linea_base_core_configura, linea_base_core y test_clases_sp.
  - `test_world` y `test_handle_pool` pasan dentro de la suite.
  - Fallan los validadores `explorar_aventuras`, `validar_audio` y `linea_base_core`, los mismos que en la base.
  - Evidencia: `~/onegai-ramas/PLAU01-R1/out/fabrica/PLAU01-R1/EVIDENCIA.md`, `HUELLAS.sha256` y `entrega_PLAU01-R1.zip`.

### 2. Revisión estática
- El Brain revisó cada diff. En E01 comprobó 20 citas fichero:línea a mano y luego las líneas reescritas en la ronda 1.
- Codex (adversarial-review) revisó el hash congelado de cada E. Qwen solo leyó, desde un directorio de análisis aislado.

### 3. Pendiente de ejecución gráfica
- Nada: el lote es gl-free (pruebas unitarias y un documento).

## Hallazgos de Codex y Qwen

| E | Origen | Hallazgo | Clasificación | Resolución |
|---|---|---|---|---|
| E01 | Codex (medium) | C1: `reserve()` (`HandlePool.h:78`) también reasigna e invalida T*/T&; el contrato lo presentaba solo como prevención | **confirmado** (lectura de :78) | Ronda 1: se añade la fila `reserve` y se corrige la sección. `66d935d` |
| E01 | Codex (medium) | C2: la cita `HandlePool.h:31-33` no justifica el vector interno | **confirmado** (lectura de :31-34) | Ronda 1: se cita `:176`/`:164` y el motivo se marca como inferido. `66d935d` |
| E01 | Qwen | Repite C1 y C2 | **no independiente**: el `codex_review.md` estaba en su directorio y lo citó | Desde E02, la salida de Codex se guarda fuera del directorio de Qwen |
| E02 | Codex / Qwen | Sin hallazgos | — | — |
| E03 | Codex / Qwen | Sin hallazgos | — | — |
| E04 | Codex / Qwen | Sin hallazgos | — | Nota del Brain: en E04, `secondBeforeErase == second` es trivialmente cierto (dos copias const). La identidad real la sostienen `contains(secondBeforeErase)` y el contenido. No bloquea. Qwen afirmó que `reused_slot…` «prueba reuse tras clear»: **descartado**, es falso |
| E02 | Brain | Variables `first`/`second` sin uso en `create_does_not_grow_size_until_flush` | estilo | No se corrige; no genera warning |

## Modelos, duración y consumo

| Agente | Modelo | Uso | Duración | Consumo |
|---|---|---|---|---|
| Implementador A | subagente Claude Sonnet | E01 + ronda 1, E03 | 60 s + 24 s + 19 s | 70,8 k tokens acumulados (subagente) |
| Implementador B | OpenCode `opencode/glm-5.3-flash-free` | E02 | 42 s | no disponible |
| Implementador B | OpenCode `opencode/deepseek-v4.1-flash-free` | E04 | 31 s | no disponible |
| Revisor | Codex (codex-cli 0.165.0, ChatGPT) | 4 adversarial-review. Threads `01a188e4…`, `01a18910…`, `01a18912…`, `01a18914…` | ~1–3 min cada una | no disponible |
| Analista | Qwen `bailian-token-plan/qwen3.7-plus` `--agent plan` vía OpenCode | 4 análisis (+1 colgado) | 52 s, 68 s, 47 s, 41 s | no disponible. Se usó `bailian-token-plan`, no `bailian-payg` (pago por uso) |

### Tiempos por E

| E | Reloj | Presupuesto | Nota |
|---|---|---|---|
| E01 | ≈57 min (19:10–20:07) | 45 min | Pasado el presupuesto pero por debajo de 1,5×. Lo causó el cuelgue de Qwen (48 min, ver Incidencias), no el implementador |
| E02 | ≈3 min | 45 min | |
| E03 | ≈4 min | 45 min | |
| E04 | ≈3 min | 45 min | |
| Entrega | 37 min (20:14–20:51) | aviso A4 de ~35 min | |

Compilación inicial de los dos targets en el worktree: 1 min 50 s.

## Incidencias y trampas nuevas (para la Parte 3 / la skill)
1. **Qwen se colgó 48 min.** El directorio de análisis no era un repo git, y `opencode serve` lanzó un snapshot git de todo `/Users/admin` (pid 11113, 45 min de CPU).
   - Se resolvió con `companion cancel`.
   - Matar procesos con `kill` fue **denegado** por el clasificador de permisos; no se insistió.
   - Arreglo: hacer `git init` y un commit en el directorio de análisis antes de lanzar Qwen. Después, cada análisis tardó menos de 70 s.
   - Propuesta para la skill: añadirlo a A6.
2. **Mutación y mtime.** Si se restaura y se hace `touch` en el mismo segundo que el build mutado, ninja no recompila y el binario sigue mutado. Hace falta `sleep 1` antes del `touch`.
3. **El regex de ctest del lote** (`test_world|test_handle_pool` sin anclar) también coge `test_world_travel_state` y `test_world_topology`, que salen «Not Run» si solo se compilan los dos targets. Se usó `^(test_world|test_handle_pool)$`.
4. **Independencia de Qwen**: no dejar la salida de Codex en su directorio (pasó en E01).
5. **Procesos de la tanda** que siguen vivos (A6): `opencode serve` (pid 9554) y el broker app-server de Codex (pids 11227…), los dos gestionados por sus plugins. No quedaron cmake, ctest ni git huérfanos.
6. La hook `opencode-rescue-auto-monitor` propone un Monitor con `declare -A`, que no funciona en el bash 3.2 de macOS. Se usó `wait-and-result` en segundo plano.

## Queda para Oriol, a mano en el Mac
- Revisar y, si procede, integrar `agente/opencode/U1-02-R1` (`f5cebcb`, contiene E01–E04) en la rama de integración. Ningún agente ha hecho merge.
- Leer `MotorGraphico/planificacion/pla_unity/descomposicio/contratos/U1-02_identidad.md`.
  - Contiene la observación de que `World` no usa `HandlePool` aunque el plan (C-18) lo dice. Puede pesar en U1-02.E05 (estrategia de adaptación), que queda fuera de este lote.
- Nada gráfico que probar.
- No se ha tocado tu checkout (`agente/qwen/integracion-av19-28-10`), salvo `out/fabrica/PLAU01-R1/` (ignorado por git). Tampoco se han creado `sesiones/` ni movido filas de `TABLERO.md`, porque viven en ese checkout. Si quieres esa huella, dímelo.
