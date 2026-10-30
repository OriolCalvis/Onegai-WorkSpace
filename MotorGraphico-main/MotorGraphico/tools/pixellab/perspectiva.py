#!/usr/bin/env python3
"""ESTIL — Mesura la perspectiva d'un sprite d'edifici o prop i la compara amb
la camera del motor: isometrica 2:1 (dimetrica). En aquesta camera les
arestes del terra alineades amb la quadricula baixen 1 px per cada 2 px
horitzontals (pendent 0,50 = 26,6 graus), les diagonals de la planta queden
horitzontals o verticals, els cercles son el·lipses el doble d'amples que
d'altes (0,50) i les parets son verticals (cap punt de fuga).

    python3 tools/pixellab/perspectiva.py peca.png [peca2.png ...]
            [--planta quadrat|hex|rodona] [--empremta AMPLExALT] [-o sortida.png]

Mesura el perfil inferior de la silueta (el punt mes baix de cada columna):
des de la cantonada davantera (el punt mes baix) ajusta una recta robusta
(Theil-Sen) cap a cada costat i, alhora, una el·lipse; es queda amb el model
que millor encaixa i escriu el numero que ha de valer 0,50:
  recta  -> pendent de l'aresta esquerra i de la dreta (0,50 = 2:1)
  rodona -> alt/ample de la mitja el·lipse de la base (0,50 = 2:1)
Veredictes: OK (0,38-0,62), ALT (camera massa amunt: >0,62, es veu massa
terra i teulada), PLA (camera massa baixa: <0,38), FRONTAL (una cara mira
a la camera o la base es una linia horitzontal: no esta en cantonada),
GIRAT (les dues arestes no s'assemblen: la planta no segueix la quadricula).
--planta hex: un hexagon regular NO pot seguir la quadricula; amb un vertex
al davant les arestes han de valer 0,29 i amb una aresta al davant 0,87.
--empremta 3x2 (cel·les) i -o: dibuixa el rombe de l'empremta sota el
sprite (64x32 per cel·la), el perfil mesurat i les arestes ajustades.
"""
import math
import os
import sys

import numpy as np
from PIL import Image, ImageDraw

ESPERAT = {"quadrat": [(0.50, "2:1")], "rodona": [(0.50, "2:1")],
           "hex": [(0.29, "vertex al davant"), (0.87, "aresta al davant")]}
MARGE = 0.12


def perfil_inferior(img):
    """(xs, ys) del punt opac mes baix de cada columna, suavitzat amb una
    mediana de 3 columnes perque un pixel penjat no compti."""
    a = np.array(img.convert("RGBA"))[:, :, 3] > 0
    cols = np.where(a.any(axis=0))[0]
    if len(cols) < 24:
        return None, None
    ys = np.array([np.where(a[:, x])[0].max() for x in cols], dtype=float)
    if len(ys) >= 3:
        ys = np.array([np.median(ys[max(0, i - 1):i + 2]) for i in range(len(ys))])
    return cols.astype(float), ys


def aresta_dominant(xs, ys):
    """La recta que passa a menys de 2 px de mes punts del perfil (cerca de
    pendents 0..1,6): l'aresta llarga guanya a les escales, els fanals o els
    estendards que sobresurten. Torna (pendent, fraccio de punts a la recta)."""
    n = len(xs)
    if n < 6:
        return None, 0.0
    millor, dins = None, 0
    for s in np.arange(0.1, 1.61, 0.02):
        b = np.sort(ys - s * xs)
        # finestra lliscant de 4 px sobre les ordenades a l'origen
        j = np.searchsorted(b, b + 4.0, side="right")
        k = int((j - np.arange(n)).max())
        if k > dins or (k == dins and millor is not None and abs(s - 0.5) < abs(millor - 0.5)):
            millor, dins = float(s), k
    return millor, dins / n


