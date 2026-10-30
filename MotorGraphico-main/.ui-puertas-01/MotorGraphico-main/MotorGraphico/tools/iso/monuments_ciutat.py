#!/usr/bin/env python3
"""ISO (MON-01) — Els set monuments de Boundington amb el kit, a la manera
d'Habbo com les cases: la mateixa llum, el mateix contorn i els mateixos
materials, a la mida de la seva empremta al mapa, i cadascun amb la seva
identitat i la geometria d'Aegroum (octàgon dels Guardians, rombe i cristall
del vent, hexàgon de l'alquímia, anells que suren).

    from monuments_ciutat import MONUMENTS
    nom, (ample, fons, tile), fer = "castillo", MONUMENTS["castillo"]
    e = fer()                       # Escena; e.dibuixa(0, alt_max=...) -> (imatge, àncora)

La façana de la porta és la +Y (o0, com el generador: la porta a la fila de
baix de l'empremta, a la cel·la del mig) i només es dibuixen amb detall les
dues cares que es veuen (+Y i +X): els monuments no giren.
"""
import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cataleg_kit import porta_u  # noqa: E402
from edificis import finestra  # noqa: E402
from monuments import anell, poligon_regular, tronc  # noqa: E402
from patis import armes, bota, caixes, estendard  # noqa: E402
from peces_iso import MATERIALS, Escena, color_de, mata  # noqa: E402

R2 = math.sqrt(2)
# Els colors dels monuments d'abans (la seva essència), en rampes del kit.
MATERIALS.update({
    "pedra_castell": [(226, 228, 230), (200, 202, 208), (172, 174, 182), (138, 140, 150), (100, 102, 114)],
    "pissarra_fosca": [(130, 156, 194), (94, 120, 162), (68, 92, 134), (48, 66, 104), (32, 46, 76)],
    "pedra_grisa": [(220, 220, 212), (192, 192, 186), (160, 160, 156), (126, 126, 124), (92, 92, 92)],
    "pedra_fosca": [(160, 160, 164), (128, 128, 134), (100, 100, 108), (76, 76, 84), (54, 54, 62)],
    "sorra": [(236, 206, 150), (216, 180, 120), (190, 152, 96), (156, 120, 74), (118, 88, 56)],
    "coure": [(150, 214, 184), (104, 180, 150), (72, 146, 122), (50, 112, 96), (34, 80, 70)],
    "cera": [(255, 252, 236), (246, 238, 214), (222, 210, 182), (190, 176, 146), (150, 136, 110)],
    "or": [(255, 244, 176), (250, 214, 110), (222, 172, 66), (176, 126, 44), (124, 86, 30)],
})


# ---------------------------------------------------------------- peces comunes
def terra(e, x0, y0, x1, y1, mat="socol", tex="llambordes", desp=-1):
    e.cara([(x0, y0, 0), (x1, y0, 0), (x1, y1, 0), (x0, y1, 0)], (0, 0, 1), color_de(mat, tex, "z", desp=desp),
           e.nova_peca(), -1.0)


def mur(e, x0, y0, x1, y1, z0, z1, g, mat="socol", tex="carreus"):
    """Quatre murs al voltant d'un rectangle (buit per dins), de gruix g."""
    e.caixa(x0, y1 - g, z0, x1, y1, z1, mat, tex)
    e.caixa(x1 - g, y0, z0, x1, y1, z1, mat, tex)
    e.caixa(x0, y0, z0, x1, y0 + g, z1, mat, tex)
    e.caixa(x0, y0, z0, x0 + g, y1, z1, mat, tex)


