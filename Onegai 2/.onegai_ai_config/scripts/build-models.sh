#!/usr/bin/env bash
# ================================================================
# BUILD de los modelos personalizados de Onegai (1 vez o actualizar)
#   onegai/story-writer  <- deepseek-r1:8.2B
#   onegai/author-critic <- gemma2:9b  (o llama3.1:8b fallback)
# ================================================================
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.."; pwd)"
CFG="$ROOT/.onegai_ai_config"
MF_DIR="$CFG/modelfiles"

log(){ printf "\033[1;37m[%s]\033[0m %s\n" "$(date +%H:%M:%S)" "$*"; }

log "=== ONEGAI AI — Build de modelos personalizados ==="

# --- Asegurar modelos base descargados ---
ensure_pull(){
    local M=$1
    if ollama list 2>/dev/null | grep -q "^${M} "; then
        log "  ✅ Base ${M} ya descargado"
    else
        log "  ⏳ Descargando modelo base ${M}..."
        ollama pull "$M" >/dev/null 2>&1 \
            || { echo "FAIL al descargar $M"; exit 2; }
        log "  ✅ Base ${M} OK"
    fi
}

ensure_pull "deepseek-r1:latest"
# Para el crítico: preferimos gemma2:9b (creativo-crítico), 
# pero si no está disponible usamos llama3.1:8b
CRITIC_BASE="gemma2:9b"
if ollama pull --help >/dev/null 2>&1 && ! ollama pull "$CRITIC_BASE" >/tmp/critic_pull.log 2>&1; then
    log "  ⚠️  Fallo pull $CRITIC_BASE — fallback a llama3.1:8b"
    CRITIC_BASE="llama3.1:8b"
    ensure_pull "$CRITIC_BASE"
else
    ensure_pull "$CRITIC_BASE"
fi

# --- Build Modelo 1: onegai/story-writer ---
log "🔨 Construyendo onegai/story-writer  (base: deepseek-r1)"
if ollama create "onegai/story-writer" -f "$MF_DIR/Modelfile.story-writer" >/dev/null 2>&1; then
    log "  ✅ onegai/story-writer creado"
else
    log "  ❌ Fallo onegai/story-writer. Reintentando verboso..."
    ollama create "onegai/story-writer" -f "$MF_DIR/Modelfile.story-writer" || exit 3
fi

# --- Build Modelo 2: onegai/author-critic (reemplazar placeholder) ---
log "🔨 Construyendo onegai/author-critic  (base: $CRITIC_BASE)"
TMP_MF=$(mktemp)
sed "s/{{BASE_MODEL_AGENT2}}/$CRITIC_BASE/" \
    "$MF_DIR/Modelfile.author-critic" > "$TMP_MF"
if ollama create "onegai/author-critic" -f "$TMP_MF" >/dev/null 2>&1; then
    log "  ✅ onegai/author-critic creado (base=$CRITIC_BASE)"
else
    log "  ❌ Fallo onegai/author-critic. Reintentando verboso..."
    ollama create "onegai/author-critic" -f "$TMP_MF" || exit 4
fi
rm -f "$TMP_MF"

echo
echo "=== RESULTADO ==="
ollama list | grep -E "(NAME|onegai/|deepseek|gemma2|llama3)"
