"""ESTETICA-01 — Las formas de las armas y de lo que se lleva en la mano.

Oriol (06/10/2026): «para cada arma y armadura necesitaremos vincularlo a un
sprite, hay algunas que pueden compartir sprites, pero todas tienen que
aplicar una estetica»: forma + esencia + acabado, y dibujo propio para las
miticas y las unicas.

Aqui esta la FORMA. Cada forma se dibuja una vez, en el formato de
actores_armas (digitos y letras por tono, '@' el puno), con tres materiales
distintos para que el objeto la vista con su paleta (actores_estetica):

    material 1 (1-5, w)  lo principal: la hoja, la cabeza, la cara del escudo,
                         la madera del arco, el cristal: lo que cambia la ESENCIA
    material 2 (6-9, @)  el mango, el astil, la cuerda
    material 3 (a-e)     la guarnicion: guarda, pomo, virolas, remaches, cerco:
                         lo que cambia el ACABADO (laton, plata, oro)

Las piezas base de cada forma llevan materiales «de origen» fijos (hierro,
cuero oscuro, laton...) que el tinte cambia por los del objeto: la daga de
plaga es la fila "daga" con hierro>plaga (ManifiestoPersonajes::paletas).

Las formas viejas (espada, daga, hacha...) son las de actores_armas, con la
guarnicion marcada con letras: con material 3 = material 2 salen igual que
antes, asi que las piezas de siempre (espada_hierro, daga_plata...) no cambian.
"""
import actores_armas as armas
import actores_poses as poses

# --- hojas ------------------------------------------------------------------------------

CUCHILLO = {  # cuchillo de cazador: hoja ancha con el lomo recortado, guarda corta
    "abajo": [
        "..da..",
        "..97..",
        "..@7..",
        "..97..",
        "..97..",
        ".dabd.",
        ".4123.",
        ".4123.",
        ".4123.",
        ".4123.",
        "..413.",
        "..43..",
        "..4...",
    ],
    "derecha": [
        ".da....",
        ".97....",
        ".@7....",
        ".97....",
        ".97....",
        "dabd...",
        ".41234.",
        "..41234",
        "..4123.",
        "...423.",
        "....4..",
    ],
}

ESTILETE = {  # hoja de aguja y gavilanes largos
    "abajo": [
        "..da..",
        "..97..",
        "..@7..",
        "..97..",
        "dabbad",
        "..41..",
        "..41..",
        "..41..",
        "..41..",
        "..41..",
        "..41..",
        "..4...",
    ],
    "derecha": [
        ".da...",
        ".97...",
        ".@7...",
        ".97...",
        "dabad.",
        ".41...",
        "..41..",
        "..41..",
        "...41.",
        "...41.",
        "....4.",
    ],
}

SABLE = {  # hoja larga y estrecha que se curva hacia delante; arco de guarda
    "abajo": [
        ".dda..",
        ".d97..",
        ".d@7..",
        ".d97..",
        ".d97..",
        ".daaad",
        "..413.",
        "..413.",
        "..413.",
        "..413.",
        "..413.",
        "..413.",
        "..413.",
        "..43..",
        "..4...",
    ],
    "derecha": [
        "da.....",
        "97d....",
        ".97d...",
        ".@7d...",
        ".97d...",
        "daaad..",
        ".d413..",
        "..413..",
        "..413..",
        "...413.",
        "...413.",
        "....413",
        ".....43",
        "......4",
    ],
}

ESPADA_CORTA = {  # hoja ancha y corta en hoja de laurel, guarda recta
    "abajo": [
        "..da..",
        "..97..",
        "..@7..",
        "..97..",
        "daaaad",
        ".4123.",
        ".4123.",
        ".4123.",
        ".4123.",
        ".4123.",
        "..423.",
        "..43..",
        "..4...",
    ],
    "derecha": [
        "da....",
        "97....",
        ".97...",
        ".@7...",
        "daaad.",
        ".d4123",
        "..4123",
        "..4123",
        "...423",
        "...43.",
        "....4.",
    ],
}

MANDOBLE = {  # espadon a dos manos: puno largo, gavilanes anchos, hoja hasta el suelo
    "abajo": [
        "..da..",
        "..97..",
        "..97..",
        "..97..",
        "..@7..",
        "..97..",
        ".dbbd.",
        "daaaad",
        ".4123.",
        ".4123.",
        ".4123.",
        ".4123.",
        ".4123.",
        ".4123.",
        ".4123.",
        ".4123.",
        ".4123.",
        "..423.",
        "..43..",
        "..4...",
    ],
    "derecha": [
        "da.....",
        "97.....",
        "97.....",
        ".97....",
        ".@7....",
        ".97....",
        "daaad..",
        ".d4123.",
        "..4123.",
        "..4123.",
        "...4123",
        "...4123",
        "...4123",
        "....423",
        "....423",
        ".....43",
        ".....4.",
    ],
}

ESTOQUE = {  # hoja larguisima de dos pixeles y cazoleta de taza
    "abajo": [
        "..da..",
        "..97..",
        "..@7..",
        "..97..",
        ".daad.",
        "dabbad",
        ".dccd.",
        "..41..",
        "..41..",
        "..41..",
        "..41..",
        "..41..",
        "..41..",
        "..41..",
        "..41..",
        "..4...",
    ],
    "derecha": [
        ".da...",
        ".97...",
        ".@7...",
        ".97...",
        "dabd..",
        "dbcd..",
        ".d41..",
        "..41..",
        "..41..",
        "...41.",
        "...41.",
        "...41.",
        "....41",
        "....4.",
    ],
}

# --- hachas, mazas y martillos (cabeza arriba, junto al hombro) ----------------------------

HACHA_MANO = {  # hacha de mano: cabeza pequena con su virola, mango corto
    "abajo": [
        ".44...",
        "4124d.",
        "4122b.",
        "4123d.",
        ".44.79",
        "....79",
        "....79",
        "....79",
        "....@9",
        "....79",
        "....cd",
    ],
    "derecha": [
        "...44.",
        ".d4214",
        ".b2214",
        ".d3214",
        "97.44.",
        "97....",
        "97....",
        "97....",
        "9@....",
        "97....",
        "dc....",
    ],
}

GRAN_HACHA = {  # hacha de dos filos en un astil largo
    "abajo": [
        ".44...44.",
        "4124d4214",
        "4122b2214",
        "4122b2214",
        "4123d3234",
        ".44.79.44",
        "....79...",
        "....79...",
        "....79...",
        "....79...",
        "....79...",
        "....79...",
        "....@9...",
        "....79...",
        "....79...",
        "....79...",
        "....cd...",
    ],
    "derecha": [
        "..444..",
        ".41224.",
        "412b214",
        "412b214",
        ".42d34.",
        "..444..",
        "...79..",
        "...79..",
        "...79..",
        "...79..",
        "...79..",
        "...79..",
        "...@9..",
        "...79..",
        "...79..",
        "...79..",
        "...cd..",
    ],
}

MACANA = {  # porra de madera dura con tachones de hierro
    "abajo": [
        "..444..",
        ".41a24.",
        ".4122a4",
        ".4a224.",
        ".4122a4",
        "..4a34.",
        "..4234.",
        "...79..",
        "...79..",
        "...@9..",
        "...79..",
        "...89..",
    ],
}
MACANA["derecha"] = MACANA["abajo"]

# --- astiles ------------------------------------------------------------------------------

PICA = {  # pica de guardia: punta pequena en un astil larguisimo
    "abajo": [
        "..4...",
        ".414..",
        ".4134.",
        "..43..",
        "..dd..",
    ] + ["..79.."] * 18 + ["..@9.."] + ["..79.."] * 6 + ["..cd.."],
}
PICA["derecha"] = PICA["abajo"]

JABALINAS = {  # una jabalina (lleva mas a la espalda): punta fina y astil corto
    "abajo": [
        "..4..",
        ".414.",
        ".434.",
        "..dd.",
    ] + ["..79."] * 13 + ["..@9.", "..79.", "..79.", "..cd."],
}
JABALINAS["derecha"] = JABALINAS["abajo"]

GUADANA = {  # guadana: la hoja curva arriba, hacia fuera, y el astil
    "abajo": [
        "44444...",
        "211224..",
        ".2223344",
        "....d.44",
        "....79..",
    ] + ["....79.."] * 14 + ["....@9.."] + ["....79.."] * 5 + ["....cd.."],
    "derecha": [  # de perfil la hoja va hacia atras, por detras de la cabeza
        "44444...",
        "211224..",
        ".2223344",
        "....d.44",
        "....79..",
    ] + ["....79.."] * 14 + ["....@9.."] + ["....79.."] * 5 + ["....cd.."],
}

# --- a distancia ----------------------------------------------------------------------------

BALLESTA = {  # ballesta: el arco de acero (material 3) cruzado abajo, la cuerda y la caja
    "abajo": [
        "...44...",
        "...41...",
        "...41...",
        "...@1...",
        "...41...",
        "...49...",
        "..9419..",
        ".9.41.9.",
        "dcbaabcd",
        "...44...",
    ],
    "derecha": [
        "..d...",
        "..c...",
        "..b4..",
        "..a14.",
        ".9a12.",
        "9.a@2.",
        "..b12.",
        "..c42.",
        "..d.4.",
    ],
}

HONDA = {  # honda: dos cuerdas que cuelgan del puno y la bolsa de cuero
    "abajo": [
        "..@...",
        "..79..",
        ".7..9.",
        ".7..9.",
        ".7..9.",
        "..44..",
        ".4124.",
        ".4234.",
        "..44..",
    ],
}
HONDA["derecha"] = HONDA["abajo"]

LATIGO = {  # latigo enrollado colgando del puno
    "abajo": [
        "..da..",
        "..97..",
        "..@7..",
        "..dc..",
        ".4444.",
        "41..14",
        "4.44.4",
        "41..14",
        ".4444.",
        "....4.",
        "...4..",
    ],
}
LATIGO["derecha"] = LATIGO["abajo"]

# --- bastones, varas y cetros --------------------------------------------------------------

VARA = {  # vara de aprendiz: corta, con un cristal en la punta
    "abajo": [
        "..4...",
        ".4w4..",
        ".424..",
        "..d...",
        "..7...",
        "..7...",
        "..@...",
        "..7...",
        "..7...",
        "..c...",
    ],
    "derecha": [
        "..4...",
        ".4w4..",
        ".424..",
        "..d...",
        "..7...",
        "..7...",
        "..@...",
        "..7...",
        "..7...",
        "..c...",
    ],
}

