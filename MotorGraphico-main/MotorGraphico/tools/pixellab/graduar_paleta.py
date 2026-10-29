#!/usr/bin/env python3
"""ARTE-03 — Etalonaje de paleta: acerca una hoja de sprites a la paleta de
Boundington (sombras calidas, piedra gris-calida, tejas terracota) para que
los bloques (que PixelLab pinta con sombras violeta) casen con los monumentos
(sombras negras y medios calidos).

    python3 tools/pixellab/graduar_paleta.py entrada.png salida.png [fuerza=1.0]

Que hace, en HSV: los tonos frios de sombra (violeta/azul 225-325) giran hacia
el ocre (35) por el camino del rojo, pierden la mitad de la saturacion y ganan
un poco de luz; los vinos (330-5, saturados) pasan a terracota (15). Los
azules limpios (pizarra, 190-225) y los verdes no se tocan. Es reversible solo
volviendo a importar la hoja original.
"""
import sys

import numpy as np
from PIL import Image


def graduar(img, fuerza=1.0):
    a = np.array(img.convert("RGBA")).astype(float) / 255.0
    rgb, alpha = a[:, :, :3], a[:, :, 3]
    mx, mn = rgb.max(axis=2), rgb.min(axis=2)
    d = mx - mn + 1e-9
    r, g, b = rgb[:, :, 0], rgb[:, :, 1], rgb[:, :, 2]
    h = np.zeros_like(mx)
    m = mx == r
    h[m] = ((g - b) / d)[m] % 6
    m = mx == g
    h[m] = ((b - r) / d + 2)[m]
    m = mx == b
    h[m] = ((r - g) / d + 4)[m]
    h = (h * 60) % 360
    s = np.where(mx > 0, d / (mx + 1e-9), 0)
    v = mx
    frio = (h >= 225) & (h <= 325)
    peso = np.clip(1 - np.abs(h - 275) / 50, 0, 1) * fuerza
    giro = ((395 - h) % 360) * 0.85          # hacia el 35 por el lado del rojo
    h2 = np.where(frio, (h + giro * peso) % 360, h)
    s2 = np.where(frio, s * (1 - 0.5 * peso), s)
    v2 = np.where(frio, np.clip(v * (1 + 0.08 * peso), 0, 1), v)
    vino = ((h >= 330) | (h <= 5)) & (s > 0.35)
    h2 = np.where(vino, 15.0 * fuerza + h * (1 - fuerza), h2)
    s2 = np.where(vino, s * (1 - 0.1 * fuerza), s2)
    c = v2 * s2
    x = c * (1 - np.abs((h2 / 60) % 2 - 1))
    mm = v2 - c
    hi = (h2 // 60) % 6
    sel = [hi == k for k in range(6)]
    r2 = np.select(sel, [c, x, 0, 0, x, c]) + mm
    g2 = np.select(sel, [x, c, c, x, 0, 0]) + mm
    b2 = np.select(sel, [0, 0, x, c, c, x]) + mm
    out = np.stack([r2, g2, b2, alpha], axis=2)
    return Image.fromarray((np.clip(out, 0, 1) * 255).astype(np.uint8), "RGBA")


if __name__ == "__main__":
    graduar(Image.open(sys.argv[1]), float(sys.argv[3]) if len(sys.argv) > 3 else 1.0).save(sys.argv[2])
    print(f"{sys.argv[2]}: paleta graduada")
