#!/usr/bin/env python3
"""INT-01 — Refà tots els interiors del kit, en ordre:

  1. tools/iso/sols_interiors.py        els terres (rajoles int_*)
  2. tools/iso/exportar_mobles.py       parets, mobles i peces grans (fulles, catàleg, rajoles int_peca_*)
  3. tools/pixellab/importar_tiles.py   les rajoles noves a terreno_mundo.png
  4. tools/pixellab/importar_monumentos.py  les peces grans, a tires (com els monuments)
  5. tools/gen_interiores_kit.py        les sales de cada edifici (nivells, mapes, portes, escales)

    python3 tools/iso/fer_interiors.py              # tot
    python3 tools/iso/fer_interiors.py taberna      # el pas 5 només per a un edifici
"""
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PASSOS = [["tools/iso/sols_interiors.py"], ["tools/iso/exportar_mobles.py"],
          ["tools/pixellab/importar_tiles.py", "assets/textures/pixellab/tiles.json"],
          ["tools/pixellab/importar_monumentos.py"], ["tools/gen_interiores_kit.py"]]


def main(edificis):
    for pas in PASSOS:
        args = pas + (edificis if pas[0] == "tools/gen_interiores_kit.py" else [])
        print(f"== {' '.join(args)}", flush=True)
        r = subprocess.run([sys.executable, *args], cwd=ROOT, capture_output=True, text=True)
        sortida = (r.stdout + r.stderr).strip().splitlines()
        print("\n".join(sortida[-6:] if r.returncode == 0 else sortida[-30:]))
        if r.returncode != 0:
            raise SystemExit(f"ha fallat: {' '.join(args)}")


if __name__ == "__main__":
    main(sys.argv[1:])
