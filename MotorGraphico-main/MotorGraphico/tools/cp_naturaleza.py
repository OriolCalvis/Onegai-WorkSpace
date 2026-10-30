"""CP-01 — Naturaleza y espíritus: cuatro clases propias (los datos).

Oriol (07/10/2026): «1 druida, domador de bestias, un par de chamanes tipo los del wow». Son clases propias
(planificacion/CANON_CLASES_PROPIAS.md): la misma forma que las SP (tools/sp_*.py), pero las cifras no salen de
una ficha de NosTale, se escriben tal cual dentro del presupuesto (§3) con las piezas de cp_reglas.py. Lo escribe
en los catalogos tools/importar_clases_sp.py.

- Druida del Roble (druida): el del circulo de piedra; la pasiva es el arbol al que se encomienda.
- Domador de Bestias (domador): su bestia se queda todo el combate y sus cartas son ordenes, como el Lobo solar
  del Cazasoles; la pasiva elige que bestia es.
- Chaman de los Elementos (chaman): los totems plantados, las curas en ola y en cadena, los choques y la lava; la
  pasiva es el elemento que llama.
- Chaman de los Espiritus (chaman): primera linea con el arma imbuida, los lobos espirituales, el maleficio y la
  sed de sangre; la pasiva es el imbuido.

Criterios comunes a las cuatro:
- Las invocaciones llevan las cifras de una criatura del bestiario (monsters.json) en CON, DES, INT y CAR (INT 0,
  como las del Cazasoles y el Sacerdote vudu) y los 5 PV de las de tier 1; la del Creador del Druida, las de una
  de tier 2 y sus 8 PV. Atacan al cerrar cada turno tuyo, y solo la bestia del Domador dura todo el combate. Los
  enemigos no las atacan (van a por el heroe o sus acompanantes): ninguna carta promete que protejan a nadie.
- Las ordenes a la bestia son cartas de dano con el texto de la bestia, como las del Cazasoles: se juegan aunque
  la bestia no este en pie (la pasiva lo deja dicho).
- Los chamanes no viven del rayo (es del Tempestario): lo usan como uno mas de sus elementos (Luz).
- Las curas y las mejoras no llevan elemento: lo lleva el golpe (§4 del canon de las SP).
- Las pasivas de elegir no repiten juego de rasgos entre las cuatro: el arbol del Druida (CA, arma, curas), el
  elemento del Chaman de los Elementos (esos tres y la defensa), la bestia del Domador (CA, ataque, arma) y el
  imbuido del Chaman de los Espiritus (arma, ataque, defensa).
"""
from cp_reglas import (DOS_MANOS, arma_comun, aturdido, carta, cond, cura, dano, elige_una, estado, invoca,
                       pendiente, pieza, propio, si)

# El arma normal del Chaman de los Elementos: el catalogo no tiene totem de tier 1.
TOTEM = arma_comun("Tótem de Viaje", "media", "Un bastón grueso con un tótem tallado arriba.",
                   restricciones=["Precisión: CAR", DOS_MANOS])