CETRO = {  # cetro: cabeza de aletas (material 1) sobre virolas, mango corto
    "abajo": [
        "..4...",
        ".414..",
        "41234.",
        ".424..",
        ".dad..",
        "..79..",
        "..79..",
        "..@9..",
        "..79..",
        "..ad..",
        "..cd..",
    ],
}
CETRO["derecha"] = CETRO["abajo"]

BASTON_HERRADO = {  # baston de caminante con contera y virolas de metal
    "abajo": [
        "..dd..",
        "..ab..",
        "..79..",
        "..dd..",
    ] + ["..79..", "..79..", "..89.."] * 4 + ["..79..", "..@9..", "..79..", "..89..",
                                                "..dd..", "..79..", "..89..", "..79..",
                                                "..cd.."],
}
BASTON_HERRADO["derecha"] = BASTON_HERRADO["abajo"]

# --- escudos y lo que va en la mano izquierda -----------------------------------------------

ESCUDO_COMETA = {  # escudo de cometa: cara (1), cruz pintada (2) y cerco de metal (3)
    "abajo": [
        "ddddddddd",
        "daa171acd",
        "da11712cd",
        "da77@77cd",
        "da12722cd",
        "da12723cd",
        ".da273cd.",
        ".da273cd.",
        "..da7cd..",
        "..dacd...",
        "...dd....",
    ],
    "derecha": [
        "ddd..",
        "da1d.",
        "da2cd",
        "d7@cd",
        "da2cd",
        "da3cd",
        "dacd.",
        "dcd..",
        "dd...",
    ],
}

RODELA = {  # rodela: escudo redondo, umbo en el centro y cerco
    "abajo": [
        "..ddddd..",
        ".dabbbcd.",
        "da11122cd",
        "da12a23cd",
        "da1a$c3cd",
        "da22c33cd",
        "dc22333cd",
        ".dcccccd.",
        "..ddddd..",
    ],
    "derecha": [
        ".ddd.",
        "da1cd",
        "da2cd",
        "db$cd",
        "da2cd",
        "dc3cd",
        ".ddd.",
    ],
}

BROQUEL = {  # broquel: pequeno, casi todo umbo
    "abajo": [
        ".ddddd.",
        "da112cd",
        "d1aab3d",
        "d1a$c3d",
        "d1bcc3d",
        "dc233cd",
        ".ddddd.",
    ],
    "derecha": [
        "ddd.",
        "d1cd",
        "d$cd",
        "d3cd",
        "ddd.",
    ],
}

PAVES = {  # paves: escudo alto y recto que tapa medio cuerpo
    "abajo": [
        "ddddddddd",
        "da11111cd",
        "da12222cd",
        "da16662cd",
        "da12622cd",
        "da12@22cd",
        "da12622cd",
        "da12622cd",
        "da12222cd",
        "da12223cd",
        "da12233cd",
        "da12333cd",
        "dc33333cd",
        "ddddddddd",
    ],
    "derecha": [
        "ddd.",
        "da1d",
        "da2d",
        "da2d",
        "d6@d",
        "da2d",
        "da2d",
        "da2d",
        "da3d",
        "da3d",
        "dc3d",
        "ddd.",
    ],
}

BRAZAL = {  # brazal de choque: placa sobre el antebrazo, con remaches; el puno debajo
    "abajo": [
        ".444.",
        "41a14",
        "41224",
        "4a234",
        "41234",
        ".444.",
        "..@..",
    ],
    "derecha": [
        "444.",
        "4a14",
        "4224",
        "4a34",
        "444.",
        ".@..",
    ],
}

SIMBOLO = {  # simbolo sagrado: un sol de metal colgando de un cordon
    "abajo": [
        "..@..",
        "..9..",
        "..9..",
        "4.4.4",
        ".414.",
        "41w14",
        ".424.",
        "4.4.4",
    ],
}
SIMBOLO["derecha"] = SIMBOLO["abajo"]

TALISMAN = {  # talisman: una piedra en su engaste, colgando del puno
    "abajo": [
        "..@..",
        "..9..",
        "..9..",
        ".dad.",
        "d4w4d",
        "d424d",
        ".d4d.",
    ],
}
TALISMAN["derecha"] = TALISMAN["abajo"]

KIT = {  # kit de herramientas: la bolsa de cuero y los mangos de las herramientas
    "abajo": [
        "..@...",
        ".d97c.",
        ".d.9c.",
        "444444",
        "411224",
        "41a234",
        "422334",
        "444444",
    ],
    "derecha": [
        ".@...",
        "d97c.",
        "44444",
        "41124",
        "41a34",
        "42334",
        "44444",
    ],
}

TEA = {  # tea: la llama (material 1), el palo y su atadura
    "abajo": [
        "..1...",
        ".121..",
        ".1w21.",
        ".2232.",
        "..ab..",
        "..cd..",
        "..79..",
        "..@9..",
        "..79..",
        "..89..",
    ],
}
TEA["derecha"] = TEA["abajo"]

ESPEJO = {  # espejo de mano: el cristal (1), el marco (3) y el mango (2)
    "abajo": [
        ".ddd.",
        "d1w1d",
        "d121d",
        "d223d",
        ".ddd.",
        "..b..",
        "..7..",
        "..@..",
        "..7..",
        "..c..",
    ],
}
ESPEJO["derecha"] = ESPEJO["abajo"]

FORMAS = {
    "cuchillo": CUCHILLO, "estilete": ESTILETE, "sable": SABLE, "espada_corta": ESPADA_CORTA,
    "mandoble": MANDOBLE, "estoque": ESTOQUE,
    "hacha_mano": HACHA_MANO, "gran_hacha": GRAN_HACHA, "macana": MACANA,
    "pica": PICA, "jabalinas": JABALINAS, "guadana": GUADANA,
    "ballesta": BALLESTA, "honda": HONDA, "latigo": LATIGO,
    "vara": VARA, "cetro": CETRO, "baston_herrado": BASTON_HERRADO,
    "escudo_cometa": ESCUDO_COMETA, "rodela": RODELA, "broquel": BROQUEL, "paves": PAVES,
    "brazal": BRAZAL, "simbolo": SIMBOLO, "talisman": TALISMAN, "kit": KIT,
    "tea": TEA, "espejo": ESPEJO,
}
MANO_IZQUIERDA = ("escudo_cometa", "rodela", "broquel", "paves", "brazal", "simbolo", "talisman",
                  "kit", "espejo")
TAPAN_EL_PUNO = ("escudo_cometa", "rodela", "broquel", "paves")  # se llevan embrazados

# Las piezas base: una fila por forma, con materiales de origen distintos en cada canal.
# id -> (familia, material 1, material 2, nombre, material 3)
HOJA, MANGO, CUERDA, MADERA, GUARNICION = "hierro", "cuero_oscuro", "cuerda", "roble", "laton"
BASES = {
    # hojas: la esencia cambia el hierro, el acabado el laton
    "f_daga": ("daga", HOJA, MANGO, "Daga", GUARNICION),
    "f_cuchillo": ("cuchillo", HOJA, MANGO, "Cuchillo de cazador", GUARNICION),
    "f_estilete": ("estilete", HOJA, MANGO, "Estilete", GUARNICION),
    "f_espada": ("espada", HOJA, MANGO, "Espada", GUARNICION),
    "f_espada_corta": ("espada_corta", HOJA, MANGO, "Espada corta", GUARNICION),
    "f_sable": ("sable", HOJA, MANGO, "Sable", GUARNICION),
    "f_mandoble": ("mandoble", HOJA, MANGO, "Mandoble", GUARNICION),
    "f_estoque": ("estoque", HOJA, MANGO, "Estoque", GUARNICION),
    # cabezas sobre madera
    "f_hacha": ("hacha", HOJA, MADERA, "Hacha", GUARNICION),
    "f_hacha_mano": ("hacha_mano", HOJA, MADERA, "Hacha de mano", GUARNICION),
    "f_gran_hacha": ("gran_hacha", HOJA, MADERA, "Gran hacha", GUARNICION),
    "f_maza": ("maza", HOJA, MADERA, "Maza", GUARNICION),
    "f_martillo": ("martillo", HOJA, MADERA, "Martillo de guerra", GUARNICION),
    "f_macana": ("macana", "madera_oscura", MANGO, "Macana tachonada", HOJA),
    "f_lanza": ("lanza", HOJA, MADERA, "Lanza", GUARNICION),
    "f_pica": ("pica", HOJA, MADERA, "Pica", GUARNICION),
    "f_jabalinas": ("jabalinas", HOJA, MADERA, "Jabalinas", GUARNICION),
    "f_tridente": ("tridente", HOJA, MADERA, "Tridente", GUARNICION),
    "f_guadana": ("guadana", HOJA, MADERA, "Guadana", GUARNICION),
    # a distancia: la madera es lo principal
    "f_arco": ("arco", MADERA, CUERDA, "Arco", GUARNICION),
    "f_ballesta": ("ballesta", MADERA, CUERDA, "Ballesta", HOJA),
    "f_honda": ("honda", "cuero", CUERDA, "Honda", GUARNICION),
    "f_latigo": ("latigo", "cuero", MANGO, "Latigo", GUARNICION),
    # bastones: el cristal o la cabeza es lo principal
    "f_baston": ("baston", "cristal_azul", MADERA, "Baston", GUARNICION),
    "f_baston_herrado": ("baston_herrado", HOJA, MADERA, "Baston herrado", GUARNICION),
    "f_vara": ("vara", "cristal_azul", MADERA, "Vara", GUARNICION),
    "f_cetro": ("cetro", HOJA, MADERA, "Cetro", GUARNICION),
    "f_tea": ("tea", "llama", MADERA, "Tea", "cuerda"),
    # mano izquierda
    "f_escudo": ("escudo_cometa", "madera", "tabardo_azul", "Escudo", HOJA),
    "f_rodela": ("rodela", "madera", MANGO, "Rodela", HOJA),
    "f_broquel": ("broquel", "madera", MANGO, "Broquel", HOJA),
    "f_paves": ("paves", "madera", "tabardo_azul", "Paves", HOJA),
    "f_brazal": ("brazal", HOJA, MANGO, "Brazal", GUARNICION),
    "f_orbe": ("orbe", "cristal_azul", "plata", "Orbe"),
    "f_grimorio": ("grimorio", "habito_purpura", "oro_emblema", "Grimorio"),
    "f_simbolo": ("simbolo", GUARNICION, CUERDA, "Simbolo sagrado"),
    "f_talisman": ("talisman", "gema_azul", CUERDA, "Talisman", GUARNICION),
    "f_kit": ("kit", "cuero", MANGO, "Kit de herramientas", HOJA),
    "f_espejo": ("espejo", "cristal_azul", MADERA, "Espejo de mano", GUARNICION),
    "f_farol": ("farol", "llama", HOJA, "Farol"),
}


