"""CP-01 — Los cuatro bardos nuevos, clases propias de MotorGraphico (los datos).

Oriol (07/10/2026): «3 clases más de bardo (en plan un actor de teatro, un cantador, un músico
instrumental, y un feriante)». No salen de una ficha de NosTale: los dados y las rondas se escriben tal
cual, dentro del presupuesto de planificacion/CANON_CLASES_PROPIAS.md (§3), y todo lo demás (estados,
reacciones, lo que no se simula) es lo de CANON_SP_CLASES.md. Lo escribe en los catálogos
tools/importar_clases_sp.py, como las SP.

Ya hay tres bardos en el catálogo (el de la Cuerda Rota, cánticos de brujo; el de la Voz Divina,
cánticos sagrados; el de las Cuatro Estaciones, canciones de estación). Estos cuatro no los repiten y
tampoco se repiten entre ellos; cada uno tiene su manera de ayudar al grupo:
- El actor (Dos Máscaras) engaña y controla: deja al enemigo embobado o abucheado, finge su muerte,
  cambia de vestuario a un aliado, y de Creador pide un bis, baja el telón y cierra con un final feliz.
- El cantador (Cantar de Gesta) inspira y cura: el cantar sube el arma del grupo, su voz cura +1
  (la pasiva), el grito de guerra asusta, y de Creador canta la epopeya y el último cantar (revive).
- El músico (Tambor de Guerra) marca el paso en primera línea: el redoble y la marcha dan ventaja al
  grupo, el bombo tumba, la baqueta atonta, y se pone delante de los suyos.
- El feriante (Feria) pega y entretiene: cuchillos, tragafuegos, malabares, el trile y el pregón que
  atrae a todos; de Creador, el gran espectáculo, el hombre bala y la noria.

Criterios comunes a los cuatro:
- Tiran con CAR (canon §4); el Feriante, con DES sus números de habilidad y con CAR el pregón y el
  trile (tira= en la carta). Lo que hace un golpe con arma (ataques_arma, el aliado del Bis) tira DES,
  como Atacar.
- Control sin daño: con salvación si es de la cabeza (defensa_mental: el monólogo, el abucheo, el
  grito, el pregón, el trile); sin salvación si es del cuerpo y no es magia (el telón, los fuegos
  artificiales: tirada enfrentada, como un golpe sin daño).
- Un estado de 1 turno puesto en tu turno se acaba al cerrar esa ronda: vale para estorbar el ataque
  del enemigo de esa ronda (sin atacar, sin paso, caído, deslumbrado) y para los aliados que actúan
  detrás de ti. Lo que es para que tú le pegues mejor (se defiende con desventaja, cegado por el telón,
  los golpes contra él son críticos) dura 2, y llega a tu turno siguiente. Lo que deja sin atacar, 1
  (canon §3).
- Las reacciones solo ponen al atacante una condición del catálogo (las de estado no saltan) y lo que
  gana quien reacciona dura 2 turnos (propio:2).
"""
from cp_reglas import arma_comun, carta, cond, cura, dano, elige_una, estado, pieza, propio, si, siempre

# ACTUAR-01: el laúd y el tambor son instrumentos (suman al actuar en la calle: include/Game/Actuacion.h)
INSTRUMENTO = "Instrumento"

# Las armas normales de tier 1 (el catálogo no tenía estoque, laúd, tambor ni bastón de feriante).
ESTOQUE_DE_ATREZO = arma_comun("Estoque de Atrezo de Viaje", "ligera",
                               "Un estoque de teatro de hoja fina y cazoleta de latón: de atrezo, pero con el filo de "
                               "verdad.", restricciones=["Precisión: DES"])
LAUD = arma_comun("Laúd de Viaje", "ligera",
                  "Un laúd sencillo de caja redonda y madera clara: suena bien y, si hace falta, golpea.",
                  restricciones=["Precisión: CAR"], etiquetas=["Arma Ligera", INSTRUMENTO])
# Una mano: el tambor cuelga de la correa y se toca (y se pega) con la baqueta.
TAMBOR_DE_GUERRA = arma_comun("Tambor de Guerra de Viaje", "media",
                              "Un tambor de guerra de madera y parche de cuero que cuelga de la correa; se toca con la "
                              "baqueta en una mano.", restricciones=["Precisión: CAR"],
                              etiquetas=["Arma Media", INSTRUMENTO])
