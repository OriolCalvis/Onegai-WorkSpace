# 🔮 ONEGAI AI — Configuración Integral de Ollama + 2 Agentes

> **Proyecto**: Onegai (Escritura narrativa y campañas de rol DnD/tabletop)
> **Fecha de despliegue**: 11/08/2026
> **Autor**: Sistema de IA Local — SIN NUBE, 100% PRIVADO

---

## 🗂️ Tabla de Contenido
1. [Arquitectura](#arquitectura)
2. [Paso 1 — Instalación base y actualización Ollama](#paso-1--instalaci%C3%B3n-base-y-actualizaci%C3%B3n-ollama)
3. [Paso 2 — Modelos personalizados](#paso-2--modelos-personalizados-2-agentes)
4. [Paso 3 — Comunicación entre Ollama y los agentes (sin conflictos)](#paso-3--comunicaci%C3%B3n-entre-ollama-y-los-agentes-sin-conflictos)
5. [Paso 4 — Pruebas de funcionamiento](#paso-4--pruebas-de-funcionamiento)
6. [Paso 5 — Comandos diarios + troubleshooting](#paso-5--comandos-diarios--troubleshooting)
7. [Ruta de archivos](#ruta-de-archivos)

---

## 🏗️ Arquitectura

```
                            ┌───────────────────────────────────────┐
                            │            TU MAC (LOCAL)            │
                            │   SIN DATOS QUE SALGAN A INTERNET    │
                            │                                       │
┌──────────────────────┐    │  ┌─────────────────────────────┐    │
│   🖊  AGENTE 1        │    │  │   OLLAMA DAEMON             │    │
│   Story-Writer       │◄───┼──►   (launchd / ollama serve)  │    │
│   - deepseek-r1 8.2B │    │  │                             │    │
│   - Temp 0.82        │    │  │   OLLAMA_NUM_PARALLEL = 3   │    │
│   - 131 K contexto   │    │  │   OLLAMA_MAX_LOADED = 3     │    │
│   (Escritor creativo)│    │  │   OLLAMA_GPU_LAYERS = 999   │    │
└──────────────────────┘    │  │   OLLAMA_KEEP_ALIVE = 2h    │    │
                            │  │   OLLAMA_HOST=127.0.0.1:11434│    │
┌──────────────────────┐    │  └────────────┬────────────────┘    │
│   🔎 AGENTE 2         │    │               │ API REST             │
│   Author-Critic      │◄───┼───────────────►                      │
│   - gemma2 9B        │    │                                      │
│   - Temp 0.25        │    │  ┌──────────────────────────────┐    │
│   - 65536 ctx        │    │  │   AUTHORAGENT UI (p:3847)    │    │
│  (Crítica literaria) │    │  │  · Perfil Writer (p.defecto) │    │
└──────────────────────┘    │  │  · Perfil Critic (switcher)  │    │
                            │  │  · + 138 MD de Onegai index.│    │
                            │  └──────────────────────────────┘    │
                            └───────────────────────────────────────┘
```

**Ventajas de esta arquitectura**:
- **2 modelos diferentes** = 2 sistemas de personalidad SIN CONFUSIÓN (no comparten system prompt, ni KV-cache)
- **Paralelismo de 3** = se pueden ejecutar **Ambos agentes SIMULTÁNEAMENTE** sin bloqueos (ej: agente1 escribe un capítulo mientras agente2 revisa el anterior)
- **Privacidad 100%**: ni una palabra de tus historias sale de tu Mac.
- **Persistence**: Ollama arranca SOLO al encender el Mac (launchd plist) — no tienes que hacer nada.

---

## Paso 1 — Instalación base y actualización Ollama

### 1.1 Estado actual
| Componente | Versión | Detalle |
|---|---|---|
| macOS | Apple Silicon | (M1/M2/M3/M4 — aceleración METAL activada) |
| Ollama | **0.32.6** | ✅ Instalado vía `ollama.com/download` |
| Python | 3.14.6 | ✅ brew |
| Node.js | 25.2.1 | ✅ npm 11.6.2 |
| Homebrew | última | ✅ |
| Tesseract | 163 idiomas | ✅ (spa + cat + eng, para OCR del timeline) |

### 1.2 Comprobar / reinstalar Ollama
Solo si algo se rompe en el futuro:
```bash
# 1) Desinstalar versión antigua
rm -rf ~/.ollama /Applications/Ollama.app

# 2) Descargar última versión de Ollama oficial
curl -L https://ollama.com/download/Ollama-darwin.zip -o /tmp/Ollama.zip
open /tmp/Ollama.zip   # y arrastrar a Applications

# 3) Ejecutar Ollama.app una vez para que acepte seguridad
# 4) Comprobar
ollama --version
```

### 1.3 Instalación PERMANENTE del paralelismo (launchd)
Esto es **LO MÁS IMPORTANTE**: el fichero `com.onegai.ollama.plist` le dice a macOS que arranque Ollama con las variables de paralelismo correctas CADA VEZ que enciendas el ordenador.

**Despliegue (ya está hecho, documentado aquí para repetir si haces reset)**:
```bash
CFG="$HOME/Documents/Onegai 2/.onegai_ai_config"
DEST="$HOME/Library/LaunchAgents/com.onegai.ollama.plist"

launchctl unload "$DEST" 2>/dev/null          # descargar versión vieja
cp "$CFG/scripts/com.onegai.ollama.plist" "$DEST"
chmod 644 "$DEST"
launchctl load   "$DEST"                       # cargar en launchd
launchctl start  com.onegai.ollama             # arrancar AHORA
```

**Comprobar que funciona**:
```bash
curl -s http://localhost:11434/api/tags | python3 -m json.tool | head -20
# Deberías ver 5 modelos: deepseek-r1, gemma2:9b, llama3.1:8b, onegai/story-writer, onegai/author-critic
```

---

## Paso 2 — Modelos personalizados (2 agentes)

### Ubicación de los Modelfiles
```
.onegai_ai_config/modelfiles/
├── Modelfile.story-writer     ← Agente 1, base=deepseek-r1:latest
└── Modelfile.author-critic    ← Agente 2, base=gemma2:9b
```

### ¿Qué hay en cada Modelfile?

| Parámetro | Agente 1 (`onegai/story-writer`) | Agente 2 (`onegai/author-critic`) |
|---|---|---|
| **Base** | deepseek-r1 8.2B | gemma2 9B |
| **System prompt** | "Eres escritor creativo de Onegai... Canon primero ✅, estructura NIVEL 1/2/3/4, tests DnD 5e con CD, termina con 💡 Siguiente paso" | "Eres editor senior. CRÍTICA de 5 partes: 🔍 RESUMEN, ✅ POSITIVOS, ⚠️ PROBLEMAS (CRÍTICO/ALTO/MEDIO/BAJO) + 🧩 TABLA INCONSISTENCIAS + 📋 CHECKLIST 🎯 ACCIÓN INMEDIATA. Regla de los 3 PORQUÉ. NUNCA inventas." |
| `temperature`  | **0.82** 🎨 creativo pero controlado | **0.25** 🧊 muy objetivo |
| `top_p`        | 0.92                           | 0.85 |
| `repeat_penalty` | 1.18                        | 1.10 |
| `num_ctx`      | **131072** (131K — el máx. de deepseek) | **65536** (64K, crítico lee docs largos) |
| `num_predict`  | 4096 tokens / ~3000 palabras     | 3072 / ~2300 palabras |
| `stop`         | Etiquetas de pregunta, contexto     | `<\|end_of_text\|>`, etc. |

### Re-construir modelos (si cambias un Modelfile)
```bash
bash "$HOME/Documents/Onegai 2/.onegai_ai_config/scripts/build-models.sh"
```
Esto hará:
1. `ollama pull deepseek-r1:latest`
2. `ollama pull gemma2:9b` (fallback a llama3.1:8b si falla)
3. `ollama create onegai/story-writer  -f Modelfile.story-writer`
4. `ollama create onegai/author-critic -f Modelfile.author-critic` (con sed del placeholder)

---

## Paso 3 — Comunicación entre Ollama y los agentes (sin conflictos)

### 3.1 AutorAgent UI (Dashboard Web, recomendado para escribir)

#### Ubicación
```
AuthorAgent/
├── config/
│   ├── default.json            ← Perfil ACTIVO (por defecto = Writer)
│   ├── profile-writer.json     ← PERFIL 1 → onegai/story-writer,  t=0.82
│   └── profile-critic.json     ← PERFIL 2 → onegai/author-critic, t=0.25
└── .env                        ← AUTHENTICATOR_KEY + paths + idiomas
```

#### Cambiar perfil (MODO ESCRITOR ↔ MODO CRÍTICO):
```bash
CFG="$HOME/Documents/Onegai 2/.onegai_ai_config/scripts"

# MODO 1 — MODO ESCRITOR (creativo, para redactar):
bash $CFG/switch-authoragent-profile.sh writer

# MODO 2 — MODO CRÍTICO (objetivo, para revisar):
bash $CFG/switch-authoragent-profile.sh critic

# VER PERFIL ACTUAL:
bash $CFG/switch-authoragent-profile.sh status
```

> ⚠️ **Cada vez que cambias el perfil** tienes que reiniciar AuthorAgent porque Node.js no "relee" el json en caliente:
> ```bash
> bash ~/.../stop-agents.sh && bash ~/.../start-agents.sh
> ```

### 3.2 StoryCraftr CLI (modo terminal)
Wrapper script: `.onegai_ai_config/scripts/storycraftr.sh`
```bash
SC_SCRIPT="$HOME/Documents/Onegai 2/.onegai_ai_config/scripts/storycraftr.sh"

bash $SC_SCRIPT --help
bash $SC_SCRIPT init "Nueva novela en Onegai: Aegroum Saga"
bash $SC_SCRIPT worldbuilding
bash $SC_SCRIPT outline
bash $SC_SCRIPT write chapter1
bash $SC_SCRIPT chat          # chatear con el writer directamente
```
Se conecta **también** a `onegai/story-writer:latest` con los mismos parámetros.

### 3.3 Prueba manual RÁPIDA de que el paralelismo funciona
```bash
# (Terminal 1) Consulta larga al Agente 1 (escribe un cuento):
curl -s http://localhost:11434/api/generate \
  -H 'Content-Type: application/json' \
  -d '{"model":"onegai/story-writer","prompt":"Escribe un cuento corto de 3 páginas sobre Venides en Boundinghton.","stream":false,"options":{"num_predict":3000}}'

# (Terminal 2, EN PARALELO, sin esperar a que acabe Terminal 1)
curl -s http://localhost:11434/api/generate \
  -H 'Content-Type: application/json' \
  -d '{"model":"onegai/author-critic","prompt":"Resume este texto en 3 puntos: ...","stream":false}'

# Si Terminal 2 empieza a responder SIN ESPERAR a Terminal 1 = paralelismo OK 🎉
```

---

## Paso 4 — Pruebas de funcionamiento

Tenemos 2 baterías de test:

### 4.1 Test RÁPIDO (~2-5 min) — el día a día
```bash
python3 "$HOME/Documents/Onegai 2/.onegai_ai_config/scripts/tests-rapidos.py"
```
Prueba:
- **A1-A2**: Agente1 identidad + Outline Nivel 3 con CD DnD
- **B1-B2**: Agente2 estructura crítica 5 puntos + detección contradicciones de canon
- **C1-C3**: Paralelismo SIMULTÁNEO 2 modelos sin confusión de personalidad
- **D x5**: Alternancia rápida 1→2→1→2→1 para confirmar no se mezclan

### 4.2 Test EXHAUSTIVO (~10-25 min) — instalación inicial / cambios mayores
```bash
bash "$HOME/Documents/Onegai 2/.onegai_ai_config/scripts/test-agents.sh"
```
14 tests con estructura TAP: 3 de writer, 3 de crítico, 3 de paralelo, 5 de alternancia. ≥ 80% = estable.

### 4.3 Dashboard de VIDA REAL (más útil que cualquier test)
```bash
bash "$HOME/Documents/Onegai 2/.onegai_ai_config/scripts/status-agents.sh"
```
Te imprime en color:
1. Estado API Ollama + modelos instalados
2. Check `onegai/story-writer:latest` y `onegai/author-critic:latest`
3. Health check de AuthorAgent en http://localhost:3847
4. RAM del sistema + VRAM usada por Ollama (`ollama ps`)
5. Últimos 5 errores de cada log

---

## Paso 5 — Comandos diarios + Troubleshooting

### 🎯 ARRANQUE RÁPIDO (lo único que tienes que hacer cada mañana)
```bash
bash "$HOME/Documents/Onegai 2/.onegai_ai_config/scripts/start-agents.sh"
```
Hace TODO: levanta Ollama si no está, hace warm-up de los 2 modelos, arranca AuthorAgent UI en http://localhost:3847.

### 🛑 PARADA DE SEGURIDAD
```bash
bash "$HOME/Documents/Onegai 2/.onegai_ai_config/scripts/stop-agents.sh"
```

### 📋 RESUMEN RÁPIDO (status)
```bash
bash "$HOME/Documents/Onegai 2/.onegai_ai_config/scripts/status-agents.sh"
```

---

### ❓ Problemas comunes y soluciones

#### Problema 1: `curl: (7) Failed to connect to localhost port 11434: Connection refused`
**Causa**: Ollama daemon no está corriendo o el plist no lo arrancó.
**Solución**:
```bash
launchctl kickstart -k gui/$UID/com.onegai.ollama
# Espera 10s, verifica
sleep 10 && curl -s http://localhost:11434/api/tags | head -c 500
```
Si sigue sin funcionar:
```bash
# Mira los logs directamente
tail -30 "$HOME/Documents/Onegai 2/.onegai_ai_config/logs/ollama.stderr.log"
# Plan B: arrancar manualmente en una Terminal
OLLAMA_NUM_PARALLEL=3 OLLAMA_MAX_LOADED_MODELS=3 OLLAMA_GPU_LAYERS=999 ollama serve
```

#### Problema 2: "Mis respuestas son muy lentas (> 60s para una frase)"
**Causa A**: Modelo no estaba cargado en RAM (fue expulsado por `keep_alive`).
  Solución: `OLLAMA_KEEP_ALIVE="8h" ollama run onegai/story-writer ""` (hace un warm-up y lo deja 8h).
**Causa B**: Aceleración METAL/GPU desactivada.
  Verifica con: `ollama ps` debería ver "gpu" en la columna de tamaño. Si no, exporta `OLLAMA_GPU_LAYERS=999` y reinicia.
**Causa C**: Estás pidiendo `num_ctx=131072` en gemma2 (que solo acepta 8K).
  Solución: cada modelo usa su `num_ctx` declarado en el Modelfile.

#### Problema 3: "El escritor responde como crítico o al revés" — confusión de personalidad
**Causa**: Has cambiado el perfil de AuthorAgent pero no reiniciaste el server.
**Fix garantizado**:
```bash
CFG="$HOME/Documents/Onegai 2/.onegai_ai_config/scripts"
bash $CFG/switch-authoragent-profile.sh writer   # (o critic)
bash $CFG/stop-agents.sh
bash $CFG/start-agents.sh
```
**Prueba final rápida** de que el perfil es correcto:
```bash
# Ver que modelo tiene AuthorAgent apuntando en config/default.json:
python3 -c "import json;print(json.load(open('$HOME/Documents/Onegai 2/AuthorAgent/config/default.json'))['ai']['ollama']['model'])"
# Y prueba manual:
curl -s http://localhost:11434/api/generate -H 'Content-Type: application/json' \
  -d '{"model":"onegai/story-writer","stream":false,"options":{"num_predict":80},"prompt":"¿Quién eres? Contesta en 1 frase."}' \
  | python3 -c "import sys,json;d=json.load(sys.stdin);r=d['response'];print(r.split('</think>')[-1].strip() if '</think>' in r else r)"
```

#### Problema 4: "No hay suficiente RAM" o "Killed 9" al cargar 2 modelos a la vez
Memoria MÍNIMA recomendada:
- deepseek-r1 8.2B q4_K_M = ~5.2 GB
- gemma2 9B q4_K_M = ~5.4 GB
- **Total ~10.6 GB** + KV cache (~1.5 GB) + macOS (~4-6 GB) = **18 GB RAM física MÍNIMA** para tener 2 modelos en paralelo.

Si tienes 16 GB o menos:
- Opción 1: baja `OLLAMA_MAX_LOADED_MODELS=1` en el plist → solo cargará uno a la vez (habrá un delay de ~5s al cambiar de perfil)
- Opción 2: usa modelos más pequeños (ej: `llama3.2:3b` en lugar de gemma2:9b para el crítico)

Para editarlo a mano:
```bash
plutil -replace ProgramArguments -string "...OLLAMA_MAX_LOADED_MODELS=1..."  \
  "$HOME/Library/LaunchAgents/com.onegai.ollama.plist"
launchctl unload "$HOME/Library/LaunchAgents/com.onegai.ollama.plist"
launchctl load   "$HOME/Library/LaunchAgents/com.onegai.ollama.plist"
```

#### Problema 5: AuthorAgent no arranca (Error EADDRINUSE 3847)
Puerto 3847 ocupado:
```bash
# Matar el proceso que lo ocupa
lsof -ti:3847 | xargs kill -9 2>/dev/null
# Y volver a iniciar
bash ~/.onegai_ai_config/scripts/start-agents.sh
```

#### Problema 6: Modelo "not found" después de `ollama create`
Asegúrate de usar el nombre EXACTO (`onegai/story-writer:latest` o `onegai/author-critic:latest`):
```bash
ollama list | grep onegai   # Ver los tags exactos
# Si no aparecen, vuelve a hacer build:
bash "$HOME/Documents/Onegai 2/.onegai_ai_config/scripts/build-models.sh"
```

---

## 📁 Ruta de archivos (para localizar TODO rápido)

```
/Users/admin/Documents/Onegai 2/
│
├── 🧠 .onegai_ai_config/                      ← CONFIGURACIÓN MAESTRA (todo está aquí)
│   ├── modelfiles/
│   │   ├── Modelfile.story-writer            ← Prompt Agente 1 (escritor)
│   │   └── Modelfile.author-critic           ← Prompt Agente 2 (crítico)
│   ├── scripts/
│   │   ├── 🟢 start-agents.sh                ← ⭐ ARRANQUE CADA MAÑANA
│   │   ├── 🔴 stop-agents.sh                 ← Parada total
│   │   ├── 🔵 status-agents.sh               ← Dashboard
│   │   ├── 🟡 build-models.sh                ← Reconstruye modelos custom
│   │   ├── 🟣 switch-authoragent-profile.sh  ← writer / critic / status
│   │   ├── tests-rapidos.py                  ← Batería test rápida (~3 min)
│   │   ├── test-agents.sh                    ← Batería test exhaustiva
│   │   ├── storycraftr.sh                    ← Wrapper CLI StoryCraftr → Agente1
│   │   └── com.onegai.ollama.plist           ← (Plantilla) Copiada a ~/Library/LaunchAgents
│   └── logs/                                  ← stdout / stderr de TODO (para debugging)
│
├── 📚 AuthorAgent/                            ← Aplicación Node UI
│   ├── config/
│   │   ├── default.json                       ← PERFIL ACTIVO (lo escribe switch-*.sh)
│   │   ├── profile-writer.json                ← Plantilla: MODO ESCRITOR
│   │   └── profile-critic.json                ← Plantilla: MODO CRÍTICO
│   └── .env                                   ← Claves (vacías = 100% local) + paths + idioma
│
├── 📖 StoryCraftr/                            ← (WIP) Aplicación CLI Python
│
├── 🌍 Onegai/OnegaiTimeLine/                  ← TÚ LORE (138 archivos .md)
│   └── Campanyes/Canon/Aegroum/Campanya 1 Boundinghton
│
├── 🕒 onegai timeline.pdf                     ← 274 páginas visuales (imagenes, OCR hecho)
├── 🕒 onegai timeline.pptx                    ← Identica fuente al .docx
└── 🕒 onegai timeline.docx                    ← 274 JPEG incrustados (OCR realizado)
                                                    ↳ Resultado en:
                                                    AuthorAgent/workspace/library/Onegai_Timeline_OCR_DOCX.md
```

---

### Contacto / Reconstrucción total desde cero (si haces reset de máquina)
```bash
# En orden:
1) brew install uv node@25 tesseract tesseract-lang python@3.14
2) Descargar Ollama: https://ollama.com/download
3) Descomprimir los 3 zips del proyecto (Onegai/ + AuthorAgent/ + StoryCraftr/)
4) cp  desde backup hasta:  .onegai_ai_config/
5) bash .onegai_ai_config/scripts/build-models.sh
6) Instalar plist (Pasos 1.3 arriba)
7) bash .onegai_ai_config/scripts/start-agents.sh
8) python3 .onegai_ai_config/scripts/tests-rapidos.py
```
✅ Fin. Todo funciona.
