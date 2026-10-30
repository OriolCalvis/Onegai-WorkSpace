"""Colas y cuerpos de serpiente: tubos sombreados anclados a la cadera.

Una cola no se dibuja pixel a pixel en 12 frames: es una linea central con
grosor (x, y, radio) relativa al ANCLA de la cadera de cada frame, y un
pintor de tubos la rasteriza con los cinco tonos del material (luz desde
arriba a la izquierda, contorno duro por fuera). El ancla sale del mapa de
partes de la talla, asi que la cola sigue al cuerpo en cada paso y en cada
talla; en los pasos la punta se balancea.

El orden de dibujo lo pone la ranura "cola" (actores_capas.ORDEN): detras
del cuerpo de frente y de lado, delante cuando da la espalda.
"""
import math

import numpy as np

from actores_pintura import fila_cinturon

# Balanceo de la punta en los pasos: frame -> desplazamiento (px) de la punta.
# Los quietos (2, 5, 8, 11) no se mueven.
BALANCEO = {1: -1, 3: 1, 4: -1, 6: 1, 7: 1, 9: -1, 10: -1, 12: 1,
            14: 1, 16: 1, 18: 1, 20: 1}  # ESTETICA-01: en el reposo la punta se mece

PARTES_CADERA = ("p", "t", "l", "L")


def ancla_cadera(talla, f):
    """(x, y) de la raiz de la cola en el frame f.

    De frente y de espaldas, el centro de la pelvis; de lado, el borde de
    atras de la cadera (la espalda mira a la izquierda en 'derecha').
    """
    lab = talla.labs[f]
    cint = fila_cinturon(lab)
    d = talla.direccion(f)
    y = cint + 2
    xs = [x for x in range(lab.shape[1]) if lab[y, x] in PARTES_CADERA]
    if d in ("abajo", "arriba"):
        return (min(xs) + max(xs)) / 2.0, float(y)
    if d == "derecha":
        return float(min(xs) + 1), float(cint + 1)
    return float(max(xs) - 1), float(cint + 1)


def _tonos_tubo(s, fino):
    """Tono por posicion transversal s (-1 sombra .. 1 luz)."""
    if fino:
        return "s" if s > -0.2 else "d"
    if s > 0.45:
        return "H"
    if s > -0.3:
        return "s"
    return "d"


def tubo(capa, puntos, material, alfas=None, vientre=None, contorno=True):
    """Rasteriza un tubo (in situ) sobre una capa RGBA.

    puntos: [(x, y, r)] en coordenadas del frame (floats). alfas: alfa por
    punto (se interpola; espectros que se desvanecen). vientre: (material,
    ancho) pinta una franja central clara con escamas cada dos filas; con
    ancho negativo la franja va por el lado de la sombra (el vientre de un
    tubo visto de perfil); un tercer valor opcional la corta en ese punto de
    la linea (la parte de la cola que se apoya en el suelo no ensena el
    vientre).
    Devuelve la mascara de pixeles pintados.
    """
    alto, ancho = capa.shape[:2]
    xs = [p[0] for p in puntos]
    ys = [p[1] for p in puntos]
    rmax = max(p[2] for p in puntos) + 1
    x0, x1 = max(0, int(min(xs) - rmax)), min(ancho - 1, int(max(xs) + rmax) + 1)
    y0, y1 = max(0, int(min(ys) - rmax)), min(alto - 1, int(max(ys) + rmax) + 1)
    dentro = np.zeros((alto, ancho), bool)
    info = {}
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            mejor = None
            for i in range(len(puntos) - 1):
                (ax, ay, ar), (bx, by, br) = puntos[i], puntos[i + 1]
                dx, dy = bx - ax, by - ay
                l2 = dx * dx + dy * dy
                t = 0.0 if l2 == 0 else max(0.0, min(1.0, ((x - ax) * dx + (y - ay) * dy) / l2))
                qx, qy = ax + t * dx, ay + t * dy
                r = ar + t * (br - ar)
                dist = math.hypot(x - qx, y - qy)
                if dist <= r and (mejor is None or dist - r < mejor[0]):
                    # normal apuntando a la luz (arriba-izquierda)
                    ln = math.sqrt(l2) or 1.0
                    nx, ny = -dy / ln, dx / ln
                    if nx + ny > 0:
                        nx, ny = -nx, -ny
                    s = ((x - qx) * nx + (y - qy) * ny) / max(r, 0.5)
                    mejor = (dist - r, s, i + t, r)
            if mejor is not None:
                dentro[y, x] = True
                info[(y, x)] = mejor
    n = len(puntos) - 1
    for (y, x), (_d, s, pos, r) in info.items():
        borde = contorno and any(
            not (0 <= y + vy < alto and 0 <= x + vx < ancho) or not dentro[y + vy, x + vx]
            for vy, vx in ((1, 0), (-1, 0), (0, 1), (0, -1)))
        m = material
        if borde:
            t = "O"
        elif vientre is not None and pos <= (vientre[2] if len(vientre) > 2 else n) and (
                (vientre[1] > 0 and abs(s) * r <= vientre[1] / 2.0) or
                (vientre[1] < 0 and s * r <= vientre[1] / 2.0)):
            m = vientre[0]
            t = "d" if y % 2 == 0 else "s"
        else:
            t = _tonos_tubo(s, r < 1.6)
        capa[y, x, :3] = m[t]
        a = m.alfa
        if alfas is not None:
            k = min(len(alfas) - 2, int(pos))
            fr = pos - k if n else 0.0
            a = int(round(alfas[k] + (alfas[k + 1] - alfas[k]) * fr))
        capa[y, x, 3] = a
    return dentro


