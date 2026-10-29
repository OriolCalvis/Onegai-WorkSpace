#!/usr/bin/env python3
"""ISO (INT-01) — Els mobles de tot arreu: taules, cadires, bancs, llits,
armaris, prestatgeries, cofres, llums, plantes, botes i caixes, i les
escales i l'estora de la porta. La cara de davant és +Y (o0)."""
import math

from mobles_base import (Escena, Registre, amb_veins, ampolla, anell, cilindre_x, coixi, color_de, espelma, gerra,
                         llibre_obert, llibres_drets, mata, poligon_regular, potes, rng_de, tronc)
import sales

R = Registre()


# ---------------------------------------------------------------- taules i seients
def mesa(mat="noguera", alt=16, coses=None):
    e = Escena(1, 1)
    e.caixa(4, 4, alt - 2.5, 28, 28, alt, mat, "taulons", dalt=mat, tex_dalt="taulons")
    e.caixa(6, 6, alt - 5, 26, 26, alt - 2.5, mat)
    potes(e, 6, 6, 26, 26, alt - 5, 2.5, mat)
    for c in coses or ():
        c(e, alt)
    return e


def _gerres(e, alt):
    gerra(e, 11, 12, alt)
    gerra(e, 20, 18, alt)
    e.cilindre(16, 22, alt, alt + 1, 4, "pedra_clara", costats=10)            # el plat
    e.caixa(14, 20, alt + 1, 18, 23, alt + 3.5, "encenall")                   # pa


def _espelma(e, alt):
    e.cilindre(16, 16, alt, alt + 1, 2.4, "llauto", costats=8)
    espelma(e, 16, 16, alt + 1)


def _llibre(e, alt):
    llibre_obert(e, 13, 16, alt)
    e.cilindre(22, 12, alt, alt + 1, 2, "llauto", costats=8)
    espelma(e, 22, 12, alt + 1, 4)


def mesa_redonda(mat="noguera", alt=15, coses=None):
    e = Escena(1, 1)
    e.cilindre(16, 16, alt - 2, alt, 12, mat, costats=16, dalt=mat)
    e.cilindre(16, 16, 2, alt - 2, 2, mat, costats=8)
    e.caixa(9, 15, 0, 23, 17, 2, mat)
    e.caixa(15, 9, 0, 17, 23, 2, mat)
    for c in coses or ():
        c(e, alt)
    return e


def taburete(mat="fusta"):
    e = Escena(1, 1)
    for a in (0.3, 2.4, 4.5):
        x, y = 16 + 3.6 * math.cos(a), 16 + 3.6 * math.sin(a)
        e.caixa(x - 0.9, y - 0.9, 0, x + 0.9, y + 0.9, 9, "noguera")
    e.cilindre(16, 16, 9, 11, 5.5, mat, costats=12, dalt=mat)
    return e


def silla(mat="noguera", coixi_mat=None, alt_resp=26):
    """Cadira de cara a +Y (el respatller a -Y)."""
    e = Escena(1, 1)
    e.caixa(9, 10, 9, 23, 23, 11, mat, dalt=mat)
    potes(e, 9, 10, 23, 23, 9, 2, mat)
    e.caixa(9, 9, 11, 11, 11, alt_resp, mat)
    e.caixa(21, 9, 11, 23, 11, alt_resp, mat)
    e.caixa(9, 9.5, alt_resp - 4, 23, 11, alt_resp, mat, dalt=mat)
    e.caixa(11, 9.8, 15, 21, 10.8, 17, mat)
    if coixi_mat:
        e.caixa(10, 12, 11, 22, 22, 13, coixi_mat, dalt=coixi_mat)
        e.caixa(11, 10.5, 13, 21, 11.5, alt_resp - 5, coixi_mat)
    return e


def banco(mat="fusta", llarg=True):
    """Banc sense respatller, al llarg de X (per a les taules llargues)."""
    e = Escena(1, 1)
    x0, x1 = (1, 31) if llarg else (6, 26)
    e.caixa(x0, 11, 9, x1, 21, 11, mat, "taulons", dalt=mat, tex_dalt="taulons")
    for x in (x0 + 3, x1 - 5):
        e.caixa(x, 12, 0, x + 2, 20, 9, "noguera")
    return e


def sofa(tela="vellut"):
    """Sofà de dues places de cara a +Y, de fusta fosca i entapissat."""
    e = Escena(1, 1)
    e.caixa(2, 8, 2, 30, 26, 9, "noguera", dalt=tela)
    potes(e, 3, 9, 29, 25, 2, 2, "noguera")
    coixi(e, 4, 11, 28, 25, 9, 3, tela)
    e.caixa(2, 6, 9, 30, 10, 26, tela, "plecs")
    e.caixa(2, 5.5, 24, 30, 10.5, 27, "noguera", dalt="noguera")
    for x in (2, 27):
        e.caixa(x, 8, 9, x + 3, 26, 17, tela, dalt="noguera")
    return e


