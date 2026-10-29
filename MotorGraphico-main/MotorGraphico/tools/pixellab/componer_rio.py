#!/usr/bin/env python3
"""BLOQUES-04 — Las baldosas del rio con muro de muelle, y el tablero del
puente con pretil, compuestas a partir de dos baldosas de PixelLab.

Las baldosas del motor son planas (solo la cara superior, un rombo 2:1 de
64x32). Un muelle de piedra que cae al agua se ve desde la camara por su
cara +x (borde superior izquierdo del rombo del agua) o +y (borde superior
derecho): el muro se pinta DENTRO del rombo de la baldosa de agua que esta
delante del muelle, como una banda que baja desde ese borde. Lo mismo vale
para el canto del puente (el agua pegada al tablero lleva muro) y para el
pretil del lado lejano del tablero (banda desde el borde del tablero).

    python3 tools/pixellab/componer_rio.py assets/textures/pixellab/lote11

Lee en esa carpeta rio.png (agua, thin tile de 64x64 con la cara en y0=23),
muelle_piedra.png y puente_piedra.png; escribe rio_muro_izq.png,
rio_muro_der.png, rio_muro_ambos.png, puente_pretil_izq.png y
puente_pretil_der.png con el mismo formato (los lee importar_tiles.py).
"""
import sys
from pathlib import Path

from PIL import Image

Y0, ANCHO, ALTO = 23, 64, 32          # cara superior del thin tile de PixelLab
MURO, PRETIL, TAPA = 12, 7, 2        # alturas en px: muro de muelle, pretil, tapa clara


def dentro(x, y):
    """La mascara exacta del rombo que recorta importar_tiles.cara_superior."""
    medio = y + 1 if y < ALTO // 2 else ALTO - y
    lleno = min(ANCHO, 4 * medio)
    x0 = (ANCHO - lleno) // 2
    return x0 <= x < x0 + lleno


def borde_superior(x, lado):
    """Fila del borde superior en la columna x, PROLONGADO a toda la anchura:
    el izquierdo es la recta de (0,15) a (31,0) (sigue en negativo a la
    derecha), el derecho la de (32,0) a (63,15). El muro es una banda bajo
    esa recta en todo el ancho de la baldosa: la parte que cae en la mitad
    "equivocada" del rombo es la continuacion del muro de la baldosa vecina
    (la orilla es recta a lo largo de la fila), y sin ella el muro salia a
    dientes de sierra, cortado en cada esquina."""
    return (31 - x) // 2 if lado == "izq" else (x - 32) // 2


def tono(piedra, x, y, k):
    """Color de la piedra del muelle muestreado por hiladas: cada 4 px una
    junta oscura, y un pequeno desfase de sillares por hilada."""
    px = piedra.load()
    sx = (x * 3 + (k // 4) * 7) % ANCHO
    sy = Y0 + 4 + ((k * 5 + x // 6) % (ALTO - 8))
    r, g, b, a = px[sx, sy]
    if a == 0:
        r, g, b = 110, 104, 100
    if k % 4 == 3:                    # junta
        return (max(0, r - 45), max(0, g - 45), max(0, b - 42), 255)
    if k == 0:                        # arista clara de la tapa
        return (min(255, r + 35), min(255, g + 35), min(255, b + 30), 255)
    oscuro = 18 + k                   # el muro, en sombra respecto a la cara
    return (max(0, r - oscuro), max(0, g - oscuro), max(0, b - oscuro + 4), 255)


def banda(base, piedra, lado, alto, tapa=TAPA, agua=True):
    """La baldosa base con una banda de piedra de `alto` px que baja desde el
    borde superior `lado` del rombo (solo dentro del rombo)."""
    img = base.copy()
    px = img.load()
    for x in range(ANCHO):
        ye = borde_superior(x, lado)
        for k in range(alto):
            y = ye + k
            if not (0 <= y < ALTO) or not dentro(x, y):
                continue
            c = tono(piedra, x, y, k if k >= tapa else 0)
            if agua and k == alto - 1:  # linea de agua: reflejo oscuro al pie
                c = (max(0, c[0] - 30), max(0, c[1] - 25), min(255, c[2] + 10), 255)
            px[x, Y0 + y] = c
    return img


def componer(carpeta):
    carpeta = Path(carpeta)
    rio = Image.open(carpeta / "rio.png").convert("RGBA")
    muelle = Image.open(carpeta / "muelle_piedra.png").convert("RGBA")
    puente = Image.open(carpeta / "puente_piedra.png").convert("RGBA")
    salidas = {
        "rio_muro_izq": banda(rio, muelle, "izq", MURO),
        "rio_muro_der": banda(rio, muelle, "der", MURO),
        "rio_muro_ambos": banda(banda(rio, muelle, "izq", MURO), muelle, "der", MURO),
        "puente_pretil_izq": banda(puente, muelle, "izq", PRETIL, agua=False),
        "puente_pretil_der": banda(puente, muelle, "der", PRETIL, agua=False),
    }
    for nombre, img in salidas.items():
        img.save(carpeta / f"{nombre}.png")
    print(f"{carpeta}: {', '.join(salidas)} compuestas")


if __name__ == "__main__":
    componer(sys.argv[1] if len(sys.argv) > 1 else Path(__file__).resolve().parents[2] / "assets/textures/pixellab/lote11")
