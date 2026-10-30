#!/usr/bin/env python3
"""ISO (INT-01) — Els mobles dels oficis: la taverna i la posada, el mercat,
la ferreria i l'armeria, el banc i la joieria, la sastreria, la casa de te i
els banys. La cara de davant és +Y (o0); els taulells, en trams."""
import math

from mobles_base import (MATERIALS, Escena, Registre, amb_veins, ampolla, anell, cilindre_x, cilindre_y, coixi,
                         color_de, espelma, gerra, mata, potes, rng_de, tassa, tronc)
from patis import armes

R = Registre()


# ---------------------------------------------------------------- taverna i posada
def barra(coses=None, frontal="fusta", tauler="noguera"):
    """Tram de taulell al llarg de X, de cara a +Y (els clients), amb el
    reposapeus de llautó; coses: el que hi ha a sobre."""
    e = Escena(1, 1)
    e.caixa(0, 8, 0, 32, 24, 18, frontal, "plafons")
    e.caixa(0, 6, 18, 32, 27, 21, tauler, dalt=tauler, tex_dalt="taulons")
    e.caixa(0, 25, 3, 32, 26.5, 4.5, "llauto")
    e.caixa(0, 24, 0, 32, 25, 2, "noguera")
    for c in coses or ():
        c(e)
    return e


def _gerres(e):
    gerra(e, 8, 16, 21)
    gerra(e, 22, 13, 21)


def _ampolles(e):
    ampolla(e, 9, 14, 21, "verdet")
    ampolla(e, 13, 12, 21, "tela_verm", 6)
    e.cilindre(22, 16, 21, 22, 3, "pedra_clara", costats=8)


def _registre(e):
    """El llibre de registre de la posada, la campaneta i les claus."""
    e.caixa(6, 12, 21, 16, 20, 22, "tela_verm", dalt="pergami")
    e.cilindre(22, 15, 21, 22, 2.2, "noguera", costats=8)
    tronc(e, 22, 15, 22, 25, 2, 0.6, 8, "llauto")
    for x in (26, 28):
        e.caixa(x, 13, 21, x + 1, 17, 21.6, "llauto")


def _monedes(e):
    for (x, y, n) in ((8, 14, 4), (12, 16, 2), (22, 13, 3)):
        for k in range(n):
            e.cilindre(x, y, 21 + k * 0.8, 21.8 + k * 0.8, 2, "or", costats=8)
    e.caixa(16, 10, 21, 20, 18, 21.5, "pergami", dalt="pergami")


def _lupa(e):
    e.caixa(6, 11, 21, 18, 19, 21.6, "vellut", dalt="vellut")
    for x, m in ((9, "cristall"), (12, "tela_verm"), (15, "porticons")):
        tronc(e, x, 15, 21.6, 23.5, 1.4, 0, 4, m)
    anell(e, 23, 15, 21.6, 22.4, 3, 2.2, 10, "llauto")
    e.caixa(25, 14.5, 21.6, 29, 15.5, 22.4, "noguera")


def _espasa(e):
    p = e.nova_peca()
    e.cara([(4, 15, 21.2), (24, 15, 21.2), (24, 17, 21.2), (4, 17, 21.2)], (0, 0, 1), color_de("ferro", "llis", "z", to_fix=1),
           p, 0.2)
    e.caixa(24, 13, 21, 25.5, 19, 22, "llauto")
    e.caixa(25.5, 15.3, 21, 29, 16.7, 22, "noguera")


def reixa_banc(e):
    """La reixa de llautó del taulell del banc (sobre el tauler)."""
    for x in range(2, 32, 5):
        e.caixa(x, 15.5, 21, x + 1, 16.5, 46, "llauto")
    e.caixa(0, 15, 44, 32, 17, 47, "noguera", dalt="noguera")
    e.caixa(0, 15, 21, 32, 17, 23, "noguera")


def estanteria_botellas():
    """Armari obert de lleixes amb ampolles i gerres (darrere la barra)."""
    e = Escena(1, 1)
    e.caixa(2, 3, 0, 30, 5, 62, "noguera")
    e.rect("y", 5, 1, 2, 30, 0, 62, "noguera", to=4, biaix=0.2)
    for x in (2, 28):
        e.caixa(x, 3, 0, x + 2, 16, 62, "noguera", dalt="noguera")
    e.caixa(2, 3, 58, 30, 16, 62, "noguera", dalt="noguera")
    e.caixa(4, 5, 0, 28, 16, 3, "noguera", dalt="noguera")
    rng = rng_de(11)
    for z in (3, 21, 39):
        e.caixa(4, 5, z, 28, 16, z + 1.5, "noguera", dalt="fusta")
        x = 6.5
        while x < 26:
            if rng.random() < 0.7:
                ampolla(e, x, 11, z + 1.5, ("verdet", "vidre", "tela_verm", "llauto", "porticons")[rng.integers(5)],
                        6 + rng.integers(0, 4))
                x += 3.6
            else:
                gerra(e, x + 0.5, 11, z + 1.5)
                x += 5
    return e