def mesa_larga(tram, estovalles=False):
    """Taula llarga en trams (ini, mig, fi) al llarg de X: de fusta (taverna,
    quarter) o amb estovalles i espelmes (banquet)."""
    e = Escena(1, 1)
    x0 = 3 if tram == "ini" else 0
    x1 = 29 if tram == "fi" else 32
    if estovalles:
        e.caixa(x0, 6, 14, x1, 26, 16.5, "noguera", dalt="tela")
        e.caixa(x0, 5.5, 10, x1, 6.5, 16.5, "tela")
        e.caixa(x0, 25.5, 10, x1, 26.5, 16.5, "tela")
    else:
        e.caixa(x0, 6, 13.5, x1, 26, 16.5, "fusta", "taulons", dalt="fusta", tex_dalt="taulons")
        e.caixa(x0 + (2 if tram == "ini" else 0), 9, 11, x1 - (2 if tram == "fi" else 0), 23, 13.5, "noguera")
    if tram != "mig":
        x = x0 + 2 if tram == "ini" else x1 - 4
        for y in (8, 22):
            e.caixa(x, y, 0, x + 2, y + 2, 14, "noguera")
        if estovalles:
            e.caixa(x0 if tram == "ini" else x1 - 0.8, 6, 10, x0 + 0.8 if tram == "ini" else x1, 26, 16.5, "tela")
        else:
            e.caixa(x, 9, 3, x + 2, 23, 5, "noguera")
    for (x, y) in ((9, 11), (23, 20)):
        e.cilindre(x, y, 16.5, 17.5, 3.4, "pedra_clara", costats=10)
    if tram == "mig":
        e.cilindre(16, 16, 16.5, 17.5, 3, "llauto", costats=8)
        for x in (13, 16, 19):
            espelma(e, x, 16, 17.5, 6)
    else:
        gerra(e, 16, 13, 16.5)
    return e


def escritorio(mat="noguera"):
    """Escriptori amb calaixos, papers, el tinter amb la ploma i una espelma."""
    e = Escena(1, 1)
    e.caixa(3, 6, 14, 29, 24, 16.5, mat, dalt=mat)
    e.caixa(3, 7, 0, 11, 23, 14, mat, "plafons")
    e.caixa(25, 7, 0, 29, 23, 14, mat)
    for z in (4, 9):
        e.caixa(5, 23, z, 9, 23.6, z + 1, "llauto")
    e.caixa(12, 9, 16.5, 21, 18, 17, "pergami", dalt="pergami")
    e.caixa(14, 11, 17, 22, 19, 17.4, "paper_cel", dalt="paper_cel")
    e.cilindre(24, 11, 16.5, 19, 1.4, "ferro", costats=6)
    e.caixa(24.2, 10.6, 19, 24.8, 11.2, 25, "roba1")
    espelma(e, 6, 10, 16.5, 5)
    return e


