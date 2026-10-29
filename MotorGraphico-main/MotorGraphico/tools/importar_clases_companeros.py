#!/usr/bin/env python3
"""CLASES-03 — Importa la 3.ª iteracion de seis clases (Oriol, 27/09/2026) a los
catalogos: classes.json, skills.json, passives.json, summons.json y los seis
companeros reclutables de npcs.json.

Fuente: el texto que dio Oriol (corregido y con los huecos marcados como
[pendiente]). Este script ES la fuente en el repo: se puede releer, corregir
un texto y volver a ejecutar; hace upsert por id y no borra nada de lo que ya
habia (las cartas de la 2.ª iteracion siguen en skills.json con su classTag,
pero la clase pasa a arrancar y aprender las de esta lista).

Reglas del esquema (tools/validar_catalogos_rpg.py): tier 0-5, rarity de la
lista, recovery/actionType/target de las listas del motor
(src/RPG/SkillDefinitionEnums.cpp). Las habilidades "Creador" son tier 2,
rareza rare y descanso largo; las demas tier 1, common y descanso corto.

    python3 tools/importar_clases_companeros.py          # escribe
    python3 tools/importar_clases_companeros.py --ver    # solo cuenta
"""
import argparse
import json
import re
import sys
import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CAT = ROOT / "assets/catalogs"
sys.path.insert(0, str(Path(__file__).resolve().parent))
import importar_clases_sp as imp_sp  # noqa: E402  (SETS-03: los sets se escriben como los de las SP)
import sets_iteracion3  # noqa: E402
from sp_reglas import etiqueta_set, slug  # noqa: E402
PENDIENTE = "[pendiente] sin efecto definido todavía"


def slug(nombre):
    s = unicodedata.normalize("NFKD", nombre).encode("ascii", "ignore").decode().lower()
    s = re.sub(r"[^a-z0-9]+", "_", s).strip("_")
    return s


# (nombre, descripcion, target, extras)
def h(nombre, desc=PENDIENTE, target="single_enemy", **extras):
    return {"nombre": nombre, "desc": desc, "target": target, **extras}


