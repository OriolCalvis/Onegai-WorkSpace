"""Oriol (24/10): lo de la cabeza (sombreros, capuchas, cascos) en las cuatro diagonales, mientras no
haya los de PixelLab.

Las piezas de cabeza son plantillas dibujadas contra la cabeza del cuerpo de siempre en cada vista
(actores_cascos, actores_capas.pintar_plantilla). Aqui se pinta la plantilla de la vista mas parecida
(VISTA: la de frente o la de espaldas, segun la diagonal) y se lleva de la cabeza de siempre a la de PixelLab
del frame: lo alto de la cabeza y su centro.

    capa_cabeza_pl(talla, ident, materiales)  -> frames (como actores_cascos.capa_cabeza)
"""
import numpy as np
from scipy import ndimage

import actores_cascos as cascos
from actores_capas import X0_PLANTILLA, pintar_plantilla
import pixellab_armas

# La vista de siempre que se pinta en cada diagonal (la de frente o la de espaldas: probadas las dos, la
# de perfil desplaza las capuchas y las alas de los sombreros), y el frame quieto de esa vista en el
# cuerpo de siempre
VISTA = {"sureste": "abajo", "suroeste": "abajo", "noreste": "arriba", "noroeste": "arriba"}
QUIETO = {"arriba": 2, "derecha": 5, "abajo": 8, "izquierda": 11}


def _cabeza(lab):
    ys, xs = np.nonzero(lab == "h")
    if not len(ys):
        return None
    return int(ys.min()), (xs.min() + xs.max()) / 2


def _mueve(m, dx, dy):
    out = np.zeros_like(m)
    alto, ancho = m.shape[:2]
    ys0, ys1 = max(0, dy), min(alto, alto + dy)
    xs0, xs1 = max(0, dx), min(ancho, ancho + dx)
    out[ys0:ys1, xs0:xs1] = m[ys0 - dy:ys1 - dy, xs0 - dx:xs1 - dx]
    return out


CAPUCHAS = ("capucha",)  # ids que llevan «capucha»: se dibujan desde la cabeza de PixelLab (capucha_pl)


def es_capucha(ident):
    return "capucha" in ident


def capa_cabeza_pl(talla, ident, materiales, vista=None):
    pieza = cascos.CABEZA[ident]
    if es_capucha(ident):
        return capucha_pl(talla, materiales[pieza["m1"]])
    plantillas, m1, m2 = pieza["plantillas"], pieza["m1"], pieza["m2"]
    # las plantillas de cabeza estan dibujadas en las coordenadas del cuerpo clasico MEDIO (actores_cascos las
    # baja con dy_cabeza para las otras tallas): la cabeza de referencia es siempre la del medio. (Antes se
    # cogia la clasica de la misma talla y en el mediano los sombreros flotaban 5-7 px sobre la cabeza.)
    clasica = pixellab_armas.clasicas()["medio"]
    pintadas = []
    for f in range(1, talla.n_frames + 1):
        d = talla.diagonal(f)
        v = (vista or VISTA)[d]
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        de, a = _cabeza(clasica.labs[QUIETO[v]]), _cabeza(talla.labs[f])
        if de is None or a is None:
            pintadas.append((capa, 0, 0))
            continue
        dy, dx = a[0] - de[0], int(round(a[1] - de[1]))
        ojos = _mueve(np.asarray(talla.ojos[f]), -dx, -dy)
        cuerpo = _mueve(np.asarray(talla.alfa[f]), -dx, -dy)
        for pl, m2_propio in plantillas:
            pintar_plantilla(capa, pl, v, 0, materiales[m1],
                             materiales[m2_propio or m2] if (m2_propio or m2) else None,
                             ojos=ojos, cuerpo=cuerpo, x0=pl.get("_x0", X0_PLANTILLA))
        pintadas.append((capa, dx, dy))
    # PL-ROPA-01 (Oriol, 25/10, «los altos pierden la punta»): si la pieza se saliera por arriba de la celda
    # en algun frame (aarakocras y nagas, con la cabeza en la fila 11-13 y un sombrero de mago de 15 filas),
    # va calada: baja lo justo (2-4 px), lo mismo en todos los frames, para que quepa entera en vez de
    # recortarse. A los lados, igual.
    calado, izq, der = 0, 0, 0
    for capa, dx, dy in pintadas:
        ys, xs = np.nonzero(capa[..., 3])
        if len(ys):
            calado = max(calado, 1 - (int(ys.min()) + dy))
            izq = max(izq, 1 - (int(xs.min()) + dx))
            der = max(der, (int(xs.max()) + dx) - (talla.ancho - 2))
    return [_mueve(capa, dx + izq - der, dy + calado) for capa, dx, dy in pintadas]


