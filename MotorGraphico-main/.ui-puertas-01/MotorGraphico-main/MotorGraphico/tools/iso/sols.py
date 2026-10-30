#!/usr/bin/env python3
"""ISO (VEG-01) — La gespa i l'herba amb flors del terra, a la manera d'Habbo:
un verd pla amb uns quants floquets d'herba (una V clara amb l'ombra a sota)
i, a l'herba florida, flors menudes de colors. Cap vora fosca: la textura és
periòdica cada cel·la (32 unitats en X i en Y), així el prat es veu continu,
sense quadrícula.

Cada rajola és el rombe de 64x32 (la cara de dalt); tools/pixellab/importar_tiles.py
la posa a terreno_mundo.png (y0 = 0). Aquest script escriu les imatges a
assets/textures/iso/sols/, apunta les baldoses del terra a
assets/textures/pixellab/tiles.json i, després, cal:

    python3 tools/iso/sols.py
    python3 tools/iso/rajoles_kit.py                      # les rajoles del kit fan servir el terra del barri
    python3 tools/pixellab/importar_tiles.py assets/textures/pixellab/tiles.json
"""
import json
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
GESPA = np.array([(176, 220, 112), (146, 202, 92), (120, 184, 78), (96, 158, 66), (74, 128, 56)], np.uint8)
FLORS = {
    "blanca": [(255, 255, 255), (232, 232, 222)],
    "groga": [(255, 244, 140), (236, 196, 64)],
    "rosa": [(255, 184, 214), (226, 108, 156)],
    "blava": [(184, 210, 255), (112, 146, 232)],
}


def _punts(n, llavor, marge=0.0):
    """n punts del domini periòdic 32x32 (X, Y), repartits en una graella amb joc."""
    if n <= 0:
        return []
    rng = np.random.default_rng(llavor)
    costat = int(np.ceil(np.sqrt(n)))
    pas = 32 / costat
    out = []
    for i in range(costat):
        for j in range(costat):
            if len(out) < n:
                out.append(((i + 0.5) * pas + rng.uniform(-pas * 0.3, pas * 0.3),
                            (j + 0.5) * pas + rng.uniform(-pas * 0.3, pas * 0.3)))
    return out


def _pinta(img, X, Y, color):
    """Un píxel del terra a les coordenades (X, Y) del domini periòdic: es repeteix
    a les quatre cantonades perquè el que surt per una vora entri per l'altra."""
    for ox in (-32, 0, 32):
        for oy in (-32, 0, 32):
            x, y = X + ox, Y + oy
            px, py = int(np.floor(32 + x - y)), int(np.floor((x + y) / 2))
            if 0 <= px < 64 and 0 <= py < 32 and img[py, px, 3]:
                img[py, px, :3] = color


def _rombe():
    img = np.zeros((32, 64, 4), np.uint8)
    for py in range(32):
        for px in range(64):
            X, Y = (px + 0.5 - 32 + 2 * (py + 0.5)) / 2, (2 * (py + 0.5) - (px + 0.5 - 32)) / 2
            if 0 <= X <= 32 and 0 <= Y <= 32:
                img[py, px] = (*GESPA[2], 255)
    return img


def gespa(llavor=11, floquets=10, flors=0):
    img = _rombe()
    rng = np.random.default_rng(llavor)
    # taques suaus d'un to més clar i d'un de més fosc (pocs píxels, formes rodones)
    for (cx, cy), to in zip(_punts(6, llavor + 1), (1, 3, 1, 3, 1, 3)):
        r = 2.2 + rng.random() * 1.2
        for dx in np.arange(-r, r + 0.01, 0.5):
            for dy in np.arange(-r, r + 0.01, 0.5):
                if dx * dx + dy * dy <= r * r and rng.random() < 0.55:
                    _pinta(img, cx + dx, cy + dy, GESPA[to])
    # floquets d'herba: una V clara (dos píxels a dalt) amb l'ombra a sota
    for (cx, cy) in _punts(floquets, llavor + 2):
        for (dx, dy, to) in ((-0.5, -1.0, 0), (1.5, -1.0, 0), (0.5, 0.0, 1), (0.5, 1.0, 4), (1.5, 0.5, 3)):
            _pinta(img, cx + dx + dy, cy - dx + dy, GESPA[to])
    # flors: una creu de quatre píxels del color i el cor groc
    colors = list(FLORS)
    for k, (cx, cy) in enumerate(_punts(flors, llavor + 3)):
        c = FLORS[colors[(k + llavor) % len(colors)]]
        for (dx, dy, t) in ((0, -1, 0), (-1, 0, 0), (1, 0, 1), (0, 1, 1)):
            _pinta(img, cx + dx + dy, cy - dx + dy, c[t])
        _pinta(img, cx, cy, FLORS["groga"][1] if colors[(k + llavor) % len(colors)] != "groga" else (255, 255, 255))
    return Image.fromarray(img, "RGBA")


SOLS = {"pradera": dict(llavor=11, floquets=10, flors=0), "viejo_cesped": dict(llavor=13, floquets=9, flors=0),
        "hierba_flores": dict(llavor=17, floquets=7, flors=6)}


def main():
    desti = ROOT / "assets/textures/iso/sols"
    desti.mkdir(parents=True, exist_ok=True)
    for nom, p in SOLS.items():
        gespa(**p).save(desti / f"{nom}.png")
    index = ROOT / "assets/textures/pixellab/tiles.json"
    dades = json.loads(index.read_text())
    canvis = 0
    for b in dades["baldosas"]:
        if b["nombre"] in SOLS:
            nou = {"imagen": f"../iso/sols/{b['nombre']}.png", "y0": 0}
            if any(b.get(k) != v for k, v in nou.items()):
                b.update(nou)
                b["nota"] = "VEG-01: gespa del kit (tools/iso/sols.py), periodica: sense quadricula"
                canvis += 1
    index.write_text(json.dumps(dades, ensure_ascii=False, indent=1) + "\n")
    print(f"{len(SOLS)} sols del kit a {desti.relative_to(ROOT)}; {canvis} baldoses apuntades -> rajoles_kit.py i importar_tiles.py")


if __name__ == "__main__":
    main()
