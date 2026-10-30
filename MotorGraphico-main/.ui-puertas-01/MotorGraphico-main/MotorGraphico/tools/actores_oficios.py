"""PNJ-01 — Prendas de oficio y piezas nuevas para vestir a los PNJ.

Todo PNJ se monta con las mismas piezas que puede llevar el jugador (raza,
piel, ojos, pelo, barba y ropa de diario), asi que cualquier jugador puede
copiar el aspecto de un PNJ corriente. Lo que los distingue es el equipo:
las armas, lo que llevan en la mano y, para algunos oficios, PRENDAS QUE
SOLO LLEVAN LOS PNJ (SOLO_PNJ): la librea de la guardia de Boundington, el
mandil del herrero, la casulla, la toga y el birrete, la librea de palacio,
la cesta, el libro de cuentas y el reloj de arena de Chronos. Van en los
mismos atlas que el resto, pero personajes.json las marca con "soloPnj": la
creacion de personaje no las ofrece y ningun objeto del catalogo las viste.

Las demas piezas de aqui (vestidos, faldas, chalecos, casacas, cofia,
toca, gorros, capas) son ropa de diario corriente: el jugador tambien las
puede elegir.

Se registran al importar este modulo en los catalogos de actores_equipo,
actores_cascos y actores_armas (actores_personajes lo importa).
"""
import actores_armas as armas
import actores_cascos as cascos
import actores_equipo as equipo
from actores_materiales import MATERIALES
from actores_pintura import Material as M
from actores_pintura import _tramo_torso, es_contorno, filas_torso, pintar, top_pie
from actores_siluetas import espejo_mitad as em
from actores_siluetas import plantilla

MATERIALES.update({m.nombre: m for m in [
    M("tela_granate", "#c45a64", "#94303e", "#6a1e2a", "#3a0e16", "#26080e"),
    M("tela_gris_clara", "#dcdce2", "#b0b0ba", "#86868f", "#45454c", "#2c2c32"),
    M("tela_ocre", "#f0c878", "#c89a48", "#946c2a", "#4e3812", "#34250b"),
    M("tela_verde_oscura", "#6a9a6a", "#44704a", "#2c4e32", "#16281a", "#0e1a10"),
    M("tela_lila", "#d0bce6", "#a08ac6", "#72609c", "#3c3056", "#281f3a"),
]})

SOLO_PNJ = set()  # ids de pieza que solo llevan los PNJ (personajes.json: "soloPnj")


# --- detalles de las prendas de oficio (extras de actores_pintura) ------------------

