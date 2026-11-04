#!/usr/bin/env python3
"""Oriol (18/10): «falten dos mapes mes del Ocaso: el seguent barri, les Afueras, i el mapa d'aconseguir
escapar». Dos niveles mas de la noche del dia 3, en la misma cadena que ciudad_centro_ocaso:

  afueras_boundington_ocaso   las Afueras esa noche: el mismo terreno que afueras_boundington (el TMX se
                              copia; Oriol lo quema y lo cierra desde el Excel), sin botin ni montana, con
                              DOS pasos: al norte, de vuelta al Casco en llamas; al oeste, la linde por
                              donde se escapa.
  huida_boundington           la huida: los Campos del Otono, 40x40. Se entra por el este desde las
                              Afueras; un camino de tierra cruza los campos hacia el oeste, con la linde de
                              la Barriada (los carros enganchados de b3_huida_gitanos) y, al final,
                              `fin_de_la_huida` (eventId, sin destino: ahi acaba la noche y empieza el
                              epilogo). Lo demas (enemigos, fuego, barricadas) lo pone Oriol en el Excel.

Y en ciudad_centro_ocaso, el paso al sur hacia las Afueras (62..66, 63), donde Oriol llevo el camino a
Egaroth de dia; `huida_a_egaroth` se queda como marcador.

    python3 tools/gen_ocaso_huida.py          # escribe
    python3 tools/gen_ocaso_huida.py --seco   # solo informa
Reejecutable: los dos niveles se regeneran enteros (lo que Oriol les ponga por Excel se perderia: por eso solo se
vuelve a pasar si se cambia ESTE fichero), y el paso del Casco se reemplaza por su id.
"""
import json
import os
import re
import shutil
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_niveles_av19 as G
from gen_niveles_av19 import Nivel, _T

BASE = G.BASE
SECO = "--seco" in sys.argv
ROCA = "roca"


# Decorado de Qwen (AV19-10, `afueras_ocaso.json` del 20/10, recuperado en F-REST.1): props que la
# version del Excel ponia en las Afueras esa noche y que el nivel de caa no tenia. Se dejan fuera el
# cofre (16,2) y los juncos (24,9), que hoy ocupan botin_afueras_cofre_40 / botin_afueras_escondite,
# y los tres enemigos perdido_* con patrulla (no son decorado: decision de Oriol si entran).
DECORADO_QWEN_OCASO = [
    ("molino", 6, 5), ("baul", 7, 23), ("barriles_pila", 19, 25), ("fuego_blanco", 20, 25),
    ("carro_o1", 21, 25), ("carro", 30, 16), ("fuego", 31, 16), ("cajas_almacen", 32, 16),
    ("runa", 31, 18), ("runa_b", 31, 19), ("columna_rota", 36, 18), ("arbol_grande", 33, 26),
    ("abrevadero", 31, 28),
]


def afueras_ocaso():
    dia = json.load(open(BASE + "assets/levels/afueras_boundington.json", encoding="utf-8"))
    n = Nivel("afueras_boundington_ocaso", "Las Afueras de Boundington, al ocaso", 40, 32,
              suelo=_T["pradera"], colision=(ROCA,))
    n.copiar_tmx = "afueras_boundington"
    n.start = (18, 2)
    # El terreno y los objetos de dia, menos el botin (de noche no se saquea el huerto) y menos los pasos,
    # que aqui son otros.
    for o in dia["objects"]:
        oid = o["objectId"]
        if oid.startswith("botin_") or o.get("targetLevel"):
            continue
        n.objetos.append(json.loads(json.dumps(o)))
    for oid, x, y in DECORADO_QWEN_OCASO:
        n.objetos.append({"objectId": oid, "position": {"x": x, "y": y}})
    # Sin la bajada a la montana: esa noche no se sube (si Oriol la quiere, la pone en el Excel).
    # Al norte, de vuelta al Casco en llamas: se llega a (64,61), el hueco que Oriol dejo entre las barricadas
    # de la fila 62, justo encima del paso del sur.
    for x in range(16, 21):
        n.salida("paso_afueras_a_ocaso", x, 0, "ciudad_centro_ocaso", (64, 61))
    # Al oeste, la linde: la huida.
    for y in (21, 22, 23):
        n.salida("paso_afueras_a_huida", 1, y, "huida_boundington", (38, y))
    return n


