#!/usr/bin/env python3
"""validar_edificis.py — guarda de funcion/material de las casas y tiendas de ciudades y pueblos (H-CIU-1).

Regla de Oriol (22/10 y 29/10): los bloques de piedra gris fria son para murallas y
estructuras militares; ninguna casa ni tienda se dibuja con ellos si hay alternativa.

  (a) ERROR: una casa o tienda dibujada con bloques (no con sprite entero) cuyo tile admite
      un material prohibido (caserna, fortaleza, muralla, muralla_rota, piedra_publica), o un
      tipo de casa/tienda de bloques.json con alguno de esos materiales. Los edificios
      publicos (banco, banos, biblioteca, universidad) pueden seguir con piedra_publica.
  (b) AVISO: dos edificios distintos del mismo nivel con la misma puerta de interior o el mismo
      nombre de local (esperan decision de Oriol; no es error todavia).
  (c) AVISO: edificios a bloques que tendrian sprite entero si se contara la familia entera de
      orientaciones (o0..o3) como un solo edificio (lo arregla H-CIU-2).

Planifica como Monumentos::planifica (src/Game/Monumentos.cpp) sin abrir el motor.
Solo stdlib. Uso: python3 tools/validar_edificis.py [--motor RUTA] [nivel ...]
Sale con 1 si hay errores.
"""
from __future__ import annotations

import collections
import json
import re
import sys
from pathlib import Path

MOTOR = Path(__file__).resolve().parent.parent
PROHIBIDOS = {"caserna", "fortaleza", "muralla", "muralla_rota", "piedra_publica"}
# Edificios publicos: pueden seguir con piedra_publica.
PUBLICOS = ("viejo_banco", "viejo_banos", "viejo_biblioteca", "viejo_universidad", "local_banc_", "local_banys_")
# Casas y tiendas por nombre de tile (prefijo).
CASAS_Y_TIENDAS = ("edifici_", "casa_alma", "viejo_casa_", "viejo_chabola", "viejo_tienda", "viejo_ropa",
                   "viejo_joyeria", "viejo_posada", "cabana_", "caserna_", "barraca_", "tenda_", "noble_", "local_")


def es_casa_o_tienda(tile: str) -> bool:
    if tile.startswith(PUBLICOS):
        return False
    return tile.startswith(CASAS_Y_TIENDAS) or tile == "casa_alma"


def materiales_de(d: dict) -> list[str]:
    m = d.get("material")
    return [m] if isinstance(m, str) else list(m or [])


class Datos:
    def __init__(self, motor: Path):
        self.motor = motor
        a = motor / "assets"
        idx = json.load(open(a / "textures" / "terreno_iso.json"))
        self.gid2n = {t["gid"]: t["nombre"] for t in idx["tiles"]}
        self.n2gid = {v: k for k, v in self.gid2n.items()}
        self.tipos = json.load(open(a / "textures" / "bloques.json"))["tipos"]
        self.defs = json.load(open(a / "textures" / "monumentos.json"))["monumentos"]
        self.familias = {m.group(1) for d in self.defs for m in [re.match(r"(.*)_o\d$", d["tiles"][0])] if m}
        self.nombres = {}
        try:
            for o in json.load(open(a / "objects" / "ciudad_objetos.json"))["objects"]:
                self.nombres[o["id"]] = o.get("name", o["id"])
        except OSError:
            pass


def leer_tmx(ruta: Path):
    s = ruta.read_text()
    w = int(re.search(r'<map [^>]*\swidth="(\d+)"', s).group(1))
    h = int(re.search(r'<map [^>]*\sheight="(\d+)"', s).group(1))
    m = re.search(r'<data encoding="csv">(.*?)</data>', s, re.S)
    v = [int(x) for x in m.group(1).replace("\n", "").split(",") if x.strip()]
    if len(v) != w * h:
        raise ValueError(f"{ruta.name}: {len(v)} casillas, se esperaban {w * h}")
    return w, h, v