def mandil(material):
    """Mandil con peto: del pecho a media pierna, de frente y por el borde de
    delante de perfil (cuelga un pixel por delante del muslo); de espaldas,
    solo el lazo de la cintura."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        m = ctx["mat"][material]
        y0, _ = filas_torso(ctx)
        pies = [v for v in (top_pie(lab, "f"), top_pie(lab, "F")) if v is not None]
        if y0 is None or cint is None or not pies:
            return
        fondo = min(pies) - 2
        if d == "arriba":
            xs = [x for x in range(lab.shape[1]) if lab[cint, x] in ("t", "p")]
            if xs:
                cx = (min(xs) + max(xs)) // 2
                for x, t in ((cx - 1, "s"), (cx, "d"), (cx + 1, "s")):
                    pintar(ctx, cint, x, m, t)
                pintar(ctx, cint + 1, cx - 1, m, "d")
                pintar(ctx, cint + 1, cx + 1, m, "d")
            return
        for y in range(y0 + 2, fondo + 1):
            xs = [x for x in range(lab.shape[1]) if lab[y, x] in ("t", "p", "l", "L")]
            if not xs:
                continue
            x0, x1 = min(xs), max(xs)
            if d == "abajo":
                cx = (x0 + x1) / 2
                media = 2 if y < y0 + 4 else (3 if y <= cint else 4)
                a, b = int(round(cx - media)), int(round(cx + media))
            elif d == "derecha":
                a, b = x1 - 3, x1 + (1 if y > cint + 1 else 0)
            else:
                a, b = x0 - (1 if y > cint + 1 else 0), x0 + 3
            for x in range(a, b + 1):
                if y == y0 + 2 or y == fondo or x in (a, b):
                    t = "O"
                elif y == y0 + 3:
                    t = "H"
                elif y >= fondo - 1:
                    t = "d"
                else:
                    t = "s"
                pintar(ctx, y, x, m, t)
        if d == "abajo":  # la tira del cuello, del peto a los hombros
            tr = _tramo_torso(lab, ctx["tono"], y0 + 2)
            if tr:
                cx = (tr[0] + tr[1]) / 2
                for y in (y0, y0 + 1):
                    for x in (int(round(cx - 2)), int(round(cx + 2))):
                        pintar(ctx, y, x, m, "d")
    return fn


def estola(material):
    """Estola de la casulla: dos bandas del cuello al bajo (de frente), una de
    perfil por delante y una por la espalda."""
    def fn(ctx):
        d, px = ctx["dir"], ctx["px"]
        m = ctx["mat"][material]
        y0, _ = filas_torso(ctx)
        filas = [y for y in range(px.shape[0]) if px[y, :, 3].any()]
        if y0 is None or not filas:
            return
        fondo = max(filas) - 1
        for y in range(y0 + 1, fondo + 1):
            xs = [x for x in range(px.shape[1]) if px[y, x, 3]]
            if not xs:
                continue
            x0, x1 = min(xs), max(xs)
            cx = (x0 + x1) // 2
            columnas = {"abajo": (cx - 2, cx + 2), "arriba": (cx,), "derecha": (x1 - 2,),
                        "izquierda": (x0 + 2,)}[d]
            for x in columnas:
                if x0 < x < x1:
                    pintar(ctx, y, x, m, "d" if y == fondo else ("H" if y == y0 + 1 else "s"))
    return fn


def cuello(material):
    """Cuello blanco (valona) en lo alto del pecho."""
    def fn(ctx):
        lab, tono, d = ctx["lab"], ctx["tono"], ctx["dir"]
        m = ctx["mat"][material]
        y0, _ = filas_torso(ctx)
        if y0 is None:
            return
        for k, y in enumerate((y0, y0 + 1)):
            tr = _tramo_torso(lab, tono, y)
            if not tr:
                continue
            cx = (tr[0] + tr[1]) / 2
            if d == "abajo":
                media = 3 if k == 0 else 2
                xs = range(int(round(cx - media)), int(round(cx + media)) + 1)
            elif d == "arriba":
                xs = range(tr[0], tr[1] + 1) if k == 0 else ()
            elif d == "derecha":
                xs = range(tr[1] - 3 + k, tr[1] + 1)
            else:
                xs = range(tr[0], tr[0] + 4 - k)
            for x in xs:
                if lab[y, x] == "t" and not es_contorno(tono[y, x]):
                    pintar(ctx, y, x, m, "H" if k == 0 else "s")
    return fn


def botonadura(material):
    """Botones dorados en el centro del pecho (de frente) o en el borde (de perfil)."""
    def fn(ctx):
        lab, tono, d, cint = ctx["lab"], ctx["tono"], ctx["dir"], ctx["cint"]
        m = ctx["mat"][material]
        y0, _ = filas_torso(ctx)
        if y0 is None or cint is None or d == "arriba":
            return
        for y in range(y0 + 3, cint, 2):
            tr = _tramo_torso(lab, tono, y)
            if not tr:
                continue
            x = {"abajo": (tr[0] + tr[1]) // 2, "derecha": tr[1] - 1, "izquierda": tr[0] + 1}[d]
            pintar(ctx, y, x, m, "H")
    return fn


def vivos(material):
    """Ribete de la casaca: la abertura de delante y el bajo."""
    def fn(ctx):
        d, px = ctx["dir"], ctx["px"]
        m = ctx["mat"][material]
        y0, _ = filas_torso(ctx)
        filas = [y for y in range(px.shape[0]) if px[y, :, 3].any()]
        if y0 is None or not filas:
            return
        fondo = max(filas)
        for y in range(y0 + 1, fondo + 1):
            xs = [x for x in range(px.shape[1]) if px[y, x, 3]]
            if not xs:
                continue
            x0, x1 = min(xs), max(xs)
            if y == fondo:
                for x in range(x0 + 1, x1):
                    pintar(ctx, y, x, m, "s")
                continue
            x = {"abajo": (x0 + x1) // 2, "arriba": None, "derecha": x1 - 1, "izquierda": x0 + 1}[d]
            if x is not None:
                pintar(ctx, y, x, m, "H" if y < y0 + 3 else "s")
    return fn


# --- constructores nuevos --------------------------------------------------------------

def casaca(tela, vivo, hebilla="plata"):
    """Casaca larga abierta con ribete (la de Venides)."""
    return {"partes": {"t": tela, "a": tela, "A": tela}, "bajo": (tela, 6),
            "cinturon": ("cuero_oscuro", hebilla, 0), "extras": [vivos(vivo)]}


def mandil_oficio(camisa, cuero):
    """Camisa de manga corta con mandil de cuero (herrero)."""
    return {"partes": {"t": camisa, "a": camisa, "A": camisa}, "mangas": "cortas",
            "bajo": (camisa, 1), "cinturon": ("cuero_oscuro", "hierro", 0), "extras": [mandil(cuero)]}


def casulla(tela, oro):
    return dict(equipo.tunica_larga(tela, oro, 0), extras=[estola(oro)])


def toga(tela, cuello_mat):
    return dict(equipo.tunica_larga(tela, tela, 0), extras=[cuello(cuello_mat)])


def librea(tela, boton, cuello_mat=None):
    extras = [botonadura(boton)] + ([cuello(cuello_mat)] if cuello_mat else [])
    return {"partes": {"t": tela, "a": tela, "A": tela}, "bajo": (tela, 3),
            "cinturon": ("cuero_oscuro", boton, 0), "extras": extras}


def _anade(catalogo, ident, valor, solo_pnj=False):
    assert ident not in catalogo, f"pieza repetida: {ident}"
    catalogo[ident] = valor
    if solo_pnj:
        SOLO_PNJ.add(ident)


# ropa de diario nueva (tambien para el jugador)
for ident, valor in {
    "vestido_azul": (equipo.vestido, ("tunica_azul",), "Vestido azul"),
    "vestido_gris": (equipo.vestido, ("tela_gris_clara", "lino"), "Vestido gris"),
    "vestido_negro": (equipo.vestido, ("tela_negra", "tela_gris_clara"), "Vestido negro"),
    "vestido_purpura": (equipo.vestido, ("lana_purpura", "oro_emblema"), "Vestido purpura"),
    "vestido_ocre": (equipo.vestido, ("tela_ocre", "tunica_parda"), "Vestido ocre"),
    "vestido_lila": (equipo.vestido, ("tela_lila", "lino"), "Vestido lila"),
    "vestido_rojo_delantal": (equipo.vestido, ("tunica_roja", "lino", "lino"), "Vestido rojo con delantal"),
    "vestido_azul_delantal": (equipo.vestido, ("tunica_azul", "lino", "lino"), "Vestido azul con delantal"),
    "vestido_verde_delantal": (equipo.vestido, ("tela_verde_oscura", "lino", "lino"),
                               "Vestido verde con delantal"),
    "camisa_chaleco_azul": (equipo.camisa_chaleco, ("lino", "tabardo_azul"), "Camisa y chaleco azul"),
    "camisa_chaleco_verde": (equipo.camisa_chaleco, ("lino", "tela_verde_oscura"), "Camisa y chaleco verde"),
    "camisa_chaleco_granate": (equipo.camisa_chaleco, ("lino", "tela_granate"), "Camisa y chaleco granate"),
    "camisa_sin_mangas": (equipo.tunica, ("lino", "cuero_oscuro", "hierro", "sin"), "Camisa blanca sin mangas"),
    "tunica_gris": (equipo.tunica, ("lana_gris",), "Tunica gris"),
    "tunica_ocre": (equipo.tunica, ("tela_ocre",), "Tunica ocre"),
    "casaca_azul": (casaca, ("tela_azul_noche", "tela_blanca"), "Casaca azul ribeteada"),
    "casaca_negra": (casaca, ("tela_negra", "plata"), "Casaca negra"),
    "casaca_granate": (casaca, ("tela_granate", "oro_emblema", "oro_emblema"), "Casaca granate"),
}.items():
    _anade(equipo.TORSO, ident, valor)

for ident, valor in {
    "falda_verde": (equipo.falda, ("tunica_verde",), "Falda verde"),
    "falda_gris": (equipo.falda, ("tela_gris_clara",), "Falda gris"),
    "falda_negra": (equipo.falda, ("tela_negra",), "Falda negra"),
    "falda_larga_parda": (equipo.falda, ("tunica_parda", 1.0), "Falda larga parda"),
    "pantalon_ocre": (equipo.pantalon, ("tela_ocre",), "Pantalon ocre"),
}.items():
    _anade(equipo.PIERNAS, ident, valor)

for ident, valor in {
    "capa_gris": ("lana_gris", "larga", "Capa gris"),
    "capa_granate": ("tela_granate", "larga", "Capa granate"),
    "capa_corta_parda": ("tunica_parda", "corta", "Capa corta parda"),
}.items():
    _anade(equipo.CAPAS, ident, valor)

# prendas de oficio (solo PNJ)
for ident, valor in {
    "librea_guardia": (equipo.sobrevesta, ("tela_granate",), "Librea de la guardia de Boundington"),
    "mandil_herrero": (mandil_oficio, ("lino", "cuero"), "Mandil de herrero"),
    "casulla": (casulla, ("tela_blanca", "oro_emblema"), "Casulla"),
    "toga": (toga, ("tela_negra", "tela_blanca"), "Toga academica"),
    "librea_palacio": (librea, ("tela_granate", "oro_emblema", "lino"), "Librea de palacio"),
}.items():
    _anade(equipo.TORSO, ident, valor, solo_pnj=True)


# --- cabeza ---------------------------------------------------------------------------
# Plantillas como las de actores_cascos: x = 22 + indice, filas del frame quieto.

COFIA = plantilla({  # gorro de lino fruncido; el volante sobre la frente
    "abajo": (17, em([
        "........###",    # 17
        "......#####",    # 18
        ".....######",    # 19
        "....#######",    # 20
        "...########",    # 21
        "...########",    # 22
        "..#########",    # 23
        "..+.+.+.+.+",    # 24 volante
    ])),
    "arriba": (17, em([
        "........###",
        "......#####",
        ".....######",
        "....#######",
        "...########",
        "...########",
        "..#########",
        "..#########",
        "..#########",
        "...+.+.+.+.",
    ])),
    "derecha": (17, [
        "........####",           # 17
        "......#######",          # 18
        ".....#########",         # 19
        "....###########",        # 20
        "...############",        # 21
        "..#############",        # 22
        "..##############",       # 23
        "..###.+.+.+.+.+",        # 24 volante hacia la cara
        "..###",                  # 25 la cofia cubre el cogote
        "...#+",                  # 26
    ]),
})

TOCA = plantilla({  # toca de lino: tapa la cabeza, enmarca la cara y cae sobre los hombros
    "abajo": (18, em([
        ".......####",    # 18
        ".....######",    # 19
        "....#######",    # 20
        "...########",    # 21
        "..#########",    # 22
        "..#########",    # 23
        "..###++++++",    # 24 borde sobre la frente
        "..###......",    # 25
        "..###......",    # 26
        "..###......",    # 27
        "..###......",    # 28
        "..###......",    # 29
        "..###......",    # 30
        "..###......",    # 31
        "..###......",    # 32
        "..####.....",    # 33
        ".######....",    # 34
        ".##########",    # 35 cae sobre el cuello
        ".##########",    # 36
    ])),
    "arriba": (18, em([
        ".......####",
        ".....######",
        "....#######",
        "...########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        ".##########",
        ".##########",
        ".##########",
    ])),
    "derecha": (18, [
        ".......#####",           # 18
        ".....########",          # 19
        "....##########",         # 20
        "...###########",         # 21
        "..############",         # 22
        "..#############",        # 23
        "..#########++++",        # 24 borde
        "..########",             # 25
        "..#######",              # 26
        "..#######",              # 27
        "..#######",              # 28
        "..######",               # 29
        "..######",               # 30
        "..######",               # 31
        "..#######",              # 32
        "..########",             # 33
        "..##########",           # 34
        "..############",         # 35
        "...###########",         # 36
    ]),
})

BIRRETE = plantilla({  # birrete academico: tablero plano, casquete y borla dorada
    "abajo": (16, [
        "....###############",      # 16 tablero
        "...#################",     # 17
        "....###############+",     # 18
        "......###########..+",     # 19 casquete
        ".....#############.+",     # 20
        ".....#############.+",     # 21
        ".....#############",       # 22
        "......###########",        # 23
    ]),
    "arriba": (16, [
        "....###############",
        "...#################",
        "+...###############",
        "+.....###########",
        "+....#############",
        "+....#############",
        ".....#############",
        "......###########",
    ]),
    "derecha": (16, [
        "...#############",         # 16
        "..###############",        # 17
        "...#############",         # 18
        "...+.##########",          # 19 borla detras
        "...+.###########",         # 20
        "...+.###########",         # 21
        ".....###########",         # 22
        "......#########",          # 23
    ]),
})

GORRO = plantilla({  # gorro de lana con vuelta
    "abajo": (17, em([
        ".........##",    # 17
        ".......####",    # 18
        "......#####",    # 19
        ".....######",    # 20
        "....#######",    # 21
        "...########",    # 22
        "...++++++++",    # 23 vuelta
        "...++++++++",    # 24
    ])),
    "arriba": (17, em([
        ".........##",
        ".......####",
        "......#####",
        ".....######",
        "....#######",
        "...########",
        "...++++++++",
        "...++++++++",
    ])),
    "derecha": (17, [
        "........##",              # 17
        "......#####",             # 18
        ".....#######",            # 19
        "....#########",           # 20
        "...###########",          # 21
        "...############",         # 22
        "...++++++++++++",         # 23
        "...++++++++++++",         # 24
    ]),
})

for ident, (plant, m1, m2, nombre, solo) in {
    "cofia": (COFIA, "lino", "tela_blanca", "Cofia", False),
    "toca": (TOCA, "tela_blanca", "lino", "Toca", False),
    "gorro_rojo": (GORRO, "tela_roja_oscura", "tunica_roja", "Gorro de lana rojo", False),
    "gorro_gris": (GORRO, "lana_gris", "pantalon_gris", "Gorro de lana gris", False),
    "birrete": (BIRRETE, "tela_negra", "oro_emblema", "Birrete", True),
}.items():
    assert ident not in cascos.CABEZA, f"pieza repetida: {ident}"
    cascos.CABEZA[ident] = {"plantillas": [(plant, None)], "m1": m1, "m2": m2, "nombre": nombre,
                            "oculta_pelo": False}
    if solo:
        SOLO_PNJ.add(ident)


# --- en la mano -------------------------------------------------------------------------

CESTA = {  # cesta de mimbre colgando del brazo, con fruta
    "abajo": [
        "...@...",
        "..9.9..",
        ".9...9.",
        "4444444",
        "4w1w1w4",
        "4121214",
        "4232324",
        "4222224",
        ".44444.",
    ],
}
CESTA["derecha"] = CESTA["abajo"]
armas.FORMAS["cesta"] = CESTA
armas.MANO_IZQUIERDA = armas.MANO_IZQUIERDA + ("cesta",)

# Chronos disfrazado de mortal (Oriol, 05/10/2026) lleva un reloj de arena colgando de
# la mano: la arena (material 1) arriba, cayendo y abajo, entre las tablas y los postes
# de bronce (material 2), que destacan sobre la ropa gris. Es su unica pista.
RELOJ_ARENA = {
    "abajo": [
        "..@..",
        "97779",
        "8w1w8",
        "8.2.8",
        "8.3.8",
        "8.1.8",
        "81228",
        "97779",
    ],
}
RELOJ_ARENA["derecha"] = RELOJ_ARENA["abajo"]
armas.FORMAS["reloj_arena"] = RELOJ_ARENA
armas.MANO_IZQUIERDA = armas.MANO_IZQUIERDA + ("reloj_arena",)

for ident, valor in {
    "libro_cuentas": ("grimorio", "cuero", "laton", "Libro de cuentas"),
    "cesta": ("cesta", "paja", "madera", "Cesta"),
    "reloj_arena": ("reloj_arena", "paja", "bronce", "Reloj de arena"),
}.items():
    assert ident not in armas.ARMAS, f"pieza repetida: {ident}"
    armas.ARMAS[ident] = valor
    SOLO_PNJ.add(ident)
