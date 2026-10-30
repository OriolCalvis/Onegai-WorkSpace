#!/usr/bin/env python3
"""Valida los atlas runtime de sprites contra su manifiesto JSON.

Comprueba lo que el motor da por hecho al cargar un atlas y que no se ve
hasta que un sprite sale cortado o vacio en partida:

  1. la imagen existe y mide columns x frameSize (y rows, si se declara);
  2. todo frame citado (frames, objetos, actores, animaciones) esta dentro
     de la rejilla y no esta vacio;
  3. los ids no se repiten;
  4. en los atlas generados (con "generador"), ningun dibujo toca el borde
     de su celda: con filtrado o redondeo de UV se veria en la celda vecina;
  5. en los atlas de actores, cada animacion usa frames de la fila de su
     actor.

Y el contrato de los personajes por capas (personajes.json):

  6. cada atlas de capas existe y tiene 12 columnas y una fila por entrada;
  7. ninguna fila citada esta vacia (y las capas que una raza no tiene, si);
  8. ningun dibujo toca el borde de su celda;
  9. toda ranura tiene orden de dibujo en las cuatro direcciones (y el de una
     pieza que trae el suyo, como los guanteletes, es por direccion), los id
     no se repiten y los personajes montados solo citan razas y piezas que hay.

    python3 tools/validar_sprites.py                 # los manifiestos por defecto
    python3 tools/validar_sprites.py a.json b.json   # los que se indiquen

Sale con 1 si hay errores. Solo lee: no toca ningun fichero.
"""
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
TEX = ROOT / "assets/textures"
POR_DEFECTO = ["personaje.json", "actores_capas.json", "razas_andando.json",
               "personajes_historia.json", "props_p0.json", "pickups_p0.json",
               "boundington_story_actors_idle.json", "personajes.json"]
DIRECCIONES = ("abajo", "arriba", "derecha", "izquierda")


def _frame_size(man):
    if "frameSize" in man:
        return int(man["frameSize"][0]), int(man["frameSize"][1])
    if "frame" in man:  # formato de race_npc_idle.json
        return int(man["frame"]["width"]), int(man["frame"]["height"])
    raise ValueError("el manifiesto no declara frameSize")


def _frames_citados(man):
    """(etiqueta, frame) de todos los frames que el manifiesto promete."""
    citados = []
    frames = man.get("frames")
    if isinstance(frames, dict):
        citados += [(f"frames.{k}", v) for k, v in frames.items()]
    elif isinstance(frames, list):
        citados += [(f"frames[{i}]", e.get("frame")) for i, e in enumerate(frames)]
    for o in man.get("objetos", []):
        citados.append((f"objetos.{o.get('id')}", o.get("frame")))
    for a in man.get("actores", []):
        for anim, lista in a.get("animaciones", {}).items():
            citados += [(f"actores.{a.get('id')}.{anim}", f) for f in lista]
    for anim, lista in man.get("animaciones", {}).items():
        citados += [(f"animaciones.{anim}", f) for f in lista]
    return citados


def _ids(man):
    ids = [o.get("id") for o in man.get("objetos", [])]
    ids += [a.get("id") for a in man.get("actores", [])]
    return ids


def _atlas_capas(man):
    """(seccion, atlas por talla o capa, filas esperadas) de personajes.json."""
    yield "razas", man["razas"]["atlas"], man["razas"]["filas"]
    for sec in ("cuerpos", "ojos", "pelo", "barba"):
        yield sec, man[sec]["atlas"], len(man[sec]["filas"])
    for ranura, e in man["equipo"].items():
        yield f"equipo.{ranura}", e["atlas"], len(e["piezas"])


