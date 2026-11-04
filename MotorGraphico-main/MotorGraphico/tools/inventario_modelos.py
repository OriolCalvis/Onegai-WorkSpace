#!/usr/bin/env python3
"""MODELOS-PL-01 — inventario de los modelos antiguos.

Para cada nivel de assets/levels/ y cada actor (PNJ y enemigo) colocado, o que
puede aparecer en una oleada / cinematica / mision, saca la hoja que le toca
con el MISMO criterio que Game::RegistroSprites (alias, patrones, race:<raza>)
y sus columnas: 40 = PixelLab en diagonal; 12 o 20 = antiguo; sin hoja =
«cuerpo base» (personaje.png).

Uso (desde MotorGraphico/):
    python3 tools/inventario_modelos.py            # escribe out/modelos_antiguos.md
    python3 tools/inventario_modelos.py --solo-resumen
Sale con 1 si queda algun actor HUMANOIDE colocado en cuerpo base o en una hoja
de 12/20 columnas (los no humanoides van a la lista «necesita dibujo nuevo»).
"""
import argparse
import collections
import glob
import json
import os
import re
import sys

RAIZ = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
ASSETS = os.path.join(RAIZ, "assets")
RESPALDO_RAZA = "race_human_marches"  # fila neutra de respaldo (ver ApplicationNivel / RegistroSprites)


def jl(ruta):
    with open(ruta, encoding="utf-8") as f:
        return json.load(f)


def cargar_registro():
    """Reproduce RegistroSprites::carga: las hojas se cargan en orden y la ultima gana."""
    idx = jl(os.path.join(ASSETS, "textures", "sprites.json"))
    actores, hojas, todas = {}, {}, []
    for h in idx["hojas"]:
        man = jl(os.path.join(ASSETS, h["manifiesto"]))
        hojas[h["id"]] = {"cols": man.get("columns", 1), "filas": man.get("rows", 1)}
        pref = h.get("prefijo", "")
        if isinstance(man.get("actores"), list):
            for a in man["actores"]:
                actores[pref + a["id"]] = {"hoja": h["id"], "fila": a.get("fila", 0)}
                todas.append((pref + a["id"], h["id"]))
    pats = [(("sufijo", p["sufijo"]) if "sufijo" in p else ("prefijo", p["prefijo"]), p["id"])
            for p in idx.get("patrones", [])]
    return actores, hojas, idx.get("alias", {}), pats, todas


def resolver(reg, oid, race):
    actores, hojas, alias, pats, _todas = reg
    cand = [alias.get(oid, oid)]
    for (tipo, txt), pid in pats:
        if (oid.endswith(txt) if tipo == "sufijo" else oid.startswith(txt)):
            cand.append(pid)
    for c in cand:
        if c in actores:
            a = actores[c]
            return a["hoja"], hojas[a["hoja"]]["cols"], c
    if race and ("race:" + race) in actores:
        a = actores["race:" + race]
        return a["hoja"], hojas[a["hoja"]]["cols"], "race:" + race
    return None, None, None


def cargar_catalogo():
    """Union de assets/objects/*.json (los del manifiesto de la campana al final: ganan)."""
    orden = sorted(glob.glob(os.path.join(ASSETS, "objects", "*.json")))
    camp = set()
    for p in glob.glob(os.path.join(ASSETS, "proyectos", "*.json")):
        for c in jl(p).get("catalogs", []) or []:
            camp.add(os.path.join(ASSETS, "objects", c))
    orden = [o for o in orden if o not in camp] + [o for o in orden if o in camp]
    cat = {}
    for f in orden:
        d = jl(f)
        objs = d.get("objects", d) if isinstance(d, dict) else d
        if isinstance(objs, dict):
            objs = list(objs.values())
        if not isinstance(objs, list):
            continue
        for x in objs:
            if isinstance(x, dict) and x.get("category") in ("npc", "enemy") and x.get("id"):
                cat[x["id"]] = {"cat": x["category"], "raza": x.get("raceId", ""), "nombre": x.get("name", ""),
                                "fuente": os.path.basename(f)}
    return cat


