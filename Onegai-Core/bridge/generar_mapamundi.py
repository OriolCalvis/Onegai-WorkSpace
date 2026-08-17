#!/usr/bin/env python3
"""world_grid.json + geografia.json  ->  TMX + nivel JSON del mapamundi.

Cada celda de la rejilla canonica 26x26 se expande a ESCALA x ESCALA tiles
caminables. El mar es colision. Las ciudades canonicas se colocan como
objetos con la transformacion afin ajustada entre los dos sistemas de
coordenadas (ver ajustar_proyeccion()).

Salida por masa de tierra: un mapamundi por continente/isla, que es como
el mundo ya se divide solo (13 masas, 2 continentes grandes).
"""
import json, os, sys, statistics, collections

ESCALA = 8            # tiles caminables por celda de la rejilla
GID_MAR, GID_TIERRA, GID_COSTA = 2, 1, 1   # GID 2 = colision (ver FORMATO_NIVELES.md)
N = 26
AQUI = os.path.dirname(os.path.abspath(__file__))
GEO = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "dndWeebCC-master", "data", "mapa", "geografia.json")
ALIAS = {"Ayashi": "Ayashii", "Giladdokx": "Gliaddokx",
         "Tabaxi Continental": "Tabaxi", "Tabaxi Occidental": "Tabaxi"}

W = json.load(open(os.path.join(AQUI, "..", "atlas", "world_grid.json"), encoding="utf-8"))
EPOCA = W.get("epoca", {})
G = json.load(open(GEO, encoding="utf-8"))
grid = W["grid"]


def ajustar_proyeccion():
    """Minimos cuadrados entre los centroides de nacion de ambas fuentes.
    Devuelve (ax,bx,ay,by) tal que celda = a*pixel + b."""
    cg = collections.defaultdict(list)
    for y in range(N):
        for x in range(N):
            c = grid[y][x]
            if c and c != "~":
                cg[c].append((x + .5, y + .5))
    cp = collections.defaultdict(list)
    for n in G["naciones"]:
        nm = ALIAS.get(n["nombre"], n["nombre"])
        cp[nm].extend(tuple(map(float, p.split(","))) for p in n["points"].split())
    cenG = {k: (statistics.mean(p[0] for p in v), statistics.mean(p[1] for p in v)) for k, v in cg.items()}
    cenP = {k: (statistics.mean(p[0] for p in v), statistics.mean(p[1] for p in v)) for k, v in cp.items() if v}
    com = sorted(set(cenG) & set(cenP))
    def fit(i):
        xs = [cenP[n][i] for n in com]; ys = [cenG[n][i] for n in com]
        mx, my = statistics.mean(xs), statistics.mean(ys)
        a = sum((x-mx)*(y-my) for x, y in zip(xs, ys)) / sum((x-mx)**2 for x in xs)
        return a, my - a*mx
    (ax, bx), (ay, by) = fit(0), fit(1)
    return ax, bx, ay, by


def tmx(w, h, capa, nombre):
    filas = ",\n".join(",".join(str(v) for v in capa[y]) for y in range(h))
    return f'''<?xml version="1.0" encoding="UTF-8"?>
<!-- GENERADO por generar_mapamundi.py — NO editar a mano.
     Fuente: Ciudades y sus elementos.xlsx Hoja 4 (rejilla canonica 26x26).
     EPOCA DEL MAPA: {EPOCA.get("etiqueta","(sin fechar)")} — {EPOCA.get("era","")}
     {nombre}: {w}x{h} tiles a {ESCALA} tiles por celda de rejilla.
     GID 1 = tierra transitable · GID 2 = mar (collision=true). -->
<map version="1.10" tiledversion="1.10.2" orientation="orthogonal" renderorder="right-down" width="{w}" height="{h}" tilewidth="64" tileheight="32" infinite="0" nextlayerid="2" nextobjectid="1">
 <tileset firstgid="1" name="mundi" tilewidth="8" tileheight="8" tilecount="2" columns="2">
  <image source="../textures/test_checker.png" width="16" height="8"/>
  <tile id="1">
   <properties>
    <property name="collision" type="bool" value="true"/>
   </properties>
  </tile>
 </tileset>
 <layer id="1" name="suelo" width="{w}" height="{h}">
  <data encoding="csv">
{filas}
</data>
 </layer>
</map>
'''


