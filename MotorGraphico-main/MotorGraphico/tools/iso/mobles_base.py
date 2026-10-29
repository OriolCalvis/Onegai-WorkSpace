#!/usr/bin/env python3
"""ISO (INT-01) — Ajudes comunes dels mobles del kit (potes, botes ajagudes,
gerres, espelmes, ampolles, llibres...) i el registre on cada mòdul de
mobles apunta les seves peces (id, com es fa, nom, si gira, si fa nosa)."""
import math
import os
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sales  # noqa: E402,F401  (materials de les sales: vellut, pergami, paper...)
import vegetacio  # noqa: E402,F401  (materials de la vegetació: fruits, flors, fulles)
from monuments import anell, poligon_regular, tronc  # noqa: E402,F401
from peces_iso import MATERIALS, Escena, color_de, mata  # noqa: E402,F401

MATERIALS.update({
    "llana_verda": [(186, 226, 160), (138, 196, 116), (98, 160, 84), (70, 124, 64), (46, 88, 46)],
    "llana_lila": [(222, 196, 240), (190, 152, 220), (152, 112, 190), (116, 82, 152), (82, 56, 112)],
    "paper_cel": [(255, 252, 236), (250, 240, 214), (234, 220, 186), (204, 186, 150), (160, 142, 110)],
    "peix": [(226, 236, 244), (186, 204, 220), (146, 168, 190), (110, 128, 150), (78, 92, 110)],
    "pa": [(250, 218, 150), (232, 186, 110), (204, 150, 76), (164, 112, 52), (118, 78, 36)],
    "carbo": [(110, 110, 120), (80, 80, 90), (58, 58, 66), (40, 40, 48), (26, 26, 32)],
    "palla": [(255, 238, 170), (240, 214, 124), (214, 182, 88), (176, 144, 62), (130, 104, 42)],
})
COLORS_LLIBRES = ("tela_verm", "porticons", "roba2", "roba3", "pergami", "vellut", "verdet", "llana_lila")


class Registre:
    """Les peces d'un mòdul: id -> funció que fa l'Escena, el nom del catàleg,
    si surt girada (o1..o3) i si fa nosa; i els mobles llargs, en trams."""

    def __init__(self):
        self.mobles, self.trams, self.noms = {}, {}, {}
        self.orientables, self.sense_nosa = set(), set()

    def afegeix(self, mid, fer, nom, gira=False, nosa=True):
        assert mid not in self.mobles and mid not in self.trams, f"moble repetit: {mid}"
        self.mobles[mid], self.noms[mid] = fer, nom
        if gira:
            self.orientables.add(mid)
        if not nosa:
            self.sense_nosa.add(mid)

    def trams_de(self, mid, trams, nom):
        """trams: {nom del tram: funció}; surten sempre en 4 orientacions."""
        assert mid not in self.mobles and mid not in self.trams, f"moble repetit: {mid}"
        self.trams[mid], self.noms[mid] = trams, nom


def amb_veins(fer, veins):
    """La mateixa peça amb els trams del costat (dx, dy en cel·les): el contorn no talla la juntura."""
    def f():
        e = fer()
        e.veins = veins
        return e
    return f


# ---------------------------------------------------------------- ajudes
def potes(e, x0, y0, x1, y1, z1, g=2, mat="noguera"):
    for x in (x0, x1 - g):
        for y in (y0, y1 - g):
            e.caixa(x, y, 0, x + g, y + g, z1, mat)


def cilindre_x(e, x0, x1, cy, cz, r, mat, tex="cercles_x", costats=12, tapa=None):
    """Cilindre ajagut al llarg de X (botes de vi, rotlles de roba)."""
    p = e.nova_peca()
    pts = [(cy + r * math.cos(2 * math.pi * k / costats), cz + r * math.sin(2 * math.pi * k / costats))
           for k in range(costats)]
    for k in range(costats):
        (ya, za), (yb, zb) = pts[k], pts[(k + 1) % costats]
        n = (0, (ya + yb) / 2 - cy, (za + zb) / 2 - cz)
        e.cara([(x0, ya, za), (x1, ya, za), (x1, yb, zb), (x0, yb, zb)], n, color_de(mat, tex, "y"), p)
    e.cara([(x1, y, z) for y, z in pts], (1, 0, 0), color_de(tapa or mat, "llis", "x"), p)
    e.cara([(x0, y, z) for y, z in reversed(pts)], (-1, 0, 0), color_de(tapa or mat, "llis", "x"), p)
    return p


