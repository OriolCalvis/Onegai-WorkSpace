"""SP-02 — Las ocho SP del Artista Marcial de NosTale, como clases de MotorGraphico (los datos).

Cada carta dice de que habilidad sale (su H en la enciclopedia de SP de Oriol) y sus cifras pasan por
las reglas de sp_reglas.py (planificacion/CANON_SP_CLASES.md): los dados por la bonificacion de la
ficha (cuerpo a cuerpo o a distancia + elemental, sumando los pasos de una continuacion), las rondas
por los segundos, los estados con un 50 % o mas. Cuando una carta se aparta de la regla, lo dice un
comentario. Lo escribe en los catalogos tools/importar_clases_sp.py.

Con que se tira (§7): todas sus tecnicas son «cuerpo a cuerpo» o «a distancia» (el Tronador dispara el
canon de descargas del guantelete izquierdo; Thor, el guante de aliento helado): DES, lo de siempre.

El arma (Oriol, 07/10: «Guanteletes grandes», uno en cada mano): el catalogo no tenia guanteletes; los
de Viaje son los normales de tier 1, a dos manos. Se ven en los dos punos (actores_formas: guanteletes,
garras, brazos mecanicos y el canon de descargas del Tronador; actores_armas.PAREJA).

Criterios comunes a las ocho (ademas de los del Espadachin, el Arquero y el Mago):
- Las formas son la pasiva (§6), como el Monje de lucha: los dos estilos del Puno del dragon (haetae y
  dragon) y las tres figuras del Druida flamigero (druida, oso y leopardo). Las tecnicas de todas las
  formas se juegan siempre; un segundo basico de otra forma (el Yoyo de llamas, el Ataque zarpazo) no es
  carta.
- Barras y recursos (los puntos definitivos del Maestro Lobo, el combustible del Puno Hidraulico, las
  marcas del Tronador y de Thor, la sincronizacion dimensional de Thor, la Posibilidad de ataque y la
  Iluminacion de las Artes Misticas): lo que los pide se juega siempre; lo que da tenerlos o gastarlos,
  fuera, y la pasiva lo deja dicho (pendiente).
- Cuando sobran cartas, se funden las que hacen lo mismo (los mismos dados, el mismo golpe): la carta
  junta lo que hacen las dos y lleva las H de las dos («H1578/H1588»); una fusion asi no suma
  bonificaciones (eso solo lo hacen las continuaciones, «H1756+H1758»). Las variantes definitivas del
  Maestro Lobo van con su tecnica, con las cifras de la normal.
- Las ventanas de defensa de 2 s (Bloqueo, Contraalas, Llama absorbente, Bloqueo frontal, Escudo de
  electrones, Escudo helado, Aguante) son reacciones que contrarrestan; lo que ganan al saltar dura 2
  rondas (§4).
- «Ataque no posible» (con o sin «no es posible mover»): sin atacar (incapacitado); «no es posible mover»
  solo, fuera (no hay posiciones).
- La vida que quitan las descargas, las quemaduras y las maldiciones cada 2 s: dano_por_turno; la de las
  heridas y los venenos: sangrado.
- El dano de los criticos (los propios y los que recibe el enemigo), fuera, como en el Mago.
"""
from sp_reglas import (CONTRARRESTA, DOS_MANOS, arma_comun, carta, condicion, dano, elige_una, estado, pieza,
                       propio, si, siempre)
from sp_reglas import atrae as _atrae
from sp_reglas import aturdido as _aturdido
from sp_reglas import pendiente as _pendiente

# El arma normal de tier 1 (Oriol: «guanteletes grandes», uno en cada mano; el catalogo no tenia ninguno):
# la traen las ocho SP.
GUANTELETES = arma_comun("Guanteletes de Viaje", "media", "Un par de guanteletes de cuero y hierro, uno en cada "
                         "mano.", restricciones=["Precisión: DES", DOS_MANOS])


