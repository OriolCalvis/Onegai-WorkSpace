"""ESTETICA-01 — Las armas grandes y como se coge cada una, como en NosTale.

Oriol (07/10/2026): «las armas en la mano del personaje no se ven nada epicas [...] hay
que hacer un poco mas grande todas las armas, porque son demasiado finas y se pierden», y
«animando las armas y dandoles un poco mas de juego». Cada forma se dibuja una vez, de
frente y en vertical (actores_armas, actores_formas); aqui se dobla (doblar(): el doble de
grande con el contorno de un pixel) y se pone como se lleva ese tipo de arma.

Como se coge cada una sale de los videos de NosTale que paso Oriol (lamina
como_se_coge_cada_arma.png, sesiones/2026-10-06-claude-estetica.md) y de sus
correcciones del 07/10: las dagas con la hoja hacia atras como el Asesino; el espadon,
si va detras, con la punta abajo, y delante, abajo en reposo; la cuerda del arco hacia
atras; y las armas de asta nunca rectas como un baston. Quieto y andando no se llevan
igual (en NosTale la espada descansa en diagonal con la punta en el suelo y al correr va
detras): los frames de andar (A, quieto, B) usan el gesto de andar y los de reposo
(reposo1, reposo2; actores_partes.paso_de) el de quieto.

Un gesto pone la maestra (al doble, vertical) en una vista ("abajo" de frente, "derecha"
de perfil; las otras dos son su espejo):

  colgar   lo principal por debajo del puno, inclinado hacia `sentido` (-1 izquierda, +1
           derecha): de frente, -1 es hacia fuera (la mano derecha cae a la izquierda de
           la imagen); de perfil, +1 es hacia delante
  alzar    lo principal por encima del puno, la punta inclinada hacia `sentido`
  tender   casi horizontal: lo principal hacia `sentido`, bajando k filas por columna
           (k < 0: subiendo)

k es la inclinacion (columnas por fila; filas por columna en tender) de cada paso: el arma
se mece al andar y un poco al respirar. actores_armas.formas_en_talla acorta la maestra lo
justo para que todo quepa en la celda de cada talla.
"""

import math

import numpy as np

# tonos por material: (luz, base, sombra, contorno, contorno suave)
MATERIALES = [("1", "2", "3", "4", "5"), ("6", "7", "8", "9", None), ("a", "b", "c", "d", "e"),
              ("f", "g", "h", "i", "j")]
CONTORNOS = {"4": 0, "5": 0, "9": 1, "d": 2, "e": 2, "i": 3, "j": 3}
CONTORNO_DE = {0: "4", 1: "9", 2: "d", 3: "i"}
ANCLAS = "@&$"
RELLENO_ANCLA = {"@": "7", "&": "2", "$": "b"}


def _material(c):
    for i, m in enumerate(MATERIALES):
        if c in m:
            return i
    return {"w": 0, "&": 0, "@": 1, "$": 2}.get(c)


def _pad(filas):
    ancho = max(len(f) for f in filas)
    return [f.ljust(ancho, ".") for f in filas]


def _recorta(filas):
    while filas and not filas[0].strip("."):
        filas = filas[1:]
    while filas and not filas[-1].strip("."):
        filas = filas[:-1]
    while all(f[:1] == "." for f in filas):
        filas = [f[1:] for f in filas]
    while all(f[-1:] == "." for f in filas):
        filas = [f[:-1] for f in filas]
    return filas


# --- el doble ----------------------------------------------------------------------------------

def doblar(filas):
    """La rejilla al doble con el contorno de un pixel: el relleno se dobla; un pixel de
    contorno solo sigue siendo contorno por los lados que dan al vacio; por dentro toma el
    tono de su vecino. El ancla (el puno) queda en un solo pixel."""
    g = _pad(filas)
    alto, ancho = len(g), len(g[0])

    def en(r, c):
        return g[r][c] if 0 <= r < alto and 0 <= c < ancho else "."

    out = [["."] * (2 * ancho) for _ in range(2 * alto)]
    for r in range(alto):
        for c in range(ancho):
            ch = g[r][c]
            if ch == ".":
                continue
            if ch in ANCLAS:
                for dr in (0, 1):
                    for dc in (0, 1):
                        out[2 * r + dr][2 * c + dc] = RELLENO_ANCLA[ch]
                out[2 * r][2 * c] = ch
                continue
            if ch not in CONTORNOS:
                for dr in (0, 1):
                    for dc in (0, 1):
                        out[2 * r + dr][2 * c + dc] = ch
                continue
            for dr in (0, 1):
                for dc in (0, 1):
                    vert = en(r - 1, c) if dr == 0 else en(r + 1, c)
                    hor = en(r, c - 1) if dc == 0 else en(r, c + 1)
                    if vert == "." or hor == ".":
                        out[2 * r + dr][2 * c + dc] = ch
                        continue
                    vecino = next((v for v in (hor, vert)
                                   if v not in CONTORNOS and v not in ANCLAS and v != "."), None)
                    out[2 * r + dr][2 * c + dc] = vecino or ch
    return perfilar(["".join(f) for f in out])