# --- ESTETICA-01: la escalera de detalle (desde raras) ----------------------------------------
# Oriol (06/10): «la epicidad y nivel de detalle en las armas [...] una escalera de progresion
# de lo que deberia ser el arma o armadura mas basica a la mas fuerte», con NosTale de
# referencia; dibujo mas grande «desde raras». Lo comun y lo poco comun llevan la forma de
# arriba (lo basico); las raras, las epicas y las miticas, estas:
#
#   raro    mas larga y una columna mas ancha, guarda con las puntas vueltas, pomo con remate
#   epico   pomo y guarda con gema (material 4: f g h i j), runas en la hoja (w), mas larga
#   mitico  corona de gema, guarda con alas, mas runas: la mas grande
#
# Se dibujan de frente; de perfil salen inclinadas (perfil()) y las hachas, en espejo.


def perfil(abajo, guarda, paso=2):
    """La vista de perfil a partir de la de frente: lo que hay bajo la guarda se va a la
    derecha una columna cada `paso` filas, y lo de encima a la izquierda (el arma inclinada
    hacia delante, como las de perfil dibujadas a mano)."""
    filas = []
    for r, f in enumerate(abajo):
        d = (r - guarda) // paso if r >= guarda else -((guarda - r + paso - 1) // paso)
        filas.append((d, f))
    minimo = min(d for d, _ in filas)
    out = ["." * (d - minimo) + f for d, f in filas]
    ancho = max(len(f) for f in out)
    out = [f.ljust(ancho, ".") for f in out]
    while all(f[0] == "." for f in out):
        out = [f[1:] for f in out]
    while all(f[-1] == "." for f in out):
        out = [f[:-1] for f in out]
    return out


def espejo(filas):
    ancho = max(len(f) for f in filas)
    return [f.ljust(ancho, ".")[::-1] for f in filas]


def _forma(abajo, guarda=2, en_espejo=False):
    """{abajo, derecha}: guarda = filas desde el puno hasta la guarda."""
    if en_espejo:
        return {"abajo": abajo, "derecha": espejo(abajo)}
    g = next(i for i, f in enumerate(abajo) if any(c in f for c in armas.ANCLAS)) + guarda
    return {"abajo": abajo, "derecha": perfil(abajo, g)}


SABLE_RARO_FRENTE = [
    "..dab..",
    ".d.97..",
    ".d.@7..",
    ".d.97..",
    ".d.97..",
    "dbaaaad",
    "e.4123e",
    "..4123.",
    "..4123.",
    "..4123.",
    "..4123.",
    "..4123.",
    "...4123",
    "...4123",
    "...4123",
    "....423",
    "....43.",
    ".....4.",
]
SABLE_EPICO_FRENTE = [
    "..dgd...",
    "..aga...",
    ".d.97...",
    ".d.@7...",
    ".d.97...",
    ".d.97...",
    "dbagaabd",
    "e.41223e",
    "..41223.",
    "..41w23.",
    "..41223.",
    "..41223.",
    "...41223",
    "...41w23",
    "...41223",
    "...41223",
    "....4123",
    "....4123",
    ".....423",
    ".....43.",
    "......4.",
]
SABLE_MITICO_FRENTE = [
    "....g....",
    "...dgd...",
    "...aga...",
    "..d.97...",
    "..d.@7...",
    "..d.97...",
    "..d.97...",
    "abd.gd.ba",
    "dbaagaabd",
    "e.d4122de",
    "...41223.",
    "...4w223.",
    "...41223.",
    "...41223.",
    "....41223",
    "....4w223",
    "....41223",
    "....41223",
    ".....4123",
    ".....4w23",
    ".....4123",
    "......423",
    "......423",
    "......43.",
    ".......4.",
]
DAGA_RARO_FRENTE = [
    ".dabd.",
    "..97..",
    "..@7..",
    "..97..",
    "dbaabd",
    "e4123e",
    ".4123.",
    ".4123.",
    ".4123.",
    "..423.",
    "..43..",
    "..4...",
]
DAGA_EPICO_FRENTE = [
    "..dggd..",
    "..abba..",
    "...97...",
    "...@7...",
    "...97...",
    "dbaggabd",
    "e.4123.e",
    "..4123..",
    "..4w23..",
    "..4123..",
    "..4123..",
    "..4w23..",
    "...423..",
    "...43...",
    "...4....",
]
DAGA_MITICO_FRENTE = [
    "...gg...",
    "..dggd..",
    "..abba..",
    "...97...",
    "...@7...",
    "...97...",
    "ab.gg.ba",
    "dbaggabd",
    "e.41223e",
    "..41223.",
    "..4w223.",
    "..41223.",
    "..41223.",
    "..4w223.",
    "...4123.",
    "...4123.",
    "...423..",
    "...43...",
    "...4....",
]
CUCHILLO_RARO_FRENTE = [
    ".dabd.",
    "..97..",
    "..97..",
    "..@7..",
    "..97..",
    ".dbbd.",
    "41223.",
    "41223.",
    "41223.",
    "41223.",
    "41223.",
    ".4123.",
    ".413..",
    "..43..",
    "..4...",
]
CUCHILLO_EPICO_FRENTE = [
    "..dggd.",
    "..abba.",
    "...97..",
    "...97..",
    "...@7..",
    "...97..",
    ".dbggbd",
    ".412233",
    ".412233",
    ".41w233",
    ".412233",
    ".412233",
    ".41w233",
    "..41233",
    "..41223",
    "...4123",
    "...413.",
    "....43.",
    "....4..",
]
CUCHILLO_MITICO_FRENTE = [
    "...gg..",
    "..dggd.",
    "..abba.",
    "...97..",
    "...97..",
    "...@7..",
    "...97..",
    "ab.gg.ba",
    "dbbggbbd",
    ".4122233",
    ".4122233",
    ".41w2233",
    ".4122233",
    ".4122233",
    ".41w2233",
    ".4122233",
    "..412233",
    "..412223",
    "...41223",
    "...4123.",
    "....413.",
    "....43..",
    "....4...",
]
ESTILETE_RARO_FRENTE = [
    "..dab..",
    "...97..",
    "...@7..",
    "...97..",
    "dbbabbd",
    "e..41.e",
    "...41..",
    "...41..",
    "...41..",
    "...41..",
    "...41..",
    "...41..",
    "...41..",
    "...4...",
]
ESTILETE_EPICO_FRENTE = [
    "...dgd...",
    "...aga...",
    "....97...",
    "....@7...",
    "....97...",
    "edbbgbbde",
    ".e..413e.",
    "....413..",
    "....4w3..",
    "....413..",
    "....413..",
    "....4w3..",
    "....413..",
    "....413..",
    "....41...",
    "....41...",
    "....4....",
]
ESTILETE_MITICO_FRENTE = [
    "....g....",
    "...dgd...",
    "...aga...",
    "....97...",
    "....@7...",
    "....97...",
    "ab..g..ba",
    "edbbgbbde",
    ".e.d413e.",
    "....413..",
    "....4w3..",
    "....413..",
    "....413..",
    "....4w3..",
    "....413..",
    "....413..",
    "....4w3..",
    "....413..",
    "....41...",
    "....41...",
    "....4....",
]
HACHA_MANO_RARO_FRENTE = [
    "..44....",
    ".4124...",
    "41224d..",
    "412234b.",
    "412234d.",
    "41234.79",
    ".444..79",
    "......79",
    "......79",
    "......79",
    "......@9",
    "......79",
    "......cd",
]
HACHA_MANO_EPICO_FRENTE = [
    "...444...",
    "..41224..",
    ".412224d.",
    "4122234g.",
    "4122234b.",
    "4122234d.",
    ".412234.79",
    "..44434.79",
    ".......79",
    ".......79",
    ".......79",
    ".......79",
    ".......@9",
    ".......79",
    ".......ad",
    ".......cd",
]
HACHA_MANO_MITICO_FRENTE = [
    "....444.....",
    "...41224....",
    "..4122224d..",
    ".41222234g4.",
    "4122222w4g34",
    "4122222w4b24",
    ".41222234d4.",
    "..4122224.79",
    "...44444..79",
    "..........79",
    "..........79",
    "..........79",
    "..........79",
    "..........@9",
    "..........79",
    "..........79",
    "..........ad",
    "..........gd",
]

ESCALERAS = {  # forma basica -> paso -> forma
    "sable": {"raro": _forma(SABLE_RARO_FRENTE), "epico": _forma(SABLE_EPICO_FRENTE),
              "mitico": _forma(SABLE_MITICO_FRENTE)},
    "daga": {"raro": _forma(DAGA_RARO_FRENTE), "epico": _forma(DAGA_EPICO_FRENTE),
             "mitico": _forma(DAGA_MITICO_FRENTE)},
    "cuchillo": {"raro": _forma(CUCHILLO_RARO_FRENTE), "epico": _forma(CUCHILLO_EPICO_FRENTE),
                 "mitico": _forma(CUCHILLO_MITICO_FRENTE)},
    "estilete": {"raro": _forma(ESTILETE_RARO_FRENTE), "epico": _forma(ESTILETE_EPICO_FRENTE),
                 "mitico": _forma(ESTILETE_MITICO_FRENTE)},
    "hacha_mano": {"raro": _forma(HACHA_MANO_RARO_FRENTE, en_espejo=True),
                   "epico": _forma(HACHA_MANO_EPICO_FRENTE, en_espejo=True),
                   "mitico": _forma(HACHA_MANO_MITICO_FRENTE, en_espejo=True)},
}

