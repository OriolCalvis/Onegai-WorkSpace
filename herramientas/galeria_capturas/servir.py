#!/usr/bin/env python3
"""Web local para navegar la galeria de .md. Solo escucha en este Mac (127.0.0.1).

Uso: python3 servir.py [puerto]  (por defecto 8765). Sin dependencias.
"""
import html
import http.server
import json
import mimetypes
import os
import re
import sys
import urllib.parse

RAIZ = os.path.dirname(os.path.abspath(__file__))
PUERTO = int(sys.argv[1]) if len(sys.argv) > 1 else 8765

ESTILO = """
:root{--fondo:#faf8f3;--texto:#2a2622;--suave:#6f675e;--borde:#ddd6c9;--acento:#8a4b1f;--caja:#fff}
@media (prefers-color-scheme:dark){:root{--fondo:#1d1b18;--texto:#ece7df;--suave:#a89f93;--borde:#3b3731;--acento:#e0a06a;--caja:#26231f}}
*{box-sizing:border-box}body{margin:0;background:var(--fondo);color:var(--texto);font:15px/1.5 -apple-system,system-ui,sans-serif}
header{position:sticky;top:0;z-index:2;display:flex;gap:12px;align-items:center;padding:10px 16px;background:var(--caja);border-bottom:1px solid var(--borde)}
header a{color:var(--acento);font-weight:600;text-decoration:none;white-space:nowrap}
input{flex:1;min-width:0;padding:7px 10px;font:inherit;color:inherit;background:var(--fondo);border:1px solid var(--borde);border-radius:6px}
main{max-width:1400px;margin:auto;padding:16px}a{color:var(--acento)}
h1{font-size:24px;margin:.3em 0}h2{font-size:17px;margin:1.6em 0 .6em;padding-bottom:4px;border-bottom:1px solid var(--borde);word-break:break-word}
.graella{display:grid;grid-template-columns:repeat(auto-fill,minmax(min(280px,100%),1fr));gap:12px}
figure{margin:0;background:var(--caja);border:1px solid var(--borde);border-radius:6px;overflow:hidden}
figure img{display:block;width:100%;height:180px;object-fit:contain;background:#00000012;image-rendering:pixelated;cursor:zoom-in}
figcaption{padding:7px 9px;font-size:12px;color:var(--suave);word-break:break-word}figcaption strong{color:var(--texto)}
li{margin:2px 0;word-break:break-word}
#gran{position:fixed;inset:0;z-index:5;display:none;background:#000d;align-items:center;justify-content:center;cursor:zoom-out}
#gran img{max-width:96vw;max-height:96vh;image-rendering:pixelated}
"""

GUION = """
const gran=document.getElementById('gran');
document.addEventListener('click',e=>{
  if(e.target.matches('figure img')){gran.firstChild.src=e.target.src;gran.style.display='flex'}
  else if(e.target.closest('#gran')){gran.style.display='none'}
});
document.addEventListener('keydown',e=>{if(e.key==='Escape')gran.style.display='none'});
"""


def en_linea(t):
    t = html.escape(t, quote=False)
    t = re.sub(r"\*\*(.+?)\*\*", r"<strong>\1</strong>", t)
    t = re.sub(r"`([^`]+)`", r"<code>\1</code>", t)
    return re.sub(r"\[([^\]]+)\]\(([^)]+)\)",
                  lambda m: f'<a href="{html.escape(m.group(2).replace(chr(32), "%20"))}">{m.group(1)}</a>', t)


