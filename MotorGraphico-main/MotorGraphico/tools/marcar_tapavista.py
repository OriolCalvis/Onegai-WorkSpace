#!/usr/bin/env python3
"""E-DISPARADORES-1 (D0026.8) — `tapaVista: true` en los catalogos de objetos que NO salen de un generador de kit.

Los de los kits (mobles_kit.json, decorados_kit.json) lo ponen sus generadores (tools/iso/exportar_*.py) con la
regla de tools/iso/tapavista.py. Estos son de mano: se marcan los ids de abajo (solo si bloquean el paso).
Idempotente:   python3 tools/marcar_tapavista.py
"""
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "iso"))
from tapavista import tapa_vista  # noqa: E402

OBJ = Path(__file__).resolve().parents[1] / "assets/objects"
# fichero -> (sangria del JSON, ids extra que tapan aunque su id no case con la regla)
CATALOGOS = {
    "pixellab_objetos.json": (1, set()),
    "boundington_npcs.json": (2, {"puerta_gurkazaal", "huida_carro", "huida_carromato", "huida_carromato_lona"}),
    "boundington_av19_objetos.json": (2, {"verja_dorada"}),
    "botin_boundington.json": (2, set()),
}
ARMARIO_BOTIN = re.compile(r"^botin_\w*armario")


def main():
    total = 0
    for nombre, (sangria, extra) in CATALOGOS.items():
        ruta = OBJ / nombre
        texto = ruta.read_text()
        doc = json.loads(texto)
        nuevos = []
        for o in doc["objects"]:
            i = o["id"]
            tapa = o.get("category") == "prop" and (tapa_vista(i, o.get("blocksMovement", False)) or
                                                    (o.get("blocksMovement") and (i in extra or ARMARIO_BOTIN.match(i))))
            if tapa and not o.get("tapaVista"):
                o2 = {}
                for k, v in o.items():
                    o2[k] = v
                    if k == "blocksMovement":
                        o2["tapaVista"] = True
                o = o2
                total += 1
            nuevos.append(o)
        doc["objects"] = nuevos
        salida = json.dumps(doc, indent=sangria, ensure_ascii=False) + ("" if sangria == 1 else "\n")
        if salida != texto:
            ruta.write_text(salida)
    print(f"tapaVista: {total} objetos marcados en catalogos de mano")


if __name__ == "__main__":
    main()
