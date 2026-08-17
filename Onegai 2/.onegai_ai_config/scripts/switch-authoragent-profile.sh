#!/usr/bin/env bash
# ================================================================
# CAMBIAR PERFIL DE AUTHORAGENT (modo escritor o modo crítico)
# USO:
#   bash switch-authoragent-profile.sh writer   → perfil story-writer
#   bash switch-authoragent-profile.sh critic   → perfil author-critic
#   bash switch-authoragent-profile.sh status   → ver perfil actual
# ================================================================
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.."; pwd)"
AA_CFG="$ROOT/AuthorAgent/config"
LOGS="$ROOT/.onegai_ai_config/logs"
C_G='\033[0;32m'; C_Y='\033[1;33m'; C_R='\033[0;31m'; C_0='\033[0m';

WHAT=${1:-status}
case "$WHAT" in
    writer|w|1)
        PROFILE="profile-writer.json"; NICE="🖊  MODO ESCRITOR (Story-Writer, t=0.82, deepseek-r1 8.2B)"
        ;;
    critic|c|2)
        PROFILE="profile-critic.json"; NICE="🔎 MODO CRÍTICO (Author-Critic, t=0.25, gemma2 9B)"
        ;;
    status|s)
        echo "Perfil actual de default.json:"
        python3 -c "
import json
p = json.load(open('$AA_CFG/default.json'))
pf = p.get('_onegai_profile','?')
desc = p.get('_onegai_description','')
print(f'  Profile tag : {pf}')
print(f'  Model Ollama: {p[\"ai\"][\"ollama\"][\"model\"]}')
print(f'  Temp.       : {p[\"ai\"][\"defaultTemperature\"]}')
print(f'  Descripció  : {desc}')"
        exit 0
        ;;
    *)
        echo "Uso: $0 [writer|critic|status]"
        exit 2
esac

[ -f "$AA_CFG/$PROFILE" ] || { echo "No existe $AA_CFG/$PROFILE"; exit 3; }

# Guardar backup antes de cambiar
cp "$AA_CFG/default.json" "$AA_CFG/.default.previous.json" 2>/dev/null || true
cp "$AA_CFG/$PROFILE" "$AA_CFG/default.json"

# Si AuthorAgent está corriendo, le indicamos que recargue (kill -HUP suave si es Node con nodemon,
# o simplemente indicamos al usuario que reinicie). La mayoría de servers Node no recargan config
# en caliente sin código específico, así que avisamos.
echo
printf "${C_G}✅ Perfil cambiado a:${C_0}  $NICE\n"
echo "   Modelo Ollama activo en AuthorAgent: $(python3 -c 'import json;print(json.load(open("'"$AA_CFG/default.json"'"))["ai"]["ollama"]["model"])')"
echo
# Avisar de reinicio
if curl -sf http://localhost:3847/api/system/health >/dev/null 2>&1; then
    printf "${C_Y}⚠️  AuthorAgent está CORRIENDO. Para aplicar el cambio, reinícialo:${C_0}\n"
    printf "     bash $ROOT/.onegai_ai_config/scripts/stop-agents.sh\n"
    printf "     bash $ROOT/.onegai_ai_config/scripts/start-agents.sh\n"
else
    echo "AuthorAgent no está activo — en el siguiente start tomará el nuevo perfil."
fi