BASTON_DE_FERIANTE = arma_comun("Bastón de Feriante de Viaje", "ligera",
                                "Un bastón corto de paseo, negro, con el puño dorado.",
                                restricciones=["Precisión: DES"])


BARDO_DE_LAS_DOS_MASCARAS = {
    "id": "bardo_de_las_dos_mascaras", "name": "Bardo de las Dos Máscaras", "familia": "bardo",
    "elemento": None, "deidad": None,
    "referencias": ["NosTale: Atuendo de boda (lo social)", "NosTale: Monje de lucha (la forma en la pasiva)",
                    "NosTale: Espía (reacciones)"],
    "description": ("Actor de una compañía ambulante que pelea con la palabra, el gesto y un estoque de atrezo. Se "
                    "pone la máscara de la tragedia o la de la comedia, embelesa con sus monólogos, muere en escena "
                    "para levantarse por sorpresa y pone al público de su parte."),
    "primaryStat": "CAR", "secondaryStat": "DES", "role": "support",
    "tira": "CAR",
    # Las dos máscaras son la forma de la clase (§5), como las meditaciones del Monje de lucha.
    "pasiva": ("Las dos máscaras",
               "Al empezar el combate eliges máscara. Tragedia: arrastras al enemigo hacia su final y atacas con "
               "ventaja. Comedia: te mueves como un bufón y te defiendes con ventaja.",
               [("Eliges una máscara al empezar el combate.", elige_una("tragedia", "comedia")),
                ("Máscara de la tragedia: atacas con ventaja.", si("tragedia", "ventaja:ataque")),
                ("Máscara de la comedia: te defiendes con ventaja.", si("comedia", "ventaja:defensa"))]),
    "cartas": [
        carta("teatro: la esgrima de escena", "Estocada de atrezo", "single_enemy",
              "Una estocada de escena, toda floritura, con un estoque que corta más de lo que parece: 3d6 de daño.",
              [("3d6 de daño.", dano("3d6"))]),
        carta("teatro: el monólogo", "Monólogo", "single_enemy",
              "Le recitas a un enemigo un monólogo tan bien dicho que se queda embobado escuchándote. Si no se "
              "resiste (defensa mental), pasa 1 turno sin atacar.",
              [("Embobado: 1 turno sin atacar.", estado(1, "incapacitado"))], salvacion="defensa_mental"),
        carta("teatro: la muerte fingida", "Morir en escena", "self",
              "Preparas tu mejor muerte: cuando te ataquen, caes antes de que llegue el golpe y no te toca. Luego "
              "te levantas por sorpresa: atacas con ventaja 2 turnos.",
              [("Bloqueas el próximo ataque.", "bloquea:ataque"),
               ("Resurrección por sorpresa: atacas con ventaja 2 turnos.", propio(2, "ventaja:ataque"))],
              accion="reaccion"),
        # El brief decía «el grupo ataca con ventaja», pero eso es el Redoble del Tambor de Guerra: el
        # aplauso del actor llega en el momento justo, a uno (la reacción de dados_extra, §3).
        carta("NosTale: Atuendo de boda, Lluvia de confeti", "El aplauso del público", "single_ally",
              "Preparas al público para que estalle en aplausos en el momento justo: la próxima tirada de un "
              "aliado, o la tuya, para atacar o para defenderse, suma 2 dados.",
              [("Ovación: +2 dados a su próxima tirada.", "dados_extra:2")], accion="reaccion"),
        carta("teatro: el abucheo", "El abucheo", "single_enemy",
              "Señalas a un enemigo y el público lo abuchea sin piedad. Si no se resiste (defensa mental), "
              "abochornado, ataca con desventaja 2 turnos.",
              [("Abucheado: ataca con desventaja 2 turnos.", estado(2, "desventaja:ataque"))],
              salvacion="defensa_mental"),
        carta("teatro: el soliloquio de la tragedia", "Soliloquio trágico", "all_enemies",
              "Recitas una tragedia tan negra que duele oírla: 2d6 de daño de tinieblas a cada enemigo.",
              [("2d6 de daño de tinieblas a cada enemigo.", dano("2d6"))], elemento="Tinieblas"),
        carta("teatro: el cambio rápido", "Cambio de vestuario", "single_ally",
              "Entre bastidores, le cambias el traje a un aliado, o a ti, en un abrir y cerrar de ojos: se libra "
              "de los estados en contra y, con el disfraz nuevo, se defiende con ventaja 2 turnos.",
              [("Se libra de los estados en contra.", "limpia"),
               ("Disfraz nuevo: se defiende con ventaja 2 turnos.", estado(2, "ventaja:defensa"))]),
    ],
    # Creador: el final de la función. El brief proponía «el gran estreno» (el grupo +1d6 al arma todo el
    # combate); ese es la Epopeya del Cantar de Gesta, y el actor cierra con lo que nadie más tiene: el bis
    # (turno_extra), el telón (control a todos) y el final feliz (limpia y cura:2d6 a todos).
    "creador": [
        # turno_extra a otro aliado: ataca ya con su arma (tira DES); a ti, un turno más.
        carta("teatro: el bis", "Bis", "single_ally",
              "¡Otra, otra! El público pide un bis: un aliado vuelve a escena y ataca ahora mismo; si te eliges a "
              "ti, juegas un turno más.",
              [("Bis: el aliado ataca otra vez ahora mismo; tú, un turno más.", "turno_extra")]),
        # El telón es del cuerpo y no es magia: sin salvación, tirada enfrentada (como un golpe sin daño).
        carta("teatro: el telón", "Telón", "all_enemies",
              "Dejas caer el telón sobre los enemigos: enredados en el terciopelo pasan 1 turno sin atacar, y a "
              "oscuras quedan cegados 2 turnos (atacan con desventaja y todo contra ellos va con ventaja).",
              [("Enredados en el telón: 1 turno sin atacar.", estado(1, "incapacitado")),
               ("A oscuras: cegados 2 turnos.", cond("cegado", 2))]),
        carta("teatro: el final de comedia", "Final feliz", "all_allies",
              "Toda comedia acaba bien: tú y tus aliados os libráis de los estados en contra y os curáis 2d6.",
              [("Os libráis de los estados en contra.", "limpia"), ("Os curáis 2d6.", cura("2d6"))]),
    ],
    # Pega (la estocada), su mecánica (la palabra que embelesa) y defensa (la muerte fingida).
    "arranque": ["Estocada de atrezo", "Monólogo", "Morir en escena"],
    "arma_inicial": "estoque_de_atrezo_de_viaje",
    "armas_nuevas": [ESTOQUE_DE_ATREZO],
    "set_de": "Comediante",
    # support: armadura media
    "set": [
        pieza("Estoque del Comediante", "MainHand", "ligera", {"CAR": 1, "DES": 1},
              "El estoque de la compañía: hoja larga y fina de acero y cazoleta dorada con las dos máscaras "
              "grabadas, la blanca sonriente y la dorada triste.",
              restricciones=["Precisión: DES"]),
        pieza("Jubón del Comediante", "Torso", "media", {"CAR": 1},
              "Jubón partido, mitad negro y mitad blanco, con gorguera blanca."),
        pieza("Sombrero del Comediante", "Head", "media", {"CAR": 1, "DES": 1},
              "Sombrero de ala ancha negro con una pluma blanca y las dos máscaras prendidas en la copa: la blanca "
              "sonriente y la dorada triste."),
        pieza("Calzas del Comediante", "Legs", "media", {"CAR": 1, "DES": 1},
              "Calzas a dos colores: una pierna negra y la otra blanca."),
        pieza("Zapatos del Comediante", "Feet", "media", {"CAR": 1, "DES": 1},
              "Zapatos negros con hebilla dorada."),
    ],
}