def atril(llibre="tela_verm", base="noguera"):
    """Faristol amb un llibre obert."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 3, 6, 5, 6, base)
    e.caixa(15, 15, 3, 17, 17, 26, base)
    p = e.nova_peca()
    e.cara([(8, 20, 26), (24, 20, 26), (24, 12, 32), (8, 12, 32)], (0, 6, 8), color_de(base, "llis", "y"), p)
    p = e.nova_peca()
    e.cara([(9, 19.5, 27), (23, 19.5, 27), (23, 12.5, 32.3), (9, 12.5, 32.3)], (0, 6, 8),
           color_de("pergami", "llis", "y", to_fix=0), p, 0.4)
    e.cara([(15.6, 19.6, 27), (16.4, 19.6, 27), (16.4, 12.6, 32.4), (15.6, 12.6, 32.4)], (0, 6, 8),
           color_de(llibre, "llis", "y"), e.nova_peca(), 0.6)
    return e


# ---------------------------------------------------------------- llits i armaris
def cama(tram, manta="roba2", coixi_mat="roba1", fusta="noguera"):
    """Llit en dos trams al llarg de Y: cap (el capçal, a -Y) i peus."""
    e = Escena(1, 1)
    if tram == "cap":
        e.caixa(4, 2, 0, 28, 5, 28, fusta, "plafons", dalt=fusta)
        for x in (3.5, 25.5):
            e.caixa(x, 1.5, 0, x + 3, 5.5, 31, fusta, dalt=fusta)
            tronc(e, x + 1.5, 3.5, 31, 33, 1.6, 0.6, 6, "llauto")
        e.caixa(5, 5, 4, 27, 32, 9, fusta)
        e.caixa(5.5, 5, 9, 26.5, 32, 13, "roba1", dalt="roba1")
        coixi(e, 8, 6.5, 24, 13, 13, 2.5, coixi_mat)
        e.caixa(5, 16, 9, 27, 32, 14, manta, dalt=manta)
        e.caixa(5, 15, 13, 27, 17, 14.6, "roba1", dalt="roba1")
    else:
        e.caixa(5, 0, 4, 27, 27, 9, fusta)
        e.caixa(5, 0, 9, 27, 27, 14, manta, dalt=manta)
        e.caixa(4.5, 26, 4, 27.5, 27.5, 14, manta)
        e.caixa(4, 27, 0, 28, 30, 18, fusta, "plafons", dalt=fusta)
        for x in (3.5, 25.5):
            e.caixa(x, 26.5, 0, x + 3, 30.5, 20, fusta, dalt=fusta)
        for x in (5, 26):
            e.caixa(x, 1, 0, x + 1.5, 3, 4, fusta)
    return e


def litera(tram):
    """Llitera del quarter (dos pisos), en dos trams: cap i peus."""
    e = Escena(1, 1)
    for x in (4, 26):
        e.caixa(x, 0 if tram == "peus" else 3, 0, x + 2, 32 if tram == "cap" else 29, 2, "noguera")
    pals = (3,) if tram == "cap" else (27,)
    for y in pals:
        for x in (4, 26):
            e.caixa(x, y, 0, x + 2, y + 2, 46, "noguera", dalt="noguera")
    for z in (8, 32):
        e.caixa(6, 3 if tram == "cap" else 0, z, 26, 32 if tram == "cap" else 29, z + 3, "noguera")
        e.caixa(6.5, 3 if tram == "cap" else 0, z + 3, 25.5, 32 if tram == "cap" else 29, z + 6, "tela", dalt="tela")
        if tram == "cap":
            coixi(e, 9, 4, 23, 9, z + 6, 2, "roba1")
        else:
            e.caixa(6.5, 10, z + 6, 25.5, 29, z + 7, "porticons", dalt="porticons")
    if tram == "cap":
        for z in range(12, 32, 6):                                          # l'escaleta
            e.caixa(26, 10, z, 28, 18, z + 1.2, "noguera")
    return e


def mesilla():
    e = Escena(1, 1)
    e.caixa(9, 9, 0, 23, 22, 15, "noguera", "plafons", dalt="noguera")
    e.caixa(14.5, 21.6, 9, 17.5, 22.4, 10.5, "llauto")
    e.cilindre(13, 14, 15, 16, 2.4, "llauto", costats=8)
    espelma(e, 13, 14, 16, 4)
    return e


def armario(mat="noguera", or_=False):
    """Armari alt de dues portes (de cara a +Y)."""
    e = Escena(1, 1)
    e.caixa(3, 6, 0, 29, 20, 3, mat, dalt=mat)
    e.caixa(4, 7, 3, 28, 19, 54, mat, "plafons", dalt=mat)
    e.caixa(2.5, 5.5, 54, 29.5, 20.5, 58, mat, dalt=mat)
    e.caixa(15.6, 19, 3, 16.4, 19.4, 54, mat)
    for x in (13.5, 17.5):
        e.caixa(x, 19, 28, x + 1, 20, 32, "or" if or_ else "llauto")
    if or_:
        e.caixa(3, 5, 58, 29, 21, 60, "or", dalt="or")
        tronc(e, 16, 13, 60, 65, 3, 0, 4, "or")
    return e


def baul():
    """Bagul de viatge (al peu dels llits)."""
    e = Escena(1, 1)
    e.caixa(5, 9, 0, 27, 23, 11, "fusta", "taulons", dalt="noguera")
    for x in (7, 24):
        e.caixa(x, 8.6, 0, x + 1.5, 23.4, 11.5, "ferro")
    for y0 in (8.6, 22.6):
        e.caixa(5, y0, 9, 27, y0 + 0.8, 11.5, "ferro")
    e.caixa(15, 22.8, 6, 17, 23.8, 9, "llauto")
    return e


def palangana():
    """Rentamans: un trípode amb la palangana i la gerra."""
    e = Escena(1, 1)
    for a in (0.5, 2.6, 4.7):
        x, y = 16 + 5 * math.cos(a), 16 + 5 * math.sin(a)
        e.caixa(x - 0.8, y - 0.8, 0, x + 0.8, y + 0.8, 17, "noguera")
    tronc(e, 16, 16, 17, 21, 4, 7, 12, "pedra_clara", dalt="aigua")
    e.cilindre(21, 19, 21, 22, 1.8, "noguera", costats=6)
    tronc(e, 21, 19, 22, 27, 2.3, 1.4, 8, "estuc_blau")
    return e


def biombo(tela="tela", fusta="noguera"):
    """Biombo de tres fulles en ziga-zaga."""
    e = Escena(1, 1)
    pts = [(3, 14), (11, 19), (20, 14), (29, 19)]
    for (xa, ya), (xb, yb) in zip(pts, pts[1:]):
        n = (-(yb - ya), xb - xa, 0)
        e.cara([(xa, ya, 2), (xb, yb, 2), (xb, yb, 40), (xa, ya, 40)], n, color_de(tela, "plecs", "y"), e.nova_peca(),
               0.0, True)
    for x, y in pts:
        e.caixa(x - 1, y - 1, 0, x + 1, y + 1, 42, fusta, dalt=fusta)
    return e


def perchero():
    """Penjador de peu amb una capa i un barret."""
    e = Escena(1, 1)
    for a in (0.0, 2.1, 4.2):
        x, y = 16 + 5 * math.cos(a), 16 + 5 * math.sin(a)
        e.caixa(min(16, x) - 0.6, min(16, y) - 0.6, 0, max(16, x) + 0.6, max(16, y) + 0.6, 2, "noguera")
    e.cilindre(16, 16, 0, 48, 1.2, "noguera", costats=6)
    tronc(e, 16, 16, 48, 51, 2, 0.5, 6, "llauto")
    e.cara([(12, 18, 20), (20, 18, 20), (19, 17, 44), (13, 17, 44)], (0, 1, 0.1), color_de("porticons", "plecs", "y"),
           e.nova_peca(), 0.4)
    tronc(e, 16, 16, 44, 46, 5, 5, 10, "noguera")
    tronc(e, 16, 16, 46, 50, 3, 2.5, 10, "noguera")
    return e


def espejo(marc="or"):
    """Mirall de peu (psiquis)."""
    e = Escena(1, 1)
    for x in (7, 24):
        e.caixa(x, 14, 0, x + 2, 18, 3, "noguera")
        e.caixa(x + 0.5, 15, 3, x + 1.5, 17, 40, "noguera")
    e.caixa(9, 15, 6, 23, 17, 46, marc, dalt=marc)
    e.rect("y", 17, 1, 10.5, 21.5, 8, 44, "vidre", to=1, biaix=0.4)
    e.rect("y", 17.1, 1, 11.5, 15, 12, 40, "vidre", to=0, biaix=0.6)
    return e


def estanteria(llavor=21):
    """Prestatgeria de llibres contra la paret (de cara a +Y)."""
    e = Escena(1, 1)
    rng = rng_de(llavor)
    e.caixa(2, 3, 0, 30, 5, 64, "noguera")
    e.rect("y", 5, 1, 2, 30, 0, 64, "noguera", to=4, biaix=0.2)
    for x in (2, 28):
        e.caixa(x, 3, 0, x + 2, 15, 64, "noguera", dalt="noguera")
    e.caixa(2, 3, 60, 30, 15, 64, "noguera", dalt="noguera")
    for z in (2, 21, 40):
        e.caixa(4, 5, z, 28, 15, z + 1.5, "noguera", dalt="noguera")
        llibres_drets(e, 4.5, 27.5, 6, 14, z + 1.5, rng)
    return e


def estanteria_libros(llavor=31):
    """Prestatgeria de biblioteca de doble cara (al mig de la sala)."""
    e = Escena(1, 1)
    rng = rng_de(llavor)
    e.caixa(2, 14.5, 0, 30, 17.5, 66, "noguera")
    for x in (2, 28):
        e.caixa(x, 6, 0, x + 2, 26, 66, "noguera", dalt="noguera")
    e.caixa(1.5, 5.5, 64, 30.5, 26.5, 68, "noguera", dalt="noguera")
    for z in (2, 22, 42):
        e.caixa(4, 6, z, 28, 26, z + 1.5, "noguera", dalt="noguera")
        llibres_drets(e, 4.5, 27.5, 17.5, 25.5, z + 1.5, rng, alt=(9, 15))
        llibres_drets(e, 4.5, 27.5, 6.5, 14.5, z + 1.5, rng, alt=(9, 15))
    return e


def reloj_pie():
    """Rellotge de peu amb l'esfera de llautó i el pèndol."""
    e = Escena(1, 1)
    e.caixa(9, 11, 0, 23, 21, 6, "noguera", dalt="noguera")
    e.caixa(10, 12, 6, 22, 20, 46, "noguera", "plafons", dalt="noguera")
    e.rect("y", 20, 1, 12.5, 19.5, 14, 36, "vidre", to=3, biaix=0.4)
    e.caixa(15.5, 20, 16, 16.5, 20.5, 32, "llauto")
    e.cilindre(16, 20.5, 16, 17, 2.4, "llauto", costats=8)
    e.caixa(9, 11, 46, 23, 21, 60, "noguera", dalt="noguera")
    e.cilindre(16, 21, 47, 59, 5, "noguera", costats=12)
    p = e.nova_peca()
    pts = [(16 + 4.4 * math.cos(a), 53 + 4.4 * math.sin(a)) for a in [k * math.pi / 6 for k in range(12)]]
    e.cara([(x, 21.5, z) for x, z in pts], (0, 1, 0), color_de("pedra_clara", "llis", "y", to_fix=0), p, 0.5)
    e.caixa(15.6, 21.6, 53, 16.4, 22, 56.5, "ferro")
    e.caixa(16, 21.6, 52.6, 18.5, 22, 53.4, "ferro")
    tronc(e, 16, 16, 60, 64, 6, 2, 4, "noguera", gir=math.pi / 4)
    return e


