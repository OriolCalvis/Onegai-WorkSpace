#!/usr/bin/env python3
"""Comprueba que Egaroth sea UN mundo y no varios que no se hablan.

    python3 tools/validar_mundo_unido.py

QUE COMPRUEBA

  1. Desde el mapamundi se llega a TODO: cada masa con mapa, cada ciudad,
     cada interior. Recorriendo puertas de verdad, no mirando nombres.
  2. De todo se puede VOLVER al mapamundi. Una puerta sin retorno deja al
     jugador encerrado y carga sin dar ningun error.
  3. Toda puerta aterriza en una celda transitable, leyendo la colision
     del propio TMX (no de un set escrito a mano: eso ya nos mordio).
  4. Ningun nivel de mundo se queda fuera del grafo.

POR QUE HACIA FALTA. Antes de unir el mapamundi habia ocho mapas de masa
de tierra y ninguno llevaba a otro: ocho islas cerradas. Y landmass_1
tenia 46 ciudades dibujadas y CERO puertas -- 46 sitios que se veian y no
se podian pisar. Nada de eso daba un error: daba un mundo que carga
perfectamente y no se puede recorrer.
"""
import json
import os
import re
import sys
from collections import deque

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
NIVELES = os.path.join(RAIZ, "assets", "levels")
RAIZ_MUNDO = "assets/levels/mapamundi.json"

# Niveles que NO forman parte del mundo a proposito: fixtures de prueba y
# prototipos sueltos. Se listan uno a uno; "lo que no reconozco lo ignoro"
# es justo como se cuela un continente fuera del mapa.
EXCLUIDOS = {
    "ejemplo_nivel.json",       # fixture de demo_level_loader
    "editor_level.json",        # lo que escribe el editor sin proyecto
    "test_level.json",          # fixture
    "mazmorra_64x64.json",      # fixture de rendimiento
    # El Vacio NO esta en el mapa a proposito: es donde cae el jugador en
    # el Cap. 1 del canon, antes de llegar a Egaroth. Un sitio al que se
    # llega cayendo no tiene puerta desde el mundo.
    "interior_vacio.json",
}


def carga_nivel(rel):
    p = os.path.join(RAIZ, rel)
    if not os.path.exists(p):
        return None
    with open(p, encoding="utf-8") as f:
        return json.load(f)


def rejilla_de(nivel):
    """(ancho, alto, celdas, gids_que_bloquean) del TMX del nivel."""
    p = os.path.join(RAIZ, nivel["map"])
    if not os.path.exists(p):
        return None
    t = re.sub(r"<!--.*?-->", "", open(p, encoding="utf-8").read(), flags=re.S)
    W = int(re.search(r'<map[^>]*?\swidth="(\d+)"', t).group(1))
    H = int(re.search(r'<map[^>]*?\sheight="(\d+)"', t).group(1))
    colision = {int(m) + 1 for m in re.findall(
        r'<tile id="(\d+)">\s*<properties>\s*<property name="collision"[^/]*/>', t)}
    g = [int(v) for v in re.search(r'<data encoding="csv">(.*?)</data>', t, re.S)
         .group(1).replace("\n", "").split(",") if v.strip()]
    return W, H, g, colision


def main():
    fallos = []
    visitados = set()
    cola = deque([RAIZ_MUNDO])
    puertas = 0

    if carga_nivel(RAIZ_MUNDO) is None:
        sys.exit("no existe " + RAIZ_MUNDO + " -- ejecuta antes tools/gen_mapamundi.py")

    while cola:
        rel = cola.popleft()
        if rel in visitados:
            continue
        visitados.add(rel)
        nivel = carga_nivel(rel)
        if nivel is None:
            fallos.append(f"nivel inexistente: {rel}")
            continue
        rej = rejilla_de(nivel)
        for o in nivel.get("objects", []):
            destino = o.get("targetLevel")
            if not destino:
                continue
            puertas += 1
            dest = carga_nivel(destino)
            if dest is None:
                fallos.append(f"{rel}: '{o['objectId']}' lleva a un nivel que no existe ({destino})")
                continue
            # Donde aterriza tiene que ser transitable.
            drej = rejilla_de(dest)
            tp = o.get("targetPosition") or dest["playerStart"]
            if drej is not None:
                W, H, g, col = drej
                if not (0 <= tp["x"] < W and 0 <= tp["y"] < H):
                    fallos.append(f"{rel}: '{o['objectId']}' aterriza fuera de {destino}")
                elif g[tp["y"] * W + tp["x"]] in col:
                    fallos.append(f"{rel}: '{o['objectId']}' aterriza en celda bloqueada de {destino}")
            cola.append(destino)

    # --- Se puede volver al mapamundi desde todas partes? ---
    # Se recorre al reves: quien alcanza el mapamundi en un paso, quien
    # alcanza a esos, etc.
    entra_a = {}
    for rel in visitados:
        nivel = carga_nivel(rel)
        if nivel is None:
            continue
        for o in nivel.get("objects", []):
            if o.get("targetLevel"):
                entra_a.setdefault(o["targetLevel"], set()).add(rel)
    vuelven = {RAIZ_MUNDO}
    cola = deque([RAIZ_MUNDO])
    while cola:
        n = cola.popleft()
        for origen in entra_a.get(n, ()):
            if origen not in vuelven:
                vuelven.add(origen)
                cola.append(origen)
    encerrados = sorted(visitados - vuelven)
    for e in encerrados:
        fallos.append(f"sin retorno al mapamundi: {e}")

    # --- Niveles de mundo que se quedaron fuera del grafo ---
    huerfanos = []
    for f in sorted(os.listdir(NIVELES)):
        if not f.endswith(".json"):
            continue
        rel = "assets/levels/" + f
        if rel in visitados:
            continue
        # TODOS los niveles, no solo los prefijos que uno espera. La
        # primera version miraba mundi_/ciudad_en_/interior_en_ y daba el
        # mundo por unido... con Boundington entero fuera, porque sus
        # niveles se llaman ciudad_centro, interior_taberna y plaza_*. Es
        # el mismo error que ya cazamos en validar_enlaces: un validador
        # que solo mira los nombres que espera confirma su propia hipotesis.
        if f not in EXCLUIDOS:
            huerfanos.append(f)

    print("Egaroth como un solo mundo\n")
    print(f"  niveles alcanzables desde el mapamundi: {len(visitados)}")
    print(f"  puertas recorridas:                     {puertas}")
    if huerfanos:
        print(f"\n  FUERA DEL MUNDO ({len(huerfanos)}): no se llega desde el mapamundi")
        for h in huerfanos[:12]:
            print(f"      {h}")
        if len(huerfanos) > 12:
            print(f"      ...y {len(huerfanos) - 12} mas")
    if fallos:
        print(f"\n  {len(fallos)} PROBLEMA(S):")
        for f in fallos[:15]:
            print(f"      {f}")
        if len(fallos) > 15:
            print(f"      ...y {len(fallos) - 15} mas")
    if not fallos and not huerfanos:
        print("\n  todo se alcanza y de todo se vuelve")
        return 0
    return 1


if __name__ == "__main__":
    sys.exit(main())
