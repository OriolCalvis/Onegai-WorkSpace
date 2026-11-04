#!/usr/bin/env python3
"""Una sola fuente para las laminas ya dibujadas (CAR-4a / E1): las filas de `personajes.json: horneados`
(razas_andando, actores_capas, personajes_historia, pnj_*) salen SIEMPRE de las capas.

Este script las rehace solo donde hace falta y avisa rapido cuando una capa cambia y la lamina queda vieja.
Es el unico camino permitido para rehacer filas de `pnj_*` (da los mismos pixeles que MontajePersonajes.cpp);
nunca con `gen_pnj.py` ni con `gen_personajes.py --clasico`. No importa ningun generador (solo Pillow + numpy).

Uso (desde MotorGraphico/):
  python3 tools/laminas_desde_capas.py                      # --comprobar: filas desfasadas, sale 1 si hay alguna
  python3 tools/laminas_desde_capas.py --registrar          # escribe las huellas del estado actual
  python3 tools/laminas_desde_capas.py --reconstruir        # rehace solo las filas desfasadas
  python3 tools/laminas_desde_capas.py --reconstruir --todo [--hoja razas_andando]
  python3 tools/laminas_desde_capas.py --a-fondo [--hoja X] [--muestra N]   # compara pixeles con el PNG
`--tex DIR` cambia la carpeta de texturas (para probar sobre una copia). Huellas: assets/textures/laminas_huellas.json.
"""
import argparse, collections, hashlib, json, sys, time
from pathlib import Path

import numpy as np
from PIL import Image

L = 64
KF = 40
HUELLAS = "laminas_huellas.json"
RUMBO_DIR = {0: "arriba", 1: "abajo", 2: "abajo", 3: "arriba"}  # arriba, derecha, abajo, izquierda
RANURAS = ["cabeza", "torso", "piernas", "pies", "capa", "arma", "mano_izq", "alas", "cola", "cuernos"]
ROPA = {"torso", "piernas", "pies", "capa"}
TEX_DEFECTO = Path(__file__).resolve().parent.parent / "assets" / "textures"


def rumbo(f):
    if f > 24:
        return (f - 24 - 1) // 4
    return (f - 1) // 6


def vista(f):
    return RUMBO_DIR[rumbo(f)]


