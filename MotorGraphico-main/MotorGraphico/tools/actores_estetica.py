"""ESTETICA-01 — La estetica de cada objeto: forma + esencia + acabado, y las unicas.

Oriol (06/10/2026): «para cada arma y armadura necesitaremos vincularlo a un
sprite, hay algunas que pueden compartir sprites, pero todas tienen que aplicar
una estetica». Y la regla: «forma + esencia + acabado + algunas unicas que sean
miticas o extremadamente raras, y obviamente las unicas, aquellas que solo se
puedan conseguir».

Cada objeto de equipment.json sale con:

  forma     la pieza (la fila del atlas): para las armas, las formas de
            actores_formas (f_daga, f_sable, f_rodela...); para la ropa, la pieza
            que ya elegia actores_catalogo por el nombre.
  esencia   la paleta de lo principal: las veinte esencias de las armas (Daga de
            Plaga, Sable Ligero de Luna...), las veinte deidades de las reliquias,
            o el tema del nombre o de las etiquetas (Sagrado, Arcano, Sigilo...).
  acabado   la guarnicion y el brillo: Milicia (acero y azul), Viaje (hierro y
            cuero), Insigne (plata), Sublime (oro); en las armas de esencia, la
            rareza (laton, plata, oro, oro rojo).
  unica     las armas con nombre («un arma con nombre se gana el nombre») y las
            miticas de las deidades: dibujo propio (actores_formas.UNICAS, sin tinte:
            ya lleva sus materiales). Las que aun no lo tienen, su forma con su tema.

El tinte es [(material de la pieza, material del objeto)]: el motor y el probador
cambian los cinco tonos de cada uno (ManifiestoPersonajes::paletas). Las paletas
nuevas estan en actores_materiales.
"""
import re

import particulas as _particulas
from actores_catalogo import _casa, normalizar, pieza_de_objeto
import actores_formas

# --- que es cada material ---------------------------------------------------------------

TIPO = {}
for _m in ("hierro", "acero", "bronce", "oro_viejo", "plata", "ceniza"):
    TIPO[_m] = "metal"
TIPO.update({"malla": "malla", "laton": "adorno", "oro_emblema": "adorno"})
for _m in ("cuero", "cuero_oscuro", "guante", "zapato", "sandalia", "chaleco_marron"):
    TIPO[_m] = "cuero"
for _m in ("tunica_verde", "tunica_azul", "tunica_roja", "tunica_parda", "tabardo_azul", "lino",
           "habito_purpura", "vestido_teja", "lana_purpura", "pantalon_pardo", "pantalon_gris",
           "pantalon_oliva", "habito_gris", "lana_gris", "gambeson", "tela_negra", "tela_blanca",
           "tela_roja_oscura", "tela_azul_noche", "tela_amarilla", "terciopelo_rojo", "harapo",
           "remiendo", "fieltro_verde", "tela_naranja", "tela_azul_marino", "tela_turquesa"):
    TIPO[_m] = "tela"
for _m in ("madera", "madera_oscura", "roble"):
    TIPO[_m] = "madera"
for _m in ("gema_roja", "gema_azul", "gema_verde", "cristal_azul"):
    TIPO[_m] = "gema"
TIPO.update({"paja": "paja", "hojas": "hojas", "porcelana": "porcelana", "pluma_roja": "pluma",
             "cuerda": "cuerda", "llama": "llama", "cuerno_claro": "cuerno"})

# --- temas: tipo de material -> material del objeto ---------------------------------------
# Claves: metal, hoja (la hoja o cabeza de un arma; si no, metal), malla, adorno
# (hebillas, remates; la guarnicion de un arma si no hay 'guarnicion'), cuero, mango
# (el puno de un arma; si no, cuero), tela, tela2 (la segunda tela de una prenda),
# madera, gema, paja, hojas, porcelana, pluma, cuerno, llama.

TEMAS = {
    # acabados de lo comun
    "milicia": {"hoja": "acero", "metal": "acero", "adorno": "hierro", "guarnicion": "hierro",
                "mango": "tabardo_azul", "cuero": "cuero_oscuro", "tela": "tabardo_azul",
                "madera": "madera_oscura", "blason": "tela_blanca"},
    "viaje": {"hoja": "hierro", "metal": "hierro", "adorno": "bronce", "guarnicion": "bronce",
              "mango": "cuero", "cuero": "cuero", "tela": "tunica_parda", "madera": "roble",
              "blason": "cuero"},
    # por etiquetas
    "sagrado": {"metal": "plata", "hoja": "plata", "adorno": "oro_viejo", "guarnicion": "oro_viejo",
                "tela": "tela_blanca", "tela2": "tela_amarilla", "cuero": "cuero_leon", "mango": "cuero_leon",
                "madera": "fresno", "gema": "gema_azul", "malla": "malla_plata", "blason": "tela_amarilla",
                "porcelana": "tela_marfil"},
    "arcano": {"metal": "plata", "hoja": "plata", "adorno": "plata", "tela": "tela_azul_noche",
               "tela2": "tela_marfil", "cuero": "cuero_oscuro", "madera": "madera_oscura",
               "gema": "cristal_azul", "blason": "tela_azul_noche"},
    "conjuro": {"metal": "plata", "adorno": "oro_emblema", "tela": "tela_purpura_real", "tela2": "tela_marfil",
                "cuero": "cuero_oscuro", "madera": "madera_oscura", "gema": "gema_violeta"},
    "naturaleza": {"metal": "bronce", "hoja": "bronce", "adorno": "bronce", "tela": "tela_verde_oscura",
                   "tela2": "tunica_parda", "cuero": "cuero", "madera": "roble", "gema": "gema_verde",
                   "hojas": "hojas"},
    "sigilo": {"metal": "hierro_negro", "hoja": "hierro_negro", "adorno": "hierro_negro",
               "tela": "tela_negra", "tela2": "tela_gris_azul", "cuero": "cuero_negro",
               "mango": "cuero_negro", "madera": "madera_oscura", "porcelana": "tela_negra"},
    "oscuro": {"metal": "ceniza", "hoja": "ceniza", "adorno": "plata_negra", "guarnicion": "plata_negra",
               "tela": "habito_purpura", "tela2": "tela_negra", "cuero": "cuero_negro", "mango": "cuero_negro",
               "madera": "madera_oscura", "gema": "gema_violeta", "malla": "hierro_negro"},
    # las veinte deidades (reliquias)
    "llama_eterna": {"metal": "oro_rojo", "hoja": "oro_rojo", "adorno": "oro_viejo", "malla": "malla_oro",
                     "tela": "tela_naranja", "tela2": "tela_roja_oscura", "cuero": "cuero_brasa",
                     "madera": "madera_oscura", "gema": "llama", "llama": "llama", "paja": "tela_naranja"},
    "padre_forja": {"metal": "hierro_forja", "hoja": "hierro_forja", "adorno": "bronce", "malla": "malla",
                    "tela": "cuero", "tela2": "tela_ocre", "cuero": "cuero", "madera": "madera_oscura",
                    "gema": "llama"},
    "cuervo": {"metal": "hierro_negro", "hoja": "hierro_negro", "adorno": "plata", "malla": "hierro_negro",
               "tela": "tela_negra", "tela2": "pergamino", "cuero": "cuero_negro", "madera": "madera_oscura",
               "gema": "gema_azul", "pluma": "tela_negra"},
    "trueno": {"metal": "acero_tormenta", "hoja": "trueno", "adorno": "trueno", "malla": "malla_plata",
               "tela": "tela_azul_noche", "tela2": "tela_gris_azul", "cuero": "cuero_azul",
               "madera": "madera_oscura", "gema": "trueno"},
    "farol_rojo": {"metal": "laton", "hoja": "laton", "adorno": "laton", "malla": "malla_oro",
                   "tela": "terciopelo_rojo", "tela2": "tela_negra", "cuero": "cuero_vino",
                   "madera": "madera_oscura", "gema": "llama"},
    "caminante": {"metal": "hierro", "hoja": "hierro", "adorno": "hierro", "tela": "habito_gris",
                  "tela2": "lana_gris", "cuero": "cuero_polvo", "madera": "fresno", "paja": "cuero_polvo"},
    "oraculo": {"metal": "vidrio", "hoja": "vidrio", "adorno": "plata", "malla": "malla_plata",
                "tela": "tela_turquesa", "tela2": "tela_marfil", "cuero": "cuero_blanco", "gema": "vidrio"},
    "leon": {"metal": "juramento", "hoja": "juramento", "adorno": "oro_viejo", "malla": "malla_oro",
             "tela": "tela_blanca", "tela2": "tela_amarilla", "cuero": "cuero_leon", "gema": "gema_roja"},
    "mareas": {"metal": "bronce_marino", "hoja": "bronce_marino", "adorno": "plata", "malla": "malla_espuma",
               "tela": "tela_turquesa", "tela2": "tela_azul_marino", "cuero": "cuero_turquesa", "gema": "perla"},
    "sepulturero": {"metal": "hierro_negro", "hoja": "hierro_negro", "adorno": "plata_negra",
                    "malla": "hierro_negro", "tela": "tela_negra", "tela2": "lino", "cuero": "cuero_ceniza",
                    "madera": "madera_oscura", "gema": "hueso", "paja": "tela_negra"},
    "tejedora": {"metal": "plata", "hoja": "estrella", "adorno": "plata", "malla": "malla_plata",
                 "tela": "tela_azul_noche", "tela2": "tela_purpura_real", "cuero": "cuero_negro",
                 "gema": "estrella", "porcelana": "tela_azul_noche"},
    "luna_hueca": {"metal": "plata", "hoja": "luna", "adorno": "plata", "malla": "malla_plata",
                   "tela": "lana_lila", "tela2": "tela_blanca", "cuero": "cuero_negro", "gema": "luna"},
    "reina_velada": {"metal": "plata_negra", "hoja": "penumbra", "adorno": "plata_negra",
                     "malla": "hierro_negro", "tela": "habito_purpura", "tela2": "tela_negra",
                     "cuero": "cuero_violeta", "gema": "gema_violeta", "porcelana": "habito_purpura"},
    "toro": {"metal": "bronce", "hoja": "bronce", "adorno": "bronce", "malla": "malla_oro",
             "tela": "tela_roja_oscura", "tela2": "tela_ocre", "cuero": "cuero_rojo", "cuerno": "cuerno_claro"},
    "rey_lobo": {"metal": "rey_lobo", "hoja": "rey_lobo", "adorno": "plata", "malla": "malla_plata",
                 "tela": "pelaje_lobo", "tela2": "lana_gris", "cuero": "pelaje_lobo", "gema": "cristal_azul"},
    "mendigo": {"metal": "hierro_viejo", "hoja": "hierro_viejo", "adorno": "hierro_viejo",
                "tela": "harapo", "tela2": "remiendo", "cuero": "sandalia", "madera": "roble"},
    "madre_doce": {"metal": "oro_viejo", "hoja": "oro_viejo", "adorno": "oro_viejo", "malla": "malla_oro",
                   "tela": "tunica_verde", "tela2": "tela_amarilla", "cuero": "cuero_verde", "gema": "gema_verde"},
    "corte_salvaje": {"metal": "cuerno_draco", "hoja": "cuerno_draco", "adorno": "bronce",
                      "tela": "fieltro_verde", "tela2": "hojas", "cuero": "corteza", "madera": "corteza",
                      "gema": "gema_verde", "hojas": "hojas"},
    "serpiente": {"metal": "verde_serpiente", "hoja": "verde_serpiente", "adorno": "oro_viejo",
                  "malla": "malla_oro", "tela": "verde_serpiente", "tela2": "tela_negra",
                  "cuero": "escamas_verdes", "gema": "gema_verde"},
    "espiga": {"metal": "oro_viejo", "hoja": "oro_viejo", "adorno": "bronce", "tela": "tela_amarilla",
               "tela2": "lino", "cuero": "cuero", "madera": "paja", "paja": "paja", "gema": "oro_viejo"},
}

