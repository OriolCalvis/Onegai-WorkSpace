"""Previews for the Señor de las Armas sketches (not part of the repo)."""
import os
import sys
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw, ImageFont

# tools/ del repo: MotorGraphico/tools (aquesta carpeta és a MotorGraphico/Claude outputs/<nom>/codi)
TOOLS = Path(os.environ.get("MG_TOOLS", Path(__file__).resolve().parents[3] / "tools"))
sys.path.insert(0, str(TOOLS))
import actores_trajes  # noqa: F401,E402
import actores_trajes_iteracion3  # noqa: F401,E402
import actores_personajes as pers  # noqa: E402
from actores_tallas import construir_tallas  # noqa: E402
from actores_materiales import MATERIALES  # noqa: E402

FONDO = (78, 84, 72)
_TALLAS = None


def tallas():
    global _TALLAS
    if _TALLAS is None:
        _TALLAS = construir_tallas()
    return _TALLAS


def tenir(frames, tinte):
    """tinte [(material origen, material destino)]: tone by tone."""
    if not tinte:
        return frames
    pares = []
    for o, d in tinte:
        for t in ("H", "s", "d", "o", "O"):
            pares.append((MATERIALES[o].tono[t], MATERIALES[d].tono[t]))
    out = []
    for px in frames:
        px = px.copy()
        rgb = px[..., :3].copy()
        hecho = np.zeros(px.shape[:2], bool)
        for de, a in pares:
            sel = (rgb == de).all(-1) & ~hecho & (px[..., 3] > 0)
            px[sel, :3] = a
            hecho |= sel
        out.append(px)
    return out


def frames_de(spec, tintes=None, extra=None):
    """All 20 frames of a spec. tintes {ranura: [(o, d)]}; extra: fn(frames, tallas) -> frames."""
    from actores_capas import componer
    capas = pers.capas_personaje(spec, tallas())
    tintes = tintes or {}
    capas = [(r, tenir(fr, tintes.get(r))) for r, fr in capas]
    fr = componer(capas)
    if extra:
        fr = extra(fr)
    return fr


def letra(n=14):
    try:
        return ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", n)
    except OSError:
        return ImageFont.load_default()


def tira(fr, cols, esc=4, caja=(0, 0, 64, 64), fondo=FONDO):
    w, h = (caja[2] - caja[0]) * esc, (caja[3] - caja[1]) * esc
    img = Image.new("RGB", (len(cols) * w, h), fondo)
    for j, fi in enumerate(cols):
        px = fr[fi - 1][caja[1]:caja[3], caja[0]:caja[2]]
        im = Image.fromarray(np.ascontiguousarray(px), "RGBA")
        f = Image.new("RGBA", im.size, fondo + (255,))
        f.alpha_composite(im)
        img.paste(f.resize((w, h), Image.NEAREST).convert("RGB"), (j * w, 0))
    return img
