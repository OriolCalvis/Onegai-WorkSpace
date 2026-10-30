#!/usr/bin/env python3
"""ISO — Els patis de davant de cada casa segons el barri: el que fa que un
carrer sembli viscut (Stardew). Mai davant de la porta.

    PATIS = {"casa", "jardi", "pescador", "armes", "cap"}
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from peces_iso import color_de, mata  # noqa: E402


def llocs(c, pas=12, marge=16):
    out = [x for x in range(int(c["bx0"]) + 6, int(c["bx1"]) - 4, pas) if abs(x - c["pc"]) > marge]
    c["rnd"].shuffle(out)
    return out


def bota(e, x, y, z=0, alt=13):
    e.cilindre(x, y, z, z + alt, 4.5, "fusta", "cercles", dalt="noguera")


def test_flors(e, x, y, llavor, mat="terra"):
    e.cilindre(x, y, 0, 5, 3.5, mat)
    e.sprite((x, y, 5), mata(5, 5, llavor=llavor))


def boix(e, x, y, llavor):
    """Boix retallat en test (jardins nobles)."""
    e.caixa(x - 4, y - 4, 0, x + 4, y + 4, 6, "pedra_clara", dalt="terra")
    e.sprite((x, y, 6), mata(5, 6, flors=("boix",), llavor=llavor))


def llenya(e, x, y):
    e.caixa(x - 7, y - 3, 0, x + 7, y + 3, 9, "noguera", "llenya", dalt="noguera")


def xarxa(e, x, y):
    """Xarxa de pesca estesa entre dos pals."""
    for xx in (x - 9, x + 7):
        e.caixa(xx, y - 1, 0, xx + 2, y + 1, 20, "noguera")
    e.cara([(x - 7, y, 6), (x + 7, y, 6), (x + 7, y, 19), (x - 7, y, 19)], (0, 1, 0),
           color_de("tela", "xarxa", "y", desp=1), e.nova_peca(), 0.0, True)


def caixes(e, x, y):
    e.caixa(x - 5, y - 4, 0, x + 5, y + 4, 8, "fusta", "taulons")
    e.caixa(x - 3, y - 3, 8, x + 4, y + 3, 14, "fusta", "taulons")


def armes(e, x, y):
    """Faristol de llances i un escut."""
    for xx in (x - 7, x + 6):
        e.caixa(xx, y - 1, 0, xx + 2, y + 1, 16, "noguera")
    e.caixa(x - 7, y - 1, 14, x + 8, y + 1, 16, "noguera")
    for k in range(4):
        xx = x - 4 + k * 3
        e.caixa(xx, y - 0.5, 2, xx + 1, y + 0.5, 26, "noguera")
        e.caixa(xx - 0.5, y - 1, 26, xx + 1.5, y + 1, 30, "ferro")


def estendard(e, x, y, alt=34):
    """Pal amb l'estendard de la ciutat (vermell, banda blanca i ganxos d'or)."""
    e.caixa(x, y, 0, x + 2, y + 2, alt, "ferro")
    e.cara([(x + 2, y + 1, alt - 14), (x + 12, y + 1, alt - 14), (x + 12, y + 1, alt - 1), (x + 2, y + 1, alt - 1)],
           (0, 1, 0), color_de("tela_verm", "llis", "y"), e.nova_peca(), 0.0, True)
    e.cara([(x + 2, y + 1, alt - 9), (x + 12, y + 1, alt - 3), (x + 12, y + 1, alt - 1), (x + 2, y + 1, alt - 7)],
           (0, 1, 0), color_de("tela", "llis", "y", to_fix=0), e.nova_peca(), 0.3, True)


def casa(e, c):
    ll = llocs(c)
    for tipus in ("bota", "test", "mata", "mata"):
        if not ll:
            break
        x = ll.pop()
        if tipus == "bota":
            bota(e, x, c["yp"])
        elif tipus == "test":
            test_flors(e, x, c["yp"], c["llavor"] + 40)
        else:
            e.sprite((x, c["yp"], 0), mata(5, 5, llavor=c["llavor"] + 50 + x))


def jardi(e, c):
    for dx in (-18, 18):                                        # dos boixos flanquejant la porta
        boix(e, c["pc"] + dx, c["yp"], c["llavor"] + dx)
    for x in llocs(c, marge=26)[:2]:
        test_flors(e, x, c["yp"], c["llavor"] + x, mat="pedra_clara")


def pescador(e, c):
    ll = llocs(c)
    for tipus in ("xarxa", "llenya", "bota"):
        if not ll:
            break
        x = ll.pop()
        {"xarxa": xarxa, "llenya": llenya, "bota": bota}[tipus](e, x, c["yp"])


def armes_pati(e, c):
    ll = llocs(c)
    if ll:
        armes(e, ll.pop(), c["yp"])
    if ll:
        estendard(e, ll.pop(), c["yp"])


def cap(e, c):
    pass


PATIS = {"casa": casa, "jardi": jardi, "pescador": pescador, "armes": armes_pati, "cap": cap}
