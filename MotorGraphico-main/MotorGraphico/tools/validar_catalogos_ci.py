#!/usr/bin/env python3
"""D-10 — Validació de catàlegs per a CI.

Executa les fases 2–5 de validar_todo.py (referències creuades,
objectIds de projectes i beats d'aventures) SENSE executar els
validadors de connectivity/mundo de la fase 1, que tenen problemes
de contingut preexistents documentats al TABLERO.

El propòsit és: si una entrada d'un catàleg perd un camp mecànic
o una referència creuada es trenca, CI ho detecta.
"""

import subprocess
import sys
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
MOTOR = TOOLS.parent


def main() -> int:
    # Executar les fases 2–5 de validar_todo.py amb --skip-phase1
    r = subprocess.run(
        [sys.executable, str(TOOLS / "validar_todo.py"), "--skip-phase1"],
        cwd=MOTOR,
        capture_output=True,
        text=True,
        timeout=120,
    )
    print(r.stdout)
    if r.stderr:
        print(r.stderr, file=sys.stderr)
    return r.returncode


if __name__ == "__main__":
    sys.exit(main())
