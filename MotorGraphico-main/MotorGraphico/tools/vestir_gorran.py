#!/usr/bin/env python3
"""Oriol (22/10): «dins de Gorran falta també de tot». Viste los mapas de Gorran que estaban casi vacíos
(las minas, las galerías, la Gola Fosca, el río y la Aldea Baja) con trastos de su sitio: puntales,
cuerdas y carretillas en las minas; cajas y barriles en las galerías del contrabando; hogueras,
carromatos y estandartes en la aldea.

Reglas, para no romper nada:
- Solo en casillas que se pisan y pegadas a la pared (dos vecinas que no se pisan, o una esquina):
  los pasillos y las plazas quedan libres.
- Nada a menos de dos casillas de una puerta o un paso, de un PNJ, de un cofre o de una casilla que
  use una cinemática (donde alguien anda, se coloca o mira).
- Después de cada trasto se comprueba que todas esas casillas importantes se siguen alcanzando
  andando; si alguno las corta, se quita.
- Determinista (misma semilla, mismos trastos) y repetible: los que pone van con la marca
  «vestido_gorran» y se quitan antes de volver a vestir.

    python3 tools/vestir_gorran.py            # todos los mapas de la lista
    python3 tools/vestir_gorran.py mina_gorran --simular
"""
import argparse
import json
import random
import re
import sys
from collections import deque
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
sys.path.insert(0, str(HERE))

from alturas_gorran import colisiones  # noqa: E402

MINA = ["puntal_roto", "rollo_cuerda", "cubo", "cajas", "sacos", "barriles_rotos", "farolillo", "huesos", "carro",
        "columna_rota", "brasero_piedras", "barril"]
ALMACEN = ["cajas", "cajas_almacen", "barriles", "barriles_pila", "sacos", "rollo_cuerda", "farolillo", "carro", "barril",
           "cubo"]
ALDEA = ["barriles", "cajas", "sacos", "hoguera", "brasero", "carromato", "carro", "estandarte", "jaula", "totem_huesos",
         "farolillo", "cubo", "rollo_cuerda", "barriles_pila", "banco_roto"]
CUEVA = ["velas_derretidas", "huesos", "barriles_rotos", "columna_rota", "brasero_piedras", "sacos", "cajas", "rollo_cuerda"]
RIO = ["huesos", "barriles_rotos", "rollo_cuerda", "cajas", "columna_rota", "sacos"]
MAPAS = {  # nivel: (trastos, cuantos por cada 100 casillas que se pisan)
    "mina_gorran": (MINA, 6), "mina2_gorran": (MINA, 6), "galeria_minas_cojo": (MINA, 5), "grieta_cojo": (MINA, 5),
    "galerias_gorran": (ALMACEN, 6), "antecova_gorran": (CUEVA, 4), "rio_gorran": (RIO, 4), "cueva_gorran": (ALDEA, 3),
}
MARCA = "vestido_gorran"


def importantes(nivel, nombre):
    celdas = set()
    for o in nivel["objects"]:
        if o.get(MARCA):
            continue
        oid = o["objectId"]
        if (o.get("targetLevel") or oid.startswith(("puerta_", "paso_", "salida_", "botin_", "cofre", "escalera"))
                or "npc" in oid or o.get("_npc")):
            celdas.add((o["position"]["x"], o["position"]["y"]))
    # todo lo que no es un trasto suelto cuenta como importante (PNJ y props de misión)
    catalogo_npc = set()
    for f in (ROOT / "assets/objects").glob("*.json"):
        try:
            d = json.loads(f.read_text(encoding="utf-8"))
        except ValueError:
            continue
        for e in d.get("objects", []) if isinstance(d, dict) else []:
            if isinstance(e, dict) and e.get("category") in ("npc", "enemy", "pickup") or (
                    isinstance(e, dict) and e.get("interactable")):
                catalogo_npc.add(e["id"])
    for o in nivel["objects"]:
        if not o.get(MARCA) and o["objectId"] in catalogo_npc:
            celdas.add((o["position"]["x"], o["position"]["y"]))
    cines = json.loads((ROOT / "assets/cinematicas/cinematicas.json").read_text(encoding="utf-8"))["cinematicas"]
    ruta = f"assets/levels/{nombre}.json"
    for c in cines:
        if ruta not in json.dumps(c):
            continue
        for p in c.get("pasos", []):
            for k in ("a", "en"):
                if isinstance(p.get(k), list) and len(p[k]) == 2:
                    celdas.add(tuple(p[k]))
            for q in p.get("por", []) or []:
                celdas.add(tuple(q))
        z = c.get("zona")
        if z and len(z) == 4:
            for y in range(z[1], z[3] + 1):
                for x in range(z[0], z[2] + 1):
                    celdas.add((x, y))
    return celdas


