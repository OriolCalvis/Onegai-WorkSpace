"""PART-01 — Las particulas del juego: de que estan hechas las cartas de cada clase y como se ven
(planificacion/CANON_PARTICULAS.md).

Oriol (07/10/2026): «trabajar particulas que reflejen todos los elementos creados [...] no sera una
particula para cada clase, mas bien que todas tengan particulas y los efectos visuales necesarios: no
deberia tener las mismas particulas en los puños un Artes Misticas que un Puño del dragon; incluso en
los fuegos, separar el Druida flamigero, mas puro fuego, del Puño del dragon, sprites de dragon. Hay
que analizar que habilidades aporta cada clase y hacer una lista de las particulas requeridas para
todas». Esta es la lista:

- FAMILIAS: cada familia de particulas (las de los trece elementos de elementos.py y las de los temas
  de las clases: notas, naipes, monedas, dragones...), con su forma, su movimiento y sus colores. El
  motor las pinta (Game::tipoDeParticula y Game::pixelesDeForma, src/Game/Personajes/FamiliasDeParticulas.cpp);
  test_validar_sprites comprueba que son las mismas.
- CLASES: las particulas de cada una de las 127 clases (la primera, la suya: la de su set y sus
  abalorios; las otras, las que echan algunas de sus cartas), sacadas de lo que hacen sus cartas
  (tools/elementos.py) y de lo que es la clase.
- de_la_carta / de_cada_carta: que familia echa cada carta (lo que nombra su texto, TEMAS, de entre
  las de su clase; la variante de su clase para su elemento; la de su elemento; la de su clase).
  test_validar_sprites comprueba que cada carta echa una de las de su clase y que cada una se usa.

Movimientos: «sube», «cae», «deriva» (de lado), «converge» (hacia el centro de la pieza), «orbita»
(gira alrededor de ella), «expande» (un aro que crece), «chispazo» (un instante, como el rayo),
«salpica» (sale disparada arriba y a un lado y cae, como las chispas).
vy: pixeles por segundo (negativo, sube); vx: cuanto se mece o deriva; vida: segundos; densidad: cuantas,
respecto a lo normal; otra_forma y con_otra_forma: la forma de algunas y que parte; gira: alterna con su
otra cara (el engranaje girado, la moneda de canto).
"""

# nombre -> {de, forma, movimiento, vy, vx, vida, suma, colores (vacio = el color de la pieza), que}
FAMILIAS = {}


def _f(nombre, de, forma, movimiento, vy, vx, vida, suma, colores, que, densidad=1.0, otra_forma=None,
       con_otra_forma=0.0, gira=False):
    FAMILIAS[nombre] = {"de": de, "forma": forma, "movimiento": movimiento, "vy": vy, "vx": vx, "vida": vida,
                        "suma": suma, "colores": colores, "que": que, "densidad": densidad,
                        "otra_forma": otra_forma, "con_otra_forma": con_otra_forma, "gira": gira}


# --- las de los elementos (tools/elementos.py) -------------------------------------------------------------
_f("llamas", "Fuego", "llama", "sube", -16.0, 3.0, 0.9, True, ["fff2a8", "ffb040", "ff6a2a"],
   "lenguas de fuego que suben y se apagan, del amarillo al naranja")
_f("agua", "Agua", "gota", "cae", 6.0, 1.0, 1.0, True, [], "gotas que caen")
_f("escarcha", "Hielo", "copo", "cae", 5.0, 3.0, 1.5, True, ["ffffff", "bff0ff", "8ad8ff"],
   "copos y cristales de escarcha que caen meciéndose")
_f("rayos", "Rayo", "rayo", "chispazo", 0.0, 0.0, 0.25, True, [],
   "un rayo en zigzag que salta un instante")
_f("piedras", "Tierra", "piedra", "cae", 14.0, 2.0, 0.8, False, ["c89a5a", "8a6a40", "e6c690"],
   "esquirlas de piedra y tierra que saltan y caen")
_f("viento", "Viento", "estela", "deriva", -2.0, 14.0, 1.1, True, ["f4fff8", "d8f5e6"],
   "estelas de aire que cruzan de lado")
