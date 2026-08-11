#!/usr/bin/env python3
"""Audita los JSON ORIGINALES de dndWeebCC contra lo que necesita la
GameMachine Onegai (GAMEMACHINE_NECESIDADES.md, Fases A-C).

Por que existe: la seccion 7 del documento afirma que "ningun dato se ha
perdido" en la importacion a assets/objects/libreria_*.json. Al comprobarlo
resulto FALSO para las 693 cartas de habilidad/hechizo y las 43 razas: solo
conservan tier/rarity/flavorText, sin recovery, castingStat ni baseBonuses.

Este script responde la pregunta que decide el plan entero:
   ¿los JSON ORIGINALES traen esos campos, o tampoco existen?

  - Si SI estan  -> es un fallo de importacion: se reimporta y adelante.
  - Si NO estan  -> es deuda de CONTENIDO, no de motor. Programar el
                    DicePoolEngine no sirve de nada hasta que alguien
                    decida que "recovery" tiene cada carta.

Uso:
    python3 tools/auditar_datos_onegai.py [ruta_a_dndWeebCC-master]

Sin argumento busca en las rutas habituales relativas al repo.
No modifica nada: solo lee y reporta.
"""

import json
import os
import sys
from collections import Counter

# --- Que necesita cada tipo, segun GAMEMACHINE_NECESIDADES.md ---------------
# (campo_canonico, [alias aceptados en el JSON original])
REQUISITOS = {
    "habilidades": [
        ("recovery",        ["recovery", "recuperacion", "pila", "recoveryPile"]),
        ("actionType",      ["actionType", "tipoAccion", "tipo_accion", "action"]),
        ("castingStat",     ["castingStat", "stat", "atributo", "statLanzamiento"]),
        ("effect",          ["effect", "efecto", "effectType"]),
        ("target",          ["target", "objetivo"]),
        ("magnitude",       ["magnitude", "power", "dano", "damage", "magnitudeByDegree",
                             "degreeBonus", "potencia"]),
        ("tier",            ["tier", "_tier", "nivel"]),
        ("tags",            ["tags", "etiquetas"]),
    ],
    "hechizos": [
        ("recovery",        ["recovery", "recuperacion", "pila", "recoveryPile"]),
        ("actionType",      ["actionType", "tipoAccion", "tipo_accion", "action"]),
        ("castingStat",     ["castingStat", "stat", "atributo", "statLanzamiento"]),
        ("effect",          ["effect", "efecto", "effectType"]),
        ("target",          ["target", "objetivo"]),
        ("magnitude",       ["magnitude", "power", "dano", "damage", "magnitudeByDegree",
                             "degreeBonus", "potencia"]),
        ("tier",            ["tier", "_tier", "nivel"]),
    ],
    "razas": [
        ("baseBonuses",     ["baseBonuses", "bonos", "statBonuses", "bonuses", "modificadores"]),
        ("speedCategory",   ["speedCategory", "velocidad", "speed"]),
        ("traitIds",        ["traitIds", "rasgos", "traits"]),
    ],
    "clases": [
        ("baseHealth",      ["baseHealth", "vidaBase", "_baseHealth", "hp"]),
        ("passiveId",       ["passiveId", "pasiva", "passive"]),
        ("primaryStat",     ["primaryStat", "_primaryStat", "statPrincipal"]),
        ("maxSkillCards",   ["maxSkillCards", "_maxSkillCards", "limiteMano", "handLimit"]),
        ("startingSkills",  ["startingSkillIds", "startingSkills", "habilidadesIniciales"]),
        ("allowedEquip",    ["allowedEquipmentTags", "_allowedEquipmentTags", "equipoPermitido"]),
    ],
    "transfondos": [
        ("virtue",          ["virtue", "virtud"]),
        ("flaw",            ["flaw", "defecto"]),
        ("goal",            ["goal", "meta", "objetivo"]),
        ("startingGold",    ["startingGoldRange", "oroInicial", "gold"]),
    ],
    "condiciones": [
        ("duration",        ["duration", "duracion", "rounds"]),
        ("effect",          ["effect", "efecto", "tickEffect"]),
        ("statModifier",    ["statModifier", "modificador", "mods", "modifiers"]),
        ("stacking",        ["stacking", "acumulable", "stacks"]),
    ],
    "armas": [
        ("slot",            ["slot", "_slot", "ranura"]),
        ("weightCategory",  ["weightCategory", "_weightCategory", "peso"]),
        ("compatibility",   ["compatibilitySymbols", "_grantedTags", "simbolos", "compatibility"]),
        ("statBonuses",     ["statBonuses", "_statBonuses", "bonos"]),
        ("precisionStat",   ["precisionStat", "_precisionStat"]),
    ],
    "enemigos": [
        ("stats",           ["stats", "_stats", "atributos"]),
        ("loot",            ["loot", "_loot", "lootTableIds", "botin"]),
        ("tier",            ["tier", "_tier"]),
    ],
}

