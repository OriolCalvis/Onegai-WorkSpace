#!/usr/bin/env python3
"""ACTUAR-01 — Los sitios para actuar en la calle de Boundington: los escribe en su catalogo y en los niveles.

Oriol (08/10/2026): «podamos hacer una performance en la calle y depende de nuestra competencia musical y
tiradas, atraer a la gente a nuestro espectáculo».

Cada sitio es un objeto de assets/objects/actuaciones.json (uno de los catalogos del proyecto de Boundington) con
su bloque "actuacion" (include/Level/ObjectCatalog.h, PerformanceData):
  - rivalDice: los dados de la indiferencia de los que pasan (cuanto cuesta que se paren);
  - afluencia: cuanta gente pasa (de ahi salen el publico y el oro);
  - lugar: para el aviso («[E] Actuar junto a la fuente») y para el dialogo.
Se dibuja como una caja (assets/textures/sprites.json: el patron «actuacion_»). Lo que pasa al actuar (la tirada
de Interpretacion, lo que suma el instrumento, el publico, el oro y el descanso de cada sitio):
include/Game/Actuacion.h.

Cada sitio va una vez en su nivel, en una celda libre (sin colision ni otro objeto). Se puede volver a ejecutar:
solo cambia lo que haya cambiado aqui (si un sitio se mueve, se mueve en su nivel).

    python3 tools/actuaciones.py [--ver]
"""
import argparse
import json
import re
from pathlib import Path

RAIZ = Path(__file__).resolve().parents[1]
CATALOGO = RAIZ / "assets/objects/actuaciones.json"
PROYECTO = RAIZ / "assets/proyectos/boundington.json"
SPRITES = RAIZ / "assets/textures/sprites.json"
NOMBRE = "Rincón del músico"
PATRON = {"prefijo": "actuacion_", "id": "caja"}

# (id, nivel, (x, y), rivalDice, afluencia, lugar). La plaza de la fuente es la de todos; a la puerta de la opera
# pasa mas gente, pero mas exigente; la del mercado y la taberna, poca y facil.
SITIOS = [
    ("actuacion_fuente", "ciudad_centro", (52, 16), 3, 12, "junto a la fuente"),  # AV16-09b: la fuente esta en (50,16)
    ("actuacion_opera", "ciudad_centro", (25, 12), 4, 16, "a las puertas de la ópera"),
    ("actuacion_mercado", "ciudad_centro", (33, 48), 2, 10, "en la plaza del mercado"),
    ("actuacion_taberna", "interior_taberna", (6, 5), 2, 6, "en la taberna"),
]


def objeto(ident, rival, afluencia, lugar):
    return {"id": ident, "name": NOMBRE, "category": "prop", "spriteId": 16, "interactable": True,
            "actuacion": {"rivalDice": rival, "afluencia": afluencia, "lugar": lugar}}


def colisiones(ruta_tmx):
    """Las celdas con colision (y las vacias) de un mapa: el conjunto de (x, y) donde no se puede estar."""
    tmx = ruta_tmx.read_text(encoding="utf-8")
    primero = int(re.search(r'firstgid="(\d+)"', tmx).group(1))
    con = {int(m.group(1)) for m in re.finditer(r'<tile id="(\d+)">\s*<properties>(.*?)</properties>', tmx, re.S)
           if 'name="collision"' in m.group(2) and 'value="true"' in m.group(2)}
    ancho = int(re.search(r'<map[^>]* width="(\d+)"', tmx).group(1))
    celdas = [int(v) for v in re.search(r'<data encoding="csv">(.*?)</data>', tmx, re.S).group(1)
              .replace("\n", "").split(",") if v.strip()]
    return {(i % ancho, i // ancho) for i, gid in enumerate(celdas) if gid == 0 or (gid - primero) in con}


def coloca(datos, nivel, ident, x, y):
    """Pone el sitio en su celda del nivel ya cargado (o lo mueve alli)."""
    if (x, y) in colisiones(RAIZ / datos["map"]):
        raise SystemExit(f"{ident}: ({x}, {y}) de {nivel} no se pisa")
    otros = [o["objectId"] for o in datos["objects"]
             if (o["position"]["x"], o["position"]["y"]) == (x, y) and o["objectId"] != ident]
    if otros:
        raise SystemExit(f"{ident}: en ({x}, {y}) de {nivel} ya esta {otros[0]}")
    suyos = [o for o in datos["objects"] if o["objectId"] == ident]
    if len(suyos) == 1 and (suyos[0]["position"]["x"], suyos[0]["position"]["y"]) == (x, y):
        return
    datos["objects"] = [o for o in datos["objects"] if o["objectId"] != ident]
    datos["objects"].append({"objectId": ident, "position": {"x": x, "y": y}})


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--ver", action="store_true", help="solo dice lo que haria")
    args = ap.parse_args()
    ids = [s[0] for s in SITIOS]
    if len(set(ids)) != len(ids) or not all(i.startswith(PATRON["prefijo"]) for i in ids):
        raise SystemExit("actuaciones: ids repetidos o sin el prefijo del dibujo")

    escritos = []
    catalogo = {"_fuente": "ACTUAR-01: tools/actuaciones.py (no editar a mano)",
                "objects": [objeto(i, r, a, lugar) for i, _n, _p, r, a, lugar in SITIOS]}
    texto = json.dumps(catalogo, indent=1, ensure_ascii=False) + "\n"
    if not CATALOGO.exists() or CATALOGO.read_text(encoding="utf-8") != texto:
        escritos.append((CATALOGO, texto))

    niveles = {}
    for ident, nivel, (x, y), *_ in SITIOS:
        ruta = RAIZ / f"assets/levels/{nivel}.json"
        datos = niveles.setdefault(ruta, json.loads(ruta.read_text(encoding="utf-8")))
        coloca(datos, nivel, ident, x, y)
    for ruta, datos in niveles.items():
        texto = json.dumps(datos, indent=1, ensure_ascii=False) + "\n"
        if ruta.read_text(encoding="utf-8") != texto:
            escritos.append((ruta, texto))

    proyecto = json.loads(PROYECTO.read_text(encoding="utf-8"))
    if CATALOGO.name not in proyecto["catalogs"]:
        proyecto["catalogs"].append(CATALOGO.name)
        escritos.append((PROYECTO, json.dumps(proyecto, indent=2, ensure_ascii=False) + "\n"))
    sprites = json.loads(SPRITES.read_text(encoding="utf-8"))
    if PATRON not in sprites["patrones"]:
        sprites["patrones"].append(PATRON)
        escritos.append((SPRITES, json.dumps(sprites, indent=2, ensure_ascii=False) + "\n"))

    for ruta, texto in escritos:
        if not args.ver:
            ruta.write_text(texto, encoding="utf-8")
        print(("cambiaria " if args.ver else "escrito ") + str(ruta.relative_to(RAIZ)))
    print(f"{len(SITIOS)} sitios para actuar; {len(escritos)} ficheros {'por cambiar' if args.ver else 'cambiados'}")


if __name__ == "__main__":
    main()
