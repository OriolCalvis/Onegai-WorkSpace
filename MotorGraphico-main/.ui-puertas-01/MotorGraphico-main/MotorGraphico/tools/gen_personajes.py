#!/usr/bin/env python3
"""Genera los personajes por capas (razas, pelo, barba y equipo) y los atlas
horneados de actores, razas y personajes de historia.

Todo sale del cuerpo base personaje.png y de los modulos actores_*.py:

  assets/textures/personajes/      atlas de CAPAS para montar personajes:
      razas/<capa>.png             cuerpo, espalda, cola, alas, morro y rasgos
                                   de las 43 razas (una fila por raza)
      pelo/pelo_<talla>.png        peinados x colores
      barba/barba_<talla>.png      barbas x colores
      equipo/<ranura>_<talla>.png  cabeza, torso, piernas, pies, capa, arma
                                   y mano izquierda (una fila por pieza)
  assets/textures/personajes.json  el contrato: orden de dibujo por direccion,
                                   filas de cada atlas y los personajes montados
  assets/textures/actores_capas.png        9 actores genericos (ARTE-01)
  assets/textures/razas_andando.png        43 razas vestidas, andando
  assets/textures/personajes_historia.png  10 personajes de historia
  (cada uno con su .json, y personaje.json para el cuerpo base)

    python3 tools/gen_personajes.py            # todo
    python3 tools/gen_personajes.py --vista    # + laminas en out/revision_personajes/

Si encuentra races.json (assets/catalogs) comprueba que todas sus razas tienen
dibujo, y con equipment.json escribe personajes/equipo_catalogo.json: la pieza
visible de cada objeto del catalogo (tools/actores_catalogo.py).
"""
import argparse
import sys
import json
from pathlib import Path

import numpy as np
from PIL import Image

import actores_armas as armas
import actores_capas as capas_mod
import actores_cascos as cascos
import actores_catalogo as catalogo
import actores_equipo as equipo
import actores_oficios as oficios
import actores_formas as formas
import actores_pelo as pelo
import actores_personajes as pers
import actores_razas as razas
from actores_tallas import TALLAS, construir_tallas
import actores_anclas

formas.registrar()  # ESTETICA-01: las formas por objeto, detras de las piezas de siempre

ROOT = Path(__file__).resolve().parent.parent
TEX = ROOT / "assets/textures"
FRAME = 64
COLUMNAS = 12        # hojas horneadas (actores, razas, historia, PNJ) y personaje.png: andar
COLUMNAS_CAPAS = 20  # ESTETICA-01: los atlas de capas llevan ademas los 8 frames de reposo
ORDEN_TALLAS = ["medio", "bajo", "pequeno", "grande"]
CAPAS_RAZA = ["cuerpo", "espalda", "cola", "alas", "morro", "rasgos"]

PIELES = ["piel_clara", "piel_morena", "piel_palida", "piel_oscura", "piel_aceituna", "piel_rojiza",
          "piel_gris", "piel_elfo", "porcelana"]
COLORES_PELO = ["pelo_castano", "pelo_rubio", "pelo_negro", "pelo_pelirrojo", "pelo_oscuro",
                "pelo_blanco", "pelo_gris"]
COLORES_PELO_RAZA = ["pelo_plata_lila", "pelo_verde_musgo", "pelo_azul_hielo"]  # solo corto/largo
BARBAS = ["BARBA", "BARBA_CORTA"]

DIRECCIONES = {"arriba": [1, 2, 3], "derecha": [4, 5, 6], "abajo": [7, 8, 9],
               "izquierda": [10, 11, 12]}
ANIMACIONES = {
    "idle": [8], "idle_down": [8], "idle_up": [2], "idle_right": [5], "idle_left": [11],
    "walk_down": [7, 8, 9, 8], "walk_up": [1, 2, 3, 2],
    "walk_right": [4, 5, 6, 5], "walk_left": [10, 11, 12, 11],
}
# ESTETICA-01: los frames de reposo de los atlas de capas (actores_partes.REPOSO): el cuerpo
# quieto y respirando, con el arma como se lleva en reposo (actores_poses)
REPOSO = {"arriba": [13, 14], "derecha": [15, 16], "abajo": [17, 18], "izquierda": [19, 20]}
NOTA_REPARTO = (
    "Frames 1-based. Cada fila: arriba 1-3, derecha 4-6, abajo 7-9, izquierda 10-12 "
    "(paso, quieto, paso). El motor lo reparte asi desde APARIENCIA-01 "
    "(HojaPersonaje::animar, tambien para personaje.png): 'animaciones' es el ciclo de "
    "los PNJ; el jugador descansa en el frame quieto y su ciclo de andar empieza en el "
    "({8, 7, 8, 9} hacia abajo). TextureManager invierte los PNG al cargarlos: en un "
    "atlas de varias filas la region del frame f es defineRegion(f, (f-1) % columns, "
    "rows - 1 - (f-1) / columns), como race_npc_idle.")


