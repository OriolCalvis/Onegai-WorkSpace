#!/usr/bin/env python3
"""ESTIL — Mapa un sprite (o una hoja sencera) a la paleta mestra: cada
color opac passa al color mes proper de assets/textures/pixellab/estil/
paleta_mestra.json (distancia en CIELAB, sense dithering), l'alfa es
conserva. Aixi arbres, props, PNJ i edificis comparteixen EXACTAMENT els
mateixos colors, que es la meitat de la coherencia d'estil.

    python3 tools/pixellab/aplicar_paleta.py entrada.png sortida.png [--rampes pedra,socol,...] [--informe]

--rampes limita el mapatge a unes rampes (una casa no ha d'agafar colors de
pell; un PNJ no hauria d'agafar verdet). --informe escriu quants colors tenia
i quants li queden. Es idempotent: aplicar-ho dos cops no canvia res.
"""
import json
import os
import sys

import numpy as np
from PIL import Image

RAIZ = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
PALETA = os.path.join(RAIZ, "assets", "textures", "pixellab", "estil", "paleta_mestra.json")


def _hex(c):
    return tuple(int(c[i:i + 2], 16) for i in (1, 3, 5))


def _lab(rgb):
    """sRGB (0-255, Nx3) -> CIELAB."""
    c = rgb.astype(float) / 255.0
    c = np.where(c > 0.04045, ((c + 0.055) / 1.055) ** 2.4, c / 12.92)
    m = np.array([[0.4124, 0.3576, 0.1805], [0.2126, 0.7152, 0.0722], [0.0193, 0.1192, 0.9505]])
    xyz = c @ m.T / np.array([0.95047, 1.0, 1.08883])
    f = np.where(xyz > 0.008856, np.cbrt(xyz), 7.787 * xyz + 16 / 116)
    return np.stack([116 * f[:, 1] - 16, 500 * (f[:, 0] - f[:, 1]), 200 * (f[:, 1] - f[:, 2])], axis=1)


def colors_paleta(rampes=None):
    p = json.load(open(PALETA))
    cols = []
    for nom, lista in p["rampes"].items():
        if rampes and nom not in rampes:
            continue
        cols += [_hex(c) for c in lista]
    cols += [_hex(p["ombra"]), _hex(p["llum"])]
    return np.array(sorted(set(cols)), dtype=np.uint8)


def aplicar(img, rampes=None):
    a = np.array(img.convert("RGBA"))
    opac = a[:, :, 3] > 0
    px = a[opac][:, :3]
    if len(px) == 0:
        return img.convert("RGBA"), 0, 0
    unics, inv = np.unique(px, axis=0, return_inverse=True)
    pal = colors_paleta(rampes)
    lu, lp = _lab(unics), _lab(pal)
    d = ((lu[:, None, :] - lp[None, :, :]) ** 2).sum(axis=2)
    nou = pal[d.argmin(axis=1)]
    out = a.copy()
    out[opac, :3] = nou[inv]
    return Image.fromarray(out, "RGBA"), len(unics), len(np.unique(nou, axis=0))


if __name__ == "__main__":
    args = [x for x in sys.argv[1:] if not x.startswith("--")]
    rampes = None
    for i, x in enumerate(sys.argv):
        if x == "--rampes":
            rampes = set(sys.argv[i + 1].split(","))
    img, antes, despues = aplicar(Image.open(args[0]), rampes)
    img.save(args[1])
    print(f"{args[1]}: {antes} colors -> {despues} de la paleta mestra")
