"""Generador de maquetas del modelo visual (MotorGraphico).

Piezas:
  px()        texto pixel con los glifos 5x7 del motor (SVG, escala entera)
  ic()        icono del atlas iconos.png (8x7, 16 px) a x2 o x1
  componentes fila(), boton(), tecla(), ayuda(), insignia(), recurso()...
  pagina()    documento HTML de 1280x720 que usa sistema/tokens.css y
              sistema/componentes.css
  render()    PNG con Chromium (Playwright) a 1280x720, escala 1

Todo el HTML se escribe en fuente/ para que se pueda abrir y retocar.
"""
import html
import json
import os

BASE = os.path.dirname(os.path.abspath(__file__))
# Rutas relativas a maquetas/_generador; se pueden cambiar con variables de entorno.
FUENTE = os.environ.get("MODELO_FUENTE", os.path.join(BASE, "..", "fuente"))
SALIDA = os.environ.get("MODELO_SALIDA", os.path.join(BASE, ".."))

GLIFOS = json.load(open(os.path.join(BASE, "glifos_5x7.json"), encoding="utf-8"))


def _b(*filas):
    return [int(f, 2) for f in filas]


# Glifos que el atlas 5x7 del motor NO tiene hoy. Se proponen con el mismo
# encaje (fila 0 para acentos, 5 columnas) y se señalan en MODELO_VISUAL.md.
PROPUESTOS = {
    "«": _b("00000", "00101", "01010", "10100", "01010", "00101", "00000"),
    "»": _b("00000", "10100", "01010", "00101", "01010", "10100", "00000"),
    "Ï": _b("01010", "01110", "00100", "00100", "00100", "00100", "01110"),
    "←": _b("00000", "00100", "01000", "11111", "01000", "00100", "00000"),
    "→": _b("00000", "00100", "00010", "11111", "00010", "00100", "00000"),
    "↑": _b("00100", "01110", "10101", "00100", "00100", "00100", "00000"),
    "↓": _b("00000", "00100", "00100", "00100", "10101", "01110", "00100"),
    "▶": _b("01000", "01100", "01110", "01111", "01110", "01100", "01000"),
    "◀": _b("00010", "00110", "01110", "11110", "01110", "00110", "00010"),
    "✓": _b("00000", "00001", "00010", "10010", "10100", "01000", "00000"),
    "×": _b("00000", "10001", "01010", "00100", "01010", "10001", "00000"),
    "●": _b("00000", "01110", "11111", "11111", "11111", "01110", "00000"),
    "▲": _b("00000", "00100", "01110", "01110", "11111", "11111", "00000"),
    "■": _b("00000", "11111", "11111", "11111", "11111", "11111", "00000"),
    "◆": _b("00100", "01110", "11111", "11111", "11111", "01110", "00100"),
    # 05/10 (pantallas en cuadrícula): rayo para las reacciones
    "ϟ": _b("00011", "00110", "01100", "11111", "00110", "01100", "11000"),
    "Ϟ": _b("00011", "00110", "01100", "11111", "00110", "01100", "11000"),  # px() pasa a mayúsculas
}
USADOS_PROPUESTOS = set()

# Propuesta tipográfica (MODELO_VISUAL.md §4.2): las mayúsculas acentuadas
# conservan la letra entera (filas 0-6) y ponen la tilde en el interlineado
# (fila -2, con la fila -1 libre). En el atlas actual la tilde ocupa la fila
# 0 y aplasta la letra: «REVISIÓN» se lee «REVISION».
_AGUDO, _GRAVE, _DIER = 0b00110, 0b01100, 0b01010
ACENTUADAS = {
    "Á": ("A", [(-2, _AGUDO)]), "É": ("E", [(-2, _AGUDO)]), "Í": ("I", [(-2, _AGUDO)]),
    "Ó": ("O", [(-2, _AGUDO)]), "Ú": ("U", [(-2, _AGUDO)]),
    "À": ("A", [(-2, _GRAVE)]), "È": ("E", [(-2, _GRAVE)]), "Ì": ("I", [(-2, _GRAVE)]),
    "Ò": ("O", [(-2, _GRAVE)]), "Ù": ("U", [(-2, _GRAVE)]),
    "Ü": ("U", [(-2, _DIER)]), "Ï": ("I", [(-2, _DIER)]),
    "Ñ": ("N", [(-2, 0b01101), (-1, 0b10010)]),
}


def _filas(ch):
    """Devuelve [(fila, bits)] con filas de -2 a 6."""
    if ch in ACENTUADAS:
        base, extra = ACENTUADAS[ch]
        if ch not in GLIFOS:
            USADOS_PROPUESTOS.add(ch)
        return list(enumerate(GLIFOS[base])) + extra
    if ch in GLIFOS:
        return list(enumerate(GLIFOS[ch]))
    if ch in PROPUESTOS:
        USADOS_PROPUESTOS.add(ch)
        return list(enumerate(PROPUESTOS[ch]))
    raise KeyError(f"glifo sin definir: {ch!r}")


