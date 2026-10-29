"""Tallas de cuerpo: el cuerpo base y tres derivados por cirugia de pixeles.

Las razas del catalogo van de 0,90 m (medianos) a 2,50 m (minotauros).
Escalar pixel-art por un factor no entero lo destroza, asi que las tallas
se derivan quitando o duplicando FILAS y COLUMNAS enteras del cuerpo base,
siempre en el mismo sitio anatomico de cada frame:

  filas del torso    cinturon - 4 y cinturon - 6 (pecho y vientre: se
                     acortan a la vez torso y brazos)
  filas de la pierna cinturon + 5 y cinturon + 6 (la rodilla: en los pasos
                     de lado las piernas van en diagonal, y a esa altura
                     quitar una fila solo las acorta)
  columnas           por el interior del torso y las piernas, solo por
                     debajo de la cabeza: la cabeza no cambia, asi que el
                     pelo, los cascos y los rasgos de raza sirven igual.

La misma operacion se aplica al dibujo y a su mapa de partes, de modo que
toda prenda pintada sobre una talla le encaja pixel a pixel. Los pies no
se mueven: siguen apoyando en la misma fila de la celda.
"""
from pathlib import Path

import numpy as np

import actores_partes as partes
from actores_pintura import clasificar

BASE = Path(__file__).resolve().parent.parent / "assets/textures/personaje.png"
FRAME = 64

# Cabeza del frame quieto de cada direccion en el cuerpo base: las
# plantillas (pelo, cascos, rasgos) se dibujaron contra ella.
TOP_QUIETO = {"abajo": 21, "arriba": 21, "derecha": 20}

# Operaciones por talla. Filas relativas al cinturon del frame; columnas
# relativas al eje del torso del frame (frente/espalda: x = 32).
TALLAS = {
    "medio": {"nombre": "Normal", "altura_m": "1,50-1,90",
              "quitar_filas": [], "duplicar_filas": [], "duplicar_columnas": []},
    "bajo": {"nombre": "Bajo y ancho (enanos)", "altura_m": "1,20-1,50",
             "quitar_filas": [-4, 5, 6, 7], "duplicar_filas": [], "duplicar_columnas": [-2, 2]},
    "pequeno": {"nombre": "Pequeno (medianos)", "altura_m": "0,90-1,20",
                "quitar_filas": [-2, -4, -6, 4, 5, 6, 7], "duplicar_filas": [],
                "duplicar_columnas": []},
    "grande": {"nombre": "Grande (orcos, minotauros, lagartos, draconidos)", "altura_m": "1,80-2,50",
               "quitar_filas": [], "duplicar_filas": [-3, -5, -7, 5, 6],
               "duplicar_columnas": [-3, -1, 1, 3]},
}


class Talla:
    """Los frames de un cuerpo (12 de andar y 8 de reposo) con su mapa de partes, tonos y ojos."""

    def __init__(self, clave, frames, labs):
        self.clave = clave
        self.frames = frames          # lista de RGBA (paleta del cuerpo base), frame f en [f - 1]
        self.labs = labs              # {1..n: etiquetas}
        self.alto = self.ancho = FRAME
        todos = range(1, len(frames) + 1)
        self.tonos = {f: clasificar(frames[f - 1]) for f in todos}
        self.ojos = {f: np.char.startswith(self.tonos[f], "e:") for f in todos}
        self.alfa = {f: frames[f - 1][..., 3] > 0 for f in todos}
        self.top = {f: int(np.nonzero(self.alfa[f].any(axis=1))[0].min()) for f in todos}

    @property
    def n_frames(self):
        return len(self.frames)

    @staticmethod
    def direccion(f):
        return partes.DIRECCION[f]

    def dy_cabeza(self, f):
        """Cuanto baja la cabeza en f respecto al frame quieto del cuerpo base."""
        d = self.direccion(f)
        return self.top[f] - TOP_QUIETO["derecha" if d == "izquierda" else d]

    def altura(self):
        """Alto del cuerpo (de la coronilla a la suela) en el frame quieto de frente."""
        ys = np.nonzero(self.alfa[8].any(axis=1))[0]
        return int(ys.max() - ys.min() + 1)

    def sin_piernas(self):
        """La misma talla sin piernas ni pies: para vestir a nagas y espectros."""
        if getattr(self, "_sin_piernas", None) is not None:
            return self._sin_piernas
        frames, labs = [], {}
        for f in range(1, self.n_frames + 1):
            img, lab = self.frames[f - 1].copy(), self.labs[f].copy()
            piernas = np.isin(lab, ["l", "L", "f", "F"])
            img[piernas] = 0
            lab[piernas] = ""
            frames.append(img)
            labs[f] = lab
        t = Talla(self.clave, frames, labs)
        t.top = dict(self.top)  # la cabeza no se mueve
        self._sin_piernas = t
        return t


