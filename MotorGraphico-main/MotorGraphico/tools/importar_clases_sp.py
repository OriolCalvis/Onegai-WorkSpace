#!/usr/bin/env python3
"""SP-02 — Las SP de NosTale como clases nuevas de MotorGraphico.

Oriol (07/10/2026): «metete de lleno en las clases nuevas»: cada SP, una clase
(ficha, cartas, pasiva y su set) que se elige al crear personaje; clase a clase
y completa; las cartas, jugables donde se pueda (lo que el combate no cubre
queda [pendiente] y bloqueado, como en H2-06); y el Espadachin entero.

Las reglas de la adaptacion estan en planificacion/CANON_SP_CLASES.md y, en
codigo, en sp_reglas.py; los datos de cada SP, en sp_espadachin.py (cada carta
dice de que habilidad de NosTale sale: su H en la enciclopedia de Oriol). Este
script los escribe en los catalogos: upsert por id, no borra nada. Se corrige
un texto y se vuelve a ejecutar.

Escribe classes.json, skills.json, passives.json y equipment.json (el set de
cada SP y alguna arma normal que faltaba), y el objeto item_<id> de cada
objeto nuevo en assets/objects/libreria_armas.json y libreria_completa.json
(el puente que piden el inventario y la tienda).

Oriol (07/10, sobre el aspecto): «no tienen que llevar los equipos estos cada
una de las clases, son magos/guerreros/arqueros/etc, no deben llevar el set
entero» y «de momento un set entero por sp». La clase empieza con el kit de
su arquetipo (Game::KitBasico) y un arma normal de su tipo; el set de su SP
son objetos sueltos (raros, tier 2) que cualquiera puede ponerse.

    python3 tools/importar_clases_sp.py          # escribe
    python3 tools/importar_clases_sp.py --ver    # solo comprueba y cuenta
"""
import argparse
import importlib
import importlib.util
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from sp_reglas import PLANTILLA_ROL, RECURSO, SLOT_PUENTE, TIRADAS, etiqueta_set, slug  # noqa: E402

# Las clases base y su modulo de datos (sp_<base>.py, con una lista en MAYUSCULAS). Las que aun
# no tienen modulo se saltan.
BASES = [("espadachin", "sp_espadachin", "ESPADACHIN"), ("arquero", "sp_arquero", "ARQUERO"),
         ("mago", "sp_mago", "MAGO"), ("artista_marcial", "sp_artista_marcial", "ARTISTA_MARCIAL"),
         # CP-01: las clases propias (planificacion/CANON_CLASES_PROPIAS.md), con la misma forma
         ("bardos", "cp_bardos", "BARDOS"), ("fe", "cp_fe", "FE"), ("ingenio", "cp_ingenio", "INGENIO"),
         ("naturaleza", "cp_naturaleza", "NATURALEZA"),
         ("noche_y_tormenta", "cp_noche_y_tormenta", "NOCHE_Y_TORMENTA")]


def es_propia(c):
    """CP-01: una clase propia no sale de una SP (no tiene ficha de NosTale)."""
    return "nostale" not in c


def clases_de(base=None):
    """[(clase base, datos de una SP)] de los modulos que existen."""
    out = []
    for nombre, modulo, lista in BASES:
        if base and nombre != base:
            continue
        if importlib.util.find_spec(modulo) is None:
            continue
        out += [(nombre, c) for c in getattr(importlib.import_module(modulo), lista)]
    return out


def clases_de_fichero(ruta):
    """Las SP de un borrador (cualquier dict con id, cartas y creador)."""
    spec = importlib.util.spec_from_file_location("borrador_sp", ruta)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return [("borrador", v) for v in vars(mod).values()
            if isinstance(v, dict) and {"id", "cartas", "creador"} <= set(v)]

ROOT = Path(__file__).resolve().parent.parent
CAT = ROOT / "assets/catalogs"
OBJ = ROOT / "assets/objects"


def arranque(c, ids):
    """Las cartas de arranque: las tres primeras de la tarjeta, salvo que la SP diga otras
    («arranque»: nombres de sus cartas)."""
    if not c.get("arranque"):
        return ids[:3]
    por_nombre = {k["nombre"]: f"{c['id']}_{slug(k['nombre'])}" for k in c["cartas"] + c["creador"]}
    faltan = [n for n in c["arranque"] if n not in por_nombre]
    if faltan or len(c["arranque"]) != 3:
        raise SystemExit(f"{c['id']}: arranque con cartas que no tiene o que no son tres: {faltan}")
    return [por_nombre[n] for n in c["arranque"]]


# --- comprobaciones ---------------------------------------------------------------------------

