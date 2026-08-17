#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
convertir_definiciones_rpg.py — FASE A: GameMachine Onegai
------------------------------------------------------------------
Convierte las definiciones CRUDAS de dndWeebCC-master/data/ (fuente unica
de verdad del sistema Onegai RPG Edicion 2) a catálogos tipados JSON
planos en MotorGraphico-main/MotorGraphico/assets/catalogs/ listos para
ser cargados con un Catalog<T> : ICatalog<T> (ver P0-9 GAMEMACHINE_NECESIDADES).

Cada catalogo tiene el formato:
    { "entries": [ {...}, {...} ] }
donde cada entry conserva TODOS los campos originales (nunca perdemos dato)
y se normalizan los que lo necesitan:
    - id: garantizado unico por catalogo (el dndWeebCC ya lo deberia ser,
      pero aun asi forzamos <subdir>_<id_sin_prefijo_original> cuando
      detectamos posible colision cross).
    - recovery: se aceptan 4 valores: activo | descanso_corto | descanso_largo | pasiva
    - actionType: accion_principal | movimiento | reaccion | canalizacion | libre | pasiva
    - castingStat: CON / DES / INT / CAR o bien null
    - magnitudeByDegree: diccionario con "botch"/"partial"/"success"/"critical"
      (si el dato original no lo tiene, se infiere de campos como magnitude,
      damage, heal; si aun asi no hay dato, se pone null por defecto y se
      tratara como "exito plano" = potencia directa, con notacion en warning).
    - compatibleSymbols: lista normalizada de compatibilidad de equipo
      (solo uno de △ ○ □ / ✦ ☠ ✝ ♞ ⚙ segun Arquitectura_Datos_Onegai.md).

Salida: un archivo por catalogo en assets/catalogs/
        + MANIFIESTO.json con estadisticas de conversion.
