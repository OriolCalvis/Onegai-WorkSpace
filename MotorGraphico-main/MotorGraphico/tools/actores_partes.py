"""Mapa de partes del cuerpo base (assets/textures/personaje.png).

personaje.png es una hoja 12x1 de 64x64 con el mismo cuerpo en cuatro
direcciones y tres frames por direccion (paso A, quieto, paso B):

    frames  1-3  arriba (espalda)
    frames  4-6  derecha
    frames  7-9  abajo (de frente)
    frames 10-12 izquierda

Solo siete frames son dibujos distintos. El resto son espejos exactos
(comprobado pixel a pixel, ignorando el RGB bajo alpha 0):

    3  = espejo(1) desplazado 1 px      9  = espejo(7) desplazado 1 px
    10 = espejo(4)   11 = espejo(5)   12 = espejo(6)
    2 y 8 comparten silueta (espalda y frente del mismo cuerpo quieto)

Aqui se etiqueta cada pixel opaco con la parte a la que pertenece. Las
filas de la cabeza (las 13 primeras de cada frame) se etiquetan solas; el
resto se declara por tramos de x por fila. El generador comprueba que cada
pixel opaco tiene etiqueta y que ninguna etiqueta cae en un pixel vacio.

Etiquetas
    h cabeza    n cuello    t torso (hombros, pecho, espalda)
    p pelvis (desde la linea del cinturon hasta la entrepierna)
    a/A brazo   m/M mano    l/L pierna   f/F pie

En las vistas de frente y espalda, minuscula = brazo/pierna de la
izquierda de la pantalla y mayuscula = de la derecha. En las vistas
laterales, mayuscula = el lado CERCANO a la camara y minuscula = el
lejano; al hacer el espejo a la izquierda se conserva (sigue siendo el
cercano), mientras que en frente/espalda el espejo intercambia los dos.
"""
import numpy as np
from PIL import Image


# Filas manuales: {fila: "x0-x1<etiqueta> x0-x1<etiqueta> ..."}
FRENTE_QUIETO = {  # frames 2 y 8 (top 21)
    34: "29-35h",
    35: "28-29t 30-34n 35-36t",
    36: "27-37t",
    37: "26-38t",
    38: "26-38t",
    39: "25-28a 29-35t 36-39A",
    40: "25-28a 29-35t 36-39A",
    41: "25-28a 29-35t 36-39A",
    42: "24-28a 29-35t 36-40A",
    43: "24-28a 29-35t 36-40A",
    44: "24-28a 29-35t 36-40A",
    45: "24-28a 29-35t 36-40A",
    46: "24-28m 29-35t 36-40M",
    47: "24-28m 29-35p 36-40M",
    48: "24-28m 29-35p 36-40M",
    49: "25-28m 29-35p 36-40M",
    50: "26-28m 29-35p 36-39M",
    51: "27-31l 33-37L",
    52: "27-31l 33-37L",
    53: "27-31l 33-37L",
    54: "27-31l 33-37L",
    55: "27-31l 33-37L",
    56: "27-31l 33-37L",
    57: "26-31f 33-38F",
    58: "25-31f 33-39F",
    59: "25-31f 33-39F",
    60: "25-31f 33-39F",
}

# Piernas comunes a los pasos de frente (7) y de espalda (1): son identicas.
_PIERNAS_PASO_VERTICAL = {
    50: "28-31l 32-36L",
    51: "28-32l 33-36L",
    52: "28-32l 33-36L",
    53: "28-32l 33-36L",
    54: "28-32l 33-36L",
    55: "28-32l 33-36L",
    56: "28-32l 33-36L",
    57: "27-32l 33-37F",
    58: "27-32l 33-37F",
    59: "27-32f 33-37F",
    60: "27-32f",
    61: "27-32f",
    62: "27-31f",
}

FRENTE_PASO = {  # frame 7 (top 22); 9 es su espejo
    35: "29-35h",
    36: "27-29t 30-34n 35-36t",
    37: "26-37t",
    38: "26-38t",
    39: "25-28a 29-35t 36-38A",
    40: "25-28a 29-34t 35-38A",
    41: "25-28a 29-34t 35-39A",
    42: "24-28a 29-34t 35-39A",
    43: "24-28a 29-34t 35-39A",
    44: "24-28a 29-33t 34-39A",
    45: "24-28m 29-32t 33-38M",
    46: "24-28m 29-31t 32-38M",
    47: "25-28m 29-31t 32-37M",
    48: "26-28m 29-32p 33-36M",
    49: "28-33p 34-36M",
    **_PIERNAS_PASO_VERTICAL,
}