_f("luz", "Luz", "punto", "sube", -10.0, 1.0, 1.3, True, [], "motas de luz que suben")
_f("sombra", "Tinieblas", "voluta", "sube", -7.0, 3.0, 1.5, False, [], "volutas de sombra que suben y tapan")
_f("hojas", "Naturaleza", "hoja", "cae", 8.0, 6.0, 1.6, False, [], "hojas que caen meciéndose")
# Oriol: las burbujas verdes «parece todo lo contrario, curaciones... ¿calaveras verdes?»
_f("veneno", "Veneno", "calavera", "sube", -6.0, 3.0, 1.6, False, ["b4f064", "8cd23c", "64aa28"],
   "calaveras verdes que suben meciéndose", densidad=0.6)
_f("sangre", "Sangre", "gota", "cae", 9.0, 0.5, 0.9, False, ["d22828", "8a1018"], "gotas de sangre que caen")
_f("arcano", "Arcano", "runa", "orbita", -2.0, 5.0, 1.6, True, ["e6b4ff", "b48cff", "ffb4e6"],
   "runas que giran alrededor y se desvanecen")
_f("vacio", "Vacio", "cruz", "converge", 0.0, 0.0, 1.2, False, ["3c1e5a", "5a3c8c", "8a5ac8"],
   "motas oscuras que caen hacia dentro, como si la pieza se las tragara")

# --- las de los temas de las clases ------------------------------------------------------------------------
_f("brasas", "Fuego", "punto", "sube", -14.0, 4.0, 1.1, True, [], "brasas de fragua que suben")
_f("ceniza", "Fuego", "punto", "sube", -5.0, 3.0, 1.6, False, [], "ceniza que flota")
# Oriol: «tampoco se entiende que sean estrellas, haz un trazo como el típico, muy simple»
_f("chispas", None, "trazo_ne", "salpica", -24.0, 10.0, 0.8, True, [],
   "chispas: trazos que saltan hacia arriba y a los lados y caen, como en la fragua")
_f("estrellas", "Arcano", "estrella", "sube", -3.0, 1.0, 1.0, True, [], "estrellas de cuatro puntas que titilan")
_f("llamas_puras", "Fuego", "llama", "sube", -18.0, 2.0, 0.8, True, ["ffffff", "fff7c8", "ffe070"],
   "fuego puro: llamas blancas y amarillas, sin humo (el Druida flamígero)")
_f("dragones", "Fuego", "dragon", "sube", -12.0, 6.0, 1.4, True, ["ffd25a", "ff7a3a", "e8483a"],
   "dragones de fuego diminutos que suben serpenteando (el Puño del dragón)")
_f("petalos", "Naturaleza", "petalo", "cae", 5.0, 7.0, 1.8, True, ["ffd2e6", "ffffff", "ffa0c8"],
   "pétalos que caen girando (el loto de las Artes Místicas, las flores de Merlín)")
_f("furia", None, "estela_vertical", "sube", -22.0, 1.0, 0.6, True, ["ff5a3a", "ff9a3a", "c8281e"],
   "el aura de la furia: llamaradas rojas que suben por el cuerpo")
# Oriol: «algo más intenso»
_f("oro", "Luz", "estrella", "sube", -4.0, 1.0, 1.2, True, ["fff6c8", "ffd23c", "f5a51e"],
   "destellos de oro, el doble y más vivos (el Señor de las Armas)", densidad=2.0)
_f("notas", None, "nota", "sube", -9.0, 4.0, 1.6, True, [], "notas musicales que suben bailando")
_f("ondas", None, "punto", "expande", 0.0, 0.0, 0.9, True, [], "ondas de sonido que se abren (el tambor, el eco)")
_f("naipes", None, "naipe", "deriva", -4.0, 8.0, 1.4, False, ["ffffff", "c83232", "28283c"],
   "naipes que revolotean")
# Oriol: «cruces doradas brillando junto a las monedas»
_f("monedas", None, "moneda", "cae", 12.0, 2.0, 1.0, True, ["ffe68a", "e6b432"],
   "monedas que caen girando, con cruces doradas brillando entre ellas", otra_forma="estrella", con_otra_forma=0.4,
   gira=True)
# Oriol: «que roten encima de sí mismos también»
_f("engranajes", None, "engranaje", "cae", 7.0, 2.0, 1.2, False, ["c8c8c8", "b48c3c"],
   "engranajes que caen girando sobre sí mismos", gira=True)