def asador():
    """La llar del mig de la sala: foc a terra dins d'un cercle de pedres, amb l'ast."""
    e = Escena(1, 1)
    from mobles_base import poligon_regular
    for k, (x, y) in enumerate(poligon_regular(16, 16, 10, 10)):
        e.caixa(x - 2.2, y - 2.2, 0, x + 2.2, y + 2.2, 4 + (k % 2), "pedra_grisa", dalt="pedra_grisa")
    for (x, h) in ((12, 9), (16, 13), (20, 8)):
        tronc(e, x, 17, 1, 1 + h, 2.6, 0.2, 6, "foc")
    for x in (5, 26):
        e.caixa(x, 15, 0, x + 1.5, 17, 20, "ferro")
    e.caixa(5, 15.5, 18.5, 27.5, 16.5, 19.5, "ferro")
    cilindre_x(e, 11, 21, 16, 19, 3, "teula", tex="llis", costats=8, tapa="teula")
    return e


def equipaje():
    """L'equipatge d'un viatger: dues maletes i un farcell."""
    e = Escena(1, 1)
    e.caixa(5, 8, 0, 23, 20, 10, "noguera", "taulons", dalt="noguera")
    e.caixa(5, 7.6, 3, 23, 20.4, 4.5, "ferro")
    e.caixa(8, 11, 10, 20, 19, 17, "tela_verm", dalt="tela_verm")
    e.caixa(12, 14.5, 17, 16, 15.5, 19, "noguera")
    tronc(e, 25, 22, 0, 9, 4.5, 5, 10, "tela")
    tronc(e, 25, 22, 9, 13, 5, 1.5, 10, "tela")
    return e


# ---------------------------------------------------------------- mercat
def puesto(genere):
    """Parada del mercat coberta: taulell amb el gènere i un tendal de ratlles."""
    e = Escena(1, 1)
    e.caixa(3, 9, 0, 29, 23, 12, "fusta", "taulons", dalt="noguera")
    for x in (3, 27):
        e.caixa(x, 8, 0, x + 2, 10, 36, "noguera")
    e.cara([(2, 6, 36), (30, 6, 36), (30, 18, 33), (2, 18, 33)], (0, 3, 12), color_de("tela_verm", "ratlles", "y"),
           e.nova_peca(), 0.4)
    genere(e)
    return e


def _fruita(e):
    for (x, y, m) in ((8, 13, "fruit_vermell"), (15, 15, "fruit_groc"), (22, 13, "fulla"), (11, 19, "fruit_groc"),
                      (19, 19, "fruit_vermell")):
        tronc(e, x, y, 12, 15, 3.6, 1.8, 8, m)


def _peix(e):
    e.caixa(5, 11, 12, 27, 21, 13, "aigua", dalt="pedra_clara")
    for k, (x, y) in enumerate(((8, 13), (14, 15), (20, 13), (10, 18), (18, 19))):
        p = e.nova_peca()
        e.cara([(x - 3, y, 13.5), (x + 3, y - 1.5, 13.5), (x + 4.5, y, 13.5), (x + 3, y + 1.5, 13.5)], (0, 0, 1),
               color_de("peix", "llis", "z", to_fix=1 + k % 2), p, 0.3)


def _pa(e):
    for (x, y) in ((8, 13), (14, 14), (20, 13), (11, 19), (18, 19)):
        cilindre_x(e, x - 3, x + 3, y, 14.5, 2.4, "pa", tex="llis", costats=8, tapa="pa")


def _tela(e):
    for k, (y, m) in enumerate(((12, "roba2"), (16, "tela_verm"), (20, "roba3"))):
        cilindre_x(e, 5, 27, y, 14.2, 2, m, tex="llis", costats=8, tapa=m)


def _ceramica(e):
    for (x, y, r, h, m) in ((8, 14, 2.8, 8, "terra"), (15, 13, 3.4, 6, "estuc_ocre"), (22, 14, 2.4, 9, "terra"),
                            (12, 19, 2.6, 5, "estuc_blau"), (20, 19, 3, 7, "terra")):
        tronc(e, x, y, 12, 12 + h * 0.7, r * 0.8, r, 10, m)
        tronc(e, x, y, 12 + h * 0.7, 12 + h, r, r * 0.6, 10, m)


def _especies(e):
    for (x, y, m) in ((8, 14, "teula"), (14, 13, "roba3"), (20, 14, "terra"), (11, 19, "fulla"), (18, 19, "tela_verm")):
        tronc(e, x, y, 12, 15, 3, 3.2, 10, "encenall", dalt=m)
        tronc(e, x, y, 15, 16.5, 2.6, 0.8, 8, m)


