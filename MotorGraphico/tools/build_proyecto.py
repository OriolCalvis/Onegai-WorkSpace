#!/usr/bin/env python3
"""Saca una BUILD de un proyecto: esa historia, sola y jugable.

    python3 tools/build_proyecto.py boundington
    python3 tools/build_proyecto.py --todos

Produce builds/<id>/ con SOLO los assets de ese pack, mas el binario si se
pide. La gracia es que la carpeta se pueda mandar a alguien y funcione: hoy
assets/ tiene 96 niveles de cuatro autores mezclados, y "pasame Boundington"
significa pasar los 96 y que el otro adivine.

QUE HACE, EN ORDEN

  1. Lee el manifiesto del proyecto (assets/proyectos/<id>.json).
  2. Copia solo lo declarado: sus niveles, sus mapas, sus aventuras, sus
     catalogos. Y las texturas, que las comparten todos.
  3. SIGUE LAS REFERENCIAS. Un nivel puede llevar a otro por una puerta, y
     ese otro puede no estar en el manifiesto. Si no se arrastra, la build
     compila y el jugador cruza una puerta y se cae al vacio. Se sigue el
     cierre transitivo de targetLevel y se avisa de lo que se arrastro.
  4. Comprueba que cada objectId usado tenga ficha DENTRO de la build. Un
     catalogo que se quedo fuera no da error al cargar: da un objeto
     invisible, que es peor.
  5. Escribe build.json con lo que hay dentro y por donde arranca.

NO COMPILA C++. Eso es "cmake --build" y ya funciona; esto empaqueta la
historia, que es lo que no habia forma de hacer.
"""
import argparse
import json
import os
import shutil
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
ASSETS = os.path.join(RAIZ, "assets")
BUILDS = os.path.join(RAIZ, "builds")

CARPETAS = {"niveles": "levels", "mapas": "maps",
            "aventuras": "adventures", "catalogos": "objects"}


def carga(p):
    with open(p, encoding="utf-8") as f:
        return json.load(f)


def manifiesto(pid):
    ruta = os.path.join(ASSETS, "proyectos", pid + ".json")
    if not os.path.exists(ruta):
        sys.exit(f"no existe el proyecto '{pid}' ({ruta})")
    return carga(ruta)


def cierre_de_niveles(niveles):
    """Niveles alcanzables siguiendo targetLevel, empezando por los del
    manifiesto. Devuelve (conjunto, arrastrados)."""
    pendientes = list(niveles)
    vistos, arrastrados = set(), []
    while pendientes:
        n = pendientes.pop()
        if n in vistos:
            continue
        vistos.add(n)
        ruta = os.path.join(ASSETS, "levels", n)
        if not os.path.exists(ruta):
            continue
        d = carga(ruta)
        if "map" not in d:
            continue
        for o in d.get("objects", []):
            t = o.get("targetLevel")
            if not t:
                continue
            base = os.path.basename(t)
            if base not in vistos:
                pendientes.append(base)
                if base not in niveles:
                    arrastrados.append((n, base))
    return vistos, arrastrados