DEIDADES = {
    "la_llama_eterna": "llama_eterna", "deity_forge_father": "padre_forja",
    "el_cuervo_bibliotecario": "cuervo", "la_forja_del_trueno": "trueno",
    "la_dama_del_farol_rojo": "farol_rojo", "el_caminante_gris": "caminante",
    "el_oraculo_fragmentado": "oraculo", "el_leon_del_alba": "leon",
    "la_senora_de_las_mareas": "mareas", "el_sepulturero_amable": "sepulturero",
    "la_tejedora_de_estrellas": "tejedora", "la_luna_hueca": "luna_hueca",
    "deity_veiled_queen": "reina_velada", "el_toro_de_gongorguma": "toro",
    "deity_wolf_king": "rey_lobo", "el_hermano_mendigo": "mendigo",
    "la_madre_de_los_doce": "madre_doce", "deity_wild_court": "corte_salvaje",
    "la_serpiente_de_contratos": "serpiente", "la_espiga_dorada": "espiga",
}

# Las veinte esencias de las armas: nombre -> paleta de la hoja (y un tema para el resto)
ESENCIAS = {
    "ceniza": ("ceniza", "oscuro"), "cristal azul": ("cristal_azul", None),
    "oro viejo": ("oro_viejo", None), "plata": ("plata", None), "roble antiguo": ("roble_antiguo", None),
    "hierro santo": ("hierro_santo", "sagrado"), "luna": ("luna", None), "niebla": ("niebla", None),
    "penumbra": ("penumbra", "sigilo"), "plaga": ("plaga", None), "trueno": ("trueno", None),
    "tumba": ("tumba", None), "estrella": ("estrella", None), "juramento": ("juramento", "sagrado"),
    "rey lobo": ("rey_lobo", None), "sangre": ("sangre", None), "sombra roja": ("sombra_roja", "oscuro"),
    "corona rota": ("corona_rota", None), "eternidad": ("eternidad", None), "vacio": ("vacio", "oscuro"),
}
# La gema de las armas de esencia desde epicas (material 4 de la escalera)
GEMA_DE_ESENCIA = {
    "hierro santo": "juramento", "luna": "gema_violeta", "niebla": "niebla", "penumbra": "gema_violeta",
    "plaga": "gema_verde", "trueno": "gema_azul", "tumba": "tumba", "estrella": "gema_azul",
    "juramento": "juramento", "rey lobo": "cristal_azul", "sangre": "gema_roja", "sombra roja": "gema_roja",
    "corona rota": "corona_rota", "eternidad": "eternidad", "vacio": "gema_violeta",
}
# ESTETICA-01: desde raras, la forma sube un escalon de la escalera de detalle (actores_formas)
PASO_POR_RAREZA = {"rare": "raro", "epic": "epico", "mythic": "mitico"}
FORMAS_DE_ESENCIA = {"cuchillo de cazador": "f_cuchillo", "daga": "f_daga", "estilete": "f_estilete",
                     "sable ligero": "f_sable", "hacha de mano": "f_hacha_mano"}
# La guarnicion de las armas de esencia dice su rareza
GUARNICION_POR_RAREZA = {"uncommon": None, "rare": "plata", "epic": "oro_viejo", "mythic": "oro_rojo"}

# Temas por el nombre (las series poco comunes, sus Insigne y Sublime, y las armas con
# nombre). Clave: palabras del nombre (sin Insigne ni Sublime).
# CP-01: el tema del Juramento va pieza a pieza (abajo)
_TEMA_JURAMENTO = {"hoja": "acero_juramento", "metal": "acero_juramento", "madera": "cuero_herrero",
                   "guarnicion": "laton_juramento", "gema": "llama"}