PUNO_DEL_DRAGON = {
    "id": "puno_del_dragon", "name": "Puño del dragón", "sp": 1, "elemento": "Fuego",
    "nostale": "Puño del dragón (Dragon Knuckles)",
    "description": ("Especialista de los guanteletes de fuego: cambia entre el estilo del haetae, que muerde y "
                    "desgarra con el puño de la bestia, y el del dragón, que quema con sus llamas hasta la llama "
                    "final."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "balanced",
    # Los dos estilos (las dos Transformación: H1585 al dragón y H1594 al haetae) son la forma de la SP: la
    # pasiva (§6), como las meditaciones del Monje de lucha. Haetae: la Energía de Haetae que le da su
    # Transformación (el acierto +(N x2) y el crítico +10 %: ventaja al atacar). Dragón: los ataques llameantes
    # (+1, fuego); el +20 al elemento del hada y la resistencia al fuego del rival -5, fuera. Las técnicas de los
    # dos estilos se juegan siempre. El ataque básico (Golpe, H1577: la cadena de tres golpes) es Atacar con los
    # guanteletes, y el Yoyó de llamas (H1586), el básico del estilo dragón (+20 +120; la quemadura leve al 8 %,
    # fuera), es el +1 llameante de la forma dragón.
    "pasiva": ("Transformación",
               "Al empezar el combate eliges tu estilo. Haetae: la energía del haetae te hace atacar con ventaja. "
               "Dragón: tus golpes con arma son llameantes y hacen +1 de daño (fuego).",
               [("Eliges un estilo al empezar el combate.", elige_una("haetae", "dragon")),
                ("Estilo haetae: atacas con ventaja.", si("haetae", "ventaja:ataque")),
                ("Estilo dragón: tus golpes con arma hacen +1 de daño.", si("dragon", "bono_dano_arma:1"))]),
    # Trece técnicas para diez cartas: se funden las que hacen lo mismo. Puño de la bestia y Golpe llameante (2d6
    # de fuego y nada más), Golpe de nudillos y Garra ígnea (2d6 de fuego y 1 turno sin atacar), y los dos saltos
    # (el salto de cada estilo).
    "cartas": [
        # Golpe llameante (H1588: +70 +700, los mismos 2d6) hace lo mismo: una carta. El aturdimiento corto del
        # puño (15 %) y el deterioro de las armas de defensa del golpe (35 %): fuera; la quemadura leve del golpe
        # (60 %): la resistencia al fuego y +20 % de golpe fatal, que no llega al 50 %: nada que entre
        carta("H1578/H1588", "Puño de la bestia", "zone_radius",
              "Golpeas con el gran puño de la bestia y del impulso surgen llamas: 2d6 de daño de fuego.",
              [("2d6 de daño de fuego.", dano(700, 30))], elemento="Fuego"),
        # le quita (N x5) HP y te los da (80 %): robar vida
        carta("H1579", "Arañazo", "zone_radius",
              "Un zarpazo salvaje con las garras del puño de la bestia: 1d6 de daño de fuego, y te curas el daño "
              "que haces.",
              [("1d6 de daño de fuego.", dano(450, 35)), ("Te curas el daño que haces.", "cura:dano_causado")],
              elemento="Fuego"),
        # Mano de la Muerte (60 %, 5 s), como el Tiro maldito del Mago de las tinieblas: «ataque no posible» y el
        # daño recibido +15 %; no moverse, fuera. Es de efecto mágico, pero la carta hace daño: golpe, sin
        # salvación. «Se fijarán en ti» no suma nada: con la mano de la muerte no atacan ese turno (como los ganchos
        # de cadena del Asesino)
        carta("H1580", "Agarre", "all_enemies",
              "Agarras a los enemigos con el puño de la bestia: 2d6 de daño de fuego a cada uno, y la mano de la "
              "muerte los deja 1 turno sin atacar y hace que tus golpes les hagan +1.",
              [("2d6 de daño de fuego a cada enemigo.", dano(500, 35)),
               ("Mano de la Muerte: 1 turno sin atacar.", estado(5, "incapacitado")),
               ("Mano de la Muerte: tus golpes les hacen +1 durante 1 turno.", estado(5, "bono_dano_contra:1"))],
              elemento="Fuego"),
        # Energía de Haetae (100 %, 180 s): el acierto +(N x2) y el crítico +10 % (ventaja) y el ataque cuerpo a
        # cuerpo +(N x2), que no llega a +200 (§5) hasta el nivel 100: +1, como la Hipergravedad de Grávitas. Es
        # también lo que da la forma haetae (la pasiva)
        carta("H1581", "Energía de Haetae", "self",
              "La energía del haetae mejora tu cuerpo hasta el final del combate: atacas con ventaja y tus golpes "
              "con arma hacen +1.",
              [("Energía de Haetae: ventaja en ataque todo el combate.", estado(180, "ventaja:ataque")),
               ("Energía de Haetae: +1 a tus golpes con arma todo el combate.", estado(180, "bono_dano_arma:1"))]),
        # Salto del haetae y Salto del dragón (H1591) son el salto de cada estilo: una carta. La carrera de la
        # bestia (velocidad +3) y el teletransporte, sin posiciones; el parpadeo de calor (100 %, 10 s, tuyo): la
        # defensa +500 y el nivel de defensa +1, +1 a la CA
        carta("H1582/H1591", "Salto del haetae", "single_enemy",
              "Saltas sobre el enemigo con la carrera de la bestia y te envuelve el parpadeo de calor del dragón: "
              "1d6 de daño de fuego y +1 a tu CA durante 2 turnos.",
              [("1d6 de daño de fuego.", dano(250, 25)),
               ("Parpadeo de calor: +1 a tu CA durante 2 turnos.", propio(10, "bono_ca:1"))], elemento="Fuego"),
        # Hemorragia Mortal (80 %, 10 s): la vida que pierde cada 2 s, sangra (como en el Berserker); no hacer
        # golpes fatales y la magia -20 %, fuera
        carta("H1583", "Morder", "zone_radius",
              "Muerdes con el puño de la bestia: 2d6 de daño de fuego, y sangra 2 turnos.",
              [("2d6 de daño de fuego.", dano(650, 40)),
               ("Hemorragia mortal: sangra 2 turnos.", condicion("sangrado", 10))], elemento="Fuego"),
        # Energía del dragón (100 %, 180 s): fuego +(N x3), +200 o más desde el nivel 67: +1d6, como las
        # bendiciones de grupo del Mago; la resistencia al fuego +10, fuera
        carta("H1590", "Energía del dragón", "all_allies",
              "La energía del dragón aviva el fuego del grupo: tú y tus aliados hacéis +1d6 con los golpes de arma "
              "hasta el final del combate.",
              [("Energía del dragón: +1d6 a los golpes con arma todo el combate.", estado(180, "bono_dano_arma:1d6"))]),
    ],
    # Creador: la tarjeta no llega a los niveles 12, 16 ni 20. Las que enseña al final son las de nivel 7 (las del
    # estilo dragón) y, de ellas, las de más peso (la bonificación): Llama final (+2500), Garra ígnea (+1285) y
    # Fuego en cruz (+875). Golpe de nudillos (+857) va con Garra ígnea; Golpe llameante (+770), con Puño de la
    # bestia; Salto del dragón, con Salto del haetae; Energía del dragón, estándar.
    "creador": [
        # Golpe de nudillos (H1587: +80 +777, los mismos 2d6) hace lo mismo: una carta. Su aturdimiento corto
        # (80 %, 2 s) cubre la rigidez de la garra (60 %, 3 s): los dos, 1 turno. Las quemaduras (la de los
        # nudillos al 40 %; la fatal de la garra, 60 %: resistencia al fuego y +30 % de golpe fatal): nada que entre
        carta("H1589/H1587", "Garra ígnea", "zone_radius",
              "Atizas al enemigo con los nudillos y lo desgarras con la garra ardiente: 2d6 de daño de fuego y "
              "aturdimiento 1 turno (no ataca y se defiende con desventaja).",
              [("2d6 de daño de fuego.", dano(85, 1200))] + _aturdido(2), elemento="Fuego"),
        # Quemadura Fatal (60 %, 30 s): la vida que pierde cada 2 s; la resistencia al fuego -25, fuera
        carta("H1592", "Fuego en cruz", "zone_radius",
              "Formas una cruz de fuego alrededor del enemigo: 2d6 de daño de fuego, y arde: 1 de daño por turno "
              "durante 3 turnos.",
              [("2d6 de daño de fuego.", dano(75, 800)),
               ("Quemadura fatal: 1 de daño por turno durante 3 turnos.", estado(30, "dano_por_turno:1"))],
              elemento="Fuego"),
        # Las tres Llama final (50 % cada una, 3 s) llegan al 50 %: «ataque no posible» de la segunda y «no hay
        # defensa posible» de la tercera. No moverse y el golpe fatal +10 % de la primera, el daño de los críticos
        # que reciben y la resistencia al fuego, fuera
        carta("H1593", "Llama final", "all_enemies",
              "Acabas con los enemigos con unas llamas: 4d6 de daño de fuego a cada uno, que queda 1 turno sin "
              "atacar y se defiende con desventaja.",
              [("4d6 de daño de fuego a cada enemigo.", dano(500, 2000)),
               ("Llama final: 1 turno sin atacar.", estado(3, "incapacitado")),
               ("Llama final: se defienden con desventaja 1 turno.", estado(3, "desventaja:defensa"))],
              elemento="Fuego"),
    ],
    "arma_inicial": "guanteletes_de_viaje",
    "armas_nuevas": [GUANTELETES],
    "set": [
        pieza("Guanteletes del Puño del dragón", "MainHand", "media", {"DES": 1, "CON": 1},
              "Los guanteletes de la SP del Puño del dragón: granates, con los nudillos de oro y una gema de fuego "
              "en el dorso.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Chaqueta del Puño del dragón", "Torso", "media", {"DES": 1},
              "Chaqueta de kung-fu roja, con el cuello de oro, los botones de nudo, la faja negra y el dragón de oro "
              "bordado a la espalda."),
        pieza("Cinta del Puño del dragón", "Head", "media", {"DES": 1, "CON": 1},
              "Cinta roja en la frente, con la placa de oro y las puntas al viento."),
        pieza("Pantalones del Puño del dragón", "Legs", "media", {"DES": 1, "CON": 1},
              "Pantalones anchos y negros, atados con cintas rojas en los tobillos."),
        pieza("Zapatillas del Puño del dragón", "Feet", "media", {"DES": 1, "CON": 1},
              "Zapatillas de tela negra del Puño del dragón."),
    ],
}

ARTES_MISTICAS = {
    "id": "artes_misticas", "name": "Artes Místicas", "sp": 2, "elemento": "Agua",
    "nostale": "Artes Místicas (Mystic Arts)",
    "description": ("Especialista de los guanteletes de agua: esquiva y aguanta para contraatacar, se prepara con la "
                    "luz de la luna y la flor de loto y siembra trampas de loto bajo los enemigos."),
    "primaryStat": "DES", "secondaryStat": "INT", "role": "agile",
    # La Posibilidad de ataque (la da Aguante) y la Iluminación (la da la Zancada) son el recurso de la SP: las
    # técnicas se juegan siempre con lo que dice su ficha; lo de más con ellas (la sombra o el maná de las de
    # cuarto creciente, la luna llena, repetir el Salto de flor de loto, más flores, que estallen, la mejora de más
    # de Entrada de luz lunar y Posición floral), fuera.
    "pasiva": ("Palmada",
               "Los golpes de palma de las Artes Místicas: tus golpes con arma hacen +1 de daño (agua).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("la Posibilidad de ataque, que da Aguante y refuerza las técnicas de la luna y del loto, "
                           "y la Iluminación, que da la Zancada y refuerza Entrada de luz lunar y Posición floral")]),
    # Catorce técnicas para diez cartas: se funden Mil pisotones con Onda expansiva y Corte de cuarto creciente con
    # Baile salvaje de cuarto creciente (los mismos dados), Semilla de loto con Trampa de loto (la que hace estallar
    # sus flores) y la Zancada con Aguante (las dos ventanas de defensa).
    "cartas": [
        # Mil pisotones (H1607: +500 +300, los mismos 2d6) hace lo mismo y añade Ligero cual pluma (60 %, 8 s,
        # tuyo): la esquiva +100, te defiendes con ventaja; la velocidad y el teletransporte, fuera. Las heridas
        # internas de la onda (40 %): fuera
        carta("H1606/H1607", "Onda expansiva", "zone_radius",
              "Golpeas rapidísimo las piernas del enemigo y lo aplastas a pisotones: 2d6 de daño de agua y, ligero "
              "cual pluma, te defiendes con ventaja 2 turnos.",
              [("2d6 de daño de agua.", dano(300, 300)),
               ("Ligero cual pluma: te defiendes con ventaja 2 turnos.", propio(8, "ventaja:defensa"))],
              elemento="Agua"),
        # Entrada de luz lunar (100 %, 300 s), un potenciador propio: el nivel de ataque +1 (+1) y el acierto
        # +(N x2) (ventaja). Las técnicas de la luna llena que abre se juegan siempre; la mejora de más con
        # Iluminación (+20 % a todos los ataques), fuera
        carta("H1609", "Entrada de luz lunar", "self",
              "La luz de la luna entra en ti hasta el final del combate: atacas con ventaja y tus golpes con arma "
              "hacen +1.",
              [("Entrada de luz lunar: ventaja en ataque todo el combate.", estado(300, "ventaja:ataque")),
               ("Entrada de luz lunar: +1 a tus golpes con arma todo el combate.", estado(300, "bono_dano_arma:1"))]),
        # Posición floral (100 %, 300 s), un potenciador propio: el nivel de defensa +1 (+1 a la CA) y el acierto
        # +(N x2) (ventaja), las dos que más pesan; el golpe fatal recibido -20 %, fuera. Las técnicas de la flor
        # de loto que abre se juegan siempre; la mejora de más con Iluminación, fuera
        carta("H1610", "Posición floral", "self",
              "Adoptas la posición floral hasta el final del combate: tienes +1 a la CA y atacas con ventaja.",
              [("Posición floral: +1 a la CA todo el combate.", estado(300, "bono_ca:1")),
               ("Posición floral: ventaja en ataque todo el combate.", estado(300, "ventaja:ataque"))]),
        # Corte de cuarto creciente (H1612: +700 +900, los mismos 3d6) hace lo mismo: una carta. Su robo de maná
        # (50 %) y lo de la Posibilidad de ataque (la Sombra de cuarto creciente, robar 75 MP por nivel): fuera
        carta("H1611/H1612", "Baile salvaje de cuarto creciente", "all_enemies",
              "Una danza salvaje y un corte como el cuarto creciente: 3d6 de daño de agua a cada enemigo.",
              [("3d6 de daño de agua a cada enemigo.", dano(700, 700))], elemento="Agua"),
        # Marca lunar (100 %, 20 s): tu próximo golpe le hace un 25 % más (marcado: tus golpes le hacen +1); el
        # daño de los críticos que recibe, fuera. Sombra de cuarto creciente (100 %, 30 s, tuya): «entrar a
        # hurtadillas», te ocultas: te defiendes con ventaja. Es «efecto negativo», no mágico: sin salvación. La
        # Marca de la luna llena (con Posibilidad de ataque) y el Ligado por el poder de la luna llena, fuera
        carta("H1614", "Eclipse lunar", "single_enemy",
              "Eclipsas la luna sobre el enemigo, que queda marcado 3 turnos, y la sombra de cuarto creciente te "
              "oculta: te defiendes con ventaja 3 turnos.",
              [("Marca lunar: marcado 3 turnos.", condicion("marcado", 20)),
               ("Sombra de cuarto creciente: te defiendes con ventaja 3 turnos.", propio(30, "ventaja:defensa"))]),
        # el salto, sin posiciones; repetirlo con la Posibilidad de ataque, fuera
        carta("H1617", "Salto de flor de loto", "zone_radius",
              "Saltas sobre los enemigos como una flor de loto: 3d6 de daño de agua.",
              [("3d6 de daño de agua.", dano(700, 800))], elemento="Agua"),
        # Semilla de loto (H1618) solo invoca dos Flores de loto, minas que no atacan: no es invocación; se funde
        # con Trampa de loto, que las hace estallar (no suma: la semilla no tiene bonificación). Minas florales
        # (70 %, 3 s): no moverse y «ataque no posible», y recuperar menos vida (pendiente); los MP, fuera. Que las
        # flores estallen con la Posibilidad de ataque (+100 x N), fuera
        carta("H1619/H1618", "Trampa de loto", "all_enemies",
              "Siembras flores de loto a tu alrededor y estallan en una trampa: 3d6 de daño de agua a cada enemigo, y "
              "las minas florales los dejan 1 turno sin atacar.",
              [("3d6 de daño de agua a cada enemigo.", dano(700, 1000)),
               ("Minas florales: 1 turno sin atacar.", estado(3, "incapacitado")),
               _pendiente("que recuperen menos vida mientras duran las minas florales")], elemento="Agua"),
    ],
    # Creador: las de profesión 12, 16 y 20. Dos de 12 y dos de 16: la de más peso (la bonificación) es de
    # Creador y la otra, estándar: Poder de la luz lunar (+2100) antes que Trampa de loto (+1700), En flor (+3500)
    # antes que Eclipse lunar (sin daño). Aguante (20) lleva dentro la Zancada.
    "creador": [
        # Ligado por el poder de la luz lunar (100 %, 6 s): no moverse (sin posiciones) y +20 % de golpe fatal, que
        # no llega al 50 %: nada que entre. Lo de la Posibilidad de ataque (la luna llena), fuera
        carta("H1613", "Poder de la luz lunar", "single_enemy",
              "El poder de la luz de la luna cae sobre el enemigo: 3d6 de daño de agua.",
              [("3d6 de daño de agua.", dano(900, 1200))], elemento="Agua"),
        # Maldición de flor de loto (70 %, 5 s): no moverse y «ataque no posible», y apenas recupera vida
        # (pendiente); los MP, fuera. Lo de la Posibilidad de ataque (más maldición, reactivar las minas), fuera
        carta("H1620", "En flor", "zone_radius",
              "Las flores de loto se abren sobre el enemigo: 4d6 de daño de agua, y la maldición de la flor de loto "
              "le deja 1 turno sin atacar.",
              [("4d6 de daño de agua.", dano(2000, 1500)),
               ("Maldición de flor de loto: 1 turno sin atacar.", estado(5, "incapacitado")),
               _pendiente("que apenas recupere vida mientras dura la maldición")], elemento="Agua"),
        # Las dos ventanas de defensa de la SP hacen lo mismo: Aguante («defensa reactiva: usarla durante el ataque
        # enemigo») y la Zancada (H1608: esquivas los ataques 3 s). Una reacción (§4): «no se consumen HP» y no
        # recibir efectos negativos los cubre el contrarrestar; lo que dan, la Posibilidad de ataque y la
        # Iluminación, es el recurso (la pasiva)
        carta("H1621/H1608", "Aguante", "self",
              "Aguantas el ataque o lo esquivas de una zancada: contrarrestas los ataques de ese turno.",
              [CONTRARRESTA], accion="reaccion"),
    ],
    "arma_inicial": "guanteletes_de_viaje",
    "armas_nuevas": [GUANTELETES],
    # El set es «de las Artes Místicas» (femenino plural: sp_reglas.etiqueta_set).
    "set": [
        pieza("Guanteletes de las Artes Místicas", "MainHand", "media", {"DES": 1, "INT": 1},
              "Los guanteletes de la SP de las Artes Místicas: azules y blancos, con la flor de loto en el dorso.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Túnica de las Artes Místicas", "Torso", "ligera", {"DES": 1},
              "Túnica blanca, cruzada con la solapa azul, de mangas anchas y con la faja azul de flores de loto."),
        pieza("Diadema de las Artes Místicas", "Head", "ligera", {"DES": 1, "INT": 1},
              "Diadema de plata con la luna creciente."),
        pieza("Pantalones de las Artes Místicas", "Legs", "ligera", {"DES": 1, "INT": 1},
              "Pantalones anchos y azules, atados en los tobillos."),
        pieza("Sandalias de las Artes Místicas", "Feet", "ligera", {"DES": 1, "INT": 1},
              "Sandalias de cuero de las Artes Místicas."),
    ],
}

