# Registro de la tanda PILOTO-29-10 (fábrica multiagente, modo día)

Brain: Claude Code (Opus 5.5). Máximo 2 E. Base: `claude/unificacion-28-10` @ `9d52ae82aa9baf5af3b6f63f24a0c491945ce948`.
Checkout de Oriol (`Software/`, rama `agente/qwen/integracion-av19-28-10`): no se toca.

## Conexiones comprobadas (29-10 ~17:20)
- `codex@openai-codex` 1.0.6 activo; codex-cli 0.165.0, sesión ChatGPT iniciada; review gate desactivado.
- `opencode@tasict-opencode-plugin-cc` 1.1.0 activo; opencode 1.19.5; review gate desactivado.
- Modelos (`opencode models`): implementador E2 = `opencode/glm-5.3-flash-free`. Qwen (análisis) = `bailian-payg/qwen3.7-plus`
  con `--agent plan` (agente de solo lectura del plugin: `opencode-companion.mjs` usa `plan` para revisiones).
  Nota: la clave de Oriol es de **pago por uso** (workspace `ws-xhtpqv8rv4qd76m6`, Singapur), no del Coding Plan.
- Subagente Sonnet (E1): herramienta Agent con `model: sonnet`.

## Fuentes de las E
`orquestador/front01/relevos/E-L1C.md` (SIGUIENTE ACCIÓN: «las dos pruebas desfasadas se arreglan en una E aparte (dos líneas
de prueba + `TIMEOUT 600` en `AparienciaTests.cmake`)»). Descartada `test_kit_basico`: duda abierta para Oriol (¿sigue vigente
AV16-02?). Descartada `test_catalog_content` (E-L1B): duda abierta (¿actualizar 16→20 o revisar el contenido?).

---

## PIL-E1 — `test_clases_sp` desfasada (implementador: subagente Sonnet)
- **Problema**: `tests/Game/ClasesSpTests.cpp:202` exige `leerGancho("siempre:velocidad_0").tipo == T::Desconocido`, pero
  `src/Game/GanchosCarta.cpp:84-85` reconoce `velocidad_0` como `T::VelocidadCero` desde el commit 763e2ad (15-10; D-11,
  `TABLERO.md:65`). La prueba falla en el ctest del Mac (`out/fabrica/RECURSOS-02/ejecuciones/20261029-144157_base_completa`,
  item `test_clases_sp`).
- **Commit base**: 9d52ae8.
- **Esperado**: el caso sigue comprobando que un gancho que el combate no simula da `Desconocido`, pero con un efecto que de
  verdad no existe; `test_clases_sp` pasa.
- **Alcance**: solo `MotorGraphico-main/MotorGraphico/tests/Game/ClasesSpTests.cpp`. Prohibido tocar `src/`, `include/`, assets.
- **Aceptación**: (a) `ctest -R ^test_clases_sp$` pasa; (b) el resto de casos del fichero intactos; (c) la línea nueva usa un
  efecto que `leerGancho` no reconoce (comprobable leyendo `GanchosCarta.cpp`); (d) ningún fallo nuevo en la fase completa.
- **Verificación**: `fabrica.py --raiz <wt> inicio PIL-E1` (línea base), `check --fase completa`, `entregar`.

## PIL-E2 — `test_personajes_capas` sin TIMEOUT propio (implementador: OpenCode, glm-5.3-flash-free)
- **Problema**: `cmake/AparienciaTests.cmake:26-29` registra `test_personajes_capas` sin `TIMEOUT`; tarda 117-124 s en el Mac y
  dio Timeout en la nube de la fábrica de noche (relevo E-L1C).
- **Commit base**: 9d52ae8.
- **Esperado**: `test_personajes_capas` con `TIMEOUT 600` y la etiqueta `lenta` añadida a las existentes (`unit;gl-free`), sin
  quitarle casos.