class Montaje:
    def __init__(s, tex, presupuesto=400*2**20):
        s.tex = Path(tex); s.m = json.load(open(s.tex/"personajes.json"))
        s.cache = collections.OrderedDict(); s.bytes = 0; s.pres = presupuesto
        m = s.m
        s.razas = {r["id"]: r for r in m["razas"]["lista"]}
        s.atl_raza = m["razas"]["atlas"]
        s.orden = m["orden"]; s.pal = {k: [tuple(int(h[i:i+2],16) for i in (0,2,4)) for h in v.split()] for k,v in m["paletas"].items()}
        s.grupo = {n: m[n] for n in ("cuerpos","ojos","pelo","barba","marcas") if n in m}
        s.equipo = m["equipo"]; s.pieles = m["pieles"]
        for r in s.razas.values():
            capas_lisas = (r["liso"]==True) if "liso" in r and isinstance(r["liso"],bool) else (r["capas"]==["cuerpo"])
            r["_liso"] = capas_lisas and s.buscar(s.grupo["cuerpos"], r.get("piel")) is not None
    def buscar(s, g, id_):
        if id_ is None: return None
        base = id_.split(":")[0]
        k = "filas" if "filas" in g else "piezas"
        for f in g[k]:
            if f["id"] == base: return f
        return None
    def atlas(s, ruta):
        if ruta in s.cache:
            s.cache.move_to_end(ruta); return s.cache[ruta]
        a = np.asarray(Image.open(s.tex/ruta).convert("RGBA")); s.cache[ruta] = a; s.bytes += a.nbytes
        while s.bytes > s.pres and len(s.cache) > 1:
            _, v = s.cache.popitem(last=False); s.bytes -= v.nbytes
        return a
    def cambios(s, pieza_tenida):
        out = []
        for o, d in pieza_tenida:
            if o not in s.pal or d not in s.pal: raise KeyError(f"paleta {o}/{d}")
            for t in range(5):
                de, a = s.pal[o][t], s.pal[d][t]
                if not any(c[0]==de for c in out) and de != a: out.append((de, a))
        return out
    @staticmethod
    def sep(v):
        p, _, r = v.partition(":"); t = []
        if r:
            for par in r.split(","):
                if ">" in par and not par.startswith(">") and not par.endswith(">"):
                    a, b = par.split(">", 1); t.append((a, b))
        return p, t
    def capas(s, c):
        r = s.razas[c["raza"]]
        talla_r = r["talla"]; talla = c.get("talla") or talla_r
        talla_ropa = r.get("tallaRopa", talla_r) if talla == talla_r else talla
        ojos = c.get("ojos") or r["ojos"]
        piel_c = c.get("piel")
        otra_talla = bool(c.get("talla")) and c["talla"] != talla_r
        otra_piel = bool(piel_c) and piel_c != r.get("piel") and piel_c != r["id"]
        por_atlas = otra_talla or (otra_piel and r["_liso"] and s.buscar(s.grupo["cuerpos"], piel_c) is not None)
        pi = s.pieles.get(talla)
        por_tinte = (not por_atlas) and otra_piel and pi and r["id"] in pi["opciones"] and piel_c in pi["opciones"]
        if otra_piel and not por_atlas and not por_tinte: raise ValueError("piel no admitida")
        out = []; ojos_cuerpo = None
        if por_atlas:
            if not r["_liso"]: raise ValueError("raza no admite otra piel/talla")
            piel = piel_c or r["piel"]; f = s.buscar(s.grupo["cuerpos"], piel)
            out.append(dict(ranura="cuerpo", atlas=s.grupo["cuerpos"]["atlas"][talla], fila=f["fila"], cam=[], orden={}))
            ojos_cuerpo = "azul"
        else:
            quitar = c.get("quitar_capas") or []
            equipo_c = c.get("equipo") or {}
            for capa in r["capas"]:
                if capa in quitar or (capa in ("alas","cola","cuernos") and equipo_c.get(capa)): continue
                d = dict(ranura=capa, atlas=s.atl_raza[capa], fila=r["fila"], cam=[], orden={})
                if por_tinte and capa in ("cuerpo","orejas","morro"):
                    de, ha = pi["opciones"][r["id"]]["colores"], pi["opciones"][piel_c]["colores"]
                    for i in range(len(de)):
                        t = tuple(de[i])
                        if not any(x[0]==t for x in d["cam"]) and de[i] != ha[i]: d["cam"].append((t, tuple(ha[i])))
                out.append(d)
            ojos_cuerpo = r["ojos"]
        if ojos != ojos_cuerpo:
            # CAR-2: la capa de ojos de la raza (su cabeza va aparte); con cuerpo liso, la de la talla; sin capa, nada
            f = s.buscar(s.grupo["ojos"], ojos)
            ruta = s.grupo["ojos"]["atlas"].get(talla if por_atlas else r["id"])
            if ruta: out.insert(min(1,len(out)), dict(ranura="ojos", atlas=ruta, fila=f["fila"], cam=[], orden={}))
        for mk in c.get("marcas") or []:
            if not mk: continue
            p, t = s.sep(mk); f = s.buscar(s.grupo["marcas"], p)
            out.append(dict(ranura="marcas", atlas=s.grupo["marcas"]["atlas"][talla], fila=f["fila"], cam=s.cambios(t), orden={}))
        def pob(g, capa, par):
            estilo, color = par
            f = s.buscar(s.grupo[g], f"{estilo}/{color}")
            if f: out.append(dict(ranura=capa, atlas=s.grupo[g]["atlas"][talla], fila=f["fila"], cam=[], orden={})); return
            f = s.buscar(s.grupo[g], f"{estilo}/pelo_castano")
            if f is None or color not in s.pal: raise ValueError(f"no hay {capa} {estilo}/{color}")
            out.append(dict(ranura=capa, atlas=s.grupo[g]["atlas"][talla], fila=f["fila"], cam=s.cambios([("pelo_castano", color)]), orden={}))
        def par_de(clave):
            if clave in c:
                v = c[clave]; return tuple(v[:2]) if isinstance(v,list) and len(v)>=2 and v[0] else None
            ra = r.get(clave); 
            if clave == "pelo" and "pelo" in r.get("sin", []): return None
            return tuple(ra[:2]) if isinstance(ra,list) and len(ra)>=2 and ra[0] else None
        bp = par_de("barba")
        if bp: pob("barba","barba",bp)
        puesto = dict(c.get("equipo") or {})
        cab = puesto.get("cabeza")
        tapa = bool(cab) and "cabeza" in s.equipo and s.sep(cab)[0] in s.equipo["cabeza"].get("ocultaPelo", [])
        pp = par_de("pelo")
        if pp and not tapa: pob("pelo","pelo",pp)
        for ra in RANURAS:
            pz = puesto.get(ra)
            if not pz or ra in r.get("sin", []): continue
            g = s.equipo[ra]; f = s.buscar(g, pz)
            if f is None: raise ValueError(f"no hay pieza {ra} {pz}")
            t = talla_ropa if ra in ROPA else talla
            p, ti = s.sep(pz)
            o = {}
            for k, z in (f.get("orden") or {}).items():
                if "/" not in k: o[k] = z
            for k, z in (f.get("orden") or {}).items():
                if k.startswith(t+"/"): o[k[len(t)+1:]] = z
            out.append(dict(ranura=ra, atlas=g["atlas"][t], fila=f["fila"], cam=s.cambios(ti), orden=o))
        return out
    def montar(s, c):
        cap = s.capas(c)
        at = [s.atlas(x["atlas"]) for x in cap]
        hoja = np.zeros((L, L*KF, 4), np.uint8)
        for fr in range(1, KF+1):
            v = vista(fr); pila = []
            for i, x in enumerate(cap):
                if f"f{fr}" in x["orden"]: z = x["orden"][f"f{fr}"]
                elif v in x["orden"]: z = x["orden"][v]
                else: z = s.orden[x["ranura"]].get(v, 0)
                pila.append((z, i))
            pila.sort()
            x0 = (fr-1)*L; dest = Image.new("RGBA", (L, L))
            for z, i in pila:
                y0 = (cap[i]["fila"]-1)*L
                px = at[i][y0:y0+L, x0:x0+L]
                if cap[i]["cam"]:
                    px = px.copy(); vis = px[...,3] != 0; hecho = np.zeros(vis.shape, bool)
                    for de, a in cap[i]["cam"]:
                        m = vis & ~hecho & (px[...,0]==de[0]) & (px[...,1]==de[1]) & (px[...,2]==de[2])
                        px[m, :3] = a; hecho |= m
                dest = Image.alpha_composite(dest, Image.fromarray(np.ascontiguousarray(px)))
            hoja[:, x0:x0+L] = np.asarray(dest)
        return hoja