# El resto de formas que llevan objetos raros y epicos (de frente; el perfil, abajo).
ESPADA_RARO_FRENTE = [
    ".dabd.",
    "..97..",
    "..97..",
    "..@7..",
    "..97..",
    "..97..",
    "dbaabd",
    "e4123e",
    ".4123.",
    ".4123.",
    ".4123.",
    ".4123.",
    ".4123.",
    ".4123.",
    ".4123.",
    "..423.",
    "..43..",
    "..4...",
]
ESTOQUE_RARO_FRENTE = [
    ".dabd.",
    "..97..",
    "..@7..",
    "..97..",
    ".daad.",
    "dabbad",
    "e.dd.e",
    ".edcde",
    "..41..",
    "..4w..",
    "..41..",
    "..41..",
    "..41..",
    "..41..",
    "..41..",
    "..41..",
    "..41..",
    "..41..",
    "..4...",
]
MANDOBLE_EPICO_FRENTE = [
    "..dggd..",
    "..abba..",
    "...97...",
    "...97...",
    "...97...",
    "...@7...",
    "...97...",
    "ab.gg.ba",
    "dbaggabd",
    "e.4122ce",
    "..41223.",
    "..41w23.",
    "..41223.",
    "..41223.",
    "..41223.",
    "..41w23.",
    "..41223.",
    "..41223.",
    "..41223.",
    "..41w23.",
    "..41223.",
    "...4123.",
    "...423..",
    "...43...",
    "...4....",
]
HACHA_RARO_FRENTE = [
    "....4......",
    "...414.....",
    "..41244....",
    ".4122234...",
    "41222234d..",
    "41222223b..",
    "41222223b..",
    "41222234d..",
    ".4122234...",
    "..41244.9..",
    "...414.d79.",
    "....4..d79.",
    "........79.",
    "........79.",
    "........79.",
    "........@9.",
    "........79.",
    "........79.",
    "........cd.",
]
MARTILLO_RARO_FRENTE = [
    "...4.....",
    "..d4d....",
    "ddddddddd",
    "d41222b3d",
    "d41223b3d",
    "d41233b3d",
    "ddddddddd",
    "....bd...",
    "....79...",
    "....79...",
    "....79...",
    "....79...",
    "....@9...",
    "....79...",
    "....79...",
    "....cd...",
]
MARTILLO_EPICO_FRENTE = [
    "....4.....",
    "...d4d....",
    "dddddddddd",
    "d4122gb23d",
    "d412gfgb3d",
    "d4123gb33d",
    "d4123bb33d",
    "dddddddddd",
    ".ab.bd.ba.",
    ".....79...",
    ".....79...",
    ".....79...",
    ".....79...",
    ".....79...",
    ".....@9...",
    ".....79...",
    ".....79...",
    ".....ad...",
    ".....cd...",
]
MAZA_EPICO_FRENTE = [
    "...4.4.4...",
    "..414141...",
    ".4121214...",
    "412122234..",
    ".4122g234..",
    "41222g2234.",
    ".4122g234..",
    "412223334..",
    ".4123334...",
    "..43434....",
    "...dbd.....",
    "....79.....",
    "....79.....",
    "....79.....",
    "....@9.....",
    "....79.....",
    "....79.....",
    "....ad.....",
    "....gd.....",
]
LANZA_RARO_FRENTE = [
    "...4...",
    "..414..",
    "..414..",
    ".41234.",
    ".41234.",
    "d41234d",
    "bd4234d",
    "..d43d.",
    "..ddd..",
    "...79..",
    "...79..",
    "...79..",
    "...79..",
    "...79..",
    "...79..",
    "...79..",
    "...79..",
    "...79..",
    "...79..",
    "...79..",
    "...79..",
    "...79..",
    "...79..",
    "...@9..",
    "...79..",
    "...79..",
    "...79..",
    "...79..",
    "...cd..",
]
TRIDENTE_EPICO_FRENTE = [
    "4..4..4",
    "41.41.41",
    "41.41.41",
    "41.41.41",
    "4122222234",
    ".4122g234.",
    "..d4g4d...",
    "...dgd....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....@9....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....79....",
    "....cd....",
]
GUADANA_EPICO_FRENTE = [
    "444444.....",
    "21112244...",
    ".222223344.",
    "...22233344",
    "......d.344",
    ".....dgd...",
    "......79...",
    "......79...",
    "......79...",
    "......79...",
    "......79...",
    "......79...",
    "......79...",
    "......79...",
    "......79...",
    "......79...",
    "......79...",
    "......79...",
    "......79...",
    "......79...",
    "......@9...",
    "......79...",
    "......79...",
    "......79...",
    "......79...",
    "......cd...",
]
ARCO_RARO_FRENTE = [
    "..d4..",
    "..412.",
    ".412.9",
    ".41..9",
    "412..9",
    "41...9",
    "41...9",
    "4b...9",
    "4@...9",
    "4b...9",
    "41...9",
    "41...9",
    "412..9",
    ".41..9",
    ".412.9",
    "..412.",
    "..d4..",
]
ARCO_EPICO_FRENTE = [
    "...dg..",
    "..d41..",
    "..412.9",
    ".412..9",
    ".41...9",
    "412...9",
    "41....9",
    "4b....9",
    "4g....9",
    "4@....9",
    "4b....9",
    "41....9",
    "412...9",
    ".41...9",
    ".412..9",
    "..412.9",
    "..d41..",
    "...dg..",
]
BALLESTA_RARO_FRENTE = [
    "...44....",
    "...41....",
    "...41....",
    "...@1....",
    "...41....",
    "...49....",
    "..9419...",
    ".9.41.9..",
    "9..41..9.",
    "dcbaabcd.",
    "d..44..d.",
]
VARA_RARO_FRENTE = [
    "..4...",
    ".4w4..",
    "4w124.",
    ".424..",
    ".dad..",
    "..7...",
    "..b...",
    "..@...",
    "..7...",
    "..b...",
    "..c...",
]
CETRO_RARO_FRENTE = [
    "...4....",
    "..414...",
    ".41234..",
    "4122234.",
    ".41234..",
    "..424...",
    ".dadad..",
    "...79...",
    "...ad...",
    "...@9...",
    "...79...",
    "...ad...",
    "...cd...",
]
BASTON_EPICO_FRENTE = [
    "..a.a...",
    ".d4w4d..",
    "d41w14d.",
    "d41224d.",
    ".d424d..",
    "..dgd...",
    "...dd...",
    "...79...",
    "...79...",
    "...ad...",
    "...79...",
    "...79...",
    "...ad...",
    "...79...",
    "...79...",
    "...ad...",
    "...79...",
    "...@9...",
    "...79...",
    "...ad...",
    "...79...",
    "...79...",
    "...ad...",
    "...79...",
    "...cd...",
]
TEA_EPICO_FRENTE = [
    "...1....",
    "..121...",
    ".1w2w1..",
    ".12321..",
    "dad.dad.",
    ".dabad..",
    "..dgd...",
    "...79...",
    "...@9...",
    "...79...",
    "...ad...",
    "...cd...",
]
FAROL_EPICO_FRENTE = [
    "...@...",
    "...9...",
    "..dad..",
    ".daaad.",
    "d4w1w4d",
    "d4w1w4d",
    "d4w1w4d",
    ".d444d.",
    "..dgd..",
    "...d...",
]
ESPEJO_EPICO_FRENTE = [
    "..dgd..",
    ".ddddd.",
    "dd1w1dd",
    "d11w21d",
    "d12w22d",
    "dd223dd",
    ".ddddd.",
    "...b...",
    "...7...",
    "...@...",
    "...7...",
    "...g...",
]
ESCUDO_COMETA_RARO_FRENTE = [
    "dddddddddd",
    "dab17112cd",
    "da117112cd",
    "da777777cd",
    "da11@112cd",
    "da12712bcd",
    "da12712bcd",
    ".da2723cd.",
    ".da2723cd.",
    "..da73cd..",
    "..da73cd..",
    "...dacd...",
    "....dd....",
]
ESCUDO_COMETA_EPICO_FRENTE = [
    ".dddddddddd.",
    "dab1171122cd",
    "da11171122cd",
    "da77ggg777cd",
    "da77g@g777cd",
    "da11ggg122cd",
    "da12271222cd",
    "da12271223cd",
    ".da122723cd.",
    ".da122723cd.",
    "..da2273cd..",
    "..da2273cd..",
    "...da273d...",
    "....dacd....",
    ".....dd.....",
]
RODELA_RARO_FRENTE = [
    "...ddddd...",
    "..dabbbcd..",
    ".da11112cd.",
    "da1112222cd",
    "da12aaab3cd",
    "da12a$cb3cd",
    "da12bccc3cd",
    "da1223333cd",
    ".dc22333cd.",
    "..dcccccd..",
    "...ddddd...",
]
RODELA_EPICO_FRENTE = [
    "....ddddd....",
    "..ddabbbcdd..",
    ".da1111122cd.",
    ".da1112222cd.",
    "da1112aab23cd",
    "da112agggc3cd",
    "da112ag$gc3cd",
    "da112agggc3cd",
    "da1223ccc33cd",
    ".da22233333d.",
    ".dc2223333cd.",
    "..ddcccccdd..",
    "....ddddd....",
]
BROQUEL_RARO_FRENTE = [
    "..ddddd..",
    ".da1112d.",
    "da1aab23d",
    "d1a$$c23d",
    "d1bccc23d",
    "dc22333cd",
    ".dc233cd.",
    "..ddddd..",
]
BROQUEL_EPICO_FRENTE = [
    "..ddddddd..",
    ".da111122d.",
    "da11aaab23d",
    "d11aggga23d",
    "d11ag$gc23d",
    "d11bgggc23d",
    "d122ccc233d",
    "dc22233333d",
    ".dc22333cd.",
    "..ddddddd..",
]
PAVES_RARO_FRENTE = [
    "dddddddddd",
    "da111111cd",
    "da122222cd",
    "da166662cd",
    "da126622cd",
    "da12@622cd",
    "da126622cd",
    "da126622cd",
    "da122222cd",
    "da122223cd",
    "da122233cd",
    "da122333cd",
    "da123333cd",
    "dc333333cd",
    "dddddddddd",
]
PAVES_EPICO_FRENTE = [
    ".dddddddddd.",
    "dda111111cdd",
    "da11222222cd",
    "da12266222cd",
    "da12666622cd",
    "da12ggg622cd",
    "da12g@g622cd",
    "da12ggg622cd",
    "da12266222cd",
    "da12266222cd",
    "da12222223cd",
    "da12222233cd",
    "da12222333cd",
    "da12233333cd",
    "ddc3333333dd",
    ".dddddddddd.",
]
ORBE_RARO_FRENTE = [
    ".a..a.",
    ".4444.",
    "41w114",
    "411224",
    "41@234",
    "422334",
    ".9449.",
    "..99..",
]
ORBE_EPICO_FRENTE = [
    "a..g..a",
    "9.444.9",
    ".41w114",
    "9411224",
    ".41@234",
    "9422334",
    ".94449.",
    "..999..",
    "...g...",
]
GRIMORIO_RARO_FRENTE = [
    "66666666",
    "61111146",
    "61666146",
    "61@66146",
    "61222346",
    "61223346",
    "69999996",
]
GRIMORIO_EPICO_FRENTE = [
    "6666666666",
    "6111111146",
    "6116ff6146",
    "61@6ff6146",
    "6122223346",
    "6122233346",
    "6122333346",
    "6999999996",
]
SIMBOLO_RARO_FRENTE = [
    "..@..",
    "..9..",
    "..9..",
    "4.4.4",
    ".444.",
    "44w44",
    ".424.",
    "4.4.4",
    "..4..",
]
SIMBOLO_EPICO_FRENTE = [
    "...@...",
    "...9...",
    "...9...",
    "4..4..4",
    ".4.4.4.",
    "..414..",
    "444g444",
    "..424..",
    ".4.4.4.",
    "4..4..4",
]
TALISMAN_RARO_FRENTE = [
    "..@..",
    "..9..",
    "..9..",
    ".dad.",
    "d414d",
    "d4w4d",
    "d424d",
    ".dad.",
    "..d..",
]
TALISMAN_EPICO_FRENTE = [
    "...@...",
    "...9...",
    "..9.9..",
    ".dabad.",
    "da414ad",
    "d44w44d",
    "da424ad",
    ".dabad.",
    "..dgd..",
    "...d...",
]