BARDO_DEL_CANTAR_DE_GESTA = {
    "id": "bardo_del_cantar_de_gesta", "name": "Bardo del Cantar de Gesta", "familia": "bardo",
    # La voz que inspira: sus golpes son de luz (la copla, el trueno de su voz, el himno).
    "elemento": "Luz", "deidad": None,
    "referencias": ["NosTale: Mago sagrado (bendiciones de grupo)", "NosTale: Pijama (Tocar la pandereta)"],
    "description": ("Juglar que canta las gestas del grupo mientras suceden: su cantar hace héroes a sus aliados, su "
                    "grito de guerra asusta, su copla burlona hiere el orgullo y su voz cura a los que caen."),
    "primaryStat": "CAR", "secondaryStat": "CON", "role": "support",
    "tira": "CAR",
    # Siempre (§5), distinta de las de los otros tres bardos: la cura de sus dos canciones que curan (Balada de
    # los caídos y Nana del herido) sube 1 por cada aliado.
    "pasiva": ("Voz de juglar",
               "Tu voz reconforta tanto como anima: cada cura que haces cura +1.",
               [("Tus curas curan +1.", siempre("bono_cura:1"))]),
    "cartas": [
        carta("juglaría: el cantar de gesta", "Cantar de gesta", "all_allies",
              "Cantas las hazañas del grupo mientras pelea y cada uno quiere estar a la altura del cantar: tú y "
              "tus aliados hacéis +1 con los golpes de arma 3 turnos.",
              [("Cantar de gesta: +1 a los golpes con arma 3 turnos.", estado(3, "bono_dano_arma:1"))]),
        carta("NosTale: Guerrero, Intimidación", "Grito de guerra", "all_enemies",
              "Lanzas un grito de guerra que hiela la sangre. Quien no se resista (defensa mental) queda asustado "
              "2 turnos: ataca con desventaja.",
              [("Miedo: asustado 2 turnos.", cond("asustado", 2))], salvacion="defensa_mental"),
        carta("NosTale: Mago sagrado, Cura de grupo", "Balada de los caídos", "all_allies",
              "Una balada por los que cayeron antes que vosotros os devuelve el aliento: tú y tus aliados os "
              "curáis 1d6.",
              [("Os curáis 1d6.", cura("1d6"))]),
        # El brief proponía una nana que duerme a un enemigo (1 turno sin atacar, defensa mental): eso ya es el
        # Monólogo del actor. La nana del juglar es para los suyos: limpia y cura 2d6 a uno.
        carta("folclore: la nana", "Nana del herido", "single_ally",
              "Le cantas una nana a un aliado herido (o a ti), bajito, como a un niño: se libra de los estados en "
              "contra y se cura 2d6.",
              [("Se libra de los estados en contra.", "limpia"), ("Se cura 2d6.", cura("2d6"))]),
        carta("juglaría: la cantiga de escarnio", "Copla burlona", "single_enemy",
              "Le cantas al enemigo una copla de escarnio que le hiere en el orgullo y en el cuerpo: 3d6 de daño "
              "de luz.",
              [("3d6 de daño de luz.", dano("3d6"))], elemento="Luz"),
        carta("propia", "Voz de trueno", "zone_radius",
              "Tu voz cae sobre el enemigo como un trueno: 2d6 de daño de luz, y, ensordecido, se defiende con "
              "desventaja 2 turnos.",
              [("2d6 de daño de luz.", dano("2d6")),
               ("Ensordecido: se defiende con desventaja 2 turnos.", estado(2, "desventaja:defensa"))],
              elemento="Luz"),
        carta("NosTale: Pijama, Tocar la pandereta", "Estribillo pegadizo", "all_allies",
              "Un estribillo que todos coreáis sin querer os marca el paso y os aparta de los golpes: tú y tus "
              "aliados os defendéis con ventaja 2 turnos.",
              [("Estribillo: os defendéis con ventaja 2 turnos.", estado(2, "ventaja:defensa"))]),
    ],
    "creador": [
        carta("NosTale: Mago sagrado, Arma santa", "Epopeya", "all_allies",
              "Cantas la epopeya entera del grupo y os sentís los héroes que nombra: tú y tus aliados hacéis +1d6 "
              "con los golpes de arma hasta el final del combate.",
              [("Epopeya: +1d6 a los golpes con arma todo el combate.", estado("combate", "bono_dano_arma:1d6"))]),
        # revive:1, como el Último Aliento del Bardo de las Cuatro Estaciones (el aliado que se prepara; tú, si no
        # eliges a nadie). La pasiva no la sube: no es una cura.
        carta("juglaría: el último cantar", "El último cantar", "single_ally",
              "Guardas tu último cantar para un aliado, o para ti: si cae, le cantas su gesta y se levanta con 1 "
              "PV.",
              [("Si cae, se levanta con 1 PV.", "revive:1")], accion="reaccion"),
        carta("NosTale: Mago sagrado, Martillo santo", "Himno de las mil voces", "all_enemies",
              "Todos los que alguna vez oyeron tus gestas cantan contigo, y el himno cae sobre los enemigos: 4d6 de "
              "daño de luz a cada uno, y quedan asustados 1 turno.",
              [("4d6 de daño de luz a cada enemigo.", dano("4d6")), ("Asustados 1 turno.", cond("asustado", 1))],
              elemento="Luz"),
    ],
    # Pega (la copla), su mecánica (el cantar que hace héroes) y apoyo (la balada).
    "arranque": ["Copla burlona", "Cantar de gesta", "Balada de los caídos"],
    "arma_inicial": "laud_de_viaje",
    "armas_nuevas": [LAUD],
    "set_de": "Juglar",
    # support: armadura media
    "set": [
        pieza("Laúd del Juglar", "MainHand", "ligera", {"CAR": 1, "CON": 1},
              "El laúd del juglar: caja de nogal con la roseta tallada, el clavijero vuelto hacia atrás y cintas "
              "rojas y doradas atadas al mástil.",
              etiquetas=["Arma Ligera", INSTRUMENTO], restricciones=["Precisión: CAR"]),
        pieza("Sobrevesta del Juglar", "Torso", "media", {"CAR": 1},
              "Sobrevesta de juglar a rombos rojos y dorados, con la correa del laúd cruzada al pecho."),
        pieza("Birrete del Juglar", "Head", "media", {"CAR": 1, "CON": 1},
              "Birrete rojo con una pluma larga de faisán."),
        pieza("Calzas del Juglar", "Legs", "media", {"CAR": 1, "CON": 1},
              "Calzas rojas."),
        pieza("Botas del Juglar", "Feet", "media", {"CAR": 1, "CON": 1},
              "Botas cortas de cuero marrón, con la caña vuelta."),
    ],
}

