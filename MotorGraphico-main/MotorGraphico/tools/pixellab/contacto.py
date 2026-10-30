#!/usr/bin/env python3
"""ESTIL — Full de contacte: la peca nova a escala real (1x) i a 2x al costat
de dos personatges ancora i, si n'hi ha, d'una casa aprovada. Es l'ultima
comprovacio abans d'importar res: si a 1x no sembla del mateix joc que la
Skilla i l'Aigren, no entra.

    python3 tools/pixellab/contacto.py peca.png[:AMPLExALT] [peca2.png ...] [-o sortida.png]

Amb :AMPLExALT (cel·les de l'empremta, p. ex. casa.png:2x2) dibuixa el rombe
de l'empremta (64x32 per cel·la) sota la peca, amb la cantonada davantera al
punt mes baix del sprite: es veu si la base segueix la isometrica 2:1 del
motor i si la peca te la mida que li toca. Sense -o escriu
<peca>_contacto.png al costat de la primera peca.
"""
import os
import sys

from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from perspectiva import mesura  # noqa: E402

RAIZ = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TEX = os.path.join(RAIZ, "assets", "textures")
ANCORES = [("pixellab_personajes.png", 1), ("razas_andando.png", 2)]   # (hoja, fila) -> frame idle_down
CASA_APROVADA = os.path.join(TEX, "pixellab", "estil", "casa_aprovada.png")
ROMBE = {"vora": (255, 220, 80, 255), "terra": (84, 76, 58, 255), "linia": (112, 102, 76, 255)}


def frame(hoja, fila, col=7):
    sh = Image.open(os.path.join(TEX, hoja)).convert("RGBA")
    return sh.crop((col * 64, fila * 64, col * 64 + 64, fila * 64 + 64))


def amb_rombe(im, ancho, alto):
    """La peca sobre el rombe de la seva empremta (ancho x alto cel·les)."""
    r = mesura(im)
    ax, ay = r.get("apex", (im.width // 2, im.height))
    W, H = (ancho + alto) * 32, (ancho + alto) * 16
    cx = max(alto * 32, ax)
    base = max(H, ay)
    lienzo = Image.new("RGBA", (max(cx + ancho * 32, cx + im.width - ax) + 1, base + 1), (0, 0, 0, 0))
    d = ImageDraw.Draw(lienzo)
    izq = (cx - alto * 32, base - alto * 16)
    der = (cx + ancho * 32, base - ancho * 16)
    arriba = (cx + (ancho - alto) * 32, base - H)
    d.polygon([izq, arriba, der, (cx, base)], fill=ROMBE["terra"])
    for k in range(1, ancho):
        d.line([(izq[0] + k * 32, izq[1] - k * 16), (cx + k * 32, base - k * 16)], fill=ROMBE["linia"])
    for k in range(1, alto):
        d.line([(der[0] - k * 32, der[1] - k * 16), (cx - k * 32, base - k * 16)], fill=ROMBE["linia"])
    lienzo.paste(im, (cx - ax, base - ay), im)
    # La vora del rombe per sobre del sprite: les arestes de la base han de coincidir-hi.
    d.line([izq, (cx, base), der], fill=ROMBE["vora"])
    d.line([izq, arriba, der], fill=ROMBE["vora"] [:3] + (110,))
    return lienzo


def carrega(espec):
    """'peca.png' o 'peca.png:3x2' -> imatge retallada (amb rombe si cal)."""
    ruta, empremta = espec, None
    if ":" in espec and "x" in espec.rsplit(":", 1)[1]:
        ruta, mida = espec.rsplit(":", 1)
        empremta = tuple(int(v) for v in mida.lower().split("x"))
    im = Image.open(ruta).convert("RGBA")
    im = im.crop(im.getbbox()) if im.getbbox() else im
    return amb_rombe(im, *empremta) if empremta else im


def contacto(peces, sortida):
    figs = [frame(h, f) for h, f in ANCORES]
    casa = Image.open(CASA_APROVADA).convert("RGBA") if os.path.exists(CASA_APROVADA) else None
    items = [carrega(p) for p in peces]
    fila = items + figs + ([casa] if casa else [])
    alt = max(i.height for i in fila) + 20
    ample = sum(i.width * 2 + 12 for i in fila) + 12
    lienzo = Image.new("RGBA", (ample, alt * 3 + 30), (50, 50, 60, 255))
    d = ImageDraw.Draw(lienzo)
    for escala, y0 in ((1, 10), (2, alt + 20)):
        x = 12
        for im in fila:
            im2 = im if escala == 1 else im.resize((im.width * 2, im.height * 2), Image.NEAREST)
            lienzo.paste(im2, (x, y0 + (alt * escala - 10) - im2.height), im2)
            x += im2.width + 12
        d.text((12, y0 - 10 if y0 > 10 else 0), f"{escala}x", fill=(230, 230, 230, 255))
    lienzo = lienzo.crop(lienzo.getbbox())
    lienzo.save(sortida)
    print(f"{sortida}: {len(peces)} peces amb {len(figs)} personatges" + (" i la casa aprovada" if casa else ""))


if __name__ == "__main__":
    args = sys.argv[1:]
    sortida = None
    if "-o" in args:
        i = args.index("-o")
        sortida = args[i + 1]
        del args[i:i + 2]
    if not args:
        sys.exit(__doc__)
    contacto(args, sortida or os.path.splitext(args[0].split(":")[0])[0] + "_contacto.png")
