#!/usr/bin/env python3
"""
Panel Onegai — control único para AuthorAgent y StoryCraftr.

Solo usa la librería estándar de Python. Arranca con:
    python3 server.py
y abre http://127.0.0.1:4100
"""

import hashlib
import html as htmllib
import json
import os
import queue
import re
import unicodedata
import shlex
import shutil
import signal
import subprocess
import sys
import threading
import time
import urllib.error
import urllib.request
import zipfile
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

# ── Rutas ────────────────────────────────────────────────────────────────────
HERE = Path(__file__).resolve().parent
ROOT = HERE.parent                      # carpeta "Onegai 2"
AUTHOR_AGENT = ROOT / "AuthorAgent"
STORYCRAFTR = ROOT / "StoryCraftr"
VAULT = ROOT / "Onegai"                 # las notas del proyecto
OUTPUTS = HERE / "salidas"
LOG_FILE = HERE / "panel.log"
CACHE_DIR = HERE / ".cache"
AI_CONFIG = ROOT / ".onegai_ai_config"
AI_SCRIPTS = AI_CONFIG / "scripts"
OUTPUTS.mkdir(exist_ok=True)
CACHE_DIR.mkdir(exist_ok=True)

PANEL_PORT = int(os.environ.get("PANEL_PORT", "4100"))
AA_URL = os.environ.get("AUTHORAGENT_URL", "http://127.0.0.1:3847")
OLLAMA_URL = os.environ.get("OLLAMA_BASE_URL", "http://127.0.0.1:11434")

AGENT_MODELS = {
    "writer": "onegai/story-writer:latest",
    "critic": "onegai/author-critic:latest",
}
AGENT_PROFILES = {
    "writer": {
        "name": "Escritor Creativo (onegai/story-writer)",
        "temp": "0.82",
        "base": "DeepSeek-R1 8.2B · 131K ctx",
        "presets": [
            ("Capítulo CD DnD", 'Redacta este capítulo siguiendo CANON + Estructura Outline Niveles 1-4 (Exposición, Pico, Beats 5, Resolución). Añade test CD DnD a personajes. Termina con "💡 Siguiente paso"'),
            ("Ficha de Personaje CD", 'Crea ficha personaje con tests CD DnD (Clase, Raza, Trasfondo, Motivación Secreta, Rasgo/Defecto/Ideal/Vínculo, Valor CD). Mantén tono misterioso Boundinghton.'),
            ("Outline Campaña", 'Diseña outline campaña: Acto 1-3, Escenas Nivel 1-4, Picos emocionales, enemigo final, finales alternativos misteriosos. Añade 💡 Siguiente paso.'),
            ("Ampliar Escena", 'Amplía esta escena: añade diálogos, descripciones sensoriales, beats emocionales, y un giro misterioso. Formato MD. Final 💡 Siguiente paso.'),
        ]
    },
    "critic": {
        "name": "Crítico Literario (onegai/author-critic)",
        "temp": "0.25",
        "base": "Gemma2 9B · Crítica estructurada",
        "presets": [
            ("Crítica Completa 5pts", 'Revisa usando estructura 5 puntos: 🔍 RESUMEN, ✅ POSITIVOS, ⚠️ PROBLEMAS (gravedad CRÍTICO/ALTO/MEDIO/BAJO con citas), 🧩 TABLA INCONSISTENCIAS (3 cols), 📋 CHECKLIST. Aplica REGLA 3 PORQUÉ a cada problema grave. Finaliza con 🎯 ACCIÓN INMEDIATA priorizada.'),
            ("Inconsistencias de Canon", 'Busca contradicciones entre estos documentos y el canon. Tabla 3 columnas: Documento A | Documento B | Descripción conflicto. Aplica regla CANON PRIMERO.'),
            ("Checklist de Estilo", 'Valida: CANON respetado? 💡 Siguiente paso presente? Tono coherente? Saltos de tono? Estructura Outline 1-4? Formato MD? Tests CD personajes?'),
            ("Coherencia Cronológica", 'Revisa cronología: fechas coherentes, causas preceden efectos, progresión personajes lógica. Marca incoherencias con citas exactas.'),
        ]
    },
}

READABLE = {".md", ".txt", ".markdown", ".docx", ".csv", ".json", ".canvas"}
SKIP_DIRS = {".obsidian", ".git", "node_modules", "__pycache__",
             ".timeline_images_extracted", ".tools_venv"}
