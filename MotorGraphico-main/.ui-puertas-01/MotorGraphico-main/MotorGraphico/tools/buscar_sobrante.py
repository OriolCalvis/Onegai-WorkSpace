#!/usr/bin/env python3
"""Lo que sobra: codigo al que no se refiere nadie, y codigo repetido.

DOS PREGUNTAS DISTINTAS, Y NINGUNA LA CONTESTA medir_alcance.py.

  · MUERTOS. medir_alcance mira que ficheros alcanzan los tres
    ejecutables, y no sigue los #include entre CABECERAS: el 24/09
    daba 43 cabeceras «sin alcance», y siete de ellas eran los
    CatalogLoaders* que RpgCatalogs.h incluye una por una. Un listado con
    falsos positivos no se usa para borrar. Aqui la pregunta es otra, y se
    puede comprobar a mano: ¿se refiere a esta cabecera alguien que no sea
    ella misma, su .cpp y su prueba? Si no, es una funcionalidad
    terminada que nadie enchufo -- que NO es lo mismo que basura, y por
    eso el informe no dice «borra», dice «decide».

  · REPETIDO. Cuerpos de funcion identicos en ficheros distintos. El
    24/09 saco doce copias de «vuelca el framebuffer a un PPM», y cinco
    de ellas no volteaban las filas: llevaban anos escribiendo la imagen
    del reves. Eso es lo que cuesta la duplicacion -- el arreglo llega a
    unas copias y a otras no -- y por eso se mide.

Uso:
    python3 tools/buscar_sobrante.py            # las dos listas
    python3 tools/buscar_sobrante.py --muertos
    python3 tools/buscar_sobrante.py --repetido
    python3 tools/buscar_sobrante.py --raiz <dir> --json <fichero>
"""
import argparse
import collections
import hashlib
import json
import os
import re
import sys

CARPETAS = ("include", "src", "editor", "examples", "tests")
EXT_CODIGO = (".h", ".hpp", ".cpp", ".cc")
# Un cuerpo mas corto que esto no es duplicacion, es coincidencia.
MINIMO_LINEAS = 5

FIRMA = re.compile(r'^[A-Za-z_][\w:<>,\s\*&]*\b(\w+)\s*\([^;]*\)\s*(?:const\s*)?\{')


def _ficheros(raiz):
    salida = []
    for carpeta in CARPETAS:
        base = os.path.join(raiz, carpeta)
        for aqui, _dirs, nombres in os.walk(base):
            if os.sep + "out" + os.sep in aqui + os.sep:
                continue
            for n in nombres:
                if n.endswith(EXT_CODIGO):
                    salida.append(os.path.relpath(os.path.join(aqui, n), raiz))
    return sorted(salida)


def _lee(raiz, rel):
    with open(os.path.join(raiz, rel), encoding="utf-8", errors="replace") as f:
        return f.read()


def _familia(rel, todos):
    """El .cpp y la prueba que 'pertenecen' a una cabecera, por nombre.

    Se emparejan por nombre y no por contenido a proposito: es la regla
    que cualquiera puede comprobar de un vistazo, y una cabecera cuyo
    .cpp se llama de otra forma ya es una pista de otra cosa.
    """
    base = os.path.splitext(os.path.basename(rel))[0]
    fam = {rel}
    for otro in todos:
        if not otro.endswith((".cpp", ".cc")):
            continue
        nombre = os.path.splitext(os.path.basename(otro))[0]
        if nombre in (base, base + "Tests", base.replace("State", "") + "Tests"):
            fam.add(otro)
    return fam


