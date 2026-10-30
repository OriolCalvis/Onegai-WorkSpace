"""Esbossos del Senyor de les armes: tres propostes (no van al repo fins que l'Oriol en triï una).

1. El Duc: casaca negra tancada de coll alt amb els cantells d'or, espatlleres d'or (la dreta, gran), el
   cinturó ample d'or amb la placa de gemma fosca, peces d'or i tires vermelles als malucs, la capa vermella
   que neix de la cintura, les línies de mana vermell als braços, calces negres i botes negres amb la greva
   d'or angular. Cabell blanc recollit, ulls violeta, la diadema amb el robí. Les fulles suren als costats.
2. El Sobirà sense rostre: yelmo d'or tancat amb la corona de puntes d'espasa i l'escletxa vermella que brilla,
   placa negra amb els cantells d'or, espatlleres grans, el tabard vermell amb la corona d'espases, la capa
   llarga vermella i les fulles espectrals vermelles que li volten la cintura.
3. El Rei de la Forja: armadura de ferro fosc amb les esquerdes d'or fos que brillen, un guantellet d'or enorme
   al braç de l'arma, la mitja capa vermella cremada, la corona de fulles trencades, cabell i barba de coure.
"""
import numpy as np

import actores_cascos as cascos
import actores_equipo as equipo
from actores_materiales import MATERIALES
from actores_pintura import Material as M
from actores_pintura import _tramo_torso, es_contorno, filas_torso, pintar, top_pie
from actores_trajes import (DELANTE, BALANCEO, _arte, _caja, _cuello, _espejo_x, _pinta, _plantilla, _sombrea,
                            cuello_alto, hombrera_grande, tabardo_largo, drapeado, EMBLEMAS)
from actores_trajes_mago import _vacio

_PALETAS = [
    # 1. El Duc
    M("negro_duc", "#53566a", "#2b2c36", "#1a1b22", "#0d0c11", "#08070b"),
    M("oro_duc", "#fff3b8", "#f5c84a", "#b98a22", "#5e430f", "#3f2d0a"),
    M("rojo_duc", "#ff6064", "#c8162d", "#80101f", "#40070f", "#2b050a"),
    M("mana_duc", "#ffe0d8", "#ff4040", "#c5172c", "#6c0b16", "#47080f"),
    M("guante_duc", "#3d3b44", "#1f1e24", "#141317", "#0a090b", "#050405"),
    M("hoja_duc", "#f6f9ff", "#cfd8ea", "#97a3ba", "#4a5162", "#323743"),
    M("gema_duc", "#ff8c8c", "#a0102a", "#5e0718", "#2f040c", "#1f0308"),
    # 2. El Sobira sense rostre
    M("placa_sobira", "#6f7386", "#3c3f4c", "#25272f", "#121318", "#0a0b0e"),
    M("oro_sobira", "#f9e092", "#d6a83f", "#986a1d", "#4c340e", "#33230a"),
    M("tabard_sobira", "#e8474f", "#a9131f", "#6c0a15", "#36050b", "#240307"),
    M("espectre_sobira", "#ffd9d6", "#ff5c5c", "#d92c3d", "#8c1124", "#5d0b18", alfa=200),
    M("visera_sobira", "#ffd2c4", "#ff3a2e", "#b8141f", "#5c0a10", "#3d070b"),
    # 3. El Rei de la Forja
    M("ferro_forja", "#77716f", "#47423f", "#2d2927", "#161413", "#0e0c0b"),
    M("or_fos", "#fffbd2", "#ffd640", "#ff8e1e", "#a3420d", "#6c2c08"),
    M("brasa_forja", "#ff7d48", "#cc361d", "#8c2113", "#461009", "#2f0b06"),
    M("cuir_forja", "#8a6a55", "#5a4334", "#3c2c22", "#1e1611", "#140e0b"),
]
_PISADAS = [m.nombre for m in _PALETAS if m.nombre in MATERIALES and MATERIALES[m.nombre].tono != m.tono]
if _PISADAS:
    raise ValueError(f"senor: estas paletas ya existen: {_PISADAS}")
_TONOS_AJENOS = {tuple(v) for n, m in MATERIALES.items() if n not in {p.nombre for p in _PALETAS}
                 for t, v in m.tono.items() if t != "X"}
_REPETIDOS = [m.nombre for m in _PALETAS for t, v in m.tono.items() if t != "X" and tuple(v) in _TONOS_AJENOS]
if _REPETIDOS:
    raise ValueError(f"senor: tonos que ya son de otra paleta: {sorted(set(_REPETIDOS))}")
MATERIALES.update({m.nombre: m for m in _PALETAS})


# --- utilitats ------------------------------------------------------------------------------------

def _torso(ctx):
    return _caja(ctx["lab"], ("t",))


def _brazos(ctx):
    """Les caixes dels dos braços que es veuen: {etiqueta: caixa}."""
    out = {}
    for e in ("a", "A"):
        c = _caja(ctx["lab"], (e,))
        if c is not None:
            out[e] = c
    return out


def _fuera_cuerpo(ctx, y, x):
    """True si (y, x) es dins la cel·la i el cos no hi és (per pintar el que va per darrere)."""
    lab = ctx["lab"]
    return 0 <= y < lab.shape[0] and 0 <= x < lab.shape[1] and not ctx["alfa"][y, x]


