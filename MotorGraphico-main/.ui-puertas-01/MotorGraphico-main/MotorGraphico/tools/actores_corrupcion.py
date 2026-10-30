"""PNJ-02 — La corrupcion de los drows (y de las sagas, que son drows).

Oriol (05/10/2026): «los sagas son los drows, por eso quiero darles este toque de
corrupcion estetica a los drow y ponerles musgos, cortezas, piedras o partes de
animales, mutando su aspecto de elfo al de un ser mas perturbador sin abandonar la
forma de su ancestro, solo que ahora mas extraña».

Los cuatro pueblos drow conservan lo que los hace elfos (las orejas puntiagudas, el
cuerpo esbelto, el pelo claro) y la corrupcion se les pega encima, de menos a mas:

  drow de la casa noble   piedra: una corona de esquirlas de obsidiana que les nace del
                          craneo y la corteza que les sube por la mandibula
  drow explorador         bosque: astas de rama con musgo y musgo en los hombros
  sagas de la locura      lo peor del bosque: astas retorcidas (la otra, rota), corteza
                          en media cara y en las manos, musgo colgando
  sagas del abismo        lo peor de la piedra: cuernos de carnero de piedra negra,
                          placas de piedra en la cara, manos de piedra y un tercer ojo

Las plantillas siguen el formato de actores_tocados (x desde 22 en la fila absoluta del
frame quieto de cada direccion; la izquierda es el espejo de la derecha), pero aqui se
dibujan por coordenadas con Dibujo: un trazo es una linea de 1 px de relleno y el
contorno se lo pone contorno(); de frente y de espaldas, la mitad derecha es el espejo
de la izquierda (o se dibuja aparte cuando el rasgo no es simetrico).

Material 1 (1-5) es la corteza o la piedra; material 2 (6-9) el musgo o el brillo.
"""
from actores_tocados import X0

# luz desde la izquierda: al pasar a la mitad derecha, la luz se vuelve sombra
_ESPEJO = {"1": "3", "6": "8"}
_RELLENOS = "123678wk"


def linea(a, b):
    """Los pixeles de un segmento (Bresenham), de a a b."""
    (x0, y0), (x1, y1) = a, b
    dx, dy = abs(x1 - x0), -abs(y1 - y0)
    sx, sy = (1 if x0 < x1 else -1), (1 if y0 < y1 else -1)
    err, out = dx + dy, []
    while True:
        out.append((x0, y0))
        if (x0, y0) == (x1, y1):
            return out
        e2 = 2 * err
        if e2 >= dy:
            err += dy
            x0 += sx
        if e2 <= dx:
            err += dx
            y0 += sy


class Dibujo:
    """Pixeles {(x, y): caracter de plantilla} que acaban en una plantilla."""

    def __init__(self):
        self.px = {}

    def trazo(self, puntos, c="2"):
        """Una polilinea de relleno de 1 px (el contorno, despues)."""
        if len(puntos) == 1:
            self.px[puntos[0]] = c
        for a, b in zip(puntos, puntos[1:]):
            for p in linea(a, b):
                self.px[p] = c
        return self

    def pon(self, c, *puntos):
        for p in puntos:
            self.px[p] = c
        return self

    def contorno(self, c="4"):
        """Contorno de 1 px (vecinos en cruz) alrededor de todo el relleno."""
        nuevos = {}
        for (x, y), ch in self.px.items():
            if ch in _RELLENOS:
                for v in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
                    if v not in self.px:
                        nuevos[v] = c
        self.px.update(nuevos)
        return self

    def con_espejo(self):
        """Anade la mitad derecha (espejo en el eje x = 32; x -> 64 - x)."""
        for (x, y), c in list(self.px.items()):
            if x < 32:
                self.px.setdefault((64 - x, y), _ESPEJO.get(c, c))
        return self

    def espejada(self):
        """Otra Dibujo, espejo completo (de espaldas, un rasgo que no es simetrico
        se ve del otro lado); la luz sigue viniendo de la izquierda."""
        otra = Dibujo()
        cambio = {"1": "3", "3": "1", "6": "8", "8": "6"}
        otra.px = {(64 - x, y): cambio.get(c, c) for (x, y), c in self.px.items()}
        return otra

    def plantilla(self):
        """(fila0, filas) en el formato de actores_tocados."""
        if min(x for x, _ in self.px) < X0:
            raise ValueError("una plantilla no puede pintar a la izquierda de x = 22")
        y0, y1 = min(y for _, y in self.px), max(y for _, y in self.px)
        filas = []
        for y in range(y0, y1 + 1):
            xs = [x for x, yy in self.px if yy == y]
            filas.append("".join(self.px.get((x, y), ".") for x in range(X0, max(xs) + 1)) if xs else "")
        return (y0, filas)


def _rasgo(abajo, derecha, arriba=None, simetrico=True):
    """Plantilla por direccion. De espaldas: la que se diga, la de frente si el rasgo
    es simetrico, o su espejo si no lo es."""
    p = {"abajo": abajo.plantilla(), "derecha": derecha.plantilla()}
    p["arriba"] = (arriba or (abajo if simetrico else abajo.espejada())).plantilla()
    return p


