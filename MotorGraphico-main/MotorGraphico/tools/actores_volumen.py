"""ESTETICA-01 — Armas con volumen: cada vista con su perspectiva.

Oriol (07/10/2026): «la hacha le falta perspectivas igual que al martillo, se entienden
fatal» y «si aumentas el numero de sprites por lado de cada arma todo quedaria super
mejor». Las armas se dibujaban una vez, de lado, y se giraban en el plano de la pantalla:
la cabeza de un martillo era siempre el mismo rectangulo, como una pegatina.

Aqui la maestra al doble (actores_poses.maestra: el dibujo de frente, con lo principal
donde lo quiere el gesto) se convierte en un objeto de voxeles: cada material con su grosor
(GROSOR: la cabeza del martillo es un bloque, la hoja del hacha es fina y se afila hacia el
filo, el astil es redondo) y se gira en 3D como se coge en cada vista: inclinado en la
pantalla (el mismo k de siempre), girado sobre su eje (`giro`: la cabeza enseña la cara) y
echado hacia delante o hacia atras (`hondo`). La camara mira un poco desde arriba, como al
personaje. Se proyecta en la rejilla de siempre (los mismos caracteres: tonos 1-3 por la
luz que le da a cada cara, contorno 4/9/d/i por fuera y donde una parte tapa a otra), asi
que todo lo demas (acortar para que quepa, no tapar la cara, pintar con los materiales del
objeto) sigue igual.
"""
import math

import numpy as np

# tonos (luz, base, sombra, contorno) de cada material de la rejilla (actores_poses.MATERIALES)
TONOS = [("1", "2", "3", "4"), ("6", "7", "8", "9"), ("a", "b", "c", "d"), ("f", "g", "h", "i")]
ANCLA_DE = {0: "&", 1: "@", 2: "$", 3: "@"}
BRILLO = "w"

# Grosor de cada material por familia (0 hoja o cabeza, 1 astil o empunadura, 2 guarnicion,
# 3 gema): ("caja", n) n voxeles de hondo; ("hoja", n) n en el centro y 1 en el borde;
# "redondo" tan hondo como ancho su tramo, de seccion redonda.
POR_DEFECTO = {0: ("hoja", 2), 1: "redondo", 2: "redondo", 3: "redondo"}
GROSOR = {
    "martillo": {0: ("caja", 8), 1: "redondo", 2: "redondo", 3: ("caja", 9)},
    "maza": {0: "redondo", 1: "redondo", 2: "redondo", 3: "redondo"},
    "macana": {0: "redondo", 1: "redondo", 2: "redondo", 3: "redondo"},
    "hacha": {0: ("hoja", 3), 1: "redondo", 2: ("caja", 5), 3: ("caja", 4)},
    "hacha_mano": {0: ("hoja", 3), 1: "redondo", 2: ("caja", 5), 3: ("caja", 4)},
    "gran_hacha": {0: ("hoja", 3), 1: "redondo", 2: ("caja", 5), 3: ("caja", 4)},
    # la ballesta: la caja de madera, la cuerda fina y el arco de acero plano
    "ballesta": {0: ("caja", 4), 1: ("caja", 1), 2: ("hoja", 2), 3: ("caja", 3)},
    # SP-02: el canon de mano del Canonero, un tubo redondo con sus aros
    "canon": {0: "redondo", 1: "redondo", 2: "redondo", 3: "redondo"},
}

# cuanto del giro de la pose sobre su eje se aplica a cada familia: las hojas de hacha se
# leen de cara (girarlas mucho las deja de canto, como un bloque)
GIRO = {"gran_hacha": 0.3, "hacha": 0.3, "hacha_mano": 0.3}

