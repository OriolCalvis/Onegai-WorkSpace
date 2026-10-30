#!/usr/bin/env python3
"""MORAL-02 — Las alas de energía de la moral: un atlas con las alas de luz y de sombra en ocho niveles.

Oriol (08/10/2026), sobre el aura de MORAL-01: «vull que sigui mes extens, que el feedback visual vagi canviant
mes progresivament [...] que apareixes primer nomes 1 ala, despres 2 per cada banda, despres ja 4 despres 6 fins
a 8 [...] arribar a ser un heroi de regio no es el mateix que ser l'heroi del mon sencer». De las alas: «la forma
de les ales d'energia», con los «patrons del marbre [...] amb els fractals de color»; para el bueno, «marfil,
daurat i blau cel», y para el malo, «morat, negre i vermell».

Cada ala es un abanico de hojas de energía que nacen del hombro (el pivote de la celda) y se abren hacia fuera:
una hoja en el nivel 1 y 2, dos en el 3 y 4, cuatro en el 5, seis en el 6 y ocho en el 7 y 8; cada nivel, más
largas y más trabajadas (desde el 4, con dientes en el canto de fuera; el 8, con una segunda capa de hojas cortas).
Las de luz son plumas finas y afiladas; las de sombra, hojas anchas (PLUMA).

Las hojas van por pisos, separados en horizontal (Oriol, 08/10: «l'element principal que vull integrar, es el de
la separacio hortizontal»): el de arriba (plumas que suben), el del medio (la grande, hacia fuera) y el de abajo
(hacia fuera y abajo), cada uno naciendo un poco más abajo en la espalda, juntos y con las plumas rectas. Un piso
en los niveles 1 y 2, dos en el 3 y el 4, y tres desde el 5; cuando entra un piso, los que había bajan un poco, y de
nivel en nivel crecen, sobre todo el de abajo.
El relleno es de mármol: bandas que salen del hombro hacia la punta, torcidas por un ruido fractal, con vetas
claras; los colores, la rampa de cada lado. Los cuatro frames desplazan las bandas: la energía corre hacia fuera.

Solo el ala derecha (la izquierda es su espejo; las dibuja Render/AuraJugador). El atlas:

    assets/textures/alas_moral.png   celdas de CELDA x CELDA: fila = (lado, nivel), columna = frame
                                     filas 0..7 la luz (niveles 1..8), 8..15 la sombra; 4 columnas
    assets/textures/alas_moral.json  el contrato (celda, pivote, niveles, frames, filas)

El motor (Game/AuraMoral.h) lleva los mismos números: tools/test_validar_sprites.py comprueba que casan.

    python3 tools/gen_alas_moral.py                 # escribe el atlas en assets/textures
    python3 tools/gen_alas_moral.py --salida DIR    # o en otra carpeta
"""
import argparse
import json
import math
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent.parent
TEX = ROOT / "assets/textures"

CELDA = 96          # lado de cada celda del atlas
PIVOTE = (10, 48)   # el hombro: donde nace el ala derecha (x, y) dentro de la celda
NIVELES = 8
FRAMES = 4
LADOS = ("luz", "sombra")
SUPER = 4           # las hojas se rasterizan a 4x y se reducen por mayoría: canto limpio

# Las rampas de cada lado, de oscuro a claro (las bandas del mármol), la veta y el canto. La luz: azul cielo,
# marfil y dorado; la sombra: negro, morado y rojo. (Oriol, 08/10, de la segunda tirada de esbozos: «el disseny
# aquest bàsicament».)
RAMPAS = {
    "luz": ["#2c62b4", "#4f93e0", "#8ccdfa", "#d6f0ff", "#fff8e6", "#ffe7a3", "#f7c64a", "#d99a22"],
    "sombra": ["#09040c", "#1b0826", "#36104e", "#5c1d86", "#8a2fb8", "#a3123a", "#d61f3c", "#ff5a52"],
}
VETA = {"luz": "#ffffff", "sombra": "#ff3d6e"}
CANTO = {"luz": "#fffbe8", "sombra": "#ff2e46"}


