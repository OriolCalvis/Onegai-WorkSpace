"""CP-01 — Dos clérigos y dos paladines, clases propias de MotorGraphico (los datos).

Oriol (07/10/2026): «me gustaría crear 2 clérigos, 2 paladines». Cada uno sirve a un dios del panteón
(assets/catalogs/deities.json), que da el tono con su dominio, su favor y su obligación: el Clérigo de la
Espiga Dorada (cosecha, abundancia y paciencia), el Clérigo del Caminante Gris (caminos, fronteras y
despedidas), el Paladín del Juramento (el Padre de la Forja: forja, artesanía y juramentos) y el Caballero
del Rey Lobo (invierno, muerte y manada). Los nombres de los hechizos de los dioses (grantedSpells) no se
copian: las ideas que coinciden con uno llevan otro nombre (Pan del Camino -> Hogaza compartida, Barbecho
Forzoso -> Tierra en barbecho, Sentencia del Alfa -> Mirada del alfa, El Último Invierno -> El gran
invierno...).

No salen de una SP: los dados y las rondas se escriben tal cual, dentro del presupuesto de
planificacion/CANON_CLASES_PROPIAS.md (§3), con las piezas de cp_reglas.py. Lo escribe en los catálogos
tools/importar_clases_sp.py. Cuando una carta se aparta de lo que proponía el encargo, lo dice
un comentario.

Criterios comunes a las cuatro:
- Tiran con CAR (la fe, §4). Salvación solo en el control sin daño que es magia: defensa mental el miedo, el
  destierro, el barbecho, el reto y el aullido; resistencia física los grilletes al rojo (el hierro, del
  cuerpo). Lo que hace daño es un golpe y su estado entra con la tirada.
- Cada clase tiene su mejora de grupo, y ninguna repite la de otra: la Espiga, ventaja al defenderse; el
  Caminante, +1 a la CA; el Juramento, +1 a los golpes con arma; el Rey Lobo, ventaja al atacar.
- Las reacciones hacen solo lo que el combate hace al saltar (CartasTiradas.cpp): bloquear el golpe dirigido
  a ti (con lo que ganas, 2 turnos, o una condición al atacante), ponerte delante de un aliado, sumar dados a
  su próxima tirada o levantar al que cae. Cada clase tiene la suya: la Espiga bloquea y cura más, el
  Caminante suma dados, el Juramento bloquea y se templa (y se pone delante), el Rey Lobo bloquea y asusta.
- Las cartas cuyo efecto es solo debilitar la defensa del enemigo (la Despedida, el Aullido, los Grilletes, el
  Barbecho) lo dejan 2 turnos: un estado se acaba al cerrar la ronda y con 1 no llegaría a tu turno siguiente.
- Ninguna carta lleva pendiente: todas se juegan enteras.

El nombre del set: «Set de la Espiga Dorada» (Espiga Dorada está en sp_reglas.NOMBRES_FEMENINOS), «Set del
Caminante Gris», «Set del Juramento» y «Set del Rey Lobo».
"""
from cp_reglas import (DOS_MANOS, arma_comun, atrae, aturdido, carta, cond, cura, dano, elige_una, estado, pieza,
                       propio, si, siempre)

# El arma normal de tier 1 de la Espiga (el catálogo no tenía hoz; solo guadañas a dos manos).
HOZ = arma_comun("Hoz de Viaje", "ligera", "Una hoz de segador con el mango de madera, afilada para el camino.",
                 restricciones=["Precisión: CAR"])


# --- B1 ---------------------------------------------------------------------------------------------------------