def mitja_el_lipse(xs, ys, ymax):
    """La mitja el·lipse (cintura yw, semieixos A i B, fons a ymax) que passa
    a menys de 2 px de mes punts del perfil. Torna (B/A, fraccio, (xc, yw, A, B))."""
    xc = (xs[0] + xs[-1]) / 2.0
    A0 = (xs[-1] - xs[0]) / 2.0
    millor = (None, 0.0, None)
    for fA in np.arange(0.6, 1.001, 0.04):
        A = A0 * fA
        t = np.clip(1 - ((xs - xc) / A) ** 2, 0, 1)
        arrel = np.sqrt(t)
        for B in np.arange(0.2 * A, 1.2 * A, 1.0):
            pred = ymax - B + B * arrel
            dins = float(np.mean(np.abs(ys - pred) <= 2.0))
            if dins > millor[1]:
                millor = (B / A, dins, (xc, ymax - B, A, B))
    return millor


def mesura(img):
    """Diccionari amb el perfil, l'apex, els dos pendents (recta), la relacio
    alt/ample (el·lipse), el model que encaixa millor i el veredicte."""
    xs, ys = perfil_inferior(img)
    if xs is None:
        return {"error": "silueta massa petita"}
    ample = xs[-1] - xs[0]
    ymax = ys.max()
    # Apex: el tram pla mes llarg a 3 px (2 % de l'ample) del punt mes baix:
    # la cantonada davantera o l'aresta frontal d'un octagon; una escala curta no guanya.
    baix = ys >= ymax - max(3, round(0.02 * ample))
    trams, ini = [], None
    for i, b in enumerate(list(baix) + [False]):
        if b and ini is None:
            ini = i
        elif not b and ini is not None:
            trams.append((ini, i - 1))
            ini = None
    centre = (xs[0] + xs[-1]) / 2
    ia, ib = max(trams, key=lambda t: (t[1] - t[0], -abs((xs[t[0]] + xs[t[1]]) / 2 - centre)))
    xa, xb = xs[ia], xs[ib]
    r = {"ample": int(ample), "apex": (int((xa + xb) / 2), int(ymax)), "xs": xs, "ys": ys}
    if xb - xa > 0.6 * ample:
        r.update(model="pla", veredicte="FRONTAL", nota="la base es una linia horitzontal (cara frontal o terra dibuixat)")
        return r
    esq = xs <= xa
    dre = xs >= xb
    if esq.sum() < 0.12 * ample or dre.sum() < 0.12 * ample:
        r.update(model="?", veredicte="?", nota="la base es massa estreta per mesurar (arbre, pal, figura?)")
        return r
    sE, dE = aresta_dominant(xs[esq], ys[esq])        # cap a l'apex el perfil baixa: pendent positiu
    sD, dD = aresta_dominant(xs[dre], -ys[dre])       # a la dreta puja: el signe canviat el fa positiu
    r["recta"] = (sE, sD, (dE + dD) / 2)
    r["rodona"] = mitja_el_lipse(xs, ys, ymax)
    r["model"] = "rodona" if r["rodona"][1] > r["recta"][2] + 0.1 else "recta"
    r["encaix"] = r["rodona"][1] if r["model"] == "rodona" else r["recta"][2]
    return r


def veredicte(r, planta="quadrat"):
    if "veredicte" in r:
        return r["veredicte"], r.get("nota", "")
    if planta == "rodona" or (planta == "quadrat" and r["model"] == "rodona"):
        v = r["rodona"][0]
        vals = [v, v]
    else:
        vals = list(r["recta"][:2])
    if any(x is None for x in vals):
        return "?", "no s'ha pogut ajustar una aresta"
    if r["encaix"] < 0.45:
        return "?", "mesura poc fiable: escales, estendards o terra trenquen la base (mira el dibuix)"
    a, b = vals
    if abs(a - b) > 0.25:
        return "GIRAT", "les dues arestes no s'assemblen: la planta no segueix la quadricula"
    m = (a + b) / 2
    obj, nom = min(ESPERAT[planta], key=lambda e: abs(e[0] - m))
    if abs(m - obj) <= MARGE:
        return "OK", f"{nom}" if planta == "hex" else ""
    return ("ALT" if m > obj else "PLA"), f"esperat {obj:.2f}" + (f" ({nom})" if planta == "hex" else "")


