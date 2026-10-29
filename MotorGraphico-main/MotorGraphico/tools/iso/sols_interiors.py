#!/usr/bin/env python3
"""ISO (INT-01) — El terra dels interiors: taulons, parquet d'espiga, lloses,
marbre, rajola dels banys, terra batuda, sorra, tatami i les catifes (en nou
peces: el camp, les quatre vores i les quatre cantonades, perquè el
generador en faci de qualsevol mida).

Cada rajola és el rombe de 64x32, amb la textura en coordenades del món
(X, Y de 0 a 32 dins la cel·la) i periòdica cada cel·la: els taulons i les
catifes continuen d'una cel·la a l'altra sense quadrícula. Escriu les
imatges a assets/textures/iso/sols_int/ i les dona d'alta com a baldoses
noves a assets/textures/pixellab/tiles.json («nuevas», y0 = 0); després:

    python3 tools/iso/sols_interiors.py
    python3 tools/pixellab/importar_tiles.py assets/textures/pixellab/tiles.json
"""
import json
import os
import sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sales  # noqa: E402,F401  (materials de les sales)
from peces_iso import MATERIALS  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
DESTI = ROOT / "assets/textures/iso/sols_int"


def _h(a, b, s=0):
    h = (np.floor(a).astype(np.int64) * 73856093) ^ (np.floor(b).astype(np.int64) * 19349663) ^ (s * 83492791)
    return (h & 0xFFFF) / 65535.0


def _rampa(mat):
    return np.array(MATERIALS[mat], np.uint8)


def _taulons(X, Y, mat="fusta", ample=8, llavor=1):
    """Taulons al llarg de X, d'amples iguals, amb les juntes a trencajunts."""
    files = 32 // ample
    fila = np.floor(Y / ample)
    off = np.array([0, 13, 6, 21, 10, 3, 17, 26])[np.mod(fila, 8).astype(int)]
    xx = np.mod(X + off, 32)
    to = 1 + (np.mod(fila, 2) == 1).astype(int) * 0
    to = np.where(_h(fila, np.floor((X + off) / 32), llavor) > 0.5, 1, 2)
    to = np.where(np.mod(Y, ample) < 1, 3, to)
    to = np.where(xx < 1, 3, to)
    to = np.where((np.mod(Y, ample) >= ample - 1) & (to < 3), np.minimum(to + 0, 2), to)
    veta = (_h(np.floor(X / 5), np.floor(Y), llavor + 5) > 0.9) & (to < 3)
    to = np.where(veta, to + 1, to)
    del files
    return _rampa(mat)[np.clip(to, 0, 4)]


def _espiga(X, Y, mat="noguera", llavor=2):
    """Parquet d'espiga: peces de 8 x 4 que alternen la direcció cada 8."""
    bx, by = np.floor(X / 8), np.floor(Y / 8)
    hor = np.mod(bx + by, 2) == 0
    u, v = np.mod(X, 8), np.mod(Y, 8)
    junta = np.where(hor, np.mod(v, 4) < 1, np.mod(u, 4) < 1) | (u < 0.7) | (v < 0.7)
    peca = np.where(hor, np.floor(Y / 4), np.floor(X / 4))
    to = np.where(_h(peca, bx * 3 + by, llavor) > 0.5, 1, 2)
    to = np.where(junta, 3, to)
    return _rampa(mat)[to]


def _lloses(X, Y, mat="pedra_castell", costat=16, llavor=3):
    a, b = np.floor(X / costat), np.floor(Y / costat)
    to = np.where(_h(a, b, llavor) > 0.55, 1, 2)
    junta = (np.mod(X, costat) < 1) | (np.mod(Y, costat) < 1)
    taca = (_h(np.floor(X / 2), np.floor(Y / 2), llavor + 9) > 0.93) & ~junta
    to = np.where(taca, to + 1, to)
    to = np.where(junta, 3, to)
    return _rampa(mat)[to]


def _escacs(X, Y, mat_a="pedra_clara", mat_b="estuc_rosa", costat=16, llavor=4):
    a, b = np.floor(X / costat), np.floor(Y / costat)
    clar = np.mod(a + b, 2) == 0
    vena = np.abs(np.mod(X + Y * 0.5 + 9 * _h(a, b, llavor), 13) - 6.5) < 0.35
    to = np.where(vena, 2, 1)
    junta = (np.mod(X, costat) < 0.8) | (np.mod(Y, costat) < 0.8)
    ca, cb = _rampa(mat_a)[np.where(junta, 3, to)], _rampa(mat_b)[np.where(junta, 3, to + 1)]
    return np.where(clar[..., None], ca, cb)


def _rajola(X, Y, llavor=5):
    a, b = np.floor(X / 8), np.floor(Y / 8)
    blava = np.mod(a + b, 4) == 0
    junta = (np.mod(X, 8) < 1) | (np.mod(Y, 8) < 1)
    c = np.where(blava[..., None], _rampa("estuc_blau")[1], _rampa("rajola")[1])
    return np.where(junta[..., None], _rampa("rajola")[3], c)


def _batuda(X, Y, mat="terra", llavor=6):
    to = np.where(_h(np.floor(X / 3), np.floor(Y / 3), llavor) > 0.7, 2, 1)
    pedreta = _h(np.floor(X), np.floor(Y), llavor + 1) > 0.97
    return _rampa(mat)[np.where(pedreta, 3, to)]


