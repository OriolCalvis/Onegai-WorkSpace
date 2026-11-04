"""Oriol (24/10): las armas y lo de la otra mano en las cuatro diagonales de PixelLab.

Las armas se dibujaron de frente y de perfil (actores_armas: «abajo» y «derecha», con las poses de
actores_poses para cada paso). En diagonal se usa el dibujo de perfil: hacia la derecha de la pantalla
(sureste, noreste) tal cual y hacia la izquierda (suroeste, noroeste) en espejo. El puno es la mano del
esqueleto de PixelLab (la muñeca derecha para el arma, la izquierda para la otra mano), y el arma va
delante o detras del cuerpo segun esa muñeca este mas cerca o mas lejos de la camara que los hombros:
cada frame trae su orden propio («f<n>», como los de la Forja).

    capa_arma_pl(talla, ident, materiales, clasicas)  -> (orden, frames) como actores_armas.capa_arma
"""
import math

import numpy as np
from scipy import ndimage

import actores_armas as armas
import pixellab_cuerpo as pc

# El cuerpo de siempre del que se cogen las formas de cada familia (el tamaño de las armas)
CLASICA = {"medio": "medio", "bajo": "bajo", "pequeno": "pequeno", "grande": "grande",
           "naga": "medio", "aarakocra": "medio", "espectro": "medio", "minotauro": "grande"}
# Que dibujo del arma lleva cada frame de PixelLab (andar de 6 y respirar de 4)
DIBUJO_ANDAR = ("A", "A", "quieto", "B", "B", "quieto")
DIBUJO_RESPIRAR = ("reposo1", "reposo1", "reposo2", "reposo2")
DELANTE, DETRAS = 80, 8
# PL-ROPA-01 (Oriol, 25/10, «las animaciones de las armas se pueden rotar cada una un poco diferente dentro de
# una animacion»): el arma gira con el antebrazo. En cada frame, lo que el antebrazo (codo -> muneca) del
# esqueleto se desvia del frame quieto de su diagonal, en pasos de GIRO_PASO grados y hasta GIRO_MAX.
GIRO_PASO, GIRO_MAX = 10, 20


def _kp(talla, f):
    return {p["label"]: (p["x"] * talla.ancho, p["y"] * talla.alto, p.get("z_index", 0.0))
            for p in talla.esqueletos[f]}


def dibujo_de(f):
    _d, anim, i = pc.reparto()[f - 1]
    return DIBUJO_ANDAR[i] if anim == pc.ANDAR else DIBUJO_RESPIRAR[i]


def angulo_antebrazo(talla, f, lado):
    """Grados del antebrazo desde la vertical (positivo hacia la derecha de la pantalla)."""
    k = _kp(talla, f)
    e, w = k[f"{lado} ELBOW"], k[f"{lado} ARM"]
    return math.degrees(math.atan2(w[0] - e[0], w[1] - e[1]))


def giro_de(talla, f, lado, vista):
    """El giro del arma en el frame f: el antebrazo respecto al del frame quieto (el primero de respirar) de
    la misma diagonal, en pasos de GIRO_PASO y hasta GIRO_MAX; en espejo (vista izquierda), al reves."""
    d = talla.diagonal(f)
    quieto = next(q for q in range(1 + len(pc.RUMBOS) * pc.FRAMES_ANDAR, talla.n_frames + 1) if talla.diagonal(q) == d)
    delta = angulo_antebrazo(talla, f, lado) - angulo_antebrazo(talla, quieto, lado)
    if vista == "izquierda":
        delta = -delta
    return int(max(-GIRO_MAX, min(GIRO_MAX, round(delta / GIRO_PASO) * GIRO_PASO)))


_GIROS = {}


def rotar_forma(filas, grados):
    """La forma (filas de caracteres) girada; se recuerda por forma y grados (giro_cara prueba muchos)."""
    clave = (tuple(filas), grados)
    if clave not in _GIROS:
        _GIROS[clave] = _rotar_forma(filas, grados)
    return _GIROS[clave]


