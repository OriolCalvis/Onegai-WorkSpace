#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Marca cada fichero del motor con los paquetes del Gantt que lo tocan.

POR QUE EXISTE
--------------
El repositorio lo escriben varios agentes a la vez. Un agente que abre
`src/Engine/Application.cpp` no tiene forma de saber que ese fichero lo
tocan CATORCE paquetes del corte B, ni cual es el criterio de cierre de
ninguno de ellos. El resultado es el que se ve: dos sesiones migrando la
misma clase el mismo dia sin enterarse.

Esto no lo arregla del todo un comentario, pero lo arregla en parte: al
abrir el fichero, lo primero que se lee es a que paquetes pertenece.

POR QUE SE GENERA Y NO SE ESCRIBE A MANO
----------------------------------------
Porque si no, caduca. Un banner que diga «C-14» seguira diciendolo un ano
despues de que C-14 cierre, y acabaria siendo lo mismo que los UML viejos
de DEC-17: documentacion que miente. La fuente es
`planificacion/pla/dades.py`, la misma de la que ya salen PLA_DEFINITIU.md,
el Gantt y la pagina. Se regenera y no puede divergir.

GUARDA CONTRA COLISIONES
------------------------
No escribe en un fichero que se haya modificado en los ultimos
`--minutos-quietos` minutos (30 por defecto): si otro agente lo esta
tocando, se salta y lo dice. Escribir encima de alguien que esta
trabajando es exactamente lo que este fichero existe para evitar.

Tampoco escribe en un fichero con trabajo sin commitear (lo que diga
`git status`), aunque lleve horas quieto. La guarda de los minutos no lo
ve: el 29/09 un --marcar habria reescrito 42 ficheros, y 12 eran trabajo
en vuelo de otras sesiones. Quien es dueno de ese trabajo lo marca con
--incluir-sucios. Sin git no se sabe, y entonces no se escribe nada.

Y no deja un fichero de src/, include/ o editor/ por encima de su techo:
el de tools/linea_base_core.py, o el suyo propio si es uno de los
`cuellos` de la linea base. Un bloque de comentarios no justifica romper
la ley del repositorio: el 29/09 EditorApplication.cpp estaba en 500 de
500 y el bloque le sumaba una linea.

BLOQUES REPETIDOS O ROTOS
-------------------------
Quita TODOS los bloques del fichero, no solo el primero, y reconoce el
cierre aunque este mal escrito («>>>» en vez de «<<<»). Asi habia uno
copiado a mano en include/Editor/ProjectDatabaseState.h (dace0c0), y la
version anterior, que quitaba solo el primero y exigia el cierre exacto,
lo dejaba debajo del bueno para siempre. Entre la apertura y el cierre
solo admite lineas de comentario, asi que un cierre roto nunca se lleva
codigo por delante. Una apertura sin cierre reconocible no se toca: se
dice, y se arregla a mano.

USO
---
    python3 tools/marcar_paquetes.py --mapa        # escribe el mapa .md
    python3 tools/marcar_paquetes.py --marcar --dry-run
    python3 tools/marcar_paquetes.py --marcar
    python3 tools/marcar_paquetes.py --marcar --solo include/X.h src/X.cpp
    python3 tools/marcar_paquetes.py --marcar --incluir-sucios   # solo el dueno del WIP
