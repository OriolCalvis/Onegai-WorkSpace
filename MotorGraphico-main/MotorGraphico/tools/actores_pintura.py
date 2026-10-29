"""Pintura por capas sobre el cuerpo base: tonos, materiales y prendas.

Cada pixel del cuerpo tiene un TONO (luz, base, sombra, contorno suave,
contorno duro) y una PARTE (actores_partes / actores_tallas). Una prenda
es un material de cinco tonos asignado a unas partes: se conserva el
volumen del cuerpo y el ciclo de caminar, y solo cambia el color.

Aqui todo se pinta en una CAPA: una imagen transparente del tamano del
frame en la que solo existen los pixeles de esa prenda. Las capas se
apilan despues en actores_capas.componer() con un orden por direccion.
"""
import numpy as np

# Tonos del cuerpo base -> clase de tono
TONO_BASE = {
    (255, 235, 198): "H",  # luz
    (250, 210, 169): "s",  # base
    (215, 139, 100): "d",  # sombra
    (119, 67, 46): "o",    # contorno suave
    (87, 46, 29): "O",     # contorno duro
    (73, 42, 30): "X",     # contorno del parpado / suela
}
OJO = {  # iris (oscuro -> claro) y blanco/brillo
    (20, 85, 105): "i1", (25, 112, 140): "i2", (26, 124, 185): "i3",
    (173, 209, 229): "w1", (191, 218, 235): "w2", (221, 235, 244): "w3",
}
PARTES_PIERNA = ("p", "l", "L", "f", "F")


def hexrgb(h):
    h = h.lstrip("#")
    return tuple(int(h[i:i + 2], 16) for i in (0, 2, 4))


class Material:
    """Rampa de cinco tonos: luz, base, sombra, contorno suave, contorno duro."""

    def __init__(self, nombre, luz, base, sombra, contorno, contorno_duro=None, alfa=255):
        self.nombre = nombre
        c = [hexrgb(v) for v in (luz, base, sombra, contorno)]
        cd = hexrgb(contorno_duro) if contorno_duro else tuple(max(0, int(v * 0.72)) for v in c[3])
        self.tono = {"H": c[0], "s": c[1], "d": c[2], "o": c[3], "O": cd, "X": cd}
        self.alfa = alfa  # < 255: materiales translucidos (espectros)

    def __getitem__(self, t):
        return self.tono[t]


def clasificar(frame_px):
    """Matriz de clases de tono ('' = vacio, 'e:<clave>' = ojo)."""
    h, w = frame_px.shape[:2]
    tono = np.full((h, w), "", dtype="<U4")
    for y in range(h):
        for x in range(w):
            if frame_px[y, x, 3] == 0:
                continue
            rgb = tuple(int(v) for v in frame_px[y, x, :3])
            if rgb in TONO_BASE:
                tono[y, x] = TONO_BASE[rgb]
            elif rgb in OJO:
                tono[y, x] = "e:" + OJO[rgb]
            else:
                raise ValueError(f"color no previsto en el cuerpo base: {rgb} en {x},{y}")
    return tono


def suavizar_tonos(tono, mascara):
    """Quita el detalle anatomico de 1 px dentro de una prenda.

    Un pixel de luz o sombra aislado (sin vecino del mismo tono en cruz)
    dentro de la mascara pasa a tono base. Los contornos no se tocan.
    """
    out = tono.copy()
    for t in ("H", "d"):
        sel = mascara & (tono == t)
        vecinos = np.zeros(sel.shape, np.int8)
        vecinos[:-1] += sel[1:]    # abajo
        vecinos[1:] += sel[:-1]    # arriba
        vecinos[:, :-1] += sel[:, 1:]  # derecha
        vecinos[:, 1:] += sel[:, :-1]  # izquierda
        out[sel & (vecinos == 0)] = "s"
    return out


def es_contorno(t):
    return t in ("o", "O", "X")


def fila_cinturon(lab):
    ys = np.nonzero((lab == "p").any(axis=1))[0]
    return int(ys.min()) if len(ys) else None


def top_pie(lab, etiqueta):
    ys = np.nonzero((lab == etiqueta).any(axis=1))[0]
    return int(ys.min()) if len(ys) else None


# --- pintar en la capa -------------------------------------------------------

