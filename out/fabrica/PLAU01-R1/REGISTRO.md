# REGISTRO — tanda PLAU01-R1 (lote U1-02.R1, E01–E04)

Encargo: `orquestador/plau01/LOTE_U1-02_R1.md` (aprobado por Oriol 29-10 19:10). Skill: `fabrica-multiagente` (versión sincronizada con A1–A7).
Brain: Claude Code (Opus 5.5), sesión iniciada 29-10 desde `~/.codex/worktrees/perfilador-motor/Software`.

## Entorno y conectores (comprobado al empezar)
- Base: `claude/unificacion-28-10` @ `9d52ae82aa9b` (= rama, verificado `git log -1`).
- Checkout de Oriol: `agente/qwen/integracion-av19-28-10` @ `46644af` — **no se toca**.
- Worktree único del lote: `~/onegai-ramas/PLAU01-R1` (las E van encadenadas; ver «Ramas»).
- Codex: `codex-cli 0.165.0`, login ChatGPT activo, review gate **apagado**.
- OpenCode: 1.19.5, review gate **apagado**. Modelos: E02 `opencode/glm-5.3-flash-free`; E04 `opencode/deepseek-v4.1-flash-free`; Qwen análisis `bailian-token-plan/qwen3.7-plus` con `--agent plan` (el piloto usó `bailian-payg`, pago por uso; aquí se evita).
- ccache: **no instalado**. Copiar `out/build` de otro worktree no sirve (CMakeCache con rutas absolutas). Se compila en este worktree **solo** `test_world` y `test_handle_pool` durante el ciclo (A5).

## Línea base (A5)
- Reutilizada: `~/onegai-ramas/PILOTO-29-10-E1/out/fabrica/PIL-E1/linea_base.json` (commit `9d52ae82…`, fase completa, 29-10 17:18) → copiada a `~/onegai-ramas/PLAU01-R1/out/fabrica/PLAU01-R1/linea_base.json`.
- En ella: `ctest_gl_free` código 8 (fallos heredados), `compilar` 0 (363 s). Ver el JSON para el detalle.

## Ramas
Un solo worktree, las E encadenadas. Rama por implementador según el lote (`agente/<implementador>/U1-02-R1`):
- E01 → `agente/claude-sonnet/U1-02-R1`
- E02 → `agente/opencode/U1-02-R1` (creada desde el commit de E01)
- E03 → `agente/claude-sonnet/U1-02-R1` avanzada (fast-forward local) hasta el commit de E02 antes de E03
- E04 → `agente/opencode/U1-02-R1` avanzada hasta el commit de E03 antes de E04

## Contratos de las E

Comunes: base `9d52ae8`; ámbito congelado `tests/World/WorldTests.cpp`, `tests/Core/HandlePoolTests.cpp`, `planificacion/pla_unity/descomposicio/contratos/U1-02_identidad.md` (solo E01). Nada de `src/`/`include/`. Verificación: `cmake --build --preset macos-debug --target test_world test_handle_pool` + `ctest -R "test_world|test_handle_pool"`. Presupuesto 45 min/E. Cada prueba nueva tiene que fallar con su mutación.

### U1-02.E01 · Contrato de vida útil (documento + 1 prueba) — Sonnet
- Supuestos (A2): `World` usa `std::deque<Slot>` (`include/World/World.h:194`) ✔; `HandlePool` usa `std::vector<Slot>` (`include/Core/Memory/HandlePool.h`, `m_slots`) ✔.
- Observación del paso 0: `World` **no usa** `HandlePool` (tiene sus propios slots en deque), aunque el bloque generado de `HandlePool.h` dice «C-18 HandlePool sosté World». Se documenta, no se cambia.
- Entregable: `contratos/U1-02_identidad.md` (tabla por operación con fichero:línea) + prueba `pointer_may_move_but_handle_stays_valid_after_growth`.
- Mutación: `findSlot` sin comprobar generación → la prueba falla.

### U1-02.E02 · Creación diferida — OpenCode GLM-5.3-Flash Free
- Supuestos: `create` reserva y encola (`src/World/World.cpp:69-76`); `fixedUpdate` salta slots sin `entity` (`:148`) ✔.
- Pruebas: `size()` no crece hasta el flush; creada en callback de `fixedUpdate` no recibe update ese tick y sí el siguiente.
- Mutación: `create` publica al momento → falla.

### U1-02.E03 · Baja inmediata — Sonnet
- Supuestos: `destroy` devuelve false si no `matches` (`:79`) o ya `destroyRequested` (`:83`) ✔; alta+baja antes del flush libera el slot sin contar (`:114`, `:129-133`) ✔.
- Pruebas: destroy de handle inválido/por defecto → false; segundo destroy → false y `size()` tras flush correcto; alta y baja antes del flush → nada vivo.
- Mutación: quitar `if (slot.destroyRequested) return false;` → falla la del doble destroy.

### U1-02.E04 · Identidad tras reutilizar slot — OpenCode DeepSeek V4.1 Flash Free
- Supuestos: `erase` incrementa generación y encadena la free list (`HandlePool.h`, `erase`) ✔.
- Pruebas: dos vivos, baja del primero, emplace nuevo: el segundo conserva contenido e identidad; el primero no resuelve a la nueva (contenido y generación); `erase` de un handle ya borrado → false.
- Mutación: `erase` sin `++generation` → falla.