def balanza():
    """Balança de braços sobre el seu peu."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 3, 5, 4, 8, "noguera")
    e.cilindre(16, 16, 3, 30, 1, "llauto", costats=6)
    e.caixa(5, 15.5, 29, 27, 16.5, 30.5, "llauto")
    for x in (7, 25):
        e.caixa(x - 0.3, 15.8, 20, x + 0.3, 16.2, 29, "ferro")
        tronc(e, x, 16, 18, 20, 2, 4, 10, "llauto", dalt="llauto")
    return e


# ---------------------------------------------------------------- ferreria i armeria
def yunque():
    """L'enclusa sobre la soca, amb el martell."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 12, 7, 6, 10, "noguera", "taulons_v", dalt="encenall")
    e.caixa(10, 13, 12, 22, 19, 15, "ferro", dalt="ferro")
    e.caixa(12, 13.5, 15, 20, 18.5, 19, "ferro")
    e.caixa(8, 12, 19, 24, 20, 22, "ferro", dalt="ferro")
    tronc(e, 24, 16, 19, 22, 4, 3, 6, "ferro")
    e.caixa(10, 6, 0, 22, 8, 2, "noguera")
    e.caixa(20, 5.5, 0, 24, 8.5, 3.5, "ferro")
    return e


def pila_agua():
    """La pica de pedra amb l'aigua per refredar el ferro."""
    e = Escena(1, 1)
    e.caixa(4, 9, 0, 28, 23, 12, "pedra_grisa", "carreus", dalt="pedra_grisa")
    e.caixa(6, 11, 12, 26, 21, 12.3, "aigua", dalt="aigua")
    e.caixa(20, 13, 12, 22, 19, 17, "ferro")
    return e


def banco_herrero():
    """Banc de treball del ferrer: tenalles, llimes i un tros de ferro roent."""
    e = Escena(1, 1)
    e.caixa(2, 8, 14, 30, 24, 17, "noguera", dalt="noguera")
    potes(e, 3, 9, 29, 23, 14, 2.5, "noguera")
    e.caixa(4, 10, 4, 28, 22, 5.5, "noguera")
    e.caixa(6, 12, 17, 18, 14, 18, "ferro")
    e.caixa(8, 17, 17, 22, 18.5, 18, "ferro")
    e.caixa(22, 11, 17, 27, 16, 18.5, "teula", dalt="foc")
    e.caixa(6, 11, 5.5, 14, 19, 9, "carbo", dalt="carbo")
    return e


def carbon():
    e = Escena(1, 1)
    e.caixa(5, 7, 0, 27, 25, 6, "noguera", "taulons", dalt="carbo")
    for (x, y, h) in ((10, 12, 6), (16, 16, 9), (22, 13, 5), (13, 20, 6), (21, 20, 7)):
        tronc(e, x, y, 6, 6 + h, 4, 0.6, 6, "carbo")
    return e


def muela():
    """La mola d'esmolar, amb el seu marc i el pedal."""
    e = Escena(1, 1)
    for x in (9, 21):
        e.caixa(x, 13, 0, x + 2, 19, 20, "noguera")
    e.caixa(8, 12, 0, 24, 20, 3, "noguera")
    cilindre_x(e, 13, 19, 16, 18, 8, "pedra_grisa", tex="llis", costats=16, tapa="pedra_grisa")
    e.caixa(7, 15.5, 17.5, 25, 16.5, 18.5, "ferro")
    return e


def armero_lanzas():
    e = Escena(1, 1)
    armes(e, 16, 16)
    return e


def armero_espadas():
    """Faristol d'espases drets, amb l'empunyadura amunt."""
    e = Escena(1, 1)
    e.caixa(5, 12, 0, 27, 20, 3, "noguera", dalt="noguera")
    e.caixa(5, 14, 16, 27, 18, 18, "noguera", dalt="noguera")
    for x in (5, 25):
        e.caixa(x, 14, 3, x + 2, 18, 18, "noguera")
    for x in (9, 13, 17, 21):
        e.caixa(x - 0.6, 15.5, 3, x + 0.6, 16.5, 26, "ferro")
        e.caixa(x - 2.5, 15, 25, x + 2.5, 17, 26.5, "llauto")
        e.caixa(x - 0.6, 15.5, 26.5, x + 0.6, 16.5, 31, "noguera")
        tronc(e, x, 16, 31, 33, 1, 0.6, 6, "llauto")
    return e