CLASES = [
    {
        "id": "monje_del_despertar",
        "name": "Monje del Despertar",
        "description": "Artista marcial que acumula Ki con cada golpe certero y lo gasta en recuperar sus técnicas o en un segundo impacto.",
        "primaryStat": "DES", "secondaryStat": "CON", "role": "striker",
        "baseHealth": 5, "healthScaling": {"CON": 1.5}, "maxArmorWeight": "ligera", "maxWeaponWeight": "ligera",
        "primaryResource": "ki",
        "pasiva": ("Ki", "Si aciertas un golpe básico, acumulas Ki. Puedes gastar Ki para recuperar habilidades. Pierdes [pendiente: cantidad] de Ki o aplicas un segundo impacto.", "Al acertar un golpe básico"),
        # SETS-03 (Oriol, 08/10: «me podrías dar descripciones a las habilidades de estos dos últimos, solo les
        # puse la pasiva y el título»): las diez cartas del Monje, escritas desde su nombre con el presupuesto de
        # las clases propias (planificacion/CANON_CLASES_PROPIAS.md §3). Tiran DES (la de la clase). La pasiva
        # (el Ki) sigue como la dejó Oriol: las cartas no dependen de ella.
        "habilidades": [
            h("Purificación Corporal", "Respiras hondo y dejas que el Ki recorra tu cuerpo y arrastre lo que lo enferma: te libras de los estados en contra y, con el cuerpo limpio, te defiendes con ventaja 2 turnos.", target="self", duration="2 turnos"),
            h("Abdomen de Marfil", "Endureces el vientre con el Ki hasta que es duro como el marfil: el próximo golpe que te llegue rebota sin hacerte daño, y te quedas con el cuerpo templado, +1 a la CA 2 turnos.", target="self", actionType="reaccion", duration="2 turnos"),
            h("Rejuvenecimiento de Jade", "Una calma verde como el jade te llena por dentro y te cierra las heridas: te curas 2d6.", target="self"),
            h("Ráfaga Elemental", "Una ráfaga de puñetazos envueltos en fuego, agua, tierra y viento: 3d6 de daño."),
            h("Tres Golpes de Puro Dolor", "Tres golpes secos donde más duele: tres ataques con los puños, a uno o a varios enemigos, empezando por el que elijas."),
            h("Golpe Sanguíneo", "Un golpe con los nudillos que abre la carne: 2d6 de daño, y la herida sangra 3 turnos (1 de daño por turno).", duration="3 turnos", mechanic=["Sangre"]),
            h("Puño Ensordecedor", "Un puñetazo junto a la oreja que retumba como una campana y le deja el eco dentro de la cabeza: 1d6 de daño, y aturdido pasa 1 turno sin atacar y se defiende con desventaja.", duration="1 turno"),
        ],
        "creador": [
            h("Estampida", "Cargas contra los enemigos como una manada en estampida: 3d6 de daño a cada uno, y caen al suelo 1 turno.", target="all_enemies", duration="1 turno"),
            h("Sentidos de Alerta", "Despiertas todos tus sentidos a la vez y ves venir cada golpe antes de que salga: atacas y te defiendes con ventaja hasta el final del combate.", target="self", duration="combate"),
            h("Canción de Dios", "Entonas el canto que se aprende al despertar y el Ki de todos responde: tú y tus aliados os libráis de los estados en contra y os curáis 2d6.", target="all_allies"),
        ],
        # Pega (los tres golpes), aguanta (el abdomen) y se cura (el jade): antes salían las tres de apoyo.
        "arranque": ["Tres Golpes de Puro Dolor", "Abdomen de Marfil", "Rejuvenecimiento de Jade"],
    },
    {
        "id": "senor_de_las_armas",
        "pasiva": ("[pendiente]", "[pendiente] la pasiva del Señor de las Armas no esta definida", "Constante"),
        "habilidades": [
            h("Escudo Divino", "Bloqueas un ataque y ciegas al enemigo.", actionType="reaccion"),
            h("Ráfagas del Arsenal", "Lanzas hasta tres armas que atacan a uno o varios enemigos.", target="all_enemies"),
            h("Arsenal del Rey", "Invocas un arma que ataca por ti y causa 1d6.", target="self", mechanic=["Invocacion", "Magico"]),
            h("Cadena Real", "El enemigo encadenado tira con desventaja durante 3 turnos.", duration="3 turnos"),
            h("Corte Real", "Cortas en un radio de 8 pies y causas 2d6 de daño en área.", target="zone_radius", magnitude="2d6"),
            h("[pendiente] Sexta habilidad", "[pendiente] faltan 2 habilidades para llegar a 7"),
            h("[pendiente] Séptima habilidad", "[pendiente] faltan 2 habilidades para llegar a 7"),
        ],
        "creador": [
            h("Confianza Extrema", "Atacas junto a un aliado. Tu ataque se convierte en crítico y tu aliado no tira dados.", target="single_ally"),
            h("Moral Inconmensurable", "Das +1 a la CA de todos tus aliados.", target="all_allies"),
            h("Destructor", "Atacas con ventaja, pero los enemigos también te atacan con ventaja.", target="self"),
        ],
    },
    {
        "id": "bardo_de_las_cuatro_estaciones",
        "pasiva": ("Las Cuatro Estaciones", "Eliges un beneficio al día. Primavera: tus curaciones curan +2. Verano: tienes ventaja en ataque. Otoño: tienes ventaja en defensa. Invierno: ganas +2 a la CA y reflejas los ataques físicos durante 1 turno.", "Al empezar el día"),
        "habilidades": [
            h("Canción del Otoño", "Lanzas una melodía que da desventaja a todos los enemigos que la oigan durante 2 turnos.", target="all_enemies", duration="2 turnos"),
            h("Ola de Elementos", "Puedes devolver una canción a la mano.", target="self", actionType="libre"),
            h("Bendición de las Estrellas", "Bloqueas un ataque.", target="self", actionType="reaccion"),
            h("Canción de la Primavera", "Curas 1d6 a los aliados cercanos.", target="all_allies", magnitude="1d6"),
            h("Invocación Elemental", "Un aliado obtiene 2 PA al final de su turno y ventaja en sus tiradas de ataque durante 2 turnos.", target="single_ally", duration="2 turnos"),
            h("Canción del Invierno", "Congelas a los enemigos cercanos. Quedan fuera de combate 2 turnos y sus tiradas de defensa tienen desventaja.", target="all_enemies", duration="2 turnos"),
            h("Canción del Verano", "Lanzas una bola de fuego que causa 3d6 en un radio de 5 pies.", target="zone_radius", magnitude="3d6"),
        ],
        "creador": [
            h("Elevación del Ser", "Un jugador obtiene un turno adicional.", target="single_ally"),
            h("Egida Divina", "Añades +2d6 a la tirada de un aliado.", target="single_ally", actionType="reaccion", magnitude="2d6"),
            h("Último Aliento", "Si un aliado muere, vuelve a la vida con 1 PV.", target="single_ally", actionType="reaccion"),
        ],
    },
    {
        "id": "el_apostata",
        "pasiva": ("Desgarro Sangriento", "Puedes recuperar una habilidad a cambio de sumar 1 al contador de Desgarro. El contador se reinicia en cada descanso. [pendiente: aclarar 'pierdes 1 punto cada vez que lanzas un hechizo o habilidad desde tu zona de reposo']", "Al recuperar una habilidad"),
        "habilidades": [
            h("Escudo de Sombras", "Acumulas hasta 10 PV adicionales como máximo. Cada golpe que das te otorga 2.", target="self"),
            h("Absorción de Alma", "Canalizas tus artes mágicas y absorbes 1d6 de vida del oponente por turno durante 3 turnos.", actionType="canalizacion", duration="3 turnos", magnitude="1d6"),
            h("Destructor del Escudo de Sombras", "Provocas una explosión en un área de 4 pies que causa tanto daño como vida acumule el escudo.", target="zone_radius"),
            h("Maldición de la Debilidad", "Aplicas desventaja a un enemigo durante 10 turnos.", duration="10 turnos"),
            h("Invocación Oscura", "Invocas un aliado oscuro durante 10 turnos de combate.", target="self", mechanic=["Invocacion", "Magico"], duration="10 turnos"),
            h("Marca Profana", "Maldices a un enemigo. Durante 3 turnos lo atacas con ventaja y le haces +1d6 de daño.", duration="3 turnos", magnitude="1d6"),
            h("Estruendo Desolador", "Con un grito dejas a un enemigo fuera de combate durante 1 turno, y sus tiradas de defensa tienen desventaja.", duration="1 turno"),
        ],
        "creador": [
            h("Paso Ilusorio", "Haces un desplazamiento espiritual y dejas una copia tuya en el punto de origen. Cuesta el doble de recuperar con Desgarro Sangriento.", target="self", actionType="movimiento"),
            h("Susurro Demoníaco", "Durante 1 turno, tus ataques hacen 1d[pendiente] a los enemigos adyacentes a tu objetivo. Cuesta el doble de recuperar con Desgarro Sangriento.", target="self", duration="1 turno"),
            h("Ataque Maldito", "Al golpear te infliges [pendiente: X] de daño y lo sumas al daño que haces. Cuesta el doble de recuperar con Desgarro Sangriento.", target="self"),
        ],
        "invocaciones": [
            {"id": "espectro_menor", "name": "Espectro Menor", "stats": {"FUE": 4, "DES": 2, "INT": 0, "CAR": 5}, "health": 3},
            {"id": "cuervo_descompuesto", "name": "Cuervo Descompuesto", "stats": {"FUE": 1, "DES": 3, "INT": 0, "CAR": 3}, "health": 2},
        ],
    },
    {
        "id": "furia_de_los_creadores",
        "pasiva": ("Furia del Creador", "En Furia: cuando atacas puedes usar Magias Antiguas, es decir, jugar una carta de magia al azar.", "Al atacar en Furia"),
        "habilidades": [
            h("Castigo Divino", "Aplicas 1d6 de daño de luz durante 3 turnos.", duration="3 turnos", magnitude="1d6"),
            h("Ataque del Rayo", "Paralizas a un enemigo y le haces 1d6 de daño de rayo durante 2 turnos.", duration="2 turnos", magnitude="1d6"),
            h("Protección del Creador", "Contrarrestas todos los ataques durante 1 turno.", target="self", actionType="reaccion", duration="1 turno"),
            h("Furia: Fe", "Haces un ataque adicional y te curas el daño que causas."),
            h("Furia Justiciera", "Durante 1 turno puedes reaccionar cuando atacan a un aliado para defenderlo.", target="single_ally", actionType="reaccion", duration="1 turno"),
            h("Ira Imparable", "Tu ira se materializa y hace un ataque que atrae al enemigo. Dura 1 turno.", duration="1 turno"),
            h("Puño Volcánico", "Tu arma inflige 2d6 de daño de fuego adicional durante 2 turnos.", target="self", duration="2 turnos", magnitude="2d6"),
        ],
        "creador": [
            h("Fuerzas Internas", "Das una ráfaga de 3 golpes consecutivos y triplicas el daño."),
            h("A las Armas", "Haces un ataque adicional.", target="self", actionType="libre"),
            h("Desarme", "Lanzas un grito que desarma a los enemigos.", target="all_enemies"),
        ],
    },
    {
        "id": "mago_chasqueador",
        "name": "Mago Chasqueador",
        "description": "Mago de reflejos y suerte: sus dados vuelven a rodar y su magia llega antes que la respuesta.",
        "primaryStat": "INT", "secondaryStat": "DES", "role": "caster",
        "baseHealth": 4, "healthScaling": {"CON": 1.0}, "maxArmorWeight": "ligera", "maxWeaponWeight": "ligera",
        "primaryResource": "mana",
        "pasiva": ("Cascada Letal", "En cada tirada puedes volver a lanzar 1 dado. Si sacas 20 o más, puedes volver a lanzar hasta 2 dados. [pendiente: confirmar esta lectura de la condición]", "En cada tirada"),
        # SETS-03 (Oriol, 08/10): las diez cartas del Mago Chasqueador, escritas desde su nombre: todo lo lanza
        # chasqueando los dedos. Tiran INT (es magia, como las del Mago de las SP); las de estándar llevan la
        # etiqueta «estandar» para la Recuperación Prestidigitadora. La pasiva (Cascada Letal) sigue como la
        # dejó Oriol: las cartas no dependen de ella.
        "habilidades": [
            h("Bola de Fuego", "Chasqueas los dedos y una bola de fuego estalla sobre el enemigo: 3d6 de daño de fuego.", target="zone_radius", mechanic=["Fuego"], tira="INT", etiquetas=["estandar"]),
            h("Paso Instantáneo", "Un chasquido y ya no estás donde te apuntaban: apareces a unos pasos y te defiendes con ventaja 1 turno.", target="self", actionType="movimiento", duration="1 turno", tira="INT", etiquetas=["estandar"]),
            h("Sombra Cegadora", "Chasqueas delante de su cara y una sombra se le pega a los ojos. Si no se resiste (resistencia física), queda cegado 2 turnos: ataca con desventaja y todo contra él va con ventaja.", duration="2 turnos", mechanic=["Tinieblas"], tira="INT", etiquetas=["estandar"]),
            h("Rayo Electrizante", "Un chasquido y te salta un rayo de los dedos al enemigo: 2d6 de daño de rayo, y electrizado se defiende con desventaja 2 turnos.", duration="2 turnos", mechanic=["Rayo"], tira="INT", etiquetas=["estandar"]),
            h("Ocupa mi Lugar", "Chasqueas y cambias el sitio con un aliado en apuros (o te quitas tú de en medio): aparece a salvo donde estabas, se libra de los estados en contra y se defiende con ventaja 2 turnos.", target="single_ally", duration="2 turnos", tira="INT", etiquetas=["estandar"]),
            h("Espiral de Espinas", "Chasqueas y del suelo brota una espiral de espinas alrededor del enemigo: 2d6 de daño de naturaleza, y las espinas le dejan sangrando 2 turnos.", target="zone_radius", duration="2 turnos", mechanic=["Naturaleza"], tira="INT", etiquetas=["estandar"]),
            h("Recuperación Prestidigitadora", "Un juego de manos y una carta que ya habías gastado vuelve a aparecer entre tus dedos: recuperas una de tus cartas estándar gastadas.", target="self", actionType="libre", tira="INT", etiquetas=["estandar"]),
        ],
        "creador": [
            h("Sin Tregua", "Chasquido tras chasquido, sin darle un respiro: 4d6 de daño arcano, y sin tregua se defiende con desventaja 2 turnos.", duration="2 turnos", mechanic=["Arcano"], tira="INT"),
            h("Repliegue Mágico", "Te preparas para replegarte: cuando te ataquen, te escondes en un pliegue de la magia y contrarrestas todos los ataques durante 1 turno; al volver, te defiendes con ventaja hasta el final del combate.", target="self", actionType="reaccion", duration="1 turno", mechanic=["Arcano"], tira="INT"),
            h("Sin Sentido", "Le chasqueas los dedos delante de los ojos y le dejas sin sentido. Si no se resiste (defensa mental), pasa 2 turnos sin atacar y se defiende con desventaja.", duration="2 turnos", mechanic=["Arcano"], tira="INT"),
        ],
    },
]