TEMAS_POR_NOMBRE = [
    # SETS-03: los sets de las seis clases de la 3.a iteracion (tools/sets_iteracion3.py; lo propio, en
    # actores_trajes_iteracion3). Como los de las SP y las clases propias: la ropa propia va dibujada con sus
    # colores (paletas sin tipo) y el tema tine el arma y las piezas del catalogo que el set reutiliza (y da al
    # set las particulas de su clase). Delante de todos. El «Collar de Dientes de la Furia» y el «Rosario del
    # Despertar» (abalorios de otra serie) no se ven puestos: estetica() no los mira.
    ("del apostata", {"hoja": "hoja_apostata", "mango": "cuero_negro", "guarnicion": "hierro_negro",
                      "gema": "gema_roja", "metal": "acero_apostata", "tela": "acero_apostata"}),
    # SETS-04: los tres sets del Senor de las Armas (el orbe: el cristal, de su luz; la montura, de su metal)
    ("del duque", {"gema": "mana_duque", "metal": "oro_duque", "guarnicion": "oro_duque"}),
    ("del soberano sin rostro", {"gema": "visera_soberano", "metal": "oro_soberano", "guarnicion": "oro_soberano"}),
    ("del rey de la forja", {"gema": "oro_fundido", "metal": "hierro_rey", "guarnicion": "hierro_rey"}),
    ("de la furia", {"hoja": "oro_arsenal", "madera": "cuero_rojo_furia", "guarnicion": "blanco_furia",
                     "metal": "blanco_furia", "adorno": "oro_arsenal"}),
    ("de las cuatro estaciones", {"hoja": "acero_escarcha", "mango": "verde_primavera", "guarnicion": "oro_verano",
                                  "gema": "rojo_otono", "cuero": "cuero_estaciones"}),
    ("del chasqueador", {"cuero": "guante_chasqueador", "mango": "guante_chasqueador",
                         "guarnicion": "violeta_chasqueador", "gema": "gema_violeta", "tela": "negro_chasqueador"}),
    ("del despertar", {"hoja": "jade", "metal": "jade", "mango": "marfil", "guarnicion": "oro_despertar",
                       "gema": "gema_verde", "cuero": "marfil"}),
    # SP-02, el Artista Marcial (actores_trajes_artista): los pantalones, las grebas y el calzado del catalogo,
    # y los guanteletes. Delante de los demas: «del guerrero demoniaco» antes que «del guerrero».
    ("del puno del dragon", {"hoja": "granate_dragon", "mango": "negro_dragon", "guarnicion": "oro_sp",
                             "gema": "llama"}),
    ("de las artes misticas", {"hoja": "azul_mistico", "mango": "blanco_mistico", "guarnicion": "plata_mistica",
                               "gema": "loto_mistico"}),
    ("del maestro lobo", {"hoja": "plata_lobo", "mango": "pelaje_oscuro_lobo", "guarnicion": "plata_lobo",
                          "gema": "sol_cazasoles", "tela": "pizarra_lobo"}),
    ("del guerrero demoniaco", {"hoja": "rojo_demonio", "mango": "negro_demonio", "guarnicion": "rojo_demonio",
                                "gema": "gema_roja", "tela": "negro_demonio", "cuero": "negro_demonio"}),
    ("del druida flamigero", {"hoja": "lava_volcano", "mango": "oso_druida", "guarnicion": "cuero_druida",
                              "gema": "llama", "tela": "cuero_druida"}),
    ("del puno hidraulico", {"hoja": "bronce_hidraulico", "mango": "hierro_hidraulico", "guarnicion": "laton_tormenta",
                             "gema": "violeta_tinieblas", "tela": "cuero_hidraulico", "cuero": "hierro_hidraulico"}),
    ("del tronador", {"hoja": "acero", "mango": "amarillo_tronador", "guarnicion": "cobre_tronador", "gema": "trueno",
                      "tela": "negro_tronador", "cuero": "negro_tronador"}),
    ("de thor$", {"hoja": "hielo_thor", "mango": "azul_thor", "guarnicion": "oro_sp", "gema": "trueno",
                  "metal": "plata_thor"}),
    # SP-02: los sets de especialista. La ropa propia ya va dibujada con sus colores en
    # actores_trajes (paletas sin tipo: ningun tema la toca); el tema tine el arma y las piezas del
    # catalogo que el set reutiliza (quijotes, escarpes, pantalones, sandalias, botas altas)
    ("del guerrero", {"hoja": "acero", "mango": "cuero_rojo", "guarnicion": "oro_viejo"}),
    ("del ninja", {"hoja": "acero", "mango": "cuero_negro", "guarnicion": "hierro_negro", "gema": "gema_azul"}),
    ("del cruzado", {"metal": "plata_cruzado", "hoja": "plata_cruzado", "madera": "plata_cruzado",
                     "guarnicion": "oro_sp", "gema": "gema_azul"}),
    ("del berserker", {"metal": "negro_berserker", "hoja": "negro_berserker", "mango": "cuero_rojo",
                       "guarnicion": "rojo_berserker"}),
    ("del gladiador", {"hoja": "acero", "mango": "cuero_rojo", "guarnicion": "oro_gladiador",
                       "cuero": "cuero_gladiador"}),
    ("del monje de lucha", {"hoja": "plata", "madera": "azul_monje", "guarnicion": "oro_viejo",
                            "tela": "blanco_monje"}),
    ("del cosechador de almas", {"hoja": "hierro_negro", "madera": "madera_oscura",
                                 "guarnicion": "violeta_cosechador", "tela": "negro_cosechador"}),
    ("del renegado", {"metal": "blanco_renegado", "hoja": "plata", "mango": "cuero_negro", "guarnicion": "oro_sp",
                      "gema": "gema_violeta", "cuero": "negro_renegado"}),
    ("del hachero de la cascada", {"hoja": "acero_tormenta", "madera": "madera_oscura", "guarnicion": "plata",
                                   "tela": "azul_cascada", "cuero": "correa_hachero"}),
    ("del caballero draconiano", {"metal": "blanco_dragon", "hoja": "oro_sp", "madera": "blanco_dragon",
                                  "guarnicion": "oro_sp", "gema": "gema_roja"}),
    ("del romperrocas", {"hoja": "roca_romperrocas", "madera": "madera_oscura", "guarnicion": "laton",
                         "gema": "llama"}),
    ("de aquiles", {"madera": "madera_oscura", "guarnicion": "bronce_aquiles", "gema": "gema_violeta",
                    "cuero": "cuero_oscuro"}),
    # SP-02, el Arquero (actores_trajes_arquero): las calzas y las botas del catalogo, y las armas
    ("del cazador", {"tela": "cuero_cazador", "cuero": "aljaba_cazador", "madera": "aljaba_cazador",
                     "guarnicion": "plata", "gema": "gema_azul"}),
    ("del asesino", {"tela": "negro_asesino", "cuero": "negro_asesino", "hoja": "hierro_negro",
                     "mango": "cuero_negro", "guarnicion": "rojo_asesino", "gema": "gema_roja"}),
    ("del destructor", {"tela": "cuero_destructor", "metal": "bomba_destructor", "cuero": "cuero_destructor",
                        "hoja": "hierro_negro", "madera": "madera_oscura", "guarnicion": "laton", "gema": "llama"}),
    ("del guardabosques", {"tela": "verde_guardabosques", "cuero": "cuero_guardabosques",
                           "madera": "asta_guardabosques", "mango": "cuero_guardabosques", "guarnicion": "oro_sp",
                           "gema": "sol_cazasoles"}),
    ("del canonero", {"tela": "cuero_canonero", "cuero": "correa_canonero", "hoja": "bronce",
                      "madera": "madera_oscura", "guarnicion": "hierro_negro", "gema": "llama"}),
    ("del espia", {"tela": "forro_espia", "cuero": "halcon_espia", "madera": "madera_oscura",
                   "guarnicion": "plata", "gema": "gema_azul"}),
    ("del cazademonios", {"tela": "abrigo_noche", "hoja": "hierro_negro", "madera": "madera_oscura",
                          "guarnicion": "plata", "gema": "gema_roja"}),
    ("del angel vengador", {"metal": "blanco_angel", "cuero": "blanco_angel", "hoja": "plata_cruzado",
                            "mango": "blanco_angel", "guarnicion": "oro_sp", "gema": "sol_cazasoles"}),
    ("del cazasoles", {"tela": "cuero_cazasoles", "cuero": "aljaba_cazador", "hoja": "oro_sp",
                       "madera": "asta_guardabosques", "guarnicion": "oro_sp"}),
    ("del granadero", {"metal": "acero_granadero", "hoja": "acero_granadero", "madera": "madera_oscura",
                       "guarnicion": "laton", "gema": "llama"}),
    ("del cazanieblas", {"tela": "gris_cazanieblas", "cuero": "negro_yi", "hoja": "hierro_negro",
                         "madera": "madera_oscura", "guarnicion": "plata", "gema": "gas_cazanieblas"}),
    ("del almirante yi", {"metal": "negro_yi", "tela": "negro_yi", "hoja": "acero_tormenta", "mango": "cuero_negro",
                          "guarnicion": "oro_sp", "gema": "gema_azul"}),
    # SP-02, el Mago (actores_trajes_mago): las calzas, los pantalones y el calzado del catalogo, y las armas
    ("del mago de fuego", {"tela": "tela_brasa", "cuero": "tela_brasa", "gema": "llama", "madera": "madera_oscura",
                           "guarnicion": "oro_fuego"}),
    ("del mago sagrado", {"tela": "blanco_sagrado", "cuero": "oro_sagrado", "metal": "blanco_sagrado",
                          "hoja": "blanco_sagrado", "madera": "blanco_sagrado", "guarnicion": "oro_sagrado",
                          "gema": "sol_cazasoles"}),
    ("del mago azul", {"tela": "azul_mago", "gema": "hielo_mago", "madera": "azul_mago", "guarnicion": "plata"}),
    ("del mago de las tinieblas", {"tela": "negro_tinieblas", "cuero": "negro_tinieblas", "metal": "negro_tinieblas",
                                   "hoja": "negro_tinieblas", "madera": "madera_oscura",
                                   "guarnicion": "plata_tinieblas", "gema": "violeta_tinieblas"}),
    ("de volcano", {"metal": "basalto_volcano", "gema": "lava_volcano", "madera": "basalto_volcano",
                    "guarnicion": "obsidiana_volcano"}),
    ("del senor de las mareas", {"tela": "verdemar_mareas", "cuero": "coral_mareas", "metal": "oro_sp",
                                 "hoja": "oro_sp", "madera": "verdemar_mareas", "guarnicion": "coral_mareas"}),
    ("del vidente", {"tela": "violeta_vidente", "gema": "magenta_vidente", "metal": "oro_sp",
                     "guarnicion": "oro_sp"}),
    ("del archimago", {"tela": "blanco_archimago", "cuero": "blanco_archimago", "guarnicion": "oro_sp",
                       "adorno": "oro_sp", "gema": "azul_archimago"}),
    ("del sacerdote vudu", {"cuero": "cuero_vudu", "gema": "hueso_cosechador", "madera": "madera_oscura",
                            "guarnicion": "pluma_roja_vudu"}),
    ("de gravitas", {"tela": "noche_gravitas", "cuero": "piedra_gravitas", "gema": "piedra_gravitas",
                     "metal": "noche_gravitas", "guarnicion": "brillo_gravitas"}),
    ("de la tormenta de fuego", {"tela": "negro_tormenta", "metal": "negro_tormenta", "hoja": "negro_tormenta",
                                 "madera": "negro_tormenta", "guarnicion": "laton_tormenta", "gema": "llama"}),
    ("de merlin", {"tela": "blanco_merlin", "cuero": "blanco_merlin", "metal": "blanco_merlin",
                   "hoja": "blanco_merlin", "madera": "celeste_merlin", "guarnicion": "oro_sp",
                   "gema": "celeste_merlin"}),
    # CP-01: los sets de las clases propias (actores_trajes_propias). Como los de las SP: la ropa propia va
    # dibujada con sus colores (paletas sin tipo) y el tema tine el arma y las piezas del catalogo que el
    # set reutiliza. Detras de «del druida flamigero» (el Druida del Roble es «del druida») y delante de
    # «alquimista» y de «faro» (casarian con el set del Alquimista y con el farol del Necromago); el
    # Juramento y el Alquimista, pieza a pieza («Tea del Juramento» y «Zuecos del Alquimista» son de otras
    # series).
    ("del comediante", {"hoja": "acero", "mango": "negro_comediante", "guarnicion": "oro_sp", "gema": "gema_roja",
                        "cuero": "negro_comediante"}),
    ("del juglar", {"madera": "nogal_juglar", "guarnicion": "oro_juglar", "gema": "gema_roja", "tela": "rojo_juglar",
                    "cuero": "cuero_juglar"}),
    ("del tamborilero", {"madera": "azul_tamborilero", "guarnicion": "rojo_tamborilero", "gema": "oro_sp",
                         "tela": "negro_tamborilero"}),
    ("del feriante", {"hoja": "oro_sp", "madera": "negro_feriante", "guarnicion": "oro_sp", "gema": "gema_roja",
                      "cuero": "negro_feriante"}),
    ("de la espiga dorada", {"hoja": "oro_espiga", "madera": "cuero_espiga", "guarnicion": "laton_juramento",
                             "gema": "gema_roja", "tela": "pardo_espiga", "cuero": "cuero_espiga"}),
    ("del caminante gris", {"hoja": "hierro", "madera": "madera_bordon", "guarnicion": "cuero_camino",
                            "tela": "gris_esclavina", "cuero": "cuero_camino"}),
    ("martillo del juramento", _TEMA_JURAMENTO),
    ("quijotes del juramento", _TEMA_JURAMENTO),
    ("escarpes del juramento", _TEMA_JURAMENTO),
    ("del rey lobo", {"hoja": "acero_escarcha", "mango": "cuero_lobo", "guarnicion": "plata_rey_lobo",
                      "gema": "cristal_azul", "metal": "hierro_lobo"}),
    ("del ingeniero", {"hoja": "acero_pavonado", "mango": "cuero_ingeniero", "guarnicion": "laton_ingeniero",
                       "gema": "gema_azul"}),
    ("del mercader", {"cuero": "granate_mercader", "cuerda": "oro_mercader", "guarnicion": "oro_mercader",
                      "gema": "gema_roja", "tela": "cuero_mercader"}),
    ("matraz del alquimista", {"gema": "pocion_dorada", "cuero": "laton_ingeniero"}),
    ("pantalon del alquimista", {"tela": "negro_alquimista"}),
    ("botas del alquimista", {"tela": "negro_alquimista"}),
    ("del cartomante", {"blason": "morado_cartomante", "guarnicion": "oro_sp", "gema": "gema_violeta",
                        "tela": "negro_cartomante", "cuero": "negro_cartomante"}),
    ("del druida", {"gema": "hoja_roble", "madera": "roble_druida", "guarnicion": "bellota",
                    "cuero": "corteza_druida"}),
    ("del domador", {"cuero": "cuero_domador", "mango": "piel_oso", "guarnicion": "colmillo", "tela": "cuero_domador"}),
    ("del chaman de los elementos", {"madera": "madera_totem", "tela": "rojo_ocre", "gema": "turquesa"}),
    ("del chaman de los espiritus", {"hoja": "acero_runico", "madera": "cuero_espiritu", "guarnicion": "turquesa",
                                     "gema": "turquesa", "tela": "cuero_espiritu", "malla": "malla_espiritu"}),
    ("del tempestario", {"hoja": "cobre_tempestario", "madera": "negro_tempestario", "guarnicion": "cobre_tempestario",
                         "gema": "rayo_tempestario", "tela": "azul_oscuro_tempestario"}),
    ("del necromago", {"madera": "madera_oscura", "guarnicion": "hierro_negro", "gema": "gema_verde",
                       "tela": "negro_necromago", "cuero": "negro_necromago"}),
    ("del corsario vampirico", {"hoja": "hoja_negra", "mango": "negro_corsario", "guarnicion": "plata_corsario",
                                "gema": "gema_roja", "tela": "negro_corsario"}),
    ("del forajido", {"hoja": "acero", "mango": "cuero_forajido", "guarnicion": "laton_forajido", "gema": "gema_azul",
                      "cuero": "cuero_forajido"}),
    ("ultima guardia", {"metal": "acero", "tela": "tela_gris_azul", "cuero": "cuero_pizarra", "adorno": "hierro"}),
    ("correo real", {"cuero": "cuero_rojo", "tela": "terciopelo_rojo", "adorno": "oro_viejo"}),
    ("tejadista", {"cuero": "cuero_polvo", "tela": "tela_gris_azul", "metal": "hierro_negro"}),
    ("rompefilas", {"metal": "hierro_negro", "adorno": "laton"}),
    ("vado sagrado", {"cuero": "cuero_leon", "metal": "plata", "tela": "tela_blanca"}),
    ("alquimista", {"madera": "madera_oscura", "cuero": "cuero_oscuro", "tela": "tela_verde_oscura",
                    "gema": "pocion_verde"}),
    ("vigilia", "sigilo"),
    ("miliciano", "milicia"),
    ("contrabandista", {"tela": "tela_verde_oscura", "cuero": "cuero_oscuro"}),
    ("abordaje", {"metal": "bronce_marino", "adorno": "laton", "tela": "tela_azul_marino"}),
    ("ramas secas", {"hojas": "corteza", "gema": "gema_verde"}),
    ("lectora", {"metal": "bronce", "gema": "gema_azul", "tela": "tela_azul_noche"}),
    ("duelista zurdo", {"madera": "terciopelo_rojo", "hoja": "acero", "metal": "acero", "guarnicion": "acero",
                        "mango": "cuero_negro"}),
    ("duelista", {"porcelana": "porcelana", "gema": "gema_roja", "tela": "terciopelo_rojo"}),
    ("daga simple", {}),
    ("daga gemela", {"hoja": "acero", "mango": "cuero_rojo", "guarnicion": "hierro"}),
    ("alba menor", {"adorno": "laton"}),
    ("simbolo sagrado", {"adorno": "bronce"}),
    ("del alba", {"metal": "acero", "adorno": "oro_viejo", "tela": "tela_blanca"}),
    ("ratero fino", {"tela": "tela_verde_oscura", "cuero": "cuero_negro"}),
    ("oficiante", {"tela": "tela_blanca", "tela2": "tela_amarilla", "adorno": "oro_viejo"}),
    ("runicas", {"metal": "acero_tormenta", "adorno": "cristal_azul", "tela": "tela_azul_noche"}),
    ("corredor de muros", {"cuero": "cuero_polvo", "tela": "tela_gris_azul", "metal": "hierro_negro"}),
    ("vadeador", {"cuero": "cuero_oscuro", "tela": "pantalon_oliva", "metal": "bronce_marino"}),
    ("baluarte", {"metal": "hierro", "adorno": "bronce"}),
    ("gremio", {"cuero": "cuero", "adorno": "laton", "tela": "tela_ocre"}),
    ("cazatormentas", {"tela": "tela_gris_azul", "tela2": "tela_amarilla", "cuero": "cuero_oscuro",
                       "metal": "acero_tormenta"}),
    ("escamas de rio", {"metal": "bronce_marino", "adorno": "laton"}),
    ("sepulturero", {"tela": "tela_negra", "malla": "hierro_negro", "metal": "hierro_negro",
                     "cuero": "cuero_negro"}),
    ("veterano", {"tela": "tela_marfil", "cuero": "cuero_polvo"}),
    ("caldera", {"metal": "hierro_negro", "adorno": "bronce"}),
    ("peregrino", {"tela": "tela_marfil", "adorno": "laton", "cuero": "cuero"}),
    ("archivista", {"tela": "tela_azul_marino", "tela2": "pergamino", "adorno": "laton"}),
    ("taberna", {"madera": "roble", "hoja": "laton", "guarnicion": "laton"}),
    ("faro", {"madera": "tela_blanca", "blason": "tela_roja_oscura", "guarnicion": "laton"}),
    ("cuarzo ahumado", {"gema": "cuarzo_ahumado", "metal": "bronce", "adorno": "bronce"}),
    ("grimorio de bolsillo", {"tela": "cuero_rojo", "adorno": "laton"}),
    ("asedio", {"madera": "madera_oscura", "blason": "tela_roja_oscura", "guarnicion": "hierro"}),
    ("mes propio", {"gema": "gema_verde", "adorno": "bronce"}),
    # armas con nombre (mientras no tengan dibujo propio)
    ("arco del censo", {"madera": "madera_oscura", "guarnicion": "laton"}),
    ("cetro de la audiencia", {"hoja": "oro_viejo", "madera": "madera_oscura", "guarnicion": "plata"}),
    ("colmillo de la primera nevada", {"hoja": "rey_lobo", "mango": "pelaje_lobo", "guarnicion": "plata"}),
    ("estoque del perdon", {"hoja": "plata", "mango": "tela_blanca", "guarnicion": "oro_viejo"}),
    ("hacha de la cosecha tardia", {"hoja": "hierro_viejo", "madera": "paja", "guarnicion": "bronce"}),
    ("lanza del vado", {"hoja": "bronce_marino", "madera": "fresno", "guarnicion": "plata"}),
    # el «Mazo del Desahucio» (TILDES-02 le corrigio la errata «Deshaucio»: el tema sigue al nombre)
    ("mazo del desahucio", {"hoja": "hierro_negro", "madera": "madera_oscura", "guarnicion": "hierro_viejo"}),
    ("replica del juicio", {"hoja": "acero", "mango": "tela_negra", "guarnicion": "plata"}),
    ("susurradora", {"madera": "madera_oscura", "cuerda": "tela_negra", "guarnicion": "hierro_negro"}),
    ("vara del interes compuesto", {"gema": "gema_verde", "madera": "madera_oscura", "guarnicion": "oro_viejo"}),
    ("arco de la lista corta", {"madera": "roble_antiguo", "guarnicion": "plata"}),
    ("baculo del coro hueco", {"gema": "luna", "madera": "madera_oscura", "guarnicion": "plata_negra"}),
    ("el argumento final", {"hoja": "acero", "madera": "madera_oscura", "guarnicion": "oro_viejo"}),
    ("espadon del puente roto", {"hoja": "ceniza", "mango": "cuero_negro", "guarnicion": "hierro_viejo"}),
    ("la impagable", {"hoja": "verde_serpiente", "mango": "cuero_negro", "guarnicion": "oro_viejo"}),
    ("martillo de fe de erratas", {"hoja": "plata", "madera": "fresno", "guarnicion": "oro_viejo"}),
    ("punal del doceavo mes", {"hoja": "acero", "mango": "tunica_verde", "guarnicion": "oro_viejo"}),
    ("tridente de la marea firme", {"hoja": "bronce_marino", "madera": "madera_oscura", "guarnicion": "plata"}),
    ("guadana del barbecho", {"hoja": "hierro_viejo", "madera": "roble", "guarnicion": "bronce"}),
]

