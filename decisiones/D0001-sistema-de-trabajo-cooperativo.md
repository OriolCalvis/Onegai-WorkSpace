# D0001 — Sistema de trabajo cooperativo por capas (4 IAs + Oriol)

- **Estado:** `accepted` — aceptada por Oriol
- **Aceptación:** 28/08/2026 — Oriol responde «OK» en sesión. Despliegue (Fase 1) registrado en `sesiones/2026-08-28-zcode-despliegue-sistema-cooperativo.md`.
- **Fecha:** 2026-08-28 · **Autor:** ZCode · **Huella:** BITACORA, Parte 0, entrada 28/08 (ZCode)
- **Supera a:** nada. La Parte 2 de la BITACORA (decisiones 2.1–2.18) queda como registro histórico; las decisiones NUEVAS viven aquí desde que esto se acepte.

---

## 1. Contexto y problema

El workspace ya coordina a varios agentes IA con la BITACORA como único punto de
encuentro, y funciona — 25+ sesiones registradas sin perder trabajo ajeno. Lo que
motiva esta decisión es un requisito nuevo (**que Oriol también trabaje en paralelo**)
y cuatro dolores con evidencia:

| Dolor | Evidencia |
|---|---|
| Contención de la bitácora única (1.130 líneas) | La Parte 0 acumula capas sin commitear de sesiones distintas (17–21/08); dos agentes no pueden escribir bien a la vez |
| Nadie reclama la tarea antes de empezar | Colisión de las dos Plazas de las Razas (16/08): vuestro propio tablero propuso la columna «tomada por / desde» y nunca se implantó |
| Carreras de git y `index.lock` | Commit que «falló en silencio» contra el del agente del editor; locks rancios de 9 h; sandbox que no puede borrar ficheros |
| Las decisiones pendientes de Oriol están dispersas | Parte 4.2, `marcadores_pendientes.json`, avisos `[TBD Oriol]` en varias carpetas — no hay bandeja |

## 2. Opciones consideradas

Resumen (estudio completo en Anexo A, con fuentes):

| Opción | Markdown puro | Exclusión mutua real | Puerta humana | Encaje |
|---|---|---|---|---|
| **Backlog.md** (CLI, MIT, muy activo) | sí | **sí** (claims + hook + worktrees) | 3 gates | Asume flujo «1 tarea = 1 PR» que no usáis; escribe secciones en AGENTS.md que competirían con LA BIBLIA; dependencia externa joven |
| **OpenSpec** | sí | no (convención) | propuesta→aprobación→archivo | Su ciclo es exactamente vuestra filosofía canon, pero es tooling Node para specs de software |
| **GitHub Spec Kit** | sí | no | implícita | Orientado a repos GitHub con issues |
| **BMAD-METHOD** | sí | no | fuerte en planning | Roles PM/arquitecto/SM: verboso y caro en tokens para 4 repos; sobra |
| **Task Master** | **no** (tasks.json) | no | chat | JSON monofichero = conflictos garantizados con 5 escritores |
| **Adoptar patrón propio por capas** | sí | sí (tablero + git-safe) | sí (Estado: proposed) | **Recomendada** |

**Por qué no un framework entero:** el caso es idiosincrático — misma máquina y
misma copia de trabajo, sin PRs ni servidor, repos anidados con builds CMake
pesadas, sandbox que no puede borrar ficheros, y reglas de dominio
(canon → cartas → motor, cultura append-only) ya codificadas y probadas. Lo que
falta no es herramienta: es estructura de ficheros. Los mecanismos se importan de
donde ya resolvieron ese problema (towncrier, MADR, PEP/RFD, flock, obsidian-kanban).

## 3. Decisión propuesta

**Evolucionar el sistema propio ensamblándolo por capas**, cada una con el patrón
probado que resuelve su necesidad. Nada de lo existente se borra ni se reescribe:
la BITACORA se adelgaza a constitución y las partes vivas migran a ficheros nuevos.

