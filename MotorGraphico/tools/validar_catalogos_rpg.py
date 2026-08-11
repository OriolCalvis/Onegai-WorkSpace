#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
validar_catalogos_rpg.py
------------------------
Valida cada catalogo de assets/catalogs/*.json contra un contrato P0:
  - raiz tiene clave "entries" con lista
  - cada entry tiene id, name, tier, rarity (campos normalizados obligatorios)
  - ids unicos por catalogo
  - para ciertos catalogos, se validan campos extra (P0 minimo):
      classes  -> baseHealth, primaryStat, startingCards, role, maxSkillCards
      races    -> statBonuses, speed
      skills/spells -> recovery, actionType, target, castingStat?, magnitudeByDegree?
      equipment-> slot, weightCategory, price, compatibleSymbols
      monsters -> stats{CON,DES,INT,CAR}, defenses, role, attacks
      loot_tables-> entries (interna) lista no vacia o al menos []
Ademas:
  - tier 0..5
  - rarity in rarityOrder
  - statBonuses en el catalogo, si existen, solo tienen CON/DES/INT/CAR y enteros
  - recovery/actionType solo valores normalizados aceptados
Si algo falla, devuelve exit code != 0 y escribe fallos por salida estandar.
Uso:  python3 validar_catalogos_rpg.py
"""

import json
import os
import sys
import glob

HERE = os.path.abspath(os.path.dirname(__file__))
ROOT = os.path.abspath(os.path.join(HERE, ".."))
CAT_DIR = os.path.join(ROOT, "assets", "catalogs")

RECOVERY_OK = {"activo", "descanso_corto", "descanso_largo", "pasiva"}
ACTION_OK = {"accion_principal", "movimiento", "reaccion", "canalizacion", "libre", "pasiva"}
RARITY_OK = {"common", "uncommon", "rare", "epic", "legendary", "mythic"}
STATS = ("CON", "DES", "INT", "CAR")
SLOTS_OK = {"Head", "Torso", "Legs", "Feet", "MainHand", "OffHand", "Accessory", "Consumable", "TwoHanded"}
WEIGHT_OK = {"light", "medium", "heavy"}
SYMBOLS_OK = {"△", "○", "□", "✦", "☠", "✝", "♞", "⚙", "?"}


def lee(p):
    with open(p, "r", encoding="utf-8") as f:
        return json.load(f)


def check(cond, msg, fallos):
    if not cond:
        fallos.append(msg)


def valida_generico(entry, path, idx, fallos, obligatorios_extra=None):
    pfx = f"{path} entry[{idx}] id={entry.get('id','?')}"
    for k in ("id", "name", "tier", "rarity"):
        check(k in entry and entry[k] not in (None, ""), f"{pfx}: falta {k}", fallos)
    # tipo valores
    if "tier" in entry:
        t = entry["tier"]
        check(isinstance(t, int) and 0 <= t <= 5, f"{pfx}: tier fuera de rango {t}", fallos)
    if "rarity" in entry:
        r = entry["rarity"]
        check(r in RARITY_OK, f"{pfx}: rareza no permitida '{r}'", fallos)
    if isinstance(entry.get("statBonuses"), dict):
        sb = entry["statBonuses"]
        for k, v in sb.items():
            check(k in STATS, f"{pfx}: statBonuses[{k}] no es {STATS}", fallos)
            check(isinstance(v, int), f"{pfx}: statBonuses[{k}]={v!r} no es entero", fallos)
    if obligatorios_extra:
        for k in obligatorios_extra:
            check(k in entry and entry[k] is not None, f"{pfx}: falta {k}", fallos)


def valida_sb_dict(entry, key, path, idx, fallos):
    pfx = f"{path} entry[{idx}] id={entry.get('id','?')}"
    sb = entry.get(key)
    check(isinstance(sb, dict), f"{pfx}: {key} no es dict", fallos)
    if isinstance(sb, dict):
        for s in STATS:
            v = sb.get(s, 0)
            check(isinstance(v, int), f"{pfx}: {key}[{s}]={v!r} no entero", fallos)


def main():
    fallos = []
    catalogs = sorted(glob.glob(os.path.join(CAT_DIR, "*.json")))
    catalogs = [c for c in catalogs if not c.endswith("MANIFIESTO.json")]
    if not catalogs:
        print(f"[FATAL] No hay catalogos en {CAT_DIR}")
        return 2
    stats = {}
    for p in catalogs:
        nombre = os.path.basename(p)[:-5]
        try:
            data = lee(p)
        except Exception as e:
            fallos.append(f"{p}: JSON INVALIDO: {e}")
            continue
        entries = data.get("entries")
        if not isinstance(entries, list):
            fallos.append(f"{p}: raiz no tiene 'entries' lista")
            continue
        ids = []
        for i, e in enumerate(entries):
            valida_generico(e, nombre, i, fallos)
            if "id" in e and e["id"]:
                ids.append(e["id"])
            # por catalogo
            if nombre in {"classes"}:
                valida_generico(e, nombre, i, fallos, obligatorios_extra=("primaryStat", "baseHealth", "role"))
                if isinstance(e.get("startingCards"), dict):
                    pass   # ok
            elif nombre == "races":
                valida_sb_dict(e, "statBonuses", nombre, i, fallos)
                check(isinstance(e.get("speed", 0), int),
                      f"{nombre}[{i}] id={e.get('id','?')}: speed no entero", fallos)
            elif nombre in {"skills", "spells"}:
                rec = (e.get("recovery") or "").lower()
                check(rec in RECOVERY_OK, f"{nombre}[{i}] id={e.get('id','?')}: recovery '{rec}' no valido",
                      fallos)
                act = (e.get("actionType") or "").lower()
                check(act in ACTION_OK, f"{nombre}[{i}] id={e.get('id','?')}: actionType '{act}' no valido",
                      fallos)
                cs = e.get("castingStat")
                check(cs in (None, *STATS),
                      f"{nombre}[{i}] id={e.get('id','?')}: castingStat={cs!r} no valido", fallos)
                mg = e.get("magnitudeByDegree")
                if mg is not None:
                    check(isinstance(mg, dict),
                          f"{nombre}[{i}] id={e.get('id','?')}: magnitudeByDegree no dict", fallos)
            elif nombre == "equipment":
                slot = e.get("slot")
                wc = e.get("weightCategory")
                syms = e.get("compatibleSymbols") or []
                check(slot in SLOTS_OK, f"{nombre}[{i}] id={e.get('id','?')}: slot '{slot}' no valido (esperado {SLOTS_OK})", fallos)
                check(wc in WEIGHT_OK, f"{nombre}[{i}] id={e.get('id','?')}: weightCategory '{wc}' no valido", fallos)
                check(isinstance(syms, list) and all(s in SYMBOLS_OK for s in syms),
                      f"{nombre}[{i}] id={e.get('id','?')}: compatibleSymbols {syms!r} no valido", fallos)
                check(isinstance(e.get("price"), int) and e.get("price", 0) >= 0,
                      f"{nombre}[{i}] id={e.get('id','?')}: price no entero >=0", fallos)
            elif nombre == "monsters":
                valida_sb_dict(e, "stats", nombre, i, fallos)
                defs = e.get("defenses")
                check(isinstance(defs, dict), f"{nombre}[{i}] id={e.get('id','?')}: defenses no dict", fallos)
                if isinstance(defs, dict):
                    for k in ("maxHealth", "ca", "defMental", "resFis", "precMag", "movement"):
                        check(isinstance(defs.get(k, 0), int),
                              f"{nombre}[{i}] id={e.get('id','?')}: defenses.{k} no entero", fallos)
                check(isinstance(e.get("attacks"), list),
                      f"{nombre}[{i}] id={e.get('id','?')}: attacks no lista", fallos)
            elif nombre == "loot_tables":
                check(isinstance(e.get("entries"), list),
                      f"{nombre}[{i}] id={e.get('id','?')}: loot entries no lista", fallos)
        # ids unicos
        dupes = sorted({x for x in ids if ids.count(x) > 1})
        if dupes:
            fallos.append(f"{p}: IDS DUPLICADOS ({len(dupes)}): {dupes[:8]}...")
        stats[nombre] = len(entries)

    # Reporte
    print("=" * 68)
    print("VALIDACION CATALOGOS RPG — RESULTADO")
    print("=" * 68)
    total = 0
    for k, n in sorted(stats.items()):
        print(f"  {k:<24s}  entries={n:>5d}")
        total += n
    print(f"\n  {'TOTAL':<24s}  entries={total:>5d}")
    print("=" * 68)
    if fallos:
        print(f"\n[FAIL] {len(fallos)} fallos:")
        for m in fallos[:200]:
            print(" -", m)
        if len(fallos) > 200:
            print(f"   ... y {len(fallos)-200} mas")
        return 1
    print("[OK] Sin fallos.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
