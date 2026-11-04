"""Oriol (24/10): los personajes nuevos en diagonal de PixelLab, vestidos con nuestra ropa por capas.

PixelLab da el cuerpo desnudo de cada raza en las cuatro diagonales (andar y respirar). Para pintar
encima la ropa de siempre (actores_equipo, actores_trajes...) hace falta lo mismo que da el cuerpo
base (actores_tallas.Talla): el mapa de partes de cada frame, la clase de tono de cada pixel y los
ojos. Aqui se sacan del esqueleto que estima PixelLab (/v2/estimate-skeleton, COCO-18 con
profundidad): cada pixel opaco va a la parte cuyo hueso le queda mas cerca, y si dos se tocan,
gana la que esta mas cerca de la camara.

    TallaPL(frames, esqueletos, direcciones)  -> se usa como una Talla en pintar_prenda

Etiquetas (las de actores_partes): h cabeza, n cuello, t torso, p pelvis, a/A brazo, m/M mano,
l/L pierna, f/F pie. Mayuscula = el lado cercano a la camara (como en los perfiles).
"""
import numpy as np

DIAGONALES = {"south-east": "sureste", "south-west": "suroeste",
              "north-east": "noreste", "north-west": "noroeste"}


def _kp(esqueleto, ancho, alto):
    return {k["label"]: (k["x"] * ancho, k["y"] * alto, k.get("z_index", 0.0)) for k in esqueleto}


def _dist_segmento(px, py, a, b):
    ax, ay = a[0], a[1]
    bx, by = b[0], b[1]
    vx, vy = bx - ax, by - ay
    n = vx * vx + vy * vy
    t = np.clip(((px - ax) * vx + (py - ay) * vy) / n, 0, 1) if n else np.zeros_like(px)
    return np.hypot(px - (ax + t * vx), py - (ay + t * vy))


def _dentro(px, py, poligono):
    """Puntos dentro de un poligono convexo (en orden)."""
    signo = None
    dentro = np.ones(px.shape, bool)
    for i in range(len(poligono)):
        (x0, y0), (x1, y1) = poligono[i][:2], poligono[(i + 1) % len(poligono)][:2]
        c = (x1 - x0) * (py - y0) - (y1 - y0) * (px - x0)
        if signo is None:
            signo = np.sign(np.nanmean(c)) or 1
        dentro &= c * signo >= -0.5
    return dentro


