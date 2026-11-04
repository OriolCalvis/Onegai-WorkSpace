#!/usr/bin/env python3
"""Reune las capturas de todas las ramas y carpetas en una galeria de .md navegable.

Uso: python3 construir.py [--salida ~/onegai-ramas/_galeria]

Fuentes: toda la historia git (todas las ramas) y, en disco, el checkout de Oriol y
los worktrees de ~/onegai-ramas. Fuera: assets del juego, compilaciones, vault de lore.
Cada imagen se guarda una vez (por contenido) en img/; solo lee las fuentes.
"""
import argparse
import collections
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys

EXT = (".png", ".jpg", ".jpeg", ".gif", ".webp", ".ppm")
# Un componente de ruta con estos nombres indica que no es una captura.
NO_CAPTURA = {
    ".git", "assets", "textures", "_deps", "build", "build-release", "build-debug",
    "node_modules", "Onegai 2", "dndWeebCC-master", "_galeria", "resources",
}
PREFIJOS_RUTA = ("MotorGraphico-main/", "MotorGraphico/")


def es_captura(ruta):
    partes = ruta.split("/")
    if any(p in NO_CAPTURA or p.startswith("juego-") for p in partes[:-1]):
        return False
    return partes[-1].lower().endswith(EXT)


def limpia(ruta):
    """Quita los prefijos del monorepo y de las copias para agrupar igual."""
    if ".ui-puertas-01/" in ruta:
        ruta = ruta.split(".ui-puertas-01/", 1)[1]
    cambiado = True
    while cambiado:
        cambiado = False
        for p in PREFIJOS_RUTA:
            if ruta.startswith(p):
                ruta = ruta[len(p):]
                cambiado = True
    return ruta


def tema_de(ruta):
    partes = limpia(ruta).split("/")[:-1]
    return "/".join(partes[:3]) if partes else "(arrel)"


def slug(texto):
    return re.sub(r"[^A-Za-z0-9._-]+", "_", texto).strip("_") or "arrel"


def git(repo, *args):
    return subprocess.run(["git", "-C", repo, "-c", "core.checkStat=minimal", *args],
                          capture_output=True, text=True, errors="replace").stdout