def _rgb(h):
    h = h.lstrip("#")
    return np.array([int(h[i:i + 2], 16) for i in (0, 2, 4)], np.float32)


# --- los niveles ---------------------------------------------------------------------------------

# Los pisos: dónde nace cada uno respecto al hombro (x, y) y cuánto se dobla su espina (casi nada: rectas, que
# con curva parecían colas de kitsune y no alas, Oriol 08/10). Los tres nacen juntos, en el centro de la espalda
# («centra y junta el nacimiento de las tres capas»), uno apenas encima del otro.
PISO_ARRIBA, PISO_MEDIO, PISO_ABAJO = 0, 1, 2
RAIZ_PISO = {PISO_ARRIBA: (0.0, -1.0), PISO_MEDIO: (0.0, 0.0), PISO_ABAJO: (0.0, 2.0)}
CURVA_PISO = {PISO_ARRIBA: 0.04, PISO_MEDIO: 0.04, PISO_ABAJO: -0.02}

# Cada nivel: el largo de la hoja mayor, el ancho, si lleva dientes, si lleva la segunda capa de hojas cortas y sus
# hojas (piso, ángulo en grados con 0 = hacia fuera y -90 = arriba, largo relativo, ancho relativo). La del medio
# es la grande; cuando entra un piso nuevo, las que había bajan un poco; las de abajo, poco caídas (Oriol, 08/10).
NIVEL = {
    1: (15, 4.5, False, False, [(PISO_MEDIO, -30, 1.0, 1.0)]),
    2: (21, 6.0, False, False, [(PISO_MEDIO, -30, 1.0, 1.0)]),
    3: (25, 6.0, False, False, [(PISO_MEDIO, -24, 1.0, 1.0), (PISO_ABAJO, 8, 0.62, 0.9)]),
    4: (30, 7.0, True, False, [(PISO_MEDIO, -24, 1.0, 1.0), (PISO_ABAJO, 8, 0.7, 0.95)]),
    5: (34, 7.0, True, False, [(PISO_ARRIBA, -42, 0.7, 0.85),
                               (PISO_MEDIO, -12, 1.0, 1.15),
                               (PISO_ABAJO, 6, 0.74, 0.95), (PISO_ABAJO, 15, 0.64, 0.9)]),
    6: (40, 7.5, True, False, [(PISO_ARRIBA, -46, 0.74, 0.85), (PISO_ARRIBA, -36, 0.68, 0.85),
                               (PISO_MEDIO, -12, 1.0, 1.2),
                               (PISO_ABAJO, 5, 0.84, 1.0), (PISO_ABAJO, 12, 0.78, 0.95), (PISO_ABAJO, 19, 0.7, 0.9)]),
    7: (45, 7.5, True, False, [(PISO_ARRIBA, -48, 0.76, 0.85), (PISO_ARRIBA, -38, 0.7, 0.85),
                               (PISO_MEDIO, -14, 1.0, 1.2), (PISO_MEDIO, -5, 0.84, 1.0),
                               (PISO_ABAJO, 6, 0.86, 1.0), (PISO_ABAJO, 12, 0.8, 0.95), (PISO_ABAJO, 18, 0.73, 0.9),
                               (PISO_ABAJO, 24, 0.64, 0.85)]),
    8: (50, 8.0, True, True, [(PISO_ARRIBA, -49, 0.78, 0.9), (PISO_ARRIBA, -39, 0.72, 0.9),
                              (PISO_MEDIO, -14, 1.0, 1.25), (PISO_MEDIO, -5, 0.86, 1.05),
                              (PISO_ABAJO, 6, 0.88, 1.05), (PISO_ABAJO, 12, 0.82, 1.0), (PISO_ABAJO, 18, 0.75, 0.95),
                              (PISO_ABAJO, 24, 0.66, 0.9)]),
}


