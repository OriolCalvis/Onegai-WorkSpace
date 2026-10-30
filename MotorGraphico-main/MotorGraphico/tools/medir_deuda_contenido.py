#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""AV12-05 - Mide la deuda de contenido aguas arriba. Solo lectura.

El plan decia: 202 hechizos huerfanos, 432 monstruos con vida 0, 59 clases
sin hechizos. Nadie lo habia vuelto a medir. Este script lo mide con la
misma regla que el C++, no con la que parece razonable:

  * VIDA de un monstruo = lo que lee MonsterDefinition (CatalogLoadersCriaturas.h):
    defenses.maxHealth, si no derived.vida, si no maxHealth / max_health /
    health en la raiz, si no 0.
  * CARTAS de una clase = lo que lee ClassDefinition (CatalogLoadersPersonaje.h):
    startingCards.skills + startingCards.spells (iniciales) y
    startingCards.learnableSkills (aprendibles). CharacterCreation las deja
    todas en knownSkillIds.
  * CARTAS de un monstruo = su skillIds.
  * HECHIZO = una entrada de spells.json; CARTA = una de skills.json.

Cuatro medidas (las del plan) y lo que cuelga de cada una:
  1. cartas y hechizos que ninguna clase ni monstruo usa;
  2. monstruos con vida 0, sin CA o sin ataque, por tier;
  3. clases sin hechizos o con menos cartas que el minimo (handLimit) de su tier;
  4. referencias rotas (un id que no esta en la familia de catalogo que le toca).

USO
  python3 tools/medir_deuda_contenido.py                 # resumen
  python3 tools/medir_deuda_contenido.py --md            # tablas en Markdown
  python3 tools/medir_deuda_contenido.py --json          # todo, para otra herramienta
  python3 tools/medir_deuda_contenido.py --proponer DIR  # escribe propuestas en DIR
  python3 tools/medir_deuda_contenido.py --catalogos CARPETA --reglas FICHERO --aventuras CARPETA

