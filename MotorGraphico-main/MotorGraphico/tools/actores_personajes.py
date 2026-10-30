"""Personajes completos: una raza, un pelo y un equipo, apilados en capas.

Un personaje es un dato:

    {"raza": "race_human_marches",            # cuerpo, talla y rasgos
     "piel": "piel_morena", "ojos": "verde",  # opcional: cambia los de la raza
     "pelo": ("largo", "pelo_negro"),         # None = calvo; sin clave, el de la raza
     "barba": ("BARBA_CORTA", "pelo_negro"),  # opcional
     "quitar_capas": ["morro"],               # opcional: quita capas de la raza
     "equipo": {"cabeza": "chapel", "torso": "sobrevesta_azul", "piernas": ...,
                "pies": ..., "capa": ..., "arma": ..., "mano_izq": ...}}

capas_personaje() devuelve la lista de capas (ranura, 12 frames) que el
motor apilaria con actores_capas.ORDEN, y componer_personaje() las junta.
Las ranuras que la raza no admite (pelo en una cabeza de animal, botas en
una pezuna, piernas en una naga) se ignoran.
"""
import actores_alas as alas
import actores_oficios  # noqa: F401  (PNJ-01: registra las prendas de oficio y la ropa nueva)
import actores_trajes  # noqa: F401  (SP-01: registra los trajes de especialista, al final de cada atlas)
import actores_armas as armas
import actores_cascos as cascos
import actores_colas as colas
import actores_cuerpos as cuerpos
import actores_equipo as equipo
import actores_pelo as pelo
import actores_rasgos as rasgos
import actores_razas as razas
from actores_capas import capa_cuerpo, capa_ojos, capa_plantilla, componer
from actores_materiales import MATERIALES as MAT
from actores_materiales import OJOS

RANURAS_EQUIPO = ("cabeza", "torso", "piernas", "pies", "capa", "arma", "mano_izq")


def piel_de(piel):
    if isinstance(piel, dict):
        return {k: MAT[v] for k, v in piel.items()}
    return MAT[piel]


def capas_raza(raza, talla, piel=None, ojos=None, quitar=()):
    """Capas del cuerpo de una raza: cuerpo, rasgos, cola y alas.

    quitar: ranuras de la raza que no se dibujan (la naga de historia tiene
    cara y pelo de persona: sin 'morro' ni 'espalda')."""
    piel_m = piel_de(piel or raza["piel"])
    ojos_d = OJOS[ojos or raza["ojos"]]
    capas = []
    cuerpo = raza.get("cuerpo")
    if cuerpo == "espectro":
        capas.append(("cuerpo", cuerpos.capa_espectro(talla, piel_m, ojos_d)))
    elif cuerpo:
        _tipo, escamas, vientre = cuerpo
        capas.append(("cuerpo", cuerpos.capa_naga(talla, piel_m, MAT[escamas], MAT[vientre], ojos_d)))
    else:
        capas.append(("cuerpo", capa_cuerpo(talla, piel_m, ojos_d)))
    for ranura, nombre, m1, m2 in raza.get("rasgos", []):
        if ranura in quitar:
            continue
        capas.append((ranura, capa_plantilla(talla, getattr(rasgos, nombre), MAT[m1],
                                             MAT[m2] if m2 else None, sobre_cuerpo=False)))
    cola = raza.get("cola")
    if cola:
        tipo, m1, m2 = cola
        if tipo == "kitsune":
            capas.append(("cola", colas.capa_colas_kitsune(talla, MAT[m1], MAT[m2])))
        else:
            capas.append(("cola", colas.capa_cola(talla, tipo, MAT[m1], MAT[m2] if m2 else None)))
    if raza.get("alas"):
        tipo, m1, m2 = raza["alas"]
        capas.append(("alas", alas.capa_alas(talla, tipo, MAT[m1], MAT[m2])))
    return capas


# Las capas de pelo, barba y equipo solo dependen de (talla, pieza): se
# guardan para no pintarlas otra vez al montar cada personaje. Nadie las
# modifica despues (componer() y los atlas copian los pixeles).
_CACHE = {}


def _guardada(clave, fn):
    if clave not in _CACHE:
        _CACHE[clave] = fn()
    return _CACHE[clave]


def capa_pelo(talla, estilo, color):
    return _guardada(("pelo", id(talla), estilo, color), lambda: capa_plantilla(
        talla, pelo.ESTILOS[estilo], MAT[color], None, sobre_cuerpo=False))