def huella(M, comp, cache):
    """Huella de una fila: composicion + contenido de cada fila de capa + tintes + ordenes de dibujo."""
    cap = M.capas(comp)
    h = hashlib.sha1(json.dumps(comp, sort_keys=True).encode())
    for x in cap:
        k = (x["atlas"], x["fila"])
        if k not in cache:
            y0 = (x["fila"] - 1) * L
            cache[k] = hashlib.sha1(np.ascontiguousarray(M.atlas(x["atlas"])[y0:y0 + L]).tobytes()).hexdigest()[:12]
        base = M.orden[x["ranura"]]
        h.update(json.dumps([x["ranura"], x["atlas"], x["fila"], cache[k], x["cam"], x["orden"], base],
                            sort_keys=True, default=list).encode())
    return h.hexdigest()[:16]


def leer_hojas(tex, solo=None):
    """[(nombre, json de la lamina)] de personajes.json: horneados; `solo` filtra por nombre."""
    man = json.load(open(tex / "personajes.json"))
    out = []
    for hj in man["horneados"]:
        nombre = hj[:-5] if hj.endswith(".json") else hj
        if solo and nombre not in solo:
            continue
        out.append((nombre, json.load(open(tex / hj))))
    if solo:
        faltan = set(solo) - {n for n, _ in out}
        if faltan:
            sys.exit(f"hoja desconocida: {sorted(faltan)}")
    return out


def huellas_actuales(M, hojas, claves=None):
    """{'hoja/id': huella}; `claves` limita a esas filas."""
    cache, out = {}, {}
    for nombre, j in hojas:
        for a in j["actores"]:
            k = f"{nombre}/{a['id']}"
            if claves is None or k in claves:
                out[k] = huella(M, a["composicion"], cache)
    return out


CLAVE_CAPAS = "_capas"  # {hoja: hash rapido}: capas + JSON de la lamina + sha1 de su PNG
CLAVE_PNG = "_png"      # {hoja: sha1 del PNG} tal como lo dejo el ultimo --registrar / --reconstruir


