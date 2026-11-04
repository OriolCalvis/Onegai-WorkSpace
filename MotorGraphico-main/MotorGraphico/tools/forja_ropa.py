"""Oriol (23/10): «aprovechar la Forja para crear también ropas y ver cómo quedan los sets completos, pero
que la edición de cada aspecto visual sea individual». La ropa de la Forja de Onegai (pestaña «Ropa»):

- Un PATRÓN es un constructor de prenda de actores_equipo (tunica, vestido, gambeson, pantalon,
  botas_piel...) con sus parámetros: los materiales (una rampa de cinco tonos cada uno) y las opciones
  (las mangas, la caña de la bota, el largo de la falda...). Cada pieza del catálogo es un patrón con
  sus valores; una pieza nueva de la Forja, también.
- tools/ajustes_ropa.json (lo escribe la Forja):
    «nuevas»   {id: {ranura, nombre, patron, args}}   piezas nuevas (args en el orden del patrón)
    «cambios»  {id: {nombre?, args?}}                   otra cosa en una pieza que ya existe
    «retoques» {id: {talla: {frame: {"x,y": código}}}}  pixeles a mano encima de lo que sale del patrón:
               «<k><tono>» = el tono (H s d o O) del material del parámetro k; «#rrggbb» = ese color;
               «.» = borrar
  Al importar actores_personajes (que registra antes la ropa de oficios y trajes) se aplica todo:
  las piezas nuevas entran en actores_equipo.CATALOGO, y capa_pieza pinta los retoques.
"""
import inspect
import json
from pathlib import Path

import numpy as np

import actores_equipo as equipo

RUTA = Path(__file__).resolve().with_name("ajustes_ropa.json")
TONOS = "HsdoO"


def ajustes():
    return json.loads(RUTA.read_text(encoding="utf-8")) if RUTA.exists() else {}


# Oriol (23/10): las pestañas Razas y Clases de la Forja guardan tools/ajustes_fichas.json:
#   «razas» {id: {campo: valor}}  y  «clases» {id: {campo: valor}}  lo de los catálogos (nombre, descripción,
#        estadísticas, equipo inicial...): lo pasa a assets/catalogs con tools/aplicar_forja_fichas.py
#   «razas_visual» {id: {talla?, mats?: [material por hueco], ojos?, pelo?: [estilo, color] | null, barba?,
#                        retoques?: {talla: {frame: {"x,y": código}}}}}
#        el aspecto: se aplica a actores_razas al importar actores_personajes (aplicar_razas); los
#        retoques son píxeles dibujados a mano en el cuerpo de la raza (retocar_raza)
#   «objetos_visual» {objeto: {ranura, pieza}}  qué pieza se ve puesta para un objeto del catálogo
RUTA_FICHAS = Path(__file__).resolve().with_name("ajustes_fichas.json")


def ajustes_fichas():
    return json.loads(RUTA_FICHAS.read_text(encoding="utf-8")) if RUTA_FICHAS.exists() else {}


def huecos_de_raza(r):
    """[(etiqueta, ruta)] de los materiales de una raza, en el orden de los huecos de la Forja
    (exportar_forja.raza_hueca): ruta = cómo se llega al material dentro de la raza."""
    from actores_materiales import MATERIALES
    out = []

    def h(etiqueta, ruta, mat):
        if isinstance(mat, str) and mat in MATERIALES:
            out.append((etiqueta, ruta))

    if isinstance(r["piel"], dict):
        for k, v in r["piel"].items():
            h("piel" if k == "*" else f"piel ({k})", ("piel", k), v)
    else:
        h("piel", ("piel",), r["piel"])
    for i, (_ranura, nombre, m1, m2) in enumerate(r.get("rasgos", [])):
        h(nombre, ("rasgos", i, 2), m1)
        if m2:
            h(nombre + " 2", ("rasgos", i, 3), m2)
    if r.get("cola"):
        h("cola", ("cola", 1), r["cola"][1])
        if r["cola"][2]:
            h("cola 2", ("cola", 2), r["cola"][2])
    if r.get("alas"):
        h("alas", ("alas", 1), r["alas"][1])
        h("alas 2", ("alas", 2), r["alas"][2])
    if r.get("cuerpo") and r["cuerpo"] != "espectro":
        h("escamas", ("cuerpo", 1), r["cuerpo"][1])
        h("vientre", ("cuerpo", 2), r["cuerpo"][2])
    return out


def _pon(r, ruta, valor):
    if ruta[0] == "piel" and len(ruta) == 1:
        r["piel"] = valor
    elif ruta[0] == "piel":
        r["piel"] = dict(r["piel"], **{ruta[1]: valor})
    elif ruta[0] == "rasgos":
        lista = [list(x) for x in r["rasgos"]]
        lista[ruta[1]][ruta[2]] = valor
        r["rasgos"] = [tuple(x) for x in lista]
    else:
        x = list(r[ruta[0]])
        x[ruta[1]] = valor
        r[ruta[0]] = tuple(x)