# Carpetas alternativas por tipo (los repos no siempre coinciden en nombre)
ALIAS_CARPETA = {
    "habilidades": ["habilidades", "skills", "habilidad"],
    "hechizos":    ["hechizos", "spells", "hechizo"],
    "razas":       ["razas", "races", "raza"],
    "clases":      ["clases", "classes", "clase"],
    "transfondos": ["transfondos", "trasfondos", "backgrounds"],
    "condiciones": ["condiciones", "conditions"],
    "armas":       ["armas", "weapons", "equipo", "equipment", "armaduras"],
    "enemigos":    ["enemigos", "monsters", "monstruos", "enemies"],
}


def localizar_raiz(argv):
    if len(argv) > 1:
        return argv[1]
    aqui = os.path.dirname(os.path.abspath(__file__))
    for cand in [
        os.path.join(aqui, "..", "..", "..", "dndWeebCC-master"),
        os.path.join(aqui, "..", "..", "dndWeebCC-master"),
        os.path.expanduser("~/Documents/Documentos - Oriol Os (2)/Software/dndWeebCC-master"),
    ]:
        if os.path.isdir(cand):
            return os.path.normpath(cand)
    return None


def claves_de(obj, prefijo=""):
    """Claves de un dict, incluyendo un nivel de anidamiento (data.x, stats.y)."""
    out = set()
    if not isinstance(obj, dict):
        return out
    for k, v in obj.items():
        out.add(prefijo + k)
        if isinstance(v, dict):
            for k2 in v:
                out.add(k + "." + k2)
    return out


def cargar_cartas(carpeta):
    """Devuelve lista de dicts de carta. Soporta 1 archivo = 1 carta y
    1 archivo = {"objects":[...]} / lista suelta."""
    cartas = []
    for root, _dirs, files in os.walk(carpeta):
        for f in files:
            if not f.endswith(".json"):
                continue
            try:
                d = json.load(open(os.path.join(root, f), encoding="utf-8"))
            except Exception as e:
                print(f"    (no se pudo leer {f}: {e})")
                continue
            if isinstance(d, list):
                cartas.extend(x for x in d if isinstance(x, dict))
            elif isinstance(d, dict):
                for clave in ("objects", "cartas", "items", "data"):
                    if isinstance(d.get(clave), list):
                        cartas.extend(x for x in d[clave] if isinstance(x, dict))
                        break
                else:
                    cartas.append(d)
    return cartas


