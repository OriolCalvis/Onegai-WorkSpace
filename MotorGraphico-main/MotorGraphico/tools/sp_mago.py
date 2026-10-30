"""SP-02 — Las doce SP del Mago de NosTale, como clases de MotorGraphico (los datos).

Cada carta dice de que habilidad sale (su H en la enciclopedia de SP de Oriol) y sus cifras
pasan por las reglas de sp_reglas.py (planificacion/CANON_SP_CLASES.md): los dados por la
bonificacion de la ficha (ataque mágico o a distancia + elemental, sumando los pasos de una
continuacion), las rondas por los segundos, los estados con un 50 % o más. Cuando una carta se
aparta de la regla, lo dice un comentario. Lo escribe en los catalogos tools/importar_clases_sp.py.

Con que se tira (§7): lo dice la bonificacion de la ficha. Nueve SP tienen «ataque mágico» en
todas sus técnicas y tiran INT («tira»); el Mago de las tinieblas y Merlin disparan el arma
magica secundaria («a distancia»): DES, lo de siempre. La Tormenta de fuego mezcla: el básico y
los disparos del lanzallamas, DES; Circulo de fuego y Remolino llameante, INT (tira= en la carta).

Las armas (Oriol, 07/10: las armas de fuego existen, pero «en comptes de pòlvora alquímia i màgia
son la base tecnològica»): la pistola magica dispara balas de maná y el lanzallamas quema aceite
alquimico, nunca polvora.

Criterios comunes a las doce (ademas de los del Espadachín y el Arquero):
- Los escudos de maná (uno por SP: Transfusion Maná, Escudo maná, Guardián Fantasma, Escudo de
  magma, Burbuja de agua, Armadura de la oscuridad...): el daño recibido -X % es +1 a tu CA
  (+2 desde el 25 %) con sus segundos; el daño que pasa a los MP y la velocidad, fuera.
- Las bendiciones de grupo con «bonificacion elemental +(N xK)», N el nivel (Bendición de fuego,
  Bendición del agua, Maldición oscura, Arma santa): +200 o más desde el nivel 34 (x6) o 40 (x5),
  +1d6, como el +(N x4) del Espiritu de lobo del Guardabosques. Las resistencias, fuera.
- Los criticos propios de un golpe y el daño crítico no entran (como en el Arquero).
- Terror leve («ataque no posible» por terror): asustado (§5: miedo).
- Un efecto que salta «con una probabilidad del 50 %» (la energía oscura del Vidente) llega al
  50 % y entra; los del 10-20 %, fuera.
- Pierden vida cada N s: por una maldición o una fuerza (fuego, gravedad), dano_por_turno; por
  una herida o veneno, sangrado.
- Lo que ataca una sola vez (el tiburon de Hora de comer, la ballena azul, la espada de magma,
  el enjambre de langostas) es una carta de daño, no una invocación. Las invocaciones sin cifras
  (el Totem sagrado, las Aranas espirituales, los Clones oscuros) llevan las de una criatura de
  tier 1 del bestiario, como el Lobo solar del Cazasoles, y duran lo mínimo, 1 turno.
- Barras y marcas (la energía sagrada, los puntos de gravedad, las marcas de la Tormenta de fuego
  y de Merlin, la sincronizacion dimensional): lo que las pide («solo si tienes 1 marca») se juega
  siempre; lo que da gastarlas, fuera, y la pasiva lo deja dicho (pendiente).
"""
from sp_reglas import (CONTRARRESTA, DOS_MANOS, arma_comun, carta, condicion, dano, estado, pieza, propio,
                       siempre)
from sp_reglas import atrae as _atrae
from sp_reglas import aturdido as _aturdido
from sp_reglas import pendiente as _pendiente

# La pistola normal de tier 1 (el catalogo no tenia ninguna): la traen el Mago de las tinieblas y Merlin.
PISTOLA_MAGICA = arma_comun("Pistola Mágica de Viaje", "ligera", "Una pistola sencilla que dispara balas de maná.",
                            restricciones=["Precisión: DES"])


MAGO_DE_FUEGO = {
    "id": "mago_de_fuego", "name": "Mago de fuego", "sp": 1, "elemento": "Fuego",
    "nostale": "Mago de fuego (Red Mage)",
    "description": ("Especialista del bastón de fuego: lanza llamaradas y lanzas de fuego, quema con su aliento y "
                    "arrasa a los enemigos con una lluvia de meteoritos."),
    "primaryStat": "INT", "secondaryStat": "CON", "role": "caster",
    "tira": "INT",
    "pasiva": ("Pernos de fuego",
               "Los pernos del Mago de fuego: tus golpes con arma hacen +1 de daño (fuego).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H856", "Llamarada de fuego", "zone_radius",
              "Provocas una pequeña explosión sobre el enemigo: 2d6 de daño de fuego.",
              [("2d6 de daño de fuego.", dano(250, 650))], elemento="Fuego"),
        carta("H857", "Aliento de fuego", "single_enemy",
              "Tu aliento de fuego quema al enemigo: 2d6 de daño de fuego, y arde: 1 de daño por turno durante 3 "
              "turnos.",
              # Quemadura (80 %): la vida que pierde cada 2 s; la resistencia al fuego -15, fuera
              [("2d6 de daño de fuego.", dano(450, 700)),
               ("Quemadura: 1 de daño por turno durante 3 turnos.", estado(20, "dano_por_turno:1"))],
              elemento="Fuego"),
        # Trasmision de Mana (100 %, 180 s): el daño recibido -20 % (+1 a la CA; el 30 % de la descripcion es el
        # daño que pasa a los MP); los MP y la velocidad +2, fuera
        carta("H858", "Transfusión Maná", "self",
              "Tu maná absorbe parte de los golpes: +1 a la CA hasta el final del combate.",
              [("Trasmisión de Maná: +1 a tu CA todo el combate.", propio(180, "bono_ca:1"))], elemento="Arcano"),
        carta("H859", "Tormenta de fuego", "zone_radius",
              "Una potente tormenta de fuego quema al enemigo: 2d6 de daño de fuego.",
              [("2d6 de daño de fuego.", dano(300, 800))], elemento="Fuego"),
        carta("H860", "Lanza de fuego", "single_enemy",
              "Invocas una lanza de fuego que atraviesa al enemigo: 3d6 de daño de fuego y -1 a su CA durante 3 "
              "turnos.",
              [("3d6 de daño de fuego.", dano(700, 1200)),
               ("Debilita su defensa: -1 a la CA durante 3 turnos.", estado(30, "penal_ca:1"))], elemento="Fuego"),
        # Bendicion del Fuego (100 %, 300 s): fuego +(N x6), +1d6 (ver arriba); la resistencia al fuego +25, fuera
        carta("H861", "Bendición de fuego", "all_allies",
              "Bendices al grupo con la energía del fuego: tú y tus aliados hacéis +1d6 con los golpes de arma hasta "
              "el final del combate.",
              [("Bendición del Fuego: +1d6 a los golpes con arma todo el combate.",
                estado(300, "bono_dano_arma:1d6"))]),
        # el muro hace su daño al jugar la carta (sin posiciones)
        carta("H862", "Muro de fuego", "zone_radius",
              "Levantas un muro de fuego sobre el enemigo: 3d6 de daño de fuego.",
              [("3d6 de daño de fuego.", dano(400, 1500))], elemento="Fuego"),
    ],
    "creador": [
        carta("H863", "Lluvia de meteoritos", "all_enemies",
              "Una lluvia de meteoritos cae sobre los enemigos: 3d6 de daño de fuego a cada uno.",
              # el 40 % de aturdimiento no llega al 50 %: fuera
              [("3d6 de daño de fuego a cada enemigo.", dano(800, 1000))], elemento="Fuego"),
        carta("H864", "Inferno", "all_enemies",
              "Alzas una columna mágica de fuego a tu alrededor: 3d6 de daño de fuego a cada enemigo, y arden: 1 de "
              "daño por turno durante 3 turnos.",
              # Quemadura fatal (70 %): la vida que pierden cada 2 s; la resistencia al fuego -25, fuera
              [("3d6 de daño de fuego a cada enemigo.", dano(300, 2000)),
               ("Quemadura fatal: 1 de daño por turno durante 3 turnos.", estado(30, "dano_por_turno:1"))],
              elemento="Fuego"),
        carta("H865", "Ataque de los meteoritos", "all_enemies",
              "Un gran meteorito arrasa al enemigo y todo lo que le rodea: 4d6 de daño de fuego a cada enemigo y "
              "aturdimiento 1 turno (no atacan y se defienden con desventaja).",
              [("4d6 de daño de fuego a cada enemigo.", dano(1200, 1500))] + _aturdido(6), elemento="Fuego"),
    ],
    "arma_inicial": "vara_de_aprendiz_de_milicia",
    "set": [
        pieza("Bastón del Mago de fuego", "MainHand", "ligera", {"INT": 1, "CON": 1},
              "El bastón de la SP del Mago de fuego, con la llama en la punta.",
              restricciones=["Precisión: INT"]),
        pieza("Túnica del Mago de fuego", "Torso", "ligera", {"INT": 1},
              "Túnica roja hasta los pies, con el cuello alto, los puños de oro y la faja de brasa."),
        pieza("Corona del Mago de fuego", "Head", "ligera", {"INT": 1, "CON": 1},
              "La corona de llamas del Mago de fuego."),
        pieza("Calzas del Mago de fuego", "Legs", "ligera", {"INT": 1, "CON": 1},
              "Calzas rojas del Mago de fuego."),
        pieza("Zapatos del Mago de fuego", "Feet", "ligera", {"INT": 1, "CON": 1},
              "Zapatos de cuero del Mago de fuego."),
    ],
}