TEMAS_POR_ETIQUETA = [("Oscuro", "oscuro"), ("Sagrado", "sagrado"), ("Conjuro", "conjuro"),
                      ("Arcano", "arcano"), ("Naturaleza", "naturaleza"), ("Sigilo", "sigilo")]

# --- la forma de las armas -------------------------------------------------------------------
# (palabras, pieza base). Gana la primera que casa con el nombre.
FORMAS_DE_ARMA = [
    # INSTRUMENTOS-01: los instrumentos y los objetos de rol (actores_formas_objetos), delante de todo («flauta de
    # pan» antes que «flauta»; «pala$» y «lira$» enteras)
    (("flauta de pan",), "f_flauta_pan"),
    (("flauta",), "f_flauta"),
    (("pandereta",), "f_pandereta"),
    (("lira$",), "f_lira"),
    (("trompeta",), "f_trompeta"),
    (("gaita",), "f_gaita"),
    (("vihuela",), "f_vihuela"),
    (("arpa de mano",), "f_arpa"),
    (("violin y arco",), "f_violin"),
    (("antorcha",), "f_tea"),
    (("cuerda de canamo",), "f_cuerda"),
    (("pala$",), "f_pala"),
    (("pico de minero",), "f_pico"),
    (("cana de pescar",), "f_cana"),
    (("catalejo",), "f_catalejo"),
    (("escoba",), "f_escoba"),
    (("almohada",), "f_almohada"),
    (("sarten",), "f_sarten"),
    (("jarra de cerveza",), "f_jarra_cerveza"),
    (("bolas de malabares",), "f_malabares"),
    (("shurikens",), "f_shurikens"),
    (("estandarte",), "f_estandarte"),
    # CP-01: las armas de las clases propias (actores_formas_propias). Delante de todas: «sable y
    # pistola» antes que «sable» y «pistola»; el baston del Feriante, el de paseo (antes que «baston»);
    # la hoz antes que la de «hacha de mano»; la vara del Tempestario y los faroles del
    # Necromago antes que «vara» y «farol»; el hacha del Chaman de los Espiritus es de una mano.
    # SETS-03: los guantes del Mago Chasqueador (actores_formas_iteracion3), antes de «guanteletes»
    (("guantes del chasqueador",), "f_guantes"),
    (("sable y pistola",), "f_sable_y_pistola"),
    (("laud",), "f_laud"),
    (("tambor",), "f_tambor"),
    (("baston de feriante", "baston del feriante"), "f_baston_paseo"),
    (("hoz",), "f_hoz"),
    (("llave",), "f_llave"),
    (("bolsa de monedas", "bolsa del mercader"), "f_bolsa"),
    (("matraz",), "f_matraz"),
    (("naipes",), "f_naipes"),
    (("totem",), "f_totem"),
    (("vara del tempestario",), "f_vara_rayo"),
    (("farol de animas", "farol del necromago"), "f_farol_vara"),
    (("hacha del chaman",), "f_hacha_mano"),
    # SP-02: los guanteletes del Artista Marcial, uno en cada mano (actores_formas): el canon de descargas del
    # Tronador, los brazos mecanicos del Puno Hidraulico y las garras (las del Maestro Lobo, el Guerrero
    # demoniaco y el Druida flamigero: «zarpas del» antes que la «zarpa» de los cuchillos)
    (("guanteletes del tronador",), "f_canon_de_mano"),
    (("brazos mecanicos",), "f_brazos_mecanicos"),
    (("garras", "zarpas del", "guanteletes del maestro lobo"), "f_garras"),
    (("guanteletes",), "f_guanteletes"),
    (("cuchillo",), "f_cuchillo"),
    (("estilete",), "f_estilete"),
    (("espada corta",), "f_espada_corta"),
    (("espadon", "mandoble"), "f_mandoble"),
    (("estoque",), "f_estoque"),
    (("sable", "katana"), "f_sable"),
    (("zarpa",), "f_cuchillo"),
    (("daga", "dagas", "punal", "colmillo", "alfiler", "aguja", "la impagable"), "f_daga"),
    (("espada", "hoja del", "replica del juicio", "pluma-espada"), "f_espada"),
    (("hacha de mano", "hoz"), "f_hacha_mano"),
    (("gran hacha",), "f_gran_hacha"),
    (("hacha",), "f_hacha"),
    (("macana",), "f_macana"),
    (("martillo", "mazo", "el argumento final", "yunque"), "f_martillo"),
    (("maza", "badajo", "campana"), "f_maza"),
    # SP-02: las armas de fuego alquimicas y el bumeran del Arquero («lanzagranadas» antes que «lanza»),
    # y la pistola magica y el lanzallamas del Mago
    (("lanzagranadas",), "f_lanzagranadas"),
    (("lanzallamas",), "f_lanzallamas"),
    (("pistola",), "f_pistola"),
    (("escopeta",), "f_escopeta"),
    (("rifle",), "f_rifle"),
    (("revolver",), "f_revolver"),
    (("canon",), "f_canon"),
    (("bumeran",), "f_bumeran"),
    (("pica",), "f_pica"),
    (("jabalina",), "f_jabalinas"),
    (("tridente",), "f_tridente"),
    (("guadana",), "f_guadana"),
    (("lanza", "astil"), "f_lanza"),
    (("ballesta", "susurradora"), "f_ballesta"),
    (("honda",), "f_honda"),
    (("latigo",), "f_latigo"),
    (("arco",), "f_arco"),
    (("baston ferrado", "bordon", "pala"), "f_baston_herrado"),
    (("baston", "baculo"), "f_baston"),
    (("vara del farol",), "f_farol"),
    (("vara",), "f_vara"),
    (("cetro",), "f_cetro"),
    (("rodela",), "f_rodela"),
    (("broquel",), "f_broquel"),
    (("paves",), "f_paves"),
    (("escudo",), "f_escudo"),
    (("guardabrazos", "brazalete de parada"), "f_brazal"),
    (("orbe", "foco", "fragmento de cielo", "ovillo", "esfera"), "f_orbe"),
    (("grimorio", "tomo"), "f_grimorio"),
    (("simbolo",), "f_simbolo"),
    (("talisman",), "f_talisman"),
    (("kit de herramientas",), "f_kit"),
    (("farol",), "f_farol"),
    (("tea",), "f_tea"),
    (("espejo",), "f_espejo"),
]

