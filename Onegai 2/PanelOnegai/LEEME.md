# 🧠 Panel Onegai · Integrado con tus 2 agentes Ollama

**Un único sitio desde el que mandar encargos a tus dos agentes personalizados (Escritor Creativo + Crítico Literario) y gestionar todo el stack IA.**

---

## 🚀 Cómo abrirlo

Opción 1 (más fácil): **doble clic en `Abrir Panel Onegai.command`**. Se abre el navegador en `http://127.0.0.1:4100`.

Opción 2 (Terminal):

```bash
cd "/Users/admin/Documents/Onegai 2/PanelOnegai"
python3 server.py
```

Para apagarlo, cierra la ventana de Terminal que se abrió.

---

## 🎯 Novedad: Dos agentes personalizados

En la **barra superior** tienes un selector de **Modo Agente**:

| Botón | Agente | Modelo Ollama | Temperatura | Mejor para |
|---|---|---|---|---|
| ✍️ **Escritor** | `onegai/story-writer:latest` | DeepSeek-R1 8.2B · 131K ctx | **0.82** | Redactar capítulos, fichas CD DnD, outlines campaña, escenas con misterio Boundinghton |
| 🧐 **Crítico** | `onegai/author-critic:latest` | Gemma2 9B · crítica estructurada | **0.25** | Revisar estilo + coherencia, detectar incoherencias de canon, checklist, tabla 3 cols, regla 3 porqués |

**Este selector:**
- Cambia automáticamente el modelo seleccionado en la pestaña **Ollama directo**.
- Aplica el perfil correspondiente a **AuthorAgent** (config JSON). Después haz clic en **Reiniciar** AuthorAgent para que lo cargue.
- Carga **presets específicos** para cada agente (botones de tareas).

---

## 🎛 Las 4 pestañas

### 1. 🧩 AuthorAgent
Encargo en lenguaje normal. **Lee los documentos marcados a la izquierda** (texto real, no inventará nada).
- **Arrancar / Parar / Reiniciar** el servidor Node UI (puerto 3847)
- **Perfil rápido AuthorAgent**: `Aplicar ✍️ Escritor` o `Aplicar 🧐 Crítico` — cambia el `default.json` y te pide reiniciar
- **Presets** (varían según perfil activo): capítulo CD DnD, ficha personaje, outline campaña, ampliar escena, crítica 5pts, inconsistencias canon…
- **Abrir su panel** → `http://127.0.0.1:3847` (proyectos completos, revisiones, exportación)

### 2. ⚡ Ollama directo
El mismo encargo, pero **directo al modelo local** sin la maquinaria de AuthorAgent en medio. Más rápido, más predecible para correcciones concretas.
- **Selector de modelo** — tus modelos `onegai/*` aparecen **PRIMEROS** automáticamente
- **Botones rápidos**: `✍️ Escritor` / `🧐 Crítico` — saltan al modelo del perfil
- **Presets dinámicos**: cambian según perfil/modelo seleccionado

### 3. 📚 StoryCraftr
Comandos CLI concretos sobre el proyecto, ejecutados dentro de la carpeta `Onegai`.
- Presets: `esquema` / `personajes` / `historia` / `coherencia`

### 4. ⚙️ Stack IA (NUEVA)
Gestiona **Ollama + AuthorAgent simultáneamente** usando los scripts de `.onegai_ai_config/scripts/`.

- **▶️ Arrancar Stack IA** → `start-agents.sh` (variables paralelismo + warmup modelos + arranca AuthorAgent)
- **⏹️ Parar Stack IA** → `stop-agents.sh` (unload modelos `keep_alive=0` + para AuthorAgent)
- **🔍 Comprobar estado detallado** → `status-agents.sh` (dashboard colores: 5 checks health, modelos, RAM/VRAM, `ollama ps`, últimas líneas de logs)
- **📋 Resumen chips** → muestra el estado compacto en el panel Registro

---

## 📂 Lo primero: marcar documentos

