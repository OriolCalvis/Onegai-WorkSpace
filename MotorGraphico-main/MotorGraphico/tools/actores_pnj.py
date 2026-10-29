"""PNJ-01 — Todos los PNJ, montados con las piezas del jugador.

Oriol (05/10/2026): «todos los npcs se tienen que generar con las mismas
esteticas que podria tener un [jugador], o sea, cualquier jugador podria
copiar la estetica de un npc estandar. Los equipos y armas modifican la
apariencia, asi que al final tambien es dificil que un npc y un aventurero
se parezcan en extremo, pudiendo crear solo equipamientos para npc».

Por eso un PNJ es una composicion de personajes.json, como el jugador:

  - la BASE sale de lo que ofrece la creacion de personaje: raza, piel y
    talla (solo los pueblos de cuerpo liso), ojos, peinado y color, barba y
    color, y la ropa de diario (cabeza, torso, piernas, pies, capa);
  - el EQUIPO es lo que el PNJ lleva puesto o en la mano: armas, escudos,
    jarras, faroles... y, para algunos oficios, prendas que solo llevan los
    PNJ (actores_oficios.SOLO_PNJ: la librea de la guardia, el mandil del
    herrero, la casulla, la toga y el birrete, la librea de palacio, la
    cesta y el libro de cuentas).

Nada de capas quitadas a mano ni pieles fuera del atlas: comprobar() lo
exige para cada PNJ. Cada PNJ (cada id de objeto de categoria npc en
assets/objects) tiene su propia composicion, distinta de las demas; los que
se colocan varias veces (la anciana, la guardia, los PNJ de raza de la
plaza) tienen VARIANTES, una por colocacion hasta MAX_VARIANTES, y el motor
reparte una distinta a cada uno.

De donde sale cada composicion:
  1. PNJ: a mano, los de Boundington y los de las cuatro naciones del oeste
     (con lo que dice su ficha o su retrato de PixelLab);
  2. reglas(): el resto, por su oficio (de la id, del nombre o de la carta de
     assets/catalogs/npcs.json), su pueblo (raceId, la nacion de la id o de los
     niveles donde esta colocado, o el lugar de la carta) y su sexo (del
     nombre), con una semilla de la id.

PNJ-02: los pueblos del este segun el lore (NACIONES), ningun yokai
(SIN_CUERPO: son espiritus sin cuerpo) y los drows corrompidos
(actores_corrupcion), que cambian el cuerpo de la raza, no la composicion.
"""
import hashlib
import json
import re
import unicodedata
from pathlib import Path

import actores_armas as armas
import actores_cascos as cascos
import actores_equipo as equipo
import actores_oficios as oficios
import actores_personajes as pers
import actores_razas as razas
from actores_materiales import OJOS

ROOT = Path(__file__).resolve().parent.parent
MAX_VARIANTES = 6
NO_SON_PERSONAS = {"ciervo"}  # fauna con categoria npc: no se viste
# Oriol (05/10/2026): «los yokai son espiritus, ya dejaron de tener cuerpo material
# [...] siguen en el lore pero no son visibles». Ningun PNJ puede ser yokai.
SIN_CUERPO = {"yokai_de_las_sombras", "yokai_de_los_vientos"}

AEG = "alquimistas_de_aegroum"  # humanos de Aegroum (races.json: «Huma de Aegroum»)
LUZ = "guardianes_de_la_luz"     # el otro pueblo humano de Aegroum (magia sagrada)
MARCAS = "race_human_marches"

# Lo que hay en los atlas (gen_personajes.py los escribe de aqui): la creacion
# de personaje ofrece exactamente esto.
PIELES = ["piel_clara", "piel_morena", "piel_palida", "piel_oscura", "piel_aceituna", "piel_rojiza",
          "piel_gris", "piel_elfo", "porcelana"]
COLORES_PELO = ["pelo_castano", "pelo_rubio", "pelo_negro", "pelo_pelirrojo", "pelo_oscuro",
                "pelo_blanco", "pelo_gris"]
COLORES_PELO_RAZA = ["pelo_plata_lila", "pelo_verde_musgo", "pelo_azul_hielo"]  # solo corto/largo
BARBAS = ["BARBA", "BARBA_CORTA"]
TALLAS = ["medio", "bajo", "pequeno", "grande"]


def combos_pelo():
    import actores_pelo as pelo
    out = {(e, c) for e in pelo.ESTILOS for c in COLORES_PELO}
    # Oriol (23/10): el pelo de llama de los elfos del fuego y los elementales de fuego (actores_razas, 2000 b.f.)
    return out | {(e, c) for e in ("corto", "largo") for c in COLORES_PELO_RAZA} | {("largo", "pelo_llama"),
                                                                                 ("cresta", "pelo_llama")}


def normalizar(texto):
    t = unicodedata.normalize("NFD", str(texto).lower())
    return "".join(c for c in t if unicodedata.category(c) != "Mn")


class Dado:
    """Azar determinista por id: la misma id da siempre el mismo PNJ."""

    def __init__(self, *semilla):
        self.h = hashlib.sha256("|".join(map(str, semilla)).encode()).digest()
        self.i = 0

    def n(self, k):
        if self.i + 4 > len(self.h):
            self.h, self.i = hashlib.sha256(self.h).digest(), 0
        v = int.from_bytes(self.h[self.i:self.i + 4], "big")
        self.i += 4
        return v % k

    def uno(self, opciones):
        return opciones[self.n(len(opciones))]

    def si(self, por_ciento):
        return self.n(100) < por_ciento


# --- a mano -------------------------------------------------------------------------
# Como en actores_personajes.HISTORIA. "_varia": por ranura, lo que pueden
# cambiar las variantes (ademas de pelo, ojos y piel).

def P(nombre, raza, sexo, **k):
    spec = {"nombre": nombre, "raza": raza, "_sexo": sexo}
    spec.update(k)
    return spec


ANCIANA_VARIA = {"torso": ["vestido_gris", "vestido_negro", "vestido_lila", "vestido_ocre", "vestido_lino"],
                 "cabeza": ["toca", "panuelo_azul", "cofia", "panuelo_rojo", None],
                 "mano_izq": ["cesta", None, "farol"], "pies": ["zapatos", "zuecos"]}

