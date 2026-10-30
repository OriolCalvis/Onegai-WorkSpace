#!/usr/bin/env python3
"""Genera 200 tablas de loot en data/loot/ (Plantilla §16).

  - 75 de facción: 15 facciones × 5 tiers (lo que sueltan sus miembros).
  - 50 de jefe: una por enemigo importante (villainProfile), con drop mítico condicionado
    a alcanzar la fase de Desesperación y artefacto de su deidad vinculada.
  - 75 de contenedor: 5 entornos × 5 tiers × 3 tipos (cofre, alijo, relicario).

Reglas §16: drops con ids reales; suma de sentido (común 60-100 · uncommon 25-60 ·
rare 10-25 · epic 5-10 · mythic solo por condición); toda tabla puede soltar una PISTA.
data/loot/ queda fuera de data/cartas para no interferir con los repositorios de la app.
"""

from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "data/loot"
CARTAS = ROOT / "data/cartas"

ORO = {1: (5, 15), 2: (10, 30), 3: (25, 60), 4: (50, 120), 5: (100, 250)}


def cargar(carpeta):
    return {p.stem: json.loads(p.read_text(encoding="utf-8")) for p in (CARTAS / carpeta).glob("*.json")}


def escribir(path: Path, payload: dict) -> bool:
    if path.exists():
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    return True


def main() -> None:
    armas = cargar("armas")
    enemigos = cargar("enemigos")
    deidades = cargar("deidades")

    comunes = sorted(i for i, a in armas.items() if a["rarity"] == "common" and not a.get("artifact"))
    uncommon = sorted(i for i, a in armas.items() if a["rarity"] == "uncommon")
    raras = sorted(i for i, a in armas.items() if a["rarity"] == "rare")
    epicas = sorted(i for i, a in armas.items() if a["rarity"] == "epic" and not a.get("artifact"))
    artefactos_por_deidad = {}
    for i, a in armas.items():
        if a.get("artifact"):
            artefactos_por_deidad.setdefault(a["deity"], []).append(i)
    for v in artefactos_por_deidad.values():
        v.sort()

    facciones = sorted({e["faction"] for e in enemigos.values()})
    importantes = sorted(i for i, e in enemigos.items() if "villainProfile" in e)

    def pick(lista, semilla, n=1):
        return [lista[(semilla * 7 + k * 13) % len(lista)] for k in range(n)]

    escritos = 0

    # --- 75 tablas de facción (15 × 5 tiers) ---
    for fi, fac in enumerate(facciones):
        pool_fac = sorted({item for e in enemigos.values() if e["faction"] == fac for item in e.get("loot", [])})
        for tier in range(1, 6):
            drops = [{"item": it, "chance": 60, "condition": None} for it in pool_fac[:2]]
            drops += [{"item": pick(uncommon, fi * 5 + tier)[0], "chance": max(15, 50 - tier * 5), "condition": None}]
            if tier >= 2:
                drops.append({"item": pick(raras, fi + tier)[0], "chance": 10 + tier * 3, "condition": None})
            if tier >= 4:
                drops.append({"item": pick(epicas, fi + tier)[0], "chance": 5 + tier, "condition": "Solo si el grupo derrotó a una élite de la facción en la escena"})
            tabla = {
                "id": f"loot_{fac}_t{tier}", "name": f"Botín: {fac.replace('_', ' ').title()} (tier {tier})",
                "type": "loot_table", "tierRange": [tier, tier],
                "drops": drops, "gold": {"min": ORO[tier][0], "max": ORO[tier][1]},
                "clue": f"clue_{fac}" if tier >= 3 else None,
                "flavorText": "Lo que la facción lleva encima cuando cree que va a ganar.",
            }
            escritos += escribir(OUT / f"{tabla['id']}.json", tabla)

    # --- 50 tablas de jefe (enemigos importantes) ---
    for bi, bid in enumerate(importantes):
        e = enemigos[bid]
        vinculo = e["villainProfile"]["vinculo"]
        arte_pool = artefactos_por_deidad.get(vinculo)
        if not arte_pool:  # vínculo a clase/facción: usa la deidad cuyo compatibleWith lo incluya, o rota
            candidatos = [did for did, d in deidades.items() if vinculo in d.get("compatibleWith", [])]
            arte_pool = artefactos_por_deidad[candidatos[0]] if candidatos else artefactos_por_deidad[sorted(artefactos_por_deidad)[bi % 20]]
        tier = e["tier"]
        drops = [
            {"item": "pocion_de_vigor", "chance": 100, "condition": None},
            {"item": pick(raras, bi)[0], "chance": 60, "condition": None},
            {"item": pick(epicas, bi)[0], "chance": 25, "condition": "Derrotarlo sin que ningún aliado quede Moribundo"},
            {"item": arte_pool[bi % len(arte_pool)], "chance": 100,
             "condition": "Solo si el combate alcanzó la fase de Desesperación (el artefacto se gana, no se recoge)"},
        ]
        tabla = {
            "id": f"loot_{bid}", "name": f"Botín de {e['name']}",
            "type": "loot_table", "tierRange": [tier, tier],
            "drops": drops, "gold": {"min": ORO[tier][0] * 3, "max": ORO[tier][1] * 3},
            "clue": f"clue_{e['faction']}",
            "flavorText": e.get("flavorText", ""),
        }
        escritos += escribir(OUT / f"{tabla['id']}.json", tabla)

    # --- 75 tablas de contenedor (5 entornos × 5 tiers × 3 tipos) ---
    ENTORNOS = ["cripta", "bosque", "ciudad", "montana", "costa"]
    TIPOS = [("cofre", "Un cofre con más historia que cerradura."),
             ("alijo", "Escondido deprisa por alguien que no volvió."),
             ("relicario", "Pequeño, sagrado y probablemente vigilado.")]
    for ei, ent in enumerate(ENTORNOS):
        for tier in range(1, 6):
            for ti, (tipo, flavor) in enumerate(TIPOS):
                semilla = ei * 31 + tier * 7 + ti * 3
                drops = [
                    {"item": pick(comunes, semilla)[0], "chance": 80, "condition": None},
                    {"item": pick(uncommon, semilla)[0], "chance": 40, "condition": None},
                ]
                if tier >= 2:
                    drops.append({"item": pick(raras, semilla)[0], "chance": 10 + tier * 4, "condition": None})
                if tier >= 4 and tipo == "relicario":
                    did = sorted(artefactos_por_deidad)[semilla % 20]
                    drops.append({"item": artefactos_por_deidad[did][semilla % 5], "chance": 8,
                                  "condition": "Solo si se abre sin forzar (llave, rito o ingenio)"})
                tabla = {
                    "id": f"loot_{tipo}_{ent}_t{tier}", "name": f"{tipo.title()} de {ent} (tier {tier})",
                    "type": "loot_table", "tierRange": [tier, min(5, tier + 1)],
                    "drops": drops, "gold": {"min": ORO[tier][0], "max": ORO[tier][1] * 2},
                    "clue": f"clue_{ent}" if tipo == "alijo" else None,
                    "flavorText": flavor,
                }
                escritos += escribir(OUT / f"{tabla['id']}.json", tabla)

    print(f"Tablas de loot escritas: {escritos}")


if __name__ == "__main__":
    main()