def _eje_torso(lab, direccion):
    if direccion in ("abajo", "arriba"):
        return 32
    ys, xs = np.nonzero(lab == "t")
    return int(round(xs.mean())) if len(xs) else 32


def _operar(img, lab, op, direccion):
    """Aplica una operacion de talla a un frame (imagen y etiquetas)."""
    cint = int(np.nonzero((lab == "p").any(axis=1))[0].min())
    filas_h = np.nonzero((lab == "h").any(axis=1))[0]
    bajo_cabeza = int(filas_h.max()) + 1
    # 1) columnas, solo por debajo de la cabeza
    if op["duplicar_columnas"]:
        eje = _eje_torso(lab, direccion)
        cols = sorted(eje + c for c in op["duplicar_columnas"])
        orden = []
        for x in range(FRAME):
            orden.append(x)
            if x in cols:
                orden.append(x)
        # recentrar: tantas columnas nuevas a cada lado como a la izquierda del eje
        izq = sum(1 for c in cols if c < eje)
        orden = orden[izq:izq + FRAME]
        nueva_img, nuevo_lab = img.copy(), lab.copy()
        nueva_img[bajo_cabeza:] = img[bajo_cabeza:, orden]
        nuevo_lab[bajo_cabeza:] = lab[bajo_cabeza:, orden]
        img, lab = nueva_img, nuevo_lab
    # 2) filas, alineando la suela con la fila de siempre
    quitar = {cint + k for k in op["quitar_filas"]}
    duplicar = {cint + k for k in op["duplicar_filas"]}
    filas = []
    for y in range(FRAME):
        if y in quitar:
            continue
        filas.append(y)
        if y in duplicar:
            filas.append(y)
    ultima_opaca = int(np.nonzero(img[..., 3].any(axis=1))[0].max())
    fondo = FRAME - 1 - ultima_opaca          # filas vacias bajo la suela
    idx = [y for y in filas if y <= ultima_opaca]
    nueva_img = np.zeros_like(img)
    nuevo_lab = np.full_like(lab, "")
    y0 = FRAME - fondo - len(idx)
    for i, y in enumerate(idx):
        if 0 <= y0 + i < FRAME:
            nueva_img[y0 + i] = img[y]
            nuevo_lab[y0 + i] = lab[y]
    return nueva_img, nuevo_lab


def respirar(img, lab):
    """El frame quieto respirando (ESTETICA-01, reposo): todo lo que esta por encima de las
    piernas baja un pixel; la primera fila de las piernas queda debajo del cinturon."""
    piernas = np.isin(lab, ["l", "L", "f", "F"])
    filas = np.nonzero(piernas.any(axis=1))[0]
    corte = int(filas.min()) if len(filas) else FRAME - 1
    nueva_img, nuevo_lab = img.copy(), lab.copy()
    nueva_img[1:corte + 1] = img[0:corte]
    nuevo_lab[1:corte + 1] = lab[0:corte]
    nueva_img[0] = 0
    nuevo_lab[0] = ""
    # en la fila del corte, donde lo de encima no tapa (entre las piernas), sigue la pierna
    hueco = img[corte - 1, :, 3] == 0
    nueva_img[corte, hueco] = img[corte, hueco]
    nuevo_lab[corte, hueco] = lab[corte, hueco]
    return nueva_img, nuevo_lab


def con_reposo(frames, labs):
    """Los 12 frames de andar mas los 8 de reposo (actores_partes.REPOSO): el primero de
    cada direccion es el cuerpo quieto y el segundo, respirando."""
    frames = list(frames)
    labs = dict(labs)
    for f in sorted(partes.REPOSO):
        q = partes.REPOSO[f]
        if partes.paso_de(f) == "reposo2":
            img, lab = respirar(frames[q - 1], labs[q])
        else:
            img, lab = frames[q - 1].copy(), labs[q].copy()
        frames.append(img)
        labs[f] = lab
    return frames, labs


def construir_tallas(ruta_base=BASE):
    frames, labs, errores = partes.construir(ruta_base)
    if errores:
        raise SystemExit("mapa de partes incompleto:\n  " + "\n  ".join(errores))
    tallas = {}
    for clave, op in TALLAS.items():
        if clave == "medio":
            tallas[clave] = Talla(clave, *con_reposo(frames, labs))
            continue
        nf, nl = [], {}
        for f in range(1, partes.N_FRAMES + 1):
            img, lab = _operar(frames[f - 1], labs[f], op, partes.DIRECCION[f])
            nf.append(img)
            nl[f] = lab
        tallas[clave] = Talla(clave, *con_reposo(nf, nl))
    return tallas
