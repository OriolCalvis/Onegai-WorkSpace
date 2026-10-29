#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Ningun test registrado a medias.

POR QUE EXISTE
--------------
En este repositorio escriben varios agentes a la vez y, para no pisarse,
cada test nuevo va en su propio `cmake/XxxTests.cmake` y se engancha con
UNA linea `include(...)` en CMakeLists.txt. La convencion funciona para lo
que fue pensada: dos agentes pueden anadir tests el mismo minuto sin tocar
el mismo bloque.

Lo que no cubre es la linea `include()`, que sigue estando en un fichero
compartido. Y ahi pasa lo que tenia que pasar: un agente guarda
CMakeLists.txt desde una version anterior y se lleva por delante las
lineas que otro habia anadido mientras tanto. El fragmento sigue en
cmake/, el test sigue en tests/, todo compila, la regresion local pasa
—porque quien la corre compila a mano— y el test simplemente NO ESTA en
CI. Nadie se entera.

El 13/09/2026, al mirarlo por primera vez, habia CINCO fragmentos
huerfanos de tres agentes distintos.

QUE COMPRUEBA
-------------
1. Todo `cmake/*.cmake` aparece en algun `include()` de CMakeLists.txt.
2. Todo `include(cmake/...)` apunta a un fichero que existe.
3. Cada fragmento que declara `add_executable(... tests/...)` cita
   ficheros de test que existen de verdad.

No comprueba que el test PASE: eso es de CI. Comprueba que llegue a
ejecutarse, que es justo lo que se pierde en silencio.

USO
---
  python3 tools/comprobar_fragmentos_cmake.py
"""

import os
import re
import sys

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CMAKELISTS = os.path.join(RAIZ, "CMakeLists.txt")
FRAGMENTOS = os.path.join(RAIZ, "cmake")

RE_INCLUDE = re.compile(r"include\s*\(\s*(cmake/[A-Za-z0-9_./-]+\.cmake)\s*\)")
# La mirada atras negativa importa: sin ella, "editor/src/main.cpp" casa
# como "src/main.cpp" y el comprobador inventa un fichero que falta.
RE_FUENTE = re.compile(
    r"(?<![A-Za-z0-9_./-])((?:tests|examples|src|editor)/[A-Za-z0-9_./-]+\.cpp)")


def main():
    if not os.path.isfile(CMAKELISTS):
        print("no encuentro CMakeLists.txt en %s" % RAIZ)
        return 1

    with open(CMAKELISTS, encoding="utf-8") as fh:
        cmakelists = fh.read()

    incluidos = set(RE_INCLUDE.findall(cmakelists))
    presentes = set()
    for nombre in sorted(os.listdir(FRAGMENTOS)):
        if nombre.endswith(".cmake"):
            presentes.add("cmake/" + nombre)

    problemas = []

    # 1. Fragmentos que no incluye nadie. Es el fallo silencioso: el test
    #    existe, compila a mano y no corre en CI.
    for ruta in sorted(presentes - incluidos):
        problemas.append(
            "%s existe y NO lo incluye CMakeLists.txt: el test que registra no "
            "llega a CI. Anade `include(%s)` en el bloque de tests." % (ruta, ruta))

    # 2. Includes que apuntan a nada. Rompen la configuracion entera.
    for ruta in sorted(incluidos - presentes):
        problemas.append(
            "CMakeLists.txt hace include(%s) y ese fichero no existe: cmake "
            "fallara al configurar." % ruta)

    # 3. Fragmentos que citan fuentes que no estan.
    for ruta in sorted(presentes & incluidos):
        absoluta = os.path.join(RAIZ, ruta)
        with open(absoluta, encoding="utf-8") as fh:
            texto = fh.read()
        # Solo las lineas de fuentes, no los comentarios: un comentario
        # puede nombrar un fichero de otro paquete a modo de referencia.
        util = "\n".join(l for l in texto.splitlines() if not l.lstrip().startswith("#"))
        for fuente in sorted(set(RE_FUENTE.findall(util))):
            if not os.path.isfile(os.path.join(RAIZ, fuente)):
                problemas.append("%s cita %s, que no existe" % (ruta, fuente))

    if problemas:
        print("FRAGMENTOS CMAKE MAL REGISTRADOS: %d" % len(problemas))
        for p in problemas:
            print("  - %s" % p)
        print("\nEsto no rompe ninguna compilacion: por eso hace falta mirarlo.")
        return 1

    print("OK: %d fragmentos cmake, todos incluidos y con sus fuentes en su sitio."
          % len(presentes))
    return 0


if __name__ == "__main__":
    sys.exit(main())