CLERIGO_ESPIGA = {
    "id": "clerigo_de_la_espiga_dorada", "name": "Clérigo de la Espiga Dorada", "familia": "clerigo",
    "elemento": "Luz", "deidad": "la_espiga_dorada",
    "referencias": ["NosTale: Mago sagrado (las curas de grupo y limpiar)",
                    "NosTale: Monje de lucha (Palabras de Buda: limpiar y curar a todos)"],
    "description": ("Sacerdote de los campos de la Espiga Dorada: reparte el pan bendito que cura, levanta gavillas "
                    "que protegen, siega con su hoz de luz y deja en barbecho al enemigo."),
    "primaryStat": "CAR", "secondaryStat": "CON", "role": "support",
    "tira": "CAR",
    # «Da más quien espera mejor» (la descripción de la diosa): las curas de la clase, +1 (§5, siempre).
    "pasiva": ("Paciencia del sembrador",
               "Da más quien espera mejor: tus curas curan +1.",
               [("Tus curas curan +1.", siempre("bono_cura:1"))]),
    # La mies compartida del encargo (limpiar y algo pequeño a todos) no entra: lo pequeño sería otra cura de grupo,
    # como la Hogaza, y limpiar a todos ya lo hace el Año de abundancia. En su sitio, el Grano de luz: lo que siembra
    # crece (marca al enemigo y la siega le hace más).
    "cartas": [
        carta("propia", "Siega dorada", "zone_radius",
              "Tu hoz dorada siega al enemigo como mies madura: 3d6 de daño de luz.",
              [("3d6 de daño de luz.", dano("3d6"))], elemento="Luz"),
        carta("propia", "Grano de luz", "single_enemy",
              "Siembras un grano de luz en la herida: 2d6 de daño de luz, y queda marcado 2 turnos (tus golpes le "
              "hacen +1). Lo que se siembra, se cosecha.",
              [("2d6 de daño de luz.", dano("2d6")),
               ("Marcado 2 turnos: tus golpes le hacen +1.", cond("marcado", 2))], elemento="Luz"),
        carta("propia", "Hogaza compartida", "all_allies",
              "Partes una hogaza bendita entre todos: tú y tus aliados os curáis 1d6.",
              [("Os curáis 1d6.", cura("1d6"))]),
        # «Un aliado +2 a la CA 3 turnos, o el grupo +1»: el +1 a la CA de grupo es la Raya gris del Caminante; las
        # gavillas cubren y el grupo se defiende con ventaja.
        carta("propia", "Gavillas en pie", "all_allies",
              "Levantas un muro de gavillas doradas delante del grupo: tú y tus aliados os defendéis con ventaja "
              "2 turnos.",
              [("Gavillas: os defendéis con ventaja 2 turnos.", estado(2, "ventaja:defensa"))]),
        # una maldición de la tierra: defensa mental
        carta("propia", "Tierra en barbecho", "single_enemy",
              "Dejas al enemigo en barbecho. Si falla la salvación (defensa mental), lo hace todo con desventaja "
              "2 turnos (atacar y defenderse).",
              [("Barbecho: lo hace todo con desventaja 2 turnos.", estado(2, "desventaja:todo"))],
              salvacion="defensa_mental"),
        carta("folclore: el espantapájaros", "Espantapájaros", "all_enemies",
              "Clavas en el campo un espantapájaros de paja dorada. Los enemigos que fallen la salvación (defensa "
              "mental) quedan asustados 1 turno (atacan con desventaja).",
              [("Asustados 1 turno.", cond("asustado", 1))], salvacion="defensa_mental"),
        # la reacción de la paciencia: lo que gana al bloquear (2 rondas, §4 del canon de las SP) es curar más
        carta("propia", "Semilla de paciencia", "self",
              "Esperas el próximo golpe y lo guardas como semilla: lo bloqueas, y durante 2 turnos tus curas "
              "curan +1.",
              [("Bloqueas el golpe.", "bloquea:ataque"),
               ("Paciencia: tus curas curan +1 durante 2 turnos.", propio(2, "bono_cura:1"))],
              accion="reaccion"),
    ],
    "creador": [
        # «El Año de las Dos Cosechas» es un hechizo de la diosa: otro nombre. Lo grande (§3): limpiar y 2d6 a todos.
        carta("NosTale: Monje de lucha, Palabras de Buda", "Año de abundancia", "all_allies",
              "La Espiga Dorada da dos cosechas en un año: tú y tus aliados os libráis de los estados en contra y "
              "os curáis 2d6.",
              [("Os libráis de los estados en contra.", "limpia"), ("Os curáis 2d6.", cura("2d6"))]),
        # 3d6 y un efecto (el encargo decía 4d6 a secas, que es El último viaje del Caminante)
        carta("propia", "Cosecha final", "all_enemies",
              "Tu hoz traza un círculo de oro: 3d6 de daño de luz a cada enemigo, y caen como la mies segada: "
              "derribados 1 turno (atacan con desventaja y los golpes cuerpo a cuerpo contra ellos tienen ventaja).",
              [("3d6 de daño de luz a cada enemigo.", dano("3d6")),
               ("Derribados 1 turno.", cond("caido", 1))], elemento="Luz"),
        carta("propia", "El grano que renace", "single_ally",
              "Preparas la promesa del grano, que muere en la tierra y vuelve a brotar: si el aliado que eliges "
              "cae, se levanta con 1 PV.",
              [("Si cae, vuelve con 1 PV.", "revive:1")], accion="reaccion"),
    ],
    # pega (la siega), su mecánica (el pan, que la pasiva cura más) y defensa (las gavillas)
    "arranque": ["Siega dorada", "Hogaza compartida", "Gavillas en pie"],
    "arma_inicial": "hoz_de_viaje",
    "armas_nuevas": [HOZ],
    "set_de": "Espiga Dorada",
    "set": [
        pieza("Hoz de la Espiga Dorada", "MainHand", "ligera", {"CAR": 1, "CON": 1},
              "La hoz de la Espiga Dorada: hoja curva de oro y mango de madera clara con una espiga grabada.",
              restricciones=["Precisión: CAR"]),
        pieza("Túnica de la Espiga Dorada", "Torso", "media", {"CAR": 1},
              "Túnica ocre hasta la rodilla con la estola verde, una espiga dorada bordada en el pecho, el bajo "
              "dorado y un cinturón de cuerda."),
        pieza("Sombrero de la Espiga Dorada", "Head", "media", {"CAR": 1, "CON": 1},
              "Sombrero de paja de ala ancha con espigas doradas en la cinta."),
        pieza("Pantalón de la Espiga Dorada", "Legs", "media", {"CAR": 1, "CON": 1},
              "Pantalón pardo de faena."),
        pieza("Sandalias de la Espiga Dorada", "Feet", "media", {"CAR": 1, "CON": 1},
              "Sandalias de cuero."),
    ],
}