# Ropa: la pieza que elige actores_catalogo, salvo estas (nombre -> pieza)
PIEZA_DE_ROPA = {
    "botas claveteadas": ("pies", "botas_acero"),
    "botas de caminante": ("pies", "botas_claras"),
    "botas de cazador": ("pies", "botas_altas"),
    "coraza de escamas de rio": ("torso", "escamas_acero"),
    "piel de escamas legales": ("torso", "escamas_acero"),
    "pellejo del senor astado": ("torso", "cuero"),
    "malla de espuma": ("torso", "cota_malla"),
    "armadura media": ("torso", "escamas_acero"),
    "armadura ligera": ("torso", "gambeson"),
    # SP-02: el set de cada especialista (las SP de NosTale, tools/importar_clases_sp.py); sus
    # piezas propias se dibujan en actores_trajes
    "yelmo del guerrero": ("cabeza", "sp_yelmo_guerrero"),
    "coraza del guerrero": ("torso", "sp_coraza_guerrero"),
    "quijotes del guerrero": ("piernas", "quijotes"),
    "escarpes del guerrero": ("pies", "escarpes"),
    "cinta del ninja": ("cabeza", "sp_cinta_ninja"),
    "gi del ninja": ("torso", "sp_gi_ninja"),
    "hakama del ninja": ("piernas", "sp_hakama_ninja"),
    "tabi del ninja": ("pies", "sp_tabi_ninja"),
    "yelmo alado del cruzado": ("cabeza", "sp_yelmo_cruzado"),
    "coraza del cruzado": ("torso", "sp_coraza_cruzado"),
    "grebas del cruzado": ("piernas", "quijotes"),
    "escarpes del cruzado": ("pies", "escarpes"),
    "yelmo astado del berserker": ("cabeza", "sp_yelmo_berserker"),
    "coraza del berserker": ("torso", "sp_coraza_berserker"),
    "quijotes del berserker": ("piernas", "quijotes"),
    "escarpes del berserker": ("pies", "escarpes"),
    "galea del gladiador": ("cabeza", "sp_casco_gladiador"),
    "armadura del gladiador": ("torso", "sp_armadura_gladiador"),
    "grebas del gladiador": ("piernas", "sp_piernas_gladiador"),
    "sandalias del gladiador": ("pies", "sandalias"),
    "diadema del monje de lucha": ("cabeza", "sp_diadema_monje"),
    "habito del monje de lucha": ("torso", "sp_habito_monje"),
    "calzones del monje de lucha": ("piernas", "pantalon_azul"),
    "sandalias del monje de lucha": ("pies", "sandalias"),
    "capucha del cosechador de almas": ("cabeza", "sp_capucha_cosechador"),
    "manto del cosechador de almas": ("torso", "sp_manto_cosechador"),
    "calzas del cosechador de almas": ("piernas", "pantalon_negro"),
    "botas del cosechador de almas": ("pies", "sp_botas_cosechador"),
    "diadema del renegado": ("cabeza", "sp_diadema_renegado"),
    "armadura del renegado": ("torso", "sp_armadura_renegado"),
    "quijotes del renegado": ("piernas", "quijotes"),
    "botas del renegado": ("pies", "botas"),
    "casco del hachero de la cascada": ("cabeza", "sp_casco_hachero"),
    "coselete del hachero de la cascada": ("torso", "sp_coselete_hachero"),
    "calzas del hachero de la cascada": ("piernas", "grebas_cuero"),
    "botas del hachero de la cascada": ("pies", "sp_botas_hachero"),
    "yelmo del caballero draconiano": ("cabeza", "sp_yelmo_draconiano"),
    "armadura del caballero draconiano": ("torso", "sp_armadura_draconiana"),
    "quijotes del caballero draconiano": ("piernas", "quijotes"),
    "escarpes del caballero draconiano": ("pies", "escarpes"),
    "casco del romperrocas": ("cabeza", "sp_casco_romperrocas"),
    "peto del romperrocas": ("torso", "sp_peto_romperrocas"),
    "pantalones del romperrocas": ("piernas", "pantalon_pardo"),
    "botas del romperrocas": ("pies", "botas_acero"),
    "yelmo de aquiles": ("cabeza", "sp_yelmo_aquiles"),
    "coraza de aquiles": ("torso", "sp_coraza_aquiles"),
    "grebas de aquiles": ("piernas", "sp_piernas_aquiles"),
    "sandalias de aquiles": ("pies", "sandalias"),
    # SP-02, el Arquero (actores_trajes_arquero; el Cazademonios con las piezas del cazador de
    # demonios de SP-01)
    "capucha del cazador": ("cabeza", "sp_capucha_cazador"),
    "jubon del cazador": ("torso", "sp_jubon_cazador"),
    "calzas del cazador": ("piernas", "pantalon_pardo"),
    "botas del cazador": ("pies", "botas"),
    "capucha del asesino": ("cabeza", "sp_capucha_asesino"),
    "chaleco del asesino": ("torso", "sp_chaleco_asesino"),
    "calzas del asesino": ("piernas", "pantalon_negro"),
    "botas del asesino": ("pies", "botas"),
    "panuelo del destructor": ("cabeza", "sp_panuelo_destructor"),
    "chaqueta del destructor": ("torso", "sp_chaqueta_destructor"),
    "pantalones del destructor": ("piernas", "pantalon_pardo"),
    "botas del destructor": ("pies", "botas_acero"),
    "diadema del guardabosques": ("cabeza", "sp_diadema_guardabosques"),
    "tunica del guardabosques": ("torso", "sp_tunica_guardabosques"),
    "calzas del guardabosques": ("piernas", "calzas_verdes"),
    "botas del guardabosques": ("pies", "sp_botas_guardabosques"),
    "gorro de aviador del canonero": ("cabeza", "sp_gorro_canonero"),
    "chaqueta del canonero": ("torso", "sp_chaqueta_canonero"),
    "pantalones del canonero": ("piernas", "pantalon_pardo"),
    "botas del canonero": ("pies", "botas_altas"),
    "gorro del espia": ("cabeza", "sp_gorro_espia"),
    "gabardina del espia": ("torso", "sp_gabardina_espia"),
    "calzas del espia": ("piernas", "pantalon_gris"),
    "botas del espia": ("pies", "botas_claras"),
    "sombrero del cazademonios": ("cabeza", "sp_sombrero_cazador"),
    "abrigo del cazademonios": ("torso", "sp_abrigo_cazador"),
    "pantalones del cazademonios": ("piernas", "pantalon_negro"),
    "botas del cazademonios": ("pies", "sp_botas_cazador"),
    "diadema del angel vengador": ("cabeza", "sp_diadema_angel"),
    "armadura del angel vengador": ("torso", "sp_armadura_angel"),
    "grebas del angel vengador": ("piernas", "quijotes"),
    "botas del angel vengador": ("pies", "escarpes"),
    "tocado del cazasoles": ("cabeza", "sp_tocado_cazasoles"),
    "peto del cazasoles": ("torso", "sp_peto_cazasoles"),
    "calzas del cazasoles": ("piernas", "pantalon_pardo"),
    "botas del cazasoles": ("pies", "botas_claras"),
    "casco del granadero": ("cabeza", "sp_casco_granadero"),
    "peto del granadero": ("torso", "sp_peto_granadero"),
    "pantalones del granadero": ("piernas", "pantalon_oliva"),
    "botas del granadero": ("pies", "botas_acero"),
    "capucha del cazanieblas": ("cabeza", "sp_capucha_cazanieblas"),
    "gabardina del cazanieblas": ("torso", "sp_gabardina_cazanieblas"),
    "pantalones del cazanieblas": ("piernas", "pantalon_negro"),
    "botas del cazanieblas": ("pies", "botas"),
    "sombrero del almirante yi": ("cabeza", "sp_sombrero_yi"),
    "armadura del almirante yi": ("torso", "sp_armadura_yi"),
    "grebas del almirante yi": ("piernas", "quijotes"),
    "botas del almirante yi": ("pies", "botas_altas"),
    # SP-02, el Mago (actores_trajes_mago; el Mago de fuego lleva la corona y la tunica de SP-01)
    "corona del mago de fuego": ("cabeza", "sp_corona_fuego"),
    "tunica del mago de fuego": ("torso", "sp_tunica_fuego"),
    "calzas del mago de fuego": ("piernas", "calzas_rojas"),
    "zapatos del mago de fuego": ("pies", "zapatos"),
    "mitra del mago sagrado": ("cabeza", "sp_mitra_sagrado"),
    "casulla del mago sagrado": ("torso", "sp_casulla_sagrado"),
    "calzas del mago sagrado": ("piernas", "calzas_rojas"),
    "sandalias del mago sagrado": ("pies", "sandalias"),
    "capucha del mago azul": ("cabeza", "sp_capucha_mago_azul"),
    "tunica del mago azul": ("torso", "sp_tunica_mago_azul"),
    "calzas del mago azul": ("piernas", "calzas_verdes"),
    "botas del mago azul": ("pies", "sp_botas_mago_azul"),
    "sombrero de copa del mago de las tinieblas": ("cabeza", "sp_sombrero_copa"),
    "levita del mago de las tinieblas": ("torso", "sp_levita_tinieblas"),
    "pantalones del mago de las tinieblas": ("piernas", "pantalon_negro"),
    "botas del mago de las tinieblas": ("pies", "botas"),
    "yelmo de volcano": ("cabeza", "sp_yelmo_volcano"),
    "coraza de volcano": ("torso", "sp_coraza_volcano"),
    "grebas de volcano": ("piernas", "quijotes"),
    "botas de volcano": ("pies", "botas_acero"),
    "corona del senor de las mareas": ("cabeza", "sp_corona_mareas"),
    "coraza del senor de las mareas": ("torso", "sp_coraza_mareas"),
    "calzas del senor de las mareas": ("piernas", "calzas_verdes"),
    "sandalias del senor de las mareas": ("pies", "sandalias"),
    "turbante del vidente": ("cabeza", "sp_turbante_vidente"),
    "tunica del vidente": ("torso", "sp_tunica_vidente"),
    "calzas del vidente": ("piernas", "calzas_rojas"),
    "babuchas del vidente": ("pies", "pantuflas"),
    "sombrero del archimago": ("cabeza", "sp_sombrero_archimago"),
    "tunica del archimago": ("torso", "sp_tunica_archimago"),
    "calzas del archimago": ("piernas", "calzas_verdes"),
    "zapatos del archimago": ("pies", "zapatos"),
    "tocado del sacerdote vudu": ("cabeza", "sp_tocado_vudu"),
    "peto del sacerdote vudu": ("torso", "sp_peto_vudu"),
    "faldellin del sacerdote vudu": ("piernas", "sp_faldellin_vudu"),
    "sandalias del sacerdote vudu": ("pies", "sandalias"),
    "diadema de gravitas": ("cabeza", "sp_diadema_gravitas"),
    "tunica de gravitas": ("torso", "sp_tunica_gravitas"),
    "calzas de gravitas": ("piernas", "pantalon_azul"),
    "botas de gravitas": ("pies", "botas"),
    "casco de la tormenta de fuego": ("cabeza", "sp_casco_tormenta"),
    "chaqueta de la tormenta de fuego": ("torso", "sp_chaqueta_tormenta"),
    "pantalones de la tormenta de fuego": ("piernas", "pantalon_negro"),
    "botas de la tormenta de fuego": ("pies", "botas_altas"),
    "corona de flores de merlin": ("cabeza", "sp_corona_flores"),
    "manto de merlin": ("torso", "sp_manto_merlin"),
    "calzas de merlin": ("piernas", "calzas_verdes"),
    "botas de merlin": ("pies", "botas_claras"),
    # SP-02, el Artista Marcial (actores_trajes_artista)
    "cinta del puno del dragon": ("cabeza", "sp_cinta_dragon"),
    "chaqueta del puno del dragon": ("torso", "sp_chaqueta_dragon"),
    "pantalones del puno del dragon": ("piernas", "sp_pantalon_kungfu"),
    "zapatillas del puno del dragon": ("pies", "sp_zapatillas_kungfu"),
    "diadema de las artes misticas": ("cabeza", "sp_diadema_mistica"),
    "tunica de las artes misticas": ("torso", "sp_tunica_mistica"),
    "pantalones de las artes misticas": ("piernas", "sp_pantalon_mistico"),
    "sandalias de las artes misticas": ("pies", "sandalias"),
    "capucha de lobo del maestro lobo": ("cabeza", "sp_capucha_lobo"),
    "chaleco del maestro lobo": ("torso", "sp_chaleco_lobo"),
    "pantalones del maestro lobo": ("piernas", "pantalon_gris"),
    "botas del maestro lobo": ("pies", "sp_vendas_lobo"),
    "cuernos del guerrero demoniaco": ("cabeza", "sp_cuernos_demonio"),
    "coraza del guerrero demoniaco": ("torso", "sp_coraza_demonio"),
    "grebas del guerrero demoniaco": ("piernas", "grebas_cuero"),
    "botas del guerrero demoniaco": ("pies", "sp_botas_demonio"),
    "mascara de oso del druida flamigero": ("cabeza", "sp_mascara_oso"),
    "pellejo del druida flamigero": ("torso", "sp_pellejo_druida"),
    "calzas del druida flamigero": ("piernas", "pantalon_pardo"),
    "botas de piel del druida flamigero": ("pies", "sp_botas_druida"),
    "gafas del puno hidraulico": ("cabeza", "sp_gafas_hidraulico"),
    "peto del puno hidraulico": ("torso", "sp_peto_hidraulico"),
    "pantalones del puno hidraulico": ("piernas", "grebas_cuero"),
    "botas del puno hidraulico": ("pies", "sp_botas_mecanico"),
    "casco del tronador": ("cabeza", "sp_casco_tronador"),
    "chaqueta del tronador": ("torso", "sp_chaqueta_tronador"),
    "pantalones del tronador": ("piernas", "pantalon_negro"),
    "botas del tronador": ("pies", "botas"),
    "yelmo alado de thor": ("cabeza", "sp_yelmo_thor"),
    "coraza de thor": ("torso", "sp_coraza_thor"),
    "grebas de thor": ("piernas", "quijotes"),
    "botas de thor": ("pies", "botas_acero"),
    # CP-01: el set de cada clase propia (tools/cp_*.py); sus piezas propias se dibujan en
    # actores_trajes_propias
    "jubon del comediante": ("torso", "cp_jubon_comediante"),
    "sombrero del comediante": ("cabeza", "cp_sombrero_comediante"),
    "calzas del comediante": ("piernas", "cp_calzas_comediante"),
    "zapatos del comediante": ("pies", "zapatos"),
    "sobrevesta del juglar": ("torso", "cp_sobrevesta_juglar"),
    "birrete del juglar": ("cabeza", "cp_birrete_juglar"),
    "calzas del juglar": ("piernas", "calzas_rojas"),
    "botas del juglar": ("pies", "botas"),
    "casaca del tamborilero": ("torso", "cp_casaca_tamborilero"),
    "chaco del tamborilero": ("cabeza", "cp_chaco_tamborilero"),
    "pantalon del tamborilero": ("piernas", "cp_pantalon_tamborilero"),
    "botas del tamborilero": ("pies", "botas_altas"),
    "frac del feriante": ("torso", "cp_frac_feriante"),
    "chistera del feriante": ("cabeza", "cp_chistera_feriante"),
    "pantalon del feriante": ("piernas", "cp_pantalon_feriante"),
    "botas del feriante": ("pies", "botas"),
    "tunica de la espiga dorada": ("torso", "cp_tunica_espiga"),
    "sombrero de la espiga dorada": ("cabeza", "cp_sombrero_espiga"),
    "pantalon de la espiga dorada": ("piernas", "pantalon_pardo"),
    "sandalias de la espiga dorada": ("pies", "sandalias"),
    "esclavina del caminante gris": ("torso", "cp_esclavina_caminante"),
    "sombrero del caminante gris": ("cabeza", "cp_sombrero_caminante"),
    "pantalon del caminante gris": ("piernas", "pantalon_gris"),
    "botas del caminante gris": ("pies", "botas"),
    "coraza del juramento": ("torso", "cp_coraza_juramento"),
    "yelmo del juramento": ("cabeza", "cp_yelmo_juramento"),
    "quijotes del juramento": ("piernas", "quijotes"),
    "escarpes del juramento": ("pies", "escarpes"),
    "coraza del rey lobo": ("torso", "cp_coraza_rey_lobo"),
    "yelmo del rey lobo": ("cabeza", "cp_yelmo_rey_lobo"),
    "quijotes del rey lobo": ("piernas", "quijotes"),
    "botas del rey lobo": ("pies", "cp_botas_rey_lobo"),
    "mandil del ingeniero": ("torso", "cp_mandil_ingeniero"),
    "casco del ingeniero": ("cabeza", "cp_casco_ingeniero"),
    "pantalon del ingeniero": ("piernas", "cp_pantalon_ingeniero"),
    "botas del ingeniero": ("pies", "cp_botas_ingeniero"),
    "casaca del mercader": ("torso", "cp_casaca_mercader"),
    "bonete del mercader": ("cabeza", "cp_bonete_mercader"),
    "bombachos del mercader": ("piernas", "cp_bombachos_mercader"),
    "botas del mercader": ("pies", "botas_altas"),
    "gaban del alquimista": ("torso", "cp_gaban_alquimista"),
    "sombrero del alquimista": ("cabeza", "cp_sombrero_alquimista"),
    "pantalon del alquimista": ("piernas", "pantalon_negro"),
    "botas del alquimista": ("pies", "botas_altas"),
    "levita del cartomante": ("torso", "cp_levita_cartomante"),
    "diadema del cartomante": ("cabeza", "cp_diadema_cartomante"),
    "pantalon del cartomante": ("piernas", "pantalon_negro"),
    "zapatos del cartomante": ("pies", "zapatos"),
    "tunica del druida": ("torso", "cp_habito_druida"),
    "corona del druida": ("cabeza", "cp_corona_druida"),
    "pantalon del druida": ("piernas", "cp_pantalon_druida"),
    "sandalias del druida": ("pies", "sandalias"),
    "chaleco del domador": ("torso", "cp_chaleco_domador"),
    "cabeza de oso del domador": ("cabeza", "cp_cabeza_oso_domador"),
    "pantalon del domador": ("piernas", "pantalon_pardo"),
    "botas del domador": ("pies", "cp_botas_domador"),
    "chaleco del chaman de los elementos": ("torso", "cp_chaleco_chaman"),
    "tocado del chaman de los elementos": ("cabeza", "cp_cuernos_chaman"),
    "pantalon del chaman de los elementos": ("piernas", "cp_pantalon_chaman"),
    "botas del chaman de los elementos": ("pies", "cp_botas_chaman"),
    "cota del chaman de los espiritus": ("torso", "cp_cota_espiritus"),
    "banda del chaman de los espiritus": ("cabeza", "cp_banda_espiritus"),
    "faldar del chaman de los espiritus": ("piernas", "grebas_malla"),
    "botas del chaman de los espiritus": ("pies", "cp_botas_espiritus"),
    "tunica del tempestario": ("torso", "cp_tunica_tempestario"),
    "sombrero del tempestario": ("cabeza", "cp_sombrero_tempestario"),
    "pantalon del tempestario": ("piernas", "pantalon_azul"),
    "botas del tempestario": ("pies", "cp_botas_tempestario"),
    "tunica del necromago": ("torso", "cp_tunica_necromago"),
    "capucha del necromago": ("cabeza", "cp_capucha_necromago"),
    "pantalon del necromago": ("piernas", "pantalon_negro"),
    "botas del necromago": ("pies", "botas"),
    "casaca del corsario vampirico": ("torso", "cp_casaca_corsario"),
    "tricornio del corsario vampirico": ("cabeza", "cp_tricornio_corsario"),
    "calzones del corsario vampirico": ("piernas", "pantalon_negro"),
    "botas del corsario vampirico": ("pies", "cp_botas_corsario"),
    "chaleco del forajido": ("torso", "cp_chaleco_forajido"),
    "panuelo del forajido": ("cabeza", "cp_panuelo_forajido"),
    "pantalon del forajido": ("piernas", "cp_pantalon_forajido"),
    "botas del forajido": ("pies", "botas"),
    # SETS-03: el set de las seis clases de la 3.a iteracion (tools/sets_iteracion3.py); sus piezas propias se
    # dibujan en actores_trajes_iteracion3
    "coraza del apostata": ("torso", "it3_coraza_apostata"),
    "capucha del apostata": ("cabeza", "it3_capucha_apostata"),
    "grebas del apostata": ("piernas", "quijotes"),
    "botas del apostata": ("pies", "botas_altas"),
    # SETS-04: los tres sets del Senor de las Armas
    "casaca del duque": ("torso", "it3_casaca_duque"),
    "diadema del duque": ("cabeza", "it3_diadema_duque"),
    "calzas del duque": ("piernas", "it3_calzas_duque"),
    "botas del duque": ("pies", "it3_botas_duque"),
    "placas del soberano sin rostro": ("torso", "it3_placas_soberano"),
    "yelmo del soberano sin rostro": ("cabeza", "it3_yelmo_soberano"),
    "grebas del soberano sin rostro": ("piernas", "it3_grebas_soberano"),
    "escarpes del soberano sin rostro": ("pies", "it3_escarpes_soberano"),
    "armadura del rey de la forja": ("torso", "it3_armadura_forja"),
    "corona del rey de la forja": ("cabeza", "it3_corona_forja"),
    "grebas del rey de la forja": ("piernas", "it3_grebas_forja"),
    "botas del rey de la forja": ("pies", "it3_botas_forja"),
    "coraza de la furia": ("torso", "it3_coraza_furia"),
    "yelmo de la furia": ("cabeza", "it3_yelmo_furia"),
    "grebas de la furia": ("piernas", "quijotes"),
    "botas de la furia": ("pies", "it3_botas_furia"),
    "poncho de las cuatro estaciones": ("torso", "it3_poncho_estaciones"),
    "corona de las cuatro estaciones": ("cabeza", "it3_corona_estaciones"),
    "pantalones de las cuatro estaciones": ("piernas", "it3_pantalones_estaciones"),
    "botas de las cuatro estaciones": ("pies", "botas"),
    "chaqueta del chasqueador": ("torso", "it3_chaqueta_chasqueador"),
    "bombin del chasqueador": ("cabeza", "it3_bombin_chasqueador"),
    "pantalon del chasqueador": ("piernas", "it3_pantalon_chasqueador"),
    "botas del chasqueador": ("pies", "botas_altas"),
    "habito del despertar": ("torso", "it3_habito_despertar"),
    "diadema del despertar": ("cabeza", "it3_diadema_despertar"),
    "pantalones del despertar": ("piernas", "it3_pantalones_despertar"),
    "sandalias del despertar": ("pies", "sandalias"),
}

