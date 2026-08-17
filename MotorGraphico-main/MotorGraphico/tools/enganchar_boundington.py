#!/usr/bin/env python3
"""Mete Boundington -- y su campana entera -- dentro de Egaroth.

    python3 tools/enganchar_boundington.py

EL PROBLEMA

El mapamundi unia las ocho masas de tierra y sus 22 ciudades... y la
campana principal se quedaba fuera. 32 niveles -- ciudad_centro,
ciudad_este, ciudad_oeste, ciudad_surysal, la Plaza de las Razas y todos
los interiores -- sin una sola puerta que los conectara con el mundo.

Se veia perfectamente en el mapa: `mundi_landmass_1` tiene un marcador
llamado 'ciudad_mrnu1agq8t6u' cuyo nombre en el catalogo es "Boundington".
Un cartel con el nombre puesto, y detras nada.

Y no salto ningun validador porque todos miraban prefijos: los niveles de
la campana se llaman ciudad_centro, no ciudad_en_algo.

LO QUE HACE

Convierte ese marcador en una puerta de verdad hacia ciudad_centro, y
pone la vuelta en Boundington. Boundington esta en el sur de Aegroum
(landmass_1), que es exactamente lo que dice el prologo: "como en todo el
sur de Aegroum".

IDEMPOTENTE.
"""
import json
import os
import re
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
NIVELES = os.path.join(RAIZ, "assets", "levels")
MAPAS = os.path.join(RAIZ, "assets", "maps")

MASA = "mundi_landmass_1.json"
MARCADOR = "ciudad_mrnu1agq8t6u"      # "Boundington" en mundi_lugares.json
CIUDAD = "ciudad_centro.json"


def rejilla(nivel):
    p = os.path.join(RAIZ, nivel["map"])
    t = re.sub(r"<!--.*?-->", "", open(p, encoding="utf-8").read(), flags=re.S)
    W = int(re.search(r'<map[^>]*?\swidth="(\d+)"', t).group(1))
    H = int(re.search(r'<map[^>]*?\sheight="(\d+)"', t).group(1))
    col = {int(m) + 1 for m in re.findall(
        r'<tile id="(\d+)">\s*<properties>\s*<property name="collision"[^/]*/>', t)}
    g = [int(v) for v in re.search(r'<data encoding="csv">(.*?)</data>', t, re.S)
         .group(1).replace("\n", "").split(",") if v.strip()]
    return W, H, g, col


def hueco_libre(nivel, cerca, ocupadas):
    """Una celda transitable y sin objeto, lo mas cerca posible de 'cerca'."""
    W, H, g, col = rejilla(nivel)
    for radio in range(0, 20):
        for dy in range(-radio, radio + 1):
            for dx in range(-radio, radio + 1):
                x, y = cerca[0] + dx, cerca[1] + dy
                if not (0 <= x < W and 0 <= y < H):
                    continue
                if g[y * W + x] in col or (x, y) in ocupadas:
                    continue
                return (x, y)
    return None


def main():
    masa = json.load(open(os.path.join(NIVELES, MASA), encoding="utf-8"))
    ciudad = json.load(open(os.path.join(NIVELES, CIUDAD), encoding="utf-8"))

    marcador = next((o for o in masa["objects"] if o["objectId"] == MARCADOR), None)
    if marcador is None:
        sys.exit(f"no encuentro el marcador {MARCADOR} en {MASA}")

    # --- Sitio de llegada en Boundington ---
    ocupadas_c = {(o["position"]["x"], o["position"]["y"]) for o in ciudad["objects"]}
    ps = ciudad["playerStart"]
    llegada = hueco_libre(ciudad, (ps["x"], ps["y"]), ocupadas_c)
    if llegada is None:
        sys.exit("no hay hueco libre en ciudad_centro para el camino")

    # --- El marcador pasa a ser puerta ---
    marcador["targetLevel"] = "assets/levels/" + CIUDAD
    marcador["targetPosition"] = {"x": llegada[0], "y": llegada[1]}

    # --- Y la vuelta, en Boundington ---
    ciudad["objects"] = [o for o in ciudad["objects"] if o["objectId"] != "camino_a_egaroth"]
    ciudad["objects"].append({
        "objectId": "camino_a_egaroth",
        "position": {"x": llegada[0], "y": llegada[1]},
        "targetLevel": "assets/levels/" + MASA,
        "targetPosition": {"x": marcador["position"]["x"], "y": marcador["position"]["y"]},
    })

    json.dump(masa, open(os.path.join(NIVELES, MASA), "w", encoding="utf-8"),
              ensure_ascii=False, indent=1)
    json.dump(ciudad, open(os.path.join(NIVELES, CIUDAD), "w", encoding="utf-8"),
              ensure_ascii=False, indent=1)

    # --- La ficha de la puerta ---
    ruta_cat = os.path.join(RAIZ, "assets", "objects", "mapamundi_objetos.json")
    cat = json.load(open(ruta_cat, encoding="utf-8"))
    ids = {o["id"] for o in cat["objects"]}
    if "camino_a_egaroth" not in ids:
        cat["objects"].append({
            "id": "camino_a_egaroth",
            "name": "Camino del norte",
            "category": "prop",
            "spriteId": -1,
            "blocksMovement": False,
            "interactable": True,
            "dialogue": ["El camino sale de Boundington y se pierde Aegroum arriba."],
        })
        json.dump(cat, open(ruta_cat, "w", encoding="utf-8"), ensure_ascii=False, indent=1)

    print(f"  {MASA}: el marcador '{MARCADOR}' (Boundington) pasa a ser puerta")
    print(f"    {marcador['position']}  ->  ciudad_centro {llegada}")
    print(f"  ciudad_centro: 'camino_a_egaroth' devuelve al mapa de Aegroum")
    return 0


if __name__ == "__main__":
    sys.exit(main())
