"""Del catalogo de objetos (equipment.json) a las piezas visibles del equipo.

Los 413 objetos del catalogo no tienen dibujo propio: cada uno se viste con
una pieza de las familias de actores_equipo, actores_cascos y
actores_armas, elegida por palabras de su nombre. Las reglas van en orden:
gana la primera que casa, y dentro de ella el primer calificativo (color o
material) que aparezca. Un objeto sin regla, o de los que no se ven puestos
(anillos, amuletos), queda sin pieza y el generador lo lista.
"""
import json
import re
import unicodedata
from pathlib import Path


def normalizar(texto):
    t = unicodedata.normalize("NFD", str(texto).lower())
    return "".join(c for c in t if unicodedata.category(c) != "Mn")


def _casa(clave, nombre_normalizado):
    """La clave empieza en un limite de palabra: 'aro' casa con 'aro de plata'
    pero no con 'claro'; 'roj' casa con 'roja' y con 'rojo'. Con '$' al final
    tiene que ser la palabra entera ('chal$' no casa con 'chaleco')."""
    entera = clave.endswith("$")
    patron = r"(?<![a-z0-9])" + re.escape(normalizar(clave.rstrip("$")))
    if entera:
        patron += r"(?![a-z0-9])"
    return re.search(patron, nombre_normalizado) is not None


SIN_PIEZA = ("anillo", "amuleto", "collar", "colgante", "pendiente", "brazalete", "lentes",
             "monoculo", "gafas", "talisman", "reliquia", "cinturon", "faja")

