#!/usr/bin/env python3
"""ISO (INT-01) — Els mobles dels monuments: el castell (tron, armadures, el
cristall dels Guardians), l'església en ruïnes (bancs, runa, les espelmes
dels Perduts, sarcòfags), l'òpera (butaques, piano, camerinos), la
universitat (pupitres, l'esfera armil·lar que sura, alquímia, telescopi), la
biblioteca (taules de lectura, llibres que volen, el grimori), el coliseu
(gàbies, torn) i el quarter."""
import math

from mobles_base import (Escena, Registre, amb_veins, ampolla, anell, cilindre_x, cilindre_y, coixi, color_de,
                         espelma, llibre_obert, llibres_drets, potes, rng_de, tronc)

R = Registre()


# ---------------------------------------------------------------- castell
def trono():
    """El tron sobre la seva tarima: respatller alt de noguera i vellut, l'anell
    d'or que sura damunt i el rombe de cristall."""
    e = Escena(1, 1)
    e.caixa(1, 1, 0, 31, 31, 4, "pedra_clara", dalt="vellut")
    e.caixa(4, 18, 4, 28, 31, 6, "pedra_clara", dalt="pedra_clara")
    e.caixa(8, 8, 6, 24, 22, 14, "noguera", "plafons", dalt="vellut")
    e.caixa(8, 6, 14, 24, 9, 46, "noguera", "plafons")
    e.caixa(9, 8.5, 16, 23, 9.5, 42, "vellut")
    for x in (6, 24):
        e.caixa(x, 7, 6, x + 2, 22, 22, "or", dalt="or")
        e.caixa(x - 0.5, 20, 22, x + 2.5, 23, 25, "or")
    e.caixa(7, 5.5, 44, 25, 9.5, 48, "or", dalt="or")
    tronc(e, 16, 7.5, 48, 54, 3, 0, 4, "or")
    anell(e, 16, 8, 60, 62, 9, 7, 16, "or")
    tronc(e, 16, 8, 57, 61, 2, 0, 4, "cristall")
    tronc(e, 16, 8, 61, 66, 2, 0, 4, "cristall", gir=math.pi / 4)
    return e


def armadura():
    """Armadura de cavaller sobre el seu suport, amb la llança."""
    e = Escena(1, 1)
    e.caixa(9, 10, 0, 23, 22, 3, "noguera", dalt="noguera")
    for x in (12, 18):
        e.caixa(x, 14, 3, x + 3, 18, 22, "ferro")
    e.caixa(10.5, 13, 22, 21.5, 19, 37, "ferro", dalt="ferro")
    e.caixa(11, 18.6, 25, 21, 19.6, 34, "tela_verm")
    for x in (7.5, 21.5):
        e.caixa(x, 14, 23, x + 3, 18, 36, "ferro", dalt="ferro")
    e.cilindre(16, 16, 37, 39, 2, "ferro", costats=8)
    e.caixa(12.5, 13, 39, 19.5, 19, 46, "ferro", dalt="ferro")
    e.caixa(13, 18.8, 41.5, 19, 19.6, 42.5, "ferro", dalt="ferro")
    tronc(e, 16, 16, 46, 50, 1.5, 0.3, 4, "tela_verm")
    e.cilindre(25, 20, 0, 56, 0.8, "noguera", costats=4)
    tronc(e, 25, 20, 56, 62, 1.6, 0, 4, "ferro")
    return e


def cristal_guardianes():
    """El cristall dels Guardians de la Llum: pedestal octogonal i el cristall
    que sura entre dos anells, com al pati del castell."""
    e = Escena(1, 1)
    g = math.pi / 8
    tronc(e, 16, 16, 0, 4, 11, 10, 8, "pedra_clara", gir=g)
    tronc(e, 16, 16, 4, 16, 6, 5, 8, "pedra_clara", "carreus", gir=g)
    tronc(e, 16, 16, 16, 19, 8, 8, 8, "or", gir=g)
    tronc(e, 16, 16, 24, 34, 0.1, 5, 4, "cristall")
    tronc(e, 16, 16, 34, 46, 5, 0, 4, "cristall")
    anell(e, 16, 16, 30, 31.5, 11, 9.5, 16, "or")
    anell(e, 16, 16, 40, 41.5, 8, 6.5, 16, "llauto", gir=0.3)
    return e


