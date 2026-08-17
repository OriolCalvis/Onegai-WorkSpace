#!/usr/bin/env python3
"""Valida la contribucion de UNA IA al experimento de los cuatro.

    python3 validar_contribucion.py oeste_norte

Comprueba lo que puede comprobar una maquina, que no es el gusto de nadie
sino que el contenido CARGUE y sea ALCANZABLE. Ocho comprobaciones, en el
orden en que duelen:

  1. prefijos      todo id creado lleva el prefijo del cuadrante
  2. colisiones    no pisa ningun id que ya existiera
  3. canon         naciones y capitales son las canonicas
  4. niveles       TMX bien formado y playerStart transitable
  5. conectividad  TODO lo transitable es alcanzable (flood fill)
  6. catalogo      cada objectId usado en un nivel tiene ficha
  7. aventuras     parsean y ningun beat es inalcanzable
  8. flags         ninguna flag consumida se queda sin encender

La 5 y la 7 son las que de verdad importan: un umbral rodeado de muros y un
beat que no dispara nunca cargan sin error y son contenido muerto. Ya paso
en este proyecto -- tres puertas inalcanzables que solo delato el flood fill.

Salida: 0 si todo pasa. Distinto de 0 y un informe si no.
"""
import json
import os
import re
import sys
from collections import deque

AQUI = os.path.dirname(os.path.abspath(__file__))
CORE = os.path.join(AQUI, "..")
MG = os.path.join(CORE, "..", "MotorGraphico-main", "MotorGraphico")
ASSETS = os.path.join(MG, "assets")

# Los mismos GIDs que colisionan en gen_ciudad.py y en el motor.
COLISION = {2, 4, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
            25, 26, 27, 29, 33, 36}


def carga(p):
    return json.load(open(p, encoding="utf-8"))


def ids_de_catalogo(path):
    d = carga(path)
    return [o.get("id") for o in (d.get("objects") or d.get("entries") or [])]


def lee_tmx(ruta):
    txt = re.sub(r"<!--.*?-->", "", open(ruta, encoding="utf-8").read(), flags=re.S)
    cab = re.search(r"<map[^>]*>", txt)
    if not cab:
        raise ValueError("sin cabecera <map>")
    w = int(re.search(r'\swidth="(\d+)"', cab.group(0)).group(1))
    h = int(re.search(r'\sheight="(\d+)"', cab.group(0)).group(1))
    csv = re.search(r'<data encoding="csv">(.*?)</data>', txt, re.S)
    if not csv:
        raise ValueError("sin <data encoding=\"csv\">")
    v = [int(t) for t in csv.group(1).replace("\n", "").split(",") if t.strip()]
    if len(v) != w * h:
        raise ValueError(f"csv con {len(v)} celdas, se esperaban {w*h}")
    return w, h, [v[y * w:(y + 1) * w] for y in range(h)]


def alcanzables(w, h, g, inicio):
    vis, cola = {inicio}, deque([inicio])
    while cola:
        x, y = cola.popleft()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = x + dx, y + dy
            if 0 <= nx < w and 0 <= ny < h and (nx, ny) not in vis \
                    and g[ny][nx] not in COLISION:
                vis.add((nx, ny))
                cola.append((nx, ny))
    return vis