_f("humo", None, "voluta", "sube", -5.0, 4.0, 1.8, False, ["e6e6e6", "a0a0a8"], "humo, vapor y niebla")
_f("polvora", "Fuego", "punto", "sube", -10.0, 6.0, 0.6, True, ["ffe68a", "ff9a3a"],
   "chispas de sal de fuego y pólvora")
_f("alquimia", None, "burbuja", "sube", -7.0, 3.0, 1.4, True, ["78f0a0", "ff7ab4", "7ab4ff", "ffe66e"],
   "burbujas de colores (el Alquimista)")
_f("plumas", None, "pluma", "cae", 4.0, 8.0, 2.0, False, ["ffffff", "e6e0d2"], "plumas que caen meciéndose")
_f("murcielagos", "Tinieblas", "murcielago", "deriva", -8.0, 12.0, 1.2, False, ["2a1e32", "4a2a46"],
   "murciélagos diminutos que salen volando")
_f("animas", "Tinieblas", "voluta", "sube", -6.0, 5.0, 2.0, True, ["c8ffc8", "8ae6a0"],
   "las ánimas: fuegos fatuos verdes y pálidos que suben")
_f("espiritus", None, "voluta", "orbita", -3.0, 6.0, 1.8, True, ["b4e6ff", "78c8ff"],
   "espíritus azules que giran alrededor (lobos, osos, antepasados)")
_f("esporas", "Naturaleza", "punto", "deriva", -3.0, 5.0, 2.2, True, ["e6f0a0", "c8dc78"],
   "esporas que flotan despacio")
_f("ki", None, "estela_vertical", "sube", -24.0, 0.5, 0.5, True, ["ffffff", "c8f0ff"],
   "el ki: trazos de luz que suben deprisa")
_f("grano", "Luz", "punto", "cae", 10.0, 3.0, 1.0, True, ["fff0a0", "e6c85a"], "granos de trigo dorado")
_f("confeti", None, "naipe", "cae", 8.0, 9.0, 1.6, False, ["ff5a5a", "5ab4ff", "ffe65a", "78e678"],
   "confeti de colores (el teatro, la feria)")
_f("polvo", "Tierra", "voluta", "deriva", -2.0, 9.0, 1.2, False, ["c8aa82", "a08664"],
   "polvo y arena que levantan los pasos")
_f("elementos", None, "cruz", "orbita", -2.0, 6.0, 1.6, True, ["ff8c3c", "5ab4ff", "c89a5a", "d8f5e6"],
   "los cuatro elementos que giran alrededor (el Chamán de los Elementos)")

# Como se escriben para leerlas (los nombres de arriba son los del motor, sin tildes)
_CON_TILDE = {"vacio": "vacío", "llamas_puras": "llamas puras", "petalos": "pétalos", "polvora": "pólvora",
              "murcielagos": "murciélagos", "animas": "ánimas", "espiritus": "espíritus"}


def nombre(familia):
    """El nombre de una familia para leerlo: «pétalos», «llamas puras»."""
    return _CON_TILDE.get(familia, familia)


FORMAS = ("punto", "cruz", "burbuja", "voluta", "hoja", "rayo", "gota", "llama", "copo", "piedra", "estela",
          "estela_vertical", "runa", "nota", "naipe", "moneda", "engranaje", "pluma", "murcielago", "dragon",
          "petalo", "calavera", "estrella", "trazo_ne", "trazo_no", "trazo_se", "trazo_so", "engranaje_girado",
          "moneda_de_canto")
MOVIMIENTOS = ("sube", "cae", "deriva", "converge", "orbita", "expande", "chispazo", "salpica")

