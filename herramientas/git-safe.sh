#!/usr/bin/env bash
# git-safe — serializa el git de ESCRITURA entre agentes.
# Uso: herramientas/git-safe.sh commit -m "…"   (envuelve UN comando git cada vez)
# Sólo hace falta para escritura (add/commit/rm/merge/rebase/reset…);
# las lecturas (status/log/diff) pueden ir en git pelado.
# Reglas de la BITACORA (Parte 3, "Git en este entorno") hechas código:
#   1. una operación de escritura a la vez por repo (lock por mkdir, sin borrar nada)
#   2. un lock rancio (>STALE s) se RENOMBRA y se ignora (el sandbox no puede borrar)
#   3. ningún fallo en silencio: el exit code se comprueba y se avisa
set -uo pipefail

STALE=300; REINTENTOS=6; PAUSA=5

GITDIR="$(git rev-parse --git-dir 2>/dev/null)" || { echo "git-safe: no estoy en un repo" >&2; exit 2; }
LOCK="$GITDIR/git-safe.lock"

# BSD/macOS usa `stat -f %m`; GNU/Linux, `stat -c %Y`. El sandbox de
# automatización puede ser Linux aunque el checkout viva en macOS: si se
# asume una sola variante, el fallback a 0 convierte cualquier candado en
# «rancio» y el bucle no llega nunca a adquirirlo.
mtime() {
  case "$(uname -s)" in
    Darwin|FreeBSD) stat -f %m "$1" ;;
    *)              stat -c %Y "$1" ;;
  esac
}

adquirir() {
  local intento=0
  while true; do
    if mkdir "$LOCK" 2>/dev/null; then
      printf '%s %s %s\n' "$$" "$(date +%s)" "${USER:-agente}" > "$LOCK/info"; return 0
    fi
    local edad=$(( $(date +%s) - $(mtime "$LOCK" 2>/dev/null || echo 0) ))
    if [ "$edad" -gt "$STALE" ]; then mv "$LOCK" "$LOCK.rancio.$$" 2>/dev/null || true; continue; fi
    intento=$((intento+1))
    if [ "$intento" -ge "$REINTENTOS" ]; then
      echo "git-safe: no pude adquirir el lock (${PAUSA}s × $REINTENTOS). ¿Otra sesión commitando?" >&2; exit 3
    fi
    sleep "$PAUSA"
  done
}

adquirir
trap 'rm -rf "$LOCK" 2>/dev/null || true' EXIT   # si el sandbox no puede borrar, el lock expira solo (STALE)

git "$@"
rc=$?
[ "$rc" -ne 0 ] && echo "git-safe: git $1 falló (código $rc) — NO sigas como si nada: revisa y reintenta" >&2
exit "$rc"
