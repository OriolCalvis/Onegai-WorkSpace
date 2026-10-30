#!/usr/bin/env python3
"""Oriol (22/10): «en la ciudad de Gorran también hay relieve: el agua está por debajo, hay puentes de roca
que pasan por encima, y desniveles para ir a diferentes sitios»; «cuanto más oscura es la tierra en el
Excel, un poco más alta es».

Pone las «alturas» (AV19-02) de un nivel a partir del suelo que pintó Oriol, de más claro (más bajo) a más
oscuro (más alto). En la Aldea Baja (cueva_gorran), con los colores de su Excel del 19/10:

    agua (río)                 0   por debajo de todo
    tierra de cañón (C3A275)   un nivel por debajo del suelo que la rodea: las calles van hundidas
    piedra antigua (9B9185)    3 si toca el suelo de cueva (un alto, «cima»); si no, 2
    losa oscura (867C78)       2   el suelo de la cueva
    barro (7D7075)             3   lo más oscuro: la meseta del sudoeste (la boca de las galerías)
    puente                     el nivel del suelo al que llega: pasa por encima del agua
    edificios                  el nivel de la casilla de delante de su puerta (el bloque se apoya ahí)
    roca                       dos por encima de lo más alto que tiene cerca (pared de la cueva)

Después comprueba que todo lo que se alcanzaba andando se sigue alcanzando (con un nivel de diferencia como
mucho entre casillas vecinas); si algo se queda aislado, lo dice y no escribe.

    python3 tools/alturas_por_suelo.py cueva_gorran            # escribe
    python3 tools/alturas_por_suelo.py cueva_gorran --simular  # solo cuenta

Para retocar a mano: la hoja «Alturas» del Excel (tools/nivel_a_excel.py) manda sobre esto.
"""
import argparse
import json
import re
import sys
from collections import Counter, deque
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
sys.path.insert(0, str(HERE))

import nivel_excel_comun as C  # noqa: E402
from alturas_gorran import colisiones  # noqa: E402

TONOS = {  # nivel: {tile: altura}; «calle» = un nivel por debajo de lo que la rodea, «cima» = ver arriba
    "cueva_gorran": {"rio": 0, "agua_somera": 1, "tierra_canon": "calle", "piedra_antigua": "cima",
                     "losa_oscura": 2, "barro": 3, "puente_piedra": "puente"},
}
EDIFICIOS = ("barraca", "losa_clara", "casa", "local", "caserna")
VECINAS = ((1, 0), (-1, 0), (0, 1), (0, -1))


def componentes(celdas):
    resto, out = set(celdas), []
    while resto:
        c = resto.pop()
        grupo, cola = {c}, deque([c])
        while cola:
            x, y = cola.popleft()
            for dx, dy in VECINAS:
                n = (x + dx, y + dy)
                if n in resto:
                    resto.remove(n)
                    grupo.add(n)
                    cola.append(n)
        out.append(grupo)
    return out


def borde(grupo):
    return {(x + dx, y + dy) for x, y in grupo for dx, dy in VECINAS} - grupo


