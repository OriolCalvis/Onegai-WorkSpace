"""Rasgos de raza sobre la cabeza: orejas, cuernos, colmillos, morros,
picos, crestas y barbas.

Formato de actores_tocados: coordenadas del frame QUIETO de cada direccion
del cuerpo base (la cadena de la fila R empieza en x = 22), la izquierda es
el espejo de la derecha y dy baja la plantilla con la cabeza en los pasos
y en las tallas (la cabeza nunca cambia de forma entre tallas).

Material 1 (caracteres 1-5) es la piel o el pelaje de la raza; material 2
(6-9) el del rasgo: cuerno, hueso, pico, morro o el interior de la oreja.
Las cabezas de animal siguen el estilo del cuerpo base: cabeza redonda,
ojos grandes y el rasgo del animal encima (morro de toro, pico, hocico).
"""
from actores_tocados import simetrico

# --- orejas ------------------------------------------------------------------

OREJAS_PUNTIAGUDAS = {  # elfos, drow, sangre feerica, orcos
    "abajo": simetrico(26, [
        ".4",       # 26
        ".43",      # 27
        "..43",     # 28
        "..432",    # 29
        "...43",    # 30
    ]),
    "arriba": simetrico(26, [
        ".4",
        ".43",
        "..43",
        "..432",
        "...43",
    ]),
    "derecha": (23, [
        ".....4",      # 23
        ".....43",     # 24
        "......42",    # 25
        "......432",   # 26
        ".......32",   # 27
    ]),
}

OREJAS_GATO = {  # material 1 = pelaje, 2 = interior de la oreja
    "abajo": simetrico(17, [
        ".....4",      # 17
        ".....44",     # 18
        ".....474",    # 19
        ".....4274",   # 20
        ".....42274",  # 21
        "......4224",  # 22
    ], sombra_derecha=False),
    "arriba": simetrico(17, [
        ".....4",
        ".....44",
        ".....424",
        ".....4224",
        ".....42224",
        "......4224",
    ], sombra_derecha=False),
    "derecha": (15, [
        "........4",      # 15
        ".......444",     # 16
        ".......4274",    # 17
        "......42274",    # 18
        "......422274",   # 19
        "......42224",    # 20
    ]),
}

OREJAS_ZORRO = {  # yokai: mas grandes que las de gato
    "abajo": simetrico(14, [
        "...4",        # 14
        "...44",       # 15
        "...414",      # 16
        "...4174",     # 17
        "...41774",    # 18
        "...417774",   # 19
        "....41774",   # 20
        ".....4124",   # 21
    ], sombra_derecha=False),
    "arriba": simetrico(14, [
        "...4",
        "...44",
        "...414",
        "...4124",
        "...41224",
        "...412224",
        "....41224",
        ".....4124",
    ], sombra_derecha=False),
    "derecha": (12, [
        "........4",        # 12
        "........44",       # 13
        ".......414",       # 14
        ".......4174",      # 15
        "......41774",      # 16
        "......417774",     # 17
        "......411774",     # 18
        ".......41124",     # 19
    ]),
}

OREJAS_TORO = {  # de lado, bajo los cuernos
    "abajo": simetrico(26, [
        ".44",       # 26
        "41774",     # 27
        "42224",     # 28
        ".4444",     # 29
    ]),
    "arriba": simetrico(26, [
        ".44",
        "41224",
        "42224",
        ".4444",
    ]),
    "derecha": (24, [
        "....44",      # 24  oreja hacia atras
        "...41774",    # 25
        "....42224",   # 26
        ".....4444",   # 27
    ]),
}

# --- cuernos -----------------------------------------------------------------

CUERNOS = {  # tieflings: suben y se abren
    "abajo": simetrico(14, [
        "..9",        # 14
        "..96",       # 15
        "..969",      # 16
        "..9679",     # 17
        "...9679",    # 18
        "...96789",   # 19
        "....96789",  # 20
        ".....9789",  # 21
        "......98",   # 22
    ]),
    "derecha": (13, [
        "......9",           # 13
        "......969",         # 14
        "......9679",        # 15
        ".......9679",       # 16
        ".......96789",      # 17
        "........96789",     # 18
        ".........9789",     # 19
        "..........98",      # 20
    ]),
}
CUERNOS["arriba"] = CUERNOS["abajo"]

