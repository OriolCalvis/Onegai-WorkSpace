#!/usr/bin/env python3
"""
Script de conversion: datos dndWeebCC/data -> ObjectCatalog del Motor Gráfico.

Lee todos los JSON de dndWeebCC-master/data/cartas/ y genera archivos JSON
compatibles con ObjectCatalog::loadFromFile() en assets/objects/.

Estructura de salida (dentro de MotorGraphico/assets/objects/):
  libreria_enemigos.json
  libreria_armas.json
  libreria_consumibles.json
  libreria_clases.json
  libreria_razas.json
  libreria_transfondos.json
  libreria_habilidades.json
  libreria_hechizos.json
  libreria_misc.json  (deidades, dotes, condiciones, pasivas, rasgos, trampas, invocaciones, monturas)
  libreria_completa.json  (TODO, aggregado de todos)
"""

import json
import os
import sys
from pathlib import Path

# --- Configuracion de rutas ---
# Rutas derivadas RELATIVAS al propio script (sin hardcodes portables).
# Script esta en:  <raiz>/MotorGraphico-main/MotorGraphico/tools/convertir_libreria_dnd.py
# dndWeebCC en:     <raiz>/dndWeebCC-master/data/
# Assets salida:    <raiz>/MotorGraphico-main/MotorGraphico/assets/objects/
SCRIPT_DIR = Path(__file__).resolve().parent          # tools/
MOTOR_DIR = SCRIPT_DIR.parent                         # MotorGraphico/
REPO_DIR = MOTOR_DIR.parent                           # MotorGraphico-main/
RAIZ_SOFTWARE = REPO_DIR.parent                       # Software/ (hermano dndWeebCC)

BASE_DND      = RAIZ_SOFTWARE / "dndWeebCC-master" / "data" / "cartas"
BASE_DND_RAIZ = RAIZ_SOFTWARE / "dndWeebCC-master" / "data"
BASE_SALIDA   = MOTOR_DIR / "assets" / "objects"

# Verificacion rapida (si alguien mueve el script avisa inmediatamente)
if not BASE_DND.is_dir():
    print(f"[ERROR] No se encuentra carpeta de entrada cartas/ en: {BASE_DND}", file=sys.stderr)
    print("[INFO]  El script espera la siguiente estructura:", file=sys.stderr)
    print("         Software/", file=sys.stderr)
    print("          ├── dndWeebCC-master/data/cartas/", file=sys.stderr)
    print("          └── MotorGraphico-main/MotorGraphico/tools/convertir_libreria_dnd.py", file=sys.stderr)
    sys.exit(1)
BASE_SALIDA.mkdir(parents=True, exist_ok=True)

SPRITE_POR_DEFECTO = -1

RARITY_PRECIO = {
    "common": 50,
    "uncommon": 150,
    "rare": 400,
    "epic": 1000,
    "legendary": 3000,
    None: 100,
}

TIER_MULTIPLICADOR_PRECIO = {1: 1, 2: 2, 3: 4, 4: 8, 5: 16, None: 1}


