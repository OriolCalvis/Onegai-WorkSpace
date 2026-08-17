#!/usr/bin/env bash
# ================================================================
# TEST EXHAUSTIVOS DE LOS DOS AGENTES ESPECIALIZADOS
# Batería de pruebas:
#   Test A: Agente 1 (Story-Writer) solo → 3 preguntas
#   Test B: Agente 2 (Author-Critic) solo → 3 preguntas
#   Test C: AMBOS agentes en PARALELO (simultáneos) → validar que
#          Ollama gestiona las 2 peticiones sin conflictos
#   Test D: 5 consultas alternadas (1→2→1→2→1) para verificar
#          que no se mezclan system prompts ni personalidades
# ================================================================
set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.."; pwd)"
CFG="$ROOT/.onegai_ai_config"
LOGS="$CFG/logs"
mkdir -p "$LOGS"

C_G='\033[0;32m'; C_R='\033[0;31m'; C_Y='\033[1;33m';
C_B='\033[1;34m'; C_W='\033[1;37m'; C_0='\033[0m';
PASS=0; FAIL=0; TOTAL=0
RESULTS=()

# Función: hacer una petición a Ollama y devolver SOLO el texto limpio
# Args: <model> <prompt> <timeout_seg> <nombre_test>
ollama_ask(){
    local MODEL="$1"; local PROMPT="$2"; local TIMEOUT="$3"; local NAME="$4"
    local OUT="$LOGS/${NAME}.json"
    local ERR="$LOGS/${NAME}.err"
    python3 - "$MODEL" "$PROMPT" "$TIMEOUT" "$OUT" "$ERR" <<'PY'
import json, urllib.request, sys
model, prompt, timeout, out_file, err_file = sys.argv[1:6]
data = json.dumps({
    "model": model,
    "prompt": prompt,
    "stream": False,
    "options": {"temperature": 0.4, "num_predict": 256, "num_ctx": 8192}
}).encode("utf-8")
try:
    req = urllib.request.Request(
        "http://localhost:11434/api/generate",
        data=data, headers={"Content-Type":"application/json"}
    )
    with urllib.request.urlopen(req, timeout=int(timeout)) as r:
        body = r.read().decode("utf-8")
        open(out_file,"w").write(body)
        d = json.loads(body)
        resp = d.get("response","")
        if "</think>" in resp: resp = resp.split("</think>",1)[-1].strip()
        # Print clean response
        sys.stdout.write(resp)
except Exception as e:
    open(err_file,"w").write(str(e))
    sys.stderr.write(f"ERROR: {e}\n")
    sys.exit(2)
PY
}

# Registrar test
registra(){
    TOTAL=$((TOTAL+1))
    local nombre=$1
    local exito=$2      # "OK" o "FAIL"
    local detalle=$3
    if [ "$exito" = "OK" ]; then
        PASS=$((PASS+1)); COLOR=$C_G; ICONO="✔";
    else
        FAIL=$((FAIL+1)); COLOR=$C_R; ICONO="✖";
    fi
    RESULTS+=("${COLOR}${ICONO} Test #${TOTAL} — ${nombre}: ${detalle}${C_0}")
    printf "${COLOR}${ICONO} Test #${TOTAL}${C_0} — ${C_W}${nombre}${C_0}: ${detalle}\n"
}

# ======================= PRE-CHECK =======================
printf "${C_B}═══════════════════════════════════════════════════════${C_0}\n"
printf "${C_B}          TEST EXHAUSTIVO — ONEGAI AI AGENTS          ${C_0}\n"
printf "${C_B}═══════════════════════════════════════════════════════${C_0}\n"
echo

# Verificar que Ollama está
if ! curl -sf http://localhost:11434/api/tags >/dev/null 2>&1; then
    echo "Ollama no corriendo. Antes de testear: bash $CFG/scripts/start-agents.sh"
    exit 1
fi
# Verificar modelos
MISSING=""
for M in onegai/story-writer onegai/author-critic; do
    if ! ollama list 2>/dev/null | grep -q "^$M "; then
        MISSING="$MISSING $M"
    fi
done
if [ -n "$MISSING" ]; then
    echo "⚠️  Faltan modelos personalizados: $MISSING"
    echo "   Corre:  bash $CFG/scripts/build-models.sh"
