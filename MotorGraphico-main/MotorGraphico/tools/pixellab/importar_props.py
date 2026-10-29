#!/usr/bin/env python3
"""PIXELLAB — Monta una hoja de props (celdas 64x64) con dibujos de PixelLab.

Lee assets/textures/pixellab/props.json:
    {"hoja": "pixellab_props", "columnas": 8,
     "props": [{"id": "pozo", "imagen": "lote0/prop_pozo.png"}, ...]}
y escribe assets/textures/<hoja>.png + <hoja>.json con el formato de
props_p0.json (image, frameSize, columns, rows, frames{id: n}) que lee
Game::RegistroSprites, dejando cada dibujo centrado y con su base en la fila
61 de la celda (el anclaje de los actores). Si la hoja no esta en
assets/textures/sprites.json la anade.

    python3 tools/pixellab/importar_props.py
"""
import json
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
TEX = ROOT / "assets/textures"
CELDA, FILA_BASE = 64, 61


def encaja(img, CELDA=CELDA, FILA_BASE=FILA_BASE):
    """El dibujo centrado en una celda de 64x64 con la base en la fila 61
    (BLOQUES-04: o de "celda" px con la base a 3 px del borde, para las
    hojas de props grandes: barcos, grua, sauce a 128x128)."""
    img = img.convert("RGBA")
    caja = img.getbbox()
    if caja is None:
        return Image.new("RGBA", (CELDA, CELDA), (0, 0, 0, 0))
    recorte = img.crop(caja)
    if recorte.width > CELDA or recorte.height > CELDA:
        f = min(CELDA / recorte.width, CELDA / recorte.height)
        recorte = recorte.resize((max(1, int(recorte.width * f)), max(1, int(recorte.height * f))), Image.NEAREST)
    celda = Image.new("RGBA", (CELDA, CELDA), (0, 0, 0, 0))
    x = (CELDA - recorte.width) // 2
    y = FILA_BASE + 1 - recorte.height
    celda.paste(recorte, (x, max(0, y)), recorte)
    return celda


def importar(indice):
    indice = Path(indice)
    datos = json.loads(indice.read_text())
    hoja, columnas = datos["hoja"], int(datos.get("columnas", 8))
    celda_px = int(datos.get("celda", CELDA))
    fila_base = celda_px - (CELDA - FILA_BASE)
    props = datos["props"]
    filas = max(1, (len(props) + columnas - 1) // columnas)
    img = Image.new("RGBA", (columnas * celda_px, filas * celda_px), (0, 0, 0, 0))
    frames = {}
    for i, p in enumerate(props):
        celda = encaja(Image.open(indice.parent / p["imagen"]), celda_px, fila_base)
        if p.get("espejo"):  # PROLOGO-02: la otra orientacion (la barca a lo largo del rio)
            celda = celda.transpose(Image.FLIP_LEFT_RIGHT)
        img.paste(celda, ((i % columnas) * celda_px, (i // columnas) * celda_px))
        frames[p["id"]] = i + 1
    img.save(TEX / f"{hoja}.png")
    manifiesto = {
        "_nota": f"Props de PixelLab (tools/pixellab/importar_props.py desde {indice.name}): no editar a mano. Frames 1-based por filas; base en la fila {fila_base} de la celda de {celda_px} px (como props_p0 a 64).",
        "generador": "tools/pixellab/importar_props.py",
        "image": f"{hoja}.png", "frameSize": [celda_px, celda_px], "columns": columnas, "rows": filas,
        "anclaje": {"x": "centro de la celda", "filaBase": fila_base},
        "frames": frames,
        "objetos": [{"id": p["id"], "frame": i + 1, "pixellab_id": p.get("pixellab_id", "")} for i, p in enumerate(props)],
    }
    (TEX / f"{hoja}.json").write_text(json.dumps(manifiesto, ensure_ascii=False, indent=2) + "\n")
    registro = json.loads((TEX / "sprites.json").read_text())
    if not any(h["id"] == hoja for h in registro["hojas"]):
        registro["hojas"].append({"id": hoja, "manifiesto": f"textures/{hoja}.json"})
        (TEX / "sprites.json").write_text(json.dumps(registro, ensure_ascii=False, indent=2) + "\n")
        print(f"sprites.json: hoja {hoja} registrada")
    print(f"{hoja}.png: {len(props)} props en {columnas}x{filas} celdas")


if __name__ == "__main__":
    importar(sys.argv[1] if len(sys.argv) > 1 else TEX / "pixellab/props.json")
