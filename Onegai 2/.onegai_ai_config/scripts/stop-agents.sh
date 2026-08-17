#!/usr/bin/env bash
# ================================================================
# STOP — detener servicios (Ollama + AuthorAgent)
# ================================================================
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.."; pwd)"
CFG="$ROOT/.onegai_ai_config"
LOGS="$CFG/logs"
mkdir -p "$LOGS"

C_R='\033[0;31m'; C_G='\033[0;32m'; C_Y='\033[1;33m'; C_W='\033[1;37m'; C_0='\033[0m';
log(){ printf "${C_W}[%s]${C_0} %s\n" "$(date +%H:%M:%S)" "$*"; }

log "${C_Y}Deteniendo AuthorAgent...${C_0}"
if [ -f "$LOGS/author-agent.pid" ]; then
    PID=$(cat "$LOGS/author-agent.pid")
    if ps -p "$PID" >/dev/null 2>&1; then
        kill "$PID" 2>/dev/null || true
        sleep 2
        # Si sigue vivo, SIGKILL
        if ps -p "$PID" >/dev/null 2>&1; then
            kill -9 "$PID" 2>/dev/null || true
        fi
        log "  AuthorAgent (PID $PID) detenido"
    fi
    rm -f "$LOGS/author-agent.pid"
fi
# Por si acaso, matar procesos de node autoragent
pkill -f "AuthorAgent.*npm start" 2>/dev/null || true
pkill -f "AuthorAgent.*node.*gateway" 2>/dev/null || true

log "${C_Y}Deteniendo ollama serve...${C_0}"
if [ -f "$LOGS/ollama.pid" ]; then
    PID=$(cat "$LOGS/ollama.pid")
    if ps -p "$PID" >/dev/null 2>&1; then
        kill "$PID" 2>/dev/null || true
        sleep 2
        ps -p "$PID" >/dev/null 2>&1 && kill -9 "$PID" 2>/dev/null || true
        log "  Ollama (PID $PID) detenido"
    fi
    rm -f "$LOGS/ollama.pid"
fi
# Fallback: matar ollama serve genérico
pkill -f "ollama serve" 2>/dev/null || true

# Descargar modelos de la VRAM para liberar
if curl -sf http://localhost:11434 >/dev/null 2>&1; then
    log "  Descargando modelos de la memoria..."
    for m in onegai/story-writer onegai/author-critic deepseek-r1 gemma2 llama3.1; do
        curl -s http://localhost:11434/api/generate \
             -H 'Content-Type: application/json' \
             -d "{\"model\":\"$m\",\"keep_alive\":0,\"prompt\":\"bye\"}" \
             >/dev/null 2>&1 || true
    done
fi

log "${C_G}✅ Todos los servicios detenidos.${C_0}"
log "   Verifica con:  bash $CFG/scripts/status-agents.sh"
