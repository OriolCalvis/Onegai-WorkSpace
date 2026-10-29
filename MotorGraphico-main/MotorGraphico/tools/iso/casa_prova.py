#!/usr/bin/env python3
"""ISO — La casa de prova del camí procedural (estil Habbo + Stardew): una
caseta de Boundington 2x2 amb pati davant, en les 4 orientacions.

    python3 tools/iso/casa_prova.py [sortida_dir]

Escriu casa_o0..o3.png (la peça retallada) i casa_vistes.png (les 4 vistes
amb el rombe de l'empremta i un personatge a escala).
"""
import os
import sys

from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from peces_iso import Escena, mata  # noqa: E402


def finestra(e, eix, c, s, u0, u1, v0, v1, porticons=True, jardinera=True, llavor=1):
    e.rect(eix, c, s, u0 - 1, u1 + 1, v0 - 1, v1 + 1, "noguera", to=4)
    e.rect(eix, c, s, u0, u1, v0, v1, "vidre", biaix=0.8)
    um, vm = (u0 + u1) / 2, (v0 + v1) / 2
    e.rect(eix, c, s, um - 1, um + 1, v0, v1, "noguera", biaix=1.0)
    e.rect(eix, c, s, u0, u1, vm - 1, vm + 1, "noguera", biaix=1.0)
    e.rect(eix, c, s, u0 + 1, u0 + 3, v0 + 2, v1 - 2, "vidre", to=0, biaix=1.1)      # reflex
    if porticons:
        w = (u1 - u0) / 2 + 1
        e.rect(eix, c, s, u0 - 1 - w, u0 - 1, v0 - 1, v1 + 1, "porticons", "taulons_v")
        e.rect(eix, c, s, u1 + 1, u1 + 1 + w, v0 - 1, v1 + 1, "porticons", "taulons_v")
    if jardinera:
        if eix == "x":
            x0, x1 = (c, c + 4) if s > 0 else (c - 4, c)
            e.caixa(x0, u0 - 1, v0 - 6, x1, u1 + 1, v0 - 1, "fusta", "taulons")
            for k, u in enumerate(range(int(u0) + 2, int(u1), 5)):
                e.sprite(((x0 + x1) / 2, u, v0 - 1), mata(3, 3, llavor=llavor + k))
        else:
            y0, y1 = (c, c + 4) if s > 0 else (c - 4, c)
            e.caixa(u0 - 1, y0, v0 - 6, u1 + 1, y1, v0 - 1, "fusta", "taulons")
            for k, u in enumerate(range(int(u0) + 2, int(u1), 5)):
                e.sprite((u, (y0 + y1) / 2, v0 - 1), mata(3, 3, llavor=llavor + k))


def entramat(e, eix, c, s, u0, u1, v0, v1):
    """Bigues de noguera sobre l'estuc de la planta alta: bandes, pals i creus."""
    e.rect(eix, c, s, u0, u1, v0, v0 + 3, "noguera")
    e.rect(eix, c, s, u0, u1, v1 - 3, v1, "noguera")
    um = (u0 + u1) / 2
    for u in (u0, um - 1.5, u1 - 3):
        e.rect(eix, c, s, u, u + 3, v0, v1, "noguera")
    for a, b in ((u0 + 3, um - 1.5), (um + 1.5, u1 - 3)):
        e.poligon_cara(eix, c, s, [(a, v0 + 3), (a + 3, v0 + 3), (b, v1 - 3), (b - 3, v1 - 3)], "noguera")