def _tatami(X, Y, llavor=7):
    vora = (np.mod(Y, 32) < 2) | (np.mod(Y, 32) >= 31)
    trena = np.mod(X, 2) < 1
    c = np.where(trena[..., None], _rampa("encenall")[0], _rampa("encenall")[1])
    return np.where(vora[..., None], _rampa("porticons")[3], c)


def _catifa(X, Y, part, camp="vellut", vora="or", llavor=8):
    """Catifa: el camp amb un rombe de dibuix i, als costats que diu part
    (xm, xp, ym, yp), la sanefa: dos fils d'or i una banda fosca."""
    a, b = np.mod(X + Y, 16), np.mod(X - Y, 16)
    to = np.where((np.abs(a - 8) < 1) | (np.abs(b - 8) < 1), 3, 2)
    to = np.where((np.abs(a - 8) < 2.2) & (np.abs(b - 8) < 2.2), 1, to)
    c = _rampa(camp)[to]
    for costat in part:
        d = {"xm": X, "xp": 32 - X, "ym": Y, "yp": 32 - Y}[costat]
        c = np.where(((d < 1.5) | ((d >= 4) & (d < 5.2)))[..., None], _rampa(vora)[1], c)
        c = np.where(((d >= 1.5) & (d < 4))[..., None], _rampa(camp)[4], c)
    return c


PARTS = {"c": (), "xm": ("xm",), "xp": ("xp",), "ym": ("ym",), "yp": ("yp",), "xm_ym": ("xm", "ym"),
         "xm_yp": ("xm", "yp"), "xp_ym": ("xp", "ym"), "xp_yp": ("xp", "yp")}

SOLS = {
    "int_taulons": lambda X, Y: _taulons(X, Y, "fusta"),
    "int_taulons_foscos": lambda X, Y: _taulons(X, Y, "noguera", llavor=3),
    "int_espiga": lambda X, Y: _espiga(X, Y, "noguera"),
    "int_espiga_clara": lambda X, Y: _espiga(X, Y, "fusta", llavor=4),
    "int_lloses": lambda X, Y: _lloses(X, Y, "pedra_castell"),
    "int_lloses_calides": lambda X, Y: _lloses(X, Y, "pedra", llavor=5),
    "int_lloses_fosques": lambda X, Y: _lloses(X, Y, "pedra_fosca", llavor=6),
    "int_marbre": lambda X, Y: _escacs(X, Y, "pedra_clara", "estuc_rosa"),
    "int_marbre_negre": lambda X, Y: _escacs(X, Y, "pedra_clara", "pedra_fosca"),
    "int_rajola": _rajola,
    "int_batuda": _batuda,
    "int_sorra": lambda X, Y: _batuda(X, Y, "sorra", llavor=9),
    "int_tatami": _tatami,
}
for nom_catifa, camp, vora in (("vermella", "vellut", "or"), ("blava", "paper_blau", "or"), ("verda", "paper_verd", "or")):
    for part, costats in PARTS.items():
        SOLS[f"int_catifa_{nom_catifa}_{part}"] = (lambda X, Y, c=costats, cm=camp, v=vora: _catifa(X, Y, c, cm, v))


def rombe(funcio, motor=True):
    """El rombe de 64x32 de la rajola. motor=True: girat de dalt a baix, com
    el dibuixarà el motor (TextureManager gira els PNG en carregar-los i la
    rajola no es torna a girar: la imatge del terra es veu invertida), perquè
    al joc es vegi com està dissenyat (les vores de les catifes al seu costat)."""
    py, px = np.mgrid[0:32, 0:64]
    X = (px + 0.5 - 32 + 2 * (py + 0.5)) / 2
    Y = (2 * (py + 0.5) - (px + 0.5 - 32)) / 2
    dins = (X >= 0) & (X <= 32) & (Y >= 0) & (Y <= 32)
    rgb = funcio(np.clip(X, 0, 31.999), np.clip(Y, 0, 31.999))
    img = np.zeros((32, 64, 4), np.uint8)
    img[..., :3] = rgb
    img[..., 3] = np.where(dins, 255, 0)
    im = Image.fromarray(img, "RGBA")
    return im.transpose(Image.FLIP_TOP_BOTTOM) if motor else im


def main():
    DESTI.mkdir(parents=True, exist_ok=True)
    for nom, f in SOLS.items():
        rombe(f).save(DESTI / f"{nom}.png")
    index = ROOT / "assets/textures/pixellab/tiles.json"
    dades = json.loads(index.read_text())
    nuevas = dades.setdefault("nuevas", [])
    per_nom = {n["nombre"]: n for n in nuevas}
    altes = 0
    for nom in SOLS:
        nou = {"nombre": nom, "imagen": f"../iso/sols_int/{nom}.png", "y0": 0, "colisiona": False}
        if nom in per_nom:
            per_nom[nom].update(nou)
        else:
            nuevas.append(nou)
            altes += 1
    index.write_text(json.dumps(dades, ensure_ascii=False, indent=1) + "\n")
    print(f"{len(SOLS)} terres d'interior a {DESTI.relative_to(ROOT)}; {altes} baldoses noves a tiles.json "
          "-> python3 tools/pixellab/importar_tiles.py assets/textures/pixellab/tiles.json")


if __name__ == "__main__":
    main()