# id de la clase -> (particulas: la primera es la suya, de que sale)
CLASES = {
    # --- las 63 de antes -----------------------------------------------------------------------------------
    "acechador_de_tejados": (["sombra"], "sigilo urbano: se esconde en la sombra"),
    "ancla_del_vacio": (["vacio"], "sus cartas son del Vacío"),
    "astronomo_errante": (["estrellas"], "presagios de las estrellas"),
    "bardo_de_la_cuerda_rota": (["notas", "sombra"], "cánticos corruptos: notas en la sombra"),
    "bardo_de_la_voz_divina": (["notas", "luz"], "cánticos sagrados"),
    "bardo_de_las_cuatro_estaciones": (["notas", "hojas", "escarcha"], "canciones de estación"),
    "caballero_templario": (["luz"], "sus cartas son sagradas"),
    "cambiaformas": (["espiritus"], "magia de transformación animal"),
    "cantor_de_la_tierra": (["piedras", "notas"], "canta muros, temblores y piedra"),
    "carcelero_de_hierro": (["chispas"], "cadenas de hierro"),
    "cazarrecompensas_de_bastrea": (["chispas"], "redes y herramientas de captura"),
    "centinela_del_alba": (["luz"], "el alba"),
    "class_arcanist": (["arcano"], "control y utilidad arcana"),
    "class_dawn_voice": (["luz"], "curación divina"),
    "class_guardian_iron": (["chispas"], "el escudo de hierro"),
    "class_road_shadow": (["sombra"], "sigilo"),
    "class_wandering_blade": (["viento"], "el filo sin magia"),
    "cocinero_de_campana": (["humo"], "el vapor de la cocina"),
    "coloso_de_ceniza": (["brasas", "ceniza"], "un incendio sagrado que no se apaga"),
    "cronista_de_batallas": (["plumas"], "la pluma del cronista"),
    "danzarin_del_filo": (["viento"], "baila y corta"),
    "devoto_del_bosque": (["hojas"], "paladín de la naturaleza"),
    "el_apostata": (["sangre"], "sacrifica su sangre por el poder oscuro (Oriol: sangre roja; también sus cartas de Tinieblas)"),
    "forjacantos": (["chispas", "notas"], "la fragua y los cantos"),
    "francotirador": (["polvora"], "cañones arcanotecnológicos"),
    "furia_de_los_creadores": (["furia", "oro"], "la furia bárbara con la devoción: furia dorada"),
    "furia_salvaje": (["furia", "hojas"], "el bárbaro chamánico: furia y aura (Oriol)"),
    "guardaespaldas": (["chispas"], "el escudo del escolta"),
    "guardian_de_la_presa": (["agua"], "embalsa el agua y la suelta en descargas"),
    "guerrero_del_puno_cortante": (["furia"], "furia y golpes seguidos"),
    "hermano_de_sangre": (["sangre", "furia"], "furia divina de sangre"),
    "hoja_del_monzon": (["agua"], "esgrimista del agua"),
    "hoja_venenosa": (["veneno", "hojas"], "venenos y trampas naturales"),
    "inventor": (["engranajes"], "artilugios de magia y tecnología"),
    "jinete_de_tormentas": (["rayos"], "corcel de tormenta"),
    "ladron_de_rostros": (["sombra"], "yokai que roba rostros"),
    "lancero_del_vendaval": (["viento"], "el asta del Aire"),
    "maestro_de_ceremonias": (["luz"], "burocracia divina y ceremonia"),
    "mago_del_paso_ligero": (["sombra"], "sigilo mágico"),
    "medium_de_la_tomba": (["animas"], "portavoz de los muertos"),
    "muralla_de_udrax": (["piedras"], "acumula Piedra"),
    "nacido_del_trueno": (["rayos"], "el poder del trueno"),
    "peregrino_de_los_doce_meses": (["luz"], "las bendiciones de los meses"),
    "portador_de_plagas": (["veneno"], "insectos y plagas"),
    "portador_del_farol": (["luz", "brasas"], "su farol"),
    "portaestandarte": (["luz"], "el aura de la bandera"),
    "quebraguardia": (["llamas"], "sus técnicas de fuego"),
    "quemador_de_reliquias": (["llamas"], "piromante: quema reliquias"),
    "rastreador_de_ecos": (["ondas"], "caza por el sonido"),
    "redimido": (["luz"], "brujo que volvió a la luz"),
    "renacido": (["sombra", "furia"], "pacto oscuro y furia"),
    "rompemareas": (["agua"], "los puertos del sur"),
    "saltador_de_planos": (["vacio"], "atraviesa los planos"),
    "sanadora_de_esporas": (["esporas"], "hongos simbiontes"),
    "senor_de_las_armas": (["oro"], "armas místicas y reliquias: dorado (Oriol)"),
    "senor_de_los_astros": (["estrellas"], "el sol, la luna y las estrellas"),
    "sombra_de_la_duna": (["polvo"], "el desierto: arena y emboscada"),
    "tejedora_de_mareas": (["agua"], "mueve las mareas"),
    "titiritero_maldito": (["sombra"], "hilos oscuros"),
    "usurero_de_almas": (["animas", "monedas"], "presta con almas"),
    "verdugo_del_silencio": (["sombra"], "el miedo del condenado"),
    # SETS-03 (08/10): con sus cartas escritas, las de lo que nombran (la rafaga de los elementos, la herida que
    # sangra, el eco del puno; el fuego, la sombra, el rayo y las espinas del Chasqueador)
    "monje_del_despertar": (["ki", "elementos", "sangre", "ondas"], "acumula ki"),
    "mago_chasqueador": (["arcano", "llamas", "sombra", "rayos", "hojas"], "magia de reflejos y suerte"),
    # --- las 44 SP (el elemento de sus cartas, tools/elementos.py) ------------------------------------------
    "guerrero": (["llamas", "piedras"], "golpes de fuego y el terremoto"),
    "ninja": (["agua"], "cuchillas de agua"),
    "cruzado": (["luz"], "flechas y cruces santas"),
    "berserker": (["furia"], "la furia del berserker (también sus cartas de Tinieblas)"),
    "gladiador": (["llamas"], "golpes de fuego"),
    "monje_de_lucha": (["agua"], "la lanza del agua"),
    "cosechador_de_almas": (["animas", "sombra", "sangre"], "cosecha almas"),
    "renegado": (["luz"], "la luz que se corrompe"),
    "hachero_de_la_cascada": (["agua"], "la cascada"),
    "caballero_draconiano": (["luz"], "la luz del dragonio"),
    "romperrocas": (["piedras", "llamas"], "roca, minería y cohetes"),
    "aquiles": (["humo"], "la niebla del Estigia (también sus cartas de Tinieblas)"),
    "cazador": (["agua", "rayos", "escarcha"], "flechas de agua, relámpagos y helada"),
    "asesino": (["sombra", "sangre"], "la sombra y la cuchillada sangrante"),
    "destructor": (["polvora", "llamas", "veneno"], "bombas y gas tóxico"),
    "guardabosques": (["espiritus", "hojas"], "los espíritus del bosque"),
    "servidor_del_canon_de_fuego": (["polvora", "llamas"], "el cañón de fuego"),
    "espia": (["viento", "plumas"], "el halcón"),
    "cazademonios": (["sombra", "animas"], "los espíritus de la venganza"),
    "angel_vengador": (["luz", "rayos"], "alas de ángel y espadas de luz"),
    "cazasoles": (["luz", "espiritus"], "el lobo solar"),
    "granadero": (["polvora", "llamas"], "granadas"),
    "cazanieblas": (["humo", "veneno"], "gas y niebla"),
    "almirante_yi": (["agua"], "el dragón de los mares"),
    "mago_de_fuego": (["llamas", "arcano"], "fuego"),
    "mago_sagrado": (["luz", "arcano"], "luz santa"),
    "mago_azul": (["escarcha"], "hielo"),
    "mago_de_las_tinieblas": (["sombra", "animas"], "tinieblas y fantasmas"),
    "volcano": (["llamas", "ceniza"], "lava y ceniza"),
    "senor_de_las_mareas": (["agua", "rayos"], "el mar y la tormenta"),
    "vidente": (["estrellas", "sombra"], "el destino y la maldición"),
    "archimago": (["luz"], "luz sagrada y libros de encantamientos"),
    "sacerdote_vudu": (["plumas", "veneno", "sombra"], "plumas, serpientes y almas"),
    "gravitas": (["vacio"], "la gravedad: lo atrae todo hacia sí"),
    "tormenta_de_fuego": (["llamas"], "fuego"),
    "merlin": (["luz", "petalos"], "luz y flores"),
    "puno_del_dragon": (["dragones"], "sprites de dragón (Oriol)"),
    "artes_misticas": (["petalos"], "el loto y la luna (no las mismas que el Puño del dragón)"),
    "maestro_lobo": (["espiritus", "ki"], "el espíritu del lobo"),
    "guerrero_demoniaco": (["murcielagos", "sombra", "sangre"], "murciélagos y tinieblas"),
    "druida_flamigero": (["llamas_puras"], "fuego más puro (Oriol)"),
    "puno_hidraulico": (["humo", "engranajes"], "vapor y mecánica"),
    "tronador": (["rayos"], "electricidad"),
    "thor": (["rayos", "escarcha"], "el trueno y el hielo"),
    # --- las 20 propias -------------------------------------------------------------------------------------
    "bardo_de_las_dos_mascaras": (["confeti"], "el teatro"),
    "bardo_del_cantar_de_gesta": (["notas", "luz"], "el laúd y la gesta"),
    "bardo_del_tambor_de_guerra": (["ondas"], "el tambor"),
    "bardo_de_la_feria": (["confeti", "llamas"], "la feria y el tragafuegos"),
    "clerigo_de_la_espiga_dorada": (["grano", "luz"], "la cosecha"),
    "clerigo_del_caminante_gris": (["humo", "luz"], "la niebla de los caminos"),
    "paladin_del_juramento": (["chispas", "brasas"], "la fragua"),
    "caballero_del_rey_lobo": (["escarcha", "espiritus"], "el invierno y la manada"),
    "ingeniero_de_asedio": (["engranajes", "polvora"], "torretas y minas"),
    "mercader_errante": (["monedas"], "el oro"),
    "alquimista": (["alquimia", "llamas", "veneno", "escarcha"], "frascos de colores"),
    "cartomante": (["naipes", "sombra"], "las cartas del tarot"),
    "druida_del_roble": (["hojas"], "el roble"),
    "domador_de_bestias": (["polvo"], "la estampida"),
    "chaman_de_los_elementos": (["elementos", "llamas", "agua", "piedras", "viento", "escarcha"], "los cuatro tótems"),
    "chaman_de_los_espiritus": (["rayos", "espiritus", "llamas", "viento", "sangre"], "la tormenta y los espíritus"),
    "tempestario": (["rayos"], "el rayo"),
    "necromago_de_las_animas": (["animas"], "las almas en pena"),
    "corsario_vampirico": (["murcielagos", "sangre", "viento"], "murciélagos y sangre"),
    "forajido_de_vinclap": (["polvora"], "la pistola de sal de fuego"),
}


