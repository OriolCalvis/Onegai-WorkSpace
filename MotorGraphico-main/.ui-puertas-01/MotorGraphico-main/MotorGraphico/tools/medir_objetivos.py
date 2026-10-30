#!/usr/bin/env python3
"""MISIONES-01 — Mide si cada objetivo de mision dice adonde ir (`donde`).

El minimapa y el mapa grande (MAPA-01) solo saben guiar al jugador si el
objetivo trae `donde`. Esta herramienta lista los objetivos de las aventuras
(assets/adventures/*.json, clave `objectives`) y de las fichas de mision
(assets/catalogs/quests.json, clave `entries[].objectives`) y dice de cada uno:

  SIN_DESTINO  no tiene `donde`.
  NO_RESUELVE  tiene `donde`, pero no se puede seguir: un nivel que no existe,
               un objectId que no esta en ningun nivel, o ningun sitio al que
               lleven las puertas desde Boundington.
  OK           al menos un destino existe y hay camino de puertas.
  OK_ESCENA    el destino esta en un nivel sin puertas (una escena aparte,
               como el prado del prologo): no hace falta camino.

Como lo lee el juego (src/HUD/Pergamino/MinimapaTinta.cpp + Game::RumboNiveles):
`donde` es una cadena o una lista; una entrada con "/" o acabada en ".json" es
un NIVEL (se compara por nombre de fichero) y el resto es un objectId (se marca
en el nivel donde estes). Para una persona de otro nivel: ["assets/levels/X.json", "id"].

Uso (solo python3, sin compilar):
  python3 tools/medir_objetivos.py                resumen corto
  python3 tools/medir_objetivos.py --detalle      una ficha por objetivo
  python3 tools/medir_objetivos.py --solo-boundington --exigir   (sale con 1 si falla alguno)
"""
import argparse
import json
import os
import sys
from collections import deque

ORIGEN = "ciudad_centro.json"  # desde donde se cuentan las puertas: el Casco Antiguo
# Misiones de prueba del motor: se listan aparte y no cuentan.
PRUEBAS = ("mision_llave", "carta_del_santuario")
MAX_LISTA = 40


def nombre(ruta):
    """Nombre de fichero de un nivel (igual que Game::RumboNiveles::nombre)."""
    return str(ruta).replace("\\", "/").rsplit("/", 1)[-1]


def es_nivel(entrada):
    return "/" in entrada or "\\" in entrada or entrada.endswith(".json")


def lista_donde(valor):
    """`donde` puede ser una cadena, una lista o faltar."""
    if valor is None or valor == "":
        return []
    if isinstance(valor, str):
        return [valor]
    return [str(v) for v in valor if str(v) != ""]


def _json(ruta):
    with open(ruta, encoding="utf-8") as f:
        return json.load(f)


def cargar_niveles(raiz):
    """nombre de fichero -> {"puertas": [nombres], "objetos": {objectId}}"""
    carpeta = os.path.join(raiz, "assets", "levels")
    niveles = {}
    if not os.path.isdir(carpeta):
        return niveles
    for fichero in sorted(os.listdir(carpeta)):
        if not fichero.endswith(".json"):
            continue
        try:
            datos = _json(os.path.join(carpeta, fichero))
        except (OSError, ValueError):
            continue
        puertas, objetos = [], set()
        for o in datos.get("objects", []) if isinstance(datos, dict) else []:
            if o.get("objectId"):
                objetos.add(o["objectId"])
            if o.get("targetLevel"):
                puertas.append(nombre(o["targetLevel"]))
        niveles[fichero] = {"puertas": puertas, "objetos": objetos}
    return niveles


def alcanzables(niveles, origen=ORIGEN):
    """Niveles a los que se llega por puertas desde `origen` (anchura)."""
    vistos = {origen} if origen in niveles else set()
    cola = deque(vistos)
    while cola:
        n = cola.popleft()
        for p in niveles[n]["puertas"]:
            if p in niveles and p not in vistos:
                vistos.add(p)
                cola.append(p)
    return vistos