def perfilar(filas):
    """El borde de la silueta en el tono de contorno de su material: a los lados siempre, y
    arriba y abajo en lo principal (la punta de la hoja, la cabeza). Se lee mejor al lado
    del cuerpo."""
    g = [list(f) for f in _pad(filas)]
    alto, ancho = len(g), len(g[0])
    out = [list(f) for f in g]
    for r in range(alto):
        for c in range(ancho):
            ch = g[r][c]
            if ch == "." or ch in CONTORNOS or ch in ANCLAS:
                continue
            m = _material(ch)
            if m is None:
                continue
            lados = [(0, -1), (0, 1)] + ([(-1, 0), (1, 0)] if m == 0 else [])
            for dr, dc in lados:
                rr, cc = r + dr, c + dc
                if not (0 <= rr < alto and 0 <= cc < ancho) or g[rr][cc] == ".":
                    out[r][c] = CONTORNO_DE[m]
                    break
    return ["".join(f) for f in out]


# --- giros ----------------------------------------------------------------------------------

def _desplazamiento(d):
    return int(d) if d >= 0 else -int(-d + 0.999)


def cizalla(filas, eje, k, hacia=1):
    """Cada fila r corrida (r - eje) * k columnas hacia `hacia` (1 derecha, -1 izquierda; las
    de encima del eje, al reves): el arma girada sobre la fila `eje`."""
    desp = [_desplazamiento((r - eje) * k) * hacia for r in range(len(filas))]
    m, M = min(desp), max(desp)
    return _recorta(["." * (d - m) + f + "." * (M - d) for f, d in zip(_pad(filas), desp)])


def tumbada(filas, eje, k=0.5):
    """La rejilla traspuesta (lo de debajo del puno hacia la derecha, la luz arriba) y cada
    columna c bajada (c - eje) * k filas (k < 0: subida): casi horizontal."""
    base = _pad(filas)
    alto, ancho = len(base), len(base[0])
    nueva = [[base[col][f] for col in range(alto)] for f in range(ancho)]
    desp = [_desplazamiento((c - eje) * k) for c in range(alto)]
    m, M = min(desp), max(desp)
    out = [["."] * alto for _ in range(ancho + (M - m))]
    for c in range(alto):
        for r in range(ancho):
            if nueva[r][c] != ".":
                out[r + desp[c] - m][c] = nueva[r][c]
    return _recorta(["".join(f) for f in out])


def espejo(filas):
    return [f[::-1] for f in _pad(filas)]


def fila_ancla(filas):
    return next(i for i, f in enumerate(filas) if any(c in f for c in ANCLAS))


# --- giro de verdad (RotSprite) ----------------------------------------------------------------
# La cizalla tuerce la cabeza de un hacha o de un martillo (un rombo en vez de un filo); para
# las inclinaciones grandes se gira: la rejilla a 8x con scale2x (los bordes en diagonal se
# suavizan sin inventar colores), girada sobre el ancla y vuelta a 1x tomando el centro de
# cada celda. Despues se repasa el contorno (perfilar).

def _scale2x(g):
    """EPX/scale2x de una rejilla de caracteres (numpy)."""
    P = g
    A = np.vstack([g[:1], g[:-1]])     # arriba
    D = np.vstack([g[1:], g[-1:]])     # abajo
    C = np.hstack([g[:, :1], g[:, :-1]])  # izquierda
    B = np.hstack([g[:, 1:], g[:, -1:]])  # derecha
    e0 = np.where((C == A) & (C != D) & (A != B), A, P)
    e1 = np.where((A == B) & (A != C) & (B != D), B, P)
    e2 = np.where((D == C) & (D != B) & (C != A), C, P)
    e3 = np.where((B == D) & (B != A) & (D != C), D, P)
    h, w = g.shape
    out = np.empty((2 * h, 2 * w), dtype=g.dtype)
    out[0::2, 0::2], out[0::2, 1::2], out[1::2, 0::2], out[1::2, 1::2] = e0, e1, e2, e3
    return out


_GIRADAS = {}