def aplicar_razas():
    """El aspecto de las razas de la Forja (razas_visual) en actores_razas.POR_ID."""
    import actores_razas as razas
    from actores_materiales import MATERIALES, OJOS
    fichas = ajustes_fichas()
    for rid, c in fichas.get("razas", {}).items():  # el nombre nuevo de la ficha, también en el dibujo
        if rid in razas.POR_ID and c.get("name"):
            razas.POR_ID[rid]["nombre"] = c["name"]
    for rid, v in fichas.get("razas_visual", {}).items():
        r = razas.POR_ID.get(rid)
        if r is None:
            continue
        rutas = huecos_de_raza(r)
        for (_etiqueta, ruta), mat in zip(rutas, v.get("mats", [])):
            if isinstance(mat, str) and mat in MATERIALES:
                _pon(r, ruta, mat)
        if v.get("talla") in ("medio", "bajo", "pequeno", "grande"):
            r["talla"] = v["talla"]
        if v.get("ojos") in OJOS:
            r["ojos"] = v["ojos"]
        for k in ("pelo", "barba"):
            if k in v:
                if v[k]:
                    r[k] = tuple(v[k])
                else:
                    r.pop(k, None)
        if v.get("nombre"):
            r["nombre"] = v["nombre"]
        if v.get("retoques"):
            r["retoques"] = v["retoques"]


def _lee(r, ruta):
    """El material de una raza en esa ruta (huecos_de_raza)."""
    if ruta[0] == "piel":
        return r["piel"] if len(ruta) == 1 else r["piel"][ruta[1]]
    if ruta[0] == "rasgos":
        return r["rasgos"][ruta[1]][ruta[2]]
    return r[ruta[0]][ruta[1]]


def retocar_raza(capas, raza, retoques):
    """Los píxeles dibujados a mano en la Forja (pestaña Razas) sobre las capas de una raza
    [(ranura, frames)] de una talla: {frame: {"x,y": código}}.
    - «.» borra ese píxel en todas las capas de la raza;
    - un color («#rrggbb» o «<k><tono>», el tono del material del hueco k) va a la capa que se ve
      en ese píxel (la de más arriba que lo tiene pintado) o, si no hay ninguna, al cuerpo; así el
      retoque queda donde estaba y la ropa lo sigue tapando igual.
    La Forja hace lo mismo (forja_plantilla.html, figura)."""
    import actores_partes as partes
    from actores_capas import ORDEN
    from actores_materiales import MATERIALES
    capas = [(ranura, [np.array(fr) for fr in frames]) for ranura, frames in capas]
    huecos = [_lee(raza, ruta) for _e, ruta in huecos_de_raza(raza)]
    for f, pixeles in (retoques or {}).items():
        f = int(f)
        if not capas or not 1 <= f <= len(capas[0][1]):
            continue
        d = partes.DIRECCION[f]
        z = [ORDEN[r][d] if isinstance(r, str) else r.get(f"f{f}", r[d]) for r, _fr in capas]
        originales = [fr[f - 1].copy() for _r, fr in capas]
        for xy, codigo in pixeles.items():
            x, y = (int(v) for v in xy.split(","))
            alto, ancho = originales[0].shape[:2]
            if not (0 <= x < ancho and 0 <= y < alto):
                continue
            if codigo in (".", None, ""):
                for _r, fr in capas:
                    fr[f - 1][y, x] = 0
                continue
            if codigo.startswith("#") and len(codigo) == 7:
                color, alfa = [int(codigo[i:i + 2], 16) for i in (1, 3, 5)], 255
            elif len(codigo) >= 2 and codigo[:-1].isdigit() and codigo[-1] in TONOS and int(codigo[:-1]) < len(huecos):
                m = MATERIALES.get(huecos[int(codigo[:-1])])
                if m is None:
                    continue
                color, alfa = m[codigo[-1]], getattr(m, "alfa", 255)
            else:
                continue
            cual, mejor = 0, None
            for i, o in enumerate(originales):
                if o[y, x, 3] and (mejor is None or (z[i], i) > mejor):
                    cual, mejor = i, (z[i], i)
            capa = capas[cual][1][f - 1]
            capa[y, x, :3] = color
            capa[y, x, 3] = alfa
    return capas


def _pid(ranura, ident, fn):
    nombre = getattr(fn, "__name__", "")
    return f"{ranura}:{ident if nombre == '<lambda>' else nombre}"


