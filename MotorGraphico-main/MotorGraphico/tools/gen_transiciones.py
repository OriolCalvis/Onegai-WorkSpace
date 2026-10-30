#!/usr/bin/env python3
"""AV16-10 — Transiciones suaves entre suelos, con el sistema de dos cuadriculas (dual-grid).

    python3 tools/gen_transiciones.py

Que hace el sistema (el del video que mando Oriol el 14/10): ademas de la cuadricula de
los suelos, hay una SEGUNDA cuadricula desplazada media casilla, con una celda en cada
esquina donde se juntan cuatro suelos. Esa celda mira cuales de sus cuatro esquinas son de
un material y dibuja uno de 15 recortes (los 16 menos «ninguna»), con el borde difuminado.
Con 15 recortes por material salen todas las uniones, sin dibujar cada pareja a mano.

Aqui los recortes SALEN del propio suelo: para cada material, su rombo de terreno_mundo.png
enmascarado con la interpolacion bilineal de las cuatro esquinas (la misma que usan los
«marching squares»), con un borde blando y un poco de ruido para que no sea una recta.

Salida:
  assets/textures/transiciones.png   una fila por material, 15 columnas (mascara 1..15)
  assets/textures/transiciones.json  gid -> fila y rango de prioridad (quien se dibuja encima)

Bits de la mascara: 1 = esquina de arriba (x,y), 2 = derecha (x+1,y), 4 = izquierda (x,y+1),
8 = abajo (x+1,y+1).
"""
import json
import math
import os
import sys

from PIL import Image

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
TEX = os.path.join(RAIZ, "assets", "textures")
W, H = 128, 64
COLS = 8

# De abajo (se dibuja primero, lo pisan) a arriba (se dibuja encima). Los suelos de
# interior (marmol, tarima, mosaico...) no entran: sus bordes se quedan duros, como una
# habitacion. Un material que no esta aqui no tiene transiciones.
PRIORIDAD = [
    "rio", "agua_somera", "estanque",
    "arena", "arena_playa", "arenisca_desertica",
    "barro", "turba_pantano", "callejon_embarrado", "tierra_canon", "tierra_piedras",
    "sendero_tierra", "tierra_musgosa", "hojarasca", "ceniza_volcanica", "nieve",
    "hierba_seca", "pradera", "hierba_flores",
    "roca", "piedra_antigua", "ruina_adoquin", "ciudad_ruinas", "piedra_enana",
    "adoquin_gastado", "adoquin_limpio", "pavimento_antiguo", "adoquin_ajedrez",
    "plaza", "patio_fortificado", "losa_clara", "losa_oscura",
]


def ruido(x, y, semilla):
    """Ruido de valor, 0..1, suave a escala ~10 px; determinista."""
    def h(i, j):
        n = (i * 374761393 + j * 668265263 + semilla * 2147483647) & 0xFFFFFFFF
        n = ((n ^ (n >> 13)) * 1274126177) & 0xFFFFFFFF
        return ((n ^ (n >> 16)) & 0xFFFF) / 65535.0
    fx, fy = x / 10.0, y / 10.0
    i, j = math.floor(fx), math.floor(fy)
    tx, ty = fx - i, fy - j
    tx, ty = tx * tx * (3 - 2 * tx), ty * ty * (3 - 2 * ty)
    a = h(i, j) * (1 - tx) + h(i + 1, j) * tx
    b = h(i, j + 1) * (1 - tx) + h(i + 1, j + 1) * tx
    return a * (1 - ty) + b * ty


def paso_suave(a, b, x):
    t = max(0.0, min(1.0, (x - a) / (b - a)))
    return t * t * (3 - 2 * t)


def alfa_de(mascara):
    """Mapa de alfa W x H de una mascara: bilineal de las esquinas + ruido, con borde blando."""
    arriba, derecha, izquierda, abajo = (1 if mascara & b else 0 for b in (1, 2, 4, 8))
    a = [[0.0] * W for _ in range(H)]
    for py in range(H):
        for px in range(W):
            u = (px + 0.5) / W * 2 - 1        # -1..1 en la pantalla
            v = (py + 0.5) / H * 2 - 1
            gx, gy = (u + v + 1) / 2, (v - u + 1) / 2
            if mascara == 15:
                a[py][px] = 1.0
                continue
            crudo = ((1 - gx) * (1 - gy) * arriba + gx * (1 - gy) * derecha +
                     (1 - gx) * gy * izquierda + gx * gy * abajo)
            crudo += (ruido(px, py, mascara) - 0.5) * 0.36
            a[py][px] = paso_suave(0.5 - 0.2, 0.5 + 0.2, crudo)
    return a


def main():
    indice = json.load(open(os.path.join(TEX, "terreno_iso.json"), encoding="utf-8"))
    gid_de = {t["nombre"]: t["gid"] for t in indice["tiles"]}
    falta = [n for n in PRIORIDAD if n not in gid_de]
    if falta:
        sys.exit("materiales que no estan en terreno_iso.json: " + ", ".join(falta))
    atlas = Image.open(os.path.join(TEX, "terreno_mundo.png")).convert("RGBA")
    alfas = {m: alfa_de(m) for m in range(1, 16)}
    salida = Image.new("RGBA", (15 * W, len(PRIORIDAD) * H), (0, 0, 0, 0))
    for fila, nombre in enumerate(PRIORIDAD):
        g = gid_de[nombre] - 1
        celda = atlas.crop(((g % COLS) * W, (g // COLS) * H, (g % COLS + 1) * W, (g // COLS + 1) * H))
        px = celda.load()
        for m in range(1, 16):
            a = alfas[m]
            trozo = Image.new("RGBA", (W, H), (0, 0, 0, 0))
            tp = trozo.load()
            for y in range(H):
                for x in range(W):
                    r, gg, b, al = px[x, y]
                    if al:
                        tp[x, y] = (r, gg, b, int(al * a[y][x]))
            salida.paste(trozo, ((m - 1) * W, fila * H))
    salida.save(os.path.join(TEX, "transiciones.png"))
    datos = {
        "_nota": "Generado por tools/gen_transiciones.py. fila = posicion en prioridad; "
                 "mayor rango se dibuja encima. Mascara 1..15 = columna + 1.",
        "imagen": "transiciones.png",
        "celda": {"ancho": W, "alto": H},
        "columnas": 15,
        "materiales": [{"gid": gid_de[n], "nombre": n, "rango": i} for i, n in enumerate(PRIORIDAD)],
    }
    with open(os.path.join(TEX, "transiciones.json"), "w", encoding="utf-8") as f:
        json.dump(datos, f, ensure_ascii=False, indent=1)
    print(f"transiciones.png: {salida.size[0]}x{salida.size[1]}, {len(PRIORIDAD)} materiales x 15 recortes")


if __name__ == "__main__":
    sys.exit(main())
