#!/usr/bin/env python3
"""Oriol (23/10): las pestañas Razas y Clases de la Forja de Onegai. Pasa lo de tools/ajustes_fichas.json
(«razas» y «clases»: {id: {campo: valor}}) a los catálogos del motor, assets/catalogs/races.json y
classes.json: solo los campos que trae, sin tocar lo demás ni el orden de las claves.

    python3 tools/aplicar_forja_fichas.py              # escribe
    python3 tools/aplicar_forja_fichas.py --simular    # solo cuenta

El aspecto (razas_visual) y la pieza que se ve de cada objeto (objetos_visual) no van aquí: los leen los
generadores (forja_ropa.aplicar_razas y actores_catalogo) al rehacer las hojas con gen_personajes.py y
gen_pnj.py. Las clases suben después a las cartas con dndWeebCC-master/scripts/importar_clases_motor.py.
"""
import argparse
import json
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAT = HERE.parent / "assets/catalogs"
AJUSTES = HERE / "ajustes_fichas.json"
# lo que la Forja puede cambiar (el resto de la ficha, p. ej. las cartas, lo lleva el creador de cartas)
CAMPOS = {
    "races.json": ("name", "description", "flavorText", "rarity", "tier", "statBonuses", "affinities",
                   "compatibleDeities", "passiveTrait", "activeTrait", "limitations", "narrativeTags", "unlocks"),
    "classes.json": ("name", "description", "flavorText", "role", "rarity", "tier", "playable", "baseHealth",
                     "healthScaling", "primaryStat", "secondaryStat", "primaryResource", "secondaryResource",
                     "maxArmorWeight", "maxWeaponWeight", "allowedEquipmentTags", "restrictedTags",
                     "startingEquipment"),
}


def aplica(nombre, cambios, simular):
    ruta = CAT / nombre
    texto = ruta.read_text(encoding="utf-8")
    datos = json.loads(texto)
    por_id = {e["id"]: e for e in datos["entries"]}
    tocados, avisos = 0, []
    for ident, c in cambios.items():
        e = por_id.get(ident)
        if e is None:
            avisos.append(f"{nombre}: no hay «{ident}»")
            continue
        for k, v in c.items():
            if k not in CAMPOS[nombre]:
                avisos.append(f"{nombre}: {ident}.{k} no se cambia desde la Forja")
                continue
            if e.get(k) != v:
                e[k] = v
                tocados += 1
    if tocados and not simular:
        m = re.match(r"\{\n( +)\S", texto)
        ind = len(m.group(1)) if m else None
        ruta.write_text(json.dumps(datos, ensure_ascii=False, indent=ind) + "\n", encoding="utf-8")
    return tocados, avisos


def main(argv):
    a = argparse.ArgumentParser()
    a.add_argument("--simular", action="store_true")
    a = a.parse_args(argv)
    if not AJUSTES.exists():
        print("No hay tools/ajustes_fichas.json: nada que aplicar.")
        return 0
    aj = json.loads(AJUSTES.read_text(encoding="utf-8"))
    total = 0
    for nombre, clave in (("races.json", "razas"), ("classes.json", "clases")):
        n, avisos = aplica(nombre, aj.get(clave, {}), a.simular)
        total += n
        print(f"{nombre}: {n} campos cambiados" + (" (simulación)" if a.simular else ""))
        for t in avisos:
            print("  aviso:", t)
    if total and not a.simular:
        print("Después: python3 tools/validar_todo.py; y las clases a las cartas con "
              "dndWeebCC-master/scripts/importar_clases_motor.py")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
