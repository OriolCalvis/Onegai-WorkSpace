"""Extraer objetos de una lamina de autoria y reducirlos a pixel-art runtime.

Lo usa tools/gen_props_p0.py. Hay dos clases de lamina:

* Con transparencia real (assets/textures/editor_*.png): el alpha manda y
  cada objeto vive en su celda de una rejilla regular.
* De fondo oscuro liso (assets/generated/authoring/**): el fondo se rellena
  desde el borde. Entra lo que se parece al fondo y lo que se oscurece poco
  a poco (la sombra de suelo del dibujo: el motor ya pinta su sombra blob),
  pero nunca salta un contorno, que es un cambio brusco. Asi las caras en
  sombra de un mueble, del mismo marron oscuro que el fondo, siguen siendo
  mueble. Los huecos cerrados de fondo liso (entre los barrotes de una
  silla) tambien pasan a vacio.

La reduccion promedia por area con alpha premultiplicado, umbraliza la
cobertura al 50 %, realza un poco (la reduccion apaga el contraste),
cuantiza a una paleta corta sin tramado y oscurece el borde de la silueta
para que se lea sobre cualquier suelo. Sin suavizado en el resultado.
"""
from collections import deque

import numpy as np
from PIL import Image, ImageFilter


def _lum(a):
    return 0.299 * a[..., 0] + 0.587 * a[..., 1] + 0.114 * a[..., 2]


try:  # scipy acelera el etiquetado; sin el, el mismo resultado en Python puro
    from scipy import ndimage as _ndi
except ImportError:  # pragma: no cover - depende de la maquina
    _ndi = None


def _etiquetar(mask, ocho=True):
    if _ndi is not None:
        estructura = np.ones((3, 3), bool) if ocho else None
        lab, n = _ndi.label(mask, structure=estructura)
        return lab.astype(np.int32), n
    h, w = mask.shape
    lab = np.zeros((h, w), np.int32)
    vecinos = ([(dy, dx) for dy in (-1, 0, 1) for dx in (-1, 0, 1) if dy or dx] if ocho
               else [(1, 0), (-1, 0), (0, 1), (0, -1)])
    n = 0
    ys, xs = np.nonzero(mask)
    for y0, x0 in zip(ys, xs):
        if lab[y0, x0]:
            continue
        n += 1
        lab[y0, x0] = n
        q = deque([(y0, x0)])
        while q:
            y, x = q.popleft()
            for dy, dx in vecinos:
                ny, nx = y + dy, x + dx
                if 0 <= ny < h and 0 <= nx < w and mask[ny, nx] and not lab[ny, nx]:
                    lab[ny, nx] = n
                    q.append((ny, nx))
    return lab, n


def componentes(mask, min_area=150, ocho=True):
    """Etiquetas (0 = fondo) y lista de (etiqueta, area, caja) de las grandes."""
    lab, n = _etiquetar(mask, ocho)
    if n == 0:
        return lab, []
    areas = np.bincount(lab.ravel(), minlength=n + 1)
    info = []
    for k in np.nonzero(areas >= min_area)[0]:
        if k == 0:
            continue
        ys, xs = np.nonzero(lab == k)
        info.append((int(k), int(areas[k]),
                     (int(xs.min()), int(ys.min()), int(xs.max()) + 1, int(ys.max()) + 1)))
    return lab, info