def _rotar_forma(filas, grados):
    """La forma (filas de caracteres de actores_armas) girada `grados` alrededor del puno ('@', '&' o '$'),
    en sentido horario en pantalla (y hacia abajo). Muestreo inverso al vecino mas cercano: cada pixel del
    dibujo girado toma el caracter del pixel de origen que cae en su centro; el puno no se mueve."""
    if not grados:
        return filas
    H = len(filas)
    W = max(len(f) for f in filas)
    src = np.full((H, W), ".", dtype="<U1")
    for i, f in enumerate(filas):
        for j, c in enumerate(f):
            src[i, j] = c
    ay, ax = next((i, j) for i in range(H) for j in range(W) if src[i, j] in armas.ANCLAS)
    ocupado = src != "."
    ys, xs = np.nonzero(ocupado)
    r = int(math.ceil(max(np.hypot(xs - ax, ys - ay).max(), 1)))
    # el dibujo girado cabe en un cuadrado de radio r alrededor del puno
    H2, W2 = 2 * r + 1, 2 * r + 1
    ay2, ax2 = r, r
    th = math.radians(grados)
    cos, sin = math.cos(th), math.sin(th)
    ii, jj = np.mgrid[0:H2, 0:W2]
    dx, dy = jj - ax2, ii - ay2
    # giro inverso: donde estaba cada pixel en el original
    sj = np.rint(ax + (dx * cos + dy * sin)).astype(int)
    si = np.rint(ay + (-dx * sin + dy * cos)).astype(int)
    ok = (si >= 0) & (si < H) & (sj >= 0) & (sj < W)
    out = np.full((H2, W2), ".", dtype="<U1")
    out[ok] = src[si[ok], sj[ok]]
    out[ay2, ax2] = src[ay, ax]  # el puno
    # quitar filas y columnas vacias por los bordes, sin perder el puno
    filas_ok = [i for i in range(H2) if (out[i] != ".").any()]
    cols_ok = [j for j in range(W2) if (out[:, j] != ".").any()]
    i0, i1 = min(filas_ok + [ay2]), max(filas_ok + [ay2])
    j0, j1 = min(cols_ok + [ax2]), max(cols_ok + [ax2])
    return ["".join(out[i, j0:j1 + 1]) for i in range(i0, i1 + 1)]


_PUNTOS = {}


def _puntos(filas):
    """(dy, dx) de cada pixel de la forma respecto al puno, una vez por forma."""
    clave = tuple(filas)
    if clave not in _PUNTOS:
        ay = next(i for i, f in enumerate(filas) if any(c in f for c in armas.ANCLAS))
        ax = next(filas[ay].index(c) for c in armas.ANCLAS if c in filas[ay])
        dy, dx = [], []
        for i, fila in enumerate(filas):
            for j, c in enumerate(fila):
                if c != ".":
                    dy.append(i - ay)
                    dx.append(j - ax)
        _PUNTOS[clave] = (np.array(dy, int), np.array(dx, int))
    return _PUNTOS[clave]


def _huella(filas, vista, ancla, alto, ancho):
    """Donde caeria la forma pintada en la celda (mascara) y cuantos pixeles se quedan fuera de ella."""
    dy, dx = _puntos(filas)
    x = ancla[0] + (-dx if vista == "izquierda" else dx)
    y = ancla[1] + dy
    dentro = (x > 0) & (x < ancho - 1) & (y > 0) & (y < alto - 1)
    m = np.zeros((alto, ancho), bool)
    m[y[dentro], x[dentro]] = True
    return m, int((~dentro).sum())


# PL-ROPA-01 (Oriol, 26/10, «rotar algunas animaciones para que las armas no pasen tanto por la cara»): de
# frente, si el arma en la mano cercana pasa por la cabeza, la diagonal entera gira el arma un poco mas (pasos
# de GIRO_PASO, hasta ESQUIVA_MAX) hacia el lado que la saca de la cara, siempre que no se salga de la celda.
# Entre los giros que la sacan, el que menos cuerpo tapa y menos gira (un hacha al hombro que pasa por la cara
# acaba en alto junto a la cabeza, no cruzada sobre el pecho). Lo que aun quede sobre la cabeza lo tapa la
# cabeza.
ESQUIVA_MAX = 90


