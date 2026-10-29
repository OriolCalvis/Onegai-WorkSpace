#!/usr/bin/env python3
"""AV15-03 - Quitar ruido de la ciudad: calles principales rectas y puertas libres.

Qué hace, sobre el nivel ya generado, sin tocar el suelo (TMX):
  1. Cada puerta (objeto con targetLevel) tiene libres sus celdas de acceso: lo que
     bloquea delante de una puerta se aparta (las termas llevaban un PNJ plantado en el
     unico acceso).
  2. Cada calle principal (BANDAS) tiene un carril RECTO de punta a punta sin nada que
     bloquee: se elige el carril de la banda con menos estorbos y se apartan los que hay.
Un estorbo se aparta a la celda libre mas cercana que no sea carril, acceso a puerta ni
paso entre mapas, de preferencia arrimada a una pared; si no hay sitio, un PNJ se queda
donde esta (y se avisa) y un objeto de decorado se quita (y se lista).

Es determinista (sin azar) e idempotente: pasarlo dos veces no cambia nada la segunda.
Nunca cierra un camino: comprueba al final que ninguna puerta alcanzable deja de serlo.

Uso:
    python3 tools/despejar_calles.py ciudad_centro             # solo cuenta lo que haria
    python3 tools/despejar_calles.py ciudad_centro --escribir  # escribe assets/levels/<nivel>.json (con "carriles")
    python3 tools/despejar_calles.py ciudad_centro --carriles  # guarda: los "carriles" del nivel, calculados y libres
    MG=<raiz> para usar otra raiz que la del propio script.
El generador (tools/gen_ciudad.py) lo llama con despejar_mapa() antes de escribir.
"""
import glob
import json
import os
import re
import sys
from collections import Counter, deque