def rotar(filas, grados):
    """La rejilla girada `grados` sobre el ancla (positivo: como las agujas del reloj en la
    pantalla, la punta de arriba hacia la derecha). Devuelve filas con su ancla."""
    clave = (tuple(filas), round(grados, 2))
    if clave in _GIRADAS:
        return list(_GIRADAS[clave])
    filas = _pad(filas)
    ay = fila_ancla(filas)
    ax = next(filas[ay].index(c) for c in ANCLAS if c in filas[ay])
    g = np.array([list(f) for f in filas])
    ancla = g[ay, ax]
    g[ay, ax] = RELLENO_ANCLA[ancla]
    S = 8
    big = g
    for _ in range(3):
        big = _scale2x(big)
    H, W = big.shape
    cy, cx = ay * S + S / 2, ax * S + S / 2
    t = math.radians(grados)
    ct, st = math.cos(t), math.sin(t)
    ex = np.array([0, W, 0, W]) - cx
    ey = np.array([0, 0, H, H]) - cy
    xs, ys = cx + ex * ct - ey * st, cy + ex * st + ey * ct
    x0, x1 = math.floor(xs.min() / S) - 1, math.ceil(xs.max() / S) + 1
    y0, y1 = math.floor(ys.min() / S) - 1, math.ceil(ys.max() / S) + 1
    oy, ox = np.mgrid[y0:y1, x0:x1]
    dy, dx = oy * S + S / 2 - cy, ox * S + S / 2 - cx
    sx = np.floor(cx + dx * ct + dy * st).astype(int)
    sy = np.floor(cy - dx * st + dy * ct).astype(int)
    dentro = (sx >= 0) & (sx < W) & (sy >= 0) & (sy < H)
    out = np.full(oy.shape, ".", dtype=g.dtype)
    out[dentro] = big[sy[dentro], sx[dentro]]
    out[int(math.floor(cy / S)) - y0, int(math.floor(cx / S)) - x0] = ancla
    res = _recorta(perfilar(["".join(f) for f in out]))
    _GIRADAS[clave] = tuple(res)
    return res


# --- como se coge cada tipo ------------------------------------------------------------------
# grupo -> estado -> vista -> (gesto, sentido, k por paso, espejo de la maestra)
# Los k de "andando" son los de (paso A, quieto, paso B); los de "quieto", (reposo1, reposo2).
# "espejo" da la vuelta a la maestra antes del gesto (el filo del hacha, la cuerda del arco,
# la hoja de la guadana hacia donde toca).

ESTADOS = {"andando": ("A", "quieto", "B"), "quieto": ("reposo1", "reposo2")}
PASOS = ("A", "quieto", "B", "reposo1", "reposo2")


def _p(gesto, sentido, ks, en_espejo=False, giro=0.0, hondo=0.0):
    """giro y hondo (grados): solo para las armas con volumen (VOLUMEN, actores_volumen): el
    arma girada sobre su eje (la cabeza enseña la cara) y echada hacia el que mira."""
    return (gesto, sentido, tuple(ks), en_espejo, giro, hondo)