def viste(nombre, semilla=22, simular=False):
    trastos, densidad = MAPAS[nombre]
    ruta = ROOT / f"assets/levels/{nombre}.json"
    texto = ruta.read_text(encoding="utf-8")
    nivel = json.loads(texto)
    nivel["objects"] = [o for o in nivel["objects"] if not o.get(MARCA)]
    tmx = (ROOT / nivel["map"]).read_text(encoding="utf-8")
    ancho = int(re.search(r'<layer[^>]*width="(\d+)"', tmx).group(1))
    datos = re.search(r'<data encoding="csv">(.*?)</data>', tmx, re.S).group(1)
    gids = [int(v) & 0x1FFFFFFF for v in datos.replace("\n", "").split(",") if v.strip()]
    alto = len(gids) // ancho
    tapa = colisiones(tmx)
    libre = {(x, y) for y in range(alto) for x in range(ancho) if gids[y * ancho + x] not in tapa}
    ocupado = {(o["position"]["x"], o["position"]["y"]) for o in nivel["objects"]}
    imp = importantes(nivel, nombre)
    vetadas = {(x + dx, y + dy) for x, y in imp for dx in range(-2, 3) for dy in range(-2, 3)}
    pisa = libre - ocupado  # para la conectividad: lo que ya hay se toma como que tapa

    def alcanzables(p):
        claves = [c for c in imp if c in libre]
        if not claves:
            return True
        inicio = next((c for c in claves if c in p), None)
        if inicio is None:
            return True
        visto, cola = {inicio}, deque([inicio])
        while cola:
            x, y = cola.popleft()
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                n = (x + dx, y + dy)
                if n not in visto and (n in p or n in imp):
                    visto.add(n)
                    cola.append(n)
        return all(c in visto for c in claves if c in libre)

    def pared(c):
        x, y = c
        tapadas = [(x + dx, y + dy) not in libre for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))]
        return sum(tapadas) >= 2 or (sum(tapadas) == 1 and any(
            (x + dx, y + dy) not in libre for dx, dy in ((1, 1), (-1, 1), (1, -1), (-1, -1))))

    # Solo en el suelo que se alcanza desde la entrada (los huecos sueltos del TMX no: conectividad.py
    # exige que todo objeto esté en suelo alcanzable).
    s = (nivel["playerStart"]["x"], nivel["playerStart"]["y"])
    region, cola = {s}, deque([s])
    while cola:
        x, y = cola.popleft()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (x + dx, y + dy)
            if n in libre and n not in region:
                region.add(n)
                cola.append(n)

    rnd = random.Random(f"{nombre}:{semilla}")
    candidatas = sorted(c for c in pisa if c in region and c not in vetadas and pared(c))
    rnd.shuffle(candidatas)
    objetivo = max(0, len(libre) * densidad // 100 - 0)
    puestos = []
    base_ok = alcanzables(pisa)
    for c in candidatas:
        if len(puestos) >= objetivo:
            break
        if any(abs(c[0] - p[0]) + abs(c[1] - p[1]) < 2 for p, _ in puestos):
            continue  # sin amontonar
        p2 = pisa - {c}
        if base_ok and not alcanzables(p2):
            continue
        pisa = p2
        puestos.append((c, rnd.choice(trastos)))
    for (x, y), oid in puestos:
        nivel["objects"].append({"objectId": oid, "position": {"x": x, "y": y}, MARCA: True})
    print(f"{nombre}: {len(libre)} casillas que se pisan, {len(nivel['objects']) - len(puestos)} objetos de antes, "
          f"{len(puestos)} trastos nuevos")
    if not simular:
        ind = len(re.match(r"\{\n( +)\S", texto).group(1))
        ruta.write_text(json.dumps(nivel, ensure_ascii=False, indent=ind) + "\n", encoding="utf-8")


def main(argv):
    a = argparse.ArgumentParser()
    a.add_argument("niveles", nargs="*")
    a.add_argument("--simular", action="store_true")
    a = a.parse_args(argv)
    for n in a.niveles or list(MAPAS):
        viste(n, simular=a.simular)


if __name__ == "__main__":
    main(sys.argv[1:])