# --- B2 ---------------------------------------------------------------------------------------------------------

CLERIGO_CAMINANTE = {
    "id": "clerigo_del_caminante_gris", "name": "Clérigo del Caminante Gris", "familia": "clerigo",
    "elemento": "Luz", "deidad": "el_caminante_gris",
    "referencias": ["NosTale: Ángel vengador (los golpes de luz)",
                    "NosTale: Mago sagrado (curar y limpiar a un aliado)",
                    "folclore: el peregrino del Camino de Santiago (bordón, concha, calabaza y fuentes)"],
    "description": ("Sacerdote de los caminos: cuida de los que viajan con él, traza rayas que el enemigo no cruza, "
                    "destierra un momento a quien estorba y da la última despedida a los que caen."),
    "primaryStat": "CAR", "secondaryStat": "INT", "role": "support",
    "tira": "CAR",
    # El favor del dios: «nadie puede emboscar a tu grupo» (§5, siempre).
    "pasiva": ("Caminante prevenido",
               "Nadie embosca al caminante: te defiendes con ventaja.",
               [("Te defiendes con ventaja.", siempre("ventaja:defensa"))]),
    "cartas": [
        # el bordón es un golpe físico: sin elemento (cp_reglas: lo físico, sin elemento)
        carta("propia", "Barrido de bordón", "single_enemy",
              "Le barres las piernas con el bordón: 2d6 de daño, y cae derribado 1 turno (ataca con desventaja y "
              "los golpes cuerpo a cuerpo contra él tienen ventaja).",
              [("2d6 de daño.", dano("2d6")), ("Derribado 1 turno.", cond("caido", 1))]),
        # El paso firme del encargo (el grupo se defiende con ventaja) sería la pasiva otra vez para ti; la frontera
        # que protege al grupo es esta raya: +1 a la CA.
        carta("propia", "La raya gris", "all_allies",
              "Trazas con el bordón una raya gris delante del grupo y los golpes que la cruzan llegan cansados: tú y "
              "tus aliados tenéis +1 a la CA 3 turnos.",
              [("La raya gris: +1 a la CA 3 turnos.", estado(3, "bono_ca:1"))]),
        carta("propia", "Destierro gris", "single_enemy",
              "Mandas al enemigo a un camino gris que no lleva a ninguna parte. Si falla la salvación (defensa "
              "mental), se queda 1 turno sin atacar.",
              [("Desterrado: 1 turno sin atacar.", estado(1, "incapacitado"))], salvacion="defensa_mental"),
        # El descanso del mojón del encargo (limpiar y 2d6 a uno) sería una cura y media: limpia y 1d6, como la
        # Recuperación espiritual del Ninja. Limpia primero (con miedo, la cura saldría a medias).
        carta("folclore: las fuentes del Camino de Santiago", "Fuente del cruce", "single_ally",
              "Das de beber a un aliado de la fuente del cruce de caminos: se libra de los estados en contra y se "
              "cura 1d6.",
              [("Se libra de los estados en contra.", "limpia"), ("Se cura 1d6.", cura("1d6"))]),
        # El desvío providencial del encargo era ponerse delante (defiende_aliado), que es del Paladín del
        # Juramento: aquí el camino se tuerce a favor del aliado, +2 dados a su próxima tirada (atacar o defenderse).
        carta("propia", "Recodo del camino", "single_ally",
              "El camino da un recodo a favor de un aliado: su próxima tirada, para atacar o para defenderse, lleva "
              "2 dados más.",
              [("+2 dados a su próxima tirada.", "dados_extra:2")], accion="reaccion"),
        # 2 turnos: con 1, el crítico se acabaría al cerrar la ronda y tu golpe siguiente no lo vería
        carta("propia", "La última despedida", "single_enemy",
              "Le das la despedida del Caminante Gris: 2d6 de daño de luz, y su viaje queda escrito: los golpes "
              "contra él son críticos 2 turnos.",
              [("2d6 de daño de luz.", dano("2d6")),
               ("Despedida: los golpes contra él son críticos 2 turnos.", estado(2, "critico_contra"))],
              elemento="Luz"),
        carta("propia", "Responso", "all_enemies",
              "Rezas el responso por los que van a caer y la luz baja sobre ellos: 2d6 de daño de luz a cada enemigo.",
              [("2d6 de daño de luz a cada enemigo.", dano("2d6"))], elemento="Luz"),
    ],
    "creador": [
        # turno_extra (§3, lo grande): si apunta a ti, juegas otra vez; si es otro aliado, ataca ya
        carta("propia", "Atajo de niebla", "single_ally",
              "Abres un atajo entre la niebla gris para un aliado y llega antes: si eres tú, juegas otro turno; si "
              "es otro, ataca ahora.",
              [("Un turno más para el aliado.", "turno_extra")]),
        # La columna de marcha del encargo (el grupo +2 a la CA todo el combate) es La obra maestra del Paladín del
        # Juramento: el Caminante cierra la frontera. Sin atacar 2 turnos, solo en Creador (§3).
        carta("propia", "La frontera cerrada", "all_enemies",
              "Cierras la frontera del Caminante Gris. Los enemigos que fallen la salvación (defensa mental) no la "
              "pueden cruzar: 2 turnos sin atacar.",
              [("Frontera cerrada: 2 turnos sin atacar.", estado(2, "incapacitado"))], salvacion="defensa_mental"),
        carta("propia", "El último viaje", "all_enemies",
              "El Caminante Gris viene a buscarlos para el último viaje: 4d6 de daño de luz a cada enemigo.",
              [("4d6 de daño de luz a cada enemigo.", dano("4d6"))], elemento="Luz"),
    ],
    # pega (el barrido), su mecánica (la raya, la frontera que protege) y apoyo (la fuente)
    "arranque": ["Barrido de bordón", "La raya gris", "Fuente del cruce"],
    "arma_inicial": "baston_ferrado_de_viaje",
    "set_de": "Caminante Gris",
    "set": [
        pieza("Bordón del Caminante Gris", "MainHand", "media", {"CAR": 1, "INT": 1},
              "El bordón del Caminante Gris: vara alta de madera gris con el puño curvo y la contera de hierro.",
              restricciones=["Precisión: CAR"]),
        pieza("Esclavina del Caminante Gris", "Torso", "media", {"CAR": 1},
              "Esclavina gris con capucha sobre un hábito gris largo, con un cinturón de cuero y una calabaza."),
        pieza("Sombrero del Caminante Gris", "Head", "media", {"CAR": 1, "INT": 1},
              "Sombrero gris de peregrino de ala muy ancha, con una concha blanca."),
        pieza("Pantalón del Caminante Gris", "Legs", "media", {"CAR": 1, "INT": 1},
              "Pantalón gris de camino."),
        pieza("Botas del Caminante Gris", "Feet", "media", {"CAR": 1, "INT": 1},
              "Botas de camino marrones."),
    ],
}


