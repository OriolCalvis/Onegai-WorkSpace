#!/usr/bin/env python3
"""PIXELLAB — Quita el fondo plano (y el suelo de baldosas) que a veces pinta
create_image_pixen aunque se pida no_background: inundacion desde los bordes
con tolerancia de color respecto a la mediana del borde, y despues se queda
solo la componente opaca mas grande (el edificio).

    python3 tools/pixellab/limpiar_fondo.py entrada.png salida.png [tolerancia=28]

Si la imagen ya es transparente en los bordes no toca nada.
"""
import sys
from collections import deque

import numpy as np
from PIL import Image


def limpiar(ruta_in, ruta_out, tol=28):
    img = Image.open(ruta_in).convert("RGBA")
    a = np.array(img).astype(int)
    h, w = a.shape[:2]
    borde = np.concatenate([a[0], a[-1], a[:, 0], a[:, -1]])
    if (borde[:, 3] == 0).mean() > 0.5:
        img.save(ruta_out)
        print(f"{ruta_in}: ya transparente")
        return
    opacos = borde[borde[:, 3] > 0][:, :3]
    # Colores de fondo: los que dominan el borde. Un fondo plano es un color;
    # el tablero de ajedrez que a veces devuelve pixen son dos o tres (grises
    # exactos), y entonces basta una tolerancia pequena para no comerse el
    # zocalo oscuro del edificio.
    from collections import Counter
    cuenta = Counter(tuple(int(v) for v in p) for p in opacos)
    fondos = [np.array(c) for c, n in cuenta.most_common(4) if n >= 0.08 * len(opacos)] or [np.median(opacos, axis=0)]
    if len(fondos) > 1:
        tol = min(tol, 8)
    parecido = np.zeros(a.shape[:2], bool)
    for fondo in fondos:
        parecido |= (np.abs(a[:, :, :3] - fondo).max(axis=2) <= tol)
    visto = np.zeros((h, w), bool)
    cola = deque()
    for x in range(w):
        for y in (0, h - 1):
            if parecido[y, x] and not visto[y, x]:
                visto[y, x] = True
                cola.append((y, x))
    for y in range(h):
        for x in (0, w - 1):
            if parecido[y, x] and not visto[y, x]:
                visto[y, x] = True
                cola.append((y, x))
    while cola:
        y, x = cola.popleft()
        for ny, nx in ((y + 1, x), (y - 1, x), (y, x + 1), (y, x - 1)):
            if 0 <= ny < h and 0 <= nx < w and not visto[ny, nx] and parecido[ny, nx]:
                visto[ny, nx] = True
                cola.append((ny, nx))
    a[visto, 3] = 0
    # Componente opaca mas grande (quita restos de suelo sueltos).
    from scipy import ndimage
    etiquetas, n = ndimage.label(a[:, :, 3] > 0)
    if n > 1:
        tam = ndimage.sum(np.ones_like(etiquetas), etiquetas, range(1, n + 1))
        mayor = int(np.argmax(tam)) + 1
        a[(etiquetas != mayor), 3] = 0
    Image.fromarray(a.astype(np.uint8), "RGBA").save(ruta_out)
    print(f"{ruta_in}: fondo {[tuple(int(v) for v in f) for f in fondos]} quitado (tol {tol}), {n} componentes")


if __name__ == "__main__":
    limpiar(sys.argv[1], sys.argv[2], int(sys.argv[3]) if len(sys.argv) > 3 else 28)