# H2-06 (05/10/2026) — Lo que cada carta HACE EN COMBATE, escrito para la
# maquina: effects[].mechanicHook, el mismo DSL que las condiciones de
# conditions.json (frase de la carta -> gancho). Lo ejecuta
# Game::CartasEnCombate; la gramatica esta en include/Game/GanchosCarta.h.
# Oriol (05/10): de momento SOLO combate de Bardo, Apostata, Senor de las
# Armas y Furia; lo de fuera del combate, al final. Lo que la carta no dice
# no se rellena: lo que falta va como pendiente: y la carta sale bloqueada
# en el menu (pendiente_parcial: juega lo que si esta definido).
GANCHOS = {
    # --- Bardo de las Cuatro Estaciones ---
    "Canción del Otoño": [("Da desventaja a todos los enemigos que la oigan durante 2 turnos.", "estado:2:desventaja:todo")],
    "Ola de Elementos": [("Puedes devolver una canción a la mano.", "devuelve_carta:Cancion")],
    "Bendición de las Estrellas": [("Bloqueas un ataque.", "bloquea:ataque")],
    "Canción de la Primavera": [("Curas 1d6 a los aliados cercanos.", "cura:1d6")],
    "Invocación Elemental": [
        ("Ventaja en sus tiradas de ataque durante 2 turnos.", "estado:2:ventaja:ataque"),
        ("Obtiene 2 PA al final de su turno.", "pendiente_parcial:que son los 2 PA"),
    ],
    "Canción del Invierno": [
        ("Quedan fuera de combate 2 turnos.", "estado:2:incapacitado"),
        ("Sus tiradas de defensa tienen desventaja.", "estado:2:desventaja:defensa"),
    ],
    "Canción del Verano": [("Una bola de fuego que causa 3d6.", "dano:3d6")],
    "Elevación del Ser": [("Un jugador obtiene un turno adicional.", "turno_extra")],
    "Egida Divina": [("Añades +2d6 a la tirada de un aliado.", "dados_extra:2")],
    "Último Aliento": [("Si un aliado muere, vuelve a la vida con 1 PV.", "revive:1")],
    # --- El Apostata ---
    "Escudo de Sombras": [("Acumulas hasta 10 PV adicionales; cada golpe que das te otorga 2.", "escudo:2:10")],
    "Absorción de Alma": [("Absorbes 1d6 de vida del oponente por turno durante 3 turnos.", "estado:3:absorbe_por_turno:1d6")],
    "Destructor del Escudo de Sombras": [("Causa tanto daño como vida acumule el escudo.", "dano:escudo")],
    "Maldición de la Debilidad": [("Aplicas desventaja a un enemigo durante 10 turnos.", "estado:10:desventaja:todo")],
    "Invocación Oscura": [("Invocas un aliado oscuro durante 10 turnos de combate.", "invoca:10:espectro_menor,cuervo_descompuesto")],
    "Marca Profana": [
        ("Durante 3 turnos lo atacas con ventaja.", "estado:3:ventaja_contra:origen"),
        ("Y le haces +1d6 de daño.", "estado:3:bono_dano_contra:1d6"),
    ],
    "Estruendo Desolador": [
        ("Lo dejas fuera de combate durante 1 turno.", "estado:1:incapacitado"),
        ("Sus tiradas de defensa tienen desventaja.", "estado:1:desventaja:defensa"),
    ],
    "Paso Ilusorio": [
        ("Desplazamiento espiritual con una copia en el punto de origen.", "requiere_posiciones"),
        ("Cuesta el doble de recuperar con Desgarro Sangriento.", "coste_desgarro:2"),
    ],
    "Susurro Demoníaco": [
        ("Tus ataques hacen 1d[pendiente] a los enemigos adyacentes a tu objetivo.", "pendiente:el dado del Susurro y los enemigos adyacentes"),
        ("Cuesta el doble de recuperar con Desgarro Sangriento.", "coste_desgarro:2"),
    ],
    "Ataque Maldito": [
        ("Te infliges [pendiente: X] de daño y lo sumas al que haces.", "pendiente:la X del Ataque Maldito"),
        ("Cuesta el doble de recuperar con Desgarro Sangriento.", "coste_desgarro:2"),
    ],
    # --- Senor de las Armas ---
    "Escudo Divino": [("Bloqueas un ataque.", "bloquea:ataque"), ("Y ciegas al enemigo.", "condicion:cegado")],
    "Ráfagas del Arsenal": [("Lanzas hasta tres armas que atacan a uno o varios enemigos.", "ataques_arma:3")],
    "Arsenal del Rey": [("Un arma invocada ataca por ti y causa 1d6.", "dano:1d6")],
    "Cadena Real": [("El enemigo encadenado tira con desventaja durante 3 turnos.", "estado:3:desventaja:todo")],
    "Corte Real": [("Causas 2d6 de daño en área.", "dano:2d6")],
    "[pendiente] Sexta habilidad": [("[pendiente]", "pendiente:faltan 2 habilidades")],
    "[pendiente] Séptima habilidad": [("[pendiente]", "pendiente:faltan 2 habilidades")],
    "Confianza Extrema": [
        ("Tu ataque se convierte en crítico.", "ataque_arma:critico"),
        ("Tu aliado ataca contigo y no tira dados.", "aliado_ataca:sin_dados"),
    ],
    "Moral Inconmensurable": [("Das +1 a la CA de todos tus aliados.", "estado:combate:bono_ca:1")],
    "Destructor": [
        ("Atacas con ventaja.", "ataque_arma:ventaja"),
        ("Los enemigos también te atacan con ventaja.", "estado:1:ventaja_contra:todo"),
    ],
    # --- Furia de los Creadores ---
    "Castigo Divino": [("Aplicas 1d6 de daño de luz durante 3 turnos.", "estado:3:dano_por_turno:1d6")],
    "Ataque del Rayo": [
        ("Paralizas a un enemigo durante 2 turnos.", "condicion:paralizado:2"),
        ("Y le haces 1d6 de daño de rayo durante 2 turnos.", "estado:2:dano_por_turno:1d6"),
    ],
    "Protección del Creador": [("Contrarrestas todos los ataques durante 1 turno.", "bloquea:ataques:1")],
    "Furia: Fe": [("Haces un ataque adicional.", "ataque_arma"), ("Y te curas el daño que causas.", "cura:dano_causado")],
    "Furia Justiciera": [("Durante 1 turno defiendes a un aliado cuando lo atacan.", "defiende_aliado:1")],
    "Ira Imparable": [("Haces un ataque.", "ataque_arma"), ("Que atrae al enemigo durante 1 turno.", "estado:1:provocado:origen")],
    "Puño Volcánico": [("Tu arma inflige 2d6 de daño de fuego adicional durante 2 turnos.", "estado:2:bono_dano_arma:2d6")],
    "Fuerzas Internas": [("Una ráfaga de 3 golpes consecutivos que triplica el daño.", "ataque_arma:x3")],
    "A las Armas": [("Haces un ataque adicional.", "ataque_arma")],
    "Desarme": [("Desarmas a los enemigos.", "estado:combate:desarmado")],
    # --- Monje del Despertar (SETS-03, 08/10/2026) ---
    "Purificación Corporal": [("Te libras de los estados en contra.", "limpia"),
                              ("Cuerpo limpio: te defiendes con ventaja 2 turnos.", "estado:2:ventaja:defensa")],
    "Abdomen de Marfil": [("Bloqueas el próximo ataque.", "bloquea:ataque"),
                          ("Cuerpo templado: +1 a la CA 2 turnos.", "propio:2:bono_ca:1")],
    "Rejuvenecimiento de Jade": [("Te curas 2d6.", "cura:2d6")],
    "Ráfaga Elemental": [("3d6 de daño.", "dano:3d6")],
    "Tres Golpes de Puro Dolor": [("Tres ataques con los puños.", "ataques_arma:3")],
    "Golpe Sanguíneo": [("2d6 de daño.", "dano:2d6"), ("La herida sangra 3 turnos.", "condicion:sangrado:3")],
    "Puño Ensordecedor": [("1d6 de daño.", "dano:1d6"), ("Aturdido: no ataca 1 turno.", "estado:1:incapacitado"),
                          ("Aturdido: se defiende con desventaja 1 turno.", "estado:1:desventaja:defensa")],
    "Estampida": [("3d6 de daño a cada enemigo.", "dano:3d6"), ("Derribo: caen al suelo 1 turno.", "condicion:caido:1")],
    "Sentidos de Alerta": [("Atacas con ventaja hasta el final del combate.", "estado:combate:ventaja:ataque"),
                           ("Te defiendes con ventaja hasta el final del combate.", "estado:combate:ventaja:defensa")],
    "Canción de Dios": [("Os libráis de los estados en contra.", "limpia"), ("Os curáis 2d6.", "cura:2d6")],
    # --- Mago Chasqueador (SETS-03, 08/10/2026) ---
    "Bola de Fuego": [("3d6 de daño de fuego.", "dano:3d6")],
    "Paso Instantáneo": [("Te defiendes con ventaja 1 turno.", "estado:1:ventaja:defensa")],
    "Sombra Cegadora": [("Cegado 2 turnos.", "condicion:cegado:2")],
    "Rayo Electrizante": [("2d6 de daño de rayo.", "dano:2d6"),
                          ("Electrizado: se defiende con desventaja 2 turnos.", "estado:2:desventaja:defensa")],
    "Ocupa mi Lugar": [("Se libra de los estados en contra.", "limpia"),
                       ("A salvo: se defiende con ventaja 2 turnos.", "estado:2:ventaja:defensa")],
    "Espiral de Espinas": [("2d6 de daño de naturaleza.", "dano:2d6"), ("Sangra 2 turnos.", "condicion:sangrado:2")],
    "Recuperación Prestidigitadora": [("Recuperas una de tus cartas estándar gastadas.", "devuelve_carta:estandar")],
    "Sin Tregua": [("4d6 de daño arcano.", "dano:4d6"),
                   ("Sin tregua: se defiende con desventaja 2 turnos.", "estado:2:desventaja:defensa")],
    "Repliegue Mágico": [("Contrarrestas todos los ataques durante 1 turno.", "bloquea:ataques:1"),
                         ("Te defiendes con ventaja hasta el final del combate.", "propio:combate:ventaja:defensa")],
    "Sin Sentido": [("Sin sentido: no ataca 2 turnos.", "estado:2:incapacitado"),
                    ("Se defiende con desventaja 2 turnos.", "estado:2:desventaja:defensa")],
}