# --- B3 ---------------------------------------------------------------------------------------------------------

PALADIN_JURAMENTO = {
    "id": "paladin_del_juramento", "name": "Paladín del Juramento", "familia": "paladin",
    "elemento": "Fuego", "deidad": "deity_forge_father",
    "referencias": ["NosTale: Guerrero (la Provocación)",
                    "NosTale: Caballero Draconiano (el escudo que para el golpe)",
                    "NosTale: Monje de lucha (la forma en la pasiva y el Sacrificio)"],
    "description": ("Caballero del Padre de la Forja que jura sobre el yunque: reta a los enemigos para que le "
                    "ataquen a él, aguanta como el hierro templado y deja caer el martillo entre chispas."),
    "primaryStat": "CAR", "secondaryStat": "CON", "role": "tank",
    "tira": "CAR",
    # El juramento que hace al empezar cada combate es la forma de la clase (§5, elegir), como las meditaciones del
    # Monje de lucha.
    "pasiva": ("Voto del yunque",
               "Al empezar el combate juras sobre el yunque. Proteger: +1 a la CA. Castigar: tus golpes con arma "
               "hacen +1 de daño (fuego).",
               [("Juras al empezar el combate.", elige_una("proteger", "castigar")),
                ("Proteger: +1 a la CA.", si("proteger", "bono_ca:1")),
                ("Castigar: tus golpes con arma hacen +1 de daño.", si("castigar", "bono_dano_arma:1"))]),
    "cartas": [
        carta("propia", "Martinete", "single_enemy",
              "Dejas caer el martillo como el martinete de la fragua: 2d6 de daño de fuego, y el enemigo se queda "
              "1 turno sin atacar.",
              [("2d6 de daño de fuego.", dano("2d6")), ("1 turno sin atacar.", estado(1, "incapacitado"))],
              elemento="Fuego"),
        # Como la Provocación del Guerrero (Oriol, 07/10: básica, a uno y con salvación); lo que gana, +1 a la CA, es
        # tuyo aunque se resista.
        carta("NosTale: Guerrero, Provocación", "Desafío jurado", "single_enemy",
              "Golpeas el yunque y retas a un enemigo por tu juramento. Si no se resiste (defensa mental), te ataca "
              "a ti 1 turno; y tú, firme, tienes +1 a la CA 2 turnos. Es un reto básico, fácil de esquivar: mejora "
              "al subir de tier.",
              [atrae(), ("Firme: +1 a tu CA 2 turnos.", propio(2, "bono_ca:1"))],
              salvacion="defensa_mental"),
        carta("NosTale: Caballero Draconiano, Escudo de Dragonio", "Firme como el yunque", "self",
              "Recibes el golpe como el yunque recibe el martillo: lo bloqueas, y el golpe te templa: +1 a la CA "
              "2 turnos.",
              [("Bloqueas el golpe.", "bloquea:ataque"),
               ("Templado: +1 a tu CA 2 turnos.", propio(2, "bono_ca:1"))],
              accion="reaccion"),
        carta("propia", "Acero templado", "all_allies",
              "Templas las armas del grupo en el agua de la fragua: tú y tus aliados hacéis +1 con los golpes de arma "
              "3 turnos.",
              [("Acero templado: +1 a los golpes con arma 3 turnos.", estado(3, "bono_dano_arma:1"))]),
        carta("WoW: Imposición de manos", "Imposición de manos", "single_ally",
              "Pones las manos, calientes como la fragua, sobre un aliado: se cura 2d6.",
              [("Se cura 2d6.", cura("2d6"))]),
        # control sin daño: el hierro al rojo es del cuerpo, resistencia física (§4)
        carta("propia", "Grilletes al rojo", "single_enemy",
              "Unos grilletes al rojo le sujetan los brazos. Si falla la salvación (resistencia física), se defiende "
              "con desventaja y con -1 a la CA 2 turnos.",
              [("Grilletes: se defiende con desventaja 2 turnos.", estado(2, "desventaja:defensa")),
               ("Grilletes: -1 a la CA 2 turnos.", estado(2, "penal_ca:1"))],
              salvacion="resistencia_fisica"),
        # ponerse delante de un aliado es solo del tanque (el Caminante suma dados y el Rey Lobo asusta)
        carta("NosTale: Monje de lucha, Sacrificio", "Bajo mi escudo", "single_ally",
              "Juras proteger a un aliado: la próxima vez que le ataquen, te pones delante y recibes tú el golpe.",
              [("Te pones delante del aliado.", "defiende_aliado:1")], accion="reaccion"),
    ],
    "creador": [
        carta("NosTale: Cruzado, Plegaria de defensa", "La obra maestra", "all_allies",
              "Tu mejor obra: templas las armaduras del grupo en la fragua sagrada. Tú y tus aliados tenéis +2 a la "
              "CA hasta el final del combate.",
              [("La obra maestra: +2 a la CA todo el combate.", estado("combate", "bono_ca:2"))]),
        carta("propia", "Juicio del martillo", "single_enemy",
              "El Padre de la Forja juzga con tu martillo: 4d6 de daño de fuego y aturdimiento 1 turno (no ataca y "
              "se defiende con desventaja).",
              [("4d6 de daño de fuego.", dano("4d6"))] + aturdido(1), elemento="Fuego"),
        # La promesa cumplida del encargo (revive) haría tres clases con revive (la Espiga y el Rey Lobo ya lo
        # tienen): el tanque reta a todos, 3d6 y un efecto (§3).
        carta("propia", "Clamor del yunque", "all_enemies",
              "Descargas el martillo sobre el yunque y llueven chispas: 3d6 de daño de fuego a cada enemigo, y el "
              "clamor los obliga a atacarte a ti 1 turno.",
              [("3d6 de daño de fuego a cada enemigo.", dano("3d6")),
               ("Te atacan a ti 1 turno.", estado(1, "provocado:origen"))], elemento="Fuego"),
    ],
    # pega (el martinete), su mecánica (el reto) y defensa (el yunque)
    "arranque": ["Martinete", "Desafío jurado", "Firme como el yunque"],
    "arma_inicial": "martillo_de_guerra_de_viaje",
    "set_de": "Juramento",
    "set": [
        pieza("Martillo del Juramento", "MainHand", "pesada", {"CAR": 1, "CON": 1},
              "El martillo del Juramento: cabeza de acero con el yunque grabado y el mango de roble con anillas de "
              "latón.",
              restricciones=["Precisión: CAR", DOS_MANOS]),
        pieza("Coraza del Juramento", "Torso", "pesada", {"CAR": 1},
              "Coraza de acero con remaches de latón, el martillo grabado en el pecho y un delantal de cuero de "
              "herrero hasta las rodillas."),
        pieza("Yelmo del Juramento", "Head", "pesada", {"CAR": 1, "CON": 1},
              "Yelmo de acero con la visera alzada (se ve la cara) y una cresta de latón en forma de yunque."),
        pieza("Quijotes del Juramento", "Legs", "pesada", {"CAR": 1, "CON": 1},
              "Quijotes de acero."),
        pieza("Escarpes del Juramento", "Feet", "pesada", {"CAR": 1, "CON": 1},
              "Escarpes de acero."),
    ],
}