def soporte_escudos():
    """Escuts rodons i un de cometa penjats d'un faristol."""
    e = Escena(1, 1)
    for x in (6, 25):
        e.caixa(x, 14, 0, x + 2, 16, 34, "noguera")
    e.caixa(6, 14, 30, 27, 16, 32, "noguera")
    for (x, m, r) in ((11, "tela_verm", 6), (21, "roba2", 6)):
        cilindre_y(e, 16, 17.5, x, 20, r, m, tapa=m, costats=12)
        cilindre_y(e, 17.5, 18.5, x, 20, 1.8, "llauto", costats=8)
    return e


def barril_flechas():
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 14, 6, 6.5, 12, "fusta", "cercles", dalt="noguera")
    for k in range(9):
        a = k * 2 * math.pi / 9
        x, y = 16 + 3.5 * math.cos(a), 16 + 3.5 * math.sin(a)
        e.caixa(x - 0.4, y - 0.4, 14, x + 0.4, y + 0.4, 30, "noguera")
        e.caixa(x - 0.9, y - 0.4, 27, x + 0.9, y + 0.4, 31, "roba1")
    return e


def muneco():
    """Ninot d'entrenament de palla amb un escut vell."""
    e = Escena(1, 1)
    e.caixa(10, 10, 0, 22, 22, 2, "noguera")
    e.caixa(15, 15, 2, 17, 17, 46, "noguera")
    tronc(e, 16, 16, 18, 36, 6, 5, 10, "palla")
    e.caixa(5, 15, 32, 27, 17, 34, "noguera")
    tronc(e, 16, 16, 38, 46, 4, 3.5, 10, "palla", dalt="palla")
    cilindre_y(e, 21, 22.5, 12, 26, 5, "fusta", tapa="fusta")
    return e


# ---------------------------------------------------------------- banc i joieria
def caja_fuerte():
    """Caixa forta de ferro amb la roda de la combinació."""
    e = Escena(1, 1)
    e.caixa(6, 7, 0, 26, 25, 30, "ferro", dalt="ferro")
    e.caixa(7, 25, 2, 25, 26, 28, "ferro")
    cilindre_y(e, 26, 27.5, 16, 17, 4, "llauto", tapa="llauto", costats=12)
    for z in (6, 24):
        e.caixa(8, 25.5, z, 11, 26.5, z + 2, "llauto")
    return e


def oro_pila():
    """Lingots i monedes d'or (la cambra cuirassada)."""
    e = Escena(1, 1)
    for (x, y, z) in ((8, 10, 0), (16, 10, 0), (12, 16, 0), (20, 16, 0), (12, 10, 3), (16, 16, 3)):
        e.caixa(x, y, z, x + 7, y + 4, z + 3, "or", dalt="or")
    for (x, y, n) in ((8, 24, 5), (14, 25, 3), (24, 23, 6)):
        for k in range(n):
            e.cilindre(x, y, k * 0.9, k * 0.9 + 0.9, 2.4, "or", costats=8)
    return e


def sacos_monedas():
    e = Escena(1, 1)
    for (x, y, r) in ((11, 13, 5), (21, 14, 5.5), (16, 21, 5)):
        tronc(e, x, y, 0, 7, r, r + 0.5, 10, "tela")
        tronc(e, x, y, 7, 11, r + 0.5, 1.5, 10, "tela")
        tronc(e, x, y, 11, 12.5, 1.6, 2.2, 8, "or")
    for (x, y) in ((6, 22), (25, 24)):
        e.cilindre(x, y, 0, 0.8, 2, "or", costats=8)
    return e


def vidre_obert(e, x0, y0, x1, y1, z0, z1, marc="llauto", top=True):
    """Caixa de vidre que deixa veure el que hi ha a dins: els cantells del marc
    i, a les cares de davant (+Y i +X), dos reflexos en diagonal."""
    for (x, y) in ((x0, y1), (x1, y1), (x1, y0)):
        e.caixa(x - 0.6, y - 0.6, z0, x + 0.6, y + 0.6, z1, marc)
    if top:
        e.caixa(x0, y1 - 0.6, z1 - 1, x1, y1 + 0.6, z1, marc, dalt=marc)
        e.caixa(x1 - 0.6, y0, z1 - 1, x1 + 0.6, y1, z1, marc, dalt=marc)
    h = z1 - z0
    for (a, b) in ((0.25, 0.42), (0.52, 0.6)):
        u0, u1 = x0 + (x1 - x0) * a, x0 + (x1 - x0) * b
        e.cara([(u0, y1 + 0.1, z0 + 1), (u0 + 1.4, y1 + 0.1, z0 + 1), (u1 + 1.4, y1 + 0.1, z1 - 1), (u1, y1 + 0.1, z1 - 1)],
               (0, 1, 0), color_de("vidre", "llis", "y", to_fix=0), e.nova_peca(), 0.8)
        v0, v1 = y0 + (y1 - y0) * a, y0 + (y1 - y0) * b
        e.cara([(x1 + 0.1, v0, z0 + 1), (x1 + 0.1, v0 + 1.4, z0 + 1), (x1 + 0.1, v1 + 1.4, z1 - 1), (x1 + 0.1, v1, z1 - 1)],
               (1, 0, 0), color_de("vidre", "llis", "x", to_fix=0), e.nova_peca(), 0.8)
    del h