def casa():
    e = Escena(2, 2)
    # Pati davant (cara +Y): llambordes planes, sense gruix.
    e.cara([(2, 52, 0), (62, 52, 0), (62, 64, 0), (2, 64, 0)], (0, 0, 1),
           __import__("peces_iso").color_de("socol", "llambordes", "z", desp=-1), e.nova_peca())
    e.caixa(2, 2, 0, 62, 54, 5, "socol", "carreus", tex_dalt="llambordes")         # sòcol
    e.caixa(4, 4, 5, 60, 52, 44, "pedra", "carreus")                               # planta baixa
    e.caixa(2, 2, 44, 62, 54, 72, "estuc", "estuc")                                # planta alta, volada
    for eix, c, s, u0, u1 in (("y", 54, 1, 2, 62), ("y", 2, -1, 2, 62), ("x", 62, 1, 2, 54), ("x", 2, -1, 2, 54)):
        entramat(e, eix, c, s, u0, u1, 44, 72)
    e.teulada(2, 2, 62, 54, 72, 30, eix="x")                                       # ràfec 4: ràfec 3:2
    e.caixa(40, 8, 84, 50, 18, 120, "socol", "carreus")                            # xemeneia
    e.caixa(38, 6, 120, 52, 20, 125, "verdet")                                     # caputxa de vent de coure
    e.caixa(44, 12, 125, 46, 14, 134, "llauto")
    # Porta (cortina brodada fins a terra) i graó.
    e.rect("y", 52, 1, 20, 40, 5, 41, "noguera", to=4, arc=True)
    e.rect("y", 52, 1, 22, 38, 5, 39, "cortina", "taulons_v", arc=True, biaix=0.8)
    e.rect("y", 52, 1, 22, 38, 8, 10, "llauto", biaix=1.0)
    e.caixa(18, 52, 0, 42, 57, 5, "socol", dalt="socol")
    # Fanal dels Guardians al costat de la porta.
    e.caixa(44, 52, 30, 46, 55, 32, "ferro")
    e.caixa(44, 54, 22, 49, 58, 30, "cristall", dalt="llauto")
    e.caixa(43, 53, 30, 50, 59, 32, "llauto")
    # Finestres.
    finestra(e, "x", 60, 1, 18, 34, 16, 32, llavor=3)
    finestra(e, "y", 52, 1, 48, 56, 18, 32, porticons=False, llavor=7)
    finestra(e, "y", 54, 1, 10, 22, 50, 66, llavor=11)
    finestra(e, "y", 54, 1, 40, 52, 50, 66, llavor=15)
    finestra(e, "x", 62, 1, 20, 34, 50, 66, jardinera=False)
    finestra(e, "y", 2, -1, 24, 36, 50, 66, jardinera=False)
    finestra(e, "x", 2, -1, 20, 32, 50, 66, jardinera=False)
    finestra(e, "x", 4, -1, 16, 30, 16, 30, llavor=19)
    e.rect("y", 4, -1, 34, 48, 5, 34, "noguera", "taulons_v")                     # porta del darrere
    # Roba estesa a la cara +X: corda i peces.
    e.rect("x", 63, 1, 6, 52, 41, 42, "ferro", biaix=1.4)
    for k, (u, mat) in enumerate(((10, "roba1"), (18, "roba2"), (27, "roba3"), (36, "roba1"), (44, "roba2"))):
        e.rect("x", 63, 1, u, u + 6, 33 - (k % 2) * 3, 41, mat, biaix=1.5)
    # Pati: bóta, test amb flors i un fanalet de pal.
    e.cilindre(56, 59, 0, 13, 4.5, "fusta", "cercles", dalt="noguera")
    e.cilindre(10, 59, 0, 5, 3.5, "terra")
    e.sprite((10, 59, 5), mata(5, 5, llavor=23))
    e.sprite((48, 60, 0), mata(4, 4, llavor=29))
    e.caixa(5, 54, 0, 7, 56, 5, "socol")
    e.sprite((6, 55, 5), mata(4, 5, llavor=31))
    return e


def full(vistes, sortida, personatge=None):
    """Les 4 vistes sobre el rombe de l'empremta, amb un personatge a escala."""
    marge = 16
    ample = sum(im.width for im, _ in vistes) + marge * (len(vistes) + 2) + (64 if personatge else 0)
    alt = max(im.height for im, _ in vistes) + 2 * marge
    out = Image.new("RGBA", (ample, alt), (58, 60, 72, 255))
    d = ImageDraw.Draw(out)
    x = marge
    for k, (im, (ax, ay)) in enumerate(vistes):
        y = alt - marge - im.height
        out.alpha_composite(im, (x, y))
        d.text((x, 4), f"o{k}", fill=(235, 235, 235, 255))
        x += im.width + marge
    if personatge:
        out.alpha_composite(personatge, (x, alt - marge - personatge.height))
    out.save(sortida)


if __name__ == "__main__":
    dest = sys.argv[1] if len(sys.argv) > 1 else "."
    os.makedirs(dest, exist_ok=True)
    e = casa()
    vistes = []
    for o in range(4):
        im, anc = e.dibuixa(o)
        caixa = im.getbbox()
        im = im.crop(caixa)
        anc = (anc[0] - caixa[0], anc[1] - caixa[1])
        im.save(os.path.join(dest, f"casa_o{o}.png"))
        vistes.append((im, anc))
        print(f"o{o}: {im.size}, àncora (cantonada davantera) {anc}")
    arrel = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    pj = Image.open(os.path.join(arrel, "assets/textures/pixellab_personajes.png")).convert("RGBA").crop((448, 64, 512, 128))
    full(vistes, os.path.join(dest, "casa_vistes.png"), pj.crop(pj.getbbox()))
