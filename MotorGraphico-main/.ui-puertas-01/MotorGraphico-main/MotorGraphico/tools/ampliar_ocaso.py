#!/usr/bin/env python3
"""Oriol (18/10, Excel ciudad6): el Ocaso tambien es el barrio Oeste (la Base Militar, Pico Dragon y la
Barriada): pego ciudad_oeste a la izquierda del Casco en llamas. Esto hace lo mismo en el juego, una vez:

  ciudad_centro_ocaso pasa de 64x64 a 96x64: columnas 0..31 = el terreno de ciudad_oeste (su TMX),
  32..95 = el Ocaso que habia. Los objetos del Ocaso se corren 32 a la derecha (inicio, rondas, el paso al
  sur y la huida incluidos); del Oeste entra su decorado, sin PNJ, botin, pasos de barrio ni puertas de
  interiores (su vuelta apunta a la ciudad de dia).
  La muralla entre los dos barrios (columna 31 del Oeste / 0 del Casco) se abre donde estaban los pasos.

    python3 tools/ampliar_ocaso.py          # solo si el Ocaso aun mide 64 de ancho (si no, no hace nada)
"""
import json
import os
import re
import sys

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OESTE = 32

def tmx(ruta):
    t = open(ruta, encoding="utf-8").read()
    W = int(re.search(r'<map[^>]*?\swidth="(\d+)"', t).group(1))
    H = int(re.search(r'<map[^>]*?\sheight="(\d+)"', t).group(1))
    m = re.search(r'(<data encoding="csv">)(.*?)(</data>)', t, re.S)
    g = [v.strip() for v in m.group(2).replace("\n", "").split(",") if v.strip()]
    col = {str(int(x) + 1) for x in re.findall(r'<tile id="(\d+)">\s*<properties>\s*<property name="collision"', t)}
    return t, W, H, g, m, col


def main():
    p_oc = os.path.join(RAIZ, "assets/levels/ciudad_centro_ocaso.json")
    texto = open(p_oc, encoding="utf-8").read()
    oc = json.loads(texto)
    ind = len(re.match(r"\{\n( +)\S", texto).group(1))
    t_oc, W, H, g_oc, m_oc, col = tmx(os.path.join(RAIZ, oc["map"]))
    if W != 64:
        print(f"ciudad_centro_ocaso ya mide {W} de ancho: nada que hacer")
        return 0
    oe = json.load(open(os.path.join(RAIZ, "assets/levels/ciudad_oeste.json"), encoding="utf-8"))
    t_oe, Wo, Ho, g_oe, _, col_oe = tmx(os.path.join(RAIZ, oe["map"]))
    assert (Wo, Ho) == (OESTE, 64), (Wo, Ho)
    # el terreno: oeste | ocaso
    filas = [g_oe[y * Wo:(y + 1) * Wo] + g_oc[y * W:(y + 1) * W] for y in range(H)]
    # la costura: donde el Oeste tenia pasos al Casco, las dos columnas de muralla se abren con el suelo del paso
    pasos = [o for o in oe["objects"] if o["objectId"].startswith("paso_oeste_a_centro")]
    for o in pasos:
        x, y = o["position"]["x"], o["position"]["y"]
        suelo = filas[y][x]
        for xx in (OESTE, OESTE + 1):
            if filas[y][xx] in col:
                filas[y][xx] = suelo
        for xx in (x - 1, x):
            if filas[y][xx] in col_oe:
                filas[y][xx] = suelo
    csv = ",\n".join(",".join(f) for f in filas)
    nuevo = t_oc[:m_oc.start(2)] + "\n" + csv + "\n" + t_oc[m_oc.end(2):]
    nuevo = re.sub(r'(<map[^>]*?\swidth=")64(")', r"\g<1>96\2", nuevo, count=1)
    nuevo = re.sub(r'(<layer[^>]*?\swidth=")64(")', r"\g<1>96\2", nuevo, count=1)
    nuevo = nuevo.replace("Boundington en el ocaso", "Boundington en el ocaso (Oeste y Casco)", 1)
    open(os.path.join(RAIZ, oc["map"]), "w", encoding="utf-8").write(nuevo)
    # los objetos del Ocaso, 32 a la derecha
    for o in oc["objects"]:
        o["position"]["x"] += OESTE
        for k in ("patrolMin", "patrolMax"):
            if k in o:
                o[k]["x"] += OESTE
    oc["playerStart"]["x"] += OESTE
    # del Oeste: decorado y las dos puertas; ni PNJ, ni mascotas, ni botin, ni pasos
    # tampoco las puertas de interiores: la vuelta de un interior apunta a la ciudad de dia, no al Ocaso (si una
    # noche hace falta entrar en el cuartel, va una copia interior_cuartel_ocaso con su vuelta)
    fuera = ("npc_", "guardia", "sargento", "anciana", "nino", "nina", "xila", "parroquiano", "mascota_", "botin_",
             "paso_", "puente_", "puerta_", "cartel_coliseo")
    for o in oe["objects"]:
        if o["objectId"].startswith(fuera):
            continue
        oc["objects"].append(json.loads(json.dumps(o)))
    oc["name"] = "Boundington en el ocaso (Oeste y Casco)"
    open(p_oc, "w", encoding="utf-8").write(json.dumps(oc, indent=ind, ensure_ascii=False) + "\n")
    # lo que apunta al Ocaso: las afueras llegan a (64,61)
    p_af = os.path.join(RAIZ, "assets/levels/afueras_boundington_ocaso.json")
    if os.path.exists(p_af):
        texto = open(p_af, encoding="utf-8").read()
        af = json.loads(texto)
        for o in af["objects"]:
            if o.get("targetLevel", "").endswith("ciudad_centro_ocaso.json"):
                o["targetPosition"]["x"] += OESTE
        open(p_af, "w", encoding="utf-8").write(json.dumps(af, indent=len(re.match(r"\{\n( +)\S", texto).group(1)), ensure_ascii=False) + "\n")
    print(f"ciudad_centro_ocaso: 96x64, {len(oc['objects'])} objetos; inicio {oc['playerStart']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