DEFAULT_BUDGET = 40000                  # caracteres de contexto por encargo

# ── Estado compartido ────────────────────────────────────────────────────────
LOG = []
LOG_LOCK = threading.Lock()
AA_PROC = None
JOBS = {}
JOB_SEQ = [0]
JOB_QUEUE = queue.Queue()
INDEX_CACHE = {"t": 0, "files": []}


def log(source, text):
    entry = {"t": time.strftime("%H:%M:%S"), "source": source, "text": str(text)}
    with LOG_LOCK:
        LOG.append(entry)
        del LOG[:-500]
    try:
        with open(LOG_FILE, "a", encoding="utf-8") as f:
            f.write(f"[{entry['t']}] {source}: {entry['text']}\n")
    except OSError:
        pass


# ── Lectura de documentos ────────────────────────────────────────────────────
def docx_text(path):
    """Saca el texto de un .docx sin dependencias externas."""
    try:
        with zipfile.ZipFile(path) as z:
            xml = z.read("word/document.xml").decode("utf-8", "replace")
    except (zipfile.BadZipFile, KeyError, OSError) as e:
        return f"(no he podido leer este .docx: {e})"
    xml = re.sub(r"</w:p>", "\n", xml)
    xml = re.sub(r"<w:tab[^>]*/>", "\t", xml)
    xml = re.sub(r"<w:br[^>]*/>", "\n", xml)
    text = htmllib.unescape(re.sub(r"<[^>]+>", "", xml))
    return re.sub(r"\n{3,}", "\n\n", text).strip()


def read_document(path):
    p = Path(path)
    if not p.is_file():
        return f"(no encuentro {p.name})"
    ext = p.suffix.lower()
    if ext == ".docx":
        return docx_text(p)
    if ext in READABLE:
        try:
            return p.read_text(encoding="utf-8", errors="replace").strip()
        except OSError as e:
            return f"(no he podido leer {p.name}: {e})"
    return f"(no sé leer archivos {ext})"


def scan_vault(force=False):
    """Índice de todo lo legible del proyecto. Se cachea 60 segundos."""
    if not force and time.time() - INDEX_CACHE["t"] < 60:
        return INDEX_CACHE["files"]
    files = []
    roots = [VAULT] if VAULT.is_dir() else []
    for extra in ROOT.glob("*.md"):                 # notas sueltas en la raíz
        files.append(entry_for(extra))
    for root in roots:
        for dirpath, dirnames, filenames in os.walk(root):
            dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
            for name in filenames:
                if Path(name).suffix.lower() in READABLE and not name.startswith("."):
                    files.append(entry_for(Path(dirpath) / name))
    files.sort(key=lambda f: f["rel"].lower())
    INDEX_CACHE.update({"t": time.time(), "files": files})
    return files


def entry_for(p):
    try:
        size = p.stat().st_size
    except OSError:
        size = 0
    return {
        "rel": str(p.relative_to(ROOT)),
        "name": p.name,
        "folder": str(p.parent.relative_to(ROOT)),
        "ext": p.suffix.lower().lstrip("."),
        "size": size,
    }


# ── Búsqueda dentro del texto ────────────────────────────────────────────────
INDEX_LOCK = threading.Lock()
INDEX_STATE = {"building": False, "done": 0, "total": 0, "at": 0}


def cached_text(p):
    """Texto de un documento, cacheado en disco. Se rehace si el archivo cambia."""
    try:
        st = p.stat()
    except OSError:
        return ""
    key = hashlib.sha1(f"{p}|{st.st_mtime_ns}|{st.st_size}".encode()).hexdigest()
    blob = CACHE_DIR / (key + ".txt")
    if blob.exists():
        try:
            return blob.read_text(encoding="utf-8")
        except OSError:
            pass
    text = read_document(p)
    try:
        blob.write_text(text, encoding="utf-8")
    except OSError:
        pass
    return text


def fold(s):
    """Minúsculas y sin acentos, para que 'Kitsune' encuentre 'kitsuné'."""
    return "".join(c for c in unicodedata.normalize("NFD", s.lower())
                   if unicodedata.category(c) != "Mn")


