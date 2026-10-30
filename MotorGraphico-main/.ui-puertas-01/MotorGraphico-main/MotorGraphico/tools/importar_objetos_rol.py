#!/usr/bin/env python3
"""INSTRUMENTOS-01 — Escribe los instrumentos y los objetos de rol (tools/objetos_rol.py) en los catalogos.

- assets/catalogs/equipment.json: cada pieza (upsert por id);
- assets/objects/libreria_armas.json y libreria_completa.json: su puente item_<id> (el editor);
- assets/objects/ciudad_objetos.json: el objeto con el mismo id que la pieza (como «cerveza» o
  «espada_corta»: lo que venden las tiendas de Boundington, que no cargan las librerias) y las ganzuas;
- la tienda de cada PNJ de Boundington: lo suyo al final de su lista (sin repetir).

Se puede volver a ejecutar: solo cambia lo que haya cambiado en objetos_rol.py.

    python3 tools/importar_objetos_rol.py [--ver]
"""
import argparse
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(RAIZ / "tools"))

import importar_clases_sp as imp_sp  # noqa: E402
import objetos_rol  # noqa: E402

EQUIPO = RAIZ / "assets/catalogs/equipment.json"
LIBRERIAS = [RAIZ / "assets/objects/libreria_armas.json", RAIZ / "assets/objects/libreria_completa.json"]
CIUDAD = RAIZ / "assets/objects/ciudad_objetos.json"


def pieza(o):
    """La pieza para equipment.json (sin lo que es solo de aqui, como la tienda)."""
    return {k: v for k, v in o.items() if not k.startswith("_")}


def objeto_de_tienda(ident, nombre, precio):
    """El objeto del catalogo de objetos que se compra (sin efecto al usarlo: se equipa o se lleva)."""
    return {"id": ident, "name": nombre, "category": "pickup", "spriteId": 16, "interactable": True,
            "price": precio, "pickup": {"effect": "none", "power": 0}}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--ver", action="store_true", help="solo dice lo que haria")
    args = ap.parse_args()
    objetos_rol.revisa()

    equipo = imp_sp.cargar(EQUIPO)
    librerias = {r: imp_sp.cargar(r) for r in LIBRERIAS}
    ciudad = imp_sp.cargar(CIUDAD)
    pnj = {o["id"]: o for o in ciudad["objects"] if o.get("shop")}

    resumen = []
    venta = {}
    for o in objetos_rol.TODOS:
        e = pieza(o)
        imp_sp.upsert(equipo["entries"], e)
        for r in LIBRERIAS:
            imp_sp.upsert(librerias[r]["objects"], imp_sp.puente(e))
        imp_sp.upsert(ciudad["objects"], objeto_de_tienda(e["id"], e["name"], e["price"]))
        venta.setdefault(o["_tienda"], []).append(e["id"])
    for m in objetos_rol.MOCHILA:
        imp_sp.upsert(ciudad["objects"], dict(objeto_de_tienda(m["id"], m["name"], m["price"]),
                                              description=m["description"]))
        venta.setdefault(m["tienda"], []).append(m["id"])
    for tendero, ids in venta.items():
        if tendero not in pnj:
            raise SystemExit(f"no hay tienda «{tendero}» en {CIUDAD.name}")
        lista = pnj[tendero]["shop"]["items"]
        nuevos = [i for i in ids if i not in lista]
        lista.extend(nuevos)
        resumen.append(f"{tendero}: {len(lista)} articulos ({len(nuevos)} nuevos)")

    resumen.insert(0, f"{len(objetos_rol.TODOS)} piezas ({len(objetos_rol.INSTRUMENTOS)} instrumentos) y "
                      f"{len(objetos_rol.MOCHILA)} de mochila")
    if not args.ver:
        imp_sp.guardar(EQUIPO, equipo)
        for r in LIBRERIAS:
            imp_sp.guardar(r, librerias[r])
        imp_sp.guardar(CIUDAD, ciudad)
    print("\n".join(resumen))


if __name__ == "__main__":
    main()