def lee_blobs(repo, shas):
    """Devuelve {sha: bytes} con una sola llamada a git cat-file --batch."""
    p = subprocess.Popen(["git", "-C", repo, "cat-file", "--batch"],
                         stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    for s in shas:
        p.stdin.write((s + "\n").encode())
    p.stdin.close()
    while True:
        cab = p.stdout.readline().split()
        if len(cab) != 3:
            break
        datos = p.stdout.read(int(cab[2]))
        p.stdout.read(1)
        yield cab[0].decode(), datos
    p.wait()


class Galeria:
    def __init__(self, salida):
        self.salida = salida
        self.img = {}  # md5 -> dict(fichero, nombre, rutas, ramas, fecha)

    def guarda(self, datos, ruta, fecha=None, rama=None):
        h = hashlib.md5(datos).hexdigest()
        e = self.img.get(h)
        if e is None:
            ext = os.path.splitext(ruta)[1].lower()
            fichero = f"img/{h[:2]}/{h}{'.png' if ext == '.ppm' else ext}"
            destino = os.path.join(self.salida, fichero)
            os.makedirs(os.path.dirname(destino), exist_ok=True)
            if not os.path.exists(destino):
                if ext == ".ppm":
                    tmp = destino + ".ppm"
                    with open(tmp, "wb") as f:
                        f.write(datos)
                    subprocess.run(["sips", "-s", "format", "png", tmp, "--out", destino],
                                   capture_output=True)
                    os.remove(tmp)
                else:
                    with open(destino, "wb") as f:
                        f.write(datos)
            e = self.img[h] = dict(fichero=fichero, nombre=os.path.basename(ruta),
                                   rutas=set(), ramas=set(), fecha=fecha, desc="")
        e["rutas"].add(ruta)
        if rama:
            e["ramas"].add(rama)
        if fecha and (not e["fecha"] or fecha < e["fecha"]):
            e["fecha"] = fecha
        return e


def recoge_git(g, repo):
    fechas = {}
    fecha = None
    for l in git(repo, "log", "--all", "--diff-filter=A", "--format=@%as", "--name-only").splitlines():
        if l.startswith("@"):
            fecha = l[1:]
        elif l and es_captura(l):
            fechas[l] = min(fecha, fechas.get(l, fecha))

    rutas_de = collections.defaultdict(set)  # sha -> rutas
    ramas_de = collections.defaultdict(set)  # sha -> ramas cuya punta lo tiene
    ramas = git(repo, "for-each-ref", "--format=%(refname:short)", "refs/heads").split("\n")
    ramas = [r for r in ramas if r]
    for r in ramas:
        for l in git(repo, "ls-tree", "-r", "refs/heads/" + r).splitlines():
            meta, ruta = l.split("\t", 1)
            ruta = ruta.strip('"')
            if es_captura(ruta):
                sha = meta.split()[2]
                rutas_de[sha].add(ruta)
                ramas_de[sha].add(r)
    # Lo que solo existio en commits intermedios
    for l in git(repo, "rev-list", "--objects", "--all").splitlines():
        p = l.split(" ", 1)
        if len(p) == 2 and es_captura(p[1]):
            rutas_de[p[0]].add(p[1])

    for sha, datos in lee_blobs(repo, list(rutas_de)):
        for ruta in rutas_de[sha]:
            e = g.guarda(datos, ruta, fechas.get(ruta))
        e["ramas"] |= ramas_de[sha]
    return ramas


def recoge_disco(g, raiz, etiqueta):
    for dp, dn, fn in os.walk(raiz):
        dn[:] = [d for d in dn if d not in NO_CAPTURA and not d.startswith("juego-")]
        for f in fn:
            if f.lower().endswith(EXT):
                p = os.path.join(dp, f)
                try:
                    with open(p, "rb") as fh:
                        datos = fh.read()
                    fecha = subprocess.run(["date", "-r", str(int(os.path.getmtime(p))), "+%F"],
                                           capture_output=True, text=True).stdout.strip()
                except OSError as err:
                    print("no se puede leer", p, err, file=sys.stderr)
                    continue
                ruta = os.path.relpath(p, raiz)
                if "_archivo_logs" in raiz:
                    ruta = ruta.split("/", 1)[1] if "/" in ruta else ruta
                e = g.guarda(datos, ruta, fecha)
                e["ramas"].add("(disc) " + etiqueta)


def lee_descripciones(rutas_md):
    """Filas de CAPTURAS.md: el nombre del PNG -> la columna «Que se ve»."""
    desc = {}
    for md in rutas_md:
        if not os.path.exists(md):
            continue
        col = 1
        with open(md, encoding="utf-8", errors="replace") as f:
            for l in f:
                if not l.startswith("|"):
                    continue
                celdas = [c.strip() for c in l.strip().strip("|").split("|")]
                for i, c in enumerate(celdas):
                    if "se ve" in c.lower() or "es veu" in c.lower():
                        col = i
                nombres = re.findall(r"`([^`]+\.(?:png|jpg|ppm))`", celdas[0])
                # Tabla de cierres de E-MISIONES-01b: la primera columna es la mision
                if not nombres and re.fullmatch(r"[a-z0-9_]+", celdas[0]):
                    nombres = [f"q1_{celdas[0]}_cierre.png"]
                if nombres and len(celdas) > col:
                    for n in nombres:
                        desc[n] = celdas[col]
    return desc


def figura(e, rel):
    pie = [f"**{e['nombre']}**"]
    if e["fecha"]:
        pie.append(e["fecha"])
    if e["desc"]:
        pie.append(e["desc"].replace("\n", " "))
    return f"![{e['nombre']}]({rel}{e['fichero']})\n{' · '.join(pie)}\n"


def pagina(ruta, titulo, cuerpo):
    os.makedirs(os.path.dirname(ruta), exist_ok=True)
    with open(ruta, "w", encoding="utf-8") as f:
        f.write(f"# {titulo}\n\n{cuerpo}\n")


def escribe_paginas(g, salida, ramas):
    por_tema = collections.defaultdict(list)
    por_rama = collections.defaultdict(list)
    por_mes = collections.defaultdict(list)
    for e in g.img.values():
        for t in {tema_de(r) for r in e["rutas"]}:
            por_tema[t].append(e)
        for r in e["ramas"]:
            por_rama[r].append(e)
        por_mes[(e["fecha"] or "sense data")[:7]].append(e)

    orden = lambda e: (e["fecha"] or "", e["nombre"])
    for d in ("temes", "branques", "cronologia"):
        shutil.rmtree(os.path.join(salida, d), ignore_errors=True)

    for t, lista in por_tema.items():
        secciones = collections.defaultdict(list)
        for e in lista:
            carpeta = next(c for c in sorted(os.path.dirname(limpia(r)) or "(arrel)" for r in e["rutas"]
                                             if tema_de(r) == t))
            secciones[carpeta].append(e)
        cuerpo = [f"{len(lista)} imatges."]
        for c in sorted(secciones):
            if len(secciones) > 1:
                cuerpo.append(f"\n## {c}\n")
            cuerpo += [figura(e, "../") for e in sorted(secciones[c], key=orden)]
        pagina(os.path.join(salida, "temes", slug(t) + ".md"), t, "\n".join(cuerpo))

    for r, lista in por_rama.items():
        unicas = [e for e in lista if len(e["ramas"]) == 1]
        cuerpo = [f"{len(lista)} imatges; {len(unicas)} només aquí."]
        if unicas:
            cuerpo.append("\n## Només en aquesta branca\n")
            cuerpo += [figura(e, "../") for e in sorted(unicas, key=orden)]
        comunes = collections.defaultdict(list)
        for e in lista:
            if len(e["ramas"]) > 1:
                comunes[tema_de(sorted(e["rutas"])[0])].append(e)
        for t in sorted(comunes):
            cuerpo.append(f"\n## {t}\n")
            cuerpo += [figura(e, "../") for e in sorted(comunes[t], key=orden)]
        pagina(os.path.join(salida, "branques", slug(r) + ".md"), r, "\n".join(cuerpo))

    for m, lista in por_mes.items():
        cuerpo = [f"{len(lista)} imatges."]
        cuerpo += [figura(e, "../") for e in sorted(lista, key=orden, reverse=True)]
        pagina(os.path.join(salida, "cronologia", slug(m) + ".md"), f"Cronologia: {m}", "\n".join(cuerpo))

    idx = [f"**{len(g.img)} imatges diferents** de {len(ramas)} branques i de les carpetes del disc. "
           "Cada imatge només es guarda una vegada, encara que surti a moltes branques.\n",
           "Fora de la galeria: assets del joc, compilacions i el vault de lore.\n",
           "## Cronologia\n"]
    idx += [f"- [{m}](cronologia/{slug(m)}.md) ({len(por_mes[m])})" for m in sorted(por_mes, reverse=True)]
    idx.append("\n## Temes\n")
    idx += [f"- [{t}](temes/{slug(t)}.md) ({len(por_tema[t])})" for t in sorted(por_tema)]
    idx.append("\n## Branques\n")
    idx += [f"- [{r}](branques/{slug(r)}.md) ({len(por_rama[r])})" for r in sorted(por_rama)]
    pagina(os.path.join(salida, "index.md"), "Captures d'Onegai", "\n".join(idx))

    with open(os.path.join(salida, "cerca.json"), "w", encoding="utf-8") as f:
        json.dump([dict(f=e["fichero"], n=e["nombre"], d=e["desc"], t=e["fecha"] or "",
                        r=sorted(limpia(x) for x in e["rutas"])[:5]) for e in g.img.values()],
                  f, ensure_ascii=False)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--salida", default=os.path.expanduser("~/onegai-ramas/_galeria"))
    a = ap.parse_args()
    salida = os.path.expanduser(a.salida)
    ramas_dir = os.path.expanduser("~/onegai-ramas")
    repo = os.path.join(ramas_dir, "UNI-02-11")
    aqui = os.path.dirname(os.path.abspath(__file__))
    checkout = os.path.expanduser("~/Documents/Documentos - Oriol Os (2)/Software")

    g = Galeria(salida)
    ramas = recoge_git(g, repo)
    print("git:", len(g.img), "imagenes en", len(ramas), "ramas", flush=True)
    recoge_disco(g, checkout, "checkout")
    print("+ checkout:", len(g.img), flush=True)
    for d in sorted(os.listdir(ramas_dir)):
        p = os.path.join(ramas_dir, d)
        if os.path.isdir(p) and d != "_galeria":
            recoge_disco(g, p, d)
    print("+ onegai-ramas:", len(g.img), flush=True)

    # De menos a mas fiable: el ultimo que describe una captura manda
    informes = os.path.join(repo, "out/fabrica/UNI-02-11")
    desc = lee_descripciones([os.path.join(informes, n) for n in (
        "REGISTRE_CAPTURES_I_PROVES.md", "MATRIZ_DIA1.md", "MATRIZ_GORRAN.md", "MATRIZ_DIA3.md",
        "MATRIZ_OCASO.md", "capturas/CAPTURAS.md")])
    for e in g.img.values():
        e["desc"] = desc.get(e["nombre"], "")
    print("con descripcion:", sum(1 for e in g.img.values() if e["desc"]), flush=True)

    escribe_paginas(g, salida, ramas)
    shutil.copy(os.path.join(aqui, "servir.py"), os.path.join(salida, "servir.py"))
    lanzador = os.path.join(salida, "Obrir galeria.command")
    with open(lanzador, "w", encoding="utf-8") as f:
        f.write('#!/bin/bash\ncd "$(dirname "$0")"\n(sleep 1; open http://127.0.0.1:8765/) &\n'
                'exec python3 -I servir.py 8765\n')
    os.chmod(lanzador, 0o755)
    print("listo:", salida)


if __name__ == "__main__":
    main()
