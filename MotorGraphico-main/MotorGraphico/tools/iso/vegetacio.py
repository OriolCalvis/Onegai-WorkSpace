#!/usr/bin/env python3
"""ISO (VEG-01) — La vegetació amb el kit, a la manera d'Habbo: tronc de
geometria del kit (amb la seva llum i el seu contorn) i capçada de lòbuls
rodons amb la vora ondulada, ombrejats a bandes netes (llum de dalt a
l'esquerra, com tot el kit), amb vores fosques entre lòbuls i unes poques
marques de fulla. Res de soroll: pocs tons, formes clares.

Es veuen igual des dels quatre costats (com els arbres d'Habbo).

    from vegetacio import VEGETACIO
    e = VEGETACIO["arbol_roble"]()      # Escena; e.dibuixa(0) -> (imatge, àncora)
"""
import math
import os
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from monuments import tronc  # noqa: E402
from peces_iso import MATERIALS, Escena  # noqa: E402

# Rampes de la vegetació (de clar a fosc), afegides als materials del kit.
MATERIALS.update({
    "escorca": [(178, 140, 108), (148, 110, 82), (120, 86, 64), (92, 64, 48), (64, 44, 34)],
    "escorca_seca": [(176, 156, 150), (140, 120, 116), (110, 92, 92), (84, 68, 70), (60, 48, 52)],
    "fulla_roure": [(158, 218, 92), (100, 176, 62), (62, 136, 52), (40, 100, 46), (26, 70, 38)],
    "fulla_fruiter": [(196, 238, 120), (142, 206, 80), (96, 168, 60), (64, 128, 50), (42, 92, 40)],
    "fulla_pi": [(130, 196, 120), (82, 154, 96), (52, 118, 78), (34, 88, 62), (22, 62, 48)],
    "fulla_salze": [(172, 220, 100), (116, 180, 68), (74, 142, 54), (48, 106, 46), (30, 74, 38)],
    "fulla_mata": [(176, 230, 112), (122, 194, 78), (80, 154, 60), (54, 116, 50), (36, 82, 42)],
    "fruit_vermell": [(255, 196, 180), (230, 70, 62), (196, 44, 50), (146, 30, 42), (100, 22, 32)],
    "fruit_groc": [(255, 252, 210), (254, 226, 100), (230, 186, 54), (184, 138, 40), (132, 96, 30)],
    "flor_blava": [(226, 238, 255), (168, 200, 255), (118, 150, 238), (80, 104, 198), (56, 70, 152)],
    "flor_blanca": [(255, 255, 255), (248, 246, 238), (224, 222, 212), (192, 190, 180), (150, 148, 140)],
    "flor_vermella": [(255, 196, 186), (246, 104, 94), (212, 62, 62), (158, 40, 48), (108, 28, 36)],
    "jonc": [(214, 234, 150), (170, 206, 104), (128, 168, 74), (94, 130, 56), (66, 94, 42)],
    "boga": [(210, 160, 108), (172, 116, 72), (136, 86, 52), (100, 62, 40), (72, 44, 30)],
})
LLUM = np.array((-0.50, -0.66, 0.56))
LLUM = LLUM / np.linalg.norm(LLUM)


def _rng(llavor):
    return np.random.default_rng(abs(int(llavor)) % (2 ** 32))


def _to(lum):
    """Bandes netes: 1 llum, 2 mig, 3 ombra, 4 ombra fonda."""
    return 1 if lum > 0.80 else (2 if lum > 0.42 else (3 if lum > -0.06 else 4))


