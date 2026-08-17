#!/usr/bin/env bash
# ================================================================
#  WRAPPER CLI STORYCRAFTR → usa el modelo onegai/story-writer
#  Autor: Onegai AI
#  USO:
#     bash storycraftr.sh --help
#     bash storycraftr.sh init "Mi nueva novela Onegai"
#     bash storycraftr.sh worldbuilding
#     bash storycraftr.sh outline
#     bash storycraftr.sh write chapter1
#     bash storycraftr.sh chat
#  Nota: si StoryCraftr acaba de romperse por incompatibilidades de
#  langchain (como nos pasó en la primera instalación), usa mejor
#  AuthorAgent UI (es más completo y actualizado).
# ================================================================
set -euo pipefail
export PATH="/opt/homebrew/bin:/usr/local/bin:/Users/admin/.local/bin:$PATH"
export OLLAMA_HOST="127.0.0.1:11434"
# Le indicamos a StoryCraftr que use NUESTRO modelo personalizado
export STORYCRAFTR_MODEL_NAME="onegai/story-writer"
export STORYCRAFTR_PROVIDER="ollama"
export STORYCRAFTR_BASE_URL="http://${OLLAMA_HOST}"
# Parámetros de temperatura (coincidentes con nuestro Modelfile)
export STORYCRAFTR_TEMPERATURE="0.82"
export STORYCRAFTR_MAX_TOKENS="4096"

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.."; pwd)"
SC_DIR="$ROOT/StoryCraftr"

# Preferimos el venv local de StoryCraftr
if [ -d "$SC_DIR/.venv" ]; then
    cd "$SC_DIR"
    exec "$SC_DIR/.venv/bin/storycraftr" "$@"
else
    echo "⚠️  StoryCraftr venv no encontrado en $SC_DIR/.venv"
    echo "   Procedemos con uv tool install global (si existe)..."
    exec storycraftr "$@"
fi
