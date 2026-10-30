#!/usr/bin/env python3
"""ISO (INT-01) — Les sales dels interiors a la manera d'Habbo: les dues parets
del fons, primes i altes, amb la cara cap a la sala, el pilar de la cantonada
i el terra.

Una sala de W x H cel·les té la paret de la fila y = 0 (cara +Y, vista o0) i
la de la columna x = 0 (cara +X, la mateixa peça vista o3); el davant queda
obert, com a Habbo. Cada cel·la de paret és un objecte (paret_<estil>_<deco>,
i _o3 per a la columna), amb la paret al llindar de la cel·la de la sala:

    from sales import PARETS
    e = PARETS["paret_fusta_finestra"]()    # Escena d'una cel·la; dibuixa(0) o dibuixa(3)

El terra són rajoles noves de terreno_mundo (int_*): textures periòdiques
cada cel·la, sense quadrícula on el material no en té (taulons, catifes).
"""
import math
import os
import sys

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import monuments_ciutat  # noqa: E402,F401  (materials dels monuments: pedra_castell, or, cera...)
from families import serrell  # noqa: E402
from monuments import anell, tronc  # noqa: E402
from peces_iso import MATERIALS, Escena, color_de  # noqa: E402

MATERIALS.update({
    "paper_verm": [(236, 150, 140), (212, 104, 98), (178, 70, 72), (140, 48, 56), (100, 32, 42)],
    "paper_blau": [(176, 200, 238), (134, 162, 216), (98, 126, 188), (70, 94, 152), (46, 64, 112)],
    "paper_verd": [(184, 218, 164), (140, 188, 124), (102, 154, 96), (72, 120, 74), (48, 86, 54)],
    "rajola": [(255, 255, 255), (234, 244, 252), (204, 224, 242), (156, 188, 224), (104, 142, 194)],
    "mao": [(226, 150, 112), (200, 118, 86), (170, 90, 66), (134, 66, 50), (94, 46, 38)],
    "vellut": [(240, 104, 112), (208, 62, 78), (174, 38, 58), (134, 24, 44), (94, 16, 32)],
    "pergami": [(255, 246, 214), (246, 228, 182), (226, 202, 146), (190, 164, 110), (146, 122, 80)],
    # INT-02: la boira lila de Mistarium dins del mirall
    "boira_lila": [(250, 236, 255), (226, 204, 246), (196, 166, 230), (160, 126, 204), (120, 90, 170)],
})

G = 6                       # gruix de la paret (unitats)
ALT = 104                   # alçada de paret de les sales normals
ALT_SALA = 136              # les sales grans dels monuments

# Estils de paret: la cara de la sala per capes (de baix a dalt).
#   mur: (material, textura) del parament; cap: material del cantell de dalt;
#   socol: material del sòcol; arrimador: (material, textura, alçada) o None;
#   motllura: material del llistó de l'arrimador; cornisa: material de la cornisa;
#   entramat: material dels pilars i la biga (parets de fusta i guix).
ESTILS = {
    "fusta": dict(mur=("estuc", "estuc"), cap="noguera", socol="noguera", arrimador=("fusta", "taulons_v", 38),
                  motllura="noguera", cornisa=None, entramat="noguera"),
    "estuc": dict(mur=("estuc_ocre", "estuc"), cap="estuc", socol="noguera", arrimador=("noguera", "plafons", 30),
                  motllura="noguera", cornisa="estuc"),
    "estuc_blau": dict(mur=("estuc_blau", "estuc"), cap="estuc", socol="noguera", arrimador=("fusta", "plafons", 30),
                       motllura="noguera", cornisa="estuc"),
    "noble": dict(mur=("paper_verm", "damasc"), cap="noguera", socol="noguera", arrimador=("noguera", "plafons", 34),
                  motllura="or", cornisa="or"),
    "noble_blau": dict(mur=("paper_blau", "damasc"), cap="noguera", socol="noguera", arrimador=("noguera", "plafons", 34),
                       motllura="or", cornisa="or"),
    "noble_verd": dict(mur=("paper_verd", "damasc"), cap="noguera", socol="noguera", arrimador=("noguera", "plafons", 34),
                       motllura="or", cornisa="or"),
    "pedra": dict(mur=("pedra_castell", "carreus"), cap="pedra_clara", socol="pedra_grisa", arrimador=None,
                  motllura=None, cornisa="pedra_clara"),
    "pedra_calida": dict(mur=("pedra", "carreus"), cap="pedra_clara", socol="socol", arrimador=None,
                         motllura=None, cornisa="pedra_clara"),
    "pedra_fosca": dict(mur=("pedra_fosca", "carreus"), cap="pedra_grisa", socol="socol", arrimador=None,
                        motllura=None, cornisa="pedra_grisa"),
    "rajola": dict(mur=("rajola", "rajoles"), cap="pedra_clara", socol="estuc_blau", arrimador=("estuc_blau", "rajoles", 28),
                   motllura="pedra_clara", cornisa="pedra_clara"),
    "mao": dict(mur=("mao", "maons"), cap="socol", socol="socol", arrimador=None, motllura=None, cornisa="noguera"),
}