PNJ = {
    # --- historia (boundington_npcs.json: la campaña de Venides y la precuela) ---
    "skilla": P("Rosita, la tabernera", AEG, "f", ojos="verde", pelo=("recogido", "pelo_pelirrojo"), barba=None,
                equipo={"torso": "blusa_corse", "piernas": "falda_verde", "pies": "zapatos", "mano_izq": "jarra"}),
    "venides": P("Venides, hijo de Nefaria", AEG, "m", ojos="azul", pelo=("largo", "pelo_negro"), barba=None,
                 equipo={"torso": "casaca_azul", "piernas": "pantalon_negro", "pies": "botas_altas"}),
    "aigren": P("Aigren, hijo de Ceaseton", AEG, "m", ojos="azul", pelo=None, barba=("BARBA", "pelo_pelirrojo"),
                equipo={"torso": "mandil_herrero", "piernas": "pantalon_pardo", "pies": "botas",
                        "arma": "martillo_acero"}),
    "luisarda": P("Luisarda, hija de Boundington", AEG, "f", ojos="verde", pelo=("coleta", "pelo_pelirrojo"),
                  barba=None, equipo={"torso": "vestido_rojo_delantal", "pies": "zapatos"}),
    # I-XILA-1: Xila es un hombre (antiguo mercader caido en desgracia): ropa buena y gastada, la capa buena
    # («un hombre en el rincon del fondo, con una capa demasiado buena para ese suelo»), botas cosidas a mano.
    "xila": P("Xila", AEG, "m", piel="piel_palida", ojos="violeta", pelo=("corto", "pelo_gris"),
              barba=("BARBA_CORTA", "pelo_gris"),
              equipo={"torso": "camisa_chaleco_oscuro", "piernas": "pantalon_negro", "pies": "botas_altas",
                      "capa": "capa_granate"}),
    "ben_kafka": P("Ben Kafka", AEG, "m", ojos="marron", pelo=("corto", "pelo_oscuro"),
                   barba=("BARBA_CORTA", "pelo_oscuro"),
                   equipo={"cabeza": "sombrero_cuero", "torso": "cuero", "piernas": "pantalon_oliva",
                           "pies": "botas", "arma": "arco_roble"}),
    "griffin": P("Griffin, limpiador de desagues", AEG, "m", piel="piel_oscura", ojos="marron", pelo=None,
                 barba=("BARBA_CORTA", "pelo_negro"),
                 equipo={"torso": "camisa_sin_mangas", "piernas": "pantalon_azul", "pies": "botas",
                         "arma": "baston_roble", "mano_izq": "farol"}),
    "nina_del_gato": P("Nina de Pico Dragon", AEG, "f", talla="pequeno", ojos="azul",
                       pelo=("largo", "pelo_castano"), barba=None,
                       equipo={"torso": "vestido_azul_delantal", "pies": "zapatos"}),
    "carterista": P("Chico del Casco Antiguo", AEG, "m", talla="pequeno", piel="piel_morena", ojos="marron",
                    pelo=("greñas", "pelo_negro"), barba=None,
                    equipo={"torso": "harapos", "piernas": "pantalon_pardo", "arma": "daga_hierro"}),
    # DIALOGOS-01 (13/10): las mesas de la Taberna Humilde, en persona
    "bordadora": P("Bordadora", AEG, "f", ojos="marron", pelo=("recogido", "pelo_gris"), barba=None,
                   equipo={"torso": "vestido_gris", "pies": "zapatos"}),
    "carretero_jubilado": P("Carretero jubilado", AEG, "m", ojos="azul", pelo=("corto", "pelo_gris"),
                            barba=("BARBA", "pelo_gris"),
                            equipo={"torso": "tunica_parda", "piernas": "pantalon_pardo", "pies": "botas",
                                    "mano_izq": "jarra"}),
    "marido_bordadora": P("Marido de la bordadora", AEG, "m", ojos="marron", pelo=("corto", "pelo_blanco"),
                          barba=("BARBA_CORTA", "pelo_blanco"),
                          equipo={"torso": "camisa_chaleco_oscuro", "piernas": "pantalon_gris", "pies": "zapatos"}),
    "tomas_jornalero": P("Tomás", AEG, "m", piel="piel_morena", ojos="marron", pelo=("corto", "pelo_negro"),
                         barba=None, equipo={"torso": "camisa_sin_mangas", "piernas": "pantalon_oliva",
                                             "pies": "zuecos", "mano_izq": "jarra"}),
    "berto_jornalero": P("Berto", AEG, "m", ojos="verde", pelo=("corto", "pelo_castano"),
                         barba=("BARBA", "pelo_castano"),
                         equipo={"torso": "camisa_chaleco", "piernas": "pantalon_pardo", "pies": "botas"}),
    "jornalero_callado": P("Jornalero", AEG, "m", ojos="azul", pelo=("corto", "pelo_rubio"), barba=None,
                           equipo={"torso": "tunica_gris", "piernas": "pantalon_gris", "pies": "zuecos",
                                   "mano_izq": "jarra"}),
    "encapuchado": P("Encapuchado", AEG, "m", ojos="ambar", barba=None,
                     equipo={"cabeza": "capucha_negra", "torso": "habito_gris", "pies": "botas",
                             "mano_izq": "jarra"}),
    "sectario_perdido": P("Sectario de los Perdidos", "traidores_oscuros", "m", ojos="ambar", barba=None,
                          equipo={"cabeza": "capucha_purpura", "torso": "habito_harapiento", "pies": "sandalias",
                                  "arma": "daga_hierro"}),
    "parroquiano_humilde": P("Parroquiano", AEG, "m", piel="piel_morena", ojos="marron",
                             pelo=("corto", "pelo_castano"), barba=("BARBA_CORTA", "pelo_castano"),
                             equipo={"torso": "camisa_chaleco", "piernas": "pantalon_oliva", "pies": "zuecos",
                                     "mano_izq": "jarra"},
                             _varia={"torso": ["camisa_chaleco", "tunica_parda", "camisa_chaleco_verde", "tunica_gris",
                                               "camisa_chaleco_oscuro"],
                                     "piernas": ["pantalon_oliva", "pantalon_pardo", "pantalon_gris"],
                                     "mano_izq": ["jarra", None], "pies": ["zuecos", "zapatos", "botas"]}),
    "viejo_surysal": P("El viejo de Surysal", AEG, "m", piel="piel_aceituna", ojos="marron",
                       pelo=("corto", "pelo_blanco"), barba=("BARBA", "pelo_blanco"),
                       equipo={"cabeza": "sombrero_paja", "torso": "tunica_ocre", "piernas": "pantalon_pardo",
                               "pies": "sandalias", "arma": "baston_roble"}),
    # --- oficios de Boundington (ciudad_objetos.json) ---
    "anciana": P("Anciana", AEG, "f", ojos="marron", pelo=("recogido", "pelo_blanco"), barba=None,
                 equipo={"cabeza": "toca", "torso": "vestido_gris", "pies": "zapatos", "mano_izq": "cesta"},
                 _varia=ANCIANA_VARIA, _pelo_varia="viejo"),
    "anfitriona": P("Anfitriona", AEG, "f", ojos="negro", pelo=("recogido", "pelo_negro"), barba=None,
                    equipo={"torso": "kimono_blanco", "pies": "sandalias", "mano_izq": "farol"},
                    _varia={"torso": ["kimono_blanco", "kimono_negro", "vestido_lila"], "mano_izq": ["farol", None]}),
    "armero": P("Armero", AEG, "m", piel="piel_morena", ojos="marron", pelo=("corto", "pelo_negro"),
                barba=("BARBA_CORTA", "pelo_negro"),
                equipo={"torso": "cuero", "piernas": "grebas_cuero", "pies": "botas", "arma": "espada_acero",
                        "mano_izq": "escudo_madera"},
                _varia={"torso": ["cuero", "gambeson", "cota_malla"], "arma": ["espada_acero", "hacha_hierro"]}),
    "banquero": P("Banquero", AEG, "m", ojos="azul", pelo=("corto", "pelo_gris"), barba=None,
                  equipo={"torso": "casaca_negra", "piernas": "pantalon_negro", "pies": "zapatos",
                          "mano_izq": "libro_cuentas"}),
    "bardo": P("Bardo", AEG, "m", ojos="verde", pelo=("largo", "pelo_rubio"), barba=None,
               equipo={"cabeza": "sombrero_pluma", "torso": "camisa_chaleco_granate", "piernas": "calzas_verdes",
                       "pies": "botas_claras", "capa": "capa_corta_roja"}),
    "bibliotecaria": P("Bibliotecaria", AEG, "f", piel="piel_palida", ojos="verde", pelo=("recogido", "pelo_castano"),
                       barba=None, equipo={"torso": "toga", "pies": "zapatos", "mano_izq": "grimorio"}),
    "chambelan": P("Chambelan", AEG, "m", ojos="azul", pelo=("corto", "pelo_gris"), barba=None,
                   equipo={"torso": "librea_palacio", "piernas": "pantalon_negro", "pies": "zapatos",
                           "mano_izq": "libro_cuentas"},
                   _varia={"mano_izq": ["libro_cuentas", None], "capa": [None, "capa_granate"]}),
    "encargada_banos": P("Encargada de los banos", AEG, "f", piel="piel_morena", ojos="marron",
                         pelo=("coleta", "pelo_negro"), barba=None,
                         equipo={"cabeza": "panuelo_azul", "torso": "vestido_lino", "pies": "sandalias"}),
    "estudiante": P("Estudiante", AEG, "m", ojos="azul", pelo=("corto", "pelo_rubio"), barba=None,
                    equipo={"cabeza": "birrete", "torso": "tunica_azul", "piernas": "pantalon_gris",
                            "pies": "zapatos", "mano_izq": "grimorio"},
                    _varia={"torso": ["tunica_azul", "camisa_chaleco_azul", "tunica_gris"]}, _sexo_varia=True),
    "guardia": P("Guardia de Boundington", AEG, "m", ojos="marron", pelo=("corto", "pelo_castano"), barba=None,
                 equipo={"cabeza": "chapel", "torso": "librea_guardia", "piernas": "pantalon_gris",
                         "pies": "botas_acero", "arma": "lanza_hierro"},
                 _varia={"cabeza": ["chapel", "bacinete"], "arma": ["lanza_hierro", "espada_hierro"],
                         "mano_izq": [None, "escudo_madera"]}, _sexo_varia=True),
    "joyero": P("Joyero", AEG, "m", piel="piel_aceituna", ojos="negro", pelo=("corto", "pelo_negro"),
                barba=("BARBA", "pelo_negro"),
                equipo={"torso": "casaca_granate", "piernas": "pantalon_negro", "pies": "zapatos", "mano_izq": "orbe"}),
    "maestro_arena": P("Maestro de la arena", AEG, "m", talla="grande", piel="piel_morena", ojos="negro", pelo=None,
                       barba=("BARBA_CORTA", "pelo_negro"),
                       equipo={"torso": "cuero_oscuro", "piernas": "grebas_cuero", "pies": "botas",
                               "capa": "capa_corta_roja", "arma": "maza_hierro"}),
    "mercader": P("Mercader", AEG, "m", piel="piel_morena", ojos="negro", pelo=("corto", "pelo_negro"),
                  barba=("BARBA", "pelo_negro"),
                  equipo={"cabeza": "turbante_purpura", "torso": "tunica_mercader", "pies": "zapatos",
                          "mano_izq": "pocion"},
                  _varia={"cabeza": ["turbante_purpura", "turbante", "sombrero_cuero"],
                          "torso": ["tunica_mercader", "tunica_ocre", "camisa_chaleco_granate"],
                          "mano_izq": ["pocion", "cesta", "libro_cuentas"]}),
    "nino": P("Nino", AEG, "m", talla="pequeno", ojos="azul", pelo=("greñas", "pelo_castano"), barba=None, _edad="nino",
              equipo={"torso": "tunica_parda", "piernas": "pantalon_pardo", "pies": "zuecos"},
              _varia={"torso": ["tunica_parda", "tunica_verde", "tunica_roja", "camisa_chaleco"],
                      "piernas": ["pantalon_pardo", "pantalon_oliva", "pantalon_gris"],
                      "pies": ["zuecos", "zapatos", None]}),
    "posadero": P("Posadero", AEG, "m", ojos="marron", pelo=("corto", "pelo_castano"),
                  barba=("BARBA_CORTA", "pelo_castano"),
                  equipo={"torso": "camisa_chaleco_verde", "piernas": "pantalon_pardo", "pies": "zapatos",
                          "mano_izq": "libro_cuentas"}),
    "profesora": P("Profesora", AEG, "f", ojos="azul", pelo=("recogido", "pelo_gris"), barba=None,
                   equipo={"cabeza": "birrete", "torso": "toga", "pies": "zapatos", "mano_izq": "grimorio"}),
    "sacerdote": P("Sacerdote", LUZ, "m", ojos="azul", pelo=("corto", "pelo_blanco"), barba=None,
                   equipo={"torso": "casulla", "pies": "sandalias", "mano_izq": "orbe"}),
    # AV19-04: la cueva de Gorran. Gorran «grande, con una barba que ha visto mejores
    # navajas», la Furia al hombro (espada_ceniza: hoja oscura con vetas rojas). El de
    # guardia, con panuelo de forajido; las variantes salen solas si se coloca mas de una vez.
    "gorran": P("Gorran, jefe de los bandidos", AEG, "m", ojos="marron", pelo=("corto", "pelo_negro"),
                barba=("BARBA", "pelo_negro"),
                equipo={"torso": "cuero_oscuro", "piernas": "pantalon_pardo", "pies": "botas",
                        "arma": "espada_ceniza"}, _unico=True),
    "bandido_gorran": P("Bandido de guardia", AEG, "m", ojos="marron", pelo=("corto", "pelo_castano"),
                        barba=("BARBA_CORTA", "pelo_castano"),
                        equipo={"cabeza": "cp_panuelo_forajido", "torso": "cuero", "piernas": "pantalon_gris",
                                "pies": "botas", "arma": "hacha_hierro"},
                        _varia={"torso": ["cuero", "aventurero_pardo"], "arma": ["hacha_hierro", "espada_hierro"]}),
    "sargento": P("Sargento", AEG, "m", ojos="marron", pelo=("corto", "pelo_negro"), barba=("BARBA", "pelo_negro"),
                  equipo={"cabeza": "bacinete", "torso": "librea_guardia", "piernas": "grebas_malla",
                          "pies": "botas_acero", "arma": "espada_acero", "mano_izq": "escudo_acero"},
                  _varia={"cabeza": ["bacinete", "chapel"], "capa": [None, "capa_granate"]}, _sexo_varia=True),
    "taquillera": P("Taquillera", AEG, "f", ojos="azul", pelo=("largo", "pelo_rubio"), barba=None,
                    equipo={"torso": "vestido_lila", "pies": "zapatos", "mano_izq": "libro_cuentas"}),
    "tendero_mercado": P("Tendero del mercado", AEG, "m", piel="piel_morena", ojos="marron",
                         pelo=("corto", "pelo_oscuro"), barba=None,
                         equipo={"cabeza": "sombrero_paja", "torso": "camisa_chaleco_azul",
                                 "piernas": "pantalon_oliva", "pies": "zuecos", "mano_izq": "cesta"}),
    "herrero": P("Herrero", AEG, "m", ojos="marron", pelo=("corto", "pelo_negro"), barba=("BARBA_CORTA", "pelo_negro"),
                 equipo={"torso": "mandil_herrero", "piernas": "pantalon_gris", "pies": "botas",
                         "arma": "martillo_acero"}),
    "sastre": P("Sastre", AEG, "m", ojos="verde", pelo=("corto", "pelo_castano"), barba=None,
                equipo={"torso": "casaca_granate", "piernas": "pantalon_negro", "pies": "zapatos"}),
    "tabernero": P("Tabernero", AEG, "m", ojos="azul", pelo=("corto", "pelo_rubio"), barba=("BARBA", "pelo_rubio"),
                   equipo={"torso": "camisa_chaleco", "piernas": "pantalon_pardo", "pies": "zapatos",
                           "mano_izq": "jarra"}),
    # --- la Plaza de las Razas: Chronos disfrazado de mortal (Oriol, 05/10/2026) ---
    # Un viejo de paso con capucha, como tantos; los ojos dorados y el reloj de arena
    # son lo unico que lo delata.
    "chronos_disfrazado": P("Viajero de paso", MARCAS, "m", piel="piel_palida", ojos="dorado",
                            pelo=("largo", "pelo_blanco"), barba=("BARBA", "pelo_blanco"),
                            equipo={"cabeza": "capucha_gris", "torso": "habito_gris", "pies": "sandalias",
                                    "capa": "capa_gris", "arma": "baston_roble", "mano_izq": "reloj_arena"}),
    # --- las cuatro naciones del oeste (on_npcs.json: cada uno sale de su ficha) ---
    "on_guardacaminos": P("Guardacaminos de Umedan", "elfos_del_bosque", "m", ojos="verde",
                          equipo={"cabeza": "capucha_verde", "torso": "cuero", "piernas": "calzas_verdes",
                                  "pies": "botas_claras", "capa": "capa_verde", "arma": "arco_roble"}),
    "on_archivera_ekait": P("Archivera de Ekait", "elfos_de_las_estrellas", "f", ojos="azul",
                            pelo=("recogido", "pelo_blanco"),
                            equipo={"torso": "toga", "pies": "zapatos", "mano_izq": "grimorio"}),
    "on_elfo_estrellas": P("Elfo de las Estrellas", "elfos_de_las_estrellas", "m", ojos="violeta",
                           pelo=("largo", "pelo_plata_lila"),
                           equipo={"cabeza": "diadema", "torso": "tunica_mago", "pies": "zapatos",
                                   "capa": "capa_azul", "arma": "baston_cristal"}),
    "on_thalgrim_menor": P("Thalgrim el Menor", "enanos_forjadores", "m", ojos="azul",
                           pelo=("corto", "pelo_negro"), barba=("BARBA", "pelo_negro"),
                           equipo={"torso": "mandil_herrero", "piernas": "pantalon_gris", "pies": "botas",
                                   "arma": "martillo_acero"}),
    "on_runista_rhenor": P("Runista de Rhenor", "enanos_historiadores", "m", ojos="dorado",
                           equipo={"cabeza": "capucha_gris", "torso": "habito_gris", "pies": "botas",
                                   "mano_izq": "grimorio", "arma": "baston_roble"}),
    "on_capataz_veta": P("Capataz de la veta", "race_dwarf_deepforge", "m", ojos="marron",
                         pelo=("corto", "pelo_oscuro"), barba=("BARBA_CORTA", "pelo_oscuro"),
                         equipo={"cabeza": "bacinete", "torso": "camisa_chaleco_oscuro", "piernas": "pantalon_pardo",
                                 "pies": "botas", "arma": "martillo_acero", "mano_izq": "farol"}),
    "on_anciana_lagarto": P("Anciana de la aldea", "hombres_lagarto_del_pantano", "f", ojos="ambar",
                            equipo={"torso": "habito_monje", "capa": "capa_corta_parda", "arma": "baston_roble"}),
    "on_barquero_celeste": P("Barquero del Rio Celeste", MARCAS, "m", piel="piel_aceituna", ojos="azul",
                             pelo=("coleta", "pelo_negro"), barba=("BARBA_CORTA", "pelo_negro"),
                             equipo={"cabeza": "gorro_gris", "torso": "camisa_sin_mangas", "piernas": "pantalon_azul",
                                     "arma": "lanza_bronce"}),
    "on_vigia_del_desierto": P("Vigia del desierto", "mercenarios_de_ascaria", "m", ojos="ambar",
                               pelo=("corto", "pelo_negro"), barba=None,
                               equipo={"cabeza": "turbante", "torso": "tunica_ocre", "piernas": "pantalon_ocre",
                                       "pies": "sandalias", "capa": "capa_parda", "arma": "lanza_hierro"}),
    "on_portaestandarte": P("Portaestandarte de los Cuatro", "orcos_del_consejo", "m", ojos="rojo",
                            equipo={"cabeza": "casco_cuernos", "torso": "escamas_bronce", "piernas": "grebas_cuero",
                                    "pies": "botas", "capa": "capa_roja", "arma": "lanza_hierro",
                                    "mano_izq": "escudo_madera"}),
    "on_chaman_bleek": P("Chaman de los Bleek", "orcos_dramaticos", "m", ojos="amarillo",
                         equipo={"cabeza": "corona_ramas", "torso": "harapos", "piernas": "falda_parda",
                                 "capa": "capa_corta_parda", "arma": "baston_roble", "mano_izq": "pocion"}),
}


# --- reglas: oficio, pueblo y sexo --------------------------------------------------------
# Por oficio, lo que puede llevar en cada ranura segun el sexo ("x": los dos).
# None = nada. La ropa de diario sale de aqui; el pelo, los ojos y la piel, del
# pueblo y de la semilla.

R = lambda **k: k  # noqa: E731  (legibilidad)

