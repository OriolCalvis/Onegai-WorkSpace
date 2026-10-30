"""Oriol (24/10): el pelo y la barba en las cuatro diagonales, sobre las cabezas de PixelLab.

Las plantillas de pelo de siempre (actores_pelo) se dibujaron contra la cabeza fija del cuerpo base en
cuatro vistas. Las cabezas de PixelLab cambian de raza a raza y las vistas son diagonales, asi que aqui
el pelo se pinta a partir de la cabeza de cada frame: su silueta (la parte «h» del mapa de partes), la
linea de los ojos (el esqueleto) y hacia donde mira (sureste/suroeste: la cara; noreste/noroeste: la
nuca). Cada peinado es una regla: cuanto cubre la cabeza, cuanto volumen le da y lo que cae (melena,
coleta, mono, cresta). Los cinco tonos son los del material del pelo.

    capa_pelo(talla, estilo, color)   -> frames RGBA (como actores_personajes.capa_pelo)
    capa_barba(talla, estilo, color)
"""
import numpy as np
from scipy import ndimage

from actores_materiales import MATERIALES as MAT

ESTILOS = ("corto", "largo", "rizado", "cresta", "greñas", "recogido", "coleta")


def _kp(talla, f):
    return {p["label"]: (p["x"] * talla.ancho, p["y"] * talla.alto) for p in talla.esqueletos[f]}


def _cabeza(talla, f):
    return talla.labs[f] == "h"


def _sombrear(capa, mascara, m, luz_izquierda=True):
    """Pinta la mascara con los cinco tonos del material: contorno fuera, luz arriba, sombra abajo."""
    if not mascara.any():
        return
    ys, xs = np.nonzero(mascara)
    y0, y1 = ys.min(), ys.max()
    borde = mascara & ~ndimage.binary_erosion(mascara)
    alto = max(1, y1 - y0)
    for y, x in zip(ys.tolist(), xs.tolist()):
        if borde[y, x]:
            t = "O"
        else:
            rel = (y - y0) / alto
            t = "H" if rel < 0.25 else ("s" if rel < 0.65 else "d")
            # mechones: una raya de sombra cada tres columnas en la mitad de abajo
            if rel >= 0.4 and (x % 3 == 0):
                t = "d" if t == "s" else "o"
        capa[y, x, :3] = m[t]
        capa[y, x, 3] = 255


def _gorro(cab, ojos_y, frente, extra=0):
    """La parte de la cabeza que tapa el pelo: de frente, por encima de la frente; de espaldas, casi toda."""
    ys = np.arange(cab.shape[0])[:, None]
    if frente:
        tapa = cab & (ys < ojos_y - 1.5 + extra)
    else:
        tapa = cab & (ys < ojos_y + 3.5 + extra)
    return tapa


def _volumen(tapa, cab, px):
    """Engorda el pelo px pixeles hacia fuera de la cabeza, solo por arriba y por los lados."""
    if px <= 0:
        return tapa
    out = tapa.copy()
    for _ in range(px):
        crece = ndimage.binary_dilation(out) & ~cab
        # no crece hacia abajo de la cabeza
        ys = np.nonzero(tapa.any(1))[0]
        if len(ys):
            crece[ys.max():] = False
        out |= crece
    return out