def capcada(lobuls, mat, llavor=1, fruits=None, penjolls=0, ones=7, peca_id=4000):
    """Capçada de lòbuls (dx, dy, r) en píxels respecte del punt de l'sprite (el
    capdamunt del tronc). Cada lòbul és una esfera amb la vora ondulada (ones
    bonys per volta) i tres o quatre bandes de to; els de davant (més avall a
    la pantalla) tapen els de darrere i el contorn del kit hi posa la vora
    fosca. fruits: material dels fruits (2x2 amb lluentor). penjolls: tires de
    fulla que pengen (salze)."""
    rampa = np.array(MATERIALS[mat], np.uint8)

    gx0 = sum(l[0] for l in lobuls) / len(lobuls)
    gy0 = sum(l[1] for l in lobuls) / len(lobuls)
    gr = max(math.hypot(l[0] - gx0, l[1] - gy0) + l[2] for l in lobuls)

    def f(ll, sx, sy, prof):
        rng = _rng(llavor)
        ordre = sorted(lobuls, key=lambda l: (l[1] + l[2] * 0.6, l[0]))
        for k, (dx, dy, r) in enumerate(ordre):
            cx, cy = sx + dx, sy + dy
            fase = rng.random() * 2 * math.pi
            p_base = prof + 40 + k * 8
            for yy in range(int(cy - r * 1.1) - 1, int(cy + r * 1.1) + 2):
                for xx in range(int(cx - r * 1.1) - 1, int(cx + r * 1.1) + 2):
                    if not (0 <= xx < ll.w and 0 <= yy < ll.h):
                        continue
                    ex, ey = (xx + 0.5 - cx) / r, (yy + 0.5 - cy) / r
                    d = math.hypot(ex, ey)
                    vora = 1.0 + 0.075 * math.sin(ones * math.atan2(ey, ex) + fase)
                    if d > vora:
                        continue
                    q = min(1.0, d / vora)
                    nx, ny = ex / vora, ey / vora
                    nz = math.sqrt(max(0.0, 1.0 - q * q))
                    p = p_base + nz * 3
                    if p <= ll.prof[yy, xx]:
                        continue
                    lum = nx * LLUM[0] + ny * LLUM[1] + nz * LLUM[2]
                    # i la llum de tota la capçada: els lòbuls de baix a la dreta, més foscos
                    qx, qy = (xx + 0.5 - sx - gx0) / gr, (yy + 0.5 - sy - gy0) / gr
                    lum = 0.55 * lum + 0.45 * (qx * LLUM[0] + qy * LLUM[1] + 0.45) * 1.25
                    ll.col[yy, xx] = (*rampa[_to(lum)], 255)
                    ll.prof[yy, xx] = p
                    ll.peca[yy, xx] = peca_id + k
            # marques de fulla: petits grups clars a la llum i foscos a l'ombra
            pas = max(4, int(r / 2.6))
            for gy in range(int(cy - r), int(cy + r) + 1, pas):
                for gx in range(int(cx - r), int(cx + r) + 1, pas):
                    jx, jy = gx + int(rng.integers(-1, 2)), gy + int(rng.integers(-1, 2))
                    if not (1 <= jx < ll.w - 2 and 1 <= jy < ll.h - 1) or ll.peca[jy, jx] != peca_id + k:
                        continue
                    ex, ey = (jx + 0.5 - cx) / r, (jy + 0.5 - cy) / r
                    if ex * ex + ey * ey > 0.72:
                        continue
                    nz = math.sqrt(max(0.0, 1.0 - ex * ex - ey * ey))
                    qx, qy = (jx + 0.5 - sx - gx0) / gr, (jy + 0.5 - sy - gy0) / gr
                    to = _to(0.55 * (ex * LLUM[0] + ey * LLUM[1] + nz * LLUM[2])
                             + 0.45 * (qx * LLUM[0] + qy * LLUM[1] + 0.45) * 1.25)
                    marca = {1: 0, 2: 1, 3: 4, 4: None}[to]
                    if marca is None or rng.random() < 0.35:
                        continue
                    forma = ((0, 0), (1, 0), (-1, 1)) if rng.random() < 0.5 else ((0, 0), (1, 0), (2, 1))
                    for ox, oy in forma:
                        x, y = jx + ox, jy + oy
                        if 0 <= x < ll.w and 0 <= y < ll.h and ll.peca[y, x] == peca_id + k:
                            ll.col[y, x] = (*rampa[marca], 255)
        if fruits:
            fr = np.array(MATERIALS[fruits], np.uint8)
            for (dx, dy, r) in lobuls:
                for _ in range(max(1, int(r / 4.5))):
                    a = rng.random() * 2 * math.pi
                    rr = math.sqrt(rng.random()) * (r - 4)
                    fx, fy = int(round(sx + dx + rr * math.cos(a))), int(round(sy + dy + rr * math.sin(a) * 0.9))
                    if not (1 <= fx < ll.w - 2 and 1 <= fy < ll.h - 2):
                        continue
                    if not all(ll.peca[fy + oy, fx + ox] >= peca_id for ox in (-1, 0, 1) for oy in (-1, 0, 1)):
                        continue
                    for (ox, oy, to) in ((0, -1, 1), (-1, 0, 1), (0, 0, 1), (1, 0, 2), (0, 1, 2), (1, 1, 3),
                                         (-1, 1, 3), (1, -1, 2)):
                        ll.col[fy + oy, fx + ox] = (*fr[to], 255)
                    ll.col[fy - 1, fx - 1] = (*fr[0], 255)
    return f


