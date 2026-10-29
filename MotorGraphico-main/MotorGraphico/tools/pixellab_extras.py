"""PL-ROPA-01 (Oriol, 26/10, 09:32: «ahora faltarian alas, colas, cuernos, manchas en el cuerpo, tatuajes,
cicatrices y otros elementos de personalizacion»): rasgos a elegir, dibujados sobre el cuerpo de PixelLab.

Como el pelo (pixellab_cabeza), cada uno es una regla sobre el esqueleto y la silueta del frame: sale igual en
las 72 razas y las 7 tallas. Cuatro familias:

  ALAS      en la espalda, colgadas entre los hombros; de frente quedan detras del cuerpo (capa 0) y de
            espaldas encima (55). Baten un poco al andar.
  COLAS     de la cadera hacia atras, con un meneo al andar. Detras del cuerpo de frente, encima de espaldas.
  CUERNOS   en lo alto de la cabeza, a los lados; encima del pelo (capa «cabeza», 70).
  MARCAS    sobre la piel: pecas, manchas, rayas, tatuajes, cicatrices, pintura de guerra. Van pegadas a los
            huesos (no se deslizan al andar) y recortadas al cuerpo. Capa propia «marcas» (12), entre el
            cuerpo y la ropa: la pide el motor (seccion 6 del encargo).

Alas y colas se ofrecen como piezas de la ranura «capa» y los cuernos como piezas de «cabeza»: asi el editor
de hoy ya las lista y el motor las tine como a cualquier pieza (ESTETICA-01). Cuando el motor tenga filas
propias, el generador las escribe tambien como grupos «alas», «cola» y «cuernos».
"""
import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage

from actores_materiales import MATERIALES as MAT
from pixellab_cabeza import _sombrear

# id -> (nombre, material principal, material secundario)
ALAS = {
    "alas_murcielago": ("Alas de murcielago", "cuero_negro", "cuero_oscuro"),
    "alas_plumas": ("Alas de plumas", "plumas_blancas", "remeras_pardas"),
    "alas_libelula": ("Alas de libelula", "membrana_hada", "nervio_hada"),
    "alas_dragon": ("Alas de dragon", "escamas_rojas", "hueso"),
    "alas_cuervo": ("Alas de cuervo", "plumas_pardas", "tinta_negra"),
}
COLAS = {
    "cola_diablo": ("Cola de diablo", "piel_tiefling", "pica_tiefling"),
    "cola_gato": ("Cola de gato", "pelaje_gato", "pelaje_gato_gris"),
    "cola_lagarto": ("Cola de lagarto", "escamas_verdes", "escamas_oliva"),
    "cola_zorro": ("Cola de zorro", "pelaje_zorro", "pelaje_zorro_blanco"),
    "cola_lobo": ("Cola de lobo", "pelaje_lobo", "pelo_gris"),
}
CUERNOS = {
    "cuernos_rectos": ("Cuernos rectos", "cuerno_claro", "cuerno_oscuro"),
    "cuernos_carnero": ("Cuernos de carnero", "cuerno_oscuro", "cuerno_claro"),
    "cuernos_astas": ("Astas", "hueso", "cuerno_claro"),
    "cuernos_diablo": ("Cuernecillos", "cuerno_draco", "cuerno_oscuro"),
    "cuernos_toro": ("Cuernos de toro", "cuerno_claro", "obsidiana"),
}
MARCAS = {
    "pecas": ("Pecas", "pelo_castano", None),
    "manchas": ("Manchas", "pelo_negro", None),
    "rayas": ("Rayas", "pelo_negro", None),
    "lunares": ("Lunares", "pelo_oscuro", None),
    "tatuaje_brazo": ("Tatuaje en el brazo", "tinta_negra", None),
    "tatuaje_cara": ("Tatuaje en la cara", "tinta_negra", None),
    "tatuaje_pecho": ("Tatuaje en el pecho", "tinta_negra", None),
    "cicatriz_ojo": ("Cicatriz en el ojo", "piel_rojiza", None),
    "cicatriz_pecho": ("Cicatriz en el pecho", "piel_rojiza", None),
    "pintura_guerra": ("Pintura de guerra", "sangre", None),
    "pintura_ojos": ("Ojos pintados", "tinta_negra", None),
}
# PIEZAS: lo que entra en las ranuras de equipo (capa: alas y colas; cabeza: cuernos)
PIEZAS_CAPA = dict(ALAS, **COLAS)
PIEZAS_CABEZA = dict(CUERNOS)