def patrones():
    """{pid: {ranura, fn, params: [nombre]}}: cada constructor que usa el catálogo, una vez."""
    out = {}
    for ranura, cat in equipo.CATALOGO.items():
        for ident, (fn, args, _nombre) in cat.items():
            pid = _pid(ranura, ident, fn)
            if pid in out:
                continue
            try:
                params = list(inspect.signature(fn).parameters)
            except (TypeError, ValueError):
                params = [f"p{i}" for i in range(len(args))]
            out[pid] = {"ranura": ranura, "fn": fn, "params": params}
    return out


def pid_de(ranura, ident):
    fn = equipo.CATALOGO[ranura][ident][0]
    return _pid(ranura, ident, fn)


def _retocar(frames, pieza_args, retoques, materiales):
    """Pinta encima los pixeles a mano de una talla ({frame: {"x,y": código}})."""
    for f, pixeles in (retoques or {}).items():
        f = int(f)
        if not 1 <= f <= len(frames):
            continue
        capa = frames[f - 1]
        for xy, codigo in pixeles.items():
            x, y = (int(v) for v in xy.split(","))
            if not (0 <= x < capa.shape[1] and 0 <= y < capa.shape[0]):
                continue
            if codigo in (".", None, ""):
                capa[y, x] = 0
            elif codigo.startswith("#") and len(codigo) == 7:
                capa[y, x, :3] = [int(codigo[i:i + 2], 16) for i in (1, 3, 5)]
                capa[y, x, 3] = 255
            elif len(codigo) >= 2 and codigo[:-1].isdigit() and codigo[-1] in TONOS:
                k = int(codigo[:-1])
                mat = pieza_args[k] if k < len(pieza_args) else None
                if isinstance(mat, str) and mat in materiales:
                    m = materiales[mat]
                    capa[y, x, :3] = m[codigo[-1]]
                    capa[y, x, 3] = getattr(m, "alfa", 255)
    return frames


_APLICADO = False


def aplicar():
    """Mete en el catálogo lo de tools/ajustes_ropa.json (una vez)."""
    global _APLICADO
    if _APLICADO:
        return
    _APLICADO = True
    aplicar_razas()
    aj = ajustes()
    if not aj:
        return
    pats = patrones()
    for ident, p in aj.get("nuevas", {}).items():
        if p.get("ranura") == "capa":  # la capa (la prenda) es «capa:<largo>» con un material
            largo = str(p.get("patron", "capa:larga")).split(":", 1)[-1]
            args = p.get("args") or [None]
            if args[0] in materiales_conocidos():
                equipo.CAPAS[ident] = (args[0], largo, p.get("nombre", ident))
            continue
        pat = pats.get(p.get("patron"))
        if pat is None or pat["ranura"] != p.get("ranura"):
            print(f"forja_ropa: la pieza nueva {ident} pide un patrón que no hay ({p.get('patron')}); se salta")
            continue
        equipo.CATALOGO[pat["ranura"]][ident] = (pat["fn"], tuple(_valor(v) for v in p.get("args", [])),
                                                 p.get("nombre", ident))
    for ident, c in aj.get("cambios", {}).items():
        if ident in equipo.CAPAS:
            mat, largo, nombre = equipo.CAPAS[ident]
            equipo.CAPAS[ident] = ((c.get("args") or [mat])[0] or mat, largo, c.get("nombre", nombre))
            continue
        for ranura, cat in equipo.CATALOGO.items():
            if ident in cat:
                fn, args, nombre = cat[ident]
                cat[ident] = (fn, tuple(_valor(v) for v in c["args"]) if "args" in c else args,
                              c.get("nombre", nombre))
    retoques = aj.get("retoques", {})
    if retoques:
        original = equipo.capa_pieza

        def capa_pieza(talla, ranura, ident, materiales):
            frames = original(talla, ranura, ident, materiales)
            propios = retoques.get(ident, {}).get(getattr(talla, "clave", ""))
            if propios and ranura == "capa" and ident in equipo.CAPAS:
                frames = _retocar([np.array(fr) for fr in frames], (equipo.CAPAS[ident][0],), propios, materiales)
            elif propios and ranura in equipo.CATALOGO and ident in equipo.CATALOGO[ranura]:
                frames = _retocar([np.array(fr) for fr in frames], equipo.CATALOGO[ranura][ident][1], propios,
                                  materiales)
            return frames

        equipo.capa_pieza = capa_pieza


def materiales_conocidos():
    from actores_materiales import MATERIALES
    return MATERIALES


def _valor(v):
    """Los argumentos vienen del JSON: un nombre de función (las texturas de las grebas) vuelve a serlo."""
    if isinstance(v, str) and v.startswith("fn:"):
        return getattr(equipo, v[3:], None)
    return v
