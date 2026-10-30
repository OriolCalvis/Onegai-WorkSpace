#!/bin/bash
# Antigravity de noche: coge E de la cola, la ejecuta con `agy -p` en un worktree aislado, guarda el
# relevo, el diff y el consumo de tokens, y hace commit en la rama de la E. Lo lanza launchd (02:10).
# Claude Code NO llama a este script ni a agy: solo deja E en la cola y lee los resultados.
# bash 3.2 (el del Mac): sin mapfile ni arrays asociativos.
set -u
REPO="${ONEGAI_REPO:-$HOME/Documents/Documentos - Oriol Os (2)/Software}"
A="$REPO/out/fabrica/agy"
RAMAS="${ONEGAI_RAMAS:-$HOME/onegai-ramas}"
export PATH="$HOME/.local/bin:/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:$PATH"
G="git -c core.checkStat=minimal -c core.trustctime=false"
SELLO="$(date +%Y%m%d-%H%M%S)"
LOG="$A/nocturno.log"
log() { printf '%s %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$*" >> "$LOG"; }

mkdir "$A/.lock" 2>/dev/null || { log "otra ejecución en marcha (.lock): salgo"; exit 0; }
trap 'rmdir "$A/.lock" 2>/dev/null' EXIT

AGY="${AGY:-$(command -v agy || true)}"
if [ -z "$AGY" ]; then log "BLOQUEADO: no encuentro agy en el PATH ($PATH)"; exit 0; fi

# --- presupuesto: cuántas E esta noche (0 si se ha agotado)
CUANTAS=$(python3 - "$A" <<'PY'
import json, sys, datetime as dt, pathlib
a = pathlib.Path(sys.argv[1]); p = json.loads((a/"presupuesto.json").read_text())
hoy = dt.date.today(); lunes = hoy - dt.timedelta(days=hoy.weekday())
semana = 0
f = a/"consumo.jsonl"
if f.exists():
    for l in f.read_text().splitlines():
        try:
            r = json.loads(l)
            if dt.date.fromisoformat(r["t"][:10]) >= lunes: semana += int(r.get("total_tokens") or 0)
        except Exception: pass
lim = p.get("tokens_por_semana")
print(0 if (lim and semana >= lim) else int(p.get("max_e_por_noche", 1)))
PY
)
[ "$CUANTAS" = "0" ] && { log "presupuesto semanal agotado: esta noche no se lanza nada"; exit 0; }
TIMEOUT=$(python3 -c 'import json,sys;print(json.load(open(sys.argv[1])).get("print_timeout","45m"))' "$A/presupuesto.json")
ARGS=$(python3 -c 'import json,sys,shlex;print(" ".join(shlex.quote(x) for x in json.load(open(sys.argv[1])).get("agy_args",[])))' "$A/presupuesto.json")

hechas=0
# Los nombres llevan espacios en la ruta («Documentos - Oriol Os (2)»): glob directo, nunca $(ls …).
LISTA_ERR="$(ls -1 "$A/cola" 2>&1 >/dev/null)"
[ -n "$LISTA_ERR" ] && log "AVISO: no puedo listar la cola: $LISTA_ERR (¿permisos de macOS sobre Documents?)"
for E_FICH in "$A"/cola/*.md; do
  [ -e "$E_FICH" ] || continue
  [ "$hechas" -ge "$CUANTAS" ] && break
  E="$(basename "$E_FICH" .md)"
  case "$E" in *[!A-Za-z0-9._-]*) log "nombre de E no válido: $E (se salta)"; continue;; esac
  BASE="$(sed -n 's/^Base:[[:space:]]*//p' "$E_FICH" | head -1)"; BASE="${BASE:-claude/unificacion-28-10}"
  RAMA="agente/antigravity/$E"
  WT="$RAMAS/agy-$E"
  OUT="$A/resultados/${SELLO}_$E"; mkdir -p "$OUT"
  mv "$E_FICH" "$A/en_curso/$E.md"
  log "E $E: empieza (base $BASE, rama $RAMA)"

  # --- worktree aislado (se reutiliza si ya existe: una E devuelta continúa donde se quedó)
  if [ ! -d "$WT" ]; then
    mkdir -p "$RAMAS"
    if $G -C "$REPO" rev-parse -q --verify "refs/heads/$RAMA" >/dev/null; then
      (cd "$REPO" && herramientas/git-safe.sh -c core.checkStat=minimal worktree add "$WT" "$RAMA") >> "$OUT/git.log" 2>&1
    else
      (cd "$REPO" && herramientas/git-safe.sh -c core.checkStat=minimal worktree add -b "$RAMA" "$WT" "$BASE") >> "$OUT/git.log" 2>&1
    fi
    if [ ! -d "$WT" ]; then
      log "E $E: BLOQUEADO creando el worktree (ver $OUT/git.log)"; mv "$A/en_curso/$E.md" "$A/cola/$E.md"; continue
    fi
  fi
  rm -f "$WT/.agy_relevo.md"

  # --- prompt: plantilla + la E
  python3 - "$A/PROMPT_NOCHE.md" "$A/en_curso/$E.md" "$OUT/prompt.md" "$E" "$WT" "$RAMA" "$BASE" <<'PY'
import sys
pl, ef, out, e, wt, rama, base = sys.argv[1:]
t = open(pl, encoding="utf-8").read()
for k, v in {"{E}": e, "{WORKTREE}": wt, "{RAMA}": rama, "{BASE}": base,
             "{ENCARGO}": open(ef, encoding="utf-8").read()}.items():
    t = t.replace(k, v)
open(out, "w", encoding="utf-8").write(t)
PY

  # --- Antigravity
  T0=$(date +%s)
  (cd "$WT" && eval "\"$AGY\" -p \"\$(cat \"$OUT/prompt.md\")\" --output-format json --print-timeout \"$TIMEOUT\" $ARGS") \
      > "$OUT/respuesta.json" 2> "$OUT/stderr.log"
  RC=$?
  DUR=$(( $(date +%s) - T0 ))

  # --- relevo, diff y commit (lo hace el script, no agy)
  [ -f "$WT/.agy_relevo.md" ] && cp "$WT/.agy_relevo.md" "$OUT/relevo.md"
  (cd "$WT" && $G add -A -N -- . ':(exclude).agy_relevo.md' 2>/dev/null; $G diff -- . ':(exclude).agy_relevo.md' > "$OUT/diff.patch"; $G status --porcelain > "$OUT/status.txt")
  COMMIT=""
  if [ -s "$OUT/status.txt" ] && grep -qv '^?? .agy_relevo.md$' "$OUT/status.txt"; then
    (cd "$WT" && "$REPO/herramientas/git-safe.sh" -c core.checkStat=minimal add -A -- . ':(exclude).agy_relevo.md' \
       && GIT_AUTHOR_NAME="Antigravity (noche)" GIT_AUTHOR_EMAIL="agy@onegai.local" \
          "$REPO/herramientas/git-safe.sh" -c core.checkStat=minimal commit -q -m "$E: trabajo nocturno de Antigravity" \
          -m "Relevo en out/fabrica/agy/resultados/${SELLO}_$E/relevo.md" -m "Fabrica-E: $E") >> "$OUT/git.log" 2>&1
    COMMIT="$($G -C "$WT" rev-parse HEAD 2>/dev/null)"
  fi

  # --- consumo y estado
  python3 - "$A" "$OUT" "$E" "$RC" "$DUR" "$RAMA" "$COMMIT" <<'PY'
import json, sys, datetime as dt, pathlib
a, out, e, rc, dur, rama, commit = sys.argv[1:]
out = pathlib.Path(out); uso = {}
try:
    j = json.loads((out/"respuesta.json").read_text(encoding="utf-8") or "{}")
    uso = j.get("usage") or {}
except Exception:
    pass
fila = {"t": dt.datetime.now().astimezone().isoformat(timespec="seconds"), "e": e, "rc": int(rc),
        "segundos": int(dur), **{k: uso.get(k) for k in ("input_tokens", "output_tokens", "thinking_tokens",
        "cache_read_tokens", "total_tokens")}}
with open(pathlib.Path(a)/"consumo.jsonl", "a", encoding="utf-8") as f:
    f.write(json.dumps(fila, ensure_ascii=False) + "\n")
estado = {**fila, "rama": rama, "commit": commit or None, "relevo": (out/"relevo.md").exists(),
          "resultado": "terminado" if int(rc) == 0 else "fallado"}
(out/"estado.json").write_text(json.dumps(estado, ensure_ascii=False, indent=2), encoding="utf-8")
PY
  mv "$A/en_curso/$E.md" "$A/hechas/$E.md"
  log "E $E: fin rc=$RC ${DUR}s commit=${COMMIT:-ninguno} relevo=$([ -f "$OUT/relevo.md" ] && echo sí || echo NO)"
  hechas=$((hechas + 1))
done
[ "$hechas" -eq 0 ] && log "cola vacía o nada lanzable"
exit 0
