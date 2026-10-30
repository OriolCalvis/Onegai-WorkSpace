#!/usr/bin/env python3
"""La prueba del Cazademonios por tier (tools/actores_trajes_caza.py, PRUEBA GUARDADA): el set de Tier 2 y
los de Tier 3, 4 y 5, quietos de frente, de perfil y de espaldas, en grande. No toca los atlas.

    python3 tools/ver_cazademonios_tiers.py salida.png [escala]

En el motor (con el brillo y las particulas de cada tier): planificacion/seguimiento/TIERS-01.json.
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, str(Path(__file__).resolve().parent))
import actores_trajes  # noqa: E402,F401
import actores_trajes_caza as caza  # noqa: E402
import actores_personajes as pers  # noqa: E402
from actores_tallas import construir_tallas  # noqa: E402

FONDO = (96, 104, 88)


def main():
    salida = sys.argv[1] if len(sys.argv) > 1 else "cazademonios_tiers.png"
    esc = int(sys.argv[2]) if len(sys.argv) > 2 else 4
    tallas = construir_tallas()
    try:
        letra = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 15)
    except OSError:
        letra = ImageFont.load_default()
    cols = [17, 15, 13]  # quieto: de frente, de perfil, de espaldas
    caja = (4, 0, 60, 64)
    w, h = (caja[2] - caja[0]) * esc, (caja[3] - caja[1]) * esc
    img = Image.new("RGB", (len(cols) * w, 4 * (h + 22)), FONDO)
    d = ImageDraw.Draw(img)
    for i, tier in enumerate((2, 3, 4, 5)):
        nombre, eq = caza.SETS[tier]
        spec = {"raza": "race_human_marches", "barba": None, "pelo": ("corto", "pelo_negro"), "equipo": eq}
        fr = pers.componer_personaje(spec, tallas)
        d.text((6, i * (h + 22) + 2), nombre, font=letra, fill=(255, 240, 200))
        for j, fi in enumerate(cols):
            px = fr[fi - 1][caja[1]:caja[3], caja[0]:caja[2]]
            im = Image.fromarray(np.ascontiguousarray(px), "RGBA")
            fondo = Image.new("RGBA", im.size, FONDO + (255,))
            fondo.alpha_composite(im)
            img.paste(fondo.resize((w, h), Image.NEAREST).convert("RGB"), (j * w, i * (h + 22) + 22))
    img.save(salida)
    print(salida, img.size)


if __name__ == "__main__":
    main()
