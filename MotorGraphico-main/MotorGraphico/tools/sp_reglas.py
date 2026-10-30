"""SP-02 — Las reglas de la adaptacion de las SP de NosTale a nuestro sistema, en codigo.

Lo que dice planificacion/CANON_SP_CLASES.md, para que las cifras de NosTale den siempre
las mismas cartas: dados por la bonificacion de la ficha, rondas por los segundos, y las
piezas con que se escribe una carta, una pasiva y el set de una SP. Los datos de cada SP
estan en sp_<clase base>.py (sp_espadachin.py); quien los escribe en los catalogos es
importar_clases_sp.py.
"""
import re
import unicodedata

RECURSO = "ninguno (sistema de pilas Activa/Descanso Corto/Descanso Largo, ver recovery de cada habilidad)"


def slug(nombre):
    s = unicodedata.normalize("NFKD", nombre).encode("ascii", "ignore").decode().lower()
    return re.sub(r"[^a-z0-9]+", "_", s).strip("_")


# --- cifras (§4) ----------------------------------------------------------------------------

def dados(bono):
    """Bonificacion total de la ficha de NosTale (cuerpo a cuerpo + elemental) -> dados."""
    if bono < 500:
        return "1d6"
    if bono < 1300:
        return "2d6"
    if bono < 2500:
        return "3d6"
    return "4d6"


def rondas(segundos):
    """Duracion de NosTale -> rondas de combate ('combate' = hasta que acabe)."""
    if segundos <= 6:
        return 1
    if segundos <= 15:
        return 2
    if segundos <= 45:
        return 3
    if segundos < 120:
        return 5
    return "combate"


# --- ganchos (include/Game/GanchosCarta.h) ---------------------------------------------------

def dano(*bonos):
    """Los dados de la carta por la suma de sus bonificaciones: (cuerpo a cuerpo, elemental) y,
    si es una tecnica con continuacion (§2), las de los pasos que se le suman."""
    return f"dano:{dados(sum(bonos))}"


def estado(segundos, rasgo):
    return f"estado:{rondas(segundos)}:{rasgo}"


def propio(segundos, rasgo):
    """Lo gana quien juega la carta (en una reaccion, al saltar)."""
    return f"propio:{rondas(segundos)}:{rasgo}"


def condicion(ident, segundos=None):
    return f"condicion:{ident}" + (f":{rondas(segundos)}" if segundos is not None else "")


def siempre(rasgo):
    return f"siempre:{rasgo}"


def si(opcion, rasgo):
    return f"si:{opcion}:{rasgo}"


def elige_una(*opciones):
    return "elige_una:" + ",".join(opciones)


# --- piezas de carta que se repiten (todas las clases base) ------------------------------------

def pendiente(que):
    """Lo que el combate no simula y la carta o la pasiva deja dicho (pendiente_parcial: se juega
    igual)."""
    return (f"[pendiente] {que}.", f"pendiente_parcial:{que}")


# Una ventana de defensa de 2 s (§4): una reaccion que contrarresta los ataques de ese turno. Lo
# que gana quien reacciona dura 2 rondas: salta en el turno enemigo y tiene que llegar al tuyo
# (con 1 se acabaria al cerrar la misma ronda).
CONTRARRESTA = ("Contrarresta los ataques de ese turno.", "bloquea:ataques:1")


def aturdido(segundos, nombre="Aturdido"):
    """El aturdimiento (o el shock, que en la mesa es lo mismo): no ataca y se defiende con
    desventaja."""
    return [(f"{nombre}: no ataca.", estado(segundos, "incapacitado")),
            (f"{nombre}: se defiende con desventaja.", estado(segundos, "desventaja:defensa"))]


def atrae():
    """«Los oponentes se fijaran en ti»: sin posiciones, te atacan a ti esa ronda."""
    return ("Te atacan a ti 1 turno.", "estado:1:provocado:origen")


# --- una carta --------------------------------------------------------------------------------

def carta(h, nombre, target, desc, efectos, salvacion=None, elemento=None, accion="accion_principal",
          tira=None):
    """efectos: [(frase de la carta, gancho)]. h: la habilidad de NosTale (H812), o varias si
    la carta junta una tecnica con su continuacion (H1093+H1100). tira: el atributo con que se
    tira, si no es el de la SP (§7: el Circulo de fuego de la Tormenta de fuego es magia)."""
    return {"h": h, "nombre": nombre, "target": target, "desc": desc, "efectos": efectos,
            "salvacion": salvacion, "elemento": elemento, "accion": accion, "tira": tira}


# §7: con que atributo se tiran las cartas. Lo dice la bonificacion de la ficha de NosTale:
# «ataque magico» -> INT (los Magos, como las cartas del Arcanista); cuerpo a cuerpo y a
# distancia -> DES (DANO-02, lo de siempre: la SP no dice nada).
TIRADAS = {"INT", "DES", "CON", "CAR"}


# --- la ficha por rol (§7: las plantillas que ya usan las 63 clases) -------------------------

