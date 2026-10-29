#!/usr/bin/env python3
"""PNJ-01 — Todos los PNJ se ven con las piezas del jugador.

Comprueba, sobre los assets (no sobre el generador):

  1. Cada objeto de categoria npc de assets/objects tiene su fila en una hoja
     pnj_*, resuelto como lo hace Game::RegistroSprites (alias, patrones y la
     ultima hoja manda): ninguno cae en un dibujo de PixelLab, en la ropa
     prestada de otro PNJ ni en el cuerpo base.
  2. Cada composicion de las hojas pnj_* la podria copiar un jugador: raza,
     piel y talla (solo cuerpos lisos), ojos, peinado, barba y ropa de diario
     de personajes.json; el equipo, de sus atlas. Las piezas "soloPnj" solo
     las llevan PNJ: la creacion no las ofrece y ningun objeto del catalogo de
     equipo las viste.
  3. Ninguna composicion se repite, y un PNJ colocado varias veces tiene
     tantas variantes como colocaciones (hasta actores_pnj.MAX_VARIANTES).

    python3 tools/validar_pnj.py
"""
import json
import sys
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))
ROOT = TOOLS.parent
TEX = ROOT / "assets/textures"


def registro():
    """(actores id -> (hoja, fila, variantes), alias, patrones) como RegistroSprites."""
    reg = json.loads((TEX / "sprites.json").read_text(encoding="utf-8"))
    actores = {}
    for h in reg["hojas"]:
        ruta = ROOT / "assets" / h["manifiesto"]
        if not ruta.exists():
            continue
        m = json.loads(ruta.read_text(encoding="utf-8"))
        for a in m.get("actores", []) if isinstance(m.get("actores"), list) else []:
            actores[h.get("prefijo", "") + a["id"]] = (h["id"], a["fila"], a.get("variantes", []), a)
    return actores, reg.get("alias", {}), reg.get("patrones", [])


def actor(oid, raza, actores, alias, patrones):
    candidatos = [alias.get(oid, oid)]
    for p in patrones:
        if ("sufijo" in p and oid.endswith(p["sufijo"])) or ("prefijo" in p and oid.startswith(p["prefijo"])):
            candidatos.append(p["id"])
    for c in candidatos:
        if c in actores:
            return c, actores[c]
    if raza and "race:" + raza in actores:
        return "race:" + raza, actores["race:" + raza]
    return None, None


def main():
    import actores_pnj as pnj
    errores = []
    man = json.loads((TEX / "personajes.json").read_text(encoding="utf-8"))
    solo = {p["id"] for g in man["equipo"].values() for p in g["piezas"] if p.get("soloPnj")}
    if not solo:
        errores.append("personajes.json no marca ninguna pieza soloPnj")
    catalogo = json.loads((TEX / "personajes/equipo_catalogo.json").read_text(encoding="utf-8"))
    for obj, pieza in catalogo.get("objetos", {}).items():
        if (pieza.get("pieza") if isinstance(pieza, dict) else pieza) in solo:
            errores.append(f"equipo_catalogo: el objeto {obj} viste una pieza solo de PNJ")
    defs = pnj.definiciones(ROOT)
    usos = pnj.colocaciones(ROOT)
    actores, alias, patrones = registro()
    vistas, n_filas = {}, 0
    for oid, (defn, cat) in sorted(defs.items()):
        cual, a = actor(oid, defn.get("raceId", ""), actores, alias, patrones)
        if a is None:
            errores.append(f"{cat}: {oid} no tiene hoja (se veria el cuerpo base)")
            continue
        hoja, _fila, variantes, datos = a
        if not hoja.startswith("pnj_") or cual != oid:
            errores.append(f"{cat}: {oid} se ve como «{cual}» de {hoja}, no con su propia fila de pnj_*")
            continue
        esperadas = 1 if pnj.PNJ.get(oid, {}).get("_unico") else max(1, min(pnj.MAX_VARIANTES, usos.get(oid, 0)))  # PROLOGO-02
        if 1 + len(variantes) != esperadas:
            errores.append(f"{oid}: {1 + len(variantes)} aspectos para {usos.get(oid, 0)} colocaciones")
        filas = [datos] + [actores[f"{oid}~{k + 2}"][3] for k in range(len(variantes)) if f"{oid}~{k + 2}" in actores]
        for fila in filas:
            spec = dict(fila["composicion"])
            for k in ("pelo", "barba"):
                if isinstance(spec.get(k), list):
                    spec[k] = tuple(spec[k])
            errores += pnj.comprobar(fila["id"], spec)
            clave = pnj.clave(spec)
            if clave in vistas:
                errores.append(f"{fila['id']} se ve igual que {vistas[clave]}")
            vistas[clave] = fila["id"]
            n_filas += 1
            usadas = sorted(p for p in spec.get("equipo", {}).values() if p in solo)
            if usadas != sorted(fila.get("soloPnj", [])):
                errores.append(f"{fila['id']}: soloPnj {fila.get('soloPnj')} no casa con {usadas}")
    if errores:
        print(f"validar_pnj: {len(errores)} problemas")
        for e in errores[:60]:
            print("  " + e)
        return 1
    print(f"validar_pnj: {len(defs)} PNJ, {n_filas} aspectos distintos montados con las piezas del jugador "
          f"({len(solo)} piezas solo de PNJ)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