OFICIOS = {
    "tabernero": R(m=R(torso=["camisa_chaleco", "camisa_chaleco_verde", "camisa_chaleco_granate", "camisa_chaleco_oscuro"],
                       piernas=["pantalon_pardo", "pantalon_oliva", "pantalon_negro"], pies=["zapatos", "botas", "zuecos"],
                       mano_izq=["jarra"]),
                   f=R(torso=["vestido_tabernera", "vestido_verde_delantal", "vestido_rojo_delantal", "blusa_corse"],
                       pies=["zapatos", "zuecos"], cabeza=[None, "cofia", "panuelo_rojo"], mano_izq=["jarra"])),
    "posadero": R(m=R(torso=["camisa_chaleco_verde", "camisa_chaleco_azul", "camisa_chaleco"],
                      piernas=["pantalon_pardo", "pantalon_gris"], pies=["zapatos"], mano_izq=["libro_cuentas", "farol"]),
                  f=R(torso=["vestido_azul_delantal", "vestido_verde_delantal", "vestido_lino"], pies=["zapatos"],
                      cabeza=[None, "cofia"], mano_izq=["libro_cuentas", "farol"])),
    "herrero": R(x=R(torso=["mandil_herrero"], piernas=["pantalon_gris", "pantalon_pardo", "pantalon_negro"],
                     pies=["botas", "botas_claras"], arma=["martillo_acero"])),
    "tendero": R(m=R(torso=["camisa_chaleco_azul", "camisa_chaleco", "tunica_mercader", "tunica_ocre"],
                     piernas=["pantalon_oliva", "pantalon_pardo"], pies=["zuecos", "zapatos", "sandalias"],
                     cabeza=[None, "sombrero_paja", "turbante"], mano_izq=["cesta", "pocion", None]),
                 f=R(torso=["vestido_ocre", "vestido_verde_delantal", "vestido_azul_delantal", "blusa_corse"],
                     piernas=[None], pies=["zapatos", "zuecos"], cabeza=[None, "panuelo_azul", "sombrero_paja"],
                     mano_izq=["cesta", "pocion"])),
    "banquero": R(m=R(torso=["casaca_negra", "casaca_granate", "camisa_chaleco_oscuro"], piernas=["pantalon_negro"],
                      pies=["zapatos"], mano_izq=["libro_cuentas"]),
                  f=R(torso=["vestido_negro", "vestido_noble", "vestido_purpura"], pies=["zapatos"],
                      mano_izq=["libro_cuentas"])),
    "erudito": R(x=R(torso=["toga", "tunica_mago", "habito_gris"], piernas=[None], pies=["zapatos", "sandalias"],
                     cabeza=[None, "birrete"], mano_izq=["grimorio", "libro_cuentas"])),
    "arena": R(x=R(torso=["cuero_oscuro", "sin_mangas_gris", "cuero"], piernas=["grebas_cuero", "pantalon_negro"],
                   pies=["botas"], capa=[None, "capa_corta_roja"], arma=["maza_hierro", "hacha_hierro", "lanza_hierro"])),
    "soldado": R(x=R(torso=["cota_malla", "gambeson", "escamas_bronce", "cuero"], piernas=["grebas_cuero", "grebas_malla"],
                     pies=["botas", "botas_acero"], cabeza=["chapel", "bacinete", None],
                     arma=["lanza_hierro", "espada_hierro", "hacha_hierro"], mano_izq=["escudo_madera", None])),
    "capitan": R(x=R(torso=["sobrevesta_roja", "sobrevesta_azul", "escamas_acero"], piernas=["grebas_malla"],
                     pies=["botas_altas", "botas_acero"], capa=["capa_roja", "capa_azul", "capa_negra"],
                     arma=["espada_acero"], mano_izq=[None, "escudo_acero"])),
    "marinero": R(x=R(torso=["camisa_sin_mangas", "tunica_azul", "camisa_chaleco_azul"],
                      piernas=["pantalon_azul", "pantalon_gris"], pies=[None, "botas"],
                      cabeza=["gorro_rojo", "gorro_gris", "panuelo_azul", None], arma=[None, "lanza_bronce"])),
    "farero": R(x=R(torso=["tunica_gris", "casaca_negra"], piernas=["pantalon_gris", "falda_larga_parda"],
                    pies=["botas"], capa=["capa_gris"], mano_izq=["farol"])),
    "heraldo": R(x=R(torso=["librea_palacio", "sobrevesta_roja", "casaca_granate"], piernas=["calzas_rojas", "pantalon_negro"],
                     pies=["botas_altas", "zapatos"], cabeza=[None, "sombrero_pluma"], capa=[None, "capa_corta_roja"])),
    "diplomatico": R(m=R(torso=["casaca_granate", "casaca_negra", "tunica_sacerdote"], piernas=["pantalon_negro"],
                         pies=["zapatos"], capa=["capa_purpura", "capa_roja", None], mano_izq=[None, "libro_cuentas"]),
                     f=R(torso=["vestido_noble", "vestido_purpura", "vestido_lila"], pies=["zapatos"],
                         cabeza=[None, "diadema"], capa=[None, "capa_purpura"])),
    "anciano": R(m=R(torso=["tunica_parda", "camisa_chaleco_oscuro", "habito_monje", "tunica_gris"],
                     piernas=["pantalon_pardo", "pantalon_gris"], pies=["sandalias", "zapatos", "zuecos"],
                     cabeza=[None, "sombrero_paja", "capucha_parda"], arma=["baston_roble"]),
                 f=ANCIANA_VARIA),
    "mercader": R(m=R(torso=["tunica_mercader", "tunica_ocre", "camisa_chaleco_granate", "casaca_granate"],
                      piernas=["pantalon_pardo", "pantalon_negro", None], pies=["zapatos", "sandalias"],
                      cabeza=[None, "turbante", "turbante_purpura", "sombrero_cuero"],
                      mano_izq=["pocion", "libro_cuentas", "cesta"]),
                  f=R(torso=["vestido_ocre", "vestido_noble", "vestido_lila", "blusa_corse"],
                      piernas=[None, "falda_larga_parda"], pies=["zapatos", "sandalias"],
                      cabeza=[None, "panuelo_rojo", "turbante"], mano_izq=["pocion", "libro_cuentas", "cesta"])),
    "autoridad": R(m=R(torso=["casaca_granate", "sobrevesta_azul", "casaca_negra", "librea_palacio"],
                       piernas=["pantalon_negro", "grebas_malla"], pies=["botas_altas", "zapatos"],
                       capa=["capa_roja", "capa_azul", "capa_granate", None], arma=[None, "espada_acero"]),
                   f=R(torso=["vestido_noble", "vestido_purpura", "casaca_granate"], piernas=[None, "falda_negra"],
                       pies=["zapatos", "botas_altas"], cabeza=[None, "diadema"], capa=["capa_roja", "capa_granate", None])),
    "mentor": R(x=R(torso=["tunica_mago", "habito_monje", "habito_gris", "tunica_sacerdote"], pies=["sandalias", "zapatos"],
                    cabeza=[None, "sombrero_mago_azul", "capucha_gris"], arma=["baston_roble", "baston_cristal", None],
                    mano_izq=["grimorio", "orbe", None])),
    "informante": R(x=R(torso=["cuero_oscuro", "casaca_negra", "camisa_chaleco_oscuro"], piernas=["pantalon_negro", "pantalon_gris"],
                        pies=["botas_altas", "botas"], cabeza=["capucha_negra", "capucha_parda", None],
                        capa=["capa_negra", "capa_gris", None], arma=[None, "daga_hierro"])),
    "traidor": R(x=R(torso=["cuero_oscuro", "casaca_negra", "habito_purpura"], piernas=["pantalon_negro"],
                     pies=["botas_altas", "botas"], cabeza=["capucha_negra", "antifaz", None], capa=["capa_negra", "capa_purpura"],
                     arma=["daga_hierro", "daga_plata"])),
    "rival": R(x=R(torso=["aventurero_rojo", "aventurero_azul", "aventurero_pardo", "aventurero_verde", "cuero"],
                   piernas=["pantalon_pardo", "pantalon_gris", "grebas_cuero"], pies=["botas", "botas_claras"],
                   capa=[None, "capa_corta_verde", "capa_corta_roja", "capa_corta_azul"],
                   arma=["espada_hierro", "arco_roble", "hacha_hierro", "daga_hierro"])),
    "victima": R(m=R(torso=["harapos", "camisa_chaleco", "tunica_parda", "camisa_sin_mangas"],
                     piernas=["pantalon_pardo", "pantalon_oliva"], pies=["zuecos", "sandalias", None]),
                 f=R(torso=["vestido_lino", "vestido_gris", "harapos", "vestido_tabernera"], pies=["zuecos", "zapatos", None],
                     cabeza=[None, "panuelo_azul", "cofia"])),
    "contacto": R(m=R(torso=["camisa_chaleco", "tunica_verde", "tunica_azul", "camisa_chaleco_azul", "tunica_gris"],
                      piernas=["pantalon_pardo", "pantalon_oliva", "pantalon_azul"], pies=["botas", "zapatos"],
                      cabeza=[None, "sombrero_cuero", "capucha_parda", "gorro_rojo"], capa=[None, "capa_parda"]),
                  f=R(torso=["vestido_verde", "vestido_azul", "blusa_corse", "vestido_teja", "vestido_lila"],
                      piernas=[None, "falda_parda", "falda_verde"], pies=["botas", "zapatos"],
                      cabeza=[None, "panuelo_azul", "capucha_verde"], capa=[None, "capa_corta_verde"])),
    "religioso": R(x=R(torso=["habito_monje", "habito_gris", "tunica_sacerdote"], pies=["sandalias"],
                       cabeza=[None, "toca", "capucha_gris"], mano_izq=["grimorio", "farol", None])),
    "explorador": R(x=R(torso=["cuero", "aventurero_verde", "aventurero_pardo"], piernas=["calzas_verdes", "pantalon_pardo"],
                        pies=["botas_claras", "botas"], cabeza=["capucha_verde", "sombrero_pluma", None],
                        capa=["capa_verde", "capa_corta_verde", "capa_parda"], arma=["arco_roble", "lanza_hierro"])),
    "artesano": R(m=R(torso=["camisa_chaleco", "camisa_sin_mangas", "tunica_parda"], piernas=["pantalon_pardo", "pantalon_gris"],
                      pies=["botas", "zuecos"], cabeza=[None, "gorro_gris"], mano_izq=[None, "cesta"]),
                  f=R(torso=["vestido_ocre", "vestido_verde_delantal", "blusa_corse"], pies=["zuecos", "zapatos"],
                      cabeza=[None, "cofia", "panuelo_rojo"], mano_izq=[None, "cesta"])),
    "chaman": R(x=R(torso=["harapos", "habito_monje"], piernas=["falda_parda", None], capa=["capa_corta_parda", None],
                    cabeza=["corona_ramas", None], arma=["baston_roble"], mano_izq=["pocion", "orbe"])),
    "guia": R(x=R(torso=["aventurero_verde", "aventurero_azul", "tunica_verde"], piernas=["pantalon_pardo"], pies=["botas"],
                  cabeza=[None, "sombrero_cuero"], capa=["capa_corta_verde", None], arma=["baston_roble"])),
}

# Palabras de la id o del nombre -> oficio (gana la primera que aparece).
PALABRAS_OFICIO = [
    ("taberna", "tabernero"), ("tabernero", "tabernero"), ("posada", "posadero"), ("herreria", "herrero"),
    ("herrero", "herrero"), ("forj", "herrero"), ("mercado", "tendero"), ("tendero", "tendero"), ("tienda", "tendero"),
    ("panader", "artesano"), ("molin", "artesano"), ("banco", "banquero"), ("biblioteca", "erudito"),
    ("archiver", "erudito"), ("cronista", "erudito"), ("cartograf", "erudito"), ("arena", "arena"),
    ("capitan", "capitan"), ("fletero", "marinero"), ("barquero", "marinero"), ("farer", "farero"),
    ("heraldo", "heraldo"), ("enviad", "diplomatico"), ("ancian", "anciano"), ("viej", "anciano"),
    ("old", "anciano"), ("matrona", "religioso"), ("madre", "religioso"), ("viuda", "victima"), ("guia", "guia"),
]

# Pueblo por nacion. Oriol (05/10/2026), los pueblos del este: «Choubar (hombres
# pajaro), Mistarium (tieflings) y Nocturnsea (inhabitada, es la entrada a la casa
# de la muerte, dicen los antiguos). Gongorguma son orcos. Ayashe, drows». En
# Nocturnsea no vive nadie: quien atiende alli son muertos (espectros y renacidos),
# en los mismos sitios y con los mismos dialogos. Neblinia es un clan de Mistarium
# (la enviada de este_norte_senal_ceniza es «una tiefling de librea oscura») y
# Qethatos, la universidad de Gongorguma. La nacion sale de la id o, si la id no
# la dice, de los niveles donde esta colocado («Choubar - Klimnebra»...).
# pelo_propio: % de las veces que llevan el color de pelo de su pueblo (el plata-lila
# de los tieflings del vacio, el blanco de los drows); colores: los unicos colores
# de pelo posibles (los muertos, desvaidos).
COLORES_MUERTO = ["pelo_blanco", "pelo_gris", "pelo_negro"]
NACIONES = {
    "gongorguma": dict(pueblos=["race_orc_gongorguma", "orcos_del_consejo", "race_orc_gongorguma", "orcos_dramaticos"]),
    "choubar": dict(pueblos=["aarakocra_del_viento", "aarakocra_de_las_montanas"]),
    "mistarium": dict(pueblos=["tieflings_del_vacio", "tieflings_de_la_sombra"], pelo_propio=50),
    # la farera y el posadero, espectros; el herrero, el mercader y el tabernero, renacidos
    "nocturnsea": dict(pueblos=["revenants", "revenants", "espectros"], colores=COLORES_MUERTO),
    "qethatos": dict(pueblos=["race_orc_gongorguma", "orcos_del_consejo"]),
    "neblinia": dict(pueblos=["tieflings_del_vacio", "tieflings_de_la_sombra"], pelo_propio=50),
    "ayash": dict(pueblos=["drow_de_la_casa_noble", "drow_explorador", "drow_de_las_sombras"], pelo_propio=60),
}

# Pueblo por el lugar de la carta (assets/catalogs/npcs.json, los PNJ de la libreria).
LUGARES = [
    ("bastrea", ["diplomaticos_de_bastrea", "guardianes_del_elemento"], None),
    ("bosque", ["elfos_del_bosque", "medianos_del_bosque", MARCAS], ["piel_clara", "piel_aceituna"]),
    ("llerba", [MARCAS, "medianos_urbanos", MARCAS], ["piel_clara", "piel_morena", "piel_rojiza"]),
    ("pozo azul", [MARCAS, "enanos_forjadores"], ["piel_clara", "piel_aceituna"]),
    ("ceniza", [MARCAS, "race_dwarf_deepforge", "mercenarios_de_ascaria"], ["piel_morena", "piel_gris"]),
    ("chimeneas", ["enanos_forjadores", "race_dwarf_deepforge", MARCAS], ["piel_rojiza", "piel_morena"]),
    ("colinas rotas", ["race_orc_gongorguma", "orcos_del_consejo"], None),
    ("colegio", ["hechiceros_oscuros", AEG, "tieflings_del_vacio"], ["piel_palida", "piel_gris"]),
    ("gran taller", [AEG, "enanos_historiadores"], ["piel_clara", "piel_rojiza"]),
    ("tomba", [MARCAS, "revenants", MARCAS], ["piel_palida", "piel_gris"]),
    ("cielo roto", ["mercenarios_de_ascaria", "draconidos_de_fuego", MARCAS], ["piel_aceituna", "piel_morena"]),
    ("lazareto", [MARCAS, AEG], ["piel_palida", "piel_clara"]),
    ("ultima carga", ["mercenarios_de_ascaria", MARCAS, "orcos_del_consejo"], ["piel_aceituna", "piel_morena"]),
    ("mijorn", [MARCAS, "medianos_urbanos"], ["piel_aceituna", "piel_morena"]),
    # los faroles rojos de Numandum (Bastrea). Ni aqui hay yokai: son espiritus sin
    # cuerpo, siguen en el lore pero no se ven (Oriol, 05/10/2026).
    ("farol", ["diplomaticos_de_bastrea", MARCAS], ["piel_palida"]),
    ("barrio antiguo", [AEG, LUZ], ["piel_clara", "piel_morena"]),
]

ROL_CARTA = {"mercader": "mercader", "autoridad": "autoridad", "mentor": "mentor", "informante": "informante",
             "traidor": "traidor", "rival": "rival", "victima": "victima", "contacto": "contacto"}

# --- sexo por el nombre -------------------------------------------------------------------
FEMENINAS = {"anciana", "tabernera", "enana", "orca", "mujer", "minotaura", "elfa", "exploradora", "diplomatica",
             "archivera", "farera", "enviada", "capitana", "matrona", "madre", "viuda", "molinera", "bibliotecaria",
             "profesora", "taquillera", "encargada", "anfitriona", "nina", "mensajera", "draconida", "guardiana",
             "mercenaria", "humana", "naga venenosa", "vieja", "hija", "tendera"}
MASCULINAS = {"anciano", "tabernero", "enano", "orco", "hombre", "minotauro", "elfo", "explorador", "archivero",
              "farero", "enviado", "capitan", "viudo", "molinero", "bibliotecario", "profesor", "nino", "chico",
              "draconido", "guardian", "mercenario", "humano", "viejo", "hijo", "tendero", "heraldo", "fletero",
              "barquero", "capataz", "chaman", "portaestandarte", "runista", "panadero", "cazador", "hechicero",
              "astronomo", "renacido", "traidor", "mediano", "noble", "padre", "old"}
NOMBRES_F = {"bruna", "delia", "guillema", "ilse", "isabeu", "laia", "nara", "neus", "ona", "petra", "quima",
             "roser", "sanca", "serena", "sunna", "urraca", "velia", "aloma", "sibil", "iru", "itx"}