def merlets(e, x0, y0, x1, y1, z, mat="socol", alt=7, g=4, pas=10, costats="sebo"):
    """Merlets sobre un rectangle: s (+Y), e (+X), b (-Y), o (-X)."""
    def fila(a0, a1, f):
        n = max(1, int((a1 - a0) // pas))
        w = (a1 - a0) / n
        for k in range(n):
            if k % 2 == 0:
                f(a0 + k * w, a0 + k * w + w * 0.6)
    if "s" in costats:
        fila(x0, x1, lambda a, b: e.caixa(a, y1 - g, z, b, y1, z + alt, mat, "carreus"))
    if "e" in costats:
        fila(y0, y1, lambda a, b: e.caixa(x1 - g, a, z, x1, b, z + alt, mat, "carreus"))
    if "b" in costats:
        fila(x0, x1, lambda a, b: e.caixa(a, y0, z, b, y0 + g, z + alt, mat, "carreus"))
    if "o" in costats:
        fila(y0, y1, lambda a, b: e.caixa(x0, a, z, x0 + g, b, z + alt, mat, "carreus"))


def torre_quadrada(e, cx, cy, h, z0, z1, mat="socol", tex="carreus", coberta="verdet", alt_coberta=None,
                   merlet=False, finestres=True):
    e.caixa(cx - h, cy - h, z0, cx + h, cy + h, z1, mat, tex)
    if finestres:
        for z in range(int(z0) + 26, int(z1) - 16, 30):
            finestra(e, "y", cy + h, 1, cx - 3, cx + 3, z, z + 10, estil="forat", jardinera=False, mat_p=mat)
            finestra(e, "x", cx + h, 1, cy - 3, cy + 3, z, z + 10, estil="forat", jardinera=False, mat_p=mat)
    if merlet:
        e.caixa(cx - h - 2, cy - h - 2, z1, cx + h + 2, cy + h + 2, z1 + 4, mat, dalt=mat)
        merlets(e, cx - h - 2, cy - h - 2, cx + h + 2, cy + h + 2, z1 + 4, mat, pas=7)
        z1 += 4
    if coberta:
        alt_c = alt_coberta or h * 1.6
        tronc(e, cx, cy, z1, z1 + 3, (h + 3) * R2, (h + 3) * R2, 4, coberta, gir=math.pi / 4)
        tronc(e, cx, cy, z1 + 3, z1 + 3 + alt_c, (h + 3) * R2, 0, 4, coberta, gir=math.pi / 4)
        return z1 + 3 + alt_c
    return z1


def torre_oct(e, cx, cy, r, z0, z1, mat="socol", tex="carreus", coberta="verdet", alt_coberta=None, merlet=False):
    """Torre octogonal (els Guardians) amb teulat cònic de coure o merlets."""
    g = math.pi / 8
    tronc(e, cx, cy, z0, z1, r, r, 8, mat, tex, gir=g)
    pts = poligon_regular(cx, cy, r + 0.3, 8, g)
    for k in range(8):
        (xa, ya), (xb, yb) = pts[k], pts[(k + 1) % 8]
        mx, my = (xa + xb) / 2 - cx, (ya + yb) / 2 - cy
        if mx + my <= 0:
            continue                                        # cares d'esquena
        for z in range(int(z0) + 30, int(z1) - 14, 34):
            f = 0.35
            e.cara([(xa + (xb - xa) * f, ya + (yb - ya) * f, z), (xb - (xb - xa) * f, yb - (yb - ya) * f, z),
                    (xb - (xb - xa) * f, yb - (yb - ya) * f, z + 10), (xa + (xb - xa) * f, ya + (yb - ya) * f, z + 10)],
                   (mx, my, 0), color_de("ferro", "llis", "y", to_fix=4), e.nova_peca(), 0.6)
    if merlet:
        tronc(e, cx, cy, z1, z1 + 4, r + 3, r + 3, 8, mat, gir=g, dalt=mat)
        for k, (x, y) in enumerate(poligon_regular(cx, cy, r + 1.5, 16, 0)):
            if k % 2 == 0:
                e.caixa(x - 2.2, y - 2.2, z1 + 4, x + 2.2, y + 2.2, z1 + 11, mat, "carreus")
        return z1 + 4
    if coberta:
        alt_c = alt_coberta or r * 1.7
        tronc(e, cx, cy, z1, z1 + 3, r + 4, r + 4, 8, coberta, gir=g)
        tronc(e, cx, cy, z1 + 3, z1 + 3 + alt_c, r + 4, 0, 8, coberta, gir=g)
        return z1 + 3 + alt_c
    return z1


def punta_cristall(e, cx, cy, z, alt=18, r=5, base="llauto"):
    """Punta de cristall romboide (el vent) sobre una base de llautó."""
    tronc(e, cx, cy, z, z + 3, r + 1, r + 1, 4, base)
    tronc(e, cx, cy, z + 3, z + 3 + alt * 0.35, r, r, 4, "cristall")
    tronc(e, cx, cy, z + 3 + alt * 0.35, z + 3 + alt, r, 0, 4, "cristall")


def teulada4(e, x0, y0, x1, y1, z0, alt, mat="pissarra", tex="teules", volat=4, vora=None):
    """Teulada a quatre aigües (pavelló): carener al llarg del costat llarg."""
    p = e.nova_peca()
    x0, y0, x1, y1 = x0 - volat, y0 - volat, x1 + volat, y1 + volat
    w, d = x1 - x0, y1 - y0
    zt = z0 + alt
    if w >= d:
        ym = (y0 + y1) / 2
        ra, rb = x0 + d / 2, x1 - d / 2
        e.careners.append((ra, ym, rb, ym, zt))
        e.cara([(x0, y1, z0), (x1, y1, z0), (rb, ym, zt), (ra, ym, zt)], (0, alt, d / 2), color_de(mat, tex, "tx"), p)
        e.cara([(x1, y0, z0), (x0, y0, z0), (ra, ym, zt), (rb, ym, zt)], (0, -alt, d / 2), color_de(mat, tex, "tx"), p)
        e.cara([(x1, y1, z0), (x1, y0, z0), (rb, ym, zt)], (alt, 0, d / 2), color_de(mat, tex, "ty"), p)
        e.cara([(x0, y0, z0), (x0, y1, z0), (ra, ym, zt)], (-alt, 0, d / 2), color_de(mat, tex, "ty"), p)
    else:
        xm = (x0 + x1) / 2
        ra, rb = y0 + w / 2, y1 - w / 2
        e.careners.append((xm, ra, xm, rb, zt))
        e.cara([(x1, y1, z0), (x1, y0, z0), (xm, ra, zt), (xm, rb, zt)], (alt, 0, w / 2), color_de(mat, tex, "ty"), p)
        e.cara([(x0, y0, z0), (x0, y1, z0), (xm, rb, zt), (xm, ra, zt)], (-alt, 0, w / 2), color_de(mat, tex, "ty"), p)
        e.cara([(x0, y1, z0), (x1, y1, z0), (xm, rb, zt)], (0, alt, w / 2), color_de(mat, tex, "tx"), p)
        e.cara([(x1, y0, z0), (x0, y0, z0), (xm, ra, zt)], (0, -alt, w / 2), color_de(mat, tex, "tx"), p)
    if vora:                                                 # la vora del ràfec, de llautó
        e.caixa(x0, y1 - 1, z0 - 2, x1, y1, z0, vora)
        e.caixa(x1 - 1, y0, z0 - 2, x1, y1, z0, vora)


def arc_fosc(e, eix, c, s, u0, u1, v0, v1, marc="pedra_clara"):
    """Obertura d'arc fosca (galeries, portes sense fulla) amb el seu marc."""
    e.rect(eix, c, s, u0 - 2, u1 + 2, v0, v1 + 2, marc, to=1, arc=True, biaix=0.6)
    e.rect(eix, c, s, u0, u1, v0, v1, "ferro", to=4, arc=True, biaix=0.9)


def portal(e, eix, c, s, u, z=0, ample=22, alt=48, mat_fulla="noguera", marc="pedra_clara"):
    """Portal d'arc amb fulles de fusta i ferramenta de llautó (la porta del monument)."""
    e.rect(eix, c, s, u - ample / 2 - 4, u + ample / 2 + 4, z, z + alt + 5, marc, to=1, arc=True, biaix=0.6)
    e.rect(eix, c, s, u - ample / 2, u + ample / 2, z, z + alt, mat_fulla, "taulons_v", arc=True, biaix=0.9)
    e.rect(eix, c, s, u - 0.5, u + 0.5, z, z + alt - 8, mat_fulla, to=4, biaix=1.0)
    for du in (-4, 3):
        e.rect(eix, c, s, u + du, u + du + 1, z + alt * 0.42, z + alt * 0.42 + 3, "llauto", to=0, biaix=1.1)


def columna(e, x, y, z0, z1, r=4, mat="pedra_clara"):
    e.caixa(x - r - 1, y - r - 1, z0, x + r + 1, y + r + 1, z0 + 3, mat)
    e.cilindre(x, y, z0 + 3, z1 - 3, r, mat, costats=8)
    e.caixa(x - r - 1.5, y - r - 1.5, z1 - 3, x + r + 1.5, y + r + 1.5, z1, mat)


def graons(e, x0, x1, y_paret, n=3, fons=5, alt=3, mat="pedra_clara"):
    """Escalinata davant d'una façana +Y."""
    for k in range(n):
        e.caixa(x0 - k * 3, y_paret, 0, x1 + k * 3, y_paret + fons * (n - k), alt * (k + 1), mat, dalt=mat)


def bandera(e, x, y, z, alt=30, mat="tela_verm"):
    """Pal amb bandera (la de la ciutat: vermella amb banda blanca)."""
    e.caixa(x - 0.8, y - 0.8, z, x + 0.8, y + 0.8, z + alt, "ferro")
    e.cara([(x + 0.8, y, z + alt - 12), (x + 13, y, z + alt - 14), (x + 13, y, z + alt - 3), (x + 0.8, y, z + alt - 1)],
           (0, 1, 0), color_de(mat, "llis", "y"), e.nova_peca(), 0.2, True)
    e.cara([(x + 0.8, y, z + alt - 7), (x + 13, y, z + alt - 9), (x + 13, y, z + alt - 7), (x + 0.8, y, z + alt - 5)],
           (0, 1, 0), color_de("tela", "llis", "y", to_fix=0), e.nova_peca(), 0.3, True)


# ---------------------------------------------------------------- els monuments
def torre_rodona(e, cx, cy, r, z0, z1, mat="pedra_castell", coberta="pissarra_fosca", alt_coberta=None,
                 anell_mat="or", punta=True, espitlleres=True):
    """Torre rodona amb el teulat cònic de pissarra (els castells d'abans), un
    anell d'or que sura al voltant del con i la punta de cristall."""
    e.cilindre(cx, cy, z0, z1, r, mat, "carreus", costats=16, dalt=mat)
    if espitlleres:
        for z in range(int(z0) + 28, int(z1) - 18, 36):
            for ang in (math.pi * 0.5, math.pi * 0.25, 0.0):        # les cares que es veuen (+Y, diagonal, +X)
                x, y = cx + (r + 0.3) * math.cos(ang), cy + (r + 0.3) * math.sin(ang)
                tx, ty = -math.sin(ang), math.cos(ang)
                e.cara([(x - tx * 1.6, y - ty * 1.6, z), (x + tx * 1.6, y + ty * 1.6, z), (x + tx * 1.6, y + ty * 1.6, z + 11),
                        (x - tx * 1.6, y - ty * 1.6, z + 11)], (math.cos(ang), math.sin(ang), 0),
                       color_de("ferro", "llis", "y", to_fix=4), e.nova_peca(), 0.6)
    e.cilindre(cx, cy, z1, z1 + 6, r + 3, mat, costats=16, dalt=mat)          # la cornisa
    alt_c = alt_coberta or r * 2.3
    tronc(e, cx, cy, z1 + 6, z1 + 6 + alt_c, r + 5, 0, 16, coberta, "teules")
    if anell_mat:
        za = z1 + 6 + alt_c * 0.42
        ra = (r + 5) * (1 - 0.42) + 9
        anell(e, cx, cy, za, za + 3, ra, ra - 3.5, 20, anell_mat)
    if punta:
        punta_cristall(e, cx, cy, z1 + 6 + alt_c - 2, alt=18, r=3.5, base="or")
    return z1 + 6 + alt_c


def castillo():
    """El castell (13x10), com el d'abans: un recinte quadrat de pedra grisa amb
    merlets grans, quatre torres rodones amb el teulat cònic de pissarra blava
    i la porta d'arc de fusta; ara les torres porten anells d'or que suren i
    punta de cristall, i al pati hi ha el cristall dels Guardians, surant sobre
    el seu pedestal octogonal amb un anell que hi dona voltes."""
    e = Escena(13, 10)
    W, D = e.W, e.D
    pc = porta_u(13)
    r, mz, g = 34, 96, 18                                  # radi de les torres, alçada i gruix del mur
    c0, c1 = 40, 40                                         # centres de les torres des de les vores
    x0, y0, x1, y1 = c0, c1, W - c0, D - c1
    e.cara([(x0, y0, 1), (x1, y0, 1), (x1, y1, 1), (x0, y1, 1)], (0, 0, 1), color_de("sorra", "llambordes", "z"),
           e.nova_peca(), -0.5)
    # els quatre murs entre torres: cara de pedra, camí de ronda i merlets grans a fora
    for (a0, b0, a1, b1, cost) in ((x0, y1 - g, x1, y1, "s"), (x1 - g, y0, x1, y1, "e"), (x0, y0, x1, y0 + g, "b"),
                                   (x0, y0, x0 + g, y1, "o")):
        e.caixa(a0, b0, 0, a1, b1, mz, "pedra_castell", "carreus", dalt="pedra_castell")
    for (cost, a0, b0, a1, b1) in (("s", x0 + r, y1 - g, x1 - r, y1), ("e", x1 - g, y0 + r, x1, y1 - r),
                                   ("b", x0 + r, y0, x1 - r, y0 + g), ("o", x0, y0 + r, x0 + g, y1 - r)):
        merlets(e, a0, b0, a1, b1, mz, "pedra_castell", alt=14, g=9, pas=13, costats=cost)
    # espitlleres a les cares que es veuen
    for u in range(int(x0 + r + 26), int(x1 - r - 10), 30):
        if abs(u - pc) > 44:
            e.rect("y", y1, 1, u - 1.6, u + 1.6, 40, 54, "ferro", to=4, biaix=0.6)
    for u in range(int(y0 + r + 26), int(y1 - r - 10), 30):
        e.rect("x", x1, 1, u - 1.6, u + 1.6, 40, 54, "ferro", to=4, biaix=0.6)
    # la porta: arc de pedra, fulles de fusta, escut i dues senyeres penjades
    portal(e, "y", y1, 1, pc, ample=30, alt=58, mat_fulla="noguera", marc="pedra_castell")
    e.rect("y", y1, 1, pc - 8, pc + 8, 70, 84, "pedra_castell", to=1, biaix=0.7)
    e.rect("y", y1, 1, pc - 5, pc + 5, 72, 82, "pissarra_fosca", to=2, biaix=0.9)
    e.rect("y", y1, 1, pc - 2, pc + 2, 74, 80, "or", to=1, biaix=1.0)
    for du in (-34, 34):
        e.rect("y", y1, 1, pc + du - 6, pc + du + 6, 44, 82, "tela_verm", biaix=0.7)
        e.poligon_cara("y", y1, 1, [(pc + du - 6, 44), (pc + du, 38), (pc + du + 6, 44)], "tela_verm", biaix=0.7)
        e.rect("y", y1, 1, pc + du - 6, pc + du + 6, 74, 77, "or", biaix=0.9)
        e.rect("y", y1, 1, pc + du - 1, pc + du + 1, 52, 66, "or", to=1, biaix=0.9)
    e.caixa(pc - 18, y1, 0, pc + 18, y1 + 10, 2, "pedra_castell", "llambordes", dalt="pedra_castell")
    # el cristall dels Guardians al pati
    cx, cy = W * 0.5, D * 0.47
    tronc(e, cx, cy, 0, 6, 26, 24, 8, "pedra_castell", gir=math.pi / 8, dalt="pedra_castell")
    tronc(e, cx, cy, 6, 16, 14, 12, 8, "pedra_castell", "carreus", gir=math.pi / 8, dalt="or")
    tronc(e, cx, cy, 36, 62, 0.1, 13, 4, "cristall")
    tronc(e, cx, cy, 62, 96, 13, 0, 4, "cristall")
    anell(e, cx, cy, 58, 61, 26, 22, 24, "or")
    anell(e, cx, cy, 80, 82, 19, 16, 24, "or")
    # les quatre torres, i les banderes de la ciutat
    for (tx, ty) in ((x0, y0), (x1, y0), (x0, y1), (x1, y1)):
        torre_rodona(e, tx, ty, r, 0, mz + 34)
    bandera(e, x1 + 2, y0 + 2, mz + 34 + 6 + r * 2.3 + 14, alt=26)
    return e


def mansarda(e, x0, y0, x1, y1, z0, alt_baix=56, entrada=26, alt_dalt=26, mat="pissarra_fosca", vora="or"):
    """Teulada a la mansarda (l'òpera): el pis de baix gairebé dret, el de dalt a
    quatre aigües i suau, i els cordons d'or a la vora i al canvi de pendent."""
    p = e.nova_peca()
    i = entrada
    z1 = z0 + alt_baix
    ix0, iy0, ix1, iy1 = x0 + i, y0 + i, x1 - i, y1 - i
    e.cara([(x0, y1, z0), (x1, y1, z0), (ix1, iy1, z1), (ix0, iy1, z1)], (0, alt_baix, i), color_de(mat, "teules", "tx"), p)
    e.cara([(x1, y1, z0), (x1, y0, z0), (ix1, iy0, z1), (ix1, iy1, z1)], (alt_baix, 0, i), color_de(mat, "teules", "ty"), p)
    e.cara([(x1, y0, z0), (x0, y0, z0), (ix0, iy0, z1), (ix1, iy0, z1)], (0, -alt_baix, i), color_de(mat, "teules", "tx"), p)
    e.cara([(x0, y0, z0), (x0, y1, z0), (ix0, iy1, z1), (ix0, iy0, z1)], (-alt_baix, 0, i), color_de(mat, "teules", "ty"), p)
    teulada4(e, ix0, iy0, ix1, iy1, z1, alt_dalt, mat, volat=0)
    if vora:
        for (a, b, c, d) in ((x0, y1 - 1.5, x1, y1 + 0.5), (x1 - 1.5, y0, x1 + 0.5, y1)):
            e.caixa(a, b, z0 - 2, c, d, z0 + 1, vora)                       # el ràfec
        for (a, b, c, d) in ((ix0 - 1, iy1 - 1, ix1 + 1, iy1 + 1.2), (ix1 - 1, iy0 - 1, ix1 + 1.2, iy1 + 1),
                             (ix0 - 1, iy0 - 1, ix1 + 1, iy0 + 1), (ix0 - 1, iy0 - 1, ix0 + 1, iy1 + 1)):
            e.caixa(a, b, z1 - 1, c, d, z1 + 1.5, vora)                     # el canvi de pendent
    return z1, (ix0, iy0, ix1, iy1)


def finestra_arc(e, eix, c, s, u0, u1, v0, v1, vidre="pissarra_fosca", marc="pedra_clara", reixa="or"):
    """Finestra alta d'arc de mig punt (edificis nobles): marc de pedra, vidre
    fosc blau i la creu de llautó."""
    e.rect(eix, c, s, u0 - 2, u1 + 2, v0 - 2, v1 + 2, marc, to=1, arc=True, biaix=0.6)
    e.rect(eix, c, s, u0, u1, v0, v1, vidre, to=3, arc=True, biaix=0.8)
    e.rect(eix, c, s, u0 + 1, u0 + 2.5, v0 + 2, v1 - 6, vidre, to=1, biaix=1.0)
    um = (u0 + u1) / 2
    e.rect(eix, c, s, um - 0.5, um + 0.5, v0, v1 - 1, reixa, to=2, biaix=1.1)
    e.rect(eix, c, s, u0, u1, (v0 + v1) / 2 - 0.5, (v0 + v1) / 2 + 0.5, reixa, to=2, biaix=1.1)


def pilastra(e, eix, c, s, u, z0, z1, mat="pedra_clara", w=3):
    if eix == "y":
        yb = c if s > 0 else c - 2
        e.caixa(u - w, yb, z0, u + w, yb + 2, z1, mat)
        e.caixa(u - w - 1, yb, z1 - 3, u + w + 1, yb + 3, z1, mat)
    else:
        xb = c if s > 0 else c - 2
        e.caixa(xb, u - w, z0, xb + 2, u + w, z1, mat)
        e.caixa(xb, u - w - 1, z1 - 3, xb + 3, u + w + 1, z1, mat)


def lluerna(e, eix, x, y, z, mat="pedra_clara", teula="pissarra_fosca", vora="or", w=10, h=18):
    """Lucana (finestra al teulat) amb frontó i vora d'or, cap a +Y o +X."""
    if eix == "y":
        e.caixa(x - w, y - 16, z, x + w, y, z + h, mat)
        e.teulada(x - w, y - 16, x + w, y + 2, z + h, w * 0.8, mat=teula, voladis=2, gruix=2, mat_timpa=mat, eix="y")
        finestra_arc(e, "y", y, 1, x - w * 0.45, x + w * 0.45, z + 3, z + h - 3)
        e.caixa(x - w - 1, y, z + h - 1, x + w + 1, y + 1.2, z + h + 1, vora)
    else:
        e.caixa(x - 16, y - w, z, x, y + w, z + h, mat)
        e.teulada(x - 16, y - w, x + 2, y + w, z + h, w * 0.8, mat=teula, voladis=2, gruix=2, mat_timpa=mat, eix="x")
        finestra_arc(e, "x", x, 1, y - w * 0.45, y + w * 0.45, z + 3, z + h - 3)
        e.caixa(x, y - w - 1, z + h - 1, x + 1.2, y + w + 1, z + h + 1, vora)


def opera():
    """L'òpera (10x9), com la d'abans: pedra clara amb pilastres i finestres
    d'arc, la gran teulada de pissarra blava amb les arestes d'or, lucanes i la
    claraboia de vidre al capdamunt, i el pòrtic de columnes amb frontó i
    escalinata davant de la porta. La màgia: una corona d'or i cristall que
    sura sobre la claraboia i fanals de cristall a les columnes."""
    e = Escena(10, 9)
    W, D = e.W, e.D
    pc = porta_u(10)
    x0, y0, x1, y1, zc = 14, 14, W - 14, D - 40, 78
    e.caixa(x0 - 3, y0 - 3, 0, x1 + 3, y1 + 3, 8, "pedra_clara", "carreus", dalt="pedra_clara")     # sòcol
    e.caixa(x0, y0, 8, x1, y1, zc, "pedra_clara", "carreus", dalt="pedra_clara")
    e.caixa(x0 - 3, y0 - 3, zc, x1 + 3, y1 + 3, zc + 6, "pedra_clara", dalt="pedra_clara")         # cornisa
    e.caixa(x0 - 3, y1 + 2, zc + 5, x1 + 3, y1 + 3, zc + 6.5, "or")
    e.caixa(x1 + 2, y0 - 3, zc + 5, x1 + 3, y1 + 3, zc + 6.5, "or")
    # façanes que es veuen: pilastres i finestres d'arc
    us = [x0 + 20 + k * (x1 - x0 - 40) / 6 for k in range(7)]
    for k, u in enumerate(us):
        pilastra(e, "y", y1, 1, u, 8, zc)
        if k < 6:
            m = (u + us[k + 1]) / 2
            if abs(m - pc) > 34:
                finestra_arc(e, "y", y1, 1, m - 6, m + 6, 24, 58)
    vs = [y0 + 18 + k * (y1 - y0 - 36) / 5 for k in range(6)]
    for k, v in enumerate(vs):
        pilastra(e, "x", x1, 1, v, 8, zc)
        if k < 5:
            m = (v + vs[k + 1]) / 2
            finestra_arc(e, "x", x1, 1, m - 6, m + 6, 24, 58)
    # la teulada a la mansarda, de pissarra blava amb cordons d'or, lucanes i claraboia
    rx0, ry0, rx1, ry1 = x0 - 3, y0 - 3, x1 + 3, y1 + 3
    zb, (ix0, iy0, ix1, iy1) = mansarda(e, rx0, ry0, rx1, ry1, zc + 6, alt_baix=74, entrada=58, alt_dalt=34)
    for u in (x0 + 44, x0 + 96, x1 - 96, x1 - 44):
        if abs(u - pc) > 40:
            lluerna(e, "y", u, y1 - 4, zc + 14)
    for v in (y0 + 52, (y0 + y1) / 2, y1 - 52):
        lluerna(e, "x", x1 - 4, v, zc + 14)
    # claraboia de vidre amb el marc d'or, al capdamunt
    cxm0, cym0 = (ix0 + ix1) / 2, (iy0 + iy1) / 2
    cx0, cx1, cy0, cy1 = cxm0 - 46, cxm0 + 46, cym0 - 20, cym0 + 20
    zt = zb + 30
    e.caixa(cx0, cy0, zb, cx1, cy1, zt, "vidre", dalt="vidre")
    teulada4(e, cx0, cy0, cx1, cy1, zt, 16, "vidre", tex="llis", volat=2, vora="or")
    for u in range(int(cx0), int(cx1) + 1, 11):
        e.caixa(u - 0.8, cy1 + 0.2, zb, u + 0.8, cy1 + 1.4, zt, "or")
    for v in range(int(cy0), int(cy1) + 1, 10):
        e.caixa(cx1 + 0.2, v - 0.8, zb, cx1 + 1.4, v + 0.8, zt, "or")
    e.caixa(cx0, cy1 - 0.5, zt - 2, cx1 + 1.4, cy1 + 1.6, zt, "or")
    e.caixa(cx1 - 0.5, cy0, zt - 2, cx1 + 1.6, cy1 + 1.4, zt, "or")
    zt += 16
    # pinacles d'or a les cantonades del teulat
    for (px, py) in ((rx1, ry1), (rx0, ry1), (rx1, ry0)):
        tronc(e, px, py, zc + 6, zc + 12, 3.5, 3.5, 8, "or")
        tronc(e, px, py, zc + 12, zc + 26, 3.5, 0, 8, "or")
        punta_cristall(e, px, py, zc + 25, alt=8, r=2, base="or")
    # el pòrtic: columnes, entaulament, frontó amb l'òcul d'or, i l'escalinata
    py1 = D - 6
    e.caixa(pc - 62, y1, 0, pc + 62, py1, 10, "pedra_clara", "carreus", dalt="pedra_clara")
    graons(e, pc - 56, pc + 56, py1, n=3, fons=2, alt=3)
    for u in (pc - 52, pc - 20, pc + 20, pc + 52):
        columna(e, u, py1 - 8, 10, 84, r=5.5)
    e.caixa(pc - 60, y1, 84, pc + 60, py1 - 2, 96, "pedra_clara", dalt="pedra_clara")
    e.caixa(pc - 60, py1 - 3, 93, pc + 60, py1 - 2, 96, "or")
    e.cara([(pc - 60, py1 - 2, 96), (pc + 60, py1 - 2, 96), (pc, py1 - 2, 126)], (0, 1, 0),
           color_de("pedra_clara", "llis", "y", to_fix=1), e.nova_peca(), 0.4)
    e.cara([(pc + 60, y1, 96), (pc + 60, py1 - 2, 96), (pc, py1 - 2, 126), (pc, y1, 126)], (30, 0, 60),
           color_de("pissarra_fosca", "teules", "ty"), e.nova_peca())
    for (a, b, z0, z1) in ((pc - 60, pc, 96, 126), (pc, pc + 60, 126, 96)):
        n = 10
        for k in range(n):                                    # el cordó d'or del frontó
            ua, ub = a + (b - a) * k / n, a + (b - a) * (k + 1) / n
            za, zb2 = z0 + (z1 - z0) * k / n, z0 + (z1 - z0) * (k + 1) / n
            e.cara([(ua, py1 - 1.8, za), (ub, py1 - 1.8, zb2), (ub, py1 - 1.8, zb2 + 2), (ua, py1 - 1.8, za + 2)], (0, 1, 0),
                   color_de("or", "llis", "y", to_fix=1), e.nova_peca(), 0.7)
    e.cara([(pc - 12, py1 - 1.6, 100), (pc + 12, py1 - 1.6, 100), (pc + 12, py1 - 1.6, 114), (pc - 12, py1 - 1.6, 114)],
           (0, 1, 0), color_de("or", "llis", "y", to_fix=2), e.nova_peca(), 0.6)
    e.cara([(pc - 8, py1 - 1.2, 102), (pc + 8, py1 - 1.2, 102), (pc + 8, py1 - 1.2, 112), (pc - 8, py1 - 1.2, 112)],
           (0, 1, 0), color_de("cristall", "llis", "y", to_fix=2), e.nova_peca(), 0.8)
    tronc(e, pc, py1 - 3, 126, 140, 3.5, 0, 8, "or")
    punta_cristall(e, pc, py1 - 3, 138, alt=10, r=2.5, base="or")
    portal(e, "y", y1, 1, pc, ample=24, alt=50, mat_fulla="noguera", marc="pedra_clara")
    for u in (pc - 36, pc + 36):                             # fanals de cristall entre columnes
        e.caixa(u - 0.8, py1 - 9, 10, u + 0.8, py1 - 7.4, 44, "ferro")
        tronc(e, u, py1 - 8, 44, 54, 3.5, 3.5, 8, "cristall", dalt="or")
    # la màgia: la corona d'or i cristall que sura sobre la claraboia
    cxm, cym = (cx0 + cx1) / 2, (cy0 + cy1) / 2
    anell(e, cxm, cym, zt + 22, zt + 25, 40, 36, 24, "or")
    for k, (x, y) in enumerate(poligon_regular(cxm, cym, 38, 8, math.pi / 8)):
        tronc(e, x, y, zt + 25, zt + 29, 2.5, 2.5, 4, "or")
        tronc(e, x, y, zt + 29, zt + 40 + (k % 2) * 5, 2.5, 0, 4, "cristall")
    anell(e, cxm, cym, zt + 10, zt + 12, 27, 24, 20, "cristall")
    return e


def finestra_gotica(e, eix, c, s, u0, u1, v0, v1, marc="pedra_grisa", vidre="pissarra_fosca", buida=False):
    """Finestra d'arc apuntat (gòtic): marc de pedra, vidre fosc (o buida: negra)."""
    um, h = (u0 + u1) / 2, (u1 - u0) * 0.9
    for (d, mat, to, b) in ((2, marc, 1, 0.6), (0, "ferro" if buida else vidre, 4 if buida else 3, 0.8)):
        uv = [(u0 - d, v0 - d), (u1 + d, v0 - d), (u1 + d, v1 - h * 0.4), (um, v1 + d), (u0 - d, v1 - h * 0.4)]
        e.poligon_cara(eix, c, s, uv, mat, to=to, biaix=b)
    if not buida:
        e.rect(eix, c, s, um - 0.5, um + 0.5, v0, v1 - 2, marc, to=2, biaix=1.0)
        e.rect(eix, c, s, u0 + 1, u0 + 2.2, v0 + 2, v1 - h * 0.5, vidre, to=1, biaix=1.0)


def ocul(e, eix, c, s, u, v, r, marc="pedra_grisa", vidre="vidre"):
    """Finestra rodona (òcul) amb el marc de pedra i la creu."""
    for (rr, mat, to, b) in ((r + 2.2, marc, 1, 0.6), (r, vidre, 3, 0.8)):
        e.poligon_cara(eix, c, s, [(u + rr * math.cos(2 * math.pi * k / 16), v + rr * math.sin(2 * math.pi * k / 16))
                                   for k in range(16)], mat, to=to, biaix=b)
    e.rect(eix, c, s, u - 0.5, u + 0.5, v - r, v + r, marc, to=2, biaix=1.0)
    e.rect(eix, c, s, u - r, u + r, v - 0.5, v + 0.5, marc, to=2, biaix=1.0)
    e.poligon_cara(eix, c, s, [(u - r * 0.6, v + r * 0.1), (u - r * 0.15, v + r * 0.6), (u - r * 0.3, v + r * 0.75),
                               (u - r * 0.75, v + r * 0.25)], vidre, to=0, biaix=1.1)


def contrafort(e, eix, c, s, u, z1, mat="pedra_grisa", w=4, sortida=7, pinacle=False):
    """Contrafort de pedra adossat a la façana (+Y o +X), amb el seu pinacle."""
    if eix == "y":
        e.caixa(u - w, c, 0, u + w, c + sortida, z1 * 0.55, mat, "carreus")
        e.caixa(u - w + 1, c, z1 * 0.55, u + w - 1, c + sortida * 0.55, z1, mat, "carreus")
        top = (u, c + sortida * 0.28)
    else:
        e.caixa(c, u - w, 0, c + sortida, u + w, z1 * 0.55, mat, "carreus")
        e.caixa(c, u - w + 1, z1 * 0.55, c + sortida * 0.55, u + w - 1, z1, mat, "carreus")
        top = (c + sortida * 0.28, u)
    if pinacle:
        tronc(e, top[0], top[1], z1, z1 + 14, w - 0.5, 0, 4, mat, gir=math.pi / 4)


def rellotge(e, eix, c, s, u, v, r=9):
    """La cara del rellotge: esfera blanca, marc d'or i les dues agulles."""
    for (rr, mat, to, b) in ((r + 1.8, "or", 2, 0.7), (r, "tela", 0, 0.9)):
        e.poligon_cara(eix, c, s, [(u + rr * math.cos(2 * math.pi * k / 16), v + rr * math.sin(2 * math.pi * k / 16))
                                   for k in range(16)], mat, to=to, biaix=b)
    e.rect(eix, c, s, u - 0.6, u + 0.6, v, v + r * 0.75, "ferro", to=4, biaix=1.1)
    e.poligon_cara(eix, c, s, [(u, v - 0.6), (u + r * 0.55, v - 0.6), (u + r * 0.55, v + 0.6), (u, v + 0.6)],
                   "ferro", to=4, biaix=1.1)


def universidad():
    """La universitat (10x9), com la d'abans: pedra grisa, dues plantes amb
    finestres gòtiques i contraforts, teulades de coure verd, el pavelló de la
    porta amb el rellotge al frontó i, darrere, la torre emmerletada amb
    l'agulla de coure. La màgia: anells d'or que suren al voltant de l'agulla,
    punta de cristall i un orbe de llum sobre el pavelló."""
    e = Escena(10, 9)
    W, D = e.W, e.D
    pc = porta_u(10)
    terra(e, pc - 26, D - 30, pc + 26, D - 2, "pedra_grisa", "llambordes", desp=0)
    # el cos llarg, de dues plantes
    bx0, by0, bx1, by1, zb = 14, 70, W - 14, D - 66, 86
    e.caixa(bx0, by0, 0, bx1, by1, zb, "pedra_grisa", "carreus")
    e.caixa(bx0 - 2, by1 - 1, 40, bx1 + 2, by1 + 2, 43, "pedra_grisa")                 # la cornisa del primer pis
    e.caixa(bx1 - 1, by0 - 2, 40, bx1 + 2, by1 + 2, 43, "pedra_grisa")
    e.teulada(bx0, by0, bx1, by1, zb, 46, mat="coure", tex="teules", voladis=3, gruix=3, mat_timpa="pedra_grisa",
              tex_timpa="carreus", eix="x")
    for u in (bx0 + 70, bx1 - 60):                                                      # xemeneies
        e.caixa(u - 6, (by0 + by1) / 2 - 6, zb + 20, u + 6, (by0 + by1) / 2 + 6, zb + 60, "pedra_grisa", "carreus")
        e.caixa(u - 7.5, (by0 + by1) / 2 - 7.5, zb + 60, u + 7.5, (by0 + by1) / 2 + 7.5, zb + 64, "pedra_grisa")
    for u in [bx0 + 24 + k * 34 for k in range(9)]:
        if abs(u - pc) < 50:
            continue
        for z in (10, 52):
            finestra_gotica(e, "y", by1, 1, u - 5, u + 5, z, z + 22)
    for k, v in enumerate([by0 + 22 + j * 32 for j in range(4)]):
        for z in (10, 52):
            finestra_gotica(e, "x", bx1, 1, v - 5, v + 5, z, z + 22)
    for u in (bx0 + 6, bx0 + 108, bx1 - 108, bx1 - 6):
        contrafort(e, "y", by1, 1, u, 70)
    # la torre emmerletada amb l'agulla de coure, darrere del pavelló
    tx, ty, th = pc, by0 + 34, 30
    e.caixa(tx - th, ty - th, 0, tx + th, ty + th, 178, "pedra_grisa", "carreus")
    for z in (110, 146):
        finestra_gotica(e, "y", ty + th, 1, tx - 9, tx - 1, z, z + 24)
        finestra_gotica(e, "y", ty + th, 1, tx + 1, tx + 9, z, z + 24)
        finestra_gotica(e, "x", tx + th, 1, ty - 9, ty - 1, z, z + 24)
        finestra_gotica(e, "x", tx + th, 1, ty + 1, ty + 9, z, z + 24)
    e.caixa(tx - th - 3, ty - th - 3, 178, tx + th + 3, ty + th + 3, 184, "pedra_grisa", dalt="pedra_grisa")
    merlets(e, tx - th - 3, ty - th - 3, tx + th + 3, ty + th + 3, 184, "pedra_grisa", alt=10, g=6, pas=11)
    tronc(e, tx, ty, 184, 266, (th - 4) * R2, 0, 4, "coure", "teules", gir=math.pi / 4)
    for (dx, dy) in ((th, th), (-th, th), (th, -th)):                                   # agulletes a les cantonades
        tronc(e, tx + dx * 0.8, ty + dy * 0.8, 184, 190, 4, 4, 8, "pedra_grisa")
        tronc(e, tx + dx * 0.8, ty + dy * 0.8, 190, 214, 4, 0, 8, "coure")
    punta_cristall(e, tx, ty, 262, alt=22, r=4, base="or")
    anell(e, tx, ty, 220, 223, 34, 30, 24, "or")
    anell(e, tx, ty, 240, 242, 24, 21, 20, "or")
    # el pavelló de la porta, amb el rellotge al frontó
    px0, px1, py0, py1, zp = pc - 34, pc + 34, by1 - 10, D - 30, 98
    e.caixa(px0, py0, 0, px1, py1, zp, "pedra_grisa", "carreus")
    e.teulada(px0, py0, px1, py1, zp, 34, mat="coure", tex="teules", voladis=3, gruix=3, mat_timpa="pedra_grisa",
              tex_timpa="carreus", eix="y")
    portal(e, "y", py1, 1, pc, ample=26, alt=52, mat_fulla="noguera", marc="pedra_grisa")
    for u in (pc - 12, pc, pc + 12):
        finestra_gotica(e, "y", py1, 1, u - 4, u + 4, 64, 84)
    rellotge(e, "y", py1, 1, pc, zp + 14, r=9)
    finestra_gotica(e, "x", px1, 1, (py0 + py1) / 2 - 6, (py0 + py1) / 2 + 6, 30, 60)
    for u in (px0, px1):
        contrafort(e, "y", py1, 1, u, 92, pinacle=True)
    e.caixa(pc - 3, py1 - 4, zp + 34, pc + 3, py1 + 2, zp + 40, "pedra_grisa")
    tronc(e, pc, py1 - 1, zp + 50, zp + 58, 0.1, 5, 8, "cristall")                       # l'orbe de llum
    tronc(e, pc, py1 - 1, zp + 58, zp + 66, 5, 0.1, 8, "cristall")
    return e


def biblioteca():
    """La biblioteca (12x9), com la d'abans: un bloc de pedra grisa de terrat
    pla amb merlets, finestres rodones, la nau de teula vermella que acaba en
    el pòrtic de columnes i frontó, i la cúpula de pedra sobre el tambor de
    finestres blaves. La màgia: llibres que suren en rotllana al voltant de la
    cúpula, un anell d'or i alambins de cristall als angles."""
    e = Escena(12, 9)
    W, D = e.W, e.D
    pc = porta_u(12)
    x0, y0, x1, y1, zb = 12, 12, W - 12, D - 34, 72
    e.caixa(x0, y0, 0, x1, y1, zb, "pedra_grisa", "carreus")
    e.cara([(x0 + 1, y0 + 1, zb + 0.1), (x1 - 1, y0 + 1, zb + 0.1), (x1 - 1, y1 - 1, zb + 0.1), (x0 + 1, y1 - 1, zb + 0.1)],
           (0, 0, 1), color_de("pedra_grisa", "llambordes", "z", desp=-1), e.nova_peca(), 0.1)
    e.caixa(x0 - 2, y1 - 2, zb - 6, x1 + 2, y1 + 2, zb, "pedra_grisa")
    e.caixa(x1 - 2, y0 - 2, zb - 6, x1 + 2, y1 + 2, zb, "pedra_grisa")
    merlets(e, x0 - 2, y0 - 2, x1 + 2, y1 + 2, zb, "pedra_grisa", alt=11, g=7, pas=16)
    for u in [x0 + 28 + k * 42 for k in range(9)]:
        if abs(u - pc) < 70:
            continue
        ocul(e, "y", y1, 1, u, 40, 9)
        if u + 21 < x1 - 8:
            pilastra(e, "y", y1, 1, u + 21, 4, zb - 6, mat="pedra_grisa")
    for v in [y0 + 30 + k * 46 for k in range(5)]:
        ocul(e, "x", x1, 1, v, 40, 9)
        if v + 23 < y1 - 8:
            pilastra(e, "x", x1, 1, v + 23, 4, zb - 6, mat="pedra_grisa")
    # la nau de teula vermella fins al pòrtic
    nx0, nx1 = pc - 60, pc + 60
    e.caixa(nx0, y0 + 40, zb, nx1, y1 + 2, zb + 10, "pedra_grisa", dalt="pedra_grisa")
    e.teulada(nx0, y0 + 40, nx1, y1 + 2, zb + 10, 40, mat="teula", tex="teules", voladis=4, gruix=3,
              mat_timpa="pedra_grisa", tex_timpa="carreus", eix="y")
    ocul(e, "y", y1 + 2, 1, pc, zb + 26, 7)
    # el pòrtic: dues columnes, frontó, porta i escalinata
    py1 = D - 8
    e.caixa(pc - 40, y1, 0, pc + 40, py1, 8, "pedra_grisa", "carreus", dalt="pedra_grisa")
    graons(e, pc - 34, pc + 34, py1, n=3, fons=2, alt=2.6, mat="pedra_grisa")
    for u in (pc - 30, pc + 30):
        columna(e, u, py1 - 7, 8, 66, r=4.5, mat="pedra_grisa")
    e.caixa(pc - 38, y1, 66, pc + 38, py1 - 2, 74, "pedra_grisa", dalt="pedra_grisa")
    e.cara([(pc - 38, py1 - 2, 74), (pc + 38, py1 - 2, 74), (pc, py1 - 2, 94)], (0, 1, 0),
           color_de("pedra_grisa", "carreus", "y", to_fix=1), e.nova_peca(), 0.4)
    e.cara([(pc + 38, y1, 74), (pc + 38, py1 - 2, 74), (pc, py1 - 2, 94), (pc, y1, 94)], (20, 0, 38),
           color_de("teula", "teules", "ty"), e.nova_peca())
    ocul(e, "y", py1 - 1.6, 1, pc, 82, 4)
    portal(e, "y", y1, 1, pc, ample=24, alt=50, mat_fulla="noguera", marc="pedra_grisa")
    # la cúpula sobre el tambor, a l'esquerra del terrat
    cx, cy = W * 0.27, D * 0.4
    e.cilindre(cx, cy, zb, zb + 34, 46, "pedra_grisa", "carreus", costats=24, dalt="pedra_grisa")
    for k in range(24):
        ang = 2 * math.pi * (k + 0.5) / 24
        if math.cos(ang) + math.sin(ang) < 0.2 or k % 2:
            continue
        x, y = cx + 46.4 * math.cos(ang), cy + 46.4 * math.sin(ang)
        tx_, ty_ = -math.sin(ang), math.cos(ang)
        e.cara([(x - tx_ * 3.5, y - ty_ * 3.5, zb + 8), (x + tx_ * 3.5, y + ty_ * 3.5, zb + 8),
                (x + tx_ * 3.5, y + ty_ * 3.5, zb + 22), (x, y, zb + 26), (x - tx_ * 3.5, y - ty_ * 3.5, zb + 22)],
               (math.cos(ang), math.sin(ang), 0), color_de("vidre", "llis", "y", to_fix=2), e.nova_peca(), 0.7)
    e.cilindre(cx, cy, zb + 34, zb + 38, 50, "pedra_grisa", costats=24, dalt="pedra_grisa")
    zc = zb + 38
    for (z0, z1, r0, r1) in ((0, 14, 46, 42), (14, 26, 42, 34), (26, 36, 34, 23), (36, 42, 23, 10)):
        tronc(e, cx, cy, zc + z0, zc + z1, r0, r1, 24, "pedra_grisa", "carreus", gir=0.0)
    e.cilindre(cx, cy, zc + 42, zc + 52, 8, "pedra_grisa", costats=12, dalt="pedra_grisa")
    tronc(e, cx, cy, zc + 52, zc + 58, 9, 3, 12, "pedra_grisa")
    punta_cristall(e, cx, cy, zc + 57, alt=16, r=3.5, base="or")
    anell(e, cx, cy, zc + 24, zc + 27, 58, 53, 28, "or")
    for k in range(10):                                                                # els llibres que suren
        ang = 2 * math.pi * k / 10 + 0.2
        bx, by = cx + 64 * math.cos(ang), cy + 64 * math.sin(ang)
        z = zc + 6 + 10 * math.sin(ang * 2)
        tapa = ("tela_verm", "pissarra_fosca", "porticons", "llauto")[k % 4]
        e.caixa(bx - 4, by - 3, z, bx + 4, by + 3, z + 1.5, tapa)
        e.caixa(bx - 3.5, by - 2.5, z + 1.5, bx + 3.5, by + 2.5, z + 4.5, "tela", dalt="tela")
        e.caixa(bx - 4, by - 3, z + 4.5, bx + 4, by + 3, z + 6, tapa)
    for (ax, ay) in ((x1 - 18, y0 + 20), (x1 - 18, y1 - 20), (x0 + 18, y1 - 20)):       # alambins de cristall
        e.cilindre(ax, ay, zb, zb + 6, 6, "llauto", costats=8, dalt="llauto")
        tronc(e, ax, ay, zb + 6, zb + 13, 2, 7, 8, "cristall")
        tronc(e, ax, ay, zb + 13, zb + 22, 7, 2, 8, "cristall")
        e.cilindre(ax, ay, zb + 22, zb + 28, 1.6, "llauto", costats=6)
    return e


def runa(e, x, y, llavor, n=6, mat="pedra_fosca"):
    """Un munt de pedres caigudes."""
    rnd = random.Random(llavor)
    for k in range(n):
        dx, dy = rnd.uniform(-12, 12), rnd.uniform(-9, 9)
        w, d, h = rnd.uniform(4, 9), rnd.uniform(4, 8), rnd.uniform(3, 7)
        z = rnd.choice((0, 0, 0, 3))
        e.caixa(x + dx, y + dy, z, x + dx + w, y + dy + d, z + h, mat, "carreus", dalt=mat)


def iglesia():
    """L'església vella (10x9), com la d'abans: pedra fosca, el campanar
    quadrat amb pinacles i la creu, el portal apuntat i la rosassa a la
    façana, contraforts i finestres gòtiques buides, i el sostre esfondrat
    (en queden les bigues). La racó del darrere, enrunat. Ara és dels
    Perduts: un cercle de ciris al mig de la nau i un anell de pedra trencat
    que encara sura (la màgia que es va espatllar)."""
    e = Escena(10, 9)
    W, D = e.W, e.D
    pc = porta_u(10)
    nx0, nx1, ny0, ny1, zn = pc - 80, pc + 80, 34, D - 30, 96
    terra(e, nx0 + 8, ny0 + 8, nx1 - 8, ny1 - 8, "pedra_fosca", "llambordes", desp=-1)
    g = 9
    e.caixa(nx0, ny1 - g, 0, nx1, ny1, zn, "pedra_fosca", "carreus")                          # façana
    e.cara([(nx0, ny1, zn), (nx1, ny1, zn), (pc, ny1, zn + 52)], (0, 1, 0),
           color_de("pedra_fosca", "carreus", "y"), e.nova_peca())
    dx = pc - nx0
    e.cara([(nx0, ny1 - g, zn), (pc, ny1 - g, zn + 52), (pc, ny1, zn + 52), (nx0, ny1, zn)], (-52, 0, dx),
           color_de("pedra_fosca", "llis", "z"), e.nova_peca())
    e.cara([(pc, ny1 - g, zn + 52), (nx1, ny1 - g, zn), (nx1, ny1, zn), (pc, ny1, zn + 52)], (52, 0, dx),
           color_de("pedra_fosca", "llis", "z"), e.nova_peca())
    e.caixa(nx1 - g, ny0, 0, nx1, ny1, zn, "pedra_fosca", "carreus")                          # mur de la dreta
    e.caixa(nx0, ny0, 0, nx1, ny0 + g, zn * 0.8, "pedra_fosca", "carreus")                   # capçalera (mig caiguda)
    e.caixa(nx0, ny0 + 70, 0, nx0 + g, ny1, zn, "pedra_fosca", "carreus")                    # mur de l'esquerra
    e.caixa(nx0, ny0, 0, nx0 + g, ny0 + 40, zn * 0.45, "pedra_fosca", "carreus")
    # el portal apuntat i la rosassa (octogonal: era dels Guardians)
    finestra_gotica(e, "y", ny1, 1, pc - 16, pc + 16, 0, 54, marc="pedra_grisa", buida=True)
    e.rect("y", ny1 + 0.3, 1, pc - 14, pc + 14, 0, 30, "noguera", "taulons_v", biaix=1.0)
    for (rr, mat, to) in ((22, "pedra_grisa", 1), (19, "ferro", 4)):
        e.poligon_cara("y", ny1, 1, [(pc + rr * math.cos(math.pi / 8 + k * math.pi / 4), zn + 4 + rr * math.sin(math.pi / 8 + k * math.pi / 4))
                                     for k in range(8)], mat, to=to, biaix=0.7 if to == 1 else 0.9)
    for k in range(8):
        a = k * math.pi / 4
        e.poligon_cara("y", ny1, 1, [(pc + 2 * math.cos(a + 1.57), zn + 4 + 2 * math.sin(a + 1.57)),
                                     (pc + 18 * math.cos(a) + 1.2 * math.cos(a + 1.57), zn + 4 + 18 * math.sin(a) + 1.2 * math.sin(a + 1.57)),
                                     (pc + 18 * math.cos(a) - 1.2 * math.cos(a + 1.57), zn + 4 + 18 * math.sin(a) - 1.2 * math.sin(a + 1.57)),
                                     (pc - 2 * math.cos(a + 1.57), zn + 4 - 2 * math.sin(a + 1.57))],
                       "pedra_grisa", to=2, biaix=1.1)
    for u in (pc - 50, pc + 50):
        finestra_gotica(e, "y", ny1, 1, u - 7, u + 7, 30, 70, marc="pedra_grisa", buida=True)
    for u in (nx0 + 2, nx1 - 2):
        contrafort(e, "y", ny1, 1, u, 104, mat="pedra_fosca", pinacle=True)
    for k, v in enumerate([ny0 + 30 + j * 44 for j in range(5)]):
        finestra_gotica(e, "x", nx1, 1, v - 7, v + 7, 30, 74, marc="pedra_grisa", buida=True)
        contrafort(e, "x", nx1, 1, v + 22, 100, mat="pedra_fosca", pinacle=(k % 2 == 0))
    # el sostre esfondrat: les bigues que queden i un tros de teulat
    for k, v in enumerate(range(int(ny0) + 20, int(ny1) - 10, 26)):
        if k in (2, 4):
            continue
        z = zn - 2 if k % 3 else zn - 10
        e.caixa(nx0 + 4, v, z, nx1 - 4, v + 4, z + 4, "noguera")
        e.caixa(nx0 + 6, v, z + 4, pc - 4, v + 3, z + 7, "noguera")
    for (v0, v1) in ((ny1 - 60, ny1 - 34), (ny0 + 30, ny0 + 52)):              # dos trossos de teulat que aguanten
        za, zb_ = zn + 52 * (1 - 40 / dx), zn + 2
        e.cara([(pc + 40, v0, za), (nx1 + 3, v0, zb_), (nx1 + 3, v1, zb_), (pc + 40, v1, za)], (52, 0, dx),
               color_de("encenall", "teules", "ty"), e.nova_peca())
    # el campanar, al davant a l'esquerra
    tx, ty, th = nx0 - 2, ny1 - 34, 30
    e.caixa(tx - th, ty - th, 0, tx + th, ty + th, 172, "pedra_fosca", "carreus")
    for z in (60, 108):
        finestra_gotica(e, "y", ty + th, 1, tx - 7, tx + 7, z, z + 30, marc="pedra_grisa", buida=True)
        finestra_gotica(e, "x", tx + th, 1, ty - 7, ty + 7, z, z + 30, marc="pedra_grisa", buida=True)
    e.caixa(tx - th - 3, ty - th - 3, 172, tx + th + 3, ty + th + 3, 178, "pedra_fosca", dalt="pedra_fosca")
    for (dx, dy) in ((th, th), (-th, th), (th, -th), (-th, -th)):
        tronc(e, tx + dx, ty + dy, 178, 186, 4.5, 4.5, 4, "pedra_fosca", gir=math.pi / 4)
        tronc(e, tx + dx, ty + dy, 186, 208, 4.5, 0, 4, "pedra_fosca", gir=math.pi / 4)
    tronc(e, tx, ty, 178, 232, (th - 6) * R2, 0, 4, "pissarra_fosca", "teules", gir=math.pi / 4)
    e.caixa(tx - 1, ty - 1, 232, tx + 1, ty + 1, 250, "ferro")
    e.caixa(tx - 6, ty - 1, 242, tx + 6, ty + 1, 245, "ferro")
    # les runes del racó del darrere
    for (x, y, ll) in ((40, 40, 1), (70, 60, 2), (48, 80, 3), (100, 26, 4)):
        runa(e, x, y, ll)
    # el cercle de ciris dels Perduts, al mig de la nau
    ccx, ccy = pc + 10, (ny0 + ny1) / 2 + 10
    e.cara([(x, y, 0.6) for x, y in poligon_regular(ccx, ccy, 30, 16)], (0, 0, 1), color_de("cera", "llis", "z", to_fix=3),
           e.nova_peca(), -0.2)
    for k, (x, y) in enumerate(poligon_regular(ccx, ccy, 26, 12)):
        h = 4 + (k * 7 % 5)
        e.cilindre(x, y, 0, h, 1.5, "cera", costats=6, dalt="cera")
        tronc(e, x, y, h + 0.5, h + 4, 1.0, 0, 4, "foc")
    # l'anell de pedra trencat que sura sobre la nau
    for k in range(10):
        if k in (2, 6, 7):
            continue
        a0, a1 = 2 * math.pi * k / 10 + 0.08, 2 * math.pi * (k + 1) / 10 - 0.08
        r0, r1 = 46, 38
        zz = zn + 40 + 5 * math.sin(k)
        pts = [(ccx + r0 * math.cos(a0), ccy + r0 * math.sin(a0)), (ccx + r0 * math.cos(a1), ccy + r0 * math.sin(a1)),
               (ccx + r1 * math.cos(a1), ccy + r1 * math.sin(a1)), (ccx + r1 * math.cos(a0), ccy + r1 * math.sin(a0))]
        pp = e.nova_peca()
        e.cara([(x, y, zz + 5) for x, y in pts], (0, 0, 1), color_de("pedra_fosca", "llis", "z"), pp)
        am = (a0 + a1) / 2
        e.cara([(pts[0][0], pts[0][1], zz), (pts[1][0], pts[1][1], zz), (pts[1][0], pts[1][1], zz + 5),
                (pts[0][0], pts[0][1], zz + 5)], (math.cos(am), math.sin(am), 0), color_de("pedra_fosca", "llis", "y"), pp)
        if k % 3 == 0:
            tronc(e, ccx + (r0 + r1) / 2 * math.cos(am), ccy + (r0 + r1) / 2 * math.sin(am), zz + 5, zz + 13, 2.5, 0, 4,
                  "cristall")
    return e


def _el·lipse(cx, cy, a, b, n, fase=0.0):
    return [(cx + a * math.cos(fase + 2 * math.pi * k / n), cy + b * math.sin(fase + 2 * math.pi * k / n)) for k in range(n)]


def coliseo():
    """El coliseu (8x6), com el d'abans: l'amfiteatre oval de pedra clara amb
    tres pisos d'arcs, les grades per dins i la sorra al mig, la part del
    darrere mig caiguda. La màgia: un anell d'or que sura sobre la sorra, pals
    amb gallarets al voltant i braseres de cristall a la porta."""
    e = Escena(8, 6)
    W, D = e.W, e.D
    pc = porta_u(8)
    cx, cy, A, B, n = W / 2, D / 2, W / 2 - 6, D / 2 - 6, 32
    H = 92
    alt = lambda ang: H - (34 if math.cos(ang - 3.6) > 0.55 else 0)          # el tros caigut, al darrere-esquerra
    ext = _el·lipse(cx, cy, A, B, n)
    inn = _el·lipse(cx, cy, A - 20, B - 20, n)
    p = e.nova_peca()
    for k in range(n):
        a0, a1 = 2 * math.pi * k / n, 2 * math.pi * (k + 1) / n
        am = (a0 + a1) / 2
        (xa, ya), (xb, yb) = ext[k], ext[(k + 1) % n]
        (xc, yc), (xd, yd) = inn[(k + 1) % n], inn[k]
        h = alt(am)
        nx, ny = math.cos(am) / A, math.sin(am) / B
        e.cara([(xa, ya, 0), (xb, yb, 0), (xb, yb, h), (xa, ya, h)], (nx, ny, 0), color_de("pedra_grisa", "carreus", "y"), p)
        e.cara([(xa, ya, h), (xb, yb, h), (xc, yc, h), (xd, yd, h)], (0, 0, 1), color_de("pedra_grisa", "llis", "z"), p)
        e.cara([(xd, yd, 30), (xc, yc, 30), (xc, yc, h), (xd, yd, h)], (-nx, -ny, 0), color_de("pedra_grisa", "carreus", "y"), p)
        if nx * A + ny * B > 0.15:                                             # arcs a la cara que es veu
            for (z0, z1) in ((8, 30), (38, 58), (66, 84)):
                if z1 > h - 4:
                    continue
                f0, f1 = 0.22, 0.78
                pa = (xa + (xb - xa) * f0, ya + (yb - ya) * f0)
                pb = (xa + (xb - xa) * f1, ya + (yb - ya) * f1)
                pm = ((pa[0] + pb[0]) / 2, (pa[1] + pb[1]) / 2)
                e.cara([(pa[0], pa[1], z0), (pb[0], pb[1], z0), (pb[0], pb[1], z1 - 5), (pm[0], pm[1], z1),
                        (pa[0], pa[1], z1 - 5)], (nx, ny, 0), color_de("ferro", "llis", "y", to_fix=4), e.nova_peca(), 0.7)
            e.cara([(xa, ya, 32), (xb, yb, 32), (xb, yb, 35), (xa, ya, 35)], (nx, ny, 0),
                   color_de("pedra_grisa", "llis", "y", to_fix=1), e.nova_peca(), 0.5)
    # les grades, la paret de l'arena i la sorra
    for j, (da, z) in enumerate(((20, 30), (32, 22), (44, 14))):
        anell_pts_e = _el·lipse(cx, cy, A - da, B - da, n)
        anell_pts_i = _el·lipse(cx, cy, A - da - 12, B - da - 12, n)
        pg = e.nova_peca()
        for k in range(n):
            (xa, ya), (xb, yb) = anell_pts_e[k], anell_pts_e[(k + 1) % n]
            (xc, yc), (xd, yd) = anell_pts_i[(k + 1) % n], anell_pts_i[k]
            am = 2 * math.pi * (k + 0.5) / n
            e.cara([(xa, ya, z), (xb, yb, z), (xc, yc, z), (xd, yd, z)], (0, 0, 1), color_de("pedra_grisa", "llis", "z", desp=j % 2), pg)
            e.cara([(xd, yd, z - 8), (xc, yc, z - 8), (xc, yc, z), (xd, yd, z)], (-math.cos(am), -math.sin(am), 0),
                   color_de("pedra_grisa", "carreus", "y"), pg)
    e.cara([(x, y, 6) for x, y in _el·lipse(cx, cy, A - 56, B - 56, n)], (0, 0, 1), color_de("sorra", "llis", "z"), e.nova_peca())
    # la porta: un arc gran a la cara de davant, i les braseres
    portal(e, "y", D - 6 + 0.1, 1, pc, ample=24, alt=48, mat_fulla="noguera", marc="pedra_grisa")
    for u in (pc - 26, pc + 26):
        e.caixa(u - 3, D - 8, 0, u + 3, D - 2, 18, "pedra_grisa", dalt="pedra_grisa")
        tronc(e, u, D - 5, 18, 22, 4.5, 4.5, 8, "or")
        tronc(e, u, D - 5, 22, 32, 0.1, 4, 4, "cristall")
    # els pals amb gallarets al voltant i l'anell d'or que sura
    for k, (x, y) in enumerate(_el·lipse(cx, cy, A - 9, B - 9, 6, fase=math.pi / 6)):
        h = alt(math.atan2((y - cy) / B, (x - cx) / A))
        e.caixa(x - 1, y - 1, h, x + 1, y + 1, h + 34, "noguera")
        e.cara([(x + 1, y, h + 33), (x + 15, y, h + 29), (x + 1, y, h + 25)], (0, 1, 0),
               color_de(("tela_verm", "llauto")[k % 2], "llis", "y"), e.nova_peca(), 0.3, True)
    anell(e, cx, cy, H + 46, H + 49, A * 0.62, A * 0.62 - 4, 28, "or")
    anell(e, cx, cy, H + 60, H + 62, A * 0.42, A * 0.42 - 3, 24, "or")
    tronc(e, cx, cy, H + 70, H + 84, 0.1, 6, 4, "cristall")
    tronc(e, cx, cy, H + 84, H + 100, 6, 0.1, 4, "cristall")
    return e


def cuartel():
    """El quarter (11x8), com el d'abans: un fortí de pedra grisa amb merlets,
    la torre de guaita amb el teulat de pissarra i la bandera, i la porta
    d'arc amb les senyeres; dins, el barracó i el pati amb armes i bótes.
    La màgia: les veles de senyals al pal de la torre (els colors parlen de
    lluny) i un anell d'or que sura al voltant del teulat de la torre."""
    e = Escena(11, 8)
    W, D = e.W, e.D
    pc = porta_u(11)
    x0, y0, x1, y1, mz, g = 20, 20, W - 20, D - 22, 64, 14
    e.cara([(x0, y0, 1), (x1, y0, 1), (x1, y1, 1), (x0, y1, 1)], (0, 0, 1), color_de("sorra", "llis", "z", desp=1),
           e.nova_peca(), -0.5)
    mur(e, x0, y0, x1, y1, 0, mz, g, "pedra_castell")
    merlets(e, x0, y0, x1, y1, mz, "pedra_castell", alt=12, g=7, pas=14)
    for u in range(int(x0 + 30), int(x1 - 20), 30):
        if abs(u - pc) > 40:
            e.rect("y", y1, 1, u - 1.5, u + 1.5, 30, 44, "ferro", to=4, biaix=0.6)
    for u in range(int(y0 + 30), int(y1 - 20), 30):
        e.rect("x", x1, 1, u - 1.5, u + 1.5, 30, 44, "ferro", to=4, biaix=0.6)
    # el barracó del fons
    e.caixa(x0 + g, y0 + g, 0, x1 - 96, y0 + 86, 54, "pedra_castell", "carreus")
    e.teulada(x0 + g, y0 + g, x1 - 96, y0 + 86, 54, 28, mat="pissarra_fosca", tex="teules", voladis=3, gruix=2,
              mat_timpa="pedra_castell", tex_timpa="carreus", eix="x")
    for u in range(int(x0 + 40), int(x1 - 110), 28):
        finestra(e, "y", y0 + 86, 1, u - 4, u + 4, 20, 34, estil="simple", jardinera=False)
    finestra(e, "y", y0 + 86, 1, x1 - 140, x1 - 120, 0, 36, estil="forat", jardinera=False, mat_p="noguera")
    # el pati: armes, bótes i un ninot d'entrenament
    armes(e, x0 + 70, y1 - 40)
    armes(e, x0 + 120, y1 - 40)
    for (bx, by) in ((x1 - 40, y1 - 30), (x1 - 52, y1 - 26)):
        bota(e, bx, by)
    e.caixa(pc + 50, y1 - 60, 0, pc + 52, y1 - 58, 30, "noguera")
    e.caixa(pc + 46, y1 - 61, 18, pc + 56, y1 - 57, 30, "encenall", dalt="encenall")
    e.caixa(pc + 48.5, y1 - 60.5, 30, pc + 53.5, y1 - 57.5, 35, "tela", dalt="tela")
    # la torre de guaita
    tx, ty, th = x1 - 40, y0 + 44, 26
    e.caixa(tx - th, ty - th, 0, tx + th, ty + th, 130, "pedra_castell", "carreus")
    for z in (48, 90):
        finestra(e, "y", ty + th, 1, tx - 3, tx + 3, z, z + 12, estil="forat", jardinera=False, mat_p="pedra_castell")
        finestra(e, "x", tx + th, 1, ty - 3, ty + 3, z, z + 12, estil="forat", jardinera=False, mat_p="pedra_castell")
    e.caixa(tx - th - 3, ty - th - 3, 130, tx + th + 3, ty + th + 3, 135, "pedra_castell", dalt="pedra_castell")
    merlets(e, tx - th - 3, ty - th - 3, tx + th + 3, ty + th + 3, 135, "pedra_castell", alt=9, g=5, pas=10)
    tronc(e, tx, ty, 140, 196, (th - 3) * R2, 0, 4, "pissarra_fosca", "teules", gir=math.pi / 4)
    anell(e, tx, ty, 156, 159, 42, 38, 24, "or")
    e.caixa(tx - 1, ty - 1, 196, tx + 1, ty + 1, 238, "noguera")
    for k, mat in enumerate(("tela_verm", "llauto", "roba2", "tela")):            # les veles de senyals
        z = 232 - k * 9
        e.cara([(tx + 1, ty, z), (tx + 16 - k, ty, z - 3), (tx + 1, ty, z - 7)], (0, 1, 0), color_de(mat, "llis", "y"),
               e.nova_peca(), 0.3, True)
    punta_cristall(e, tx, ty, 236, alt=10, r=2.5, base="or")
    # la porta: arc, reixa a mitja alçada, senyeres i dues torretes
    for dx in (-30, 30):
        e.caixa(pc + dx - 12, y1 - 12, 0, pc + dx + 12, y1 + 4, 82, "pedra_castell", "carreus")
        e.caixa(pc + dx - 14, y1 - 14, 82, pc + dx + 14, y1 + 6, 86, "pedra_castell", dalt="pedra_castell")
        merlets(e, pc + dx - 14, y1 - 14, pc + dx + 14, y1 + 6, 86, "pedra_castell", alt=8, g=5, pas=9)
    portal(e, "y", y1, 1, pc, ample=26, alt=48, mat_fulla="noguera", marc="pedra_castell")
    for du in (-30, 30):
        e.rect("y", y1 + 4, 1, pc + du - 6, pc + du + 6, 36, 72, "tela_verm", biaix=0.7)
        e.poligon_cara("y", y1 + 4, 1, [(pc + du - 6, 36), (pc + du, 30), (pc + du + 6, 36)], "tela_verm", biaix=0.7)
        e.rect("y", y1 + 4, 1, pc + du - 2, pc + du + 2, 48, 60, "or", to=1, biaix=0.9)
    return e


MONUMENTS = {
    "castillo": ((13, 10, "viejo_castillo"), castillo),
    "opera": ((10, 9, "viejo_opera"), opera),
    "universidad": ((10, 9, "viejo_universidad"), universidad),
    "biblioteca": ((12, 9, "viejo_biblioteca"), biblioteca),
    "iglesia": ((10, 9, "viejo_iglesia"), iglesia),
    "coliseo": ((8, 6, "viejo_coliseo"), coliseo),
    "cuartel": ((11, 8, "viejo_militar"), cuartel),
}