def mascara_fondo_oscuro(im, umbral=14, paso=7, hueco_min=150, cierre=11):
    """Primer plano (bool HxW) de una lamina de fondo oscuro liso.

    Es fondo lo que esta conectado con el borde de la lamina a traves de
    pixeles libres: parecidos al fondo, o mas oscuros y sin color (la
    sombra de suelo) SIEMPRE QUE la zona sea suave. Un contorno es un salto
    brusco de color: ahi el gradiente local supera 'paso' y el relleno se
    para, aunque el contorno sea tan oscuro como la sombra.
    """
    rgb = np.array(im.convert("RGB")).astype(np.int32)
    h, w = rgb.shape[:2]
    borde = np.concatenate([rgb[0], rgb[-1], rgb[:, 0], rgb[:, -1]])
    fondo = np.median(borde, axis=0)
    d = np.sqrt(((rgb - fondo) ** 2).sum(axis=2))
    sat = rgb.max(axis=2) - rgb.min(axis=2)
    grad = np.zeros((h, w), np.int32)
    pad = np.pad(rgb, ((1, 1), (1, 1), (0, 0)), mode="edge")
    for dy, dx in ((0, 1), (2, 1), (1, 0), (1, 2)):
        vec = pad[dy:dy + h, dx:dx + w]
        grad = np.maximum(grad, np.abs(vec - rgb).max(axis=2))
    lum = _lum(rgb)
    # la sombra de suelo es algo mas oscura que el fondo, no negra: el negro
    # casi puro es la cara en sombra del propio objeto (una caja, un cofre)
    sombra_suave = (lum < _lum(fondo) - 2) & (lum > 7) & (sat < 11) & (grad < paso)
    libre = (d < umbral) | sombra_suave
    lab, _n = _etiquetar(libre, ocho=False)
    toca = np.unique(np.concatenate([lab[0], lab[-1], lab[:, 0], lab[:, -1]]))
    toca = toca[toca > 0]
    fg = ~np.isin(lab, toca)
    if cierre:
        img = Image.fromarray(np.uint8(fg * 255))
        img = img.filter(ImageFilter.MaxFilter(cierre)).filter(ImageFilter.MinFilter(cierre))
        fg = np.array(img) > 0
    lab, info = componentes(fg & (d < umbral), min_area=hueco_min)
    for n, _area, _caja in info:
        region = lab == n
        if rgb[region].std(axis=0).max() < 4:
            fg[region] = False
    return fg


class Separador:
    """Objetos de una lamina: se etiqueta una vez y se consulta por semilla."""

    def __init__(self, fg, fusion=9, min_area=2000):
        self.fg = fg
        unido = np.array(Image.fromarray(np.uint8(fg * 255)).filter(ImageFilter.MaxFilter(fusion))) > 0
        self.lab, self.info = componentes(unido, min_area=min_area)

    def objeto(self, semilla):
        sx_, sy_ = semilla
        k = int(self.lab[sy_, sx_])
        if k == 0 or all(k != n for n, _a, _c in self.info):
            cand = [(((c[0] + c[2]) / 2 - sx_) ** 2 + ((c[1] + c[3]) / 2 - sy_) ** 2, n)
                    for n, _a, c in self.info]
            if not cand:
                raise ValueError(f"ningun objeto cerca de la semilla {semilla}")
            k = min(cand)[1]
        m = (self.lab == k) & self.fg
        ys, xs = np.nonzero(m)
        return m, (int(xs.min()), int(ys.min()), int(xs.max()) + 1, int(ys.max()) + 1)


def objeto_en_celda(im, col, fila, columnas, filas):
    """Mascara del objeto de la celda (col, fila) de una lamina con alpha."""
    a = np.array(im.convert("RGBA"))[..., 3]
    h, w = a.shape
    cw, ch = w / columnas, h / filas
    x0, y0 = int(round(col * cw)), int(round(fila * ch))
    x1, y1 = int(round((col + 1) * cw)), int(round((fila + 1) * ch))
    m = np.zeros((h, w), bool)
    m[y0:y1, x0:x1] = a[y0:y1, x0:x1] >= 128
    ys, xs = np.nonzero(m)
    return m, (int(xs.min()), int(ys.min()), int(xs.max()) + 1, int(ys.max()) + 1)


