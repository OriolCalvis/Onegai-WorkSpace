#!/usr/bin/env python3
"""ISO (INT-01) — Les peces grans dels interiors, de més d'una cel·la: la
tarima del tron, l'escenari de l'òpera, l'altar de l'església, la piscina
dels banys, la forja, la taula de guerra i el llit amb dosser. Al motor són
com els monuments (Game::Monumentos): una huella de rajoles int_peca_<nom>
(que fan nosa) de la mida de la peça, dibuixada a tires de 32 px.

La cara de davant és +Y (o0), l'empremta va de (0, 0) a (W, D) cel·les i
l'àncora és la cantonada de davant, com els monuments del kit.

    from peces_grans import PECES
    (W, D, terra), fer = PECES["tron"]
    e = fer()                      # Escena de W x D cel·les; e.dibuixa(0, alt_max=...)
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from families import serrell  # noqa: E402
from mobles_base import (Escena, anell, cilindre_x, coixi, color_de, espelma, gerra, llibre_obert,  # noqa: E402
                         poligon_regular, potes, tronc)
from peces_iso import fum  # noqa: E402


def graons_front(e, x0, x1, y_front, n, fons, alt, mat, dalt=None):
    """Graons de pujada a la cara +Y d'una tarima (de baix a dalt, cap a dins)."""
    for k in range(n):
        y = y_front - k * fons
        e.caixa(x0, y - fons, 0, x1, y, alt * (k + 1), mat, dalt=dalt or mat)


# ---------------------------------------------------------------- el tron
def tron():
    """5 x 3: tarima de tres graons coberta de vellut, el tron daurat al mig,
    braserets de cristall a les cantonades, dos estendards i, damunt del tron,
    l'anell d'or que sura amb el cristall dels Guardians."""
    W, D = 5 * 32, 3 * 32
    e = Escena(5, 3)
    e.caixa(0, 0, 0, W, D, 7, "pedra_clara", dalt="vellut")
    e.caixa(0, D - 3, 7, W, D, 7.4, "or")
    e.caixa(16, 0, 7, W - 16, D - 22, 14, "pedra_clara", dalt="vellut")
    e.caixa(16, D - 25, 14, W - 16, D - 22, 14.4, "or")
    graons_front(e, 56, W - 56, D, 3, 7, 4.6, "pedra_clara", "vellut")
    # el tron
    cx = W / 2
    e.caixa(cx - 18, 6, 14, cx + 18, 40, 26, "noguera", "plafons", dalt="vellut")
    e.caixa(cx - 18, 2, 26, cx + 18, 8, 92, "noguera", "plafons")
    e.caixa(cx - 15, 7.5, 30, cx + 15, 8.6, 86, "vellut")
    for x in (cx - 22, cx + 18):
        e.caixa(x, 4, 14, x + 4, 40, 40, "or", dalt="or")
        e.caixa(x - 1, 36, 40, x + 5, 42, 45, "or", dalt="or")
    e.caixa(cx - 20, 1, 88, cx + 20, 9, 96, "or", dalt="or")
    for dx in (-14, 0, 14):
        tronc(e, cx + dx, 5, 96, 104 if dx else 112, 3 if dx else 4, 0, 4, "or")
    tronc(e, cx, 6, 118, 126, 0.5, 5, 4, "cristall")
    tronc(e, cx, 6, 126, 138, 5, 0, 4, "cristall")
    anell(e, cx, 6, 124, 126, 18, 15, 24, "or")
    anell(e, cx, 6, 134, 135.5, 12, 10, 20, "llauto", gir=0.3)
    # braserets de cristall i estendards
    for x in (22, W - 22):
        tronc(e, x, D - 34, 14, 22, 6, 8, 8, "llauto", dalt="llauto")
        tronc(e, x, D - 34, 22, 30, 0.5, 6, 4, "cristall")
        tronc(e, x, D - 34, 30, 40, 6, 0, 4, "cristall")
    for x in (34, W - 34):
        e.cilindre(x, 12, 14, 100, 1.2, "noguera", costats=6)
        tronc(e, x, 12, 100, 105, 2, 0, 6, "or")
        e.caixa(x - 9, 12.5, 52, x + 9, 13.6, 96, "tela_verm", "plecs")
        e.caixa(x - 9, 12.7, 72, x + 9, 13.8, 76, "roba1")
        e.caixa(x - 10, 12, 96, x + 10, 14, 98, "or")
    return e


