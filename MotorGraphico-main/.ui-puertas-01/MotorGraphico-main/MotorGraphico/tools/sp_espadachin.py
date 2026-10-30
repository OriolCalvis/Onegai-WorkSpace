"""SP-02 — Las doce SP del Espadachin de NosTale, como clases de MotorGraphico (los datos).

Cada carta dice de que habilidad sale (su H en la enciclopedia de SP de Oriol) y sus cifras
pasan por las reglas de sp_reglas.py (planificacion/CANON_SP_CLASES.md): los dados por la
bonificacion de la ficha (cuerpo a cuerpo + elemental), las rondas por los segundos, los
estados con un 50 % o mas. Cuando una carta se aparta de la regla, lo dice un comentario.
Lo escribe en los catalogos tools/importar_clases_sp.py.
"""
from sp_reglas import (CONTRARRESTA, DOS_MANOS, arma_comun, carta, condicion, dano, elige_una, estado, pieza,
                       propio, si, siempre)
from sp_reglas import atrae as _atrae
from sp_reglas import aturdido as _aturdido
from sp_reglas import pendiente as _pendiente


GUERRERO = {
    "id": "guerrero", "name": "Guerrero", "sp": 1, "elemento": "Fuego", "nostale": "Guerrero (Warrior)",
    "description": ("Especialista del espadón de fuego. Reúne a los enemigos con Provocación, aguanta con "
                    "Piel de acero y los castiga en área con Torbellino y Terremoto."),
    "primaryStat": "CON", "secondaryStat": "DES", "role": "tank",
    "pasiva": ("Espadón a dos manos",
               "La cadena de tres tajos del Guerrero: tus golpes con arma hacen +1 de daño (fuego).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H812", "Batalla Triple", "zone_radius",
              "Encadenas tres tajos de espadón: 2d6 de daño de fuego.",
              [("2d6 de daño de fuego.", dano(700, 400))], elemento="Fuego"),
        carta("H813", "Piel de acero", "self",
              "Tu piel se endurece como el acero: +1 a la CA durante 3 turnos, y con la Ira de hierro "
              "tus golpes con arma hacen +1 durante 5 turnos.",
              [("+1 a la CA durante 3 turnos.", estado(45, "bono_ca:1")),
               ("Ira de hierro: +1 al daño de tus golpes con arma durante 5 turnos.",
                estado(60, "bono_dano_arma:1"))]),
        carta("H814", "Torbellino", "all_enemies",
              "Giras con el espadón y alcanzas a todos los enemigos: 1d6 de daño de fuego.",
              # el 40 % de aturdimiento no llega al 50 % (§4): fuera
              [("1d6 de daño de fuego a cada enemigo.", dano(250, 200))], elemento="Fuego"),
        # Oriol (07/10): de arranque, «un provocar molt bàsic i fàcil d'esquivar. Al pujar de tier ja
        # ho anirem millorant». Un enemigo y con salvación; lo de NosTale (a todos 2 turnos, la
        # rigidez y la Voluntad del defensor) queda para cuando mejore con el tier.
        carta("H815", "Provocación", "single_enemy",
              "Retas a un enemigo. Si no se resiste (defensa mental), te ataca a ti 1 turno. Es una "
              "provocación básica, fácil de esquivar: mejora al subir de tier.",
              [("Te ataca a ti 1 turno.", "estado:1:provocado:origen")],
              salvacion="defensa_mental"),
        carta("H816", "Golpe último", "zone_radius",
              "Atraviesas con el espadón a los que tienes en fila: 2d6 de daño de fuego y -1 a su CA "
              "durante 3 turnos.",
              [("2d6 de daño de fuego.", dano(650, 350)),
               ("Debilita su defensa: -1 a la CA durante 3 turnos.", estado(30, "penal_ca:1"))],
              elemento="Fuego"),
        carta("H817", "Intimidación", "all_enemies",
              "Un rugido que da miedo. Quien falle la salvación (defensa mental) queda en shock 1 turno "
              "(no ataca y se defiende con desventaja) y con -1 a la CA durante 3 turnos.",
              [("Shock: 1 turno sin atacar.", estado(5, "incapacitado")),
               ("Shock: se defiende con desventaja 1 turno.", estado(5, "desventaja:defensa")),
               ("Intimidación: -1 a la CA durante 3 turnos.", estado(20, "penal_ca:1"))],
              salvacion="defensa_mental"),
        carta("H818", "Fisura del hombro", "single_enemy",
              "Te lanzas con la hombrera por delante: 1d6 de daño de fuego y el enemigo queda aturdido "
              "1 turno (no ataca y se defiende con desventaja).",
              [("1d6 de daño de fuego.", dano(200, 100))] + _aturdido(5), elemento="Fuego"),
    ],
    "creador": [
        carta("H819", "Estimulación", "all_allies",
              "Un grito que sube la moral del grupo: tú y tus aliados atacáis con ventaja hasta el final "
              "del combate.",
              [("Moral: ventaja en ataque todo el combate.", estado(300, "ventaja:ataque"))]),
        carta("H820", "Terremoto", "all_enemies",
              "Clavas el espadón y la tierra tiembla: 2d6 de daño de tierra a todos los enemigos y shock "
              "1 turno (no atacan y se defienden con desventaja).",
              [("2d6 de daño de tierra a cada enemigo.", dano(800, 400))] + _aturdido(5, "Shock"), elemento="Tierra"),
        carta("H821", "Dragón volador", "zone_radius",
              "El remate de fuego: 3d6 de daño y el enemigo se queda sin escudo 2 turnos (se defiende "
              "con desventaja).",
              [("3d6 de daño de fuego.", dano(900, 1500)),
               ("Destruir escudo: se defiende con desventaja 2 turnos.", estado(10, "desventaja:defensa"))],
              elemento="Fuego"),
    ],
    # Oriol (07/10): Provocación, entre las de arranque (por defecto, las tres primeras).
    "arranque": ["Batalla Triple", "Piel de acero", "Provocación"],
    # El arma con la que empieza (una normal de su tipo: la SP lucha con espadón).
    "arma_inicial": "espadon_de_recluta_de_milicia",
    # El set: lo que se ve en NosTale (actores_trajes: yelmo y coraza).
    "set": [
        pieza("Espadón del Guerrero", "MainHand", "pesada", {"CON": 1, "DES": 1},
              "El espadón de la SP del Guerrero: acero templado al fuego, a dos manos.",
              restricciones=["Precisión: CON", DOS_MANOS]),
        pieza("Coraza del Guerrero", "Torso", "pesada", {"CON": 1},
              "Placas de acero con hombreras grandes y el tabardo carmesí del Guerrero."),
        pieza("Yelmo del Guerrero", "Head", "pesada", {"CON": 1, "CAR": 1},
              "Yelmo de acero con el penacho carmesí del Guerrero."),
        pieza("Quijotes del Guerrero", "Legs", "pesada", {"CON": 1, "DES": 1},
              "Quijotes y grebas de acero del Guerrero."),
        pieza("Escarpes del Guerrero", "Feet", "pesada", {"CON": 1, "DES": 1},
              "Escarpes de placas del Guerrero."),
    ],
}