def px(texto, s=2, cls="", estilo="", titulo=None, hoy=False):
    """Texto en la fuente pixel del motor. Siempre en mayúsculas (el
    atlas no tiene minúsculas) y a escala entera. Avance de 6 columnas por
    glifo (5 + 1 de separación: hoy BitmapFont avanza 5 y las letras se
    tocan). La caja mide 9 filas: 2 de interlineado arriba para las tildes."""
    t = texto.upper()
    x = 0
    trazos = []
    avance = 5 if hoy else 6   # hoy=True reproduce BitmapFont tal cual (avance 5, tildes en la fila 0)
    for ch in t:
        if ch == " ":
            x += avance
            continue
        filas = list(enumerate(GLIFOS[ch])) if (hoy and ch in GLIFOS) else _filas(ch)
        for fila, bits in filas:
            c = 0
            while c < 5:
                if bits & (1 << (4 - c)):
                    ini = c
                    while c < 5 and bits & (1 << (4 - c)):
                        c += 1
                    trazos.append(f"M{x + ini} {fila}h{c - ini}v1h-{c - ini}z")
                else:
                    c += 1
        x += avance
    ancho = max(x - (0 if hoy else 1), 1)
    etiqueta = html.escape(titulo or texto)
    st = f' style="{estilo}"' if estilo else ""
    return (f'<svg class="px {cls}"{st} viewBox="0 -2 {ancho} 9" width="{ancho * s}" height="{9 * s}" '
            f'shape-rendering="crispEdges" role="img" aria-label="{etiqueta}">'
            f'<path d="{"".join(trazos)}" fill="currentColor"/></svg>')


def ancho_px(texto, s=2):
    return (len(texto) * 6 - 1) * s


# ---------------------------------------------------------------- iconos
ICONOS = json.load(open(os.path.join(FUENTE, "img", "iconos.json"), encoding="utf-8"))["iconos"]


def ic(nombre, x=2, cls="", estilo=""):
    d = ICONOS[nombre]
    col, fila = d["col"], d["row"]
    tam = 16 * x
    clase = "ic" + (" x1" if x == 1 else "") + (f" {cls}" if cls else "")
    return (f'<i class="{clase}" title="{html.escape(d["nombre"])}" '
            f'style="background-position:-{col * tam}px -{fila * tam}px;{estilo}"></i>')


# Éter: el atlas solo tiene un consumible (rosa). Variante de color
# propuesta para distinguirlo de la poción sin arte nuevo.
ETER = "filter:hue-rotate(-115deg) saturate(1.2)"


# ----------------------------------------------------------- componentes
def e(t):
    return html.escape(str(t))


def tecla(*teclas):
    return "".join(f'<span class="tecla">{px(k, 2)}</span>' for k in teclas)


def ayuda(teclas, etiqueta):
    if isinstance(teclas, str):
        teclas = [teclas]
    return f'<span class="ayuda">{tecla(*teclas)}<span>{e(etiqueta)}</span></span>'


def insignia(texto, tipo="", icono=None):
    i = ic(icono, 1) if icono else ""
    t = f" i-{tipo}" if tipo else ""
    return f'<span class="insignia{t}">{i}{px(texto, 2)}</span>'


def boton(etiqueta, tipo="secundario", foco=False, icono=None, ancho=None):
    cls = f"boton {tipo}" + (" foco" if foco else "")
    st = f' style="width:{ancho}px"' if ancho else ""
    i = ic(icono, 1) if icono else ""
    return f'<span class="{cls}"{st}>{i}{px(etiqueta, 2)}</span>'


def fila(nombre, icono=None, valor="", estado="", motivo=None, elegida=False, x_icono=2,
         estilo_icono="", html_nombre=None, marca=True):
    icono_html = ic(icono, x_icono, estilo=estilo_icono) if icono else '<span class="sin-icono"></span>'
    m = f'<span class="marca">{px("▶", 2)}</span>' if marca else "<span></span>"
    mot = f'<span class="motivo">{e(motivo)}</span>' if motivo else ""
    eleg = f'<span class="elegida">{px("✓", 2)}</span>' if elegida else ""
    nom = html_nombre if html_nombre is not None else e(nombre)
    return (f'<div class="fila {estado}">{m}{icono_html}'
            f'<span class="principal"><span class="nombre">{nom}</span>{mot}</span>'
            f'<span class="valor">{valor}{eleg}</span></div>')


def recurso(tipo, etiqueta, actual, maximo, ancho=220, icono=None):
    pct = 0 if maximo <= 0 else max(0.0, min(1.0, actual / maximo)) * 100
    i = ic(icono, 1) if icono else ""
    return (f'<div class="recurso {tipo}" style="width:{ancho}px">'
            f'<div class="linea">{i}{px(etiqueta, 2)}<span class="cifra">{px(f"{actual}/{maximo}", 2)}</span></div>'
            f'<div class="barra"><i style="width:{pct:.1f}%"></i></div></div>')


