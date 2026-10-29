#!/usr/bin/env python3
"""PART-02d — Que particulas echa cada carta al jugarse en el combate, para el motor.

La regla es la de tools/particulas.py (de_cada_carta: lo que nombra su texto de entre las de su clase, la
variante de su clase para su elemento, la de su clase, la de su elemento; planificacion/
CANON_PARTICULAS.md §3). Este script la escribe en assets/textures/particulas_cartas.json, que lee
Game::ParticulasDeCartas: "cartas" (id de skills.json -> familia) y "colores" (el color de la luz de cada
familia, para las que no traen los suyos: el agua, la luz, la sombra, las notas...). Una carta que no es
de ninguna clase (las de los objetos, las de los monstruos) no va: el motor le pone la de su elemento.

    python3 tools/particulas_cartas.py          # escribe
    python3 tools/particulas_cartas.py --ver    # solo cuenta
"""
import argparse
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import elementos  # noqa: E402
import particulas  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
CAT = ROOT / "assets/catalogs"
SALIDA = ROOT / "assets/textures/particulas_cartas.json"

# El color de las familias de tema que no traen colores ni son de un elemento.
COLOR_DE_TEMA = {"chispas": "ffd28c", "notas": "ffe6a0", "ondas": "c8e6ff"}


def color_de(familia):
    """El color de la luz de una familia: el primero de los suyos, el de su elemento o el de su tema."""
    f = particulas.FAMILIAS[familia]
    if f.get("colores"):
        return f["colores"][0]
    if f.get("de"):
        return elementos.ELEMENTOS[f["de"]][1]
    return COLOR_DE_TEMA.get(familia, "fff0d2")


def datos():
    clases = json.loads((CAT / "classes.json").read_text(encoding="utf-8"))["entries"]
    cartas = json.loads((CAT / "skills.json").read_text(encoding="utf-8"))["entries"]
    por_carta = particulas.de_cada_carta(clases, cartas)
    return {
        "_nota": ("PART-02d: la familia de particulas que echa cada carta al jugarse en el combate (la regla de "
                  "tools/particulas.py, planificacion/CANON_PARTICULAS.md §3) y el color de la luz de cada "
                  "familia. Generado por tools/particulas_cartas.py: no editar a mano. Las cartas que no son de "
                  "ninguna clase no van: el motor les pone la de su elemento (Game::ParticulasDeCartas)."),
        "generador": "tools/particulas_cartas.py",
        "cartas": dict(sorted(por_carta.items())),
        "colores": {f: color_de(f) for f in sorted(particulas.FAMILIAS)},
    }


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--ver", action="store_true", help="solo cuenta")
    args = ap.parse_args()
    d = datos()
    print(f"{len(d['cartas'])} cartas con su familia; {len(d['colores'])} familias con su color")
    if not args.ver:
        SALIDA.write_text(json.dumps(d, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