def hojas(nivel):
    """Cuántas hojas lleva cada ala en un nivel."""
    return len(NIVEL[nivel][4])


def pisos(nivel):
    """Cuántos pisos tiene el ala en un nivel."""
    return len({h[0] for h in NIVEL[nivel][4]})


# --- la forma ------------------------------------------------------------------------------------

def _bezier(p0, p1, p2, t):
    a = (1 - t) ** 2
    b = 2 * (1 - t) * t
    c = t ** 2
    return (a * p0[0] + b * p1[0] + c * p2[0], a * p0[1] + b * p1[1] + c * p2[1])


# Cómo es la pluma de cada lado. Oriol (08/10): «hazme mas finas las "plumas" de las alas de luz que has hecho»: las
# de luz, finas, con la punta larga y una sierra menuda (como las barbas de una pluma); las de sombra, anchas y con
# dientes.
PLUMA = {
    "luz": {"ancho": 0.55, "dientes": 0.28, "frecuencia": 15.0, "punta": 1.25},
    "sombra": {"ancho": 1.0, "dientes": 0.45, "frecuencia": 9.0, "punta": 0.85},
}


def contorno_hoja(raiz, angulo, largo, ancho, dientes, curva=0.22, muestras=64, estilo=PLUMA["sombra"]):
    """El polígono de una hoja de energía: la espina es una curva que se dobla hacia arriba; el ancho sube rápido
    desde la raíz y se afila hasta la punta. Con dientes, el canto de fuera (el de abajo) lleva sierra."""
    a = math.radians(angulo)
    d = (math.cos(a), math.sin(a))
    perp = (d[1], -d[0])  # a la izquierda de la dirección: hacia arriba y adentro
    p0 = raiz
    p2 = (raiz[0] + d[0] * largo, raiz[1] + d[1] * largo)
    p1 = (raiz[0] + d[0] * largo * 0.5 + perp[0] * largo * curva, raiz[1] + d[1] * largo * 0.5 + perp[1] * largo * curva)
    arriba, abajo = [], []
    for i in range(muestras + 1):
        t = i / muestras
        x, y = _bezier(p0, p1, p2, t)
        t2 = min(1.0, t + 1e-3)
        x2, y2 = _bezier(p0, p1, p2, t2)
        tx, ty = x2 - x, y2 - y
        norma = math.hypot(tx, ty) or 1.0
        nx, ny = ty / norma, -tx / norma
        if t < 0.3:
            w = ancho * math.sin(math.pi / 2 * t / 0.3) ** 0.7
        else:
            w = ancho * (1 - (t - 0.3) / 0.7) ** estilo["punta"]
        w_in, w_out = w * 0.45, w * 0.55
        if dientes and 0.25 < t < 0.92:
            fase = (t - 0.25) * estilo["frecuencia"]
            w_out += ancho * estilo["dientes"] * (fase - math.floor(fase)) * (1 - t)
        arriba.append((x + nx * w_in, y + ny * w_in))
        abajo.append((x - nx * w_out, y - ny * w_out))
    return arriba + abajo[::-1]


def mascara(poligonos):
    """La máscara (CELDA x CELDA) de unos polígonos: a 4x y reducida por mayoría."""
    grande = Image.new("L", (CELDA * SUPER, CELDA * SUPER), 0)
    d = ImageDraw.Draw(grande)
    for pol in poligonos:
        d.polygon([(x * SUPER, y * SUPER) for x, y in pol], fill=255)
    a = np.asarray(grande, np.float32).reshape(CELDA, SUPER, CELDA, SUPER).mean(axis=(1, 3))
    return a > 110