def vitrina(tipus="joies"):
    """Vitrina baixa: moble de noguera amb la caixa de vidre i el que s'hi mostra."""
    e = Escena(1, 1)
    e.caixa(4, 7, 0, 28, 25, 14, "noguera", "plafons", dalt="vellut")
    if tipus == "joies":
        for (x, y, m) in ((8, 11, "cristall"), (14, 13, "tela_verm"), (20, 11, "porticons"), (11, 19, "roba2"),
                          (19, 19, "cristall")):
            tronc(e, x, y, 14, 15.5, 1.6, 0, 6, m)
        from mobles_base import anell as _anell
        _anell(e, 24, 18, 14, 14.8, 2.5, 1.6, 10, "or")
        _anell(e, 9, 21, 14, 14.8, 2.2, 1.4, 10, "or")
    else:
        for (x, y) in ((10, 13), (20, 17)):
            tronc(e, x, y, 14, 18, 2, 2.4, 8, "vidre")
    vidre_obert(e, 4, 7, 28, 25, 14, 23)
    return e


def vitrina_alta():
    """Vitrina alta de vidre amb collarets i corones."""
    e = Escena(1, 1)
    e.caixa(5, 8, 0, 27, 22, 8, "noguera", dalt="noguera")
    e.caixa(5, 8, 52, 27, 22, 56, "noguera", dalt="noguera")
    for x in (5, 25.5):
        e.caixa(x, 8, 8, x + 1.5, 22, 52, "noguera")
    for z in (24, 38):
        e.caixa(6.5, 9, z, 25.5, 21, z + 1, "vidre", dalt="vellut")
    from mobles_base import anell as _anell
    _anell(e, 16, 15, 39, 43, 4, 3, 10, "or")
    tronc(e, 16, 15, 43, 46, 1.4, 0, 4, "cristall")
    for x in (11, 21):
        _anell(e, x, 15, 25, 25.6, 3, 2.4, 10, "or")
        tronc(e, x, 18.2, 22, 24, 1, 0, 4, "tela_verm")
    vidre_obert(e, 5, 8, 27, 22, 8, 52, marc="noguera", top=False)
    return e


def busto_joyas():
    """Bust de vellut amb un collaret d'or i cristall, sobre una columneta."""
    e = Escena(1, 1)
    e.caixa(10, 10, 0, 22, 22, 3, "pedra_clara", dalt="pedra_clara")
    e.cilindre(16, 16, 3, 24, 3.5, "pedra_clara", costats=8)
    e.caixa(9, 9, 24, 23, 23, 27, "pedra_clara", dalt="pedra_clara")
    tronc(e, 16, 16, 27, 36, 6, 4.5, 10, "vellut")
    e.cilindre(16, 16, 36, 40, 2, "vellut", costats=8)
    tronc(e, 16, 16, 40, 46, 3.6, 3, 10, "vellut", dalt="vellut")
    from mobles_base import anell as _anell
    _anell(e, 16, 16, 34, 35, 5.6, 4.6, 12, "or")
    tronc(e, 16, 21.5, 31, 34, 1.6, 0, 4, "cristall")
    return e


# ---------------------------------------------------------------- sastreria
def maniqui(roba="tela_verm", faldilla=True):
    """Maniquí de modista amb un vestit a mig fer."""
    e = Escena(1, 1)
    for a in (0.4, 2.5, 4.6):
        x, y = 16 + 5 * math.cos(a), 16 + 5 * math.sin(a)
        e.caixa(min(16, x) - 0.6, min(16, y) - 0.6, 0, max(16, x) + 0.6, max(16, y) + 0.6, 2, "noguera")
    e.cilindre(16, 16, 0, 22, 1, "noguera", costats=6)
    if faldilla:
        tronc(e, 16, 16, 14, 26, 8, 5, 12, roba, "plecs")
    tronc(e, 16, 16, 26, 36, 4.5, 5.6, 12, roba)
    tronc(e, 16, 16, 36, 40, 5.6, 2.5, 12, roba)
    e.cilindre(16, 16, 40, 43, 1.4, "noguera", costats=6)
    tronc(e, 16, 16, 43, 45, 2, 0.6, 6, "noguera")
    return e