def aislados(niveles):
    """Niveles sin ninguna puerta, ni de salida ni de entrada: escenas aparte."""
    entran = set()
    for n in niveles.values():
        entran.update(n["puertas"])
    return {k for k, v in niveles.items() if not v["puertas"] and k not in entran}


def es_boundington(mision):
    return mision.startswith("adv_boundington") or mision.startswith("bnd")


def leer_objetivos(raiz):
    """Todos los objetivos: dicts con mision, id, texto, donde, origen, opcional."""
    salida = []
    carpeta = os.path.join(raiz, "assets", "adventures")
    if os.path.isdir(carpeta):
        for fichero in sorted(os.listdir(carpeta)):
            if not fichero.endswith(".json"):
                continue
            try:
                datos = _json(os.path.join(carpeta, fichero))
            except (OSError, ValueError):
                continue
            if not isinstance(datos, dict):
                continue
            for o in datos.get("objectives", []) or []:
                salida.append({"mision": datos.get("id", fichero), "id": o.get("id", ""),
                               "texto": o.get("text", ""), "donde": lista_donde(o.get("donde")),
                               "origen": "assets/adventures/" + fichero, "opcional": False})
    quests = os.path.join(raiz, "assets", "catalogs", "quests.json")
    if os.path.isfile(quests):
        try:
            datos = _json(quests)
        except (OSError, ValueError):
            datos = {}
        for q in datos.get("entries", []) if isinstance(datos, dict) else []:
            for o in q.get("objectives", []) or []:
                salida.append({"mision": q.get("id", ""), "id": o.get("id", ""),
                               "texto": o.get("description", ""), "donde": lista_donde(o.get("donde")),
                               "origen": "assets/catalogs/quests.json", "opcional": bool(o.get("optional"))})
    for o in salida:
        o["prueba"] = o["mision"] in PRUEBAS
        o["boundington"] = es_boundington(o["mision"])
    return salida


def evaluar(obj, niveles, llegan, solos):
    """Pone en `obj` el estado y las notas. Devuelve `obj`."""
    notas = []
    entradas = obj["donde"]
    obj["notas"] = notas
    if not entradas:
        obj["estado"] = "SIN_DESTINO"
        return obj
    nivs = [nombre(e) for e in entradas if es_nivel(e)]
    ids = [e for e in entradas if not es_nivel(e)]
    malo = False
    metas = []  # niveles donde se cumple, existan o no
    propios = set()  # niveles que solo salen porque un objeto esta en ellos
    for n in nivs:
        if n not in niveles:
            notas.append("el nivel %s no existe" % n)
            malo = True
        else:
            metas.append(n)
    for i in ids:
        donde = [k for k, v in niveles.items() if i in v["objetos"]]
        if not donde:
            notas.append("el objeto %s no esta en ningun nivel" % i)
            malo = True
            continue
        if len(nivs) == 1 and len(ids) == 1 and nivs[0] not in donde:
            notas.append("%s esta en %s, no en %s" % (i, donde[0], nivs[0]))  # [nivel, id] mal puesto
        metas.extend(donde)
        propios.update(d for d in donde if d not in nivs)
    if malo:
        obj["estado"] = "NO_RESUELVE"
        return obj
    con_camino = [m for m in dict.fromkeys(metas) if m in llegan]
    escena = [m for m in dict.fromkeys(metas) if m in solos]
    for m in dict.fromkeys(metas):
        # Un objeto repetido en una escena aparte (Venides en un recuerdo) no es un aviso si otro sitio si se alcanza.
        if m not in llegan and m not in solos and not (m in propios and (con_camino or escena)):
            notas.append("sin camino de puertas desde Boundington hasta %s" % m)
    if con_camino:
        obj["estado"] = "OK"
    elif escena:
        obj["estado"] = "OK_ESCENA"
    else:
        obj["estado"] = "NO_RESUELVE"
    return obj