def _r(e, u0, u1, v0, v1, mat, tex="llis", to=None, desp=0, biaix=0.6, arc=False, y=32):
    """Rectangle sobre la cara de la sala (el pla Y = 32 a la vista o0)."""
    e.rect("y", y, 1, u0, u1, v0, v1, mat, tex, to=to, desp=desp, biaix=biaix, arc=arc)


def _cos(e, s, alt):
    mat, tex = s["mur"]
    e.caixa(0, 32 - G, 0, 32, 32, alt, mat, tex, dalt=s["cap"])
    if s.get("entramat"):
        f = s["entramat"]
        e.caixa(0, 32 - G, 0, 3, 32.6, alt, f)                     # el pilar (un a cada cel·la: fa ritme)
        e.caixa(0, 32 - G, alt - 7, 32, 32.6, alt, f, dalt=s["cap"])  # la biga
    if s.get("arrimador"):
        m, t, h = s["arrimador"]
        _r(e, 0, 32, 0, h, m, t, biaix=0.3)
        if s.get("motllura"):
            e.caixa(0, 32, h, 32, 33.2, h + 2.5, s["motllura"])
    if s.get("socol"):                                                             # sòcol a ras (no surt per baix)
        _r(e, 0, 32, 0, 4, s["socol"], biaix=0.5)
    if s.get("cornisa"):
        e.caixa(0, 32, alt - 6, 32, 34, alt - 3, s["cornisa"])
        e.caixa(0, 32, alt - 3, 32, 33, alt, s["cornisa"])


# ---------------------------------------------------------------- decoracions
def _finestra(e, s, alt, alta=False):
    pedra = s["mur"][0].startswith("pedra") or s["mur"][0] == "mao"
    v0, v1 = (34, alt - 22) if alta else (40, 82)
    u0, u1 = (9, 23) if alta else (8, 24)
    marc = "pedra_clara" if pedra else "noguera"
    _r(e, u0 - 2, u1 + 2, v0 - 2, v1 + 2, marc, to=1 if pedra else 2, arc=pedra or alta)
    _r(e, u0, u1, v0, v1, "vidre", to=1, biaix=0.9, arc=pedra or alta)
    _r(e, u0, (u0 + u1) / 2 - 0.5, v0 + 2, v1 - 8, "vidre", to=0, biaix=1.0)      # el reflex
    _r(e, (u0 + u1) / 2 - 0.6, (u0 + u1) / 2 + 0.6, v0, v1 - (u1 - u0) / 2 * 0.2, marc, to=3, biaix=1.1)
    for v in (v0 + (v1 - v0) * 0.45,) if not alta else (v0 + (v1 - v0) * 0.35, v0 + (v1 - v0) * 0.65):
        _r(e, u0, u1, v - 0.6, v + 0.6, marc, to=3, biaix=1.1)
    e.caixa(u0 - 3, 32, v0 - 4, u1 + 3, 35, v0 - 2, marc)                          # l'ampit
    if s["mur"][0].startswith("paper"):                                           # cortines de vellut
        for a, b in ((u0 - 4, u0 + 1), (u1 - 1, u1 + 4)):
            e.caixa(a, 32, v0 - 6, b, 34, v1 + 4, "vellut", "plecs")
        e.caixa(u0 - 6, 32, v1 + 4, u1 + 6, 34.5, v1 + 7, "or")