MAGO_SAGRADO = {
    "id": "mago_sagrado", "name": "Mago sagrado", "sp": 2, "elemento": "Luz", "nostale": "Mago sagrado (Holy Mage)",
    "description": ("Especialista del cetro de luz: cura y protege al grupo, bendice sus armas, adormece a los "
                    "enemigos con su canto y los castiga con el martillo santo."),
    "primaryStat": "INT", "secondaryStat": "CAR", "role": "support",
    "tira": "INT",
    # La barra de energia sagrada no se simula (§10): la llenan Ataque santo (1 %), Cura (10 %, solo a NosMates),
    # Luz santa (5 %) y Cura de grupo (10 %), y el Totem sagrado la pide llena.
    "pasiva": ("Ataque santo",
               "La bola de luz santa del Mago sagrado: tus golpes con arma hacen +1 de daño (luz).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("la barra de energía sagrada (la llenan Ataque santo, Cura, Luz santa y Cura de grupo, "
                           "y el Tótem sagrado la pide llena)")]),
    # El Totem sagrado (H872) es una invocacion de la que NosTale no publica cifras: las de una criatura de apoyo
    # de tier 1 del bestiario (Capellan Castrense: CON 2, DES 1, INT 2, CAR 4; el cargador no lee FUE) y los 5 PV
    # de las invocaciones de tier 1, como el Lobo solar del Cazasoles. Sin segundos en la ficha dura lo minimo, 1
    # turno (como los Clones oscuros del Vidente). Lo que hace (apoyar al lanzador y a sus aliados cercanos) no
    # esta en la ficha: la carta sale bloqueada.
    "invocaciones": [
        {"id": "totem_sagrado", "name": "Tótem sagrado", "description": "CON 2, DES 1, INT 2, CAR 4, 5 PV.",
         "health": 5, "duration": "1 turno", "summonedBy": "mago_sagrado_totem_sagrado",
         "stats": {"CON": 2, "DES": 1, "INT": 2, "CAR": 4},
         # un totem no ataca: lo suyo es apoyar
         "attacks": [],
         "passive": {"name": "Apoyo sagrado",
                     "description": ("Apoya al lanzador y a sus aliados cercanos. [pendiente] La ficha de NosTale no "
                                     "dice cómo ni cuánto dura.")}},
    ],
    "cartas": [
        # Cura: (N x20) HP y el 5 % de la vida del lanzador, cura:1d6. Refuerzo sagrado (100 %, 40 s, al aliado):
        # todos los ataques +5 %, +1. El 10 % de cancelar el enfriamiento del Martillo santo, fuera. El Martillo
        # santo que la ficha da como «relacionada» es una tecnica propia de la tarjeta (profesion 20): carta aparte.
        carta("H867", "Cura", "single_ally",
              "Una luz santa cura a un aliado 1d6, y el refuerzo sagrado hace que sus golpes con arma hagan +1 "
              "durante 3 turnos.",
              [("Cura 1d6.", "cura:1d6"),
               ("Refuerzo sagrado: +1 a sus golpes con arma durante 3 turnos.", estado(40, "bono_dano_arma:1"))]),
        carta("H868", "Luz santa", "zone_radius",
              "Una luz santa brilla desde el cielo sobre el enemigo: 2d6 de daño de luz.",
              # 30 % de luz de la magia: fuera
              [("2d6 de daño de luz.", dano(200, 500))], elemento="Luz"),
        # Escudo de Mana (100 %, 180 s): el daño recibido -20 % (+1 a la CA) y el foco + en los ataques magicos
        # (ventaja); el 25 % del daño que pasa a los MP y la velocidad +2, fuera
        carta("H869", "Escudo maná", "self",
              "Un escudo de maná te protege hasta el final del combate: +1 a la CA, y atacas con ventaja.",
              [("Escudo de Maná: +1 a tu CA todo el combate.", propio(180, "bono_ca:1")),
               ("Escudo de Maná: atacas con ventaja todo el combate.", propio(180, "ventaja:ataque"))], elemento="Arcano"),
        # Canto del Cielo (100 %, 8 s: el sueño, ataque no posible) y Somnolencia (100 %, 15 s: defensa -2). Son de
        # enfermedad y negativo, no magicos: sin salvacion
        carta("H870", "Canto del cielo", "all_enemies",
              "Un canto del cielo quita a los enemigos las ganas de luchar: se quedan 2 turnos sin atacar y, "
              "adormecidos, con -1 a la CA.",
              [("Canto del cielo: 2 turnos sin atacar.", estado(8, "incapacitado")),
               ("Somnolencia: -1 a la CA durante 2 turnos.", estado(15, "penal_ca:1")),
               _pendiente("que el canto se rompa si reciben daño")]),
        # los MP que devuelve no se simulan; «hasta el nivel 4»: los estados del combate no tienen nivel
        carta("H871", "Efecto cura", "all_allies",
              "Una luz que purifica: tú y tus aliados os libráis de los estados en contra.",
              [("Os libráis de los estados en contra.", "limpia")]),
        # La carta que trae el Totem sagrado («invocaciones»). Lo que hace no se puede jugar: sale bloqueada (§10).
        # «Exige que el medidor de energia sagrada haya alcanzado su maximo»: la barra no se simula (la pasiva).
        carta("H872", "Tótem sagrado", "self",
              "Invocas un tótem sagrado que apoya 1 turno a tus aliados cercanos. [pendiente] La ficha no dice qué "
              "hace ni cuánto dura.",
              [("El Tótem sagrado se alza a tu lado 1 turno.", "invoca:1:totem_sagrado"),
               ("[pendiente] Lo que hace el tótem.",
                "pendiente:el apoyo del Tótem sagrado (la ficha no dice qué hace ni cuánto dura)")]),
        # (N x55) HP y el 10 % de la vida del lanzador: mas que N x50, cura:2d6
        carta("H873", "Cura de grupo", "all_allies",
              "Una luz santa cura al grupo: tú y tus aliados os curáis 2d6.",
              [("Os curáis 2d6.", "cura:2d6")]),
    ],
    "creador": [
        # Arma Bendita (100 %, 300 s): luz +(N x5), +1d6 (ver arriba), y el nivel de ataque +1, +1 (§5)
        carta("H874", "Arma santa", "all_allies",
              "Bendices con la luz las armas del grupo: hasta el final del combate, tú y tus aliados hacéis +1d6 "
              "con los golpes de arma, y +1 más.",
              [("Arma bendita: +1d6 a los golpes con arma todo el combate.", estado(300, "bono_dano_arma:1d6")),
               ("Arma bendita: +1 más a los golpes con arma todo el combate.", estado(300, "bono_dano_arma:1"))]),
        # Bendicion (100 %, 300 s): defensa +1. «Protege de aciertos criticos» es de la descripcion, no del estado;
        # la resistencia a las tinieblas +25, fuera
        carta("H875", "Bendición", "all_allies",
              "Una bendición protege al grupo: tú y tus aliados tenéis +1 a la CA hasta el final del combate.",
              [("Bendición: +1 a la CA todo el combate.", estado(300, "bono_ca:1"))]),
        # Terror leve (100 %, 5 s): «ataque no posible» por terror: asustado (§5: miedo, terror; como en el
        # Archimago); la resistencia a la luz -10, fuera. Eliminacion de las tinieblas (70 %: no puede usar su
        # fuerza, como en la Cruz santa del Cruzado) y Luz resplandeciente (50 %: resistencias), no se simulan
        carta("H876", "Martillo santo", "all_enemies",
              "Un martillo santo baja del cielo: 4d6 de daño de luz a cada enemigo, y el terror los deja asustados 1 "
              "turno.",
              [("4d6 de daño de luz a cada enemigo.", dano(700, 1800)),
               ("Terror leve: asustado 1 turno.", condicion("asustado", 5))], elemento="Luz"),
    ],
    "arma_inicial": "cetro_de_bronce_de_milicia",
    # support: la casulla, media (el maximo del rol); la mitra, las calzas y las sandalias, ligeras; el cetro, ligero
    "set": [
        pieza("Cetro del Mago sagrado", "MainHand", "ligera", {"INT": 1, "CAR": 1},
              "El cetro de la SP del Mago sagrado, blanco y oro.",
              restricciones=["Precisión: INT"]),
        pieza("Casulla del Mago sagrado", "Torso", "media", {"INT": 1},
              "Casulla blanca con la estola de oro y la cruz en el pecho."),
        pieza("Mitra del Mago sagrado", "Head", "ligera", {"INT": 1, "CAR": 1},
              "Mitra blanca y alta, con la cruz de oro."),
        pieza("Calzas del Mago sagrado", "Legs", "ligera", {"INT": 1, "CAR": 1},
              "Calzas blancas del Mago sagrado."),
        pieza("Sandalias del Mago sagrado", "Feet", "ligera", {"INT": 1, "CAR": 1},
              "Sandalias de cuero del Mago sagrado."),
    ],
}

MAGO_AZUL = {
    "id": "mago_azul", "name": "Mago azul", "sp": 3, "elemento": "Hielo", "nostale": "Mago azul (Blue Mage)",
    "description": ("Especialista de la vara de hielo: congela a los enemigos con lanzas y cadenas heladas, se "
                    "protege con un escudo de hielo y los barre con granizo y tormentas de nieve."),
    "primaryStat": "INT", "secondaryStat": "DES", "role": "caster",
    "tira": "INT",
    "pasiva": ("Bola de hielo",
               "Las bolas de hielo del Mago azul: tus golpes con arma hacen +1 de daño (hielo).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H934", "Fragmento de hielo", "zone_radius",
              "Lanzas trozos de hielo contra el enemigo: 2d6 de daño de hielo.",
              [("2d6 de daño de hielo.", dano(250, 600))], elemento="Hielo"),
        carta("H935", "Destello de hielo", "zone_radius",
              "Un poderoso hechizo de hielo estalla sobre el enemigo: 3d6 de daño de hielo.",
              # Escalofrio (40 %: velocidad): fuera
              [("3d6 de daño de hielo.", dano(500, 800))], elemento="Hielo"),
        carta("H936", "Lanza de hielo", "all_enemies",
              "Una lanza de hielo cae sobre los enemigos: 3d6 de daño de hielo a cada uno y -1 a su CA durante 3 "
              "turnos.",
              [("3d6 de daño de hielo a cada enemigo.", dano(900, 1400)),
               ("Debilita su defensa: -1 a la CA durante 3 turnos.", estado(30, "penal_ca:1"))], elemento="Hielo"),
        carta("H937", "Cadena helada", "zone_radius",
              "Atas al enemigo con una cadena helada: 3d6 de daño de hielo, y la helada le deja 2 turnos sin atacar.",
              [("3d6 de daño de hielo.", dano(800, 1600)),
               ("Helada: 2 turnos sin atacar.", estado(8, "incapacitado"))], elemento="Hielo"),
        # Dos estados al 100 %. Escudo helado de mana (180 s): el daño recibido -20 % (+1 a la CA) y los ataques
        # magicos +5 % (+1); el 20 % del daño que pasa a los MP, la velocidad y el elemento del hada, fuera.
        # Escudo Congelado, el efecto principal: anula el daño de todos los ataques (daño recibido -25 % o mas:
        # +2 a la CA). La ficha no da su duracion (ni dice que sea una ventana de 2 s): accion principal y 1 turno,
        # lo minimo (como la Guarida del Asesino). Congelar a quien te ataca (Congelacion leve, 4 s) pide poner un
        # estado al atacante, que solo hace una reaccion: pendiente
        carta("H938", "Escudo de hielo", "self",
              "Te cubres con un escudo de hielo: el escudo congelado te da +2 a la CA 1 turno, y el escudo helado de "
              "maná, hasta el final del combate, +1 a la CA y +1 a tus golpes con arma.",
              [("Escudo Congelado: +2 a tu CA 1 turno.", "propio:1:bono_ca:2"),
               ("Escudo helado de maná: +1 a tu CA todo el combate.", propio(180, "bono_ca:1")),
               ("Escudo helado de maná: +1 a tus golpes con arma todo el combate.", propio(180, "bono_dano_arma:1")),
               _pendiente("cuánto dura el Escudo Congelado (la ficha no lo da) y que congele 1 turno a quien te "
                          "ataque")], elemento="Hielo"),
        carta("H939", "Glaciar", "all_enemies",
              "Se alzan columnas de hielo a tu alrededor: 3d6 de daño de hielo a cada enemigo, que se queda sin escudo "
              "2 turnos (se defiende con desventaja).",
              [("3d6 de daño de hielo a cada enemigo.", dano(700, 1000)),
               ("Destruir escudo: se defienden con desventaja 2 turnos.", estado(10, "desventaja:defensa"))],
              elemento="Hielo"),
        # Bendicion del Agua (100 %, 300 s): agua +(N x6), +1d6 (ver arriba); la resistencia al agua +25, fuera
        carta("H940", "Bendición del agua", "all_allies",
              "Bendices al grupo con la energía del agua: tú y tus aliados hacéis +1d6 con los golpes de arma hasta "
              "el final del combate.",
              [("Bendición del Agua: +1d6 a los golpes con arma todo el combate.",
                estado(300, "bono_dano_arma:1d6"))]),
    ],
    "creador": [
        # Granizo fuerte abre Carambanos mortales (H2000, su continuacion, al 100 %): una carta que suma las dos
        # (§2). El aturdimiento del granizo (60 %) y el Destruir escudo de los carambanos (100 %)
        carta("H941+H2000", "Granizo fuerte", "all_enemies",
              "Cae una granizada y se forman carámbanos mortales alrededor de los enemigos: 4d6 de daño de hielo a "
              "cada uno, aturdimiento 1 turno (no atacan y se defienden con desventaja) y se quedan sin escudo 2 "
              "turnos (se defienden con desventaja).",
              [("4d6 de daño de hielo a cada enemigo.", dano(1000, 1200, 700, 1000))] + _aturdido(5)
              + [("Destruir escudo: se defienden con desventaja 2 turnos.", estado(10, "desventaja:defensa"))],
              elemento="Hielo"),
        # Congelacion (100 %, 7 s): «recibir siempre un ataque fatal» y el daño recibido de los ataques magicos
        # +20 %; la velocidad y la resistencia al agua, fuera
        carta("H942", "Frío", "all_enemies",
              "Una corriente de aire frío congela a los enemigos: 4d6 de daño de hielo a cada uno; durante 2 turnos, "
              "los golpes contra ellos son críticos y tus golpes les hacen +1.",
              [("4d6 de daño de hielo a cada enemigo.", dano(1200, 2000)),
               ("Congelación: los golpes contra ellos son críticos 2 turnos.", estado(7, "critico_contra")),
               ("Congelación: tus golpes les hacen +1 durante 2 turnos.", estado(7, "bono_dano_contra:1"))],
              elemento="Hielo"),
        carta("H943", "Tormenta de nieve", "all_enemies",
              "Desatas una potente tormenta de hielo: 4d6 de daño de hielo a cada enemigo y aturdimiento 1 turno (no "
              "atacan y se defienden con desventaja).",
              [("4d6 de daño de hielo a cada enemigo.", dano(1700, 2000))] + _aturdido(6), elemento="Hielo"),
    ],
    "arma_inicial": "orbe_de_mano_de_milicia",
    "set": [
        pieza("Vara del Mago azul", "MainHand", "ligera", {"INT": 1, "DES": 1},
              "La vara de la SP del Mago azul, con el cristal de hielo.",
              restricciones=["Precisión: INT"]),
        pieza("Túnica del Mago azul", "Torso", "ligera", {"INT": 1},
              "Túnica azul con las hombreras de hielo y el bajo blanco de copos."),
        pieza("Capucha del Mago azul", "Head", "ligera", {"INT": 1, "DES": 1},
              "Capucha azul con el ribete de piel blanca y un cristal de hielo en la frente."),
        pieza("Calzas del Mago azul", "Legs", "ligera", {"INT": 1, "DES": 1},
              "Calzas azules del Mago azul."),
        pieza("Botas del Mago azul", "Feet", "ligera", {"INT": 1, "DES": 1},
              "Botas de piel blanca del Mago azul."),
    ],
}

