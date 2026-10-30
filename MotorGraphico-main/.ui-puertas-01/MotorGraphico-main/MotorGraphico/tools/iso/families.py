#!/usr/bin/env python3
"""ISO — Les famílies d'habitatge que no són una casa de dues aigües:
barraques de la Barriada (taulons apedaçats, teulada d'un vessant de llauna o
de tela) i tendes del vado de Surysal (lona a ratlles: de parets, pavelló rodó o
amb tendal). La resta de famílies (popular, noble, cabana, caserna) són estils
de edificis.casa.

    from families import barraca, tenda
"""
import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from monuments import poligon_regular  # noqa: E402
from peces_iso import Escena, color_de, mata  # noqa: E402
import patis  # noqa: E402


def barraca(ample, fons, llavor=0, pati=10):
    """Barraca: taulons de colors diferents, pedaços, teulada d'un vessant, cortina de
    tela per porta, un forat per finestra, caixes, una bóta trencada i roba estesa."""
    rnd = random.Random(llavor)
    e = Escena(ample, fons)
    W, D = ample * 32, fons * 32
    bx0, by0, bx1, by1 = 6, 6, W - 6, D - pati
    h = 34 + rnd.randint(0, 6)
    mur = rnd.choice(["fusta", "encenall", "noguera"])
    e.caixa(bx0, by0, 0, bx1, by1, h, mur, "taulons")
    for (eix, c, s, u0, u1) in (("y", by1, 1, bx0, bx1), ("x", bx1, 1, by0, by1), ("y", by0, -1, bx0, bx1), ("x", bx0, -1, by0, by1)):
        for _ in range(2):                                      # pedaços
            u = rnd.uniform(u0 + 2, u1 - 12)
            v = rnd.uniform(4, h - 12)
            e.rect(eix, c, s, u, u + rnd.choice([6, 8, 10]), v, v + rnd.choice([6, 8]),
                   rnd.choice(["fusta", "noguera", "llauna", "tela"]), "taulons_v" if rnd.random() < 0.5 else "llis")
    sostre = rnd.choice([("llauna", "ondulat"), ("tela", "ratlles"), ("tela_verm", "ratlles"), ("encenall", "teules")])
    e.un_vessant(bx0, by0, bx1, by1, h, h + (by1 - by0 + 6) / 2, mat=sostre[0], tex=sostre[1], mat_paret=mur)
    pc = (bx0 + bx1) / 2 + rnd.choice([-8, 0, 8])
    e.rect("y", by1, 1, pc - 7, pc + 7, 0, 30, "noguera", to=4)
    e.rect("y", by1, 1, pc - 6, pc + 6, 0, 29, rnd.choice(["tela_verm", "roba2", "tela"]), "taulons_v", biaix=0.8)
    fx = bx1 - 10 if pc < (bx0 + bx1) / 2 else bx0 + 4
    e.rect("y", by1, 1, fx, fx + 7, 16, 23, "ferro", to=4)
    e.rect("x", bx1, 1, (by0 + by1) / 2 - 4, (by0 + by1) / 2 + 4, 15, 22, "ferro", to=4)
    yp = (by1 + D) / 2
    xs = [x for x in range(int(bx0) + 6, int(bx1) - 2, 11) if abs(x - pc) > 12]
    rnd.shuffle(xs)
    for tipus in ("caixes", "bota", "mata"):
        if not xs:
            break
        x = xs.pop()
        if tipus == "caixes":
            patis.caixes(e, x, yp)
        elif tipus == "bota":
            e.cilindre(x, yp, 0, 8, 4.5, "fusta", "cercles", dalt="ferro")
        else:
            e.sprite((x, yp, 0), mata(4, 4, flors=("fulla",), llavor=llavor + 9))
    if rnd.random() < 0.7:                                      # roba estesa a la cara +X
        e.rect("x", bx1 + 1, 1, by0 + 2, by1 - 2, h - 4, h - 3, "ferro", biaix=1.4)
        for j, u in enumerate(range(int(by0) + 4, int(by1) - 6, 7)):
            e.rect("x", bx1 + 1, 1, u, u + 5, h - 11 + (j % 2) * 2, h - 4, rnd.choice(["roba1", "roba2", "roba3", "tela_verm"]), biaix=1.5)
    return e