def pelo_frame(talla, f, estilo, color):
    capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
    cab = _cabeza(talla, f)
    if not cab.any():
        return capa
    k = _kp(talla, f)
    d = talla.diagonal(f)
    frente = d.startswith("sur")
    ojos_y = (k["LEFT EYE"][1] + k["RIGHT EYE"][1]) / 2
    ys, xs = np.nonzero(cab)
    top, cx = ys.min(), xs.mean()
    m = MAT[color]
    hacia_der = d.endswith("este")  # sureste/noreste: mira a la derecha de la pantalla
    vol = {"corto": 0, "largo": 1, "rizado": 2, "cresta": 0, "greñas": 1, "recogido": 0, "coleta": 0}[estilo]
    if estilo == "cresta":
        tapa = cab & (np.abs(np.arange(talla.ancho)[None, :] - cx) <= 1.5) & (np.arange(talla.alto)[:, None] < ojos_y + (0 if frente else 4))
        for y in range(max(0, top - 3), top):
            for x in range(int(cx - 1), int(cx + 2)):
                tapa[y, x] = True
    else:
        tapa = _volumen(_gorro(cab, ojos_y, frente), cab, vol)
    # lo que cae: melena, greñas, coleta, moño
    cuello = k.get("NECK", (cx, ojos_y + 6))
    if estilo in ("largo", "greñas"):
        largo = 9 if estilo == "largo" else 5
        y_fin = int(min(talla.alto - 1, cuello[1] + largo))
        filas = np.arange(talla.alto)[:, None]
        if frente:
            # dos mechones a los lados de la cara, por detras de los hombros
            lados = (np.abs(np.arange(talla.ancho)[None, :] - cx) >= (xs.max() - xs.min()) / 2 - 1.5)
            melena = ndimage.binary_dilation(cab, iterations=1) & lados & (filas >= ojos_y - 2) & (filas <= y_fin)
            melena &= ~talla.alfa[f] | cab
        else:
            # de espaldas tapa la nuca y la espalda hasta y_fin
            ancho = (xs.max() - xs.min()) / 2 + 0.5
            melena = (np.abs(np.arange(talla.ancho)[None, :] - cx) <= ancho) & (filas >= top) & (filas <= y_fin)
        tapa |= melena
    if estilo in ("coleta", "recogido"):
        atras = -1 if hacia_der else 1  # la nuca queda al otro lado de donde mira
        bx = int(round(cx + atras * ((xs.max() - xs.min()) / 2 + (0 if frente else -2))))
        by = int(top + (2 if estilo == "recogido" else 4))
        r = 2 if estilo == "recogido" else 1.5
        yy, xx = np.mgrid[0:talla.alto, 0:talla.ancho]
        tapa |= (yy - by) ** 2 + (xx - bx) ** 2 <= r * r
        if estilo == "coleta":
            for i in range(7):
                y = by + 1 + i
                x = bx + atras * (i // 3)
                if 0 <= y < talla.alto and 0 <= x < talla.ancho:
                    tapa[y, x] = True
                    if 0 <= x - atras < talla.ancho and i < 4:
                        tapa[y, x - atras] = True
    if frente:
        # el pelo nunca tapa los ojos ni la cara por debajo de la frente (salvo los mechones de los lados)
        cara = cab & (np.arange(talla.alto)[:, None] >= ojos_y - 1.5) & (np.abs(np.arange(talla.ancho)[None, :] - cx) < (xs.max() - xs.min()) / 2 - 1)
        tapa &= ~cara
        tapa &= ~talla.ojos[f]
    # la melena va por detras del cuerpo de frente: lo que cae fuera de la cabeza no tapa los hombros
    if frente:
        tapa &= cab | ~talla.alfa[f]
    _sombrear(capa, tapa, m)
    return capa


def capa_pelo(talla, estilo, color):
    return [pelo_frame(talla, f, estilo, color) for f in range(1, talla.n_frames + 1)]


def barba_frame(talla, f, estilo, color):
    capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
    d = talla.diagonal(f)
    if not d.startswith("sur"):
        return capa  # de espaldas no se ve
    cab = _cabeza(talla, f)
    if not cab.any():
        return capa
    k = _kp(talla, f)
    nariz = k["NOSE"]
    ys, xs = np.nonzero(cab)
    yy = np.arange(talla.alto)[:, None]
    larga = estilo == "BARBA"
    barba = cab & (yy >= nariz[1] + 1)
    if larga:
        # baja por debajo de la barbilla hasta el pecho
        bajo = int(ys.max())
        cx = int(round(nariz[0]))
        for y in range(bajo + 1, min(talla.alto, bajo + 5)):
            ancho = 3 - (y - bajo) // 2
            for x in range(cx - ancho, cx + ancho + 1):
                barba[y, x] = True
    barba &= ~talla.ojos[f]
    _sombrear(capa, barba, MAT[color])
    return capa


def capa_barba(talla, estilo, color):
    return [barba_frame(talla, f, estilo, color) for f in range(1, talla.n_frames + 1)]