NINJA = {
    "id": "ninja", "name": "Ninja", "sp": 2, "elemento": "Agua", "nostale": "Ninja",
    "description": ("Especialista de la espada de agua: golpes rápidos, se acerca de un salto y una defensa "
                    "que convierte el golpe enemigo en fuerza propia."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "agile",
    "pasiva": ("Cuchillada de espada",
               "La cadena de cuchilladas rápidas del Ninja: tus golpes con arma hacen +1 de daño (agua).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H823", "Cuchillada de cruz", "single_enemy",
              "Una cuchillada en cruz: 1d6 de daño de agua.",
              [("1d6 de daño de agua.", dano(300, 100))], elemento="Agua"),
        carta("H824", "Cuchilla energética", "zone_radius",
              "El tajo parte la tierra hasta el enemigo lejano: 2d6 de daño de agua.",
              [("2d6 de daño de agua.", dano(150, 650))], elemento="Agua"),
        carta("H825", "Ataque en cadena", "zone_radius",
              "Tres tajos encadenados que confunden al enemigo: 2d6 de daño de agua.",
              [("2d6 de daño de agua.", dano(600, 400))], elemento="Agua"),
        carta("H826", "Protector de cuchillas", "self",
              "Preparas la hoja: contrarrestas los ataques de ese turno y la fuerza del golpe pasa a tu "
              "espada: +1d6 al daño de tus golpes con arma durante 2 turnos.",
              [CONTRARRESTA,
               ("Absorbe el poder de ataque: +1d6 a tus golpes con arma 2 turnos.",
                "propio:2:bono_dano_arma:1d6")],
              accion="reaccion"),
        carta("H827", "Tormenta de espadas", "all_enemies",
              "Una tormenta de tajos a tu alrededor: 2d6 de daño de agua a cada enemigo, que sangra "
              "3 turnos.",
              # 70 % de pierna herida: velocidad, no se simula
              [("2d6 de daño de agua a cada enemigo.", dano(400, 500)),
               ("Hemorragia seria: sangra 3 turnos.", condicion("sangrado", 20))], elemento="Agua"),
        carta("H828", "Ataque a la velocidad de la luz", "single_enemy",
              "Cruzas el campo en un instante: 2d6 de daño de agua y -1 a su CA durante 3 turnos.",
              [("2d6 de daño de agua.", dano(600, 200)),
               ("Debilita su defensa: -1 a la CA durante 3 turnos.", estado(30, "penal_ca:1"))],
              elemento="Agua"),
        carta("H829", "Recuperación espiritual", "self",
              "Meditas un instante: te libras de los estados en contra y te curas 1d6.",
              [("Te libras de los estados en contra.", "limpia"), ("Te curas 1d6.", "cura:1d6")]),
    ],
    "creador": [
        carta("H830", "Cuchillada mortal", "zone_radius",
              "Reúnes toda tu energía en un punto: 3d6 de daño de agua y su defensa se parte: -2 a la CA "
              "durante 3 turnos.",
              [("3d6 de daño de agua.", dano(1000, 600)),
               ("Cortar: -2 a la CA durante 3 turnos.", estado(20, "penal_ca:2"))], elemento="Agua"),
        carta("H831", "Borde afilado", "self",
              "Afilas la vista y la hoja hasta el final del combate: atacas con ventaja y tus golpes con "
              "arma hacen +1.",
              [("Ventaja en ataque todo el combate.", estado(180, "ventaja:ataque")),
               ("+1 a tus golpes con arma todo el combate.", estado(180, "bono_dano_arma:1"))]),
        carta("H832", "Espada danzante", "zone_radius",
              "Dejas volar la espada: 4d6 de daño de agua y shock 1 turno (no ataca y se defiende con "
              "desventaja).",
              [("4d6 de daño de agua.", dano(1800, 1000))] + _aturdido(5, "Shock"), elemento="Agua"),
    ],
    "arma_inicial": "sable_ligero_de_milicia",
    "set": [
        pieza("Katana del Ninja", "MainHand", "ligera", {"DES": 1, "CON": 1},
              "La katana de la SP del Ninja: hoja fina y curva, templada en agua."),
        pieza("Gi del Ninja", "Torso", "ligera", {"DES": 1},
              "Gi azul noche cruzado al pecho, con el fajín y la bufanda larga del Ninja."),
        pieza("Cinta del Ninja", "Head", "ligera", {"DES": 1, "CAR": 1},
              "Cinta celeste en la frente, con la placa de acero y las colas largas al viento."),
        pieza("Hakama del Ninja", "Legs", "ligera", {"DES": 1, "CON": 1},
              "Pantalón ancho atado a los tobillos."),
        pieza("Tabi del Ninja", "Feet", "ligera", {"DES": 1, "INT": 1},
              "Calzado ligero para no hacer ruido."),
    ],
}

CRUZADO = {
    "id": "cruzado", "name": "Cruzado", "sp": 3, "elemento": "Luz", "nostale": "Cruzado (Crusader)",
    "description": ("Especialista de la ballesta de luz: dispara de lejos, cura y bendice al grupo con sus "
                    "plegarias."),
    "primaryStat": "DES", "secondaryStat": "CAR", "role": "support",
    "pasiva": ("Ataque elemental",
               "El virote de luz del Cruzado: tus golpes con arma hacen +1 de daño (luz).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H890", "Flecha santa", "zone_radius",
              "Un virote de luz santa: 2d6 de daño de luz.",
              # 20 % de luz de la magia: fuera
              [("2d6 de daño de luz.", dano(570, 450))], elemento="Luz"),
        carta("H891", "Ataque triple", "zone_radius",
              "Tres virotes seguidos: 2d6 de daño de luz.",
              [("2d6 de daño de luz.", dano(750, 500))], elemento="Luz"),
        carta("H892", "Aliento santo", "zone_radius",
              "La luz sagrada debilita al enemigo: -1 a su CA durante 2 turnos.",
              [("Aliento sagrado: -1 a la CA durante 2 turnos.", estado(15, "penal_ca:1"))]),
        carta("H893", "Elemento santo", "self",
              "La luz carga tus virotes: +1 al daño de tus golpes con arma durante 3 turnos.",
              [("+1 a tus golpes con arma durante 3 turnos.", estado(20, "bono_dano_arma:1"))]),
        carta("H894", "Efecto deslumbrador", "single_ally",
              "Una luz santa envuelve a un aliado y le cura 1d6.",
              [("Cura 1d6.", "cura:1d6")]),
        carta("H895", "Cruz santa", "zone_radius",
              "Una gran cruz de luz estalla sobre el enemigo: 2d6 de daño de luz.",
              # eliminacion de las tinieblas (no puede usar su fuerza): no se simula; 30 % de shock: fuera
              [("2d6 de daño de luz.", dano(700, 500))], elemento="Luz"),
        carta("H896", "Escudo creciente", "all_allies",
              "Levantas un escudo sagrado que cubre al grupo: +1 a la CA de todos durante 2 turnos.",
              [("Armadura sagrada: +1 a la CA durante 2 turnos.", estado(10, "bono_ca:1"))]),
    ],
    "creador": [
        carta("H897", "Plegaria de defensa", "all_allies",
              "Una plegaria por el grupo: +1 a la CA de todos hasta el final del combate.",
              [("Súplica de defensa: +1 a la CA todo el combate.", estado(300, "bono_ca:1"))]),
        carta("H898", "Plegaria de ataque", "all_allies",
              "Una plegaria de guerra: todos atacáis con ventaja y vuestros golpes con arma hacen +1 hasta "
              "el final del combate.",
              [("Súplica de ataque: ventaja en ataque todo el combate.", estado(300, "ventaja:ataque")),
               ("Súplica de ataque: +1 a los golpes con arma todo el combate.", estado(300, "bono_dano_arma:1"))]),
        carta("H899", "Fuerza concentrada", "all_enemies",
              "El poder concentrado de la luz: 4d6 de daño de luz a cada enemigo y aturdimiento 1 turno "
              "(no atacan y se defienden con desventaja).",
              [("4d6 de daño de luz a cada enemigo.", dano(1500, 2500))] + _aturdido(6), elemento="Luz"),
    ],
    "arma_inicial": "ballesta_ligera_de_milicia",
    "set": [
        pieza("Ballesta del Cruzado", "MainHand", "media", {"DES": 1, "CAR": 1},
              "La ballesta de la SP del Cruzado: plata y oro, virotes de luz."),
        pieza("Coraza del Cruzado", "Torso", "media", {"CON": 1},
              "Plata con el tabardo blanco y la cruz de oro del Cruzado."),
        pieza("Yelmo Alado del Cruzado", "Head", "media", {"CON": 1, "CAR": 1},
              "Yelmo de plata con dos alas blancas."),
        pieza("Grebas del Cruzado", "Legs", "media", {"CON": 1, "DES": 1},
              "Grebas de plata del Cruzado."),
        pieza("Escarpes del Cruzado", "Feet", "media", {"DES": 1, "CAR": 1},
              "Escarpes de plata del Cruzado."),
    ],
}

