"""CP-01 — La tormenta, las ánimas y los piratas de Vinclap: cuatro clases propias (los datos).

Cuatro clases propias (planificacion/CANON_CLASES_PROPIAS.md) con la forma de las SP: el Tempestario (el rayo), el
Necromago de las Ánimas (las almas en pena y la Santa Compaña) y los dos piratas de Vinclap, el Corsario Vampírico y
el Forajido. No salen de una ficha de NosTale: los dados y las rondas van tal cual, dentro del presupuesto del canon
(§3), con las piezas de cp_reglas.py. Cuando una carta se aparta del brief o deja algo fuera, lo dice un comentario.

Criterios comunes a las cuatro:
- Las reacciones bloquean un golpe (`bloquea:ataque`) y dan una cosa al saltar: una condición al atacante (el
  Pararrayos, el Velo de ánimas), lo que gana quien reacciona, 2 turnos (la Forma de niebla) o librarse de los estados
  (el Barril de ron). Cada una distinta.
- Lo que una carta deja en un enemigo para que lo aproveches tú en tu turno siguiente (se defiende con desventaja,
  cegado, caído, los golpes críticos), 2 turnos: puesto en tu turno, con 1 se acabaría al cerrar la ronda, antes de tu
  golpe siguiente (el mismo motivo que los 2 turnos de lo que gana una reacción, §4 del canon de las SP). Lo que deja
  sin atacar, 1 turno (§3): llega al turno del enemigo de esa ronda.
- Las invocaciones atacan con su golpe al cerrar tu turno (CartasEnCombate::turnoInvocaciones), al primer enemigo en
  pie: sus «ataques» son el nombre de ese golpe, y la carta que las trae apunta a ti. Una por carta.
"""
from cp_reglas import (DOS_MANOS, arma_comun, aturdido, carta, cond, cura, dano, estado, invoca, pieza, propio,
                       siempre)

# Bloquear el golpe: la reacción de las cuatro (§3: `bloquea:ataque` y una cosa más al saltar).
BLOQUEA = ("Bloqueas el golpe.", "bloquea:ataque")


# --- E1. Tempestario -----------------------------------------------------------------------------------------------

