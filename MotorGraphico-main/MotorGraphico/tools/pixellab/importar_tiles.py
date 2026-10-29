#!/usr/bin/env python3
"""PIXELLAB — Mete baldosas de PixelLab en terreno_mundo.png (celdas 128x64).

PixelLab (create_isometric_tile, size=64, "thin tile") devuelve un bloque de
64x64 con la cara superior (rombo 2:1 de 64x32) a partir de la fila 23 y un
canto debajo. El motor dibuja celdas planas (el rombo de 128x64 de la celda,
pintado a 64x32), asi que esto recorta la cara superior, la enmascara con el
rombo exacto y la amplia x2 (vecino mas proximo) sobre la celda del gid.

    python3 tools/pixellab/importar_tiles.py assets/textures/pixellab/tiles.json

tiles.json: {"baldosas": [{"gid": 17, "imagen": "lote0/tile_empedrado.png", "y0": 23}, ...]}
(y0 = fila donde empieza la cara superior; 23 para size=64 thin tile).
El TMX no cambia; el generador tools/gen_terreno_mundo.py vuelve a dejar la
celda procedimental, asi que tras regenerar hay que volver a importar.
"""
import json
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
TEX = ROOT / "assets/textures"
CELDA_W, CELDA_H, COLUMNAS = 128, 64, 8


def cara_superior(img, y0, ancho=64):
    """La cara superior del bloque como rombo 2:1 de ancho x ancho/2."""
    alto = ancho // 2
    cara = img.convert("RGBA").crop((0, y0, ancho, y0 + alto))
    px = cara.load()
    for y in range(alto):
        # Rombo: en la fila y caben 2*(y+1) px en la mitad superior (y simetrico).
        medio = y + 1 if y < alto // 2 else alto - y
        lleno = min(ancho, 2 * medio * 2)  # rombo 2:1: dos px por fila y lado
        x0 = (ancho - lleno) // 2
        for x in range(ancho):
            if x < x0 or x >= x0 + lleno:
                px[x, y] = (0, 0, 0, 0)
    return cara


def dar_de_alta(nuevas, idx, hoja):
    """BLOQUES-04 — Baldosas NUEVAS ("nuevas": [{"nombre", "imagen", "colisiona"}]):
    reciben el gid siguiente del indice (o el suyo, si el nombre ya existe),
    y terreno_mundo.png crece por filas enteras cuando hace falta. El TMX
    toma tilecount y alto del indice, y el motor reparte los gids por las
    filas REALES de la textura, asi que los mapas viejos siguen valiendo."""
    por_nombre = {t["nombre"]: t for t in idx["tiles"]}
    for n in nuevas:
        t = por_nombre.get(n["nombre"])
        if t is None:
            t = {"gid": max(x["gid"] for x in idx["tiles"]) + 1, "nombre": n["nombre"],
                 "colisiona": bool(n.get("colisiona", False)), "pixellab": True}
            idx["tiles"].append(t)
            por_nombre[n["nombre"]] = t
        n["gid"] = t["gid"]
        t["colisiona"] = bool(n.get("colisiona", t.get("colisiona", False)))
        if t["colisiona"] and t["gid"] not in idx["colisionGids"]:
            idx["colisionGids"].append(t["gid"])
        elif not t["colisiona"] and t["gid"] in idx["colisionGids"]:
            idx["colisionGids"].remove(t["gid"])
    total = max(x["gid"] for x in idx["tiles"])
    filas = (total + COLUMNAS - 1) // COLUMNAS
    if hoja.height < filas * CELDA_H:
        mayor = Image.new("RGBA", (hoja.width, filas * CELDA_H), (0, 0, 0, 0))
        mayor.paste(hoja, (0, 0))
        hoja = mayor
    idx["tileCountTotal"] = total
    idx["altoTotal"] = hoja.height
    idx["colisionGids"].sort()
    return hoja


def importar(indice):
    datos = json.loads(Path(indice).read_text())
    hoja = Image.open(TEX / "terreno_mundo.png").convert("RGBA")
    idx = json.loads((TEX / "terreno_iso.json").read_text())
    nuevas = datos.get("nuevas", [])
    hoja = dar_de_alta(nuevas, idx, hoja)
    hechas = []
    for b in datos["baldosas"] + nuevas:
        gid = int(b["gid"])
        ruta = (Path(indice).parent / b["imagen"]) if not Path(b["imagen"]).is_absolute() else Path(b["imagen"])
        cara = cara_superior(Image.open(ruta), int(b.get("y0", 23)))
        celda = cara.resize((CELDA_W, CELDA_H), Image.NEAREST)
        i = gid - 1
        x, y = (i % COLUMNAS) * CELDA_W, (i // COLUMNAS) * CELDA_H
        hoja.paste(Image.new("RGBA", (CELDA_W, CELDA_H), (0, 0, 0, 0)), (x, y))
        hoja.paste(celda, (x, y), celda)
        hechas.append(gid)
    hoja.save(TEX / "terreno_mundo.png")
    if nuevas:
        (TEX / "terreno_iso.json").write_text(json.dumps(idx, ensure_ascii=False, indent=1) + "\n")
        print(f"terreno_iso.json: {len(nuevas)} baldosas nuevas, {idx['tileCountTotal']} en total, alto {idx['altoTotal']}")
    print(f"terreno_mundo.png: celdas {hechas} con baldosas de PixelLab")


if __name__ == "__main__":
    importar(sys.argv[1] if len(sys.argv) > 1 else TEX / "pixellab/tiles.json")