CUERNOS_TORO = {  # minotauros: salen de lado y suben; punta oscura
    "abajo": simetrico(16, [
        "9",            # 16 punta
        "99",           # 17
        "969",          # 18
        "9679",         # 19
        "96779",        # 20
        ".967799",      # 21
        "..96677899",   # 22
        "...9988889",   # 23
        ".....99999",   # 24
    ]),
    "derecha": (14, [
        "..........99",       # 14 punta
        ".........969",       # 15
        "........9679",       # 16
        ".......96789",       # 17
        ".......96789",       # 18
        "......96789",        # 19
        "......9789",         # 20
        ".......99",          # 21
    ]),
}
CUERNOS_TORO["arriba"] = CUERNOS_TORO["abajo"]

CUERNOS_DRACO = {  # draconidos: nacen sobre las sienes y van hacia atras
    "abajo": simetrico(18, [
        "...9",         # 18
        "...969",       # 19
        "....9679",     # 20
        ".....9679",    # 21
        "......9789",   # 22
        ".......98",    # 23
    ]),
    "derecha": (18, [
        "9",                 # 18 punta hacia atras
        "969",               # 19
        ".96799",            # 20
        "..9677899",         # 21
        "....9978889",       # 22
        "......99999",       # 23
    ]),
}
CUERNOS_DRACO["arriba"] = CUERNOS_DRACO["abajo"]

# --- bocas, morros y picos ---------------------------------------------------

COLMILLOS = {  # orcos: dos colmillos que asoman sobre el labio
    "abajo": simetrico(32, [
        ".......9",    # 32
        ".......69",   # 33
        ".......79",   # 34
    ], sombra_derecha=False),
    "derecha": (30, [
        "..............9",   # 30
        ".............96",   # 31
        ".............97",   # 32
    ]),
}

MORRO_TORO = {  # material 1 = pelaje, 2 = morro
    "abajo": simetrico(33, [
        "......4777",    # 33
        ".....47777",    # 34
        ".....476777",   # 35
        ".....479777",   # 36  ollar
        ".....478777",   # 37
        "......48888",   # 38
        ".......4444",   # 39
    ]),
    "derecha": (29, [
        ".............4444",       # 29
        ".............777774",     # 30
        ".............6777774",    # 31
        ".............7777794",    # 32  ollar
        ".............7777784",    # 33
        ".............888884",     # 34
        "..............4444",      # 35
    ]),
}

PICO = {  # aarakocra: material 2 = pico
    "abajo": simetrico(30, [
        "..........6",   # 30
        ".........76",   # 31
        ".........76",   # 32
        "........977",   # 33
        ".........98",   # 34
        "..........9",   # 35
    ]),
    "derecha": (28, [
        "..............99",         # 28
        "..............66679",      # 29
        "..............777789",     # 30
        "..............8888899",    # 31
        "...............999.9",     # 32
    ]),
}

HOCICO_GATO = {  # nariz rosa y hocico claro
    "abajo": simetrico(32, [
        "..........9",   # 32 nariz
        ".........77",   # 33
    ], sombra_derecha=False),
    "derecha": (30, [
        "................9",     # 30 nariz
        "...............77",     # 31
    ]),
}

HOCICO_LAGARTO = {  # material 1 = escamas, 2 = garganta clara
    "abajo": simetrico(33, [
        ".........9",    # 33 ollares
        "..........",    # 34
        "......4444",    # 35 boca
        ".......777",    # 36 garganta
    ]),
    "derecha": (28, [
        "..............44444",      # 28
        "..............2222224",    # 29
        "..............1222294",    # 30 ollar
        "..............4444444",    # 31 boca
        "..............777784",     # 32
        "..............88884",      # 33
        "...............444",       # 34
    ]),
}

HOCICO_DRACO = {  # como el de lagarto, mas corto y con barbillas
    "abajo": simetrico(33, [
        ".........9",    # 33 ollares
        "..........",    # 34
        "......4444",    # 35 boca
        ".....9.777",    # 36 barbillas
        ".....9",        # 37
    ]),
    "derecha": (28, [
        "..............4444",       # 28
        "..............222224",     # 29
        "..............122294",     # 30 ollar
        "..............444444",     # 31 boca
        "..............77784",      # 32
        ".............98884",       # 33
        "............9.444",        # 34 barbilla
        "............9",            # 35
    ]),
}