CAMARA = 18.0  # grados: la camara mira un poco desde arriba (se ve la cara de arriba)
# Lo que queda detras del cuerpo (mas de DETRAS_DE voxeles por detras de la mano, que va al
# costado, a media hondura del cuerpo) se marca con su tono "de detras" (DETRAS): pintar_arma
# no lo pinta donde esta el cuerpo, que lo tapa (Oriol: «superpongas el pj a la arma [...]
# solo el trozo justo»).
DETRAS_DE = 2.5
_TONOS_DELANTE = "123456789abcdefghijw"
_TONOS_DETRAS = "ABCDEFGHIJKLMNOPQRST"
DETRAS = dict(zip(_TONOS_DELANTE, _TONOS_DETRAS))
DELANTE = dict(zip(_TONOS_DETRAS, _TONOS_DELANTE))
LUZ = np.array([-0.55, 0.65, 0.52])  # de arriba a la izquierda y de delante
LUZ = LUZ / np.linalg.norm(LUZ)
SUB = 3  # submuestras por voxel y eje: sin agujeros al girar


def _material(c):
    import actores_poses as poses
    return poses._material(c)


def _grosor_de(familia):
    import actores_poses as poses
    return GROSOR.get(poses.familia_base(familia), POR_DEFECTO)


def modelo(filas, familia):
    """Voxeles (y, x, z enteros, en la rejilla) de la maestra extruida con el grosor de cada
    material: devuelve (ocupado[alto, ancho, hondo] con material+1 o 0, especial[...] con el
    caracter de brillo, origen (fila, columna, capa) del ancla)."""
    import actores_poses as poses
    g = poses._pad(filas)
    alto, ancho = len(g), len(g[0])
    ay = poses.fila_ancla(g)
    ax = next(g[ay].index(c) for c in poses.ANCLAS if c in g[ay])
    grosor = _grosor_de(familia)
    mats = np.full((alto, ancho), -1, int)
    brillo = np.zeros((alto, ancho), bool)
    tono = np.ones((alto, ancho), int)  # el tono del dibujo: 0 luz, 1 base, 2 sombra, 3 contorno
    for r, f in enumerate(g):
        for c, ch in enumerate(f):
            if ch == ".":
                continue
            m = _material(ch)
            if ch == "@":
                m = 1
            elif ch == "&":
                m = 0
            elif ch == "$":
                m = 2
            if m is None:
                continue
            mats[r, c] = m
            brillo[r, c] = ch == BRILLO
            fila = poses.MATERIALES[m]
            if ch in fila:
                tono[r, c] = min(fila.index(ch), 3)
    hondo = np.zeros((alto, ancho), float)  # media profundidad de cada pixel
    for r in range(alto):
        c = 0
        while c < ancho:
            m = mats[r, c]
            if m < 0:
                c += 1
                continue
            d = c
            while d + 1 < ancho and mats[r, d + 1] == m:
                d += 1
            regla = grosor.get(m, POR_DEFECTO[m])
            for x in range(c, d + 1):
                if regla == "redondo":
                    radio = (d - c + 1) / 2.0
                    dx = x + 0.5 - (c + d + 1) / 2.0
                    hondo[r, x] = math.sqrt(max(radio * radio - dx * dx, 0.25))
                else:
                    tipo, n = regla
                    if tipo == "caja":
                        hondo[r, x] = n / 2.0
                    else:  # hoja: se afila hacia el borde de la silueta
                        borde = any(not (0 <= rr < alto and 0 <= cc < ancho) or mats[rr, cc] < 0
                                    for rr, cc in ((r - 1, x), (r + 1, x), (r, x - 1), (r, x + 1)))
                        hondo[r, x] = 0.5 if borde else n / 2.0
            c = d + 1
    H = int(math.ceil(hondo.max())) if hondo.max() > 0 else 1
    capas = 2 * H
    ocup = np.zeros((alto, ancho, capas), np.int8)
    esp = np.zeros((alto, ancho, capas), bool)
    # en las hojas se queda el tono del dibujo (el filo claro, los biseles): un plano liso con
    # la luz de un plano seria una piedra gris; -1 = tono por la luz (bloques, astiles)
    t2d = np.full((alto, ancho, capas), -1, np.int8)
    zc = np.arange(capas) + 0.5 - H  # centro de cada capa
    for r in range(alto):
        for c in range(ancho):
            if mats[r, c] < 0:
                continue
            dentro = np.abs(zc) <= hondo[r, c] + 1e-6
            if not dentro.any():
                dentro[H - 1:H + 1] = True
            ocup[r, c, dentro] = mats[r, c] + 1
            esp[r, c, dentro] = brillo[r, c]
            regla = grosor.get(mats[r, c], POR_DEFECTO[mats[r, c]])
            if regla != "redondo" and regla[0] == "hoja":
                t2d[r, c, dentro] = tono[r, c]
    return ocup, esp, t2d, (ay, ax, H)