def main():
    ax, bx, ay, by = ajustar_proyeccion()
    os.makedirs(os.path.join(AQUI, "..", "atlas", "salida/maps"), exist_ok=True)
    os.makedirs(os.path.join(AQUI, "..", "atlas", "salida/levels"), exist_ok=True)
    ciudades = collections.defaultdict(list)
    for c in G["ciudades"]:
        ciudades[(int(ax*c["x"]+bx), int(ay*c["y"]+by))].append(c)
    resumen = []
    for m in W["masas_de_tierra"]:
        if m["celdas"] < 2:
            continue
        b = m["bbox"]
        w, h = b["w"]*ESCALA, b["h"]*ESCALA
        capa = [[GID_MAR]*w for _ in range(h)]
        for cy in range(b["y"], b["y"]+b["h"]):
            for cx in range(b["x"], b["x"]+b["w"]):
                v = grid[cy][cx]
                if v and v != "~" and v in m["naciones"]:
                    for ty in range((cy-b["y"])*ESCALA, (cy-b["y"]+1)*ESCALA):
                        for tx in range((cx-b["x"])*ESCALA, (cx-b["x"]+1)*ESCALA):
                            capa[ty][tx] = GID_TIERRA
        objetos, colocadas = [], 0
        for (gx, gy), lst in ciudades.items():
            if not (b["x"] <= gx < b["x"]+b["w"] and b["y"] <= gy < b["y"]+b["h"]):
                continue
            for i, c in enumerate(lst):
                tx = (gx-b["x"])*ESCALA + ESCALA//2 + (i % 3) - 1
                ty = (gy-b["y"])*ESCALA + ESCALA//2 + (i//3)
                if 0 <= ty < h and 0 <= tx < w and capa[ty][tx] == GID_TIERRA:
                    objetos.append({"objectId": c.get("id", c["nombre"]),
                                    "position": {"x": tx, "y": ty},
                                    "_nombre": c["nombre"], "_nacion": c["nacion"]})
                    colocadas += 1
        libres = sum(r.count(GID_TIERRA) for r in capa)
        inicio = next(({"x": x, "y": y} for y in range(h) for x in range(w)
                       if capa[y][x] == GID_TIERRA), {"x": 0, "y": 0})
        nom = m["id"]
        open(os.path.join(AQUI, "..", "atlas", "salida", "maps", f"mundi_{nom}.tmx"), "w", encoding="utf-8").write(
            tmx(w, h, capa, ", ".join(m["naciones"])))
        json.dump({"name": f"Mapamundi ({EPOCA.get('etiqueta','?')}) — " + ", ".join(m["naciones"]),
                   "epoca": EPOCA,
                   "map": f"assets/maps/mundi_{nom}.tmx",
                   "playerStart": inicio, "objects": objetos},
                  open(os.path.join(AQUI, "..", "atlas", "salida", "levels", f"mundi_{nom}.json"), "w", encoding="utf-8"),
                  ensure_ascii=False, indent=1)
        resumen.append((nom, w, h, libres, colocadas, len(m["naciones"])))
    print(f"{'masa':14} {'TMX':>10} {'tiles tierra':>13} {'ciudades':>9} {'naciones':>9}")
    print("-"*62)
    for nom, w, h, libres, col, nn in resumen:
        print(f"{nom:14} {w:4}x{h:<5} {libres:13,} {col:9} {nn:9}")
    print(f"\ntotal: {sum(r[4] for r in resumen)}/95 ciudades colocadas · "
          f"{sum(r[3] for r in resumen):,} tiles caminables")


if __name__ == "__main__":
    main()