BASE = os.environ.get("MG") or (os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
BASE = BASE.rstrip("/") + "/"
VECINOS = ((1, 0), (-1, 0), (0, 1), (0, -1))
CALLES = {"adoquin_limpio", "pavimento_antiguo", "adoquin_ajedrez", "adoquin_gastado", "marmol_real"}
# Suelo donde NO se coloca lo apartado: el puente y los umbrales de las puertas de la muralla.
NO_COLOCAR = {"tarima_madera", "adoquin_gastado"}
# Lo que es monumento o mobiliario con intencion: no se mueve (si estorba en un carril, se elige otro).
#   bandera: DECO-01 las puso a los lados de la puerta del castillo y de las puertas de la muralla.
FIJOS = ("pozo", "fuente", "fanal", "banco", "grua", "carro", "mesa", "puerta", "paso", "camino", "cartel", "bandera")
# AV16-09: los PNJ que las aventuras buscan por su id (assets/adventures/): ni se mueven
# ni se les pone nada encima, y el carril recto de cada calle los evita si puede.
CON_PAPEL = {"guardia", "anciana", "bardo", "mercader", "parroquiano_humilde", "sectario_perdido", "carterista"}

# (nombre, orientacion, desde, hasta, extension_desde, extension_hasta)
#   "v": carriles = columnas x en [desde, hasta], a lo largo de las filas [ext_desde, ext_hasta]
#   "h": carriles = filas    y en [desde, hasta], a lo largo de las columnas [ext_desde, ext_hasta]
# Salen de las llamadas de gen_ciudad.centro() (AV16-09): avenida_v(30, ancho=4),
# avenida_h(12, ancho=3), calles comerciales 18-20 y 26-28, y las dos calles de 2 del
# Casco (40-41 y 56-57) que antes eran callejones que tuercen.
#
# La avenida norte-sur va en tramos porque no es una calle de cuatro carriles de punta a punta:
# al sur del rio las casas del Casco se comen tres de los cuatro, y la plaza (filas 42-55, con el
# pozo en medio del eje) se compone a mano y queda fuera. Cada tramo tiene su propio carril; entre
# tramos se cambia de carril en la calle que los cruza.
BANDAS = {
    "ciudad_centro": [
        ("avenida norte-sur, norte", "v", 30, 33, 1, 27),
        ("avenida norte-sur, puente", "v", 30, 33, 28, 33),
        ("avenida norte-sur, Casco (norte)", "v", 30, 33, 34, 41),
        ("avenida norte-sur, Casco (sur)", "v", 30, 33, 56, 62),
        ("avenida de los monumentos", "h", 12, 14, 1, 62),
        ("calle comercial norte", "h", 18, 20, 1, 62),
        ("calle comercial sur", "h", 26, 28, 1, 62),
        ("calle del sur del Casco", "h", 40, 42, 1, 62),
        ("calle de la herreria", "h", 60, 61, 1, 62),
    ],
}


# ------------------------------------------------------------------ carga
def catalogo(base=BASE):
    """Los catalogos del proyecto, como los carga el juego (assets/proyectos/boundington.json)."""
    cat = {}
    try:
        lista = json.load(open(base + "assets/proyectos/boundington.json"))["catalogs"]
    except (OSError, ValueError, KeyError):
        lista = [os.path.basename(p) for p in sorted(glob.glob(base + "assets/objects/*.json"))]
    for nombre in lista:
        try:
            for o in json.load(open(base + "assets/objects/" + nombre)).get("objects", []):
                cat[o["id"]] = o
        except (OSError, ValueError):
            pass
    return cat


def cargar_nivel(nombre, base=BASE):
    lvl = json.load(open(base + f"assets/levels/{nombre}.json"))
    tmx = open(base + lvl["map"]).read()
    cab = tmx.split("<tileset")[0]
    w, h = (int(re.search(rf'\b{k}="(\d+)"', cab).group(1)) for k in ("width", "height"))
    colision = {int(i) + 1 for i in re.findall(r'<tile id="(\d+)">\s*<properties>\s*<property name="collision"', tmx)}
    datos = re.search(r'<data encoding="csv">(.*?)</data>', tmx, re.S).group(1)
    g = [[int(v) for v in fila.split(",") if v.strip()] for fila in datos.strip().splitlines()]
    assert len(g) == h and all(len(f) == w for f in g), "rejilla distinta de width x height"
    nombres = {t["gid"]: t["nombre"] for t in json.load(open(base + "assets/textures/terreno_iso.json"))["tiles"]}
    return lvl, g, colision, nombres


# ------------------------------------------------------------------ nucleo
def despejar(nombre, g, colision, nombres, objetos, cat, inicio, bandas=None):
    """Aparta estorbos. `objetos` es la lista del nivel y se modifica en su sitio.
    Devuelve el informe. Lanza AssertionError si el resultado cerrase algun camino."""
    bandas = BANDAS.get(nombre, []) if bandas is None else bandas
    h, w = len(g), len(g[0])
    pos = lambda o: (o["position"]["x"], o["position"]["y"])
    anda = lambda x, y: 0 <= x < w and 0 <= y < h and g[y][x] not in colision
    nom = lambda x, y: nombres.get(g[y][x], "")
    d = lambda o: cat.get(o["objectId"], {})
    bloquea = lambda o: bool(d(o).get("blocksMovement")) and not d(o).get("flotante")
    es_npc = lambda o: d(o).get("category") == "npc"
    es_fijo = lambda o: o["objectId"].startswith(FIJOS) or o["objectId"] in CON_PAPEL

    def ocupacion():
        return {pos(o): o for o in objetos}

    def alcanzables():
        occ = {pos(o) for o in objetos if bloquea(o)}
        dist, q = {inicio}, deque([inicio])
        while q:
            x, y = q.popleft()
            for dx, dy in VECINOS:
                n = (x + dx, y + dy)
                if n not in dist and anda(*n) and n not in occ:
                    dist.add(n)
                    q.append(n)
        return dist

    transiciones = [o for o in objetos if o.get("targetLevel")]
    celdas_puerta = {pos(o) for o in transiciones}
    accesos = {}
    for o in transiciones:
        x, y = pos(o)
        accesos[pos(o)] = [(x + dx, y + dy) for dx, dy in VECINOS if anda(x + dx, y + dy) and (x + dx, y + dy) not in celdas_puerta]
    cerca_de_paso = {(x + dx, y + dy) for o in transiciones if o["objectId"].startswith(("paso_", "camino_"))
                     for (x, y) in [pos(o)] for dx in (-1, 0, 1) for dy in (-1, 0, 1)}
    antes = alcanzables()
    puertas_antes = {p for p in celdas_puerta if p in antes}

    # --- carriles: el de cada banda con menos estorbos (sin monumentos) que sea todo andable
    carriles, informe_bandas = [], []
    occ = ocupacion()
    for (bn, ori, a0, a1, e0, e1) in bandas:
        cands = []
        for a in range(a0, a1 + 1):
            celdas = [(a, e) for e in range(e0, e1 + 1)] if ori == "v" else [(e, a) for e in range(e0, e1 + 1)]
            if not all(anda(*c) for c in celdas):
                continue
            ob = [occ[c] for c in celdas if c in occ and bloquea(occ[c])]
            # El carril se elige con lo que NUNCA se mueve (monumentos y PNJ con papel) y
            # con la distancia al eje: si mandara el numero de estorbos moviles, la
            # eleccion cambiaria entre pasadas y los "carriles" declarados no casarian.
            cands.append((sum(1 for o in ob if es_fijo(o)), abs(a - (a0 + a1) / 2), len(ob), a, celdas))
        if not cands:
            informe_bandas.append({"banda": bn, "carril": None, "motivo": "ningun carril es todo andable"})
            continue
        cands.sort(key=lambda c: c[:4])
        fijos, n, _, a, celdas = cands[0]
        carriles.append((bn, a, celdas))
        informe_bandas.append({"banda": bn, "carril": a, "estorbos": n, "monumentos_en_el_carril": fijos})
    celdas_carril = {c for _, _, cs in carriles for c in cs}
    reservadas = celdas_carril | celdas_puerta | {c for cs in accesos.values() for c in cs} | cerca_de_paso

    # --- que hay que apartar, en orden estable: primero accesos de puerta, luego carriles
    por_apartar, vistos = [], set()
    for p, cs in sorted(accesos.items()):
        for c in cs:
            if c in occ and bloquea(occ[c]) and id(occ[c]) not in vistos:
                vistos.add(id(occ[c]))
                por_apartar.append((occ[c], "acceso de " + (occ[p]["objectId"] if p in occ else "una puerta")))
    for bn, a, celdas in carriles:
        for c in celdas:
            if c in occ and bloquea(occ[c]) and id(occ[c]) not in vistos:
                vistos.add(id(occ[c]))
                por_apartar.append((occ[c], f"carril de {bn}"))

    movidos, quitados, quietos = [], [], []
    ocupadas_ahora = {pos(o) for o in objetos}

    def destino(o, radio):
        x0, y0 = pos(o)
        mejor = None
        for yy in range(y0 - radio, y0 + radio + 1):
            for xx in range(x0 - radio, x0 + radio + 1):
                c = (xx, yy)
                if c == (x0, y0) or not anda(xx, yy) or c in ocupadas_ahora or c in reservadas:
                    continue
                if nom(xx, yy) in NO_COLOCAR:
                    continue
                paredes = sum(1 for dx, dy in VECINOS if not anda(xx + dx, yy + dy))
                r = max(abs(xx - x0), abs(yy - y0))
                clave = (r + (0 if paredes else 2), yy, xx)
                if mejor is None or clave < mejor[0]:
                    mejor = (clave, c)
        return None if mejor is None else mejor[1]

    for o, motivo in por_apartar:
        if es_fijo(o):
            quietos.append({"objeto": o["objectId"], "pos": pos(o), "motivo": motivo + " (monumento: no se mueve)"})
            continue
        c = destino(o, 6) or (destino(o, 14) if es_npc(o) else None)
        origen = pos(o)
        if c is None:
            if es_npc(o):
                quietos.append({"objeto": o["objectId"], "pos": origen, "motivo": motivo + " (sin sitio donde ponerlo)"})
            else:
                objetos.remove(o)
                ocupadas_ahora.discard(origen)
                quitados.append({"objeto": o["objectId"], "pos": origen, "motivo": motivo})
            continue
        ocupadas_ahora.discard(origen)
        ocupadas_ahora.add(c)
        o["position"] = {"x": c[0], "y": c[1]}
        movidos.append({"objeto": o["objectId"], "de": origen, "a": c, "motivo": motivo})

    # --- garantias
    despues = alcanzables()
    perdidas = {p for p in puertas_antes if p not in despues}
    assert not perdidas, f"{nombre}: el cambio cerraria el camino a {sorted(perdidas)}"
    sobre = [o["objectId"] for o in objetos if bloquea(o) and not anda(*pos(o))]
    assert not sobre, f"{nombre}: quedan objetos que bloquean sobre colision {sobre}"
    occ2 = {pos(o): o for o in objetos}
    for bn, a, celdas in carriles:
        malos = [c for c in celdas if c in occ2 and bloquea(occ2[c]) and not es_fijo(occ2[c]) and not es_npc(occ2[c])]
        assert not malos, f"{nombre}: el carril de {bn} sigue con estorbos {malos}"
    # AV15-03b: las casillas de los carriles rectos, para el nivel ("carriles": [[x, y], ...]). Contrato con
    # pnj_motor (RutinasPnj): un PNJ puede cruzar un carril, pero no se para en el.
    lista_carriles = sorted({c for _, _, cs in carriles for c in cs}, key=lambda c: (c[1], c[0]))
    return {"nivel": nombre, "bandas": informe_bandas, "movidos": movidos, "quitados": quitados, "quietos": quietos,
            "puertas_ahora_alcanzables": len({p for p in celdas_puerta if p in despues}) - len(puertas_antes),
            "carriles": [list(c) for c in lista_carriles]}


def puertas_a_calle(nombre, base=BASE):
    """AV16-09: cada salida mira a una calle de 2 o mas casillas, con su casilla de
    delante libre y visible desde la calle: una acera recta de al menos 3 celdas mas
    a lo largo de la calle, sin nada que bloquee ni una casa que la tape.
    Devuelve la lista de fallos (vacia si esta bien). Solo lectura."""
    lvl, g, colision, nombres = cargar_nivel(nombre, base)
    cat = catalogo(base)
    h, w = len(g), len(g[0])
    anda = lambda x, y: 0 <= x < w and 0 <= y < h and g[y][x] not in colision
    occ = {(o["position"]["x"], o["position"]["y"]): o["objectId"] for o in lvl["objects"]
           if cat.get(o["objectId"], {}).get("blocksMovement") and not cat.get(o["objectId"], {}).get("flotante")}
    fallos = []
    for o in lvl["objects"]:
        if not o.get("targetLevel"):
            continue
        x, y = o["position"]["x"], o["position"]["y"]
        mejor = None
        for dx, dy in VECINOS:                       # la casilla de delante, libre
            fx, fy = x + dx, y + dy
            if not anda(fx, fy) or (fx, fy) in occ:
                continue
            if dy:                                   # mira al norte o al sur: la calle corre en horizontal
                ancho, k = 0, fy
                while anda(fx, k):
                    ancho, k = ancho + 1, k + 1
                k = fy - 1
                while anda(fx, k):
                    ancho, k = ancho + 1, k - 1
                eje = "h"
            else:                                    # mira al este o al oeste: la calle corre en vertical
                ancho, k = 0, fx
                while anda(k, fy):
                    ancho, k = ancho + 1, k + 1
                k = fx - 1
                while anda(k, fy):
                    ancho, k = ancho + 1, k - 1
                eje = "v"
            if mejor is None or ancho > mejor[0]:
                mejor = (ancho, fx, fy, eje)
        if mejor is None:
            fallos.append(f"{nombre}: {o['objectId']} en ({x}, {y}) no tiene ninguna casilla delante andable")
            continue
        ancho, fx, fy, eje = mejor
        if ancho < 2:
            fallos.append(f"{nombre}: {o['objectId']} en ({x}, {y}) mira a una calle de 1 casilla")
            continue
        # Visible: desde alguna fila (o columna) de la banda de la calle se ve la
        # puerta en linea recta: la banda es andable hasta la casilla de delante y
        # la acera sigue recta y despejada al menos 3 casillas mas.
        banda = []
        if eje == "h":
            k = fy
            while anda(fx, k):
                banda.append(k)
                k += 1
            k = fy - 1
            while anda(fx, k):
                banda.append(k)
                k -= 1
        else:
            k = fx
            while anda(k, fy):
                banda.append(k)
                k += 1
            k = fx - 1
            while anda(k, fy):
                banda.append(k)
                k -= 1
        vista = 0
        for r in banda:
            for s in (1, -1):
                n, k = 0, 1
                while True:
                    cx, cy = (fx + s * k, r) if eje == "h" else (r, fy + s * k)
                    if not anda(cx, cy) or (cx, cy) in occ:
                        break
                    n, k = n + 1, k + 1
                vista = max(vista, n)
        if vista < 3:
            fallos.append(f"{nombre}: {o['objectId']} en ({x}, {y}) no se ve desde la calle "
                          f"(acera recta de {vista} casillas)")
    return fallos


def comprobar_carriles(nombre, base=BASE):
    """AV15-03b: los "carriles" que declara el nivel son los que calcula despejar() y estan libres (tile andable y
    sin nada que bloquee, PNJ incluidos). Devuelve la lista de fallos (vacia si esta bien)."""
    import copy
    lvl, g, colision, nombres = cargar_nivel(nombre, base)
    if nombre not in BANDAS:
        return []
    declarados = lvl.get("carriles")
    if declarados is None:
        return [f"{nombre}: el nivel no declara \"carriles\""]
    cat = catalogo(base)
    ps = (lvl["playerStart"]["x"], lvl["playerStart"]["y"])
    inf = despejar(nombre, g, colision, nombres, copy.deepcopy(lvl["objects"]), cat, ps)
    fallos = []
    if sorted(map(tuple, declarados)) != sorted(map(tuple, inf["carriles"])):
        fallos.append(f"{nombre}: los carriles declarados ({len(declarados)}) no son los calculados ({len(inf['carriles'])})")
    if inf["movidos"] or inf["quitados"]:
        fallos.append(f"{nombre}: el nivel no esta despejado ({len(inf['movidos'])} por mover, {len(inf['quitados'])} por quitar)")
    occ = {(o["position"]["x"], o["position"]["y"]): o["objectId"] for o in lvl["objects"]
           if cat.get(o["objectId"], {}).get("blocksMovement") and not cat.get(o["objectId"], {}).get("flotante")}
    for x, y in declarados:
        if g[y][x] in colision:
            fallos.append(f"{nombre}: carril ({x}, {y}) sobre colision")
        elif (x, y) in occ:
            fallos.append(f"{nombre}: carril ({x}, {y}) ocupado por {occ[(x, y)]}")
    return fallos


def despejar_mapa(nombre, g, colision_gids, nombres_por_gid, objetos, inicio):
    """Entrada para el generador: lo llama escribir() con el mapa definitivo."""
    if nombre not in BANDAS:
        return None
    return despejar(nombre, g, colision_gids, nombres_por_gid, objetos, catalogo(), inicio)


# ------------------------------------------------------------------ linea de ordenes
def texto(inf):
    out = [f"== {inf['nivel']} =="]
    for b in inf["bandas"]:
        out.append(f"  banda {b['banda']}: carril {b.get('carril')}  " +
                   (f"estorbos {b['estorbos']}, monumentos {b['monumentos_en_el_carril']}" if b.get("carril") is not None else b["motivo"]))
    out.append(f"  se mueven {len(inf['movidos'])}, se quitan {len(inf['quitados'])}, se quedan {len(inf['quietos'])}")
    for m in inf["movidos"]:
        out.append(f"    mover  {m['objeto']:<32} {tuple(m['de'])} -> {tuple(m['a'])}   [{m['motivo']}]")
    for m in inf["quitados"]:
        out.append(f"    QUITAR {m['objeto']:<32} {tuple(m['pos'])}   [{m['motivo']}]")
    for m in inf["quietos"]:
        out.append(f"    QUIETO {m['objeto']:<32} {tuple(m['pos'])}   [{m['motivo']}]")
    return "\n".join(out)


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if not args:
        sys.exit(__doc__)
    nivel = args[0]
    lvl, g, colision, nombres = cargar_nivel(nivel)
    ps = (lvl["playerStart"]["x"], lvl["playerStart"]["y"])
    inf = despejar(nivel, g, colision, nombres, lvl["objects"], catalogo(), ps)
    print(texto(inf))
    if "--carriles" in sys.argv:                       # AV15-03b: guarda del contrato con pnj_motor
        fallos = comprobar_carriles(nivel)
        print("\n".join(fallos) if fallos else f"  carriles: bien ({len(lvl.get('carriles', []))} casillas, libres y calculadas)")
        sys.exit(1 if fallos else 0)
    if "--puertas" in sys.argv:                        # AV16-09: cada puerta mira a una calle de 2 o mas y se ve
        fallos = puertas_a_calle(nivel)
        print("\n".join(fallos) if fallos else "  puertas: bien (todas miran a una calle de 2 o mas, con la "
                                               "casilla de delante libre y visibles desde la calle)")
        sys.exit(1 if fallos else 0)
    if "--escribir" in sys.argv:
        lvl["carriles"] = inf["carriles"]
        ruta = BASE + f"assets/levels/{nivel}.json"
        open(ruta, "w").write(json.dumps(lvl, indent=1, ensure_ascii=False) + "\n")
        print(f"  escrito {ruta}")
    else:
        print("  (nada escrito: usa --escribir)")