DRUIDA_DEL_ROBLE = {
    "id": "druida_del_roble", "name": "Druida del Roble", "familia": "druida",
    "elemento": "Naturaleza", "deidad": "deity_wild_court",
    "referencias": ["NosTale: Druida flamígero (Raíz flamígera, Llama sanadora; la figura en la pasiva)",
                    "NosTale: Guardabosques (los espíritus del bosque)",
                    "folclore: los druidas del círculo de piedra y el roble sagrado"],
    "description": ("Druida del círculo de piedra al servicio de la Corte Salvaje: ata con raíces, hace sangrar con "
                    "espinas, cubre con corteza y cura con la lluvia, y al final despierta al bosque."),
    "primaryStat": "INT", "secondaryStat": "CON", "role": "support",
    "tira": "INT",
    # El árbol al que se encomienda es la forma de la clase (§5 del canon), como las figuras del Druida flamígero.
    "pasiva": ("Árbol tutelar",
               "Al empezar el combate te encomiendas a un árbol. Roble: su corteza te cubre: +1 a la CA. Espino: sus "
               "púas pasan a tu arma: tus golpes con arma hacen +1 de daño. Sauce: su savia te corre por las manos: "
               "tus curas curan +1.",
               [("Te encomiendas a un árbol al empezar el combate.", elige_una("roble", "espino", "sauce")),
                ("Roble: +1 a la CA.", si("roble", "bono_ca:1")),
                ("Espino: tus golpes con arma hacen +1 de daño.", si("espino", "bono_dano_arma:1")),
                ("Sauce: tus curas curan +1.", si("sauce", "bono_cura:1"))]),
    # El roble que despierta la de Creador: las cifras de una criatura de tier 2 del bestiario (Centinela de Corteza
    # Veterano: CON 4, DES 3, CAR 3; INT 0, como las invocaciones) y sus 8 PV: es el árbol guardián del encargo. Los
    # enemigos no atacan a las invocaciones (van a por el héroe o sus acompañantes): lo que pesa son sus golpes.
    "invocaciones": [
        {"id": "roble_despierto", "name": "Roble despierto", "description": "CON 4, DES 3, INT 0, CAR 3, 8 PV.",
         "health": 8, "duration": "3 turnos", "summonedBy": "druida_del_roble_despertar_del_roble",
         "stats": {"CON": 4, "DES": 3, "INT": 0, "CAR": 3},
         "attacks": [
             {"name": "Golpe de rama",
              "effect": "Barre al enemigo con sus ramas al cerrar tu turno (la carta Despertar del roble)."},
         ],
         "passive": {"name": "Roble del círculo",
                     "description": ("Un roble viejo que se arranca del suelo: lucha a tu lado 3 turnos, golpea al "
                                     "cerrar cada turno tuyo y vuelve a echar raíces al acabar.")}},
    ],
    "cartas": [
        # Control sin daño a uno, del cuerpo: resistencia física (§4 del canon). Las raíces que atan son el
        # aturdimiento de las SP: no ataca y se defiende con desventaja.
        carta("NosTale: Druida flamígero, Raíz flamígera", "Raíces enredadoras", "single_enemy",
              "Unas raíces salen del suelo y atan al enemigo. Si no se resiste (resistencia física), queda 1 turno "
              "sin atacar y se defiende con desventaja.",
              aturdido(1, "Raíces"), salvacion="resistencia_fisica", elemento="Naturaleza"),
        carta("propia", "Lluvia de espinas", "all_enemies",
              "Una lluvia de espinas cae sobre los enemigos: 1d6 de daño a cada uno, y sangran 2 turnos.",
              [("1d6 de daño a cada enemigo.", dano("1d6")),
               ("Espinas clavadas: sangran 2 turnos.", cond("sangrado", 2))], elemento="Naturaleza"),
        # La «piel de corteza» del encargo: otro nombre, que el Devoto del Bosque ya tiene una Piel de Corteza.
        carta("NosTale: Druida flamígero, Piel de oso pardo", "Corteza de roble", "single_ally",
              "La corteza del roble cubre a un aliado (o a ti): +2 a la CA durante 3 turnos.",
              [("Corteza de roble: +2 a la CA durante 3 turnos.", estado(3, "bono_ca:2"))], elemento="Naturaleza"),
        carta("propia", "Lluvia que sana", "all_allies",
              "Llamas a una lluvia mansa que sana: tú y tus aliados os curáis 1d6.",
              [("Os curáis 1d6.", cura("1d6"))], elemento="Naturaleza"),
        # El enjambre: las avispas envenenan (envenenado: ataca con desventaja, §3 del canon).
        carta("NosTale: Sacerdote vudú, Enjambre de langostas voraces", "Avispero", "zone_radius",
              "Revientas un avispero sobre el enemigo: 2d6 de daño, y las picaduras lo dejan envenenado 2 turnos "
              "(ataca con desventaja).",
              [("2d6 de daño.", dano("2d6")),
               ("Picaduras: envenenado 2 turnos.", cond("envenenado", 2))], elemento="Naturaleza"),
        # Reacción: bloquea el golpe y deja una condición al atacante (§3), como el Escudo divino del catálogo (ciega).
        carta("propia", "Zarzal", "self",
              "Un zarzal brota a tu alrededor en el último momento: bloqueas el golpe, y el atacante se engancha en "
              "las zarzas y sangra 2 turnos.",
              [("Bloqueas el golpe.", "bloquea:ataque"),
               ("Zarzas: el atacante sangra 2 turnos.", cond("sangrado", 2))], accion="reaccion", elemento="Naturaleza"),
        # Primero limpia y después cura (§4 del canon de las SP).
        carta("NosTale: Druida flamígero, Llama sanadora", "Savia viva", "single_ally",
              "La savia del bosque corre por las venas de un aliado (o por las tuyas): se libra de los estados en "
              "contra y se cura 2d6.",
              [("Se libra de los estados en contra.", "limpia"), ("Se cura 2d6.", cura("2d6"))], elemento="Naturaleza"),
    ],
    "creador": [
        # La invocación («invocaciones»): 3 rondas, un roble de 8 PV. Solo, el roble golpearía como el Tótem
        # abrasador (una estándar): al despertar cae sobre el enemigo, 3d6 (un golpe a uno de Creador es 4d6 y un
        # efecto: aquí, menos dados y el roble como efecto).
        carta("folclore: los árboles que caminan", "Despertar del roble", "single_enemy",
              "Despiertas a un roble viejo, que se arranca del suelo y cae sobre el enemigo: 3d6 de daño. Después "
              "lucha a tu lado 3 turnos y golpea al cerrar cada turno tuyo.",
              [("3d6 de daño.", dano("3d6")),
               ("El roble despierto lucha a tu lado 3 turnos.", invoca(3, "roble_despierto"))], elemento="Naturaleza"),
        # El encargo decía «y caídos»; la Estampida del Domador y el Seísmo del Chamán ya tiran al suelo: aquí las
        # raíces los atan, 1 turno sin atacar (un efecto, como la Hiedra del Guardabosques).
        carta("propia", "Ira de la tierra", "all_enemies",
              "La tierra se revuelve y sus raíces atrapan a los enemigos: 4d6 de daño a cada uno, y quedan 1 turno "
              "sin atacar.",
              [("4d6 de daño a cada enemigo.", dano("4d6")),
               ("Atrapados por las raíces: 1 turno sin atacar.", estado(1, "incapacitado"))], elemento="Naturaleza"),
        # El encargo decía «limpia + cura:2d6 a todos», que es el Tótem de la marea del Chamán de los Elementos; el
        # Druida ya limpia con la Savia viva: aquí, la cura y una corteza nueva para el grupo.
        carta("propia", "El bosque renace", "all_allies",
              "El bosque renace alrededor del grupo: tú y tus aliados os curáis 2d6, y una corteza nueva os cubre: +1 "
              "a la CA durante 3 turnos.",
              [("Os curáis 2d6.", cura("2d6")),
               ("Corteza nueva: +1 a la CA durante 3 turnos.", estado(3, "bono_ca:1"))], elemento="Naturaleza"),
    ],
    # Una que pega, la que controla (las raíces son la clase) y una que cura (§6).
    "arranque": ["Lluvia de espinas", "Raíces enredadoras", "Lluvia que sana"],
    "arma_inicial": "vara_de_aprendiz_de_viaje",
    "set_de": "Druida",
    # support: media (el arma, como el Báculo de Volcano)
    "set": [
        pieza("Báculo del Druida", "MainHand", "media", {"INT": 1, "CON": 1},
              "Báculo de roble retorcido, con una rama de muérdago y tres bellotas en la punta.",
              restricciones=["Precisión: INT"]),
        pieza("Túnica del Druida", "Torso", "media", {"INT": 1},
              "Túnica verde musgo con un manto de hojas sobre los hombros y un cinturón de corteza."),
        pieza("Corona del Druida", "Head", "media", {"INT": 1, "CON": 1},
              "Corona de hojas de roble con bellotas y dos astas pequeñas de ciervo."),
        pieza("Pantalón del Druida", "Legs", "media", {"INT": 1, "CON": 1},
              "Pantalón pardo envuelto con tiras de cuero."),
        pieza("Sandalias del Druida", "Feet", "media", {"INT": 1, "CON": 1},
              "Sandalias de cuero de corteza."),
    ],
}

