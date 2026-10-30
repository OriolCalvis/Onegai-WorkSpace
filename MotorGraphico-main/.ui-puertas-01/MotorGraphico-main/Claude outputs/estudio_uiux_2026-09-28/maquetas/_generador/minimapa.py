"""Minimapa real a partir de la rejilla del nivel (colisión TMX + objetos)."""
import os
import sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from PIL import Image, ImageDraw
from nav import Level, catalog
from lib import FUENTE

C_BLOQ = (18, 20, 27); C_SUELO = (58, 65, 88); C_SALIDA = (156, 195, 255)
C_PNJ = (193, 201, 214); C_OBJ = (235, 199, 125); C_JUG = (243, 235, 221); C_NEGRO = (11, 13, 18)


def generar(nivel, jugador, objetivos=(), ancho=56, alto=34, px=4, salida=os.path.join(FUENTE, "img", "minimapa.png"), pnj_extra=()):
    L = Level(nivel); cat = catalog()
    x0 = max(0, min(L.w - ancho, jugador[0] - ancho // 2))
    y0 = max(0, min(L.h - alto, jugador[1] - alto // 2))
    im = Image.new("RGB", (ancho * px, alto * px), C_BLOQ)
    d = ImageDraw.Draw(im)
    for y in range(alto):
        for x in range(ancho):
            c = (x0 + x, y0 + y)
            if c not in L.blocked:
                d.rectangle([x * px, y * px, x * px + px - 1, y * px + px - 1], fill=C_SUELO)
    puertas = {(o["position"]["x"], o["position"]["y"]) for o in L.data["objects"] if "targetLevel" in o}
    def dentro(c):
        return x0 <= c[0] < x0 + ancho and y0 <= c[1] < y0 + alto
    def centro(c):
        return ((c[0] - x0) * px + px // 2, (c[1] - y0) * px + px // 2)
    for c in puertas:
        if dentro(c) and c not in objetivos:
            cx, cy = centro(c)
            d.rectangle([cx - 3, cy - 3, cx + 3, cy + 3], fill=C_NEGRO)
            d.rectangle([cx - 2, cy - 2, cx + 2, cy + 2], fill=C_SALIDA)
    pnjs = [c for c, i in L.objects.items() if cat.get(i, {}).get("category") == "npc"] + list(pnj_extra)
    for c in pnjs:
        if dentro(c):
            cx, cy = centro(c)
            d.ellipse([cx - 3, cy - 3, cx + 3, cy + 3], fill=C_NEGRO)
            d.ellipse([cx - 2, cy - 2, cx + 2, cy + 2], fill=C_PNJ)
    for c in objetivos:
        if dentro(c):
            cx, cy = centro(c)
            d.polygon([(cx, cy - 5), (cx + 5, cy), (cx, cy + 5), (cx - 5, cy)], fill=C_NEGRO)
            d.polygon([(cx, cy - 3), (cx + 3, cy), (cx, cy + 3), (cx - 3, cy)], fill=C_OBJ)
    cx, cy = centro(jugador)
    d.polygon([(cx, cy - 6), (cx + 6, cy + 5), (cx - 6, cy + 5)], fill=C_NEGRO)
    d.polygon([(cx, cy - 4), (cx + 4, cy + 3), (cx - 4, cy + 3)], fill=C_JUG)
    im.save(salida)
    return im.size


if __name__ == "__main__":
    # Jugador junto a la anciana (26,34); objetivo seguido: las tres tiendas del tutorial
    print(generar("assets/levels/ciudad_centro.json", (26, 34), objetivos=[(54, 27), (8, 27), (42, 27)],
                  salida=os.path.join(FUENTE, "img", "minimapa_ciudad.png")))