def cubierto(ctx, y, x):
    """Si la prenda ya ha pintado (y, x). PL-ROPA-01: en PixelLab solo cuenta dentro del cuerpo (+1 px): las
    mangas anchas que cuelgan fuera no ensanchan la faja ni el cinturon."""
    if (y, x) not in ctx["cubre"]:
        return False
    limite = ctx.get("limite")
    return limite is None or bool(limite[y, x])


def pintar(ctx, y, x, m, t, alfa=None):
    if not (0 <= y < ctx["px"].shape[0] and 0 <= x < ctx["px"].shape[1]):
        return
    limite = ctx.get("limite")
    if limite is not None and not limite[y, x]:
        return  # PL-ROPA-01: fuera del cuerpo no se pinta: hombreras y fajines de la vista recta que flotaban
    ctx["px"][y, x, :3] = m[t]
    ctx["px"][y, x, 3] = m.alfa if alfa is None else alfa


def limite_cuerpo(talla, f, cint, flotante=False):
    """PL-ROPA-01: hasta donde puede pintar en el frame f una prenda que no es de cabeza (torso, piernas,
    pies). El cuerpo y 1 px alrededor (2 del cinturon para abajo); si la prenda es «flotante», todo.
    Y nunca la cara (Oriol, 26/10: «vigila en tapar la cara amb objectes que no son de cap», «d'esquenes
    esta bé tapar el cap»): de frente (sureste, suroeste) nada en las filas de la cabeza; de espaldas, lo
    de siempre (un cuello alto o una capucha echada pueden subir por la nuca). Una prenda flotante pierde
    de frente solo la cabeza en si y 1 px alrededor: sus hojas y orbes flotan al lado, no encima."""
    from scipy import ndimage
    lab = talla.labs[f]
    alfa_ = np.asarray(talla.alfa[f]) > 0
    if flotante:
        limite = np.ones(alfa_.shape, bool)
    else:
        limite = ndimage.binary_dilation(alfa_, iterations=1)
        if cint is not None:
            limite[cint:] = ndimage.binary_dilation(alfa_, iterations=2)[cint:]
    cabeza = lab == "h"
    diag = talla.diagonal(f) if hasattr(talla, "diagonal") else ""
    if not cabeza.any() or not diag.startswith("sur"):
        return limite
    if flotante:
        limite &= ~ndimage.binary_dilation(cabeza, iterations=1)
        return limite
    filas = np.nonzero(cabeza.any(axis=1))[0]
    limite[filas.min():filas.max() + 1] = False
    return limite


def fila_cinturon_x(lab, cint):
    """PL-ROPA-01: la fila del cinturon en cada columna. En diagonal la cadera va inclinada y la pelvis (p)
    empieza una fila mas abajo en un lado que en el otro: el cinturon y el bajo de la prenda la siguen. En
    las vistas rectas de siempre la pelvis es horizontal y sale cint en todas las columnas."""
    if cint is None:
        return None
    out = np.full(lab.shape[1], cint, int)
    for x in range(lab.shape[1]):
        col = np.nonzero(lab[:, x] == "p")[0]
        if len(col) and cint <= col.min() <= cint + 2:
            out[x] = int(col.min())
    return out


def contexto(talla, f, materiales):
    """Contexto de pintura de un frame: capa vacia + datos del cuerpo."""
    lab = talla.labs[f]
    cint = fila_cinturon(lab)
    diagonal = talla.diagonal(f) if hasattr(talla, "diagonal") else None
    return {"px": np.zeros((talla.alto, talla.ancho, 4), np.uint8), "lab": lab,
            "tono": talla.tonos[f].copy(), "dir": talla.direccion(f), "mat": materiales,
            "cint": cint, "cint_x": fila_cinturon_x(lab, cint), "diag": diagonal,
            "cubre": {}, "alfa": talla.alfa[f], "ojos": talla.ojos[f], "talla": talla, "frame": f}