MAGO_DE_LAS_TINIEBLAS = {
    "id": "mago_de_las_tinieblas", "name": "Mago de las tinieblas", "sp": 4, "elemento": "Tinieblas",
    "nostale": "Mago de las tinieblas (Dark Gunner)",
    "description": ("Especialista de la pistola mágica de tinieblas: dispara balas de maná, maldice y posee a sus "
                    "enemigos, se protege con un guardián fantasma y desata la invasión de los fantasmas."),
    "primaryStat": "DES", "secondaryStat": "INT", "role": "agile",
    # §7: sus tecnicas son «de ataque a distancia», no magicas: tiran DES, lo de siempre (sin «tira»). El critico
    # +10 % del disparo basico no entra en la pasiva.
    "pasiva": ("Dispara el arma mágica",
               "La cadena de tres disparos del Mago de las tinieblas: tus golpes con arma hacen +1 de daño "
               "(tinieblas).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H945", "Tiro combinado", "single_enemy",
              "Disparas el arma mágica una y otra vez: 2d6 de daño de tinieblas.",
              # el critico +15 % de la tecnica no entra
              [("2d6 de daño de tinieblas.", dano(270, 580))], elemento="Tinieblas"),
        carta("H946", "Rayo mágico", "zone_radius",
              "Un rayo mágico estalla sobre el enemigo: 2d6 de daño de tinieblas.",
              # herida de bala magica (20 %): fuera
              [("2d6 de daño de tinieblas.", dano(500, 700))], elemento="Tinieblas"),
        # Guardian Fantasma (100 %, 300 s): el daño recibido -20 % (+1 a la CA) y el critico +30 % con el acierto
        # +(N x2) (ventaja); el 20 % del daño que pasa a los MP y la velocidad +3, fuera. Arma espiritual (100 %:
        # daño critico +25 %): el daño critico no se simula
        carta("H947", "Guardián Fantasma", "self",
              "Un guardián fantasma aparece para protegerte hasta el final del combate: +1 a la CA, y atacas con "
              "ventaja.",
              [("Guardián Fantasma: +1 a tu CA todo el combate.", propio(300, "bono_ca:1")),
               ("Guardián Fantasma: atacas con ventaja todo el combate.", propio(300, "ventaja:ataque"))]),
        carta("H948", "Tiro maldito", "single_enemy",
              "Un tiro maldito: 3d6 de daño de tinieblas, y la mano de la muerte le deja 1 turno sin atacar y hace "
              "que tus golpes le hagan +1.",
              # Mano de la Muerte (60 %, 5 s): ataque no posible y el daño recibido +15 %; herida de bala magica
              # (25 %): fuera
              [("3d6 de daño de tinieblas.", dano(900, 1300)),
               ("Mano de la Muerte: 1 turno sin atacar.", estado(5, "incapacitado")),
               ("Mano de la Muerte: tus golpes le hacen +1 durante 1 turno.", estado(5, "bono_dano_contra:1"))],
              elemento="Tinieblas"),
        # Poder de las Tinieblas (100 %, 300 s): tinieblas +(N x6), +1d6 (ver arriba); la resistencia a las
        # tinieblas +10 y el golpe fatal recibido -10 %, fuera
        carta("H949", "Maldición oscura", "all_allies",
              "Una maldición oscura refuerza el elemento de las tinieblas: tú y tus aliados hacéis +1d6 con los "
              "golpes de arma hasta el final del combate.",
              [("Poder de las Tinieblas: +1d6 a los golpes con arma todo el combate.",
                estado(300, "bono_dano_arma:1d6"))]),
        # Obsesion (100 %, 12 s), magica negativa y sin daño: salvacion de defensa mental (posesion). Ataque no
        # posible; el acierto -(N x2) no suma nada si no ataca, y la velocidad, fuera
        carta("H950", "Posesión", "single_enemy",
              "Un poderoso demonio maldice al enemigo. Si falla la salvación (defensa mental), queda poseído 2 "
              "turnos sin atacar.",
              [("Obsesión: 2 turnos sin atacar.", estado(12, "incapacitado"))],
              salvacion="defensa_mental"),
        carta("H951", "Recarga fantasma", "zone_radius",
              "Disparas fantasmas malditos una y otra vez: 3d6 de daño de tinieblas.",
              # Oscuridad de DOOM (70 %): la resistencia a las tinieblas -30 y el daño de sus criticos -50 %: ni
              # resistencias ni daño critico se simulan, nada que entre. Herida de bala magica (25 %): fuera
              [("3d6 de daño de tinieblas.", dano(1100, 650))], elemento="Tinieblas"),
    ],
    "creador": [
        # Pocion del Mal (100 %, 20 s), magica negativa y sin daño: salvacion de defensa mental (el espiritu que
        # lo llena). La vida que le quita cada 2 s pasa a ti: absorbe (como el Captor de espiritus del Cosechador)
        carta("H952", "Poción maléfica", "single_enemy",
              "Llenas al enemigo con un espíritu de las tinieblas. Si falla la salvación (defensa mental), le "
              "absorbes 1 de vida por turno durante 3 turnos.",
              [("Poción del Mal: le absorbes 1 de vida por turno durante 3 turnos.",
                estado(20, "absorbe_por_turno:1")),
               _pendiente("que le quite sus estados a favor")],
              salvacion="defensa_mental"),
        # Requiem (100 %, 10 s; negativo, no magico: sin salvacion): el daño recibido +15 % y la vida que pierde
        # cada 2 s. Que «huyan aterrorizados» es de la descripcion, no del estado
        carta("H953", "Réquiem", "all_enemies",
              "El réquiem del demonio cae sobre los enemigos: durante 2 turnos pierden 1 de vida por turno y tus "
              "golpes les hacen +1.",
              [("Réquiem: tus golpes les hacen +1 durante 2 turnos.", estado(10, "bono_dano_contra:1")),
               ("Réquiem: 1 de daño por turno durante 2 turnos.", estado(10, "dano_por_turno:1"))]),
        carta("H954", "Invasión de los fantasmas", "all_enemies",
              "Disparas contra los fantasmas malditos que te rodean: 4d6 de daño de tinieblas a cada enemigo, y la "
              "plaga los hace sangrar 3 turnos.",
              # Plaga (70 %, 30 s): la vida cada 2 s, sangran; los MP, fuera. Sin penalizacion a corta distancia:
              # el alcance no se simula
              [("4d6 de daño de tinieblas a cada enemigo.", dano(1500, 2200)),
               ("Plaga: sangran 3 turnos.", condicion("sangrado", 30))], elemento="Tinieblas"),
    ],
    # El catalogo no tiene pistola de tier 1: una normal, como las de Viaje (balas de mana, nunca polvora).
    "arma_inicial": "pistola_magica_de_viaje",
    "armas_nuevas": [
        PISTOLA_MAGICA,
    ],
    "set": [
        pieza("Pistola Mágica del Mago de las tinieblas", "MainHand", "ligera", {"DES": 1, "INT": 1},
              "La pistola mágica de la SP del Mago de las tinieblas, negra con el cristal violeta: dispara balas de "
              "maná.",
              restricciones=["Precisión: DES"]),
        pieza("Levita del Mago de las tinieblas", "Torso", "ligera", {"DES": 1},
              "Levita negra con el forro violeta y los botones de plata."),
        pieza("Sombrero de copa del Mago de las tinieblas", "Head", "ligera", {"DES": 1, "INT": 1},
              "Sombrero de copa negro con la cinta violeta."),
        pieza("Pantalones del Mago de las tinieblas", "Legs", "ligera", {"DES": 1, "INT": 1},
              "Pantalones negros del Mago de las tinieblas."),
        pieza("Botas del Mago de las tinieblas", "Feet", "ligera", {"DES": 1, "INT": 1},
              "Botas negras del Mago de las tinieblas."),
    ],
}