def _pinta_detras(ctx, tonos, m):
    """Pinta només on no hi ha cos (el que va per darrere: la capa de cintura de cara)."""
    for (y, x), t in tonos.items():
        if _fuera_cuerpo(ctx, y, x) and (y, x) not in ctx["cubre"]:
            pintar(ctx, y, x, m, t)
            ctx["cubre"][(y, x)] = m


# --- 1. El Duc ------------------------------------------------------------------------------------

def tancat_duc(oro):
    """La casaca tancada: el cantell d'or que la tanca pel mig (de cara) i dos botons d'or rodons al
    pit, sota el coll; d'esquena, la costura del mig en ombra. De perfil, el cantell davanter d'or."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][oro]
        y0, y1 = filas_torso(ctx)
        if y0 is None:
            return
        if d == "abajo":
            tr = _tramo_torso(lab, ctx["tono"], y0 + 4)
            if not tr:
                return
            cx = (tr[0] + tr[1]) // 2
            for y in range(y0 + 1, y1):
                if lab[y, cx] == "t":
                    pintar(ctx, y, cx, m, "H" if y < y0 + 4 else "s")
            for (yy, xx) in ((y0 + 2, cx - 2), (y0 + 2, cx + 2)):
                if lab[yy, xx] == "t":
                    pintar(ctx, yy, xx, m, "H")
        elif d in ("derecha", "izquierda"):
            for y in range(y0 + 1, y1):
                tr = _tramo_torso(lab, ctx["tono"], y)
                if not tr:
                    continue
                x = tr[1] if d == "derecha" else tr[0]
                if lab[y, x] == "t":
                    pintar(ctx, y, x, m, "s")
    return fn


def linias_mana(mana):
    """Les línies de mana vermell: una per la part de fora de cada braç, de l'espatlla al puny, i una que
    creua el pit sota les espatlleres (de cara i d'esquena)."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][mana]
        torso = _torso(ctx)
        if torso is None:
            return
        cx = (torso[2] + torso[3]) / 2
        for e, caja in _brazos(ctx).items():
            if d in ("derecha", "izquierda") and e != "A":
                continue
            ay0, ay1, ax0, ax1 = caja
            for y in range(ay0 + 2, ay1 + 1):
                xs = [x for x in range(ax0, ax1 + 1) if lab[y, x] == e and not es_contorno(ctx["tono"][y, x])]
                if not xs:
                    continue
                if d in ("abajo", "arriba"):
                    x = xs[0] if (ax0 + ax1) / 2 < cx else xs[-1]
                else:
                    x = xs[len(xs) // 2]
                pintar(ctx, y, x, m, "s" if (y % 3) else "H")
    return fn


def cinturon_duc(oro, gema):
    """El cinturó ample d'or (dues files) i, de cara, la placa del mig: tres de ample i tres d'alt, el
    marc d'or i la gemma fosca amb el reflex vermell al centre."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        if cint is None:
            return
        m, mg = ctx["mat"][oro], ctx["mat"][gema]
        xs = [x for x in range(lab.shape[1]) if lab[cint, x] in ("p", "t")]
        if not xs:
            return
        for y in (cint - 1, cint):
            for x in range(min(xs), max(xs) + 1):
                if lab[y, x] in DELANTE:
                    continue
                t = ctx["tono"][y, x]
                pintar(ctx, y, x, m, "O" if es_contorno(t) else ("H" if y == cint - 1 else "s"))
        if d != "abajo":
            return
        cx = (min(xs) + max(xs)) // 2
        for (yy, xx, mm, t) in ((cint - 1, cx - 1, m, "H"), (cint - 1, cx, m, "H"), (cint - 1, cx + 1, m, "H"),
                                (cint, cx - 1, m, "d"), (cint, cx, mg, "s"), (cint, cx + 1, m, "d"),
                                (cint + 1, cx - 1, m, "O"), (cint + 1, cx, m, "d"), (cint + 1, cx + 1, m, "O")):
            pintar(ctx, yy, xx, mm, t)
    return fn


def maluc_duc(oro, rojo):
    """Les peces dels malucs: a cada costat, una placa d'or de dues files sobre la cadera i, sota, dues
    tires de tela vermella que cauen fins a mitja cuixa (de cara i d'esquena); de perfil, la del costat de
    la càmera."""
    def fn(ctx):
        lab, d, cint, f = ctx["lab"], ctx["dir"], ctx["cint"], ctx["frame"]
        if cint is None:
            return
        mo, mr = ctx["mat"][oro], ctx["mat"][rojo]
        fila = [x for x in range(lab.shape[1]) if lab[cint + 1, x] == "p"]
        if not fila:
            return
        x0, x1 = min(fila), max(fila)
        mece = BALANCEO.get(f, 0)
        if d in ("abajo", "arriba"):
            costats = [(x0 - 1, x0 + 1), (x1 - 1, x1 + 1)]
        else:
            davant = x1 if d == "derecha" else x0
            centre = (x0 + x1) // 2
            costats = [(centre - 1, centre + 1)]
            _ = davant
        for (a, b) in costats:
            placa = {(y, x) for y in (cint + 1, cint + 2) for x in range(a, b + 1)}
            tonos = _sombrea(placa)
            for (y, x), t in tonos.items():
                if lab[y, x] not in DELANTE:
                    pintar(ctx, y, x, mo, t if t != "d" else "s")
            tires = set()
            for k in range(1, 6):
                y = cint + 2 + k
                s = mece if k >= 4 else 0
                for x in (a + s, a + 1 + s, b + s):
                    if x == a + 1 + s and k % 2 == 0:
                        continue
                    tires.add((y, x))
            tonos = _sombrea(tires)
            for (y, x), t in tonos.items():
                if 0 <= y < lab.shape[0] and lab[y, x] not in DELANTE:
                    pintar(ctx, y, x, mr, t)
    return fn


def capa_cintura(rojo, oro=None, largo=1.0, vuelo=5):
    """La capa vermella que neix de la cintura i cau fins als turmells, oberta: d'esquena tapa les
    cames; de cara es veu per darrere, als costats i entre les cames; de perfil vola cap enrere. Es
    gronxa en caminar; amb `oro`, el cantell de baix d'or."""
    def fn(ctx):
        lab, d, cint, f = ctx["lab"], ctx["dir"], ctx["cint"], ctx["frame"]
        if cint is None:
            return
        m = ctx["mat"][rojo]
        pies = [v for v in (top_pie(lab, "f"), top_pie(lab, "F")) if v is not None]
        if not pies:
            return
        bajo = cint + max(4, int(round((max(pies) + 2 - cint) * largo)))
        alto = max(1, bajo - cint)
        mece = BALANCEO.get(f, 0)
        punts = set()
        previo = None
        for y in range(cint, min(bajo, lab.shape[0] - 2) + 1):
            xs = np.nonzero(np.isin(lab[y], ("p", "l", "L", "f", "F")))[0]
            if len(xs):
                previo = (int(xs.min()), int(xs.max()))
            if previo is None:
                continue
            x0, x1 = previo
            t = (y - cint) / alto
            w = 1 + int(round(vuelo * t))
            s = int(round(mece * max(0.0, t - 0.3) / 0.7))
            if d == "abajo":  # de cara, el vent l'emporta cap a un costat (el de la mà sense arma)
                for x in range(x0 - 2 * w + s, x1 + 1 + s):
                    punts.add((y, x))
            elif d == "arriba":
                for x in range(x0 - w + s, x1 + w + 2 * (w // 2) + s + 1):
                    punts.add((y, x))
            else:
                extra = 2 + int(round(1.6 * vuelo * t))
                if d == "derecha":
                    a, b = x0 - extra - s, x0 + 2
                else:
                    a, b = x1 - 2, x1 + extra - s
                for x in range(a, b + 1):
                    punts.add((y, x))
        tonos = _sombrea(punts)
        for (y, x), t in list(tonos.items()):
            if t == "s" and (x % 3 == 0) and (y - cint) * 3 > alto:
                tonos[(y, x)] = "d"
        if oro:
            mo = ctx["mat"][oro]
            ybajo = max(y for y, _ in punts)
        if d == "arriba":
            _pinta(ctx, tonos, m, respeta_delante=True)
            if oro:
                for (y, x), t in tonos.items():
                    if y == ybajo - 1 and t != "O" and lab[y, x] not in DELANTE:
                        pintar(ctx, y, x, mo, "s")
        else:
            _pinta_detras(ctx, tonos, m)
            if oro:
                for (y, x), t in tonos.items():
                    if y == ybajo - 1 and t != "O" and _fuera_cuerpo(ctx, y, x):
                        pintar(ctx, y, x, mo, "s")
    return fn


# Una fulla flotant del Duc (de punta amunt): la fulla clara amb el fil de mana
# vermell, la guarda i el pom d'or.
_FULLA_DUC = [
    ".h.",    # la punta
    "oHr",
    "oHr",
    "oHr",
    "oho",
    "GgG",    # la guarda
    ".g.",    # el puny
    ".P.",    # el pom
]


def fulles_duc(hoja, oro, mana):
    """Les fulles del Duc: dues a cada costat, fora del cos, una a l'alçada del cap i l'altra de les
    espatlles, de punta amunt; de perfil, les dues de darrere l'esquena."""
    mats = {"h": (hoja, "s"), "H": (hoja, "H"), "o": (hoja, "O"), "G": (oro, "s"), "g": (oro, "d"),
            "P": (oro, "H"), "r": (mana, "s")}

    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        torso = _torso(ctx)
        if torso is None:
            return
        ty0, _ty1, tx0, tx1 = torso
        if d in ("abajo", "arriba"):
            sitios = [(ty0 - 8, tx0 - 6), (ty0 - 2, tx0 - 10), (ty0 - 8, tx1 + 4), (ty0 - 2, tx1 + 8)]
        else:
            atras = -1 if d == "derecha" else 1
            xb = tx0 if d == "derecha" else tx1
            sitios = [(ty0 - 9, xb + atras * 5 - 1), (ty0 - 3, xb + atras * 9 - 1)]
        for (y, x) in sitios:
            for i, f in enumerate(_FULLA_DUC):
                for j, c in enumerate(f):
                    if c != "." and _vacio(lab, y + i, x + j):
                        nombre, tono = mats[c]
                        pintar(ctx, y + i, x + j, ctx["mat"][nombre], tono)
    return fn


def casaca_duc(negro, oro, rojo, mana, guante, hoja, gema):
    """La casaca del Duc."""
    return {"partes": {"t": negro, "a": negro, "A": negro, "n": negro, "m": guante, "M": guante},
            "extras": [capa_cintura(rojo, oro), tancat_duc(oro), linias_mana(mana), punys(oro),
                       cuello_alto(negro, oro, 3),
                       hombrera_grande(oro, rojo, "A", placas=2), hombrera_grande(oro, None, "a", placas=1),
                       cinturon_duc(oro, gema), maluc_duc(oro, rojo), fulles_duc(hoja, oro, mana)]}


def rodillera(oro):
    """Un rombe d'or a cada genoll (les calces del Duc)."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][oro]
        for pierna, pie in (("l", "f"), ("L", "F")):
            if d in ("derecha", "izquierda") and pierna == "l":
                continue
            tp = top_pie(lab, pie)
            if tp is None:
                continue
            y = tp - 5
            xs = np.nonzero(lab[y] == pierna)[0]
            if not len(xs):
                continue
            if d in ("abajo", "arriba"):
                cx = int(xs[len(xs) // 2])
            else:
                cx = int(xs.max()) - 1 if d == "derecha" else int(xs.min()) + 1
            for (dy, dx, t) in ((-1, 0, "H"), (0, -1, "s"), (0, 0, "H"), (0, 1, "d"), (1, 0, "d")):
                if lab[y + dy, cx + dx] == pierna:
                    pintar(ctx, y + dy, cx + dx, m, t)
    return fn


def calces_duc(negro, oro):
    return {"partes": {"p": negro, "l": negro, "L": negro}, "extras": [rodillera(oro)]}


def greva_duc(oro):
    """La greva d'or angular a la canya de la bota: de cara, una punta que puja pel mig de la canya i
    s'obre avall; de perfil, la placa al cantell de davant; la puntera d'or."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][oro]
        for pierna, pie in (("l", "f"), ("L", "F")):
            tp = top_pie(lab, pie)
            if tp is None:
                continue
            if d in ("derecha", "izquierda") and pierna == "l":
                continue
            for k, y in enumerate(range(tp - 4, tp)):
                xs = np.nonzero(lab[y] == pierna)[0]
                if not len(xs):
                    continue
                if d in ("abajo", "arriba"):
                    cx = int(xs[len(xs) // 2])
                    semi = 0 if k == 0 else 1
                    for x in range(cx - semi, cx + semi + 1):
                        pintar(ctx, y, x, m, "H" if x < cx else ("s" if x == cx else "d"))
                else:
                    x = int(xs.max()) if d == "derecha" else int(xs.min())
                    pintar(ctx, y, x, m, "s")
                    x2 = x - 1 if d == "derecha" else x + 1
                    if k > 0:
                        pintar(ctx, y, x2, m, "H")
            if d != "arriba":
                ys = np.nonzero((lab == pie).any(axis=1))[0]
                yb = int(ys.max())
                xs = np.nonzero(lab[yb] == pie)[0]
                if len(xs):
                    puntera = xs[-2:] if d == "derecha" else (xs[:2] if d == "izquierda" else xs[1:-1])
                    for x in puntera:
                        pintar(ctx, yb - 1, int(x), m, "s")
    return fn


def botes_duc(cuero, oro):
    return {"botas": (cuero, 5), "extras": [greva_duc(oro)]}


# La diadema del Duc: el cèrcol fi d'or i el robí al front (la de cascos.DIADEMA, amb la gemma pròpia).
DIADEMA_DUC = cascos.DIADEMA



def punys(material):
    """Els punys de la màniga (la fila de baix de cada braç) en un altre material."""
    def fn(ctx):
        lab = ctx["lab"]
        m = ctx["mat"][material]
        for e in ("a", "A"):
            caja = _caja(lab, (e,))
            if caja is None:
                continue
            y = caja[1]
            for x in np.nonzero(lab[y] == e)[0]:
                t = ctx["tono"][y, int(x)]
                pintar(ctx, y, int(x), m, "O" if es_contorno(t) else "s")
    return fn


# --- 2. El Sobirà sense rostre ----------------------------------------------------------------------

from actores_siluetas import espejo_mitad as em  # noqa: E402
from actores_siluetas import plantilla  # noqa: E402
from actores_trajes_iteracion3 import canto_bajo  # noqa: E402

# El yelmo tancat negre (la forma del yelmo de cascos.YELMO): la corona d'or de cinc puntes d'espasa al
# voltant de la closca, la cresta d'or pel mig, l'escletxa en T (fosca: la llum hi va a VISERA_SOBIRA) i la
# gola d'or. Material 1, la placa negra; 2, l'or.
YELMO_SOBIRA = plantilla({
    "abajo": (13, em([
        "..........+",   # 13 la punta del mig
        "..........+",   # 14
        "......+...+",   # 15
        "...+..+...+",   # 16
        "...+..+..++",   # 17
        "...+..+###+",   # 18
        "..+++++++++",   # 19 la corona
        "..+++++++++",   # 20
        "...#######+",   # 21 la cresta
        "..########+",   # 22
        "..########+",   # 23
        "..########+",   # 24
        "..########+",   # 25
        "..########+",   # 26
        "..########+",   # 27
        "..#########",   # 28
        "..##4444444",   # 29 l'escletxa
        "..##4444444",   # 30
        "..#######44",   # 31
        "..#######44",   # 32
        "..#######44",   # 33
        "...########",   # 34
        "..+++++++++",   # 35 la gola
    ])),
    "arriba": (13, em([
        "..........+",
        "..........+",
        "......+...+",
        "...+..+...+",
        "...+..+..++",
        "...+..+###+",
        "..+++++++++",
        "..+++++++++",
        "...#######+",
        "..########+",
        "..########+",
        "..########+",
        "..########+",
        "..########+",
        "..########+",
        "..########+",
        "..########+",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "...########",
        "..+++++++++",
    ])),
    "derecha": (13, [
        ".............+",        # 13
        ".............+",        # 14
        ".........+...+",        # 15
        ".....+...+...+",        # 16
        ".....+...+..++",        # 17
        ".....+###+##.+",        # 18
        "..++++++++++++++",      # 19 la corona
        "..++++++++++++++",      # 20
        "...############",       # 21
        "..#############",       # 22
        "..##############",      # 23
        "..##############",      # 24
        "..###############",     # 25
        "..###############",     # 26
        "..###############",     # 27
        "..###############",     # 28
        "..###########4444",     # 29 l'escletxa
        "..###########4444",     # 30
        "..##############4",     # 31
        "..##############4",     # 32
        "..###############",     # 33
        "...#############",      # 34
        "..++++++++++++++",      # 35 la gola
    ]),
})
# La llum vermella de l'escletxa (material 2, el seu).
VISERA_SOBIRA = plantilla({
    "abajo": (29, em([
        "....8777777",   # 29
        "....7wwwwww",   # 30
        ".........7w",   # 31
        ".........7w",   # 32
        ".........88",   # 33
    ])),
    "derecha": (29, [
        "............8777",   # 29
        "............7www",   # 30
        "...............w",   # 31
        "...............8",   # 32
    ]),
})

# La corona d'espases brodada al tabard: tres fulles petites de punta sobre una banda.
EMBLEMAS.setdefault("corona_espadas", lambda y0, cx: [
    (y0 + 3, cx, "H"),
    (y0 + 4, cx - 2, "H"), (y0 + 4, cx, "H"), (y0 + 4, cx + 2, "H"),
    (y0 + 5, cx - 2, "s"), (y0 + 5, cx, "s"), (y0 + 5, cx + 2, "s"),
    (y0 + 6, cx - 2, "d"), (y0 + 6, cx - 1, "d"), (y0 + 6, cx, "d"), (y0 + 6, cx + 1, "d"), (y0 + 6, cx + 2, "d")])


def capa_llarga(tela, oro=None):
    """La capa llarga de les espatlles als turmells: d'esquena tapa l'esquena i les cames; de cara es veu
    per darrere, als costats; de perfil vola cap enrere. Es gronxa en caminar; amb `oro`, el cantell de baix
    d'or."""
    def fn(ctx):
        lab, d, f = ctx["lab"], ctx["dir"], ctx["frame"]
        m = ctx["mat"][tela]
        cuerpo = np.isin(lab, ("t", "a", "A", "m", "M", "p", "l", "L"))
        filas_t = np.nonzero((lab == "t").any(axis=1))[0]
        if not len(filas_t):
            return
        hombros = int(filas_t.min())
        pies = [v for v in (top_pie(lab, "f"), top_pie(lab, "F")) if v is not None]
        if not pies:
            return
        fondo = max(pies) + 1
        mitad = (hombros + fondo) / 2
        punts = set()
        for y in range(hombros, fondo + 1):
            xs = np.nonzero(cuerpo[min(y, lab.shape[0] - 1)])[0]
            if not len(xs):
                continue
            vuelo = max(0, (y - hombros) // 5)
            sway = int(round(BALANCEO.get(f, 0) * max(0.0, (y - mitad) / max(1.0, fondo - mitad))))
            if d in ("abajo", "arriba"):
                x0, x1 = int(xs.min()) - 1 - vuelo + sway, int(xs.max()) + 1 + vuelo + sway
                if y == hombros:
                    x0, x1 = x0 + 2, x1 - 2
            else:
                atras = int(xs.min()) if d == "derecha" else int(xs.max())
                k = 1 + min(5, (y - hombros) // 3)
                if d == "derecha":
                    x0, x1 = atras - k - sway, atras + 3
                else:
                    x0, x1 = atras - 3, atras + k + sway
            for x in range(x0, x1 + 1):
                punts.add((y, x))
        tonos = _sombrea(punts)
        for (y, x), t in list(tonos.items()):
            if t == "s" and x % 4 == 0:
                tonos[(y, x)] = "d"
        ybajo = max(y for y, _ in punts)
        if d == "arriba":
            for (y, x), t in tonos.items():
                pintar(ctx, y, x, m, t)
                ctx["cubre"][(y, x)] = m
            if oro:
                for (y, x), t in tonos.items():
                    if y == ybajo - 1 and t != "O":
                        pintar(ctx, y, x, ctx["mat"][oro], "s")
        else:
            _pinta_detras(ctx, tonos, m)
            if oro:
                for (y, x), t in tonos.items():
                    if y == ybajo - 1 and t != "O" and _fuera_cuerpo(ctx, y, x):
                        pintar(ctx, y, x, ctx["mat"][oro], "s")
    return fn


# Una fulla espectral (vermella i translúcida, el nucli clar), de punta avall: la guarda petita i el puny.
_ESPECTRE = [
    ".s.",
    ".d.",
    "sHs",
    "OHO",
    "OHO",
    "OHO",
    "OHO",
    ".H.",
]


def fulles_espectre(material):
    """Les fulles espectrals del Sobirà, que li volten la cintura: dues a cada costat (una a l'alçada del
    cinturó i l'altra més avall i més enfora); de perfil, una davant i una darrere."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        torso = _torso(ctx)
        if torso is None or cint is None:
            return
        _ty0, _ty1, tx0, tx1 = torso
        m = ctx["mat"][material]
        if d in ("abajo", "arriba"):
            sitios = [(cint - 9, tx0 - 8), (cint + 1, tx0 - 10), (cint - 9, tx1 + 6), (cint + 1, tx1 + 8)]
        else:
            davant = 1 if d == "derecha" else -1
            xf = tx1 if d == "derecha" else tx0
            xb = tx0 if d == "derecha" else tx1
            sitios = [(cint - 8, xf + davant * 6 - 1), (cint + 1, xb - davant * 9 - 1)]
        for (y, x) in sitios:
            for i, fila in enumerate(_ESPECTRE):
                for j, c in enumerate(fila):
                    if c != "." and _vacio(lab, y + i, x + j):
                        pintar(ctx, y + i, x + j, m, c)
    return fn


def placa_sobira(placa, oro, tabard, espectre):
    """La placa del Sobirà."""
    return {"partes": {"t": placa, "a": placa, "A": placa, "m": placa, "M": placa, "n": placa},
            "bajo": (placa, 3), "texturas": {placa: equipo.chapa}, "cinturon": (oro, None, 0),
            "extras": [equipo.peto(placa), canto_bajo(oro), punys(oro),
                       tabardo_largo(tabard, oro, oro, largo=10, semi=1, forma="corona_espadas", cantos=True),
                       hombrera_grande(oro, tabard, "A", placas=3), hombrera_grande(oro, tabard, "a", placas=3),
                       capa_llarga(tabard, oro), fulles_espectre(espectre)]}


def grebes_sobira(placa, oro):
    return {"partes": {"p": placa, "l": placa, "L": placa}, "texturas": {placa: equipo.chapa},
            "extras": [rodillera(oro)]}


def puntera(material):
    """La puntera de la bota en un altre material (les dues files de baix de la punta)."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        if d == "arriba":
            return
        m = ctx["mat"][material]
        for pie in ("f", "F"):
            ys = np.nonzero((lab == pie).any(axis=1))[0]
            if not len(ys):
                continue
            yb = int(ys.max())
            for y in (yb - 1, yb):
                xs = np.nonzero(lab[y] == pie)[0]
                if not len(xs):
                    continue
                if d == "derecha":
                    sel = xs[-3:]
                elif d == "izquierda":
                    sel = xs[:3]
                else:
                    sel = xs[1:-1]
                for x in sel:
                    pintar(ctx, y, int(x), m, "s" if y == yb - 1 else "d")
    return fn


def sabates_sobira(placa, oro):
    return {"botas": (placa, 4), "texturas": {placa: equipo.chapa}, "extras": [puntera(oro)]}


# --- 3. El Rei de la Forja --------------------------------------------------------------------------

# La corona de fulles trencades: el cèrcol de ferro amb cinc fulles de punta, trencades a alçades
# diferents, amb la punta encara roent, i l'or fos que brilla per les esquerdes del cèrcol (material 2).
CORONA_FORJA = plantilla({
    "abajo": (14, [
        "..........w",            # 14 la del mig, la més alta
        "......w...#....w",       # 15
        "......#..##..w.#",       # 16
        "....w.##.##..#.#",       # 17
        "....#.##.##.##.#",       # 18
        "....#############",      # 19 el cèrcol
        "....#+###w###+##+",      # 20 l'or fos a les esquerdes
        "....#############",      # 21
    ]),
    "arriba": (14, [
        "......w",
        "......#...w....w",
        "....w.#..##....#",
        "....#.##.##..w.#",
        "....#.##.##.##.#",
        "....#############",
        "....+##+###w###+#",
        "....#############",
    ]),
    "derecha": (14, [
        ".........w",             # 14
        "......w..#...w",         # 15
        ".....##..##..#",         # 16
        ".....##.###.##",         # 17
        ".....##.###.##",         # 18
        "....###########",        # 19
        "....##+##w##+##",        # 20
        "....###########",        # 21
    ]),
})


def _linia(y0, x0, y1, x1):
    """Els punts d'una línia (Bresenham), de (y0, x0) a (y1, x1)."""
    punts = []
    dy, dx = abs(y1 - y0), abs(x1 - x0)
    sy, sx = (1 if y1 > y0 else -1), (1 if x1 > x0 else -1)
    err = dx - dy
    y, x = y0, x0
    while True:
        punts.append((y, x))
        if (y, x) == (y1, x1):
            break
        e2 = 2 * err
        if e2 > -dy:
            err -= dy
            x += sx
        if e2 < dx:
            err += dx
            y += sy
    return punts


def espasa_esquena(fulla, fos, empunyadura):
    """Un espadó a l'esquena, en diagonal, acabat de forjar: la fulla d'acer amb el tall d'or fos que encara
    brilla, la guarda d'or fos i el puny de cuir. D'esquena es veu sencer (el puny per sobre de l'espatlla
    dreta, la punta al turmell esquerre); de cara, el puny i la guarda al costat del cap i la punta per sota
    del maluc; de perfil, el puny per sobre de l'espatlla i la punta darrere els genolls."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        torso = _torso(ctx)
        if torso is None or cint is None:
            return
        ty0, _ty1, tx0, tx1 = torso
        mf, mo, me = ctx["mat"][fulla], ctx["mat"][fos], ctx["mat"][empunyadura]
        if d == "abajo":
            a, b = (ty0 - 9, tx1 + 4), (cint + 9, tx0 - 4)
        elif d == "arriba":
            a, b = (ty0 - 9, tx0 - 4), (cint + 9, tx1 + 4)
        else:
            atras = -1 if d == "derecha" else 1
            xb = tx0 if d == "derecha" else tx1
            a, b = (ty0 - 9, xb + atras * 1), (cint + 9, xb + atras * 7)
        cami = _linia(a[0], a[1], b[0], b[1])
        n = len(cami)
        for i, (y, x) in enumerate(cami):
            if i == 0:
                trossos = [(x, me, "H")]                                   # el pom
            elif i < 4:
                trossos = [(x, me, "s" if i % 2 else "d")]                 # el puny
            elif i == 4:
                trossos = [(x - 2, mo, "d"), (x - 1, mo, "s"), (x, mo, "H"), (x + 1, mo, "s"), (x + 2, mo, "d")]
            elif i == n - 1:
                trossos = [(x, mf, "s")]                                   # la punta
            else:
                trossos = [(x - 1, mf, "H"), (x, mf, "s"), (x + 1, mo, "H" if i % 3 else "s")]
            for (xx, m, t) in trossos:
                if not (0 <= y < lab.shape[0] and 0 <= xx < lab.shape[1]):
                    continue
                if d == "arriba":
                    if lab[y, xx] in DELANTE:
                        continue
                    pintar(ctx, y, xx, m, t)
                elif _fuera_cuerpo(ctx, y, xx) and (y, xx) not in ctx["cubre"]:
                    pintar(ctx, y, xx, m, t)
    return fn


def _lado_fisico(d, lado):
    """La etiqueta del mateix braç físic en cada direcció (com hombrera_grande)."""
    return lado if d in ("abajo", "derecha", "izquierda") else ("a" if lado == "A" else "A")


def esquerdes(material):
    """Les esquerdes d'or fos de l'armadura: camins de dos o tres trams pel pit (de cara), per l'esquena (de
    darrere) i pel costat (de perfil), i un pel braç de l'escut; relatius al tors, així segueixen el cos en
    caminar. El tram del mig, més clar."""
    CAMINS = {
        "abajo": [[(2, -3), (3, -2), (4, -2), (5, -1), (6, -1), (7, 0), (8, 0)], [(3, 2), (4, 2), (5, 3)],
                  [(9, 1), (9, 2), (10, 2)]],
        "arriba": [[(2, 2), (3, 1), (4, 1), (5, 0), (6, 0), (7, 1)], [(5, -3), (6, -2), (7, -2)]],
        "derecha": [[(2, 0), (3, 0), (4, 1), (5, 1), (6, 2), (7, 2)], [(8, -1), (9, -1)]],
    }
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        torso = _torso(ctx)
        if torso is None:
            return
        ty0, _ty1, tx0, tx1 = torso
        cx = (tx0 + tx1) // 2
        clave = "derecha" if d in ("derecha", "izquierda") else d
        for cami in CAMINS[clave]:
            for i, (dy, dx) in enumerate(cami):
                if d == "izquierda":
                    dx = -dx
                y, x = ty0 + dy, cx + dx
                if 0 <= y < lab.shape[0] and lab[y, x] == "t" and not es_contorno(ctx["tono"][y, x]):
                    pintar(ctx, y, x, m, "H" if 0 < i < len(cami) - 1 else "s")
                    v = x + 1
                    if lab[y, v] == "t" and not es_contorno(ctx["tono"][y, v]):
                        pintar(ctx, y, v, m, "d")
        e = "a" if d in ("abajo", "derecha", "izquierda") else "A"
        if d in ("derecha", "izquierda"):
            return
        caja = _caja(lab, (e,))
        if caja is None:
            return
        ay0, ay1, ax0, ax1 = caja
        for k, y in enumerate(range(ay0 + 2, min(ay1, ay0 + 6))):
            xs = [x for x in range(ax0, ax1 + 1) if lab[y, x] == e and not es_contorno(ctx["tono"][y, x])]
            if xs:
                pintar(ctx, y, xs[(k // 2) % len(xs)], m, "s")
    return fn


def guantellet(metal, fos, lado="A"):
    """El guantellet enorme: l'avantbraç i la mà del braç de l'arma, un píxel més gruixuts que el braç,
    de metall, amb els artells d'or fos."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m, mf = ctx["mat"][metal], ctx["mat"][fos]
        e = _lado_fisico(d, lado)
        if d in ("derecha", "izquierda") and lado == "A":
            e = "A"
        mano = "M" if e == "A" else "m"
        caja = _caja(lab, (e,))
        caja_m = _caja(lab, (mano,))
        if caja is None or caja_m is None:
            return
        ay0, ay1, _ax0, _ax1 = caja
        punts = set()
        for y in range(ay0 + 3, caja_m[1] + 1):
            xs = [x for x in range(lab.shape[1]) if lab[y, x] in (e, mano)]
            if not xs:
                continue
            for x in range(min(xs) - 1, max(xs) + 2):
                punts.add((y, x))
        tonos = _sombrea(punts)
        for (y, x), t in tonos.items():
            pintar(ctx, y, x, m, t)
            ctx["cubre"][(y, x)] = m
        ys = sorted({y for y, _ in punts})
        if len(ys) >= 3:
            y = ys[-3]
            fila = sorted(x for (yy, x) in punts if yy == y)
            for x in fila[1:-1:2]:
                pintar(ctx, y, x, mf, "s")
        y = ys[0] + 1 if ys else None
        if y is not None:
            fila = sorted(x for (yy, x) in punts if yy == y)
            for x in fila[1:-1]:
                pintar(ctx, y, x, mf, "d")
    return fn


def artells(material):
    """Els artells d'or fos dels guantellets: la fila de dalt de cada mà, un píxel sí i un no."""
    def fn(ctx):
        lab = ctx["lab"]
        m = ctx["mat"][material]
        for e in ("m", "M"):
            caja = _caja(lab, (e,))
            if caja is None:
                continue
            y = caja[0] + 1
            xs = [x for x in range(lab.shape[1]) if lab[y, x] == e and not es_contorno(ctx["tono"][y, x])]
            for x in xs[::2]:
                pintar(ctx, y, x, m, "s")
    return fn


def armadura_forja(ferro, fos, brasa, cuir, oro):
    """L'armadura del Rei de la Forja."""
    return {"partes": {"t": ferro, "a": ferro, "A": ferro, "m": ferro, "M": ferro, "n": ferro},
            "bajo": (ferro, 3), "texturas": {ferro: equipo.chapa}, "cinturon": (cuir, fos, 0),
            "extras": [equipo.peto(ferro), esquerdes(fos), artells(fos), canto_bajo(brasa),
                       hombrera_grande(ferro, fos, "A", placas=2), hombrera_grande(ferro, fos, "a", placas=2),
                       espasa_esquena("hoja_duc", fos, cuir)]}


def esquerda_cama(material):
    """Una esquerda d'or fos a cada canya (de cara i d'esquena; de perfil, la de la cama de davant)."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        for pierna, pie in (("l", "f"), ("L", "F")):
            if d in ("derecha", "izquierda") and pierna == "l":
                continue
            tp = top_pie(lab, pie)
            if tp is None:
                continue
            for k, y in enumerate(range(tp - 6, tp - 2)):
                xs = np.nonzero(lab[y] == pierna)[0]
                if len(xs) < 3:
                    continue
                x = int(xs[1 + (k % 2)])
                pintar(ctx, y, x, m, "H" if k == 1 else "s")
    return fn


def grebes_forja(ferro, cuir, fos):
    return {"partes": {"p": cuir, "l": ferro, "L": ferro}, "texturas": {ferro: equipo.chapa},
            "extras": [esquerda_cama(fos)]}


def botes_forja(ferro, fos):
    return {"botas": (ferro, 4), "texturas": {ferro: equipo.chapa}, "extras": [puntera(fos)]}


# --- registre ---------------------------------------------------------------------------------------

TORSO = {
    "esb_casaca_duc": (casaca_duc, ("negro_duc", "oro_duc", "rojo_duc", "mana_duc", "guante_duc", "hoja_duc",
                                    "gema_duc"), "Casaca del Duc"),
    "esb_placa_sobira": (placa_sobira, ("placa_sobira", "oro_sobira", "tabard_sobira", "espectre_sobira"),
                         "Placa del Sobirà"),
    "esb_armadura_forja": (armadura_forja, ("ferro_forja", "or_fos", "brasa_forja", "cuir_forja", "oro_duc"),
                           "Armadura de la Forja"),
}
PIERNAS = {
    "esb_calces_duc": (calces_duc, ("negro_duc", "oro_duc"), "Calces del Duc"),
    "esb_grebes_sobira": (grebes_sobira, ("placa_sobira", "oro_sobira"), "Grebes del Sobirà"),
    "esb_grebes_forja": (grebes_forja, ("ferro_forja", "cuir_forja", "or_fos"), "Grebes de la Forja"),
}
PIES = {
    "esb_botes_duc": (botes_duc, ("guante_duc", "oro_duc"), "Botes del Duc"),
    "esb_sabates_sobira": (sabates_sobira, ("placa_sobira", "oro_sobira"), "Sabates del Sobirà"),
    "esb_botes_forja": (botes_forja, ("ferro_forja", "or_fos"), "Botes de la Forja"),
}
CABEZA = {
    "esb_diadema_duc": ([(DIADEMA_DUC, None)], "oro_duc", "gema_duc", "Diadema del Duc", False),
    "esb_yelmo_sobira": ([(YELMO_SOBIRA, None), (VISERA_SOBIRA, "visera_sobira")], "placa_sobira", "oro_sobira",
                         "Yelmo del Sobirà", True),
    "esb_corona_forja": ([(CORONA_FORJA, None)], "ferro_forja", "or_fos", "Corona de la Forja", False),
}


def registrar():
    for ident, valor in TORSO.items():
        equipo.TORSO[ident] = valor
    for ident, valor in PIERNAS.items():
        equipo.PIERNAS[ident] = valor
    for ident, valor in PIES.items():
        equipo.PIES[ident] = valor
    for ident, (plant, m1, m2, nombre, oculta) in CABEZA.items():
        cascos.CABEZA[ident] = {"plantillas": plant, "m1": m1, "m2": m2, "nombre": nombre, "oculta_pelo": oculta}


registrar()