FUENTES_HUMANAS = {"boundington_enemigos.json"}  # bandidos, guardias, mercenarios y cultistas con nombre
PRIORIDAD = re.compile(r"ciudad|boundington|afueras|aldea|interior|gorran|cueva|montana|cripta|antecova|camara|plaza")


def cadenas(o):
    if isinstance(o, str):
        yield o
    elif isinstance(o, dict):
        for k, v in o.items():
            yield from cadenas(v)
    elif isinstance(o, list):
        for v in o:
            yield from cadenas(v)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--solo-resumen", action="store_true")
    ap.add_argument("--salida", default=os.path.join(RAIZ, "out", "modelos_antiguos.md"))
    args = ap.parse_args()
    reg = cargar_registro()
    cat = cargar_catalogo()

    colocados = collections.defaultdict(lambda: collections.Counter())  # id -> nivel -> n
    for f in sorted(glob.glob(os.path.join(ASSETS, "levels", "*.json"))):
        nivel = os.path.basename(f)[:-5]
        d = jl(f)
        for o in d.get("objects", []) if isinstance(d, dict) else []:
            oid = o.get("objectId") if isinstance(o, dict) else None
            if oid in cat:
                colocados[oid][nivel] += 1

    # lo que puede aparecer sin estar colocado: oleadas, emboscadas, acompanantes, cinematicas, misiones
    mencion = collections.defaultdict(set)
    fuentes = glob.glob(os.path.join(ASSETS, "adventures", "*.json")) + \
        glob.glob(os.path.join(ASSETS, "cinematicas", "*.json")) + \
        glob.glob(os.path.join(ASSETS, "misiones", "*.json"))
    for f in fuentes:
        base = os.path.basename(f)[:-5]
        for s in cadenas(jl(f)):
            t = s[len("acompanante_"):] if s.startswith("acompanante_") else s
            if t in cat:
                mencion[t].add(base)

    ids = set(colocados) | set(mencion)
    filas = []
    for oid in ids:
        info = cat[oid]
        hoja, cols, via = resolver(reg, oid, info["raza"])
        niv = colocados.get(oid, collections.Counter())
        peso = sum(n * (1000 if PRIORIDAD.search(l) else 1) for l, n in niv.items())
        filas.append({"id": oid, **info, "hoja": hoja, "cols": cols, "via": via, "niveles": niv,
                      "total": sum(niv.values()), "peso": peso, "menciones": sorted(mencion.get(oid, []))})

    def estado(r):
        if r["hoja"] is None:
            return "cuerpo base"
        return "PixelLab 40" if r["cols"] == 40 else "ANTIGUO %s" % r["cols"]

    # un actor es «malo» si no esta en PixelLab diagonal
    malos = [r for r in filas if estado(r) != "PixelLab 40"]
    # humanoide: PNJ siempre; enemigo solo si tiene raza
    def humanoide(r):
        # PNJ siempre; enemigo si tiene raza o es de la trama (boundington_enemigos.json: personas con nombre)
        return r["cat"] == "npc" or bool(r["raza"]) or r["fuente"] in FUENTES_HUMANAS
    malos_hum = [r for r in malos if humanoide(r) and r["total"] > 0]
    malos_hum_oleada = [r for r in malos if humanoide(r) and r["total"] == 0]
    sin_dibujo = [r for r in malos if not humanoide(r)]

    out = []
    w = out.append
    w("# Modelos antiguos — inventario\n")
    w("Generado por `tools/inventario_modelos.py` (volver a pasar cuando cambie `sprites.json`, los objetos o los niveles).\n")
    w("Criterio: el de `Game::RegistroSprites::actor` (alias, patrones, `race:<raceId>`); la ultima hoja cargada gana. "
      "40 columnas = PixelLab en diagonal; 12/20 = antiguo; «cuerpo base» = `personaje.png`.\n")
    por_estado = collections.Counter(estado(r) for r in filas)
    w("## Resumen\n")
    w("- Actores distintos (colocados o con mencion en trama): **%d**" % len(filas))
    for k, v in sorted(por_estado.items()):
        w("- %s: %d" % (k, v))
    w("- **Humanoides colocados sin PixelLab: %d** (objetivo 0)" % len(malos_hum))
    w("- Humanoides solo en oleada/cinematica sin PixelLab: %d" % len(malos_hum_oleada))
    w("- No humanoides sin dibujo PixelLab (punto 4): %d\n" % len(sin_dibujo))

    def tabla(rs, titulo):
        w("## %s\n" % titulo)
        if not rs:
            w("_Ninguno._\n")
            return
        grupos = collections.defaultdict(list)
        for r in rs:
            grupos[(r["hoja"] or "cuerpo base", r["cols"])].append(r)
        orden = sorted(grupos.items(), key=lambda kv: -sum(r["peso"] for r in kv[1]))
        for (hoja, cols), lst in orden:
            w("### %s%s — %d actores\n" % (hoja, (" (%s columnas)" % cols) if cols else "", len(lst)))
            w("| id | tipo | raza | colocados | niveles (mas visibles primero) | otras menciones |")
            w("|---|---|---|---|---|---|")
            for r in sorted(lst, key=lambda r: -r["peso"]):
                niv = sorted(r["niveles"].items(), key=lambda kv: (not PRIORIDAD.search(kv[0]), -kv[1], kv[0]))
                nv = ", ".join("%s×%d" % (l, n) for l, n in niv[:5]) + (" …" if len(niv) > 5 else "")
                w("| `%s` | %s | %s | %d | %s | %s |" % (r["id"], r["cat"], r["raza"] or "—", r["total"], nv or "—",
                                                      ", ".join(r["menciones"][:3]) or "—"))
            w("")

    tabla(malos_hum, "Humanoides colocados que NO van en PixelLab (hay que arreglarlos)")
    tabla(malos_hum_oleada, "Humanoides que pueden aparecer (oleada/cinematica) sin PixelLab")
    tabla(sin_dibujo, "No humanoides sin PixelLab (punto 4: necesitan dibujo nuevo)")

    # las hojas antiguas que siguen cargadas y quien las lleva de verdad
    antiguas = sorted((k, {"hoja": h}) for k, h in reg[4] if reg[1][h]["cols"] != 40)
    w("## Filas de hojas antiguas (12/20 columnas) que siguen en `sprites.json`\n")
    w("Una fila antigua solo se ve si nadie mas (ni un alias, ni una hoja posterior) responde por su id.\n")
    w("| fila | hoja | columnas | ¿la gana otra hoja? | actores que la usan |")
    w("|---|---|---|---|---|")
    for k, v in antiguas:
        gana = reg[0][k]["hoja"] == v["hoja"]
        usan = [r["id"] for r in filas if r["via"] == k] if gana else []
        w("| `%s` | %s | %s | %s | %s |" % (k, v["hoja"], reg[1][v["hoja"]]["cols"], "no (se ve)" if gana else "si",
                                           ", ".join(usan) or "—"))
    w("")
    # por nivel: cuantos actores de cada estado
    w("## Por nivel (Boundington y Gorran primero)\n")
    w("| nivel | actores | PixelLab 40 | antiguo/cuerpo base humanoide | no humanoide sin dibujo |")
    w("|---|---|---|---|---|")
    niveles = collections.defaultdict(lambda: [0, 0, 0, 0])
    for r in filas:
        for l, n in r["niveles"].items():
            niveles[l][0] += n
            e = estado(r)
            if e == "PixelLab 40":
                niveles[l][1] += n
            elif humanoide(r):
                niveles[l][2] += n
            else:
                niveles[l][3] += n
    for l, v in sorted(niveles.items(), key=lambda kv: (not PRIORIDAD.search(kv[0]), kv[0])):
        w("| %s | %d | %d | %d | %d |" % (l, *v))
    w("")
    texto = "\n".join(out) + "\n"
    if not args.solo_resumen:
        os.makedirs(os.path.dirname(args.salida), exist_ok=True)
        with open(args.salida, "w", encoding="utf-8") as f:
            f.write(texto)
        print("escrito", args.salida)
    print("actores %d | PixelLab %d | humanoides colocados sin PixelLab %d | humanoides en oleada sin PixelLab %d | "
          "no humanoides sin dibujo %d" % (len(filas), por_estado.get("PixelLab 40", 0), len(malos_hum),
                                           len(malos_hum_oleada), len(sin_dibujo)))
    return 1 if malos_hum else 0


if __name__ == "__main__":
    sys.exit(main())
