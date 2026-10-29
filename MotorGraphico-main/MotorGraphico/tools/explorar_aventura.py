#!/usr/bin/env python3
"""explorar_aventura.py — recorre una aventura al azar y cuenta finales y atascos (plan E-060..E-066).

Simplificaciones declaradas (el simulador NO es el motor):
  - ignora `trigger` y `lugar`: el jugador puede estar en cualquier sitio;
  - `skillCheck` y `startBattle` se resuelven a cara o cruz (rng < 0.5 = éxito / victoria);
  - los demás efectos que no tocan flags ni variables se ignoran;
  - una partida que agota el tope de pasos cuenta como atasco.
"""
from __future__ import annotations

import json
import random
import sys
from pathlib import Path

MOTOR = Path(__file__).resolve().parent.parent
MAX_PASOS = 2000
AVENTURAS = ("prologo", "precuela_taberna", "primer_dia", "segundo_dia", "ocaso")


def carga(ruta: str) -> tuple[list[dict], str]:
    """(beats, doneFlag del último objetivo)."""
    with open(ruta, encoding="utf-8") as f:
        d = json.load(f)
    objetivos = d.get("objectives") or []
    done = objetivos[-1].get("doneFlag", "") if objetivos else ""
    return list(d.get("beats") or []), done


def _en_rango(valor: int, cond) -> bool:
    if isinstance(cond, dict):
        if "min" in cond and valor < cond["min"]:
            return False
        if "max" in cond and valor > cond["max"]:
            return False
        return True
    return valor == cond


def cumple(estado: dict, req: dict | None) -> bool:
    req = req or {}
    flags, vars_ = estado["flags"], estado["vars"]
    if any(f not in flags for f in req.get("allFlags") or []):
        return False
    if any(f in flags for f in req.get("notFlags") or []):
        return False
    anys = req.get("anyFlags")
    if anys and not any(f in flags for f in anys):
        return False
    for nombre, cond in (req.get("variables") or {}).items():
        if not _en_rango(vars_.get(nombre, 0), cond):
            return False
    return True


def disponibles(estado: dict, beats: list[dict]) -> list[dict]:
    return [b for b in beats if cumple(estado, b.get("requires"))]


def aplica(estado: dict, efectos: list[dict], rng: random.Random) -> None:
    flags, vars_ = estado["flags"], estado["vars"]
    for e in efectos or []:
        t = e.get("type")
        if t == "setFlag":
            flags.add(e["arg"])
        elif t == "clearFlag":
            flags.discard(e["arg"])
        elif t == "setVariable":
            vars_[e["arg"]] = e.get("value", 0)
        elif t == "addVariable":
            nuevo = vars_.get(e["arg"], 0) + e.get("value", 0)
            if "min" in e:
                nuevo = max(nuevo, e["min"])
            if "max" in e:
                nuevo = min(nuevo, e["max"])
            vars_[e["arg"]] = nuevo
        elif t == "skillCheck":
            sc = e.get("skillCheck") or {}
            clave = "flagSuccess" if rng.random() < 0.5 else "flagBotch"
            if sc.get(clave):
                flags.add(sc[clave])
        elif t == "startBattle":
            sb = e.get("startBattle") or {}
            clave = "flagVictory" if rng.random() < 0.5 else "flagDefeat"
            if sb.get(clave):
                flags.add(sb[clave])


def _partida(beats: list[dict], done: str, rng: random.Random, inicial: dict | None = None):
    estado = {"flags": set((inicial or {}).get("flags", ())), "vars": dict((inicial or {}).get("vars", {}))}
    disparados: set[str] = set()
    ultimo = "<inicio>"
    for _ in range(MAX_PASOS):
        if done and done in estado["flags"]:
            return "final", ultimo, disparados
        libres = [b for b in disponibles(estado, beats) if b.get("id") not in disparados]
        if not libres:
            return "atasco", ultimo, disparados
        b = rng.choice(libres)
        disparados.add(b.get("id"))
        ultimo = b.get("id", "?")
        aplica(estado, b.get("effects") or [], rng)
        ops = [o for o in b.get("opciones") or [] if cumple(estado, o.get("requires"))]
        if ops:
            aplica(estado, rng.choice(ops).get("effects") or [], rng)
    return "atasco", ultimo, disparados


def explora(ruta: str, partidas: int, semilla: int, inicial: dict | None = None) -> dict:
    beats, done = carga(ruta)
    rng = random.Random(semilla)
    finales = atascos = 0
    por_beat: dict[str, int] = {}
    vistos: set[str] = set()
    for _ in range(partidas):
        res, ultimo, disp = _partida(beats, done, rng, inicial)
        vistos |= disp
        if res == "final":
            finales += 1
        else:
            atascos += 1
            por_beat[ultimo] = por_beat.get(ultimo, 0) + 1
    ids = [b.get("id") for b in beats]
    return {"finales": finales, "atascos": atascos, "beats_atasco": por_beat,
            "no_visitados": [i for i in ids if i not in vistos]}


def main() -> int:
    hay_atascos = False
    print(f"{'aventura':<20}{'finales':>8}{'atascos':>8}{'no_vis':>8}  beats_atasco (top 3)")
    for nombre in AVENTURAS:
        ruta = MOTOR / "assets" / "adventures" / f"boundington_{nombre}.json"
        try:
            r = explora(str(ruta), 300, 1)
        except OSError as e:
            print(f"{nombre:<20} NO SE LEE: {e}")
            hay_atascos = True
            continue
        top = sorted(r["beats_atasco"].items(), key=lambda kv: -kv[1])[:3]
        print(f"{nombre:<20}{r['finales']:>8}{r['atascos']:>8}{len(r['no_visitados']):>8}  {top}")
        hay_atascos = hay_atascos or r["atascos"] > 0
    return 1 if hay_atascos else 0


if __name__ == "__main__":
    sys.exit(main())