def reducir(im, mask, caja, alto, ancho_max=62, colores=18, contorno=0.42,
            saturacion=1.12, gamma=0.92, nitidez=0.6, tapar=8):
    """Recorta 'caja' y la reduce a 'alto' px (o menos, si no cabe en ancho).

    'tapar' rellena los agujeros cerrados de hasta ese tamano: son rendijas
    oscuras del dibujo que el relleno del fondo se comio. Un objeto con
    huecos de verdad (una valla, las ramas de un arbol) pasa tapar=0.
    """
    x0, y0, x1, y1 = caja
    rgb = np.array(im.convert("RGB")).astype(np.float64)[y0:y1, x0:x1]
    m = mask[y0:y1, x0:x1].astype(np.float64)
    h, w = m.shape
    s = alto / h
    if w * s > ancho_max:
        s = ancho_max / w
    tw, th = max(1, int(round(w * s))), max(1, int(round(h * s)))
    prem = Image.fromarray(np.uint8(np.clip(rgb * m[..., None], 0, 255)))
    cov = Image.fromarray(np.uint8(m * 255))
    prem_s = np.array(prem.resize((tw, th), Image.BOX)).astype(np.float64)
    cov_s = np.array(cov.resize((tw, th), Image.BOX)).astype(np.float64) / 255.0
    col = np.where(cov_s[..., None] > 0, prem_s / np.maximum(cov_s[..., None], 1e-6), 0.0)
    col = np.clip(col, 0, 255)
    opaco = cov_s >= 0.5
    if nitidez:
        # desenfoque normalizado DENTRO de la silueta: si el vacio (negro)
        # entrara en la media, el realce blanquearia el borde del objeto
        a = opaco.astype(np.float64)
        num = _desenfocar(col * a[..., None])
        den = _desenfocar(np.repeat(a[..., None], 3, axis=2))
        media = np.where(den > 1e-6, num / np.maximum(den, 1e-6), col)
        col = np.clip(col + nitidez * (col - media), 0, 255)
    col = realzar(col, saturacion, gamma)
    rgba = np.zeros((th, tw, 4), np.uint8)
    rgba[..., :3] = np.uint8(np.round(col))
    rgba[..., 3] = np.where(opaco, 255, 0)
    rgba = quitar_motas(rgba)
    if tapar:
        rgba = tapar_huecos(rgba, tapar)
    rgba = cuantizar(rgba, colores)
    if contorno:
        rgba = oscurecer_borde(rgba, contorno)
    return recortar(rgba)


def _desenfocar(arr, radio=0.8):
    """Desenfoque gaussiano de una matriz float HxWx3 (via PIL, canal a canal)."""
    out = np.zeros_like(arr)
    escala = max(1.0, float(arr.max()))
    for k in range(arr.shape[2]):
        canal = Image.fromarray(np.uint8(np.clip(arr[..., k] / escala * 255, 0, 255)))
        suave = np.array(canal.filter(ImageFilter.GaussianBlur(radio))).astype(np.float64)
        out[..., k] = suave / 255 * escala
    return out


def quitar_motas(rgba, minimo=3):
    """Quita islas de menos de 'minimo' px sueltas alrededor del dibujo."""
    lab, info = componentes(rgba[..., 3] > 0, min_area=0)
    if len(info) <= 1:
        return rgba
    mayor = max(a for _n, a, _c in info)
    out = rgba.copy()
    for n, area, _c in info:
        if area < minimo and area < mayor:
            out[lab == n] = 0
    return out


def tapar_huecos(rgba, maximo):
    """Rellena huecos cerrados pequenos con el color mas oscuro de su borde."""
    vacio = rgba[..., 3] == 0
    lab, info = componentes(vacio, min_area=0, ocho=False)
    h, w = vacio.shape
    borde = set(np.unique(np.concatenate([lab[0], lab[-1], lab[:, 0], lab[:, -1]])).tolist())
    out = rgba.copy()
    for n, area, _c in info:
        if n in borde or area > maximo:
            continue
        ys, xs = np.nonzero(lab == n)
        vecinos = []
        for y, x in zip(ys, xs):
            for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                ny, nx = y + dy, x + dx
                if 0 <= ny < h and 0 <= nx < w and rgba[ny, nx, 3]:
                    vecinos.append(rgba[ny, nx, :3])
        if vecinos:
            v = np.array(vecinos)
            color = v[np.argmin(_lum(v.astype(np.float64)))]
            out[ys, xs, :3] = color
            out[ys, xs, 3] = 255
    return out


