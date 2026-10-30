"""SETS-03 — Los sets de tier 2 de las seis clases de la 3.ª iteración (como actores_trajes_propias).

Oriol (08/10/2026): los sets «del apóstata, señor de las armas, furia de los creadores, bardo de las 4
estaciones de tier 2, siguiendo los mismos criterios, y el mago de los chasquidos y el monje que hice». Los
datos (nombres, bonos, descripciones) están en tools/sets_iteracion3.py; aquí se dibuja lo propio de cada
set (las cabezas, los torsos, las calzas partidas, el pantalón con franja, los pantalones de jade y las botas
de piel); lo que se reutiliza del catálogo (las grebas, las calzas, las botas, los zapatos, las sandalias) lo
tiñe el tema del set (actores_estetica.TEMAS_POR_NOMBRE). Las paletas son propias y sin tipo: ningún tema las
toca, y ninguno de sus tonos es de otra paleta.

- El Apóstata: la coraza de paladín ennegrecida con las hombreras de pinchos, el tabardo gris rasgado con el
  sol de la orden boca abajo, la cadena por cinturón; la capucha granate con la aureola partida encima.
- El Señor de las Armas (SETS-04, 08/10: tres sets, de los esbozos que eligió Oriol, «estan perfectes els 3»):
  · el Duque: la casaca negra cerrada de cuello alto con los cantos de oro, las hombreras de oro, el cinturón
    ancho de oro con la placa de gema, las placas y las tiras rojas de las caderas, la capa roja que nace de la
    cintura, las líneas de maná rojo de los brazos y cuatro hojas que flotan; las calzas negras con el rombo de
    oro en la rodilla, las botas negras con la greba de oro y la diadema con el rubí;
  · el Soberano sin Rostro: el yelmo negro cerrado con la corona de cinco puntas de espada y la rendija en T que
    brilla de rojo, las placas negras con los cantos de oro, el tabardo rojo con la corona de espadas, la capa
    larga roja y las hojas espectrales que le rondan la cintura; las grebas y los escarpes negros y oro;
  · el Rey de la Forja: el hierro oscuro con las grietas de oro fundido, los nudillos al rojo, la corona de
    hojas rotas con las puntas al rojo y el mandoble recién forjado a la espalda; las grebas y las botas de
    hierro.
  Sustituyen al manto carmesí con armiño y la corona de espadas de SETS-03.
- La Furia de los Creadores: el peto blanco de cantos de oro con los brazos al aire, la piel de oso negra y la
  faja roja; el yelmo blanco con la corona de rayos de oro detrás; las botas rojas de borde de piel.
- El Bardo de las Cuatro Estaciones (nació entre los hombres gato, con la Balada de las Cuatro Estaciones): el
  poncho de lana del oro de las Praderas de Oro con la Marcha tejida en las cenefas, los pantalones de lana
  parda y la corona de hojas que cambia de estación al dar la vuelta y deja libres las orejas.
- El Mago Chasqueador: la chaqueta corta de cuello alto con los alamares de plata y la faja violeta, el
  pantalón con la franja violeta y el bombín con la estrella.
- El Monje del Despertar: el hábito de marfil sin mangas con las solapas y la faja de jade y las vendas en los
  antebrazos, los pantalones de jade atados al tobillo y la diadema con el tercer ojo de jade.

Se registra al importar actores_trajes (después de los de las clases propias: al final de cada atlas, ninguna
fila de antes se mueve).
"""
import numpy as np

import actores_cascos as cascos
import actores_equipo as equipo
from actores_materiales import MATERIALES
from actores_pintura import Material as M
from actores_pintura import _tramo_torso, es_contorno, filas_torso, pintar, top_pie
from actores_siluetas import espejo_mitad as em
from actores_siluetas import plantilla
from actores_trajes import (BALANCEO, CAPUCHA_COSECHADOR, _CUPULA_ESPALDA, _CUPULA_FRENTE, _CUPULA_PERFIL, DELANTE,
                            EMBLEMAS, _arte, _caja, _cuello, _espejo_x, _pinta, _plantilla, _sombrea, botas_piel,
                            brazales, cruce_gi, cuello_alto, faja, hakama, hombrera_grande, manto_piel,
                            tabardo_largo)
from actores_trajes_mago import _vacio
from actores_trajes_propias import _rejilla, pantalon_franja, raido

# --- paletas (luz, base, sombra, contorno, contorno duro) ----------------------------------------