# Lo que dice el texto de una carta (su nombre y su descripcion, sin tildes y en minusculas) de cada familia: una
# carta echa la de su clase que su texto nombre (la bomba de humo del Ninja, humo; el halcon de la Espia, plumas).
TEMAS = {
    "llamas": r"fuego|llama|arde|ardien|quema|incendi|abrasa|lava|hoguera",
    "agua": r"\bagua|marea|\bolas?\b|lluvia|torrente|cascada|oleaje|\bmar\b",
    "escarcha": r"hielo|escarcha|helad|congel|nieve|invierno|glaci|ventisca|granizo",
    "rayos": r"\brayos?\b|relampag|trueno|tormenta|electric|centella|estatic|tronad",
    "piedras": r"piedra|\brocas?\b|terremoto|seism|sism|temblor|grieta|derrumbe|canteria",
    "viento": r"viento|vendaval|rafaga|torbellino|tornado|huracan|galerna|\baire\b",
    "luz": r"\bluz\b|sagrad|\bsant[oa]s?\b|divin|bendic|\balba\b|\bsol\b|solar|resplandor",
    "sombra": r"sombra|oscur|tiniebla|maldic|\bnoche\b|umbr",
    "hojas": r"\bhojas?\b|\braic|espina|\bramas?\b|bosque|enredadera|zarza|hiedra|roble|savia",
    "veneno": r"venen|toxic|acido|\bgas\b|plaga|ponzon",
    "sangre": r"sangr|mordisco|vampir",
    "arcano": r"arcan|\brunas?\b|\bmana\b",
    "vacio": r"vacio|planos?\b|dimension|graveda|atraccion",
    "brasas": r"fragua|yunque|\bforja|brasa|ascua",
    "ceniza": r"ceniza",
    "estrellas": r"estrella|astro|cometa|constelac|\bluna\b|firmamento|presagio",
    "dragones": r"dragon",
    "petalos": r"\bloto\b|\bflor|petalo",
    "furia": r"furia|\bira\b|rabia|frenesi|enfurec|berserk",
    "oro": r"\boro\b|dorad|arsenal|reliquia",
    "notas": r"cancion|\bcanto|cantar|\bnotas?\b|melod|himno|balada|\bcoro\b|laud|musica|verso",
    "ondas": r"tambor|redoble|\becos?\b|grito|rugido|\bondas?\b|marcha",
    "naipes": r"naipe|\bcartas?\b|tarot|baraja|fortuna",
    "monedas": r"moneda|\boro\b|\bprecio|\bpago|deuda|prestamo|cobr|mercader|regate",
    "engranajes": r"engranaje|torreta|mecanic|artilugio|piston|trampa|maquina|llave inglesa|tuerca",
    "humo": r"\bhumo|niebla|bruma|vapor",
    "polvora": r"polvora|bomba|granada|explosi|canon|pistola|\bdisparo|\bminas?\b|sal de fuego",
    "alquimia": r"frasco|pocion|elixir|matraz|alquim|reactivo|brebaje",
    "plumas": r"pluma|halcon|\balas?\b|vuelo|cuervo|\baves?\b",
    "murcielagos": r"murcielago",
    "animas": r"\balmas?\b|anima|fantasma|espectr|muert|tumba|difunt",
    "espiritus": r"espiritu|\blob[oa]s?\b|\boso\b|antepasad|totem|bestia|manada|aullido",
    "esporas": r"espora|hongo|\bseta|moho",
    "ki": r"\bki\b|\bchi\b|medita|concentra",
    "grano": r"\bgrano|trigo|cosecha|espiga|siega",
    "confeti": r"teatro|funcion|aplauso|feria|mascara|publico|escenario|telon",
    "polvo": r"\bpolvo|\barena|estampida|\bduna",
    "elementos": r"elemental|los elementos",
}


