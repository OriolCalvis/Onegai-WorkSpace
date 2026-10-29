#!/usr/bin/env python3
"""BLOQUES-02 — La hoja de pajaros (pixellab_pajaros.png, celdas de 32x32).

Lee assets/textures/pixellab/pajaros.json:
    {"hoja": "pixellab_pajaros",
     "especies": [{"id": "paloma", "posado": "lote9/paloma_posada.png", "arriba": "...", "abajo": "..."}]}
Cada PNG de PixelLab es un pajaro de 64x64 mirando a la derecha; aqui se
recorta, se reduce a la mitad (vecino mas proximo) y se centra en una celda
de 32x32 con la base en la fila 29. Una fila por especie: 6 frames, los tres
mirando a la derecha (posado, alas arriba, alas abajo) y los mismos
espejados. Render/BandadaNivel usa frame = 1 + especie * 6 + estado (+3 si
mira a la izquierda). Da de alta la hoja en sprites.json.

    python3 tools/pixellab/importar_pajaros.py
"""
import json
import sys
from pathlib import Path

from PIL import Image, ImageOps

ROOT = Path(__file__).resolve().parents[2]
TEX = ROOT / "assets/textures"
CELDA, FILA_BASE, FRAMES = 32, 29, 6


def encaja(img):
    img = img.convert("RGBA")
    caja = img.getbbox()
    recorte = img.crop(caja) if caja else img
    f = min(CELDA / recorte.width, CELDA / recorte.height, 0.42)
    recorte = recorte.resize((max(1, round(recorte.width * f)), max(1, round(recorte.height * f))), Image.NEAREST)
    celda = Image.new("RGBA", (CELDA, CELDA), (0, 0, 0, 0))
    celda.paste(recorte, ((CELDA - recorte.width) // 2, max(0, FILA_BASE + 1 - recorte.height)), recorte)
    return celda


def importar(indice):
    indice = Path(indice)
    datos = json.loads(indice.read_text())
    hoja, especies = datos["hoja"], datos["especies"]
    img = Image.new("RGBA", (FRAMES * CELDA, len(especies) * CELDA), (0, 0, 0, 0))
    frames = {}
    for fila, e in enumerate(especies):
        derecha = [encaja(Image.open(indice.parent / e[k])) for k in ("posado", "arriba", "abajo")]
        celdas = derecha + [ImageOps.mirror(c) for c in derecha]
        for i, c in enumerate(celdas):
            img.paste(c, (i * CELDA, fila * CELDA))
            frames[f"{e['id']}_{('posado', 'arriba', 'abajo')[i % 3]}_{'d' if i < 3 else 'i'}"] = fila * FRAMES + i + 1
    img.save(TEX / f"{hoja}.png")
    (TEX / f"{hoja}.json").write_text(json.dumps({
        "_nota": "Pajaros de PixelLab (tools/pixellab/importar_pajaros.py): no editar a mano. Celdas de 32x32, una fila por especie: posado, alas arriba, alas abajo mirando a la derecha y los tres espejados. Render/BandadaNivel elige el frame; TextureManager invierte los PNG al cargarlos.",
        "generador": "tools/pixellab/importar_pajaros.py",
        "image": f"{hoja}.png", "frameSize": [CELDA, CELDA], "columns": FRAMES, "rows": len(especies),
        "anclaje": {"x": "centro de la celda", "filaBase": FILA_BASE},
        "especies": [e["id"] for e in especies], "frames": frames,
    }, ensure_ascii=False, indent=2) + "\n")
    registro = json.loads((TEX / "sprites.json").read_text())
    if not any(h["id"] == hoja for h in registro["hojas"]):
        registro["hojas"].append({"id": hoja, "manifiesto": f"textures/{hoja}.json"})
        (TEX / "sprites.json").write_text(json.dumps(registro, ensure_ascii=False, indent=2) + "\n")
        print(f"sprites.json: hoja {hoja} registrada")
    print(f"{hoja}.png: {len(especies)} especies, {len(frames)} frames")


if __name__ == "__main__":
    importar(sys.argv[1] if len(sys.argv) > 1 else TEX / "pixellab/pajaros.json")