A la izquierda tienes la lista de todo lo legible del proyecto — 251 archivos, incluidos los 88 `.docx` de THE ONEGAI PROJECT. **Marca los que quieres que el agente lea antes de mandarle el encargo.** El panel abre esos archivos, saca el texto y lo mete dentro del mensaje (bloque `### Archivo: ruta`).

- **Por nombre** / **Dentro del texto**: dos modos de búsqueda (el segundo escribe + Enter, hace búsqueda full-text)
- **Marcar lo visible**: útil para filtrar "campaña" y marcar 20 documentos con un clic
- **Contexto deslizador**: límite caracteres por documento (40.000 por defecto, hasta 120.000)

### Todo junto o uno por uno

| Modo | Cuándo usarlo |
|---|---|
| **Todo junto** (defecto) | 1-4 documentos — una sola llamada al modelo |
| **Uno por uno** | 5+ documentos — un pase completo por cada archivo (sin competir por espacio), y opcionalmente un pase final que junta resultados y señala contradicciones entre ellos |

Las respuestas se guardan solas en la carpeta **salidas**, en markdown, con la lista de documentos que se usaron.

---

## 🛠 Nuevos endpoints API (para scripts)

El `server.py` ahora expone estas rutas extra (además de las originales):

| Método | Ruta | Descripción |
|---|---|---|
| GET | `/api/status` | ahora incluye `authoragent_profile`, `ollama_alive`, `agent_models`, `ai_config`, `ai_scripts` + modelos **onegai/* ordenados PRIMERO** |
| GET | `/api/profiles` | devuelve `profiles` (con presets específicos), `current`, `agent_models`, `models_prioritized` |
| GET | `/api/stack/status` | ejecuta `status-agents.sh` y devuelve `{output}` (sin ANSI) |
| POST | `/api/stack/start` | ejecuta `start-agents.sh` (timeout 15 min) |
| POST | `/api/stack/stop` | ejecuta `stop-agents.sh` |
| POST | `/api/authoragent/restart` | parar + esperar 2s + arrancar AuthorAgent |
| POST | `/api/authoragent/switch-profile` | body `{profile: "writer" \| "critic" \| "status"}`. Cambia `AuthorAgent/config/default.json` usando el script bash |

---

## ✅ Requisitos previos (comprobación rápida)

1. **Ollama corriendo** (debe haber 5 modelos: `deepseek-r1`, `gemma2:9b`, `llama3.1:8b`, `onegai/story-writer`, `onegai/author-critic`)
   - Si no → pestaña **⚙️ Stack IA** → **▶️ Arrancar Stack IA**
   - Si faltan modelos `onegai/*` → Terminal: `bash "/Users/admin/Documents/Onegai 2/.onegai_ai_config/scripts/build-models.sh"`

2. **Perfil AuthorAgent** — El `_onegai_profile` en `AuthorAgent/config/default.json` lo cambian los botones "Aplicar".

3. **Node.js 22+** — para arrancar AuthorAgent.

---

## 🔗 Flujo de trabajo recomendado

```
1. Doble clic en Abrir Panel Onegai.command  →  http://127.0.0.1:4100
2. ⚙️ Stack IA  →  ▶️ Arrancar Stack IA     ← Ollama paralelo + warmup + AuthorAgent
3. ✍️ Escritor / 🧐 Crítico  en la barra superior (elige perfil)
4. Marca documentos a la izquierda (busca: boundinghton / personaje / campaña…)
5. Elige pestaña:
   · Ollama directo (correcciones 1 documento)
   · AuthorAgent (encargos complejos, proyectos)
6. Salidas se guardan en PanelOnegai/salidas/*.md
```

---

## 🆘 Cosas que verás y no son fallos

- En el registro `[memory-search] query failed: fts5 syntax error`. Fallo interno de AuthorAgent; no afecta.
- `.docx` se leen extrayendo texto (pierdes tablas/imágenes). Para prosa da igual; para fichas complejas, exporta a `.md` antes.
- Los modelos locales son notablemente más lentos que cloud, pero no envían tu obra a ningún sitio. Para estructura/fichas van de lujo; para prosa final, puedes cambiar luego el proveedor en Ajustes del panel de AuthorAgent sin tocar nada aquí.