# Abalorios: lo que no se ve puesto (o no del todo) brilla (actores_estetica.PARTICULAS)
ABALORIOS = ("anillo", "amuleto", "talisman", "simbolo", "argolla", "caracola", "cuerno", "escudilla",
             "haz de espigas", "mojon", "sello", "lentes", "monoculo", "tercer ojo")


def _base_del_nombre(nombre):
    n = normalizar(nombre)
    return re.sub(r"\s+(insigne|sublime)$", "", n).strip(), (
        "insigne" if n.endswith(" insigne") else "sublime" if n.endswith(" sublime") else None)


def _tema(nombre_base, objeto):
    """El tema de un objeto: su deidad, su nombre o sus etiquetas (o ninguno)."""
    deidad = objeto.get("deity")
    if deidad in DEIDADES:
        return DEIDADES[deidad], "deidad"
    for clave, tema in TEMAS_POR_NOMBRE:
        if _casa(clave, nombre_base):
            return tema, "nombre"
    etiquetas = set(objeto.get("grantedTags") or [])
    for etiqueta, tema in TEMAS_POR_ETIQUETA:
        if etiqueta in etiquetas:
            return tema, "etiqueta"
    return None, None


def _resuelve(tema):
    if tema is None:
        return {}
    return dict(TEMAS[tema]) if isinstance(tema, str) else dict(tema)


