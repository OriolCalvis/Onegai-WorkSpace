"""Oriol (24/10): sacar una capa (pelo, casco, arma) de lo que PixelLab dibuja encima del cuerpo.

PixelLab (edit_image) pinta la pieza sobre el cuerpo de pie de cada familia en cada diagonal. La capa es
la diferencia: lo que el dibujo editado tiene y el cuerpo no (o tiene de otro color), quitando el ruido
de los pixeles sueltos. Luego se pasa a los cinco tonos de un material (la luz de cada pixel decide su
tono), asi la misma pieza sirve en todos los colores.

    extraer(original, editado)            -> RGBA de la pieza sola
    a_material(capa, material)            -> la pieza con los tonos del material
    pegar(capa, ancla_de_pie, ancla)      -> movida con su hueso (la cabeza, la mano)
"""
import numpy as np
from scipy import ndimage


def _lum(c):
    c = np.asarray(c, float)
    return c[..., 0] * 0.299 + c[..., 1] * 0.587 + c[..., 2] * 0.114


def alinear(original, editado, radio=(6, 8)):
    """edit_image redibuja el cuerpo un poco movido: el desplazamiento (dx, dy) del original que mas
    coincide con el editado en la mitad de abajo (las piernas, donde la pieza no llega)."""
    o, e = np.asarray(original), np.asarray(editado)
    ys = np.nonzero((e[..., 3] > 0).any(1))[0]
    corte = ys.min() + (ys.max() - ys.min()) // 2
    mejor = (-1, 0, 0)
    for dy in range(-radio[1], radio[1] + 1):
        for dx in range(-radio[0], radio[0] + 1):
            o2 = np.roll(np.roll(o, dy, 0), dx, 1)
            m = (o2[..., 3] > 0) & (e[..., 3] > 0)
            m[:corte] = False
            if m.sum() < 50:
                continue
            s = (np.abs(o2[m][:, :3].astype(int) - e[m][:, :3].astype(int)).sum(-1) < 20).mean()
            if s > mejor[0]:
                mejor = (s, dx, dy)
    return mejor[1], mejor[2]


def extraer(original, editado, desde_arriba=6, minimo=6):
    """La pieza que PixelLab pinto encima: lo que cae fuera del cuerpo o lo cambia mucho de color, en
    trozos que nacen en lo alto de la figura (pelo, casco). Devuelve (capa, (dx, dy)): la capa esta en
    las coordenadas del editado; (dx, dy) lleva el original hasta el editado."""
    dx, dy = alinear(original, editado)
    o = np.roll(np.roll(np.asarray(original), dy, 0), dx, 1)
    e = np.asarray(editado)
    vo, ve = o[..., 3] > 0, e[..., 3] > 0
    dif = np.abs(o[..., :3].astype(int) - e[..., :3].astype(int)).sum(-1)
    c = (ve & ~ndimage.binary_dilation(vo)) | (ve & vo & (dif > 60))
    abierto = ndimage.binary_opening(c, np.ones((2, 2)))
    c = abierto | (c & ndimage.binary_dilation(abierto))
    lab, n = ndimage.label(c, np.ones((3, 3)))
    top = np.nonzero(ve.any(1))[0].min()
    sigue = np.zeros_like(c)
    for k in range(1, n + 1):
        mk = lab == k
        if mk.sum() >= minimo and np.nonzero(mk.any(1))[0].min() <= top + desde_arriba:
            sigue |= mk
    sigue = ndimage.binary_fill_holes(sigue) & ve
    # el contorno oscuro de la pieza, que a veces se confunde con el del cuerpo
    sigue |= ndimage.binary_dilation(sigue) & ve & (_lum(e[..., :3]) < 90) & ((dif > 30) | ~vo)
    capa = np.zeros(e.shape, np.uint8)
    capa[sigue] = e[sigue]
    return capa, (dx, dy)


def a_material(capa, material):
    """Los pixeles de la capa con los cinco tonos del material: por cuantiles de luz."""
    out = capa.copy()
    m = capa[..., 3] > 0
    if not m.any():
        return out
    L = _lum(capa[..., :3])
    v = L[m]
    q = np.percentile(v, [8, 30, 70, 90])
    tono = np.where(L <= q[0], "O", np.where(L <= q[1], "o", np.where(L <= q[2], "d", np.where(L <= q[3], "s", "H"))))
    # el contorno de fuera siempre es el duro
    borde = m & ~ndimage.binary_erosion(m)
    tono = np.where(borde & (L <= q[1]), "O", tono)
    for t in ("H", "s", "d", "o", "O"):
        sel = m & (tono == t)
        out[sel, :3] = material[t]
    return out


def pegar(capa, a0, a):
    dx, dy = int(round(a[0] - a0[0])), int(round(a[1] - a0[1]))
    out = np.zeros_like(capa)
    alto, ancho = capa.shape[:2]
    ys0, ys1 = max(0, dy), min(alto, alto + dy)
    xs0, xs1 = max(0, dx), min(ancho, ancho + dx)
    out[ys0:ys1, xs0:xs1] = capa[ys0 - dy:ys1 - dy, xs0 - dx:xs1 - dx]
    return out