ESPALDA_PASO = {  # frame 1 (top 22); 3 es su espejo
    35: "29-35h",
    36: "28-29t 30-34n 35-36t",
    37: "27-37t",
    38: "26-37t",
    39: "26-28a 29-35t 36-37A",
    40: "25-28a 29-35t 36-38A",
    41: "25-28a 29-35t 36-38A",
    42: "25-28a 29-34t 35-38A",
    43: "25-28a 29-34t 35-39A",
    44: "25-28a 29-34t 35-39A",
    45: "25-28m 29-34t 35-39M",
    46: "25-28m 29-34t 35-39M",
    47: "26-28m 29-34t 35-39M",
    48: "27-28m 29-34p 35-39M",
    49: "28-34p 35-38M",
    **_PIERNAS_PASO_VERTICAL,
}

LADO_QUIETO = {  # frame 5 (top 20); 11 es su espejo
    33: "28-36h",
    34: "27-31n 32-35h",
    35: "27-33t",
    36: "26-34t",
    37: "26-34t",
    38: "25-29A 30-35t",
    39: "25-29A 30-35t",
    40: "25-29A 30-35t",
    41: "25-29A 30-35t",
    42: "25-29A 30-35t",
    43: "25-29A 30-35t",
    44: "25-30A 31-35t",
    45: "25-30A 31-35t",
    46: "25-31M 32-35t",
    47: "26-31M 32-35p",
    48: "26-30M 31-35p",
    49: "27-29M 30-35p",
    50: "27-35p",
    51: "27-31L 32-35l",
    52: "26-30L 31-34l",
    53: "26-30L 31-34l",
    54: "26-30L 31-34l",
    55: "25-30L 31-34l",
    56: "25-30L 31-35l",
    57: "25-30F 31-36f",
    58: "25-31F 32-36f",
    59: "25-31F 32-36f",
    60: "25-31F",
}

LADO_PASO_A = {  # frame 4 (top 21); 10 es su espejo. Brazo cercano atras.
    34: "28-36h",
    35: "27-31n 32-35h",
    36: "26-33t",
    37: "26-34t",
    38: "25-34t",
    39: "25-29A 30-35t",
    40: "24-28A 29-35t",
    41: "24-28A 29-35t 36-36a",
    42: "24-27A 28-35t 36-36a",
    43: "23-27A 28-35t 36-37a",
    44: "23-28M 29-35t 36-38m",
    45: "23-28M 29-35t 36-38m",
    46: "23-28M 29-35t 36-38m",
    47: "23-28M 29-35t 36-37m",
    48: "24-27M 28-35p",
    49: "25-26M 27-35p",
    50: "26-28l 29-36L",
    51: "25-29l 30-36L",
    52: "25-29l 30-37L",
    53: "24-29l 30-37L",
    54: "24-29l 32-37L",
    55: "23-28l 32-38L",
    56: "22-27f 33-41F",
    57: "21-26f 34-42F",
    58: "21-27f 34-42F",
    59: "22-28f 35-42F",
    60: "23-28f 35-40F",
    61: "24-27f 35-39F",
    62: "35-37F",
}

LADO_PASO_B = {  # frame 6 (top 21); 12 es su espejo. Brazo cercano delante.
    34: "28-36h",
    35: "27-31n 32-35h",
    36: "26-33t",
    37: "26-34t",
    38: "26-34t",
    39: "26-29t 30-35A",
    40: "25-27a 28-30t 31-35A",
    41: "25-27a 28-31t 32-35A",
    42: "25-27a 28-32t 33-37A",
    43: "25-27a 28-33t 34-38A",
    44: "24-27m 28-29t 30-38M",
    45: "24-27m 28-29t 30-39M",
    46: "24-27m 28-31t 32-39M",
    47: "25-27m 28-32t 33-38M",
    48: "26-27m 28-34p 35-37M",
    49: "26-35p",
    50: "26-32L 33-35l",
    51: "25-32L 33-36l",
    52: "25-31L 32-36l",
    53: "24-30L 31-36l",
    54: "24-29L 32-37l",
    55: "23-28L 33-40l",
    56: "22-27L 33-41f",
    57: "21-26F 34-41f",
    58: "21-27F 34-41f",
    59: "22-28F 35-39f",
    60: "23-28F 35-38f",
    61: "24-27F",
}

# Frame (1-based) -> (especificacion, direccion). Los que faltan se derivan.
DIBUJADOS = {
    1: (ESPALDA_PASO, "arriba"),
    2: (FRENTE_QUIETO, "arriba"),
    4: (LADO_PASO_A, "derecha"),
    5: (LADO_QUIETO, "derecha"),
    6: (LADO_PASO_B, "derecha"),
    7: (FRENTE_PASO, "abajo"),
    8: (FRENTE_QUIETO, "abajo"),
}
# Frame derivado -> frame fuente del que es espejo (el desplazamiento en x
# se mide al construir, y si deja de ser un espejo exacto, falla).
ESPEJOS = {3: 1, 9: 7, 10: 4, 11: 5, 12: 6}
DIRECCION = {1: "arriba", 2: "arriba", 3: "arriba", 4: "derecha", 5: "derecha",
             6: "derecha", 7: "abajo", 8: "abajo", 9: "abajo", 10: "izquierda",
             11: "izquierda", 12: "izquierda"}

