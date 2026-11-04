# ECC (everything-claude-code) instalado en Claude Code, Codex y Qwen Code

- **Fecha:** 2026-11-04
- **Autor:** Claude (Code tab, Fable 5.1)
- **Encargo:** petición directa de Oriol: analizar el plugin `everything-claude-code` que acababa de instalar y dejarlo usable también desde Qwen y Codex.
- **Commits:** ninguno. Esta sesión no toca código ni ramas; solo configuración global de las tres herramientas (fuera del repo) y este registro.
- **Deja esperando a Oriol:** nada bloqueante. Decidió «paquete entero disponible, pero cada proyecto carga solo lo útil»; aplicado (ver «Cierre»). Queda a su criterio commitear `.claude/` del workspace.

## Qué había

Oriol había instalado `everything-claude-code@everything-claude-code`, el **fork `worldflowai`** (commit `432485b`): una foto antigua del proyecto con 9 agentes, 11 skills y 15 comandos, orientada a TypeScript/Next.js. Sus hooks de bloqueo (ficheros `.md`, `npm run dev` fuera de tmux) usaban una sintaxis de `matcher` que esta versión de Claude Code no evalúa: comprobado creando un `.md` con la herramienta Write sin que saltara nada. Solo corrían SessionStart/SessionEnd/PreCompact/Stop (escriben en `~/.claude/sessions/`).

El repositorio original es ahora `affaan-m/ECC` v2.2.3 (2026-10-29): 71 agentes, 314 skills, 98 comandos, 122 reglas, plugin nativo para Codex, adaptador para Qwen, agentes `cpp-reviewer`/`cpp-build-resolver` y reglas de C++ y Python. El fork no aparece en el upstream y no recibirá actualizaciones.

## Qué se ha hecho (decisión de Oriol: «upstream als tres»)

| Herramienta | Acción | Resultado |
|---|---|---|
| Claude Code | `claude plugin uninstall` del fork, `marketplace remove`, `marketplace add https://github.com/affaan-m/ECC`, `plugin install ecc@ecc` | `ecc@ecc` 2.2.3 habilitado (scope user). Marketplace `ecc` declarado en `~/.claude/settings.json`. |
| Codex CLI 0.165 | `codex plugin marketplace add affaan-m/ECC` + `codex plugin add ecc@ecc` | Instalado y habilitado en `~/.codex/plugins/cache/ecc/ecc/2.2.3`; `scripts/codex/check-plugin-cache.js` → todas las referencias resuelven. Un solo hook (SessionStart) que exige confianza explícita en `/hooks` dentro de Codex. |
| Qwen Code 0.21.12 | `qwen extensions install https://github.com/affaan-m/ECC:ecc --consent` | Extensión `ecc` 2.2.3 en `~/.qwen/extensions/ecc`: 68 agentes, 293 skills, 94 comandos. Los hooks no se activan por esta vía. La guía del upstream avisa de 11 warnings de reglas sin `paths:`; es comportamiento esperado de Qwen. |

Clon de referencia del upstream (temporal, fuera del repo): scratchpad de la sesión de Claude. No se ha copiado nada dentro de `Software/`.

## Hallazgos que Oriol debe conocer