GANCHOS_PASIVA = {
    "bardo_de_las_cuatro_estaciones": [
        ("Eliges un beneficio al día.", "elige_una:primavera,verano,otono,invierno"),
        ("Primavera: tus curaciones curan +2.", "si:primavera:bono_cura:2"),
        ("Verano: tienes ventaja en ataque.", "si:verano:ventaja:ataque"),
        ("Otoño: tienes ventaja en defensa.", "si:otono:ventaja:defensa"),
        ("Invierno: ganas +2 a la CA.", "si:invierno:bono_ca:2"),
        ("Invierno: reflejas los ataques físicos durante 1 turno.", "si:invierno@1:refleja:fisico"),
    ],
    "el_apostata": [
        ("Recuperas una habilidad a cambio de sumar 1 al contador de Desgarro.", "recupera_carta:1"),
        ("[pendiente] que hace el contador de Desgarro.", "pendiente_parcial:que hace el contador de Desgarro"),
    ],
    "senor_de_las_armas": [("[pendiente]", "pendiente:la pasiva no esta definida")],
    "furia_de_los_creadores": [
        ("En Furia: al atacar puedes jugar una carta de magia al azar.", "pendiente:como entra en Furia esta clase y que cartas son de magia"),
    ],
}


def efectos(pares):
    return [{"description": d, "mechanicHook": g} for d, g in pares]