VOLCANO = {
    "id": "volcano", "name": "Volcano", "sp": 5, "elemento": "Fuego", "nostale": "Volcano",
    "description": ("Especialista del magma de fuego: hace estallar el volcán bajo los enemigos, se cubre con el "
                    "tanque de magma y los arrasa con la tormenta de cenizas."),
    "primaryStat": "INT", "secondaryStat": "CON", "role": "balanced", "tira": "INT",
    "pasiva": ("Bola de magma",
               "Las bolas de magma de Volcano: tus golpes con arma hacen +1 de daño (fuego).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    # El Poder del volcán se activa con su carta (§2.3: es carta, no forma). Lo que refuerza de las demás («si el mago
    # se ve influido por el poder del volcán, esto resulta muy efectivo»: Erupción volcánica, Gas volcánico, Terremoto,
    # Rugido del volcán, Tormenta de cenizas) no tiene cifras en la ficha (§10): nada.
    "cartas": [
        # Erupción volcánica abre Erupción continua (H1931, su continuación): una carta que suma las dos (§2). La
        # Lengua de lava de las dos (20 %): fuera
        carta("H1079+H1931", "Erupción volcánica", "zone_radius",
              "El volcán entra en erupción bajo el enemigo y sigue estallando: 3d6 de daño de fuego.",
              [("3d6 de daño de fuego.", dano(250, 450, 250, 450))], elemento="Fuego"),
        carta("H1080", "Gas volcánico", "all_enemies",
              "Un gas volcánico se extiende a tu alrededor: 2d6 de daño de fuego a cada enemigo, y el veneno los hace "
              "sangrar 3 turnos.",
              # Gas venenoso del volcán (50 %): pierde vida cada 4 s por el veneno (sangra); la resistencia al fuego,
              # fuera
              [("2d6 de daño de fuego a cada enemigo.", dano(450, 650)),
               ("Gas venenoso del volcán: sangran 3 turnos.", condicion("sangrado", 25))], elemento="Fuego"),
        # Escudo de magma (240 s), el escudo de maná de la SP: el daño recibido -20 % (bono_ca:1); tratar el daño con
        # los MP, fuera. Tanque de magma (12 s): la defensa +(N *3) (bono_ca:1, como el Caldero mágico del Ángel
        # vengador) y el tope al daño de los críticos, como el Escudo vital del Espía; la velocidad y el espejismo,
        # fuera
        carta("H1081", "Tanque de magma", "self",
              "El magma cubre tu armadura: el escudo de magma te da +1 a la CA hasta el final del combate, y el tanque "
              "de magma, +1 más durante 2 turnos.",
              [("Escudo de magma: +1 a tu CA hasta el final del combate.", propio(240, "bono_ca:1")),
               ("Tanque de magma: +1 más a tu CA durante 2 turnos.", estado(12, "bono_ca:1")),
               _pendiente("que el tanque de magma limite el daño de los críticos que recibes (3 veces)")]),
        carta("H1082", "Ola de lava", "zone_radius",
              "Una ola de lava alcanza al enemigo y se solidifica sobre él: 3d6 de daño de fuego.",
              # Lengua de lava grande (75 %): solo la resistencia al fuego -15 %, que no se simula; quedar atrapado en
              # la lava es «una determinada probabilidad», sin cifra: fuera
              [("3d6 de daño de fuego.", dano(700, 1200))], elemento="Fuego"),
        carta("H1084", "Terremoto", "all_enemies",
              "El rugido del volcán hace temblar la tierra a tu alrededor: 3d6 de daño de fuego a cada enemigo.",
              # Lengua de lava (35 %): fuera
              [("3d6 de daño de fuego a cada enemigo.", dano(800, 900))], elemento="Fuego"),
        # Rugido del volcán abre Réplicas (H1932, su continuación): una carta que suma las dos (§2). El teletransporte,
        # sin posiciones; la Lengua de lava de las réplicas (25 %), fuera. Lava solidificada (75 %): «ataque no
        # posible» y «no hay defensa posible»; no moverse, fuera
        carta("H1085+H1932", "Rugido del volcán", "all_enemies",
              "Te teletransportas sobre el enemigo, el volcán ruge y las réplicas sacuden a los cercanos: 4d6 de daño "
              "de fuego a cada enemigo, y la lava solidificada los deja 1 turno sin atacar y defendiéndose con "
              "desventaja.",
              [("4d6 de daño de fuego a cada enemigo.", dano(700, 900, 800, 900)),
               ("Lava solidificada: 1 turno sin atacar.", estado(3, "incapacitado")),
               ("Lava solidificada: se defienden con desventaja 1 turno.", estado(3, "desventaja:defensa"))],
              elemento="Fuego"),
        # Poder del volcán (240 s): el foco +(N *1) (ventaja en ataque) y los ataques mágicos +5 % (+1; las habilidades
        # ígneas +(N /3) son el mismo +1); el elemento del hada, fuera. «Puede derribar al enemigo con una probabilidad
        # que va en aumento»: sin cifras, fuera
        carta("H1083", "Poder del volcán", "self",
              "El poder del volcán te llena hasta el final del combate: atacas con ventaja y tus golpes con arma hacen "
              "+1.",
              [("Poder del volcán: ventaja en ataque todo el combate.", estado(240, "ventaja:ataque")),
               ("Poder del volcán: +1 a tus golpes con arma todo el combate.", estado(240, "bono_dano_arma:1"))]),
    ],
    # Creador: las de profesión 12, 16 y 20.
    "creador": [
        # El pantano es un área colocada: sin posiciones, hace su efecto al jugar la carta. Pantano de lava (100 %): el
        # acierto -(N *4) (desventaja en ataque); la velocidad y la resistencia al fuego, fuera
        carta("H1086", "Pantano de lava", "all_enemies",
              "Un pantano de lava se abre a tu alrededor: 3d6 de daño de fuego a cada enemigo, que ataca con "
              "desventaja 1 turno.",
              [("3d6 de daño de fuego a cada enemigo.", dano(500, 900)),
               ("Pantano de lava: atacan con desventaja 1 turno.", estado(6, "desventaja:ataque"))], elemento="Fuego"),
        # La espada ataca una vez: carta de daño, no invocación. Lava ardiente (75 %): solo la resistencia al fuego
        # -25 %: fuera
        carta("H1087", "Espada de magma", "zone_radius",
              "Invocas una espada de lava que cae sobre el enemigo: 4d6 de daño de fuego.",
              [("4d6 de daño de fuego.", dano(1200, 1700))], elemento="Fuego"),
        # Primera onda expansiva (100 %): pierde vida cada 2 s, como con el fuego; no moverse (sin posiciones) y la
        # resistencia al fuego, fuera
        carta("H1088", "Tormenta de cenizas", "all_enemies",
              "Una tormenta de cenizas ardientes arrasa a tu alrededor: 4d6 de daño de fuego a cada enemigo, y la onda "
              "expansiva les quita 1 de vida por turno durante 1 turno.",
              [("4d6 de daño de fuego a cada enemigo.", dano(1500, 2000)),
               ("Primera onda expansiva: 1 de daño por turno durante 1 turno.", estado(3, "dano_por_turno:1"))],
              elemento="Fuego"),
    ],
    "arma_inicial": "baston_ferrado_de_milicia",
    "set": [
        pieza("Báculo de Volcano", "MainHand", "media", {"INT": 1, "CON": 1},
              "El báculo de la SP de Volcano: de roca, con la brasa de magma.",
              restricciones=["Precisión: INT"]),
        pieza("Coraza de Volcano", "Torso", "media", {"INT": 1},
              "Placas de basalto con vetas de lava."),
        pieza("Yelmo de Volcano", "Head", "media", {"INT": 1, "CON": 1},
              "Yelmo de obsidiana con dos cuernos de roca y grietas de lava."),
        pieza("Grebas de Volcano", "Legs", "media", {"INT": 1, "CON": 1},
              "Grebas de basalto de Volcano."),
        pieza("Botas de Volcano", "Feet", "media", {"INT": 1, "CON": 1},
              "Botas de roca de Volcano."),
    ],
}

SENOR_DE_LAS_MAREAS = {
    "id": "senor_de_las_mareas", "name": "Señor de las mareas", "sp": 6, "elemento": "Agua",
    "nostale": "Señor de las mareas (Tide Lord)",
    "description": ("Especialista del tridente de agua: desata tormentas y rayos, duerme a los enemigos con el canto "
                    "de las sirenas y suelta al tiburón y a la ballena azul."),
    "primaryStat": "INT", "secondaryStat": "CAR", "role": "caster", "tira": "INT",
    "pasiva": ("Tridente del trueno",
               "El rayo en forma de tridente del Señor de las mareas: tus golpes con arma hacen +1 de daño (agua).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        carta("H1103", "Megatormenta", "all_enemies",
              "Tres torbellinos se unen en un tifón sobre los enemigos: 2d6 de daño de agua a cada uno.",
              # Cascada pequeña (30 %): fuera
              [("2d6 de daño de agua a cada enemigo.", dano(200, 500))], elemento="Agua"),
        carta("H1104", "El gran tridente", "zone_radius",
              "El gran tridente abre un campo magnético sobre el enemigo: 2d6 de daño de agua.",
              # Electrochoque pequeño (40 %): fuera
              [("2d6 de daño de agua.", dano(600, 450))], elemento="Agua"),
        # Bendición del océano (300 s): el nivel de defensa +1 y la defensa +(N *1) (+1 a la CA); el 5 % de no recibir
        # nunca un efecto negativo no llega al 50 %: fuera
        carta("H1105", "Bendición del océano", "all_allies",
              "El océano envuelve al grupo en un aura mágica: tú y tus aliados tenéis +1 a la CA hasta el final del "
              "combate.",
              [("Bendición del océano: +1 a la CA todo el combate.", estado(300, "bono_ca:1"))]),
        # Rayo abre Rayo doble (H1113, su continuación, al 80 %): una carta que suma los dos (§2). El Megarrelámpago
        # (H1445) que el Rayo doble abre un 20 % de las veces no llega al 50 %: fuera (los dados ya son 4d6 sin él y su
        # electrochoque es el mismo turno sin atacar del Rayo). Electrochoque (50 %): «ataque no posible»; el
        # Electrochoque grande del Rayo doble (40 %), fuera
        carta("H1106+H1113", "Rayo", "single_enemy",
              "El rayo quema al enemigo y el trueno vuelve con un rayo doble: 4d6 de daño de rayo, y el electrochoque "
              "le deja 1 turno sin atacar.",
              [("4d6 de daño de rayo.", dano(850, 600, 1000, 650)),
               ("Electrochoque: 1 turno sin atacar.", estado(4, "incapacitado"))], elemento="Rayo"),
        # El tiburón muerde una vez: carta de daño, no invocación. Hemorragia mortal (60 %): sangra, como el mismo
        # estado en el Arquero; Cascada (60 %): la velocidad y la resistencia al agua, no se simulan
        carta("H1107", "Hora de comer", "all_enemies",
              "Sueltas a tu tiburón hambriento, que nada hasta los enemigos y les da un buen mordisco: 3d6 de daño de "
              "agua a cada uno, que sangra 2 turnos.",
              [("3d6 de daño de agua a cada enemigo.", dano(950, 950)),
               ("Hemorragia mortal: sangran 2 turnos.", condicion("sangrado", 10))], elemento="Agua"),
        # Ni daño ni estado en la ficha, y lo que hace (atrapar y frenar a los enemigos) son posiciones y velocidad: la
        # carta se queda vacía y sale bloqueada (§10), como la Granada gravitatoria del Granadero
        carta("H1108", "Remolino gigante", "single_enemy",
              "Abres un remolino gigante donde eliges, que atrapa a los enemigos y los frena. [pendiente] atraparlos y "
              "frenarlos: el combate no tiene posiciones ni velocidad.",
              [("[pendiente] Atrapar y frenar a los enemigos en el remolino.",
                "pendiente:atrapar y frenar a los enemigos en el remolino (sin posiciones ni velocidad en el "
                "combate)")]),
        # El canto de las sirenas (100 %, 16 s): el sueño, «ataque no posible»; no moverse y la defensa contra los
        # elementos que gana el dormido, fuera; lo de PvP, también. Control sin daño y de efecto mágico: magia, con
        # salvación (§4)
        carta("H1109", "El canto de las sirenas", "all_enemies",
              "Cantas como las sirenas. Quien falle la salvación (defensa mental) se duerme 3 turnos (no ataca).",
              [("Sueño: 3 turnos sin atacar.", estado(16, "incapacitado"))], salvacion="defensa_mental"),
    ],
    # Creador: las de profesión 12, 16 y 20 (Rayo doble y Megarrelámpago, también de 20, son continuaciones del Rayo).
    "creador": [
        # Cascada grande (70 %): la defensa mágica -(N *2) (-1 a la CA); la velocidad y la resistencia al agua, fuera.
        # Empujar (solo en PvP): fuera
        carta("H1110", "Tsunami", "all_enemies",
              "Una ola monstruosa arrasa a los enemigos: 4d6 de daño de agua a cada uno, y la cascada grande les deja "
              "-1 a la CA durante 2 turnos.",
              [("4d6 de daño de agua a cada enemigo.", dano(600, 2000)),
               ("Cascada grande: -1 a la CA durante 2 turnos.", estado(10, "penal_ca:1"))], elemento="Agua"),
        # Burbuja de agua (180 s), el escudo de maná de la SP: el daño recibido -25 % (bono_ca:2); tratar el daño con
        # los MP, fuera; el 10 % de resistencia a las quemaduras no llega al 50 %, y debilitar las mejoras de ataque
        # del enemigo no trae cifras: fuera
        carta("H1111", "Burbuja de agua", "self",
              "Una burbuja de agua te protege de los ataques: +2 a la CA hasta el final del combate.",
              [("Burbuja de agua: +2 a tu CA hasta el final del combate.", propio(180, "bono_ca:2"))]),
        # La ballena cae una vez: carta de daño, no invocación. Presión hidráulica (80 %): «ataque no posible»; no
        # moverse, fuera
        carta("H1112", "¡Cuidado, una ballena azul!", "all_enemies",
              "Una ballena azul cae del cielo sobre los enemigos: 4d6 de daño de agua a cada uno, y la presión "
              "hidráulica los deja 1 turno sin atacar.",
              [("4d6 de daño de agua a cada enemigo.", dano(1850, 1850)),
               ("Presión hidráulica: 1 turno sin atacar.", estado(5, "incapacitado"))], elemento="Agua"),
    ],
    "arma_inicial": "vara_de_aprendiz_de_viaje",
    "set": [
        pieza("Tridente del Señor de las mareas", "MainHand", "ligera", {"INT": 1, "CAR": 1},
              "El tridente de la SP del Señor de las mareas: tres puntas que lanzan el rayo.",
              restricciones=["Precisión: INT"]),
        pieza("Coraza del Señor de las mareas", "Torso", "ligera", {"INT": 1},
              "Escamas verde mar, con hombreras de concha y la capa de agua."),
        pieza("Corona del Señor de las mareas", "Head", "ligera", {"INT": 1, "CAR": 1},
              "Corona de coral y oro, con perlas."),
        pieza("Calzas del Señor de las mareas", "Legs", "ligera", {"INT": 1, "CAR": 1},
              "Calzas verde mar del Señor de las mareas."),
        pieza("Sandalias del Señor de las mareas", "Feet", "ligera", {"INT": 1, "CAR": 1},
              "Sandalias de cuero del Señor de las mareas."),
    ],
}