def globo():
    """Globus terraqüi sobre peu de fusta."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 3, 6, 5, 8, "noguera")
    e.cilindre(16, 16, 3, 16, 1.2, "noguera", costats=6)
    anell(e, 16, 16, 16, 17, 9, 8, 16, "llauto")
    for k, (r, z0, z1) in enumerate(((4, 17, 19), (6.5, 19, 21), (7.5, 21, 24), (7.5, 24, 27), (6.5, 27, 29),
                                     (4, 29, 31))):
        e.cilindre(16, 16, z0, z1, r, "aigua" if k % 2 == 0 else "fulla", costats=12)
    anell(e, 16, 16, 30, 31, 9, 8, 16, "llauto")
    return e


# ---------------------------------------------------------------- guardar
def barril(mat="fusta"):
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 9, 7, 8, 12, mat, "cercles")
    tronc(e, 16, 16, 9, 18, 8, 7, 12, mat, "cercles", dalt="noguera")
    return e


def barril_grifo():
    """Bota de vi ajaguda sobre un cavallet, amb l'aixeta de llautó cap a +Y."""
    e = Escena(1, 1)
    for x in (7, 23):
        e.caixa(x, 10, 0, x + 2.5, 22, 7, "noguera")
    cilindre_x(e, 5, 27, 16, 15, 8.5, "fusta", tapa="noguera")
    e.caixa(15, 23.5, 12, 17, 26, 14, "llauto")
    e.caixa(15.5, 25, 9.5, 16.5, 26, 12, "llauto")
    return e


