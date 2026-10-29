#!/usr/bin/env python3
"""ISO — Monuments d'Aegroum amb la geometria de fantasia (octàgon dels
Guardians, rombe i cristall del vent, anells flotants) fets amb el kit
isomètric: les mateixes línies i la mateixa llum que les cases, i les 4
orientacions sense límit de mida.

    from monuments import temple_guardians
    e = temple_guardians()      # 7 x 5 cel·les
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from peces_iso import Escena, color_de  # noqa: E402


def poligon_regular(cx, cy, r, costats, gir=0.0):
    return [(cx + r * math.cos(gir + 2 * math.pi * k / costats), cy + r * math.sin(gir + 2 * math.pi * k / costats))
            for k in range(costats)]


def tronc(e, cx, cy, z0, z1, r0, r1, costats, mat, tex="llis", gir=0.0, dalt=None, peca=None):
    """Prisma (r0 == r1), tronc de piràmide o piràmide (r1 == 0) de base regular."""
    p = peca or e.nova_peca()
    a = poligon_regular(cx, cy, r0, costats, gir)
    b = poligon_regular(cx, cy, r1, costats, gir) if r1 > 0 else [(cx, cy)] * costats
    for k in range(costats):
        (xa, ya), (xb, yb) = a[k], a[(k + 1) % costats]
        (xc, yc), (xd, yd) = b[(k + 1) % costats], b[k]
        mx, my = (xa + xb) / 2 - cx, (ya + yb) / 2 - cy
        l = math.hypot(mx, my) or 1
        n = (mx / l * (z1 - z0), my / l * (z1 - z0), (r0 - r1) * math.cos(math.pi / costats))
        verts = [(xa, ya, z0), (xb, yb, z0), (xc, yc, z1), (xd, yd, z1)] if r1 > 0 else [(xa, ya, z0), (xb, yb, z0), (cx, cy, z1)]
        e.cara(verts, n, color_de(mat, tex, "y"), p)
    if r1 > 0:
        e.cara([(x, y, z1) for x, y in b], (0, 0, 1), color_de(dalt or mat, "llis", "z"), p)
    return p


def anell(e, cx, cy, z0, z1, r_ext, r_int, costats, mat, gir=0.0):
    """Anell flotant (màgia de vent): cares de fora, de dins i el damunt."""
    p = e.nova_peca()
    a = poligon_regular(cx, cy, r_ext, costats, gir)
    b = poligon_regular(cx, cy, r_int, costats, gir)
    for k in range(costats):
        (xa, ya), (xb, yb) = a[k], a[(k + 1) % costats]
        (xc, yc), (xd, yd) = b[k], b[(k + 1) % costats]
        mx, my = (xa + xb) / 2 - cx, (ya + yb) / 2 - cy
        e.cara([(xa, ya, z0), (xb, yb, z0), (xb, yb, z1), (xa, ya, z1)], (mx, my, 0), color_de(mat, "llis", "y"), p)
        e.cara([(xd, yd, z0), (xc, yc, z0), (xc, yc, z1), (xd, yd, z1)], (-mx, -my, 0), color_de(mat, "llis", "y", desp=1), p)
        e.cara([(xa, ya, z1), (xb, yb, z1), (xd, yd, z1), (xc, yc, z1)], (0, 0, 1), color_de(mat, "llis", "z"), p)


def finestra_pla(e, x0, y0, x1, y1, z0, z1, n, mat="vidre", marc="llauto"):
    """Finestra alta sobre una cara vertical qualsevol (de (x0,y0) a (x1,y1))."""
    dx, dy = x1 - x0, y1 - y0
    for (f, mt, b, to) in ((0.0, marc, 0.6, 3), (0.18, mat, 0.9, None)):
        ax, ay = x0 + dx * f, y0 + dy * f
        bx, by = x1 - dx * f, y1 - dy * f
        za, zb = z0 + (z1 - z0) * f * 0.5, z1 - (z1 - z0) * f * 0.3
        e.cara([(ax, ay, za), (bx, by, za), (bx, by, zb), ((ax + bx) / 2, (ay + by) / 2, zb + 4), (ax, ay, zb)],
               n, color_de(mt, "llis", "y", to), e.nova_peca(), b)


def temple_guardians():
    """El temple octagonal dels Guardians de la Llum: sala de vuit cares amb
    cúpula baixa de coure i fanal de cristall, dues torres romboides amb punta
    de cristall i un anell de coure que sura al voltant de la cúpula."""
    e = Escena(7, 5)
    W, D = e.W, e.D
    cx, cy = W / 2 + 8, D / 2 + 4
    R, H = 62, 96
    e.cara([(4, 4, 0), (W - 4, 4, 0), (W - 4, D - 2, 0), (4, D - 2, 0)], (0, 0, 1), color_de("socol", "llambordes", "z", desp=-1), e.nova_peca())
    tronc(e, cx, cy, 0, 6, R + 14, R + 12, 8, "socol", "carreus", gir=math.pi / 8)  # graonada
    tronc(e, cx, cy, 6, 11, R + 8, R + 6, 8, "socol", "carreus", gir=math.pi / 8)
    tronc(e, cx, cy, 11, H, R, R, 8, "pedra", "carreus", gir=math.pi / 8)           # la sala
    pts = poligon_regular(cx, cy, R + 0.5, 8, math.pi / 8)
    cares = []
    for k in range(8):
        (xa, ya), (xb, yb) = pts[k], pts[(k + 1) % 8]
        cares.append(((xa + xb) / 2 - cx, (ya + yb) / 2 - cy, xa, ya, xb, yb))
    porta = max(range(8), key=lambda k: cares[k][1])                                # la cara que mira a +Y
    for k, (mx, my, xa, ya, xb, yb) in enumerate(cares):
        lerp = lambda f: (xa + (xb - xa) * f, ya + (yb - ya) * f)
        if k == porta:
            (a1, b1), (a2, b2), (a3, b3) = lerp(0.22), lerp(0.78), lerp(0.5)
            e.cara([(a1, b1, 11), (a2, b2, 11), (a2, b2, 62), (a3, b3, 72), (a1, b1, 62)], (mx, my, 0),
                   color_de("llauto", "llis", "y", to_fix=3), e.nova_peca(), 0.6)
            (a1, b1), (a2, b2) = lerp(0.28), lerp(0.72)
            e.cara([(a1, b1, 11), (a2, b2, 11), (a2, b2, 60), (a3, b3, 68), (a1, b1, 60)], (mx, my, 0),
                   color_de("cortina", "taulons_v", "y"), e.nova_peca(), 0.9)
        else:
            (a1, b1), (a2, b2) = lerp(0.3), lerp(0.7)
            finestra_pla(e, a1, b1, a2, b2, 34, 80, (mx, my, 0))
    tronc(e, cx, cy, H, H + 7, R + 5, R + 5, 8, "pedra", gir=math.pi / 8, dalt="pedra")   # cornisa
    tronc(e, cx, cy, H + 7, H + 26, R - 2, R - 12, 8, "verdet", gir=math.pi / 8)          # cúpula de coure
    tronc(e, cx, cy, H + 26, H + 40, R - 12, R - 30, 8, "verdet", gir=math.pi / 8)
    tronc(e, cx, cy, H + 40, H + 48, R - 30, 12, 8, "verdet", gir=math.pi / 8)
    tronc(e, cx, cy, H + 48, H + 62, 8, 8, 8, "cristall", gir=math.pi / 8, dalt="llauto")  # fanal de llum
    tronc(e, cx, cy, H + 62, H + 76, 8, 0, 8, "llauto", gir=math.pi / 8)
    anell(e, cx, cy, H + 30, H + 35, R + 14, R + 7, 16, "llauto")                          # anell flotant
    for tx, ty in ((24, 24), (24, D - 40)):                                                # torres romboides
        tronc(e, tx, ty, 0, 150, 15, 15, 4, "pedra", "carreus")
        tronc(e, tx, ty, 60, 65, 17, 17, 4, "verdet")
        tronc(e, tx, ty, 150, 157, 18, 18, 4, "llauto")
        tronc(e, tx, ty, 157, 186, 12, 0, 4, "cristall")
    return e


if __name__ == "__main__":
    from full_vistes import full, vistes
    desti = "/home/claude/work/habbo/proc"
    os.makedirs(desti, exist_ok=True)
    out = full([("temple", vistes(temple_guardians(), desti, "temple"))], os.path.join(desti, "temple_vistes.png"))
    print(out.size)