def capa_barba(talla, estilo, color):
    return _guardada(("barba", id(talla), estilo, color), lambda: capa_plantilla(
        talla, getattr(rasgos, estilo), MAT[color], None, sobre_cuerpo=False))


def capa_equipo(talla, ranura, ident, talla_ropa=None):
    """(ranura, frames) de una pieza de equipo."""
    if ranura == "cabeza":
        return ranura, _guardada(("cabeza", id(talla), ident),
                                 lambda: cascos.capa_cabeza(talla, ident, MAT))
    if ranura in ("arma", "mano_izq"):
        r, frames = _guardada(("mano", id(talla), ident), lambda: armas.capa_arma(talla, ident, MAT))
        # SP-02: los guanteletes se apilan con su orden propio (encima del cuerpo tambien de espaldas)
        return armas.orden_de(ident) or r, frames
    t = talla_ropa or talla
    return ranura, _guardada((ranura, id(t), ident), lambda: equipo.capa_pieza(t, ranura, ident, MAT))


def cambia_cuerpo(spec, raza):
    """True si el personaje no usa el cuerpo de su raza tal cual (otra piel u
    otra talla): entonces sale del atlas de cuerpos, que solo tiene pieles
    lisas, asi que la raza no puede tener rasgos, cola, alas ni cuerpo especial."""
    otra_talla = spec.get("talla", raza["talla"]) != raza["talla"]
    otra_piel = "piel" in spec and spec["piel"] != raza["piel"]
    if not (otra_talla or otra_piel):
        return False
    if (raza.get("cuerpo") or raza.get("rasgos") or raza.get("cola") or raza.get("alas")
            or isinstance(raza["piel"], dict)):
        raise ValueError(f"{spec.get('nombre', spec['raza'])}: la raza {raza['id']} tiene rasgos "
                         "propios y no admite otra piel ni otra talla")
    return True


def capas_personaje(spec, tallas):
    raza = razas.POR_ID[spec["raza"]]
    talla = tallas[spec.get("talla", raza["talla"])]
    sin = set(raza.get("sin", []))
    ojos = spec.get("ojos", raza["ojos"])
    if cambia_cuerpo(spec, raza):
        capas = [("cuerpo", capa_cuerpo(talla, MAT[spec.get("piel", raza["piel"])]))]
        if ojos != "azul":  # el cuerpo del atlas de cuerpos lleva los ojos del cuerpo base
            capas.append(("ojos", capa_ojos(talla, OJOS[ojos])))
    else:
        capas = capas_raza(raza, talla, quitar=spec.get("quitar_capas", ()))
        if ojos != raza["ojos"]:
            capas.insert(1, ("ojos", capa_ojos(talla, OJOS[ojos])))
    talla_ropa = talla.sin_piernas() if raza.get("cuerpo") else talla
    barba = spec.get("barba", raza.get("barba"))
    if barba:
        capas.append(("barba", capa_barba(talla, *barba)))
    eq = spec.get("equipo", {})
    cabeza = eq.get("cabeza")
    oculta = bool(cabeza) and cascos.CABEZA[cabeza]["oculta_pelo"]
    estilo = spec["pelo"] if "pelo" in spec else raza.get("pelo")
    if estilo and not oculta and ("pelo" not in sin or "pelo" in spec):
        capas.append(("pelo", capa_pelo(talla, *estilo)))
    for ranura in RANURAS_EQUIPO:
        ident = eq.get(ranura)
        if ident and ranura not in sin:
            capas.append(capa_equipo(talla, ranura, ident, talla_ropa))
    return capas


def componer_personaje(spec, tallas):
    return componer(capas_personaje(spec, tallas))


# --- actores genericos (los de ARTE-01, ahora como composiciones) ----------------