DOMADOR_DE_BESTIAS = {
    "id": "domador_de_bestias", "name": "Domador de Bestias", "familia": "domador",
    "elemento": None, "deidad": None,
    "referencias": ["NosTale: Entrenador de mascotas (Latigazo, Domesticación, los monstruos de entrenamiento)",
                    "NosTale: Cazasoles (el Lobo solar y sus órdenes)"],
    "description": ("Lucha con el látigo en la mano y su bestia al lado: la llama al empezar el combate, le ordena "
                    "morder, derribar y rugir, y se pone delante de los suyos."),
    "primaryStat": "DES", "secondaryStat": "CAR", "role": "balanced",
    "tira": "DES",
    # Qué bestia es, la forma de la clase (§5 del canon). Lo que da, te lo da a ti: la bestia tiene siempre las
    # mismas cifras (una sola invocación). Las órdenes se juegan aunque la bestia no esté en pie, como las del
    # Cazasoles: queda dicho.
    "pasiva": ("Bestia de confianza",
               "Al empezar el combate eliges qué bestia te acompaña. Oso: su corpachón te cubre: +1 a la CA. Lobo: "
               "cazáis en pareja: atacas con ventaja. Jabalí: su embestida se te contagia: tus golpes con arma hacen "
               "+1 de daño.",
               [("Eliges tu bestia al empezar el combate.", elige_una("oso", "lobo", "jabali")),
                ("Oso: +1 a la CA.", si("oso", "bono_ca:1")),
                ("Lobo: atacas con ventaja.", si("lobo", "ventaja:ataque")),
                ("Jabalí: tus golpes con arma hacen +1 de daño.", si("jabali", "bono_dano_arma:1")),
                pendiente("que las órdenes a la bestia solo salgan con la bestia en pie")]),
    # La bestia la trae Llamar a la bestia y se queda hasta el final del combate (cp_reglas.invoca: solo el compañero
    # del Domador). Las cifras de un animal de tier 1 del bestiario, las del Lobo solar (Lobo de Cripta: CON 2, DES
    # 4, CAR 1; INT 0, como las invocaciones) y los 5 PV de las invocaciones de tier 1.
    "invocaciones": [
        {"id": "bestia_domada", "name": "Bestia domada", "description": "CON 2, DES 4, INT 0, CAR 1, 5 PV.",
         "health": 5, "duration": "hasta el final del combate", "summonedBy": "domador_de_bestias_llamar_a_la_bestia",
         "stats": {"CON": 2, "DES": 4, "INT": 0, "CAR": 1},
         # sus ataques son las órdenes del Domador: el daño lo ponen esas cartas
         "attacks": [
             {"name": "¡Muerde!", "effect": "Hinca los dientes en el enemigo y lo deja sangrando (la carta ¡Muerde!)."},
             {"name": "¡Derriba!", "effect": "Embiste al enemigo y lo tira al suelo (la carta ¡Derriba!)."},
             {"name": "¡Ruge!", "effect": "Un rugido que asusta a los enemigos (la carta ¡Ruge!)."},
             {"name": "Estampida",
              "effect": "Llama a su manada y arrolla a los enemigos (la carta Estampida de la manada)."},
             {"name": "Salto del rey",
              "effect": "Cae sobre el enemigo a la vez que tú y lo aturde (la carta El rey de la manada)."},
         ],
         "passive": {"name": "Órdenes del Domador",
                     "description": ("El oso, el lobo o el jabalí que eliges al empezar el combate: lucha a tu lado "
                                     "hasta el final del combate, ataca en cada ronda y cumple las órdenes de tus "
                                     "cartas.")}},
    ],
    "cartas": [
        # La carta que trae la bestia (como Curación natural trae al Lobo solar). Solo la llama: es la clase.
        carta("NosTale: Entrenador de mascotas, Invocar monstruo de entrenamiento", "Llamar a la bestia", "self",
              "Silbas y tu bestia sale de la espesura: lucha a tu lado hasta el final del combate y ataca al cerrar "
              "cada turno tuyo.",
              [("Tu bestia lucha a tu lado hasta el final del combate.", invoca("combate", "bestia_domada"))]),
        carta("NosTale: Cazasoles, Orden de ataque fugaz", "¡Muerde!", "single_enemy",
              "A tu orden, la bestia hinca los dientes en el enemigo: 2d6 de daño, y sangra 2 turnos.",
              [("2d6 de daño.", dano("2d6")), ("Dentellada: sangra 2 turnos.", cond("sangrado", 2))]),
        carta("NosTale: Cazasoles, Patada animal", "¡Derriba!", "single_enemy",
              "A tu orden, la bestia embiste al enemigo y lo tira al suelo: 2d6 de daño, y queda caído 1 turno "
              "(ataca con desventaja y los golpes cuerpo a cuerpo contra él van con ventaja).",
              [("2d6 de daño.", dano("2d6")), ("Derribado: caído 1 turno.", cond("caido", 1))]),
        # Control sin daño a todos; miedo: defensa mental (como la Intimidación del Guerrero).
        carta("NosTale: Druida flamígero, Rugido del Loa oso", "¡Ruge!", "all_enemies",
              "A tu orden, la bestia suelta un rugido que hiela la sangre. Quien no se resista (defensa mental) queda "
              "asustado 1 turno (ataca con desventaja).",
              [("Rugido: asustados 1 turno.", cond("asustado", 1))], salvacion="defensa_mental"),
        # El encargo decía «con desventaja al defenderse», que es el Choque de escarcha del Chamán de los Elementos:
        # el látigo deja su marca (marcado: tus golpes le hacen +1).
        carta("NosTale: Entrenador de mascotas, Latigazo", "Latigazo", "single_enemy",
              "Un chasquido del látigo le deja la marca en la piel: 2d6 de daño, y queda marcado 2 turnos (tus golpes "
              "le hacen +1).",
              [("2d6 de daño.", dano("2d6")), ("Marca del látigo: marcado 2 turnos.", cond("marcado", 2))]),
        # Te pones tú delante (defiende_aliado), como el Sacrificio del Monje de lucha. Salta cuando atacan a un
        # acompañante: a la bestia no la atacan (los enemigos no van a por las invocaciones).
        carta("NosTale: Monje de lucha, Sacrificio", "¡Guarda!", "single_ally",
              "Das la orden de guardia y cubres a un aliado: la próxima vez que le ataquen, te pones delante y recibes "
              "tú el golpe.",
              [("Te pones delante del aliado.", "defiende_aliado:1")], accion="reaccion"),
        # Control sin daño a uno; encanto: defensa mental. Lo de NosTale (domar de verdad a la mascota) no es del
        # combate.
        carta("NosTale: Entrenador de mascotas, Domesticación", "Domesticación", "single_enemy",
              "Le hablas bajo, sin apartar la mirada, y le enseñas el látigo. Si no se resiste (defensa mental), se "
              "amansa y queda 1 turno sin atacar.",
              [("Amansado: 1 turno sin atacar.", estado(1, "incapacitado"))], salvacion="defensa_mental"),
    ],
    "creador": [
        # «Estampida» a secas ya es una carta del Monje del Despertar.
        carta("propia", "Estampida de la manada", "all_enemies",
              "Tu bestia llama a su manada y una estampida pasa por encima de los enemigos: 4d6 de daño a cada uno, y "
              "caen al suelo: caídos 1 turno.",
              [("4d6 de daño a cada enemigo.", dano("4d6")), ("Pisoteados: caídos 1 turno.", cond("caido", 1))]),
        # La bestia es un aliado más: también la alcanza.
        carta("propia", "El vínculo salvaje", "all_allies",
              "Tu bestia y tú cazáis como uno y el grupo se contagia: hasta el final del combate, tú y tus aliados (la "
              "bestia también) atacáis con ventaja y vuestros golpes con arma hacen +1.",
              [("Vínculo salvaje: ventaja en ataque todo el combate.", estado("combate", "ventaja:ataque")),
               ("Vínculo salvaje: +1 a los golpes con arma todo el combate.", estado("combate", "bono_dano_arma:1"))]),
        # Golpe a uno de Creador: 4d6 y un efecto, el aturdimiento (como la Erupción solar del Cazasoles).
        carta("NosTale: Cazasoles, Erupción solar", "El rey de la manada", "single_enemy",
              "Tu bestia y tú caéis a la vez sobre el enemigo: 4d6 de daño y aturdimiento 1 turno (no ataca y se "
              "defiende con desventaja).",
              [("4d6 de daño.", dano("4d6"))] + aturdido(1)),
    ],
    # La que trae la bestia, una orden que pega y una que defiende (§6): el rugido, que sirve en el primer combate;
    # ¡Guarda! solo salta con acompañantes.
    "arranque": ["Llamar a la bestia", "¡Muerde!", "¡Ruge!"],
    "arma_inicial": "latigo_de_cuero_de_viaje",
    "set_de": "Domador",
    # balanced: armadura media; el látigo, ligero como el del catálogo
    "set": [
        pieza("Látigo del Domador", "MainHand", "ligera", {"DES": 1, "CAR": 1},
              "Látigo largo de cuero trenzado, con el mango forrado de piel y un colmillo en el pomo.",
              restricciones=["Precisión: DES"]),
        pieza("Chaleco del Domador", "Torso", "media", {"DES": 1},
              "Chaleco de cuero con una hombrera de piel de oso y un collar de colmillos."),
        pieza("Cabeza de Oso del Domador", "Head", "media", {"DES": 1, "CAR": 1},
              "La piel de la cabeza de un oso pardo puesta como gorro: las orejas, los ojos y el hocico sobre la "
              "frente, con los colmillos asomando."),
        pieza("Pantalón del Domador", "Legs", "media", {"DES": 1, "CAR": 1},
              "Pantalón de cuero marrón del Domador."),
        pieza("Botas del Domador", "Feet", "media", {"DES": 1, "CAR": 1},
              "Botas de cuero con la vuelta de piel."),
    ],
}