- **Alcance**: solo `MotorGraphico-main/MotorGraphico/cmake/AparienciaTests.cmake`, solo el bloque de `test_personajes_capas`.
- **Aceptación**: (a) `ctest --test-dir out/build/macos-debug -R ^test_personajes_capas$ --show-only=json-v1` muestra
  `TIMEOUT 600` y labels `unit;gl-free;lenta`; (b) la prueba sigue en `-L gl-free`; (c) pasa; (d) sin fallos nuevos.
- **Verificación**: igual que E1.

---

## Bitácora de pasos
- 17:2x Worktrees creados: `~/onegai-ramas/PILOTO-29-10-E1` (rama `agente/claude-sonnet/PIL-E1`) y
  `~/onegai-ramas/PILOTO-29-10-E2` (rama `agente/opencode/PIL-E2`), ambos en 9d52ae8.
- 17:18-17:46 Líneas base completas (`fabrica.py --raiz <wt> inicio PIL-E1|PIL-E2`), en paralelo: idénticas. 18 fallos
  heredados: validadores `explorar_aventuras`, `validar_audio`, `linea_base_core`; ctest 15/241 (incluye `test_clases_sp`
  Failed 0.7 s y `test_personajes_capas` **Timeout 1505 s**). compilar 363 s; ctest 1673 s. Registros en
  `<wt>/out/fabrica/PIL-E?/`.
- **Hallazgo que cuestiona PIL-E2**: el relevo E-L1C decía 117-124 s en el Mac; aquí, con dos ctest en paralelo y build
  debug recién hecho, superó los 1500 s por defecto. Con `TIMEOUT 600` fallaría antes. Antes de encargar E2: medición
  aislada (`ctest -R ^test_personajes_capas$ --timeout 1800`, en el worktree E2, en segundo plano).
- 17:4x PIL-E1 lanzada al subagente Sonnet (Agent, `model: sonnet`), cwd = worktree E1.
- 17:50 Subagente Sonnet devuelve PIL-E1 (37 s, 6 herramientas, ~44,5k tokens): 1 línea en `ClasesSpTests.cpp:202`,
  `siempre:velocidad_0` → `siempre:no_existe`. Brain lo comprueba: diff de 1 fichero; `GanchosCarta.cpp:197-203`
  (rama `siempre`: si el interior no es rasgo → Desconocido); `no_existe` no aparece en el fichero; `ctest -R
  ^test_clases_sp$` pasa (0,30 s).
- 17:52 Candidata congelada: `52d105f` (rama `agente/claude-sonnet/PIL-E1`, trailer `Fabrica-E: PIL-E1`).
- 17:54 En paralelo sobre `52d105f`: `fabrica.py check --fase completa`; Codex `adversarial-review --base 9d52ae8`
  (cwd worktree E1); Qwen `bailian-payg/qwen3.7-plus` `--agent plan` desde
  `scratchpad/analisis-PIL-E1` (encargo, diff, extractos, logs).
- 18:0x Codex (hilo `01a18895-7504-77b3-bf32-a9ef401d4141`) sobre 52d105f: needs-attention, 1 hallazgo [medium]:
  `siempre:no_existe` no ejercita la guarda `&& dentro.tipo != T::Ignorado` (`GanchosCarta.cpp:202`); quitarla dejaría
  el caso verde y `siempre:ventaja_social` pasaría a Siempre. Propone añadir `siempre:ventaja_social == Desconocido` y un
  positivo `siempre:velocidad_0 → Siempre + rasgo VelocidadCero`.
  Brain, verificación estática: `esRasgo()` = `DesventajaTodo..Ignorado` (`:160`), así que un Ignorado ES rasgo y solo
  la guarda lo frena; `ventaja_social` está en `ignoradoEnCombate` (`:50-55`); ningún otro test usa `siempre:` con un
  ignorado (`grep` en tests/Game). → **plausible, pendiente de demostrar por mutación** cuando acabe el check completo
  (no se recompila en el worktree mientras corre ctest sobre el hash congelado).
- 18:2x `fabrica check --fase completa` sobre 52d105f: **VERDE**. `✓ ARREGLADO ctest_gl_free · test_clases_sp`; 17
  heredados; compilar 20 s; ctest 1582 s (`test_personajes_capas` Timeout 1531 s, heredado). Registro:
  `~/onegai-ramas/PILOTO-29-10-E1/out/fabrica/PIL-E1/ejecuciones/20261029-175417_completa/`.
