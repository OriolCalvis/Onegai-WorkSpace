#!/usr/bin/env python3
"""NET-E1b: quita de los atlas `pelo_*` y `barba_*` las filas de color que se reproducen tiniendo la fila
castana (el motor, el creador y las herramientas ya tinen: NET-E1a). Quedan las castanas y las propias de raza.

Para cada atlas (UNO cada vez, ~0,5 GB de pico) comprueba que cada fila de color es, pixel a pixel, la castana
del mismo estilo con los pares de la paleta (`paletas.pelo_castano` -> `paletas.<color>`). Si UNA falla no escribe
nada (ni PNG ni JSON). Despues renumera `fila` en `personajes.json` (pelo y barba). Es idempotente.

Uso (desde MotorGraphico/):
  python3 tools/recortar_colores_capa.py                 # comprueba y recorta
  python3 tools/recortar_colores_capa.py --comprobar     # solo comprueba, no escribe
  python3 tools/recortar_colores_capa.py --tex DIR       # otra carpeta de texturas (copias de prueba)
"""
import argparse, json, sys
from pathlib import Path

import numpy as np
from PIL import Image

Image.MAX_IMAGE_PIXELS = None
L = 64
GRUPOS = ("pelo", "barba")
BASE = "pelo_castano"
TEX_DEFECTO = Path(__file__).resolve().parent.parent / "assets" / "textures"


def _rgb(h):
    h = h.lstrip("#")
    return tuple(int(h[i:i + 2], 16) for i in (0, 2, 4))


def pares(pal, color):
    """[(origen, destino)] de castano a `color`: el primero gana si el origen se repite, sin los que no cambian."""
    o, d = pal[BASE].split(), pal[color].split()
    out = []
    for a, b in zip(o, d):
        a, b = _rgb(a), _rgb(b)
        if a != b and all(a != x for x, _ in out):
            out.append((a, b))
    return out


def tenir(fila, cambios):
    """Misma regla que el motor: RGB exacto con alfa > 0 pasa al destino; el alfa no cambia."""
    r = fila.copy()
    for a, b in cambios:
        m = (fila[..., 0] == a[0]) & (fila[..., 1] == a[1]) & (fila[..., 2] == a[2]) & (fila[..., 3] > 0)
        r[..., :3][m] = b
    return r


def plan(man, grupo):
    """(filas que quedan [indices 0-based en el atlas], filas a quitar [(indice, indice de la castana, color)])."""
    filas = man[grupo]["filas"]
    base = {f["estilo"]: f["fila"] - 1 for f in filas if f["color"] == BASE}
    quedan, quitar = [], []
    for f in filas:
        i = f["fila"] - 1
        if f["color"] in ("", BASE) or f["estilo"] not in base:
            quedan.append(i)
        else:
            quitar.append((i, base[f["estilo"]], f["color"], f["id"]))
    return quedan, quitar


def comprobar_atlas(tex, man, grupo, talla, quitar):
    """Lista de (id, px de diferencia) de las filas de `quitar` que NO salen de tenir la castana."""
    a = np.asarray(Image.open(tex / man[grupo]["atlas"][talla]).convert("RGBA"))
    mal = []
    for i, b, color, ident in quitar:
        esperado = tenir(a[b * L:(b + 1) * L], pares(man["paletas"], color))
        real = a[i * L:(i + 1) * L]
        dif = int(((esperado != real).any(-1) & ((esperado[..., 3] > 0) | (real[..., 3] > 0))).sum())
        if dif:
            mal.append((ident, dif))
    return mal


def recortar_atlas(tex, man, grupo, talla, quedan):
    ruta = tex / man[grupo]["atlas"][talla]
    a = np.asarray(Image.open(ruta).convert("RGBA"))
    nuevo = np.concatenate([a[i * L:(i + 1) * L] for i in quedan], axis=0)
    del a
    Image.fromarray(nuevo, "RGBA").save(ruta, optimize=True)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--tex", default=str(TEX_DEFECTO))
    ap.add_argument("--comprobar", action="store_true", help="solo comprueba; no escribe nada")
    a = ap.parse_args(argv)
    tex = Path(a.tex)
    man = json.load(open(tex / "personajes.json"))
    trabajo = {g: plan(man, g) for g in GRUPOS}
    if not any(q for _, q in trabajo.values()):
        print("nada que recortar: los atlas ya solo llevan castanas y propias")
        return 0
    total = comprobadas = 0
    fallos = []
    for g in GRUPOS:  # pasada 1: solo lectura, un atlas cada vez
        _, quitar = trabajo[g]
        if not quitar:
            continue
        for t in man[g]["atlas"]:
            mal = comprobar_atlas(tex, man, g, t, quitar)
            comprobadas += len(quitar)
            fallos += [(g, t, i, d) for i, d in mal]
            print(f"{g}_{t}: {len(quitar)} filas de color, {len(mal)} con diferencia", flush=True)
    print(f"comparaciones: {comprobadas}, con diferencia: {len(fallos)}")
    if fallos:
        for f in fallos[:20]:
            print("  FALLA", *f)
        print("NO se escribe nada", file=sys.stderr)
        return 1
    if a.comprobar:
        return 0
    for g in GRUPOS:  # pasada 2: escribir, un atlas cada vez
        quedan, quitar = trabajo[g]
        if not quitar:
            continue
        for t in man[g]["atlas"]:
            recortar_atlas(tex, man, g, t, quedan)
            print(f"escrito {man[g]['atlas'][t]}", flush=True)
        nuevas = []
        for n, i in enumerate(quedan):
            f = dict(next(x for x in man[g]["filas"] if x["fila"] - 1 == i))
            f["fila"] = n + 1
            nuevas.append(f)
        man[g]["filas"] = nuevas
    # el mismo formato con el que lo guarda gen_personajes.py (ver abajo si cambia)
    with open(tex / "personajes.json", "w") as fh:
        json.dump(man, fh, ensure_ascii=False, indent=1)
        fh.write("\n")
    print("personajes.json:", {g: len(man[g]["filas"]) for g in GRUPOS})
    return 0


if __name__ == "__main__":
    sys.exit(main())
