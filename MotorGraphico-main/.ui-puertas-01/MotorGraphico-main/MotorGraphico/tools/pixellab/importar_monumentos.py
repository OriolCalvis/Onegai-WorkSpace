#!/usr/bin/env python3
"""BLOQUES-03 — Monumentos de PixelLab (un sprite grande por edificio) como
hojas de tiras para Game::Monumentos / Render::MonumentosNivel.

Lee assets/textures/pixellab/monumentos.json:
    {"monumentos": [{"id": "coliseo", "imagen": "lote10/coliseo.png", "tiles": ["viejo_coliseo"],
                     "ancho": 8, "alto": 6, "pixellab_id": "..."}]}
Para cada uno: recorta el margen transparente, lo escala (vecino mas proximo)
a la anchura de su huella en pantalla, (ancho + alto) * 32 px, lo deja en una
hoja de tiras de 32 px de ancho (una fila, assets/textures/monumentos/<id>.png
+ .json con el formato de las hojas de props) y calcula el anclaje: el punto
mas bajo del dibujo, en el centro, es la esquina delantera de la huella.
Escribe assets/textures/monumentos.json (lo que lee el motor) y da de alta
cada hoja en sprites.json.

    python3 tools/pixellab/importar_monumentos.py [indice.json ...]

Sin argumentos lee pixellab/monumentos.json y, si existen, iso/peces.json (las
piezas del kit isometrico: "escala": false y "anclaje" exacto, sin reescalar)
e iso/monuments/monuments.json (MON-01: los siete monumentos del kit) e
iso/interiors/peces_grans.json (INT-01: las piezas grandes de los interiores).
Una entrada con "retirado": true se conserva en el indice pero no se importa.
"""
import json
import math
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
TEX = ROOT / "assets/textures"
TIRA = 32


def prepara(img, ancho, alto):
    """La imagen recortada y escalada a la anchura de la huella; devuelve (hoja, anclaje)."""
    img = img.convert("RGBA")
    caja = img.getbbox()
    if caja is None:
        raise SystemExit("imagen vacia")
    img = img.crop(caja)
    objetivo = (ancho + alto) * TIRA
    factor = objetivo / img.width
    img = img.resize((objetivo, max(1, round(img.height * factor))), Image.NEAREST)
    columnas = math.ceil(img.width / TIRA)
    hoja = Image.new("RGBA", (columnas * TIRA, img.height), (0, 0, 0, 0))
    hoja.paste(img, (0, 0), img)
    # Anclaje: el pixel opaco mas bajo de la mitad central es la esquina delantera.
    alpha = hoja.getchannel("A")
    centro = hoja.width // 2
    ay = hoja.height - 1
    for y in range(hoja.height - 1, -1, -1):
        if any(alpha.getpixel((x, y)) > 0 for x in range(max(0, centro - 8), min(hoja.width, centro + 8))):
            ay = y + 1
            break
    return hoja, (centro, ay), columnas


def exacta(img, ax, ay):
    """ISO — Una pieza del kit isometrico (tools/iso): sin escalar; se rellena por
    la izquierda para que la esquina delantera caiga en el borde de una tira de
    32 px (las tiras quedan alineadas con las columnas de celdas)."""
    img = img.convert("RGBA")
    izq = (-ax) % TIRA
    columnas = math.ceil((img.width + izq) / TIRA)
    hoja = Image.new("RGBA", (columnas * TIRA, img.height), (0, 0, 0, 0))
    hoja.paste(img, (izq, 0), img)
    return hoja, (ax + izq, ay), columnas


def importar(*indices):
    (TEX / "monumentos").mkdir(exist_ok=True)
    registro = json.loads((TEX / "sprites.json").read_text())
    salida = []
    entradas = []
    for indice in indices:
        indice = Path(indice)
        if indice.exists():
            entradas += [(indice, m) for m in json.loads(indice.read_text())["monumentos"] if not m.get("retirado")]
    for indice, m in entradas:
        img = Image.open(indice.parent / m["imagen"])
        if m.get("escala") is False:                    # ISO: pieza exacta con su anclaje
            hoja, (ax, ay), columnas = exacta(img, int(m["anclaje"]["x"]), int(m["anclaje"]["y"]))
        else:
            hoja, (ax, ay), columnas = prepara(img, int(m["ancho"]), int(m["alto"]))
        hid = f"monumento_{m['id']}"
        hoja.save(TEX / "monumentos" / f"{m['id']}.png")
        (TEX / "monumentos" / f"{m['id']}.json").write_text(json.dumps({
            "_nota": "Monumento de PixelLab en tiras (tools/pixellab/importar_monumentos.py): no editar a mano. Una fila; cada frame es una tira de 32 px de ancho por todo el alto. TextureManager invierte los PNG al cargarlos.",
            "generador": "tools/pixellab/importar_monumentos.py",
            "image": f"{m['id']}.png", "frameSize": [TIRA, hoja.height], "columns": columnas, "rows": 1,
            "frames": {f"{m['id']}_tira_{k + 1}": k + 1 for k in range(columnas)},
        }, ensure_ascii=False, indent=2) + "\n")
        if not any(h["id"] == hid for h in registro["hojas"]):
            registro["hojas"].append({"id": hid, "manifiesto": f"textures/monumentos/{m['id']}.json"})
        entrada = {"id": m["id"], "hoja": hid, "tiles": m["tiles"], "ancho": int(m["ancho"]), "alto": int(m["alto"]),
                   "tiraAncho": TIRA, "altoHoja": hoja.height, "columnas": columnas, "anclaje": {"x": ax, "y": ay},
                   "pixellab_id": m.get("pixellab_id", "")}
        if m.get("posaderos"):                          # MASC-01: on es posen els coloms (peces del kit)
            entrada["posaderos"] = m["posaderos"]
        salida.append(entrada)
        print(f"{m['id']}: hoja {hoja.width}x{hoja.height}, {columnas} tiras, anclaje ({ax}, {ay})")
    # ISO: las piezas del kit (ids iso_*) que ya no estan en su indice se quitan del
    # registro y de assets/textures/monumentos (el kit las regenera todas).
    vivas = {f"monumento_{m['id']}" for m in salida}
    for h in [h for h in registro["hojas"] if h["id"].startswith("monumento_iso_") and h["id"] not in vivas]:
        registro["hojas"].remove(h)
        nombre = h["id"][len("monumento_"):]
        for ext in (".png", ".json"):
            ruta = TEX / "monumentos" / f"{nombre}{ext}"
            if ruta.exists():
                ruta.unlink()
    (TEX / "sprites.json").write_text(json.dumps(registro, ensure_ascii=False, indent=2) + "\n")
    (TEX / "monumentos.json").write_text(json.dumps({
        "_nota": "BLOQUES-03 — edificios con sprite propio (Game::Monumentos): no editar a mano, lo escribe tools/pixellab/importar_monumentos.py. Una huella del mapa que mida ancho x alto celdas de esos tiles se dibuja con la hoja en tiras; si no mide eso, con bloques.",
        "tileset": "terreno_mundo", "indiceTileset": "textures/terreno_iso.json", "monumentos": salida,
    }, ensure_ascii=False, indent=2) + "\n")


if __name__ == "__main__":
    importar(*(sys.argv[1:] or [TEX / "pixellab/monumentos.json", TEX / "iso/peces.json",
                                TEX / "iso/monuments/monuments.json", TEX / "iso/interiors/peces_grans.json"]))