MAESTRO_LOBO = {
    "id": "maestro_lobo", "name": "Maestro Lobo", "sp": 3, "elemento": "Luz", "nostale": "Maestro Lobo (Master Wolf)",
    "description": ("Especialista de los guanteletes de luz: patadas, ganchos y bolas de energía que acumulan puntos "
                    "definitivos, un bloqueo que los multiplica y el estilo de combate definitivo para el remate."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "balanced",
    # El ataque básico (H645) se llama así, «Ataque básico»: la pasiva toma el nombre de su ficha («un golpe duro
    # seguido de una patada»), como el Bombardeo del Cañonero. Los puntos definitivos no se simulan: los que piden
    # las versiones definitivas se juegan siempre (cada carta lleva la suya en pendiente).
    "pasiva": ("Golpe y patada",
               "El golpe duro y la patada del Maestro Lobo: tus golpes con arma hacen +1 de daño (luz).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("los puntos definitivos (los dan sus golpes y el Bloqueo, y las versiones definitivas de "
                           "sus técnicas piden 1000)")]),
    # Las cuatro variantes definitivas (H656-H659) van con su técnica: la carta lleva las cifras de la normal y la
    # definitiva queda en pendiente. Los puntos definitivos que da cada golpe, la pasiva.
    "cartas": [
        # derribo (30 %): fuera
        carta("H646", "Patada baja", "zone_radius",
              "Una patada giratoria por abajo: 2d6 de daño de luz.",
              [("2d6 de daño de luz.", dano(350, 300))], elemento="Luz"),
        # Noqueo (50 %, 4 s): «ataque no posible», como el aturdimiento (§5); no moverse y +30 % de golpe fatal,
        # fuera. El salto, sin posiciones
        carta("H647", "Patada con salto", "single_enemy",
              "Saltas hacia el enemigo y le pateas desde el aire: 2d6 de daño de luz y noqueo 1 turno (no ataca y se "
              "defiende con desventaja).",
              [("2d6 de daño de luz.", dano(600, 600))] + _aturdido(4, "Noqueo"), elemento="Luz"),
        # derribo (35 %): fuera. La Bola de energía definitiva (H656: +1200 +1600, derribo al 75 % y Carrera con la
        # muerte, que sube el daño de las definitivas), su variante
        carta("H648/H656", "Bola de energía", "zone_radius",
              "Lanzas un proyectil de energía contra el enemigo: 2d6 de daño de luz.",
              [("2d6 de daño de luz.", dano(400, 800)),
               _pendiente("la versión definitiva, con 1000 puntos definitivos")], elemento="Luz"),
        # «se fijarán en ti». La Patada giradora definitiva (H657: +1600 +1400, en 6 casillas, y Carrera con la
        # muerte), su variante
        carta("H649/H657", "Patada Giradora", "all_enemies",
              "Una patada giratoria de varios movimientos: 3d6 de daño de luz a cada enemigo, que te ataca a ti 1 "
              "turno.",
              [("3d6 de daño de luz a cada enemigo.", dano(800, 700)), _atrae(),
               _pendiente("la versión definitiva, con 1000 puntos definitivos")], elemento="Luz"),
        # Noqueo (75 %, 4 s), como en la Patada con salto; las Tinieblas mortales (hasta un 40 % según la vida que
        # le falta), fuera. El Gancho hacia delante definitivo (H658: +700 +1600, noqueo seguro), su variante
        carta("H650/H658", "Gancho hacia delante", "zone_radius",
              "Lanzas un gancho mientras saltas: 2d6 de daño de luz y noqueo 1 turno (no ataca y se defiende con "
              "desventaja).",
              [("2d6 de daño de luz.", dano(400, 800))] + _aturdido(4, "Noqueo")
              + [_pendiente("la versión definitiva, con 1000 puntos definitivos")], elemento="Luz"),
        # Ventana de defensa de 2 s (§4): «no se consumen HP» es lo que hace la reacción, y no recibir efectos
        # negativos lo cubre el contrarrestar; los 1000 puntos definitivos de bloquear, la pasiva
        carta("H651", "Bloqueo", "self",
              "Te cubres de los ataques enemigos: contrarrestas los ataques de ese turno.",
              [CONTRARRESTA], accion="reaccion"),
        # Energía sacra (100 %, 300 s): el acierto +(N x2) (ventaja) y los ataques cuerpo a cuerpo +5 % (+1). La
        # descripción habla de reforzar a tus aliados, pero la ficha dice «el propio personaje»: a ti
        carta("H652", "Expulsión energética", "self",
              "Expulsas andanadas de energía purificadora: hasta el final del combate atacas con ventaja y tus golpes "
              "con arma hacen +1.",
              [("Energía sacra: ventaja en ataque todo el combate.", estado(300, "ventaja:ataque")),
               ("Energía sacra: +1 a tus golpes con arma todo el combate.", estado(300, "bono_dano_arma:1"))]),
    ],
    # Creador: las de profesión 12, 16 y 20.
    "creador": [
        # Estilo de combate definitivo (100 %, 8 s): las variantes definitivas tienen un «índice de acierto de
        # 100 %»: atacas con ventaja; las versiones definitivas mismas, pendiente
        carta("H653", "Estilo de combate definitivo", "self",
              "Adoptas el estilo de combate definitivo: tus técnicas no fallan y atacas con ventaja 2 turnos.",
              [("Estilo de combate definitivo: atacas con ventaja 2 turnos.", propio(8, "ventaja:ataque")),
               _pendiente("las versiones definitivas de sus técnicas")]),
        # Meditación de combate (100 %, 7 s): quita los efectos dañinos de nivel menor que 4 (limpia) y no deja que
        # te pongan otros (pendiente); la defensa contra los elementos +30, fuera
        carta("H654", "Voluntad del buscaverdades", "self",
              "Meditas y tu espíritu se fortalece: te libras de los estados en contra.",
              [("Te libras de los estados en contra.", "limpia"),
               _pendiente("que no te pongan estados en contra durante 2 turnos")]),
        # Tinieblas mortales (75 %, 5 s): no moverse y «ataque no posible». Impulso fuerte (100 %, 4 s): solo no
        # moverse (sin posiciones), y la Herida mortal que puede dejar no tiene ficha: fuera. El Combo triple
        # definitivo (H659: +3000 +2000, con 1000 puntos y el tercer efecto definitivo), su variante
        carta("H655/H659", "Combo triple", "single_enemy",
              "Encadenas la patada con salto, la bola de energía y el gancho: 3d6 de daño de luz, y las tinieblas "
              "mortales le dejan 1 turno sin atacar.",
              [("3d6 de daño de luz.", dano(1200, 1000)),
               ("Tinieblas mortales: 1 turno sin atacar.", estado(5, "incapacitado")),
               _pendiente("la versión definitiva, con 1000 puntos definitivos")], elemento="Luz"),
    ],
    "arma_inicial": "guanteletes_de_viaje",
    "armas_nuevas": [GUANTELETES],
    "set": [
        pieza("Guanteletes del Maestro Lobo", "MainHand", "media", {"DES": 1, "CON": 1},
              "Los guanteletes de la SP del Maestro Lobo: de piel gris oscuro, con tres garras de lobo de plata.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Chaleco del Maestro Lobo", "Torso", "media", {"DES": 1},
              "Chaleco de piel gris oscuro, abierto sobre el pecho vendado, con el cuello de piel, la faja blanca y "
              "los brazos vendados."),
        pieza("Capucha de lobo del Maestro Lobo", "Head", "media", {"DES": 1, "CON": 1},
              "La cabeza de un lobo gris, con las orejas, como capucha."),
        pieza("Pantalones del Maestro Lobo", "Legs", "media", {"DES": 1, "CON": 1},
              "Pantalones gris pizarra del Maestro Lobo."),
        pieza("Botas del Maestro Lobo", "Feet", "media", {"DES": 1, "CON": 1},
              "Botas vendadas del Maestro Lobo."),
    ],
}