VIDENTE = {
    "id": "vidente", "name": "Vidente", "sp": 7, "elemento": "Tinieblas", "nostale": "Vidente (Seer)",
    "description": ("Especialista del orbe de tinieblas: siembra el caos y el espejismo, consume a los enemigos con la "
                    "energía oscura y desata la magia prohibida de sus clones."),
    "primaryStat": "INT", "secondaryStat": "DES", "role": "caster", "tira": "INT",
    # Robar o quemar maná y tocar los enfriamientos (la «manipulación de recursos» del Vidente) no se simulan (§10).
    "pasiva": ("Bola mágica",
               "Las bolas mágicas oscuras del Vidente: tus golpes con arma hacen +1 de daño (tinieblas).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    # Los Clones oscuros de Súperabracadabra («invoca hasta 4»). NosTale no publica sus cifras: las de una sombra de
    # tier 1 del bestiario, la Sombra del Paso Ligero (CON 1, DES 4, CAR 1; INT 0, como las invocaciones; el cargador
    # lee CON, DES, INT y CAR), y los 5 PV de las invocaciones de tier 1. La ficha no dice cuánto duran: lo mínimo,
    # 1 turno (atacan una vez), como la Nube de vapor del Espía.
    "invocaciones": [
        {"id": "clon_oscuro", "name": "Clon oscuro", "description": "CON 1, DES 4, INT 0, CAR 1, 5 PV.",
         "health": 5, "duration": "1 turno", "summonedBy": "vidente_superabracadabra",
         "stats": {"CON": 1, "DES": 4, "INT": 0, "CAR": 1},
         "attacks": [
             {"name": "Bola mágica", "effect": "Dispara una bola mágica oscura al enemigo, como las del Vidente."},
         ],
         "passive": {"name": "Magia prohibida",
                     "description": ("Una copia oscura del Vidente: ataca una vez al cerrar tu turno y se desvanece al "
                                     "acabar la ronda.")}},
    ],
    "cartas": [
        # Caos (100 %, 3 s): «ataque no posible» y la vida que pierde; los MP, fuera. Energía oscura débil (100 %,
        # 6 s): la misma vida que pierde (cada 2 s: 1 de daño por turno) y el daño de tinieblas +10 % «con una
        # probabilidad del 50 %» (llega al 50 %: tus golpes le hacen +1); la velocidad, fuera
        carta("H1129", "Caos", "all_enemies",
              "Arruinas el juicio de los enemigos: 2d6 de daño de tinieblas a cada uno; el caos los deja 1 turno sin "
              "atacar, y la energía oscura les quita 1 de vida por turno y hace que tus golpes les hagan +1 durante 1 "
              "turno.",
              [("2d6 de daño de tinieblas a cada enemigo.", dano(200, 300)),
               ("Caos: 1 turno sin atacar.", estado(3, "incapacitado")),
               ("Caos y energía oscura débil: 1 de daño por turno durante 1 turno.", estado(6, "dano_por_turno:1")),
               ("Energía oscura débil: tus golpes les hacen +1 durante 1 turno.", estado(6, "bono_dano_contra:1"))],
              elemento="Tinieblas"),
        # Atraparlo en la jaula: sin posiciones; el daño, al romperla. Energía oscura (70 %): la vida que pierde y el
        # daño de tinieblas +15 % la mitad de las veces, como en el Caos; la velocidad, fuera
        carta("H1130", "Jaula mortal", "zone_radius",
              "Encierras al enemigo en una jaula de acero y la rompes sobre él: 2d6 de daño de tinieblas, y la energía "
              "oscura le quita 1 de vida por turno y hace que tus golpes le hagan +1 durante 2 turnos.",
              [("2d6 de daño de tinieblas.", dano(300, 400)),
               ("Energía oscura: 1 de daño por turno durante 2 turnos.", estado(8, "dano_por_turno:1")),
               ("Energía oscura: tus golpes le hacen +1 durante 2 turnos.", estado(8, "bono_dano_contra:1"))],
              elemento="Tinieblas"),
        # Los dos Parpadeo (el segundo sale un 60 % de las veces tras el primero y da lo mismo) son la misma técnica:
        # una carta (§2). El teletransporte, sin posiciones. Armadura de la oscuridad (180 s), el escudo de maná de la
        # SP: el daño recibido -15 % (bono_ca:1); la velocidad y tratar el daño con los MP, fuera
        carta("H1131+H1149", "Parpadeo", "self",
              "Te teletransportas en un parpadeo y te cubre la armadura de la oscuridad: +1 a la CA hasta el final del "
              "combate.",
              [("Armadura de la oscuridad: +1 a tu CA hasta el final del combate.", propio(180, "bono_ca:1"))]),
        carta("H1132", "Manos malditas", "zone_radius",
              "Unas manos malditas se aferran al enemigo: 3d6 de daño de tinieblas.",
              # Manos malditas (40 %) y Energía oscura (30 %): fuera
              [("3d6 de daño de tinieblas.", dano(550, 800))], elemento="Tinieblas"),
        # El Espejismo de los monstruos (100 %, 10 s) es control por miedo (asustado) y no recuperar vida (pendiente);
        # el de PvP, fuera. Energía oscura fuerte (100 %): la vida que pierde y el daño de tinieblas +25 % la mitad de
        # las veces, como en el Caos; la velocidad, fuera. Control sin daño y de efecto mágico: magia, con salvación
        # (§4)
        carta("H1133", "Espejismo", "all_enemies",
              "Un espejismo confunde a los enemigos. Quien falle la salvación (defensa mental) huye asustado 2 turnos, "
              "y la energía oscura le quita 1 de vida por turno y hace que tus golpes le hagan +1 durante 2 turnos.",
              [("Espejismo: asustado 2 turnos.", condicion("asustado", 10)),
               ("Energía oscura fuerte: 1 de daño por turno durante 2 turnos.", estado(10, "dano_por_turno:1")),
               ("Energía oscura fuerte: tus golpes le hacen +1 durante 2 turnos.", estado(10, "bono_dano_contra:1")),
               _pendiente("que no recupere vida mientras dura el espejismo")], salvacion="defensa_mental"),
        carta("H1134", "Extracción de maná", "zone_radius",
              "Un golpe de tinieblas le arranca el maná al enemigo: 3d6 de daño de tinieblas.",
              # robarle maná: el MP no se simula; Energía oscura débil (20 %): fuera
              [("3d6 de daño de tinieblas.", dano(1000, 1000))], elemento="Tinieblas"),
        # Absorción mental (la ficha no da su duración): el 100 % de no recibir efectos negativos de nivel 4 o menos (lo
        # que el combate puede hacer es quitarte los que llevas: limpia) y el 5 % de la vida por golpe recibido, dos
        # veces (un 10 %: cura 1d6); los MP, fuera. Primero limpia y después cura (§4)
        carta("H1135", "Absorción mental", "self",
              "Tu mente absorbe el daño que recibes: te libras de los estados en contra y te curas 1d6.",
              [("Te libras de los estados en contra.", "limpia"), ("Te curas 1d6.", "cura:1d6"),
               _pendiente("que la vida sea temporal (se pierde a los 10 s) y llegue con los golpes que recibes")]),
    ],
    # Creador: las de profesión 12, 16 y 20 (los Cambio de hoja y el Parpadeo de 20 van con su técnica).
    "creador": [
        # Los tres Cambio de hoja (el H1136 abre el H1139, que va y vuelve con el H1140) son la misma técnica: una carta
        # que suma cada paso una vez (§2), como la Recarga rápida del Espía. El estado Cambio de hoja (los objetos de
        # recuperación -15 %) y la Energía oscura débil (10 %): fuera
        carta("H1136+H1139+H1140", "Cambio de hoja", "zone_radius",
              "Tus bolas mágicas se vuelven hojas que disparas sin parar: 2d6 de daño de tinieblas.",
              [("2d6 de daño de tinieblas.", dano(200, 200, 200, 10, 10, 200))], elemento="Tinieblas"),
        # Vacío temporal fuerte (los enfriamientos del enemigo +50 %) y la reducción de enfriamiento de la Rueda de la
        # fortuna intensa: no se simulan (§10). Queda el +20 % a los ataques mágicos de la Rueda (5 s, tuya): +1d6. Con
        # 1 turno se acabaría al cerrar esta ronda, antes de tu golpe: 2, para que llegue a tu turno siguiente, como la
        # Emboscada del Espía
        carta("H1137", "Pérdida de tiempo", "all_enemies",
              "Frenas el tiempo de los enemigos y aceleras el tuyo: la rueda de la fortuna te da +1d6 a tus golpes con "
              "arma durante 2 turnos.",
              [("Rueda de la fortuna intensa: +1d6 a tus golpes con arma 2 turnos.", "propio:2:bono_dano_arma:1d6")]),
        # La carta que trae los clones («invocaciones»): los cuatro del «hasta 4», uno por gancho, como las arañas del
        # Sacerdote vudú. Sin segundos en la ficha, 1 turno (ver arriba)
        carta("H1138", "Súperabracadabra", "single_enemy",
              "El gigantesco hechizo de la magia prohibida: invocas cuatro clones oscuros que luchan a tu lado 1 "
              "turno.",
              [("Un clon oscuro lucha a tu lado 1 turno.", "invoca:1:clon_oscuro"),
               ("El segundo clon oscuro lucha a tu lado 1 turno.", "invoca:1:clon_oscuro"),
               ("El tercer clon oscuro lucha a tu lado 1 turno.", "invoca:1:clon_oscuro"),
               ("El cuarto clon oscuro lucha a tu lado 1 turno.", "invoca:1:clon_oscuro"),
               _pendiente("cuánto duran los clones (la ficha no da los segundos)")]),
    ],
    "arma_inicial": "orbe_de_mano_de_viaje",
    "set": [
        pieza("Orbe del Vidente", "MainHand", "ligera", {"INT": 1, "DES": 1},
              "El orbe de la SP del Vidente: la bola de cristal violeta.",
              restricciones=["Precisión: INT"]),
        pieza("Túnica del Vidente", "Torso", "ligera", {"INT": 1},
              "Túnica violeta oscuro con estrellas de oro y la faja de las cartas."),
        pieza("Turbante del Vidente", "Head", "ligera", {"INT": 1, "DES": 1},
              "Turbante violeta con la joya del ojo."),
        pieza("Calzas del Vidente", "Legs", "ligera", {"INT": 1, "DES": 1},
              "Calzas violeta oscuro del Vidente."),
        pieza("Babuchas del Vidente", "Feet", "ligera", {"INT": 1, "DES": 1},
              "Babuchas de punta curva del Vidente."),
    ],
}

ARCHIMAGO = {
    "id": "archimago", "name": "Archimago", "sp": 8, "elemento": "Luz", "nostale": "Archimago (Archmage)",
    "description": ("Especialista del grimorio de luz: marca a los enemigos con el polvo luminoso, los aterra con la "
                    "luz sacra y bendice al grupo con la iluminación."),
    "primaryStat": "INT", "secondaryStat": "CAR", "role": "caster", "tira": "INT",
    "pasiva": ("Magia de la luz",
               "El encantamiento de luz del Archimago: tus golpes con arma hacen +1 de daño (luz).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    "cartas": [
        # Polvo luminoso (80 %, 30 s): el acierto -(N *2) (desventaja en ataque) y la marca que aprovecha la Explosión
        # sagrada (marcado: los golpes contra ellos hacen +1); la energía de la luz -10, una resistencia: fuera
        carta("H1323", "Catalizador lumínico", "all_enemies",
              "Dispersas el polvo luminoso y lo haces estallar: 2d6 de daño de luz a cada enemigo, y el polvo los deja "
              "marcados y atacando con desventaja 3 turnos.",
              [("2d6 de daño de luz a cada enemigo.", dano(250, 450)),
               ("Polvo luminoso: atacan con desventaja 3 turnos.", estado(30, "desventaja:ataque")),
               ("Polvo luminoso: quedan marcados 3 turnos.", condicion("marcado", 30))], elemento="Luz"),
        carta("H1324", "Supresión", "all_enemies",
              "Cubres el cuerpo de los enemigos con luz: 2d6 de daño de luz a cada uno.",
              # retrasarlos: Interrumpir el movimiento levemente (15 %), y es velocidad: fuera
              [("2d6 de daño de luz a cada enemigo.", dano(350, 500))], elemento="Luz"),
        carta("H1325", "Prisma sacro", "zone_radius",
              "Una luz nívea atraviesa al enemigo: 3d6 de daño de luz, y el terror lo deja asustado 1 turno.",
              # Terror leve (60 %): «ataque no posible» por terror: asustado (§5: miedo, terror); la resistencia a la
              # luz -10, fuera
              [("3d6 de daño de luz.", dano(550, 1250)),
               ("Terror leve: asustado 1 turno.", condicion("asustado", 5))], elemento="Luz"),
        # El daño de más contra los marcados con Polvo luminoso, sin cifra en la ficha: la marca del Catalizador
        # lumínico (marcado, +1 a los golpes contra ellos). Interrumpir el movimiento (10 %): fuera
        carta("H1326", "Explosión sagrada", "all_enemies",
              "La luz sacra estalla sobre los enemigos: 2d6 de daño de luz a cada uno (los marcados por el polvo "
              "luminoso reciben +1).",
              [("2d6 de daño de luz a cada enemigo.", dano(280, 450))], elemento="Luz"),
        # Carril de adelantamiento 1 (8 s): un 5 % de las veces, +5 % al poder de ataque; no llega al 50 % y la carta se
        # queda vacía: sale bloqueada (§10)
        carta("H1327", "Carril de adelantamiento", "self",
              "Llevas tus hechizos al máximo. [pendiente] los niveles del carril: la ficha solo publica el primero.",
              [("[pendiente] Los niveles del Carril de adelantamiento.",
                "pendiente:los niveles del Carril de adelantamiento (la ficha solo da el 1: un 5 % de las veces, +5 % "
                "al ataque)")]),
        # Maestro de los libros de encantamientos (300 s), el escudo de maná de la SP: el daño recibido -(N /5) % (hasta
        # un 20 %: bono_ca:1); tratar el daño con los MP, los objetos de recuperación y la velocidad, fuera
        carta("H1328", "Maestro de los libros de encantamientos", "self",
              "Lanzas los hechizos con la maestría de los libros de encantamientos: +1 a la CA hasta el final del "
              "combate.",
              [("Maestro de los libros: +1 a tu CA hasta el final del combate.", propio(300, "bono_ca:1"))]),
        # Terror leve (80 %): asustado, como en el Prisma sacro; la resistencia a la luz, fuera. Control sin daño y de
        # efecto mágico: magia, con salvación (§4)
        carta("H1329", "Niebla sacra", "all_enemies",
              "Una niebla espesa en la que solo la cruz sagrada muestra el camino. Quien falle la salvación (defensa "
              "mental) queda asustado 1 turno.",
              [("Terror leve: asustado 1 turno.", condicion("asustado", 5))], salvacion="defensa_mental"),
    ],
    # Creador: las de profesión 12, 16 y 20. Dos salen bloqueadas: el combate no tiene posiciones y la ficha no da el
    # daño de los meteoritos.
    "creador": [
        # Monumento (60 s): la velocidad y los MP, no se simulan; sin el teletransporte la carta se queda vacía: sale
        # bloqueada (§10)
        carta("H1330", "Teleportador en masa", "all_allies",
              "Te llevas al grupo al lugar guardado. [pendiente] el teletransporte: el combate no tiene posiciones.",
              [("[pendiente] El teletransporte del grupo.",
                "pendiente:el teletransporte del grupo al lugar guardado (sin posiciones en el combate)")]),
        # «Diez meteoritos y (N /5) más», pero ni bonificación ni estado en la ficha: sin cifras, sale bloqueada, como
        # el Colocabombas del Destructor
        carta("H1331", "Tormenta de meteoritos", "all_enemies",
              "Haces llover meteoritos sobre los enemigos. [pendiente] el daño de los meteoritos: la ficha no lo "
              "publica.",
              [("[pendiente] El daño de los meteoritos.",
                "pendiente:el daño de los meteoritos (la ficha no lo publica)")], elemento="Luz"),
        # Iluminación e Iluminación débil (10 s): cada 2 s, un (N /5) % y un (N /7) % de la vida (a nivel 50, un 50 % y
        # un 36 % en los 10 s): más de un 20 %, cura 2d6, de una vez (el combate no cura por turnos); la resistencia a
        # la luz, fuera
        carta("H1332", "Iluminación", "all_allies",
              "Una bendición de luz sobre todo el grupo: tú y tus aliados os curáis 2d6.",
              [("Os curáis 2d6.", "cura:2d6")]),
    ],
    "arma_inicial": "baston_arcano",
    "set": [
        pieza("Grimorio del Archimago", "MainHand", "ligera", {"INT": 1, "CAR": 1},
              "El grimorio de la SP del Archimago: tapas blancas y oro.",
              restricciones=["Precisión: INT"]),
        pieza("Túnica del Archimago", "Torso", "ligera", {"INT": 1},
              "Túnica blanca y oro, con el cuello alto y el libro al cinto."),
        pieza("Sombrero del Archimago", "Head", "ligera", {"INT": 1, "CAR": 1},
              "Sombrero alto y puntiagudo, blanco, con la banda de oro y la estrella."),
        pieza("Calzas del Archimago", "Legs", "ligera", {"INT": 1, "CAR": 1},
              "Calzas blancas del Archimago."),
        pieza("Zapatos del Archimago", "Feet", "ligera", {"INT": 1, "CAR": 1},
              "Zapatos blancos del Archimago."),
    ],
}


SACERDOTE_VUDU = {
    "id": "sacerdote_vudu", "name": "Sacerdote vudú", "sp": 9, "elemento": "Tinieblas",
    "nostale": "Sacerdote vudú (Voodoo Priest)",
    "description": ("Especialista de las maldiciones de tinieblas: envenena y maldice a los enemigos, invoca arañas "
                    "espirituales que estallan y se vuelve espíritu para que no le alcancen."),
    "primaryStat": "INT", "secondaryStat": "CAR", "role": "caster", "tira": "INT",
    "pasiva": ("Rayo maldito",
               "El rayo oscuro del Sacerdote vudú: tus golpes con arma hacen +1 de daño (tinieblas).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1"))]),
    # La Maldición del sacerdote vudú (más daño de los estados que quitan vida por cada estado en contra) la ponen
    # casi todas sus técnicas, pero del 5 al 25 % de las veces: no llega al 50 % en ninguna, fuera.
    #
    # Las arañas de Araña espiritual («Invoca 3 veces»). NosTale no publica sus cifras: las de una criatura de tier 1
    # del bestiario. La única araña (Aranha de Taller) es de tier 2 y escaramuzadora: la escaramuzadora de tier 1 de
    # las tinieblas, la Sombra del Paso Ligero (CON 1, DES 4, CAR 1; INT 0, como las invocaciones; el cargador lee
    # CON, DES, INT y CAR), y los 5 PV de las invocaciones de tier 1. Atacan estallando: una vez cada una (1 turno).
    "invocaciones": [
        {"id": "arana_espiritual", "name": "Araña espiritual", "description": "CON 1, DES 4, INT 0, CAR 1, 5 PV.",
         "health": 5, "duration": "1 turno", "summonedBy": "sacerdote_vudu_arana_espiritual",
         "stats": {"CON": 1, "DES": 4, "INT": 0, "CAR": 1},
         "attacks": [
             {"name": "Estallido", "effect": "Se lanza contra el enemigo y estalla (la carta Araña espiritual)."},
         ],
         "passive": {"name": "Ataque explosivo",
                     "description": ("Una araña azul gigante: ataca una vez al cerrar tu turno, estallando, y se "
                                     "desvanece al acabar la ronda.")}},
    ],
    "cartas": [
        # Maldición de la serpiente venenosa (100 %, 15 s): una maldición que quita vida cada 2 s (dano_por_turno),
        # aunque la nube sea de veneno.
        carta("H1641", "Nube venenosa", "all_enemies",
              "Con el alma de Loa serpiente invocas una nube muy venenosa: 2d6 de daño de veneno a cada enemigo, y "
              "la maldición de la serpiente les quita 1 de vida por turno durante 2 turnos.",
              [("2d6 de daño de veneno a cada enemigo.", dano(300, 500)),
               ("Maldición de la serpiente venenosa: 1 de daño por turno durante 2 turnos.",
                estado(15, "dano_por_turno:1"))], elemento="Veneno"),
        # La carta que trae las arañas («invocaciones»): tres, una por «vez». Las maldiciones que «pueden» lanzar al
        # estallar no tienen cifra: fuera.
        carta("H1642", "Araña espiritual", "single_enemy",
              "Invocas tres arañas espirituales azules gigantes: durante 1 turno se lanzan contra los enemigos y "
              "estallan.",
              [("Una araña espiritual ataca 1 turno y estalla.", "invoca:1:arana_espiritual"),
               ("La segunda araña espiritual ataca 1 turno y estalla.", "invoca:1:arana_espiritual"),
               ("La tercera araña espiritual ataca 1 turno y estalla.", "invoca:1:arana_espiritual")]),
        # Transformación en espíritu (7 s): «no puedes recibir daños y no se te puede atacar» (como la ocultación: te
        # defiendes con ventaja) y la vida que recupera cada 2 s (N x10, unos N x30 en 7 s: 1d6), las dos que más
        # pesan. Quitar efectos dañinos (solo los de nivel menor que 4) y no poder atacar mientras dura, fuera.
        carta("H1643", "Figura de espíritu", "self",
              "Te transformas en un espíritu al que no se puede atacar: te defiendes con ventaja 2 turnos y te curas "
              "1d6.",
              [("Te curas 1d6.", "cura:1d6"),
               ("Transformación en espíritu: te defiendes con ventaja 2 turnos.", propio(7, "ventaja:defensa"))]),
        carta("H1644", "Alma pegajosa", "zone_radius",
              "Un alma y unos brazos que salen del suelo sujetan al enemigo: 3d6 de daño de tinieblas, y queda "
              "petrificado 1 turno (no ataca).",
              [("3d6 de daño de tinieblas.", dano(900, 900)),
               ("Cuerpo petrificado: 1 turno sin atacar.", estado(4, "incapacitado"))], elemento="Tinieblas"),
        # El enjambre ataca al invocarlo y nada más: carta de daño, no invocación. «Restaura (N x30) HP»: 1d6.
        carta("H1645", "Enjambre de langostas voraces", "all_enemies",
              "Invocas un enjambre de langostas voraces sobre los enemigos: 3d6 de daño de tinieblas a cada uno, y te "
              "curas 1d6.",
              [("3d6 de daño de tinieblas a cada enemigo.", dano(500, 1200)), ("Te curas 1d6.", "cura:1d6")],
              elemento="Tinieblas"),
        # Alma vengativa (15 s): el enemigo pierde el 30 % del daño que te hace (devuelve el daño: refleja). Escudo
        # anímico (300 s), el escudo de maná: daño recibido -25 % (+2 a la CA); el 30 % que pasa a los MP y la
        # velocidad, fuera.
        carta("H1646", "Alma vengativa", "self",
              "Invocas un alma con sed de venganza: durante 2 turnos devuelve los golpes a quien te ataca, y el escudo "
              "anímico te da +2 a la CA hasta el final del combate.",
              [("Alma vengativa: reflejas los golpes físicos 2 turnos.", propio(15, "refleja:fisico")),
               ("Escudo anímico: +2 a la CA todo el combate.", propio(300, "bono_ca:2"))]),
        # Muñeco vudú (100 %, 20 s): pierde vida cada 4 s, una maldición (dano_por_turno); la resistencia a las
        # tinieblas -20 %, fuera. Es «efecto negativo», no mágico: sin salvación (como la Posesión del
        # Cazademonios). La «consecuencia posterior si permanece sin eliminarse» no está en la ficha del estado.
        carta("H1647", "Muñeco vudú", "single_enemy",
              "Maldices al enemigo con un muñeco vudú: pierde 1 de vida por turno durante 3 turnos.",
              [("Muñeco vudú: 1 de daño por turno durante 3 turnos.", estado(20, "dano_por_turno:1")),
               _pendiente("la consecuencia si la maldición sigue sin quitarse (la ficha no la da)")]),
    ],
    # Creador: las de profesión 12, 16 y 20.
    "creador": [
        # Maldición del acuántropo (100 %, 8 s): no ataca y, contra un monstruo, daño recibido +20 %
        # (bono_dano_contra); a un jugador lo transforma (PvP), fuera. «Efecto negativo», no mágico: sin salvación.
        carta("H1648", "Figura de acuántropo", "zone_radius",
              "Una maldición convierte en acuántropos a los enemigos de la zona: durante 2 turnos no atacan y tus "
              "golpes les hacen +1.",
              [("Maldición del acuántropo: 2 turnos sin atacar.", estado(8, "incapacitado")),
               ("Maldición del acuántropo: tus golpes les hacen +1 durante 2 turnos.",
                estado(8, "bono_dano_contra:1")),
               _pendiente("que la maldición se rompa al recibir daño dos veces")]),
        # Guarda el 30 % del daño que hace para la técnica siguiente (Poder del alma concentrada, 20 s): la carta no
        # tiene cómo.
        carta("H1649", "Absorción del alma", "all_enemies",
              "Absorbes la energía espiritual de los enemigos: 3d6 de daño de tinieblas a cada uno.",
              [("3d6 de daño de tinieblas a cada enemigo.", dano(1000, 1200)),
               _pendiente("que la próxima técnica sume el 30 % del daño que guarda")], elemento="Tinieblas"),
        # «Solo afecta a enemigos con penalizadores»: sin contar estados, alcanza a todos (como «solo si tienes 1
        # marca»). Maldición mortal (100 %, 20 s): el daño de tinieblas que reciben +20 % (bono_dano_contra) y pierden
        # vida cada 2 s, una maldición (dano_por_turno).
        carta("H1650", "Maldición mortal", "all_enemies",
              "Lanzas una maldición mortal a tu alrededor: 4d6 de daño de tinieblas a cada enemigo; durante 3 turnos "
              "tus golpes les hacen +1 y pierden 1 de vida por turno.",
              [("4d6 de daño de tinieblas a cada enemigo.", dano(1500, 1000)),
               ("Maldición mortal: tus golpes les hacen +1 durante 3 turnos.", estado(20, "bono_dano_contra:1")),
               ("Maldición mortal: 1 de daño por turno durante 3 turnos.", estado(20, "dano_por_turno:1")),
               _pendiente("que el daño crezca con cada estado en contra que lleve el enemigo")], elemento="Tinieblas"),
    ],
    "arma_inicial": "cetro_de_bronce_de_viaje",
    "set": [
        pieza("Vara del Sacerdote vudú", "MainHand", "ligera", {"INT": 1, "CAR": 1},
              "La vara de la SP del Sacerdote vudú, con la calavera y las plumas.",
              restricciones=["Precisión: INT"]),
        pieza("Peto del Sacerdote vudú", "Torso", "ligera", {"INT": 1},
              "Correas cruzadas sobre el pecho pintado de blanco, con el collar de huesos."),
        pieza("Tocado del Sacerdote vudú", "Head", "ligera", {"INT": 1, "CAR": 1},
              "Tocado de plumas con una calavera pequeña."),
        pieza("Faldellín del Sacerdote vudú", "Legs", "ligera", {"INT": 1, "CAR": 1},
              "Faldellín de rafia del Sacerdote vudú."),
        pieza("Sandalias del Sacerdote vudú", "Feet", "ligera", {"INT": 1, "CAR": 1},
              "Sandalias de cuero del Sacerdote vudú."),
    ],
}