# --- B4 ---------------------------------------------------------------------------------------------------------

CABALLERO_REY_LOBO = {
    "id": "caballero_del_rey_lobo", "name": "Caballero del Rey Lobo", "familia": "paladin",
    "elemento": "Hielo", "deidad": "deity_wolf_king",
    "referencias": ["NosTale: Guardabosques (el Espíritu de lobo)",
                    "NosTale: Cazasoles (el lobo que muerde y hace sangrar)",
                    "mitología nórdica: el Fimbulvetr, el gran invierno"],
    "description": ("Paladín del Rey Lobo: nunca deja atrás a nadie de su manada, aúlla para helar la sangre del "
                    "enemigo, muerde con el frío del invierno y huele la sangre de su presa."),
    "primaryStat": "CAR", "secondaryStat": "CON", "role": "balanced",
    "tira": "CAR",
    # siempre (§5): +1 a la CA. «Pelaje del Invierno» es un hechizo del dios: otro nombre.
    "pasiva": ("Pelaje de escarcha",
               "El invierno del Rey Lobo te cubre como un pelaje: +1 a la CA.",
               [("+1 a la CA.", siempre("bono_ca:1"))]),
    # Nunca solo (defiende_aliado) es del Paladín del Juramento: el Rey Lobo bloquea y asusta (Gruñido), y no
    # abandonar a nadie de la manada es Volver del invierno. Lamer las heridas (2d6 a un aliado) es la Imposición de
    # manos del Juramento: el lobo se cura mordiendo (Colmillo de escarcha).
    "cartas": [
        # la desventaja al atacar del encargo ya la ponen el barrido del Caminante y el espantapájaros: aquí roba vida
        carta("NosTale: Cosechador de almas, Aspirasangre", "Colmillo de escarcha", "single_enemy",
              "Muerdes con el filo helado del espadón y bebes del golpe: 2d6 de daño de hielo, y te curas el daño que "
              "haces.",
              [("2d6 de daño de hielo.", dano("2d6")), ("Te curas el daño que haces.", cura("dano_causado"))],
              elemento="Hielo"),
        # el miedo es defensa mental; se les hiela la sangre: se defienden peor (2 turnos, para que llegue a tu turno)
        carta("propia", "Aullido helado", "all_enemies",
              "Aúllas con la voz del Rey Lobo y se les hiela la sangre. Los enemigos que fallen la salvación "
              "(defensa mental) se defienden con desventaja 2 turnos.",
              [("Sangre helada: se defienden con desventaja 2 turnos.", estado(2, "desventaja:defensa"))],
              salvacion="defensa_mental", elemento="Hielo"),
        carta("propia", "Caza en manada", "all_allies",
              "La manada caza junta: tú y tus aliados atacáis con ventaja 2 turnos.",
              [("En manada: atacáis con ventaja 2 turnos.", estado(2, "ventaja:ataque"))]),
        # sin daño ni salvación (marcar no es miedo ni maldición): la marca entra con la tirada; el +1 es tuyo
        carta("propia", "Mirada del alfa", "single_enemy",
              "Eliges la presa con la mirada del alfa: queda marcada 3 turnos (tus golpes le hacen +1), y tú haces "
              "+1 con los golpes de arma 3 turnos.",
              [("Presa marcada 3 turnos.", cond("marcado", 3)),
               ("El alfa: +1 a tus golpes con arma 3 turnos.", propio(3, "bono_dano_arma:1"))]),
        carta("NosTale: Cazasoles, Orden de ataque fugaz", "La cacería", "zone_radius",
              "La jauría acosa a la presa y le muerde los flancos: 2d6 de daño de hielo, y sangra 2 turnos.",
              [("2d6 de daño de hielo.", dano("2d6")), ("Sangra 2 turnos.", cond("sangrado", 2))], elemento="Hielo"),
        # bloquea y deja una condición al atacante, 2 turnos (§3)
        carta("propia", "Gruñido", "self",
              "Enseñas los colmillos a quien te ataca: bloqueas el golpe, y queda asustado 2 turnos (ataca con "
              "desventaja).",
              [("Bloqueas el golpe.", "bloquea:ataque"), ("Asustado 2 turnos.", cond("asustado", 2))],
              accion="reaccion"),
        carta("propia", "Ventisca", "all_enemies",
              "Desatas la ventisca del Rey Lobo: 2d6 de daño de hielo a cada enemigo.",
              [("2d6 de daño de hielo a cada enemigo.", dano("2d6"))], elemento="Hielo"),
    ],
    "creador": [
        carta("mitología nórdica: el Fimbulvetr", "El gran invierno", "all_enemies",
              "Llega el invierno que no se acaba: 4d6 de daño de hielo a cada enemigo, y el frío los deja 1 turno "
              "sin atacar.",
              [("4d6 de daño de hielo a cada enemigo.", dano("4d6")),
               ("El frío: 1 turno sin atacar.", estado(1, "incapacitado"))], elemento="Hielo"),
        carta("NosTale: Guardabosques, Espíritu de lobo", "La manada entera", "all_allies",
              "Toda la manada corre contigo: tú y tus aliados hacéis +1d6 con los golpes de arma hasta el final del "
              "combate.",
              [("La manada: +1d6 a los golpes con arma todo el combate.", estado("combate", "bono_dano_arma:1d6"))]),
        # la obligación del dios: «nunca abandones a un miembro de tu manada en peligro de muerte»
        carta("propia", "Volver del invierno", "single_ally",
              "El Rey Lobo no deja atrás a nadie de su manada: si el aliado que eliges cae, vuelve del invierno con "
              "1 PV.",
              [("Si cae, vuelve con 1 PV.", "revive:1")], accion="reaccion", elemento="Hielo"),
    ],
    # pega y se cura (el colmillo), su mecánica (la manada) y defensa (el gruñido)
    "arranque": ["Colmillo de escarcha", "Caza en manada", "Gruñido"],
    "arma_inicial": "espadon_de_recluta_de_viaje",
    "set_de": "Rey Lobo",
    "set": [
        pieza("Espadón del Rey Lobo", "MainHand", "pesada", {"CAR": 1, "CON": 1},
              "El espadón del Rey Lobo: hoja ancha de acero oscuro con escarcha en el filo y la guarda en forma de "
              "fauces de lobo.",
              restricciones=["Precisión: CAR", DOS_MANOS]),
        pieza("Coraza del Rey Lobo", "Torso", "media", {"CAR": 1},
              "Coraza de acero ennegrecido con un manto de piel de lobo gris sobre los hombros y la cabeza del lobo "
              "en plata en el pecho."),
        pieza("Yelmo del Rey Lobo", "Head", "media", {"CAR": 1, "CON": 1},
              "Yelmo de acero oscuro con dos orejas de lobo y el cuello de piel gris; la cara se ve."),
        pieza("Quijotes del Rey Lobo", "Legs", "media", {"CAR": 1, "CON": 1},
              "Quijotes de hierro negro."),
        pieza("Botas del Rey Lobo", "Feet", "media", {"CAR": 1, "CON": 1},
              "Botas forradas de piel gris."),
    ],
}


FE = [CLERIGO_ESPIGA, CLERIGO_CAMINANTE, PALADIN_JURAMENTO, CABALLERO_REY_LOBO]