NOMBRES_M = {"bastia", "biel", "corvo", "cosme", "damaso", "eudald", "ezra", "ferran", "guifre", "hug", "ilun",
             "jofre", "miquel", "marc", "ot", "pau", "ramon", "roderic", "tomas", "tonet", "vidal"}


def sexo_de(nombre):
    """'f', 'm' o None (nombre neutro: guardia, guia, estudiante, cronista...)."""
    n = normalizar(nombre)
    for frase in ("naga venenosa",):
        if frase in n:
            return "f"
    palabras = re.findall(r"[a-z]+", n)
    for w in palabras:
        if w in FEMENINAS:
            return "f"
        if w in MASCULINAS:
            return "m"
    for w in palabras:
        if w in NOMBRES_F:
            return "f"
        if w in NOMBRES_M:
            return "m"
    return None


# --- utilidades de composicion ------------------------------------------------------------

OJOS_NATURALES = ["marron", "azul", "verde", "negro", "marron", "azul", "ambar"]
PIELES_HUMANAS = ["piel_clara", "piel_clara", "piel_morena", "piel_aceituna", "piel_palida", "piel_oscura",
                  "piel_rojiza"]
PELO_M = ["corto", "corto", "rizado", "greñas", "largo", "coleta"]
PELO_F = ["largo", "recogido", "coleta", "rizado", "largo"]
COLORES_JOVEN = ["pelo_castano", "pelo_rubio", "pelo_negro", "pelo_pelirrojo", "pelo_oscuro", "pelo_castano"]
COLORES_VIEJO = ["pelo_blanco", "pelo_gris"]

# Familias de color: una pieza puede cambiar por una hermana en las variantes.
FAMILIAS = [
    ["tunica_verde", "tunica_azul", "tunica_roja", "tunica_parda", "tunica_gris", "tunica_ocre"],
    ["aventurero_verde", "aventurero_azul", "aventurero_rojo", "aventurero_pardo"],
    ["camisa_chaleco", "camisa_chaleco_oscuro", "camisa_chaleco_azul", "camisa_chaleco_verde", "camisa_chaleco_granate"],
    ["vestido_teja", "vestido_verde", "vestido_azul", "vestido_gris", "vestido_ocre", "vestido_lila", "vestido_lino"],
    ["vestido_tabernera", "vestido_rojo_delantal", "vestido_azul_delantal", "vestido_verde_delantal"],
    ["pantalon_pardo", "pantalon_gris", "pantalon_oliva", "pantalon_negro", "pantalon_azul", "pantalon_ocre"],
    ["falda_roja", "falda_parda", "falda_verde", "falda_gris", "falda_negra"],
    ["capa_roja", "capa_verde", "capa_azul", "capa_parda", "capa_negra", "capa_purpura", "capa_gris", "capa_granate"],
    ["capa_corta_roja", "capa_corta_verde", "capa_corta_azul", "capa_corta_parda"],
    ["capucha_purpura", "capucha_gris", "capucha_verde", "capucha_parda", "capucha_negra", "capucha_roja"],
    ["sobrevesta_azul", "sobrevesta_roja", "sobrevesta_blanca"],
    ["kimono_negro", "kimono_blanco"],
    ["casaca_azul", "casaca_negra", "casaca_granate"],
    ["botas", "botas_claras", "zapatos"],
    ["panuelo_rojo", "panuelo_azul"],
    ["gorro_rojo", "gorro_gris"],
    ["turbante", "turbante_purpura"],
    ["espada_hierro", "espada_acero"],
    ["lanza_hierro", "lanza_bronce"],
]
HERMANAS = {p: fam for fam in FAMILIAS for p in fam}

RANURAS_ROPA = ("cabeza", "torso", "piernas", "pies", "capa")


def es_liso(raza):
    r = razas.POR_ID[raza]
    return (isinstance(r["piel"], str) and r["piel"] in PIELES and not r.get("rasgos") and not r.get("cola")
            and not r.get("alas") and not r.get("cuerpo"))


def admite(raza, ranura):
    return ranura not in razas.POR_ID[raza].get("sin", [])


def clave(spec):
    """Texto estable: dos PNJ con la misma clave se ven iguales."""
    return json.dumps({k: (list(v) if isinstance(v, tuple) else v) for k, v in spec.items()
                       if not k.startswith("_") and k != "nombre"}, sort_keys=True, ensure_ascii=False)


def limpia(spec):
    """Quita las ranuras que la raza no admite y las piezas vacias."""
    raza = spec["raza"]
    eq = {r: p for r, p in spec.get("equipo", {}).items() if p and admite(raza, r)}
    spec["equipo"] = eq
    if not admite(raza, "pelo"):
        spec.pop("pelo", None)
        spec.pop("barba", None)
    return spec


def pelo_para(dado, sexo, edad, colores=None):
    estilos = PELO_F if sexo == "f" else (["corto", "greñas", "rizado"] if edad == "nino" else PELO_M)
    colores = colores or (COLORES_VIEJO if edad == "viejo" else COLORES_JOVEN)
    if sexo != "f" and edad == "viejo" and dado.si(30):
        return None  # calvo
    return (dado.uno(estilos), dado.uno(colores))


def pelo_de_nacion(dado, raza, sexo, edad, nacion):
    """El pelo de un PNJ de una nacion del este: a veces el color de su pueblo
    (NACIONES[..]["pelo_propio"]) y, en Nocturnsea, solo colores desvaidos."""
    nacion = nacion or {}
    propio = (razas.POR_ID[raza].get("pelo") or (None, None))[1]
    if nacion.get("pelo_propio") and propio and dado.si(nacion["pelo_propio"]):
        estilos = ["corto", "largo"] if propio in COLORES_PELO_RAZA else (PELO_F if sexo == "f" else PELO_M)
        return (dado.uno(estilos), propio)
    return pelo_para(dado, sexo, edad, nacion.get("colores"))


def barba_para(dado, sexo, edad, color):
    if sexo == "f" or edad == "nino" or not dado.si(55 if edad == "viejo" else 35):
        return None
    c = color if color in COLORES_PELO else dado.uno(COLORES_VIEJO if edad == "viejo" else COLORES_JOVEN)
    return (dado.uno(BARBAS), c)


def de_plantilla(dado, oficio, sexo):
    t = OFICIOS[oficio]
    opciones = t.get(sexo) or t.get("x") or t.get("m")
    return {r: dado.uno(ops) for r, ops in opciones.items()}


# --- de una definicion de objeto a una composicion -----------------------------------------

def oficio_de(oid, nombre, carta):
    texto = normalizar(oid + " " + nombre)
    for palabra, oficio in PALABRAS_OFICIO:
        if palabra in texto:
            return oficio
    if carta and carta.get("role") in ROL_CARTA:
        return ROL_CARTA[carta["role"]]
    return "contacto"


def pueblo_de(oid, defn, carta, dado, nacion=None):
    """(raza, pieles posibles o None, la entrada de NACIONES o {}).
    nacion: la de los niveles donde esta colocado, si la id no la dice."""
    if defn.get("raceId"):
        return defn["raceId"], None, {}
    for clave_nacion, n in NACIONES.items():
        if clave_nacion in oid:
            return dado.uno(n["pueblos"]), None, n
    if nacion in NACIONES:
        return dado.uno(NACIONES[nacion]["pueblos"]), None, NACIONES[nacion]
    lugar = normalizar((carta or {}).get("location") or "")
    for clave_lugar, pueblos, pieles in LUGARES:
        if clave_lugar in lugar:
            return dado.uno(pueblos), pieles, {}
    return dado.uno([MARCAS, AEG, MARCAS, "mercenarios_de_ascaria"]), PIELES_HUMANAS, {}


def edad_de(nombre, oficio):
    n = normalizar(nombre)
    if oficio == "anciano" or re.search(r"\b(viej[oa]|old|ancian[oa])\b", n):
        return "viejo"
    if re.search(r"\b(nin[oa]|chic[oa]|menut)\b", n):
        return "nino"
    return None


def reglas(oid, defn, carta=None, nacion=None):
    nombre = defn.get("name", oid)
    dado = Dado(oid)
    sexo = sexo_de(nombre)
    sexo_varia = sexo is None
    sexo = sexo or dado.uno(["m", "f"])
    oficio = oficio_de(oid, nombre, carta)
    raza, pieles, de_nacion = pueblo_de(oid, defn, carta, dado, nacion)
    edad = edad_de(nombre, oficio)
    spec = {"nombre": nombre, "raza": raza, "_sexo": sexo, "_oficio": oficio, "_edad": edad, "_pieles": pieles,
            "_sexo_varia": sexo_varia}
    if de_nacion:
        spec["_nacion"] = de_nacion
    if es_liso(raza):
        if pieles:
            spec["piel"] = dado.uno(pieles)
        if edad == "nino":
            spec["talla"] = "pequeno"
        elif re.search(r"\b(breve|menut)\b", normalizar(nombre)):
            spec["talla"] = "bajo"
    spec["ojos"] = dado.uno(OJOS_NATURALES)
    if admite(raza, "pelo"):
        spec["pelo"] = pelo_de_nacion(dado, raza, sexo, edad, de_nacion)
        spec["barba"] = barba_para(dado, sexo, edad, (spec["pelo"] or (None, None))[1])
    spec["equipo"] = de_plantilla(dado, oficio, sexo)
    return limpia(spec)


def de_raza(oid, defn):
    """Los PNJ de raza del editor (npc_race_*): como viste su pueblo, con su sexo."""
    raza = defn["raceId"]
    r = razas.POR_ID[raza]
    nombre = defn.get("name", oid)
    sexo = sexo_de(nombre)
    spec = {"nombre": nombre, "raza": raza, "_sexo": sexo or "m", "_sexo_varia": sexo is None,
            "equipo": dict(pers.EQUIPO_RAZAS[raza]), "_de_raza": True}
    if sexo == "f" and admite(raza, "pelo"):
        estilo, color = r.get("pelo") or ("largo", "pelo_castano")
        spec["pelo"] = ("largo" if estilo in ("corto", "cresta") else estilo, color)
        spec["barba"] = None
    return limpia(spec)


# --- variantes ------------------------------------------------------------------------------

def variante(base, oid, k):
    """La variante k (1..) de un PNJ: otra cara y algo de ropa, mismo oficio y pueblo."""
    d = Dado(oid, "variante", k)
    v = {kk: (dict(vv) if isinstance(vv, dict) else vv) for kk, vv in base.items()}
    sexo = base.get("_sexo", "m")
    if base.get("_sexo_varia") and k % 2 == 1:
        sexo = "f" if sexo == "m" else "m"
    v["_sexo"] = sexo
    raza = v["raza"]
    edad = base.get("_edad") or ("viejo" if base.get("_pelo_varia") == "viejo" else None)
    if es_liso(raza):
        v["piel"] = d.uno(base.get("_pieles") or PIELES_HUMANAS)
    v["ojos"] = d.uno(OJOS_NATURALES)
    if admite(raza, "pelo"):
        propio = (razas.POR_ID[raza].get("pelo") or (None, None))[1]
        if base.get("_de_raza") and propio and d.si(60):
            # el color de su pueblo (elfos de pelo blanco, tieflings plata-lila...)
            estilos = ["corto", "largo"] if propio in COLORES_PELO_RAZA else (PELO_F if sexo == "f" else PELO_M)
            v["pelo"] = (d.uno(estilos), propio)
        else:
            v["pelo"] = pelo_de_nacion(d, raza, sexo, edad, base.get("_nacion"))
        v["barba"] = barba_para(d, sexo, edad, (v["pelo"] or (None, None))[1])
    eq = dict(base.get("equipo", {}))
    if base.get("_oficio") and base["_oficio"] in OFICIOS:
        eq = de_plantilla(d, base["_oficio"], sexo)
    for ranura, opciones in base.get("_varia", {}).items():
        eq[ranura] = d.uno(opciones)
    if not base.get("_oficio"):
        cambia = 70 if base.get("_de_raza") else 50
        for ranura, pieza in list(eq.items()):
            if pieza in HERMANAS and ranura not in base.get("_varia", {}) and d.si(cambia):
                eq[ranura] = d.uno(HERMANAS[pieza])
        if base.get("_de_raza") and admite(raza, "capa") and d.si(35):
            eq["capa"] = None if eq.get("capa") else d.uno(["capa_parda", "capa_corta_verde", "capa_gris"])
    # (una plantilla de oficio puede dejar la ranura a None: «or ""», no el defecto de get)
    if sexo == "f" and base.get("_sexo") == "m" and (eq.get("piernas") or "").startswith("pantalon") and d.si(40):
        eq["piernas"] = d.uno(["falda_parda", "falda_gris", "falda_verde"])
    v["equipo"] = eq
    return limpia(v)


def unicas(todos):
    """Ninguna composicion se repite: si dos chocan, la segunda cambia de ojos y,
    si hace falta, de color de pelo (siempre lo mismo para la misma id)."""
    vistas = {}
    for oid, specs in todos.items():
        for i, spec in enumerate(specs):
            intentos = 0
            while clave(spec) in vistas:
                d = Dado(oid, i, "unica", intentos)
                spec["ojos"] = d.uno(list(OJOS))
                if intentos >= 3 and spec.get("pelo"):
                    spec["pelo"] = (spec["pelo"][0], d.uno(COLORES_JOVEN + COLORES_VIEJO))
                intentos += 1
                if intentos > 40:
                    raise SystemExit(f"{oid}: no encuentro una composicion distinta de {vistas[clave(spec)]}")
            vistas[clave(spec)] = f"{oid}#{i}"
    return todos


# --- comprobar: lo que el jugador podria copiar ------------------------------------------------

def piezas_de(ranura):
    if ranura == "cabeza":
        return set(cascos.CABEZA)
    if ranura in ("arma", "mano_izq"):
        return {k for k in armas.ARMAS if armas.ranura_de(k) == ranura}
    return set({"torso": equipo.TORSO, "piernas": equipo.PIERNAS, "pies": equipo.PIES,
                "capa": equipo.CAPAS}[ranura])


def comprobar(oid, spec):
    """Errores de una composicion de PNJ (lista vacia = se puede montar y copiar)."""
    err = []
    raza = spec.get("raza")
    if raza not in razas.POR_ID:
        return [f"{oid}: raza desconocida {raza!r}"]
    if raza in SIN_CUERPO:
        return [f"{oid}: {raza} es un espiritu sin cuerpo: sigue en el lore, pero no se ve"]
    r = razas.POR_ID[raza]
    if "quitar_capas" in spec:
        err.append(f"{oid}: quita capas de la raza (el jugador no puede)")
    otra_piel = "piel" in spec and spec["piel"] != r["piel"]
    otra_talla = "talla" in spec and spec["talla"] != r["talla"]
    if (otra_piel or otra_talla) and not es_liso(raza):
        err.append(f"{oid}: {raza} no es de cuerpo liso: no admite otra piel ni otra talla")
    if "piel" in spec and spec["piel"] not in PIELES and spec["piel"] != r["piel"]:
        err.append(f"{oid}: piel {spec['piel']!r} fuera del atlas de cuerpos")
    if "talla" in spec and spec["talla"] not in TALLAS:
        err.append(f"{oid}: talla {spec['talla']!r}")
    if spec.get("ojos", r["ojos"]) not in OJOS:
        err.append(f"{oid}: ojos {spec['ojos']!r}")
    if not admite(raza, "pelo") and (spec.get("pelo") or spec.get("barba")):
        err.append(f"{oid}: {raza} no tiene pelo")
    if spec.get("pelo") and tuple(spec["pelo"]) not in combos_pelo():
        err.append(f"{oid}: peinado {spec['pelo']!r} fuera del atlas")
    if spec.get("barba") and (spec["barba"][0] not in BARBAS or spec["barba"][1] not in COLORES_PELO):
        err.append(f"{oid}: barba {spec['barba']!r} fuera del atlas")
    for ranura, pieza in spec.get("equipo", {}).items():
        if ranura not in pers.RANURAS_EQUIPO:
            err.append(f"{oid}: ranura {ranura!r}")
        elif pieza not in piezas_de(ranura):
            err.append(f"{oid}: {ranura} {pieza!r} no existe")
        elif not admite(raza, ranura):
            err.append(f"{oid}: {raza} no admite {ranura}")
    return err