BATIR = {0: 0, 1: -1, 2: -1, 3: 0, 4: 1, 5: 1}  # el aleteo y el meneo de la cola, por frame de andar


def _kp(talla, f):
    return {p["label"]: (p["x"] * talla.ancho, p["y"] * talla.alto) for p in talla.esqueletos[f]}


class _Frame:
    """Lo que necesitan todas las reglas de un frame: esqueleto, silueta, cabeza y hacia donde mira."""

    def __init__(self, talla, f):
        self.talla, self.f = talla, f
        self.lab = talla.labs[f]
        self.alfa = np.asarray(talla.alfa[f]) > 0
        self.k = _kp(talla, f)
        self.d = talla.diagonal(f)
        self.frente = self.d.startswith("sur")
        self.hacia_der = self.d.endswith("este")
        self.delante = 1 if self.hacia_der else -1
        self.atras = -self.delante
        _d, anim, i = __import__("pixellab_cuerpo").reparto()[f - 1]
        self.paso = i if anim == __import__("pixellab_cuerpo").ANDAR else 0
        cab = self.lab == "h"
        self.cab = cab
        if cab.any():
            ys, xs = np.nonzero(cab)
            self.top, self.bottom = int(ys.min()), int(ys.max())
            self.cx = float(xs.mean())
            self.semi = (xs.max() - xs.min()) / 2
        else:
            self.top = self.bottom = None
            self.cx, self.semi = talla.ancho / 2, 6
        self.ojos_y = (self.k["LEFT EYE"][1] + self.k["RIGHT EYE"][1]) / 2
        hl, hr = self.k["LEFT HIP"], self.k["RIGHT HIP"]
        self.cadera = ((hl[0] + hr[0]) / 2, (hl[1] + hr[1]) / 2)
        sl, sr = self.k["LEFT SHOULDER"], self.k["RIGHT SHOULDER"]
        self.hombros = ((sl[0] + sr[0]) / 2, (sl[1] + sr[1]) / 2)
        self.ancho_hombros = abs(sl[0] - sr[0])
        if getattr(talla, "clave", "") == "minotauro":
            # La cola nace en la grupa del toro, no en la cintura humana.
            y = min(talla.alto - 1, int(round(self.cadera[1])) + 5)
            xs = np.nonzero(self.alfa[y])[0]
            if len(xs): self.cadera = (float(xs.min() + 2 if self.hacia_der else xs.max() - 2), float(y))

    def lienzo(self):
        return Image.new("L", (self.talla.ancho, self.talla.alto), 0)


def _mascara(img):
    return np.asarray(img) > 0


def _pintar(capa, mascara, m, tono="s", alfa=255):
    capa[mascara, :3] = m[tono]
    capa[mascara, 3] = alfa


def _linea(mask, x0, y0, x1, y1, grosor=1):
    img = Image.fromarray((mask * 255).astype(np.uint8))
    ImageDraw.Draw(img).line([(x0, y0), (x1, y1)], fill=255, width=grosor)
    mask |= _mascara(img)


# --- alas ----------------------------------------------------------------------------------------