VERBOS = {"dano", "cura", "ataque_arma", "ataques_arma", "estado", "condicion", "escudo", "turno_extra",
          "devuelve_carta", "invoca", "bloquea", "dados_extra", "revive", "defiende_aliado", "aliado_ataca",
          "pendiente", "pendiente_parcial", "requiere_posiciones", "coste_desgarro", "propio", "limpia",
          "elige_una", "si", "recupera_carta", "siempre", "azar_una"}
RASGOS = {"desventaja", "ventaja", "ventaja_contra", "bono_ca", "penal_ca", "incapacitado", "critico_contra",
          "dano_por_turno", "absorbe_por_turno", "bono_dano_arma", "bono_dano_contra", "bono_dano_melee",
          "refleja", "provocado", "desarmado", "bono_cura", "penal_dados"}


def revisa_gancho(g, donde, condiciones):
    p = g.split(":")
    if p[0] not in VERBOS:
        raise SystemExit(f"{donde}: verbo desconocido en «{g}»")
    if p[0] in ("estado", "propio", "si") and (len(p) < 3 or p[2] not in RASGOS):
        raise SystemExit(f"{donde}: rasgo desconocido en «{g}»")
    if p[0] == "siempre" and p[1] not in RASGOS:
        raise SystemExit(f"{donde}: rasgo desconocido en «{g}»")
    if p[0] == "condicion" and p[1] not in condiciones:
        raise SystemExit(f"{donde}: la condicion «{p[1]}» no esta en conditions.json")


def revisa_carta(clase, c, condiciones):
    """Los ganchos existen, y las cifras de los ganchos estan en el texto de la carta."""
    donde = f"{clase} / {c['nombre']}"
    for _frase, g in c["efectos"]:
        revisa_gancho(g, donde, condiciones)
        m = re.match(r"^(dano|cura):(\d+d\d+)$", g)
        if m and m.group(2) not in c["desc"]:
            raise SystemExit(f"{donde}: el texto no dice {m.group(2)} ({g})")
        m = re.match(r"^(estado|propio):(\d+):", g)
        if m:
            n = int(m.group(2))
            dicho = f"{n} turno" if n == 1 else f"{n} turnos"
            if dicho not in c["desc"]:
                raise SystemExit(f"{donde}: el texto no dice «{dicho}» ({g})")
        if g.startswith(("estado:combate:", "propio:combate:")) and "final del combate" not in c["desc"]:
            raise SystemExit(f"{donde}: el texto no dice «hasta el final del combate» ({g})")
    if c["accion"] == "reaccion" and c["target"] not in ("self", "single_ally"):
        raise SystemExit(f"{donde}: una reaccion se prepara para ti o para un aliado")
    if c.get("tira") not in TIRADAS | {None}:
        raise SystemExit(f"{donde}: no se tira con {c.get('tira')} ({sorted(TIRADAS)})")


def revisa_set(clase, equipo_ids):
    """Un set entero: el arma y una pieza por ranura de armadura; y el arma inicial existe."""
    slots = sorted(e["slot"] for e in clase["set"])
    if slots != sorted(["MainHand", "Torso", "Head", "Legs", "Feet"]):
        raise SystemExit(f"{clase['id']}: el set tiene que ser arma, cabeza, torso, piernas y pies: {slots}")
    nuevas = {e["id"] for e in clase.get("armas_nuevas", [])}
    if clase["arma_inicial"] not in equipo_ids | nuevas:
        raise SystemExit(f"{clase['id']}: el arma inicial {clase['arma_inicial']} no esta en equipment.json")
    if clase["arma_inicial"] in {e["id"] for e in clase["set"]}:
        raise SystemExit(f"{clase['id']}: la clase no empieza con su set (Oriol, 07/10)")


# --- escribir ---------------------------------------------------------------------------------

def sangria(ruta):
    with open(ruta, encoding="utf-8") as f:
        f.readline()
        segunda = f.readline()
    return max(1, len(segunda) - len(segunda.lstrip(" ")))


def cargar(ruta):
    with open(ruta, encoding="utf-8") as f:
        return json.load(f)


def guardar(ruta, doc):
    """Con la sangria del fichero y su final de linea (o sin el): el diff solo lleva lo nuevo."""
    indent = sangria(ruta)
    salto = ruta.read_bytes().endswith(b"\n")
    with open(ruta, "w", encoding="utf-8") as f:
        json.dump(doc, f, ensure_ascii=False, indent=indent)
        if salto:
            f.write("\n")


def upsert(entries, entry):
    for i, e in enumerate(entries):
        if e.get("id") == entry["id"]:
            entries[i] = {**e, **entry}
            return "actualizada"
    entries.append(entry)
    return "nueva"