ESCALERAS.update({
    "espada": {"raro": _forma(ESPADA_RARO_FRENTE, 3)},
    "estoque": {"raro": _forma(ESTOQUE_RARO_FRENTE, 2)},
    "mandoble": {"epico": _forma(MANDOBLE_EPICO_FRENTE, 2)},
    "hacha": {"raro": _forma(HACHA_RARO_FRENTE, en_espejo=True)},
    "martillo": {"raro": {"abajo": MARTILLO_RARO_FRENTE, "derecha": MARTILLO_RARO_FRENTE}, "epico": {"abajo": MARTILLO_EPICO_FRENTE, "derecha": MARTILLO_EPICO_FRENTE}},
    "maza": {"epico": {"abajo": MAZA_EPICO_FRENTE, "derecha": MAZA_EPICO_FRENTE}},
    "lanza": {"raro": {"abajo": LANZA_RARO_FRENTE, "derecha": LANZA_RARO_FRENTE}},
    "tridente": {"epico": {"abajo": TRIDENTE_EPICO_FRENTE, "derecha": TRIDENTE_EPICO_FRENTE}},
    "guadana": {"epico": {"abajo": GUADANA_EPICO_FRENTE, "derecha": GUADANA_EPICO_FRENTE}},
    "arco": {"raro": {"abajo": ARCO_RARO_FRENTE, "derecha": ARCO_RARO_FRENTE}, "epico": {"abajo": ARCO_EPICO_FRENTE, "derecha": ARCO_EPICO_FRENTE}},
    "ballesta": {"raro": {"abajo": BALLESTA_RARO_FRENTE, "derecha": BALLESTA_RARO_FRENTE}},
    "vara": {"raro": {"abajo": VARA_RARO_FRENTE, "derecha": VARA_RARO_FRENTE}},
    "cetro": {"raro": {"abajo": CETRO_RARO_FRENTE, "derecha": CETRO_RARO_FRENTE}},
    "baston": {"epico": {"abajo": BASTON_EPICO_FRENTE, "derecha": BASTON_EPICO_FRENTE}},
    "tea": {"epico": {"abajo": TEA_EPICO_FRENTE, "derecha": TEA_EPICO_FRENTE}},
    "farol": {"epico": {"abajo": FAROL_EPICO_FRENTE, "derecha": FAROL_EPICO_FRENTE}},
    "espejo": {"epico": {"abajo": ESPEJO_EPICO_FRENTE, "derecha": ESPEJO_EPICO_FRENTE}},
    "escudo_cometa": {"raro": {"abajo": ESCUDO_COMETA_RARO_FRENTE, "derecha": FORMAS["escudo_cometa"]["derecha"]}, "epico": {"abajo": ESCUDO_COMETA_EPICO_FRENTE, "derecha": FORMAS["escudo_cometa"]["derecha"]}},
    "rodela": {"raro": {"abajo": RODELA_RARO_FRENTE, "derecha": FORMAS["rodela"]["derecha"]}, "epico": {"abajo": RODELA_EPICO_FRENTE, "derecha": FORMAS["rodela"]["derecha"]}},
    "broquel": {"raro": {"abajo": BROQUEL_RARO_FRENTE, "derecha": FORMAS["broquel"]["derecha"]}, "epico": {"abajo": BROQUEL_EPICO_FRENTE, "derecha": FORMAS["broquel"]["derecha"]}},
    "paves": {"raro": {"abajo": PAVES_RARO_FRENTE, "derecha": FORMAS["paves"]["derecha"]}, "epico": {"abajo": PAVES_EPICO_FRENTE, "derecha": FORMAS["paves"]["derecha"]}},
    "orbe": {"raro": {"abajo": ORBE_RARO_FRENTE, "derecha": ORBE_RARO_FRENTE}, "epico": {"abajo": ORBE_EPICO_FRENTE, "derecha": ORBE_EPICO_FRENTE}},
    "grimorio": {"raro": {"abajo": GRIMORIO_RARO_FRENTE, "derecha": GRIMORIO_RARO_FRENTE}, "epico": {"abajo": GRIMORIO_EPICO_FRENTE, "derecha": GRIMORIO_EPICO_FRENTE}},
    "simbolo": {"raro": {"abajo": SIMBOLO_RARO_FRENTE, "derecha": SIMBOLO_RARO_FRENTE}, "epico": {"abajo": SIMBOLO_EPICO_FRENTE, "derecha": SIMBOLO_EPICO_FRENTE}},
    "talisman": {"raro": {"abajo": TALISMAN_RARO_FRENTE, "derecha": TALISMAN_RARO_FRENTE}, "epico": {"abajo": TALISMAN_EPICO_FRENTE, "derecha": TALISMAN_EPICO_FRENTE}},
})

# --- SP-02: el Arquero, las armas de fuego alquimicas y el bumeran --------------------------
# Oriol (07/10): «Añadir el concepte d'armes de foc però, en comptes de pòlvora alquímia i
# màgia son la base tecnològica». La escopeta del Destructor, el cañón de mano del Cañonero, el
# revólver del Cazademonios, el lanzagranadas del Granadero, el rifle del Cazanieblas y el
# bumerán del Guardabosques (y sus armas «de Viaje» para empezar). Material 1 el cañón
# (hierro), 2 la culata y el puño (madera), 3 los herrajes (latón); desde raras, la gema es la
# carga alquímica (las granadas, el núcleo del cañón, la mira, el tambor del revólver). Como
# se coge cada una, en actores_poses (de los vídeos de NosTale).
#
# Las de fuego se dibujan DE LADO, como se ven de perfil: la culata a la izquierda, la boca a
# la derecha y la luz arriba. _de_lado() las pone de pie como las demás maestras (lo de debajo
# del puño es lo que va delante, y actores_poses.tumbada las vuelve a tumbar).


def _de_lado(filas):
    ancho = max(len(f) for f in filas)
    filas = [f.ljust(ancho, ".") for f in filas]
    return ["".join(f[c] for f in filas) for c in range(ancho)]


ESCOPETA_LADO = [  # dos cañones, el corredor de madera debajo, el martillo y el guardamonte
    ".99.......d.....",
    "96699...4414444d",
    "9776@9441132222d",
    "987877223377779.",
    ".989.....dd.....",
]
ESCOPETA_RARO_LADO = [  # con el vial de la carga encima, aros de latón y la boca reforzada
    ".99......did......",
    "96699...411a44d4dd",
    "9776@944123c22b2bd",
    "98787722337777779.",
    ".989.....dd.......",
]
RIFLE_LADO = [  # culata larga, la mira de latón encima, el cañón largo y fino
    "...........ddddddd.............",
    ".99.......dbbbbbbbd............",
    "9669999.....d...d..............",
    "97766669@94414441444444444444dd",
    "98778887772232223222222222222bd",
    ".9889..99..dd.97777779.........",
]
RIFLE_RARO_LADO = [  # la lente de la mira es la gema; aros y freno de boca
    "...........dddddddi.............",
    ".99.......dbbbbbbbi.............",
    "9669999.....d...d...............",
    "97766669@9441444a44444d444444ddd",
    "9877888777223222c22222b222222bbd",
    ".9889..99..dd.97777779..........",
]
LANZAGRANADAS_LADO = [  # culata corta, el tambor de las granadas y el tubo ancho
    ".........ddddddd..........",
    ".99.....d1212121d44444444d",
    "96699...d2424242a11111111d",
    "9776@999b2222222b22222222d",
    "98889...d2424242b22222222d",
    ".99.....d3232323c33333333d",
    ".........dbbbbbd..........",
]
LANZAGRANADAS_RARO_LADO = [  # las granadas del tambor brillan (la gema)
    ".........ddddddd...........",
    ".99.....d1212121d44444444dd",
    "96699...d2g2g2g2a111a1111ad",
    "9776@999b2222222b222b2222bd",
    "98889...d2g2g2g2b222c2222bd",
    ".99.....d3232323c33323333cd",
    ".........dbbbbbd...........",
]
REVOLVER_LADO = [  # el martillo, el tambor con sus estrías, el cañón y la culata hacia abajo
    "...dd........",
    "..d124444444d",
    "..4241412222d",
    ".9@32323bd...",
    ".979..dd.....",
    "979..........",
    "99...........",
]
REVOLVER_RARO_LADO = [  # balas rúnicas en el tambor, cañón con aro, culata con remate
    "...dd..........",
    "..d1244444444dd",
    "..42g1g12b222bd",
    ".9@32323bd.....",
    ".9b9..dd.......",
    "979............",
    "9d.............",
]
CANON = [  # cañón de mano: la boca arriba, acampanada; aros de latón y el asa
    "44444444",
    "41222234",
    ".dabbbc.",
    ".412223.",
    ".412223.",
    ".dabbbc.",
    ".412223.",
    ".412223.",
    ".412223.",
    ".dabbcd.",
    "..4134..",
    "...97...",
    "...@7...",
    "...97...",
    "...dd...",
]
CANON_RARO = [  # la boca mas ancha y el nucleo de sal de fuego (la gema) a la vista
    "4444444444",
    ".41222234.",
    "..dabbbc..",
    "..412223..",
    "..42fh13..",
    "..dabbbc..",
    "..412223..",
    "..412223..",
    "..dabbcd..",
    "...4134...",
    "....97....",
    "....@7....",
    "....97....",
    "....dd....",
]
BUMERAN = [  # una V de madera cogida por un brazo (forrado de cuero), el codo arriba
    "....444....",
    "...42424...",
    "..424.424..",
    ".424...424.",
    "4b4.....d79",
    "44......9@7",
    "........999",
]
BUMERAN_RARO = [  # la gema en el codo y la punta de latón
    "....444....",
    "...42g24...",
    "..424i424..",
    ".424...424.",
    "4b4.....d79",
    "dd......9@7",
    "........999",
]


def _igual(abajo):
    return {"abajo": abajo, "derecha": abajo}


