#!/usr/bin/env python3
"""El mapamundi: la rejilla canonica 26x26, navegable, que une las masas.

    python3 tools/gen_mapamundi.py

EL PROBLEMA QUE RESUELVE

Habia ocho mapas de masa de tierra (mundi_landmass_1..8) y NINGUNO llevaba
a otro. Cada uno era una isla cerrada: sus puertas entraban en ciudades y
las ciudades volvian a esa misma isla. El mundo no era un mundo, eran ocho
mundos que no se hablan.

Ademas el canon tiene CATORCE masas (world_grid.json, 26x26), no ocho: seis
existen en la geografia y no tienen mapa. Aparecen aqui igualmente, con su
desembarco marcado como pendiente, porque un mapamundi al que le faltan
seis islas sin decirlo es un mapamundi que miente.

COMO SE UNE

Este mapa ES el mar. Cada masa de tierra se dibuja en su bbox del canon, y
en su costa se pone un DESEMBARCO que lleva a su mapa de detalle. Volver
es la puerta que este script anade en el otro extremo.

    mapamundi  --desembarco-->  mundi_landmass_N  --ciudad-->  ciudad_en_X
               <--regreso-----                    <--vuelta---

Asi el mundo entero es un solo grafo y se puede recorrer.

ESCALA. 4 tiles de mapa por celda de la rejilla: 26x26 celdas = 104x104
tiles. Con 1 tile por celda no cabe un desembarco sin pisar el mar, y con
8 el mapa se va a 208x208 y cruzarlo andando es un castigo.

IDEMPOTENTE.
"""
import json
import os
import re
import sys
from collections import deque

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
MAPAS = os.path.join(RAIZ, "assets", "maps")
NIVELES = os.path.join(RAIZ, "assets", "levels")
OBJETOS = os.path.join(RAIZ, "assets", "objects")
REJILLA = os.path.join(os.path.dirname(os.path.dirname(RAIZ)), "Onegai-Core", "atlas",
                       "world_grid.json")

ESCALA = 4          # tiles de mapa por celda de la rejilla canonica
NOMBRE = "mapamundi"


def carga_rejilla():
    if not os.path.exists(REJILLA):
        sys.exit("no encuentro la rejilla canonica en " + REJILLA)
    with open(REJILLA, encoding="utf-8") as f:
        return json.load(f)


def celdas_de_masa(grid, masa):
    """Las celdas (x,y) de la rejilla que pertenecen a esta masa.

    La rejilla guarda el NOMBRE DE NACION en cada celda, no el id de masa,
    asi que se cruza por bbox + nacion: dos masas pueden compartir nacion
    (Mistarium tiene tres islas) y solo el bbox las separa.
    """
    b = masa["bbox"]
    naciones = set(masa["naciones"])
    fuera = set()
    for y in range(b["y"], b["y"] + b["h"]):
        for x in range(b["x"], b["x"] + b["w"]):
            if y >= len(grid) or x >= len(grid[y]):
                continue
            v = grid[y][x]
            if v and v != "~" and v in naciones:
                fuera.add((x, y))
    return fuera


