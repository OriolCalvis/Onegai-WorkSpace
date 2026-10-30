#!/usr/bin/env python3
"""ISO — Els oficis a la vista (Stardew): el que diu de què viu cada local,
dins de la mateixa peça i girant amb ella. Cada ofici té un estil d'edifici
(ESTIL_OFICI) i una funció que rep l'escena i el context de la casa (cos,
porta, pati) i hi afegeix porxo, aparador, forja, terrassa, parades, rètol...
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from peces_iso import color_de, fum, mata  # noqa: E402

ICONES = {   # rètols: 10x8, '#' = tinta
    "gerra": ["..######..", ".#......#.", ".#......##", ".#......#.", ".#......#.", ".#......#.", "..######..", ".........."],
    "enclusa": [".........", "########.", ".#######.", "...###...", "...###...", "..#####..", "#########", "........."],
    "poma": ["....#....", "...#.....", "..####...", ".######..", ".######..", ".######..", "..####...", "........."],
    "anell": ["....##....", "...####...", "..#....#..", ".#......#.", ".#......#.", "..#....#..", "...####...", ".........."],
    "monedes": ["..######..", ".#......#.", "..######..", ".#......#.", "..######..", ".#......#.", "..######..", ".........."],
    "agulla": [".......#..", "......#.#.", ".....#.#..", "....#.....", "...#......", "..#.......", ".#........", "#........."],
    "clau": ["..##......", ".#..#.....", ".#..######", ".#..#.#.#.", "..##......", "..........", "..........", ".........."],
    "gota": ["....#.....", "...###....", "..#####...", ".#######..", ".#######..", ".#######..", "..#####...", ".........."],
    "tassa": ["..........", "#######...", "#.....###.", "#.....#.#.", "#.....###.", ".#####....", "#######...", ".........."],
    "espasa": [".........#", "........#.", ".......#..", "......#...", "..#..#....", "...##.....", "..#.#.....", ".#........"],
}

ESTIL_OFICI = {
    "taverna": dict(estil="popular"),
    "ferreria": dict(estil="popular", mur="socol", teula="pissarra", porticons="roba2"),
    "botiga": dict(estil="popular", plantes=1),
    "mercat": dict(estil="popular", alta="estuc_ocre"),
    "joieria": dict(estil="noble"),
    "banc": dict(estil="noble", teula="verdet"),
    "sastreria": dict(estil="popular", alta="estuc_rosa", porticons="roba2"),
    "posada": dict(estil="popular", plantes=3, alta="estuc_ocre", alt_pa=26),
    "banys": dict(estil="noble", plantes=1, teula="verdet", balcons=False, cupula=True),
    "casa_te": dict(estil="noble", plantes=1, teula="verdet", porta="arc", balcons=False),
    "armeria": dict(estil="caserna", plantes=2, lucanes=True),
}


def rotol(e, x, y, z, icona, ample=12, alt=10, taula="fusta"):
    """Rètol penjat perpendicular a la façana +Y (pla X = x), amb la icona de l'ofici."""
    e.caixa(x - 1, y, z + alt + 1, x + 1, y + ample + 2, z + alt + 3, "ferro")
    e.cara([(x, y + 2, z), (x, y + ample + 2, z), (x, y + ample + 2, z + alt), (x, y + 2, z + alt)], (1, 0, 0),
           color_de(taula, "taulons" if taula == "fusta" else "llis", "x"), e.nova_peca(), 0.0, True)
    p = e.nova_peca()
    files = ICONES[icona]
    u0 = y + 2 + (ample - len(files[0])) / 2
    v1 = z + alt - 1
    for j, fila in enumerate(files):
        for i, ch in enumerate(fila):
            if ch == "#":
                e.cara([(x, u0 + i, v1 - j - 1), (x, u0 + i + 1, v1 - j - 1), (x, u0 + i + 1, v1 - j), (x, u0 + i, v1 - j)],
                       (1, 0, 0), color_de("noguera", "llis", "x", to_fix=4), p, 0.7, True)


