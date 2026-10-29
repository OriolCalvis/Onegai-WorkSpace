#!/usr/bin/env python3
"""INT-02 — Els mobles amb el toc de cada nació de l'est (Oriol, 05/10/2026:
«amueblamos las zonas», amb «un toque por nación»):

  Nocturnsea  és la porta de la casa de la mort i hi atenen els morts: espelmes
              foses per terra i taüts en lloc de llits (a la Posada del Faro).
  Choubar     els homes ocell del riu: perxes on s'ajupen en lloc de cadires,
              rotlles de corda i la taula de cartes de nòlit.
  Mistarium   les illes de boira i les corts de miralls: el mirall de cort i els
              cristalls de boira.
  Gongorguma  els clans orcs del consell i el teatre: el tambor de guerra i el
              tòtem d'ossos.

Les decoracions de paret de cada nació (teranyines, miralls, màscares de teatre i
cordes) són a sales.py. Com la resta del kit: una cel·la cadascun, la cara de
davant a +Y (o0), i els que tenen sentit girats surten en 4 orientacions.
"""
import math

import numpy as np

from mobles_base import (MATERIALS, Escena, Registre, cilindre_x, color_de, espelma, potes,  # noqa: F401
                         rng_de, tronc)

MATERIALS.update({
    "fusta_morta": [(150, 140, 136), (118, 108, 106), (92, 84, 84), (68, 62, 64), (46, 42, 46)],
    "corda": [(236, 214, 160), (212, 184, 124), (182, 152, 96), (146, 118, 72), (108, 86, 54)],
    "cristall_lila": [(255, 240, 255), (232, 200, 250), (196, 150, 232), (150, 106, 200), (104, 70, 150)],
    "argent": [(250, 250, 255), (214, 216, 228), (176, 178, 196), (136, 138, 158), (96, 98, 118)],
    "boira": [(250, 244, 255), (236, 226, 250), (218, 204, 240), (196, 180, 226), (170, 152, 206)],
    "os": [(255, 250, 236), (240, 230, 206), (214, 200, 170), (178, 162, 130), (136, 122, 96)],
    "pell": [(250, 232, 200), (236, 210, 170), (212, 182, 138), (176, 146, 104), (134, 108, 76)],
})

R = Registre()