def leer_json(ruta: Path) -> dict:
    try:
        with open(ruta, "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception as e:
        print(f"  [WARN] No se pudo leer {ruta}: {e}", file=sys.stderr)
        return {}


def id_seguro(d: dict, fallback: str, prefijo: str = "") -> str:
    raw = d.get("id") or fallback
    limpio = str(raw).replace(" ", "_").lower()
    return prefijo + limpio


def nombre_seguro(d: dict, fallback: str) -> str:
    return d.get("name") or fallback


def estimar_precio(d: dict) -> int:
    rarity = d.get("rarity")
    if rarity not in RARITY_PRECIO:
        rarity = None
    tier = d.get("tier")
    if tier not in TIER_MULTIPLICADOR_PRECIO:
        tier = None
    base = RARITY_PRECIO[rarity]
    mult = TIER_MULTIPLICADOR_PRECIO[tier]
    return base * mult


def extraer_skills_enemigo(d: dict) -> list:
    skills = []
    for atq in d.get("attacks", []) or []:
        nombre = atq.get("name", "")
        if nombre:
            skill_id = "atk_" + nombre.lower().replace(" ", "_").replace("á", "a").replace("é", "e").replace("í", "i").replace("ó", "o").replace("ú", "u").replace("ñ", "n")
            skills.append(skill_id)
    pasiva = d.get("passive", {}) or {}
    if pasiva.get("name"):
        p_id = "passive_" + pasiva["name"].lower().replace(" ", "_")
        skills.append(p_id)
    return skills


def convertir_enemigo(d: dict, fallback_id: str, prefijo: str = "enemy_") -> dict:
    obj_id = id_seguro(d, fallback_id, prefijo)
    name = nombre_seguro(d, fallback_id)
    derived = d.get("derived", {}) or {}
    vida = derived.get("vida", d.get("baseHealth", 10))
    obj = {
        "id": obj_id,
        "name": name,
        "category": "enemy",
        "spriteId": SPRITE_POR_DEFECTO,
        "blocksMovement": True,
        "interactable": False,
        "combat": {
            "maxHealth": int(vida) if vida else 10,
            "maxMana": 0,
            "skills": extraer_skills_enemigo(d),
        },
    }
    extra = {}
    if d.get("flavorText"):
        extra["_flavorText"] = d["flavorText"]
    if d.get("tier"):
        extra["_tier"] = d["tier"]
    if d.get("role"):
        extra["_role"] = d["role"]
    if d.get("faction"):
        extra["_faction"] = d["faction"]
    if d.get("stats"):
        extra["_stats"] = d["stats"]
    if d.get("conditionsInflicted"):
        extra["_conditionsInflicted"] = d["conditionsInflicted"]
    if d.get("loot"):
        extra["_loot"] = d["loot"]
    if extra:
        obj["_sourceData"] = extra
    return obj


def convertir_arma_o_equipo(d: dict, fallback_id: str, prefijo: str = "item_") -> dict:
    obj_id = id_seguro(d, fallback_id, prefijo)
    name = nombre_seguro(d, fallback_id)
    obj = {
        "id": obj_id,
        "name": name,
        "category": "pickup",
        "spriteId": SPRITE_POR_DEFECTO,
        "blocksMovement": False,
        "interactable": True,
        "price": estimar_precio(d),
        "pickup": {
            "effect": "none",
            "power": 0,
        },
    }
    extra = {}
    if d.get("slot"):
        extra["_slot"] = d["slot"]
    if d.get("tier"):
        extra["_tier"] = d["tier"]
    if d.get("rarity"):
        extra["_rarity"] = d["rarity"]
    if d.get("statBonuses"):
        extra["_statBonuses"] = d["statBonuses"]
    if d.get("weightCategory"):
        extra["_weightCategory"] = d["weightCategory"]
    if d.get("grantedTags"):
        extra["_grantedTags"] = d["grantedTags"]
    if d.get("flavorText"):
        extra["_flavorText"] = d["flavorText"]
    if d.get("precisionStat"):
        extra["_precisionStat"] = d["precisionStat"]
    if extra:
        obj["_sourceData"] = extra
    return obj


def convertir_consumible(d: dict, fallback_id: str, prefijo: str = "cons_") -> dict:
    obj_id = id_seguro(d, fallback_id, prefijo)
    name = nombre_seguro(d, fallback_id)
    efecto_str = (d.get("effect", {}) or {}).get("description", "")
    effect = "none"
    power = 0
    el = efecto_str.lower()
    if "vida" in el or "curar" in el or "recuperas" in el:
        effect = "heal"
        import re
        nums = re.findall(r"\d+", el)
        if nums:
            power = int(nums[0])
        else:
            tier = d.get("tier", 1) or 1
            power = (tier * 5) + 5
    elif "mana" in el:
        effect = "restoreMana"
        import re
        nums = re.findall(r"\d+", el)
        power = int(nums[0]) if nums else 5
    obj = {
        "id": obj_id,
        "name": name,
        "category": "pickup",
        "spriteId": SPRITE_POR_DEFECTO,
        "blocksMovement": False,
        "interactable": True,
        "price": estimar_precio(d),
        "pickup": {
            "effect": effect,
            "power": power,
        },
    }
    extra = {}
    if d.get("tier"):
        extra["_tier"] = d["tier"]
    if d.get("rarity"):
        extra["_rarity"] = d["rarity"]
    if d.get("actionType"):
        extra["_actionType"] = d["actionType"]
    if d.get("uses"):
        extra["_uses"] = d["uses"]
    if d.get("effect"):
        extra["_effectDescription"] = d["effect"].get("description", "") if isinstance(d["effect"], dict) else str(d["effect"])
    if d.get("flavorText"):
        extra["_flavorText"] = d["flavorText"]
    if extra:
        obj["_sourceData"] = extra
    return obj


def convertir_prop_referencia(d: dict, fallback_id: str, tipo_fuente: str, prefijo: str = "") -> dict:
    obj_id = id_seguro(d, fallback_id, prefijo)
    name = nombre_seguro(d, fallback_id)
    obj = {
        "id": obj_id,
        "name": name,
        "category": "prop",
        "spriteId": SPRITE_POR_DEFECTO,
        "blocksMovement": False,
        "interactable": True,
    }
    extra = {"_tipo": tipo_fuente}
    for k in ["tier", "rarity", "role", "type", "lore", "description", "flavorText"]:
        if d.get(k):
            extra[f"_{k}"] = d[k]
    for k in ["primaryStat", "secondaryStat", "baseHealth", "maxSkillCards"]:
        if d.get(k):
            extra[f"_{k}"] = d[k]
    if d.get("stats"):
        extra["_stats"] = d["stats"]
    if d.get("startingEquipment"):
        extra["_startingEquipment"] = d["startingEquipment"]
    if d.get("specializations"):
        extra["_specializations"] = d["specializations"]
    if d.get("allowedEquipmentTags"):
        extra["_allowedEquipmentTags"] = d["allowedEquipmentTags"]
    if d.get("compatibility"):
        extra["_compatibility"] = d["compatibility"]
    obj["_sourceData"] = extra
    return obj


def procesar_directorio(carpeta: str, convertidor, **conv_kwargs) -> list:
    ruta_dir = BASE_DND / carpeta
    if not ruta_dir.is_dir():
        print(f"  [INFO] No existe directorio: {ruta_dir}")
        return []
    resultados = []
    for archivo in sorted(ruta_dir.glob("*.json")):
        datos = leer_json(archivo)
        if not datos:
            continue
        fallback = archivo.stem
        try:
            obj = convertidor(datos, fallback, **conv_kwargs) if conv_kwargs else convertidor(datos, fallback)
            resultados.append(obj)
        except Exception as e:
            print(f"  [WARN] Error al convertir {archivo}: {e}", file=sys.stderr)
    return resultados


def escribir_salida(nombre_archivo: str, objetos: list) -> Path:
    BASE_SALIDA.mkdir(parents=True, exist_ok=True)
    salida = BASE_SALIDA / nombre_archivo
    wrapper = {"objects": objetos}
    with open(salida, "w", encoding="utf-8") as f:
        json.dump(wrapper, f, ensure_ascii=False, indent=2)
    print(f"  -> {salida.name}: {len(objetos)} objetos")
    return salida


# ============================================================================
# Helpers para carpetas RAIZ (no /cartas/): lootTables, aventuras, npcs, events
# y para backgrounds (fallback desde kits_iniciales.json)
# ============================================================================

def convertir_loot_table(d: dict, fallback_id: str) -> dict:
    obj_id = id_seguro(d, fallback_id, "loot_")
    name = nombre_seguro(d, fallback_id)
    obj = {
        "id": obj_id,
        "name": name,
        "category": "prop",
        "spriteId": SPRITE_POR_DEFECTO,
        "blocksMovement": False,
        "interactable": True,
    }
    extra = {"_tipo": "loot_table"}
    for k in ["tier", "rarity", "description"]:
        if d.get(k):
            extra[f"_{k}"] = d[k]
    if d.get("entries"):
        extra["_entries_count"] = len(d["entries"])
        # Guardamos un sample, no todo (ObjectCatalog no necesita el detalle):
        extra["_sample_refs"] = [str(e.get("refId", e.get("id", ""))) for e in d["entries"][:6]]
    if d.get("minGold") is not None or d.get("maxGold") is not None:
        extra["_gold_range"] = [d.get("minGold"), d.get("maxGold")]
    obj["_sourceData"] = extra
    return obj


def convertir_npc(d: dict, fallback_id: str) -> dict:
    obj_id = id_seguro(d, fallback_id, "npc_")
    name = nombre_seguro(d, fallback_id)
    obj = {
        "id": obj_id,
        "name": name,
        "category": "npc",
        "spriteId": SPRITE_POR_DEFECTO,
        "blocksMovement": False,
        "interactable": True,
    }
    extra = {"_tipo": "npc"}
    for k in ["tier", "raceId", "classId", "factionId", "occupation", "description", "faction", "factionName"]:
        if d.get(k):
            extra[f"_{k}"] = d[k]
    for k in ["isMerchant", "hasDialogue", "isHostile"]:
        if d.get(k) is not None:
            extra[f"_{k}"] = bool(d[k])
    if d.get("shopLootId"):
        extra["_shopLootId"] = d["shopLootId"]
    if d.get("dialogueId"):
        extra["_dialogueId"] = d["dialogueId"]
    if d.get("monsterId"):
        extra["_monsterId"] = d["monsterId"]
    extra["_gold"] = d.get("gold", 0)
    obj["_sourceData"] = extra
    return obj


def convertir_aventura(d: dict, fallback_id: str) -> dict:
    obj_id = id_seguro(d, fallback_id, "adv_")
    name = nombre_seguro(d, fallback_id)
    obj = {
        "id": obj_id,
        "name": name,
        "category": "prop",
        "spriteId": SPRITE_POR_DEFECTO,
        "blocksMovement": False,
        "interactable": True,
    }
    extra = {"_tipo": "aventura"}
    for k in ["tier", "summary", "description", "mainFactionId"]:
        if d.get(k):
            extra[f"_{k}"] = d[k]
    if d.get("recommendedTierMin") is not None or d.get("recommendedTierMax") is not None:
        extra["_tier_range"] = [d.get("recommendedTierMin"), d.get("recommendedTierMax")]
    for k in ["questIds", "requiredNpcIds"]:
        if d.get(k):
            extra[f"_{k}"] = len(d[k])
    obj["_sourceData"] = extra
    return obj


def convertir_evento(d: dict, fallback_id: str) -> dict:
    obj_id = id_seguro(d, fallback_id, "ev_")
    name = nombre_seguro(d, fallback_id)
    obj = {
        "id": obj_id,
        "name": name,
        "category": "prop",
        "spriteId": SPRITE_POR_DEFECTO,
        "blocksMovement": False,
        "interactable": True,
    }
    extra = {"_tipo": "evento"}
    for k in ["tier", "description", "triggerKind", "trigger", "weight"]:
        if d.get(k):
            extra[f"_{k}"] = d[k]
    if d.get("options"):
        extra["_options_count"] = len(d["options"])
    obj["_sourceData"] = extra
    return obj


def procesar_directorio_raiz(nombre_carpeta_raiz: str, convertidor) -> list:
    """Lee de dndWeebCC-master/data/<carpeta> (fuera de /cartas/)."""
    ruta_dir = BASE_DND_RAIZ / nombre_carpeta_raiz
    if not ruta_dir.is_dir():
        print(f"  [INFO] No existe directorio raiz: {ruta_dir}")
        return []
    resultados = []
    for archivo in sorted(ruta_dir.glob("*.json")):
        datos = leer_json(archivo)
        if not datos:
            continue
        fallback = archivo.stem
        try:
            obj = convertidor(datos, fallback)
            resultados.append(obj)
        except Exception as e:
            print(f"  [WARN] Error al convertir {archivo}: {e}", file=sys.stderr)
    return resultados


def generar_backgrounds_desde_kits() -> list:
    """Lee backgrounds desde kits/kits_iniciales.json (si existen)."""
    ruta_kits = BASE_DND_RAIZ / "kits" / "kits_iniciales.json"
    if not ruta_kits.is_file():
        print("  [INFO] No hay kits_iniciales.json para backgrounds")
        return []
    with open(ruta_kits, "r", encoding="utf-8") as f:
        datos = json.load(f)
    lista = datos if isinstance(datos, list) else datos.get("kits", datos.get("entries", []))
    objs = []
    for idx, d in enumerate(lista):
        fallback = f"kit_{idx}"
        try:
            obj = convertir_prop_referencia(d, fallback, tipo_fuente="trasfondo", prefijo="kit_bg_")
            objs.append(obj)
        except Exception as e:
            print(f"  [WARN] Error al procesar kit #{idx}: {e}", file=sys.stderr)
    return objs


def main():
    print("=== Conversion dndWeebCC -> ObjectCatalog (Motor Gráfico) ===")
    print(f"Fuente:     {BASE_DND}")
    print(f"Fuente raiz:{BASE_DND_RAIZ}")
    print(f"Destino:    {BASE_SALIDA}")
    print()

    todos = []

    print("[1/13] Enemigos...")
    enemigos = procesar_directorio("enemigos", convertir_enemigo, prefijo="enemy_")
    escribir_salida("libreria_enemigos.json", enemigos)
    todos.extend(enemigos)

    print("[2/13] Armas / Equipo...")
    armas = procesar_directorio("armas", convertir_arma_o_equipo, prefijo="item_")
    escribir_salida("libreria_armas.json", armas)
    todos.extend(armas)

    print("[3/13] Consumibles...")
    consumibles = procesar_directorio("consumibles", convertir_consumible, prefijo="cons_")
    escribir_salida("libreria_consumibles.json", consumibles)
    todos.extend(consumibles)

    print("[4/13] Clases...")
    clases = procesar_directorio("clases", convertir_prop_referencia, tipo_fuente="clase", prefijo="class_")
    escribir_salida("libreria_clases.json", clases)
    todos.extend(clases)

    print("[5/13] Razas...")
    razas = procesar_directorio("razas", convertir_prop_referencia, tipo_fuente="raza", prefijo="race_")
    escribir_salida("libreria_razas.json", razas)
    todos.extend(razas)

    print("[6/13] Transfondos...")
    transfondos = procesar_directorio("transfondos", convertir_prop_referencia, tipo_fuente="trasfondo", prefijo="bg_")
    extras_kits = generar_backgrounds_desde_kits()
    if extras_kits:
        print(f"  (+ {len(extras_kits)} backgrounds adicionales desde kits/kits_iniciales.json)")
        transfondos = list(transfondos) + list(extras_kits)
    escribir_salida("libreria_transfondos.json", transfondos)
    todos.extend(transfondos)

    print("[7/13] Habilidades...")
    habilidades = procesar_directorio("habilidades", convertir_prop_referencia, tipo_fuente="habilidad", prefijo="skill_")
    escribir_salida("libreria_habilidades.json", habilidades)
    todos.extend(habilidades)

    print("[8/13] Hechizos...")
    hechizos = procesar_directorio("hechizos", convertir_prop_referencia, tipo_fuente="hechizo", prefijo="spell_")
    escribir_salida("libreria_hechizos.json", hechizos)
    todos.extend(hechizos)

    print("[9/13] Miscelánea (deidades, dotes, condiciones, pasivas, rasgos, trampas, invocaciones, monturas)...")
    misc = []
    for subdir, tipo, prefijo in [
        ("deidades", "deidad", "deity_"),
        ("dotes", "dote", "feat_"),
        ("condiciones", "condicion", "cond_"),
        ("pasivas", "pasiva", "passive_"),
        ("rasgos", "rasgo", "trait_"),
        ("trampas", "trampa", "trap_"),
        ("invocaciones", "invocacion", "summon_"),
        ("monturas", "montura", "mount_"),
    ]:
        items = procesar_directorio(subdir, convertir_prop_referencia, tipo_fuente=tipo, prefijo=prefijo)
        print(f"    - {subdir} ({prefijo[:-1]}): {len(items)}")
        misc.extend(items)
    escribir_salida("libreria_misc.json", misc)
    todos.extend(misc)

    print("[10/13] NPCs (carpeta raiz: npcs/)...")
    npcs = procesar_directorio_raiz("npcs", convertir_npc)
    if not npcs:
        npcs = procesar_directorio_raiz("personatges", convertir_npc)
    escribir_salida("libreria_npcs.json", npcs)
    todos.extend(npcs)

    print("[11/13] LootTables (carpeta raiz: loot/)...")
    loots = procesar_directorio_raiz("loot", convertir_loot_table)
    if not loots:
        loots = procesar_directorio_raiz("loot_tables", convertir_loot_table)
    escribir_salida("libreria_loot.json", loots)
    todos.extend(loots)

    print("[12/13] Aventuras (carpeta raiz: aventuras/)...")
    advs = procesar_directorio_raiz("aventuras", convertir_aventura)
    if not advs:
        advs = procesar_directorio_raiz("aventures", convertir_aventura)
    escribir_salida("libreria_aventuras.json", advs)
    todos.extend(advs)

    print("[13/13] Events (carpeta raiz: eventos/)...")
    evs = procesar_directorio_raiz("eventos", convertir_evento)
    if not evs:
        evs = procesar_directorio_raiz("events", convertir_evento)
    escribir_salida("libreria_events.json", evs)
    todos.extend(evs)

    print()
    escribir_salida("libreria_completa.json", todos)

    print()
    print(f"=== Finalizado. Total: {len(todos)} objetos convertidos ===")
    print()
    print("Los archivos generados son compatibles con ObjectCatalog::loadFromFile().")
    print("Para cargarlos todos desde C++:")
    print("  ObjectCatalog catalog;")
    for n in ["libreria_enemigos", "libreria_armas", "libreria_consumibles",
              "libreria_clases", "libreria_razas", "libreria_transfondos",
              "libreria_habilidades", "libreria_hechizos", "libreria_misc",
              "libreria_npcs", "libreria_loot", "libreria_aventuras", "libreria_events"]:
        print(f'  catalog.loadFromFile("assets/objects/{n}.json");')
    print("  // O directamente el agregado:")
    print('  catalog.loadFromFile("assets/objects/libreria_completa.json");')


if __name__ == "__main__":
    main()