def barriles_pila():
    """Tres botes ajagudes apilades contra la paret."""
    e = Escena(1, 1)
    for y in (9, 23):
        cilindre_x(e, 3, 29, y, 6.5, 6.5, "fusta", tapa="noguera")
    cilindre_x(e, 3, 29, 16, 18, 6.5, "fusta", tapa="noguera")
    return e


def cofre(mat="noguera", tresor=False):
    e = Escena(1, 1)
    e.caixa(7, 10, 0, 25, 22, 10, mat, "taulons")
    p = e.nova_peca()
    pts = [(10 + 6 * math.cos(a), 10 + 2.5 * math.sin(a)) for a in [k * math.pi / 6 for k in range(7)]]
    for k in range(len(pts) - 1):
        (ya, za), (yb, zb) = pts[k], pts[k + 1]
        n = (0, ((ya + yb) / 2 - 10) / 6, ((za + zb) / 2 - 10) / 2.5)
        e.cara([(7, ya + 6, za), (25, ya + 6, za), (25, yb + 6, zb), (7, yb + 6, zb)], n, color_de(mat, "llis", "y"), p)
    for x in (7, 25):
        s = 1 if x == 25 else -1
        e.cara([(x, y + 6, z) for y, z in pts], (s, 0, 0), color_de(mat, "llis", "x"), p)
    for x in (9, 22):
        e.caixa(x, 9.6, 0, x + 1.5, 22.4, 10.5, "ferro")
    e.caixa(15, 21.6, 6, 17, 22.6, 10, "llauto")
    if tresor:
        e.caixa(9, 13, 10, 23, 19, 12, "or", dalt="or")
    return e


def cajas():
    """Dues caixes de fusta, la petita damunt."""
    e = Escena(1, 1)
    e.caixa(5, 7, 0, 25, 25, 14, "fusta", "taulons", dalt="fusta", tex_dalt="taulons")
    for x in (5, 23.5):
        e.caixa(x, 6.6, 0, x + 1.5, 25.4, 14.4, "noguera")
    e.caixa(9, 10, 14, 22, 21, 23, "fusta", "taulons", dalt="fusta", tex_dalt="taulons")
    e.caixa(9, 9.6, 21.5, 22, 21.4, 23.4, "noguera")
    return e


def sacos(mat="encenall"):
    """Tres sacs de gra lligats."""
    e = Escena(1, 1)
    for (x, y, z, r) in ((10, 12, 0, 6), (21, 13, 0, 6), (15, 21, 0, 6.5)):
        tronc(e, x, y, z, z + 8, r, r + 0.5, 10, mat)
        tronc(e, x, y, z + 8, z + 13, r + 0.5, 2, 10, mat)
        tronc(e, x, y, z + 13, z + 15, 1.6, 2.4, 8, "terra")
    return e


def cesta(fruita="fruit_vermell"):
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 7, 6, 8, 12, "encenall", "taulons_v")
    for (dx, dy) in ((-3, -2), (2, -3), (0, 2), (3, 1), (-2, 3)):
        tronc(e, 16 + dx, 16 + dy, 7, 10, 2.2, 1.5, 6, fruita)
    return e