GRUPOS = {
    # Espadachin SP1: quieta, baja y en diagonal hacia fuera, la punta casi en el suelo;
    # corriendo, detras y casi horizontal a la altura de la cadera.
    "espada": {
        "quieto": {"abajo": _p("colgar", -1, (1.2, 1.1)), "derecha": _p("colgar", 1, (0.6, 0.5))},
        "andando": {"abajo": _p("colgar", -1, (0.8, 1.0, 1.2)), "derecha": _p("tender", -1, (0.25, 0.3, 0.4))},
    },
    # Asesino: agarre invertido, la hoja sale por debajo del puno hacia atras (de frente se
    # ve subiendo junto al antebrazo, hacia fuera).
    "daga": {
        "quieto": {"abajo": _p("alzar", -1, (0.6, 0.5)), "derecha": _p("tender", -1, (-0.3, -0.25))},
        "andando": {"abajo": _p("alzar", -1, (0.4, 0.6, 0.8)), "derecha": _p("tender", -1, (-0.15, -0.3, -0.45))},
    },
    # Espadon: quieto, delante con la punta arriba (el Cruzado; con la punta en el suelo no
    # cabe en la celda: el puno va a la altura de la cadera); andando, detras con la punta
    # hacia abajo.
    "espadon": {
        "quieto": {"abajo": _p("alzar", -1, (0.12, 0.08)), "derecha": _p("alzar", 1, (0.3, 0.25))},
        "andando": {"abajo": _p("colgar", -1, (0.3, 0.4, 0.5)), "derecha": _p("tender", -1, (0.5, 0.6, 0.7))},
    },
    # Monje de batalla: la lanza horizontal a la cadera, la punta hacia fuera (de frente) o
    # hacia delante (de perfil). Nunca recta como un baston.
    "lanza": {
        "quieto": {"abajo": _p("tender", -1, (-0.05, 0.0)), "derecha": _p("tender", 1, (-0.05, 0.0))},
        "andando": {"abajo": _p("tender", -1, (-0.1, -0.05, 0.0)), "derecha": _p("tender", 1, (-0.1, -0.05, 0.0))},
    },
    # Segador: cruzada por delante, la hoja abajo junto al pie y el astil subiendo por detras
    # del otro hombro.
    "guadana": {
        "quieto": {"abajo": _p("colgar", -1, (0.9, 0.85), True), "derecha": _p("colgar", 1, (0.7, 0.65))},
        "andando": {"abajo": _p("colgar", -1, (0.8, 0.9, 1.0), True), "derecha": _p("colgar", 1, (0.6, 0.7, 0.8))},
    },
    # Bastones (Oriol, 07/10: «un poco mas recto», pero inclinado le gusta mas): algo mas
    # rectos que antes (hacia fuera de frente; de perfil, hacia delante quieto y hacia atras
    # andando). De frente, si pasaria por la cara, la cabeza lo tapa (TAPA_LA_CABEZA).
    "baston": {
        "quieto": {"abajo": _p("alzar", -1, (0.35, 0.32)), "derecha": _p("alzar", 1, (0.28, 0.25))},
        "andando": {"abajo": _p("alzar", -1, (0.3, 0.35, 0.4)), "derecha": _p("alzar", -1, (0.22, 0.28, 0.34))},
    },
    # Varas, cetros y teas (cortos): en la mano, un poco inclinados (andando de perfil, atras).
    "corto": {
        "quieto": {"abajo": _p("alzar", -1, (0.3, 0.25)), "derecha": _p("alzar", 1, (0.25, 0.2))},
        "andando": {"abajo": _p("alzar", -1, (0.2, 0.3, 0.4)), "derecha": _p("alzar", -1, (0.15, 0.25, 0.35))},
    },
    # Arco: vertical al costado, la cuerda hacia atras (de perfil, detras; de frente, hacia
    # el cuerpo).
    "arco": {
        "quieto": {"abajo": _p("alzar", -1, (0.0, 0.04)), "derecha": _p("alzar", 1, (0.0, 0.04), True)},
        "andando": {"abajo": _p("alzar", -1, (0.0, 0.04, 0.08)), "derecha": _p("alzar", 1, (-0.04, 0.0, 0.04), True)},
    },
    # Explorador: la ballesta horizontal a la cintura.
    "ballesta": {
        "quieto": {"abajo": _p("tender", -1, (0.0, 0.05)), "derecha": _p("tender", 1, (0.0, 0.05))},
        "andando": {"abajo": _p("tender", -1, (-0.05, 0.0, 0.05)), "derecha": _p("tender", 1, (-0.05, 0.0, 0.05))},
    },
    # Gran hacha y martillo (Oriol, 07/10: «la cogeria con las dos manos hacia el hombro y
    # para correr la dejaria caer hacia atras como un espadon»; «le falta perspectivas»): con
    # volumen (VOLUMEN); quietos, de frente cruzados por delante del cuerpo hacia el hombro
    # contrario con la cabeza junto a la del personaje, y de perfil al hombro con la cabeza
    # detras; andando, de frente igual y de perfil caidos hacia atras, arrastrando la cabeza.
    "hombro": {
        "quieto": {"abajo": _p("alzar", 1, (1.2, 1.15), False, 30, -15),
                   "derecha": _p("alzar", -1, (0.5, 0.47), False, 35)},
        "andando": {"abajo": _p("alzar", 1, (1.15, 1.2, 1.25), False, 30, -15),
                    "derecha": _p("colgar", -1, (1.6, 1.8, 2.0), False, 35)},
    },
    # Hachas de una mano (Oriol, 07/10: se entendian fatal, «le falta perspectivas»): con
    # volumen y la hoja de barba (MAESTRAS_2X); la cabeza arriba, junto al hombro y hacia
    # fuera (el filo se lee contra el fondo), cogidas por el final del mango.
    "hacha": {
        "quieto": {"abajo": _p("alzar", -1, (0.3, 0.27), False, 20),
                   "derecha": _p("alzar", -1, (0.45, 0.42), False, 20)},
        "andando": {"abajo": _p("alzar", -1, (0.2, 0.3, 0.4), False, 20),
                    "derecha": _p("alzar", -1, (0.45, 0.55, 0.65), False, 20)},
    },
    # Mazas y macanas de una mano: abajo junto a la pierna, la cabeza hacia el suelo
    # (como el revolver del Cazador de demonios); andando se mecen como un pendulo.
    "cabeza_abajo": {
        "quieto": {"abajo": _p("colgar", -1, (0.15, 0.1)), "derecha": _p("colgar", 1, (0.1, 0.05), True)},
        "andando": {"abajo": _p("colgar", -1, (0.05, 0.15, 0.25)), "derecha": _p("colgar", 1, (-0.25, 0.0, 0.25), True)},
    },
    # Latigos (Oriol, 07/10, opcion B): el mango en el puno y la tralla colgando hasta el
    # suelo (MAESTRAS_2X); andando, de perfil se arrastra detras.
    "latigo": {
        "quieto": {"abajo": _p("colgar", -1, (0.0, 0.05)), "derecha": _p("colgar", -1, (0.0, 0.05))},
        "andando": {"abajo": _p("colgar", -1, (0.0, 0.1, 0.2)), "derecha": _p("colgar", -1, (0.2, 0.3, 0.4))},
    },
    # SP-02 (el Arquero): los revolveres del Cazador de demonios, abajo junto a la pierna,
    # apuntando al suelo y un poco hacia fuera (de perfil, hacia delante); andando se mecen.
    "revolver": {
        "quieto": {"abajo": _p("colgar", -1, (0.35, 0.3)), "derecha": _p("colgar", 1, (0.35, 0.3))},
        "andando": {"abajo": _p("colgar", -1, (0.25, 0.35, 0.45)), "derecha": _p("colgar", 1, (0.1, 0.3, 0.5))},
    },
    # SP-02: el canon de mano del Canonero, al hombro, con volumen como el martillo: de frente
    # cruzado hacia el hombro contrario con la boca junto a la cabeza, de perfil al hombro con la
    # boca detras; andando tambien al hombro (no se arrastra).
    "canon": {
        "quieto": {"abajo": _p("alzar", 1, (1.2, 1.15), False, 30, -15),
                   "derecha": _p("alzar", -1, (0.5, 0.47), False, 35)},
        "andando": {"abajo": _p("alzar", 1, (1.15, 1.2, 1.25), False, 30, -15),
                    "derecha": _p("alzar", -1, (0.45, 0.5, 0.55), False, 35)},
    },
    # Hondas: colgando junto a la pierna.
    "colgando": {
        "quieto": {"abajo": _p("colgar", -1, (0.0, 0.12)), "derecha": _p("colgar", 1, (0.34, 0.42))},
        "andando": {"abajo": _p("colgar", -1, (-0.25, 0.0, 0.25)), "derecha": _p("colgar", 1, (0.34, 0.5, 0.67))},
    },
}