def _suaviza(a):
    """Media de 3 en cada eje (dos pasadas): para sacar las normales de la ocupacion."""
    out = a.astype(float)
    for _ in range(2):
        for eje in range(3):
            p = np.pad(out, [(1, 1) if e == eje else (0, 0) for e in range(3)], mode="constant")
            s = [slice(None)] * 3
            s0, s1, s2 = list(s), list(s), list(s)
            s0[eje], s1[eje], s2[eje] = slice(0, -2), slice(1, -1), slice(2, None)
            out = (p[tuple(s0)] + p[tuple(s1)] + p[tuple(s2)]) / 3.0
    return out


def _rot_x(a):
    c, s = math.cos(a), math.sin(a)
    return np.array([[1, 0, 0], [0, c, -s], [0, s, c]])


def _rot_y(a):
    c, s = math.cos(a), math.sin(a)
    return np.array([[c, 0, s], [0, 1, 0], [-s, 0, c]])


def _rot_z(a):
    c, s = math.cos(a), math.sin(a)
    return np.array([[c, -s, 0], [s, c, 0], [0, 0, 1]])


def angulo(gesto, sentido, k):
    """Giro en la pantalla (grados, como las agujas del reloj) de lo principal: el mismo que
    la cizalla o el giro plano de actores_poses (colgar y alzar inclinados k columnas por fila
    hacia `sentido`; tender casi horizontal hacia `sentido`, bajando k filas por columna)."""
    if gesto == "alzar":
        return math.degrees(math.atan(abs(k))) * (sentido if k >= 0 else -sentido)
    if gesto == "colgar":
        return -math.degrees(math.atan(abs(k))) * (sentido if k >= 0 else -sentido)
    return math.degrees(math.atan2(-sentido, k))  # tender: lo de abajo hacia `sentido`


_VISTAS = {}