def hojas_del_nivel(nivel, lado="luz"):
    """Los polígonos de las plumas del ala derecha de un nivel y un lado, piso a piso."""
    largo, ancho, dientes, segunda, lista = NIVEL[nivel]
    estilo = PLUMA[lado]
    ancho *= estilo["ancho"]
    out = []
    for piso, ang, rel, relancho in lista:
        dx, dy = RAIZ_PISO[piso]
        raiz = (PIVOTE[0] + dx, PIVOTE[1] + dy)
        out.append(contorno_hoja(raiz, ang, largo * rel, ancho * relancho * (0.8 + 0.2 * rel), dientes,
                                 curva=CURVA_PISO[piso], estilo=estilo))
    if segunda:  # la capa de dentro: hojas cortas entre las largas de cada piso
        for piso, ang, rel, relancho in lista:
            dx, dy = RAIZ_PISO[piso]
            out.append(contorno_hoja((PIVOTE[0] + dx + 1, PIVOTE[1] + dy), ang + 7, largo * rel * 0.55,
                                     ancho * relancho * 0.7, False, curva=0.15, estilo=estilo))
    return out


# --- el mármol -----------------------------------------------------------------------------------

# El mármol se pensó en celdas de 80 px con el hombro en (8, 50) (la tirada que eligió Oriol): el ruido se
# muestrea a esa escala y respecto al hombro, así la celda más grande lleva el mismo dibujo y más margen.
REF_LADO, REF_PIVOTE = 80, (8, 50)


def _ruido(semilla, celdas):
    """Ruido de valor (CELDA x CELDA) desde una rejilla aleatoria de `celdas` celdas por cada REF_LADO px,
    interpolado suave."""
    rng = np.random.default_rng(semilla)
    g0 = rng.random((celdas + 2, celdas + 2)).astype(np.float32)
    paso = celdas / REF_LADO
    borde = 4
    n = int(math.ceil(CELDA * paso)) + 2 * borde + 2
    g = np.random.default_rng(semilla + 7919).random((n, n)).astype(np.float32)
    g[borde:borde + celdas + 2, borde:borde + celdas + 2] = g0
    ys = (np.arange(CELDA, dtype=np.float32) - (PIVOTE[1] - REF_PIVOTE[1])) * paso + borde
    xs = (np.arange(CELDA, dtype=np.float32) - (PIVOTE[0] - REF_PIVOTE[0])) * paso + borde
    y0, x0 = np.floor(ys).astype(int), np.floor(xs).astype(int)
    fy, fx = ys - y0, xs - x0
    fy, fx = fy * fy * (3 - 2 * fy), fx * fx * (3 - 2 * fx)
    a = g[np.ix_(y0, x0)]
    b = g[np.ix_(y0, x0 + 1)]
    c = g[np.ix_(y0 + 1, x0)]
    d = g[np.ix_(y0 + 1, x0 + 1)]
    fx2, fy2 = fx[None, :], fy[:, None]
    return (a * (1 - fx2) + b * fx2) * (1 - fy2) + (c * (1 - fx2) + d * fx2) * fy2


def fbm(semilla, octavas=4, base=4):
    out = np.zeros((CELDA, CELDA), np.float32)
    amp, total = 1.0, 0.0
    for o in range(octavas):
        out += amp * _ruido(semilla + 101 * o, base * 2 ** o)
        total += amp
        amp *= 0.5
    return out / total