def forma_de_arma(nombre):
    n = normalizar(nombre)
    for palabras, pieza in FORMAS_DE_ARMA:
        if any(_casa(p, n) for p in palabras):
            return pieza
    return None


def _es_unica(o):
    """Las armas con nombre y las miticas de las deidades."""
    return ("arma con nombre" in (o.get("description") or "")
            or (o.get("rarity") == "mythic" and o.get("deity") and normalizar(o.get("slot")) == "mainhand"))


# --- el tinte ---------------------------------------------------------------------------------

def _destino_arma(canal, material, tema):
    """Material del objeto para un canal de una forma de arma (1 principal, 2 mango, 3 guarnicion)."""
    tipo = TIPO.get(material, material)
    if canal == 1:
        claves = {"metal": ("hoja", "metal"), "madera": ("madera",), "gema": ("gema",),
                  "tela": ("tela",), "cuero": ("cuero",), "llama": ("llama",)}.get(tipo, (tipo,))
    elif canal == 2:
        claves = {"cuero": ("mango", "cuero"), "madera": ("madera",), "tela": ("blason", "tela"),
                  "cuerda": ("cuerda",), "adorno": ("guarnicion", "adorno")}.get(tipo, (tipo,))
    elif canal == 3:
        claves = ("guarnicion", "adorno") if tipo in ("adorno", "metal") else (tipo,)
    else:
        claves = ("gema",)
    for c in claves:
        if c in tema:
            return tema[c]
    return None


def tinte_de_arma(pieza, tema):
    """[(origen, destino)] para una pieza base de actores_formas."""
    familia, m1, m2, _nombre, *resto = actores_formas.BASES[pieza]
    canales = [(1, m1), (2, m2)] + [(3 + i, m) for i, m in enumerate(resto)]
    tinte = []
    for canal, material in canales:
        destino = _destino_arma(canal, material, tema)
        if destino and destino != material:
            tinte.append((material, destino))
    return tinte


def tinte_de_ropa(materiales, tema):
    """[(origen, destino)] para una prenda: sus materiales (por pixeles, de mas a menos)."""
    tinte, telas = [], 0
    for material in materiales:
        tipo = TIPO.get(material)
        if tipo is None:
            continue
        clave = tipo
        if tipo == "tela":
            clave = "tela" if telas == 0 else "tela2"
            telas += 1
        destino = tema.get(clave)
        if destino and destino != material:
            tinte.append((material, destino))
    return tinte


METALES_LLANOS = ("hierro", "acero", "laton", "bronce", "ceniza")
TIPOS_CON_ACABADO = ("metal", "malla", "adorno", "cuero", "tela", "madera", "porcelana", "paja", "hojas",
                     "cuerno")


def con_acabado(tema, acabado):
    """El tema con el acabado encima: los remates y la guarnicion de plata (Insigne) o
    de oro (Sublime); el resto lo pone aplicar_acabado."""
    if acabado not in ("insigne", "sublime"):
        return tema
    t = dict(tema)
    noble = "plata" if acabado == "insigne" else "oro_viejo"
    t.update({"adorno": noble, "guarnicion": noble, "_acabado": acabado})
    return t


def destino_con_acabado(origen, destino, acabado):
    """El material final de `origen` (que el tema pinta de `destino`) con el acabado."""
    import actores_materiales as mats
    tipo = TIPO.get(origen) or TIPO.get(destino)
    if tipo not in TIPOS_CON_ACABADO or destino in ("plata", "oro_viejo", "oro_emblema"):
        return destino
    if tipo in ("metal", "malla") and destino in METALES_LLANOS + ("malla",):
        if tipo == "malla" or destino == "malla":
            return "malla_plata" if acabado == "insigne" else "malla_oro"
        return "plata" if acabado == "insigne" else "oro_viejo"
    metal = tipo in ("metal", "malla") or destino in ("bronce_marino", "acero_tormenta", "hierro_negro",
                                                      "hierro_forja", "hierro_viejo", "plata_negra", "rey_lobo",
                                                      "vidrio", "malla_espuma", "malla_plata", "malla_oro")
    return mats.acabado(destino, acabado, metal=metal)


def aplicar_acabado(tinte, materiales, acabado):
    """El tinte de una pieza con el acabado: cada material de la pieza (tenido o no)."""
    destinos = dict(tinte)
    out = []
    for material in materiales:
        destino = destinos.get(material, material)
        final = destino_con_acabado(material, destino, acabado)
        if final != material:
            out.append((material, final))
    return out


# --- el catalogo --------------------------------------------------------------------------------

def estetica(o, materiales_de_pieza, paletas):
    """{ranura, pieza, tinte, estetica} de un objeto de equipment.json, o None si no se ve.

    materiales_de_pieza(ranura, pieza) -> [materiales de la fila, de mas a menos pixeles]."""
    nombre = o.get("name") or o.get("id")
    base, acabado_nombre = _base_del_nombre(nombre)
    slot = normalizar(o.get("slot") or "")
    rareza = o.get("rarity")
    est = {}
    # PART-02: los anillos, el collar y el cinturon no se ven puestos: son abalorios (brillo_sin_pieza)
    if slot in ("ring", "ring2", "neck", "belt", "accessory"):
        return None

    # 1. la forma
    if slot == "mainhand":
        propia = f"f_u_{o.get('id')}"
        if propia in actores_formas.BASES:  # las unicas con dibujo propio (actores_formas.UNICAS)
            pieza = propia
        else:
            pieza = forma_de_arma(nombre)
            if pieza is None:
                return None
            paso = PASO_POR_RAREZA.get(rareza)
            escalon = f"f_{actores_formas.BASES[pieza][0]}_{paso}" if paso else None
            if escalon in actores_formas.BASES:
                pieza = escalon
                est["escalon"] = paso
        ranura = "mano_izq" if actores_formas.BASES[pieza][0] in _mano_izquierda() else "arma"
    else:
        res = PIEZA_DE_ROPA.get(base) or pieza_de_objeto(o)
        if res is None:
            return None
        ranura, pieza = res
    est["forma"] = pieza

    # 2. la esencia (o el tema)
    tema = {}
    m = re.match(r"^(cuchillo de cazador|daga|estilete|sable ligero|hacha de mano) de (.+)$", base)
    if slot == "mainhand" and m and m.group(2) in ESENCIAS:
        hoja, tema_esencia = ESENCIAS[m.group(2)]
        tema = _resuelve(tema_esencia)
        tema["hoja"] = hoja
        if m.group(2) in GEMA_DE_ESENCIA:
            tema["gema"] = GEMA_DE_ESENCIA[m.group(2)]
        est["esencia"] = m.group(2)
        guarnicion = GUARNICION_POR_RAREZA.get(rareza)
        if guarnicion:
            tema["guarnicion"] = guarnicion
        est["acabado"] = rareza
    elif base.endswith(" de milicia") or base.endswith(" de viaje"):
        tema = _resuelve("milicia" if base.endswith(" de milicia") else "viaje")
        est["acabado"] = "milicia" if base.endswith(" de milicia") else "viaje"
    else:
        clave, de_donde = _tema(base, o)
        tema = _resuelve(clave)
        if clave is not None:
            est["deidad" if de_donde == "deidad" else "tema"] = clave if isinstance(clave, str) else base
        if acabado_nombre:
            est["acabado"] = acabado_nombre
            tema = con_acabado(tema, acabado_nombre)
    if _es_unica(o):
        est["unica"] = True
        if pieza.startswith("f_u_"):  # su dibujo ya lleva sus materiales: sin tinte
            est["dibujo"] = "propio"
            tema = {}
        elif rareza == "mythic":  # mientras no tenga dibujo propio: el metal mitico
            tema = dict(tema, guarnicion="oro_rojo", metal="oro_rojo")

    # 3. el tinte
    if pieza in actores_formas.BASES:
        _f, *canales = actores_formas.BASES[pieza]
        materiales = [canales[0], canales[1]] + list(canales[3:])
        tinte = tinte_de_arma(pieza, tema)
    else:
        materiales = materiales_de_pieza(ranura, pieza)
        tinte = tinte_de_ropa(materiales, tema)
    if tema.get("_acabado"):
        tinte = aplicar_acabado(tinte, materiales, tema["_acabado"])
    import actores_materiales as mats
    faltan = [d for _o, d in tinte if d not in paletas and d not in mats.MATERIALES]
    if faltan:
        raise SystemExit(f"{nombre}: el tinte pide paletas que no hay: {faltan}")
    return {"nombre": nombre, "slot": o.get("slot"), "ranura": ranura, "pieza": pieza,
            "tinte": [list(p) for p in tinte], "estetica": est,
            "brillo": brillo(o, est, tinte, materiales[0] if materiales else None)}