def capucha_pl(talla, m):
    """Oriol (25/10, «la capucha un 0/10»): la capucha dibujada desde la silueta de la cabeza de cada frame, en
    vez de la plantilla de frente de siempre. La cabeza engordada 1 px (2 por arriba), una punta hacia atras,
    la esclavina sobre los hombros (del cuello, 3 filas, el ancho del tronco), y de frente la abertura de la
    cara (la mitad delantera de la cabeza, de las cejas a la barbilla) con sombra alrededor; de espaldas, la
    costura. El material m da los cinco tonos. Tapa el pelo (oculta_pelo en modo PixelLab)."""
    frames = []
    for f in range(1, talla.n_frames + 1):
        lab = talla.labs[f]
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        cab = lab == "h"
        if not cab.any():
            frames.append(capa)
            continue
        d = talla.diagonal(f)
        frente = d.startswith("sur")
        hacia_der = d.endswith("este")
        lado = 1 if hacia_der else -1
        ys, xs = np.nonzero(cab)
        top, bottom = ys.min(), ys.max()
        cx = (xs.min() + xs.max()) / 2
        k = {p["label"]: (p["x"] * talla.ancho, p["y"] * talla.alto) for p in talla.esqueletos[f]}
        cuello_y = int(round(k["NECK"][1] - 0.5))  # la fila del cuello (n)
        yy, xx = np.mgrid[0:talla.alto, 0:talla.ancho]
        # 1) el cuerpo de la capucha: la cabeza engordada 1 px por arriba y por los lados (no por abajo)
        hood = ndimage.binary_dilation(cab, structure=np.ones((3, 3)))
        hood[:-1] |= hood[1:]  # y 2 por arriba: la cabeza de la raza (la del frame quieto, pegada) puede ser 1 px mas alta
        hood &= yy <= bottom
        # la punta, hacia atras y arriba
        px_atras = int(round(cx - lado * 2))
        for i in range(1, 5):
            y = top - i
            x = px_atras - lado * ((i + 1) // 2)
            if 0 <= y < talla.alto and 0 <= x < talla.ancho:
                hood[y, x] = True
                if i < 4 and 0 <= x + lado < talla.ancho:
                    hood[y, x + lado] = True
                if i < 2 and 0 <= x + 2 * lado < talla.ancho:
                    hood[y, x + 2 * lado] = True
        # 2) la esclavina: del cuello hasta 2 filas mas abajo, el ancho del tronco + 1 a cada lado
        for y in range(cuello_y, min(talla.alto, cuello_y + 3)):
            cols = np.nonzero(np.isin(lab[y], ["n", "t", "a", "A"]))[0]
            if len(cols):
                hood[y, max(0, cols.min() - 1):min(talla.ancho, cols.max() + 2)] = True
        # 3) la abertura de la cara (de frente): la mitad delantera de la cabeza, de las cejas a la barbilla
        abertura = np.zeros_like(hood)
        if frente:
            ceja = top + 4
            abertura = cab & (yy >= ceja) & (yy <= bottom - 1)
            if hacia_der:
                abertura &= xx >= cx - 1
            else:
                abertura &= xx <= cx + 1
        hood &= ~abertura
        # 4) tonos: contorno fuera, sombra junto a la abertura, luz arriba-delante, base el resto
        borde_ext = hood & ~ndimage.binary_erosion(hood | abertura, structure=np.ones((3, 3)), border_value=0)
        junto = hood & ndimage.binary_dilation(abertura, structure=np.ones((3, 3)))
        luz = hood & (yy < top + 5) & ((xx - cx) * lado > 0)
        tono = np.full(hood.shape, "s", dtype="<U1")
        tono[luz] = "H"
        tono[yy >= cuello_y + 2] = "d"  # el borde bajo de la esclavina
        tono[junto] = "d"
        if not frente:  # de espaldas, la costura de la capucha, de la punta a la nuca
            costura = hood & (xx == int(round(cx - lado))) & (yy < cuello_y - 1)
            tono[costura] = "d"
        tono[borde_ext] = "O"
        for t in ("H", "s", "d", "O"):
            sel = hood & (tono == t)
            capa[sel, :3] = m[t]
            capa[sel, 3] = m.alfa
        frames.append(capa)
    return frames