# SETS-03: las cartas que se escriben aqui (no las dio Oriol con su texto) dicen sus cifras, como las de las
# SP y las clases propias: los dados, «N turno(s)» y «hasta el final del combate».
REVISADAS = {"monje_del_despertar", "mago_chasqueador"}


def revisa_texto(clase_id, hab):
    donde = f"{clase_id} / {hab['nombre']}"
    if hab["nombre"] not in GANCHOS:
        raise SystemExit(f"{donde}: sin ganchos")
    for _frase, g in GANCHOS[hab["nombre"]]:
        m = re.match(r"^(dano|cura):(\d+d\d+)$", g)
        if m and m.group(2) not in hab["desc"]:
            raise SystemExit(f"{donde}: el texto no dice {m.group(2)} ({g})")
        m = re.match(r"^(estado|propio|condicion:\w+):?(\d+)(:|$)", g)
        if m:
            n = int(m.group(2))
            dicho = f"{n} turno" if n == 1 else f"{n} turnos"
            if dicho not in hab["desc"]:
                raise SystemExit(f"{donde}: el texto no dice «{dicho}» ({g})")
        if g.startswith(("estado:combate:", "propio:combate:")) and "final del combate" not in hab["desc"]:
            raise SystemExit(f"{donde}: el texto no dice «hasta el final del combate» ({g})")
    if hab.get("actionType") == "reaccion" and hab["target"] not in ("self", "single_ally"):
        raise SystemExit(f"{donde}: una reaccion se prepara para ti o para un aliado")