```
Software/
├── AGENTS.md                  ← entrada única (estándar agents.md); apunta a todo
├── BITACORA_DEL_PROYECTO.md   ← CONSTITUCIÓN: protocolo + trampas (Parte 3) + canon histórico (Parte 5)
│                                 Parte 0 (sesiones) y Parte 2 (decisiones) se CONGELAN como histórico
├── TABLERO.md                 ← VISTA VIVA: quién está en qué + bandeja «Esperando a Oriol»
├── sesiones/                  ← REGISTRO append-only: 1 fichero por sesión (patrón towncrier)
│   ├── INDICE.md              ←   regenerado por script, no se edita a mano
│   └── 2026-08-28-zcode-<slug>.md
├── decisiones/                ← DECISIONES numeradas D0001+ (patrón MADR)
│   └── D0001-…                ←   Estado: proposed = bandeja de Oriol; accepted = inmutable
└── herramientas/
    ├── git-safe.sh            ← serializa el git de escritura (patrón flock, adaptado al sandbox)
    └── regenerar_indice.py    ← reconstruye sesiones/INDICE.md
```

| Necesidad | La resuelve | Patrón importado de |
|---|---|---|
| Contención de la bitácora | `sesiones/` un fichero por sesión + INDICE regenerado | towncrier / changelog.d |
| Carreras de git | `herramientas/git-safe.sh` (mutex con expiración) | wrappers flock + práctica index.lock |
| Reclamar trabajo | fila en TABLERO escrita ANTES de crear el primer fichero | Backlog.md (claims) + vuestra propia propuesta del 16/08 |
| Bandeja de Oriol | columna «Esperando a Oriol» + `decisiones/` con Estado: proposed | PEP/RFD (un decisor) |
| Decisiones auditables | `decisiones/DNNNN` inmutables tras aceptarse | MADR |
| Visión humana de conjunto | TABLERO.md es kanban markdown que Obsidian renderiza | obsidian-kanban / todomd |

**Deliberadamente fuera:** `locks/` como carpeta separada (el reclamo vinculante es
la fila del TABLERO; el mutex de git es interno de git-safe), worktrees por agente
(fase 3 opcional: cuestan un `build/` propio cada uno), y cualquier runtime/servidor.

## 4. Las piezas (plantillas exactas)

### 4.1 `TABLERO.md`

```markdown
# TABLERO — trabajo cooperativo (4 IAs + Oriol)

<!-- REGLAS (protocolo, reglas 1bis y 4ter):
  1. ANTES de crear el primer fichero de una tarea: escribe tu fila en «En curso»
     con «Tomada por» y «Desde». Sin fila, la tarea está libre.
  2. No toques rutas de una fila ajena viva; las zonas por prefijo (on_/os_/en_/es_,
     bnd_…) siguen mandando igual que en el CONTRATO del experimento.
  3. Al terminar: mueve la fila a «Hecho» enlazando tu fichero de sesiones/.
  4. «Esperando a Oriol» es SU bandeja: sólo él la vacía, aceptando o rechazando
     la decisión enlazada en decisiones/.
  5. Lo editan agentes Y Oriol. Nunca se borra la fila de otro: se comenta.
-->

## En curso
| Tarea | Tomada por | Desde | Zona (rutas / prefijo) | Entrega verificable |
|---|---|---|---|---|

## Pendiente — libre para quien la coja
- [ ] _tarea_ — _entrega verificable_ — _zona_

## Esperando a Oriol  ← bandeja de decisiones
| Asunto | De | Desde | Dónde decide |
|---|---|---|---|

## Hecho — últimas
| Fecha | Autor | Qué | Huella |
|---|---|---|---|
```

En Fase 1 se siembra con lo que hoy está EN CURSO según la Parte 0: F1 (dt activo
de timers, zcode, 19/08), F0 (barrido de etiquetas, Qwen) y la sesión del editor
(`EditorValidation.h` + `demo_editor_state.cpp`).

### 4.2 `sesiones/YYYY-MM-DD-<autor>-<slug>.md`

Nombre de fichero obligatorio: `2026-08-28-zcode-propuesta-d0001.md` (fecha-autor-slug;
el INDICE se construye desde el nombre).