def vista(filas, familia, gesto, sentido, k, giro=0.0, hondo=0.0):
    """La maestra (orientada: lo principal arriba para alzar, abajo para colgar y tender) como
    objeto con volumen, puesta con ese gesto (k como siempre), girada `giro` grados sobre su
    eje y echada `hondo` grados hacia el que mira (la punta hacia delante), y proyectada:
    filas con su ancla, listas para pintar_arma."""
    clave = (tuple(filas), familia, gesto, sentido, round(k, 3), round(giro, 1), round(hondo, 1))
    if clave in _VISTAS:
        return list(_VISTAS[clave])
    import actores_poses as poses
    ocup, esp, t2d, (ay, ax, H) = modelo(filas, familia)
    alto, ancho, capas = ocup.shape
    # normales: hacia donde baja la ocupacion suavizada (fuera del objeto)
    s = _suaviza(ocup > 0)
    gy, gx, gz = np.gradient(s)
    # puntos de cada voxel (submuestreados) en coordenadas del arma: x derecha, y arriba, z hacia el que mira
    rr, cc, zz = np.nonzero(ocup)
    off = (np.arange(SUB) + 0.5) / SUB - 0.5
    oy, ox, oz = np.meshgrid(off, off, off, indexing="ij")
    oy, ox, oz = oy.ravel(), ox.ravel(), oz.ravel()
    py = (ay - rr)[:, None] - oy[None, :]          # arriba +
    px = (cc - ax)[:, None] + ox[None, :]
    pz = (zz + 0.5 - H)[:, None] + oz[None, :]
    P = np.stack([px.ravel(), py.ravel(), pz.ravel()], axis=1)
    n = np.stack([gx[rr, cc, zz], -gy[rr, cc, zz], gz[rr, cc, zz]], axis=1)  # gy por filas: hacia abajo
    n = -n
    largo = np.linalg.norm(n, axis=1, keepdims=True)
    n = np.where(largo > 1e-6, n / np.maximum(largo, 1e-6), np.array([0.0, 0.0, 1.0]))
    N = np.repeat(n, len(off) ** 3, axis=0)
    mat = np.repeat(ocup[rr, cc, zz] - 1, len(off) ** 3)
    bri = np.repeat(esp[rr, cc, zz], len(off) ** 3)
    tdib = np.repeat(t2d[rr, cc, zz], len(off) ** 3)
    # giro: en el eje del arma, echada hacia delante, inclinada en la pantalla y la camara
    giro = giro * GIRO.get(poses.familia_base(familia), 1.0)
    theta = math.radians(angulo(gesto, sentido, k))
    pose = _rot_z(-theta) @ _rot_x(math.radians(hondo)) @ _rot_y(math.radians(giro))
    R = _rot_x(math.radians(CAMARA)) @ pose
    Q = P @ R.T
    NN = N @ R.T
    zpose = P @ pose[2]  # hondura respecto a la mano, sin la camara: detras del cuerpo si < -DETRAS_DE
    u = np.floor(Q[:, 0] + 0.5).astype(int)
    v = np.floor(-Q[:, 1] + 0.5).astype(int)
    z = Q[:, 2]
    u0, v0 = u.min(), v.min()
    W, Hh = u.max() - u0 + 1, v.max() - v0 + 1
    idx = (v - v0) * W + (u - u0)
    orden = np.lexsort((z, idx))  # por pixel, el mas cercano el ultimo
    ultimo = np.ones(len(orden), bool)
    ultimo[:-1] = idx[orden][1:] != idx[orden][:-1]
    sel = orden[ultimo]
    prof = np.full(W * Hh, -1e9)
    m_px = np.full(W * Hh, -1, int)
    luz = np.zeros(W * Hh)
    br = np.zeros(W * Hh, bool)
    tras = np.zeros(W * Hh, bool)
    td = np.full(W * Hh, -1, int)
    td[idx[sel]] = tdib[sel]
    prof[idx[sel]] = z[sel]
    m_px[idx[sel]] = mat[sel]
    luz[idx[sel]] = NN[sel] @ LUZ
    br[idx[sel]] = bri[sel]
    tras[idx[sel]] = zpose[sel] < -DETRAS_DE
    prof, m_px, luz, br, tras, td = (a.reshape(Hh, W) for a in (prof, m_px, luz, br, tras, td))
    out = [["."] * W for _ in range(Hh)]
    for y in range(Hh):
        for x in range(W):
            m = m_px[y, x]
            if m < 0:
                continue
            borde = False
            for yy, xx in ((y - 1, x), (y + 1, x), (y, x - 1), (y, x + 1)):
                if not (0 <= yy < Hh and 0 <= xx < W) or m_px[yy, xx] < 0:
                    borde = True
                    break
                if prof[yy, xx] - prof[y, x] > 1.6 and m_px[yy, xx] != m:
                    borde = True  # otra parte, mas cerca, la tapa: su silueta
                    break
            t = TONOS[m]
            if borde:
                c = t[3]
            elif br[y, x]:
                c = BRILLO if m == 0 else t[0]
            elif td[y, x] >= 0:  # hoja: el tono del dibujo, mas oscuro si le da la espalda a la luz
                c = t[min(3, td[y, x] + (1 if luz[y, x] < -0.25 and td[y, x] < 2 else 0))]
            elif luz[y, x] > 0.55:
                c = t[0]
            elif luz[y, x] > 0.05:
                c = t[1]
            else:
                c = t[2]
            out[y][x] = DETRAS[c] if tras[y, x] else c
    # el ancla: donde cae el puno (el origen)
    ya, xa = -v0, -u0
    m = m_px[ya, xa] if 0 <= ya < Hh and 0 <= xa < W else -1
    out[ya][xa] = ANCLA_DE.get(m, "@")
    res = poses._recorta(["".join(f) for f in out])
    _VISTAS[clave] = tuple(res)
    return res