# Por donde se coge: grupo -> vista -> (filas de mango entre el ancla y lo principal, filas
# de mango por el otro lado), en la maestra al doble (None: las que tenga), con el ancla en
# medio del mango (doblar() la deja en el borde izquierdo y el mango asomaba por un lado del
# puno).
AGARRE = {
    "baston": {"abajo": (None, None), "derecha": (None, None)},  # solo el ancla en medio
    # gran hacha y martillo: por el final del mango; de frente, con el mango largo para que la
    # cabeza pase del hombro y no tape la cara
    "hombro": {"abajo": (18, 2), "derecha": (None, 2)},
    "hacha": {"abajo": (None, 2), "derecha": (None, 2)},
}

# Oriol (07/10): «me mola que superpongas el pj a la arma [...] pero solo en los frames
# frontales si fuera necesario y solo el trozo justo». De frente, si el arma pasaria por la
# cara, la cabeza la tapa (solo lo que cae sobre la cabeza); asi no hay que inclinarla mas.
TAPA_LA_CABEZA = {"baston"}
# armas sin cuerda dibujada (Oriol, 07/10: «el arco si no le dibujas la cuerda, mejor»)
SIN_CUERDA = {"arco"}
# lo que se coge con las dos manos: de frente el otro puno tambien lo tapa
DOS_MANOS = set()
# de frente, el antebrazo tapa la empunadura que asoma por encima del puno (solo el trozo justo)
MANGO_TRAS_EL_BRAZO = set()
# lo que se gira de verdad (rotar) en vez de cizallarse al inclinarlo: las cabezas grandes
GIRAR = set()
# lo que tiene volumen (actores_volumen): cada vista con su perspectiva
VOLUMEN = {"hombro", "hacha", "canon"}


