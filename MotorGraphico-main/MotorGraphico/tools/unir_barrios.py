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

# (a, borde_de_a, b, huecos) -- b se entra por el borde opuesto.
# `huecos` es None (el paso se busca como siempre: la celda libre del borde mas
# cercana al centro) o una lista de (nombre, fila0, fila1, fila_del_paso).
#
# AV16-09b (Excel de Oriol, 15/10 20:38): la muralla se abre donde llega una calle, y
# cada hueco lleva su paso en los dos sentidos. El id del paso se compone con el nombre
# del hueco (`paso_centro_a_oeste_casco`), salvo la banda "rio", que conserva los ids de
# siempre (`paso_centro_a_oeste` / `paso_oeste_a_centro`) para no romper lo que ya los
# cita. Las bandas tienen que coincidir con HUECOS_MURALLA de tools/gen_ciudad.py, que
# es quien pinta el hueco en el TMX: aqui se comprueba que este abierto y, si no, se
# aborta con el motivo (un paso sobre la muralla seria una puerta inalcanzable).
ENLACES = [
    ("ciudad_centro", "E", "ciudad_este", [
        ("norte", 12, 14, 13), ("comercial", 18, 20, 19), ("muelle", 26, 28, 27),
        ("rio", 32, 34, 32), ("casco", 48, 50, 49), ("sur", 60, 62, 61)]),
    ("ciudad_centro", "O", "ciudad_oeste", [
        ("norte", 12, 14, 13), ("comercial", 18, 20, 19), ("muelle", 25, 28, 27),
        ("rio", 32, 34, 32), ("casco", 48, 50, 49), ("sur", 60, 62, 61)]),
    ("ciudad_oeste",  "S", "ciudad_surysal", None),
]
OPUESTO = {"E": "O", "O": "E", "N": "S", "S": "N"}
NOMBRE_BORDE = {"E": "este", "O": "oeste", "N": "norte", "S": "sur"}


def id_paso(origen, destino, nombre=None):
    """Id del paso; la banda 'rio' (o un enlace sin huecos) no lleva sufijo."""
    sufijo = "" if nombre in (None, "rio") else "_" + nombre
    return f"paso_{origen.replace('ciudad_', '')}_a_{destino.replace('ciudad_', '')}{sufijo}"


def huecos_de(a, borde, b):
    """Los (id_ida, id_vuelta, nombre) de un enlace: uno por hueco, o uno solo."""
    pares = next(h for x, br, y, h in ENLACES if (x, br, y) == (a, borde, b))
    if pares is None:
        return [(id_paso(a, b), id_paso(b, a), None)]
    return [(id_paso(a, b, n), id_paso(b, a, n), n) for (n, _, _, _) in pares]


def celda_de_fila(nivel, borde, y, ocupadas):
    """La celda del borde E/O en la fila `y`, si esta abierta y libre. None si no."""
    W, H, g, col = rejilla(nivel)
    x = W - 1 if borde == "E" else 0
    if not (0 <= y < H) or g[y * W + x] in col or (x, y) in ocupadas:
        return None
    return (x, y)


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
    for a, borde, b, bandas in ENLACES:
        pa = os.path.join(NIVELES, a + ".json")
        pb = os.path.join(NIVELES, b + ".json")
        if not (os.path.exists(pa) and os.path.exists(pb)):
            print(f"  (falta {a} o {b}, se salta)")
            continue
        na = json.load(open(pa, encoding="utf-8"))
        nb = json.load(open(pb, encoding="utf-8"))

        huecos = huecos_de(a, borde, b)
        na["objects"] = [o for o in na["objects"] if o["objectId"] not in [i for i, _, _ in huecos]]
        nb["objects"] = [o for o in nb["objects"] if o["objectId"] not in [v for _, v, _ in huecos]]
        fila = {n: f for (n, _, _, f) in (bandas or [])}

        for id_ida, id_vuelta, nombre in huecos:
            oa = {(o["position"]["x"], o["position"]["y"]) for o in na["objects"]}
            ob = {(o["position"]["x"], o["position"]["y"]) for o in nb["objects"]}
            if nombre is None:
                ca = celda_de_borde(na, borde, oa)
                cb = celda_de_borde(nb, OPUESTO[borde], ob)
            else:
                ca = celda_de_fila(na, borde, fila[nombre], oa)
                cb = celda_de_fila(nb, OPUESTO[borde], fila[nombre], ob)
                if ca is None or cb is None:
                    sys.exit(f"ABORTA: el hueco '{nombre}' (fila {fila[nombre]}) no esta abierto o esta "
                             f"ocupado en {a if ca is None else b}. Lo pinta tools/gen_ciudad.py "
                             f"(HUECOS_MURALLA): ejecutalo antes, o un paso caeria sobre la muralla "
                             f"y seria una puerta inalcanzable.")
            if ca is None or cb is None:
                sys.exit(f"ABORTA: sin celda libre en el borde para {a} <-> {b}")

            na["objects"].append({"objectId": id_ida, "position": {"x": ca[0], "y": ca[1]},
                                  "targetLevel": f"assets/levels/{b}.json",
                                  "targetPosition": {"x": cb[0], "y": cb[1]}})
            nb["objects"].append({"objectId": id_vuelta, "position": {"x": cb[0], "y": cb[1]},
                                  "targetLevel": f"assets/levels/{a}.json",
                                  "targetPosition": {"x": ca[0], "y": ca[1]}})
            print(f"  {a} {ca} <-> {b} {cb}   (por el {NOMBRE_BORDE[borde]}"
                  + (f", hueco {nombre}" if nombre else "") + ")")
            creadas += 2
        json.dump(na, open(pa, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
        json.dump(nb, open(pb, "w", encoding="utf-8"), ensure_ascii=False, indent=1)

    # Fichas de los pasos, para que no sean objetos invisibles.
    ruta_cat = os.path.join(RAIZ, "assets", "objects", "boundington_pasos.json")
    fichas = []
    for a, borde, b, _ in ENLACES:
        for _, _, nombre in huecos_de(a, borde, b):
            for origen, destino, sentido in ((a, b, borde), (b, a, OPUESTO[borde])):
                fichas.append({
                    "id": id_paso(origen, destino, nombre),
                    "name": "Hacia " + destino.replace("ciudad_", "").capitalize()
                            + (f" ({nombre})" if nombre not in (None, "rio") else ""),
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