def solo_pnj(spec):
    """Las piezas que el jugador no puede llevar (prendas de oficio)."""
    return sorted(p for p in spec.get("equipo", {}).values() if p in oficios.SOLO_PNJ)


# --- todos los PNJ del juego -------------------------------------------------------------------

def definiciones(raiz=ROOT):
    """id -> (definicion, catalogo) de los objetos de categoria npc de assets/objects."""
    out = {}
    for ruta in sorted((Path(raiz) / "assets/objects").glob("*.json")):
        try:
            datos = json.loads(ruta.read_text(encoding="utf-8"))
        except ValueError:
            continue
        for o in datos.get("objects", []) if isinstance(datos, dict) else []:
            if o.get("category") == "npc" and o["id"] not in NO_SON_PERSONAS:
                out[o["id"]] = (o, ruta.name)
    return out


def colocaciones(raiz=ROOT):
    """id -> cuantas veces esta colocado en los niveles (assets/levels)."""
    n = {}
    for ruta in sorted((Path(raiz) / "assets/levels").glob("*.json")):
        try:
            datos = json.loads(ruta.read_text(encoding="utf-8"))
        except ValueError:
            continue
        for o in datos.get("objects", []) if isinstance(datos, dict) else []:
            oid = o.get("objectId")
            if oid:
                n[oid] = n.get(oid, 0) + 1
    return n


def naciones_colocadas(raiz=ROOT):
    """id -> nacion (clave de NACIONES) cuando todos los niveles donde esta colocado
    son de esa nacion: el nombre del nivel la lleva delante («Choubar - Klimnebra»)."""
    vistas = {}
    for ruta in sorted((Path(raiz) / "assets/levels").glob("*.json")):
        try:
            datos = json.loads(ruta.read_text(encoding="utf-8"))
        except ValueError:
            continue
        if not isinstance(datos, dict):
            continue
        nombre = str(datos.get("name", ""))
        nacion = normalizar(nombre.split(" - ")[0]).strip() if " - " in nombre else None
        for o in datos.get("objects", []):
            if o.get("objectId"):
                vistas.setdefault(o["objectId"], set()).add(nacion if nacion in NACIONES else None)
    return {oid: next(iter(n)) for oid, n in vistas.items() if len(n) == 1 and None not in n}


def cartas(raiz=ROOT):
    ruta = Path(raiz) / "assets/catalogs/npcs.json"
    if not ruta.exists():
        return {}
    return {e["id"]: e for e in json.loads(ruta.read_text(encoding="utf-8")).get("entries", [])}


# Oriol (22/10): «¿tengo alguna manera de editar su apariencia?». El editor de aspecto
# (tools/editor_aspecto.html, de tools/exportar_editor_aspecto.py) guarda tools/ajustes_pnj.json:
# {id: {"equipo": {ranura: pieza o null}, "pelo": [estilo, color] o null, "barba": [estilo, color] o null}}.
# Lo que no dice, se queda como esta. Las variantes de un PNJ repetido salen de la base ya ajustada.
_RUTA_AJUSTES = Path(__file__).resolve().with_name("ajustes_pnj.json")


def ajustes():
    if not _RUTA_AJUSTES.exists():
        return {}
    datos = json.loads(_RUTA_AJUSTES.read_text(encoding="utf-8"))
    return {k: v for k, v in datos.items() if not k.startswith("_")}


def ajusta(oid, base, cambios=None):
    aj = (ajustes() if cambios is None else cambios).get(oid)
    if not aj:
        return base
    base = {k: (dict(v) if isinstance(v, dict) else v) for k, v in base.items()}
    if "equipo" in aj:
        eq = dict(base.get("equipo", {}))
        for ranura, pieza in aj["equipo"].items():
            if pieza:
                eq[ranura] = pieza
            else:
                eq.pop(ranura, None)
        base["equipo"] = eq
    for k in ("pelo", "barba"):
        if k in aj:
            base[k] = tuple(aj[k]) if aj[k] else None
    return base


def todos(raiz=ROOT):
    """id -> [composicion, variante 2, ...] de todos los PNJ, ya comprobados."""
    defs, usos, fichas = definiciones(raiz), colocaciones(raiz), cartas(raiz)
    naciones = naciones_colocadas(raiz)
    cambios = ajustes()
    out = {}
    for oid in sorted(defs):
        defn, _cat = defs[oid]
        if oid in PNJ:
            base = limpia({k: (dict(v) if isinstance(v, dict) else v) for k, v in PNJ[oid].items()})
        elif oid.startswith("npc_race_") and defn.get("raceId") in pers.EQUIPO_RAZAS:
            base = de_raza(oid, defn)
        else:
            carta = fichas.get(oid.replace("npc_npc_", "npc_", 1))
            base = reglas(oid, defn, carta, naciones.get(oid))
        base = ajusta(oid, base, cambios)  # Oriol (22/10): lo cambiado a mano en el editor de aspecto
        n = 1 if base.get("_unico") else max(1, min(MAX_VARIANTES, usos.get(oid, 0)))  # PROLOGO-02
        out[oid] = [base] + [variante(base, oid, k) for k in range(1, n)]
    unicas(out)
    errores = [e for oid, specs in out.items() for s in specs for e in comprobar(oid, s)]
    if errores:
        raise SystemExit("PNJ que el jugador no podria copiar:\n  " + "\n  ".join(errores[:40]))
    return out


def publica(spec):
    """La composicion tal como va al manifiesto (sin las marcas internas _*)."""
    return {k: v for k, v in spec.items() if not k.startswith("_")}


# PROLOGO-02 (13/10) — El reparto del prólogo nuevo: Sho-Noco, los cuatro amigos
# en Ceaseton (756, el brindis) y un año después (757, ya con la capucha morada de
# los que escuchan), Venides de vuelta de las montañas, el capitán de Aegroum y la
# gente de la taberna del puerto.
_VERINA = dict(ojos="verde", pelo=("largo", "pelo_castano"), barba=None)
_EDUL = dict(ojos="verde", pelo=("corto", "pelo_castano"), barba=None)
_SANDES = dict(ojos="ambar", pelo=("coleta", "pelo_rubio"), barba=None)
PNJ.update({
    "sho_noco": P("Sho-Noco, el barquero", "revenants", "m", ojos="blanco", barba=None,
                  equipo={"cabeza": "capucha_gris", "torso": "kimono_blanco", "arma": "baston_roble", "mano_izq": "farol"}),
    "venides_joven": P("Venides, 756", AEG, "m", ojos="azul", pelo=("largo", "pelo_negro"), barba=None,
                       equipo={"torso": "casaca_azul", "piernas": "pantalon_negro", "pies": "botas_altas",
                               "capa": "capa_azul", "mano_izq": "jarra"}),
    "venides_vuelta": P("Venides, 757", AEG, "m", ojos="azul", pelo=("greñas", "pelo_negro"),
                        barba=("BARBA_CORTA", "pelo_negro"),
                        equipo={"torso": "gambeson", "piernas": "pantalon_negro", "pies": "botas_altas",
                                "capa": "capa_azul"}),
    "verina_joven": P("Verina, 756", AEG, "f", **_VERINA,
                      equipo={"torso": "vestido_azul", "pies": "zapatos", "mano_izq": "grimorio"}),
    "edul_joven": P("Edul, 756", AEG, "m", **_EDUL,
                    equipo={"torso": "camisa_chaleco_granate", "piernas": "pantalon_pardo", "pies": "botas",
                            "mano_izq": "jarra"}),
    "sandes_joven": P("Sandes, 756", AEG, "m", **_SANDES,
                      equipo={"torso": "casaca_granate", "piernas": "calzas_rojas", "pies": "botas_altas",
                              "capa": "capa_corta_roja", "mano_izq": "jarra"}),
    # Oriol (22/10): en el sueño de la puerta (757) ya son de los que escuchan: la túnica morada
    # con su capucha, no la ropa de calle con una capa encima.
    "verina_puerta": P("Verina, 757", AEG, "f", **_VERINA,
                       equipo={"cabeza": "capucha_purpura", "torso": "habito_purpura", "pies": "sandalias"}),
    "edul_puerta": P("Edul, 757", AEG, "m", **_EDUL,
                     equipo={"cabeza": "capucha_purpura", "torso": "habito_purpura", "pies": "sandalias"}),
    "sandes_puerta": P("Sandes, 757", AEG, "m", **_SANDES,
                       equipo={"cabeza": "capucha_purpura", "torso": "habito_purpura", "pies": "botas_altas"}),
    "capitan_aegroum": P("Capitán de la guardia real", AEG, "m", ojos="marron", pelo=("corto", "pelo_gris"),
                         barba=("BARBA", "pelo_gris"),
                         equipo={"torso": "sobrevesta_azul", "piernas": "pantalon_gris", "pies": "botas_altas",
                                 "capa": "capa_azul"}),
    "tabernero_ceaseton": P("Tabernero de Ceaseton", AEG, "m", ojos="marron", pelo=None, barba=("BARBA", "pelo_castano"),
                            equipo={"torso": "camisa_chaleco", "piernas": "pantalon_pardo", "pies": "zuecos",
                                    "mano_izq": "jarra"}),
    "pescador_ceaseton": P("Pescador de Ceaseton", AEG, "m", piel="piel_morena", ojos="marron",
                           pelo=("greñas", "pelo_negro"), barba=("BARBA", "pelo_negro"),
                           equipo={"cabeza": "gorro_rojo", "torso": "camisa_sin_mangas", "piernas": "pantalon_azul",
                                   "pies": "zuecos", "mano_izq": "jarra"}),
})
# Personas de la historia que salen en varios niveles (el prólogo, la taberna):
# son la misma persona, sin variantes de cara.
UNICOS = ["sho_noco", "venides", "venides_joven", "venides_vuelta", "verina_joven", "edul_joven", "sandes_joven",
          "verina_puerta", "edul_puerta", "sandes_puerta", "capitan_aegroum", "tabernero_ceaseton", "skilla",
          "bordadora", "carretero_jubilado", "marido_bordadora", "tomas_jornalero", "berto_jornalero",
          "jornalero_callado", "encapuchado"]
for _k in UNICOS:
    if _k in PNJ:
        PNJ[_k]["_unico"] = True

# PROLOGO-02 — Edul en 757, en la herrería de Aigren: el delantal que no es suyo,
# la cara manchada de hollín y un frasco (el fuego que no se apaga con agua).
PNJ["edul"] = P("Edul", AEG, "m", ojos="verde", pelo=("corto", "pelo_castano"), barba=None,
                equipo={"torso": "mandil_herrero", "piernas": "pantalon_pardo", "pies": "botas",
                        "arma": "martillo_acero", "mano_izq": "pocion"}, _unico=True)


# ESCENAS-01 (16/10/2026) — la Barriada de Surysal en persona. Antorchas y horcas no existen
# como pieza: farol y lanza mientras Pixel no las dibuje.
PNJ.update({
    "zarela": P("Zarela, la que habla por la Barriada", AEG, "f", piel="piel_aceituna", ojos="negro",
                pelo=("recogido", "pelo_gris"), barba=None,
                equipo={"cabeza": "panuelo_rojo", "torso": "vestido_teja", "pies": "sandalias"}, _unico=True),
    "anciana_barriada": P("Anciana de la Barriada", AEG, "f", piel="piel_aceituna", ojos="marron",
                          pelo=("recogido", "pelo_blanco"), barba=None,
                          equipo={"cabeza": "panuelo_azul", "torso": "vestido_ocre", "pies": "zuecos", "mano_izq": "cesta"},
                          _varia=ANCIANA_VARIA, _pelo_varia="viejo"),
    "padre_de_mira": P("El padre de Mira", AEG, "m", piel="piel_morena", ojos="marron", pelo=("corto", "pelo_castano"),
                       barba=("BARBA_CORTA", "pelo_castano"),
                       equipo={"torso": "camisa_sin_mangas", "piernas": "pantalon_pardo", "pies": "zuecos"}, _unico=True),
    "forjador_surysal": P("Forjador de Surysal", AEG, "m", piel="piel_aceituna", ojos="negro", pelo=("corto", "pelo_negro"),
                          barba=("BARBA", "pelo_negro"),
                          equipo={"torso": "mandil_herrero", "piernas": "pantalon_pardo", "pies": "botas",
                                  "arma": "martillo_acero"}, _unico=True),
    "vecina_surysal": P("Vecina con una horca", AEG, "f", piel="piel_morena", ojos="marron", pelo=("coleta", "pelo_castano"),
                        barba=None,
                        equipo={"torso": "vestido_verde_delantal", "pies": "zuecos", "arma": "lanza_hierro"}, _unico=True),
    "guerrero_gitano": P("Guerrero de la Barriada", AEG, "m", piel="piel_aceituna", ojos="negro", pelo=("largo", "pelo_negro"),
                         barba=("BARBA_CORTA", "pelo_negro"),
                         equipo={"cabeza": "panuelo_rojo", "torso": "camisa_chaleco_granate", "piernas": "pantalon_pardo",
                                 "pies": "sandalias", "arma": "hacha_hierro"}, _unico=True),
    "guerrero_gitano_2": P("Guerrero de la Barriada", AEG, "m", piel="piel_aceituna", ojos="marron", pelo=("coleta", "pelo_negro"),
                           barba=None,
                           equipo={"cabeza": "panuelo_azul", "torso": "camisa_chaleco_verde", "piernas": "pantalon_gris",
                                   "pies": "sandalias", "arma": "espada_hierro"}, _unico=True),
    "saqueador_antorcha": P("Saqueador con antorcha", AEG, "m", piel="piel_morena", ojos="marron", pelo=("greñas", "pelo_negro"),
                            barba=None,
                            equipo={"torso": "camisa_sin_mangas", "piernas": "pantalon_gris", "pies": "zuecos",
                                    "arma": "daga_hierro", "mano_izq": "farol"},
                            _varia={"torso": ["camisa_sin_mangas", "tunica_gris", "camisa_chaleco", "tunica_parda", "harapos"],
                                    "piernas": ["pantalon_gris", "pantalon_pardo", "pantalon_oliva"],
                                    "arma": ["daga_hierro", "hacha_hierro", "baston_roble"]}),
    "cabecilla_saqueadores": P("Cabecilla de los saqueadores", AEG, "m", piel="piel_morena", ojos="negro",
                               pelo=("corto", "pelo_negro"), barba=None,
                               equipo={"torso": "camisa_chaleco_oscuro", "piernas": "pantalon_negro", "pies": "botas",
                                       "arma": "hacha_hierro", "mano_izq": "farol"}, _unico=True),
})