HUMANO = "race_human_marches"
ACTORES = {
    "aventurero": {"nombre": "Aventurero (jugador)", "raza": HUMANO, "ojos": "azul", "barba": None,
                   "pelo": ("corto", "pelo_castano"),
                   "equipo": {"torso": "aventurero_verde", "piernas": "pantalon_pardo",
                              "pies": "botas"}},
    "aventurero_azul": {"nombre": "Aventurero (tinte azul)", "raza": HUMANO, "barba": None,
                        "pelo": ("corto", "pelo_negro"),
                        "equipo": {"torso": "aventurero_azul", "piernas": "pantalon_pardo",
                                   "pies": "botas"}},
    "aventurero_rojo": {"nombre": "Aventurero (tinte rojo)", "raza": HUMANO, "ojos": "verde",
                        "barba": None, "pelo": ("corto", "pelo_rubio"),
                        "equipo": {"torso": "aventurero_rojo", "piernas": "pantalon_gris",
                                   "pies": "botas"}},
    "aventurero_pardo": {"nombre": "Aventurero (tinte pardo)", "raza": HUMANO, "piel": "piel_morena",
                         "barba": None, "pelo": ("corto", "pelo_pelirrojo"),
                         "equipo": {"torso": "aventurero_pardo", "piernas": "pantalon_oliva",
                                    "pies": "botas"}},
    "guardia": {"nombre": "Guardia de Boundington", "raza": HUMANO, "barba": None, "pelo": None,
                "equipo": {"cabeza": "chapel", "torso": "sobrevesta_azul", "piernas": "pantalon_gris",
                           "pies": "botas_acero"}},
    "aldeano": {"nombre": "Aldeano", "raza": HUMANO, "piel": "piel_morena", "barba": None,
                "pelo": ("corto", "pelo_rubio"),
                "equipo": {"torso": "camisa_chaleco", "piernas": "pantalon_oliva", "pies": "zapatos"}},
    "aldeana": {"nombre": "Aldeana", "raza": HUMANO, "ojos": "verde", "barba": None,
                "pelo": ("largo", "pelo_oscuro"),
                "equipo": {"torso": "vestido_tabernera", "pies": "zapatos"}},
    "cultista": {"nombre": "Perdido encapuchado (cultista)", "raza": HUMANO, "piel": "piel_palida",
                 "ojos": "ambar", "barba": None, "pelo": None,
                 "equipo": {"cabeza": "capucha_purpura", "torso": "habito_purpura",
                            "pies": "sandalias"}},
    "cultista_carcelero": {"nombre": "Carcelero del culto", "raza": HUMANO, "piel": "piel_palida",
                           "ojos": "ambar", "barba": None, "pelo": None,
                           "equipo": {"cabeza": "capucha_gris", "torso": "habito_gris",
                                      "pies": "sandalias"}},
}

# --- personajes de historia (retratos de Boundington rehechos a escala de juego) ------

HISTORIA = {
    "luisarda": {"nombre": "Luisarda", "raza": HUMANO, "ojos": "marron", "barba": None,
                 "pelo": ("largo", "pelo_castano"),
                 "equipo": {"cabeza": "panuelo_rojo", "torso": "blusa_corse", "piernas": "falda_roja",
                            "pies": "zapatos", "mano_izq": "jarra"}},
    "ben_kafka": {"nombre": "Ben Kafka", "raza": HUMANO, "ojos": "marron",
                  "pelo": ("corto", "pelo_oscuro"), "barba": ("BARBA_CORTA", "pelo_oscuro"),
                  "equipo": {"cabeza": "sombrero_pluma", "torso": "cuero", "piernas": "pantalon_oliva",
                             "pies": "botas", "capa": "capa_verde", "arma": "arco_roble"}},
    "griffin": {"nombre": "Griffin", "raza": HUMANO, "piel": "piel_morena", "pelo": None,
                "barba": ("BARBA_CORTA", "pelo_castano"),
                "equipo": {"torso": "sin_mangas_gris", "piernas": "pantalon_azul", "pies": "botas",
                           "arma": "baston_roble", "mano_izq": "farol"}},
    "duende_porcelana": {"nombre": "Duende de porcelana", "raza": HUMANO, "talla": "pequeno",
                         "piel": "porcelana", "ojos": "azul", "barba": None, "pelo": None,
                         "equipo": {"cabeza": "gorro_bufon", "torso": "bufon",
                                    "piernas": "calzas_rojas", "pies": "pantuflas",
                                    "arma": "daga_hierro"}},
    "saga_del_bosque": {"nombre": "Saga del bosque", "raza": "drow_de_la_locura",
                        "equipo": {"cabeza": "capucha_purpura", "torso": "habito_harapiento",
                                   "pies": "sandalias", "arma": "baston_roble"}},
    "verdugo": {"nombre": "Verdugo", "raza": HUMANO, "talla": "grande", "piel": "piel_morena",
                "barba": None, "pelo": None,
                "equipo": {"cabeza": "yelmo_oscuro", "torso": "cuero_oscuro",
                           "piernas": "pantalon_negro", "pies": "botas", "arma": "maza_hierro"}},
    "nina_del_gato": {"nombre": "Nina del gato", "raza": HUMANO, "talla": "pequeno", "ojos": "azul",
                      "barba": None, "pelo": ("largo", "pelo_castano"),
                      "equipo": {"torso": "vestido_lino", "pies": "zapatos",
                                 "capa": "capa_corta_azul"}},
    "caballero": {"nombre": "Caballero de la guardia", "raza": HUMANO, "barba": None,
                  "pelo": ("corto", "pelo_rubio"),
                  "equipo": {"cabeza": "bacinete", "torso": "sobrevesta_azul",
                             "piernas": "grebas_malla", "pies": "escarpes", "capa": "capa_azul",
                             "arma": "espada_acero", "mano_izq": "escudo_acero"}},
    "mercader": {"nombre": "Mercader", "raza": HUMANO, "piel": "piel_morena", "ojos": "marron",
                 "pelo": ("corto", "pelo_negro"), "barba": ("BARBA", "pelo_negro"),
                 "equipo": {"cabeza": "turbante_purpura", "torso": "tunica_mercader",
                            "pies": "zapatos", "mano_izq": "pocion"}},
    "naga_del_rio": {"nombre": "Naga del rio", "raza": "nagas_arcanos",
                     "piel": "escamas_azules", "pelo": ("largo", "pelo_negro"),
                     "quitar_capas": ["espalda", "morro"],
                     "equipo": {"torso": "blusa_corse", "arma": "tridente"}},
}