def pelo_y_barba(base, peinado, lab, esqueleto, frente):
    """Oriol (24/10): el pelo y la barba que PixelLab pinto sobre `base` (el mismo frame sin pelo).

    En la cabeza (y lo que le crece alrededor) es pelo todo lo que no es piel: la piel son los colores
    claros de la cabeza sin pelo, y lo que no cambio (los ojos, el contorno de la cara por debajo de los
    ojos) sigue siendo del cuerpo. El contorno oscuro de arriba de la cabeza es el del pelo. La barba es
    lo que queda por debajo de la nariz en la cara (de frente); el resto, el pelo. Devuelve (pelo, barba)."""
    b, p = np.asarray(base, np.int16), np.asarray(peinado, np.int16)
    alto, ancho = b.shape[:2]
    k = {q["label"]: (q["x"] * ancho, q["y"] * alto) for q in esqueleto}
    vb, vp = b[..., 3] > 0, p[..., 3] > 0
    dif = np.abs(b[..., :3] - p[..., :3]).sum(-1)
    cabeza = lab == "h"
    ys = np.arange(alto)[:, None]
    ojos_y = (k["LEFT EYE"][1] + k["RIGHT EYE"][1]) / 2
    nariz_y = k["NOSE"][1]
    Lb = _lum(b[..., :3])
    piel = np.unique(b[cabeza & vb & (Lb > 110)][:, :3], axis=0)
    if len(piel):
        dp = np.abs(p[..., :3][:, :, None, :] - piel[None, None, :, :]).sum(-1).min(-1)
    else:
        dp = np.full(dif.shape, 999)
    no_piel = dp > 40
    oscuro = _lum(p[..., :3]) < 70
    zona = ndimage.binary_dilation(cabeza, iterations=4) & (ys < k["NECK"][1] + 8)
    igual = vb & (dif < 25)
    pelo_y = vp & zona & no_piel & (~igual | (oscuro & (ys < ojos_y - 1)) | ~cabeza)
    # lo que cae fuera de la cabeza solo si es nuevo y queda cerca de ella: hasta el cuello, y de frente
    # la barba que baja por el centro del pecho (si PixelLab pinto la ropa a la vez, los hombros de la
    # tunica tambien cambian y no son pelo)
    xs = np.arange(ancho)[None, :]
    cuello_y = k["NECK"][1]
    cerca_cab = ys < cuello_y + 1
    if frente:
        cerca_cab = cerca_cab | ((ys < cuello_y + 5) & (np.abs(xs - k["NOSE"][0]) <= 4))
    pelo_y &= cabeza | ((~vb | (dif > 45)) & cerca_cab)
    # y de los colores del pelo (los que salen encima de la cabeza): el cuello de la tunica no
    pal = np.unique(p[pelo_y & cabeza & (ys < nariz_y)][:, :3], axis=0)
    fuera = pelo_y & (~cabeza | (ys > nariz_y + 1))
    if len(pal) and fuera.any():
        dmin = np.abs(p[..., :3][fuera][:, None, :] - pal[None, :, :]).sum(-1).min(1)
        malos = np.zeros_like(fuera)
        malos[fuera] = dmin > 30
        pelo_y &= ~malos
    lab_c, n = ndimage.label(pelo_y, np.ones((3, 3)))
    sigue = np.zeros_like(pelo_y)
    for i in range(1, n + 1):
        m = lab_c == i
        if m.sum() >= 3 and (m & cabeza).any():
            sigue |= m
    barba = np.zeros_like(sigue)
    if frente:
        cara = ys > nariz_y - 0.5
        barba = sigue & cara & ((ys > ojos_y + 1))
        # la patilla une barba y pelo: lo de la barba por encima de la boca y pegado al pelo es pelo
    pelo = sigue & ~barba
    out = []
    for m in (pelo, barba):
        c = np.zeros(p.shape, np.uint8)
        c[m] = p[m]
        out.append(c)
    return out[0], out[1]


def fijar_por_diagonal(talla, frames):
    """Oriol (24/10): una capa de la cabeza (pelo, barba) igual en todos los frames de cada diagonal: la del
    primer frame quieto, llevada con la cara (PixelLab la redibuja un poco distinta en cada frame)."""
    import pixellab_cuerpo as pc
    import pixellab_razas as prz
    rep = pc.reparto()
    out = []
    for f in range(1, talla.n_frames + 1):
        d = rep[f - 1][0]
        f0 = next(i for i, (dd, anim, _k) in enumerate(rep, 1) if dd == d and anim == pc.RESPIRAR)
        a0 = prz.anclas(talla.esqueletos[f0], talla.ancho, talla.alto)["cabeza"]
        a = prz.anclas(talla.esqueletos[f], talla.ancho, talla.alto)["cabeza"]
        out.append(prz._mueve(frames[f0 - 1], a, a0))
    return out