# --- tipos de cola -----------------------------------------------------------
# Por direccion, la linea central relativa al ancla: (dx, dy, radio). La
# izquierda es el espejo de la derecha. 'punta' dice que hacer al final:
# "pincel" (mechon oscuro del toro), "pica" (punta de flecha del tiefling),
# "blanca" (punta clara del zorro).

COLAS = {
    "gato": {
        "abajo": [(0, 0, 1.2), (4, 3, 1.2), (8, 3, 1.2), (10, 0, 1.2), (10, -4, 1.1), (9, -7, 1.0)],
        "arriba": [(0, 0, 1.3), (0, 4, 1.3), (3, 7, 1.2), (7, 7, 1.2), (9, 4, 1.1), (9, 0, 1.0),
                   (8, -3, 1.0)],
        "derecha": [(0, 0, 1.2), (-4, 2, 1.2), (-7, 2, 1.2), (-9, -1, 1.1), (-9, -5, 1.0),
                    (-8, -8, 1.0)],
        "punta": None,
    },
    "zorro": {
        "abajo": [(0, 0, 1.4), (4, 1, 2.2), (8, 0, 3.0), (10, -4, 3.2), (10, -8, 2.6), (9, -11, 1.4)],
        "arriba": [(0, 0, 1.6), (1, 4, 2.4), (4, 7, 3.0), (8, 6, 3.2), (10, 2, 3.0), (10, -3, 2.4),
                   (9, -6, 1.4)],
        "derecha": [(0, 0, 1.4), (-4, 1, 2.4), (-8, 0, 3.0), (-11, -3, 3.2), (-12, -7, 2.6),
                    (-11, -10, 1.4)],
        "punta": "blanca",
    },
    "lagarto": {
        "abajo": [(0, 0, 2.6), (2, 5, 2.4), (5, 9, 2.0), (9, 11, 1.5), (13, 11, 0.9), (15, 10, 0.5)],
        "arriba": [(0, 0, 2.8), (0, 4, 2.6), (1, 8, 2.3), (4, 11, 1.9), (8, 12, 1.4), (12, 11, 0.8),
                   (14, 9, 0.5)],
        "derecha": [(0, 0, 2.8), (-4, 3, 2.5), (-8, 6, 2.1), (-12, 8, 1.6), (-16, 9, 1.0),
                    (-19, 8, 0.5)],
        "punta": None,
    },
    "draco": {
        "abajo": [(0, 0, 2.8), (2, 5, 2.6), (5, 9, 2.2), (9, 11, 1.7), (13, 10, 1.1), (16, 8, 0.6)],
        "arriba": [(0, 0, 3.0), (0, 4, 2.8), (1, 8, 2.5), (4, 11, 2.1), (8, 12, 1.6), (12, 10, 1.0),
                   (14, 7, 0.6)],
        "derecha": [(0, 0, 3.0), (-4, 3, 2.7), (-8, 6, 2.3), (-12, 7, 1.8), (-16, 7, 1.2),
                    (-19, 5, 0.6)],
        "punta": "pica",
    },
    "tiefling": {
        "abajo": [(0, 0, 1.0), (4, 3, 1.0), (8, 4, 0.9), (11, 2, 0.9), (12, -2, 0.8)],
        "arriba": [(0, 0, 1.0), (0, 4, 1.0), (3, 7, 0.9), (7, 8, 0.9), (10, 6, 0.8), (11, 2, 0.8)],
        "derecha": [(0, 0, 1.0), (-4, 3, 1.0), (-8, 4, 0.9), (-11, 2, 0.9), (-12, -2, 0.8)],
        "punta": "pica",
    },
    "toro": {
        "abajo": [(0, 0, 1.0), (3, 4, 0.9), (5, 8, 0.8), (6, 11, 0.8)],
        "arriba": [(0, 0, 1.0), (0, 4, 0.9), (2, 8, 0.8), (3, 11, 0.8)],
        "derecha": [(0, 0, 1.0), (-3, 3, 0.9), (-5, 7, 0.8), (-6, 10, 0.8)],
        "punta": "pincel",
    },
}


