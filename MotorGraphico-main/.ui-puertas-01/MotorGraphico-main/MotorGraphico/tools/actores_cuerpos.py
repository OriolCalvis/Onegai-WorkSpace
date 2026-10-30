"""Cuerpos especiales: la cola de serpiente de las nagas y el jiron de los
espectros. Sustituyen a las piernas del cuerpo base (las nagas y los
espectros no llevan piernas ni pies del equipo).

Se pintan con el pintor de tubos de actores_colas sobre el ancla de la
pelvis de cada frame, y el cuerpo de cintura para arriba se pinta encima
(las manos cuelgan por delante de la cola).
"""
import numpy as np

from actores_capas import pintar_cuerpo
from actores_colas import BALANCEO, tubo
from actores_pintura import fila_cinturon

PIERNAS = ("l", "L", "f", "F")

# Linea central (dx, dy, radio) desde el centro de la pelvis (cinturon + 2).
SERPIENTE = {
    "abajo": [(0, 0, 5.0), (0, 3, 4.9), (1, 6, 4.7), (4, 8, 4.0), (8, 8.5, 3.4), (11, 7.5, 2.8),
              (13, 5.5, 2.2), (13, 3, 1.6), (12, 1, 1.0)],
    "arriba": [(0, 0, 5.0), (0, 3, 4.9), (-1, 6, 4.7), (-4, 8, 4.0), (-8, 8.5, 3.4), (-11, 7.5, 2.8),
               (-13, 5.5, 2.2), (-13, 3, 1.6), (-12, 1, 1.0)],
    "derecha": [(0, 0, 4.6), (0, 4, 4.5), (-2, 7, 4.2), (-6, 8.5, 3.6), (-11, 8.5, 3.0), (-15, 7.5, 2.3),
                (-18, 6, 1.6), (-20, 5, 0.8)],
}
JIRON = {
    "abajo": [(0, 0, 4.6), (0, 4, 4.2), (1, 8, 3.3), (3, 11, 2.3), (6, 12, 1.3)],
    "arriba": [(0, 0, 4.6), (0, 4, 4.2), (-1, 8, 3.3), (-3, 11, 2.3), (-6, 12, 1.3)],
    "derecha": [(0, 0, 4.2), (-1, 4, 3.8), (-4, 7, 3.0), (-8, 9, 2.1), (-12, 9, 1.2)],
}
ALFA_JIRON = [200, 180, 130, 80, 30]


def _centro_pelvis(lab, d):
    cint = fila_cinturon(lab)
    y = cint + 2
    xs = [x for x in range(lab.shape[1]) if lab[y, x] in ("p", "t", "l", "L")]
    return (min(xs) + max(xs)) / 2.0, float(y)


def _linea(tabla, d, ax, ay, balanceo):
    espejo = d == "izquierda"
    base = tabla["derecha" if espejo else d]
    n = len(base) - 1
    pts = []
    for i, (dx, dy, r) in enumerate(base):
        k = i / n
        if d in ("abajo", "arriba"):
            dx += balanceo * k * 1.5
        else:
            dy += balanceo * k
        pts.append((ax + (-dx if espejo else dx), ay + dy, r))
    return pts


def _sin_piernas(talla, f, piel, ojos):
    px = pintar_cuerpo(talla, f, piel, ojos)
    px[np.isin(talla.labs[f], PIERNAS)] = 0
    return px


def capa_naga(talla, piel, escamas_cola, vientre, ojos=None):
    """Cuerpo de naga: torso de la raza sobre una cola de serpiente."""
    frames = []
    for f in range(1, talla.n_frames + 1):
        lab, d = talla.labs[f], talla.direccion(f)
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        ax, ay = _centro_pelvis(lab, d)
        pts = _linea(SERPIENTE, d, ax, ay, BALANCEO.get(f, 0))
        franja = None if d == "arriba" else ((vientre, 5, 2.6) if d == "abajo" else (vientre, -3))
        tubo(capa, pts, escamas_cola, vientre=franja)
        torso = _sin_piernas(talla, f, piel, ojos)
        img = capa.copy()
        encima = torso[..., 3] > 0
        img[encima] = torso[encima]
        frames.append(img)
    return frames


def capa_espectro(talla, piel, ojos=None):
    """Espectro: cuerpo translucido que se deshace en un jiron."""
    frames = []
    for f in range(1, talla.n_frames + 1):
        lab, d = talla.labs[f], talla.direccion(f)
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        ax, ay = _centro_pelvis(lab, d)
        pts = _linea(JIRON, d, ax, ay - 1, BALANCEO.get(f, 0))
        tubo(capa, pts, piel, alfas=ALFA_JIRON)
        torso = _sin_piernas(talla, f, piel, ojos)
        encima = torso[..., 3] > 0
        capa[encima] = torso[encima]
        frames.append(capa)
    return frames
