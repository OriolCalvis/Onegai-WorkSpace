"""Oriol (24/10): la ropa dibujada por PixelLab sobre el cuerpo de la familia, separada en capas.

PixelLab viste el cuerpo (edit_image en un frame de cada diagonal y transfer_outfit para llevarlo al
andar y al respirar, que mantiene la pose de cada frame). La ropa es lo que cambia respecto al cuerpo
desnudo, de los hombros para abajo (la cabeza siempre es la del cuerpo o la de la raza). Se reparte
en las ranuras de siempre por el mapa de partes del cuerpo: torso (tronco, brazos y lo que cuelga de
la cintura), piernas y pies. Lo que sale fuera del cuerpo (los faldones, el vuelo de las mangas) va a
la parte del cuerpo mas cercana.

    capas_ropa(talla, vestidos)  -> {"torso": frames, "piernas": frames, "pies": frames}
    teñir(frames, de, a)         -> la pieza con un grupo de color cambiado de material
"""
import numpy as np
from scipy import ndimage

RANURA = {"t": "torso", "n": "torso", "a": "torso", "A": "torso", "m": "torso", "M": "torso",
          "p": "piernas", "l": "piernas", "L": "piernas", "f": "pies", "F": "pies"}
UMBRAL = 45  # diferencia de color (suma RGB) para que un pixel cuente como ropa


def _lum(c):
    c = np.asarray(c, float)
    return c[..., 0] * 0.299 + c[..., 1] * 0.587 + c[..., 2] * 0.114


def mascara_ropa(original, vestido, lab, cuello_y, nariz_y=None):
    o, e = np.asarray(original, np.int16), np.asarray(vestido, np.int16)
    vo, ve = o[..., 3] > 0, e[..., 3] > 0
    dif = np.abs(o[..., :3] - e[..., :3]).sum(-1)
    ys = np.arange(o.shape[0])[:, None]
    # lo que no es de ningun color de la piel del cuerpo desnudo tambien es ropa (una bota parda donde
    # el cuerpo tenia sombra parda: debajo hay otra piel en otras razas)
    piel = np.unique(o[vo & (lab != "h")][:, :3], axis=0)
    lejos = np.ones(dif.shape, bool)
    if len(piel):
        cand = e[..., :3][ve]
        dmin = np.abs(cand[:, None, :] - piel[None, :, :]).sum(-1).min(1)
        lejos = np.zeros(dif.shape, bool)
        lejos[ve] = dmin > 30
    # Oriol (24/10, «no haces bien los hombros»): la ropa no se corta en la linea del cuello: en diagonal el
    # hombro de atras sube por encima. Lo unico que no es ropa es la cabeza (y lo que esta por encima de ella).
    cabeza = ndimage.binary_dilation(lab == "h", iterations=1) & (ys < cuello_y + 1)
    # el cuello de la tunica sube por encima de la barbilla del cuerpo desnudo: en la parte de abajo de la
    # cabeza, lo que no es del color de la piel y cambio es ropa
    if nariz_y is not None:
        cabeza &= ~((ys > nariz_y + 2) & lejos & (dif > UMBRAL))
    hs = np.nonzero((lab == "h").any(1))[0]
    techo = int(hs.min()) if len(hs) else 0
    ropa = ve & (~vo | (dif > UMBRAL) | lejos) & ~cabeza & (ys > techo)
    # sin pixeles sueltos; los huecos de un pixel dentro de la ropa, si
    ropa = ndimage.binary_opening(ropa, np.ones((2, 1))) | ndimage.binary_opening(ropa, np.ones((1, 2)))
    ropa |= ndimage.binary_closing(ropa) & ve & ~cabeza & (ys > techo)
    return ropa