BARDO_DEL_TAMBOR_DE_GUERRA = {
    "id": "bardo_del_tambor_de_guerra", "name": "Bardo del Tambor de Guerra", "familia": "bardo",
    "elemento": None, "deidad": None,
    "referencias": ["NosTale: Cruzado (gritos de grupo)", "NosTale: Gladiador"],
    "description": ("Tamborilero que marcha delante de la tropa: su tambor da los toques del combate, el bombo tumba "
                    "al enemigo y lo saca del paso, y a su llamada el grupo forma y se cubre."),
    "primaryStat": "CAR", "secondaryStat": "CON", "role": "balanced",
    "tira": "CAR",
    # Los toques son la forma de la clase (§5): se elige uno al empezar cada combate.
    "pasiva": ("Toques de tambor",
               "Al empezar el combate eliges el toque. Marcha: llevas el paso y atacas con ventaja. Carga: tus "
               "golpes con arma hacen +1 de daño. Retirada: te defiendes con ventaja.",
               [("Eliges un toque al empezar el combate.", elige_una("marcha", "carga", "retirada")),
                ("Toque de marcha: atacas con ventaja.", si("marcha", "ventaja:ataque")),
                ("Toque de carga: tus golpes con arma hacen +1 de daño.", si("carga", "bono_dano_arma:1")),
                ("Toque de retirada: te defiendes con ventaja.", si("retirada", "ventaja:defensa"))]),
    "cartas": [
        carta("música militar: el redoble", "Redoble", "all_allies",
              "Un redoble largo que acelera el pulso de todos: tú y tus aliados atacáis con ventaja 2 turnos.",
              [("Redoble: ventaja en ataque 2 turnos.", estado(2, "ventaja:ataque"))]),
        carta("NosTale: Gladiador, Ataque de escudo", "Golpe de bombo", "zone_radius",
              "Das un golpe de bombo tan hondo que el suelo salta bajo el enemigo: 2d6 de daño, y cae al suelo 1 "
              "turno.",
              [("2d6 de daño.", dano("2d6")), ("Derribo: cae al suelo 1 turno.", cond("caido", 1))]),
        # El brief proponía ataques_arma:2, pero los golpes con arma tiran DES y el tamborilero es CAR/CON (los
        # varios golpes con arma son del Feriante, que tira DES): dos baquetazos que tiran CAR y atontan.
        carta("propia", "Doble baqueta", "single_enemy",
              "Dos baquetazos secos, uno en cada sien: 2d6 de daño, y atontado pasa 1 turno sin atacar.",
              [("2d6 de daño.", dano("2d6")), ("Atontado: 1 turno sin atacar.", estado(1, "incapacitado"))]),
        carta("NosTale: Gladiador, Cuerno de caza", "Retumbar", "all_enemies",
              "Haces retumbar el tambor hasta que tiembla el suelo: 1d6 de daño a cada enemigo, que pierde el paso "
              "y ataca con desventaja 1 turno.",
              [("1d6 de daño a cada enemigo.", dano("1d6")),
               ("Sin paso: ataca con desventaja 1 turno.", estado(1, "desventaja:ataque"))]),
        carta("NosTale: Cruzado, Escudo creciente", "Llamada a formar", "all_allies",
              "Tocas llamada y el grupo forma a tu alrededor: tú y tus aliados os libráis de los estados en contra "
              "y tenéis +1 a la CA 2 turnos.",
              [("Os libráis de los estados en contra.", "limpia"),
               ("En formación: +1 a la CA 2 turnos.", estado(2, "bono_ca:1"))]),
        # Una reacción solo le pone al atacante una condición (§3): «descolocado, con desventaja al atacar» es
        # asustado (desventaja en sus ataques).
        carta("música: el contratiempo", "Contratiempo", "self",
              "Paras el golpe con el aro del tambor y respondes a contratiempo: el próximo ataque que te llegue no "
              "te toca, y el atacante, sobresaltado, queda asustado 2 turnos (ataca con desventaja).",
              [("Bloqueas el próximo ataque.", "bloquea:ataque"),
               ("Sobresaltado: asustado 2 turnos.", cond("asustado", 2))], accion="reaccion"),
        # En vez del «ritmo de guerra» (el grupo +1 al arma 3 turnos, que ya es el Cantar de gesta del juglar): el
        # tamborilero es el bardo de primera línea (balanced) y se pone delante de los suyos.
        carta("NosTale: Monje de lucha, Sacrificio", "Detrás del tambor", "single_ally",
              "¡Detrás del tambor! Cubres a un aliado con el tambor por escudo: el próximo golpe que vaya contra "
              "él lo recibes tú.",
              [("Recibes tú el golpe que iba contra el aliado.", "defiende_aliado:1")], accion="reaccion"),
    ],
    # Creador: el +1d6 al arma de todo el combate ya es la Epopeya del juglar, y el turno_extra y el revive son del
    # Bis del actor y del Último cantar: la marcha triunfal da al grupo ventaja todo el combate (como la Estimulación
    # del Guerrero), y el «pulso del corazón» del brief deja su sitio a la marcha fúnebre, el remate a uno.
    "creador": [
        carta("NosTale: Guerrero, Estimulación", "Marcha triunfal", "all_allies",
              "Tocas la marcha de los que ya han ganado: tú y tus aliados atacáis con ventaja hasta el final del "
              "combate.",
              [("Marcha triunfal: ventaja en ataque todo el combate.", estado("combate", "ventaja:ataque"))]),
        carta("NosTale: Guerrero, Terremoto", "Tambor del juicio", "all_enemies",
              "Tocas el tambor del juicio y retumba el mundo entero: 4d6 de daño a cada enemigo, que queda 1 turno "
              "sin atacar.",
              [("4d6 de daño a cada enemigo.", dano("4d6")),
               ("Juicio: 1 turno sin atacar.", estado(1, "incapacitado"))]),
        carta("música: la marcha fúnebre", "Marcha fúnebre", "single_enemy",
              "Le tocas al enemigo su marcha fúnebre, despacio, y su final se cumple: 4d6 de daño de tinieblas, y "
              "durante 2 turnos los golpes contra él son críticos.",
              [("4d6 de daño de tinieblas.", dano("4d6")),
               ("Su marcha fúnebre: los golpes contra él son críticos 2 turnos.", estado(2, "critico_contra"))],
              elemento="Tinieblas"),
    ],
    # Pega y tumba (el bombo), su mecánica (el redoble que da el toque al grupo) y defensa (el contratiempo).
    "arranque": ["Golpe de bombo", "Redoble", "Contratiempo"],
    "arma_inicial": "tambor_de_guerra_de_viaje",
    "armas_nuevas": [TAMBOR_DE_GUERRA],
    "set_de": "Tamborilero",
    # balanced: armadura media
    "set": [
        pieza("Tambor del Tamborilero", "MainHand", "media", {"CAR": 1, "CON": 1},
              "El tambor del tamborilero: caja azul con los aros rojos, los cordones dorados en zigzag y dos "
              "baquetas de madera clara.",
              etiquetas=["Arma Media", INSTRUMENTO], restricciones=["Precisión: CAR"]),
        pieza("Casaca del Tamborilero", "Torso", "media", {"CAR": 1},
              "Casaca militar azul con correajes blancos cruzados y botones dorados."),
        pieza("Chacó del Tamborilero", "Head", "media", {"CAR": 1, "CON": 1},
              "Chacó alto azul con penacho rojo y cordones dorados."),
        pieza("Pantalón del Tamborilero", "Legs", "media", {"CAR": 1, "CON": 1},
              "Pantalón blanco con una franja roja."),
        pieza("Botas del Tamborilero", "Feet", "media", {"CAR": 1, "CON": 1},
              "Botas altas negras."),
    ],
}