# --- drow de la casa noble: piedra ----------------------------------------------------
# Esquirlas de obsidiana que les nacen del craneo como una corona (encima del pelo),
# y corteza que les sube por la mandibula.

def _esquirla(d, x, punta, base, inclina=0):
    """Una esquirla de 2 px: cara de luz a la izquierda y de sombra a la derecha."""
    for y in range(punta, base + 1):
        dx = round(inclina * (base - y) / max(1, base - punta))
        d.pon("1" if y > punta else "6", (x + dx, y))
        if y >= punta + 2:
            d.pon("3", (x + dx + 1, y))


def _corona_frente():
    # crece mas por un lado: tres esquirlas a la izquierda (la mas alta, inclinada hacia
    # fuera) y dos pequenas a la derecha; de espaldas, al reves
    d = Dibujo()
    _esquirla(d, 27, 13, 21, inclina=-1)
    _esquirla(d, 30, 16, 21)
    _esquirla(d, 25, 17, 22, inclina=-2)
    _esquirla(d, 34, 17, 21)
    _esquirla(d, 37, 19, 23, inclina=1)
    d.contorno()
    d.pon("w", (26, 15))
    return d


def _corona_lado():
    d = Dibujo()
    _esquirla(d, 30, 12, 20, inclina=-1)
    _esquirla(d, 33, 16, 20, inclina=1)
    _esquirla(d, 27, 15, 21, inclina=-2)
    _esquirla(d, 25, 19, 22, inclina=-1)
    d.contorno()
    d.pon("w", (29, 14))
    return d


CORONA_PIEDRAS = _rasgo(_corona_frente(), _corona_lado(), simetrico=False)


def _corteza_mandibula_frente():
    d = Dibujo()
    d.pon("2", (27, 31), (27, 32), (28, 33))
    d.pon("3", (28, 32), (29, 33))
    d.pon("4", (26, 31), (26, 30), (27, 33), (28, 34), (29, 34))
    return d


def _corteza_mandibula_lado():
    d = Dibujo()
    d.pon("2", (33, 32), (34, 33))
    d.pon("3", (32, 32), (33, 33))
    d.pon("4", (32, 31), (34, 34), (32, 33))
    return d


CORTEZA_MANDIBULA = {"abajo": _corteza_mandibula_frente().plantilla(),
                     "derecha": _corteza_mandibula_lado().plantilla()}


# --- drow explorador: astas de rama y musgo --------------------------------------------

def _astas_frente():
    d = Dibujo()
    d.trazo([(29, 21), (28, 19), (27, 17), (26, 15), (26, 13)])   # rama
    d.trazo([(27, 18), (25, 17), (24, 16)])                        # candil de fuera
    d.trazo([(26, 15), (24, 13)])                                  # horquilla de arriba
    d.pon("7", (28, 20)).pon("6", (27, 18))                        # musgo en la base
    d.contorno().con_espejo()
    return d


def _astas_lado():
    d = Dibujo()
    # la cercana sube y se va hacia atras; la lejana asoma delante, en sombra
    d.trazo([(32, 20), (31, 18), (30, 16), (29, 14), (29, 12)])
    d.trazo([(31, 18), (33, 17), (34, 16)])
    d.trazo([(30, 16), (28, 15)])
    d.trazo([(35, 19), (36, 17), (37, 15)], c="3")
    d.pon("7", (32, 19)).pon("6", (31, 18))
    d.contorno()
    return d


ASTAS_RAMAS = _rasgo(_astas_frente(), _astas_lado())


def _musgo_hombros_frente():
    d = Dibujo()
    d.pon("2", (26, 36), (27, 36), (27, 35), (25, 37))
    d.pon("1", (26, 35))
    d.pon("3", (28, 36))
    d.contorno().con_espejo()
    return d


def _musgo_hombros_lado():
    d = Dibujo()
    d.pon("2", (30, 35), (31, 35), (29, 36))
    d.pon("1", (30, 34))
    d.pon("3", (32, 35))
    d.contorno()
    return d


MUSGO_HOMBROS = _rasgo(_musgo_hombros_frente(), _musgo_hombros_lado())


# --- sagas de la locura: lo peor del bosque ----------------------------------------------
# Astas retorcidas (la izquierda grande, con tres candiles y musgo colgando; la derecha,
# un munon roto con musgo) y corteza que les cubre media cara alrededor del ojo.

def _astas_retorcidas_frente():
    d = Dibujo()
    d.trazo([(29, 21), (28, 19), (26, 17), (25, 15), (25, 13), (24, 11), (24, 9)])
    d.trazo([(26, 17), (24, 17), (23, 16)])          # candil de abajo
    d.trazo([(25, 13), (23, 12)])                    # candil de en medio
    d.trazo([(25, 14), (27, 12), (27, 10)])          # candil de dentro
    d.trazo([(35, 21), (36, 19), (37, 17)])          # el munon roto
    d.pon("3", (38, 16))
    d.contorno()
    for p in ((23, 18), (23, 19), (24, 18), (27, 20), (36, 16), (37, 16), (38, 17)):
        d.px[p] = "7"                                # musgo colgando
    d.pon("6", (36, 15), (23, 18))
    d.pon("8", (24, 19), (23, 20), (27, 21), (38, 18))
    return d