def calcula(nombre):
    tabla = TONOS[nombre]
    ruta = ROOT / f"assets/levels/{nombre}.json"
    nivel = json.loads(ruta.read_text(encoding="utf-8"))
    tmx = (ROOT / nivel["map"]).read_text(encoding="utf-8")
    ancho = int(re.search(r'<layer[^>]*width="(\d+)"', tmx).group(1))
    datos = re.search(r'<data encoding="csv">(.*?)</data>', tmx, re.S).group(1)
    gids = [int(v) & 0x1FFFFFFF for v in datos.replace("\n", "").split(",") if v.strip()]
    alto = len(gids) // ancho
    idx = json.loads((ROOT / "assets/textures/terreno_iso.json").read_text(encoding="utf-8"))
    nom = {t["gid"]: t["nombre"] for t in idx["tiles"]}
    tapa = colisiones(tmx)
    tile = {(x, y): nom.get(gids[y * ancho + x], "") for y in range(alto) for x in range(ancho)}
    pisa = {c for c in tile if gids[c[1] * ancho + c[0]] not in tapa}
    g = {}
    for c, t in tile.items():
        if isinstance(tabla.get(t), int):
            g[c] = tabla[t]
    por_tile = lambda t: [grupo for grupo in componentes([c for c in tile if tile[c] == t])]  # noqa: E731

    def moda(celdas, defecto):
        vals = [g[c] for c in celdas if c in g and c in pisa]
        return Counter(vals).most_common(1)[0][0] if vals else defecto

    for t, regla in tabla.items():
        if regla == "cima":
            for grupo in por_tile(t):
                toca = any(tile.get(c) == "losa_oscura" for c in borde(grupo))
                for c in grupo:
                    g[c] = 3 if toca else 2
    for t, regla in tabla.items():
        if regla == "calle":
            for grupo in por_tile(t):
                alrededor = [c for c in borde(grupo) if tile.get(c) not in tabla or isinstance(tabla.get(tile.get(c)), int)
                             or tabla.get(tile.get(c)) == "cima"]
                base = moda(alrededor, 2)
                for c in grupo:
                    g[c] = max(0, base - 1)
    for t, regla in tabla.items():
        if regla == "puente":
            for grupo in por_tile(t):
                base = max((g[c] for c in borde(grupo) if c in g and c in pisa), default=2)
                for c in grupo:
                    g[c] = base
    # Los edificios: al nivel de lo que tienen delante (las puertas del nivel), o de lo que los rodea.
    puertas = [(o["position"]["x"], o["position"]["y"]) for o in nivel["objects"] if o.get("targetLevel")]
    for grupo in componentes([c for c in tile if c not in g and any(k in tile[c] for k in EDIFICIOS)]):
        delante = [n for p in puertas if p in grupo for n in borde({p}) if n in pisa and n not in grupo]
        base = moda(delante, None)
        if base is None:
            base = moda(borde(grupo), 2)
        for c in grupo:
            g[c] = base
    # La roca: pared por encima de lo que tiene cerca.
    for c in tile:
        if c in g:
            continue
        x, y = c
        cerca = [g[(nx, ny)] for ny in range(y - 3, y + 4) for nx in range(x - 3, x + 4) if (nx, ny) in g]
        g[c] = min(C.NIVEL_MAX, (max(cerca) if cerca else 2) + 2)
    rejilla = [[g[(x, y)] for x in range(ancho)] for y in range(alto)]
    return ruta, nivel, rejilla, pisa, (ancho, alto)


def alcanzables(nivel, pisa, rejilla):
    s = (nivel["playerStart"]["x"], nivel["playerStart"]["y"])
    visto, cola = {s}, deque([s])
    while cola:
        x, y = cola.popleft()
        for dx, dy in VECINAS:
            n = (x + dx, y + dy)
            if n in pisa and n not in visto and abs(rejilla[n[1]][n[0]] - rejilla[y][x]) <= 1:
                visto.add(n)
                cola.append(n)
    return visto


def main(argv):
    a = argparse.ArgumentParser()
    a.add_argument("nivel")
    a.add_argument("--simular", action="store_true")
    a = a.parse_args(argv)
    ruta, nivel, rejilla, pisa, (ancho, alto) = calcula(a.nivel)
    antes = alcanzables(nivel, pisa, [[0] * ancho for _ in range(alto)])
    ahora = alcanzables(nivel, pisa, rejilla)
    perdidos = antes - ahora
    objetos = [o for o in nivel["objects"] if (o["position"]["x"], o["position"]["y"]) in perdidos]
    regiones = C.regiones_de_rejilla(rejilla)
    cuenta = Counter(rejilla[y][x] for x, y in pisa)
    print(f"{a.nivel}: {len(regiones)} regiones; suelo por nivel {dict(sorted(cuenta.items()))}; "
          f"{len(perdidos)} casillas que se dejarían de alcanzar")
    if perdidos:
        for o in objetos[:20]:
            print(f"  aislado: {o['objectId']} en ({o['position']['x']},{o['position']['y']})", file=sys.stderr)
        print("  ERROR: no se escribe", file=sys.stderr)
        return 1
    if a.simular:
        return 0
    nivel["alturas"] = regiones
    texto = ruta.read_text(encoding="utf-8")
    ind = len(re.match(r"\{\n( +)\S", texto).group(1))
    ruta.write_text(json.dumps(nivel, ensure_ascii=False, indent=ind) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
