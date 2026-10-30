"""CP-01 — Las clases propias de ingenio y oficios: los datos.

Oriol (07/10/2026): «un ingeniero (que se dedique a crear torretas, trampas y utiliza su ingenio), 1 mercader,
alquimista, gambito o un símil tarotista». Cuatro clases propias con la forma de las SP (ficha, 7 cartas + 3 de
Creador, pasiva y un set de tier 2 que la clase no lleva): el Ingeniero de Asedio, el Mercader Errante, el Alquimista
y el Cartomante. No salen de una ficha de NosTale: los dados y las rondas se escriben tal cual, dentro del presupuesto
de planificacion/CANON_CLASES_PROPIAS.md (§3), con las piezas de cp_reglas.py. Lo escribe en los catálogos
tools/importar_clases_sp.py.

Criterios comunes a las cuatro:
- Sin pólvora (Oriol, 07/10): las torretas son ballestas rúnicas con un núcleo alquímico; la mina, el petardo y las
  bombas cargan sal de fuego y alquimia.
- Ninguna repite lo de otra. Las ideas del encargo chocaban en cinco sitios y cada una se queda en una sola clase: la
  lluvia de 2d6 a todos es del Mercader (los naipes del Cartomante cortan y hacen sangrar); la poción de 2d6 a uno,
  del Mercader (el Alquimista cura con niebla a todo el grupo); el humo que ciega, del Alquimista (el Ingeniero lleva
  un petardo); la marca, del Cartomante (el Mercader no lleva pagaré); y el -1 a la CA de un golpe, del ácido del
  Alquimista (el llavazo del Ingeniero aturde).
- Lo que deja una reacción, al atacante o a ti, dura 2 turnos: salta en el turno del enemigo y tiene que llegar al
  tuyo (con 1 se acabaría al cerrar la misma ronda). Por lo mismo, lo que ayuda a tus golpes contra un enemigo (-1 a
  la CA, se defiende con desventaja, los golpes contra él son críticos) dura 2 turnos o más.
- La marca (`marcado`) es del que la pone: «tus golpes le hacen +1» (la mesa no se la da al resto del grupo).
- Las invocaciones (las torretas y la escolta) llevan las cifras de una criatura del bestiario y atacan al cerrar
  cada turno tuyo, como el Lobo solar del Cazasoles; duran 3 turnos (solo el compañero del Domador dura todo el
  combate).
"""
from cp_reglas import (arma_comun, aturdido, azar_una, carta, cond, cura, dano, elige_una, estado, invoca, pieza,
                       propio, si, siempre)


# --- C1. Ingeniero de Asedio -------------------------------------------------------------------------------------