def main():
    d = carga_rejilla()
    ancho_celdas = d["dimensiones"]["ancho"]
    alto_celdas = d["dimensiones"]["alto"]
    grid = d["grid"]
    # El grid puede venir como lista de listas o de cadenas por fila.
    if grid and isinstance(grid[0], str):
        sys.exit("la rejilla viene como cadenas por fila; se esperaba lista de celdas")
    masas = d["masas_de_tierra"]

    W = ancho_celdas * ESCALA
    H = alto_celdas * ESCALA

    idx_path = os.path.join(RAIZ, "assets", "textures", "terreno_iso.json")
    with open(idx_path, encoding="utf-8") as f:
        idx = json.load(f)
    gid = {t["nombre"]: t["gid"] for t in idx["tiles"]}

    MAR = gid["rio"]              # bloquea: no se cruza el oceano andando
    COSTA = gid["arena"]
    TIERRA = gid["pradera"]
    MONTE = gid["roca"]

    rejilla = [[MAR] * W for _ in range(H)]

    # --- Pintar cada masa en su bbox ---
    ocupadas_por = {}
    for masa in masas:
        for (cx, cy) in celdas_de_masa(grid, masa):
            for dy in range(ESCALA):
                for dx in range(ESCALA):
                    x, y = cx * ESCALA + dx, cy * ESCALA + dy
                    if 0 <= x < W and 0 <= y < H:
                        rejilla[y][x] = TIERRA
                        ocupadas_por[(x, y)] = masa["id"]

    # Costa: tierra que toca mar. Da silueta y marca donde se desembarca.
    costa = []
    for y in range(H):
        for x in range(W):
            if rejilla[y][x] != TIERRA:
                continue
            if any(not (0 <= x + dx < W and 0 <= y + dy < H) or rejilla[y + dy][x + dx] == MAR
                   for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                costa.append((x, y))
    for (x, y) in costa:
        rejilla[y][x] = COSTA
    # Un poco de relieve dentro, para que no sea una mancha plana.
    for y in range(H):
        for x in range(W):
            if rejilla[y][x] == TIERRA and (x * 7 + y * 13) % 23 == 0:
                rejilla[y][x] = MONTE

    # --- Un desembarco por masa, en su costa ---
    # Se elige la celda de costa mas al sur-oeste: criterio fijo, para que
    # regenerar el mapa no mueva los desembarcos de sitio y rompa las
    # posiciones de vuelta que ya estan escritas en los otros niveles.
    desembarcos = {}
    for masa in masas:
        mias = [(x, y) for (x, y) in costa if ocupadas_por.get((x, y)) == masa["id"]]
        if not mias:
            continue
        mias.sort(key=lambda p: (-p[1], p[0]))
        desembarcos[masa["id"]] = mias[0]

    # --- Comprobar que TODOS los desembarcos son alcanzables por mar ---
    # Un puerto rodeado de tierra por los cuatro lados no se puede tocar
    # desde el barco, y eso deja una isla incomunicada otra vez.
    mar = [(x, y) for y in range(H) for x in range(W) if rejilla[y][x] == MAR]
    if not mar:
        sys.exit("ABORTA: no hay mar en el mapa")
    vistas = {mar[0]}
    cola = deque([mar[0]])
    while cola:
        x, y = cola.popleft()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (x + dx, y + dy)
            if (0 <= n[0] < W and 0 <= n[1] < H and n not in vistas
                    and rejilla[n[1]][n[0]] == MAR):
                vistas.add(n)
                cola.append(n)
    sueltos = []
    for mid, (x, y) in desembarcos.items():
        toca = any((x + dx, y + dy) in vistas for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
        if not toca:
            sueltos.append(mid)
    if sueltos:
        sys.exit("ABORTA: desembarcos que no tocan el mar navegable: " + ", ".join(sueltos))

    # --- TMX ---
    bloquean = {MAR}
    tileset = [
        f' <tileset firstgid="1" name="terreno_mundo" tilewidth="{idx["celda"]["ancho"]}"'
        f' tileheight="{idx["celda"]["alto"]}" tilecount="{idx["tileCountTotal"]}"'
        f' columns="{idx["columnas"]}">',
        f'  <image source="../textures/terreno_mundo.png" width="{idx["anchoTotal"]}"'
        f' height="{idx["altoTotal"]}"/>',
    ]
    for c in sorted(bloquean):
        tileset += [f'  <tile id="{c - 1}">', '   <properties>',
                    '    <property name="collision" type="bool" value="true"/>',
                    '   </properties>', '  </tile>']
    tileset.append(' </tileset>')

    csv = ",\n".join(",".join(str(v) for v in fila) for fila in rejilla)
    tmx = ('<?xml version="1.0" encoding="UTF-8"?>\n'
           f'<map version="1.10" tiledversion="1.10.2" orientation="orthogonal"'
           f' renderorder="right-down" width="{W}" height="{H}" tilewidth="64" tileheight="32"'
           f' infinite="0" nextlayerid="2" nextobjectid="1">\n'
           + "\n".join(tileset) + "\n"
           f' <layer id="1" name="suelo" width="{W}" height="{H}">\n'
           '  <data encoding="csv">\n' + csv + '\n  </data>\n </layer>\n</map>\n')
    open(os.path.join(MAPAS, NOMBRE + ".tmx"), "w", encoding="utf-8").write(tmx)

    # --- Nivel del mapamundi, con un desembarco por masa ---
    objetos = []
    fichas = []
    con_mapa, sin_mapa = 0, []
    for masa in masas:
        mid = masa["id"]
        if mid not in desembarcos:
            continue
        x, y = desembarcos[mid]
        destino = f"assets/levels/mundi_{mid}.json"
        existe = os.path.exists(os.path.join(RAIZ, destino))
        oid = "desembarco_" + mid
        naciones = ", ".join(masa["naciones"])
        obj = {"objectId": oid, "position": {"x": x, "y": y}}
        if existe:
            obj["targetLevel"] = destino
            con_mapa += 1
        else:
            sin_mapa.append(mid)
        objetos.append(obj)
        fichas.append({
            "id": oid,
            "name": ("Desembarco: " + naciones) if existe else ("Costa sin explorar: " + naciones),
            "category": "prop",
            "spriteId": -1,
            "blocksMovement": False,
            "interactable": True,
            "dialogue": ([f"Se pisa tierra de {naciones}."] if existe else
                         [f"Costa de {naciones}.",
                          "No hay mapa de este sitio todavia. El barco sigue."]),
        })

    nivel = {
        "_fuente": "Generado por tools/gen_mapamundi.py desde la rejilla canonica del atlas.",
        "name": "Egaroth",
        "map": f"assets/maps/{NOMBRE}.tmx",
        "playerStart": {"x": desembarcos["landmass_1"][0], "y": desembarcos["landmass_1"][1]},
        "objects": objetos,
    }
    with open(os.path.join(NIVELES, NOMBRE + ".json"), "w", encoding="utf-8") as f:
        json.dump(nivel, f, ensure_ascii=False, indent=1)

    with open(os.path.join(OBJETOS, "mapamundi_objetos.json"), "w", encoding="utf-8") as f:
        json.dump({"_fuente": "Desembarcos del mapamundi. Generado por tools/gen_mapamundi.py.",
                   "objects": fichas}, f, ensure_ascii=False, indent=1)

    # --- La vuelta: cada masa necesita una puerta al mapamundi ---
    vueltas = 0
    for masa in masas:
        mid = masa["id"]
        ruta = os.path.join(NIVELES, f"mundi_{mid}.json")
        if not os.path.exists(ruta) or mid not in desembarcos:
            continue
        lvl = json.load(open(ruta, encoding="utf-8"))
        lvl["objects"] = [o for o in lvl["objects"] if o["objectId"] != "regreso_al_mundo"]
        ps = lvl["playerStart"]
        # Junto al inicio del jugador: es donde se aparece al desembarcar,
        # asi que la vuelta esta donde se llego. Sin buscar hueco libre
        # complicado: si la celda esta ocupada, se pone en el propio inicio,
        # que por definicion es transitable.
        ocupadas = {(o["position"]["x"], o["position"]["y"]) for o in lvl["objects"]}
        destino = (ps["x"], ps["y"])
        for cand in ((ps["x"] + 1, ps["y"]), (ps["x"] - 1, ps["y"]),
                     (ps["x"], ps["y"] + 1), (ps["x"], ps["y"] - 1)):
            if cand[0] >= 0 and cand[1] >= 0 and cand not in ocupadas:
                destino = cand
                break
        dx, dy = desembarcos[mid]
        lvl["objects"].append({
            "objectId": "regreso_al_mundo",
            "position": {"x": destino[0], "y": destino[1]},
            "targetLevel": f"assets/levels/{NOMBRE}.json",
            "targetPosition": {"x": dx, "y": dy},
        })
        with open(ruta, "w", encoding="utf-8") as f:
            json.dump(lvl, f, ensure_ascii=False, indent=1)
        vueltas += 1

    fichas.append({
        "id": "regreso_al_mundo",
        "name": "Volver al barco",
        "category": "prop",
        "spriteId": -1,
        "blocksMovement": False,
        "interactable": True,
        "dialogue": ["El barco espera donde se dejo."],
    })
    with open(os.path.join(OBJETOS, "mapamundi_objetos.json"), "w", encoding="utf-8") as f:
        json.dump({"_fuente": "Desembarcos del mapamundi. Generado por tools/gen_mapamundi.py.",
                   "objects": fichas}, f, ensure_ascii=False, indent=1)

    print(f"  {NOMBRE}.tmx  {W}x{H} tiles ({ancho_celdas}x{alto_celdas} celdas x{ESCALA})")
    print(f"  {len(desembarcos)} desembarcos: {con_mapa} con mapa, {len(sin_mapa)} sin explorar")
    if sin_mapa:
        print(f"    sin mapa todavia: {', '.join(sin_mapa)}")
    print(f"  {vueltas} masas ganan puerta de regreso al mapamundi")
    return 0


if __name__ == "__main__":
    sys.exit(main())