fi

echo
# ======================= TEST A =======================
printf "${C_Y}[TEST A] AGENTE 1 SOLO — onegai/story-writer (3 preguntas)${C_0}\n"
echo

OUT1=$(ollama_ask "onegai/story-writer" \
          "Describe brevemente tu propósito como Agente 1 (Story-Writer de Onegai). Responde en 2 frases CORTAS en español y ACABA con la frase exacta '💡 Siguiente paso sugerido:'. Muéstrame SOLO las frases, nada más." \
          120 "tA1")
if [ -n "$OUT1" ] && echo "$OUT1" | grep -q "Siguiente paso sugerido"; then
    registra "A1 — Writer declara identidad + 💡 final" "OK" "Responde con estructura pedida"
else
    registra "A1 — Writer declara identidad + 💡 final" "FAIL" "Output: $(echo "$OUT1" | head -2 | tr '\n' '|')"
fi

OUT2=$(ollama_ask "onegai/story-writer" \
          "Crea en 10 líneas un outline de NIVEL 3 (scenes) para una escena del Capítulo 3 de la campaña Boundinghton: los personajes Venides y Skilla llegan al Pico Dragón a medianoche y escuchan un ruido. Incluye tests de habilidad con CD sugeridos (DnD)." \
          180 "tA2")
if echo "$OUT2" | grep -qiE "(CD|Scene|Escena|Percepci|Perception|Atletism)"; then
    registra "A2 — Outline Nivel 3 escena Pico Dragón + CD DnD" "OK" "Contiene estructura escenas + tests"
else
    registra "A2 — Outline Nivel 3 escena Pico Dragón + CD DnD" "FAIL" "No se detectó estructura. Primeras 4 líneas: $(echo "$OUT2" | head -4 | tr '\n' ' / ')"
fi

OUT3=$(ollama_ask "onegai/story-writer" \
          "Imagina un diálogo de 3-4 turnos entre Venides (idealista) y Skilla (escéptica) en el bosque. Usa la regla 'mostrar no decir'." \
          180 "tA3")
NDIAL=$(echo "$OUT3" | grep -ciE '["«»].+["«»]|— [A-Za-z]+')
if [ "$NDIAL" -ge 3 ]; then
    registra "A3 — Diálogo Venides-Skilla ≥3 turnos" "OK" "Detectados $NDIAL turnos de diálogo"
else
    registra "A3 — Diálogo Venides-Skilla ≥3 turnos" "FAIL" "Detectados $NDIAL turnos. Output: $(echo "$OUT3" | head -3)"
fi
echo

# ======================= TEST B =======================
printf "${C_Y}[TEST B] AGENTE 2 SOLO — onegai/author-critic (3 preguntas)${C_0}\n"
echo