## Bloqueos
(ninguno)

## Tiempos
- 19:12 E01: subagente Sonnet lanzado (cwd lógico ~/onegai-ramas/PLAU01-R1). Compilación base de test_world/test_handle_pool en curso.
- 19:11–19:13 Compilación base (configure + targets test_world/test_handle_pool): 1 min 50 s. `ctest -R "^(test_world|test_handle_pool)$"` @ 9d52ae8 macos-debug: 2/2 OK. (El regex sin anclar del lote también coge test_world_travel_state/test_world_topology, no compilados → «Not Run»; se usa el regex anclado.)
- 19:14 E01 subagente Sonnet terminó (60 s, 59,6 k tokens, 6 herramientas). Contrato 49 líneas + prueba 33 líneas. Brain verificó 20 citas fichero:línea a mano: correctas.
- E01 compilación targets OK; `ctest -R "^(test_world|test_handle_pool)$"` 2/2 OK (`E01_ctest.log`).
- E01 mutación (findSlot sin generación, las 2 versiones): `test_handle_pool` ROJO, 3 fallos incl. `pointer_may_move_but_handle_stays_valid_after_growth:133 !pool.contains(victim)`. Restaurado → 2/2 verde.
  - TRAMPA: tras restaurar con `cp` el binario NO se recompiló (primer intento tras restaurar seguía rojo); hace falta `touch` del fichero restaurado. Aplicado.
- E01 candidata congelada: `9e84394` en `agente/claude-sonnet/U1-02-R1`.
- E01 revisión: Codex adversarial-review (--base 9d52ae8, cwd worktree) y Qwen `bailian-token-plan/qwen3.7-plus --agent plan` job `task-mvtv7zjd-ehom1n` (cwd `~/onegai-ramas/PLAU01-R1-analisis/E01`).
- E01 `fabrica.py check --fase rapida` @ worktree (sin commitear aún, mismo contenido que 9e84394): VERDE, 3 heredados (explorar_aventuras, validar_audio, linea_base_core — este tardó 196 s vs 17 s en la base, máquina cargada por compilación).
- E01 Codex (thread 01a188e4-c39f-73d3-b731-7a5bf43bb794): needs-attention, 2 hallazgos medium:
  - C1 `reserve()` (HandlePool.h:78) también reasigna e invalida T*/T& si crece la capacidad; el contrato solo lo presenta como prevención → CONFIRMADO (lectura: :78 = `m_slots.reserve(capacity)`; semántica estándar de vector::reserve).
  - C2 la cita HandlePool.h:31-33 no justifica el vector interno (recomienda vector<T> como alternativa al pool) → CONFIRMADO (lectura de :31-34).
- 19:17–20:05 Qwen job `task-mvtv7zjd-ehom1n` (sesión ses_e771ecaccffeDNQgSjxm72pzQD) COLGADO 48 min sin salida. Causa observada: `opencode serve` (pid 9554) lanzó un `git … --git-dir ~/.local/share/opencode/snapshot/…` con cwd `/Users/admin` (snapshot de todo el home, 45 min de CPU) porque el directorio de análisis no era repo git. Brain canceló el job (`companion cancel`); el git terminó solo. Matar procesos con `kill` fue DENEGADO por el clasificador de permisos (anotado, no se insistió).
  - TRAMPA: el directorio de análisis de Qwen debe ser un repo git (se hizo `git init` + commit en `~/onegai-ramas/PLAU01-R1-analisis/E01`).