def giro_cara(talla, formas, lado):
    """{diagonal de frente: grados extra} para que el arma no pase por la cabeza (dilatada 1 px) en ningun
    frame de esa diagonal donde la mano este cerca. El menor giro (en valor absoluto) que lo consigue; si
    ninguno lo consigue del todo, el que menos cabeza tapa. 0 si ya no toca."""
    out = {}
    por_d = {}
    for f in range(1, talla.n_frames + 1):
        d = talla.diagonal(f)
        if not d.startswith("sur"):
            continue
        etiqueta, ancla, cerca = mano_de(talla, f, lado)
        if not cerca:
            continue
        forma = formas.get(dibujo_de(f)) or formas.get("quieto")
        if forma is None:
            continue
        vista = "derecha" if d == "sureste" else "izquierda"
        lab = talla.labs[f]
        cabeza = ndimage.binary_dilation(lab == "h", iterations=1)
        tronco = np.isin(lab, ["h", "n", "t", "p", "l", "L"])  # el arma gira hacia fuera, no hacia el cuerpo
        por_d.setdefault(d, []).append((forma["derecha"], vista, ancla, cabeza, tronco, giro_de(talla, f, lado, vista)))
    for d, frames in por_d.items():
        def coste(extra):
            tapa, fuera, cuerpo = 0, 0, 0
            for filas, vista, ancla, cabeza, tronco, giro in frames:
                g = giro + extra
                m, fu = _huella(rotar_forma(filas, g) if g else filas, vista, ancla, talla.alto, talla.ancho)
                tapa += int((m & cabeza).sum())
                cuerpo += int((m & tronco).sum())
                fuera += fu
            return tapa, fuera, cuerpo
        tapa0, fuera0, cuerpo0 = coste(0)
        if not tapa0:
            continue
        n = len(frames)
        mejor = (tapa0, cuerpo0 / n, 0)
        for paso in range(GIRO_PASO, ESQUIVA_MAX + 1, GIRO_PASO):
            for extra in (paso, -paso):
                tapa, fuera, cuerpo = coste(extra)
                if fuera > fuera0:
                    continue  # se saldria de la celda
                mejor = min(mejor, (tapa, cuerpo / n + paso / 5, extra))
        if mejor[2] and mejor[0] < tapa0:
            out[d] = mejor[2]
    return out


def mano_de(talla, f, lado):
    """(etiqueta de la mano en el mapa de partes, ancla del puno, si esta mas cerca de la camara que
    los hombros) de la mano 'RIGHT' o 'LEFT'."""
    k = _kp(talla, f)
    lab = talla.labs[f]
    mx, my, mz = k[f"{lado} ARM"]  # COCO: «ARM» es la muñeca
    mejor = None
    for et in ("m", "M"):
        ys, xs = np.nonzero(lab == et)
        if len(ys):
            d = np.hypot(xs.mean() - mx, ys.mean() - my)
            if mejor is None or d < mejor[0]:
                mejor = (d, et)
    etiqueta = mejor[1] if mejor else ("M" if lado == "RIGHT" else "m")
    otra = "m" if etiqueta == "M" else "M"
    ancla = armas._ancla(lab, etiqueta, otra)
    if ancla is None or (mejor and mejor[0] > 4):
        ancla = (int(round(mx)), int(round(my)))
    hombros = (k["LEFT SHOULDER"][2] + k["RIGHT SHOULDER"][2]) / 2
    return etiqueta, ancla, mz >= hombros


_CLASICAS = {}


def clasicas():
    """Las tallas de siempre (de ahi salen las formas de las armas), una vez."""
    if not _CLASICAS:
        from actores_tallas import construir_tallas
        _CLASICAS.update(construir_tallas())
    return _CLASICAS


