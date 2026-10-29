"""Alas: plumas plegadas (aarakocra) y alas de hada translucidas.

Plantillas como las de cabeza, pero ancladas a los HOMBROS: se escriben
contra el frame quieto del cuerpo base (hombros en la fila 35) y bajan o
suben con la fila de los hombros de cada frame y talla. En las tallas
anchas cada mitad se abre hacia fuera tantas columnas como gano el torso.
La ranura "alas" las pone detras del cuerpo de frente y de lado, y encima
de la espalda cuando el personaje se aleja.
"""
import numpy as np

from actores_capas import pintar_plantilla
from actores_tallas import TALLAS

X0_ALAS = 14         # la cadena de cada fila empieza en x = 14
EJE = 32
HOMBROS_QUIETO = 35  # primera fila del torso en el frame quieto del cuerpo base
SOMBRA = {"1": "3", "6": "8"}


def _filas(tramos):
    """[(y, x, texto)] -> (fila0, [cadenas desde X0_ALAS])."""
    y0 = min(y for y, _x, _t in tramos)
    y1 = max(y for y, _x, _t in tramos)
    filas = [""] * (y1 - y0 + 1)
    for y, x, t in tramos:
        filas[y - y0] = "." * (x - X0_ALAS) + t
    return (y0, filas)


def _simetrico(tramos, sombra=True):
    """Mitad izquierda (x < 32) -> plantilla simetrica respecto a x = 32."""
    fila0, filas = _filas(tramos)
    out = []
    for f in filas:
        izq = f.ljust(EJE - X0_ALAS, ".")[:EJE - X0_ALAS]
        der = "".join(SOMBRA.get(c, c) if sombra else c for c in reversed(izq))
        out.append((izq + "." + der).rstrip("."))
    return (fila0, out)


# --- aarakocra: alas plegadas a la espalda ----------------------------------
# material 1 = plumas cobertoras, 2 = remeras (las plumas largas del borde)

_ALA_ESPALDA = [
    (35, 28, "444"),
    (36, 26, "411224"),
    (37, 24, "41122234"),
    (38, 23, "412222334"),
    (39, 22, "4122232334"),
    (40, 22, "4222322334"),
    (41, 22, "4223223234"),
    (42, 22, "422322334"),
    (43, 22, "422232334"),
    (44, 22, "477749944"),
    (45, 22, "47894784"),
    (46, 22, "47894784"),
    (47, 22, "4789478"),
    (48, 22, "4789478"),
    (49, 22, "478948"),
    (50, 22, "49.49"),
    (51, 22, "9..9"),
]
_ALA_FRENTE = [  # detras del cuerpo: asoma por encima de los hombros y por fuera
    (32, 26, "44"),
    (33, 24, "4114"),
    (34, 23, "411224"),
    (35, 22, "4122224"),
    (36, 21, "41222234"),
    (37, 21, "4222334"),
    (38, 21, "422334"),
    (39, 21, "42334"),
    (40, 21, "42234"),
    (41, 21, "42334"),
    (42, 21, "4234"),
    (43, 21, "4234"),
    (44, 21, "4784"),
    (45, 21, "4784"),
    (46, 21, "4784"),
    (47, 21, "4784"),
    (48, 21, "478"),
    (49, 21, "478"),
    (50, 21, "49"),
    (51, 21, "9"),
]
_ALA_LADO = [  # de perfil: cuelga del hombro y asoma por detras de la espalda
    (33, 25, "444"),
    (34, 23, "4411224"),
    (35, 21, "441122224"),
    (36, 20, "4112222234"),
    (37, 19, "41222223334"),
    (38, 19, "4222232334"),
    (39, 18, "42223232334"),
    (40, 18, "4223223234"),
    (41, 18, "422322334"),
    (42, 18, "42232334"),
    (43, 18, "4777994"),
    (44, 18, "478478"),
    (45, 18, "478478"),
    (46, 19, "478478"),
    (47, 19, "47847"),
    (48, 20, "4784"),
    (49, 20, "4784"),
    (50, 21, "494"),
    (51, 21, "9"),
]

ALAS_AVE = {
    "arriba": _simetrico(_ALA_ESPALDA),
    "abajo": _simetrico(_ALA_FRENTE),
    "derecha": _filas(_ALA_LADO),
}

# --- sangre feerica: alas de mariposa translucidas --------------------------
# material 1 = membrana (translucida), 2 = nervios y borde

_HADA_ESPALDA = [
    (24, 18, "999"),
    (25, 17, "91119"),
    (26, 16, "9111129"),
    (27, 16, "91112229"),
    (28, 16, "911222229"),
    (29, 16, "9112222229"),
    (30, 17, "9122232229"),
    (31, 17, "91222322229"),
    (32, 18, "9222232229"),
    (33, 19, "922223229"),
    (34, 20, "92222329"),
    (35, 21, "9222329"),
    (36, 22, "922339"),
    (37, 23, "99339"),
    (38, 24, "9939"),
    (39, 23, "9229"),
    (40, 22, "912229"),
    (41, 21, "9122229"),
    (42, 21, "9122239"),
    (43, 21, "912239"),
    (44, 22, "92239"),
    (45, 22, "9239"),
    (46, 23, "999"),
]
_HADA_LADO = [
    (23, 13, "999"),
    (24, 12, "91119"),
    (25, 12, "9111229"),
    (26, 12, "91122229"),
    (27, 12, "911222229"),
    (28, 13, "9122223229"),
    (29, 13, "91222232229"),
    (30, 14, "91222232229"),
    (31, 15, "9122223229"),
    (32, 16, "912223229"),
    (33, 17, "91223329"),
    (34, 18, "9223329"),
    (35, 19, "923339"),
    (36, 20, "99339"),
    (37, 21, "9939"),
    (38, 20, "9229"),
    (39, 18, "912229"),
    (40, 17, "9122229"),
    (41, 17, "9122239"),
    (42, 17, "912239"),
    (43, 18, "92239"),
    (44, 18, "9239"),
    (45, 19, "999"),
]

ALAS_HADA = {
    "arriba": _simetrico(_HADA_ESPALDA, sombra=False),
    "abajo": _simetrico(_HADA_ESPALDA, sombra=False),
    "derecha": _filas(_HADA_LADO),
}

ALAS = {"ave": ALAS_AVE, "hada": ALAS_HADA}


def abrir_talla(talla):
    """Columnas que gana cada lado del torso en esta talla (0, 1 o 2)."""
    cols = TALLAS[talla.clave]["duplicar_columnas"]
    return max(sum(1 for c in cols if c < 0), sum(1 for c in cols if c > 0))


def capa_alas(talla, tipo, mat, mat2):
    """Capa de 12 frames con alas ancladas a los hombros."""
    plantilla = ALAS[tipo]
    abrir = abrir_talla(talla)
    frames = []
    for f in range(1, talla.n_frames + 1):
        lab = talla.labs[f]
        hombros = int(np.nonzero((lab == "t").any(axis=1))[0].min())
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        pintar_plantilla(capa, plantilla, talla.direccion(f), hombros - HOMBROS_QUIETO, mat, mat2,
                         x0=X0_ALAS, abrir=abrir)
        frames.append(capa)
    return frames