# AV19-08 (19/10/2026) — la cripta en cinco mapas: Edul y Verina en la sala de estudio (con la capucha
# de la cripta), Sandes en la cámara honda y el sereno encadenado (su casaca azul, sin lo demás).
PNJ.update({
    "edul_cripta": P("Edul", AEG, "m", **_EDUL,
                     equipo={"cabeza": "capucha_purpura", "torso": "camisa_chaleco_granate", "piernas": "pantalon_pardo",
                             "pies": "botas", "mano_izq": "pocion"}, _unico=True),
    "verina_cripta": P("Verina", AEG, "f", **_VERINA,
                       equipo={"cabeza": "capucha_purpura", "torso": "vestido_azul", "pies": "zapatos", "mano_izq": "grimorio"},
                       _unico=True),
    "sandes_cripta": P("Sandes", AEG, "m", **_SANDES,
                       equipo={"torso": "casaca_granate", "piernas": "calzas_rojas", "pies": "botas_altas",
                               "capa": "capa_corta_roja"}, _unico=True),
    "prisionero_cripta": P("Sereno encadenado", AEG, "m", ojos="marron", pelo=("greñas", "pelo_castano"),
                           barba=("BARBA_CORTA", "pelo_castano"),
                           equipo={"torso": "casaca_azul", "piernas": "pantalon_gris"}, _unico=True),
})

# ESCENAS-02 guardia_castillo
import copy as _c02
PNJ['guardia_castillo'] = _c02.deepcopy(PNJ['guardia'])
PNJ['guardia_castillo']["ojos"] = "verde"

# ESCENAS-02 capitan_guardia
import copy as _c02
PNJ['capitan_guardia'] = _c02.deepcopy(PNJ['sargento'])
PNJ['capitan_guardia']["ojos"] = "verde"

# ESCENAS-02 venides_castillo
import copy as _c02
PNJ['venides_castillo'] = _c02.deepcopy(PNJ['venides'])
PNJ['venides_castillo']["ojos"] = "verde"

# ESCENAS-02 vecina_ventana
import copy as _c02
PNJ['vecina_ventana'] = _c02.deepcopy(PNJ['bordadora'])
PNJ['vecina_ventana']["ojos"] = "verde"

# ESCENAS-02 xila_noche
import copy as _c02
PNJ['xila_noche'] = _c02.deepcopy(PNJ['xila'])
PNJ['xila_noche']["ojos"] = "verde"

# ESCENAS-07 hijo_de_aigren
import copy as _c07
PNJ['hijo_de_aigren'] = _c07.deepcopy(PNJ['nino'])
PNJ['hijo_de_aigren']["ojos"] = "verde"

# ESCENAS-07 aigren_carro
import copy as _c07
PNJ['aigren_carro'] = _c07.deepcopy(PNJ['aigren'])
PNJ['aigren_carro']["ojos"] = "verde"

# ESCENAS-07 sastre_enfrente
import copy as _c07
PNJ['sastre_enfrente'] = _c07.deepcopy(PNJ['sastre'])
PNJ['sastre_enfrente']["ojos"] = "verde"

# ESCENAS-07 xila_escalon
import copy as _c07
PNJ['xila_escalon'] = _c07.deepcopy(PNJ['xila'])
PNJ['xila_escalon']["ojos"] = "verde"

# ESCENAS-07 xila_tienda
import copy as _c07
PNJ['xila_tienda'] = _c07.deepcopy(PNJ['xila'])
PNJ['xila_tienda']["ojos"] = "verde"

# ESCENAS-07 instructor_perdido
import copy as _c07
PNJ['instructor_perdido'] = _c07.deepcopy(PNJ['sectario_perdido'])
PNJ['instructor_perdido']["ojos"] = "verde"

# ESCENAS-07 sandes_culto_d2
import copy as _c07
PNJ['sandes_culto_d2'] = _c07.deepcopy(PNJ['sectario_perdido'])
PNJ['sandes_culto_d2']["ojos"] = "verde"

# ESCENAS-07 sandes_culto_d3
import copy as _c07
PNJ['sandes_culto_d3'] = _c07.deepcopy(PNJ['sectario_perdido'])
PNJ['sandes_culto_d3']["ojos"] = "verde"

# ESCENAS-03 cobrador_fiesta
import copy as _c03
PNJ['cobrador_fiesta'] = _c03.deepcopy(PNJ['mercader'])
PNJ['cobrador_fiesta']["ojos"] = "verde"

# ESCENAS-03 vigilante_fiesta
import copy as _c03
PNJ['vigilante_fiesta'] = _c03.deepcopy(PNJ['sargento'])
PNJ['vigilante_fiesta']["ojos"] = "verde"

# ESCENAS-03 edul_mascara
import copy as _c03
PNJ['edul_mascara'] = _c03.deepcopy(PNJ['edul'])
PNJ['edul_mascara']["ojos"] = "verde"

# ESCENAS-03 invitado_noble
import copy as _c03
PNJ['invitado_noble'] = _c03.deepcopy(PNJ['chambelan'])
PNJ['invitado_noble']["ojos"] = "verde"

# ESCENAS-03 invitado_mercader
import copy as _c03
PNJ['invitado_mercader'] = _c03.deepcopy(PNJ['mercader'])
PNJ['invitado_mercader']["ojos"] = "verde"

# ESCENAS-03 seguidor_brinda
import copy as _c03
PNJ['seguidor_brinda'] = _c03.deepcopy(PNJ['parroquiano_humilde'])
PNJ['seguidor_brinda']["ojos"] = "verde"

# ESCENAS-03 musico_fiesta
import copy as _c03
PNJ['musico_fiesta'] = _c03.deepcopy(PNJ['bardo'])
PNJ['musico_fiesta']["ojos"] = "verde"

# ESCENAS-03 criado_fiesta
import copy as _c03
PNJ['criado_fiesta'] = _c03.deepcopy(PNJ['tendero_mercado'])
PNJ['criado_fiesta']["ojos"] = "verde"

# ESCENAS-03 anfitrion_fiesta
import copy as _c03
PNJ['anfitrion_fiesta'] = _c03.deepcopy(PNJ['chambelan'])
PNJ['anfitrion_fiesta']["ojos"] = "verde"

# ESCENAS-03 anfitriona_fiesta
import copy as _c03
PNJ['anfitriona_fiesta'] = _c03.deepcopy(PNJ['anfitriona'])
PNJ['anfitriona_fiesta']["ojos"] = "verde"

# ESCENAS-03 portero_fiesta
import copy as _c03
PNJ['portero_fiesta'] = _c03.deepcopy(PNJ['guardia'])
PNJ['portero_fiesta']["ojos"] = "verde"

# ESCENAS-03 noble_recuerda
import copy as _c03
PNJ['noble_recuerda'] = _c03.deepcopy(PNJ['chambelan'])
PNJ['noble_recuerda']["ojos"] = "verde"

# ESCENAS-03 mascaras provisionales hasta mascara_media de Pixel
PNJ['invitado_noble'].setdefault("equipo", {})["cabeza"] = "sombrero_pluma"
PNJ['invitado_noble']["_varia"] = {"cabeza": ["sombrero_pluma", "capucha_gris"], "pies": ["botas", "zapatos"]}
PNJ['invitado_mercader'].setdefault("equipo", {})["cabeza"] = "sombrero_pluma"
PNJ['invitado_mercader']["_varia"] = {"cabeza": ["sombrero_pluma", "capucha_gris"], "pies": ["botas", "zapatos"]}
PNJ['seguidor_brinda'].setdefault("equipo", {})["cabeza"] = "sombrero_pluma"
PNJ['seguidor_brinda']["_varia"] = {"cabeza": ["sombrero_pluma", "capucha_gris"], "pies": ["botas", "zapatos"]}
PNJ['musico_fiesta'].setdefault("equipo", {})["cabeza"] = "sombrero_pluma"
PNJ['musico_fiesta']["_varia"] = {"cabeza": ["sombrero_pluma", "capucha_gris"], "pies": ["botas", "zapatos"]}
PNJ["edul_mascara"].setdefault("equipo", {})["cabeza"] = "mascara_porcelana"

# CRIPTA-25 (21/10/2026) — la gente de la cripta, con la capucha de los Perdidos.
import copy as _c25
for _id, _nombre, _sexo, _torso, _mano in (
        ("guardia_trampilla", "Sectario de guardia", "m", "habito_harapiento", None),
        ("guardia_trampilla_2", "Sectario de guardia", "m", "habito_gris", None),
        ("novicio_candil", "Novicio del candil", "m", "habito_gris", "farol"),
        ("copista_cripta", "Novicia copista", "f", "habito_gris", "grimorio"),
        ("archivero_cripta", "Archivero de los Perdidos", "m", "habito_harapiento", "libro_cuentas"),
        ("novicio_litera", "Novicio dormido", "m", "habito_gris", None),
        ("fiel_cripta", "Perdido", "m", "habito_harapiento", None),
        ("oficiante_cripta", "Oficiante", "m", "habito_harapiento", "grimorio")):
    PNJ[_id] = _c25.deepcopy(PNJ["sectario_perdido"])
    PNJ[_id]["nombre"] = _nombre
    PNJ[_id]["_sexo"] = _sexo
    PNJ[_id]["equipo"] = dict(PNJ[_id]["equipo"], torso=_torso)
    PNJ[_id]["equipo"].pop("arma", None)
    if _mano:
        PNJ[_id]["equipo"]["mano_izq"] = _mano
    PNJ[_id]["_unico"] = True
PNJ["guardia_trampilla"]["equipo"]["arma"] = "daga_hierro"
PNJ["guardia_trampilla_2"]["equipo"]["arma"] = "daga_hierro"
PNJ["fiel_cripta"]["_varia"] = {"torso": ["habito_harapiento", "habito_gris"]}
PNJ["fiel_cripta"]["_sexo_varia"] = True
PNJ["fiel_cripta"].pop("_unico", None)
PNJ["edul_rescate"] = _c25.deepcopy(PNJ["edul_cripta"])

# ESCENAS-04 sandes_plaza
import copy as _c04
PNJ['sandes_plaza'] = _c04.deepcopy(PNJ['sandes_cripta'])
PNJ['sandes_plaza']["ojos"] = "verde"

# ESCENAS-04 verina_estrado
import copy as _c04
PNJ['verina_estrado'] = _c04.deepcopy(PNJ['verina_cripta'])
PNJ['verina_estrado']["ojos"] = "verde"

# ESCENAS-04 edul_plaza
import copy as _c04
PNJ['edul_plaza'] = _c04.deepcopy(PNJ['edul_cripta'])
PNJ['edul_plaza']["ojos"] = "verde"

# ESCENAS-04 captor_estrado
import copy as _c04
PNJ['captor_estrado'] = _c04.deepcopy(PNJ['sectario_perdido'])
PNJ['captor_estrado']["ojos"] = "verde"

# ESCENAS-04 venides_base
import copy as _c04
PNJ['venides_base'] = _c04.deepcopy(PNJ['venides'])
PNJ['venides_base']["ojos"] = "verde"

# ESCENAS-04 soldado_20
import copy as _c04
PNJ['soldado_20'] = _c04.deepcopy(PNJ['guardia'])
PNJ['soldado_20']["ojos"] = "verde"

# ESCENAS-04 soldado_10
import copy as _c04
PNJ['soldado_10'] = _c04.deepcopy(PNJ['guardia'])
PNJ['soldado_10']["ojos"] = "verde"

# ESCENAS-04 explorador_herido
import copy as _c04
PNJ['explorador_herido'] = _c04.deepcopy(PNJ['guardia'])
PNJ['explorador_herido']["ojos"] = "verde"

# ESCENAS-04 saqueador_muralla
import copy as _c04
PNJ['saqueador_muralla'] = _c04.deepcopy(PNJ['sectario_perdido'])
PNJ['saqueador_muralla']["ojos"] = "verde"

# ESCENAS-04 fanatico_tejado
import copy as _c04
PNJ['fanatico_tejado'] = _c04.deepcopy(PNJ['sectario_perdido'])
PNJ['fanatico_tejado']["ojos"] = "verde"

# ESCENAS-04 xila_base
import copy as _c04
PNJ['xila_base'] = _c04.deepcopy(PNJ['xila'])
PNJ['xila_base']["ojos"] = "verde"

# ESCENAS-04 aigren_base
import copy as _c04
PNJ['aigren_base'] = _c04.deepcopy(PNJ['aigren'])
PNJ['aigren_base']["ojos"] = "verde"

# ESCENAS-04 capitan_base
import copy as _c04
PNJ['capitan_base'] = _c04.deepcopy(PNJ['capitan_guardia'])
PNJ['capitan_base']["ojos"] = "verde"

# ESCENAS-05 verina_retenida
import copy as _c05
PNJ['verina_retenida'] = _c05.deepcopy(PNJ['verina_cripta'])
PNJ['verina_retenida']["ojos"] = "verde"

# ESCENAS-05 verina_camino
import copy as _c05
PNJ['verina_camino'] = _c05.deepcopy(PNJ['verina_cripta'])
PNJ['verina_camino']["ojos"] = "verde"

# ESCENAS-05 captor_verina
import copy as _c05
PNJ['captor_verina'] = _c05.deepcopy(PNJ['sectario_perdido'])
PNJ['captor_verina']["ojos"] = "verde"

# ESCENAS-05 devoto_verina
import copy as _c05
PNJ['devoto_verina'] = _c05.deepcopy(PNJ['sectario_perdido'])
PNJ['devoto_verina']["ojos"] = "verde"

# ESCENAS-05 venides_casco
import copy as _c05
PNJ['venides_casco'] = _c05.deepcopy(PNJ['venides'])
PNJ['venides_casco']["ojos"] = "verde"

PNJ["captor_verina"].setdefault("equipo", {})["cabeza"] = "capucha_parda"
PNJ["captor_verina"]["_varia"] = {"cabeza": ["capucha_parda"]}

PNJ["devoto_verina"].setdefault("equipo", {})["cabeza"] = "capucha_purpura"
PNJ["devoto_verina"]["_varia"] = {"cabeza": ["capucha_purpura"]}

PNJ["verina_retenida"].setdefault("equipo", {}).pop("mano_izq", None)

# ESCENAS-05 skilla_furia
import copy as _c05
PNJ['skilla_furia'] = _c05.deepcopy(PNJ['skilla'])
PNJ['skilla_furia']["ojos"] = "verde"

# ESCENAS-05 perdido_antorcha
import copy as _c05
PNJ['perdido_antorcha'] = _c05.deepcopy(PNJ['saqueador_antorcha'])
PNJ['perdido_antorcha']["ojos"] = "verde"

