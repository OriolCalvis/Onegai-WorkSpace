"""SP-02 — Las doce SP del Arquero de NosTale, como clases de MotorGraphico (los datos).

Cada carta dice de que habilidad sale (su H en la enciclopedia de SP de Oriol) y sus cifras
pasan por las reglas de sp_reglas.py (planificacion/CANON_SP_CLASES.md): los dados por la
bonificacion de la ficha (a distancia o cuerpo a cuerpo + elemental, sumando los pasos de una
continuacion), las rondas por los segundos, los estados con un 50 % o mas. Cuando una carta se
aparta de la regla, lo dice un comentario. Lo escribe en los catalogos tools/importar_clases_sp.py.

Las armas de fuego (Oriol, 07/10: «Añadir el concepte d'armes de foc però, en comptes de
pòlvora alquímia i màgia son la base tecnològica»): la escopeta, el cañón de mano, el revólver,
el lanzagranadas y el rifle cargan alquimia y magia (sal de fuego, granadas alquímicas, balas
rúnicas), nunca pólvora.

Criterios comunes a las doce (ademas de los del Espadachin):
- Los criticos propios de un golpe («la probabilidad de impacto critico aumenta», «dano critico
  +X %») no entran: la carta no tiene como.
- «Esquiva -X %» (Poner al descubierto el punto debil, Gas nervioso, Gas opaco): se defiende con
  desventaja, el reves de «esquiva + -> ventaja:defensa» del diccionario.
- Un efecto de un estado que solo salta a veces (Luz danina: +20 % al dano de luz un 20 % de las
  veces; Impacto del oleaje, un 10 %) no llega al 50 % y se queda fuera.
- Las ordenes al Lobo solar no traen cifras en la tarjeta: las pone la tecnica del lobo que
  habilitan (H1651-H1655), que se suma como una continuacion (§2).
- Calor y marcas (como el Romperrocas): lo que los pide («disponible con Caliente», «solo si
  tienes 1 marca») se juega siempre; lo que da gastarlos o tener mas, fuera, salvo donde la carta
  dice pendiente.
- Arranque: si una de las tres primeras sale bloqueada, entra la siguiente jugable (Cazador,
  Destructor, Espia); el Cazasoles empieza con la carta que trae al Lobo solar.
"""
from sp_reglas import (CONTRARRESTA, DOS_MANOS, arma_comun, carta, condicion, dano, elige_una, estado, pieza,
                       propio, rondas, si, siempre)
from sp_reglas import atrae as _atrae
from sp_reglas import aturdido as _aturdido
from sp_reglas import pendiente as _pendiente


CAZADOR = {
    "id": "cazador", "name": "Cazador", "sp": 1, "elemento": "Agua", "nostale": "Cazador (Ranger)",
    "description": ("Especialista del arco de agua: apunta con el ojo del halcón, congela a los enemigos con "
                    "sus rayos y los barre con una tormenta de flechas."),
    "primaryStat": "DES", "secondaryStat": "INT", "role": "agile",
    "pasiva": ("Tiro de arco",
               "La cadena de tres flechas del Cazador: tus golpes con arma hacen +1 de daño (agua).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        # Ojo del halcon: el acierto del 100 % (ventaja) y el dano +3 % segun la distancia (+1); los
        # criticos -15 % y el alcance +1, fuera
        carta("H834", "Ataque de falcón", "self",
              "Apuntas con la precisión de un halcón hasta el final del combate: atacas con ventaja y tus "
              "golpes con arma hacen +1.",
              [("Ojo del halcón: ventaja en ataque todo el combate.", estado(300, "ventaja:ataque")),
               ("Ojo del halcón: +1 a tus golpes con arma todo el combate.", estado(300, "bono_dano_arma:1"))]),
        carta("H835", "Tiro de cabeza", "single_enemy",
              "Un tiro certero a la cabeza: 1d6 de daño de agua y aturdimiento 1 turno (no ataca y se "
              "defiende con desventaja).",
              # el critico +25 % de la tecnica no entra (las cifras de critico de las del Espadachin, tampoco)
              [("1d6 de daño de agua.", dano(250, 180))] + _aturdido(5), elemento="Agua"),
        # Corredor de viento: velocidad +3 y un espejismo, nada que el combate simule: sale bloqueada (§10)
        carta("H836", "Corredor de viento", "self",
              "Corres como el viento. [pendiente] la velocidad de movimiento: el combate no tiene posiciones.",
              [("[pendiente] la velocidad de movimiento.",
                "pendiente:la velocidad de movimiento (sin posiciones en el combate)")]),
        # La Explosion-Relampago (la de los monstruos) no publica efectos: la habilidad dice que no puede
        # contraatacar (la de PvP: «ataque no posible») y las dos enlazan Poner al descubierto el punto
        # debil, «recibir siempre un ataque fatal» (5 s)
        carta("H837", "Ataque relámpago", "zone_radius",
              "Un relámpago cegador: 2d6 de daño de rayo, y el enemigo queda ciego 1 turno: no ataca y los "
              "golpes contra él son críticos.",
              [("2d6 de daño de rayo.", dano(600, 150)),
               ("Ciego: 1 turno sin atacar.", estado(3, "incapacitado")),
               ("Punto débil al descubierto: los golpes contra él son críticos 1 turno.",
                estado(5, "critico_contra"))], elemento="Rayo"),
        carta("H838", "Flecha triple", "zone_radius",
              "Tres flechas a la vez que hieren a varios: 2d6 de daño de agua.",
              # el critico +50 % de la tecnica no entra
              [("2d6 de daño de agua.", dano(500, 350))], elemento="Agua"),
        carta("H839", "Aumento de larga distancia", "single_enemy",
              "Un disparo desde muy lejos: 3d6 de daño de agua.",
              # 70 % de Increible (velocidad) y el alcance: no se simulan
              [("3d6 de daño de agua.", dano(1000, 500))], elemento="Agua"),
        carta("H840", "Rayo de espiral", "zone_radius",
              "Un rayo en espiral que hiere de muerte: 3d6 de daño de rayo, y la helada le deja 2 turnos sin "
              "atacar.",
              [("3d6 de daño de rayo.", dano(450, 1300)),
               ("Helada: 2 turnos sin atacar.", estado(8, "incapacitado"))], elemento="Rayo"),
    ],
    "creador": [
        carta("H841", "Explosión relámpago", "all_enemies",
              "Haces caer rayos a tu alrededor: 2d6 de daño de rayo a cada enemigo.",
              # empujar (solo PvP) y el 80 % de Ralentizacion (velocidad): no se simulan
              [("2d6 de daño de rayo a cada enemigo.", dano(800, 400))], elemento="Rayo"),
        # Rapido: el enfriamiento -20 % de las tecnicas, lo que le da nombre, no se simula (§10); queda el
        # dano recibido -10 %: +1 a la CA
        carta("H842", "Recuperación rápida", "self",
              "Recuperas el ritmo y el daño te hace menos mella: +1 a la CA hasta el final del combate.",
              [("Rápido: +1 a la CA todo el combate.", estado(180, "bono_ca:1"))]),
        carta("H843", "Tormenta", "all_enemies",
              "Caen flechas como una tormenta: 3d6 de daño de hielo a cada enemigo, y la helada les deja 2 "
              "turnos sin atacar.",
              # Helado (100 %): la resistencia al agua no se simula y el +30 % al dano de agua solo sale el
              # 20 % de las veces: fuera
              [("3d6 de daño de hielo a cada enemigo.", dano(1200, 800)),
               ("Helada: 2 turnos sin atacar.", estado(8, "incapacitado"))], elemento="Hielo"),
    ],
    # Corredor de viento (la tercera) sale bloqueada: entra Ataque relampago (§3)
    "arranque": ["Ataque de falcón", "Tiro de cabeza", "Ataque relámpago"],
    "arma_inicial": "arco_cazador_de_milicia",
    "set": [
        pieza("Arco del Cazador", "MainHand", "ligera", {"DES": 1, "INT": 1},
              "El arco de la SP del Cazador: madera curvada y flechas de agua."),
        pieza("Jubón del Cazador", "Torso", "ligera", {"DES": 1},
              "Jubón de cuero con la aljaba cruzada a la espalda y el pañuelo azul al cuello."),
        pieza("Capucha del Cazador", "Head", "ligera", {"DES": 1, "INT": 1},
              "Capucha verde azulado con una pluma."),
        pieza("Calzas del Cazador", "Legs", "ligera", {"DES": 1, "INT": 1},
              "Calzas de cuero del Cazador."),
        pieza("Botas del Cazador", "Feet", "ligera", {"DES": 1, "INT": 1},
              "Botas de cuero del Cazador."),
    ],
}

