"""Oriol (24/10): lo de la cabeza (sombreros, capuchas, cascos) en las cuatro diagonales, mientras no
haya los de PixelLab.

Las piezas de cabeza son plantillas dibujadas contra la cabeza del cuerpo de siempre en cada vista
(actores_cascos, actores_capas.pintar_plantilla). Aqui se pinta la plantilla de la vista mas parecida
(VISTA: la de frente o la de espaldas, segun la diagonal) y se lleva de la cabeza de siempre a la de PixelLab
del frame: lo alto de la cabeza y su centro.

    capa_cabeza_pl(talla, ident, materiales)  -> frames (como actores_cascos.capa_cabeza)
"""
import numpy as np

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


def capa_cabeza_pl(talla, ident, materiales, vista=None):
    pieza = cascos.CABEZA[ident]
    plantillas, m1, m2 = pieza["plantillas"], pieza["m1"], pieza["m2"]
    clasica = pixellab_armas.clasicas()[pixellab_armas.CLASICA.get(talla.clave, "medio")]
    frames = []
    for f in range(1, talla.n_frames + 1):
        d = talla.diagonal(f)
        v = (vista or VISTA)[d]
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        de, a = _cabeza(clasica.labs[QUIETO[v]]), _cabeza(talla.labs[f])
        if de is None or a is None:
            frames.append(capa)
            continue
        dy, dx = a[0] - de[0], int(round(a[1] - de[1]))
        ojos = _mueve(np.asarray(talla.ojos[f]), -dx, -dy)
        cuerpo = _mueve(np.asarray(talla.alfa[f]), -dx, -dy)
        for pl, m2_propio in plantillas:
            pintar_plantilla(capa, pl, v, 0, materiales[m1],
                             materiales[m2_propio or m2] if (m2_propio or m2) else None,
                             ojos=ojos, cuerpo=cuerpo, x0=pl.get("_x0", X0_PLANTILLA))
        frames.append(_mueve(capa, dx, dy))
    return frames