def rotol_c(e, c, icona, taula="fusta"):
    x = c["pc"] + 19 if c["pc"] + 19 < c["bx1"] - 2 else c["pc"] - 20
    rotol(e, x, c["by1"], c["z_pb"] + 30, icona, 13, 11, taula)


def bota(e, x, y, z=0, alt=13):
    e.cilindre(x, y, z, z + alt, 4.5, "fusta", "cercles", dalt="noguera")


def pila_botes(e, x, y):
    bota(e, x, y)
    bota(e, x + 9, y)
    bota(e, x + 4.5, y - 7)
    bota(e, x + 4.5, y - 3.5, 13, 12)


def terrassa(e, x, y, para_sol=False):
    e.cilindre(x, y, 0, 10, 1.5, "noguera")
    e.cilindre(x, y, 10, 12, 6.5, "fusta", dalt="fusta")
    for dx, dy in ((-9, 0), (9, 0), (0, -8)):
        e.cilindre(x + dx, y + dy, 0, 6, 2.5, "noguera", dalt="fusta")
    e.cilindre(x - 2, y - 1, 12, 15, 1.3, "roba1")
    e.cilindre(x + 2.5, y + 1, 12, 15, 1.3, "roba1")
    if para_sol:
        from monuments import tronc
        e.cilindre(x, y, 12, 30, 0.8, "noguera")
        tronc(e, x, y, 28, 35, 11, 0, 8, "tela_verm", "ratlles")


def porxo(e, pc, by1, z_pb, ample=30, fons=11, alt=40, mat="teula"):
    """Porxo damunt la porta: dos pals i una teuladeta."""
    x0, x1 = pc - ample / 2, pc + ample / 2
    for x in (x0, x1 - 3):
        e.caixa(x, by1 + fons - 3, 0, x + 3, by1 + fons, z_pb + alt, "noguera")
    p = e.nova_peca()
    z = z_pb + alt
    e.cara([(x0 - 2, by1, z + 8), (x1 + 2, by1, z + 8), (x1 + 2, by1 + fons + 2, z), (x0 - 2, by1 + fons + 2, z)],
           (0, fons + 2, 8), color_de(mat, "teules", "tx"), p)
    e.cara([(x0 - 2, by1 + fons + 2, z - 3), (x1 + 2, by1 + fons + 2, z - 3), (x1 + 2, by1 + fons + 2, z),
            (x0 - 2, by1 + fons + 2, z)], (0, 1, 0), color_de(mat, "llis", "y", desp=1), p)
    for xx, s in ((x1 + 2, 1), (x0 - 2, -1)):
        e.cara([(xx, by1, z + 5), (xx, by1, z + 8), (xx, by1 + fons + 2, z), (xx, by1 + fons + 2, z - 3)],
               (s, 0, 0), color_de(mat, "llis", "x", desp=1), p)


def tendal(e, u0, u1, y, z, ratlla=("cortina", "roba1"), sortint=9, caiguda=7):
    p = e.nova_peca()
    for k, u in enumerate(range(int(u0), int(u1), 4)):
        mat = ratlla[k % 2]
        e.cara([(u, y, z), (u + 4, y, z), (u + 4, y + sortint, z - caiguda), (u, y + sortint, z - caiguda)],
               (0, caiguda, sortint), color_de(mat, "llis", "y"), p)
        e.cara([(u, y + sortint, z - caiguda - 3), (u + 4, y + sortint, z - caiguda - 3), (u + 4, y + sortint, z - caiguda),
                (u, y + sortint, z - caiguda)], (0, 1, 0), color_de(mat, "llis", "y", desp=1), p)


def zona_aparador(c):
    a0, a1 = c["aparador"]
    if a1 - a0 < 20:
        a0, a1 = c["pc"] + 24, c["bx1"] - 8
    return a0, a1


