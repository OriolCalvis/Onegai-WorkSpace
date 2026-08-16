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
    # Paleta tomada de las bibliotecas editoriales de civilizacion: piedra
    # calida, madera oscura y acentos azul/oro. Se conservan los GIDs para
    # que los 81 mapas urbanos existentes reciban el cambio sin migracion.
    (1,  "adoquin",     (157, 151, 130), False),
    (2,  "muralla",     (73,  81,  87),  True),
    (3,  "cesped",      (92,  132, 72),  False),
    (4,  "agua",        (52,  112, 144), True),
    (5,  "marmol",      (213, 218, 207), False),
    (6,  "tierra",      (151, 112, 70),  False),
    (7,  "castillo",    (92,  101, 112), True),
    (8,  "iglesia",     (210, 207, 186), True),
    (9,  "universidad", (151, 104, 61),  True),
    (10, "biblioteca",  (110, 73,  42),  True),
    (11, "opera",       (173, 77,  55),  True),
    (12, "coliseo",     (198, 174, 122), True),
    (13, "militar",     (79,  88,  91),  True),
    (14, "tienda",      (181, 116, 61),  True),
    (15, "ropa",        (123, 79,  119), True),
    (16, "joyeria",     (193, 157, 55),  True),
    (17, "banco",       (116, 123, 108), True),
    (18, "posada",      (130, 82,  43),  True),
    (19, "banos",       (109, 166, 174), True),
    (20, "seto",        (47,  93,  55),  True),
    (21, "arena",       (216, 177, 103), False),
    (22, "escenario",   (116, 63,  64),  False),
    (23, "umbral",      (224, 194, 117), False),  # puerta: se pisa
    (24, "grava",       (132, 128, 117), False),
    # --- Boundington (GIDs 25-36) ---
    # Casa popular: piedra fuera, madera dentro (canon). Dos tonos para que
    # una manzana de casas pequenas no lea como un bloque macizo.
    (25, "casa_piedra",  (119, 118, 109), True),
    (26, "casa_madera",  (128, 83,  49),  True),
    # Barrios Altos: mismo material mejor cuidado, y calle mas limpia.
    (27, "casa_noble",   (197, 191, 169), True),
    (28, "adoquin_fino", (181, 175, 151), False),
    # Casco Antiguo: piedra vieja y ruina. La ruina NO colisiona: se puede
    # entrar en un edificio caido, y ahi es donde estan los secretos.
    (29, "piedra_vieja",  (90,  92,  88),  True),
    (30, "ruina",         (124, 113, 96),  False),
    # Calzada con carriles: el centro es para carruajes, los lados para la
    # gente. Un tile distinto por carril, que es lo que hace que la calle
    # se lea como calle y no como pasillo.
    (31, "carril",        (142, 137, 119), False),
    # Pico Dragon y la Barriada: barro y tablones, sin orden.
    (32, "barro",         (103, 76,  55),  False),
    (33, "chabola",       (112, 74,  43),  True),
    # Cuerdas de ropa tendida entre casa y casa. No colisiona: pasas por
    # debajo. Es decoracion, pero es LA imagen del barrio popular.
    (34, "tendedero",     (186, 156, 126), False),
    # El Puente Principal y el rio que separa Boundington de Surysal.
    (35, "puente",        (137, 93,  51),  False),
    (36, "rio",           (43,  102, 145), True),
]

img = Image.new("RGBA", (COLS*CELL, ROWS*CELL), (0, 0, 0, 0))
d = ImageDraw.Draw(img)
for gid, name, col, _ in TILES:
    i = gid - 1
    x, y = (i % COLS) * CELL, (i // COLS) * CELL
    d.rectangle([x, y, x+CELL-1, y+CELL-1], fill=col + (255,))
    borde = tuple(max(0, c - 34) for c in col)
    d.rectangle([x, y, x+CELL-1, y+CELL-1], outline=borde + (255,))
    # Texturas de baja frecuencia: a 16px una textura debe ser una pista
    # clara, no ruido. Cada familia conserva una silueta legible al verse
    # comprimida sobre el rombo isometrico de 64x32.
    claro = tuple(min(255, c + 22) for c in col) + (255,)
    oscuro = borde + (255,)
    if name in {"adoquin", "adoquin_fino", "marmol", "arena", "grava", "carril"}:
        for yy in range(y + 3, y + CELL, 5):
            d.line([(x + 1, yy), (x + CELL - 2, yy)], fill=oscuro)
        for xx in range(x + 4, x + CELL, 6):
            d.line([(xx, y + 1), (xx - 2, y + CELL - 2)], fill=claro)
    elif name in {"casa_madera", "biblioteca", "posada", "chabola", "puente"}:
        for yy in range(y + 3, y + CELL, 4):
            d.line([(x + 1, yy), (x + CELL - 2, yy)], fill=oscuro)
        d.line([(x + 2, y + 2), (x + CELL - 3, y + CELL - 3)], fill=claro)
    elif name in {"agua", "rio", "banos"}:
        for yy in range(y + 3, y + CELL, 5):
            d.line([(x + 2, yy), (x + 6, yy)], fill=claro)
            d.line([(x + 10, yy + 1), (x + CELL - 3, yy + 1)], fill=oscuro)
    elif name in {"cesped", "seto"}:
        for xx, yy in ((3, 4), (9, 3), (6, 10), (13, 12)):
            d.point((x + xx, y + yy), fill=claro)
            d.point((x + xx + 1, y + yy), fill=oscuro)
    elif name in {"barro", "tierra", "ruina", "piedra_vieja"}:
        for xx, yy in ((3, 3), (10, 5), (6, 11), (13, 13)):
            d.rectangle([x + xx, y + yy, x + xx + 1, y + yy + 1], fill=oscuro)
    elif any(t[0] == gid and t[3] for t in TILES):
        # Muros y edificios: junta de bloques diagonal, más clara en la
        # parte superior, para diferenciar estructura de suelo.
        for k in range(0, CELL, 5):
            d.line([(x, y + k), (x + k, y)], fill=oscuro)
        d.line([(x + 2, y + 2), (x + CELL - 3, y + 2)], fill=claro)

# Raiz del repo, deducida de la ubicacion de este script. Antes era una ruta
# absoluta a un sandbox que ya no existe, asi que ningun script corria fuera
# de la maquina donde se escribio.
BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
img.save(BASE + "assets/textures/ciudad_tileset.png")
print(f"tileset {img.size[0]}x{img.size[1]} px, {len(TILES)} tiles")
print("colisionan:", [t[1] for t in TILES if t[3]])
