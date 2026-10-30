#!/usr/bin/env python3
"""MORAL-01 — La moral, noche a noche, con los seis números del juego.

Lee assets/rules/moral.json (los números), assets/rules/actos.json (los actos)
y tools/moral/rutas_boundington.json (cinco maneras de jugar Boundington), y
cuenta lo mismo que Morality::asienta (src/Game/Morality.cpp): los menores y
los medios del día con la repetición y el tope de la noche; después, cada mayor
por orden; todos con la resistencia.

Para ver qué pasa si cambias un número: cambia moral.json (o pasa --peso-mayor,
--tope...) y vuelve a lanzarlo.

Uso: python3 tools/simular_moral.py [--peso-menor N] [--peso-medio N]
         [--peso-mayor N] [--tope N] [--resistencia N] [--escribir-espera]
"""
import argparse
import json
import os

RAIZ = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
GRAV = {"menor": 0, "medio": 1, "mayor": 2}


def tramo(v):
    v = round(v)
    return ("Villano" if v <= -75 else "Desgraciado" if v <= -60 else "Canalla" if v <= -20 else
            "Neutral" if v < 20 else "Respetado" if v < 60 else "Salvador" if v < 75 else "Héroe")


def aplica(m, v, p):
    if v == 0:
        return m
    if m == 0 or (v > 0) == (m > 0):
        v *= max(0.0, 1 - abs(m) / p["resistencia"])
    return max(-100.0, min(100.0, m + v))


def asienta(m, actos, catalogo, p):
    vistos, cotidiano, mayores = {}, 0.0, []
    for id_ in actos:
        a = catalogo[id_]
        if a.get("signo", 0) == 0:
            continue
        peso = p["peso"][GRAV[a["gravedad"]]] * (1 if a["signo"] > 0 else -1)
        if a["gravedad"] == "mayor":
            mayores.append(peso)
            continue
        clave = a.get("tipo") or a["id"]
        k = vistos.get(clave, 0)
        vistos[clave] = k + 1
        rep = p["repeticion"][min(k, len(p["repeticion"]) - 1)]
        cotidiano += peso * rep
    m = aplica(m, max(-p["tope"], min(p["tope"], cotidiano)), p)
    for v in mayores:
        m = aplica(m, v, p)
    return m


def main():
    ap = argparse.ArgumentParser()
    for nombre in ("peso-menor", "peso-medio", "peso-mayor", "tope", "resistencia"):
        ap.add_argument("--" + nombre, type=float)
    ap.add_argument("--escribir-espera", action="store_true", help="guarda los valores en el fichero de rutas")
    args = ap.parse_args()
    moral = json.load(open(os.path.join(RAIZ, "assets/rules/moral.json"), encoding="utf-8"))
    p = {"peso": [moral["peso"]["menor"], moral["peso"]["medio"], moral["peso"]["mayor"]],
         "repeticion": moral["repeticion"], "tope": moral["topePorNoche"], "resistencia": moral["resistencia"]}
    for i, n in enumerate(("peso_menor", "peso_medio", "peso_mayor")):
        if getattr(args, n) is not None:
            p["peso"][i] = getattr(args, n)
    if args.tope is not None:
        p["tope"] = args.tope
    if args.resistencia is not None:
        p["resistencia"] = args.resistencia
    catalogo = {a["id"]: a for a in json.load(open(os.path.join(RAIZ, "assets/rules/actos.json"), encoding="utf-8"))["actos"]}
    ruta_rutas = os.path.join(RAIZ, "tools/moral/rutas_boundington.json")
    rutas = json.load(open(ruta_rutas, encoding="utf-8"))
    print(f"Pesos {p['peso']}, repetición {p['repeticion']}, tope {p['tope']}, resistencia {p['resistencia']}\n")
    print(f"{'Ruta':<22}{'1.ª noche':>11}{'2.ª noche':>11}{'La barca':>11}   Tramo")
    for r in rutas["rutas"]:
        m, serie = 0.0, []
        for noche in r["noches"]:
            m = asienta(m, noche, catalogo, p)
            serie.append(round(m, 1))
        r["espera"] = serie
        print(f"{r['nombre']:<22}" + "".join(f"{v:>11}" for v in serie) + f"   {tramo(m)}")
    print("\nDesde 0, cuántos actos hacen falta (todos distintos, uno detrás de otro):")
    for umbral, nombre in ((20, "Respetado o Canalla"), (60, "Salvador o Desgraciado"), (75, "Héroe o Villano")):
        trozos = []
        for g, por_noche in (("mayor", 1), ("medio", 2), ("menor", 3)):
            m, n, noches = 0.0, 0, 0
            falso = {f"x{i}": {"id": f"x{i}", "gravedad": g, "signo": 1} for i in range(por_noche)}
            while m < umbral and noches < 500:
                m = asienta(m, list(falso), falso, p)
                n += por_noche
                noches += 1
            trozos.append(f"{n} " + {"mayor": "mayores", "medio": "medios", "menor": "menores"}[g] + ("" if g == "mayor" else f" ({noches} noches)"))
        print(f"  {nombre:<24}" + ", ".join(trozos))
    if args.escribir_espera:
        json.dump(rutas, open(ruta_rutas, "w", encoding="utf-8"), ensure_ascii=False, indent=2)
        open(ruta_rutas, "a", encoding="utf-8").write("\n")
        print("\nGuardado en", os.path.relpath(ruta_rutas, RAIZ))


if __name__ == "__main__":
    main()
