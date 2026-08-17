#!/usr/bin/env bash
# ================================================================
# STATUS / DASHBOARD rápida de todo el sistema
# ================================================================
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.."; pwd)"
CFG="$ROOT/.onegai_ai_config"
LOGS="$CFG/logs"
C_G='\033[0;32m'; C_R='\033[0;31m'; C_Y='\033[1;33m'; C_W='\033[1;37m'; C_0='\033[0m';

ok() {  printf " ${C_G}✔ OK ${C_0} %s\n" "$*"; }
err() { printf " ${C_R}✖ NO ${C_0} %s\n" "$*"; }
warn() { printf " ${C_Y}⚡    ${C_0} %s\n" "$*"; }

# --- Cabecera ---
echo
printf "${C_W}═══════════════════ ONEGAI AI AGENTS — STATUS ═══════════════════${C_0}\n"
date "+Fecha: %d/%m/%Y %H:%M:%S"
uname -a | awk '{print "Sistema: "$1" "$2" "$3" "$NF}'
echo

# --- Ollama daemon health ---
printf "${C_W}[1/5] Ollama daemon:${C_0} "
if curl -sf http://localhost:11434/api/tags >/dev/null 2>&1; then
    ok "API reachable (http://localhost:11434)"
    echo "      Modelos instalados:"
    ollama list 2>/dev/null | awk 'NR==1 {print "         " $0} NR>1 && $1 !~ /^$/ {printf "         %-30s %-10s %s\n",$1,$2,$3}'
else
    err "Ollama NO reachable. Arranca con:  bash $CFG/scripts/start-agents.sh"
fi
echo

# --- Modelos personalizados ---
printf "${C_W}[2/5] Modelos personalizados onegai/*:${C_0}\n"
for m in onegai/story-writer onegai/author-critic; do
    if ollama list 2>/dev/null | grep -q "^$m:"; then
        echo "   " | ok "$m  ($(ollama list 2>/dev/null | awk -v M="$m:" '$1 ~ M {print $2, $3, $4}'))"
    else
        echo "   " | err "$m  — NO BUILT. Corre:  bash $CFG/scripts/build-models.sh"
    fi
done
echo

# --- AuthorAgent dashboard ---
printf "${C_W}[3/5] AuthorAgent (Node UI):${C_0} "
if curl -sf http://localhost:3847/api/system/health >/dev/null 2>&1; then
    ok "http://localhost:3847"
    # Mostrar conexiones detectadas
    CONN=$(curl -s http://localhost:3847/api/system/connections 2>/dev/null \
           | python3 -c "import sys,json;d=json.load(sys.stdin);print([x for x in d.get('connections',[])])" 2>/dev/null \
           || echo "?")
    echo "      Conexiones: $CONN"
else
    err "No reachable. Arráncalo con start-agents.sh"
fi
echo

# --- Memoria y carga ---
printf "${C_W}[4/5] Recursos del sistema:${C_0}\n"
RAM_MB=$(vm_stat 2>/dev/null | awk '/Pages free/ {free=$3} /Pages active/ {act=$3} /Pages inactive/ {in=$3} END {page=4096; total=(free+act+in)*page/1024/1024; used=(act+in)*page/1024/1024; printf "%.0f MB usados / %.0f MB total (%.1f%%)", used, total, 100*used/total}' 2>/dev/null)
echo "      RAM macOS (vm_stat aprox): $RAM_MB"
echo "      Ollama models RAM used (via ollama ps):"
ollama ps 2>/dev/null | awk 'NR==1{print "         "$0} NR>1{print "         "$0}'
echo

# --- Logs recientes (últimas 5 líneas error) ---
printf "${C_W}[5/5] Últimos mensajes en logs (si existen):${C_0}\n"
for f in "$LOGS/ollama.stderr.log" "$LOGS/author-agent.stderr.log"; do
    if [ -s "$f" ]; then
        bn=$(basename "$f")
        echo "      ┌─ $bn (tail 5):"
        tail -5 "$f" | sed 's/^/      │ /'
        echo "      └"
    fi
done
printf "${C_W}══════════════════════════════════════════════════════════════════════${C_0}\n"