def moral(valor, etiqueta, ancho=220):
    pos = (valor + 100) / 200 * 100
    return (f'<div class="recurso moral" style="width:{ancho}px">'
            f'<div class="linea">{ic("moral", 1)}{px("MORAL", 2)}<span class="cifra">{px(etiqueta, 2)}</span></div>'
            f'<div class="escala"><b style="left:{pos:.1f}%"></b></div></div>')


def contador(valor, menos=True, mas=True, s=3):
    bm = "b" + ("" if menos else " deshab")
    bM = "b" + ("" if mas else " deshab")
    return (f'<span class="contador"><span class="{bm}">{px("-", 2)}</span>'
            f'<span class="v">{px(str(valor), s)}</span><span class="{bM}">{px("+", 2)}</span></span>')


def deslizador(valor, maximo=10, ancho=260):
    pct = valor / maximo * 100
    marcas = "".join("<i></i>" for _ in range(maximo + 1))
    return (f'<span class="deslizador" style="width:{ancho}px"><span class="relleno" style="width:{pct:.0f}%"></span>'
            f'<span class="marcas">{marcas}</span>'
            f'<span class="mando" style="left:calc({pct:.0f}% - 5px)"></span></span>')


def selector(texto):
    return (f'<span class="selector"><span class="flecha">{px("◀", 2)}</span>'
            f'<span>{e(texto)}</span><span class="flecha">{px("▶", 2)}</span></span>')


def pestanas(items, activa, teclas=("←", "→")):
    """items: lista de (etiqueta, icono)."""
    partes = [tecla(teclas[0])] if teclas else []
    for i, (et, icono) in enumerate(items):
        cls = "pestana activa" if i == activa else "pestana"
        partes.append(f'<span class="{cls}">{ic(icono, 1) if icono else ""}{px(et, 2)}</span>')
    if teclas:
        partes.append('<span class="hueco"></span>' + tecla(teclas[1]))
    return f'<div class="pestanas">{"".join(partes)}</div>'


def bloque_titulo(texto):
    return f'<div class="bloque-titulo">{px(texto, 2)}</div>'


def pagina(titulo, cuerpo, fondo=None, velo=False, css="", filtro_fondo="", alto=720):
    mundo = ""
    if fondo:
        f = f";filter:{filtro_fondo}" if filtro_fondo else ""
        mundo = f'<div class="mundo" style="background-image:url(img/fondos/{fondo}.png){f}"></div>'
    v = '<div class="velo"></div>' if velo is True else (f'<div class="velo" style="background:{velo}"></div>' if velo else "")
    return f"""<!doctype html>
<html lang="es">
<head>
<meta charset="utf-8">
<title>{e(titulo)} · Modelo visual MotorGraphico</title>
<link rel="stylesheet" href="sistema/tokens.css">
<link rel="stylesheet" href="sistema/componentes.css">
<style>{css}</style>
</head>
<body>
<div class="pantalla" style="height:{alto}px">
{mundo}{v}
{cuerpo}
</div>
</body>
</html>
"""


def escribir(nombre, doc):
    ruta = os.path.join(FUENTE, nombre + ".html")
    with open(ruta, "w", encoding="utf-8") as f:
        f.write(doc)
    return ruta


def render(nombres, alto=720, altos=None):
    """Renderiza fuente/<nombre>.html a salida/<nombre>.png."""
    altos = altos or {}
    from playwright.sync_api import sync_playwright
    os.makedirs(SALIDA, exist_ok=True)
    with sync_playwright() as p:
        nav = p.chromium.launch()
        pag = nav.new_page(viewport={"width": 1280, "height": max([alto] + list(altos.values()))}, device_scale_factor=1)
        for n in nombres:
            h = altos.get(n, alto)
            pag.goto("file://" + os.path.join(FUENTE, n + ".html"))
            pag.evaluate("document.fonts.ready")
            pag.wait_for_timeout(150)
            desb = pag.evaluate("""() => {
                const r=[]; const W=document.querySelector('.pantalla');
                const pr=W.getBoundingClientRect();
                for (const el of W.querySelectorAll('.nombre, .desc, .texto, .fila, .boton, .ayuda, .pestana')) {
                  if (el.scrollWidth > el.clientWidth + 1 && getComputedStyle(el).overflow !== 'visible') r.push('corta: '+el.className+' '+el.textContent.trim().slice(0,40));
                  const b=el.getBoundingClientRect();
                  if (b.right>pr.right+0.5||b.bottom>pr.bottom+0.5||b.left<pr.left-0.5||b.top<pr.top-0.5) r.push('fuera: '+el.className+' '+el.textContent.trim().slice(0,40));
                }
                return r;}""")
            pag.screenshot(path=os.path.join(SALIDA, n + ".png"), clip={"x": 0, "y": 0, "width": 1280, "height": h})
            print(f"{n}: ok" + (f"  AVISOS {desb}" if desb else ""))
        nav.close()