# --- las 43 razas vestidas como en race_npc_idle.png -------------------------------
# id de raza -> equipo por defecto (las ranuras que la raza no admite se ignoran)

EQUIPO_RAZAS = {
    "aarakocra_de_las_montanas": {"torso": "tunica_azul", "arma": "baston_roble"},
    "aarakocra_del_viento": {"torso": "tunica_sacerdote", "mano_izq": "orbe"},
    "alquimistas_de_aegroum": {"torso": "camisa_chaleco_oscuro", "piernas": "pantalon_pardo",
                               "pies": "botas", "mano_izq": "pocion"},
    "cara_loca": {"torso": "tunica_roja", "piernas": "pantalon_pardo", "pies": "botas",
                  "capa": "capa_corta_verde", "arma": "baston_roble"},
    "cazador_nocturno": {"cabeza": "capucha_negra", "torso": "cuero_oscuro",
                         "piernas": "pantalon_negro", "pies": "botas", "capa": "capa_purpura",
                         "arma": "arco_ceniza"},
    "diplomaticos_de_bastrea": {"torso": "tunica_sacerdote", "pies": "sandalias",
                                "arma": "baston_roble"},
    "draconidos_de_fuego": {"torso": "escamas_bronce", "piernas": "grebas_cuero", "pies": "botas",
                            "arma": "espada_ceniza"},
    "draconidos_de_hielo": {"torso": "cota_malla", "piernas": "pantalon_gris", "pies": "botas",
                            "arma": "hacha_acero"},
    "drow_de_la_casa_noble": {"torso": "habito_purpura", "pies": "botas_altas", "arma": "espada_ceniza",
                        "mano_izq": "orbe"},
    "drow_explorador": {"torso": "cuero_oscuro", "piernas": "pantalon_negro", "pies": "botas_altas",
                          "capa": "capa_verde", "arma": "martillo_acero"},
    "elfos_de_las_estrellas": {"cabeza": "diadema", "torso": "tunica_mago", "pies": "zapatos",
                             "arma": "baston_cristal"},
    "elfos_del_bosque": {"torso": "tunica_verde", "piernas": "calzas_verdes",
                               "pies": "botas_claras", "capa": "capa_corta_verde",
                               "arma": "arco_roble"},
    "enanos_forjadores": {"torso": "camisa_chaleco_oscuro", "piernas": "pantalon_pardo",
                                 "pies": "botas", "arma": "martillo_acero"},
    "enanos_historiadores": {"torso": "tunica_mago", "pies": "botas", "mano_izq": "grimorio"},
    "espectros": {},
    "guardianes_de_la_luz": {"torso": "placas_acero", "piernas": "quijotes", "pies": "escarpes",
                             "capa": "capa_azul", "arma": "espada_oro", "mano_izq": "escudo_oro"},
    "guardianes_del_elemento": {"cabeza": "corona_ramas", "torso": "cuero", "piernas": "pantalon_pardo",
                                "pies": "sandalias", "arma": "baston_roble"},
    "hechiceros_oscuros": {"torso": "habito_purpura", "pies": "zapatos", "capa": "capa_negra",
                           "arma": "baston_cristal"},
    "hombres_lagarto_de_la_selva": {"torso": "cuero", "arma": "lanza_bronce",
                                      "mano_izq": "escudo_madera"},
    "hombres_lagarto_del_pantano": {"torso": "harapos", "arma": "tridente"},
    "medianos_del_bosque": {"torso": "camisa_chaleco", "piernas": "pantalon_pardo",
                                "capa": "capa_corta_verde", "arma": "baston_roble"},
    "medianos_urbanos": {"torso": "aventurero_rojo", "piernas": "pantalon_pardo"},
    "mercenarios_de_ascaria": {"torso": "cuero", "piernas": "pantalon_negro", "pies": "botas_altas",
                               "arma": "espada_hierro"},
    "minotauros_de_la_llanura": {"torso": "cuero_oscuro", "piernas": "falda_parda",
                                 "arma": "hacha_ceniza"},
    "minotauros_del_laberinto": {"torso": "sin_mangas_gris", "piernas": "falda_parda",
                                   "arma": "baston_roble", "mano_izq": "farol"},
    "nagas_arcanos": {"torso": "habito_purpura", "arma": "baston_cristal"},
    "nagas_venenosos": {"torso": "cuero", "arma": "baston_roble"},
    "nobles_de_ascaria": {"cabeza": "diadema", "torso": "vestido_noble", "pies": "zapatos",
                          "capa": "capa_roja"},
    "orcos_del_consejo": {"torso": "cuero_oscuro", "piernas": "pantalon_pardo", "pies": "botas",
                          "capa": "capa_parda", "arma": "baston_roble"},
    "orcos_dramaticos": {"torso": "bufon", "piernas": "calzas_rojas", "pies": "zapatos",
                         "capa": "capa_corta_roja"},
    "race_ascaria_fey_blood": {"torso": "tunica_sacerdote", "pies": "sandalias"},
    "race_dwarf_deepforge": {"cabeza": "bacinete", "torso": "cuero", "piernas": "pantalon_gris",
                             "pies": "botas", "arma": "martillo_acero", "mano_izq": "farol"},
    "race_elf_canopy": {"cabeza": "corona_ramas", "torso": "tunica_verde", "piernas": "calzas_verdes",
                        "pies": "botas_claras", "capa": "capa_verde", "arma": "arco_roble"},
    "race_human_marches": {"torso": "gambeson", "piernas": "pantalon_pardo", "pies": "botas",
                           "capa": "capa_parda", "arma": "lanza_hierro"},
    "race_orc_gongorguma": {"torso": "harapos", "piernas": "pantalon_pardo", "pies": "botas",
                            "arma": "maza_hierro"},
    "revenants": {"cabeza": "capucha_gris", "torso": "habito_gris", "mano_izq": "farol"},
    "drow_de_la_locura": {"torso": "habito_harapiento", "arma": "baston_roble"},
    "drow_del_abismo": {"torso": "tunica_mago", "mano_izq": "grimorio"},
    "tieflings_de_la_sombra": {"torso": "cuero_oscuro", "piernas": "pantalon_negro",
                               "pies": "botas_altas", "capa": "capa_negra", "arma": "daga_hierro"},
    "tieflings_del_vacio": {"torso": "tunica_mago", "pies": "zapatos", "arma": "baston_cristal"},
    "traidores_oscuros": {"cabeza": "capucha_parda", "torso": "harapos", "piernas": "pantalon_pardo",
                          "pies": "botas"},
    # DIALOGOS-01: lo que vestian los yokai, ahora en los drows y sagas de Ayashii que los heredan
    "drow_de_las_sombras": {"torso": "kimono_negro", "pies": "sandalias", "mano_izq": "farol"},
    "drow_de_los_vientos": {"torso": "kimono_blanco", "pies": "sandalias"},
    "yokai_de_las_sombras": {"torso": "kimono_negro", "pies": "sandalias", "mano_izq": "farol"},
    "yokai_de_los_vientos": {"torso": "kimono_blanco", "pies": "sandalias"},
}


def spec_raza(ident, vestida=True):
    """Personaje de una raza con su pelo por defecto y, si vestida, su equipo."""
    spec = {"raza": ident}
    if vestida:
        spec["equipo"] = EQUIPO_RAZAS[ident]
    return spec