# --- PL-JUEGO-01 (Oriol, 25/10: «todos a la vez»): los personajes en diagonal de PixelLab ---------------------
# Con --pl (lo de siempre a partir de ahora) las tallas son las familias de PixelLab (pixellab_cuerpo:
# medio = humano, bajo = enano, pequeno = mediano, grande = orco, y naga, aarakocra, espectro y minotauro
# con su cuerpo propio), cada fila de los atlas lleva 40 frames (andar de seis y respirar de cuatro en las
# cuatro diagonales: noreste, sureste, suroeste y noroeste, que son arriba, derecha, abajo e izquierda de
# la rejilla) y las hojas horneadas (actores, razas, historia y PNJ) tambien. El motor lo sabe por las
# columnas (Game::repartoDeColumnas) y por «frames» del manifiesto. --clasico escribe lo de antes.
PL = False
NOMBRES_PL = {"medio": "Humano (PixelLab)", "bajo": "Enano (PixelLab)", "pequeno": "Mediano (PixelLab)",
              "grande": "Orco (PixelLab)", "naga": "Naga (PixelLab)", "aarakocra": "Aarakocra (PixelLab)",
              "espectro": "Espectro (PixelLab)", "minotauro": "Minotauro (PixelLab)"}


def _modo_pl():
    """Pasa el modulo a PixelLab: columnas, tallas, reparto y animaciones de 40 frames."""
    global PL, COLUMNAS, COLUMNAS_CAPAS, ORDEN_TALLAS, DIRECCIONES, ANIMACIONES, REPOSO, NOTA_REPARTO
    import pixellab_cuerpo as pc
    PL = True
    COLUMNAS = COLUMNAS_CAPAS = pc.COLUMNAS
    ORDEN_TALLAS = [t for t in pc.FAMILIAS if (pc.CARPETA / f"{pc.FAMILIAS[t]}.json").exists()]
    rumbos = ["arriba", "derecha", "abajo", "izquierda"]
    na, nr = pc.FRAMES_ANDAR, pc.FRAMES_RESPIRAR
    DIRECCIONES = {r: [i * na + k + 1 for k in range(na)] for i, r in enumerate(rumbos)}
    REPOSO = {r: [4 * na + i * nr + k + 1 for k in range(nr)] for i, r in enumerate(rumbos)}
    ANIMACIONES = {"idle": REPOSO["abajo"], "idle_down": REPOSO["abajo"], "idle_up": REPOSO["arriba"],
                   "idle_right": REPOSO["derecha"], "idle_left": REPOSO["izquierda"],
                   "walk_down": DIRECCIONES["abajo"], "walk_up": DIRECCIONES["arriba"],
                   "walk_right": DIRECCIONES["derecha"], "walk_left": DIRECCIONES["izquierda"]}
    NOTA_REPARTO = (
        "Frames 1-based, 40 por fila (PL-JUEGO-01, PixelLab en diagonal): andar 1-24, seis por rumbo "
        "(arriba = noreste 1-6, derecha = sureste 7-12, abajo = suroeste 13-18, izquierda = noroeste 19-24), y "
        "respirar 25-40, cuatro por rumbo en el mismo orden. El motor lo reparte asi (Game::Reparto::Diagonal, "
        "por las columnas de la hoja). TextureManager invierte los PNG al cargarlos.")
    return pc.construir_tallas_pl()


def talla_de_raza(r, tallas):
    """La talla (en PixelLab, la familia) de una raza; sin el cuerpo de su familia, el mas parecido."""
    if not PL:
        return r["talla"]
    import pixellab_razas
    t = pixellab_razas.familia_de(r)
    if t not in tallas:
        t = "grande" if t == "minotauro" else "medio"
    return t


# --- utilidades ----------------------------------------------------------------

def _ruta(p):
    try:
        return str(Path(p).resolve().relative_to(ROOT))
    except ValueError:
        return str(p)


RECORTADOS = []  # PL-JUEGO-01: lo que se ha recortado del borde (las piezas dibujadas para otro cuerpo)