def revisa_set(st, clases):
    """Un set entero (el arma y una pieza por ranura de armadura), raro de tier 2, de una clase que existe y
    que no empieza con ninguna de sus piezas (Oriol, 07/10: la clase no lleva su set)."""
    slots = sorted(p["slot"] for p in st["set"])
    if slots != sorted(["MainHand", "Torso", "Head", "Legs", "Feet"]):
        raise SystemExit(f"{st['id']}: el set tiene que ser arma, cabeza, torso, piernas y pies: {slots}")
    clase = next((c for c in clases if c["id"] == st["id"]), None)
    if clase is None:
        raise SystemExit(f"{st['id']}: no hay clase")
    piezas = {p["id"] for p in st["set"]}
    if piezas & set(clase.get("startingEquipment") or []):
        raise SystemExit(f"{st['id']}: la clase no empieza con su set")
    for p in st["set"]:
        if (p["rarity"], p["tier"]) != ("rare", 2):
            raise SystemExit(f"{p['id']}: las piezas del set son raras de tier 2")


# Los companeros reclutables: un NPC por clase. Sin canon propio todavia, el
# nombre y la raza son provisionales ([pendiente]) y solo fijan el contrato.
COMPANEROS = [
    ("monje_del_despertar", "Sunna del Despertar", "race_human_marches"),
    ("senor_de_las_armas", "Roderic Arsenal", "race_human_marches"),
    ("bardo_de_las_cuatro_estaciones", "Ilse de las Estaciones", "elfos_del_bosque"),
    ("el_apostata", "Corvo el Apóstata", "drow_explorador"),
    ("furia_de_los_creadores", "Bruna Furia", "draconidos_de_fuego"),
    ("mago_chasqueador", "Ezra Chasquido", "race_human_marches"),
]