OUT4=$(ollama_ask "onegai/author-critic" \
          "Tu material de contexto es este TEXTO MALO:
          'Venides entró en la taberna. Tenía sed. Bebió agua. Luego se fue. Todo fue normal y no pasó nada. El final.'
          Analízalo siguiendo tus 5 normas de crítico (🔍 RESUMEN, ✅ POSITIVOS, ⚠️ PROBLEMAS por gravedad con CITAS, 🧩 Tabla inconsistencias, 📋 CHECKLIST final + 🎯 ACCIÓN INMEDIATA)." \
          240 "tB1")
# Debe tener al menos 3 de los 5 encabezados clave
MATCH=0
echo "$OUT4" | grep -q "RESUMEN EJECUTIVO\|🔍"   && MATCH=$((MATCH+1))
echo "$OUT4" | grep -q "POSITIVOS\|ASPECTOS POSITIVOS\|✅" && MATCH=$((MATCH+1))
echo "$OUT4" | grep -q "PROBLEMAS\|⚠️"             && MATCH=$((MATCH+1))
echo "$OUT4" | grep -q "INCONSISTEN\|🧩\|LACUNA\|contradict" && MATCH=$((MATCH+1))
echo "$OUT4" | grep -q "CHECKLIST\|COMPROBACI\|📋\|ACCION\|🎯" && MATCH=$((MATCH+1))
if [ "$MATCH" -ge 3 ]; then
    registra "B1 — Crítica con estructura 5+ apartados" "OK" "Detectados $MATCH/5 apartados clave"
else
    registra "B1 — Crítica con estructura 5+ apartados" "FAIL" "Solo $MATCH/5. Output head: $(echo "$OUT4" | head -3)"
fi

OUT5=$(ollama_ask "onegai/author-critic" \
          "Texto a analizar: 'Capítulo 1 dice que Boundinghton fue fundada en el año 930. Capítulo 3 dice que en el 930 aún era un pueblo pesquero llamado Surysal. Capítulo 5 dice que Skilla tiene 19 años. Unos párrafos más tarde dice que nació hace 22 inviernos.' Señala INCONSISTENCIAS DE CANON en una tabla y marca el nivel de gravedad." \
          180 "tB2")
# Debe mencionar al menos 2 de las 3 contradicciones obvias
ERR=0
echo "$OUT5" | grep -qE "930|fundad|Surysal"     && ERR=$((ERR+1))
echo "$OUT5" | grep -qE "19|22|años|invierno"    && ERR=$((ERR+1))
if [ "$ERR" -ge 2 ]; then
    registra "B2 — Detecta contradicciones de canon (Boundinghton 930 / edad Skilla)" "OK" "Detectadas $ERR/2 contradicciones"
else
    registra "B2 — Detecta contradicciones de canon" "FAIL" "Solo $ERR. Output: $(echo "$OUT5" | head -5)"
fi

OUT6=$(ollama_ask "onegai/author-critic" \
          "Aplica la REGLA DE LOS 3 POR QUÉ a este giro: 'Los Perdidos atacan la fiesta de disfraces de Boundinghton.' Explica los 3 porqués." \
          180 "tB3")
# Detectar los 3 porqués
PORQ=0
echo "$OUT6" | grep -qE "POR QUÉ|por qué|¿Por|1\).*ahora|2\).*personaje|3\).*manera" && PORQ=$((PORQ+1))
echo "$OUT6" | grep -qE "ahora y no antes|antes|tiempo|momento" && PORQ=$((PORQ+1))
echo "$OUT6" | grep -qE "personaje|quién|por qué ESTE|otro" && PORQ=$((PORQ+1))
echo "$OUT6" | grep -qE "manera|forma|por qué DE ESTA" && PORQ=$((PORQ+1))
if [ "$PORQ" -ge 2 ]; then
    registra "B3 — Regla de los 3 POR QUÉ en ataque fiesta" "OK" "$PORQ pilares detectados"
else
    registra "B3 — Regla de los 3 POR QUÉ" "FAIL" "$PORQ pilares. Output head: $(echo "$OUT6" | head -3)"
fi
echo

# ======================= TEST C =======================
printf "${C_Y}[TEST C] PARALELO — 2 peticiones SIMULTÁNEAS a DISTINTOS modelos${C_0}\n"
echo "(Esto valida que Ollama no se cuelga y que no se mezclan personalidades)"
echo
START=$(date +%s)
# Llamadas simultáneas. Cada una guarda su salida.
(ollama_ask "onegai/story-writer" \
    "Escribe SOLO la frase exacta 'SOY EL AGENTE 1: STORY-WRITER DE ONEGAI.' Nada más." \
    180 "tC_writer" > "$LOGS/paralelo_writer.txt") &
WPID=$!
(ollama_ask "onegai/author-critic" \
    "Escribe SOLO la frase exacta 'SOY EL AGENTE 2: CRÍTICO LITERARIO DE ONEGAI.' Nada más." \
    180 "tC_critic" > "$LOGS/paralelo_critic.txt") &
CPID=$!
wait "$WPID" "$CPID" 2>/dev/null || true
END=$(date +%s)
DIFF=$(( END - START ))

WRITER_OUT=$(head -1 "$LOGS/paralelo_writer.txt" 2>/dev/null | tr -d '\n\t ')
CRITIC_OUT=$(head -1 "$LOGS/paralelo_critic.txt" 2>/dev/null | tr -d '\n\t ')

# Validar personalidad no mezclada: writer NO debe decir CRITICO, y viceversa
if echo "$WRITER_OUT" | grep -qi "AGENTE 1\|STORY\|WRITER" \
   && ! echo "$WRITER_OUT" | grep -qi "CRÍTICO\|CRITICO\|AGENTE 2"; then
    registra "C1 — Paralelo: Agente1 NO se disfraza de Agente2" "OK" "Salida: $WRITER_OUT"
else
    registra "C1 — Paralelo: Agente1 NO se disfraza de Agente2" "FAIL" "Salida: $WRITER_OUT"
fi
if echo "$CRITIC_OUT" | grep -qi "AGENTE 2\|CRÍTICO\|CRITICO" \
   && ! echo "$CRITIC_OUT" | grep -qi "WRITER\|AGENTE 1\|STORY"; then
    registra "C2 — Paralelo: Agente2 NO se disfraza de Agente1" "OK" "Salida: $CRITIC_OUT"
else
    registra "C2 — Paralelo: Agente2 NO se disfraza de Agente1" "FAIL" "Salida: $CRITIC_OUT"
fi
if [ "$DIFF" -lt 300 ]; then
    registra "C3 — Tiempo total razonable (< 5 min)" "OK" "${DIFF}s"
else
    registra "C3 — Tiempo total razonable" "FAIL" "${DIFF}s (muy lento, revisar GPU offload)"
fi
echo

# ======================= TEST D =======================
printf "${C_Y}[TEST D] ALTERNANCIA (1→2→1→2→1) — 5 preguntas seguidas${C_0}\n"
echo "Valida que cambiar de modelo 5 veces no rompe Ollama ni mezcla system prompts"
echo

declare -a ORDER=("writer" "critic" "writer" "critic" "writer")
declare -a NAMES=("D1" "D2" "D3" "D4" "D5")
ALT_OK=0
for i in 0 1 2 3 4; do
    MOD=${ORDER[$i]}; NM=${NAMES[$i]}
    case $MOD in
        writer) MODEL="onegai/story-writer"; EXPECT="Agente 1|Escritor|Writer|Siguiente paso"
                PROMPT="Dime tu nombre y tu objetivo en UNA FRASE. No te inventes nada.";;
        critic) MODEL="onegai/author-critic"; EXPECT="Agente 2|Crítico|Critic|Resumen ejecutivo|CHECKLIST|PROBLEMAS"
                PROMPT="Dime tu nombre y tu objetivo en UNA FRASE. No te inventes nada.";;
    esac
    OUT=$(ollama_ask "$MODEL" "$PROMPT" 180 "t${NM}")
    if echo "$OUT" | grep -qiE "$EXPECT"; then
        registra "${NM} — Alternancia $((i+1))/5 (${MODEL#onegai/})" "OK" "Frase detectada"
        ALT_OK=$((ALT_OK+1))
    else
        registra "${NM} — Alternancia $((i+1))/5 (${MODEL#onegai/})" "FAIL" "Output: $(echo "$OUT" | head -2 | tr '\n' ' ')"
    fi