_PALETAS = [
    # El Apostata: el acero ennegrecido, el gris del tabardo, el rojo del sol y de los ribetes, el granate de
    # la capucha y el oro ennegrecido de la aureola
    M("acero_apostata", "#8d8894", "#58535f", "#39343e", "#1c191f", "#130f15"),
    M("tabardo_apostata", "#d6d2c9", "#a7a298", "#767168", "#3b3834", "#272522"),
    M("rojo_apostata", "#ff6a69", "#c21b23", "#7d0c13", "#3f0609", "#2a0406"),
    M("granate_apostata", "#a63f4b", "#6d1d29", "#49111b", "#24080d", "#180508"),
    # la hoja negra del sable con el filo rojo (la luz del material es el filo)
    M("hoja_apostata", "#ff5c5b", "#3b2b31", "#251b20", "#130d10", "#0b0709"),
    M("aureola_apostata", "#d9b96b", "#9b7a37", "#5d4921", "#2e2410", "#1e180a"),
    # El oro de la Furia (era el del arsenal del Senor de las Armas de SETS-03; el set de la Furia lo usa)
    M("oro_arsenal", "#fff5c6", "#f8d25e", "#c99a2a", "#634a12", "#42310c"),
    # SETS-04, el Senor de las Armas. El Duque: el negro de la casaca, de las calzas y de los guantes, el oro, el
    # rojo de la capa y de las tiras, el mana rojo de los brazos y del filo de sus hojas, la hoja clara y la gema
    # oscura del cinturon y de la diadema
    M("negro_duque", "#53566a", "#2b2c36", "#1a1b22", "#0d0c11", "#08070b"),
    M("oro_duque", "#fff3b8", "#f5c84a", "#b98a22", "#5e430f", "#3f2d0a"),
    M("rojo_duque", "#ff6064", "#c8162d", "#80101f", "#40070f", "#2b050a"),
    M("mana_duque", "#ffe0d8", "#ff4040", "#c5172c", "#6c0b16", "#47080f"),
    M("guante_duque", "#3d3b44", "#1f1e24", "#141317", "#0a090b", "#050405"),
    M("hoja_duque", "#f6f9ff", "#cfd8ea", "#97a3ba", "#4a5162", "#323743"),
    M("gema_duque", "#ff8c8c", "#a0102a", "#5e0718", "#2f040c", "#1f0308"),
    # El Soberano sin Rostro: las placas negras, el oro viejo, el rojo del tabardo y de la capa, las hojas
    # espectrales (translucidas) y la luz de la rendija del yelmo
    M("placa_soberano", "#6f7386", "#3c3f4c", "#25272f", "#121318", "#0a0b0e"),
    M("oro_soberano", "#f9e092", "#d6a83f", "#986a1d", "#4c340e", "#33230a"),
    M("tabardo_soberano", "#e8474f", "#a9131f", "#6c0a15", "#36050b", "#240307"),
    M("espectro_soberano", "#ffd9d6", "#ff5c5c", "#d92c3d", "#8c1124", "#5d0b18", alfa=200),
    M("visera_soberano", "#ffd2c4", "#ff3a2e", "#b8141f", "#5c0a10", "#3d070b"),
    # El Rey de la Forja: el hierro oscuro, el oro fundido de las grietas, la brasa y el cuero
    M("hierro_rey", "#77716f", "#47423f", "#2d2927", "#161413", "#0e0c0b"),
    M("oro_fundido", "#fffbd2", "#ffd640", "#ff8e1e", "#a3420d", "#6c2c08"),
    M("brasa_forja", "#ff7d48", "#cc361d", "#8c2113", "#461009", "#2f0b06"),
    M("cuero_forja", "#8a6a55", "#5a4334", "#3c2c22", "#1e1611", "#140e0b"),
    # La Furia de los Creadores: el acero blanco, la piel de oso negra, el rojo de la faja y el cuero rojo de las
    # botas y de las vendas de las manos
    M("blanco_furia", "#fefefd", "#e3e7ed", "#adb5c1", "#585f6a", "#3b4047"),
    M("oso_negro", "#5b5151", "#342d2d", "#231d1d", "#120f0f", "#0c0a0a"),
    M("rojo_furia", "#ff795b", "#d7351f", "#942013", "#4a1009", "#310b06"),
    M("cuero_rojo_furia", "#d86f5f", "#a4392b", "#70231a", "#38110c", "#250b08"),
    # El Bardo de las Cuatro Estaciones: el verde de la primavera y sus flores, el oro del verano, el rojo del
    # otono, el blanco del invierno, el cuero de las botas, la lana del poncho (el oro de las Praderas de Oro de
    # Bosmurg: el color de la nacion en el mapa, locations.json) y la lana parda de la camisa y los pantalones
    M("verde_primavera", "#c7ef8b", "#77bf3f", "#498925", "#234412", "#172d0c"),
    M("flor_primavera", "#ffd5e9", "#ff8bbf", "#d8548e", "#6c2947", "#481b2f"),
    M("oro_verano", "#fff19f", "#f4c32f", "#c18913", "#604409", "#402d06"),
    M("rojo_otono", "#ff9f5f", "#d1531d", "#953313", "#4a1909", "#311006"),
    M("blanco_invierno", "#fbfeff", "#e1f1fe", "#a7c7e5", "#4f6377", "#35424f"),
    M("cuero_estaciones", "#b88a5d", "#86603b", "#5b3f24", "#2d1f12", "#1e140c"),
    M("lana_bosmurg", "#fdf5d2", "#e8d98a", "#bba65c", "#5e532c", "#3f381e"),
    M("lana_parda", "#a07d5e", "#6e5038", "#4b3524", "#251a11", "#19110b"),
    # El Mago Chasqueador: el azul noche de la chaqueta, del pantalon y del bombin, el violeta de la faja, de la
    # franja y de la cinta, la plata de los alamares y de la estrella, y el blanco de los guantes
    M("azul_chasqueador", "#6b80d1", "#2f3e90", "#202b67", "#101634", "#0b0f23"),
    M("violeta_chasqueador", "#e1a1ff", "#a151e1", "#6f31a9", "#381955", "#261139"),
    M("plata_chasqueador", "#fcfdfe", "#cfd5e1", "#99a1b1", "#4c5059", "#32353b"),
    M("negro_chasqueador", "#4f5872", "#242a3a", "#171b26", "#0b0d13", "#06070b"),
    M("guante_chasqueador", "#fefdfb", "#f1f1f5", "#c3c3cf", "#616169", "#414146"),
    # El Monje del Despertar: el marfil del habito y de las vendas, el jade de las solapas, de la faja, de los
    # pantalones y del tercer ojo, y el oro de la diadema
    M("marfil", "#fffcf1", "#f1e7ce", "#ccbe9b", "#665e4d", "#443e33"),
    M("jade", "#9cefbf", "#2ead73", "#1c794f", "#0d3c27", "#09281a"),
    M("oro_despertar", "#fff3b3", "#ecc547", "#b98e1f", "#5c450c", "#3d2e08"),
]
_PISADAS = [m.nombre for m in _PALETAS if m.nombre in MATERIALES and MATERIALES[m.nombre].tono != m.tono]
if _PISADAS:
    raise ValueError(f"actores_trajes_iteracion3: estas paletas ya existen: {_PISADAS}")
_TONOS_AJENOS = {tuple(v) for n, m in MATERIALES.items() if n not in {p.nombre for p in _PALETAS}
                 for t, v in m.tono.items() if t != "X"}
_REPETIDOS = [m.nombre for m in _PALETAS for t, v in m.tono.items() if t != "X" and tuple(v) in _TONOS_AJENOS]
_PROPIOS = [tuple(v) for m in _PALETAS for t, v in m.tono.items() if t != "X"]
_REPETIDOS += [m.nombre for m in _PALETAS for t, v in m.tono.items() if t != "X" and _PROPIOS.count(tuple(v)) > 1]
if _REPETIDOS:
    raise ValueError(f"actores_trajes_iteracion3: tonos que ya son de otra paleta: {sorted(set(_REPETIDOS))}")
MATERIALES.update({m.nombre: m for m in _PALETAS})


# --- utilidades ----------------------------------------------------------------------------------

def _torso(ctx):
    return _caja(ctx["lab"], ("t",))


def _pinta_arte(ctx, y0, x0, filas, mats):
    """Un dibujo pequeno a mano sobre la capa: `filas` con una letra por material (en mayuscula la luz,
    en minuscula la base, '*' la sombra del ultimo material, 'o' el contorno duro) — ver _pixeles del
    Arquero; aqui con tonos explicitos: cada caracter es (material, tono) por la tabla `mats`."""
    for i, f in enumerate(filas):
        for j, c in enumerate(f):
            if c == "." or c not in mats:
                continue
            nombre, tono = mats[c]
            pintar(ctx, y0 + i, x0 + j, ctx["mat"][nombre], tono)


# --- A. El Apostata -------------------------------------------------------------------------------

# El sol de la orden, boca abajo: el canto plano arriba, el medio disco colgando y los rayos hacia abajo.
EMBLEMAS.setdefault("sol_caido", lambda y0, cx: [
    (y0 + 4, cx - 2, "H"), (y0 + 4, cx - 1, "H"), (y0 + 4, cx, "H"), (y0 + 4, cx + 1, "s"), (y0 + 4, cx + 2, "s"),
    (y0 + 5, cx - 1, "s"), (y0 + 5, cx, "s"), (y0 + 5, cx + 1, "d"),
    (y0 + 6, cx - 2, "d"), (y0 + 6, cx, "d"), (y0 + 6, cx + 2, "d"),
    (y0 + 7, cx, "d")])


def coraza_apostata(acero, tabardo, rojo, cadena):
    """La coraza del Apostata: el acero de paladin ennegrecido (peto, brazos y manoplas), las hombreras de dos
    placas con pinchos y el ribete rojo, el tabardo gris hasta medio muslo con el sol de la orden bordado boca
    abajo en rojo y el bajo rasgado, y una cadena por cinturon con el cabo colgando."""
    return {"partes": {"t": acero, "a": acero, "A": acero, "m": acero, "M": acero, "n": acero},
            "bajo": (acero, 3), "texturas": {acero: equipo.chapa}, "cinturon": (cadena, None, 3),
            "extras": [equipo.peto(acero), tabardo_largo(tabardo, rojo, rojo, largo=8, forma="sol_caido"), raido(),
                       hombrera_grande(acero, rojo, "A", placas=2, pinchos=acero),
                       hombrera_grande(acero, rojo, "a", placas=2, pinchos=acero)]}