FORMAS.update({
    "escopeta": _igual(_de_lado(ESCOPETA_LADO)), "rifle": _igual(_de_lado(RIFLE_LADO)),
    "lanzagranadas": _igual(_de_lado(LANZAGRANADAS_LADO)), "revolver": _igual(_de_lado(REVOLVER_LADO)),
    "canon": _igual(CANON), "bumeran": _igual(BUMERAN),
})
# (sus piezas base y su escalon raro se suman al final del modulo, despues de las unicas: van al
# final del atlas de armas y ninguna fila de antes se mueve)

GEMA = "gema_roja"  # la gema de origen de las formas de la escalera (material 4)
# la pieza base de cada forma basica (la escalera copia sus materiales)
_BASE_DE_FORMA = {v[0]: k for k, v in BASES.items()}
for _forma_basica, _pasos in ESCALERAS.items():
    for _paso, _dibujo in _pasos.items():
        FORMAS[f"{_forma_basica}_{_paso}"] = _dibujo
        _base = BASES[_BASE_DE_FORMA[_forma_basica]]
        _m3 = _base[4] if len(_base) > 4 else ("bronce" if GUARNICION in _base[1:3] else GUARNICION)
        BASES[f"f_{_forma_basica}_{_paso}"] = (f"{_forma_basica}_{_paso}", _base[1], _base[2],
                                               f"{_base[3]} ({_paso})", _m3, GEMA)


# --- ESTETICA-01: las unicas, con dibujo propio -------------------------------------------
# Oriol (06/10): «algunas unicas que sean miticas o extremadamente raras, y obviamente las
# unicas, aquellas que solo se puedan conseguir». Las veinte miticas de las deidades llevan
# su dibujo (actores_estetica las viste con el, sin tinte: los materiales ya son los suyos)
# y su brillo de deidad. Del tamano de las miticas de la escalera; de perfil, "hoja" es la
# vista de frente inclinada (perfil(), con la guarda tantas filas bajo el puno), "espejo"
# la de frente en espejo (las cabezas de hacha) e "igual" la misma (astiles y arcos). En la
# mano, actores_armas.forma_en_talla las acorta lo justo para cada talla.
#
# id del objeto -> (nombre, perfil, filas de la puno a la guarda, materiales 1..4, frente)
UNICAS = {
    # Aguja del Firmamento (Tejedora de Estrellas): aguja de plata, pomo de estrella y el hilo rojo
    "aguja_del_firmamento": ("Aguja del Firmamento", "hoja", 2,
        ("plata", "tela_azul_noche", "estrella", "tela_roja_oscura"), [
            "...a....",
            "..aba...",
            "aabwbaa.",
            "..aba...",
            "...a....",
            "...97...",
            "...@7...",
            "...97...",
            "..dbbd..",
            "...41...",
            "..4g31..",
            "...41.g.",
            "...41..g",
            "...41..g",
            "...41.g.",
            "...41.g.",
            "...41..g",
            "...41...",
            "...4w...",
            "...41...",
            "...41...",
            "...4....",
        ]),
    # Alfiler del Velo (Reina Velada): alfiler de plata negra, perla violeta y el velo que cuelga
    "alfiler_del_velo": ("Alfiler del Velo", "hoja", 2,
        ("plata_negra", "habito_purpura", "plata_negra", "gema_violeta"), [
            "...iii...",
            "..ifggi..",
            "..igggi..",
            "..ighhi..",
            "...iii...",
            "..dbabd..",
            "..96&....",
            ".9674....",
            ".9671....",
            "96774....",
            "96784....",
            "96881....",
            "9688.4...",
            ".988.4...",
            "..98.1...",
            "...9.4...",
            ".....4...",
            ".....w...",
            ".....4...",
            ".....4...",
        ]),
    # Arco de la Caceria Interminable (Corte Salvaje): arco de cuerna con hojas
    "arco_de_la_caceria_interminable": ("Arco de la Cacería Interminable", "igual", 2,
        ("cuerno_draco", "cuerda", "corteza", "hojas"), [
            "f...4...",
            "gf.414..",
            ".g4124.9",
            "..412..9",
            ".412...9",
            "412....9",
            "41.....9",
            "41f....9",
            "4cg....9",
            "4@g....9",
            "4cf....9",
            "41.....9",
            "41.....9",
            "412....9",
            ".412...9",
            "..412..9",
            ".g4124.9",
            "gf.414..",
            "f...4...",
        ]),
    # Astil del Rayo Escrito (Oraculo Fragmentado): lanza con la punta en rayo y esquirlas de vidrio
    "astil_del_rayo_escrito": ("Astil del Rayo Escrito", "igual", 2,
        ("vidrio", "madera_oscura", "acero_tormenta", "trueno"), [
            "...44...",
            "..414...",
            "..41.f..",
            ".414.g..",
            ".414....",
            "..4134..",
            "f..413..",
            "g.4134..",
            "..413...",
            "..43....",
            ".dadad..",
            "..dfd...",
            *["...79..."] * 40,
            "...@9...",
            "...79...",
            "...79...",
            "...79...",
            "...ad...",
            "...cd...",
        ]),
    # Baston del Camino Compartido (Hermano Mendigo): bordon con calabaza y un hatillo de tela
    "baston_del_camino_compartido": ("Bastón del Camino Compartido", "igual", 2,
        ("roble", "harapo", "cuerda", "tela_amarilla"), [
            "..12...",
            ".1222..",
            ".1223..",
            "..23...",
            "..bb...",
            "..12...",
            ".9bb9..",
            "97b867.",
            "9b6789.",
            "9b7889.",
            ".b999..",
            "..12...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..12...",
            "..13...",
            "..1@...",
            "..13...",
            "..12...",
            "..13...",
            "..ab...",
            "..cb...",
        ]),
    # Bordon del Cruce (Caminante Gris): bordon con el cruce de caminos (dos tablillas) arriba
    "bordon_del_cruce": ("Bordón del Cruce", "igual", 2,
        ("fresno", "lana_gris", "hierro", "pergamino"), [
            "...dd....",
            "...ab....",
            "ffffgggh.",
            "hgggfh...",
            "...ab....",
            ".hgggfff.",
            "...ab....",
            "...12....",
            "..9779...",
            "...79....",
            "...12....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...12....",
            "...13....",
            "...1@....",
            "...13....",
            "...12....",
            "...13....",
            "...ab....",
            "...cd....",
        ]),
    # Colmillo del Ultimo Invierno (Rey Lobo): colmillo de hielo curvo, pomo de cabeza de lobo
    "colmillo_del_ultimo_invierno": ("Colmillo del Último Invierno", "hoja", 2,
        ("rey_lobo", "pelaje_lobo", "plata", "cristal_azul"), [
            ".d.d....",
            ".dbd....",
            "dbfbd...",
            ".dbd....",
            "..97....",
            "..@7....",
            "..97....",
            "dbaabd..",
            ".4123...",
            ".41223..",
            ".41223..",
            "..41223.",
            "..41w23.",
            "..41223.",
            "...4123.",
            "...4123.",
            "....423.",
            "....423.",
            ".....43.",
            "......4.",
        ]),
    # Espada de la Brasa Inextinguible (Llama Eterna): hoja flamigera con un nucleo de brasa
    "espada_de_la_brasa_inextinguible": ("Espada de la Brasa Inextinguible", "hoja", 2,
        ("oro_rojo", "cuero_brasa", "oro_viejo", "llama"), [
            "..dfd..",
            "..aga..",
            "...97..",
            "...97..",
            "...@7..",
            "...97..",
            "ab.g.ba",
            "dbaggad",
            "..4fg3.",
            "..41g23",
            ".41fg3.",
            ".41g23.",
            "..4fg23",
            "..41g3.",
            ".41fg3.",
            ".41g23.",
            "..4fg23",
            "..41g3.",
            "..4f23.",
            "...423.",
            "...43..",
            "...4...",
        ]),
    # Espada del Primer Rayo (Leon del Alba): guarda de sol naciente, hoja ancha
    "espada_del_primer_rayo": ("Espada del Primer Rayo", "hoja", 4,
        ("juramento", "cuero_leon", "oro_viejo", "gema_roja"), [
            "...dgd...",
            "...aga...",
            "....97...",
            "....97...",
            "....@7...",
            "....97...",
            "a.a.a.a.a",
            ".abaaaba.",
            "abaggbaba",
            ".d41223d.",
            "..41223..",
            "..41w23..",
            "..41223..",
            "..41223..",
            "..41w23..",
            "..41223..",
            "..41223..",
            "..41w23..",
            "..41223..",
            "...4123..",
            "...423...",
            "...43....",
            "...4.....",
        ]),
    # Estilete del Firmante (Serpiente de Contratos): estilete-pluma con la serpiente al puno
    "estilete_del_firmante": ("Estilete del Firmante", "hoja", 2,
        ("plata", "tinta_negra", "verde_serpiente", "oro_viejo"), [
            "...f....",
            "..fgf...",
            "..fgf...",
            "...g....",
            "..a97...",
            "...@b...",
            "..b97...",
            "...9a...",
            "..dabd..",
            "...41...",
            "...41...",
            "...41...",
            "...4w...",
            *["...41..."] * 4,
            "...4....",
            "...9....",
        ]),
    # Guadana de la Buena Siega (Espiga Dorada): guadana dorada con espigas atadas
    "guadana_de_la_buena_siega": ("Guadaña de la Buena Siega", "igual", 2,
        ("oro_viejo", "fresno", "cuerda", "paja"), [
            "444444.....",
            "2111224f...",
            ".22222334f.",
            "...222333g4",
            "......d.344",
            ".....f.f...",
            "....fgbgf..",
            ".....gbg...",
            *["......79..."] * 40,
            "......@9...",
            "......79...",
            "......79...",
            "......79...",
            "......cd...",
        ]),
    # Hacha del Cuerno Roto (Toro de Gongorguma): hacha con la hoja de cuerno, una punta rota
    "hacha_del_cuerno_roto": ("Hacha del Cuerno Roto", "espejo", 2,
        ("cuerno_claro", "madera_oscura", "bronce", "gema_roja"), [
            "...4.......",
            "..414......",
            ".4124......",
            "41224......",
            "412234.....",
            "4122234d...",
            "4122223b...",
            "4122223f...",
            "4122223b...",
            "4122234d...",
            "412234.79..",
            ".4134..79..",
            "..44...79..",
            ".......79..",
            ".......79..",
            ".......79..",
            ".......@9..",
            ".......79..",
            ".......79..",
            ".......ad..",
            ".......cd..",
        ]),
    # Hoja del Ano Nuevo (Madre de los Doce): doce muescas de colores en la hoja
    "hoja_del_ano_nuevo": ("Hoja del Año Nuevo", "hoja", 2,
        ("acero", "tunica_verde", "oro_viejo", "gema_verde"), [
            "...dgd..",
            "...aga..",
            "....97..",
            "....97..",
            "....@7..",
            "....97..",
            ".dbagabd",
            "..d4123.",
            "...4f23.",
            "...4123.",
            "...4123.",
            "...4f23.",
            "...4123.",
            "...4123.",
            "...4f23.",
            "...4123.",
            "...4123.",
            "...4f23.",
            "...4123.",
            "....423.",
            "....43..",
            "....4...",
        ]),
    # Hoz del Cuarto Menguante (Luna Hueca): una luna creciente en un mango corto
    "hoz_del_cuarto_menguante": ("Hoz del Cuarto Menguante", "espejo", 2,
        ("luna", "madera_oscura", "plata", "gema_violeta"), [
            "...444...",
            "..41124..",
            ".4122.4..",
            "4122....4",
            "412......",
            "412......",
            "4122.....",
            ".41224...",
            "..41224d.",
            "...444fb.",
            ".......79",
            ".......79",
            ".......79",
            ".......@9",
            ".......79",
            ".......cd",
        ]),
    # Mandoble del Estampido (Forja del Trueno): espadon de runas de tormenta, guarda de nube
    "mandoble_del_estampido": ("Mandoble del Estampido", "hoja", 4,
        ("acero_tormenta", "cuero_azul", "acero", "trueno"), [
            "..dffd..",
            "..aggb..",
            "...97...",
            "...97...",
            "...97...",
            "...@7...",
            "...97...",
            ".abbbba.",
            "abaggaba",
            ".dbaabd.",
            "..41223.",
            "..41f23.",
            "..4g223.",
            "..412f3.",
            "..41223.",
            "..41f23.",
            "..4g223.",
            "..412f3.",
            "..41223.",
            "..41f23.",
            "..41223.",
            "...4123.",
            "...423..",
            "...43...",
            "...4....",
        ]),
    # Martillo de la Primera Chispa (Padre de la Forja): martillo de herrero con la cabeza al rojo
    "martillo_de_la_primera_chispa": ("Martillo de la Primera Chispa", "igual", 2,
        ("hierro_forja", "madera_oscura", "bronce", "llama"), [
            ".dddddddd.",
            "d412222ffd",
            "d412222fgd",
            "d412233ghd",
            ".dddddddd.",
            "....bd....",
            *["....79...."] * 5,
            "....@9....",
            "....79....",
            "....79....",
            "....ad....",
            "....cd....",
        ]),
    # Pala del Ultimo Servicio (Sepulturero Amable): pala con un farolillo atado al astil
    "pala_del_ultimo_servicio": ("Pala del Último Servicio", "igual", 2,
        ("hierro_negro", "madera_oscura", "cuerda", "llama"), [
            "..412..",
            ".41223.",
            ".41223.",
            ".41223.",
            "..423..",
            "...4...",
            "...79..",
            "..b79..",
            ".d.79..",
            "dfd79..",
            "dgd79..",
            ".d.79..",
            *["...79.."] * 40,
            "...@9..",
            "...79..",
            "...79..",
            "...79..",
            "...cd..",
        ]),
    # Pluma-Espada del Indice (Cuervo Bibliotecario): la hoja es una pluma negra
    "pluma_espada_del_indice": ("Pluma-Espada del Índice", "hoja", 2,
        ("tinta_negra", "pergamino", "plata", "gema_azul"), [
            "...dgd..",
            "...aga..",
            "....97..",
            "....@7..",
            "....97..",
            "..dbabd.",
            ".4.412..",
            "4.41223.",
            ".441223.",
            "4.41223.",
            ".441w23.",
            "4.41223.",
            ".441223.",
            "4.41223.",
            ".441223.",
            "..41223.",
            "..4123..",
            "..4123..",
            "...423..",
            "...43...",
            "...4....",
        ]),
    # Tridente de la Marea Alta (Senora de las Mareas): tridente de coral con una perla
    "tridente_de_la_marea_alta": ("Tridente de la Marea Alta", "igual", 2,
        ("bronce_marino", "madera_oscura", "plata", "perla"), [
            "4...4...4",
            "41..41..41",
            ".41.41.41",
            ".41.41.41",
            "..41414..",
            "..4fgf4..",
            "...4g4...",
            "...dbd...",
            *["....79..."] * 40,
            "....@9...",
            *["....79..."] * 4,
            "....cd...",
        ]),
    # Vara del Farol Encendido (Dama del Farol Rojo): vara con un farol rojo colgado de un gancho
    "vara_del_farol_encendido": ("Vara del Farol Encendido", "igual", 2,
        ("terciopelo_rojo", "madera_oscura", "laton", "llama"), [
            "..dddd...",
            "..d..d...",
            "..d..79..",
            "..d..79..",
            ".dad.79..",
            "daaad79..",
            "d1f1d79..",
            "d1g1d79..",
            "d1f1d79..",
            ".d4d.79..",
            "..d..79..",
            *[".....79.."] * 40,
            ".....@9..",
            ".....79..",
            ".....79..",
            ".....79..",
            ".....cd..",
        ]),
}