def _astas_retorcidas_lado():
    d = Dibujo()
    d.trazo([(31, 20), (30, 18), (28, 16), (27, 14), (27, 12), (26, 10), (26, 8)])
    d.trazo([(28, 16), (26, 16), (24, 15)])
    d.trazo([(27, 12), (25, 11), (24, 11)])
    d.trazo([(27, 13), (29, 11), (29, 9)])
    d.trazo([(34, 20), (35, 18), (36, 17)], c="3")   # el munon, delante y en sombra
    d.contorno()
    for p in ((25, 17), (25, 18), (24, 17), (35, 16), (36, 16)):
        d.px[p] = "7"
    d.pon("8", (25, 19), (24, 18))
    return d


ASTAS_RETORCIDAS = _rasgo(_astas_retorcidas_frente(), _astas_retorcidas_lado(), simetrico=False)


def _media_cara(d, grieta="4"):
    """Media cara (la izquierda de la pantalla) tomada alrededor del ojo, que queda al
    aire: de la ceja a la mandibula, del borde de la cara a la nariz."""
    filas = {   # x: caracter (el ojo, x 28-30 de las filas 30-32, no se toca)
        28: {27: "4", 28: "2", 29: "1", 30: "2"},
        29: {25: "4", 26: "2", 27: "4", 28: "2", 29: "3", 30: grieta},
        30: {25: "4", 26: "2", 27: "2"},
        31: {26: "4", 27: "3"},
        32: {27: grieta},
        33: {28: "2", 29: grieta, 30: "2", 31: "3"},
        34: {29: "4", 30: "2", 31: "4"},
    }
    for y, xs in filas.items():
        for x, c in xs.items():
            d.pon(c, (x, y))
    return d


def _media_cara_lado(d, grieta="4"):
    """La mejilla que se ve de perfil, de la sien a la mandibula, con el ojo al aire."""
    filas = {
        27: {30: "4", 31: "2"},
        28: {30: "2", 31: "1"},
        29: {30: "3", 31: "2"},
        30: {31: "4", 32: "2", 33: "2", 34: "3"},
        31: {32: "2", 33: grieta, 34: "2", 35: "2", 36: "3"},
        32: {31: "4", 32: "2", 33: "2", 34: grieta, 35: "2"},
        33: {32: "3", 33: "2", 34: "4"},
        34: {32: "4", 33: "4"},
    }
    for y, xs in filas.items():
        for x, c in xs.items():
            d.pon(c, (x, y))
    return d


def _corteza_cara_frente():
    return _media_cara(Dibujo())


def _corteza_cara_lado():
    return _media_cara_lado(Dibujo())


CORTEZA_CARA = _rasgo(_corteza_cara_frente(), _corteza_cara_lado(), simetrico=False)
CORTEZA_CARA.pop("arriba")  # de espaldas no se ve la cara


# --- sagas del abismo: lo peor de la piedra ------------------------------------------------
# Cuernos de carnero de piedra negra que se enroscan sobre las orejas, placas de piedra
# en media cara (la otra mitad que en la locura) y un tercer ojo en la frente.

def _cuernos_carnero_frente():
    d = Dibujo()
    d.trazo([(29, 22), (28, 20), (26, 19), (25, 19), (24, 20), (23, 22), (24, 24), (25, 25)])
    d.trazo([(29, 21), (27, 20)])                    # la raiz, mas gruesa
    d.contorno()
    for p in ((26, 19), (24, 20), (23, 22)):
        d.px[p] = "1"                                # las vueltas, con luz
    d.con_espejo()
    return d


def _cuernos_carnero_lado():
    d = Dibujo()
    d.trazo([(31, 21), (30, 19), (28, 18), (26, 18), (25, 19), (25, 21), (26, 23), (28, 23), (29, 22)])
    d.trazo([(31, 20), (29, 19)])
    d.contorno()
    for p in ((28, 18), (25, 20), (28, 23)):
        d.px[p] = "1"
    return d


CUERNOS_CARNERO = _rasgo(_cuernos_carnero_frente(), _cuernos_carnero_lado())


def _placas_piedra_frente():
    # la otra media cara; las juntas de las placas, con luz en el canto de arriba
    return _media_cara(Dibujo(), grieta="1").espejada()


def _placas_piedra_lado():
    return _media_cara_lado(Dibujo(), grieta="1")


PLACAS_PIEDRA = _rasgo(_placas_piedra_frente(), _placas_piedra_lado(), simetrico=False)
PLACAS_PIEDRA.pop("arriba")

TERCER_OJO = {  # en la frente, encima del flequillo: material 2 = el brillo
    "abajo": (24, ["..........4", ".........4w4", ".........474", "..........4"]),
    "derecha": (24, ["...............4", "................w4", "...............4"]),
}
