#!/usr/bin/env python3
"""Oriol (23/10): la Forja de Onegai entera: armas, ROPA y CONJUNTOS (y después razas y clases). Saca lo que
necesitan las pestañas «Ropa» y «Conjuntos» de tools/forja.html, y mete dentro la pestaña «Armas» (la de
tools/editor_armas_plantilla.html, con sus datos de exportar_editor_armas.py).

Todo lo que tiene materiales se pinta UNA vez con materiales «hueco» (un color distinto por parámetro y
tono) y el navegador lo recolorea con la rampa que se elija: así se ve al momento cualquier combinación
sin volver a pasar por Python, y lo que se ve es lo que saldrá.

    python3 tools/exportar_forja.py         # escribe tools/forja.html (abrir en el navegador)

Lo que guarda la Forja: tools/ajustes_armas.json (armas) y tools/ajustes_ropa.json (ropa: ver forja_ropa.py).
Después: `python3 tools/gen_personajes.py` (el jugador) y `python3 tools/gen_pnj.py` (los PNJ).
"""
import base64
import hashlib
import io
import itertools
import json
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import numpy as np  # noqa: E402
from PIL import Image  # noqa: E402

import actores_personajes as pers  # noqa: E402  (registra oficios, trajes y lo de la Forja)
import actores_formas  # noqa: E402

actores_formas.registrar()  # todas las armas (las de base y las formas de actores_formas), y las nuevas de la Forja
import actores_armas as armas  # noqa: E402
import actores_capas as capas  # noqa: E402
import actores_cascos as cascos  # noqa: E402
import actores_equipo as equipo  # noqa: E402
import actores_pelo as pelo  # noqa: E402
import actores_pnj as pnj  # noqa: E402
import actores_partes as partes  # noqa: E402
import actores_rasgos as rasgos  # noqa: E402
import actores_razas as razas  # noqa: E402
import actores_tallas as tallas  # noqa: E402
import forja_ropa  # noqa: E402
from actores_capas import capa_plantilla  # noqa: E402
from actores_materiales import MATERIALES  # noqa: E402
from actores_pintura import Material  # noqa: E402

TONOS = "HsdoO"
B_HUECO = 77  # el azul de los colores «hueco»: R = 200 + parámetro, G = 20 + 30 * tono


def hueco(k):
    c = ["#%02x%02x%02x" % (200 + k, 20 + i * 30, B_HUECO) for i in range(5)]
    return Material(f"__hueco{k}", *c)


HUECOS = {f"__hueco{k}": hueco(k) for k in range(20)}
MAT_H = dict(MATERIALES, **HUECOS)
REALES = sorted(MATERIALES)  # los de verdad (los huecos se suman a MATERIALES para pintar las razas)


class Imagenes:
    """Los recortes, sin repetir: cada frame es [x, y, índice] o None."""

    def __init__(self):
        self.lista, self.indice = [], {}

    def recorte(self, px):
        a = px[..., 3] > 0
        if not a.any():
            return None
        ys, xs = np.nonzero(a)
        x0, x1, y0, y1 = int(xs.min()), int(xs.max()) + 1, int(ys.min()), int(ys.max()) + 1
        trozo = np.ascontiguousarray(px[y0:y1, x0:x1].astype(np.uint8))
        h = hashlib.md5(trozo.tobytes() + bytes([x1 - x0])).hexdigest()
        if h not in self.indice:
            b = io.BytesIO()
            Image.fromarray(trozo, "RGBA").save(b, "PNG", optimize=True)
            self.indice[h] = len(self.lista)
            self.lista.append(base64.b64encode(b.getvalue()).decode())
        return [x0, y0, self.indice[h]]

    def frames(self, lista):
        return [self.recorte(np.asarray(fr)) for fr in lista]


def tipo_de(valores):
    if all((v is None) or (isinstance(v, str) and v in MATERIALES) for v in valores):
        return "mat?" if None in valores else "mat"
    return "opt"