LONES = [("tela_verm", "tela"), ("roba2", "tela"), ("porticons", "tela"), ("roba3", "tela_verm"), ("tela", "roba2")]


def serrell(e, P0, P1, z, n, mat, alt=6, pas=6, peca=None):
    """Serrell de lona amb dents penjant d'una vora horitzontal de P0 a P1 ((x, y)) a l'alçada z."""
    (xa, ya), (xb, yb) = P0, P1
    nd = max(1, round(math.hypot(xb - xa, yb - ya) / pas))
    pts = [(xa, ya, z), (xb, yb, z)]
    for i in range(nd, 0, -1):
        for t, dz in ((i / nd, alt * 0.45), ((i - 0.5) / nd, alt)):
            pts.append((xa + (xb - xa) * t, ya + (yb - ya) * t, z - dz))
    pts.append((xa, ya, z - alt * 0.45))
    e.cara(pts, n, color_de(mat, "llis", "y"), peca or e.nova_peca(), 0.6)


def pal_bandera(e, x, y, z0, z1, mat):
    """Pal amb una bandereta triangular i el pom de llautó."""
    e.caixa(x - 1, y - 1, z0, x + 1, y + 1, z1, "noguera")
    e.caixa(x - 1.5, y - 1.5, z1, x + 1.5, y + 1.5, z1 + 3, "llauto")
    e.cara([(x + 1, y, z1 - 1), (x + 13, y, z1 - 4), (x + 1, y, z1 - 8)], (0, 1, 0), color_de(mat, "llis", "y"),
           e.nova_peca(), 0.6, True)


def estora(e, x0, y0, x1, y1, mat):
    e.cara([(x0, y0, 0.2), (x1, y0, 0.2), (x1, y1, 0.2), (x0, y1, 0.2)], (0, 0, 1), color_de(mat, "ratlles", "z"), e.nova_peca())


def obertura(e, xm, y, alt, ample, lona):
    """Entrada de tenda a la cara +Y: el forat fosc i les dues solapes lligades."""
    e.poligon_cara("y", y, 1, [(xm - ample, 0), (xm + ample, 0), (xm + 3, alt), (xm - 3, alt)], "ferro", to=4, biaix=0.8)
    for s in (-1, 1):
        e.poligon_cara("y", y, 1, [(xm + s * ample, 0), (xm + s * (ample + 6), 0), (xm + s * 3, alt)], lona, to=2, biaix=1.0)