GRUPO_DE = {}
for _g, _fams in {
    "espada": ("espada", "espada_corta", "sable", "estoque"),
    "daga": ("daga", "cuchillo", "estilete"),
    "espadon": ("mandoble",),
    "lanza": ("lanza", "pica", "tridente", "jabalinas"),
    "guadana": ("guadana",),
    "baston": ("baston", "baston_herrado"),
    "corto": ("vara", "cetro", "tea", "bumeran"),  # el bumeran, en alto junto al costado
    "arco": ("arco",),
    # la ballesta y las armas de fuego largas (SP-02: como el lanzacohetes del Destructor, a la cintura)
    "ballesta": ("ballesta", "escopeta", "rifle", "lanzagranadas", "lanzallamas"),
    # el revolver y la pistola magica del Mago (SP-02), abajo junto a la pierna
    "revolver": ("revolver", "pistola"),
    "canon": ("canon",),
    "hombro": ("gran_hacha", "martillo"),
    "hacha": ("hacha", "hacha_mano"),
    "cabeza_abajo": ("maza", "macana"),
    "colgando": ("honda",),
    "latigo": ("latigo",),
}.items():
    for _f in _fams:
        GRUPO_DE[_f] = _g

# como esta dibujada la maestra: "abajo" si lo principal cuelga del puno, "arriba" si sube
ORIENTACION = {f: "abajo" for f in ("espada", "espada_corta", "sable", "estoque", "mandoble", "daga",
                                    "cuchillo", "estilete", "honda", "latigo", "ballesta",
                                    "escopeta", "rifle", "lanzagranadas", "revolver", "pistola",
                                    "lanzallamas")}
ORIENTACION.update({f: "arriba" for f in ("hacha", "hacha_mano", "gran_hacha", "maza", "macana", "martillo",
                                         "lanza", "pica", "tridente", "jabalinas", "baston", "baston_herrado",
                                         "vara", "cetro", "guadana", "arco", "tea", "canon", "bumeran")})

# formas que se cogen como otra familia (las unicas de actores_formas, que las suma aqui)
FAMILIA_DE = {}


def tapada_por_la_cabeza(familia, vista):
    """True si en esa vista la cabeza tapa el arma cuando se cruzan (TAPA_LA_CABEZA: de frente)."""
    return vista == "abajo" and grupo_de(familia) in TAPA_LA_CABEZA


def familia_base(familia):
    """'sable_mitico' -> 'sable' (los escalones se cogen como su forma); 'u_...' -> la de su
    familia (FAMILIA_DE)."""
    if familia in FAMILIA_DE:
        return FAMILIA_DE[familia]
    for paso in ("_raro", "_epico", "_mitico"):
        if familia.endswith(paso):
            return familia[: -len(paso)]
    return familia


def grupo_de(familia):
    return GRUPO_DE.get(familia_base(familia))


def pose_de(familia):
    """{estado: {vista: (gesto, sentido, ks, espejo)}} o None (mano izquierda, oficios)."""
    g = grupo_de(familia)
    return GRUPOS[g] if g else None


# --- el puno ------------------------------------------------------------------------------

MANGO = set("6789@")  # lo que pinta el material 2: empunadura, astil


def _es_mango(fila):
    hay = [c for c in fila if c != "."]
    return bool(hay) and all(c in MANGO for c in hay)


def centrar_ancla(filas):
    """El ancla en medio de la empunadura (del tramo de material 2 de su fila)."""
    filas = _pad(filas)
    ay = fila_ancla(filas)
    f = filas[ay]
    ax = next(f.index(c) for c in ANCLAS if c in f)
    a = ax
    while a > 0 and f[a - 1] in MANGO:
        a -= 1
    b = ax
    while b < len(f) - 1 and f[b + 1] in MANGO:
        b += 1
    centro = (a + b) // 2
    if centro == ax:
        return filas
    nueva = list(f)
    nueva[ax], nueva[centro] = RELLENO_ANCLA[f[ax]], f[ax]
    return filas[:ay] + ["".join(nueva)] + filas[ay + 1:]


def agarrar(filas, principal, sobre=None, bajo=None):
    """La maestra cogida por otro sitio: deja `sobre` filas de mango entre el ancla y lo
    principal (`principal`: "arriba" o "abajo" del ancla) y `bajo` por el otro lado (antes
    del pomo); quita las de junto al ancla."""
    filas = list(filas)
    for lado, quedan in (("principal", sobre), ("otro", bajo)):
        if quedan is None:
            continue
        ay = fila_ancla(filas)
        paso = -1 if (principal == "arriba") == (lado == "principal") else 1
        r, mango = ay + paso, []
        while 0 <= r < len(filas) and _es_mango(filas[r]):
            mango.append(r)
            r += paso
        if len(mango) < quedan and mango:  # alargar: se repite la fila de mango de junto al ancla
            r0 = mango[0]
            extra = [filas[r0]] * (quedan - len(mango))
            filas = filas[:r0] + extra + filas[r0:] if paso < 0 else filas[:r0 + 1] + extra + filas[r0 + 1:]
            continue
        sobran = set(mango[: max(0, len(mango) - quedan)])
        filas = [f for i, f in enumerate(filas) if i not in sobran]
    return filas