# ---------------------------------------------------------------- l'escenari
def escenari():
    """10 x 4: l'escenari de l'òpera, de taulons, amb el frontal daurat, les
    candilejas de cristall, el teló de vellut recollit als costats i la
    bambolina amb serrell d'or; damunt, l'anell de la sala que sura."""
    W, D = 10 * 32, 4 * 32
    e = Escena(10, 4)
    e.caixa(0, 0, 0, W, D, 16, "noguera", "plafons", dalt="fusta", tex_dalt="taulons")
    e.caixa(0, D - 2, 14, W, D + 0.5, 17, "or", dalt="or")
    for x in range(16, W - 8, 32):
        tronc(e, x, D - 6, 16, 19, 3, 3, 8, "llauto", dalt="llauto")
        tronc(e, x, D - 6, 19, 23, 0.5, 2.6, 4, "cristall")
        tronc(e, x, D - 6, 23, 26, 2.6, 0, 4, "cristall")
    for x0 in (0, W - 26):                                  # els cortinatges dels costats
        e.caixa(x0, D - 22, 16, x0 + 26, D - 10, 150, "vellut", "plecs", dalt="vellut")
        e.caixa(x0 + (20 if x0 == 0 else 0), D - 24, 60, x0 + (26 if x0 == 0 else 6), D - 8, 63, "or")
    e.caixa(0, D - 24, 136, W, D - 8, 154, "vellut", "plecs", dalt="vellut")
    serrell(e, (0, D - 8), (W, D - 8), 136, (0, 1, 0), "or", alt=6, pas=6)
    e.caixa(0, D - 25, 154, W, D - 7, 158, "or", dalt="or")
    # l'atrezzo: dues columnes de cartó i un trono pintat
    for x in (80, W - 80):
        e.cilindre(x, 24, 16, 90, 6, "pedra_clara", "taulons_v", costats=10)
        e.caixa(x - 8, 16, 90, x + 8, 32, 94, "pedra_clara", dalt="pedra_clara")
    e.caixa(W / 2 - 14, 10, 16, W / 2 + 14, 34, 30, "or", dalt="vellut")
    e.caixa(W / 2 - 14, 8, 30, W / 2 + 14, 12, 64, "or")
    return e