def _quadre(e, s, alt):
    marc = "or" if s["mur"][0].startswith("paper") or s["mur"][0] == "pedra_castell" else "noguera"
    e.caixa(7, 32, 44, 25, 33.5, 72, marc)
    _r(e, 9, 23, 46, 70, "estuc_blau", to=1, biaix=0.4, y=33.5)                 # cel
    _r(e, 9, 23, 46, 55, "fulla", to=2, biaix=0.6, y=33.5)                      # turons
    _r(e, 15, 23, 46, 52, "fulla", to=3, biaix=0.7, y=33.5)
    _r(e, 17, 20, 62, 65, "or", to=0, biaix=0.7, y=33.5)                        # el sol


def _prestatge(e, s, alt, llavor=3):
    rng = np.random.default_rng(llavor)
    e.caixa(4, 32, 52, 28, 39, 54, "noguera")
    for x in (7, 24):
        e.caixa(x, 32, 46, x + 1.5, 36, 52, "ferro")
    x = 6.0
    while x < 26:
        tipus = rng.integers(3)
        if tipus == 0:                                                            # ampolla
            mat = ("verdet", "vidre", "tela_verm", "llauto")[rng.integers(4)]
            e.cilindre(x + 1.5, 35.5, 54, 61, 1.6, mat, costats=6)
            e.cilindre(x + 1.5, 35.5, 61, 64, 0.6, mat, costats=4)
            x += 4
        elif tipus == 1:                                                          # pot
            e.cilindre(x + 2, 35.5, 54, 59, 2.2, ("terra", "pedra_clara", "llauto")[rng.integers(3)], costats=8)
            x += 5
        else:                                                                     # llibres drets
            for k in range(rng.integers(2, 4)):
                h = 7 + rng.integers(0, 3)
                e.caixa(x, 33, 54, x + 1.6, 38, 54 + h, ("tela_verm", "porticons", "roba2", "roba3")[rng.integers(4)])
                x += 1.8
            x += 1


def _aplic(e, s, alt, cristall=False):
    e.caixa(14, 32, 56, 18, 33, 62, "llauto")
    e.caixa(15.3, 32, 60, 16.7, 38, 61, "llauto")
    e.cilindre(16, 37, 61, 63, 2.2, "llauto", costats=6)
    if cristall:
        tronc(e, 16, 37, 63, 69, 2, 0.5, 6, "cristall")
    else:
        e.cilindre(16, 37, 63, 68, 1.0, "cera", costats=6)
        e.caixa(15.6, 36.6, 68, 16.4, 37.4, 71, "foc", dalt="foc")


def _torxa(e, s, alt):
    e.caixa(14.5, 32, 54, 17.5, 33, 60, "ferro")
    e.caixa(15.2, 32, 57, 16.8, 37, 58.5, "ferro")
    tronc(e, 16, 36.5, 52, 64, 1.0, 1.6, 6, "noguera")
    tronc(e, 16, 36.5, 64, 67, 2.4, 2.8, 6, "ferro")
    tronc(e, 16, 36.5, 67, 74, 2.6, 0.4, 6, "foc")
    tronc(e, 16, 36.5, 67, 71, 1.6, 0.4, 6, "foc", dalt="foc")