```markdown
# SESIÓN 2026-08-28 — autor — slug corto

- **Dónde:** proyecto(s)/ruta tocada
- **Qué se hizo:** 2–6 líneas (qué, no cómo)
- **Commits:** hash(s), o «(sin commit — motivo)»
- **Verificación:** comando(s) ejecutado(s) + resultado
- **Trabajo ajeno encontrado:** rutas y estado, o «ninguno»
- **Deja esperando a Oriol:** enlace(s) a decisiones/ con Estado: proposed, o «nada»
```

Append-only como la Parte 0: no se editan sesiones ajenas; las propias se pueden
corregir dejando nota visible (precedente: la corrección del 19/08 sobre la entrada
del 17/08).

### 4.3 `decisiones/DNNNN-slug.md`

- Numeración `D0001`, `D0002`… = máximo existente + 1; nunca se renumera; tras
  aceptarse, el fichero es inmutable (una decisión que cambia genera una nueva que
  dice «supera a DNNNN»).
- Si dos agentes crean el mismo número a la vez, el segundo renumera al commitear
  (el INDICE y las referencias se regeneran; git-safe serializa el commit).

```markdown
# DNNNN — título corto
- **Estado:** proposed   ← editable SÓLO por Oriol → accepted / rejected / superseded por DMMMM
- **Fecha:** · **Autor:** · **Decide:** Oriol

## Contexto
## Opciones consideradas
## Decisión
## Consecuencias
```

### 4.4 `herramientas/git-safe.sh`

Envuelve **sólo el git de escritura** (`add`, `commit`, `rm`, `merge`, `rebase`,
`reset`…). Las lecturas (`status`, `log`, `diff`) van en git pelado. Tres reglas de
la Parte 3 hechas código: una escritura a la vez por repo, el lock rancio se
renombra (nunca se borra — el sandbox no puede), y ningún fallo en silencio.

```bash
#!/usr/bin/env bash
# git-safe — serializa el git de ESCRITURA entre agentes (uso: herramientas/git-safe.sh commit -m "…")
set -uo pipefail

STALE=300; REINTENTOS=6; PAUSA=5

GITDIR="$(git rev-parse --git-dir 2>/dev/null)" || { echo "git-safe: no estoy en un repo" >&2; exit 2; }
LOCK="$GITDIR/git-safe.lock"

adquirir() {
  local intento=0
  while true; do
    if mkdir "$LOCK" 2>/dev/null; then
      printf '%s %s %s\n' "$$" "$(date +%s)" "${USER:-agente}" > "$LOCK/info"; return 0
    fi
    local edad=$(( $(date +%s) - $(stat -f %m "$LOCK" 2>/dev/null || echo 0) ))
    if [ "$edad" -gt "$STALE" ]; then mv "$LOCK" "$LOCK.rancio.$$" 2>/dev/null || true; continue; fi
    intento=$((intento+1))
    if [ "$intento" -ge "$REINTENTOS" ]; then
      echo "git-safe: no pude adquirir el lock (${PAUSA}s × $REINTENTOS). ¿Otra sesión commitando?" >&2; exit 3
    fi
    sleep "$PAUSA"
  done
}

adquirir
trap 'rmdir "$LOCK" 2>/dev/null || true' EXIT   # si el sandbox no puede, el lock expira solo (STALE)

git "$@"
rc=$?
[ "$rc" -ne 0 ] && echo "git-safe: git $1 falló (código $rc) — NO sigas como si nada: revisa y reintenta" >&2
exit "$rc"
```

### 4.5 `herramientas/regenerar_indice.py`

Reglas de la casa aplicadas: paths relativos a `__file__` (nunca `/Users/admin`,
BITACORA 2.15) y `ensure_ascii=False` (trampa del `\uXXXX`).

