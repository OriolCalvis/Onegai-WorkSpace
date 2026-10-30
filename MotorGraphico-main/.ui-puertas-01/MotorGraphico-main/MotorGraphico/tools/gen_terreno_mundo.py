#!/usr/bin/env python3
"""El atlas de runtime definitivo: suelos nuevos + los tiles viejos.

    python3 tools/gen_tileset_iso.py     # primero: los suelos nuevos
    python3 tools/gen_terreno_mundo.py   # despues: el combinado

POR QUE UN COMBINADO Y NO SOLO LOS SUELOS NUEVOS

TileMap solo admite UN <tileset> por mapa. Y los mapas no son solo suelo:
22 de los 36 tiles viejos BLOQUEAN el paso y son los edificios de
Boundington (20.578 celdas, el 25%). Si el atlas solo trae suelos, o se
pierden los edificios o no se puede remapear nada.

    GID  1..40   suelos nuevos, de terreno_iso.png
    GID 41..76   los 36 tiles de ciudad_tileset.png, escalados a la celda
                 nueva y conservados tal cual

Los viejos son cuadrados planos de color de 16x16 -- eran marcadores, no
arte -- asi que se escalan con NEAREST: ampliar un color plano con
interpolacion solo le anade bordes borrosos. Se dibujan como rombo para
que encajen con los suelos nuevos en vez de aparecer como cuadrados
girados 45 grados respecto a todo lo demas.
"""
import json
import os
import sys

try:
    from PIL import Image, ImageDraw
except ImportError:
    sys.exit("hace falta Pillow:  pip install Pillow --break-system-packages")

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
TEX = os.path.join(RAIZ, "assets", "textures")

COLUMNAS = 8


def rombo(color, w, h):
    """Un rombo 2:1 del color dado, para sustituir un cuadrado plano."""
    im = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.polygon([(w // 2, 0), (w - 1, h // 2), (w // 2, h - 1), (0, h // 2)], fill=color)
    return im


def main():
    idx_path = os.path.join(TEX, "terreno_iso.json")
    if not os.path.exists(idx_path):
        sys.exit("falta terreno_iso.json: ejecuta antes tools/gen_tileset_iso.py")
    with open(idx_path, encoding="utf-8") as f:
        base_idx = json.load(f)

    cw = base_idx["celda"]["ancho"]
    ch = base_idx["celda"]["alto"]
    nuevos = Image.open(os.path.join(TEX, "terreno_iso.png")).convert("RGBA")
    n_nuevos = base_idx["tileCount"]

    # Los 36 viejos, de su atlas de 16x16.
    viejo = Image.open(os.path.join(TEX, "ciudad_tileset.png")).convert("RGBA")
    vcols = viejo.size[0] // 16
    n_viejos = (viejo.size[0] // 16) * (viejo.size[1] // 16)

    # Sus nombres y colores, de tools/gen_tileset.py: es la fuente de
    # verdad de que era cada GID.
    import re
    nombres, colores = {}, {}
    for linea in open(os.path.join(AQUI, "gen_tileset.py"), encoding="utf-8"):
        m = re.match(r'\s*\((\d+),\s*"([a-z_]+)",\s*\((\d+),\s*(\d+),\s*(\d+)\),\s*(True|False)\)', linea)
        if m:
            g = int(m.group(1))
            nombres[g] = m.group(2)
            colores[g] = (int(m.group(3)), int(m.group(4)), int(m.group(5)), 255)

    total = n_nuevos + n_viejos
    filas = (total + COLUMNAS - 1) // COLUMNAS
    atlas = Image.new("RGBA", (COLUMNAS * cw, filas * ch), (0, 0, 0, 0))

    # Zona 1: los suelos nuevos, tal cual.
    for i in range(n_nuevos):
        src = nuevos.crop(((i % base_idx["columnas"]) * cw, (i // base_idx["columnas"]) * ch,
                           (i % base_idx["columnas"]) * cw + cw, (i // base_idx["columnas"]) * ch + ch))
        atlas.paste(src, ((i % COLUMNAS) * cw, (i // COLUMNAS) * ch))

    tiles = list(base_idx["tiles"])
    colision = list(base_idx["colisionGids"])

    # Zona 2: los viejos, como rombo de su color.
    for v in range(1, n_viejos + 1):
        destino = n_nuevos + v
        i = destino - 1
        col = colores.get(v)
        if col is None:
            # Sin ficha en gen_tileset.py: se escala el cuadrado original,
            # que es mejor que dejar el hueco vacio y no enterarse.
            c0 = ((v - 1) % vcols) * 16
            r0 = ((v - 1) // vcols) * 16
            img = viejo.crop((c0, r0, c0 + 16, r0 + 16)).resize((cw, ch), Image.NEAREST)
        else:
            img = rombo(col, cw, ch)
        atlas.paste(img, ((i % COLUMNAS) * cw, (i // COLUMNAS) * ch))
        tiles.append({"gid": destino, "nombre": "viejo_" + nombres.get(v, str(v)),
                      "colisiona": False, "conservado": True})

    salida = os.path.join(TEX, "terreno_mundo.png")
    atlas.save(salida)

    base_idx.update({
        "_nota": "Generado por tools/gen_terreno_mundo.py. Suelos nuevos + tiles viejos conservados.",
        "imagen": "../textures/terreno_mundo.png",
        "nombre": "terreno_mundo",
        "baseViejos": n_nuevos,
        "tileCountTotal": total,
        "anchoTotal": atlas.size[0],
        "altoTotal": atlas.size[1],
        "tiles": tiles,
        "colisionGids": colision,
    })
    with open(idx_path, "w", encoding="utf-8") as f:
        json.dump(base_idx, f, ensure_ascii=False, indent=1)

    print(f"  terreno_mundo.png: {atlas.size[0]}x{atlas.size[1]}, {total} tiles de {cw}x{ch}")
    print(f"    GID 1..{n_nuevos}  suelos nuevos")
    print(f"    GID {n_nuevos+1}..{total}  los {n_viejos} viejos, conservados")
    return 0


if __name__ == "__main__":
    sys.exit(main())