# ---------------------------------------------------------------- l'altar
def altar():
    """4 x 3: l'altar de l'església en ruïnes: tarima de pedra fosca de dos
    graons, la mesa amb el drap vermell, canelobres alts, el llibre obert i
    l'anell de pedra trencat que encara sura damunt (els Perduts no l'han tocat)."""
    W, D = 4 * 32, 3 * 32
    e = Escena(4, 3)
    e.caixa(0, 0, 0, W, D, 6, "pedra_fosca", "carreus", dalt="pedra_grisa")
    e.caixa(12, 0, 6, W - 12, D - 24, 12, "pedra_fosca", "carreus", dalt="pedra_grisa")
    graons_front(e, 40, W - 40, D, 2, 6, 6, "pedra_grisa")
    e.caixa(30, 18, 12, W - 30, 44, 34, "pedra_clara", "carreus", dalt="pedra_clara")
    e.caixa(29, 17, 32, W - 29, 45, 35, "pedra_clara", dalt="tela_verm")
    e.caixa(48, 44, 14, W - 48, 45.2, 34, "tela_verm")
    e.caixa(50, 44.6, 20, W - 50, 45.6, 30, "or")
    llibre_obert(e, W / 2 - 10, 30, 35, "vellut")
    e.cilindre(W / 2 + 14, 30, 35, 36, 2.4, "llauto", costats=8)
    espelma(e, W / 2 + 14, 30, 36, 6)
    for x in (20, W - 20):
        tronc(e, x, 30, 12, 15, 5, 4, 8, "llauto")
        e.cilindre(x, 30, 15, 56, 1.4, "llauto", costats=6)
        e.caixa(x - 6, 29.4, 54, x + 6, 30.6, 55.5, "llauto")
        for dx in (-5, 0, 5):
            espelma(e, x + dx, 30, 55.5, 7 if dx == 0 else 5)
    # l'anell de pedra trencat (tres trossos que suren)
    cx, cy = W / 2, 24
    for k, (a0, a1, z) in enumerate(((0.2, 1.7, 84), (2.2, 3.6, 87), (4.0, 5.6, 83))):
        p = e.nova_peca()
        n = 6
        for i in range(n):
            ta, tb = a0 + (a1 - a0) * i / n, a0 + (a1 - a0) * (i + 1) / n
            xa, ya = cx + 26 * math.cos(ta), cy + 26 * math.sin(ta)
            xb, yb = cx + 26 * math.cos(tb), cy + 26 * math.sin(tb)
            xc, yc = cx + 20 * math.cos(tb), cy + 20 * math.sin(tb)
            xd, yd = cx + 20 * math.cos(ta), cy + 20 * math.sin(ta)
            e.cara([(xa, ya, z + 4), (xb, yb, z + 4), (xc, yc, z + 4), (xd, yd, z + 4)], (0, 0, 1),
                   color_de("pedra_grisa", "llis", "z"), p)
            e.cara([(xa, ya, z), (xb, yb, z), (xb, yb, z + 4), (xa, ya, z + 4)],
                   ((xa + xb) / 2 - cx, (ya + yb) / 2 - cy, 0), color_de("pedra_grisa", "carreus", "y"), p)
    tronc(e, cx, cy, 70, 78, 0.5, 4, 4, "cristall")
    tronc(e, cx, cy, 78, 88, 4, 0, 4, "cristall")
    return e


# ---------------------------------------------------------------- la piscina
def piscina():
    """5 x 4: la piscina de les termes, alçada sobre el terra: vora de pedra
    clara, l'aigua amb onades, uns graons per entrar i el vapor que en surt."""
    W, D = 5 * 32, 4 * 32
    e = Escena(5, 4)
    g = 8
    e.caixa(g, g, 0, W - g, D - g, 9, "estuc_blau", "rajoles", dalt="aigua")
    for (x0, y0, x1, y1) in ((0, 0, W, g), (0, 0, g, D), (W - g, 0, W, D), (0, D - g, W / 2 - 18, D),
                             (W / 2 + 18, D - g, W, D)):
        e.caixa(x0, y0, 0, x1, y1, 12, "pedra_clara", "carreus", dalt="pedra_clara")
    for k, (y0, h) in enumerate(((D - g, 4), (D - g - 6, 8))):   # els graons per entrar
        e.caixa(W / 2 - 18, y0, 0, W / 2 + 18, y0 + (g if k == 0 else 6), h, "pedra_clara", dalt="pedra_clara")
    p = e.nova_peca()
    for k in range(9):                                      # les onades
        y = g + 8 + k * 12
        if y > D - g - 16:
            break
        x0 = g + 6 + (k % 3) * 9
        e.cara([(x0, y, 9.1), (x0 + 14, y, 9.1), (x0 + 14, y + 1.2, 9.1), (x0, y + 1.2, 9.1)], (0, 0, 1),
               color_de("aigua", "llis", "z", to_fix=0), p, 0.3)
        e.cara([(x0 + 50, y + 5, 9.1), (x0 + 62, y + 5, 9.1), (x0 + 62, y + 6.2, 9.1), (x0 + 50, y + 6.2, 9.1)],
               (0, 0, 1), color_de("aigua", "llis", "z", to_fix=0), p, 0.3)
    for (x, y) in ((10, 10), (W - 10, 10), (10, D - 10), (W - 10, D - 10)):
        tronc(e, x, y, 12, 16, 4, 4, 8, "pedra_clara", dalt="pedra_clara")
        tronc(e, x, y, 16, 22, 2.4, 0, 4, "cristall")
    for k, (x, y) in enumerate(((40, 40), (100, 70), (70, 100), (130, 40))):
        e.sprite((x, y, 10), fum(30 + k, 3), despres=True)
    return e