def capa_arma_pl(talla, ident, materiales, clasicas=None):
    clasicas = clasicas or globals()["clasicas"]()
    familia, n1, n2 = armas.ARMAS[ident][:3]
    n3 = armas.ARMAS[ident][4] if len(armas.ARMAS[ident]) > 4 else None
    n4 = armas.ARMAS[ident][5] if len(armas.ARMAS[ident]) > 5 else None
    ranura = armas.ranura_de(ident)
    m1, m2 = materiales[n1], materiales[n2]
    m3 = materiales[n3] if n3 else None
    m4 = materiales[n4] if n4 else None
    if familia in armas.PAREJA:
        return dict(armas.ORDEN_PAREJA), capa_pareja_pl(talla, armas.FORMAS[familia], m1, m2, m3, m4)
    formas = armas.formas_en_talla(clasicas[CLASICA.get(talla.clave, "medio")], familia, ranura)
    lado = "RIGHT" if ranura == "arma" else "LEFT"
    orden = dict(armas.ORDEN_MANOS[ranura])
    esquiva = giro_cara(talla, formas, lado) if ranura == "arma" else {}
    frames = []
    for f in range(1, talla.n_frames + 1):
        d = talla.diagonal(f)
        vista = "derecha" if d in ("sureste", "noreste") else "izquierda"
        etiqueta, ancla, cerca = mano_de(talla, f, lado)
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        forma = formas.get(dibujo_de(f)) or formas.get("quieto")
        if forma is not None and ranura == "arma":
            giro = giro_de(talla, f, lado, vista) + (esquiva.get(d, 0) if cerca else 0)
            if giro:
                forma = dict(forma, derecha=rotar_forma(forma["derecha"], giro))
        if forma is not None:
            lab = talla.labs[f]
            # PL-ROPA-01 (Oriol, 26/10): de frente, la cabeza (y 1 px alrededor, el pelo) tapa el arma que
            # va delante del cuerpo: la hoja del hacha al hombro, el estandarte o el baston alzados no pasan
            # por la cara (como TAPA_LA_CABEZA en la vista recta, aqui para todas las armas). De espaldas
            # no: lo que sube por encima del hombro cercano queda delante de la nuca.
            oculta = None
            if cerca and d.startswith("sur"):
                oculta = ndimage.binary_dilation(lab == "h", iterations=1)
            armas.pintar_arma(capa, forma, vista, ancla, m1, m2, lab if cerca else None,
                              etiqueta if cerca else None, m3, m4, oculta,
                              np.asarray(talla.alfa[f]) > 0 if cerca else None)
        orden[f"f{f}"] = DELANTE if cerca else DETRAS
        frames.append(capa)
    return orden, frames


def capa_pareja_pl(talla, forma, m1, m2, m3=None, m4=None):
    """PL-JUEGO-01 (SP-02 en diagonal): un guantelete (garra, tambor...) en cada mano. Como las armas, de
    perfil: tal cual hacia la derecha de la pantalla y en espejo hacia la izquierda, en el puno de cada
    muneca del esqueleto. Van encima del cuerpo (ORDEN_PAREJA); el de la mano que queda mas lejos de la
    camara que los hombros se pinta primero y solo sobre su mano, su brazo o fuera del cuerpo (lo demas lo
    tapa el cuerpo). Una pareja con un dibujo en cada mano («abajo_2», «derecha_2»: el canon del
    Tronador) lleva el segundo en la izquierda."""
    frames = []
    for f in range(1, talla.n_frames + 1):
        d = talla.diagonal(f)
        vista = "derecha" if d in ("sureste", "noreste") else "izquierda"
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        manos = [(lado,) + mano_de(talla, f, lado) for lado in ("RIGHT", "LEFT")]
        manos.sort(key=lambda m: m[3])  # la de lejos primero
        lab = talla.labs[f]
        alfa = np.asarray(talla.alfa[f]) > 0
        for lado, etiqueta, ancla, cerca in manos:
            dibujo = forma
            if "abajo_2" in forma and lado == "LEFT":
                dibujo = {"abajo": forma["abajo_2"], "derecha": forma["derecha_2"]}
            oculta = None if cerca else alfa & ~np.isin(lab, [etiqueta, armas.brazo_de(etiqueta)])
            if cerca and d.startswith("sur"):  # PL-ROPA-01 (26/10): de frente, tampoco la pareja tapa la cara
                oculta = ndimage.binary_dilation(lab == "h", iterations=1)
            armas.pintar_arma(capa, dibujo, vista, ancla, m1, m2, None, None, m3, m4, oculta)
        frames.append(capa)
    return frames