def _puntos(tipo, direccion, ax, ay, balanceo):
    espejo = direccion == "izquierda"
    clave = "derecha" if espejo else direccion
    base = COLAS[tipo][clave]
    n = len(base) - 1
    pts = []
    for i, (dx, dy, r) in enumerate(base):
        k = i / n if n else 0
        if clave in ("abajo", "arriba"):
            dx += balanceo * k * 1.5
        else:
            dy += balanceo * k
        pts.append((ax + (-dx if espejo else dx), ay + dy, r))
    return pts


def _punta(capa, tipo, pts, material, material_punta):
    """Remate de la punta de la cola."""
    forma = COLAS[tipo]["punta"]
    if not forma:
        return
    (ax, ay, _), (bx, by, _) = pts[-2], pts[-1]
    if forma == "pincel":
        m = material_punta or material
        tubo(capa, [(bx, by - 0.5, 1.0), (bx, by + 1.5, 1.7)], m)
    elif forma == "blanca":
        m = material_punta or material
        k = 0.45
        mx, my = ax + (bx - ax) * (1 - k), ay + (by - ay) * (1 - k)
        tubo(capa, [(mx, my, 1.9), (bx, by, 1.4)], m)
    elif forma == "pica":
        m = material_punta or material
        dx, dy = bx - ax, by - ay
        ln = math.hypot(dx, dy) or 1.0
        ux, uy = dx / ln, dy / ln
        cx, cy = bx + ux * 1.5, by + uy * 1.5
        tubo(capa, [(bx - ux * 0.5, by - uy * 0.5, 1.8), (cx, cy, 1.4),
                    (cx + ux * 1.6, cy + uy * 1.6, 0.4)], m)


def capa_cola(talla, tipo, material, material_punta=None):
    """Capa de 12 frames con una cola del tipo dado."""
    frames = []
    for f in range(1, talla.n_frames + 1):
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        ax, ay = ancla_cadera(talla, f)
        pts = _puntos(tipo, talla.direccion(f), ax, ay, BALANCEO.get(f, 0))
        tubo(capa, pts, material)
        _punta(capa, tipo, pts, material, material_punta)
        frames.append(capa)
    return frames


def capa_colas_kitsune(talla, material, material_punta, n=3):
    """Varias colas de zorro en abanico (yokai)."""
    frames = []
    giros = {1: [0.0], 2: [-0.5, 0.5], 3: [-0.9, 0.0, 0.9]}[n]
    for f in range(1, talla.n_frames + 1):
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        ax, ay = ancla_cadera(talla, f)
        d = talla.direccion(f)
        for g in giros:
            pts = _puntos("zorro", d, ax, ay, BALANCEO.get(f, 0))
            rot = []
            for (x, y, r) in pts:
                ang = g * 0.55
                dx, dy = x - ax, y - ay
                if d in ("abajo", "arriba"):
                    # abanico: gira cada cola alrededor del ancla
                    dx, dy = dx * math.cos(ang) - dy * math.sin(ang), dx * math.sin(ang) + dy * math.cos(ang)
                else:
                    dy += g * 3
                rot.append((ax + dx, ay + dy, r * 0.9))
            tubo(capa, rot, material)
            _punta(capa, "zorro", rot, material, material_punta)
        frames.append(capa)
    return frames