TEMPESTARIO = {
    "id": "tempestario", "name": "Tempestario", "familia": "tempestario",
    # El rayo es su propio elemento (Oriol, 07/10: «no estamos limitados a los 4 elementos del nostale aqui»).
    "elemento": "Rayo", "deidad": "la_forja_del_trueno",
    "referencias": ["folclore: los tempestarios, que llamaban a la tormenta", "NosTale: Tronador", "NosTale: Thor",
                    "WoW: Cadena de relámpagos"],
    "description": ("Hechicero de la tormenta al servicio de La Forja del Trueno: lanza rayos que saltan de enemigo en "
                    "enemigo, los carga de electricidad estática, se protege con su pararrayos y desata la tormenta "
                    "entera."),
    "primaryStat": "INT", "secondaryStat": "CAR", "role": "caster",
    "tira": "INT",
    "pasiva": ("La vara cargada",
               "La vara del Tempestario guarda la carga de la tormenta: tus golpes con arma hacen +1 de daño (rayo).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("folclore: los tempestarios", "Saeta de rayo", "single_enemy",
              "Señalas al enemigo con la vara y el cielo le lanza una saeta de rayo: 3d6 de daño de rayo.",
              [("3d6 de daño de rayo.", dano("3d6"))], elemento="Rayo"),
        # La «cadena de relámpagos» del brief es el nombre de WoW tal cual (y el Nacido del Trueno tiene ya una Cadena
        # de Rayos): otro nombre. Sin posiciones, «salta de enemigo en enemigo» es a todos.
        carta("WoW: Cadena de relámpagos", "Rayo bifurcado", "all_enemies",
              "El rayo se bifurca y salta de enemigo en enemigo: 2d6 de daño de rayo a cada uno.",
              [("2d6 de daño de rayo a cada enemigo.", dano("2d6"))], elemento="Rayo"),
        # La mecánica de la clase: la marca (+1 a tus golpes) y, con el pelo de punta, sin defensa para tu golpe
        # siguiente (2 turnos, ver arriba). Un golpe con dos efectos: 1d6 (§3).
        carta("propia", "Carga estática", "single_enemy",
              "Cargas al enemigo de electricidad estática: 1d6 de daño de rayo; queda marcado 3 turnos (tus golpes le "
              "hacen +1) y, con el pelo de punta, se defiende con desventaja 2 turnos.",
              [("1d6 de daño de rayo.", dano("1d6")),
               ("Carga estática: marcado 3 turnos.", cond("marcado", 3)),
               ("Carga estática: se defiende con desventaja 2 turnos.", estado(2, "desventaja:defensa"))],
              elemento="Rayo"),
        # La bola de rayos que paraliza: no atacar (incapacitado). La condición «paralizado» del catálogo no lo hace
        # (solo frena y quita defensa), así que la carta no la usa.
        carta("NosTale: Tronador, Tiro de alta tensión", "Centella", "zone_radius",
              "Sueltas una centella, una bola de rayos que estalla sobre el enemigo: 2d6 de daño de rayo, y la descarga "
              "le deja 1 turno sin atacar.",
              [("2d6 de daño de rayo.", dano("2d6")),
               ("Descarga: 1 turno sin atacar.", estado(1, "incapacitado"))], elemento="Rayo"),
        # El brief: bloquea y +1 al arma 2 turnos. Es la misma carta que el escudo de relámpagos que el brief de la
        # parte D da al Chamán de los Espíritus: aquí la descarga salta al atacante y lo marca (la carga estática de la
        # clase). La condición la pone la reacción al saltar, sin tirada (como la ceguera del Escudo Divino).
        carta("propia", "Pararrayos", "self",
              "Alzas la vara como un pararrayos y atrae el golpe hacia ella: lo bloqueas, y la descarga salta al "
              "atacante y lo deja marcado 2 turnos (tus golpes le hacen +1).",
              [BLOQUEA, ("La descarga marca al atacante 2 turnos.", cond("marcado", 2))], accion="reaccion", elemento="Rayo"),
        # Daño que dura, no un control: sin salvación (como la Sobrecarga de combustible de la Tormenta de fuego); el
        # estado entra con la tirada.
        carta("folclore: los tempestarios", "Nubarrón", "all_enemies",
              "Llamas un nubarrón sobre los enemigos: sus rayos les quitan 1 de vida por turno durante 3 turnos.",
              [("Nubarrón: 1 de daño por turno durante 3 turnos.", estado(3, "dano_por_turno:1"))], elemento="Rayo"),
        # La «sobrecarga» del brief («Sobrecarga» es ya carta del Inventor): ventaja y +1 al arma, 2 turnos.
        carta("propia", "Tormenta en las venas", "self",
              "Dejas que la tormenta te corra por las venas: durante 2 turnos atacas con ventaja y tus golpes con arma "
              "hacen +1.",
              [("Tormenta en las venas: atacas con ventaja 2 turnos.", propio(2, "ventaja:ataque")),
               ("Tormenta en las venas: +1 a tus golpes con arma durante 2 turnos.", propio(2, "bono_dano_arma:1"))], elemento="Rayo"),
    ],
    "creador": [
        carta("folclore: los tempestarios", "Tormenta perfecta", "all_enemies",
              "Desatas la tormenta entera: 4d6 de daño de rayo a cada enemigo, y los rayos les prenden fuego: 1 de "
              "daño por turno durante 2 turnos.",
              [("4d6 de daño de rayo a cada enemigo.", dano("4d6")),
               ("Arden: 1 de daño por turno durante 2 turnos.", estado(2, "dano_por_turno:1"))], elemento="Rayo"),
        carta("propia", "Juicio del relámpago", "single_enemy",
              "La Forja del Trueno juzga al enemigo con un relámpago: 4d6 de daño de rayo y aturdimiento 1 turno (no "
              "ataca y se defiende con desventaja).",
              [("4d6 de daño de rayo.", dano("4d6"))] + aturdido(1), elemento="Rayo"),
        # «Ojo de la Tormenta» es un milagro de La Forja del Trueno (spells.json): otro nombre.
        carta("propia", "Ojo de la tempestad", "self",
              "Te plantas en el ojo de la tempestad, donde todo está en calma: hasta el final del combate tienes +2 a "
              "la CA y atacas con ventaja.",
              [("Ojo de la tempestad: +2 a tu CA hasta el final del combate.", propio("combate", "bono_ca:2")),
               ("Ojo de la tempestad: atacas con ventaja hasta el final del combate.",
                propio("combate", "ventaja:ataque"))], elemento="Rayo"),
    ],
    # Pega (la saeta), su mecánica (la carga estática que marca) y su defensa (el pararrayos).
    "arranque": ["Saeta de rayo", "Carga estática", "Pararrayos"],
    "arma_inicial": "vara_de_aprendiz_de_viaje",
    "set_de": "Tempestario",
    "set": [
        pieza("Vara del Tempestario", "MainHand", "ligera", {"INT": 1, "CAR": 1},
              "Vara de madera negra rematada en una horquilla de cobre, donde chisporrotea un rayo amarillo.",
              restricciones=["Precisión: INT"]),
        pieza("Túnica del Tempestario", "Torso", "ligera", {"INT": 1},
              "Túnica azul noche hasta la rodilla, con un rayo amarillo bordado en el pecho, el bajo amarillo y "
              "bobinas de cobre en los hombros."),
        pieza("Sombrero del Tempestario", "Head", "ligera", {"INT": 1, "CAR": 1},
              "Sombrero de ala ancha azul noche con dos pararrayos de cobre que salen de la copa."),
        pieza("Pantalón del Tempestario", "Legs", "ligera", {"INT": 1, "CAR": 1},
              "Pantalón azul oscuro del Tempestario."),
        pieza("Botas del Tempestario", "Feet", "ligera", {"INT": 1, "CAR": 1},
              "Botas de cuero negro con hebillas de cobre."),
    ],
}