def habilidad(clase, c, creador):
    sid = f"{clase['id']}_{slug(c['nombre'])}"
    elemento = [c["elemento"]] if c["elemento"] else []
    pendiente = any(g.startswith("pendiente:") for _f, g in c["efectos"])
    return {
        "id": sid, "name": c["nombre"], "type": "skill",
        "tier": 2 if creador else 1, "rarity": "rare" if creador else "common",
        "description": c["desc"], "flavorText": "",
        "classTags": [clase["id"]], "roleTags": [],
        # «estandar»: la que puede devolver a la mano una carta como Convertidor temporal
        "tags": [clase["id"]] + (["creador"] if creador else ["estandar"]) + elemento,
        "mechanicTags": elemento + (["Creador"] if creador else []),
        "actionType": c["accion"], "target": c["target"], "range": "short", "duration": "instant",
        "cost": {"amount": 0, "resource": "none"},
        "recovery": "descanso_largo" if creador else "descanso_corto",
        # §7: con que se tira (la ficha de NosTale: ataque magico -> INT); sin nada, DES
        "castingStat": c.get("tira") or clase.get("tira"), "defenseStat": c["salvacion"],
        "magnitudeByDegree": None,
        "effect": {"description": c["desc"], "scaling": "none"},
        "effects": [{"description": f, "mechanicHook": g} for f, g in c["efectos"]],
        "requiredStats": {}, "requiredTags": [], "incompatibleTags": [],
        "limitations": (["[pendiente] efecto sin definir"] if pendiente else []),
        "evolvesInto": None, "upgradePath": [],
        "fuente": fuente_de(clase, c),
    }


def fuente_de(clase, c=None):
    """De donde sale: la habilidad de NosTale (su H) o, en una clase propia (CP-01), la referencia."""
    if not es_propia(clase):
        return f"NosTale · {clase['nostale']}" + (f" · {c['h']} {c['nombre']}" if c else "")
    ref = c.get("h") if c else None
    return f"Propia · {clase['name']}" + (f" · {ref}" if ref and ref != "propia" else "")


def puente(e):
    """El objeto item_<id> del catalogo de objetos (como tools/convertir_libreria_dnd.py)."""
    precio = {"common": 50, "uncommon": 150, "rare": 400}.get(e["rarity"], 100)
    peso = {"light": "ligera", "medium": "media", "heavy": "pesada"}[e["weightCategory"]]
    bonos = dict(e["statBonuses"], health=0, armor=0)
    return {
        "id": f"item_{e['id']}", "name": e["name"], "category": "pickup", "spriteId": -1,
        "blocksMovement": False, "interactable": True, "price": precio,
        "pickup": {"effect": "none", "power": 0},
        "_sourceData": {"_slot": SLOT_PUENTE[e["slot"]], "_tier": e["tier"], "_rarity": e["rarity"],
                        "_statBonuses": bonos, "_weightCategory": peso, "_grantedTags": e["grantedTags"],
                        "_flavorText": e["flavorText"]},
    }