CARA_SERPIENTE = {  # nagas: ollares, boca ancha y capucha de cobra detras
    "abajo": simetrico(33, [
        ".........9",    # 33
        "..........",    # 34
        ".......444",    # 35
    ]),
    "derecha": (29, [
        "................4",       # 29
        "..............22224",     # 30
        "..............12294",     # 31
        "..............44444",     # 32
        "..............7774",      # 33
    ]),
}

CAPUCHA_COBRA = {  # detras de la cabeza (ranura "espalda"); 2 = cara interior
    "abajo": simetrico(20, [
        ".......4444",   # 20
        ".....447777",   # 21
        "....4767777",   # 22
        "...47677777",   # 23
        "..476777777",   # 24
        "..476777777",   # 25
        ".4767777777",   # 26
        ".4767777777",   # 27
        "47677777777",   # 28
        "47677777777",   # 29
        "47687777777",   # 30
        "47687777777",   # 31
        "47887777777",   # 32
        ".4787777777",   # 33
        ".4787777777",   # 34
        "..478777777",   # 35
        "...47877777",   # 36
        "....4877777",   # 37
    ], sombra_derecha=False),
    "arriba": simetrico(20, [
        ".......4444",   # 20
        ".....441222",   # 21
        "....4122222",   # 22
        "...41222222",   # 23
        "..412222222",   # 24
        "..412222322",   # 25
        ".4122223332",   # 26
        ".4122237732",   # 27
        "41222377732",   # 28
        "41222377732",   # 29
        "41222237732",   # 30
        "41222223332",   # 31
        "42222222322",   # 32
        ".4222222222",   # 33
        ".4222222222",   # 34
        "..432222222",   # 35
        "...43222222",   # 36
        "....4322222",   # 37
    ]),
    "derecha": (20, [
        ".....444",         # 20
        "....4224",         # 21
        "...42224",         # 22
        "...42224",         # 23
        "..422224",         # 24
        "..422224",         # 25
        ".4222224",         # 26
        ".4222224",         # 27
        ".4222224",         # 28
        ".4222224",         # 29
        ".422224",          # 30
        ".422224",          # 31
        "..42224",          # 32
        "..42224",          # 33
        "...4224",          # 34
        "...4224",          # 35
        "....44",           # 36
    ]),
}

# --- crestas -----------------------------------------------------------------

PLUMAS = {  # cresta de plumas de aarakocra (material 1 = plumas)
    "abajo": simetrico(16, [
        ".........4",    # 16
        "........42",    # 17
        "......4.421",   # 18
        "......42121",   # 19
        ".......4211",   # 20
    ]),
    "arriba": simetrico(16, [
        ".........4",
        "........42",
        "......4.421",
        "......42121",
        ".......4211",
    ]),
    "derecha": (17, [
        "..44",             # 17
        "...424.44",        # 18
        "....4214224",      # 19
        ".....422114",      # 20
        "......4221",       # 21
    ]),
}

CRESTA_ESPINAS = {  # lagartos: espinas por el centro de la cabeza
    "abajo": simetrico(18, [
        "..........9",   # 18
        ".........97",   # 19
        "..........9",   # 20
    ], sombra_derecha=False),
    "arriba": simetrico(18, [
        "..........9",
        ".........97",
        "..........9",
        ".........97",
        "..........9",
        ".........97",
        "..........9",
    ], sombra_derecha=False),
    "derecha": (17, [
        ".......9.9",       # 17
        "......97979",      # 18
        ".....9.99.9",      # 19
        "....97",           # 20
        "....9",            # 21
    ]),
}

NARIZ_GANCHUDA = {  # sagas
    "abajo": simetrico(31, [
        "..........2",   # 31
        "..........3",   # 32
        ".........34",   # 33
    ], sombra_derecha=False),
    "derecha": (28, [
        "................2",      # 28
        "................22",     # 29
        "................223",    # 30
        "................2234",   # 31
        ".................44",    # 32
    ]),
}

# --- barba (ranura "barba": sobre la ropa del pecho, bajo el pelo) ----------

BARBA = {
    "abajo": simetrico(30, [
        "...42",         # 30
        "....42",        # 31
        "....422",       # 32
        "...42212333",   # 33
        "...42122223",   # 34
        "...42212122",   # 35
        "....4212122",   # 36
        "....4221212",   # 37
        ".....421222",   # 38
        "......42122",   # 39
        ".......4212",   # 40
        "........422",   # 41
        ".........44",   # 42
    ]),
    "derecha": (29, [
        ".........42",           # 29
        ".........42",           # 30
        ".........42212233",     # 31
        ".........422122234",    # 32
        ".........4221222234",   # 33
        "..........42212234",    # 34
        "..........4221234",     # 35
        "...........422134",     # 36
        "...........42234",      # 37
        "............4234",      # 38
        ".............434",      # 39
        "..............4",       # 40
    ]),
}