# La capucha del Apostata es la de pico del Cosechador de almas (la cara se ve), granate y con el ribete de oro
# ennegrecido que enmarca la cara. Encima, la aureola partida: un aro de oro ennegrecido (material 2, tonos a
# mano) roto por la derecha, con la mitad rota caida un pixel, que flota sobre el pico (fila 13). De perfil, el
# aro de canto.
_AUREOLA_FRENTE = [
    "....667......",    # 7
    ".667....6....",    # 8 la mitad de la derecha, partida, cae un pixel
    "7........667.",    # 9
    ".778........7",    # 10
    "....778..778.",    # 11
    "........8....",    # 12
]
AUREOLA_ROTA = _plantilla({
    "abajo": _rejilla(7, 26, _AUREOLA_FRENTE),
    "arriba": _rejilla(7, 26, [f[::-1] for f in _AUREOLA_FRENTE]),
    "derecha": _rejilla(9, 29, [
        "667....",    # 9 de canto
        "....778",    # 10
    ]),
})


# --- B. El Senor de las Armas (SETS-04: tres sets) ------------------------------------------------

def _fuera_cuerpo(ctx, y, x):
    """True si (y, x) cae en la celda y alli no hay cuerpo (lo que va por detras: capas vistas de frente)."""
    lab = ctx["lab"]
    return 0 <= y < lab.shape[0] and 0 <= x < lab.shape[1] and not ctx["alfa"][y, x]


def _pinta_detras(ctx, tonos, m):
    """Pinta solo donde no hay cuerpo ni otra prenda (lo que va por detras del cuerpo)."""
    for (y, x), t in tonos.items():
        if _fuera_cuerpo(ctx, y, x) and (y, x) not in ctx["cubre"]:
            pintar(ctx, y, x, m, t)
            ctx["cubre"][(y, x)] = m


def punos(material):
    """Los punos de la manga (la fila de abajo de cada brazo) en otro material."""
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


def rodillera(oro):
    """Un rombo de oro en cada rodilla (de perfil, en la pierna de delante)."""
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


def puntera(material):
    """La puntera de la bota en otro material (las dos filas de abajo de la punta); de espaldas no se ve."""
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
                sel = xs[-3:] if d == "derecha" else (xs[:3] if d == "izquierda" else xs[1:-1])
                for x in sel:
                    pintar(ctx, y, int(x), m, "s" if y == yb - 1 else "d")
    return fn


# B1. El Duque ----------------------------------------------------------------------------------

def cierre_duque(oro):
    """La casaca cerrada: de frente, el canto de oro que la cierra por el medio y dos botones de oro bajo el
    cuello; de perfil, el canto de delante."""
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


def lineas_mana(mana):
    """Las lineas de mana rojo: una por la parte de fuera de cada brazo, del hombro al puno (de perfil, la del
    brazo de delante); un pixel de cada tres, mas claro."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][mana]
        torso = _caja(lab, ("t",))
        if torso is None:
            return
        cx = (torso[2] + torso[3]) / 2
        for e in ("a", "A"):
            caja = _caja(lab, (e,))
            if caja is None or (d in ("derecha", "izquierda") and e != "A"):
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
                pintar(ctx, y, x, m, "s" if y % 3 else "H")
    return fn


def cinturon_duque(oro, gema):
    """El cinturon ancho de oro (dos filas) y, de frente, la placa del medio: tres de ancho y tres de alto, el
    marco de oro y la gema oscura."""
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


def caderas_duque(oro, rojo):
    """Las piezas de las caderas: a cada lado, una placa de oro de dos filas sobre la cadera y, debajo, dos tiras
    de tela roja que caen hasta medio muslo y se mecen al andar (de frente y de espaldas); de perfil, una."""
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
            lados = [(x0 - 1, x0 + 1), (x1 - 1, x1 + 1)]
        else:
            centro = (x0 + x1) // 2
            lados = [(centro - 1, centro + 1)]
        for (a, b) in lados:
            placa = {(y, x) for y in (cint + 1, cint + 2) for x in range(a, b + 1)}
            for (y, x), t in _sombrea(placa).items():
                if lab[y, x] not in DELANTE:
                    pintar(ctx, y, x, mo, t if t != "d" else "s")
            tiras = set()
            for k in range(1, 6):
                y = cint + 2 + k
                s = mece if k >= 4 else 0
                for x in (a + s, a + 1 + s, b + s):
                    if x == a + 1 + s and k % 2 == 0:
                        continue
                    tiras.add((y, x))
            for (y, x), t in _sombrea(tiras).items():
                if 0 <= y < lab.shape[0] and lab[y, x] not in DELANTE:
                    pintar(ctx, y, x, mr, t)
    return fn


def capa_cintura(rojo, oro=None, vuelo=5):
    """La capa roja que nace de la cintura y cae hasta los tobillos: de espaldas tapa las piernas; de frente va
    por detras y el viento se la lleva hacia un lado (el de la mano sin arma); de perfil vuela hacia atras. Se
    mece al andar; con `oro`, el bajo de oro."""
    def fn(ctx):
        lab, d, cint, f = ctx["lab"], ctx["dir"], ctx["cint"], ctx["frame"]
        if cint is None:
            return
        m = ctx["mat"][rojo]
        pies = [v for v in (top_pie(lab, "f"), top_pie(lab, "F")) if v is not None]
        if not pies:
            return
        bajo = cint + max(4, max(pies) + 2 - cint)
        alto = max(1, bajo - cint)
        mece = BALANCEO.get(f, 0)
        puntos, previo = set(), None
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
            if d == "abajo":
                xa, xb = x0 - 2 * w + s, x1 + s
            elif d == "arriba":
                xa, xb = x0 - w + s, x1 + w + 2 * (w // 2) + s
            elif d == "derecha":
                xa, xb = x0 - 2 - int(round(1.6 * vuelo * t)) - s, x0 + 2
            else:
                xa, xb = x1 - 2, x1 + 2 + int(round(1.6 * vuelo * t)) - s
            for x in range(xa, xb + 1):
                puntos.add((y, x))
        tonos = _sombrea(puntos)
        for (y, x), t in list(tonos.items()):  # los pliegues
            if t == "s" and x % 3 == 0 and (y - cint) * 3 > alto:
                tonos[(y, x)] = "d"
        ybajo = max(y for y, _ in puntos)
        if d == "arriba":
            _pinta(ctx, tonos, m, respeta_delante=True)
        else:
            _pinta_detras(ctx, tonos, m)
        if oro:
            mo = ctx["mat"][oro]
            for (y, x), t in tonos.items():
                if y == ybajo - 1 and t != "O" and ctx["cubre"].get((y, x)) is m and lab[y, x] not in DELANTE:
                    pintar(ctx, y, x, mo, "s")
    return fn


# Una hoja del Duque, de punta (la hoja clara con el filo de mana rojo, la guarda y el pomo de oro).
_HOJA_DUQUE = [
    ".h.",    # la punta
    "oHr",
    "oHr",
    "oHr",
    "oho",
    "GgG",    # la guarda
    ".g.",    # el puno
    ".P.",    # el pomo
]


def hojas_duque(hoja, oro, mana):
    """Las hojas que le flotan al Duque: dos a cada lado, fuera del cuerpo, una a la altura de la cabeza y otra
    de los hombros; de perfil, las dos de detras de la espalda."""
    mats = {"h": (hoja, "s"), "H": (hoja, "H"), "o": (hoja, "O"), "G": (oro, "s"), "g": (oro, "d"),
            "P": (oro, "H"), "r": (mana, "s")}

    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        torso = _caja(lab, ("t",))
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
            for i, f in enumerate(_HOJA_DUQUE):
                for j, c in enumerate(f):
                    if c != "." and _vacio(lab, y + i, x + j):
                        nombre, tono = mats[c]
                        pintar(ctx, y + i, x + j, ctx["mat"][nombre], tono)
    return fn


def casaca_duque(negro, oro, rojo, mana, guante, hoja, gema):
    """La casaca del Duque: negra, cerrada y de cuello alto (con el forro de oro), los cantos y los botones de
    oro, las lineas de mana rojo de los brazos y los punos de oro, los guantes negros, la hombrera grande de oro
    con el ribete rojo en el brazo del arma y la pequena en el otro, el cinturon ancho de oro con la placa de
    gema, las placas de oro y las tiras rojas de las caderas, la capa roja de la cintura y sus cuatro hojas."""
    return {"partes": {"t": negro, "a": negro, "A": negro, "n": negro, "m": guante, "M": guante},
            "extras": [capa_cintura(rojo, oro), cierre_duque(oro), lineas_mana(mana), punos(oro),
                       cuello_alto(negro, oro, 3),
                       hombrera_grande(oro, rojo, "A", placas=2), hombrera_grande(oro, None, "a", placas=1),
                       cinturon_duque(oro, gema), caderas_duque(oro, rojo), hojas_duque(hoja, oro, mana)]}


def calzas_duque(negro, oro):
    return {"partes": {"p": negro, "l": negro, "L": negro}, "extras": [rodillera(oro)]}


def greba_duque(oro):
    """La greba de oro en punta sobre la cana de la bota: de frente, una punta que sube por el medio de la cana
    y se abre abajo; de perfil, la placa en el canto de delante; y la puntera de oro."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][oro]
        for pierna, pie in (("l", "f"), ("L", "F")):
            tp = top_pie(lab, pie)
            if tp is None or (d in ("derecha", "izquierda") and pierna == "l"):
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
                    if k > 0:
                        pintar(ctx, y, x - 1 if d == "derecha" else x + 1, m, "H")
            if d != "arriba":
                ys = np.nonzero((lab == pie).any(axis=1))[0]
                yb = int(ys.max())
                xs = np.nonzero(lab[yb] == pie)[0]
                if len(xs):
                    sel = xs[-2:] if d == "derecha" else (xs[:2] if d == "izquierda" else xs[1:-1])
                    for x in sel:
                        pintar(ctx, yb - 1, int(x), m, "s")
    return fn