# ESTETICA-01 (reposo animado, como en NosTale: casi quieto, pero vivo): dos frames de
# reposo por direccion, 13-20, sacados del quieto. El primero es el cuerpo quieto; en el
# segundo lo que esta por encima de las piernas baja un pixel (respira) y el arma y lo que
# cuelga se mecen. En los dos el arma se lleva como en reposo (actores_poses: la espada con
# la punta en el suelo, el espadon delante...), que no es como se lleva andando. El motor
# los pone en bucle mientras no anda; el frame quieto (2, 5, 8, 11) queda para el ciclo
# de andar.
REPOSO = {13: 2, 14: 2, 15: 5, 16: 5, 17: 8, 18: 8, 19: 11, 20: 11}  # frame -> su quieto
DIRECCION.update({f: DIRECCION[q] for f, q in REPOSO.items()})
N_TOTAL = 20  # los 12 de andar y los 8 de reposo
TODOS = tuple(range(1, N_TOTAL + 1))


def paso_de(f):
    """Que es el frame f para lo que se mece: 'A', 'quieto', 'B' (andar), 'reposo1' o 'reposo2'."""
    if f in REPOSO:
        return "reposo1" if (f - 13) % 2 == 0 else "reposo2"
    return ("A", "quieto", "B")[(f - 1) % 3]


# --- Construccion y validacion -------------------------------------------

FRAME = 64
N_FRAMES = 12
FILAS_CABEZA = 13
ETIQUETAS = "hntpaAmMlLfF"
SWAP_LR = {"a": "A", "A": "a", "m": "M", "M": "m", "l": "L", "L": "l", "f": "F", "F": "f"}


def cargar_base(ruta):
    """Los 12 frames de personaje.png como matrices RGBA (RGB bajo alpha 0 a cero)."""
    im = np.array(Image.open(ruta).convert("RGBA"))
    if im.shape[:2] != (FRAME, FRAME * N_FRAMES):
        raise SystemExit(f"{ruta}: se esperaba {FRAME * N_FRAMES}x{FRAME}, hay {im.shape[1]}x{im.shape[0]}")
    im[im[:, :, 3] == 0] = 0  # el RGB bajo alpha 0 es basura: se normaliza
    return [im[:, i * FRAME:(i + 1) * FRAME].copy() for i in range(N_FRAMES)]


def _parse(fila_txt):
    for trozo in fila_txt.split():
        rango, etiqueta = trozo[:-1], trozo[-1]
        if etiqueta not in ETIQUETAS:
            raise ValueError(f"etiqueta desconocida {etiqueta!r} en {fila_txt!r}")
        x0, x1 = (int(v) for v in rango.split("-"))
        yield x0, x1, etiqueta


def _etiquetar(frame_px, filas):
    opaco = frame_px[:, :, 3] > 0
    ys = np.nonzero(opaco.any(axis=1))[0]
    top = int(ys.min())
    lab = np.full((FRAME, FRAME), "", dtype="<U1")
    for y in range(top, top + FILAS_CABEZA):
        lab[y, opaco[y]] = "h"
    for y, txt in filas.items():
        for x0, x1, e in _parse(txt):
            lab[y, x0:x1 + 1] = e
    return lab, top


def _mejor_espejo(fuente_px, destino_px):
    esp = fuente_px[:, ::-1]
    for dx in (0, 1, -1, 2, -2):
        if np.array_equal(np.roll(esp, dx, axis=1), destino_px):
            return dx
    return None


def construir(ruta_base):
    """(frames, mapas de partes por frame 1..12, errores). Sin errores = mapa completo."""
    frames = cargar_base(ruta_base)
    labs = {}
    errores = []
    for f, (filas, _dir) in DIBUJADOS.items():
        lab, _top = _etiquetar(frames[f - 1], filas)
        labs[f] = lab
    for f, fuente in ESPEJOS.items():
        dx = _mejor_espejo(frames[fuente - 1], frames[f - 1])
        if dx is None:
            errores.append(f"frame {f}: ya no es espejo del {fuente}")
            continue
        lab = np.roll(labs[fuente][:, ::-1], dx, axis=1)
        if DIRECCION[f] in ("arriba", "abajo"):
            lab = np.vectorize(lambda c: SWAP_LR.get(c, c))(lab)
        labs[f] = lab
    for f in range(1, N_FRAMES + 1):
        opaco = frames[f - 1][:, :, 3] > 0
        lab = labs[f]
        sin = np.argwhere(opaco & (lab == ""))
        sobra = np.argwhere(~opaco & (lab != ""))
        for y, x in sin[:6]:
            errores.append(f"frame {f}: pixel opaco sin etiqueta en x={x} y={y}")
        for y, x in sobra[:6]:
            errores.append(f"frame {f}: etiqueta {lab[y, x]!r} en pixel vacio x={x} y={y}")
    return frames, labs, errores