# --- E2. Necromago de las Ánimas -----------------------------------------------------------------------------------

# El arma normal de tier 1 (el catálogo no tiene farol de combate: los dos que hay son reliquias épicas).
FAROL_DE_ANIMAS = arma_comun("Farol de Ánimas de Viaje", "ligera",
                             "Un farol con una luz verde pálida colgado de una vara corta.",
                             restricciones=["Precisión: INT"])

NECROMAGO_DE_LAS_ANIMAS = {
    "id": "necromago_de_las_animas", "name": "Necromago de las Ánimas", "familia": "necromago",
    "elemento": "Tinieblas", "deidad": None,
    "referencias": ["folclore: las almas en pena y la Santa Compaña",
                    "NosTale: Sacerdote vudú (las invocaciones cortas)", "NosTale: Cosechador de almas"],
    "description": ("Nigromante del farol de ánimas: llama a las almas en pena para que luchen por él, asusta con su "
                    "lamento, sujeta con sus cadenas y bebe la vida de los vivos hasta traer a la Santa Compaña."),
    "primaryStat": "INT", "secondaryStat": "CAR", "role": "caster",
    "tira": "INT",
    # Ventaja al defenderse (la otra pasiva del brief) y no el +1 al arma: el Tempestario y los dos piratas ya lo
    # llevan, y un lanzador que vive de sus almas necesita aguantar mientras luchan.
    "pasiva": ("Cortejo de ánimas",
               "Las ánimas que siguen la luz de tu farol desvían los golpes que te buscan: te defiendes con ventaja.",
               [("Te defiendes con ventaja.", siempre("ventaja:defensa"))]),
    # Las almas no tienen ficha: las cifras de una criatura del bestiario, como las arañas del Sacerdote vudú. El alma
    # en pena, las de la escaramuzadora de tier 1 de las tinieblas (Sombra del Paso Ligero: CON 1, DES 4, CAR 1; INT 0,
    # como las invocaciones) y los 5 PV de las invocaciones de tier 1. La Santa Compaña, más fuerte, las de la
    # escaramuzadora de tier 2 de las tinieblas (Excavador incansable: CON 2, DES 5, CAR 2) y sus 8 PV.
    "invocaciones": [
        {"id": "alma_en_pena", "name": "Alma en pena", "description": "CON 1, DES 4, INT 0, CAR 1, 5 PV.",
         "health": 5, "duration": "3 turnos", "summonedBy": "necromago_de_las_animas_llamar_a_un_alma_en_pena",
         "stats": {"CON": 1, "DES": 4, "INT": 0, "CAR": 1},
         "attacks": [
             {"name": "Toque helado",
              "effect": "Toca al enemigo con su mano helada (la carta Llamar a un alma en pena)."},
         ],
         "passive": {"name": "Pena sin descanso",
                     "description": ("Un alma envuelta en su sudario que sigue la luz del farol: ataca al cerrar tu "
                                     "turno y se desvanece a los 3 turnos.")}},
        {"id": "la_santa_compana", "name": "La Santa Compaña", "description": "CON 2, DES 5, INT 0, CAR 2, 8 PV.",
         "health": 8, "duration": "3 turnos", "summonedBy": "necromago_de_las_animas_la_santa_compana",
         "stats": {"CON": 2, "DES": 5, "INT": 0, "CAR": 2},
         "attacks": [
             {"name": "Paso de la procesión",
              "effect": "La procesión pasa sobre el enemigo con sus velas (la carta La Santa Compaña)."},
         ],
         "passive": {"name": "La procesión",
                     "description": ("Las ánimas de la Santa Compaña, en fila y con las velas encendidas, luchan como "
                                     "una sola criatura: atacan al cerrar tu turno y se van a los 3 turnos.")}},
    ],
    "cartas": [
        # La carta que trae al alma («invocaciones»), la mecánica de la clase.
        carta("folclore: las almas en pena", "Llamar a un alma en pena", "self",
              "Alzas el farol y acude un alma en pena: lucha a tu lado 3 turnos y, al cerrar cada turno tuyo, toca al "
              "enemigo con su mano helada.",
              [("Un alma en pena lucha a tu lado 3 turnos.", invoca(3, "alma_en_pena"))]),
        # Miedo, control sin daño y de magia: salvación de defensa mental (§4).
        carta("folclore: las almas en pena", "Lamento de las ánimas", "all_enemies",
              "Las ánimas lloran a tu alrededor. Quien falle la salvación (defensa mental) queda asustado 2 turnos "
              "(ataca con desventaja).",
              [("Lamento: asustado 2 turnos.", cond("asustado", 2))], salvacion="defensa_mental"),
        # El «toque gélido» del brief: otro nombre («Toque helado» es el golpe del alma en pena), y se defiende con
        # desventaja en vez de atacar con desventaja: eso ya lo hace el Lamento, y así prepara el golpe del alma.
        carta("propia", "Escalofrío", "single_enemy",
              "El frío de la tumba le cala los huesos: 2d6 de daño de tinieblas y, entumecido, se defiende con "
              "desventaja 2 turnos.",
              [("2d6 de daño de tinieblas.", dano("2d6")),
               ("Entumecido: se defiende con desventaja 2 turnos.", estado(2, "desventaja:defensa"))],
              elemento="Tinieblas"),
        # Un golpe que deja sin atacar (1d6, §3) y no el control con salvación del brief: ese control, con salvación de
        # defensa mental, es la Mirada hipnótica del Corsario Vampírico.
        carta("folclore: las almas en pena", "Cadenas de ánima", "single_enemy",
              "Las cadenas que arrastran las ánimas se enroscan en el enemigo: 1d6 de daño de tinieblas, y queda 1 "
              "turno sin atacar.",
              [("1d6 de daño de tinieblas.", dano("1d6")),
               ("Encadenado: 1 turno sin atacar.", estado(1, "incapacitado"))], elemento="Tinieblas"),
        # El brief: 2d6 y te curas lo que haces, que es el Mordisco del Corsario Vampírico. El vampiro bebe de golpe;
        # el farol bebe despacio: la absorción por turno (como el Captor de espíritus del Cosechador o la Absorción de
        # Alma del Apóstata, 1d6 por turno 3 turnos, de tier 1), 1d6 por turno 2 turnos, y el golpe baja a 1d6 para no
        # pasarse (§3: un golpe con un efecto). El trago grande, de una vez, es El último aliento (Creador).
        carta("NosTale: Cosechador de almas, Captor de espíritus", "Beber la vida", "single_enemy",
              "Acercas el farol al enemigo y su llama verde empieza a beberle la vida: 1d6 de daño de tinieblas, y "
              "durante 2 turnos le absorbes 1d6 de vida por turno (la pierde él y te la quedas tú).",
              [("1d6 de daño de tinieblas.", dano("1d6")),
               ("Le absorbes 1d6 de vida por turno durante 2 turnos.", estado(2, "absorbe_por_turno:1d6"))],
              elemento="Tinieblas"),
        carta("propia", "Velo de ánimas", "self",
              "Las ánimas se cruzan entre tú y el golpe: lo bloqueas, y quien te atacaba queda asustado 2 turnos "
              "(ataca con desventaja).",
              [BLOQUEA, ("El atacante queda asustado 2 turnos.", cond("asustado", 2))], accion="reaccion"),
        # Una maldición sin daño: salvación de defensa mental (§4). La marca (+1) y el crítico (+1d6 a cada golpe que le
        # acierte), 2 turnos para que lleguen a tu turno siguiente y a los golpes del alma.
        carta("NosTale: Cosechador de almas, Tinieblas mortales", "Marca de la muerte", "single_enemy",
              "Señalas al enemigo para la muerte. Si falla la salvación (defensa mental), durante 2 turnos queda "
              "marcado (tus golpes le hacen +1) y todos los golpes que le aciertan son críticos.",
              [("Marca de la muerte: marcado 2 turnos.", cond("marcado", 2)),
               ("Marca de la muerte: los golpes contra él son críticos 2 turnos.", estado(2, "critico_contra"))],
              salvacion="defensa_mental"),
    ],
    "creador": [
        # La procesión de ánimas del folclore: una sola criatura de 3 turnos, más fuerte que el alma (ver arriba).
        carta("folclore: la Santa Compaña", "La Santa Compaña", "self",
              "Llamas a la Santa Compaña: la procesión de ánimas sale de la niebla con sus velas encendidas y lucha a "
              "tu lado 3 turnos, como una sola criatura.",
              [("La Santa Compaña lucha a tu lado 3 turnos.", invoca(3, "la_santa_compana"))]),
        carta("propia", "El último aliento", "single_enemy",
              "Le arrancas al enemigo el último aliento: 4d6 de daño de tinieblas, y te curas el daño que haces.",
              [("4d6 de daño de tinieblas.", dano("4d6")), ("Te curas el daño que haces.", cura("dano_causado"))],
              elemento="Tinieblas"),
        carta("propia", "Camposanto", "all_enemies",
              "Abres el camposanto bajo los enemigos y las manos de los muertos los agarran: 4d6 de daño de tinieblas "
              "a cada enemigo, y quedan asustados 2 turnos.",
              [("4d6 de daño de tinieblas a cada enemigo.", dano("4d6")),
               ("Camposanto: asustados 2 turnos.", cond("asustado", 2))], elemento="Tinieblas"),
    ],
    # Su mecánica (el alma en pena), pega (el escalofrío, que además le quita la defensa ante el alma) y su defensa (el
    # velo).
    "arranque": ["Llamar a un alma en pena", "Escalofrío", "Velo de ánimas"],
    "arma_inicial": "farol_de_animas_de_viaje",
    "armas_nuevas": [FAROL_DE_ANIMAS],
    "set_de": "Necromago",
    "set": [
        pieza("Farol del Necromago", "MainHand", "ligera", {"INT": 1, "CAR": 1},
              "Farol de hierro negro con la luz verde pálida de las ánimas, colgado de una vara corta.",
              restricciones=["Precisión: INT"]),
        pieza("Túnica del Necromago", "Torso", "ligera", {"INT": 1},
              "Túnica negra hasta los pies, raída, con las mangas y el bajo violeta, el cinturón de cadena y un "
              "cordón de huesos al cuello."),
        pieza("Capucha del Necromago", "Head", "ligera", {"INT": 1, "CAR": 1},
              "Capucha negra con una corona de velas encendidas: las luces de la Santa Compaña."),
        pieza("Pantalón del Necromago", "Legs", "ligera", {"INT": 1, "CAR": 1},
              "Pantalón negro del Necromago."),
        pieza("Botas del Necromago", "Feet", "ligera", {"INT": 1, "CAR": 1},
              "Botas negras raídas."),
    ],
}