def muertos(raiz):
    """Cabeceras a las que solo se refieren ellas, su .cpp y su prueba."""
    todos = _ficheros(raiz)
    texto = {rel: _lee(raiz, rel) for rel in todos}
    filas = []
    for rel in todos:
        if not rel.endswith((".h", ".hpp")):
            continue
        nombre = os.path.basename(rel)
        fam = _familia(rel, todos)
        if any(nombre in t for otro, t in texto.items() if otro not in fam):
            continue
        lineas = sum(texto[x].count("\n") + 1 for x in fam)
        filas.append({"cabecera": rel, "lineas": lineas,
                      "se_va_con_ella": sorted(x for x in fam if x != rel)})
    filas.sort(key=lambda f: -f["lineas"])
    return filas


def repetido(raiz, minimo=MINIMO_LINEAS):
    """Cuerpos de funcion identicos (salvo comentarios y espacios)."""
    todos = [r for r in _ficheros(raiz) if not r.startswith("tests")]
    bolsa = collections.defaultdict(list)
    for rel in todos:
        lineas = _lee(raiz, rel).split("\n")
        i = 0
        while i < len(lineas):
            m = FIRMA.match(lineas[i])
            if not m:
                i += 1
                continue
            prof, empezo, j = 0, False, i
            while j < len(lineas):
                prof += lineas[j].count("{") - lineas[j].count("}")
                if "{" in lineas[j]:
                    empezo = True
                if empezo and prof <= 0:
                    break
                j += 1
            if j - i + 1 >= minimo:
                cuerpo = "\n".join(lineas[i:j + 1])
                plano = re.sub(r"\s+", " ", re.sub(r"//[^\n]*", "", cuerpo)).strip()
                bolsa[hashlib.md5(plano.encode()).hexdigest()].append(
                    {"fichero": rel, "linea": i + 1, "lineas": j - i + 1, "nombre": m.group(1)})
            i = j + 1
    grupos = []
    for sitios in bolsa.values():
        if len({s["fichero"] for s in sitios}) < 2:
            continue
        grupos.append({"nombre": sitios[0]["nombre"], "lineas": sitios[0]["lineas"],
                       "copias": len(sitios), "sitios": sitios,
                       "lineas_de_sobra": sitios[0]["lineas"] * (len(sitios) - 1)})
    grupos.sort(key=lambda g: -g["lineas_de_sobra"])
    return grupos


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--raiz", default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    ap.add_argument("--muertos", action="store_true")
    ap.add_argument("--repetido", action="store_true")
    ap.add_argument("--json", help="escribe el informe entero en este fichero")
    a = ap.parse_args(argv)
    los_dos = not a.muertos and not a.repetido

    informe = {}
    if a.muertos or los_dos or a.json:
        informe["muertos"] = muertos(a.raiz)
    if a.repetido or los_dos or a.json:
        informe["repetido"] = repetido(a.raiz)

    if a.muertos or los_dos:
        filas = informe["muertos"]
        print("NADIE SE REFIERE A ELLAS (ni un fichero fuera de su propia familia): %d" % len(filas))
        print("  Una funcionalidad terminada que nadie enchufo NO es basura.")
        print("  Esto es una lista para DECIDIR (cablear o retirar), no para borrar a ciegas.\n")
        for f in filas:
            print("  %6d  %-52s %s" % (f["lineas"], f["cabecera"], " ".join(f["se_va_con_ella"])))
        print("\n  lineas en juego: %d" % sum(f["lineas"] for f in filas))

    if a.repetido or los_dos:
        grupos = informe["repetido"]
        print("\nEL MISMO CUERPO EN FICHEROS DISTINTOS: %d grupos" % len(grupos))
        for g in grupos:
            print("\n  %s  (%d lineas x %d copias)" % (g["nombre"], g["lineas"], g["copias"]))
            for s in g["sitios"]:
                print("      %s:%d" % (s["fichero"], s["linea"]))
        print("\n  lineas de sobra: %d" % sum(g["lineas_de_sobra"] for g in grupos))

    if a.json:
        with open(a.json, "w", encoding="utf-8") as f:
            json.dump(informe, f, ensure_ascii=False, indent=1, sort_keys=True)
        print("\ninforme en " + a.json)
    return 0


if __name__ == "__main__":
    sys.exit(main())