ASESINO = {
    "id": "asesino", "name": "Asesino", "sp": 2, "elemento": "Tinieblas", "nostale": "Asesino (Assassin)",
    "description": ("Especialista del puñal de tinieblas: se esconde en la sombra y en la niebla, apuñala de "
                    "cerca y convierte el golpe enemigo en fuerza propia."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "agile",
    # La Sombra se activa con su carta (§2.3: es carta, no forma). Los «danos adicionales en un ataque
    # escondido» de sus golpes no se simulan: lo dice cada carta.
    "pasiva": ("Puñalada",
               "La cadena de tres puñaladas del Asesino: tus golpes con arma hacen +1 de daño (tinieblas).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        # Cre-Up: critico +5 % (ventaja) y todos los ataques +5 % (+1). El Pacto de la oscuridad (100 %,
        # 6 s) da la misma ventaja, ya dentro de los 3 turnos; su dano critico, la velocidad y el
        # enfriamiento no se simulan
        carta("H845", "Acierto crítico", "self",
              "Buscas el golpe crítico: durante 3 turnos atacas con ventaja y tus golpes con arma hacen +1.",
              [("Cre-Up: ventaja en ataque durante 3 turnos.", estado(30, "ventaja:ataque")),
               ("Cre-Up: +1 a tus golpes con arma durante 3 turnos.", estado(30, "bono_dano_arma:1"))]),
        carta("H846", "Cuchillada sangrante", "single_enemy",
              "Una cuchillada malévola: 2d6 de daño de sangre.",
              # 40 % de hemorragia mortal: fuera; el dano adicional de un ataque escondido no se simula
              [("2d6 de daño de sangre.", dano(350, 400))], elemento="Sangre"),
        carta("H847", "Niebla espesa", "all_enemies",
              "Reúnes una niebla densa: los enemigos atacan con desventaja 2 turnos y tú, con pies ágiles, te "
              "defiendes con ventaja 1 turno.",
              [("Esmog: atacan con desventaja 2 turnos.", estado(10, "desventaja:ataque")),
               ("Pies ágiles: te defiendes con ventaja 1 turno.", propio(3, "ventaja:defensa"))]),
        # tirar de el: sin posiciones; «se fijaran en ti» no suma nada, con el shock no ataca ese turno
        carta("H848", "Lanzar ganchos de cadena", "single_enemy",
              "Lanzas los ganchos de cadena y tiras del enemigo: shock 1 turno (no ataca y se defiende con "
              "desventaja).",
              _aturdido(5, "Shock")),
        # el mismo estado que el Protector de cuchillas del Ninja: Absorbe el poder de ataque (2 s)
        carta("H849", "Tambalearse", "self",
              "Saltas hacia atrás en el último momento: contrarrestas los ataques de ese turno y la fuerza del "
              "golpe pasa a tu puñal: +1d6 al daño de tus golpes con arma durante 2 turnos.",
              [CONTRARRESTA,
               ("Absorbe el poder de ataque: +1d6 a tus golpes con arma 2 turnos.",
                "propio:2:bono_dano_arma:1d6")],
              accion="reaccion"),
        carta("H850", "Tomahawk", "zone_radius",
              "Lanzas dagas voladoras: 2d6 de daño de tinieblas.",
              # Veneno mental (70 %): quita vida segun el MP que gaste, y el MP no se simula; el dano
              # adicional de un ataque escondido, tampoco
              [("2d6 de daño de tinieblas.", dano(600, 500))], elemento="Tinieblas"),
        # Guarida: te oculta un tiempo aleatorio (0 s en la ficha): 1 turno. El teletransporte y la
        # velocidad no se simulan
        carta("H851", "Sombra", "self",
              "Te escondes en las sombras: te defiendes con ventaja 1 turno.",
              [("Guarida: te defiendes con ventaja 1 turno.", propio(0, "ventaja:defensa"))]),
    ],
    "creador": [
        carta("H852", "Cuchillada giradora", "zone_radius",
              "Giras con el puñal a toda velocidad: 3d6 de daño de tinieblas.",
              # 80 % de pierna herida: velocidad, no se simula; el dano de un ataque escondido, tampoco
              [("3d6 de daño de tinieblas.", dano(900, 600))], elemento="Tinieblas"),
        # su fuerza es el ataque escondido (+2500 sobre +800): sin el, 2d6; queda dicho y se juega
        carta("H853", "Cuchillada mortal", "single_enemy",
              "Un golpe reforzado a quemarropa: 2d6 de daño de tinieblas.",
              [("2d6 de daño de tinieblas.", dano(300, 500)),
               _pendiente("el daño adicional si atacas escondido en la Sombra")], elemento="Tinieblas"),
        # la bomba estalla al jugar la carta; el salto atras, sin posiciones. Veneno revitalizante (100 %):
        # +20 % de recibir un ataque fatal (no llega al 50 %) y resistencias: fuera. El dano de un ataque
        # escondido no se simula
        carta("H854", "Bomba", "all_enemies",
              "Saltas hacia atrás y una bomba estalla a los pies de los enemigos: 4d6 de daño de tinieblas a "
              "cada uno.",
              [("4d6 de daño de tinieblas a cada enemigo.", dano(1000, 1800))], elemento="Tinieblas"),
    ],
    "arma_inicial": "daga_de_milicia",
    "set": [
        pieza("Puñal del Asesino", "MainHand", "ligera", {"DES": 1, "CON": 1},
              "El puñal de la SP del Asesino: hoja negra y corta, filo de tinieblas."),
        pieza("Chaleco del Asesino", "Torso", "ligera", {"DES": 1},
              "Chaleco de cuero negro con correas de cuchillos y la faja roja."),
        pieza("Capucha del Asesino", "Head", "ligera", {"DES": 1, "CON": 1},
              "Capucha negra y honda."),
        pieza("Calzas del Asesino", "Legs", "ligera", {"DES": 1, "CON": 1},
              "Calzas negras del Asesino."),
        pieza("Botas del Asesino", "Feet", "ligera", {"DES": 1, "CON": 1},
              "Botas negras del Asesino."),
    ],
}

DESTRUCTOR = {
    "id": "destructor", "name": "Destructor", "sp": 3, "elemento": "Fuego", "nostale": "Destructor (Destroyer)",
    "description": ("Especialista de la escopeta alquímica de fuego: dispara cargas de sal de fuego, siembra "
                    "bombas y minas y remata con la bomba más fuerte."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "balanced",
    "pasiva": ("Dispara la escopeta",
               "La cadena de tres disparos del Destructor: tus golpes con arma hacen +1 de daño (fuego).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H912", "Tiro rápido", "single_enemy",
              "Un tiro rápido que deja al enemigo sin sentido: 2d6 de daño de fuego y aturdimiento 1 turno "
              "(no ataca y se defiende con desventaja).",
              [("2d6 de daño de fuego.", dano(300, 350))] + _aturdido(5), elemento="Fuego"),
        carta("H913", "Tiro lejano", "zone_radius",
              "La escopeta se vuelve un arma mortal: 3d6 de daño de fuego.",
              # el critico +100 % de la tecnica no entra (las cifras de critico de las del Espadachin, tampoco)
              [("3d6 de daño de fuego.", dano(700, 700))], elemento="Fuego"),
        # Fortalecedor encendido (y la Velocidad que enlaza): velocidad y un espejismo, nada que el combate
        # simule: sale bloqueada (§10)
        carta("H914", "Acelerador activado", "self",
              "Enciendes el acelerador alquímico. [pendiente] la velocidad de movimiento: el combate no tiene "
              "posiciones.",
              [("[pendiente] la velocidad de movimiento.",
                "pendiente:la velocidad de movimiento (sin posiciones en el combate)")]),
        # Una bomba colocada estalla al jugar la carta (sin posiciones), pero la ficha no da su dano ni
        # su estado: sale bloqueada
        carta("H915", "Colocabombas", "self",
              "Colocas una bomba alquímica que estalla sobre los enemigos cercanos. [pendiente] el daño de la "
              "bomba: la ficha no lo publica.",
              [("[pendiente] el daño de la bomba.", "pendiente:el daño de la bomba (la ficha no lo publica)")],
              elemento="Fuego"),
        # Gas toxico: +15 % de recibir un ataque fatal, por debajo del 50 % (como el Juicio del Renegado),
        # fuera; El 2do Gas toxico que enlaza no dice cuando salta, fuera
        carta("H916", "Bomba de gas tóxico", "zone_radius",
              "Una bomba alquímica de gas tóxico. Quien falle la salvación (resistencia física) tiene -1 a la "
              "CA durante 2 turnos.",
              [("Gas tóxico: -1 a la CA durante 2 turnos.", estado(15, "penal_ca:1"))],
              salvacion="resistencia_fisica", elemento="Veneno"),
        carta("H917", "Tiro explosivo", "single_enemy",
              "Una carga alquímica reforzada revienta en el enemigo: 3d6 de daño de fuego.",
              # «haz clic sobre 4 bloques del oponente»: posiciones, no se simula
              [("3d6 de daño de fuego.", dano(300, 1500))], elemento="Fuego"),
        # Sanacion milagrosa: entran las dos que mas pesan (dano recibido -15 %, todos los ataques +10 %);
        # el critico +10 %, fuera
        carta("H918", "Cura milagrosa", "self",
              "Un remedio alquímico milagroso: te libras de los estados en contra y, hasta el final del "
              "combate, tienes +1 a la CA y tus golpes con arma hacen +1.",
              [("Te libras de los estados en contra.", "limpia"),
               ("Sanación milagrosa: +1 a la CA todo el combate.", estado(180, "bono_ca:1")),
               ("Sanación milagrosa: +1 a tus golpes con arma todo el combate.",
                estado(180, "bono_dano_arma:1"))]),
    ],
    "creador": [
        carta("H919", "Tiro boom", "all_enemies",
              "Tiras una bomba alquímica sobre los enemigos: 3d6 de daño de fuego a cada uno, que sangra 2 "
              "turnos.",
              [("3d6 de daño de fuego a cada enemigo.", dano(900, 1100)),
               ("Hemorragia mortal: sangran 2 turnos.", condicion("sangrado", 10))], elemento="Fuego"),
        # Las tres minas estallarian al jugar la carta (sin posiciones), pero la ficha no da su dano ni
        # su estado: sale bloqueada
        carta("H920", "Mina de fuego", "self",
              "Lanzas tres minas incendiarias contra el enemigo. [pendiente] el daño de las minas: la ficha no "
              "lo publica.",
              [("[pendiente] el daño de las minas.", "pendiente:el daño de las minas (la ficha no lo publica)")],
              elemento="Fuego"),
        carta("H921", "Caída del infierno", "all_enemies",
              "La bomba alquímica más fuerte estalla: 4d6 de daño de fuego a cada enemigo y aturdimiento 1 "
              "turno (no atacan y se defienden con desventaja).",
              [("4d6 de daño de fuego a cada enemigo.", dano(1000, 1500))] + _aturdido(6), elemento="Fuego"),
    ],
    # El catalogo no tiene escopeta de tier 1: una normal, como las de Viaje (la carga, alquimia).
    # Acelerador activado y Colocabombas salen bloqueadas: entra Bomba de gas toxico (§3)
    "arranque": ["Tiro rápido", "Tiro lejano", "Bomba de gas tóxico"],
    "arma_inicial": "escopeta_de_viaje",
    "armas_nuevas": [
        arma_comun("Escopeta de Viaje", "media",
                   "Una escopeta alquímica de cañón corto que dispara cargas de sal de fuego, a dos manos.",
                   restricciones=["Precisión: DES", DOS_MANOS]),
    ],
    "set": [
        pieza("Escopeta del Destructor", "MainHand", "media", {"DES": 1, "CON": 1},
              "La escopeta de la SP del Destructor: cañón corto y cargas de sal de fuego, a dos manos.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Chaqueta del Destructor", "Torso", "media", {"CON": 1},
              "Chaqueta acolchada con la bandolera de bombas alquímicas."),
        pieza("Pañuelo del Destructor", "Head", "ligera", {"DES": 1, "CON": 1},
              "Pañuelo rojo atado a la cabeza, con las gafas de taller subidas."),
        pieza("Pantalones del Destructor", "Legs", "ligera", {"DES": 1, "CON": 1},
              "Pantalones de trabajo del Destructor."),
        pieza("Botas del Destructor", "Feet", "media", {"DES": 1, "CON": 1},
              "Botas de trabajo del Destructor."),
    ],
}

GUARDABOSQUES = {
    "id": "guardabosques", "name": "Guardabosques", "sp": 4, "elemento": "Luz",
    "nostale": "Guardabosques (Wild Keeper)",
    "description": ("Especialista del bumerán de luz: barre a los enemigos en área y llama a los espíritus del "
                    "águila, el lobo y el oso para el grupo."),
    "primaryStat": "DES", "secondaryStat": "CAR", "role": "support",
    # Los espiritus del aguila, el lobo y el oso se activan con su carta (§2.3: son cartas, no forma).
    "pasiva": ("Lanza el bumerán",
               "La cadena de tres bumeranes del Guardabosques: tus golpes con arma hacen +1 de daño (luz).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H923", "Golpe de las serpientes", "all_enemies",
              "Dos bumeranes que serpentean entre los enemigos: 2d6 de daño de luz a cada uno.",
              # el 40 % de aturdimiento no llega al 50 %: fuera
              [("2d6 de daño de luz a cada enemigo.", dano(240, 350))], elemento="Luz"),
        carta("H924", "Zarpa de bestia", "zone_radius",
              "El bumerán corta como una zarpa: 2d6 de daño de luz.",
              # 30 % de hemorragia mortal: fuera
              [("2d6 de daño de luz.", dano(360, 500))], elemento="Luz"),
        carta("H925", "Espíritu de águila", "self",
              "Invocas el alma del águila: atacas con ventaja hasta el final del combate.",
              [("Espíritu del águila: ventaja en ataque todo el combate.", estado(180, "ventaja:ataque"))]),
        # Brillo elemental: defensa contra los elementos +5 y el elemento del hada +5; sin resistencias ni
        # hadas en el combate (§4: el elemento, para cuando haya resistencias) no queda nada: sale bloqueada
        carta("H926", "Resplandor elemental", "all_allies",
              "Empapas al grupo del espíritu de la naturaleza. [pendiente] la defensa contra los elementos: el "
              "combate aún no tiene resistencias.",
              [("[pendiente] la defensa contra los elementos.",
                "pendiente:la defensa contra los elementos (sin resistencias en el combate)")]),
        carta("H927", "Volar en círculo", "all_enemies",
              "Lanzas el bumerán con fuerza en círculo: 3d6 de daño de luz a cada enemigo.",
              # sin penalizacion a corta distancia: el alcance no se simula
              [("3d6 de daño de luz a cada enemigo.", dano(800, 650))], elemento="Luz"),
        # «varios ataques +(N x4)», N el nivel del jugador: +200 o mas desde el nivel 50, +1d6 (como el
        # +(N x5) del Berserker). La velocidad, fuera
        carta("H928", "Espíritu de lobo", "all_allies",
              "Llamas al espíritu del lobo: tú y tus aliados hacéis +1d6 con los golpes de arma hasta el final "
              "del combate.",
              [("Espíritu del lobo: +1d6 a los golpes con arma todo el combate.",
                estado(300, "bono_dano_arma:1d6"))]),
        # Perdida de los elementos (+30 % de recibir un ataque fatal, resistencias -25) y el +15 % de la
        # Maldicion elemental: por debajo del 50 % del critico, y las resistencias no se simulan
        carta("H929", "Down elemental", "all_enemies",
              "Absorbes el espíritu de la naturaleza de los enemigos: -1 a su CA durante 2 turnos.",
              [("Maldición elemental: -1 a la CA durante 2 turnos.", estado(15, "penal_ca:1"))]),
    ],
    "creador": [
        carta("H930", "Hiedra", "all_enemies",
              "Una lluvia de bumeranes a tu alrededor: 4d6 de daño de luz a cada enemigo y shock 1 turno (no "
              "atacan y se defienden con desventaja).",
              [("4d6 de daño de luz a cada enemigo.", dano(1200, 1500))] + _aturdido(5, "Shock"), elemento="Luz"),
        # Espiritu del oso: HP y MP maximos +10 %; la habilidad dice «aumenta la fuerza de defensa»: +1 a
        # la CA, como la Suplica de defensa del Cruzado (el MP no se simula)
        carta("H931", "Espíritu de oso", "all_allies",
              "Llamas al alma del oso: tú y tus aliados tenéis +1 a la CA hasta el final del combate.",
              [("Espíritu del oso: +1 a la CA todo el combate.", estado(300, "bono_ca:1"))]),
        carta("H932", "Fuerza del bosque", "all_enemies",
              "Un bumerán enorme con el espíritu de la naturaleza: 4d6 de daño de naturaleza a cada enemigo, y su "
              "defensa se parte: -2 a la CA durante 3 turnos.",
              # Cortar: el MP que tira no se simula
              [("4d6 de daño de naturaleza a cada enemigo.", dano(1700, 1800)),
               ("Cortar: -2 a la CA durante 3 turnos.", estado(20, "penal_ca:2"))], elemento="Naturaleza"),
    ],
    # El catalogo no tiene bumeran de tier 1: uno normal, como las de Viaje.
    "arma_inicial": "bumeran_de_viaje",
    "armas_nuevas": [
        arma_comun("Bumerán de Viaje", "ligera", "Un bumerán de madera curvada que vuelve a la mano.",
                   restricciones=["Precisión: DES"]),
    ],
    "set": [
        pieza("Bumerán del Guardabosques", "MainHand", "ligera", {"DES": 1, "CAR": 1},
              "El bumerán de la SP del Guardabosques: madera tallada que vuelve con la luz."),
        pieza("Túnica del Guardabosques", "Torso", "ligera", {"DES": 1},
              "Túnica verde con el manto de hojas y la piel."),
        pieza("Diadema del Guardabosques", "Head", "ligera", {"DES": 1, "CAR": 1},
              "Diadema de cuero con astas de ciervo."),
        pieza("Calzas del Guardabosques", "Legs", "ligera", {"DES": 1, "CAR": 1},
              "Calzas verdes del Guardabosques."),
        pieza("Botas del Guardabosques", "Feet", "ligera", {"DES": 1, "CAR": 1},
              "Botas de piel del Guardabosques."),
    ],
}