def _tapis(e, s, alt, mat="tela_verm"):
    top = alt - 14
    e.caixa(5, 32, top, 27, 35, top + 2, "or")
    for x in (5, 26):
        e.caixa(x - 1, 32, top - 1, x + 1, 35.5, top + 3, "or")
    e.caixa(8, 32, 30, 24, 33.5, top, mat, "plecs")
    _r(e, 8, 24, 30, 33, "or", to=1, y=33.5, biaix=0.7)
    p = e.nova_peca()                                                             # el rombe dels Guardians, d'or
    cx, cz = 16, (30 + top) / 2 + 4
    e.cara([(cx, 33.6, cz - 9), (cx + 6, 33.6, cz), (cx, 33.6, cz + 9), (cx - 6, 33.6, cz)], (0, 1, 0),
           color_de("or", "llis", "y"), p, 0.8)
    e.cara([(cx, 33.7, cz - 4), (cx + 3, 33.7, cz), (cx, 33.7, cz + 4), (cx - 3, 33.7, cz)], (0, 1, 0),
           color_de("cristall", "llis", "y", to_fix=1), e.nova_peca(), 0.9)
    serrell(e, (8, 33.5), (24, 33.5), 30, (0, 1, 0), "or", alt=3, pas=3)


def _escut(e, s, alt):
    for d in (-1, 1):                                                             # dues espases creuades
        p = e.nova_peca()
        e.cara([(16 - 9 * d, 33.2, 42), (16 - 8 * d, 33.2, 41), (16 + 9 * d, 33.2, 76), (16 + 8 * d, 33.2, 77)],
               (0, 1, 0), color_de("ferro", "llis", "y", to_fix=1), p, 0.6)
    e.cilindre(16, 32.5, 50, 54, 9, "tela_verm", costats=12)
    p = e.nova_peca()
    pts = [(16 + 9 * math.cos(a), 54 + 3.5, 58 + 9 * math.sin(a)) for a in np.linspace(0, 2 * math.pi, 14)[:-1]]
    e.cara([(x, 32 + 4.5, z) for x, _, z in pts], (0, 1, 0), color_de("tela_verm", "llis", "y"), p, 0.7)
    e.cara([(x, 32 + 4.6, z) for x, z in ((16, 51), (21, 58), (16, 65), (11, 58))], (0, 1, 0),
           color_de("or", "llis", "y"), e.nova_peca(), 0.8)


def _porta(e, s, alt):
    """La porta de sortida: l'obertura d'arc amb la fulla oberta cap a la sala."""
    pedra = s["mur"][0].startswith("pedra") or s["mur"][0] == "mao"
    marc = "pedra_clara" if pedra else "noguera"
    _r(e, 6, 26, 0, 62, marc, to=1 if pedra else 2, arc=True)
    _r(e, 8, 24, 0, 58, "ferro", to=4, arc=True, biaix=0.9)
    _r(e, 8, 24, 0, 14, "ferro", to=3, biaix=1.0)                                # el terra de fora, més clar
    # La fulla, oberta a 45° (les dues cares): així no surt per sota del vèrtex de la cel·la en cap vista.
    fx, fy = 8 + 10.6, 32 + 10.6
    e.cara([(8, 32, 0), (fx, fy, 0), (fx, fy, 50), (8, 32, 50)], (1, -1, 0), color_de("noguera", "taulons_v", "x"),
           e.nova_peca(), 0.4, True)
    e.caixa(fx - 2.6, fy - 3.4, 22, fx - 1.4, fy - 2.2, 25, "llauto")


def _xemeneia(e, s, alt):
    """Llar de foc: el marc de pedra (surt 4 a partir de z 2, perquè res no
    passi per sota del vèrtex de la cel·la), la boca fosca amb el foc, la
    lleixa i el cos de la xemeneia fins a dalt."""
    pedra = "pedra" if not s["mur"][0].startswith("pedra_fosca") else "pedra_fosca"
    _r(e, 3, 29, 0, 3, pedra, to=3, biaix=0.4)
    e.caixa(3, 32, 2.5, 29, 36, 36, pedra, "carreus", dalt=pedra)
    e.rect("y", 36, 1, 8, 24, 2.5, 26, "ferro", to=4, arc=True, biaix=0.8)
    e.caixa(9, 33, 2.5, 23, 35.5, 4, "ferro")
    e.caixa(10, 33.5, 4, 22, 35.5, 6, "noguera")                                   # els troncs
    for (x, h) in ((12, 12), (16, 16), (20, 11)):                                  # les flames
        tronc(e, x, 35, 5, 5 + h, 2.4, 0.3, 6, "foc")
    tronc(e, 16, 35.2, 5, 11, 3.6, 0.6, 6, "foc", dalt="foc")
    e.caixa(1, 31, 36, 31, 40, 39, "noguera")                                      # la lleixa
    e.caixa(7, 32, 39, 25, 36, alt - 2, pedra, "carreus", dalt=pedra)             # el cos de la xemeneia
    e.cilindre(6, 37.5, 39, 44, 1.2, "cera", costats=6)
    e.cilindre(26, 37.5, 39, 46, 1.2, "cera", costats=6)
    e.cilindre(16, 37.5, 39, 44, 2.6, "llauto", costats=8)