def cubo():
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 9, 4.5, 5.5, 10, "fusta", "taulons_v", dalt="aigua")
    for z in (2, 7):
        tronc(e, 16, 16, z, z + 1, 4.8 + z * 0.1, 4.9 + z * 0.1, 10, "ferro")
    return e


# ---------------------------------------------------------------- llum i plantes
def candelabro(alt=44, mat="llauto"):
    """Canelobre de peu amb tres espelmes."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 3, 6, 4, 8, mat, gir=math.pi / 8)
    e.cilindre(16, 16, 3, alt - 6, 1.1, mat, costats=6)
    e.caixa(9, 15.4, alt - 8, 23, 16.6, alt - 6.5, mat)
    for x in (9.5, 16, 22.5):
        e.cilindre(x, 16, alt - 6.5, alt - 5, 1.8, mat, costats=6)
        espelma(e, x, 16, alt - 5, 5 if x != 16 else 7)
    return e


def lampara_cristal():
    """Llum de peu tecnomàgic: un cristall que brilla dins d'un anell de llautó."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 3, 6, 5, 8, "llauto")
    e.cilindre(16, 16, 3, 36, 1, "llauto", costats=6)
    tronc(e, 16, 16, 36, 38, 2, 3.5, 8, "llauto")
    tronc(e, 16, 16, 38, 43, 0.5, 3.4, 4, "cristall")
    tronc(e, 16, 16, 43, 49, 3.4, 0, 4, "cristall")
    anell(e, 16, 16, 44, 45, 6.5, 5.5, 16, "llauto")
    return e


def brasero(foc="foc"):
    """Braser de ferro sobre tres potes."""
    e = Escena(1, 1)
    for a in (0.4, 2.5, 4.6):
        x, y = 16 + 6 * math.cos(a), 16 + 6 * math.sin(a)
        e.caixa(x - 0.8, y - 0.8, 0, x + 0.8, y + 0.8, 12, "ferro")
    tronc(e, 16, 16, 10, 16, 5, 9, 10, "ferro", dalt="ferro")
    for (dx, h) in ((-3, 7), (0, 10), (3, 6)):
        tronc(e, 16 + dx, 16, 16, 16 + h, 2.6, 0.3, 6, foc)
    return e