done
echo

# ======================= RESUMEN =======================
printf "${C_W}═══════════════════════════════════════════════════════${C_0}\n"
printf "${C_W}                RESUMEN DE TESTS                        ${C_0}\n"
printf "${C_W}═══════════════════════════════════════════════════════${C_0}\n"
for l in "${RESULTS[@]}"; do printf "  %b\n" "$l"; done
echo
printf "${C_W}Tests ejecutados : %2d${C_0}\n" "$TOTAL"
printf "${C_G}Pasados          : %2d${C_0}\n" "$PASS"
printf "${C_R}Fallidos         : %2d${C_0}\n" "$FAIL"
PCT=$(( TOTAL>0 ? PASS*100/TOTAL : 0 ))
if [ "$PCT" -ge 80 ]; then
    printf "${C_G}ÉXITO TOTAL      : %2d%%  ✅ CONFIGURACIÓN ESTABLE${C_0}\n" "$PCT"
elif [ "$PCT" -ge 50 ]; then
    printf "${C_Y}ÉXITO PARCIAL    : %2d%%  — leer tests fallidos y afinar prompts${C_0}\n" "$PCT"
else
    printf "${C_R}MUCHOS FALLOS    : %2d%% — revisar modelos / builds${C_0}\n" "$PCT"
fi
printf "${C_W}═══════════════════════════════════════════════════════${C_0}\n"
printf "Logs detallados de cada request JSON en:  $LOGS/\n"