def comprobar(frames, que):
    """Nada puede tocar el borde de la celda: se veria en el frame vecino. En PixelLab, las piezas de
    siempre puestas en cuerpos mas grandes (los cuernos del minotauro en el del orco) a veces llegan:
    se recorta ese pixel del borde y se apunta, en vez de parar."""
    for f, px in enumerate(frames, 1):
        a = px[..., 3] > 0
        if a[0].any() or a[-1].any() or a[:, 0].any() or a[:, -1].any():
            if not PL:
                raise SystemExit(f"{que}, frame {f}: el dibujo toca el borde de la celda")
            for sl in (np.s_[0, :], np.s_[-1, :], np.s_[:, 0], np.s_[:, -1]):
                px[sl] = 0
            RECORTADOS.append(f"{que}, frame {f}")


def guardar_atlas(filas, ruta, columnas=None):
    """filas: lista de listas de frames RGBA -> PNG de `columnas` columnas (sin ellas, las de las hojas
    horneadas del modo en curso: 12, o 40 en PixelLab)."""
    columnas = columnas or COLUMNAS
    ruta = Path(ruta)
    ruta.parent.mkdir(parents=True, exist_ok=True)
    img = np.zeros((max(1, len(filas)) * FRAME, columnas * FRAME, 4), np.uint8)
    for r, fila in enumerate(filas):
        for c, px in enumerate(fila):
            img[r * FRAME:(r + 1) * FRAME, c * FRAME:(c + 1) * FRAME] = px
    Image.fromarray(img, "RGBA").save(ruta, optimize=True)
    return img


def vacia():
    return [np.zeros((FRAME, FRAME, 4), np.uint8) for _ in range(COLUMNAS_CAPAS)]


def juntar(capas):
    """Varias capas de la misma ranura en una (mismo orden de dibujo)."""
    if len(capas) == 1:
        return capas[0]
    out = []
    for f in range(min(len(c) for c in capas)):
        base = Image.new("RGBA", (FRAME, FRAME), (0, 0, 0, 0))
        for c in capas:
            base.alpha_composite(Image.fromarray(c[f], "RGBA"))
        out.append(np.array(base))
    return out


