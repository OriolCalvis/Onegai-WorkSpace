#!/usr/bin/env python3
"""Reanomena 5 cartes canòniques d'espanyol a anglès i actualitza les referències creuades.

Fa dues cosos en una passada:
  1. Per cada carta: reanomena el fitxer (`skill_shield_bash.json` → `skill_shield_bash.json`)
     i actualitza el camp `id` intern.
  2. Per cada referència creuada (en altres .json, personatges, loot, enemigos, classes,
     linkedSkill...): substitueix el valor ES pel valor EN.

Mapa de reanomenats (extret de l'auditoria del catàleg):
  skill_shield_bash  → skill_shield_bash       (1 fitxer + 6 referències)
  spell_ember_lance  → spell_ember_lance       (1 fitxer + 0 referències)
  passive_living_wall    → passive_living_wall     (1 fitxer + 0 referències)
  item_reinforced_shield → item_reinforced_shield  (1 fitxer + 4 referències)
  item_short_sword     → item_short_sword        (1 fitxer + ~43 referències)

Ús:
  python3 scripts/renombrar_canonicos_es_en.py           # executa
  python3 scripts/renombrar_canonicos_es_en.py --dry-run # només llista què faria
"""
from __future__ import annotations
import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DATA = ROOT / "data"
SCRIPTS = ROOT / "scripts"

# (id_vell_es, id_nou_en, carpeta_on_viu_la_carta)
RENAMES = [
    ("skill_shield_bash",  "skill_shield_bash",      DATA / "cartas/habilidades"),
    ("spell_ember_lance",  "spell_ember_lance",      DATA / "cartas/hechizos"),
    ("passive_living_wall",    "passive_living_wall",    DATA / "cartas/pasivas"),
    ("item_reinforced_shield", "item_reinforced_shield", DATA / "cartas/armas"),
    ("item_short_sword",     "item_short_sword",       DATA / "cartas/armas"),
]


def actualitzar_id_intern(path: Path, id_vell: str, id_nou: str, dry: bool) -> bool:
    """Llegeix el JSON d'una carta, canvia el camp `id` i el torna a escriure."""
    try:
        obj = json.loads(path.read_text(encoding="utf-8"))
    except Exception as e:
        print(f"  ⚠ {path.name}: no s'ha pogut llegir ({e})")
        return False
    if obj.get("id") != id_vell:
        print(f"  ⚠ {path.name}: id intern={obj.get('id')!r}, esperava {id_vell!r}")
        return False
    obj["id"] = id_nou
    if not dry:
        path.write_text(json.dumps(obj, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return True


def substituir_referencies_en_fitxer(path: Path, mapa: dict[str, str], dry: bool) -> int:
    """Substitueix els ids ES per EN com a strings literals dins d'un fitxer JSON.

    Com que els ids són sempre snake_case sense ambigüitats (no apareixen com a substring
    d'altres ids), és segur fer un replace directe sobre el text.
    """
    try:
        text = path.read_text(encoding="utf-8")
    except Exception:
        return 0
    nou_text = text
    count = 0
    for id_vell, id_nou in mapa.items():
        # Només substitueix quan apareix com a string entre cometes o com a paraula clau.
        # Usem una regex amb boundaries de paraula per evitar falsos positius.
        pat = re.compile(r'(?<![A-Za-z0-9_])' + re.escape(id_vell) + r'(?![A-Za-z0-9_])')
        nou_text, n = pat.subn(id_nou, nou_text)
        count += n
    if count > 0 and not dry and nou_text != text:
        path.write_text(nou_text, encoding="utf-8")
    return count


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    mapa = {v: n for v, n, _ in RENAMES}

    print("═══ PAS 1: reanomenar fitxers de carta i id intern ═══")
    renombrats = 0
    for id_vell, id_nou, carpeta in RENAMES:
        vell = carpeta / f"{id_vell}.json"
        nou = carpeta / f"{id_nou}.json"
        if not vell.exists():
            print(f"  ⊘ {id_vell}: fitxer no trobat ({vell})")
            continue
        if nou.exists():
            print(f"  ⚠ {id_vell} → {id_nou}: el destí ja existeix, no sobreescric")
            continue
        # 1. Actualitza l'id intern
        if not actualitzar_id_intern(vell, id_vell, id_nou, args.dry_run):
            continue
        # 2. Reanomena el fitxer
        if not args.dry_run:
            vell.rename(nou)
        print(f"  ✓ {id_vell} → {id_nou}")
        renombrats += 1

    print(f"\n═══ PAS 2: actualitzar referències creuades ═══")
    print(f"  Mapa: {len(mapa)} ids a substituir")
    total_refs = 0
    fitxers_tocats = 0

    # Busca a tot data/ i scripts/
    directoris = [DATA, SCRIPTS]
    for dirbase in directoris:
        if not dirbase.exists():
            continue
        for path in sorted(dirbase.rglob("*.json")):
            n = substituir_referencies_en_fitxer(path, mapa, args.dry_run)
            if n > 0:
                rel = path.relative_to(ROOT)
                print(f"  ✓ {rel}: {n} substitució/ns")
                total_refs += n
                fitxers_tocats += 1
        # També scripts Python (.py) per si hi ha literals
        for path in sorted(dirbase.rglob("*.py")):
            n = substituir_referencies_en_fitxer(path, mapa, args.dry_run)
            if n > 0:
                rel = path.relative_to(ROOT)
                print(f"  ✓ {rel}: {n} substitució/ns (.py)")
                total_refs += n
                fitxers_tocats += 1

    print(f"\n═══ Resum ═══")
    print(f"  Cartes reanomenades: {renombrats}/{len(RENAMES)}")
    print(f"  Fitxers amb referències actualitzades: {fitxers_tocats}")
    print(f"  Referències substituïdes: {total_refs}")
    if args.dry_run:
        print("  (mode dry-run: cap canvi escrit)")


if __name__ == "__main__":
    main()
