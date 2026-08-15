"""Tileset de ciudad: colores planos por material, con un borde mas oscuro
para que en la vista isometrica se distinga cada celda. 6 columnas x 6
filas de 16x16 px -> GIDs 1..36 en orden de lectura (convencion Tiled).

Los GIDs 25-36 son de Boundington. El canon describe la ciudad con bastante
detalle y el tileset de 24 no daba para contarlo: pedia casas de piedra por
fuera y madera por dentro, calles con carril de peatones a los lados y sitio
para carruajes en el centro, cuerdas de ropa entre las casas, un Casco Antiguo
"deteriorado", chabolas "sin orden alguno" en Pico Dragon y un puente a
Surysal. Con un solo tile de "tienda" para todo eso, los tres mapas salian
identicos."""
import os
from PIL import Image, ImageDraw

CELL, COLS, ROWS = 16, 6, 6
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
    # --- Boundington (GIDs 25-36) ---
    # Casa popular: piedra fuera, madera dentro (canon). Dos tonos para que
    # una manzana de casas pequenas no lea como un bloque macizo.
    (25, "casa_piedra",  (128, 120, 110), True),
    (26, "casa_madera",  (146, 112, 74),  True),
    # Barrios Altos: mismo material mejor cuidado, y calle mas limpia.
    (27, "casa_noble",   (170, 160, 148), True),
    (28, "adoquin_fino", (150, 146, 140), False),
    # Casco Antiguo: piedra vieja y ruina. La ruina NO colisiona: se puede
    # entrar en un edificio caido, y ahi es donde estan los secretos.
    (29, "piedra_vieja",  (96,  92,  88),  True),
    (30, "ruina",         (118, 112, 104), False),
    # Calzada con carriles: el centro es para carruajes, los lados para la
    # gente. Un tile distinto por carril, que es lo que hace que la calle
    # se lea como calle y no como pasillo.
    (31, "carril",        (122, 118, 112), False),
    # Pico Dragon y la Barriada: barro y tablones, sin orden.
    (32, "barro",         (110, 92,  70),  False),
    (33, "chabola",       (124, 96,  62),  True),
    # Cuerdas de ropa tendida entre casa y casa. No colisiona: pasas por
    # debajo. Es decoracion, pero es LA imagen del barrio popular.
    (34, "tendedero",     (188, 172, 150), False),
    # El Puente Principal y el rio que separa Boundington de Surysal.
    (35, "puente",        (142, 118, 88),  False),
    (36, "rio",           (72,  116, 172), True),
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
