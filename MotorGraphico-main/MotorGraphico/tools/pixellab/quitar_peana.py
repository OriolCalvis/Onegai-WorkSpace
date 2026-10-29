#!/usr/bin/env python3
"""ESTIL/ALMA — Quita la peana (la losa de pavimento que Pixen pinta bajo un
edificio aunque se le pida «no ground») y el fondo plano.

    python3 tools/pixellab/quitar_peana.py entrada.png salida.png [--solo-fondo] [--inset N]

La losa es un rombo 2:1 plano con la casa encima: su esquina delantera es el
punto mas bajo del dibujo y la base de las paredes es otro rombo paralelo,
N px mas arriba (el «inset»). El inset se mide en la columna de la esquina:
canto de la losa, cara superior (colores de la paleta de la losa) y el
primer tramo de 2+ px que no es de esa paleta ni es una junta fina. Luego
se borra, por debajo de las dos rectas de pendiente 1/2 que pasan por la
base de las paredes, todo lo que sea del color de la losa o contorno sin
vecino de color: los barriles, mesas y macetas que estan sobre la losa se
quedan (no son de su color). Si no encuentra losa, no quita nada y lo dice.
"""
import os
import sys

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from limpiar_fondo import limpiar  # noqa: E402

OSCURO = 70          # luminancia por debajo de la cual un pixel es contorno o junta
TOL = 36             # suma de diferencias RGB para «mismo color que la losa»


def _lum(rgb):
    return 0.299 * rgb[..., 0] + 0.587 * rgb[..., 1] + 0.114 * rgb[..., 2]


def _de_paleta(rgb, paleta):
    return np.abs(paleta[None, None, :, :] - rgb[:, :, None, :]).sum(axis=3).min(axis=2) <= TOL


def medir_inset(op, rgb, oscuro, xb, yb):
    """Inset de la base de las paredes respecto a la esquina de la losa.
    1) La paleta de la losa: los colores mas frecuentes en los primeros 8 px
       de color por encima del perfil inferior, a lo largo de toda la base.
    2) En cada columna, desde el perfil se sube mientras el pixel sea de la
       paleta o una junta oscura de 1 px; la mediana de esas subidas (en las
       columnas que acaban en pared, no en borde) es el inset.
    Devuelve (inset, paleta)."""
    h, w = op.shape
    cols = np.where(op.any(axis=0))[0]
    perfil = {int(x): int(np.where(op[:, x])[0].max()) for x in cols}
    muestras = []
    for x, y0 in perfil.items():
        # Los primeros 8 px de color por encima del perfil (saltando el canto
        # oscuro y el contorno), hasta 24 px arriba.
        n = 0
        for d in range(1, 25):
            y = y0 - d
            if y < 0 or not op[y, x]:
                break
            if not oscuro[y, x]:
                muestras.append(rgb[y, x])
                n += 1
                if n >= 8:
                    break
    if len(muestras) < 20:
        return 0, None
    muestras = np.array(muestras)
    cuant = (muestras // 12) * 12 + 6
    unicos, cuenta = np.unique(cuant, axis=0, return_counts=True)
    orden = np.argsort(-cuenta)
    paleta = [unicos[i] for i in orden if cuenta[i] >= 0.04 * len(muestras)][:6]
    paleta = np.array(paleta)
    subidas = []
    for x, y0 in perfil.items():
        if abs(x - xb) > 0.45 * (cols.max() - cols.min()):
            continue                                            # cerca de los lados la losa es mas alta
        y = y0
        while y > 0 and op[y, x] and oscuro[y, x]:
            y -= 1
        while y > 0 and op[y, x]:
            if oscuro[y, x] and (y - 1 >= 0 and op[y - 1, x] and not oscuro[y - 1, x]
                                 and np.abs(paleta - rgb[y - 1, x]).sum(axis=1).min() <= TOL):
                y -= 1                                          # junta fina de 1 px
                continue
            if np.abs(paleta - rgb[y, x]).sum(axis=1).min() > TOL:
                break
            y -= 1
        if y > 0 and op[y, x]:
            subidas.append(y0 - y)
    if len(subidas) < 8:
        return 0, paleta
    return int(np.median(subidas)), paleta


def quitar_peana(img, inset=None):
    a = np.array(img.convert("RGBA"))
    h, w = a.shape[:2]
    op = a[:, :, 3] > 0
    rgb = a[:, :, :3].astype(int)
    oscuro = op & (_lum(rgb) < OSCURO)
    ys, xs = np.where(op)
    if len(xs) == 0:
        return img, 0
    yb = int(ys.max())
    cols = xs[ys == yb]
    xb = int(cols[np.argmin(np.abs(cols - w / 2))])
    medido, paleta = medir_inset(op, rgb, oscuro, xb, yb)
    if paleta is None:
        return img, 0
    inset = inset or medido
    if inset < 3 or inset > 0.4 * (ys.max() - ys.min()):
        return img, 0
    # Por debajo de las rectas de la base de las paredes (pendiente 1/2 desde
    # la esquina), fuera lo que es del color de la losa.
    X, Y = np.meshgrid(np.arange(w), np.arange(h))
    bajo = Y > (yb - inset - 0.5 * np.abs(X - xb))
    quitar = op & bajo & _de_paleta(rgb, paleta)
    # Contornos y juntas de la losa: oscuros bajo la recta sin vecino de color conservado.
    for _ in range(4):
        resto = op & ~quitar
        color = resto & ~oscuro
        vec = np.zeros((h, w), bool)
        vec[1:, :] |= color[:-1, :]
        vec[:-1, :] |= color[1:, :]
        vec[:, 1:] |= color[:, :-1]
        vec[:, :-1] |= color[:, 1:]
        nuevos = resto & oscuro & bajo & ~vec
        if not nuevos.any():
            break
        quitar |= nuevos
    a[quitar] = 0
    return Image.fromarray(a, "RGBA"), int(quitar.sum())


def sin_restos(img, minimo=0.01):
    """Quita las componentes opacas sueltas mas pequenas que minimo del total."""
    from scipy import ndimage
    a = np.array(img.convert("RGBA"))
    et, n = ndimage.label(a[:, :, 3] > 0)
    if n > 1:
        tam = ndimage.sum(np.ones_like(et), et, range(1, n + 1))
        total = tam.sum()
        for i, t in enumerate(tam, 1):
            if t < minimo * total:
                a[et == i] = 0
    return Image.fromarray(a, "RGBA")


if __name__ == "__main__":
    inset = int(sys.argv[sys.argv.index("--inset") + 1]) if "--inset" in sys.argv else None
    args = [x for x in sys.argv[1:] if not x.startswith("--") and x != str(inset)]
    limpiar(args[0], args[1])
    img = Image.open(args[1]).convert("RGBA")
    if "--solo-fondo" not in sys.argv:
        img, n = quitar_peana(img, inset)
        print(f"{args[1]}: peana de {n} px quitada" if n else f"{args[1]}: sin peana que quitar")
        img = sin_restos(img)
    img = img.crop(img.getbbox()) if img.getbbox() else img
    img.save(args[1])