def penjolls(lobuls, mat, llavor=1, per_lobul=6, llarg=(0.9, 1.6)):
    """Salze: tires de fulla que pengen de la vora de baix dels lòbuls. Van
    després dels contorns (fulla prima: sense contorn negre), amb la vora dreta
    i la punta d'un to més fosc."""
    rampa = np.array(MATERIALS[mat], np.uint8)

    def f(ll, sx, sy, prof):
        rng = _rng(llavor + 7)
        tires = []
        for (dx, dy, r) in lobuls:
            for t in range(per_lobul):
                u = -0.9 + 1.8 * (t + 0.5) / per_lobul + (rng.random() - 0.5) * 0.1
                tx = int(round(sx + dx + u * r))
                ty0 = int(round(sy + dy + math.sqrt(max(0.0, 1 - u * u)) * r * 0.62))
                tires.append((tx, ty0, int(r * (llarg[0] + rng.random() * (llarg[1] - llarg[0]))), dy + r))
        for (tx, ty0, n, fons) in sorted(tires, key=lambda t: t[3]):
            p = prof + 200 + fons
            for h in range(n):
                yy = ty0 + h
                ona = int(round(math.sin(h * 0.35 + tx) * 0.6))
                for ox, to in ((0, 1), (1, 2), (2, 3), (3, 4)):
                    xx = tx + ox + ona
                    if not (0 <= xx < ll.w and 0 <= yy < ll.h):
                        continue
                    if h >= n - 2 and ox == 0:
                        continue                                  # la punta, més prima
                    t2 = to if h < n * 0.6 else min(4, to + 1)
                    ll.col[yy, xx] = (*rampa[t2], 255)
                    ll.prof[yy, xx] = p
    return f


def branques(llavor=7, peca_id=4500):
    """Arbre sec: el tronc es parteix en branques que es fan primes, a la pantalla."""
    rampa = np.array(MATERIALS["escorca_seca"], np.uint8)

    def f(ll, sx, sy, prof):
        rng = _rng(llavor)
        seg = []

        def creix(x, y, ang, llarg, gruix, nivell):
            x2, y2 = x + math.cos(ang) * llarg, y - math.sin(ang) * llarg
            seg.append((x, y, x2, y2, gruix, nivell))
            if nivell >= 4 or gruix < 1.0:
                return
            for s in (-1, 1):
                if rng.random() < (1.0 if nivell < 2 else 0.8):
                    creix(x2, y2, ang + s * (0.38 + rng.random() * 0.35), llarg * (0.62 + rng.random() * 0.12),
                          gruix * 0.66, nivell + 1)
        creix(sx, sy, math.pi / 2 + (rng.random() - 0.5) * 0.15, 24, 7.5, 0)
        for i, (x0, y0, x1, y1, g, niv) in enumerate(seg):
            n = int(max(abs(x1 - x0), abs(y1 - y0)) * 2) + 1
            for t in range(n + 1):
                x, y = x0 + (x1 - x0) * t / n, y0 + (y1 - y0) * t / n
                gr = g * (1 - 0.35 * t / n)
                for ox in range(-int(gr), int(gr) + 1):
                    xx, yy = int(round(x + ox * 0.5)), int(round(y))
                    if not (0 <= xx < ll.w and 0 <= yy < ll.h):
                        continue
                    to = 1 if ox < 0 else (2 if ox == 0 else 3)
                    ll.col[yy, xx] = (*rampa[to], 255)
                    ll.prof[yy, xx] = prof + 20 + niv
                    ll.peca[yy, xx] = peca_id
    return f