PLANTILLA_ROL = {
    "tank": {"baseHealth": 8, "healthScaling": {"CON": 4.0}, "maxArmorWeight": "pesada",
             "maxWeaponWeight": "pesada"},
    "balanced": {"baseHealth": 6, "healthScaling": {"CON": 3.0}, "maxArmorWeight": "media",
                 "maxWeaponWeight": "pesada"},
    "agile": {"baseHealth": 5, "healthScaling": {"CON": 2.0}, "maxArmorWeight": "ligera",
              "maxWeaponWeight": "media"},
    "support": {"baseHealth": 5, "healthScaling": {"CON": 2.0}, "maxArmorWeight": "media",
                "maxWeaponWeight": "media"},
    # los Magos que lanzan desde atras (la de 13 de las 15 clases caster del catalogo)
    "caster": {"baseHealth": 4, "healthScaling": {"CON": 1.5}, "maxArmorWeight": "ligera",
               "maxWeaponWeight": "ligera"},
}


# --- el set (§8) ------------------------------------------------------------------------------
# Un set entero por SP (Oriol, 07/10): el arma y las cuatro piezas, objetos raros de tier 2
# (nivel 20, 550 de precio, como las raras del catalogo) que cualquiera puede llevar y
# combinar. No es el equipo inicial de la clase.

PESO = {"ligera": "light", "media": "medium", "pesada": "heavy"}
ETIQUETA_ARMADURA = {"ligera": "Armadura Ligera", "media": "Armadura Media", "pesada": "Armadura Pesada"}
ETIQUETA_ARMA = {"ligera": "Arma Ligera", "media": "Arma Media", "pesada": "Arma Pesada"}
SLOT_PUENTE = {"MainHand": "arma_principal", "Torso": "torso", "Head": "cabeza", "Legs": "piernas",
               "Feet": "pies",
               # PART-02: las ranuras nuevas (los abalorios del Tier 2, tools/abalorios_t2.py)
               "Arms": "brazos", "Neck": "cuello", "Belt": "cintura", "Ring": "anillo"}
DOS_MANOS = "A dos manos: deja inutilizable el slot de arma secundaria"


def objeto(nombre, slot, peso, bonos, desc, etiquetas=None, restricciones=None, rareza="rare", tier=2,
           nivel=20, precio=550):
    b = {"CAR": 0, "CON": 0, "DES": 0, "INT": 0}
    b.update(bonos)
    if etiquetas is None:
        etiquetas = ([ETIQUETA_ARMADURA[peso]] if slot == "Torso" else
                     [ETIQUETA_ARMA[peso]] if slot == "MainHand" else [])
    return {
        "compatibleSymbols": [], "defenseBonuses": {}, "description": desc, "flavorText": desc,
        "grantedTags": etiquetas, "id": slug(nombre), "linkedSkill": None, "name": nombre,
        "penalties": {"CAR": 0, "CON": 0, "DES": 0, "INT": 0}, "price": precio, "rarity": rareza,
        "requiredLevel": nivel, "requiredStats": {"CAR": 0, "CON": 0, "DES": 0, "INT": 0},
        "restrictions": restricciones or [], "slot": slot, "statBonuses": b, "tier": tier,
        "type": "equipment", "weightCategory": PESO[peso],
    }


# Las SP con nombre propio no llevan articulo: «Set de Aquiles», pero «Set del Guerrero»; las de
# nombre femenino, «de la»: «Set de la Tormenta de fuego»; y en plural, «de las»: «Set de las Artes
# Místicas».
NOMBRES_PROPIOS = {"Aquiles", "Volcano", "Grávitas", "Merlín", "Thor"}
NOMBRES_FEMENINOS = {"Tormenta de fuego", "Espiga Dorada", "Furia"}  # SETS-03: «Set de la Furia»
NOMBRES_FEMENINOS_PLURALES = {"Artes Místicas", "Cuatro Estaciones"}  # SETS-03: «Set de las Cuatro Estaciones»


def etiqueta_set(clase):
    if clase in NOMBRES_PROPIOS:
        return f"Set de {clase}"
    if clase in NOMBRES_FEMENINOS_PLURALES:
        return f"Set de las {clase}"
    return f"Set de la {clase}" if clase in NOMBRES_FEMENINOS else f"Set del {clase}"


def pieza(nombre, slot, peso, bonos, desc, etiquetas=None, restricciones=None):
    """Una pieza del set de una SP (rara, tier 2)."""
    return objeto(nombre, slot, peso, bonos, desc, etiquetas, restricciones)


def arma_comun(nombre, peso, desc, restricciones=None, etiquetas=None):
    """Un arma normal de tier 1 para empezar, cuando el catalogo no tiene ninguna de ese tipo
    (la guadana): comun, sin bonos, como las de Milicia y de Viaje. Sin etiquetas, la de su peso."""
    return objeto(nombre, "MainHand", peso, {}, desc, etiquetas=etiquetas, restricciones=restricciones,
                  rareza="common", tier=1, nivel=1, precio=100)
