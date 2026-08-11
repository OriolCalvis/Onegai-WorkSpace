"""Tileset de ciudad: colores planos por material, con un borde mas oscuro
para que en la vista isometrica se distinga cada celda. 6 columnas x 4
filas de 16x16 px -> GIDs 1..24 en orden de lectura (convencion Tiled)."""
import os
from PIL import Image, ImageDraw

CELL, COLS, ROWS = 16, 6, 4
# (gid, nombre, color, colisiona)
TILES = [
    (1,  "adoquin",     (108, 104, 100), False),
    (2,  "muralla",     (74,  70,  66),  True),
    (3,  "cesped",      (86,  132, 74),  False),
    (4,  "agua",        (58,  102, 160), True),
    (5,  "marmol",      (198, 194, 186), False),
    (6,  "tierra",      (134, 110, 82),  False),
    (7,  "castillo",    (122, 122, 138), True),
    (8,  "iglesia",     (176, 168, 200), True),
    (9,  "universidad", (150, 122, 90),  True),
    (10, "biblioteca",  (128, 96,  72),  True),
    (11, "opera",       (196, 150, 96),  True),
    (12, "coliseo",     (190, 176, 148), True),
    (13, "militar",     (96,  106, 82),  True),
    (14, "tienda",      (170, 96,  76),  True),
    (15, "ropa",        (168, 92,  140), True),
    (16, "joyeria",     (206, 176, 62),  True),
    (17, "banco",       (140, 140, 96),  True),
    (18, "posada",      (150, 106, 62),  True),
    (19, "banos",       (110, 160, 172), True),
    (20, "seto",        (60,  104, 58),  True),
    (21, "arena",       (206, 188, 148), False),
    (22, "escenario",   (128, 74,  74),  False),
    (23, "umbral",      (222, 200, 130), False),  # puerta: se pisa
    (24, "grava",       (150, 146, 138), False),
]

img = Image.new("RGBA", (COLS*CELL, ROWS*CELL), (0, 0, 0, 0))
d = ImageDraw.Draw(img)
for gid, name, col, _ in TILES:
    i = gid - 1
    x, y = (i % COLS) * CELL, (i // COLS) * CELL
    d.rectangle([x, y, x+CELL-1, y+CELL-1], fill=col + (255,))
    borde = tuple(max(0, c - 34) for c in col)
    d.rectangle([x, y, x+CELL-1, y+CELL-1], outline=borde + (255,))
    # Trama diagonal en los muros: los distingue del suelo aunque el
    # color se parezca, y ayuda a leer el mapa en blanco y negro.
    if any(t[0] == gid and t[3] for t in TILES):
        for k in range(0, CELL, 5):
            d.line([(x, y+k), (x+k, y)], fill=borde + (255,))

# Raiz del repo, deducida de la ubicacion de este script. Antes era una ruta
# absoluta a un sandbox que ya no existe, asi que ningun script corria fuera
# de la maquina donde se escribio.
BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
img.save(BASE + "assets/textures/ciudad_tileset.png")
print(f"tileset {img.size[0]}x{img.size[1]} px, {len(TILES)} tiles")
print("colisionan:", [t[1] for t in TILES if t[3]])