def flors_terra(taques, colors, llavor=3, peca_id=5000):
    """Un clap de flors baixes: mates de fulla menuda (taques (dx, dy, r)) i, al
    damunt, flors de cinc píxels (creu) de dos tons amb el cor groc."""
    fulla = np.array(MATERIALS["fulla_mata"], np.uint8)
    cor = np.array(MATERIALS["flor2"], np.uint8)

    def f(ll, sx, sy, prof):
        rng = _rng(llavor)
        for k, (dx, dy, r) in enumerate(sorted(taques, key=lambda t: t[1])):
            cx, cy = sx + dx, sy + dy
            for yy in range(int(cy - r * 0.75) - 1, int(cy + r * 0.45) + 2):
                for xx in range(int(cx - r) - 1, int(cx + r) + 2):
                    if not (0 <= xx < ll.w and 0 <= yy < ll.h):
                        continue
                    ex, ey = (xx + 0.5 - cx) / r, (yy + 0.5 - cy) / (r * 0.6)
                    vora = 1 + 0.1 * math.sin(9 * math.atan2(ey, ex) + k)
                    if math.hypot(ex, ey) > vora:
                        continue
                    p = prof + 4 + k * 4
                    if p <= ll.prof[yy, xx]:
                        continue
                    to = 2 if ey < -0.35 else (3 if ey < 0.45 else 4)
                    ll.col[yy, xx] = (*fulla[to], 255)
                    ll.prof[yy, xx] = p
                    ll.peca[yy, xx] = peca_id + k
        flors = []
        for k, (dx, dy, r) in enumerate(taques):
            n = max(3, int(r * 0.8))
            for i in range(n):
                a = (i + rng.random() * 0.6) * 2 * math.pi / n
                rr = (0.25 + 0.55 * rng.random()) * r
                flors.append((sx + dx + rr * math.cos(a), sy + dy - 3 - int(rng.integers(0, 4)) + rr * math.sin(a) * 0.45,
                              colors[int(rng.integers(len(colors)))]))
        for i, (fx, fy, col) in enumerate(sorted(flors, key=lambda t: t[1])):
            fx, fy = int(round(fx)), int(round(fy))
            c = np.array(MATERIALS[col], np.uint8)
            for (ox, oy, to) in ((0, -1, 1), (-1, 0, 1), (1, 0, 2), (0, 1, 2), (-1, -1, 1), (1, -1, 2), (-1, 1, 2),
                                 (1, 1, 3), (0, -2, 1), (-2, 0, 1), (2, 0, 2), (0, 2, 3)):
                x, y = fx + ox, fy + oy
                if 0 <= x < ll.w and 0 <= y < ll.h:
                    ll.col[y, x] = (*c[to], 255)
                    ll.prof[y, x] = prof + 60 + i
                    ll.peca[y, x] = peca_id + 100 + i
            if 0 <= fx < ll.w and 0 <= fy < ll.h:
                ll.col[fy, fx] = (*cor[1], 255)
    return f


def joncs(llavor=5, n=9, peca_id=5500):
    """Joncs de ribera: fulles llargues i tiges de dos píxels, algunes amb la boga."""
    jo = np.array(MATERIALS["jonc"], np.uint8)
    bo = np.array(MATERIALS["boga"], np.uint8)

    def f(ll, sx, sy, prof):
        rng = _rng(llavor)
        tiges = sorted(((int(rng.integers(-11, 12)), int(rng.integers(-3, 4)), int(rng.integers(16, 32)),
                         rng.random() < 0.55, (rng.random() - 0.5) * 0.28) for _ in range(n)), key=lambda t: t[1])
        for k, (dx, dy, alt, boga, inclina) in enumerate(tiges):
            x0, y0 = sx + dx, sy + dy
            p = prof + 3 + k * 4
            for h in range(alt):
                x = int(round(x0 + inclina * h))
                y = y0 - h
                for ox, to in ((-1, 4), (0, 2), (1, 4)):
                    if 0 <= x + ox < ll.w and 0 <= y < ll.h and p > ll.prof[y, x + ox]:
                        ll.col[y, x + ox] = (*jo[to], 255)
                        ll.prof[y, x + ox] = p
            if boga:
                xt, yt = int(round(x0 + inclina * (alt - 4))), y0 - alt + 1
                for oy in range(8):
                    for ox in (-2, -1, 0, 1, 2):
                        vora = ox in (-2, 2) or oy in (0, 7)
                        if ox in (-2, 2) and oy in (0, 7):
                            continue
                        to = 4 if vora else (1 if (ox == -1 and oy < 4) else (2 if ox <= 0 else 3))
                        x, y = xt + ox, yt + oy
                        if 0 <= x < ll.w and 0 <= y < ll.h:
                            ll.col[y, x] = (*bo[to], 255)
                            ll.prof[y, x] = p + 1
        for k in range(10):                                     # fulles corbes a la base
            dx = int(rng.integers(-12, 13))
            s = -1 if k % 2 else 1
            for h in range(9):
                x, y = sx + dx + s * (h * h) // 14, sy + 3 - h
                for ox, to in ((0, 2), (1, 3)):
                    if 0 <= x + ox < ll.w and 0 <= y < ll.h and prof + 2 > ll.prof[y, x + ox]:
                        ll.col[y, x + ox] = (*jo[to], 255)
                        ll.prof[y, x + ox] = prof + 2
                        ll.peca[y, x + ox] = peca_id + 90
    return f