def realzar(col, saturacion, gamma):
    """Sube algo la saturacion y aclara las sombras (las laminas son oscuras)."""
    if saturacion == 1 and gamma == 1:
        return col
    gris = _lum(col)[..., None]
    col = gris + (col - gris) * saturacion
    col = np.clip(col, 0, 255)
    return 255.0 * (col / 255.0) ** gamma


def cuantizar(rgba, colores):
    op = rgba[..., 3] > 0
    if op.sum() == 0:
        return rgba
    pix = rgba[op][:, :3]
    tira = Image.fromarray(pix.reshape(1, -1, 3).astype(np.uint8), "RGB")
    n = min(colores, len(np.unique(pix, axis=0)))
    q = tira.quantize(colors=n, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    pal = np.array(q.getpalette()[:n * 3]).reshape(-1, 3)
    out = rgba.copy()
    out[op, :3] = pal[np.array(q).reshape(-1)]
    return out


def oscurecer_borde(rgba, k):
    """Contorno interior: el pixel opaco que toca el vacio se oscurece."""
    a = rgba[..., 3] > 0
    vecino_vacio = np.zeros_like(a)
    pad = np.pad(a, 1, constant_values=False)
    for dy, dx in ((0, 1), (2, 1), (1, 0), (1, 2)):
        vecino_vacio |= ~pad[dy:dy + a.shape[0], dx:dx + a.shape[1]]
    borde = a & vecino_vacio
    out = rgba.copy()
    out[borde, :3] = (rgba[borde, :3].astype(np.float64) * (1 - k)).astype(np.uint8)
    return out


def recortar(rgba):
    ys, xs = np.nonzero(rgba[..., 3])
    if len(ys) == 0:
        return rgba
    return rgba[ys.min():ys.max() + 1, xs.min():xs.max() + 1]


def tenir(rgba, tono_destino=None, gris=False, hue_min=330, hue_max=25, sat_min=0.35):
    """Cambia el tono de los pixeles saturados de un rango (liquido de pocion...)."""
    out = rgba.copy()
    op = rgba[..., 3] > 0
    hsv = np.array(Image.fromarray(rgba[..., :3]).convert("HSV")).astype(np.float64)
    hue = hsv[..., 0] * 360 / 255
    sat = hsv[..., 1] / 255
    if hue_min > hue_max:
        rango = (hue >= hue_min) | (hue <= hue_max)
    else:
        rango = (hue >= hue_min) & (hue <= hue_max)
    sel = op & rango & (sat >= sat_min)
    if gris:
        hsv[sel, 1] *= 0.12
    else:
        hsv[sel, 0] = (tono_destino % 360) * 255 / 360
    rgb = np.array(Image.fromarray(np.uint8(np.clip(hsv, 0, 255)), "HSV").convert("RGB"))
    out[sel, :3] = rgb[sel]
    return out


def colocar(rgba, celda=64, fila_suelo=61):
    """Pone el sprite en una celda: centrado en x y con la base en fila_suelo."""
    c = np.zeros((celda, celda, 4), np.uint8)
    h, w = rgba.shape[:2]
    if h > fila_suelo + 1 or w > celda:
        raise ValueError(f"sprite de {w}x{h} no cabe en la celda de {celda}")
    x0 = (celda - w) // 2
    y0 = fila_suelo + 1 - h
    c[y0:y0 + h, x0:x0 + w] = rgba
    return c