GUERRERO_DEMONIACO = {
    "id": "guerrero_demoniaco", "name": "Guerrero demoníaco", "sp": 4, "elemento": "Tinieblas",
    "nostale": "Guerrero demoníaco (Demon Warrior)",
    "description": ("Especialista de las garras de tinieblas: cae en picado sobre los enemigos, los envuelve en "
                    "murciélagos, devuelve los golpes con las alas y remata con el rayo del Juicio de Lucifer."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "agile",
    "pasiva": ("Arañazo",
               "El arañazo salvaje de las garras del Guerrero demoníaco: tus golpes con arma hacen +1 de daño "
               "(tinieblas).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H718", "Picado", "zone_radius",
              "Te elevas y caes en picado sobre el enemigo con las garras: 2d6 de daño de tinieblas.",
              # Absorción de sangre suave (20 %): fuera
              [("2d6 de daño de tinieblas.", dano(400, 300))], elemento="Tinieblas"),
        # Bandada de murciélagos (100 %, 10 s): «no se consumen HP» no es aquí una ventana de 2 s: te defiendes con
        # ventaja, como la Transformación en espíritu del Sacerdote vudú; y (N x5) HP cada 2 s (N x25 en 10 s: cura
        # 1d6, de una vez). La velocidad +3, fuera
        carta("H719", "Bandada de murciélagos", "self",
              "Te conviertes en una bandada de murciélagos: te defiendes con ventaja 2 turnos y te curas 1d6.",
              [("Bandada de murciélagos: te defiendes con ventaja 2 turnos.", estado(10, "ventaja:defensa")),
               ("Te curas 1d6.", "cura:1d6")]),
        carta("H720", "Tormenta de tinieblas", "all_enemies",
              "Giras en círculo y conjuras murciélagos que atacan a todos los que te rodean: 2d6 de daño de tinieblas "
              "a cada enemigo.",
              # Absorción de sangre suave (10 %): fuera
              [("2d6 de daño de tinieblas a cada enemigo.", dano(400, 800))], elemento="Tinieblas"),
        carta("H721", "Fuego demoníaco", "zone_radius",
              "Disparas un rayo rojo con los ojos: 3d6 de daño de tinieblas.",
              # Terror de la noche (30 %, y es la resistencia a las tinieblas): fuera
              [("3d6 de daño de tinieblas.", dano(900, 1000))], elemento="Tinieblas"),
        carta("H722", "Patada giratoria doble", "single_enemy",
              "Una patada giratoria en salto y una patada baja, seguidas: 3d6 de daño de tinieblas.",
              # Cadenas (20 %): fuera
              [("3d6 de daño de tinieblas.", dano(800, 900))], elemento="Tinieblas"),
        # Ventana de defensa de 2 s (§4). Rebote (100 %, 2 s): «no se consumen HP» es lo que hace la reacción, y
        # devuelve el daño recibido (hasta N x20): refleja, 2 rondas como lo que gana toda reacción. Devolver el efecto
        # maligno del rival «con una probabilidad determinada» (sin cifra), fuera
        carta("H723", "Contraalas", "self",
              "Esquivas con las alas: contrarrestas los ataques de ese turno y, durante 2 turnos, devuelves los golpes "
              "a quien te ataca.",
              [CONTRARRESTA, ("Rebote: reflejas los golpes físicos 2 turnos.", "propio:2:refleja:fisico")],
              accion="reaccion"),
        # Quitarle (N x5) HP y dártelos (50 %): llega al 50 %, te curas el daño que haces. Cadenas (50 %, 6 s): pierde
        # un 10 % del daño que causa y te da maná con el que recibe; el maná no se simula y el combate no tiene cómo
        # cobrarle sus golpes: pendiente
        carta("H724", "Cadenas sangrientas", "zone_radius",
              "Disparas cadenas de sangre que rodean al enemigo: 2d6 de daño de sangre, y te curas el daño que "
              "haces.",
              [("2d6 de daño de sangre.", dano(0, 800)), ("Te curas el daño que haces.", "cura:dano_causado"),
               _pendiente("las Cadenas: encadenado, pierde vida al golpear (un 10 % del daño que causa)")],
              elemento="Sangre"),
    ],
    # Creador: las de profesión 12, 16 y 20.
    "creador": [
        carta("H725", "Muerte desde arriba", "all_enemies",
              "Caes desde el aire sobre los enemigos: 3d6 de daño de tinieblas a cada uno.",
              # Antigravedad (80 %): solo «no es posible mover»; sin posiciones, nada
              [("3d6 de daño de tinieblas a cada enemigo.", dano(1000, 1000))], elemento="Tinieblas"),
        # Poderes demoníacos (100 %, 60 s): todos los ataques +5 % (+1) y todas las defensas +5 % (+1 a la CA); el 2 %
        # de más por cada penalización que lleves, fuera
        carta("H726", "Poderes demoníacos", "all_allies",
              "Imbuyes al grupo de poder demoníaco: durante 5 turnos, tú y tus aliados tenéis +1 a la CA y vuestros "
              "golpes con arma hacen +1.",
              [("Poderes demoníacos: +1 a los golpes con arma durante 5 turnos.", estado(60, "bono_dano_arma:1")),
               ("Poderes demoníacos: +1 a la CA durante 5 turnos.", estado(60, "bono_ca:1"))]),
        carta("H727", "Juicio de Lucifer", "zone_radius",
              "Te alzas en el aire y disparas un rayo poderoso: 3d6 de daño de tinieblas.",
              # Hace que desaparezcan mundos (100 %, 20 s): la resistencia a las tinieblas -30 % y +20 % de recibir
              # efectos dañinos (no llega al 50 %): nada que entre
              [("3d6 de daño de tinieblas.", dano(1200, 1200))], elemento="Tinieblas"),
    ],
    "arma_inicial": "guanteletes_de_viaje",
    "armas_nuevas": [GUANTELETES],
    "set": [
        pieza("Garras del Guerrero demoníaco", "MainHand", "media", {"DES": 1, "CON": 1},
              "Las garras de la SP del Guerrero demoníaco: guanteletes negros con garras rojas, uno en cada mano.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Coraza del Guerrero demoníaco", "Torso", "ligera", {"DES": 1},
              "Coraza negra y roja con las alas de murciélago a la espalda."),
        pieza("Cuernos del Guerrero demoníaco", "Head", "ligera", {"DES": 1, "CON": 1},
              "La diadema negra con la gema roja y dos cuernos negros curvados."),
        pieza("Grebas del Guerrero demoníaco", "Legs", "ligera", {"DES": 1, "CON": 1},
              "Grebas negras del Guerrero demoníaco."),
        pieza("Botas del Guerrero demoníaco", "Feet", "ligera", {"DES": 1, "CON": 1},
              "Botas negras con las punteras rojas."),
    ],
}

