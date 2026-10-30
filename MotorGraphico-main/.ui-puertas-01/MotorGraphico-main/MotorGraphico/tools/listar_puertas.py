#!/usr/bin/env python3
"""AV16-06 — Lista cómo se llama, para el jugador, CADA puerta de CADA nivel.

Una puerta es todo objeto de un nivel con `targetLevel`. Para cada una dice el aviso que
verá el jugador («Entrar en La Boutique de Madame», «Salir a la calle»), el rótulo
«Hacia …» si es una salida, y si hay algo que corregir. Replica las reglas de
Game::Lugares::textoDePuerta (src/Game/Lugares.cpp) y lee los mismos datos
(assets/catalogs/lugares.json: «lugares» y «calles»).

    python3 tools/listar_puertas.py                 # tabla de todas las puertas
    python3 tools/listar_puertas.py ciudad_centro   # solo ese nivel
    python3 tools/listar_puertas.py --comprobar     # sale con 1 si alguna puerta no tiene nombre digno
"""
import argparse
import glob
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def cargar(ruta):
    with open(os.path.join(ROOT, ruta), encoding="utf-8") as f:
        return json.load(f)


def base(nivel):
    return os.path.basename(nivel or "")


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("nivel", nargs="?", help="solo este nivel (nombre sin carpeta ni .json)")
    ap.add_argument("--comprobar", action="store_true", help="falla si alguna puerta no tiene nombre digno")
    a = ap.parse_args(argv)

    cat = cargar("assets/catalogs/lugares.json")
    lugares = {base(x["nivel"]): x for x in cat.get("lugares", [])}
    calles = {base(x["nivel"]): x for x in cat.get("calles", [])}
    nombres = {}
    for f in glob.glob(os.path.join(ROOT, "assets/objects/*.json")):
        try:
            for o in json.load(open(f, encoding="utf-8")).get("objects", []):
                nombres.setdefault(o.get("id"), o.get("name", ""))
        except (ValueError, OSError):
            pass

    filas, problemas = [], []
    for f in sorted(glob.glob(os.path.join(ROOT, "assets/levels/*.json"))):
        nivel = os.path.basename(f)[:-5]
        if a.nivel and nivel != a.nivel:
            continue
        try:
            objetos = json.load(open(f, encoding="utf-8")).get("objects", [])
        except (ValueError, OSError):
            continue
        for o in objetos:
            if not isinstance(o, dict) or not o.get("targetLevel"):
                continue
            oid, destino = o.get("objectId", ""), base(o["targetLevel"])
            q = o.get("position", {})
            pos = f"({q.get('x', '?')},{q.get('y', '?')})"
            if oid.startswith("salida_"):
                c = calles.get(destino)
                aviso = "Salir a " + (c["salir"] if c and c.get("salir") else "la calle")
                cartel = "Hacia " + (c["nombre"] if c else lugares[destino]["nombre"] if destino in lugares else "")
                if not c and destino not in lugares:
                    problemas.append(f"{nivel}: la salida {oid} lleva a {destino}, que no tiene nombre en lugares.json")
                    cartel = "(sin nombre)"
            else:
                l = lugares.get(destino)
                nombre = l["nombre"] if l else nombres.get(oid) or oid
                aviso = "Entrar en " + nombre
                if not l:  # mismas reglas que Game::Lugares::textoDePuerta
                    verbo = next((v for v in ("Volver", "Subir", "Bajar", "Ir", "Salir") if nombre.startswith(v + " ")), None)
                    if verbo:
                        aviso = nombre
                    elif nombre.startswith("Hacia "):
                        aviso = "Ir h" + nombre[1:]
                cartel = (l["nombre"] + " / " + l["funcion"]) if l else "(sin cartel)"
                if not nombre or nombre.lower() == "salida":
                    problemas.append(f"{nivel}: la puerta {oid} diría «{aviso}»")
            filas.append((nivel, oid, pos, aviso, cartel))

    ancho = [max(len(str(r[i])) for r in filas) for i in range(5)] if filas else [0] * 5
    for r in filas:
        print("  ".join(str(r[i]).ljust(ancho[i]) for i in range(5)).rstrip())
    print(f"\n{len(filas)} puertas.", f"{len(problemas)} por corregir." if problemas else "Todas tienen nombre.")
    for p in problemas:
        print("  ! " + p)
    return 1 if (a.comprobar and problemas) else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