def _trofeu(e, s, alt):
    e.caixa(11, 32, 58, 21, 33.5, 70, "noguera")
    e.caixa(14, 33.5, 59, 18, 39, 66, "fusta")
    e.caixa(14.5, 38, 58, 17.5, 41, 62, "fusta")
    for d in (-1, 1):
        x = 16 + 3 * d
        e.caixa(min(x, x + 2 * d), 34, 66, max(x, x + 2 * d) + 0.1, 35.5, 76, "encenall")
        e.caixa(min(x + 2 * d, x + 6 * d), 34, 73, max(x + 2 * d, x + 6 * d), 35.5, 74.5, "encenall")
        e.caixa(min(x + 5 * d, x + 6.5 * d), 34, 74, max(x + 5 * d, x + 6.5 * d), 35.5, 79, "encenall")


def _llibres(e, s, alt, llavor=5):
    """Llibreria encastada: tota l'alçada, amb lleixes cada 18 i llibres de colors."""
    rng = np.random.default_rng(llavor)
    top = alt - 10
    _r(e, 0, 32, 0, 4, "noguera", to=4, biaix=0.4)                                 # el sòcol, a ras
    e.caixa(0, 32, 4, 32, 39, top, "noguera", dalt="noguera")
    colors = ("tela_verm", "porticons", "roba2", "roba3", "pergami", "vellut", "verdet")
    for z0 in range(5, int(top) - 14, 18):
        _r(e, 2, 30, z0, z0 + 15, "noguera", to=4, y=39, biaix=0.3)
        x = 2.2
        while x < 29.5:
            w = 1.4 + rng.random() * 1.2
            h = 9 + rng.integers(0, 6)
            if rng.random() < 0.08:
                x += w
                continue
            e.caixa(x, 34, z0, min(x + w, 29.8), 39.2, z0 + h, colors[rng.integers(len(colors))])
            x += w + 0.2
        e.caixa(0, 32, z0 + 15, 32, 39.6, z0 + 16.5, "noguera")


def _armes(e, s, alt):
    for z in (44, 74):
        e.caixa(4, 32, z, 28, 34, z + 3, "noguera")
    for k, x in enumerate((8, 13, 18, 23)):
        if k == 2:                                                                 # una llança
            e.caixa(x, 34, 30, x + 1, 35, 90, "noguera")
            tronc(e, x + 0.5, 34.5, 90, 97, 1.6, 0, 4, "ferro")
        else:                                                                      # espases
            e.caixa(x - 0.8, 34, 40, x + 0.8, 35, 72, "ferro")
            e.caixa(x - 3, 34, 70, x + 3, 35.5, 71.5, "llauto")
            e.caixa(x - 0.6, 34, 71.5, x + 0.6, 35.5, 77, "noguera")


def _mapa(e, s, alt):
    e.caixa(5, 32, 44, 27, 33, 74, "pergami", dalt="pergami")
    _r(e, 8, 16, 52, 62, "aigua", to=2, y=33, biaix=0.7)
    _r(e, 15, 23, 60, 70, "fulla", to=2, y=33, biaix=0.7)
    _r(e, 9, 24, 48, 49, "terra", to=3, y=33, biaix=0.8)
    for x, z in ((10, 66), (20, 54), (14, 50)):
        e.caixa(x, 33, z, x + 1.4, 35, z + 1.4, "tela_verm")


