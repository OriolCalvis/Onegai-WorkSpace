#!/usr/bin/env python3
"""ESTIL — La paleta mestra de Boundington (biblia d'estil): les rampes de
color que comparteixen TOTS els sprites del joc (edificis, arbres, props,
PNJ, personatge, baldoses). Nomes canvia l'arquitectura entre cultures; els
colors i la manera de dibuixar, no.

    python3 tools/pixellab/paleta_mestra.py            # escriu la paleta i la mostra
    python3 tools/pixellab/paleta_mestra.py --mostra   # nomes la mostra PNG

Escriu assets/textures/pixellab/estil/paleta_mestra.json ({"rampes": {nom:
[hex...]}} de fosc a clar) i paleta_mestra.png (la mostra). Cada rampa va de
l'ombra a la llum amb la deriva classica: les ombres tiren una mica cap al
blau, les llums cap al calid; mai negre pur ni blanc pur (#1a1c26 i #f4efe2
son els extrems de tot el joc). Per canviar la paleta, canvia RAMPES i torna
a importar el que calgui amb aplicar_paleta.py.
"""
import colorsys
import json
import os
import sys

from PIL import Image, ImageDraw

RAIZ = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CARPETA = os.path.join(RAIZ, "assets", "textures", "pixellab", "estil")

# nom: (to en graus, saturacio (fosc, clar), valor (fosc, clar), passos)
RAMPES = {
    "pedra": (38, (0.32, 0.22), (0.22, 0.92), 7),          # calcaria calida d'Aegroum
    "socol": (215, (0.22, 0.14), (0.10, 0.58), 6),         # socols, pissarra fosca, ferro blavos
    "noguera": (25, (0.55, 0.42), (0.14, 0.66), 6),        # fusta treballada, portes, bigues
    "fusta_crua": (36, (0.50, 0.38), (0.28, 0.82), 6),     # taulons, molls, barraques
    "terracota": (14, (0.62, 0.52), (0.28, 0.86), 6),      # teules, coure nou, maons
    "verdet": (158, (0.48, 0.34), (0.18, 0.76), 6),        # coure amb patina, cupules
    "llauto": (44, (0.58, 0.48), (0.38, 0.96), 6),         # or, llauto, ferratges, penells
    "fulla": (98, (0.58, 0.48), (0.16, 0.82), 7),          # vegetacio de fulla
    "pi": (142, (0.48, 0.36), (0.10, 0.62), 5),            # pins, fulla fosca, setos
    "aigua": (194, (0.55, 0.40), (0.18, 0.78), 6),         # riu, estanys
    "llum": (202, (0.30, 0.06), (0.62, 1.00), 5),          # fanals dels Guardians, vidre, cel
    "pell": (24, (0.40, 0.30), (0.42, 0.96), 6),           # pells (les fosques, els 3 primers)
    "roba_vermella": (354, (0.70, 0.62), (0.24, 0.84), 6),
    "roba_blava": (226, (0.58, 0.48), (0.22, 0.80), 6),
    "roba_porpra": (286, (0.46, 0.36), (0.22, 0.72), 5),
    "gris": (40, (0.06, 0.03), (0.10, 0.96), 8),           # ferro, pedra freda, blancs
    # Accents vius (cabells, estendards, or): els personatges els fan servir.
    "accent_vermell": (358, (0.86, 0.78), (0.40, 0.92), 5),
    "accent_or": (46, (0.80, 0.66), (0.55, 1.00), 5),
    "accent_blau": (212, (0.72, 0.55), (0.35, 0.92), 5),
    "accent_verd": (108, (0.70, 0.55), (0.30, 0.86), 5),
}
OMBRA, LLUM = "#1a1c26", "#f4efe2"   # extrems de tot el joc: cap sprite baixa o puja d'aqui


def _deriva(h, t):
    """Deriva de to: cap al blau (230) a les ombres, cap al calid (50) a les llums."""
    def cap_a(h, objectiu, pes):
        d = ((objectiu - h + 180) % 360) - 180
        return (h + d * pes) % 360
    if t < 0.5:
        return cap_a(h, 230, 0.12 * (0.5 - t) * 2)
    return cap_a(h, 50, 0.10 * (t - 0.5) * 2)


def rampa(h, sat, val, passos):
    colors = []
    for i in range(passos):
        t = i / (passos - 1)
        s = sat[0] + (sat[1] - sat[0]) * t
        v = val[0] + (val[1] - val[0]) * t
        r, g, b = colorsys.hsv_to_rgb(_deriva(h, t) / 360, s, v)
        colors.append("#%02x%02x%02x" % (round(r * 255), round(g * 255), round(b * 255)))
    return colors


def paleta():
    return {nom: rampa(*spec) for nom, spec in RAMPES.items()}


def mostra(p, ruta):
    files = list(p.items())
    img = Image.new("RGB", (8 * 56 + 150, len(files) * 40 + 12), (30, 30, 36))
    d = ImageDraw.Draw(img)
    for r, (nom, colors) in enumerate(files):
        d.text((6, r * 40 + 14), nom, fill=(230, 230, 230))
        for i, c in enumerate(colors):
            d.rectangle((150 + i * 56, r * 40 + 6, 150 + i * 56 + 52, r * 40 + 36), fill=c)
    img.save(ruta)


if __name__ == "__main__":
    os.makedirs(CARPETA, exist_ok=True)
    p = paleta()
    if "--mostra" not in sys.argv:
        json.dump({"_nota": "Paleta mestra de Boundington (tools/pixellab/paleta_mestra.py): rampes de fosc a clar. "
                            "Tots els sprites s'hi mapegen amb aplicar_paleta.py. Ombra i llum extremes: " + OMBRA + " / " + LLUM,
                   "ombra": OMBRA, "llum": LLUM, "rampes": p},
                  open(os.path.join(CARPETA, "paleta_mestra.json"), "w"), ensure_ascii=False, indent=1)
    mostra(p, os.path.join(CARPETA, "paleta_mestra.png"))
    total = sum(len(v) for v in p.values())
    print(f"paleta mestra: {len(p)} rampes, {total} colors -> {CARPETA}")