1. **Coste de contexto en Claude.** `claude plugin details ecc@ecc` proyecta **~48.079 tokens fijos por sesión** (412 entradas de skill a ~100 tokens cada una). El fork costaba ~1.700. Es el precio de tener las 314 skills siempre descritas en el prompt.
2. **Hook GateGuard.** En el perfil `standard` (por defecto) el PreToolUse `gateguard-fact-force` **deniega el primer Bash de cada sesión** hasta que el modelo «presenta los hechos» y reintenta. Probado con `herramientas/git-safe.sh commit`: denegado. Interactivamente es un estorbo; para `claude -p` (fábrica de noche) puede romper la tanda. Se apaga con `ECC_GATEGUARD=off` o `ECC_DISABLED_HOOKS=pre:bash:gateguard-fact-force`.
3. **Otros hooks probados contra el protocolo del repo:** `doc-file-warning` deja pasar `sesiones/*.md`; `config-protection` deja editar `TABLERO.md`; `git push` no se bloquea. Ninguno choca con la bitácora.
4. **Dependencias npm no instaladas** en la caché del plugin de Claude (su `package.json` usa `overrides`). Los 25 scripts de hook referenciados no las necesitan; sí las usan `scripts/lib/project-detect.js` y los perfiles de contexto (`js-yaml`, `ajv`, `@iarna/toml`). `session-start-bootstrap` ha corrido bien sin ellas.
5. **Reglas de ECC vs. bitácora.** Las reglas comunes del upstream piden commits convencionales en inglés (`feat:`), TDD con 80% de cobertura e inmutabilidad; la bitácora manda commits en castellano, `require()` en demos y el flujo canon → cartas → motor. Como plugin, las reglas no se cargan solas; no copiarlas a `~/.claude/rules/` sin adaptarlas.
6. **Qwen en la fábrica.** La fábrica de día llama a Qwen a través de OpenCode, no de Qwen Code; la extensión en `~/.qwen` no llega por ese camino. El upstream tiene un plugin propio para OpenCode (`npm run build:opencode` + `install.sh --target opencode`), no instalado.

## Cierre (decisión de Oriol: «el quiero entero, pero solo lo útil para cada proyecto»)

- **Claude Code:** `claude plugin disable ecc@ecc` (scope user). El plugin, su caché y el marketplace `ecc` siguen instalados para reactivarlos o para sacar más piezas; lo que deja de cargarse son las 314 skills y los 27 hooks (GateGuard incluido).
- **Selección para este workspace en `Software/.claude/`** (copiada a mano desde la caché 2.2.3; el instalador `--target claude-project` arrastraba módulos enteros y ficheros de plugin que no queríamos):
  - skills (10): `cpp-coding-standards`, `python-patterns`, `python-testing`, `verification-loop`, `strategic-compact`, `security-review`, `git-workflow`, `codebase-onboarding`, `error-handling`, `context-budget`.
  - agentes (10): `cpp-reviewer`, `cpp-build-resolver`, `python-reviewer`, `code-reviewer`, `planner`, `architect`, `security-reviewer`, `silent-failure-hunter`, `code-explorer`, `code-simplifier`.
  - comandos (13): `/cpp-review`, `/cpp-build`, `/python-review`, `/code-review`, `/plan`, `/checkpoint`, `/build-fix`, `/learn`, `/save-session`, `/resume-session`, `/test-coverage`, `/update-codemaps`, `/refactor-clean`.
  - reglas en `rules/ecc/`: C++ (estilo, patrones, seguridad, testing) y Python (estilo, patrones, testing), condicionadas por `paths:` a ficheros C++/CMake y `.py`. Adaptadas: `cpp/testing.md` ya no pide GoogleTest sino el patrón real del motor (`add_test` + `require()`, etiquetas `gl-free`/`lenta`); `python/testing.md` respeta los validadores existentes; quitadas las referencias a `common/` (no instalado). `00-prioridad.md` fija que ante conflicto manda la bitácora.
  - Descartados a propósito: `cpp-test`/`cpp-testing` (GoogleTest), `tdd-guide`/`tdd-workflow` (80 % de cobertura), `commit-reviewer`/`conventional-commit-review` (commits en inglés), `refactor-cleaner` (herramientas JS), `continuous-learning*` (necesita hooks), todo lo web, móvil, negocio y dominio.
- **Coste:** la selección añade del orden de 1–2k tokens fijos (descripciones) frente a los ~48k del plugin completo; las reglas C++/Python solo entran al tocar esos ficheros.
- **Codex y Qwen Code** se quedan con el paquete completo (Codex tiene `skill_search`; Qwen trabaja con 1M de contexto).
- `Software/.claude/` queda **sin commitear** (no está en `.gitignore`): Oriol decide si entra en el repo para que lo compartan todas las sesiones de Claude.

## Pendiente

- Dentro de Codex: aceptar o rechazar el hook de ECC en `/hooks`.
- Adaptar `/verify` y `/build-fix` al toolchain real (CMake, `ctest -L gl-free`, `validar_todo.py`) y una skill `project-guidelines` del repo que recuerde la bitácora.
- Valorar el plugin de ECC para OpenCode si se quiere que Qwen lo use desde la fábrica.
