#!/usr/bin/env python3
"""Convierte la hoja fuente de actores de Boundington en un atlas runtime.

La fuente es una cuadrícula 4x3 de conceptos a 362 px por celda. El motor
usa frames de 64x64: se reduce cada celda completa con NEAREST, conservando
el alpha, para que los pies permanezcan en el borde inferior de su frame.
"""
from pathlib import Path
import json

from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "assets/textures/boundington/boundington_story_actors_source_v1.png"
ATLAS = ROOT / "assets/textures/boundington_story_actors_idle.png"
MANIFEST = ROOT / "assets/textures/boundington_story_actors_idle.json"

CELL_SOURCE = 362
CELL_RUNTIME = 64
COLUMNS = 4
ROWS = 3

FRAMES = {
    "luisarda": 1,
    "ben_kafka": 2,
    "griffin": 3,
    "duende_porcelana": 4,
    "perdido_saqueador": 5,
    "perdido_fanatico": 5,
    "sectario_perdido": 5,
    "cultista_carcelero": 6,
    "saga_bosque": 7,
    "nina_del_gato": 8,
    "guardia": 9,
    "parroquiano_humilde": 10,
    "tendero_mercado": 10,
    "naga_desague": 11,
    "rata_alcantarilla": 12,
}


def main() -> None:
    source = Image.open(SOURCE).convert("RGBA")
    expected = (CELL_SOURCE * COLUMNS, CELL_SOURCE * ROWS)
    if source.size != expected:
        raise SystemExit(f"Fuente inesperada {source.size}; se esperaba {expected}")

    atlas = Image.new("RGBA", (CELL_RUNTIME * COLUMNS, CELL_RUNTIME * ROWS))
    for frame in range(1, COLUMNS * ROWS + 1):
        index = frame - 1
        col, row = index % COLUMNS, index // COLUMNS
        box = (col * CELL_SOURCE, row * CELL_SOURCE,
               (col + 1) * CELL_SOURCE, (row + 1) * CELL_SOURCE)
        sprite = source.crop(box).resize((CELL_RUNTIME, CELL_RUNTIME), Image.Resampling.NEAREST)
        atlas.alpha_composite(sprite, (col * CELL_RUNTIME, row * CELL_RUNTIME))

    atlas.save(ATLAS)
    MANIFEST.write_text(json.dumps({
        "_nota": "Atlas runtime 4x3 de actores principales de Boundington. Frames 1-based.",
        "image": "boundington_story_actors_idle.png",
        "frameSize": [CELL_RUNTIME, CELL_RUNTIME],
        "columns": COLUMNS,
        "frames": FRAMES,
    }, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"{ATLAS.relative_to(ROOT)}: {COLUMNS * ROWS} frames")


if __name__ == "__main__":
    main()