def sangria(nombre):
    """Respeta la sangría del fichero (los catalogos van con indent=1): asi el
    diff solo lleva las entradas nuevas, no el fichero entero."""
    with open(CAT / f"{nombre}.json", encoding="utf-8") as f:
        f.readline()
        segunda = f.readline()
    return max(1, len(segunda) - len(segunda.lstrip(" ")))


def cargar(nombre):
    with open(CAT / f"{nombre}.json", encoding="utf-8") as f:
        return json.load(f)


def guardar(nombre, doc):
    indent = sangria(nombre)
    with open(CAT / f"{nombre}.json", "w", encoding="utf-8") as f:
        json.dump(doc, f, ensure_ascii=False, indent=indent)
        f.write("\n")


def upsert(entries, entry):
    for i, e in enumerate(entries):
        if e.get("id") == entry["id"]:
            entries[i] = {**e, **entry}
            return "actualizada"
    entries.append(entry)
    return "nueva"


# DANO-02 (Oriol, 06/10/2026) — Las cartas de magia piden salvacion: el
# objetivo tira contra ella; si la supera, recibe la mitad del dano y nunca
# los estados de la carta. Las demas son golpes: se restan los exitos y la
# diferencia son d6 (ver include/RPG/DanoEnfrentado.h).
SALVACION = {
    "Canción del Otoño": "defensa_mental",
    "Canción del Invierno": "resistencia_fisica",
    "Canción del Verano": "resistencia_fisica",
    "Castigo Divino": "resistencia_fisica",
    "Ataque del Rayo": "resistencia_fisica",
    "Desarme": "defensa_mental",
    "Absorción de Alma": "resistencia_fisica",
    "Destructor del Escudo de Sombras": "resistencia_fisica",
    "Maldición de la Debilidad": "defensa_mental",
    "Marca Profana": "defensa_mental",
    "Estruendo Desolador": "defensa_mental",
    # SETS-03: los controles sin daño del Mago Chasqueador (la sombra en los ojos es del cuerpo; dejarle sin
    # sentido, de la cabeza)
    "Sombra Cegadora": "resistencia_fisica",
    "Sin Sentido": "defensa_mental",
}


