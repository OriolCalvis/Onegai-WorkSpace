#!/usr/bin/env python3
"""Las vueltas de los interiores aterrizan donde está la puerta. Cuando una puerta se mueve desde el
Excel (Oriol, 18/10) la salida del interior seguía apuntando a la casilla vieja; si esa casilla ya no es
pisable (o no toca la puerta), aquí se pone en la casilla de la puerta. Idempotente.

    python3 tools/cuadrar_vueltas.py [--seco]
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ensanchar_pasos as E

RAIZ = E.RAIZ
SECO = "--seco" in sys.argv


def main():
    man = json.load(open(os.path.join(RAIZ, "assets/proyectos/boundington.json"), encoding="utf-8"))
    niveles = [os.path.splitext(n)[0] for n in man["levels"]]
    mapas = {}
    def mapa(n):
        if n not in mapas:
            try:
                mapas[n] = E.Mapa(n)
            except Exception:
                mapas[n] = None
        return mapas[n]
    cambios = 0
    for n in niveles:
        m = mapa(n)
        if m is None:
            continue
        # por destino: todas las puertas de este nivel que llevan allí; la vuelta vale si toca alguna
        por_destino = {}
        for o in m.lvl["objects"]:
            if o.get("targetLevel") and o["objectId"].startswith(("puerta_", "paso_", "salida_", "sendero_", "boca_", "puente_")):
                por_destino.setdefault(os.path.splitext(os.path.basename(o["targetLevel"]))[0], []).append(o)
        for destino, puertas in por_destino.items():
            md = mapa(destino)
            if md is None:
                continue
            for s in md.lvl["objects"]:
                if not s.get("targetLevel", "").endswith(n + ".json"):
                    continue
                tp = s.get("targetPosition") or {}
                tx, ty = tp.get("x"), tp.get("y")
                if tx is not None and m.andable(tx, ty) and any(abs(tx - p["position"]["x"]) + abs(ty - p["position"]["y"]) <= 2 for p in puertas):
                    continue
                p = min(puertas, key=lambda q: (abs((tx or 0) - q["position"]["x"]) + abs((ty or 0) - q["position"]["y"])))
                px, py = p["position"]["x"], p["position"]["y"]
                print(f"  {destino}: {s['objectId']} -> {n} ({tx},{ty}) -> ({px},{py}) [la puerta {p['objectId']}]")
                s["targetPosition"] = {"x": px, "y": py}
                md.cambiado = True
                cambios += 1
    if not SECO:
        for m in mapas.values():
            if m is not None and m.cambiado:
                m.escribir()
    print(f"  {cambios} vueltas cuadradas" + (" (en seco)" if SECO else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