# --- el brillo por fuerza (estilo NosTale) -------------------------------------------------
# Oriol (06/10): «nada de implementar el sistema de mejoras de NosTale [...] podemos igualar
# los brillos en relacion a lo fuerte que sea el arma: de +1 a +2 Tier 1, +3 a +4 Tier 2 y
# asi consecutivamente». El nivel va de 1 a 10: el primero del par de su tier para la pieza
# de entrada, el segundo para la fuerte (la que pide mas nivel en su tier, las reliquias y
# las unicas). Cuanto brilla cada nivel lo dice el motor (Game::escalonDeBrillo); aqui va de
# que color y con que particulas, por su esencia, su deidad o su tema.
# PART-01: las familias de particulas son las de tools/particulas.py (las de los elementos y las de los
# temas de las clases)
PARTICULAS = tuple(_particulas.FAMILIAS)
BRILLO_DE_ESENCIA = {
    "ceniza": ("aaaab4", "ceniza"), "cristal azul": ("8cc8ff", "chispas"), "oro viejo": ("ffd26e", "chispas"),
    "plata": ("ebf0ff", "chispas"), "roble antiguo": ("96be64", "hojas"), "hierro santo": ("ffebaa", "luz"),
    "luna": ("d2beff", "estrellas"), "niebla": ("d2e1eb", "ceniza"), "penumbra": ("5a4682", "sombra"),
    "plaga": ("aae650", "veneno"), "trueno": ("96c8ff", "rayos"), "tumba": ("c8d2b4", "ceniza"),
    "estrella": ("c8d2ff", "estrellas"), "juramento": ("ffe696", "luz"), "rey lobo": ("c8ebff", "escarcha"),
    "sangre": ("e62828", "sangre"), "sombra roja": ("781419", "sombra"), "corona rota": ("e6c864", "chispas"),
    "eternidad": ("aafff0", "luz"), "vacio": ("3c1e5a", "vacio"),
}
BRILLO_DE_TEMA = {
    "llama_eterna": ("ff963c", "brasas"), "padre_forja": ("ff8c3c", "brasas"), "cuervo": ("3c3c6e", "ceniza"),
    "trueno": ("96c8ff", "rayos"), "farol_rojo": ("ff5a46", "brasas"), "caminante": ("bebebe", "ceniza"),
    "oraculo": ("aaf0ff", "rayos"), "leon": ("ffdc8c", "luz"), "mareas": ("78dcdc", "agua"),
    "sepulturero": ("b4c8b4", "ceniza"), "tejedora": ("bec8ff", "estrellas"), "luna_hueca": ("d7c8ff", "estrellas"),
    "reina_velada": ("783ca0", "sombra"), "toro": ("f0a050", "chispas"), "rey_lobo": ("c8ebff", "escarcha"),
    "mendigo": ("e6d2aa", "luz"), "madre_doce": ("a0e678", "hojas"), "corte_salvaje": ("6eaa3c", "hojas"),
    "serpiente": ("50dc78", "veneno"), "espiga": ("fadc6e", "hojas"),
    "sagrado": ("ffe696", "luz"), "arcano": ("8caaff", "estrellas"), "conjuro": ("be82ff", "estrellas"),
    # abalorios sin tema: por lo que dan
    "amuleto de foco": ("b4c8ff", "estrellas"), "anillo de vitalidad": ("ff6e78", "luz"),
    "naturaleza": ("82c85a", "hojas"), "sigilo": ("46465a", "sombra"), "oscuro": ("5a3278", "sombra"),
    "milicia": ("c8d7ff", "chispas"), "viaje": ("ebe1c8", "chispas"),
}


def _brillo_de_los_sets():
    """PART-01: el set de cada SP y de cada clase propia brilla con las particulas de su clase
    (tools/particulas.py: el Puño del dragon, dragones; las Artes Misticas, el loto...). El color, el de
    sus particulas o el de su elemento; si no tienen, el de lo principal de la pieza (None)."""
    import cp_bardos
    import cp_fe
    import cp_ingenio
    import cp_naturaleza
    import cp_noche_y_tormenta
    import elementos
    import sp_arquero
    import sp_artista_marcial
    import sp_espadachin
    import sp_mago
    import sets_iteracion3
    from sp_reglas import etiqueta_set
    out = {}
    for c in (sp_espadachin.ESPADACHIN + sp_arquero.ARQUERO + sp_mago.MAGO + sp_artista_marcial.ARTISTA_MARCIAL +
              cp_bardos.BARDOS + cp_fe.FE + cp_ingenio.INGENIO + cp_naturaleza.NATURALEZA +
              cp_noche_y_tormenta.NOCHE_Y_TORMENTA + sets_iteracion3.SETS):
        familia = _particulas.CLASES[c["id"]][0][0]
        colores = _particulas.FAMILIAS[familia]["colores"]
        color = colores[0] if colores else (elementos.ELEMENTOS[c["elemento"]][1] if c.get("elemento") else None)
        # la clave del tema: «del guerrero», «de las artes misticas», «de thor»...
        out[normalizar(etiqueta_set(c.get("set_de", c["name"]))[len("Set "):])] = (color, familia)
        for p in c["set"]:  # los temas pieza a pieza (el Juramento, el Alquimista)
            out.setdefault(normalizar(p["name"]), (color, familia))
    return out


NIVEL_DE_ENTRADA = {1: 1, 2: 20, 3: 40, 4: 60, 5: 80}  # el de niveles_equipo para la pieza floja


def nivel_de_brillo(o):
    """1..10: Tier 1 = +1/+2, Tier 2 = +3/+4 ... Tier 5 = +9/+10."""
    import niveles_equipo
    tier = max(1, min(5, int(o.get("tier") or 1)))
    pide = niveles_equipo.nivel_por_regla(o, set())  # sin la excepcion del equipo inicial
    fuerte = pide > NIVEL_DE_ENTRADA[tier] or bool(o.get("artifact")) or _es_unica(o)
    return 2 * tier - 1 + (1 if fuerte else 0)


_BRILLO_SETS = {}


def _brillo_sets():
    if not _BRILLO_SETS:
        _BRILLO_SETS.update(_brillo_de_los_sets())
    return _BRILLO_SETS


def brillo(o, est, tinte, principal):
    """{nivel, color, particulas} del objeto. principal: el material que manda en su aspecto."""
    import actores_materiales as mats
    clave = None
    if est.get("esencia"):
        color, particulas = BRILLO_DE_ESENCIA[est["esencia"]]
        clave = "esencia"
    elif est.get("deidad") in BRILLO_DE_TEMA:
        color, particulas = BRILLO_DE_TEMA[est["deidad"]]
    elif isinstance(est.get("tema"), str) and est["tema"] in BRILLO_DE_TEMA:
        color, particulas = BRILLO_DE_TEMA[est["tema"]]
    elif isinstance(est.get("tema"), str) and est["tema"] in _brillo_sets():
        color, particulas = _brillo_sets()[est["tema"]]
    elif est.get("acabado") in BRILLO_DE_TEMA:
        color, particulas = BRILLO_DE_TEMA[est["acabado"]]
    else:
        color, particulas = None, "chispas"
    if color is None:  # el color de la luz de lo principal, ya tenido
        destino = dict(tinte).get(principal, principal)
        color = "%02x%02x%02x" % mats.MATERIALES[destino].tono["H"] if destino in mats.MATERIALES else "ebf0ff"
    del clave
    return {"nivel": nivel_de_brillo(o), "color": color, "particulas": particulas}


def brillo_sin_pieza(o):
    """El brillo de lo que no se ve puesto (anillos, amuletos, reliquias pequenas): Oriol,
    «yo pondria que dieran particulas al jugador, dependiendo de lo que den o su lore un
    brillo acorde [...] si su lore es de algo malo pues particulas oscuras». Salen de su
    sitio (PART-02b: el motor lo saca de la ranura del catalogo, "slot", y de donde va
    puesto: el anillo de la mano que lo lleva, el collar del pecho y el cinturon de la
    cintura, con las anclas de actores_anclas). Un abalorio de dos efectos lleva ademas
    "otrasParticulas" y "otroColor". PART-02c: los abalorios del Tier 2 traen los suyos
    (tools/abalorios_t2.py: el anillo de su elemento, el collar o el cinturon de su tema)."""
    import abalorios_t2
    propio = abalorios_t2.BRILLO.get(o.get("id"))
    if propio is not None:
        return dict({"nivel": nivel_de_brillo(o)}, **propio, abalorio=True)
    nombre = o.get("name") or o.get("id")
    base, _acabado = _base_del_nombre(nombre)
    est = {}
    if base in BRILLO_DE_TEMA:
        est["tema"] = base
    else:
        clave, de_donde = _tema(base, o)
        if isinstance(clave, str):
            est["deidad" if de_donde == "deidad" else "tema"] = clave
    b = brillo(o, est, [], None)
    b["abalorio"] = True
    return b


def _mano_izquierda():
    import actores_armas
    return actores_armas.MANO_IZQUIERDA