def js(v):
    """Un valor de opción en el JSON (las texturas son funciones: «fn:<nombre>»)."""
    if callable(v):
        return "fn:" + v.__name__
    return v


def patrones_de_ropa(ts, img):
    pats = forja_ropa.patrones()
    usos = {}
    for ranura, cat in equipo.CATALOGO.items():
        for ident, (fn, args, nombre) in cat.items():
            usos.setdefault(forja_ropa.pid_de(ranura, ident), []).append((ident, args, nombre))
    salida, piezas = {}, {}
    for pid, pat in sorted(pats.items()):
        sig_def = {}
        try:
            import inspect
            sig_def = {n: p.default for n, p in inspect.signature(pat["fn"]).parameters.items()
                       if p.default is not inspect.Parameter.empty}
        except (TypeError, ValueError):
            pass
        params = []
        for k, nom in enumerate(pat["params"]):
            vals = []
            for _i, args, _n in usos[pid]:
                v = args[k] if k < len(args) else sig_def.get(nom)
                if v not in vals:
                    vals.append(v)
            t = tipo_de(vals)
            prm = {"n": nom, "tipo": t, "defecto": js(vals[0])}
            if t == "opt":
                prm["valores"] = [js(v) for v in vals]
            params.append(prm)
        # una pintura por cada combinación de opciones (y de «con / sin» en los materiales opcionales)
        # la clave de una combinación: «k=i» por cada opción (i, la posición del valor en «valores») y por
        # cada material opcional (0 = sin, 1 = con), separadas por «|»
        ejes = []
        for k, p in enumerate(params):
            if p["tipo"] == "opt":
                ejes.append([(k, i) for i in range(len(p["valores"]))])
            elif p["tipo"] == "mat?":
                ejes.append([(k, 0), (k, 1)])
        combos = {}
        for elec in itertools.product(*ejes) if ejes else [()]:
            fijo = dict(elec)
            args = []
            for k, p in enumerate(params):
                if p["tipo"] == "opt":
                    args.append(forja_ropa._valor(p["valores"][fijo[k]]))
                elif p["tipo"] == "mat?" and fijo[k] == 0:
                    args.append(None)
                else:
                    args.append(f"__hueco{k}")
            clave = "|".join(f"{k}={i}" for k, i in sorted(fijo.items()))
            prenda = pat["fn"](*args)
            por_talla = {}
            for tk, t in ts.items():
                from actores_pintura import pintar_prenda
                por_talla[tk] = img.frames([pintar_prenda(t, f, prenda, MAT_H) for f in range(1, t.n_frames + 1)])
            combos[clave] = por_talla
        salida[pid] = {"ranura": pat["ranura"], "params": params, "combos": combos}
        for ident, args, nombre in usos[pid]:
            piezas.setdefault(pat["ranura"], {})[ident] = {
                "nombre": nombre, "patron": pid,
                "args": [js(args[k] if k < len(args) else sig_def.get(n)) for k, n in enumerate(pat["params"])]}
    return salida, piezas