NUNCA escribe en assets/catalogs/. --proponer se niega si DIR es esa carpeta.
"""

import argparse
import copy
import csv
import json
import os
import sys
from collections import Counter, defaultdict

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CATALOGOS = os.path.join(RAIZ, "assets", "catalogs")
REGLAS = os.path.join(RAIZ, "assets", "rules", "tier_rules.json")
AVENTURAS = os.path.join(RAIZ, "assets", "adventures")

# Ajustada a los datos de hoy (AV6-06): vida = base + paso * tier, segun el rango.
# Acierta en 430 de 432 fichas; las dos que no, estan en `formula.vida_fuera`.
FORMULA_VIDA = {"criatura": (8, 4), "elite": (16, 6), "jefe": (30, 10)}
BONO_CA_RANGO = {"criatura": 0, "elite": 1, "jefe": 2}  # CA = 10 + DES + bono

# Catalogos que nadie carga (grep en src/ e include/): no cuentan en las referencias.
IGNORADOS = {"locations_trae_test", "MANIFIESTO"}

CARTAS = ("skills", "spells")
EQUIPO = ("equipment", "consumables")
# (catalogo, ruta, familias donde tiene que existir el id). Solo campos que son
# ids de verdad: los de texto libre, etiquetas e ids locales se dejan fuera (ver
# DEUDA_DE_CONTENIDO.md, "Lo que no se cuenta").
REFERENCIAS = [
    ("adventures", "/deidadIds[]", ("deities",)),
    ("adventures", "/enemigoIds[]", ("monsters", "summons")),
    ("adventures", "/combates[]/enemigoIds[]", ("monsters",)),
    ("adventures", "/bossId", ("monsters",)),
    ("adventures", "/monsters[]", ("monsters",)),
    ("adventures", "/villanoIds[]", ("monsters",)),
    ("adventures", "/references/enemyIds[]", ("monsters",)),
    ("adventures", "/lootIds[]", ("loot_tables",)),
    ("adventures", "/lootTables[]", ("loot_tables",)),
    ("adventures", "/references/lootTableId", ("loot_tables",)),
    ("adventures", "/trampaIds[]", ("traps",)),
    ("adventures", "/npcIds[]", ("npcs",)),
    ("adventures", "/npcs[]", ("npcs",)),
    ("adventures", "/references/npcIds[]", ("npcs",)),
    ("backgrounds", "/lore/deities[]", ("deities",)),
    ("backgrounds", "/skillProficiencyIds[]", CARTAS),
    ("classes", "/startingCards/skills[]", CARTAS),
    ("classes", "/startingCards/spells[]", CARTAS),
    ("classes", "/startingCards/learnableSkills[]", CARTAS),
    ("classes", "/startingCards/passives[]", ("passives",)),
    ("classes", "/startingCards/passive", ("passives",)),
    ("classes", "/startingEquipment[]", EQUIPO),
    ("classes", "/propia/deidad", ("deities",)),
    ("deities", "/grantedSpells[]", ("spells",)),
    ("deities", "/grantedPassiveIds[]", ("passives",)),
    ("encounter_tables", "/entries[]/refId", ("monsters",)),
    ("encounter_tables", "/lootTableId", ("loot_tables",)),
    ("equipment", "/deity", ("deities",)),
    ("equipment", "/linkedSkill", CARTAS),
    ("equipment", "/awakening/unlocks", CARTAS),
    ("locations", "/deityIds[]", ("deities",)),
    ("loot_tables", "/drops[]/item", EQUIPO),
    ("loot_tables", "/entries[]/refId", EQUIPO),
    ("monsters", "/skillIds[]", CARTAS),
    ("monsters", "/passiveId", ("passives",)),
    ("monsters", "/conditionsInflicted[]", ("conditions",)),
    ("monsters", "/loot[]", EQUIPO),
    ("monsters", "/lootIds[]", EQUIPO),
    ("monsters", "/lootTable", ("loot_tables",)),
    ("monsters", "/lootTable/drops[]", EQUIPO),
    ("npcs", "/classId", ("classes",)),
    ("npcs", "/raceId", ("races",)),
    ("passives", "/classTags[]", ("classes", "skills")),
    ("pregen_characters", "/claseId", ("classes",)),
    ("pregen_characters", "/doteIds[]", ("feats",)),
    ("pregen_characters", "/equipoIds[]", EQUIPO),
    ("pregen_characters", "/habilidadIds[]", CARTAS),
    ("pregen_characters", "/hechizoIds[]", CARTAS),
    ("pregen_characters", "/razaId", ("races",)),
    ("pregen_characters", "/transfonsId", ("backgrounds",)),  # sic: asi se llama la clave
    ("races", "/compatibleDeities[]", ("deities",)),
    ("races", "/unlocks[]", CARTAS + ("passives", "traits")),
    ("skills", "/evolvesInto", CARTAS),
    ("skills", "/classTags[]", ("classes", "skills")),
    ("spells", "/deity", ("deities",)),
    ("spells", "/evolvesInto", CARTAS),
    ("spells", "/classTags[]", ("classes",)),
    ("summons", "/summonedBy", CARTAS),
    ("summons", "/attackSkillIds[]", CARTAS),
    ("summons", "/passiveIds[]", ("passives",)),
    ("traps", "/effect/condicionInfligida", ("conditions",)),
]
# Las seis clases que Oriol fijo el 28/09 como tono de referencia (AV12-05, metodo).
CLASES_DEL_METODO = ("monje_del_despertar", "senor_de_las_armas", "bardo_de_las_cuatro_estaciones",
                     "el_apostata", "furia_de_los_creadores", "mago_chasqueador")
PREFIJOS = ("monster_", "skill_", "spell_", "class_", "equip_", "item_", "passive_", "deity_")


# ---------------------------------------------------------------- carga ----
def _lee(ruta):
    with open(ruta, encoding="utf-8") as f:
        return json.load(f)


def carga(carpeta):
    """nombre -> lista de entradas. Un fichero que falta es una lista vacia."""
    cat, ausentes = {}, []
    for ruta in sorted(os.listdir(carpeta)) if os.path.isdir(carpeta) else []:
        if not ruta.endswith(".json"):
            continue
        nombre = ruta[:-5]
        if nombre in IGNORADOS:
            continue
        d = _lee(os.path.join(carpeta, ruta))
        es = d.get("entries") if isinstance(d, dict) else d
        cat[nombre] = [e for e in es if isinstance(e, dict)] if isinstance(es, list) else []
    for n in ("skills", "spells", "classes", "monsters"):
        if n not in cat:
            cat[n] = []
            ausentes.append(n)
    return cat, ausentes


def lee_hand_limit(ruta):
    try:
        return {int(t["tier"]): int(t["handLimit"]) for t in _lee(ruta)["tiers"]}
    except (OSError, ValueError, KeyError, TypeError):
        return {}


def lee_skill_checks(carpeta):
    """ids de habilidad que piden las aventuras en sus skillCheck."""
    ids = Counter()

    def anda(o):
        if isinstance(o, dict):
            sc = o.get("skillCheck")
            if isinstance(sc, dict) and isinstance(sc.get("skillId"), str):
                ids[sc["skillId"]] += 1
            for v in o.values():
                anda(v)
        elif isinstance(o, list):
            for v in o:
                anda(v)

    if carpeta and os.path.isdir(carpeta):
        for f in sorted(os.listdir(carpeta)):
            if f.endswith(".json"):
                try:
                    anda(_lee(os.path.join(carpeta, f)))
                except (OSError, ValueError):
                    pass
    return ids


def _cadenas(v):
    if isinstance(v, str):
        return [v]
    return [x for x in v if isinstance(x, str)] if isinstance(v, list) else []


def _int(v):
    return isinstance(v, int) and not isinstance(v, bool)


def camina(o, ruta=""):
    if isinstance(o, dict):
        for k, v in o.items():
            yield from camina(v, ruta + "/" + k)
    elif isinstance(o, list):
        for v in o:
            yield from camina(v, ruta + "[]")
    elif isinstance(o, str):
        yield ruta, o


# ---------------------------------------------- lo que lee el cargador ----
def vida_cargada(e):
    d = e.get("defenses") if isinstance(e.get("defenses"), dict) else {}
    dv = e.get("derived") if isinstance(e.get("derived"), dict) else {}
    for v in (d.get("maxHealth"), dv.get("vida"), e.get("maxHealth"), e.get("max_health"), e.get("health")):
        if _int(v):
            return v
    return 0


def tiene_ca(e):
    d = e.get("defenses") if isinstance(e.get("defenses"), dict) else {}
    dv = e.get("derived") if isinstance(e.get("derived"), dict) else {}
    return _int(d.get("ca")) or _int(dv.get("ca")) or _int(e.get("ca")) or _int(e.get("ac"))


def cartas_de_clase(c):
    """(iniciales, aprendibles) como las deja ClassDefinition."""
    sc = c.get("startingCards")
    if isinstance(sc, dict):
        ini = _cadenas(sc.get("skills")) + _cadenas(sc.get("spells"))
        apr = _cadenas(sc.get("learnableSkills"))
    else:
        ini, apr = _cadenas(sc), []
    if not ini:
        ini = _cadenas(c.get("startingSkillIds"))
    return ini, apr


def formula_vida(tier, rango):
    if rango not in FORMULA_VIDA or not _int(tier):
        return None
    base, paso = FORMULA_VIDA[rango]
    return base + paso * tier


# ------------------------------------------------------------- medidas ----
def mide_cartas(cat, usadas_clase, usadas_monstruo, extra_fondos, extra_aventura):
    res = {}
    deidades = {e.get("id") for e in cat.get("deities", [])}
    enlaces = defaultdict(set)  # id de carta -> fuentes que la nombran sin ser clase/monstruo
    for d in cat.get("deities", []):
        for i in _cadenas(d.get("grantedSpells")) + _cadenas(d.get("prayerSkillIds")):
            enlaces[i].add("deidad")
    for q in cat.get("equipment", []):
        for i in _cadenas(q.get("linkedSkill")) + _cadenas(q.get("awakening", {}).get("unlocks") if isinstance(q.get("awakening"), dict) else None):
            enlaces[i].add("equipo")
    for p in cat.get("pregen_characters", []):
        for i in _cadenas(p.get("habilidadIds")) + _cadenas(p.get("hechizoIds")):
            enlaces[i].add("personaje")
    for r in cat.get("races", []):
        for i in _cadenas(r.get("unlocks")):
            enlaces[i].add("raza")
    for i in extra_fondos:
        enlaces[i].add("trasfondo")
    for i in extra_aventura:
        enlaces[i].add("aventura")
    clases = {c.get("id") for c in cat.get("classes", [])}
    for fam in CARTAS:
        es = cat.get(fam, [])
        ids = [e.get("id") for e in es]
        huer = [e for e in es if e.get("id") not in usadas_clase and e.get("id") not in usadas_monstruo]
        sin_tag = [e for e in es if not any(t in clases for t in _cadenas(e.get("classTags")))]
        por_tier = {}
        for e in es:
            t = e.get("tier") if _int(e.get("tier")) else 0
            tot, hu = por_tier.get(t, (0, 0))
            por_tier[t] = (tot + 1, hu + (1 if e in huer else 0))
        # una carta cuya evolucion se usa tambien esta alcanzable por el que evoluciona
        evol = {e.get("id"): _cadenas(e.get("evolvesInto")) for e in es}
        sin_enlace, con_enlace = [], Counter()
        for e in huer:
            i = e.get("id")
            fuentes = set(enlaces.get(i, ()))
            if any(i in evol.get(o, []) for o in usadas_clase):
                fuentes.add("evolucion")
            for f in fuentes:
                con_enlace[f] += 1
            if not fuentes:
                sin_enlace.append(i)
        r = {
            "total": len(ids),
            "huerfanas": len(huer),
            "huerfanas_ids": [e.get("id") for e in huer],
            "sin_classTags": len(sin_tag),
            "por_tier": {str(k): v for k, v in sorted(por_tier.items())},
            "con_enlace": dict(con_enlace),
            "sin_ningun_enlace": len(sin_enlace),
            "sin_ningun_enlace_ids": sin_enlace,
        }
        if fam == "spells":
            ce = Counter()
            for e in huer:
                d = e.get("deity")
                ce["sin deidad" if not isinstance(d, str) or not d else ("deidad existe" if d in deidades else "deidad falta")] += 1
            r["por_deidad"] = dict(ce)
            r["deidades_que_faltan"] = sorted({e["deity"] for e in huer if isinstance(e.get("deity"), str) and e["deity"] and e["deity"] not in deidades})
        res[fam] = r
    return res


def mide_monstruos(cat, hand_cap):
    mons = cat.get("monsters", [])
    por_tier = defaultdict(Counter)
    for e in mons:
        t = e.get("tier") if _int(e.get("tier")) else 0
        c = por_tier[t]
        c["fichas"] += 1
        v = vida_cargada(e)
        if v <= 0:
            c["vida0"] += 1
        if not tiene_ca(e):
            c["sin_ca"] += 1
        at = e.get("attacks")
        if not isinstance(at, list) or not at:
            c["sin_ataque"] += 1
        else:
            c["ataques"] += len(at)
            c["ataques_sin_cifra"] += sum(1 for a in at if isinstance(a, dict) and not _int(a.get("magnitude")) and not _int(a.get("damage")))
        if not _cadenas(e.get("skillIds")):
            c["sin_cartas"] += 1
        dv = e.get("derived") if isinstance(e.get("derived"), dict) else {}
        if _int(dv.get("vida")) and dv["vida"] != v:
            c["vida_distinta_de_derived"] += 1
        if t in hand_cap and v > hand_cap[t]:
            c["vida_sobre_healthCap"] += 1
    tot = Counter()
    for c in por_tier.values():
        tot.update(c)
    # ajuste de las formulas a los datos
    fit = Counter()
    vida_fuera, ca_fuera = [], []
    for e in mons:
        s = e.get("stats") if isinstance(e.get("stats"), dict) else {}
        d = e.get("defenses") if isinstance(e.get("defenses"), dict) else {}
        rango = e.get("rank")
        f = formula_vida(e.get("tier"), rango)
        v = vida_cargada(e)
        if f is not None:
            fit["vida_medidas"] += 1
            if v == f:
                fit["vida_acierta"] += 1
            else:
                vida_fuera.append({"id": e.get("id"), "tier": e.get("tier"), "rango": rango, "vida": v, "formula": f})
        if rango in BONO_CA_RANGO and _int(d.get("ca")) and _int(s.get("DES")):
            fit["ca_medidas"] += 1
            if d["ca"] == 10 + s["DES"] + BONO_CA_RANGO[rango]:
                fit["ca_acierta"] += 1
            else:
                ca_fuera.append({"id": e.get("id"), "ca": d["ca"], "formula": 10 + s["DES"] + BONO_CA_RANGO[rango]})
        for clave, stat in (("defMental", "CAR"), ("resFis", "CON")):
            if _int(d.get(clave)) and _int(s.get(stat)):
                fit[clave + "_medidas"] += 1
                fit[clave + "_acierta"] += d[clave] == 10 + s[stat]
        if _int(d.get("precMag")):
            fit["precMag_medidas"] += 1
            fit["precMag_es_10"] += d["precMag"] == 10
    return {
        "total": len(mons),
        "vida0": tot["vida0"],
        "sin_ca": tot["sin_ca"],
        "sin_ataque": tot["sin_ataque"],
        "sin_cartas": tot["sin_cartas"],
        "ataques": tot["ataques"],
        "ataques_sin_cifra": tot["ataques_sin_cifra"],
        "vida_distinta_de_derived": tot["vida_distinta_de_derived"],
        "vida_sobre_healthCap": tot["vida_sobre_healthCap"],
        "por_tier": {str(t): dict(c) for t, c in sorted(por_tier.items())},
        "formula": {"vida": {k: list(v) for k, v in FORMULA_VIDA.items()}, "bono_ca": BONO_CA_RANGO,
                    "ajuste": dict(fit), "vida_fuera": vida_fuera, "ca_fuera": ca_fuera},
    }


def mide_clases(cat, hand):
    es = cat.get("classes", [])
    spell_ids = {e.get("id") for e in cat.get("spells", [])}
    por_tag = defaultdict(set)
    for s in cat.get("spells", []):
        for t in _cadenas(s.get("classTags")):
            por_tag[t].add(s.get("id"))
    filas = []
    for c in es:
        ini, apr = cartas_de_clase(c)
        todas = list(dict.fromkeys(ini + apr))
        tier = c.get("tier") if _int(c.get("tier")) else 1
        minimo = hand.get(tier)
        filas.append({
            "id": c.get("id"),
            "jugable": c.get("playable") if isinstance(c.get("playable"), bool) else True,
            "tier": tier,
            "cartas": len(todas),
            "minimo": minimo,
            "hechizos": sum(1 for i in todas if i in spell_ids),
            "hechizos_por_classTags": len(por_tag.get(c.get("id"), ())),
            "pocas_cartas": minimo is not None and len(todas) < minimo,
        })
    por_tier = defaultdict(Counter)
    for f in filas:
        for clave, cond in (("clases", True), ("sin_hechizos", f["hechizos"] == 0),
                            ("pocas_cartas", f["pocas_cartas"]),
                            ("jugables", f["jugable"]),
                            ("jugables_sin_hechizos", f["jugable"] and f["hechizos"] == 0),
                            ("jugables_pocas_cartas", f["jugable"] and f["pocas_cartas"])):
            por_tier[f["tier"]][clave] += 1 if cond else 0
    tot = Counter()
    for c in por_tier.values():
        tot.update(c)
    return {"total": len(filas), "sin_hechizos": tot["sin_hechizos"], "pocas_cartas": tot["pocas_cartas"],
            "jugables": tot["jugables"], "jugables_sin_hechizos": tot["jugables_sin_hechizos"],
            "jugables_pocas_cartas": tot["jugables_pocas_cartas"],
            "por_tier": {str(t): dict(c) for t, c in sorted(por_tier.items())}, "filas": filas}


def mide_referencias(cat):
    ids = {fam: {e.get("id") for e in es if isinstance(e.get("id"), str)} for fam, es in cat.items()}
    tabla = {(c, r): fams for c, r, fams in REFERENCIAS}
    por_campo = {k: [0, 0] for k in tabla}
    rotas = []
    for nombre, es in cat.items():
        for e in es:
            for ruta, valor in camina(e):
                fams = tabla.get((nombre, ruta))
                if fams is None:
                    continue
                por_campo[(nombre, ruta)][0] += 1
                if any(valor in ids.get(f, ()) for f in fams):
                    continue
                por_campo[(nombre, ruta)][1] += 1
                pista = ""
                for p in PREFIJOS:
                    if valor.startswith(p) and any(valor[len(p):] in ids.get(f, ()) for f in fams):
                        pista = "quitar el prefijo '%s'" % p
                rotas.append({"catalogo": nombre, "entrada": e.get("id"), "campo": ruta, "valor": valor,
                              "familias": "/".join(fams), "pista": pista,
                              "arreglo": "formula" if pista else "persona"})
    campos = [{"catalogo": c, "campo": r, "refs": n, "rotas": m, "familias": "/".join(tabla[(c, r)])}
              for (c, r), (n, m) in sorted(por_campo.items()) if n]
    distintos = Counter((x["valor"], x["familias"]) for x in rotas)
    return {"total": sum(n for n, _ in por_campo.values()), "rotas": len(rotas),
            "ids_rotos_distintos": len(distintos), "por_campo": campos,
            "mas_repetidos": [{"valor": v, "familias": f, "veces": n} for (v, f), n in distintos.most_common(15)],
            "lista": rotas}


def mide(carpeta=CATALOGOS, reglas=REGLAS, aventuras=AVENTURAS):
    cat, ausentes = carga(carpeta)
    hand = lee_hand_limit(reglas)
    cap = {}
    try:
        cap = {int(t["tier"]): int(t["healthCap"]) for t in _lee(reglas)["tiers"]}
    except (OSError, ValueError, KeyError, TypeError):
        pass
    usadas_clase, usadas_monstruo = set(), set()
    for c in cat["classes"]:
        ini, apr = cartas_de_clase(c)
        usadas_clase.update(ini)
        usadas_clase.update(apr)
    for m in cat["monsters"]:
        usadas_monstruo.update(_cadenas(m.get("skillIds")))
    fondos = {i for b in cat.get("backgrounds", []) for i in _cadenas(b.get("skillProficiencyIds"))}
    checks = lee_skill_checks(aventuras)
    cartas = mide_cartas(cat, usadas_clase, usadas_monstruo, fondos, set(checks))
    monstruos = mide_monstruos(cat, cap)
    clases = mide_clases(cat, hand)
    refs = mide_referencias(cat)
    return {
        "catalogos": {n: len(es) for n, es in sorted(cat.items())},
        "ausentes": ausentes,
        "cuatro": {
            "hechizos_huerfanos": [cartas["spells"]["huerfanas"], cartas["spells"]["total"]],
            "monstruos_vida0": [monstruos["vida0"], monstruos["total"]],
            "clases_sin_hechizos": [clases["sin_hechizos"], clases["total"]],
            "referencias_rotas": [refs["rotas"], refs["total"]],
        },
        "cartas": cartas, "monstruos": monstruos, "clases": clases, "referencias": refs,
        "skill_checks_aventuras": dict(checks),
    }


# --------------------------------------------------------------- salida ----
def resumen(r):
    q = r["cuatro"]
    c, m, k, rf = r["cartas"], r["monstruos"], r["clases"], r["referencias"]
    L = ["DEUDA DE CONTENIDO (AV12-05)", "",
         "  1. hechizos que ninguna clase ni monstruo usa : %d de %d" % tuple(q["hechizos_huerfanos"]),
         "     (cartas de skills.json en el mismo caso   : %d de %d)" % (c["skills"]["huerfanas"], c["skills"]["total"]),
         "  2. monstruos con vida 0 (como la lee el C++)   : %d de %d" % tuple(q["monstruos_vida0"]),
         "     sin CA: %d, sin ataque: %d, sin cartas (skillIds): %d" % (m["sin_ca"], m["sin_ataque"], m["sin_cartas"]),
         "  3. clases sin ningun hechizo                   : %d de %d (jugables: %d de %d)" % (
             k["sin_hechizos"], k["total"], k["jugables_sin_hechizos"], k["jugables"]),
         "     clases con menos cartas que su minimo       : %d (jugables: %d)" % (k["pocas_cartas"], k["jugables_pocas_cartas"]),
         "  4. referencias rotas                           : %d de %d (%d ids distintos)" % (
             rf["rotas"], rf["total"], rf["ids_rotos_distintos"])]
    if r["ausentes"]:
        L.append("  AVISO: faltan catalogos: " + ", ".join(r["ausentes"]))
    return "\n".join(L)


def markdown(r):
    c, m, k, rf = r["cartas"], r["monstruos"], r["clases"], r["referencias"]
    out = ["| Medida | Hoy | Sobre |", "|---|---:|---:|"]
    q = r["cuatro"]
    for nombre, clave in (("Hechizos que ninguna clase ni monstruo usa", "hechizos_huerfanos"),
                          ("Monstruos con vida 0", "monstruos_vida0"),
                          ("Clases sin hechizos", "clases_sin_hechizos"),
                          ("Referencias rotas", "referencias_rotas")):
        out.append("| %s | %d | %d |" % (nombre, q[clave][0], q[clave][1]))
    out += ["", "Monstruos por tier", "", "| Tier | Fichas | Vida 0 | Sin CA | Sin ataque | Sin cartas | Ataques sin cifra | Vida sobre healthCap |",
            "|---:|---:|---:|---:|---:|---:|---:|---:|"]
    for t, x in m["por_tier"].items():
        out.append("| %s | %d | %d | %d | %d | %d | %d | %d |" % (
            t, x.get("fichas", 0), x.get("vida0", 0), x.get("sin_ca", 0), x.get("sin_ataque", 0),
            x.get("sin_cartas", 0), x.get("ataques_sin_cifra", 0), x.get("vida_sobre_healthCap", 0)))
    out += ["", "Clases por tier", "", "| Tier | Clases | Jugables | Sin hechizos | Con menos cartas que el minimo |", "|---:|---:|---:|---:|---:|"]
    for t, x in k["por_tier"].items():
        out.append("| %s | %d | %d | %d | %d |" % (t, x.get("clases", 0), x.get("jugables", 0), x.get("sin_hechizos", 0), x.get("pocas_cartas", 0)))
    out += ["", "Referencias rotas por campo", "", "| Catalogo | Campo | Referencias | Rotas | Debe existir en |", "|---|---|---:|---:|---|"]
    for x in rf["por_campo"]:
        if x["rotas"]:
            out.append("| %s | `%s` | %d | %d | %s |" % (x["catalogo"], x["campo"], x["refs"], x["rotas"], x["familias"]))
    return "\n".join(out)


# ------------------------------------------------------------ propuestas ----
def _csv(ruta, cabecera, filas):
    with open(ruta, "w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerow(cabecera)
        w.writerows(filas)


def candidatos(cat, spell, jugables, tags_clase):
    puntos = []
    for c in jugables:
        p = 0
        if spell.get("castingStat") == c.get("primaryStat"):
            p += 3
        elif spell.get("castingStat") == c.get("secondaryStat"):
            p += 1
        comun = set(_cadenas(spell.get("mechanicTags"))) & tags_clase.get(c.get("id"), set())
        p += 2 * len(comun)
        puntos.append((p, c.get("id")))
    puntos.sort(key=lambda x: (-x[0], x[1]))
    return [(i, p) for p, i in puntos[:3] if p > 0]


def propone(carpeta, destino, r, reglas=REGLAS):
    if os.path.abspath(destino) == os.path.abspath(carpeta):
        raise SystemExit("Me niego: el destino es la carpeta de catalogos.")
    os.makedirs(destino, exist_ok=True)
    cat, _ = carga(carpeta)
    # 1. catalogo de monstruos propuesto: solo la vida de las fichas que se salen de la formula
    original = _lee(os.path.join(carpeta, "monsters.json"))
    prop = copy.deepcopy(original)
    cambios = []
    for e in prop["entries"]:
        f = formula_vida(e.get("tier"), e.get("rank"))
        v = vida_cargada(e)
        if f is not None and f != v:
            d = e.setdefault("defenses", {})
            d["maxHealth"] = f
            cambios.append((e["id"], e.get("tier"), e.get("rank"), v, f))
    with open(os.path.join(destino, "monsters.propuesto.json"), "w", encoding="utf-8") as fo:
        json.dump(prop, fo, indent=2, ensure_ascii=False)
    _csv(os.path.join(destino, "monstruos_vida_cambios.csv"), ["id", "tier", "rango", "vida_hoy", "vida_formula"], cambios)
    filas = []
    for rango, (base, paso) in FORMULA_VIDA.items():
        for t in range(1, 6):
            n = sum(1 for e in cat["monsters"] if e.get("tier") == t and e.get("rank") == rango)
            filas.append([t, rango, base + paso * t, n])
    _csv(os.path.join(destino, "vida_por_tier.csv"), ["tier", "rango", "vida_formula", "fichas_hoy"], filas)
    # 2. que hacer con cada hechizo huerfano. 200 de los 210 son plegarias de una deidad
    # (no cartas de clase): su via es la deidad. Solo los demas se reparten entre clases.
    deidades = {e.get("id") for e in cat.get("deities", [])}
    clases = {c.get("id"): c for c in cat["classes"]}
    pool = [clases[i] for i in CLASES_DEL_METODO if i in clases] or \
        [c for c in cat["classes"] if c.get("playable", True) is not False]
    tags_clase = defaultdict(set)
    por_id = {e.get("id"): e for e in cat["skills"] + cat["spells"]}
    for c in pool:
        ini, apr = cartas_de_clase(c)
        for i in ini + apr:
            tags_clase[c.get("id")].update(_cadenas(por_id.get(i, {}).get("mechanicTags")))
    sp = {e.get("id"): e for e in cat["spells"]}
    filas = []
    for i in r["cartas"]["spells"]["huerfanas_ids"]:
        s = sp[i]
        deidad = s.get("deity") if isinstance(s.get("deity"), str) else ""
        etiquetadas = [t for t in _cadenas(s.get("classTags")) if t in clases]
        cs = []
        if deidad and deidad in deidades:
            via, arreglo = "plegaria de una deidad que existe: cablear prayerSkillIds", "motor"
        elif deidad:
            via, arreglo = "plegaria de una deidad que NO existe: crear la deidad", "persona"
        elif etiquetadas:
            via, arreglo = "la etiqueta classTags ya la nombra: anadirlo a startingCards.spells", "formula"
        else:
            via, arreglo = "elegir clase", "persona"
            cs = candidatos(cat, s, pool, tags_clase)
        fila = [i, s.get("name"), s.get("tier"), s.get("castingStat"), deidad, "/".join(etiquetadas), via, arreglo]
        for j in range(3):
            fila += list(cs[j]) if j < len(cs) else ["", ""]
        filas.append(fila)
    _csv(os.path.join(destino, "reparto_hechizos.csv"),
         ["hechizo", "nombre", "tier", "castingStat", "deidad", "clase_etiquetada", "via", "arreglo",
          "candidata1", "puntos1", "candidata2", "puntos2", "candidata3", "puntos3"], filas)
    # 3. referencias rotas y clases pendientes
    _csv(os.path.join(destino, "referencias_rotas.csv"), ["catalogo", "entrada", "campo", "valor", "debe_existir_en", "pista", "arreglo"],
         [[x["catalogo"], x["entrada"], x["campo"], x["valor"], x["familias"], x["pista"], x["arreglo"]] for x in r["referencias"]["lista"]])
    _csv(os.path.join(destino, "clases_pendientes.csv"), ["clase", "jugable", "tier", "cartas", "minimo", "hechizos"],
         [[f["id"], f["jugable"], f["tier"], f["cartas"], f["minimo"], f["hechizos"]] for f in r["clases"]["filas"]
          if f["hechizos"] == 0 or f["pocas_cartas"]])
    return {"monstruos_cambiados": len(cambios), "hechizos_repartidos": len(filas)}


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--catalogos", default=CATALOGOS)
    ap.add_argument("--reglas", default=REGLAS)
    ap.add_argument("--aventuras", default=AVENTURAS)
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--md", action="store_true")
    ap.add_argument("--proponer", metavar="DIR")
    a = ap.parse_args(argv)
    r = mide(a.catalogos, a.reglas, a.aventuras)
    if a.proponer:
        print("propuesta:", propone(a.catalogos, a.proponer, r))
    if a.json:
        print(json.dumps(r, ensure_ascii=False, indent=2))
    elif a.md:
        print(markdown(r))
    else:
        print(resumen(r))
    return 0


if __name__ == "__main__":
    sys.exit(main())