def _pissarra(e, s, alt):
    e.caixa(3, 32, 38, 29, 33.5, 72, "noguera")
    _r(e, 5, 27, 40, 70, "porticons", to=4, y=33.5, biaix=0.4)
    for (u0, u1, v) in ((7, 20, 64), (7, 24, 59), (7, 15, 54), (17, 25, 48)):
        _r(e, u0, u1, v, v + 0.8, "roba1", to=1, y=33.5, biaix=0.8)
    e.caixa(5, 32, 38, 27, 35, 39.5, "noguera")


def _cortina(e, s, alt):
    _r(e, 0, 32, 0, 3, "vellut", to=4, biaix=0.4)
    e.caixa(0, 32, 2.5, 32, 36, alt - 8, "vellut", "plecs")
    e.caixa(0, 32, alt - 12, 32, 37, alt - 6, "vellut", dalt="vellut")
    serrell(e, (0, 37), (32, 37), alt - 12, (0, 1, 0), "or", alt=4, pas=4)


def _reixa(e, s, alt):
    _r(e, 4, 28, 0, 66, "pedra_clara", to=1, arc=True)
    _r(e, 6, 26, 0, 62, "ferro", to=4, arc=True, biaix=0.9)
    for x in range(7, 26, 4):
        e.caixa(x, 33, 1.5, x + 1.2, 34.5, 60, "ferro")
    for z in (14, 30, 46):
        e.caixa(6, 33, z, 26, 34.6, z + 1.2, "ferro")


def _vitrall(e, s, alt):
    v0, v1 = 34, alt - 16
    _r(e, 7, 25, v0 - 2, v1 + 4, "pedra_clara", to=1, arc=True)
    _r(e, 9, 23, v0, v1, "ferro", to=3, arc=True, biaix=0.8)
    colors = ("vidre", "tela_verm", "roba3", "porticons", "roba2", "cristall")
    k = 0
    for v in range(int(v0) + 1, int(v1) - 10, 7):
        for u in (9.6, 16.4):
            _r(e, u, u + 6, v, v + 6, colors[k % len(colors)], to=1, biaix=1.0)
            k += 1


def _ninxol(e, s, alt):
    _r(e, 9, 23, 22, 50, "pedra_grisa", to=1, arc=True)
    _r(e, 11, 21, 24, 48, "ferro", to=4, arc=True, biaix=0.9)
    e.caixa(13.5, 30, 24, 18.5, 32.5, 28, "pedra_clara")                          # la calavera
    e.cilindre(20, 31, 24, 29, 0.9, "cera", costats=5)


# ---------------------------------------------------------------- INT-02: el toc de cada nació de l'est
def _fil(e, u0, v0, u1, v1, mat="tela", y=32.5, gruix=0.55, to=0):
    """Un fil (de teranyina o de corda) sobre la paret, de (u0, v0) a (u1, v1)."""
    du, dv = u1 - u0, v1 - v0
    l = math.hypot(du, dv) or 1
    nu, nv = -dv / l * gruix, du / l * gruix
    e.cara([(u0 - nu, y, v0 - nv), (u1 - nu, y, v1 - nv), (u1 + nu, y, v1 + nv), (u0 + nu, y, v0 + nv)], (0, 1, 0),
           color_de(mat, "llis", "y", to_fix=to), e.nova_peca(), 0.9)


def _teranyina(e, s, alt):
    """Nocturnsea: la teranyina del racó de dalt, la que ningú no treu."""
    cu, cv = 1.0, alt - 3
    radis = [(math.cos(a), math.sin(a)) for a in (math.radians(g) for g in (-6, -27, -48, -69, -88))]
    for (c, si) in radis:
        _fil(e, cu, cv, cu + 22 * c, cv + 22 * si)
    for r in (7, 13, 19):
        pts = [(cu + r * c, cv + r * si) for c, si in radis]
        for (a0, b0), (a1, b1) in zip(pts, pts[1:]):
            mu, mv = (a0 + a1) / 2, (b0 + b1) / 2
            ku = (mu - cu) * 0.92 + cu
            kv = (mv - cv) * 0.92 + cv
            _fil(e, a0, b0, ku, kv, gruix=0.45)
            _fil(e, ku, kv, a1, b1, gruix=0.45)
    e.caixa(11, 32.4, cv - 15, 13, 33.6, cv - 13, "ferro")                          # l'aranya
    _fil(e, 12, cv - 13, 12, cv - 9, gruix=0.3)


