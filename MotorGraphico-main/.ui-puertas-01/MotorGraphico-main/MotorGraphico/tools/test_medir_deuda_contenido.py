#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Pruebas de tools/medir_deuda_contenido.py sobre un catalogo de mentira.

No mira assets/catalogs/ a proposito: un medidor probado contra el catalogo que
mide no es falsable (mismo criterio que test_buscar_sobrante.py). El catalogo
de mentira tiene UN caso de cada cosa que el medidor tiene que encontrar.

  python3 tools/test_medir_deuda_contenido.py
"""
import importlib.util
import json
import os
import pathlib
import sys
import tempfile


def _cargar():
    ruta = pathlib.Path(__file__).with_name("medir_deuda_contenido.py")
    spec = importlib.util.spec_from_file_location("medir_deuda_contenido", ruta)
    modulo = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(modulo)
    return modulo


def escribe(carpeta, nombre, entradas):
    with open(os.path.join(carpeta, nombre + ".json"), "w", encoding="utf-8") as f:
        json.dump({"entries": entradas}, f, ensure_ascii=False, indent=2)


def monstruo(i, tier=1, rango="criatura", **extra):
    e = {"id": i, "tier": tier, "rank": rango, "stats": {"CON": 1, "DES": 2, "INT": 1, "CAR": 1},
         "defenses": {"ca": 12, "defMental": 11, "maxHealth": 12, "precMag": 10, "resFis": 11},
         "attacks": [{"name": "Golpe", "defense": "CA", "effect": "daño"}], "skillIds": [], "loot": []}
    e.update(extra)
    return e


def catalogo(carpeta):
    # cartas: una usada por clase, una usada por monstruo, una huerfana, una que otro enlaza
    escribe(carpeta, "skills", [
        {"id": "s_clase", "tier": 1, "classTags": ["c_con_hechizo"], "evolvesInto": "s_rota"},
        {"id": "s_monstruo", "tier": 1, "classTags": []},
        {"id": "s_huerfana", "tier": 3, "classTags": []},
        {"id": "s_equipo", "tier": 2, "classTags": []},
    ])
    escribe(carpeta, "spells", [
        {"id": "h_clase", "tier": 1, "classTags": ["c_con_hechizo"]},
        {"id": "h_huerfana_sin_nada", "tier": 2, "classTags": []},
        {"id": "h_deidad_existe", "tier": 1, "deity": "dios_real", "classTags": []},
        {"id": "h_deidad_falta", "tier": 1, "deity": "dios_fantasma", "classTags": []},
        {"id": "h_solo_tag", "tier": 1, "classTags": ["c_con_hechizo"]},  # la etiqueta dice clase, la clase no lo lista
    ])
    escribe(carpeta, "classes", [
        {"id": "c_con_hechizo", "tier": 1, "startingCards": {"skills": ["s_clase"], "spells": ["h_clase"], "learnableSkills": ["s_equipo"]}},
        {"id": "c_sin_hechizo", "tier": 1, "startingCards": {"skills": ["s_clase"], "spells": [], "learnableSkills": []}},
        {"id": "c_completa", "tier": 1, "playable": False,
         "startingCards": {"skills": ["s_clase", "s_monstruo", "s_equipo"], "spells": [], "learnableSkills": []}},
    ])
    escribe(carpeta, "monsters", [
        monstruo("m_bien", derived={"vida": 6}),  # la mitad de maxHealth, como las 432 reales
        monstruo("m_vida0", defenses={"ca": 12}),  # sin vida en ningun sitio -> 0
        monstruo("m_vida_en_derived", defenses={"ca": 12}, derived={"vida": 7}),  # el cargador lo lee: no es 0
        monstruo("m_sin_ca", defenses={"maxHealth": 12}),
        monstruo("m_sin_ataque", attacks=[]),
        monstruo("m_con_carta", skillIds=["s_monstruo"]),
        monstruo("m_botin_roto", loot=["espada_que_no_existe", "espada"]),
        monstruo("m_jefe_t3", tier=3, rango="jefe", defenses={"ca": 14, "maxHealth": 60}),
    ])
    escribe(carpeta, "equipment", [{"id": "espada", "linkedSkill": "s_equipo"}])
    escribe(carpeta, "deities", [{"id": "dios_real", "grantedSpells": ["h_deidad_existe"]}])
    # un catalogo que nadie carga: no tiene que contar
    escribe(carpeta, "locations_trae_test", [{"id": "x", "deityIds": ["no_existe"]}])
    reglas = os.path.join(carpeta, "..", "tier_rules.json")
    with open(reglas, "w", encoding="utf-8") as f:
        json.dump({"tiers": [{"tier": 1, "handLimit": 3, "healthCap": 18}, {"tier": 3, "handLimit": 5, "healthCap": 35}]}, f)
    return reglas


def ejecutar():
    m = _cargar()
    casos, fallos = 0, []

    def comprueba(nombre, condicion, detalle=""):
        nonlocal casos
        casos += 1
        if not condicion:
            fallos.append("%s %s" % (nombre, detalle))

    with tempfile.TemporaryDirectory() as raiz:
        cat = os.path.join(raiz, "catalogs")
        os.makedirs(cat)
        reglas = catalogo(cat)
        antes = {f: open(os.path.join(cat, f), "rb").read() for f in os.listdir(cat)}
        r = m.mide(cat, reglas, os.path.join(raiz, "no_hay_aventuras"))
        q = r["cuatro"]

        # 1. hechizos y cartas huerfanos
        sp = r["cartas"]["spells"]
        comprueba("hechizos: el total", sp["total"] == 5, sp["total"])
        comprueba("hechizos huerfanos = los que ninguna clase lista",
                  sorted(sp["huerfanas_ids"]) == ["h_deidad_existe", "h_deidad_falta", "h_huerfana_sin_nada", "h_solo_tag"],
                  sp["huerfanas_ids"])
        comprueba("el cuarto numero del plan coincide", q["hechizos_huerfanos"] == [4, 5], q["hechizos_huerfanos"])
        comprueba("por etiqueta classTags: dos tienen clase y tres no", sp["sin_classTags"] == 3, sp["sin_classTags"])
        comprueba("deidad que existe / que falta / sin deidad",
                  sp["por_deidad"] == {"deidad existe": 1, "deidad falta": 1, "sin deidad": 2}, sp["por_deidad"])
        comprueba("deidades que faltan", sp["deidades_que_faltan"] == ["dios_fantasma"], sp["deidades_que_faltan"])
        comprueba("el enlace de la deidad se ve", sp["con_enlace"].get("deidad") == 1, sp["con_enlace"])
        comprueba("sin ningun enlace: los otros tres", sp["sin_ningun_enlace"] == 3, sp["sin_ningun_enlace"])
        sk = r["cartas"]["skills"]
        comprueba("carta huerfana: solo la que nadie usa", sk["huerfanas_ids"] == ["s_huerfana"], sk["huerfanas_ids"])
        comprueba("carta de monstruo no es huerfana", "s_monstruo" not in sk["huerfanas_ids"])
        comprueba("carta aprendible de clase no es huerfana", "s_equipo" not in sk["huerfanas_ids"])

        # 2. monstruos
        mo = r["monstruos"]
        comprueba("monstruos: total", mo["total"] == 8, mo["total"])
        comprueba("vida 0 = solo el que no la trae por ningun lado", mo["vida0"] == 1, mo["vida0"])
        comprueba("vida en derived.vida cuenta como vida (el cargador la lee)", q["monstruos_vida0"] == [1, 8])
        comprueba("sin CA: uno", mo["sin_ca"] == 1, mo["sin_ca"])
        comprueba("sin ataque: uno", mo["sin_ataque"] == 1, mo["sin_ataque"])
        comprueba("sin cartas: todos menos uno", mo["sin_cartas"] == 7, mo["sin_cartas"])
        comprueba("por tier: el tier 3 tiene un jefe", mo["por_tier"]["3"]["fichas"] == 1 and mo["por_tier"]["1"]["fichas"] == 7)
        comprueba("jefe de 60 contra healthCap 35", mo["vida_sobre_healthCap"] == 1, mo["vida_sobre_healthCap"])
        comprueba("vida distinta de derived.vida", mo["vida_distinta_de_derived"] == 1, mo["vida_distinta_de_derived"])
        comprueba("la formula acierta el jefe de tier 3 (30+10*3=60)", m.formula_vida(3, "jefe") == 60)
        comprueba("la formula de la criatura de tier 2", m.formula_vida(2, "criatura") == 16)
        comprueba("rango desconocido: sin formula", m.formula_vida(2, "dragon") is None)

        # 3. clases
        cl = r["clases"]
        comprueba("clases sin hechizos: dos de tres", q["clases_sin_hechizos"] == [2, 3], q["clases_sin_hechizos"])
        comprueba("las dos que faltan son la sin hechizo y la completa",
                  sorted(f["id"] for f in cl["filas"] if f["hechizos"] == 0) == ["c_completa", "c_sin_hechizo"])
        comprueba("pocas cartas: la sin hechizo (1 carta contra 3)", cl["pocas_cartas"] == 1, cl["pocas_cartas"])
        comprueba("jugables: la no jugable no cuenta", cl["jugables"] == 2 and cl["jugables_sin_hechizos"] == 1, cl["jugables_sin_hechizos"])
        comprueba("hechizo por etiqueta: la clase con hechizo tiene 2", cl["filas"][0]["hechizos_por_classTags"] == 2)

        # 4. referencias rotas
        rf = r["referencias"]
        valores = sorted(x["valor"] for x in rf["lista"])
        comprueba("referencias rotas: el botin, la evolucion y la deidad",
                  valores == ["dios_fantasma", "espada_que_no_existe", "s_rota"], valores)
        comprueba("el catalogo que nadie carga no cuenta", all(x["catalogo"] != "locations_trae_test" for x in rf["lista"]))
        comprueba("cuarto numero: rotas", q["referencias_rotas"][0] == 3, q["referencias_rotas"])
        comprueba("cuenta las referencias buenas tambien", rf["total"] > rf["rotas"])

        # el medidor no escribe en el catalogo
        despues = {f: open(os.path.join(cat, f), "rb").read() for f in os.listdir(cat)}
        comprueba("solo lectura", antes == despues)

        # propuestas: se escriben fuera y no tocan el catalogo
        destino = os.path.join(raiz, "propuesta")
        resultado = m.propone(cat, destino, r, reglas)
        comprueba("propuesta: ficheros escritos",
                  all(os.path.exists(os.path.join(destino, f)) for f in
                      ("monsters.propuesto.json", "monstruos_vida_cambios.csv", "vida_por_tier.csv",
                       "reparto_hechizos.csv", "referencias_rotas.csv", "clases_pendientes.csv")))
        comprueba("propuesta: 4 hechizos repartidos", resultado["hechizos_repartidos"] == 4, resultado)
        import csv
        vias = {f[0]: f[7] for f in list(csv.reader(open(os.path.join(destino, "reparto_hechizos.csv"), encoding="utf-8")))[1:]}
        comprueba("via: deidad que existe -> motor, que falta -> persona, etiqueta -> formula, nada -> persona",
                  vias == {"h_deidad_existe": "motor", "h_deidad_falta": "persona", "h_solo_tag": "formula",
                           "h_huerfana_sin_nada": "persona"}, vias)
        prop = json.load(open(os.path.join(destino, "monsters.propuesto.json"), encoding="utf-8"))
        vidas = {e["id"]: m.vida_cargada(e) for e in prop["entries"]}
        comprueba("propuesta: la vida de la criatura de tier 1 por formula es 12", vidas["m_bien"] == 12)
        comprueba("propuesta: la ficha sin vida recibe la de su tier y rango", vidas["m_vida0"] == 12, vidas["m_vida0"])
        comprueba("propuesta: la ficha que ya cumplia no cambia",
                  [e for e in prop["entries"] if e["id"] == "m_jefe_t3"][0]["defenses"]["maxHealth"] == 60)
        despues = {f: open(os.path.join(cat, f), "rb").read() for f in os.listdir(cat)}
        comprueba("propuesta: el catalogo sigue igual", antes == despues)
        try:
            m.propone(cat, cat, r, reglas)
            comprueba("propuesta: se niega a escribir en el catalogo", False)
        except SystemExit:
            comprueba("propuesta: se niega a escribir en el catalogo", True)

        # falta un fichero: no se cae y lo dice
        vacio = os.path.join(raiz, "vacio")
        os.makedirs(vacio)
        rv = m.mide(vacio, reglas, vacio)
        comprueba("carpeta vacia: no se cae y avisa", rv["cuatro"]["hechizos_huerfanos"] == [0, 0] and "spells" in rv["ausentes"])

    print("%d comprobaciones, %d fallos" % (casos, len(fallos)))
    for f in fallos:
        print("  FALLA:", f)
    return len(fallos)


if __name__ == "__main__":
    sys.exit(1 if ejecutar() else 0)