CHAMAN_DE_LOS_ELEMENTOS = {
    "id": "chaman_de_los_elementos", "name": "Chamán de los Elementos", "familia": "chaman",
    "elemento": None, "deidad": None,
    "referencias": ["WoW: el chamán (los tótems, la ola de sanación, la sanación en cadena, los choques, el estallido "
                    "de lava)",
                    "NosTale: Sacerdote vudú (los tótems y los espíritus)"],
    "description": ("Chamán de los tótems: los planta en el suelo para proteger y guiar al grupo, cura con la ola y la "
                    "sanación en cadena y castiga con los choques de tierra y de escarcha y con la lava."),
    "primaryStat": "CAR", "secondaryStat": "CON", "role": "support",
    "tira": "CAR",
    # El elemento que llama al empezar es la forma de la clase (§5 del canon).
    "pasiva": ("Llamada de los elementos",
               "Al empezar el combate llamas a un elemento. Tierra: te sostiene: +1 a la CA. Fuego: arde en tu arma: "
               "tus golpes con arma hacen +1 de daño. Agua: te corre por las manos: tus curas curan +1. Aire: te "
               "aparta de los golpes: te defiendes con ventaja.",
               [("Llamas a un elemento al empezar el combate.", elige_una("tierra", "fuego", "agua", "aire")),
                ("Tierra: +1 a la CA.", si("tierra", "bono_ca:1")),
                ("Fuego: tus golpes con arma hacen +1 de daño.", si("fuego", "bono_dano_arma:1")),
                ("Agua: tus curas curan +1.", si("agua", "bono_cura:1")),
                ("Aire: te defiendes con ventaja.", si("aire", "ventaja:defensa"))]),
    # El tótem que ataca: las cifras de una criatura de tier 1 del bestiario que dispara (el artefacto Dron
    # Chispeante: CON 1, DES 3, CAR 1; INT 0, como las invocaciones) y los 5 PV de las invocaciones de tier 1.
    "invocaciones": [
        {"id": "totem_abrasador", "name": "Tótem abrasador", "description": "CON 1, DES 3, INT 0, CAR 1, 5 PV.",
         "health": 5, "duration": "3 turnos", "summonedBy": "chaman_de_los_elementos_totem_abrasador",
         "stats": {"CON": 1, "DES": 3, "INT": 0, "CAR": 1},
         "attacks": [
             {"name": "Llamarada",
              "effect": "Escupe fuego contra el enemigo al cerrar tu turno (la carta Tótem abrasador)."},
         ],
         "passive": {"name": "Tótem de fuego",
                     "description": ("Un tótem de madera tallada con brasas en los ojos: plantado en el suelo, escupe "
                                     "fuego al enemigo al cerrar cada turno tuyo durante 3 turnos y se apaga al "
                                     "acabar.")}},
    ],
    "cartas": [
        # Los tótems que solo apoyan son cartas de grupo (el texto los planta): una mejora de 2-3 turnos (§3).
        carta("WoW: Tótem Piel de piedra", "Tótem de piel de piedra", "all_allies",
              "Plantas en el suelo un tótem de piedra que endurece la piel del grupo: tú y tus aliados tenéis +1 a la "
              "CA durante 3 turnos.",
              [("Piel de piedra: +1 a la CA durante 3 turnos.", estado(3, "bono_ca:1"))], elemento="Tierra"),
        # La invocación («invocaciones»): 3 rondas, ataca al cerrar cada turno tuyo.
        carta("WoW: Tótem abrasador", "Tótem abrasador", "self",
              "Plantas un tótem de fuego que escupe llamas: durante 3 turnos ataca al enemigo al cerrar cada turno "
              "tuyo.",
              [("El tótem abrasador ataca 3 turnos.", invoca(3, "totem_abrasador"))], elemento="Fuego"),
        carta("WoW: Ola de sanación", "Ola de sanación", "single_ally",
              "Una ola de agua limpia cubre a un aliado (o a ti) y lo sana: se cura 2d6.",
              [("Se cura 2d6.", cura("2d6"))], elemento="Agua"),
        carta("WoW: Sanación en cadena", "Sanación en cadena", "all_allies",
              "La sanación salta de uno a otro por todo el grupo: tú y tus aliados os curáis 1d6.",
              [("Os curáis 1d6.", cura("1d6"))], elemento="Agua"),
        # Golpe que deja sin atacar: 1d6 (§3). El choque corta el ataque, como el de WoW corta el hechizo.
        carta("WoW: Choque de tierra", "Choque de tierra", "single_enemy",
              "Una sacudida de tierra golpea al enemigo y le corta el ataque: 1d6 de daño de tierra, y queda 1 turno sin "
              "atacar.",
              [("1d6 de daño de tierra.", dano("1d6")), ("Sacudida: 1 turno sin atacar.", estado(1, "incapacitado"))], elemento="Tierra"),
        carta("WoW: Choque de escarcha", "Choque de escarcha", "single_enemy",
              "Una descarga de escarcha congela al enemigo: 2d6 de daño de hielo, y entumecido se defiende con "
              "desventaja 2 turnos.",
              [("2d6 de daño de hielo.", dano("2d6")),
               ("Escarcha: se defiende con desventaja 2 turnos.", estado(2, "desventaja:defensa"))], elemento="Hielo"),
        carta("WoW: Tótem Viento Furioso", "Tótem del viento", "all_allies",
              "Plantas un tótem de viento que guía los golpes del grupo: tú y tus aliados atacáis con ventaja 2 "
              "turnos.",
              [("Tótem del viento: ventaja en ataque 2 turnos.", estado(2, "ventaja:ataque"))], elemento="Viento"),
    ],
    "creador": [
        # Primero limpia y después cura (§4 del canon de las SP).
        carta("WoW: Tótem de marea de sanación", "Tótem de la marea", "all_allies",
              "Plantas el tótem de la marea y el agua sube y baja por el grupo: tú y tus aliados os libráis de los "
              "estados en contra y os curáis 2d6.",
              [("Os libráis de los estados en contra.", "limpia"), ("Os curáis 2d6.", cura("2d6"))], elemento="Agua"),
        carta("WoW: Estallido de lava", "Estallido de lava", "single_enemy",
              "Lanzas un estallido de lava contra el enemigo: 4d6 de daño de fuego, y arde: 1 de daño por turno "
              "durante 3 turnos.",
              [("4d6 de daño de fuego.", dano("4d6")),
               ("Lava: 1 de daño por turno durante 3 turnos.", estado(3, "dano_por_turno:1"))], elemento="Fuego"),
        # El «terremoto» del encargo: Terremoto ya es una carta del Guerrero y de Volcano. Menos daño que la Estampida
        # del Domador (3d6) y más tiempo en el suelo (2 turnos): las réplicas no los dejan levantarse.
        carta("WoW: Terremoto", "Seísmo", "all_enemies",
              "Clavas el tótem en el suelo y la tierra tiembla bajo los enemigos: 3d6 de daño de tierra a cada uno, y las "
              "réplicas los tiran al suelo: caídos 2 turnos.",
              [("3d6 de daño de tierra a cada enemigo.", dano("3d6")), ("Seísmo: caídos 2 turnos.", cond("caido", 2))], elemento="Tierra"),
    ],
    # Un choque que pega, el tótem que ataca (la mecánica) y la cura (§6).
    "arranque": ["Choque de escarcha", "Tótem abrasador", "Ola de sanación"],
    "arma_inicial": "totem_de_viaje",
    "armas_nuevas": [TOTEM],
    "set_de": "Chamán de los Elementos",
    # support: media; el tótem, a dos manos como el de Viaje
    "set": [
        pieza("Tótem del Chamán de los Elementos", "MainHand", "media", {"CAR": 1, "CON": 1},
              "Bastón grueso con un tótem tallado arriba, pintado de rojo y ocre, con las cuatro caras de los "
              "elementos: la piedra, la llama, la ola y el viento.",
              restricciones=["Precisión: CAR", DOS_MANOS]),
        pieza("Chaleco del Chamán de los Elementos", "Torso", "media", {"CAR": 1},
              "Chaleco de cuero y piel con pinturas rojas y ocres de tótem y un collar de huesos."),
        # sin plumas de jefe (el encargo)
        pieza("Tocado del Chamán de los Elementos", "Head", "media", {"CAR": 1, "CON": 1},
              "Tocado de cuernos de carnero con cuentas y huesos."),
        pieza("Pantalón del Chamán de los Elementos", "Legs", "media", {"CAR": 1, "CON": 1},
              "Pantalón de piel con flecos."),
        pieza("Botas del Chamán de los Elementos", "Feet", "media", {"CAR": 1, "CON": 1},
              "Botas de piel parda con cuentas ocres."),
    ],
}