def partes(ropa, lab):
    """La ranura de cada pixel de ropa: la de su parte del cuerpo, o la de la mas cercana."""
    con = np.isin(lab, list(RANURA))
    _d, (iy, ix) = ndimage.distance_transform_edt(~con, return_indices=True)
    cerca = lab[iy, ix]
    out = {}
    for r in ("torso", "piernas", "pies"):
        letras = [k for k, v in RANURA.items() if v == r]
        out[r] = ropa & np.isin(cerca, letras)
    return out


def _tono(px):
    """Matiz (grados) y saturacion de cada pixel."""
    rgb = px[..., :3].astype(float) / 255
    mx, mn = rgb.max(-1), rgb.min(-1)
    c = mx - mn
    sat = np.where(mx > 0, c / np.maximum(mx, 1e-6), 0)
    r, g, b = rgb[..., 0], rgb[..., 1], rgb[..., 2]
    cc = np.maximum(c, 1e-6)
    h = np.where(mx == r, ((g - b) / cc) % 6, np.where(mx == g, (b - r) / cc + 2, (r - g) / cc + 4)) * 60
    return np.where(c > 0, h, 0), sat


def _dif_matiz(a, b):
    d = np.abs(a - b) % 360
    return np.minimum(d, 360 - d)


def faldon_al_torso(trozos, px, lab):
    """El faldon de la tunica (lo que cae por debajo del cinturon, sobre la pelvis y los muslos) es del
    torso aunque caiga en las piernas: los pixeles de las piernas con el color del torso y no el de las
    piernas pasan al torso."""
    h, s = _tono(px)
    color = s > 0.18
    def dominante(m):
        v = h[m & color]
        return float(np.median(v)) if len(v) > 4 else None
    ht, hp = dominante(trozos["torso"] & np.isin(lab, ["t", "a", "A"])), dominante(trozos["piernas"] & np.isin(lab, ["l", "L"]))
    if ht is None:
        return trozos
    pasa = trozos["piernas"] & color & (_dif_matiz(h, ht) < 30)
    if hp is not None:
        pasa &= _dif_matiz(h, hp) > _dif_matiz(h, ht)
    # solo lo que esta pegado al torso (el faldon), no una mancha suelta en la bota
    lab_t, n = ndimage.label(pasa | trozos["torso"], np.ones((3, 3)))
    unidas = set(np.unique(lab_t[trozos["torso"]])) - {0}
    pasa &= np.isin(lab_t, list(unidas))
    trozos["torso"] = trozos["torso"] | pasa
    trozos["piernas"] = trozos["piernas"] & ~pasa
    return trozos