def escribir_json(datos, ruta):
    Path(ruta).write_text(json.dumps(datos, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")


def buscar(nombre):
    """Primer fichero con ese nombre en el repo, sin entrar en out/, build/..."""
    import os
    saltar = {"out", "build", "node_modules", ".git", "cmake-build-debug", "cmake-build-release"}
    for dirpath, dirnames, filenames in os.walk(ROOT):
        dirnames[:] = sorted(d for d in dirnames if d not in saltar and not d.startswith("."))
        if nombre in filenames:
            return Path(dirpath) / nombre
    return None


# --- cruce con el catalogo ------------------------------------------------------------

def comprobar_razas(ruta):
    """races.json y actores_razas.RAZAS tienen que tener las mismas razas."""
    datos = json.loads(Path(ruta).read_text(encoding="utf-8"))
    lista = datos.get("entries", datos) if isinstance(datos, dict) else datos
    catalogo_ids = {r["id"] for r in lista}
    nuestros = {r["id"] for r in razas.RAZAS}
    faltan, sobran = sorted(catalogo_ids - nuestros), sorted(nuestros - catalogo_ids)
    if faltan or sobran:
        raise SystemExit(f"{_ruta(ruta)} no casa con actores_razas.RAZAS:\n"
                         f"  sin dibujo: {faltan}\n  ya no estan en el catalogo: {sobran}")
    return len(catalogo_ids)


# --- atlas de capas -----------------------------------------------------------------

def generar_capas(tallas, salida, filas_medio=None):
    """Escribe los atlas de capas y devuelve la seccion del manifiesto.

    filas_medio: si se da, guarda ahi los frames de cada pieza de equipo en la talla
    media ((ranura, id) -> frames), para ver de que materiales esta hecha."""
    dir_capas = salida / "personajes"
    man = {"razas": {}, "cuerpos": {}, "ojos": {}, "pelo": {}, "barba": {}, "equipo": {}}
    tallas_ropa = dict(tallas)
    if not PL:  # en PixelLab, nagas y espectros visten su propio cuerpo
        tallas_ropa["medio_sin_piernas"] = tallas["medio"].sin_piernas()

    # razas: una fila por raza y una hoja por capa
    filas = {c: [] for c in CAPAS_RAZA}
    lista = []
    for r in razas.RAZAS:
        t = tallas[talla_de_raza(r, tallas)]
        por_ranura = {}
        for ranura, frames in pers.capas_raza(r, t):
            por_ranura.setdefault(ranura, []).append(frames)
        for c in CAPAS_RAZA:
            fr = juntar(por_ranura[c]) if c in por_ranura else vacia()
            comprobar(fr, f"raza {r['id']} ({c})")
            filas[c].append(fr)
        lista.append({
            "id": r["id"], "nombre": r["nombre"], "linaje": r["linaje"],
            "talla": talla_de_raza(r, tallas),
            "tallaRopa": talla_de_raza(r, tallas) if PL else ("medio_sin_piernas" if r.get("cuerpo") else r["talla"]),
            "fila": len(lista) + 1, "capas": [c for c in CAPAS_RAZA if c in por_ranura],
            "sin": r.get("sin", []), "piel": r["piel"] if isinstance(r["piel"], str) else None,
            "ojos": r["ojos"], "pelo": list(r["pelo"]) if r.get("pelo") else None,
            "barba": list(r["barba"]) if r.get("barba") else None,
            "equipo": pers.EQUIPO_RAZAS[r["id"]],
            # PL-JUEGO-01: si admite otra piel u otra talla (pers.cambia_cuerpo): sin rasgos, cola, alas ni
            # cuerpo propios en el catalogo, aunque su cabeza de PixelLab venga en «rasgos»
            **({"liso": bool(isinstance(r["piel"], str) and not (r.get("cuerpo") or r.get("rasgos")
                                                                or r.get("cola") or r.get("alas")))}
               if PL else {}),
        })
    for c in CAPAS_RAZA:
        guardar_atlas(filas[c], dir_capas / "razas" / f"{c}.png", COLUMNAS_CAPAS)
    man["razas"] = {"atlas": {c: f"personajes/razas/{c}.png" for c in CAPAS_RAZA},
                    "filas": len(lista), "lista": lista}

    # cuerpos lisos (otra piel u otra talla que la de la raza) y ojos de otro color
    from actores_capas import capa_cuerpo, capa_ojos
    from actores_materiales import MATERIALES, OJOS
    for clave, lista_ids, fn in (
            ("cuerpos", PIELES, lambda t, i: capa_cuerpo(t, MATERIALES[i])),
            ("ojos", list(OJOS), lambda t, i: capa_ojos(t, OJOS[i]))):
        atlas = {}
        for tk in ORDEN_TALLAS:
            filas_t = [fn(tallas[tk], i) for i in lista_ids]
            guardar_atlas(filas_t, dir_capas / "cuerpos" / f"{clave}_{tk}.png", COLUMNAS_CAPAS)
            atlas[tk] = f"personajes/cuerpos/{clave}_{tk}.png"
        man[clave] = {"atlas": atlas, "filas": [{"id": i, "fila": n + 1} for n, i in enumerate(lista_ids)]}

    # pelo y barba: una hoja por talla
    combos = [(e, c) for e in pelo.ESTILOS for c in COLORES_PELO]
    combos += [(e, c) for e in ("corto", "largo") for c in COLORES_PELO_RAZA]
    combos += [("largo", "pelo_llama"), ("cresta", "pelo_llama")]  # Oriol (23/10): los del fuego (2000 b.f.)
    barbas = [(b, c) for b in BARBAS for c in COLORES_PELO]
    for clave, lista_combos, fn in (("pelo", combos, pers.capa_pelo), ("barba", barbas, pers.capa_barba)):
        atlas = {}
        for tk in ORDEN_TALLAS:
            filas_t = []
            for e, c in lista_combos:
                fr = fn(tallas[tk], e, c)
                comprobar(fr, f"{clave} {e}/{c} ({tk})")
                filas_t.append(fr)
            guardar_atlas(filas_t, dir_capas / clave / f"{clave}_{tk}.png", COLUMNAS_CAPAS)
            atlas[tk] = f"personajes/{clave}/{clave}_{tk}.png"
        man[clave] = {"atlas": atlas, "filas": [
            {"id": f"{e}/{c}", "estilo": e, "color": c, "fila": i + 1}
            for i, (e, c) in enumerate(lista_combos)]}

    # equipo: una hoja por ranura y talla
    piezas_por_ranura = {
        "cabeza": {k: v["nombre"] for k, v in cascos.CABEZA.items()},
        "torso": {k: v[2] for k, v in equipo.TORSO.items()},
        "piernas": {k: v[2] for k, v in equipo.PIERNAS.items()},
        "pies": {k: v[2] for k, v in equipo.PIES.items()},
        "capa": {k: v[2] for k, v in equipo.CAPAS.items()},
        "arma": {k: v[3] for k, v in armas.ARMAS.items() if armas.ranura_de(k) == "arma"},
        "mano_izq": {k: v[3] for k, v in armas.ARMAS.items() if armas.ranura_de(k) == "mano_izq"},
    }
    for ranura, piezas in piezas_por_ranura.items():
        atlas = {}
        tallas_ranura = ORDEN_TALLAS + (["medio_sin_piernas"] if ranura in ("torso", "capa") and not PL else [])
        ordenes = {}  # PL-JUEGO-01: {ident: {talla: orden}} de las armas (delante o detras, frame a frame)
        for tk in tallas_ranura:
            t = tallas_ropa[tk]
            filas_t = []
            for ident in piezas:
                _r, fr = pers.capa_equipo(t, ranura, ident, t)
                if PL and ranura in ("arma", "mano_izq") and isinstance(_r, dict):
                    ordenes.setdefault(ident, {})[tk] = dict(_r)
                comprobar(fr, f"{ranura} {ident} ({tk})")
                filas_t.append(fr)
                if filas_medio is not None and tk == "medio":
                    filas_medio[(ranura, ident)] = fr
            guardar_atlas(filas_t, dir_capas / "equipo" / f"{ranura}_{tk}.png", COLUMNAS_CAPAS)
            atlas[tk] = f"personajes/equipo/{ranura}_{tk}.png"
        extra = {}
        if ranura == "cabeza":
            extra = {"ocultaPelo": [k for k, v in cascos.CABEZA.items() if v["oculta_pelo"]]}
        # PNJ-01: las prendas de oficio solo las llevan los PNJ (la creacion no las ofrece). SP-02: los
        # guanteletes (uno en cada mano) traen su orden de dibujo: encima del cuerpo tambien de espaldas
        man["equipo"][ranura] = dict({"atlas": atlas, "piezas": [
            dict({"id": k, "nombre": n, "fila": i + 1}, **({"soloPnj": True} if k in oficios.SOLO_PNJ else {}),
                 **({"orden": orden_pl(ordenes[k])} if k in ordenes else
                    {"orden": armas.orden_de(k)} if ranura in ("arma", "mano_izq") and armas.orden_de(k) else {}))
            for i, (k, n) in enumerate(piezas.items())]}, **extra)
    return man


def orden_pl(por_talla):
    """PL-JUEGO-01: el orden de dibujo de un arma de PixelLab. Cada talla tiene su esqueleto, y en cada frame el
    arma va delante o detras del cuerpo segun su muneca este mas cerca o mas lejos de la camara que los
    hombros: el de la talla media para todas («f<n>») y, donde otra talla cambia, «<talla>/f<n>»
    (MontajePersonajes::capas se queda con el de la talla del atlas)."""
    base = por_talla.get("medio") or next(iter(por_talla.values()))
    orden = dict(base)
    for tk, o in por_talla.items():
        for clave, z in o.items():
            if clave.startswith("f") and base.get(clave) != z:
                orden[f"{tk}/{clave}"] = z
    return orden


# --- ESTETICA-01: paletas y estetica de cada objeto ----------------------------------------

def paletas():
    """material -> "luz base sombra contorno contorno_duro" en hex."""
    from actores_materiales import MATERIALES
    return {n: " ".join("%02x%02x%02x" % m.tono[t] for t in ("H", "s", "d", "o", "O"))
            for n, m in sorted(MATERIALES.items())}


def materiales_de_frames(frames, tonos_de):
    """Materiales de una pieza, de mas a menos pixeles: cada color opaco va al material
    que lo tiene entre sus tonos (si son varios, al que tiene mas tonos en la pieza)."""
    colores = {}
    for px in frames:
        opaco = px[..., 3] > 0
        for c, n in zip(*np.unique(px[opaco][:, :3], axis=0, return_counts=True)):
            colores[tuple(int(v) for v in c)] = colores.get(tuple(int(v) for v in c), 0) + int(n)
    candidatos = {c: tonos_de.get(c, ()) for c in colores}
    apoyo = {}
    for c, ms in candidatos.items():
        for m in ms:
            apoyo[m] = apoyo.get(m, 0) + 1
    pixeles = {}
    for c, ms in candidatos.items():
        if ms:
            m = max(ms, key=lambda x: (apoyo[x], x))
            pixeles[m] = pixeles.get(m, 0) + colores[c]
    # un material cuenta si tiene dos tonos en la pieza, o uno que no es de nadie mas
    # (los remaches de laton, la hebilla)
    propio = {m for c, ms in candidatos.items() if len(ms) == 1 for m in ms}
    return [m for m, _n in sorted(pixeles.items(), key=lambda t: (-t[1], t[0]))
            if apoyo[m] >= 2 or m in propio]


def catalogo_estetico(ruta_equipo, filas_medio):
    """(objetos, sin pieza, avisos) de equipment.json con actores_estetica."""
    import actores_estetica as estetica
    from actores_materiales import MATERIALES
    # los acabados ("_insigne", "_sublime") salen de otro material y comparten sus
    # tonos: para saber de que esta hecha una pieza solo cuentan los materiales de base
    tonos_de = {}
    for n, m in MATERIALES.items():
        if n.endswith(("_insigne", "_sublime")):
            continue
        for t in ("H", "s", "d", "o", "O"):
            tonos_de.setdefault(m.tono[t], set()).add(n)
    cache = {}

    def materiales_de_pieza(ranura, pieza):
        if (ranura, pieza) not in cache:
            cache[(ranura, pieza)] = materiales_de_frames(filas_medio[(ranura, pieza)], tonos_de)
        return cache[(ranura, pieza)]

    datos = json.loads(Path(ruta_equipo).read_text(encoding="utf-8"))
    mapa, sin, avisos = {}, [], []
    for o in catalogo._objetos(datos):
        ident = o.get("id") or o.get("name")
        res = estetica.estetica(o, materiales_de_pieza, MATERIALES)
        if res is None:
            # ESTETICA-01: lo que no se ve puesto brilla igual (particulas en el cuerpo)
            sin.append({"id": ident, "nombre": o.get("name") or ident, "slot": o.get("slot"),
                        "brillo": estetica.brillo_sin_pieza(o)})
            continue
        # Un tono de un material de origen que tambien sea de otro material de la pieza
        # tenaria pixeles que no son suyos: se avisa (y manda el primero, como en el motor).
        if res["tinte"] and res["pieza"] not in formas.BASES:
            presentes = set(materiales_de_pieza(res["ranura"], res["pieza"]))
            for origen, _destino in res["tinte"]:
                otros = {m for t in ("H", "s", "d", "o", "O") for m in tonos_de[MATERIALES[origen].tono[t]]}
                choque = (otros & presentes) - {origen} - {o_ for o_, _ in res["tinte"]}
                if choque:
                    avisos.append(f"{ident}: {origen} comparte tonos con {sorted(choque)} en {res['pieza']}")
        mapa[ident] = res
    return mapa, sin, avisos


# --- atlas horneados -------------------------------------------------------------------

def hornear(nombre, grupo, tallas, salida, usos, nota):
    """Atlas runtime con una fila por personaje (como actores_capas.png)."""
    filas, actores = [], []
    for r, (ident, spec) in enumerate(grupo.items()):
        fr = pers.componer_personaje(spec, tallas)
        comprobar(fr, f"{nombre} {ident}")
        filas.append(fr[:COLUMNAS])  # las hojas horneadas son de andar (sin el reposo)
        actores.append({
            "id": ident, "nombre": spec.get("nombre", ident), "fila": r + 1,
            "primerFrame": r * COLUMNAS + 1, "uso": usos.get(ident, ""),
            "composicion": _spec_json(spec),
            "animaciones": {k: [r * COLUMNAS + f for f in v] for k, v in ANIMACIONES.items()},
        })
    guardar_atlas(filas, salida / f"{nombre}.png")
    escribir_json({
        "_nota": nota + " Generado por tools/gen_personajes.py: no editar a mano. " + NOTA_REPARTO,
        "estado": "runtime", "generador": "tools/gen_personajes.py", "cuerpoBase": "personaje.png",
        "image": f"{nombre}.png", "frameSize": [FRAME, FRAME], "columns": COLUMNAS,
        "rows": len(filas),
        "anclaje": {"pies": "centro inferior de la celda", "filaPies": 60,
                    "dibujo": "64x64 con characterAnchor(), igual que personaje.png"},
        "direcciones": DIRECCIONES, "animacionesPorFila": ANIMACIONES, "actores": actores,
    }, salida / f"{nombre}.json")
    return filas


def _spec_json(spec):
    out = {}
    for k, v in spec.items():
        if k == "nombre":
            continue
        out[k] = list(v) if isinstance(v, tuple) else v
    return out


USOS_ACTORES = {
    "aventurero": "Jugador: sustituto de personaje.png como sprite del protagonista.",
    "aventurero_azul": "Tinte del aventurero: companero o PNJ viajero.",
    "aventurero_rojo": "Tinte del aventurero: companero o PNJ viajero.",
    "aventurero_pardo": "Tinte del aventurero: companero o PNJ viajero.",
    "guardia": "Guardia que patrulla (puertas y zonas de riesgo).",
    "aldeano": "Parroquiano, tendero o vecino que pasea.",
    "aldeana": "Parroquiana, tabernera o vecina que pasea.",
    "cultista": "Perdido encapuchado: saqueador o fanatico que se mueve.",
    "cultista_carcelero": "Carcelero del culto.",
}


def generar(vista=False, salida=TEX, pl=True):
    salida = Path(salida)
    tallas = _modo_pl() if pl else construir_tallas()
    ruta_razas = buscar("races.json")
    if ruta_razas:
        n = comprobar_razas(ruta_razas)
        print(f"{_ruta(ruta_razas)}: las {n} razas del catalogo tienen dibujo")
    filas_medio = {}
    man = generar_capas(tallas, salida, filas_medio)

    actores = hornear("actores_capas", pers.ACTORES, tallas, salida, USOS_ACTORES,
                      "Atlas runtime de actores genericos con caminar en 4 direcciones.")
    specs_razas = {r["id"]: pers.spec_raza(r["id"]) for r in razas.RAZAS}
    usos_razas = {r["id"]: f"PNJ de raza {r['nombre']} (talla {r['talla']}), vestido como en "
                           "race_npc_idle.png" for r in razas.RAZAS}
    for ident, spec in specs_razas.items():
        spec["nombre"] = razas.POR_ID[ident]["nombre"]
    filas_razas = hornear("razas_andando", specs_razas, tallas, salida, usos_razas,
                          "Las 43 razas del catalogo vestidas, andando en 4 direcciones "
                          "(sustituye a los retratos quietos de race_npc_idle.png).")
    historia = hornear("personajes_historia", pers.HISTORIA, tallas, salida,
                       {k: "Personaje de historia de Boundington a escala de juego."
                        for k in pers.HISTORIA},
                       "Personajes de historia de Boundington andando, en el estilo de los actores.")
    # PNJ-01: todos los PNJ del juego, con las piezas del jugador (tools/gen_pnj.py). Lanzado como
    # programa, este modulo es __main__: gen_pnj tiene que ver este (con el modo PixelLab puesto), no otro
    sys.modules["gen_personajes"] = sys.modules[__name__]
    import gen_pnj
    hojas_pnj = gen_pnj.generar(tallas, salida, ROOT)

    # PL-JUEGO-01: personaje.png (el cuerpo base de antes, del que leen las herramientas) no cambia con
    # PixelLab: su contrato tampoco. Solo se reescribe con los personajes de antes.
    if not PL:
      escribir_json({
        "_nota": "Contrato del cuerpo base: 12 frames de 64x64 en una fila. " + NOTA_REPARTO,
        "estado": "runtime temporal (fallback de actores sin hoja propia)",
        "image": "personaje.png", "frameSize": [FRAME, FRAME], "columns": COLUMNAS, "rows": 1,
        "direcciones": DIRECCIONES, "animaciones": ANIMACIONES,
        "espejos": {"3": "espejo de 1", "9": "espejo de 7", "10": "espejo de 4",
                    "11": "espejo de 5", "12": "espejo de 6"},
        "vestidoEn": "actores_capas.png, razas_andando.png y personajes_historia.png "
                     "(tools/gen_personajes.py); capas en personajes.json",
      }, salida / "personaje.json")

    # ESTETICA-01: la estetica de cada objeto antes que el manifiesto (los acabados
    # crean sus paletas, y el manifiesto las lleva todas)
    ruta_equipo = buscar("equipment.json")
    if ruta_equipo:
        mapa, sin, avisos = catalogo_estetico(ruta_equipo, filas_medio)
        escribir_json({"_nota": ("Pieza visible de cada objeto del catalogo: su forma (la fila), su tinte "
                                 "([material de la pieza, material del objeto]) y de donde sale su estetica "
                                 "(tools/actores_estetica.py; ESTETICA-01)."),
                       "fuente": _ruta(ruta_equipo), "objetos": mapa, "sinPieza": sin},
                      salida / "personajes" / "equipo_catalogo.json")
        print(f"{_ruta(ruta_equipo)}: {len(mapa)} objetos con pieza, {len(sin)} sin pieza")
        for aviso in avisos:
            print("  aviso:", aviso)

    orden = {r: dict(v) for r, v in capas_mod.ORDEN.items()}
    alturas = {k: tallas[k].altura() for k in ORDEN_TALLAS}
    nombre_talla = (lambda k: NOMBRES_PL[k]) if PL else (lambda k: TALLAS[k]["nombre"])
    altura_m = (lambda k: TALLAS.get(k, {}).get("altura_m", "")) if PL else (lambda k: TALLAS[k]["altura_m"])
    # PART-02b: las manos, el pecho y la cintura de cada frame, de donde echan los abalorios
    anclas = actores_anclas.anclas(tallas, ORDEN_TALLAS)
    manifiesto = {
        "_nota": ("Personajes por capas: cada atlas es una tira de 20 frames por fila, "
                  "transparente fuera de su pieza: los 12 de andar con el mismo reparto que "
                  "personaje.png y 8 de reposo ('reposo': dos por direccion, el cuerpo quieto y "
                  "respirando, con el arma como se lleva en reposo; ESTETICA-01). Un personaje se "
                  "monta dibujando, en cada frame, la fila de cada capa en el mismo sitio y en el "
                  "orden de 'orden' (menor = antes) para la direccion del frame. Generado por "
                  "tools/gen_personajes.py: no editar a mano. " + NOTA_REPARTO + " PART-02b: "
                  "'anclas' de cada talla, donde estan en cada frame (x,y dentro del frame, del 1 al "
                  "20) el puno de la mano del arma ('mano'), el de la otra ('otraMano'), el pecho "
                  "bajo el cuello ('pecho') y la linea del cinturon ('cintura'): de ahi echan sus "
                  "particulas los anillos, el collar y el cinturon (tools/actores_anclas.py)."),
        "generador": "tools/gen_personajes.py", "cuerpoBase": "personaje.png",
        "frameSize": [FRAME, FRAME], "columns": COLUMNAS_CAPAS, "frames": COLUMNAS_CAPAS,
        "direcciones": DIRECCIONES, "reposo": REPOSO, "animacionesPorFila": ANIMACIONES,
        "orden": orden,
        "tallas": dict({k: {"nombre": nombre_talla(k), "alturaM": altura_m(k),
                            "alturaPx": alturas[k],
                            "anclas": {p: " ".join(f"{x},{y}" for x, y in v)
                                       for p, v in anclas[k].items()}} for k in ORDEN_TALLAS},
                       **({} if PL else {"medio_sin_piernas": {"nombre": "Normal sin piernas (nagas y espectros)",
                                                               "base": "medio"}})),
        "razas": man["razas"], "cuerpos": man["cuerpos"], "ojos": man["ojos"], "pelo": man["pelo"],
        "barba": man["barba"], "equipo": man["equipo"],
        "personajes": {"actores": {k: _spec_json(v) for k, v in pers.ACTORES.items()},
                       "historia": {k: _spec_json(v) for k, v in pers.HISTORIA.items()}},
        "horneados": ["actores_capas.json", "razas_andando.json", "personajes_historia.json"]
                     + [f"{n}.json" for n in hojas_pnj],
        # ESTETICA-01: los cinco tonos de cada material (luz, base, sombra, contorno y
        # contorno duro), para tenir una pieza ("daga:hierro>plaga")
        "paletas": paletas(),
    }
    escribir_json(manifiesto, salida / "personajes.json")

    if RECORTADOS:
        import collections
        por = collections.Counter(r.rsplit(", frame", 1)[0] for r in RECORTADOS)
        print(f"PL-JUEGO-01: {len(RECORTADOS)} frames de {len(por)} piezas tocaban el borde y se han recortado:")
        for que, n in por.most_common(40):
            print(f"   {n:3d}  {que}")
    n_pelo = len(man["pelo"]["filas"])
    n_equipo = sum(len(v["piezas"]) for v in man["equipo"].values())
    print(f"{_ruta(salida / 'personajes')}: {len(razas.RAZAS)} razas x {len(CAPAS_RAZA)} capas, {n_pelo} peinados, "
          f"{len(man['barba']['filas'])} barbas y {n_equipo} piezas de equipo, en {len(ORDEN_TALLAS)} tallas"
          + (" (PixelLab, 40 frames)" if PL else ""))
    print(f"{_ruta(salida)}: actores_capas ({len(actores)}), razas_andando ({len(filas_razas)}) y "
          f"personajes_historia ({len(historia)}), + personajes.json" + ("" if PL else " y personaje.json"))
    if vista and not PL:
        import revision_personajes
        revision_personajes.revision(tallas, actores, filas_razas, historia)


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--vista", action="store_true", help="laminas en out/revision_personajes/")
    ap.add_argument("--salida", default=str(TEX), help="carpeta de salida (assets/textures)")
    ap.add_argument("--clasico", action="store_true",
                    help="los personajes de antes (cuatro vistas rectas, 20 frames) en vez de los de PixelLab")
    args = ap.parse_args()
    generar(args.vista, args.salida, pl=not args.clasico)
    # PIXELLAB-L7: las prendas de PixelLab viven en filas propias de los
    # atlas de equipo; regenerarlos las borra, asi que se reponen aqui.
    # PL-JUEGO-01: solo con los personajes de antes: son filas de 20 frames y
    # cuatro vistas rectas, que en los atlas de 40 de PixelLab no caben.
    indice_prendas = Path(args.salida) / "pixellab" / "prendas.json"
    if args.clasico and indice_prendas.is_file():
        import subprocess
        subprocess.run([sys.executable, str(Path(__file__).resolve().parent / "pixellab" / "importar_prendas.py"),
                        str(indice_prendas)], check=True)