def aparador(e, c, amb_tendal=True, ratlla=("cortina", "roba1"), dins=None):
    """Aparador a la façana; dins = llista de (material, ample) que es veuen darrere el vidre."""
    a0, a1 = zona_aparador(c)
    by1, v0, v1 = c["by1"], c["vb0"] - 2, c["vb1"] + 2
    e.rect("y", by1, 1, a0 - 2, a1 + 2, v0 - 2, v1 + 2, "noguera", to=4)
    e.rect("y", by1, 1, a0, a1, v0, v1, "vidre", biaix=0.8)
    if dins:
        u, k = a0 + 2, 0
        while u < a1 - 3:
            mat, w = dins[k % len(dins)]
            e.rect("y", by1, 1, u, min(u + w, a1 - 2), v0 + 2, v0 + 2 + (v1 - v0) * 0.6, mat, biaix=0.9)
            u += w + 1
            k += 1
    for u in range(int(a0) + 8, int(a1), 9):
        e.rect("y", by1, 1, u, u + 2, v0, v1, "noguera", biaix=1.0)
    if amb_tendal:
        tendal(e, a0 - 2, a1 + 2, by1, v1 + 10, ratlla)
    return a0, a1


def parada(e, x, y, llavor, ratlla=("tela_verm", "tela")):
    """Parada de mercat: taula, quatre pals, tendal a ratlles i fruita."""
    e.caixa(x - 10, y - 5, 0, x + 10, y + 5, 9, "fusta", "taulons")
    for xx, yy in ((x - 10, y - 5), (x + 8, y - 5), (x - 10, y + 3), (x + 8, y + 3)):
        e.caixa(xx, yy, 0, xx + 2, yy + 2, 24, "noguera")
    p = e.nova_peca()
    for k, u in enumerate(range(int(x) - 12, int(x) + 12, 4)):
        e.cara([(u, y - 7, 27), (u + 4, y - 7, 27), (u + 4, y + 7, 22), (u, y + 7, 22)], (0, 5, 14),
               color_de(ratlla[k % 2], "llis", "tx"), p)
    for k, (dx, fl) in enumerate(((-6, ("flor2",)), (0, ("teula",)), (6, ("tela_verm", "fulla")))):
        e.sprite((x + dx, y, 9), mata(3, 2, flors=fl, llavor=llavor + k))


def finestres_facana(e, c, est="altes", evita_aparador=False):
    from edificis import centres, finestra
    ev = [c["pc"]]
    if evita_aparador:
        a0, a1 = zona_aparador(c)
        ev += list(range(int(a0), int(a1) + 1, 6))
    for u in centres(c["bx0"], c["bx1"], evita=ev):
        finestra(e, "y", c["by1"], 1, u - 6, u + 6, c["vb0"], c["vb1"], est, jardinera=True, llavor=c["llavor"] + int(u))


# ---------------- els oficis ----------------
def taverna(e, c):
    porxo(e, c["pc"], c["by1"], c["z_pb"])
    a0, _ = aparador(e, c, amb_tendal=False)
    rotol_c(e, c, "gerra")
    terrassa(e, a0 + 10, c["yp"] + 1)
    pila_botes(e, c["bx1"] - 16, c["yp"] + 3)
    e.sprite((c["bx0"] + 4, c["yp"], 0), mata(5, 5, llavor=c["llavor"] + 3))