CHAMAN_DE_LOS_ESPIRITUS = {
    "id": "chaman_de_los_espiritus", "name": "Chamán de los Espíritus", "familia": "chaman",
    "elemento": None, "deidad": None,
    "referencias": ["WoW: el chamán de mejora (las armas imbuidas, el golpe de tormenta, los lobos espirituales, el "
                    "maleficio, la sed de sangre)",
                    "NosTale: Sacerdote vudú (los espíritus y las maldiciones)"],
    "description": ("Chamán de primera línea: imbuye su hacha con los elementos, golpea con la tormenta y la lava, "
                    "convierte en sapo a quien estorba y llama a los lobos espirituales de sus antepasados."),
    "primaryStat": "DES", "secondaryStat": "CAR", "role": "balanced",
    # Los golpes del arma imbuida tiran DES; las de espíritu (el lobo, el maleficio, la sed de sangre y la tormenta
    # de los antepasados), CAR en la carta (§4 del canon).
    "tira": "DES",
    # El imbuido es la forma de la clase (§5 del canon). La escarcha del encargo daba +1 a la CA, que es el oso del
    # Domador (su pasiva tendría los mismos tres rasgos): aquí frena los golpes del enemigo, te defiendes con
    # ventaja. Las opciones van con espacios, como «sin elemento» del Ángel vengador: la fila dice «Elegir Lengua
    # de fuego».
    "pasiva": ("Arma imbuida",
               "Al empezar el combate imbuyes tu arma. Lengua de fuego: tu arma arde: tus golpes con arma hacen +1 de "
               "daño (fuego). Viento furioso: el viento guía tu brazo: atacas con ventaja. Escarcha: el frío de tu "
               "arma frena los golpes del enemigo: te defiendes con ventaja.",
               [("Imbuyes tu arma al empezar el combate.", elige_una("lengua de fuego", "viento furioso", "escarcha")),
                ("Lengua de fuego: tus golpes con arma hacen +1 de daño.", si("lengua de fuego", "bono_dano_arma:1")),
                ("Viento furioso: atacas con ventaja.", si("viento furioso", "ventaja:ataque")),
                ("Escarcha: te defiendes con ventaja.", si("escarcha", "ventaja:defensa"))]),
    # Los lobos de WoW son dos; aquí uno (una invocación por carta), con las cifras de un lobo de tier 1 del
    # bestiario distinto del de la bestia del Domador (Lobo de Hiedra Roja: CON 1, DES 4, CAR 1; INT 0, como las
    # invocaciones) y los 5 PV de las invocaciones de tier 1.
    "invocaciones": [
        {"id": "lobo_espiritual", "name": "Lobo espiritual", "description": "CON 1, DES 4, INT 0, CAR 1, 5 PV.",
         "health": 5, "duration": "2 turnos", "summonedBy": "chaman_de_los_espiritus_lobo_espiritual",
         "stats": {"CON": 1, "DES": 4, "INT": 0, "CAR": 1},
         "attacks": [
             {"name": "Dentellada de los antepasados",
              "effect": "Muerde al enemigo al cerrar tu turno (la carta Lobo espiritual)."},
         ],
         "passive": {"name": "Lobo de los antepasados",
                     "description": ("Un lobo gris de humo y luz turquesa: lucha a tu lado 2 turnos, muerde al "
                                     "cerrar cada turno tuyo y se deshace al acabar.")}},
    ],
    "cartas": [
        # El encargo decía 3d6 de luz, que es el Rayo del Tempestario: aquí, como el de WoW, la tormenta se queda en
        # el enemigo y tus golpes le hacen más (un efecto: 2d6).
        carta("WoW: Golpe de tormenta", "Golpe de tormenta", "single_enemy",
              "Golpeas con el arma cargada de tormenta: 2d6 de daño de rayo, y la carga se queda en el enemigo: tus "
              "golpes le hacen +1 durante 2 turnos.",
              [("2d6 de daño de rayo.", dano("2d6")),
               ("Carga de la tormenta: tus golpes le hacen +1 durante 2 turnos.", estado(2, "bono_dano_contra:1"))],
              elemento="Rayo"),
        # El latigazo de lava del encargo, con el hacha. Sin quemadura, que ya la pone el Choque de llamas: el golpe
        # grande a uno (3d6, §3).
        carta("WoW: Latigazo de lava", "Hachazo de lava", "single_enemy",
              "Tu hacha se cubre de lava y la descargas sobre el enemigo: 3d6 de daño de fuego.",
              [("3d6 de daño de fuego.", dano("3d6"))], elemento="Fuego"),
        # El viento furioso como carta (la pasiva tiene el suyo): dos golpes con el arma, a uno o a dos enemigos;
        # suman lo que lleve el arma (la lengua de fuego, el Ascenso elemental, la Sed de sangre).
        carta("WoW: Arma Viento Furioso", "Ráfaga del vendaval", "single_enemy",
              "El viento furioso te mueve el brazo: das dos golpes seguidos con el arma, el primero al enemigo que "
              "eliges y el segundo al siguiente (los dos a él, si está solo).",
              [("Dos golpes con el arma.", "ataques_arma:2")], elemento="Viento"),
        # La invocación («invocaciones»): 2 rondas.
        carta("WoW: Espíritu feral", "Lobo espiritual", "self",
              "Llamas a un lobo espiritual de tus antepasados: lucha a tu lado 2 turnos y muerde al enemigo al cerrar "
              "cada turno tuyo.",
              [("El lobo espiritual lucha a tu lado 2 turnos.", invoca(2, "lobo_espiritual"))], tira="CAR"),
        # Control sin daño a uno; maldición: defensa mental. El sapo es el aturdimiento de las SP.
        carta("WoW: Maleficio", "Maleficio", "single_enemy",
              "Con un maleficio conviertes al enemigo en sapo. Si no se resiste (defensa mental), queda 1 turno sin "
              "atacar y se defiende con desventaja.",
              aturdido(1, "Sapo"), salvacion="defensa_mental", tira="CAR"),
        carta("WoW: Choque de llamas", "Choque de llamas", "zone_radius",
              "Una descarga de llamas sale de tu hacha y prende al enemigo: 2d6 de daño de fuego, y arde: 1 de daño "
              "por turno durante 2 turnos.",
              [("2d6 de daño de fuego.", dano("2d6")),
               ("Llamas: 1 de daño por turno durante 2 turnos.", estado(2, "dano_por_turno:1"))], elemento="Fuego"),
        # Reacción: bloquea el golpe y lo que gana dura 2 rondas (§4 del canon de las SP). Devolver los golpes sería
        # el Escudo de electrones del Tronador: aquí la descarga pasa al arma.
        carta("WoW: Escudo de relámpagos", "Escudo de relámpagos", "self",
              "Te rodean relámpagos en el último momento: bloqueas el golpe, y su descarga pasa a tu arma: +1 a tus "
              "golpes con arma durante 2 turnos.",
              [("Bloqueas el golpe.", "bloquea:ataque"),
               ("Descarga: +1 a tus golpes con arma durante 2 turnos.", propio(2, "bono_dano_arma:1"))],
              accion="reaccion", elemento="Rayo"),
    ],
    "creador": [
        # La de WoW dura poco y pega fuerte: 3 turnos, ventaja y +1d6 (el grupo +1d6 al arma es de Creador, §3). Así
        # no repite El vínculo salvaje del Domador (ventaja y +1 todo el combate).
        carta("WoW: Ansia de sangre", "Sed de sangre", "all_allies",
              "Tu grito despierta la sed de sangre del grupo: durante 3 turnos, tú y tus aliados atacáis con ventaja y "
              "vuestros golpes con arma hacen +1d6.",
              [("Sed de sangre: ventaja en ataque durante 3 turnos.", estado(3, "ventaja:ataque")),
               ("Sed de sangre: +1d6 a los golpes con arma durante 3 turnos.", estado(3, "bono_dano_arma:1d6"))],
              tira="CAR", elemento="Sangre"),
        carta("WoW: Ascensión", "Ascenso elemental", "self",
              "Te fundes con los elementos hasta el final del combate: tus golpes con arma hacen +1d6 y tienes +1 a "
              "la CA.",
              [("Ascenso elemental: +1d6 a tus golpes con arma todo el combate.",
                propio("combate", "bono_dano_arma:1d6")),
               ("Ascenso elemental: +1 a la CA todo el combate.", propio("combate", "bono_ca:1"))]),
        # Los espíritus bajan con la tormenta: de rayo (PART-01; antes, sin elemento).
        carta("propia", "Tormenta de los antepasados", "all_enemies",
              "Los espíritus de tus antepasados bajan con la tormenta y caen sobre los enemigos: 4d6 de daño de rayo a cada "
              "uno.",
              [("4d6 de daño de rayo a cada enemigo.", dano("4d6"))], tira="CAR", elemento="Rayo"),
    ],
    # Un golpe del arma, el lobo (la mecánica) y la reacción que defiende (§6).
    "arranque": ["Golpe de tormenta", "Lobo espiritual", "Escudo de relámpagos"],
    "arma_inicial": "hacha_de_una_mano_de_viaje",
    "set_de": "Chamán de los Espíritus",
    # balanced: armadura media; el hacha, de una mano y de peso medio como la de Viaje (sus golpes tiran DES)
    "set": [
        pieza("Hacha del Chamán de los Espíritus", "MainHand", "media", {"DES": 1, "CAR": 1},
              "Hacha de una mano con la hoja grabada de runas turquesa y el mango envuelto en cuero con cuentas.",
              restricciones=["Precisión: DES"]),
        pieza("Cota del Chamán de los Espíritus", "Torso", "media", {"DES": 1},
              "Cota de malla con hombreras de hueso y runas turquesa pintadas."),
        pieza("Banda del Chamán de los Espíritus", "Head", "media", {"DES": 1, "CAR": 1},
              "Banda de cuero en la frente con un amuleto turquesa del espíritu y dos trenzas con cuentas a los "
              "lados."),
        pieza("Faldar del Chamán de los Espíritus", "Legs", "media", {"DES": 1, "CAR": 1},
              "Faldar de malla sobre pantalón de piel."),
        pieza("Botas del Chamán de los Espíritus", "Feet", "media", {"DES": 1, "CAR": 1},
              "Botas de piel gris con tiras de cuero cruzadas."),
    ],
}

NATURALEZA = [DRUIDA_DEL_ROBLE, DOMADOR_DE_BESTIAS, CHAMAN_DE_LOS_ELEMENTOS, CHAMAN_DE_LOS_ESPIRITUS]