BARDO_DE_LA_FERIA = {
    "id": "bardo_de_la_feria", "name": "Bardo de la Feria", "familia": "bardo",
    "elemento": None, "deidad": None,
    "referencias": ["NosTale: Jajamaru (fuego y bombas)", "NosTale: Pijama"],
    "description": ("Feriante de la feria ambulante: lanza cuchillos, escupe fuego, engaña con el trile y atrae todas "
                    "las miradas con su pregón, y salta como un acróbata para que no le alcancen."),
    "primaryStat": "DES", "secondaryStat": "CAR", "role": "agile",
    # Sus números de habilidad (cuchillos, malabares, tragafuegos) tiran DES; el pregón y el trile, CAR (en la carta).
    "tira": "DES",
    "pasiva": ("El acróbata",
               "Saltas, ruedas y te descuelgas como en la pista de la feria: te defiendes con ventaja.",
               [("Te defiendes con ventaja.", siempre("ventaja:defensa"))]),
    "cartas": [
        # Tres golpes con el arma (tiran DES, como Atacar), repartidos desde el elegido: como las Ráfagas del
        # Arsenal del Señor de las Armas, tier 1.
        carta("NosTale: Jajamaru, Ataque de Shuriken", "Lanzar cuchillos", "single_enemy",
              "Lanzas tres cuchillos, uno detrás de otro: tres ataques con el arma, a uno o a varios enemigos, "
              "empezando por el que elijas.",
              [("Tres ataques con el arma.", "ataques_arma:3")]),
        carta("NosTale: Jajamaru, Fuego", "Tragafuegos", "zone_radius",
              "Das un buche de licor de fuego y escupes una llamarada: 2d6 de daño de fuego, y arde: 1 de daño por "
              "turno durante 2 turnos.",
              [("2d6 de daño de fuego.", dano("2d6")),
               ("Arde: 1 de daño por turno durante 2 turnos.", estado(2, "dano_por_turno:1"))], elemento="Fuego"),
        # La pasiva ya le hace defenderse con ventaja: los malabares le afinan el pulso para atacar.
        carta("feria: los malabares", "Malabares", "self",
              "Haces volar las mazas sin que se te caiga ninguna y afinas el pulso: atacas con ventaja 3 turnos.",
              [("Pulso de malabarista: ventaja en ataque 3 turnos.", estado(3, "ventaja:ataque"))]),
        carta("feria: el pregón", "¡Pasen y vean!", "all_enemies",
              "Pregonas tu número a voz en grito y nadie puede apartar la vista de ti. Quien no se resista (defensa "
              "mental) te ataca a ti 1 turno, y tú, en guardia, tienes +1 a la CA 2 turnos.",
              [("Te atacan a ti 1 turno.", "estado:1:provocado:origen"),
               ("En guardia: +1 a tu CA 2 turnos.", propio(2, "bono_ca:1"))],
              salvacion="defensa_mental", tira="CAR"),
        carta("feria: el trile", "El trile", "single_enemy",
              "Tres cubiletes, una bolita y unas manos más rápidas que la vista. Si el enemigo no se resiste "
              "(defensa mental), anda tan liado buscando la bolita que se defiende con desventaja 2 turnos.",
              [("Liado con la bolita: se defiende con desventaja 2 turnos.", estado(2, "desventaja:defensa"))],
              salvacion="defensa_mental", tira="CAR"),
        # El brief decía +1d6 a tus golpes 2 turnos; una mejora propia estándar es +1 (§3), y todo el combate «solo
        # un +1»: ese.
        carta("feria: el forzudo", "El forzudo", "self",
              "Doblas una barra de hierro delante de todos y ya no hay quien te pare: tus golpes con arma hacen +1 "
              "hasta el final del combate.",
              [("Forzudo: +1 a tus golpes con arma todo el combate.", estado("combate", "bono_dano_arma:1"))]),
        # Deslumbra el cuerpo y no es magia: sin salvación, tirada enfrentada (DES).
        carta("NosTale: Jajamaru, Caída de bombas", "Fuegos artificiales", "all_enemies",
              "Prendes una traca de fuegos artificiales delante de los enemigos: deslumbrados, quedan cegados 1 "
              "turno (atacan con desventaja y todo contra ellos va con ventaja).",
              [("Deslumbrados: cegados 1 turno.", cond("cegado", 1))]),
    ],
    "creador": [
        carta("feria: el número final", "El gran espectáculo", "all_enemies",
              "El número final: cohetes, ruedas de chispas y fuego sobre los enemigos: 4d6 de daño de fuego a cada "
              "uno, y arden: 1 de daño por turno durante 2 turnos.",
              [("4d6 de daño de fuego a cada enemigo.", dano("4d6")),
               ("Arden: 1 de daño por turno durante 2 turnos.", estado(2, "dano_por_turno:1"))], elemento="Fuego"),
        # Sin pólvora (Oriol, 07/10): el cañón de la feria es de muelles.
        carta("feria: el hombre bala", "El hombre bala", "single_enemy",
              "Te metes en el cañón de muelles de la feria y sales disparado contra el enemigo: 4d6 de daño, y cae "
              "al suelo 2 turnos.",
              [("4d6 de daño.", dano("4d6")), ("Derribo: cae al suelo 2 turnos.", cond("caido", 2))]),
        carta("feria: la noria", "La noria", "all_allies",
              "Plantas la noria de la feria en mitad del combate y sus cestas giran entre el grupo y los enemigos, "
              "parando los golpes: tú y tus aliados tenéis +2 a la CA hasta el final del combate.",
              [("La noria: +2 a la CA todo el combate.", estado("combate", "bono_ca:2"))]),
    ],
    # Pega (los cuchillos), su número de fuego (el tragafuegos) y defensa (el pregón: los atrae y aguanta).
    "arranque": ["Lanzar cuchillos", "Tragafuegos", "¡Pasen y vean!"],
    "arma_inicial": "baston_de_feriante_de_viaje",
    "armas_nuevas": [BASTON_DE_FERIANTE],
    "set_de": "Feriante",
    # agile: armadura ligera
    "set": [
        pieza("Bastón del Feriante", "MainHand", "ligera", {"DES": 1, "CAR": 1},
              "El bastón del feriante: de paseo, negro y lustroso, con el puño y la contera dorados.",
              restricciones=["Precisión: DES"]),
        pieza("Frac del Feriante", "Torso", "ligera", {"DES": 1},
              "Frac rojo con botones dorados, solapas negras y faldones."),
        pieza("Chistera del Feriante", "Head", "ligera", {"DES": 1, "CAR": 1},
              "Chistera roja con una cinta dorada."),
        pieza("Pantalón del Feriante", "Legs", "ligera", {"DES": 1, "CAR": 1},
              "Pantalón a rayas negras y blancas."),
        pieza("Botas del Feriante", "Feet", "ligera", {"DES": 1, "CAR": 1},
              "Botas negras lustrosas."),
    ],
}

BARDOS = [BARDO_DE_LAS_DOS_MASCARAS, BARDO_DEL_CANTAR_DE_GESTA, BARDO_DEL_TAMBOR_DE_GUERRA, BARDO_DE_LA_FERIA]