def _mirall(e, s, alt):
    """Mistarium: el mirall de cort penjat, de marc de plata, amb boira lila al fons."""
    _r(e, 8, 24, 36, 78, "pedra_clara", to=0, y=32.4, arc=True)                    # el marc de plata
    _r(e, 10, 22, 38, 75, "vidre", to=1, biaix=0.9, y=32.8, arc=True)
    _r(e, 10, 22, 52, 56, "boira_lila", to=1, biaix=0.8, y=33.0)                  # la boira que s'hi veu
    _r(e, 11, 14, 42, 70, "vidre", to=0, biaix=1.0, y=33.1)                         # el reflex
    tronc(e, 16, 33.5, 78, 82, 2, 0, 4, "boira_lila")                               # la gemma de dalt


def _mascares(e, s, alt):
    """Gongorguma: les dues màscares del teatre (Qethatos), la que riu i la que plora."""
    for (cu, mat, riu) in ((11, "roba3", True), (21, "pedra_clara", False)):
        cv = 58
        pts = [(cu + 5 * math.cos(a), cv + 6.5 * math.sin(a)) for a in np.linspace(0, 2 * math.pi, 14)[:-1]]
        e.cara([(u, 33.2 if riu else 33.6, v) for u, v in pts], (0, 1, 0), color_de(mat, "llis", "y"), e.nova_peca(), 0.8)
        y = 33.8 if riu else 34.2
        for du in (-2, 2):
            _r(e, cu + du - 1, cu + du + 1, cv + 1, cv + 3, "ferro", to=3, y=y, biaix=1.0)
        boca = (cv - 4, cv - 2) if riu else (cv - 2, cv - 4)
        _fil(e, cu - 2.5, boca[1], cu, boca[0], "ferro", y=y, gruix=0.5)
        _fil(e, cu, boca[0], cu + 2.5, boca[1], "ferro", y=y, gruix=0.5)
    _fil(e, 7, 66, 25, 66, "tela_verm", y=33.0, gruix=0.7)                          # la cinta que les penja
    _fil(e, 16, 66, 16, 70, "tela_verm", y=33.0, gruix=0.7)


def _cordes(e, s, alt):
    """Choubar: cordes de nòlit enrotllades penjant de dues clavilles."""
    for cu in (10, 22):
        e.caixa(cu - 1, 32, 70, cu + 1, 36, 72, "noguera")                           # la clavilla
        for k, r in enumerate((7.5, 6, 4.5)):
            cv = 62 - k * 0.6
            pts = [(cu + r * math.cos(a), cv + r * 1.25 * math.sin(a)) for a in np.linspace(0, 2 * math.pi, 15)]
            for (u0, v0), (u1, v1) in zip(pts, pts[1:]):
                _fil(e, u0, v0, u1, v1, "encenall", y=32.6 + k * 0.4, gruix=0.9, to=1 + k)
    _fil(e, 22, 54, 24, 40, "encenall", y=33.0, gruix=0.9, to=2)                    # la punta que penja


DECOS = {
    None: lambda e, s, alt: None,
    "finestra": _finestra,
    "finestra_alta": lambda e, s, alt: _finestra(e, s, alt, alta=True),
    "quadre": _quadre,
    "prestatge": _prestatge,
    "aplic": _aplic,
    "aplic_cristall": lambda e, s, alt: _aplic(e, s, alt, cristall=True),
    "torxa": _torxa,
    "tapis": _tapis,
    "tapis_blau": lambda e, s, alt: _tapis(e, s, alt, "roba2"),
    "escut": _escut,
    "porta": _porta,
    "xemeneia": _xemeneia,
    "trofeu": _trofeu,
    "llibres": _llibres,
    "armes": _armes,
    "mapa": _mapa,
    "pissarra": _pissarra,
    "cortina": _cortina,
    "reixa": _reixa,
    "vitrall": _vitrall,
    "ninxol": _ninxol,
    "teranyina": _teranyina,
    "mirall": _mirall,
    "mascares": _mascares,
    "cordes": _cordes,
}


