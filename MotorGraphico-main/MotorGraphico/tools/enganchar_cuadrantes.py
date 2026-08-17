#!/usr/bin/env python3
"""Mete las aportaciones de los cuadrantes dentro del mundo.

    python3 tools/enganchar_cuadrantes.py

EL PROBLEMA

El experimento de las cuatro IAs produjo niveles con prefijo por autor:
`on_*` (oeste-norte) y `es_*` (este-sur). Estaban en assets/levels, se
cargaban bien, pasaban los validadores... y no se llegaba a ellos desde
ninguna parte. Contenido escrito, verificado y no jugable.

El prefijo que evitaba que dos autores se pisaran los ficheros era
tambien lo que los dejaba fuera: ningun validador miraba esos nombres.

LO QUE HACE

Cada cuadrante cuelga de la masa de tierra que le toca por geografia:

    on_*  ->  landmass_1 (Aegroum, Ascaria, Bastrea, Ecla, Ostad, Udrax)
    es_*  ->  landmass_2 (Ashye, Bosmurg, Choubar, Esmua, Gongorguma...)

Se crea una puerta en la masa y la vuelta en el nivel, igual que con
Boundington. Los sitios se eligen por orden alfabetico del id, para que
regenerar no los mueva.

IDEMPOTENTE.
"""
import json
import os
import re
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
NIVELES = os.path.join(RAIZ, "assets", "levels")

CUADRANTES = [("on_", "mundi_landmass_1.json", "oeste-norte"),
              ("es_", "mundi_landmass_2.json", "este-sur")]


def rejilla(nivel):
    p = os.path.join(RAIZ, nivel["map"])
    if not os.path.exists(p):
        return None
    t = re.sub(r"<!--.*?-->", "", open(p, encoding="utf-8").read(), flags=re.S)
    W = int(re.search(r'<map[^>]*?\swidth="(\d+)"', t).group(1))
    H = int(re.search(r'<map[^>]*?\sheight="(\d+)"', t).group(1))
    col = {int(m) + 1 for m in re.findall(
        r'<tile id="(\d+)">\s*<properties>\s*<property name="collision"[^/]*/>', t)}
    g = [int(v) for v in re.search(r'<data encoding="csv">(.*?)</data>', t, re.S)
         .group(1).replace("\n", "").split(",") if v.strip()]
    return W, H, g, col


def hueco(nivel, cerca, ocupadas):
    r = rejilla(nivel)
    if r is None:
        return None
    W, H, g, col = r
    for radio in range(0, 40):
        for dy in range(-radio, radio + 1):
            for dx in range(-radio, radio + 1):
                x, y = cerca[0] + dx, cerca[1] + dy
                if 0 <= x < W and 0 <= y < H and g[y * W + x] not in col \
                        and (x, y) not in ocupadas:
                    return (x, y)
    return None


def main():
    fichas = []
    total = 0
    for prefijo, masa_f, nombre in CUADRANTES:
        ruta_masa = os.path.join(NIVELES, masa_f)
        if not os.path.exists(ruta_masa):
            print(f"  (no existe {masa_f}, se salta {nombre})")
            continue
        masa = json.load(open(ruta_masa, encoding="utf-8"))
        candidatos = sorted(f for f in os.listdir(NIVELES)
                            if f.startswith(prefijo) and f.endswith(".json"))
        if not candidatos:
            continue

        ps = masa["playerStart"]
        for i, fichero in enumerate(candidatos):
            ruta = os.path.join(NIVELES, fichero)
            nivel = json.load(open(ruta, encoding="utf-8"))
            base = fichero[:-5]
            id_puerta = "acceso_" + base
            id_vuelta = "regreso_al_mundo"

            masa["objects"] = [o for o in masa["objects"] if o["objectId"] != id_puerta]
            ocupadas = {(o["position"]["x"], o["position"]["y"]) for o in masa["objects"]}
            # Repartidos alrededor del inicio de la masa, no amontonados.
            sitio = hueco(masa, (ps["x"] + (i + 1) * 3, ps["y"] + (i % 3) * 3), ocupadas)
            if sitio is None:
                print(f"    sin hueco en {masa_f} para {base}")
                continue

            nivel["objects"] = [o for o in nivel["objects"] if o["objectId"] != id_vuelta]
            ocup_n = {(o["position"]["x"], o["position"]["y"]) for o in nivel["objects"]}
            nps = nivel["playerStart"]
            llegada = hueco(nivel, (nps["x"], nps["y"]), ocup_n)
            if llegada is None:
                print(f"    sin hueco de vuelta en {base}")
                continue

            masa["objects"].append({"objectId": id_puerta,
                                    "position": {"x": sitio[0], "y": sitio[1]},
                                    "targetLevel": f"assets/levels/{fichero}",
                                    "targetPosition": {"x": llegada[0], "y": llegada[1]}})
            nivel["objects"].append({"objectId": id_vuelta,
                                     "position": {"x": llegada[0], "y": llegada[1]},
                                     "targetLevel": f"assets/levels/{masa_f}",
                                     "targetPosition": {"x": sitio[0], "y": sitio[1]}})
            json.dump(nivel, open(ruta, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
            fichas.append({"id": id_puerta,
                           "name": base.replace(prefijo, "").replace("_", " ").title(),
                           "category": "prop", "spriteId": -1,
                           "blocksMovement": False, "interactable": True,
                           "dialogue": [f"Camino hacia {base.replace(prefijo, '').replace('_', ' ')}."]})
            total += 1
            print(f"  {masa_f} {sitio} -> {base} {llegada}")

        json.dump(masa, open(ruta_masa, "w", encoding="utf-8"), ensure_ascii=False, indent=1)

    ruta_cat = os.path.join(RAIZ, "assets", "objects", "accesos_cuadrantes.json")
    json.dump({"_fuente": "Accesos a los niveles de los cuadrantes. tools/enganchar_cuadrantes.py",
               "objects": fichas},
              open(ruta_cat, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
    print(f"\n  {total} niveles de cuadrante enganchados al mundo")
    return 0


if __name__ == "__main__":
    sys.exit(main())