BARBA_CORTA = {
    "abajo": simetrico(30, [
        "...42",         # 30
        "....42",        # 31
        "....422",       # 32
        "....4221333",   # 33
        ".....421223",   # 34
        "......42122",   # 35
        ".......4444",   # 36
    ]),
    "derecha": (29, [
        ".........42",           # 29
        ".........42",           # 30
        ".........42212233",     # 31
        ".........422122234",    # 32
        "..........42212234",    # 33
        "...........444444",     # 34
    ]),
}


# --- la corrupcion de los drows (PNJ-02): musgo, corteza, piedras y astas --------------
from actores_corrupcion import (ASTAS_RAMAS, ASTAS_RETORCIDAS, CORONA_PIEDRAS,  # noqa: E402,F401
                                CORTEZA_CARA, CORTEZA_MANDIBULA, CUERNOS_CARNERO, MUSGO_HOMBROS,
                                PLACAS_PIEDRA, TERCER_OJO)


# --- marcas de Egaroth ----------------------------------------------------------------------
# Oriol (23/10): a las razas que sin ropa eran iguales que otra, «alguna marca racial diferencial, como manchas
# de diferentes colores, tocadas por la energía de Egaroth, el planeta / padre elemental». No es una plantilla
# de cabeza: son manchas sobre las partes del cuerpo (actores_personajes.capa_manchas), en el sitio relativo de
# cada parte, así que siguen al cuerpo en todos los frames y tallas. (parte, x, y, tamaño, material):
# x e y de 0 a 1 dentro de la parte; tamaño 1 una mancha pequeña, 2 una grande; material 1 o 2.
MANCHAS_EGAROTH_PIEDRA = [  # enanos historiadores: dormienta (gris azul) y ámbar, como vetas de mena
    ("h", 0.22, 0.62, 1, 1), ("h", 0.78, 0.35, 1, 2), ("t", 0.30, 0.25, 2, 1), ("t", 0.72, 0.62, 1, 2),
    ("a", 0.50, 0.40, 1, 2), ("A", 0.50, 0.70, 1, 1), ("l", 0.50, 0.45, 1, 1), ("L", 0.50, 0.30, 2, 2),
]
# Oriol (23/10): «la marca racial diferencial... alguns més fosques, altres com si hi hagués una gema negra
# i obscura com l'obsidiana». Partes: h cabeza, n cuello, t torso, p cadera, a/A brazos, m/M manos,
# l/L piernas; (parte, x relativa, y relativa, tamaño 1|2, material 1|2).
MANCHAS_EGAROTH_SOL = [  # pecas de luz en la cara y los hombros
    ("h", 0.30, 0.62, 1, 1), ("h", 0.70, 0.62, 1, 1), ("h", 0.50, 0.70, 1, 2), ("t", 0.20, 0.10, 1, 1),
    ("t", 0.80, 0.10, 1, 2), ("a", 0.50, 0.15, 1, 1), ("A", 0.50, 0.15, 1, 2), ("t", 0.50, 0.30, 1, 1),
]
MANCHAS_EGAROTH_ESTIGMAS = [  # en el dorso de las manos y en la frente: la marca de la Proclama
    ("m", 0.50, 0.50, 1, 1), ("M", 0.50, 0.50, 1, 1), ("h", 0.50, 0.45, 1, 2), ("t", 0.50, 0.25, 2, 2),
]
MANCHAS_EGAROTH_VETAS = [  # gotas de tinta en fila por los brazos y el cuello
    ("a", 0.50, 0.20, 1, 1), ("a", 0.50, 0.50, 1, 2), ("a", 0.50, 0.80, 1, 1), ("A", 0.50, 0.20, 1, 1),
    ("A", 0.50, 0.50, 1, 2), ("A", 0.50, 0.80, 1, 1), ("n", 0.30, 0.50, 1, 2), ("h", 0.20, 0.75, 1, 1),
]
MANCHAS_EGAROTH_MOTAS = [  # muchas motas pequeñas por todo el cuerpo
    ("h", 0.25, 0.70, 1, 1), ("h", 0.75, 0.40, 1, 2), ("t", 0.25, 0.20, 1, 2), ("t", 0.70, 0.45, 1, 1),
    ("t", 0.40, 0.80, 1, 2), ("a", 0.50, 0.30, 1, 1), ("A", 0.50, 0.65, 1, 2), ("m", 0.50, 0.50, 1, 1),
    ("l", 0.50, 0.30, 1, 2), ("L", 0.50, 0.60, 1, 1), ("p", 0.30, 0.50, 1, 1),
]
MANCHAS_EGAROTH_LEOPARDO = [  # rosetas en el pelaje
    ("h", 0.20, 0.30, 1, 1), ("h", 0.80, 0.30, 1, 1), ("t", 0.20, 0.20, 2, 1), ("t", 0.70, 0.30, 1, 2),
    ("t", 0.35, 0.65, 2, 1), ("t", 0.75, 0.80, 1, 1), ("a", 0.50, 0.30, 1, 1), ("a", 0.50, 0.75, 1, 2),
    ("A", 0.50, 0.40, 1, 1), ("l", 0.50, 0.25, 2, 1), ("L", 0.50, 0.55, 1, 2), ("p", 0.70, 0.40, 1, 1),
]
MANCHAS_EGAROTH_GRIETAS = [  # grietas de lava en la obsidiana: grandes y encendidas
    ("t", 0.30, 0.25, 2, 1), ("t", 0.65, 0.60, 2, 2), ("h", 0.70, 0.30, 1, 1), ("a", 0.50, 0.40, 2, 1),
    ("A", 0.50, 0.60, 1, 2), ("l", 0.50, 0.40, 1, 1), ("L", 0.50, 0.30, 2, 2), ("p", 0.50, 0.50, 1, 1),
]
MANCHAS_OBSIDIANA = [  # esquirlas de obsidiana que asoman en la piel (material 1: la gema, 2: su brillo)
    ("a", 0.50, 0.25, 2, 1), ("A", 0.50, 0.25, 2, 1), ("m", 0.50, 0.40, 1, 1), ("M", 0.50, 0.40, 1, 1),
    ("h", 0.80, 0.35, 1, 2), ("t", 0.50, 0.20, 2, 1), ("t", 0.30, 0.70, 1, 2),
]
MANCHAS_EGAROTH_CONSTELACION = [  # pocas, sueltas y brillantes, como estrellas
    ("h", 0.80, 0.70, 1, 1), ("t", 0.30, 0.35, 1, 2), ("t", 0.75, 0.75, 1, 1), ("a", 0.50, 0.60, 1, 2),
    ("L", 0.50, 0.45, 1, 1), ("M", 0.50, 0.50, 1, 2),
]
MANCHAS_EGAROTH_FUEGO_AGUA = [  # genasi del Cónclave (1 de 2): fuego y agua
    ("h", 0.25, 0.65, 1, 1), ("t", 0.30, 0.25, 2, 1), ("a", 0.50, 0.45, 1, 1), ("t", 0.75, 0.70, 2, 2),
    ("A", 0.50, 0.45, 1, 2), ("L", 0.50, 0.40, 1, 2),
]
MANCHAS_EGAROTH_TIERRA_AIRE = [  # genasi del Cónclave (2 de 2): tierra y aire
    ("h", 0.75, 0.65, 1, 1), ("t", 0.70, 0.25, 1, 1), ("l", 0.50, 0.40, 2, 1), ("t", 0.30, 0.70, 1, 2),
    ("m", 0.50, 0.50, 1, 2), ("n", 0.50, 0.50, 1, 1),
]
# la gema negra en la frente (material 1: la obsidiana, 2: el brillo del corazón)
GEMA_FRENTE = {
    "abajo": (25, ["..........4", ".........414", "........43w34", ".........434", "..........4"]),
    "derecha": (25, ["...............4", "..............41", "..............3w", "..............4"]),
}

MANCHAS_EGAROTH_AGUA = [  # nobles de Ascaria: turquesa y violeta, como reflejos de agua
    ("h", 0.70, 0.70, 1, 1), ("n", 0.50, 0.50, 1, 2), ("t", 0.65, 0.30, 2, 2), ("t", 0.25, 0.70, 1, 1),
    ("a", 0.50, 0.75, 1, 1), ("A", 0.50, 0.35, 2, 2), ("p", 0.40, 0.50, 1, 1), ("l", 0.50, 0.70, 1, 2),
]