def _tronc(e, alt, r0, r1, mat="escorca", arrels=True):
    tronc(e, 16, 16, 0, alt, r0, r1, 8, mat, gir=math.pi / 8)
    if arrels:
        tronc(e, 16, 16, 0, 2.5, r0 + 2, r0, 8, mat, gir=math.pi / 8)


def arbol_roble():
    """Roure: tronc gruixut i capçada ampla de lòbuls foscos."""
    e = Escena(1, 1)
    _tronc(e, 36, 4.4, 3.0)
    e.sprite((16, 16, 36), capcada([(-19, -14, 15), (18, -14, 15), (0, -30, 17), (-11, -3, 14), (11, -2, 14),
                                    (-3, -17, 15), (6, -24, 13)], "fulla_roure", llavor=11))
    return e


def arbol_frutal():
    """Fruiter: tronc prim, capçada rodona i clara, pomes vermelles."""
    e = Escena(1, 1)
    _tronc(e, 24, 3.0, 2.2)
    e.sprite((16, 16, 24), capcada([(-12, -11, 12), (12, -11, 12), (0, -21, 13), (0, -4, 12)], "fulla_fruiter",
                                   llavor=21, fruits="fruit_vermell"))
    return e


def pisos(pisos_, mat="fulla_pi", llavor=9, peca_id=4800):
    """Avet a la manera d'Habbo: pisos triangulars (amplada, alçada, y de la base)
    amb la vora de baix en dents; el de dalt tapa el de sota. La meitat de
    l'esquerra a la llum, la de la dreta a l'ombra."""
    rampa = np.array(MATERIALS[mat], np.uint8)

    def f(ll, sx, sy, prof):
        rng = _rng(llavor)
        for k, (w, h, yb) in enumerate(pisos_):
            cy0 = sy - yb
            p = prof + 10 + k * 6
            for yy in range(cy0 - h, cy0 + 4):
                for xx in range(sx - w // 2 - 2, sx + w // 2 + 3):
                    if not (0 <= xx < ll.w and 0 <= yy < ll.h):
                        continue
                    t = (yy - (cy0 - h)) / h                     # 0 a dalt, 1 a la base
                    mig = w / 2 * min(1.0, t)
                    dx = xx + 0.5 - sx
                    dent = 2.5 * abs(((xx - sx) % 6) - 3) / 3    # dents de 6 px
                    if abs(dx) > mig or yy > cy0 + 1 - dent:
                        continue
                    if p <= ll.prof[yy, xx]:
                        continue
                    u = dx / max(1.0, mig)
                    to = 1 if u < -0.45 and t < 0.75 else (2 if u < 0.1 else (3 if u < 0.6 else 4))
                    if yy > cy0 - 3:
                        to = min(4, to + 1)
                    ll.col[yy, xx] = (*rampa[to], 255)
                    ll.prof[yy, xx] = p
                    ll.peca[yy, xx] = peca_id + k
    return f


def pino():
    """Avet: cinc pisos de branques en dents sobre un tronc curt."""
    e = Escena(1, 1)
    _tronc(e, 12, 2.8, 2.4, arrels=False)
    e.sprite((16, 16, 0), pisos([(46, 24, 14), (38, 22, 29), (30, 20, 43), (22, 17, 56), (13, 15, 67)]))
    return e


def arbol_seco():
    """Arbre sec: sense fulles, les branques grises que es parteixen."""
    e = Escena(1, 1)
    _tronc(e, 10, 3.8, 3.2, mat="escorca_seca")
    e.sprite((16, 16, 10), branques(llavor=17))
    e.ombra_capcada = 0.3              # INT-03: sense fulles, l'ombra de les branques és petita
    return e


def sauce():
    """Salze de ribera: capçada ampla i fosca i la cortina de tires que pengen."""
    e = Escena(1, 1)
    _tronc(e, 40, 4.6, 3.2)
    lob = [(-22, -10, 14), (21, -10, 14), (0, -24, 17), (-11, -1, 13), (11, 0, 13), (0, -9, 14)]
    e.sprite((16, 16, 40), capcada(lob, "fulla_salze", llavor=61))
    e.sprite((16, 16, 40), penjolls(lob, "fulla_salze", llavor=61, per_lobul=4, llarg=(1.3, 2.1)), despres=True)
    return e


def mata():
    """Mata rodona de jardí (sense tronc), a ras de terra."""
    e = Escena(1, 1)
    e.sprite((16, 16, 0), capcada([(-10, -10, 11), (10, -10, 11), (0, -17, 12), (0, -5, 11)], "fulla_mata", llavor=31,
                                  ones=8))
    return e


def flores():
    """Un clap de flors silvestres de quatre colors."""
    e = Escena(1, 1)
    e.sprite((16, 16, 0), flors_terra([(-10, 1, 10), (9, 0, 10), (0, -6, 10), (0, 6, 9)],
                                      ("flor1", "flor_blava", "flor2", "flor_vermella", "flor_blanca"), llavor=41))
    return e


def juncos():
    """Joncs: tiges i boges primes (sense contorn negre, com la roba estesa) amb
    les fulles corbes a la base."""
    e = Escena(1, 1)
    e.sprite((16, 16, 0), joncs(llavor=51, n=13), despres=True)
    return e


def arbol_roble_b():
    """Roure més alt i estret (la variant que el generador alterna amb l'altre)."""
    e = Escena(1, 1)
    _tronc(e, 40, 4.0, 2.8)
    e.sprite((16, 16, 40), capcada([(-14, -12, 14), (14, -13, 14), (0, -32, 16), (-6, -2, 13), (8, -1, 13),
                                    (2, -18, 15)], "fulla_roure", llavor=13))
    return e


def arbol_frutal_b():
    """Fruiter de fruits grocs (codonys i llimones)."""
    e = Escena(1, 1)
    _tronc(e, 22, 2.8, 2.0)
    e.sprite((16, 16, 22), capcada([(-11, -10, 11), (11, -10, 11), (0, -19, 12), (0, -3, 11)], "fulla_fruiter",
                                   llavor=23, fruits="fruit_groc"))
    return e


def pino_b():
    """Avet jove, més baix."""
    e = Escena(1, 1)
    _tronc(e, 10, 2.4, 2.0, arrels=False)
    e.sprite((16, 16, 0), pisos([(38, 21, 12), (30, 19, 25), (21, 16, 37), (12, 13, 47)], llavor=19))
    return e


def mata_b():
    """Mata florida (flors roses menudes)."""
    e = Escena(1, 1)
    e.sprite((16, 16, 0), capcada([(-9, -9, 10), (9, -9, 10), (0, -15, 11), (0, -4, 10)], "fulla_mata", llavor=33,
                                  ones=8, fruits="flor1"))
    return e


def flores_b():
    """Clap de flors blaves i blanques."""
    e = Escena(1, 1)
    e.sprite((16, 16, 0), flors_terra([(-9, 0, 10), (9, 1, 10), (0, -6, 10), (1, 6, 9)],
                                      ("flor_blava", "flor_blanca", "flor_blava", "flor2"), llavor=43))
    return e


VEGETACIO = {"arbol_roble": arbol_roble, "arbol_frutal": arbol_frutal, "pino": pino, "arbol_seco": arbol_seco,
             "sauce": sauce, "mata": mata, "flores": flores, "juncos": juncos}
# Variants que el generador alterna (per la cel·la): <id>_b.
VARIANTS = {"arbol_roble": {"b": arbol_roble_b}, "arbol_frutal": {"b": arbol_frutal_b}, "pino": {"b": pino_b},
            "mata": {"b": mata_b}, "flores": {"b": flores_b}}