# ESCENAS-05 pulga_tejado
import copy as _c05
PNJ['pulga_tejado'] = _c05.deepcopy(PNJ['carterista'])
PNJ['pulga_tejado']["ojos"] = "verde"

PNJ["skilla_furia"].setdefault("equipo", {})["arma"] = "espada_ceniza"

# ESCENAS-05 sandes_ocaso
import copy as _c05
PNJ['sandes_ocaso'] = _c05.deepcopy(PNJ['sandes_cripta'])
PNJ['sandes_ocaso']["ojos"] = "verde"

# ESCENAS-05 sectario_portal
import copy as _c05
PNJ['sectario_portal'] = _c05.deepcopy(PNJ['sectario_perdido'])
PNJ['sectario_portal']["ojos"] = "verde"

# ESCENAS-05 fanatico_escalera
import copy as _c05
PNJ['fanatico_escalera'] = _c05.deepcopy(PNJ['sectario_perdido'])
PNJ['fanatico_escalera']["ojos"] = "verde"

# ESCENAS-05 verina_segunda
import copy as _c05
PNJ['verina_segunda'] = _c05.deepcopy(PNJ['verina_cripta'])
PNJ['verina_segunda']["ojos"] = "verde"

PNJ["sectario_portal"].setdefault("equipo", {})["cabeza"] = "capucha_negra"
PNJ["sectario_portal"]["_varia"] = {"cabeza": ["capucha_negra"]}

PNJ["fanatico_escalera"].setdefault("equipo", {})["cabeza"] = "capucha_roja"
PNJ["fanatico_escalera"]["_varia"] = {"cabeza": ["capucha_roja"]}

PNJ["verina_segunda"].setdefault("equipo", {}).pop("mano_izq", None)

# ESCENAS-06 sandes_final
import copy as _c06
PNJ['sandes_final'] = _c06.deepcopy(PNJ['sandes_cripta'])
PNJ['sandes_final']["ojos"] = "verde"

# ESCENAS-06 venides_final
import copy as _c06
PNJ['venides_final'] = _c06.deepcopy(PNJ['venides'])
PNJ['venides_final']["ojos"] = "verde"

# ESCENAS-06 mago_de_las_tinieblas
import copy as _c06
PNJ['mago_de_las_tinieblas'] = _c06.deepcopy(PNJ['estudiante'])
PNJ['mago_de_las_tinieblas']["ojos"] = "verde"

# ESCENAS-06 guardabosques
import copy as _c06
PNJ['guardabosques'] = _c06.deepcopy(PNJ['ben_kafka'])
PNJ['guardabosques']["ojos"] = "verde"

# ESCENAS-06 guerrero_demoniaco
import copy as _c06
PNJ['guerrero_demoniaco'] = _c06.deepcopy(PNJ['guardia'])
PNJ['guerrero_demoniaco']["ojos"] = "verde"

# ESCENAS-06 edul_retenido
import copy as _c06
PNJ['edul_retenido'] = _c06.deepcopy(PNJ['edul_cripta'])
PNJ['edul_retenido']["ojos"] = "verde"

# ESCENAS-06 edul_retenido_fuera
import copy as _c06
PNJ['edul_retenido_fuera'] = _c06.deepcopy(PNJ['edul_cripta'])
PNJ['edul_retenido_fuera']["ojos"] = "verde"

# ESCENAS-06 captor_edul
import copy as _c06
PNJ['captor_edul'] = _c06.deepcopy(PNJ['sectario_perdido'])
PNJ['captor_edul']["ojos"] = "verde"

# ESCENAS-06 guardia_edul
import copy as _c06
PNJ['guardia_edul'] = _c06.deepcopy(PNJ['guardia'])
PNJ['guardia_edul']["ojos"] = "verde"

# ESCENAS-06 devoto_edul
import copy as _c06
PNJ['devoto_edul'] = _c06.deepcopy(PNJ['sectario_perdido'])
PNJ['devoto_edul']["ojos"] = "verde"

# ESCENAS-06 edul_afueras
import copy as _c06
PNJ['edul_afueras'] = _c06.deepcopy(PNJ['edul_cripta'])
PNJ['edul_afueras']["ojos"] = "verde"

# ESCENAS-06 verina_abrazo
import copy as _c06
PNJ['verina_abrazo'] = _c06.deepcopy(PNJ['verina_cripta'])
PNJ['verina_abrazo']["ojos"] = "verde"

PNJ["captor_edul"].setdefault("equipo", {})["cabeza"] = "capucha_verde"
PNJ["captor_edul"]["_varia"] = {"cabeza": ["capucha_verde"]}

PNJ["devoto_edul"].setdefault("equipo", {})["cabeza"] = "capucha_gris"
PNJ["devoto_edul"]["_varia"] = {"cabeza": ["capucha_gris"]}

PNJ["edul_retenido_fuera"].setdefault("equipo", {}).pop("arma", None)
PNJ["edul_retenido"].setdefault("equipo", {}).pop("arma", None)
PNJ["edul_afueras"].setdefault("equipo", {}).pop("arma", None)

# ESCENAS-06 gorran_afueras
import copy as _c06
PNJ['gorran_afueras'] = _c06.deepcopy(PNJ['gorran'])
PNJ['gorran_afueras']["ojos"] = "verde"

# ESCENAS-06 bandido_gorran_afueras
import copy as _c06
PNJ['bandido_gorran_afueras'] = _c06.deepcopy(PNJ['bandido_gorran'])
PNJ['bandido_gorran_afueras']["ojos"] = "verde"

# ESCENAS-06 zarela_afueras
import copy as _c06
PNJ['zarela_afueras'] = _c06.deepcopy(PNJ['zarela'])
PNJ['zarela_afueras']["ojos"] = "verde"

# ESCENAS-06 guerrero_afueras
import copy as _c06
PNJ['guerrero_afueras'] = _c06.deepcopy(PNJ['guerrero_gitano'])
PNJ['guerrero_afueras']["ojos"] = "verde"

# ESCENAS-06 teo_afueras
import copy as _c06
PNJ['teo_afueras'] = _c06.deepcopy(PNJ['cabecilla_saqueadores'])
PNJ['teo_afueras']["ojos"] = "verde"

# ESCENAS-06 venides_rodillas
import copy as _c06
PNJ['venides_rodillas'] = _c06.deepcopy(PNJ['venides'])
PNJ['venides_rodillas']["ojos"] = "verde"

# ESCENAS-06 guardia_venides
import copy as _c06
PNJ['guardia_venides'] = _c06.deepcopy(PNJ['guardia'])
PNJ['guardia_venides']["ojos"] = "verde"

# ESCENAS-06 venides_huida
import copy as _c06
PNJ['venides_huida'] = _c06.deepcopy(PNJ['venides'])
PNJ['venides_huida']["ojos"] = "verde"

# ESCENAS-06 zarela_huida
import copy as _c06
PNJ['zarela_huida'] = _c06.deepcopy(PNJ['zarela'])
PNJ['zarela_huida']["ojos"] = "verde"

# GORRAN-TRAMA (22/10/2026) — la gente de la aldea de Gorran. Victor, Asdrugal y Joel son de Oriol; el resto, propuesta.
import copy as _cg
PNJ["victor_yungblood"] = P("Victor Yungblood", AEG, "m", ojos="azul", pelo=("corto", "pelo_gris"),
                            barba=("BARBA_CORTA", "pelo_gris"),
                            equipo={"cabeza": "sombrero_cuero", "torso": "aventurero_pardo", "piernas": "pantalon_pardo",
                                    "pies": "botas", "arma": "daga_hierro"}, _unico=True)
PNJ["asdrugal_viejo"] = P("Asdrugal el Viejo", AEG, "m", ojos="marron", pelo=("largo", "pelo_blanco"),
                          barba=("BARBA", "pelo_blanco"),
                          equipo={"torso": "tunica_parda", "piernas": "pantalon_gris", "pies": "botas",
                                  "arma": "baston_roble", "mano_izq": "jarra"}, _unico=True)
PNJ["joel_joran"] = P("Joel Joran", "orcos_dramaticos", "m", ojos="amarillo",
                      equipo={"torso": "camisa_chaleco_granate", "piernas": "calzas_rojas", "pies": "zapatos",
                              "capa": "capa_corta_roja"}, _unico=True)
PNJ["joel_pujador"] = _cg.deepcopy(PNJ["joel_joran"])
PNJ["joel_pujador"]["nombre"] = "Pujador enmascarado"
PNJ["joel_pujador"]["equipo"] = dict(PNJ["joel_pujador"]["equipo"], cabeza="mascara_porcelana", capa="capa_roja",
                                     torso="casaca_granate")
for _id, _base in (("victor_subasta", "victor_yungblood"), ("asdrugal_subasta", "asdrugal_viejo")):
    PNJ[_id] = _cg.deepcopy(PNJ[_base])
for _id in ("gorran_tajo", "gorran_subasta"):
    PNJ[_id] = _cg.deepcopy(PNJ["gorran"])
    PNJ[_id]["_unico"] = True
PNJ["cojo"] = P("El Cojo", AEG, "m", ojos="negro", pelo=("corto", "pelo_negro"), barba=("BARBA_CORTA", "pelo_negro"),
                equipo={"torso": "casaca_granate", "piernas": "pantalon_negro", "pies": "botas_altas",
                        "arma": "baston_roble", "mano_izq": "libro_cuentas"}, _unico=True)
for _id in ("cojo_tajo", "cojo_subasta"):
    PNJ[_id] = _cg.deepcopy(PNJ["cojo"])
PNJ["matones_cojo"] = _cg.deepcopy(PNJ["bandido_gorran"])
PNJ["matones_cojo"]["nombre"] = "Matón del Cojo"
PNJ["reclutador_gorran"] = _cg.deepcopy(PNJ["bandido_gorran"])
PNJ["reclutador_gorran"]["nombre"] = "Reclutador"
PNJ["tabernera_tajo"] = P("La tabernera de El Tajo", AEG, "f", ojos="verde", pelo=("recogido", "pelo_castano"), barba=None,
                          equipo={"torso": "blusa_corse", "piernas": "falda_verde", "pies": "zapatos", "mano_izq": "jarra"},
                          _unico=True)
PNJ["serra"] = P("Doña Serra", AEG, "f", ojos="ambar", pelo=("recogido", "pelo_negro"), barba=None,
                 equipo={"torso": "vestido_rojo_delantal", "pies": "zapatos"}, _unico=True)
PNJ["herrero_sucio"] = _cg.deepcopy(PNJ["aigren"])
PNJ["herrero_sucio"]["nombre"] = "El herrero sucio"
PNJ["herrero_sucio"]["ojos"] = "marron"
PNJ["recadero"] = P("El recadero", AEG, "m", ojos="marron", pelo=("greñas", "pelo_castano"), barba=None,
                    equipo={"torso": "camisa_chaleco", "piernas": "pantalon_pardo", "pies": "zuecos"}, _unico=True)
PNJ["contrabandista"] = _cg.deepcopy(PNJ["bandido_gorran"])
PNJ["contrabandista"]["nombre"] = "Contrabandista"
PNJ["herido_aldea"] = _cg.deepcopy(PNJ["parroquiano_humilde"])
PNJ["herido_aldea"]["nombre"] = "Un herido"
PNJ["herido_aldea"].pop("_varia", None)
PNJ["madre_aldea"] = _cg.deepcopy(PNJ["bordadora"])
PNJ["madre_aldea"]["nombre"] = "Una madre de la aldea"
PNJ["pujador_mercader"] = _cg.deepcopy(PNJ["mercader"])
PNJ["pujador_mercader"]["nombre"] = "Pujador de Killville"
PNJ["pujador_mercader"].pop("_varia", None)
PNJ["pujador_noble"] = _cg.deepcopy(PNJ["mercader"])
PNJ["pujador_noble"]["nombre"] = "Pujador con anillos"
PNJ["pujador_noble"].pop("_varia", None)
PNJ["pujador_noble"]["ojos"] = "azul"
for _id in ("reclutador_gorran", "matones_cojo", "contrabandista", "herrero_sucio", "herido_aldea", "madre_aldea",
            "pujador_mercader", "pujador_noble", "victor_subasta", "asdrugal_subasta", "cojo_tajo", "cojo_subasta",
            "joel_pujador"):
    PNJ[_id]["_unico"] = True

