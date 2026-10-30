#!/usr/bin/env python3
"""Oriol (22/10): «els exteriors, el final del mapa és precipici, i falten els desnivells: queda tot pla».

Pone «alturas» (AV19-02) a un mapa de exterior a partir del camino:
- El suelo que se pisa sube con la distancia andando desde la entrada (el paso de abajo): cada
  `paso` casillas, un nivel. Dos casillas vecinas nunca se llevan más de un nivel, así que todo lo
  que se andaba se sigue andando; los cambios se ven como escalones y terrazas.
- La roca de entre los caminos sube dos niveles por encima del camino que tiene al lado (paredes
  de montaña), sin pasar de 9.
- La roca del borde del mapa (a `borde` casillas o menos del final) se queda a nivel 0: el camino
  termina en un precipicio, y por delante (abajo y a la derecha) se ve la caída.

    python3 tools/alturas_gorran.py montana_gorran            # escribe el nivel (deja .bak la primera vez)
    python3 tools/alturas_gorran.py montana_gorran --simular  # solo cuenta
    python3 tools/alturas_gorran.py santuario_subterraneo --niveles 0 --borde 0 --muro 3   # catacumbas: suelo plano,
                                                                        # paredes de roca de tres niveles

Las alturas van al JSON del nivel; nivel_a_excel.py las saca en la hoja «Alturas» para retocarlas.
"""
import argparse
import json
import re
import shutil
import sys
from collections import deque
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
sys.path.insert(0, str(HERE))

import nivel_excel_comun as C  # noqa: E402


def colisiones(tmx_texto):
    """Los gid que no se pisan: los del indice del tileset y los que el TMX marca con collision."""
    idx = json.loads((ROOT / "assets/textures/terreno_iso.json").read_text(encoding="utf-8"))
    gids = set(idx.get("colisionGids", []))
    first = int(re.search(r'<tileset[^>]*firstgid="(\d+)"', tmx_texto).group(1))
    for m in re.finditer(r'<tile id="(\d+)">(.*?)</tile>', tmx_texto, re.S):
        if re.search(r'name="collision"[^>]*value="true"', m.group(2)):
            gids.add(first + int(m.group(1)))
    return gids


def calcula(nombre, niveles=6, borde=2, muro=2):
    ruta = ROOT / f"assets/levels/{nombre}.json"
    nivel = json.loads(ruta.read_text(encoding="utf-8"))
    tmx = (ROOT / nivel["map"]).read_text(encoding="utf-8")
    ancho = int(re.search(r'<layer[^>]*width="(\d+)"', tmx).group(1))
    datos = re.search(r'<data encoding="csv">(.*?)</data>', tmx, re.S).group(1)
    gids = [int(v) & 0x1FFFFFFF for v in datos.replace("\n", "").split(",") if v.strip()]
    alto = len(gids) // ancho
    tapa = colisiones(tmx)
    pisa = [[gids[y * ancho + x] not in tapa for x in range(ancho)] for y in range(alto)]
    # La entrada: el paso del borde de abajo (o, si no hay, el inicio del jugador).
    pasos = [o for o in nivel["objects"] if o.get("targetLevel")]
    abajo = max((o["position"]["y"] for o in pasos), default=None)
    origen = [(o["position"]["x"], o["position"]["y"]) for o in pasos if o["position"]["y"] == abajo] or \
             [(nivel["playerStart"]["x"], nivel["playerStart"]["y"])]
    dist = [[-1] * ancho for _ in range(alto)]
    cola = deque()
    for x, y in origen:
        if 0 <= x < ancho and 0 <= y < alto and pisa[y][x]:
            dist[y][x] = 0
            cola.append((x, y))
    while cola:
        x, y = cola.popleft()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = x + dx, y + dy
            if 0 <= nx < ancho and 0 <= ny < alto and pisa[ny][nx] and dist[ny][nx] < 0:
                dist[ny][nx] = dist[y][x] + 1
                cola.append((nx, ny))
    maxd = max(max(f) for f in dist)
    # niveles = 0: el suelo plano (las catacumbas); solo suben las paredes de roca.
    paso = max(1, -(-maxd // niveles)) if niveles > 0 else maxd + 1  # techo
    g = [[0] * ancho for _ in range(alto)]
    for y in range(alto):
        for x in range(ancho):
            if pisa[y][x] and dist[y][x] >= 0:
                g[y][x] = min(niveles, dist[y][x] // paso)
    # El agua que no se pisa (el río) va un nivel POR DEBAJO de la orilla, no sube como la roca.
    idx = json.loads((ROOT / "assets/textures/terreno_iso.json").read_text(encoding="utf-8"))
    nombres = {t["gid"]: t["nombre"] for t in idx.get("tiles", [])}
    agua = {gid for gid, n in nombres.items() if any(k in n for k in ("rio", "agua", "mar", "lago", "charca"))}
    # La roca: pared por encima del camino de al lado; en el borde, el precipicio (nivel 0).
    for y in range(alto):
        for x in range(ancho):
            if pisa[y][x]:
                continue
            if gids[y * ancho + x] in agua:
                cerca = [g[ny][nx] for ny in range(max(0, y - 2), min(alto, y + 3))
                         for nx in range(max(0, x - 2), min(ancho, x + 3)) if pisa[ny][nx]]
                g[y][x] = max(0, (min(cerca) if cerca else 0) - 1)
                continue
            al_borde = min(x, y, ancho - 1 - x, alto - 1 - y) < borde
            if al_borde:
                continue
            cerca = [g[ny][nx] for ny in range(max(0, y - 3), min(alto, y + 4))
                     for nx in range(max(0, x - 3), min(ancho, x + 4)) if pisa[ny][nx]]
            g[y][x] = min(C.NIVEL_MAX, (max(cerca) if cerca else niveles) + muro)
    # Lo que se pisa junto a otra casilla pisable: nunca más de un nivel de diferencia.
    malos = sum(1 for y in range(alto) for x in range(ancho) if pisa[y][x]
                for dx, dy in ((1, 0), (0, 1)) if x + dx < ancho and y + dy < alto and pisa[y + dy][x + dx]
                and dist[y][x] >= 0 and dist[y + dy][x + dx] >= 0 and abs(g[y][x] - g[y + dy][x + dx]) > 1)
    return ruta, nivel, g, {"paso": paso, "maxd": maxd, "cortes": malos}


def main(argv):
    a = argparse.ArgumentParser()
    a.add_argument("nivel")
    a.add_argument("--niveles", type=int, default=6)
    a.add_argument("--borde", type=int, default=2)
    a.add_argument("--muro", type=int, default=2, help="cuántos niveles sube la roca sobre el camino de al lado")
    a.add_argument("--simular", action="store_true")
    a = a.parse_args(argv)
    ruta, nivel, g, info = calcula(a.nivel, a.niveles, a.borde, a.muro)
    regiones = C.regiones_de_rejilla(g)
    print(f"{a.nivel}: un nivel cada {info['paso']} casillas de camino (el mas largo, {info['maxd']}); "
          f"{len(regiones)} regiones; {info['cortes']} lindes del camino con más de un nivel")
    if info["cortes"]:
        print("  ERROR: el camino se cortaría", file=sys.stderr)
        return 1
    if a.simular:
        return 0
    bak = ruta.with_suffix(".json.bak")
    if not bak.exists():
        shutil.copy2(ruta, bak)
    nivel["alturas"] = regiones
    texto = ruta.read_text(encoding="utf-8")
    ind = len(re.match(r"\{\n( +)\S", texto).group(1))
    ruta.write_text(json.dumps(nivel, ensure_ascii=False, indent=ind) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