def componentes(w, h, v, clave):
    """Componentes conexas (4 vecinos) de casillas con la misma clave (None = no cuenta)."""
    visto, salida = set(), []
    for y0 in range(h):
        for x0 in range(w):
            k = clave(v[y0 * w + x0])
            if k is None or (x0, y0) in visto:
                continue
            pila, celdas = [(x0, y0)], []
            visto.add((x0, y0))
            while pila:
                x, y = pila.pop()
                celdas.append((x, y))
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < w and 0 <= ny < h and (nx, ny) not in visto and clave(v[ny * w + nx]) == k:
                        visto.add((nx, ny))
                        pila.append((nx, ny))
            xs, ys = [c[0] for c in celdas], [c[1] for c in celdas]
            salida.append(dict(clave=k, cells=celdas, x0=min(xs), y0=min(ys), ancho=max(xs) - min(xs) + 1,
                               alto=max(ys) - min(ys) + 1, celdas=len(celdas)))
    return salida


def sobra_max(a, b):
    return 3 if a * b >= 30 else 1


def candidatas(defs, tile0, hh):
    """Definiciones de sprite que caben en la huella hh (como Monumentos::planifica)."""
    c = [d for d in defs if d["tiles"][0] == tile0 and d["ancho"] == hh["ancho"] and d["alto"] == hh["alto"]
         and hh["celdas"] * 10 >= d["ancho"] * d["alto"] * 7]
    if not c:
        c = [d for d in defs if d["tiles"][0] == tile0
             and 0 <= hh["ancho"] - d["ancho"] <= sobra_max(d["ancho"], d["alto"])
             and 0 <= hh["alto"] - d["alto"] <= sobra_max(d["ancho"], d["alto"])
             and hh["celdas"] * 10 >= hh["ancho"] * hh["alto"] * 7]
    return c


def cubiertas_por_sprite(D: Datos, w, h, v):
    cub = set()
    vistas = {}
    for d in D.defs:
        key = tuple(d["tiles"])
        if key not in vistas:
            gids = {D.n2gid[n] for n in key if n in D.n2gid}
            vistas[key] = componentes(w, h, v, lambda g, gids=gids: 1 if g in gids else None) if gids else []
    for key, lista in vistas.items():
        for hh in lista:
            if candidatas([d for d in D.defs if tuple(d["tiles"]) == key], key[0], hh):
                cub |= {(hh["x0"] + i, hh["y0"] + j) for j in range(hh["alto"]) for i in range(hh["ancho"])}
    return cub


def familia(D: Datos, nombre: str):
    m = re.match(r"(.*)_o(\d)$", nombre or "")
    return m.group(1) if m and m.group(1) in D.familias else None


def edificio_de_puerta(edificios, x, y):
    """Edificio mas cercano a la puerta (a 2 casillas como mucho: el umbral suele ser un hueco)."""
    mejor, dmin = None, 3
    for i, e in enumerate(edificios):
        for (cx, cy) in e["set"]:
            d = abs(cx - x) + abs(cy - y)
            if d < dmin:
                mejor, dmin = i, d
    return mejor