SERVIDOR_DEL_CANON = {
    "id": "servidor_del_canon_de_fuego", "name": "Servidor del cañón de fuego", "sp": 5, "elemento": "Fuego",
    "nostale": "Servidor del cañón de fuego (Fire Cannoneer)",
    "description": ("Especialista del cañón de fuego: bombardea desde lejos, hace estallar cohetes sobre los grupos y "
                    "lleva el arma al límite."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "balanced",
    # El ataque básico (H1067) se llama «Disparo de cañón», como la carta H1070: la pasiva toma el nombre de su
    # ficha («ataque de bombardeo básico»).
    "pasiva": ("Bombardeo",
               "El bombardeo a distancia del Cañonero: tus golpes con arma hacen +1 de daño (fuego).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    # «La potenciación de las explosiones» que citan Disparo con objetivo, Fuego infernal, Cohete de señal, Montar
    # en cohete y Cañón supremo no tiene cifras en la ficha (§10): nada.
    "cartas": [
        carta("H1068", "Disparos retardados", "zone_radius",
              "Tres balas de cañón, alta, baja y media, llegan a la vez: 2d6 de daño de fuego.",
              # shock de explosión, quemadura e «increíble», al 40 %: fuera
              [("2d6 de daño de fuego.", dano(220, 380))], elemento="Fuego"),
        carta("H1069", "Desviamiento de cañón", "zone_radius",
              "Encaras el cañón al enemigo y disparas de cerca: 2d6 de daño de fuego y -1 a su CA durante 3 turnos.",
              [("2d6 de daño de fuego.", dano(400, 500)),
               ("Debilita su defensa: -1 a la CA durante 3 turnos.", estado(30, "penal_ca:1"))], elemento="Fuego"),
        carta("H1070", "Disparo de cañón", "single_enemy",
              "Un cañonazo violento de sal de fuego: 3d6 de daño de fuego y aturdimiento 1 turno (no ataca y se "
              "defiende con desventaja).",
              [("3d6 de daño de fuego.", dano(500, 1000))] + _aturdido(5), elemento="Fuego"),
        carta("H1071", "Disparo con objetivo", "zone_radius",
              "Un disparo desde muy lejos que estalla sobre el blanco: 3d6 de daño de fuego, y la onda expansiva le "
              "deja 1 turno sin atacar.",
              [("3d6 de daño de fuego.", dano(900, 700)),
               ("Gran onda expansiva: 1 turno sin atacar.", estado(4, "incapacitado"))], elemento="Fuego"),
        carta("H1072", "Granada de mano", "zone_radius",
              "Lanzas una granada alquímica de mano: 3d6 de daño de fuego, y la onda expansiva le deja 1 turno sin "
              "atacar.",
              [("3d6 de daño de fuego.", dano(800, 800)),
               ("Gran onda expansiva: 1 turno sin atacar.", estado(4, "incapacitado"))], elemento="Fuego"),
        carta("H1073", "Fuego infernal", "zone_radius",
              "Un cohete estalla sobre el blanco y los que tiene cerca: 3d6 de daño de fuego.",
              # Quemadura fatal (60 %): resistencia al fuego y +30 % de golpe fatal, que no llega al 50 % del
              # diccionario (§5): nada que entre
              [("3d6 de daño de fuego.", dano(650, 900))], elemento="Fuego"),
        carta("H1074", "Cohete de señal", "zone_radius",
              "El cohete de señal estalla y bombardea la zona: 3d6 de daño de fuego, y la onda expansiva le deja 1 "
              "turno sin atacar.",
              [("3d6 de daño de fuego.", dano(700, 1200)),
               ("Gran onda expansiva: 1 turno sin atacar.", estado(4, "incapacitado"))], elemento="Fuego"),
    ],
    "creador": [
        carta("H1075", "Montar en cohete", "zone_radius",
              "Montas en un cohete, arrasas a los que tienes cerca y vuelves: 3d6 de daño de fuego y shock 1 turno "
              "(no ataca y se defiende con desventaja).",
              # te teletransporta 8 casillas: sin posiciones
              [("3d6 de daño de fuego.", dano(600, 700))] + _aturdido(2, "Shock"), elemento="Fuego"),
        carta("H1076", "Maestro de las armas", "self",
              "Llevas el cañón al límite hasta el final del combate: atacas con ventaja, tus golpes con arma hacen +1 "
              "y tienes +1 a la CA.",
              # Refuerzo de la explosión (90 s): más radio (sin posiciones) y acierto, la ventaja que ya da el
              # Cargador turbo; la reducción de enfriamiento y la resistencia al fuego del rival, fuera
              [("Cargador turbo: ventaja en ataque todo el combate.", estado(120, "ventaja:ataque")),
               ("Cargador turbo: +1 a tus golpes con arma todo el combate.", estado(120, "bono_dano_arma:1")),
               ("Defensa con el arma: +1 a la CA todo el combate.", estado(120, "bono_ca:1"))]),
        carta("H1077", "Cañón supremo", "all_enemies",
              "El remate del cañón: 4d6 de daño de fuego a cada enemigo; la gran onda expansiva los deja 1 turno sin "
              "atacar y, chamuscados, tus golpes les hacen +1 durante 2 turnos.",
              # Chamuscado: el daño de fuego que reciben puede subir un 30 % (daño recibido +: bono_dano_contra,
              # como la Herida interna del Monje); la resistencia al fuego, fuera
              [("4d6 de daño de fuego a cada enemigo.", dano(1100, 1500)),
               ("La gran onda expansiva lo destroza todo: 1 turno sin atacar.", estado(6, "incapacitado")),
               ("Chamuscado: tus golpes les hacen +1 durante 2 turnos.", estado(10, "bono_dano_contra:1"))],
              elemento="Fuego"),
    ],
    # El catálogo no tiene cañón de tier 1: uno normal, como los de Viaje.
    "arma_inicial": "canon_de_mano_de_viaje",
    "armas_nuevas": [
        arma_comun("Cañón de Mano de Viaje", "pesada",
                   "Un cañón de mano de bronce que dispara cargas de sal de fuego, a dos manos.",
                   restricciones=["Precisión: DES", DOS_MANOS]),
    ],
    # El set, como el Fire Cannoneer de los videos de Oriol (lamina como_se_coge_cada_arma): el gorro de
    # aviador con las gafas de vuelo y la chaqueta de cuero (actores_trajes_arquero).
    # El nombre de la SP es largo: el set y sus piezas son «del Cañonero» (set_de, en la etiqueta del set).
    "set_de": "Cañonero",
    "set": [
        pieza("Cañón del Cañonero", "MainHand", "pesada", {"DES": 1, "CON": 1},
              "El cañón de la SP del Servidor del cañón de fuego: bronce y hierro negro, cargado con sal de fuego, a "
              "dos manos.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Chaqueta del Cañonero", "Torso", "media", {"CON": 1},
              "Chaqueta de cuero de aviador con el cuello de borrego, la bufanda roja y la correa cruzada."),
        pieza("Gorro de aviador del Cañonero", "Head", "ligera", {"DES": 1, "CON": 1},
              "Gorro de cuero con orejeras y las gafas de vuelo subidas."),
        pieza("Pantalones del Cañonero", "Legs", "ligera", {"DES": 1, "CON": 1},
              "Pantalones de artillero."),
        pieza("Botas del Cañonero", "Feet", "media", {"DES": 1, "CON": 1},
              "Botas altas de artillero."),
    ],
}

ESPIA = {
    "id": "espia", "name": "Espía", "sp": 6, "elemento": "Agua", "nostale": "Espía (Scout)",
    "description": ("Especialista de la ballesta de agua: su halcón caza al más débil, se esconde en emboscada y "
                    "remata de lejos como francotirador."),
    "primaryStat": "DES", "secondaryStat": "INT", "role": "agile",
    "pasiva": ("Ataque elemental",
               "El virote de agua del Espía: tus golpes con arma hacen +1 de daño (agua).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        # Vuelo en picado de halcón abre Flecha rotatoria - nivel máximo (H1127, su continuación): una carta (§2).
        carta("H1115+H1127", "Vuelo en picado de halcón", "all_enemies",
              "El halcón cae en picado y abre paso a la flecha rotatoria de nivel máximo: 2d6 de daño de agua a cada "
              "enemigo.",
              # rigidez (10 %) y veneno suave (15 %) de la flecha: fuera
              [("2d6 de daño de agua a cada enemigo.", dano(200, 300, 450, 300))], elemento="Agua"),
        # Una trampa estalla al jugar la carta (sin posiciones), pero la ficha de la Trampa móvil no da daño ni
        # estado: no hay nada que hacer estallar.
        carta("H1116", "Trampa móvil", "self",
              "Colocas una trampa móvil para tus enemigos. [pendiente] La ficha no dice qué les hace.",
              [("[pendiente] Lo que hace la trampa.", "pendiente:lo que hace la trampa móvil (la ficha no lo dice)")]),
        # Ni bonificación ni estado: el ataque del halcón no tiene cifras.
        carta("H1117", "Espiar", "single_enemy",
              "Tu halcón sigue y acecha al enemigo que elijas para atacarlo desde el aire. [pendiente] La ficha no da "
              "el daño ni el efecto del halcón.",
              [("[pendiente] El ataque del halcón.",
                "pendiente:el ataque del halcón desde el aire (la ficha no da cifras)")], elemento="Agua"),
        carta("H1118", "Retroceso", "single_enemy",
              "Disparas al enemigo y retrocedes de un salto: 3d6 de daño de agua.",
              # retroceder 8 casillas: sin posiciones; veneno de Mandra (30 %): fuera
              [("3d6 de daño de agua.", dano(800, 550))], elemento="Agua"),
        carta("H1119", "Fuerza vital", "self",
              "La energía del bosque refuerza tu fuerza vital: atacas con ventaja hasta el final del combate.",
              # Ojo de águila (120 s): más alcance (no se simula) y acierto, la misma ventaja; la protección de la
              # Fuerza vital contra las enfermedades, fuera
              [("Fuerza vital: ventaja en ataque todo el combate.", estado(300, "ventaja:ataque")),
               _pendiente("el Escudo vital, que limita el daño de los críticos que recibes 1 turno")]),
        carta("H1120", "Flecha de niebla", "single_enemy",
              "Conviertes al enemigo en una nube de vapor que le quita la vista: ataca con desventaja 1 turno.",
              # La Nube de vapor no tiene ficha ni segundos: 1 turno, lo mínimo (como «se fijarán en ti»)
              [("Nube de vapor: ataca con desventaja 1 turno.", "estado:1:desventaja:ataque"),
               _pendiente("cuánto dura la nube de vapor (la ficha no da los segundos)")]),
        carta("H1121", "Sentidos aguzados del halcón", "single_enemy",
              "Los sentidos del halcón encuentran al enemigo más débil: 3d6 de daño de agua, y queda marcado 2 "
              "turnos.",
              # «área de 4 casillas», pero el halcón ataca solo al que tiene menos vida: un objetivo (lo eliges tú)
              [("3d6 de daño de agua.", dano(850, 900)),
               ("En la retícula del francotirador: marcado 2 turnos.", condicion("marcado", 12))], elemento="Agua"),
    ],
    "creador": [
        # Recarga rápida abre Flecha rotatoria - nivel 1 (H1125) y este el nivel 2 (H1126), que vuelve al 1 (hasta
        # cinco vueltas): una carta que suma cada paso una vez (§2), como Siete punzadas.
        carta("H1122+H1125+H1126", "Recarga rápida", "zone_radius",
              "Recargas a toda prisa y encadenas la flecha rotatoria: 2d6 de daño de agua, y la evasión rápida hace "
              "que te defiendas con ventaja 1 turno.",
              # venenos de Madra (30 % y 20 %) y rigidez (35 %): fuera. La Emboscada - Preparación 2 que deja el
              # nivel 2 da alcance y daño al disparo desde la emboscada («daños adicionales en un ataque
              # escondido»): no se simula
              [("2d6 de daño de agua.", dano(350, 400, 200, 200)),
               ("Evasión rápida: te defiendes con ventaja 1 turno.", propio(2, "ventaja:defensa"))], elemento="Agua"),
        # Ocultación: te defiendes con ventaja y el golpe desde la emboscada sale con ventaja. La Emboscada dura 0 s
        # en la ficha (hasta que atacas desde ella): 2 rondas, para que llegue a tu turno siguiente (con 1 se
        # acabaría al cerrar esta), como lo que gana una reacción. Te teletransporta: sin posiciones.
        carta("H1123", "Emboscada", "single_enemy",
              "Desapareces sin dejar rastro y acechas al enemigo: durante 2 turnos te defiendes y atacas con ventaja.",
              [("Emboscada: te defiendes con ventaja 2 turnos.", "propio:2:ventaja:defensa"),
               ("Emboscada: atacas con ventaja 2 turnos.", "propio:2:ventaja:ataque")]),
        carta("H1124", "Francotirador", "zone_radius",
              "Disparas desde muy lejos: 3d6 de daño de agua, y la rigidez de Mandra le deja 1 turno sin atacar.",
              # el crítico seguro desde la Emboscada («daños adicionales en un ataque escondido»): no se simula, la
              # Emboscada ya da ventaja al golpe; veneno mortífero de Mandra (45 %): fuera
              [("3d6 de daño de agua.", dano(1200, 1000)),
               ("Rigidez de Mandra: 1 turno sin atacar.", estado(2, "incapacitado"))], elemento="Agua"),
    ],
    # Trampa móvil y Espiar salen bloqueadas: arrancan las tres primeras que se juegan.
    "arranque": ["Vuelo en picado de halcón", "Retroceso", "Fuerza vital"],
    "arma_inicial": "ballesta_ligera_de_milicia",
    "set": [
        pieza("Ballesta del Espía", "MainHand", "media", {"DES": 1, "INT": 1},
              "La ballesta de la SP del Espía: ligera y precisa, con virotes de agua."),
        pieza("Gabardina del Espía", "Torso", "ligera", {"DES": 1},
              "Gabardina corta con el halcón posado en el hombro."),
        pieza("Gorro del Espía", "Head", "ligera", {"DES": 1, "INT": 1},
              "Gorro con plumas de halcón."),
        pieza("Calzas del Espía", "Legs", "ligera", {"DES": 1, "INT": 1},
              "Calzas ajustadas de explorador."),
        pieza("Botas del Espía", "Feet", "ligera", {"DES": 1, "INT": 1},
              "Botas blandas de explorador."),
    ],
}

CAZADEMONIOS = {
    "id": "cazademonios", "name": "Cazademonios", "sp": 7, "elemento": "Tinieblas",
    "nostale": "Cazademonios (Demon Hunter)",
    "description": ("Especialista del revólver de tinieblas: dispara balas rúnicas, desata a los espíritus de la "
                    "venganza y posee a sus enemigos."),
    "primaryStat": "DES", "secondaryStat": "CAR", "role": "agile",
    "pasiva": ("Tiro del diablo",
               "Los disparos del revólver del Cazademonios: tus golpes con arma hacen +1 de daño (tinieblas).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H1155", "Lluvia de balas", "single_enemy",
              "Apuntas y vacías el revólver de balas rúnicas a toda velocidad: 1d6 de daño de tinieblas.",
              # herida de bala (10 %): fuera
              [("1d6 de daño de tinieblas.", dano(180, 250))], elemento="Tinieblas"),
        # Hoja de tiro abre Péndulo de la sombra lunar (H1167, su continuación, al 70 %): una carta (§2).
        carta("H1156+H1167", "Hoja de tiro", "single_enemy",
              "Disparas y la sombra lunar atraviesa al enemigo: 3d6 de daño de tinieblas, -2 a su CA durante 3 "
              "turnos y te curas el daño que haces.",
              # herida de bala (15 %): fuera
              [("3d6 de daño de tinieblas.", dano(330, 450, 900, 300)),
               ("Cortar: -2 a la CA durante 3 turnos.", estado(20, "penal_ca:2")),
               ("Te curas el daño que haces.", "cura:dano_causado")], elemento="Tinieblas"),
        carta("H1157", "Escalofrío del espíritu de la venganza", "zone_radius",
              "Lanzas al cielo el espíritu de la venganza y cae sobre la zona: 2d6 de daño de tinieblas.",
              # aturdimiento (10 %): fuera
              [("2d6 de daño de tinieblas.", dano(450, 700))], elemento="Tinieblas"),
        # Péndulo del espíritu de la venganza 1 abre el 2 (H1165) y este el 3 (H1166): una carta que suma los
        # tres (§2).
        carta("H1158+H1165+H1166", "Péndulo del espíritu de la venganza", "all_enemies",
              "Corres y acometes tres veces con el espíritu de la venganza: 2d6 de daño de tinieblas a cada enemigo; "
              "durante 1 turno tienes +1 a la CA y te defiendes con ventaja, y te curas el daño que haces.",
              # empujar 4 casillas (60 %, solo PvP): fuera
              [("2d6 de daño de tinieblas a cada enemigo.", dano(100, 200, 200, 200, 300, 200)),
               ("Péndulo del espíritu de la venganza: +1 a tu CA 1 turno.", propio(4, "bono_ca:1")),
               ("Péndulo del espíritu de la venganza: te defiendes con ventaja 1 turno.",
                propio(4, "ventaja:defensa")),
               ("Te curas el daño que haces.", "cura:dano_causado")], elemento="Tinieblas"),
        carta("H1159", "Asalto del espíritu de la venganza", "single_enemy",
              "Disparas los espíritus de la venganza contra un enemigo: 3d6 de daño de tinieblas.",
              # «haz clic sobre 5 bloques del oponente» (el minijuego): nada; aturdimiento mortal (30 %): fuera
              [("3d6 de daño de tinieblas.", dano(800, 800))], elemento="Tinieblas"),
        carta("H1160", "Espíritu de la furia", "self",
              "El espíritu de la furia te envuelve: tus golpes con arma hacen +1 hasta el final del combate.",
              [("Espíritu de la furia débil: +1 a tus golpes con arma todo el combate.",
                estado(300, "bono_dano_arma:1")),
               _pendiente("que se refuerce al recibir daño (la ficha solo da el espíritu débil)")]),
        carta("H1161", "Conjuro de los espíritus", "zone_radius",
              "Disparas en línea recta y conjuras a los espíritus contra el enemigo: 3d6 de daño de tinieblas.",
              # aturdimiento mortal (20 %): fuera
              [("3d6 de daño de tinieblas.", dano(650, 950))], elemento="Tinieblas"),
    ],
    "creador": [
        carta("H1162", "Explosión de espíritus", "all_enemies",
              "Liberas espíritus poderosísimos: 3d6 de daño de tinieblas a cada enemigo y te curas el daño que haces; "
              "quedan rígidos 1 turno (no atacan).",
              [("3d6 de daño de tinieblas a cada enemigo.", dano(600, 900)),
               ("Te curas el daño que haces.", "cura:dano_causado"),
               ("Rigidez: 1 turno sin atacar.", estado(3, "incapacitado"))], elemento="Tinieblas"),
        carta("H1163", "Posesión", "single_enemy",
              "Conviertes al enemigo en huésped de un espíritu: pierde 1 de vida por turno durante 2 turnos.",
              # Posesión fuerte (de enfermedad, no mágica: sin salvación): la vida que le quita cada 2 s, como una
              # maldición; MP y velocidad, fuera
              [("Posesión fuerte: 1 de daño por turno durante 2 turnos.", estado(10, "dano_por_turno:1")),
               _pendiente("que la posesión pase a los enemigos cercanos al acabar")]),
        carta("H1164", "Cuchilla de almas", "all_enemies",
              "Invocas la cuchilla de almas: 3d6 de daño de tinieblas a cada enemigo y te curas el daño que haces; la "
              "desesperanza los deja sin defensa (se defienden con desventaja) y tus golpes les hacen +1 durante 3 "
              "turnos.",
              # Desesperanza (50 %): sin defensa y +30 % al daño de los críticos que recibe (daño recibido +:
              # bono_dano_contra, como la Herida interna del Monje); la resistencia a las tinieblas, fuera
              [("3d6 de daño de tinieblas a cada enemigo.", dano(1200, 1200)),
               ("Desesperanza: se defienden con desventaja 3 turnos.", estado(25, "desventaja:defensa")),
               ("Desesperanza: tus golpes les hacen +1 durante 3 turnos.", estado(25, "bono_dano_contra:1")),
               ("Te curas el daño que haces.", "cura:dano_causado")], elemento="Tinieblas"),
    ],
    # El catálogo no tiene revólver de tier 1: uno normal, como los de Viaje.
    "arma_inicial": "revolver_de_viaje",
    "armas_nuevas": [
        arma_comun("Revólver de Viaje", "ligera", "Un revólver sencillo de seis balas rúnicas.",
                   restricciones=["Precisión: DES"]),
    ],
    "set": [
        pieza("Revólver del Cazademonios", "MainHand", "ligera", {"DES": 1, "CAR": 1},
              "El revólver de la SP del Cazademonios: cañón largo grabado con runas."),
        pieza("Abrigo del Cazademonios", "Torso", "ligera", {"DES": 1},
              "Abrigo largo y abierto con el forro rojo, sobre la camisa blanca con correas."),
        pieza("Sombrero del Cazademonios", "Head", "ligera", {"DES": 1, "CAR": 1},
              "Sombrero negro de ala ancha."),
        pieza("Pantalones del Cazademonios", "Legs", "ligera", {"DES": 1, "CAR": 1},
              "Pantalones negros de cazador."),
        pieza("Botas del Cazademonios", "Feet", "ligera", {"DES": 1, "CAR": 1},
              "Botas negras con hebillas."),
    ],
}

ANGEL_VENGADOR = {
    "id": "angel_vengador", "name": "Ángel vengador", "sp": 8, "elemento": "Luz",
    "nostale": "Ángel vengador (Avenging Angel)",
    "description": ("Especialista de la espada de luz: corta de cerca y de lejos, absorbe con su espada la energía "
                    "de los elementos y remata con espadas divinas."),
    "primaryStat": "DES", "secondaryStat": "CAR", "role": "balanced",
    # Llama, Hielo, Halo, Oscuridad y Sin elemento (H1191-H1195) son energías del mismo sistema, no cartas: la
    # pasiva (§6), como las meditaciones del Monje. En NosTale duran poco (2 s; Oscuridad 6 s, Sin elemento 12 s):
    # con sus segundos, 1 turno (Sin elemento, 2) desde que la eliges (si:<op>@<rondas>, como el invierno del
    # Bardo). Elegir no gasta el turno y se puede aplazar: se elige cuando hace falta.
    # Llama devuelve el 20 % del daño y te libra de estados: refleja (§5). Hielo convierte el ataque del rival en
    # fuerza propia, como el Protector de cuchillas del Ninja: +1d6. Halo cura un 7 %: la pasiva no cura. Oscuridad
    # te hace invisible (ocultación: ventaja en defensa); su teletransporte, sin posiciones. Sin elemento: daño
    # recibido -10 % (bono_ca:1); la defensa elemental y lo de PvP, fuera.
    "pasiva": ("Energía de los elementos",
               "Una vez por combate, cuando quieras, tu espada absorbe un elemento. Llama: 1 turno, los golpes "
               "físicos rebotan contra quien te ataca. Hielo: 1 turno, la fuerza del rival pasa a tu espada: +1d6 a "
               "tus golpes con arma. Halo: recuperas un 7 % de vida (pendiente). Oscuridad: 1 turno invisible, te "
               "defiendes con ventaja. Sin elemento: +1 a la CA durante 2 turnos.",
               [("Eliges una energía una vez por combate.",
                 elige_una("llama", "hielo", "halo", "oscuridad", "sin elemento")),
                ("Llama: reflejas los golpes físicos 1 turno.", si(f"llama@{rondas(2)}", "refleja:fisico")),
                ("Hielo: +1d6 a tus golpes con arma 1 turno.", si(f"hielo@{rondas(2)}", "bono_dano_arma:1d6")),
                _pendiente("que el Halo te cure un 7 % de vida"),
                ("Oscuridad: te defiendes con ventaja 1 turno.", si(f"oscuridad@{rondas(6)}", "ventaja:defensa")),
                ("Sin elemento: +1 a la CA 2 turnos.", si(f"sin elemento@{rondas(12)}", "bono_ca:1"))]),
    "cartas": [
        carta("H1181", "Corte huidizo", "single_enemy",
              "Lanzas la espada, que corta una y otra vez: 1d6 de daño de luz.",
              [("1d6 de daño de luz.", dano(175, 230))], elemento="Luz"),
        carta("H1182", "Explosión aguzada", "all_enemies",
              "El camino de la espada es el de la explosión: 2d6 de daño de luz a cada enemigo.",
              # su +50 % de crítico es del propio golpe: no entra (ver arriba)
              [("2d6 de daño de luz a cada enemigo.", dano(400, 500))], elemento="Luz"),
        carta("H1183", "Espada del juicio", "all_enemies",
              "La espada toma el poder de la luz: 2d6 de daño de luz a cada enemigo.",
              # aturdimiento corto (30 %): fuera
              [("2d6 de daño de luz a cada enemigo.", dano(550, 700))], elemento="Luz"),
        carta("H1184", "Atadura sombría", "single_enemy",
              "Sujetas al enemigo con las sombras: 2d6 de daño de luz.",
              # «es imposible moverse» (75 %): sin posiciones, nada
              [("2d6 de daño de luz.", dano(250, 380))], elemento="Luz"),
        carta("H1185", "Cuchilla relámpago", "all_enemies",
              "Clavas la espada en el suelo: 2d6 de daño de rayo a cada enemigo, y la descarga eléctrica les hace 1 de "
              "daño por turno durante 1 turno.",
              # Descarga eléctrica (50 %): no se mueve (sin posiciones) y pierde vida, como con el fuego
              [("2d6 de daño de rayo a cada enemigo.", dano(380, 450)),
               ("Descarga eléctrica: 1 de daño por turno durante 1 turno.", estado(5, "dano_por_turno:1"))],
              elemento="Rayo"),
        carta("H1186", "Carrera y corte", "single_enemy",
              "Le hieres en las piernas para frenarlo: 2d6 de daño de luz y se defiende con desventaja 2 turnos.",
              # Carrera y corte (80 %): velocidad (no se simula) y esquiva reducida, el reverso de «esquiva +»
              # (ventaja:defensa)
              [("2d6 de daño de luz.", dano(300, 350)),
               ("Carrera y corte: se defiende con desventaja 2 turnos.", estado(10, "desventaja:defensa"))],
              elemento="Luz"),
        carta("H1187", "Caldero mágico", "self",
              "Tu espada absorbe el poder del caldero mágico: +1 a la CA hasta el final del combate.",
              # las energías que abre (Llama, Hielo, Halo, Oscuridad y Sin elemento) son la pasiva; la velocidad,
              # fuera
              [("Caldero mágico: +1 a la CA todo el combate.", estado(300, "bono_ca:1"))]),
    ],
    "creador": [
        carta("H1188", "Punzada meteorito", "all_enemies",
              "Lanzas la espada y estalla con un golpe de luz: 3d6 de daño de luz a cada enemigo y shock 1 turno (no "
              "atacan y se defienden con desventaja).",
              [("3d6 de daño de luz a cada enemigo.", dano(650, 900))] + _aturdido(5, "Shock"), elemento="Luz"),
        carta("H1189", "Espada divina", "all_enemies",
              "Una espada gigantesca taladra el suelo: 3d6 de daño de luz a cada enemigo, y su punto débil queda al "
              "descubierto: los golpes contra ellos son críticos 1 turno.",
              [("3d6 de daño de luz a cada enemigo.", dano(800, 1100)),
               ("Punto débil al descubierto: los golpes contra ellos son críticos 1 turno.",
                estado(5, "critico_contra"))], elemento="Luz"),
        carta("H1190", "Pincho fantasmal", "all_enemies",
              "Nueve filos fantasmales hieren una y otra vez: 4d6 de daño de luz a cada enemigo, que sangra 2 turnos.",
              [("4d6 de daño de luz a cada enemigo.", dano(1300, 1250)),
               ("Hemorragia mortal: sangran 2 turnos.", condicion("sangrado", 10))], elemento="Luz"),
    ],
    "arma_inicial": "item_short_sword",
    "set": [
        pieza("Espada del Ángel vengador", "MainHand", "media", {"DES": 1, "CAR": 1},
              "La espada de la SP del Ángel vengador: hoja blanca de luz con la guarda de oro."),
        pieza("Armadura del Ángel vengador", "Torso", "media", {"DES": 1},
              "Armadura blanca y oro con dos alas pequeñas a la espalda."),
        pieza("Diadema del Ángel vengador", "Head", "ligera", {"DES": 1, "CAR": 1},
              "Aro de oro con dos alas pequeñas sobre la frente."),
        pieza("Grebas del Ángel vengador", "Legs", "media", {"DES": 1, "CAR": 1},
              "Grebas blancas con ribetes de oro."),
        pieza("Botas del Ángel vengador", "Feet", "ligera", {"DES": 1, "CAR": 1},
              "Botas blancas del Ángel vengador."),
    ],
}

CAZASOLES = {
    "id": "cazasoles", "name": "Cazasoles", "sp": 9, "elemento": "Luz", "nostale": "Cazasoles (Sunchaser)",
    "description": ("Especialista de las jabalinas de luz: lanza desde lejos, se cura y lucha junto al Lobo solar, "
                    "que acomete, patea y gira a sus órdenes."),
    "primaryStat": "DES", "secondaryStat": "CAR", "role": "support",
    "pasiva": ("Lanzamiento",
               "Las lanzas del Cazasoles: tus golpes con arma hacen +1 de daño (luz).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("que las órdenes al Lobo solar solo salgan con el lobo en pie")]),
    # En NosTale el lobo va siempre con la SP; aqui lo trae Curacion natural, la unica de la tarjeta que lo pone
    # en pie («si ya esta derrotado, lo resucita con el 100 % de sus HP»). NosTale no publica sus cifras: las de
    # un lobo de tier 1 del bestiario (Lobo de Cripta: CON 2, DES 4, CAR 1; INT 0, como las
    # invocaciones; el cargador lee CON, DES, INT y CAR) y los 5 PV de las invocaciones de tier 1 (Lobo de Ceniza).
    "invocaciones": [
        {"id": "lobo_solar", "name": "Lobo solar", "description": "CON 2, DES 4, INT 0, CAR 1, 5 PV.",
         "health": 5, "duration": "hasta el final del combate", "summonedBy": "cazasoles_curacion_natural",
         "stats": {"CON": 2, "DES": 4, "INT": 0, "CAR": 1},
         # sus tecnicas (H1651-H1655) son las ordenes del Cazasoles: el dano lo ponen esas cartas
         "attacks": [
             {"name": "Orden de ataque fugaz",
              "effect": "Se abalanza sobre el enemigo (la carta Orden de ataque fugaz)."},
             {"name": "Patada animal",
              "effect": "Una patada que aparta y aturde al enemigo (la carta Patada animal)."},
             {"name": "Manada de lobos",
              "effect": "Las almas de sus antepasados atacan al enemigo (la carta Grilletes)."},
             {"name": "Erupción solar",
              "effect": "Una luz cegadora que aturde al enemigo (la carta Erupción solar)."},
             {"name": "Lobo torbellino",
              "effect": "Gira sobre sí mismo y ataca al enemigo y a los cercanos (la carta Red lumínica)."},
         ],
         "passive": {"name": "Órdenes del Cazasoles",
                     "description": ("Lucha a tu lado hasta el final del combate: ataca en cada ronda y cumple las "
                                     "órdenes de tus cartas.")}},
    ],
    "cartas": [
        # La orden (H1393) abre la acometida del lobo (H1651): una carta (§2). El «posible aturdimiento» de la
        # tarjeta es, en la ficha del lobo, Hemorragia Mortal (80 %).
        carta("H1393+H1651", "Orden de ataque fugaz", "single_enemy",
              "El Lobo solar se abalanza sobre el enemigo: 2d6 de daño de luz, y sangra 2 turnos.",
              [("2d6 de daño de luz.", dano(0, 0, 500, 300)),
               ("Hemorragia mortal: sangra 2 turnos.", condicion("sangrado", 10))], elemento="Luz"),
        # Relampago abre Retirada inmediata (H1634): una carta (§2). La retirada no suma bonificacion: te
        # teletransportas (sin posiciones) y la trampa para osos no tiene ficha.
        carta("H1394+H1634", "Relámpago", "single_enemy",
              "Te lanzas sobre el enemigo, le atacas una y otra vez y te retiras de un salto: 2d6 de daño de luz, "
              "y sangra 2 turnos.",
              # Broma letal (70 %): pierde vida cada 2 s, sangra; que dure mas si le ataca el lobo, no
              [("2d6 de daño de luz.", dano(400, 200)),
               ("Broma letal: sangra 2 turnos.", condicion("sangrado", 8))], elemento="Luz"),
        carta("H1395", "Lanza solar", "zone_radius",
              "Tiras una lanza gigante que crece por arte de magia: 3d6 de daño de luz.",
              # 35 % de punto debil al descubierto: fuera; el +2 % por la distancia: sin posiciones
              [("3d6 de daño de luz.", dano(700, 600))], elemento="Luz"),
        # La carta que trae al Lobo solar («invocaciones»). Curacion natural (4 s): la cura y quitar los efectos
        # daninos; los HP de mas (el 20 % de los del lobo), fuera.
        carta("H1396", "Curación natural", "self",
              "La naturaleza te sana: te libras de los estados en contra y te curas 1d6, y el Lobo solar se levanta "
              "a tu lado hasta el final del combate.",
              [("El Lobo solar lucha a tu lado hasta el final del combate.", "invoca:combate:lobo_solar"),
               ("Te libras de los estados en contra.", "limpia"), ("Te curas 1d6.", "cura:1d6")]),
        carta("H1397", "Meteoro solar", "all_enemies",
              "Del cielo caen cuatro lanzas relucientes: 3d6 de daño de luz a cada enemigo; con el punto débil al "
              "descubierto, se defienden con desventaja y tus golpes les hacen +1 durante 2 turnos.",
              # Poner al descubierto el punto debil (50 %): esquiva -50 % y +10 % al dano a distancia; el +10 % de
              # golpe fatal no llega a critico_contra
              [("3d6 de daño de luz a cada enemigo.", dano(800, 900)),
               ("Punto débil al descubierto: se defienden con desventaja 2 turnos.",
                estado(10, "desventaja:defensa")),
               ("Punto débil al descubierto: tus golpes les hacen +1 durante 2 turnos.",
                estado(10, "bono_dano_contra:1"))], elemento="Luz"),
        # La orden (H1398) abre la patada del lobo (H1652): una carta (§2). Empujarlo: sin posiciones.
        carta("H1398+H1652", "Patada animal", "single_enemy",
              "El Lobo solar salta sobre el enemigo y lo aparta de una patada: 3d6 de daño de luz y aturdimiento 1 "
              "turno (no ataca y se defiende con desventaja).",
              [("3d6 de daño de luz.", dano(0, 0, 700, 600))] + _aturdido(5), elemento="Luz"),
        # Grilletes abre Manada de lobos (H1635, una orden que el lobo cumple con la suya, H1653): una carta que
        # suma las tres (§2). El 70 % de no poder moverse: velocidad, no se simula; la Luz danina de la manada no
        # llega.
        carta("H1399+H1635+H1653", "Grilletes", "zone_radius",
              "La lanza se vuelve un cepo y el Lobo solar llama a las almas de su manada: 3d6 de daño de luz.",
              [("3d6 de daño de luz.", dano(500, 500, 0, 0, 800, 600))], elemento="Luz"),
    ],
    # Creador: las de profesion 12, 16 y 20, cada una con sus continuaciones.
    "creador": [
        # Erupcion solar (una orden) abre la del lobo (H1654) y Explosion de la mancha solar (H1636): una carta
        # que suma las tres (§2). Las consecuencias de la explosion (50 %: resistencia a la luz, y Luz danina un
        # 20 % de las veces): nada que entre.
        carta("H1584+H1654+H1636", "Erupción solar", "single_enemy",
              "El Lobo solar aparece junto al enemigo y lo ciega con su luz, y tu lanza de energía solar estalla: "
              "4d6 de daño de luz y aturdimiento 1 turno (no ataca y se defiende con desventaja).",
              [("4d6 de daño de luz.", dano(0, 0, 800, 700, 1000, 900))] + _aturdido(5), elemento="Luz"),
        # En NosTale, tambien para el Lobo solar. El alcance +2 y la velocidad: no se simulan.
        carta("H1615", "Protección de la naturaleza", "self",
              "Una runa de la lanza invoca la fuerza de la naturaleza: hasta el final del combate atacas con "
              "ventaja, tienes +1 a la CA y tus golpes con arma hacen +1.",
              [("Protección de la naturaleza: ventaja en ataque todo el combate.", estado(180, "ventaja:ataque")),
               ("Protección de la naturaleza: +1 a la CA todo el combate.", estado(180, "bono_ca:1")),
               ("Fuerza vital recuperada: +1 a tus golpes con arma todo el combate.",
                estado(180, "bono_dano_arma:1"))]),
        # Red luminica abre Lobo torbellino (H1637, una orden que el lobo cumple con la suya, H1655) y este Juicio
        # del sol (H1638): una carta que suma las cuatro (§2). La red sagrada (no se mueven): sin posiciones; la
        # Luz danina del torbellino no llega. El poder del sol es tuyo (luz y +10 % de ataque con el lobo en pie).
        carta("H1616+H1637+H1655+H1638", "Red lumínica", "all_enemies",
              "Una red de luz atrapa a los enemigos, el Lobo solar gira entre ellos y cae del cielo una lanza "
              "sagrada: 4d6 de daño de luz a cada enemigo; con el punto débil al descubierto, se defienden con "
              "desventaja y tus golpes les hacen +1 durante 2 turnos, y el poder del sol da +1 a tus golpes con "
              "arma durante 3 turnos.",
              [("4d6 de daño de luz a cada enemigo.", dano(600, 500, 0, 0, 800, 600, 1300, 1000)),
               ("Punto débil al descubierto: se defienden con desventaja 2 turnos.",
                estado(10, "desventaja:defensa")),
               ("Punto débil al descubierto: tus golpes les hacen +1 durante 2 turnos.",
                estado(10, "bono_dano_contra:1")),
               ("Poder del sol: +1 a tus golpes con arma durante 3 turnos.", propio(20, "bono_dano_arma:1"))],
              elemento="Luz"),
    ],
    # El Cazasoles es su lobo: empieza con la carta que lo trae (y se cura), en vez de Relampago, que
    # repite lo de la Orden de ataque fugaz (§3)
    "arranque": ["Orden de ataque fugaz", "Lanza solar", "Curación natural"],
    "arma_inicial": "jabalinas_de_milicia",
    "set": [
        pieza("Jabalinas del Cazasoles", "MainHand", "ligera", {"DES": 1, "CAR": 1},
              "Las jabalinas de la SP del Cazasoles: astas ligeras con la punta de oro del sol."),
        pieza("Peto del Cazasoles", "Torso", "media", {"DES": 1},
              "Peto de cuero con el sol de oro en el pecho."),
        pieza("Tocado del Cazasoles", "Head", "ligera", {"DES": 1, "CAR": 1},
              "Tocado con la cabeza de lobo y el sol en la frente."),
        pieza("Calzas del Cazasoles", "Legs", "ligera", {"DES": 1, "CAR": 1},
              "Calzas de cuero del Cazasoles."),
        pieza("Botas del Cazasoles", "Feet", "ligera", {"DES": 1, "CAR": 1},
              "Botas de cuero del Cazasoles."),
    ],
}

GRANADERO = {
    "id": "granadero", "name": "Granadero", "sp": 10, "elemento": "Fuego", "nostale": "Granadero (Blaster)",
    "description": ("Especialista del lanzagranadas de fuego: granadas alquímicas de choque, de napalm y HEAT, un "
                    "escudo de dragonio y el calor del arma, que aviva su potencia de fuego."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "balanced",
    "pasiva": ("Disparar granada",
               "Las granadas alquímicas del Granadero: tus golpes con arma hacen +1 de daño (fuego).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("los puntos de calor y sus niveles (Caliente, Al rojo vivo, Sobrecalentamiento): los "
                           "dan los disparos y los gastan Recarga, Potencia de fuego aumentada, Transformación "
                           "energética y Aliento de dragón")]),
    # Sin niveles de profesion en la tarjeta: Creador = las tres ultimas (§3).
    "cartas": [
        carta("H1733", "Granada de choque", "zone_radius",
              "Una granada alquímica que estalla en una onda expansiva: 2d6 de daño de fuego y rigidez 1 turno (no "
              "ataca).",
              # empujar 4 casillas (solo en PvP): fuera
              [("2d6 de daño de fuego.", dano(500, 400)),
               ("Rigidez: 1 turno sin atacar.", estado(2, "incapacitado"))], elemento="Fuego"),
        carta("H1734", "Disparo vertiginoso", "zone_radius",
              "Disparas una granada en un ángulo abrupto: 2d6 de daño de fuego.",
              # con Al rojo vivo arrastra a los cercanos a un mismo punto: calor y posiciones, no se simula
              [("2d6 de daño de fuego.", dano(500, 400))], elemento="Fuego"),
        carta("H1735", "Granada de napalm", "zone_radius",
              "Una granada de napalm alquímico quema todo el radio: 3d6 de daño de fuego, y arde: 1 de daño por "
              "turno durante 3 turnos.",
              # el ataque que sube con el calor (del 3 % al 15 %): fuera; la resistencia al fuego -25: no se simula
              [("3d6 de daño de fuego.", dano(700, 600)),
               ("Quemadura fatal: 1 de daño por turno durante 3 turnos.", estado(30, "dano_por_turno:1"))],
              elemento="Fuego"),
        # «Disponible con Caliente o mas» y los 40 de calor que gasta: sin calor, se juega siempre.
        carta("H1736", "Recarga", "self",
              "Recargas granadas alquímicas más potentes: tus golpes con arma hacen +1d6 durante 3 turnos.",
              [("Recarga: +1d6 a tus golpes con arma durante 3 turnos.", estado(30, "bono_dano_arma:1d6"))]),
        # Una ventana de defensa de 2 s (el escudo espejado: no pierdes vida) y el escudo de 20 s, que se gana al
        # saltar y dura 2 rondas, como lo de todas las reacciones (§4). Las resistencias elementales: no se simulan.
        carta("H1737", "Escudo energético de Dragonio", "self",
              "El escudo de energía de dragonio contrarresta los ataques de ese turno y te protege: +1 a tu CA "
              "durante 2 turnos.",
              [CONTRARRESTA, ("Escudo energético: +1 a tu CA 2 turnos.", "propio:2:bono_ca:1"),
               _pendiente("con Sobrecalentamiento, devolver al atacante el 20 % del daño")], accion="reaccion"),
        # Sin dano en la ficha, y lo que hace (sujetar a los enemigos donde cae) son posiciones: la carta se queda
        # vacia y sale bloqueada (§10).
        carta("H1738", "Granada gravitatoria", "single_enemy",
              "Disparas una granada alquímica que aumenta la gravedad donde cae y sujeta allí a los enemigos.",
              [("[pendiente] sujetar a los enemigos donde cae la granada (sin posiciones).",
                "pendiente:sujetar a los enemigos donde cae la granada (sin posiciones)")]),
        # «Disponible con Caliente o mas» y el calor que gasta: sin calor, se juega siempre. El alcance que crece
        # con el calor: sin posiciones; el critico (del 2 % al 10 %): ventaja en ataque.
        carta("H1739", "Potencia de fuego aumentada", "self",
              "El calor del arma aumenta tu potencia de fuego: atacas con ventaja durante 5 turnos.",
              [("Ventaja en ataque durante 5 turnos.", estado(70, "ventaja:ataque"))]),
    ],
    "creador": [
        carta("H1740", "Disparo HEAT", "single_enemy",
              "Una granada alquímica HEAT de penetración extraordinaria: 3d6 de daño de fuego.",
              # el ataque que sube con el calor (del 4 % al 20 %): fuera
              [("3d6 de daño de fuego.", dano(1000, 1000)), _pendiente("que ignore el 30 % de su defensa")],
              elemento="Fuego"),
        # «Disponible con Al rojo vivo» y el calor que gasta: sin calor, se juega siempre. El elemento del hada y
        # la velocidad: no se simulan; el ataque (del 2 % al 10 % segun el calor): +1.
        carta("H1741", "Transformación energética", "self",
              "Transformas el calor del arma en fuerza: tus golpes con arma hacen +1 durante 5 turnos.",
              [("+1 a tus golpes con arma durante 5 turnos.", estado(60, "bono_dano_arma:1"))]),
        carta("H1742", "Aliento de dragón", "zone_radius",
              "Un haz de llamas con el calor del arma y la magia de dragón atraviesa la fila: 4d6 de daño de fuego, "
              "y el aliento de dragón te da +1d6 a tus golpes con arma durante 2 turnos.",
              # la resistencia del rival a los elementos -10: no se simula
              [("4d6 de daño de fuego.", dano(1300, 1500)),
               ("Aliento de dragón mejorado: +1d6 a tus golpes con arma durante 2 turnos.",
                propio(10, "bono_dano_arma:1d6")),
               _pendiente("que gaste todo el calor y sume al daño la mitad del que gasta")], elemento="Fuego"),
    ],
    # El catalogo no tiene lanzagranadas de tier 1: uno normal, como los de Viaje.
    "arma_inicial": "lanzagranadas_de_viaje",
    "armas_nuevas": [
        arma_comun("Lanzagranadas de Viaje", "media", "Un lanzagranadas corto de granadas alquímicas, a dos manos.",
                   restricciones=["Precisión: DES", DOS_MANOS]),
    ],
    "set": [
        pieza("Lanzagranadas del Granadero", "MainHand", "media", {"DES": 1, "CON": 1},
              "El lanzagranadas de la SP del Granadero: granadas alquímicas de sal de fuego, a dos manos.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Peto del Granadero", "Torso", "media", {"CON": 1},
              "Peto blindado con la correa de granadas alquímicas al pecho."),
        pieza("Casco del Granadero", "Head", "media", {"DES": 1, "CON": 1},
              "Casco de acero con la visera subida."),
        pieza("Pantalones del Granadero", "Legs", "ligera", {"DES": 1, "CON": 1},
              "Pantalones de campaña del Granadero."),
        pieza("Botas del Granadero", "Feet", "media", {"DES": 1, "CON": 1},
              "Botas altas reforzadas del Granadero."),
    ],
}

CAZANIEBLAS = {
    "id": "cazanieblas", "name": "Cazanieblas", "sp": 11, "elemento": "Tinieblas",
    "nostale": "Cazanieblas (Fog Hunter)",
    "description": ("Especialista del rifle de gas de tinieblas: dispara de lejos, envenena con gas, ciega a los "
                    "enemigos y se oculta en la niebla."),
    "primaryStat": "DES", "secondaryStat": "INT", "role": "agile",
    "pasiva": ("Disparo sencillo",
               "El rifle de gas del Cazanieblas: tus golpes con arma hacen +1 de daño (tinieblas).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("la barra de marcas (la llenan los disparos y las marcas las gastan sus técnicas de "
                           "gas)")]),
    "cartas": [
        carta("H399", "Andanada", "zone_radius",
              "Disparas sin parar contra el enemigo: 2d6 de daño de tinieblas.",
              [("2d6 de daño de tinieblas.", dano(500, 550))], elemento="Tinieblas"),
        carta("H460", "Vial de gas nervioso", "zone_radius",
              "Lanzas un vial de gas nervioso: se defiende con desventaja y tus golpes le hacen +1 durante 2 "
              "turnos.",
              # Gas nervioso: esquiva -40 % y +10 % al dano recibido; la velocidad, no. El gas mejorado de gastar
              # 1 marca: fuera
              [("Gas nervioso: se defiende con desventaja 2 turnos.", estado(7, "desventaja:defensa")),
               ("Gas nervioso: tus golpes le hacen +1 durante 2 turnos.", estado(7, "bono_dano_contra:1"))], elemento="Veneno"),
        carta("H461", "Fuego de retirada", "zone_radius",
              "Disparas mientras te retiras para mantenerlo a distancia: 3d6 de daño de tinieblas y aturdimiento 1 "
              "turno (no ataca y se defiende con desventaja).",
              # retroceder 8 casillas: sin posiciones
              [("3d6 de daño de tinieblas.", dano(800, 650))] + _aturdido(5), elemento="Tinieblas"),
        # «Solo si tienes 1 marca» (y el +20 % de gastarla): sin marcas, se juega siempre.
        carta("H1852", "Objetivo mental", "zone_radius",
              "Apuntas con calma y disparas a la distancia justa: 3d6 de daño de tinieblas.",
              [("3d6 de daño de tinieblas.", dano(1200, 500))], elemento="Tinieblas"),
        # El modo que refuerza las tecnicas con las marcas, como la Inyeccion mineral del Romperrocas.
        carta("H1853", "Condensación de gas", "self",
              "Una explosión de gas carga el rifle: tus golpes con arma hacen +1 durante 2 turnos.",
              [("+1 a tus golpes con arma durante 2 turnos.", estado(10, "bono_dano_arma:1")),
               _pendiente("las marcas que refuerza")]),
        # Toxicosis gaseosa (veneno): sangran; la vida por el mana gastado y el +10 % de golpe fatal, no. Gas
        # nebulizado (invisible): te defiendes con ventaja; el dano de mas en un ataque escondido no se simula.
        carta("H1854", "Liberación de gas", "all_enemies",
              "Liberas gas a tu alrededor: la toxicosis hace sangrar a cada enemigo 3 turnos y tú, oculto en el "
              "gas, te defiendes con ventaja 2 turnos.",
              [("Toxicosis gaseosa: sangran 3 turnos.", condicion("sangrado", 20)),
               ("Gas nebulizado: te defiendes con ventaja 2 turnos.", propio(10, "ventaja:defensa"))], elemento="Veneno"),
        carta("H1855", "Explosión de gas", "zone_radius",
              "Liberas gas hacia delante: 3d6 de daño de veneno, y la toxicosis le hace sangrar 3 turnos.",
              # el shock de gastar 1 marca: sin marcas, fuera
              [("3d6 de daño de veneno.", dano(800, 1500)),
               ("Toxicosis gaseosa: sangra 3 turnos.", condicion("sangrado", 20))], elemento="Veneno"),
    ],
    # Creador: las de profesion 12, 16 y 20.
    "creador": [
        # Gas opaco: acierto y esquiva -50 %. Refuerzo del sistema nervioso (tuyo): ataques +5 % y critico +5 %;
        # lo de gastar marcas (la mejora del sistema nervioso), fuera.
        carta("H1856", "Bloqueo de visión", "all_enemies",
              "Liberas un gas que ciega a los enemigos: atacan y se defienden con desventaja durante 2 turnos; el "
              "gas que absorbes te refuerza hasta el final del combate: atacas con ventaja y tus golpes con arma "
              "hacen +1.",
              [("Gas opaco: atacan con desventaja 2 turnos.", estado(10, "desventaja:ataque")),
               ("Gas opaco: se defienden con desventaja 2 turnos.", estado(10, "desventaja:defensa")),
               ("Refuerzo del sistema nervioso: ventaja en ataque todo el combate.", propio(120, "ventaja:ataque")),
               ("Refuerzo del sistema nervioso: +1 a tus golpes con arma todo el combate.",
                propio(120, "bono_dano_arma:1"))]),
        # Danos organicos: mas dano cuantos mas estados en contra lleva: +1 a tus golpes contra el.
        carta("H1857", "Tiro perforante", "zone_radius",
              "Un proyectil perforador atraviesa a los que tienes delante: 4d6 de daño de tinieblas, y los daños "
              "orgánicos hacen que tus golpes le hagan +1 durante 3 turnos.",
              [("4d6 de daño de tinieblas.", dano(1200, 1800)),
               ("Daños orgánicos: tus golpes le hacen +1 durante 3 turnos.", estado(20, "bono_dano_contra:1"))],
              elemento="Tinieblas"),
        # «Solo con 2 marcas» y el +50 % de gastarlas: sin marcas, se juega siempre. Gas toxico: pierde vida cada
        # 2 s (sangra) y +15 % al dano recibido; el mana, no.
        carta("H1858", "Disparo a todo gas", "all_enemies",
              "Cargas el rifle con todo el gas y disparas: 4d6 de daño de veneno a cada enemigo; el gas tóxico "
              "los hace sangrar 2 turnos y tus golpes les hacen +1 durante 2 turnos.",
              [("4d6 de daño de veneno a cada enemigo.", dano(1700, 1900)),
               ("Gas tóxico: sangran 2 turnos.", condicion("sangrado", 10)),
               ("Gas tóxico: tus golpes les hacen +1 durante 2 turnos.", estado(10, "bono_dano_contra:1"))],
              elemento="Veneno"),
    ],
    # El catalogo no tiene rifle de tier 1: uno normal, como los de Viaje.
    "arma_inicial": "rifle_de_viaje",
    "armas_nuevas": [
        arma_comun("Rifle de Viaje", "media", "Un rifle largo de balas rúnicas, a dos manos.",
                   restricciones=["Precisión: DES", DOS_MANOS]),
    ],
    "set": [
        pieza("Rifle de Gas del Cazanieblas", "MainHand", "media", {"DES": 1, "INT": 1},
              "El rifle de francotirador de la SP del Cazanieblas: dispara con gas alquímico, a dos manos.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Gabardina del Cazanieblas", "Torso", "ligera", {"DES": 1},
              "Gabardina larga con los botes de gas y la máscara colgando al cuello."),
        pieza("Capucha del Cazanieblas", "Head", "ligera", {"DES": 1, "INT": 1},
              "Capucha honda con el forro oliva."),
        pieza("Pantalones del Cazanieblas", "Legs", "ligera", {"DES": 1, "INT": 1},
              "Pantalones oscuros del Cazanieblas."),
        pieza("Botas del Cazanieblas", "Feet", "ligera", {"DES": 1, "INT": 1},
              "Botas oscuras del Cazanieblas."),
    ],
}