# (palabras, ranura, pieza por defecto, [(calificativos, pieza)])
REGLAS = [
    # cabeza
    (("yelmo",), "cabeza", "yelmo", [(("ceniza", "negr", "oscur", "sombr"), "yelmo_oscuro")]),
    (("chapel", "capacete", "almofar"), "cabeza", "chapel", []),
    (("casco", "bacinete", "morrion"), "cabeza", "bacinete", [(("cuerno", "vikingo"), "casco_cuernos")]),
    (("capucha", "capuchon", "caperuza"), "cabeza", "capucha_parda",
     [(("negr", "sombr"), "capucha_negra"), (("gris",), "capucha_gris"), (("verde",), "capucha_verde"),
      (("roj", "sangr"), "capucha_roja"), (("purpura", "morad", "culto"), "capucha_purpura")]),
    (("corona", "laurel"), "cabeza", "corona",
     [(("rama", "laurel", "flor", "hoja", "espino"), "corona_ramas")]),
    (("diadema", "tiara", "aro", "circulo"), "cabeza", "diadema", []),
    (("cimera",), "cabeza", "yelmo", []),
    (("sombrero", "gorro", "chambergo", "capirote", "ala ancha"), "cabeza", "sombrero_cuero",
     [(("mago", "brujo", "hechicer", "arcan", "estrell"), "sombrero_mago_azul"),
      (("paja", "campesin"), "sombrero_paja"), (("pluma", "cazador", "arquero"), "sombrero_pluma"),
      (("bufon", "cascabel"), "gorro_bufon")]),
    (("panuelo", "bandana", "toca"), "cabeza", "panuelo_rojo", [(("azul",), "panuelo_azul")]),
    (("turbante",), "cabeza", "turbante", [(("purpura", "morad"), "turbante_purpura")]),
    (("mascara",), "cabeza", "mascara_porcelana", []),
    (("antifaz",), "cabeza", "antifaz", []),
    (("testuz",), "cabeza", "casco_cuernos", []),
    # torso
    (("placas", "peto", "coraza", "arnes", "armadura completa", "armadura pesada"), "torso",
     "placas_acero",
     [(("oro", "dorad", "solar"), "placas_oro")]),
    (("cota de malla", "malla", "loriga", "cota", "armadura media"), "torso", "cota_malla", []),
    (("escamas",), "torso", "escamas_acero", [(("bronce", "cobre", "dragon"), "escamas_bronce")]),
    (("brigantina",), "torso", "cuero_oscuro", []),
    (("sobrevesta", "tabardo"), "torso", "sobrevesta_azul",
     [(("roj",), "sobrevesta_roja"), (("blanc", "luz"), "sobrevesta_blanca")]),
    (("gambeson", "acolchad", "jubon"), "torso", "gambeson", []),
    (("cuero", "piel curtida", "tachonad", "armadura ligera", "equipo ligero", "pellejo"), "torso",
     "cuero", [(("negr", "oscur", "sombr"), "cuero_oscuro")]),
    (("gaban", "sobretodo", "casaca", "levita"), "torso", "camisa_chaleco_oscuro", []),
    (("sobrepelliz", "alba"), "torso", "tunica_sacerdote", []),
    (("sayo",), "torso", "tunica_parda", []),
    (("chal$", "melena"), "capa", "capa_corta_roja", []),
    (("capa", "manto", "esclavina"), "capa", "capa_parda",
     [(("roj", "sangr"), "capa_roja"), (("verde", "bosque"), "capa_verde"), (("azul",), "capa_azul"),
      (("negr", "sombr", "noche"), "capa_negra"), (("purpura", "morad"), "capa_purpura")]),
    (("tunica", "habito", "sotana", "toga", "vestidura"), "torso", "tunica_parda",
     [(("mago", "hechicer", "arcan", "brujo", "estrell"), "tunica_mago"),
      (("sacerdot", "clerig", "blanc", "luz", "sagrad"), "tunica_sacerdote"),
      (("monje", "penitent"), "habito_monje"), (("culto", "purpura", "morad"), "habito_purpura"),
      (("gris", "carceler"), "habito_gris"), (("verde",), "tunica_verde"), (("azul",), "tunica_azul"),
      (("roj",), "tunica_roja"), (("mercader", "seda"), "tunica_mercader")]),
    (("kimono",), "torso", "kimono_negro", [(("blanc", "viento"), "kimono_blanco")]),
    (("vestido", "saya", "gown"), "torso", "vestido_teja",
     [(("noble", "seda", "terciopelo", "gala"), "vestido_noble"), (("verde",), "vestido_verde"),
      (("delantal", "tabern"), "vestido_tabernera"), (("lino", "blanc"), "vestido_lino")]),
    (("corse", "corpino", "blusa"), "torso", "blusa_corse", []),
    (("chaleco", "camisa", "camison", "delantal"), "torso", "camisa_chaleco",
     [(("oscur", "negr"), "camisa_chaleco_oscuro")]),
    (("harapo", "andrajo", "arpillera"), "torso", "harapos", []),
    (("bufon", "arlequin"), "torso", "bufon", []),
    # piernas
    (("quijote", "grebas runicas", "grebas de placas", "grebas de acero"), "piernas", "quijotes", []),
    (("grebas", "perneras", "espinilleras"), "piernas", "grebas_cuero",
     [(("malla",), "grebas_malla"), (("runic", "placa", "acero"), "quijotes")]),
    (("calzas", "medias"), "piernas", "calzas_verdes", [(("roj",), "calzas_rojas")]),
    (("falda", "faldon", "faldar", "enaguas"), "piernas", "falda_parda",
     [(("roj",), "falda_roja"), (("larga", "azul"), "falda_larga_azul"),
      (("malla",), "grebas_malla")]),
    (("pantalon", "calzon", "bragas", "zaragüelles", "zaraguelles"), "piernas", "pantalon_pardo",
     [(("gris",), "pantalon_gris"), (("negr",), "pantalon_negro"), (("azul",), "pantalon_azul"),
      (("oliva", "verde"), "pantalon_oliva")]),
    # pies
    (("escarpe",), "pies", "escarpes", []),
    (("botas", "bota"), "pies", "botas",
     [(("alta", "montar", "caballer"), "botas_altas"), (("hierro", "herrad", "acero"), "botas_acero"),
      (("clar", "gamuza"), "botas_claras")]),
    (("botin", "zapato"), "pies", "zapatos", []),
    (("sandalia", "alpargata"), "pies", "sandalias", []),
    (("zueco", "almadrena"), "pies", "zuecos", []),
    (("pantufla", "zapatilla", "babucha", "escarpin"), "pies", "pantuflas", []),
    (("pezuna", "herradura"), "pies", "botas_acero", []),
    # manos
    (("escudo", "broquel", "rodela", "pavés", "paves"), "mano_izq", "escudo_madera",
     [(("acero", "hierro", "torre"), "escudo_acero"), (("oro", "dorad", "solar", "luz"), "escudo_oro")]),
    (("grimorio", "libro", "tomo", "codice"), "mano_izq", "grimorio", []),
    (("orbe", "esfera", "foco"), "mano_izq", "orbe", []),
    (("farol", "linterna", "antorcha", "lampara", "tea"), "mano_izq", "farol", []),
    (("espejo", "fragmento de cielo", "ovillo"), "mano_izq", "orbe", []),
    (("pocion", "frasco", "vial"), "mano_izq", "pocion", []),
    (("tridente",), "arma", "tridente", []),
    (("lanza", "pica", "alabarda", "guadana", "venablo", "jabalina", "astil"), "arma", "lanza_hierro",
     [(("bronce",), "lanza_bronce")]),
    (("baston", "baculo", "vara", "cetro", "cayado", "bordon"), "arma", "baston_roble",
     [(("cristal", "arcan", "mago", "cetro", "estrell", "hielo"), "baston_cristal")]),
    (("arco", "ballesta"), "arma", "arco_roble", [(("negr", "oscur", "sombr", "ceniza"), "arco_ceniza")]),
    (("martillo", "mazo", "yunque"), "arma", "martillo_acero", []),
    (("maza", "mangual", "lucero", "garrote", "porra", "macana", "badajo", "campana"), "arma",
     "maza_hierro", []),
    (("hacha", "segur", "hoz"), "arma", "hacha_hierro",
     [(("ceniza", "negr", "oscur", "sombr"), "hacha_ceniza"), (("acero",), "hacha_acero")]),
    (("daga", "cuchillo", "punal", "navaja", "estilete", "colmillo", "aguja", "alfiler", "zarpa"),
     "arma", "daga_hierro",
     [(("plata", "elfic"), "daga_plata")]),
    (("espada", "espadon", "sable", "mandoble", "estoque", "cimitarra", "alfanje", "montante",
      "katana", "hoja"),
     "arma", "espada_hierro",
     [(("cristal", "hielo", "escarcha"), "espada_cristal"), (("oro", "dorad", "solar", "luz"), "espada_oro"),
      (("ceniza", "negr", "oscur", "sombr", "sangr"), "espada_ceniza"), (("acero",), "espada_acero")]),
]