BERSERKER = {
    "id": "berserker", "name": "Berserker", "sp": 4, "elemento": "Tinieblas", "nostale": "Berserker",
    "description": ("Especialista del arma enorme de tinieblas: rompe la defensa del enemigo y entra en furia, "
                    "más fuerte cuanto más arriesga."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "balanced",
    "pasiva": ("Mueve la barra",
               "Los golpes salvajes del Berserker: tus golpes con arma hacen +1 de daño (tinieblas).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H901", "Golpe", "zone_radius",
              "Golpeas con el arma enorme: 2d6 de daño de tinieblas.",
              [("2d6 de daño de tinieblas.", dano(360, 500))], elemento="Tinieblas"),
        carta("H902", "Gran golpe", "all_enemies",
              "Un barrido con el arma de tinieblas: 2d6 de daño a cada enemigo.",
              [("2d6 de daño de tinieblas a cada enemigo.", dano(360, 300))], elemento="Tinieblas"),
        carta("H903", "Berserker", "self",
              "Entras en furia hasta el final del combate: atacas con ventaja y tus golpes con arma hacen "
              "+1d6, pero tu guardia se abre: -1 a tu CA.",
              # el daño recibido +20 % es lo que cuesta la furia: -1 a la CA (bono_dano_contra en ti
              # solo lo cobraria quien te lo puso, que eres tu)
              [("Ventaja en ataque todo el combate.", estado(120, "ventaja:ataque")),
               ("+1d6 a tus golpes con arma todo el combate.", estado(120, "bono_dano_arma:1d6")),
               ("-1 a tu CA todo el combate.", estado(120, "penal_ca:1"))]),
        carta("H904", "Asalto sónico", "single_enemy",
              "Te lanzas sobre el enemigo: 1d6 de daño de tinieblas y sangra 2 turnos.",
              [("1d6 de daño de tinieblas.", dano(100, 250)),
               ("Hemorragia mortal: sangra 2 turnos.", condicion("sangrado", 10))], elemento="Tinieblas"),
        carta("H905", "Sobre la cara", "zone_radius",
              "Un golpe de locura en la cara: 2d6 de daño de tinieblas y aturdimiento 1 turno (no ataca y "
              "se defiende con desventaja).",
              [("2d6 de daño de tinieblas.", dano(570, 720))] + _aturdido(5), elemento="Tinieblas"),
        carta("H906", "Ataque de locura", "all_enemies",
              "Golpeas a todos sin control: 3d6 de daño de tinieblas a cada enemigo, que ataca con "
              "desventaja 2 turnos.",
              [("3d6 de daño de tinieblas a cada enemigo.", dano(700, 950)),
               ("Incapacidad: ataca con desventaja 2 turnos.", estado(15, "desventaja:ataque"))],
              elemento="Tinieblas"),
        carta("H907", "Ruptura de la armadura", "all_enemies",
              "Destrozas las armaduras. Quien falle la salvación (resistencia física) tiene -2 a la CA "
              "durante 3 turnos.",
              [("Destroza la armadura: -2 a la CA durante 3 turnos.", estado(20, "penal_ca:2"))],
              salvacion="resistencia_fisica"),
    ],
    "creador": [
        carta("H908", "Chute de adrenalina", "zone_radius",
              "Destrozas el suelo bajo el enemigo: 3d6 de daño de tinieblas y shock 1 turno (no ataca y "
              "se defiende con desventaja).",
              [("3d6 de daño de tinieblas.", dano(1100, 1200))] + _aturdido(5, "Shock"), elemento="Tinieblas"),
        carta("H909", "Vida y muerte", "self",
              "La adrenalina te desborda: tus golpes con arma hacen +2d6 durante 2 turnos.",
              [("+2d6 a tus golpes con arma durante 2 turnos.", estado(8, "bono_dano_arma:2d6")),
               _pendiente("que crezca cuanta menos vida te queda")]),
        carta("H910", "Corte del tornado", "all_enemies",
              "Un tornado de tinieblas: 4d6 de daño a cada enemigo, que se queda sin escudo 2 turnos (se "
              "defiende con desventaja).",
              [("4d6 de daño de tinieblas a cada enemigo.", dano(1400, 1700)),
               ("Destruir escudo: se defienden con desventaja 2 turnos.", estado(10, "desventaja:defensa"))],
              elemento="Tinieblas"),
    ],
    "arma_inicial": "espadon_de_recluta_de_viaje",
    "set": [
        pieza("Mandoble del Berserker", "MainHand", "pesada", {"CON": 1, "DES": 1},
              "El arma enorme de la SP del Berserker: hoja negra con filo rojo, a dos manos.",
              restricciones=["Precisión: CON", DOS_MANOS]),
        pieza("Coraza del Berserker", "Torso", "media", {"CON": 1},
              "Placas negras con pinchos y la faja roja del Berserker."),
        pieza("Yelmo Astado del Berserker", "Head", "media", {"CON": 1, "DES": 1},
              "Yelmo negro con dos cuernos."),
        pieza("Quijotes del Berserker", "Legs", "media", {"DES": 1, "CON": 1},
              "Quijotes de hierro negro."),
        pieza("Escarpes del Berserker", "Feet", "media", {"DES": 1, "CAR": 1},
              "Escarpes de hierro negro."),
    ],
}

GLADIADOR = {
    "id": "gladiador", "name": "Gladiador", "sp": 5, "elemento": "Fuego", "nostale": "Gladiador (Gladiator)",
    "description": ("Especialista de la espada y el escudo de fuego: derriba, se burla del rival y remata "
                    "ante la multitud."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "balanced",
    "pasiva": ("Golpes de todas partes",
               "Los ataques del Gladiador llegan de todas direcciones: tus golpes con arma hacen +1 de daño "
               "(fuego).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H1057", "Golpe doble", "single_enemy",
              "Escudo y espada a la vez: 1d6 de daño de fuego.",
              # 20 % de derribo: fuera
              [("1d6 de daño de fuego.", dano(300, 100))], elemento="Fuego"),
        carta("H1058", "Relámpago", "zone_radius",
              "Una puñalada repentina: 1d6 de daño de fuego.",
              [("1d6 de daño de fuego.", dano(150, 200))], elemento="Fuego"),
        carta("H1059", "Ataque de escudo", "single_enemy",
              "Embistes con el escudo: 2d6 de daño de fuego y el enemigo cae al suelo.",
              [("2d6 de daño de fuego.", dano(450, 300)),
               ("Derribo: cae al suelo 1 turno.", condicion("caido", 3))], elemento="Fuego"),
        carta("H1060", "Cuerno de caza", "all_enemies",
              "Un impacto sobre todo el campamento enemigo: 2d6 de daño de fuego a cada uno y -1 a su CA "
              "durante 3 turnos.",
              [("2d6 de daño de fuego a cada enemigo.", dano(600, 350)),
               ("Debilita su defensa: -1 a la CA durante 3 turnos.", estado(30, "penal_ca:1"))],
              elemento="Fuego"),
        carta("H1061", "Burla", "zone_radius",
              "Te burlas del rival: 2d6 de daño de fuego, y tiembla: ataca con desventaja y tiene -1 a la "
              "CA durante 2 turnos.",
              [("2d6 de daño de fuego.", dano(800, 400)),
               ("Tembloroso: ataca con desventaja 2 turnos.", estado(15, "desventaja:ataque")),
               ("Tembloroso: -1 a la CA durante 2 turnos.", estado(15, "penal_ca:1"))], elemento="Fuego"),
        carta("H1062", "Reflexión mágica", "self",
              "Alzas el escudo: contrarrestas los ataques de ese turno.",
              [CONTRARRESTA, _pendiente("devolver al atacante el 20 % del daño")], accion="reaccion"),
        carta("H1063", "Rodillo de hierro", "all_enemies",
              "Giras con el escudo y apartas a todos: 3d6 de daño de fuego a cada enemigo.",
              # 45 % de derribo: fuera
              [("3d6 de daño de fuego a cada enemigo.", dano(900, 400))], elemento="Fuego"),
    ],
    "creador": [
        carta("H1064", "Gangrena", "self",
              "Un grito de guerra con sed de sangre: te curas 1d6 y, hasta el final del combate, atacas con "
              "ventaja y tus golpes con arma hacen +1.",
              [("Te curas 1d6.", "cura:1d6"),
               ("Gangrena: ventaja en ataque todo el combate.", estado(120, "ventaja:ataque")),
               ("Gangrena: +1 a tus golpes con arma todo el combate.", estado(120, "bono_dano_arma:1"))]),
        carta("H1065", "Declaración de guerra", "zone_radius",
              "Los truenos acompañan el remate: 3d6 de daño de fuego.",
              # suspiro del perdedor: no se mueve; sin posiciones, nada
              [("3d6 de daño de fuego.", dano(1200, 800))], elemento="Fuego"),
        carta("H1066", "Ejecución", "zone_radius",
              "La ejecución ante la multitud: 4d6 de daño de fuego y aturdimiento 1 turno (no ataca y se "
              "defiende con desventaja); el júbilo de la multitud te libra de los estados en contra.",
              [("4d6 de daño de fuego.", dano(1800, 1000))] + _aturdido(6)
              + [("Júbilo de la multitud: te libras de los estados en contra.", "limpia")], elemento="Fuego"),
    ],
    "arma_inicial": "item_longsword",
    "set": [
        pieza("Espada Corta del Gladiador", "MainHand", "media", {"DES": 1, "CON": 1},
              "El gladio de la SP del Gladiador: hoja ancha y corta, guarnición de oro."),
        pieza("Armadura del Gladiador", "Torso", "media", {"CON": 1},
              "Coraza musculada de oro, una hombrera grande, brazales y el faldellín de tiras."),
        pieza("Galea del Gladiador", "Head", "media", {"CON": 1, "CAR": 1},
              "La galea de oro con la cresta roja."),
        pieza("Grebas del Gladiador", "Legs", "media", {"DES": 1, "CON": 1},
              "Grebas de oro con la pierna al aire."),
        pieza("Sandalias del Gladiador", "Feet", "ligera", {"DES": 1, "CAR": 1},
              "Sandalias de cuero atadas a la pierna."),
    ],
}