def paret(estil, deco=None, alt=ALT):
    s = ESTILS[estil]
    e = Escena(1, 1)
    _cos(e, s, alt)
    DECOS[deco](e, s, alt)
    return e


def cantonada(estil, alt=ALT):
    """El pilar de la cantonada (cel·la (0, 0)): tapa la juntura dels dos cantells."""
    s = ESTILS[estil]
    e = Escena(1, 1)
    mat = s.get("entramat") or s["cap"]
    e.caixa(32 - G - 1, 32 - G - 1, 0, 32, 32, alt + 1, mat, dalt=s["cap"])
    return e


# Quines decoracions té cada estil (les que tenen sentit en aquella paret).
DECOS_ESTIL = {
    "fusta": [None, "finestra", "quadre", "prestatge", "aplic", "porta", "xemeneia", "trofeu", "armes", "mapa",
              "cordes"],
    "estuc": [None, "finestra", "finestra_alta", "quadre", "prestatge", "aplic", "porta", "xemeneia", "pissarra", "mapa",
              "llibres"],
    "estuc_blau": [None, "finestra", "quadre", "prestatge", "aplic", "porta", "xemeneia", "llibres"],
    "noble": [None, "finestra", "finestra_alta", "quadre", "aplic_cristall", "tapis", "porta", "xemeneia", "llibres",
              "cortina"],
    "noble_blau": [None, "finestra", "finestra_alta", "quadre", "aplic_cristall", "tapis_blau", "porta", "xemeneia",
                   "llibres", "mirall"],
    "noble_verd": [None, "finestra", "finestra_alta", "quadre", "aplic_cristall", "porta", "xemeneia", "llibres",
                   "pissarra", "mapa"],
    "pedra": [None, "finestra", "finestra_alta", "torxa", "tapis", "tapis_blau", "escut", "porta", "xemeneia", "armes",
              "mapa", "reixa", "aplic_cristall"],
    "pedra_calida": [None, "finestra", "finestra_alta", "torxa", "escut", "porta", "xemeneia", "armes", "prestatge",
                     "reixa", "tapis", "mascares"],
    "pedra_fosca": [None, "finestra_alta", "vitrall", "torxa", "porta", "ninxol", "aplic", "reixa", "teranyina"],
    "rajola": [None, "finestra", "aplic", "porta", "prestatge"],
    "mao": [None, "finestra", "torxa", "porta", "xemeneia", "armes", "prestatge", "cordes"],   # cordes: la ferreria de Choubar
}


# Els estils que també tenen paret alta (les sales grans dels monuments).
ESTILS_ALTS = {"pedra", "pedra_fosca", "pedra_calida", "noble", "noble_blau", "noble_verd", "estuc"}


def ids_parets():
    """{id: (estil, deco, alt)} de totes les peces de paret (alt: normal o de sala gran)."""
    out = {}
    for estil, decos in DECOS_ESTIL.items():
        for alt, sufix in ((ALT, ""), (ALT_SALA, "_alta")) if estil in ESTILS_ALTS else ((ALT, ""),):
            for d in decos:
                if d == "finestra_alta" and alt == ALT:
                    continue
                out[f"paret_{estil}{sufix}_{d or 'llisa'}"] = (estil, d, alt)
            out[f"paret_{estil}{sufix}_cantonada"] = (estil, "cantonada", alt)
    return out


PARETS = {pid: (lambda e=e, d=d, a=a: cantonada(e, a) if d == "cantonada" else paret(e, d, a))
          for pid, (e, d, a) in ids_parets().items()}