DRUIDA_FLAMIGERO = {
    "id": "druida_flamigero", "name": "Druida flamígero", "sp": 5, "elemento": "Fuego",
    "nostale": "Druida flamígero (Flame Druid)",
    "description": ("Especialista de las zarpas de fuego: druida que cura y quema, oso flamígero que protege al grupo "
                    "y golpea, y leopardo flamígero que salta sobre los enemigos."),
    "primaryStat": "CON", "secondaryStat": "DES", "role": "tank",
    # Las tres figuras (H1025 oso flamígero, H1026 leopardo flamígero y H1349 druida) son la forma de la SP: la pasiva
    # (§6), como los espíritus del Monje de lucha. Oso: Energía del oso pardo (defensa +5 %: +1 a la CA). Leopardo:
    # Energía del leopardo rojo (esquiva +(N x10): te defiendes con ventaja). Druida: su básico, las Garras afiladas
    # (H1021: dos zarpazos de fuego, +1 al arma). En NosTale se cambia de figura cada 5 s; aquí se elige al empezar el
    # combate. Ataque zarpazo (H1027), el básico del oso, no es carta y no suma: el oso se queda con su defensa.
    "pasiva": ("Figura flamígera",
               "Al empezar el combate eliges tu figura. Druida: tus golpes con arma hacen +1 de daño (fuego). Oso "
               "flamígero: +1 a la CA. Leopardo flamígero: te defiendes con ventaja.",
               [("Eliges una figura al empezar el combate.", elige_una("druida", "oso", "leopardo")),
                ("Druida: tus golpes con arma hacen +1 de daño.", si("druida", "bono_dano_arma:1")),
                ("Oso flamígero: +1 a la CA.", si("oso", "bono_ca:1")),
                ("Leopardo flamígero: te defiendes con ventaja.", si("leopardo", "ventaja:defensa"))]),
    # 22 entradas: el básico, las tres figuras (la pasiva) y el básico del oso no son cartas, y la cadena del leopardo
    # es una. De las 15 que quedan, las diez que mejor cuentan la SP; las técnicas de las tres figuras se juegan
    # siempre. Fuera: Llama protectora (H1023: el daño recibido -8 %, +1 a la CA todo el combate, lo que ya da la Piel
    # de oso pardo), Ovillo de ataque (H1028: 2d6 y rigidez, el Pisotón paralizador con menos daño), Golpe llameante
    # (H1030) y Agrietador (H1196) (3d6 sin más: la Quemadura fatal del Agrietador no deja nada que entre) y Llama
    # purgadora (H1198: te libra de los estados en contra, lo que ya hace la Llama sanadora).
    "cartas": [
        # Energía del Loa oso (100 %, 300 s): todos los ataques +5 % (+1) y el fuego +(N x2), que no llega a +200 hasta
        # el nivel 100: el mismo +1, como las habilidades ígneas del Poder del volcán
        carta("H1022", "Energía del Loa oso", "self",
              "La energía del Loa oso mejora tu ataque y tu fuego: tus golpes con arma hacen +1 hasta el final del "
              "combate.",
              [("Energía del Loa oso: +1 a tus golpes con arma todo el combate.", estado(300, "bono_dano_arma:1"))]),
        # Tanque flamígero (100 %, 10 s): devolver el 20 % del daño al defender (refleja); la Quemadura al que te golpea
        # (10 %), fuera
        carta("H1029", "Tanque flamígero", "self",
              "Las llamas que te rodean queman a quien te golpea: durante 2 turnos devuelves los golpes físicos a "
              "quien te ataca.",
              [("Tanque flamígero: reflejas los golpes físicos 2 turnos.", estado(10, "refleja:fisico"))]),
        # «Se fijarán en ti»: te atacan a ti. Bravata del oso (100 %, 10 s, tuya): el daño recibido -20 %, +1 a la CA
        # (el 25 % de más que salta la mitad de las veces no se suma: +1, lo seguro). Nerviosismo (100 %, 15 s): todos
        # sus ataques -10 % (desventaja); el daño de sus críticos, fuera
        carta("H1031", "Rugido del Loa oso", "all_enemies",
              "Te alzas sobre las patas traseras y ruges: 3d6 de daño de fuego a cada enemigo, que te ataca a ti 1 "
              "turno y, nervioso, ataca con desventaja 2 turnos; la bravata del oso te da +1 a la CA durante 2 turnos.",
              [("3d6 de daño de fuego a cada enemigo.", dano(800, 1000)), _atrae(),
               ("Nerviosismo: atacan con desventaja 2 turnos.", estado(15, "desventaja:ataque")),
               ("Bravata del oso: +1 a tu CA durante 2 turnos.", propio(10, "bono_ca:1"))], elemento="Fuego"),
        carta("H1032", "Pisotón paralizador", "single_enemy",
              "Te das media vuelta y le das una patada con las garras traseras: 3d6 de daño de fuego y rigidez 1 turno "
              "(no ataca).",
              # apartarlo 4 casillas: sin posiciones
              [("3d6 de daño de fuego.", dano(1100, 800)),
               ("Rigidez: 1 turno sin atacar.", estado(3, "incapacitado"))], elemento="Fuego"),
        # Ventana de defensa de 2 s (§4): «no se consumen HP» es lo que hace la reacción. La Quemadura fatal al que te
        # golpea (50 %): nada que entre
        carta("H1033", "Llama absorbente", "self",
              "La llama te envuelve y te protege: contrarrestas los ataques de ese turno.",
              [CONTRARRESTA], accion="reaccion"),
        carta("H1197", "Ataque en salto", "all_enemies",
              "Saltas y caes a toda velocidad sobre los enemigos: 3d6 de daño de fuego a cada uno y shock 1 turno (no "
              "atacan y se defienden con desventaja).",
              [("3d6 de daño de fuego a cada enemigo.", dano(1000, 900))] + _aturdido(5, "Shock"), elemento="Fuego"),
        # Remolino de asalto abre Salto de fuego (H1232) y este Gancho de tormenta de fuego (H1233): una carta que suma
        # los tres (§2). El aturdimiento del remolino (80 %) y el del gancho (100 %) son el mismo turno; la Quemadura
        # del salto, nada que entre. Quemaduras graves (100 %, 10 s): la vida que pierde cada 2 s por el fuego; el
        # enfriamiento +50 %, fuera. El +5 % por la distancia: sin posiciones
        carta("H1199+H1232+H1233", "Remolino de asalto", "zone_radius",
              "Giras sobre ti mismo y te lanzas sobre el enemigo, saltas envuelto en llamas y rematas con un gancho de "
              "mandíbula: 4d6 de daño de fuego y aturdimiento 1 turno (no ataca y se defiende con desventaja), y las "
              "quemaduras graves le quitan 1 de vida por turno durante 2 turnos.",
              [("4d6 de daño de fuego.", dano(500, 600, 700, 1000, 1300, 1500))] + _aturdido(5)
              + [("Quemaduras graves: 1 de daño por turno durante 2 turnos.", estado(10, "dano_por_turno:1"))],
              elemento="Fuego"),
    ],
    # Creador: las tres que la SP enseña al final (§3). La tarjeta no tiene ninguna de profesión 16: las de 8, 12 y 20.
    "creador": [
        # Raíz flamígera (100 %, 4 s): la vida que pierde cada 2 s por el fuego; no poder moverse, sin posiciones
        carta("H1024", "Raíz flamígera", "zone_radius",
              "Una raíz flamígera brota del suelo y atrapa al enemigo: 2d6 de daño de fuego, y arde: 1 de daño por "
              "turno durante 1 turno.",
              [("2d6 de daño de fuego.", dano(0, 800)),
               ("Raíz flamígera: 1 de daño por turno durante 1 turno.", estado(4, "dano_por_turno:1"))],
              elemento="Fuego"),
        # Piel de oso pardo (100 %, 300 s): la defensa +5 % (+1 a la CA); el daño de los críticos -10 %, fuera
        carta("H1350", "Piel de oso pardo", "self",
              "Tu armadura se vuelve tan resistente como la gruesa piel de un oso pardo: +1 a la CA hasta el final del "
              "combate.",
              [("Piel de oso pardo: +1 a la CA todo el combate.", estado(300, "bono_ca:1"))]),
        # Llama sanadora (100 %, 20 s): bloquea 2 efectos negativos (lo que el combate puede hacer es quitarte los que
        # llevas: limpia, como la Absorción mental del Vidente) y (N x10) HP cada 2 s (N x100 en 20 s: más de N x50,
        # cura 2d6, de una vez). Primero limpia y después cura (§4)
        carta("H1351", "Llama sanadora", "self",
              "Te rodea una llama que insufla vida: te libras de los estados en contra y te curas 2d6.",
              [("Te libras de los estados en contra.", "limpia"), ("Te curas 2d6.", "cura:2d6")]),
    ],
    "arma_inicial": "guanteletes_de_viaje",
    "armas_nuevas": [GUANTELETES],
    "set": [
        pieza("Zarpas del Druida flamígero", "MainHand", "media", {"CON": 1, "DES": 1},
              "Las zarpas de la SP del Druida flamígero: guanteletes de piel de oso con las garras de brasa, uno en "
              "cada mano.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Pellejo del Druida flamígero", "Torso", "pesada", {"CON": 1},
              "La piel del oso pardo sobre los hombros, con el collar de colmillos y las hojas rojas."),
        pieza("Máscara de oso del Druida flamígero", "Head", "pesada", {"CON": 1, "DES": 1},
              "La cabeza del oso pardo, con brasas en los ojos."),
        pieza("Calzas del Druida flamígero", "Legs", "pesada", {"CON": 1, "DES": 1},
              "Calzas de cuero marrón del Druida flamígero."),
        pieza("Botas de piel del Druida flamígero", "Feet", "pesada", {"CON": 1, "DES": 1},
              "Botas de piel del Druida flamígero."),
    ],
}