def pintar_prenda(talla, f, prenda, materiales):
    """Capa RGBA de una prenda en el frame f.

    prenda (dict):
      partes    {etiqueta: material}  lo que sigue la silueta
      mangas    "largas" | "cortas" | "sin"  cuanto brazo cubren las partes a/A
      bajo      (material, filas)     faldon bajo el cinturon (sobre las piernas)
      habito    material              faldon hasta los tobillos, relleno entre piernas
      botas     (material, filas)     pies + cana por encima del pie
      cinturon  (material, material_hebilla|None, cabo)
      texturas  {material: funcion(y, x, tono, ctx) -> tono}  malla, escamas...
      extras    [funcion(ctx)]        correas, tabardos, chalecos, delantales...
    """
    ctx = contexto(talla, f, materiales)
    lab, cint = ctx["lab"], ctx["cint"]
    mat = ctx["cubre"]
    if hasattr(talla, "esqueletos"):
        # PL-ROPA-01 (Oriol, 25/10, el catalogo en diagonal): los extras de las corazas (hombreras, fajines,
        # placas) estan pensados para la vista recta y en diagonal sobresalian 3-5 px del cuerpo por el lado
        # lejano. Nada de la prenda se pinta a mas de 1 px del cuerpo (2 del cinturon para abajo, el vuelo de
        # los faldones), salvo que la prenda sea «flotante» (las hojas del duque y del soberano, los orbes
        # del gravitas, las alas del angel y del demonio: de diseno).
        ctx["limite"] = limite_cuerpo(talla, f, cint, prenda.get("flotante", False))
    mangas = prenda.get("mangas", "largas")
    hombro = None
    brazo_t = getattr(talla, "brazo_t", {}).get(f)  # PL-ROPA-01: la manga corta llega al codo por el hueso
    if mangas != "largas":
        filas_t = np.nonzero((lab == "t").any(axis=1))[0]
        hombro = int(filas_t.min()) if len(filas_t) else 0
    for e, nombre in prenda.get("partes", {}).items():
        m = materiales[nombre]
        ys, xs = np.nonzero(lab == e)
        for y, x in zip(ys.tolist(), xs.tolist()):
            if e in ("a", "A") and hombro is not None:
                if mangas == "sin":
                    continue
                if brazo_t is not None and not np.isnan(brazo_t[y, x]):
                    if brazo_t[y, x] > 0.5:
                        continue
                elif y > hombro + 5:
                    continue
            mat[(y, x)] = m
    if "bajo" in prenda and cint is not None:
        m, filas = prenda["bajo"]
        cx = ctx["cint_x"]
        for y in range(cint + 1, min(talla.alto, cint + 3 + filas)):
            for x in np.nonzero(np.isin(lab[y], ("p", "l", "L")))[0].tolist():
                if cx[x] < y <= cx[x] + filas:
                    mat[(y, x)] = materiales[m]
    if "botas" in prenda:
        m, cana = prenda["botas"]
        for pierna, pie in (("l", "f"), ("L", "F")):
            tp = top_pie(lab, pie)
            if tp is None:
                continue
            for y in range(max(0, tp - cana), talla.alto):
                for x in np.nonzero(np.isin(lab[y], (pierna, pie)))[0].tolist():
                    mat[(y, x)] = materiales[m]
    tono = ctx["tono"]
    mascaras = {}
    for (y, x), v in mat.items():
        if id(v) not in mascaras:
            mascaras[id(v)] = np.zeros(lab.shape, bool)
        mascaras[id(v)][y, x] = True
    for mascara in mascaras.values():
        tono = np.where(mascara, suavizar_tonos(tono, mascara), tono)
    ctx["tono"] = tono
    texturas = {materiales[k]: fn for k, fn in prenda.get("texturas", {}).items()}
    for (y, x), m in mat.items():
        t = tono[y, x]
        if t and not t.startswith("e:"):
            if m in texturas and not es_contorno(t):
                t = texturas[m](y, x, t, ctx)
            pintar(ctx, y, x, m, t)
    if "habito" in prenda and cint is not None:
        hab = prenda["habito"]
        if isinstance(hab, tuple):  # (material, vuelo, hasta)
            habito(ctx, materiales[hab[0]], hab[1], hab[2])
        else:
            habito(ctx, materiales[hab])
    if "cinturon" in prenda and cint is not None:
        cinturon(ctx, *prenda["cinturon"])
    for fn in prenda.get("extras", []):
        fn(ctx)
    return ctx["px"]