def raza_hueca(r):
    """La raza con sus materiales cambiados por huecos, y [(etiqueta, material)] en el orden de los huecos."""
    import copy
    slots = []

    def h(etiqueta, mat):
        if not isinstance(mat, str) or mat not in MATERIALES or mat.startswith("__"):
            return mat
        slots.append((etiqueta, mat))
        return f"__hueco{len(slots) - 1}"

    e = copy.deepcopy(r)
    e.pop("retoques", None)  # los dibujos a mano de la raza los pinta la Forja encima (razas_visual)
    if isinstance(r["piel"], dict):
        e["piel"] = {k: h("piel" if k == "*" else f"piel ({k})", v) for k, v in r["piel"].items()}
    else:
        e["piel"] = h("piel", r["piel"])
    e["rasgos"] = [(ranura, nombre, h(nombre.lower().replace("_", " "), m1),
                    h(nombre.lower().replace("_", " ") + " 2", m2) if m2 else None)
                   for ranura, nombre, m1, m2 in r.get("rasgos", [])]
    if r.get("cola"):
        tipo, m1, m2 = r["cola"]
        e["cola"] = (tipo, h("cola", m1), h("cola 2", m2) if m2 else None)
    if r.get("alas"):
        tipo, m1, m2 = r["alas"]
        e["alas"] = (tipo, h("alas", m1), h("alas 2", m2))
    if r.get("cuerpo") and r["cuerpo"] != "espectro":
        tipo, esc, vientre = r["cuerpo"]
        e["cuerpo"] = (tipo, h("escamas", esc), h("vientre", vientre))
    assert len(slots) <= len(HUECOS), f"{r['id']}: {len(slots)} materiales, mas que huecos"
    return e, slots


def fichas_del_juego():
    """Lo que se edita de las razas y las clases (los catálogos del motor) y el equipo para elegir."""
    cat = HERE.parent / "assets/catalogs"
    razas_c = json.loads((cat / "races.json").read_text(encoding="utf-8"))["entries"]
    clases_c = json.loads((cat / "classes.json").read_text(encoding="utf-8"))["entries"]
    equipo_c = json.loads((cat / "equipment.json").read_text(encoding="utf-8"))["entries"]
    campos_eq = ("id", "name", "slot", "grantedTags", "weightCategory", "tier", "rarity", "twoHanded")
    vis = HERE.parent / "assets/textures/personajes/equipo_catalogo.json"
    visual = json.loads(vis.read_text(encoding="utf-8")).get("objetos", {}) if vis.exists() else {}
    return {"razas": {e["id"]: e for e in razas_c},
            "clases": {e["id"]: {k: v for k, v in e.items() if k not in ("startingCardsJava",)} for e in clases_c},
            "equipo": {e["id"]: {k: e.get(k) for k in campos_eq} for e in equipo_c},
            "visual": visual}


def con_huecos(diccionario, ident, campos):
    """Una copia de la entrada con sus materiales cambiados por huecos (para pintar una vez y recolorear)."""
    e = dict(diccionario[ident])
    mats = []
    for k, c in enumerate(campos):
        mats.append(e.get(c))
        if e.get(c):
            e[c] = f"__hueco{k}"
    return e, mats


