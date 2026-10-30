#!/usr/bin/env python3
"""Genera indice.html y LEEME.md para la carpeta de capturas."""
import html, os, sys
sys.path.insert(0, "/home/claude/work")
from manifiesto import SECCIONES, PIES, FALLOS, VISUAL

RAIZ = sys.argv[1] if len(sys.argv) > 1 else "/home/claude/capturas"
FECHA = "28/09/2026"

secciones = []
total = 0
faltan = []
for carpeta, titulo, intro in SECCIONES:
    pngs = sorted(f for f in os.listdir(os.path.join(RAIZ, carpeta)) if f.endswith(".png"))
    items = []
    for f in pngs:
        clave = f[:-4]
        pie = PIES.get(clave)
        if pie is None:
            faltan.append(clave)
            pie = clave
        items.append((f, clave, pie))
    total += len(items)
    secciones.append((carpeta, titulo, intro, items))
if faltan:
    print("SIN PIE:", faltan)


def insignia(clave):
    if "_SIMULADO" in clave:
        return '<span class="tag sim">SIMULADO</span>'
    if "_HOY_" in clave or clave.endswith("_HOY"):
        return '<span class="tag hoy">HOY</span>'
    if "CUELGA" in clave:
        return '<span class="tag cuelga">CUELGA</span>'
    return ""


e = html.escape
partes = []
partes.append(f"""<!doctype html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Capturas de menús · MotorGraphico</title>
<style>
:root {{
  --bg: #f6f5f2; --panel: #ffffff; --text: #1d1d1f; --muted: #5f6368; --line: #dedbd5;
  --accent: #3a5fb0; --hoy: #b3261e; --sim: #8a5a00; --simbg: #fff3d6; --hoybg: #fde7e5;
}}
@media (prefers-color-scheme: dark) {{
  :root {{ --bg: #16171b; --panel: #1f2126; --text: #e8e6e3; --muted: #a3a6ad; --line: #34373e;
    --accent: #8fb0ff; --hoy: #ff8a80; --sim: #ffcc66; --simbg: #3a2f14; --hoybg: #3d1d1b; }}
}}
* {{ box-sizing: border-box; }}
body {{ margin: 0; background: var(--bg); color: var(--text);
  font: 15px/1.55 -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }}
main {{ max-width: 1280px; margin: 0 auto; padding: 24px 16px 64px; }}
h1 {{ font-size: 26px; margin: 0 0 4px; }}
h2 {{ font-size: 20px; margin: 40px 0 6px; padding-top: 12px; border-top: 1px solid var(--line); }}
h3 {{ font-size: 16px; margin: 24px 0 8px; }}
p.sub, .intro {{ color: var(--muted); margin: 0 0 12px; }}
a {{ color: var(--accent); }}
nav.toc {{ display: flex; flex-wrap: wrap; gap: 6px 10px; margin: 16px 0; }}
nav.toc a {{ text-decoration: none; background: var(--panel); border: 1px solid var(--line);
  border-radius: 6px; padding: 4px 10px; font-size: 13px; }}
.leyenda {{ display: flex; flex-wrap: wrap; gap: 8px 16px; font-size: 13px; color: var(--muted); }}
.tag {{ display: inline-block; font-size: 11px; font-weight: 600; letter-spacing: .03em;
  padding: 1px 6px; border-radius: 4px; vertical-align: 1px; }}
.tag.hoy, .tag.cuelga {{ color: var(--hoy); background: var(--hoybg); }}
.tag.sim {{ color: var(--sim); background: var(--simbg); }}
.grid {{ display: grid; grid-template-columns: repeat(auto-fill, minmax(290px, 1fr)); gap: 14px; }}
figure {{ margin: 0; background: var(--panel); border: 1px solid var(--line); border-radius: 8px; overflow: hidden; }}
figure a {{ display: block; line-height: 0; background: #111; }}
figure img {{ width: 100%; height: auto; aspect-ratio: 16 / 9; object-fit: contain; }}
figcaption {{ padding: 8px 10px 10px; font-size: 13.5px; }}
figcaption .nombre {{ display: block; color: var(--muted); font: 11.5px/1.4 ui-monospace, Menlo, monospace;
  margin-bottom: 4px; overflow-wrap: anywhere; }}
.fallos {{ display: grid; gap: 10px; }}
.fallo {{ background: var(--panel); border: 1px solid var(--line); border-left: 3px solid var(--hoy);
  border-radius: 6px; padding: 10px 12px; }}
.fallo b {{ display: block; margin-bottom: 2px; }}
.fallo .arreglo {{ color: var(--muted); font-size: 13.5px; margin-top: 4px; }}
ul.visual li {{ margin-bottom: 6px; }}
code {{ font: 12.5px ui-monospace, Menlo, monospace; }}
</style>
</head>
<body>
<main>
<h1>Capturas de menús · MotorGraphico</h1>
<p class="sub">{FECHA} · {total} capturas a 1280×720 del árbol actual (con los cambios sin commit): juego, motor_editor y level_editor. Pulsa una imagen para verla a tamaño real.</p>
<div class="leyenda">
  <span><span class="tag hoy">HOY</span> así se ve hoy (muestra un fallo)</span>
  <span><span class="tag sim">SIMULADO</span> menú que hoy no se dibuja; se ve con sus propios widgets gracias a un parche local</span>
  <span><span class="tag cuelga">CUELGA</span> al confirmar esa opción el juego se cierra</span>
</div>
<nav class="toc">
<a href="#fallos">Fallos encontrados</a><a href="#visual">Notas visuales</a>
""")
for carpeta, titulo, _, items in secciones:
    partes.append(f'<a href="#{carpeta}">{e(titulo.split(" · ")[-1] if " · " in titulo else titulo)} ({len(items)})</a>')