def sha_png(tex, j):
    try:
        return hashlib.sha1((tex / j["image"]).read_bytes()).hexdigest()[:16]
    except OSError:
        return None


def hash_capas(tex, nombre, j):
    """Hash rapido de una lamina: bytes de personajes.json y de las capas (personajes/), el JSON de la lamina
    y el sha1 de su PNG. Si no cambia desde el ultimo registro, ninguna fila puede haberse quedado vieja ni
    la lamina haberse tocado a mano."""
    h = hashlib.sha1()
    ficheros = [tex / "personajes.json", tex / f"{nombre}.json"]
    ficheros += sorted((tex / "personajes").rglob("*.png")) + sorted((tex / "personajes").rglob("*.json"))
    for f in ficheros:
        h.update(str(f.relative_to(tex)).encode())
        try:
            h.update(f.read_bytes())
        except OSError:
            h.update(b"?")
    h.update(str(sha_png(tex, j)).encode())
    return h.hexdigest()[:16]


def anotar(tex, reg, hojas):
    """Pone al dia _capas y _png de estas laminas en `reg`."""
    c, p = (dict(x) if isinstance(x, dict) else {} for x in (reg.get(CLAVE_CAPAS), reg.get(CLAVE_PNG)))
    for nombre, j in hojas:
        c[nombre], p[nombre] = hash_capas(tex, nombre, j), sha_png(tex, j)
    reg[CLAVE_CAPAS], reg[CLAVE_PNG] = c, p


def png_tocados(tex, reg, hojas):
    """Laminas cuyo PNG ya no es el que se registro."""
    p = reg.get(CLAVE_PNG)
    p = p if isinstance(p, dict) else {}
    return [n for n, j in hojas if p.get(n) != sha_png(tex, j)]


def leer_huellas(tex):
    try:
        return json.load(open(tex / HUELLAS))
    except (OSError, ValueError):
        return {}


def guardar_huellas(tex, d):
    with open(tex / HUELLAS, "w") as f:
        json.dump(dict(sorted(d.items())), f, indent=0, sort_keys=True)
        f.write("\n")


def desfasadas(tex, M, hojas):
    """Filas ('hoja:id') cuya huella no coincide con la registrada (o que no tienen)."""
    reg = leer_huellas(tex)
    ahora = huellas_actuales(M, hojas)
    reg = {k: v for k, v in reg.items() if not k.startswith("_")}
    return [k.replace("/", ":", 1) for k, v in ahora.items() if reg.get(k) != v], ahora, reg


def registrar(salida=None):
    """Escribe las huellas del estado actual (la llama gen_personajes.py al final)."""
    tex = Path(salida) if salida else TEX_DEFECTO
    M = Montaje(tex)
    hojas = leer_hojas(tex)
    d = huellas_actuales(M, hojas)
    anotar(tex, d, hojas)
    guardar_huellas(tex, d)


def cargar_png(tex, j):
    return np.array(Image.open(tex / j["image"]).convert("RGBA"))


def reconstruir(tex, M, hojas, filas):
    """Rehace las filas indicadas ('hoja:id') y reescribe los PNG tocados."""
    por_hoja = collections.defaultdict(set)
    for k in filas:
        h, _, i = k.partition(":")
        por_hoja[h].add(i)
    for nombre, j in hojas:
        ids = por_hoja.get(nombre)
        if not ids:
            continue
        img = cargar_png(tex, j)
        for a in j["actores"]:
            if a["id"] in ids:
                y0 = (a["fila"] - 1) * L
                img[y0:y0 + L] = M.montar(a["composicion"])
        Image.fromarray(img, "RGBA").save(tex / j["image"], optimize=True)
        print(f"{nombre}: {len(ids)} filas rehechas")