def validar_personajes(ruta_json, man):
    """Errores del contrato de personajes por capas (personajes.json)."""
    base, nombre = Path(ruta_json).parent, Path(ruta_json).name
    fw, fh = _frame_size(man)
    cols = int(man.get("columns", 0))
    errores = []
    imgs = {}
    for sec, atlas, filas in _atlas_capas(man):
        for clave, ruta in atlas.items():
            p = base / ruta
            if not p.is_file():
                errores.append(f"{nombre}: {sec}.{clave}: falta {ruta}")
                continue
            img = np.array(Image.open(p).convert("RGBA"))
            if img.shape[1] != cols * fw or img.shape[0] != filas * fh:
                errores.append(f"{nombre}: {ruta} mide {img.shape[1]}x{img.shape[0]} y deberia medir "
                               f"{cols * fw}x{filas * fh} ({filas} filas)")
                continue
            a = img[..., 3] > 0
            bordes = np.zeros_like(a)
            for k in range(cols):
                bordes[:, k * fw] = bordes[:, (k + 1) * fw - 1] = True
            for k in range(filas):
                bordes[k * fh] = bordes[(k + 1) * fh - 1] = True
            if (a & bordes).any():
                ys, xs = np.nonzero(a & bordes)
                errores.append(f"{nombre}: {ruta}: un dibujo toca el borde de su celda "
                               f"(fila {ys[0] // fh + 1}, frame {xs[0] // fw + 1})")
            imgs[ruta] = a
    llenas = {}
    for sec, atlas, filas in _atlas_capas(man):
        for clave, ruta in atlas.items():
            if ruta in imgs:
                por_fila = imgs[ruta].reshape(filas, fh, -1).any(axis=(1, 2))
                llenas[ruta] = por_fila
                if sec != "razas":
                    for n in np.nonzero(~por_fila)[0]:
                        errores.append(f"{nombre}: {ruta}: la fila {n + 1} esta vacia")
    for r in man["razas"]["lista"]:
        for capa, ruta in man["razas"]["atlas"].items():
            if ruta not in llenas:
                continue
            tiene = bool(llenas[ruta][r["fila"] - 1])
            if tiene != (capa in r["capas"]):
                estado = "vacia" if not tiene else "con dibujo"
                errores.append(f"{nombre}: raza {r['id']}: la capa {capa} esta {estado} y "
                               f"'capas' dice {r['capas']}")
    ranuras = set(man["razas"]["atlas"]) | {"ojos", "pelo", "barba"} | set(man["equipo"])
    for ranura in sorted(ranuras):
        orden = man.get("orden", {}).get(ranura)
        if not orden or any(d not in orden for d in DIRECCIONES):
            errores.append(f"{nombre}: la ranura {ranura} no tiene orden de dibujo en las 4 direcciones")
    # SP-02: una fila con orden propio (los guanteletes) lo da por direccion, con numeros enteros
    for k, e in man["equipo"].items():
        for p in e["piezas"]:
            propio = p.get("orden")
            # (y, desde la Forja de Onegai, «f<n>»: el orden de un frame suelto, 1..20)
            if propio is not None and (not isinstance(propio, dict) or any(d not in propio for d in DIRECCIONES) or any(
                    (d not in DIRECCIONES and not (d[:1] == "f" and d[1:].isdigit() and 1 <= int(d[1:]) <= 20))
                    or not isinstance(z, int) for d, z in propio.items())):
                errores.append(f"{nombre}: equipo.{k}: la pieza {p['id']} trae un orden propio que no vale: {propio}")
    # PART-02b: las anclas de cada talla (la mano del arma, la otra, el pecho y la cintura), un punto
    # "x,y" dentro del frame por cada frame de la fila
    for talla, t in man.get("tallas", {}).items():
        if "anclas" not in t:
            continue
        for punto in ("mano", "otraMano", "pecho", "cintura"):
            pares = str(t["anclas"].get(punto, "")).split()
            ok = len(pares) == cols and all(
                len(p.split(",")) == 2 and all(c.isdigit() for c in p.split(",")) and
                int(p.split(",")[0]) < fw and int(p.split(",")[1]) < fh for p in pares)
            if not ok:
                errores.append(f"{nombre}: tallas.{talla}.anclas.{punto}: tiene que llevar {cols} "
                               f"puntos 'x,y' dentro del frame")
    secciones = [("razas", [r["id"] for r in man["razas"]["lista"]])]
    secciones += [(s, [f["id"] for f in man[s]["filas"]]) for s in ("cuerpos", "ojos", "pelo", "barba")]
    secciones += [(f"equipo.{k}", [p["id"] for p in e["piezas"]]) for k, e in man["equipo"].items()]
    for sec, ids in secciones:
        rep = sorted({i for i in ids if ids.count(i) > 1})
        if rep:
            errores.append(f"{nombre}: {sec}: ids repetidos {rep}")
    razas = {r["id"] for r in man["razas"]["lista"]}
    piezas = {k: {p["id"] for p in e["piezas"]} for k, e in man["equipo"].items()}
    for grupo, specs in man.get("personajes", {}).items():
        for ident, spec in specs.items():
            if spec.get("raza") not in razas:
                errores.append(f"{nombre}: {grupo}.{ident}: raza desconocida {spec.get('raza')!r}")
            for ranura, pieza in spec.get("equipo", {}).items():
                if pieza not in piezas.get(ranura, ()):
                    errores.append(f"{nombre}: {grupo}.{ident}: no hay pieza {pieza!r} en {ranura}")
    return errores