PUNO_HIDRAULICO = {
    "id": "puno_hidraulico", "name": "Puño Hidráulico", "sp": 6, "elemento": "Tinieblas",
    "nostale": "Puño Hidráulico (Hydraulic Fist)",
    "description": ("Especialista de los brazos mecánicos de tinieblas: salta sobre los enemigos, los arrastra y los "
                    "machaca, bloquea con los brazos robóticos y quema combustible para repararse."),
    "primaryStat": "CON", "secondaryStat": "DES", "role": "tank",
    "pasiva": ("Golpe fuerte",
               "Los brazos robóticos pesados del Puño Hidráulico: tus golpes con arma hacen +1 de daño (tinieblas).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("el combustible (lo da Tanquear; gastarlo da más daño a Puñetazo al suelo, Ataque en salto "
                           "potente, Ataque de escombros y Golpe de dragón, la rigidez de Machacar, ignorar defensa "
                           "con el Ataque con el martillo, la Barrera mágica del Bloqueo frontal, el Excedente de "
                           "combustible mejorado y reiniciar el Golpe de asalto)")]),
    # Sin niveles de profesión en la tarjeta: Creador = las tres últimas (§3), sin contar Ataque con el martillo ni
    # Ataque de escombros, que van con sus saltos.
    "cartas": [
        # Solo convierte maná en combustible: ni el maná ni la barra se simulan, la carta se queda vacía y sale
        # bloqueada (§10)
        carta("H1755", "Tanquear", "self",
              "Conviertes tu poder mágico en combustible para los brazos mecánicos. [pendiente] el combustible: el "
              "combate no lleva el maná ni la barra de combustible.",
              [("[pendiente] El combustible.",
                "pendiente:combustible (Tanquear solo convierte maná en combustible: ni el maná ni la barra se "
                "simulan)")]),
        # Ataque en salto abre Ataque con el martillo (H1758, su continuación): una carta que suma las dos (§2).
        # Rigidez (80 %, 2 s) entra; ignorar un 20 % de la defensa al gastar 5 de combustible, fuera (la pasiva)
        carta("H1756+H1758", "Ataque en salto", "zone_radius",
              "Saltas sobre el enemigo y lo golpeas como con un martillo: 3d6 de daño de tinieblas y rigidez 1 turno "
              "(no ataca).",
              [("3d6 de daño de tinieblas.", dano(600, 400, 800, 600)),
               ("Rigidez: 1 turno sin atacar.", estado(2, "incapacitado"))], elemento="Tinieblas"),
        # Arrastrarlos a un punto: sin posiciones; «se fijarán en ti», te atacan a ti. La Rigidez de gastar 10 de
        # combustible, fuera (la pasiva)
        carta("H1757", "Machacar", "all_enemies",
              "Arrastras a los enemigos a un punto y los aplastas entre las manos robóticas: 2d6 de daño de tinieblas "
              "a cada enemigo, que te ataca a ti 1 turno.",
              [("2d6 de daño de tinieblas a cada enemigo.", dano(800, 400)), _atrae()], elemento="Tinieblas"),
        # Se juega aunque el combustible no se simule: Tanquear da 80 y gastarlo todo (un 2 % por cada 10) cura un
        # 16 %: cura 1d6 (10-20 %)
        carta("H1765", "Protocolo de recuperación", "self",
              "Quemas todo el combustible de los brazos para repararte: te curas 1d6.",
              [("Te curas 1d6.", "cura:1d6")]),
        # Temblores de tierra (80 %, 5 s): «ataque no posible» y la esquiva -10 % (se defienden con desventaja, como
        # con el Gas nervioso del Cazanieblas); la velocidad, fuera. El daño +25 % de gastar 15 de combustible, fuera
        carta("H1759", "Puñetazo al suelo", "all_enemies",
              "Golpeas el suelo con fuerza: 3d6 de daño de tinieblas a cada enemigo, y los temblores de tierra los "
              "dejan 1 turno sin atacar y defendiéndose con desventaja.",
              [("3d6 de daño de tinieblas a cada enemigo.", dano(800, 550)),
               ("Temblores de tierra: 1 turno sin atacar.", estado(5, "incapacitado")),
               ("Temblores de tierra: se defienden con desventaja 1 turno.", estado(5, "desventaja:defensa"))],
              elemento="Tinieblas"),
        # Ventana de defensa de 2 s (§4). Bloqueo frontal (100 %, 2 s): «no se consumen HP» y no recibir efectos
        # negativos, lo que ya hace la reacción; el maná por golpe recibido, fuera. La Barrera mágica (3 s) de gastar
        # 15 de combustible, fuera (la pasiva)
        carta("H1760", "Bloqueo frontal", "self",
              "Bloqueas los golpes con los brazos robóticos: contrarrestas los ataques de ese turno.",
              [CONTRARRESTA], accion="reaccion"),
        # Ataque en salto potente abre Ataque de escombros (H1766, su continuación): una carta que suma las dos (§2). El
        # daño de más del combustible (+25 % y +20 %), fuera (la pasiva). Cuerpo destrozado (100 %, 10 s): la vida que
        # pierde cada 2 s por las heridas (sangra) y la defensa -20 % (-1 a la CA)
        carta("H1761+H1766", "Ataque en salto potente", "zone_radius",
              "Saltas sobre el enemigo y lo aplastas con las manos robóticas entrelazadas: 4d6 de daño de tinieblas; "
              "con el cuerpo destrozado, sangra y tiene -1 a la CA durante 2 turnos.",
              [("4d6 de daño de tinieblas.", dano(700, 600, 700, 900)),
               ("Cuerpo destrozado: sangra 2 turnos.", condicion("sangrado", 10)),
               ("Cuerpo destrozado: -1 a la CA durante 2 turnos.", estado(10, "penal_ca:1"))], elemento="Tinieblas"),
    ],
    "creador": [
        # Ataque furioso (100 %, 5 s): el daño crítico +30 %, que no se simula. Reiniciar el enfriamiento (50 %, al
        # gastar 10 de combustible), fuera
        carta("H1762", "Golpe de asalto", "zone_radius",
              "Concentras el poder mágico en un brazo y te precipitas sobre el enemigo: 3d6 de daño de tinieblas.",
              [("3d6 de daño de tinieblas.", dano(800, 600))], elemento="Tinieblas"),
        # Excedente de combustible (100 %, 180 s): el acierto +(N x2) (ventaja en ataque); la velocidad +1, fuera. El
        # Excedente mejorado de gastar 20 de combustible, fuera (la pasiva)
        carta("H1763", "Excedente de combustible", "self",
              "El combustible de más refuerza los brazos mecánicos: atacas con ventaja hasta el final del combate.",
              [("Excedente de combustible: ventaja en ataque todo el combate.", estado(180, "ventaja:ataque"))]),
        # Shock mágico (100 %, 6 s): el enfriamiento, la velocidad y el coste de maná: nada que entre. Barrera mágica
        # mejorada (100 %, 10 s, tuya): el daño recibido -15 % (+1 a la CA). El daño +50 % de gastar 30 de combustible,
        # fuera (la pasiva)
        carta("H1764", "Golpe de dragón", "zone_radius",
              "Concentras todo el poder en los puños y golpeas en línea recta a los enemigos en fila: 3d6 de daño de "
              "tinieblas, y la barrera mágica mejorada te da +1 a la CA durante 2 turnos.",
              [("3d6 de daño de tinieblas.", dano(1300, 900)),
               ("Barrera mágica mejorada: +1 a tu CA durante 2 turnos.", propio(10, "bono_ca:1"))],
              elemento="Tinieblas"),
    ],
    # Tanquear (la primera) sale bloqueada: entra Protocolo de recuperación, la siguiente jugable (§3)
    "arranque": ["Ataque en salto", "Machacar", "Protocolo de recuperación"],
    "arma_inicial": "guanteletes_de_viaje",
    "armas_nuevas": [GUANTELETES],
    "set": [
        pieza("Brazos mecánicos del Puño Hidráulico", "MainHand", "pesada", {"CON": 1, "DES": 1},
              "Los brazos mecánicos de la SP del Puño Hidráulico: de bronce y hierro, con los pistones y los "
              "remaches, en los dos puños.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Peto del Puño Hidráulico", "Torso", "pesada", {"CON": 1},
              "Peto de cuero y placas de hierro, con el depósito de combustible a la espalda."),
        pieza("Gafas del Puño Hidráulico", "Head", "pesada", {"CON": 1, "DES": 1},
              "Gafas de mecánico de bronce, con las lentes violeta, sobre la frente."),
        pieza("Pantalones del Puño Hidráulico", "Legs", "pesada", {"CON": 1, "DES": 1},
              "Pantalones de mecánico con las rodilleras de hierro."),
        pieza("Botas del Puño Hidráulico", "Feet", "pesada", {"CON": 1, "DES": 1},
              "Botas con las punteras de hierro."),
    ],
}