INGENIERO_DE_ASEDIO = {
    "id": "ingeniero_de_asedio", "name": "Ingeniero de Asedio", "familia": "ingeniero",
    "elemento": None,  # la clase no tiene; el fuego, en las minas y el petardo
    "deidad": None,
    "referencias": ["NosTale: Granadero", "NosTale: Servidor del cañón de fuego", "NosTale: Destructor (las minas)",
                    "NosTale: Espía (la trampa)", "NosTale: Cazasoles (la invocación)"],
    "description": ("Ingeniero de las guerras de asedio: monta torretas de ballesta rúnica con un núcleo alquímico, "
                    "esconde cepos y minas bajo los pies del enemigo y lo arregla todo con su llave."),
    "primaryStat": "INT", "secondaryStat": "DES", "role": "balanced",
    "tira": "INT",
    "pasiva": ("Llave bien apretada",
               "La llave del Ingeniero aprieta pernos y cascos por igual: tus golpes con arma hacen +1 de daño.",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    # Las torretas (las dos invocaciones). Sin cifras de NosTale: las de un artefacto del bestiario que dispara, como
    # el Lobo solar del Cazasoles; INT 0, como las invocaciones (el cargador lee CON, DES, INT y CAR). La de ballesta,
    # el Dron Chispeante (tier 1: CON 1, DES 3, CAR 1) con los 5 PV de las invocaciones de tier 1; la pesada (Creador),
    # la Boca de Vapor (tier 2: CON 2, DES 4, CAR 2) con sus 8 PV.
    "invocaciones": [
        {"id": "torreta_de_ballesta", "name": "Torreta de ballesta", "description": "CON 1, DES 3, INT 0, CAR 1, 5 PV.",
         "health": 5, "duration": "3 turnos", "summonedBy": "ingeniero_de_asedio_torreta_de_ballesta",
         "stats": {"CON": 1, "DES": 3, "INT": 0, "CAR": 1},
         "attacks": [
             {"name": "Virote rúnico", "effect": "Dispara un virote rúnico al enemigo al cerrar cada turno tuyo."},
         ],
         "passive": {"name": "Núcleo alquímico",
                     "description": ("Una ballesta rúnica sobre un trípode, cargada por un núcleo alquímico: dispara "
                                     "una vez al cerrar cada turno tuyo y se desmonta a los 3 turnos.")}},
        {"id": "torreta_pesada", "name": "Torreta pesada", "description": "CON 2, DES 4, INT 0, CAR 2, 8 PV.",
         "health": 8, "duration": "3 turnos", "summonedBy": "ingeniero_de_asedio_torreta_pesada",
         "stats": {"CON": 2, "DES": 4, "INT": 0, "CAR": 2},
         "attacks": [
             {"name": "Virote de asedio",
              "effect": "Dispara un virote de asedio al enemigo al cerrar cada turno tuyo."},
         ],
         "passive": {"name": "Doble núcleo",
                     "description": ("Una gran ballesta de asedio blindada con planchas de hierro y cargada por dos "
                                     "núcleos alquímicos: dispara una vez al cerrar cada turno tuyo y se desmonta a "
                                     "los 3 turnos.")}},
    ],
    "cartas": [
        # La carta que trae la torreta («invocaciones»).
        carta("propia", "Torreta de ballesta", "self",
              "Montas una ballesta rúnica sobre un trípode, con un núcleo alquímico que la carga: la torreta dispara a "
              "tu lado durante 3 turnos, al cerrar cada turno tuyo.",
              [("La torreta de ballesta dispara a tu lado 3 turnos.", invoca(3, "torreta_de_ballesta"))]),
        # La trampa del Espía que en NosTale no tenía cifras, aquí con las suyas. El encargo decía caído 1 turno: al
        # atacante le dura 2, como todo lo que deja una reacción (con 1 se acabaría antes de tu turno).
        carta("NosTale: Espía, Trampa móvil", "Cepo", "self",
              "Escondes un cepo de hierro a tus pies. El próximo golpe que te llegue se queda en él: lo bloqueas, y el "
              "atacante cae al suelo 2 turnos (ataca con desventaja y los golpes cuerpo a cuerpo contra él van con "
              "ventaja).",
              [("Bloqueas el golpe.", "bloquea:ataque"),
               ("El atacante cae al suelo 2 turnos.", cond("caido", 2))], accion="reaccion"),
        # El encargo le daba -1 a la CA, que es lo del Frasco de ácido del Alquimista: el llavazo aturde.
        carta("propia", "Llavazo", "single_enemy",
              "Un golpe seco de llave en el casco: 1d6 de daño, y queda aturdido 1 turno (no ataca y se defiende con "
              "desventaja).",
              [("1d6 de daño.", dano("1d6"))] + aturdido(1)),
        # En lugar de la bomba de humo del encargo: el humo que ciega es del Alquimista (Humo de azufre).
        carta("asedio: el petardo", "Petardo", "single_enemy",
              "Le pegas a la armadura un petardo de sal de fuego, de los que vuelan las puertas de una muralla, y "
              "estalla: 3d6 de daño de fuego.",
              [("3d6 de daño de fuego.", dano("3d6"))], elemento="Fuego"),
        carta("NosTale: Destructor, Mina de fuego", "Mina rúnica", "zone_radius",
              "Entierras una mina con una runa de fuego y la haces saltar bajo los pies del enemigo: 2d6 de daño de "
              "fuego, y cae al suelo 1 turno (ataca con desventaja y los golpes cuerpo a cuerpo contra él van con "
              "ventaja).",
              [("2d6 de daño de fuego.", dano("2d6")),
               ("La onda lo tira al suelo 1 turno.", cond("caido", 1))], elemento="Fuego"),
        # Al grupo entero: también a las torretas que estén montadas (son aliados en la mesa).
        carta("propia", "Trayectoria calculada", "all_allies",
              "Calculas cada tiro con el compás y la regla: tú, tus aliados y las torretas que tengas montadas atacáis "
              "con ventaja durante 3 turnos.",
              [("Atacáis con ventaja 3 turnos.", estado(3, "ventaja:ataque"))]),
        # +2 a uno solo durante 2 turnos, como lo pide el encargo (al grupo entero sería +1). A las torretas no hace
        # falta: los enemigos no las atacan (van a por ti o a por tus acompañantes).
        carta("propia", "Remiendo de campaña", "single_ally",
              "Remachas una plancha de hierro en la armadura de un aliado (o en la tuya): +2 a su CA durante 2 turnos.",
              [("Plancha remachada: +2 a la CA 2 turnos.", estado(2, "bono_ca:2"))]),
    ],
    "creador": [
        # Una torreta de 3 turnos, más fuerte y con 8 PV; y, como es de Creador, te parapetas tras ella.
        carta("propia", "Torreta pesada", "self",
              "Montas una gran ballesta de asedio, blindada y con dos núcleos alquímicos: la torreta pesada (8 PV) "
              "dispara a tu lado durante 3 turnos, y tú te parapetas tras ella: +1 a tu CA 3 turnos.",
              [("La torreta pesada dispara a tu lado 3 turnos.", invoca(3, "torreta_pesada")),
               ("Te parapetas tras ella: +1 a tu CA 3 turnos.", propio(3, "bono_ca:1"))]),
        carta("NosTale: Destructor, Colocabombas", "Campo de minas", "all_enemies",
              "Siembras el campo entero de minas rúnicas y las haces saltar a la vez: 4d6 de daño de fuego a cada "
              "enemigo, y caen al suelo 1 turno (atacan con desventaja y los golpes cuerpo a cuerpo contra ellos van "
              "con ventaja).",
              [("4d6 de daño de fuego a cada enemigo.", dano("4d6")),
               ("La onda los tira al suelo 1 turno.", cond("caido", 1))], elemento="Fuego"),
        carta("asedio: el ariete", "Ariete de asedio", "single_enemy",
              "Embistes al enemigo con un ariete de cabeza de bronce que empuja un núcleo alquímico: 4d6 de daño, y le "
              "revientas la guardia 2 turnos (-2 a la CA).",
              [("4d6 de daño.", dano("4d6")),
               ("Guardia rota: -2 a la CA 2 turnos.", cond("guardia_rota", 2))]),
    ],
    # La que pega (y aturde), la que trae la torreta y la trampa.
    "arranque": ["Llavazo", "Torreta de ballesta", "Cepo"],
    "arma_inicial": "llave_de_ingeniero_de_viaje",
    "armas_nuevas": [
        arma_comun("Llave de Ingeniero de Viaje", "media",
                   "Una llave inglesa enorme de hierro, de las que aprietan los pernos de una máquina de asedio.",
                   restricciones=["Precisión: INT"]),
    ],
    # balanced: armadura media.
    "set_de": "Ingeniero",
    "set": [
        pieza("Llave del Ingeniero", "MainHand", "media", {"INT": 1, "DES": 1},
              "La llave del set del Ingeniero: una llave inglesa enorme de hierro pavonado, con la boca ajustable de "
              "latón y el mango envuelto en cuero.",
              restricciones=["Precisión: INT"]),
        pieza("Mandil del Ingeniero", "Torso", "media", {"INT": 1},
              "Mandil de cuero marrón con los bolsillos llenos de herramientas, sobre un correaje cruzado con remaches "
              "de latón y hombreras de cuero."),
        pieza("Casco del Ingeniero", "Head", "media", {"INT": 1, "DES": 1},
              "Casco de zapador de cuero marrón, redondo y con barboquejo, con unas gafas de latón de cristales "
              "redondos subidas en la frente."),
        pieza("Pantalón del Ingeniero", "Legs", "media", {"INT": 1, "DES": 1},
              "Pantalón de trabajo de lona parda con rodilleras de cuero."),
        pieza("Botas del Ingeniero", "Feet", "media", {"INT": 1, "DES": 1},
              "Botas de trabajo de cuero marrón con la puntera de hierro."),
    ],
}


# --- C2. Mercader Errante ----------------------------------------------------------------------------------------

MERCADER_ERRANTE = {
    "id": "mercader_errante", "name": "Mercader Errante", "familia": "mercader",
    "elemento": None, "deidad": None,
    "referencias": ["NosTale: Atuendo de boda (el oro)", "NosTale: Cazasoles (la escolta, como el Lobo solar)",
                    "NosTale: Mago sagrado (curas y bendiciones)"],
    "description": ("Comerciante de los caminos para quien el oro es magia: soborna al enemigo, contrata escoltas, "
                    "vende pociones a mitad de combate y asegura la vida de los suyos."),
    "primaryStat": "CAR", "secondaryStat": "INT", "role": "support",
    "tira": "CAR",
    # Ni «siempre tiene una salida» (te defiendes con ventaja: lo da el Regateo) ni «género de calidad» (+1 a las curas:
    # sería +1 a una sola poción por combate): la cota que lleva bajo la casaca, +1 a la CA.
    "pasiva": ("Cota bajo la casaca",
               "Bajo la casaca de brocado llevas una cota de malla fina, por si el trato sale mal: +1 a la CA.",
               [("+1 a la CA.", siempre("bono_ca:1"))]),
    # La escolta: un mercenario del bestiario, el Recluta Sobornado («Juraron proteger la puerta norte. Ahora cobran
    # por abrirla»: tier 1, CON 2, DES 4, INT 1, CAR 1, con su lanza), y los 5 PV de las invocaciones de tier 1. Es
    # una persona: conserva su INT.
    "invocaciones": [
        {"id": "escolta_a_sueldo", "name": "Escolta a sueldo", "description": "CON 2, DES 4, INT 1, CAR 1, 5 PV.",
         "health": 5, "duration": "3 turnos", "summonedBy": "mercader_errante_contratar_escolta",
         "stats": {"CON": 2, "DES": 4, "INT": 1, "CAR": 1},
         "attacks": [
             {"name": "Lanzada a sueldo", "effect": "Ataca con la lanza al enemigo al cerrar cada turno tuyo."},
         ],
         "passive": {"name": "Pagado por adelantado",
                     "description": ("Un mercenario del camino con lanza y peto de cuero: lucha a tu lado lo que has "
                                     "pagado, 3 turnos, y se va.")}},
    ],
    "cartas": [
        carta("propia", "Soborno", "single_enemy",
              "Le pones en la mano una bolsa para que mire hacia otro lado. Si no se resiste (defensa mental), se "
              "queda 1 turno sin atacar.",
              [("Sobornado: 1 turno sin atacar.", estado(1, "incapacitado"))], salvacion="defensa_mental"),
        # La carta que trae la escolta («invocaciones»).
        carta("propia", "Contratar escolta", "self",
              "Pagas a un mercenario del camino para que luche a tu lado: la escolta a sueldo ataca durante 3 turnos, "
              "al cerrar cada turno tuyo.",
              [("La escolta a sueldo lucha a tu lado 3 turnos.", invoca(3, "escolta_a_sueldo"))]),
        carta("propia", "Poción de la tienda", "single_ally",
              "Sacas la poción buena, la que vendes cara, y se la das a un aliado (o te la bebes tú): se cura 2d6.",
              [("Se cura 2d6.", cura("2d6"))]),
        carta("NosTale: Atuendo de boda, Lluvia de confeti", "Lluvia de monedas", "all_enemies",
              "Lanzas un puñado de monedas con tanta fuerza que silban como piedras de honda: 2d6 de daño a cada "
              "enemigo.",
              [("2d6 de daño a cada enemigo.", dano("2d6"))]),
        carta("propia", "Oferta irrechazable", "single_enemy",
              "Le haces una oferta que no puede rechazar y se queda dándole vueltas. Si no se resiste (defensa "
              "mental), ataca y se defiende con desventaja 2 turnos.",
              [("Ataca y se defiende con desventaja 2 turnos.", estado(2, "desventaja:todo"))],
              salvacion="defensa_mental"),
        # En lugar del pagaré del encargo (la marca es del Cartomante): la prima, que llega también a la escolta.
        carta("propia", "Prima de combate", "all_allies",
              "Prometes una prima al que más golpee: tú, tus aliados y tu escolta hacéis +1 con los golpes de arma "
              "durante 3 turnos.",
              [("Prima de combate: +1 a los golpes con arma 3 turnos.", estado(3, "bono_dano_arma:1"))]),
        # El encargo dejaba al atacante con desventaja. En una reacción solo le llega una condición del catálogo, y
        # ninguna es regatear (asustado es miedo; envenenado, veneno): lo que ganas tú (canon §3), 2 turnos.
        carta("propia", "Regateo", "self",
              "Le regateas el golpe hasta dejarlo en nada: lo bloqueas, y durante 2 turnos te defiendes con ventaja.",
              [("Bloqueas el golpe.", "bloquea:ataque"),
               ("Te defiendes con ventaja 2 turnos.", propio(2, "ventaja:defensa"))], accion="reaccion"),
    ],
    "creador": [
        # Vuelve con 1 PV, como el Último Aliento del Bardo de las Cuatro Estaciones (y las otras clases propias).
        carta("propia", "Seguro de vida", "single_ally",
              "Firmas una póliza sobre la vida de un aliado (o la tuya): si cae, la póliza paga y se levanta con 1 PV.",
              [("Si cae, vuelve con 1 PV.", "revive:1")], accion="reaccion"),
        carta("propia", "La caravana", "all_allies",
              "Cerráis las carretas de la caravana en círculo y peleáis tras ellas: tú y tus aliados tenéis +2 a la CA "
              "hasta el final del combate.",
              [("La caravana: +2 a la CA hasta el final del combate.", estado("combate", "bono_ca:2"))]),
        carta("propia", "Liquidación total", "all_enemies",
              "Todo tiene que salir: vacías la caravana sobre los enemigos, monedas, lingotes y cacharros: 4d6 de daño "
              "a cada uno.",
              [("4d6 de daño a cada enemigo.", dano("4d6"))]),
    ],
    # La que controla, la que trae la escolta y la de apoyo.
    "arranque": ["Soborno", "Contratar escolta", "Poción de la tienda"],
    "arma_inicial": "bolsa_de_monedas_de_viaje",
    "armas_nuevas": [
        arma_comun("Bolsa de Monedas de Viaje", "ligera",
                   "Una bolsa de cuero llena de monedas atada a una cuerda, que se voltea como un mangual.",
                   restricciones=["Precisión: CAR"]),
    ],
    # support: armadura media.
    "set_de": "Mercader",
    "set": [
        pieza("Bolsa del Mercader", "MainHand", "ligera", {"CAR": 1, "INT": 1},
              "La bolsa del set del Mercader: una bolsa de terciopelo granate bordada en oro, repleta de monedas y "
              "atada a una cuerda trenzada para voltearla como un mangual.",
              restricciones=["Precisión: CAR"]),
        pieza("Casaca del Mercader", "Torso", "media", {"CAR": 1},
              "Casaca de brocado granate y oro con una balanza bordada en el pecho y bolsas de cuero colgadas del "
              "cinto."),
        pieza("Bonete del Mercader", "Head", "media", {"CAR": 1, "INT": 1},
              "Bonete ancho de terciopelo granate con la cinta de oro, una pluma blanca y una moneda de oro."),
        pieza("Bombachos del Mercader", "Legs", "media", {"CAR": 1, "INT": 1},
              "Bombachos pardos, anchos y recogidos bajo la rodilla."),
        pieza("Botas del Mercader", "Feet", "media", {"CAR": 1, "INT": 1},
              "Botas de montar de cuero marrón, altas hasta la rodilla."),
    ],
}


# --- C3. Alquimista ----------------------------------------------------------------------------------------------

ALQUIMISTA = {
    "id": "alquimista", "name": "Alquimista", "familia": "alquimista",
    # el de cada bomba: Fuego (la incendiaria y la gran obra), Agua (la escarcha), Tinieblas (el ácido)
    "elemento": None,
    "deidad": None,
    "referencias": ["NosTale: Granadero (las granadas alquímicas)", "NosTale: Destructor (las bombas)",
                    "NosTale: Monje de lucha (la forma en la pasiva)", "NosTale: Mago sagrado (las curas)"],
    "description": ("Sabio del alambique que prepara la tria prima: lanza bombas de fuego, escarcha y ácido, "
                    "transmuta la carne en plomo, reparte elixires y persigue la gran obra."),
    "primaryStat": "INT", "secondaryStat": "CON", "role": "caster",
    "tira": "INT",
    # La tria prima es la forma de la clase (canon §5: elegir), como las meditaciones del Monje de lucha.
    "pasiva": ("Tria prima",
               "Al empezar el combate preparas uno de los tres principios. Sal: +1 a la CA. Azufre: tus golpes con "
               "arma hacen +1 de daño. Mercurio: te defiendes con ventaja.",
               [("Eliges un principio al empezar el combate.", elige_una("sal", "azufre", "mercurio")),
                ("Sal: +1 a la CA.", si("sal", "bono_ca:1")),
                ("Azufre: tus golpes con arma hacen +1 de daño.", si("azufre", "bono_dano_arma:1")),
                ("Mercurio: te defiendes con ventaja.", si("mercurio", "ventaja:defensa"))]),
    "cartas": [
        carta("NosTale: Granadero, Granada de napalm", "Bomba incendiaria", "zone_radius",
              "Lanzas un frasco de aceite alquímico que estalla en llamas sobre el enemigo: 2d6 de daño de fuego, y "
              "arde: 1 de daño por turno durante 2 turnos.",
              [("2d6 de daño de fuego.", dano("2d6")),
               ("Arde: 1 de daño por turno durante 2 turnos.", estado(2, "dano_por_turno:1"))], elemento="Fuego"),
        carta("alquimia: la transmutación", "Transmutar en plomo", "single_enemy",
              "Transmutas en plomo la carne del enemigo. Si no se resiste (resistencia física), queda 1 turno sin "
              "atacar y, pesado como el plomo, se defiende con desventaja.",
              [("Plomo: 1 turno sin atacar.", estado(1, "incapacitado")),
               ("Plomo: se defiende con desventaja 1 turno.", estado(1, "desventaja:defensa"))],
              salvacion="resistencia_fisica"),
        # En lugar de la poción curativa a uno (esa es la Poción de la tienda del Mercader): una niebla para todos.
        carta("NosTale: Mago sagrado, Cura de grupo", "Niebla balsámica", "all_allies",
              "Rompes contra el suelo una redoma de bálsamo y su niebla tibia os cierra las heridas: tú y tus aliados "
              "os curáis 1d6.",
              [("Os curáis 1d6.", cura("1d6"))]),
        # El encargo decía 3 turnos; con un golpe, el estado dura 1-2 (canon §3): 2, que llega a tu turno siguiente.
        carta("propia", "Frasco de ácido", "single_enemy",
              "Le rompes encima un frasco de ácido que muerde la carne y el metal: 2d6 de daño de veneno, y con la "
              "armadura corroída tiene -1 a la CA 2 turnos.",
              [("2d6 de daño de veneno.", dano("2d6")),
               ("Armadura corroída: -1 a la CA 2 turnos.", estado(2, "penal_ca:1"))], elemento="Veneno"),
        # Se defienden con desventaja 2 turnos: con 1 se acabaría al cerrar esta ronda, antes de tu próximo golpe.
        carta("propia", "Bomba de escarcha", "all_enemies",
              "Una bomba de sales frías estalla en escarcha sobre los enemigos: 1d6 de daño de hielo a cada uno, y "
              "entumecidos se defienden con desventaja 2 turnos.",
              [("1d6 de daño de hielo a cada enemigo.", dano("1d6")),
               ("Entumecidos: se defienden con desventaja 2 turnos.", estado(2, "desventaja:defensa"))],
              elemento="Hielo"),
        carta("propia", "Elixir de vigor", "single_ally",
              "Le das a beber a un aliado (o te bebes tú) un elixir de vigor: sus golpes con arma hacen +1 durante 3 "
              "turnos.",
              [("Elixir de vigor: +1 a los golpes con arma 3 turnos.", estado(3, "bono_dano_arma:1"))]),
        # Un humo de alquimista, del cuerpo: salvación de resistencia física (canon §4), como la Bomba de gas tóxico
        # del Destructor.
        carta("propia", "Humo de azufre", "all_enemies",
              "Rompes un frasco de azufre y un humo amarillo que pica en los ojos lo cubre todo. Quien no se resista "
              "(resistencia física) queda cegado 1 turno: ataca con desventaja y todos los golpes contra él van con "
              "ventaja.",
              [("Cegado 1 turno.", cond("cegado", 1))], salvacion="resistencia_fisica"),
    ],
    "creador": [
        # Solo para ti: el que levanta a un aliado es el Seguro de vida del Mercader. 1 PV, como el Último Aliento.
        carta("alquimia: la piedra filosofal", "La piedra filosofal", "self",
              "Guardas bajo la lengua una pizca de la piedra filosofal: si caes, el elixir de la vida te levanta con 1 "
              "PV.",
              [("Si caes, vuelves con 1 PV.", "revive:1")], accion="reaccion"),
        # Primero limpia y después cura (canon de las SP, §4).
        carta("alquimia: la panacea", "La panacea", "all_allies",
              "Destilas la panacea, el remedio de todos los males: tú y tus aliados os libráis de los estados en "
              "contra y os curáis 2d6.",
              [("Os libráis de los estados en contra.", "limpia"), ("Os curáis 2d6.", cura("2d6"))]),
        carta("alquimia: la gran obra", "La gran obra", "all_enemies",
              "Culminas la gran obra en un solo frasco y lo arrojas: el fuego rojo de la piedra cae sobre los "
              "enemigos, 4d6 de daño de fuego a cada uno, y arden: 1 de daño por turno durante 2 turnos.",
              [("4d6 de daño de fuego a cada enemigo.", dano("4d6")),
               ("Arden: 1 de daño por turno durante 2 turnos.", estado(2, "dano_por_turno:1"))], elemento="Fuego"),
    ],
    # La bomba (lo suyo), la que controla y la de apoyo.
    "arranque": ["Bomba incendiaria", "Transmutar en plomo", "Niebla balsámica"],
    "arma_inicial": "matraz_de_viaje",
    "armas_nuevas": [
        arma_comun("Matraz de Viaje", "ligera", "Un matraz de cristal con un líquido que burbujea.",
                   restricciones=["Precisión: INT"]),
    ],
    # caster: armadura ligera. El tema «alquimista» de actores_estetica ya tiñe de verde oscuro lo que se llame así.
    "set_de": "Alquimista",
    "set": [
        pieza("Matraz del Alquimista", "MainHand", "ligera", {"INT": 1, "CON": 1},
              "El matraz del set del Alquimista: un matraz de cristal verde botella con un líquido dorado que "
              "burbujea, tapado con corcho y sellado con lacre.",
              restricciones=["Precisión: INT"]),
        pieza("Gabán del Alquimista", "Torso", "ligera", {"INT": 1},
              "Gabán largo verde botella con una bandolera de frascos de colores (rojo, azul y amarillo) cruzada al "
              "pecho y un mandil de cuero manchado encima."),
        pieza("Sombrero del Alquimista", "Head", "ligera", {"INT": 1, "CON": 1},
              "Sombrero de copa baja verde botella con unos anteojos de aumento de latón en la cinta."),
        pieza("Pantalón del Alquimista", "Legs", "ligera", {"INT": 1, "CON": 1},
              "Pantalón negro de paño."),
        pieza("Botas del Alquimista", "Feet", "ligera", {"INT": 1, "CON": 1},
              "Botas altas negras."),
    ],
}


# --- C4. Cartomante ----------------------------------------------------------------------------------------------

CARTOMANTE = {
    "id": "cartomante", "name": "Cartomante", "familia": "cartomante",
    "elemento": "Arcano",  # el de cada arcano: la Muerte, Tinieblas; los naipes, ninguno
    "deidad": None,
    "referencias": ["NosTale: Vidente (el tarot)", "NosTale: Espía (las reacciones)",
                    "NosTale: Monje de lucha (la forma en la pasiva, aquí al azar)"],
    "description": ("Tahúr que lee el tarot: echa las cartas al empezar el combate y la suerte decide su arcano; "
                    "lanza naipes cargados de magia, se guarda un as en la manga y hace girar la fortuna de los "
                    "suyos."),
    "primaryStat": "INT", "secondaryStat": "CAR", "role": "caster",
    "tira": "INT",
    # La lectura (canon §5 y §8): la opción la decide la suerte, con el gancho nuevo azar_una (una fila «Echar las
    # cartas» al empezar el combate). La Estrella cura más: por eso la clase lleva una cura, la Templanza.
    "pasiva": ("La lectura",
               "Al empezar el combate echas las cartas y la suerte saca tu arcano. El Sol: atacas con ventaja. La "
               "Luna: te defiendes con ventaja. La Estrella: tus curas curan +1. La Torre: tus golpes con arma hacen "
               "+1 de daño.",
               [("La suerte saca tu arcano al empezar el combate.", azar_una("sol", "luna", "estrella", "torre")),
                ("El Sol: atacas con ventaja.", si("sol", "ventaja:ataque")),
                ("La Luna: te defiendes con ventaja.", si("luna", "ventaja:defensa")),
                ("La Estrella: tus curas curan +1.", si("estrella", "bono_cura:1")),
                ("La Torre: tus golpes con arma hacen +1 de daño.", si("torre", "bono_dano_arma:1"))]),
    "cartas": [
        carta("propia", "Naipe cortante", "single_enemy",
              "Lanzas de canto un naipe cargado de magia que corta como una navaja: 3d6 de daño arcano.",
              [("3d6 de daño arcano.", dano("3d6"))], elemento="Arcano"),
        # Para un aliado o para ti: salta en su próxima tirada, de ataque o de defensa, como la Égida Divina del Bardo
        # de las Cuatro Estaciones.
        carta("tarot: la Rueda de la Fortuna", "La Rueda de la Fortuna", "single_ally",
              "Haces girar la Rueda de la Fortuna por un aliado (o por ti): en su próxima tirada, de ataque o de "
              "defensa, tira 2 dados más.",
              [("Su próxima tirada: +2 dados.", "dados_extra:2")], accion="reaccion", elemento="Arcano"),
        carta("propia", "As en la manga", "self",
              "Te guardas un as en la manga. Cuando te llegue el próximo golpe, lo bloqueas, y con la mano ganada "
              "atacas con ventaja 2 turnos.",
              [("Bloqueas el golpe.", "bloquea:ataque"),
               ("Atacas con ventaja 2 turnos.", propio(2, "ventaja:ataque"))], accion="reaccion"),
        # La marca es del Cartomante (el Mercader no lleva pagaré): «tus golpes le hacen +1», los de quien la pone.
        carta("propia", "Carta marcada", "single_enemy",
              "Le clavas un naipe marcado: 2d6 de daño arcano, y ya sabes qué juego lleva: queda marcado 3 turnos (tus golpes "
              "le hacen +1).",
              [("2d6 de daño arcano.", dano("2d6")),
               ("Marcado 3 turnos: tus golpes le hacen +1.", cond("marcado", 3))], elemento="Arcano"),
        # En lugar de la lluvia de naipes del encargo (2d6 a todos es la Lluvia de monedas del Mercader): menos daño y
        # los cantos que cortan.
        carta("propia", "Abanico de naipes", "all_enemies",
              "Abres la baraja en abanico y la lanzas entera: 1d6 de daño arcano a cada enemigo, y los cantos de los naipes "
              "los dejan sangrando 2 turnos (1 de daño por turno).",
              [("1d6 de daño arcano a cada enemigo.", dano("1d6")),
               ("Sangran 2 turnos.", cond("sangrado", 2))], elemento="Arcano"),
        # «Farol» a secas se confunde con los faroles de luz del catálogo (el Portador del Farol, el Farol de Ánimas).
        # Un engaño a la mente, sin daño: salvación de defensa mental (canon §4).
        carta("póquer: el farol", "Farol de tahúr", "all_enemies",
              "Te tiras un farol tan grande que los enemigos dudan de todo. Quien no se resista (defensa mental) ataca "
              "con desventaja 2 turnos.",
              [("Atacan con desventaja 2 turnos.", estado(2, "desventaja:ataque"))], salvacion="defensa_mental"),
        # En lugar de El colgado del encargo («1 turno sin atacar» ya lo hacen el Soborno del Mercader y el Plomo del
        # Alquimista): el arcano que cura, para que La Estrella de la lectura sirva.
        carta("tarot: la Templanza", "La Templanza", "single_ally",
              "Echas el arcano de la Templanza sobre un aliado (o sobre ti): el agua de sus dos copas lo libra de los "
              "estados en contra y lo cura 1d6.",
              [("Se libra de los estados en contra.", "limpia"), ("Se cura 1d6.", cura("1d6"))], elemento="Arcano"),
    ],
    "creador": [
        # Los golpes contra él son críticos 2 turnos: con 1 se acabaría al cerrar esta ronda, antes de tu turno.
        carta("tarot: la Muerte", "La Muerte", "single_enemy",
              "Le muestras el arcano de la Muerte: 4d6 de daño de tinieblas, y durante 2 turnos los golpes contra él "
              "son críticos.",
              [("4d6 de daño de tinieblas.", dano("4d6")),
               ("La Muerte lo ronda: los golpes contra él son críticos 2 turnos.", estado(2, "critico_contra"))],
              elemento="Tinieblas"),
        carta("tarot: el Mundo", "El Mundo", "all_allies",
              "Sale el arcano del Mundo, la partida ganada: tú y tus aliados hacéis +1d6 con los golpes de arma hasta "
              "el final del combate.",
              [("El Mundo: +1d6 a los golpes con arma hasta el final del combate.",
                estado("combate", "bono_dano_arma:1d6"))], elemento="Arcano"),
        carta("póquer: la escalera real", "Escalera real", "all_enemies",
              "Despliegas la mano que nadie gana, del diez al as del mismo palo, y los cinco naipes caen sobre los "
              "enemigos: 4d6 de daño arcano a cada uno, y quedan marcados 2 turnos (tus golpes les hacen +1).",
              [("4d6 de daño arcano a cada enemigo.", dano("4d6")),
               ("Marcados 2 turnos: tus golpes les hacen +1.", cond("marcado", 2))], elemento="Arcano"),
    ],
    # La que pega, la de la suerte (lo suyo) y la defensa.
    "arranque": ["Naipe cortante", "La Rueda de la Fortuna", "As en la manga"],
    "arma_inicial": "naipes_de_viaje",
    "armas_nuevas": [
        arma_comun("Naipes de Viaje", "ligera", "Un abanico de naipes en la mano.",
                   restricciones=["Precisión: INT"]),
    ],
    # caster: armadura ligera.
    "set_de": "Cartomante",
    "set": [
        pieza("Naipes del Cartomante", "MainHand", "ligera", {"INT": 1, "CAR": 1},
              "Los naipes del set del Cartomante: un abanico de naipes de tarot con el reverso morado y los cantos "
              "dorados.",
              restricciones=["Precisión: INT"]),
        pieza("Levita del Cartomante", "Torso", "ligera", {"INT": 1},
              "Levita morada larga de cuello alto, abierta sobre un chaleco negro con los botones de oro."),
        pieza("Diadema del Cartomante", "Head", "ligera", {"INT": 1, "CAR": 1},
              "Diadema de cinta morada con tres naipes de tarot en abanico."),
        pieza("Pantalón del Cartomante", "Legs", "ligera", {"INT": 1, "CAR": 1},
              "Pantalón negro estrecho."),
        pieza("Zapatos del Cartomante", "Feet", "ligera", {"INT": 1, "CAR": 1},
              "Zapatos negros de punta."),
    ],
}


INGENIO = [INGENIERO_DE_ASEDIO, MERCADER_ERRANTE, ALQUIMISTA, CARTOMANTE]
