"""BND-04 — requires.reputacion de las aventuras: {faccion: {min, max}} (-100..100).

La faccion tiene que estar en assets/rules/facciones.json (lo mismo que mira el
cargador de C++). Lo llama validar_todo.py con cada requires de un beat y de
sus opciones; devuelve los fallos como texto, uno por cosa.
"""
from __future__ import annotations

import json
from functools import lru_cache
from pathlib import Path

FACCIONES = Path(__file__).resolve().parent.parent / "assets" / "rules" / "facciones.json"


@lru_cache(maxsize=1)
def facciones_del_fichero() -> frozenset[str]:
    return frozenset(f["id"] for f in json.loads(FACCIONES.read_text(encoding="utf-8")).get("facciones", []) if f.get("id"))


def revisa(req: object, err, fichero: str, bid: str) -> None:
    """Da por error (err(fichero, beat, campo, texto)) cada fallo del requires."""
    for fallo in fallos(req, facciones_del_fichero()) if isinstance(req, dict) else []:
        err(fichero, bid, "requires.reputacion", fallo)


def fallos(req: dict, conocidas: set[str]) -> list[str]:
    rep = req.get("reputacion")
    if rep is None:
        return []
    if not isinstance(rep, dict) or not rep:
        return ["es {faccion: {min, max}}"]
    out = []
    for faccion, rango in rep.items():
        if faccion not in conocidas:
            out.append(f"'{faccion}' no es una facción de assets/rules/facciones.json")
            continue
        claves = set(rango) if isinstance(rango, dict) else set()
        if not claves or not claves <= {"min", "max"}:  # claves de más fuera (el C++ hace lo mismo)
            out.append(f"{faccion}: es {{min, max}}, alguno de los dos")
            continue
        lo, hi = rango.get("min", -100), rango.get("max", 100)
        if not all(isinstance(v, int) and not isinstance(v, bool) for v in (lo, hi)) or not -100 <= lo <= hi <= 100:
            out.append(f"{faccion}: min y max enteros de -100 a 100, con min <= max")
    return out
