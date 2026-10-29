#!/usr/bin/env python3
"""MASC-01 — La màgia de les mascotes, dibuixada per codi amb els colors dels
cristalls del kit (tools/iso/peces_iso.MATERIALS): la llum a terra, les
partícules que pugen (espurna, hexàgon, octàgon, mota, brasa, espora, boira,
fum) i l'anell de vent (meitat del darrere i meitat del davant).

Cada efecte és una cel·la de la fulla de mascotes amb el centre a (cx, cy)
(el peu de la cel·la); les partícules i l'anell en fan 4 seguides.

    from fx_mascotas import efectes_de, dibuixa
"""
import math

from PIL import Image, ImageDraw

# Rampes (de clar a fosc): les dels cristalls del kit i les de la màgia.
COLORS = {
    "cristall": [(255, 255, 255), (220, 248, 255), (150, 222, 255), (90, 176, 236), (60, 120, 190)],
    "llum": [(255, 255, 255), (236, 246, 255), (196, 222, 255), (150, 186, 250), (104, 140, 220)],
    "llauto": [(255, 250, 210), (255, 236, 150), (246, 206, 104), (214, 164, 64), (166, 120, 44)],
    "foc": [(255, 250, 210), (255, 220, 110), (255, 160, 60), (226, 96, 40), (160, 50, 30)],
    "vent": [(250, 255, 245), (214, 255, 222), (166, 240, 190), (110, 206, 160), (70, 160, 130)],
    "boira": [(250, 252, 255), (226, 238, 252), (198, 218, 246), (164, 190, 232), (130, 156, 210)],
    "espora": [(240, 255, 255), (170, 250, 255), (100, 222, 250), (60, 170, 230), (40, 110, 190)],
    "gris": [(246, 246, 246), (220, 220, 224), (190, 190, 198), (150, 150, 160), (110, 110, 122)],
}


def _px(d, x, y, c, a=255):
    d.point((x, y), fill=(*c, a))


def llum(color):
    """Llum a terra: un rombe-el·lipse en tres bandes (no un degradat: pixel art)."""
    def f(im, cx, cy, k):
        d = ImageDraw.Draw(im)
        r = COLORS[color]
        for (rx, ry, a, to) in ((17, 8, 105, 3), (12, 5.5, 145, 2), (7, 3.2, 185, 1)):
            for y in range(int(cy - ry) - 1, int(cy + ry) + 2):
                for x in range(int(cx - rx) - 1, int(cx + rx) + 2):
                    if ((x + 0.5 - cx) / rx) ** 2 + ((y + 0.5 - cy) / ry) ** 2 <= 1.0:
                        _px(d, x, y, r[to], a)
    return f


def espurna(color):
    """Estrella de quatre puntes que parpelleja: punt, creu, estrella, creu."""
    def f(im, cx, cy, k):
        d = ImageDraw.Draw(im)
        r = COLORS[color]
        mida = (0, 1, 2, 1)[k]
        _px(d, cx, cy, r[0])
        for s in range(1, mida + 1):
            for dx, dy in ((s, 0), (-s, 0), (0, s), (0, -s)):
                _px(d, cx + dx, cy + dy, r[2 if s == 1 else 3])
        if mida == 2:
            for dx, dy in ((1, 1), (-1, -1), (1, -1), (-1, 1)):
                _px(d, cx + dx, cy + dy, r[3], 200)
    return f