def md_a_html(texto):
    """Solo lo que escribe construir.py: titulos, listas, parrafos y figuras (imagen + pie)."""
    sal, lineas, i, abierta, lista = [], texto.split("\n"), 0, False, False

    def cierra():
        nonlocal abierta, lista
        if abierta:
            sal.append("</div>")
            abierta = False
        if lista:
            sal.append("</ul>")
            lista = False

    while i < len(lineas):
        l = lineas[i]
        img = re.match(r"^!\[(.*)\]\((.*)\)$", l)
        if img:
            if lista:
                cierra()
            if not abierta:
                sal.append('<div class="graella">')
                abierta = True
            pie = lineas[i + 1] if i + 1 < len(lineas) else ""
            sal.append(f'<figure><img loading="lazy" alt="{html.escape(img.group(1))}" src="{html.escape(img.group(2))}">'
                       f"<figcaption>{en_linea(pie)}</figcaption></figure>")
            i += 2
            continue
        if l.startswith("- "):
            if abierta:
                cierra()
            if not lista:
                sal.append("<ul>")
                lista = True
            sal.append(f"<li>{en_linea(l[2:])}</li>")
        elif l.strip():
            cierra()
            m = re.match(r"^(#{1,3}) (.*)$", l)
            if m:
                n = len(m.group(1))
                sal.append(f"<h{n}>{en_linea(m.group(2))}</h{n}>")
            else:
                sal.append(f"<p>{en_linea(l)}</p>")
        i += 1
    cierra()
    return "\n".join(sal)


def plantilla(titulo, cuerpo, q=""):
    return (f'<!doctype html><html lang="ca"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">'
            f"<title>{html.escape(titulo)}</title><style>{ESTILO}</style>"
            f'<header><a href="/">Captures</a><form action="/cerca" style="display:flex;flex:1">'
            f'<input name="q" placeholder="Cerca per nom, carpeta o descripció" value="{html.escape(q)}"></form></header>'
            f'<main>{cuerpo}</main><div id="gran"><img alt=""></div><script>{GUION}</script></html>').encode()


class Manejador(http.server.BaseHTTPRequestHandler):
    def log_message(self, *a):
        pass

    def envia(self, datos, tipo="text/html; charset=utf-8", codigo=200):
        self.send_response(codigo)
        self.send_header("Content-Type", tipo)
        self.send_header("Content-Length", str(len(datos)))
        self.end_headers()
        self.wfile.write(datos)

    def do_GET(self):
        u = urllib.parse.urlparse(self.path)
        ruta = urllib.parse.unquote(u.path).lstrip("/") or "index.md"
        if ruta == "cerca":
            return self.cerca(urllib.parse.parse_qs(u.query).get("q", [""])[0])
        real = os.path.realpath(os.path.join(RAIZ, ruta))
        if not real.startswith(RAIZ + os.sep) or not os.path.isfile(real):
            return self.envia(plantilla("No trobat", "<h1>No trobat</h1>"), codigo=404)
        with open(real, "rb") as f:
            datos = f.read()
        if real.endswith(".md"):
            texto = datos.decode("utf-8")
            titulo = texto.split("\n", 1)[0].lstrip("# ")
            return self.envia(plantilla(titulo, md_a_html(texto)))
        self.envia(datos, mimetypes.guess_type(real)[0] or "application/octet-stream")

    def cerca(self, q):
        with open(os.path.join(RAIZ, "cerca.json"), encoding="utf-8") as f:
            items = json.load(f)
        trozos = q.lower().split()
        hits = [e for e in items if trozos and all(
            t in (e["n"] + " " + e["d"] + " " + " ".join(e["r"])).lower() for t in trozos)]
        hits.sort(key=lambda e: e["t"], reverse=True)
        figs = "".join(
            f'<figure><img loading="lazy" alt="{html.escape(e["n"])}" src="/{e["f"]}"><figcaption>'
            f'<strong>{html.escape(e["n"])}</strong> · {e["t"]}<br>{en_linea(e["d"])}<br>{html.escape(e["r"][0] if e["r"] else "")}'
            "</figcaption></figure>" for e in hits[:400])
        mas = f"<p>Es mostren les 400 primeres de {len(hits)}.</p>" if len(hits) > 400 else ""
        cuerpo = f'<h1>Cerca: {html.escape(q)}</h1><p>{len(hits)} resultats.</p>{mas}<div class="graella">{figs}</div>'
        self.envia(plantilla("Cerca", cuerpo, q))


if __name__ == "__main__":
    print(f"Galeria a http://127.0.0.1:{PUERTO}/  (Ctrl+C per aturar)", flush=True)
    http.server.ThreadingHTTPServer(("127.0.0.1", PUERTO), Manejador).serve_forever()