def main():
    raiz = localizar_raiz(sys.argv)
    if raiz is None or not os.path.isdir(raiz):
        print("No encuentro dndWeebCC-master.")
        print("Uso: python3 tools/auditar_datos_onegai.py /ruta/a/dndWeebCC-master")
        return 2

    base_cartas = None
    for cand in [os.path.join(raiz, "data", "cartas"), os.path.join(raiz, "data"), raiz]:
        if os.path.isdir(cand):
            base_cartas = cand
            break
    print(f"Auditando: {base_cartas}\n")

    veredictos = {}
    for tipo, requisitos in REQUISITOS.items():
        carpeta = None
        for alias in ALIAS_CARPETA.get(tipo, [tipo]):
            cand = os.path.join(base_cartas, alias)
            if os.path.isdir(cand):
                carpeta = cand
                break
        if carpeta is None:
            print(f"── {tipo:14s} CARPETA NO ENCONTRADA (buscadas: "
                  f"{', '.join(ALIAS_CARPETA.get(tipo, [tipo]))})")
            veredictos[tipo] = "sin datos"
            print()
            continue

        cartas = cargar_cartas(carpeta)
        if not cartas:
            print(f"── {tipo:14s} 0 cartas legibles en {carpeta}")
            veredictos[tipo] = "sin datos"
            print()
            continue

        # Cobertura de cada requisito
        presentes = Counter()
        for carta in cartas:
            ks = claves_de(carta)
            # tambien dentro de "data"/"mechanics" si existieran
            for anidado in ("data", "mechanics", "mecanica", "stats"):
                if isinstance(carta.get(anidado), dict):
                    ks |= set(carta[anidado].keys())
            for canon, alias in requisitos:
                if any(a in ks for a in alias):
                    presentes[canon] += 1

        total = len(cartas)
        faltan = [c for c, _ in requisitos if presentes[c] == 0]
        parciales = [c for c, _ in requisitos if 0 < presentes[c] < total]

        # OK solo si TODOS los requisitos estan en TODAS las cartas. Un
        # campo presente en la mitad del mazo no permite construir el
        # catalogo: la otra mitad quedaria con valores inventados.
        if not faltan and not parciales:
            estado = "OK"
        elif len(faltan) == len(requisitos):
            estado = "FALTA TODO"
        else:
            estado = "PARCIAL"
        veredictos[tipo] = estado
        print(f"── {tipo:14s} {total:5d} cartas   [{estado}]")
        for canon, _alias in requisitos:
            n = presentes[canon]
            marca = "OK " if n == total else ("~~ " if n else "NO ")
            print(f"     {marca} {canon:16s} {n}/{total}")
        if parciales:
            print(f"     (parciales: {', '.join(parciales)})")
        # Muestra las claves reales de una carta, para ver como se llaman
        ejemplo = sorted(claves_de(cartas[0]))
        print(f"     claves reales de ejemplo: {ejemplo[:14]}")
        print()

    # --- Veredicto para el plan -------------------------------------------
    print("=" * 68)
    print("VEREDICTO PARA GAMEMACHINE_NECESIDADES.md")
    print("=" * 68)
    criticos = ["habilidades", "hechizos", "razas"]
    bloqueado = [t for t in criticos if veredictos.get(t) in ("FALTA TODO", "sin datos")]
    incompleto = [t for t in criticos if veredictos.get(t) == "PARCIAL"]
    if bloqueado:
        print(f"BLOQUEADO: {', '.join(bloqueado)} no tienen los campos mecanicos")
        print("           ni siquiera en el origen.")
        print()
        print("  => NO es un fallo de importacion: es deuda de CONTENIDO.")
        print("     Antes de programar DicePoolEngine/CardPileSystem hay que")
        print("     DECIDIR y escribir, por carta: recovery, actionType,")
        print("     castingStat, effect, target y magnitud.")
        print("     Con 693 cartas, eso es diseño de juego, no ingenieria.")
        print()
        print("  Camino recomendado: definir el esquema, rellenar un LOTE")
        print("  PEQUENO (las 20-30 cartas de 1 clase) y construir las Fases")
        print("  A-C contra ese lote. El resto se rellena despues sin tocar codigo.")
    elif incompleto:
        print(f"PARCIAL: {', '.join(incompleto)} tienen los campos solo en ALGUNAS cartas.")
        print()
        print("  => Se puede empezar, pero SOLO con el subconjunto completo.")
        print("     Filtra las cartas que tengan todos los campos y construye")
        print("     las Fases A-C contra ellas; el resto se completa despues.")
        print("     No inventes valores por defecto para las incompletas: un")
        print("     'recovery' supuesto rompe el balance sin que nadie lo note.")
    else:
        print("VIABLE: el origen conserva los campos mecanicos en todas las cartas.")
        print("  => Fue un fallo de la importacion a assets/objects/.")
        print("     Escribir tools/convertir_definiciones_rpg.py leyendo de")
        print("     data/cartas/ y la Fase A puede empezar.")
    print()
    for t, v in veredictos.items():
        print(f"  {t:14s} {v}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