def _contorn(punts, color, k):
    """Un contorn del color saturat de la rampa amb una guspira clara que hi dona la volta."""
    def f(im, cx, cy, kk):
        d = ImageDraw.Draw(im)
        r = COLORS[color]
        n = len(punts)
        for i, (x, y) in enumerate(punts):
            guspira = (i - kk * n // 4) % n in (0, 1)
            _px(d, cx + x, cy + y, r[0] if guspira else r[2])
        _px(d, cx, cy, r[1], 200)
    return f


def hexagon(color):
    """Hexàgon petit (els alquimistes): contorn de 6 costats que brilla per torns."""
    punts = [(-1, -2), (0, -2), (1, -2), (2, -1), (2, 0), (2, 1), (1, 2), (0, 2), (-1, 2), (-2, 1), (-2, 0), (-2, -1)]
    return _contorn(punts, color, 0)


def octagon(color):
    """Runa octogonal (els Guardians de la Llum)."""
    punts = [(-1, -3), (0, -3), (1, -3), (2, -2), (3, -1), (3, 0), (3, 1), (2, 2), (1, 3), (0, 3), (-1, 3), (-2, 2),
             (-3, 1), (-3, 0), (-3, -1), (-2, -2)]
    def f(im, cx, cy, k):
        _contorn(punts, color, k)(im, cx, cy, k)
        d = ImageDraw.Draw(im)
        r = COLORS[color]
        for dx, dy in ((0, -1), (0, 1), (-1, 0), (1, 0)):          # la creu de la runa
            _px(d, cx + dx, cy + dy, r[2 + (k % 2)])
    return f


def mota(color):
    """Mota de llum: un punt clar amb un halo d'un píxel que respira."""
    def f(im, cx, cy, k):
        d = ImageDraw.Draw(im)
        r = COLORS[color]
        for dx, dy in ((0, 0), (1, 0), (0, 1), (1, 1)):
            _px(d, cx + dx, cy + dy, r[0] if k % 2 == 0 else r[1])
        for dx, dy in ((-1, 0), (2, 0), (0, -1), (1, -1), (-1, 1), (2, 1), (0, 2), (1, 2)):
            _px(d, cx + dx, cy + dy, r[3], 210 if k in (1, 2) else 150)
    return f


def brasa(color):
    """Brasa: nucli groc, vora taronja i vermella, que tremola."""
    def f(im, cx, cy, k):
        d = ImageDraw.Draw(im)
        r = COLORS[color]
        _px(d, cx, cy, r[0])
        _px(d, cx + (k % 2), cy - 1, r[1])
        for dx, dy in ((-1, 0), (1, 0), (0, 1)):
            _px(d, cx + dx, cy + dy, r[2 + (k % 2)])
        if k in (1, 3):
            _px(d, cx, cy - 2, r[3], 160)
    return f


def espora(color):
    """Espora bioluminiscent: dos píxels i un halo tènue."""
    def f(im, cx, cy, k):
        d = ImageDraw.Draw(im)
        r = COLORS[color]
        _px(d, cx, cy, r[0])
        _px(d, cx, cy - 1, r[1 + (k % 2)])
        for dx, dy in ((-1, 0), (1, 0), (-1, -1), (1, -1), (0, 1), (0, -2)):
            _px(d, cx + dx, cy + dy, r[3], 150 + 40 * (k % 2))
    return f


def bufada(color, mida0=4, a0=150):
    """Bufada que s'eixampla (boira, fum): el·lipse clara, cada frame més gran."""
    def f(im, cx, cy, k):
        d = ImageDraw.Draw(im)
        r = COLORS[color]
        rx, ry = mida0 + k * 1.5, (mida0 + k * 1.5) * 0.55
        for y in range(int(cy - ry) - 1, int(cy + ry) + 2):
            for x in range(int(cx - rx) - 1, int(cx + rx) + 2):
                q = ((x + 0.5 - cx) / rx) ** 2 + ((y + 0.5 - cy) / ry) ** 2
                if q <= 1.0:
                    to = 1 if (y < cy - ry * 0.2 and x < cx) else (2 if q < 0.6 else 3)
                    _px(d, x, y, r[to], a0 - 25 * k)
    return f


def anell(color, part):
    """Anell de vent al voltant de la mascota: guions que giren. part «rere» (l'arc
    de dalt, darrere el cos) o «davant» (l'arc de baix)."""
    def f(im, cx, cy, k):
        d = ImageDraw.Draw(im)
        r = COLORS[color]
        for (rx, ry, gruix) in ((17, 7, 0), (16, 6.4, 1)):     # dues el·lipses: el guió fa 2 px
            for i in range(56):
                a = 2 * math.pi * i / 56
                x, y = cx + rx * math.cos(a), cy + ry * math.sin(a)
                darrere = math.sin(a) < 0
                if darrere != (part == "rere"):
                    continue
                if (i // 5 + k) % 3 == 2:                   # els buits del guió avancen
                    continue
                to = (1 if (i // 5 + k) % 3 == 0 else 2) + gruix
                _px(d, int(round(x)), int(round(y)), r[to], 235 if part == "davant" else 170)
    return f


PARTICULES = {"espurna": espurna, "hexagon": hexagon, "octagon": octagon, "mota": mota, "brasa": brasa,
              "espora": espora, "boira": lambda c: bufada(c, 4, 140)}


def efectes_de(magies):
    """Els efectes que fan falta per a unes màgies: [(nom, funció, frames)]."""
    out, vistos = [], set()

    def afegeix(nom, f, n):
        if nom not in vistos:
            vistos.add(nom)
            out.append((nom, f, n))
    for mg in magies:
        c = mg["color"]
        afegeix(f"llum_{c}", llum(c), 1)
        if mg.get("particula") in PARTICULES and mg.get("quantes", 0) > 0:
            afegeix(f"{mg['particula']}_{c}", PARTICULES[mg["particula"]](c), 4)
            if mg["particula"] == "brasa":
                afegeix("fum_gris", bufada("gris", 3, 150), 4)
        if mg.get("anell"):
            afegeix(f"anell_rere_{c}", anell(c, "rere"), 4)
            afegeix(f"anell_davant_{c}", anell(c, "davant"), 4)
    return out


def dibuixa(f, cella, cx, cy, k):
    im = Image.new("RGBA", (cella, cella), (0, 0, 0, 0))
    f(im, cx, cy, k)
    return im