def main():
    t0 = time.time()
    ts = tallas.construir_tallas()
    img = Imagenes()
    pats, piezas = patrones_de_ropa(ts, img)
    print(f"ropa: {len(pats)} patrones, {sum(len(v) for v in piezas.values())} piezas ({time.time() - t0:.0f}s)")

    # capas (la prenda): un patrón por largo, con el material en el hueco 0
    capas_esp = {}
    largos = sorted({v[1] for v in equipo.CAPAS.values()})
    capa_render = {lg: {tk: img.frames(equipo.capa_capa(t, HUECOS["__hueco0"], lg)) for tk, t in ts.items()}
                   for lg in largos}
    for ident, (mat, largo, nombre) in equipo.CAPAS.items():
        capas_esp[ident] = {"nombre": nombre, "largo": largo, "mats": [mat]}

    # la cabeza: cada pieza pintada con huecos (m1, m2)
    cabeza = {}
    for ident in cascos.CABEZA:
        e, mats = con_huecos(cascos.CABEZA, ident, ("m1", "m2"))
        guardado = cascos.CABEZA[ident]
        cascos.CABEZA[ident] = e
        try:
            render = {tk: img.frames(cascos.capa_cabeza(t, ident, MAT_H)) for tk, t in ts.items()}
        finally:
            cascos.CABEZA[ident] = guardado
        cabeza[ident] = {"nombre": guardado["nombre"], "ocultaPelo": bool(guardado["oculta_pelo"]),
                         "mats": mats, "t": render}
    print(f"cabeza: {len(cabeza)} ({time.time() - t0:.0f}s)")

    # las armas: cada FORMA pintada una vez con los cuatro materiales en huecos (las armas de la misma forma
    # solo cambian de materiales), con su ranura y su orden de dibujo; y cada arma, su forma y sus materiales
    armas_fam, armas_d = {}, {}
    ejemplo = {}
    for ident, fila in sorted(armas.ARMAS.items()):
        ejemplo.setdefault(fila[0], ident)
    for familia, ident in sorted(ejemplo.items()):
        fila = armas.ARMAS[ident]
        armas.ARMAS[ident] = (familia, "__hueco0", "__hueco1", fila[3], "__hueco2", "__hueco3")
        try:
            render, ranura = {}, None
            for tk, t in ts.items():
                ranura, frames = armas.capa_arma(t, ident, MAT_H)
                render[tk] = img.frames(frames)
        finally:
            armas.ARMAS[ident] = fila
        armas_fam[familia] = {"ranura": ranura, "orden": armas.orden_de(ident), "t": render}
    for ident, fila in sorted(armas.ARMAS.items()):
        armas_d[ident] = {"nombre": fila[3], "familia": fila[0], "mats": [fila[1], fila[2], *fila[4:6]]}
    print(f"armas: {len(armas_d)} en {len(armas_fam)} formas ({time.time() - t0:.0f}s)")

    # pelo y barba: cada estilo en el hueco 0
    pelos = {e: {tk: img.frames(capa_plantilla(t, pelo.ESTILOS[e], HUECOS["__hueco0"], None, sobre_cuerpo=False))
                 for tk, t in ts.items()} for e in pelo.ESTILOS}
    barbas = {b: {tk: img.frames(capa_plantilla(t, getattr(rasgos, b), HUECOS["__hueco0"], None, sobre_cuerpo=False))
                  for tk, t in ts.items()} for b in pnj.BARBAS}

    # las razas: el cuerpo con cada material suyo en un hueco (la piel, o cada parte de la piel; los rasgos,
    # la cola, las alas, las escamas), en las cuatro tallas; y lo que admite
    MATERIALES.update(HUECOS)  # actores_personajes busca los materiales de la raza en este diccionario
    from actores_capas import capa_ojos
    from actores_materiales import OJOS
    razas_d = {}
    for rid, r in razas.POR_ID.items():
        hueca, slots = raza_hueca(r)
        por_talla = {tk: [{"ranura": ranura, "frames": img.frames(fr)} for ranura, fr in pers.capas_raza(hueca, t)]
                     for tk, t in ts.items()}
        pelo_r, barba_r = r.get("pelo"), r.get("barba")
        razas_d[rid] = {"nombre": r.get("nombre", rid), "talla": r["talla"], "capas": por_talla,
                        "slots": [s[0] for s in slots], "mats": [s[1] for s in slots], "ojos": r["ojos"],
                        "sin": list(r.get("sin", [])), "naga": bool(r.get("cuerpo")),
                        "pelo": list(pelo_r) if pelo_r else None, "barba": list(barba_r) if barba_r else None,
                        "equipo": dict(pers.EQUIPO_RAZAS.get(rid, {}))}
    ojos_d = {k: {tk: img.frames(capa_ojos(t, OJOS[k])) for tk, t in ts.items()} for k in OJOS}
    for h in HUECOS:
        MATERIALES.pop(h, None)
    print(f"razas: {len(razas_d)} ({time.time() - t0:.0f}s)")

    materiales = {n: ["#%02x%02x%02x" % tuple(MATERIALES[n].tono[t]) for t in TONOS] + [getattr(MATERIALES[n], "alfa", 255)]
                  for n in REALES}
    fichas = fichas_del_juego()
    # las piezas de arma del catálogo visual (actores_formas.BASES: «f_arco»...): su forma y sus materiales
    import actores_formas
    bases = {k: [b[0], b[3], *b[1:3], *b[4:]] for k, b in actores_formas.BASES.items() if b[0] in armas_fam}
    import actores_animaciones as anim
    salida = {
        "animBase": anim.base_forja(ts), "animaciones": anim.ANIMACIONES, "ajustesAnimaciones": anim.ajustes(),
        "manos": __import__("actores_armas").MANO,
        "_nota": "Lo genera tools/exportar_forja.py para tools/forja.html (no editar).",
        "frame": 64, "frames": 20, "tallas": list(ts), "orden": capas.ORDEN,
        "direcciones": {str(f): ts["medio"].direccion(f) for f in range(1, 21)},
        "pasos": {str(f): partes.paso_de(f) for f in range(1, 21)},
        "materiales": materiales, "huecoB": B_HUECO, "img": img.lista,
        "patrones": pats, "piezas": piezas, "capas": capas_esp, "capaRender": capa_render,
        "cabeza": cabeza, "armas": armas_d, "armasFam": armas_fam, "bases": bases, "pelos": pelos, "barbas": barbas,
        "coloresPelo": sorted({c for _e, c in pnj.combos_pelo()}),
        "razas": razas_d, "ojos": ojos_d, "ajustesRopa": forja_ropa.ajustes(),
        "fichas": fichas, "ajustesFichas": forja_ropa.ajustes_fichas(),
    }
    datos = json.dumps(salida, ensure_ascii=False, separators=(",", ":")).replace("</", "<\\/")
    print(f"datos: {len(datos) // 1024} KB, {len(img.lista)} recortes ({time.time() - t0:.0f}s)")

    # la pestaña de las armas, tal cual (con sus datos)
    import exportar_editor_armas as ea
    destino_armas = HERE / "editor_armas.html"
    ea.main()
    armas_html = destino_armas.read_text(encoding="utf-8").split("\n", 3)[3]  # sin doctype ni metas
    armas_html = armas_html.replace("<title>Forja de Onegai</title>", "")
    # en la Forja se guarda con «Guardar» (para todos) y se saca con «Exportar JSON»: sin la caja de ajustes
    import re
    armas_html, n = re.subn(r'<section class="panel" style="max-width:1280px;margin-top:18px" aria-label="Ajustes">.*?</section>',
                            '<section class="panel" style="max-width:1280px;margin-top:18px" aria-label="Empezar de cero">\n'
                            '  <div class="fila"><button id="borrarTodo">Empezar de cero</button></div>\n'
                            '  <p class="ayuda">Quita todos los cambios de las armas (al pulsar Guardar, también para los demás).</p>\n'
                            '  <p class="estado" id="estado" role="status"></p>\n</section>', armas_html, count=1, flags=re.S)
    assert n == 1, "no encuentro la caja de ajustes de las armas"
    armas_html = armas_html.replace("Al acabar, copia los ajustes a <code class=\"dato\">tools/ajustes_armas.json</code> y rehaz las hojas con <code class=\"dato\">python3 tools/gen_personajes.py</code> (el jugador) y <code class=\"dato\">python3 tools/gen_pnj.py</code> (los PNJ).",
                                    "Al acabar, pulsa Guardar (arriba) para que lo vean todos los que tienen el enlace.")

    pagina = (HERE / "forja_plantilla.html").read_text(encoding="utf-8")
    pagina = pagina.replace("/*DATOS*/", datos).replace("<!--ARMAS-->", armas_html)
    destino = HERE / "forja.html"
    destino.write_text('<!doctype html>\n<meta charset="utf-8">\n<meta name="viewport" content="width=device-width, initial-scale=1">\n'
                       + pagina, encoding="utf-8")
    print(f"{destino.name}: {destino.stat().st_size // 1024} KB")


if __name__ == "__main__":
    main()
