#!/usr/bin/env bash
# ================================================================
# SCRIPT MAESTRO DE INICIO: Onegai AI Agents
# Arranca:
#   1) Ollama daemon (si no está ya) con paralelismo configurado
#   2) AuthorAgent dashboard (web) en  http://localhost:3847
#   3) Pre-carga en RAM los dos modelos personalizados (Warm-up)
# ================================================================
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.."; pwd)"
CONFIG="$ROOT/.onegai_ai_config"
LOGS="$CONFIG/logs"
mkdir -p "$LOGS"

# --------- COLORES ---------
C_R='\033[0;31m'; C_G='\033[0;32m'; C_Y='\033[1;33m';
C_B='\033[1;34m'; C_W='\033[1;37m'; C_0='\033[0m';
log(){ printf "${C_W}[%s]${C_0} %b\n" "$(date +%H:%M:%S)" "$*"; }
ok(){ printf "  ${C_G}✔${C_0} %b\n" "$*"; }
warn(){ printf "  ${C_Y}⚡${C_0} %b\n" "$*"; }
err(){ printf "  ${C_R}✖${C_0} %b\n" "$*"; }

# ================================================================
# 0. EXPORTS DEL ENTORNO OLLAMA (igual que el plist, por si el
#    usuario arranca este script sin usar el launchd)
# ================================================================
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"
export OLLAMA_NUM_PARALLEL=3
export OLLAMA_MAX_LOADED_MODELS=3
export OLLAMA_MAX_BATCH=512
export OLLAMA_GPU_LAYERS=999
export OLLAMA_HOST="127.0.0.1:11434"
export OLLAMA_ORIGINS="http://localhost:*,http://127.0.0.1:*"
export OLLAMA_KEEP_ALIVE="2h"

log "${C_B}=== ONEGAI AI AGENTS — STARTUP ===${C_0}"
log "ROOT: $ROOT"

# ================================================================
# 1. OLLAMA — comprobar si el daemon ya está corriendo. Si no,
#    arrancarlo en BG. Si lo está, sólo nos aseguramos de que
#    tenga las variables adecuadas (reload si no).
# ================================================================
log "${C_B}Paso 1/3: Verificando Ollama daemon${C_0}"
if curl -sf "http://${OLLAMA_HOST}/api/tags" >/dev/null 2>&1; then
    ok "Ollama ya está corriendo en ${OLLAMA_HOST}"
else
    warn "Ollama no activo — arrancando en background..."
    nohup ollama serve \
        >"$LOGS/ollama.stdout.log" \
        2>"$LOGS/ollama.stderr.log" &
    OLLAMA_PID=$!
    echo "$OLLAMA_PID" > "$LOGS/ollama.pid"
    # Esperar hasta 20s
    for _ in {1..20}; do
        curl -sf "http://${OLLAMA_HOST}/api/tags" >/dev/null 2>&1 && break
        sleep 1
    done
    if curl -sf "http://${OLLAMA_HOST}/api/tags" >/dev/null 2>&1; then
        ok "Ollama arrancado correctamente (PID $OLLAMA_PID)"
    else
        err "No se pudo arrancar Ollama. Mirar $LOGS/ollama.stderr.log"
        exit 1
    fi
fi
sleep 1

# ================================================================
# 2. Verificar que EXISTEN los modelos personalizados. Si no,
#    lanzar el build de uno o ambos.
# ================================================================
log "${C_B}Paso 2/3: Verificando modelos personalizados${C_0}"
NEED_BUILD=0
check_model(){
    local M=$1
    if ollama list 2>/dev/null | grep -q "^$M "; then
        ok "Modelo $M presente ✅"
    else
        warn "Modelo $M FALTANTE — requiere build 🔨"
        NEED_BUILD=1
    fi
}
check_model "onegai/story-writer"
check_model "onegai/author-critic"
if [ $NEED_BUILD -eq 1 ]; then
    warn "Llama a:  bash \"$CONFIG/scripts/build-models.sh\""
    warn "(No lo hacemos ahora en este script para no bloquear)"
fi

# ================================================================
# 3. AuthorAgent dashboard (Node, puerto 3847)
# ================================================================
log "${C_B}Paso 3/3: AuthorAgent dashboard${C_0}"
AA_DIR="$ROOT/AuthorAgent"
if [ -d "$AA_DIR" ]; then
    if curl -sf http://localhost:3847/api/system/health >/dev/null 2>&1; then
        ok "AuthorAgent ya corriendo en http://localhost:3847"
    else
        warn "AuthorAgent no activo — arrancando..."
        (
            cd "$AA_DIR"
            nohup npm start \
                >"$LOGS/author-agent.stdout.log" \
                2>"$LOGS/author-agent.stderr.log" &
        )
        echo $! > "$LOGS/author-agent.pid"
        for _ in {1..30}; do
            curl -sf http://localhost:3847/api/system/health >/dev/null 2>&1 && break
            sleep 1
        done
        if curl -sf http://localhost:3847/api/system/health >/dev/null 2>&1; then
            ok "AuthorAgent listo en http://localhost:3847"
        else
            warn "AuthorAgent puede estar arrancando aún — revisa $LOGS/author-agent.stderr.log"
        fi
    fi
else
    err "AuthorAgent no encontrado en $AA_DIR. Instálalo primero."
fi

# ================================================================
# 4. Warm-up opcional: enviar una petición corta a ambos modelos
#    para que estén precargados en GPU/RAM y la primera pregunta
#    del usuario no sea lenta.
# ================================================================
log "${C_B}Warm-up (opcional) — precargando modelos en RAM...${C_0}"
warmup(){
    local M=$1 Q=$2
    curl -s -X POST "http://${OLLAMA_HOST}/api/generate" \
        -H 'Content-Type: application/json' \
        -d "{\"model\":\"$M\",\"prompt\":\"$Q\",\"stream\":false,\"options\":{\"num_predict\":32}}" \
        >/dev/null 2>&1
}
warmup "onegai/story-writer"  "Hola. Di tu nombre y propósito." &
warmup "onegai/author-critic" "Hola. Di tu nombre y propósito." &
wait
ok "Warm-up finalizado (respuestas cortas)"

# ================================================================
# FIN — print URLs y comandos útiles
# ================================================================
echo
printf "${C_G}══════════════════════════════════════════════════${C_0}\n"
printf "${C_G}✔ ONEGAI AI AGENTS — TODOS LOS SERVICIOS ACTIVOS ✔${C_0}\n"
printf "${C_G}══════════════════════════════════════════════════${C_0}\n"
printf "  📘 Ollama daemon    : ${C_W}http://%s${C_0} (API REST)\n" "${OLLAMA_HOST}"
printf "  📖 AuthorAgent UI   : ${C_W}http://localhost:3847${C_0}\n"
printf "  🖊  Modelo Escritor  : ${C_W}onegai/story-writer${C_0}   (deepseek-r1 8.2B)\n"
printf "  🔎 Modelo Crítico   : ${C_W}onegai/author-critic${C_0}  (gemma2 9B)\n"
printf "  📝 Scripts de ayuda : ${C_W}%s/scripts${C_0}\n" "$CONFIG"
printf "  📋 Ver estado       : ${C_W}bash %s/scripts/status-agents.sh${C_0}\n" "$CONFIG"
printf "  🛑 Detener todo     : ${C_W}bash %s/scripts/stop-agents.sh${C_0}\n" "$CONFIG"
printf "  🧪 Test unitario    : ${C_W}bash %s/scripts/test-agents.sh${C_0}\n" "$CONFIG"
printf "\n"