def medir(raiz):
    niveles = cargar_niveles(raiz)
    llegan = alcanzables(niveles)
    solos = aislados(niveles)
    return [evaluar(o, niveles, llegan, solos) for o in leer_objetivos(raiz)]


def contar(objetivos):
    c = {"total": len(objetivos), "SIN_DESTINO": 0, "NO_RESUELVE": 0, "OK": 0, "OK_ESCENA": 0}
    for o in objetivos:
        c[o["estado"]] += 1
    return c


def _lista(titulo, objetivos):
    if not objetivos:
        return []
    ids = ["%s/%s" % (o["mision"], o["id"]) for o in objetivos]
    extra = ["... y %d mas" % (len(ids) - MAX_LISTA)] if len(ids) > MAX_LISTA else []
    return ["  %s:" % titulo] + ["    " + i for i in ids[:MAX_LISTA]] + (["    " + extra[0]] if extra else [])


def resumen(objetivos, solo_boundington=False):
    reales = [o for o in objetivos if not o["prueba"]]
    if solo_boundington:
        reales = [o for o in reales if o["boundington"]]
    lineas = []
    pruebas = [o for o in objetivos if o["prueba"]]
    if pruebas and not solo_boundington:
        lineas.append("De prueba (no cuentan): %d" % len(pruebas))
    for titulo, grupo in (("Boundington", [o for o in reales if o["boundington"]]),
                          ("Fuera de Boundington", [o for o in reales if not o["boundington"]])):
        if solo_boundington and titulo != "Boundington":
            continue
        c = contar(grupo)
        resueltos = c["OK"] + c["OK_ESCENA"]
        lineas.append("%s: %d objetivos | con destino que se sigue: %d | sin destino: %d | no se resuelven: %d"
                      % (titulo, c["total"], resueltos, c["SIN_DESTINO"], c["NO_RESUELVE"]))
        lineas += _lista("sin destino", [o for o in grupo if o["estado"] == "SIN_DESTINO"])
        lineas += _lista("no se resuelven", [o for o in grupo if o["estado"] == "NO_RESUELVE"])
    return lineas


def detalle(objetivos, solo_boundington=False):
    lineas = []
    for o in objetivos:
        if solo_boundington and not o["boundington"]:
            continue
        marca = " (de prueba)" if o["prueba"] else ""
        lineas.append("%s/%s%s%s" % (o["mision"], o["id"], " [opcional]" if o["opcional"] else "", marca))
        lineas.append("    estado: %s" % o["estado"])
        lineas.append("    texto:  %s" % o["texto"])
        lineas.append("    donde:  %s" % (json.dumps(o["donde"], ensure_ascii=False) if o["donde"] else "-"))
        for n in o["notas"]:
            lineas.append("    aviso:  %s" % n)
    return lineas


def principal(argv=None):
    p = argparse.ArgumentParser(description="Mide si cada objetivo de mision dice adonde ir.")
    p.add_argument("--raiz", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."),
                   help="carpeta del proyecto (la que tiene assets/)")
    p.add_argument("--detalle", action="store_true", help="una ficha por objetivo")
    p.add_argument("--solo-boundington", action="store_true", help="solo las misiones de Boundington")
    p.add_argument("--exigir", action="store_true",
                   help="salir con 1 si algun objetivo de Boundington no tiene destino o no se resuelve")
    a = p.parse_args(argv)
    objetivos = medir(os.path.abspath(a.raiz))
    if not objetivos:
        print("No encuentro objetivos en %s (¿es la carpeta del proyecto?)" % a.raiz)
        return 2
    salida = detalle(objetivos, a.solo_boundington) if a.detalle else []
    salida += resumen(objetivos, a.solo_boundington)
    print("\n".join(salida))
    if a.exigir:
        mal = [o for o in objetivos if o["boundington"] and not o["prueba"]
               and o["estado"] in ("SIN_DESTINO", "NO_RESUELVE")]
        return 1 if mal else 0
    return 0


if __name__ == "__main__":
    sys.exit(principal())
