"""INSTRUMENTOS-01 — Instrumentos y objetos de rol en la mano (tools/objetos_rol.py).

Oriol (08/10/2026): «Aprovechamos para crear una variedad de instrumentos musicales? Escobas y otros objetos
que no sean armas pero podamos usar en un juego de rol? Para hacer el tonto, por ejemplo una almohada,
cerveza, una antorcha, estandarte, shirukens [...] también una cuerda». Se ven en la mano como las armas
(ranura arma): la antorcha es la tea de siempre; lo demas, formas nuevas (la jarra de cerveza, cogida por el asa:
la de los PNJ, en la izquierda, la tapaba el puno).

Como se cogen:
- sin pose, a su tamano y colgando del puno (como el laud): la flauta, la flauta de pan, la gaita, el arpa,
  la lira, la pandereta, la vihuela, la trompeta, la almohada, los shurikens, el catalejo, las bolas de
  malabares, la cuerda y la jarra de cerveza;
- con pose, como un baston (en alto, la cabeza arriba): la escoba (el cepillo arriba, como quien la blande),
  la pala, la cana de pescar y el estandarte (este, de perfil, recto); como un cetro (corto, un poco
  inclinado): el pico y la sarten;
- en las dos manos (actores_armas.PAREJA, como el tambor y su baqueta): el arco en la derecha y el violin en
  la izquierda.

Las letras de siempre:

    1-5 material 1 (luz, base, sombra, contorno, contorno duro)   6-9 material 2   a-e material 3
    f-j material 4   @ el ancla en el material 2 (& en el 1, $ en el 3)

Va al final del atlas de armas (ninguna fila de antes se mueve): la importa actores_formas detras de
actores_formas_iteracion3.
"""
import actores_formas as F
import actores_poses as poses

# letras de cada material: (luz, base, sombra, contorno)
_LETRAS = {1: "1234", 2: "6789", 3: "abcd", 4: "fghi"}


def _relleno(lienzo, pixeles, material):
    """Pinta una region en el lienzo {(x, y): letra} con su contorno, su luz arriba a la izquierda y su
    sombra abajo a la derecha (lo redondo: el parche de la pandereta, la caja de la vihuela)."""
    luz, base, sombra, contorno = _LETRAS[material]
    pixeles = set(pixeles)
    xs = [x for x, _ in pixeles]
    ys = [y for _, y in pixeles]
    x0, x1, y0, y1 = min(xs), max(xs), min(ys), max(ys)
    for (x, y) in pixeles:
        if any(v not in pixeles for v in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1))):
            lienzo[(x, y)] = contorno
            continue
        u = (x - x0) / max(1, x1 - x0)
        v = (y - y0) / max(1, y1 - y0)
        t = (1 - u) * 0.55 + (1 - v) * 0.45
        lienzo[(x, y)] = luz if t > 0.68 else sombra if t < 0.34 else base


def _filas(lienzo):
    ancho = max(x for x, _ in lienzo) + 1
    alto = max(y for _, y in lienzo) + 1
    return ["".join(lienzo.get((x, y), ".") for x in range(ancho)) for y in range(alto)]


def _disco(cx, cy, r):
    return {(x, y) for x in range(int(cx - r) - 1, int(cx + r) + 2) for y in range(int(cy - r) - 1, int(cy + r) + 2)
            if (x - cx) ** 2 + (y - cy) ** 2 <= r * r}


def _ovalo(cx, cy, rx, ry):
    return {(x, y) for x in range(int(cx - rx) - 1, int(cx + rx) + 2) for y in range(int(cy - ry) - 1, int(cy + ry) + 2)
            if ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2 <= 1.0}


# --- instrumentos de viento ---------------------------------------------------------------------

# La flauta de madera colgando del puno por la boquilla (material 2), con los agujeros (el contorno duro)
FLAUTA = [
    ".9.",
    "979",
    "9@9",
    "979",
    "414",
    "424",
    "454",
    "424",
    "454",
    "424",
    "454",
    "424",
    "434",
    ".4.",
]