def cilindre_y(e, y0, y1, cx, cz, r, mat, tex="llis", costats=12, tapa=None):
    """Cilindre ajagut al llarg de Y."""
    p = e.nova_peca()
    pts = [(cx + r * math.cos(2 * math.pi * k / costats), cz + r * math.sin(2 * math.pi * k / costats))
           for k in range(costats)]
    for k in range(costats):
        (xa, za), (xb, zb) = pts[k], pts[(k + 1) % costats]
        n = ((xa + xb) / 2 - cx, 0, (za + zb) / 2 - cz)
        e.cara([(xa, y0, za), (xa, y1, za), (xb, y1, zb), (xb, y0, zb)], n, color_de(mat, tex, "x"), p)
    e.cara([(x, y1, z) for x, z in reversed(pts)], (0, 1, 0), color_de(tapa or mat, "llis", "y"), p)
    e.cara([(x, y0, z) for x, z in pts], (0, -1, 0), color_de(tapa or mat, "llis", "y"), p)
    return p


def gerra(e, x, y, z):
    """Gerra de cervesa amb escuma."""
    e.cilindre(x, y, z, z + 4, 1.6, "fusta", "cercles", costats=8)
    e.cilindre(x, y, z + 4, z + 5, 1.7, "roba1", costats=8)
    e.caixa(x + 1.5, y - 0.4, z + 1, x + 2.6, y + 0.4, z + 3.5, "noguera")


def espelma(e, x, y, z, alt=5):
    e.cilindre(x, y, z, z + alt, 0.9, "cera", costats=6)
    tronc(e, x, y, z + alt, z + alt + 3, 0.8, 0.1, 4, "foc")


def ampolla(e, x, y, z, mat="verdet", alt=7):
    e.cilindre(x, y, z, z + alt, 1.4, mat, costats=6)
    e.cilindre(x, y, z + alt, z + alt + 3, 0.5, mat, costats=4)


def tassa(e, x, y, z, mat="pedra_clara"):
    e.cilindre(x, y, z, z + 2.5, 1.3, mat, costats=8)


def llibres_drets(e, x0, x1, y0, y1, z, rng, alt=(7, 11), buits=0.06):
    """Una filera de llibres drets de colors entre x0 i x1 (sobre una lleixa a l'alçada z)."""
    x = x0
    while x < x1 - 1:
        w = 1.3 + rng.random() * 1.1
        if rng.random() < buits:
            x += w + 0.4
            continue
        h = alt[0] + rng.random() * (alt[1] - alt[0])
        e.caixa(x, y0, z, min(x + w, x1), y1, z + h, COLORS_LLIBRES[rng.integers(len(COLORS_LLIBRES))])
        x += w + 0.15


def llibre_obert(e, x, y, z, mat="tela_verm"):
    """Un llibre obert: les tapes i les dues pàgines blanques en V suau."""
    e.caixa(x - 5, y - 3.5, z, x + 5, y + 3.5, z + 0.8, mat)
    for s in (-1, 1):
        p = e.nova_peca()
        e.cara([(x, y - 3, z + 0.8), (x + 4.6 * s, y - 3, z + 1.6), (x + 4.6 * s, y + 3, z + 1.6), (x, y + 3, z + 0.8)],
               (0, 0, 1), color_de("pergami", "llis", "z", to_fix=0 if s < 0 else 1), p, 0.2, True)


def coixi(e, x0, y0, x1, y1, z, alt, mat):
    e.caixa(x0, y0, z, x1, y1, z + alt, mat, dalt=mat)
    e.caixa(x0 + 1, y0 + 1, z + alt, x1 - 1, y1 - 1, z + alt + 1, mat, dalt=mat)


def vapor(llavor=1, bufs=3, mida=1.0):
    """Vapor d'aigua calenta (té, banys): bufs blancs que pugen, sense contorn."""
    from peces_iso import fum

    def f(ll, sx, sy, prof):
        fum(llavor, bufs)(ll, sx, sy, prof)
    return f


def rng_de(llavor):
    return np.random.default_rng(llavor)
