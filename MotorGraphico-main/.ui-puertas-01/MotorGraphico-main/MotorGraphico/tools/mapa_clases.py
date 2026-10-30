#!/usr/bin/env python3
"""Mapa de clases de un ejecutable, desde arriba y medido.

    python3 tools/mapa_clases.py juego
    python3 tools/mapa_clases.py motor_editor
    python3 tools/mapa_clases.py --todos

POR QUE EXISTE. El analisis clase a clase de `analisis_clases/` es del 31/08 y
cubria 101 cabeceras; hoy el juego alcanza 142 y el arbol tiene 213. Un mapa
escrito a mano en un repo donde varias sesiones parten ficheros cada semana
miente en un mes, y ya lo ha hecho: tres paquetes estaban cerrados en falso
(E-06, E-09, E-16) porque la documentacion decia una cosa y el codigo otra.

Este mapa sale del codigo. No opina: mide. Las cuatro preguntas queOriol pidio
se responden asi:

  que hace realmente   -> el inventario de abajo (clases, lineas, quien la alcanza)
  que deberia hacer    -> la columna Paquete: el bloque PAQUETES DEL PLAN que cada
                          cabecera ya lleva, con su criterio de cierre en
                          planificacion/PLA_DEFINITIU.md
  que se lo impide     -> la columna Bloqueos, cruzada con linea_base_core.json
  como optimizarla     -> lineas y miembros: los dos numeros que la guardia
                          vigila porque son los que crecen antes de romperse

Los datos de alcance se leen de un JSON de medir_alcance.py. Si no se pasa
--alcance, se regenera a un temporal: nunca se escribe sobre
planificacion/alcance_real.json, que es la foto congelada de la guardia.
"""

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
import collections

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)

CLASE = re.compile(r"^(?:class|struct)\s+([A-Za-z_]\w*)\b[^;]*$", re.M)
PAQUETE = re.compile(r"^//\s+([A-Z]{1,4}-\d{2})\s+\(fase", re.M)
STUB = re.compile(r"\(void\)\s*\w+\s*;")
# Miembros de datos propios. Solo los que empiezan por m_, que es el convenio
# de la casa: contar cualquier identificador daria falsos positivos con variables
# locales declaradas en metodos inline.
MIEMBRO = re.compile(
    r"^\s+[A-Za-z_][\w:<>,&*\s]*?[\s*&](m_[A-Za-z0-9_]+)\s*(?:\{[^}]*\})?\s*(?:=[^;]*)?;",
    re.M)


def alcance(ruta_json):
    if ruta_json and os.path.isfile(ruta_json):
        return json.load(open(ruta_json, encoding="utf-8"))
    destino = os.path.join(tempfile.gettempdir(), "mapa_clases_alcance.json")
    orden = [sys.executable, os.path.join(AQUI, "medir_alcance.py"), "--json", destino]
    r = subprocess.run(orden, cwd=RAIZ, capture_output=True, text=True)
    if r.returncode != 0 or not os.path.isfile(destino):
        raise SystemExit("no he podido medir el alcance: " + (r.stderr or r.stdout)[-400:])
    return json.load(open(destino, encoding="utf-8"))


def fichas(cabeceras, base):
    cuellos = base.get("cuellos", {})
    estado = base.get("estado", {})
    muertos = set(base.get("muertos", []))
    vivos = set()
    for area, cab in cabeceras:
        texto = open(os.path.join(RAIZ, area), encoding="utf-8", errors="replace").read()
        lineas = texto.count("\n") + 1
        clases = CLASE.findall(texto)
        paquetes = ", ".join(sorted(set(PAQUETE.findall(texto)))) or "-"
        bloqueos = []
        if area in cuellos:
            bloqueos.append("techo %d" % cuellos[area])
        if area in estado:
            bloqueos.append("%d miembros" % estado[area])
        if area in muertos:
            # Ojo: la lista de muertos viene de la foto CONGELADA de la guardia.
            # Si la cabecera esta aqui es porque el ejecutable la alcanza HOY,
            # o sea que la etiqueta es falsa. Se dice en vez de repetirla.
            bloqueos.append("la base la da por MUERTA pero el ejecutable la alcanza")
        if STUB.search(texto):
            bloqueos.append("tiene (void)param: posible stub")
        miembros = len(set(MIEMBRO.findall(texto)))
        cab.append((area, lineas, clases, paquetes, bloqueos, miembros))
        vivos |= {area}
    return vivos