def _flauta_de_pan(n=5, largo=9):
    """n canas (material 1) de largo de mayor a menor, atadas por arriba con una tira de cuero (material 3),
    por la que se coge."""
    ancho = 2 * n + 1
    largos = [largo - i for i in range(n)]
    filas = ["d" * ancho, "dab" + "$" + "b" * (ancho - 6) + "cd", "d" * ancho]
    for r in range(largo + 1):
        f = []
        for x in range(ancho):
            if x % 2:
                ln = largos[(x - 1) // 2]
                f.append(("1" if r == 0 else "3" if r == ln - 1 else "2") if r < ln else "4" if r == ln else ".")
            else:
                vecinos = [largos[i] for i in ((x // 2) - 1, x // 2) if 0 <= i < n]
                f.append("4" if r <= max(vecinos) else ".")
        filas.append("".join(f))
    return filas


FLAUTA_PAN = _flauta_de_pan()

# La gaita: el soplete en el puno, el fuelle de tela (material 1), los tres roncones (material 2) con sus
# aros de laton (material 3) y el puntero colgando con sus agujeros
GAITA = [
    ".7...a.....",
    ".@...7.a...",
    ".7...7.7.a.",
    ".7...7.7.7.",
    "..7..7.7.7.",
    "..74444447.",
    "..41122224.",
    "..41222224.",
    ".4122222234",
    "..42222234.",
    "..42223334.",
    "..9794444..",
    "..9c9......",
    "..979......",
    "..9c9......",
    "..999......",
]

# La trompeta de laton (material 1) en horizontal: la boquilla a la izquierda, los pistones (material 3) por
# donde se coge y la campana abierta a la derecha
TROMPETA = [
    "...ddd...44.",
    "444dbd444114",
    "411dbd111124",
    "422d$d222234",
    "444dbd444234",
    "...ddd...44.",
]


# --- instrumentos de cuerda ---------------------------------------------------------------------

# El arpa de mano: la columna (por la que se coge), el cuello arriba, la caja a la derecha y las cuerdas
# (material 3) entre los dos
ARPA = [
    "444.......",
    "4144444...",
    "42411114..",
    "424a.a.44.",
    "424a.a.a14",
    "424a.a.a24",
    "4&4a.a.a24",
    "424a.a.a24",
    "424a.a.a34",
    "424a.a.a34",
    "424a.a.a34",
    "424a.a.a34",
    "4244444444",
    "4122222234",
    "4444444444",
]

# La lira: los dos brazos curvos, el travesano (material 3), las dos cuerdas y la caja, cogida por abajo
LIRA = [
    ".4.....4.",
    "414...414",
    "41ddddd14",
    "42dbbbd34",
    "42.a.a.34",
    "42.a.a.34",
    ".42a.a34.",
    ".44a.a44.",
    "412a2a234",
    "4122&2234",
    ".4222334.",
    "..44444..",
]

# El violin (en la izquierda): la voluta y el mastil (material 2) por donde se coge, la caja de madera
# (material 1) con el diapason y las efes (material 3)
VIOLIN = [
    "..999..",
    "..979..",
    "..979..",
    "..9@9..",
    "..979..",
    "..999..",
    "..494..",
    ".41924.",
    "4229234",
    ".42934.",
    ".4c9c4.",
    "4119224",
    "4229234",
    "4229334",
    ".44444.",
]
# El arco (en la derecha): la vara de madera (material 2) y las cerdas (material 3) a su lado
ARCO_VIOLIN = [
    "9b..",
    "@b..",
    "9b..",
    "7b..",
    "7b..",
    "7b..",
    "7b..",
    "7b..",
    "7b..",
    "7b..",
    "8d..",
]

def _pandereta():
    """La pandereta redonda: el parche (material 1) dentro del aro de madera (material 2), con las sonajas de
    laton (material 3) en el aro; se coge por el aro."""
    lienzo = {}
    for (x, y) in _disco(5, 5, 5.2):
        lienzo[(x, y)] = "9"
    for (x, y) in _disco(5, 5, 4.3):
        lienzo[(x, y)] = "7"
    _relleno(lienzo, _disco(5, 5, 3.3), 1)
    for (x, y) in ((5, 1), (9, 5), (5, 9), (1, 5), (2, 2), (8, 8)):
        lienzo[(x, y)] = "b"
    lienzo[(2, 2)] = "@"
    return _filas(lienzo)


PANDERETA = _pandereta()

def _vihuela():
    """La vihuela (de cintura, como una guitarra, para no confundirla con el laud): el clavijero (material 3)
    arriba, el mastil (material 2) por donde se coge y la caja de madera (material 1) con la boca y el
    puente."""
    lienzo = {}
    for y in range(0, 2):
        for x in (3, 4, 5):
            lienzo[(x, y)] = "d" if x != 4 else "b"
    for y in range(2, 8):
        lienzo[(3, y)], lienzo[(4, y)], lienzo[(5, y)] = "9", "7", "9"
    _relleno(lienzo, _ovalo(4.0, 9.5, 3.2, 2.4) | _ovalo(4.0, 13.5, 4.2, 3.0), 1)
    lienzo[(4, 7)] = "9"
    lienzo[(4, 10)] = "9"
    lienzo[(4, 11)] = "9"
    for x in (2, 3, 4, 5, 6):
        lienzo[(x, 14)] = "c"
    lienzo[(4, 4)] = "@"
    return _filas(lienzo)


VIHUELA = _vihuela()


# --- objetos de rol -----------------------------------------------------------------------------

def _con_palo(cabeza, largo, puno, ancho=7, centro=3):
    """La cabeza arriba y el palo (material 2) debajo, con el ancla a `puno` filas de la cabeza."""
    palo = [("." * (centro - 1) + "979" + "." * (ancho - centro - 2)) for _ in range(largo)]
    palo[puno] = "." * (centro - 1) + "9@9" + "." * (ancho - centro - 2)
    return cabeza + palo + ["." * (centro - 1) + "989" + "." * (ancho - centro - 2)]


# La escoba en alto, como quien la blande: el cepillo de paja (material 1) arriba, atado con cuerda
# (material 3), y el palo
ESCOBA = _con_palo([
    ".4.4.4.",
    "4141414",
    "4121214",
    "4122224",
    ".42234.",
    ".42334.",
    "..434..",
    "..dbd..",
    "..dbd..",
], 18, 12)

# La pala en alto (como la del Sepulturero): la hoja de hierro (material 1) y el mango
PALA = _con_palo([
    "4444444",
    "4112224",
    "4112224",
    "4122224",
    "4222234",
    "4222234",
    "4222334",
    ".42234.",
    "..444..",
], 19, 13)

# El pico de minero en alto: la cabeza de hierro curva y el mango
PICO = _con_palo([
    "..44444..",
    ".4112224.",
    "412444234",
    "44.....44",
    "4.......4",
], 8, 4, ancho=9, centro=4)
PICO[3] = "44.979.44"
PICO[4] = "4..979..4"

# La sarten de hierro negro en alto, cogida por el mango de madera
SARTEN = _con_palo([
    "..44444..",
    ".4111224.",
    "411222234",
    "412222234",
    "412222334",
    ".4223334.",
    "..44444..",
], 6, 3, ancho=9, centro=4)

# El estandarte en alto: la punta y el travesano de laton (material 3), el pano (material 1) con el emblema
# (material 4) y el borde en picos, y el asta larga, para que el pano quede por encima de la cabeza (de perfil,
# si no, la taparia y se inclinaria). Cada fila del pano es distinta: si hay que acortarlo para que quepa en
# la celda, se acorta el asta y no el pano.
ESTANDARTE = _con_palo([
    "...a...",
    "dbbbbbd",
    ".44444.",
    ".41124.",
    ".412i4.",
    ".41ih4.",
    ".422i4.",
    ".42234.",
    ".42334.",
    ".4.4.4.",
], 18, 14, ancho=7, centro=3)


def _cana():
    """La cana de pescar en alto (la vara de fresno, material 2) con el sedal (material 3) colgando de la punta,
    el corcho rojo (material 4) y el carrete de hierro (material 1) abajo."""
    filas = []
    for y in range(26):
        f = ["."] * 8
        x = 3 if y < 9 else 2
        f[x] = "8" if y % 5 == 4 else "7"
        if 0 < y < 9:
            f[x - 1] = "9"
        filas.append(f)
    filas[0][3], filas[0][4], filas[0][5] = "9", "a", "a"
    for y in range(1, 11):
        filas[y][6] = "a"
    filas[11][5], filas[11][6], filas[11][7] = "f", "g", "i"
    filas[12][6] = "h"
    filas[19][2] = "@"
    filas[22][1], filas[22][2], filas[22][3] = "4", "1", "4"
    filas[23][1], filas[23][2], filas[23][3] = "4", "2", "4"
    return ["".join(f) for f in filas]


CANA = _cana()

# La almohada de lino (material 1), pellizcada por una esquina, con la costura
ALMOHADA = [
    "&44444444",
    "411111224",
    "411222224",
    "412333234",
    "412222334",
    "422233334",
    "444444444",
]

# Tres shurikens de acero (material 1) en la mano, de cuatro puntas, con el agujero en medio
SHURIKENS = [
    "..4.......",
    ".414......",
    "4434.4....",
    ".424414...",
    "..4.4@44..",
    "..44414...",
    ".414.4.4..",
    "4434..414.",
    ".424.4434.",
    "..4...424.",
    ".......4..",
]

# El catalejo de laton (material 1): la funda de cuero (material 2) por donde se coge y la lente (material 3)
CATALEJO = [
    "9999.",
    "9679.",
    "9@79.",
    "9779.",
    "9789.",
    "4444.",
    "4124.",
    "4224.",
    "4224.",
    "4224.",
    "4234.",
    "44444",
    "41224",
    "42234",
    "4aa44",
]


def _malabares():
    """Tres bolas de malabares (materiales 4 arriba, 1 y 3 abajo) en la mano."""
    def bola(c):  # c: (contorno, luz, base, sombra)
        o, h, s, d = c
        return [f".{o}{o}{o}.", f"{o}{h}{h}{s}{o}", f"{o}{h}{s}{s}{o}", f"{o}{s}{s}{d}{o}", f".{o}{o}{o}."]
    lienzo = [["."] * 9 for _ in range(9)]
    for (x0, y0), c in (((0, 4), "4123"), ((4, 4), "dabc"), ((2, 0), "ifgh")):
        for dy, fila in enumerate(bola(c)):
            for dx, ch in enumerate(fila):
                if ch != ".":
                    lienzo[y0 + dy][x0 + dx] = ch
    lienzo[7][4] = "&"
    return ["".join(f) for f in lienzo]


MALABARES = _malabares()

# La jarra de cerveza en la derecha, cogida por el asa: la jarra de madera (material 1) con la espuma
# (material 2) rebosando por arriba (la de los PNJ, en la izquierda, la tapa el puno)
JARRA_CERVEZA = [
    ".66766..",
    "6677776.",
    "4444444.",
    "41122344",
    "411223&4",
    "41122344",
    "41223344",
    "4444444.",
]

# La cuerda de canamo enrollada (material 1), colgando del puno por la lazada
CUERDA = [
    "...4&4...",
    "..41124..",
    ".4144414.",
    "414...414",
    "413...424",
    "424...434",
    "413...424",
    ".4244424.",
    "..42334..",
    "...444...",
]


FORMAS = {
    "flauta": {"abajo": FLAUTA, "derecha": FLAUTA},
    "flauta_pan": {"abajo": FLAUTA_PAN, "derecha": FLAUTA_PAN},
    "gaita": {"abajo": GAITA, "derecha": GAITA},
    "trompeta": {"abajo": TROMPETA, "derecha": TROMPETA},
    "arpa": {"abajo": ARPA, "derecha": ARPA},
    "lira": {"abajo": LIRA, "derecha": LIRA},
    "violin": {"abajo": ARCO_VIOLIN, "derecha": ARCO_VIOLIN, "abajo_2": VIOLIN, "derecha_2": VIOLIN},
    "pandereta": {"abajo": PANDERETA, "derecha": PANDERETA},
    "vihuela": {"abajo": VIHUELA, "derecha": VIHUELA},
    "escoba": {"abajo": ESCOBA, "derecha": ESCOBA},
    "pala": {"abajo": PALA, "derecha": PALA},
    "cana": {"abajo": CANA, "derecha": CANA},
    "estandarte": {"abajo": ESTANDARTE, "derecha": ESTANDARTE},
    "pico": {"abajo": PICO, "derecha": PICO},
    "sarten": {"abajo": SARTEN, "derecha": SARTEN},
    "almohada": {"abajo": ALMOHADA, "derecha": ALMOHADA},
    "shurikens": {"abajo": SHURIKENS, "derecha": SHURIKENS},
    "catalejo": {"abajo": CATALEJO, "derecha": CATALEJO},
    "malabares": {"abajo": MALABARES, "derecha": MALABARES},
    "cuerda": {"abajo": CUERDA, "derecha": CUERDA},
    "jarra_cerveza": {"abajo": JARRA_CERVEZA, "derecha": JARRA_CERVEZA},
}
# id -> (forma, material 1, material 2, nombre[, material 3[, material 4]])
BASES = {
    "f_flauta": ("flauta", "fresno", "madera", "Flauta"),
    "f_flauta_pan": ("flauta_pan", "paja", "cuerda", "Flauta de pan", "cuero"),
    "f_gaita": ("gaita", "tela_roja_oscura", "madera_oscura", "Gaita", "laton"),
    "f_trompeta": ("trompeta", "laton", "laton", "Trompeta", "cuero"),
    "f_arpa": ("arpa", "roble", "madera_oscura", "Arpa", "oro_viejo"),
    "f_lira": ("lira", "madera", "madera_oscura", "Lira", "oro_viejo"),
    "f_violin": ("violin", "nogal_juglar", "madera_oscura", "Violin", "lino"),
    "f_pandereta": ("pandereta", "pergamino", "madera", "Pandereta", "laton"),
    "f_vihuela": ("vihuela", "nogal_juglar", "madera_oscura", "Vihuela", "marfil"),
    "f_escoba": ("escoba", "paja", "madera", "Escoba", "cuerda"),
    "f_pala": ("pala", "hierro", "madera", "Pala"),
    "f_cana": ("cana", "hierro", "fresno", "Cana de pescar", "lino", "gema_roja"),
    "f_estandarte": ("estandarte", "tela_roja_oscura", "madera", "Estandarte", "laton", "oro_viejo"),
    "f_pico": ("pico", "hierro", "madera", "Pico"),
    "f_sarten": ("sarten", "hierro_negro", "madera_oscura", "Sarten"),
    "f_almohada": ("almohada", "tela_blanca", "lino", "Almohada"),
    "f_shurikens": ("shurikens", "acero", "cuero_oscuro", "Shurikens"),
    "f_catalejo": ("catalejo", "laton", "cuero", "Catalejo", "vidrio"),
    "f_malabares": ("malabares", "tela_roja_oscura", "cuero", "Bolas de malabares", "tela_amarilla", "gema_azul"),
    "f_cuerda": ("cuerda", "cuerda", "cuerda", "Cuerda"),
    "f_jarra_cerveza": ("jarra_cerveza", "madera", "espuma", "Jarra de cerveza"),
}
PAREJA = {"violin"}
# como se cogen las que tienen pose (las demas, sin pose: a su tamano en la mano)
GRUPOS = {"escoba": "baston", "pala": "baston", "cana": "baston", "estandarte": "estandarte", "pico": "corto",
          "sarten": "corto"}


def _grupo_estandarte():
    """El estandarte se coge como un baston de frente y de espaldas, pero de perfil va recto (inclinado, el pano
    se aplastaba en una raya): el grupo «estandarte» de actores_poses."""
    baston = poses.GRUPOS["baston"]
    recto = poses._p("alzar", 1, (0.0, 0.0))
    recto_andando = poses._p("alzar", -1, (0.0, 0.0, 0.0))
    poses.GRUPOS.setdefault("estandarte", {
        "quieto": {"abajo": baston["quieto"]["abajo"], "derecha": recto},
        "andando": {"abajo": baston["andando"]["abajo"], "derecha": recto_andando},
    })
    poses.AGARRE.setdefault("estandarte", poses.AGARRE["baston"])
    poses.TAPA_LA_CABEZA.add("estandarte")


def _sumar():
    _grupo_estandarte()
    F.FORMAS.update(FORMAS)
    F.BASES.update(BASES)
    F.PAREJA.update(PAREJA)
    for familia, grupo in GRUPOS.items():
        poses.GRUPO_DE[familia] = grupo
        poses.ORIENTACION[familia] = "arriba"


_sumar()