def tenda(ample, fons, llavor=0, model="paret"):
    """Tenda dels qui passen pel vado, més alta que un personatge (la porta d'una casa fa 44):
    'paret' (parets de lona i dues aigües amb el frontó i l'entrada a +Y), 'pavello' (pavelló
    rodó a gallons amb serrell i bandereta) o 'tendal' (entrada al costat llarg amb un tendal
    sobre dos pals). Estora, cistells i un fanalet de cristall."""
    rnd = random.Random(llavor)
    e = Escena(ample, fons)
    W, D = ample * 32, fons * 32
    lona, ratlla = rnd.choice(LONES)
    xm = W / 2
    if model == "pavello":
        cy, rp, rv, zp, zc = D / 2 - 5, 21, 26, 28, 70
        gir = math.pi / 12                                      # una cara mira just a +Y
        parets, pp = poligon_regular(xm, cy, rp, 12, gir), e.nova_peca()
        for k in range(12):
            (xa, ya), (xb, yb) = parets[k], parets[(k + 1) % 12]
            n = ((xa + xb) / 2 - xm, (ya + yb) / 2 - cy, 0)
            e.cara([(xa, ya, 0), (xb, yb, 0), (xb, yb, zp), (xa, ya, zp)], n, color_de((lona, ratlla)[k % 2], "llis", "y"), pp)
        p = e.nova_peca()
        vores = poligon_regular(xm, cy, rv, 12, gir)
        for k in range(12):
            (xa, ya), (xb, yb) = vores[k], vores[(k + 1) % 12]
            mx, my = (xa + xb) / 2 - xm, (ya + yb) / 2 - cy
            l = math.hypot(mx, my)
            e.cara([(xa, ya, zp - 2), (xb, yb, zp - 2), (xm, cy, zc)], (mx / l * (zc - zp + 2), my / l * (zc - zp + 2), rv * math.cos(math.pi / 12)),
                   color_de((lona, ratlla)[k % 2], "llis", "y"), p)
            serrell(e, (xa, ya), (xb, yb), zp - 2, (mx, my, 0), ratlla, alt=7, pas=5)
        yp = cy + rp * math.cos(math.pi / 12) + 0.3
        e.rect("y", yp, 1, xm - 5, xm + 5, 0, 24, "ferro", to=4, biaix=0.8)
        e.rect("y", yp, 1, xm - 5, xm + 5, 21, 24, ratlla, to=2, biaix=1.0)
        pal_bandera(e, xm, cy, zc - 6, zc + 10, ratlla)
        estora(e, xm - 9, yp + 1, xm + 9, D - 2, rnd.choice(["tela_verm", "roba3"]))
        y_davant = D - 8
    elif model == "tendal":
        x0, y0, x1, y1, zp = 8, 6, W - 8, D - 24, 24
        e.caixa(x0, y0, 0, x1, y1, zp, lona, "ratlles", dalt=lona)
        e.teulada(x0, y0, x1, y1, zp, 24, mat=lona, tex="ratlles", voladis=3, gruix=2, mat_timpa=lona, tex_timpa="ratlles", eix="x")
        serrell(e, (x0 - 3, y1 + 3), (x1 + 3, y1 + 3), zp, (0, 1, 0), ratlla)
        serrell(e, (x1 + 3, y0 - 3), (x1 + 3, y1 + 3), zp, (1, 0, 0), ratlla)
        e.rect("y", y1, 1, xm - 7, xm + 7, 0, 20, "ferro", to=4, biaix=0.8)
        ta, tb, zt = y1 + 3, y1 + 17, zp - 2                    # tendal sobre dos pals
        e.cara([(xm - 15, ta, zt + 4), (xm + 15, ta, zt + 4), (xm + 15, tb, zt - 2), (xm - 15, tb, zt - 2)], (0, 6, 14),
               color_de(ratlla, "ratlles", "y"), e.nova_peca(), 0.6)
        serrell(e, (xm - 15, tb), (xm + 15, tb), zt - 2, (0, 1, 0), lona, alt=5)
        for x in (xm - 14, xm + 13):
            e.caixa(x, tb - 2, 0, x + 1.5, tb - 0.5, zt - 1, "noguera")
        estora(e, xm - 12, y1 + 1, xm + 12, tb - 2, rnd.choice(["tela_verm", "roba3", "roba2"]))
        for x in (x0 + 4, x1 - 10):                              # mercaderia a banda i banda
            patis.caixes(e, x + 3, y1 + 8)
        y_davant = D - 4
    else:
        x0, y0, x1, y1, zp = 12, 6, W - 12, D - 16, 24
        e.caixa(x0, y0, 0, x1, y1, zp, lona, "ratlles", dalt=lona)
        e.teulada(x0, y0, x1, y1, zp, 30, mat=lona, tex="ratlles", voladis=3, gruix=2, mat_timpa=lona, tex_timpa="ratlles", eix="y")
        for x, s in ((x1 + 3, 1), (x0 - 3, -1)):
            serrell(e, (x, y0 - 3), (x, y1 + 3), zp, (s, 0, 0), ratlla)
        obertura(e, xm, y1, zp + 14, 6, ratlla)
        for yy in (y0 - 3, y1 + 3):
            pal_bandera(e, xm, yy, zp + 24, zp + 40, ratlla)
        estora(e, xm - 9, y1 + 1, xm + 9, D - 2, rnd.choice(["tela_verm", "roba2", "roba3"]))
        y_davant = D - 6
    for x in ((4, W - 12) if model != "tendal" else ()):
        e.cilindre(x + 4, y_davant, 0, 6, 3, "encenall", "taulons", dalt="fusta")
        e.sprite((x + 4, y_davant, 6), mata(3, 2, flors=("flor2", "teula"), llavor=llavor + x))
    xf = W - 5 if model != "pavello" else W - 9                  # fanalet tecnomàgic
    e.caixa(xf - 1, D - 18, 0, xf + 1, D - 16, 22, "ferro")
    e.caixa(xf - 2, D - 19, 14, xf + 2, D - 15, 21, "cristall", dalt="llauto")
    return e