"""

import json
import os
import sys
import re
import glob
from collections import defaultdict

# ---------------------------------------------------------------------------
# CONFIGURACION DE PATHS (rutas absolutas porque el script se ejecuta con
# cwd arbitrario)
# ---------------------------------------------------------------------------
REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
DND_ROOT = os.path.join(REPO_ROOT, "dndWeebCC-master", "data")
MOTOR_ROOT = os.path.join(REPO_ROOT, "MotorGraphico-main", "MotorGraphico")
SALIDA_DIR = os.path.join(MOTOR_ROOT, "assets", "catalogs")

# Mapeo subcarpeta data/cartas/XXX  →  catalogo
# La lista viene del §7 y §9 del GDD de Onegai: 24 entidades
SUBIR_A_CATALOGO = {
    # Entidades cartas de data/cartas/*
    "clases":         ("classes",            "class_"),
    "razas":          ("races",              "race_"),
    # La carpeta se llama "transfondos" (con n), no "trasfondos". Por esa letra
    # el exportador no la encontraba nunca y caia al fallback de kits_iniciales:
    # el catalogo de backgrounds traia 50 kits genericos en vez de los 12 meses
    # de nacimiento, que son los que llevan el lore, la astrologia y las
    # referencias a deidades.
    "transfondos":    ("backgrounds",        "bg_"),
    "deidades":       ("deities",            "deity_"),
    "pasivas":        ("passives",           "passive_"),
    "habilidades":    ("skills",             "skill_"),
    "hechizos":       ("spells",             "spell_"),
    "armas":          ("equipment",          "equip_"),      # equipaje general
    "consumibles":    ("consumables",        "cons_"),
    "rasgos":         ("traits",             "trait_"),
    "dotes":          ("feats",              "feat_"),
    "condiciones":    ("conditions",         "cond_"),
    "trampas":        ("traps",              "trap_"),
    "invocaciones":   ("summons",            "summon_"),
    "monturas":       ("mounts",             "mount_"),
    "enemigos":       ("monsters",           "monster_"),
}

# Los que NO viven en data/cartas sino en directorios data/ propios
DATA_RAIZ_A_CATALOGO = {
    "loot":      ("loot_tables",   "loot_"),
    "aventuras": ("adventures",    "adv_"),
    # CORREGIDO: 'personatges' son fichas de personaje jugador (claseId, razaId,
    # stats, tier), no PNJs. Los PNJs de verdad viven en data/npcs/ (134, con
    # role/faction/location/dialogue/services). Antes se exportaban las 12 fichas
    # como si fueran el catalogo de PNJs y los 134 PNJs no llegaban al motor.
    "npcs":          ("npcs",             "npc_"),
    "personatges":   ("pregen_characters", "pc_"),
    "eventos":   ("events",        "ev_"),
}

# Recuperacion que tambien ya existe en data/cartas/pasivas etc. pero
# el usuario tambien tiene historias / mapas; si no existen, ignoramos
EXTRA_OPCIONALES = {
    # "historias":  ("stories", "story_"),
    # "mapa":       ("locations", "loc_"),
    # "kits":       ("kits", "kit_"),
}

RAREZA_ORDEN = ["common", "uncommon", "rare", "epic", "legendary", "mythic"]

RECOVERY_NORMALIZADO = {
    "activo":          "activo",
    "per_turn":        "activo",
    "per_ronda":       "activo",
    "cada_turno":      "activo",
    "descanso_corto":  "descanso_corto",
    "short_rest":      "descanso_corto",
    "corto":           "descanso_corto",
    "descanso_largo":  "descanso_largo",
    "long_rest":       "descanso_largo",
    "largo":           "descanso_largo",
    "pasiva":          "pasiva",
    "passive":         "pasiva",
    "ninguno":         "activo",          # si no especifica, es activa por turno
    "none":            "activo",
}

ACTIONTYPE_NORMALIZADO = {
    "accion_principal": "accion_principal",
    "main_action":      "accion_principal",
    "principal":        "accion_principal",
    "movimiento":       "movimiento",
    "move":             "movimiento",
    "reaccion":         "reaccion",
    "reaction":         "reaccion",
    "canalizacion":     "canalizacion",
    "channel":          "canalizacion",
    "libre":            "libre",
    "free":             "libre",
    "pasiva":           "pasiva",
    "passive":          "pasiva",
    "bonus":            "libre",
    "ninguno":          "pasiva",
}

SYMBOLOS_LEGALES = {"△", "○", "□", "✦", "☠", "✝", "♞", "⚙", "?", "N/A"}


# ---------------------------------------------------------------------------
# Utilidades
# ---------------------------------------------------------------------------
def asegura_dir(path):
    os.makedirs(path, exist_ok=True)


def lee_json(path):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def guarda_json(path, dato):
    with open(path, "w", encoding="utf-8") as f:
        json.dump(dato, f, ensure_ascii=False, indent=2, sort_keys=True)


# Ids que colisionan entre catalogos si se conservan tal cual (comprobado sobre
# los 2747 ids explicitos del origen: solo estos tres). Se desempatan a mano.
COLISIONES = {
    ("eclipse_menor", "monster_"):    "monster_eclipse_menor",
    ("paje_traslucido", "monster_"):  "monster_paje_traslucido",
    ("alce_de_ribera", "mount_"):     "mount_alce_de_ribera",
}


def id_seguro(d, prefijo, fallback=None):
    """Conserva el id explicito del origen; solo prefija los generados.

    Antes se prefijaba SIEMPRE, lo que renombraba el 86% de los ids (2364 de
    2747) y dejaba 1910 referencias cruzadas apuntando al vacio: una aventura
    citaba 'chambelan_roto_real' y el catalogo guardaba
    'monster_chambelan_roto_real'. Si el autor puso un id, ese id es la
    identidad y no se toca.
    """
    explicito = d.get("id") is not None
    raw = d.get("id") if explicito else (fallback or d.get("name") or "unnamed")
    raw = str(raw).strip()
    slug = re.sub(r"[^a-z0-9_]+", "_", raw.lower()).strip("_") or "unnamed"
    if explicito:
        return COLISIONES.get((slug, prefijo), slug)
    if slug.startswith(prefijo.rstrip("_") + "_"):
        return slug
    return prefijo + slug


def lista_o_nada(x):
    if x is None:
        return []
    if isinstance(x, list):
        return x
    return [x]


def dict_o_nada(x):
    if x is None:
        return {}
    if isinstance(x, dict):
        return x
    return {}


def normaliza_recuperacion(entrada):
    r = (entrada or "").strip().lower()
    return RECOVERY_NORMALIZADO.get(r, "activo")


def normaliza_actionType(entrada):
    a = (entrada or "").strip().lower()
    return ACTIONTYPE_NORMALIZADO.get(a, "accion_principal")


def normaliza_simbolos(entrada_iterable):
    """Normaliza simbolos de compatibilidad de equipo. Devuelve lista de
    strings unicos y legales solo (cualquier cosa rara se mete como ?)."""
    out = []
    for s in lista_o_nada(entrada_iterable):
        ss = str(s).strip()
        if ss in SYMBOLOS_LEGALES:
            out.append(ss)
        else:
            # Quizas es un multi-caracter (ej: "△✦"), lo desglosamos
            for ch in ss:
                if ch in SYMBOLOS_LEGALES:
                    out.append(ch)
    # dedupe preservando orden
    seen = set()
    uniq = []
    for s in out:
        if s not in seen:
            seen.add(s)
            uniq.append(s)
    return uniq


def infiere_magnitudes(entrada, entry_id, warnings):
    """Devuelve dict {botch, partial, success, critical} o None si no hay
    informacion. Se mira magnitude, damage/heal, power, effect.damage..."""
    mags = entrada.get("magnitudeByDegree") or entrada.get("magnitudes")
    if isinstance(mags, dict):
        # normaliza claves
        out = {}
        for k, v in mags.items():
            kl = str(k).lower()
            if kl in {"botch", "falla", "pifia", "error"}:
                out["botch"] = v
            elif kl in {"partial", "parcial", "medio"}:
                out["partial"] = v
            elif kl in {"success", "exito", "ok", "normal"}:
                out["success"] = v
            elif kl in {"critical", "critico", "crit", "crits"}:
                out["critical"] = v
        if out:
            return out

    # Fallback: magnitude/damage/heal/power unico -> se asigna a success
    # y el resto se estima: botch=0, partial=mitad, critico=doble (heuristica razonable segun pool de dados Nd6: critico da el doble de exitos normalmente)
    probe = (
        entrada.get("magnitude")
        or entrada.get("damage")
        or entrada.get("heal")
        or entrada.get("power")
        or (isinstance(entrada.get("effect"), dict) and (
                entrada["effect"].get("damage")
                or entrada["effect"].get("heal")
                or entrada["effect"].get("power")
                or entrada["effect"].get("magnitude")
        ))
    )
    if probe is None:
        return None
    try:
        base = float(probe)
    except (TypeError, ValueError):
        return None

    warnings.append(f"{entry_id}: magnitudes inferidas por valor unico base={base}")
    return {
        "botch":    0.0,
        "partial":  round(base * 0.5, 2),
        "success":  round(base * 1.0, 2),
        "critical": round(base * 2.0, 2),
    }


def infiere_castingStat(entrada, classTags):
    """Si entry trae castingStat se usa. Si no, se busca por classTags:
    class_arcanist/bibliotecario... -> INT
    bardos / sanadores / canalizadores fe -> CAR
    guerreros / brutos -> CON (melee pool)
    aces / furia_salvaje / acechador -> DES
    Por defecto, None (se usara la stat primaria del CharacterSheet en runtime)"""
    cs = entrada.get("castingStat")
    if cs in {"CON", "DES", "INT", "CAR"}:
        return cs
    # por type/role
    role = (entrada.get("role") or "").lower()
    ctags = [str(x).lower() for x in lista_o_nada(classTags)]
    rolemap = {
        "caster":  "INT",
        "healer":  "CAR",
        "tank":    "CON",
        "martial": "CON",
        "striker": "DES",
        "ranged":  "DES",
        "support": "CAR",
        "skill":   "DES",
    }
    if role in rolemap:
        return rolemap[role]
    # Heuristicas por tags de clase
    if any("arcanist" in t or "wizard" in t or "mago" in t or "forja" in t for t in ctags):
        return "INT"
    if any("bard" in t or "cleri" in t or "devoto" in t or "sanador" in t for t in ctags):
        return "CAR"
    if any("furia" in t or "bruto" in t or "guardian" in t or "coloso" in t for t in ctags):
        return "CON"
    if any("acechador" in t or "ranger" in t or "picar" in t or "ladron" in t for t in ctags):
        return "DES"
    return None


# ---------------------------------------------------------------------------
# Normalizadores por categoria
# ---------------------------------------------------------------------------
def norm_generico(entry, prefijo, fallback_fname):
    """Normalizacion comun a todas las entidades: id, metadatos, flatten suave
    de campos que en Java eran anidados. Nunca borra campos originales; los
    conserva todos pero anade los normalizados con clave 'normalizado_*'."""
    out = dict(entry)           # shallow copy; luego se hace deep-copy a mano en campos necesarios
    warnings = []
    nid = id_seguro(entry, prefijo, fallback=fallback_fname)
    out["id"] = nid

    # Normaliza tier: int 0..5, o bien 0 si falta
    raw_tier = entry.get("tier", entry.get("tier_min") or entry.get("tierRequired") or 0)
    try:
        tier = int(raw_tier)
    except (TypeError, ValueError):
        warnings.append(f"{nid}: tier '{raw_tier}' no numerico, se pone 1")
        tier = 1
    out["tier"] = max(0, min(5, tier))

    # Normaliza rareza (si no la tiene, se asigna por tier + rarityOrder)
    rareza = (entry.get("rarity") or "").strip().lower()
    if rareza not in RAREZA_ORDEN:
        # por tier: 0->common, 1->uncommon, 2->rare, 3->epic, 4->epic, 5->legendary
        rareza_por_tier = ["common", "uncommon", "rare", "epic", "epic", "legendary"]
        rareza = rareza_por_tier[out["tier"]]
        warnings.append(f"{nid}: rareza ausente, asignada '{rareza}' por tier {out['tier']}")
    out["rarity"] = rareza

    # Nombre display (asegurado)
    out["name"] = (entry.get("name") or nid).strip()
    out["description"] = (
        entry.get("description")
        or (entry.get("effect") and entry["effect"].get("description") if isinstance(entry.get("effect"), dict) else None)
        or entry.get("lore")
        or entry.get("flavorText")
        or ""
    ).strip()
    out["flavorText"] = entry.get("flavorText") or ""

    return out, warnings


def norm_habilidad_o_hechizo(entry, prefijo, fallback_fname):
    base, w = norm_generico(entry, prefijo, fallback_fname)
    nid = base["id"]
    base["recovery"] = normaliza_recuperacion(entry.get("recovery"))
    base["actionType"] = normaliza_actionType(entry.get("actionType") or entry.get("action"))
    base["castingStat"] = infiere_castingStat(entry, entry.get("classTags"))
    base["magnitudeByDegree"] = infiere_magnitudes(entry, nid, w)
    base["target"] = (
        entry.get("target")
        or entry.get("skillTarget")
        or ("self" if entry.get("typeIcon") == "🛡" else "single_enemy")
    )
    # tags
    base["tags"] = list({
        *lista_o_nada(entry.get("mechanicTags")),
        *lista_o_nada(entry.get("classTags")),
        *lista_o_nada(entry.get("roleTags")),
    })
    # applyCondition (si conditionsInflicted lo declara)
    conds = lista_o_nada(entry.get("conditionsInflicted"))
    if conds:
        base["applyConditionId"] = conds[0]
        base["applyConditionMinDegree"] = entry.get("applyConditionMinDegree", "success")
        base["applyConditionRounds"] = entry.get("applyConditionRounds", 3)

    # Si es tipo invocacion / summon: marcamos grantedSummonId
    if prefijo in {"skill_", "spell_"} and entry.get("summonId"):
        base["grantedSummonId"] = entry["summonId"]
    # Heuristica: si mechanicTags contiene Invocar / Summon
    if any(isinstance(x,str) and any(k in x.lower() for k in ("invocar","summon","convocar"))
           for x in (entry.get("mechanicTags") or [])):
        # mejor que nada
        base["_hint_summon"] = True

    return base, w


def norm_equipo(entry, _prefijo, fallback_fname):
    """Equipo / arma / armadura / accesorio. Normaliza slot, peso, simbolos,
    statBonuses, rareza, precio heuristico."""
    base, w = norm_generico(entry, "equip_", fallback_fname)
    nid = base["id"]
    # Slot
    slot_raw = (entry.get("slot") or entry.get("equipSlot") or entry.get("type") or "").strip().lower()
    slot_map = [
        (("cabeza","casco","capucha","diadema","yelmo","bacinete","cimera"), "Head"),
        (("torso","armadura","peto","cota","coraza","tunica","manto","capa","brigantina","gambeson","camison","sayo"), "Torso"),
        (("piernas","perneras","grebas","calzas","faldar","faldon","quijotes"), "Legs"),
        (("pies","botas","botines","zuecos","pantuflas","escarpes","pisadas","pezunas"), "Feet"),
        (("mano","arma","daga","estilete","estoque","lanza","jabalina","hacha","maza","mazo","mandoble","mandoble","espada","espadon","bast�n","baston","baculo","cetro","guadana","sable","arco","honda","orbe","foco","grimorio","lentes","monoculo"), "MainHand"),
        (("escudo","broquel","rodela","paves","parry"), "OffHand"),
        (("accesorio","amuleto","anillo","argolla","talisman","simbolo","campana","caracola","tercer_ojo","sobrepelliz","calavera","simbologia","amulet"), "Accessory"),
    ]
    slot_ok = None
    for keys, s in slot_map:
        if any(k in slot_raw for k in keys):
            slot_ok = s
            break
    if not slot_ok:
        # Mirar si la compatibilidad armour/weapons lo delata
        comp = entry.get("compatibility") or {}
        if isinstance(comp, dict) and comp.get("weapons"):
            slot_ok = "MainHand"
        elif isinstance(comp, dict) and comp.get("armor"):
            slot_ok = "Torso"
        else:
            slot_ok = "Accessory"
            w.append(f"{nid}: slot no detectable, se asigna Accessory")
    base["slot"] = slot_ok

    # Peso △ ligera / ○ media / □ pesada
    peso_raw = (entry.get("weight") or entry.get("category") or entry.get("maxArmorWeight") or
                entry.get("maxWeaponWeight") or entry.get("peso") or entry.get("weightCategory") or "").strip().lower()
    peso_map = {
        "ligera": "light", "light": "light", "△": "light",
        "media":  "medium", "medium": "medium", "○": "medium", "medio": "medium",
        "pesada": "heavy", "heavy": "heavy", "□": "heavy", "pesado": "heavy",
    }
    wcat = peso_map.get(peso_raw, "medium")
    # si todavia no es claro, mirar simbolos
    simb = normaliza_simbolos(entry.get("compatibilitySymbols"))
    if peso_raw == "":
        if "△" in simb:  wcat = "light"
        elif "□" in simb: wcat = "heavy"
    base["weightCategory"] = wcat

    # Simbolos de compatibilidad
    base["compatibleSymbols"] = simb or normaliza_simbolos(
        list(lista_o_nada((entry.get("compatibility") or {}).get("weapons"))) +
        list(lista_o_nada((entry.get("compatibility") or {}).get("armor")))
    )

    # Stat bonuses normalizado: 4 keys fijas + cualquier extra en _extra
    stats_raw = dict_o_nada(entry.get("statBonuses"))
    stats_ok = {}
    for k in ("CON","DES","INT","CAR"):
        try:
            stats_ok[k] = int(stats_raw.get(k, 0) or 0)
        except (TypeError, ValueError):
            stats_ok[k] = 0
    base["statBonuses"] = stats_ok
    # Otros bonuses (armadura, CA, etc.)
    defense_bonuses = {}
    for k in ("ca", "armorBonus", "defensaMental", "resistenciaFisica", "precisionMagica", "bonusCA", "CA", "defMental"):
        if k in entry:
            try:
                defense_bonuses[k] = int(entry[k])
            except (TypeError, ValueError):
                pass
    base["defenseBonuses"] = defense_bonuses

    # Precio heuristico
    precio = entry.get("price") or entry.get("value") or entry.get("cost")
    if precio is None:
        base_precio = {"common":25,"uncommon":100,"rare":400,"epic":1800,"legendary":8000,"mythic":40000}
        precio = base_precio[base["rarity"]] + int(base["tier"]) * 75
        w.append(f"{nid}: precio ausente, estimado en {precio} (por rareza+tier)")
    try:
        base["price"] = int(precio)
    except (TypeError, ValueError):
        base["price"] = 50

    # GrantedTags (ej: "Arcano", "Armadura Ligera", etc.)
    tags = set(lista_o_nada(entry.get("grantedTags")))
    for t in lista_o_nada(entry.get("allowedEquipmentTags")):
        tags.add(t)
    for t in lista_o_nada(entry.get("mechanicTags")):
        tags.add(t)
    base["grantedTags"] = sorted(tags)

    return base, w


def norm_monstruo(entry, _prefijo, fallback_fname):
    """Monstruo/enemigo. Conserva stats, derived, attacks, passive, faction.
    El entry ya viene con todos los datos calculados; lo unico normalizamos
    es id/rareza y el skill list (declaramos skillIds desde attacks y
    conditionsInflicted en `_skill_refs` para que BattleStateOnegai pueda
    cargarlos al inicializar ICombatant.)"""
    base, w = norm_generico(entry, "monster_", fallback_fname)
    # Stats a formato dict fijo (si trae stats: {CON,DES,INT,CAR} lo usamos; si no, rellenamos con zeros)
    s = dict_o_nada(entry.get("stats"))
    stats_ok = {}
    for k in ("CON","DES","INT","CAR"):
        try:
            stats_ok[k] = int(s.get(k, 0) or 0)
        except (TypeError, ValueError):
            stats_ok[k] = 0
    base["stats"] = stats_ok
    # Defensas a dict fijo
    der = dict_o_nada(entry.get("derived"))
    def_ok = {
        "maxHealth":      int(der.get("vida", der.get("maxHealth", 0)) or 0),
        "ca":             int(der.get("ca", 10) or 10),
        "defMental":      int(der.get("defensaMental", 10) or 10),
        "resFis":         int(der.get("resistenciaFisica", 10) or 10),
        "precMag":        int(der.get("precisionMagica", 10) or 10),
        "movement":       int(der.get("movimiento", 5) or 5),
    }
    base["defenses"] = def_ok
    # Role
    base["role"] = (entry.get("role") or entry.get("rank") or "striker")
    base["faction"] = (entry.get("faction") or "neutral")
    base["factionName"] = entry.get("factionName") or base["faction"]
    # Attack refs (se guardan tal cual; la BattleState Onegai podra mapearlos a
    # SkillDefinition al vuelo o bien a una skill generica de ataque arma)
    base["attacks"] = lista_o_nada(entry.get("attacks"))
    base["conditionsInflicted"] = lista_o_nada(entry.get("conditionsInflicted"))
    base["passive"] = entry.get("passive")
    base["phases"] = lista_o_nada(entry.get("phases"))
    base["loot"] = lista_o_nada(entry.get("loot"))
    # skillIds que podemos inferir: por ahora vacio (se resolvera al cargar)
    base["skillIds"] = []
    return base, w


def norm_npc(entry, _prefijo, fallback_fname):
    """PNJ desde data/npcs/: role, faction, location, dialogue, services,
    agenda, secretHook."""
    base, w = norm_generico(entry, "npc_", fallback_fname)
    for k in ("role", "faction", "location", "dialogue", "services", "agenda",
              "attitude", "secretHook", "month"):
        if entry.get(k) is not None:
            base[k] = entry[k]
    return base, w


def norm_pj(entry, _prefijo, fallback_fname):
    """Ficha de personaje jugador pregenerado desde data/personatges/:
    claseId, razaId, transfonsId, stats CON/DES/INT/CAR, tier, habilidadIds."""
    base, w = norm_generico(entry, "pc_", fallback_fname)
    for k in ("claseId", "razaId", "transfonsId", "doteIds", "equipoIds",
              "habilidadIds", "tier", "historia"):
        if entry.get(k) is not None:
            base[k] = entry[k]
    st = {c: entry.get("stat" + c.title()) for c in ("con", "des", "int", "car")}
    if any(v is not None for v in st.values()):
        base["stats"] = {k.upper(): v for k, v in st.items() if v is not None}
    return base, w


def norm_loot(entry, _prefijo, fallback_fname):
    """Loot table. Entries deben venir con {entries:[{refId, weight, min, max}]};
    si no, lo adaptamos de cualquier estructura."""
    base, w = norm_generico(entry, "loot_", fallback_fname)
    nid = base["id"]
    entries = entry.get("entries")
    if not isinstance(entries, list):
        # Intentamos adivinar la estructura: a veces viene {"items":..., "coins":...}
        items = entry.get("items") or entry.get("lootItems") or entry.get("drops") or entry.get("table") or []
        if not isinstance(items, list):
            items = []
        coins = entry.get("coins") or entry.get("gold") or entry.get("oro")
        entries = []
        for it in items:
            if isinstance(it, dict):
                entries.append({
                    "refId":  it.get("refId") or it.get("id") or it.get("item"),
                    "weight": int(it.get("weight", it.get("chance", 1)) or 1),
                    "min":    int(it.get("min", it.get("count", 1)) or 1),
                    "max":    int(it.get("max", it.get("min", 1)) or 1),
                })
        if coins:
            try:
                entries.append({"refId":"__gold__", "weight":1, "min":int(coins), "max":int(coins)})
            except (TypeError, ValueError):
                pass
        if entries:
            w.append(f"{nid}: estructura loot no estandar, adaptada")
    base["entries"] = entries
    return base, w


def norm_aventura(entry, _prefijo, fallback_fname):
    """Aventura. Si es adventure.json de carpeta, lo aceptamos; si es un
    numero plano (1.json, 2.json...) lo normalizamos tal cual venga."""
    base, w = norm_generico(entry, "adv_", fallback_fname)
    return base, w


def norm_evento(entry, _prefijo, fallback_fname):
    base, w = norm_generico(entry, "ev_", fallback_fname)
    return base, w


# ---------------------------------------------------------------------------
# Motor de conversion por categoria
# ---------------------------------------------------------------------------
def procesa_carpeta_cartas(subdir, catalogo_key, prefijo):
    cartas_dir = os.path.join(DND_ROOT, "cartas", subdir)
    if not os.path.isdir(cartas_dir):
        return None, 0, []
    salidas = []
    warnings = []
    ficheros = sorted(glob.glob(os.path.join(cartas_dir, "*.json")))
    for f in ficheros:
        try:
            raw = lee_json(f)
        except Exception as e:
            warnings.append(f"[{catalogo_key}] Salto {os.path.basename(f)}: JSON invalido -> {e}")
            continue
        fname = os.path.splitext(os.path.basename(f))[0]
        if catalogo_key in ("skills", "spells"):
            entry, w = norm_habilidad_o_hechizo(raw, prefijo, fname)
        elif catalogo_key == "equipment":
            entry, w = norm_equipo(raw, prefijo, fname)
        elif catalogo_key == "monsters":
            entry, w = norm_monstruo(raw, prefijo, fname)
        else:
            entry, w = norm_generico(raw, prefijo, fname)
        warnings.extend(w)
        salidas.append(entry)
    return salidas, len(ficheros), warnings


def procesa_carpeta_raiz(subdir_raiz, catalogo_key, prefijo, fn_norm):
    root_dir = os.path.join(DND_ROOT, subdir_raiz)
    if not os.path.isdir(root_dir):
        return None, 0, []
    salidas = []
    warnings = []
    ficheros = sorted(glob.glob(os.path.join(root_dir, "*.json")))
    # Tambien capturamos <subdir>/*/adventure.json con glob recursivo
    ficheros.extend(sorted(glob.glob(os.path.join(root_dir, "**", "*.json"), recursive=True)))
    # deduplicar por ruta absoluta
    ficheros = list({os.path.abspath(x) for x in ficheros})
    ficheros.sort()
    for f in ficheros:
        try:
            raw = lee_json(f)
        except Exception as e:
            warnings.append(f"[{catalogo_key}] Salto {os.path.basename(f)}: JSON invalido -> {e}")
            continue
        fname = os.path.splitext(os.path.basename(f))[0]
        # las aventuras a veces se llaman "adventure.json"; si es asi, pillamos el nombre del parent dir
        if fname == "adventure":
            fname = os.path.basename(os.path.dirname(f)) or fname
        entry, w = fn_norm(raw, prefijo, fname)
        # Deshace colision id: ya prefijada; pero si dos adv se llaman igual
        # (mismo nombre de carpeta padre igual), hacemos append sha corto
        if any(x["id"] == entry["id"] for x in salidas):
            import hashlib
            suffix = hashlib.md5(f.encode("utf-8")).hexdigest()[:4]
            entry["id"] = f"{entry['id']}_{suffix}"
            warnings.append(f"{catalogo_key}: id duplicado {entry['id']}, desduplicado")
        warnings.extend(w)
        salidas.append(entry)
    return salidas, len(ficheros), warnings


def ordena_entries_por_id(entries):
    return sorted(entries, key=lambda e: (e.get("tier",0), e.get("id","")))



# ---------------------------------------------------------------------------
# Geografia: data/mapa/geografia.json -> locations.json
#
# Cierra la brecha B24 de GAMEMACHINE_NECESIDADES ("no hay 'localizacion' con
# nombre narrativo, faccion controladora, tier recomendado"). El mapa del mundo
# vivia solo en el motor de cartas -- 19 naciones con poligono, 95 ciudades con
# coordenadas y 26 zonas -- y no se exportaba a ningun sitio, asi que el motor
# grafico no tenia mundo: solo tres niveles de ciudad sueltos.
#
# Las tres clases de localizacion van al MISMO catalogo con un discriminador
# locationType, porque el consumidor generico ("dame todo lo de tier <= 2",
# "que hay en Udrax") no quiere saber si algo es nacion, ciudad o zona.
# ---------------------------------------------------------------------------
TAMANO_A_TIER = {"capital": 3, "ciudad": 2, "pueblo": 1, "aldea": 1}
PELIGRO_A_TIER = {"bajo": 1, "medio": 2, "alto": 3, "extremo": 4}


def exporta_geografia():
    """Devuelve (entries, warnings, meta). Lista vacia si no hay geografia."""
    ruta = os.path.join(DND_ROOT, "mapa", "geografia.json")
    if not os.path.exists(ruta):
        return [], [f"[locations] no existe {ruta}"], {}
    g = lee_json(ruta)
    w, entries = [], []
    canon = g.get("_canon", {})

    for n in g.get("naciones", []):
        nid = "nation_" + re.sub(r"[^a-z0-9_]+", "_", n["nombre"].lower()).strip("_")
        entries.append({
            "id": nid, "name": n["nombre"], "locationType": "nation",
            "tier": 0, "rarity": "common",
            "polygon": n.get("points", ""), "color": n.get("color"),
            "culture": n.get("cultura") or None,
            "alignment": n.get("alineacion") or None,
            "deityIds": n.get("deidadIds") or [], "raceIds": n.get("razaIds") or [],
        })

    for c in g.get("ciudades", []):
        tam = (c.get("tamano") or "").lower()
        if tam not in TAMANO_A_TIER:
            w.append(f"{c.get('id','?')}: tamano '{tam}' desconocido, tier 1")
        if c.get("pendienteCanon"):
            w.append(f"{c.get('id','?')}: sin nombre canonico todavia")
        entries.append({
            "id": c.get("id") or ("city_" + str(c.get("nombre", "?"))),
            "name": c.get("nombre", "?"), "locationType": "city",
            "tier": TAMANO_A_TIER.get(tam, 1), "rarity": "common",
            "settlementSize": tam or None,
            "controlledBy": c.get("nacion"),
            "position": {"x": c.get("x"), "y": c.get("y")},
            "population": c.get("poblacion") or None,
            "ruler": c.get("gobernante") or None,
            "trait": c.get("rasgo") or None,
            "description": c.get("descripcion") or None,
            "storyIds": c.get("historiaIds") or [], "npcIds": c.get("npcIds") or [],
            "pendingCanonName": bool(c.get("pendienteCanon")),
        })

    for z in g.get("zonas", []):
        pel = (z.get("peligro") or "").lower()
        entries.append({
            "id": z.get("id") or ("zone_" + str(z.get("nombre", "?"))),
            "name": z.get("nombre", "?"), "locationType": "zone",
            "tier": PELIGRO_A_TIER.get(pel, 1), "rarity": "common",
            "terrain": z.get("tipo") or None,
            "controlledBy": z.get("nacion") or None,
            "danger": pel or None,
            "description": z.get("descripcion") or None,
            "position": {"x": z.get("x"), "y": z.get("y")},
            "storyIds": z.get("historiaIds") or [],
        })

    meta = {"epoca": canon.get("epoca"), "era": canon.get("era"),
            "fuente_nombres": canon.get("fuente_nombres")}
    return entries, w, meta


def catalogo_wrapper(entries):
    return {"entries": ordena_entries_por_id(entries)}


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def main():
    asegura_dir(SALIDA_DIR)
    estadisticas = {}
    todas_warnings = []

    # 1) subdirs de data/cartas/
    for subdir, (catalogo, prefijo) in SUBIR_A_CATALOGO.items():
        entries, n_fic, w = procesa_carpeta_cartas(subdir, catalogo, prefijo)
        todas_warnings.extend(w)
        if entries is None:
            estadisticas[catalogo] = {"fuente": f"cartas/{subdir}", "status":"no_existe", "entradas": 0, "ficheros": 0}
            continue
        # controlar ids unicos
        ids = [e["id"] for e in entries]
        dup = [x for x in set(ids) if ids.count(x)>1]
        if dup:
            todas_warnings.append(f"[{catalogo}] IDS DUPLICADOS: {dup[:10]}")
        stats = {
            "fuente":    f"cartas/{subdir}",
            "status":    "ok",
            "entradas":  len(entries),
            "ficheros":  n_fic,
            "ids_unicos": len(set(ids)),
            "ids_duplicados": dup,
        }
        estadisticas[catalogo] = stats
        guarda_json(os.path.join(SALIDA_DIR, f"{catalogo}.json"), catalogo_wrapper(entries))

    # 2) data/ raiz
    map_norm = {
        "loot":      norm_loot,
        "aventuras": norm_aventura,
        "npcs":          norm_npc,
        "personatges":   norm_pj,
        "eventos":   norm_evento,
    }
    for subdir, (catalogo, prefijo) in DATA_RAIZ_A_CATALOGO.items():
        fn = map_norm[subdir]
        entries, n_fic, w = procesa_carpeta_raiz(subdir, catalogo, prefijo, fn)
        todas_warnings.extend(w)
        if entries is None:
            estadisticas[catalogo] = {"fuente": subdir, "status":"no_existe", "entradas": 0, "ficheros":0}
            continue
        ids = [e["id"] for e in entries]
        dup = [x for x in set(ids) if ids.count(x)>1]
        stats = {
            "fuente": subdir,
            "status": "ok",
            "entradas": len(entries),
            "ficheros": n_fic,
            "ids_unicos": len(set(ids)),
            "ids_duplicados": dup,
        }
        estadisticas[catalogo] = stats
        guarda_json(os.path.join(SALIDA_DIR, f"{catalogo}.json"), catalogo_wrapper(entries))

    # 2b) geografia -> locations.json
    entries, w, meta = exporta_geografia()
    todas_warnings.extend(w)
    if entries:
        env = catalogo_wrapper(entries)
        env["_mapa"] = meta
        guarda_json(os.path.join(SALIDA_DIR, "locations.json"), env)
        porTipo = {}
        for e in entries:
            porTipo[e["locationType"]] = porTipo.get(e["locationType"], 0) + 1
        estadisticas["locations"] = {
            "fuente": "mapa/geografia.json", "status": "ok",
            "entradas": len(entries), "ficheros": 1,
            "ids_unicos": len({e["id"] for e in entries}), "ids_duplicados": [],
            "por_tipo": porTipo, "epoca": meta.get("epoca"),
        }
    else:
        estadisticas["locations"] = {"fuente": "mapa/geografia.json",
                                     "status": "no_existe", "entradas": 0, "ficheros": 0}

    # 3) trasfondos = el GDD esperaba 12 pero la carpeta data/cartas/trasfondos no existe.
    #    Consultamos si data/kits/kits_iniciales.json (o similares) pueden proveerlos; si no,
    #    generamos backgrounds.json vacio y anotamos.
    bg_path = os.path.join(SALIDA_DIR, "backgrounds.json")
    if not os.path.exists(bg_path):
        # Miramos si hay algo en data/kits
        kits_path = os.path.join(DND_ROOT, "kits", "kits_iniciales.json")
        if os.path.exists(kits_path):
            entries = []
            try:
                kits = lee_json(kits_path)
                if isinstance(kits, dict) and "kits" in kits and isinstance(kits["kits"], list):
                    raw_list = kits["kits"]
                elif isinstance(kits, list):
                    raw_list = kits
                else:
                    raw_list = [kits]
                for i, raw in enumerate(raw_list):
                    entry, w = norm_generico(raw, "bg_", fallback_fname=f"kit_{i}")
                    entries.append(entry)
                    todas_warnings.extend(w)
            except Exception as e:
                todas_warnings.append(f"[backgrounds] kits_iniciales.json no usable: {e}")
            if entries:
                guarda_json(bg_path, catalogo_wrapper(entries))
                estadisticas["backgrounds"] = {
                    "fuente": "kits/kits_iniciales.json (fallback)",
                    "status": "ok_fallback",
                    "entradas": len(entries),
                    "ficheros": 1,
                    "ids_unicos": len({e["id"] for e in entries}),
                    "ids_duplicados": [],
                }
        else:
            guarda_json(bg_path, catalogo_wrapper([]))
            todas_warnings.append("[backgrounds] data/cartas/trasfondos no existe y kits_iniciales.json no usable -> catalogo VACIO")
            estadisticas["backgrounds"] = {"fuente": "-", "status": "vacio", "entradas": 0, "ficheros": 0}

    # 4) Escribir manifiesto
    manifest = {
        "version": 1,
        "generado_por": "tools/convertir_definiciones_rpg.py",
        "fuente_dnd": DND_ROOT,
        "motor_salida": SALIDA_DIR,
        "total_catalogos": len(estadisticas),
        "total_entradas":  sum(s.get("entradas",0) for s in estadisticas.values()),
        "catalogos":       estadisticas,
        "warnings_count":  len(todas_warnings),
        "warnings":        todas_warnings[-500:],
    }
    guarda_json(os.path.join(SALIDA_DIR, "MANIFIESTO.json"), manifest)

    # 5) Salida a consola
    print("=" * 70)
    print("CONVERSION DEFINICIONES RPG — RESULTADO")
    print("=" * 70)
    total = 0
    for k, s in sorted(estadisticas.items()):
        print(f"  {k:<24s}  {s.get('status','?'):<14s}  entradas={s.get('entradas',0):>5d}  (fich={s.get('ficheros',0)})  {s.get('fuente','')}")
        total += s.get("entradas", 0)
    print(f"\n  {'TOTAL':<24s}  {' ':<14s}  entradas={total:>5d}")
    print(f"  warnings: {len(todas_warnings)} (ver MANIFIESTO.json para detalle ultimos 500)")
    print("=" * 70)
    return 0


if __name__ == "__main__":
    sys.exit(main())