def cinturon(ctx, material, hebilla=None, cabo=0):
    """Una fila de cuero (o cuerda) sobre la linea del cinturon (PL-ROPA-01: la de cada columna, que en
    diagonal va inclinada; la hebilla, en el lado cercano a la camara)."""
    lab, tono, cint, d = ctx["lab"], ctx["tono"], ctx["cint"], ctx["dir"]
    cx_ = ctx["cint_x"]
    mc = ctx["mat"][material]
    xs = [x for x in range(lab.shape[1]) if lab[cx_[x], x] in ("p", "t")]
    if not xs:
        return
    for x in xs:
        t = tono[cx_[x], x]
        pintar(ctx, cx_[x], x, mc, "O" if es_contorno(t) else ("d" if t == "d" else "s"))
    interior = [x for x in xs if not es_contorno(tono[cx_[x], x])]
    if not interior or d == "arriba":
        return
    diag = ctx.get("diag")
    if diag == "sureste":
        hx = interior[(len(interior) * 2) // 3]
    elif diag == "suroeste":
        hx = interior[len(interior) // 3]
    elif d == "abajo":
        hx = interior[len(interior) // 2]
    elif d == "derecha":
        hx = interior[-1]
    else:
        hx = interior[0]
    hy = cx_[hx]
    if hebilla:
        pintar(ctx, hy, hx, ctx["mat"][hebilla], "H")
    for k in range(1, cabo + 1):  # cabo de la cuerda colgando
        y = hy + k
        if y < lab.shape[0] and (ctx["alfa"][y, hx] or cubierto(ctx, y, hx)):
            pintar(ctx, y, hx, mc, "s" if k < cabo else "d")


def habito(ctx, m, vuelo=1, hasta=None):
    """Faldon hasta los tobillos: rellena el hueco entre las piernas.

    La tela del hueco es continua (tono base) con un pliegue central en
    sombra; la silueta se ensancha 'vuelo' px en la mitad baja. hasta
    corta el faldon antes (0..1 del camino del cinturon al empeine: una
    falda por la rodilla es 0.6).
    """
    lab, tono, cint = ctx["lab"], ctx["tono"], ctx["cint"]
    pies = [v for v in (top_pie(lab, "f"), top_pie(lab, "F")) if v is not None]
    bajo = min(pies) if pies else lab.shape[0] - 1
    if hasta is not None:
        bajo = cint + max(2, int(round((bajo - cint) * hasta)))
    alto = max(1, bajo - cint)
    for y in range(cint + 1, bajo + 1):
        xs = [x for x in range(lab.shape[1]) if lab[y, x] in PARTES_PIERNA]
        if not xs:
            continue
        ens = vuelo if (y - cint) * 2 >= alto else 0
        x0, x1 = min(xs) - ens, max(xs) + ens
        huecos = [x for x in range(x0 + 1, x1) if lab[y, x] not in PARTES_PIERNA]
        pliegue = huecos[len(huecos) // 2] if huecos else None
        for x in range(x0, x1 + 1):
            if x in (x0, x1) or y == bajo:
                t = "O"
            elif x == pliegue:
                t = "d"
            elif lab[y, x] in PARTES_PIERNA and tono[y, x] in ("H", "s", "d"):
                t = tono[y, x]
            else:
                t = "s"
            pintar(ctx, y, x, m, t)
            ctx["cubre"][(y, x)] = m


# --- detalles de prenda (extras) --------------------------------------------

def _tramo_torso(lab, tono, y):
    xs = [x for x in range(lab.shape[1]) if lab[y, x] == "t" and not es_contorno(tono[y, x])]
    return (min(xs), max(xs)) if xs else None


def filas_torso(ctx):
    lab, cint = ctx["lab"], ctx["cint"]
    ys = [y for y in range(lab.shape[0]) if (lab[y] == "t").any()]
    return (min(ys), cint) if ys else (None, None)


def bandolera(material):
    """Correa cruzada del hombro a la cadera (dos pixeles: cuero y su sombra)."""
    def fn(ctx):
        lab, tono, d = ctx["lab"], ctx["tono"], ctx["dir"]
        m = ctx["mat"][material]
        y0, y1 = filas_torso(ctx)
        if y0 is None:
            return
        ini, fin = {"abajo": (0.1, 0.9), "arriba": (0.9, 0.1),
                    "derecha": (0.25, 0.8), "izquierda": (0.75, 0.2)}[d]
        y0 += 1
        for y in range(y0, y1):
            tr = _tramo_torso(lab, tono, y)
            if not tr:
                continue
            f = ini + (fin - ini) * (y - y0) / max(1, (y1 - 1 - y0))
            x = int(round(tr[0] + f * (tr[1] - tr[0])))
            for dx, t in ((0, "s"), (1, "d")):
                if lab[y, x + dx] == "t" and not es_contorno(tono[y, x + dx]):
                    pintar(ctx, y, x + dx, m, t if tono[y, x + dx] != "H" or t == "d" else "H")
    return fn


def tabardo(material_malla, material_emblema):
    """Tabardo de la guardia: hombros de malla y una cruz en el pecho."""
    def fn(ctx):
        lab, tono, d = ctx["lab"], ctx["tono"], ctx["dir"]
        malla, oro = ctx["mat"][material_malla], ctx["mat"][material_emblema]
        y0, y1 = filas_torso(ctx)
        if y0 is None:
            return
        for y in range(y0, y0 + 3):  # hombros de malla (patron de anillas)
            for x in range(lab.shape[1]):
                if lab[y, x] == "t":
                    t = tono[y, x]
                    pintar(ctx, y, x, malla, "O" if es_contorno(t) else ("s" if (x + y) % 2 else "d"))
        if d not in ("abajo", "arriba"):
            return
        tr = _tramo_torso(lab, tono, y0 + 5)
        if not tr:
            return
        cx = (tr[0] + tr[1]) // 2
        for y in range(y0 + 4, y1):
            if lab[y, cx] == "t":
                pintar(ctx, y, cx, oro, "s")
        yb = y0 + 6
        for x in (cx - 1, cx + 1):
            if lab[yb, x] == "t":
                pintar(ctx, yb, x, oro, "d")
    return fn


def chaleco(material):
    """Chaleco abierto: cubre el torso salvo la camisa del centro (o del frente)."""
    def fn(ctx):
        lab, tono, d = ctx["lab"], ctx["tono"], ctx["dir"]
        m = ctx["mat"][material]
        y0, y1 = filas_torso(ctx)
        if y0 is None:
            return
        for y in range(y0 + 1, y1):
            tr = _tramo_torso(lab, tono, y)
            if not tr:
                continue
            cx = (tr[0] + tr[1]) / 2
            for x in range(lab.shape[1]):
                if lab[y, x] != "t":
                    continue
                t = tono[y, x]
                abierto = ((d == "abajo" and abs(x - cx) <= 1) or
                           (d == "derecha" and x >= tr[1] - 1) or
                           (d == "izquierda" and x <= tr[0] + 1))
                if abierto:
                    continue
                borde = ((d == "abajo" and abs(x - cx) <= 2) or
                         (d == "derecha" and x == tr[1] - 2) or
                         (d == "izquierda" and x == tr[0] + 2))
                if es_contorno(t):
                    pintar(ctx, y, x, m, "O")
                else:
                    pintar(ctx, y, x, m, "d" if borde else ("s" if t == "H" else t))
    return fn


def delantal(material):
    """Delantal de cintura: de frente cubre el centro de la falda; de lado, el
    borde delantero; de espaldas solo se ve el lazo sobre el cinturon."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        m = ctx["mat"][material]
        pies = [v for v in (top_pie(lab, "f"), top_pie(lab, "F")) if v is not None]
        if cint is None or not pies:
            return
        fondo = min(pies) - 3
        for y in range(cint, fondo + 1):
            xs = [x for x in range(lab.shape[1]) if cubierto(ctx, y, x)]
            if not xs:
                continue
            x0, x1 = min(xs), max(xs)
            if d == "arriba":
                if y == cint:  # lazo
                    cx = (x0 + x1) // 2
                    for x in (cx - 1, cx, cx + 1):
                        pintar(ctx, y, x, m, "s" if x != cx else "d")
                continue
            if d == "abajo":
                cx = (x0 + x1) / 2
                a, b = int(round(cx - 3)), int(round(cx + 3))
            elif d == "derecha":
                a, b = x1 - 3, x1 - 1
            else:
                a, b = x0 + 1, x0 + 3
            for x in range(a, b + 1):
                borde = x in (a, b) or y == fondo
                pintar(ctx, y, x, m, "d" if borde else ("H" if y == cint + 1 else "s"))
    return fn