- 18:2x Mutación (Brain): quitar `&& dentro.tipo != T::Ignorado` en `GanchosCarta.cpp:202` → `test_clases_sp` **sigue
  pasando** (1/1). Hallazgo de Codex **CONFIRMADO**. Mutación deshecha (`git checkout --` del fichero; nota: restauración
  local con git pelado, no por git-safe) y binario recompilado; worktree limpio salvo `out/`.
- 18:2x Medición aislada en worktree E2 (build macos-debug de 9d52ae8): `test_personajes_capas` **Passed en 1775,7 s**
  (real 1775,8 s; user 1712 s, sys 23 s → limitado por CPU propia, no por espera; coincidió con el check de E1).
  **PIL-E2 → BLOQUEADA, sin implementar**: `TIMEOUT 600` convertiría una prueba que pasa en un Timeout seguro en este
  Mac (Debug). El 117-124 s del relevo E-L1C no se reproduce aquí (¿otro build/preset? sin verificar). El propio relevo
  deja la salida a Oriol («si pasa de 600 s, decidir si va a una fase aparte»). No se encarga a OpenCode: ampliar el
  alcance para inventar otro valor sería decidir por Oriol.
- 18:3x Qwen (`bailian-payg/qwen3.7-plus`, `--agent plan`, sesión OpenCode `ses_e776a6178ffei2jR6To1qdE73T`, job
  `task-mvts9suf-boaxrv`, 1 min 9 s): «sin hallazgos»; **no detectó** la guarda sin cubrir. Respetó solo lectura (el
  directorio de análisis no cambió). Consumo: no disponible en el plugin.
- 18:3x Consolidación Brain: Codex [medium] CONFIRMADO (mutación); Qwen sin hallazgos → no aporta. El acuerdo «parcial»
  no cuenta como prueba.
- 18:3x Ronda 1 (de 2 máx.) al mismo subagente Sonnet (~49,4k tokens acumulados, 33 s): +7 líneas en
  `ClasesSpTests.cpp` (`ventaja_social` suelto = Ignorado; `siempre:ventaja_social` = Desconocido;
  `siempre:velocidad_0` = Siempre + rasgo VelocidadCero). Brain re-verifica: pasa; con la guarda quitada **falla**
  (`ClasesSpTests.cpp:205`); restaurado pasa; worktree limpio.
- 18:4x Candidata final `1beb819`. `fabrica.py entregar` desde clon limpio en marcha.
- Limpieza: parado un `git add --all` huérfano de OpenCode (PID 52190, 1 h 43 min) que hacía snapshot de `/Users/admin`
  (venía de las pruebas de `opencode run` lanzadas desde `~` antes del piloto). Sin `index.lock` residual.
- Configuración global: el plugin de OpenCode escribió `~/.config/opencode/opencode.json` con `permission` = allow
  (bash, edit, webfetch, external_directory) al arrancar su servidor — cambio conocido y aceptado por la skill.
- 19:1x Oriol para todas las pruebas y confirma que el límit (TIMEOUT 600) està bé. La lentitud de 30 min es un problema
  aparte, a investigar.
- 19:2x PIL-E2 implementada por OpenCode `opencode/glm-5.3-flash-free` (`task --write`, cwd worktree E2). El cambio
  quedó escrito, pero el proceso del plugin no volvía; se paró a los ~7 min. Verificación (segundos):
  `cmake --preset macos-debug` OK; `ctest --show-only=json-v1` → TIMEOUT 600.0, LABELS gl-free/lenta/unit; sigue en
  `-L gl-free`. Commit `e64da06` (rama `agente/opencode/PIL-E2`). No se ejecutó la prueba: con la lentitud actual daría
  Timeout. Sin revisión de Codex ni de Qwen, porque Oriol paró las pruebas.
- PIL-E1: `entregar` parado por Oriol. Queda `1beb819`, verificado con ctest dirigido y mutación.