for _ident, (_nombre, _modo, _guarda, _mats, _frente) in UNICAS.items():
    if _modo == "hoja":
        _dibujo = _forma(_frente, _guarda)
    elif _modo == "espejo":
        _dibujo = _forma(_frente, en_espejo=True)
    else:
        _dibujo = {"abajo": _frente, "derecha": _frente}
    FORMAS[f"u_{_ident}"] = _dibujo
    BASES[f"f_u_{_ident}"] = (f"u_{_ident}", _mats[0], _mats[1], _nombre, _mats[2], _mats[3])

# En la mano, cada unica se coge como la familia a la que se parece (actores_poses).
COGIDA_COMO = {
    "aguja_del_firmamento": "daga", "alfiler_del_velo": "daga", "colmillo_del_ultimo_invierno": "daga",
    "estilete_del_firmante": "estilete",
    "espada_de_la_brasa_inextinguible": "espada", "espada_del_primer_rayo": "espada",
    "hoja_del_ano_nuevo": "espada", "pluma_espada_del_indice": "espada",
    "mandoble_del_estampido": "mandoble",
    "hacha_del_cuerno_roto": "hacha", "hoz_del_cuarto_menguante": "hacha_mano",
    "martillo_de_la_primera_chispa": "martillo",
    "astil_del_rayo_escrito": "lanza", "tridente_de_la_marea_alta": "tridente",
    "baston_del_camino_compartido": "baston", "bordon_del_cruce": "baston",
    "vara_del_farol_encendido": "baston", "pala_del_ultimo_servicio": "baston",
    "guadana_de_la_buena_siega": "guadana", "arco_de_la_caceria_interminable": "arco",
}
assert set(COGIDA_COMO) == set(UNICAS), sorted(set(UNICAS) ^ set(COGIDA_COMO))
poses.FAMILIA_DE.update({f"u_{k}": v for k, v in COGIDA_COMO.items()})

# SP-02: las armas del Arquero, al final del atlas de armas (ninguna fila de antes se mueve): sus
# piezas base y despues su escalon raro (desde raras, como la escalera de las demas).
BASES.update({
    "f_escopeta": ("escopeta", HOJA, MADERA, "Escopeta", GUARNICION),
    "f_rifle": ("rifle", HOJA, MADERA, "Rifle", GUARNICION),
    "f_lanzagranadas": ("lanzagranadas", HOJA, MADERA, "Lanzagranadas", GUARNICION),
    "f_revolver": ("revolver", HOJA, MADERA, "Revolver", GUARNICION),
    "f_canon": ("canon", HOJA, MADERA, "Canon de mano", GUARNICION),
    "f_bumeran": ("bumeran", MADERA, MANGO, "Bumeran", GUARNICION),
})
_ESCALERA_ARQUERO = {
    "escopeta": {"raro": _igual(_de_lado(ESCOPETA_RARO_LADO))},
    "rifle": {"raro": _igual(_de_lado(RIFLE_RARO_LADO))},
    "lanzagranadas": {"raro": _igual(_de_lado(LANZAGRANADAS_RARO_LADO))},
    "revolver": {"raro": _igual(_de_lado(REVOLVER_RARO_LADO))},
    "canon": {"raro": _igual(CANON_RARO)},
    "bumeran": {"raro": _igual(BUMERAN_RARO)},
}
ESCALERAS.update(_ESCALERA_ARQUERO)
for _forma_basica, _pasos in _ESCALERA_ARQUERO.items():
    for _paso, _dibujo in _pasos.items():
        FORMAS[f"{_forma_basica}_{_paso}"] = _dibujo
        _base = BASES[f"f_{_forma_basica}"]
        BASES[f"f_{_forma_basica}_{_paso}"] = (f"{_forma_basica}_{_paso}", _base[1], _base[2],
                                               f"{_base[3]} ({_paso})", _base[4], GEMA)


# --- SP-02: el Mago, la pistola magica y el lanzallamas -------------------------------------
# La pistola magica del Mago de las tinieblas y de Merlin (dispara balas de mana: la llave de
# laton y, desde raras, el cristal de mana) y el lanzallamas de la Tormenta de fuego (quema
# aceite alquimico: el deposito bajo el tubo y, desde raras, la llama piloto en la boquilla).
# De lado como las del Arquero; van al final del atlas de armas (ninguna fila de antes se mueve).
PISTOLA_LADO = [  # el martillo, la llave, el cañón largo y fino con la boca de campana, el puño curvo
    "...dd...........d",
    "..da14444444444dd",
    ".9@c32222222222bd",
    ".979.dd.........d",
    "969..............",
    "98d..............",
    "dd...............",
]
PISTOLA_RARO_LADO = [  # el cristal de maná en la llave (la gema) y los aros del cañón
    "...ii............d",
    "..dgg4444d444444dd",
    ".9@b22222b222222bd",
    ".979.dd..........d",
    "969...............",
    "98d...............",
    "dd................",
]
LANZALLAMAS_LADO = [  # la culata, la válvula, el tubo largo, el depósito de aceite debajo y la boquilla
    ".99.......d................",
    "96699....dbd...........dd..",
    "9776@999d111444444444441add",
    "98889...d322111222222223cbd",
    ".99......d22222d.......dd..",
    ".........d33333d...........",
    "..........dbbbd............",
]
LANZALLAMAS_RARO_LADO = [  # la llama piloto en la boquilla (la gema) y los aros del tubo y del depósito
    ".99.......d..................",
    "96699....dbd...........dd....",
    "9776@999d111d444444d4441addii",
    "98889...d322a112222b2223cbbgi",
    ".99......d22b22d.......dd....",
    ".........d33b33d.............",
    "..........dbcbd..............",
]
FORMAS.update({"pistola": _igual(_de_lado(PISTOLA_LADO)), "lanzallamas": _igual(_de_lado(LANZALLAMAS_LADO))})
BASES.update({
    "f_pistola": ("pistola", HOJA, MADERA, "Pistola magica", GUARNICION),
    "f_lanzallamas": ("lanzallamas", HOJA, MADERA, "Lanzallamas", GUARNICION),
})
_ESCALERA_MAGO = {
    "pistola": {"raro": _igual(_de_lado(PISTOLA_RARO_LADO))},
    "lanzallamas": {"raro": _igual(_de_lado(LANZALLAMAS_RARO_LADO))},
}
ESCALERAS.update(_ESCALERA_MAGO)
for _forma_basica, _pasos in _ESCALERA_MAGO.items():
    for _paso, _dibujo in _pasos.items():
        FORMAS[f"{_forma_basica}_{_paso}"] = _dibujo
        _base = BASES[f"f_{_forma_basica}"]
        BASES[f"f_{_forma_basica}_{_paso}"] = (f"{_forma_basica}_{_paso}", _base[1], _base[2],
                                               f"{_base[3]} ({_paso})", _base[4], GEMA)