# ---------------------------------------------------------------- la forja
def forja():
    """2 x 2: la forja de la ferreria contra la paret: llar de maó amb les
    brases, la campana que puja fins al sostre, la manxa i les eines penjades."""
    W, D = 2 * 32, 2 * 32
    e = Escena(2, 2)
    e.caixa(4, 4, 0, W - 4, D - 10, 22, "mao", "maons", dalt="mao")
    e.caixa(12, 12, 18, W - 12, D - 18, 22.5, "carbo", dalt="teula")
    for (x, y, h) in ((22, 22, 6), (32, 26, 8), (42, 22, 5), (28, 34, 5), (38, 34, 6)):
        tronc(e, x, y, 22, 22 + h, 3.6, 0.3, 6, "foc")
    p = e.nova_peca()
    e.cara([(4, D - 10, 40), (W - 4, D - 10, 40), (W - 14, 30, 70), (14, 30, 70)], (0, 3, 1),
           color_de("mao", "maons", "y"), p)
    e.cara([(W - 4, 4, 40), (W - 4, D - 10, 40), (W - 14, 30, 70), (W - 14, 4, 70)], (3, 0, 1),
           color_de("mao", "maons", "x"), p)
    e.caixa(4, 4, 38, W - 4, D - 10, 41, "noguera", dalt="noguera")
    e.caixa(14, 4, 70, W - 14, 30, 140, "mao", "maons", dalt="socol")
    for x in (10, W - 10):
        e.caixa(x - 1.5, D - 12, 0, x + 1.5, D - 9, 38, "noguera")
    # la manxa
    e.caixa(W - 12, 40, 4, W - 3, 58, 12, "noguera", dalt="encenall")
    e.caixa(W - 11, 42, 12, W - 4, 56, 15, "encenall", dalt="encenall")
    # eines penjades a la campana
    for k, x in enumerate((20, 30, 40)):
        e.caixa(x, D - 10.5, 28, x + 1.4, D - 9.6, 36, "ferro")
        e.caixa(x - 1.5 + k, D - 10.5, 26, x + 2.5, D - 9.6, 28, "ferro")
    e.sprite((W / 2, 20, 140), fum(41, 3), despres=True)
    return e


