"""Config del cuadrante ESTE-NORTE de Egaroth (experimento 4-IAs, ZCode).

Unica fuente de verdad de mi trabajo: los 22 asentimientos que genero,
con su nacion, nombre (canonico / placeholder / propuesta), tier y
posicion en el mapamundi cuando la tiene.

Tiers -> tamano del mapa y numero de locales:
    aldea    32x32, 0 locales
    pueblo   40x40, 1 local  (taberna)
    ciudad   48x48, 2 locales (taberna, mercado)
    capital  64x64, 4 locales (taberna, mercado, herreria + uno propio)

Marcadores: 14 asentimientos ya tienen objeto-ciudad en mundi_landmass_2
(13) y _7 (Venordemn). Los otros 8 (Qethatos + 6 de Mistarium + la
capital propuesta de Nocturnsea) no existen en ningun landmass:
wire_este_norte.py los anade en un ancla dada, buscando la celda
transitable mas cercana.

Todo ASCII en nombres (el parser JSON del motor no decodifica \\uXXXX).
"""

# (slug, nacion, nombre_display, tier, landmass, pos_o_ancla)
#   landmass: None = no hay marcador (wire lo crea en el ancla indicada)
#   pos_o_ancla: {"x":..,"y":..} del marcador existente, o ancla donde
#                wire debe COLOCAR el marcador nuevo.
ASENTAMIENTOS = [
    # --- Gongorguma (orcos del este; Guskedor y Qethatos con nombre canonico) ---
    ("guskedor",  "gongorguma", "Guskedor",  "capital", 2, (35, 36)),
    ("qethatos",  "gongorguma", "Qethatos",  "ciudad",  2, (27, 28)),   # sin marcador: ancla
    ("ciudad_xiii", "gongorguma", "Ciudad XIII", "ciudad", 2, (19, 20)),
    ("ciudad_xiv",  "gongorguma", "Ciudad XIV",  "ciudad", 2, (51, 44)),
    ("ciudad_xv",   "gongorguma", "Ciudad XV",   "ciudad", 2, (51, 20)),
    ("pueblo_vi",   "gongorguma", "Pueblo VI",   "pueblo",  2, (19, 28)),
    ("pueblo_vii",  "gongorguma", "Pueblo VII",  "pueblo",  2, (27, 44)),
    ("aldea_xvi",   "gongorguma", "Aldea XVI",   "aldea",   2, (51, 28)),
    ("aldea_xvii",  "gongorguma", "Aldea XVII",  "aldea",   2, (43, 44)),

    # --- Choubar (mercantes fluviales; Klimnebra con nombre canonico) ---
    ("klimnebra",  "choubar", "Klimnebra", "capital", 2, (35, 52)),
    ("aldea_xviii","choubar", "Aldea XVIII", "aldea",  2, (27, 52)),
    ("aldea_xix",  "choubar", "Aldea XIX",   "aldea",  2, (36, 52)),
    ("aldea_xx",   "choubar", "Aldea XX",    "aldea",  2, (43, 52)),
    ("aldea_xxi",  "choubar", "Aldea XXI",   "aldea",  2, (51, 60)),

    # --- Mistarium (islas de la bruma; Venordemn con nombre canonico) ---
    # landmass_8 es 8x16 y todo transitable: anclas dentro.
    ("venordemn",  "mistarium", "Venordemn", "capital", 7, (3, 12)),
    ("ciudad_m",   "mistarium", "Ciudad",     "ciudad", 8, (2, 4)),    # sin marcador
    ("ciudad_m_i", "mistarium", "Ciudad I",   "ciudad", 8, (5, 5)),    # sin marcador
    ("pueblo_m",   "mistarium", "Pueblo",     "pueblo",  8, (2, 8)),   # sin marcador
    ("pueblo_m_i", "mistarium", "Pueblo I",   "pueblo",  8, (5, 9)),   # sin marcador
    ("pueblo_m_ii","mistarium", "Pueblo II",  "pueblo",  8, (2, 12)),  # sin marcador
    ("aldea_m",    "mistarium", "Aldea",      "aldea",   8, (5, 12)),  # sin marcador

    # --- Nocturnsea (mar nocturno; SIN asentimientos en geografia.json) ---
    # La capital canonica de Nocturnsea es la "Ciudad de Grytoz" (solo
    # nombre, en indice_lugares.md; ausente de geografia.json). Este
    # experimento la materializa como nivel. La propuesta anterior
    # ("Umbrahal") queda RETIRADA al descubrir el nombre canonico.
    # landmass_4 es 32x16 todo transitable: ancla dentro.
    ("grytoz",     "nocturnsea", "Grytoz", "capital", 4, (16, 8)),   # sin marcador canonico
]

# Ids de ciudad del canon (geografia.json) por slug, para que wire pueda
# engancharse al objeto existente del landmass cuando lo hay. Los que no
# estan, wire los crea con el id de la izquierda.
ID_CANON = {
    "guskedor": "ciudad_mrnu4asi7ixs",
    "klimnebra": "ciudad_mrnu7hq9u9sc",
    "ciudad_xiii": "ciudad_mrnv156p3ctn",
    "ciudad_xiv": "ciudad_mrnv0vk8guc6",
    "ciudad_xv": "ciudad_mrnuzrasknq6",
    "pueblo_vi": "ciudad_mrnv1t6vnqic",
    "pueblo_vii": "ciudad_mrnv26sszhqw",
    "aldea_xvi": "ciudad_mrnv2hkbov1q",
    "aldea_xvii": "ciudad_mrnv3mg7qqjy",
    "aldea_xviii": "ciudad_mrnv42ea1w7o",
    "aldea_xix": "ciudad_mrnv4guj5ol8",
    "aldea_xx": "ciudad_mrnv4pyiji2a",
    "aldea_xxi": "ciudad_mrnv4zdbbeba",
    "venordemn": "ciudad_mrnu58hdc33s",
}

TAMANOS = {"aldea": (32, 32), "pueblo": (40, 40), "ciudad": (48, 48), "capital": (64, 64)}

# Locales por tier. El 4o local de cada capital es propio de la nacion.
LOCALES_POR_TIER = {
    "aldea": [],
    "pueblo": ["taberna"],
    "ciudad": ["taberna", "mercado"],
    "capital": ["taberna", "mercado", "herreria", "distinctivo"],
}

# El local "distinctivo" de cada capital y su titulo (nombre del interior).
DISTINTIVO = {
    "guskedor":  ("arena",    "Arena de clanes"),
    "klimnebra": ("banco",    "Casa de fletamento"),
    "venordemn": ("biblioteca","Archivo de la Bruma"),
    "grytoz":     ("posada",   "Posada del Faro"),
}

NACIONES = {
    "gongorguma": {"nombre": "Gongorguma", "adj": "de los clanes orcos"},
    "choubar":    {"nombre": "Choubar",    "adj": "de los senores del rio"},
    "mistarium":  {"nombre": "Mistarium",  "adj": "de las islas de bruma"},
    "nocturnsea": {"nombre": "Nocturnsea", "adj": "del mar nocturno"},
}

def seed_de(slug: str) -> int:
    """Seed determinista por asentimiento: mismo slug, mismo mapa."""
    return 20260815 + (zlib_crc(slug) % 100000)

def zlib_crc(s: str) -> int:
    import zlib
    return zlib.crc32(s.encode("ascii"))

def nivel_exterior(slug: str) -> str:
    return f"ciudad_en_{slug}"

def nivel_interior(slug: str, local: str) -> str:
    return f"interior_en_{slug}_{local}"
