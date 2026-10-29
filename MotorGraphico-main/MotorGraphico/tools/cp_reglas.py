"""CP-01 — Las reglas de las clases propias (las que no salen de una SP de NosTale), en codigo.

Oriol (07/10/2026) pidio clases nuevas «con estos ejemplos y referencias del nostale»: cuatro bardos,
dos clerigos, dos paladines, una clase del rayo, un ingeniero, un mercader, un alquimista, un druida,
un necromago, un domador de bestias, un tarotista, dos chamanes y dos piratas de Vinclap, con una
armadura de tier 2 cada una. Tienen la misma forma que las SP (ficha, 7 cartas + 3 de Creador,
pasiva, un set entero que la clase no lleva) y las escribe el mismo importador
(importar_clases_sp.py); lo que cambia es de donde salen las cifras: no hay ficha de NosTale, asi que
los dados y las rondas se escriben tal cual, dentro del presupuesto de planificacion/
CANON_CLASES_PROPIAS.md (§3), que esta sacado de las 44 SP ya hechas.

Los datos estan en cp_<grupo>.py (cp_bardos.py...). Las piezas que no dependen de cifras (el set, el
arma normal, la ficha por rol, los nombres de set) son las de sp_reglas.py.
"""
import re

from elementos import ELEMENTOS as _ELEMENTOS_DEL_JUEGO
from sp_reglas import (CONTRARRESTA, DOS_MANOS, PLANTILLA_ROL, TIRADAS, arma_comun, condicion, elige_una,  # noqa: F401
                       etiqueta_set, pendiente, pieza, si, siempre, slug)

# Los dados que puede llevar una carta propia (§3): los mismos escalones que las SP.
DADOS = ("1d6", "2d6", "3d6", "4d6")


def dano(dados):
    """El dano de la carta: «2d6» (§3, el presupuesto por objetivo y por lo que lleva la carta)."""
    if dados not in DADOS:
        raise ValueError(f"dados fuera de escala: {dados} ({DADOS})")
    return f"dano:{dados}"


def cura(dados):
    """La cura: «1d6» a todos o «2d6» a uno (§3); «dano_causado» para robar vida."""
    if dados not in DADOS[:2] + ("dano_causado",):
        raise ValueError(f"cura fuera de escala: {dados}")
    return f"cura:{dados}"


def _rondas(rondas):
    if rondas != "combate" and not (isinstance(rondas, int) and 1 <= rondas <= 3):
        raise ValueError(f"rondas fuera de escala: {rondas} (1-3 o 'combate')")
    return rondas


def estado(rondas, rasgo):
    """Un estado sobre los objetivos de la carta: 1-3 rondas o «combate» (§3)."""
    return f"estado:{_rondas(rondas)}:{rasgo}"


def propio(rondas, rasgo):
    """Lo gana quien juega la carta (en una reaccion, al saltar: 2 rondas, §4 del canon de las SP)."""
    return f"propio:{_rondas(rondas)}:{rasgo}"


def cond(ident, rondas=None):
    """Una condicion del catalogo (conditions.json) con sus rondas."""
    return f"condicion:{ident}" + (f":{_rondas(rondas)}" if rondas is not None else "")


def azar_una(*opciones):
    """CP-01: como elige_una, pero la opcion sale al azar al empezar el combate (la lectura del tarot de la
    Cartomante): una fila «Echar las cartas» y la suerte decide."""
    return "azar_una:" + ",".join(opciones)


def invoca(rondas, ident):
    """Una invocacion de summons.json (`invocaciones` en los datos): 1-3 rondas, o «combate» solo para el
    companero de la clase (el Domador)."""
    return f"invoca:{_rondas(rondas)}:{ident}"


def aturdido(rondas=1, nombre="Aturdido"):
    """No ataca y se defiende con desventaja (como el aturdimiento de las SP)."""
    return [(f"{nombre}: no ataca.", estado(rondas, "incapacitado")),
            (f"{nombre}: se defiende con desventaja.", estado(rondas, "desventaja:defensa"))]


def atrae():
    """Provocar: el enemigo te ataca a ti 1 turno."""
    return ("Te ataca a ti 1 turno.", "estado:1:provocado:origen")


def carta(ref, nombre, target, desc, efectos, salvacion=None, elemento=None, accion="accion_principal",
          tira=None):
    """Una carta propia. ref: de donde sale la idea, corto («NosTale: Pirata, Fuego de cañón», «folclore:
    la Santa Compaña», «propia»): va a `fuente` del catalogo. efectos: [(frase de la carta, gancho)]."""
    return {"h": ref, "nombre": nombre, "target": target, "desc": desc, "efectos": efectos,
            "salvacion": salvacion, "elemento": elemento, "accion": accion, "tira": tira}


# Los elementos de las cartas son los del juego (tools/elementos.py; Oriol, 07/10: «no estamos limitados
# a los 4 elementos del nostale aqui»): de lo que esta hecha la carta (el rayo, Rayo; el hielo, Hielo; la
# sangre, Sangre...). Lo fisico, sin elemento.
ELEMENTOS = set(_ELEMENTOS_DEL_JUEGO) | {None}


def revisa_presupuesto(clase):
    """Lo que el importador no mira y el canon de las clases propias pide (§3): ningun dado ni ronda
    fuera de escala, las de Creador con lo suyo y el arranque jugable. Devuelve los avisos."""
    avisos = []
    for grupo, cartas in (("cartas", clase["cartas"]), ("creador", clase["creador"])):
        for k in cartas:
            if k["elemento"] not in ELEMENTOS:
                avisos.append(f"{clase['id']} / {k['nombre']}: elemento {k['elemento']}")
            for _f, g in k["efectos"]:
                m = re.match(r"^dano:(\d)d6$", g)
                if m and grupo == "cartas" and int(m.group(1)) > 3:
                    avisos.append(f"{clase['id']} / {k['nombre']}: 4d6 es de Creador")
                if g.startswith(("revive", "turno_extra")) and grupo == "cartas":
                    avisos.append(f"{clase['id']} / {k['nombre']}: {g} es de Creador")
    return avisos