def validar(ruta_json):
    """Lista de errores (vacia si el atlas cumple el contrato)."""
    ruta_json = Path(ruta_json)
    nombre = ruta_json.name
    try:
        man = json.loads(ruta_json.read_text(encoding="utf-8"))
    except (OSError, ValueError) as e:
        return [f"{nombre}: no se puede leer ({e})"]
    if "razas" in man and "equipo" in man and "image" not in man:
        try:
            return validar_personajes(ruta_json, man)
        except (KeyError, TypeError, ValueError) as e:
            return [f"{nombre}: manifiesto de personajes incompleto ({e!r})"]
    errores = []
    try:
        fw, fh = _frame_size(man)
    except ValueError as e:
        return [f"{nombre}: {e}"]
    ruta_png = ruta_json.parent / man.get("image", "")
    if not man.get("image") or not ruta_png.is_file():
        return [f"{nombre}: falta la imagen {man.get('image')!r}"]
    img = np.array(Image.open(ruta_png).convert("RGBA"))
    alto, ancho = img.shape[:2]
    cols = int(man.get("columns", 0))
    if cols <= 0 or ancho != cols * fw:
        errores.append(f"{nombre}: la imagen mide {ancho} px de ancho y el manifiesto dice "
                       f"{cols} columnas de {fw}")
        return errores
    filas = int(man.get("rows", alto // fh))
    if alto != filas * fh:
        errores.append(f"{nombre}: la imagen mide {alto} px de alto y el manifiesto dice "
                       f"{filas} filas de {fh}")
        return errores
    total = cols * filas

    def celda(f):
        c, r = (f - 1) % cols, (f - 1) // cols
        return img[r * fh:(r + 1) * fh, c * fw:(c + 1) * fw]

    vistos = set()
    for etiqueta, f in _frames_citados(man):
        if not isinstance(f, int) or not 1 <= f <= total:
            errores.append(f"{nombre}: {etiqueta} cita el frame {f!r}, fuera de 1..{total}")
            continue
        if f in vistos:
            continue
        vistos.add(f)
        a = celda(f)[..., 3] > 0
        if not a.any():
            errores.append(f"{nombre}: {etiqueta} cita el frame {f}, que esta vacio")
        elif man.get("generador") and (a[0].any() or a[-1].any() or a[:, 0].any() or a[:, -1].any()):
            errores.append(f"{nombre}: el frame {f} ({etiqueta}) toca el borde de su celda")

    ids = _ids(man)
    repetidos = sorted({i for i in ids if ids.count(i) > 1})
    if repetidos:
        errores.append(f"{nombre}: ids repetidos {repetidos}")

    for a in man.get("actores", []):
        fila = a.get("fila")
        if not isinstance(fila, int):
            continue
        rango = range((fila - 1) * cols + 1, fila * cols + 1)
        for anim, lista in a.get("animaciones", {}).items():
            fuera = [f for f in lista if f not in rango]
            if fuera:
                errores.append(f"{nombre}: {a.get('id')}.{anim} usa frames {fuera} "
                               f"fuera de su fila {fila}")
    return errores


def main(argv):
    rutas = [Path(p) for p in argv] or [TEX / n for n in POR_DEFECTO]
    errores = []
    for ruta in rutas:
        errores += validar(ruta)
    if errores:
        print("\n".join(errores))
        return 1
    print(f"sprites: {len(rutas)} atlas conformes a su manifiesto")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