def ferreria(e, c):
    """Forja oberta (el foc es veu des del carrer), enclusa, abeurador, eines i barres."""
    pc, by1, z_pb = c["pc"], c["by1"], c["z_pb"]
    e.rect("y", by1, 1, pc - 16, pc + 16, z_pb, z_pb + 40, "noguera", to=4, arc=True)
    e.rect("y", by1, 1, pc - 14, pc + 14, z_pb, z_pb + 38, "ferro", to=4, arc=True, biaix=0.8)
    e.rect("y", by1, 1, pc - 10, pc + 10, z_pb, z_pb + 10, "foc", to=2, biaix=1.0)
    e.rect("y", by1, 1, pc - 7, pc + 7, z_pb + 2, z_pb + 8, "foc", to=0, biaix=1.2)
    e.rect("y", by1, 1, pc - 12, pc + 12, z_pb + 10, z_pb + 14, "foc", to=3, biaix=1.1)
    rotol(e, pc + 22, by1, z_pb + 30, "enclusa", 13, 11)
    yp = c["yp"]
    x = pc - 4
    e.cilindre(x, yp, 0, 8, 4, "noguera", dalt="fusta")
    e.caixa(x - 5, yp - 2, 8, x + 5, yp + 2, 11, "ferro")
    e.caixa(x - 3, yp - 1.5, 11, x + 3, yp + 1.5, 13, "ferro")
    e.caixa(x + 5, yp - 1, 10, x + 8, yp + 1, 12, "ferro")
    xa = c["bx1"] - 18
    e.caixa(xa, yp - 3, 0, xa + 14, yp + 4, 7, "fusta", "taulons", dalt="aigua")
    xe = c["bx0"] + 6
    for xx in (xe, xe + 12):
        e.caixa(xx, yp - 1, 0, xx + 2, yp + 1, 22, "noguera")
    e.caixa(xe, yp - 1, 20, xe + 14, yp + 1, 22, "noguera")
    for k, xx in enumerate((xe + 3, xe + 6.5, xe + 10)):
        e.caixa(xx, yp - 0.5, 9 + k % 2 * 2, xx + 1.5, yp + 0.5, 20, "ferro")
    for k in range(3):
        e.caixa(pc + 14, yp - 3 + k * 2.5, k % 2 * 2, pc + 30, yp - 1.5 + k * 2.5, 2 + k % 2 * 2, "ferro")


def botiga(e, c):
    a0, a1 = aparador(e, c)
    rotol_c(e, c, "poma")
    for k, x in enumerate(range(int(a0) + 4, int(a1) - 4, 12)):
        e.caixa(x, c["yp"] - 3, 0, x + 10, c["yp"] + 4, 6, "fusta", "taulons")
        e.sprite((x + 5, c["yp"], 6), mata(4, 2, flors=("teula", "flor2") if k % 2 else ("fulla",), llavor=c["llavor"] + k))


def mercat(e, c):
    aparador(e, c, ratlla=("porticons", "tela"))
    rotol_c(e, c, "poma")
    xs = [x for x in range(int(c["bx0"]) + 16, int(c["bx1"]) - 12, 28) if abs(x - c["pc"]) > 22]
    for k, x in enumerate(xs[:3]):
        parada(e, x, c["yp"] + 1, c["llavor"] + 10 * k, (("tela_verm", "tela"), ("porticons", "tela"), ("roba2", "tela"))[k % 3])


def joieria(e, c):
    a0, a1 = aparador(e, c, amb_tendal=False, dins=[("cortina", 6), ("llauto", 2), ("cortina", 6), ("cristall", 2)])
    for u in range(int(a0) + 3, int(a1) - 2, 5):                # pedres que brillen damunt el vellut
        e.rect("y", c["by1"], 1, u, u + 1, c["vb0"] + 9, c["vb0"] + 10, "cristall", to=0, biaix=1.2)
    rotol_c(e, c, "anell", taula="llauto")
    for dx in (-18, 18):
        e.caixa(c["pc"] + dx - 4, c["yp"] - 4, 0, c["pc"] + dx + 4, c["yp"] + 4, 6, "pedra_clara", dalt="terra")
        e.sprite((c["pc"] + dx, c["yp"], 6), mata(5, 6, flors=("boix",), llavor=c["llavor"] + dx))