def rollos_tela():
    """Faristol amb peces de roba de colors enrotllades."""
    e = Escena(1, 1)
    for x in (4, 26):
        e.caixa(x, 10, 0, x + 2, 22, 30, "noguera")
    for (z, y, m) in ((6, 13, "roba2"), (6, 19, "tela_verm"), (16, 13, "roba3"), (16, 19, "porticons"),
                      (26, 16, "llana_lila")):
        cilindre_x(e, 6, 26, y, z, 3.4, m, tex="plecs", costats=10, tapa="roba1")
    return e


def mesa_corte():
    """La taula de tallar: roba estesa, les tisores i la cinta de mesurar."""
    e = Escena(1, 1)
    e.caixa(2, 5, 14, 30, 27, 16.5, "fusta", "taulons", dalt="fusta")
    potes(e, 3, 6, 29, 26, 14, 2.5, "noguera")
    e.caixa(4, 8, 16.5, 22, 24, 17, "roba2", dalt="roba2")
    for d in (-1, 1):
        p = e.nova_peca()
        e.cara([(23, 12, 17.2), (28, 12 + 3 * d, 17.2), (28.5, 12.6 + 3 * d, 17.2), (23.4, 12.8, 17.2)], (0, 0, 1),
               color_de("ferro", "llis", "z", to_fix=1), p, 0.3)
    e.caixa(5, 22, 17, 27, 23, 17.4, "roba3", dalt="roba3")
    return e


def rueca():
    """El torn de filar: la roda, la filosa i el pedal."""
    e = Escena(1, 1)
    e.caixa(6, 13, 4, 26, 19, 7, "noguera", dalt="noguera")
    for (x, y) in ((7, 14), (24, 14), (16, 18)):
        e.caixa(x, y, 0, x + 2, y + 2, 4, "noguera")
    for x in (14, 18):
        e.caixa(x, 15, 7, x + 1.5, 17, 26, "noguera")
    from mobles_base import anell as _anell
    p = e.nova_peca()
    pts = [(16 + 9 * math.cos(a), 20 + 9 * math.sin(a)) for a in [k * math.pi / 8 for k in range(16)]]
    for k in range(16):
        (xa, za), (xb, zb) = pts[k], pts[(k + 1) % 16]
        e.cara([(xa, 15.8, za), (xb, 15.8, zb), (xb, 17.2, zb), (xa, 17.2, za)], ((xa + xb) / 2 - 16, 0, (za + zb) / 2 - 20),
               color_de("fusta", "llis", "x"), p)
    e.cara([(x, 17.2, z) for x, z in pts], (0, 1, 0), color_de("fusta", "llis", "y", to_fix=4), p, -0.3)
    for k in range(0, 16, 4):
        xa, za = pts[k]
        e.caixa(min(16, xa) - 0.3, 16.2, min(20, za) - 0.3, max(16, xa) + 0.3, 16.8, max(20, za) + 0.3, "noguera")
    e.caixa(22, 15, 7, 23.5, 16.5, 30, "noguera")
    tronc(e, 22.7, 15.7, 22, 28, 2, 1, 8, "llana_lila")
    del _anell
    return e


def perchero_ropa():
    """Barra de penjar amb vestits de colors."""
    e = Escena(1, 1)
    for x in (3, 27):
        e.caixa(x, 15, 0, x + 2, 17, 44, "noguera")
        e.caixa(x - 2, 13, 0, x + 4, 19, 2, "noguera")
    e.caixa(3, 15.4, 42, 29, 16.6, 43.5, "llauto")
    for k, (x, m) in enumerate(((7, "roba2"), (12, "tela_verm"), (17, "porticons"), (22, "roba3"))):
        tronc(e, x + 2, 16, 18, 41, 4.5, 3, 4, m, gir=math.pi / 4)
    return e


# ---------------------------------------------------------------- casa de te
def mesa_baja():
    """Taula baixa amb la tetera fumejant i les tasses."""
    e = Escena(1, 1)
    e.caixa(5, 5, 6, 27, 27, 8.5, "noguera", dalt="noguera")
    potes(e, 6, 6, 26, 26, 6, 2.5, "noguera")
    tronc(e, 16, 16, 8.5, 13, 3.6, 4, 10, "estuc_blau")
    tronc(e, 16, 16, 13, 15, 4, 1.2, 10, "estuc_blau")
    e.caixa(19, 15.5, 10, 22, 16.5, 11, "estuc_blau")
    for (x, y) in ((9, 10), (23, 22), (10, 22)):
        tassa(e, x, y, 8.5, "roba1")
    from peces_iso import fum
    e.sprite((16, 16, 16), fum(5, 2), despres=True)
    return e


def cojin(mat="vellut"):
    e = Escena(1, 1)
    coixi(e, 8, 8, 24, 24, 0, 4, mat)
    return e