def marmol(lado, nivel, frame):
    """El color (CELDA x CELDA x 3) y la veta (bool) del mármol de un nivel y un frame: bandas que salen del
    hombro, muy torcidas por un ruido fractal (el mármol), y una segunda capa de ruido que reparte los colores
    (los fractales de color): en la luz, del azul al oro pasando por el marfil; en la sombra, del negro al rojo
    pasando por el morado. Cada frame corre las bandas hacia fuera."""
    ys, xs = np.mgrid[0:CELDA, 0:CELDA].astype(np.float32)
    r = np.hypot(xs - PIVOTE[0], ys - PIVOTE[1])
    semilla = 7 + (0 if lado == "luz" else 50)
    torcido = fbm(semilla, 5, 3) * 2 - 1
    torcido2 = fbm(semilla + 13, 4, 6) * 2 - 1
    fase = frame / FRAMES * 2 * math.pi
    m = np.sin(r * 0.22 - fase + torcido * 7.5 + torcido2 * 3.0)
    rampa = [_rgb(c) for c in RAMPAS[lado]]
    zona = fbm(semilla + 29, 4, 3)
    v = np.where(zona > 0.5, 0.5 + (m + 1) / 4, 0.5 - (m + 1) / 4)
    v = np.clip(v + (zona - 0.5) * 0.3, 0, 0.999)
    idx = (v * len(rampa)).astype(int)
    color = np.stack(rampa)[idx]
    veta = np.abs(m) < 0.07 + 0.006 * nivel
    return color, veta


# --- el atlas ------------------------------------------------------------------------------------

def celda(lado, nivel, frame):
    """La celda RGBA de un ala (la derecha)."""
    hojas = hojas_del_nivel(nivel, lado)
    mask = mascara(hojas)
    color, veta = marmol(lado, nivel, frame)
    out = np.zeros((CELDA, CELDA, 4), np.uint8)
    out[mask, :3] = color[mask].astype(np.uint8)
    out[mask & veta, :3] = _rgb(VETA[lado]).astype(np.uint8)
    # el canto: los píxeles de la máscara que tocan fuera
    borde = mask & ~(np.roll(mask, 1, 0) & np.roll(mask, -1, 0) & np.roll(mask, 1, 1) & np.roll(mask, -1, 1))
    out[borde, :3] = _rgb(CANTO[lado]).astype(np.uint8)
    # alfa: la energía se desvanece hacia la punta (más lejos del hombro, más transparente)
    ys, xs = np.mgrid[0:CELDA, 0:CELDA].astype(np.float32)
    r = np.hypot(xs - PIVOTE[0], ys - PIVOTE[1])
    largo = NIVEL[nivel][0]
    alfa = np.clip(1.15 - 0.55 * (r / largo) ** 2, 0.45, 1.0)
    if lado == "sombra":
        alfa = np.clip(alfa + 0.15, 0, 1)
    out[mask, 3] = (alfa[mask] * 255).astype(np.uint8)
    return out


def atlas():
    filas = []
    for lado in LADOS:
        for nivel in range(1, NIVELES + 1):
            filas.append(np.concatenate([celda(lado, nivel, f) for f in range(FRAMES)], axis=1))
    return np.concatenate(filas, axis=0)


def contrato():
    return {
        "_nota": ("MORAL-02 — Las alas de energía de la moral (tools/gen_alas_moral.py; no editar a mano). Cada "
                  "celda es el ala derecha de un lado y un nivel en un frame; la izquierda es su espejo. El pivote "
                  "es el hombro. Filas: la luz (niveles 1..8) y después la sombra; columnas: los frames."),
        "image": "alas_moral.png", "celda": CELDA, "pivote": list(PIVOTE), "niveles": NIVELES, "frames": FRAMES,
        "lados": list(LADOS),
        "hojasPorNivel": {str(n): hojas(n) for n in NIVEL},
        "pisosPorNivel": {str(n): pisos(n) for n in NIVEL},
    }


def generar(salida=TEX):
    salida = Path(salida)
    img = atlas()
    Image.fromarray(img, "RGBA").save(salida / "alas_moral.png", optimize=True)
    (salida / "alas_moral.json").write_text(json.dumps(contrato(), ensure_ascii=False, indent=1) + "\n",
                                            encoding="utf-8")
    return img


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--salida", default=str(TEX))
    args = ap.parse_args()
    generar(args.salida)
    print(f"{args.salida}/alas_moral.png: {len(LADOS)} lados x {NIVELES} niveles x {FRAMES} frames de {CELDA}px")