def habilidad(clase_id, hab, creador):
    sid = slug(hab["nombre"])
    if sid.startswith("pendiente_"):
        sid = f"{clase_id}_{sid}"
    pendiente = hab["desc"].startswith("[pendiente]")
    mech = hab.get("mechanic", [])
    # H2-06 — las canciones del Bardo llevan su etiqueta: Ola de Elementos
    # devuelve «una cancion» a la mano (devuelve_carta:Cancion). SETS-03: con
    # o sin tilde (TILDES-04 puso «Canción» y la etiqueta se perdia al volver a
    # escribir el catalogo).
    if slug(hab["nombre"]).startswith("cancion_"):
        mech = mech + ["Cancion"]
    extra = {"effects": efectos(GANCHOS[hab["nombre"]])} if hab["nombre"] in GANCHOS else {}
    return {
        "id": sid,
        "name": hab["nombre"],
        "type": "skill",
        "tier": 2 if creador else 1,
        "rarity": "rare" if creador else "common",
        "description": hab["desc"],
        "flavorText": "",
        "classTags": [clase_id],
        "roleTags": [],
        "tags": [clase_id] + (["creador"] if creador else []) + mech + hab.get("etiquetas", []),
        "mechanicTags": mech + (["Creador"] if creador else []),
        "actionType": hab.get("actionType", "accion_principal"),
        "target": hab["target"],
        "range": "short",
        "duration": hab.get("duration", "instant"),
        "cost": {"amount": 0, "resource": "none"},
        "recovery": "descanso_largo" if creador else "descanso_corto",
        # SETS-03: con que se tira (el Mago Chasqueador, INT); sin nada, DES
        "castingStat": hab.get("tira"),
        "defenseStat": SALVACION.get(hab["nombre"]),
        "magnitudeByDegree": ({"1": hab["magnitude"]} if "magnitude" in hab else None),
        "effect": {"description": hab["desc"], "scaling": "none"},
        "requiredStats": {},
        "requiredTags": [],
        "incompatibleTags": [],
        "limitations": (["[pendiente] efecto sin definir"] if pendiente else []),
        "evolvesInto": None,
        "upgradePath": [],
        "iteracion": 3,
        **extra,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ver", action="store_true")
    args = ap.parse_args()
    clases, skills, passives, summons, npcs = (cargar(n) for n in ("classes", "skills", "passives", "summons", "npcs"))
    resumen = []
    for c in CLASES:
        cid = c["id"]
        pas_nombre, pas_desc, pas_trigger = c["pasiva"]
        pas_id = f"{cid}_pasiva" if pas_nombre.startswith("[pendiente]") else slug(pas_nombre)
        pasiva = {
            "id": pas_id, "name": pas_nombre if not pas_nombre.startswith("[pendiente]") else "Pasiva [pendiente]",
            "type": "passive", "tier": 1, "rarity": "common", "description": pas_desc,
            "flavorText": "", "classTags": [cid], "trigger": pas_trigger,
            "effect": {"description": pas_desc, "scaling": "none"}, "limitations": [],
            "synergyTags": [], "unique": True, "iteracion": 3,
        }
        if cid in GANCHOS_PASIVA:
            pasiva["effects"] = efectos(GANCHOS_PASIVA[cid])
        upsert(passives["entries"], pasiva)
        ids_base = []
        for hab in c["habilidades"]:
            s = habilidad(cid, hab, creador=False)
            upsert(skills["entries"], s)
            ids_base.append(s["id"])
        ids_creador = []
        for hab in c["creador"]:
            s = habilidad(cid, hab, creador=True)
            upsert(skills["entries"], s)
            ids_creador.append(s["id"])
        for inv in c.get("invocaciones", []):
            st = inv["stats"]
            upsert(summons["entries"], {
                "id": inv["id"], "name": inv["name"], "type": "summon", "tier": 1, "rarity": "uncommon",
                "description": f"FUE {st['FUE']}, DES {st['DES']}, INT {st['INT']}, CAR {st['CAR']}, {inv['health']} PV.",
                "flavorText": "", "health": inv["health"], "movement": 6, "control": "jugador",
                "duration": "10 turnos", "summonedBy": "invocacion_oscura",
                "stats": st,
                "attacks": [{"name": "Zarpazo", "effect": "[pendiente] ataque básico de la invocación"}],
                "passive": {"name": "", "description": ""}, "iteracion": 3,
            })
        existente = next((e for e in clases["entries"] if e["id"] == cid), None)
        arranque = [i for i in ids_base if not i.startswith(f"{cid}_pendiente")][:3]
        if c.get("arranque"):  # SETS-03: las que dicen que es la clase (el Monje: pega, aguanta y se cura)
            por_nombre = {h_["nombre"]: slug(h_["nombre"]) for h_ in c["habilidades"]}
            arranque = [por_nombre[n] for n in c["arranque"]]
        if cid in REVISADAS:
            for hab in c["habilidades"] + c["creador"]:
                revisa_texto(cid, hab)
        entrada = {
            "id": cid, "type": "class", "tier": 1, "rarity": "uncommon",
            "startingCards": {"skills": arranque, "learnableSkills": ids_base + ids_creador,
                              "passives": [pas_id], "spells": []},
            "iteracion": 3,
        }
        if existente is None:
            entrada.update({
                "name": c["name"], "description": c["description"], "flavorText": "",
                "primaryStat": c["primaryStat"], "secondaryStat": c["secondaryStat"], "role": c["role"],
                "baseHealth": c["baseHealth"], "healthScaling": c["healthScaling"],
                "maxArmorWeight": c["maxArmorWeight"], "maxWeaponWeight": c["maxWeaponWeight"],
                "primaryResource": c["primaryResource"], "secondaryResource": None,
                "allowedEquipmentTags": [], "restrictedTags": [], "specializations": [],
                "startingEquipment": [],
            })
        estado = upsert(clases["entries"], entrada)
        resumen.append(f"{cid}: {estado}, {len(ids_base)} habilidades + {len(ids_creador)} creador, pasiva {pas_id}")
    for cid, nombre, raza in COMPANEROS:
        upsert(npcs["entries"], {
            "id": f"companero_{cid}", "name": nombre, "type": "npc", "tier": 1, "rarity": "uncommon",
            "description": f"[pendiente] Compañero reclutable de la clase {cid}: nombre y raza provisionales.",
            "classId": cid, "raceId": raza, "occupation": "companero",
            "companion": {"recruitable": True, "joinFlag": f"companero_{cid}", "usesClassCards": True},
            "iteracion": 3,
        })
    # SETS-03: el set de tier 2 de cada una (tools/sets_iteracion3.py): objetos raros sueltos que la clase no
    # lleva, con su puente item_<id> en las librerias de objetos (el inventario y la tienda)
    rutas = {"equipo": CAT / "equipment.json", "armas": ROOT / "assets/objects/libreria_armas.json",
             "completa": ROOT / "assets/objects/libreria_completa.json"}
    docs = {n: imp_sp.cargar(r) for n, r in rutas.items()}
    # SETS-04: el set de SETS-03 del Señor de las Armas sale de los catálogos (lo sustituyen sus tres sets)
    retirados = {slug(n) for n in sets_iteracion3.RETIRADOS}
    antes = len(docs["equipo"]["entries"])
    docs["equipo"]["entries"] = [e for e in docs["equipo"]["entries"] if e["id"] not in retirados]
    for n in ("armas", "completa"):
        docs[n]["objects"] = [o for o in docs[n]["objects"] if o["id"] not in {f"item_{r}" for r in retirados}]
    if antes != len(docs["equipo"]["entries"]):
        resumen.append(f"retirados: {antes - len(docs['equipo']['entries'])} objetos del set de antes")
    for st in sets_iteracion3.SETS:
        revisa_set(st, clases["entries"])
        etiqueta = etiqueta_set(st["set_de"])
        for pz in st["set"]:
            e = dict(pz, grantedTags=pz["grantedTags"] + [etiqueta])
            upsert(docs["equipo"]["entries"], e)
            for n in ("armas", "completa"):
                upsert(docs[n]["objects"], imp_sp.puente(e))
        resumen.append(f"{st['id']}: {etiqueta}, {len(st['set'])} piezas")
    if args.ver:
        print("\n".join(resumen))
        return
    for n, r in rutas.items():
        imp_sp.guardar(r, docs[n])
    guardar("classes", clases)
    guardar("skills", skills)
    guardar("passives", passives)
    guardar("summons", summons)
    guardar("npcs", npcs)
    print("\n".join(resumen))
    print(f"companeros: {len(COMPANEROS)}")


if __name__ == "__main__":
    main()