def sin_cuerda(filas):
    """La maestra sin lo del material 2 que no sea el puno (la cuerda del arco)."""
    filas = _pad(filas)
    ay = fila_ancla(filas)
    ax = next(filas[ay].index(c) for c in ANCLAS if c in filas[ay])
    return _recorta(["".join(c if (c not in "6789" or (abs(r - ay) <= 1 and abs(j - ax) <= 1)) else "."
                             for j, c in enumerate(f)) for r, f in enumerate(filas)])


# Maestras dibujadas ya al doble (en vez de doblar() el dibujo pequeno): las hachas de una
# mano, con la hoja de barba (filo largo, cuello estrecho hasta el ojo de metal y el astil
# asomando por arriba; las raras con virolas, las epicas con gema, las miticas con runas). Al
# doble, la hoja en forma de D de antes se leia como una piedra. Y el latigo: el mango con su
# pomo y su virola y la tralla cayendo hasta el suelo.
MAESTRAS_2X = {
    'hacha': [
        '.............9999..',
        '.............9779..',
        '.44..........9779..',
        '41244.......433334.',
        '412224444...433334.',
        '412222222444333334.',
        '412222222233333334.',
        '412222222233333334.',
        '412222222233333334.',
        '412222222233333334.',
        '412222222244333334.',
        '4122222234..433334.',
        '412222234...433334.',
        '412222234....9779..',
        '41222234.....9779..',
        '41222234.....9779..',
        '.412234......9779..',
        '.412234......9779..',
        '.41234.......9779..',
        '..4134.......9779..',
        '..414........9779..',
        '...4.........9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9@79..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '............dbbbbd.',
        '............dddddd.',
    ],
    'hacha_raro': [
        '.............9999..',
        '.............9779..',
        '.44..........9779..',
        '41244.......dbbbbd.',
        '412224444...dbbbbd.',
        '412222222444333334.',
        '412222222233333334.',
        '412222222233333334.',
        '412222222233333334.',
        '412222222233333334.',
        '412222222244333334.',
        '4122222234..dbbbbd.',
        '412222234...dbbbbd.',
        '412222234....9779..',
        '41222234.....9779..',
        '41222234.....9779..',
        '.412234......9779..',
        '.412234......9779..',
        '.41234.......9779..',
        '..4134.......9779..',
        '..414........9779..',
        '...4.........9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9@79..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '............dbbbbd.',
        '............dddddd.',
    ],
    'hacha_mano': [
        '..........9999..',
        '.44.......9779..',
        '4134.....433334.',
        '4122444..433334.',
        '412222244333334.',
        '412222233333334.',
        '412222233333334.',
        '412222233333334.',
        '412222234333334.',
        '41222234.433334.',
        '4122234..433334.',
        '4122234...9779..',
        '412234....9779..',
        '.41234....9779..',
        '.4134.....9779..',
        '..44......9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9@79..',
        '..........9779..',
        '..........9779..',
        '.........dbbbbd.',
        '.........dddddd.',
    ],
    'hacha_mano_raro': [
        '..........9999..',
        '.44.......9779..',
        '4134.....dbbbbd.',
        '4122444..dbbbbd.',
        '412222244333334.',
        '412222233333334.',
        '412222233333334.',
        '412222233333334.',
        '412222234333334.',
        '41222234.dbbbbd.',
        '4122234..dbbbbd.',
        '4122234...9779..',
        '412234....9779..',
        '.41234....9779..',
        '.4134.....9779..',
        '..44......9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9@79..',
        '..........9779..',
        '..........9779..',
        '.........dbbbbd.',
        '.........dddddd.',
    ],
    'hacha_mano_epico': [
        '..........9999..',
        '.44.......9779..',
        '4134.....dbbbbd.',
        '4122444..dbbbbd.',
        '412222244333334.',
        '41222223333gg34.',
        '41222223333gg34.',
        '412222233333334.',
        '412222234333334.',
        '41222234.dbbbbd.',
        '4122234..dbbbbd.',
        '4122234...9779..',
        '412234....9779..',
        '.41234....9779..',
        '.4134.....9779..',
        '..44......9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9@79..',
        '..........9779..',
        '..........9779..',
        '.........dbbbbd.',
        '.........dddddd.',
    ],
    'hacha_mano_mitico': [
        '..........9999..',
        '.44.......9779..',
        '4134.....dbbbbd.',
        '4122444..dbbbbd.',
        '412222244333334.',
        '41222223333gg34.',
        '41222223333gg34.',
        '412w22233333334.',
        '412w22234333334.',
        '41222234.dbbbbd.',
        '4122234..dbbbbd.',
        '4122w34...9779..',
        '4122w4....9779..',
        '.41234....9779..',
        '.4134.....9779..',
        '..44......9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9779..',
        '..........9@79..',
        '..........9779..',
        '..........9779..',
        '.........dggggd.',
        '.........dddddd.',
    ],
    'u_hacha_del_cuerno_roto': [
        '.4...........9999..',
        '414..........9779..',
        '41244........9779..',
        '412234......dbbbbd.',
        '.412234.....dbbbbd.',
        '.412222444..433334.',
        '.41222222244333334.',
        '.4122222223333gg34.',
        '41222222223333gg34.',
        '412222222233333334.',
        '412222222233333334.',
        '412222222244333334.',
        '4122222234..dbbbbd.',
        '412222234...dbbbbd.',
        '41222234.....9779..',
        '41222234.....9779..',
        '.412234......9779..',
        '.412234......9779..',
        '.412234......9779..',
        '..4134.......9779..',
        '..4134.......9779..',
        '...414.......9779..',
        '...44........9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '.............9@79..',
        '.............9779..',
        '.............9779..',
        '.............9779..',
        '............dbbbbd.',
        '............dddddd.',
    ],
    'latigo': [
        '.............dddd...',
        '.............dbbd...',
        '.............9779...',
        '.............9779...',
        '.............9@79...',
        '.............9779...',
        '.............dbbd...',
        '.............123....',
        '.............124....',
        '.............13.....',
        '.............13.....',
        '............423.....',
        '...........4224.....',
        '..........4224......',
        '..........124.......',
        '........4123........',
        '.......42234........',
        '......4224..........',
        '....41224...........',
        '...42234............',
        '411224..............',
        '43334...............',
    ],
}