ALMIRANTE_YI = {
    "id": "almirante_yi", "name": "Almirante Yi", "sp": 12, "elemento": "Agua",
    "nostale": "Almirante Yi (Admiral Yi)",
    "description": ("Especialista del sable de agua: tajos rápidos, atrae a los enemigos con las mareas y los "
                    "arrasa con el Dragón de los Mares."),
    "primaryStat": "DES", "secondaryStat": "CON", "role": "balanced",
    "pasiva": ("Cuchillada de espada",
               "Los cortes rápidos del Almirante Yi: tus golpes con arma hacen +1 de daño (agua).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("la barra de marcas que llenan sus golpes y la sincronización dimensional (con el héroe "
                           "que invoca)")]),
    # Sin niveles de profesion: Creador = las tres ultimas de la tarjeta (§3), sin contar Golpe al torso ni
    # Partir por la mitad, que van con Golpe en el tobillo.
    "cartas": [
        # Golpe en el tobillo abre Golpe al torso (H1973) y este Partir por la mitad (H1974): una carta que suma
        # los tres (§2). La Broma letal del torso (40 %) no llega; la de partir por la mitad (50 %), si.
        carta("H1963+H1973+H1974", "Golpe en el tobillo", "zone_radius",
              "Le atacas las piernas, luego el torso y lo partes por la mitad: 2d6 de daño de agua, y la broma "
              "letal hace que tus golpes le hagan +1 durante 2 turnos.",
              [("2d6 de daño de agua.", dano(200, 100, 250, 150, 350, 200)),
               ("Broma letal: tus golpes le hacen +1 durante 2 turnos.", estado(10, "bono_dano_contra:1"))],
              elemento="Agua"),
        carta("H1964", "Golpe de la corriente marina", "single_enemy",
              "Te lanzas sobre el enemigo rápido como la corriente marina: 2d6 de daño de agua, y el incendio de "
              "agua marina le quita 1 de vida por turno durante 2 turnos.",
              # acercarse: sin posiciones; lo demas del incendio (velocidad, -20 % con Broma letal): no se simula
              [("2d6 de daño de agua.", dano(500, 400)),
               ("Incendio de agua marina: 1 de daño por turno durante 2 turnos.", estado(10, "dano_por_turno:1"))],
              elemento="Agua"),
        carta("H1965", "Doble marea", "all_enemies",
              "El poder de las mareas arrastra a los enemigos hacia ti: 3d6 de daño de agua a cada enemigo, que te "
              "ataca a ti 1 turno, y quedan grogui: atacan y se defienden con desventaja 1 turno.",
              # Grogui: acierto al 70 % y sin defensa; la velocidad y la magia -15 %, no
              [("3d6 de daño de agua a cada enemigo.", dano(600, 850)), _atrae(),
               ("Grogui: atacan con desventaja 1 turno.", estado(5, "desventaja:ataque")),
               ("Grogui: se defienden con desventaja 1 turno.", estado(5, "desventaja:defensa"))], elemento="Agua"),
        carta("H1966", "Tormenta del Dragón de los Mares", "all_enemies",
              "Invocas una tormenta sobre una gran superficie: 3d6 de daño de agua a cada enemigo y shock 1 turno "
              "(no atacan y se defienden con desventaja).",
              # El despertar del dragon (el elemento del hada y la velocidad): no se simula
              [("3d6 de daño de agua a cada enemigo.", dano(900, 900))] + _aturdido(5, "Shock"), elemento="Agua"),
        carta("H1967", "Descenso del Dragón de los Mares", "all_enemies",
              "El Dragón de los Mares cae sobre el enemigo y los cercanos: 3d6 de daño de agua a cada enemigo, que "
              "ataca con desventaja 2 turnos, y la corriente marina te da +1 a la CA durante 2 turnos.",
              # Golpe dragonico: sus ataques -15 % y nivel de ataque -2. Corriente marina (tuya): defensa +10 %;
              # la Broma letal a quien te ataca y el aturdimiento mas corto, no se simulan
              [("3d6 de daño de agua a cada enemigo.", dano(1000, 1000)),
               ("Golpe dragónico: atacan con desventaja 2 turnos.", estado(10, "desventaja:ataque")),
               ("Corriente marina: +1 a tu CA durante 2 turnos.", propio(7, "bono_ca:1"))], elemento="Agua"),
        carta("H1968", "Zarpazo de oleaje", "all_enemies",
              "Blandes la espada y una ola marina golpea a los cercanos: 3d6 de daño de agua a cada enemigo.",
              # Impacto del oleaje: +20 % al dano de agua solo un 10 % de las veces, y -30 % a su dano con Broma
              # letal: nada que entre
              [("3d6 de daño de agua a cada enemigo.", dano(700, 600))], elemento="Agua"),
        carta("H1969", "Espada del oleaje", "self",
              "Bloqueas con la espada: contrarrestas los ataques de ese turno.",
              # el indicador que sube al recibir dano: la barra no se simula
              [CONTRARRESTA, _pendiente("Perder la cabeza, el efecto que recibe quien te ataca")],
              accion="reaccion"),
    ],
    "creador": [
        # Critico +5 %: ventaja; el +3 % de ataque con la sincronizacion: +1 al arma, como la Maldicion creciente
        # de Aquiles.
        carta("H1970", "Bendición del Dragón de los Mares", "self",
              "La bendición del Dragón de los Mares te fortalece hasta el final del combate: atacas con ventaja y "
              "tus golpes con arma hacen +1.",
              [("Ventaja en ataque todo el combate.", estado(300, "ventaja:ataque")),
               ("+1 a tus golpes con arma todo el combate.", estado(300, "bono_dano_arma:1"))]),
        # El +3 % de defensa con la sincronizacion: +1 a la CA; el heroe invocado (mas a menudo y mas fuerte): no
        # se simula.
        carta("H1971", "Flota de guerra orgullosa", "self",
              "Recuerdas la misión de la orgullosa flota de guerra de Joseon: +1 a la CA hasta el final del "
              "combate.",
              [("Orgullo de Joseon: +1 a la CA todo el combate.", estado(300, "bono_ca:1"))]),
        carta("H1972", "Oleada salvaje", "all_enemies",
              "Convocas una ola salvaje sobre los enemigos cercanos: 4d6 de daño de agua a cada enemigo, y el "
              "encadenamiento marino hace que tus golpes les hagan +1 durante 2 turnos.",
              # +5 % por nivel de sincronizacion: como la Lluvia de lanzas de Aquiles, fuera
              [("4d6 de daño de agua a cada enemigo.", dano(1800, 2000)),
               ("Encadenamiento marino: tus golpes les hacen +1 durante 2 turnos.",
                estado(10, "bono_dano_contra:1"))],
              elemento="Agua"),
    ],
    "arma_inicial": "sable_ligero_de_milicia",
    "set": [
        pieza("Sable del Almirante Yi", "MainHand", "media", {"DES": 1, "CON": 1},
              "El sable de la SP del Almirante Yi: hoja curva templada en agua."),
        pieza("Armadura del Almirante Yi", "Torso", "media", {"CON": 1},
              "Armadura de láminas azules tachonada de oro, con el forro rojo."),
        pieza("Sombrero del Almirante Yi", "Head", "ligera", {"DES": 1, "CON": 1},
              "Sombrero negro de ala ancha con la borla roja."),
        pieza("Grebas del Almirante Yi", "Legs", "media", {"DES": 1, "CON": 1},
              "Grebas de láminas del Almirante Yi."),
        pieza("Botas del Almirante Yi", "Feet", "ligera", {"DES": 1, "CON": 1},
              "Botas negras del Almirante Yi."),
    ],
}

ARQUERO = [CAZADOR, ASESINO, DESTRUCTOR, GUARDABOSQUES, SERVIDOR_DEL_CANON, ESPIA, CAZADEMONIOS,
           ANGEL_VENGADOR, CAZASOLES, GRANADERO, CAZANIEBLAS, ALMIRANTE_YI]