def banc(e, c):
    """Pòrtic de dues columnes amb frontó de coure, escut de la ciutat i finestres altes."""
    pc, by1, z_pb = c["pc"], c["by1"], c["z_pb"]
    finestres_facana(e, c, "altes")
    for dx in (-14, 14):
        e.cilindre(pc + dx, by1 + 8, 0, z_pb + 50, 3, "pedra_clara", costats=10)
    e.caixa(pc - 19, by1, z_pb + 50, pc + 19, by1 + 12, z_pb + 55, "pedra_clara", dalt="pedra_clara")
    e.teulada(pc - 18, by1, pc + 18, by1 + 12, z_pb + 55, 10, mat="verdet", tex="teules", voladis=1, eix="y",
              mat_timpa="pedra_clara", tex_timpa="llis")
    x = c["bx0"] + 10                                           # escut: vermell amb la banda blanca i ganxos d'or
    e.rect("y", by1, 1, x, x + 10, z_pb + 16, z_pb + 36, "tela_verm", biaix=0.8)
    e.poligon_cara("y", by1, 1, [(x, z_pb + 30), (x + 10, z_pb + 20), (x + 10, z_pb + 23), (x, z_pb + 33)], "tela", to=0, biaix=1.0)
    for k in range(3):
        e.rect("y", by1, 1, x + 2 + k * 3, x + 3 + k * 3, z_pb + 18, z_pb + 19, "llauto", to=0, biaix=1.1)
    rotol(e, pc + 22, by1, z_pb + 30, "monedes", 13, 11, "llauto")


def sastreria(e, c):
    aparador(e, c, ratlla=("roba2", "roba1"),
             dins=[("roba2", 3), ("tela_verm", 3), ("roba3", 3), ("estuc_rosa", 3), ("porticons", 3), ("roba1", 3)])
    rotol_c(e, c, "agulla")
    e.sprite((c["bx1"] - 10, c["yp"], 0), mata(5, 5, llavor=c["llavor"] + 7))


def posada(e, c):
    """Galeria de fusta a la primera planta, porxo, rètol de la clau, bancs, bótes i palla."""
    porxo(e, c["pc"], c["by1"], c["z_pb"], ample=26)
    z = c["z_pa"] + 4
    y1 = c["by1"] + 2
    e.caixa(c["bx0"], y1, z - 3, c["bx1"], y1 + 7, z, "fusta", dalt="fusta")
    e.caixa(c["bx0"], y1 + 6, z + 9, c["bx1"], y1 + 7, z + 10, "noguera")
    for u in range(int(c["bx0"]), int(c["bx1"]), 4):
        e.caixa(u, y1 + 6, z, u + 1, y1 + 7, z + 9, "noguera")
    finestres_facana(e, c, "porticons")
    rotol_c(e, c, "clau")
    yp = c["yp"]
    for x in (c["bx0"] + 8, c["bx1"] - 30):
        e.caixa(x, yp - 2, 0, x + 18, yp + 2, 6, "noguera", dalt="fusta")
        e.caixa(x, yp - 3, 6, x + 18, yp - 2, 12, "noguera")
    pila_botes(e, c["bx1"] - 12, yp + 2)


