#!/usr/bin/env python3
"""Une los barrios de Boundington entre si.

    python3 tools/unir_barrios.py

EL PROBLEMA

Boundington son cuatro mapas -- centro, este, oeste y surysal -- y no
estaban conectados. ciudad_centro tenia 21 puertas y las 21 iban a
interiores: ni una llevaba a otro barrio. oeste y surysal se hablaban
entre ellos y con nadie mas, y este estaba solo del todo.

Es decir: la ciudad principal del juego eran tres trozos incomunicados
dentro de la propia ciudad. Y no lo cazaba ningun validador porque
validar_enlaces comprueba que cada puerta lleve a sitio pisable y tenga
vuelta -- las de los interiores la tienen -- pero no que el conjunto sea
UN grafo. Alcanzable no es lo mismo que conectado.

COMO SE UNEN

Por el BORDE que les toca segun su nombre: centro-este por el este de
centro y el oeste de este, centro-oeste al reves, oeste-surysal por el
sur. La puerta se pone en una celda transitable y libre del borde
correspondiente, y se crea en los dos sentidos.

IDEMPOTENTE.
"""
import json
import os
import re
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
NIVELES = os.path.join(RAIZ, "assets", "levels")

# (a, borde_de_a, b) -- b se entra por el borde opuesto.
ENLACES = [
    ("ciudad_centro", "E", "ciudad_este"),
    ("ciudad_centro", "O", "ciudad_oeste"),
    ("ciudad_oeste",  "S", "ciudad_surysal"),
]
OPUESTO = {"E": "O", "O": "E", "N": "S", "S": "N"}
NOMBRE_BORDE = {"E": "este", "O": "oeste", "N": "norte", "S": "sur"}


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


def celda_de_borde(nivel, borde, ocupadas):
    """Celda transitable y libre lo mas pegada posible a ese borde.

    Se barre hacia dentro: si la primera columna es muralla (lo normal en
    una ciudad amurallada) se prueba la siguiente, hasta encontrar suelo.
    Poner la salida sobre la muralla la haria inalcanzable.
    """
    W, H, g, col = rejilla(nivel)
    libre = lambda x, y: (0 <= x < W and 0 <= y < H and g[y * W + x] not in col
                          and (x, y) not in ocupadas)
    # Orden de barrido: desde el borde hacia el centro, y del medio hacia
    # los extremos en el otro eje (una salida en una esquina es rara).
    if borde in ("E", "O"):
        columnas = range(W - 1, -1, -1) if borde == "E" else range(W)
        filas = sorted(range(H), key=lambda y: abs(y - H // 2))
        for x in columnas:
            for y in filas:
                if libre(x, y):
                    return (x, y)
    else:
        filas = range(H - 1, -1, -1) if borde == "S" else range(H)
        columnas = sorted(range(W), key=lambda x: abs(x - W // 2))
        for y in filas:
            for x in columnas:
                if libre(x, y):
                    return (x, y)
    return None


def main():
    creadas = 0
    for a, borde, b in ENLACES:
        pa = os.path.join(NIVELES, a + ".json")
        pb = os.path.join(NIVELES, b + ".json")
        if not (os.path.exists(pa) and os.path.exists(pb)):
            print(f"  (falta {a} o {b}, se salta)")
            continue
        na = json.load(open(pa, encoding="utf-8"))
        nb = json.load(open(pb, encoding="utf-8"))

        id_ida = f"paso_{a.replace('ciudad_', '')}_a_{b.replace('ciudad_', '')}"
        id_vuelta = f"paso_{b.replace('ciudad_', '')}_a_{a.replace('ciudad_', '')}"
        na["objects"] = [o for o in na["objects"] if o["objectId"] != id_ida]
        nb["objects"] = [o for o in nb["objects"] if o["objectId"] != id_vuelta]

        oa = {(o["position"]["x"], o["position"]["y"]) for o in na["objects"]}
        ob = {(o["position"]["x"], o["position"]["y"]) for o in nb["objects"]}
        ca = celda_de_borde(na, borde, oa)
        cb = celda_de_borde(nb, OPUESTO[borde], ob)
        if ca is None or cb is None:
            sys.exit(f"ABORTA: sin celda libre en el borde para {a} <-> {b}")

        na["objects"].append({"objectId": id_ida, "position": {"x": ca[0], "y": ca[1]},
                              "targetLevel": f"assets/levels/{b}.json",
                              "targetPosition": {"x": cb[0], "y": cb[1]}})
        nb["objects"].append({"objectId": id_vuelta, "position": {"x": cb[0], "y": cb[1]},
                              "targetLevel": f"assets/levels/{a}.json",
                              "targetPosition": {"x": ca[0], "y": ca[1]}})
        json.dump(na, open(pa, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
        json.dump(nb, open(pb, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
        print(f"  {a} {ca} <-> {b} {cb}   (por el {NOMBRE_BORDE[borde]})")
        creadas += 2

    # Fichas de los pasos, para que no sean objetos invisibles.
    ruta_cat = os.path.join(RAIZ, "assets", "objects", "boundington_pasos.json")
    fichas = []
    for a, borde, b in ENLACES:
        for origen, destino, sentido in ((a, b, borde), (b, a, OPUESTO[borde])):
            fichas.append({
                "id": f"paso_{origen.replace('ciudad_', '')}_a_{destino.replace('ciudad_', '')}",
                "name": "Hacia " + destino.replace("ciudad_", "").capitalize(),
                "category": "prop",
                "spriteId": -1,
                "blocksMovement": False,
                "interactable": True,
                "dialogue": [f"La calle sigue hacia el {NOMBRE_BORDE[sentido]}."],
            })
    json.dump({"_fuente": "Pasos entre los barrios de Boundington. tools/unir_barrios.py",
               "objects": fichas},
              open(ruta_cat, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
    print(f"  {creadas} pasos creados, con sus fichas en assets/objects/boundington_pasos.json")
    return 0


if __name__ == "__main__":
    sys.exit(main())
