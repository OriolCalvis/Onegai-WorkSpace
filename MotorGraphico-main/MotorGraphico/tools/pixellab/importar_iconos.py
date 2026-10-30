#!/usr/bin/env python3
"""PIXELLAB — Iconos de PixelLab en el atlas iconos.png (celdas 16x16).

Lee assets/textures/pixellab/iconos.json:
    {"iconos": {"pv": "lote2/pv.png", "oro": "lote2/oro.png", ...}}
y pinta cada PNG (16x16; si viene mas grande se reduce al vecino mas proximo)
en la celda que iconos.json asigna a ese id (el contrato que lee el motor no
cambia: mismos frames, mismas tablas). Un id que no este en iconos.json se
avisa y se salta. tools/gen_iconos.py vuelve a dibujar los suyos y al acabar
llama a esto para reponer los de PixelLab.

    python3 tools/pixellab/importar_iconos.py
"""
import json
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
TEX = ROOT / "assets/textures"


def importar(indice):
    indice = Path(indice)
    datos = json.loads(indice.read_text())
    contrato = json.loads((TEX / "iconos.json").read_text())
    w, h = contrato["frameSize"]
    columnas = contrato["columns"]
    atlas = Image.open(TEX / contrato["image"]).convert("RGBA")
    hechos, saltados = [], []
    for icono, rel in datos["iconos"].items():
        entrada = contrato["iconos"].get(icono)
        if entrada is None:
            saltados.append(icono)
            continue
        frame = entrada["frame"] if isinstance(entrada, dict) else int(entrada)
        img = Image.open(indice.parent / rel).convert("RGBA")
        if img.size != (w, h):
            img = img.resize((w, h), Image.NEAREST)
        x, y = ((frame - 1) % columnas) * w, ((frame - 1) // columnas) * h
        atlas.paste(Image.new("RGBA", (w, h), (0, 0, 0, 0)), (x, y))
        atlas.paste(img, (x, y), img)
        hechos.append(icono)
    atlas.save(TEX / contrato["image"])
    print(f"{contrato['image']}: {len(hechos)} iconos de PixelLab" + (f"; sin celda: {saltados}" if saltados else ""))


if __name__ == "__main__":
    importar(sys.argv[1] if len(sys.argv) > 1 else TEX / "pixellab/iconos.json")