def huida():
    n = Nivel("huida_boundington", "La huida: los Campos del Otono", 40, 40, suelo=_T["hierba_seca"],
              colision=(ROCA,))
    n.start = (37, 22)
    # Los campos: bancales de hierba seca con lindes de roca (setos) que obligan a seguir el camino.
    n.rect(0, 0, 40, 1, _T[ROCA]); n.rect(0, 39, 40, 1, _T[ROCA]); n.rect(0, 0, 1, 40, _T[ROCA])
    n.rect(39, 0, 1, 18, _T[ROCA]); n.rect(39, 27, 1, 13, _T[ROCA])          # el este, abierto solo en 18..26
    # El camino de tierra, de la linde del este al final del oeste, con dos quiebros.
    n.rect(30, 20, 9, 5, _T["sendero_tierra"])
    n.rect(22, 12, 10, 13, _T["sendero_tierra"])
    n.rect(10, 12, 13, 5, _T["sendero_tierra"])
    n.rect(2, 10, 10, 7, _T["sendero_tierra"])
    # Setos que cierran los atajos.
    for (x, y, w, h) in [(6, 20, 20, 1), (6, 20, 1, 12), (26, 27, 1, 10), (10, 5, 1, 7), (16, 2, 8, 1), (30, 5, 1, 12)]:
        n.rect(x, y, w, h, _T[ROCA])
    n.rect(12, 30, 10, 4, _T["pradera"])                                  # un prado donde esperan los carros
    # La vuelta a las Afueras, por donde se entra; y el fin de la huida (eventId, sin destino: se queda en el
    # mapa como la huida_a_egaroth del Casco).
    for y in (21, 22, 23):
        n.salida("paso_huida_a_afueras", 39, y, "afueras_boundington_ocaso", (2, y))
    n.obj("fin_de_la_huida", 1, 13)
    # Lo que se ve: arboles tortos, matas, la linde de la Barriada con sus carros enganchados, una hoguera.
    for (oid, x, y) in [("arbol_seco", 34, 18), ("arbol_seco", 33, 27), ("arbol_seco", 20, 9), ("arbol_roble", 8, 8),
                        ("arbol_roble", 4, 19), ("mata", 27, 11), ("mata", 14, 18), ("mata", 3, 23),
                        ("carromato_lona", 13, 31), ("carro", 16, 31), ("carromato", 19, 31), ("hoguera", 16, 33),
                        ("fardos", 14, 33), ("barril", 20, 33), ("columna_rota", 2, 9)]:
        n.obj(oid, x, y)
    return n


def paso_del_casco():
    """ciudad_centro_ocaso: el paso al sur (30..34, 63) hacia las Afueras."""
    ruta = BASE + "assets/levels/ciudad_centro_ocaso.json"
    texto = open(ruta, encoding="utf-8").read()
    lvl = json.loads(texto)
    ind = len(re.match(r"\{\n( +)\S", texto).group(1))
    lvl["objects"] = [o for o in lvl["objects"] if o["objectId"] != "paso_ocaso_a_afueras"]
    for x in range(62, 67):  # el Ocaso mide 96 desde ampliar_ocaso.py: el Casco empieza en la columna 32
        lvl["objects"].append({"objectId": "paso_ocaso_a_afueras", "position": {"x": x, "y": 63},
                               "targetLevel": "assets/levels/afueras_boundington_ocaso.json",
                               "targetPosition": {"x": x - 46, "y": 1}})
    # la fila 63 del ocaso es muralla: se abre como abrio Oriol la de la ciudad de dia
    tmx = BASE + lvl["map"]
    t = open(tmx, encoding="utf-8").read()
    m = re.search(r'(<data encoding="csv">)(.*?)(</data>)', t, re.S)
    g = [v.strip() for v in m.group(2).replace("\n", "").split(",") if v.strip()]
    W = int(re.search(r'<map[^>]*?\swidth="(\d+)"', t).group(1))
    for x in range(62, 67):
        g[63 * W + x] = str(_T["sendero_tierra"])
    filas = [",".join(g[i:i + W]) for i in range(0, len(g), W)]
    if not SECO:
        open(tmx, "w", encoding="utf-8").write(t[:m.start(2)] + "\n" + ",\n".join(filas) + "\n" + t[m.end(2):])
        open(ruta, "w", encoding="utf-8").write(json.dumps(lvl, indent=ind, ensure_ascii=False) + "\n")
    print("  ciudad_centro_ocaso: paso al sur (30..34, 63) -> afueras_boundington_ocaso")