def alas_frame(fr, ident):
    nombre, m1, m2 = ALAS[ident]
    capa = np.zeros((fr.talla.alto, fr.talla.ancho, 4), np.uint8)
    hx, hy = fr.hombros
    hx += fr.atras * 1
    hy = int(round(hy)) + 1
    extension = max(9, min(16, int(round(fr.ancho_hombros * 0.8 + 5))))
    altura = max(6, int(round(extension * 0.7)))
    bate = BATIR[fr.paso]
    membrana = np.zeros(capa.shape[:2], bool)
    nervios = np.zeros(capa.shape[:2], bool)
    for lado in (-1, 1):
        # En diagonal el ala lejana se acorta; no son dos formas frontales gemelas.
        factor = 1.0 if lado == fr.atras else 0.65
        span = max(5, int(round(extension * factor)))
        alto = max(4, int(round(altura * factor)))
        img = fr.lienzo()
        dr = ImageDraw.Draw(img)
        x0 = int(round(hx))
        tip = (x0 + lado * span, hy - alto + bate)
        if ident in ("alas_murcielago", "alas_dragon"):
            # membrana: del hombro a la punta, con el borde de abajo festoneado (tres dedos)
            # el borde de arriba sube del hombro a la punta; el de abajo vuelve festoneado (tres dedos)
            pts = [(x0, hy - 1), (x0 + lado * int(span * 0.5), hy - int(alto * 0.75)), tip,
                   (x0 + lado * int(span * 0.95), hy - int(alto * 0.35) + bate), (x0 + lado * int(span * 0.8), hy + 1 + bate),
                   (x0 + lado * int(span * 0.6), hy - 1), (x0 + lado * int(span * 0.45), hy + 2),
                   (x0 + lado * int(span * 0.25), hy), (x0 + lado * int(span * 0.12), hy + 3), (x0, hy + 1)]
            dr.polygon(pts, fill=255)
            for p in (pts[2], pts[4], pts[6]):
                _linea(nervios, x0, hy, p[0], p[1])
        elif ident == "alas_libelula":
            # dos alas ovaladas y finas por lado
            dr.ellipse([min(x0, tip[0]), tip[1] - 1, max(x0, tip[0]), hy + 1], fill=255)
            dr.ellipse([min(x0, x0 + lado * int(span * 0.7)), hy, max(x0, x0 + lado * int(span * 0.7)), hy + 5 + bate], fill=255)
            _linea(nervios, x0, hy, tip[0], tip[1])
            _linea(nervios, x0, hy + 1, x0 + lado * int(span * 0.7), hy + 4)
        else:
            # plumas: un ala redondeada que cae un poco, con las remeras en lineas
            # el borde de arriba sube hasta la punta, el de fuera baja redondo y el de abajo cae en plumas
            pts = [(x0, hy - 1), (x0 + lado * int(span * 0.35), hy - int(alto * 0.8)), (x0 + lado * int(span * 0.7), hy - alto + bate),
                   tip, (x0 + lado * int(span * 0.95), hy - int(alto * 0.4) + bate), (x0 + lado * int(span * 0.9), hy + 2 + bate),
                   (x0 + lado * int(span * 0.65), hy + 5), (x0 + lado * int(span * 0.4), hy + 5),
                   (x0 + lado * int(span * 0.15), hy + 3), (x0, hy + 1)]
            dr.polygon(pts, fill=255)
            for i in range(1, 4):
                _linea(nervios, x0 + lado * 2, hy + i, x0 + lado * (span - i), hy + 2 + i + bate // 2)
        membrana |= _mascara(img)
    membrana[:, :] &= np.arange(fr.talla.alto)[:, None] < fr.talla.alto - 1
    _sombrear(capa, membrana, MAT[m1])
    nervios &= membrana & ~(membrana & ~ndimage.binary_erosion(membrana))
    _pintar(capa, nervios, MAT[m2], "d")
    return capa


# --- colas ---------------------------------------------------------------------------------------

def cola_frame(fr, ident):
    nombre, m1, m2 = COLAS[ident]
    capa = np.zeros((fr.talla.alto, fr.talla.ancho, 4), np.uint8)
    cx, cy = fr.cadera
    x0 = int(round(cx + fr.atras * 2))
    y0 = int(round(cy)) + 1
    largo = max(8, min(15, int(round(fr.ancho_hombros * 0.5 + 6))))
    menea = BATIR[fr.paso]
    # la curva: sale hacia atras y abajo y remata hacia arriba
    curva = [(0, 0), (0.2, 0.15), (0.45, 0.3), (0.7, 0.3), (0.9, 0.1), (1.0, -0.2 + menea * 0.15)]
    pts = [(x0 + fr.atras * int(round(u * largo)), y0 + int(round(v * largo))) for u, v in curva]
    cuerpo = np.zeros(capa.shape[:2], bool)
    grosor = {"cola_diablo": 1, "cola_gato": 2, "cola_lagarto": 3, "cola_zorro": 3, "cola_lobo": 3}[ident]
    for i in range(len(pts) - 1):
        g = grosor
        if ident == "cola_lagarto":
            g = max(1, 3 - i // 2)  # se afina hacia la punta
        _linea(cuerpo, pts[i][0], pts[i][1], pts[i + 1][0], pts[i + 1][1], g)
    punta = np.zeros_like(cuerpo)
    if ident == "cola_diablo":
        # la punta de flecha
        px, py = pts[-1]
        for dx, dy in ((0, 0), (-1, 1), (1, 1), (0, -1), (-1, 0), (1, 0)):
            x, y = px + dx * fr.atras, py + dy
            if 0 <= x < fr.talla.ancho and 0 <= y < fr.talla.alto:
                punta[y, x] = True
    elif ident in ("cola_zorro", "cola_lobo"):
        # esponjosa: engorda y la punta es clara
        cuerpo = ndimage.binary_dilation(cuerpo)
        px, py = pts[-1]
        punta = np.zeros_like(cuerpo)
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                x, y = px + dx, py + dy
                if 0 <= x < fr.talla.ancho and 0 <= y < fr.talla.alto:
                    punta[y, x] = True
        punta &= ndimage.binary_dilation(cuerpo)
        cuerpo |= punta
    elif ident == "cola_gato":
        px, py = pts[-1]
        if 0 <= py - 1 < fr.talla.alto and 0 <= px < fr.talla.ancho:
            punta[py - 1, px] = True
        cuerpo |= punta
    cuerpo |= punta
    cuerpo &= np.arange(fr.talla.alto)[:, None] < fr.talla.alto - 1
    _sombrear(capa, cuerpo, MAT[m1])
    if ident == "cola_diablo":
        _pintar(capa, punta, MAT[m2], "s")
    elif ident in ("cola_zorro", "cola_lobo", "cola_gato"):
        _pintar(capa, punta & cuerpo, MAT[m2], "H")
    elif ident == "cola_lagarto":
        # el lomo, un poco mas oscuro cada dos columnas
        lomo = cuerpo & ~ndimage.binary_erosion(cuerpo) & (np.arange(fr.talla.ancho)[None, :] % 2 == 0)
        _pintar(capa, lomo, MAT[m2], "d")
    return capa


# --- cuernos -------------------------------------------------------------------------------------

def cuernos_frame(fr, ident):
    nombre, m1, m2 = CUERNOS[ident]
    capa = np.zeros((fr.talla.alto, fr.talla.ancho, 4), np.uint8)
    if fr.top is None:
        return capa
    cuerno = np.zeros(capa.shape[:2], bool)
    punta = np.zeros_like(cuerno)
    for lado in (-1, 1):
        bx = int(round(fr.cx + lado * fr.semi * 0.55))
        by = fr.top + 1
        if ident == "cuernos_rectos":
            _linea(cuerno, bx, by, bx + lado * 2, by - 5, 2)
            _linea(cuerno, bx + lado * 2, by - 5, bx + lado * 3, by - 7, 1)
        elif ident == "cuernos_carnero":
            # el rizo: hacia atras y abajo alrededor de la oreja
            pts = [(0, 0), (1, -2), (3, -2), (4, 0), (3, 2), (2, 2)]
            for i in range(len(pts) - 1):
                _linea(cuerno, bx + lado * pts[i][0], by + pts[i][1], bx + lado * pts[i + 1][0], by + pts[i + 1][1], 2)
        elif ident == "cuernos_astas":
            _linea(cuerno, bx, by, bx + lado * 2, by - 6, 1)
            _linea(cuerno, bx + lado * 1, by - 3, bx + lado * 4, by - 5, 1)
            _linea(cuerno, bx + lado * 2, by - 6, bx + lado * 1, by - 9, 1)
            _linea(cuerno, bx + lado * 2, by - 6, bx + lado * 4, by - 8, 1)
        elif ident == "cuernos_diablo":
            _linea(cuerno, bx, by, bx + lado * 1, by - 3, 1)
            if 0 <= by - 3 < fr.talla.alto and 0 <= bx + lado * 1 < fr.talla.ancho:
                punta[by - 3, bx + lado * 1] = True
        else:  # toro: hacia fuera y luego arriba, gruesos
            _linea(cuerno, bx, by + 1, bx + lado * 4, by - 1, 2)
            _linea(cuerno, bx + lado * 4, by - 1, bx + lado * 5, by - 5, 2)
            if 0 <= by - 5 < fr.talla.alto and 0 <= bx + lado * 5 < fr.talla.ancho:
                punta[by - 5, bx + lado * 5] = True
    cuerno &= np.arange(fr.talla.alto)[:, None] >= 0
    _sombrear(capa, cuerno, MAT[m1])
    _pintar(capa, punta & cuerno, MAT[m2], "H")
    return capa


# --- marcas en la piel ---------------------------------------------------------------------------

def _en_hueso(fr, a, b, t):
    """Punto a lo largo del hueso a -> b (0..1), redondeado."""
    (ax, ay), (bx, by) = fr.k[a][:2], fr.k[b][:2]
    return int(round(ax + (bx - ax) * t)), int(round(ay + (by - ay) * t))


def _mancha(mask, x, y, r=1):
    yy, xx = np.mgrid[0:mask.shape[0], 0:mask.shape[1]]
    mask |= (yy - y) ** 2 + (xx - x) ** 2 <= r * r


def marcas_frame(fr, ident):
    nombre, m1, _m2 = MARCAS[ident]
    capa = np.zeros((fr.talla.alto, fr.talla.ancho, 4), np.uint8)
    piel = fr.alfa & np.isin(fr.lab, ["h", "n", "t", "p", "a", "A", "m", "M", "l", "L", "f", "F"])
    marca = np.zeros(capa.shape[:2], bool)
    tono = "s"
    yy, xx = np.mgrid[0:fr.talla.alto, 0:fr.talla.ancho]
    if ident == "pecas" and fr.frente and fr.top is not None:
        for dx, dy in ((1, 0), (3, 1), (2, 1), (-1, 1), (4, 0)):
            x, y = int(round(fr.cx + fr.delante * dx)), int(round(fr.ojos_y + 1 + dy * 0.5))
            if 0 <= x < fr.talla.ancho and 0 <= y < fr.talla.alto and fr.cab[y, x]:
                marca[y, x] = True
        tono = "d"
    elif ident in ("manchas", "lunares"):
        r = 1.2 if ident == "manchas" else 0.5
        sitios = [("LEFT SHOULDER", "LEFT ELBOW", 0.4), ("RIGHT SHOULDER", "RIGHT ELBOW", 0.6),
                  ("LEFT HIP", "LEFT KNEE", 0.3), ("RIGHT HIP", "RIGHT KNEE", 0.7), ("NECK", "LEFT HIP", 0.45),
                  ("NECK", "RIGHT HIP", 0.7), ("LEFT KNEE", "LEFT LEG", 0.4), ("RIGHT ELBOW", "RIGHT ARM", 0.3)]
        if ident == "lunares":
            sitios = sitios[:3] + [("NOSE", "LEFT EYE", 0.5)]
        for a, b, t in sitios:
            if a in fr.k and b in fr.k:
                x, y = _en_hueso(fr, a, b, t)
                _mancha(marca, x, y, r)
        tono = "d" if ident == "lunares" else "s"
    elif ident == "rayas":
        for a, b in (("LEFT SHOULDER", "LEFT ELBOW"), ("RIGHT SHOULDER", "RIGHT ELBOW"), ("LEFT ELBOW", "LEFT ARM"),
                     ("RIGHT ELBOW", "RIGHT ARM"), ("LEFT HIP", "LEFT KNEE"), ("RIGHT HIP", "RIGHT KNEE"),
                     ("LEFT KNEE", "LEFT LEG"), ("RIGHT KNEE", "RIGHT LEG"), ("NECK", "LEFT HIP"), ("NECK", "RIGHT HIP")):
            for t in (0.3, 0.65):
                x, y = _en_hueso(fr, a, b, t)
                marca |= (yy == y) & (np.abs(xx - x) <= 2)
        if fr.top is not None:
            marca |= fr.cab & (yy == fr.top + 3) & (np.abs(xx - fr.cx) <= 2)
        tono = "d"
    elif ident == "tatuaje_brazo":
        lado = "RIGHT" if fr.lab[fr.lab == "A"].size else "LEFT"
        x, y = _en_hueso(fr, f"{lado} SHOULDER", f"{lado} ELBOW", 0.45)
        # un anillo con una punta
        for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1), (0, 2)):
            if 0 <= y + dy < fr.talla.alto and 0 <= x + dx < fr.talla.ancho:
                marca[y + dy, x + dx] = True
        marca |= (yy == y - 2) & (np.abs(xx - x) <= 1)
    elif ident == "tatuaje_cara" and fr.frente:
        ex = int(round(fr.cx + fr.delante * 2))
        for dy in (1, 2, 3):
            y = int(round(fr.ojos_y + dy))
            if 0 <= y < fr.talla.alto:
                marca[y, ex] = True
        y = int(round(fr.ojos_y - 2))
        marca |= (yy == y) & (np.abs(xx - (ex + fr.delante)) <= 1)
    elif ident == "tatuaje_pecho" and fr.frente:
        nx, ny = fr.k["NECK"][:2]
        x0, y0 = int(round(nx)), int(round(ny)) + 3
        # un rombo
        for dy in range(-2, 3):
            w = 2 - abs(dy)
            marca |= (yy == y0 + dy) & (np.abs(xx - x0) == w)
    elif ident == "cicatriz_ojo" and fr.frente:
        ex = int(round(fr.cx + fr.delante * 2))
        _linea(marca, ex - 1, int(fr.ojos_y) - 3, ex + 1, int(fr.ojos_y) + 2)
        tono = "d"
    elif ident == "cicatriz_pecho" and fr.frente:
        nx, ny = fr.k["NECK"][:2]
        _linea(marca, int(nx) - 3, int(ny) + 2, int(nx) + 2, int(ny) + 6)
        tono = "d"
    elif ident == "pintura_guerra" and fr.frente:
        for dy in (-1, 1):
            y = int(round(fr.ojos_y + dy))
            marca |= fr.cab & (yy == y) & ((xx - fr.cx) * fr.delante >= -2)
    elif ident == "pintura_ojos" and fr.frente:
        ojos = np.asarray(fr.talla.ojos[fr.f]) > 0
        # una sombra bajo cada ojo
        marca |= np.roll(ojos, 1, axis=0) & ~ojos & fr.cab
        tono = "o"
    marca &= piel
    if ident not in ("pintura_ojos",):
        marca &= ~(np.asarray(fr.talla.ojos[fr.f]) > 0)
    _pintar(capa, marca, MAT[m1], tono)
    return capa


# --- capas enteras -------------------------------------------------------------------------------

def capa_extra(talla, ident):
    """Frames RGBA de una pieza de ALAS, COLAS, CUERNOS o MARCAS en la talla."""
    if ident in ALAS:
        fn = alas_frame
    elif ident in COLAS:
        fn = cola_frame
    elif ident in CUERNOS:
        fn = cuernos_frame
    elif ident in MARCAS:
        fn = marcas_frame
    else:
        raise KeyError(ident)
    return [fn(_Frame(talla, f), ident) for f in range(1, talla.n_frames + 1)]


def es_extra(ident):
    return ident in ALAS or ident in COLAS or ident in CUERNOS or ident in MARCAS