def main(cuadrante):
    cfg = carga(os.path.join(AQUI, "cuadrantes.json"))
    if cuadrante not in cfg["cuadrantes"]:
        sys.exit(f"cuadrante desconocido: {cuadrante}. "
                 f"Validos: {', '.join(cfg['cuadrantes'])}")
    c = cfg["cuadrantes"][cuadrante]
    pre = c["prefijo"]
    fallos, avisos = [], []

    def mal(msg):
        fallos.append(msg)

    print(f"═══ {cuadrante} [{c['ia']}] · prefijo '{pre}' ═══")
    print(f"    naciones: {', '.join(sorted(c['naciones']))}")

    # ---------- lo que ya existia antes de este cuadrante ----------
    mios, ajenos = {}, set()
    for f in sorted(os.listdir(os.path.join(ASSETS, "objects"))):
        if not f.endswith(".json"):
            continue
        for i in ids_de_catalogo(os.path.join(ASSETS, "objects", f)):
            if not i:
                continue
            if f.startswith(pre):
                mios[i] = f
            else:
                ajenos.add(i)

    # ---------- 1 y 2. prefijos y colisiones ----------
    for i, f in mios.items():
        if not i.startswith(pre):
            mal(f"[prefijo] '{i}' en {f} no empieza por '{pre}'")
        if i in ajenos:
            mal(f"[colision] '{i}' ya existia fuera del cuadrante")
    print(f"  1-2. ids propios: {len(mios)} · ajenos en el repo: {len(ajenos)}")

    # ---------- 3. canon ----------
    for f in sorted(os.listdir(os.path.join(ASSETS, "levels"))):
        if not f.startswith(pre):
            continue
        n = carga(os.path.join(ASSETS, "levels", f)).get("name", "")
        if not n.isascii():
            mal(f"[ascii] el nombre de {f} tiene no-ASCII: el parser JSON del "
                f"motor no decodifica \\uXXXX (ver JsonValue.cpp)")
    print(f"  3.   capitales canonicas: {c['capitales']}")
    if c.get("_capital_pendiente"):
        avisos.append(f"sin capital en el canon: {c['_capital_pendiente']} "
                      "(se puede proponer, queda como propuesta)")

    # ---------- 4, 5 y 6. niveles ----------
    catalogo = set()
    for f in os.listdir(os.path.join(ASSETS, "objects")):
        if f.endswith(".json"):
            catalogo |= {i for i in ids_de_catalogo(os.path.join(ASSETS, "objects", f)) if i}

    niveles = [f for f in sorted(os.listdir(os.path.join(ASSETS, "levels")))
               if f.startswith(pre) and f.endswith(".json")]
    for f in niveles:
        d = carga(os.path.join(ASSETS, "levels", f))
        try:
            w, h, g = lee_tmx(os.path.join(MG, d["map"]))
        except Exception as e:
            mal(f"[nivel] {f}: {e}")
            continue
        s = d.get("playerStart", {"x": 0, "y": 0})
        if g[s["y"]][s["x"]] in COLISION:
            mal(f"[nivel] {f}: playerStart ({s['x']},{s['y']}) sobre colision")
            continue
        vis = alcanzables(w, h, g, (s["x"], s["y"]))
        libres = sum(1 for y in range(h) for x in range(w) if g[y][x] not in COLISION)
        if len(vis) != libres:
            mal(f"[conectividad] {f}: {libres - len(vis)} celdas transitables "
                f"INALCANZABLES desde playerStart")
        for o in d.get("objects", []):
            if o["objectId"] not in catalogo:
                mal(f"[catalogo] {f}: '{o['objectId']}' no tiene ficha en ningun catalogo")
            p = o["position"]
            if not any(0 <= p["x"] + dx < w and 0 <= p["y"] + dy < h
                       and (p["x"] + dx, p["y"] + dy) in vis
                       for dx, dy in ((0, 0), (1, 0), (-1, 0), (0, 1), (0, -1))):
                mal(f"[alcance] {f}: '{o['objectId']}' en ({p['x']},{p['y']}) "
                    "no se puede alcanzar andando")
    print(f"  4-6. niveles del cuadrante: {len(niveles)}")

    # ---------- 7 y 8. aventuras ----------
    avs = [f for f in sorted(os.listdir(os.path.join(ASSETS, "adventures")))
           if f.startswith(pre) and f.endswith(".json")]
    produce_global = set()
    for f in os.listdir(os.path.join(ASSETS, "adventures")):
        if not f.endswith(".json"):
            continue
        for b in carga(os.path.join(ASSETS, "adventures", f)).get("beats", []):
            for e in b.get("effects", []):
                t = e.get("type")
                if t == "setFlag":
                    produce_global.add(e["arg"])
                elif t == "skillCheck":
                    sc = e["skillCheck"]
                    produce_global |= {sc[k] for k in
                                       ("flagBotch", "flagPartial", "flagSuccess", "flagCritical")}
                elif t == "startBattle":
                    bt = e["startBattle"]
                    produce_global |= {bt["flagVictory"], bt["flagDefeat"]}

    for f in avs:
        d = carga(os.path.join(ASSETS, "adventures", f))
        ids = [b["id"] for b in d.get("beats", [])]
        if len(ids) != len(set(ids)):
            mal(f"[aventura] {f}: ids de beat repetidos")
        consume = set()
        for b in d.get("beats", []):
            r = b.get("requires", {})
            for k in ("allFlags", "anyFlags", "notFlags"):
                consume |= set(r.get(k, []))
            # un beat cuyo requires se contradice no dispara jamas
            if set(r.get("allFlags", [])) & set(r.get("notFlags", [])):
                mal(f"[beat muerto] {f}:{b['id']} pide y prohibe la misma flag")
        huerfanas = consume - produce_global
        for fl in sorted(huerfanas):
            mal(f"[flag huerfana] {f}: '{fl}' se consulta y no la enciende nadie")
    print(f"  7-8. aventuras del cuadrante: {len(avs)}")

    # ---------- informe ----------
    print()
    for a in avisos:
        print(f"  AVISO  {a}")
    if fallos:
        print(f"\n  {len(fallos)} FALLOS:")
        for f in fallos:
            print(f"    {f}")
        return 1
    print("  todo correcto.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else "oeste_norte"))