# GORRAN-VIDA (22/10/2026): la gente de las notas de escena de los mapas de Gorran.
PNJ['nino_aldea'] = P('Un crío de la aldea', AEG, 'm', ojos='marron', pelo=('greñas', 'pelo_castano'), barba=None, equipo={'torso': 'tunica_parda', 'piernas': 'pantalon_pardo', 'pies': 'zuecos'}, _unico=True)
PNJ['nina_aldea'] = P('Una niña con un palo', AEG, 'f', ojos='verde', pelo=('coleta', 'pelo_rubio'), barba=None, equipo={'torso': 'vestido_azul_delantal', 'pies': 'zuecos'}, _unico=True)
PNJ['lavandera_aldea'] = P('La lavandera', AEG, 'f', ojos='marron', pelo=('recogido', 'pelo_negro'), barba=None, equipo={'cabeza': 'panuelo_azul', 'torso': 'vestido_lino', 'pies': 'zuecos', 'mano_izq': 'cesta'}, _unico=True)
PNJ['pescador_aldea'] = P('Un viejo que pesca', AEG, 'm', ojos='azul', pelo=('corto', 'pelo_blanco'), barba=('BARBA', 'pelo_blanco'), equipo={'cabeza': 'sombrero_paja', 'torso': 'tunica_gris', 'piernas': 'pantalon_pardo', 'pies': 'botas'}, _unico=True)
PNJ['borracho_tajo'] = P('Un borracho', AEG, 'm', ojos='marron', pelo=('greñas', 'pelo_negro'), barba=('BARBA_CORTA', 'pelo_negro'), equipo={'torso': 'camisa_chaleco', 'piernas': 'pantalon_oliva', 'pies': 'zuecos', 'mano_izq': 'jarra'}, _unico=True)
PNJ['setera_aldea'] = P('La vieja de las setas', AEG, 'f', ojos='ambar', pelo=('recogido', 'pelo_gris'), barba=None, equipo={'cabeza': 'toca', 'torso': 'vestido_ocre', 'pies': 'zapatos', 'mano_izq': 'cesta'}, _unico=True)
PNJ['ancianos_familias'] = P('Dos ancianos desconfiados', AEG, 'm', ojos='azul', pelo=('corto', 'pelo_gris'), barba=('BARBA', 'pelo_gris'), equipo={'torso': 'tunica_parda', 'piernas': 'pantalon_gris', 'pies': 'zuecos'}, _unico=True)
PNJ['neus_serra'] = P('Neus', AEG, 'f', ojos='verde', pelo=('largo', 'pelo_pelirrojo'), barba=None, equipo={'torso': 'vestido_teja', 'pies': 'zapatos'}, _unico=True)
PNJ['bandido_cuartel'] = P('Bandido fuera de turno', AEG, 'm', ojos='marron', pelo=('corto', 'pelo_castano'), barba=('BARBA_CORTA', 'pelo_castano'), equipo={'torso': 'cuero', 'piernas': 'pantalon_gris', 'pies': 'botas'}, _unico=True)
PNJ['bandido_cuartel_2'] = P('Bandido que afila', AEG, 'm', ojos='negro', pelo=('greñas', 'pelo_negro'), barba=None, equipo={'cabeza': 'cp_panuelo_forajido', 'torso': 'cuero_oscuro', 'piernas': 'pantalon_pardo', 'pies': 'botas', 'arma': 'hacha_hierro'}, _unico=True)
PNJ['contable_almacen'] = P('El contable del almacén', AEG, 'm', ojos='azul', pelo=('corto', 'pelo_oscuro'), barba=None, equipo={'torso': 'casaca_negra', 'piernas': 'pantalon_negro', 'pies': 'zapatos', 'mano_izq': 'libro_cuentas'}, _unico=True)
PNJ['escribiente_gorran'] = P('El escribiente de Gorran', AEG, 'm', ojos='marron', pelo=('corto', 'pelo_rubio'), barba=None, equipo={'cabeza': 'birrete', 'torso': 'tunica_azul', 'piernas': 'pantalon_gris', 'pies': 'zapatos', 'mano_izq': 'libro_cuentas'}, _unico=True)
PNJ['cocinera_recinto'] = P('La cocinera', AEG, 'f', ojos='marron', pelo=('recogido', 'pelo_castano'), barba=None, equipo={'cabeza': 'cofia', 'torso': 'vestido_verde_delantal', 'pies': 'zuecos'}, _unico=True)
PNJ['pinche_recinto'] = P('El pinche', AEG, 'm', ojos='azul', pelo=('greñas', 'pelo_rubio'), barba=None, equipo={'torso': 'camisa_sin_mangas', 'piernas': 'pantalon_pardo', 'pies': 'zuecos'}, _unico=True)
PNJ['musico_recinto'] = P('Un músico contratado', AEG, 'm', ojos='verde', pelo=('largo', 'pelo_castano'), barba=None, equipo={'cabeza': 'sombrero_pluma', 'torso': 'camisa_chaleco_verde', 'piernas': 'calzas_verdes', 'pies': 'botas_claras', 'capa': 'capa_corta_parda'}, _unico=True)
PNJ['juerguista_recinto'] = P('Un bandido de juerga', AEG, 'm', ojos='marron', pelo=('corto', 'pelo_negro'), barba=('BARBA', 'pelo_negro'), equipo={'torso': 'cuero', 'piernas': 'pantalon_gris', 'pies': 'botas', 'mano_izq': 'jarra'}, _unico=True)
PNJ['juerguista_recinto_2'] = P('Una contrabandista de juerga', AEG, 'f', ojos='ambar', pelo=('coleta', 'pelo_negro'), barba=None, equipo={'torso': 'cuero_oscuro', 'piernas': 'pantalon_negro', 'pies': 'botas_altas', 'mano_izq': 'jarra'}, _unico=True)
PNJ['preso_calabozo'] = P('Teobaldo, mercader de Killville', AEG, 'm', ojos='azul', pelo=('corto', 'pelo_gris'), barba=('BARBA_CORTA', 'pelo_gris'), equipo={'torso': 'tunica_mercader', 'pies': 'zapatos'}, _unico=True)
PNJ['criada_gorran'] = P('La criada de la casa', AEG, 'f', ojos='verde', pelo=('recogido', 'pelo_rubio'), barba=None, equipo={'cabeza': 'cofia', 'torso': 'vestido_gris', 'pies': 'zapatos'}, _unico=True)
PNJ['bandido_patio'] = P('Bandido que entrena', AEG, 'm', ojos='marron', pelo=('corto', 'pelo_castano'), barba=None, equipo={'torso': 'camisa_sin_mangas', 'piernas': 'pantalon_gris', 'pies': 'botas', 'arma': 'espada_hierro'}, _unico=True)
PNJ['bandido_patio_2'] = P('Bandida que entrena', AEG, 'f', ojos='negro', pelo=('coleta', 'pelo_castano'), barba=None, equipo={'torso': 'cuero', 'piernas': 'pantalon_pardo', 'pies': 'botas', 'arma': 'lanza_hierro'}, _unico=True)
PNJ['aduanero_galeria'] = P('El aduanero', AEG, 'm', ojos='marron', pelo=('corto', 'pelo_gris'), barba=('BARBA_CORTA', 'pelo_gris'), equipo={'cabeza': 'chapel', 'torso': 'casaca_negra', 'piernas': 'pantalon_gris', 'pies': 'botas', 'mano_izq': 'libro_cuentas'}, _unico=True)
PNJ['estibador_galeria'] = P('Un estibador', AEG, 'm', ojos='marron', pelo=('greñas', 'pelo_castano'), barba=None, equipo={'torso': 'camisa_sin_mangas', 'piernas': 'pantalon_oliva', 'pies': 'botas'}, _unico=True)
PNJ['estibadora_galeria'] = P('Una estibadora', AEG, 'f', ojos='marron', pelo=('recogido', 'pelo_negro'), barba=None, equipo={'torso': 'camisa_chaleco_oscuro', 'piernas': 'pantalon_pardo', 'pies': 'botas'}, _unico=True)
PNJ['carpintero_galeria'] = P('El carpintero del taller', AEG, 'm', ojos='azul', pelo=('corto', 'pelo_rubio'), barba=('BARBA', 'pelo_rubio'), equipo={'torso': 'mandil_herrero', 'piernas': 'pantalon_pardo', 'pies': 'botas', 'arma': 'martillo_acero'}, _unico=True)
PNJ['agotado_galeria'] = P('Un trabajador agotado', AEG, 'm', ojos='marron', pelo=('greñas', 'pelo_oscuro'), barba=('BARBA_CORTA', 'pelo_oscuro'), equipo={'torso': 'harapos', 'piernas': 'pantalon_pardo', 'pies': 'sandalias'}, _unico=True)
PNJ['minero_literas'] = P('Un minero que descansa', AEG, 'm', ojos='marron', pelo=('corto', 'pelo_negro'), barba=('BARBA', 'pelo_negro'), equipo={'cabeza': 'gorro_gris', 'torso': 'camisa_sin_mangas', 'piernas': 'pantalon_gris', 'pies': 'botas'}, _unico=True)
PNJ['minera_comedor'] = P('Una minera que come', AEG, 'f', ojos='verde', pelo=('coleta', 'pelo_castano'), barba=None, equipo={'cabeza': 'panuelo_rojo', 'torso': 'camisa_chaleco', 'piernas': 'pantalon_oliva', 'pies': 'botas'}, _unico=True)
PNJ['minero_extraccion'] = P('Un minero con pico', AEG, 'm', ojos='azul', pelo=('corto', 'pelo_castano'), barba=None, equipo={'cabeza': 'gorro_gris', 'torso': 'camisa_sin_mangas', 'piernas': 'pantalon_pardo', 'pies': 'botas', 'arma': 'martillo_acero'}, _unico=True)
PNJ['sectario_mina'] = P('Un hombre de capa gris', AEG, 'm', ojos='negro', pelo=('corto', 'pelo_negro'), barba=None, equipo={'cabeza': 'capucha_gris', 'torso': 'habito_gris', 'pies': 'sandalias'}, _unico=True)
PNJ['sectaria_mina'] = P('Una mujer de capa gris', AEG, 'f', ojos='azul', pelo=('largo', 'pelo_negro'), barba=None, equipo={'cabeza': 'capucha_gris', 'torso': 'habito_gris', 'pies': 'sandalias'}, _unico=True)
PNJ['viejo_minero_mina2'] = P('El último minero', AEG, 'm', ojos='azul', pelo=('largo', 'pelo_blanco'), barba=('BARBA', 'pelo_blanco'), equipo={'torso': 'tunica_gris', 'piernas': 'pantalon_gris', 'pies': 'botas', 'arma': 'baston_roble'}, _unico=True)
PNJ['vigia_cojo'] = P('El vigía del Cojo', AEG, 'm', ojos='negro', pelo=('corto', 'pelo_negro'), barba=None, equipo={'cabeza': 'capucha_negra', 'torso': 'cuero_oscuro', 'piernas': 'pantalon_negro', 'pies': 'botas', 'arma': 'daga_hierro'}, _unico=True)

# GORRAN-DIA2 misión A
PNJ['iris_serra'] = P('Iris', AEG, 'f', ojos='verde', pelo=('largo', 'pelo_pelirrojo'), barba=None, equipo={'torso': 'vestido_verde_delantal', 'pies': 'zapatos'}, _unico=True)
PNJ['mercenario_tajo'] = P('Mercenario de Killville', AEG, 'm', ojos='marron', pelo=('corto', 'pelo_negro'), barba=('BARBA_CORTA', 'pelo_negro'), equipo={'cabeza': 'casco_cuernos', 'torso': 'gambeson', 'piernas': 'grebas_cuero', 'pies': 'botas', 'arma': 'espada_hierro'}, _unico=True)
PNJ['morvan'] = P('Morvan Tallapedra', AEG, 'm', ojos='negro', pelo=('corto', 'pelo_gris'), barba=('BARBA', 'pelo_gris'), equipo={'torso': 'escamas_bronce', 'piernas': 'grebas_malla', 'pies': 'botas_acero', 'capa': 'capa_parda', 'arma': 'espada_acero'}, _unico=True)
PNJ['garsa'] = P('La Garsa', AEG, 'f', ojos='ambar', pelo=('coleta', 'pelo_negro'), barba=None, equipo={'torso': 'cuero_oscuro', 'piernas': 'pantalon_negro', 'pies': 'botas_altas', 'arma': 'daga_hierro'}, _unico=True)
PNJ['mercenario_morvan'] = P('Mercenario de Morvan', AEG, 'm', ojos='azul', pelo=('greñas', 'pelo_rubio'), barba=('BARBA_CORTA', 'pelo_rubio'), equipo={'torso': 'gambeson', 'piernas': 'grebas_cuero', 'pies': 'botas', 'arma': 'hacha_hierro'}, _unico=True)

# GORRAN-DIA2 misión B
PNJ['tecla_rocafort'] = P('Tecla Rocafort, la capataz', AEG, 'f', ojos='marron', pelo=('recogido', 'pelo_gris'), barba=None, equipo={'cabeza': 'gorro_gris', 'torso': 'cuero', 'piernas': 'pantalon_pardo', 'pies': 'botas', 'mano_izq': 'farol'}, _unico=True)
PNJ['bru_minero'] = P('Bru, minero', AEG, 'm', ojos='azul', pelo=('greñas', 'pelo_castano'), barba=('BARBA_CORTA', 'pelo_castano'), equipo={'cabeza': 'gorro_gris', 'torso': 'camisa_sin_mangas', 'piernas': 'pantalon_gris', 'pies': 'botas'}, _unico=True)
PNJ['ramon_sisdits'] = P('Ramon Sisdits', AEG, 'm', ojos='marron', pelo=('corto', 'pelo_negro'), barba=('BARBA', 'pelo_negro'), equipo={'cabeza': 'gorro_gris', 'torso': 'camisa_sin_mangas', 'piernas': 'pantalon_pardo', 'pies': 'botas', 'arma': 'martillo_acero'}, _unico=True)
PNJ['mineros_atrapados'] = P('Mineros atrapados', AEG, 'm', ojos='verde', pelo=('corto', 'pelo_castano'), barba=None, equipo={'cabeza': 'gorro_gris', 'torso': 'harapos', 'piernas': 'pantalon_gris', 'pies': 'botas'}, _unico=True)
PNJ['mineros_salvados'] = P('Los mineros de la Veta Honda', AEG, 'm', ojos='marron', pelo=('corto', 'pelo_negro'), barba=('BARBA', 'pelo_negro'), equipo={'cabeza': 'gorro_gris', 'torso': 'camisa_sin_mangas', 'piernas': 'pantalon_pardo', 'pies': 'botas', 'mano_izq': 'jarra'}, _unico=True)

# GORRAN-DIA2 misión C
PNJ['bet_saldana'] = P('Bet Saldaña, la jefa del muelle', AEG, 'f', ojos='azul', pelo=('corto', 'pelo_rubio'), barba=None, equipo={'cabeza': 'panuelo_azul', 'torso': 'cuero', 'piernas': 'pantalon_azul', 'pies': 'botas_altas', 'mano_izq': 'libro_cuentas'}, _unico=True)
PNJ['gil_fumat'] = P('Gil Fumat', AEG, 'm', ojos='negro', pelo=('greñas', 'pelo_negro'), barba=('BARBA_CORTA', 'pelo_negro'), equipo={'cabeza': 'capucha_negra', 'torso': 'cuero_oscuro', 'piernas': 'pantalon_negro', 'pies': 'botas', 'arma': 'daga_hierro'}, _unico=True)
PNJ['lluc_fumat'] = P('Lluc Fumat', AEG, 'm', ojos='negro', pelo=('greñas', 'pelo_castano'), barba=None, equipo={'torso': 'camisa_chaleco_oscuro', 'piernas': 'pantalon_pardo', 'pies': 'botas', 'arma': 'daga_hierro'}, _unico=True)
PNJ['fumat_galeria'] = P('Los hermanos Fumat', AEG, 'm', ojos='negro', pelo=('greñas', 'pelo_negro'), barba=('BARBA_CORTA', 'pelo_negro'), equipo={'cabeza': 'capucha_negra', 'torso': 'cuero_oscuro', 'piernas': 'pantalon_negro', 'pies': 'botas', 'arma': 'daga_hierro'}, _unico=True)

# GORRAN-DIA2 misión D
PNJ['candela'] = P('La hermana Candela', AEG, 'f', ojos='verde', pelo=('largo', 'pelo_castano'), barba=None, equipo={'cabeza': 'capucha_gris', 'torso': 'habito_gris', 'pies': 'sandalias', 'mano_izq': 'grimorio'}, _unico=True)
PNJ['orfeu'] = P('El diácono Orfeu', AEG, 'm', ojos='negro', pelo=('corto', 'pelo_blanco'), barba=None, equipo={'cabeza': 'capucha_purpura', 'torso': 'casulla', 'pies': 'sandalias', 'arma': 'baston_cristal'}, _unico=True)
PNJ['devoto_puerta'] = P('Un devoto de gris', AEG, 'm', ojos='marron', pelo=('corto', 'pelo_castano'), barba=('BARBA_CORTA', 'pelo_castano'), equipo={'cabeza': 'capucha_gris', 'torso': 'habito_harapiento', 'pies': 'sandalias', 'arma': 'daga_hierro'}, _unico=True)
PNJ['tinet_atado'] = P('Tinet', AEG, 'm', ojos='marron', pelo=('greñas', 'pelo_castano'), barba=None, equipo={'torso': 'tunica_parda', 'piernas': 'pantalon_pardo', 'pies': 'zuecos'}, _unico=True)
PNJ['mia_atada'] = P('Mia', AEG, 'f', ojos='verde', pelo=('coleta', 'pelo_rubio'), barba=None, equipo={'torso': 'vestido_azul_delantal', 'pies': 'zuecos'}, _unico=True)