- 20:08 Qwen relanzado: job `task-mvtwyxdg-onr6ip`.
- A4: E01 empezó 19:10; a las 20:08 lleva 58 min (> 45 presupuesto; 1,5× = 67 min a las 20:17). Retraso atribuible al cuelgue de Qwen, no al implementador.
- E01 Qwen `task-mvtwyxdg-onr6ip` (ses_e76f1fe22ffeZCpcbzfvDwSv5S, 52 s): sin errores nuevos; mutación coherente; repite C1 y C2. **Independencia comprometida**: el `codex_review.md` estaba en su directorio de análisis y lo citó. A partir de E02 la salida de Codex se guarda FUERA del directorio de Qwen.
- E01 ronda 1 (Sonnet, 24 s, 65 k tokens acumulados): C1 y C2 corregidos en el contrato; Brain revisó el diff (cita :164 verificada). Solo documento → no afecta a pruebas.
- E01 ACEPTADA: `66d935d` (`agente/claude-sonnet/U1-02-R1`). 20:07. Duración E01 ≈ 57 min (19:10–20:07; cuelgue de Qwen ~48 min en paralelo).
- 20:07 E02 OpenCode `opencode/glm-5.3-flash-free` job `task-mvtx0rmp-zz2xb3` (ses_e76f0aebaffexLbteMpxVypHOP, 42 s, cwd worktree). Diff solo en WorldTests.cpp (+54). Nota menor: variables `first`/`second` sin uso en la prueba 1 (sin warning de compilación).
- E02 Brain: compilación OK; `ctest` anclado 2/2 OK (`E02_ctest.log`, test_world 12/12).
- E02 mutación (create publica al momento): test_world ROJO 4 fallos, incl. las 2 nuevas (`:217 size()==1`, `:247 hijoCalls==0`). Restaurado + touch → 12/12.
- E02 candidata `924853e` en `agente/opencode/U1-02-R1`. Revisión: Codex (salida en `~/onegai-ramas/PLAU01-R1-analisis/codex/`, fuera del dir de Qwen) y Qwen `task-mvtx2gnz-wrfwiy` (dir `PLAU01-R1-analisis/E02`, repo git).
- E02 Codex (thread 01a18910-87ea-71b1-8262-a0a0462e6d62): approve, sin hallazgos. Qwen `task-mvtx2gnz-wrfwiy` (ses_e76ef79ccffenMIQFOzVa8kC19, 1 min 8 s): sin hallazgos.
- E02 ACEPTADA: `924853e` (`agente/opencode/U1-02-R1`). Duración E02 ≈ 3 min de reloj (20:07–20:10).
- E03: `agente/claude-sonnet/U1-02-R1` avanzada (fast-forward, comprobado con merge-base) a `924853e`.
- 20:10 E03 Sonnet (mismo subagente, 19 s, 70,8 k tokens acumulados): +48 líneas en WorldTests.cpp, 3 pruebas. Diff revisado por el Brain: dentro de ámbito.
- E03 Brain: compilación OK; ctest anclado 2/2 OK (test_world 15/15).
- E03 mutación (sin `if (slot.destroyRequested) return false;`): ROJO 1 fallo, `second_destroy_returns_false_and_does_not_duplicate_the_queue:307 !world.destroy(doomed)`. Restaurado → 15/15.
  - TRAMPA (2.ª vez): restaurar + `touch` en el mismo segundo que el build mutado → ninja no recompila (granularidad de mtime). Hace falta `sleep 1` antes del `touch`. En E01/E02 el verde tras restaurar es válido (verificado: E01 recompiló tras touch explícito; E02 mostró 12/12 y 0 fallos).
- E03 candidata `b790ff9` (`agente/claude-sonnet/U1-02-R1`). Revisión: Codex + Qwen `task-mvtx5l5d-n3cok2`.
- E03 Codex (thread 01a18912-c128-7cb2-a685-ef45b3858efb): approve, sin hallazgos. Qwen `task-mvtx5l5d-n3cok2` (ses_e76ed4041ffeDq3bMY4GywL18b, 47 s): sin hallazgos.
- E03 ACEPTADA: `b790ff9` (`agente/claude-sonnet/U1-02-R1`). Duración ≈ 4 min de reloj (20:10–20:14).
- E04: `agente/opencode/U1-02-R1` avanzada (fast-forward) a `b790ff9`.
- 20:12 E04 OpenCode `opencode/deepseek-v4.1-flash-free` job `task-mvtx6xnr-lvpsmj` (ses_e76ec4ae3ffezAl7TL78QrQQ2i, 31 s, cwd worktree). +44 líneas en HandlePoolTests.cpp. Nota: `secondBeforeErase` vs `second` es trivialmente igual (dos copias const); la identidad real la sostiene `contains(secondBeforeErase)` + contenido.
- E04 Brain: compilación OK; ctest anclado 2/2 OK (test_handle_pool 8/8).
- E04 mutación (erase sin ++generation ni bloque gen 0; clear intacto): ROJO 5 fallos, incl. las 2 nuevas (`:156 newcomer.generation != first.generation`, `:177 !pool.erase(only)`). Restaurado (sleep 1 + touch) → 8/8, ctest 2/2.
- E04 candidata `f5cebcb` (`agente/opencode/U1-02-R1`). Revisión: Codex + Qwen `task-mvtx8ae9-t4vnyo`.
- E04 Codex (thread 01a18914-ad70-7001-8621-4b85b608bb31): approve, sin hallazgos. Qwen `task-mvtx8ae9-t4vnyo` (ses_e76eb53f4ffedyTe3fvIQAIi5f, 41 s): sin hallazgos (no detectó la comprobación trivial `secondBeforeErase`; su frase «reused_slot… prueba reuse tras clear» es incorrecta → descartada).
- E04 ACEPTADA: `f5cebcb` (`agente/opencode/U1-02-R1`). Duración ≈ 3 min de reloj (20:12–20:15).
- 20:14 ENTREGA: `fabrica.py entregar` (clon limpio) desde f5cebcb. Aviso A4 dado: ~35 min.
- A6 20:14: procesos vivos de la tanda = `opencode serve --port 4096` (pid 9554, del plugin, desde 19:17) y broker Codex app-server (pids 11227/11234/11235/11530, del plugin). Sin cmake/ctest/git huérfanos. No se matan (son runtimes de plugin; además `kill` fue denegado antes).
- 20:52 ENTREGA VERDE (sin fallos nuevos; ctest_gl_free 15 = los 15 de la base). INFORME.md escrito. FIN de la tanda.