def _sin_tildes(texto):
    import unicodedata
    return unicodedata.normalize("NFKD", texto or "").encode("ascii", "ignore").decode().lower()


def de_la_carta(carta, clase, elemento_de_la_clase=None):
    """La familia de particulas de una carta (planificacion/CANON_PARTICULAS.md §3). `carta`: la de skills.json
    (name, description, mechanicTags); `elemento_de_la_clase`: el de elemento_de_la_clase(). En este orden:
    1. la de su clase que nombre su texto (TEMAS), si no es de otro elemento que la carta; antes las otras que la
       primera (la cancion del invierno del bardo de las estaciones, escarcha; el halcon de la Espia, plumas);
    2. si es de un elemento y su clase tiene una familia de ese elemento, esa (el fuego del Druida flamigero son
       llamas puras; el del Puño del dragon, dragones; el del Paladin del Juramento, brasas de fragua);
    3. si es del elemento de su clase, la de su clase (las cartas de Agua de las Artes Misticas, que en NosTale
       son de Agua, echan los petalos del loto; las de Tinieblas del Apostata, sangre);
    4. si no, la de su elemento; y sin elemento, la de su clase."""
    import re
    import elementos
    suyas = CLASES.get(clase, (["chispas"], ""))[0]
    e = next((elementos.de_la_etiqueta(t) for t in carta.get("mechanicTags") or [] if elementos.de_la_etiqueta(t)),
             None)
    texto = _sin_tildes(f"{carta.get('name', '')} {carta.get('description', '')}")
    for f in suyas[1:] + suyas[:1]:
        if f in TEMAS and FAMILIAS[f]["de"] in (None, e) and re.search(TEMAS[f], texto):
            return f
    if e is None:
        return suyas[0]
    variante = next((f for f in suyas if FAMILIAS[f]["de"] == e), None)
    if variante:
        return variante
    if e == elementos.de_la_etiqueta(elemento_de_la_clase or ""):
        return suyas[0]
    return elementos.ELEMENTOS[e][2]


def elemento_de_la_clase(clase, cartas):
    """El elemento de una clase: el suyo (las SP y las propias lo traen) o, si no trae, el de la mayoria de sus
    cartas con elemento (el Apostata, de Tinieblas), o None."""
    import elementos
    from collections import Counter
    datos = clase.get("especialista") or clase.get("propia") or {}
    if datos.get("elemento"):
        return elementos.de_la_etiqueta(datos["elemento"])
    cuenta = Counter(e for c in cartas for e in [next((elementos.de_la_etiqueta(t) for t in c.get("mechanicTags") or []
                                                        if elementos.de_la_etiqueta(t)), None)] if e)
    return cuenta.most_common(1)[0][0] if cuenta else None


def de_cada_carta(clases, cartas):
    """{id de la carta: familia} de todas las cartas de las clases (classes.json y skills.json, sus entries)."""
    por_clase = {}
    for c in cartas:
        for k in c.get("classTags") or []:
            por_clase.setdefault(k, []).append(c)
    out = {}
    for clase in clases:
        suyas = por_clase.get(clase["id"], [])
        elemento = elemento_de_la_clase(clase, suyas)
        for c in suyas:
            out.setdefault(c["id"], de_la_carta(c, clase["id"], elemento))
    return out