def informa(datos, nombre, juego, base):
    por_area = collections.defaultdict(list)
    for area, lineas, clases, paquetes, bloqueos, miembros in datos:
        zona = area.split("/")[1] if area.startswith("include/") else area.split("/")[0]
        por_area[zona].append((area, lineas, clases, paquetes, bloqueos, miembros))

    total = sum(len(f[2]) for f in datos)
    out = []
    out.append("# Mapa de clases de `%s`, desde arriba\n" % nombre)
    out.append("Generado por `tools/mapa_clases.py`. **No se edita a mano:** se "
               "regenera, que es lo unico que evita que mienta. El analisis escrito "
               "de `analisis_clases/` es del 31/08 y cubria 101 cabeceras; esto sale "
               "del arbol de hoy.\n")
    out.append("`%s` enlaza **%d** fuentes y alcanza **%d** ficheros. De ellos **%d** "
               "son cabeceras con **%d** clases o structs.\n"
               % (nombre, juego["fuentes_enlazadas"], juego["ficheros_alcanzados"],
                  len(datos), total))
    out.append("Si una clase no esta aqui, este ejecutable no la alcanza.\n")
    for zona in sorted(por_area, key=lambda z: (-len(por_area[z]), z)):
        filas = por_area[zona]
        out.append("\n## %s — %d cabeceras, %d clases\n"
                   % (zona, len(filas), sum(len(f[2]) for f in filas)))
        out.append("| Cabecera | Líneas | Miembros | Clases | Paquete | Bloqueos |")
        out.append("| --- | ---: | ---: | --- | --- | --- |")
        for area, lineas, clases, paquetes, bloqueos, miembros in sorted(filas, key=lambda f: -f[1]):
            cs = ", ".join("`%s`" % c for c in clases) or "—"
            out.append("| `%s` | %d | %s | %s | %s | %s |"
                       % (area, lineas, miembros or "—", cs, paquetes,
                          "; ".join(bloqueos) or "—"))
    return "\n".join(out) + "\n"


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("ejecutable", nargs="?", default="juego")
    p.add_argument("--todos", action="store_true", help="los tres ejecutables reales")
    p.add_argument("--alcance", default=None, help="JSON de medir_alcance.py ya generado")
    p.add_argument("--base", default=os.path.join(RAIZ, "planificacion/linea_base_core.json"))
    p.add_argument("-o", "--salida", default=None)
    a = p.parse_args(argv)

    datos_alcance = alcance(a.alcance)
    base = json.load(open(a.base, encoding="utf-8"))
    nombres = sorted(datos_alcance["ejecutables"]) if a.todos else [a.ejecutable]

    for nombre in nombres:
        if nombre not in datos_alcance["ejecutables"]:
            raise SystemExit("no conozco el ejecutable %r; los que hay: %s"
                             % (nombre, ", ".join(sorted(datos_alcance["ejecutables"]))))
        juego = datos_alcance["ejecutables"][nombre]
        cabeceras = sorted(x for x in juego["alcanzados"] if x.endswith(".h"))
        faltan = [c for c in cabeceras if not os.path.isfile(os.path.join(RAIZ, c))]
        if faltan:
            print("aviso: %d cabeceras del alcance no estan en el disco" % len(faltan),
                  file=sys.stderr)
        datos = []
        fichas([(c, datos) for c in cabeceras if os.path.isfile(os.path.join(RAIZ, c))], base)
        texto = informa(datos, nombre, juego, base)
        if a.salida and not a.todos:
            open(a.salida, "w", encoding="utf-8").write(texto)
            print("escrito %s (%d cabeceras)" % (a.salida, len(datos)))
        else:
            sys.stdout.write(texto)
    return 0


if __name__ == "__main__":
    sys.exit(main())