def botas_duque(cuero, oro):
    return {"botas": (cuero, 5), "extras": [greba_duque(oro)]}


# B2. El Soberano sin Rostro -------------------------------------------------------------------

# El yelmo negro cerrado (la forma de cascos.YELMO): la corona de oro de cinco puntas de espada alrededor de la
# calota, la cresta de oro por el medio, la rendija en T (oscura: la luz va en VISERA_SOBERANO) y la gola de
# oro. Material 1, la placa negra; 2, el oro.
YELMO_SOBERANO = plantilla({
    "abajo": (13, em([
        "..........+",   # 13 la punta del medio
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
        "..##4444444",   # 29 la rendija
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
        "..###########4444",     # 29 la rendija
        "..###########4444",     # 30
        "..##############4",     # 31
        "..##############4",     # 32
        "..###############",     # 33
        "...#############",      # 34
        "..++++++++++++++",      # 35 la gola
    ]),
})
# La luz roja de la rendija (material 2, el suyo).
VISERA_SOBERANO = plantilla({
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

# La corona de espadas bordada en el tabardo: tres hojas pequenas de punta sobre una banda.
EMBLEMAS.setdefault("corona_espadas", lambda y0, cx: [
    (y0 + 3, cx, "H"),
    (y0 + 4, cx - 2, "H"), (y0 + 4, cx, "H"), (y0 + 4, cx + 2, "H"),
    (y0 + 5, cx - 2, "s"), (y0 + 5, cx, "s"), (y0 + 5, cx + 2, "s"),
    (y0 + 6, cx - 2, "d"), (y0 + 6, cx - 1, "d"), (y0 + 6, cx, "d"), (y0 + 6, cx + 1, "d"), (y0 + 6, cx + 2, "d")])


def capa_larga(tela, oro=None):
    """La capa larga de los hombros a los tobillos: de espaldas tapa la espalda y las piernas; de frente va por
    detras, a los lados; de perfil vuela hacia atras. Se mece al andar; con `oro`, el bajo de oro."""
    def fn(ctx):
        lab, d, f = ctx["lab"], ctx["dir"], ctx["frame"]
        m = ctx["mat"][tela]
        cuerpo = np.isin(lab, ("t", "a", "A", "m", "M", "p", "l", "L"))
        filas_t = np.nonzero((lab == "t").any(axis=1))[0]
        pies = [v for v in (top_pie(lab, "f"), top_pie(lab, "F")) if v is not None]
        if not len(filas_t) or not pies:
            return
        hombros = int(filas_t.min())
        fondo = max(pies) + 1
        mitad = (hombros + fondo) / 2
        puntos = set()
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
                x0, x1 = (atras - k - sway, atras + 3) if d == "derecha" else (atras - 3, atras + k + sway)
            for x in range(x0, x1 + 1):
                puntos.add((y, x))
        tonos = _sombrea(puntos)
        for (y, x), t in list(tonos.items()):  # los pliegues
            if t == "s" and x % 4 == 0:
                tonos[(y, x)] = "d"
        ybajo = max(y for y, _ in puntos)
        if d == "arriba":
            for (y, x), t in tonos.items():
                pintar(ctx, y, x, m, t)
                ctx["cubre"][(y, x)] = m
        else:
            _pinta_detras(ctx, tonos, m)
        if oro:
            mo = ctx["mat"][oro]
            for (y, x), t in tonos.items():
                if y == ybajo - 1 and t != "O" and ctx["cubre"].get((y, x)) is m:
                    pintar(ctx, y, x, mo, "s")
    return fn


# Una hoja espectral (roja y translucida, el nucleo claro), de punta hacia abajo: la guarda pequena y el puno.
_ESPECTRO = [
    ".s.",
    ".d.",
    "sHs",
    "OHO",
    "OHO",
    "OHO",
    "OHO",
    ".H.",
]


def hojas_espectrales(material):
    """Las hojas espectrales que le rondan la cintura al Soberano: dos a cada lado (una a la altura del pecho y
    otra mas abajo y mas afuera); de perfil, una delante y otra detras."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        torso = _caja(lab, ("t",))
        if torso is None or cint is None:
            return
        _ty0, _ty1, tx0, tx1 = torso
        m = ctx["mat"][material]
        if d in ("abajo", "arriba"):
            sitios = [(cint - 9, tx0 - 8), (cint + 1, tx0 - 10), (cint - 9, tx1 + 6), (cint + 1, tx1 + 8)]
        else:
            delante = 1 if d == "derecha" else -1
            xf = tx1 if d == "derecha" else tx0
            xb = tx0 if d == "derecha" else tx1
            sitios = [(cint - 8, xf + delante * 6 - 1), (cint + 1, xb - delante * 9 - 1)]
        for (y, x) in sitios:
            for i, fila in enumerate(_ESPECTRO):
                for j, c in enumerate(fila):
                    if c != "." and _vacio(lab, y + i, x + j):
                        pintar(ctx, y + i, x + j, m, c)
    return fn


def placas_soberano(placa, oro, tabardo, espectro):
    """Las placas del Soberano: negras con la arista, las lamas y el canto de oro abajo, los punos de oro, tres
    placas de oro en cada hombro con el ribete rojo, el cinturon de oro, el tabardo rojo con la corona de espadas
    de oro, la capa larga roja con el bajo de oro y las hojas espectrales."""
    return {"partes": {"t": placa, "a": placa, "A": placa, "m": placa, "M": placa, "n": placa},
            "bajo": (placa, 3), "texturas": {placa: equipo.chapa}, "cinturon": (oro, None, 0),
            "extras": [equipo.peto(placa), canto_bajo(oro), punos(oro),
                       tabardo_largo(tabardo, oro, oro, largo=10, semi=1, forma="corona_espadas", cantos=True),
                       hombrera_grande(oro, tabardo, "A", placas=3), hombrera_grande(oro, tabardo, "a", placas=3),
                       capa_larga(tabardo, oro), hojas_espectrales(espectro)]}


def grebas_soberano(placa, oro):
    return {"partes": {"p": placa, "l": placa, "L": placa}, "texturas": {placa: equipo.chapa},
            "extras": [rodillera(oro)]}


def escarpes_soberano(placa, oro):
    return {"botas": (placa, 4), "texturas": {placa: equipo.chapa}, "extras": [puntera(oro)]}


# B3. El Rey de la Forja -----------------------------------------------------------------------

# La corona de hojas rotas: el aro de hierro con cinco hojas de punta, rotas a distintas alturas, con la punta
# todavia al rojo, y el oro fundido que brilla por las grietas del aro (material 2).
CORONA_FORJA = plantilla({
    "abajo": (14, [
        "..........w",            # 14 la del medio, la mas alta
        "......w...#....w",       # 15
        "......#..##..w.#",       # 16
        "....w.##.##..#.#",       # 17
        "....#.##.##.##.#",       # 18
        "....#############",      # 19 el aro
        "....#+###w###+##+",      # 20 el oro fundido de las grietas
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


def _linea(y0, x0, y1, x1):
    """Los puntos de una linea (Bresenham), de (y0, x0) a (y1, x1)."""
    puntos = []
    dy, dx = abs(y1 - y0), abs(x1 - x0)
    sy, sx = (1 if y1 > y0 else -1), (1 if x1 > x0 else -1)
    err = dx - dy
    y, x = y0, x0
    while True:
        puntos.append((y, x))
        if (y, x) == (y1, x1):
            break
        e2 = 2 * err
        if e2 > -dy:
            err -= dy
            x += sx
        if e2 < dx:
            err += dx
            y += sy
    return puntos


def mandoble_espalda(hoja, fundido, puno):
    """El mandoble a la espalda, en diagonal, recien forjado: la hoja de acero con el filo de oro fundido que aun
    brilla, la guarda de oro fundido y el puno de cuero. De espaldas se ve entero (el puno sobre un hombro, la
    punta en el otro tobillo); de frente, solo lo que asoma por fuera del cuerpo (la guarda junto a la cabeza y la
    punta bajo la cadera); de perfil, el puno sobre el hombro y la punta detras de las rodillas."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        torso = _caja(lab, ("t",))
        if torso is None or cint is None:
            return
        ty0, _ty1, tx0, tx1 = torso
        mh, mf, mp = ctx["mat"][hoja], ctx["mat"][fundido], ctx["mat"][puno]
        if d == "abajo":
            a, b = (ty0 - 9, tx1 + 4), (cint + 9, tx0 - 4)
        elif d == "arriba":
            a, b = (ty0 - 9, tx0 - 4), (cint + 9, tx1 + 4)
        else:
            atras = -1 if d == "derecha" else 1
            xb = tx0 if d == "derecha" else tx1
            a, b = (ty0 - 9, xb + atras * 1), (cint + 9, xb + atras * 7)
        camino = _linea(a[0], a[1], b[0], b[1])
        n = len(camino)
        for i, (y, x) in enumerate(camino):
            if i == 0:
                trozos = [(x, mp, "H")]                                    # el pomo
            elif i < 4:
                trozos = [(x, mp, "s" if i % 2 else "d")]                  # el puno
            elif i == 4:
                trozos = [(x - 2, mf, "d"), (x - 1, mf, "s"), (x, mf, "H"), (x + 1, mf, "s"), (x + 2, mf, "d")]
            elif i == n - 1:
                trozos = [(x, mh, "s")]                                    # la punta
            else:
                trozos = [(x - 1, mh, "H"), (x, mh, "s"), (x + 1, mf, "H" if i % 3 else "s")]
            for (xx, m, t) in trozos:
                if not (0 <= y < lab.shape[0] and 0 <= xx < lab.shape[1]):
                    continue
                if d == "arriba":
                    if lab[y, xx] not in DELANTE:
                        pintar(ctx, y, xx, m, t)
                elif _fuera_cuerpo(ctx, y, xx) and (y, xx) not in ctx["cubre"]:
                    pintar(ctx, y, xx, m, t)
    return fn


def grietas(material):
    """Las grietas de oro fundido de la armadura: caminos por el pecho (de frente), por la espalda (de espaldas)
    y por el costado (de perfil), de dos pixeles (el de la derecha, en sombra) y el tramo del medio mas claro;
    relativos al torso, asi siguen el cuerpo al andar. Y una en el brazo de la mano sin arma, de frente y de
    espaldas."""
    caminos = {
        "abajo": [[(2, -3), (3, -2), (4, -2), (5, -1), (6, -1), (7, 0), (8, 0)], [(3, 2), (4, 2), (5, 3)],
                  [(9, 1), (9, 2), (10, 2)]],
        "arriba": [[(2, 2), (3, 1), (4, 1), (5, 0), (6, 0), (7, 1)], [(5, -3), (6, -2), (7, -2)]],
        "derecha": [[(2, 0), (3, 0), (4, 1), (5, 1), (6, 2), (7, 2)], [(8, -1), (9, -1)]],
    }

    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        torso = _caja(lab, ("t",))
        if torso is None:
            return
        ty0, _ty1, tx0, tx1 = torso
        cx = (tx0 + tx1) // 2
        for camino in caminos["derecha" if d in ("derecha", "izquierda") else d]:
            for i, (dy, dx) in enumerate(camino):
                if d == "izquierda":
                    dx = -dx
                y, x = ty0 + dy, cx + dx
                if 0 <= y < lab.shape[0] and lab[y, x] == "t" and not es_contorno(ctx["tono"][y, x]):
                    pintar(ctx, y, x, m, "H" if 0 < i < len(camino) - 1 else "s")
                    if lab[y, x + 1] == "t" and not es_contorno(ctx["tono"][y, x + 1]):
                        pintar(ctx, y, x + 1, m, "d")
        if d in ("derecha", "izquierda"):
            return
        e = "a" if d == "abajo" else "A"
        caja = _caja(lab, (e,))
        if caja is None:
            return
        ay0, ay1, ax0, ax1 = caja
        for k, y in enumerate(range(ay0 + 2, min(ay1, ay0 + 6))):
            xs = [x for x in range(ax0, ax1 + 1) if lab[y, x] == e and not es_contorno(ctx["tono"][y, x])]
            if xs:
                pintar(ctx, y, xs[(k // 2) % len(xs)], m, "s")
    return fn


def nudillos(material):
    """Los nudillos de oro fundido de los guanteletes: la segunda fila de cada mano, un pixel si y otro no."""
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


def armadura_forja(hierro, fundido, brasa, cuero, hoja):
    """La armadura del Rey de la Forja: laminas finas de hierro oscuro (el peto con la arista y las lamas, los
    guanteletes, el faldar con el canto al rojo) con las grietas de oro fundido, los nudillos de oro fundido, las
    hombreras de dos placas con el canto de oro fundido, el cinturon de cuero con la hebilla de oro fundido y el
    mandoble a la espalda."""
    return {"partes": {"t": hierro, "a": hierro, "A": hierro, "m": hierro, "M": hierro, "n": hierro},
            "bajo": (hierro, 3), "texturas": {hierro: equipo.chapa}, "cinturon": (cuero, fundido, 0),
            "extras": [equipo.peto(hierro), grietas(fundido), nudillos(fundido), canto_bajo(brasa),
                       hombrera_grande(hierro, fundido, "A", placas=2), hombrera_grande(hierro, fundido, "a", placas=2),
                       mandoble_espalda(hoja, fundido, cuero)]}


def grieta_espinilla(material):
    """Una grieta de oro fundido en cada espinilla (de perfil, en la pierna de delante)."""
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
                pintar(ctx, y, int(xs[1 + (k % 2)]), m, "H" if k == 1 else "s")
    return fn


def grebas_forja(hierro, cuero, fundido):
    return {"partes": {"p": cuero, "l": hierro, "L": hierro}, "texturas": {hierro: equipo.chapa},
            "extras": [grieta_espinilla(fundido)]}


def botas_forja(hierro, fundido):
    return {"botas": (hierro, 4), "texturas": {hierro: equipo.chapa}, "extras": [puntera(fundido)]}


# --- C. La Furia de los Creadores -----------------------------------------------------------------

def canto_bajo(material):
    """El canto de oro del faldar de placas: la fila de abajo de lo que cubre bajo la faja."""
    def fn(ctx):
        cint = ctx["cint"]
        if cint is None:
            return
        abajo = {}
        for (y, x) in ctx["cubre"]:
            if y > cint:
                abajo[x] = max(abajo.get(x, y), y)
        m = ctx["mat"][material]
        for x, y in abajo.items():
            if ctx["lab"][y, x] not in DELANTE:
                pintar(ctx, y, x, m, "s")
    return fn


def coraza_furia(blanco, oro, piel, rojo, cuero):
    """La coraza de la Furia: el peto de placas blancas con la arista y el faldar de cantos de oro, los brazos al
    aire con las manos vendadas de cuero rojo, la piel de oso negra sobre los hombros y la faja roja de guerra con
    los cabos."""
    return {"partes": {"t": blanco, "n": blanco, "m": cuero, "M": cuero}, "bajo": (blanco, 3),
            "texturas": {blanco: equipo.chapa},
            "extras": [equipo.peto(blanco), canto_bajo(oro), manto_piel(piel, 4), faja(rojo, 4), brazales(cuero)]}


# El yelmo de la Furia: la cupula blanca (la cara se ve) y, detras, la corona de rayos de oro que se abre como
# un sol (otra plantilla: siete puntas de frente, cuatro de perfil, la ultima tapada por la cabeza).
_RAYOS_FRENTE = _rejilla(9, 21, [
    "...........9...........",   # 9
    "..........969..........",   # 10
    "....9.....676.....9....",   # 11
    "....69....676....96....",   # 12
    ".....69...777...96.....",   # 13
    "9.....67..787..76.....9",   # 14
    "699....77.787.77....996",   # 15
    ".7766...7.....7...6677.",   # 16
    "...7776...........6777.",   # 17
    "......7............7...",   # 18
])
RAYOS_FURIA = _plantilla({
    "abajo": _RAYOS_FRENTE,
    "arriba": _RAYOS_FRENTE,
    "derecha": _rejilla(8, 16, [
        "..........9.......",   # 8
        "....9.....69......",   # 9
        "....69....67......",   # 10
        ".....69...77......",   # 11
        "9.....67..77......",   # 12
        "699....77.78......",   # 13
        ".7766...7.........",   # 14
        "...7776...........",   # 15
    ]),
})
YELMO_FURIA = _plantilla({"abajo": _CUPULA_FRENTE, "arriba": _CUPULA_ESPALDA, "derecha": _CUPULA_PERFIL})


# --- D. El Bardo de las Cuatro Estaciones ---------------------------------------------------------
# Oriol (08/10): «el origen del bardo de las 4 estaciones salio en la cultura de los hombres gato». Su vault
# (Gran Compendio de los Creadores): los Hombres Gato se asentaron en las Praderas de Oro de Bosmurg, con una
# cultura nomada «basada en la tradicion oral, en las canciones y las historias que contaban alrededor de
# fuegos»; la Marcha de las Cuatro Estaciones (8790 b.f.: verano, otono, invierno y primavera, la campana del
# Clan Cara Loca que hizo nacer Bosmurg) quedo en «la Balada de las Cuatro Estaciones», que «narra la marcha y
# el destino de cada estacion». Y su ficha de raza (Hombres Gato (Felinos)): tribus nomadas en contacto con la
# naturaleza, que se mueven en silencio. De ahi un bardo de camino, no de corte: el poncho de lana del oro de
# las praderas con la Marcha tejida en las cenefas (en su orden), los pantalones de lana parda, las botas
# blandas y la corona de hojas de las cuatro estaciones, que rodea la cabeza por debajo de las orejas.

def poncho(lana, estaciones):
    """El poncho del Bardo: de lana, de los hombros hasta las manos (tapa el pecho y los brazos; de frente, con
    la abertura del cuello), con los pliegues que caen de los hombros, dos cenefas tejidas con las cuatro
    estaciones (`estaciones`, en el orden de la Marcha: verano, otono, invierno y primavera; bloques de 2 px),
    una en el pecho (si cabe) y otra doble en el bajo, y los flecos por debajo."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        c = _cuello(ctx)
        if c is None or cint is None:
            return
        ty0, nx0, nx1 = c
        m = ctx["mat"][lana]
        brazos = np.nonzero(np.isin(lab, ("a", "A")).any(axis=1))[0]
        fin_brazo = int(brazos.max()) if len(brazos) else cint - 1
        bajo = min(cint, max(cint - 2, fin_brazo + 1))  # tapa los brazos hasta las manos
        puntos, tramo = set(), None
        for y in range(ty0 - 1, bajo + 1):
            xs = [x for x in range(lab.shape[1]) if lab[y, x] in ("t", "a", "A", "n")]
            if xs:
                tramo = (min(xs) - 1, max(xs) + 1)  # de brazo a brazo, un pixel mas ancho
            if tramo is None:
                continue
            for x in range(tramo[0], tramo[1] + 1):
                if y == ty0 - 1 and nx0 - 1 <= x <= nx1 + 1 and d != "arriba":
                    continue  # el cuello queda libre
                puntos.add((y, x))
        if d == "abajo":  # la abertura del cuello, por delante
            cx = (nx0 + nx1) // 2
            puntos -= {(ty0, cx - 1), (ty0, cx), (ty0, cx + 1), (ty0 + 1, cx)}
        tonos = _sombrea(puntos)
        for (y, x), t in list(tonos.items()):  # los pliegues
            if t in ("s", "H") and y >= bajo - 3 and x % 4 == 1:
                tonos[(y, x)] = "d"
        _pinta(ctx, tonos, m)
        ms = [ctx["mat"][e] for e in estaciones]

        def cenefa(y, desfase=0, tono="s"):
            xs = sorted(x for (yy, x) in puntos if yy == y and tonos[(yy, x)] != "O")
            for x in xs:
                pintar(ctx, y, x, ms[((x - xs[0] + desfase) // 2) % 4], tono)

        if bajo - 2 - (ty0 + 3) > 3:  # la del pecho, si cabe (en la talla pequena se juntarian)
            cenefa(ty0 + 3)
        cenefa(bajo - 2)
        cenefa(bajo - 1, 1, "d")
        for x in sorted({x for (y, x) in puntos if y == bajo}):  # los flecos (las manos van delante)
            if x % 2 == 0 and lab[bajo + 1, x] not in DELANTE:
                pintar(ctx, bajo + 1, x, m, "d")
    return fn


def poncho_estaciones(lana, camisa, primavera, verano, otono, invierno):
    """El poncho de las Cuatro Estaciones sobre la camisa de lana parda (sus mangas asoman al andar)."""
    return {"partes": {"t": camisa, "a": camisa, "A": camisa, "n": camisa},
            "extras": [poncho(lana, (verano, otono, invierno, primavera))]}


# La corona de las estaciones: una corona de hojas (tonos a mano del material 2: las puntas arriba y abajo, la
# luz y la sombra alternas) que cambia de estacion al dar la vuelta a la cabeza: de frente, la primavera (a la
# derecha del personaje) y el verano; de espaldas, el invierno y el otono; de perfil, la primavera delante y el
# otono detras. En la primavera, dos flores (otra plantilla). Va por debajo de las orejas (las de los hombres
# gato asoman por encima, actores_rasgos.OREJAS_GATO).
_HOJAS_FRENTE = [
    "..6.6.6.6.6.6.6.6..",   # 20 las puntas de arriba
    ".67676767676767676.",   # 21
    "8787878787878787878",   # 22
    ".8.8.8.8.8.8.8.8.8.",   # 23 las de abajo
]
_HOJAS_PERFIL = [
    "..6.6.6.6.6.6.6.6.",    # 19
    ".6767676767676767.",    # 20
    "878787878787878787",    # 21
    ".8.8.8.8.8.8.8.8..",    # 22
]


def _hojas(y0, x0, filas, quedarse):
    return [(y, x, x, c) for (y, x, _x, c) in _rejilla(y0, x0, filas) if quedarse(x)]


CORONA_PRIMAVERA = _plantilla({
    "abajo": _hojas(20, 23, _HOJAS_FRENTE, lambda x: x < 32),
    "derecha": _hojas(19, 24, _HOJAS_PERFIL, lambda x: x >= 33),
})
CORONA_VERANO = _plantilla({"abajo": _hojas(20, 23, _HOJAS_FRENTE, lambda x: x >= 32)})
CORONA_OTONO = _plantilla({
    "arriba": _hojas(20, 23, _HOJAS_FRENTE, lambda x: x >= 32),
    "derecha": _hojas(19, 24, _HOJAS_PERFIL, lambda x: x < 33),
})
CORONA_INVIERNO = _plantilla({"arriba": _hojas(20, 23, _HOJAS_FRENTE, lambda x: x < 32)})
FLORES_PRIMAVERA = _plantilla({
    "abajo": _arte(20, 25, [".6", "68"]) + _arte(21, 29, ["7"]),
    "derecha": _arte(19, 37, [".6", "68"]) + _arte(20, 34, ["7"]),
})


# --- E. El Mago Chasqueador -----------------------------------------------------------------------

def alamares(material):
    """Los alamares de plata de la chaqueta: tres cordones que cruzan el pecho de lado a lado (de frente); de
    perfil, sus cabos sobre el pecho; de espaldas no se ven."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        y0, _y1 = filas_torso(ctx)
        if y0 is None or d == "arriba":
            return
        m = ctx["mat"][material]
        for k in (3, 5, 7):
            y = y0 + k
            tr = _tramo_torso(lab, ctx["tono"], y)
            if not tr:
                continue
            if d == "abajo":
                cx = (tr[0] + tr[1]) // 2
                xs = range(cx - 3, cx + 4)
            elif d == "derecha":
                xs = range(tr[1] - 4, tr[1] - 1)
            else:
                xs = range(tr[0] + 2, tr[0] + 5)
            for x in xs:
                if lab[y, x] == "t" and not es_contorno(ctx["tono"][y, x]):
                    pintar(ctx, y, x, m, "H" if x % 2 == 0 else "s")
    return fn


def chaqueta_chasqueador(azul, plata, violeta):
    """La chaqueta corta del Chasqueador: azul noche, el cuello alto forrado de violeta, los alamares de plata
    y la faja violeta con sus cabos."""
    return {"partes": {"t": azul, "a": azul, "A": azul, "n": azul}, "bajo": (azul, 2),
            "extras": [cuello_alto(azul, violeta, 2), alamares(plata), faja(violeta, 3)]}


# El bombin del Chasqueador: la copa redonda azul noche (material 1) con la cinta violeta (material 2) y el ala
# corta vuelta; la estrella de plata prendida en la cinta, otra plantilla.
_COPA_BOMBIN = _espejo_x(_arte(13, 26, [
    "....###",   # 13
    "..#####",   # 14
    ".######",   # 15
    ".######",   # 16
    "#######",   # 17
    "#######",   # 18
    "+++++++",   # 19 la cinta
    "+++++++",   # 20
])) + [(15, 28, 28, "1"), (16, 28, 29, "1")]
# el ala corta, con los cantos vueltos hacia arriba
_ALA_BOMBIN = [(20, 24, 24, "#"), (20, 40, 40, "#"), (21, 24, 40, "#"), (22, 25, 39, "3")]
BOMBIN = _plantilla({
    "abajo": _COPA_BOMBIN + _ALA_BOMBIN,
    "arriba": [t for t in _COPA_BOMBIN if t[3] != "1"] + _ALA_BOMBIN,
    "derecha": _arte(13, 28, [
        "....####....",   # 13
        "..########..",   # 14
        ".##########.",   # 15
        ".##########.",   # 16
        "############",   # 17
        "############",   # 18
        "++++++++++++",   # 19
        "++++++++++++",   # 20
    ]) + [(20, 26, 26, "#"), (20, 41, 41, "#"), (21, 26, 41, "#"), (22, 27, 40, "3"), (15, 30, 30, "1"),
          (16, 30, 31, "1")],
})
ESTRELLA_BOMBIN = _plantilla({
    "abajo": _arte(18, 35, [".6.", "676", ".8."]),
    "derecha": _arte(18, 36, [".6.", "676", ".8."]),
})


# --- F. El Monje del Despertar --------------------------------------------------------------------

def habito_despertar(marfil, jade):
    """El habito del Despertar: marfil y sin mangas (los brazos al aire), cruzado al pecho con las solapas de
    jade, la faja de jade de cabos largos y las vendas de marfil en los antebrazos."""
    return {"partes": {"t": marfil, "n": marfil}, "mangas": "sin", "bajo": (marfil, 3),
            "extras": [cruce_gi(jade, None, 6), faja(jade, 6), brazales(marfil)]}


# La diadema del Despertar: el aro fino de oro (el del catalogo, material 1) y, en la frente, el tercer ojo
# abierto: los parpados de oro, el iris de jade (material 2) y la pupila oscura.
_OJO = _arte(24, 30, [
    ".414.",   # 24 el parpado de arriba
    "47Z74",   # 25 el iris y la pupila, sobre el aro
    ".434.",   # 26 el de abajo
])
OJO_DESPERTAR = _plantilla({
    "abajo": _OJO,
    "derecha": _arte(23, 39, ["14", "7Z", "34"]),
})


# --- el registro ---------------------------------------------------------------------------------

TORSO = {
    "it3_coraza_apostata": (coraza_apostata, ("acero_apostata", "tabardo_apostata", "rojo_apostata",
                                              "cadena_necromago"), "Coraza del Apostata"),
    # SETS-04: la casaca del Duque, en el sitio del manto del Senor de las Armas (ninguna fila se mueve)
    "it3_casaca_duque": (casaca_duque, ("negro_duque", "oro_duque", "rojo_duque", "mana_duque", "guante_duque",
                                        "hoja_duque", "gema_duque"), "Casaca del Duque"),
    "it3_coraza_furia": (coraza_furia, ("blanco_furia", "oro_arsenal", "oso_negro", "rojo_furia",
                                        "cuero_rojo_furia"), "Coraza de la Furia"),
    "it3_poncho_estaciones": (poncho_estaciones, ("lana_bosmurg", "lana_parda", "verde_primavera", "oro_verano",
                                                  "rojo_otono", "blanco_invierno"),
                              "Poncho de las Cuatro Estaciones"),
    "it3_chaqueta_chasqueador": (chaqueta_chasqueador, ("azul_chasqueador", "plata_chasqueador",
                                                        "violeta_chasqueador"), "Chaqueta del Chasqueador"),
    "it3_habito_despertar": (habito_despertar, ("marfil", "jade"), "Habito del Despertar"),
    # SETS-04: los otros dos sets del Senor de las Armas, al final
    "it3_placas_soberano": (placas_soberano, ("placa_soberano", "oro_soberano", "tabardo_soberano",
                                              "espectro_soberano"), "Placas del Soberano sin Rostro"),
    "it3_armadura_forja": (armadura_forja, ("hierro_rey", "oro_fundido", "brasa_forja", "cuero_forja",
                                            "hoja_duque"), "Armadura del Rey de la Forja"),
}
PIERNAS = {
    "it3_pantalones_estaciones": (equipo.pantalon, ("lana_parda",), "Pantalones de las Cuatro Estaciones"),
    "it3_pantalon_chasqueador": (pantalon_franja, ("azul_chasqueador", "violeta_chasqueador"),
                                 "Pantalon del Chasqueador"),
    "it3_pantalones_despertar": (hakama, ("jade", "marfil"), "Pantalones del Despertar"),
    # SETS-04: el Senor de las Armas
    "it3_calzas_duque": (calzas_duque, ("negro_duque", "oro_duque"), "Calzas del Duque"),
    "it3_grebas_soberano": (grebas_soberano, ("placa_soberano", "oro_soberano"), "Grebas del Soberano sin Rostro"),
    "it3_grebas_forja": (grebas_forja, ("hierro_rey", "cuero_forja", "oro_fundido"), "Grebas del Rey de la Forja"),
}
PIES = {
    "it3_botas_furia": (botas_piel, ("cuero_rojo_furia", "oso_negro", 4), "Botas de la Furia"),
    # SETS-04: el Senor de las Armas
    "it3_botas_duque": (botas_duque, ("guante_duque", "oro_duque"), "Botas del Duque"),
    "it3_escarpes_soberano": (escarpes_soberano, ("placa_soberano", "oro_soberano"),
                              "Escarpes del Soberano sin Rostro"),
    "it3_botas_forja": (botas_forja, ("hierro_rey", "oro_fundido"), "Botas del Rey de la Forja"),
}
CABEZA = {
    "it3_capucha_apostata": ([(CAPUCHA_COSECHADOR, None), (AUREOLA_ROTA, "aureola_apostata")], "granate_apostata",
                             "aureola_apostata", "Capucha del Apostata", False),
    # SETS-04: la diadema del Duque, en el sitio de la corona del Senor de las Armas (ninguna fila se mueve)
    "it3_diadema_duque": ([(cascos.DIADEMA, None)], "oro_duque", "gema_duque", "Diadema del Duque", False),
    "it3_yelmo_furia": ([(YELMO_FURIA, None), (RAYOS_FURIA, "oro_arsenal")], "blanco_furia", "oro_arsenal",
                        "Yelmo de la Furia", True),
    "it3_corona_estaciones": ([(CORONA_PRIMAVERA, "verde_primavera"), (CORONA_VERANO, "oro_verano"),
                               (CORONA_OTONO, "rojo_otono"), (CORONA_INVIERNO, "blanco_invierno"),
                               (FLORES_PRIMAVERA, "flor_primavera")], "verde_primavera", "verde_primavera",
                              "Corona de las Cuatro Estaciones", False),
    "it3_bombin_chasqueador": ([(BOMBIN, None), (ESTRELLA_BOMBIN, "plata_chasqueador")], "azul_chasqueador",
                               "violeta_chasqueador", "Bombin del Chasqueador", False),
    "it3_diadema_despertar": ([(cascos.DIADEMA, None), (OJO_DESPERTAR, "jade")], "oro_despertar", "jade",
                              "Diadema del Despertar", False),
    # SETS-04: el Senor de las Armas
    "it3_yelmo_soberano": ([(YELMO_SOBERANO, None), (VISERA_SOBERANO, "visera_soberano")], "placa_soberano",
                           "oro_soberano", "Yelmo del Soberano sin Rostro", True),
    "it3_corona_forja": ([(CORONA_FORJA, None)], "hierro_rey", "oro_fundido", "Corona del Rey de la Forja", False),
}

# El set de cada clase (las piezas del catalogo que se reutilizan van aqui sin su tinte: el tinte lo pone el tema
# del set en actores_estetica) y el arma, por su forma. El Senor de las Armas tiene tres (SETS-04): una lista de
# (nombre corto, piezas).
TRAJES = {
    "el_apostata": ("Apóstata", {"cabeza": "it3_capucha_apostata", "torso": "it3_coraza_apostata",
                                 "piernas": "quijotes", "pies": "botas_altas"}),
    # SETS-04: el Senor de las Armas tiene tres sets (el Duque, el Soberano sin Rostro y el Rey de la Forja)
    "senor_de_las_armas": [
        ("Duque", {"cabeza": "it3_diadema_duque", "torso": "it3_casaca_duque", "piernas": "it3_calzas_duque",
                   "pies": "it3_botas_duque"}),
        ("Soberano sin Rostro", {"cabeza": "it3_yelmo_soberano", "torso": "it3_placas_soberano",
                                 "piernas": "it3_grebas_soberano", "pies": "it3_escarpes_soberano"}),
        ("Rey de la Forja", {"cabeza": "it3_corona_forja", "torso": "it3_armadura_forja",
                             "piernas": "it3_grebas_forja", "pies": "it3_botas_forja"}),
    ],
    "furia_de_los_creadores": ("Furia", {"cabeza": "it3_yelmo_furia", "torso": "it3_coraza_furia",
                                         "piernas": "quijotes", "pies": "it3_botas_furia"}),
    "bardo_de_las_cuatro_estaciones": ("Cuatro Estaciones", {"cabeza": "it3_corona_estaciones",
                                                             "torso": "it3_poncho_estaciones",
                                                             "piernas": "it3_pantalones_estaciones",
                                                             "pies": "botas"}),
    "mago_chasqueador": ("Chasqueador", {"cabeza": "it3_bombin_chasqueador", "torso": "it3_chaqueta_chasqueador",
                                         "piernas": "it3_pantalon_chasqueador", "pies": "botas_altas"}),
    "monje_del_despertar": ("Despertar", {"cabeza": "it3_diadema_despertar", "torso": "it3_habito_despertar",
                                          "piernas": "it3_pantalones_despertar", "pies": "sandalias"}),
}


def registrar():
    """Suma las piezas a los catalogos de actores_equipo y actores_cascos (una vez)."""
    for ident, valor in TORSO.items():
        equipo.TORSO.setdefault(ident, valor)
    for ident, valor in PIERNAS.items():
        equipo.PIERNAS.setdefault(ident, valor)
    for ident, valor in PIES.items():
        equipo.PIES.setdefault(ident, valor)
    for ident, (plant, m1, m2, nombre, oculta) in CABEZA.items():
        plantillas = plant if isinstance(plant, list) else [(plant, None)]
        cascos.CABEZA.setdefault(ident, {"plantillas": plantillas, "m1": m1, "m2": m2,
                                         "nombre": nombre, "oculta_pelo": oculta})


registrar()