def tocador():
    """Tocador amb mirall oval, pots i un cofret."""
    e = Escena(1, 1)
    e.caixa(4, 8, 13, 28, 20, 16, "noguera", dalt="noguera")
    potes(e, 5, 9, 27, 19, 13, 2, "noguera")
    e.caixa(5, 9, 9, 27, 19, 13, "noguera", "plafons")
    e.caixa(9, 8, 16, 23, 10, 42, "or")
    e.rect("y", 10, 1, 10.5, 21.5, 18, 40, "vidre", to=1, biaix=0.4)
    e.rect("y", 10.1, 1, 11.5, 15, 22, 36, "vidre", to=0, biaix=0.6)
    for (x, m) in ((7, "tela_verm"), (25, "porticons")):
        tronc(e, x, 14, 16, 20, 1.4, 1, 6, m)
    e.caixa(14, 13, 16, 20, 18, 19, "or", dalt="vellut")
    return e


def estandarte():
    """Estendard de peu amb la bandera de la ciutat."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 3, 5, 4, 8, "pedra_clara")
    e.cilindre(16, 16, 3, 62, 0.9, "noguera", costats=6)
    e.caixa(9, 15.4, 58, 23, 16.6, 59.5, "or")
    e.caixa(10, 16, 30, 22, 17, 58, "tela_verm", "plecs")
    e.caixa(10, 16.2, 42, 22, 17.2, 45, "roba1")
    tronc(e, 16, 16, 62, 66, 1.6, 0, 6, "or")
    return e


# ---------------------------------------------------------------- església
def banco_iglesia(tram, trencat=False):
    """Banc d'església (de cara a +Y, cap a l'altar quan es gira) en trams al
    llarg de X, amb el respatller i el reclinatori."""
    e = Escena(1, 1)
    x0 = 3 if tram == "ini" else 0
    x1 = 29 if tram == "fi" else 32
    e.caixa(x0, 10, 9, x1, 22, 11.5, "noguera", dalt="noguera", tex_dalt="taulons")
    e.caixa(x0, 7, 11.5, x1, 9.5, 30, "noguera", "plafons", dalt="noguera")
    e.caixa(x0, 25, 4, x1, 28, 6, "noguera", dalt="noguera")
    if tram != "mig":
        x = x0 if tram == "ini" else x1 - 2.5
        e.caixa(x, 7, 0, x + 2.5, 28, 22, "noguera", "plafons", dalt="noguera")
    else:
        for x in (4, 26):
            e.caixa(x, 12, 0, x + 2, 20, 9, "noguera")
    if trencat:
        e.caixa(6, 11, 11.5, 18, 21, 12.5, "pedra_fosca", dalt="pedra_fosca")
    return e


def banco_roto():
    """Un banc trencat i tombat, amb runa."""
    e = Escena(1, 1)
    p = e.nova_peca()
    e.cara([(3, 9, 0), (29, 13, 0), (29, 13, 3), (3, 9, 10)], (0, 1, 0.4), color_de("noguera", "taulons", "y"), p)
    e.cara([(3, 9, 10), (29, 13, 3), (29, 23, 3), (3, 19, 10)], (0, 0.2, 1), color_de("noguera", "taulons", "z"), p)
    e.caixa(5, 20, 0, 27, 24, 4, "noguera", dalt="noguera")
    for (x, y, h) in ((8, 26, 3), (20, 27, 2), (25, 7, 4)):
        e.caixa(x, y, 0, x + 4, y + 3, h, "pedra_fosca", dalt="pedra_fosca")
    return e


def runa(llavor=5, mat="pedra_fosca"):
    """Pila de runa: carreus caiguts del sostre."""
    e = Escena(1, 1)
    rng = rng_de(llavor)
    for k in range(7):
        x, y = 5 + rng.random() * 18, 5 + rng.random() * 18
        w, d, h = 4 + rng.random() * 5, 3 + rng.random() * 4, 2 + rng.random() * 4
        z = 0 if k < 4 else rng.random() * 4
        e.caixa(x, y, z, x + w, y + d, z + h, mat, "carreus", dalt=mat)
    return e


def viga():
    """Biga del sostre caiguda, de biaix sobre la runa."""
    e = runa(9)
    p = e.nova_peca()
    e.cara([(2, 14, 0), (30, 10, 14), (30, 13, 14), (2, 17, 0)], (0, 0.2, 1), color_de("noguera", "taulons", "z"), p, 0.5)
    e.cara([(2, 17, 0), (30, 13, 14), (30, 13, 10), (2, 17, -2)], (0, 1, 0), color_de("noguera", "taulons", "y"), p, 0.5)
    e.cara([(30, 10, 14), (30, 13, 14), (30, 13, 10), (30, 10, 10)], (1, 0, 0), color_de("noguera", "llis", "x"), p, 0.5)
    return e


def columna_rota():
    e = Escena(1, 1)
    e.caixa(7, 7, 0, 25, 25, 4, "pedra_grisa", dalt="pedra_grisa")
    e.cilindre(16, 16, 4, 30, 6, "pedra_grisa", "taulons_v", costats=12)
    p = e.nova_peca()
    pts = [(16 + 6 * math.cos(a), 16 + 6 * math.sin(a)) for a in [k * math.pi / 6 for k in range(12)]]
    e.cara([(x, y, 30 + 6 * math.sin(i) + (x - 16) * 0.6) for i, (x, y) in enumerate(pts)], (-0.4, 0, 1),
           color_de("pedra_grisa", "llis", "z"), p)
    e.caixa(20, 22, 0, 30, 28, 5, "pedra_grisa", dalt="pedra_grisa")
    return e


def pila_bautismal():
    e = Escena(1, 1)
    g = math.pi / 8
    tronc(e, 16, 16, 0, 3, 9, 9, 8, "pedra_grisa", gir=g)
    tronc(e, 16, 16, 3, 14, 4, 4, 8, "pedra_grisa", gir=g)
    tronc(e, 16, 16, 14, 20, 6, 10, 8, "pedra_grisa", gir=g, dalt="aigua")
    return e


def estatua_santa():
    """Estàtua de la Llum sobre un pedestal, amb les mans obertes."""
    e = Escena(1, 1)
    e.caixa(7, 7, 0, 25, 25, 10, "pedra_grisa", "carreus", dalt="pedra_grisa")
    tronc(e, 16, 16, 10, 34, 6, 3.5, 10, "pedra_clara")
    tronc(e, 16, 16, 34, 38, 3.5, 2, 8, "pedra_clara")
    tronc(e, 16, 16, 38, 44, 2.6, 2.2, 8, "pedra_clara", dalt="pedra_clara")
    for d in (-1, 1):
        e.caixa(16 + d * 3.5 - 1, 15, 30, 16 + d * 7 + 1, 17, 32, "pedra_clara")
    anell(e, 16, 16, 47, 48, 5, 4, 12, "or")
    return e


def circulo_velas():
    """El cercle de ciris dels Perduts: ciris a terra en rotllana al voltant del
    seu signe (un triangle vermell dins d'un cercle)."""
    e = Escena(1, 1)
    p = e.nova_peca()
    pts = [(16 + 13 * math.cos(a), 16 + 13 * math.sin(a)) for a in [k * math.pi / 8 for k in range(16)]]
    e.cara([(x, y, 0.3) for x, y in pts], (0, 0, 1), color_de("tela_verm", "llis", "z", to_fix=3), p, 0.2)
    ints = [(16 + 11.5 * math.cos(a), 16 + 11.5 * math.sin(a)) for a in [k * math.pi / 8 for k in range(16)]]
    e.cara([(x, y, 0.35) for x, y in ints], (0, 0, 1), color_de("ferro", "llis", "z", to_fix=4), e.nova_peca(), 0.25)
    q = [(16 + 9 * math.cos(a), 16 + 9 * math.sin(a)) for a in [k * 2 * math.pi / 3 + 0.3 for k in range(3)]]
    for i in range(3):
        (xa, ya), (xb, yb) = q[i], q[(i + 1) % 3]
        dx, dy = yb - ya, xa - xb
        l = math.hypot(dx, dy) or 1
        dx, dy = dx / l * 0.8, dy / l * 0.8
        e.cara([(xa - dx, ya - dy, 0.4), (xb - dx, yb - dy, 0.4), (xb + dx, yb + dy, 0.4), (xa + dx, ya + dy, 0.4)],
               (0, 0, 1), color_de("tela_verm", "llis", "z", to_fix=1), e.nova_peca(), 0.3)
    for k in range(0, 16, 2):
        x, y = pts[k]
        e.cilindre(x, y, 0, 6 + (k % 3) * 3, 1.3, "cera", costats=6)
        tronc(e, x, y, 6 + (k % 3) * 3, 10 + (k % 3) * 3, 1.2, 0.1, 4, "foc")
    return e


def sarcofago(tram):
    """Sarcòfag de pedra en dos trams al llarg de Y (cap i peus), amb la figura jacent."""
    e = Escena(1, 1)
    y0, y1 = (4, 32) if tram == "cap" else (0, 28)
    e.caixa(5, y0, 0, 27, y1, 14, "pedra_grisa", "carreus", dalt="pedra_grisa")
    e.caixa(4, y0 - (1 if tram == "cap" else 0), 14, 28, y1 + (1 if tram == "peus" else 0), 16, "pedra_grisa",
            dalt="pedra_grisa")
    if tram == "cap":
        tronc(e, 16, 12, 16, 20, 3, 2.6, 8, "pedra_clara")
        e.caixa(11, 16, 16, 21, 32, 19, "pedra_clara", dalt="pedra_clara")
    else:
        e.caixa(11, 0, 16, 21, 16, 19, "pedra_clara", dalt="pedra_clara")
        e.caixa(12, 16, 16, 20, 22, 21, "pedra_clara", dalt="pedra_clara")
    return e


# ---------------------------------------------------------------- òpera
def butaca(tram=None):
    """Butaca de platea de vellut vermell, de cara a +Y."""
    e = Escena(1, 1)
    for x0 in (2, 17):
        e.caixa(x0, 12, 6, x0 + 13, 24, 10, "vellut", dalt="vellut")
        e.caixa(x0, 9, 10, x0 + 13, 12.5, 26, "vellut", "plecs", dalt="noguera")
    for x in (1, 15, 29.5):
        e.caixa(x, 9, 0, x + 1.5, 24, 15, "noguera", dalt="or")
    return e


def piano():
    """Piano de cua de noguera negra, amb la tapa alçada i les tecles."""
    e = Escena(1, 1)
    p = e.nova_peca()
    cos = [(4, 8), (28, 8), (28, 18), (20, 27), (12, 28), (4, 22)]
    for (xa, ya), (xb, yb) in zip(cos, cos[1:] + cos[:1]):
        n = (yb - ya, -(xb - xa), 0)
        e.cara([(xa, ya, 10), (xb, yb, 10), (xb, yb, 17), (xa, ya, 17)], n, color_de("carbo", "llis", "y"), p)
    e.cara([(x, y, 17) for x, y in cos], (0, 0, 1), color_de("carbo", "llis", "z"), p)
    for (x, y) in ((6, 10), (25, 10), (14, 25)):
        e.caixa(x, y, 0, x + 2, y + 2, 10, "carbo")
    e.caixa(5, 4, 12, 27, 8, 15, "carbo", dalt="roba1")
    for x in range(6, 27, 2):
        e.caixa(x + 0.5, 4.2, 15, x + 1.3, 6, 15.6, "carbo")
    p = e.nova_peca()
    e.cara([(28, 18, 17), (12, 28, 17), (14, 26, 33)], (1, 1.6, 1.4), color_de("carbo", "llis", "y"), p, 0.4, True)
    e.caixa(14, 1, 0, 22, 4, 9, "noguera", dalt="vellut")
    return e


def atril_musica():
    e = Escena(1, 1)
    for a in (0.4, 2.5, 4.6):
        x, y = 16 + 4.5 * math.cos(a), 16 + 4.5 * math.sin(a)
        e.caixa(x - 0.5, y - 0.5, 0, x + 0.5, y + 0.5, 3, "ferro")
    e.cilindre(16, 16, 3, 30, 0.6, "ferro", costats=4)
    p = e.nova_peca()
    e.cara([(10, 18, 30), (22, 18, 30), (22, 15, 38), (10, 15, 38)], (0, 8, 3), color_de("ferro", "llis", "y"), p)
    e.cara([(11, 17.8, 30.6), (21, 17.8, 30.6), (21, 15.2, 37.4), (11, 15.2, 37.4)], (0, 8, 3),
           color_de("pergami", "llis", "y", to_fix=0), e.nova_peca(), 0.4)
    return e


def tocador_camerino():
    """El tocador dels camerinos: mirall voltat de llums de cristall."""
    e = Escena(1, 1)
    e.caixa(3, 6, 13, 29, 20, 16, "noguera", dalt="noguera")
    potes(e, 4, 7, 28, 19, 13, 2, "noguera")
    e.caixa(6, 5, 16, 26, 7, 44, "noguera")
    e.rect("y", 7, 1, 8, 24, 19, 41, "vidre", to=1, biaix=0.4)
    for z in (20, 28, 36):
        for x in (6.5, 25.5):
            tronc(e, x, 7.5, z, z + 3, 1.2, 0, 4, "cristall")
    for x in (9, 14, 19):
        tronc(e, x, 14, 16, 18, 1.2, 0.8, 6, ("tela_verm", "roba3", "porticons")[x % 3])
    e.caixa(20, 11, 16, 26, 17, 17, "llana_lila", dalt="llana_lila")
    return e


def perchero_vestuario():
    """Burra de vestuari de teatre: capes, un vestit de reina, una armadura de cartó."""
    from mobles_oficis import perchero_ropa
    e = perchero_ropa()
    tronc(e, 25, 16, 38, 42, 3, 3, 10, "or", dalt="or")
    return e


def taquilla():
    """La taquilla de l'òpera: caseta de noguera amb la finestreta i el rètol."""
    e = Escena(1, 1)
    e.caixa(3, 3, 0, 29, 29, 18, "noguera", "plafons", dalt="noguera")
    e.caixa(3, 3, 18, 29, 7, 44, "noguera", "plafons")
    for x in (3, 26):
        e.caixa(x, 3, 18, x + 3, 29, 44, "noguera")
    e.caixa(2, 2, 44, 30, 30, 48, "vellut", dalt="or")
    e.rect("y", 29, 1, 8, 24, 20, 36, "vidre", to=2, biaix=0.4)
    e.caixa(6, 29, 37, 26, 30, 43, "or")
    e.rect("y", 30.1, 1, 8, 24, 38, 42, "vellut", to=2, biaix=0.6)
    return e


def foco():
    """Fanal de bambolina (llum de cristall del proscenio)."""
    e = Escena(1, 1)
    e.caixa(10, 12, 0, 22, 20, 6, "llauto", dalt="llauto")
    tronc(e, 16, 16, 6, 10, 4, 5, 8, "llauto")
    tronc(e, 16, 16, 10, 16, 0.5, 4, 4, "cristall")
    tronc(e, 16, 16, 16, 20, 4, 0, 4, "cristall")
    return e


# ---------------------------------------------------------------- universitat
def pupitre():
    """Pupitre d'estudiant amb el seu banc (de cara a +Y), llibre i tinter."""
    e = Escena(1, 1)
    p = e.nova_peca()
    e.cara([(4, 16, 16), (28, 16, 16), (28, 28, 13), (4, 28, 13)], (0, 3, 12), color_de("noguera", "taulons", "z"), p)
    e.caixa(4, 16, 5, 28, 28, 13, "noguera", "plafons")
    e.caixa(5, 4, 8, 27, 11, 10, "noguera", dalt="noguera")
    for x in (6, 24):
        e.caixa(x, 5, 0, x + 2, 10, 8, "noguera")
    e.caixa(5, 2, 10, 27, 4, 20, "noguera")
    e.caixa(9, 19, 14.6, 17, 25, 15.4, "pergami", dalt="pergami")
    e.cilindre(23, 18, 16, 18, 1.2, "ferro", costats=6)
    return e


def orrery():
    """L'esfera armil·lar de la universitat: anells que suren al voltant d'un sol de cristall."""
    e = Escena(1, 1)
    g = math.pi / 6
    tronc(e, 16, 16, 0, 4, 9, 8, 6, "noguera", gir=g)
    tronc(e, 16, 16, 4, 16, 2.5, 2, 6, "llauto", gir=g)
    tronc(e, 16, 16, 16, 18, 5, 5, 6, "llauto", gir=g)
    tronc(e, 16, 16, 22, 28, 0.5, 4, 4, "cristall")
    tronc(e, 16, 16, 28, 34, 4, 0.5, 4, "cristall")
    anell(e, 16, 16, 27, 28, 12, 11, 20, "or")
    anell(e, 16, 16, 33, 34, 9, 8, 16, "llauto", gir=0.2)
    anell(e, 16, 16, 22, 23, 7, 6, 14, "coure", gir=0.5)
    for (x, y, z, m) in ((27, 16, 28, "roba2"), (8, 12, 34, "teula"), (16, 23, 23, "fulla")):
        tronc(e, x, y, z, z + 2, 1.6, 1.6, 6, m, dalt=m)
    return e


def mesa_alquimia():
    """Taula d'alquimista: alambí, matràs que bull amb una flama de cristall, pots."""
    e = Escena(1, 1)
    e.caixa(2, 6, 14, 30, 26, 16.5, "noguera", dalt="pedra_grisa")
    potes(e, 3, 7, 29, 25, 14, 2.5, "noguera")
    tronc(e, 9, 14, 16.5, 19, 2.6, 2.6, 6, "ferro")
    tronc(e, 9, 14, 19, 24, 0.4, 4, 4, "cristall")
    tronc(e, 9, 14, 24, 29, 4, 0.4, 4, "vidre")
    e.cilindre(9, 14, 29, 33, 0.8, "vidre", costats=6)
    e.caixa(9, 13.6, 32, 20, 14.4, 33, "vidre")
    e.caixa(19.6, 13.6, 25, 20.4, 14.4, 33, "vidre")
    tronc(e, 20, 14, 16.5, 21, 3, 3.4, 8, "llauto", dalt="verdet")
    for (x, y, m) in ((25, 20, "tela_verm"), (14, 22, "porticons"), (21, 22, "roba3")):
        ampolla(e, x, y, 16.5, m, 5)
    return e


def estanteria_frascos(llavor=41):
    """Prestatgeria de pots i matrassos de colors (laboratori, herbolari)."""
    e = Escena(1, 1)
    rng = rng_de(llavor)
    e.caixa(2, 3, 0, 30, 5, 62, "noguera")
    e.rect("y", 5, 1, 2, 30, 0, 62, "noguera", to=4, biaix=0.2)
    for x in (2, 28):
        e.caixa(x, 3, 0, x + 2, 15, 62, "noguera", dalt="noguera")
    e.caixa(2, 3, 58, 30, 15, 62, "noguera", dalt="noguera")
    for z in (2, 20, 38):
        e.caixa(4, 5, z, 28, 15, z + 1.5, "noguera", dalt="noguera")
        x = 6.5
        while x < 26:
            m = ("verdet", "tela_verm", "roba2", "roba3", "porticons", "llana_lila", "vidre")[rng.integers(7)]
            if rng.random() < 0.5:
                tronc(e, x, 10, z + 1.5, z + 6, 2, 2, 8, m, dalt="noguera")
                x += 5
            else:
                ampolla(e, x, 10, z + 1.5, m, 4 + rng.integers(0, 4))
                x += 3.6
    return e


def telescopio():
    """Telescopi de llautó sobre trípode, apuntant al cel."""
    e = Escena(1, 1)
    for a in (0.3, 2.4, 4.5):
        x, y = 16 + 8 * math.cos(a), 16 + 8 * math.sin(a)
        p = e.nova_peca()
        e.cara([(x - 0.8, y, 0), (x + 0.8, y, 0), (16.8, 16, 24), (15.2, 16, 24)], (math.cos(a), math.sin(a), 0.3),
               color_de("noguera", "llis", "y"), p, 0.0, True)
    e.cilindre(16, 16, 22, 26, 2, "llauto", costats=8)
    p = e.nova_peca()
    for (a, b, r) in ((0, 1, 2.6), (1, 2, 2.0)):
        za, zb = 26 + a * 14, 26 + b * 14
        xa, xb = 10 + a * 9, 10 + b * 9
        e.cara([(xa, 16 - r, za), (xb, 16 - r * 0.8, zb), (xb, 16 + r * 0.8, zb), (xa, 16 + r, za)], (-1, 0, 0.6),
               color_de("llauto", "llis", "z"), p)
        e.cara([(xa, 16 + r, za), (xb, 16 + r * 0.8, zb), (xb, 16 + r * 0.8, zb - 2 * r * 0.8), (xa, 16 + r, za - 2 * r)],
               (0, 1, 0), color_de("llauto", "llis", "y"), p)
    return e


def bola_cristal():
    e = Escena(1, 1)
    g = math.pi / 6
    tronc(e, 16, 16, 0, 3, 7, 6, 6, "noguera", gir=g)
    tronc(e, 16, 16, 3, 14, 2, 1.6, 6, "noguera", gir=g)
    tronc(e, 16, 16, 14, 17, 4, 4.5, 6, "llauto", gir=g)
    tronc(e, 16, 16, 17, 22, 3, 5.5, 10, "cristall")
    tronc(e, 16, 16, 22, 27, 5.5, 3, 10, "cristall")
    tronc(e, 16, 16, 27, 29, 3, 0.5, 10, "cristall")
    return e


# ---------------------------------------------------------------- biblioteca
def mesa_lectura(tram):
    """Taula llarga de lectura en trams al llarg de X: llibres oberts i llums de cristall."""
    e = Escena(1, 1)
    x0 = 3 if tram == "ini" else 0
    x1 = 29 if tram == "fi" else 32
    e.caixa(x0, 6, 13.5, x1, 26, 16.5, "noguera", "plafons", dalt="noguera", tex_dalt="taulons")
    e.caixa(x0, 9, 8, x1, 23, 13.5, "noguera")
    if tram != "mig":
        x = x0 + 1 if tram == "ini" else x1 - 4
        e.caixa(x, 7, 0, x + 3, 25, 13.5, "noguera", "plafons")
    rng = rng_de({"ini": 1, "mig": 2, "fi": 3}[tram])
    llibre_obert(e, 10, 12, 16.5, ("tela_verm", "porticons", "roba2")[rng.integers(3)])
    llibre_obert(e, 21, 20, 16.5, ("vellut", "roba3", "verdet")[rng.integers(3)])
    if tram == "mig":
        e.cilindre(24, 11, 16.5, 25, 0.7, "llauto", costats=4)
        tronc(e, 24, 11, 25, 28, 2.4, 0.5, 4, "cristall")
    else:
        llibres_drets(e, 7, 15, 20, 24, 16.5, rng, alt=(5, 7))
    return e


def libros_flotantes():
    """Llibres que suren en rotllana (la màgia de la biblioteca), sobre un peu de cristall."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 3, 6, 5, 8, "pedra_clara")
    tronc(e, 16, 16, 3, 9, 2, 0.5, 4, "cristall")
    for k in range(5):
        a = k * 2 * math.pi / 5
        x, y, z = 16 + 9 * math.cos(a), 16 + 9 * math.sin(a), 24 + 6 * math.sin(a * 2 + 1)
        m = ("tela_verm", "porticons", "roba2", "roba3", "llana_lila")[k]
        e.caixa(x - 3, y - 2, z, x + 3, y + 2, z + 1.6, m, dalt="pergami")
    anell(e, 16, 16, 18, 18.8, 11, 10, 16, "llauto")
    return e


def escalera_biblioteca():
    """L'escala de biblioteca recolzada a la prestatgeria (de cara a +Y)."""
    from mobles_comuns import estanteria
    e = estanteria(51)
    for x in (9, 21):
        p = e.nova_peca()
        e.cara([(x, 26, 0), (x + 2, 26, 0), (x + 2, 15, 60), (x, 15, 60)], (0, 5.5, 1), color_de("fusta", "llis", "y"), p,
               0.6)
    for k in range(1, 9):
        z = k * 6.5
        y = 26 - z * 11 / 60
        e.caixa(10, y - 1, z, 22, y + 0.5, z + 1, "fusta")
    return e


def atril_grimorio():
    """El grimori del faristol de l'arxiu prohibit: tancat amb cadenes i un ull de cristall."""
    from mobles_comuns import atril
    e = atril("vellut", "pedra_grisa")
    tronc(e, 16, 15.5, 33, 36, 1.4, 0, 4, "cristall")
    anell(e, 16, 16, 40, 41, 7, 6, 14, "or")
    return e


def estanteria_cadenas(llavor=61):
    """Prestatgeria de l'arxiu prohibit: llibres encadenats i runes que brillen."""
    from mobles_comuns import estanteria
    e = estanteria(llavor)
    for z in (12, 31, 50):
        e.caixa(4, 15, z, 28, 15.6, z + 0.8, "ferro")
        for x in (6, 16, 26):
            e.caixa(x - 0.7, 15, z - 2, x + 0.7, 15.8, z + 2, "ferro")
    for (x, z) in ((8, 60.5), (16, 61), (24, 60.5)):
        tronc(e, x, 15.5, z, z + 2, 1, 0, 4, "cristall")
    return e


# ---------------------------------------------------------------- coliseu i quarter
def jaula():
    """Gàbia de ferro de les feres (als fossats del coliseu)."""
    e = Escena(1, 1)
    e.caixa(3, 3, 0, 29, 29, 2, "ferro", dalt="palla")
    e.caixa(3, 3, 34, 29, 29, 36, "ferro", dalt="ferro")
    for k in range(0, 27, 4):
        for (x, y) in ((3 + k, 3), (3 + k, 28), (3, 3 + k), (28, 3 + k)):
            e.caixa(x, y, 2, x + 1, y + 1, 34, "ferro")
    e.caixa(8, 10, 2, 20, 16, 4, "pedra_clara", dalt="pedra_clara")
    return e


def cabrestante():
    """El torn del muntacàrregues de l'arena, amb la cadena."""
    e = Escena(1, 1)
    for x in (5, 25):
        e.caixa(x, 10, 0, x + 2, 22, 22, "noguera")
    cilindre_x(e, 7, 25, 16, 16, 5, "noguera", tex="taulons_v", costats=12, tapa="noguera")
    for y in (12, 20):
        e.caixa(14, y, 4, 18, y + 0.8, 28, "noguera")
    for z in range(22, 64, 4):
        e.caixa(15.4, 20.5, z, 16.6, 21.5, z + 3, "ferro")
    return e


def huesos():
    e = Escena(1, 1)
    rng = rng_de(13)
    for k in range(6):
        x, y = 6 + rng.random() * 18, 6 + rng.random() * 18
        cilindre_x(e, x, x + 7, y, 1.2, 0.9, "pedra_clara", tex="llis", costats=6, tapa="pedra_clara")
    tronc(e, 18, 18, 0, 4, 3.5, 2.5, 8, "pedra_clara", dalt="pedra_clara")
    return e


# ---------------------------------------------------------------- el registre
R.afegeix("trono", trono, "Trono", gira=True)
R.afegeix("armadura", armadura, "Armadura", gira=True)
R.afegeix("cristal_guardianes", cristal_guardianes, "Cristal de los Guardianes")
R.afegeix("tocador", tocador, "Tocador", gira=True)
R.afegeix("estandarte", estandarte, "Estandarte")
_bi = lambda t, r=False: (lambda: banco_iglesia(t, r))                  # noqa: E731
R.trams_de("banco_iglesia", {"ini": amb_veins(_bi("ini"), [(_bi("mig"), 1, 0)]),
                             "mig": amb_veins(_bi("mig"), [(_bi("mig"), -1, 0), (_bi("mig"), 1, 0)]),
                             "fi": amb_veins(_bi("fi"), [(_bi("mig"), -1, 0)]),
                             "roto": amb_veins(_bi("mig", True), [(_bi("mig"), -1, 0), (_bi("mig"), 1, 0)])},
           "Banco de iglesia")
R.afegeix("banco_roto", banco_roto, "Banco roto", gira=True)
R.afegeix("runa", lambda: runa(), "Escombros")
R.afegeix("runa_b", lambda: runa(23), "Escombros (otro)")
R.afegeix("viga", viga, "Viga caída", gira=True)
R.afegeix("columna_rota", columna_rota, "Columna rota")
R.afegeix("pila_bautismal", pila_bautismal, "Pila bautismal")
R.afegeix("estatua_santa", estatua_santa, "Estatua de la Luz", gira=True)
R.afegeix("circulo_velas", circulo_velas, "Círculo de velas de los Perdidos")
R.trams_de("sarcofago", {"cap": amb_veins(lambda: sarcofago("cap"), [(lambda: sarcofago("peus"), 0, 1)]),
                         "peus": amb_veins(lambda: sarcofago("peus"), [(lambda: sarcofago("cap"), 0, -1)])},
           "Sarcófago")
R.afegeix("butaca", butaca, "Butaca", gira=True)
R.afegeix("piano", piano, "Piano", gira=True)
R.afegeix("atril_musica", atril_musica, "Atril de música", gira=True)
R.afegeix("tocador_camerino", tocador_camerino, "Tocador de camerino", gira=True)
R.afegeix("perchero_vestuario", perchero_vestuario, "Vestuario", gira=True)
R.afegeix("taquilla", taquilla, "Taquilla", gira=True)
R.afegeix("foco", foco, "Foco del escenario")
R.afegeix("pupitre", pupitre, "Pupitre", gira=True)
R.afegeix("orrery", orrery, "Esfera armilar")
R.afegeix("mesa_alquimia", mesa_alquimia, "Mesa de alquimia", gira=True)
R.afegeix("estanteria_frascos", lambda: estanteria_frascos(), "Estantería de frascos", gira=True)
R.afegeix("telescopio", telescopio, "Telescopio", gira=True)
R.afegeix("bola_cristal", bola_cristal, "Bola de cristal")
_ml = lambda t: (lambda: mesa_lectura(t))                               # noqa: E731
R.trams_de("mesa_lectura", {"ini": amb_veins(_ml("ini"), [(_ml("mig"), 1, 0)]),
                            "mig": amb_veins(_ml("mig"), [(_ml("mig"), -1, 0), (_ml("mig"), 1, 0)]),
                            "fi": amb_veins(_ml("fi"), [(_ml("mig"), -1, 0)])}, "Mesa de lectura")
R.afegeix("libros_flotantes", libros_flotantes, "Libros que flotan")
R.afegeix("escalera_biblioteca", escalera_biblioteca, "Escalera de biblioteca", gira=True)
R.afegeix("atril_grimorio", atril_grimorio, "Grimorio", gira=True)
R.afegeix("estanteria_cadenas", lambda: estanteria_cadenas(), "Estantería de libros encadenados", gira=True)
R.afegeix("jaula", jaula, "Jaula")
R.afegeix("cabrestante", cabrestante, "Cabrestante", gira=True)
R.afegeix("huesos", huesos, "Huesos")