def maceta(llavor=3, test="terra"):
    """Planta en test (la mata de la vegetació, sobre un test de terrissa)."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 9, 5, 6.5, 10, test, dalt="terra")
    e.sprite((16, 16, 9), mata(8, 9, flors=("fulla",), llavor=llavor))
    return e


def columna_sala(alt=sales.ALT_SALA, mat="pedra_clara"):
    e = Escena(1, 1)
    e.caixa(7, 7, 0, 25, 25, 4, mat, dalt=mat)
    e.caixa(8, 8, 4, 24, 24, 7, mat, dalt=mat)
    e.cilindre(16, 16, 7, alt - 9, 6, mat, "taulons_v", costats=12)
    tronc(e, 16, 16, alt - 9, alt - 5, 6, 8, 12, mat)
    e.caixa(7, 7, alt - 5, 25, 25, alt, mat, dalt=mat)
    return e


# ---------------------------------------------------------------- portes i escales
def felpudo():
    """L'estora de la porta de sortida (l'objecte de la sortida, a terra)."""
    e = Escena(1, 1)
    e.caixa(6, 5, 0, 26, 27, 0.8, "tela_verm", "ratlles", dalt="tela_verm", tex_dalt="ratlles")
    return e


def escalera_sube(tram, fusta="fusta"):
    """Escala que puja cap a la paret del fons (-Y): base (la cel·la on es
    pitja: els vuit primers graons) i alto (la de la paret: els altres vuit i
    l'arc fosc del replà)."""
    e = Escena(1, 1)
    z0 = 0 if tram == "base" else 32
    for k in range(8):
        e.caixa(5, 32 - (k + 1) * 4, 0, 27, 32 - k * 4, z0 + (k + 1) * 4, fusta, "taulons", dalt=fusta)
        e.caixa(5, 32 - k * 4 - 0.8, z0 + (k + 1) * 4 - 0.8, 27, 32 - k * 4, z0 + (k + 1) * 4, "noguera")
    for x in (3, 27):                                                   # les baranes
        for k in range(0, 8, 3):
            y = 30 - k * 4
            e.caixa(x, y - 1, z0 + (k + 1) * 4, x + 2, y + 1, z0 + (k + 1) * 4 + 14, "noguera")
        p = e.nova_peca()
        e.cara([(x, 32, z0 + 14), (x + 2, 32, z0 + 14), (x + 2, 0, z0 + 46), (x, 0, z0 + 46)], (0, 1, 1),
               color_de("noguera", "llis", "y"), p, 0.3)
        e.cara([(x + 2, 32, z0 + 11), (x + 2, 0, z0 + 43), (x + 2, 0, z0 + 46), (x + 2, 32, z0 + 14)], (1, 0, 0),
               color_de("noguera", "llis", "x"), p, 0.3)
    if tram == "alto":
        e.rect("y", 0.4, 1, 3, 29, 58, 104, "noguera", to=2, arc=True, biaix=0.2)
        e.rect("y", 0.6, 1, 5, 27, 62, 100, "ferro", to=4, arc=True, biaix=0.4)
    return e


def escalera_baja(tram):
    """L'escala que baixa (la boca al terra amb les baranes): alto, la cel·la on
    es pitja (els primers graons), i fondo, la de la paret (la fosca). Els
    graons es pinten al pla del terra, cada cop més foscos (res surt per sota
    del vèrtex de la cel·la)."""
    e = Escena(1, 1)
    p = e.nova_peca()
    if tram == "alto":
        for k in range(4):
            y1, y0 = 32 - k * 8, 32 - (k + 1) * 8
            e.cara([(7, y0, 0.3), (25, y0, 0.3), (25, y1, 0.3), (7, y1, 0.3)], (0, 0, 1),
                   color_de("fusta", "taulons", "z", to_fix=min(4, 1 + k)), p, 0.2)
            e.cara([(7, y0, 0.4), (25, y0, 0.4), (25, y0 + 1.2, 0.4), (7, y0 + 1.2, 0.4)], (0, 0, 1),
                   color_de("noguera", "llis", "z", to_fix=4), p, 0.3)
    else:
        e.cara([(7, 2, 0.3), (25, 2, 0.3), (25, 32, 0.3), (7, 32, 0.3)], (0, 0, 1),
               color_de("ferro", "llis", "z", to_fix=4), p, 0.2)
        for k in range(3):
            y = 30 - k * 9
            e.cara([(7, y - 1.2, 0.4), (25, y - 1.2, 0.4), (25, y, 0.4), (7, y, 0.4)], (0, 0, 1),
                   color_de("noguera", "llis", "z", to_fix=4 if k else 3), p, 0.3)
    for x in (5, 25):                                                   # les vores de la boca, de pedra
        e.cara([(x, 0 if tram == "alto" else 2, 0.35), (x + 2, 0 if tram == "alto" else 2, 0.35), (x + 2, 32, 0.35),
                (x, 32, 0.35)], (0, 0, 1), color_de("pedra_grisa", "llis", "z", to_fix=2), p, 0.25)
    for x in (3, 27):                                                   # les baranes dels costats
        for y in ((4, 16, 28) if tram == "fondo" else (6, 18)):
            e.caixa(x, y - 1, 0, x + 2, y + 1, 15, "noguera")
        e.caixa(x - 0.3, 0 if tram == "alto" else 1, 15, x + 2.3, 32 if tram == "fondo" else 21, 17, "noguera",
                dalt="noguera")
    if tram == "fondo":
        e.caixa(3, 1, 15, 29, 3, 17, "noguera", dalt="noguera")
        for x in (9, 16, 23):
            e.caixa(x, 1, 0, x + 2, 3, 15, "noguera")
    return e


# ---------------------------------------------------------------- el registre
R.afegeix("mesa", lambda: mesa(), "Mesa")
R.afegeix("mesa_taberna", lambda: mesa(coses=(_gerres,)), "Mesa de taberna")
R.afegeix("mesa_vela", lambda: mesa(coses=(_espelma,)), "Mesa con vela")
R.afegeix("mesa_libro", lambda: mesa(coses=(_llibre,)), "Mesa con libro")
R.afegeix("mesa_redonda", lambda: mesa_redonda(coses=(_espelma,)), "Mesa redonda")
R.afegeix("taburete", taburete, "Taburete")
R.afegeix("silla", lambda: silla(), "Silla", gira=True)
R.afegeix("silla_noble", lambda: silla("noguera", coixi_mat="vellut", alt_resp=32), "Silla tapizada", gira=True)
R.afegeix("banco", lambda: banco(), "Banco", gira=True)
R.afegeix("sofa", lambda: sofa(), "Sofá", gira=True)
R.afegeix("sofa_verde", lambda: sofa("llana_verda"), "Sofá verde", gira=True)
R.afegeix("escritorio", escritorio, "Escritorio", gira=True)
R.afegeix("atril", atril, "Atril", gira=True)
R.afegeix("mesilla", mesilla, "Mesilla")
R.afegeix("armario", lambda: armario(), "Armario", gira=True)
R.afegeix("armario_noble", lambda: armario("noguera", or_=True), "Armario dorado", gira=True)
R.afegeix("baul", baul, "Baúl", gira=True)
R.afegeix("palangana", palangana, "Palangana")
R.afegeix("biombo", lambda: biombo(), "Biombo", gira=True)
R.afegeix("perchero", perchero, "Perchero")
R.afegeix("espejo", lambda: espejo(), "Espejo", gira=True)
R.afegeix("estanteria", lambda: estanteria(), "Estantería de libros", gira=True)
R.afegeix("estanteria_libros", lambda: estanteria_libros(), "Estantería de biblioteca", gira=True)
R.afegeix("reloj_pie", reloj_pie, "Reloj de pie", gira=True)
R.afegeix("globo", globo, "Globo terráqueo")
R.afegeix("barril", barril, "Barril")
R.afegeix("barril_grifo", barril_grifo, "Barril con grifo", gira=True)
R.afegeix("barriles_pila", barriles_pila, "Barriles apilados", gira=True)
R.afegeix("cofre", lambda: cofre(), "Cofre", gira=True)
R.afegeix("cofre_tesoro", lambda: cofre("noguera", tresor=True), "Cofre del tesoro", gira=True)
R.afegeix("cajas_almacen", cajas, "Cajas de almacén")
R.afegeix("sacos", lambda: sacos(), "Sacos de grano")
R.afegeix("cesta", lambda: cesta(), "Cesta de manzanas")
R.afegeix("cesta_peras", lambda: cesta("fruit_groc"), "Cesta de peras")
R.afegeix("cubo", cubo, "Cubo")
R.afegeix("candelabro", candelabro, "Candelabro")
R.afegeix("lampara_cristal", lampara_cristal, "Lámpara de cristal")
R.afegeix("brasero", lambda: brasero(), "Brasero")
R.afegeix("brasero_cristal", lambda: brasero("cristall"), "Brasero de cristal")
R.afegeix("maceta", maceta, "Maceta")
R.afegeix("maceta_flores", lambda: maceta(7, "estuc"), "Maceta con flores")
R.afegeix("columna_sala", columna_sala, "Columna")
R.afegeix("columna_oscura", lambda: columna_sala(mat="pedra_grisa"), "Columna de piedra gris")
R.afegeix("felpudo", felpudo, "Felpudo", nosa=False)
R.afegeix("escalera_sube_base", lambda: escalera_sube("base"), "Escalera (pie)", gira=True, nosa=False)
R.afegeix("escalera_sube_alto", lambda: escalera_sube("alto"), "Escalera (rellano)", gira=True)
R.afegeix("escalera_piedra_base", lambda: escalera_sube("base", "pedra_clara"), "Escalera de piedra (pie)", gira=True,
          nosa=False)
R.afegeix("escalera_piedra_alto", lambda: escalera_sube("alto", "pedra_clara"), "Escalera de piedra (rellano)",
          gira=True)
R.afegeix("escalera_baja_alto", lambda: escalera_baja("alto"), "Escalera que baja (arriba)", gira=True, nosa=False)
R.afegeix("escalera_baja_fondo", lambda: escalera_baja("fondo"), "Escalera que baja (hueco)", gira=True)
_mig = lambda: mesa_larga("mig")                                        # noqa: E731
R.trams_de("mesa_larga", {"ini": amb_veins(lambda: mesa_larga("ini"), [(_mig, 1, 0)]),
                          "mig": amb_veins(_mig, [(_mig, -1, 0), (_mig, 1, 0)]),
                          "fi": amb_veins(lambda: mesa_larga("fi"), [(_mig, -1, 0)])}, "Mesa larga")
_bmig = lambda: mesa_larga("mig", True)                                 # noqa: E731
R.trams_de("mesa_banquete", {"ini": amb_veins(lambda: mesa_larga("ini", True), [(_bmig, 1, 0)]),
                             "mig": amb_veins(_bmig, [(_bmig, -1, 0), (_bmig, 1, 0)]),
                             "fi": amb_veins(lambda: mesa_larga("fi", True), [(_bmig, -1, 0)])}, "Mesa de banquete")
for _nom, _manta, _coixi, _fusta, _txt in (("cama", "roba2", "roba1", "noguera", "Cama"),
                                           ("cama_roja", "tela_verm", "roba1", "noguera", "Cama de manta roja"),
                                           ("cama_noble", "vellut", "roba1", "noguera", "Cama noble"),
                                           ("cama_pobre", "encenall", "tela", "fusta", "Jergón")):
    _cap = (lambda m=_manta, c=_coixi, f=_fusta: cama("cap", m, c, f))
    _peus = (lambda m=_manta, c=_coixi, f=_fusta: cama("peus", m, c, f))
    R.trams_de(_nom, {"cap": amb_veins(_cap, [(_peus, 0, 1)]), "peus": amb_veins(_peus, [(_cap, 0, -1)])}, _txt)
R.trams_de("litera", {"cap": amb_veins(lambda: litera("cap"), [(lambda: litera("peus"), 0, 1)]),
                      "peus": amb_veins(lambda: litera("peus"), [(lambda: litera("cap"), 0, -1)])}, "Litera")
