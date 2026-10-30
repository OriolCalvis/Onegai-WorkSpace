#!/usr/bin/env python3
"""ISO (DECO-01) — Els decorats del carrer amb el kit: la mateixa llum, les
mateixes línies i els mateixos materials que les cases, i 4 orientacions
(la cara de davant és +Y, com les portes). Cada decorat és una escena d'una
cel·la (32 x 32 unitats) que no surt del seu rombe.

    from decorats import DECORATS
    e = DECORATS["banco_plaza"]()     # Escena; e.dibuixa(o) -> (imatge, àncora)

Els trets tecnomàgics: els fanals porten cristall que brilla (alambí dels
alquimistes, fanal octogonal dels Guardians), el banc del braser crema amb
flama de cristall i la parada del mercat fa ombra amb tela a ratlles.
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from families import serrell  # noqa: E402
from monuments import poligon_regular, tronc  # noqa: E402
from patis import bota, caixes, estendard  # noqa: E402
from peces_iso import Escena, color_de, mata  # noqa: E402


def _banc(e, x0, x1, y0=12, y1=20):
    """Seient i respatller de taulons sobre potes de ferro; el respatller va a -Y."""
    e.caixa(x0, y0, 7, x1, y1, 9, "fusta", "taulons", dalt="fusta", tex_dalt="taulons_v")
    e.caixa(x0, y0 - 2, 9, x1, y0, 19, "fusta", "taulons")
    e.caixa(x0, y0 - 2, 19, x1, y0 + 1, 20, "noguera")
    for x in (x0 + 1, x1 - 2):
        e.caixa(x, y0, 0, x + 1.5, y0 + 1.5, 7, "ferro")
        e.caixa(x, y1 - 1.5, 0, x + 1.5, y1, 7, "ferro")
        e.caixa(x, y0, 9, x + 1.5, y1, 12, "ferro")             # braç


def banco_plaza():
    e = Escena(1, 1)
    _banc(e, 4, 28)
    return e


def banco_brasero():
    """Banc curt i, al costat, el braser de cristall (tecnomàgia: crema sense fum)."""
    e = Escena(1, 1)
    _banc(e, 12, 29)
    for dx, dy in ((-2.5, -2.5), (2.5, -2.5), (0, 2.5)):         # trípode
        e.caixa(6 + dx, 16 + dy, 0, 7 + dx, 17 + dy, 8, "ferro")
    tronc(e, 6.5, 16.5, 8, 11, 3.5, 5, 8, "ferro", gir=math.pi / 8)
    tronc(e, 6.5, 16.5, 11, 15, 3.5, 2, 8, "cristall", gir=math.pi / 8)
    tronc(e, 6.5, 16.5, 15, 21, 2, 0, 8, "cristall", gir=math.pi / 8)
    return e


def _base(e, r=5.5, alt=4, mat="socol"):
    tronc(e, 16, 16, 0, alt, r, r - 1, 8, mat, "carreus", gir=math.pi / 8)


def farola():
    """Fanal de ferro amb la gàbia de cristall."""
    e = Escena(1, 1)
    _base(e)
    e.caixa(15, 15, 4, 17, 17, 44, "ferro")
    e.caixa(12, 12, 44, 20, 20, 46, "ferro")
    e.caixa(13, 13, 46, 19, 19, 54, "cristall", dalt="ferro")
    for x, y in ((12.5, 12.5), (18.5, 12.5), (12.5, 18.5), (18.5, 18.5)):
        e.caixa(x, y, 46, x + 1, y + 1, 54, "ferro")
    tronc(e, 16, 16, 54, 60, 5, 0, 4, "ferro", gir=math.pi / 4)
    return e


def farola_alambique():
    """Fanal dels alquimistes: un alambí de cristall amb el broc de llautó."""
    e = Escena(1, 1)
    _base(e, 6, 5, "pedra_clara")
    e.cilindre(16, 16, 5, 38, 1.3, "ferro", costats=8)
    e.caixa(10, 15.5, 36, 22, 16.5, 38, "llauto")               # la creu que aguanta el vidre
    e.cilindre(16, 16, 38, 41, 5, "llauto", costats=10)
    e.cilindre(16, 16, 41, 49, 5.5, "cristall", costats=10)
    tronc(e, 16, 16, 49, 54, 5.5, 2, 10, "cristall")
    e.cilindre(16, 16, 54, 58, 1.5, "llauto", costats=8)
    e.caixa(16, 15.5, 56, 22, 16.5, 57, "llauto")                # el broc
    e.caixa(21, 15.5, 52, 22, 16.5, 57, "llauto")
    return e


def fanal_guardianes():
    """Fanal octogonal dels Guardians de la Llum."""
    e = Escena(1, 1)
    g = math.pi / 8
    tronc(e, 16, 16, 0, 3, 6.5, 6, 8, "pedra_clara", gir=g)
    tronc(e, 16, 16, 3, 40, 1.8, 1.5, 8, "ferro", gir=g)
    tronc(e, 16, 16, 39, 41, 5, 5, 8, "llauto", gir=g)
    tronc(e, 16, 16, 41, 50, 4.5, 4.5, 8, "cristall", gir=g)
    tronc(e, 16, 16, 50, 52, 5.2, 5.2, 8, "llauto", gir=g)
    tronc(e, 16, 16, 52, 58, 5.2, 0, 8, "verdet", gir=g)
    e.caixa(15.5, 15.5, 58, 16.5, 16.5, 62, "llauto")
    return e


def parada_mercado():
    """Parada: taulell de fusta amb caixes de fruita i tendal a ratlles."""
    e = Escena(1, 1)
    e.caixa(4, 8, 0, 28, 22, 11, "fusta", "taulons", dalt="noguera")
    e.rect("y", 22, 1, 4, 28, 0, 11, "noguera", "taulons_v", biaix=0.4)
    for x, flors in ((7, ("flor1", "teula")), (15, ("roba3", "flor2")), (22, ("fulla", "boix"))):
        e.caixa(x, 12, 11, x + 6, 19, 14, "fusta", "taulons")
        e.sprite((x + 3, 15.5, 14), mata(3, 2, flors=flors, llavor=x))
    for x, y in ((4, 6), (27, 6), (4, 25), (27, 25)):
        e.caixa(x, y, 0, x + 1.5, y + 1.5, 30 if y < 10 else 26, "noguera")
    p = e.nova_peca()
    e.cara([(3, 5, 31), (29, 5, 31), (29, 27, 27), (3, 27, 27)], (0, 4, 22), color_de("tela_verm", "ratlles", "y"), p, 0.6)
    serrell(e, (3, 27), (29, 27), 27, (0, 1, 0), "tela", alt=4, pas=5)
    return e


def flors(radi, llavor, colors=("flor1", "flor2", "roba3", "roba2")):
    """Un pom de flors (més flor que fulla): com mata(), però la meitat dels
    píxels són pètals dels colors donats, amb el cor clar."""
    import numpy as np
    from peces_iso import MATERIALS

    def f(ll, sx, sy, prof):
        rng = np.random.default_rng(abs(int(llavor)) % (2 ** 32))
        fulla = np.array(MATERIALS["fulla"], np.uint8)
        for yy in range(sy - 2 * radi, sy + 1):
            for xx in range(sx - radi - 1, sx + radi + 2):
                if not (0 <= xx < ll.w and 0 <= yy < ll.h):
                    continue
                dx, dy = (xx - sx) / radi, (yy - (sy - radi)) / radi
                d = dx * dx + dy * dy + 0.2 * math.sin(xx * 1.9 + yy * 1.3)
                if d > 1.0 or prof + 2 <= ll.prof[yy, xx]:
                    continue
                if rng.random() < 0.62 and dy < 0.55:
                    rampa = np.array(MATERIALS[colors[int(rng.integers(len(colors)))]], np.uint8)
                    c = rampa[0 if rng.random() < 0.25 else (1 if rng.random() < 0.6 else 2)]
                else:
                    c = fulla[1 if (dx < -0.2 and dy < -0.1) else (3 if dx > 0.35 or dy > 0.45 else 2)]
                ll.col[yy, xx] = (*c, 255)
                ll.prof[yy, xx] = prof + 2 + (1 - d) * 2
                ll.peca[yy, xx] = 950 + abs(int(llavor)) % 40
    return f


def jardinera():
    e = Escena(1, 1)
    e.caixa(5, 9, 0, 27, 23, 8, "fusta", "taulons", dalt="terra")
    for k, (x, y) in enumerate(((10, 13), (16, 18), (22, 14), (12, 19), (21, 20))):
        e.sprite((x, y, 8), flors(5, 30 + k))
    return e


def _roda(e, x, cy, cz, r, s):
    """Roda de carro al pla X = x (s = +1 a la dreta, -1 a l'esquerra)."""
    pts = [(cy + r * math.cos(2 * math.pi * k / 12), cz + r * math.sin(2 * math.pi * k / 12)) for k in range(12)]
    e.poligon_cara("x", x, s, pts, "noguera", to=3, biaix=0.4)
    e.poligon_cara("x", x, s, [(cy + (r - 1.5) * math.cos(2 * math.pi * k / 12), cz + (r - 1.5) * math.sin(2 * math.pi * k / 12))
                                for k in range(12)], "fusta", biaix=0.6)
    for a in (0, math.pi / 3, 2 * math.pi / 3):
        e.poligon_cara("x", x, s, [(cy - (r - 1.5) * math.cos(a) - 0.5 * math.sin(a), cz - (r - 1.5) * math.sin(a) + 0.5 * math.cos(a)),
                                    (cy + (r - 1.5) * math.cos(a) - 0.5 * math.sin(a), cz + (r - 1.5) * math.sin(a) + 0.5 * math.cos(a)),
                                    (cy + (r - 1.5) * math.cos(a) + 0.5 * math.sin(a), cz + (r - 1.5) * math.sin(a) - 0.5 * math.cos(a)),
                                    (cy - (r - 1.5) * math.cos(a) + 0.5 * math.sin(a), cz - (r - 1.5) * math.sin(a) - 0.5 * math.cos(a))],
                       "noguera", to=3, biaix=0.8)
    e.poligon_cara("x", x, s, [(cy - 1.2, cz - 1.2), (cy + 1.2, cz - 1.2), (cy + 1.2, cz + 1.2), (cy - 1.2, cz + 1.2)], "ferro", biaix=1.0)


def carro():
    """Carro de mà amb sacs: caixa de taulons, dues rodes i les vares cap a +Y."""
    e = Escena(1, 1)
    e.caixa(9, 5, 7, 23, 22, 15, "fusta", "taulons", dalt="noguera")
    for x in (8.5, 23):
        e.caixa(x, 22, 6, x + 0.5 + 1, 30, 8, "noguera")         # vares
    _roda(e, 24, 13, 7.5, 7.5, 1)
    _roda(e, 8, 13, 7.5, 7.5, -1)
    for (x, y, z) in ((11, 8, 15), (16, 9, 15), (13, 14, 15), (18, 15, 15), (14, 11, 19)):
        e.caixa(x, y, z, x + 5, y + 5, z + 4, "tela", "taulons_v", dalt="tela")
    return e


def pozo():
    """Pou: brocal de pedra, aigua, dos pals, el torn i la teuladeta."""
    e = Escena(1, 1)
    e.cilindre(16, 16, 0, 10, 10, "pedra", "carreus", costats=14, dalt="pedra_clara")
    e.cilindre(16, 16, 10, 10.6, 7.5, "aigua", costats=14, dalt="aigua")
    for x in (5, 26):
        e.caixa(x, 15, 0, x + 2, 17, 30, "noguera")
    e.caixa(5, 15.5, 22, 28, 16.5, 23.5, "noguera")              # el torn
    e.caixa(14.5, 15, 14, 17.5, 18, 18, "fusta", "cercles", dalt="aigua")   # galleda
    e.teulada(4, 11, 28, 21, 30, 7, mat="teula", voladis=2, gruix=2, mat_timpa="noguera", tex_timpa="taulons", eix="x")
    return e


def barriles():
    e = Escena(1, 1)
    bota(e, 11, 12)
    bota(e, 21, 15)
    bota(e, 13, 22, alt=11)
    return e


def cajas():
    e = Escena(1, 1)
    caixes(e, 11, 12)
    e.caixa(17, 15, 0, 26, 24, 9, "fusta", "taulons")
    e.sprite((21.5, 19.5, 9), mata(3, 2, flors=("flor2", "teula"), llavor=7))
    return e


def bolardo():
    e = Escena(1, 1)
    g = math.pi / 8
    tronc(e, 16, 16, 0, 2, 5, 4.5, 8, "socol", gir=g)
    tronc(e, 16, 16, 2, 12, 3.5, 3, 8, "pedra", "carreus", gir=g)
    tronc(e, 16, 16, 7, 9, 3.8, 3.8, 8, "llauto", gir=g)
    tronc(e, 16, 16, 12, 14, 3.2, 1, 8, "pedra_clara", gir=g)
    return e


def fardos():
    """Sacs de gra apilats al moll."""
    e = Escena(1, 1)
    for (x, y, z, w) in ((6, 9, 0, 9), (16, 9, 0, 9), (8, 18, 0, 9), (18, 18, 0, 9), (11, 12, 6, 9)):
        e.caixa(x, y, z, x + w, y + 7, z + 6, "tela", "taulons_v", dalt="tela")
        e.caixa(x + w / 2 - 1, y + 1, z + 6, x + w / 2 + 1, y + 6, z + 7, "encenall")
    return e


def hoguera():
    """Foguera: un cercle de pedres, la llenya creuada i la flama."""
    e = Escena(1, 1)
    for (x, y) in poligon_regular(16, 16, 9, 9):
        e.caixa(x - 2, y - 1.5, 0, x + 2, y + 1.5, 3, "socol", dalt="pedra")
    e.caixa(9, 15, 0, 23, 17, 3, "noguera", "llenya")
    e.caixa(15, 9, 3, 17, 23, 5, "noguera", "llenya")
    for (r, z0, z1, to) in ((4.5, 4, 12, 2), (3, 8, 17, 1), (1.5, 13, 21, 0)):
        tronc(e, 16, 16, z0, z1, r, 0.3, 6, "foc", gir=z0)
    return e


# La roba de la corda: (amplada, fins a quina alçada penja, color); la camisa
# porta mànigues (una franja més ampla a dalt).
ROBES = [(7, 10, "tela"), (5, 14, "roba1"), (2, 17, "roba3"), (6, 12, "roba2"), (5, 14, "tela_verm"), (2, 18, "roba1"),
         (7, 11, "roba3"), (5, 14, "roba2")]


def _penja(e, u, ample, fins, mat, y=15.75, z=22):
    pts = [(u, y, z), (u + ample, y, z), (u + ample, y, fins), (u, y, fins)]
    e.cara(pts, (0, 1, 0), color_de(mat, "llis", "y"), e.nova_peca(), 0.4, True)
    if ample == 5:                                                 # camisa: les mànigues
        e.cara([(u - 1.5, y, z), (u + ample + 1.5, y, z), (u + ample + 1.5, y, z - 3), (u - 1.5, y, z - 3)], (0, 1, 0),
               color_de(mat, "llis", "y"), e.nova_peca(), 0.4, True)


def tendedero(pals="dos"):
    """Corda d'estendre al llarg de X amb roba (als patis i entre cases, no a la
    plaça). Una corda de diverses cel·les seguides només porta pal als extrems:
    pals «dos» (una cel·la sola), «ini» (el pal a -X, la corda segueix cap a +X),
    «mig» i «mig2» (cap pal, la corda d'una vora a l'altra; dues robes diferents
    perquè una corda llarga no es repeteixi) o «fi» (el pal a +X)."""
    e = Escena(1, 1)
    x0 = 2 if pals in ("dos", "ini") else 0
    x1 = 30 if pals in ("dos", "fi") else 32
    for x in ((x0,) if pals in ("dos", "ini") else ()) + ((x1 - 1.5,) if pals in ("dos", "fi") else ()):
        e.caixa(x, 15, 0, x + 1.5, 16.5, 25, "noguera")
    e.caixa(x0, 15.5, 22, x1, 16, 22.5, "tela")
    u = x0 + (4 if x0 else 2)
    fi = x1 - (3.5 if x1 < 32 else 2)
    k = {"dos": 0, "ini": 3, "mig": 5, "mig2": 2, "fi": 1}[pals]
    while True:
        ample, fins, mat = ROBES[k % len(ROBES)]
        marge = 1.5 if ample == 5 else 0
        if u + marge + ample + marge > fi:
            break
        _penja(e, u + marge, ample, fins, mat)
        u += ample + 2 * marge + 1.5
        k += 1
    return e


def _cilindre_x(e, x0, x1, cy, cz, r, mat, tex="llis", costats=10, tapa="noguera", tex_tapa="llis"):
    """Cilindre ajagut al llarg de X (una bóta a terra, una vela de carro)."""
    p = e.nova_peca()
    pts = [(cy + r * math.cos(2 * math.pi * (k + 0.5) / costats), cz + r * math.sin(2 * math.pi * (k + 0.5) / costats))
           for k in range(costats)]
    for k in range(costats):
        (ya, za), (yb, zb) = pts[k], pts[(k + 1) % costats]
        n = (0, (ya + yb) / 2 - cy, (za + zb) / 2 - cz)
        e.cara([(x0, ya, za), (x1, ya, za), (x1, yb, zb), (x0, yb, zb)], n, color_de(mat, tex, "x"), p)
    e.poligon_cara("x", x1, 1, pts, tapa, tex_tapa, peca=p)
    e.poligon_cara("x", x0, -1, pts, tapa, tex_tapa, peca=p)
    return p


def carromato(lona="tela", tex_lona="llis"):
    """Carro de les caravanes de Surysal: caixa llarga sobre quatre rodes, la vela
    de lona sobre arcs i les vares cap a +Y (la cara de davant)."""
    e = Escena(1, 1)
    e.caixa(9, 3, 8, 23, 26, 12, "fusta", "taulons", dalt="noguera")
    for x in (11.5, 19.5):
        e.caixa(x, 26, 7, x + 1, 31.5, 8.5, "noguera")                 # vares
    e.caixa(11.5, 30.5, 7, 20.5, 31.5, 8.5, "noguera")                  # el travesser
    for y in (8, 21):
        _roda(e, 23.5, y, 6, 6, 1)
        _roda(e, 8.5, y, 6, 6, -1)
    # la vela: mig cilindre al llarg de Y, de 7 trossos, i les boques (la de davant, fosca per dins)
    p = e.nova_peca()
    cx, cz, r, y0, y1, n = 16, 12, 8, 4.5, 24.5, 7
    arc = [(cx + r * math.cos(math.pi * k / n), cz + r * math.sin(math.pi * k / n)) for k in range(n + 1)]
    for k in range(n):
        (xa, za), (xb, zb) = arc[k], arc[k + 1]
        am = math.pi * (k + 0.5) / n
        e.cara([(xa, y0, za), (xa, y1, za), (xb, y1, zb), (xb, y0, zb)], (math.cos(am), 0, math.sin(am)),
               color_de(lona, tex_lona, "x"), p)
    e.poligon_cara("y", y1, 1, arc, lona, tex_lona, peca=p)
    e.poligon_cara("y", y0, -1, arc, lona, tex_lona, peca=p)
    dins = [(cx + (r - 2) * math.cos(math.pi * k / n), cz + (r - 2.5) * math.sin(math.pi * k / n)) for k in range(n + 1)]
    e.poligon_cara("y", y1 + 0.2, 1, dins, "noguera", to=4)
    for y in (y0 + 0.4, (y0 + y1) / 2, y1 - 0.4):                      # els arcs de fusta
        for k in range(n):
            (xa, za), (xb, zb) = arc[k], arc[k + 1]
            am = math.pi * (k + 0.5) / n
            e.cara([(xa + 0.3 * math.cos(am), y - 0.4, za + 0.3 * math.sin(am)),
                    (xb + 0.3 * math.cos(am), y - 0.4, zb + 0.3 * math.sin(am)),
                    (xb + 0.3 * math.cos(am), y + 0.4, zb + 0.3 * math.sin(am)),
                    (xa + 0.3 * math.cos(am), y + 0.4, za + 0.3 * math.sin(am))],
                   (math.cos(am), 0, math.sin(am)), color_de("noguera", "llis", "x"), p)
    return e


def carromato_lona():
    return carromato("tela_verm", "ratlles")


def barriles_rotos():
    """Bótes velles: una de dreta sense tapa, una d'ajaguda i dues dogues a terra."""
    e = Escena(1, 1)
    e.cilindre(10, 11, 0, 11, 4.5, "fusta", "cercles", dalt="noguera")
    e.cilindre(10, 11, 11, 11.3, 3.6, "noguera", dalt="noguera")
    e.cara([(x, y, 11.4) for x, y in poligon_regular(10, 11, 3.4, 10)], (0, 0, 1), color_de("aigua", "llis", "z"), e.nova_peca())
    _cilindre_x(e, 13, 26, 20, 4.5, 4.5, "fusta", "cercles", tapa="noguera")
    for (x0, y0, x1, y1) in ((5, 22, 11, 23.2), (8, 27, 9.2, 31)):
        e.caixa(x0, y0, 0, x1, y1, 1, "fusta", "taulons")
    return e


def basura_taberna():
    """Les deixalles de la taverna: una caixa trencada, un sac, ampolles i encenalls."""
    e = Escena(1, 1)
    for (x0, y0, x1, y1) in ((5, 8, 15, 9), (5, 17, 15, 18), (5, 8, 6, 18), (14, 8, 15, 18)):
        e.caixa(x0, y0, 0, x1, y1, 7 if x0 != 14 else 4, "fusta", "taulons")       # la caixa sense tapa
    e.caixa(6, 9, 0, 14, 17, 2, "encenall")
    e.caixa(18, 8, 0, 27, 16, 7, "tela", "taulons_v", dalt="tela")                # el sac
    e.caixa(21, 10, 7, 24, 13, 9, "tela")
    for (x, y, h, mat) in ((10, 23, 7, "verdet"), (13.5, 25, 6, "noguera"), (21, 23, 7, "verdet")):
        e.cilindre(x, y, 0, h, 1.4, mat, costats=6, dalt=mat)                    # ampolles buides
        e.cilindre(x, y, h, h + 2.5, 0.7, mat, costats=6)
    _cilindre_x(e, 16, 22, 27, 1.4, 1.4, "verdet", costats=6, tapa="verdet")     # una de tombada
    return e


def red_pesca():
    """La xarxa a assecar entre dos pals, una cistella de peix i un rotlle de corda."""
    e = Escena(1, 1)
    for x in (4, 26):
        e.caixa(x, 9, 0, x + 2, 11, 24, "noguera")
    e.caixa(4, 9.5, 22, 28, 10.5, 23.5, "noguera")
    e.cara([(6, 10, 6), (26, 10, 6), (26, 10, 22), (6, 10, 22)], (0, 1, 0), color_de("tela", "xarxa", "y", desp=1),
           e.nova_peca(), 0.0, True)
    e.cilindre(10, 22, 0, 6, 5, "fusta", "taulons_v", dalt="noguera")
    for (dx, dy) in ((-2, -1), (1, -2), (2, 1), (-1, 2), (0, 0)):         # el peix, platejat
        e.caixa(10 + dx - 1.2, 22 + dy - 0.8, 5.5, 10 + dx + 1.2, 22 + dy + 0.8, 6.8, "llauna", dalt="llauna")
    e.cilindre(22, 23, 0, 3, 5, "encenall", costats=12, dalt="encenall")
    e.cilindre(22, 23, 3, 3.3, 2, "noguera", costats=8, dalt="noguera")
    return e


def bandera_ciudad():
    e = Escena(1, 1)
    e.caixa(13, 13, 0, 19, 19, 3, "socol", dalt="pedra")
    estendard(e, 15, 15, alt=40)
    return e


def tanca_fusta():
    """Tanca de taulons al llarg de X (dos travessers i tres pals)."""
    e = Escena(1, 1)
    for x in (1, 15, 29):
        e.caixa(x, 15, 0, x + 2, 17, 14, "noguera")
        tronc(e, x + 1, 16, 14, 16, 1.4, 0, 4, "noguera", gir=math.pi / 4)
    for z in (4, 10):
        e.caixa(0, 15.5, z, 32, 16.5, z + 2.5, "fusta", "taulons")
    return e


def empalizada():
    """Empalissada del camp d'instrucció (Barrio Militar): troncs arrenglerats
    amb la punta esmolada al llarg de X i el travesser que els lliga, darrere."""
    e = Escena(1, 1)
    e.caixa(0, 13.6, 9, 32, 14.6, 11.5, "noguera")
    e.caixa(0, 13.6, 19, 32, 14.6, 21.5, "noguera")
    for i in range(8):
        x = 2 + i * 4
        alt = 25 + (i * 5 % 3) * 2
        tronc(e, x, 16, 0, alt, 1.95, 1.95, 6, "fusta", gir=0.3)
        tronc(e, x, 16, alt, alt + 5, 1.95, 0, 6, "fusta", gir=0.3)
    return e


DECORATS = {
    "banco_plaza": banco_plaza, "banco_brasero": banco_brasero, "farola": farola, "farola_alambique": farola_alambique,
    "fanal_guardianes": fanal_guardianes, "parada_mercado": parada_mercado, "jardinera": jardinera, "carro": carro,
    "pozo": pozo, "barriles": barriles, "cajas": cajas, "bolardo": bolardo, "fardos": fardos, "hoguera": hoguera,
    "tendedero": tendedero, "bandera_ciudad": bandera_ciudad, "tanca_fusta": tanca_fusta,
    "carromato": carromato, "carromato_lona": carromato_lona, "barriles_rotos": barriles_rotos,
    "basura_taberna": basura_taberna, "red_pesca": red_pesca, "empalizada": empalizada,
}
# Els que tenen cara de davant (el generador els gira cap a la plaça o el carrer):
# a la fulla, <id> és o0 (davant a +Y) i <id>_o1.._o3 les altres tres.
ORIENTABLES = {"banco_plaza", "banco_brasero", "parada_mercado", "carro", "tanca_fusta", "pozo", "carromato",
               "carromato_lona", "red_pesca", "empalizada"}
# La roba estesa en cordes de diverses cel·les (al llarg de X): <id>_ini, _mig, _mig2, _fi.
TRAMS = {"tendedero": {t: (lambda t=t: tendedero(t)) for t in ("ini", "mig", "mig2", "fi")}}