def maestra(frente, familia, gesto, en_espejo=False, vista_de=None):
    """La forma de frente al doble, con lo principal hacia donde lo pide el gesto (debajo
    del puno para colgar y tender, encima para alzar), cogida como su tipo en esa vista
    (AGARRE) y en espejo si se pide."""
    m = list(MAESTRAS_2X[familia]) if familia in MAESTRAS_2X else doblar(frente)
    quiere = "arriba" if gesto == "alzar" else "abajo"
    if ORIENTACION.get(familia_base(familia), "abajo") != quiere:
        m = list(reversed(m))
    agarre = AGARRE.get(grupo_de(familia), {}).get(vista_de)
    if agarre:
        m = agarrar(centrar_ancla(m), quiere, *agarre)
    if grupo_de(familia) in SIN_CUERDA:
        m = sin_cuerda(m)
    return espejo(m) if en_espejo else list(m)


def a_dos_manos(familia):
    """True si se coge con las dos manos: de frente, los dos punos lo tapan (DOS_MANOS)."""
    return grupo_de(familia) in DOS_MANOS


def con_volumen(familia):
    """True si el arma se dibuja con volumen en cada vista (VOLUMEN)."""
    return grupo_de(familia) in VOLUMEN


def gira(familia):
    """True si el arma se gira (rotar) en vez de cizallarse al inclinarla (GIRAR)."""
    return grupo_de(familia) in GIRAR


def vista(m, gesto, sentido, k, girar=False):
    """La maestra (ya orientada) puesta con ese gesto e inclinacion: filas listas para pintar.
    girar: rotada de verdad (rotar) en vez de cizallada; la cabeza de un hacha no se deforma."""
    ay = fila_ancla(m)
    if girar and gesto in ("colgar", "alzar") and k:
        grados = math.degrees(math.atan(abs(k)))
        if gesto == "alzar":  # la punta hacia `sentido` (k < 0: hacia el otro lado)
            return rotar(m, grados * (sentido if k >= 0 else -sentido))
        return rotar(m, -grados * (sentido if k >= 0 else -sentido))
    if gesto == "colgar":
        return cizalla(m, ay, abs(k), hacia=sentido if k >= 0 else -sentido) if k else _recorta(list(m))
    if gesto == "alzar":
        return cizalla(m, ay, abs(k), hacia=-sentido if k >= 0 else sentido) if k else _recorta(list(m))
    if gesto == "tender":
        horizontal = tumbada(m, ay, k)
        return horizontal if sentido > 0 else espejo(horizontal)
    raise ValueError(gesto)
