#!/usr/bin/env bash
# Puja NOMÉS EL CODI de Software/ al repo GitHub Onegai-Biodiversitat.
#   - El filtre és herramientas/biodiversitat.gitignore (s'hi publica com a .gitignore).
#   - Treballa en una carpeta temporal: no toca cap repo ni cap fitxer de Software/.
#   - Cada execució afegeix un commit nou a sobre del que ja hi ha a GitHub (mai --force).
#   - Els fitxers esborrats a Software/ també s'esborren a GitHub a la següent pujada.
# Ús:  bash herramientas/pujar_a_biodiversitat.sh [URL]      (BRANCA=main per defecte)
#      NOMES_LLISTA=1 bash herramientas/pujar_a_biodiversitat.sh   → només mostra què pujaria
# Preparat per Claude (Cowork) el 17/09/2026.
set -euo pipefail
URL="${1:-https://github.com/OriolCalvis/Onegai-Biodiversitat.git}"
BRANCA="${BRANCA:-main}"
SW="$(cd "$(dirname "$0")/.." && pwd)"
FILTRE="$SW/herramientas/biodiversitat.gitignore"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
REPO="$TMP/repo"
export COPYFILE_DISABLE=1   # macOS: que tar no afegeixi fitxers ._*

if [ -n "${NOMES_LLISTA:-}" ]; then
  git init -q "$REPO"
else
  echo >&2 "1/4 Llegint l'estat de GitHub ($BRANCA)…"
  if git ls-remote --exit-code --heads "$URL" "$BRANCA" >/dev/null 2>&1; then
    git clone -q --depth 1 --branch "$BRANCA" "$URL" "$REPO"
  else
    git init -q "$REPO"
    git -C "$REPO" symbolic-ref HEAD "refs/heads/$BRANCA"
    git -C "$REPO" remote add origin "$URL"
  fi
fi

# Guardar README/LICENSE que ja hi hagi a GitHub si Software/ no en té
mkdir -p "$TMP/guardats"
for f in README.md LICENSE LICENSE.md LICENSE.txt; do
  [ -f "$REPO/$f" ] && [ ! -f "$SW/$f" ] && cp "$REPO/$f" "$TMP/guardats/"
done
find "$REPO" -mindepth 1 -maxdepth 1 ! -name .git -exec rm -rf {} +
cp "$FILTRE" "$REPO/.gitignore"
cp "$TMP/guardats/"* "$REPO/" 2>/dev/null || true

echo >&2 "2/4 Triant els fitxers de codi…"
( cd "$SW" && find . -name .git -prune -o -type f -print | sed 's#^\./##' ) > "$TMP/tots.txt"
git -C "$REPO" -c core.excludesFile=/dev/null -c core.quotePath=off \
    check-ignore --no-index --stdin -v -n < "$TMP/tots.txt" \
  | awk -F'\t' '$1 ~ /^[^:]*:[0-9]+:!/ { print $2 }' \
  | grep -vx ".gitignore" > "$TMP/codi.txt" || true
N=$(wc -l < "$TMP/codi.txt" | tr -d ' ')
[ "$N" -gt 0 ] || { echo "No s'ha trobat cap fitxer de codi."; exit 1; }

if [ -n "${NOMES_LLISTA:-}" ]; then
  cat "$TMP/codi.txt"; echo "== $N fitxers" >&2; exit 0
fi

echo >&2 "3/4 Copiant $N fitxers…"
tar -C "$SW" -cf - -T "$TMP/codi.txt" | tar -C "$REPO" -xf -
GRANS=$(find "$REPO" -path "$REPO/.git" -prune -o -type f -size +95M -print)
[ -z "$GRANS" ] || { echo "Fitxers massa grans per a GitHub:"; echo "$GRANS"; exit 1; }

echo >&2 "4/4 Commit i pujada…"
cd "$REPO"
git add -A
if git diff --cached --quiet; then echo "Res de nou: GitHub ja està al dia."; exit 0; fi
NOM=$(git config user.name || git -C "$SW/MotorGraphico-main" config user.name || echo Oriol)
MAIL=$(git config user.email || git -C "$SW/MotorGraphico-main" config user.email || echo ocalvis@gmail.com)
{
  echo "Codi de Software/ — $(date '+%Y-%m-%d %H:%M')"
  echo
  echo "Només codi (filtre: herramientas/biodiversitat.gitignore). Origen:"
  for r in MotorGraphico-main dndWeebCC-master "Onegai 2" "Onegai 2/AuthorAgent" "Onegai 2/StoryCraftr"; do
    [ -d "$SW/$r/.git" ] || continue
    echo "- $r: $(git -C "$SW/$r" --no-optional-locks log -1 --format='%h %s' 2>/dev/null || echo '?')" \
         "($(git -C "$SW/$r" --no-optional-locks rev-parse --abbrev-ref HEAD 2>/dev/null || echo '?'))"
  done
} > "$TMP/missatge.txt"
git -c user.name="$NOM" -c user.email="$MAIL" commit -q -F "$TMP/missatge.txt"
git diff --stat HEAD~1 HEAD 2>/dev/null | tail -1 || true
git push origin "HEAD:refs/heads/$BRANCA"
echo "FET: $N fitxers de codi a $URL ($BRANCA)"