# --- SP-02: los guanteletes del Artista Marcial (actores_armas.PAREJA) ------------------------
# Oriol (07/10): «Guanteletes grandes», uno en cada mano. Una pieza de la ranura "arma" que se pinta en
# los dos punos (actores_armas.capa_pareja) y encima del cuerpo tambien de espaldas (su fila lleva su
# propio orden de dibujo). Se dibujan ya a su tamano (no se doblan ni se cogen con pose): de frente, el
# de la mano de la izquierda de la imagen, con el pulgar hacia el cuerpo (el otro, en espejo), y de
# perfil, mirando a la derecha, los nudillos delante. El ancla '&' (o '@' en los de cuero) es el centro
# del puno.
GUANTE = [
    "..dbbbbd..",
    "..412234..",
    "..412234..",
    ".dabbbbad.",
    ".41122234.",
    "4111&22234",
    "4112222234",
    "4122222334",
    ".41313134.",
    "..444444..",
]
GUANTE_LADO = [
    "..dbbbd...",
    "..41234...",
    "..41234...",
    ".dabbbad..",
    ".41122234.",
    "4111&22234",
    "4112222234",
    "4122222314",
    ".412223134",
    "..444444..",
]
# Raro: el puno de la manopla con su gema en el dorso, los nudillos de guarnicion y el aro de
# arriba mas ancho
GUANTE_RARO = [
    ".dbbbbbbd.",
    "..412234..",
    "..412234..",
    ".dabbbbad.",
    ".41122234.",
    "4111fg2234",
    "411&gh2234",
    "4122222334",
    ".4b3b3b34.",
    "..444444..",
]
GUANTE_RARO_LADO = [
    ".dbbbbd...",
    "..41234...",
    "..41234...",
    ".dabbbad..",
    ".41122234.",
    "4111fg2234",
    "411&gh2234",
    "4122222b34",
    ".412223b34",
    "..444444..",
]
# Garras: la manopla de cuero o pelaje (material 2) y tres garras (material 1) bajo los nudillos
GARRAS = [
    "..dbbbbd..",
    "..967789..",
    "..967789..",
    ".dabbbbad.",
    ".96677789.",
    "9666@77789",
    "9667777789",
    "9677777889",
    ".9a8a8a89.",
    "..1.1.1...",
    "..2.2.2...",
    "..3.3.3...",
    "..4.4.4...",
]
GARRAS_LADO = [
    "..dbbbd....",
    "..96789....",
    "..96789....",
    ".dabbbad...",
    ".96677789..",
    "9666@77789.",
    "9667777789.",
    "9677777a89.",
    ".9677788a9.",
    "..999999.1.",
    ".......1.2.",
    "........23.",
    "........4..",
]
GARRAS_RARO = [
    ".dbbbbbbd.",
    "..967789..",
    "..967789..",
    ".dabbbbad.",
    ".96677789.",
    "9666fg7789",
    "966@gh7789",
    "9677777889",
    ".9a8a8a89.",
    ".11.11.11.",
    "..2.2..2..",
    "..3..3.3..",
    "..4..4..4.",
]
GARRAS_RARO_LADO = [
    ".dbbbbd....",
    "..96789....",
    "..96789....",
    ".dabbbad...",
    ".96677789..",
    "9666fg7789.",
    "966@gh7789.",
    "9677777a89.",
    ".9677788a9.",
    "..999999.11",
    ".......1.2.",
    "........23.",
    "........4..",
]
# Brazos mecanicos (Puno Hidraulico): el antebrazo de placas con el piston por fuera, los
# remaches y un puno mucho mas grande
BRAZOS = [
    "...d8888d...",
    "...412234...",
    ".a.412234...",
    ".b.988889...",
    ".b.412234...",
    ".c.412234...",
    ".dabbbbbbad.",
    "411112222234",
    "41112&222234",
    "412222222334",
    "412412412434",
    "423423423434",
    ".4444444444.",
]
BRAZOS_LADO = [
    "..d8888d....",
    "..412234....",
    "a.412234....",
    "b.988889....",
    "b.412234....",
    "c.412234....",
    "dabbbbbbad..",
    "4111122222d.",
    "41112&222234",
    "41222222b334",
    "4122222b3234",
    "41222223b434",
    ".4444444444.",
]
# El Tronador: en la derecha, el guantelete con las bobinas (aros de guarnicion); en la
# izquierda, el canon de descargas (la boca abajo de frente, delante de perfil)
BOBINAS = [
    ".dbbbbbbd.",
    "..4b22b4..",
    "..4122b4..",
    ".dabbbbad.",
    ".41122234.",
    "4111&22234",
    "4112222234",
    "4122222334",
    ".4b3b3b34.",
    "..444444..",
]
BOBINAS_LADO = [
    ".dbbbbd...",
    "..4b2b4...",
    "..412b4...",
    ".dabbbad..",
    ".41122234.",
    "4111&22234",
    "4112222234",
    "4122222314",
    ".4b2223b34",
    "..444444..",
]
CANON_MANO = [
    ".dbbbbbbd.",
    "..412234..",
    ".dabbbbad.",
    ".41122234.",
    "4111&22234",
    "4fg22222h4",
    "4gh22223h4",
    "4122222334",
    "d4bbbbbb4d",
    ".41222234.",
    ".41iiii34.",
    "..4jjjj4..",
]
CANON_MANO_LADO = [
    ".dbbbbd.......",
    "..41234.......",
    ".dabbbad......",
    ".411222244444.",
    "4111&22b1112d4",
    "41122222b2222d",
    "4122222b322fid",
    "41222223b333jd",
    ".41222234444d4",
    "..444444......",
]

FORMAS.update({
    "guanteletes": {"abajo": GUANTE, "derecha": GUANTE_LADO},
    "garras": {"abajo": GARRAS, "derecha": GARRAS_LADO},
    "brazos_mecanicos": {"abajo": BRAZOS, "derecha": BRAZOS_LADO},
    "canon_de_mano": {"abajo": BOBINAS, "derecha": BOBINAS_LADO, "abajo_2": CANON_MANO, "derecha_2": CANON_MANO_LADO},
})
BASES.update({
    "f_guanteletes": ("guanteletes", HOJA, MANGO, "Guanteletes", GUARNICION),
    "f_garras": ("garras", HOJA, MANGO, "Garras", GUARNICION),
    "f_brazos_mecanicos": ("brazos_mecanicos", HOJA, MANGO, "Brazos mecanicos", GUARNICION),
    "f_canon_de_mano": ("canon_de_mano", HOJA, MANGO, "Guanteletes con canon de descargas", GUARNICION, GEMA),
})
_ESCALERA_ARTISTA = {
    "guanteletes": {"raro": {"abajo": GUANTE_RARO, "derecha": GUANTE_RARO_LADO}},
    "garras": {"raro": {"abajo": GARRAS_RARO, "derecha": GARRAS_RARO_LADO}},
}
ESCALERAS.update(_ESCALERA_ARTISTA)
for _forma_basica, _pasos in _ESCALERA_ARTISTA.items():
    for _paso, _dibujo in _pasos.items():
        FORMAS[f"{_forma_basica}_{_paso}"] = _dibujo
        _base = BASES[f"f_{_forma_basica}"]
        BASES[f"f_{_forma_basica}_{_paso}"] = (f"{_forma_basica}_{_paso}", _base[1], _base[2],
                                               f"{_base[3]} ({_paso})", _base[4], GEMA)
PAREJA = {"guanteletes", "guanteletes_raro", "garras", "garras_raro", "brazos_mecanicos", "canon_de_mano"}

# CP-01: las armas de las clases propias (el laud, el tambor, la hoz, la llave, la bolsa, el
# matraz, los naipes, el totem y el sable y pistola), al final del atlas de armas.
import actores_formas_propias  # noqa: E402,F401
# SETS-03: los guantes del Mago Chasqueador, detras de las de las clases propias
import actores_formas_iteracion3  # noqa: E402,F401
# INSTRUMENTOS-01: los instrumentos y los objetos de rol (la escoba, la almohada, el estandarte...), detras
import actores_formas_objetos  # noqa: E402,F401


def registrar():
    """Suma las formas y las piezas base a actores_armas (una vez)."""
    if "f_daga" in armas.ARMAS:
        return
    armas.FORMAS.update(FORMAS)
    armas.TAPAN_EL_PUNO.update(TAPAN_EL_PUNO)
    armas.PAREJA.update(PAREJA)  # SP-02: los guanteletes, uno en cada mano
    armas.MANO_IZQUIERDA = armas.MANO_IZQUIERDA + tuple(f for f in MANO_IZQUIERDA
                                                        if f not in armas.MANO_IZQUIERDA)
    # los escalones de la escalera van en la mano de su forma basica
    for forma, pasos in ESCALERAS.items():
        for paso in pasos:
            if forma in armas.MANO_IZQUIERDA:
                armas.MANO_IZQUIERDA = armas.MANO_IZQUIERDA + (f"{forma}_{paso}",)
            if forma in armas.TAPAN_EL_PUNO:
                armas.TAPAN_EL_PUNO.add(f"{forma}_{paso}")
    for ident, valor in BASES.items():
        assert ident not in armas.ARMAS, f"pieza repetida: {ident}"
        assert valor[0] in armas.FORMAS, f"{ident}: no hay forma '{valor[0]}'"
        armas.ARMAS[ident] = valor
    armas.aplicar_forja()  # Oriol (23/10): las armas nuevas de la Forja con estas formas