```python
#!/usr/bin/env python3
"""Regenera sesiones/INDICE.md desde los fragmentos de sesión. Uso: python3 herramientas/regenerar_indice.py"""
from datetime import datetime
from pathlib import Path
import re, sys

RAIZ = Path(__file__).resolve().parent.parent
DIR, SALIDA = RAIZ / "sesiones", RAIZ / "sesiones" / "INDICE.md"
PATRON = re.compile(r"^(?P<fecha>\d{4}-\d{2}-\d{2})-(?P<autor>[a-z0-9]+)-(?P<slug>.+)\.md$")

def campo(texto, nombre):
    m = re.search(rf"^\s*-\s*\*\*{nombre}\*\*:\s*(.+)$", texto, re.MULTILINE)
    return m.group(1).strip() if m else "—"

def main():
    if not DIR.is_dir():
        print(f"No existe {DIR}; crea sesiones/ primero.", file=sys.stderr); return 1
    filas = []
    for f in sorted(DIR.glob("*.md")):
        if f.name == "INDICE.md": continue
        m = PATRON.match(f.name)
        if not m: continue
        t = f.read_text(encoding="utf-8")
        filas.append((m["fecha"], m["autor"], m["slug"].replace("-", " "), f.name,
                      campo(t, "Commits"), campo(t, "Deja esperando a Oriol")))
    filas.sort(key=lambda r: r[0], reverse=True)
    lineas = ["# Índice de sesiones — regenerado, no editar a mano", "",
              f"_Regenerado: {datetime.now().isoformat(timespec='minutes')} · {len(filas)} sesiones_", "",
              "| Fecha | Autor | Sesión | Commits | Esperando a Oriol |", "|---|---|---|---|---|"]
    for fecha, autor, titulo, fichero, commits, oriol in filas:
        lineas.append(f"| {fecha} | {autor} | [{titulo}]({fichero}) | {commits} | {oriol} |")
    SALIDA.write_text("\n".join(lineas) + "\n", encoding="utf-8")
    print(f"INDICE.md regenerado con {len(filas)} sesiones.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
```

### 4.6 AGENTS.md — texto nuevo (se añade, no se sustituye)

```markdown
## → TABLERO.md — quién está en qué. Reclama ANTES de empezar; «Esperando a Oriol» es su bandeja.
## → sesiones/ — registro append-only: 1 fichero por sesión (YYYY-MM-DD-autor-slug.md); INDICE.md se regenera.
## → decisiones/ — decisiones numeradas D0001+; Estado: proposed espera a Oriol; accepted = inmutable.
## → herramientas/git-safe.sh — TODO git de escritura va con él (una operación a la vez, sin fallos silenciosos).
```

Y en la BITACORA, al desplegar (Fase 1), dos avisos sin tocar nada existente:
bajo el título de la Parte 0 → *«CONGELADA: las sesiones nuevas van a `sesiones/`;
este registro queda como histórico»*; idéntico aviso en la Parte 2 apuntando a `decisiones/`.

## 5. Cambios exactos al PROTOCOLO

| Regla | Ahora | Con D0001 |
|---|---|---|
| 1 (antes de trabajar) | lee la biblia, mira git log y Parte 0 | + **mira TABLERO.md y reclama tu fila ANTES de crear el primer fichero** |
| 4 (al terminar) | commit + entrada en Parte 0 | commit **con git-safe** + fichero en `sesiones/` + mover tu fila del TABLERO (+ generar INDICE si tocó sesiones) |
| — (nueva 4bis) | — | decisiones nuevas → `decisiones/DNNNN` con Estado: proposed; **sólo Oriol cambia Estado** |
| 6 (discrepancias) | documéntalas en Parte 4 o tu entrada | + si requiere decisión de Oriol: fichero en `decisiones/` y fila en la bandeja del TABLERO |

Todo lo demás del protocolo queda igual: reglas de propiedad (canon → cartas → motor),
append-only, trabajo ajeno sin commitear, convenciones del repo.

## 6. Plan de migración

- **Fase 0 — aceptación.** Oriol edita `Estado: accepted` en este fichero (o responde
  OK y la próxima sesión lo hace dejando constancia de quién y cuándo).
- **Fase 1 — despliegue** (una sesión corta): crear TABLERO.md (sembrado con las
  sesiones EN CURSO de la Parte 0), `sesiones/`, `herramientas/` (los dos scripts,
  `chmod +x`), avisos de congelación en Partes 0 y 2, texto nuevo de AGENTS.md.
  Commit único con git-safe. Nada se borra.
