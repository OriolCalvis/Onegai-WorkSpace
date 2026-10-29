#!/usr/bin/env python3
"""inventari_arbre.py — clasifica los cambios de `git status --short` (plan E-001, E-003)."""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

CATEGORIAS = ("codi", "dades", "docs", "config_local", "brossa")
EXT_CODI = (".cpp", ".h", ".hpp", ".cmake")


def _sin_comillas(ruta: str) -> str:
    ruta = ruta.strip()
    if len(ruta) >= 2 and ruta[0] == '"' and ruta[-1] == '"':
        ruta = ruta[1:-1]
        try:  # git escapa los bytes no ASCII como \303\241
            ruta = ruta.encode("latin-1", "backslashreplace").decode("unicode_escape").encode("latin-1").decode("utf-8")
        except (UnicodeError, ValueError):
            pass
        ruta = ruta.replace('\\"', '"')
    return ruta


def _categoria(ruta: str) -> str:
    if ruta.startswith((".papelera_sandbox/", ".claude_tmp/")):
        return "brossa"
    if ruta.startswith((".obsidian/", ".qwen/")):
        return "config_local"
    nombre = ruta.rstrip("/").rsplit("/", 1)[-1]
    if ruta.endswith(EXT_CODI) or nombre == "CMakeLists.txt":
        return "codi"
    if "assets/" in ruta:
        return "dades"
    return "docs"


def classifica(porcelain: str) -> dict[str, list[str]]:
    out: dict[str, list[str]] = {c: [] for c in CATEGORIAS}
    for linea in porcelain.splitlines():
        if not linea.strip():
            continue
        ruta = linea[3:] if len(linea) > 3 else linea.strip()
        if " -> " in ruta:  # renombrado: cuenta el destino
            ruta = ruta.split(" -> ", 1)[1]
        ruta = _sin_comillas(ruta)
        out[_categoria(ruta)].append(ruta)
    return out


def main() -> int:
    raiz = Path(__file__).resolve().parents[3]
    res = subprocess.run(["git", "status", "--short"], cwd=raiz, capture_output=True, text=True)
    datos = classifica(res.stdout)
    print(f"{'categoría':<14}{'nº':>5}  primeros 5 ficheros")
    for cat in CATEGORIAS:
        fich = datos[cat]
        print(f"{cat:<14}{len(fich):>5}  {', '.join(fich[:5])}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