def construye(pid, con_binario=False):
    m = manifiesto(pid)
    destino = os.path.join(BUILDS, pid)
    if os.path.exists(destino):
        shutil.rmtree(destino, ignore_errors=True)
    for sub in ("levels", "maps", "adventures", "objects", "textures"):
        os.makedirs(os.path.join(destino, "assets", sub), exist_ok=True)

    niveles, arrastrados = cierre_de_niveles(list(m.get("niveles", [])))
    mapas = set(m.get("mapas", []))
    for n in niveles:                       # los mapas de lo arrastrado tambien
        r = os.path.join(ASSETS, "levels", n)
        if os.path.exists(r):
            d = carga(r)
            if "map" in d:
                mapas.add(os.path.basename(d["map"]))

    plan = {"levels": sorted(niveles), "maps": sorted(mapas),
            "adventures": list(m.get("aventuras", [])),
            "objects": list(m.get("catalogos", []))}

    copiados, faltan = 0, []
    for sub, ficheros in plan.items():
        for f in ficheros:
            org = os.path.join(ASSETS, sub, f)
            if not os.path.exists(org):
                faltan.append(f"{sub}/{f}")
                continue
            shutil.copy2(org, os.path.join(destino, "assets", sub, f))
            copiados += 1
    # Texturas: las comparten todos los proyectos, no hay que declararlas.
    # Texturas: las comparten todos los proyectos, no hay que declararlas.
    # copytree y no un bucle de copy2: dentro de textures/ hay CARPETAS
    # (race_npc_sprites, 43 ficheros), y copy2 sobre un directorio revienta
    # con IsADirectoryError a mitad de la build.
    tex = os.path.join(ASSETS, "textures")
    if os.path.isdir(tex):
        for raiz, _, ficheros in os.walk(tex):
            rel = os.path.relpath(raiz, tex)
            sub = os.path.join(destino, "assets", "textures", rel) if rel != "." \
                else os.path.join(destino, "assets", "textures")
            os.makedirs(sub, exist_ok=True)
            for f in ficheros:
                shutil.copy2(os.path.join(raiz, f), os.path.join(sub, f))
                copiados += 1

    # --- Comprobacion: todo objectId usado debe tener ficha DENTRO ---
    catalogo = set()
    dir_obj = os.path.join(destino, "assets", "objects")
    for f in os.listdir(dir_obj):
        d = carga(os.path.join(dir_obj, f))
        for o in (d.get("objects") or d.get("entries") or []):
            if o.get("id"):
                catalogo.add(o["id"])
    sin_ficha = set()
    for n in plan["levels"]:
        r = os.path.join(destino, "assets", "levels", n)
        if not os.path.exists(r):
            continue
        d = carga(r)
        for o in d.get("objects", []):
            if o["objectId"] not in catalogo:
                sin_ficha.add(o["objectId"])

    if con_binario:
        for cand in (os.path.join(RAIZ, "build", "juego"), os.path.join(RAIZ, "juego")):
            if os.path.exists(cand):
                shutil.copy2(cand, os.path.join(destino, "juego"))
                copiados += 1
                break

    info = {
        "_nota": "Build de un proyecto. Generado por tools/build_proyecto.py.",
        "id": pid, "nombre": m.get("nombre", pid), "epoca": m.get("epoca", ""),
        "autor": m.get("autor", ""), "entrada": m.get("entrada", ""),
        "contenido": {k: len(v) for k, v in plan.items()},
        "niveles_arrastrados": [{"desde": a, "arrastra": b} for a, b in arrastrados],
        "ficheros_declarados_que_faltan": faltan,
        "objectIds_sin_ficha": sorted(sin_ficha),
        "jugable": bool(m.get("entrada")) and not faltan and not sin_ficha,
    }
    with open(os.path.join(destino, "build.json"), "w", encoding="utf-8") as f:
        json.dump(info, f, ensure_ascii=False, indent=1)

    print(f"  {pid:13} {copiados:4} ficheros -> builds/{pid}/")
    print(f"                niveles {len(plan['levels']):3} · mapas {len(plan['maps']):3} · "
          f"aventuras {len(plan['adventures'])} · catalogos {len(plan['objects'])}")
    if arrastrados:
        print(f"                arrastrados por puertas: {len(arrastrados)} "
              f"(p.ej. {arrastrados[0][0]} -> {arrastrados[0][1]})")
    if faltan:
        print(f"                FALTAN {len(faltan)}: {faltan[:4]}")
    if sin_ficha:
        print(f"                {len(sin_ficha)} objectId sin ficha: {sorted(sin_ficha)[:4]}")
    if not info["jugable"]:
        motivo = ("sin entrada" if not m.get("entrada")
                  else "faltan ficheros" if faltan else "objetos sin ficha")
        print(f"                NO JUGABLE: {motivo}")
    return info


def main():
    ap = argparse.ArgumentParser(description="Saca la build de un proyecto.")
    ap.add_argument("proyecto", nargs="?", help="id del proyecto")
    ap.add_argument("--todos", action="store_true", help="todos los del indice")
    ap.add_argument("--con-binario", action="store_true",
                    help="copia tambien build/juego dentro de la build")
    a = ap.parse_args()

    ids = ([p for p in carga(os.path.join(ASSETS, "proyectos", "index.json"))["proyectos"]]
           if a.todos else [a.proyecto])
    if not ids or ids == [None]:
        ap.error("dime un proyecto o usa --todos")

    print(f"Builds en {os.path.relpath(BUILDS, RAIZ)}/\n")
    infos = [construye(i, a.con_binario) for i in ids]
    jug = sum(1 for i in infos if i["jugable"])
    print(f"\n{jug} de {len(infos)} jugables")
    return 0 if jug == len(infos) else 1


if __name__ == "__main__":
    sys.exit(main())