def a_fondo(tex, M, hojas, muestra):
    """Rehace cada fila y la compara pixel a pixel con el PNG. Devuelve las que difieren."""
    mal, total = [], 0
    for nombre, j in hojas:
        img = cargar_png(tex, j)
        acts = j["actores"]
        if muestra and len(acts) > muestra:
            acts = [acts[i * len(acts) // muestra] for i in range(muestra)]
        ig = 0
        for a in acts:
            y0 = (a["fila"] - 1) * L
            ref = img[y0:y0 + L]
            hoja = M.montar(a["composicion"])
            if hoja.shape == ref.shape and (hoja == ref).all():
                ig += 1
            else:
                mal.append(f"{nombre}:{a['id']}")
        total += len(acts)
        print(f"{nombre}: {ig}/{len(acts)} iguales", flush=True)
    return mal, total


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--tex", default=str(TEX_DEFECTO))
    ap.add_argument("--hoja", action="append", help="limita a esta lamina (repetible)")
    g = ap.add_mutually_exclusive_group()
    g.add_argument("--comprobar", action="store_true")
    g.add_argument("--reconstruir", action="store_true")
    g.add_argument("--a-fondo", action="store_true", dest="fondo")
    g.add_argument("--registrar", action="store_true")
    ap.add_argument("--todo", action="store_true", help="con --reconstruir: todas las filas, no solo las desfasadas")
    ap.add_argument("--rapido", action="store_true",
                    help="con --comprobar: si el hash de las capas no ha cambiado desde --registrar, sale 0 sin calcular")
    ap.add_argument("--muestra", type=int, default=0, help="con --a-fondo: N filas por lamina")
    a = ap.parse_args()
    tex = Path(a.tex)
    t0 = time.time()
    hojas = leer_hojas(tex, a.hoja)
    if a.rapido and not (a.registrar or a.fondo or a.reconstruir):
        c = leer_huellas(tex).get(CLAVE_CAPAS)
        c = c if isinstance(c, dict) else {}
        if all(c.get(n) == hash_capas(tex, n, j) for n, j in hojas):
            print(f"laminas_desde_capas: capas y laminas sin cambios desde el ultimo registro ({time.time() - t0:.1f}s)")
            return 0
    M = Montaje(tex)

    if a.registrar:
        reg = leer_huellas(tex)
        reg.update(huellas_actuales(M, hojas))
        anotar(tex, reg, hojas)
        guardar_huellas(tex, reg)
        print(f"huellas registradas: {len(reg)} ({time.time() - t0:.0f}s)")
        return 0
    if a.fondo:
        mal, total = a_fondo(tex, M, hojas, a.muestra)
        print(f"a fondo: {total - len(mal)}/{total} iguales ({time.time() - t0:.0f}s)")
        for k in mal[:40]:
            print("  DISTINTA", k)
        return 1 if mal else 0
    if a.reconstruir:
        if a.todo:
            filas = [f"{n}:{x['id']}" for n, j in hojas for x in j["actores"]]
            ahora = None
        else:
            filas, ahora, reg0 = desfasadas(tex, M, hojas)
            # una lamina tocada a mano (PNG distinto del registrado) se rehace entera
            tocadas = png_tocados(tex, leer_huellas(tex), hojas)
            filas = sorted(set(filas) | {f"{n}:{x['id']}" for n, j in hojas if n in tocadas for x in j["actores"]})
        if filas:
            reconstruir(tex, M, hojas, filas)
        # las huellas de las filas rehechas (los PNG ya no estan viejos); el resto se queda como estaba
        reg = leer_huellas(tex)
        reg.update(huellas_actuales(M, hojas, {f.replace(":", "/", 1) for f in filas}) if filas else {})
        anotar(tex, reg, hojas)  # solo las laminas en curso: las demas conservan su estado
        guardar_huellas(tex, reg)
        print(f"reconstruidas {len(filas)} filas ({time.time() - t0:.0f}s)")
        return 0
    # --comprobar (por defecto)
    if not (tex / HUELLAS).exists():
        print(f"falta {HUELLAS}: python3 tools/laminas_desde_capas.py --registrar", file=sys.stderr)
        return 1
    viejas, ahora, reg = desfasadas(tex, M, hojas)
    tocadas = png_tocados(tex, leer_huellas(tex), hojas)
    for k in viejas:
        print(k)
    for n in tocadas:
        print(f"{n}:(PNG distinto del registrado: lamina tocada a mano o regenerada)")
    if tocadas and not viejas:
        print(f"{len(tocadas)} laminas con PNG distinto: python3 tools/laminas_desde_capas.py --reconstruir",
              file=sys.stderr)
        return 1
    if viejas:
        print(f"{len(viejas)} filas desfasadas (o sin huella): python3 tools/laminas_desde_capas.py --reconstruir",
              file=sys.stderr)
        return 1
    print(f"laminas_desde_capas: {len(ahora)} filas al dia ({time.time() - t0:.0f}s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
