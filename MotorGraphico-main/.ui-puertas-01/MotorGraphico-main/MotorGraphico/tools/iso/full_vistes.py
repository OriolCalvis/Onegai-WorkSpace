#!/usr/bin/env python3
"""ISO — Full de vistes: cada peça en les 4 orientacions (una fila per peça),
cada vista sobre el rombe de la seva empremta, amb un personatge a escala.

    python3 tools/iso/full_vistes.py [sortida.png] [--x2]

Dibuixa les peces de prova d'edificis.py (caseta 2x2, casa 3x2 de pissarra,
taverna 4x3 i ferreria 4x3) i les desa a /home/claude/work/habbo/proc/.
"""
import os
import sys

from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from edificis import casa, local  # noqa: E402

ARREL = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def personatge():
    pj = Image.open(os.path.join(ARREL, "assets/textures/pixellab_personajes.png")).convert("RGBA").crop((448, 64, 512, 128))
    return pj.crop(pj.getbbox())


def rombe(d, cx, cy, ample_cel, fons_cel, color=(255, 214, 90, 255)):
    """Rombe de l'empremta amb la cantonada davantera a (cx, cy): costat esquerre
    = ample (eix x del mapa), costat dret = fons (eix y)."""
    esq = (cx - ample_cel * 32, cy - ample_cel * 16)
    dre = (cx + fons_cel * 32, cy - fons_cel * 16)
    dalt = (cx + (fons_cel - ample_cel) * 32, cy - (ample_cel + fons_cel) * 16)
    d.polygon([esq, dalt, dre, (cx, cy)], fill=(84, 92, 84, 255))
    d.line([esq, (cx, cy), dre], fill=color)


def vistes(escena, desti=None, nom="peca"):
    out = []
    for o in range(4):
        im, (ax, ay) = escena.dibuixa(o)
        caixa = im.getbbox()
        im = im.crop(caixa)
        anc = (ax - caixa[0], ay - caixa[1])
        if desti:
            im.save(os.path.join(desti, f"{nom}_o{o}.png"))
        a, f = (escena.W // 32, escena.D // 32) if o % 2 == 0 else (escena.D // 32, escena.W // 32)
        out.append((im, anc, a, f))
    return out


def full(files, sortida, x2=False, ors=None):
    """ors: les orientacions de cada columna (per defecte 0, 1, 2, 3)."""
    marge = 18
    pj = personatge()
    amples = [sum(max(im.width, (a + f) * 32) + marge for im, _, a, f in v) + pj.width + 2 * marge for _, v in files]
    alts = [max(im.height for im, _, _, _ in v) + 2 * marge for _, v in files]
    out = Image.new("RGBA", (max(amples), sum(alts)), (58, 60, 72, 255))
    d = ImageDraw.Draw(out)
    y0 = 0
    for (nom, v), alt in zip(files, alts):
        x = marge
        base = y0 + alt - marge
        for o, (im, (ax, ay), a, f) in enumerate(v):
            cx = x + max(ax, a * 32)
            rombe(d, cx, base, a, f)
            out.alpha_composite(im, (cx - ax, base - ay))
            d.text((x, y0 + 4), f"{nom} o{ors[o] if ors else o} ({a}x{f})", fill=(235, 235, 235, 255))
            x += max(im.width, (a + f) * 32) + marge
        out.alpha_composite(pj, (x, base - pj.height - 4))
        y0 += alt
    if x2:
        out = out.resize((out.width * 2, out.height * 2), Image.NEAREST)
    out.save(sortida)
    return out


if __name__ == "__main__":
    desti = "/home/claude/work/habbo/proc"
    os.makedirs(desti, exist_ok=True)
    peces = [
        ("caseta", casa(2, 2, plantes=2, llavor=1)),
        ("casa_pissarra", casa(3, 2, plantes=2, alta="fusta", teula="ferro", llavor=2, porticons="cortina")),
        ("taverna", local(4, 3, "taverna", llavor=3)),
        ("ferreria", local(4, 3, "ferreria", llavor=4, mur="socol", teula="ferro", porticons="roba2")),
        ("botiga", local(3, 3, "botiga", llavor=5, plantes=1)),
    ]
    files = [(nom, vistes(e, desti, nom)) for nom, e in peces]
    sortida = sys.argv[1] if len(sys.argv) > 1 and not sys.argv[1].startswith("--") else os.path.join(desti, "full_vistes.png")
    full(files, sortida, "--x2" in sys.argv)
    print(sortida)