def objetos_nuevos():
    """Los ids nuevos, en boundington_av19_objetos.json (como los demas pasos del AV19)."""
    ruta = BASE + "assets/objects/boundington_av19_objetos.json"
    texto = open(ruta, encoding="utf-8").read()
    cat = json.loads(texto)
    ind = len(re.match(r"\{\n( +)\S", texto).group(1))
    hay = {o["id"] for o in cat["objects"]}
    nuevos = [
        {"id": "paso_ocaso_a_afueras", "name": "Hacia las Afueras (la noche del dia 3)", "category": "prop",
         "spriteId": 23, "blocksMovement": False, "interactable": True},
        {"id": "paso_afueras_a_ocaso", "name": "De vuelta al Casco en llamas", "category": "prop",
         "spriteId": 23, "blocksMovement": False, "interactable": True},
        {"id": "paso_afueras_a_huida", "name": "La linde: hacia los Campos del Otono", "category": "prop",
         "spriteId": 23, "blocksMovement": False, "interactable": True},
        {"id": "paso_huida_a_afueras", "name": "De vuelta a las Afueras", "category": "prop",
         "spriteId": 23, "blocksMovement": False, "interactable": True},
        {"id": "fin_de_la_huida", "name": "El fin de la huida", "category": "prop",
         "spriteId": 23, "blocksMovement": False, "interactable": True, "eventId": "fin_de_la_huida"},
    ]
    for o in nuevos:
        if o["id"] not in hay:
            cat["objects"].append(o)
    if not SECO:
        open(ruta, "w", encoding="utf-8").write(json.dumps(cat, indent=ind, ensure_ascii=False) + "\n")


def main():
    objetos_nuevos()
    # la huida comprueba su vuelta a las afueras y las afueras su paso a la huida: dos pasadas
    if not os.path.exists(BASE + "assets/levels/afueras_boundington_ocaso.json"):
        afueras_ocaso_sin = afueras_ocaso
    h = huida(); h.escribir()
    a = afueras_ocaso(); a.escribir()
    paso_del_casco()
    # manifiesto y lugares
    ruta = BASE + "assets/proyectos/boundington.json"
    texto = open(ruta, encoding="utf-8").read()
    man = json.loads(texto)
    ind = len(re.match(r"\{\n( +)\S", texto).group(1))
    for lv in ("afueras_boundington_ocaso.json", "huida_boundington.json"):
        if lv not in man["levels"]:
            man["levels"].insert(man["levels"].index("ciudad_centro_ocaso.json") + 1, lv)
    if not SECO:
        open(ruta, "w", encoding="utf-8").write(json.dumps(man, indent=ind, ensure_ascii=False) + "\n")
    ruta = BASE + "assets/catalogs/lugares.json"
    texto = open(ruta, encoding="utf-8").read()
    lug = json.loads(texto)
    ind = len(re.match(r"\{\n( +)\S", texto).group(1))
    clave = next(k for k, v in lug.items() if isinstance(v, list) and v and "nivel" in v[0])
    hay = {e["nivel"] for e in lug[clave]}
    for lv, nombre, salir in (("afueras_boundington_ocaso", "las Afueras, al ocaso", "el camino"),
                              ("huida_boundington", "los Campos del Otono", "el camino")):
        if f"assets/levels/{lv}.json" not in hay:
            lug[clave].append({"nivel": f"assets/levels/{lv}.json", "nombre": nombre, "salir": salir})
    if not SECO:
        open(ruta, "w", encoding="utf-8").write(json.dumps(lug, indent=ind, ensure_ascii=False) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