TRONADOR = {
    "id": "tronador", "name": "Tronador", "sp": 7, "elemento": "Rayo", "nostale": "Tronador (Thunderer)",
    "description": ("Especialista de los guanteletes del rayo: dispara el cañón de descargas del guantelete izquierdo, "
                    "deja sin atacar a los enemigos con la alta tensión, se cubre con el escudo de electrones y lo "
                    "descarga todo en el sprint eléctrico."),
    "primaryStat": "DES", "secondaryStat": "INT", "role": "balanced",
    # Dispara el cañón de descargas que lleva en el guantelete izquierdo: de ahí «a distancia» (DES). Las marcas, en la
    # pasiva: las llenan sus disparos y la Bonificación eléctrica, y con el Amplificador de batería las gastan sus
    # técnicas para reforzarse; nada de eso se simula (§10).
    "pasiva": ("Disparo de cañón de mano",
               "Las descargas del cañón del guantelete izquierdo del Tronador: tus golpes con arma hacen +1 de daño "
               "(rayo).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("la barra de marcas (la llenan sus disparos y, con el Amplificador de batería, las gastan "
                           "Tiro de alta tensión, Electrochoque, Escudo de electrones, Reforzador de ataque y Sprint "
                           "eléctrico para reforzarse)")]),
    # Once habilidades: el básico y diez cartas, sin continuaciones ni variantes que fundir.
    "cartas": [
        # el crítico +15 % de la técnica no entra
        carta("H1869", "Fuego perenne", "zone_radius",
              "Disparas sin parar el cañón de descargas contra el enemigo: 2d6 de daño de rayo.",
              [("2d6 de daño de rayo.", dano(450, 500))], elemento="Rayo"),
        # Electricidad estática (35 %): fuera; el crítico +20 % de la técnica no entra
        carta("H1870", "Fuego cercano", "zone_radius",
              "Disparas a quemarropa al enemigo que tienes cerca: 3d6 de daño de rayo.",
              [("3d6 de daño de rayo.", dano(700, 650))], elemento="Rayo"),
        # Descarga eléctrica (35 %): fuera
        carta("H1871", "Tiro planeador", "zone_radius",
              "Disparas a toda prisa mientras cargas contra el enemigo: 3d6 de daño de rayo.",
              [("3d6 de daño de rayo.", dano(700, 650))], elemento="Rayo"),
        # Shock por alta tensión (50 %): «ataque no posible»; el +30 % al daño de luz un 30 % de las veces y la
        # velocidad, fuera. La marca que da (20 %) y el +20 % de gastarla: la pasiva
        carta("H1872", "Tiro de alta tensión", "all_enemies",
              "Subes la tensión del cañón y lanzas una enorme esfera eléctrica: 3d6 de daño de rayo a cada enemigo y "
              "shock 1 turno (no atacan y se defienden con desventaja).",
              [("3d6 de daño de rayo a cada enemigo.", dano(900, 900))] + _aturdido(5, "Shock"), elemento="Rayo"),
        # Amplificador de batería (100 %, 10 s): solo activa el refuerzo de las marcas, que no se simulan: la carta se
        # queda vacía y sale bloqueada (§10)
        carta("H1873", "Amplificador de batería", "self",
              "Instalas una batería que amplifica el guantelete. [pendiente] el refuerzo de las marcas: el combate aún "
              "no tiene la barra de marcas.",
              [("[pendiente] El refuerzo de las marcas.",
                "pendiente:el refuerzo de las marcas que activa la batería (sin la barra de marcas en el combate)")]),
        # «Se fijarán en ti» no suma nada: con la penetración eléctrica no atacan ese turno (como los Ganchos de cadena
        # del Asesino). Penetración eléctrica (70 %): «ataque no posible»; no moverse, fuera. Campo magnético (100 %,
        # 15 s, tuyo): la defensa +15 % (+1 a la CA); la velocidad, fuera
        carta("H1874", "Onda eléctrica", "all_enemies",
              "Una descarga eléctrica recorre el suelo a tu alrededor: 3d6 de daño de rayo a cada enemigo, que queda 1 "
              "turno sin atacar, y el campo magnético te da +1 a la CA durante 2 turnos.",
              [("3d6 de daño de rayo a cada enemigo.", dano(1000, 1200)),
               ("Penetración eléctrica: 1 turno sin atacar.", estado(5, "incapacitado")),
               ("Campo magnético: +1 a tu CA durante 2 turnos.", propio(15, "bono_ca:1"))], elemento="Rayo"),
        # «Para aturdirlo»: la Penetración eléctrica (50 %), «ataque no posible»; no moverse, fuera. La Penetración
        # eléctrica potente de gastar 1 marca: la pasiva
        carta("H1875", "Electrochoque", "zone_radius",
              "Una fuerte descarga eléctrica sobre el enemigo que tienes cerca: 3d6 de daño de rayo, y queda 1 turno "
              "sin atacar.",
              [("3d6 de daño de rayo.", dano(1000, 1200)),
               ("Penetración eléctrica: 1 turno sin atacar.", estado(5, "incapacitado"))], elemento="Rayo"),
    ],
    # Creador: las de profesión 12, 16 y 20.
    "creador": [
        # Ventana de defensa de 2 s (§4). Barrera eléctrica (100 %, 2 s): no se consumen HP (lo que hace la reacción)
        # y el enemigo pierde el 10 % del daño que te hace (devuelve el daño: refleja); que no te pongan efectos
        # negativos ya lo cubre el contrarrestar. El indicador que sube al recibir daño y la Barrera eléctrica mejorada
        # de gastar 1 marca: la pasiva
        carta("H1876", "Escudo de electrones", "self",
              "Creas un escudo eléctrico: contrarrestas los ataques de ese turno y, durante 2 turnos, devuelves los "
              "golpes a quien te ataca.",
              [CONTRARRESTA,
               ("Barrera eléctrica: reflejas los golpes físicos 2 turnos.", "propio:2:refleja:fisico")],
              accion="reaccion", elemento="Rayo"),
        # Reforzador de ataque (100 %, 300 s): todos los ataques +5 % (+1). La Bonificación eléctrica menor (100 %,
        # 120 s) llena la barra de marcas y, con 1 marca, sube el ataque un 3 %; las de gastar 1 o 2 marcas, igual: la
        # pasiva
        carta("H1877", "Reforzador de ataque", "self",
              "Subes la intensidad de la corriente del guantelete: tus golpes con arma hacen +1 hasta el final del "
              "combate.",
              [("Reforzador de ataque: +1 a tus golpes con arma todo el combate.", estado(300, "bono_dano_arma:1"))]),
        # «Solo se puede realizar si tienes 2 marcas» (y el +50 % de gastarlas): sin marcas, se juega siempre. Shock por
        # sobretensión (100 %, 10 s): no es «ataque no posible» (como el Shock llameante de la Tormenta de fuego); el
        # daño recibido +15 % (tus golpes les hacen +1) y la vida que pierde por el daño que hace (pendiente); la
        # velocidad y el +20 % al daño de luz un 10 % de las veces, fuera
        carta("H1878", "Sprint eléctrico", "all_enemies",
              "Concentras toda la electricidad en el guantelete y la descargas hacia delante: 4d6 de daño de rayo a "
              "cada enemigo, y el shock por sobretensión hace que tus golpes les hagan +1 durante 2 turnos.",
              [("4d6 de daño de rayo a cada enemigo.", dano(1600, 1000)),
               ("Shock por sobretensión: tus golpes les hacen +1 durante 2 turnos.", estado(10, "bono_dano_contra:1")),
               _pendiente("que el shock por sobretensión les quite vida al atacar (un 20 % del daño que hacen)")],
              elemento="Rayo"),
    ],
    "arma_inicial": "guanteletes_de_viaje",
    "armas_nuevas": [GUANTELETES],
    "set": [
        pieza("Guanteletes del Tronador", "MainHand", "media", {"DES": 1, "INT": 1},
              "Los guanteletes de la SP del Tronador: el derecho, de acero con bobinas; el izquierdo, el cañón de "
              "descargas.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Chaqueta del Tronador", "Torso", "media", {"DES": 1},
              "Chaqueta amarilla con las bandas blancas, las bobinas de cobre en los hombros y el cable cruzado."),
        pieza("Casco del Tronador", "Head", "media", {"DES": 1, "INT": 1},
              "Casco de latón con la antena del rayo en lo alto."),
        pieza("Pantalones del Tronador", "Legs", "media", {"DES": 1, "INT": 1},
              "Pantalones negros del Tronador."),
        pieza("Botas del Tronador", "Feet", "media", {"DES": 1, "INT": 1},
              "Botas negras del Tronador."),
    ],
}

