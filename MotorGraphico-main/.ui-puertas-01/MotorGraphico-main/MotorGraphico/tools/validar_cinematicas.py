#!/usr/bin/env python3
"""PROLOGO-02 — Comprueba assets/cinematicas/cinematicas.json contra lo demás:

  - los niveles que nombra existen;
  - quien anda, se coloca o mira está en el nivel donde lo hace (o es «jugador»);
  - las celdas caen dentro del mapa;
  - cada «beat» lo pide algún beat de las aventuras (si no, la cinemática se
    para a esperar un diálogo que nunca sale: no se atasca, pero no dice nada);
  - lo que «requiere» una cinemática lo enciende alguien (un beat, una opción u
    otra cinemática);
  - los objetos de las apariciones y de los espectros existen en los catálogos.

    python3 tools/validar_cinematicas.py      # 0 errores -> sale con 0
"""
import glob
import json
import os
import re
import sys

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def lee(rel):
    return json.load(open(os.path.join(RAIZ, rel), encoding="utf-8"))


def lista(v):
    return [v] if isinstance(v, str) else list(v or [])


def main():
    ruta = "assets/cinematicas/cinematicas.json"
    if not os.path.exists(os.path.join(RAIZ, ruta)):
        print("validar_cinematicas: no hay cinematicas")
        return 0
    cat = lee(ruta)
    errores, avisos = [], []

    # lo que piden y lo que encienden las aventuras
    pedidas, encendidas = set(), set()
    for aventura in glob.glob(os.path.join(RAIZ, "assets/adventures/*.json")):
        d = json.load(open(aventura, encoding="utf-8"))
        for b in d.get("beats", []):
            pedidas.update((b.get("requires") or {}).get("allFlags", []))
            efs = list(b.get("effects", []))
            for o in b.get("opciones", []):
                efs += o.get("effects", [])
            for e in efs:
                if e.get("type") == "setFlag":
                    encendidas.add(e.get("arg"))
                sb = e.get("startBattle") or {}
                encendidas.update(v for k, v in sb.items() if k in ("flagVictory", "flagDefeat", "flagHuida") and isinstance(v, str))
                sc = e.get("skillCheck") or {}
                encendidas.update(v for k, v in sc.items() if k.startswith("flag"))
    for c in cat.get("cinematicas", []):
        encendidas.add(f"cine_{c['id']}_vista")
        encendidas.update(lista(c.get("alTerminar")))
        for p in c.get("pasos", []):
            if "flag" in p:
                encendidas.add(p["flag"])
            if "beat" in p:
                encendidas.add(p["beat"])
    # el juego enciende estas (el cofre, los temporizadores)
    encendidas.update({"kit_basico_recogido", "kit_basico_puesto", "bnd_prologo_kit_listo"})

    proyecto = lee("assets/proyectos/boundington.json")
    ids = set()
    for c in proyecto["catalogs"]:
        p = os.path.join(RAIZ, "assets/objects", c)
        if os.path.exists(p):
            ids.update(o["id"] for o in json.load(open(p, encoding="utf-8")).get("objects", []))

    def nivel(rel):
        p = os.path.join(RAIZ, rel)
        if not os.path.exists(p):
            return None
        d = json.load(open(p, encoding="utf-8"))
        tmx = open(os.path.join(RAIZ, d["map"]), encoding="utf-8").read()
        m = re.search(r'<map [^>]*width="(\d+)" height="(\d+)"', tmx)
        return {"objetos": {o["objectId"] for o in d.get("objects", [])},
                "w": int(m.group(1)), "h": int(m.group(2))}

    for c in cat.get("cinematicas", []):
        donde = f"cinematica «{c['id']}»"
        actual = nivel(c["nivel"]) if c.get("nivel") else None
        if c.get("nivel") and actual is None:
            errores.append(f"{donde}: no existe el nivel {c['nivel']}")
        for f in lista(c.get("requiere")):
            if f not in encendidas:
                errores.append(f"{donde}: pide «{f}» y no la enciende nadie")
        for k, p in enumerate(c.get("pasos", [])):
            d = f"{donde}, paso {k}"
            if "nivel" in p:
                actual = nivel(p["nivel"])
                if actual is None:
                    errores.append(f"{d}: no existe el nivel {p['nivel']}")
            quien = p.get("anda") or p.get("coloca") or p.get("mira")
            if quien and quien != "jugador" and actual is not None and quien not in actual["objetos"]:
                errores.append(f"{d}: «{quien}» no está en ese nivel")
            celdas = ([p["a"]] if "a" in p else []) + list(p.get("por", [])) + ([p["en"]] if "en" in p else [])
            for x, y in celdas:
                if actual is not None and not (0 <= x < actual["w"] and 0 <= y < actual["h"]):
                    errores.append(f"{d}: la celda {(x, y)} cae fuera del mapa")
            if "beat" in p and p["beat"] not in pedidas:
                avisos.append(f"{d}: ningún beat pide «{p['beat']}» (no dirá nada)")
    for a in cat.get("apariciones", []):
        if a["objeto"] not in ids:
            errores.append(f"aparición: «{a['objeto']}» no está en ningún catálogo")
    for o in cat.get("espectro", {}).get("objetos", []):
        if o not in ids:
            errores.append(f"espectro: «{o}» no está en ningún catálogo")

    for e in errores:
        print("ERROR  ", e)
    for a in avisos:
        print("aviso  ", a)
    print(f"validar_cinematicas: {len(cat.get('cinematicas', []))} cinematicas, {len(errores)} errores, {len(avisos)} avisos")
    return 1 if errores else 0


if __name__ == "__main__":
    sys.exit(main())
