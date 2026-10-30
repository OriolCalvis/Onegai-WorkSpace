#!/usr/bin/env python3
"""NIVELES-01 — El nivel que pide cada pieza de equipo para llevarla puesta.

Oriol (06/10/2026): «podríamos limitar ciertos objetos con los niveles, para
nivelar el progreso». El nivel no da atributos; los atributos suben con el
equipo. Así que el nivel decide CUÁNDO se puede llevar una pieza, y el equipo
de un tier se reparte a lo largo de sus veinte niveles.

La regla (por tier y por lo que suma la pieza a los atributos):

    Tier 1 (niveles 1-20)    común: 1 · poco común +0/+1: 5 · poco común +2: 10
    Tier 2 (niveles 21-40)   +0/+1: 20 · +2: 26
    Tier 3 (niveles 41-60)   +0/+2: 40 · +3: 46 · +4: 52
    Tier 4 (niveles 61-80)   60

El equipo inicial de cualquier clase (`startingEquipment` en classes.json) es
de nivel 1 siempre (la armadura de placas del Coloso de Ceniza es de Tier 2 y
se lleva desde el principio si llegas a CON 3). Las piezas que deban salirse de la regla (una recompensa de
misión, un botín raro) se fijan a mano en EXCEPCIONES, y esas mandan. El kit
del cofre se elige con un héroe de nivel 1, así que solo lleva piezas de nivel
1: test_kit_basico comprueba que las 63 clases siguen saliendo exactas.

Uso:
    python3 tools/niveles_equipo.py          # escribe requiredLevel en equipment.json
    python3 tools/niveles_equipo.py --ver    # solo cuenta cuántas quedan en cada nivel
"""
import argparse
import collections
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
EQUIPO = ROOT / "assets/catalogs/equipment.json"
CLASES = ROOT / "assets/catalogs/classes.json"

# id -> nivel, para las piezas que no siguen la regla.
#
# Las poco comunes que el kit del cofre necesita para salir exacto (+2, +2,
# +1, +1) en alguna de las 63 clases (`simulador_dificultad kits`, 06/10/2026):
# son equipo de partida, nivel 1. Si una clase nueva cambia los kits,
# test_kit_basico lo dice (deja de salir exacto) y se añade aquí.
PIEZAS_DEL_KIT = [
    "antifaz_de_la_vigilia", "botas_de_la_ultima_guardia", "calzas_del_ratero_fino", "casco_de_abordaje",
    "corona_de_ramas_secas", "diadema_de_lectora", "faldon_del_oficiante", "grebas_del_corredor_de_muros",
    "grebas_runicas_menores", "mascara_del_duelista", "perneras_del_vadeador", "sandalias_del_vado_sagrado",
    "sobrevesta_del_peregrino", "tunica_del_archivista", "zuecos_del_alquimista",
]
EXCEPCIONES = {pieza: 1 for pieza in PIEZAS_DEL_KIT}


def suma(pieza):
    bonos = pieza.get("statBonuses") or {}
    return sum(v for v in bonos.values() if isinstance(v, (int, float)))


def equipo_inicial():
    """Las piezas del equipo inicial de alguna clase: se llevan desde el nivel 1."""
    ids = set()
    for clase in json.loads(CLASES.read_text(encoding="utf-8"))["entries"]:
        inicial = clase.get("startingEquipment") or {}
        listas = inicial.values() if isinstance(inicial, dict) else [inicial]
        for lista in listas:
            for x in (lista if isinstance(lista, list) else [lista]):
                if isinstance(x, str):
                    ids.add(x)
                elif isinstance(x, dict) and x.get("id"):
                    ids.add(x["id"])
    return ids


def nivel_por_regla(pieza, iniciales):
    tier = pieza.get("tier", 1)
    if pieza["id"] in iniciales:
        return 1
    rareza = pieza.get("rarity", "common")
    s = suma(pieza)
    if tier <= 1:
        if rareza == "common":
            return 1
        return 10 if s >= 2 else 5
    if tier == 2:
        return 26 if s >= 2 else 20
    if tier == 3:
        return 52 if s >= 4 else (46 if s >= 3 else 40)
    return 60


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ver", action="store_true")
    args = ap.parse_args()
    raw = EQUIPO.read_text(encoding="utf-8")
    doc = json.loads(raw)
    cuenta = collections.Counter()
    iniciales = equipo_inicial()
    for pieza in doc["entries"]:
        nivel = EXCEPCIONES.get(pieza["id"], nivel_por_regla(pieza, iniciales))
        cuenta[(pieza.get("tier"), nivel)] += 1
        if not args.ver:
            pieza["requiredLevel"] = nivel
    for (tier, nivel), n in sorted(cuenta.items()):
        print(f"tier {tier}  nivel {nivel:>2}: {n} piezas")
    if not args.ver:
        # El mismo formato del fichero: sangría 2, claves ordenadas, UTF-8.
        EQUIPO.write_text(json.dumps(doc, indent=2, ensure_ascii=False, sort_keys=True), encoding="utf-8")


if __name__ == "__main__":
    main()