GRAVITAS = {
    "id": "gravitas", "name": "Grávitas", "sp": 10, "elemento": "Agua", "nostale": "Grávitas (Gravity)",
    "description": ("Especialista de la gravedad de agua: atrae y aparta a los enemigos, les deja caer piedras "
                    "gigantes y los traga con un agujero negro."),
    "primaryStat": "INT", "secondaryStat": "CON", "role": "balanced", "tira": "INT",
    "pasiva": ("Telequinesis",
               "La piedra que lanza Grávitas con la telequinesis: tus golpes con arma hacen +1 de daño (agua).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("los puntos de gravedad y antigravedad (los dan sus técnicas y los gasta el Agujero "
                           "negro) y el Equilibrio gravitatorio")]),
    # Sin niveles de profesión en la tarjeta: Creador = las tres últimas (§3). Lo que añade el Equilibrio
    # gravitatorio a Aplastar, Huida rápida, Hipergravedad y la Reflexión por antigravedad, fuera (lo dice la pasiva).
    "cartas": [
        # rigidez (30 %): fuera
        carta("H1744", "Atracción", "zone_radius",
              "La energía gravitatoria atrae a los enemigos hacia ti: 2d6 de daño de agua, y te atacan a ti 1 turno.",
              [("2d6 de daño de agua.", dano(300, 500)), _atrae()], elemento="Agua"),
        # empujar 4 casillas (solo en PvP): sin posiciones; rigidez (30 %): fuera
        carta("H1745", "Empujar", "zone_radius",
              "La energía antigravitatoria aparta a los enemigos de ti: 2d6 de daño de agua.",
              [("2d6 de daño de agua.", dano(400, 600))], elemento="Agua"),
        # el Aturdimiento mortal (50 %; con el Equilibrio, siempre) entra
        carta("H1746", "Aplastar", "all_enemies",
              "Dejas caer una piedra gigante sobre los enemigos: 3d6 de daño de agua a cada uno y aturdimiento 1 "
              "turno (no atacan y se defienden con desventaja).",
              [("3d6 de daño de agua a cada enemigo.", dano(800, 800))] + _aturdido(6), elemento="Agua"),
        # Campo antigravitatorio (180 s), el escudo de maná: daño recibido -15 % (+1 a la CA); el 30 % que pasa a los
        # MP y las resistencias con el Equilibrio gravitatorio, fuera.
        carta("H1747", "Campo antigravitatorio", "self",
              "Te rodeas de una protección antigravitatoria: +1 a la CA hasta el final del combate.",
              [("Campo antigravitatorio: +1 a la CA todo el combate.", propio(180, "bono_ca:1"))]),
        # alejarlo (4 %) y las Consecuencias gravitatorias con el Equilibrio (velocidad y enfriamientos): fuera
        carta("H1748", "Huida rápida", "single_enemy",
              "Golpeas al enemigo y lo apartas de un empujón para huir de él: 3d6 de daño de agua.",
              [("3d6 de daño de agua.", dano(700, 900))], elemento="Agua"),
        # Campo gravitatorio (tuyo, 10 s): ataques mágicos +5 % (+1 al arma); las Consecuencias gravitatorias que
        # reparte (velocidad y enfriamientos), fuera.
        carta("H1749", "Campo gravitatorio", "all_enemies",
              "Generas un campo gravitatorio a tu alrededor: 3d6 de daño de agua a cada enemigo, que te ataca a ti 1 "
              "turno, y tus golpes con arma hacen +1 durante 2 turnos.",
              [("3d6 de daño de agua a cada enemigo.", dano(700, 900)), _atrae(),
               ("Campo gravitatorio: +1 a tus golpes con arma durante 2 turnos.", propio(10, "bono_dano_arma:1"))],
              elemento="Agua"),
        # Hipergravedad (180 s): ataque mágico +(N x2), que no llega a +200 (§5) hasta el nivel 100: +1. El elemento
        # del hada y el maná de más (los efectos secundarios), fuera.
        carta("H1750", "Hipergravedad", "self",
              "Amplías la esfera gravitatoria y tu poder de ataque crece: tus golpes con arma hacen +1 hasta el final "
              "del combate.",
              [("Hipergravedad: +1 a tus golpes con arma todo el combate.", estado(180, "bono_dano_arma:1"))]),
    ],
    "creador": [
        # Ventana de defensa de 2 s (§4): devuelve el daño al defender (refleja); que no te pongan efectos negativos ya
        # lo cubre el contrarrestar. El Campo antigravitatorio reactivo del Equilibrio (devuelve el 40 %), fuera.
        carta("H1751", "Reflexión por antigravedad", "self",
              "Un campo antigravitatorio contrarresta los ataques de ese turno y, durante 2 turnos, devuelve los "
              "golpes a quien te ataca.",
              [CONTRARRESTA,
               ("Reflexión por antigravedad: reflejas los golpes físicos 2 turnos.", "propio:2:refleja:fisico")],
              accion="reaccion"),
        # Ingravidez (tuya: alcance y velocidad), fuera. Torpeza (7 s): ataques a distancia y mágicos -20 %
        # (desventaja); el alcance, la velocidad y los enfriamientos, fuera.
        carta("H1752", "Ingravidez", "all_enemies",
              "Creas un área de ingravidez a tu alrededor: 3d6 de daño de agua a cada enemigo, y la torpeza les hace "
              "atacar con desventaja 2 turnos.",
              [("3d6 de daño de agua a cada enemigo.", dano(800, 1000)),
               ("Torpeza: atacan con desventaja 2 turnos.", estado(7, "desventaja:ataque"))], elemento="Agua"),
        # Arrastrarlos a un punto: sin posiciones. Consecuencias de una gravedad fuerte (100 %, 6 s): pierden vida cada
        # 2 s (dano_por_turno, como la Descarga eléctrica del Ángel vengador); velocidad y enfriamientos, fuera.
        carta("H1753", "Agujero negro", "all_enemies",
              "Generas un agujero negro que lo absorbe todo: 4d6 de daño de agua a cada enemigo, y la gravedad fuerte "
              "les quita 1 de vida por turno durante 1 turno.",
              [("4d6 de daño de agua a cada enemigo.", dano(1100, 1500)),
               ("Consecuencias de una gravedad fuerte: 1 de daño por turno durante 1 turno.",
                estado(6, "dano_por_turno:1")),
               _pendiente("que sume un 20 % de daño por cada punto de gravedad y antigravedad, y los gaste todos")],
              elemento="Agua"),
    ],
    "arma_inicial": "baston_ferrado_de_viaje",
    "set": [
        pieza("Esfera de Grávitas", "MainHand", "media", {"INT": 1, "CON": 1},
              "La esfera de la SP de Grávitas: piedra oscura que flota sobre la mano.",
              restricciones=["Precisión: INT"]),
        pieza("Túnica de Grávitas", "Torso", "media", {"INT": 1},
              "Túnica azul noche con las esferas de piedra que orbitan sobre los hombros."),
        pieza("Diadema de Grávitas", "Head", "media", {"INT": 1, "CON": 1},
              "Aro oscuro con la piedra que flota encima."),
        pieza("Calzas de Grávitas", "Legs", "media", {"INT": 1, "CON": 1},
              "Calzas azul noche de Grávitas."),
        pieza("Botas de Grávitas", "Feet", "media", {"INT": 1, "CON": 1},
              "Botas oscuras de Grávitas."),
    ],
}