- **Fase 2 — primer ciclo real.** La siguiente sesión de cualquier agente usa el
  formato nuevo y regenera el INDICE. Si algo del formato chirría, se corrige en
  ese momento (la plantilla es versión 1, no dogma).
- **Fase 3 — opcional.** Worktrees por agente para el trabajo pesado de cuadrantes
  (cuestan un `build/` propio); decidir con datos de las Fases 1–2.

Los tableros por proyecto (`PARALLEL_DEVELOPMENT_BOARD.md` del motor, el CONTRATO
del experimento) **no se tocan**: el TABLERO global es la vista de workspace, los
de proyecto son el detalle de su zona.

## 7. Consecuencias

**A favor:** muere la contención de la bitácora (5 escritores, ficheros distintos);
Oriol tiene una bandeja real y decide editando un campo; las decisiones quedan
auditables e inmutables una a una; todo markdown puro sin runtime, renderizable en
Obsidian, legible por cualquier agente sin instrucciones especiales; git-safe
convierte tres trampas de la Parte 3 en código.

**En contra / riesgos:** es disciplina social, como el protocolo actual — nada
impone mecánicamente reclamar en el TABLERO (mitigado: la revisión al inicio de
sesión ya es obligatoria y el coste de mirar un fichero corto es bajo); el TABLERO
puede desactualizarse si alguien se olvida de mover filas (mitigado: el INDICE se
contrastaría en la revisión de inicio); rompe el hábito de 25 sesiones de escribir
en la Parte 0 (mitigado: aviso de congelación visible); la numeración D puede
colisionar (raro, resolución definida en §4.3).

---

## Anexo A — Estudio de alternativas (resumen, 28/08/2026)

Frameworks evaluados con su mecanismo de coordinación y puerta humana:
**Backlog.md** (claims de rutas con duración + pre-commit hook + worktrees +
orquestador; el único con exclusión mutua real, pero flujo 1-tarea-1-PR, config TOML,
proyecto joven) · **OpenSpec** (propuesta → aprobación → archivo fechado + specs
canónicas; sin claims) · **GitHub Spec Kit** (constitution/specify/plan/tasks/
implement; paralelismo por convención) · **BMAD-METHOD** (roles y story files;
verboso) · **Task Master** (tasks.json — descartado) · **Agent OS** (single-agent,
enfriado) · **AGENTS.md** (estándar de instrucciones, no coordinación — ya lo usáis).

Patrones humanos adoptados: **MADR** (decisión = fichero inmutable numerado con
ciclo proposed→accepted→superseded) · **PEP/RFD** (cola de propuestas para UN
decisor) · **towncrier/changelog.d** (un fragmento por cambio, cero conflictos,
índice consolidado) · **design docs** (contexto/objetivos-y-no-objetivos/diseño/
alternativas — estructura que ya usan vuestros docs buenos) · **obsidian-kanban**
(`## Columna` + ítems = markdown plano renderizable como tablero) · **git worktree
por escritor** (cada worktree tiene su index; descartado por ahora por coste de
build) · **wrappers flock** para git de escritura (adoptado, adaptado al sandbox
con expiración en vez de borrado).

Fuentes: github.com/MrLesk/Backlog.md · github.com/Fission-AI/OpenSpec ·
github.com/github/spec-kit · github.com/bmad-code-org/bmad-method · agents.md ·
adr.github.io/madr · peps.python.org/pep-0001 · rfd.shared.oxide.computer ·
towncrier.readthedocs.io · github.com/mgmeyers/obsidian-kanban · git-scm.com/docs/git-worktree ·
industrialempathy.com/posts/design-docs-at-google

## Anexo B — Necesidades con evidencia

N1 5+ escritores concurrentes en la misma máquina y copia de trabajo (4 IAs + Oriol) ·
N2 contención de la bitácora única · N3 carreras de git e `index.lock` (commits
silenciosos, sandbox sin borrado) · N4 reclamo previo de tareas (colisión de Plazas,
16/08) · N5 cola de decisiones para Oriol dispersa hoy · N6 registro auditable de
decisiones con motivo (Parte 2 es ADR de facto) · N7 sin servidor/PRs, español,
repos anidados · N8 que Oriol pueda entrar al workspace sin leer 1.130 líneas.
