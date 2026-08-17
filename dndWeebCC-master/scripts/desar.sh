#!/usr/bin/env bash
# desar.sh — Desa la feina al control de versions amb una sola ordre.
#
#   bash scripts/desar.sh "content(dotes): 20 dotes noves per tiers 3-5"
#   bash scripts/desar.sh                    # obre un resum i demana el missatge
#
# Fa tres coses: ensenya què ha canviat, fa el commit i, si hi ha un remot
# configurat, hi puja. Pensat per a la rutina de "cada cop que una cosa funciona".
set -euo pipefail

cd "$(dirname "$0")/.."

if [ -n "$(git status --porcelain)" ]; then
    echo "── Canvis pendents ──────────────────────────────"
    git status --short
    echo "────────────────────────────────────────────────"
else
    echo "No hi ha res per desar: l'arbre de treball està net."
    exit 0
fi

MISSATGE="${1:-}"
if [ -z "$MISSATGE" ]; then
    read -r -p "Missatge de commit (tipus(àmbit): què fa): " MISSATGE
fi

if [ -z "$MISSATGE" ]; then
    echo "Cal un missatge de commit. Consulta docs/Guia_Control_Versions.md §4."
    exit 1
fi

git add -A
git commit -m "$MISSATGE"

if git remote | grep -q .; then
    REMOT="$(git remote | head -1)"
    echo "Pujant a '$REMOT'…"
    git push "$REMOT" HEAD --tags
else
    echo "Commit fet. (Sense remot configurat — veure docs/Guia_Control_Versions.md §7.)"
fi