def analiza_nivel(D: Datos, nombre: str):
    """Devuelve (errores, avisos_b, avisos_c, n_casas_a_bloques)."""
    errs, av_b, av_c = [], [], []
    w, h, v = leer_tmx(D.motor / "assets" / "maps" / f"{nombre}.tmx")
    gid2n = D.gid2n
    cub = cubiertas_por_sprite(D, w, h, v)

    # (a) casas y tiendas que quedan a bloques
    es_edif = lambda g: g if (gid2n.get(g) in D.tipos) else None
    n_bloques = 0
    for c in componentes(w, h, [0 if (i % w, i // w) in cub else x for i, x in enumerate(v)], es_edif):
        tile = gid2n[c["clave"]]
        if not es_casa_o_tienda(tile):
            continue
        n_bloques += 1
        malos = sorted(set(materiales_de(D.tipos[tile])) & PROHIBIDOS)
        if malos:
            errs.append((f"assets/maps/{nombre}.tmx", f"{tile} ({c['x0']},{c['y0']}) {c['ancho']}x{c['alto']}",
                         "material", f"casa/tienda a bloques con material prohibido: {', '.join(malos)}"))

    # (c) a bloques pero con sprite si se cuenta la familia entera
    fams = componentes(w, h, v, lambda g: familia(D, gid2n.get(g, "")))
    for f in fams:
        sin = [c for c in f["cells"] if c not in cub]
        if not sin or not es_casa_o_tienda(f["clave"] + "_o0"):
            continue
        for o in range(4):
            if candidatas(D.defs, f"{f['clave']}_o{o}", f):
                av_c.append(f"{f['clave']} ({f['x0']},{f['y0']}) {f['ancho']}x{f['alto']}: {len(sin)} casilla(s) a bloques, cabria sprite _o{o}")
                break

    # (b) nombres e interiores repetidos entre edificios distintos
    lvl = json.load(open(D.motor / "assets" / "levels" / f"{nombre}.json"))
    edificios = [{"set": set(f["cells"]), "clave": f["clave"], "x0": f["x0"], "y0": f["y0"]} for f in fams]
    resto = componentes(w, h, v, lambda g: gid2n.get(g) if (gid2n.get(g) in D.tipos and not familia(D, gid2n.get(g))) else None)
    edificios += [{"set": set(c["cells"]), "clave": c["clave"], "x0": c["x0"], "y0": c["y0"]} for c in resto]
    por_interior, por_nombre = collections.defaultdict(set), collections.defaultdict(set)
    for o in lvl.get("objects", []):
        oid, tl = o.get("objectId", ""), o.get("targetLevel", "")
        if not oid.startswith("puerta_") or "/interior_" not in tl:
            continue
        e = edificio_de_puerta(edificios, o["position"]["x"], o["position"]["y"])
        if e is None:
            continue
        por_interior[tl.split("/")[-1]].add((e, oid, o["position"]["x"], o["position"]["y"]))
        por_nombre[D.nombres.get(oid, oid)].add((e, oid, o["position"]["x"], o["position"]["y"]))
    for tipo, grupos in (("interior", por_interior), ("nombre", por_nombre)):
        for clave, puertas in sorted(grupos.items()):
            if len({p[0] for p in puertas}) > 1:
                donde = ", ".join(f"{p[1]}@{p[2]},{p[3]}" for p in sorted(puertas, key=lambda p: (p[2], p[3])))
                av_b.append(f"{tipo} '{clave}' en {len({p[0] for p in puertas})} edificios distintos: {donde}")
    return errs, av_b, av_c, n_bloques


def tipos_prohibidos(D: Datos):
    errs = []
    for tile, d in D.tipos.items():
        if es_casa_o_tienda(tile):
            malos = sorted(set(materiales_de(d)) & PROHIBIDOS)
            if malos:
                errs.append(("assets/textures/bloques.json", tile, "material", f"tipo de casa/tienda con material prohibido: {', '.join(malos)}"))
    return errs


def niveles(motor: Path):
    mapas = motor / "assets" / "maps"
    return sorted(p.stem for p in mapas.glob("*.tmx") if p.stem.startswith(("ciudad_", "es_")) and (motor / "assets" / "levels" / f"{p.stem}.json").exists())


def comprueba(motor: Path = MOTOR, solo=None):
    """Devuelve (errores, avisos, n_niveles, n_casas_a_bloques); errores = (fichero, entidad, campo, problema)."""
    D = Datos(motor)
    errs, avisos, total = tipos_prohibidos(D), [], 0
    lista = solo or niveles(motor)
    for n in lista:
        e, b, c, nb = analiza_nivel(D, n)
        errs += e
        total += nb
        for t in b:
            avisos.append((f"assets/levels/{n}.json", "puertas", "identidad", t))
        if c:
            avisos.append((f"assets/maps/{n}.tmx", f"{len(c)} edificio(s)", "sprite por familia", "; ".join(c[:4]) + (" ..." if len(c) > 4 else "") + " (H-CIU-2)"))
    return errs, avisos, len(lista), total


def main() -> int:
    motor, solo, a = MOTOR, [], sys.argv[1:]
    if "--motor" in a:
        i = a.index("--motor")
        motor = Path(a[i + 1]).resolve()
        del a[i:i + 2]
    solo = a
    errs, avisos, nn, nb = comprueba(motor, solo or None)
    for f, e, c, t in avisos:
        print(f"AVISO {f} :: {e} :: {c} :: {t}")
    for f, e, c, t in errs:
        print(f"ERROR {f} :: {e} :: {c} :: {t}")
    print(f"validar_edificis: {nn} niveles, {nb} casas/tiendas a bloques, {len(errs)} error(es), {len(avisos)} aviso(s)")
    return 1 if errs else 0


if __name__ == "__main__":
    sys.exit(main())