# ---------------------------------------------------------------- la taula de guerra
def taula_guerra():
    """3 x 2: la taula de guerra del castell amb el mapa en relleu de la
    comarca (el riu, els turons, Boundington) i les figuretes dels exèrcits."""
    W, D = 3 * 32, 2 * 32
    e = Escena(3, 2)
    e.caixa(6, 6, 14, W - 6, D - 6, 18, "noguera", "plafons", dalt="noguera")
    potes(e, 8, 8, W - 8, D - 8, 14, 4, "noguera")
    e.caixa(10, 10, 18, W - 10, D - 10, 19, "fulla", dalt="fulla")
    p = e.nova_peca()
    riu = [(10, 30), (30, 26), (50, 34), (70, 28), (W - 10, 32)]
    for (xa, ya), (xb, yb) in zip(riu, riu[1:]):
        e.cara([(xa, ya - 2, 19.1), (xb, yb - 2, 19.1), (xb, yb + 2, 19.1), (xa, ya + 2, 19.1)], (0, 0, 1),
               color_de("aigua", "llis", "z", to_fix=1), p, 0.3)
    for (x, y, r, h) in ((22, 18, 7, 7), (34, 16, 5, 5), (72, 44, 8, 6)):
        tronc(e, x, y, 19, 19 + h, r, 1, 8, "terra")
    e.caixa(52, 38, 19, 62, 46, 23, "pedra_castell", dalt="pissarra_fosca")
    e.caixa(55, 40, 23, 59, 44, 27, "pedra_castell", dalt="pissarra_fosca")
    for (x, y, m) in ((40, 40, "tela_verm"), (44, 44, "tela_verm"), (30, 46, "tela_verm"),
                      (76, 20, "roba2"), (80, 24, "roba2"), (70, 16, "roba2")):
        e.cilindre(x, y, 19, 22, 1.4, m, costats=6)
        tronc(e, x, y, 22, 24, 1.4, 0, 6, m)
    for (x, y) in ((10, 10), (W - 10, 10), (10, D - 10), (W - 10, D - 10)):
        e.cilindre(x, y, 18, 19, 2.2, "llauto", costats=8)
        espelma(e, x, y, 19, 5)
    return e


# ---------------------------------------------------------------- el llit amb dosser
def llit_dosser():
    """2 x 3: el llit reial amb dosser de vellut i les columnes de noguera; el capçal a -Y."""
    W, D = 2 * 32, 3 * 32
    e = Escena(2, 3)
    e.caixa(4, 8, 0, W - 4, D - 6, 12, "noguera", "plafons", dalt="noguera")
    e.caixa(5, 9, 12, W - 5, D - 7, 18, "roba1", dalt="roba1")
    coixi(e, 10, 11, 30, 22, 18, 4, "roba1")
    coixi(e, 34, 11, 54, 22, 18, 4, "roba1")
    e.caixa(5, 30, 12, W - 5, D - 7, 20, "vellut", dalt="vellut")
    e.caixa(4.5, 29, 18, W - 4.5, 32, 21, "roba1", dalt="roba1")
    e.caixa(4, 4, 0, W - 4, 9, 44, "noguera", "plafons", dalt="noguera")
    e.caixa(6, 3, 40, W - 6, 10, 48, "or", dalt="or")
    for (x, y) in ((4, 4), (W - 8, 4), (4, D - 10), (W - 8, D - 10)):
        e.caixa(x, y, 0, x + 4, y + 4, 92, "noguera", dalt="noguera")
        tronc(e, x + 2, y + 2, 92, 98, 2.4, 0, 6, "or")
    e.caixa(2, 2, 86, W - 2, D - 8, 92, "vellut", "plecs", dalt="vellut")
    serrell(e, (2, D - 8), (W - 2, D - 8), 86, (0, 1, 0), "or", alt=5, pas=5)
    for x0 in (2, W - 6):                                   # cortines recollides
        e.caixa(x0, D - 12, 22, x0 + 4, D - 8, 86, "vellut", "plecs")
        e.caixa(x0 - 0.5, D - 12.5, 50, x0 + 4.5, D - 7.5, 53, "or")
    e.caixa(W - 6, 12, 22, W - 2, 30, 86, "vellut", "plecs")
    return e


# nom: ((ample, fons, terra de sota), fer)
PECES = {
    "tron": ((5, 3, "int_lloses"), tron),
    "escenari": ((10, 4, "int_espiga"), escenari),
    "altar": ((4, 3, "int_lloses_fosques"), altar),
    "piscina": ((5, 4, "int_rajola"), piscina),
    "forja": ((2, 2, "int_batuda"), forja),
    "taula_guerra": ((3, 2, "int_espiga"), taula_guerra),
    "llit_dosser": ((2, 3, "int_espiga"), llit_dosser),
}