def banys(e, c):
    """Cúpula de coure sobre un tambor octagonal amb finestres que surt del carener, llanterna
    de cristall i vapor; banyeres de fusta i tovalloles."""
    from monuments import finestra_pla, poligon_regular, tronc
    cx, cy, g = (c["bx0"] + c["bx1"]) / 2, (c["by0"] + c["by1"]) / 2, math.pi / 8
    rd = min(26, min(c["bx1"] - c["bx0"], c["by1"] - c["by0"]) / 4)
    e.evita_posador.append((cx, cy, rd + 10))                  # cap colom dins la cúpula
    zb, zd = c["z_t"] + 2, c["z_t"] + c["mig"] + 10                # el tambor neix sota la teulada
    tronc(e, cx, cy, zb, zd, rd, rd, 8, "pedra_clara", "carreus", gir=g)
    vs = poligon_regular(cx, cy, rd + 0.4, 8, g)
    for k in range(8):
        (xa, ya), (xb, yb) = vs[k], vs[(k + 1) % 8]
        n = ((xa + xb) / 2 - cx, (ya + yb) / 2 - cy, 0)
        finestra_pla(e, xa + (xb - xa) * 0.25, ya + (yb - ya) * 0.25, xb - (xb - xa) * 0.25, yb - (yb - ya) * 0.25,
                     zd - 14, zd - 4, n, mat="cristall")
    tronc(e, cx, cy, zd, zd + 3, rd + 3, rd + 3, 8, "pedra_clara", gir=g)
    z, r = zd + 3, rd + 1
    for a0, a1 in ((0, 30), (30, 58), (58, 82)):                  # mitja esfera en tres anells
        r0, r1 = r * math.cos(math.radians(a0)), r * math.cos(math.radians(a1))
        z0, z1 = z + r * 0.8 * math.sin(math.radians(a0)), z + r * 0.8 * math.sin(math.radians(a1))
        tronc(e, cx, cy, z0, z1, r0, r1, 8, "verdet", "teules", gir=g)
    zl = z + r * 0.8 * math.sin(math.radians(82))
    tronc(e, cx, cy, zl, zl + 8, 4.5, 4.5, 8, "cristall", gir=g)
    tronc(e, cx, cy, zl + 8, zl + 13, 6.5, 0, 8, "verdet", gir=g)
    e.caixa(cx - 1, cy - 1, zl + 13, cx + 1, cy + 1, zl + 19, "llauto")
    e.sprite((cx, cy, zl + 8), fum(c["llavor"] + 3), despres=True)
    finestres_facana(e, c, "altes", evita_aparador=False)
    rotol_c(e, c, "gota", taula="llauto")
    yp = c["yp"]
    xs = [x for x in (c["bx0"] + 12, c["bx1"] - 14) if abs(x - c["pc"]) > 20]
    for x in xs:
        e.cilindre(x, yp, 0, 6, 7, "fusta", "cercles", dalt="aigua")
    e.rect("x", c["bx1"] + 1, 1, c["by0"] + 4, c["by1"] - 4, c["z_pb"] + 30, c["z_pb"] + 31, "ferro", biaix=1.4)
    for j, u in enumerate(range(int(c["by0"]) + 8, int(c["by1"]) - 8, 8)):
        e.rect("x", c["bx1"] + 1, 1, u, u + 6, c["z_pb"] + 22, c["z_pb"] + 30, "roba1", biaix=1.5)


def casa_te(e, c):
    finestres_facana(e, c, "altes")
    rotol_c(e, c, "tassa", taula="llauto")
    yp = c["yp"]
    for x in [x for x in (c["bx0"] + 14, c["bx1"] - 16) if abs(x - c["pc"]) > 20]:
        terrassa(e, x, yp + 1, para_sol=True)
    for dx in (-16, 16):
        e.caixa(c["pc"] + dx, yp - 1, 0, c["pc"] + dx + 2, yp + 1, 26, "ferro")
        e.caixa(c["pc"] + dx - 1, yp - 2, 20, c["pc"] + dx + 3, yp + 2, 26, "cristall", dalt="llauto")


def armeria(e, c):
    from patis import armes, estendard
    finestres_facana(e, c, "simple")
    rotol_c(e, c, "espasa")
    by1, z = c["by1"], c["z_pb"] + 22
    for x in [x for x in (c["bx0"] + 8, c["bx1"] - 20) if abs(x - c["pc"]) > 18]:   # escuts a la façana
        e.rect("y", by1, 1, x, x + 10, z, z + 12, "pissarra", to=2, biaix=0.8)
        e.rect("y", by1, 1, x + 4, x + 6, z + 1, z + 11, "llauto", biaix=1.0)
    yp = c["yp"]
    armes(e, c["bx0"] + 14, yp)
    armes(e, c["bx1"] - 18, yp)
    estendard(e, c["pc"] + 22, yp + 2)


OFICIS = {"taverna": taverna, "ferreria": ferreria, "botiga": botiga, "mercat": mercat, "joieria": joieria,
          "banc": banc, "sastreria": sastreria, "posada": posada, "banys": banys, "casa_te": casa_te, "armeria": armeria}


def xemeneia_fum(e, x, y, z, llavor=1):
    e.sprite((x, y, z), fum(llavor), despres=True)