TORMENTA_DE_FUEGO = {
    "id": "tormenta_de_fuego", "name": "Tormenta de fuego", "sp": 11, "elemento": "Fuego",
    "nostale": "Tormenta de fuego (Fire Storm)",
    "description": ("Especialista del lanzallamas alquímico de fuego: abrasa de lejos, empapa a los enemigos en "
                    "aceite, quema en círculo con su magia y remata con una bola de fuego gigante."),
    "primaryStat": "DES", "secondaryStat": "INT", "role": "balanced",
    # §7: el básico y los disparos son ataque a distancia (DES, lo de siempre); Círculo de fuego y Remolino llameante,
    # ataque mágico: tiran INT.
    "pasiva": ("Infierno llameante",
               "Las llamas del lanzallamas de la Tormenta de fuego: tus golpes con arma hacen +1 de daño (fuego).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("la barra de marcas (la llenan sus técnicas y las marcas las gastan el Haz de llamas, la "
                           "Explosión infernal y las versiones potentes de aceite, erupción, barrera y reignición)")]),
    "cartas": [
        carta("H1859", "Infierno llameante reforzado", "zone_radius",
              "Refuerzas la potencia del lanzallamas: 2d6 de daño de fuego.",
              # el crítico +20 % de la técnica no entra
              [("2d6 de daño de fuego.", dano(400, 500))], elemento="Fuego"),
        carta("H1860", "Círculo de fuego", "all_enemies",
              "Lanzas llamas en círculo a tu alrededor: 3d6 de daño de fuego a cada enemigo, que queda petrificado 1 "
              "turno (no ataca).",
              [("3d6 de daño de fuego a cada enemigo.", dano(1000, 700)),
               ("Petrificación: 1 turno sin atacar.", estado(5, "incapacitado"))], elemento="Fuego", tira="INT"),
        # Aceite de napalm (100 %, 20 s): «varios ataques» -(N x1) (desventaja); la velocidad y la resistencia al
        # fuego, fuera. El Napalm ardiente de gastar 1 marca hace lo mismo más fuerte: fuera. «Efecto negativo», no
        # mágico: sin salvación.
        carta("H1861", "Lanzamiento de vial de aceite", "zone_radius",
              "Lanzas un vial de aceite alquímico al enemigo: empapado de napalm, ataca con desventaja 3 turnos.",
              [("Aceite de napalm: ataca con desventaja 3 turnos.", estado(20, "desventaja:ataque"))]),
        # el remolino hace su efecto al jugar la carta (sin posiciones). Combustión potente (100 %): la resistencia
        # al fuego y +20 % al daño de fuego solo un 10 % de las veces: nada que entre
        carta("H1862", "Remolino llameante", "all_enemies",
              "El lanzallamas levanta un remolino llameante: 3d6 de daño de fuego a cada enemigo, que queda "
              "petrificado 1 turno (no ataca).",
              [("3d6 de daño de fuego a cada enemigo.", dano(1300, 1000)),
               ("Petrificación: 1 turno sin atacar.", estado(5, "incapacitado"))], elemento="Fuego", tira="INT"),
        # El modo que refuerza las técnicas con las marcas (Modo de potencia de fuego, 10 s), como la Condensación de
        # gas del Cazanieblas.
        carta("H1863", "Refuerzo de fuego", "self",
              "Una bola de fuego mágica refuerza el lanzallamas: tus golpes con arma hacen +1 durante 2 turnos.",
              [("+1 a tus golpes con arma durante 2 turnos.", estado(10, "bono_dano_arma:1")),
               _pendiente("las marcas que refuerza")]),
        # Erupción de llamas (15 s): es tuya, pero quema cada 1,5 s a los enemigos a 5 casillas; sin posiciones, la
        # zona hace su efecto al jugar la carta, sobre todos los enemigos. La velocidad y la Erupción potente de gastar
        # 1 marca, fuera.
        carta("H1864", "Sobrecarga de combustible", "all_enemies",
              "Sobrecargas el depósito de combustible y el lanzallamas escupe llamas sin parar: los enemigos pierden 1 "
              "de vida por turno durante 2 turnos.",
              [("Erupción de llamas: 1 de daño por turno durante 2 turnos.", estado(15, "dano_por_turno:1"))]),
        # Barrera de llamas (180 s), el escudo de maná: daño recibido -15 % (+1 a la CA) y el acierto +(N x2)
        # (ventaja), las dos que más pesan; el 30 % que pasa a los MP y el softcrit -5 %, fuera. La Barrera potente de
        # gastar 1 marca, fuera.
        carta("H1865", "Barrera de llamas", "self",
              "Un escudo de llamas te defiende: hasta el final del combate tienes +1 a la CA y atacas con ventaja.",
              [("Barrera de llamas: +1 a la CA todo el combate.", propio(180, "bono_ca:1")),
               ("Barrera de llamas: ventaja en ataque todo el combate.", propio(180, "ventaja:ataque"))]),
    ],
    # Creador: las de profesión 12, 16 y 20.
    "creador": [
        # «Solo si tienes 1 marca» (y el +25 % de gastarla): sin marcas, se juega siempre. Combustión potente: nada que
        # entre.
        carta("H1866", "Haz de llamas", "zone_radius",
              "Refuerzas el lanzallamas para que el haz llegue a más enemigos: 3d6 de daño de fuego.",
              [("3d6 de daño de fuego.", dano(1000, 1200))], elemento="Fuego"),
        # Reignición (120 s): todos los ataques +5 % (+1) y el crítico +20 % (ventaja); las marcas cada 2 s y el
        # enfriamiento que se restablece, fuera. La Reignición potente de gastar 1 marca, fuera.
        carta("H1867", "Reignición", "self",
              "Un hechizo evita que el lanzallamas se sobrecargue: hasta el final del combate tus golpes con arma "
              "hacen +1 y atacas con ventaja.",
              [("Reignición: +1 a tus golpes con arma todo el combate.", estado(120, "bono_dano_arma:1")),
               ("Reignición: ventaja en ataque todo el combate.", estado(120, "ventaja:ataque"))]),
        # «Solo con 2 marcas» (y el +50 % de gastarlas): sin marcas, se juega siempre. Shock llameante (100 %): el
        # ataque en PvP y +20 % al daño de fuego un 20 % de las veces; no deja sin atacar: nada que entre.
        carta("H1868", "Explosión infernal", "all_enemies",
              "Creas una bola de fuego gigante y la lanzas contra los enemigos: 4d6 de daño de fuego a cada uno.",
              [("4d6 de daño de fuego a cada enemigo.", dano(1300, 1500))], elemento="Fuego"),
    ],
    # El catálogo no tiene lanzallamas de tier 1: uno normal, como los de Viaje (quema aceite alquímico).
    "arma_inicial": "lanzallamas_de_viaje",
    "armas_nuevas": [
        arma_comun("Lanzallamas de Viaje", "media", "Un lanzallamas alquímico con su depósito de aceite, a dos manos.",
                   restricciones=["Precisión: DES", DOS_MANOS]),
    ],
    "set": [
        pieza("Lanzallamas de la Tormenta de fuego", "MainHand", "media", {"DES": 1, "INT": 1},
              "El lanzallamas de la SP de la Tormenta de fuego, con el depósito de aceite alquímico, a dos manos.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Chaqueta de la Tormenta de fuego", "Torso", "media", {"DES": 1},
              "Chaqueta ignífuga negra y naranja con los tubos del depósito."),
        pieza("Casco de la Tormenta de fuego", "Head", "media", {"DES": 1, "INT": 1},
              "Casco de bombero de latón con la cresta."),
        pieza("Pantalones de la Tormenta de fuego", "Legs", "media", {"DES": 1, "INT": 1},
              "Pantalones ignífugos negros de la Tormenta de fuego."),
        pieza("Botas de la Tormenta de fuego", "Feet", "media", {"DES": 1, "INT": 1},
              "Botas negras de bombero de la Tormenta de fuego."),
    ],
}