def farolillo():
    """Fanal de paper sobre un peu, encès."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 2, 5, 4, 8, "noguera")
    e.cilindre(16, 16, 2, 26, 0.9, "noguera", costats=6)
    e.caixa(11, 11, 26, 21, 21, 27, "noguera")
    tronc(e, 16, 16, 27, 39, 6, 6, 8, "paper_cel", "taulons", dalt="noguera")
    e.caixa(11, 11, 39, 21, 21, 40, "noguera", dalt="noguera")
    return e


def biombo_papel():
    from mobles_comuns import biombo
    return biombo("paper_cel", "noguera")


def bonsai():
    e = Escena(1, 1)
    e.caixa(8, 10, 0, 24, 22, 4, "estuc_blau", dalt="terra")
    tronc(e, 16, 16, 4, 12, 1.2, 0.8, 6, "noguera")
    e.caixa(15, 15, 10, 21, 16.4, 11, "noguera")
    e.sprite((14, 16, 13), mata(5, 4, flors=("fulla",), llavor=17))
    e.sprite((20, 16, 12), mata(4, 3, flors=("fulla",), llavor=18))
    return e


# ---------------------------------------------------------------- banys
def banco_piedra():
    e = Escena(1, 1)
    e.caixa(2, 10, 8, 30, 22, 11, "pedra_clara", dalt="pedra_clara")
    for x in (4, 24):
        e.caixa(x, 11, 0, x + 4, 21, 8, "pedra_clara")
    return e


def estante_toallas():
    """Prestatge amb tovalloles plegades i sabó."""
    e = Escena(1, 1)
    e.caixa(4, 6, 0, 28, 8, 40, "noguera")
    for x in (4, 26):
        e.caixa(x, 6, 0, x + 2, 18, 40, "noguera", dalt="noguera")
    for z in (2, 14, 26):
        e.caixa(6, 8, z, 26, 18, z + 1.5, "noguera", dalt="noguera")
        for k, m in enumerate(("roba1", "estuc_blau", "roba1")):
            e.caixa(7 + k * 6.3, 9, z + 1.5, 12.5 + k * 6.3, 17, z + 4.5, m, dalt=m)
    e.caixa(8, 9, 39, 26, 17, 40, "noguera")
    return e


def estatua_fuente():
    """La font de la nimfa: una figura amb túnica que aboca aigua d'una gerra a la pica."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 6, 13, 13, 12, "pedra_clara", dalt="aigua")
    tronc(e, 16, 16, 6, 7.5, 13, 13, 12, "pedra_clara", dalt="pedra_clara")
    e.caixa(12, 9, 0, 20, 17, 12, "pedra_clara", "carreus", dalt="pedra_clara")
    tronc(e, 16, 13, 12, 30, 5, 3.4, 10, "pedra_clara", "plecs")              # la túnica
    tronc(e, 16, 13, 30, 34, 3.6, 3, 10, "pedra_clara")                        # les espatlles
    e.cilindre(16, 13, 34, 36, 1.3, "pedra_clara", costats=6)                  # el coll
    tronc(e, 16, 13, 36, 41, 2.6, 2.2, 8, "pedra_clara", dalt="pedra_clara")   # el cap
    e.caixa(14, 11, 39, 18, 13, 42, "pedra_grisa", dalt="pedra_grisa")        # el cabell recollit
    e.caixa(18, 13, 29, 21, 15, 31, "pedra_clara")                             # el braç
    cilindre_x(e, 19, 24, 16, 30, 2.4, "pedra_clara", tex="llis", costats=8, tapa="pedra_grisa")   # la gerra
    for k in range(5):                                                         # el raig d'aigua
        e.caixa(23.5 + k * 0.5, 16.5 + k * 0.6, 26 - k * 4, 24.5 + k * 0.5, 17.5 + k * 0.6, 29 - k * 4, "aigua",
                dalt="aigua")
    return e


def brasero_piedras():
    """El braser de pedres calentes de la sala de vapor."""
    e = brasero_base()
    from peces_iso import fum
    e.sprite((16, 16, 20), fum(9, 3), despres=True)
    return e


def brasero_base():
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 12, 9, 10, 12, "pedra_grisa", "carreus", dalt="ferro")
    for (x, y) in ((12, 13), (19, 13), (15, 19), (21, 19), (10, 19)):
        tronc(e, x, y, 12, 15, 3, 1.5, 6, "teula")
    return e