partes.append("</nav>")

partes.append('<h2 id="fallos">Lo que salió al hacer las capturas</h2>')
partes.append('<p class="intro">Fallos que impiden ver o usar algún menú. Cada uno lleva las capturas donde se ve y una propuesta de arreglo.</p><div class="fallos">')
for titulo, desc, caps, arreglo in FALLOS:
    partes.append(f'<div class="fallo"><b>{e(titulo)}</b>{e(desc)}'
                  f'<div class="arreglo">Capturas: {e(caps)} · Arreglo propuesto: {e(arreglo)}</div></div>')
partes.append("</div>")

partes.append('<h2 id="visual">Notas visuales</h2><p class="intro">Lo que más salta a la vista al repasar las capturas.</p><ul class="visual">')
for v in VISUAL:
    partes.append(f"<li>{e(v)}</li>")
partes.append("</ul>")

for carpeta, titulo, intro, items in secciones:
    partes.append(f'<h2 id="{carpeta}">{e(titulo)}</h2><p class="intro">{e(intro)}</p><div class="grid">')
    for f, clave, pie in items:
        ruta = f"{carpeta}/{f}"
        partes.append(
            f'<figure><a href="{e(ruta)}" target="_blank"><img loading="lazy" src="{e(ruta)}" alt="{e(pie)}"></a>'
            f'<figcaption><span class="nombre">{e(clave)}</span>{insignia(clave)} {e(pie)}</figcaption></figure>')
    partes.append("</div>")

partes.append("""
<h2 id="como">Cómo se hicieron</h2>
<p class="intro">Se compiló el árbol actual en un Linux con OpenGL por software (Mesa llvmpipe, EGL sin pantalla) y una copia de GLFW 3.4 en su plataforma «null».
Esa copia lee un guion de teclas y ratón fotograma a fotograma y vuelca el framebuffer justo antes de cada swap. El código del motor no se tocó salvo tres parches locales, solo para poder fotografiar:
(1) refreshHud() durante la creación de personaje; (2) registrar y encender los widgets de Pausa, Opciones y Game Over (capturas SIMULADO; con la variable CAPTURAS_COMO_HOY se ven como hoy);
(3) CAPTURAS_TITULO, para pasar por título y creación de personaje en la galería de ejemplos. Todo está en <code>_herramienta/</code>.</p>
</main>
</body>
</html>
""")

with open(os.path.join(RAIZ, "indice.html"), "w") as f:
    f.write("\n".join(partes))

# ------------------------------------------------------------ LEEME.md
L = []
L.append(f"# Capturas de menús · MotorGraphico · {FECHA}\n")
L.append(f"{total} capturas a 1280×720 del árbol actual del repo (con los cambios sin commit). Abre **indice.html** para verlas en galería con su pie de foto.\n")
L.append("- **HOY**: así se ve hoy (muestra un fallo).")
L.append("- **SIMULADO**: menú que hoy no se dibuja; se ve con sus propios widgets gracias a un parche local.")
L.append("- **CUELGA**: al confirmar esa opción el juego se cierra.\n")
L.append("## Carpetas\n")
for carpeta, titulo, intro, items in secciones:
    L.append(f"- `{carpeta}/` ({len(items)}): {titulo}. {intro}")
L.append("\n## Fallos encontrados al capturar\n")
for i, (titulo, desc, caps, arreglo) in enumerate(FALLOS, 1):
    L.append(f"{i}. **{titulo}.** {desc} *Capturas: {caps}.* Arreglo propuesto: {arreglo}")
L.append("\n## Notas visuales\n")
for v in VISUAL:
    L.append(f"- {v}")
L.append("\n## Cómo se hicieron\n")
L.append("Se compiló el árbol actual en Linux con OpenGL por software (Mesa llvmpipe, EGL sin pantalla) y una copia de GLFW 3.4 en su plataforma «null». "
         "Esa copia lee un guion de teclas y ratón fotograma a fotograma y vuelca el framebuffer antes de cada swap (`_herramienta/`). "
         "El motor no se tocó salvo tres parches locales, solo para poder fotografiar: refreshHud() en la creación de personaje, "
         "registrar los widgets de Pausa/Opciones/Game Over (capturas SIMULADO) y `CAPTURAS_TITULO`, para crear personaje en la galería de ejemplos. "
         "Están en `_herramienta/parches_motor_solo_capturas.patch`; no se aplicaron al repo.\n")
L.append("## Lista completa\n")
for carpeta, titulo, intro, items in secciones:
    L.append(f"### {titulo}\n")
    for f, clave, pie in items:
        L.append(f"- `{carpeta}/{f}`: {pie}")
    L.append("")
with open(os.path.join(RAIZ, "LEEME.md"), "w") as f:
    f.write("\n".join(L) + "\n")
print(f"ok: {total} capturas")