def revisa_clase(c, condiciones, equipo_ids):
    """Todo lo que se comprueba de una SP antes de escribirla."""
    cid = c["id"]
    if len(c["cartas"]) != 7 or len(c["creador"]) != 3:
        raise SystemExit(f"{cid}: hacen falta 7 cartas y 3 de Creador")
    nombres = [k["nombre"] for k in c["cartas"] + c["creador"]]
    if len(set(nombres)) != len(nombres):
        raise SystemExit(f"{cid}: dos cartas con el mismo nombre")
    for k in c["cartas"] + c["creador"]:
        revisa_carta(cid, k, condiciones)
    revisa_set(c, equipo_ids)
    for _f, g in c["pasiva"][2]:
        revisa_gancho(g, f"{cid} / pasiva", condiciones)
    if c["role"] not in PLANTILLA_ROL:
        raise SystemExit(f"{cid}: rol desconocido {c['role']}")
    if c.get("tira") not in TIRADAS | {None}:
        raise SystemExit(f"{cid}: no se tira con {c.get('tira')} ({sorted(TIRADAS)})")
    for inv in c.get("invocaciones", []):
        if not {"id", "name", "health", "stats", "attacks"} <= set(inv):
            raise SystemExit(f"{cid}: invocacion incompleta {inv.get('id')}")
    if c.get("arranque"):
        arranque(c, [f"{cid}_{slug(k['nombre'])}" for k in c["cartas"] + c["creador"]])
    if es_propia(c):  # CP-01: el presupuesto de las clases propias
        if not c.get("familia"):
            raise SystemExit(f"{cid}: una clase propia dice su familia (bardo, clerigo...)")
        from cp_reglas import revisa_presupuesto
        avisos = revisa_presupuesto(c)
        if avisos:
            raise SystemExit("\n".join(avisos))
        ids = [f"{cid}_{slug(k['nombre'])}" for k in c["cartas"] + c["creador"]]
        for sid in arranque(c, ids):
            k = next(k for k in c["cartas"] + c["creador"] if f"{cid}_{slug(k['nombre'])}" == sid)
            if k in c["creador"] or any(g.startswith("pendiente:") for _f, g in k["efectos"]):
                raise SystemExit(f"{cid}: el arranque tiene que ser jugable y estandar: {k['nombre']}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ver", action="store_true", help="solo comprueba y cuenta")
    ap.add_argument("--base", help="solo una clase base (espadachin, arquero...)")
    ap.add_argument("--borrador", help="comprueba las SP de un fichero .py (no escribe nada)")
    args = ap.parse_args()

    rutas = {n: CAT / f"{n}.json" for n in ("classes", "skills", "passives", "equipment", "summons")}
    rutas["armas"] = OBJ / "libreria_armas.json"
    rutas["completa"] = OBJ / "libreria_completa.json"
    docs = {n: cargar(r) for n, r in rutas.items()}
    entradas = {n: docs[n]["entries"] for n in ("classes", "skills", "passives", "equipment", "summons")}
    condiciones = {c["id"] for c in cargar(CAT / "conditions.json")["entries"]}
    equipo_ids = {e["id"] for e in entradas["equipment"]}

    if args.borrador:
        for _base, c in clases_de_fichero(args.borrador):
            revisa_clase(c, condiciones, equipo_ids)
            print(f"{c['id']}: bien (7 cartas + 3 de Creador, pasiva, set)")
        return

    resumen = []
    for base, c in clases_de(args.base):
        cid = c["id"]
        revisa_clase(c, condiciones, equipo_ids)

        pas_nombre, pas_desc, pas_efectos = c["pasiva"]
        pas_id = f"{cid}_{slug(pas_nombre)}"
        upsert(entradas["passives"], {
            "id": pas_id, "name": pas_nombre, "type": "passive", "tier": 1, "rarity": "common",
            "description": pas_desc, "flavorText": "", "classTags": [cid], "trigger": "Constante",
            "effect": {"description": pas_desc, "scaling": "none"}, "limitations": [], "synergyTags": [],
            "unique": True, "effects": [{"description": f, "mechanicHook": g} for f, g in pas_efectos],
            "fuente": fuente_de(c),
        })
        ids = []
        for k in c["cartas"]:
            s = habilidad(c, k, creador=False)
            upsert(entradas["skills"], s)
            ids.append(s["id"])
        for k in c["creador"]:
            s = habilidad(c, k, creador=True)
            upsert(entradas["skills"], s)
            ids.append(s["id"])
        for inv in c.get("invocaciones", []):  # el Lobo solar del Cazasoles...
            upsert(entradas["summons"], dict({"type": "summon", "tier": 1, "rarity": "uncommon", "movement": 6,
                                              "control": "jugador", "flavorText": ""}, **inv))
        nuevos = list(c.get("armas_nuevas", []))
        # «Set del <SP>», o del nombre corto que dan sus piezas (set_de: el Cañonero)
        etiqueta = etiqueta_set(c.get("set_de", c["name"]))
        nuevos += [dict(e, grantedTags=e["grantedTags"] + [etiqueta]) for e in c["set"]]
        for e in nuevos:
            upsert(entradas["equipment"], e)
            for n in ("armas", "completa"):
                upsert(docs[n]["objects"], puente(e))
        estado_clase = upsert(entradas["classes"], {
            "id": cid, "type": "class", "tier": 1, "rarity": "uncommon",
            "name": c["name"], "description": c["description"], "flavorText": "",
            "primaryStat": c["primaryStat"], "secondaryStat": c["secondaryStat"], "role": c["role"],
            **PLANTILLA_ROL[c["role"]],
            "primaryResource": RECURSO, "secondaryResource": None,
            "allowedEquipmentTags": [], "restrictedTags": [], "specializations": [],
            "startingEquipment": [c["arma_inicial"]],
            "startingCards": {"skills": arranque(c, ids), "learnableSkills": ids, "passives": [pas_id], "spells": []},
            **({"propia": {"familia": c["familia"], "elemento": c["elemento"], "deidad": c.get("deidad"),
                            "referencias": c.get("referencias", [])}} if es_propia(c) else
               {"especialista": {"claseBase": base, "sp": c["sp"], "elemento": c["elemento"],
                                 "nostale": c["nostale"]}}),
        })
        resumen.append(f"{cid}: {estado_clase}, 7 cartas + 3 de Creador, pasiva {pas_id}, "
                       f"set de {len(c['set'])} piezas")

    if args.ver:
        print("\n".join(resumen))
        return
    for n, r in rutas.items():
        guardar(r, docs[n])
    print("\n".join(resumen))


if __name__ == "__main__":
    sys.exit(main())
