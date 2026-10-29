#!/usr/bin/env python3
"""ISO (VEG-01) — La bardissa retallada dels jardins (el tile «viejo_seto»)
a la manera d'Habbo: un bloc de boix amb el damunt clar, la cara de la llum i
la de l'ombra i grups de fulles arrodonits.

La textura és periòdica cada cel·la (32 unitats), així una filera de blocs
es veu com una sola bardissa: sense línia entre bloc i bloc. Per això no porta
el contorn fosc del kit (cada bloc es dibuixa sol i el contorn faria una
quadrícula): les vores del damunt van un to més fosc i a terra, el més fosc.

Surt en el format de bloc de 64x64 que llegeix tools/pixellab/importar_bloques.py
(el vèrtex de baix del rombe al centre de la vora de baix), així que el motor
el dibuixa com dibuixava el seto de PixelLab.

    python3 tools/iso/bardissa.py      # escriu assets/textures/iso/blocs/seto_pp.png
                                       # i el posa a assets/textures/pixellab/bloques.json
"""
import json
import math
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
ALT = 22                                    # alçada de la bardissa (unitats = píxels)
RAMPA = np.array([(146, 206, 96), (100, 172, 72), (66, 134, 58), (46, 102, 48), (30, 70, 38)], np.uint8)


def _fulles(u, v, llavor):
    """Grups de fulles en un domini periòdic de 32: (to relatiu, dins) per a
    cada punt. +1 llum (la part de dalt d'un grup), -1 ombra (la de baix)."""
    rng = np.random.default_rng(llavor)
    centres = [((i + 0.5) * 8 + rng.uniform(-2, 2), (j + 0.5) * 8 + rng.uniform(-2, 2)) for i in range(4) for j in range(4)]
    millor, to = 9.9, 0
    for (cu, cv) in centres:
        du = (u - cu + 16) % 32 - 16
        dv = (v - cv + 16) % 32 - 16
        d = (du / 3.6) ** 2 + (dv / 2.8) ** 2
        if d < millor:
            millor = d
            to = 1 if (d < 1.0 and dv < -0.6) else (-1 if (0.55 < d < 1.25 and dv > 0.8) else 0)
    return to


def imatge_bloc(llavor=5):
    im = np.zeros((64, 64, 4), np.uint8)
    for py in range(64):
        for px in range(64):
            # el damunt (Z = ALT)
            s = 2 * (py - 32 + ALT)
            X, Y = ((px - 32) + s) / 2, (s - (px - 32)) / 2
            if 0 <= X <= 32 and 0 <= Y <= 32:
                t = 1 + {1: -1, -1: 1, 0: 0}[_fulles(X, Y, llavor)]
                if X > 31 or Y > 31 or X < 1 or Y < 1:
                    t = 2                                      # les vores del damunt, un to més fosc
                im[py, px] = (*RAMPA[t], 255)
                continue
            if px < 32:                                        # cara +Y (la de la llum)
                X, Z = px, 48 + px / 2 - py
                base, u, v = 2, X, ALT - Z
            else:                                              # cara +X (la de l'ombra)
                Y, Z = 64 - px, 48 + (64 - px) / 2 - py
                base, u, v = 3, 32 - Y, ALT - Z
            if not (0 <= Z <= ALT):
                continue
            t = base + {1: -1, -1: 1, 0: 0}[_fulles(u, v * 1.0 + 3, llavor + 1)]
            if Z < 1.5:
                t = 4                                          # a tocar de terra
            im[py, px] = (*RAMPA[min(4, t)], 255)
    return Image.fromarray(im, "RGBA")


def main():
    desti = ROOT / "assets/textures/iso/blocs"
    desti.mkdir(parents=True, exist_ok=True)
    imatge_bloc().save(desti / "seto_pp.png")
    index = ROOT / "assets/textures/pixellab/bloques.json"
    dades = json.loads(index.read_text())
    if dades["materiales"].get("seto") != {"pp": "../iso/blocs/seto_pp.png"}:
        dades["materiales"]["seto"] = {"pp": "../iso/blocs/seto_pp.png"}
        index.write_text(json.dumps(dades, ensure_ascii=False, indent=2) + "\n")
    print("seto_pp.png (bardissa del kit) -> python3 tools/pixellab/importar_bloques.py")


if __name__ == "__main__":
    main()