# ranura del catalogo (equipment.json) -> ranuras visibles donde puede caer
RANURAS_DE_SLOT = {"head": ("cabeza",), "torso": ("torso", "capa"), "legs": ("piernas",),
                   "feet": ("pies",), "mainhand": ("arma", "mano_izq"), "offhand": ("mano_izq",)}


def pieza_de(nombre, slot=None):
    """(ranura, pieza) para un nombre de objeto, o None si no se ve puesto.

    slot (Head, Torso, Legs, Feet, MainHand) restringe las reglas: un
    'faldon de malla' de Legs es una pieza de piernas, no una cota."""
    n = normalizar(nombre)
    if any(_casa(p, n) for p in SIN_PIEZA):
        return None
    validas = RANURAS_DE_SLOT.get(normalizar(slot or "").replace("_", "").replace(" ", ""))
    for palabras, ranura, defecto, calificativos in REGLAS:
        if validas and ranura not in validas:
            continue
        if any(_casa(p, n) for p in palabras):
            for claves, pieza in calificativos:
                if any(_casa(c, n) for c in claves):
                    return ranura, pieza
            return ranura, defecto
    return None


# Si el nombre no dice nada ("La Impagable"), las etiquetas y el peso del objeto
TORSO_POR_PESO = {"light": "cuero", "medium": "cota_malla", "heavy": "placas_acero"}
ARMA_POR_ETIQUETA = [("Proyectil", "arma", "arco_roble"), ("Escudo", "mano_izq", "escudo_madera"),
                     ("Arma Pesada", "arma", "martillo_acero"), ("Arma Media", "arma", "espada_hierro"),
                     ("Arma Ligera", "arma", "daga_hierro")]


def pieza_de_objeto(o):
    """(ranura, pieza) de un objeto de equipment.json: por nombre, y si no,
    por etiquetas (grantedTags) y peso (weightCategory)."""
    nombre = o.get("name") or o.get("nombre") or o.get("id") or ""
    slot = normalizar(o.get("slot") or "")
    if any(_casa(p, normalizar(nombre)) for p in SIN_PIEZA):
        return None
    if slot in ("ring", "ring2", "neck", "belt", "accessory"):  # PART-02: abalorios, no se ven puestos
        return None
    res = pieza_de(nombre, o.get("slot"))
    if res is not None:
        return res
    etiquetas = set(o.get("grantedTags") or [])
    if slot == "mainhand":
        for etiqueta, ranura, pieza in ARMA_POR_ETIQUETA:
            if etiqueta in etiquetas:
                oscuro = "Oscuro" in etiquetas and pieza in ("espada_hierro", "arco_roble")
                return ranura, {"espada_hierro": "espada_ceniza", "arco_roble": "arco_ceniza"}[pieza] \
                    if oscuro else pieza
        return None
    if slot == "torso":
        return "torso", TORSO_POR_PESO.get(o.get("weightCategory"), "tunica_parda")
    if slot == "feet":
        return "pies", "botas"
    if slot == "legs":
        return "piernas", "pantalon_pardo"
    return None


def _objetos(datos):
    """Lista de objetos de equipment.json, sea cual sea el envoltorio."""
    if isinstance(datos, list):
        return [o for o in datos if isinstance(o, dict)]
    if isinstance(datos, dict):
        for clave in ("items", "equipment", "objetos", "equipo", "entries"):
            if isinstance(datos.get(clave), list):
                return _objetos(datos[clave])
        return [dict(v, id=v.get("id", k)) for k, v in datos.items() if isinstance(v, dict)]
    return []


def mapear(ruta_equipment):
    """{id: {ranura, pieza, nombre, slot}} y la lista de objetos sin pieza."""
    datos = json.loads(Path(ruta_equipment).read_text(encoding="utf-8"))
    mapa, sin = {}, []
    for o in _objetos(datos):
        ident = o.get("id") or o.get("itemId") or o.get("name")
        nombre = o.get("name") or o.get("nombre") or ident
        res = pieza_de_objeto(o)
        if res is None:
            sin.append({"id": ident, "nombre": nombre, "slot": o.get("slot")})
            continue
        mapa[ident] = {"nombre": nombre, "slot": o.get("slot"), "ranura": res[0], "pieza": res[1]}
    return mapa, sin