def capas_ropa(talla, vestidos, quitar=None):
    """vestidos: los frames de PixelLab, en el orden de la hoja de la talla (40). quitar: por frame, una
    mascara de lo que no es ropa aunque cambie (el pelo y la barba, si PixelLab los pinto a la vez)."""
    salida = {"torso": [], "piernas": [], "pies": [], "piel": []}
    for f in range(1, talla.n_frames + 1):
        e = np.asarray(vestidos[f - 1])
        k = {p["label"]: (p["x"] * talla.ancho, p["y"] * talla.alto) for p in talla.esqueletos[f]}
        cuello_y = int(round(k["NECK"][1])) - 1
        lab = talla.labs[f]
        ropa = mascara_ropa(talla.frames[f - 1], e, lab, cuello_y, k["NOSE"][1])
        if quitar is not None:
            ropa &= ~quitar[f - 1]
        # Oriol (24/10, «el color de las manos»): lo que PixelLab dibujo con el color de la piel (una mano
        # que se movio un pixel, un trozo de brazo) no es ropa: va aparte, para pintarlo con la piel de
        # cada raza. Solo los tonos claros de la piel (los contornos oscuros se parecen a los de la ropa).
        o = talla.frames[f - 1]
        claros = o[(o[..., 3] > 0) & (lab != "h")][:, :3].astype(int)
        if len(claros):
            # los claros de esa piel (el 60 % mas claro): en un orco verde oscuro tambien
            claros = np.unique(claros[_lum(claros) > np.percentile(_lum(claros), 40)], axis=0)
        piel = np.zeros(ropa.shape, bool)
        if len(claros) and ropa.any():
            dmin = np.abs(e[..., :3][ropa].astype(int)[:, None, :] - claros[None, :, :]).sum(-1).min(1)
            piel[ropa] = dmin <= 24
            # y tan claro como esa piel (una barba parda sobre el pecho se parece a la sombra de la piel)
            piel &= _lum(e[..., :3].astype(float)) >= _lum(claros.astype(float)).min() - 4
            # solo en los brazos y las manos (la parte del cuerpo mas cercana): las luces de las botas o del
            # pantalon se parecen a la piel clara y no lo son
            con = lab != ""
            _d, (iy, ix) = ndimage.distance_transform_edt(~con, return_indices=True)
            piel &= np.isin(lab[iy, ix], ["a", "A", "m", "M"])
            # y el contorno pegado a esa piel por fuera de la ropa (el borde de la mano)
            borde = ndimage.binary_dilation(piel) & ropa & (_lum(e[..., :3]) < 90) & np.isin(lab, ["m", "M", ""])
            piel |= borde & ~ndimage.binary_dilation(ropa & ~piel & (_lum(e[..., :3]) >= 90), iterations=1)
            # Oriol (24/10, «los grandes aun salen mal»): con una piel de color que ninguna tela tiene (el
            # verde del orco), todo lo de ese matiz es piel, tambien las sombras y el contorno de la mano y
            # el cuello; si no, quedaban pixeles verdes en las manos del draconido o del oni
            hp, sp, _vp = _hsv(claros[None, :, :].astype(np.uint8))
            hm = float(np.angle(np.exp(2j * np.pi * hp[sp > 0.15]).mean()) / (2 * np.pi) % 1) if (sp > 0.15).any() else None
            if hm is not None and min(abs(hm - 30 / 360), 1 - abs(hm - 30 / 360)) * 360 > 45:
                h, s_, v = _hsv(e[..., :3])
                dh = np.minimum(np.abs(h - hm), 1 - np.abs(h - hm)) * 360
                verde = (e[..., 3] > 0) & (dh < 30) & (s_ > 0.2) & (v > 0.12)
                # en las piernas no hay piel a la vista: ahi es un borron de PixelLab sobre el pantalon (a
                # veces el verde del cuerpo sin tocar), y se pinta con el color de la tela mas cercana
                pierna = np.isin(lab[iy, ix], ["l", "L", "f", "F", "p"])
                piel |= verde & ropa & ~pierna
                borron = verde & pierna & ndimage.binary_dilation(ropa & ~verde, iterations=2)
                tela = ropa & ~verde & ~piel
                if borron.any() and tela.any():
                    _d2, (ty, tx) = ndimage.distance_transform_edt(~tela, return_indices=True)
                    e = e.copy()
                    e[borron] = e[ty[borron], tx[borron]]
                    ropa |= borron
            ropa &= ~piel
        capa_piel = np.zeros_like(e)
        capa_piel[piel] = e[piel]
        salida["piel"].append(capa_piel)
        trozos = faldon_al_torso(partes(ropa, lab), e, lab)
        relleno = None
        if quitar is not None:
            # lo que tapaba la barba (si PixelLab pinto pelo y ropa a la vez) era tunica: se rellena con el
            # color de la ropa mas cercana, para las razas sin barba o con otra barba
            ys = np.arange(e.shape[0])[:, None]
            hueco = quitar[f - 1] & (e[..., 3] > 0) & (ys >= k["NOSE"][1] + 4)
            hueco &= (lab != "") | ndimage.binary_dilation(lab != "", iterations=1)
            if hueco.any() and trozos["torso"].any():
                _d, (iy, ix) = ndimage.distance_transform_edt(~trozos["torso"], return_indices=True)
                relleno = (hueco, iy, ix)
        for r, m in trozos.items():
            capa = np.zeros_like(e)
            capa[m] = e[m]
            if r == "torso" and relleno is not None:
                hueco, iy, ix = relleno
                capa[hueco] = e[iy[hueco], ix[hueco]]
            salida[r].append(capa)
    return salida


