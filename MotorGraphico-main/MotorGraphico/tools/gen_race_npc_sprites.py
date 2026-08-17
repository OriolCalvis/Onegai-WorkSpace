#!/usr/bin/env python3
"""Genera sprites idle de runtime (64x64) para los 43 PNJ raciales.

Las laminas editoriales son la fuente de arte; esta herramienta recorta una
celda por raza, preserva alpha cuando existe, normaliza el personaje a una
celda 64x64 apoyada por los pies y produce tanto archivos individuales como
un atlas 7x7 que puede cargar el motor en una sola textura.
"""
import json
import os
import sys

try:
    from PIL import Image
except ImportError:
    sys.exit("hace falta Pillow: python3 -m pip install Pillow")

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TEX = os.path.join(ROOT, "assets", "textures")
OUT = os.path.join(TEX, "race_npc_sprites")
FRAME = 64
COLUMNS = 7

# raceId, atlas editorial (1..4), frame editorial (1-based).
RACES = [
    ("aarakocra_de_las_montanas", 1, 1), ("aarakocra_del_viento", 1, 2),
    ("alquimistas_de_aegroum", 1, 3), ("cara_loca", 1, 4),
    ("cazador_nocturno", 1, 5), ("diplomaticos_de_bastrea", 1, 6),
    ("draconidos_de_fuego", 1, 7), ("draconidos_de_hielo", 1, 8),
    ("drow_de_la_casa_noble", 1, 9), ("drow_explorador", 1, 10),
    ("elfos_de_las_estrellas", 1, 11), ("elfos_del_bosque", 1, 12),
    ("enanos_forjadores", 2, 1), ("enanos_historiadores", 2, 2),
    ("espectros", 2, 3), ("guardianes_de_la_luz", 2, 4),
    ("guardianes_del_elemento", 2, 5), ("hechiceros_oscuros", 2, 6),
    ("hombres_lagarto_de_la_selva", 2, 7), ("hombres_lagarto_del_pantano", 2, 8),
    ("medianos_del_bosque", 2, 9), ("medianos_urbanos", 2, 10),
    ("mercenarios_de_ascaria", 2, 11), ("minotauros_de_la_llanura", 2, 12),
    ("minotauros_del_laberinto", 3, 1), ("nagas_arcanos", 3, 2),
    ("nagas_venenosos", 3, 3), ("nobles_de_ascaria", 3, 4),
    ("orcos_del_consejo", 3, 5), ("orcos_dramaticos", 3, 6),
    ("race_ascaria_fey_blood", 3, 7), ("race_dwarf_deepforge", 3, 8),
    ("race_elf_canopy", 3, 9), ("race_human_marches", 3, 10),
    ("race_orc_gongorguma", 3, 11), ("revenants", 3, 12),
    ("sagas_de_la_locura", 4, 1), ("sagas_del_abismo", 4, 2),
    ("tieflings_de_la_sombra", 4, 3), ("tieflings_del_vacio", 4, 4),
    ("traidores_oscuros", 4, 5), ("yokai_de_las_sombras", 4, 6),
    ("yokai_de_los_vientos", 4, 7),
]


def source_cell(image, atlas, frame):
    columns, rows = 4, (3 if atlas < 4 else 2)
    cell_w, cell_h = image.width // columns, image.height // rows
    index = frame - 1
    return image.crop(((index % columns) * cell_w, (index // columns) * cell_h,
                       (index % columns + 1) * cell_w, (index // columns + 1) * cell_h))


def make_transparent_if_opaque(image):
    # Los PNG con alpha real ya tienen fondo transparente. Si una fuente
    # viniera opaca con negro puro de fondo, quitar solo ese negro permite
    # reutilizarla sin introducir un rectangulo negro en el juego.
    alpha = image.getchannel("A")
    if alpha.getextrema() != (255, 255):
        return image
    pixels = image.load()
    for y in range(image.height):
        for x in range(image.width):
            r, g, b, _ = pixels[x, y]
            if r < 5 and g < 5 and b < 5:
                pixels[x, y] = (r, g, b, 0)
    return image


def normalize(cell):
    cell = make_transparent_if_opaque(cell.convert("RGBA"))
    box = cell.getchannel("A").getbbox()
    if box is None:
        return Image.new("RGBA", (FRAME, FRAME), (0, 0, 0, 0))
    sprite = cell.crop(box)
    sprite.thumbnail((58, 60), Image.Resampling.LANCZOS)
    output = Image.new("RGBA", (FRAME, FRAME), (0, 0, 0, 0))
    output.alpha_composite(sprite, ((FRAME - sprite.width) // 2, FRAME - sprite.height))
    return output


def main():
    sources = {}
    for atlas in range(1, 5):
        path = os.path.join(TEX, f"editor_race_npcs_{atlas:02d}.png")
        sources[atlas] = Image.open(path).convert("RGBA")
    os.makedirs(OUT, exist_ok=True)

    rows = (len(RACES) + COLUMNS - 1) // COLUMNS
    combined = Image.new("RGBA", (COLUMNS * FRAME, rows * FRAME), (0, 0, 0, 0))
    frames = []
    for index, (race_id, atlas, source_frame) in enumerate(RACES, start=1):
        image = normalize(source_cell(sources[atlas], atlas, source_frame))
        image.save(os.path.join(OUT, race_id + "_idle.png"))
        combined.alpha_composite(image, (((index - 1) % COLUMNS) * FRAME,
                                        ((index - 1) // COLUMNS) * FRAME))
        frames.append({"raceId": race_id, "frame": index})

    combined.save(os.path.join(TEX, "race_npc_idle.png"))
    with open(os.path.join(TEX, "race_npc_idle.json"), "w", encoding="utf-8") as out:
        json.dump({"image": "race_npc_idle.png", "frame": {"width": FRAME, "height": FRAME},
                   "columns": COLUMNS, "frames": frames}, out, ensure_ascii=False, indent=2)
    print(f"sprites: {len(frames)} individuales + atlas {combined.size[0]}x{combined.size[1]}")


if __name__ == "__main__":
    main()