# --- E3. Corsario Vampírico ----------------------------------------------------------------------------------------

CORSARIO_VAMPIRICO = {
    "id": "corsario_vampirico", "name": "Corsario Vampírico", "familia": "pirata",
    "elemento": "Sangre", "deidad": None,
    "referencias": ["NosTale: Pirata (Ataque de tormenta, Fuego de cañón)", "NosTale: Cosechador de almas",
                    "NosTale: Renegado", "folclore: el vampiro y el barco fantasma"],
    "description": ("Capitán maldito de un barco de Vinclap, de donde salieron los primeros piratas: muerde y bebe la "
                    "sangre, deja sangrando con su sable, se deshace en niebla y en murciélagos y manda una "
                    "tripulación de ahogados."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "agile",
    "tira": "DES",
    "pasiva": ("Hoja sedienta",
               "El sable del Corsario siempre tiene sed: tus golpes con arma hacen +1 de daño (sangre).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    # El marinero ahogado no tiene ficha: las cifras de un atacante de tier 1 del bestiario (el Salteador novato,
    # lo que fue en vida: CON 2, DES 4, CAR 1; INT 0, como las invocaciones) y los 5 PV de las invocaciones de tier 1.
    "invocaciones": [
        {"id": "marinero_ahogado", "name": "Marinero ahogado", "description": "CON 2, DES 4, INT 0, CAR 1, 5 PV.",
         "health": 5, "duration": "2 turnos", "summonedBy": "corsario_vampirico_tripulacion_de_ahogados",
         "stats": {"CON": 2, "DES": 4, "INT": 0, "CAR": 1},
         "attacks": [
             {"name": "Machete oxidado",
              "effect": "Un tajo con el machete oxidado (la carta Tripulación de ahogados)."},
         ],
         "passive": {"name": "Tripulación maldita",
                     "description": ("Un marinero ahogado del barco del Corsario, chorreando agua de mar: ataca al "
                                     "cerrar tu turno y vuelve al fondo a los 2 turnos.")}},
    ],
    "cartas": [
        carta("folclore: el vampiro", "Mordisco", "single_enemy",
              "Le hincas los colmillos al enemigo: 2d6 de daño de sangre, y te curas el daño que haces.",
              [("2d6 de daño de sangre.", dano("2d6")), ("Te curas el daño que haces.", cura("dano_causado"))],
              elemento="Sangre"),
        carta("propia", "Sable sangriento", "zone_radius",
              "Tu sable maldito abre una herida que no se cierra: 2d6 de daño de sangre, y sangra 2 turnos.",
              [("2d6 de daño de sangre.", dano("2d6")), ("Sangra 2 turnos.", cond("sangrado", 2))],
              elemento="Sangre"),
        # «Bandada de murciélagos» es carta del Guerrero demoníaco: otro nombre.
        carta("folclore: el vampiro", "Revuelo de murciélagos", "all_enemies",
              "Te rodea un revuelo de murciélagos que muerde a todos: 1d6 de daño de tinieblas a cada enemigo, y "
              "atacan con desventaja 1 turno.",
              [("1d6 de daño de tinieblas a cada enemigo.", dano("1d6")),
               ("Murciélagos en la cara: atacan con desventaja 1 turno.", estado(1, "desventaja:ataque"))],
              elemento="Tinieblas"),
        carta("folclore: el vampiro", "Forma de niebla", "self",
              "Te deshaces en niebla cuando llega el golpe: lo bloqueas, y durante 2 turnos te defiendes con ventaja.",
              [BLOQUEA, ("Forma de niebla: te defiendes con ventaja 2 turnos.", propio(2, "ventaja:defensa"))],
              accion="reaccion"),
        # Encanto, control sin daño y de magia: salvación de defensa mental (§4).
        carta("folclore: el vampiro", "Mirada hipnótica", "single_enemy",
              "Le clavas los ojos rojos. Si falla la salvación (defensa mental), queda hipnotizado 1 turno sin atacar.",
              [("Hipnotizado: 1 turno sin atacar.", estado(1, "incapacitado"))], salvacion="defensa_mental"),
        # El Ataque de tormenta de la SP Pirata sin su hemorragia: la sangre ya la pone el Sable sangriento, y así
        # queda un golpe seco a uno (3d6, §3).
        carta("NosTale: Pirata, Ataque de tormenta", "Tajo de galerna", "single_enemy",
              "Caes sobre el enemigo como la galerna sobre un barco: 3d6 de daño de viento.",
              [("3d6 de daño de viento.", dano("3d6"))], elemento="Viento"),
        # La carta que trae al marinero («invocaciones»): uno, 2 turnos (la invocación secundaria de la clase).
        carta("folclore: el barco fantasma", "Tripulación de ahogados", "self",
              "Silbas, y un marinero ahogado de tu tripulación sale chorreando de la niebla: lucha a tu lado 2 turnos.",
              [("Un marinero ahogado lucha a tu lado 2 turnos.", invoca(2, "marinero_ahogado"))]),
    ],
    "creador": [
        carta("folclore: el vampiro", "Banquete de sangre", "all_enemies",
              "Te lanzas sobre todos con el hambre de la noche: 3d6 de daño de sangre a cada enemigo, y te curas el "
              "daño que haces.",
              [("3d6 de daño de sangre a cada enemigo.", dano("3d6")),
               ("Te curas el daño que haces.", cura("dano_causado"))], elemento="Sangre"),
        # El Fuego de cañón de la SP Pirata (el Shock de explosión, sin defensa) es el del Forajido: el barco fantasma
        # rompe la guardia (-2 a la CA) para que los dos cañones de Vinclap no sean la misma carta.
        carta("NosTale: Pirata, Fuego de cañón", "El barco fantasma", "all_enemies",
              "Tu barco fantasma sale de la niebla y abre fuego: 4d6 de daño de tinieblas a cada enemigo, y la "
              "metralla les rompe la guardia 2 turnos (-2 a la CA).",
              [("4d6 de daño de tinieblas a cada enemigo.", dano("4d6")),
               ("Guardia rota 2 turnos: -2 a la CA.", cond("guardia_rota", 2))], elemento="Tinieblas"),
        carta("folclore: el vampiro", "Señor de la noche", "self",
              "La noche te reconoce como su señor: hasta el final del combate atacas con ventaja y tus golpes con arma "
              "hacen +1d6.",
              [("Señor de la noche: +1d6 a tus golpes con arma hasta el final del combate.",
                propio("combate", "bono_dano_arma:1d6")),
               ("Señor de la noche: atacas con ventaja hasta el final del combate.",
                propio("combate", "ventaja:ataque"))]),
    ],
    # Pega (el sable que hace sangrar), su mecánica (beber la sangre) y su defensa (la niebla). La tripulación, después.
    "arranque": ["Sable sangriento", "Mordisco", "Forma de niebla"],
    "arma_inicial": "sable_ligero_de_viaje",
    "set_de": "Corsario Vampírico",
    "set": [
        pieza("Sable del Corsario Vampírico", "MainHand", "ligera", {"DES": 1, "CON": 1},
              "Sable de hoja negra con la guarda de plata.",
              restricciones=["Precisión: DES"]),
        pieza("Casaca del Corsario Vampírico", "Torso", "ligera", {"DES": 1},
              "Casaca larga granate oscuro de cuello alto, como la capa de un vampiro, con botones de plata y el forro "
              "rojo."),
        pieza("Tricornio del Corsario Vampírico", "Head", "ligera", {"DES": 1, "CON": 1},
              "Tricornio negro con una calavera de plata y dos alas de murciélago."),
        pieza("Calzones del Corsario Vampírico", "Legs", "ligera", {"DES": 1, "CON": 1},
              "Calzones negros del Corsario Vampírico."),
        pieza("Botas del Corsario Vampírico", "Feet", "ligera", {"DES": 1, "CON": 1},
              "Botas altas negras con la vuelta granate."),
    ],
}


# --- E4. Forajido de Vinclap ---------------------------------------------------------------------------------------

# El arma normal de tier 1: el sable y la pistola son una sola pieza de arma, una en cada mano (como los guanteletes
# del Artista Marcial). La pistola no lleva pólvora: carga sal de fuego y balas rúnicas (Oriol, 07/10).
SABLE_Y_PISTOLA = arma_comun("Sable y Pistola de Viaje", "media",
                             "Un sable en una mano y una pistola de sal de fuego en la otra.",
                             restricciones=["Precisión: DES", DOS_MANOS])

FORAJIDO_DE_VINCLAP = {
    "id": "forajido_de_vinclap", "name": "Forajido de Vinclap", "familia": "pirata",
    # Sin elemento: las balas de sal de fuego son Fuego en las cartas en que queman.
    "elemento": None, "deidad": None,
    "referencias": ["NosTale: Pirata (Disparar un mosquete, Esconder en barril de ron, Fuego de cañón)",
                    "NosTale: Almirante Yi", "NosTale: Mago de las tinieblas (la pistola)"],
    "description": ("Pirata de Vinclap, de donde salieron los primeros piratas: pelea con el sable en una mano y la "
                    "pistola de sal de fuego en la otra, a base de trucos sucios, abordajes, el barril de ron y el "
                    "cañón del barco."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "balanced",
    "tira": "DES",
    "pasiva": ("Sable y pistola",
               "El sable en una mano y la pistola en la otra: tus golpes con arma hacen +1 de daño.",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        # La Herida de bala del mosquete (10 s): la vida que pierde por la herida, sangra; la velocidad y el daño de
        # los críticos, fuera.
        carta("NosTale: Pirata, Disparar un mosquete", "Disparo a quemarropa", "single_enemy",
              "Le disparas a bocajarro una bala rúnica: 2d6 de daño, y la herida de bala le hace sangrar 2 turnos.",
              [("2d6 de daño.", dano("2d6")), ("Herida de bala: sangra 2 turnos.", cond("sangrado", 2))]),
        # Dos golpes con el arma (como las Ráfagas del Arsenal del Señor de las Armas, que son tres): se reparten
        # entre los enemigos en pie empezando por el elegido.
        carta("propia", "Estocada y disparo", "single_enemy",
              "Una estocada con el sable y, sin esperar, un disparo con la pistola: atacas dos veces con tu arma, al "
              "enemigo que eliges y al siguiente (si está solo, las dos a él).",
              [("Atacas dos veces con tu arma.", "ataques_arma:2")]),
        # Un truco sucio, no magia: sin salvación, entra con la tirada (§4). 2 turnos, no 1 (ver arriba): con 1, el
        # «todo contra él va con ventaja» se acabaría antes de tu golpe siguiente.
        carta("propia", "Arena en los ojos", "single_enemy",
              "Le tiras un puñado de arena a la cara: queda cegado 2 turnos (ataca con desventaja y todo contra él va "
              "con ventaja).",
              [("Cegado 2 turnos.", cond("cegado", 2))]),
        # De las dos ideas del brief, el golpe que derriba: reforzar al grupo ya lo hace, más grande, ¡Todos a cubierta!
        carta("propia", "¡Al abordaje!", "zone_radius",
              "Saltas sobre el enemigo al grito de ¡al abordaje!: 2d6 de daño, y lo tiras al suelo: caído 2 turnos "
              "(ataca con desventaja y los golpes cuerpo a cuerpo contra él van con ventaja).",
              [("2d6 de daño.", dano("2d6")), ("Caído 2 turnos.", cond("caido", 2))]),
        # Esconder en barril de ron (100 %, 2 s): no consumir HP es el bloqueo, y no recibir efectos negativos es, al
        # saltar, librarte de los que llevas.
        carta("NosTale: Pirata, Esconder en barril de ron", "Barril de ron", "self",
              "Te metes en un barril de ron cuando llega el golpe: lo bloqueas, y te libras de los estados en contra.",
              [BLOQUEA, ("Te libras de los estados en contra.", "limpia")], accion="reaccion"),
        # «Tus golpes queman»: el +1 es de fuego. Que dejen ardiendo al que golpeas (un estado en cada golpe) no tiene
        # gancho: fuera.
        carta("propia", "Sal de fuego", "self",
              "Recargas la pistola con sal de fuego: durante 3 turnos tus golpes con arma queman y hacen +1 de daño de "
              "fuego.",
              [("Sal de fuego: +1 a tus golpes con arma durante 3 turnos.", propio(3, "bono_dano_arma:1"))],
              elemento="Fuego"),
        # Miedo, control sin daño: salvación de defensa mental (§4). 1 turno: el miedo de verdad es del Necromago.
        carta("propia", "La bandera negra", "all_enemies",
              "Izas la bandera negra. Quien falle la salvación (defensa mental) queda asustado 1 turno (ataca con "
              "desventaja).",
              [("La bandera negra: asustado 1 turno.", cond("asustado", 1))], salvacion="defensa_mental"),
    ],
    "creador": [
        # El Fuego de cañón de la SP Pirata («Fuego de cañón» es su nombre en NosTale; «Andanada», carta del
        # Cazanieblas): el Shock de explosión, «no hay defensa posible» (7 s): sin defensa 2 turnos.
        carta("NosTale: Pirata, Fuego de cañón", "¡Fuego por babor!", "all_enemies",
              "Tu barco dispara una andanada de sal de fuego: 4d6 de daño de fuego a cada enemigo, y la explosión los "
              "deja sin defensa: se defienden con desventaja 2 turnos.",
              [("4d6 de daño de fuego a cada enemigo.", dano("4d6")),
               ("Shock de la explosión: se defienden con desventaja 2 turnos.", estado(2, "desventaja:defensa"))],
              elemento="Fuego"),
        # Los golpes críticos, 2 turnos: el de la carta no se aprovecha (el estado entra después), los siguientes sí.
        carta("propia", "Duelo al amanecer", "single_enemy",
              "Le retas a un duelo al amanecer, a sable y pistola: 4d6 de daño, y durante 2 turnos todos los golpes "
              "que le aciertan son críticos.",
              [("4d6 de daño.", dano("4d6")),
               ("Duelo al amanecer: los golpes contra él son críticos 2 turnos.", estado(2, "critico_contra"))]),
        carta("propia", "¡Todos a cubierta!", "all_allies",
              "Llamas a toda la tripulación: tú y tus aliados hacéis +1d6 con los golpes de arma hasta el final del "
              "combate.",
              [("¡Todos a cubierta!: +1d6 a los golpes con arma hasta el final del combate.",
                estado("combate", "bono_dano_arma:1d6"))]),
    ],
    # Pega (el disparo), su mecánica (sable y pistola: dos golpes) y su defensa (el barril).
    "arranque": ["Disparo a quemarropa", "Estocada y disparo", "Barril de ron"],
    "arma_inicial": "sable_y_pistola_de_viaje",
    "armas_nuevas": [SABLE_Y_PISTOLA],
    "set_de": "Forajido",
    "set": [
        pieza("Sable y Pistola del Forajido", "MainHand", "media", {"DES": 1, "CON": 1},
              "Un sable curvo de cazoleta de latón en una mano y, en la otra, una pistola de sal de fuego con runas "
              "grabadas en el cañón.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Chaleco del Forajido", "Torso", "media", {"DES": 1},
              "Camisa blanca abierta con un chaleco de cuero marrón, fajín rojo y una bandolera con balas rúnicas."),
        pieza("Pañuelo del Forajido", "Head", "media", {"DES": 1, "CON": 1},
              "Pañuelo rojo anudado a la cabeza, con un aro de oro en la oreja."),
        pieza("Pantalón del Forajido", "Legs", "media", {"DES": 1, "CON": 1},
              "Pantalón a rayas rojas y blancas."),
        pieza("Botas del Forajido", "Feet", "media", {"DES": 1, "CON": 1},
              "Botas de cuero marrón."),
    ],
}


NOCHE_Y_TORMENTA = [TEMPESTARIO, NECROMAGO_DE_LAS_ANIMAS, CORSARIO_VAMPIRICO, FORAJIDO_DE_VINCLAP]