MONJE = {
    "id": "monje_de_lucha", "name": "Monje de lucha", "sp": 6, "elemento": "Agua",
    "nostale": "Monje de lucha (Battle Monk)",
    "description": ("Especialista de la lanza de agua: encadena técnicas, protege a un aliado con su cuerpo y "
                    "medita para afianzar su fuerza."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "balanced",
    # La Meditacion (H1096, con sus tres espiritus H2001-H2003) es la forma de la SP: la pasiva (§6).
    "pasiva": ("Meditación",
               "Al empezar el combate eliges tu espíritu. Fuerza: atacas con ventaja. Iluminado: +1 a la CA. "
               "Mesura: te defiendes con ventaja.",
               [("Eliges un espíritu al empezar el combate.", elige_una("fuerza", "iluminado", "mesura")),
                ("Fuerza mental: atacas con ventaja.", si("fuerza", "ventaja:ataque")),
                ("Espíritu iluminado: +1 a la CA.", si("iluminado", "bono_ca:1")),
                ("Espíritu de la mesura: te defiendes con ventaja.", si("mesura", "ventaja:defensa"))]),
    "cartas": [
        carta("H1090", "Mar partido", "zone_radius",
              "Partes el mar y al enemigo en dos: 1d6 de daño de agua.",
              [("1d6 de daño de agua.", dano(180, 120))], elemento="Agua"),
        carta("H1091", "Punzada directa", "single_enemy",
              "La fuerza acumulada en la lanza va directa al enemigo: 2d6 de daño de agua, y la herida "
              "interna hace que tus golpes le hagan +1 durante 3 turnos.",
              [("2d6 de daño de agua.", dano(330, 190)),
               ("Herida interna: tus golpes le hacen +1 durante 3 turnos.", estado(25, "bono_dano_contra:1"))],
              elemento="Agua"),
        carta("H1092", "Remolino violento", "zone_radius",
              "Un remolino que arrasa al enemigo de un plumazo: 2d6 de daño de agua.",
              [("2d6 de daño de agua.", dano(440, 330))], elemento="Agua"),
        # Siete punzadas abre Arpon de dragon (H1100, su continuacion): una carta (§2).
        carta("H1093+H1100", "Siete punzadas", "zone_radius",
              "Siete punzadas seguidas y el arpón de dragón: 2d6 de daño de agua; la herida interna hace "
              "que tus golpes le hagan +1 durante 3 turnos, y sangra.",
              [("2d6 de daño de agua.", dano(200, 80, 330, 330)),
               ("Herida interna: tus golpes le hacen +1 durante 3 turnos.", estado(25, "bono_dano_contra:1")),
               ("Trauma fuerte: sangra 1 turno.", condicion("sangrado", 6))], elemento="Agua"),
        carta("H1094", "Ciclón", "all_enemies",
              "El ciclón derriba y atrae: 2d6 de daño de agua a cada enemigo, que te ataca a ti 1 turno.",
              [("2d6 de daño de agua a cada enemigo.", dano(550, 300)), _atrae()], elemento="Agua"),
        carta("H1095", "Sacrificio", "single_ally",
              "Proteges a un aliado: la próxima vez que le ataquen, te pones delante y recibes tú el golpe.",
              [("Te pones delante del aliado.", "defiende_aliado:1")], accion="reaccion"),
        carta("H1097", "Lanza en espiral", "zone_radius",
              "La lanza en espiral llega lejos: 2d6 de daño de agua; el trauma le deja -1 a la CA 1 turno "
              "y sangrando.",
              [("2d6 de daño de agua.", dano(330, 230)),
               ("Trauma crítico: -1 a la CA 1 turno.", estado(4, "penal_ca:1")),
               ("Trauma crítico: sangra 1 turno.", condicion("sangrado", 4))], elemento="Agua"),
    ],
    # Creador: las de profesion 16 y 20. Arpon de dragon (20) va con Siete punzadas; Lanza de
    # dragon (20), continuacion de Ciclon, queda como carta propia: no cabe en Ciclon y es de
    # las definitivas. Lanza en espiral (12) se queda estandar.
    "creador": [
        carta("H1098", "Palabras de Buda", "all_allies",
              "Invocas las palabras de Buda con un talismán: todos os libráis de los estados en contra y "
              "os curáis 1d6.",
              [("Os libráis de los estados en contra.", "limpia"), ("Os curáis 1d6.", "cura:1d6")]),
        carta("H1099", "Borrón y cuenta nueva", "all_enemies",
              "Pones fin al caos: 4d6 de daño de agua a cada enemigo; se quedan sin fuerzas 1 turno (no "
              "atacan) y tus golpes les hacen +1 durante 2 turnos.",
              [("4d6 de daño de agua a cada enemigo.", dano(1350, 1150)),
               ("Falta crítica de energía: 1 turno sin atacar.", estado(5, "incapacitado")),
               ("Herida interna: tus golpes les hacen +1 durante 2 turnos.", estado(15, "bono_dano_contra:1"))],
              elemento="Agua"),
        carta("H1101", "Lanza de dragón", "all_enemies",
              "La lanza de dragón lanza a los enemigos al cielo: 3d6 de daño de agua a cada uno, con -1 a "
              "la CA 1 turno y sangrando.",
              [("3d6 de daño de agua a cada enemigo.", dano(700, 1000)),
               ("Trauma crítico: -1 a la CA 1 turno.", estado(4, "penal_ca:1")),
               ("Trauma crítico: sangran 1 turno.", condicion("sangrado", 4))], elemento="Agua"),
    ],
    "arma_inicial": "lanza_corta_de_milicia",
    "set": [
        pieza("Lanza del Monje de Lucha", "MainHand", "media", {"DES": 1, "CON": 1},
              "La lanza de la SP del Monje de lucha: asta azul y punta de plata."),
        pieza("Hábito del Monje de Lucha", "Torso", "ligera", {"CON": 1},
              "Hábito azul con la faja blanca y el rosario grande del Monje de lucha."),
        pieza("Diadema del Monje de Lucha", "Head", "ligera", {"DES": 1, "CAR": 1},
              "Diadema de cuentas de oración."),
        pieza("Calzones del Monje de Lucha", "Legs", "ligera", {"DES": 1, "CON": 1},
              "Calzones anchos de tela."),
        pieza("Sandalias del Monje de Lucha", "Feet", "ligera", {"DES": 1, "INT": 1},
              "Sandalias de monje."),
    ],
}

COSECHADOR = {
    "id": "cosechador_de_almas", "name": "Cosechador de almas", "sp": 7, "elemento": "Tinieblas",
    "nostale": "Cosechador de almas (Death Reaper)",
    "description": ("Especialista de la guadaña de tinieblas: siega, absorbe la vida de sus enemigos y los "
                    "marca para la muerte."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "balanced",
    "pasiva": ("Guadaña de la muerte",
               "Los tajos de la guadaña del Cosechador: tus golpes con arma hacen +1 de daño (tinieblas).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H1169", "Cuchilla", "single_enemy",
              "Cortas en rodajas con la cuchilla de la guadaña: 1d6 de daño de tinieblas.",
              [("1d6 de daño de tinieblas.", dano(330, 90))], elemento="Tinieblas"),
        carta("H1170", "Aspirasangre", "all_enemies",
              "Rajas a todos los que tienes cerca y les absorbes la vida: 1d6 de daño de sangre a cada "
              "enemigo, y te curas lo que hagas.",
              [("1d6 de daño de sangre a cada enemigo.", dano(200, 180)),
               ("Te curas el daño que haces.", "cura:dano_causado")], elemento="Sangre"),
        # La muerte se acerca abre su segundo golpe (H1179, su continuacion): una carta (§2).
        carta("H1171+H1179", "La muerte se acerca", "single_enemy",
              "Te acercas al enemigo y le atacas dos veces: 3d6 de daño de tinieblas y aturdimiento 1 "
              "turno (no ataca y se defiende con desventaja).",
              [("3d6 de daño de tinieblas.", dano(300, 200, 700, 400))] + _aturdido(2), elemento="Tinieblas"),
        carta("H1172", "Empuje de almas", "zone_radius",
              "Empujas a los enemigos de delante y de detrás: 2d6 de daño de tinieblas.",
              # empujar y ralentizar: sin posiciones ni velocidad
              [("2d6 de daño de tinieblas.", dano(500, 600))], elemento="Tinieblas"),
        carta("H1173", "Coraza suprema", "self",
              "Una coraza de almas hasta el final del combate: +1 a la CA.",
              [("+1 a la CA todo el combate.", estado(180, "bono_ca:1"))]),
        carta("H1174", "Captor de espíritus", "zone_radius",
              "Atrapas al enemigo cercano: se queda rígido 1 turno (no ataca) y le absorbes 1 de vida por "
              "turno durante 2 turnos.",
              # la absorcion espiritual (te cura poco a poco) se lee como absorber su vida
              [("Rigidez: 1 turno sin atacar.", estado(2, "incapacitado")),
               ("Absorción espiritual: 1 de vida por turno para ti durante 2 turnos.",
                estado(15, "absorbe_por_turno:1"))]),
        carta("H1175", "Guadaña mortal", "zone_radius",
              "Lanzas la guadaña y dejas la marca de la muerte: 3d6 de daño de tinieblas, y queda marcado.",
              [("3d6 de daño de tinieblas.", dano(800, 750)),
               ("Marca de la muerte débil: queda marcado 1 turno.", condicion("marcado", 5))],
              elemento="Tinieblas"),
    ],
    "creador": [
        carta("H1176", "Tinieblas mortales", "all_enemies",
              "Las tinieblas siembran el miedo. Quien falle la salvación (defensa mental) queda asustado y "
              "marcado para la muerte.",
              [("Miedo: asustado 1 turno.", condicion("asustado", 5)),
               ("Marca de la muerte: marcado 1 turno.", condicion("marcado", 5))],
              salvacion="defensa_mental"),
        carta("H1177", "Convertidor temporal", "self",
              "Ajustas el tiempo a tu favor: recuperas una carta estándar gastada.",
              [("Recuperas una carta estándar gastada.", "devuelve_carta:estandar")], accion="libre"),
        carta("H1178", "División de espíritus", "zone_radius",
              "Un salto que divide el espíritu del enemigo: 3d6 de daño de tinieblas y -2 a su CA hasta el "
              "final del combate.",
              [("3d6 de daño de tinieblas.", dano(1200, 900)),
               ("Destrucción de las armas de defensa: -2 a la CA todo el combate.", estado(120, "penal_ca:2"))],
              elemento="Tinieblas"),
    ],
    # El catalogo no tiene guadaña de tier 1: una normal, como las de Viaje.
    "arma_inicial": "guadana_de_viaje",
    "armas_nuevas": [
        arma_comun("Guadaña de Viaje", "pesada", "Una guadaña de segar, larga y pesada, a dos manos.",
                   restricciones=["Precisión: CON", DOS_MANOS]),
    ],
    "set": [
        pieza("Guadaña del Cosechador de Almas", "MainHand", "pesada", {"CON": 1, "DES": 1},
              "La guadaña de la SP del Cosechador de almas: hoja negra con el filo violeta.",
              restricciones=["Precisión: CON", DOS_MANOS]),
        pieza("Manto del Cosechador de Almas", "Torso", "media", {"CON": 1},
              "Manto negro largo, abierto, con el forro violeta y la calavera del cinturón."),
        pieza("Capucha del Cosechador de Almas", "Head", "ligera", {"DES": 1, "INT": 1},
              "Capucha negra honda que deja la cara en sombra."),
        pieza("Calzas del Cosechador de Almas", "Legs", "ligera", {"DES": 1, "CON": 1},
              "Calzas negras."),
        pieza("Botas del Cosechador de Almas", "Feet", "ligera", {"DES": 1, "CAR": 1},
              "Botas negras con hebillas de plata."),
    ],
}

RENEGADO = {
    "id": "renegado", "name": "Renegado", "sp": 8, "elemento": "Luz", "nostale": "Renegado (Renegade)",
    "description": ("Especialista de la espada de luz y de sombra: blande la Espada Luminosa para atacar y la "
                    "Espada Sombría para aguantar."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "balanced",
    "pasiva": ("Espada de la luz",
               "Los dos tajos del Renegado: tus golpes con arma hacen +1 de daño (luz).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    # La forma de luz y la de tinieblas tienen las mismas cifras (H1339-H1348 y H2004-H2012): cada
    # carta es las dos. Ignicion y Reignicion, las que cambian de espada, son cartas.
    "cartas": [
        carta("H1339/H2005", "Juicio de la luz", "zone_radius",
              "Dos puñaladas rápidas con la espada sagrada: 1d6 de daño de luz.",
              [("1d6 de daño de luz.", dano(140, 220))], elemento="Luz"),
        # La segunda tambien se llama «Juicio de la luz» en la ficha; su gemela de tinieblas es
        # «Patibulo de la oscuridad»: Patibulo de la luz.
        carta("H1340/H2006", "Patíbulo de la luz", "all_enemies",
              "Una oleada de energía que todo lo atraviesa: 1d6 de daño de luz a cada enemigo, que queda "
              "1 turno sin atacar y con -1 a la CA.",
              [("1d6 de daño de luz a cada enemigo.", dano(300, 150)),
               ("Juicio de la condenación: 1 turno sin atacar.", estado(5, "incapacitado")),
               ("Juicio de la condenación: -1 a la CA 1 turno.", estado(5, "penal_ca:1"))], elemento="Luz"),
        carta("H1341/H2007", "Justicia", "all_enemies",
              "Invocas la gran espada sacra: 2d6 de daño de luz a cada enemigo, que se defiende con "
              "desventaja 2 turnos.",
              [("2d6 de daño de luz a cada enemigo.", dano(600, 250)),
               ("Juicio: se defienden con desventaja 2 turnos.", estado(8, "desventaja:defensa"))],
              elemento="Luz"),
        carta("H1342/H2008", "Rasgadura doble", "all_enemies",
              "Te abalanzas con la espada: 2d6 de daño de luz a cada enemigo y rigidez 1 turno (no ataca).",
              [("2d6 de daño de luz a cada enemigo.", dano(550, 400)),
               ("Rigidez: 1 turno sin atacar.", estado(2, "incapacitado"))], elemento="Luz"),
        carta("H1343/H2009", "Santidad", "all_enemies",
              "Una barrera de luz que ataca: 2d6 de daño de luz a cada enemigo.",
              [("2d6 de daño de luz a cada enemigo.", dano(700, 330))], elemento="Luz"),
        carta("H1344", "Ignición", "self",
              "Blandes la Espada Luminosa hasta el final del combate: tus golpes con arma hacen +1d6 y "
              "atacas con ventaja.",
              [("+1d6 a tus golpes con arma todo el combate.", estado(300, "bono_dano_arma:1d6")),
               ("Ventaja en ataque todo el combate.", estado(300, "ventaja:ataque"))]),
        carta("H1345/H2010", "Cuchilla en cruz", "zone_radius",
              "Atraviesas el espacio a gran velocidad: 3d6 de daño de luz y aturdimiento 1 turno (no ataca "
              "y se defiende con desventaja).",
              [("3d6 de daño de luz.", dano(900, 440))] + _aturdido(2), elemento="Luz"),
    ],
    "creador": [
        carta("H1346", "Reignición", "self",
              "Blandes la Espada Sombría hasta el final del combate: +1 a la CA.",
              [("+1 a la CA todo el combate.", estado(300, "bono_ca:1"))]),
        carta("H1347/H2011", "Corrupción", "all_enemies",
              "Corrompes con miedo: 3d6 de daño de luz a cada enemigo, que queda asustado y sin defensa 1 "
              "turno.",
              [("3d6 de daño de luz a cada enemigo.", dano(1000, 650)),
               ("Miedo: asustado 1 turno.", condicion("asustado", 6)),
               ("Corrupción: se defienden con desventaja 1 turno.", estado(6, "desventaja:defensa"))],
              elemento="Luz"),
        carta("H1348/H2012", "Conversión", "all_enemies",
              "El arma del archidemonio atraviesa a todos: 4d6 de daño de luz a cada enemigo y aturdimiento "
              "1 turno (no atacan y se defienden con desventaja).",
              [("4d6 de daño de luz a cada enemigo.", dano(1700, 1100))] + _aturdido(6), elemento="Luz"),
    ],
    "arma_inicial": "item_longsword",
    "set": [
        pieza("Espada del Renegado", "MainHand", "media", {"DES": 1, "CON": 1},
              "La espada de la SP del Renegado: media hoja de luz y media de sombra."),
        pieza("Armadura del Renegado", "Torso", "media", {"CON": 1},
              "Armadura partida en dos: un lado blanco y oro, el otro negro y violeta."),
        pieza("Diadema del Renegado", "Head", "ligera", {"CAR": 1, "DES": 1},
              "Diadema de oro y plata negra con la gema violeta."),
        pieza("Quijotes del Renegado", "Legs", "media", {"CON": 1, "DES": 1},
              "Quijotes blancos del Renegado."),
        pieza("Botas del Renegado", "Feet", "ligera", {"DES": 1, "CAR": 1},
              "Botas negras del Renegado."),
    ],
}

HACHERO = {
    "id": "hachero_de_la_cascada", "name": "Hachero de la cascada", "sp": 9, "elemento": "Agua",
    "nostale": "Hachero de la cascada (Waterfall Berserker)",
    "description": ("Especialista del hacha de agua: la lanza como un bumerán, atrae a los enemigos y descarga "
                    "la ira de la cascada."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "balanced",
    "pasiva": ("Golpe de hacha",
               "Los dos golpes de hacha del Hachero: tus golpes con arma hacen +1 de daño (agua).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("la barra de ira (la llenan los golpes y la gasta la Rabia de la cascada)")]),
    "cartas": [
        carta("H1353", "Lanzamiento de hacha", "single_enemy",
              "Lanzas el hacha y vuelve como un bumerán: 2d6 de daño de agua.",
              [("2d6 de daño de agua.", dano(500, 250))], elemento="Agua"),
        carta("H1354", "Ataque fugaz intenso", "single_enemy",
              "Te lanzas sobre el enemigo: 2d6 de daño de agua y aturdimiento 1 turno (no ataca y se "
              "defiende con desventaja).",
              [("2d6 de daño de agua.", dano(650, 450))] + _aturdido(5), elemento="Agua"),
        carta("H1382", "Hacha de la ira", "single_enemy",
              "Cinco hachazos en el mismo sitio: 2d6 de daño de agua.",
              [("2d6 de daño de agua.", dano(800, 400))], elemento="Agua"),
        carta("H1383", "Energía rúnica del agua", "self",
              "La runa del hacha te cura 1d6 y te da +1 a la CA durante 5 turnos.",
              [("Te curas 1d6.", "cura:1d6"),
               ("Energía del agua pura: +1 a la CA durante 5 turnos.", estado(60, "bono_ca:1"))]),
        carta("H1384", "Contracorriente", "all_enemies",
              "Atraes a los enemigos con el hacha: 3d6 de daño de agua a cada uno, que te ataca a ti 1 "
              "turno.",
              [("3d6 de daño de agua a cada enemigo.", dano(1000, 550)), _atrae()], elemento="Agua"),
        # Arrebato de ira abre Arrebato de ira del berserker (H1390) y este Tormenta atronadora
        # (H1391): una carta que suma los tres (§2).
        carta("H1385+H1390+H1391", "Arrebato de ira", "zone_radius",
              "Tres golpes de ira, cada uno más fuerte: 3d6 de daño de agua.",
              [("3d6 de daño de agua.", dano(300, 200, 400, 200, 500, 400))], elemento="Agua"),
        carta("H1386", "Protección de la cascada", "self",
              "El agua de la runa rechaza los ataques de ese turno y te libra de los estados en contra.",
              [CONTRARRESTA, ("Te libras de los estados en contra.", "limpia")], accion="reaccion"),
    ],
    "creador": [
        carta("H1387", "Remolino en cascada", "all_enemies",
              "Giras como un remolino sobre tu eje: 3d6 de daño de agua a cada enemigo.",
              [("3d6 de daño de agua a cada enemigo.", dano(1100, 900)),
               _pendiente("que la Rabia de la cascada sume la barra de ira")], elemento="Agua"),
        carta("H1388", "Poder de la cascada", "self",
              "La energía de la cascada hasta el final del combate: atacas con ventaja y tus golpes con "
              "arma hacen +1.",
              [("Ventaja en ataque todo el combate.", estado(180, "ventaja:ataque")),
               ("+1 a tus golpes con arma todo el combate.", estado(180, "bono_dano_arma:1"))]),
        carta("H1389", "Golpe furioso de cascada", "all_enemies",
              "Saltas y descargas el hacha en diagonal: 3d6 de daño de agua a cada enemigo, y tus golpes "
              "les hacen +1 durante 2 turnos.",
              [("3d6 de daño de agua a cada enemigo.", dano(1100, 1000)),
               ("Golpe de cascada: tus golpes les hacen +1 durante 2 turnos.", estado(8, "bono_dano_contra:1"))],
              elemento="Agua"),
    ],
    "arma_inicial": "gran_hacha_de_milicia",
    "set": [
        pieza("Gran Hacha del Hachero de la Cascada", "MainHand", "pesada", {"CON": 1, "DES": 1},
              "La gran hacha de la SP del Hachero: la runa del agua en la hoja, a dos manos.",
              restricciones=["Precisión: CON", DOS_MANOS]),
        pieza("Coselete del Hachero de la Cascada", "Torso", "media", {"CON": 1},
              "Correajes de cuero sobre el pecho, con el manto de piel y la runa del agua."),
        pieza("Casco del Hachero de la Cascada", "Head", "media", {"CON": 1, "DES": 1},
              "Casco de hierro en gajos con el borde de piel."),
        pieza("Calzas del Hachero de la Cascada", "Legs", "ligera", {"DES": 1, "CON": 1},
              "Calzas azules atadas con tiras de cuero."),
        pieza("Botas del Hachero de la Cascada", "Feet", "media", {"CON": 1, "CAR": 1},
              "Botas altas con el borde de piel."),
    ],
}

CABALLERO = {
    "id": "caballero_draconiano", "name": "Caballero Draconiano", "sp": 10, "elemento": "Luz",
    "nostale": "Caballero Draconiano (Dragon Knight)",
    "description": ("Especialista de la lanza y el escudo de dragón: perfora armaduras, atrae a los enemigos y "
                    "protege al grupo con la luz del dragón."),
    "primaryStat": "CON", "secondaryStat": "DES", "role": "tank",
    "pasiva": ("Golpe rápido",
               "La lanza del Caballero Draconiano: tus golpes con arma hacen +1 de daño (luz).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("la agudeza que dan los golpes y la Lanza aguijón que la gasta")]),
    "cartas": [
        carta("H1722", "Golpe largo", "zone_radius",
              "Alcanzas con la lanza a un enemigo lejano: 2d6 de daño de luz.",
              [("2d6 de daño de luz.", dano(530, 370))], elemento="Luz"),
        carta("H1723", "Golpe de escudo", "zone_radius",
              "Un golpe de escudo que aturde: 2d6 de daño de luz y rigidez 1 turno (no ataca).",
              [("2d6 de daño de luz.", dano(500, 500)),
               ("Rigidez: 1 turno sin atacar.", estado(3, "incapacitado"))], elemento="Luz"),
        carta("H1724", "Embestida lancera", "zone_radius",
              "Esprintas con la lanza contra la hilera de enemigos: 2d6 de daño de luz.",
              # 30 % de herida grave: fuera
              [("2d6 de daño de luz.", dano(800, 400))], elemento="Luz"),
        carta("H1725", "Torbellino sagrado", "all_enemies",
              "Un torbellino de luz con la lanza: 3d6 de daño de luz a cada enemigo, que te ataca a ti 1 "
              "turno.",
              [("3d6 de daño de luz a cada enemigo.", dano(750, 900)), _atrae()], elemento="Luz"),
        carta("H1726", "Tiro con lanza", "zone_radius",
              "Arrojas la lanza a través de la fila: 3d6 de daño de luz y la armadura perforada: -1 a la CA "
              "durante 2 turnos.",
              [("3d6 de daño de luz.", dano(600, 750)),
               ("Armadura perforada: -1 a la CA durante 2 turnos.", estado(10, "penal_ca:1"))], elemento="Luz"),
        carta("H1727", "Golpe triple", "zone_radius",
              "Tres lanzadas seguidas: 3d6 de daño de luz; la herida grave le hace sangrar y le deja -1 a "
              "la CA durante 2 turnos.",
              [("3d6 de daño de luz.", dano(1200, 940)),
               ("Herida grave: sangra 2 turnos.", condicion("sangrado", 10)),
               ("Herida grave: -1 a la CA durante 2 turnos.", estado(10, "penal_ca:1"))], elemento="Luz"),
        carta("H1728", "Escudo de Dragonio", "self",
              "El escudo de dragonio contrarresta los ataques de ese turno.",
              [CONTRARRESTA], accion="reaccion"),
    ],
    "creador": [
        carta("H1729", "Destrucción de armadura", "single_enemy",
              "Cuatro lanzas de luz en los puntos débiles: 3d6 de daño de luz y la armadura perforada del "
              "todo: -2 a la CA durante 2 turnos.",
              [("3d6 de daño de luz.", dano(700, 900)),
               ("Armadura completamente perforada: -2 a la CA durante 2 turnos.", estado(10, "penal_ca:2"))],
              elemento="Luz"),
        carta("H1730", "Luz de protección", "all_allies",
              "La magia de dragón purga y protege: todos os libráis de los estados en contra y, hasta el "
              "final del combate, tenéis +1 a la CA y atacáis con ventaja.",
              [("Os libráis de los estados en contra.", "limpia"),
               ("Luz de protección: +1 a la CA todo el combate.", estado(180, "bono_ca:1")),
               ("Luz de protección: ventaja en ataque todo el combate.", estado(180, "ventaja:ataque"))]),
        carta("H1731", "Aguijón de dragón", "zone_radius",
              "Concentras la magia de dragón en un punto: 3d6 de daño de luz.",
              [("3d6 de daño de luz.", dano(950, 1300)),
               _pendiente("el crítico que suma si la Lanza aguijón está activa")], elemento="Luz"),
    ],
    "arma_inicial": "lanza_corta_de_viaje",
    "set": [
        pieza("Lanza del Caballero Draconiano", "MainHand", "media", {"CON": 1, "DES": 1},
              "La lanza de la SP del Caballero Draconiano: oro y escamas de dragón."),
        pieza("Armadura del Caballero Draconiano", "Torso", "pesada", {"CON": 1},
              "Escamas blancas con las alas de oro en el pecho y hombreras de oro en forma de ala de dragón."),
        pieza("Yelmo del Caballero Draconiano", "Head", "pesada", {"CON": 1, "CAR": 1},
              "Yelmo blanco con la cresta de dragón de oro."),
        pieza("Quijotes del Caballero Draconiano", "Legs", "pesada", {"CON": 1, "DES": 1},
              "Quijotes blancos del Caballero Draconiano."),
        pieza("Escarpes del Caballero Draconiano", "Feet", "pesada", {"CON": 1, "DES": 1},
              "Escarpes blancos del Caballero Draconiano."),
    ],
}

ROMPERROCAS = {
    "id": "romperrocas", "name": "Romperrocas", "sp": 11, "elemento": "Fuego",
    "nostale": "Romperrocas (Stone Breaker)",
    "description": ("Especialista del martillo de fuego: arranca mineral, se propulsa con cohetes y gira el "
                    "martillo para cubrirse."),
    "primaryStat": "CON", "secondaryStat": "DES", "role": "balanced",
    "pasiva": ("Golpe de martillo",
               "El martillo oscilante del Romperrocas: tus golpes con arma hacen +1 de daño (fuego).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("la barra de marcas de mineral (la llenan los golpes y la gastan las técnicas)")]),
    "cartas": [
        carta("H364", "Extracción", "zone_radius",
              "Un golpe que hace ceder hasta la roca: 2d6 de daño de fuego y -1 a su CA durante 2 turnos.",
              [("2d6 de daño de fuego.", dano(400, 200)),
               ("Agotado: -1 a la CA durante 2 turnos.", estado(10, "penal_ca:1"))], elemento="Fuego"),
        carta("H365", "Inyección mineral", "self",
              "Inyectas mineral en el martillo: tus golpes con arma hacen +1 durante 2 turnos.",
              [("+1 a tus golpes con arma durante 2 turnos.", estado(10, "bono_dano_arma:1")),
               _pendiente("las marcas que refuerza")]),
        carta("H366", "Propulsión por cohete", "zone_radius",
              "Cargas con la potencia de un cohete: 2d6 de daño de fuego y aturdimiento 1 turno (no ataca "
              "y se defiende con desventaja).",
              [("2d6 de daño de fuego.", dano(650, 400))] + _aturdido(2), elemento="Fuego"),
        carta("H367", "Respiración llameante", "zone_radius",
              "Lanzas bolas de fuego: 3d6 de daño de fuego, y arde: 1 de daño por turno durante 2 turnos.",
              [("3d6 de daño de fuego.", dano(500, 800)),
               ("Fuego de cohete: 1 de daño por turno durante 2 turnos.", estado(8, "dano_por_turno:1"))],
              elemento="Fuego"),
        carta("H368", "Martillo rotativo", "all_enemies",
              "El martillo gira y atrae: 3d6 de daño de fuego a cada enemigo, que te ataca a ti 1 turno.",
              [("3d6 de daño de fuego a cada enemigo.", dano(800, 600)), _atrae()], elemento="Fuego"),
        carta("H386", "Salto con cohete", "all_enemies",
              "Saltas con el cohete sobre los enemigos: 3d6 de daño de fuego a cada uno, y la protección de "
              "carga te da +1 a la CA durante 2 turnos.",
              [("3d6 de daño de fuego a cada enemigo.", dano(1200, 700)),
               ("Protección de carga: +1 a tu CA durante 2 turnos.", propio(10, "bono_ca:1"))], elemento="Fuego"),
        carta("H387", "Ataque con esquirlas", "all_enemies",
              "Las esquirlas del suelo hieren a los cercanos: 3d6 de daño de fuego a cada enemigo, y tus "
              "golpes les hacen +1 durante 2 turnos.",
              # «solo si tienes 1 marca»: sin marcas, se juega siempre
              [("3d6 de daño de fuego a cada enemigo.", dano(800, 900)),
               ("Heridas de metralla: tus golpes les hacen +1 durante 2 turnos.", estado(10, "bono_dano_contra:1"))],
              elemento="Fuego"),
    ],
    "creador": [
        carta("H388", "Impulso propulsor por cohete", "self",
              "El martillo se refuerza con mineral hasta el final del combate: tus golpes con arma hacen +1 "
              "y atacas con ventaja.",
              [("+1 a tus golpes con arma todo el combate.", estado(120, "bono_dano_arma:1")),
               ("Ventaja en ataque todo el combate.", estado(120, "ventaja:ataque"))]),
        carta("H389", "Defensa rotativa", "self",
              "El martillo gira y contrarresta los ataques de ese turno.",
              [CONTRARRESTA], accion="reaccion"),
        carta("H390", "Extracción explosiva", "all_enemies",
              "Haces estallar minerales explosivos: 3d6 de daño de fuego a cada enemigo, y tus golpes les "
              "hacen +1 durante 2 turnos.",
              [("3d6 de daño de fuego a cada enemigo.", dano(1400, 1000)),
               ("Shock extractivo: tus golpes les hacen +1 durante 2 turnos.", estado(8, "bono_dano_contra:1"))],
              elemento="Fuego"),
    ],
    "arma_inicial": "martillo_de_guerra_de_milicia",
    "set": [
        pieza("Martillo del Romperrocas", "MainHand", "pesada", {"CON": 1, "DES": 1},
              "El martillo de la SP del Romperrocas: cabeza de roca y propulsor de cohete.",
              restricciones=["Precisión: CON", DOS_MANOS]),
        pieza("Peto del Romperrocas", "Torso", "media", {"CON": 1},
              "Peto de minero con tirantes, cinturón de herramientas y hombreras de cuero."),
        pieza("Casco del Romperrocas", "Head", "media", {"CON": 1, "INT": 1},
              "Casco de minero con la lámpara y las gafas en la frente."),
        pieza("Pantalones del Romperrocas", "Legs", "ligera", {"CON": 1, "DES": 1},
              "Pantalones de trabajo."),
        pieza("Botas del Romperrocas", "Feet", "media", {"CON": 1, "DES": 1},
              "Botas de puntera de acero."),
    ],
}

AQUILES = {
    "id": "aquiles", "name": "Aquiles", "sp": 12, "elemento": "Tinieblas", "nostale": "Aquiles (Achilles)",
    "description": ("Especialista de las lanzas de niebla del Estigia: hiere de lejos, maldice y se oculta en "
                    "la niebla."),
    "primaryStat": "DES", "secondaryStat": "INT", "role": "agile",
    "pasiva": ("Lanzada",
               "Las lanzas de niebla de Aquiles: tus golpes con arma hacen +1 de daño (tinieblas).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("la sincronización dimensional (menor, simple y alta) y el eco del héroe")]),
    "cartas": [
        carta("H1952", "Lanza del olvido", "single_enemy",
              "Una lanza con la niebla oscura del Estigia: 2d6 de daño de tinieblas.",
              [("2d6 de daño de tinieblas.", dano(350, 400))], elemento="Tinieblas"),
        carta("H1953", "Niebla ascendente", "all_enemies",
              "Una niebla densa se propaga: 2d6 de daño de tinieblas a cada enemigo.",
              # 40 % de niebla negra: fuera
              [("2d6 de daño de tinieblas a cada enemigo.", dano(400, 450))], elemento="Tinieblas"),
        carta("H1954", "Maldición del olvido", "single_enemy",
              "Una lanza maldita: 2d6 de daño de tinieblas; queda petrificado 1 turno (no ataca) y la "
              "infestación le quita 1 de vida por turno durante 2 turnos.",
              [("2d6 de daño de tinieblas.", dano(600, 500)),
               ("Petrificación: 1 turno sin atacar.", estado(5, "incapacitado")),
               ("Infestado: 1 de daño por turno durante 2 turnos.", estado(15, "dano_por_turno:1"))],
              elemento="Tinieblas"),
        carta("H1955", "Niebla arrastrada", "all_enemies",
              "La niebla envuelve a los alcanzados: 3d6 de daño de tinieblas a cada enemigo, y tus golpes "
              "les hacen +1 durante 2 turnos.",
              [("3d6 de daño de tinieblas a cada enemigo.", dano(700, 700)),
               ("Maldición sigilosa: tus golpes les hacen +1 durante 2 turnos.", estado(10, "bono_dano_contra:1"))],
              elemento="Tinieblas"),
        carta("H1956", "Maldición creciente", "self",
              "La maldición del Estigia fortalece tu lanza hasta el final del combate: atacas con ventaja y "
              "tus golpes con arma hacen +1.",
              [("Ventaja en ataque todo el combate.", estado(300, "ventaja:ataque")),
               ("+1 a tus golpes con arma todo el combate.", estado(300, "bono_dano_arma:1"))]),
        carta("H1957", "Niebla impenetrable", "all_enemies",
              "La niebla te oculta: durante 2 turnos te defiendes y atacas con ventaja, y tus golpes les "
              "hacen +1.",
              # te teletransporta: sin posiciones
              [("Envuelto en niebla: te defiendes con ventaja 2 turnos.", propio(15, "ventaja:defensa")),
               ("Envuelto en niebla: atacas con ventaja 2 turnos.", propio(15, "ventaja:ataque")),
               ("Niebla maldita: tus golpes les hacen +1 durante 2 turnos.", estado(10, "bono_dano_contra:1"))]),
        carta("H1958", "Escudo de la niebla", "self",
              "La niebla contrarresta los ataques de ese turno.",
              [CONTRARRESTA, _pendiente("devolver al atacante el 20 % del daño")], accion="reaccion"),
    ],
    "creador": [
        carta("H1959", "Mar de niebla", "all_enemies",
              "Un mar de niebla sobre los enemigos: 3d6 de daño de tinieblas a cada uno, y tus golpes les "
              "hacen +1 durante 2 turnos.",
              [("3d6 de daño de tinieblas a cada enemigo.", dano(800, 900)),
               ("Niebla tóxica: tus golpes les hacen +1 durante 2 turnos.", estado(10, "bono_dano_contra:1"))],
              elemento="Tinieblas"),
        carta("H1960", "Lanza del Estigia", "all_enemies",
              "Giras la lanza y agrupas a los enemigos: 3d6 de daño de tinieblas a cada uno, que te ataca a "
              "ti 1 turno, y tus golpes les hacen +1 durante 2 turnos.",
              [("3d6 de daño de tinieblas a cada enemigo.", dano(700, 1000)), _atrae(),
               ("Niebla perforadora: tus golpes les hacen +1 durante 2 turnos.", estado(10, "bono_dano_contra:1"))],
              elemento="Tinieblas"),
        carta("H1961", "Lluvia de lanzas del Estigia", "all_enemies",
              "Una lluvia de lanzas de niebla: 4d6 de daño de tinieblas a cada enemigo, y la maldición les "
              "quita 1 de vida por turno durante 2 turnos.",
              [("4d6 de daño de tinieblas a cada enemigo.", dano(1200, 1500)),
               ("Maldición de las nubes: 1 de daño por turno durante 2 turnos.", estado(10, "dano_por_turno:1"))],
              elemento="Tinieblas"),
    ],
    "arma_inicial": "ballesta_ligera_de_viaje",
    "set": [
        pieza("Ballesta de Aquiles", "MainHand", "media", {"DES": 1, "INT": 1},
              "La ballesta de la SP de Aquiles: bronce oscuro y niebla del Estigia."),
        pieza("Coraza de Aquiles", "Torso", "ligera", {"DES": 1},
              "Coraza de bronce oscuro con el manto violeta del Estigia."),
        pieza("Yelmo de Aquiles", "Head", "ligera", {"DES": 1, "CAR": 1},
              "Yelmo de bronce con la cresta negra."),
        pieza("Grebas de Aquiles", "Legs", "ligera", {"DES": 1, "CON": 1},
              "Grebas de bronce con la pierna al aire."),
        pieza("Sandalias de Aquiles", "Feet", "ligera", {"DES": 1, "INT": 1},
              "Sandalias de cuero oscuro."),
    ],
}

ESPADACHIN = [GUERRERO, NINJA, CRUZADO, BERSERKER, GLADIADOR, MONJE, COSECHADOR, RENEGADO, HACHERO, CABALLERO,
              ROMPERROCAS, AQUILES]