def boira(llavor=1, bufs=3):
    """Boira lila que puja del cristall (sense contorn, com el fum de les xemeneies)."""
    def f(ll, sx, sy, prof):
        rng = np.random.default_rng(abs(int(llavor)) % (2 ** 32))
        rampa = np.array(MATERIALS["boira"], np.uint8)
        for k in range(bufs):
            r = 3 + k
            cx, cy = sx + int(rng.integers(-3, 4)), sy - 3 - k * 6
            for yy in range(cy - r, cy + r + 1):
                for xx in range(cx - r - 1, cx + r + 2):
                    if not (0 <= xx < ll.w and 0 <= yy < ll.h):
                        continue
                    d = ((xx - cx) ** 2 / 1.4 + (yy - cy) ** 2) / (r * r)
                    if d > 1:
                        continue
                    to = 1 if (xx - cx) + (yy - cy) < -r * 0.4 else (3 if d > 0.6 and (xx - cx) > 0 else 2)
                    ll.col[yy, xx] = (*rampa[min(4, to + k // 2)], 210 - 40 * k)
    return f


# ---------------------------------------------------------------- Nocturnsea
def velas_derretidas(llavor=7):
    """Espelmes foses per terra, amb els tolls de cera: la llum dels morts."""
    e = Escena(1, 1)
    rng = rng_de(llavor)
    for (x, y, r) in ((11, 12, 6), (21, 18, 5), (14, 22, 4)):
        tronc(e, x, y, 0, 0.8, r, r - 1, 10, "cera", dalt="cera")
    for (x, y, alt) in ((10, 11, 9), (13, 14, 5), (20, 17, 12), (23, 20, 4), (15, 22, 7), (8, 15, 3)):
        r = 1.3 + rng.random() * 0.6
        e.cilindre(x, y, 0.8, 0.8 + alt, r, "cera", costats=6)
        tronc(e, x, y, 0.8 + alt - 1, 0.8 + alt + 0.5, r + 0.5, r - 0.2, 6, "cera", dalt="cera")
        tronc(e, x, y, 0.8 + alt + 0.5, 0.8 + alt + 3.5, 0.8, 0.1, 4, "foc")
    return e


def ataud(tram):
    """Taüt de fusta morta en dos trams al llarg de Y (cap i peus): els llits de la
    Posada del Faro, on dormen els que ja no es lleven."""
    e = Escena(1, 1)
    y0, y1 = (6, 32) if tram == "cap" else (0, 26)
    if tram == "cap":
        # l'espatlla: el taüt s'eixampla abans de tancar-se cap al cap
        e.caixa(8, y0, 0, 24, y0 + 4, 11, "fusta_morta", "taulons_v", dalt="fusta_morta")
        e.caixa(6, y0 + 4, 0, 26, y1, 11, "fusta_morta", "taulons_v", dalt="fusta_morta")
        e.caixa(7, y0 + 5, 11, 25, y1, 13, "noguera", dalt="noguera")              # la tapa, una mica desplaçada
        e.caixa(15, y0 + 8, 13, 17, y0 + 20, 14, "llauto")                           # la creu de llautó
        e.caixa(12, y0 + 12, 13, 20, y0 + 14, 14, "llauto")
        espelma(e, 26, y0 + 2, 0, 6)
    else:
        e.caixa(7, y0, 0, 25, y1 - 4, 11, "fusta_morta", "taulons_v", dalt="fusta_morta")
        e.caixa(9, y1 - 4, 0, 23, y1, 11, "fusta_morta", "taulons_v", dalt="fusta_morta")
        e.caixa(8, y0, 11, 24, y1 - 5, 13, "noguera", dalt="noguera")
    return e


# ---------------------------------------------------------------- Choubar
def percha():
    """Perxa d'homes ocell: un pal amb la barra per ajupir-s'hi (fa de cadira)."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 3, 6, 5, 8, "noguera", dalt="noguera")
    e.cilindre(16, 16, 3, 28, 1.4, "noguera", costats=6)
    e.caixa(5, 15, 28, 27, 17.4, 30.5, "fusta", dalt="fusta")
    e.caixa(13, 14.6, 27.5, 19, 17.8, 31, "corda", dalt="corda")                      # l'agafador de corda
    for x in (5, 26):
        e.caixa(x, 14.6, 27.5, x + 1.5, 17.8, 31.5, "noguera", dalt="noguera")
    return e


def rollo_cuerda():
    """Rotlles de corda dels nòlits del riu, l'un damunt de l'altre."""
    from monuments import anell
    e = Escena(1, 1)
    for k, (r, z) in enumerate(((11, 0), (9.5, 2.6), (8, 5.2))):
        anell(e, 16, 16, z, z + 2.6, r, r - 3.2, 14, "corda", gir=k * 0.3)
    e.caixa(21, 4, 0, 23, 16, 1.6, "corda", dalt="corda")                               # la punta, per terra
    return e


def mesa_cartas():
    """La taula de les cartes de nòlit: el mapa del riu, la brúixola, el compàs i
    els rotlles dels contractes."""
    e = Escena(1, 1)
    potes(e, 3, 6, 29, 26, 15, g=2, mat="noguera")
    e.caixa(2, 5, 15, 30, 27, 17, "noguera", dalt="noguera")
    e.caixa(5, 8, 17, 25, 23, 17.6, "pergami", dalt="pergami")
    e.caixa(8, 12, 17.6, 15, 13, 17.8, "aigua", dalt="aigua")                          # el riu, pintat
    e.caixa(14, 12, 17.6, 15, 20, 17.8, "aigua", dalt="aigua")
    e.cilindre(21, 11, 17.6, 18.6, 2.6, "llauto", costats=10)                          # la brúixola
    e.caixa(20.5, 10.5, 18.6, 21.5, 11.5, 19, "tela_verm")
    cilindre_x(e, 6, 13, 24.5, 18.8, 1.4, "pergami", tex="llis", costats=8, tapa="pergami")
    cilindre_x(e, 16, 26, 25, 18.8, 1.4, "pergami", tex="llis", costats=8, tapa="pergami")
    return e


# ---------------------------------------------------------------- Mistarium
def espejo_corte():
    """Mirall de cort de Mistarium: alt, de marc de plata amb gemmes lila."""
    e = Escena(1, 1)
    for x in (6, 24):
        e.caixa(x, 13, 0, x + 3, 19, 3, "argent", dalt="argent")
        e.caixa(x + 0.8, 15, 3, x + 2.2, 17, 50, "argent")
    e.caixa(8, 15, 6, 25, 17, 60, "argent", dalt="argent")
    e.rect("y", 17, 1, 9.5, 23.5, 8, 58, "vidre", to=1, biaix=0.4)
    e.rect("y", 17.1, 1, 10.5, 14, 14, 52, "vidre", to=0, biaix=0.6)                  # el reflex
    e.rect("y", 17.2, 1, 9.5, 23.5, 30, 34, "cristall_lila", to=2, biaix=0.5)        # la boira que s'hi veu
    for (x, z) in ((16.5, 61), (8.5, 34), (24.5, 34)):
        tronc(e, x, 16, z, z + 3, 1.6, 0, 4, "cristall_lila")
    tronc(e, 16.5, 16, 60, 66, 3, 0, 6, "argent")
    return e


def cristal_bruma():
    """Cristalls lila de les illes de boira, sobre una roca, i la boira que en surt."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 5, 10, 7, 7, "pedra_fosca", dalt="pedra_fosca")
    for (dx, dy, alt, r, gir) in ((0, 0, 30, 4, 0.2), (-5, 3, 18, 3, 0.9), (5, -2, 22, 3, 0.5),
                                  (3, 5, 12, 2.4, 1.3), (-4, -4, 14, 2.4, 0.1)):
        tronc(e, 16 + dx, 16 + dy, 4, 4 + alt * 0.75, r, r * 0.8, 6, "cristall_lila", gir=gir)
        tronc(e, 16 + dx, 16 + dy, 4 + alt * 0.75, 4 + alt, r * 0.8, 0, 6, "cristall_lila", gir=gir)
    e.sprite((16, 16, 36), boira(5, 3), despres=True)
    return e


# ---------------------------------------------------------------- Gongorguma
def tambor_guerra():
    """El tambor de guerra dels clans, de pell i fusta, amb les dues mases."""
    e = Escena(1, 1)
    for a in (0.5, 2.6, 4.7):
        x, y = 16 + 9 * math.cos(a), 16 + 9 * math.sin(a)
        e.caixa(x - 1, y - 1, 0, x + 1, y + 1, 6, "noguera")
    tronc(e, 16, 16, 5, 24, 10, 11, 14, "noguera", "taulons_v")
    tronc(e, 16, 16, 24, 26, 11.5, 11.5, 14, "tela_verm")
    tronc(e, 16, 16, 26, 26.6, 11, 11, 14, "pell", dalt="pell")
    for k in range(7):                                                                 # les cordes en ziga-zaga
        a = k * 2 * math.pi / 7
        x, y = 16 + 10.6 * math.cos(a), 16 + 10.6 * math.sin(a)
        e.caixa(x - 0.5, y - 0.5, 7, x + 0.5, y + 0.5, 24, "corda")
    cilindre_x(e, 9, 21, 13, 27.8, 0.9, "noguera", tex="llis", costats=6, tapa="noguera")
    cilindre_x(e, 12, 24, 19, 27.8, 0.9, "noguera", tex="llis", costats=6, tapa="noguera")
    e.cilindre(21.5, 13, 27, 28.8, 1.8, "pell", costats=6)
    e.cilindre(24.5, 19, 27, 28.8, 1.8, "pell", costats=6)
    return e


def totem_huesos():
    """El tòtem dels clans: un pal amb el crani banyut d'una bèstia, ossos penjant i
    les plomes del clan."""
    e = Escena(1, 1)
    tronc(e, 16, 16, 0, 4, 7, 5, 8, "pedra_fosca", dalt="pedra_fosca")
    e.cilindre(16, 16, 4, 52, 2, "noguera", costats=6)
    for z in (20, 34):
        e.caixa(13.5, 13.5, z, 18.5, 18.5, z + 2, "tela_verm", dalt="tela_verm")
    tronc(e, 16, 16, 50, 58, 5, 4.4, 8, "os", dalt="os")                              # el crani
    e.caixa(13, 18, 53, 15, 20.5, 55.5, "carbo")                                        # les òrbites
    e.caixa(17, 18, 53, 19, 20.5, 55.5, "carbo")
    e.caixa(14, 18.5, 50, 18, 21.5, 52, "os", dalt="os")                                # el musell
    for s in (-1, 1):                                                                   # les banyes
        tronc(e, 16 + 6 * s, 16, 56, 63, 1.8, 0.4, 5, "os")
        e.caixa(min(16 + 4 * s, 16 + 6 * s), 15.2, 55, max(16 + 4 * s, 16 + 6 * s), 16.8, 57, "os")
    for (x, z) in ((11, 40), (21, 38)):                                                 # ossos penjats
        e.caixa(x - 0.4, 15.6, z, x + 0.4, 16.4, z + 8, "corda")
        cilindre_x(e, x - 3, x + 3, 16, z - 1, 0.9, "os", tex="llis", costats=6, tapa="os")
    for (x, z, mat) in ((12, 46, "tela_verm"), (20, 47, "roba3")):                       # plomes
        tronc(e, x, 16, z, z + 6, 0.9, 0, 4, mat)
    return e


# ---------------------------------------------------------------- el registre
R.afegeix("velas_derretidas", velas_derretidas, "Velas derretidas")
R.afegeix("ataud_cap", lambda: ataud("cap"), "Ataúd (cabeza)", gira=True)
R.afegeix("ataud_peus", lambda: ataud("peus"), "Ataúd (pies)", gira=True)
R.afegeix("percha", percha, "Percha de aarakocra", gira=True)
R.afegeix("rollo_cuerda", rollo_cuerda, "Rollos de cuerda")
R.afegeix("mesa_cartas", mesa_cartas, "Mesa de cartas de flete", gira=True)
R.afegeix("espejo_corte", espejo_corte, "Espejo de corte", gira=True)
R.afegeix("cristal_bruma", cristal_bruma, "Cristales de bruma")
R.afegeix("tambor_guerra", tambor_guerra, "Tambor de guerra")
R.afegeix("totem_huesos", totem_huesos, "Tótem de huesos")