def _hsv(rgb):
    rgb = rgb.astype(float) / 255
    mx, mn = rgb.max(-1), rgb.min(-1)
    c = mx - mn
    v = mx
    s_ = np.where(mx > 0, c / np.maximum(mx, 1e-6), 0)
    r, g, b = rgb[..., 0], rgb[..., 1], rgb[..., 2]
    cc = np.maximum(c, 1e-6)
    h = np.where(mx == r, ((g - b) / cc) % 6, np.where(mx == g, (b - r) / cc + 2, (r - g) / cc + 4)) / 6
    return np.where(c > 0, h, 0), s_, v


def _rgb(h, s_, v):
    i = np.floor(h * 6).astype(int) % 6
    f = h * 6 - np.floor(h * 6)
    p, q, t = v * (1 - s_), v * (1 - f * s_), v * (1 - (1 - f) * s_)
    r = np.choose(i, [v, q, p, p, t, v])
    g = np.choose(i, [t, v, v, q, p, p])
    b = np.choose(i, [p, p, t, v, v, q])
    return (np.stack([r, g, b], -1) * 255).round().clip(0, 255).astype(np.uint8)


def color_dominante(frames):
    """(matiz, saturacion, luz) del color que mas pixeles ocupa en la pieza (sin grises): la tela, no el cinto."""
    hs, ss, vs = [], [], []
    for px in frames:
        h, s_, v = _hsv(px[..., :3])
        m = (px[..., 3] > 0) & (s_ > 0.12)
        hs.append(h[m]); ss.append(s_[m]); vs.append(v[m])
    h, s_, v = np.concatenate(hs), np.concatenate(ss), np.concatenate(vs)
    cubo = np.bincount((h * 36).astype(int) % 36, minlength=36)
    c = (np.argmax(cubo) + 0.5) / 36
    dh = np.minimum(np.abs(h - c), 1 - np.abs(h - c))
    cerca = dh < 20 / 360
    ang = np.angle(np.exp(2j * np.pi * h[cerca]).mean()) / (2 * np.pi) % 1
    return float(ang), float(np.median(s_[cerca])), float(np.median(v[cerca]))


def teñir(frames, de, a, tolerancia=40):
    """Oriol (24/10): cambia el color de la pieza sin perder el sombreado de PixelLab: los pixeles cuyo
    matiz se parece al de `de` (RGB) toman el matiz del material `a`, y su saturacion y su luz se escalan
    de la base de `de` a la base del material (asi un pixel de luz sigue siendo de luz)."""
    if de is None:
        h0, s0, v0 = color_dominante(frames)
    else:
        h0, s0, v0 = [float(np.ravel(x)[0]) for x in _hsv(np.array([[de]], np.uint8))]
    base = np.array([[a["s"]]], np.uint8)
    h1, s1, v1 = [float(np.ravel(x)[0]) for x in _hsv(base)]
    out = []
    for px in frames:
        px = px.copy()
        h, s_, v = _hsv(px[..., :3])
        dh = np.minimum(np.abs(h - h0), 1 - np.abs(h - h0)) * 360
        sel = (px[..., 3] > 0) & (dh < tolerancia) & (s_ > 0.12)
        if sel.any():
            ns = np.clip(s_ * (s1 / max(s0, 1e-3)), 0, 1)
            nv = np.clip(v * (v1 / max(v0, 1e-3)), 0, 1)
            nuevo = _rgb(np.full(h.shape, h1), ns, nv)
            px[sel, :3] = nuevo[sel]
        out.append(px)
    return out