def build_index(background=True):
    """Precalcula el texto de todos los documentos."""
    def run():
        files = scan_vault()
        with INDEX_LOCK:
            INDEX_STATE.update({"building": True, "done": 0, "total": len(files)})
        for i, f in enumerate(files, 1):
            try:
                cached_text(ROOT / f["rel"])
            except Exception:                        # noqa: BLE001
                pass
            with INDEX_LOCK:
                INDEX_STATE["done"] = i
        with INDEX_LOCK:
            INDEX_STATE.update({"building": False, "at": time.time()})
        log("panel", f"Índice de búsqueda listo: {len(files)} documentos.")

    if INDEX_STATE["building"]:
        return
    if background:
        threading.Thread(target=run, daemon=True).start()
    else:
        run()


def search_text(query, limit=60):
    """Busca en el contenido. Devuelve documentos con la frase donde aparece."""
    needle = fold(query.strip())
    if len(needle) < 2:
        return []
    hits = []
    for f in scan_vault():
        text = cached_text(ROOT / f["rel"])
        if not text:
            continue
        hay = fold(text)
        pos = hay.find(needle)
        if pos < 0:
            continue
        count = hay.count(needle)
        snippets, start = [], 0
        while len(snippets) < 3:
            at = hay.find(needle, start)
            if at < 0:
                break
            a, b = max(0, at - 90), min(len(text), at + len(needle) + 90)
            frag = " ".join(text[a:b].split())
            snippets.append(("…" if a else "") + frag + ("…" if b < len(text) else ""))
            start = at + len(needle)
        hits.append({**f, "count": count, "snippets": snippets})
    hits.sort(key=lambda h: -h["count"])
    return hits[:limit]


def safe_path(rel):
    """Resuelve una ruta relativa sin dejar salir de la carpeta del proyecto."""
    p = (ROOT / rel).resolve()
    if ROOT.resolve() not in p.parents and p != ROOT.resolve():
        raise ValueError("ruta fuera del proyecto")
    return p


