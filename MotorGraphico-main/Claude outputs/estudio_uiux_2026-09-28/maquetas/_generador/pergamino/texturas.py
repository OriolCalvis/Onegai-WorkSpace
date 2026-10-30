"""Texturas pixel-art del tema pergamino (generadas, sin arte de terceros).

Todo se dibuja en una rejilla lógica y se amplía x2 con vecino más
cercano, como los sprites del juego: cada píxel lógico = 2x2 en pantalla.

  hoja(w, h)       pergamino con bordes rasgados, canto envejecido y manchas
  papiro(w, h)     hoja con rodillos de madera y remates de latón arriba y abajo
  cuero(w, h)      banda de cuero con costura y remaches (lomo del cuaderno)
  pestana(w, h)    pestaña de índice de pergamino (activa o inactiva)
  adorno(w)        filete de tinta con rombo central
  sello()          sello de lacre para confirmaciones
"""
import os
import random

import numpy as np
from PIL import Image

E = 2  # escala de píxel

PAPEL = np.array([
    (242, 228, 193),  # 0 luz
    (235, 219, 178),  # 1 claro
    (226, 205, 156),  # 2 base
    (214, 188, 133),  # 3 sombra
    (198, 165, 108),  # 4 mancha
    (174, 138, 85),   # 5 canto
], dtype=np.uint8)
LINEA = (74, 52, 33)        # contorno de tinta
ADORNO = (107, 80, 56)      # tinta desvaída para filetes
BAYER = np.array([[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]) / 16.0


def _ruido(W, H, celda, rnd):
    gw, gh = max(2, W // celda + 2), max(2, H // celda + 2)
    g = rnd.random((gh, gw))
    ys, xs = np.mgrid[0:H, 0:W]
    fy, fx = ys / celda, xs / celda
    y0, x0 = fy.astype(int), fx.astype(int)
    ty, tx = fy - y0, fx - x0
    ty, tx = ty * ty * (3 - 2 * ty), tx * tx * (3 - 2 * tx)
    a = g[y0, x0] * (1 - tx) + g[y0, x0 + 1] * tx
    b = g[y0 + 1, x0] * (1 - tx) + g[y0 + 1, x0 + 1] * tx
    return a * (1 - ty) + b * ty


def _perfil(n, amp, rnd, celda=5):
    """Desplazamiento 0..amp a lo largo de un borde (rasgado suave)."""
    if amp <= 0:
        return np.zeros(n, dtype=int)
    puntos = rnd.random(n // celda + 3)
    xs = np.arange(n) / celda
    i = xs.astype(int)
    t = xs - i
    v = puntos[i] * (1 - t) + puntos[i + 1] * t
    v = v + (rnd.random(n) - 0.5) * 0.35
    return np.clip(np.round(v * amp), 0, amp).astype(int)


def _mascara(W, H, rasgado, rnd, lados="tblr"):
    m = np.ones((H, W), dtype=bool)
    if "t" in lados:
        p = _perfil(W, rasgado, rnd)
        for x in range(W):
            m[:p[x], x] = False
    if "b" in lados:
        p = _perfil(W, rasgado, rnd)
        for x in range(W):
            if p[x]:
                m[H - p[x]:, x] = False
    if "l" in lados:
        p = _perfil(H, rasgado, rnd)
        for y in range(H):
            m[y, :p[y]] = False
    if "r" in lados:
        p = _perfil(H, rasgado, rnd)
        for y in range(H):
            if p[y]:
                m[y, W - p[y]:] = False
    # esquinas mordidas
    for (cy, cx) in ((0, 0), (0, W - 1), (H - 1, 0), (H - 1, W - 1)):
        r = rnd.integers(1, 3 + rasgado)
        for dy in range(r):
            for dx in range(r - dy):
                yy = cy + dy if cy == 0 else cy - dy
                xx = cx + dx if cx == 0 else cx - dx
                m[yy, xx] = False
    return m


def _distancia_borde(m):
    """Distancia (en píxeles lógicos, 4-vecinos) al exterior de la máscara."""
    H, W = m.shape
    d = np.where(m, 999, 0).astype(int)
    for _ in range(14):
        n = d.copy()
        n[1:, :] = np.minimum(n[1:, :], d[:-1, :] + 1)
        n[:-1, :] = np.minimum(n[:-1, :], d[1:, :] + 1)
        n[:, 1:] = np.minimum(n[:, 1:], d[:, :-1] + 1)
        n[:, :-1] = np.minimum(n[:, :-1], d[:, 1:] + 1)
        d = np.where(m, n, 0)
    d[0, :] = np.minimum(d[0, :], 1)
    d[-1, :] = np.minimum(d[-1, :], 1)
    d[:, 0] = np.minimum(d[:, 0], 1)
    d[:, -1] = np.minimum(d[:, -1], 1)
    return d


def _papel(W, H, rnd, tono=0.0, manchas=3, canto=9, fibras=True, marco=None, oscuro=0.0):
    t = (0.34 + tono
         + 0.20 * (_ruido(W, H, 26, rnd) - 0.5)
         + 0.10 * (_ruido(W, H, 7, rnd) - 0.5)
         + 0.05 * (rnd.random((H, W)) - 0.5))
    ys, xs = np.mgrid[0:H, 0:W]
    for _ in range(manchas):
        cy, cx = rnd.random() * H, rnd.random() * W
        r = 6 + rnd.random() * min(W, H) * 0.18
        t += 0.10 * np.exp(-(((ys - cy) ** 2 + (xs - cx) ** 2) / (r * r)))
    if fibras:
        n = int(W * H * 0.004)
        for _ in range(n):
            y, x = rnd.integers(0, H), rnd.integers(0, W)
            L = rnd.integers(3, 8)
            t[y, x:x + L] += 0.11
    t += oscuro
    return t


def _cuantiza(t, m, d, canto):
    rim = np.clip(1 - d / float(canto), 0, 1) ** 1.3 * 0.72
    t = t + rim
    H, W = t.shape
    umbral = BAYER[np.arange(H)[:, None] % 4, np.arange(W)[None, :] % 4]
    idx = np.clip(np.floor(t * (len(PAPEL) - 1) + umbral - 0.5 + 0.5), 0, len(PAPEL) - 1).astype(int)
    img = np.zeros((H, W, 4), dtype=np.uint8)
    img[..., :3] = PAPEL[idx]
    img[..., 3] = np.where(m, 255, 0)
    # contorno de tinta de 1 px lógico
    quemado = m & (d == 2)
    img[quemado, :3] = PAPEL[5]
    borde = m & (d <= 1)
    img[borde, :3] = LINEA
    return img


def _filete(img, inset, m, color=ADORNO):
    """Marco interior de tinta desvaída con rombos en las esquinas."""
    H, W = img.shape[:2]
    a, b = inset, inset
    c, dd = W - 1 - inset, H - 1 - inset
    for x in range(a + 3, c - 2):
        img[b, x, :3] = color
        img[dd, x, :3] = color
    for y in range(b + 3, dd - 2):
        img[y, a, :3] = color
        img[y, c, :3] = color
    for (y, x) in ((b, a), (b, c), (dd, a), (dd, c)):
        for dy in range(-2, 3):
            for dx in range(-2, 3):
                if abs(dy) + abs(dx) <= 2 and 0 <= y + dy < H and 0 <= x + dx < W:
                    img[y + dy, x + dx, :3] = color
    return img


def _escala(img):
    return Image.fromarray(img, "RGBA").resize((img.shape[1] * E, img.shape[0] * E), Image.NEAREST)


def hoja(w, h, semilla=1, rasgado=2, canto=9, filete=None, tono=0.0, manchas=3, oscuro=0.0):
    rnd = np.random.default_rng(semilla)
    W, H = w // E, h // E
    m = _mascara(W, H, rasgado, rnd)
    d = _distancia_borde(m)
    img = _cuantiza(_papel(W, H, rnd, tono, manchas, canto, oscuro=oscuro), m, d, canto)
    if filete:
        img = _filete(img, filete, m)
    return _escala(img)


MADERA = [(58, 36, 18), (110, 68, 36), (143, 93, 52), (176, 122, 69), (160, 108, 60), (143, 93, 52), (122, 78, 42), (98, 61, 31), (75, 46, 23), (58, 36, 18)]
LATON = [(74, 53, 20), (156, 122, 46), (201, 162, 74), (230, 198, 106), (214, 180, 90), (201, 162, 74), (176, 140, 60), (156, 122, 46), (122, 94, 34), (74, 53, 20)]


def _rodillo(W, alto=10, remate=7):
    img = np.zeros((alto + 2, W, 4), dtype=np.uint8)
    for y in range(alto):
        img[y + 1, remate:W - remate, :3] = MADERA[y]
        img[y + 1, remate:W - remate, 3] = 255
    # remates de latón, un píxel más altos
    for lado in (range(0, remate), range(W - remate, W)):
        for x in lado:
            for y in range(alto + 2):
                yy = min(max(y - 1, 0), alto - 1)
                img[y, x, :3] = LATON[yy]
                img[y, x, 3] = 255
        xs = list(lado)
        for x in (xs[0], xs[-1]):  # redondeo
            img[0, x, 3] = 0
            img[alto + 1, x, 3] = 0
    # contorno
    a = img[..., 3] > 0
    borde = a & ~(np.roll(a, 1, 0) & np.roll(a, -1, 0) & np.roll(a, 1, 1) & np.roll(a, -1, 1))
    img[borde, :3] = (45, 28, 14)
    return img


def papiro(w, h, semilla=3, sobresale=8):
    """Hoja enrollada: rodillos arriba y abajo que sobresalen por los lados."""
    rnd = np.random.default_rng(semilla)
    W, H = w // E, h // E
    Wh = W - 2 * sobresale
    alto_r = 12
    Hh = H - 2 * (alto_r // 2)
    m = _mascara(Wh, Hh, 1, rnd, lados="lr")
    d = _distancia_borde(m)
    t = _papel(Wh, Hh, rnd, 0.0, 3, 8)
    # sombra de la curva junto a los rodillos
    ys = np.arange(Hh)[:, None]
    t = t + 0.40 * np.clip(1 - ys / 7.0, 0, 1) + 0.40 * np.clip(1 - (Hh - 1 - ys) / 7.0, 0, 1)
    # solo los lados llevan canto envejecido fuerte
    dl = np.minimum(np.arange(Wh)[None, :], (Wh - 1 - np.arange(Wh))[None, :]).repeat(Hh, 0)
    dd = np.where(m, np.minimum(dl + 1, 99), 0)
    hojaimg = _cuantiza(t, m, dd, 7)
    hojaimg = _filete(hojaimg, 7, m)
    img = np.zeros((H, W, 4), dtype=np.uint8)
    y0 = alto_r // 2
    img[y0:y0 + Hh, sobresale:sobresale + Wh] = hojaimg
    rod = _rodillo(W, alto_r - 2)
    for yy in (0, H - rod.shape[0]):
        a = rod[..., 3] > 0
        img[yy:yy + rod.shape[0]][a] = rod[a]
    return _escala(img)


CUERO = np.array([(63, 40, 22), (78, 50, 29), (94, 61, 35), (109, 72, 42)], dtype=np.uint8)


def _esquina_laton(img, tam=9):
    """Cantoneras de latón en las cuatro esquinas (sobre una imagen RGBA lógica)."""
    H, W = img.shape[:2]
    base = np.zeros((tam, tam, 4), dtype=np.uint8)
    for y in range(tam):
        for x in range(tam - y):
            col = LATON[2]
            if x == 0 or y == 0:
                col = LATON[3]
            if x + y == tam - 1:
                col = LATON[0]
            elif x + y == tam - 2:
                col = LATON[6]
            base[y, x] = (*col, 255)
    base[2, 2] = (*LATON[0], 255)  # clavo
    base[3, 3] = (*LATON[4], 255)
    for (fy, fx) in ((False, False), (False, True), (True, False), (True, True)):
        t = base
        if fy:
            t = t[::-1]
        if fx:
            t = t[:, ::-1]
        y0 = H - tam if fy else 0
        x0 = W - tam if fx else 0
        a = t[..., 3] > 0
        img[y0:y0 + tam, x0:x0 + tam][a] = t[a]
    return img


def cuero(w, h, semilla=5, costura=True, remaches=True, esquinas=False):
    rnd = np.random.default_rng(semilla)
    W, H = w // E, h // E
    t = 0.45 + 0.25 * (_ruido(W, H, 9, rnd) - 0.5) + 0.20 * (rnd.random((H, W)) - 0.5)
    umbral = BAYER[np.arange(H)[:, None] % 4, np.arange(W)[None, :] % 4]
    idx = np.clip(np.floor(t * (len(CUERO) - 1) + umbral - 0.5), 0, len(CUERO) - 1).astype(int)
    img = np.zeros((H, W, 4), dtype=np.uint8)
    img[..., :3] = CUERO[idx]
    img[..., 3] = 255
    img[0, :, :3] = (42, 26, 14); img[-1, :, :3] = (42, 26, 14)
    img[:, 0, :3] = (42, 26, 14); img[:, -1, :3] = (42, 26, 14)
    img[1, 1:-1, :3] = (122, 84, 50)  # brillo superior
    if costura:
        for y in (3, H - 4):
            for x in range(4, W - 4):
                if (x // 2) % 2 == 0:
                    img[y, x, :3] = (201, 165, 106)
    if esquinas:
        img = _esquina_laton(img)
    if remaches:
        for (y, x) in ((H // 2, 3), (H // 2, W - 4)):
            for dy in (-1, 0, 1):
                for dx in (-1, 0, 1):
                    img[y + dy, x + dx, :3] = LATON[3 if (dy, dx) == (-1, -1) else 2 if dy + dx < 1 else 5]
    return _escala(img)


def pestana(w, h, activa=False, semilla=7):
    rnd = np.random.default_rng(semilla)
    W, H = w // E, h // E
    m = np.ones((H, W), dtype=bool)
    for (x, y) in ((0, 0), (1, 0), (0, 1), (W - 1, 0), (W - 2, 0), (W - 1, 1)):
        m[y, x] = False
    d = _distancia_borde(m)
    d[-1, :] = 99 if activa else d[-1, :]  # la activa se funde con la hoja
    t = _papel(W, H, rnd, 0.0 if activa else 0.28, 0, 5, fibras=False)
    img = _cuantiza(t, m, d, 4)
    if activa:
        img[-1, 1:-1, :3] = PAPEL[2]
    return _escala(img)


def adorno(w, color=ADORNO):
    W, H = w // E, 5
    img = np.zeros((H, W, 4), dtype=np.uint8)
    c = W // 2
    for x in range(2, W - 2):
        if abs(x - c) > 4:
            img[2, x] = (*color, 255)
    for dy in range(-2, 3):
        for dx in range(-2, 3):
            if abs(dy) + abs(dx) <= 2:
                img[2 + dy, c + dx] = (*color, 255)
    for x in (c - 6, c + 6):
        img[1, x] = (*color, 255); img[3, x] = (*color, 255)
    return _escala(img)


def sello():
    """Sello de lacre 12x12 lógicos."""
    S = 12
    img = np.zeros((S, S, 4), dtype=np.uint8)
    for y in range(S):
        for x in range(S):
            r = ((x - 5.5) ** 2 + (y - 5.5) ** 2) ** 0.5
            if r <= 5.6:
                col = (154, 42, 28) if r < 4.2 else (110, 26, 16)
                if r < 2.2:
                    col = (184, 64, 44)
                img[y, x] = (*col, 255)
    for (y, x) in ((1, 9), (10, 2), (9, 10)):  # goterones
        img[y, x] = (110, 26, 16, 255)
    img[4:8, 5:7] = (122, 30, 20, 255)  # marca
    img[5:7, 4:8] = (122, 30, 20, 255)
    return _escala(img)


def chincheta():
    """Chincheta de latón 6x6 lógicos para clavar notas."""
    S = 6
    img = np.zeros((S, S, 4), dtype=np.uint8)
    for y in range(S):
        for x in range(S):
            r = ((x - 2.5) ** 2 + (y - 2.5) ** 2) ** 0.5
            if r <= 2.9:
                col = LATON[1] if r > 2.0 else LATON[2]
                if (x, y) in ((1, 1), (2, 1), (1, 2)):
                    col = LATON[3]
                img[y, x] = (*col, 255)
    a = img[..., 3] > 0
    borde = a & ~(np.roll(a, 1, 0) & np.roll(a, -1, 0) & np.roll(a, 1, 1) & np.roll(a, -1, 1))
    img[borde, :3] = LATON[0]
    return _escala(img)


def guardar(img, ruta):
    os.makedirs(os.path.dirname(ruta), exist_ok=True)
    img.save(ruta)
    return ruta


if __name__ == "__main__":
    dst = os.path.join(os.path.dirname(os.path.abspath(__file__)), "_prueba_texturas")
    guardar(hoja(560, 300, 1, filete=6), f"{dst}/hoja.png")
    guardar(papiro(480, 520, 3), f"{dst}/papiro.png")
    guardar(cuero(1120, 56), f"{dst}/cuero.png")
    guardar(pestana(170, 38, True), f"{dst}/pest_a.png")
    guardar(pestana(170, 38, False), f"{dst}/pest_i.png")
    guardar(adorno(300), f"{dst}/adorno.png")
    guardar(sello(), f"{dst}/sello.png")
    print("ok")