THOR = {
    "id": "thor", "name": "Thor", "sp": 8, "elemento": "Rayo", "nostale": "Thor",
    "description": ("Especialista de los guanteletes del trueno: golpea con hielo y rayos, hace perder pie a los "
                    "enemigos, refuerza al grupo con el trueno y remata con los rayos precipitados."),
    "primaryStat": "DES", "secondaryStat": "CAR", "role": "support",
    # La sincronización dimensional, en la pasiva (pendiente), como Aquiles y Almirante Yi; el héroe invocado y el
    # elemento del hada, fuera.
    "pasiva": ("Puño helado",
               "Los puñetazos de aliento helado de Thor: tus golpes con arma hacen +1 de daño (hielo).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("la barra de marcas que llenan sus técnicas y la sincronización dimensional (con el héroe "
                           "que invoca)")]),
    # Sin niveles de profesión en la tarjeta: Creador = las tres últimas (§3). Once habilidades: el básico y diez
    # cartas, sin continuaciones ni variantes que fundir.
    "cartas": [
        # Aliento helado (35 %): fuera; sin penalización a corta distancia: el alcance no se simula
        carta("H1987", "Grietas heladas", "all_enemies",
              "Una helada cubre el suelo a tu alrededor y ataca a los enemigos: 2d6 de daño de hielo a cada uno.",
              [("2d6 de daño de hielo a cada enemigo.", dano(500, 600))], elemento="Hielo"),
        # Sujeción insegura (80 %): todos sus ataques -15 % (desventaja); no moverse, fuera
        carta("H1988", "Desestabilización", "zone_radius",
              "El enemigo lejano pierde pie: 3d6 de daño de hielo, y ataca con desventaja 1 turno.",
              [("3d6 de daño de hielo.", dano(650, 750)),
               ("Sujeción insegura: ataca con desventaja 1 turno.", estado(4, "desventaja:ataque"))], elemento="Hielo"),
        # Descarga eléctrica helada (70 %): la vida que pierden cada 2 s y el acierto al 70 % (desventaja); el 20 %
        # menos de daño con Aliento helado, fuera
        carta("H1989", "Relámpagos helados", "all_enemies",
              "Invocas rayos helados y electrizantes sobre los enemigos: 3d6 de daño de rayo a cada uno; durante 1 "
              "turno pierden 1 de vida por turno y atacan con desventaja.",
              [("3d6 de daño de rayo a cada enemigo.", dano(650, 800)),
               ("Descarga eléctrica helada: 1 de daño por turno durante 1 turno.", estado(5, "dano_por_turno:1")),
               ("Descarga eléctrica helada: atacan con desventaja 1 turno.", estado(5, "desventaja:ataque"))],
              elemento="Rayo"),
        # Ventana de defensa de 2 s (§4). Escudo helado (100 %, 2 s): no se consumen HP (lo que hace la reacción) y no
        # te ponen efectos negativos (ya lo cubre el contrarrestar); la velocidad y el indicador que sube al recibir
        # daño (la sincronización), fuera
        carta("H1990", "Escudo helado", "self",
              "Un escudo de hielo te cubre en el último momento: contrarrestas los ataques de ese turno.",
              [CONTRARRESTA], accion="reaccion", elemento="Hielo"),
        # Congelación grave (100 %): el nivel de defensa -1 (-1 a la CA); el +20 % al daño de agua un 20 % de las veces
        # y la velocidad, fuera
        carta("H1991", "Erupción helada", "zone_radius",
              "Un puñetazo tremendo lanza hacia delante una ola de hielo: 3d6 de daño de hielo, y la congelación grave "
              "le deja -1 a la CA durante 2 turnos.",
              [("3d6 de daño de hielo.", dano(850, 1050)),
               ("Congelación grave: -1 a la CA durante 2 turnos.", estado(10, "penal_ca:1"))], elemento="Hielo"),
        # Sujeción insegura (80 %): todos sus ataques -15 % (desventaja); no moverse y el alcance, fuera
        carta("H1992", "Torbellino helado", "all_enemies",
              "Un remolino helado ataca a los enemigos cercanos: 3d6 de daño de hielo a cada uno, que te ataca a ti 1 "
              "turno, y con desventaja.",
              [("3d6 de daño de hielo a cada enemigo.", dano(950, 1400)), _atrae(),
               ("Sujeción insegura: atacan con desventaja 1 turno.", estado(4, "desventaja:ataque"))], elemento="Hielo"),
        # Torbellino de ventisca helada (100 %, 10 s): es tuyo, pero hiere cada 1,5 s a los enemigos a 5 casillas; sin
        # posiciones, la zona hace su efecto al jugar la carta, sobre todos los enemigos (como la Sobrecarga de
        # combustible de la Tormenta de fuego), y pierden vida. La Alta tensión que reparte a 3 casillas no tiene ficha
        # (pendiente); la velocidad y el aturdimiento 2 s más corto, fuera
        carta("H1993", "Protección contra el hielo y los rayos", "all_enemies",
              "Un torbellino de hielo y rayos gira a tu alrededor y sigue atacando: los enemigos pierden 1 de vida por "
              "turno durante 2 turnos.",
              [("Torbellino de ventisca helada: 1 de daño por turno durante 2 turnos.", estado(10, "dano_por_turno:1")),
               _pendiente("la Alta tensión que reciben los enemigos más cercanos (la ficha no la da)")]),
    ],
    "creador": [
        # Frío glacial (100 %, 20 s, tuyo): la técnica siguiente suma el 30 % del daño que guarda; la carta no tiene
        # cómo, como la Absorción del alma del Sacerdote vudú
        carta("H1994", "¡Atención: iceberg!", "all_enemies",
              "Un frío glacial e imparable convierte a los enemigos en icebergs: 3d6 de daño de hielo a cada uno.",
              [("3d6 de daño de hielo a cada enemigo.", dano(1100, 1000)),
               _pendiente("que la próxima técnica sume el 30 % del daño que guarda")], elemento="Hielo"),
        # Trueno fortalecedor (100 %, 300 s): todos los ataques +5 % (+1); lo del héroe invocado y lo que suma con la
        # sincronización dimensional, fuera (la pasiva). Soplo helado fortalecedor (180 s): el enfriamiento, los HP
        # máximos y el elemento del hada no se simulan
        carta("H1995", "Trueno creciente", "all_allies",
              "El poder del trueno crece en tu guantelete y en las armas del grupo: tú y tus aliados hacéis +1 con los "
              "golpes de arma hasta el final del combate.",
              [("Trueno fortalecedor: +1 a los golpes con arma todo el combate.", estado(300, "bono_dano_arma:1"))], elemento="Rayo"),
        # correr hacia los enemigos: sin posiciones. Descarga eléctrica helada divina (100 %): la vida que pierden cada
        # 2 s y todos sus ataques -15 % (desventaja); la resistencia al agua y la velocidad, fuera. Bendición de Thor
        # (100 %, tuya): todos los ataques +10 % (+1); el daño de los críticos que recibes, fuera
        carta("H1996", "Rayos precipitados", "all_enemies",
              "Corres hacia los enemigos e invocas un enorme rayo de hielo: 4d6 de daño de rayo a cada uno; durante 2 "
              "turnos pierden 1 de vida por turno y atacan con desventaja, y la bendición de Thor te da +1 a tus "
              "golpes con arma durante 2 turnos.",
              [("4d6 de daño de rayo a cada enemigo.", dano(1500, 1800)),
               ("Descarga eléctrica helada divina: 1 de daño por turno durante 2 turnos.",
                estado(10, "dano_por_turno:1")),
               ("Descarga eléctrica helada divina: atacan con desventaja 2 turnos.", estado(10, "desventaja:ataque")),
               ("Bendición de Thor: +1 a tus golpes con arma durante 2 turnos.", propio(10, "bono_dano_arma:1")),
               _pendiente("que cada nivel de sincronización dimensional sume un 10 % al daño")], elemento="Rayo"),
    ],
    "arma_inicial": "guanteletes_de_viaje",
    "armas_nuevas": [GUANTELETES],
    # Nombre propio: las piezas son «de Thor» y el set, «Set de Thor».
    "set": [
        pieza("Guanteletes de Thor", "MainHand", "media", {"DES": 1, "CAR": 1},
              "Los guanteletes de la SP de Thor: de hielo azul, con los aros de oro y la gema del rayo.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Coraza de Thor", "Torso", "media", {"DES": 1},
              "Placas de plata con el rayo de oro, las hombreras azul hielo y la capa azul oscuro."),
        pieza("Yelmo alado de Thor", "Head", "media", {"DES": 1, "CAR": 1},
              "Yelmo de plata con dos alas blancas."),
        pieza("Grebas de Thor", "Legs", "media", {"DES": 1, "CAR": 1},
              "Grebas de plata de Thor."),
        pieza("Botas de Thor", "Feet", "media", {"DES": 1, "CAR": 1},
              "Botas de plata de Thor."),
    ],
}


ARTISTA_MARCIAL = [PUNO_DEL_DRAGON, ARTES_MISTICAS, MAESTRO_LOBO, GUERRERO_DEMONIACO, DRUIDA_FLAMIGERO,
                   PUNO_HIDRAULICO, TRONADOR, THOR]
