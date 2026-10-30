"""A rough preview of the engine's glow on the sketches: a gold aura behind the silhouette, a bloom on the
glowing pixels (mana, visor, molten gold) and a few rising sparks. Only for the sketch sheet."""
import numpy as np
from PIL import Image, ImageFilter, ImageChops

from actores_materiales import MATERIALES


def tonos_que_brillan(nombres, tonos=("H", "s")):
    out = set()
    for n in nombres:
        for t in tonos:
            out.add(tuple(MATERIALES[n].tono[t]))
    return out


def con_efectos(px, esc, brillan, aura=(255, 205, 90), chispas=(), fondo=(34, 36, 42), t=0.0,
                fuerza_aura=0.55, fuerza_brillo=1.0):
    """px: frame RGBA (h, w, 4) uint8 -> RGB image scaled by esc with the effects."""
    h, w = px.shape[:2]
    sprite = Image.fromarray(np.ascontiguousarray(px), "RGBA").resize((w * esc, h * esc), Image.NEAREST)
    base = Image.new("RGBA", sprite.size, fondo + (255,))
    # aura: the silhouette, blurred, in gold, behind the sprite
    a = sprite.split()[3].point(lambda v: 255 if v else 0)
    a = a.filter(ImageFilter.MaxFilter(5)).filter(ImageFilter.GaussianBlur(esc * 2.2))
    a = a.point(lambda v: int(v * fuerza_aura))
    capa_aura = Image.new("RGBA", sprite.size, aura + (0,))
    capa_aura.putalpha(a)
    base.alpha_composite(capa_aura)
    base.alpha_composite(sprite)
    # bloom of the glowing pixels
    rgb = px[..., :3]
    sel = np.zeros(px.shape[:2], bool)
    for c in brillan:
        sel |= (rgb == c).all(-1) & (px[..., 3] > 0)
    if sel.any():
        glow = np.zeros_like(px)
        glow[sel] = px[sel]
        glow[sel, 3] = 255
        g = Image.fromarray(glow, "RGBA").resize(sprite.size, Image.NEAREST)
        g = g.filter(ImageFilter.GaussianBlur(esc * 1.3))
        arr = np.array(g).astype(np.float32)
        arr[..., :3] *= (arr[..., 3:4] / 255.0) * fuerza_brillo
        out = np.array(base.convert("RGB")).astype(np.float32)
        out = 255 - (255 - out) * (255 - np.clip(arr[..., :3], 0, 255)) / 255  # screen
        base = Image.fromarray(np.clip(out, 0, 255).astype(np.uint8), "RGB").convert("RGBA")
    # sparks: (x, y0, speed, color) in sprite pixels; they rise with t and fade
    if chispas:
        arr = np.array(base)
        for (x, y0, vel, col) in chispas:
            y = y0 - (t * vel) % 18
            vida = 1 - ((t * vel) % 18) / 18
            yy, xx = int(round(y * esc)), int(round(x * esc))
            for dy in range(esc // 2 + 1):
                for dx in range(esc // 2 + 1):
                    if 0 <= yy + dy < arr.shape[0] and 0 <= xx + dx < arr.shape[1]:
                        c = np.array(col, np.float32)
                        arr[yy + dy, xx + dx, :3] = np.clip(arr[yy + dy, xx + dx, :3] * (1 - vida) + c * vida, 0, 255)
        base = Image.fromarray(arr, "RGBA")
    return base.convert("RGB")