# ---------------------------------------------------------------- el registre
# Els taulells van en trams al llarg de X (ini, mig, fi) i un de sol: el
# contorn no talla la juntura entre trams i el del final tanca el taulell.
for _id, _coses, _nom, _k in (("barra_taberna", None, "Barra de la taberna", {}),
                              ("barra_taberna_b", (_gerres,), "Barra con jarras", {}),
                              ("barra_taberna_c", (_ampolles,), "Barra con botellas", {}),
                              ("mostrador_posada", (_registre,), "Mostrador de la posada", {}),
                              ("mostrador", None, "Mostrador", {}),
                              ("mostrador_banco", (reixa_banc, _monedes), "Mostrador del banco",
                               {"frontal": "noguera", "tauler": "pedra_clara"}),
                              ("mostrador_joyero", (_lupa,), "Mostrador del joyero",
                               {"frontal": "noguera", "tauler": "vellut"}),
                              ("mostrador_armeria", (_espasa,), "Mostrador de la armería", {})):
    _fer = (lambda c=_coses, k=_k: barra(c, **k))
    _vei = (lambda k=_k: barra(None, **k))
    R.trams_de(_id, {"ini": amb_veins(_fer, [(_vei, 1, 0)]), "mig": amb_veins(_fer, [(_vei, -1, 0), (_vei, 1, 0)]),
                     "fi": amb_veins(_fer, [(_vei, -1, 0)]), "sol": _fer}, _nom)
R.afegeix("estanteria_botellas", estanteria_botellas, "Estantería de botellas", gira=True)
R.afegeix("asador", asador, "Asador")
R.afegeix("equipaje", equipaje, "Equipaje", gira=True)
for _id, _g, _nom in (("puesto_fruta", _fruita, "Puesto de fruta"), ("puesto_pescado", _peix, "Puesto de pescado"),
                      ("puesto_pan", _pa, "Puesto de pan"), ("puesto_telas", _tela, "Puesto de telas"),
                      ("puesto_ceramica", _ceramica, "Puesto de cerámica"),
                      ("puesto_especias", _especies, "Puesto de especias")):
    R.afegeix(_id, (lambda g=_g: puesto(g)), _nom, gira=True)
R.afegeix("balanza", balanza, "Balanza")
R.afegeix("yunque", yunque, "Yunque", gira=True)
R.afegeix("pila_agua", pila_agua, "Pila de templar", gira=True)
R.afegeix("banco_herrero", banco_herrero, "Banco de herrero", gira=True)
R.afegeix("carbon", carbon, "Carbón")
R.afegeix("muela", muela, "Muela de afilar", gira=True)
R.afegeix("armero_lanzas", armero_lanzas, "Armero de lanzas", gira=True)
R.afegeix("armero_espadas", armero_espadas, "Armero de espadas", gira=True)
R.afegeix("soporte_escudos", soporte_escudos, "Soporte de escudos", gira=True)
R.afegeix("barril_flechas", barril_flechas, "Barril de flechas")
R.afegeix("muneco", muneco, "Muñeco de entrenamiento", gira=True)
R.afegeix("caja_fuerte", caja_fuerte, "Caja fuerte", gira=True)
R.afegeix("oro_pila", oro_pila, "Oro")
R.afegeix("sacos_monedas", sacos_monedas, "Sacos de monedas")
R.afegeix("vitrina", lambda: vitrina(), "Vitrina de joyas", gira=True)
R.afegeix("vitrina_frascos", lambda: vitrina("frascos"), "Vitrina de frascos", gira=True)
R.afegeix("vitrina_alta", vitrina_alta, "Vitrina alta", gira=True)
R.afegeix("busto_joyas", busto_joyas, "Busto con collar")
R.afegeix("maniqui", lambda: maniqui(), "Maniquí", gira=True)
R.afegeix("maniqui_azul", lambda: maniqui("roba2"), "Maniquí (vestido azul)", gira=True)
R.afegeix("maniqui_jubon", lambda: maniqui("porticons", faldilla=False), "Maniquí (jubón)", gira=True)
R.afegeix("rollos_tela", rollos_tela, "Rollos de tela", gira=True)
R.afegeix("mesa_corte", mesa_corte, "Mesa de corte", gira=True)
R.afegeix("rueca", rueca, "Rueca", gira=True)
R.afegeix("perchero_ropa", perchero_ropa, "Perchero de ropa", gira=True)
R.afegeix("mesa_baja", mesa_baja, "Mesa de té")
R.afegeix("cojin", lambda: cojin(), "Cojín")
R.afegeix("cojin_verde", lambda: cojin("llana_verda"), "Cojín verde")
R.afegeix("farolillo", farolillo, "Farolillo")
R.afegeix("biombo_papel", biombo_papel, "Biombo de papel", gira=True)
R.afegeix("bonsai", bonsai, "Bonsái")
R.afegeix("banco_piedra", banco_piedra, "Banco de piedra", gira=True)
R.afegeix("estante_toallas", estante_toallas, "Estante de toallas", gira=True)
R.afegeix("estatua_fuente", estatua_fuente, "Fuente de la ninfa")
R.afegeix("brasero_piedras", brasero_piedras, "Brasero de piedras")