MERLIN = {
    "id": "merlin", "name": "Merlín", "sp": 12, "elemento": "Luz", "nostale": "Merlín (Merlin)",
    "description": ("Especialista de la pistola mágica de luz: dispara rayos y cadenas de luz, prevé los ataques del "
                    "enemigo, protege al grupo con la magia de las flores y remata con Excalibur."),
    "primaryStat": "DES", "secondaryStat": "INT", "role": "support",
    "pasiva": ("Rayo de luz",
               "Los rayos de luz de la pistola de Merlín: tus golpes con arma hacen +1 de daño (luz).",
               [("Tus golpes con arma hacen +1 de daño.", siempre("bono_dano_arma:1")),
                _pendiente("la barra de marcas que llenan sus técnicas y la sincronización dimensional (con el héroe "
                           "que invoca)")]),
    # Sin niveles de profesión en la tarjeta: Creador = las tres últimas (§3). Los críticos propios de cada golpe
    # (del +20 al +35 %), fuera.
    "cartas": [
        # Deslumbramiento sagrado (35 %): fuera
        carta("H1976", "Destello luminoso", "all_enemies",
              "Una luz estalla justo delante de los ojos de los enemigos: 2d6 de daño de luz a cada uno.",
              [("2d6 de daño de luz a cada enemigo.", dano(350, 600))], elemento="Luz"),
        # te teletransporta 6 casillas: sin posiciones
        carta("H1977", "Velocidad de la luz", "all_enemies",
              "Te teletransportas a la velocidad del rayo y desatas una explosión de luz: 3d6 de daño de luz a cada "
              "enemigo.",
              [("3d6 de daño de luz a cada enemigo.", dano(800, 1000))], elemento="Luz"),
        # Luz del alivio (100 %, 10 s): daño recibido +10 % (bono_dano_contra); la velocidad, fuera. El círculo de
        # luz (tuyo: el aturdimiento que te pongan dura 2 s menos, y la velocidad): nada que entre.
        carta("H1978", "Rayo de luz salvador", "all_enemies",
              "Un círculo de luz sagrada debilita a los enemigos: 3d6 de daño de luz a cada uno, y tus golpes les "
              "hacen +1 durante 2 turnos.",
              [("3d6 de daño de luz a cada enemigo.", dano(1000, 1300)),
               ("Luz del alivio: tus golpes les hacen +1 durante 2 turnos.", estado(10, "bono_dano_contra:1"))],
              elemento="Luz"),
        # Ventana de defensa de 2 s (§4): esquivas un ataque y el próximo es certero (ventaja). El indicador que sube
        # al recibir daño (la sincronización), fuera.
        carta("H1979", "Adivinación", "self",
              "Prevés los movimientos del enemigo: contrarrestas los ataques de ese turno y, con lo que has visto, "
              "atacas con ventaja 2 turnos.",
              [CONTRARRESTA, ("Adivinación: atacas con ventaja 2 turnos.", "propio:2:ventaja:ataque")],
              accion="reaccion"),
        # Futuro manipulado (100 %, 10 s): acumula el 10 % del daño que recibe y lo pierde al acabar (daño recibido +:
        # bono_dano_contra) y defensa -1 (penal_ca:1); que no se muevan: sin posiciones.
        carta("H1980", "Un sabio paso adelante", "all_enemies",
              "La magia de la previsión se adelanta a los enemigos: 3d6 de daño de luz a cada uno; con el futuro "
              "manipulado, tus golpes les hacen +1 y tienen -1 a la CA durante 2 turnos.",
              [("3d6 de daño de luz a cada enemigo.", dano(800, 650)),
               ("Futuro manipulado: tus golpes les hacen +1 durante 2 turnos.", estado(10, "bono_dano_contra:1")),
               ("Futuro manipulado: -1 a la CA durante 2 turnos.", estado(10, "penal_ca:1"))], elemento="Luz"),
        # Cadena luminosa (100 %, 5 s): ataque no posible y sin defensa; que no se muevan: sin posiciones.
        carta("H1981", "Cadena luminosa", "all_enemies",
              "Unos rayos de luz hipnóticos atan al objetivo y a los cercanos: 3d6 de daño de luz a cada enemigo, que "
              "queda 1 turno sin atacar y se defiende con desventaja.",
              [("3d6 de daño de luz a cada enemigo.", dano(1000, 1300)),
               ("Cadena luminosa: 1 turno sin atacar.", estado(5, "incapacitado")),
               ("Cadena luminosa: se defienden con desventaja 1 turno.", estado(5, "desventaja:defensa"))],
              elemento="Luz"),
        # Cúpula floral (300 s), el escudo de maná: daño recibido -15 % (+1 a la CA); el 25 % que pasa a los MP, la
        # velocidad y las defensas con la sincronización, fuera.
        carta("H1982", "Cúpula floral", "self",
              "Un escudo de flores sagradas repele los ataques: +1 a la CA hasta el final del combate.",
              [("Cúpula floral: +1 a la CA todo el combate.", propio(300, "bono_ca:1"))], elemento="Naturaleza"),
    ],
    "creador": [
        # reunirlos en un mismo lugar: sin posiciones
        carta("H1983", "Luz unificadora", "all_enemies",
              "Una luz sagrada reúne a los enemigos cercanos en un mismo lugar: 4d6 de daño de luz a cada uno, que te "
              "ataca a ti 1 turno.",
              [("4d6 de daño de luz a cada enemigo.", dano(1100, 1500)), _atrae()], elemento="Luz"),
        # Energía floral cálida (30 s): cura el 25 % (2d6) y defensa +5 % (+1 a la CA); el MP, el hada y el
        # enfriamiento, fuera. Energía floral sagrada (300 s): el acierto +(N x2) (ventaja); el héroe invocado y lo
        # de la sincronización, fuera.
        carta("H1984", "Don de la naturaleza", "all_allies",
              "Las energías florales llenan al grupo: os curáis 2d6, tenéis +1 a la CA durante 3 turnos y atacáis con "
              "ventaja hasta el final del combate.",
              [("Energía floral cálida: os curáis 2d6.", "cura:2d6"),
               ("Energía floral cálida: +1 a la CA durante 3 turnos.", estado(30, "bono_ca:1")),
               ("Energía floral sagrada: ventaja en ataque todo el combate.", estado(300, "ventaja:ataque"))], elemento="Naturaleza"),
        # Herida sagrada (100 %, 10 s): pierden vida cada 2 s (una herida: sangran) y todos sus ataques -20 %
        # (desventaja); la resistencia a la luz y la velocidad, fuera. Energía de Excálibur (tuya, 10 s): cura el 15 %
        # (1d6) y todos los ataques +5 % (+1); el MP y la resistencia al fuego del rival, fuera.
        carta("H1985", "¡Aparece, Excalibur!", "all_enemies",
              "Excalibur aparece y golpea la zona: 4d6 de daño de luz a cada enemigo; la herida sagrada los hace "
              "sangrar y atacar con desventaja 2 turnos, y la energía de Excálibur te cura 1d6 y da +1 a tus golpes "
              "con arma durante 2 turnos.",
              [("4d6 de daño de luz a cada enemigo.", dano(1700, 2200)),
               ("Herida sagrada: sangran 2 turnos.", condicion("sangrado", 10)),
               ("Herida sagrada: atacan con desventaja 2 turnos.", estado(10, "desventaja:ataque")),
               ("Energía de Excálibur: te curas 1d6.", "cura:1d6"),
               ("Energía de Excálibur: +1 a tus golpes con arma durante 2 turnos.", propio(10, "bono_dano_arma:1")),
               _pendiente("que cada nivel de sincronización dimensional sume un 10 % al daño")], elemento="Luz"),
    ],
    # La misma pistola normal que el Mago de las tinieblas (SP4).
    "arma_inicial": "pistola_magica_de_viaje",
    "armas_nuevas": [
        PISTOLA_MAGICA,
    ],
    "set": [
        pieza("Pistola de Luz de Merlín", "MainHand", "ligera", {"DES": 1, "INT": 1},
              "La pistola mágica de la SP de Merlín, blanca y oro, que dispara balas de maná de luz.",
              restricciones=["Precisión: DES"]),
        pieza("Manto de Merlín", "Torso", "media", {"DES": 1},
              "Manto blanco y azul cielo con runas de oro y el bajo de flores."),
        pieza("Corona de flores de Merlín", "Head", "ligera", {"DES": 1, "INT": 1},
              "Corona de flores blancas y hojas de oro."),
        pieza("Calzas de Merlín", "Legs", "ligera", {"DES": 1, "INT": 1},
              "Calzas blancas de Merlín."),
        pieza("Botas de Merlín", "Feet", "ligera", {"DES": 1, "INT": 1},
              "Botas blancas de Merlín."),
    ],
}


MAGO = [MAGO_DE_FUEGO, MAGO_SAGRADO, MAGO_AZUL, MAGO_DE_LAS_TINIEBLAS, VOLCANO, SENOR_DE_LAS_MAREAS, VIDENTE, ARCHIMAGO,
        SACERDOTE_VUDU, GRAVITAS, TORMENTA_DE_FUEGO, MERLIN]