def fila_cuello(alfa, esqueleto):
    """PL-ROPA-01: la fila del cuello segun el dibujo: la mas estrecha entre los ojos y el punto NECK del
    esqueleto (+2). En diagonal PixelLab pone NECK 2-4 filas por debajo de la barbilla, en los hombros, y
    con el esqueleto a secas los hombros salian como «cabeza» (y la camisa dejaba la piel del pecho a la
    vista). Si no hay estrechamiento, la del esqueleto."""
    alto, ancho = alfa.shape
    k = {p["label"]: (p["x"] * ancho, p["y"] * alto) for p in esqueleto}
    ojo_y = (k["LEFT EYE"][1] + k["RIGHT EYE"][1]) / 2
    cuello_y = k["NECK"][1]
    # entre 3 y 10 filas bajo los ojos (la barbilla y el cuello; mas abajo ya es el pecho, y en el respirar del
    # sureste, con el esqueleto 2-3 px mas bajo, el estrechamiento mas bajo era la cintura)
    y0, y1 = int(round(ojo_y)) + 3, min(alto - 1, int(round(cuello_y)) + 2, int(round(ojo_y)) + 8)
    if y1 < y0:
        return int(cuello_y)
    # el centro de la cabeza, por su dibujo (la nariz del esqueleto cae en el lado de la cara, y con ella el
    # recuento se comia la nuca)
    top = int(np.nonzero(alfa.any(axis=1))[0].min())
    ys_c, xs_c = np.nonzero(alfa[top:int(round(ojo_y)) + 1])
    cx = float(xs_c.mean()) + 0.5 if len(xs_c) else k["NOSE"][0]
    xs = np.arange(ancho)
    cerca = np.abs(xs + 0.5 - cx) <= 8
    anchos = {y: int((alfa[y] & cerca).sum()) for y in range(y0, y1 + 1)}
    anchos = {y: w for y, w in anchos.items() if w > 0}
    if not anchos:
        return int(cuello_y)
    minimo = min(anchos.values())
    filas = sorted(y for y, w in anchos.items() if w == minimo)
    # en una meseta de filas igual de estrechas (la barbilla y el cuello, o el cuello de un mediano, que es
    # tan ancho como sus hombros), la del medio (la de abajo si son dos)
    return filas[len(filas) // 2]


def ajustar_cuello(px, esqueleto):
    """El esqueleto con NECK en la fila del cuello del dibujo (fila_cuello)."""
    alfa = np.asarray(px)[..., 3] > 0
    if not alfa.any():
        return esqueleto
    y = fila_cuello(alfa, esqueleto) + 0.5
    return [dict(p, y=y / alfa.shape[0]) if p["label"] == "NECK" else p for p in esqueleto]


def suavizar_esqueletos(frames, esqueletos, direcciones):
    """PL-ROPA-01 (Oriol, 25/10, «la ropa se despega»): PixelLab estima el esqueleto frame a frame y el cuello,
    los hombros y la cadera bailan 1-2 px respecto al dibujo entre frames vecinos de la misma diagonal; con
    ellos bailaba el cinturon, el bajo del torso y lo alto del pantalon. Aqui el cuello es el del dibujo
    (fila_cuello) y, por diagonal, los hombros y las caderas quedan a la misma distancia del cuello en todos
    los frames (la mediana). Lo que se mueve de verdad (brazos, rodillas, tobillos) no se toca."""
    alto, ancho = frames[0].shape[:2]
    esqueletos = [ajustar_cuello(px, esq) for px, esq in zip(frames, esqueletos)]
    # el cuello a la misma distancia de la coronilla en todos los frames de la diagonal: la mediana de los
    # seis del andar (en el respirar del sureste, redibujado, el estrechamiento salia 4 filas mas abajo)
    tops = [int(np.nonzero((px[..., 3] > 0).any(axis=1))[0].min()) if (px[..., 3] > 0).any() else 0 for px in frames]
    for d in set(direcciones):
        idx = [i for i, dd in enumerate(direcciones) if dd == d]
        andar = [i for i in idx if i < len(RUMBOS) * FRAMES_ANDAR] or idx
        rel = float(np.median([{p["label"]: p for p in esqueletos[i]}["NECK"]["y"] * alto - tops[i] for i in andar]))
        for i in idx:
            esqueletos[i] = [dict(p, y=(tops[i] + rel) / alto) if p["label"] == "NECK" else p for p in esqueletos[i]]
    por_d = {}
    for f, (px, esq, d) in enumerate(zip(frames, esqueletos, direcciones)):
        alfa = px[..., 3] > 0
        if not alfa.any():
            continue
        k = {p["label"]: p for p in esq}
        cuello = k["NECK"]["y"] * alto
        por_d.setdefault(d, []).append((f, cuello, {n: k[n]["y"] * alto - cuello for n in
                                                  ("LEFT SHOULDER", "RIGHT SHOULDER", "LEFT HIP", "RIGHT HIP")}))
    out = [list(e) for e in esqueletos]
    for d, lista in por_d.items():
        rel = {n: float(np.median([r[n] for _f, _c, r in lista])) for n in lista[0][2]}
        for f, cuello, _r in lista:
            out[f] = [dict(p, y=(cuello + rel[p["label"]]) / alto) if p["label"] in rel else p
                      for p in esqueletos[f]]
    return out


def _linea(X, a, b):
    """La y de la recta a-b en cada X (fuera del tramo, la del extremo)."""
    (ax, ay), (bx, by) = a[:2], b[:2]
    if abs(bx - ax) < 1e-6:
        return np.full(X.shape, (ay + by) / 2)
    t = np.clip((X - ax) / (bx - ax), 0, 1)
    return ay + t * (by - ay)


def etiquetar(px, esqueleto):
    """Mapa de partes de un frame RGBA a partir de su esqueleto.

    PL-ROPA-01 (25/10): el tronco es el cuerpo entero entre el cuello y la cadera (la tira de pixeles de cada
    fila que contiene la linea cuello-cadera), no solo el poligono de los cuatro puntos del esqueleto, que en
    diagonal es una tira estrecha y dejaba los lados del pecho como «brazo» (una camisa sin mangas parecia un
    tirante). Los brazos son lo que sobresale de esa tira y, encima de ella, solo la franja de 2 px pegada al
    hueso del brazo cercano cuando pasa por delante. El cinturon sigue la linea entre las dos caderas, que en
    diagonal va inclinada. El cuello es una fila.
    """
    alto, ancho = px.shape[:2]
    k = _kp(esqueleto, ancho, alto)
    alfa = px[..., 3] > 0
    ys, xs = np.nonzero(alfa)
    X, Y = xs.astype(float) + 0.5, ys.astype(float) + 0.5
    lab = np.full((alto, ancho), "", dtype="<U1")
    if not len(ys):
        return lab
    # el lado cercano: el hombro con mas profundidad (z mas alto = mas cerca de la camara)
    cerca = "RIGHT" if k["RIGHT SHOULDER"][2] >= k["LEFT SHOULDER"][2] else "LEFT"
    lejos = "LEFT" if cerca == "RIGHT" else "RIGHT"
    letra = {cerca: str.upper, lejos: str.lower}
    cuello = k["NECK"]
    hombros = [k["LEFT SHOULDER"], k["RIGHT SHOULDER"]]
    caderas = [k["LEFT HIP"], k["RIGHT HIP"]]
    cadera = ((caderas[0][0] + caderas[1][0]) / 2, (caderas[0][1] + caderas[1][1]) / 2)
    # 1) la cabeza: todo lo que queda por encima del cuello. Oriol (24/10, «no haces bien los hombros»): en
    # diagonal lo alto de los hombros queda por encima del NECK del esqueleto; desde PL-ROPA-01 el cuello es
    # la fila mas estrecha del dibujo (fila_cuello, ya puesta en el esqueleto), y los hombros quedan debajo.
    cabeza = Y < cuello[1] - 0.5
    # 2) el tronco: en cada fila entre el cuello y la entrepierna, la tira de pixeles que contiene la linea del
    # cuello a la cadera (o la mas cercana a ella)
    entrepierna = cadera[1] + 2.0
    tronco = np.zeros(len(ys), bool)
    borde_i = np.zeros(len(ys), bool)   # a 2 px del borde de la tira (donde cuelga un brazo pegado al cuerpo)
    borde_2 = np.zeros(len(ys), bool)   # a 1 px del borde
    idx_por_fila = {}
    for i, y in enumerate(ys):
        idx_por_fila.setdefault(int(y), []).append(i)
    for y, idxs in idx_por_fila.items():
        yc = y + 0.5
        if yc < cuello[1] - 0.5 or yc >= entrepierna:
            continue
        t = np.clip((yc - cuello[1]) / max(1e-6, cadera[1] - cuello[1]), 0, 1)
        xm = cuello[0] + t * (cadera[0] - cuello[0])
        cols = np.array(sorted(xs[idxs]))
        cortes = np.nonzero(np.diff(cols) > 1)[0]
        tiras = np.split(cols, cortes + 1)
        mejor = min(tiras, key=lambda tt: 0 if tt.min() - 0.5 <= xm <= tt.max() + 1.5 else min(abs(tt.min() + 0.5 - xm), abs(tt.max() + 0.5 - xm)))
        if not (mejor.min() - 0.5 <= xm <= mejor.max() + 1.5) and min(abs(mejor.min() + 0.5 - xm), abs(mejor.max() + 0.5 - xm)) > 3:
            continue
        for i in idxs:
            if mejor.min() <= xs[i] <= mejor.max():
                tronco[i] = True
                borde_i[i] = xs[i] <= mejor.min() + 2 or xs[i] >= mejor.max() - 2
                borde_2[i] = xs[i] <= mejor.min() + 1 or xs[i] >= mejor.max() - 1
    # 3) candidatos por hueso: (etiqueta, distancia, profundidad). Oriol (25/10, sus marcas sobre la lamina):
    # los puntos ARM y LEG del esqueleto de PixelLab caen en la PUNTA de la extremidad (los dedos, la suela),
    # no en la muneca ni en el tobillo: la mano es la bola de 2 px de radio que queda 2 px por encima de ARM
    # (hacia el codo), y el pie son las 4 filas que acaban en LEG.
    huesos = []
    # la pierna cercana: la cadera con mas profundidad; si empatan (pasa en algun frame del andar), el mismo
    # lado que los hombros, que es el de la diagonal
    dz = k["RIGHT HIP"][2] - k["LEFT HIP"][2]
    cerca_cadera = cerca if abs(dz) < 0.5 else ("RIGHT" if dz > 0 else "LEFT")
    for lado in ("LEFT", "RIGHT"):
        L = letra[lado]
        hom, codo, mun = k[f"{lado} SHOULDER"], k[f"{lado} ELBOW"], k[f"{lado} ARM"]
        cad, rod, tob = k[f"{lado} HIP"], k[f"{lado} KNEE"], k[f"{lado} LEG"]
        z = hom[2]
        # el brazo no sigue por debajo de los dedos (ARM)
        d_brazo = np.minimum(_dist_segmento(X, Y, hom, codo), _dist_segmento(X, Y, codo, mun))
        d_brazo = d_brazo + np.where(Y > mun[1] + 1.0, 9.0, 0.0)
        huesos.append((L("a"), d_brazo, z))
        dx, dy = mun[0] - codo[0], mun[1] - codo[1]
        n = max(1e-6, np.hypot(dx, dy))
        centro = (mun[0] - dx / n * 1.0, mun[1] - dy / n * 1.0)
        d_mano = np.hypot(X - centro[0], Y - centro[1])
        huesos.append((L("m"), d_mano - 2.0 + np.where(d_mano > 3.6, 9.0, 0.0), z + 0.1))
        zp = cad[2]
        Lp = str.upper if lado == cerca_cadera else str.lower
        huesos.append((Lp("l"), np.minimum(_dist_segmento(X, Y, cad, rod), _dist_segmento(X, Y, rod, tob)), zp))
        pie_alto, pie_bajo = (tob[0], tob[1] - 3.0), (tob[0], tob[1] + 0.5)
        d_pie = _dist_segmento(X, Y, pie_alto, pie_bajo) - 1.0 + np.where(Y < tob[1] - 3.5, 9.0, 0.0)
        huesos.append((Lp("f"), d_pie, zp + 0.1))
    # Oriol (25/10, sus marcas): donde la extremidad cercana pasa por delante (a menos de 2,5 px de su hueso), la
    # lejana no se ve: gana la cercana. Y en un empate, tambien.
    D_ = {h[0]: h[1] for h in huesos}
    for cerca_, lejos_ in ((("A", "M"), ("a", "m")), (("L", "F"), ("l", "f"))):
        tapa = np.minimum(D_[cerca_[0]], D_[cerca_[1]]) <= 2.5
        for l_ in lejos_:
            D_[l_] = D_[l_] + np.where(tapa, 9.0, 0.5)
    huesos = [(h[0], D_[h[0]], h[2]) for h in huesos]
    z_torso = (hombros[0][2] + hombros[1][2]) / 2 - 0.5
    D = np.stack([h[1] for h in huesos])
    Z = np.array([h[2] for h in huesos])
    nombres = np.array([h[0] for h in huesos])
    es_brazo = np.isin(nombres, [letra[cerca]("a"), letra[cerca]("m")])
    # fuera del tronco gana el hueso mas cercano
    etiqueta = nombres[np.argmin(D, 0)]
    # dentro del tronco, el tronco; salvo la franja del brazo cercano que pasa por delante (a 1 px de su hueso;
    # la mano, a 1,5 px de su centro), o un brazo colgando pegado al cuerpo. Oriol (25/10, «la perspectiva desde
    # donde vemos la camara es mas asi»): el brazo que cuelga son las 2 columnas del borde (3 si el hueso pasa
    # a 1,3 px), y la mano no entra mas de 3 columnas en el tronco
    es_mano = np.isin(nombres, [letra[cerca]("m")])
    delante = ((D <= 1.0) & es_brazo[:, None] & ~es_mano[:, None] | (D <= -0.5) & es_mano[:, None]) & (Z[:, None] > z_torso)
    brazos = np.isin(nombres, ["a", "A"])[:, None]
    manos = np.isin(nombres, ["m", "M"])[:, None]
    colgando = brazos & ((borde_2[None, :] & (D <= 2.6)) | (borde_i[None, :] & (D <= 1.3)))
    colgando |= manos & borde_i[None, :] & (D <= 0.6)
    encima = delante | colgando
    hay = encima.any(0)
    mejor_encima = np.argmin(np.where(encima, D, 1e9), 0)
    etiqueta = np.where(tronco, np.where(hay, nombres[mejor_encima], "t"), etiqueta)
    etiqueta = np.where(cabeza, "h", etiqueta)
    # el cuello: una fila bajo la cabeza; la pelvis: del cinturon (la linea entre las dos caderas, inclinada
    # en diagonal) a la entrepierna
    cint = _linea(X, caderas[0], caderas[1]) - 1.0
    etiqueta = np.where((etiqueta == "t") & (Y >= cuello[1] - 0.5) & (Y < cuello[1] + 0.5), "n", etiqueta)
    etiqueta = np.where((etiqueta == "t") & (Y >= cint), "p", etiqueta)
    piernas = np.isin(etiqueta, ["l", "L"]) & (Y < entrepierna) & tronco
    etiqueta = np.where(piernas, "p", etiqueta)
    lab[ys, xs] = etiqueta
    # Oriol (25/10): un brazo nunca lleva tronco por fuera: si en una fila el pixel del borde de la tira es
    # tronco y los de dentro son brazo, el del borde es ese brazo. Y el brazo lejano, que el esqueleto de
    # PixelLab estima mal cuando queda tapado, cuelga por defecto en las 2 columnas del borde de su lado, del
    # hombro a la cadera, con la mano en las ultimas 4 filas (lo que Oriol marco en los frames de espaldas).
    hombro_lejos = k[f"{lejos} SHOULDER"]
    lado_lejos = 1 if hombro_lejos[0] > cuello[0] else -1  # +1: el brazo lejano queda a la derecha
    y_hombro, y_cadera = hombro_lejos[1], cadera[1]
    for y, idxs in idx_por_fila.items():
        cols = sorted(xs[i] for i in idxs if tronco[i])
        if len(cols) < 4:
            continue
        for x_borde, paso in ((cols[0], 1), (cols[-1], -1)):
            if lab[y, x_borde] == "t" and lab[y, x_borde + paso] in ("a", "A", "m", "M"):
                lab[y, x_borde] = lab[y, x_borde + paso]
        if y_hombro + 0.5 <= y + 0.5 <= y_cadera + 2.5 and len(cols) >= 6:
            borde = (cols[-1], cols[-2]) if lado_lejos > 0 else (cols[0], cols[1])
            if all(lab[y, x] in ("t", "p") for x in borde):
                que = "m" if y + 0.5 >= y_cadera - 1.5 else "a"
                for x in borde:
                    lab[y, x] = que
    return lab


def fraccion_brazo(lab, esqueleto):
    """PL-ROPA-01: en cada pixel de brazo o mano (a, A, m, M), en que punto del brazo esta: 0 en el hombro,
    0,5 en el codo, 1 en la muneca (y mas alla, la mano). Las mangas cortas de las prendas llegan hasta el
    codo por aqui, no por filas (en diagonal, un brazo que se adelanta sube en la celda)."""
    alto, ancho = lab.shape
    k = _kp(esqueleto, ancho, alto)
    out = np.full((alto, ancho), np.nan)
    cerca = "RIGHT" if k["RIGHT SHOULDER"][2] >= k["LEFT SHOULDER"][2] else "LEFT"
    for lado in ("LEFT", "RIGHT"):
        letras = ["A", "M"] if lado == cerca else ["a", "m"]
        ys, xs = np.nonzero(np.isin(lab, letras))
        if not len(ys):
            continue
        X, Y = xs + 0.5, ys + 0.5
        hom, codo, mun = k[f"{lado} SHOULDER"], k[f"{lado} ELBOW"], k[f"{lado} ARM"]
        mejor = np.full(len(ys), np.inf)
        frac = np.zeros(len(ys))
        for (a, b), (t0, t1) in (((hom, codo), (0.0, 0.5)), ((codo, mun), (0.5, 1.0))):
            vx, vy = b[0] - a[0], b[1] - a[1]
            n = vx * vx + vy * vy
            t = np.clip(((X - a[0]) * vx + (Y - a[1]) * vy) / n, 0, 1) if n else np.zeros(len(ys))
            d = np.hypot(X - (a[0] + t * vx), Y - (a[1] + t * vy))
            gana = d < mejor
            mejor[gana] = d[gana]
            frac[gana] = t0 + (t1 - t0) * t[gana]
        out[ys, xs] = frac
    return out


def _lum(c):
    return c[..., 0] * 0.299 + c[..., 1] * 0.587 + c[..., 2] * 0.114


def tonos(px, lab):
    """Clase de tono de cada pixel (H luz, s base, d sombra, o contorno, O contorno duro) por parte."""
    alto, ancho = px.shape[:2]
    alfa = px[..., 3] > 0
    L = _lum(px[..., :3].astype(float))
    t = np.full((alto, ancho), "", dtype="<U4")
    borde = np.zeros_like(alfa)
    for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        vecino = np.zeros_like(alfa)
        sy, sx = slice(max(0, dy), alto + min(0, dy)), slice(max(0, dx), ancho + min(0, dx))
        ty, tx = slice(max(0, -dy), alto + min(0, -dy)), slice(max(0, -dx), ancho + min(0, -dx))
        vecino[ty, tx] = alfa[sy, sx]
        borde |= alfa & ~vecino
    for parte in np.unique(lab[alfa]):
        m = alfa & (lab == parte)
        v = L[m]
        if not len(v):
            continue
        q_osc, q_som, q_luz = np.percentile(v, [12, 40, 85])
        clase = np.where(v <= q_osc, "o", np.where(v <= q_som, "d", np.where(v >= q_luz, "H", "s")))
        t[m] = clase
    t[borde & (L < np.percentile(L[alfa], 30))] = "O"
    return t


def ojos(px, lab, esqueleto):
    """Los pixeles de los ojos: junto a los puntos de los ojos, los que no son piel."""
    alto, ancho = px.shape[:2]
    k = _kp(esqueleto, ancho, alto)
    m = np.zeros((alto, ancho), bool)
    cabeza = lab == "h"
    if not cabeza.any():
        return m
    piel = np.median(px[cabeza][:, :3].astype(float), axis=0)
    for lado in ("LEFT EYE", "RIGHT EYE"):
        x, y = int(k[lado][0]), int(k[lado][1])
        for yy in range(y - 1, y + 2):
            for xx in range(x - 1, x + 2):
                if 0 <= yy < alto and 0 <= xx < ancho and cabeza[yy, xx]:
                    if np.abs(px[yy, xx, :3].astype(float) - piel).sum() > 90:
                        m[yy, xx] = True
    return m


class TallaPL:
    """Los frames de PixelLab de un cuerpo con su mapa de partes, tonos y ojos (como actores_tallas.Talla)."""

    def __init__(self, clave, frames, esqueletos, direcciones):
        self.clave = clave
        self.frames = [np.asarray(f, np.uint8) for f in frames]
        self.alto, self.ancho = self.frames[0].shape[:2]
        todos = range(1, len(frames) + 1)
        self._dir = {f: direcciones[f - 1] for f in todos}
        esqueletos = suavizar_esqueletos(self.frames, esqueletos, direcciones)  # PL-ROPA-01
        self.esqueletos = {f: esqueletos[f - 1] for f in todos}
        self.labs = {f: etiquetar(self.frames[f - 1], esqueletos[f - 1]) for f in todos}
        self.brazo_t = {f: fraccion_brazo(self.labs[f], esqueletos[f - 1]) for f in todos}  # PL-ROPA-01
        self.tonos = {f: tonos(self.frames[f - 1], self.labs[f]) for f in todos}
        # de espaldas (las diagonales de arriba) no se ven los ojos aunque el esqueleto los ponga
        self.ojos = {f: (ojos(self.frames[f - 1], self.labs[f], esqueletos[f - 1]) if self._dir[f].startswith("sur")
                         else np.zeros((self.alto, self.ancho), bool)) for f in todos}
        for f in todos:
            self.tonos[f] = np.where(self.ojos[f], "e:o", self.tonos[f])
        self.alfa = {f: self.frames[f - 1][..., 3] > 0 for f in todos}
        self.top = {f: int(np.nonzero(self.alfa[f].any(axis=1))[0].min()) for f in todos}

    @property
    def n_frames(self):
        return len(self.frames)

    # Las prendas de siempre (actores_trajes, actores_pintura...) preguntan por las cuatro vistas de antes:
    # a ellas, las diagonales de frente son «abajo» y las de espaldas «arriba». La diagonal de verdad la da
    # diagonal(f).
    LEGADO = {"sureste": "abajo", "suroeste": "abajo", "noreste": "arriba", "noroeste": "arriba"}

    def direccion(self, f):
        return self.LEGADO[self._dir[f]]

    def diagonal(self, f):
        return self._dir[f]

    def dy_cabeza(self, f):
        return 0

    def altura(self):
        """Alto del cuerpo (de la coronilla a la suela) de pie, mirando al suroeste."""
        f = 1 + len(RUMBOS) * FRAMES_ANDAR + 2 * FRAMES_RESPIRAR
        ys = np.nonzero(self.alfa[f].any(axis=1))[0]
        return int(ys.max() - ys.min() + 1)


# --- las familias: el cuerpo animado de PixelLab de cada talla ---------------------------------
# Oriol (24/10): «un cos per familia». Los rumbos del motor (Game::Rumbo) son los de la rejilla, que en
# pantalla son diagonales: arriba = noreste, derecha = sureste, abajo = suroeste, izquierda = noroeste.
from pathlib import Path  # noqa: E402

CARPETA = Path(__file__).resolve().parent / "pixellab" / "familias"
RUMBOS = ["north-east", "south-east", "south-west", "north-west"]  # arriba, derecha, abajo, izquierda
ANDAR, RESPIRAR = "walking-6-frames", "breathing-idle"
FRAMES_ANDAR, FRAMES_RESPIRAR = 6, 4
COLUMNAS = len(RUMBOS) * (FRAMES_ANDAR + FRAMES_RESPIRAR)  # 40
# la talla (la clave de los atlas de ropa) de cada familia
FAMILIAS = {"medio": "humano", "bajo": "enano", "pequeno": "mediano", "grande": "orco", "naga": "naga",
            "aarakocra": "aarakocra", "espectro": "espectro", "minotauro": "minotauro"}


def reparto():
    """[(diagonal de PixelLab, animacion, i)] de los 40 frames, en el orden del motor."""
    out = []
    for d in RUMBOS:
        out += [(d, ANDAR, i) for i in range(FRAMES_ANDAR)]
    for d in RUMBOS:
        out += [(d, RESPIRAR, i) for i in range(FRAMES_RESPIRAR)]
    return out


def _dec(b64):
    import base64
    import io

    from PIL import Image
    return np.asarray(Image.open(io.BytesIO(base64.b64decode(b64))).convert("RGBA"))


# PL-JUEGO-01 (Oriol, 25/10: «los personajes parece que floten, su sombra está muy caída»): el motor pone la
# sombra y el aura donde los personajes de antes tenian los pies, la fila 60 de la celda de 64. PixelLab los
# dibuja mas arriba y a otra altura en cada familia (los pies, de la fila 46 del mediano a la 55 del
# aarakocra), asi que cada familia se baja entera hasta que sus pies quedan en esa fila (sin pasar de la 62,
# que el borde se recorta), y con ella su esqueleto y las bases de sus razas (pixellab_razas._preparar).
FILA_PIES = 60


def bajar(px, dy):
    """La imagen (alto x ancho x 4) bajada dy pixeles, con lo de abajo fuera."""
    if not dy:
        return px
    out = np.zeros_like(px)
    out[dy:] = px[:-dy]
    return out


def _bajar_kp(kp, dy, alto):
    return [dict(p, y=p["y"] + dy / alto) for p in kp] if dy else kp


def _dy_pies(frames):
    pies = [int(np.nonzero(f[..., 3].any(axis=1))[0].max()) for f in frames if f[..., 3].any()]
    if not pies:
        return 0
    return max(0, min(FILA_PIES - int(np.median(pies)), frames[0].shape[0] - 2 - max(pies)))


def cargar_familia(talla, datos=None):
    import json
    familia = FAMILIAS[talla]
    if datos is None:
        datos = json.loads((CARPETA / f"{familia}.json").read_text(encoding="utf-8"))
    frames, esqueletos, dirs = [], [], []
    for d, a, i in reparto():
        frames.append(_dec(datos["anims"][a][d][i]))
        esqueletos.append(datos["kp"][f"{a}|{d}"][i])
        dirs.append(DIAGONALES[d])
    dy = _dy_pies(frames)
    alto = frames[0].shape[0]
    t = TallaPL(talla, [bajar(f, dy) for f in frames], [_bajar_kp(k, dy, alto) for k in esqueletos], dirs)
    t.familia = familia
    t.dy_pies = dy
    if datos.get("anatomia") == "centauro":
        from pixellab_minotauro import ajustar
        ajustar(t)
    # el frame quieto de cada diagonal es el primero de respirar: la misma imagen y el mismo esqueleto (ya
    # suavizado) que ese frame de la hoja
    t.quietos = {}
    for i, d in enumerate(RUMBOS):
        f = 1 + len(RUMBOS) * FRAMES_ANDAR + i * FRAMES_RESPIRAR
        t.quietos[d] = (t.frames[f - 1], t.esqueletos[f])
    return t


def construir_tallas_pl():
    return {t: cargar_familia(t) for t, f in FAMILIAS.items() if (CARPETA / f"{f}.json").exists()}


def direccion_de_frame(f):
    """La vista de siempre (para el orden de las capas) del frame f de la hoja de 40."""
    d, _a, _i = reparto()[f - 1]
    return TallaPL.LEGADO[DIAGONALES[d]]
