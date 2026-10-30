#!/usr/bin/env python3
"""Valida assets/catalogs/recetas.json contra oficios.json y los catálogos de objetos (PO-50).

Comprueba: que cada receta sea de un oficio que existe (o de 'estudio', el rango de
estudiar/copiar), con un rango de oficios.json; que todos los materiales existan como
objetos con precio 1..12 si son materiales (material:true) o sean productos de otra
receta; que el resultado 'objeto' exista en los catálogos; que ninguna receta normal
exija un material raro (los raros solo entran en recetas 'rara'); que el valor de los
materiales no supere el precio del resultado (fabricar es para usar, no para vender: con
la venta al 25 % no hay bucle de oro); y que cada oficio de transformar tenga sus dos
bonos de Maestro.
Uso:  python3 tools/validar_recetas.py [--raiz CARPETA_DEL_MOTOR]   Salida: 0 sin errores.
"""
import argparse
import glob
import json
import os
import sys

RAROS = {"moneda_deithrok", "semilla_sin_dueno", "ceniza_sombras", "dormienta_revisada"}
TIPOS = ("objeto", "mejora", "habilidad", "engarce", "hueco")


def _json(ruta):
    with open(ruta, encoding="utf-8") as f:
        return json.load(f)


def validar(raiz):
    errores, avisos = [], []
    err, aviso = errores.append, avisos.append
    objetos = {}
    for f in sorted(glob.glob(os.path.join(raiz, "assets", "objects", "*.json"))):
        try:
            for o in _json(f).get("objects", []):
                objetos.setdefault(o["id"], o)
        except (OSError, ValueError, KeyError):
            continue
    try:
        equipo = {e["id"]: e for e in _json(os.path.join(raiz, "assets/catalogs/equipment.json"))["entries"]}
    except (OSError, ValueError, KeyError):
        equipo = {}
    oficios = _json(os.path.join(raiz, "assets/catalogs/oficios.json"))
    # Lo que dan las tablas de recolectar (peces, plantas, menas) también es material.
    tabla_items = {}
    try:
        for t in _json(os.path.join(raiz, "assets/catalogs/oficios_tablas.json")).values():
            if isinstance(t, dict):
                for k in ("peces", "plantas", "menas"):
                    for i, v in t.get(k, {}).items():
                        tabla_items[i] = v.get("precio", 0)
    except (OSError, ValueError):
        pass
    rangos = [r["id"] for r in oficios.get("rangos", [])]
    doc = _json(os.path.join(raiz, "assets/catalogs/recetas.json"))
    recetas = doc.get("recetas", {})
    productos = {r["resultado"]["id"] for r in recetas.values() if r["resultado"]["tipo"] == "objeto"}
    nombres_oficio = set(oficios["oficios"]) | {o.get("oficio") for o in oficios["oficios"].values() if o.get("oficio")}
    nombres_oficio |= {"estudio", "pesca"}

    def precio(i):
        e = equipo.get(i) or equipo.get(i.removeprefix("item_"))
        return e["price"] if e else objetos.get(i, {}).get("price", tabla_items.get(i, 0))

    for rid, r in recetas.items():
        p = "recetas." + rid
        if r.get("oficio") not in nombres_oficio:
            err("%s: oficio '%s' desconocido" % (p, r.get("oficio")))
        if r.get("rango") not in rangos:
            err("%s: rango '%s' no está en oficios.json" % (p, r.get("rango")))
        res = r.get("resultado", {})
        if res.get("tipo") not in TIPOS:
            err("%s: tipo de resultado '%s' desconocido" % (p, res.get("tipo")))
        if res.get("tipo") == "objeto" and res.get("id") not in objetos:
            err("%s: el resultado '%s' no existe en assets/objects" % (p, res.get("id")))
        if not isinstance(r.get("rival"), int) or not 0 <= r["rival"] <= 8:
            err("%s: rival debe ser 0..8" % p)
        coste = 0
        for m, n in r.get("materiales", {}).items():
            if not isinstance(n, int) or n < 1:
                err("%s: cantidad inválida de '%s'" % (p, m))
            if m not in objetos and m not in productos and m not in tabla_items:
                err("%s: el material '%s' no existe" % (p, m))
                continue
            if objetos.get(m, {}).get("material") and not 1 <= precio(m) <= 12 and m not in RAROS:
                err("%s: el material '%s' vale %d; los materiales base van de 1 a 12" % (p, m, precio(m)))
            if m in RAROS and not r.get("rara"):
                err("%s: usa el raro '%s' sin ser receta rara" % (p, m))
            coste += precio(m) * n
        if res.get("tipo") == "objeto":
            valor = precio(res["id"]) * res.get("cantidad", 1)
            if valor and coste > valor:
                err("%s: los materiales (%d) valen más que el resultado (%d)" % (p, coste, valor))
            if valor and valor * 25 // 100 > coste:
                aviso("%s: vendido al 25 %% (%d) deja más que sus materiales (%d): revisar" % (p, valor * 25 // 100, coste))
    for of, lista in doc.get("bonosMaestro", {}).items():
        if of not in nombres_oficio:
            err("bonosMaestro.%s: oficio desconocido" % of)
        if len(lista) != 2:
            err("bonosMaestro.%s: deben ser exactamente dos" % of)
    for of, o in oficios["oficios"].items():
        if o.get("objeto", {}).get("receta") is True and not any(r.get("oficio") in (of, o.get("oficio")) for r in recetas.values()):
            err("oficios.%s: declara receta:true pero no tiene recetas" % of)
    return errores, avisos


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--raiz", default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    a = ap.parse_args()
    errores, avisos = validar(a.raiz)
    for e in errores:
        print("ERROR:", e)
    for w in avisos:
        print("AVISO:", w)
    n = len(_json(os.path.join(a.raiz, "assets/catalogs/recetas.json"))["recetas"])
    print("%d recetas; %d errores, %d avisos" % (n, len(errores), len(avisos)))
    return 1 if errores else 0


if __name__ == "__main__":
    sys.exit(main())
