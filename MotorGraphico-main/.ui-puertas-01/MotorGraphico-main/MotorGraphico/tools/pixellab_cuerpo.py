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


def etiquetar(px, esqueleto):
    """Mapa de partes de un frame RGBA a partir de su esqueleto."""
    alto, ancho = px.shape[:2]
    k = _kp(esqueleto, ancho, alto)
    alfa = px[..., 3] > 0
    ys, xs = np.nonzero(alfa)
    X, Y = xs.astype(float) + 0.5, ys.astype(float) + 0.5
    lab = np.full((alto, ancho), "", dtype="<U1")
    # el lado cercano: el hombro con mas profundidad (z mas alto = mas cerca de la camara)
    cerca = "RIGHT" if k["RIGHT SHOULDER"][2] >= k["LEFT SHOULDER"][2] else "LEFT"
    lejos = "LEFT" if cerca == "RIGHT" else "RIGHT"
    letra = {cerca: str.upper, lejos: str.lower}
    cuello = k["NECK"]
    hombros = [k["LEFT SHOULDER"], k["RIGHT SHOULDER"]]
    caderas = [k["LEFT HIP"], k["RIGHT HIP"]]
    cadera_y = (caderas[0][1] + caderas[1][1]) / 2
    hombro_y = min(h[1] for h in hombros)
    # 1) la cabeza: todo lo que queda por encima del cuello (menos lo que sea brazo en alto)
    cabeza = Y < cuello[1] - 0.5
    # Oriol (24/10, «no haces bien los hombros»): en diagonal lo alto de los hombros queda por encima del
    # cuello. Por debajo de los ojos la cabeza no es mas ancha que a la altura de los ojos: lo que sobresale
    # a los lados es hombro (torso), no cabeza.
    ojo_y = int(round((k["LEFT EYE"][1] + k["RIGHT EYE"][1]) / 2))
    if 0 <= ojo_y < alto:
        fila = np.nonzero(alfa[ojo_y])[0]
        if len(fila):
            x0, x1 = fila.min() - 0.5, fila.max() + 1.5
            cabeza &= (Y < ojo_y) | ((X >= x0) & (X <= x1))
    # 2) candidatos por hueso: (etiqueta, distancia, profundidad)
    huesos = []
    for lado in ("LEFT", "RIGHT"):
        L = letra[lado]
        hom, codo, mun = k[f"{lado} SHOULDER"], k[f"{lado} ELBOW"], k[f"{lado} ARM"]
        cad, rod, tob = k[f"{lado} HIP"], k[f"{lado} KNEE"], k[f"{lado} LEG"]
        z = hom[2]
        huesos.append((L("a"), np.minimum(_dist_segmento(X, Y, hom, codo), _dist_segmento(X, Y, codo, mun)), z))
        # la mano: el punto del muneco y un poco mas alla, hacia donde apunta el antebrazo
        dx, dy = mun[0] - codo[0], mun[1] - codo[1]
        n = max(1e-6, np.hypot(dx, dy))
        punta = (mun[0] + dx / n * 2.0, mun[1] + dy / n * 2.0)
        huesos.append((L("m"), _dist_segmento(X, Y, mun, punta) - 0.6, z + 0.1))
        zp = cad[2]
        huesos.append((L("l"), np.minimum(_dist_segmento(X, Y, cad, rod), _dist_segmento(X, Y, rod, tob)), zp))
        pie = (tob[0], tob[1] + 2.5)
        huesos.append((L("f"), _dist_segmento(X, Y, tob, pie) - 0.4 + np.where(Y < tob[1] - 0.5, 9, 0), zp + 0.1))
    torso = _dentro(X, Y, [hombros[0], hombros[1], caderas[1], caderas[0]])
    d_torso = np.where(torso, 0.0, np.minimum(_dist_segmento(X, Y, cuello, ((caderas[0][0] + caderas[1][0]) / 2, cadera_y)),
                                             9.0) - 1.5)
    z_torso = (hombros[0][2] + hombros[1][2]) / 2 - 0.5
    huesos.append(("t", d_torso, z_torso))
    # fuera del torso gana el hueso mas cercano; dentro, el torso, salvo un brazo o una pierna del lado
    # cercano que pase justo por encima (a menos de 1,3 px de su hueso)
    D = np.stack([h[1] for h in huesos[:-1]])
    Z = np.array([h[2] for h in huesos[:-1]])
    nombres = np.array([h[0] for h in huesos[:-1]])
    gana = np.argmin(D, 0)
    etiqueta = nombres[gana]
    encima = (D <= 1.3) & (Z[:, None] > z_torso)
    hay = encima.any(0)
    mejor_encima = np.argmin(np.where(encima, D, 1e9), 0)
    etiqueta = np.where(torso, np.where(hay, nombres[mejor_encima], "t"), etiqueta)
    etiqueta = np.where(~torso & (d_torso < D.min(0) - 0.5), "t", etiqueta)
    etiqueta = np.where(cabeza, "h", etiqueta)
    # cuello: la franja entre la cabeza y los hombros que era torso; pelvis: del cinturon a la entrepierna
    etiqueta = np.where((etiqueta == "t") & (Y >= cuello[1] - 0.5) & (Y < hombro_y + 0.5), "n", etiqueta)
    etiqueta = np.where((etiqueta == "t") & (Y >= cadera_y - 1.0), "p", etiqueta)
    piernas = np.isin(etiqueta, ["l", "L"]) & (Y < cadera_y + 2.0) & torso
    etiqueta = np.where(piernas, "p", etiqueta)
    lab[ys, xs] = etiqueta
    return lab


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
        self.esqueletos = {f: esqueletos[f - 1] for f in todos}
        self.labs = {f: etiquetar(self.frames[f - 1], esqueletos[f - 1]) for f in todos}
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
    t.quietos = {d: (bajar(_dec(datos["anims"][RESPIRAR][d][0]), dy),
                     _bajar_kp(datos["kp"][f"{RESPIRAR}|{d}"][0], dy, alto)) for d in RUMBOS}
    return t


def construir_tallas_pl():
    return {t: cargar_familia(t) for t, f in FAMILIAS.items() if (CARPETA / f"{f}.json").exists()}


def direccion_de_frame(f):
    """La vista de siempre (para el orden de las capas) del frame f de la hoja de 40."""
    d, _a, _i = reparto()[f - 1]
    return TallaPL.LEGADO[DIAGONALES[d]]