"""

import argparse
import collections
import json
import os
import re
import subprocess
import sys
import time

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
sys.path.insert(0, os.path.join(RAIZ, "planificacion", "pla"))
sys.path.insert(0, AQUI)
import dades  # noqa: E402
from linea_base_core import CARPETAS_DEL_CORE, TECHO  # noqa: E402

INICIO = "// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>"
FIN = "// <<< fin del bloque generado <<<"
# Apertura, solo lineas de comentario y cierre; el cierre se reconoce con
# «<<<» o con «>>>» al final. Las lineas en blanco que siguen al bloque se
# van con el: el bloque nuevo pone la suya.
BLOQUE = re.compile(
    r"^" + re.escape(INICIO) + r"[ \t]*\r?\n"
    r"(?:[ \t]*//[^\n]*\n)*?"
    r"[ \t]*//[ \t]*<<<[ \t]*fin del bloque generado[ \t]*(?:<<<|>>>)[ \t]*(?:\r?\n|\Z)"
    r"(?:[ \t]*\r?\n)*",
    re.M)

FASES = {c: t for c, t, _s, _d in dades.PHASES}


def sin_html(texto):
    return re.sub(r"</?code>", "`", texto)


def por_fichero():
    """fichero -> [paquete, ...], en el orden del plan."""
    mapa = collections.OrderedDict()
    for pid, titulo, fase, jornadas, deps, porque, ficheros, criterio, prueba in dades.P:
        for f in ficheros:
            mapa.setdefault(f, []).append(
                {"id": pid, "titulo": titulo, "fase": fase, "jornadas": jornadas,
                 "criterio": sin_html(criterio), "prueba": sin_html(prueba)})
    return mapa


def banner(ruta, paquetes):
    L = [INICIO]
    if len(paquetes) == 1:
        L.append("// Este fichero pertenece a 1 paquete del corte B:")
    else:
        L.append("// Este fichero lo tocan %d paquetes del corte B:" % len(paquetes))
    for p in paquetes:
        L.append("//   %s  (fase %s, %d j) - %s" % (p["id"], p["fase"], p["jornadas"], p["titulo"]))
    if len(paquetes) > 3:
        L.append("//")
        L.append("// AVISO - FICHERO DE CRUCE. Con %d paquetes encima, es de los" % len(paquetes))
        L.append("// sitios donde dos agentes se pisan sin enterarse. Antes de una")
        L.append("// refactorizacion grande aqui, mira si hay trabajo sin commitear")
        L.append("// (git status) y di en que paquete estas trabajando.")
    L.append("//")
    L.append("// Si tocas este fichero estas trabajando dentro de esos paquetes: su")
    L.append("// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.")
    L.append("// Criterios y pruebas: planificacion/PLA_DEFINITIU.md")
    L.append("// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md")
    L.append("// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar")
    L.append(FIN)
    return "\n".join(L) + "\n"


def lineas(texto):
    """Lineas como las cuenta tools/linea_base_core.py (sum(1 for _ in f))."""
    return texto.count("\n") + (1 if texto and not texto.endswith("\n") else 0)


def reescribe(original, bloque):
    """(texto nuevo, estado). Funcion pura: es la que prueba el test.

    estado: "igual"; "nuevo" (no tenia bloque); "cambia" (tenia uno);
    "repetido" (tenia varios y queda uno); "roto" (queda una apertura sin
    cierre reconocible: se devuelve el original sin tocar)."""
    crlf = "\r\n" in original
    cuerpo, quitados = BLOQUE.subn("", original)
    if INICIO in cuerpo:
        return original, "roto"
    salto = "\r\n" if crlf else "\n"
    cabeza = bloque.replace("\n", salto) if crlf else bloque
    nuevo = cabeza + salto + cuerpo.lstrip("\r\n")
    if nuevo == original:
        return original, "igual"
    if quitados == 0:
        return nuevo, "nuevo"
    return nuevo, ("cambia" if quitados == 1 else "repetido")


def sucios(raiz):
    """Rutas relativas a `raiz` con trabajo sin commitear, o None sin git.

    --no-optional-locks no es un detalle: un `git status` normal refresca el
    indice y toma .git/index.lock, y en una carpeta donde no se puede borrar
    (la sincronizada, la de Cowork) el candado se queda huerfano y deja sin
    git a las demas sesiones."""
    try:
        prefijo = subprocess.run(
            ["git", "-C", raiz, "--no-optional-locks", "rev-parse", "--show-prefix"],
            capture_output=True, text=True, check=True).stdout.strip()
        salida = subprocess.run(
            ["git", "-C", raiz, "--no-optional-locks", "status", "--porcelain", "-z",
             "--untracked-files=all", "--", "."],
            capture_output=True, text=True, check=True).stdout
    except (OSError, subprocess.CalledProcessError):
        return None
    rutas = set()
    trozos = salida.split("\0")
    i = 0
    while i < len(trozos):
        entrada = trozos[i]
        i += 1
        if len(entrada) < 4:
            continue
        estado, ruta = entrada[:2], entrada[3:]
        if estado[0] in "RC":
            i += 1  # con -z, la ruta de origen del renombrado va en la entrada siguiente
        if ruta.startswith(prefijo):
            rutas.add(ruta[len(prefijo):])
    return rutas


def techos(raiz):
    """(techo general, {ruta: techo propio}) de la linea base de la guardia."""
    try:
        with open(os.path.join(raiz, "planificacion", "linea_base_core.json"),
                  encoding="utf-8") as fh:
            base = json.load(fh)
    except (OSError, ValueError):
        return TECHO, {}
    return base.get("techo") or TECHO, base.get("cuellos") or {}


def limite(ruta, techo, cuellos):
    """Lineas que puede tener `ruta`, o None si la ley no mira esa carpeta."""
    if not ruta.startswith(tuple(c.rstrip("/") + "/" for c in CARPETAS_DEL_CORE)):
        return None
    return cuellos.get(ruta, techo)


def marcar_en(raiz, mapa, dry_run=True, minutos_quietos=30, sucias=frozenset(),
              techo=TECHO, cuellos=None, ahora=None):
    """Pone o actualiza el bloque en los ficheros de `mapa` bajo `raiz`.

    Devuelve {categoria: [...]}. Separada de marcar() para que la prueba
    la ejecute sobre un arbol sembrado, no sobre el de verdad."""
    cuellos = cuellos or {}
    ahora = time.time() if ahora is None else ahora
    r = collections.OrderedDict((k, []) for k in (
        "escritos", "repetidos", "iguales", "recientes", "sucios", "techo", "rotos", "ausentes"))
    for ruta, paquetes in mapa.items():
        if not ruta.endswith((".cpp", ".h")):
            continue
        absoluta = os.path.join(raiz, ruta)
        if not os.path.isfile(absoluta):
            r["ausentes"].append(ruta)
            continue
        edad = (ahora - os.path.getmtime(absoluta)) / 60.0
        if edad < minutos_quietos:
            r["recientes"].append((ruta, edad))
            continue
        with open(absoluta, encoding="utf-8", newline="") as fh:
            original = fh.read()
        nuevo, estado = reescribe(original, banner(ruta, paquetes))
        if estado == "igual":
            r["iguales"].append(ruta)
            continue
        if estado == "roto":
            r["rotos"].append(ruta)
            continue
        if ruta in sucias:
            r["sucios"].append(ruta)
            continue
        tope = limite(ruta, techo, cuellos)
        if tope is not None and lineas(nuevo) > tope and lineas(nuevo) > lineas(original):
            r["techo"].append((ruta, lineas(original), lineas(nuevo), tope))
            continue
        if not dry_run:
            with open(absoluta, "w", encoding="utf-8", newline="") as fh:
                fh.write(nuevo)
        r["escritos"].append(ruta)
        if estado == "repetido":
            r["repetidos"].append(ruta)
    return r


def marcar(dry_run, minutos_quietos, solo=None, incluir_sucios=False):
    mapa = por_fichero()
    if solo:
        pedidas = [os.path.normpath(s).replace(os.sep, "/") for s in solo]
        for s in pedidas:
            if s not in mapa:
                print("no esta en el plan (dades.py), no hay bloque que ponerle: %s" % s)
        mapa = collections.OrderedDict((k, v) for k, v in mapa.items() if k in pedidas)
    sucias = frozenset()
    if not incluir_sucios:
        sucias = sucios(RAIZ)
        if sucias is None:
            if not dry_run:
                raise SystemExit("sin git no se sabe que ficheros tienen trabajo sin commitear: "
                                 "no se escribe nada (usa --dry-run, o --incluir-sucios si "
                                 "todo el arbol es tuyo)")
            print("aviso: sin git, el --dry-run no puede separar el trabajo sin commitear")
            sucias = frozenset()
    techo, cuellos = techos(RAIZ)
    r = marcar_en(RAIZ, mapa, dry_run=dry_run, minutos_quietos=minutos_quietos,
                  sucias=sucias, techo=techo, cuellos=cuellos)

    print("marcados:      %d" % len(r["escritos"]))
    for ruta in r["repetidos"]:
        print("   %s: tenia el bloque repetido; queda uno" % ruta)
    print("ya al dia:     %d" % len(r["iguales"]))
    print("SALTADOS (los esta tocando alguien, < %d min): %d"
          % (minutos_quietos, len(r["recientes"])))
    for ruta, edad in sorted(r["recientes"], key=lambda x: x[1]):
        print("   %5.1f min  %s" % (edad, ruta))
    print("SALTADOS (trabajo sin commitear; su dueno usa --incluir-sucios): %d" % len(r["sucios"]))
    for ruta in r["sucios"]:
        print("   %s" % ruta)
    if r["techo"]:
        print("SALTADOS (el bloque lo dejaria por encima de su techo): %d" % len(r["techo"]))
        for ruta, antes, despues, tope in r["techo"]:
            print("   %s: %d -> %d lineas, techo %d" % (ruta, antes, despues, tope))
    if r["rotos"]:
        print("BLOQUE ROTO (apertura sin cierre reconocible; se arregla a mano): %d"
              % len(r["rotos"]))
        for ruta in r["rotos"]:
            print("   %s" % ruta)
    if r["ausentes"]:
        print("citados por el plan y NO existen en disco: %d" % len(r["ausentes"]))
        for ruta in r["ausentes"]:
            print("   %s" % ruta)
    if dry_run:
        print("\n(--dry-run: no se ha escrito nada)")
    return r


def escribir_mapa():
    mapa = por_fichero()
    reales = []
    for base in ("src", "include", "editor/src", "tests", "examples"):
        for dp, _dn, fn in os.walk(os.path.join(RAIZ, base)):
            if "build" in dp:
                continue
            for f in fn:
                if f.endswith((".cpp", ".h")):
                    reales.append(os.path.relpath(os.path.join(dp, f), RAIZ))
    citados = {r.rstrip("/") for r in mapa}
    huerfanos = sorted(f for f in reales if f not in citados)

    L = []
    A = L.append
    A("# Mapa del codigo <-> paquetes del Gantt\n")
    A("Generado por `tools/marcar_paquetes.py --mapa` desde "
      "`planificacion/pla/dades.py`. **No se edita a mano: se regenera.**\n")
    A("Para que sirve: que cualquier agente que abra un fichero sepa a que "
      "paquete del corte B pertenece, y que cualquiera que coja un paquete "
      "sepa que ficheros va a tocar — y con quien se va a cruzar.\n")

    A("## 1. Los ficheros de cruce\n")
    A("Ordenados por numero de paquetes que los tocan. **Son los sitios donde "
      "dos agentes se pisan.** Antes de una refactorizacion grande en uno de "
      "estos, mira `git status` por si hay trabajo sin commitear.\n")
    A("| Fichero | Paquetes | Cuales |")
    A("| --- | ---: | --- |")
    for ruta, paquetes in sorted(mapa.items(), key=lambda kv: -len(kv[1])):
        if len(paquetes) < 2:
            continue
        A("| `%s` | %d | %s |" % (ruta, len(paquetes), ", ".join(p["id"] for p in paquetes)))
    A("")

    A("## 2. Que ficheros toca cada paquete\n")
    for c, titulo, _s, _d in dades.PHASES:
        A("### Fase %s — %s\n" % (c, titulo))
        for pid, ptit, fase, jornadas, deps, porque, ficheros, criterio, prueba in dades.P:
            if fase != c:
                continue
            A("**%s — %s** (%d j)%s" % (pid, ptit, jornadas,
                                        "  ·  depende de: " + ", ".join(deps) if deps else ""))
            A("")
            for f in ficheros:
                otros = [p["id"] for p in mapa.get(f, []) if p["id"] != pid]
                marca = ("  — compartido con " + ", ".join(otros)) if otros else ""
                A("- `%s`%s" % (f, marca))
            A("")
            A("*Cerrado cuando:* %s" % sin_html(criterio))
            A("")

    A("## 3. Ficheros sin paquete asignado\n")
    A("%d de %d ficheros `.cpp`/`.h` del repositorio **no los cita ningun "
      "paquete del plan**. No significa que sobren: significa que nadie sabe, "
      "mirandolos, dentro de que trabajo caen. Un agente que los modifique no "
      "tiene criterio de cierre contra el que medirse.\n"
      % (len(huerfanos), len(reales)))
    porDir = collections.Counter(os.path.dirname(f) for f in huerfanos)
    A("| Carpeta | Ficheros sin paquete |")
    A("| --- | ---: |")
    for d, n in porDir.most_common():
        A("| `%s/` | %d |" % (d, n))
    A("")
    A("<details><summary>La lista completa</summary>\n")
    for f in huerfanos:
        A("- `%s`" % f)
    A("\n</details>\n")

    destino = os.path.join(RAIZ, "planificacion", "MAPA_DEL_CODIGO.md")
    with open(destino, "w", encoding="utf-8") as fh:
        fh.write("\n".join(L))
    print("escrito %s (%d ficheros citados, %d sin paquete)"
          % (os.path.relpath(destino, RAIZ), len(citados), len(huerfanos)))


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--mapa", action="store_true", help="escribe planificacion/MAPA_DEL_CODIGO.md")
    ap.add_argument("--marcar", action="store_true", help="inserta/actualiza el banner en cada fichero")
    ap.add_argument("--dry-run", action="store_true", help="con --marcar: dice que haria, sin escribir")
    ap.add_argument("--minutos-quietos", type=int, default=30,
                    help="no tocar ficheros modificados hace menos de N minutos (por defecto 30)")
    ap.add_argument("--solo", nargs="+", metavar="RUTA",
                    help="con --marcar: solo estos ficheros (rutas relativas a MotorGraphico/)")
    ap.add_argument("--incluir-sucios", action="store_true",
                    help="con --marcar: escribir tambien en ficheros con trabajo sin commitear "
                         "(solo quien es dueno de ese trabajo)")
    args = ap.parse_args()
    if not args.mapa and not args.marcar:
        ap.print_help()
        sys.exit(1)
    if args.mapa:
        escribir_mapa()
    if args.marcar:
        marcar(args.dry_run, args.minutos_quietos, args.solo, args.incluir_sucios)
