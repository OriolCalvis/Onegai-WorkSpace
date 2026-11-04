#!/usr/bin/env python3
"""Hornea las animaciones de la Forja (actores_animaciones) en hojas: una por personaje y animación.

    python3 tools/gen_animaciones.py                       # las 45 razas vestidas y los conjuntos guardados
    python3 tools/gen_animaciones.py --raza race_human_marches --anim atacar

Sale en out/animaciones/<personaje>/<animación>.png (una fila por dirección: abajo, derecha, arriba,
izquierda; un frame de 64x64 por columna) y out/animaciones/animaciones.json (los milisegundos de cada
frame). Es para mirarlas y, cuando estén a gusto, pasarlas al motor.
"""
import argparse
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import actores_animaciones as anim  # noqa: E402
import actores_personajes as pers  # noqa: E402
import actores_razas as razas  # noqa: E402
import forja_ropa  # noqa: E402
from actores_tallas import construir_tallas  # noqa: E402


def personajes(solo_raza=None):
    out = {}
    for r in razas.RAZAS:
        if not solo_raza or r["id"] == solo_raza:
            out[r["id"]] = pers.spec_raza(r["id"])
    for nombre, c in forja_ropa.ajustes().get("conjuntos", {}).items():
        if c.get("raza") in razas.POR_ID and (not solo_raza or c["raza"] == solo_raza):
            spec = {"raza": c["raza"], "equipo": {k: v for k, v in (c.get("equipo") or {}).items() if v}}
            for k in ("pelo", "barba"):
                if k in c:
                    spec[k] = tuple(c[k]) if c[k] else None
            out["conjunto_" + forja_ropa_slug(nombre)] = spec
    return out


def forja_ropa_slug(t):
    import re
    import unicodedata
    t = unicodedata.normalize("NFD", t.lower())
    t = "".join(c for c in t if unicodedata.category(c) != "Mn")
    return re.sub(r"[^a-z0-9]+", "_", t).strip("_") or "conjunto"


def main(argv):
    a = argparse.ArgumentParser()
    a.add_argument("--raza")
    a.add_argument("--anim")
    a.add_argument("--salida", default=str(HERE.parent / "out" / "animaciones"))
    a = a.parse_args(argv)
    tallas = construir_tallas()
    anims = {k: v for k, v in anim.todas().items() if not a.anim or k == a.anim}
    salida = Path(a.salida)
    indice = {"_nota": "Lo escribe tools/gen_animaciones.py. Filas: " + ", ".join(anim.DIRS) + "; frames de 64x64.",
              "animaciones": {k: {"nombre": v["nombre"], "ms": {d: [fr.get("ms", 120) for fr in anim.frames_de(v, d)]
                                                                for d in anim.DIRS}} for k, v in anims.items()},
              "personajes": []}
    for pid, spec in personajes(a.raza).items():
        (salida / pid).mkdir(parents=True, exist_ok=True)
        for aid, an in anims.items():
            filas = anim.animacion(spec, tallas, an)
            ancho = max(len(filas[d]) for d in anim.DIRS) or 1
            hoja = np.zeros((64 * 4, 64 * ancho, 4), np.uint8)
            for i, d in enumerate(anim.DIRS):
                for j, (px, _ms) in enumerate(filas[d]):
                    hoja[i * 64:(i + 1) * 64, j * 64:(j + 1) * 64] = px
            Image.fromarray(hoja, "RGBA").save(salida / pid / f"{aid}.png")
        indice["personajes"].append(pid)
        print(f"{pid}: {len(anims)} animaciones")
    salida.mkdir(parents=True, exist_ok=True)
    (salida / "animaciones.json").write_text(json.dumps(indice, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