def build_context(rels, budget=DEFAULT_BUDGET):
    """Monta el bloque de contexto con el contenido real de los archivos."""
    if not rels:
        return "", []
    share = max(2000, budget // len(rels))
    parts, used = [], []
    for rel in rels:
        try:
            p = safe_path(rel)
        except ValueError:
            continue
        text = read_document(p)
        cut = False
        if len(text) > share:
            text, cut = text[:share], True
        parts.append(
            f"### Archivo: {rel}\n\n{text}"
            + ("\n\n[…documento recortado por longitud…]" if cut else "")
        )
        used.append({"rel": rel, "chars": len(text), "cut": cut})
    block = (
        "A continuación tienes el contenido REAL de los documentos del proyecto. "
        "Trabaja únicamente sobre este texto; no inventes nada que no esté aquí.\n\n"
        + "\n\n---\n\n".join(parts)
        + "\n\n--- fin de los documentos ---\n\n"
    )
    return block, used


# ── Utilidades HTTP ──────────────────────────────────────────────────────────
def http_json(url, method="GET", payload=None, timeout=600):
    data = None
    headers = {"Accept": "application/json"}
    if payload is not None:
        data = json.dumps(payload).encode("utf-8")
        headers["Content-Type"] = "application/json"
    req = urllib.request.Request(url, data=data, headers=headers, method=method)
    with urllib.request.urlopen(req, timeout=timeout) as r:
        body = r.read().decode("utf-8", "replace")
    try:
        return json.loads(body)
    except json.JSONDecodeError:
        return {"raw": body}


# ── AuthorAgent ──────────────────────────────────────────────────────────────
def aa_alive():
    try:
        http_json(AA_URL + "/api/health", timeout=2)
        return True
    except Exception:
        return False


def link_vault_into_workspace():
    """AuthorAgent solo puede tocar su carpeta workspace. Le enlazamos Onegai."""
    ws = AUTHOR_AGENT / "workspace"
    if not ws.is_dir() or not VAULT.is_dir():
        return
    link = ws / "Onegai"
    try:
        if not link.exists() and not link.is_symlink():
            link.symlink_to(VAULT, target_is_directory=True)
            log("panel", "Enlazada la carpeta Onegai dentro del espacio de AuthorAgent.")
    except OSError as e:
        log("panel", f"No he podido enlazar Onegai: {e}")


def _pump(stream, source):
    for line in iter(stream.readline, ""):
        line = line.rstrip()
        if line and "[memory-search] query failed" not in line:
            log(source, line)
    stream.close()


def aa_start():
    global AA_PROC
    if aa_alive():
        return "AuthorAgent ya estaba en marcha."
    if not AUTHOR_AGENT.is_dir():
        return "No encuentro la carpeta AuthorAgent."
    npm = shutil.which("npm")
    if not npm:
        return "No encuentro npm. Instala Node.js 22 o superior."
    if not (AUTHOR_AGENT / "node_modules").is_dir():
        log("panel", "Instalando dependencias de AuthorAgent (puede tardar)…")
        subprocess.run([npm, "install"], cwd=str(AUTHOR_AGENT))
    link_vault_into_workspace()
    log("panel", "Arrancando AuthorAgent…")
    AA_PROC = subprocess.Popen(
        [npm, "start"], cwd=str(AUTHOR_AGENT),
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, bufsize=1,
        env={**os.environ, "OLLAMA_BASE_URL": OLLAMA_URL},
    )
    threading.Thread(target=_pump, args=(AA_PROC.stdout, "authoragent"), daemon=True).start()
    for _ in range(90):
        if aa_alive():
            return "AuthorAgent listo en " + AA_URL
        if AA_PROC.poll() is not None:
            return "AuthorAgent se cerró al arrancar. Mira el registro."
        time.sleep(1)
    return "AuthorAgent tarda más de lo normal. Mira el registro."


def aa_stop():
    global AA_PROC
    if AA_PROC and AA_PROC.poll() is None:
        AA_PROC.send_signal(signal.SIGTERM)
        try:
            AA_PROC.wait(timeout=15)
        except subprocess.TimeoutExpired:
            AA_PROC.kill()
        AA_PROC = None
        return "AuthorAgent detenido."
    return "AuthorAgent no lo arrancó este panel (o ya está parado)."


# ── Ollama ───────────────────────────────────────────────────────────────────
def ollama_models():
    try:
        data = http_json(OLLAMA_URL + "/api/tags", timeout=3)
        return [m.get("name") for m in data.get("models", []) if m.get("name")]
    except Exception:
        return []


def ollama_models_prioritized():
    """Devuelve modelos Ollama con los onegai/* personalizados PRIMERO."""
    names = ollama_models()
    def sort_key(n):
        if n.startswith("onegai/story-writer"):
            return 0
        if n.startswith("onegai/author-critic"):
            return 1
        if n.startswith("onegai/"):
            return 2
        return 3
    return sorted(names, key=sort_key)


def ollama_running():
    try:
        urllib.request.urlopen(OLLAMA_URL + "/api/tags", timeout=2)
        return True
    except Exception:
        return False


def ollama_ask(model, prompt, ctx=8192):
    payload = {"model": model, "prompt": prompt, "stream": False,
               "options": {"num_ctx": ctx}}
    data = http_json(OLLAMA_URL + "/api/generate", "POST", payload, timeout=3600)
    return data.get("response", "") or json.dumps(data)[:2000]


# ── Stack: Ollama + AuthorAgent vía scripts .onegai_ai_config ─────────────────
STACK_PROC = None

def _run_script(name, timeout=600, args=()):
    script = AI_SCRIPTS / name
    if not script.is_file():
        msg = f"Falta script: {script}"
        log("panel", msg)
        return False, msg
    log("panel", f"Ejecutando .onegai_ai_config/scripts/{name} {' '.join(args)}…")
    try:
        p = subprocess.run(
            ["bash", str(script), *args], cwd=str(ROOT),
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, timeout=timeout,
            env={**os.environ, "OLLAMA_BASE_URL": OLLAMA_URL,
                 "AUTHORAGENT_URL": AA_URL, "PYTHONUNBUFFERED": "1"},
        )
        output = p.stdout or ""
        for line in output.strip().splitlines()[-30:]:
            if line.strip():
                log("stack", line.rstrip())
        ok = p.returncode == 0
        return ok, output[-4000:]
    except subprocess.TimeoutExpired as e:
        out = getattr(e, "stdout", "") or ""
        log("stack", f"[timeout {timeout}s] {out[-1000:]}")
        return False, f"timeout {timeout}s: {out[-1000:]}"
    except OSError as e:
        log("stack", f"[error] {e}")
        return False, str(e)


def stack_start():
    ok, out = _run_script("start-agents.sh", timeout=900)
    return ("Stack arrancando… mira el registro." if ok else "Fallo al arrancar stack."), out


def stack_stop():
    ok, out = _run_script("stop-agents.sh", timeout=300)
    return ("Stack parándose…" if ok else "Fallo al parar stack."), out


def stack_status_run():
    ok, out = _run_script("status-agents.sh", timeout=120)
    return out or "(status sin salida)"


# ── Switch perfil AuthorAgent ────────────────────────────────────────────────
def aa_switch_profile(profile):
    profile = profile.lower().strip()
    if profile not in ("writer", "critic", "status"):
        return False, "Perfil no válido. Usa: writer | critic | status"
    script = AI_SCRIPTS / "switch-authoragent-profile.sh"
    if not script.is_file():
        return False, f"Falta script {script}"
    ok, out = _run_script("switch-authoragent-profile.sh", timeout=60, args=(profile,))
    if profile != "status" and ok:
        log("panel", f"Perfil AuthorAgent cambiado a: {profile}. Reinicia AuthorAgent para que surta efecto.")
    return ok, out[-2000:]


def aa_current_profile():
    """Lee el perfil actual del config default.json de AuthorAgent."""
    path = AUTHOR_AGENT / "config" / "default.json"
    if not path.is_file():
        return None
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None
    return data.get("_onegai_profile") or data.get("model")


# ── Encargos ─────────────────────────────────────────────────────────────────
def new_job(label, files):
    JOB_SEQ[0] += 1
    jid = str(JOB_SEQ[0])
    JOBS[jid] = {"id": jid, "label": label, "status": "en cola", "output": "",
                 "files": files, "saved": None, "started": None, "ended": None,
                 "step": ""}
    return JOBS[jid]


def save_output(job):
    if not job["output"].strip():
        return
    slug = re.sub(r"[^\w\s-]", "", job["label"])[:50].strip().replace(" ", "-") or "encargo"
    path = OUTPUTS / f"{int(job['id']):03d}-{slug}.md"
    header = (
        f"# {job['label']}\n\n"
        f"_Encargo #{job['id']} · {time.strftime('%d/%m/%Y')} {job['started']}–{job['ended']}_\n\n"
        + ("**Documentos usados:** " + ", ".join(job["files"]) + "\n\n" if job["files"] else "")
        + "---\n\n"
    )
    try:
        path.write_text(header + job["output"], encoding="utf-8")
        job["saved"] = str(path.relative_to(ROOT))
        log("panel", f"Respuesta guardada en {job['saved']}")
    except OSError as e:
        log("panel", f"No he podido guardar la respuesta: {e}")


def run_command(job, args, cwd):
    log("panel", "$ " + " ".join(args))
    try:
        p = subprocess.Popen(
            args, cwd=str(cwd), stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, bufsize=1,
            env={**os.environ, "OLLAMA_BASE_URL": OLLAMA_URL, "PYTHONUNBUFFERED": "1"},
        )
    except FileNotFoundError as e:
        job["output"] += f"\nNo encuentro el programa: {e}"
        return False
    for line in iter(p.stdout.readline, ""):
        job["output"] += line
        if line.strip():
            log("storycraftr", line.rstrip())
    p.stdout.close()
    return p.wait() == 0


def storycraftr_cmd():
    exe = shutil.which("storycraftr")
    if exe:
        return [exe]
    uvx = shutil.which("uvx")
    if uvx and STORYCRAFTR.is_dir():
        return [uvx, "--from", str(STORYCRAFTR), "storycraftr"]
    return []


def ask_engine(kind, model, message, job):
    """Manda un mensaje al motor elegido y devuelve (texto, ok)."""
    if kind == "authoragent":
        if not aa_alive():
            job["output"] += aa_start() + "\n\n"
        res = http_json(AA_URL + "/api/chat", "POST",
                        {"message": message[:200000]}, timeout=3600)
        return str(res.get("response") or res.get("error") or res), ("error" not in res)
    ctx = min(32768, max(8192, 2048 + len(message) // 3))
    return ollama_ask(model, message, ctx), True


SYNTHESIS_PROMPT = (
    "Estos son los resultados de aplicar el mismo encargo a cada documento por "
    "separado. Júntalos en una sola respuesta coherente: agrupa lo que se repite, "
    "señala lo que se contradice entre documentos y no inventes nada nuevo.\n\n"
    "Encargo original: {prompt}\n\n{parts}"
)


def worker():
    while True:
        job, kind, spec = JOB_QUEUE.get()
        job["status"] = "en marcha"
        job["started"] = time.strftime("%H:%M:%S")
        ok = True
        budget = spec.get("budget", DEFAULT_BUDGET)
        prompt = spec["prompt"]
        try:
            if kind in ("storycraftr", "shell"):
                if kind == "shell":
                    ok = run_command(job, shlex.split(prompt), ROOT)
                else:
                    base = storycraftr_cmd()
                    if not base:
                        job["output"] += ("No encuentro StoryCraftr instalado. "
                                          "Mira el LEEME para instalarlo.")
                        ok = False
                    else:
                        cwd = VAULT if VAULT.is_dir() else ROOT
                        ok = run_command(job, base + shlex.split(prompt), cwd)

            elif spec.get("mode") == "uno" and len(job["files"]) > 1:
                # Un pase por documento, y luego una síntesis.
                results = []
                total = len(job["files"])
                for i, rel in enumerate(job["files"], 1):
                    job["step"] = f"documento {i} de {total}"
                    log("panel", f"Encargo #{job['id']}: {job['step']} — {rel}")
                    context, _ = build_context([rel], budget)
                    text, good = ask_engine(kind, spec.get("model"), context + prompt, job)
                    ok = ok and good
                    results.append((rel, text))
                    job["output"] = "\n\n".join(
                        f"## {r}\n\n{t}" for r, t in results
                    ) + f"\n\n_({i} de {total} procesados)_"
                if spec.get("synthesis", True) and len(results) > 1:
                    job["step"] = "juntando resultados"
                    log("panel", f"Encargo #{job['id']}: juntando resultados…")
                    parts = "\n\n".join(f"### {r}\n\n{t[:budget // len(results)]}"
                                        for r, t in results)
                    summary, good = ask_engine(
                        kind, spec.get("model"),
                        SYNTHESIS_PROMPT.format(prompt=prompt, parts=parts), job)
                    ok = ok and good
                    job["output"] = ("# Conjunto\n\n" + summary
                                     + "\n\n---\n\n# Documento por documento\n\n"
                                     + "\n\n".join(f"## {r}\n\n{t}" for r, t in results))
                else:
                    job["output"] = "\n\n".join(f"## {r}\n\n{t}" for r, t in results)
                job["step"] = ""

            else:
                context, used = build_context(job["files"], budget)
                if used:
                    total = sum(u["chars"] for u in used)
                    log("panel", f"Adjuntando {len(used)} documento(s), {total:,} caracteres.")
                text, ok = ask_engine(kind, spec.get("model"), context + prompt, job)
                job["output"] += text

        except Exception as e:                       # noqa: BLE001
            job["output"] += f"\nFallo: {e}"
            ok = False
        job["status"] = "hecho" if ok else "con errores"
        job["step"] = ""
        job["ended"] = time.strftime("%H:%M:%S")
        save_output(job)
        log("panel", f"Encargo #{job['id']} → {job['status']}")
        JOB_QUEUE.task_done()


threading.Thread(target=worker, daemon=True).start()


# ── Servidor web ─────────────────────────────────────────────────────────────
class Handler(BaseHTTPRequestHandler):
    def _send(self, code, body, ctype="application/json; charset=utf-8"):
        raw = body.encode("utf-8") if isinstance(body, str) else body
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(raw)))
        self.end_headers()
        self.wfile.write(raw)

    def log_message(self, *_a):
        pass

    def do_GET(self):
        path = self.path.split("?")[0]
        if path in ("/", "/index.html"):
            return self._send(200, (HERE / "index.html").read_bytes(),
                              "text/html; charset=utf-8")
        if path == "/api/status":
            models_pri = ollama_models_prioritized()
            return self._send(200, json.dumps({
                "authoragent": aa_alive(), "authoragent_url": AA_URL,
                "authoragent_profile": aa_current_profile(),
                "ollama": models_pri, "ollama_alive": ollama_running(),
                "storycraftr": bool(storycraftr_cmd()),
                "vault": str(VAULT.name),
                "ai_config": AI_CONFIG.is_dir(),
                "ai_scripts": AI_SCRIPTS.is_dir(),
                "agent_models": AGENT_MODELS,
            }))
        if path == "/api/profiles":
            return self._send(200, json.dumps({
                "profiles": AGENT_PROFILES,
                "current": aa_current_profile(),
                "agent_models": AGENT_MODELS,
                "models_prioritized": ollama_models_prioritized(),
            }))
        if path == "/api/stack/status":
            return self._send(200, json.dumps({
                "output": stack_status_run(),
            }))
        if path == "/api/files":
            return self._send(200, json.dumps({"files": scan_vault("refresh" in self.path)}))
        if path == "/api/index":
            with INDEX_LOCK:
                return self._send(200, json.dumps(dict(INDEX_STATE)))
        if path == "/api/jobs":
            return self._send(200, json.dumps({
                "jobs": sorted(JOBS.values(), key=lambda j: -int(j["id"]))[:40]}))
        if path == "/api/log":
            with LOG_LOCK:
                return self._send(200, json.dumps({"log": LOG[-200:]}))
        return self._send(404, json.dumps({"error": "no existe"}))

    def do_POST(self):
        length = int(self.headers.get("Content-Length") or 0)
        try:
            body = json.loads(self.rfile.read(length) or b"{}")
        except json.JSONDecodeError:
            return self._send(400, json.dumps({"error": "json inválido"}))

        if self.path == "/api/authoragent/start":
            return self._send(200, json.dumps({"message": aa_start()}))
        if self.path == "/api/authoragent/stop":
            return self._send(200, json.dumps({"message": aa_stop()}))
        if self.path == "/api/authoragent/restart":
            aa_stop()
            time.sleep(2)
            return self._send(200, json.dumps({"message": aa_start()}))
        if self.path == "/api/authoragent/switch-profile":
            profile = (body.get("profile") or "writer").lower()
            ok, out = aa_switch_profile(profile)
            return self._send(200, json.dumps({"ok": ok, "profile": profile,
                                               "output": out,
                                               "message": ("Perfil cambiado. Reinicia AuthorAgent para aplicar."
                                                           if ok else "No se ha podido cambiar el perfil")}))

        if self.path == "/api/stack/start":
            msg, out = stack_start()
            return self._send(200, json.dumps({"message": msg, "output": out}))
        if self.path == "/api/stack/stop":
            msg, out = stack_stop()
            return self._send(200, json.dumps({"message": msg, "output": out}))

        if self.path == "/api/search":
            q = (body.get("q") or "").strip()
            t0 = time.time()
            hits = search_text(q)
            log("panel", f"Búsqueda «{q}»: {len(hits)} documento(s) "
                         f"en {time.time() - t0:.1f}s")
            return self._send(200, json.dumps({"q": q, "hits": hits}))

        if self.path == "/api/reindex":
            build_index()
            return self._send(200, json.dumps({"message": "Reconstruyendo el índice…"}))

        if self.path == "/api/preview":
            rel = body.get("rel", "")
            try:
                text = read_document(safe_path(rel))
            except ValueError:
                text = "(ruta no válida)"
            return self._send(200, json.dumps({"rel": rel, "chars": len(text),
                                               "preview": text[:1500]}))

        if self.path == "/api/enqueue":
            kind = body.get("kind")
            prompt = (body.get("prompt") or "").strip()
            files = [f for f in (body.get("files") or []) if isinstance(f, str)]
            if kind not in ("authoragent", "storycraftr", "ollama", "shell") or not prompt:
                return self._send(400, json.dumps({"error": "encargo incompleto"}))
            job = new_job(body.get("label") or prompt[:70], files)
            spec = {"prompt": prompt, "model": body.get("model"),
                    "budget": int(body.get("budget") or DEFAULT_BUDGET),
                    "mode": body.get("mode") or "juntos",
                    "synthesis": bool(body.get("synthesis", True))}
            JOB_QUEUE.put((job, kind, spec))
            log("panel", f"Encargo #{job['id']} en cola: {job['label']}")
            return self._send(200, json.dumps({"job": job}))

        return self._send(404, json.dumps({"error": "no existe"}))


def main():
    log("panel", f"Panel Onegai en http://127.0.0.1:{PANEL_PORT}")
    log("panel", f"{len(scan_vault(True))} documentos legibles encontrados.")
    build_index()
    srv = ThreadingHTTPServer(("127.0.0.1", PANEL_PORT), Handler)
    try:
        srv.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        aa_stop()


if __name__ == "__main__":
    main()
