# Informe — tanda PILOTO-29-10 (fábrica multiagente, modo día)

Base: `claude/unificacion-28-10` @ `9d52ae8`. Registro paso a paso: `out/fabrica/PILOTO-29-10/REGISTRO.md`.
El checkout de Oriol (`Software/`, rama `agente/qwen/integracion-av19-28-10`) no se ha tocado. Sin merge ni push.

## Estado

| E | Estado | Rama · hash | Worktree |
|---|---|---|---|
| PIL-E1 `test_clases_sp` desfasada | **VALIDADA** con ctest dirigido + mutación; `entregar` parado por Oriol | `agente/claude-sonnet/PIL-E1` · `1beb819` | `~/onegai-ramas/PILOTO-29-10-E1` |
| PIL-E2 `TIMEOUT 600` + `lenta` a `test_personajes_capas` | **HECHA** (cambio verificado con metadatos de ctest; prueba no ejecutada) | `agente/opencode/PIL-E2` · `e64da06` | `~/onegai-ramas/PILOTO-29-10-E2` |

**Problema aparte, a investigar:** en este Mac `test_personajes_capas` tarda ~1776 s (Debug) frente a los 117-124 s del
relevo E-L1C. Con `TIMEOUT 600` dará Timeout hasta que se resuelva esa lentitud.

## PIL-E1
**Cambio** (`tests/Game/ClasesSpTests.cpp`, solo prueba): el caso exigía `siempre:velocidad_0 == Desconocido`, pero
`velocidad_0` es `VelocidadCero` desde D-11. Ahora:
- `siempre:no_existe` → Desconocido (la intención original);
- `ventaja_social` → Ignorado y `siempre:ventaja_social` → Desconocido (cubre la guarda `GanchosCarta.cpp:202`);
- `siempre:velocidad_0` → Siempre + rasgo VelocidadCero (positivo).

**Pruebas ejecutadas** (comando y salida en los logs citados):
- Línea base `fabrica.py inicio` sobre 9d52ae8: `test_clases_sp` Failed; 18 fallos heredados
  (`.../PIL-E1/ejecuciones/20261029-171821_base_completa/`).
- `fabrica.py check --fase completa` sobre 52d105f: **VERDE**, `✓ ARREGLADO test_clases_sp`, 0 nuevos, 17 heredados
  (`.../20261029-175417_completa/`).
- Sobre 1beb819: `ctest -R ^test_clases_sp$` 100 % (1/1). Mutación (quitar `&& dentro.tipo != T::Ignorado` en
  `GanchosCarta.cpp:202`): **0 %**, falla en `ClasesSpTests.cpp:205`. Restaurado: 100 %. Lo hizo el subagente y lo repitió
  el Brain por su cuenta.
- `fabrica.py entregar` desde clon limpio de 1beb819: **en marcha** al escribir esto
  (`.../20261029-184032_entrega/`). Su `EVIDENCIA.md`, `HUELLAS.sha256` y `entrega_PIL-E1.zip` quedan en
  `~/onegai-ramas/PILOTO-29-10-E1/out/fabrica/PIL-E1/`. **Si ese veredicto saliera ROJO, E1 pasa a fallida.**

**Revisión estática / hallazgos:**
- Codex (hilo `01a18895-7504-77b3-bf32-a9ef401d4141`) sobre 52d105f: [medium] la guarda de Ignorado quedaba sin cubrir →
  **confirmado** por mutación → corregido en la ronda 1 → la mutación ahora muere. No se relanzó Codex sobre 1beb819: la
  corrección se verificó ejecutando la mutación.
- Qwen (sesión OpenCode `ses_e776a6178ffei2jR6To1qdE73T`): «sin hallazgos». **No vio** el fallo de Codex. Respetó el modo
  solo lectura.

**Pendiente de ejecución gráfica:** nada; es una prueba gl-free.

## PIL-E2
Implementada por OpenCode (`opencode/glm-5.3-flash-free`): `LABELS "unit;gl-free;lenta"` + `TIMEOUT 600` en
`cmake/AparienciaTests.cmake`. Oriol confirmó que el límite de 600 s es el correcto. Verificado con `cmake --preset` y
`ctest --show-only=json-v1`. La prueba no se ejecutó: con la lentitud actual (~30 min en Debug) daría Timeout. Esa
lentitud es un problema aparte.

## Modelos, duración y consumo
| Rol | Modelo | Uso | Consumo |
|---|---|---|---|
| Brain | Claude Opus 5.5 (esta sesión) | elegir, verificar, mutaciones, registro | no disponible |
| Implementador E1 | Claude **Sonnet** (subagente) | 2 rondas, ~70 s en total | ~49,4k tokens (acumulado) |
| Revisor | Codex (codex-cli 0.165.0, cuenta ChatGPT) | 1 revisión adversarial | no disponible en el plugin |
| Analista | Qwen `bailian-payg/qwen3.7-plus` vía OpenCode, `--agent plan` | 1 análisis, 1 min 9 s | no disponible; **es pago por uso** (no Coding Plan) |
| Implementador E2 | OpenCode `opencode/glm-5.3-flash-free` | 1 edición (el proceso del plugin se colgó al terminar) | gratuito; no disponible |

Tiempo de reloj: unas 1 h 30 min, casi todo de máquina: compilar desde cero, 6 min por worktree; cada ctest gl-free,
unos 28 min, de los que 25 son de `test_personajes_capas`.

## Lecciones del piloto
1. `test_personajes_capas` multiplica por 6 el coste de cada fase completa. Mientras no se decida E2, cada línea base y cada
   entrega de la fábrica tarda unos 30 min por culpa de esa sola prueba.
2. Qwen dio un falso «sin hallazgos»; el hallazgo bueno vino de Codex y la confirmación, de la mutación. No usar el
   acuerdo entre modelos como prueba.
3. OpenCode lanzado desde `~` intentaba un snapshot de todo el home (proceso huérfano de 1 h 43 min, parado). Lanzarlo
   siempre desde un directorio pequeño.
4. El plugin de OpenCode puso en `allow` los permisos de `~/.config/opencode/opencode.json` (cambio aceptado por la skill).

## Qué queda para Oriol en el Mac
- Revisar `git show agente/claude-sonnet/PIL-E1` y juntarla si le parece.
- Leer el veredicto de `entregar` en `~/onegai-ramas/PILOTO-29-10-E1/out/fabrica/PIL-E1/EVIDENCIA.md`.
- Investigar por qué `test_personajes_capas` tarda ~30 min aquí.
- Los worktrees siguen montados para la revisión. Cuando ya no hagan falta:
  `herramientas/git-safe.sh worktree remove ~/onegai-ramas/PILOTO-29-10-E{1,2}`.