def resum(ruta, r, planta):
    v, nota = veredicte(r, planta)
    nom = os.path.basename(ruta)
    if "error" in r:
        return f"{nom}: {r['error']}"
    if r["model"] in ("pla", "?"):
        return f"{nom}: {v} — {nota}"
    def g(p):
        return "?" if p is None else f"{p:.2f} ({math.degrees(math.atan(p)):.0f}°)"
    sE, sD, dins = r["recta"]
    txt = f"{nom}: {v}  arestes esq {g(sE)} / dre {g(sD)}"
    if r["model"] == "rodona" or planta == "rodona":
        txt += f"  el·lipse alt/ample {r['rodona'][0]:.2f}  [planta rodona, encaix {r['rodona'][1]:.0%}]"
    else:
        txt += f"  (encaix {dins:.0%})"
    return txt + (f" — {nota}" if nota else "")


def dibuixa(img, r, sortida, empremta=None):
    """El sprite sobre el rombe de la seva empremta, amb el perfil mesurat
    (cian) i les arestes o l'el·lipse ajustades (groc)."""
    img = img.convert("RGBA")
    ancho, alto = empremta or (0, 0)
    W = (ancho + alto) * 32
    H = (ancho + alto) * 16
    lienzo = Image.new("RGBA", (max(W, img.width) + 48, img.height + H // 2 + 48), (50, 50, 60, 255))
    d = ImageDraw.Draw(lienzo)
    cx = lienzo.width // 2
    base = lienzo.height - 24
    if empremta:
        izq = (cx - alto * 32, base - alto * 16)
        der = (cx + ancho * 32, base - ancho * 16)
        arriba = (cx + (ancho - alto) * 32, base - (ancho + alto) * 16)
        d.polygon([izq, arriba, der, (cx, base)], outline=(255, 220, 80, 255), fill=(90, 80, 60, 255))
        for k in range(1, ancho):
            d.line([(izq[0] + k * 32, izq[1] - k * 16), (cx + k * 32, base - k * 16)], fill=(120, 110, 80, 255))
        for k in range(1, alto):
            d.line([(der[0] - k * 32, der[1] - k * 16), (cx - k * 32, base - k * 16)], fill=(120, 110, 80, 255))
    ax, ay = r.get("apex", (img.width // 2, img.height))
    ox, oy = cx - ax, base - ay
    lienzo.paste(img, (ox, oy), img)
    if "xs" in r:
        for x, y in zip(r["xs"], r["ys"]):
            d.point((ox + x, oy + y + 1), fill=(80, 230, 255, 255))
    if r.get("model") == "recta":
        sE, sD, _ = r["recta"]
        xs = r["xs"]
        for s, x0 in ((sE, xs[0]), (sD, xs[-1])):
            if s is None:
                continue
            x1 = ax
            y0 = ay - s * abs(x1 - x0)
            d.line([(ox + x0, oy + y0), (ox + x1, oy + ay)], fill=(255, 230, 60, 255))
    elif r.get("model") == "rodona" and r["rodona"][2]:
        xc, yw, A, B = r["rodona"][2]
        d.ellipse([ox + xc - A, oy + yw - B, ox + xc + A, oy + yw + B], outline=(255, 230, 60, 255))
    v, _ = veredicte(r)
    d.text((6, 4), v, fill=(255, 230, 60, 255))
    lienzo.save(sortida)


if __name__ == "__main__":
    args = sys.argv[1:]
    planta, empremta, sortida = "quadrat", None, None
    for flag in ("--planta", "--empremta", "-o"):
        if flag in args:
            i = args.index(flag)
            val = args[i + 1]
            del args[i:i + 2]
            if flag == "--planta":
                planta = val
            elif flag == "--empremta":
                empremta = tuple(int(v) for v in val.lower().split("x"))
            else:
                sortida = val
    if not args or planta not in ESPERAT:
        sys.exit(__doc__)
    for ruta in args:
        img = Image.open(ruta)
        r = mesura(img)
        print(resum(ruta, r, planta))
        if (sortida or empremta) and "error" not in r:
            out = sortida if (sortida and len(args) == 1) else os.path.splitext(ruta)[0] + "_perspectiva.png"
            dibuixa(img, r, out, empremta)
